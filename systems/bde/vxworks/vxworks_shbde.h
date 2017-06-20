/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef __VXWORKS_SHBDE_H__
#define __VXWORKS_SHBDE_H__

#include <shbde.h>
#include <shbde_pci.h>
#include <shbde_iproc.h>

extern int
vxworks_shbde_hal_init(shbde_hal_t *shbde, shbde_log_func_t log_func);

#endif /* __VXWORKS_SHBDE_H__ */
