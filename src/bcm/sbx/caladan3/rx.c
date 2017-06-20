/*
 * $Id: rx.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        rx.c
 */


#ifdef BCM_CALADAN3_SUPPORT


#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <bcm/error.h>
#include <bcm/rx.h>
#include <bcm/switch.h>
#include <bcm/stack.h>
#include <bcm_int/sbx/rx.h>
#include <bcm_int/sbx/error.h>
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_defs.h>

#define _EXC_ARRAY_SIZE  15


int
sbx_caladan3_rx_config(int unit, bcm_rx_cfg_t *cfg)
{
    int rv = BCM_E_NONE;

    if (cfg->pkt_size != RX_PKT_SIZE_DFLT )
        rv = BCM_E_UNAVAIL;

    return rv;
}



#ifdef BCM_CALADAN3_G3P1_SUPPORT

/*
 *  Function:
 *    _bcm_caladan3_g3p1_rx_exc_base_qid_get
 *  Purpose:
 *    Get the base queue for all exceptions
 *  Parameters:
 *    (in)  unit   - bcm device number
 *    (out) baseQid - storage for baseQid
 */
int
_bcm_caladan3_g3p1_rx_exc_base_qid_get(int unit, int *baseQid)
{
    int rv;
    int gport;
    uint32 qePort, qeMod, qeNode;

    rv = bcm_switch_control_get(unit, bcmSwitchCpuCopyDestination, &gport);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    qeMod = BCM_GPORT_MODPORT_MODID_GET(gport);
    qePort = BCM_GPORT_MODPORT_PORT_GET(gport);

    /* check for switch gport*/
    if (!BCM_STK_MOD_IS_NODE(qeMod)) {
        int temp;
        if (qePort < SBX_MAX_PORTS && qeMod < SBX_MAX_MODIDS) {
            temp = SOC_SBX_CONTROL(unit)->modport[qeMod][qePort];
            qePort = temp & 0xffff;
            qeMod = (temp >> 16) & 0xffff;
        } else {
            return BCM_E_PARAM;
        }
    }

    qeNode = BCM_STK_MOD_TO_NODE(qeMod);
    *baseQid = SOC_SBX_NODE_PORT_TO_QID(unit,qeNode,
                                        qePort, NUM_COS(unit));
    LOG_INFO(BSL_LS_BCM_RX,
             (BSL_META_U(unit,
                         "Found baseQid=0x%04x, qeNode=%d qePort=%d\n"),
              *baseQid, qeNode, qePort));

    return rv;
}


/*
 * Function:
 *   _bcm_caladan3_g3p1_rx_cosq_mapping_set
 * Purpose:
 *   Set the exception destination priority queue for the given set of reasons
 * Parameters:
 *   (in)   unit     - bcm device number
 *   (in)   reasons  - mappable bcm reason to sbx exceptions to update
 *   (in)   int_prio - bcm priority (7=high pri, 0=low pri)
*/
int
_bcm_caladan3_g3p1_rx_cosq_mapping_set(int unit,
                                     bcm_rx_reasons_t reasons,
                                     uint8 int_prio)

{
    int                rv, rtn_rv;
    bcm_rx_reason_t    reasonIdx;
    int                excCount, excIdx;
    int                baseQid, qeCos;
    uint32           mappedExcs[_EXC_ARRAY_SIZE];
    soc_sbx_g3p1_xt_t  xt;

    rtn_rv = BCM_E_NONE;

    for (reasonIdx = 0; reasonIdx < bcmRxReasonCount; reasonIdx++) {

        rv = BCM_E_NONE;
        excCount = _EXC_ARRAY_SIZE;
        if (BCM_RX_REASON_GET(reasons, reasonIdx)) {
            rv = _bcm_to_sbx_reasons(unit, reasonIdx, mappedExcs, &excCount);

            if (BCM_FAILURE(rv) || excCount == 0) {
                rtn_rv = BCM_E_PARAM;
                LOG_WARN(BSL_LS_BCM_RX,
                         (BSL_META_U(unit,
                                     "error mapping reason %d to exception: %d %s\n"),
                          reasonIdx, rv, bcm_errmsg(rv)));
                /* Keep going, don't return */
            } else {

                /* get the base Exception Queue Id - */
                rv = _bcm_caladan3_g3p1_rx_exc_base_qid_get(unit, &baseQid);
                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_RX,
                              (BSL_META_U(unit,
                                          "Failed to get base qid: %d %s\n"),
                               rv, bcm_errmsg(rv)));
                    return rv;
                }
                LOG_INFO(BSL_LS_BCM_RX,
                         (BSL_META_U(unit,
                                     "Found baseQid=0x%04x\n"),
                          baseQid));

                /* for each mapped exception index, update the qid to reflect the
                 * new COS.  Note that the QE's highest priority is 0, while
                 * the highest COS value is 7
                 */
                qeCos = (SBX_MAX_COS - 1) - int_prio;
                for (excIdx = 0; excIdx < excCount; excIdx++) {
                    LOG_VERBOSE(BSL_LS_BCM_RX,
                                (BSL_META_U(unit,
                                            "Updating exception %d to QID=0x%04x "
                                             "cos/prio=%d/%d\n"),
                                 mappedExcs[excIdx], baseQid + qeCos,
                                 qeCos, int_prio));


                    rv = soc_sbx_g3p1_xt_get(unit, mappedExcs[excIdx], &xt);
                    if (BCM_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_BCM_RX,
                                  (BSL_META_U(unit,
                                              "failed to read exception %d: %d %s\n"),
                                   mappedExcs[excIdx], rv, bcm_errmsg(rv)));
                        return rv;
                    }

                    xt.qid = baseQid + qeCos;

                    rv = soc_sbx_g3p1_xt_set(unit, mappedExcs[excIdx], &xt);
                    if (BCM_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_BCM_RX,
                                  (BSL_META_U(unit,
                                              "failed to write exception %d: %d %s\n"),
                                   mappedExcs[excIdx], rv, bcm_errmsg(rv)));
                        return rv;
                    }
                }
            }
        }
    }

    return rtn_rv;
}


/*
 *  Function:
 *    _bcm_caladan3_rx_cosq_mapping_get
 *  Purpose:
 *    Get the exception destination priority queue for ONE reason
 *  Parameters:
 *    (in)  unit        - bcm device number
 *    (in)  reasons     - mappable reasons to sbx exceptions update
 *    (out) int_prio    - bcm priority (7=high, 0=low)
 */
int
_bcm_caladan3_g3p1_rx_cosq_mapping_get(int unit,
                                     bcm_rx_reasons_t *reasons,
                                     uint8 *int_prio)
{
    int                rv;
    bcm_rx_reason_t    reasonIdx;
    int                excCount;
    int                baseQid;
    uint32           mappedExcs[_EXC_ARRAY_SIZE];
    soc_sbx_g3p1_xt_t  xt;

    for (reasonIdx = 0; reasonIdx < bcmRxReasonCount; reasonIdx++) {

        rv = BCM_E_NONE;
        excCount = _EXC_ARRAY_SIZE;
        if (BCM_RX_REASON_GET(*reasons, reasonIdx)) {
          rv = _bcm_to_sbx_reasons(unit, reasonIdx, mappedExcs, &excCount);


            if (BCM_FAILURE(rv) || excCount == 0) {
                LOG_WARN(BSL_LS_BCM_RX,
                         (BSL_META_U(unit,
                                     "error mapping reason %d to exception: %d %s\n"),
                          reasonIdx, rv, bcm_errmsg(rv)));
                /* Keep going, don't return */
            } else {

                /* get the base Exception Queue Id - */
                rv = _bcm_caladan3_g3p1_rx_exc_base_qid_get(unit, &baseQid);
                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_RX,
                              (BSL_META_U(unit,
                                          "Failed to get base qid: %d %s\n"),
                               rv, bcm_errmsg(rv)));
                    return rv;
                }
                LOG_INFO(BSL_LS_BCM_RX,
                         (BSL_META_U(unit,
                                     "Found baseQid=0x%04x\n"),
                          baseQid));

                /* Since all mapped reasons will have same cos, only need to
                 * read the first one; then return because this routine only
                 * supports one reason for the _get case
                 */

                rv = soc_sbx_g3p1_xt_get(unit, mappedExcs[0], &xt);
                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_RX,
                              (BSL_META_U(unit,
                                          "failed to read exception %d: %d %s\n"),
                               mappedExcs[0], rv, bcm_errmsg(rv)));
                    return rv;
                }

                /* convert the QID to a QeCos, then to a priority */
                *int_prio = (baseQid - xt.qid) + (SBX_MAX_COS - 1);
                return rv;
            }
        }
    }

    return rv;
}




#endif /* #ifdef BCM_CALADAN3_G3P1_SUPPORT */



/*
 *  Function:
 *    bcm_caladan3_rx_cosq_mapping_set
 *  Purpose:
 *    Set the exception destination priority queue for the given set of reasons
 *  Parameters:
 *    (in)  unit        - bcm device number
 *    (in)  reasons     - mappable reasons to sbx exceptions update
 *    (in)  reasons_mask - ignored
 *    (in)  int_prio    - bcm priority (7=high, 0=low)
 *    (in)  int_prio_mask- ignored
 *    (in)  packet_type  - ignored
 *    (in)  packet_typemask- ignored
 *    (in)  cosq         - ignored
 */

int
bcm_caladan3_rx_cosq_mapping_set(int unit,
                               int index,
                               bcm_rx_reasons_t reasons,
                               bcm_rx_reasons_t reasons_mask,
                               uint8 int_prio,
                               uint8 int_prio_mask,
                               uint32 packet_type,
                               uint32 packet_type_mask,
                               bcm_cos_queue_t cosq)
{
    int rv = BCM_E_INTERNAL;

    if (int_prio > 7) {
        return BCM_E_PARAM;
    }

    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_rx_cosq_mapping_set(unit, reasons, int_prio);
        break;
#endif /* BCM_CALADAN3_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
    }

   return rv;
}



/*
 *  Function:
 *    bcm_caladan3_rx_cosq_mapping_get
 *  Purpose:
 *    Get the exception destination priority queue for ONE reason
 *  Parameters:
 *    (in)  unit        - bcm device number
 *    (in)  reasons     - mappable reasons to sbx exceptions update
 *    (in)  reasons_mask - ignored
 *    (out)  int_prio    - bcm priority (7=high, 0=low)
 *    (in)  int_prio_mask- ignored
 *    (in)  packet_type  - ignored
 *    (in)  packet_typemask- ignored
 *    (in)  cosq         - ignored
 */
int
bcm_caladan3_rx_cosq_mapping_get(int unit,
                               int index,
                               bcm_rx_reasons_t *reasons,
                               bcm_rx_reasons_t *reasons_mask,
                               uint8 *int_prio,
                               uint8 *int_prio_mask,
                               uint32 *packet_type,
                               uint32 *packet_type_mask,
                               bcm_cos_queue_t *cosq)
{
    int rv = BCM_E_INTERNAL;

    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_rx_cosq_mapping_get(unit, reasons, int_prio);
        break;
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
    }

    return rv;
}

/*  
 * Function:
 *      bcm_rx_queue_channel_get
 * Purpose:
 *      Get the associated rx channel with a given cosq
 * Parameters:
 *      unit - Unit reference
 *      queue_id - CPU cos queue index (0 - (max cosq - 1)) 
 *      chan_id - channel index (0 - (BCM_RX_CHANNELS-1))
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_caladan3_rx_queue_channel_get(int unit, bcm_cos_queue_t queue_id, 
                             bcm_rx_chan_t *chan_id)
{
    return (_bcm_common_rx_queue_channel_get(unit, queue_id, 
	  				     chan_id));
}

/*  
 *  Function:
 *    bcm_rx_queue_channel_set
 *  Purpose:
 *    Assign a RX channel to a cosq 
 *  Parameters:
 *    unit - Unit reference
 *    queue_id - CPU cos queue index (0 - (max cosq - 1)) 
 *                                        (Negative for all)
 *    chan_id - channel index (0 - (BCM_RX_CHANNELS-1))
 *  Returns:
 *    BCM_E_XXX
 * 
 */
int
bcm_caladan3_rx_queue_channel_set (int unit, bcm_cos_queue_t queue_id, 
                              bcm_rx_chan_t chan_id)
{

#ifdef BCM_CMICM_SUPPORT
    if (chan_id >= BCM_RX_CHANNELS) {
        /* API access is constrained to only the PCI host channels. */
        return BCM_E_PARAM;
    }
#endif

    return (_bcm_common_rx_queue_channel_set(unit, queue_id, 
	  				     chan_id));
}

#endif
