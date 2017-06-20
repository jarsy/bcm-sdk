/*
 * $Id: boot.c,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: 	boot.c
 * Purpose:	Kernel initialization
 */

#include <sal/core/boot.h>

#include <vxWorks.h>
#include <logLib.h>
#include <sysLib.h>
#include <usrLib.h>
#include <vmLib.h>
#include <vxLib.h>
#include <version.h>

#include <stdio.h>
#include <sal/core/spl.h>
#include <sal/core/dpc.h>
#include <sal/core/sync.h>

/*
 * Function:
 *	sal_core_init
 * Purpose:
 *	Initialize the Kernel SAL
 * Returns:
 *	0 on success, -1 on failure
 */

int
sal_core_init(void)
{
    sal_udelay(0);	/* Cause sal_udelay() to self-calibrate */

    sal_thread_main_set(sal_thread_self());

    if (sal_dpc_init()) {
        return -1;
    }

    if (sal_global_lock_init()) {
        return -1;
    }

    return 0;
}

/*
 * Function:
 *	sal_boot_flags_get
 * Purpose:
 *	Return boot flags from startup
 * Parameters:
 *	None
 * Returns:
 *	32-bit flags value, 0 if not supported or no flags set.
 */

uint32
sal_boot_flags_get(void)
{
    extern	BOOT_PARAMS	sysBootParams;

    return ((uint32)sysBootParams.flags);
}

/*
 * Function:
 *	sal_boot_flags_set
 * Purpose:
 *	Change boot flags
 * Parameters:
 *	flags - New boot flags
 */

void
sal_boot_flags_set(uint32 flags)
{
    extern	BOOT_PARAMS	sysBootParams;

    sysBootParams.flags = flags;
}

/*
 * Function:
 *	sal_boot_script
 * Purpose:
 *	Return name of boot script from startup
 * Parameters:
 *	None
 * Returns:
 *	Name of boot script, NULL if none
 */

char *
sal_boot_script(void)
{
    extern	BOOT_PARAMS	sysBootParams;

    if (sysBootParams.startupScript[0] != 0) {
	return sysBootParams.startupScript;
    }

    return NULL;
}

/*
 * Function:
 *      sal_os_name
 * Purpose:
 *      Provide a description of the underlying operating system
 * Parameters:
 *      None
 * Returns:
 *      String describing the OS
 */
const char* 
sal_os_name(void)
{
# ifdef RUNTIME_VERSION
    return "VxWorks " RUNTIME_VERSION; 
# else
    return "VxWorks " VXWORKS_VERSION;     
# endif
}

