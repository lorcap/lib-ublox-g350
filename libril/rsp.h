/**
 * @file   rsp.h
 * @author Lorenzo Cappelletti
 * @date   2021-03-21
 *
 * @page ril_rsp AT Response Parsing for Radio Interface Library
 *
 * @author  Lorenzo Cappelletti
 * @date    2021-03-21
 *
 *
 * This library provides a set of functions for easing AT response parsing.
 * Instead of reading an entire response line from the AT-capable device,
 * input stream is tentatively matched against expected data, storing
 * intermediate bytes to an internal buffer. When data match, the corresponding
 * value is returned; when data don't match, bytes are left for another attempt.
 *
 *
 * @section ril_rsp_pattern Pattern
 *
 * Several functions require a `pattern` argument which corresponds to a very
 * basic form of regular expression. It may consists in:
 * - 0 or 1 caret `^` which negates the following pattern
 * - 0 or more characters to match
 * - 0 or more character _ranges_, consisting in a lower limit character, a
 *   dash `-`, an upper limit character.
 *
 * Examples of patterns are:
 * - `a`: matches `a`, `aa`, `aaa`, etc.
 * - `aAbB': matches `a`, `A`, `ab`, `aBaA`, etc.
 * - `a-z`: matches any word made of lower case letters
 * - `^:`: matches anything up to a double colon
 */

#include <stdbool.h>
#include "common.h"

#ifndef RIL_RSP_H
#define RIL_RSP_H


/* ----------------------------------------------------------------------- */
/**
 * @defgroup ril_rsp_main Main functions
 *
 * These are the main functions provided by this library. They relay on the
 * other functions which can be used for special cases.
 */
///@{

/// initialize the AT response parser
/**
 * @param st
 *      internal state
 * @param read
 *      pointer to custom read function
 * @param read_obj
 *      object for read function
 * @param buffer_max
 *      size of internal buffer
 * @return
 *      initialized internal state
 */
void ril_rsp_init (ril_state_t* st, ril_rsp_read_t read,
                   void* read_obj, size_t buffer_max);

/// destroy AT parser's state
/**
 * @param st
 *      internal state
 */
void ril_rsp_deinit (ril_state_t* st);

/// dump the echo line
/**
 * An echo line is either:
 * - an empty line
 * - a line starting with string 'AT'.
 *
 * In the latter case, no RIL_ERR_RSP_READ_OVERFLOW is generated
 * (see @ref ril_rsp_line_dump()).
 *
 * @param st
 *      internal state
 * @param timeout
 *      maximum time allowed for the whole response, or `0` to keep
 *      going with previous value
 * @return
 *      number of read bytes
 */
int ril_rsp_echo (ril_state_t* st, unsigned int timeout);

/// read a line containing the final result code for the AT command
/**
 * The final result code consists in a line terminated with EOL and
 * containing:
 * - `OK`
 * - `ERROR`
 * - `ABORT`
 * - `+CME ERROR: <int>`, where `int` is reported by @ref ril_state_t.cme
 * - `+CMS ERROR: <int>`, where `int` is reported by @ref ril_state_t.cms
 *
 * Error code @ref ril_state_t.error is set to the corresponding
 * code `RIL_ERR_RSP_FINAL_*`.
 *
 * @param st
 *      internal state
 */
int ril_rsp_final (ril_state_t* st);

/// read a response
/**
 * Responses from the radio device can be read with this scanf-like function,
 * interpreting it according to a format and storing the results into given
 * locations.
 *
 * @param st
 *      internal state
 * @param fmt
 *      pointer to a null-terminated string specifying how to read the input
 * @param ...
 *      receiving arguments
 * @return
 *      number of read bytes
 *
 * The `format` string consists of:
 * - characters except %: each such character in the format string consumes
 *   exactly one identical character from the input stream, or causes the
 *   function to fail if the next character on the stream does not compare
 *   equal.
 * - conversion specifications: each conversion specification has the
 *   following format:
 * - introductory % character:
 *   * (optional) assignment-suppressing character `*`. If this option is
 *     present, the function does not assign the result of the conversion
 *     to any receiving argument.
 *   * (optional) integer number (greater than zero) that specifies the
 *     maximum number of character to read. Alternatively, character `#`
 *     requires a information field of type `int`.
 *   * (optional) pattern character `/`. If this option is present, a
 *     null-terminated string field is required. The field represents a
 *     pattern as described in @ref ril_rsp_pattern.
 *   * (optional) quote-escape character `"`. If this option is present,
 *     the matching string shall be sorrounded by `"` which can be
 *     escaped with `\\`.
 *   * (optional) quote character `\``. If this option is present, a
 *     character field is required. It represents the opening and
 *     closing quote character for the matching string.
 *   * (optional) escape character `|`. If this option is present, a
 *     character field is require. It represents the escape character.
 *   * (optional) opening-quote character `<`. If this option is present,
 *     a character field is required. It represents the opening quote for
 *     the matching string.
 *   * (optional) closing-quote character `>`. If this option is present,
 *     a character field is required. It represents the closing quote for
 *     the matching string.
 *
 * The following format specifiers are available:
 * - `%`: matches literal %.
 * - `c`: matches:
 *      * a character (see @ref ril_rsp_char()),
 *      * with `#`, a sequence of fixed length (see @ref ril_rsp_charn()),
 *      * with `/`, a pattern-matching sequence of characters
 *        (see @ref ril_rsp_charp()).
 * - `s`: matches:
 *      * a literal string (requires a null-terminated string field
 *        (see @ref ril_rsp_str()),
 *      * with `/`, a pattern-matching string (see @ref ril_rsp_strp()),
 *        which can be limited in number by `#` (see @ref ril_rsp_strpn()),
 *      * a quote delimited string with `"`, `\`` (see @ref ril_rsp_strq()),
 *        `<` or `>` (see @ref ril_rsp_strqq()), optionally escaped with
 *        `|` (see @ref ril_rsp_strqe() and @ref ril_rsp_strqqe()).
 * - `d`: matches a decimal number (see @ref ril_rsp_int()).
 * - `u`: matches an unsigned decimal integer (see @ref ril_rsp_uint()).
 * - `x`: matches an unsigned hexadecimal integer (see @ref ril_rsp_hex()).
 */
int ril_rsp_scanf (ril_state_t* st, const char* fmt, ...);

/// read query response in the format `<str>: <val>`
/**
 * @param st
 *      internal state
 * @param str
 *      fixed string to match
 * @param val
 *      responded integer
 * @return
 *      number of read bytes
 */
int ril_rsp_query (ril_state_t* st, const char* str, int* val);

/// read a string matching a pattern and return the corresponding element in the given array
/**
 * This function is meant for URC (unsolicited received commands). Given a
 * sorted array of URCs entries containing the URC string identifiers, this
 * function performs a binary search of the matching string read from the
 * input stream. If a match is found, the corresponding index is returned;
 * if no URCs are read or no entries match, `-1` is returned.
 *
 * @code
 * typedef struct urc { char* id, int prop, ... } urc_t;
 * urc_t URC[] = {
 *     { "URC_A", 0},
 *     { "URC_B", 1},
 * };
 *
 * int i = ril_rsp_stra(state, "A-Z_", URC, sizeof(URC), sizeof(urc), offsetof(urc, id));
 * @endcode
 *
 * @note
 * The array must be sorted by the string identifier.
 *
 * @param st
 *      internal state
 * @param pattern
 *      matching pattern (see @ref ril_rsp_pattern)
 * @param array
 *      pointer to sorted URC array
 * @param array_size
 *      size of URC array
 * @param element_size
 *      size of array's elements
 * @param offset
 *      offset of URC string identifier
 * @return
 *      the element's index in the array or -1 if none is found
 */
int ril_rsp_stra (ril_state_t* st, const char* pattern, const void* array,
                  size_t array_size, size_t element_size, size_t offset);

/// flush all bytes from internal buffer and input stream
/**
 * @param st
 *      internal state
 * @return
 *      number of flushed bytes
 */
int ril_rsp_flush (ril_state_t* st);

///@}


/* ----------------------------------------------------------------------- */
/**
 * @defgroup ril_rsp_general General
 *
 * Functions for initialization and management of the internal buffer.
 */
///@{

/// match a query response line of type `<str>: <int>`
/**
 * @param st
 *      internal state
 * @param str
 *      fixed string to match
 * @param val
 *      responded integer
 * @return
 *      number of matching bytes
 */
int ril_rsp_match_query (ril_state_t* st, const char* str, int* val);

/// match result: abort
/**
 * @param st
 *      internal state
 * @param error
 *      detected error, if any
 * @return
 *      number of read bytes (always 0)
 */
int ril_rsp_res_abort (ril_state_t* st, int error);

/// match result: ok, dump matching characters
/**
 * @param st
 *      internal state
 * @return
 *      number of dumped bytes
 */
int ril_rsp_res_ok (ril_state_t* st);

/// match result: ok, copy matching characters
/**
 * @param st
 *      internal state
 * @param str
 *      output string
 * @return
 *      number of dumped bytes
 */
int ril_rsp_res_ok_str (ril_state_t* st, char* str);

/// match result depending on condition
/**
 * @param st
 *      internal state
 * @param cond
 *      condition
 * @param error
 *      detected error, if any
 * @return
 *      number of read bytes
 */
int ril_rsp_res (ril_state_t* st, bool cond, int error);

/// match result depending on condition
/**
 * @param st
 *      internal state
 * @param cond
 *      condition
 * @param str
 *      output string
 * @param error
 *      detected error, if any
 * @return
 *      number of read bytes
 */
int ril_rsp_res_str (ril_state_t* st, bool cond, char* str, int error);

///@}


/* ----------------------------------------------------------------------- */
/**
 * @defgroup ril_rsp_char Characters
 *
 * Functions for matching single characters from input stream.
 */
///@{

/// match an expected character from input stream
/**
 * @param st
 *      internal state
 * @param c
 *      character to match
 * @return
 *      number of matching bytes
 */
int ril_rsp_match_char (ril_state_t* st, char c);

/// match a single character matching the given pattern
/**
 * @param st
 *      internal state
 * @param pattern
 *      matching pattern (see @ref ril_rsp_pattern)
 * @return
 *      number of matching bytes
 */
int ril_rsp_match_charp (ril_state_t* st, const char* pattern);

/// read an expected character from input stream
/**
 * This function is faster than @ref ril_rsp_str().
 *
 * @param st
 *      internal state
 * @param c
 *      extected character
 * @return
 *      1 or 0, according to the number of read bytes
 */
int ril_rsp_char (ril_state_t* st, char c);

/// read at most `n` characters from input stream
/**
 * This function copies data directly from input stream to the provided
 * `buffer`. Thus, `n` can exceeds the internal buffer size.
 *
 * See @ref ril_rsp_strpn() when a matching pattern is required.
 *
 * @param st
 *      internal state
 * @param n
 *      number of characters to read at most
 * @param buffer
 *      output buffer
 * @return
 *      number of read bytes
 */
int ril_rsp_charn (ril_state_t* st, size_t n, char* buffer);

/// read a single character matching the given pattern
/**
 * @param st
 *      internal state
 * @param pattern
 *      matching pattern (see @ref ril_rsp_pattern)
 * @return
 *      number of read bytes
 */
int ril_rsp_charp (ril_state_t* st, const char* pattern);

///@}


/* ----------------------------------------------------------------------- */
/**
 * @defgroup ril_rsp_string Strings
 *
 * Functions for matching strings from input stream.
 */
///@{

/// match a fixed string
/**
 * @param st
 *      internal state
 * @param str
 *      fixed string to match
 * @return
 *      number of matching bytes
 */
int ril_rsp_match_str (ril_state_t* st, const char* str);

/// match at most 'n' bytes matching a pattern from input stream
/**
 * See @ref ril_rsp_charn() when a faster, non-matching function can be used.
 *
 * @param st
 *      internal state
 * @param pattern
 *      matching pattern (see @ref ril_rsp_pattern)
 * @param n
 *      maximum number of characters to match
 * @return
 *      number of matching bytes
 */
int ril_rsp_match_strpn (ril_state_t* st, const char* pattern, size_t n);

/// match a string matching the given pattern
/**
 * @param st
 *      internal state
 * @param pattern
 *      matching pattern (see @ref ril_rsp_pattern)
 * @return
 *      number of matching bytes
 */
int ril_rsp_match_strp  (ril_state_t* st, const char* pattern);

/// match the end of line
/**
 * @param st
 *      internal state
 * @return
 *      number of matching bytes
 */
int ril_rsp_match_eol (ril_state_t* st);

/// read an expected string from input stream
/**
 * @param st
 *      internal state
 * @param str
 *      output string
 * @return
 *      number of read bytes
 */
int ril_rsp_str (ril_state_t* st, const char* str);

/// read at most 'n' bytes matching a pattern from input stream
/**
 * See @ref ril_rsp_charn() when a faster, non-matching function can be used.
 *
 * @param st
 *      internal state
 * @param pattern
 *      matching pattern (see @ref ril_rsp_pattern)
 * @param n
 *      number of character to read
 * @param str
 *      output string
 * @return
 *      number of read bytes
 */
int ril_rsp_strpn (ril_state_t* st, const char* pattern, size_t n, char* str);

/// read a string matching the given pattern
/**
 * @param st
 *      internal state
 * @param pattern
 *      matching pattern (see @ref ril_rsp_pattern)
 * @param str
 *      output string
 * @return
 *      number of read bytes
 */
int ril_rsp_strp (ril_state_t* st, const char* pattern, char* str);

/// read a quoted string containing escaped quotes such as `(foo\(\)bar)`
/**
 * @param st
 *      internal state
 * @param quote_begin
 *      prefix character
 * @param quote_end
 *      suffix character
 * @param escape
 *      escape character
 * @param str
 *      output string (including escape, excluding quotes)
 * @return
 *      number of read bytes
 */
int ril_rsp_strqqe (ril_state_t* st, char quote_begin, char quote_end,
                    char escape, char* str);

/// read a quoted string containing escaped quotes such as `"foo\"bar"`
/**
 * @param st
 *      internal state
 * @param quote
 *      prefix and suffix character
 * @param escape
 *      escape character
 * @param str
 *      output string (including escape, excluding quotes)
 * @return
 *      number of read bytes
 */
int ril_rsp_strqe (ril_state_t* st, char quote, char escape, char* str);

/// read a quoted string such as `<foo>`
/**
 * @param st
 *      internal state
 * @param quote_begin
 *      prefix character
 * @param quote_end
 *      suffix character
 * @param str
 *      output string (excluding quotes)
 * @return
 *      number of read bytes
 */
int ril_rsp_strqq (ril_state_t* st, char quote_begin, char quote_end, char* str);

/// read a quoted string such as `"foo"`
/**
 * @param st
 *      internal state
 * @param quote
 *      prefix and suffix character
 * @param str
 *      output string (excluding quotes)
 * @return
 *      number of read bytes
 */
int ril_rsp_strq (ril_state_t* st, char quote, char* str);

/// read the end of line
/**
 * @param st
 *      internal state
 * @return
 *      number of read bytes
 */
int ril_rsp_eol (ril_state_t* st);

///@}


/* ----------------------------------------------------------------------- */
/**
 * @defgroup ril_rsp_line Lines
 *
 * Functions for matching an entire line from input stream. A line is
 * terminated with end-of-line characters.
 */
///@{

/// match a whole line terminated with EOL
/**
 * @param st
 *      internal state
 * @param str
 *      fixed string to match
 * @return
 *      number of matching bytes
 */
int ril_rsp_match_line (ril_state_t* st, const char* str);

/// match a whole containing "<str>: <val>" and terminated with EOL
/**
 * @param st
 *      internal state
 * @param str
 *      fixed string to match
 * @param val
 *      responded integer
 * @return
 *      number of matching bytes
 */
int ril_rsp_match_line_query (ril_state_t* st, const char* str, int* val);

/// read a line containing the given string
/**
 * @param st
 *      internal state
 * @param str
 *      matching string
 * @return
 *      number of read bytes
 */
int ril_rsp_line (ril_state_t* st, const char* str);

/// read 'ABORT' line result
/**
 * @param st
 *      internal state
 * @return
 *      number of read bytes
 */
int ril_rsp_line_abort (ril_state_t* st);

/// read 'ERROR' line result
/**
 * @param st
 *      internal state
 * @return
 *      number of read bytes
 */
int ril_rsp_line_error (ril_state_t* st);

/// read 'OK' line result
/**
 * @param st
 *      internal state
 * @return
 *      number of read bytes
 */
int ril_rsp_line_ok    (ril_state_t* st);

/// dump a line (typically, echo) containing any character
/**
 * @param st
 *      internal state
 * @return
 *      number of read bytes
 *
 * @note
 * Line length being dumped may exceed buffer size:
 * no `RIL_ERR_RSP_READ_OVERFLOW` is generated.
 */
int ril_rsp_line_dump (ril_state_t* st);

///@}


/* ----------------------------------------------------------------------- */
/**
 * @defgroup ril_rsp_number Numbers
 *
 * Functions for matching single characters from input stream.
 */
///@{

/// match an integer
/**
 * Digits may be prefixed by:
 * - a positive sign '+' for positive numbers,
 * - a minus sign '-' for negative numbers.
 *
 * @param st
 *      internal state
 * @return
 *      number of read bytes
 */
int ril_rsp_match_int (ril_state_t* st);

/// match an unsigned integer
/**
 * @param st
 *      internal state
 * @return
 *      number of read bytes
 */
int ril_rsp_match_uint (ril_state_t* st);

/// match an hexadecimal number
/**
 * Digits are in the ranges 0-9, a-f, and A-F.
 *
 * @param st
 *      internal state
 * @return
 *      number of read bytes
 */
int ril_rsp_match_hex (ril_state_t* st);

/// read as many digits as possible and calculate the corresponding integer
/**
 * Digits may be prefixed by a minus sign '-' for negative numbers.
 *
 * @param st
 *      internal state
 * @param val
 *      output value
 * @return
 *      number of read bytes
 */
int ril_rsp_int (ril_state_t* st, int* val);

/// read as many digits as possible and calculate the corresponding integer
/**
 * @param st
 *      internal state
 * @param val
 *      output value
 * @return
 *      number of read bytes
 */
int ril_rsp_uint (ril_state_t* st, unsigned int* val);

/// read as many digits as possible and calculate the corresponding hexadecimal integer
/**
 * @param st
 *      internal state
 * @param val
 *      output value
 * @return
 *      number of read bytes
 */
int ril_rsp_hex (ril_state_t* st, unsigned int* val);

///@}

#endif // RIL_RSP_H

// vim: syntax=c ts=8 sw=8
