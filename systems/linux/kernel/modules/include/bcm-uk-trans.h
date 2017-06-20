/*
 * $Id: bcm-uk-trans.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Provides a BCM transport driver using a User/Kernel proxy service
 */

#ifndef __BCM_UK_TRANS_H__
#define __BCM_UK_TRANS_H__

#include <bcm/rx.h>

extern int bcm_uk_trans_create(const char* service, int port, bcm_trans_ptr_t** trans); 

#endif /* __BCM_UK_TRANS_H__ */
