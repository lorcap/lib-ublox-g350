/** @file atcmd.h
 *
 * @page ril_cmd AT Command Generation for Radio Interface Library
 *
 * @author  Lorenzo Cappelletti
 * @date    2021-03-21
 *
 *
 * This library provides a set of functions for generating and sending AT
 * commands. Source code gains readability because it is not cluttered with
 * `snprintf()` and `strtol()` functions.
 */

#include "common.h"

#ifndef RIL_CMD_H
#define RIL_CMD_H


/* ----------------------------------------------------------------------- */
/**
 * @defgroup ril_cmd_main Main functions
 *
 * These are the main functions provided by this library. They relay on the
 * other functions which can be used for special cases.
 */
///@{

/// initialize the AT command generator
/**
 * @param st
 *      internal state
 * @param write
 *      pointer to custom write function
 * @param write_obj
 *      object for write function
 * @return
 *      initialized internal state
 */
void ril_cmd_init (ril_state_t* st, ril_cmd_write_t write, void* write_obj);

/// destroy AT parser's state
/**
 * @param st
 *      internal state
 */
void ril_cmd_deinit (ril_state_t* st);

/// write a command
/**
 * Commands to the radio device can be written with this printf-like function,
 * by loading the data from the given locations and converting them to
 * character string equivalents.
 *
 * @param st
 *      internal state
 * @param fmt
 *      pointer to a null-terminated string specifying how to write the output
 * @param ...
 *      arguments specifying data to print
 * @return
 *      number of written bytes
 *
 * The `format` string consists of:
 * - introductory % character.
 * - one or more flags that modify the behavior of the conversion:
 *   * (optional) plus sign `+`. The sign of signed conversions is always
 *     prepended to the result of the conversion (by default the result is
 *     preceded by minus only when it is negative).
 *   * (optional) integer number (greater than zero) that specifies the
 *     maximum number of character to write. Alternatively, character `*`
 *     requires a information field of type `int`.
 *   * (optional) quote-escape character `"`. If this option is present,
 *     the output string will be sorrounded by `"` which can be escaped
 *     with `\\`.
 *   * (optional) quote character `\``. If this option is present, a
 *     character field is required. It represents the opening and
 *     closing quote character for the output string.
 *   * (optional) escape character `|`. If this option is present, a
 *     character field is require. It represents the escape character.
 *   * (optional) opening-quote character `<`. If this option is present,
 *     a character field is required. It represents the opening quote for
 *     the output string.
 *   * (optional) closing-quote character `>`. If this option is present,
 *     a character field is required. It represents the closing quote for
 *     the output string.
 *
 * The following format specifiers are available:
 * - `%`: writes literal %.
 * - `A`: writes `AT` followed by the command passed as a null-terminated
 *        string field.
 * - `c`: writes:
 *      * a character (see @ref ril_cmd_char()),
 *      * with `*`, a sequence of fixed length (see @ref ril_cmd_charn()).
 * - `s`: writes:
 *      * a literal string (requires a null-terminated string field
 *        (see @ref ril_cmd_str()),
 *      * a quote delimited string with `"`, `\`` (see @ref ril_cmd_strq()),
 *        `<` or `>` (see @ref ril_cmd_strqq()), optionally escaped with
 *        `|` (see @ref ril_cmd_strqe() and @ref ril_cmd_strqqe()).
 * - `d`: writes a decimal number (see @ref ril_cmd_int()) optionally
 *        prefixed by a plus sign when `+` is specified
 *        (see @ref ril_cmd_intp()) and padded with zeros when field
 *        width `*` is specified (see @ref ril_cmd_intw() and
 *        @ref ril_cmd_intpw()).
 * - `u`: writes an unsigned decimal integer (see @ref ril_cmd_uint())
 *        optionally padded with zeros when field width `*` is specified
 *        (see @ref ril_cmd_uintw()).
 * - `x`: writes an unsigned hexadecimal integer (see @ref ril_cmd_hex())
 *        optionally padded with zeros when field width `*` is specified
 *        (see @ref ril_cmd_hexw()).
 */
int ril_cmd_printf (ril_state_t* st, const char* fmt, ...);

/// write a query command in the format `AT<CMD>?\r\n`
/**
 * @param st
 *      internal state
 * @param cmd
 *      command string
 * @return
 *      number of written bytes if successful, 0 otherwise
 */
int ril_cmd_query (ril_state_t* st, const char* cmd);

/// write a setting command in the format `AT<CMD>=`
/**
 * @param st
 *      internal state
 * @param cmd
 *      command string
 * @return
 *      number of written bytes if successful, 0 otherwise
 */
int ril_cmd_set (ril_state_t* st, const char* cmd);

///@}


/* ----------------------------------------------------------------------- */
/**
 * @defgroup ril_cmd_char Characters
 *
 * Functions for matching single characters from input stream.
 */
///@{

/// write a single byte
/**
 * @param st
 *      internal state
 * @return
 *      1 if successful, 0 otherwise
 */
int ril_cmd_char (ril_state_t* st, char c);

/// write `n` bytes
/**
 * @param st
 *      internal state
 * @param buffer
 *      buffer with bytes to write
 * @param n
 *      number of bytes to write
 * @return
 *      number of written bytes or 0 if an error occurred
 */
int ril_cmd_charn (ril_state_t* st, const char* buffer, size_t n);

/// write the end-of-line `\r\n`
/**
 * @param st
 *      internal state
 * @return
 *      number of written bytes or 0 if an error occurred
 */
int ril_cmd_eol (ril_state_t* st);

///@}


/* ----------------------------------------------------------------------- */
/**
 * @defgroup ril_cmd_string Strings
 *
 * Functions for writing strings to output stream.
 */
///@{

/// write at most `n` bytes from a null-terminated string
/**
 * The number of written bytes is less than `n` when `strlen(str) < n`.
 *
 * @param st
 *      internal state
 * @param str
 *      string to write
 * @param n
 *      number of bytes to write
 * @return
 *      number of written bytes or 0 if an error occurred
 */
int ril_cmd_strn (ril_state_t* st, const char* str, size_t n);

/// write a null-terminated string
/**
 * @param st
 *      internal state
 * @param str
 *      string to write
 * @return
 *      number of written bytes or 0 if an error occurred
 */
int ril_cmd_str (ril_state_t* st, const char* str);

/// write a null-terminated, escaped string surrounded with quotes
/**
 * Any `quote` or `escape` character within the string are escaped
 * with `escape`.
 *
 * @param st
 *      internal state
 * @param str
 *      string to write
 * @param quote_begin
 *      opening quote
 * @param quote_end
 *      closing quote
 * @param escape
 *      escape character
 * @return
 *      number of written bytes or 0 if an error occurred
 */
int ril_cmd_strqqe (ril_state_t* st, const char* str, char quote_begin, char quote_end, char escape);

/// write a null-terminated, escaped string surrounded with quotes
/**
 * Any `quote` or `escape` character within the string are escaped
 * with `escape`.
 *
 * @param st
 *      internal state
 * @param str
 *      string to write
 * @param quote
 *      opening and closing quote
 * @param escape
 *      escape character
 * @return
 *      number of written bytes or 0 if an error occurred
 */
int ril_cmd_strqe (ril_state_t* st, const char* str, char quote, char escape);

/// write a null-terminated string surrounded with quotes
/**
 * @param st
 *      internal state
 * @param str
 *      string to write
 * @param quote_begin
 *      opening quote
 * @param quote_end
 *      closing quote
 * @return
 *      number of written bytes or 0 if an error occurred
 */
int ril_cmd_strqq (ril_state_t* st, const char* str, char quote_begin, char quote_end);

/// write a null-terminated string surrounded with quotes
/**
 * @param st
 *      internal state
 * @param str
 *      string to write
 * @param quote
 *      opening and closing quote
 * @return
 *      number of written bytes or 0 if an error occurred
 */
int ril_cmd_strq (ril_state_t* st, const char* str, char quote);

/// write the command prefix 'AT'
/**
 * @param st
 *      internal state
 * @return
 *      number of written bytes or 0 if an error occurred
 */
int ril_cmd_at (ril_state_t* st);

/// write 'AT' followed by a command
/**
 * @param st
 *      internal state
 * @param cmd
 *      command string to write
 * @return
 *      number of written bytes or 0 if an error occurred
 */
int ril_cmd_atc (ril_state_t* st, const char* cmd);

/// write 'AT' followed by a command and an operator
/**
 * @param st
 *      internal state
 * @param cmd
 *      command string to write
 * @param op
 *      operator character
 * @return
 *      number of written bytes or 0 if an error occurred
 */
int ril_cmd_atco (ril_state_t* st, const char* cmd, char op);

///@}


/* ----------------------------------------------------------------------- */
/**
 * @defgroup ril_cmd_number Numbers
 *
 * Functions for writing numbers to output stream.
 */
///@{

/// write a signed integer
/**
 * @param st
 *      internal state
 * @param i
 *      input value
 * @return
 *      number of written bytes
 */
int ril_cmd_int (ril_state_t* st, int i);

/// write a signed integer with leading '+'
/**
 * @param st
 *      internal state
 * @param i
 *      input value
 * @return
 *      number of written bytes
 */
int ril_cmd_intp (ril_state_t* st, int i);

/// write a signed integer with leading zeros
/**
 * @param st
 *      internal state
 * @param i
 *      input value
 * @return
 *      number of written bytes
 */
int ril_cmd_intw (ril_state_t* st, int i, int width);

/// write a signed integer with leading '+' and zeros
/**
 * @param st
 *      internal state
 * @param i
 *      input value
 * @return
 *      number of written bytes
 */
int ril_cmd_intpw (ril_state_t* st, int i, int width);

/// write an unsigned integer
/**
 * @param st
 *      internal state
 * @param u
 *      input value
 * @return
 *      number of written bytes
 */
int ril_cmd_uint (ril_state_t* st, unsigned u);

/// write an integer in hexadecimal format
/**
 * Output format is that generated by `snprintf("%X", x)`.
 *
 * @param st
 *      internal state
 * @param x
 *      input value
 * @return
 *      number of written bytes
 */
int ril_cmd_hex (ril_state_t* st, unsigned x);

/// write an integer in hexadecimal format of the given length
/**
 * Output format is that generated by `snprintf("%0*X", width, x)`.
 *
 * @param st
 *      internal state
 * @param x
 *      input value
 * @param width
 *      number width
 * @return
 *      number of written bytes
 */
int ril_cmd_hexw (ril_state_t* st, unsigned x, int width);

///@}

#endif // RIL_CMD_H

// vim: syntax=c ts=8 sw=8
