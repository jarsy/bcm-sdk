/*
 * This file is not auto-generated from the registers 
 * file.
 *
 * $Id: chips.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#include <soc/defs.h>
#include <soc/drv.h>
#include <soc/ea/tk371x/TkDefs.h>

int
soc_features_tk371x_a0(int unit, soc_feature_t feature)
{
   switch (feature) {
    case soc_feature_field:
        return TRUE;
    default:
        return FALSE;
    }
}

static soc_block_info_t soc_blocks_tk371x_a0[] = {
    { SOC_EA_BLK_PON,   0,  0,  3   },  /* 0 Pon */
    { SOC_EA_BLK_GE,    0,  0,  0   },  /* 1 Ge */
    { SOC_EA_BLK_FE,    1,  0,  1   },  /* 2 Fe */
    { SOC_EA_BLK_LLID,  2,  0,  2   },  /* 3 LLID */
    { -1,               -1, -1, -1  }   /* end */
};

static soc_port_info_t soc_ports_tk371x_a0[] = {
    { 0,      0   },    /* 0 Pon.0 */
    { 1,      1   },    /* 1 Ge.0 */
    { 2,      2   },    /* 2 FE.0 */
    { 3,      3   },    /* 3 Llid.0 */
    { 3,      3   },    /* 4 Llid.1 */
    { 3,      3   },    /* 5 Llid.2 */
    { 3,      3   },    /* 6 Llid.3 */
    { 3,      3   },    /* 7 Llid.4 */
    { 3,      3   },    /* 8 Llid.5 */
    { 3,      3   },    /* 9 Llid.6 */
    { 3,      3   },    /* 10 Llid.7 */
    { -1,    -1   } /* end */
};

soc_driver_t soc_driver_tk371x_a0 = {
    /* type                 */ SOC_CHIP_TK371X_A0,
    /* chip_string          */ "TK371X",
    /* origin               */ " Not located in Server Need Verified",
    /* pci_vendor           */ BROADCOM_VENDOR_ID,
    /* pci_device           */ TK371X_DEVICE_ID,
    /* pci_revision         */ TK371X_A0_REV_ID,
    /* num_cos              */ 0,
    /* reg_info             */ NULL,
    /* reg_unique_acc       */ NULL,
    /* reg_above_64_info    */ NULL,
    /* reg_array_info       */ NULL,
    /* mem_info             */ NULL,
    /* mem_unique_acc       */ NULL,
    /* soc_mem_t            */ NULL,
    /* mem_array_info       */ NULL,
    /* block_info           */ soc_blocks_tk371x_a0,
    /* port_info            */ soc_ports_tk371x_a0,
    /* counter_maps         */ NULL,
    /* features             */ soc_features_tk371x_a0,
    /* init                 */ NULL,
    /* services             */ NULL,
    /* port_num_blktype     */ 0,
    /* cmicd_base           */ 0x00000000
};  /* soc_driver */
