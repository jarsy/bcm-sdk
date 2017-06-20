/*
 * $Id: tdpll_outputs.c, Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: tdpll_outputs.c
 *
 * Purpose: Telecom DPLL output clock (synthesizer) configuration and management.
 *
 * Functions:
 *      bcm_tdpll_output_clock_init
 *      bcm_tdpll_output_clock_enable_get
 *      bcm_tdpll_output_clock_enable_set
 *      bcm_tdpll_output_clock_synth_frequency_get
 *      bcm_tdpll_output_clock_synth_frequency_set
 *      bcm_tdpll_output_clock_deriv_frequency_get
 *      bcm_tdpll_output_clock_deriv_frequency_set
 *      bcm_tdpll_output_clock_holdover_data_get
 *      bcm_tdpll_output_clock_holdover_frequency_set
 *      bcm_tdpll_output_clock_holdover_mode_get
 *      bcm_tdpll_output_clock_holdover_mode_set
 *      bcm_tdpll_output_clock_holdover_reset
 */

#if defined(INCLUDE_PTP)

#include <shared/bsl.h>

#include <bcm/ptp.h>
#include <bcm_int/common/ptp.h>
#include <bcm_int/ptp_common.h>
#include <bcm/error.h>

/* Definitions. */
#define TDPLL_OUTPUT_CLOCK_STATE_ENABLE_BIT                            (0u)

#define OUTPUT_CLOCK(clock_index)       \
(objdata.output_clock[clock_index])

/* Macros. */

/* Types. */

/* Constants and variables. */
static bcm_tdpll_output_clock_data_t objdata;

/* Static functions. */


/*
 * Function:
 *      bcm_tdpll_output_clock_init()
 * Purpose:
 *      Initialize T-DPLL output clock configuration and management data.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      stack_id  - (IN) Stack identifier index.
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 */
int
bcm_tdpll_output_clock_init(
    int unit,
    int stack_id)
{
    int i;

    /*Initialize output clock attributes. */
    for (i = 0; i < TDPLL_OUTPUT_CLOCK_NUM_MAX; ++i) {
        /* Identification attributes. */
        OUTPUT_CLOCK(i).index = i;

        /* Synthesizer frequency and TS EVENT (timestamp) frequency. */
        OUTPUT_CLOCK(i).frequency.synth = 0;
        OUTPUT_CLOCK(i).frequency.tsevent = 0;
        OUTPUT_CLOCK(i).frequency.tsevent_quotient = -1;

        /* Derivative clock frequency. */
        OUTPUT_CLOCK(i).frequency.deriv = 0;
        OUTPUT_CLOCK(i).frequency.deriv_quotient = -1;

        /* State. */
        OUTPUT_CLOCK(i).state = 0;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tdpll_output_clock_cleanup()
 * Purpose:
 *      Uninitialize T-DPLL output clock configuration and management data.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      stack_id  - (IN) Stack identifier index.
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 */
int
bcm_tdpll_output_clock_cleanup(
    int unit,
    int stack_id)
{
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_common_tdpll_output_clock_index_validate()
 * Purpose:
 *      Validate the TDPLL output clock index.
 * Parameters:
 *      unit        - (IN) Unit number.
 *      clock_index - (IN) Output clock index.
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 */

static int
_bcm_common_tdpll_output_clock_index_validate (
    int unit,
    int clock_index)
{
    int rv = BCM_E_NONE;

    if (clock_index < 0 || clock_index >= TDPLL_OUTPUT_CLOCK_NUM_MAX) {
        return BCM_E_PARAM;
    }

    if (!soc_feature(unit, soc_feature_tdpll_outputclk_synce3) &&
       (clock_index == TDPLL_OUTPUT_CLOCK_IDX_SYNCE3) ) {
        return BCM_E_PARAM;
    }  
     
    if (!soc_feature(unit, soc_feature_tdpll_outputclk_xgpll3) &&
	 (clock_index == TDPLL_OUTPUT_CLOCK_IDX_XGPLL3)) {
       return BCM_E_PARAM;
    }
    return rv;
}

/*
 * Function:
 *      bcm_tdpll_output_clock_enable_get()
 * Purpose:
 *      Get output clock enable Boolean.
 * Parameters:
 *      unit        - (IN) Unit number.
 *      stack_id    - (IN) Stack identifier index.
 *      clock_index - (IN) Output clock index.
 *      enable      - (OUT) Output clock enable Boolean.
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 */
int
bcm_common_tdpll_output_clock_enable_get(
    int unit,
    int stack_id,
    int clock_index,
    int *enable)
{
    int i;
    int rv;

    uint8 payload[PTP_MGMTMSG_PAYLOAD_INDEXED_PROPRIETARY_MSG_SIZE_OCTETS] = {0};
    uint8 resp[PTP_MGMTMSG_PAYLOAD_OUTPUT_CLOCK_ENABLED_SIZE_OCTETS] = {0};
    int resp_len = PTP_MGMTMSG_PAYLOAD_OUTPUT_CLOCK_ENABLED_SIZE_OCTETS;

    bcm_ptp_port_identity_t portid;

    if (clock_index < 0 || clock_index >= TDPLL_OUTPUT_CLOCK_NUM_MAX) {
        return BCM_E_PARAM;
    }

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;
    }

    if (BCM_FAILURE(rv = bcm_common_ptp_clock_port_identity_get(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_IEEE1588_ALL_PORTS, &portid))) {
        PTP_ERROR_FUNC("bcm_common_ptp_clock_port_identity_get()");
        return rv;
    }

    /* Make indexed payload to get frequency for specified output clock. */
    sal_memcpy(payload, "BCM\0\0\0", 6);
    payload[6] = (uint8)clock_index;
    if (BCM_FAILURE(rv = _bcm_ptp_management_message_send(unit, stack_id, PTP_CLOCK_NUMBER_DEFAULT,
            &portid, PTP_MGMTMSG_GET, PTP_MGMTMSG_ID_OUTPUT_CLOCK_ENABLED,
            payload, PTP_MGMTMSG_PAYLOAD_INDEXED_PROPRIETARY_MSG_SIZE_OCTETS,
            resp, &resp_len))) {
        PTP_ERROR_FUNC("_bcm_ptp_management_message_send()");
        return rv;
    }

    /*
     * Parse response.
     *    Octet 0...5   : Custom management message key/identifier.
     *                    BCM<null><null><null>.
     *    Octet 6       : Output clock index.
     *    Octet 7       : Output clock enable Boolean.
     */
    i = 6; /* Advance cursor past custom management message identifier. */
    ++i;   /* Advance past output clock index. */

    *enable = resp[i] ? 1:0;

    /* Set host-maintained output clock enable Boolean. */
    if (*enable) {
        OUTPUT_CLOCK(clock_index).state |= (1 << TDPLL_OUTPUT_CLOCK_STATE_ENABLE_BIT);
    } else {
        OUTPUT_CLOCK(clock_index).state &= ~(1 << TDPLL_OUTPUT_CLOCK_STATE_ENABLE_BIT);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tdpll_output_clock_enable_set()
 * Purpose:
 *      Set output clock enable Boolean.
 * Parameters:
 *      unit        - (IN) Unit number.
 *      stack_id    - (IN) Stack identifier index.
 *      clock_index - (IN) Output clock index.
 *      enable      - (IN) Output clock enable Boolean.
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 */
int
bcm_common_tdpll_output_clock_enable_set(
    int unit,
    int stack_id,
    int clock_index,
    int enable)
{
    int i;
    int rv;

    uint8 payload[PTP_MGMTMSG_PAYLOAD_OUTPUT_CLOCK_ENABLED_SIZE_OCTETS] = {0};
    uint8 resp[PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS];
    int resp_len = PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS;

    bcm_ptp_port_identity_t portid;

    if (BCM_FAILURE(rv = _bcm_common_tdpll_output_clock_index_validate(unit, clock_index))) {
        PTP_ERROR_FUNC("_bcm_common_tdpll_output_clock_index_validate()");
        return rv;
    }

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;
    }

    if (BCM_FAILURE(rv = bcm_common_ptp_clock_port_identity_get(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_IEEE1588_ALL_PORTS, &portid))) {
        PTP_ERROR_FUNC("bcm_common_ptp_clock_port_identity_get()");
        return rv;
    }

    /*
     * Make payload.
     *    Octet 0...5   : Custom management message key/identifier.
     *                    BCM<null><null><null>.
     *    Octet 6       : Output clock index.
     *    Octet 7       : Output clock enable Boolean.
     */
    sal_memcpy(payload, "BCM\0\0\0", 6);
    i = 6;
    payload[i++] = (uint8)clock_index;
    payload[i] = enable ? 1:0;

    if (BCM_FAILURE(rv = _bcm_ptp_management_message_send(unit, stack_id, PTP_CLOCK_NUMBER_DEFAULT,
            &portid, PTP_MGMTMSG_SET, PTP_MGMTMSG_ID_OUTPUT_CLOCK_ENABLED,
            payload, PTP_MGMTMSG_PAYLOAD_OUTPUT_CLOCK_ENABLED_SIZE_OCTETS,
            resp, &resp_len))) {
        PTP_ERROR_FUNC("_bcm_ptp_management_message_send()");
        return rv;
    }

    /* Set host-maintained output clock enable Boolean. */
    if (enable) {
        OUTPUT_CLOCK(clock_index).state |= (1 << TDPLL_OUTPUT_CLOCK_STATE_ENABLE_BIT);
    } else {
        OUTPUT_CLOCK(clock_index).state &= ~(1 << TDPLL_OUTPUT_CLOCK_STATE_ENABLE_BIT);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tdpll_output_clock_synth_frequency_get()
 * Purpose:
 *      Get output-clock (synthesizer) frequency.
 * Parameters:
 *      unit              - (IN) Unit number.
 *      stack_id          - (IN) Stack identifier index.
 *      clock_index       - (IN) Output clock index.
 *      synth_frequency   - (OUT) Synthesizer frequency (Hz).
 *      tsevent_frequency - (OUT) TS event frequency (Hz).
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 */
int
bcm_common_tdpll_output_clock_synth_frequency_get(
    int unit,
    int stack_id,
    int clock_index,
    uint32 *synth_frequency,
    uint32 *tsevent_frequency)
{
    int i;
    int rv;

    uint8 payload[PTP_MGMTMSG_PAYLOAD_INDEXED_PROPRIETARY_MSG_SIZE_OCTETS] = {0};
    uint8 resp[PTP_MGMTMSG_PAYLOAD_OUTPUT_CLOCK_SYNTH_FREQUENCY_SIZE_OCTETS] = {0};
    int resp_len = PTP_MGMTMSG_PAYLOAD_OUTPUT_CLOCK_SYNTH_FREQUENCY_SIZE_OCTETS;

    bcm_ptp_port_identity_t portid;

    if (BCM_FAILURE(rv = _bcm_common_tdpll_output_clock_index_validate(unit, clock_index))) {
        PTP_ERROR_FUNC("_bcm_common_tdpll_output_clock_index_validate()");
        return rv;
    }

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;
    }

    if (BCM_FAILURE(rv = bcm_common_ptp_clock_port_identity_get(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_IEEE1588_ALL_PORTS, &portid))) {
        PTP_ERROR_FUNC("bcm_common_ptp_clock_port_identity_get()");
        return rv;
    }

    /* Make indexed payload to get synthesizer frequency for specified output clock. */
    sal_memcpy(payload, "BCM\0\0\0", 6);
    payload[6] = (uint8)clock_index;
    if (BCM_FAILURE(rv = _bcm_ptp_management_message_send(unit, stack_id, PTP_CLOCK_NUMBER_DEFAULT,
            &portid, PTP_MGMTMSG_GET, PTP_MGMTMSG_ID_OUTPUT_CLOCK_SYNTH_FREQUENCY,
            payload, PTP_MGMTMSG_PAYLOAD_INDEXED_PROPRIETARY_MSG_SIZE_OCTETS,
            resp, &resp_len))) {
        PTP_ERROR_FUNC("_bcm_ptp_management_message_send()");
        return rv;
    }

    /*
     * Parse response.
     *    Octet 0...5   : Custom management message key/identifier.
     *                    BCM<null><null><null>.
     *    Octet 6       : Output clock index.
     *    Octet 7       : Reserved.
     *    Octet 8...11  : Synthesizer frequency (Hz).
     *    Octet 12...15 : TS event frequency (Hz).
     */
    i = 6; /* Advance cursor past custom management message identifier. */
    ++i;   /* Advance past output clock index. */
    ++i;   /* Advance past reserved octet. */

    *synth_frequency = _bcm_ptp_uint32_read(resp + i);
    i += 4;
    *tsevent_frequency = _bcm_ptp_uint32_read(resp + i);

    /* Set host-maintained output clock frequencies. */
    OUTPUT_CLOCK(clock_index).frequency.synth = *synth_frequency;
    OUTPUT_CLOCK(clock_index).frequency.tsevent = *tsevent_frequency;
    OUTPUT_CLOCK(clock_index).frequency.tsevent_quotient = *tsevent_frequency ?
        (*synth_frequency + (*tsevent_frequency >> 1))/(*tsevent_frequency) : -1;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tdpll_output_clock_synth_frequency_set()
 * Purpose:
 *      Set output-clock (synthesizer) frequency.
 * Parameters:
 *      unit              - (IN) Unit number.
 *      stack_id          - (IN) Stack identifier index.
 *      clock_index       - (IN) Output clock index.
 *      synth_frequency   - (IN) Synthesizer frequency (Hz).
 *      tsevent_frequency - (IN) TS event frequency (Hz).
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 */
int
bcm_common_tdpll_output_clock_synth_frequency_set(
    int unit,
    int stack_id,
    int clock_index,
    uint32 synth_frequency,
    uint32 tsevent_frequency)
{
    int i;
    int rv;

    uint8 payload[PTP_MGMTMSG_PAYLOAD_OUTPUT_CLOCK_SYNTH_FREQUENCY_SIZE_OCTETS] = {0};
    uint8 resp[PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS];
    int resp_len = PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS;

    bcm_ptp_port_identity_t portid;

    if (BCM_FAILURE(rv = _bcm_common_tdpll_output_clock_index_validate(unit, clock_index))) {
        PTP_ERROR_FUNC("_bcm_common_tdpll_output_clock_index_validate()");
        return rv;
    }

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;
    }

    if (BCM_FAILURE(rv = bcm_common_ptp_clock_port_identity_get(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_IEEE1588_ALL_PORTS, &portid))) {
        PTP_ERROR_FUNC("bcm_common_ptp_clock_port_identity_get()");
        return rv;
    }

    /*
     * CONSTRAINT: TS event frequency is a multiple of 1 kHz / 100 Hz.
     *             DPLL instances (and physical synthesizers bound to them)
     *             operate at 1 kHz / 100 Hz.
     */
    if ((0 == tsevent_frequency) || (tsevent_frequency % BCM_TDPLL_FREQUENCY)) {
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "TS EVENT frequency is not a multiple of %u Hz. fTS: %u\n"),
                     (unsigned)BCM_TDPLL_FREQUENCY, (unsigned)tsevent_frequency));
        return BCM_E_PARAM;
    }

    /*
     * CONSTRAINT: TS event frequency and synthesizer frequency are integrally
     *             related, N = f_synth / f_tsevent, such that TS events occur
     *             at every Nth synthesizer clock edge.
     */
    if ((0 == synth_frequency) || (synth_frequency % tsevent_frequency)) {
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "SYNTH and TS EVENT frequencies are not integrally related. fSYNTH: %u fTS: %u"),
                     (unsigned)synth_frequency, (unsigned)tsevent_frequency));
        return BCM_E_PARAM;
    }

    /*
     * Make payload.
     *    Octet 0...5   : Custom management message key/identifier.
     *                    BCM<null><null><null>.
     *    Octet 6       : Output clock index.
     *    Octet 7       : Reserved.
     *    Octet 8...11  : Synthesizer frequency (Hz).
     *    Octet 12...15 : TS event frequency (Hz).
     */
    sal_memcpy(payload, "BCM\0\0\0", 6);
    i = 6;
    payload[i++] = (uint8)clock_index;
    payload[i++] = 0;

    _bcm_ptp_uint32_write(payload+i, synth_frequency);
    i += 4;
    _bcm_ptp_uint32_write(payload+i, tsevent_frequency);

    if (BCM_FAILURE(rv = _bcm_ptp_management_message_send(unit, stack_id, PTP_CLOCK_NUMBER_DEFAULT,
            &portid, PTP_MGMTMSG_SET, PTP_MGMTMSG_ID_OUTPUT_CLOCK_SYNTH_FREQUENCY,
            payload, PTP_MGMTMSG_PAYLOAD_OUTPUT_CLOCK_SYNTH_FREQUENCY_SIZE_OCTETS,
            resp, &resp_len))) {
        PTP_ERROR_FUNC("_bcm_ptp_management_message_send()");
        return rv;
    }

    /* Set host-maintained output clock frequencies. */
    OUTPUT_CLOCK(clock_index).frequency.synth = synth_frequency;
    OUTPUT_CLOCK(clock_index).frequency.tsevent = tsevent_frequency;
    OUTPUT_CLOCK(clock_index).frequency.tsevent_quotient = synth_frequency/tsevent_frequency;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tdpll_output_clock_deriv_frequency_get()
 * Purpose:
 *      Get synthesizer derivative-clock frequency.
 * Parameters:
 *      unit            - (IN) Unit number.
 *      stack_id        - (IN) Stack identifier index.
 *      clock_index     - (IN) Output clock index.
 *      deriv_frequency - (OUT) Derivative clock frequency (Hz).
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 */
int
bcm_common_tdpll_output_clock_deriv_frequency_get(
    int unit,
    int stack_id,
    int clock_index,
    uint32 *deriv_frequency)
{
    int i;
    int rv;

    uint32 synth_frequency;

    uint8 payload[PTP_MGMTMSG_PAYLOAD_INDEXED_PROPRIETARY_MSG_SIZE_OCTETS] = {0};
    uint8 resp[PTP_MGMTMSG_PAYLOAD_OUTPUT_CLOCK_DERIV_FREQUENCY_SIZE_OCTETS] = {0};
    int resp_len = PTP_MGMTMSG_PAYLOAD_OUTPUT_CLOCK_DERIV_FREQUENCY_SIZE_OCTETS;

    bcm_ptp_port_identity_t portid;

    if (BCM_FAILURE(rv = _bcm_common_tdpll_output_clock_index_validate(unit, clock_index))) {
        PTP_ERROR_FUNC("_bcm_common_tdpll_output_clock_index_validate()");
        return rv;
    }

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;
    }

    if (BCM_FAILURE(rv = bcm_common_ptp_clock_port_identity_get(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_IEEE1588_ALL_PORTS, &portid))) {
        PTP_ERROR_FUNC("bcm_common_ptp_clock_port_identity_get()");
        return rv;
    }

    /* Make indexed payload to get derivative-clock frequency(ies) for specified output clock. */
    sal_memcpy(payload, "BCM\0\0\0", 6);
    payload[6] = (uint8)clock_index;
    if (BCM_FAILURE(rv = _bcm_ptp_management_message_send(unit, stack_id, PTP_CLOCK_NUMBER_DEFAULT,
            &portid, PTP_MGMTMSG_GET, PTP_MGMTMSG_ID_OUTPUT_CLOCK_DERIV_FREQUENCY,
            payload, PTP_MGMTMSG_PAYLOAD_INDEXED_PROPRIETARY_MSG_SIZE_OCTETS,
            resp, &resp_len))) {
        PTP_ERROR_FUNC("_bcm_ptp_management_message_send()");
        return rv;
    }

    /*
     * Parse response.
     *    Octet 0...5   : Custom management message key/identifier.
     *                    BCM<null><null><null>.
     *    Octet 6       : Output clock index.
     *    Octet 7       : Reserved.
     *    Octet 8...11  : Derivative clock frequency (Hz).
     */
    i = 6; /* Advance cursor past custom management message identifier. */
    ++i;   /* Advance past output clock index. */
    ++i;   /* Advance past reserved octet. */

    *deriv_frequency = _bcm_ptp_uint32_read(resp + i);

    /* Set host-maintained output clock frequencies and ratios. */
    OUTPUT_CLOCK(clock_index).frequency.deriv = *deriv_frequency;

    synth_frequency = OUTPUT_CLOCK(clock_index).frequency.synth;
    OUTPUT_CLOCK(clock_index).frequency.deriv_quotient = *deriv_frequency ?
        (synth_frequency + (*deriv_frequency >> 1))/(*deriv_frequency) : -1;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tdpll_output_clock_deriv_frequency_set()
 * Purpose:
 *      Set synthesizer derivative clock frequency.
 * Parameters:
 *      unit            - (IN) Unit number.
 *      stack_id        - (IN) Stack identifier index.
 *      clock_index     - (IN) Output clock index.
 *      deriv_frequency - (IN) Derivative clock frequency (Hz).
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 */
int
bcm_common_tdpll_output_clock_deriv_frequency_set(
    int unit,
    int stack_id,
    int clock_index,
    uint32 deriv_frequency)
{
    int i;
    int rv;

    uint32 synth_frequency;

    uint8 payload[PTP_MGMTMSG_PAYLOAD_OUTPUT_CLOCK_DERIV_FREQUENCY_SIZE_OCTETS] = {0};
    uint8 resp[PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS];
    int resp_len = PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS;

    bcm_ptp_port_identity_t portid;

    if (BCM_FAILURE(rv = _bcm_common_tdpll_output_clock_index_validate(unit, clock_index))) {
        PTP_ERROR_FUNC("_bcm_common_tdpll_output_clock_index_validate()");
        return rv;
    }

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;
    }

    if (BCM_FAILURE(rv = bcm_common_ptp_clock_port_identity_get(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_IEEE1588_ALL_PORTS, &portid))) {
        PTP_ERROR_FUNC("bcm_common_ptp_clock_port_identity_get()");
        return rv;
    }

    /*
     * CONSTRAINT: Derivative clock frequency and synthesizer frequency are
     *             integrally related, i.e. M = f_synth / f_deriv, such that
     *             derivative clock pulses occur every Mth synthesizer clock
     *             pulse.
     */
    synth_frequency = OUTPUT_CLOCK(clock_index).frequency.synth;
    if (deriv_frequency) {
        if ((0 == synth_frequency) || (synth_frequency % deriv_frequency)) {
            LOG_VERBOSE(BSL_LS_BCM_COMMON,
                        (BSL_META_U(unit,
                                    "SYNTH and DERIV frequencies are not integrally related. fSYNTH: %u fDERIV: %u"),
                         (unsigned)synth_frequency, (unsigned)deriv_frequency));
            return BCM_E_PARAM;
        }
    }

    /*
     * Make payload.
     *    Octet 0...5   : Custom management message key/identifier.
     *                    BCM<null><null><null>.
     *    Octet 6       : Output clock index.
     *    Octet 7       : Reserved.
     *    Octet 8...11  : Derivative-clock frequency (Hz).
     */
    sal_memcpy(payload, "BCM\0\0\0", 6);
    i = 6;
    payload[i++] = (uint8)clock_index;
    payload[i++] = 0;

    _bcm_ptp_uint32_write(payload+i, deriv_frequency);

    if (BCM_FAILURE(rv = _bcm_ptp_management_message_send(unit, stack_id, PTP_CLOCK_NUMBER_DEFAULT,
            &portid, PTP_MGMTMSG_SET, PTP_MGMTMSG_ID_OUTPUT_CLOCK_DERIV_FREQUENCY,
            payload, PTP_MGMTMSG_PAYLOAD_OUTPUT_CLOCK_DERIV_FREQUENCY_SIZE_OCTETS,
            resp, &resp_len))) {
        PTP_ERROR_FUNC("_bcm_ptp_management_message_send()");
        return rv;
    }

    /* Set host-maintained output clock frequencies. */
    OUTPUT_CLOCK(clock_index).frequency.deriv = deriv_frequency;

    OUTPUT_CLOCK(clock_index).frequency.deriv_quotient = deriv_frequency ?
        synth_frequency/deriv_frequency : -1;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tdpll_output_clock_holdover_data_get()
 * Purpose:
 *      Get holdover configuration data.
 * Parameters:
 *      unit        - (IN)  Unit number.
 *      stack_id    - (IN) Stack identifier index.
 *      clock_index - (IN) Output clock index.
 *      hdata       - (OUT) Holdover configuration data.
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 */
int
bcm_common_tdpll_output_clock_holdover_data_get(
    int unit,
    int stack_id,
    int clock_index,
    bcm_tdpll_holdover_data_t *hdata)
{
    int rv;
    int i;

    uint8 payload[PTP_MGMTMSG_PAYLOAD_INDEXED_PROPRIETARY_MSG_SIZE_OCTETS] = {0};
    uint8 resp[PTP_MGMTMSG_PAYLOAD_HOLDOVER_DATA_SIZE_OCTETS] = {0};
    int resp_len = PTP_MGMTMSG_PAYLOAD_HOLDOVER_DATA_SIZE_OCTETS;

    bcm_ptp_port_identity_t portid;

    if (BCM_FAILURE(rv = _bcm_common_tdpll_output_clock_index_validate(unit, clock_index))) {
        PTP_ERROR_FUNC("_bcm_common_tdpll_output_clock_index_validate()");
        return rv;
    }

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;
    }

    if (BCM_FAILURE(rv = bcm_common_ptp_clock_port_identity_get(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_IEEE1588_ALL_PORTS, &portid))) {
        PTP_ERROR_FUNC("bcm_common_ptp_clock_port_identity_get()");
        return rv;
    }

    /* Make indexed payload to get holdover data for specified output clock. */
    sal_memcpy(payload, "BCM\0\0\0", 6);
    payload[6] = (uint8)clock_index;
    if (BCM_FAILURE(rv = _bcm_ptp_management_message_send(unit, stack_id, PTP_CLOCK_NUMBER_DEFAULT,
            &portid, PTP_MGMTMSG_GET, PTP_MGMTMSG_ID_HOLDOVER_DATA,
            payload, PTP_MGMTMSG_PAYLOAD_INDEXED_PROPRIETARY_MSG_SIZE_OCTETS,
            resp, &resp_len))) {
        PTP_ERROR_FUNC("_bcm_ptp_management_message_send()");
        return rv;
    }

    /*
     * Parse response.
     *    Octet 0...5   : Custom management message key/identifier.
     *                    BCM<null><null><null>.
     *    Octet 6       : Output clock index.
     *    Octet 7       : Reserved.
     *    Octet 8...11  : Instantaneous holdover frequency (ppt).
     *    Octet 12...15 : 1s average holdover frequency (ppt).
     *    Octet 16...19 : Manual holdover frequency (ppt).
     *    Octet 20...23 : Fast-average holdover frequency (ppt).
     *    Octet 24...27 : Slow-average holdover frequency (ppt).
     *    Octet 28      : Fast-average valid Boolean.
     *    Octet 29      : Slow-average valid Boolean.
     *    Octet 30      : Holdover mode.
     *    Octet 31      : Reserved.
     */
    i = 6; /* Advance cursor past custom management message identifier. */
    ++i;   /* Advance past output clock index. */
    ++i;   /* Advance past reserved octet. */

    hdata->freq_instantaneous = (bcm_tdpll_frequency_correction_t)_bcm_ptp_uint32_read(resp + i);
    i += 4;
    hdata->freq_avg1s = (bcm_tdpll_frequency_correction_t)_bcm_ptp_uint32_read(resp + i);
    i += 4;
    hdata->freq_manual = (bcm_tdpll_frequency_correction_t)_bcm_ptp_uint32_read(resp + i);
    i += 4;
    hdata->freq_fast_average = (bcm_tdpll_frequency_correction_t)_bcm_ptp_uint32_read(resp + i);
    i += 4;
    hdata->freq_slow_average = (bcm_tdpll_frequency_correction_t)_bcm_ptp_uint32_read(resp + i);
    i += 4;
    hdata->freq_fast_average_valid = resp[i++] ? 1:0;
    hdata->freq_slow_average_valid = resp[i++] ? 1:0;
    hdata->mode = resp[i++];

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tdpll_output_clock_holdover_frequency_set()
 * Purpose:
 *      Set manual holdover frequency correction.
 * Parameters:
 *      unit        - (IN) Unit number.
 *      stack_id    - (IN) Stack identifier index.
 *      clock_index - (IN) Output clock index.
 *      hfreq       - (IN) Holdover frequency correction (ppt).
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 *      Manual frequency correction is used if holdover mode is manual.
 */
int
bcm_common_tdpll_output_clock_holdover_frequency_set(
    int unit,
    int stack_id,
    int clock_index,
    bcm_tdpll_frequency_correction_t hfreq)
{
    int rv;
    int i;

    uint8 payload[PTP_MGMTMSG_PAYLOAD_HOLDOVER_FREQUENCY_SIZE_OCTETS] = {0};
    uint8 resp[PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS];
    int resp_len = PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS;

    bcm_ptp_port_identity_t portid;

    if (BCM_FAILURE(rv = _bcm_common_tdpll_output_clock_index_validate(unit, clock_index))) {
        PTP_ERROR_FUNC("_bcm_common_tdpll_output_clock_index_validate()");
        return rv;
    }

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;
    }

    if (BCM_FAILURE(rv = bcm_common_ptp_clock_port_identity_get(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_IEEE1588_ALL_PORTS, &portid))) {
        PTP_ERROR_FUNC("bcm_common_ptp_clock_port_identity_get()");
        return rv;
    }

    /*
     * Make payload.
     *    Octet 0...5  : Custom management message key/identifier.
     *                   BCM<null><null><null>.
     *    Octet 6      : Output clock index.
     *    Octet 7      : Reserved.
     *    Octet 8...11 : Holdover frequency (ppt).
     */
    sal_memcpy(payload, "BCM\0\0\0", 6);
    i = 6;
    payload[i++] = (uint8)clock_index;
    payload[i++] = 0;
    _bcm_ptp_uint32_write(payload + i, (uint32)hfreq);

    if (BCM_FAILURE(rv = _bcm_ptp_management_message_send(unit, stack_id, PTP_CLOCK_NUMBER_DEFAULT,
            &portid, PTP_MGMTMSG_SET, PTP_MGMTMSG_ID_HOLDOVER_FREQUENCY,
            payload, PTP_MGMTMSG_PAYLOAD_HOLDOVER_FREQUENCY_SIZE_OCTETS,
            resp, &resp_len))) {
        PTP_ERROR_FUNC("_bcm_ptp_management_message_send()");
        return rv;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tdpll_output_clock_holdover_mode_get()
 * Purpose:
 *      Get holdover mode.
 *      |Manual|Instantaneous|One-Second|Fast Average|Slow Average|
 * Parameters:
 *      unit        - (IN)  Unit number.
 *      stack_id    - (IN) Stack identifier index.
 *      clock_index - (IN) Output clock index.
 *      hmode       - (OUT) Holdover mode.
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 */
int
bcm_common_tdpll_output_clock_holdover_mode_get(
    int unit,
    int stack_id,
    int clock_index,
    bcm_tdpll_holdover_mode_t *hmode)
{
    int rv;

    uint8 payload[PTP_MGMTMSG_PAYLOAD_INDEXED_PROPRIETARY_MSG_SIZE_OCTETS] = {0};
    uint8 resp[PTP_MGMTMSG_PAYLOAD_HOLDOVER_MODE_SIZE_OCTETS] = {0};
    int resp_len = PTP_MGMTMSG_PAYLOAD_HOLDOVER_MODE_SIZE_OCTETS;

    bcm_ptp_port_identity_t portid;

    if (BCM_FAILURE(rv = _bcm_common_tdpll_output_clock_index_validate(unit, clock_index))) {
        PTP_ERROR_FUNC("_bcm_common_tdpll_output_clock_index_validate()");
        return rv;
    }

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;
    }

    if (BCM_FAILURE(rv = bcm_common_ptp_clock_port_identity_get(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_IEEE1588_ALL_PORTS, &portid))) {
        PTP_ERROR_FUNC("bcm_common_ptp_clock_port_identity_get()");
        return rv;
    }

    /* Make indexed payload to get holdover mode for specified output clock. */
    sal_memcpy(payload, "BCM\0\0\0", 6);
    payload[6] = (uint8)clock_index;
    if (BCM_FAILURE(rv = _bcm_ptp_management_message_send(unit, stack_id, PTP_CLOCK_NUMBER_DEFAULT,
            &portid, PTP_MGMTMSG_GET, PTP_MGMTMSG_ID_HOLDOVER_MODE,
            payload, PTP_MGMTMSG_PAYLOAD_INDEXED_PROPRIETARY_MSG_SIZE_OCTETS,
            resp, &resp_len))) {
        PTP_ERROR_FUNC("_bcm_ptp_management_message_send()");
        return rv;
    }

    /*
     * Parse response.
     *    Octet 0...5 : Custom management message key/identifier.
     *                  BCM<null><null><null>.
     *    Octet 6     : Output clock index.
     *    Octet 7     : Holdover mode.
     */
    *hmode = resp[7];

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tdpll_output_clock_holdover_mode_set()
 * Purpose:
 *      Set holdover mode.
 *      |Manual|Instantaneous|Fast Average|Slow Average|
 * Parameters:
 *      unit        - (IN) Unit number.
 *      stack_id    - (IN) Stack identifier index.
 *      clock_index - (IN) Output clock index.
 *      hmode       - (IN) Holdover mode.
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 */
int
bcm_common_tdpll_output_clock_holdover_mode_set(
    int unit,
    int stack_id,
    int clock_index,
    bcm_tdpll_holdover_mode_t hmode)
{
    int rv;
    int i;

    uint8 payload[PTP_MGMTMSG_PAYLOAD_HOLDOVER_MODE_SIZE_OCTETS] = {0};
    uint8 resp[PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS];
    int resp_len = PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS;

    bcm_ptp_port_identity_t portid;

    if (BCM_FAILURE(rv = _bcm_common_tdpll_output_clock_index_validate(unit, clock_index))) {
        PTP_ERROR_FUNC("_bcm_common_tdpll_output_clock_index_validate()");
        return rv;
    }

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;
    }

    if (BCM_FAILURE(rv = bcm_common_ptp_clock_port_identity_get(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_IEEE1588_ALL_PORTS, &portid))) {
        PTP_ERROR_FUNC("bcm_common_ptp_clock_port_identity_get()");
        return rv;
    }

    /*
     * Make payload.
     *    Octet 0...5 : Custom management message key/identifier.
     *                  BCM<null><null><null>.
     *    Octet 6     : Output clock index.
     *    Octet 7     : Holdover mode.
     */
    sal_memcpy(payload, "BCM\0\0\0", 6);
    i = 6;
    payload[i++] = (uint8)clock_index;
    payload[i++] = (uint8)hmode;

    if (BCM_FAILURE(rv = _bcm_ptp_management_message_send(unit, stack_id, PTP_CLOCK_NUMBER_DEFAULT,
            &portid, PTP_MGMTMSG_SET, PTP_MGMTMSG_ID_HOLDOVER_MODE,
            payload, PTP_MGMTMSG_PAYLOAD_HOLDOVER_MODE_SIZE_OCTETS,
            resp, &resp_len))) {
        PTP_ERROR_FUNC("_bcm_ptp_management_message_send()");
        return rv;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tdpll_output_clock_holdover_reset()
 * Purpose:
 *      Reset holdover frequency calculations.
 * Parameters:
 *      unit        - (IN) Unit number.
 *      stack_id    - (IN) Stack identifier index.
 *      clock_index - (IN) Output clock index.
 * Returns:
 *      BCM_E_XXX - Function status.
 * Notes:
 */
int
bcm_common_tdpll_output_clock_holdover_reset(
    int unit,
    int stack_id,
    int clock_index)
{
    int rv;

    uint8 payload[PTP_MGMTMSG_PAYLOAD_HOLDOVER_RESET_SIZE_OCTETS] = {0};
    uint8 resp[PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS];
    int resp_len = PTP_MGMTMSG_RESP_MAX_SIZE_OCTETS;

    bcm_ptp_port_identity_t portid;

    if (BCM_FAILURE(rv = _bcm_common_tdpll_output_clock_index_validate(unit, clock_index))) {
        PTP_ERROR_FUNC("_bcm_common_tdpll_output_clock_index_validate()");
        return rv;
    }

    if (BCM_FAILURE(rv = _bcm_ptp_function_precheck(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_CLOCK_PORT_NUMBER_DEFAULT))) {
        PTP_ERROR_FUNC("_bcm_ptp_function_precheck()");
        return rv;
    }

    if (BCM_FAILURE(rv = bcm_common_ptp_clock_port_identity_get(unit, stack_id,
            PTP_CLOCK_NUMBER_DEFAULT, PTP_IEEE1588_ALL_PORTS, &portid))) {
        PTP_ERROR_FUNC("bcm_common_ptp_clock_port_identity_get()");
        return rv;
    }

    /*
     * Make payload.
     *    Octet 0...5 : Custom management message key/identifier.
     *                  BCM<null><null><null>.
     *    Octet 6     : Output clock index.
     *    Octet 7     : Reserved.
     */
    sal_memcpy(payload, "BCM\0\0\0", 6);
    payload[6] = (uint8)clock_index;

    if (BCM_FAILURE(rv = _bcm_ptp_management_message_send(unit, stack_id, PTP_CLOCK_NUMBER_DEFAULT,
            &portid, PTP_MGMTMSG_CMD, PTP_MGMTMSG_ID_HOLDOVER_RESET,
            payload, PTP_MGMTMSG_PAYLOAD_HOLDOVER_RESET_SIZE_OCTETS,
            resp, &resp_len))) {
        PTP_ERROR_FUNC("_bcm_ptp_management_message_send()");
        return rv;
    }

    return BCM_E_NONE;
}

#endif /* defined(INCLUDE_PTP) */
