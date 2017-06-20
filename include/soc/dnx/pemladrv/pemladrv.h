/** \file dnx/pemladrv/pemladrv.h
 * 
 * PEMLA related functions that should be exposed
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef PEMLADRV_INCLUDED
/*
 * { 
 */
#define PEMLADRV_INCLUDED

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

int pemladrv_init(
    const int restore_after_reset,
    const char *file_name);

/*
 * } 
 */
#endif /* PEMLADRV_INCLUDED */
