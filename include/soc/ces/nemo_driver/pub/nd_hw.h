/* $Id: nd_hw.h 1.3.4.2 Broadcom SDK $
 * Copyright 2011 BATM
 */

#ifndef __NEMO_HW_H__
#define __NEMO_HW_H__
#define CES16_BCM_VERSION


#include "../nd_platform.h"
#include "../nd_list.h"
#include "../nd_mm.h"
#include "nd_hw_hl.h"
#include "../nd_registers.h"
#include "classification/clsb_api.h"
#include "classification/cls_types.h"


#ifdef __cplusplus
extern "C"
{
#endif


struct AgNdDevice_S;

typedef struct AgNdDevice_S AgNdDevice;

typedef enum 
{
    AG_ND_STATE_OPEN,
    AG_ND_STATE_INIT

} AgNdDeviceState;


#define AG_ND_CHANNEL_MAX                  2048
#define AG_ND_PORT_MAX                     16
#define AG_ND_HEADER_MAX                   128
#define AG_ND_SLOT_MAX                     32
#define AG_ND_SPE_MAX                      3
#define AG_ND_VTG_MAX                      7
#define AG_ND_VT_MAX                       4
#define AG_ND_PME_GROUP_MAX                3
#define AG_ND_CIRCUIT_IDX_MAX              84


/* */
/* alignments (in bits of address) */
/*  */
#define AG_ND_ALIGNMENT_LSTREE             16
#define AG_ND_ALIGNMENT_STRICT             16
#define AG_ND_ALIGNMENT_HEADERS            10

/* PBF alignment depends of buffer size */
/*#define AG_ND_ALIGNMENT_PBF                   */
/*  */
#define AG_ND_ALIGNMENT_JBF                10
#define AG_ND_ALIGNMENT_JBF_SEGMENT        2

/* */
/* LOPS/AOPS registers array size */
/* */
 #ifndef CES16_BCM_VERSION
  #define AG_ND_SYNC_THRESHOLD_TABLE_SIZE    8
 #else
    #define AG_ND_SYNC_THRESHOLD_TABLE_SIZE    1
 #endif
/* */
/* LOPS/AOPS threshold value boundaries */
/* */
#define AG_ND_SYNC_THRESHOLD_MIN           0x1
#define AG_ND_SYNC_THRESHOLD_MAX           0x3FF


#define AG_ND_TRACE_BUS_MAX                1024

/* */
/* Memory Unit */
/* */
#define AG_ND_MEM_UNIT_MAX                 9

typedef struct
{
    AG_U32          n_bank;             
    AG_BOOL         b_internal;
    AG_U32          n_start;
    AG_U32          n_size;
    AG_CHAR         a_name[80];         /* MU name reported by BIT in the case of failure */
    
} AgNdMemUnit;

/* */
/* PTP stuff */
/*  */
#define AG_ND_PTP_IDX_MAX                   2


#define AG_ND_SYSTEM_IDX_MAX                   2

/* */
/* MAC stuff */
/* */
#define AG_ND_PHY_DEVICE_MAX                2
#define AG_ND_PHY_REG_MAX                   32

/* */
/* TPP stuff */
/* */
#define AG_ND_RCR_CHANNEL_MAX               16
#define AG_ND_RCR_FLL_RESULT_MAX            20

typedef enum
{
    AG_ND_RCR_TS_125_RATE_1_MHZ             = 0x0,
    AG_ND_RCR_TS_125_RATE_25_MHZ            = 0x1,
    AG_ND_RCR_TS_125_RATE_125_MHZ           = 0x2

} AgNdRcrTs125Rate;

typedef enum
{
    AG_ND_RCR_TS_77_RATE_1_MHZ              = 0x0,
    AG_ND_RCR_TS_77_RATE_19_44_MHZ          = 0x1,
    AG_ND_RCR_TS_77_RATE_77_76_MHZ          = 0x2

} AgNdRcrTs77Rate;

/* */
/* RPC stuff */
/* */
#define AG_ND_RPC_CCNT_MAX                  4       /* the number of per channel RPC counters */
#define AG_ND_RPC_CGLB_MAX                  8       /* the number of global RPC counters */
#define AG_ND_RPC_UCODE_INSTR_MAX           0x80    /* max ucode size in instructions */
#define AG_ND_RPC_STRICT_MAX                32      /* strict data buffer size */


typedef enum
{
    /* */
    /* hardwired ucode flags */
    /* */
    AG_ND_RPC_FLAG_UNEXPECTED_END_OF_PACKET     = 0x80000000, /* bit 31 */
    AG_ND_RPC_FLAG_IP_HEADER_CHECKSUM_ERROR     = 0x40000000, /* bit 30 */
    AG_ND_RPC_FLAG_UNKNOWN_PW_LABEL             = 0x20000000, /* bit 29 */
    AG_ND_RPC_FLAG_CES_CW_ERROR                 = 0x10000000, /* bit 28 */
    AG_ND_RPC_FLAG_SIGNALING_PACKET             = 0x08000000, /* bit 27 */
    AG_ND_RPC_FLAG_AIS_DBA_PACKET               = 0x04000000, /* bit 26 */
    AG_ND_RPC_FLAG_RAI_R_BIT_IS_1               = 0x02000000, /* bit 25 */
    AG_ND_RPC_FLAG_STRICT_CHECK_RESULTS         = 0x01000000, /* bit 24 */
    AG_ND_RPC_FLAG_CRC_ERROR                    = 0x00800000, /* bit 23 */
    AG_ND_RPC_FLAG_UDP_CHECKSUM_ERROR           = 0x00400000, /* bit 22 */
    AG_ND_RPC_FLAG_IP_LEN_ERROR                 = 0x00200000, /* bit 21 */
    AG_ND_RPC_FLAG_UDP_LENGTH_ERROR             = 0x00100000, /* bit 20 */
    AG_ND_RPC_FLAG_UDP_FORWARD_HP_QUEUE         = 0x00080000, /* bit 19 */
    AG_ND_RPC_FLAG_UDP_FORWARD_LP_QUEUE         = 0x00040000, /* bit 18 */
    AG_ND_RPC_FLAG_DISABLED_CES_CHANNEL         = 0x00020000, /* bit 17 */
    AG_ND_RPC_FLAG_RAI_M_BITS_ARE_10            = 0x00010000, /* bit 16 */

    /* */
    /* ucode assigned flags */
    /* */
    AG_ND_RPC_FLAG_FOUND_PTP                    = 0x00008000, /* bit 15 */
    AG_ND_RPC_FLAG_RTP_FLAG_ERROR               = 0x00004000, /* bit 14 */
    AG_ND_RPC_FLAG_HOST_DESIGNATED_PACKET       = 0x00002000, /* bit 13 */
    AG_ND_RPC_FLAG_MPLS_TTL_ZERO                = 0x00001000, /* bit 12 */
    AG_ND_RPC_FLAG_IP_ADDRESS_ERROR             = 0x00000800, /* bit 11 */
    AG_ND_RPC_FLAG_IP_WRONG_PROTOCOL            = 0x00000400, /* bit 10 */
    AG_ND_RPC_FLAG_BAD_IP_HEADER                = 0x00000200, /* bit 9 */
    AG_ND_RPC_FLAG_IP_NOT_UNICAST_PACKET        = 0x00000100, /* bit 8 */
    AG_ND_RPC_FLAG_UNKNOWN_ETHER_TYPE           = 0x00000080, /* bit 7 */
    AG_ND_RPC_FLAG_ETH_DESTADDR_MISMATCH        = 0x00000040, /* bit 6 */
    AG_ND_RPC_FLAG_ETH_NOT_UNICAST_PACKET       = 0x00000020, /* bit 5 */
    AG_ND_RPC_FLAG_FOUND_UDP                    = 0x00000010, /* bit 4 */
    AG_ND_RPC_FLAG_FOUND_ECID                   = 0x00000008, /* bit 3 */
    AG_ND_RPC_FLAG_FOUND_VLAN                   = 0x00000004, /* bit 2 */
    AG_ND_RPC_FLAG_FOUND_IP                     = 0x00000002, /* bit 1 */
    AG_ND_RPC_FLAG_FOUND_MPLS                   = 0x00000001  /* bit 0 */

} AgNdRpcFlag;

typedef enum 
{
    AG_ND_RPC_POLICY_CNT_DROPPED                = 0x0,
    AG_ND_RPC_POLICY_CNT_FWD_TDM                = 0x1,
    AG_ND_RPC_POLICY_CNT_FWD_CPU                = 0x2,
    AG_ND_RPC_POLICY_CNT_ALL                    = 0x3

} AgNdRpcCntPolicy;


/* */
/* */
/* channel/circuit id stuff */
/* */
typedef AG_U8   AgNdCircuit; 
typedef AG_U8   AgNdPort; 
typedef AG_U8   AgNdSlot;
typedef AG_U16  AgNdChannel;
typedef AG_U8   AgNdPtpIdx;


typedef struct
{
    AG_U32      n_code_id;
    AG_U32      n_mask;
    AG_U32      n_total_channels;
    AG_U32      n_total_ports;
    AG_U32      n_total_ptp_channels;

} AgNdChipInfo;


typedef struct 
{
    AgNdCircuit n_circuit_id;
    AgNdSlot    n_slot_idx;

} AgNdTsId;


#define AG_ND_TS_ISVALID(ts)         ((ts).n_circuit_id != 0xff && (ts).n_slot_idx != 0xff)
#define AG_ND_TS_INVALIDATE(ts)      { (ts).n_circuit_id = 0xff; (ts).n_slot_idx = 0xff; }
#define AG_ND_TS_ISEQUAL(ts1,ts2)      ((ts1).n_circuit_id == (ts2).n_circuit_id && (ts1).n_slot_idx == (ts2).n_slot_idx)


typedef enum 
{
    AG_ND_PATH_FIRST    = 0,
    AG_ND_PATH_EGRESS   = AG_ND_PATH_FIRST,

    AG_ND_PATH_LAST     = 1,
    AG_ND_PATH_INGRESS  = AG_ND_PATH_LAST,
    
    AG_ND_PATH_MAX      = 2

} AgNdPath;


/*ORI*/
/*ADD TO L2TP */

typedef enum
{
    AG_ND_ENCAPSULATION_FIRST   = 0,
    AG_ND_ENCAPSULATION_IP      = AG_ND_ENCAPSULATION_FIRST,

    AG_ND_ENCAPSULATION_MPLS    = 1,

    AG_ND_ENCAPSULATION_ETH     = 2,

    AG_ND_ENCAPSULATION_PTP_IP  = 3,

    AG_ND_ENCAPSULATION_PTP_EHT = 4,

	AG_ND_ENCAPSULATION_L2TP 	= 5,

    AG_ND_ENCAPSULATION_LAST    = AG_ND_ENCAPSULATION_L2TP,

    AG_ND_ENCAPSULATION_MAX     = 6

} AgNdEncapsulation;



typedef struct
{
    AgResult            (*p_print)(AG_CHAR *);
    AG_U32              n_verbose;      /* verbose level (ORed mask of AG_ND_BIT_VERBOSE_LEVEL_ values) */
    AG_U32              n_msg_max;      /* max number of messages per register/memunit */
    AG_BOOL             b_passed;       /* test result */
    AG_CHAR             a_buf[1024];    /* temporary text buffer */
                                        
    AG_U32              n_fail_addr;    /* the first failed register address  */
    
} AgNdBit;

typedef struct 
{
    AgNdList        x_list;
    AgNdTsId        x_id;
    AgNdChannel     n_channel_id;
    AG_BOOL         b_enable;

} AgNdTsInfo;


typedef struct 
{
    AgNdList        a_ts_list;      /*/ enabled timeslots list */
    AgNdTsId        x_first;


} AgNdChannelizer;

typedef struct
{
    AgNdChannelizer     a_channelizer[AG_ND_PATH_MAX];
    AG_BOOL             a_enable[AG_ND_PATH_MAX];
	AG_U32				n_jbf_ring_size;
} AgNdChannelInfo;

/* */
/* Stores the ingress configurations that can not be stored direcly in HW */
/*  */
typedef struct
{
    AG_BOOL             b_dba;          /* channel works in DBA mode, if true */

    /* */
    /* precalculated checksums as specified by FPGA spec: includes only the  */
    /* constant fields of IP/UDP and upper layer protocol headers (CES/RTP),  */
    /* not negated. the TDM payload length assumed to be 0. */
    /* */
    AG_U32              n_udp_chksum;   
    AG_U16              n_udp_len;

    AG_U32              n_ip_chksum;
    AG_U16              n_ip_len;
    AG_BOOL             b_mef_len_support;

    /* */
    /* although the following information is available in HW */
    /* we keep it here in order to save the cost of SRAM and registers access */
    /*  */
    AgNdRegTransmitHeaderFormatDescriptor x_tx_hdr_fmt;
    AG_U16              n_cw;
    AG_U32              n_payload_size;

} AgNdChannelIngressInfo;


/* */
/* stores RCR channel data: */
/* */
typedef struct
{
    AgNdChannel         n_psn_channel_id;

} AgNdRcrChannel;


/* */
/* */
/* Registers test and reset stuff */
/* */
#define AG_ND_NO_SPECIFIC_ENTITY 0xffff
typedef enum AgNdRegAddressFormat
{
    AG_ND_REG_FMT_GLOBAL,
    AG_ND_REG_FMT_CHANNEL,
    AG_ND_REG_FMT_CIRCUIT,
    AG_ND_REG_FMT_PTP_IDX,
    AG_ND_REG_FMT_ALL

} AgNdRegAddressFormat;

typedef enum AgNdRegType
{
    AG_ND_REG_TYPE_RW,
    AG_ND_REG_TYPE_RO,
    AG_ND_REG_TYPE_W1CLR,
    AG_ND_REG_TYPE_RCLR,
    AG_ND_REG_TYPE_WO

} AgNdRegType;

typedef struct AgNdRegProperties
{
    AG_U32                  n_address;        /* register base address */
    AG_CHAR                 *p_name;          /* print friendy name */
    AG_U32                  n_width;          /* register width in bits */
    AgNdRegType             e_type;           /* register type */

    void (*test)(AgNdDevice*, 
                 struct AgNdRegProperties*, 
                 AG_U32 n_offset, 
                 AG_BOOL b_test);             /* test function */

    AgNdRegAddressFormat    e_format;
    AG_U32                  n_mask;           /* reserved bits mask */
    AG_U32                  n_reset;          /* reset value: writing this into registers should yield default value */
    AG_U32                  n_default;        /* default value */
    AG_U32                  n_size;           /* number of words for registers that spread on several words */
    AG_U32                  n_chip;           /* bit mask, each bit corresponds to chip type */
    
} AgNdRegProperties;


extern AgNdRegProperties g_ndRegTable[];
extern AgNdChipInfo      g_ndChipInfoTable[];


AG_U32 ag_nd_next_reg_id(AgNdDevice *p_device, AgNdRegProperties *p_reg, AG_U32 n_current_id);

void ag_nd_regs_reset(AgNdDevice *p_device);


/* */
/* */
/* HW access services */
/* */

typedef enum 
{
    AG_ND_HW_ACCESS_READ,
    AG_ND_HW_ACCESS_WRITE

} AgNdHwAccess;

void    ag_nd_reg_read(AgNdDevice *p_device, AG_U32 n_offset, AG_U16 *p_value);
void    ag_nd_reg_write(AgNdDevice *p_device, AG_U32 n_offset, AG_U16 n_value);

void    ag_nd_reg_read_32bit(AgNdDevice *p_device, AG_U32 n_offset, AG_U32 *p_value);
void    ag_nd_reg_write_32bit(AgNdDevice *p_device, AG_U32 n_offset, AG_U32 n_value);

void    ag_nd_mem_unit_set(AgNdDevice *p_device, AgNdMemUnit *p_mu);
void    ag_nd_mem_read(AgNdDevice *p_device, AgNdMemUnit *p_mu, AG_U32 n_offset, void *p_data, AG_U32 n_word_count);
void    ag_nd_mem_write(AgNdDevice *p_device, AgNdMemUnit *p_mu, AG_U32 n_offset, void *p_data, AG_U32 n_word_count);

void    ag_nd_hw_kiss(AgNdDevice *p_device, AgNdHwAccess e_action, AG_U32 n_address, AG_U16 *p_value);


/* */
/* Register fields values */
/* */
#define AG_ND_JBF_NO_LIMIT      1024

typedef enum
{
    AG_ND_ARCH_MASK_NEMO      = 0x01,
    AG_ND_ARCH_MASK_NEPTUNE   = 0x08

} AgNdArchMask;

typedef enum 
{
    AG_ND_ONE_SECOND_PULSE_DIRECTION_IN  = 0x00,
    AG_ND_ONE_SECOND_PULSE_DIRECTION_OUT = 0x01

} AgNd1SecDirection;

typedef enum
{
    AG_ND_TDM_PAYLOAD_BIT_ORDERING_STANDARD = 0x00,
    AG_ND_TDM_PAYLOAD_BIT_ORDERING_REDUX    = 0x01

} AgNdTdmBitOrder;

typedef enum
{
    AG_ND_CCLK_SELECT_REF_CLK_1         = 0x00,
    AG_ND_CCLK_SELECT_REF_CLK_2         = 0x01,
    AG_ND_CCLK_SELECT_EXTERNAL_CLK      = 0x02

} AgNdCClkSelect;

typedef enum
{
    AG_ND_REF_CLK_SELECT_RCLK           = 0x00,
    AG_ND_REF_CLK_SELECT_REF_INPUT_1    = 0x01,
    AG_ND_REF_CLK_SELECT_REF_INPUT_2    = 0x02,
    AG_ND_REF_CLK_SELECT_NOMAD_BRG      = 0x03,
    AG_ND_REF_CLK_SELECT_PORT_BRG       = 0x04,
    AG_ND_REF_CLK_SELECT_PTP            = 0x07

} AgNdRefClkSelect;

typedef enum
{
    AG_ND_PTP_CLK_1                     = 0x0,
    AG_ND_PTP_CLK_2                     = 0x1,
    AG_ND_PTP_1PPS_1                    = 0x2,
    AG_ND_PTP_1PPS_2                    = 0x3

} AgNdPtpClkSelect;

typedef enum
{
     AG_ND_PTP_CLK_EXT_1                = 0x0,
     AG_ND_PTP_CLK_EXT_2                = 0x1,  
     AG_ND_PTP_CLK_TDM_PORT_RX          = 0x2,
     AG_ND_PTP_CLK_WAN_MMI_RX           = 0x3,
     AG_ND_PTP_CLK_LAN_MMI_RX           = 0x4,
     AG_ND_PTP_CLK_25_MHZ               = 0x5,
     AG_ND_PTP_CLK_BRG_1                = 0x6,
     AG_ND_PTP_CLK_BRG_2                = 0x7

} AgPtpClkSrc;


typedef enum
{
    AG_ND_EXT_CLK_DIR_INPUT             = 0x00,
    AG_ND_EXT_CLK_DIR_OUTPUT            = 0x01

} AgNdExtClkDir;

typedef enum
{
    AG_ND_RX_CLK_SELECT_INDEPENDENT     = 0x00,
    AG_ND_RX_CLK_SELECT_FPGA_SLAVE      = 0x01

} AgNdRxClkSelect;

typedef enum
{
    AG_ND_TX_CLK_SELECT_LOOPBACK     = 0x00,
    AG_ND_TX_CLK_SELECT_CCLK           = 0x02,
    AG_ND_TX_CLK_SELECT_INTERNAL_BRG = 0x03

} AgNdTxClkSelect;

typedef enum
{
    AG_ND_TDM_PROTOCOL_E1 = 0x00,
    AG_ND_TDM_PROTOCOL_T1 = 0x01

} AgNdTdmProto;

typedef enum
{
    AG_ND_PACKET_HEADER_32B   = 0x00,
    AG_ND_PACKET_HEADER_64B   = 0x01,
    AG_ND_PACKET_HEADER_128B  = 0x02

} AgNdHeaderSize;

typedef enum
{
    AG_ND_PACKET_SYNC_LOPS = 0x00,
    AG_ND_PACKET_SYNC_AOPS = 0x01

} AgNdSyncType;

typedef enum
{
    AG_ND_JITTER_BUFFER_STATE_IDLE   = 0x00,
    AG_ND_JITTER_BUFFER_STATE_FILL   = 0x01,
    AG_ND_JITTER_BUFFER_STATE_NORMAL = 0x02,
    AG_ND_JITTER_BUFFER_STATE_FLUSH  = 0x03

} AgNdJbfState;

typedef enum
{
    AG_ND_SLIP_DIRECTION_FORWARD  = 0x00,
    AG_ND_SLIP_DIRECTION_BACKWARD = 0x01

} AgNdSlipDir;

typedef enum
{
    AG_ND_SLIP_UNIT_BYTE   = 0x00,
    AG_ND_SLIP_UNIT_PACKET = 0x01

} AgNdSlipUnit;

typedef enum
{
    AG_ND_RTP_HEADER_FORMAT_TX_BEFORE_CES_CW = 0x00,
    AG_ND_RTP_HEADER_FORMAT_TX_AFTER_CES_CW  = 0x01

} AgNdRtpHdrFmtTx;

typedef enum
{
    AG_ND_RTP_HEADER_FORMAT_RX_NO_RTP        = 0x00,
    AG_ND_RTP_HEADER_FORMAT_RX_BEFORE_CES_CW = 0x02,
    AG_ND_RTP_HEADER_FORMAT_RX_AFTER_CES_CW  = 0x03

} AgNdRtpHdrFmtRx;

typedef enum
{
    AG_ND_MAC_TX_ADDR_SEL_NODE          = 0x00,
    AG_ND_MAC_TX_ADDR_SEL_SMAC0         = 0x04,
    AG_ND_MAC_TX_ADDR_SEL_SMAC1         = 0x05,
    AG_ND_MAC_TX_ADDR_SEL_SMAC2         = 0x06,
    AG_ND_MAC_TX_ADDR_SEL_SMAC3         = 0x07

} AgNdMacCCTxAddrSel;

typedef enum
{
    AG_ND_TPP_SAMPLE_EVERY_PACKET   = 0x0,
    AG_ND_TPP_SAMPLE_EVERY_PERIOD   = 0x1

} AgNdTppTimestampSample;

typedef enum
{
    AG_ND_SPE_MODE_CHANNELIZED      = 0x0,
    AG_ND_SPE_MODE_T3               = 0x2,  
    AG_ND_SPE_MODE_E3               = 0x3

} AgNdSpeMode;

typedef enum
{
    AG_ND_TDM_BUS_TYPE_SBI          = 0x0,
    AG_ND_TDM_BUS_TYPE_TELECOM      = 0x1

} AgNdTdmBusType;

typedef enum
{
    AG_ND_VC4_SELECT_CHANNELIZED    = 0x0,
    AG_ND_VC4_SELECT_NONCHANNELIZED = 0x1

} AgNdVc4Selection;

typedef enum
{
    AG_ND_ADJ_POLARITY_NEG          = 0x0,
    AG_ND_ADJ_POLARITY_POS          = 0x1

} AgNdAdjustPolarity;

typedef enum
{
    AG_ND_ADJ_TYPE_BIT              = 0x0,
    AG_ND_ADJ_TYPE_BYTE             = 0x1

} AgNdAdjustType;

typedef enum
{
    AG_ND_OSCILLATOR_OCXO           = 0x0,
    AG_ND_OSCILLATOR_TCXO           = 0x1

} AgNdOscillatorType;

typedef enum
{
    AG_ND_DCR_EXT_1 = 0,
    AG_ND_DCR_EXT_2 = 1,
    AG_ND_DCR_TDM_PORT_RX_CLK = 2,
    AG_ND_DCR_WAN_MII_RX_CLK = 3,
    AG_ND_DCR_LAN_MII_RX_CLK = 4,
	AG_ND_DCR_FREE_RUNNING_25_MHZ = 5,
	AG_ND_DCR_PTP_1 = 6,
	AG_ND_DCR_PTP_2 = 7

} AgNdNemoDcrClkSource;

typedef enum
{
	AG_ND_DCR_TDM_PORT_RX_CLK_0 = 0,
	AG_ND_DCR_TDM_PORT_RX_CLK_1 = 1,
	AG_ND_DCR_TDM_PORT_RX_CLK_2 = 2,
	AG_ND_DCR_TDM_PORT_RX_CLK_3 = 3,
	AG_ND_DCR_TDM_PORT_RX_CLK_4 = 4,
	AG_ND_DCR_TDM_PORT_RX_CLK_5 = 5,
	AG_ND_DCR_TDM_PORT_RX_CLK_6 = 6,
	AG_ND_DCR_TDM_PORT_RX_CLK_7 = 7,
    AG_ND_DCR_SYSTEM_EXT_1 		= 8,
    AG_ND_DCR_SYSTEM_EXT_2 		= 9,
    AG_ND_DCR_NOMAD_BRG         = 10
} AgNdNemoDcrSystemClkSource;

typedef enum
{
	AG_ND_DCR_USE_FIRST_TSO_MODE = 0,
	AG_ND_DCR_SEPERATE_TSO_MODE = 1
} AgNdNemoDcrSystemTsoMode;

typedef enum
{
	AG_ND_DCR_SYS_TSO_REF_EXTERNAL_1 = 0,
	AG_ND_DCR_SYS_TSO_REF_EXTERNAL_2 = 1,
	AG_ND_DCR_SYS_TSO_REF_WAN_25_MHZ = 2,
	AG_ND_DCR_SYS_TSO_REF_FREE_RUN_25_MHZ = 3
} AgNdNemoDcrSystemTsoRefClockSelect;


/* */
/* Device structure */
/* */
struct AgNdDevice_S
{
    AG_U32                  n_handle;
    AgNdDeviceState         e_state;

    AG_U16                  n_arch;
    AG_U16                  n_code_id;
    AG_U16                  n_revision;
    AG_U32                  n_chip_mask;

    AG_U32                  n_base;
    AG_BOOL                 b_use_hw;          
    AG_U32                  n_ext_mem_bank0_size; 
    AG_U32                  n_ext_mem_bank1_size; 

    AG_U32                  n_total_channels;
    AG_U32                  n_total_ports;

	AG_BOOL 				b_dynamic_memory;
    /* */
    /* fixed memory allocation stuff */
    /* */
    AG_U32                  n_vba_size;
    AG_U32                  n_pw_max;
    AG_U32                  n_ring_max;
    AG_U32                  n_jbf_size;
    
    /* */
    /* memory management stuff */
    /* */
    AgNdMemUnit             *p_mem_ucode;
    AgNdMemUnit             *p_mem_data_strict;
    AgNdMemUnit             *p_mem_ptp_strict;
    AgNdMemUnit             *p_mem_lstree;
    AgNdMemUnit             *p_mem_data_header;
    AgNdMemUnit             *p_mem_cas_header;
    AgNdMemUnit             *p_mem_ptp_header;
    AgNdMemUnit             *p_mem_pbf;
    AgNdMemUnit             *p_mem_jbf;

    AgNdMemUnit             a_mem_unit[AG_ND_MEM_UNIT_MAX];
    AG_U32                  n_mem_unit_count;

    AgNdAllocator           x_allocator_pbf;
    AgNdAllocator           x_allocator_jbf;
    AgNdAllocator           x_allocator_vba;

    /* */
    /* BIT stuff */
    /* */
    AgNdBit                 x_bit;
    AG_BOOL                 b_do_not_reset_mac;

    /* */
    /* channels */
    /* */
    AgNdChannelInfo         a_channel[AG_ND_CHANNEL_MAX];
    AgNdChannelIngressInfo  a_channel_ingress[AG_ND_CHANNEL_MAX];

    /* */
    /* timeslots  */
    /* */
    AgNdTsInfo              (*a_ts_info_table)[AG_ND_SLOT_MAX][AG_ND_PATH_MAX];
    AgNdList                a_ts_list[AG_ND_PATH_MAX]; /* disabled timeslots */

    /* */
    /* RCR stuff */
    /* */
                            /* */
                            /* the maximal possible number of RCR channels is limited */
                            /* by number of circuits */
                            /*  */
    AgNdRcrChannel          a_rcr_channel[AG_ND_SPE_MAX * AG_ND_VTG_MAX * AG_ND_VT_MAX * 2];
    AG_BOOL                 b_rcr_support;


	/* ptp stuff */
    AG_BOOL                 b_ptp_support;

    /* */
    /* circuit architecture dependent callbacks */
    /* */
    AG_BOOL                 (*p_circuit_is_valid_id)(AgNdDevice*, AgNdCircuit);
    AG_U32                  (*p_circuit_get_max_idx)(AgNdDevice*);
    AG_U32                  (*p_circuit_id_to_idx)(AgNdCircuit n_circuit_id);
    AgNdCircuit             (*p_circuit_idx_to_id)(AG_U32 n_circuit_idx);

    /* */
    /* interrupts */
    /* */
    AG_BOOL                 b_isr_mode;

    AgResult                (*p_cb_pmi)(void);

    AgResult                (*p_cb_pbi)(AG_U32 n_channel_id, AG_U32 n_user_data);
    AG_U32                  n_user_data_pbi;

    AgResult                (*p_cb_cwi)(AG_U32 n_channel_id, AG_U32 n_user_data, AG_U16 n_cw);
    AG_U32                  n_user_data_cwi;

    AgResult                (*p_cb_psi)(AG_U32 n_channel_id, AG_U32 n_user_data, AgNdSyncType n_sync);
    AG_U32                  n_user_data_psi;

    AgResult                (*p_cb_tpi)(AG_U32 n_channel_id, AG_U32 user_data);
    AG_U32                  n_user_data_tpi;

    AG_BOOL                 b_intr_task_run;
    AG_BOOL                 b_intr_task_done;
    AG_U32                  n_intr_task_entry_count;

    void                    *p_intr_task_stack;
    AG_U32                  n_intr_task_priority;
    AG_U32                  n_intr_task_wakeup;

    AgNdTask                x_intr_task;
    AgNdTimer               x_intr_timer;
    AgNdEvent               x_intr_event;


    /*  */
    /* debug stuff */
    /*  */
    AG_BOOL                 b_trace;

    AG_U32                  n_bus_access_read_count;
    AG_U32                  n_bus_access_write_count;
    AG_U32                  n_current_bank_pointer;   


    /* */
    /* protection */
    /* */
    AgNdMutex               x_api_lock;


    /* */
    /* chid classification (RPC) */
    /* */
    AgClsbMemHndl           n_lstree_handle;    /* classification builder hw mem area handle */
    MpId                    n_lstree_mpid;      /* miniprogram handle */


    /* */
    /* buffer for packet header template, will be used by bitwise */
    /*  */
    AG_U8                   a_buf[AG_ND_HEADER_MAX + 8];


    /* */
    /*  */
    /*  */
    AG_U32                  n_data_header_memory_block_size;
};

AgNdTsInfo*
ag_nd_get_ts_info(
    AgNdDevice  *p_device,
    AgNdCircuit n_circuit_id,
    AG_U32      n_ts_idx,
    AgNdPath    e_path);


#ifdef __cplusplus
}
#endif

#endif /* __NEMO_HW_H__ */
