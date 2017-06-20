/*
 * $Id: drvtypes.h,v 1.59 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * this header file defines types used in driver services.
 */
 
#ifndef _DRVTYPES_H
#define _DRVTYPES_H

#include <soc/portmode.h>
#include <soc/types.h>
#include <soc/cfp.h>
#include <soc/vm.h>
#include <soc/robo_fp.h>
#include <soc/ll.h>
#include <soc/timesync.h>
#include <shared/switch.h>

/* mem type */
typedef enum drv_mem_e {
    DRV_MEM_ARL = 0x10000,
    DRV_MEM_VLAN,
    DRV_MEM_MSTP,
    DRV_MEM_MCAST,
    DRV_MEM_SECMAC,
    DRV_MEM_GEN,
    DRV_MEM_ARL_HW,
    DRV_MEM_MARL_HW,
    DRV_MEM_MARL,
    DRV_MEM_VLANVLAN, /* VLAN Translation table */
    DRV_MEM_MACVLAN, /* Mac based VLAN table */
    DRV_MEM_PROTOCOLVLAN, /* Protocol VLAN table */
    DRV_MEM_FLOWVLAN, /* Flow based VLAN table */
    DRV_MEM_EGRVID_REMARK, /* Egress VID Remark table */
    DRV_MEM_TCAM_DATA,    
    DRV_MEM_TCAM_MASK,    
    DRV_MEM_CFP_ACT,    
    DRV_MEM_CFP_METER,    
    DRV_MEM_CFP_STAT_IB,    
    DRV_MEM_CFP_STAT_OB,
    DRV_MEM_IRC_PORT,
    DRV_MEM_ERC_PORT,
    DRV_MEM_1P_TO_TCDP,
    DRV_MEM_TCDP_TO_1P,
    DRV_MEM_PORTMASK,           /* Port Mask Table */
    DRV_MEM_SALRN_CNT_CTRL,     /* SA Learn Count Configuration Table */
    DRV_MEM_MCAST_VPORT_MAP,    /* Multicast Group vPort mapping Table */
    DRV_MEM_VPORT_VID_MAP,      /* vPort VID Remarking Table */
    DRV_MEM_IVM_KEY,
    DRV_MEM_IVM_ACT,
    DRV_MEM_EVM_KEY,
    DRV_MEM_EVM_ACT,
    DRV_MEM_CFP_DATA_MASK,
    DRV_MEM_CFP_STAT,
    DRV_MEM_IVM_KEY_DATA_MASK,
    DRV_MEM_EVM_KEY_DATA_MASK,
    DRV_MEM_8051_RAM,
    DRV_MEM_8051_ROM
} drv_mem_t;

/* mem field type */
typedef enum drv_mem_field_e {
    /* ARL/MCAST table */
    DRV_MEM_FIELD_MAC = 0x10000, /* ARL/MCAST/SECMAC */
    DRV_MEM_FIELD_VLANID,
    DRV_MEM_FIELD_SRC_PORT,
    DRV_MEM_FIELD_VPORT,        /* vertial port */
    DRV_MEM_FIELD_USER,
    DRV_MEM_FIELD_DEST_BITMAP, /* MCAST */
    DRV_MEM_FIELD_DEST_BITMAP1,
    DRV_MEM_FIELD_CHIP_ID,
    DRV_MEM_FIELD_PRIORITY,
    DRV_MEM_FIELD_VALID, /* ARL/MCAST/VLAN/SECMAC */
    DRV_MEM_FIELD_VALID1, /* MCAST */
    DRV_MEM_FIELD_STATIC,
    DRV_MEM_FIELD_AGE, /* ARL/MSTP */
    DRV_MEM_FIELD_ARL_CONTROL,
    /* VLAN table */
    DRV_MEM_FIELD_SPT_GROUP_ID,
    DRV_MEM_FIELD_PORT_BITMAP,
    DRV_MEM_FIELD_OUTPUT_UNTAG,
    DRV_MEM_FIELD_FWD_MODE,
    DRV_MEM_FIELD_DIR_FWD,
    DRV_MEM_FIELD_UCAST_DROP,
    DRV_MEM_FIELD_MCAST_DROP,
    DRV_MEM_FIELD_ISO_MAP,
    DRV_MEM_FIELD_POLICER_EN,
    DRV_MEM_FIELD_POLICER_ID,

    /* MSTP table */
    DRV_MEM_FIELD_MSTP_PORTST,

    /* SECMAC (Security MAC) table */

    /* VLAN Translation table */
    DRV_MEM_FIELD_TRANSPARENT_MODE,
    DRV_MEM_FIELD_MAPPING_MODE,
    DRV_MEM_FIELD_NEW_VLANID,

    /* Egress VLAN Translation table */
    DRV_MEM_FIELD_OUTER_OP,
    DRV_MEM_FIELD_OUTER_VID,
    DRV_MEM_FIELD_INNER_OP,
    DRV_MEM_FIELD_INNER_VID,

    /* Ingress Rate Control Port table */
    DRV_MEM_FIELD_IRC_BKT0_REF_CNT,
    DRV_MEM_FIELD_IRC_BKT0_BKT_SIZE,
    DRV_MEM_FIELD_IRC_BKT0_PKT_MASK,
    DRV_MEM_FIELD_IRC_BKT0_IRC_EN,
    DRV_MEM_FIELD_IRC_BKT1_REF_CNT,
    DRV_MEM_FIELD_IRC_BKT1_BKT_SIZE,
    DRV_MEM_FIELD_IRC_BKT1_PKT_MASK,
    DRV_MEM_FIELD_IRC_BKT1_IRC_EN,
    DRV_MEM_FIELD_IRC_BKT2_REF_CNT,
    DRV_MEM_FIELD_IRC_BKT2_BKT_SIZE,
    DRV_MEM_FIELD_IRC_BKT2_PKT_MASK,
    DRV_MEM_FIELD_IRC_BKT2_IRC_EN,
    DRV_MEM_FIELD_IRC_DROP_EN,

    /* Egress Rate Control Port table */
    DRV_MEM_FIELD_ERC_BKT_Q0_REF_CNT_MIN,
    DRV_MEM_FIELD_ERC_BKT_Q0_REF_CNT_MAX,
    DRV_MEM_FIELD_ERC_BKT_Q0_BKT_SIZE,
    DRV_MEM_FIELD_ERC_BKT_Q0_ERC_Q_EN,
    DRV_MEM_FIELD_ERC_BKT_Q1_REF_CNT_MIN,
    DRV_MEM_FIELD_ERC_BKT_Q1_REF_CNT_MAX,
    DRV_MEM_FIELD_ERC_BKT_Q1_BKT_SIZE,
    DRV_MEM_FIELD_ERC_BKT_Q1_ERC_Q_EN,
    DRV_MEM_FIELD_ERC_BKT_Q2_REF_CNT_MIN,
    DRV_MEM_FIELD_ERC_BKT_Q2_REF_CNT_MAX,
    DRV_MEM_FIELD_ERC_BKT_Q2_BKT_SIZE,
    DRV_MEM_FIELD_ERC_BKT_Q2_ERC_Q_EN,
    DRV_MEM_FIELD_ERC_BKT_Q3_REF_CNT_MIN,
    DRV_MEM_FIELD_ERC_BKT_Q3_REF_CNT_MAX,
    DRV_MEM_FIELD_ERC_BKT_Q3_BKT_SIZE,
    DRV_MEM_FIELD_ERC_BKT_Q3_ERC_Q_EN,
    DRV_MEM_FIELD_ERC_BKT_Q4_REF_CNT_MIN,
    DRV_MEM_FIELD_ERC_BKT_Q4_REF_CNT_MAX,
    DRV_MEM_FIELD_ERC_BKT_Q4_BKT_SIZE,
    DRV_MEM_FIELD_ERC_BKT_Q4_ERC_Q_EN,
    DRV_MEM_FIELD_ERC_BKT_Q5_REF_CNT_MIN,
    DRV_MEM_FIELD_ERC_BKT_Q5_REF_CNT_MAX,
    DRV_MEM_FIELD_ERC_BKT_Q5_BKT_SIZE,
    DRV_MEM_FIELD_ERC_BKT_Q5_ERC_Q_EN,
    DRV_MEM_FIELD_ERC_BKT_Q6_REF_CNT_MIN,
    DRV_MEM_FIELD_ERC_BKT_Q6_REF_CNT_MAX,
    DRV_MEM_FIELD_ERC_BKT_Q6_BKT_SIZE,
    DRV_MEM_FIELD_ERC_BKT_Q6_ERC_Q_EN,
    DRV_MEM_FIELD_ERC_BKT_Q7_REF_CNT_MIN,
    DRV_MEM_FIELD_ERC_BKT_Q7_REF_CNT_MAX,
    DRV_MEM_FIELD_ERC_BKT_Q7_BKT_SIZE,
    DRV_MEM_FIELD_ERC_BKT_Q7_ERC_Q_EN,
    DRV_MEM_FIELD_ERC_BKT_T_REF_CNT_MAX,
    DRV_MEM_FIELD_ERC_BKT_T_BKT_SIZE,
    DRV_MEM_FIELD_ERC_BKT_T_TYPE,
    DRV_MEM_FIELD_ERC_BKT_T_ERC_T_EN,
    DRV_MEM_FIELD_ERC_BKT_BAC,

    /* 1P-to-TCDP Port Mapping table */
    DRV_MEM_FIELD_TC,
    DRV_MEM_FIELD_DP,

    /* TCDP-to-1P Port Mapping table */
    DRV_MEM_FIELD_C_DEI,
    DRV_MEM_FIELD_C_PCP,
    DRV_MEM_FIELD_S_DEI,
    DRV_MEM_FIELD_S_PCP,

    /* Port Mask Table */
    DRV_MEM_FIELD_MASK_ANY,
    DRV_MEM_FIELD_MASK_DLF_UCST,
    DRV_MEM_FIELD_MASK_DLF_L2MCST,
    DRV_MEM_FIELD_MASK_DLF_L3MCAST,
    DRV_MEM_FIELD_MASK_BCST,
    DRV_MEM_FIELD_PORT_CONFIG,
    
    /* SA Learn Count Configuration Table */
    DRV_MEM_FIELD_SA_LRN_CNT_LIM,
    DRV_MEM_FIELD_SA_LRN_CNT_NO,
    
    /* vPort VID Remarking Table */
    /* No other DRV field id betweeen DRV_MEM_FIELD_VPORT_VID0 and 
     *  DRV_MEM_FIELD_VPORT_VID15 is allowed for some special field ID process 
     *  in subport.c(src/soc/robo/bcm53280) has a MACRO defined to reference 
     *  here.
     */
    DRV_MEM_FIELD_VPORT_VID0,
    DRV_MEM_FIELD_VPORT_VID1,
    DRV_MEM_FIELD_VPORT_VID2,
    DRV_MEM_FIELD_VPORT_VID3,
    DRV_MEM_FIELD_VPORT_VID4,
    DRV_MEM_FIELD_VPORT_VID5,
    DRV_MEM_FIELD_VPORT_VID6,
    DRV_MEM_FIELD_VPORT_VID7,
    DRV_MEM_FIELD_VPORT_VID8,
    DRV_MEM_FIELD_VPORT_VID9,
    DRV_MEM_FIELD_VPORT_VID10,
    DRV_MEM_FIELD_VPORT_VID11,
    DRV_MEM_FIELD_VPORT_VID12,
    DRV_MEM_FIELD_VPORT_VID13,
    DRV_MEM_FIELD_VPORT_VID14,
    DRV_MEM_FIELD_VPORT_VID15,
    DRV_MEM_FIELD_VPORT_UNTAG,

    /* VM Key table*/
    DRV_MEM_FIELD_VM_KEY_DATA,
    DRV_MEM_FIELD_VM_KEY_MASK,
    
    /* Protocol VLAN table */
    DRV_MEM_FIELD_ETHER_TYPE

} drv_mem_field_t;

/* mem search/insert/delete flags */
typedef enum drv_mem_op_flag_e {
    DRV_MEM_OP_BY_INDEX           = 0x1,
    DRV_MEM_OP_BY_HASH_BY_MAC     = 0x2,
    DRV_MEM_OP_BY_HASH_BY_VLANID  = 0x4,
    
    DRV_MEM_OP_DELETE_BY_PORT = 0x8,
    DRV_MEM_OP_DELETE_BY_VLANID = 0x10,
    DRV_MEM_OP_DELETE_BY_SPT =0x20,
    DRV_MEM_OP_DELETE_BY_STATIC =0x40,
    DRV_MEM_OP_DELETE_ALL_ARL =0x80,
    DRV_MEM_OP_DELETE_BY_DYN_MCAST =0x100,
    DRV_MEM_OP_DELETE_BY_ST_MCAST =0x200,
    DRV_MEM_OP_DELETE_BY_DYN_UCAST =0x400,
    DRV_MEM_OP_DELETE_BY_ST_UCAST =0x800,
    DRV_MEM_OP_DELETE_BY_TRUNK =0x1000,
    DRV_MEM_OP_DELETE_BY_STATIC_ONLY =0x2000,   /* dynamic is excluded */
    DRV_MEM_OP_DELETE_BY_MCAST =0x4000,
    DRV_MEM_OP_DELETE_BY_MCAST_ONLY =0x8000,    /* unicast is excluded */
    
    DRV_MEM_OP_SEARCH_VALID_START = 0x10000,
    DRV_MEM_OP_SEARCH_VALID_GET = 0x20000,
    DRV_MEM_OP_SEARCH_PORT = 0x40000,
    DRV_MEM_OP_SEARCH_DONE = 0x80000,
    DRV_MEM_OP_SEARCH_CONFLICT = 0x100000,  /* search the conflict entries */
    DRV_MEM_OP_REPLACE = 0x200000,      /* replace dynamic when full */
    DRV_MEM_OP_PENDING = 0x400000,
    DRV_MEM_OP_MODIFY = 0x800000,        /* modify existing entry */
    DRV_MEM_OP_HIT = 0x1000000
} drv_mem_op_flag_t;

/* trunk hash field type */
typedef enum drv_trunk_hash_field_e {
    DRV_TRUNK_HASH_FIELD_MACDA = 0x1,
    DRV_TRUNK_HASH_FIELD_MACSA = 0x2,
    DRV_TRUNK_HASH_FIELD_ETHERTYPE = 0x4,
    DRV_TRUNK_HASH_FIELD_VLANID = 0x8,
    DRV_TRUNK_HASH_FIELD_IP_MACDA = 0x10,
    DRV_TRUNK_HASH_FIELD_IP_MACSA = 0x20,
    DRV_TRUNK_HASH_FIELD_IP_ETHERTYPE = 0x40,
    DRV_TRUNK_HASH_FIELD_IP_VLANID = 0x80,
    DRV_TRUNK_HASH_FIELD_L3 = 0x100
} drv_trunk_hash_field_t;

/* trunk flag type */
typedef enum drv_trunk_flag_e {
    DRV_TRUNK_FLAG_ENABLE = 0x1,
    DRV_TRUNK_FLAG_DISABLE = 0x2,
    DRV_TRUNK_FLAG_BITMAP = 0x4,
    DRV_TRUNK_FLAG_HASH_CURRENT = 0x8,
    DRV_TRUNK_FLAG_HASH_DEFAULT = 0x10
} drv_trunk_flag_t;

/* trunk timers */
typedef enum drv_trunk_timer_e {
    DRV_TRUNK_TIMER_FAILOVER = 1,
    DRV_TRUNK_TIMER_FAILBACK
} drv_trunk_timer_t;

/* System basis control type(management type/mode) */
typedef enum drv_dev_ctrl_e {
    /* used by (dev_control_get/set) */
    DRV_DEV_CTRL_MCASTREP = 0x0,      /* on/off multicast replication */
    DRV_DEV_CTRL_L2_USERADDR,      /* on/off L2 user MAC address */
    DRV_DEV_CTRL_RATE_METER_PLUS_IPG, /* rate meter operation will count IPG  
                                       * and preamble. (12+8 bytes). */
    DRV_DEV_CTRL_ARPDHCP_TOCPU,     /* on/off ARP/DHCP copy to CPU */
    DRV_DEV_CTRL_IGMP_MLD_SNOOP_MODE, /* set IGMP/MLD check mode */
    DRV_DEV_CTRL_DA_ALL0_DROP,      /* drop all zero dest MAC */
    DRV_DEV_CTRL_MCAST_SA_DROP,      /* drop if SA is mcast */
    DRV_DEV_CTRL_RANGE_ERROR_DROP,      /* drop if range error */
    DRV_DEV_CTRL_JUMBO_FRAME_DROP,      /* drop if jumbo frame */
    DRV_DEV_CTRL_DROPPED_RANGE_ERR_NO_LEARN, /* no learn on the dropped frame 
                                              * with range error */
    DRV_DEV_CTRL_DROPPED_JUMBO_FRM_NO_LEARN, /* no learn on the dropped jumbo 
                                              * frame */

    DRV_DEV_CTRL_FL_BYPASS_SUPPORT_LIST, /* supported bypass items (bitmap) */
    
    /* --- bypass control related --- */
    DRV_DEV_CTRL_RX_BYPASS_CRCCHK,  /* on/off CPU Rx bypass CRC check */
    DRV_DEV_CTRL_DOS_BYPASS_TOCPU,  /* on/off DOC bypass copy to CPU */
    DRV_DEV_CTRL_STP_BYPASS_USERADDR, /* on/off STP bypass L2 user_addr */
    DRV_DEV_CTRL_STP_BYPASS_MAC0X,  /* on/off STP bypass MAC0X */
    /* VLAN related bypass were defined in drv_vlan_prop_type_t */
    DRV_DEV_CTRL_EAP_BYPASS_USERADDR,  /* on/off EAP bypass L2 user_addr */
    DRV_DEV_CTRL_EAP_BYPASS_DHCP,  /* on/off EAP bypass DHCP */
    DRV_DEV_CTRL_EAP_BYPASS_ARP,  /* on/off EAP bypass ARP */
    DRV_DEV_CTRL_EAP_BYPASS_MAC_22_2F,  /* on/off EAP bypass MAC at 22-2F */
    DRV_DEV_CTRL_EAP_BYPASS_MAC_21,  /* on/off EAP bypass MAC at 21 */
    DRV_DEV_CTRL_EAP_BYPASS_MAC_20,  /* on/off EAP bypass MAC at 20 */
    DRV_DEV_CTRL_EAP_BYPASS_MAC_11_1F,  /* on/off EAP bypass MAC at 11-1F */
    DRV_DEV_CTRL_EAP_BYPASS_MAC_10,  /* on/off EAP bypass MAC at 10 */
    DRV_DEV_CTRL_EAP_BYPASS_MAC_0X,  /* on/off EAP bypass MAC at 0X */
    DRV_DEV_CTRL_EAP_BYPASS_MAC_BPDU,  /* on/off EAP bypass BPDU */
    DRV_DEV_CTRL_RESERVED_MCAST_SA_LEARN,  /* on/off SA learning of reserved multicasts */
    DRV_DEV_CTRL_CPU_RXULF,  /* on/off CPU to receive Ucast DLF frame */
    DRV_DEV_CTRL_CPU_RXMLF,  /* on/off CPU to receive Mcast DLF frame */

    DRV_DEV_CTRL_ARLBINFULL_TOCPU,  /* on/off CPU to receive ARL BIN Full frame */
    DRV_DEV_CTRL_ARLBINFULL_CNT,  /* get/set ARL BIN Full counter */

    DRV_DEV_CTRL_EGRESS_PPPOEDSCP_REMARK,    /* remark dscp on PPPoE frame */
    DRV_DEV_CTRL_CNT      /* number of the system control items */
    
}drv_dev_ctrl_t;

/* System configuration property type */
typedef enum drv_dev_prop_e {
    /* used by (dev_prop_get/set) */
    DRV_DEV_PROP_MCAST_NUM = 1,
    DRV_DEV_PROP_MCAST_REP_NUM,
    DRV_DEV_PROP_VPORT_MCAST_REP,
    DRV_DEV_PROP_AGE_TIMER_MAX_S,
    DRV_DEV_PROP_AGE_HIT_VALUE,
    DRV_DEV_PROP_TRUNK_NUM,
    DRV_DEV_PROP_TRUNK_MAX_PORT_NUM,
    DRV_DEV_PROP_COSQ_NUM,
    DRV_DEV_PROP_MSTP_NUM,
    DRV_DEV_PROP_SEC_MAC_NUM_PER_PORT,
    DRV_DEV_PROP_COSQ_MAX_WEIGHT_VALUE,
    DRV_DEV_PROP_AUTH_PBMP,
    DRV_DEV_PROP_RATE_CONTROL_PBMP,
    DRV_DEV_PROP_VLAN_ENTRY_NUM,
    DRV_DEV_PROP_BPDU_NUM,
    DRV_DEV_PROP_INTERNAL_MII_PBMP,
    DRV_DEV_PROP_EXTERNAL_MII_PBMP,
    DRV_DEV_PROP_CFP_TCAM_SIZE,
    DRV_DEV_PROP_CFP_UDFS_NUM,
    DRV_DEV_PROP_CFP_RNG_NUM,
    DRV_DEV_PROP_CFP_VID_RNG_NUM,
    DRV_DEV_PROP_CFP_UDFS_OFFSET_MAX,
    DRV_DEV_PROP_SA_STATION_MOVE_DROP,
    DRV_DEV_PROP_AUTH_SEC_MODE,
    DRV_DEV_PROP_IVM_TCAM_SIZE,
    DRV_DEV_PROP_EVM_TCAM_SIZE,
    DRV_DEV_PROP_IVM_RNG_NUM,
    DRV_DEV_PROP_PROFILE_NUM,
    DRV_DEV_PROP_FLOW_ID_NUM,
    DRV_DEV_PROP_VLAN_TRANSLATE_MODE_NUM,
    DRV_DEV_PROP_MAX_INGRESS_SFLOW_VALUE,
    DRV_DEV_PROP_MAX_EGRESS_SFLOW_VALUE,
    DRV_DEV_PROP_EEE_GLOBAL_CONG_THRESH,
    DRV_DEV_PROP_EEE_PIPELINE_TIMER,
    DRV_DEV_PROP_EEE_INIT,
    DRV_DEV_PROP_RESOURCE_ARBITER_REQ,
    DRV_DEV_PROP_SUPPORTED_LED_FUNCTIONS,
    /* Low Power Mode for MAC chip */
    DRV_DEV_PROP_LOW_POWER_ENABLE, /* enable/disable MAC low power mode */
    /* port bitmap that support low power mode */
    DRV_DEV_PROP_LOW_POWER_SUPPORT_PBMP,
    DRV_DEV_PROP_POWER_DOWN_SUPPORT_PBMP,
    DRV_DEV_PROP_LOW_POWER_PHY_CFG_RESTORE_PBMP,
    DRV_DEV_PROP_ULTRA_LOW_POWER,
    /* Internal CPU port number */
    DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM,
    /* Additional SOC port number */
    DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM,
    /* IMP1 port number */
    DRV_DEV_PROP_IMP1_PORT_NUM, 
    /* COSQ ID for output of SCH1 */
    DRV_DEV_PROP_SCH2_OUTPUT_COSQ,
    /* NUMBER of COSQ for SCH1 */
    DRV_DEV_PROP_SCH1_NUM_COSQ, 
    /* NUMBER of COSQ for SCH2 */
    DRV_DEV_PROP_SCH2_NUM_COSQ,
    DRV_DEV_PROP_PPPOE_SESSION_ETYPE, /* Ethertype of PPPOE session packets */
    DRV_DEV_PROP_SWITCHMACSEC_SW_INIT, /* Swich-MACSEC switch layer init */
    DRV_DEV_PROP_SWITCHMACSEC_EN_PBMP, /* Swich-MACSEC enabled bitmap in this device */
    DRV_DEV_PROP_SWITCHMACSEC_ATTACH_PBMP /* Swich-MACSEC attachd bitmap in this device */
} drv_dev_prop_t;

/* tag or port-base vlan */
typedef enum drv_vlan_mode_e {
    DRV_VLAN_MODE_TAG = 1,
    DRV_VLAN_MODE_PORT_BASE,
    DRV_VLAN_MODE_MISS_DROP_EN,
    DRV_VLAN_MODE_MISS_DROP_DIS
} drv_vlan_mode_t;

/* vlan property */
typedef enum drv_vlan_prop_type_e {
    DRV_VLAN_PROP_VTABLE_MISS_DROP,
    DRV_VLAN_PROP_VTABLE_MISS_LEARN,
    DRV_VLAN_PROP_VTABLE_MISS_TO_CPU,
    DRV_VLAN_PROP_MEMBER_MISS_DROP,
    DRV_VLAN_PROP_MEMBER_MISS_LEARN,
    DRV_VLAN_PROP_MEMBER_MISS_TO_CPU,
    DRV_VLAN_PROP_VLAN_LEARNING_MODE, /* SVL, IVL selection */
    DRV_VLAN_PROP_SP_TAG_TPID,      /* service provider tag TPID */
    DRV_VLAN_PROP_DOUBLE_TAG_MODE,
    DRV_VLAN_PROP_IDT_MODE_ENABLE,  /* iDT_Mode(intelligent DT_Mode) */
    DRV_VLAN_PROP_V2V_INIT,     /* bcm53242 v2v table */
    DRV_VLAN_PROP_EVR_INIT,     /* bcm53115 EVR table */
    DRV_VLAN_PROP_V2V,     
    DRV_VLAN_PROP_ISP_PORT,
    DRV_VLAN_PROP_V2V_PORT,
    DRV_VLAN_PROP_MAC2V_PORT,
    DRV_VLAN_PROP_PROTOCOL2V_PORT,
    DRV_VLAN_PROP_TRUST_VLAN_PORT,
    DRV_VLAN_PROP_INNER_TAG_PORT,
    DRV_VLAN_PROP_PER_PORT_TRANSLATION,
    DRV_VLAN_PROP_TRANSLATE_MODE,
    DRV_VLAN_PROP_GVRP_TO_CPU,
    DRV_VLAN_PROP_GMRP_TO_CPU,
    DRV_VLAN_PROP_BYPASS_IGMP_MLD,
    DRV_VLAN_PROP_BYPASS_ARP_DHCP,
    DRV_VLAN_PROP_BYPASS_MIIM,
    DRV_VLAN_PROP_BYPASS_MCAST,
    DRV_VLAN_PROP_BYPASS_RSV_MCAST,
    DRV_VLAN_PROP_BYPASS_L2_USER_ADDR,
    DRV_VLAN_PROP_UNTAGGED_DROP,
    DRV_VLAN_PROP_PRI_TAGGED_DROP,
    DRV_VLAN_PROP_INGRESS_VT_HIT_DROP,
    DRV_VLAN_PROP_INGRESS_VT_MISS_DROP,
    DRV_VLAN_PROP_EGRESS_VT_MISS_UNTAG,
    DRV_VLAN_PROP_POLICING,
    DRV_VLAN_PROP_JOIN_ALL_VLAN,
    DRV_VLAN_PROP_COUNT
}drv_vlan_prop_type_t;

/* vlan xlate property */
typedef enum drv_vt_table_type_e{
    DRV_VLAN_XLAT_INGRESS,
    DRV_VLAN_XLAT_EGRESS,
    DRV_VLAN_XLAT_INGRESS_MAP,  /* VT_Type + VT_Mode(Map) */
    DRV_VLAN_XLAT_INGRESS_TRAN, /* VT_Type + VT_Mode(Transparent) */
    DRV_VLAN_XLAT_INGRESS_PER_PORT,
    
    DRV_VLAN_XLAT_EVR  /* target at Egress VLAN Remark table */
}drv_vt_tbl_type_t;

/* vlan xlate property (start from 100 to prevent the conflict with 
 * drv_vlan_prop_type_t) 
 */
typedef enum drv_vt_prop_type_e{
    DRV_VLAN_PROP_VT_MODE = 100,      /* VT mode : trasparent / mapping */
    DRV_VLAN_PROP_ING_VT_SPVID,     /* ingress SP VID */
    DRV_VLAN_PROP_EGR_VT_SPVID,     /* egress SP VID */
    DRV_VLAN_PROP_ING_VT_PRI,
    DRV_VLAN_PROP_EGR_VT_PRI,
    DRV_VLAN_PROP_ING_VT_SPTPID,    /* ingress SP TPID */
    DRV_VLAN_PROP_EGR_VT_SPTPID,     /* egress SP TPID */

    /* for CFP based VLAN translation */
    DRV_VLAN_PROP_EVR_VT_NEW_ENTRY_ID,  /* indicating to a EVR index */
    DRV_VLAN_PROP_EVR_VT_ISP_CHANGE  /* indicating to a EVR index */
    
}drv_vt_prop_type_t;

/* queue operation flag */
typedef enum drv_q_flag_e {
    DRV_QUEUE_FLAG_LEVLE2 = 0x1
} drv_q_flag_t;

/* queue operation mode */
typedef enum drv_q_mode_e {
    DRV_QUEUE_MODE_STRICT = 1,
    DRV_QUEUE_MODE_WRR,
    DRV_QUEUE_MODE_HYBRID,
    DRV_QUEUE_MODE_1STRICT,
    DRV_QUEUE_MODE_2STRICT,
    DRV_QUEUE_MODE_1STRICT_7WDRR,
    DRV_QUEUE_MODE_2STRICT_6WDRR,
    DRV_QUEUE_MODE_3STRICT_5WDRR,
    DRV_QUEUE_MODE_4STRICT_4WDRR,
    DRV_QUEUE_MODE_WDRR
} drv_q_mode_t;

typedef enum drv_q_map_e {
    DRV_QUEUE_MAP_NONE = 0,
    DRV_QUEUE_MAP_PRIO,
    DRV_QUEUE_MAP_TOS,
    DRV_QUEUE_MAP_DFSV,
    DRV_QUEUE_MAP_PORT, /* Port based priority */
    DRV_QUEUE_MAP_MAC, /* Mac based priority */
    DRV_QUEUE_MAP_HYBRID, /* hybrid priority from port-based, Diffserv, 802.1p, ... */
    DRV_QUEUE_MAP_PRIO_S1P, /* 802.1p priority for Service PCP */
    DRV_QUEUE_MAP_PRIO_C1P, /* 802.1p priority for Customer PCP */
    DRV_QUEUE_MAP_TC_SEL_RAW /* Raw date format on setting TC selection */
} drv_q_map_t;

typedef enum drv_rx_reason_e {
    DRV_RX_REASON_MIRRORING,
    DRV_RX_REASON_SA_LEARNING,
    DRV_RX_REASON_SWITCHING,
    DRV_RX_REASON_PROTO_TERM,
    DRV_RX_REASON_PROTO_SNOOP,
    DRV_RX_REASON_EXCEPTION,
    DRV_RX_REASON_ARL_8021_PROT_TRAP,
    DRV_RX_REASON_ARL_VALN_DIR_FWD,
    DRV_RX_REASON_ARL_CFP_FWD,
    DRV_RX_REASON_ARL_LOOPBACK,
    DRV_RX_REASON_INGRESS_SFLOW,
    DRV_RX_REASON_EGRESS_SFLOW,
    DRV_RX_REASON_SA_MOVEMENT_EVENT,
    DRV_RX_REASON_SA_UNKNOWN_EVENT,
    DRV_RX_REASON_SA_OVER_LIMIT_EVENT,
    DRV_RX_REASON_INP_NON_MEMBER,
    DRV_RX_REASON_VLAN_UNKNOWN,
    DRV_RX_REASON_COUNT
}drv_rx_reason_t;

typedef enum drv_qos_ctl_e {
    DRV_QOS_CTL_QOS_EN = 1,
    DRV_QOS_CTL_USE_TC,
    DRV_QOS_CTL_WDRR_GRANULARTTY,
    DRV_QOS_CTL_WDRR_TXQEMPTY,
    DRV_QOS_CTL_WDRR_NEGCREDIT_CLR,
    DRV_QOS_CTL_WDRR_BURSTMODE,
    DRV_QOS_CTL_DP_VALUE_DLF,
    DRV_QOS_CTL_DP_CHANGE_DLF,
    DRV_QOS_CTL_DP_CHANGE_XOFF,
    DRV_QOS_CTL_SW_SHADOW,
    DRV_QOS_CTL_FLOOD_DROP_TCMAP
} drv_qos_ctl_t;

#define DRV_COSQ_SCHEDULER_EMPTY_TX_QUEUE   (0x0)
#define DRV_COSQ_SCHEDULER_EMPTY_TXQ_SHAPER (0x1)

/* port type */
typedef enum drv_port_type_e {
    DRV_PORT_TYPE_10_100 = 1,
    DRV_PORT_TYPE_G,
    DRV_PORT_TYPE_XG,
    DRV_PORT_TYPE_CPU, /* MII port */
    DRV_PORT_TYPE_MGNT, /* SPI */
    DRV_PORT_TYPE_ALL
} drv_port_type_t;

/* port state */
typedef enum drv_portst_e {
    DRV_PORTST_DISABLE = 1,
    DRV_PORTST_BLOCK,
    DRV_PORTST_LISTEN,
    DRV_PORTST_LEARN,
    DRV_PORTST_FORWARD
} drv_portst_t;

/* station mac property */
typedef enum drv_mac_type_e {
    DRV_MAC_CUSTOM_BPDU = 1,
    DRV_MAC_MULTIPORT_0,
    DRV_MAC_MULTIPORT_1,
    DRV_MAC_MULTIPORT_2,
    DRV_MAC_MULTIPORT_3,
    DRV_MAC_MULTIPORT_4,
    DRV_MAC_MULTIPORT_5,
    DRV_MAC_CUSTOM_EAP,
    DRV_MAC_MIRROR_IN,
    DRV_MAC_MIRROR_OUT,
    DRV_MAC_IGMP_REPLACE,
    DRV_MAC_SECURITY_ADD,
    DRV_MAC_SECURITY_REMOVE,
    DRV_MAC_SECURITY_CLEAR
} drv_mac_type_t;

/* port/mac advertisable abilities */
/* adopt abilities defined in port.h */
typedef enum drv_abil_type_e {
    DRV_PORT_ABIL_10MB_HD = SOC_PM_10MB_HD,
    DRV_PORT_ABIL_10MB_FD = SOC_PM_10MB_FD,
    DRV_PORT_ABIL_100MB_HD = SOC_PM_100MB_HD,
    DRV_PORT_ABIL_100MB_FD = SOC_PM_100MB_FD,
    DRV_PORT_ABIL_1000MB_HD = SOC_PM_1000MB_HD,
    DRV_PORT_ABIL_1000MB_FD = SOC_PM_1000MB_FD,
    DRV_PORT_ABIL_10GB_HD = SOC_PM_10GB_HD,
    DRV_PORT_ABIL_10GB_FD = SOC_PM_10GB_FD,
    DRV_PORT_ABIL_TBI = SOC_PM_TBI,
    DRV_PORT_ABIL_MII = SOC_PM_MII,
    DRV_PORT_ABIL_GMII = SOC_PM_GMII,
    DRV_PORT_ABIL_SGMII = SOC_PM_SGMII,
    DRV_PORT_ABIL_XGMII = SOC_PM_XGMII,
    DRV_PORT_ABIL_LB_MAC = SOC_PM_LB_MAC,
    DRV_PORT_ABIL_LB_PHY = SOC_PM_LB_PHY,
    DRV_PORT_ABIL_PAUSE_TX = SOC_PM_PAUSE_TX,
    DRV_PORT_ABIL_PAUSE_RX = SOC_PM_PAUSE_RX,
    DRV_PORT_ABIL_PAUSE = SOC_PM_PAUSE,
    DRV_PORT_ABIL_PAUSE_ASYMM = SOC_PM_PAUSE_ASYMM
} drv_abil_type_t;

/* port operation property */
/* constants for port speed/duplex/an/pause -- bitmap */
typedef enum drv_port_prop_e {
    DRV_PORT_PROP_SPEED= 1,
    DRV_PORT_PROP_DUPLEX,
    DRV_PORT_PROP_AUTONEG,
    DRV_PORT_PROP_TX_PAUSE,
    DRV_PORT_PROP_RX_PAUSE,
    DRV_PORT_PROP_LOCAL_ADVERTISE,
    DRV_PORT_PROP_REMOTE_ADVERTISE,
    DRV_PORT_PROP_PORT_ABILITY,
    DRV_PORT_PROP_MAC_ABILITY,
    DRV_PORT_PROP_PHY_ABILITY,
    DRV_PORT_PROP_INTERFACE,
    DRV_PORT_PROP_MAC_ENABLE,
    DRV_PORT_PROP_ENABLE,
    DRV_PORT_PROP_ENABLE_DROP_NON1Q,
    DRV_PORT_PROP_ENABLE_DROP_1Q,
    DRV_PORT_PROP_ENABLE_DROP_UNTAG,
    DRV_PORT_PROP_ENABLE_DROP_PRITAG,
    DRV_PORT_PROP_ENABLE_DROP_1QTAG_IVM_HIT,
    DRV_PORT_PROP_ENABLE_DROP_1QTAG_IVM_MISS,
    DRV_PORT_PROP_ENABLE_RX,
    DRV_PORT_PROP_ENABLE_TX,
    DRV_PORT_PROP_ENABLE_TXRX,
    DRV_PORT_PROP_IPG_FE,
    DRV_PORT_PROP_IPG_GE,
    DRV_PORT_PROP_JAM,
    DRV_PORT_PROP_BPDU_RX,
    DRV_PORT_PROP_RESTART_AUTONEG,
    DRV_PORT_PROP_MAC_LOOPBACK,
    DRV_PORT_PROP_PHY_LOOPBACK,
    DRV_PORT_PROP_PHY_MEDIUM,
    DRV_PORT_PROP_PHY_MDIX,
    DRV_PORT_PROP_PHY_MDIX_STATUS,
    DRV_PORT_PROP_MS,
    DRV_PORT_PROP_SEC_MAC_MODE_NONE,
    DRV_PORT_PROP_SEC_MAC_MODE_STATIC_ACCEPT,
    DRV_PORT_PROP_SEC_MAC_MODE_STATIC_REJECT,
    DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM,
    DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH,
    DRV_PORT_PROP_SEC_MAC_MODE_EXTEND,
    DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY,
    DRV_PORT_PROP_SA_MOVE_DROP,
    DRV_PORT_PROP_SA_MOVE_CPUCOPY,
    DRV_PORT_PROP_SA_UNKNOWN_DROP,
    DRV_PORT_PROP_SA_UNKNOWN_CPUCOPY,
    DRV_PORT_PROP_SA_OVERLIMIT_DROP,
    DRV_PORT_PROP_SA_OVERLIMIT_CPUCOPY,
    DRV_PORT_PROP_ROAMING_OPT,          /* for station move operation */
    DRV_PORT_PROP_EAP_EN_CHK_OPT,
    DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP,
    DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU,   /* to CPU */
    DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_NONE,    
    DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION,    
    DRV_PORT_PROP_DISABLE_LEARN,   /* force disable SA learning */
    DRV_PORT_PROP_SW_LEARN_MODE,   /* switch learning mode between SW/HW */
    DRV_PORT_PROP_PHY_LINKUP_EVT,
    DRV_PORT_PROP_PHY_LINKDN_EVT,
    DRV_PORT_PROP_PHY_RESET,
    DRV_PORT_PROP_PHY_CABLE_DIAG,
    DRV_PORT_PROP_PHY_LINK_CHANGE,
    DRV_PORT_PROP_PAUSE_FRAME_BYPASS_RX,
    DRV_PORT_PROP_PAUSE_FRAME_BYPASS_TX,
    DRV_PORT_PROP_DTAG_MODE_INTELLIGENT,
    DRV_PORT_PROP_DTAG_MODE,
    DRV_PORT_PROP_DTAG_ISP_PORT,
    DRV_PORT_PROP_DTAG_TPID,
    DRV_PORT_PROP_802_1X_MODE,
    DRV_PORT_PROP_802_1X_BLK_RX,
    DRV_PORT_PROP_MAC_BASE_VLAN,
    DRV_PORT_PROP_MAX_FRAME_SZ,
    DRV_PORT_PROP_EGRESS_DSCP_REMARK,   /* remark egress packet DSCP */
    DRV_PORT_PROP_EGRESS_ECN_REMARK,   /* remark egress packet ECN*/
    DRV_PORT_PROP_EGRESS_PCP_REMARK,    /* remark egress packet dot1p pri */
    DRV_PORT_PROP_EGRESS_CFI_REMARK,    /* remark egress packet dot1p cfi */
    DRV_PORT_PROP_EGRESS_C_PCP_REMARK,   /* remark egress packet c-tag pri */
    DRV_PORT_PROP_EGRESS_S_PCP_REMARK,   /* remark egress packet s-tag pri */
    DRV_PORT_PROP_INGRESS_VLAN_CHK,
    DRV_PORT_PROP_SFLOW_INGRESS_RATE,
    DRV_PORT_PROP_SFLOW_EGRESS_RATE,
    DRV_PORT_PROP_SFLOW_INGRESS_PRIO,
    DRV_PORT_PROP_SFLOW_EGRESS_PRIO,
    DRV_PORT_PROP_DEFAULT_TC_PRIO,
    DRV_PORT_PROP_DEFAULT_DROP_PRECEDENCE,
    DRV_PORT_PROP_UNTAG_DEFAULT_TC,
    DRV_PORT_PROP_PROFILE,
    DRV_PORT_PROP_EEE_ENABLE,
    DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_G,
    DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_H,
    DRV_PORT_PROP_EEE_MIN_LPI_TIMER_G,
    DRV_PORT_PROP_EEE_MIN_LPI_TIMER_H,
    DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_G,
    DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_H,    
    DRV_PORT_PROP_MIB_CLEAR, /* Clear MIB counter */
    DRV_PORT_PROP_RAW_TC_MAP_MODE_SELECT, /* TC map mode selection(raw value)*/
    DRV_PORT_PROP_PPPOE_PARSE_EN /* Enable to parse PPPoE session packets */
    
} drv_port_prop_t;

/* ROBO's learn limit actions */
#define DRV_PORT_LEARN_LIMIT_ACTION_NONE            (0x0)
#define DRV_PORT_LEARN_LIMIT_ACTION_DROP            (0x1)
#define DRV_PORT_LEARN_LIMIT_ACTION_COPY2CPU        (0x2)
#define DRV_PORT_LEARN_LIMIT_ACTION_REDIRECT2CPU    (0x3)
#define DRV_PORT_LEARN_LIMIT_ACTION_MASK            (0x3)

typedef enum drv_port_rate_limit_action_e {
    DRV_PORT_PROP_INGRESS_RATE_LIMIT_ACTION_DROP = 1,
    DRV_PORT_PROP_INGRESS_RATE_LIMIT_ACTION_FLOW_CONTROL
} drv_port_rate_limit_action_t;

/* flags for the actions when SA move occurred */
typedef enum drv_station_move_action_e {
    DRV_SA_MOVE_ARL = 0x0001,       /* Port Update in ARL */
    DRV_SA_MOVE_CPU = 0x0002,       /* to CPU */
    DRV_SA_MOVE_DROP = 0x0004,      /* Drop (No forward) */
    DRV_SA_MOVE_PEND = 0x0008       /* Pending Learning */
} drv_station_move_action_t;

/* flag types for flood block type */
typedef enum drv_block_type_e {
    DRV_BLOCK_ALL = 0x0001,             /* block all packets */
    DRV_BLOCK_DLF_UCAST = 0x0002,
    DRV_BLOCK_DLF_MCAST = 0x0004,
    DRV_BLOCK_BCAST = 0x0008,
    
    DRV_BLOCK_DLF_IP_MCAST = 0x0010,    /* subtype in DLF_MCAST */
    DRV_BLOCK_DLF_NONIP_MCAST = 0x0020 /* subtype in DLF_MCAST */
} drv_block_type_t;

/* additional constants for port status */
typedef enum drv_port_status_e {
    DRV_PORT_STATUS_LINK_UP = 1,
    DRV_PORT_STATUS_LINK_SPEED,
    DRV_PORT_STATUS_LINK_DUPLEX,
    DRV_PORT_STATUS_PROBE,
    DRV_PORT_STATUS_DETACH,
    DRV_PORT_STATUS_AUTONEG_ENABLE,
    DRV_PORT_STATUS_AUTONEG_DISABLED,
    DRV_PORT_STATUS_AUTONEG_COMPLETED,
    DRV_PORT_STATUS_AUTONEG_FAILED,
    DRV_PORT_STATUS_AUTONEG_PAGE_RECEIVED,
    DRV_PORT_STATUS_SRC_VLAN_VIOLATION,
    DRV_PORT_STATUS_DST_VLAN_VIOLATION,
    DRV_PORT_STATUS_ALL_QUEUE_DROP,
    DRV_PORT_STATUS_IN_BUF_FULL,
    DRV_PORT_STATUS_L2_TBL_UNAVAIL,
    DRV_PORT_STATUS_SECURITY_VIOLATION_DISABLED,
    DRV_PORT_STATUS_TX_PAUSE_DISABLED,
    DRV_PORT_STATUS_RX_PAUSE_DISABLED,
    DRV_PORT_STATUS_INIT,
    DRV_PORT_STATUS_PHY_DRV_NAME
} drv_port_status_t;

typedef enum drv_port_status_duplex_e {
    DRV_PORT_STATUS_DUPLEX_HALF = 1,
    DRV_PORT_STATUS_DUPLEX_FULL
} drv_port_status_duplex_t;

typedef enum drv_port_status_speed_e {
    DRV_PORT_STATUS_SPEED_10M = 1,
    DRV_PORT_STATUS_SPEED_100M,
    DRV_PORT_STATUS_SPEED_1G,
    DRV_PORT_STATUS_SPEED_2G,
    DRV_PORT_STATUS_SPEED_2500M,
    DRV_PORT_STATUS_SPEED_10G
} drv_port_status_speed_t;

/* port operation mode */
typedef enum drv_port_oper_mode_e {
    DRV_PORT_MODE_MAC_MASTER = 1,
    DRV_PORT_MODE_PHY_MASTER_RO,
    DRV_PORT_MODE_PHY_MASTER_RW,
    
    /* dot1p priority mapping operation :
     *  - PCP (dot1p pri)
     *  - TC (internal pri)
     */
    DRV_PORT_OP_PCP2TC,         /* Ingress PCP to TC */
    DRV_PORT_OP_NORMAL_TC2PCP,  /* TC to egress PCP, normal or inband case */
    DRV_PORT_OP_OUTBAND_TC2PCP,  /* TC to egress PCP, outband case */
    DRV_PORT_OP_NORMAL_TC2CPCP,  /* TC to egress C-PCP, normal or inband case */
    DRV_PORT_OP_OUTBAND_TC2CPCP  /* TC to egress C-PCP, outband case */
    
} drv_port_oper_mode_t;

/* port-base, 802.1x security */
typedef enum drv_security_state_e {
    DRV_SECURITY_PORT_UNCONTROLLED = 1,
    DRV_SECURITY_PORT_UNAUTHENTICATE,
    DRV_SECURITY_PORT_AUTHENTICATED
} drv_security_state_t;

typedef enum drv_security_flag_e {
    DRV_SECURITY_VIOLATION_NONE = 0x1,
    DRV_SECURITY_VIOLATION_STATIC_ACCEPT =0x2,
    DRV_SECURITY_VIOLATION_STATIC_REJECT = 0x4,
    DRV_SECURITY_VIOLATION_SA_NUM = 0x8,
    DRV_SECURITY_VIOLATION_SA_MATCH = 0x10,
    DRV_SECURITY_VIOLATION_SA_MOVEMENT = 0x20,
    DRV_SECURITY_BLOCK_IN = 0x40,
    DRV_SECURITY_BLOCK_INOUT = 0x80,
    DRV_SECURITY_LEARN = 0x100,
    DRV_SECURITY_IGNORE_LINK= 0x200,
    DRV_SECURITY_IGNORE_VIOLATION = 0x400,
    DRV_SECURITY_EAP_MODE_BASIC = 0x800,
    DRV_SECURITY_EAP_MODE_EXTEND = 0x1000,
    DRV_SECURITY_EAP_MODE_SIMPLIFIED = 0x2000,
    DRV_SECURITY_RX_EAP_DROP = 0x4000
} drv_security_flag_t;

#define MAC_SEC_FLAGS (DRV_SECURITY_VIOLATION_SA_MATCH | \
                       DRV_SECURITY_VIOLATION_STATIC_ACCEPT |\
                       DRV_SECURITY_VIOLATION_STATIC_REJECT )


/* switch to CPU type */
typedef enum drv_switch_type_e {
    /* Trap */
    DRV_SWITCH_TRAP_IPMC = 0x1,
    DRV_SWITCH_TRAP_IGMP = 0x2,
    DRV_SWITCH_TRAP_GARP = 0x4,
    DRV_SWITCH_TRAP_ARP = 0x8,
    DRV_SWITCH_TRAP_8023AD = 0x10,
    DRV_SWITCH_TRAP_ICMP = 0x20,
    DRV_SWITCH_TRAP_BPDU1 = 0x40,
    DRV_SWITCH_TRAP_BPDU2 = 0x80,
    DRV_SWITCH_TRAP_RARP = 0x100,
    DRV_SWITCH_TRAP_8023AD_DIS = 0x200,
    DRV_SWITCH_TRAP_BGMP = 0x400,
    DRV_SWITCH_TRAP_8021X = 0x800,
    DRV_SWITCH_TRAP_LLDP = 0x1000,
    DRV_SWITCH_TRAP_BCST = 0x2000,
    DRV_SWITCH_TRAP_ICMPV6 = 0x4000,
    DRV_SWITCH_TRAP_MLD = 0x8000,
    DRV_SWITCH_TRAP_IGMP_UNKNOW = 0x10000,
    DRV_SWITCH_TRAP_IGMP_QUERY = 0x20000,
    DRV_SWITCH_TRAP_IGMP_REPLEV = 0x40000,
    DRV_SWITCH_TRAP_MLD_QUERY = 0x80000,
    DRV_SWITCH_TRAP_MLD_REPDONE = 0x100000,
    
    /* disable IGMP pkt(all type) to CPU. */
    DRV_SWITCH_TRAP_IGMP_DISABLE = 0x200000, 
    
    /* disable MLD pkt(all type) to CPU */
    DRV_SWITCH_TRAP_MLD_DISABLE = 0x400000,
    
    /* disable ICMPV6 trap feature */
    DRV_SWITCH_TRAP_ICMPV6_DISABLE = 0x800000
} drv_switch_type_t;

typedef enum drv_snoop_mode_e {
    DRV_SNOOP_MODE_NONE = 0,
    DRV_SNOOP_MODE_CPU_TRAP,    /* to CPU only */
    DRV_SNOOP_MODE_CPU_SNOOP,   /* normal forward + copy to CPU */
    DRV_SNOOP_MODE_CNT
} drv_snoop_mode_t;

typedef enum drv_snoop_type_e {
    /* Snoop */
    DRV_SNOOP_IGMP = 0x1,
    DRV_SNOOP_ARP = 0x2,
    DRV_SNOOP_RARP = 0x4,
    DRV_SNOOP_ICMP = 0x8,
    DRV_SNOOP_ICMPV6 = 0x10,
    DRV_SNOOP_DHCP = 0x20,
    DRV_SNOOP_MLD = 0x40,
    DRV_SNOOP_IGMP_UNKNOW = 0x80,
    DRV_SNOOP_IGMP_QUERY = 0x100,
    DRV_SNOOP_IGMP_REPLEV = 0x200,
    DRV_SNOOP_MLD_QUERY = 0x400,
    DRV_SNOOP_MLD_REPDONE = 0x800,
    DRV_SNOOP_IGMP_DISABLE = 0x1000,  /* disable IGMP pkt(all type) to CPU */
    DRV_SNOOP_IGMP_UNKNOW_DISABLE = 0x2000, 
    DRV_SNOOP_IGMP_QUERY_DISABLE = 0x4000,
    DRV_SNOOP_IGMP_REPLEV_DISABLE = 0x8000,
    DRV_SNOOP_MLD_DISABLE = 0x10000, /* disable MLD pkt(all type) to CPU */
    DRV_SNOOP_MLD_QUERY_DISABLE = 0x20000,
    DRV_SNOOP_MLD_REPDONE_DISABLE = 0x40000,
    DRV_SNOOP_ICMPV6_DISABLE = 0x80000    /* disable ICMPv6 snooping */
} drv_snoop_type_t;

/* Storm Control type */
typedef enum drv_storm_control_type_e {
    DRV_STORM_CONTROL_BCAST = 0x1,
    DRV_STORM_CONTROL_MCAST = 0x2,
    DRV_STORM_CONTROL_DLF = 0x4,
    DRV_STORM_CONTROL_SALF = 0x8,
    DRV_STORM_CONTROL_RSV_MCAST = 0x10,
    DRV_STORM_CONTROL_UCAST = 0x20,
    DRV_STORM_CONTROL_VLAN_POLICING = 0x20,
    DRV_STORM_CONTROL_BUCKET_1 = 0x00010000,
    DRV_STORM_CONTROL_BUCKET_2 = 0x00020000
} drv_storm_control_type_t;

/* Storm Control rate definition */
typedef enum drv_storm_control_rate_e {
    DRV_STORM_RATE_ENABLE,    
    DRV_STORM_RATE_3POINT3_PERCENT,      
    DRV_STORM_RATE_5_PERCENT,      
    DRV_STORM_RATE_10_PERCENT,      
    DRV_STORM_RATE_20_PERCENT   
} drv_storm_control_rate_t;

/* Port Rate Control:Rate defintion */
typedef enum drv_rate_control_rate_e {
    DRV_RATE_CONTROL_INGRESS_EN,
    DRV_RATE_CONTROL_EGRESS_EN,
    DRV_RATE_CONTROL_FIXED,
    DRV_RATE_CONTROL_RATIO,
    DRV_RATE_CONTROL_INGRESS_RATE_LIMIT,
    DRV_RATE_CONTROL_INGRESS_RATE_LIMIT_ACTION
} drv_rate_control_rate_t;

/* Port Rate Control: action defintion */
typedef enum drv_rate_control_act_e {
    DRV_RATE_ACT_DROP,
    DRV_RATE_ACT_FC       /* FC is Flow Control */  
} drv_rate_control_act_t;

/* Port Rate Control: config defintion */
typedef enum drv_rate_control_config_e {
    DRV_RATE_CONFIG_RATE_TYPE,
    DRV_RATE_CONFIG_DROP_ENABLE,
    DRV_RATE_CONFIG_PKT_MASK,
    DRV_RATE_CONFIG_RATE_BAC,
    DRV_RATE_CONFIG_INGRESS_IPG_INCLUDE,
    DRV_RATE_CONFIG_INGRESS_SHAPER_PAUSE_OFF,
    DRV_RATE_CONFIG_INGRESS_SHAPER_PAUSE_ON
} drv_rate_control_config_t;

typedef enum drv_rate_control_type_e {
    DRV_RATE_CONFIG_RATE_TYPE_FIXED,
    DRV_RATE_CONFIG_RATE_TYPE_RATIO
} drv_rate_control_config_type_t;


/* Port Rate Control: config defintion */
typedef enum drv_rate_control_direction_e {
    DRV_RATE_CONTROL_DIRECTION_INGRESSS = 1,
    DRV_RATE_CONTROL_DIRECTION_EGRESSS,
    DRV_RATE_CONTROL_DIRECTION_EGRESSS_PER_QUEUE
} drv_rate_control_direction_t;

/* Port Rate Control: flags definition */
typedef enum drv_rate_control_flag_e {
    DRV_RATE_CONTROL_FLAG_EGRESS_IPG_INCLUDE = 0x1,
    DRV_RATE_CONTROL_FLAG_EGRESS_AVB_MODE = 0x2,
    DRV_RATE_CONTROL_FLAG_EGRESS_PACKET_BASED = 0x4
} drv_rate_control_flag_t;


/* mirror enable flags */
typedef enum drv_port_mrror_enable_e {
    DRV_PORT_MIRROR_INGRESS_ENABLE = 1,
    DRV_PORT_MIRROR_EGRESS_ENABLE
} drv_port_mirror_enable_t;

/* SNMP counters */
typedef enum drv_counter_thread_e {
    DRV_COUNTER_THREAD_START,
    DRV_COUNTER_THREAD_STOP,
    DRV_COUNTER_THREAD_SYNC
} drv_counter_thread_t;


typedef enum drv_mcast_index_e {
    DRV_MCAST_INDEX_ADD = 1,
    DRV_MCAST_INDEX_REMOVE
}drv_mcast_index_t;

/* Ethernet AV control type */
typedef enum drv_eav_control_flag_e {
    DRV_EAV_CONTROL_TIME_STAMP_TO_IMP = 1,
    DRV_EAV_CONTROL_MAX_AV_SIZE,
    DRV_EAV_CONTROL_STREAM_CLASSA_PCP,
    DRV_EAV_CONTROL_STREAM_CLASSB_PCP,
    DRV_EAV_CONTROL_MMU_INIT
} drv_eav_control_flag_t;

/* Ethernet AV Time Sync flag */
typedef enum drv_eav_time_sync_flag_e {
    DRV_EAV_TIME_SYNC_TIME_BASE = 0x1,
        /* p0 : time base value (ns) */
    DRV_EAV_TIME_SYNC_TIME_ADJUST,
        /* p0 : time increment value (ns), p1 : adjust priod (ticks) */
    DRV_EAV_TIME_SYNC_TICK_COUNTER, 
        /* p0 : tick counter in a slot time */
    DRV_EAV_TIME_SYNC_SLOT_NUMBER,
        /* p0 : slot number */
    DRV_EAV_TIME_SYNC_MACRO_SLOT_PERIOD,
        /* p0 : macro slot time (ms). */
    DRV_EAV_TIME_SYNC_SLOT_ADJUST
        /* p0 : slot adjustmenet (ticks), p1 : adjust period (slots) */
} drv_eav_time_sync_flag_t;

/* Ethernet AV queue control type */
typedef enum drv_eav_queuel_flag_e {
    DRV_EAV_QUEUE_Q4_BANDWIDTH = 0x1,
        /* parameter : bandwidth value (bytes/slot time) */
    DRV_EAV_QUEUE_Q5_BANDWIDTH,
        /* parameter : bandwidth value (bytes/slot time) */
    DRV_EAV_QUEUE_Q5_WINDOW,
        /* parameter : enable/disable */
    DRV_EAV_QUEUE_Q4_BANDWIDTH_MAX_VALUE,
        /* parameter : Maximum Q4 bandwidth value (bytes/slot time) */
    DRV_EAV_QUEUE_Q5_BANDWIDTH_MAX_VALUE
        /* parameter : Maximum Q5 bandwidth value (bytes/slot time) */
} drv_eav_queuel_flag_t;

/* Dos Attack type */
typedef enum drv_dos_type_e {
    DRV_DOS_NONE = 0x1,
    DRV_DOS_TCP_FRAG,
    DRV_DOS_SMURF,
    DRV_DOS_SYN_WITH_SP_LT1024,
    DRV_DOS_SYN_FIN_SCAN,
    DRV_DOS_XMASS_SCAN,
    DRV_DOS_XMASS_WITH_TCP_SEQ_ZERO,
    DRV_DOS_NULL_SCAN,
    DRV_DOS_NULL_WITH_TCP_SEQ_ZERO,
    DRV_DOS_BLAT,
    DRV_DOS_LAND,
    DRV_DOS_MIN_TCP_HDR_SZ,
    DRV_DOS_TCP_FRAG_OFFSET,
    DRV_DOS_TCP_BLAT,
    DRV_DOS_UDP_BLAT,
    DRV_DOS_ICMPV4_FRAGMENTS,
    DRV_DOS_ICMPV6_FRAGMENTS,
    DRV_DOS_MAX_ICMPV4_SIZE,
    DRV_DOS_MAX_ICMPV6_SIZE,
    DRV_DOS_ICMPV4_LONG_PING,
    DRV_DOS_ICMPV6_LONG_PING,
    DRV_DOS_TCP_SHORT_HDR,
    DRV_DOS_DISABLE_LEARN
} drv_dos_type_t;

/* Dos Attack event related OP type */
#define DRV_DOS_EVT_OP_ENABLED      1
#define DRV_DOS_EVT_OP_STATUS       2

/* Dos Attack event */
typedef enum drv_dos_event_e {
    DRV_DOS_EVT_ICMPV6_LONGPING = 0,
    DRV_DOS_EVT_ICMPV4_LONGPING,
    DRV_DOS_EVT_ICMPV6_FRAG,
    DRV_DOS_EVT_ICMPV4_FRAG,
    DRV_DOS_EVT_TCP_FRAG_ERR,
    DRV_DOS_EVT_TCP_SHORT_HDR,
    DRV_DOS_EVT_TCP_SYN_ERR,
    DRV_DOS_EVT_TCP_SINFIN_SCAN,
    DRV_DOS_EVT_TCP_XMAS_SCAN,
    DRV_DOS_EVT_TCP_NULL_SCAN,
    DRV_DOS_EVT_TCP_BLAT,
    DRV_DOS_EVT_UDP_BLAT,
    DRV_DOS_EVT_IP_LAND,
    DRV_DOS_EVT_MAC_LAND,
    DRV_DOS_EVT_ALL,
    DRV_DOS_EVT_CNT
} drv_dos_event_t;

/* Port Learn Mode */
typedef enum drv_port_learn_mode_e {
    DRV_PORT_HW_LEARN = 0x1,    /* SW learn + Forward */
    DRV_PORT_DISABLE_LEARN,     /* learning disabled + Forward */
    DRV_PORT_SW_LEARN,
    /* new ROBO CML definition */
    DRV_PORT_HW_SW_LEARN,
    DRV_PORT_DROP,          /* no (Learn|CPU|Forward) */
    DRV_PORT_SWLRN_DROP,         /* SW learn + Drop */
    DRV_PORT_HWLRN_DROP,         /* HW learn + Drop */
    DRV_PORT_SWHWLRN_DROP          /* SW&HW learn + Drop */
} drv_port_learn_mode_t;

/* LEARN COUNT related definition */
typedef enum drv_port_sa_learn_cnt_e {
    DRV_PORT_SA_LRN_CNT_NUMBER,     /* for get only */
    DRV_PORT_SA_LRN_CNT_INCREASE,   /* for set only */
    DRV_PORT_SA_LRN_CNT_DECREASE,   /* for set only */
    DRV_PORT_SA_LRN_CNT_RESET,      /* for set only, reset the lrn_cnt only */
    DRV_PORT_SA_LRN_CNT_LIMIT
} drv_port_sa_learn_cnt_t;

/* Multiport control type : behavior for multiport address */
typedef enum drv_multiport_control_e {
    DRV_MULTIPORT_CTRL_DISABLE = 0x0,
    DRV_MULTIPORT_CTRL_MATCH_ETYPE,
    DRV_MULTIPORT_CTRL_MATCH_ADDR,
    DRV_MULTIPORT_CTRL_MATCH_ETYPE_ADDR
} drv_multiport_control_t;

/* Multicast replication control type :
 *  - define the Mcast replication's group and vport control type together.
 */
typedef enum drv_mcrep_control_flag_e {
    DRV_MCREP_VPGRP_OP_VPORT_MEMBER = 0,/* OP is add/remove when set */
    DRV_MCREP_VPGRP_OP_VPGRP_RESET,     /* for set only, reset vp_bmp */
    DRV_MCREP_VPGRP_OP_ENTRY_ID,        /* for get only, retrieve entry_id */
    DRV_MCREP_VPORT_OP_VID,
    DRV_MCREP_VPORT_OP_VID_RESET,   /* for set only, reset VID in vp0-vp15 */
    DRV_MCREP_VPORT_OP_UNTAG_VP,    /* OP is set untag_vp */  
    DRV_MCREP_VPORT_OP_UNTAG_RESET, /* for set only, reset untag_vp */
    DRV_MCREP_VPORT_OP_VID_UNTAG_RESET, /* reset VID(vp0-vp15) and untag_vp */
    DRV_MCREP_OP_CNT
} drv_mcrep_control_flag_t;

typedef enum drv_mcrep_vid_search_result_e {
    DRV_MCREP_VID_SEARCH_FOUND,
    DRV_MCREP_VID_SEARCH_NOT_FOUND,
    DRV_MCREP_VID_SEARCH_FOUND_BUT_UNTAG
} drv_mcrep_vid_search_result_t;

typedef enum drv_igmp_mld_snoop_type_e {
    DRV_IGMP_MLD_TYPE_IGMP = 0, 
    DRV_IGMP_MLD_TYPE_MLD,     
    DRV_IGMP_MLD_TYPE_IGMP_MLD,     /* for both IGMP and MLD together */
    DRV_IGMP_MLD_TYPE_IGMP_REPLEV,    
    DRV_IGMP_MLD_TYPE_IGMP_QUERY,
    DRV_IGMP_MLD_TYPE_IGMP_UNKNOWN,   
    DRV_IGMP_MLD_TYPE_MLD_QUERY, 
    DRV_IGMP_MLD_TYPE_MLD_REPDONE,    
    DRV_IGMP_MLD_TYPE_CNT
} drv_igmp_mld_snoop_type_t;

#define DRV_IGMP_MLD_MODE_DISABLE       0x0 /* disable the snooping feature */
#define DRV_IGMP_MLD_MODE_ENABLE        0x1 /* enable the snooping feature.
                                               (for those chips can't indicate 
                                               snoop or trap mode) */
#define DRV_IGMP_MLD_MODE_SNOOP         0x2 /* for snooping mode */
#define DRV_IGMP_MLD_MODE_TRAP          0x3 /* for trap mode */



/* Weight Random Early Drop (WRED) */
typedef struct drv_wred_map_info_s {
    soc_port_t                      port; /* port number */
    int                             cosq;
    uint32                          flags;
    struct drv_wred_map_info_s     *next;
} drv_wred_map_info_t;


typedef struct drv_wred_config_s {
    uint32                  flags;
    int                     drop_prob; /* drop probability */
    int                     max_threshold;
    int                     min_threshold;
    int                     gain;/* Determines the smoothing that should be applied. */
    uint32                  hw_index; /* physical hardware index */
    int                     refer_count;
} drv_wred_config_t;

/* Reserved for default WRED configuration */
#define DRV_WRED_CONFIG_FLAGS_DEFAULT_PROFILE   (0x10) 


#define DRV_WRED_MAP_FLAGS_COLOR_ALL            (0x7)
#define DRV_WRED_MAP_FLAGS_COLOR_RED            (0x1)
#define DRV_WRED_MAP_FLAGS_COLOR_YELLOW         (0x2)
#define DRV_WRED_MAP_FLAGS_COLOR_GREEN          (0x4)
#define DRV_WRED_MAP_FLAGS_DEI                  (0x8) 
#define DRV_WRED_MAP_FLAGS_ALL                  (DRV_WRED_MAP_FLAGS_COLOR_ALL | \
                                            DRV_WRED_MAP_FLAGS_DEI)




/* WRED control type */
typedef enum drv_wred_control_e {
    DRV_WRED_CONTROL_ENABLE = 0x1,
    DRV_WRED_CONTROL_AQD_PERIOD,
    DRV_WRED_CONTROL_AQD_EXPONENT,
    DRV_WRED_CONTROL_AQD_FAST_CORRECTION, 
    /* The average queue depth never bigger than actual queue depth */
    DRV_WRED_CONTROL_DROP_BYPASS, /* Bypass the drop action at egress side */
    DRV_WRED_CONTROL_MAX_QUEUE_SIZE /* Get the maximum size of  the queue */
}drv_wred_control_t;

/* WRED counter type */
typedef enum drv_wred_counter_e {
    DRV_WRED_COUNTER_DROP_PACKETS = 0x1,
    DRV_WRED_COUNTER_DROP_BYTES
} drv_wred_counter_t;


/* GNATS 40307 : Unexpected override on Port discard configuration while the 
 *          link been re-established.
 *
 *  Note :
 *  1. Except TB chips, all ROBO chips use RX disable to implemente the 
 *     port discard related configuration.
 *  2. This definition is used to be a special MAC enable case to indicate 
 *     the MAC been enabled exclude RX (RX is disabled).
 */
#define DRV_SPECIAL_MAC_ENABLE_NORX     2 /* original MAC_ENABLE is 0 or 1 */

/* Definitions for filter bypass control items */
#define DRV_FILTER_BYPASS_NONE          _SHR_SWITCH_FILTER_NONE
#define DRV_FILTER_BYPASS_RX_CRCCHK     _SHR_SWITCH_FILTER_RX_CRCCHK
#define DRV_FILTER_BYPASS_DOS_TOCPU     _SHR_SWITCH_FILTER_DOS_TOCPU
#define DRV_FILTER_BYPASS_STP_USERADDR  _SHR_SWITCH_FILTER_STP_USERADDR
#define DRV_FILTER_BYPASS_STP_MAC0X     _SHR_SWITCH_FILTER_STP_MAC0X
#define DRV_FILTER_BYPASS_VLAN_IGMP_MLD _SHR_SWITCH_FILTER_VLAN_IGMP_MLD
#define DRV_FILTER_BYPASS_VLAN_ARP_DHCP _SHR_SWITCH_FILTER_VLAN_ARP_DHCP
#define DRV_FILTER_BYPASS_VLAN_MIIM     _SHR_SWITCH_FILTER_VLAN_MIIM
#define DRV_FILTER_BYPASS_VLAN_MCAST    _SHR_SWITCH_FILTER_VLAN_MCAST
#define DRV_FILTER_BYPASS_VLAN_RSV_MCAST _SHR_SWITCH_FILTER_VLAN_RSV_MCAST
#define DRV_FILTER_BYPASS_VLAN_USERADDR  _SHR_SWITCH_FILTER_VLAN_USERADDR
#define DRV_FILTER_BYPASS_EAP_USERADDR  _SHR_SWITCH_FILTER_EAP_USERADDR
#define DRV_FILTER_BYPASS_EAP_DHCP      _SHR_SWITCH_FILTER_EAP_DHCP
#define DRV_FILTER_BYPASS_EAP_ARP       _SHR_SWITCH_FILTER_EAP_ARP
#define DRV_FILTER_BYPASS_EAP_MAC_22_2F _SHR_SWITCH_FILTER_EAP_MAC_22_2F
#define DRV_FILTER_BYPASS_EAP_MAC_21    _SHR_SWITCH_FILTER_EAP_MAC_21
#define DRV_FILTER_BYPASS_EAP_MAC_20    _SHR_SWITCH_FILTER_EAP_MAC_20
#define DRV_FILTER_BYPASS_EAP_MAC_11_1F _SHR_SWITCH_FILTER_EAP_MAC_11_1F
#define DRV_FILTER_BYPASS_EAP_MAC_10    _SHR_SWITCH_FILTER_EAP_MAC_10
#define DRV_FILTER_BYPASS_EAP_MAC_0X    _SHR_SWITCH_FILTER_EAP_MAC_0X
#define DRV_FILTER_BYPASS_EAP_BPDU      _SHR_SWITCH_FILTER_EAP_BPDU

#define DRV_FILTER_BYPASS_TYPE_GET(_type, _id) \
        _SHR_SWITCH_FILTER_TYPE_GET(_type, _id)

#define DRV_TYPE_FROM_FILTER_BYPASS_TO_PROP(_type) \
        (((_type) == DRV_FILTER_BYPASS_RX_CRCCHK) ? \
            DRV_DEV_CTRL_RX_BYPASS_CRCCHK : \
        ((_type) == DRV_FILTER_BYPASS_DOS_TOCPU) ? \
            DRV_DEV_CTRL_DOS_BYPASS_TOCPU:\
        ((_type) == DRV_FILTER_BYPASS_STP_USERADDR) ? \
            DRV_DEV_CTRL_STP_BYPASS_USERADDR:\
        ((_type) == DRV_FILTER_BYPASS_STP_MAC0X) ? \
            DRV_DEV_CTRL_STP_BYPASS_MAC0X:\
        ((_type) == DRV_FILTER_BYPASS_EAP_USERADDR) ? \
            DRV_DEV_CTRL_EAP_BYPASS_USERADDR:\
        ((_type) == DRV_FILTER_BYPASS_EAP_DHCP) ? \
            DRV_DEV_CTRL_EAP_BYPASS_DHCP:\
        ((_type) == DRV_FILTER_BYPASS_EAP_ARP) ? \
            DRV_DEV_CTRL_EAP_BYPASS_ARP:\
        ((_type) == DRV_FILTER_BYPASS_EAP_MAC_22_2F) ? \
            DRV_DEV_CTRL_EAP_BYPASS_MAC_22_2F:\
        ((_type) == DRV_FILTER_BYPASS_EAP_MAC_21) ? \
            DRV_DEV_CTRL_EAP_BYPASS_MAC_21:\
        ((_type) == DRV_FILTER_BYPASS_EAP_MAC_20) ? \
            DRV_DEV_CTRL_EAP_BYPASS_MAC_20:\
        ((_type) == DRV_FILTER_BYPASS_EAP_MAC_11_1F) ? \
            DRV_DEV_CTRL_EAP_BYPASS_MAC_11_1F:\
        ((_type) == DRV_FILTER_BYPASS_EAP_MAC_10) ? \
            DRV_DEV_CTRL_EAP_BYPASS_MAC_10:\
        ((_type) == DRV_FILTER_BYPASS_EAP_MAC_0X) ? \
            DRV_DEV_CTRL_EAP_BYPASS_MAC_0X : \
            DRV_DEV_CTRL_EAP_BYPASS_MAC_BPDU)

/* LED Functions group */
#define DRV_LED_FUNCGRP_0    _SHR_SWITCH_LED_FUNCGRP_0
#define DRV_LED_FUNCGRP_1    _SHR_SWITCH_LED_FUNCGRP_1


/* Definitions for filter bypass control items */
#define DRV_LED_FUNC_NONE       _SHR_SWITCH_LED_FUNC_NONE 
#define DRV_LED_FUNC_PHYLED4    _SHR_SWITCH_LED_FUNC_PHYLED4
#define DRV_LED_FUNC_LNK        _SHR_SWITCH_LED_FUNC_LNK
#define DRV_LED_FUNC_DPX        _SHR_SWITCH_LED_FUNC_DPX
#define DRV_LED_FUNC_ACT        _SHR_SWITCH_LED_FUNC_ACT
#define DRV_LED_FUNC_COL        _SHR_SWITCH_LED_FUNC_COL
#define DRV_LED_FUNC_LINK_ACT   _SHR_SWITCH_LED_FUNC_LINK_ACT
#define DRV_LED_FUNC_DPX_COL    _SHR_SWITCH_LED_FUNC_DPX_COL
#define DRV_LED_FUNC_SP_10      _SHR_SWITCH_LED_FUNC_SP_10
#define DRV_LED_FUNC_SP_100     _SHR_SWITCH_LED_FUNC_SP_100
#define DRV_LED_FUNC_SP_1G      _SHR_SWITCH_LED_FUNC_SP_1G
#define DRV_LED_FUNC_10_ACT     _SHR_SWITCH_LED_FUNC_10_ACT
#define DRV_LED_FUNC_100_ACT    _SHR_SWITCH_LED_FUNC_100_ACT
#define DRV_LED_FUNC_10_100_ACT _SHR_SWITCH_LED_FUNC_10_100_ACT
#define DRV_LED_FUNC_1G_ACT     _SHR_SWITCH_LED_FUNC_1G_ACT
#define DRV_LED_FUNC_EAV_LINK   _SHR_SWITCH_LED_FUNC_EAV_LINK
#define DRV_LED_FUNC_PHYLED3    _SHR_SWITCH_LED_FUNC_PHYLED3

#define DRV_LED_FUNC_SP_100_200     _SHR_SWITCH_LED_FUNC_SP_100_200
#define DRV_LED_FUNC_100_200_ACT    _SHR_SWITCH_LED_FUNC_100_200_ACT
#define DRV_LED_FUNC_LNK_ACT_SP     _SHR_SWITCH_LED_FUNC_LNK_ACT_SP

#define DRV_LED_FUNC_ALL_MASK   \
    (DRV_LED_FUNC_PHYLED4 | DRV_LED_FUNC_LNK | DRV_LED_FUNC_DPX | \
    DRV_LED_FUNC_ACT | DRV_LED_FUNC_COL| DRV_LED_FUNC_LINK_ACT | \
    DRV_LED_FUNC_DPX_COL| DRV_LED_FUNC_SP_10 | DRV_LED_FUNC_SP_100 | \
    DRV_LED_FUNC_SP_1G | DRV_LED_FUNC_10_ACT | DRV_LED_FUNC_100_ACT | \
    DRV_LED_FUNC_10_100_ACT | DRV_LED_FUNC_1G_ACT | DRV_LED_FUNC_EAV_LINK | \
    DRV_LED_FUNC_PHYLED3 | DRV_LED_FUNC_SP_100_200 | \
    DRV_LED_FUNC_100_200_ACT | DRV_LED_FUNC_LNK_ACT_SP)

/* LED mode */
#define DRV_LED_MODE_OFF        _SHR_SWITCH_LED_MODE_OFF  
#define DRV_LED_MODE_ON         _SHR_SWITCH_LED_MODE_ON  
#define DRV_LED_MODE_BLINK      _SHR_SWITCH_LED_MODE_BLINK
#define DRV_LED_MODE_AUTO       _SHR_SWITCH_LED_MODE_AUTO

extern int _drv_arl_database_insert(int unit, int index, void *entry);
extern int _drv_arl_database_delete_by_fastage(int unit, uint32 src_port, uint32 vlanid, uint32 flags);
extern int _drv_arl_database_delete(int unit, int index, void *entry);
#endif /* _DRVTYPES_H */

