/* 
 * $Id: qe2000_scoreboard.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        scoreboard.h
 * Purpose:     SBX Software scoreboard Collection module definitions.
 */

#ifndef   _SOC_SBX_SCOREBOARD_H_
#define   _SOC_SBX_SCOREBOARD_H_

#include <sal/types.h>
#include <soc/types.h>

/*
 * SCOREBOARD Block Definition
 *
 * Contains definition for counter blocks available in a device.
 *
 * Definitions / Assumptions:
 *   - a 'block' contains 1 or more equal 'sets' of counters,
 *   - a 'set' contains 1 or more 'counters'
 */

#define QE2000_SB_NBUFS_IN_BYTE                 8
#define QE2000_SB_HALFBUS_256_BIT2IGNORE        0x04
#define QE2000_SB_FULLBUS_256_BIT2IGNORE        0x08
#define QE2000_SB_BUFS_DELTA                    0


#define QE2000_BUF_ALLOC_PER_TS_0               0x00
#define QE2000_BUF_ALLOC_PER_TS_1               0x01
#define QE2000_BUF_ALLOC_PER_TS_2               0x02
#define QE2000_BUF_ALLOC_PER_TS_DISABLE         QE2000_BUF_ALLOC_PER_TS_0
#define QE2000_BUF_ALLOC_PER_TS_DEFAULT         QE2000_BUF_ALLOC_PER_TS_1
#define QE2000_BUF_ALLOC_PER_TS                 QE2000_BUF_ALLOC_PER_TS_2

/*
 * 2 sec   -> (0x01FCA055)
 * 1 sec   -> (0x00FE502A)
 * .5 sec  -> (0x007F2815)
 * .25 sec -> (0x003F940A)
 */
#define QE2000_HW_AGE_SCALER_2SW_OFFSET_VAL     (256)            /* 15 micro sec */
#define QE2000_HW_AGE_SCALER_DEF_VAL            (0x01FCA055)     /* 2 Sec */
#define QE2000_HW_AGE_FIC_SCALER_DEF_VAL        (0x01FCA055 - 1) /* 2 Sec */
#define QE2000_HW_AGE_TME_SCALER_DEF_VAL        (0x007F2815)
#define QE2000_MIN_SCOREBOARD_POLL_INTVL_US     100000 /* 100 milli seconds */
#define QE2000_SCOREBOARD_FIC_POLL_FACTOR       2
#define QE2000_SCOREBOARD_TME_POLL_FACTOR       2

#define QE2000_BUF_AVL_FULLBUS_512_CONFIG       0x1FE00
#define QE2000_BUF_AVL_HALFBUS_512_CONFIG       0x0FC00
#define QE2000_BUF_AVL_FULLBUS_256_CONFIG       0x03000
#define QE2000_BUF_AVL_HALFBUS_256_CONFIG       0x01800

#define QE2000_FB_CACHE_EXT_CACHE_ACCESS_SZ     192

#define QE2000_QM_MEM_ACC_DATA0_QS_ENABLE_MASK  0x01
#define QE2000_QM_MEM_ACC_DATA0_QS_ENABLE_SHFT  0x00

#define QE2000_MARKED_BUFFER_NUMBER             0x1FFFF
#define QE2000_MARKED_BUFFER_CHECK_THRESHOLD    256 /* 1024 */

#define QE2000_QM_WRITE_RETRY_COUNT             10
#define QE2000_QS_WRITE_RETRY_COUNT             10

#define QE2000_QUEUESIZE_NORMALIZE(x)           (((x)>>3) << ((x)&0x7))

#define SB_FAB_NULL_POINTER ((void*)0)

#ifdef NOT_NEEDED
typedef int (*fn_scorebd_read)(int unit, int set, int counter, uint64 *val, int *width);
typedef int (*fn_scorebd_write)(int unit, int set, int counter, uint64 val);
#endif

typedef struct soc_sbx_scoreboard_block_info_s {
    int           block;         /* ID for a counter block */
    int           num_sets;      /* Number of counter sets in block */
    int           num_counters;  /* Number of counters in each set */
#ifdef NOT_NEEDED
    fn_read       read;          /* Function to read a given counter */
    fn_write      write;         /* Function to write a given counter */
#endif
} soc_sbx_scoreboard_block_info_t;


/*
 * External Functions
 */
extern int soc_sbx_qe2000_scoreboard_init(int unit);
#ifdef NOT_NEEDED
                                soc_sbx_scoreboard_block_info_t *block_info,
                                int block_count);
#endif

extern int soc_sbx_scoreboard_detach(int unit);

extern int soc_sbx_scoreboard_bset_add(int unit, int block, int set);

extern int soc_sbx_scoreboard_start(int unit, uint32 flags, int interval);
extern int soc_sbx_scoreboard_stop(int unit);
extern int soc_sbx_scoreboard_sync(int unit);

extern int soc_sbx_scoreboard_get(int unit, int block, int set,
                               int counter, uint64 *val);

#ifdef NOT_NEEDED
extern int soc_sbx_scoreboard_get_zero(int unit, int block, int set,
                                    int counter, uint64 *val);

extern int soc_sbx_scoreboard_set(int unit, int block, int set,
                               int counter, uint64 val);
#endif

extern int soc_sbx_scoreboard_dump(int unit);

extern int soc_sbx_scoreboard_bset_clear(int unit, int block);

extern int soc_qe2000_pkt_age_set(int unit, int value);

extern int soc_qe2000_pkt_age_get(int unit, int *value);

extern int soc_qe2000_scoreboard_stats_get(int unit, uint32 *pBufLost, uint32 *pBufFrees,
                uint32 *pWatchdogErrs, uint32 *pShortIntervals, uint32 *pScoreboardTicks);

#endif /* _SOC_SBX_SCOREBOARD_H_ */
