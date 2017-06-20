/*
 * $Id: vlan.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#include <bcm/error.h>
#include <bcm/vlan.h>
#include <bcm/l3.h>
#include <bcm/debug.h>
#include <bcm_int/robo/port.h>
#include <bcm_int/common/multicast.h>

#include <soc/mem.h>

cmd_result_t
if_ea_pvlan(int u, args_t *a)
{
	return CMD_OK;
}
