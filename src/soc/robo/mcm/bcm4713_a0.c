/*
 * In strata Switch, this file is auto-generated from the
 * registers file. But not the same with Strata Switch,
 * BCM47XX has no such generator/tool to prepare this
 *
 * $Id: bcm4713_a0.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    bcm4713_a0.i
 * Purpose: bcm4713_a0 chip specific information (register)
 *
 * Note :
 */


#include <sal/core/libc.h>
#include <soc/defs.h>
#include <soc/mem.h>
#include <soc/robo/mcm/driver.h>
#include <soc/register.h>
#include <soc/robo/mcm/allfields.h>
#include <soc/counter.h>
#include <soc/memory.h>


/* Forward declaration of init function */
static void chip_init_bcm4713_a0(void);

static soc_block_info_t soc_blocks_bcm4713_a0[] = {
    { -1,        -1,    -1,    -1    }    /* end */
};

soc_driver_t soc_driver_bcm4713_a0 = {
    /* type                 */ SOC_CHIP_BCM4713_A0,
    /* chip_string          */ "BCM4713",
    /* origin               */ "HND",
    /* pci_vendor           */ BROADCOM_VENDOR_ID,
    /* pci_device           */ BCM4713_DEVICE_ID,
    /* pci_revision         */ BCM4713_A0_REV_ID,
    /* num_cos              */ 0,
    /* reg_info             */ NULL,
    /* reg_unique_acc       */ NULL,
    /* reg_above_64_info    */ NULL,
    /* reg_array_info       */ NULL,
    /* mem_info             */ NULL,
    /* mem_unique_acc       */ NULL,
    /* mem_aggr             */ NULL,
    /* mem_array_info       */ NULL,
    /* block_info           */ soc_blocks_bcm4713_a0,
    /* port_info            */ NULL,
    /* counter_maps         */ NULL,
    /* features             */ NULL,
    /* init                 */ chip_init_bcm4713_a0,
    /* services             */ NULL,
    /* port_num_blktype     */ 0,
    /* cmicd_base           */ 0x00000000
};  /* soc_driver */

/* Chip specific bottom matter from memory file */

/****************************************************************
 *
 * Function:    chip_init_bcm5338_a0
 * Purpose:
 *     Initialize software internals for bcm5690_a0 device:
 *         Initialize null memories
 * Parameters:  void
 * Returns:     void
 *
 ****************************************************************/
static void
chip_init_bcm4713_a0(void)
{

}

/* End of chip specific bottom matter */
