/* @file ublox.h
 *
 * @author  Lorenzo Cappelletti
 * @date    2021-03-21
 *
 *
 * @page ril_ublox AT command set for u-blox radios
 *
 * This library implements custom AT commands for u-blox GSM terminals.
 *
 * References:
 * - u-blox cellular modules - Data and voice modules - AT commands manual,
 *   UBX-13002752 - R70, 09-Dec-2020
 * - AT Commands Examples - Examples for u-blox cellular modules -
 *   Application Note, UBX-13001820 - R13, 27-May-2019
 */

#include "ril.h"

#ifndef RIL_UBLOX_H
#define RIL_UBLOX_H

/// read HEX mode configuration
int ril_at_udconf1 (ril_state_t* st, int* hex_mode);

/// set HEX mode configuration
int ril_at_udconf1_set (ril_state_t* st, int hex_mode);

/// read proprietary features
/**
 * @param op_code
 *      operation code
 * @param ...
 *      0 or more integer parameters terminated by NULL
 */
int ril_at_udconf (ril_state_t* st, int op_code, ...);

/// configure u-blox features
/**
 * @param op_code
 *      operation code
 * @param ...
 *      0 or more integer parameters terminated by -1
 */
int ril_at_udconf_set (ril_state_t* st, int op_code, ...);

#endif // RIL_UBLOX_H

// vim: syntax=c ts=8 sw=8
