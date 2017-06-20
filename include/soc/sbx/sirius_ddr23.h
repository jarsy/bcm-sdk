/*
 * $Id: sirius_ddr23.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        sirius_ddr.h
 * Purpose:     Sirius ddr definitions
 * Requires:
 */

#ifndef _SIRIUS_DDR23_H_
#define _SIRIUS_DDR23_H_

#include <soc/sbx/sirius.h>
#include <sal/core/sync.h>
#include <sal/core/thread.h>

/* need to check on this */
#define INTERVAL_TOLERANCE 100

/* addresses in rdb files  */
#define DDR23_PHY_ADDR_CTL_MIN (0x0)
#define DDR23_PHY_ADDR_CTL_MAX (0x0044)
#define DDR23_PHY_BYTE_LANE0_ADDR_MIN (0x0100)
#define DDR23_PHY_BYTE_LANE0_ADDR_MAX (0x014c)
#define DDR23_PHY_BYTE_LANE1_ADDR_MIN (0x0200)
#define DDR23_PHY_BYTE_LANE1_ADDR_MAX (0x024c)

#define DDR23_PHY_ADDR_CTL_BLOCK (0)
#define DDR23_PHY_BYTE_LANE0_BLOCK (1)
#define DDR23_PHY_BYTE_LANE1_BLOCK (2)

#define DDR23_READ_EN_VDL_WIDTH (64)

/* between two passing tread_en windows, there is a gap
 * of about 78 (estimated) that gap is included in when
 * calculating the longest_span of passing results between
 * tread_en windows */

#define TREAD_EN_GAP_WIDTH (78)


/*
 * Tuning results per ci
 */

typedef struct ci_tune_params_s {
  uint32 read_vdl;
  uint32 read_en_vdl;
  uint32 tread_en;
  uint8  valid;
} ci_tune_params_t;

/*
 * General Utility Macros
 */
#ifdef UNIT_VALID_CHECK
#undef UNIT_VALID_CHECK
#endif
#define UNIT_VALID_CHECK(_unit) \
    if (((_unit) < 0) || ((_unit) >= SOC_MAX_NUM_DEVICES)) { \
        return SOC_E_UNIT; \
    }

#ifdef UNIT_INIT_CHECK
#undef UNIT_INIT_CHECK
#endif
#define UNIT_INIT_CHECK(_unit)  \
    do { \
        UNIT_VALID_CHECK(_unit);  \
        if (DDR_TRAINING_CONTROL(_unit) == NULL) { return SOC_E_INIT; } \
    } while (0)

/*  DRAM Address to PLA_ADDR mapping
 *  bank[2:0] --> pla_addr[0:2]
 *  col[10:4] --> pla_addr[3:9]
 *  row[13:0] --> pla_addr[10:23]
 */

#define SIRIUS_DDR23_BANK(_addr) \
  (_addr & 0x7)

#define SIRIUS_DDR23_COL(_addr) \
  ((_addr >> 0x3) & 0x7f) << 4 

#define SIRIUS_DDR23_ROW(_addr) \
  (_addr >> 10) & 0x3fff

extern int 
soc_sbx_sirius_sw_ddr23_train_init(int unit, siriusInitParams_t *pInitParams);
extern int 
soc_sbx_sirius_sw_ddr23_train_start(int unit, uint32 flags, int interval);
extern int 
soc_sbx_sirius_sw_ddr23_train_stop(int unit);
extern int 
soc_sbx_sirius_sw_ddr23_train_detach(int unit);
extern int 
soc_sbx_sirius_ddr23_write(int unit, int ci, uint32 addr,
			   uint32 data0, uint32 data1, uint32 data2,
			   uint32 data3, uint32 data4, uint32 data5,
			   uint32 data6, uint32 data7);
extern int 
soc_sbx_sirius_ddr23_read(int unit, int ci, uint32 addr,
			  uint32 *pData0, uint32 *pData1, uint32 *pData2,
			  uint32 *pData3, uint32 *pData4, uint32 *pData5,
			  uint32 *pData6, uint32 *pData7);

extern int
soc_sbx_sirius_ddr23_vdl_set(int unit, int ci, uint block, uint vdl, 
			     uint tap, uint fine);

extern void
soc_sbx_sirius_ddr23_phy_tune_best_setting(int window, ci_tune_params_t *pCiTune,uint8 *p);



extern 
int soc_sbx_sirius_ddr23_done(int unit,
			      int ci,
			      uint32 uTimeout);
#endif /* _SIRIUS_DDR_H_ */
