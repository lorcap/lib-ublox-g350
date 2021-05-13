/// @file common.h

#include <stdbool.h>
#include "common.h"

void
_buf2int (const char* buf,
          size_t n,
          unsigned int base,
          int* val)
{
        const bool neg = (*buf == '-');

        if (neg || *buf == '+')
        {
                ++buf;
                --n;
        }

        int v = 0;
        for (; n--; ++buf)
        {
                char c = *buf;

                if ('0' <= c && c <= '9')
                        c -= '0';
                else
                if (base == 10)
                        break;
                else
                if ('A' <= c && c <= 'F')
                        c -= 'A' - 10;
                else
                if ('a' <= c && c <= 'f')
                        c -= 'a' - 10;
                else
                        break;

                v = v*base + c;
        }

        if (neg)
                v *= -1;

        if (val)
                *val = v;
}

// vim: syntax=c ts=8 sw=8
