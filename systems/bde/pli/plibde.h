/*
 * $Id: plibde.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef __PLIBDE_H__
#define __PLIBDE_H__

#include <ibde.h>

extern int
plibde_create(ibde_t** bde);

#ifdef CMODEL_SERVER_MODE
extern int
plibde_cmodel_create(ibde_t** bde);
#endif

#ifdef SINGLE_MODE
extern int
plibde_local_create(ibde_t** bde);
#endif

#endif /* __PLIBDE_H__ */
