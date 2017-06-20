/*
 * $Id: ltm.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef _BCM_INT_SBX_FE2000_OAM_LTM_H_
#define _BCM_INT_SBX_FE2000_OAM_LTM_H_


#define _OAM_LTM_INVALID_ID  (~0)

/*
 *   Function
 *      _bcm_ltm_init
 *   Purpose
 *      Initialize the loss threshold table manager
 *   Parameters
 *       unit           = BCM device number
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int _bcm_ltm_init(int unit);

/*
 *   Function
 *      _bcm_ltm_cleanup
 *   Purpose
 *      Cleanup resources associated with the loss threshold table manager
 *   Parameters
 *       unit           = BCM device number
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int _bcm_ltm_cleanup(int unit);

/*
 *   Function
 *      _bcm_ltm_threshold_alloc
 *   Purpose
 *      Allocate  loss threshold table entry for use by loss managment oam
 *   Parameters
 *       unit           = BCM device number
 *       loss_threshold = loss threshold to allocate in 100ths of a percent
 *       id             = loss threshold table index allocated
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int _bcm_ltm_threshold_alloc(int unit, int loss_threshold, int *id);

/*
 *   Function
 *      _bcm_ltm_free
 *   Purpose
 *      Return a loss threshold table index to the available pool.
 *   Parameters
 *       unit           = BCM device number
 *       id             = id to free
 *   Returns
 *       BCM_E_*
 *  Notes:
 */
int _bcm_ltm_threshold_free(int unit, int id);



#endif  /* _BCM_INT_SBX_FE2000_LTM_H_  */
