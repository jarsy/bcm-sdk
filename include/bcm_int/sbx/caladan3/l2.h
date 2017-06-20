/*
 * $Id: l3.h,v 1.18 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        l2.h
 * Purpose:     L2 internal definitions.
 */

#ifndef _BCM_INT_SBX_CALADAN3_L2_H_
#define _BCM_INT_SBX_CALADAN3_L2_H_

/* Information needed by L2 aging */
typedef struct _ageid_to_mac_info_s {
    soc_sbx_g3p1_6_byte_t       mac;
    int                         vid;
    uint8                    dontage;
    uint8                      valid;
} _ageid_to_mac_info_t;

/* Age indexes are maintained on a stack */
typedef struct _age_id_stack_s {
    uint32                     p;     /* Location of current top of stack */
    uint32                  size;     /* used for range checking */
    uint32          *age_indexes;     /* actual indexes stored here in an array impl */
    sal_mutex_t             lock;     /* protect the index stack */
} _age_id_stack_t;


#endif /* _BCM_INT_SBX_CALADAN3_L2_H_ */
