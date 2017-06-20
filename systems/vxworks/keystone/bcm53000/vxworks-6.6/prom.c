/* $Id: prom.c,v 1.3 2011/07/21 16:14:28 yshtil Exp $
 *  Copyright 2009, Broadcom Corporation
 *   All Rights Reserved.

 *  This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 *  the contents of this file may not be disclosed to third parties, copied or
 *  duplicated in any form, in whole or in part, without the prior written
 *  permission of Broadcom Corporation.
 */

#include "typedefs.h"
#include "bcmendian.h"
#include "cfe_api.h"

/*
 * OS needs to store a0~a3 to these variables at its entry point
 *
 * Note: These variables are initialized with non-zero values
 *       since OS could overwrite them if they are in BSS section.
 */
uint32 fw_arg0 = -1;
uint32 fw_arg1 = -1;
uint32 fw_arg2 = -1;
uint32 fw_arg3 = -1;

void
prom_init(void)
{
	uint32_t cfe_ept, cfe_handle;
	unsigned int cfe_eptseal;
	int argc = fw_arg0;
	char **envp = (char **) fw_arg2;
	int *prom_vec = (int *) fw_arg3;
	
	/*
	 * Check if a loader was used; if NOT, the 4 arguments are
	 * what CFE gives us (handle, 0, EPT and EPTSEAL)
	 */
	cfe_handle = (uint32_t)(long)argc;
	cfe_ept = (long)envp;
	cfe_eptseal = (uint32_t)(unsigned long)prom_vec;
	if (cfe_eptseal != CFE_EPTSEAL) {
		/* too early for panic to do any good */
		return;
	}
	cfe_init(cfe_handle, cfe_ept);
}
