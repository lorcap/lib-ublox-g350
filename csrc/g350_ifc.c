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
    if (!_gs_poweron()) {
        err = ERR_HARDWARE_INITIALIZATION_ERROR;
    }
    else {
        if (!_gs_config0())
            err = ERR_HARDWARE_INITIALIZATION_ERROR;
    }
    ACQUIRE_GIL();

    if (err==ERR_OK){
        //let's start modem thread
        printf("Starting modem thread with size %i\n", VM_DEFAULT_THREAD_SIZE);
        gs.thread = vosThCreate(VM_DEFAULT_THREAD_SIZE, VOS_PRIO_NORMAL, _gs_loop, NULL, NULL);
        vosThResume(gs.thread);
    }

    return err;
}
