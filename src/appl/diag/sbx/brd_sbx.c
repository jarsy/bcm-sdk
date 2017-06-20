/*
 * $Id: brd_sbx.c,v 1.30 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        brd_sbx.c
 * Purpose:     Board support for SBX reference boards
 *
 */ 

#include <shared/bsl.h>

#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/bsc.h>
#include <appl/diag/sbx/brd_sbx.h>
#include <sal/appl/io.h>
#include <sal/appl/config.h>

typedef struct brd_sbx_control_s {
    uint8 pre_init;   /* Pre Init status: TRUE or FALE */
} brd_sbx_control_t;

static brd_sbx_control_t brd_sbx_control[BOARD_TYPE_MAX];

#define FPGA_BASE                     (0x10000)
#define FPGA_MASTER_REG_OFFSET        0x14
#define FPGA_MASTER_MODE_BIT          0x10
#define FPGA_SCI_ROUTING_OFFSET       0x19
#define FPGA_SCI_TO_LCM               0xA0
#define FPGA_FE2K_DLL_ENABLE_OFFSET   0x1a
#define FPGA_FE2K_DLL_ENABLE          0x03

#define FPGA_LC_PL_INT_OFFSET         0xa 
#define FPGA_LC_PL_INT                0x08 /* bit 19 of 32 bit reg starting at 0x8 */
#define FPGA_FC_PL_INT_OFFSET         0x8
#define FPGA_FC_PL_INT                0x1  /* bit 0 of 32 bit reg starting at 0x8 */

static int
board_metrocore_fpga_read8(int addr, uint8 *v)
{
    int lcm0 = -1;
    int lcm1 = -1;
    int word = FPGA_BASE + (addr & ~0x3);
    int shift = (3 - (addr & 3)) * 8;

    int i;

    for (i=0;i<soc_ndev;i++){
      if (SOC_IS_SBX_BME3200(SOC_NDEV_IDX2DEV(i))){
        if (lcm0==-1)
          lcm0=SOC_NDEV_IDX2DEV(i);
        else if (lcm1==-1)
          lcm1=SOC_NDEV_IDX2DEV(i);
      }
    }

    if (lcm1 == -1) {
        cli_out("LCM/BME with FPGA not found\n");
        return BOARD_E_FAIL;
    }

    *v = (CMREAD(lcm1, word) >> shift) & 0xff;

    return BOARD_E_NONE;
}

static int
board_metrocore_fpga_write8(int addr, uint8 v)
{
    int lcm0 = -1;
    int lcm1 = -1;
    int shift = (3 - (addr & 3)) * 8;
    int word = FPGA_BASE + (addr & ~0x3);
    int i;
    uint32 v0;

    for (i=0;i<soc_ndev;i++){
      if (SOC_IS_SBX_BME3200(SOC_NDEV_IDX2DEV(i))){
        if (lcm0==-1)
          lcm0=SOC_NDEV_IDX2DEV(i);
        else if (lcm1==-1) 
          lcm1=SOC_NDEV_IDX2DEV(i);
      } 
    }

    if (lcm1 == -1) {
        cli_out("LCM/BME with FPGA not found\n");
        return BOARD_E_FAIL;
    }

    v0 = CMREAD(lcm1, word);
    v0 &= ~(0xff << shift);
    v0 |= (v << shift);
    CMWRITE(lcm1, word, v0);

    return BOARD_E_NONE;
}   

int
board_sbx_register(brd_sbx_type_t brd_type, int max_devices)
{
    int rv = BOARD_E_NONE;

    if (brd_type != BOARD_TYPE_METROCORE && 
	brd_type != BOARD_TYPE_POLARIS_LC &&
	brd_type != BOARD_TYPE_SIRIUS_SIM &&
	brd_type != BOARD_TYPE_FE2KXT_QE2K_POLARIS_LC) {
        return BOARD_E_PARAM;
    }
    return rv;
}

void
board_sbx_unregister(brd_sbx_type_t brd_type)
{

}

int
board_preinit(brd_sbx_type_t brd_type)
{
    int rv = BOARD_E_NONE;
    int addr, data;
    uint8 v;

    switch(brd_type) {
    case BOARD_TYPE_METROCORE:
        if (brd_sbx_control[brd_type].pre_init) {
            return BOARD_E_NONE;
        }

        /*
         * mcfpga addr=0x1a data=0x0 write ; \
         * mcfpga addr=0x1a data=0x3 write"
         */
        addr = 0x1a;
        data = 0x0;
        rv = board_metrocore_fpga_read8(addr, &v);
        if (rv) {
            return rv;
        }

        v = ((unsigned int) data) & 0xff;
        rv = board_metrocore_fpga_write8(addr, v);
        if (rv) {
            return rv;
        }

        addr = 0x1a;
        data = 0x3;
        rv = board_metrocore_fpga_read8(addr, &v);
        if (rv) {
            return rv;
        }

        v = ((unsigned int) data) & 0xff;
        rv = board_metrocore_fpga_write8(addr, v);
        if (rv) {
            return rv;
        }

        /*
         * Register board specific sram reset dll 
         */
        rv = board_sbx_register(brd_type, soc_all_ndev);
        if (rv) {
            return rv;
        }

        brd_sbx_control[brd_type].pre_init = TRUE;

        /*
         * Set the FABRIC_CONFIGURATION property
         */
        rv = sal_config_set(spn_FABRIC_CONFIGURATION, "0");
        if (rv != 0) {
            return rv;
        }
        return BOARD_E_NONE;

    case BOARD_TYPE_POLARIS_LC:
    case BOARD_TYPE_SIRIUS_SIM:
    /* same as BOARD_TYPE_SIRIUS_IPASS */
        if (brd_sbx_control[brd_type].pre_init) {
            return BOARD_E_NONE;
        }

        /*
         * Register board specific sram reset dll 
         */
        rv = board_sbx_register(brd_type, soc_all_ndev);
        if (rv) {
            return rv;
        }

        brd_sbx_control[brd_type].pre_init = TRUE;

        return BOARD_E_NONE;
    case BOARD_TYPE_FE2KXT_QE2K_POLARIS_LC:
        if (brd_sbx_control[brd_type].pre_init) {
  	    return BOARD_E_NONE;
        }
	brd_sbx_control[brd_type].pre_init = TRUE;
        rv = board_sbx_register(brd_type, soc_all_ndev);
        if (rv) {
            return rv;
        }

	return BOARD_E_NONE;
    case BOARD_TYPE_LCMODEL:
    case BOARD_TYPE_METROCORE_FABRIC:
        brd_sbx_control[brd_type].pre_init = TRUE;
	/* need to make sure that interrupts are enabled */
	rv = board_metrocore_fpga_read8(0x0, &v);
        if (rv) {
            return rv;
        }
	if (v == 0x15){
	  /* SFM enable FPGAs */
	  rv = board_metrocore_fpga_read8(0x14, &v);
          if (rv) {
              return rv;
          }
	  v|= 0xf;
          board_metrocore_fpga_write8(0x14, v);
        }else if (v == 0x17){
          /* SFM enable interrupts */
          rv = board_metrocore_fpga_read8(0x15, &v);
          if (rv) {
              return rv;
          }
          v|= 0xc;
          rv = board_metrocore_fpga_write8(0x15, v);
          if (rv) {
              return rv;
          }
	}
        return BOARD_E_NONE;
    case BOARD_TYPE_POLARIS_FC:
        brd_sbx_control[brd_type].pre_init = TRUE;
        return BOARD_E_NONE;

    /* The following Board Type is for the QE2K Benchscreen board. */
    case BOARD_TYPE_QE2K_BSCRN_LC:
    {
        if (brd_sbx_control[brd_type].pre_init) {
  	    return BOARD_E_NONE;
        }
	brd_sbx_control[brd_type].pre_init = TRUE;
	return BOARD_E_NONE;
    }
    case BOARD_TYPE_POLARIS_IPASS:
    {
	brd_sbx_control[brd_type].pre_init = TRUE;
        return BOARD_E_NONE;
        break;
    }

    default:
      break;
    }

    return BOARD_E_PARAM;
}

