/*
 * $Id: TkDefs.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        name.h
 * Purpose:     Purpose of the file
 */

#ifndef _SOC_TKDEFS_H_
#define _SOC_TKDEFS_H_

#include <soc/ea/tk371x/TkConfig.h>

#define SDK_PORT_VEC_BASE       1
#define SDK_MAX_NUM_OF_PORT     0x10

#define SDK_LINK_VEC_BASE       0
#define SDK_MAX_NUM_OF_LINK     8

#define MAX_NUM_OF_PON_CHIP     7

#define MAX_CNT_OF_UP_QUEUE     20
#define MAX_CNT_OF_DOWN_QUEUE   20
#define MAX_CNT_OF_LINK         8
#define MAX_CNT_OF_PORT 		2
#define CNT_OF_FAIL_SAFE        1

#define MAX_ARL_TAB_SIZE        64

typedef enum soc_ea_block_type_e {
    SOC_EA_BLK_NONE,
    SOC_EA_BLK_PON,
    SOC_EA_BLK_GE,
    SOC_EA_BLK_FE,
    SOC_EA_BLK_LLID,
    SOC_EA_TYPE_INVALID = -1
} soc_ea_block_type_t;

#endif

