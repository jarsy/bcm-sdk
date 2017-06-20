/*
 * $Id: mbCommon.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * == mbCommon.h - Common MetroBox Definitions  ==
 */

#ifndef __MBCOMMON_H__
#define __MBCOMMON_H__

/*
** System Scaling Constants
**
** All of the system-wide scaling constants (MAX, MIN, etc) go here
*/
#define MB_LINE_CARDS_MAX_K      (6)
#define MB_LM_TEMP_SENSOR_MAX_K  (3)
#define MB_MAX_TEMP_SENSOR_MAX_K (5)
#define MB_FAN_CONTROLLER_MAX_K  (5)
#define MB_POWER_SUPPLY_MAX_K    (2)

/*
** The socket port numbers used - According to the IANA, these numbers
** are private and unassigned
*/
#define MB_STATUS_MGR_SERVER_PORT_K        (60004)
#define MB_STATUS_MGR_CMD_PORT_K           (60005)
#define MB_STATUS_MGR_SLAVE_PORT_BASE_K    (60100)
#define MB_FERM_SLAVE_PORT_K               (60200)
#define MB_FERM_MASTER_PORT_BASE_K         (60201)
#define MB_DIAG_SERVER_PORT_K              (60300)
#define MB_OPTO_SERVER_PORT_K              (60350)
#define MB_CHMD_MASTER_SERVER_PORT_K       (60400)
#define MB_HWINIT_PORT_K                   (60500)
#define MB_FABD_SERVER_PORT_K              (60600)

/*
** Board-to-board communication IP address assignments
*/
#define MB_B2B_BRDLCL_IPADDR_K ("127.0.0.1")
#define MB_B2B_BRD1_IPADDR_K   ("10.0.0.1")
#define MB_B2B_BRD2_IPADDR_K   ("10.0.0.2")
#define MB_B2B_BRD3_IPADDR_K   ("10.0.0.3")
#define MB_B2B_BRD4_IPADDR_K   ("10.0.0.4")
#define MB_B2B_BRD5_IPADDR_K   ("10.0.0.5")
#define MB_B2B_BRD6_IPADDR_K   ("10.0.0.6")
#define MB_B2B_IPADDR_UNKNOWN_K (" ")

#define MB_B2B_MAX_SLOTS_K     (6)

/*
** Packet size settings used throughout system
*/
#define MB_SHIMHDR_SZ_K            (14)
#define MB_MIN_FRM_SZ_K            (64)
#define MB_MAX_FRM_SZ_NRML_K     (1536)
#define MB_DIX_HDR_SZ_K            (14)
#define MB_VLAN_TAG_SZ_K            (4)
#define MB_MAX_FRM_SZ_JMBO_K     (9216)
#define MB_CRC_SZ_K                 (4)

/*
** IF Device IDs
*/
#define MB_DEVID_MAC_IXF18201_K   (0x210) /* 2x10G MAC */
#define MB_DEVID_MAC_IXF1110_K    (0x101) /* 10x1G MAC */
#define MB_DEVID_MAC_IXF1024_K    (0x241) /* 24x1G MAC */

/*
** Board Number Indication
*/
typedef enum mb_brdnum_e {

    MB_BRDNUM_UNKNOWN_K = -1,
    MB_BRDNUM_1_K       =  0,
    MB_BRDNUM_2_K       =  1,
    MB_BRDNUM_3_K       =  2,
    MB_BRDNUM_4_K       =  3,
    MB_BRDNUM_5_K       =  4,
    MB_BRDNUM_6_K       =  5,
    MB_BRDNUM_7_K       =  6,
    MB_BRDNUM_8_K       =  7,
    MB_BRDNUM_9_K       =  8,
    MB_BRDNUM_10_K      =  9
} mb_brdnum_t;

/*
** Secondary Board Presence Indication
*/
typedef enum mb_brd2pres_e {
    MB_BRD2_UNKNOWN_K = -1,
    MB_BRD2_PRESENT_K =  1,
    MB_BRD2_ABSENT_K  =  0

} mb_brd2pres_t;

/*
** Board port types (from optod boardinfo id field)
*/
typedef enum mb_brd_ports_e {
    MB_BRD_PORT_UNKNOWN_K = -1,
    MB_BRD_PORT_20x1G  = 0,
    MB_BRD_PORT_2x10G  = 1,
    MB_BRD_PORT_10p2v1 = 2,  /* First rev use 1 port only */
    MB_BRD_PORT_10p2v2 = 3,  /* Second rev, new 10 gig mac */
    MB_BRD_PORT_24x1G  = 4,  /* Acction ES4824 */
    MB_BRD_PORT_24p2   = 5,
    MB_BRD_PORT_10x1G  = 6   /* BenNevis Eval board on Ref Chassis */
} mb_brd_ports_t;

/*
** Metrobox form factor board type
*/
typedef enum mb_brd_type_e {
    MB_BOARD_TYPE_UNKNOWN = -1,
    MB_BOARD_TYPE_METROBOX = 0,
    MB_BOARD_TYPE_KAREF = 1,
    MB_BOARD_TYPE_NEO2 = 2,
    MB_BOARD_TYPE_ES4824 = 3,
    MB_BOARD_TYPE_ES4826 = 4,
    MB_BOARD_TYPE_ES4824_SFP = 5,
    MB_BOARD_TYPE_ES4826_SFP = 6,
    MB_BOARD_TYPE_RC_FC_MA = 7,
    MB_BOARD_TYPE_RC_FC_PT = 8,
    MB_BOARD_TYPE_RC_LC = 9
} mb_brd_type_t;

/*
** KaRef ofem board type (SW)
*/
typedef enum ka_ofem_brd_type_e {
    KA_OFEM_BOARD_TYPE_UNKNOWN = -1,
    KA_OFEM_BOARD_TYPE_10x2_V1 = 0,
    KA_OFEM_BOARD_TYPE_10x2_V2 = 1
} ka_ofem_brd_type_t;

/*
** KaRef ofem hardware board type (HW)
*/
typedef enum ka_ofem_hw_brd_type_e {
    KA_OFEM_HW_BOARD_TYPE_10x2_V1 = 1,
    KA_OFEM_HW_BOARD_TYPE_10x2_V2 = 2
} ka_ofem_hw_brd_type_t;

/*
** Ka Reference Design Optical Front End Module board type
*/
typedef enum mb_ofem_brd_type_e {
    MB_OFEM_BOARD_TYPE_UNKNOWN = -1,
    MB_OFEM_BOARD_TYPE_10p2v1 = 0,  /* First rev use 1 port only */
    MB_OFEM_BOARD_TYPE_10p2v2 = 1   /* Second rev, new 10 gig mac */
} mb_ofem_brd_type_t;


/*
 * Board revision (from optod boardinfo rev field)
 */
typedef enum mb_brd_rev_e {
    MB_BRD_REV_UNKNOWN_K = -1,
    MB_BRD_REV_1_K = 1,
    MB_BRD_REV_2_K

} mb_brd_rev_t;

/*
 * Switch type
 */
typedef enum mb_switch_type_e {
    MB_SWITCH_TYPE_UNKNOWN_K = -1,
    MB_SWITCH_TYPE_SWAP_K = 1,
    MB_SWITCH_TYPE_NO_SWAP_K = 0

} mb_switch_type_t;

/*
** Message levels
*/
typedef enum mb_msg_e {
    MB_MSG_INFO
} mb_msg_t;

/*
** Device Strings for accessing /dev files
*/

/* Thin access for LocalBus devices */
#define MB_OPTO_STR_K       "/dev/sb/opto"
#define MB_BME_STR_K        "/dev/sb/bme"
#define MB_MAC_STR_K        "/dev/sb/mac"   /* Append with device number */
#define MB_NVRAM_STR_K      "/dev/sb/nvram"

#define MB_SCAN_STR_K       "/dev/sb/scan"  /* On KA REF board */
#define MB_OFEM_STR_K       "/dev/sb/ofem"  /* On KA REF board */

#define MB_ES48XX_STR_K     "/dev/sb/es48xx" /* Accton ES4824 & ES4826 */


/* Thin access for PCI devices */
#define MB_IFE_STR_K        "/dev/sb/ife"   /* Append with device number */
#define MB_EFE_STR_K        "/dev/sb/efe"   /* Append with device number */
#ifndef QENIC
#define MB_QE_STR_K         "/dev/sb/qe"    /* Append with device number */
#define MB_KA_STR_K         "/dev/sb/qe"    /* Append with device number */
#else
#define MB_QE_STR_K         "sbqe"    /* Append with device number */
#define MB_KA_STR_K         "sbqe"    /* Append with device number */
#endif

#define MB_PLX_STR_K        "/dev/sb/plx"

/* Thin access for Local Bus Sub-Devices */
#define MB_OFFSET_BB_CPLD (0x00800000)
#define MB_OFFSET_DB_CPLD (0x00200000)
#define MB_OFFSET_FC_ARB  (0x00206000)
#define MB_OFFSET_FC_XB0  (0x00208000)
#define MB_OFFSET_FC_LCM  (0x00208000)
#define MB_OFFSET_FC_BME  (0x00204000)
#define MB_OFFSET_FC_XB1  (0x0020A000)

#endif
