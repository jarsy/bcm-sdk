
/* $Id: stat.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        stat.h
 * Purpose:     stat internal definitions specific to Caladan3 BCM library
 */

#ifndef _BCM_INT_SBX_CALADAN3_STAT_H_
#define _BCM_INT_SBX_CALADAN3_STAT_H_

#ifdef BCM_CALADAN3_G3P1_SUPPORT

typedef enum caladan3_g3p1_counter_id_s {
    CALADAN3_G3P1_COUNTER_INGRESS,
    CALADAN3_G3P1_COUNTER_EGRESS,
    CALADAN3_G3P1_COUNTER_MAX
} caladan3_g3p1_counter_id_t;


#define BCM_CALADAN3_STAT_WITH_ID 1

/*
 * Function:
 *      _bcm_caladan3_stat_block_alloc
 * Description:
 *      Allocate counters from a statistics block
 * Parameters:
 *      unit      - device unit number.
 *      type      - one of the defined segment types
 *      count     - number of counters required
 *      start     - (OUT) where to put first of allocated counter block
 *      flags     - BCM_CALADAN3_STAT_WITH_ID indicates start value passed in.
 * Returns:
 *      BCM_E_NONE      - Success
 *      BCM_E_XXXX      - Failure
 */
int _bcm_caladan3_stat_block_alloc(int unit,
                                   int type,
                                   shr_aidxres_element_t *start,
                                   shr_aidxres_element_t count,
                                   uint32 flags);

/*
 * Function:
 *      _bcm_caladan3_stat_block_free
 * Description:
 *      Free counters from a statistics block
 * Parameters:
 *      unit      - device unit number.
 *      type      - one of the defined segment types
 *      start     - (OUT) where to put first of allocated counter block
 * Returns:
 *      BCM_E_NONE      - Success
 *      BCM_E_XXXX      - Failure
 */
int _bcm_caladan3_stat_block_free(int unit,
                                int type,
                                shr_aidxres_element_t start);


#endif /* BCM_CALADAN3_G3P1_SUPPORT */

#endif /* _BCM_INT_SBX_CALADAN3_STAT_H_ */

