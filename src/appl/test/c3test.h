/*
 * $Id: c3test.h,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

typedef struct c3testinfo_s {
  int unit;
  int board_type;
  /* TCAM Test Parameters */
  uint32 tcam_testid;
  uint32 tcam_busid;
  uint32 tcam_testdev;
  uint32 tcam_testaddr;
  /* I2C Test Parameters */
  uint32 i2c_mode;
  uint32 i2c_test;
  uint32 i2c_board;
  int32 i2c_muxid;
  int32 i2c_muxch;
  /* MDIO Test Parameters */
  uint32 mdio_mode;
  uint32 mdio_test;
  uint32 mdio_clause45;
  uint32 mdio_sbusid;
  uint32 mdio_lbusid;
  uint32 mdio_sphyid;
  uint32 mdio_lphyid;
  uint32 mdio_devaddr;
  uint32 mdio_rd_reg;
  uint32 mdio_wr_reg;
  uint32 mdio_wrdata;
  uint32 mdio_ecmpdata;
  uint32 mdio_icmpdata;
} c3testinfo_t;
