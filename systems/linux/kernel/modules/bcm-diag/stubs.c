/*
 * $Id: stubs.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <sal/appl/pci.h>
#include <appl/diag/sysconf.h>
#include <appl/diag/system.h>
#include <appl/stktask/attach.h>

/*
 * Satisfy shell.c w/o BCMX library
 */

int bcmx_unit_count;

#if defined(BROADCOM_DEBUG)
uint32 bcmx_debug_level;
char *bcmx_debug_names[] = {""};
#endif /*defined(BROADCOM_DEBUG)*/


/* 
 * Satisfy dispatch.c w/o stacking library
 */

int
bcm_stack_attach_register(bcm_stack_attach_cb_f callback)
{
    return 0;
}

int
bcm_stack_attach_unregister(bcm_stack_attach_cb_f callback)
{
    return 0;
}

/*
 * These stubs are here for legacy compatability reasons. 
 * They are used only by the diag/test code, not the driver, 
 * so they are really not that important. 
 */

void
pci_print_all(void)
{
}
