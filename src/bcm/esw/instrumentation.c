/*
 * $Id: instr.c,v 1.26 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Tomahawk Instrumentation API
 */

#include <soc/drv.h>

#ifdef BCM_INSTRUMENTATION_SUPPORT

#include <soc/mem.h>
#include <soc/util.h>
#include <soc/debug.h>
#include <sal/types.h>
 #include <sal/core/sync.h>
 #include <sal/core/time.h>
#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/switch.h>
#include <bcm/tx.h>
#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/tx.h>
#include <bcm_int/esw/instrumentation.h> 

/* Flag to check initialized status */
STATIC int _pkt_trace_initialized[BCM_MAX_NUM_UNITS];

/* Protection mutexes for each unit */
sal_mutex_t _pkt_trace_mlock[BCM_MAX_NUM_UNITS];

#define PKT_TRACE_LOCK(unit)    sal_mutex_take(_pkt_trace_mlock[unit], sal_mutex_FOREVER)
#define PKT_TRACE_UNLOCK(unit)  sal_mutex_give(_pkt_trace_mlock[unit])

#define UNIT_VALID_CHECK(unit) \
    if (((unit) < 0) || ((unit) >= BCM_MAX_NUM_UNITS)) { return BCM_E_UNIT; }

#define PKT_TRACE_INIT_DONE(unit)    (_pkt_trace_mlock[unit] != NULL)

#define PKT_TRACE_INIT_CHECK(unit) \
    do { \
        UNIT_VALID_CHECK(unit); \
        if (_pkt_trace_mlock[unit] == NULL) { return BCM_E_INIT; } \
    } while (0)

/*
 * Function:
 *      bcm_esw_pkt_trace_hw_reset
 * Purpose:
 *      clean PTR_RESULTS_BUFFER_IVP/ISW1/ISW2 registers
 *      call this functio before sending a 
 *      visibilty packet
 * Parameters:
 *      IN :  unit
 * Returns:
 *      BCM_E_XXX
 */
int _bcm_esw_pkt_trace_hw_reset(int unit)
{
    if (SOC_IS_TOMAHAWKX(unit)) {
        return _bcm_th_pkt_trace_hw_reset(unit);
    }     
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      _bcm_esw_pkt_trace_src_port_set 
 * Purpose:
 *      set the destination port of 
 *      visiblity packet
 *      this must be called before visibilty packet 
 *      is sent to set the pipe to read the resuls 
 * Parameters:
 *      IN :  unit
 *      IN : logical_dst_port
 * Returns:
 *      BCM_E_XXX
 */
int _bcm_esw_pkt_trace_src_port_set(int unit, uint32 logical_dst_port) 
{
     if (SOC_IS_TOMAHAWKX(unit)) {
        return _bcm_th_pkt_trace_src_port_set(unit, logical_dst_port);
    }     
    return BCM_E_UNAVAIL; 
 }

/*
 * Function:
 *      _bcm_esw_pkt_trace_src_port_get 
 * Purpose:
 *      get the destination port of 
 *      visiblity packet
 * Parameters:
 *      IN :  unit
 *      IN : logical_dst_port
 * Returns:
 *      BCM_E_XXXd
 */

int _bcm_esw_pkt_trace_src_port_get(int unit) 
{
    if (SOC_IS_TOMAHAWKX(unit)) {
        return _bcm_th_pkt_trace_src_port_get(unit);
    }     
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      _bcm_esw_pkt_trace_src_pipe_get 
 * Purpose:
 *      get the destination pipe of 
 *      visiblity packet
 * Parameters:
 *      IN :  unit
 *      IN : logical_dst_port
 * Returns:
 *      BCM_E_XXXd
 */
int _bcm_esw_pkt_trace_src_pipe_get(int unit) 
{
    if (SOC_IS_TOMAHAWKX(unit)) {
        return _bcm_th_pkt_trace_src_pipe_get(unit);
    }     
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      _bcm_esw_pkt_trace_pkt_profile_set 
 * Purpose:
 *      given visibility packet behavior options
 *      setting the profile id for the next visibility packet
 * Parameters:
 *      IN :  unit
 *      IN :  options (BCM_PKT_TRACE_LEARN/NO_IFP/FORWARD)
 * Returns:
 *      BCM_E_XXX
 */
int _bcm_esw_pkt_trace_cpu_profile_set(int unit,uint32 options)
{
    if (SOC_IS_TOMAHAWKX(unit)) {
        return _bcm_th_pkt_trace_cpu_profile_set(unit, options);
    }     
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      _bcm_esw_pkt_trace_cpu_pkt_profile_get
 * Purpose:
 *      retrieve cpu profile id to be used for the
 *      next visibility packet
 * Parameters:
 *      IN :  unit
 *      INOUT : profile id 
 * Returns:
 *      BCM_E_XXX
 */

int _bcm_esw_pkt_trace_cpu_profile_get(int unit, uint32* profile_id)
{
    if (SOC_IS_TOMAHAWKX(unit)) {
        return _bcm_th_pkt_trace_cpu_profile_get(unit, profile_id);
    }     
    return BCM_E_UNAVAIL; 
}
/*
 * Function:
 *      _bcm_esw_pkt_trace_info_get
 * Purpose:
 *      Read visibility packet process data from PTR_RESULTS_BUFFER_IVP, ISW1,
 *      and ISW2 and store into bcm_switch_pkt_trace_info_s *
 * Parameters:
 *      unit - (IN) Unit number.
 *      pkt_trace_info - (INOUT) visibility pkt trace prcoess result
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int _bcm_esw_pkt_trace_info_get(int unit, 
                               uint32 options, uint8 port, 
                               int len, uint8 *data, 
                  bcm_switch_pkt_trace_info_t *pkt_trace_info)
{
    bcm_pkt_t *pkt; 
    int rv = BCM_E_UNAVAIL;
    int values[6] = {0, 0, 0, 0, 0, 0}; 
    UNIT_VALID_CHECK(unit);

    if (soc_feature(unit, soc_feature_visibility)) {
        soc_timeout_t   to; 
        uint8           num_trial = 0;

        if (pkt_trace_info == NULL || 
            data == NULL) {
            return BCM_E_PARAM;
        }
        memset(pkt_trace_info, 0x0, sizeof(bcm_switch_pkt_trace_info_t));
        /* mutex lock*/
        PKT_TRACE_LOCK(unit);
        BCM_IF_ERROR_RETURN(bcm_pkt_alloc(unit, len, 0, &pkt));
        pkt->call_back = 0;
        pkt->blk_count = 1;
        pkt->unit = unit;
        
        rv = bcm_pkt_memcpy(pkt, 0, data, len);
        /*new flag indicating that the packet is for visibility or masquerade */
        pkt->flags2 = BCM_PKT_F2_RX_PORT;
        /* adding visibility option to packet flag */
        pkt->flags2 |= BCM_PKT_F2_VISIBILITY_PKT;
        /* setting pkt trace src port number and profile id on the packet */
        pkt->rx_port = port; 

        /* store profile info to be used during tx*/
        _bcm_esw_pkt_trace_cpu_profile_set(unit, options);
        /* Software workaround for TH */
        if (SOC_IS_TOMAHAWK(unit) &&
            soc_feature(unit, soc_feature_th_pkt_trace_sw_war)) {
            BCM_IF_ERROR_RETURN(
                _bcm_th_pkt_trace_int_lbport_set(unit, port, 1, values));
        }
        /* transmit packet -- support only synchronous mode */
        BCM_IF_ERROR_RETURN(bcm_esw_tx(unit, pkt, NULL));
        /* Software workaround for TH, recover the original configuration */
        if (SOC_IS_TOMAHAWK(unit) &&
            soc_feature(unit, soc_feature_th_pkt_trace_sw_war)) {
            BCM_IF_ERROR_RETURN(
                _bcm_th_pkt_trace_int_lbport_set(unit, port, 0, values));
        }
        soc_timeout_init(&to, SECOND_USEC, 0);

        if (SOC_IS_TOMAHAWKX(unit)) {
            for (;;num_trial++) {
                rv = _bcm_th_pkt_trace_info_get(unit, pkt_trace_info);
                if (rv == BCM_E_NONE) {
                    break;
                }
                if (soc_timeout_check(&to)) {
                    rv = BCM_E_TIMEOUT;
                    break;
                }
                sal_usleep(SECOND_USEC/4);
            }
        }  
        bcm_pkt_free(unit, pkt);      
        /*mutex unlock */
        PKT_TRACE_UNLOCK(unit);
        return rv;
    }
    return rv;
}

/*
 * Function:    _bcm_esw_pkt_trace_free_resources
 * Purpose:     Free pkt trace resources
 * Parameters:  unit - SOC unit number
 * Returns:     Nothing
 */
STATIC void
_bcm_esw_pkt_trace_free_resources(int unit)
{
    if (_pkt_trace_mlock[unit]) {
        sal_mutex_destroy(_pkt_trace_mlock[unit]);
        _pkt_trace_mlock[unit] = NULL;
    }
}

/* De-initialize pkt trace module */
STATIC int
_bcm_esw_pkt_trace_cleanup(int unit)
{
    if (soc_feature(unit, soc_feature_visibility)) {
        _bcm_esw_pkt_trace_free_resources(unit);

        _pkt_trace_initialized[unit] = FALSE;

        return BCM_E_NONE;
    }    

    return BCM_E_UNAVAIL;
}

/* Initialize pkt trace cpu profile table */
int 
_bcm_esw_cpu_pkt_profile_init(int unit)
{
    if (SOC_IS_TOMAHAWKX(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_th_pkt_trace_cpu_profile_init(unit));
        return BCM_E_NONE;
    }       
    return BCM_E_UNAVAIL; 
}

/* Initialize pkt trace module */
int
bcm_esw_pkt_trace_init(int unit)
{
    UNIT_VALID_CHECK(unit);
    if (soc_feature(unit, soc_feature_visibility)) {
        if (_pkt_trace_initialized[unit]) {
            BCM_IF_ERROR_RETURN(_bcm_esw_pkt_trace_cleanup(unit));
        }
        if (_pkt_trace_mlock[unit] == NULL) {
            _pkt_trace_mlock[unit] = sal_mutex_create("packet trace mutex");
            if (_pkt_trace_mlock[unit] == NULL) {
                _bcm_esw_pkt_trace_cleanup(unit);
                return BCM_E_MEMORY;
            }
        }
        _pkt_trace_initialized[unit] = TRUE;

        BCM_IF_ERROR_RETURN(_bcm_esw_cpu_pkt_profile_init(unit));

        return BCM_E_NONE;
    }
    return BCM_E_UNAVAIL;    
}
#endif /* BCM_INSTRUMENTATION_SUPPORT */
