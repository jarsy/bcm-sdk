/*
 * $Id: assert.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <sal/core/boot.h>

#include <vxWorks.h>
#include <logLib.h>
#include <sysLib.h>
#include <usrLib.h>
#include <vmLib.h>
#include <vxLib.h>

#include <stdio.h>
#include <sal/core/spl.h>

/*
 * Function:
 *	_default_assert
 * Purpose:
 *	Default assertion routine used if not assigned by
 *	a call to sal_assert_set().
 */

void
_default_assert(const char *expr, const char *file, int line)
{
    extern void sysReboot(void);
    int			int_ctxt;

    printf("ERROR: Assertion failed: (%s) at %s:%d\n", expr, file, line);
  
    int_ctxt = sal_int_context();

    sysReboot();
}

/*
 * Function:
 *	sal_assert_set
 * Purpose:
 *	Set new assertion handler routine.
 * Parameters:
 *	f - assertion handler function
 */

static sal_assert_func_t _assert = _default_assert;

void
sal_assert_set(sal_assert_func_t f)
{
    _assert = (f != NULL) ? f : _default_assert;
}

/*
 * Function:
 *	_sal_assert
 * Purpose:
 *	Main assertion routine
 * Notes:
 *	Normally #define'd to assert().
 */

/* coverity[+kill] */
void
_sal_assert(const char *expr, const char *file, int line)
{
    (*_assert)(expr, file, line);
}
