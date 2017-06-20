
/*
 * $Id: mde_agent_msg.h,v 1.5 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:      mde_agent_msg.h
 *
 * Purpose:   MDE Agent
 *
 *            Message related defines shared between the MDE Agent and the MDE
 */

#ifndef _MDE_AGENT_MSG_H_
#define _MDE_AGENT_MSG_H_

/* 
 * Message Types 
 */
#define MDE_AGENT_MSG_T_NULL                    0x00

/***************************************************
 * Debug message types
 */
#define MDE_AGENT_MSG_T_DEBUG_BREAKPOINT_SET    0x10
#define MDE_AGENT_MSG_T_DEBUG_BREAKPOINT_CLEAR  0x11
#define MDE_AGENT_MSG_T_DEBUG_REG_READ          0x12
#define MDE_AGENT_MSG_T_DEBUG_REG_WRITE         0x13
#define MDE_AGENT_MSG_T_DEBUG_HEADER_READ       0x14
#define MDE_AGENT_MSG_T_DEBUG_HEADER_WRITE      0x15
#define MDE_AGENT_MSG_T_DEBUG_STEP              0x16
#define MDE_AGENT_MSG_T_DEBUG_CONTINUE          0x17
#define MDE_AGENT_MSG_T_DEBUG_WP_SET            0x18
#define MDE_AGENT_MSG_T_DEBUG_WP_CLEAR          0x19 
#define MDE_AGENT_MSG_T_DEBUG_STATE             0x1A
#define MDE_AGENT_MSG_T_DEBUG_BREAKPOINT_LIST   0x1B
#define MDE_AGENT_MSG_T_DEBUG_BREAKPOINT_CLEAR_ALL  0x1C
#define MDE_AGENT_MSG_T_DEBUG_MAX               0x20 /* Add new debug messages before this point */

/*
 * OCM message types
 */
#define MDE_AGENT_MSG_T_OCM_READ                0x30
#define MDE_AGENT_MSG_T_OCM_WRITE               0x31
#define MDE_AGENT_MSG_T_OCM_TABLE_INFO_GET      0x32
#define MDE_AGENT_MSG_T_OCM_MAX                 0x40 /* Add new OCM message types before this point */

/*
 * uCode message types
 */
#define MDE_AGENT_MSG_T_UCODE_UPDATE            0x50
#define MDE_AGENT_MSG_T_UCODE_SYMBOL_GET        0x51
#define MDE_AGENT_MSG_T_UCODE_SYMBOL_SET        0x52
#define MDE_AGENT_MSG_T_UCODE_MAX               0x60 /* Add new uCode message types before this point */

/*
 * System message types
 */
#define MDE_AGENT_MSG_T_SYSTEM_STATUS           0x70
#define MDE_AGENT_MSG_T_SYSTEM_START            0x71
#define MDE_AGENT_MSG_T_SYSTEM_STOP             0x72
#define MDE_AGENT_MSG_T_SYSTEM_TX_PACKET        0x73
#define MDE_AGENT_MSG_T_SYSTEM_HEADER_CAPTURE   0x74
#define MDE_AGENT_MSG_T_SYSTEM_HEADER_DUMP      0x75
#define MDE_AGENT_MSG_T_SYSTEM_MAX              0x80 /* Add new system message types before this point */

/*
 * End of message types
 **************************************************/



/*
 * Modules
 */
#define    MDE_AGENT_MODULE_NULL    0x00
#define    MDE_AGENT_MODULE_LRP     0x01
#define    MDE_AGENT_MODULE_OCM     0x02
#define    MDE_AGENT_MODULE_SYSTEM  0x03
#define    MDE_AGENT_MODULE_UCODE   0x04


/* 
 * Message flags 
 */
#define MDE_AGENT_MSG_FLAG_LAST_MSG      0x0001
#define MDE_AGENT_MSG_FLAG_RESPONSE      0x0002
#define MDE_AGENT_MSG_FLAG_ACK           0x0004
#define MDE_AGENT_MSG_FLAG_STATE_NULL    0x0000
#define MDE_AGENT_MSG_FLAG_STATE_RUNNING 0x0008
#define MDE_AGENT_MSG_FLAG_STATE_DEBUG   0x0010
#define MDE_AGENT_MSG_FLAG_CONTEXT_VALID 0x0020
#define MDE_AGENT_MSG_FLAG_ACTIVE        0x0040
#define MDE_AGENT_MSG_FLAG_DYNAMIC_BP    0x0080
#define MDE_AGENT_MSG_FLAG_INGRESS       0x0100

/*
 * Misc
 */
#define MDE_AGENT_C3_DEVICE 88030
#define MDE_AGENT_VERSION   1
#define MDE_AGENT_HEADER_SIZE 256
#define MDE_AGENT_MAX_REG_NAME 8
#define MDE_AGENT_MAX_TABLE_NAME_LEN 64


/*
 * Message struct
 */
typedef struct mde_agent_msg_pkt_hdr_s {
    uint8  version;
    uint16 flags;
    uint8  module;
    uint8  type;
    uint8  reserved[3];
    uint32 device;
    uint32 sequence_number;
    uint32 response_code;
    uint32 length;
} __attribute__ ((packed)) mde_agent_msg_pkt_hdr_t;


#endif /* _MDE_AGENT_MSG_H_ */
