/*
 * $Id: brd_sbx.h,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SBX Reference Boards
 */

#ifndef __BRD_SBX_H__
#define __BRD_SBX_H__

/*
 * Board error codes
 */
typedef enum {
    BOARD_E_NONE,
    BOARD_E_PARAM,
    BOARD_E_FAIL
} brd_error_t;

typedef enum brd_sbx_type_e {
    BOARD_TYPE_METROCORE = 1,     /* Metrocore Linecard */
    BOARD_TYPE_METROCORE_FABRIC,  /* Metrocore Fabric card */
    BOARD_TYPE_LCMODEL,           /* BCMSIM */
    BOARD_TYPE_POLARIS_LC,        /* FE2K LC with Polaris LCMs */
    BOARD_TYPE_POLARIS_FC,        /* Polaris fabric card */
    BOARD_TYPE_FE2KXT_QE2K_POLARIS_LC, /* FE2KXT,QE2K  LC with Polaris LCM */
    BOARD_TYPE_FE2KXT_4X10G_QE2K_POLARIS_LC, /* FE2KXT 4x10G */
    BOARD_TYPE_SIRIUS_SIM = 8,    /* Sirius - single chip SIM */
    BOARD_TYPE_QE2K_BSCRN_LC = 9, /* QE2K - Dual Chip Benchscreen Board. */
    BOARD_TYPE_SIRIUS_IPASS = BOARD_TYPE_SIRIUS_SIM,  /* Sirius Ipass board */
    BOARD_TYPE_POLARIS_IPASS = 11, /* Polaris Ipass board */
    /* Must be the last */
    BOARD_TYPE_MAX
} brd_sbx_type_t;

/*
 *  FPGA Offsets
 */

#define FPGA_BASE  (0x10000)
#define FPGA_PL_BASE  (0x40000)
#define FPGA_MASTER_REG_OFFSET  0x14
#define FPGA_MASTER_MODE_BIT    0x10
#define FPGA_SCI_ROUTING_OFFSET 0x19
#define FPGA_SCI_TO_LCM         0xA0
#define FPGA_SCI_TO_BP          0x50
#define FPGA_FE2K_DLL_ENABLE_OFFSET 0x1a
#define FPGA_FE2K_DLL_ENABLE    0x03
#define FPGA_SLOT_ID_OFFSET     0x11
#define FPGA_ID_OFFSET 0x0
#define FPGA_REVISION_OFFSET  0x1
#define FPGA_BOARD_ID_OFFSET    0x2
#define FPGA_LC_PL_BOARD	0x04
#define FPGA_FC_PL_BOARD	0x05
#define FPGA_BOARD_REV_OFFSET 0x3
#define FPGA_LC_PL_INT_OFFSET         0xa 
#define FPGA_LC_PL_INT                0x08 /* bit 19 of 32 bit reg starting at 0x8 */
#define FPGA_LC_PL_PCI_INT                0xf0 
#define FPGA_LC_PL_SLOT_ID_OFFSET	0x18
#define FPGA_LC_PL_BIB_CHASIS		4
#define FPGA_FC_PL_INT_OFFSET         0x8
#define FPGA_FC_PL_INT                0x1  /* bit 0 of 32 bit reg starting at 0x8 */


extern int
board_preinit(brd_sbx_type_t type);

#endif /* !__BRD_SBX_H__ */

