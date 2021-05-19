/// @file atrsp.c

#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "rsp.h"

static void
_clear (ril_state_t* st)
{
        st->error = 0;
        st->count = 0;
        st->index = 0;
}

static char
_read (ril_state_t* st)
{
        if (st->count == st->buf_max)
        {
                st->error = -RIL_ERR_RSP_READ_OVERFLOW;
                return '\0';
        }

        const int c = st->read(st->read_obj, st->timeout);

        if (0 <= c && c <= 255)
        {
                st->buf[st->count] = c;
                ++st->count;
                st->timeout = 0;
                return c;
        }

        if (c == -2)
                st->error = -RIL_ERR_RSP_READ_TIMEOUT;

        return '\0';
}

static char
_get (ril_state_t* st)
{
        if (st->index == st->buf_max)
        {
                st->error = -RIL_ERR_RSP_READ_UNDERFLOW;
                return '\0';
        }

        char c = (st->index == st->count)
               ? _read(st)
               : st->buf[st->index];
        if (c)
                ++st->index;
        return c;
}


/* *** Main ************************************************************** */

void
ril_rsp_init (ril_state_t* st,
              ril_rsp_read_t read,
              void* read_obj,
              size_t buffer_max)
{
        st->read = read;
        st->read_obj = read_obj;
        st->buf_max = buffer_max;
        _clear(st);
        ril_rsp_flush(st);
}

void
ril_rsp_deinit (ril_state_t* st)
{
        ril_rsp_flush(st);
}

int
ril_rsp_echo (ril_state_t* st,
              unsigned int timeout)
{
        if (st->error)
                return 0;
        _clear(st);

        int ret;
        st->timeout = RIL_RT_10ms;
        if (ril_rsp_match_eol(st))
                ret = ril_rsp_res_ok(st);
        else
        if (ril_rsp_match_str(st, "AT"))
                ret = ril_rsp_line_dump(st);
        else
                return ril_rsp_res_abort(st, -RIL_ERR_RSP_ECHO);

        st->timeout = timeout;
        return ret;
}

int
ril_rsp_final (ril_state_t* st)
{
        if (st->error)
                return 0;

        if (ril_rsp_match_line(st, "OK"))
                ;
        else
        if (ril_rsp_match_line(st, "ERROR"))
                st->error = -RIL_ERR_RSP_FINAL_ERROR;
        else
        if (ril_rsp_match_line(st, "ABORT"))
                st->error = -RIL_ERR_RSP_FINAL_ABORT;
        else
        if (ril_rsp_match_line_query(st, "+CME ERROR", &st->cm_err))
                st->error = -RIL_ERR_RSP_FINAL_CME;
        else
        if (ril_rsp_match_line_query(st, "+CMS ERROR", &st->cm_err))
                st->error = -RIL_ERR_RSP_FINAL_CMS;
        else
                return ril_rsp_res_abort(st, -RIL_ERR_RSP_FINAL_UNKNOWN);

        return ril_rsp_res_ok(st);
}

int
ril_rsp_scanf (ril_state_t* st,
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

                        const char* slash = NULL;
                        bool star   = false;
                        bool dquote = false;
                        char squote = '\0';
                        char pipe   = '\0';
                        char langle = '\0';
                        char rangle = '\0';
                        int  hash   = 0;

                        for (;; ++fmt)
                        {
                                if (*fmt == '/') {
                                        slash = va_arg(args, const char*);
                                } else
                                if (*fmt == '#') {
                                        hash = va_arg(args, int);
                                } else
                                if (*fmt == '*') {
                                        star = true;
                                } else
                                if (*fmt == '"') {
                                        dquote = true;
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
                                if ('0' <= *fmt && *fmt <= '9') {
                                        _buf2int(fmt, -1, 10, &hash);
                                } else {
                                        break;
                                }
                        }

                        switch (*fmt)
                        {
                        case '%':
                                n = ril_rsp_char(st, '%');
                                break;

                        case '$':
                                  n = ril_rsp_eol(st);
                                  break;

                        case 'c':
                                if (slash) {
                                        n = ril_rsp_charp(st, slash);
                                } else
                                if (hash) {
                                        n = ril_rsp_charn(st, hash, star ? NULL : va_arg(args, char*));
                                } else {
                                        n = ril_rsp_char(st, va_arg(args, int));
                                }
                                break;

                        case 'd':
                                n = ril_rsp_int(st, star ? NULL : va_arg(args, int*));
                                break;

                        case 's': {
                                char* str = star ? NULL : va_arg(args, char*);

                                if (slash && hash) {
                                        n = ril_rsp_strpn(st, slash, hash, str);
                                } else
                                if (slash) {
                                        n = ril_rsp_strp(st, slash, str);
                                } else
                                if (dquote) {
                                        n = ril_rsp_strqe(st, '"', '\\', str);
                                } else
                                if (squote) {
                                        n = pipe
                                          ? ril_rsp_strqe(st, squote, pipe, str)
                                          : ril_rsp_strq (st, squote, str);
                                } else
                                if (langle && rangle) {
                                        n = pipe
                                          ? ril_rsp_strqqe(st, langle, rangle, pipe, str)
                                          : ril_rsp_strqq (st, langle, rangle, str);
                                } else {
                                        n = ril_rsp_str(st, str);
                                }
                                } break;

                        case 'u':
                                n = ril_rsp_uint(st, star ? NULL : va_arg(args, unsigned int*));
                                break;

                        case 'x':
                                n = ril_rsp_hex(st, star ? NULL : va_arg(args, unsigned int*));
                                break;

                        default:
                                n = 0;
                        }
                } else {
                        n = ril_rsp_char(st, *fmt);
                }

                if (n == 0)
                        return 0;
                count += n;
        }

        va_end(args);
        return count;
}

int
ril_rsp_query (ril_state_t* st,
               const char* str,
               int* val)
{
        if (st->error)
                return 0;

        return ril_rsp_res(st, ril_rsp_match_query(st, str, val), -RIL_ERR_RSP_QUERY);
}

int
ril_rsp_stra (ril_state_t* st,
              const char* pattern,
              const void* array,
              size_t array_size,
              size_t element_size,
              size_t offset)
{
        _clear(st);

        int error;
        const size_t count = ril_rsp_match_strp(st, pattern);

        if (count == 0)
        {
                error = -RIL_ERR_RSP_STRA_NONE;
                goto abort;
        }

        size_t min = 0;
        size_t max = array_size/element_size;
        while (true)
        {
                size_t target = (min + max)/2;

                const char* arr = (const char*)array + target*element_size + offset;
                for (size_t i = 0;;)
                {
                        const char a = *arr      ? *arr++       : '\0';
                        const char b = i < count ? st->buf[i++] : '\0';

                        if (b < a)
                        {
                                if (target == min)
                                {
                                        error = -RIL_ERR_RSP_STRA_UNDERFLOW;
                                        goto abort;
                                }
                                max = target;
                                break;
                        } else
                        if (b > a) {
                                if (target == max)
                                {
                                        error = -RIL_ERR_RSP_STRA_OVERFLOW;
                                        goto abort;
                                }
                                min = target;
                                break;
                        } else
                        if (b == '\0')
                        {
                                ril_rsp_res_ok(st);
                                return target;
                        }
                }
        }

abort:
        ril_rsp_res_abort(st, error);
        return -1;
}

int
ril_rsp_flush (ril_state_t* st)
{
        size_t count = st->count;
        _clear(st);
        for (; st->read(st->read_obj, RIL_RT_1ms) >= 0; ++count);
        return count;
}


/* *** General *********************************************************** */

int
ril_rsp_match_query (ril_state_t* st,
                     const char* str,
                     int* val)
{
        int count, n;

        if (!(n = ril_rsp_match_str(st, str)))
                return 0;

        count = n;
        if (!(n = ril_rsp_match_str(st, ": ")))
                return 0;

        count += n;
        if (!(n = ril_rsp_match_int(st)))
                return 0;

        _buf2int(&st->buf[count], -1, 10, val);
        count += n;
        return count;
}

int
ril_rsp_res_abort (ril_state_t* st,
                   int error)
{
        st->error = error;
        st->index = 0;
        return 0;
}

int
ril_rsp_res_ok (ril_state_t* st)
{
        char* src = &st->buf[st->index];
        char* end = &st->buf[st->count];
        char* dst = &st->buf[0];

        for (; src < end; ++src, ++dst)
                *dst = *src;

        st->count -= st->index;
        size_t i = st->index;
        st->index = 0;
        return i;
}

int
ril_rsp_res_ok_str (ril_state_t* st,
                    char* str)
{
        if (st->error)
                return 0;

        if (str)
                memcpy(str, st->buf, st->index);
        return ril_rsp_res_ok(st);
}

int
ril_rsp_res (ril_state_t* st,
             bool cond,
             int error)
{
        return cond ? ril_rsp_res_ok(st) : ril_rsp_res_abort(st, error);
}


/* *** Chars ************************************************************* */

int
ril_rsp_res_str (ril_state_t* st,
                 bool cond,
                 char* str,
                 int error)
{
        return cond ? ril_rsp_res_ok_str(st, str) : ril_rsp_res_abort(st, error);
}

int
ril_rsp_match_char (ril_state_t* st,
                    char c)
{
        if (_get(st) == c)
                return 1;

        if (!st->error)
                --st->index;
        return 0;
}

int
ril_rsp_match_charp (ril_state_t* st,
                     const char* pattern)
{
        const char c = _get(st);
        if (!c)
                return 0;

        int neg = 0;    // negate match
        if (pattern[0] == '^' && pattern[1] != '\0')
        {
                neg = 1;
                ++pattern;
        }

        int ret = 0;
        for (size_t i = 0; pattern[i]; ++i)
        {
                if (// match range 'a-b'
                    (pattern[i] == '-'
                     && i > 0 && pattern[i+1] != '\0'
                     && pattern[i+1] > c && c > pattern[i-1])
                ||  // match single char
                    (c == pattern[i]))
                {
                        ret = 1;
                        break;
                }
        }

        ret ^= neg;
        if (!ret)
                st->index--;
        return ret;
}

int
ril_rsp_seek_char (ril_state_t* st,
                   char c)
{
        for (char k; (k = _get(st)) && (k != c); );

        return --st->index;
}

int
ril_rsp_char (ril_state_t* st,
              char c)
{
        if (st->error)
                return 0;

        return ril_rsp_res(st, ril_rsp_match_char(st, c), -RIL_ERR_RSP_CHAR);
}

int
ril_rsp_charn (ril_state_t* st,
               size_t n,
               char* buffer)
{
        if (st->error)
                return 0;

        size_t i = 0;

        for (; (i < n) && (st->index < st->count); i++, st->index++)
                if (buffer)
                        buffer[i] = st->buf[st->index];
        ril_rsp_res_ok(st);

        int b;
        for (; (i < n) && (b = st->read(st->read_obj, RIL_RT_1ms)) >= 0; i++)
                if (buffer)
                        buffer[i] = b;

        return i;
}

int
ril_rsp_charp (ril_state_t* st,
               const char* pattern)
{
        if (st->error)
                return 0;

        return ril_rsp_res(st, ril_rsp_match_charp(st, pattern), -RIL_ERR_RSP_CHARP);
}


/* *** Strings *********************************************************** */

int
ril_rsp_match_str (ril_state_t* st,
                   const char* str)
{
        size_t index = st->index;

        for (; *str; ++str)
        {
                if (!ril_rsp_match_char(st, *str))
                {
                        st->index = index;
                        break;
                }
        }

        return st->index - index;
}

int
ril_rsp_match_strpn (ril_state_t* st,
                     const char* pattern,
                     size_t n)
{
        int count = 0;

        for (; count < n; ++count)
        {
                if (!ril_rsp_match_charp(st, pattern))
                        break;
        }

        return count;
}

int
ril_rsp_match_strp (ril_state_t* st,
                    const char* pattern)
{
        return ril_rsp_match_strpn(st, pattern, -1);
}

int
ril_rsp_match_eol (ril_state_t* st)
{
        return ril_rsp_match_str(st, "\r\n");
}

int
ril_rsp_str (ril_state_t* st,
             const char* str)
{
        if (st->error)
                return 0;

        return ril_rsp_res(st, ril_rsp_match_str(st, str), -RIL_ERR_RSP_STR);
}

int
ril_rsp_strpn (ril_state_t* st,
               const char* pattern,
               size_t n,
               char* str)
{
        if (st->error)
                return 0;

        return ril_rsp_res_str(st, ril_rsp_match_strpn(st, pattern, n), str, -RIL_ERR_RSP_STRPN);
}

int
ril_rsp_strp (ril_state_t* st,
              const char* pattern,
              char* str)
{
        return ril_rsp_strpn(st, pattern, -1, str);
}

int
ril_rsp_strqqe (ril_state_t* st,
                char quote_begin,
                char quote_end,
                char escape,
                char* str)
{
        if (st->error)
                return 0;

        if (!quote_begin || !quote_end)
                return ril_rsp_res_abort(st, -RIL_ERR_RSP_STRQQE_NO_QUOTES);

        if (!ril_rsp_match_char(st, quote_begin))
                return ril_rsp_res_abort(st, -RIL_ERR_RSP_STRQQE_BEGIN);

        char pattern[] = {'^', quote_end, '\0'};
        while (ril_rsp_match_strp(st, pattern))
        {
                if (st->buf[st->index - 1] == escape)
                        _get(st);
                else
                        break;
        }

        if (!ril_rsp_match_char(st, quote_end))
                return ril_rsp_res_abort(st, -RIL_ERR_RSP_STRQQE_END);

        memcpy(str, st->buf + 1, st->index - 2);
        return ril_rsp_res_ok(st);
}

int
ril_rsp_strqe (ril_state_t* st,
               char quote,
               char escape,
               char* str)
{
        return ril_rsp_strqqe(st, quote, quote, escape, str);
}

int
ril_rsp_strqq (ril_state_t* st,
               char quote_begin,
               char quote_end,
               char* str)
{
        return ril_rsp_strqqe(st, quote_begin, quote_end, '\0', str);
}

int
ril_rsp_strq (ril_state_t* st,
              char quote,
              char* str)
{
        return ril_rsp_strqq(st, quote, quote, str);
}

int
ril_rsp_eol (ril_state_t* st)
{
        if (st->error)
                return 0;

        return ril_rsp_res(st, ril_rsp_match_eol(st), -RIL_ERR_RSP_EOL);
}


/* *** Lines ************************************************************* */

int
ril_rsp_match_line (ril_state_t* st,
                    const char* str)
{
        int count, n;

        if (!(n = ril_rsp_match_str(st, str)))
                return 0;

        count = n;
        if (!(n = ril_rsp_match_eol(st)))
                return 0;

        count += n;
        return count;
}

int
ril_rsp_match_line_query (ril_state_t* st,
                          const char* str,
                          int* val)
{
        int count, n, v;

        if (!(n = ril_rsp_match_query(st, str, &v)))
                return 0;

        count = n;
        if (!(n = ril_rsp_match_eol(st)))
                return 0;

        *val = v;
        count += n;
        return count;
}

int
ril_rsp_line (ril_state_t* st,
              const char* str)
{
        if (st->error)
                return 0;

        return ril_rsp_res(st, ril_rsp_match_line(st, str), -RIL_ERR_RSP_LINE);
}

int
ril_rsp_line_abort (ril_state_t* st)
{
        return ril_rsp_line(st, "ABORT");
}

int
ril_rsp_line_error (ril_state_t* st)
{
        return ril_rsp_line(st, "ERROR");
}

int
ril_rsp_line_ok (ril_state_t* st)
{
        return ril_rsp_line(st, "OK");
}

int
ril_rsp_line_dump (ril_state_t* st)
{
        if (st->error)
                return 0;

        for (size_t i = -1; i != st->index; )
        {
                i = st->index;
                ril_rsp_seek_char(st, '\r');
                if (ril_rsp_match_eol(st))
                        return ril_rsp_res_ok(st);
        }

        return ril_rsp_res_abort(st, -RIL_ERR_RSP_LINE_DUMP);
}


/* *** Numbers *********************************************************** */

int
ril_rsp_match_int (ril_state_t* st)
{
        const size_t index = st->index;

        ril_rsp_match_charp(st, "+-");
        if (!ril_rsp_match_strp(st, "0-9"))
                st->index = index;

        return st->index - index;
}

int
ril_rsp_match_uint (ril_state_t* st)
{
        return ril_rsp_match_strp(st, "0-9");
}

int
ril_rsp_match_hex (ril_state_t* st)
{
        return ril_rsp_match_strp(st, "A-Fa-f0-9");
}

int
ril_rsp_int (ril_state_t* st,
             int* val)
{
        if (st->error)
                return 0;

        if (!ril_rsp_match_int(st))
                return ril_rsp_res_abort(st, -RIL_ERR_RSP_INT);

        _buf2int(st->buf, st->index, 10, val);
        return ril_rsp_res_ok(st);
}

int
ril_rsp_uint (ril_state_t* st,
              unsigned int* val)
{
        if (st->error)
                return 0;

        if (!ril_rsp_match_uint(st))
                return ril_rsp_res_abort(st, -RIL_ERR_RSP_UINT);

        _buf2int(st->buf, st->index, 10, (int*) val);
        return ril_rsp_res_ok(st);
}

int
ril_rsp_hex (ril_state_t* st,
             unsigned int* val)
{
        if (st->error)
                return 0;

        if (!ril_rsp_match_hex(st))
                return ril_rsp_res_abort(st, -RIL_ERR_RSP_HEX);

        _buf2int(st->buf, st->index, 16, (int*) val);
        return ril_rsp_res_ok(st);
}

// vim: syntax=c ts=8 sw=8
