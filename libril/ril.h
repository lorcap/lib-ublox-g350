/* @file ril.h
 *
 * @author  Lorenzo Cappelletti
 * @date    2021-03-21
 *
 *
 * @page ril AT command set for Radio Interface Layer
 *
 * This library implements a minimal set of AT commands defined by
 * [3GPP](https://en.wikipedia.org/wiki/3GPP) for GSM terminals.
 *
 * References:
 * - u-blox cellular modules - Data and voice modules - AT commands manual,
 *   UBX-13002752 - R70, 09-Dec-2020
 * - AT Commands Examples - Examples for u-blox cellular modules -
 *   Application Note, UBX-13001820 - R13, 27-May-2019
 */

#include <stdbool.h>
#include "cmd.h"
#include "common.h"
#include "rsp.h"

#ifndef RIL_H
#define RIL_H


/// initialize the internal state
ril_state_t* ril_init (ril_cmd_write_t write, void* write_obj,
                       ril_rsp_read_t read, void* read_obj, size_t buffer_max);

/// destroy the internal state
void ril_deinit (ril_state_t* st);


/* ----------------------------------------------------------------------- */
/**
 * @defgroup ril_general_operation General operation
 */
///@{

/// Firmware version identification +CGMR
int ril_at_cgmr (ril_state_t* st, char* version);

/// Card identification +CCID
int ril_at_ccid (ril_state_t* st, char* ccid);

///@}

/* ----------------------------------------------------------------------- */
/**
 * @defgroup ril_general General
 */
///@{

/// values of 'chset' for +CSCS
enum RIL_CSCS_CHSET
{
        RIL_CSCS_CHSET_UNKNOWN,
        RIL_CSCS_CHSET_8859_1,
        RIL_CSCS_CHSET_GSM,
        RIL_CSCS_CHSET_HEX,
        RIL_CSCS_CHSET_IRA,
        RIL_CSCS_CHSET_PCCP437,
        RIL_CSCS_CHSET_PCCP936,
        RIL_CSCS_CHSET_UCS2,
};

/// TE character set configuration +CSCS
int ril_at_cscs (ril_state_t* st, enum RIL_CSCS_CHSET* chset);

/// TE character set configuration +CSCS (set)
int ril_at_cscs_set (ril_state_t* st, enum RIL_CSCS_CHSET chset);

///@}


/* ----------------------------------------------------------------------- */
/**
 * @defgroup ril_mobile_equipment_control_and_status Mobile equipment control and status
 */
///@{

/// values of 'mode' for +CMER
enum RIL_CMER_MODE
{
        RIL_CMER_MODE_BUFFER_URC,
        RIL_CMER_MODE_DISCARD_URC,
        RIL_CMER_MODE_BUFFER_URC_RESERVED,
        RIL_CMER_MODE_SAME_AS_1,
};

/// values of 'ind' for +CMER
enum RIL_CMER_IND
{
        RIL_CMER_IND_NONE,
        RIL_CMER_IND_VIA_CIEV_URC_NOT_CIND,
        RIL_CMER_IND_VIA_CIEV_URC,
};

/// values of 'bfr' for +CMER
enum RIL_CMER_BFR
{
        RIL_CMER_BFR_CLEAR,
        RIL_CMER_BFR_FLUSH,
};

/// Mobile termination event reporting +CMER
int ril_at_cmer (ril_state_t* st, enum RIL_CMER_MODE* mode,
                 enum RIL_CMER_IND* ind, enum RIL_CMER_BFR* bfr);

/// Mobile termination event reporting +CMER (set)
int ril_at_cmer_set (ril_state_t* st, enum RIL_CMER_MODE mode,
                     enum RIL_CMER_IND ind, enum RIL_CMER_BFR bfr);

/// Clock +CCLK (read)
/**
 * @note
 * `year` shall be a number greater than 2000. `timezone` shall be in
 * minutes rather than quarter of hour.
 */
int ril_at_cclk (ril_state_t* st,
                 int* year, int* month, int* day,
                 int* hours, int* minutes, int* seconds,
                 int* timezone);

/// Clock +CCLK (set)
/**
 * @note
 * `year` is a number greater than 2000. `timezone` is in minutes rather
 * than quarter of hour.
 */
int ril_at_cclk_set (ril_state_t* st,
                     int year, int month, int day,
                     int hours, int minutes, int seconds,
                     int timezone);

/// values of 'n' for +CMEE
enum RIL_CMEE_ERROR
{
        RIL_CMEE_ERROR_DISABLED,
        RIL_CMEE_ERROR_ENABLED,
        RIL_CMEE_ERROR_ENABLED_VERBOSE,
};

/// Report mobile termination error +CMEE
int ril_at_cmee (ril_state_t* st, enum RIL_CMEE_ERROR* n);

/// Report mobile termination error +CMEE (set)
int ril_at_cmee_set (ril_state_t* st, enum RIL_CMEE_ERROR n);

///@}


/* ----------------------------------------------------------------------- */
/**
 * @defgroup ril_network_service Network service
 */
///@{

/// modes for +CGED
enum RIL_CGED_MODE
{
        RIL_CGED_MODE_ONE_SHOT_DUMP,
        RIL_CGED_MODE_PERIODIC_REFRESHED_DUMP,
        RIL_CGED_MODE_STOP_PERIODIC_DUMP,
        RIL_CGED_MODE_ONE_SHOT_SERVING_CELL_DUMP,
        RIL_CGED_MODE_PERIODIC_SERVING_CELL_REFRESHED_DUMP,
        RIL_CGED_MODE_ONE_SHOT_SERVING_CELL_AND_NEIGHBOUR_CELLS_DUMP,
        RIL_CGED_MODE_PERIODIC_SERVING_CELL_AND_NEIGHBOUR_CELLS_REFRESHED_DUMP,
        RIL_CGED_MODE_DEFAULT = RIL_CGED_MODE_ONE_SHOT_DUMP
};

///  Cell environment description +CGED
int ril_at_cged (ril_state_t* st, enum RIL_CGED_MODE* mode);

///  Cell environment description +CGED (set)
int ril_at_cged_set (ril_state_t* st, enum RIL_CGED_MODE mode);

///@}


/* ----------------------------------------------------------------------- */
/**
 * @defgroup ril_short_messages_service Short Messages Service
 */
///@{

/// Preferred message format +CMGF
int ril_at_cmgf (ril_state_t* st, int* mode);

/// Preferred message format +CMGF (set)
int ril_at_cmgf_set (ril_state_t* st, int mode);

/// Show text mode parameters +CSDH
int ril_at_csdh (ril_state_t* st, int* mode);

/// Show text mode parameters +CSDH (set)
int ril_at_csdh_set (ril_state_t* st, int mode);

/// New message indication +CNMI
int ril_at_cnmi (ril_state_t* st, int* mode, int* mt, int* bm, int* ds, int* bfr);

/// New message indication +CNMI (set)
int ril_at_cnmi_set (ril_state_t* st, int mode, int mt, int bm, int ds, int bfr);

/// Service center address +CSCA
int ril_at_csca (ril_state_t* st, char* sca, int* tosca);

/// Service center address +CSCA (set)
int ril_at_csca_set (ril_state_t* st, const char* sca);

///@}


/* ----------------------------------------------------------------------- */
/**
 * @defgroup ril_v24_control_and_v25ter V24 control and V25ter
 */
///@{

/// Command echo E
int ril_ate_set (ril_state_t* st, int value);

///@}


/* ----------------------------------------------------------------------- */
/**
 * @defgroup ril_packet_switched_data_services Packet switched data servicies
 */
///@{

/// GPRS attach or detach +CGATT
int ril_at_cgatt (ril_state_t* st, int* state);

/// GPRS attach or detach +CGATT (set)
int ril_at_cgatt_set (ril_state_t* st, int state);

/// values of 'n' for +CREG
enum RIL_CGREG_N
{
        RIL_CGREG_N_NETWORK_REGISTRATION_URC_DISABLED,
        RIL_CGREG_N_NETWORK_REGISTRATION_URC_ENABLED,
        RIL_CGREG_N_NETWORK_REGISTRATION_AND_LOCATION_INFORMATION_URC_ENABLED,
        RIL_CGREG_N_DEFAULT = RIL_CGREG_N_NETWORK_REGISTRATION_URC_DISABLED
};

/// values of 'stat' for +CREG
enum RIL_CGREG_STAT
{
        RIL_CGREG_STAT_NOT_REGISTERED,
        RIL_CGREG_STAT_REGISTERED_HOME_NETWORK,
        RIL_CGREG_STAT_SEARCHING,
        RIL_CGREG_STAT_REGISTRATION_DENIED,
        RIL_CGREG_STAT_UNKNOWN,
        RIL_CGREG_STAT_REGISTERED_ROAMING,
};

/// GPRS network registration status +CGREG
int ril_at_cgreg (ril_state_t* st, enum RIL_CGREG_N* n,
                  enum RIL_CGREG_STAT* stat, unsigned int* lac,
                  unsigned int* ci);

/// GPRS network registration status +CGREG (set)
int ril_at_cgreg_set (ril_state_t* st, enum RIL_CGREG_N n);

///@}

#endif // RIL_H

// vim: syntax=c ts=8 sw=8
