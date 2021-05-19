/** @file ublox.c
 *
 * @author  Lorenzo Cappelletti
 * @date    2021-03-21
 */

#include <stdarg.h>
#include "rsp.h"
#include "ublox.h"

int
ril_at_udconf1 (ril_state_t* st,
                int* hex_mode)
{
        return ril_at_udconf(st, 120*60*1000, 1, hex_mode, NULL);
}

int
ril_at_udconf1_set (ril_state_t* st,
                    int hex_mode)
{
        return ril_at_udconf_set(st, 120*60*1000, 1, hex_mode, -1);
}

int
ril_at_udconf (ril_state_t* st,
               unsigned int timeout,
               int op_code,
               ...)
{
        if (st->error)
                return st->error;

        va_list args;
        va_start(args, op_code);

        ril_cmd_printf(st, "%A=%d", "+UDCONF", op_code);
        ril_cmd_eol   (st);
        ril_rsp_echo  (st, timeout);
        ril_rsp_scanf (st, "%s: %*d", "+UDCONF");

        for (int* arg; !st->error && (arg = va_arg(args, int*)); )
        {
                ril_rsp_char(st, ',');
                ril_rsp_int (st, arg);
        }

        ril_rsp_eol  (st);
        ril_rsp_final(st);

        va_end(args);
        return st->error;
}

int
ril_at_udconf_set (ril_state_t* st,
                   unsigned int timeout,
                   int op_code,
                   ...)
{
        if (st->error)
                return st->error;

        va_list args;
        va_start(args, op_code);

        ril_cmd_printf(st, "%A=%d", "+UDCONF", op_code);

        for (int arg; !st->error && (arg = va_arg(args, int)) >= 0; )
        {
                ril_cmd_char(st, ',');
                ril_cmd_int (st, arg);
        }

        ril_cmd_eol  (st);
        ril_rsp_echo (st, timeout);
        ril_rsp_final(st);

        va_end(args);
        return st->error;
}
