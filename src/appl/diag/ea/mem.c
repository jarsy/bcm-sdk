/*
 * $Id: mem.c,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * socdiag memory commands
 */

#include <sal/core/libc.h>
#include <sal/appl/sal.h>
#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/error.h>
#include <appl/diag/ea/ea.h>
#include <appl/diag/system.h>

cmd_result_t
cmd_ea_mem_list(int unit, args_t *a)
{
	return CMD_FAIL;
}
