/** @file ril.c
 *
 * @author  Lorenzo Cappelletti
 * @date    2021-03-21
 */

#include <stdlib.h>
#include <string.h>
#include "ril.h"

/* *** Internal ********************************************************** */

static bool
_at_read (ril_state_t* st,
          const char* cmd,
          int* val)
{
        ril_cmd_query(st, cmd     );
        ril_rsp_echo (st          );
        ril_rsp_query(st, cmd, val);
        ril_rsp_eol  (st          );
        ril_rsp_final(st          );
        return st->error;
}

static bool
_at_set (ril_state_t* st,
         const char* cmd,
         int val)
{
        ril_cmd_set  (st, cmd);
        ril_cmd_int  (st, val);
        ril_cmd_eol  (st     );
        ril_rsp_echo (st     );
        ril_rsp_final(st     );
        return st->error;
}


/* *** Init/deinit ******************************************************* */

ril_state_t*
ril_init (ril_cmd_write_t write,
          void* write_obj,
          ril_rsp_read_t read,
          void* read_obj,
          size_t buffer_max)
{
        ril_state_t* st = malloc(sizeof(ril_state_t) + buffer_max);
        ril_cmd_init(st, write, write_obj);
        ril_rsp_init(st, read, read_obj, buffer_max);
        return st;
}

void
ril_deinit (ril_state_t* st)
{
        ril_cmd_deinit(st);
        ril_rsp_deinit(st);
        free(st);
}


/* *** General operation ************************************************* */

int
ril_at_cgmr_read (ril_state_t* st,
                  char* v)
{
        ril_cmd_printf(st, "%A%$", "+CGMR");
        ril_rsp_echo  (st);
        ril_rsp_strp  (st, "^\r\n", v);
        ril_rsp_eol   (st);
        ril_rsp_final (st);
        return st->error;
}

int
ril_at_ccid_read (ril_state_t* st,
                  char* ccid)
{
        ril_cmd_printf(st, "%A%$", "+CCID");
        ril_rsp_echo  (st);
        ril_rsp_scanf (st, "%s: %/s%$", "+CCID", "0-9", ccid);
        ril_rsp_final (st);
        return st->error;
}


/* *** General *********************************************************** */

int
ril_at_cscs_read (ril_state_t* st,
                  enum RIL_CSCS_CHSET* chset)
{
        char chset_[8];

        ril_cmd_query(st, "+CSCS" );
        ril_rsp_echo (st          );
        ril_rsp_scanf(st, "%s: %\"s%$", "+CSCS", chset_);
        ril_rsp_final(st          );

        if (!st->error)
        {
                if (strcmp(chset_, "IRA"))
                        *chset = RIL_CSCS_CHSET_IRA;
                else
                if (strcmp(chset_, "GSM"))
                        *chset = RIL_CSCS_CHSET_GSM;
                else
                if (strcmp(chset_, "PCCP437"))
                        *chset = RIL_CSCS_CHSET_PCCP437;
                else
                if (strcmp(chset_, "8859-1"))
                        *chset = RIL_CSCS_CHSET_8859_1;
                else
                if (strcmp(chset_, "UCS2"))
                        *chset = RIL_CSCS_CHSET_UCS2;
                else
                if (strcmp(chset_, "HEX"))
                        *chset = RIL_CSCS_CHSET_HEX;
                else
                if (strcmp(chset_, "PCCP936"))
                        *chset = RIL_CSCS_CHSET_PCCP936;
                else
                        *chset = RIL_CSCS_CHSET_UNKNOWN;
        }

        return st->error;
}

int
ril_at_cscs_set (ril_state_t* st,
                 enum RIL_CSCS_CHSET chset)
{
        const char* chset_ = NULL;

        switch (chset)
        {
        case RIL_CSCS_CHSET_IRA:
                chset_ = "IRA";
                break;

        case RIL_CSCS_CHSET_GSM:
                chset_ = "GSM";
                break;

        case RIL_CSCS_CHSET_PCCP437:
                chset_ = "PCCP437";
                break;

        case RIL_CSCS_CHSET_8859_1:
                chset_ = "8859-1";
                break;

        case RIL_CSCS_CHSET_UCS2:
                chset_ = "UCS2";
                break;

        case RIL_CSCS_CHSET_HEX:
                chset_ = "HEX";
                break;

        case RIL_CSCS_CHSET_PCCP936:
                chset_ = "PCCP936";
                break;

        default:
                st->error = RIL_ERR_BAD_PARAMETER;
        }

        ril_cmd_printf(st, "%A=%\"s%$", "+CSCS", chset_);
        ril_rsp_echo (st);
        ril_rsp_final(st);

        return st->error;
}


/* *** Mobile equipment control and status ******************************* */

int
ril_at_cmer_read (ril_state_t* st,
                  enum RIL_CMER_MODE* mode,
                  enum RIL_CMER_IND* ind,
                  enum RIL_CMER_BFR* bfr)
{
        ril_cmd_printf(st, "%A?%$", "+CMER");
        ril_rsp_echo  (st);
        ril_rsp_scanf (st, "%s: %d,%*d,%*d,%d,%d%$", "+CMER", (int*)mode, (int*)ind, (int*)bfr);
        ril_rsp_final (st);
        return st->error;
}

int
ril_at_cmer_set (ril_state_t* st,
                 enum RIL_CMER_MODE mode,
                 enum RIL_CMER_IND ind,
                 enum RIL_CMER_BFR bfr)
{
        ril_cmd_printf(st, "%A=%d,0,0,%d,%d%$", "+CMER", mode, ind, bfr);
        ril_rsp_echo  (st);
        ril_rsp_final (st);
        return st->error;
}

int
ril_at_cclk_read (ril_state_t* st,
                  int* year,
                  int* month,
                  int* day,
                  int* hours,
                  int* minutes,
                  int* seconds,
                  int* timezone)
{
        ril_cmd_printf(st, "%A?%$", "+CCLK");
        ril_rsp_echo  (st);
        ril_rsp_scanf (st, "%s: \"%d/%d/%d,%d:%d:%d%d\"%$", "+CCLK", year, month, day, hours, minutes, seconds, timezone);
        ril_rsp_final (st);

        if (!st->error)
        {
                *year += 2000;
                *timezone *= 15;
        }

        return st->error;
}

int
ril_at_cclk_set (ril_state_t* st,
                 int year,
                 int month,
                 int day,
                 int hours,
                 int minutes,
                 int seconds,
                 int timezone)
{
        year -= 2000;
        timezone /= 15;

        ril_cmd_printf(st, "%A=\"%2d/%2d/%2d,%2d:%2d:%2d%+3d\"%$", "+CCLK", year, month, day, hours, minutes, seconds, timezone);
        ril_rsp_echo  (st);
        ril_rsp_final (st);
        return st->error;
}

int
ril_at_cmee_read (ril_state_t* st,
                  enum RIL_CMEE_ERROR* n)
{
        return _at_read(st, "+CMEE", (int*)n);
}

int
ril_at_cmee_set (ril_state_t* st,
                 enum RIL_CMEE_ERROR n)
{
        return _at_set(st, "+CMEE", n);
}


/* *** Network service *************************************************** */

int
ril_at_cged_read (ril_state_t* st,
                  enum RIL_CGED_MODE* mode)
{
        ril_cmd_printf(st, "%A?%$", "+CGED");
        ril_rsp_echo  (st);
        ril_rsp_query (st, "+CGED", (int*)mode);
        ril_rsp_line_dump(st);
        ril_rsp_final (st);
        return st->error;
}

int
ril_at_cged_set (ril_state_t* st,
                 enum RIL_CGED_MODE mode)
{
        return _at_set(st, "+CGED", mode);
}


/* *** Short Messages Service ******************************************** */

int
ril_at_cmgf_read (ril_state_t* st,
                  int* mode)
{
        return _at_read(st, "+CMGF", mode);
}

int
ril_at_cmgf_set (ril_state_t* st,
                 int mode)
{
        return _at_set(st, "+CMGF", mode);
}

int
ril_at_csdh_read (ril_state_t* st,
                  int* mode)
{
        return _at_read(st, "+CMGF", mode);
}

int
ril_at_csdh_set (ril_state_t* st,
                 int mode)
{
        return _at_set(st, "+CMGF", mode);
}

int
ril_at_cnmi_read (ril_state_t* st,
                  int* mode,
                  int* mt,
                  int* bm,
                  int* ds,
                  int* bfr)
{
        ril_cmd_query(st, "+CNMI");
        ril_rsp_echo (st);
        ril_rsp_scanf(st, "%s: %d,%d,%d,%d,%d%$", "+CNMI", mode, mt, bm, ds, bfr);
        ril_rsp_final(st);
        return st->error;
}

int
ril_at_cnmi_set (ril_state_t* st,
                 int mode,
                 int mt,
                 int bm,
                 int ds,
                 int bfr)
{
        ril_cmd_printf(st, "%A=%d,%d,%d,%d,%d%$", "+CNMI", mode, mt, bm, ds, bfr);
        ril_rsp_echo  (st);
        ril_rsp_final (st);
        return st->error;
}

int
ril_at_csca_read (ril_state_t* st,
                  char* csa,
                  int* tosca)
{
        ril_cmd_query(st, "+CSCA");
        ril_rsp_echo (st);
        ril_rsp_scanf(st, "%s: %\"s,%d%$", "+CSCA", csa, tosca);
        ril_rsp_final(st);
        return st->error;
}

int
ril_at_csca_set (ril_state_t* st,
                 const char* csa)
{
        ril_cmd_printf(st, "%A=%\"s%$", "+CSCA", csa);
        ril_rsp_echo  (st);
        ril_rsp_final (st);
        return st->error;
}


/* *** V24 control and V25ter ******************************************** */

int
ril_ate_set (ril_state_t* st,
             int value)
{
        ril_cmd_at   (st       );
        ril_cmd_char (st, 'E'  );
        ril_cmd_int  (st, value);
        ril_cmd_eol  (st       );
        ril_rsp_echo (st);
        ril_rsp_final(st);
        return st->error;
}


/* *** Packet switched data servicies ************************************ */

int
ril_at_cgatt_read (ril_state_t* st,
                   int* state)
{
        return _at_read(st, "+CGATT", state);
}

int
ril_at_cgatt_set (ril_state_t* st,
                  int state)
{
        return _at_set(st, "+CGATT", state);
}

int
ril_at_cgreg_read (ril_state_t* st,
                   enum RIL_CGREG_N* n,
                   enum RIL_CGREG_STAT* stat,
                   unsigned int* lac,
                   unsigned int* ci)
{
        ril_cmd_printf(st, "%A?%$", "+CGREG");
        ril_rsp_echo  (st);
        ril_rsp_scanf (st, "%s: %d,%d", "+CGREG", (int*)n, (int*)stat);

        if (!st->error && *n == RIL_CGREG_N_NETWORK_REGISTRATION_AND_LOCATION_INFORMATION_URC_ENABLED)
                ril_rsp_scanf(st, ",\"%x\",\"%x\"", lac, ci);

        ril_rsp_eol  (st);
        ril_rsp_final(st);
        return st->error;
}

int
ril_at_cgreg_set (ril_state_t* st,
                  enum RIL_CGREG_N n)
{
        return _at_set(st, "+CGREG", n);
}
