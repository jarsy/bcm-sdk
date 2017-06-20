/*
 * $Id: c3hppc_sws.h,v 1.22 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * c3hppc_sws.h : C3-HPPC SWS defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _C3HPPC_SWS_H_
#define _C3HPPC_SWS_H_

#include <sal/appl/config.h>
#include <sal/appl/io.h>
#include <sal/types.h>
#include <appl/test/caladan3/c3hppc_utils.h>
#include <soc/types.h>
#include <soc/drv.h>
#include <sal/core/thread.h>
#include <sal/appl/sal.h>
#include <bcm/port.h>
#include <soc/sbx/caladan3/sws.h>


#define C3HPPC_SWS_LINE_INTERFACE                     (0)
#define C3HPPC_SWS_FABRIC_INTERFACE                   (1)
#define C3HPPC_SWS_TDM__48x1G_BY_100IL                (48)
#define C3HPPC_SWS_TDM__100IL_BY_100IL_64CHNLS        (0)
#define C3HPPC_SWS_TDM__100IL_BY_100IL_1CHNL          (1)
#define C3HPPC_SWS_TDM__100IL_BY_100IL_2CHNLS         (2)
#define C3HPPC_SWS_TDM__12x10G_BY_100IL               (12)
#define C3HPPC_SWS_TDM__3x40G_BY_100IL                (3)
#define C3HPPC_SWS_TDM__100G_BY_100IL                 (4)
#define C3HPPC_SWS_HOTSWAP__10G_40G_MIX               (1)
#define C3HPPC_SWS_HOTSWAP__10G_1G_MIX                (2)

#define C3HPPC_SWS_QM_INGRESS_MAX_PAGES               (7539)
#define C3HPPC_SWS_QM_EGRESS_MAX_PAGES                (7539)
#define C3HPPC_SWS_QM_64_CHNL_FLOW_CTRL_THOLD_DELTA   (30)
#define C3HPPC_SWS_QM_LE10G_FLOW_CTRL_THOLD_DELTA     (75)
#define C3HPPC_SWS_QM_GT10G_FLOW_CTRL_THOLD_DELTA     (225)

#define C3HPPC_SWS_IL_INSTANCE_NUM                    (2)
#define C3HPPC_SWS_IL_MAX_CHANNEL_NUM                 (64)
#define C3HPPC_SWS_IL_NUM_OF_STAT_TYPES               (16)
#define C3HPPC_SWS_IL_OP_MODE__50G_NARROW             (0)
#define C3HPPC_SWS_IL_OP_MODE__50G_WIDE               (1)
#define C3HPPC_SWS_IL_OP_MODE__100G_NARROW            (2)
#define C3HPPC_SWS_IL_OP_MODE__100G_NARROW            (2)
#define C3HPPC_SWS_PR_INSTANCE_NUM                    (2)
#define C3HPPC_SWS_PT_INSTANCE_NUM                    (2)
#define C3HPPC_SWS_SOURCE_QUEUE_NUM                   (128)
#define C3HPPC_SWS_INGRESS_SOURCE_QUEUE_NUM           (C3HPPC_SWS_SOURCE_QUEUE_NUM/2) 
#define C3HPPC_SWS_EGRESS_SOURCE_QUEUE_NUM            (C3HPPC_SWS_SOURCE_QUEUE_NUM/2) 
#define C3HPPC_SWS_FREE_PAGE_NUM                      (16384) 
#define C3HPPC_SWS_PR_XL_BASE_PORT                    (51) 
#define ILRXSTAT__LT64                                (0)
#define ILRXSTAT__EQ64                                (1)
#define ILRXSTAT__65_127                              (2)
#define ILRXSTAT__128_255                             (3)
#define ILRXSTAT__256_511                             (4)
#define ILRXSTAT__512_1023                            (5)
#define ILRXSTAT__1024_2047                           (6)
#define ILRXSTAT__2048_4095                           (7)
#define ILRXSTAT__4096_9216                           (8)
#define ILRXSTAT__9217_15999                          (9)
#define ILRXSTAT__BAD_PKT_ILERR                       (10)
#define ILRXSTAT__PKT                                 (11)
#define ILRXSTAT__BYTE                                (12)
#define ILRXSTAT__GTMTU                               (13)
#define ILRXSTAT__EQMTU                               (14)
#define ILRXSTAT__IEEE_CRCERR                         (15)
#define ILTXSTAT__LT64                                (0)
#define ILTXSTAT__EQ64                                (1)
#define ILTXSTAT__65_127                              (2)
#define ILTXSTAT__128_255                             (3)
#define ILTXSTAT__256_511                             (4)
#define ILTXSTAT__512_1023                            (5)
#define ILTXSTAT__1024_1518                           (6)
#define ILTXSTAT__1519_2047                           (7)
#define ILTXSTAT__2048_4095                           (8)
#define ILTXSTAT__4096_9216                           (9)
#define ILTXSTAT__9217_16383                          (10)
#define ILTXSTAT__GTMTU                               (11)
#define ILTXSTAT__EQMTU                               (12)
#define ILTXSTAT__BAD_PKT_PERR                        (13)
#define ILTXSTAT__PKT                                 (14)
#define ILTXSTAT__BYTE                                (15)
#define ILPKTGEN__INJECT_TOWARDS_PRE                  (0)
#define ILPKTGEN__INJECT_TOWARDS_WARPCORE             (1)
#define ILPKTGEN__PAYLOAD_PATTEN__PACKET_NUM          (0)
#define ILPKTGEN__PAYLOAD_PATTEN__WORD_NUM            (1)
#define ILPKTGEN__PAYLOAD_PATTEN__SSO                 (2)
#define ILPKTGEN__PAYLOAD_PATTEN__CHECKERBOARD        (3)
#define ILPKTGEN__FIXED                               (0)
#define ILPKTGEN__INCREMENT                           (1)
#define ILPKTGEN__PINGPONG                            (2)
#define ILPKTGEN__RANDOM                              (3)
#define ILPKTGEN__CAPTURE_COMING_FROM_WARPCORE        (0)
#define ILPKTCAP__CAPTURE_COMING_FROM_PT              (1)

#define ILPKTGEN__BUILTIN_HEADERS__SINGLE_VLAN         (0)
#define ILPKTGEN__BUILTIN_HEADERS__PATTERN_256B        (1)
#define ILPKTGEN__BUILTIN_HEADERS__NULL0               (2)
#define ILPKTGEN__BUILTIN_HEADERS__NULL1               (3)
#define ILPKTGEN__BUILTIN_HEADERS_NUM                  (4)
#define ILPKTGEN__BUILTIN_HEADERS_MAX_WORDS            (64)





typedef struct c3hppc_sws_il_stats_s {
  uint64    Array[C3HPPC_SWS_IL_MAX_CHANNEL_NUM][C3HPPC_SWS_IL_NUM_OF_STAT_TYPES];
} c3hppc_sws_il_stats_t;

typedef struct c3hppc_sws_il_control_info_s {
  uint8                        bBringUp;
  uint8                        bL1Loopback;
  uint8                        bExternalLoopback;
  uint8                        bFCLoopback;
  uint32                       uOperationalMode;
} c3hppc_sws_il_control_info_t;

typedef struct c3hppc_sws_mac_control_info_s {
  uint8                        bBringUp;
  uint8                        bPauseDisable;
  uint8                        bExternalLoopback;
  uint32                       uOperationalMode;
} c3hppc_sws_mac_control_info_t;

typedef struct c3hppc_sws_pp_control_info_s {
  uint8                        bBypassParsing;
  uint8                        bMultiStreamUcode;
} c3hppc_sws_pp_control_info_t;

typedef struct c3hppc_sws_pd_control_info_s {
  uint32                       bPlaceHolder;
} c3hppc_sws_pd_control_info_t;

typedef struct c3hppc_sws_pb_control_info_s {
  uint32                       bPlaceHolder;
} c3hppc_sws_pb_control_info_t;

typedef struct c3hppc_sws_pr_control_info_s {
  uint8                        bBypassParsing;
} c3hppc_sws_pr_control_info_t;

typedef struct c3hppc_sws_pt_control_info_s {
  int                          nTDM;
} c3hppc_sws_pt_control_info_t;

typedef struct c3hppc_sws_qm_control_info_s {
  int                          nTDM;
} c3hppc_sws_qm_control_info_t;

typedef struct c3hppc_sws_control_info_s {
  c3hppc_sws_il_control_info_t    aIlControlInfo[C3HPPC_SWS_IL_INSTANCE_NUM];
  c3hppc_sws_pp_control_info_t    PpControlInfo;
  c3hppc_sws_pd_control_info_t    PdControlInfo;
  c3hppc_sws_pb_control_info_t    PbControlInfo;
  c3hppc_sws_pr_control_info_t    PrControlInfo;
  c3hppc_sws_pt_control_info_t    PtControlInfo;
  c3hppc_sws_qm_control_info_t    QmControlInfo;
  c3hppc_sws_mac_control_info_t   MacControlInfo;
  uint8                           bLrpLearningEnable;
  uint8                           bOamEnable;
  uint8                           bSwsOnlyTest;
  uint8                           bLineTrafficGenOnly;
  uint8                           bFabricTrafficGenOnly;
  int                             nTDM;
  int                             bXLportAndCmicPortsActive;
  int                             nHotSwap;
} c3hppc_sws_control_info_t;

typedef struct c3hppc_sws_il_stat_name_s {
  char                            sName[8];
} c3hppc_sws_il_stat_name_t;





typedef union {
  uint32 value;
  struct {
    uint32 HdrData_3to0:32;
  } bits;
} c3hppc_sws_ppe_tcam_data_31to0_ut;

typedef union {
  uint32 value;
  struct {
    uint32 HdrData_7to4:32;
  } bits;
} c3hppc_sws_ppe_tcam_data_63to32_ut;

typedef union {
  uint32 value;
  struct {
    uint32 HdrData_11to8:32;
  } bits;
} c3hppc_sws_ppe_tcam_data_95to64_ut;

typedef union {
  uint32 value;
  struct {
    uint32 HdrData_15to12:32;
  } bits;
} c3hppc_sws_ppe_tcam_data_127to96_ut;

typedef union {
  uint32 value;
  struct {
    uint32 HdrData_19to16:32;
  } bits;
} c3hppc_sws_ppe_tcam_data_159to128_ut;

typedef union {
  uint32 value;
  struct {
    uint32 HdrData_23to20:32;
  } bits;
} c3hppc_sws_ppe_tcam_data_191to160_ut;

typedef union {
  uint32 value;
  struct {
    uint32 State:24,
           HdrData_24to24:8;
  } bits;
} c3hppc_sws_ppe_tcam_data_223to192_ut;

typedef union {
  uint32 value;
  struct {
    uint32 unused:22,
           PropertyTableSelect:1,
           TiedLow:1,
           PropertyTableResult:8;
  } bits;
} c3hppc_sws_ppe_tcam_data_233to224_ut;


/*
 * The following word order results in:
 *
 * struct    -->    1,0     3,2      5,4       7,6
 * sbus      -->    0,1     2,3      4,5       6,7
 * tcam data -->  [63:0] [127:64] [191:128] [255:192]
 *
 *-----------------------------------------------------------------------------*/
typedef struct c3hppc_sws_ppe_tcam_data_s {
  c3hppc_sws_ppe_tcam_data_31to0_ut    Word0;
  c3hppc_sws_ppe_tcam_data_63to32_ut   Word1;
  c3hppc_sws_ppe_tcam_data_95to64_ut   Word2;
  c3hppc_sws_ppe_tcam_data_127to96_ut  Word3;
  c3hppc_sws_ppe_tcam_data_159to128_ut Word4;
  c3hppc_sws_ppe_tcam_data_191to160_ut Word5;
  c3hppc_sws_ppe_tcam_data_223to192_ut Word6;
  c3hppc_sws_ppe_tcam_data_233to224_ut Word7;
} c3hppc_sws_ppe_tcam_data_t;





typedef union {
  uint32 value;
  struct {
    uint32 zeroes:22,
           Profile:4,
           RxPort:6;
  } bits;
} c3hppc_sws_pr_tcam_data_31to0_first_lookup_ut;

typedef union {
  uint32 value;
  struct {
    uint32 zeroes:8,
           State:24;
  } bits;
} c3hppc_sws_pr_tcam_data_31to0_ut;

typedef union {
  uint32 value;
  struct {
    uint32 HdrData_29to0:30,
           zeroes:2;
  } bits;
} c3hppc_sws_pr_tcam_data_63to32_ut;

typedef union {
  uint32 value;
  struct {
    uint32 HdrData_61to30:32;
  } bits;
} c3hppc_sws_pr_tcam_data_95to64_ut;

typedef union {
  uint32 value;
  struct {
    uint32 HdrData_93to62:32;
  } bits;
} c3hppc_sws_pr_tcam_data_127to96_ut;

typedef union {
  uint32 value;
  struct {
    uint32 HdrData_125to94:32;
  } bits;
} c3hppc_sws_pr_tcam_data_159to128_ut;

typedef union {
  uint32 value;
  struct {
    uint32 HdrData_157to126:32;
  } bits;
} c3hppc_sws_pr_tcam_data_191to160_ut;

typedef union {
  uint32 value;
  struct {
    uint32 HdrData_189to158:32;
  } bits;
} c3hppc_sws_pr_tcam_data_223to192_ut;

typedef union {
  uint32 value;
  struct {
    uint32 OutOfRangeZeroes:22;
    uint32 HdrData_199to190:10;
  } bits;
} c3hppc_sws_pr_tcam_data_233to224_ut;


/*
 * The following word order results in:
 *
 * struct    -->    1,0     3,2      5,4       7,6
 * sbus      -->    0,1     2,3      4,5       6,7
 * tcam data -->  [63:0] [127:64] [191:128] [255:192]
 *
 *-----------------------------------------------------------------------------*/
typedef struct c3hppc_sws_pr_tcam_data_s {
  c3hppc_sws_pr_tcam_data_31to0_ut    Word0;
  c3hppc_sws_pr_tcam_data_63to32_ut   Word1;
  c3hppc_sws_pr_tcam_data_95to64_ut   Word2;
  c3hppc_sws_pr_tcam_data_127to96_ut  Word3;
  c3hppc_sws_pr_tcam_data_159to128_ut Word4;
  c3hppc_sws_pr_tcam_data_191to160_ut Word5;
  c3hppc_sws_pr_tcam_data_223to192_ut Word6;
  c3hppc_sws_pr_tcam_data_233to224_ut Word7;
} c3hppc_sws_pr_tcam_data_t;

typedef struct c3hppc_sws_pr_tcam_data_first_lookup_s {
  c3hppc_sws_pr_tcam_data_31to0_first_lookup_ut    Word0;
  c3hppc_sws_pr_tcam_data_63to32_ut                Word1;
  c3hppc_sws_pr_tcam_data_95to64_ut                Word2;
  c3hppc_sws_pr_tcam_data_127to96_ut               Word3;
  c3hppc_sws_pr_tcam_data_159to128_ut              Word4;
  c3hppc_sws_pr_tcam_data_191to160_ut              Word5;
  c3hppc_sws_pr_tcam_data_223to192_ut              Word6;
  c3hppc_sws_pr_tcam_data_233to224_ut              Word7;
} c3hppc_sws_pr_tcam_data_first_lookup_t;










int c3hppc_sws_il_bringup( int nUnit, int nInstance, c3hppc_sws_il_control_info_t *pIlControlInfo );
int c3hppc_sws_pp_bringup( int nUnit, c3hppc_sws_pp_control_info_t *pPpControlInfo );
int c3hppc_sws_pd_bringup( int nUnit, c3hppc_sws_pd_control_info_t *pPdControlInfo );
int c3hppc_sws_pb_bringup( int nUnit, c3hppc_sws_pb_control_info_t *pPbControlInfo );
int c3hppc_sws_pr_bringup( int nUnit, c3hppc_sws_pr_control_info_t *pPrControlInfo );
int c3hppc_sws_pt_bringup( int nUnit, c3hppc_sws_pt_control_info_t *pPtControlInfo, c3hppc_sws_il_control_info_t *pIlControlInfo );
int c3hppc_sws_qm_bringup( int nUnit, c3hppc_sws_qm_control_info_t *pQmControlInfo );
int c3hppc_sws_mac_bringup( int nUnit, c3hppc_sws_mac_control_info_t *pMacControlInfo );
int c3hppc_sws_il_stats_open( int nUnit );
int c3hppc_sws_il_stats_collect( int nUnit );
int c3hppc_sws_il_stats_close( int nUnit );
void   c3hppc_sws_il_stats_setrx( int nInstance, int nChannel, int nType, uint64 uuValue );
uint64 c3hppc_sws_il_stats_getrx( int nInstance, int nChannel, int nType );
uint64 c3hppc_sws_il_stats_gettx( int nInstance, int nChannel, int nType );
char *c3hppc_sws_il_stats_rxtype2string( int nType );
char *c3hppc_sws_il_stats_txtype2string( int nType );
void c3hppc_sws_il_stats_display_nonzero( int nInstance );
int c3hppc_sws_il_pktgen_builtin_packet_header_init( void );
int c3hppc_sws_il_pktgen_start( int nUnit, int nInstance );
int c3hppc_sws_il_pktgen_stop( int nUnit, int nInstance );
int c3hppc_sws_il_pktgen_go_control( int nUnit, int nInstance, int nGo );
int c3hppc_sws_il_pktgen_wait_for_done( int nUnit, int nInstance, int nTimeoutInIterations );
int c3hppc_sws_il_pktgen_is_done( int nUnit, int nInstance );
int c3hppc_sws_il_pktgen_setup( int nUnit, int nInstance, int nInjectDirection, int nPacketCount,
                                uint64 uuTransmitDuration, int nPayloadPattern, int nIPG,
                                uint32 uMinChannel, uint32 uMaxChannel, int nChannelMode,
                                int nMinPacketLen, int nMaxPacketLen, int nPacketLenMode,
                                uint32 *pPacketHeader, int nBuiltInHeaderSelect,
                                int nPacketHeaderLenInBytes );
int c3hppc_sws_il_pktcap_setup( int nUnit, int nInstance, int nCaptureDirection, int nOneShot,
                                uint64 uuChannelMask );
int c3hppc_sws_il_pktcap_arm( int nUnit, int nInstance );
int c3hppc_sws_il_pktcap_wait_for_done( int nUnit, int nInstance, int nTimeoutInIterations );
int c3hppc_sws_il_pktcap_get( int nUnit, int nInstance, uint32 *pPacket, uint32 *pPacketLen,
                              uint32 *pSOP, uint32 *pEOP, uint32 *pERR, uint32 *pBV, uint32 *pChannel );

int c3hppc_sws_transition_to_16x1G_8x10G_from_12x10G( int nUnit );
int c3hppc_sws_transition_to_12x10G_from_16x1G_8x10G( int nUnit );
int c3hppc_sws_transition_to_1x40G_8x10G_from_12x10G( int nUnit );
int c3hppc_sws_transition_to_12x10G_from_1x40G_8x10G( int nUnit );
uint32 c3hppc_sws_get_port_register_addr( int nUnit, soc_reg_t nReg, int nPort, int nIndex );
uint8 c3hppc_sws_get_port_register_acc_type( int nUnit, soc_reg_t nReg );
int c3hppc_sws_soc_reg32_get( int nUnit, int nBlk, soc_reg_t nReg, int nPort, int nIndex, uint32 *puData );
int c3hppc_sws_soc_reg32_set( int nUnit, int nBlk, soc_reg_t nReg, int nPort, int nIndex, uint32 uData );
int c3hppc_sws_soc_reg64_get( int nUnit, int nBlk, soc_reg_t nReg, int nPort, int nIndex, uint64 *puData );
int c3hppc_sws_soc_reg64_set( int nUnit, int nBlk, soc_reg_t nReg, int nPort, int nIndex, uint64 uData );




int c3hppc_sws_hw_init( int nUnit, c3hppc_sws_control_info_t *pC3SwsControlInfo );
int c3hppc_sws_hw_cleanup( int nUnit );
int c3hppc_sws_display_error_state( int nUnit );
int c3hppc_sws_check_free_page_list( int nUnit );


#endif /* _C3HPPC_SWS_H_ */
