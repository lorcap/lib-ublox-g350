#include "zerynth.h"
#include "g350.h"

//a reference to a Python exception to be returned on error (g350Exception)
int32_t g350exc;

#if 1
#define printf(...) vbl_printf_stdout(__VA_ARGS__)
#define print_buffer(bf, ln)    for(uint8_t i = 0; i < ln; i++) { \
                                        printf("%c", bf[i]); \
                                } \
                                printf("\n");
#else
#define printf(...)
#define print_buffer(bf, ln)
#endif


///////// CNATIVES
// The following functions are callable from Python.
// Functions starting with "_" are utility functions called by CNatives

/**
 * @brief _g350_init calls _gs_init, _gs_poweron and _gs_config0
 *
 * As last parameter, requires an integer saved to global g350exc, representing the name assigned to g350Exception
 * so that it can be raised by returning g350exc. If modules initialization is successful, starts the main thread
 *
 */
C_NATIVE(_g350_init)
{
    NATIVE_UNWARN();
    int32_t serial;
    int32_t rts;
    int32_t dtr;
    int32_t poweron;
    int32_t reset;
    int32_t err = ERR_OK;
    int32_t exc;

    if (parse_py_args("iiiiii", nargs, args, &serial, &dtr, &rts, &poweron, &reset, &exc) != 6)
        return ERR_TYPE_EXC;

    g350exc = exc;

    *res = MAKE_NONE();


    RELEASE_GIL();
    _gs_init();
    gs.serial = serial&0xff;
    gs.rx = _vm_serial_pins[gs.serial].rxpin;
    gs.tx = _vm_serial_pins[gs.serial].txpin;
    gs.dtr = dtr;
    gs.rts = rts;
    gs.poweron = poweron;
    gs.reset = reset;
    ACQUIRE_GIL();

    return err;
}

/**
 * @brief Setup modem serial port, AT configuration and start modem thread
 */
C_NATIVE(_g350_startup)
{
    NATIVE_UNWARN();
    int32_t err = ERR_OK;
    *res = MAKE_NONE();

    RELEASE_GIL();
    vosSemWait(gs.slotlock);

    if (_gs_stop() != 0)
        err = ERR_HARDWARE_INITIALIZATION_ERROR;
    else
    if (!_gs_poweron()) {
        err = ERR_HARDWARE_INITIALIZATION_ERROR;
    }
    else
    if (!_gs_config0())
        err = ERR_HARDWARE_INITIALIZATION_ERROR;
    else {
        if (gs.thread==NULL){
            //let's start modem thread (if not already started)
            printf("Starting modem thread with size %i\n", VM_DEFAULT_THREAD_SIZE);
            gs.thread = vosThCreate(VM_DEFAULT_THREAD_SIZE, VOS_PRIO_NORMAL, _gs_loop, NULL, NULL);
            vosThResume(gs.thread);
            vosThSleep(TIME_U(1000, MILLIS)); // let modem thread have a chance to start
        }
    }
    // reset driver status (assuming modem has restarted)
    gs.attached = 0;
    gs.registered = 0;

    // start loop and wait
    if (_gs_start() != 0)
        err = ERR_HARDWARE_INITIALIZATION_ERROR;

    vosSemSignal(gs.slotlock);
    ACQUIRE_GIL();
    return err;
}

/**
 * @brief Stop modem thread and close serial port
 */
C_NATIVE(_g350_shutdown)
{
    NATIVE_UNWARN();
    int32_t err = ERR_OK;
    *res = MAKE_NONE();

    RELEASE_GIL();
    vosSemWait(gs.slotlock);

    if (_gs_stop() != 0)
        err = ERR_HARDWARE_INITIALIZATION_ERROR;

    // attempt normal shutdown
    vhalSerialInit(gs.serial, 115200, SERIAL_CFG(SERIAL_PARITY_NONE,SERIAL_STOP_ONE, SERIAL_BITS_8, 0, 0), gs.rx, gs.tx);
    // check alive
    vhalSerialWrite(gs.serial, "ATE0\r\n", 6);
    if (_gs_wait_for_ok(500)) {
        //enter minimal functionality
        vhalSerialWrite(gs.serial, "AT+CFUN=0\r\n", 11);
        _gs_wait_for_ok(15000);
        *res = PSMALLINT_NEW(1);
    }
    vhalSerialDone(gs.serial);

    vosSemSignal(gs.slotlock);

    ACQUIRE_GIL();
    return err;
}

/**
 * @brief Stop/restart modem thread
 *
 * Give direct access to modem serial port
 */
C_NATIVE(_g350_bypass){
    NATIVE_UNWARN();
    int32_t mode;
    int32_t err=ERR_OK;

    if(parse_py_args("i",nargs,args,&mode)!=1) return ERR_TYPE_EXC;

    *res = MAKE_NONE();
    if (mode) {
        vosSemWait(gs.slotlock);
        if (_gs_stop() != 0)
            err = ERR_HARDWARE_INITIALIZATION_ERROR;
    }
    else {
        if (_gs_start() != 0)
            err = ERR_HARDWARE_INITIALIZATION_ERROR;
        vosSemSignal(gs.slotlock);
    }
    return err;
}

/**
 * @brief _bg96_detach removes the link with the APN while keeping connected to the GSM network
 */
C_NATIVE(_g350_detach)
{
    NATIVE_UNWARN();
    int err = ERR_OK;
    *res = MAKE_NONE();
    RELEASE_GIL();

    if(!_gs_control_psd(4))
        err = g350exc;

    ACQUIRE_GIL();
    return err;
}

/**
 * @brief _g350_attach tries to link to the given APN
 *
 * This function can block for a very long time (up to 2 minutes) due to long timeout of used AT commands
 */
C_NATIVE(_g350_attach)
{
    NATIVE_UNWARN();
    uint8_t* apn;
    uint32_t apn_len;
    uint8_t* user;
    uint32_t user_len;
    uint8_t* password;
    uint32_t password_len;
    uint32_t authmode;
    int32_t timeout;
    int32_t wtimeout;
    int32_t err = ERR_OK;

    if (parse_py_args("sssii", nargs, args, &apn, &apn_len, &user, &user_len, &password, &password_len, &authmode, &wtimeout) !=5 )
        return ERR_TYPE_EXC;

    *res = MAKE_NONE();
    RELEASE_GIL();

    err = _gs_attach();
    if (err)
        goto exit;

    //wait until timeut or GPRS attached (by urc +CREG or +CIEV)
    timeout = wtimeout;
    while (timeout > 0) {
        _gs_check_network();
        if (gs.registered == GS_REG_OK || gs.registered == GS_REG_ROAMING)
            break;
        vosThSleep(TIME_U(100, MILLIS));
        timeout -= 100;
    }
    if (timeout < 0) {
        err = ERR_TIMEOUT_EXC;
        goto exit;
    }

    //deactivate PSD
    _gs_control_psd(4);

    //Get profile status (if already linked, ignore the following configuration)
    // i = _gs_query_psd(8,NULL,NULL);
    // if(i==1) {
        // goto exit;
    // }
    //configure PSD: give apn first, then username, password and authmode
    err = g350exc;
    if (!_gs_configure_psd(1, apn, apn_len))
        goto exit;
    if (user_len) {
        if (!_gs_configure_psd(2, user, user_len))
            goto exit;
    }
    if (password_len) {
        if (!_gs_configure_psd(3, password, password_len))
        goto exit;
    }
    if (!_gs_configure_psd(6, NULL, authmode))
            goto exit;

    //activate PSD
    gs.attached = 0;
    if (!_gs_control_psd(3))
        goto exit;

    //wait for attached (set by +UUPSDA or queried by +UPSND)
    timeout = wtimeout;
    while (timeout > 0) {
        if (gs.attached)
            break;
        vosThSleep(TIME_U(1000, MILLIS));
        timeout -= 1000;
        if (_gs_query_psd(8, NULL, NULL)) {
            gs.attached = 1;
            break;
        }
    }
    if (timeout < 0)
        err = ERR_TIMEOUT_EXC;
    else
        err = ERR_OK;

exit:
    ACQUIRE_GIL();
    return err;
}

/**
 * @brief _g350_operators retrieve the operator list and converts it to a tuple
 *
 *
 */
C_NATIVE(_g350_operators)
{
    NATIVE_UNWARN();
    int i;

    RELEASE_GIL();
    i = _gs_list_operators();
    ACQUIRE_GIL();
    if (i) {
        *res = MAKE_NONE();
        return ERR_OK;
    }
    PTuple* tpl = ptuple_new(gsopn, NULL);
    for (i = 0; i < gsopn; i++) {
        PTuple* tpi = ptuple_new(4, NULL);
        PTUPLE_SET_ITEM(tpi, 0, PSMALLINT_NEW(gsops[i].type));
        PTUPLE_SET_ITEM(tpi, 1, pstring_new(gsops[i].fmtl_l, gsops[i].fmt_long));
        PTUPLE_SET_ITEM(tpi, 2, pstring_new(gsops[i].fmts_l, gsops[i].fmt_short));
        PTUPLE_SET_ITEM(tpi, 3, pstring_new(gsops[i].fmtc_l, gsops[i].fmt_code));
        PTUPLE_SET_ITEM(tpl, i, tpi);
    }

    *res = tpl;
    return ERR_OK;
}

/**
 * @brief _g350_set_operator try to set the current operator given its name
 */
C_NATIVE(_g350_set_operator)
{
    NATIVE_UNWARN();
    int i;
    uint8_t* opname;
    uint32_t oplen;

    if (parse_py_args("s", nargs, args, &opname, &oplen) != 1)
        return ERR_TYPE_EXC;

    RELEASE_GIL();
    i = _gs_set_operator(opname, oplen);
    ACQUIRE_GIL();
    *res = MAKE_NONE();
    if (i == GS_TIMEOUT) {
        return ERR_TIMEOUT_EXC;
    } else {
        return g350exc;
    }
    return ERR_OK;
}

/**
 * @brief _g350_rssi return the signal strength as reported by +CIEV urc
 */
C_NATIVE(_g350_rssi)
{
    NATIVE_UNWARN();
    int32_t rssi = -105 + 12 * gs.rssi;

    *res = PSMALLINT_NEW(rssi);
    return ERR_OK;
}

/**
 * @brief _g350_network_info retrieves network information through +URAT and *CGED
 */
C_NATIVE(_g350_network_info)
{
    NATIVE_UNWARN();
    int mcc = 0, mnc = 0;
    PString* str = NULL;
    PTuple* tpl = ptuple_new(8, NULL);

    _gs_get_rat();
    str = pstring_new(strlen(gs.rat), gs.rat);
    PTUPLE_SET_ITEM(tpl, 0, str);

    _gs_cell_info(&mcc, &mnc);
    PTUPLE_SET_ITEM(tpl, 1, PSMALLINT_NEW(mcc));
    PTUPLE_SET_ITEM(tpl, 2, PSMALLINT_NEW(mnc));
    str = pstring_new(strlen(gs.bsic), gs.bsic);
    PTUPLE_SET_ITEM(tpl, 3, str);
    str = pstring_new(strlen(gs.lac), gs.lac);
    PTUPLE_SET_ITEM(tpl, 4, str);
    str = pstring_new(strlen(gs.ci), gs.ci);
    PTUPLE_SET_ITEM(tpl, 5, str);
    if (!PTUPLE_ITEM(tpl, 1)) {
        //empty result
        str = pstring_new(0, NULL);
        PTUPLE_SET_ITEM(tpl, 1, PSMALLINT_NEW(-1));
        PTUPLE_SET_ITEM(tpl, 2, PSMALLINT_NEW(-1));
        PTUPLE_SET_ITEM(tpl, 3, str);
        PTUPLE_SET_ITEM(tpl, 4, str);
        PTUPLE_SET_ITEM(tpl, 5, str);
    }

    //registered to network
    PTUPLE_SET_ITEM(tpl, 6, gs.registered ? PBOOL_TRUE() : PBOOL_FALSE());
    //attached to APN
    PTUPLE_SET_ITEM(tpl, 7, gs.attached ? PBOOL_TRUE() : PBOOL_FALSE());

    *res = tpl;
    return ERR_OK;
}

/**
 * @brief _g350_mobile_info retrieves info on IMEI and SIM card by means of +CGSN and *CCID
 */
C_NATIVE(_g350_mobile_info)
{
    NATIVE_UNWARN();
    uint8_t imei[16];
    uint8_t iccid[22];
    int im_len;
    int ic_len;

    PTuple* tpl = ptuple_new(2, NULL);
    RELEASE_GIL();
    im_len = _gs_imei(imei);
    ic_len = _gs_iccid(iccid);

    if(im_len <= 0) {
        PTUPLE_SET_ITEM(tpl, 0, pstring_new(0, NULL));
    } else {
        PTUPLE_SET_ITEM(tpl, 0, pstring_new(im_len, imei));
    }
    if(ic_len <= 0) {
        PTUPLE_SET_ITEM(tpl, 1, pstring_new(0, NULL));
    } else {
        PTUPLE_SET_ITEM(tpl, 1, pstring_new(ic_len, iccid));
    }
    ACQUIRE_GIL();
    *res = tpl;
    return ERR_OK;
}

/**
 * @brief _g350_link_info retrieves ip and dns by means of +UPSND
 */
C_NATIVE(_g350_link_info)
{
    NATIVE_UNWARN();
    PString* ips;
    PString* dns;
    uint8_t* addr;
    uint32_t addrlen;

    RELEASE_GIL();

    if (_gs_query_psd(0, &addr, &addrlen)) {
        ips = pstring_new(addrlen - 2, addr + 1);
    } else {
        ips = pstring_new(0, NULL);
    }

    if (_gs_query_psd(1, &addr, &addrlen)) {
        dns = pstring_new(addrlen - 2, addr + 1);
    } else {
        dns = pstring_new(0, NULL);
    }

    ACQUIRE_GIL();
    PTuple *tpl = ptuple_new(2, NULL);
    PTUPLE_SET_ITEM(tpl, 0, ips);
    PTUPLE_SET_ITEM(tpl, 1, dns);
    *res = tpl;
    return ERR_OK;
}
