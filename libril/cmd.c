/**
 * @file   cmd.c
 * @author Lorenzo Cappelletti
 * @date   2021-03-21
 */

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include "cmd.h"

/* *** Main ************************************************************** */

void
ril_cmd_init (ril_state_t* st,
              ril_cmd_write_t write,
              void* write_obj)
{
        st->error = 0;
        st->write = write;
        st->write_obj = write_obj;
}

void
ril_cmd_deinit (ril_state_t* st)
{
}

int
ril_cmd_printf (ril_state_t* st,
                const char* fmt,
                ...)
{
        if (st->error)
                return 0;

        va_list args;
        va_start(args, fmt);
        size_t count = 0;

        for (size_t n = 0; *fmt; ++fmt)
        {
                if (*fmt == '%') {
                        ++fmt;
                        bool dquote = false;
                        bool plus   = false;
                        char squote = '\0';
                        char pipe   = '\0';
                        char langle = '\0';
                        char rangle = '\0';
                        int  star   = 0;

                        for (;; ++fmt)
                        {
                                if (*fmt == '"') {
                                        dquote = true;
                                } else
                                if (*fmt == '+') {
                                        plus = true;
                                } else
                                if (*fmt == '\'') {
                                        squote = va_arg(args, int);
                                } else
                                if (*fmt == '|') {
                                        pipe = va_arg(args, int);
                                } else
                                if (*fmt == '<') {
                                        langle = va_arg(args, int);
                                } else
                                if (*fmt == '>') {
                                        rangle = va_arg(args, int);
                                } else
                                if (*fmt == '*') {
                                        star = va_arg(args, int);
                                } else
                                if ('0' <= *fmt && *fmt <= '9') {
                                        _buf2int(fmt, -1, 10, &star);
                                } else {
                                        break;
                                }
                        }

                        switch (*fmt)
                        {
                        case '%':
                                n = ril_cmd_char(st, '%');
                                break;

                        case '$':
                                n = ril_cmd_eol(st);
                                break;

                        case 'A':
                                n = ril_cmd_atc(st, va_arg(args, const char*));
                                break;

                        case 'c':
                                if (star) {
                                        n = ril_cmd_charn(st, va_arg(args, const char*), star);
                                } else {
                                        n = ril_cmd_char(st, va_arg(args, int));
                                }
                                break;

                        case 'd': {
                                int i = va_arg(args, int);

                                if (star && plus) {
                                        n = ril_cmd_intpw(st, i, star);
                                } else
                                if (star) {
                                        n = ril_cmd_intw(st, i, star);
                                } else
                                if (plus) {
                                        n = ril_cmd_intp(st, i);
                                } else {
                                        n = ril_cmd_int(st, i);
                                }
                                } break;

                        case 's': {
                                const char* str = va_arg(args, const char*);

                                if (star) {
                                        n = ril_cmd_strn(st, str, star);
                                } else
                                if (dquote) {
                                        n = ril_cmd_strqe(st, str, '"', '\\');
                                } else
                                if (squote) {
                                        n = pipe
                                          ? ril_cmd_strqe(st, str, squote, pipe)
                                          : ril_cmd_strq (st, str, squote);
                                } else
                                if (langle && rangle) {
                                        n = pipe
                                          ? ril_cmd_strqqe(st, str, langle, rangle, pipe)
                                          : ril_cmd_strqq (st, str, langle, rangle);
                                } else {
                                        n = ril_cmd_str(st, str);
                                }
                                } break;

                        case 'u':
                                n = ril_cmd_uint(st, va_arg(args, unsigned));
                                break;

                        case 'x': {
                                unsigned x = va_arg(args, unsigned);

                                if (star)
                                {
                                        n = ril_cmd_hexw(st, x, star);
                                } else {
                                        n = ril_cmd_hex(st, x);
                                }
                                } break;

                        default:
                                n = 0;
                        }
                } else {
                        n = ril_cmd_char(st, *fmt);
                }

                if (n == 0)
                        return 0;
                count += n;
        }

        va_end(args);
        return count;
}

int
ril_cmd_query (ril_state_t* st,
               const char* cmd)
{
        if (st->error)
                return 0;

        size_t n, count = 0;

        return
        (n = ril_cmd_atco(st, cmd, '?')) && (count += n) &&
        (n = ril_cmd_eol (st          )) && (count += n)
        ? count : 0;
}

int
ril_cmd_set (ril_state_t* st,
             const char* cmd)
{
        return ril_cmd_atco(st, cmd, '=');
}


/* *** Chars ************************************************************* */

int
ril_cmd_char (ril_state_t* st,
              char c)
{
        int n = st->write(st->write_obj, c);

        if (!n)
                st->error = -RIL_ERR_CMD_WRITE;

        return n;
}

int
ril_cmd_charn (ril_state_t* st,
               const char* buffer,
               size_t n)
{
        if (st->error)
                return 0;

        const char *const end = buffer + n;

        for (; buffer < end; ++buffer)
        {
                if (!ril_cmd_char(st, *buffer))
                        return 0;
        }

        return n;
}

int
ril_cmd_eol (ril_state_t* st)
{
        return ril_cmd_charn(st, "\r\n", 2);
}


/* *** Strings *********************************************************** */

int
ril_cmd_strn (ril_state_t* st,
              const char* str,
              size_t n)
{
        if (st->error)
                return 0;

        const char* s = str;

        for (size_t i = 0; i < n && *s; ++i)
        {
                if (!ril_cmd_char(st, *s++))
                        return 0;
        }

        return s - str;
}

int
ril_cmd_str (ril_state_t* st, const char* str)
{
        return ril_cmd_strn(st, str, -1);
}

int
ril_cmd_strqqe (ril_state_t* st,
                const char* str,
                char quote_begin,
                char quote_end,
                char escape)
{
        if (st->error)
                return 0;

        int count = 0;

        if (!ril_cmd_char(st, quote_begin))
                return 0;
        ++count;

        for (; *str; ++str)
        {
                if (escape
                &&  (*str == escape || *str == quote_begin || *str == quote_end))
                {
                        if (!ril_cmd_char(st, escape))
                                return 0;
                        ++count;
                }
                if (!ril_cmd_char(st, *str))
                        return 0;
                ++count;
        }

        if (!ril_cmd_char(st, quote_end))
                return 0;
        ++count;

        return count;
}

int
ril_cmd_strqe (ril_state_t* st,
               const char* str,
               char quote,
               char escape)
{
        return ril_cmd_strqqe(st, str, quote, quote, escape);
}

int
ril_cmd_strqq (ril_state_t* st,
               const char* str,
               char quote_begin,
               char quote_end)
{
        return ril_cmd_strqqe(st, str, quote_begin, quote_end, '\0');
}

int
ril_cmd_strq (ril_state_t* st,
              const char* str,
              char quote)
{
        return ril_cmd_strqe(st, str, quote, '\0');
}

int
ril_cmd_at (ril_state_t* st)
{
        return ril_cmd_str(st, "AT");
}

int
ril_cmd_atc (ril_state_t* st,
             const char* cmd)
{
        st->error = 0;

        size_t n, count = 0;

        return
        (n = ril_cmd_at  (st     )) && (count += n) &&
        (n = ril_cmd_str (st, cmd)) && (count += n)
        ? count : 0;
}

int
ril_cmd_atco (ril_state_t* st,
              const char* cmd,
              char op)
{
        if (st->error)
                return 0;

        size_t n, count = 0;

        return
        (n = ril_cmd_atc (st, cmd)) && (count += n) &&
        (n = ril_cmd_char(st, op )) && (count += n)
        ? count : 0;
}


/* *** Numbers *********************************************************** */

static int
_int2buf (ril_state_t* st,
          const char* format,
          ...)
{
        if (st->error)
                return 0;

        va_list va;
        va_start(va, format);
        char str[sizeof(int) <= 1 ? sizeof(                "-128") :
                 sizeof(int) <= 2 ? sizeof(              "-32768") :
                 sizeof(int) <= 4 ? sizeof(         "-2147483648") :
                 sizeof(int) <= 8 ? sizeof("-9223372036854775808") :
                 -1];

        size_t count = vsnprintf(str, sizeof(str), format, va);
        va_end(va);
        return ril_cmd_charn(st, str, count);
}

int
ril_cmd_int (ril_state_t* st,
             int i)
{
        return _int2buf(st, "%i", i);
}

int
ril_cmd_intp (ril_state_t* st,
              int i)
{
        return _int2buf(st, "%+i", i);
}

int
ril_cmd_intw (ril_state_t* st,
              int i,
              int width)
{
        return _int2buf(st, "%0*i", width, i);
}

int
ril_cmd_intpw (ril_state_t* st,
               int i,
               int width)
{
        return _int2buf(st, "%+0*i", width, i);
}

int
ril_cmd_uint (ril_state_t* st,
              unsigned u)
{
        return _int2buf(st, "%u", u);
}

int
ril_cmd_hex (ril_state_t* st,
             unsigned x)
{
        return _int2buf(st, "%X", x);
}

int
ril_cmd_hexw (ril_state_t* st,
              unsigned x,
              int width)
{
        return _int2buf(st, "%0*X", width, x);
}

// vim: syntax=c ts=8 sw=8
