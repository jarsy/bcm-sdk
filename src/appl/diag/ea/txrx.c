/*
 * $Id: txrx.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * socdiag tx (transmit) and rx (receive) commands
 */

#include <sal/core/libc.h>

#include <soc/types.h>
#include <soc/debug.h>
#include <soc/cm.h>

#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/net.h>
#include <linux/in.h>
#else
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#endif

#ifdef BCM_ROBO_SUPPORT
#include <soc/robo/mcm/driver.h>
#endif /* BCM_ROBO_SUPPORT */
#include <soc/dma.h>
#include <sal/types.h>
#include <sal/appl/sal.h>
#include <sal/appl/io.h>
#include <sal/core/thread.h>

#include <bcm/tx.h>
#include <bcm/pkt.h>
#include <bcm_int/robo/rx.h>
#include <bcm/port.h>
#include <bcm/error.h>

#include <appl/diag/system.h>

/*
 * Function:
 * Purpose:
 * Parameters:
 * Returns:
 */
cmd_result_t
cmd_ea_tx(int u, args_t *a)
{
	return CMD_OK;
}

/*
 * Function:    tx_count
 * Purpose: Print out status of any currently running TX command.
 * Parameters:  u - unit number.
 *      a - arguments, none expected.
 * Returns: CMD_OK
 */
cmd_result_t
cmd_ea_tx_count(int u, args_t *a)
{
	return CMD_OK;
}

/*
 * Function:    tx_start
 * Purpose: Start off a background TX thread.
 * Parameters:  u - unit number
 *      a - arguments.
 * Returns: CMD_XXX
 */
cmd_result_t
cmd_ea_tx_start(int u, args_t *a)
{
	return CMD_OK;
}


/*
 * Function:    tx_stop
 * Purpose: Stop a currently running TX command
 * Parameters:  u - unit number.
 *      a - arguments (none expected).
 * Returns: CMD_OK/CMD_USAGE/CMD_FAIL
 */
cmd_result_t
cmd_ea_tx_stop(int u, args_t *a)
{
	return CMD_OK;
}


cmd_result_t
cmd_ea_rx_cfg(int unit, args_t *args)
/*
 * Function:    rx
 * Purpose:     Perform simple RX test
 * Parameters:  unit - unit number
 *              args - arguments
 * Returns:     CMD_XX
 */
{
	return CMD_OK;
}


/****************************************************************
 *
 * RX commands
 *
 ****************************************************************/
cmd_result_t
cmd_ea_rx_init(int unit, args_t *args)
{
	return CMD_OK;
}
