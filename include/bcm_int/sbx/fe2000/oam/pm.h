/*
 * $Id: pm.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef _BCM_INT_SBX_FE2000_OAM_PM_H_
#define _BCM_INT_SBX_FE2000_OAM_PM_H_


/* g2p3 ucode requires 2 consecutive counter ids for loss measurement.
 * The size is cut in half where 
 *   RxCounterIdx = id * 2
 *   TxCounterIdx = (id * 2) + 1
 */
#define OAM_NUM_COHERENT_COUNTERS   (8 * 1024)
#define OAM_COCO_BASE(x)    ((x) * 2)
#define OAM_COCO_RES(x)     ((x) / 2)
#define OAM_COCO_RX(x)      (x)
#define OAM_COCO_TX(x)      (OAM_COCO_RX(x) + 1)

#define OAM_COCO_VALID(x)     ((x) < OAM_NUM_COHERENT_COUNTERS)
#define OAM_COCO_INVAID_ID     OAM_NUM_COHERENT_COUNTERS


#define SUPPORTED_DELAY_FLAGS (BCM_OAM_DELAY_ONE_WAY               | \
                               BCM_OAM_DELAY_TX_ENABLE             | \
                               BCM_OAM_DELAY_ALL_RX_COPY_TO_CPU    | \
                               BCM_OAM_DELAY_FIRST_RX_COPY_TO_CPU  | \
                               BCM_OAM_DELAY_WITH_ID)

#define SUPPORTED_LOSS_FLAGS  (BCM_OAM_LOSS_TX_ENABLE              | \
                               BCM_OAM_LOSS_SINGLE_ENDED           | \
                               BCM_OAM_LOSS_FIRST_RX_COPY_TO_CPU   | \
                               BCM_OAM_LOSS_ALL_RX_COPY_TO_CPU     | \
                               BCM_OAM_LOSS_WITH_ID                | \
                               BCM_OAM_LOSS_COUNT_GREEN_AND_YELLOW | \
                               BCM_OAM_LOSS_COUNT_POST_TRAFFIC_CONDITIONING)

extern int _oam_g2p3_lm_create(int unit, 
                               int flags,
                               uint16 lmIdx, 
                               uint16 epIdx, 
                               int cocoIdx,
                               int multId,
                               int oameptype);

extern int _oam_g2p3_coco_configure(int unit,
                                    oam_sw_hash_data_t *ep_data,
                                    int coco_idx);

extern int _oam_g2p3_dm_create(int unit, 
                               int flags, 
                               uint16 dmIdx, 
                               uint16 epIdx, 
                               uint32 ftIdx);

extern int convert_ep_to_time_spec(bcm_time_spec_t* bts,
                                   int sec,
                                   int ns);

extern int time_spec_subtract(bcm_time_spec_t* d,
                              bcm_time_spec_t* m,
                              bcm_time_spec_t* s);

#endif  /* _BCM_INT_SBX_FE2000_OAM_PM_H_  */
