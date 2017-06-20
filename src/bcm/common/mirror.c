
/*
 * $Id: mirror.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    mirror.c
 * Purpose: Common Mirror API
 */

#include <soc/defs.h>

#include <sal/core/libc.h>

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/util.h>
#include <soc/debug.h>

#include <bcm/mirror.h>

/*
 * Function:
 *      bcm_mirror_destination_t_init
 * Purpose:
 *      Initialize a mirror destination structure
 * Parameters:
 *      mirror_dest - pointer to the bcm_mirror_destination_t structure.
 * Returns:
 *      NONE
 */
void
bcm_mirror_destination_t_init(bcm_mirror_destination_t *mirror_dest)
{
    if (NULL != mirror_dest) {
        sal_memset (mirror_dest, 0, sizeof(bcm_mirror_destination_t));
        mirror_dest->stat_id2 = -1;
    }
    return;
}

/*
 * Function:
 *      bcm_mirror_port_info_t_init
 * Purpose:
 *      Initialize a mirror port info structure
 * Parameters:
 *      mirror_info - pointer to the bcm_mirror_port_info_t structure.
 * Returns:
 *      NONE
 */
void
bcm_mirror_port_info_t_init(bcm_mirror_port_info_t *info)
{
    if (NULL != info) {
        sal_memset (info, 0, sizeof(bcm_mirror_port_info_t));
    }
    return;
}

/*
 * Function:
 *      bcm_mirror_options_t_init
 * Purpose:
 *      Initialize a mirror options  structure
 * Parameters:
 *      options - pointer to the bcm_mirror_options_t structure.
 * Returns:
 *      NONE
 */
void
bcm_mirror_options_t_init(bcm_mirror_options_t *options)
{
    if (NULL != options) {
        sal_memset (options, 0, sizeof(bcm_mirror_options_t));
        /* Enable mirroring by default*/
        options->forward_strength = 1;
        options->copy_strength = 1 ;
    }

    return;
}

/*
 * Function:
 *      bcm_mirror_pkt_header_updates_t_init
 * Purpose:
 *      Initialize a mirror pkt header updates  structure
 * Parameters:
 *      updates - pointer to the bcm_mirror_pkt_header_updates_t structure.
 * Returns:
 *      NONE
 */
void
bcm_mirror_pkt_header_updates_t_init(bcm_mirror_pkt_header_updates_t  *updates)
{
    if (NULL != updates) {
        sal_memset (updates, 0, sizeof(bcm_mirror_pkt_header_updates_t));
    }
    return;
}

