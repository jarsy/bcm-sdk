/*
 * $Id: bm9600.h,v 1.21 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _BM9600_H
#define _BM9600_H

#include "sbTypes.h"
#include "bm9600_properties.h"

#include <soc/sbx/bme3200.h>

/* Future versions of Polaris may not require this */
#if 0
#define _NO_NM_CACHE_FIX_        1
#endif /* 0 */

/* if the following is defined INA block will always be enabled */
#if 0
#define _BM9600_INA_ENABLE_ON_
#endif /* 0 */

#define SB_FAB_DEVICE_BM9600_MAX_WRED_TEMPLATES          (BM9600_BW_WRED_CURVE_TABLE_SIZE)
#define SB_FAB_DEVICE_BM9600_MAX_DS_IDS                  (BM9600_MAX_MULTICAST_EXTENDED_ESETS)
#define SB_FAB_DEVICE_BM9600_MAX_NODES                   (BM9600_NUM_NODES)

#define SB_FAN_DEVICE_BM9600_MAX_NODE_LOW_ESET_FLD       (63)


/* bw repository 0 only */

#define SB_FAB_DEVICE_BM9600_BANDWIDTH_TABLE_WDT   0
#define SB_FAB_DEVICE_BM9600_BANDWIDTH_TABLE_QLOP  1
#define SB_FAB_DEVICE_BM9600_BANDWIDTH_TABLE_LTHR  2
#define SB_FAB_DEVICE_BM9600_BANDWIDTH_TABLE_NPC2Q 3
#define SB_FAB_DEVICE_BM9600_BANDWIDTH_TABLE_Q2NPC 4
#define SB_FAB_DEVICE_BM9600_BANDWIDTH_TABLE_BWP   5
#define SB_FAB_DEVICE_BM9600_BANDWIDTH_TABLE_R0    6

/* bw repository 0 and 1 */
#define SB_FAB_DEVICE_BM9600_BANDWIDTH_TABLE_WAT   7
#define SB_FAB_DEVICE_BM9600_BANDWIDTH_TABLE_DST   8
#define SB_FAB_DEVICE_BM9600_BANDWIDTH_TABLE_PRT   9

/* bw repository 1 only */
#define SB_FAB_DEVICE_BM9600_BANDWIDTH_TABLE_WST  10
#define SB_FAB_DEVICE_BM9600_BANDWIDTH_TABLE_WCT  11
#define SB_FAB_DEVICE_BM9600_BANDWIDTH_TABLE_QLT  12
#define SB_FAB_DEVICE_BM9600_BANDWIDTH_TABLE_R1   13

#define SB_FAB_DEVICE_BM9600_BANDWIDTH_TABLE_NUM_REPOSITORIES 14
#define SB_FAB_DEVICE_BM9600_BANDWIDTH_TABLE_MAX_REPOSITORY_SIZE 0x8000
#define SB_FAB_DEVICE_BM9600_BANDWIDTH_TABLE_NUM_BANKS   2

#define SB_FAB_DEVICE_BM9600_NM_SYSPORT_ARRAY_TABLE_ENTRY_INVALID (0xFFF)

#define SB_FAB_DEVICE_BM9600_EPOCH_PRIME_PIPE                 6
#define SB_FAB_DEVICE_BM9600_EPOCH_WRED_KILL                  8
#define SB_FAB_DEVICE_BM9600_EPOCH_ALLOC_PRIME_PIPE           32

/* Hypercore register shifts and masks */
#define BM9600_HYPERCORE_8017_8B10B_SHIFT           12
#define BM9600_HYPERCORE_8017_KCHAR_SHIFT            8
#define BM9600_HYPERCORE_8019_PRBS_INVERT_SHIFT      2
#define BM9600_HYPERCORE_8019_PRBS_ENABLE_SHIFT      3
#define BM9600_HYPERCORE_8019_PRBS_POLY_SHIFT        0

#define BM9600_HYPERCORE_8019_LANE_SHIFT             4
#define BM9600_HYPERCORE_80B1_STATUS_MASK            7
#define BM9600_HYPERCORE_80B1_PRBS_STATUS_VALUE      7
#define BM9600_HYPERCORE_80B1_SIGDET_STATUS_VALUE    0

#define BM9600_HYPERCORE_8065_TX_SEL_HALF_RATE_MASK  1
#define BM9600_HYPERCORE_8065_TX_SEL_HALF_RATE_SHIFT 3

#define BM9600_HYPERCORE_8000_PLL_SEQUENCER_MASK    0x2000
#define BM9600_HYPERCORE_833C_8B10B_SCRAMBLER_MASK  0x0100


/*
 * NM Memory Cache definitions
 */
#define BM9600_NMEMTENTRY_MAX_INDEX                  1024
#define BM9600_NMEMTENTRY_SIZE                       3
#define BM9600_NMEMTENTRY_DEFAULT_VALUE              0

#define BM9600_NMPORTSETINFOENTRY_MAX_INDEX          176
#define BM9600_NMPORTSETINFOENTRY_SIZE               1
#define BM9600_NMPORTSETINFOENTRY_DEFAULT_VALUE      0 

#define BM9600_NMPORTSETLINKENTRY_MAX_INDEX          176
#define BM9600_NMPORTSETLINKENTRY_SIZE               1
#define BM9600_NMPORTSETLINKENTRY_DEFAULT_VALUE      0xFF

#define BM9600_NMSYSPORTARRAYENTRY_MAX_INDEX         176
#define BM9600_NMSYSPORTARRAYENTRY_SIZE              6
#define BM9600_NMSYSPORTARRAYENTRY_DEFAULT_VALUE     0

#define SB_FAB_DEVICE_BM9600_NUM_AI_PORTS            72

/*
 * INA Memory Cache definitions
 */
#define BM9600_INASYSPORTENTRY_MAX_INDEX             2816
#define BM9600_INASYSPORTENTRY_PORTSET_DEFAULT_VALUE 0xFF
#define BM9600_INASYSPORTENTRY_OFFSET_DEFAULT_VALUE  0xF


/* general definations */
#define SB_FAB_DEVICE_BM9600_WRD0_LOW_NODE           0
#define SB_FAB_DEVICE_BM9600_WRD0_HI_NODE            31
#define SB_FAB_DEVICE_BM9600_WRD1_LOW_NODE           32
#define SB_FAB_DEVICE_BM9600_WRD1_HI_NODE            63
#define SB_FAB_DEVICE_BM9600_WRD2_LOW_NODE           64
#define SB_FAB_DEVICE_BM9600_WRD2_HI_NODE            71


/* These defines are the timeouts for acks on all BM9600 init code */
/* these can be updated to increase timeout.                       */
#define HW_BM9600_TIMEOUT_GENERAL           100000 /* usec */
#define HW_BM9600_POLL_GENERAL              1000   /* minimum polls */


/* definitios for Node types used for various devices */
#define SB_FAB_DEVICE_BM9600_NODE_TYPE0     0x00
             /* current usage, bcmModuleProtocol3, bcmModuleProtocol4, bcmModuleProtocol5 */
#define SB_FAB_DEVICE_BM9600_NODE_TYPE1     0x01
             /* current usage, bcmModuleProtocol1, bcmModuleProtocol2 */
#define SB_FAB_DEVICE_BM9600_NODE_TYPE2     0x02
#define SB_FAB_DEVICE_BM9600_NODE_TYPE3     0x03
            /* current usage, bcmModuleProtocolCustom1 */


typedef struct bm9600_NmEmtCache_s {
    uint32    value[BM9600_NMEMTENTRY_MAX_INDEX][BM9600_NMEMTENTRY_SIZE];
} bm9600_NmEmtCache_t;

typedef struct bm9600_NmPortsetInfoCache_s {
    uint32    value[BM9600_NMPORTSETINFOENTRY_MAX_INDEX][BM9600_NMPORTSETINFOENTRY_SIZE];
} bm9600_NmPortsetInfoCache_t;

typedef struct bm9600_NmPortsetLinkCache_s {
    uint32    value[BM9600_NMPORTSETLINKENTRY_MAX_INDEX][BM9600_NMPORTSETLINKENTRY_SIZE];
} bm9600_NmPortsetLinkCache_t;

typedef struct bm9600_NmSysportArrayCache_s {
    uint32    value[BM9600_NMSYSPORTARRAYENTRY_MAX_INDEX][BM9600_NMSYSPORTARRAYENTRY_SIZE];
} bm9600_NmSysportArrayCache_t;

typedef struct bm9600_NmCache_s {
    bm9600_NmEmtCache_t             emt;
    bm9600_NmPortsetInfoCache_t     porsetInfo;
    bm9600_NmPortsetLinkCache_t     portsetlink;
    bm9600_NmSysportArrayCache_t    sysportArray;
} bm9600_NmCache_t;

typedef struct bm9600_InaSysportCache_s {
    uint8 portset_index[BM9600_INASYSPORTENTRY_MAX_INDEX];
    uint8 offset[BM9600_INASYSPORTENTRY_MAX_INDEX];
} bm9600_InaSysportCache_t;

typedef struct bm9600_InaCache_s {
    bm9600_InaSysportCache_t             sysport;
} bm9600_InaCache_t;


int
soc_bm9600_FoLinkStateTableRead( UINT uBaseAddress,
                                      UINT uAddress,
                                      UINT *pData0,
                                      UINT *pData1);
int
soc_bm9600_FoLinkStateTableWrite( UINT uBaseAddress,
                                       UINT uAddress,
                                       UINT uData0,
                                       UINT uData1);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_FoLinkStateTableClear( UINT uBaseAddress);
int
soc_bm9600_FoLinkStateTableFillPattern( UINT uBaseAddress,
                                             UINT uData0,
                                             UINT uData1);
#endif
int
soc_bm9600_NmEgressRankerRead( UINT uBaseAddress,
                                    UINT uAddress,
                                    UINT *pData);
int
soc_bm9600_NmEgressRankerWrite( UINT uBaseAddress,
                                     UINT uAddress,
                                     UINT uData);

int
soc_bm9600_NmFullStatusRead( UINT uBaseAddress,
                                    UINT uAddress,
                                    UINT *pData0,
                                    UINT *pData1,
                                    UINT *pData2,
                                    UINT *pData3,
                                    UINT *pData4,
                                    UINT *pData5,
                                    UINT *pData6,
                                    UINT *pData7,
                                    UINT *pData8);
int
soc_bm9600_NmFullStatusWrite( UINT uBaseAddress,
                                     UINT uAddress,
                                     UINT uData0,
                                     UINT uData1,
                                     UINT uData2,
                                     UINT uData3,
                                     UINT uData4,
                                     UINT uData5,
                                     UINT uData6,
                                     UINT uData7,
                                     UINT uData8);


#ifdef FUNCTION_NOW_USED
int
soc_bm9600_NmEgressRankerClear( UINT uBaseAddress);
int
soc_bm9600_NmEgressRankerFillPattern( UINT uBaseAddress,
                                           UINT uData);
#endif

int
soc_bm9600_NmEmtRead( UINT uBaseAddress,
                           UINT uAddress,
                           UINT *pData0,
                           UINT *pData1,
                           UINT *pData2);

int
soc_bm9600_HwNmEmtRead( UINT uBaseAddress,
                           UINT uAddress,
                           UINT *pData0,
                           UINT *pData1,
                           UINT *pData2);

int
soc_bm9600_NmEmtWrite( UINT uBaseAddress,
                            UINT uAddress,
                            UINT uData0,
                            UINT uData1,
                            UINT uData2);


int
soc_bm9600_HwNmEmtWrite( UINT uBaseAddress,
                            UINT uAddress,
                            UINT uData0,
                            UINT uData1,
                            UINT uData2);


#ifdef FUNCTION_NOW_USED
int
soc_bm9600_NmEmtClear( UINT uBaseAddress);
#endif

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_NmEmtFillPattern( UINT uBaseAddress,
                                  UINT uData0,
                                  UINT uData1,
                                  UINT uData2);
#endif

int
soc_bm9600_NmEmtdebugbank0Read( UINT uBaseAddress,
                                     UINT uAddress,
                                     UINT *pData0,
                                     UINT *pData1,
                                     UINT *pData2);
int
soc_bm9600_NmEmtdebugbank0Write( UINT uBaseAddress,
                                      UINT uAddress,
                                      UINT uData0,
                                      UINT uData1,
                                      UINT uData2);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_NmEmtdebugbank0Clear( UINT uBaseAddress);
#endif

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_NmEmtdebugbank0FillPattern( UINT uBaseAddress,
                                            UINT uData0,
                                            UINT uData1,
                                            UINT uData2);
#endif

int
soc_bm9600_NmEmtdebugbank1Read( UINT uBaseAddress,
                                     UINT uAddress,
                                     UINT *pData0,
                                     UINT *pData1,
                                     UINT *pData2);
int
soc_bm9600_NmEmtdebugbank1Write( UINT uBaseAddress,
                                      UINT uAddress,
                                      UINT uData0,
                                      UINT uData1,
                                      UINT uData2);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_NmEmtdebugbank1Clear( UINT uBaseAddress);
int
soc_bm9600_NmEmtdebugbank1FillPattern( UINT uBaseAddress,
                                            UINT uData0,
                                            UINT uData1,
                                            UINT uData2);
#endif

int
soc_bm9600_NmIngressRankerRead( UINT uBaseAddress,
                                     UINT uAddress,
                                     UINT *pData);
int
soc_bm9600_NmIngressRankerWrite( UINT uBaseAddress,
                                      UINT uAddress,
                                      UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_NmIngressRankerClear( UINT uBaseAddress);
int
soc_bm9600_NmIngressRankerFillPattern( UINT uBaseAddress,
                                            UINT uData);
#endif

int
soc_bm9600_NmPortsetInfoRead( UINT uBaseAddress,
                                   UINT uAddress,
                                   UINT *pData);
int
soc_bm9600_HwNmPortsetInfoRead( UINT uBaseAddress,
                                   UINT uAddress,
                                   UINT *pData);
int
soc_bm9600_NmPortsetInfoWrite( UINT uBaseAddress,
                                    UINT uAddress,
                                    UINT uData);
int
soc_bm9600_HwNmPortsetInfoWrite( UINT uBaseAddress,
                                    UINT uAddress,
                                    UINT uData);

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_NmPortsetInfoClear( UINT uBaseAddress);
int
soc_bm9600_NmPortsetInfoFillPattern( UINT uBaseAddress,
                                          UINT uData);
#endif
int
soc_bm9600_NmPortsetLinkRead( UINT uBaseAddress,
                                   UINT uAddress,
                                   UINT *pData);
int
soc_bm9600_HwNmPortsetLinkRead( UINT uBaseAddress,
                                   UINT uAddress,
                                   UINT *pData);
int
soc_bm9600_NmPortsetLinkWrite( UINT uBaseAddress,
                                    UINT uAddress,
                                    UINT uData);
int
soc_bm9600_HwNmPortsetLinkWrite( UINT uBaseAddress,
                                    UINT uAddress,
                                    UINT uData);

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_NmPortsetLinkClear( UINT uBaseAddress);
int
soc_bm9600_NmPortsetLinkFillPattern( UINT uBaseAddress,
                                          UINT uData);
#endif

int
soc_bm9600_NmRandomNumGenRead( UINT uBaseAddress,
                                    UINT uAddress,
                                    UINT *pData);
int
soc_bm9600_NmRandomNumGenWrite( UINT uBaseAddress,
                                     UINT uAddress,
                                     UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_NmRandomNumGenClear( UINT uBaseAddress);
int
soc_bm9600_NmRandomNumGenFillPattern( UINT uBaseAddress,
                                           UINT uData);
#endif

int
soc_bm9600_NmSysportArrayRead( UINT uBaseAddress,
                                    UINT uAddress,
                                    UINT *pData0,
                                    UINT *pData1,
                                    UINT *pData2,
                                    UINT *pData3,
                                    UINT *pData4,
                                    UINT *pData5);
int
soc_bm9600_HwNmSysportArrayRead( UINT uBaseAddress,
                                    UINT uAddress,
                                    UINT *pData0,
                                    UINT *pData1,
                                    UINT *pData2,
                                    UINT *pData3,
                                    UINT *pData4,
                                    UINT *pData5);
int
soc_bm9600_NmSysportArrayWrite( UINT uBaseAddress,
                                     UINT uAddress,
                                     UINT uData0,
                                     UINT uData1,
                                     UINT uData2,
                                     UINT uData3,
                                     UINT uData4,
                                     UINT uData5);

int
soc_bm9600_HwNmSysportArrayWrite( UINT uBaseAddress,
                                     UINT uAddress,
                                     UINT uData0,
                                     UINT uData1,
                                     UINT uData2,
                                     UINT uData3,
                                     UINT uData4,
                                     UINT uData5);

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_NmSysportArrayClear( UINT uBaseAddress);
int
soc_bm9600_NmSysportArrayFillPattern( UINT uBaseAddress,
                                           UINT uData0,
                                           UINT uData1,
                                           UINT uData2,
                                           UINT uData3,
                                           UINT uData4,
                                           UINT uData5);
#endif

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_XbXcfgRemapRead( UINT uBaseAddress,
                                 UINT uAddress,
                                 UINT *pData);
int
soc_bm9600_XbXcfgRemapWrite( UINT uBaseAddress,
                                  UINT uAddress,
                                  UINT uData);
int
soc_bm9600_XbXcfgRemapClear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapFillPattern( UINT uBaseAddress,
                                        UINT uData);
#endif

int
soc_bm9600_XbXcfgRemapSelectRead(uint32 uBaseAddress,
                                 uint32 uAddress,
                                 uint32 uSelect,
                                 uint32 *pData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_XbXcfgRemapSelect0Read( UINT uBaseAddress,
                                        UINT uAddress,
                                        UINT *pData);
#endif

int
soc_bm9600_XbXcfgRemapSelectWrite(uint32 uBaseAddress,
                                  uint32 uAddress,
                                  uint32 uSelect,
                                  uint32 uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_XbXcfgRemapSelect0Write( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT uData);
#endif
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_XbXcfgRemapSelect0Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect0FillPattern( UINT uBaseAddress,
                                               UINT uData);
#endif

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_XbXcfgRemapSelect1Read( UINT uBaseAddress,
                                        UINT uAddress,
                                        UINT *pData);
#endif

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_XbXcfgRemapSelect1Write( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT uData);
#endif
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_XbXcfgRemapSelect1Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect1FillPattern( UINT uBaseAddress,
                                               UINT uData);
int
soc_bm9600_XbXcfgRemapSelect10Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect10Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect10Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect10FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect100Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect100Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect100Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect100FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect101Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect101Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect101Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect101FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect102Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect102Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect102Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect102FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect103Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect103Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect103Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect103FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect104Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect104Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect104Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect104FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect105Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect105Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect105Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect105FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect106Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect106Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect106Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect106FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect107Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect107Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect107Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect107FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect108Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect108Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect108Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect108FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect109Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect109Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect109Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect109FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect11Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect11Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect11Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect11FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect110Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect110Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect110Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect110FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect111Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect111Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect111Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect111FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect112Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect112Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect112Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect112FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect113Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect113Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect113Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect113FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect114Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect114Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect114Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect114FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect115Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect115Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect115Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect115FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect116Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect116Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect116Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect116FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect117Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect117Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect117Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect117FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect118Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect118Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect118Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect118FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect119Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect119Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect119Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect119FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect12Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect12Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect12Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect12FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect120Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect120Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect120Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect120FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect121Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect121Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect121Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect121FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect122Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect122Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect122Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect122FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect123Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect123Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect123Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect123FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect124Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect124Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect124Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect124FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect125Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect125Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect125Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect125FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect126Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect126Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect126Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect126FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect127Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect127Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect127Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect127FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect128Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect128Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect128Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect128FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect129Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect129Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect129Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect129FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect13Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect13Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect13Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect13FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect130Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect130Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect130Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect130FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect131Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect131Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect131Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect131FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect132Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect132Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect132Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect132FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect133Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect133Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect133Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect133FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect134Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect134Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect134Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect134FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect135Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect135Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect135Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect135FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect136Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect136Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect136Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect136FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect137Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect137Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect137Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect137FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect138Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect138Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect138Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect138FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect139Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect139Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect139Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect139FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect14Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect14Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect14Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect14FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect140Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect140Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect140Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect140FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect141Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect141Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect141Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect141FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect142Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect142Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect142Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect142FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect143Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect143Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect143Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect143FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect144Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect144Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect144Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect144FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect145Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect145Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect145Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect145FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect146Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect146Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect146Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect146FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect147Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect147Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect147Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect147FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect148Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect148Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect148Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect148FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect149Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect149Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect149Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect149FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect15Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect15Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect15Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect15FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect150Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect150Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect150Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect150FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect151Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect151Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect151Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect151FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect152Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect152Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect152Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect152FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect153Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect153Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect153Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect153FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect154Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect154Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect154Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect154FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect155Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect155Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect155Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect155FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect156Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect156Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect156Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect156FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect157Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect157Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect157Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect157FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect158Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect158Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect158Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect158FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect159Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect159Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect159Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect159FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect16Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect16Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect16Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect16FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect160Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect160Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect160Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect160FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect161Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect161Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect161Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect161FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect162Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect162Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect162Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect162FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect163Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect163Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect163Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect163FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect164Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect164Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect164Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect164FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect165Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect165Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect165Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect165FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect166Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect166Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect166Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect166FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect167Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect167Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect167Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect167FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect168Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect168Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect168Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect168FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect169Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect169Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect169Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect169FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect17Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect17Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect17Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect17FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect170Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect170Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect170Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect170FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect171Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect171Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect171Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect171FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect172Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect172Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect172Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect172FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect173Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect173Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect173Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect173FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect174Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect174Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect174Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect174FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect175Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect175Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect175Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect175FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect176Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect176Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect176Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect176FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect177Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect177Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect177Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect177FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect178Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect178Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect178Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect178FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect179Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect179Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect179Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect179FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect18Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect18Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect18Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect18FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect180Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect180Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect180Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect180FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect181Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect181Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect181Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect181FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect182Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect182Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect182Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect182FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect183Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect183Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect183Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect183FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect184Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect184Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect184Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect184FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect185Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect185Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect185Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect185FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect186Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect186Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect186Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect186FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect187Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect187Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect187Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect187FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect188Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect188Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect188Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect188FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect189Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect189Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect189Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect189FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect19Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect19Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect19Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect19FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect190Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect190Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect190Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect190FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect191Read( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect191Write( UINT uBaseAddress,
                                           UINT uAddress,
                                           UINT uData);
int
soc_bm9600_XbXcfgRemapSelect191Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect191FillPattern( UINT uBaseAddress,
                                                 UINT uData);
int
soc_bm9600_XbXcfgRemapSelect2Read( UINT uBaseAddress,
                                        UINT uAddress,
                                        UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect2Write( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT uData);
int
soc_bm9600_XbXcfgRemapSelect2Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect2FillPattern( UINT uBaseAddress,
                                               UINT uData);
int
soc_bm9600_XbXcfgRemapSelect20Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect20Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect20Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect20FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect21Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect21Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect21Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect21FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect22Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect22Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect22Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect22FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect23Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect23Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect23Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect23FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect24Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect24Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect24Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect24FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect25Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect25Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect25Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect25FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect26Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect26Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect26Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect26FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect27Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect27Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect27Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect27FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect28Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect28Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect28Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect28FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect29Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect29Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect29Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect29FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect3Read( UINT uBaseAddress,
                                        UINT uAddress,
                                        UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect3Write( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT uData);
int
soc_bm9600_XbXcfgRemapSelect3Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect3FillPattern( UINT uBaseAddress,
                                               UINT uData);
int
soc_bm9600_XbXcfgRemapSelect30Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect30Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect30Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect30FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect31Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect31Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect31Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect31FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect32Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect32Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect32Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect32FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect33Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect33Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect33Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect33FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect34Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect34Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect34Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect34FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect35Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect35Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect35Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect35FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect36Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect36Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect36Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect36FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect37Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect37Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect37Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect37FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect38Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect38Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect38Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect38FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect39Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect39Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect39Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect39FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect4Read( UINT uBaseAddress,
                                        UINT uAddress,
                                        UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect4Write( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT uData);
int
soc_bm9600_XbXcfgRemapSelect4Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect4FillPattern( UINT uBaseAddress,
                                               UINT uData);
int
soc_bm9600_XbXcfgRemapSelect40Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect40Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect40Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect40FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect41Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect41Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect41Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect41FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect42Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect42Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect42Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect42FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect43Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect43Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect43Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect43FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect44Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect44Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect44Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect44FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect45Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect45Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect45Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect45FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect46Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect46Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect46Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect46FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect47Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect47Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect47Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect47FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect48Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect48Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect48Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect48FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect49Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect49Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect49Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect49FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect5Read( UINT uBaseAddress,
                                        UINT uAddress,
                                        UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect5Write( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT uData);
int
soc_bm9600_XbXcfgRemapSelect5Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect5FillPattern( UINT uBaseAddress,
                                               UINT uData);
int
soc_bm9600_XbXcfgRemapSelect50Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect50Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect50Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect50FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect51Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect51Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect51Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect51FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect52Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect52Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect52Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect52FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect53Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect53Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect53Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect53FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect54Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect54Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect54Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect54FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect55Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect55Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect55Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect55FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect56Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect56Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect56Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect56FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect57Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect57Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect57Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect57FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect58Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect58Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect58Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect58FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect59Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect59Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect59Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect59FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect6Read( UINT uBaseAddress,
                                        UINT uAddress,
                                        UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect6Write( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT uData);
int
soc_bm9600_XbXcfgRemapSelect6Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect6FillPattern( UINT uBaseAddress,
                                               UINT uData);
int
soc_bm9600_XbXcfgRemapSelect60Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect60Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect60Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect60FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect61Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect61Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect61Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect61FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect62Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect62Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect62Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect62FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect63Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect63Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect63Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect63FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect64Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect64Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect64Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect64FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect65Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect65Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect65Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect65FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect66Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect66Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect66Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect66FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect67Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect67Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect67Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect67FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect68Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect68Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect68Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect68FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect69Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect69Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect69Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect69FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect7Read( UINT uBaseAddress,
                                        UINT uAddress,
                                        UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect7Write( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT uData);
int
soc_bm9600_XbXcfgRemapSelect7Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect7FillPattern( UINT uBaseAddress,
                                               UINT uData);
int
soc_bm9600_XbXcfgRemapSelect70Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect70Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect70Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect70FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect71Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect71Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect71Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect71FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect72Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect72Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect72Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect72FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect73Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect73Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect73Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect73FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect74Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect74Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect74Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect74FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect75Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect75Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect75Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect75FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect76Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect76Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect76Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect76FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect77Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect77Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect77Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect77FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect78Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect78Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect78Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect78FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect79Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect79Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect79Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect79FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect8Read( UINT uBaseAddress,
                                        UINT uAddress,
                                        UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect8Write( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT uData);
int
soc_bm9600_XbXcfgRemapSelect8Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect8FillPattern( UINT uBaseAddress,
                                               UINT uData);
int
soc_bm9600_XbXcfgRemapSelect80Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect80Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect80Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect80FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect81Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect81Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect81Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect81FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect82Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect82Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect82Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect82FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect83Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect83Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect83Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect83FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect84Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect84Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect84Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect84FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect85Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect85Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect85Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect85FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect86Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect86Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect86Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect86FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect87Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect87Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect87Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect87FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect88Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect88Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect88Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect88FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect89Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect89Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect89Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect89FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect9Read( UINT uBaseAddress,
                                        UINT uAddress,
                                        UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect9Write( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT uData);
int
soc_bm9600_XbXcfgRemapSelect9Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect9FillPattern( UINT uBaseAddress,
                                               UINT uData);
int
soc_bm9600_XbXcfgRemapSelect90Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect90Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect90Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect90FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect91Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect91Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect91Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect91FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect92Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect92Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect92Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect92FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect93Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect93Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect93Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect93FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect94Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect94Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect94Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect94FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect95Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect95Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect95Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect95FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect96Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect96Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect96Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect96FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect97Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect97Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect97Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect97FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect98Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect98Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect98Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect98FillPattern( UINT uBaseAddress,
                                                UINT uData);
int
soc_bm9600_XbXcfgRemapSelect99Read( UINT uBaseAddress,
                                         UINT uAddress,
                                         UINT *pData);
int
soc_bm9600_XbXcfgRemapSelect99Write( UINT uBaseAddress,
                                          UINT uAddress,
                                          UINT uData);
int
soc_bm9600_XbXcfgRemapSelect99Clear( UINT uBaseAddress);
int
soc_bm9600_XbXcfgRemapSelect99FillPattern( UINT uBaseAddress,
                                                UINT uData);

#endif
int
soc_bm9600_BwAllocCfgBaseRead( UINT uBaseAddress,
                                    UINT uAddress,
                                    UINT *pData);
int
soc_bm9600_BwAllocCfgBaseWrite( UINT uBaseAddress,
                                     UINT uAddress,
                                     UINT uData);

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwAllocCfgBaseClear( UINT uBaseAddress);
int
soc_bm9600_BwAllocCfgBaseFillPattern( UINT uBaseAddress,
                                           UINT uData);
#endif
int
soc_bm9600_BwAllocRateRead( UINT uBaseAddress,
                                 UINT uAddress,
                                 UINT *pData);
int
soc_bm9600_BwAllocRateWrite( UINT uBaseAddress,
                                  UINT uAddress,
                                  UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwAllocRateClear( UINT uBaseAddress);
int
soc_bm9600_BwAllocRateFillPattern( UINT uBaseAddress,
                                        UINT uData);
#endif

int
soc_bm9600_BwFetchDataRead( UINT uBaseAddress,
                                 UINT uAddress,
                                 UINT *pData);
int
soc_bm9600_BwFetchDataWrite( UINT uBaseAddress,
                                  UINT uAddress,
                                  UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwFetchDataClear( UINT uBaseAddress);
int
soc_bm9600_BwFetchDataFillPattern( UINT uBaseAddress,
                                        UINT uData);
#endif

int
soc_bm9600_BwFetchSumRead( UINT uBaseAddress,
                                UINT uAddress,
                                UINT *pData);
int
soc_bm9600_BwFetchSumWrite( UINT uBaseAddress,
                                 UINT uAddress,
                                 UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwFetchSumClear( UINT uBaseAddress);
int
soc_bm9600_BwFetchSumFillPattern( UINT uBaseAddress,
                                       UINT uData);
#endif

int
soc_bm9600_BwFetchValidRead( UINT uBaseAddress,
                                  UINT uAddress,
                                  UINT *pData);
int
soc_bm9600_BwFetchValidWrite( UINT uBaseAddress,
                                   UINT uAddress,
                                   UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwFetchValidClear( UINT uBaseAddress);
int
soc_bm9600_BwFetchValidFillPattern( UINT uBaseAddress,
                                         UINT uData);
#endif
int
soc_bm9600_BwR0BagRead( UINT uBaseAddress,
                             UINT uAddress,
                             UINT *pData);
int
soc_bm9600_BwR0BagWrite( UINT uBaseAddress,
                              UINT uAddress,
                              UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR0BagClear( UINT uBaseAddress);
int
soc_bm9600_BwR0BagFillPattern( UINT uBaseAddress,
                                    UINT uData);
#endif
int
soc_bm9600_BwR0BwpRead( UINT uBaseAddress,
                             UINT uAddress,
                             UINT *pData);
int
soc_bm9600_BwR0BwpWrite( UINT uBaseAddress,
                              UINT uAddress,
                              UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR0BwpClear( UINT uBaseAddress);
int
soc_bm9600_BwR0BwpFillPattern( UINT uBaseAddress,
                                    UINT uData);
#endif
int
soc_bm9600_BwR0WdtRead( UINT uBaseAddress,
                             UINT uAddress,
                             UINT *pData);
int
soc_bm9600_BwR0WdtWrite( UINT uBaseAddress,
                              UINT uAddress,
                              UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR0WdtClear( UINT uBaseAddress);
int
soc_bm9600_BwR0WdtFillPattern( UINT uBaseAddress,
                                    UINT uData);
#endif
int
soc_bm9600_BwR1BagRead( UINT uBaseAddress,
                             UINT uAddress,
                             UINT *pData);
int
soc_bm9600_BwR1BagWrite( UINT uBaseAddress,
                              UINT uAddress,
                              UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR1BagClear( UINT uBaseAddress);
int
soc_bm9600_BwR1BagFillPattern( UINT uBaseAddress,
                                    UINT uData);
#endif

int
soc_bm9600_BwR1Wct0ARead( UINT uBaseAddress,
                               UINT uAddress,
                               UINT *pData);
int
soc_bm9600_BwR1Wct0AWrite( UINT uBaseAddress,
                                UINT uAddress,
                                UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR1Wct0AClear( UINT uBaseAddress);
int
soc_bm9600_BwR1Wct0AFillPattern( UINT uBaseAddress,
                                      UINT uData);
#endif

int
soc_bm9600_BwR1Wct0BRead( UINT uBaseAddress,
                               UINT uAddress,
                               UINT *pData);
int
soc_bm9600_BwR1Wct0BWrite( UINT uBaseAddress,
                                UINT uAddress,
                                UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR1Wct0BClear( UINT uBaseAddress);
int
soc_bm9600_BwR1Wct0BFillPattern( UINT uBaseAddress,
                                      UINT uData);
#endif

int
soc_bm9600_BwR1Wct1ARead( UINT uBaseAddress,
                               UINT uAddress,
                               UINT *pData);
int
soc_bm9600_BwR1Wct1AWrite( UINT uBaseAddress,
                                UINT uAddress,
                                UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR1Wct1AClear( UINT uBaseAddress);
int
soc_bm9600_BwR1Wct1AFillPattern( UINT uBaseAddress,
                                      UINT uData);
#endif
int
soc_bm9600_BwR1Wct1BRead( UINT uBaseAddress,
                               UINT uAddress,
                               UINT *pData);
int
soc_bm9600_BwR1Wct1BWrite( UINT uBaseAddress,
                                UINT uAddress,
                                UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR1Wct1BClear( UINT uBaseAddress);
int
soc_bm9600_BwR1Wct1BFillPattern( UINT uBaseAddress,
                                      UINT uData);
#endif

int
soc_bm9600_BwR1Wct2ARead( UINT uBaseAddress,
                               UINT uAddress,
                               UINT *pData);
int
soc_bm9600_BwR1Wct2AWrite( UINT uBaseAddress,
                                UINT uAddress,
                                UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR1Wct2AClear( UINT uBaseAddress);
int
soc_bm9600_BwR1Wct2AFillPattern( UINT uBaseAddress,
                                      UINT uData);
#endif

int
soc_bm9600_BwR1Wct2BRead( UINT uBaseAddress,
                               UINT uAddress,
                               UINT *pData);
int
soc_bm9600_BwR1Wct2BWrite( UINT uBaseAddress,
                                UINT uAddress,
                                UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR1Wct2BClear( UINT uBaseAddress);
int
soc_bm9600_BwR1Wct2BFillPattern( UINT uBaseAddress,
                                      UINT uData);
#endif

int
soc_bm9600_BwR1WstRead( UINT uBaseAddress,
                             UINT uAddress,
                             UINT *pData);
int
soc_bm9600_BwR1WstWrite( UINT uBaseAddress,
                              UINT uAddress,
                              UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwR1WstClear( UINT uBaseAddress);
int
soc_bm9600_BwR1WstFillPattern( UINT uBaseAddress,
                                    UINT uData);
#endif
int
soc_bm9600_BwWredCfgBaseRead( UINT uBaseAddress,
                                   UINT uAddress,
                                   UINT *pData);
int
soc_bm9600_BwWredCfgBaseWrite( UINT uBaseAddress,
                                    UINT uAddress,
                                    UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwWredCfgBaseClear( UINT uBaseAddress);
int
soc_bm9600_BwWredCfgBaseFillPattern( UINT uBaseAddress,
                                          UINT uData);
#endif

int
soc_bm9600_BwWredDropNPart1Read( UINT uBaseAddress,
                                      UINT uAddress,
                                      UINT *pData);
int
soc_bm9600_BwWredDropNPart1Write( UINT uBaseAddress,
                                       UINT uAddress,
                                       UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwWredDropNPart1Clear( UINT uBaseAddress);
int
soc_bm9600_BwWredDropNPart1FillPattern( UINT uBaseAddress,
                                             UINT uData);
#endif

int
soc_bm9600_BwWredDropNPart2Read( UINT uBaseAddress,
                                      UINT uAddress,
                                      UINT *pData);
int
soc_bm9600_BwWredDropNPart2Write( UINT uBaseAddress,
                                       UINT uAddress,
                                       UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_BwWredDropNPart2Clear( UINT uBaseAddress);
int
soc_bm9600_BwWredDropNPart2FillPattern( UINT uBaseAddress,
                                             UINT uData);
#endif
int
soc_bm9600_InaEsetPriRead( UINT uBaseAddress,
			   UINT uAddress,
			   UINT uInstance,
			   UINT *pData0,
			   UINT *pData1,
			   UINT *pData2,
			   UINT *pData3);
int
soc_bm9600_InaEsetPriWrite( UINT uBaseAddress,
                                 UINT uAddress,
                                 UINT uInstance,
                                 UINT uData0,
                                 UINT uData1,
                                 UINT uData2,
                                 UINT uData3);

int
soc_bm9600_InaEsetPriClear( UINT uBaseAddress,UINT uInstance);
int
soc_bm9600_InaEsetPriFillPattern( UINT uBaseAddress,
                                       UINT uInstance,
                                       UINT uData0,
                                       UINT uData1,
                                       UINT uData2,
                                       UINT uData3);


int
soc_bm9600_InaHi1Selected_0Read( UINT uBaseAddress,
				 UINT uAddress,
				 UINT uInstance,
				 UINT *pData);
int
soc_bm9600_InaHi1Selected_0Write( UINT uBaseAddress,
				  UINT uAddress,
				  UINT uInstance,
				  UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaHi1Selected_0Clear( UINT uBaseAddress,UINT uInstance);
int
soc_bm9600_InaHi1Selected_0FillPattern( UINT uBaseAddress,
					UINT uInstance,
					UINT uData);
#endif

int
soc_bm9600_InaHi1Selected_1Read( UINT uBaseAddress,
				 UINT uAddress,
				 UINT uInstance,
				 UINT *pData);
int
soc_bm9600_InaHi1Selected_1Write( UINT uBaseAddress,
                                       UINT uAddress,
                                       UINT uInstance,
                                       UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaHi1Selected_1Clear( UINT uBaseAddress,UINT uInstance);
int
soc_bm9600_InaHi1Selected_1FillPattern( UINT uBaseAddress,
					UINT uInstance,
					UINT uData);
#endif
int
soc_bm9600_InaHi2Selected_0Read( UINT uBaseAddress,
				 UINT uAddress,
				 UINT uInstance,
				 UINT *pData);
int
soc_bm9600_InaHi2Selected_0Write( UINT uBaseAddress,
				  UINT uAddress,
				  UINT uInstance,
				  UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaHi2Selected_0Clear( UINT uBaseAddress,UINT uInstance);
int
soc_bm9600_InaHi2Selected_0FillPattern( UINT uBaseAddress,
					UINT uInstance,
					UINT uData);
#endif

int
soc_bm9600_InaHi2Selected_1Read( UINT uBaseAddress,
				 UINT uAddress,
				 UINT uInstance,
				 UINT *pData);
int
soc_bm9600_InaHi2Selected_1Write( UINT uBaseAddress,
				  UINT uAddress,
				  UINT uInstance,
				  UINT uData);

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaHi2Selected_1Clear( UINT uBaseAddress,UINT uInstance);
int
soc_bm9600_InaHi2Selected_1FillPattern( UINT uBaseAddress,
					UINT uInstance,
					UINT uData);
#endif

int
soc_bm9600_InaHi3Selected_0Read( UINT uBaseAddress,
				 UINT uAddress,
				 UINT uInstance,
				 UINT *pData);
int
soc_bm9600_InaHi3Selected_0Write( UINT uBaseAddress,
				  UINT uAddress,
				  UINT uInstance,
				  UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaHi3Selected_0Clear( UINT uBaseAddress,UINT uInstance);
int
soc_bm9600_InaHi3Selected_0FillPattern( UINT uBaseAddress,
					UINT uInstance,
					UINT uData);
#endif

int
soc_bm9600_InaHi3Selected_1Read( UINT uBaseAddress,
				 UINT uAddress,
				 UINT uInstance,
				 UINT *pData);
int
soc_bm9600_InaHi3Selected_1Write( UINT uBaseAddress,
				  UINT uAddress,
                                       UINT uInstance,
				  UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaHi3Selected_1Clear( UINT uBaseAddress,UINT uInstance);
int
soc_bm9600_InaHi3Selected_1FillPattern( UINT uBaseAddress,
					UINT uInstance,
					UINT uData);
#endif
int
soc_bm9600_InaHi4Selected_0Read( UINT uBaseAddress,
				 UINT uAddress,
				 UINT uInstance,
				 UINT *pData);
int
soc_bm9600_InaHi4Selected_0Write( UINT uBaseAddress,
				  UINT uAddress,
				  UINT uInstance,
				  UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaHi4Selected_0Clear( UINT uBaseAddress,UINT uInstance);
int
soc_bm9600_InaHi4Selected_0FillPattern( UINT uBaseAddress,
					UINT uInstance,
					UINT uData);
#endif
int
soc_bm9600_InaHi4Selected_1Read( UINT uBaseAddress,
				 UINT uAddress,
				 UINT uInstance,
				 UINT *pData);
int
soc_bm9600_InaHi4Selected_1Write( UINT uBaseAddress,
				  UINT uAddress,
				  UINT uInstance,
				  UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaHi4Selected_1Clear( UINT uBaseAddress,UINT uInstance);
int
soc_bm9600_InaHi4Selected_1FillPattern( UINT uBaseAddress,
					UINT uInstance,
					UINT uData);
#endif
int
soc_bm9600_InaPortPriRead( UINT uBaseAddress,
			   UINT uAddress,
			   UINT uInstance,
			   UINT *pData0,
			   UINT *pData1,
			   UINT *pData2,
			   UINT *pData3);
int
soc_bm9600_InaPortPriWrite( UINT uBaseAddress,
			    UINT uAddress,
			    UINT uInstance,
			    UINT uData0,
			    UINT uData1,
			    UINT uData2,
			    UINT uData3);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaPortPriClear( UINT uBaseAddress,UINT uInstance);
int
soc_bm9600_InaPortPriFillPattern( UINT uBaseAddress,
				  UINT uInstance,
				  UINT uData0,
				  UINT uData1,
				  UINT uData2,
				  UINT uData3);
#endif
int
soc_bm9600_InaRandomNumGenRead( UINT uBaseAddress,
				UINT uAddress,
				UINT uInstance,
				UINT *pData);
int
soc_bm9600_InaRandomNumGenWrite( UINT uBaseAddress,
				 UINT uAddress,
				 UINT uInstance,
				 UINT uData);
#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaRandomNumGenClear( UINT uBaseAddress,UINT uInstance);
int
soc_bm9600_InaRandomNumGenFillPattern( UINT uBaseAddress,
				       UINT uInstance,
				       UINT uData);
#endif
int
soc_bm9600_InaSysportMapRead( UINT uBaseAddress,
			      UINT uAddress,
			      UINT uInstance,
			      UINT *pData);
int
soc_bm9600_InaSysportMapWrite( UINT uBaseAddress,
			       UINT uAddress,
			       UINT uInstance,
			       UINT uData);

int
soc_bm9600_InaSysportMapCacheRead(uint32 uBaseAddress,
                                  uint32 uAddress,
                                  uint32 uInstance,
                                  uint32 *pData);

#ifdef FUNCTION_NOW_USED
int
soc_bm9600_InaSysportMapClear( UINT uBaseAddress,UINT uInstance);
int
soc_bm9600_InaSysportMapFillPattern( UINT uBaseAddress,
				     UINT uInstance,
				     UINT uData);

#endif

int
soc_bm9600_lcm_mode_set(int unit, soc_lcm_mode_t mode);

int
soc_bm9600_lcm_mode_get(int unit, soc_lcm_mode_t *mode);

int
soc_bm9600_lcm_fixed_config(int unit,
                            int src_modid,
                            int configAB,
                            int src_xbport,
                            int dst_xbport);

int
soc_bm9600_lcm_fixed_config_validate(int unit,
				     int src_modid,
				     int configAB,
				     int src_xbport,
				     int dst_xbport);

/*
 * NM Memory Cache support functions
 */
int
soc_bm9600_NmMemoryCacheAllocate(int unit);

int
soc_bm9600_NmMemoryCacheDeAllocate(int unit);

/*
 * INA Memory Cache support functions
 */
int
soc_bm9600_InaMemoryCacheAllocate(int unit);

int
soc_bm9600_InaMemoryCacheDeAllocate(int unit);

int
soc_bm9600_InaUpdateCachedSysportMap(uint32 unit, 
                                     uint32 ina_instance,
                                     uint32 sysport, 
                                     uint32 portset, 
                                     uint32 offset);
int
soc_bm9600_ina_sysport_sync(int unit, int ina);

int
soc_bm9600_NmEmtCacheRead(int unit, uint32 uAddress,
                          uint32 *pData0, uint32 *pData1, uint32 *pData2);

int
soc_bm9600_NmEmtCacheWrite(int unit, uint32 uAddress,
                           uint32 uData0, uint32 uData1, uint32 uData2); 

int
soc_bm9600_NmPortsetInfoCacheRead(int unit, uint32 uAddress, uint32 *pData);

int
soc_bm9600_NmPortsetInfoCacheWrite(int unit, uint32 uAddress, uint32 uData);

int
soc_bm9600_NmPortsetLinkCacheRead(int unit, uint32 uAddress, uint32 *pData);

int
soc_bm9600_NmPortsetLinkCacheWrite(int unit, uint32 uAddress, uint32 uData);

int
soc_bm9600_NmSysportArrayCacheRead(int unit, uint32 uAddress,
                                   uint32 *pData0, uint32 *pData1, uint32 *pData2,
                                   uint32 *pData3, uint32 *pData4, uint32 *pData5);

int
soc_bm9600_NmSysportArrayCacheWrite(int unit, uint32 uAddress,
                                    uint32 uData0, uint32 uData1, uint32 uData2,
                                    uint32 uData3, uint32 uData4, uint32 uData5);

void
soc_bm9600_disable_fo(int unit);

void
soc_bm9600_enable_fo(int unit);

int
soc_bm9600_is_fo_disabled(int unit);

extern int
soc_bm9600_epoch_in_timeslot_config_get(int unit, int num_queues, 
                                        uint32 *epoch_in_timeslots);

#endif /* _BM9600_H */
