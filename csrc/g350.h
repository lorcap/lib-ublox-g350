#define MAX_BUF 1024
#define MAX_CMD 545
#define MAX_SOCKS 7
#define MAX_SOCK_HEX_BUF 128
#define MAX_SOCK_HEX_RXBUF 128+32
#define MAX_OPS   6
#define MAX_ERR_LEN 32
#define MAX_LAC_LEN 5
#define MAX_CI_LEN 5
#define MAX_BSIC_LEN 3
#define MAX_SMS_OADDR_LEN 16
#define MAX_SMS_TS_LEN 24
#define MAX_SMS_TXT_LEN 160
#define MAX_SMS_SCSA_LEN 32
#define GS_TIMEOUT 1000
#define GS_TLS_PROFILE 1

typedef struct _gsm_socket {
    uint8_t acquired;
    uint8_t proto;
    uint8_t to_be_closed;
    uint8_t secure;
    uint16_t unused;
    uint16_t timeout;
    VSemaphore rx;
    VSemaphore lock;
    uint8_t txbuf[MAX_SOCK_HEX_BUF];
    uint8_t rxbuf[MAX_SOCK_HEX_RXBUF];
} GSocket;

//COMMANDS

#define MAKE_CMD(group, command, response) (((group) << 24) | ((command) << 16) | (response))
#define DEF_CMD(cmd, response, urc, id)         \
    {                                           \
        cmd, sizeof(cmd) - 1, response, urc, id \
    }

typedef struct _gs_cmd {
    uint8_t body[16];
    uint8_t len;
    uint8_t response_type;
    uint8_t urc;
    uint8_t id;
} GSCmd;

//COMMAND SLOTS

#define MAX_SLOTS 16
typedef struct _gs_slot {
    GSCmd *cmd;
    uint8_t err;
    uint8_t allocated;
    uint8_t has_params;
    uint8_t params;
    uint16_t max_size;
    uint16_t unused2;
    uint32_t stime;
    uint32_t timeout;
    uint8_t *resp;
    uint8_t *eresp;
} GSSlot;

////////////OPERATORS

typedef struct _gs_operator {
    uint8_t type;
    uint8_t fmtl_l;
    uint8_t fmts_l;
    uint8_t fmtc_l;
    uint8_t fmt_long[24];
    uint8_t fmt_short[10];
    uint8_t fmt_code[6];
}GSOp;

typedef struct _gs_timestamp {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    int16_t timezone;
} GSTimestamp;

typedef struct _gs_sms {
    uint8_t oaddr[MAX_SMS_OADDR_LEN];
    uint8_t ts[MAX_SMS_TS_LEN];
    uint8_t txt[MAX_SMS_TXT_LEN];
    uint8_t oaddrlen;
    uint8_t tslen;
    uint8_t unread;
    uint8_t txtlen;
    int index;
} GSSMS;

////////////GSM STATUS

typedef struct _gsm_status{
    uint8_t initialized;
    uint8_t volatile talking;
    uint8_t volatile running;
    uint8_t attached;
    uint8_t registered;
    uint8_t gsm_status;
    uint8_t gprs_status;
    int8_t secure_sock_id;
    uint16_t gprs;
    uint8_t errlen;
    uint8_t mode;
    uint8_t rssi;
    uint8_t serial;
    uint16_t dtr;
    uint16_t rts;
    uint16_t rx;
    uint16_t tx;
    uint16_t poweron;
    uint16_t reset;
    uint16_t bytes;
    GSSlot *slot;
    VSemaphore sendlock;
    VSemaphore slotlock;
    VSemaphore slotdone;
    VThread thread;
    uint8_t errmsg[MAX_ERR_LEN];
    uint8_t buffer[MAX_CMD];
    uint8_t lac[MAX_LAC_LEN];
    uint8_t ci[MAX_CI_LEN];
    uint8_t bsic[MAX_BSIC_LEN];
    uint8_t tech;
    uint8_t skipsms;
    uint8_t maxsms;
    int offsetsms;
    int cursms;
    int pendingsms;
    GSSMS* sms;
} GStatus;

//DEFINES
#define GS_PROFILE 0

#define GS_ERR_OK      0
#define GS_ERR_TIMEOUT 1
#define GS_ERR_INVALID 2

// keep order, so that >= OK is registered
#define GS_REG_NOT     0
#define GS_REG_UNKNOWN 1
#define GS_REG_SEARCH  2
#define GS_REG_DENIED  3
#define GS_REG_OK      4
#define GS_REG_ROAMING 5

// Radio Access Technology (bit field)
#define GS_RAT_GSM      0x01
#define GS_RAT_GPRS     0x02

#define KNOWN_COMMANDS (sizeof(gs_commands) / sizeof(GSCmd))
#define GS_MIN(a)   (((a) < (gs.bytes)) ? (a) : (gs.bytes))

#define GS_MODE_NORMAL 0
#define GS_MODE_PROMPT 1

#define GS_CMD_NORMAL 1
#define GS_CMD_URC    2
#define GS_CMD_LINE   4

//RESPONSES
// only ok
#define GS_RES_OK        0
// one line of params, then ok
#define GS_RES_PARAM_OK  1
// no answer
#define GS_RES_NO        2

enum {
    GS_CMD_CCID,
    GS_CMD_CCLK,
    GS_CMD_CGATT,
    GS_CMD_CGED,
    GS_CMD_CGREG,
    GS_CMD_CGSN,
    GS_CMD_CIEV,
    GS_CMD_CMEE,
    GS_CMD_CMER,
    GS_CMD_CMGD,
    GS_CMD_CMGF,
    GS_CMD_CMGL,
    GS_CMD_CMGS,
    GS_CMD_CMTI,
    GS_CMD_CNMI,
    GS_CMD_COPS,
    GS_CMD_CREG,
    GS_CMD_CSCA,
    GS_CMD_CSCS,
    GS_CMD_ECHO,
    GS_CMD_GMR,
    GS_CMD_IPR,
    GS_CMD_UDCONF,
    GS_CMD_UDNSRN,
    GS_CMD_UPSD,
    GS_CMD_UPSDA,
    GS_CMD_UPSND,
    GS_CMD_URAT,
    GS_CMD_USECMNG,
    GS_CMD_USECPRF,
    GS_CMD_USOCL,
    GS_CMD_USOCO,
    GS_CMD_USOCR,
    GS_CMD_USOCTL,
    GS_CMD_USOGO,
    GS_CMD_USOLI,
    GS_CMD_USORD,
    GS_CMD_USORF,
    GS_CMD_USOSEC,
    GS_CMD_USOSO,
    GS_CMD_USOST,
    GS_CMD_USOWR,
    GS_CMD_UUPSDA,
    GS_CMD_UUPSDD,
    GS_CMD_UUSOCL,
    GS_CMD_UUSOLI,
    GS_CMD_UUSORD,
    GS_CMD_UUSORF,
};

#define GS_GET_CMD(cmdid) (&gs_commands[cmdid])

static const GSCmd gs_commands[] = {
    DEF_CMD("+CCID"   ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_CCID   ),
    DEF_CMD("+CCLK"   ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_CCLK   ),
    DEF_CMD("+CGATT"  ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_CGATT  ),
    DEF_CMD("+CGED"   ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_CGED   ),
    DEF_CMD("+CGREG"  ,   GS_RES_OK, GS_CMD_NORMAL|GS_CMD_URC, GS_CMD_CGREG  ),
    DEF_CMD("+CGSN"   ,   GS_RES_NO, GS_CMD_NORMAL           , GS_CMD_CGSN   ),
    DEF_CMD("+CIEV"   ,   GS_RES_NO, GS_CMD_NORMAL|GS_CMD_URC, GS_CMD_CIEV   ),
    DEF_CMD("+CMEE"   ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_CMEE   ),
    DEF_CMD("+CMER"   ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_CMER   ),
    DEF_CMD("+CMGD"   ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_CMGD   ),
    DEF_CMD("+CMGF"   ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_CMGF   ),
    DEF_CMD("+CMGL"   ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_CMGL   ),
    DEF_CMD("+CMGS"   ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_CMGS   ),
    DEF_CMD("+CMTI"   ,   GS_RES_OK,               GS_CMD_URC, GS_CMD_CMTI   ),
    DEF_CMD("+CNMI"   ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_CNMI   ),
    DEF_CMD("+COPS"   ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_COPS   ),
    DEF_CMD("+CREG"   ,   GS_RES_OK, GS_CMD_NORMAL|GS_CMD_URC, GS_CMD_CREG   ),
    DEF_CMD("+CSCA"   ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_CSCA   ),
    DEF_CMD("+CSCS"   ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_CSCS   ),
    DEF_CMD("E"       ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_ECHO   ),
    DEF_CMD("+GMR"    ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_GMR    ),
    DEF_CMD("+IPR"    ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_IPR    ),
    DEF_CMD("+UDCONF" ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_UDCONF ),
    DEF_CMD("+UDNSRN" ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_UDNSRN ),
    DEF_CMD("+UPSD"   ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_UPSD   ),
    DEF_CMD("+UPSDA"  ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_UPSDA  ),
    DEF_CMD("+UPSND"  ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_UPSND  ),
    DEF_CMD("+URAT"   ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_URAT   ),
    DEF_CMD("+USECMNG",   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_USECMNG),
    DEF_CMD("+USECPRF",   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_USECPRF),
    DEF_CMD("+USOCL"  ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_USOCL  ),
    DEF_CMD("+USOCO"  ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_USOCO  ),
    DEF_CMD("+USOCR"  ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_USOCR  ),
    DEF_CMD("+USOCTL" ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_USOCTL ),
    DEF_CMD("+USOGO"  ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_USOGO  ),
    DEF_CMD("+USOLI"  ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_USOLI  ),
    DEF_CMD("+USORD"  ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_USORD  ),
    DEF_CMD("+USORF"  ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_USORF  ),
    DEF_CMD("+USOSEC" ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_USOSEC ),
    DEF_CMD("+USOSO"  ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_USOSO  ),
    DEF_CMD("+USOST"  ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_USOST  ),
    DEF_CMD("+USOWR"  ,   GS_RES_OK, GS_CMD_NORMAL           , GS_CMD_USOWR  ),
    DEF_CMD("+UUPSDA" ,   GS_RES_NO,               GS_CMD_URC, GS_CMD_UUPSDA ),
    DEF_CMD("+UUPSDD" ,   GS_RES_NO,               GS_CMD_URC, GS_CMD_UUPSDD ),
    DEF_CMD("+UUSOCL" ,   GS_RES_NO,               GS_CMD_URC, GS_CMD_UUSOCL ),
    DEF_CMD("+UUSOLI" ,   GS_RES_NO,               GS_CMD_URC, GS_CMD_UUSOLI ),
    DEF_CMD("+UUSORD" ,   GS_RES_NO,               GS_CMD_URC, GS_CMD_UUSORD ),
    DEF_CMD("+UUSORF" ,   GS_RES_NO,               GS_CMD_URC, GS_CMD_UUSORF ),
};

extern GStatus gs;
extern GSOp gsops[MAX_OPS];
extern int gsopn;

void _gs_init(void);
int _gs_start(void);
int _gs_stop(void);
void _gs_loop(void* args);
int _gs_poweron(void);
int _gs_wait_for_ok(int timeout);
int _gs_attach (void);
int _gs_config0(void);
int _gs_control_psd(int tag);
int _gs_configure_psd(int tag, uint8_t* param, int len);
int _gs_query_psd(int query, uint8_t** param, uint32_t* param_len);
int _gs_set_gsm_status_from_creg(uint8_t* buf, uint8_t* ebuf, int from_urc);
int _gs_set_gprs_status_from_cgreg(uint8_t* buf, uint8_t* ebuf, int from_urc);
int _gs_list_operators(void);
int _gs_set_operator(uint8_t *opname, uint32_t oplen);
int _gs_check_network(void);
int _gs_cell_info(int* mcc, int* mnc);
int _gs_imei(uint8_t* imei);
int _gs_iccid(uint8_t* iccid);
int _gs_resolve(uint8_t* url, int len, uint8_t* addr);
int _gs_get_rtc(uint8_t* time);

int _gs_sms_send(uint8_t* num, int numlen, uint8_t* txt, int txtlen);
int _gs_sms_list(int unread, GSSMS* sms, int maxsms, int offset);
int _gs_sms_delete(int index);
int _gs_sms_get_scsa(uint8_t* scsa);
int _gs_sms_set_scsa(uint8_t* scsa, int scsalen);
