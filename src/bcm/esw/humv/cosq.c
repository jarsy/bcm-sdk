/*
 * $Id: cosq.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * COS Queue Management
 * Purpose: API to set different cosq, priorities, and scheduler registers.
 */

#include <sal/core/libc.h>

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/debug.h>

#include <bcm/error.h>
#include <bcm/cosq.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/humv.h>

/* Currently identical to Bradley */
int _humv_cosq_not_empty;
