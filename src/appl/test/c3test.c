/*
 * $Id: c3test.c,v 1.11 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        c3test.c
 * Purpose:     Caladan3 Specific TR tests
 *             
 *
 */


#include <shared/bsl.h>

#include <soc/defs.h>

#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_SVK)

#include <soc/types.h>
#include <soc/drv.h>
#include <sal/appl/sal.h>
#include <sal/appl/config.h>
#include <appl/diag/test.h> 
#include <bcm_int/control.h>
#include <bcm/bcmi2c.h>
#include <soc/i2c.h>
#include <soc/caladan3_pio_defs.h>
#include <soc/sbx/caladan3/cop.h>
#include <soc/sbx/caladan3/cmu.h>
#include <soc/phyctrl.h>
#include <soc/phyreg.h>

#include "testlist.h" 
#include "c3test.h"

#define BOARD_48G  0
#define BOARD_100G 1

c3testinfo_t  c3TestInfo[SOC_MAX_NUM_DEVICES];

/*
 * Function:
 *      sbx_c3_test_reset_init
 *
 * Purpose:
 *      Init 
 */
int
sbx_c3_test_reset_init(c3testinfo_t *testinfo, int cleanstart) 
{
    int rv;
    soc_control_t *soc;

    if (!SOC_UNIT_VALID(testinfo->unit)) {
        test_error(testinfo->unit, "Unit %d: not attached", testinfo->unit);
        return -1;
    }
    if (test_get_last_test_status(testinfo->unit) < 0) {
        cleanstart = 1;
    }
    soc = SOC_CONTROL(testinfo->unit);
    if ((cleanstart) || (soc && !(soc->soc_flags & SOC_F_INITED))) {

        rv = soc_reset_init(testinfo->unit);
        if (SOC_FAILURE(rv)) {
            test_error(testinfo->unit, "Unit %d: soc reset failed got (%d)\n", testinfo->unit, rv);
            return -1;
        }
        /* force bcm re-init after soc init */
        cleanstart = 1;
    }

    if ((cleanstart) || (!(BCM_UNIT_VALID(testinfo->unit)))) {
        rv = bcm_init(testinfo->unit);
        if (SOC_FAILURE(rv)) {
            test_error(testinfo->unit, "Unit %d: init bcm failed got (%d)\n", testinfo->unit, rv);
            return -1;
        }
    }
    rv = bcm_linkscan_enable_set(testinfo->unit, 0);
    if (SOC_FAILURE(rv)) {
        test_error(testinfo->unit, "Unit %d: link scan disable failed got (%d)\n", testinfo->unit, rv);
        return -1;
    }

    return 0;
}

/*
 * Function:
 *      sbx_c3_test_board_detect
 *
 * Purpose:
 *      board detect
 */
int
sbx_c3_test_board_detect(c3testinfo_t *testinfo) 
{

    int rv = SOC_E_NONE;
    uint8 board_id = 0;
    int id_detected = 0;
    int bus = cpu_i2c_defaultbus();
    int data = 0;
    if (testinfo->board_type != 0) {
         return 0;
    }
    /* Read the board ID register */
    cpu_i2c_attach(bus, SOC_I2C_NO_PROBE, CMIC_I2C_SPEED_SLOW_IO);
    rv = cpu_i2c_read(I2C_BRD_ID_IOP_SADDR, I2C_BRD_ID_IOP_PORT,
                       CPU_I2C_ALEN_BYTE_DLEN_BYTE, &data);
    if (rv == 0) {
        board_id = (uint8)data;
        id_detected = 1;
    } else {
        
        if (soc_ndev > 0) {
            rv = soc_i2c_attach(testinfo->unit, SOC_I2C_NO_PROBE, 0);
            if (SOC_SUCCESS(rv)) {
                rv = soc_i2c_read_byte_data(testinfo->unit, I2C_BRD_ID_IOP_SADDR,
                                            I2C_BRD_ID_IOP_PORT, &board_id);
                if (SOC_SUCCESS(rv)) {
                    id_detected = 1;
                    cli_out("\n Probing DUT I2c chain");
                    soc_i2c_probe(testinfo->unit);
                }
            }
        }
    }
    if (id_detected) {
        testinfo->board_type = board_id;
        testinfo->i2c_board = ((testinfo->board_type & 0x7f) == 0x39);
        cli_out("\n Probing CPU I2c chain");
        cpu_i2c_probe(testinfo->unit);
    }
    return 0;
}

/*
 * Function:
 *      sbx_c3_mdio_test_init_default
 *
 * Purpose:
 *      Init default
 */
void
sbx_c3_mdio_test_init_default(c3testinfo_t *testinfo) 
{
  
   /* MDIO defaults */
  testinfo->mdio_sbusid = 0;
  testinfo->mdio_lbusid = 4;
  testinfo->mdio_sphyid = 0;
  testinfo->mdio_lphyid = 1;
  testinfo->mdio_devaddr = 1;
  testinfo->mdio_rd_reg = 0x2;
  testinfo->mdio_wr_reg = 0x1f;
  testinfo->mdio_wrdata = 0xABCD;
  testinfo->mdio_clause45 = 0;
  testinfo->mdio_icmpdata = 0x143bff0;
  testinfo->mdio_ecmpdata = 0x3625d2c;

}

/*
 * Function:
 *      sbx_c3_mdio_test_init
 *
 * Purpose:
 *      Parse test arguments and save parameter structure locally
 *
 * Parameters:
 *      unit            - unit to test
 *      args            - test arguments
 *      pa              - test cookie (not used)
 *
 * Returns:
 *      TEST_OK
 *      TEST_FAIL
 *
 */

int
sbx_c3_mdio_test_init(int unit, args_t *args, void **pa)
{
  parse_table_t pt;
  int rv;

  c3testinfo_t *testinfo = &c3TestInfo[unit];

  if (!SOC_UNIT_VALID(testinfo->unit)) {
        test_error(testinfo->unit, "Unit %d: not attached", testinfo->unit);
        return -1;
  }
  rv = soc_reset_init(testinfo->unit);
  if (SOC_FAILURE(rv)) {
         test_error(testinfo->unit, "Unit %d: soc reset failed got (%d)\n", testinfo->unit, rv);
         return -1;
  }


  sbx_c3_test_board_detect(testinfo);

  sbx_c3_mdio_test_init_default(testinfo);

  parse_table_init(unit, &pt);

  parse_table_add(&pt, "C45",        PQ_INT | PQ_DFL, 0, &testinfo->mdio_clause45, NULL);
  parse_table_add(&pt, "StartBus",    PQ_INT | PQ_DFL, 0, &testinfo->mdio_sbusid, NULL);
  parse_table_add(&pt, "EndBus",      PQ_INT | PQ_DFL, 0, &testinfo->mdio_lbusid, NULL);
  parse_table_add(&pt, "StartPhy", PQ_INT | PQ_DFL, 0, &testinfo->mdio_sphyid, NULL);
  parse_table_add(&pt, "EndPhy",   PQ_INT | PQ_DFL, 0, &testinfo->mdio_lphyid, NULL);

  /* Parse arguments */
  if (0 > parse_arg_eq(args, &pt)) {
    test_error(unit,
               "%s: Invalid option: %s\n",
               ARG_CMD(args),
               ARG_CUR(args) ? ARG_CUR(args) : "*");
    parse_arg_eq_done(&pt);
    return -1;
  }

  parse_arg_eq_done(&pt);
 
  *pa = testinfo;

  return 0;
}

/*
 * MDIO Scan Device ID Test
 */
int
_c3_mdio_scan(int unit, void *pa, uint16 phyid, uint32 cmp) 
{
  c3testinfo_t *testinfo = pa;
  uint16 data;
  uint32 devid = 0;
  int rv;

  if (phyid & 0x80) {
      if (testinfo->mdio_clause45) {
          rv = soc_miimc45_write(unit, phyid, 1, 0x1f, 0x8000);
      } else {
          rv = soc_miim_write(unit, phyid, 0x1f, 0x8000);
      }
  }

  if (testinfo->mdio_clause45) {
      rv = soc_miimc45_read(unit, phyid, 1, 2, &data);
      test_msg("Mdio read test on phy(%x) devad(%x) reg(%x), status(%d) got data(%x)\n", 
               phyid, 1, 2, rv, data);
      if (SOC_SUCCESS(rv)) {
          devid = data << 16;
          rv = soc_miimc45_read(unit, phyid, 1, 3, &data);
          test_msg("Mdio read test on phy(%x) devad(%x) reg(%x), status(%d) got data(%x)\n", 
                  phyid, 1, 3, rv, data);
          devid |= data;
      }
  } else {
      rv = soc_miim_read(unit, phyid, testinfo->mdio_rd_reg, &data);
      test_msg("Mdio read test on phy(%x) reg(%x), status(%d) got data(%x)\n",
               phyid, testinfo->mdio_rd_reg, rv, data);
      if (SOC_SUCCESS(rv)) {
          devid = data << 16;
          rv = soc_miim_read(unit, phyid, testinfo->mdio_rd_reg+1, &data);
          test_msg("Mdio read test on phy(%x) reg(%x), status(%d) got data(%x)\n",
                   phyid, testinfo->mdio_rd_reg+1, rv, data);
          devid |= data;
      }
  }
  if (SOC_FAILURE(rv)) {
      test_msg("Read Test failed\n");
      return -1;
  } else {
      if (devid != cmp) {
          test_msg("Mdio scan test on phy(%x) failed got devid (%x) expected (%x)\n", phyid, devid, cmp);
          return -1;
      }
  }
  return 0;
}

int
_c3_mdio_scan_test(int unit, args_t *a, void *pa)
{
  int rv = 0;
  soc_port_t port;
  uint16 phy_addr, phy_addr_int;
  c3testinfo_t *testinfo = pa;

  PBMP_PORT_ITER(unit, port) {
      if (IS_GE_PORT(unit, port) && !(IS_XL_PORT(unit, port))) {
          rv = soc_phy_cfg_addr_get(unit, port, 0, &phy_addr);
          if (SOC_SUCCESS(rv)) {
              rv = _c3_mdio_scan(unit, pa, phy_addr, testinfo->mdio_ecmpdata);
              if (rv < 0) {
                  break;
              }
          }
          if ((port % 8) != 0) {
              continue;
          }  
      }
      rv = soc_phy_cfg_addr_get(unit, port, SOC_PHY_INTERNAL, &phy_addr_int);
      if (SOC_SUCCESS(rv)) {
          rv = _c3_mdio_scan(unit, pa, phy_addr_int, testinfo->mdio_icmpdata);
          if (rv < 0) {
              break;
          }
      }
  }
  return rv;
}



/*
 * MDIO Write Test
 */
int
_c3_mdio_write_test(int unit, args_t *a, void *pa)
{

  int bus;
  int phyid;
  int rv;
  uint16 data = 0;
  uint16 origdata = 0;
  c3testinfo_t *testinfo = pa;

  for (bus = testinfo->mdio_sbusid; bus < testinfo->mdio_lbusid; bus++) {
      for (phyid = testinfo->mdio_sphyid; phyid < testinfo->mdio_lphyid; phyid++) {
          if (testinfo->mdio_clause45) {
              rv = soc_miimc45_read(unit, phyid, testinfo->mdio_devaddr, testinfo->mdio_wr_reg, 
                                     &origdata);
              if (SOC_FAILURE(rv)) {
                  test_error(unit, "\nMDIO write test,failed reading devad(%x) reg(%x)", testinfo->mdio_devaddr, testinfo->mdio_wr_reg);
                  return -1;
              }
              rv = soc_miimc45_write(unit, phyid, testinfo->mdio_devaddr, testinfo->mdio_wr_reg, 
                                     testinfo->mdio_wrdata);
              test_msg("Mdio write test on bus(%d) phy(%x) devad(%x) reg(%x), status(%d) got data(%x)\n", 
                       bus, phyid, testinfo->mdio_devaddr, testinfo->mdio_rd_reg, rv, testinfo->mdio_wrdata);
          } else {
              
              rv = soc_miim_read(unit, phyid, testinfo->mdio_rd_reg, &origdata);
              if (SOC_FAILURE(rv)) {
                  test_error(unit, "\nMDIO write test,failed reading devad(%x) reg(%x)", testinfo->mdio_devaddr, testinfo->mdio_wr_reg);
                  return -1;
              }
              rv = soc_miim_write(unit, phyid, testinfo->mdio_rd_reg, testinfo->mdio_wrdata);
              test_msg("Mdio write test on bus(%d) phy(%x) reg(%x), status(%d) got data(%x)\n",
                       bus, phyid, testinfo->mdio_rd_reg, rv, testinfo->mdio_wrdata);
          }
          if (SOC_FAILURE(rv)) {
              test_msg("MDIO Write Test failed\n");
              return -1;
          } else {
              if (testinfo->mdio_clause45) {
                  rv = soc_miimc45_read(unit, phyid, testinfo->mdio_devaddr, testinfo->mdio_rd_reg, &data);
                  test_msg("Mdio readback test on bus(%d) phy(%x) devad(%x) reg(%x), status(%d) got data(%x)\n", 
                           bus, phyid, testinfo->mdio_devaddr, testinfo->mdio_rd_reg, rv, data);
              } else {
                  rv = soc_miim_read(unit, phyid, testinfo->mdio_rd_reg, &data);
                  test_msg("Mdio readback test on bus(%d) phy(%x) reg(%x), status(%d) got data(%x)\n",
                           bus, phyid, testinfo->mdio_rd_reg, rv, data);
              }
              if (SOC_FAILURE(rv)) {
                  test_msg("MDIO Read Test failed\n");
                  return -1;
              } else {
                  if (data != testinfo->mdio_wrdata) {
                      test_msg("MDIO RW Test failed\n");
                      return -1;
                  } else {
                      /* Restore orig data */
                      if (testinfo->mdio_clause45) {
                          rv = soc_miimc45_write(unit, phyid, testinfo->mdio_devaddr, 
                                                 testinfo->mdio_wr_reg, origdata);
                      } else {
                          rv = soc_miim_write(unit, phyid, testinfo->mdio_rd_reg, testinfo->mdio_wrdata);
                      }
                  }
              }    
          }
      }
  }
  return 0;
}

/*
 * Function:
 *      sbx_c3_mdio_test_run
 *
 * Purpose:
 *      Runs MDIO test
 *
 * Parameters:
 *      unit            - unit to test
 *      a               - test arguments (ignored)
 *      pa              - test cookie (ignored)
 *
 * Returns:
 *      TEST_OK
 *      TEST_FAIL
 *
 */

int
sbx_c3_mdio_test_run(int unit, args_t *a, void *pa)
{

  int ret = 0;
  c3testinfo_t *testinfo = pa;

  if (testinfo->mdio_test == 0) {
      ret = _c3_mdio_scan_test(unit, a, pa);
  } 
  if (testinfo->mdio_test == 1) {
      ret = _c3_mdio_write_test(unit, a, pa);
  }

  return ret;
}


/*
 * Function:
 *      sbx_c3_mdio_test_done
 *
 * Purpose:
 *      Cleans up 
 *
 * Parameters:
 *      unit            - unit to test
 *      pa              - test cookie (ignored)
 *
 * Returns:
 *      TEST_OK
 *      TEST_FAIL
 */

int
sbx_c3_mdio_test_done(int unit, void *pa)
{
  c3testinfo_t *testinfo = pa;
  sbx_c3_test_reset_init(testinfo, 1);
  return 0;
}


/*
 * Function:
 *      sbx_c3_i2c_test_init
 *
 * Purpose:
 *      Parse test arguments and save parameter structure locally
 *
 * Parameters:
 *      unit            - unit to test
 *      args            - test arguments
 *      pa              - test cookie (not used)
 *
 * Returns:
 *      TEST_OK
 *      TEST_FAIL
 *
 */
void
sbx_c3_i2c_test_init_default(c3testinfo_t *testinfo) 
{

  testinfo->i2c_mode = 0;
  testinfo->i2c_test = 0;
  if (testinfo->board_type != 0) {
      testinfo->i2c_board = ((testinfo->board_type & 0x7f) == 0x39);
  }
  testinfo->i2c_muxid = -1;
  testinfo->i2c_muxch = -1;

}

int
sbx_c3_i2c_test_init(int unit, args_t *args, void **pa)
{
  parse_table_t pt;

  c3testinfo_t *testinfo = &c3TestInfo[unit];
  
  sbx_c3_test_reset_init(testinfo, 1);
  sbx_c3_test_board_detect(testinfo);

  sbx_c3_i2c_test_init_default(testinfo);

  parse_table_init(unit, &pt);

  parse_table_add(&pt, "Mode",        PQ_INT | PQ_DFL, 0, &testinfo->i2c_mode, NULL);
  parse_table_add(&pt, "Test",        PQ_INT | PQ_DFL, 0, &testinfo->i2c_test, NULL);
  parse_table_add(&pt, "Board",      PQ_INT | PQ_DFL, 0, &testinfo->i2c_board, NULL);
  parse_table_add(&pt, "MuxId",      PQ_INT | PQ_DFL, 0, &testinfo->i2c_muxid, NULL);
  parse_table_add(&pt, "MuxChannel",      PQ_INT | PQ_DFL, 0, &testinfo->i2c_muxch, NULL);

  /* Parse arguments */
  if (0 > parse_arg_eq(args, &pt)) {
    test_error(unit,
               "%s: Invalid option: %s\n",
               ARG_CMD(args),
               ARG_CUR(args) ? ARG_CUR(args) : "*");
    parse_arg_eq_done(&pt);
    return -1;
  }
  if (testinfo->i2c_muxid > 6) {
      test_error(unit, "Invalid muxid: %d\n", testinfo->i2c_muxid);
      return -1;
  }
  if (testinfo->i2c_muxch > 7) {
      test_error(unit, "Invalid mux channel: %d\n", testinfo->i2c_muxch);
      return -1;
  }

  parse_arg_eq_done(&pt);

  *pa = testinfo;

  return 0;
}

/*
 * Function:
 *      sbx_c3_i2c_optics_test_run
 *
 * Purpose:
 *      Performs the PIO test only presense detect now
 *
 * Parameters:
 *      unit            - unit to test
 *      pa              - test cookie (ignored)
 *
 * Returns:
 *      TEST_OK
 *      TEST_FAIL
 */
int
sbx_c3_i2c_optics_test_run(int unit, args_t *a, void *pa)
{
  int i, j, rv, fd = -1, fd1 = -1;
  uint8 data[264], ch=0, devtype=0;
  uint32 len=0;
  char *muxes[]      = {"mux0", "mux1", "mux2", "mux3", "mux4", "mux5", "mux6", "mux7"};
  char result_48g[]  = { 0x1F,  0x00,    0x00,   0x00,  0x00,   0x00,   0x00,   0x00  };
  char result_100g[] = { 0x3F,  0xFF,    0xFF,   0xFF,  0xFF,   0xFF,   0xFF,   0xFF  };
  int ndev = sizeof(muxes)/sizeof(muxes[0]);
  int start, mux0, status = 0;
  int board COMPILER_ATTRIBUTE((unused));
  int result;
  c3testinfo_t *testinfo = pa;
  
  test_msg("\nCaladan3 Optics probe test - Started\n\n");
  for (ch = 0, j = 0; j < ndev; j++) {
      fd = bcm_i2c_open(unit, muxes[j], 0, 0);
      if (fd >= 0) {
          rv = bcm_i2c_write(unit, fd, 0, &ch, 1);
      }
  }
  for (j = 0; j < ndev; j++) {
      if ((testinfo->i2c_muxid >= 0) && (testinfo->i2c_muxid != j)) {
          continue;
      }

      result = 0;
      fd = bcm_i2c_open(unit, muxes[j], 0, 0);
      if (fd >= 0) {
          mux0 = (sal_strcasecmp("mux0", muxes[j])==0);
          for (i=0; i<8; i++) {
               if ((mux0) && (i > 5)) { /* Nothing beyond ch 5 */ continue; }
               if ((mux0) && (i == 5) && (testinfo->i2c_board==0)) { /* No ch 5 on 48G board */ continue; }
               if ((testinfo->i2c_muxch >= 0) && (testinfo->i2c_muxch != i)) { continue; }

              ch = 1 << i;
              rv = bcm_i2c_write(unit, fd, 0, &ch, 1);
              test_msg("Probing Mux %s channel %d\n", muxes[j], i);
              if (SOC_SUCCESS(rv)) {
                  fd1 = bcm_i2c_open(unit, "nvram0", SOC_I2C_DO_PROBE, 0);
                  if (fd1 < 0) 
                      fd1 = bcm_i2c_open(unit, "cxp0", SOC_I2C_DO_PROBE, 0);
                  if (fd1 < 0) {
                      test_msg("Failed accessing cxp/sfp/qsfp device eeprom at addr 0x50\n");
                      status = -1;
                      continue;
                  }
                  len = 256;
                  sal_memset(&data[0], 0, len);
                  rv = bcm_i2c_read(unit, fd1, 0x0, data, &len);
                  if (SOC_SUCCESS(rv)) {
                      devtype = (data[0] ? data[0] : data[0x80]);
                      if (devtype == 0xc) {
                          start = 148;
                      } else if (devtype < 0xc) {
                          start = 20;
                      } else {
                          start = 152;
                      }
                  }
                  if (SOC_FAILURE(rv)) {
                      test_msg("Failed reading cxp/sfp/qsfp device eeprom at addr 0x50\n");
                      continue;
                  } else {
                      test_msg("Read Device type 0x%x Vendor: %s\n", devtype, &data[start]);
                      board = 0;
                  }
                  result |= ch;
                  sal_udelay(10);
              }
          }
          /* Turn off all channels on the mux */
          ch = 0;
          bcm_i2c_write(unit, fd, 0, &ch, 1);
          if (testinfo->i2c_muxch >= 0) {
              if (((testinfo->i2c_muxch & result_100g[j]) != (result & testinfo->i2c_muxch)) &&
                  ((testinfo->i2c_muxch & result_48g[j]) != (result & testinfo->i2c_muxch)))
              test_msg("Test failed on mux %s, got %x", muxes[j], result);
              status = -1;
          } else {
              if ((result != result_100g[j]) && (result != result_48g[j])) {
                  test_msg("Test failed on mux %s, got %x", muxes[j], result);
                  status = -1;
              }
          }
      }
  }
  test_msg("\nCaladan3 Optics probe test Completed Successfully\n");
  return status;
}

/*
 * Function:
 *      sbx_c3_i2c_pio_test_run
 *
 * Purpose:
 *      Performs the PIO test for prs/abs
 *
 * Parameters:
 *      unit            - unit to test
 *      pa              - test cookie (ignored)
 *
 * Returns:
 *      TEST_OK
 *      TEST_FAIL
 */
int
sbx_c3_i2c_pio_test_run(int unit, args_t *a, void *pa)
{
#ifdef BCM_CALADAN3_SVK
  int i, j, rv = 0, fd = -1;
  unsigned char data = 0;
  c3testinfo_t *testinfo = pa;
  iop_bit_config_t *fields;
  char *devicename, *fieldname;
  char *testfields_100g[] = {
      "QSFP0_MOD_PRS_N", "QSFP1_MOD_PRS_N", "QSFP2_MOD_PRS_N", 
      "CXP_PRST_N", "GE0_ABS", "GE1_ABS"
  };
  char *testfields_48g[] = {
      "QSFP0_MOD_PRS_N", "QSFP1_MOD_PRS_N", "QSFP2_MOD_PRS_N", 
      "CXP_PRST_N", "GE0_ABS", "GE1_ABS",
      "P0_ABS",  "P1_ABS",  "P2_ABS",  "P3_ABS",  "P4_ABS",  "P5_ABS", 
      "P6_ABS",  "P7_ABS",  "P8_ABS",  "P9_ABS",  "P10_ABS", "P11_ABS", 
      "P12_ABS", "P13_ABS", "P14_ABS", "P15_ABS", "P16_ABS", "P17_ABS", 
      "P18_ABS", "P19_ABS", "P20_ABS", "P21_ABS", "P22_ABS", "P23_ABS", 
      "P24_ABS", "P25_ABS", "P26_ABS", "P27_ABS", "P28_ABS", "P29_ABS", 
      "P30_ABS", "P31_ABS", "P32_ABS", "P33_ABS", "P34_ABS", "P35_ABS", 
      "P36_ABS", "P37_ABS", "P38_ABS", "P39_ABS", "P40_ABS", "P41_ABS", 
      "P42_ABS", "P43_ABS", "P44_ABS", "P45_ABS", "P46_ABS", "P47_ABS"
  };
  int testresult[] = {
      0, 0, 0,
      0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0
  };

  int max, maxfields, dev, port, pos, size;
  uint32 len;
  int bus = cpu_i2c_defaultbus();
  int cpumode = 1; /* Always on C3 */
  int status = 0;

  test_msg("\nCaladan3 Transceiver Presence using 40Bit IOP Test Started\n");
  if (testinfo->i2c_board == BOARD_48G) {
      max = PIO_MAX_BIT_FIELDS_48G;
      fields = pio_bit_config_48g;
      maxfields = sizeof(testfields_48g)/sizeof(testfields_48g[0]);
  } else {
      max = PIO_MAX_BIT_FIELDS_100G;
      fields = pio_bit_config_100g;
      maxfields = sizeof(testfields_100g)/sizeof(testfields_100g[0]);
  }
  for (j = 0; j < maxfields; j++) {
      if (testinfo->i2c_board == BOARD_48G) {
          fieldname = testfields_48g[j];
      } else {
          fieldname = testfields_100g[j];
      }
      if ((testinfo->i2c_board == BOARD_48G) && 
           sal_strcasecmp("CXP_PRST_N", fieldname)==0) {
          continue;
      }
      for (i = 0; i < max; i++) {
          if (sal_strcasecmp(fields[i].name, fieldname) == 0) {
              break;
          }
      }
      if (i == max) {
          test_msg("\nCannot field field %s", fieldname);
          return -1;
      }
      dev = fields[i].dev;
      port = fields[i].port;
      pos = fields[i].pos;
      size = fields[i].size;
      pio_devname(dev, &devicename);
      if (testinfo->board_type & 0x80) {
              /* On rev A, MAX127 adc, 40bit IOP on dut bus */
          cpumode = (sal_strncasecmp(devicename, "adc", 3) == 0) ? 0 : 1;
          cpumode = (sal_strncasecmp(devicename, "iop", 3) == 0) ? 0 : 1;
      } else {
          cpumode = 1;
      }
      if (cpumode) {
          fd = cpu_i2c_dev_open(bus, devicename, 0, 0);
      } else {
          fd = bcm_i2c_open(unit, devicename, 0, 0);
      }
      if (fd >= 0) {
          len = 1;
          if (cpumode) {
              rv =cpu_i2c_dev_read(bus, fd, I2C_IOP_IN(port), &data, &len);
          } else {
              rv = bcm_i2c_read(unit, fd, I2C_IOP_IN(port), &data, &len);
          }
          /* test_msg("Reading PCA9505 Port %d\n", port); */
          if (SOC_SUCCESS(rv)) {
              data = ((data >> pos) & ((1 << size)-1));
              if (data != testresult[j]) {
                  test_msg("Failed for entry %s(%d) got %x, expected %x\n", 
                       fields[i].name, j ,data, testresult[j]);
                  status = -1;
                  continue;
              } 
              test_msg("%s -> Ok\n", fields[i].name);
          }
      } else {
          test_msg("Dev open failed for %s\n", devicename);
      }
  }
  test_msg("\nCaladan3 Transceiver Presence Test Completed Successfully\n");
  return status;
#else
  return 0;
#endif
}

/*
 * Function:
 *      sbx_c3_i2c_board_test_run
 *
 * Purpose:
 *      Runs Voltage and Current tests
 *
 * Parameters:
 *      unit            - unit to test
 *      pa              - test cookie (ignored)
 *
 * Returns:
 *      TEST_OK
 *      TEST_FAIL
 */

int
sbx_c3_i2c_board_test_run(int unit, args_t *a, void *pa)
{
  int rv = 0;
 
#ifdef BCM_CALADAN3_SVK

  c3testinfo_t *testinfo = pa;
  int i, j, adc = -1;
  i2c_adc_t adc_data;
  int cpu_mode = 0;
  char *devices[]  = {"adoc0", "adc0"};
  int  volt_ch[][6] = {{0, 1, 2, 3, 4, 5}, {4, 5, 6, -1, -1, -1}};
  char* volt_src[][6] = {
             {
               "Digital 1 volt", "Analog 1 volt ", "1.5 volts     ", 
               "3.3 volts     ", "1.2 volts     ", "Tcam          "
             },
             {
               "VDD 5 volts   ", "VDD 3.3 volts ", "VDD 1.2 volts "
             }
         };
#ifdef COMPILER_HAS_DOUBLE
  double  volt_level[][6]={ 
             { 
                1, 1, 1.5,
                3.3, 1.2, 1.8
             },
             {  5, 3.3, 1.2,
                0,   0,   0
             }
          };
  double range = 1; /* percent */
#else
  int  volt_level[][6]={
            { 
              1000, 1000, 1500, 
              3300, 1500, 1800
            },
            { 
              5000, 3300, 1200
            }
         };
  int range = 10; /* percent */
#endif

  int ndev = sizeof(devices)/sizeof(devices[0]);
  int nch = sizeof(volt_ch[0])/sizeof(int);

  test_msg("\nCaladan3 Board Voltage level Test Started\n\n");
  for (i=0; i<ndev; i++) {
      for (j=0; j<nch; j++) {
          if ((volt_ch[i][j]) < 0) {
              continue;
          }
          if ((testinfo->i2c_board == BOARD_100G) && (6 == volt_ch[i][j])) {
              /* 1.2V not used on 100G board */
              continue;
          }
          if (testinfo->board_type & 0x80) {
              /* On rev A, max127 adc is on dut bus */
              cpu_mode = (sal_strncasecmp(devices[i], "adc", 3) == 0) ? 0 : 1;
          } else {
              cpu_mode = 1;
          }
          if (cpu_mode) {
             /* Cpu mode, Cannot call the BCM api since the unit_valid check would fail */
             adc = cpu_i2c_dev_open(cpu_i2c_defaultbus(), devices[i], 0, 0);
             if (adc < 0) {
                 test_msg("Device open failed for %s\n", devices[i]);
                    return (-1);
             }
             if ( cpu_i2c_dev_ioctl(cpu_i2c_defaultbus(), adc, I2C_ADC_QUERY_CHANNEL,
                                    &adc_data, volt_ch[i][j]) < 0) {
                 test_msg("Error: failed to perform A/D conversions.\n");
             }
          } else {
             /* BCM mode */
             adc = bcm_i2c_open(unit, devices[i], 0, 0);
             if (adc < 0) {
                 test_msg("Device open failed for %s\n", devices[i]);
                    return (-1);
             }
             if ( bcm_i2c_ioctl(unit, adc, I2C_ADC_QUERY_CHANNEL,
                                    &adc_data, volt_ch[i][j]) < 0) {
                 test_msg("Error: failed to perform A/D conversions.\n");
             }
          }
#ifdef COMPILER_HAS_DOUBLE
          test_msg("%-.20s\t%2.3f Volts \n",
                   volt_src[i][j],
                   adc_data.val);
#else
          test_msg("%-.20s\t%d.%03d Volts \n",
                   volt_src[i][j],
                   INT_FRAC_3PT_3(adc_data.val));
#endif
          if (adc_data.val < volt_level[i][j] - volt_level[i][j]*(range/100)) {
              test_msg("Error Voltage Source %s is less than 1 percent range\n", volt_src[i][j]);
              rv = -1;
          }
      }
  }
  test_msg("\nCaladan3 Board Voltage level Test Completed Successfully\n");
#endif
  return rv;
}

/*
 * Function:
 *      sbx_c3_i2c_test_run
 *
 * Purpose:
 *      Runs I2c test
 *
 * Parameters:
 *      unit            - unit to test
 *      a               - test arguments (ignored)
 *      pa              - test cookie (ignored)
 *
 * Returns:
 *      TEST_OK
 *      TEST_FAIL
 *
 */
int
sbx_c3_i2c_test_run(int unit, args_t *a, void *pa)
{
  int rv = 0;
  c3testinfo_t *testinfo = pa;

  switch (testinfo->i2c_test) {
  case 0:
      rv = sbx_c3_i2c_optics_test_run(unit, a, pa);
      if (rv == 0)
          rv = sbx_c3_i2c_board_test_run(unit, a, pa);
      if (rv == 0)
          rv = sbx_c3_i2c_pio_test_run(unit, a, pa);
      break;
  case 1:
      rv = sbx_c3_i2c_board_test_run(unit, a, pa);
      break;
  case 2:
      rv = sbx_c3_i2c_pio_test_run(unit, a, pa);
      break;
  case 3:
      rv = sbx_c3_i2c_optics_test_run(unit, a, pa);
      break;
  }
  return rv;
}


/*
 * Function:
 *      sbx_c3_i2c_test_done
 *
 * Purpose:
 *      Cleans up 
 *
 * Parameters:
 *      unit            - unit to test
 *      pa              - test cookie (ignored)
 *
 * Returns:
 *      TEST_OK
 *      TEST_FAIL
 */

int
sbx_c3_i2c_test_done(int unit, void *pa)
{
  c3testinfo_t *testinfo = pa;
  sbx_c3_test_reset_init(testinfo, 1);
  return 0;
}

/*
 * Function:
 *      sbx_c3_tcam_test_init
 *
 * Purpose:
 *      Init Tcam test
 *
 * Parameters:
 *      unit            - unit to test
 *      args            - test arguments
 *      pa              - test cookie (not used)
 *
 * Returns:
 *      TEST_OK
 *      TEST_FAIL
 *
 */

int
sbx_c3_tcam_test_init(int unit, args_t *args, void **pa)
{
  parse_table_t pt;

  c3testinfo_t *testinfo = &c3TestInfo[unit];

  sbx_c3_test_reset_init(testinfo, 1);
  sbx_c3_test_board_detect(testinfo);

  parse_table_init(unit, &pt);

  parse_table_add(&pt, "Test",       PQ_INT | PQ_DFL, 0, &testinfo->tcam_testid, NULL);
  parse_table_add(&pt, "Bus",        PQ_INT | PQ_DFL, 0, &testinfo->tcam_busid, NULL);
  parse_table_add(&pt, "TestDevice", PQ_INT | PQ_DFL, 0, &testinfo->tcam_testdev, NULL);
  parse_table_add(&pt, "TestAddr",   PQ_INT | PQ_DFL, 0, &testinfo->tcam_testaddr, NULL);

  /* Parse arguments */
  if (0 > parse_arg_eq(args, &pt)) {
    test_error(unit,
               "%s: Invalid option: %s\n",
               ARG_CMD(args),
               ARG_CUR(args) ? ARG_CUR(args) : "*");
    parse_arg_eq_done(&pt);
    return -1;
  }

  parse_arg_eq_done(&pt);

  *pa = testinfo;

  return 0;
}

/*
 * Function:
 *      sbx_c3_tcam_test_run
 *
 * Purpose:
 *      Runs Tcam Access test
 *
 * Parameters:
 *      unit            - unit to test
 *      a               - test arguments (ignored)
 *      pa              - test cookie (ignored)
 *
 * Returns:
 *      TEST_OK
 *      TEST_FAIL
 *
 */

int
sbx_c3_tcam_test_run(int unit, args_t *a, void *pa)
{
   int rv;
   uint16 data;
   
   /* Only TCAM MDIO rw now */
   rv = soc_miimc45_read(unit, 0x60, 2, 0x12 , &data);
   if (SOC_SUCCESS(rv)) {
       if (data != 0xaaaa) {
           test_msg("TCAM MDIO read access failed, expected 0xaaaa got %x\n", data);
           return -1;
       }
   }
   rv = soc_miimc45_write(unit, 0x60, 2, 0x12, 0xbbbb);
   if (SOC_FAILURE(rv)) {
       test_msg("TCAM MDIO write access failed\n");
       return -1;
   }
   rv = soc_miimc45_read(unit, 0x60, 2, 0x12 , &data);
   if (SOC_SUCCESS(rv)) {
       if (data != 0xbbbb) {
           test_msg("TCAM MDIO read access failed, expected 0xbbbb got %x\n", data);
           return -1;
       }
   }
   /* Revert back */
   rv = soc_miimc45_write(unit, 0x60, 2, 0x12, 0xaaaa);
   return 0;
}


/*
 * Function:
 *      sbx_c3_tcam_test_done
 *
 * Purpose:
 *      Cleans up 
 *
 * Parameters:
 *      unit            - unit to test
 *      pa              - test cookie (ignored)
 *
 * Returns:
 *      TEST_OK
 *      TEST_FAIL
 */

int
sbx_c3_tcam_test_done(int unit, void *pa)
{
  c3testinfo_t *testinfo = pa;
  sbx_c3_test_reset_init(testinfo, 1);
  return 0;
}

/*
 * Function:
 *      sbx_c3_sys_test_init
 *
 * Purpose:
 *      Init System test
 *
 * Parameters:
 *      unit            - unit to test
 *      args            - test arguments
 *      pa              - test cookie (not used)
 *
 * Returns:
 *      TEST_OK
 *      TEST_FAIL
 *
 */
int
sbx_c3_sys_test_init_internal(int unit, args_t *args, void **pa)
{
    char *name = NULL, *value = NULL;
    int rv = SOC_E_NONE;
    uint16 dev_id = 0;
    uint8 rev_id = 0;

    soc_cm_get_id( unit, &dev_id, &rev_id);

    test_msg("Please wait while setting up configuration\n");
    /* Clear the current config and load ILKN config */
    for (;;) {
        if (sal_config_get_next(&name, &value) < 0) {
            break;
        }
        if (sal_strcasecmp("c3_ucode_path", name)==0) {continue;}
        if (sal_strcasecmp("c3_ucode_test", name)==0) {continue;}
        if (sal_strncasecmp(name, "ddr3", 4)==0) {continue;}
        if (sal_config_set(name, 0) < 0) {
            break;
        }
        name = NULL;
    }

    /* Cooked config for IL mode */
    sal_config_set("lrp_bypass", "2");  /* Note special mode to skip tmu/cmu/cop init */

    sal_config_set(spn_LOAD_FIRMWARE, "1");

    value = sal_config_get("c3_ucode_test");
    if ( !sal_strcmp(value,"173_48G") ) {
      sal_config_set("ucode_num_ports.0","48");
      if ( dev_id == BCM88034_DEVICE_ID ) {
        sal_config_set("ucode_port.port1.0","clport0.ge.0.0:il1.il50n.0.0");
        sal_config_set("ucode_port.port2.0","clport0.ge.0.1:il1.il50n.0.1");
        sal_config_set("ucode_port.port3.0","clport0.ge.0.2:il1.il50n.0.2");
        sal_config_set("ucode_port.port4.0","clport0.ge.0.3:il1.il50n.0.3");
        sal_config_set("ucode_port.port5.0","clport0.ge.0.4:il1.il50n.0.4");
        sal_config_set("ucode_port.port6.0","clport0.ge.0.5:il1.il50n.0.5");
        sal_config_set("ucode_port.port7.0","clport0.ge.0.6:il1.il50n.0.6");
        sal_config_set("ucode_port.port8.0","clport0.ge.0.7:il1.il50n.0.7");
        sal_config_set("ucode_port.port9.0","clport0.ge.0.8:il1.il50n.0.8");
        sal_config_set("ucode_port.port10.0","clport0.ge.0.9:il1.il50n.0.9");
        sal_config_set("ucode_port.port11.0","clport0.ge.0.10:il1.il50n.0.10");
        sal_config_set("ucode_port.port12.0","clport0.ge.0.11:il1.il50n.0.11");
        sal_config_set("ucode_port.port13.0","xtport0.ge.0.0:il1.il50n.0.12");
        sal_config_set("ucode_port.port14.0","xtport0.ge.0.1:il1.il50n.0.13");
        sal_config_set("ucode_port.port15.0","xtport0.ge.0.2:il1.il50n.0.14");
        sal_config_set("ucode_port.port16.0","xtport0.ge.0.3:il1.il50n.0.15");
        sal_config_set("ucode_port.port17.0","xtport0.ge.0.4:il1.il50n.0.16");
        sal_config_set("ucode_port.port18.0","xtport0.ge.0.5:il1.il50n.0.17");
        sal_config_set("ucode_port.port19.0","xtport0.ge.0.6:il1.il50n.0.18");
        sal_config_set("ucode_port.port20.0","xtport0.ge.0.7:il1.il50n.0.19");
        sal_config_set("ucode_port.port21.0","xtport0.ge.0.8:il1.il50n.0.20");
        sal_config_set("ucode_port.port22.0","xtport0.ge.0.9:il1.il50n.0.21");
        sal_config_set("ucode_port.port23.0","xtport0.ge.0.10:il1.il50n.0.22");
        sal_config_set("ucode_port.port24.0","xtport0.ge.0.11:il1.il50n.0.23");
        sal_config_set("ucode_port.port25.0","xtport1.ge.0.0:il1.il50n.0.24");
        sal_config_set("ucode_port.port26.0","xtport1.ge.0.1:il1.il50n.0.25");
        sal_config_set("ucode_port.port27.0","xtport1.ge.0.2:il1.il50n.0.26");
        sal_config_set("ucode_port.port28.0","xtport1.ge.0.3:il1.il50n.0.27");
        sal_config_set("ucode_port.port29.0","xtport1.ge.0.4:il1.il50n.0.28");
        sal_config_set("ucode_port.port30.0","xtport1.ge.0.5:il1.il50n.0.29");
        sal_config_set("ucode_port.port31.0","xtport1.ge.0.6:il1.il50n.0.30");
        sal_config_set("ucode_port.port32.0","xtport1.ge.0.7:il1.il50n.0.31");
        sal_config_set("ucode_port.port33.0","xtport1.ge.0.8:il1.il50n.0.32");
        sal_config_set("ucode_port.port34.0","xtport1.ge.0.9:il1.il50n.0.33");
        sal_config_set("ucode_port.port35.0","xtport1.ge.0.10:il1.il50n.0.34");
        sal_config_set("ucode_port.port36.0","xtport1.ge.0.11:il1.il50n.0.35");
        sal_config_set("ucode_port.port37.0","xtport2.ge.0.0:il1.il50n.0.36");
        sal_config_set("ucode_port.port38.0","xtport2.ge.0.1:il1.il50n.0.37");
        sal_config_set("ucode_port.port39.0","xtport2.ge.0.2:il1.il50n.0.38");
        sal_config_set("ucode_port.port40.0","xtport2.ge.0.3:il1.il50n.0.39");
        sal_config_set("ucode_port.port41.0","xtport2.ge.0.4:il1.il50n.0.40");
        sal_config_set("ucode_port.port42.0","xtport2.ge.0.5:il1.il50n.0.41");
        sal_config_set("ucode_port.port43.0","xtport2.ge.0.6:il1.il50n.0.42");
        sal_config_set("ucode_port.port44.0","xtport2.ge.0.7:il1.il50n.0.43");
        sal_config_set("ucode_port.port45.0","xtport2.ge.0.8:il1.il50n.0.44");
        sal_config_set("ucode_port.port46.0","xtport2.ge.0.9:il1.il50n.0.45");
        sal_config_set("ucode_port.port47.0","xtport2.ge.0.10:il1.il50n.0.46");
        sal_config_set("ucode_port.port48.0","xtport2.ge.0.11:il1.il50n.0.47");
      } else {
        sal_config_set("ucode_port.port1.0","clport0.ge.0.0:il1.il100.0.0");
        sal_config_set("ucode_port.port2.0","clport0.ge.0.1:il1.il100.0.1");
        sal_config_set("ucode_port.port3.0","clport0.ge.0.2:il1.il100.0.2");
        sal_config_set("ucode_port.port4.0","clport0.ge.0.3:il1.il100.0.3");
        sal_config_set("ucode_port.port5.0","clport0.ge.0.4:il1.il100.0.4");
        sal_config_set("ucode_port.port6.0","clport0.ge.0.5:il1.il100.0.5");
        sal_config_set("ucode_port.port7.0","clport0.ge.0.6:il1.il100.0.6");
        sal_config_set("ucode_port.port8.0","clport0.ge.0.7:il1.il100.0.7");
        sal_config_set("ucode_port.port9.0","clport0.ge.0.8:il1.il100.0.8");
        sal_config_set("ucode_port.port10.0","clport0.ge.0.9:il1.il100.0.9");
        sal_config_set("ucode_port.port11.0","clport0.ge.0.10:il1.il100.0.10");
        sal_config_set("ucode_port.port12.0","clport0.ge.0.11:il1.il100.0.11");
        sal_config_set("ucode_port.port13.0","xtport0.ge.0.0:il1.il100.0.12");
        sal_config_set("ucode_port.port14.0","xtport0.ge.0.1:il1.il100.0.13");
        sal_config_set("ucode_port.port15.0","xtport0.ge.0.2:il1.il100.0.14");
        sal_config_set("ucode_port.port16.0","xtport0.ge.0.3:il1.il100.0.15");
        sal_config_set("ucode_port.port17.0","xtport0.ge.0.4:il1.il100.0.16");
        sal_config_set("ucode_port.port18.0","xtport0.ge.0.5:il1.il100.0.17");
        sal_config_set("ucode_port.port19.0","xtport0.ge.0.6:il1.il100.0.18");
        sal_config_set("ucode_port.port20.0","xtport0.ge.0.7:il1.il100.0.19");
        sal_config_set("ucode_port.port21.0","xtport0.ge.0.8:il1.il100.0.20");
        sal_config_set("ucode_port.port22.0","xtport0.ge.0.9:il1.il100.0.21");
        sal_config_set("ucode_port.port23.0","xtport0.ge.0.10:il1.il100.0.22");
        sal_config_set("ucode_port.port24.0","xtport0.ge.0.11:il1.il100.0.23");
        sal_config_set("ucode_port.port25.0","xtport1.ge.0.0:il1.il100.0.24");
        sal_config_set("ucode_port.port26.0","xtport1.ge.0.1:il1.il100.0.25");
        sal_config_set("ucode_port.port27.0","xtport1.ge.0.2:il1.il100.0.26");
        sal_config_set("ucode_port.port28.0","xtport1.ge.0.3:il1.il100.0.27");
        sal_config_set("ucode_port.port29.0","xtport1.ge.0.4:il1.il100.0.28");
        sal_config_set("ucode_port.port30.0","xtport1.ge.0.5:il1.il100.0.29");
        sal_config_set("ucode_port.port31.0","xtport1.ge.0.6:il1.il100.0.30");
        sal_config_set("ucode_port.port32.0","xtport1.ge.0.7:il1.il100.0.31");
        sal_config_set("ucode_port.port33.0","xtport1.ge.0.8:il1.il100.0.32");
        sal_config_set("ucode_port.port34.0","xtport1.ge.0.9:il1.il100.0.33");
        sal_config_set("ucode_port.port35.0","xtport1.ge.0.10:il1.il100.0.34");
        sal_config_set("ucode_port.port36.0","xtport1.ge.0.11:il1.il100.0.35");
        sal_config_set("ucode_port.port37.0","xtport2.ge.0.0:il1.il100.0.36");
        sal_config_set("ucode_port.port38.0","xtport2.ge.0.1:il1.il100.0.37");
        sal_config_set("ucode_port.port39.0","xtport2.ge.0.2:il1.il100.0.38");
        sal_config_set("ucode_port.port40.0","xtport2.ge.0.3:il1.il100.0.39");
        sal_config_set("ucode_port.port41.0","xtport2.ge.0.4:il1.il100.0.40");
        sal_config_set("ucode_port.port42.0","xtport2.ge.0.5:il1.il100.0.41");
        sal_config_set("ucode_port.port43.0","xtport2.ge.0.6:il1.il100.0.42");
        sal_config_set("ucode_port.port44.0","xtport2.ge.0.7:il1.il100.0.43");
        sal_config_set("ucode_port.port45.0","xtport2.ge.0.8:il1.il100.0.44");
        sal_config_set("ucode_port.port46.0","xtport2.ge.0.9:il1.il100.0.45");
        sal_config_set("ucode_port.port47.0","xtport2.ge.0.10:il1.il100.0.46");
        sal_config_set("ucode_port.port48.0","xtport2.ge.0.11:il1.il100.0.47");
      }
      sal_config_set("xgxs_rx_lane_map_ge0","0x1032");
      sal_config_set("xgxs_rx_lane_map_ge8","0x1032");
      sal_config_set("xgxs_rx_lane_map_ge16","0x3210");
      sal_config_set("xgxs_rx_lane_map_ge24","0x3210");
      sal_config_set("xgxs_rx_lane_map_ge32","0x1032");
      sal_config_set("xgxs_rx_lane_map_ge40","0x1032");
      sal_config_set("xgxs_tx_lane_map_ge0","0x3210");
      sal_config_set("xgxs_tx_lane_map_ge8","0x3210");
      sal_config_set("xgxs_tx_lane_map_ge16","0x1032");
      sal_config_set("xgxs_tx_lane_map_ge24","0x1032");
      sal_config_set("xgxs_tx_lane_map_ge32","0x3210");
      sal_config_set("xgxs_tx_lane_map_ge40","0x3210");
      sal_config_set("phy_xaui_rx_polarity_flip_ge0","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge1","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge2","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge3","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge4","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge5","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge6","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge7","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge8","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge9","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge10","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge11","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge12","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge13","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge14","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge15","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge16","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge17","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge18","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge19","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge20","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge21","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge22","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge23","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge24","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge25","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge26","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge27","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge28","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge29","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge30","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge31","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge32","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge33","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge34","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge35","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge36","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge37","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge38","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge39","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge40","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge41","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge42","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge43","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge44","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge45","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge46","0x0001");
      sal_config_set("phy_xaui_rx_polarity_flip_ge47","0x0001");
      sal_config_set("phy_fiber_pref_ge0","1");
      sal_config_set("phy_fiber_pref_ge1","1");
      sal_config_set("phy_fiber_pref_ge2","1");
      sal_config_set("phy_fiber_pref_ge3","1");
      sal_config_set("phy_fiber_pref_ge4","1");
      sal_config_set("phy_fiber_pref_ge5","1");
      sal_config_set("phy_fiber_pref_ge6","1");
      sal_config_set("phy_fiber_pref_ge7","1");
      sal_config_set("phy_fiber_pref_ge8","1");
      sal_config_set("phy_fiber_pref_ge9","1");
      sal_config_set("phy_fiber_pref_ge10","1");
      sal_config_set("phy_fiber_pref_ge11","1");
      sal_config_set("phy_fiber_pref_ge12","1");
      sal_config_set("phy_fiber_pref_ge13","1");
      sal_config_set("phy_fiber_pref_ge14","1");
      sal_config_set("phy_fiber_pref_ge15","1");
      sal_config_set("phy_fiber_pref_ge16","1");
      sal_config_set("phy_fiber_pref_ge17","1");
      sal_config_set("phy_fiber_pref_ge18","1");
      sal_config_set("phy_fiber_pref_ge19","1");
      sal_config_set("phy_fiber_pref_ge20","1");
      sal_config_set("phy_fiber_pref_ge21","1");
      sal_config_set("phy_fiber_pref_ge22","1");
      sal_config_set("phy_fiber_pref_ge23","1");
      sal_config_set("phy_fiber_pref_ge24","1");
      sal_config_set("phy_fiber_pref_ge25","1");
      sal_config_set("phy_fiber_pref_ge26","1");
      sal_config_set("phy_fiber_pref_ge27","1");
      sal_config_set("phy_fiber_pref_ge28","1");
      sal_config_set("phy_fiber_pref_ge29","1");
      sal_config_set("phy_fiber_pref_ge30","1");
      sal_config_set("phy_fiber_pref_ge31","1");
      sal_config_set("phy_fiber_pref_ge32","1");
      sal_config_set("phy_fiber_pref_ge33","1");
      sal_config_set("phy_fiber_pref_ge34","1");
      sal_config_set("phy_fiber_pref_ge35","1");
      sal_config_set("phy_fiber_pref_ge36","1");
      sal_config_set("phy_fiber_pref_ge37","1");
      sal_config_set("phy_fiber_pref_ge38","1");
      sal_config_set("phy_fiber_pref_ge39","1");
      sal_config_set("phy_fiber_pref_ge40","1");
      sal_config_set("phy_fiber_pref_ge41","1");
      sal_config_set("phy_fiber_pref_ge42","1");
      sal_config_set("phy_fiber_pref_ge43","1");
      sal_config_set("phy_fiber_pref_ge44","1");
      sal_config_set("phy_fiber_pref_ge45","1");
      sal_config_set("phy_fiber_pref_ge46","1");
      sal_config_set("phy_fiber_pref_ge47","1");
      sal_config_set("phy_automedium_ge0","1");
      sal_config_set("phy_automedium_ge1","1");
      sal_config_set("phy_automedium_ge2","1");
      sal_config_set("phy_automedium_ge3","1");
      sal_config_set("phy_automedium_ge4","1");
      sal_config_set("phy_automedium_ge5","1");
      sal_config_set("phy_automedium_ge6","1");
      sal_config_set("phy_automedium_ge7","1");
      sal_config_set("phy_automedium_ge8","1");
      sal_config_set("phy_automedium_ge9","1");
      sal_config_set("phy_automedium_ge10","1");
      sal_config_set("phy_automedium_ge11","1");
      sal_config_set("phy_automedium_ge12","1");
      sal_config_set("phy_automedium_ge13","1");
      sal_config_set("phy_automedium_ge14","1");
      sal_config_set("phy_automedium_ge15","1");
      sal_config_set("phy_automedium_ge16","1");
      sal_config_set("phy_automedium_ge17","1");
      sal_config_set("phy_automedium_ge18","1");
      sal_config_set("phy_automedium_ge19","1");
      sal_config_set("phy_automedium_ge20","1");
      sal_config_set("phy_automedium_ge21","1");
      sal_config_set("phy_automedium_ge22","1");
      sal_config_set("phy_automedium_ge23","1");
      sal_config_set("phy_automedium_ge24","1");
      sal_config_set("phy_automedium_ge25","1");
      sal_config_set("phy_automedium_ge26","1");
      sal_config_set("phy_automedium_ge27","1");
      sal_config_set("phy_automedium_ge28","1");
      sal_config_set("phy_automedium_ge29","1");
      sal_config_set("phy_automedium_ge30","1");
      sal_config_set("phy_automedium_ge31","1");
      sal_config_set("phy_automedium_ge32","1");
      sal_config_set("phy_automedium_ge33","1");
      sal_config_set("phy_automedium_ge34","1");
      sal_config_set("phy_automedium_ge35","1");
      sal_config_set("phy_automedium_ge36","1");
      sal_config_set("phy_automedium_ge37","1");
      sal_config_set("phy_automedium_ge38","1");
      sal_config_set("phy_automedium_ge39","1");
      sal_config_set("phy_automedium_ge40","1");
      sal_config_set("phy_automedium_ge41","1");
      sal_config_set("phy_automedium_ge42","1");
      sal_config_set("phy_automedium_ge43","1");
      sal_config_set("phy_automedium_ge44","1");
      sal_config_set("phy_automedium_ge45","1");
      sal_config_set("phy_automedium_ge46","1");
      sal_config_set("phy_automedium_ge47","1");
      sal_config_set("phy_port_primary_and_offset_ge0","0x0000");
      sal_config_set("phy_port_primary_and_offset_ge1","0x0001");
      sal_config_set("phy_port_primary_and_offset_ge2","0x0002");
      sal_config_set("phy_port_primary_and_offset_ge3","0x0003");
      sal_config_set("phy_port_primary_and_offset_ge4","0x0004");
      sal_config_set("phy_port_primary_and_offset_ge5","0x0005");
      sal_config_set("phy_port_primary_and_offset_ge6","0x0006");
      sal_config_set("phy_port_primary_and_offset_ge7","0x0007");
      sal_config_set("phy_port_primary_and_offset_ge8","0x0800");
      sal_config_set("phy_port_primary_and_offset_ge9","0x0801");
      sal_config_set("phy_port_primary_and_offset_ge10","0x0802");
      sal_config_set("phy_port_primary_and_offset_ge11","0x0803");
      sal_config_set("phy_port_primary_and_offset_ge12","0x0804");
      sal_config_set("phy_port_primary_and_offset_ge13","0x0805");
      sal_config_set("phy_port_primary_and_offset_ge14","0x0806");
      sal_config_set("phy_port_primary_and_offset_ge15","0x0807");
      sal_config_set("phy_port_primary_and_offset_ge16","0x1000");
      sal_config_set("phy_port_primary_and_offset_ge17","0x1001");
      sal_config_set("phy_port_primary_and_offset_ge18","0x1002");
      sal_config_set("phy_port_primary_and_offset_ge19","0x1003");
      sal_config_set("phy_port_primary_and_offset_ge20","0x1004");
      sal_config_set("phy_port_primary_and_offset_ge21","0x1005");
      sal_config_set("phy_port_primary_and_offset_ge22","0x1006");
      sal_config_set("phy_port_primary_and_offset_ge23","0x1007");
      sal_config_set("phy_port_primary_and_offset_ge24","0x1800");
      sal_config_set("phy_port_primary_and_offset_ge25","0x1801");
      sal_config_set("phy_port_primary_and_offset_ge26","0x1802");
      sal_config_set("phy_port_primary_and_offset_ge27","0x1803");
      sal_config_set("phy_port_primary_and_offset_ge28","0x1804");
      sal_config_set("phy_port_primary_and_offset_ge29","0x1805");
      sal_config_set("phy_port_primary_and_offset_ge30","0x1806");
      sal_config_set("phy_port_primary_and_offset_ge31","0x1807");
      sal_config_set("phy_port_primary_and_offset_ge32","0x2000");
      sal_config_set("phy_port_primary_and_offset_ge33","0x2001");
      sal_config_set("phy_port_primary_and_offset_ge34","0x2002");
      sal_config_set("phy_port_primary_and_offset_ge35","0x2003");
      sal_config_set("phy_port_primary_and_offset_ge36","0x2004");
      sal_config_set("phy_port_primary_and_offset_ge37","0x2005");
      sal_config_set("phy_port_primary_and_offset_ge38","0x2006");
      sal_config_set("phy_port_primary_and_offset_ge39","0x2007");
      sal_config_set("phy_port_primary_and_offset_ge40","0x2800");
      sal_config_set("phy_port_primary_and_offset_ge41","0x2801");
      sal_config_set("phy_port_primary_and_offset_ge42","0x2802");
      sal_config_set("phy_port_primary_and_offset_ge43","0x2803");
      sal_config_set("phy_port_primary_and_offset_ge44","0x2804");
      sal_config_set("phy_port_primary_and_offset_ge45","0x2805");
      sal_config_set("phy_port_primary_and_offset_ge46","0x2806");
      sal_config_set("phy_port_primary_and_offset_ge47","0x2807");
    } else { 
      sal_config_set("ucode_port.port1.0", "il0.il100.0.0:il1.il100.0.0");
      sal_config_set("ucode_num_ports.0", "1");
      sal_config_set("xgxs_rx_lane_map_core0_il0", "0x1230");
      sal_config_set("xgxs_rx_lane_map_core1_il0", "0x1032");
      sal_config_set("xgxs_rx_lane_map_core2_il0", "0x1230");
      sal_config_set("xgxs_tx_lane_map_core0_il0", "0x1032");
      sal_config_set("xgxs_tx_lane_map_core1_il0", "0x3210");
      sal_config_set("xgxs_tx_lane_map_core2_il0", "0x1032");
      sal_config_set("phy_xaui_rx_polarity_flip_il0", "0x0bf9");
      sal_config_set("phy_xaui_tx_polarity_flip_il0", "0x0cf3");
      sal_config_set("xgxs_rx_lane_map_core0_il1", "0x1032");
      sal_config_set("xgxs_rx_lane_map_core1_il1", "0x3210");
      sal_config_set("xgxs_rx_lane_map_core2_il1", "0x1032");
      sal_config_set("xgxs_tx_lane_map_core0_il1", "0x1032");
      sal_config_set("xgxs_tx_lane_map_core1_il1", "0x3210");
      sal_config_set("xgxs_tx_lane_map_core2_il1", "0x1032");
    }

    rv = soc_reset_init(unit);
    if (SOC_FAILURE(rv)) {
        test_error(unit, "Unit %d: soc reset failed got (%d)\n", unit, rv);
        return -1;
    }
    rv = bcm_init(unit);
    if (SOC_FAILURE(rv)) {
        test_error(unit, "Unit %d: init bcm failed got (%d)\n", unit, rv);
        return -1;
    }
    test_msg("Please wait while setting up test env\n");
    
    /*sal_config_set("ddr3_auto_tune", "1");*/
    return c3hppc_test_init(unit, args, pa); 

}
int
sbx_c3_sys_test_172_init(int unit, args_t *args, void **pa)
{
    int rv = SOC_E_NONE;
    c3testinfo_t *testinfo = &c3TestInfo[unit];
    sbx_c3_test_board_detect(testinfo);
    if (testinfo->i2c_board == BOARD_48G) {
        sal_config_set("c3_ucode_test", "172_48G");
    } else {
        sal_config_set("c3_ucode_test", "172_100G");
    }
    rv = sbx_c3_sys_test_init_internal(unit, args, pa);
    return rv;
}

int
sbx_c3_sys_test_173_init(int unit, args_t *args, void **pa)
{
    int rv = SOC_E_NONE;
    c3testinfo_t *testinfo = &c3TestInfo[unit];
    sbx_c3_test_board_detect(testinfo);
    if (testinfo->i2c_board == BOARD_48G) {
        sal_config_set("c3_ucode_test", "173_48G");
    } else {
        sal_config_set("c3_ucode_test", "173_100G");
    }
    rv  = sbx_c3_sys_test_init_internal(unit, args, pa);
    return rv;
}


/*
 * Function:
 *      sbx_c3_sys_test_run
 *
 * Purpose:
 *      Runs System  test
 *
 * Parameters:
 *      unit            - unit to test
 *      a               - test arguments (ignored)
 *      pa              - test cookie (ignored)
 *
 * Returns:
 *      TEST_OK
 *      TEST_FAIL
 *
 */

int
sbx_c3_sys_test_run(int unit, args_t *a, void *pa)
{
    test_msg("Starting system test\n");
    return c3hppc_test_run(unit, a, pa);
}

/*
 * Function:
 *      sbx_c3_sys_test_done
 *
 * Purpose:
 *      Cleans up 
 *
 * Parameters:
 *      unit            - unit to test
 *      pa              - test cookie (ignored)
 *
 * Returns:
 *      TEST_OK
 *      TEST_FAIL
 */

int
sbx_c3_sys_test_done(int unit, void *pa)
{
    c3testinfo_t *testinfo = &c3TestInfo[unit];

    c3hppc_test_done(unit, pa);
    test_msg("Please wait while refreshing system config \n");
    sal_config_refresh();
    test_msg("Completed System test\n");
    sbx_c3_test_reset_init(testinfo, 1);
    return 0;
}



#endif /* #ifdef BCM_CALADAN3_SUPPORT */

