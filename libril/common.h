/// @file common.h

#include <stdlib.h>

#ifndef RIL_COMMON_H
#define RIL_COMMON_H

/// pointer to function for writing a single byte to output stream
/**
 * Function argument `obj` is anything useful to the writing function
 * or NULL, if not used. It is not used by this library.
 *
 * It shall return the number of written bytes.
 */
typedef int (*ril_cmd_write_t) (void* obj, char c);

/// pointer to function for reading a single byte from input stream
/**
 * Function argument `obj` is anything useful to the reading function
 * or NULL, if not used. It is not used by this library.
 *
 * It shall return the number of read bytes.
 */
typedef char (*ril_rsp_read_t) (void* obj);

/// state
/**
 * @note
 * `cm_err` holds either:
 * - the mobile termination error result code of `+CME ERROR` and
 *   is valid only when `error` reports an `RIL_ERR_RSP_CME`, or
 * - the mobile service error result code of `+CMS ERROR` and
 *   is valid only when `error` reports an `RIL_ERR_RSP_CMS`.
 */
typedef struct ril_state {
        // common
        int error;                      ///< detected error (@ref RIL_ERR)
        int cm_err;                     ///< CME/CMS error code

        // write
        ril_cmd_write_t write;          ///< custom write function
        void* write_obj;                ///< custom object for write function

        // read
        ril_rsp_read_t read;            ///< custom read function
        void* read_obj;                 ///< custom object for read function
        size_t count;                   ///< number of valid bytes in the buffer
        size_t index;                   ///< number of matching bytes in the buffer
        size_t buf_max;                 ///< internal buffer size
        char buf[];                     ///< internal buffer
} ril_state_t;

/// error codes
enum RIL_ERR
{
        RIL_ERR_NONE,
        RIL_ERR_BAD_PARAMETER,

        // errors related to command transmission
        RIL_ERR_CMD_WRITE,

        // errors related to response reception
        RIL_ERR_RSP_CHAR,
        RIL_ERR_RSP_CHARP,
        RIL_ERR_RSP_ECHO,
        RIL_ERR_RSP_EOL,
        RIL_ERR_RSP_FINAL_ABORT,
        RIL_ERR_RSP_FINAL_CME,
        RIL_ERR_RSP_FINAL_CMS,
        RIL_ERR_RSP_FINAL_ERROR,
        RIL_ERR_RSP_FINAL_UNKNOWN,
        RIL_ERR_RSP_HEX,
        RIL_ERR_RSP_INT,
        RIL_ERR_RSP_LINE,
        RIL_ERR_RSP_LINE_DUMP,
        RIL_ERR_RSP_QUERY,
        RIL_ERR_RSP_STR,
        RIL_ERR_RSP_STRA_NONE,
        RIL_ERR_RSP_STRA_OVERFLOW,
        RIL_ERR_RSP_STRA_UNDERFLOW,
        RIL_ERR_RSP_STRPN,
        RIL_ERR_RSP_STRQQE_BEGIN,
        RIL_ERR_RSP_STRQQE_END,
        RIL_ERR_RSP_STRQQE_NO_QUOTES,
        RIL_ERR_RSP_UINT,
};

/// convert a string of digits into a an integer
void _buf2int (const char* buf, size_t n, unsigned int base, int* val);

#endif // RIL_COMMON_H

// vim: syntax=c ts=8 sw=8
