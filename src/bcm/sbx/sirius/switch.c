/* $Id: switch.c,v 1.20 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Broadcom QE2000 Switch API.
 */

#include <shared/bsl.h>

#include <soc/drv.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sirius.h>

#include <bcm/error.h>
#include <bcm/switch.h>
#include <bcm/error.h>
#include <bcm/fabric.h>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx_dispatch.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/fabric.h>

static int
_bcm_sirius_sc_pfc_sctype_to_queue(bcm_switch_control_t type);

int
_bcm_switch_stat_threshold_get(int unit, int32 *threshold)
{
    uint32 regval = 0, result = 0;

    BCM_IF_ERROR_RETURN(READ_CS_ACE_CTRLr(unit, &regval));
    if (soc_reg_field_get(unit, CS_ACE_CTRLr, regval, THRESHOLD_EJECT_ENf) == 0) {
      /*
       * We are using ACE
       */
      *threshold = 0;
      return (BCM_E_NONE);
    } else {
      uint64 temp;
      /*
       * We are using Threshold, get the threshold event value and determine percentage
       */
      BCM_IF_ERROR_RETURN(READ_CS_ACE_BYTE_THRESHOLDr(unit, &regval));
      
      COMPILER_64_SET(temp, 0, regval);
      COMPILER_64_ADD_32(temp, 3);
      COMPILER_64_UMUL_32(temp, 100);
      if(soc_sbx_div64(temp, 0x7ffffff, &result) == -1) {
	return (BCM_E_FAIL);
      }
      *threshold = result;
      return(BCM_E_NONE);
    }
}

int
_bcm_sirius_qm_buffer_ager_get(int unit, int *ager_ms)
{
    int     rv = BCM_E_NONE;
    uint32  regval = 0;
    int     enable, ager_sf, age_tick_us;

    BCM_IF_ERROR_RETURN(READ_QMC_AGER_CONFIG0r(unit, &regval));
    enable = soc_reg_field_get(unit, QMC_AGER_CONFIG0r, regval, AGER_ENABLEf);
    ager_sf = soc_reg_field_get(unit, QMC_AGER_CONFIG0r, regval, 
                                AGER_SCALING_FACTORf);
    if (!enable) {
        *ager_ms = 0;
    } else {
        age_tick_us = (SIRIUS_CLOCK_CYCLE_BUFFER_AGER_FACTOR) / (SOC_SBX_CFG(unit)->uClockSpeedInMHz);
        age_tick_us = age_tick_us << ager_sf;
        (*ager_ms) = age_tick_us / 1000;
    }

    return rv;
}

int
_bcm_switch_stat_interval_get(int unit, int *interval)
{
    int     rv = BCM_E_NONE;
    uint32  data;

    if (!interval) {
        return BCM_E_PARAM;
    }

    /* read value from HW which is in clock cycles */
    SOC_IF_ERROR_RETURN(READ_CS_CONFIG_BACKGROUND_RATEr(unit, &data));
    /* convert the clock cycles from 3.5ns to  ms */
    *interval = (data / 286);

    return rv;
}

int
bcm_sirius_switch_control_get(int unit,
			      bcm_switch_control_t type,
			      int *val)
{
    int     rv = BCM_E_NONE;
    int32 threshold = 0;
    int     base_rate;
    int     data;
    int     queue;
    uint32  regval;

    if (!val) {
        return BCM_E_PARAM;
    }

    switch (type) {
    case bcmSwitchCosqStatThreshold:
        BCM_IF_ERROR_RETURN(_bcm_switch_stat_threshold_get(unit,&threshold));
        *val = threshold;
        break;
    case bcmSwitchPktAge:
        BCM_IF_ERROR_RETURN(_bcm_sirius_qm_buffer_ager_get(unit, &base_rate));
        *val = base_rate;
        break;
    case bcmSwitchCosqStatInterval:
        BCM_IF_ERROR_RETURN(_bcm_switch_stat_interval_get(unit, &data));
        *val = data;
        break;
    case bcmSwitchEgressPktAge:
        *val = SOC_SBX_STATE(unit)->fabric_state->egress_aging;
        break;
    case bcmSwitchPFCQueue0Class:
    case bcmSwitchPFCQueue1Class:
    case bcmSwitchPFCQueue2Class:
    case bcmSwitchPFCQueue3Class:
    case bcmSwitchPFCQueue4Class:
    case bcmSwitchPFCQueue5Class:
    case bcmSwitchPFCQueue6Class:
    case bcmSwitchPFCQueue7Class:
	queue = _bcm_sirius_sc_pfc_sctype_to_queue(type);
	BCM_IF_ERROR_RETURN(READ_QM_PFC_CONFIG2r(unit, &regval));
	switch (queue) {
	    case 0:
		*val = soc_reg_field_get(unit, QM_PFC_CONFIG2r, regval, PFC_COS_MAPPING0f);
		break;
	    case 1:
		*val = soc_reg_field_get(unit, QM_PFC_CONFIG2r, regval, PFC_COS_MAPPING1f);
		break;
	    case 2:
		*val = soc_reg_field_get(unit, QM_PFC_CONFIG2r, regval, PFC_COS_MAPPING2f);
		break;
	    case 3:
		*val = soc_reg_field_get(unit, QM_PFC_CONFIG2r, regval, PFC_COS_MAPPING3f);
		break;
	    case 4:
		*val = soc_reg_field_get(unit, QM_PFC_CONFIG2r, regval, PFC_COS_MAPPING4f);
		break;
	    case 5:
		*val = soc_reg_field_get(unit, QM_PFC_CONFIG2r, regval, PFC_COS_MAPPING5f);
		break;
	    case 6:
		*val = soc_reg_field_get(unit, QM_PFC_CONFIG2r, regval, PFC_COS_MAPPING6f);
		break;
	    case 7:
		*val = soc_reg_field_get(unit, QM_PFC_CONFIG2r, regval, PFC_COS_MAPPING7f);
		break;
	}
	rv = BCM_E_NONE;
	break;
    default:
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Unsupported switch control type (%d) \n"), type));
        rv = BCM_E_UNAVAIL;
        break;
    }

    return rv;
}

int
_bcm_switch_stat_threshold_set(int unit, int32 threshold)
{
    uint32 regval = 0;

    if (threshold == 0) {
      /*
       * Enable ACE mode
       */

      BCM_IF_ERROR_RETURN(READ_CS_ACE_CTRLr(unit, &regval));
      soc_reg_field_set(unit, CS_ACE_CTRLr, &regval, THRESHOLD_EJECT_ENf, 0);
      BCM_IF_ERROR_RETURN(WRITE_CS_ACE_CTRLr(unit, regval));
    } else {
      uint64 temp = COMPILER_64_INIT(0,0);

      if ((threshold < 25) || (threshold > 75)) {
	return(BCM_E_PARAM);
      }
      
      BCM_IF_ERROR_RETURN(READ_CS_ACE_CTRLr(unit, &regval));
      soc_reg_field_set(unit, CS_ACE_CTRLr, &regval, THRESHOLD_EJECT_ENf, 1);
      BCM_IF_ERROR_RETURN(WRITE_CS_ACE_CTRLr(unit, regval));

      /*
       * If using THRESHOLD instead of ACE, the following information
       * needs to be set.
       * The Packet (event) field is 21 bits long, so the default of half
       * is used if a maximum limit of 75% is exceeded.
       */

      regval = (0x1fffff * threshold) / 100;
      BCM_IF_ERROR_RETURN(WRITE_CS_ACE_EVENT_THRESHOLDr(unit, regval));

      

      COMPILER_64_SET(temp, 0, threshold);
      COMPILER_64_UMUL_32(temp, 0x7ffffff);

      

      if(soc_sbx_div64(temp, 100, &regval) == -1) {
	  return(BCM_E_INTERNAL);
      }
      BCM_IF_ERROR_RETURN(WRITE_CS_ACE_BYTE_THRESHOLDr(unit, regval));
    }
    return(BCM_E_NONE);
}

int
_bcm_sirius_qm_buffer_ager_set(int unit, int aging_ms)
{
    int     rv = BCM_E_NONE;
    int     ager_sf = 0, enable = 0;
    uint32  data = 0;
    int     aging_us, age_tick_us;

    if (aging_ms > 0) {
        enable = 1;

        age_tick_us = (SIRIUS_CLOCK_CYCLE_BUFFER_AGER_FACTOR) / (SOC_SBX_CFG(unit)->uClockSpeedInMHz);
        aging_us = ((int)(aging_ms * 1000) > aging_ms) ? aging_ms * 1000 : aging_ms;

        while ((age_tick_us < aging_us) && (ager_sf < 7)) {
            age_tick_us = age_tick_us << 1;
            ager_sf++;
        }
    }
    soc_reg_field_set(unit, QMC_AGER_CONFIG0r, &data, AGER_ENABLEf, enable);
    soc_reg_field_set(unit, QMC_AGER_CONFIG0r, &data, AGER_SCALING_FACTORf, ager_sf);
    BCM_IF_ERROR_RETURN(WRITE_QMC_AGER_CONFIG0r(unit, data));

    return rv;
}

int
_bcm_switch_stat_interval_set(int unit, int interval)
{
    int     rv = BCM_E_NONE;
    uint32  data;

    if (interval < 0) {
        return BCM_E_PARAM; /* cant be negative */
    }

    /* check if max value can be supported */
    if (((uint32)interval) >= 0xffffffff) {
        /* Passed in interval is too large. Set the background eject rate to
           max 32bit value */
        data = 0xffffffff;
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "Specified interval is too large. Interval "
                             "set to max supported value of 0x%x ms \n"), data));
    } else {
        /* passed in interval can be supported. Calculate the clock ticks */
        data = (286 * interval);
    }

    rv = WRITE_CS_CONFIG_BACKGROUND_RATEr(unit, data);
    if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Failed to set background eject rate. switch "
                              "control set failed. rv: %d (%s) \n"), rv, soc_errmsg(rv)));
    }

    return rv;
}

int
_bcm_sirius_switch_egress_pkt_age_set(int unit, int egress_pkt_age)
{
    int     rv = BCM_E_NONE;
    uint32  aging_reg_val, class_reg_val = 0;
    int is_gran_ms = 0, gran, i;
    int hw_age;
    int start_limit, min_limit;
    int rounding_accuracy, min_rounding_accuracy;


    BCM_IF_ERROR_RETURN(READ_PKTAGINGTIMERr(unit, &aging_reg_val));

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "Egress Packet Age: 0x%x(%d)\n"),
                 egress_pkt_age, egress_pkt_age));

    /* Is aging disabled */
    if (egress_pkt_age == 0) {
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "Disable Egress Packet Age\n")));
        soc_reg_field_set(unit, PKTAGINGTIMERr, &aging_reg_val, DURATIONSELECTf, 0);
        rv = WRITE_PKTAGINGTIMERr(unit, aging_reg_val);
        SOC_SBX_STATE(unit)->fabric_state->egress_aging = egress_pkt_age;
        return(rv);
    }

    /* determine h/w granularity */
    if (egress_pkt_age > ((500 * 0x3FFF * 15) / 1000)) {
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "Egress Packet Age, h/w granulatity in msec\n")));
        is_gran_ms = 1;
    }

    /* parameter check */
    if (egress_pkt_age > (500 * 0x3FFF * 15)) {
        return(BCM_E_PARAM);
    }

    /* convert aging to h/w granularity. If required do a round up */
    if (is_gran_ms == 1) {
        hw_age = egress_pkt_age / 500 + (((egress_pkt_age % 500) == 0) ? 0 : 1);
    }
    else {
        hw_age = (egress_pkt_age  * 1000) / 500;
    }

    /* Determine the h/w configuration. If required do a round up */
    for (start_limit = 2; (hw_age >> start_limit) & ~(0x3FFF); start_limit++) {
    }

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "Egress Packet Age, h/w age: 0x%x(%d), start_limit: 0x%x(%d)\n"),
                 hw_age, hw_age, start_limit, start_limit));

    min_rounding_accuracy = 0;
    min_limit = start_limit;
    for (i = start_limit; i < 14; i++) {
        gran = hw_age >> i;
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "Egress Packet Age, gran: 0x%x(%d)\n"),
                     gran, gran));
        if ((hw_age - (gran << i)) == 0) {
            min_limit = i;
            min_rounding_accuracy = 0;
            LOG_VERBOSE(BSL_LS_BCM_COMMON,
                        (BSL_META_U(unit,
                                    "Egress Packet Age, sel_min_limit: %d round_inaccuracy: %d\n"),
                         min_limit, min_rounding_accuracy));
            break;
        }
        rounding_accuracy = ((gran + 1) << i) - hw_age;

        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "Egress Packet Age, rounded_gran: 0x%x(%d) rounded_age: 0x%x(%d) hw_age: 0x%x(%d), min_limit: %d round_inaccuracy: %d\n"),
                     (gran + 1), (gran + 1), ((gran + 1) << i), ((gran + 1) << i), hw_age, hw_age, i, rounding_accuracy));

        if (i == start_limit) {
            min_rounding_accuracy = rounding_accuracy;
            min_limit = i;
            LOG_VERBOSE(BSL_LS_BCM_COMMON,
                        (BSL_META_U(unit,
                                    "Egress Packet Age, cur_min_limit: %d round_inaccuracy: %d\n"),
                         min_limit, min_rounding_accuracy));
        }
        else if (rounding_accuracy < min_rounding_accuracy) {
            min_rounding_accuracy = rounding_accuracy;
            min_limit = i;
            LOG_VERBOSE(BSL_LS_BCM_COMMON,
                        (BSL_META_U(unit,
                                    "Egress Packet Age, cur_min_limit: %d round_inaccuracy: %d\n"),
                         min_limit, min_rounding_accuracy));
        }
    }

    gran = (hw_age >> min_limit);
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "Egress Packet Age, gran: 0x%x(%d)\n"),
                 gran, gran));
    if ((hw_age - (gran << min_limit)) > 0) {
       if (gran < 0x3FFF) {
           gran++;
       }
    }
    min_limit = 15 - min_limit;
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "Egress Packet Age, gran: 0x%x(%d) min_limit: %d\n"),
                 gran, gran, min_limit));

    soc_reg_field_set(unit, PKTAGINGTIMERr, &aging_reg_val, DURATIONSELECTf, gran);
    soc_reg_field_set(unit, PKTAGINGTIMERr, &aging_reg_val, AGINGTICKSELf, is_gran_ms);
    BCM_IF_ERROR_RETURN(WRITE_PKTAGINGTIMERr(unit, aging_reg_val));

    /* device/chip level aging */
    
    soc_reg_field_set(unit, PKTAGINGLIMITr, &class_reg_val, CLASS0_LIMITf, min_limit);
    soc_reg_field_set(unit, PKTAGINGLIMITr, &class_reg_val, CLASS1_LIMITf, min_limit);
    soc_reg_field_set(unit, PKTAGINGLIMITr, &class_reg_val, CLASS2_LIMITf, min_limit);
    soc_reg_field_set(unit, PKTAGINGLIMITr, &class_reg_val, CLASS3_LIMITf, min_limit);
    BCM_IF_ERROR_RETURN(WRITE_PKTAGINGTIMERr(unit, class_reg_val));

    SOC_SBX_STATE(unit)->fabric_state->egress_aging = egress_pkt_age;

    return(rv);
}

int
_bcm_sirius_switch_egress_pkt_age_get(int unit, int *egress_pkt_age)
{
    int     rv = BCM_E_NONE;
    uint32  aging_reg_val, class_reg_val;
    int     ms_den_conv_factor = 1, ms_num_conv_factor = 500;
    int     aging_limit, aging_count, duration_count;


    *egress_pkt_age = 0;

    BCM_IF_ERROR_RETURN(READ_PKTAGINGTIMERr(unit, &aging_reg_val));

    /* is aging disabled */
    duration_count = soc_reg_field_get(unit, PKTAGINGTIMERr, aging_reg_val, DURATIONSELECTf);
    if (duration_count == 0) {
        return(rv);
    }

    if (soc_reg_field_get(unit, PKTAGINGTIMERr, aging_reg_val, AGINGTICKSELf) == 0) {
        ms_den_conv_factor = 1000;
    }

    BCM_IF_ERROR_RETURN(READ_PKTAGINGLIMITr(unit, &class_reg_val));

    /* device/chip level aging */
    aging_limit = soc_reg_field_get(unit, PKTAGINGLIMITr, class_reg_val, CLASS0_LIMITf);
    aging_count = 15 - aging_limit;

    (*egress_pkt_age) = (duration_count * ms_num_conv_factor * aging_count) / ms_den_conv_factor;

    return(rv);
}

static int
_bcm_sirius_sc_pfc_sctype_to_queue(bcm_switch_control_t type)
{
    int queue = 0;

    switch (type) {
    case bcmSwitchPFCQueue7Class:
        queue = 7;
        break;
    case bcmSwitchPFCQueue6Class:
        queue = 6;
        break;
    case bcmSwitchPFCQueue5Class:
        queue = 5;
        break;
    case bcmSwitchPFCQueue4Class:
        queue = 4;
        break;
    case bcmSwitchPFCQueue3Class:
        queue = 3;
        break;
    case bcmSwitchPFCQueue2Class:
        queue = 2;
        break;
    case bcmSwitchPFCQueue1Class:
        queue = 1;
        break;
    case bcmSwitchPFCQueue0Class:
        queue = 0;
        break;
    default:
        queue = -1;
        break;
    }
    return queue;
}

int
bcm_sirius_switch_control_set(int unit,
			      bcm_switch_control_t type,
			      int val)
{
    int rv = BCM_E_NONE;
    int queue;
    uint32 regval, pg;

    switch (type) {
    case bcmSwitchCosqStatThreshold:
        rv = _bcm_switch_stat_threshold_set(unit,val);
        break;
    case bcmSwitchPktAge:
        rv = _bcm_sirius_qm_buffer_ager_set(unit, val);
        break;
    case bcmSwitchCosqStatInterval:
        rv = _bcm_switch_stat_interval_set(unit, val);
        break;
    case bcmSwitchEgressPktAge:
        rv = _bcm_sirius_switch_egress_pkt_age_set(unit, val);
        break;
    case bcmSwitchPFCQueue0Class:
    case bcmSwitchPFCQueue1Class:
    case bcmSwitchPFCQueue2Class:
    case bcmSwitchPFCQueue3Class:
    case bcmSwitchPFCQueue4Class:
    case bcmSwitchPFCQueue5Class:
    case bcmSwitchPFCQueue6Class:
    case bcmSwitchPFCQueue7Class:
	if ( (val < 0) || (val > 7) ) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Unsupported PFC priority (%d) \n"), val));
	    return BCM_E_PARAM;
	}

	queue = _bcm_sirius_sc_pfc_sctype_to_queue(type);
	BCM_IF_ERROR_RETURN(READ_QM_PFC_CONFIG2r(unit, &regval));
	switch (queue) {
	    case 0:
		soc_reg_field_set(unit, QM_PFC_CONFIG2r, &regval,
				  PFC_COS_MAPPING0f, val);
		break;
	    case 1:
		soc_reg_field_set(unit, QM_PFC_CONFIG2r, &regval,
				  PFC_COS_MAPPING1f, val);
		break;
	    case 2:
		soc_reg_field_set(unit, QM_PFC_CONFIG2r, &regval,
				  PFC_COS_MAPPING2f, val);
		break;
	    case 3:
		soc_reg_field_set(unit, QM_PFC_CONFIG2r, &regval,
				  PFC_COS_MAPPING3f, val);
		break;
	    case 4:
		soc_reg_field_set(unit, QM_PFC_CONFIG2r, &regval,
				  PFC_COS_MAPPING4f, val);
		break;
	    case 5:
		soc_reg_field_set(unit, QM_PFC_CONFIG2r, &regval,
				  PFC_COS_MAPPING5f, val);
		break;
	    case 6:
		soc_reg_field_set(unit, QM_PFC_CONFIG2r, &regval,
				  PFC_COS_MAPPING6f, val);
		break;
	    case 7:
		soc_reg_field_set(unit, QM_PFC_CONFIG2r, &regval,
				  PFC_COS_MAPPING7f, val);
		break;
	    default:
		return BCM_E_PARAM;
	}
	BCM_IF_ERROR_RETURN(WRITE_QM_PFC_CONFIG2r(unit, regval));

	if (soc_feature(unit, soc_feature_priority_flow_control) &&
	    !soc_feature(unit, soc_feature_source_port_priority_flow_control)) {
	    /* reverse mapping of PFC class to queue for SAFC */
	    switch (val) {
		case 0:
		    BCM_IF_ERROR_RETURN(READ_QM_LLFC_COS_TO_PG_MAP0r(unit, &regval));
		    pg = soc_reg_field_get(unit, QM_LLFC_COS_TO_PG_MAP0r, regval, COS0_PGf);
		    pg |= (1<<queue);
		    soc_reg_field_set(unit, QM_LLFC_COS_TO_PG_MAP0r, &regval, COS0_PGf, pg);
		    BCM_IF_ERROR_RETURN(WRITE_QM_LLFC_COS_TO_PG_MAP0r(unit, regval));
		    break;
		case 1:
		    BCM_IF_ERROR_RETURN(READ_QM_LLFC_COS_TO_PG_MAP0r(unit, &regval));
		    pg = soc_reg_field_get(unit, QM_LLFC_COS_TO_PG_MAP0r, regval, COS1_PGf);
		    pg |= (1<<queue);
		    soc_reg_field_set(unit, QM_LLFC_COS_TO_PG_MAP0r, &regval, COS1_PGf, pg);
		    BCM_IF_ERROR_RETURN(WRITE_QM_LLFC_COS_TO_PG_MAP0r(unit, regval));
		    break;
		case 2:
		    BCM_IF_ERROR_RETURN(READ_QM_LLFC_COS_TO_PG_MAP1r(unit, &regval));
		    pg = soc_reg_field_get(unit, QM_LLFC_COS_TO_PG_MAP1r, regval, COS2_PGf);
		    pg |= (1<<queue);
		    soc_reg_field_set(unit, QM_LLFC_COS_TO_PG_MAP1r, &regval, COS2_PGf, pg);
		    BCM_IF_ERROR_RETURN(WRITE_QM_LLFC_COS_TO_PG_MAP1r(unit, regval));
		    break;
		case 3:
		    BCM_IF_ERROR_RETURN(READ_QM_LLFC_COS_TO_PG_MAP1r(unit, &regval));
		    pg = soc_reg_field_get(unit, QM_LLFC_COS_TO_PG_MAP1r, regval, COS3_PGf);
		    pg |= (1<<queue);
		    soc_reg_field_set(unit, QM_LLFC_COS_TO_PG_MAP1r, &regval, COS3_PGf, pg);
		    BCM_IF_ERROR_RETURN(WRITE_QM_LLFC_COS_TO_PG_MAP1r(unit, regval));
		    break;
		case 4:
		    BCM_IF_ERROR_RETURN(READ_QM_LLFC_COS_TO_PG_MAP2r(unit, &regval));
		    pg = soc_reg_field_get(unit, QM_LLFC_COS_TO_PG_MAP2r, regval, COS4_PGf);
		    pg |= (1<<queue);
		    soc_reg_field_set(unit, QM_LLFC_COS_TO_PG_MAP2r, &regval, COS4_PGf, pg);
		    BCM_IF_ERROR_RETURN(WRITE_QM_LLFC_COS_TO_PG_MAP2r(unit, regval));
		    break;
		case 5:
		    BCM_IF_ERROR_RETURN(READ_QM_LLFC_COS_TO_PG_MAP2r(unit, &regval));
		    pg = soc_reg_field_get(unit, QM_LLFC_COS_TO_PG_MAP2r, regval, COS5_PGf);
		    pg |= (1<<queue);
		    soc_reg_field_set(unit, QM_LLFC_COS_TO_PG_MAP2r, &regval, COS5_PGf, pg);
		    BCM_IF_ERROR_RETURN(WRITE_QM_LLFC_COS_TO_PG_MAP2r(unit, regval));
		    break;
		case 6:
		    BCM_IF_ERROR_RETURN(READ_QM_LLFC_COS_TO_PG_MAP3r(unit, &regval));
		    pg = soc_reg_field_get(unit, QM_LLFC_COS_TO_PG_MAP3r, regval, COS6_PGf);
		    pg |= (1<<queue);
		    soc_reg_field_set(unit, QM_LLFC_COS_TO_PG_MAP3r, &regval, COS6_PGf, pg);
		    BCM_IF_ERROR_RETURN(WRITE_QM_LLFC_COS_TO_PG_MAP3r(unit, regval));
		    break;
		case 7:
		    BCM_IF_ERROR_RETURN(READ_QM_LLFC_COS_TO_PG_MAP3r(unit, &regval));
		    pg = soc_reg_field_get(unit, QM_LLFC_COS_TO_PG_MAP3r, regval, COS7_PGf);
		    pg |= (1<<queue);
		    soc_reg_field_set(unit, QM_LLFC_COS_TO_PG_MAP3r, &regval, COS7_PGf, pg);
		    BCM_IF_ERROR_RETURN(WRITE_QM_LLFC_COS_TO_PG_MAP3r(unit, regval));
	    }
	}

	/* save the config */
	SOC_SBX_CFG(unit)->bcm_cosq_priority_group[queue] = val;
	rv = BCM_E_NONE;
	break;
    default:
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Unsupported switch control type (%d) \n"), type));
        rv = BCM_E_UNAVAIL;
        break;
    }

    return rv;
}
