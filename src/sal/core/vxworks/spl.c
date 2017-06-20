/*
 * $Id: spl.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: 	spl.c
 * Purpose:	Interrupt Blocking
 */

#include <vxWorks.h>
#include <sysLib.h>
#include <intLib.h>

static int _int_locked = 0;

/*
 * Function:
 *	sal_spl
 * Purpose:
 *	Set interrupt level.
 * Parameters:
 *	level - interrupt level to set.
 * Returns:
 *	Value of interrupt level in effect prior to the call.
 * Notes:
 *	Used most often to restore interrupts blocked by sal_splhi.
 */

int
sal_spl(int level)
{
    int	il;
    _int_locked--;
#if VX_VERSION >= 64
    intUnlock(level);
    il = 0;
#else
    il = intUnlock(level);
#endif
    return il;
}

/*
 * Function:
 *	sal_splhi
 * Purpose:
 *	Set interrupt mask to highest level.
 * Parameters:
 *	None
 * Returns:
 *	Value of interrupt level in effect prior to the call.
 */

int
sal_splhi(void)
{
    int il;
    il = intLock();
    _int_locked++;
    return (il);
}

/*
 * Function:
 *	sal_int_locked
 * Purpose:
 *	Return TRUE if interrupts have been blocked via splhi.
 * Parameters:
 *	None
 * Returns:
 *	Boolean
 */

int
sal_int_locked(void)
{
    return _int_locked;
}

/*
 * Function:
 *	sal_int_context
 * Purpose:
 *	Return TRUE if running in interrupt context.
 * Parameters:
 *	None
 * Returns:
 *	Boolean
 */

int
sal_int_context(void)
{
    return INT_CONTEXT();
}

/*
 * Function:
 *	sal_no_sleep
 * Purpose:
 *	Return TRUE if current context cannot sleep.
 * Parameters:
 *	None
 * Returns:
 *	Boolean
 */

int
sal_no_sleep(void)
{
    return sal_int_locked();
}
