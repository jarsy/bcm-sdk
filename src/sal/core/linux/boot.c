/*
 * $Id: boot.c,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: 	boot.c
 * Purpose:	Kernel initialization
 */

#include <sal/core/boot.h>
#include <sal/core/thread.h>
#include <sal/core/dpc.h>
#include <sal/core/sync.h>
#include <lkm.h>

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
 *	Flags are set for PLISIM and NO_PROBE by default, and can be
 *	overridden by setting the environment variable SOC_BOOT_FLAGS.
 * Parameters:
 *	None
 * Returns:
 *	32-bit flags value, 0 if not supported or no flags set.
 */

static uint32 _sal_boot_flags = BOOT_F_NO_RC;

uint32
sal_boot_flags_get(void)
{
    /* This function should probably never be needed, 
     * but if it is we should turn off these things */
    return _sal_boot_flags;
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
    _sal_boot_flags = flags;
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
    return "Linux Kernel"; 
}
