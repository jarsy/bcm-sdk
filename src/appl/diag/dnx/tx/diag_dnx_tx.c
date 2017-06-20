/** \file diag_dnx_tx.c
 *
 * Main diagnostics for tx applications
 *
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifdef BSL_LOG_MODULE
#  error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SOCDNX_TX

/*************
 * INCLUDES  *
 *************/
#include <shared/bsl.h>
#include <sal/appl/sal.h>
#include <shared/bslnames.h>
#include <soc/mcm/memregs.h>
#include <soc/dpp/SAND/Management/sand_low_level.h>
#include <bcm_int/dpp/error.h>
#include <appl/diag/system.h>
#include <appl/diag/diag.h>

/* DNX TX Includes */
#include <appl/diag/shell.h>
#include <appl/diag/parse.h>
#include <appl/diag/dport.h>
#include <soc/cm.h>
#include "diag_dnx_tx.h"

/*************
 * TYPEDEFS  *
 *************/

/**
 * diagnostic function definition
 */
typedef cmd_result_t(
    *CMD_DIAG_FUNC) (
    int unit,
    char *params[CMD_MAX_NOF_INPUTS]);

/**
 * diagnostic command definition structure
 */
typedef struct
{
    /**
     * command full
     */
    char comand_name[CMD_MAX_STRING_LENGTH];

    /**
     * command short
     */
    char comand_name_short[CMD_MAX_STRING_LENGTH / 4];

    /**
     * number of mandatory input parameters for command
     */
    int num_of_mandatory_params;

    /**
     * number of optional input parameters for command
     */
    int num_of_optional_params;

    /**
     * command function
     */
    CMD_DIAG_FUNC diag_func;

} cmd_string_func_t;

/**
 * unitest procedures structures
 */
typedef struct
{
    /**
     * unitest function
     */
    CMD_DIAG_FUNC diag_func;

    /**
     * unitest function's inputs
     */
    char *inputs[CMD_MAX_NOF_INPUTS];

    /**
     * unitest function's result
     */
    uint8 is_passed;

    /**
     * unitest description
     */
    char *test_description;

} regression_test_list_t;

/* Struct for TX/RX parameters */
typedef struct xd_s
{
    int xd_unit;                /* Unit # */
    int xd_tx_unit;             /* TX device */
    int xd_ppsm;                /* true --> per port src macs */
    enum
    {
        XD_IDLE,                /* Not doing anything */
        XD_RUNNING,             /* Running */
        XD_STOP                 /* Stop requested */
    } xd_state;
    uint32 xd_tot_cnt;          /* # to send/receive */
    uint32 xd_cur_cnt;          /* # sent/received */
    int xd_pkt_len;             /* Packet length */
    char *xd_file;              /* File name of packet data */
    sal_mac_addr_t xd_mac_dst;  /* Destination mac address */
    sal_mac_addr_t xd_mac_src;  /* Source mac address */
    sal_mac_addr_t xd_mac_src_base;     /* Source mac address for pps */
    /*
     * port source mac 
     */
    uint32 xd_mac_dst_inc;      /* Destination mac increment */
    uint32 xd_mac_src_inc;      /* Source mac increment */
    uint32 xd_pat;              /* XMIT pattern */
    uint32 xd_pat_inc;          /* Pattern Increment */
    int xd_pat_random;          /* Use Random Pattern */
    pbmp_t xd_ppsm_pbm;         /* Saved port bit map for per */
    /*
     * port source mac 
     */
    uint32 xd_vlan;             /* vlan id (0 if untagged) */
    uint32 xd_prio;             /* vlan prio */
    uint32 xd_prio_int;         /* Internal priority */
    uint32 xd_crc;

    /*
     * Packet info is now in bcm_pkt_t structure 
     */
    bcm_pkt_t pkt_info;

    /*
     * XGS Header Fields.  Most are in pkt_info 
     */
    int hdr_mode;               /* Mode type */
    int ether_pkt;              /* Use Ethernet packet */
    uint32 w0, w1, w2, w3;      /* Raw mode words */
    uint32 xd_hg_dmod;          /* Higig Dest Mod */
    uint32 xd_hg_dport;         /* Higig Dest Port */
    uint32 xd_hg_smod;          /* Higig Src Mod */
    uint32 xd_hg_sport;         /* Higig Src Port */
    uint32 xd_hg_opcode;        /* Higig Opcode */
    uint32 xd_hg_cos;           /* Higig Class of Service */
    uint32 xd_hg_pfm;           /* Higig Port Filtering Mode */
#ifdef BCM_HIGIG2_SUPPORT
    uint32 xd_hg_mgid;          /* Higig2 Multicast Group ID */
    uint32 xd_hg_lbid;          /* Higig2 Load Balancing ID */
    uint32 xd_hg_dp;            /* Higig2 Drop Precendece */
#endif                          /* BCM_HIGIG2_SUPPORT */
    uint32 xd_s_portid;         /* BCM5632 Source port ID */
    uint32 xd_d_portid;         /* BCM5632 Destination port ID */
    uint32 xd_length;           /* BCM5632 Length Field */
    uint32 xd_start;            /* BCM5632 Start Field */
    int xd_untag;               /* FB/ER Send Untagged */
    int xd_purge;               /* FB/ER Send Purge */
    char *xd_data;              /* packet data */
    uint16 xd_ptch_src_port;    /* PTCH src port */
    int itmh_pph_type;
    int itmh_in_mirr_flag;
    uint32 itmh_isq_flow;
    uint32 itmh_out_lif;
    uint32 itmh_multicast_flow;
    uint32 itmh_dest_ext;
    int xd_itmh_present /* indicate whether itmh header is present or not */ ;
    int itmh_version;
    int itmh_pph_present;
    int itmh_out_mirr_dis;
    int itmh_in_mirr_dis;
    int itmh_exclude_src;
    uint32 itmh_snoop_cmd;
    uint32 itmh_traffic_class;
    uint32 itmh_dp;
    uint32 itmh_fwd_type;
    uint32 itmh_dest_sys_port;
    uint32 itmh_flow_id;
    uint32 itmh_fec_ptr;
    uint32 itmh_multicast_id;
    int itmh_src_sys_port_flag;
    uint32 itmh_src_sys_port;
    uint32 itmh_src_local_port;
} xd_t;

static xd_t *xd_units[SOC_MAX_NUM_DEVICES];
static void diag_dnx_xd_init(
    int unit);

#define ENCAP_IEEE      0

#define XD_FILE(xd)     ((xd)->xd_file != NULL && (xd)->xd_file[0] != 0)
#define XD_STRING(xd)     ((xd)->xd_data != NULL && (xd)->xd_data[0] != 0)

#define TAGGED_PACKET_LENGTH           68
#define UNTAGGED_PACKET_LENGTH         64

#define _XD_INIT(unit, xd) do { \
        if (xd_units[unit] == NULL) \
            diag_dnx_xd_init(unit); \
        if (xd_units[unit] == NULL) \
            return CMD_FAIL; \
        xd = xd_units[unit]; } while (0)

char cmd_dnx_tx_usage[] = "Parameters: <Count> [options]\n"
#ifndef COMPILER_STRING_CONST_LIMIT
    "  Transmit the specified number of packets, if the contents of the\n"
    "  packets is important, they may be specified using options.\n"
    "  Supported options are:\n"
    "      Untagged[yes/no]           - Specify packet should be sent untagged(XGS3)\n"
    "      TXUnit=<val>               - Transmit unit number\n"
    "      PortBitMap=<pbmp>          - Specify port bitmap packet is sent to.\n"
    "      UntagBitMap=<pbmp>         - Specify untag bitmap used for DMA.\n"
    "      File=<filename>            - Load hex packet data from file and ignore\n"
    "                                   various other pattern parameters below.\n"
    "      DATA=<value>               - Packet value (Hex).\n"
    "      Length=<value>             - Specify the total length of the packet,\n"
    "                                   including header, possible tag, and CRC.\n"
    "      VLantag=<value>            - Specify the VLAN tag used, only the low\n"
    "                                   order 16-bits are used (VLANID=0 for none)\n"
    "      VlanPrio                   - VLAN Priority.\n"
    "      PrioInt                    - Internal Priority.\n"
    "      Pattern=<value>            - Specify 32-bit data pattern used.\n"
    "      PatternInc=<value>         - Value by which each word of the data\n"
    "                                   pattern is incremented\n"
    "      PatternRandom=[0|1]        - Use Random data pattern\n"
    "      PerPortSrcMac=[0|1]        - Associate specific (different) src macs\n"
    "                                   with each source port.\n"
    "      SourceMac=<value>          - Source MAC address in packet\n"
    "      SourceMacInc=<val>         - Source MAC increment\n"
    "      DestMac=<value>            - Destination MAC address in packet.\n"
    "      DestMacInc=<value>         - Destination MAC increment.\n"
    "      PtchSRCport=<value>        - PTCH source port (For ARAD only).\n"
    "\n"
    "  Ingress TM Header Format-specific options:\n"
    "      Itmh_present=[0|1]         - ITMH header is present\n"
    "      Pph_type=<value>           - Packet processing header is present\n"
    "      In_mirr_flag=[0|1]         - Disable out-bound port mirroring\n"
    "      Snoop_cmd=<value>          - One of 15 snoop cmds \n"
    "      Traffic_class=<value>      - System level TM traffic class\n"
    "      Drop_precedence=<value>    - Drop precedence for WRED\n"
    "      Forward_type=<value>       - Forwarding destination info type \n"
    "      Dest_sys_port=<value>      - System level Destination system port \n"
    "      Flow_id=<value>            - Unicast flow id \n"
    "      Fec_ptr=<value>            - Fec pointer id \n"
    "      Multicast_id=<value>       - Multicast ID identifying group \n"
    "      Isq_flow=<value>           - Ingress shaping flow ID \n"
    "      Out_lif=<value>            - Out LIF for unicast packet \n"
    "      Multicast_flow=<value>     - Multicast flow ID \n"
    "      Dest_ext=<value>           - Destination information extention \n"
#endif
    ;

/*************
* FUNCTIONS *
*************/

/** ****************************************************
* \brief
* Parse TX command from BCM shell
*****************************************************/
static cmd_result_t
diag_dnx_tx_parse(
    int u,
    args_t * a,
    xd_t * xd)
{
    char *crc_list[] = { "None", "Recompute", "Append", NULL };
    parse_table_t pt;
    int min_len, tagged;
    char *xfile, *xdata;
    uint32 ptch_src_port = 0;
    bcm_port_config_t port_config;
    uint32 port_ndx;

    /*
     * First arg is count 
     */
    if (!ARG_CNT(a) || !isint(ARG_CUR(a)))
    {
        return (CMD_USAGE);
    }

    /*
     * initialize ptch_src_port 
     */
    
    sal_memset(&port_config, 0, sizeof(port_config));
    port_config.cpu = PBMP_CMIC(u);

    

    BCM_PBMP_ITER(port_config.cpu, port_ndx)
    {
        /*
         * get first CPU port 
         */
        ptch_src_port = port_ndx;
        break;
    }

    _XD_INIT(u, xd);
    xd->xd_tot_cnt = parse_integer(ARG_GET(a));

    parse_table_init(u, &pt);

    parse_table_add(&pt, "Untagged", PQ_DFL | PQ_BOOL, 0, &xd->xd_untag, NULL);
    parse_table_add(&pt, "EthernetPacket", PQ_DFL | PQ_BOOL, 0, &xd->ether_pkt, NULL);

    /*
     * parse ITMH header contents 
     */
    parse_table_add(&pt, "Itmh_Present", PQ_DFL | PQ_BOOL, 0, &xd->xd_itmh_present, NULL);
    parse_table_add(&pt, "Pph_Type", PQ_DFL | PQ_INT, 0, &xd->itmh_pph_type, NULL);
    parse_table_add(&pt, "In_Mirr_Flag", PQ_DFL | PQ_BOOL, 0, &xd->itmh_in_mirr_flag, NULL);
    parse_table_add(&pt, "Isq_Flow", PQ_DFL | PQ_INT, 0, &xd->itmh_isq_flow, NULL);
    parse_table_add(&pt, "Out_Lif", PQ_DFL | PQ_INT, 0, &xd->itmh_out_lif, NULL);
    parse_table_add(&pt, "Multicast_Flow", PQ_DFL | PQ_INT, 0, &xd->itmh_multicast_flow, NULL);
    parse_table_add(&pt, "Dest_Ext", PQ_DFL | PQ_INT, 0, &xd->itmh_dest_ext, NULL);
    parse_table_add(&pt, "Snoop_cmd", PQ_DFL | PQ_INT, 0, &xd->itmh_snoop_cmd, NULL);
    parse_table_add(&pt, "Traffic_class", PQ_DFL | PQ_INT, 0, &xd->itmh_traffic_class, NULL);
    parse_table_add(&pt, "Drop_precedence", PQ_DFL | PQ_INT, 0, &xd->itmh_dp, NULL);
    parse_table_add(&pt, "Forward_type", PQ_DFL | PQ_INT, 0, &xd->itmh_fwd_type, NULL);
    parse_table_add(&pt, "Dest_sys_port", PQ_DFL | PQ_INT, 0, &xd->itmh_dest_sys_port, NULL);
    parse_table_add(&pt, "Flow_id", PQ_DFL | PQ_INT, 0, &xd->itmh_flow_id, NULL);
    parse_table_add(&pt, "Fec_ptr", PQ_DFL | PQ_INT, 0, &xd->itmh_fec_ptr, NULL);
    parse_table_add(&pt, "Multicast_id", PQ_DFL | PQ_INT, 0, &xd->itmh_multicast_id, NULL);
    parse_table_add(&pt, "Source_sys_port_flag", PQ_DFL | PQ_BOOL, 0, &xd->itmh_src_sys_port_flag, NULL);
    parse_table_add(&pt, "Source_sys_port", PQ_DFL | PQ_INT, 0, &xd->itmh_src_sys_port, NULL);
    parse_table_add(&pt, "Source_local_port", PQ_DFL | PQ_INT, 0, &xd->itmh_src_local_port, NULL);

    parse_table_add(&pt, "TXUnit", PQ_DFL | PQ_INT, 0, &xd->xd_tx_unit, NULL);
    parse_table_add(&pt, "PortBitMap", PQ_DFL | PQ_PBMP | PQ_BCM, 0, &xd->pkt_info.tx_pbmp, NULL);

    if (!SOC_IS_XGS_FABRIC(u))
    {
        parse_table_add(&pt, "UntagBitMap", PQ_DFL | PQ_PBMP | PQ_BCM, 0, &xd->pkt_info.tx_upbmp, NULL);
    }

    if (!SOC_IS_XGS3_SWITCH(u))
    {
        if (SOC_IS_XGS_SWITCH(u))
        {
            parse_table_add(&pt, "L3BitMap", PQ_DFL | PQ_PBMP | PQ_BCM, 0, &xd->pkt_info.tx_l3pbmp, NULL);
        }
    }

    parse_table_add(&pt, "Filename", PQ_DFL | PQ_STRING, 0, &xd->xd_file, NULL);
    parse_table_add(&pt, "Length", PQ_DFL | PQ_INT, 0, &xd->xd_pkt_len, NULL);
    parse_table_add(&pt, "VLantag", PQ_DFL | PQ_HEX, 0, &xd->xd_vlan, NULL);
    parse_table_add(&pt, "VlanPrio", PQ_DFL | PQ_INT, 0, &xd->xd_prio, NULL);
    parse_table_add(&pt, "PrioInt", PQ_DFL | PQ_INT, 0, &xd->xd_prio_int, NULL);
    parse_table_add(&pt, "Pattern", PQ_DFL | PQ_HEX, 0, &xd->xd_pat, NULL);
    parse_table_add(&pt, "PatternInc", PQ_DFL | PQ_INT, 0, &xd->xd_pat_inc, NULL);
    parse_table_add(&pt, "PatternRandom", PQ_DFL | PQ_BOOL, 0, &xd->xd_pat_random, NULL);
    parse_table_add(&pt, "PerPortSrcMac", PQ_DFL | PQ_INT, 0, &xd->xd_ppsm, NULL);
    parse_table_add(&pt, "SourceMac", PQ_DFL | PQ_MAC, 0, &xd->xd_mac_src, NULL);
    parse_table_add(&pt, "SourceMacInc", PQ_DFL | PQ_INT, 0, &xd->xd_mac_src_inc, NULL);
    parse_table_add(&pt, "DestMac", PQ_DFL | PQ_MAC, 0, &xd->xd_mac_dst, NULL);
    parse_table_add(&pt, "DestMacInc", PQ_DFL | PQ_INT, 0, &xd->xd_mac_dst_inc, NULL);
    parse_table_add(&pt, "COSqueue", PQ_DFL | PQ_INT, 0, &xd->xd_hg_cos, NULL);
    parse_table_add(&pt, "CRC", PQ_DFL | PQ_MULTI, 0, &xd->xd_crc, crc_list);
    parse_table_add(&pt, "PtchSRCport", PQ_DFL | PQ_INT, 0, &ptch_src_port, NULL);
    parse_table_add(&pt, "DATA", PQ_DFL | PQ_STRING, 0, &xd->xd_data, NULL);

    /*
     * Parse remaining arguments 
     */
    if (0 > parse_arg_eq(a, &pt))
    {
        cli_out("%s: Error: Invalid option or malformed expression: %s\n", ARG_CMD(a), ARG_CUR(a));
        parse_arg_eq_done(&pt);
        return (CMD_FAIL);
    }
    if (xd->xd_file)
    {
        xfile = sal_strdup(xd->xd_file);
    }
    else
    {
        xfile = NULL;
    }
    if (xd->xd_data)
    {
        xdata = sal_strdup(xd->xd_data);
    }
    else
    {
        xdata = NULL;
    }

    parse_arg_eq_done(&pt);
    xd->xd_file = xfile;
    xd->xd_data = xdata;
    xd->xd_ptch_src_port = (uint16) ptch_src_port;

    tagged = (xd->xd_vlan != 0);
    min_len = (tagged ? TAGGED_PACKET_LENGTH : UNTAGGED_PACKET_LENGTH);

    if (!XD_FILE(xd) && !XD_STRING(xd) && !tagged)
    {
        cli_out("%s: Warning: Sending untagged packets from CPU " "not recommended\n", ARG_CMD(a));
    }

    if (!XD_FILE(xd) && !XD_STRING(xd) && xd->xd_pkt_len < min_len)
    {
        cli_out("%s: Warning: Length %d too small for %s packet (min %d)\n",
                ARG_CMD(a), xd->xd_pkt_len, tagged ? "tagged" : "untagged", min_len);
    }

    if (xd->xd_hg_cos > NUM_COS(xd->xd_unit) - 1 &&
        xd->xd_hg_cos != 14 && xd->xd_hg_cos != 15 && !SOC_IS_TR_VL(xd->xd_unit))
    {
        cli_out("%s: Error: Invalid COS queue %d, must be < %d\n", ARG_CMD(a), xd->xd_hg_cos, NUM_COS(xd->xd_unit));
        return (CMD_FAIL);
    }

    if (xd->xd_untag)
    {
        xd->xd_vlan = 0;
        xd->xd_prio = 0;
    }

    return CMD_OK;
}

/**
 * \brief
 *   Initialize the TX/RX parameters struct
 *
 *  \par DIRECT INPUT:
 *    \param [in] unit -
 *      Unit number.
 *  \par DIRECT OUTPUT:
 *    None
 *  \remark
 *    None
 *  \see
 *    None
 *****************************************************/
static void
diag_dnx_xd_init(
    int unit)
{
    xd_t *xd;
    
    sal_mac_addr_t default_mac_src = { 0x00, 0x00, 0x00, 0x07, 0x00, 0x00 };
    sal_mac_addr_t default_mac_dst = { 0x00, 0x00, 0x00, 0xe3, 0x00, 0x00 };
    bcm_port_config_t pcfg;

    sal_memset(&pcfg, 0, sizeof(pcfg));
    pcfg.all = PBMP_ALL(unit);

    

    /*
     * Allocate memory for the xd struct 
     */
    xd = xd_units[unit];
    if (xd == NULL)
    {
        xd = sal_alloc(sizeof(xd_t), "xd");
        if (xd == NULL)
        {
            cli_out("WARNING: xd memory allocation failed\n");
            return;
        }
        sal_memset(xd, 0, sizeof(xd_t));
        xd_units[unit] = xd;
    }

    /*
     * get the encapsulation of the first higig port or default to IEEE 
     */
    xd->hdr_mode = ENCAP_IEEE;

    

    /*
     * Init struct parameters 
     */
    xd->xd_unit = unit;
    xd->xd_tx_unit = unit;
    xd->xd_state = XD_IDLE;
    xd->xd_file = NULL;
    xd->xd_pkt_len = TAGGED_PACKET_LENGTH;
    xd->xd_pat = 0x12345678;
    xd->xd_pat_inc = 1;
    xd->xd_pat_random = 0;
    sal_memset(&xd->pkt_info.tx_pbmp, 0, sizeof(bcm_pbmp_t));
    BCM_PBMP_ASSIGN(xd->xd_ppsm_pbm, pcfg.all);
    xd->xd_vlan = 0x1;
    xd->xd_prio = 0;
    xd->xd_prio_int = -1;
    xd->xd_ppsm = 0;

    ENET_SET_MACADDR(xd->xd_mac_dst, default_mac_dst);
    ENET_SET_MACADDR(xd->xd_mac_src, default_mac_src);
    /*
     * Re-gen CRC by default 
     */
    xd->xd_crc = 1;

    if (xd->pkt_info.pkt_data)
    {
        /*
         * Has been setup before 
         */
        soc_cm_sfree(unit, xd->pkt_info._pkt_data.data);
        xd->pkt_info.pkt_data = NULL;
    }
    xd->pkt_info.flags = 0;

    if ((xd->pkt_info._pkt_data.data = (uint8 *) soc_cm_salloc(unit, xd->xd_pkt_len, "xd tx")) == NULL)
    {
        cli_out("WARNING: xd tx packet memory allocation failed\n");
        xd->pkt_info.pkt_data = NULL;
        xd->pkt_info._pkt_data.len = 0;
    }
    else
    {
        xd->pkt_info.pkt_data = &xd->pkt_info._pkt_data;
        xd->pkt_info.blk_count = 1;
        xd->pkt_info._pkt_data.len = xd->xd_pkt_len;
    }

    /*
     * clear out the itmh header contents 
     */
    xd->xd_itmh_present = 0;
    xd->itmh_version = 0;
    xd->itmh_pph_present = 0;
    xd->itmh_out_mirr_dis = 0;
    xd->itmh_in_mirr_dis = 0;
    xd->itmh_exclude_src = 0;
    xd->itmh_snoop_cmd = 0;
    xd->itmh_traffic_class = 0;
    xd->itmh_dp = 0;
    xd->itmh_fwd_type = 0;
    xd->itmh_dest_sys_port = 0;
    xd->itmh_flow_id = 0;
    xd->itmh_fec_ptr = 0;
    xd->itmh_multicast_id = 0;
    xd->itmh_src_sys_port_flag = 0;
    xd->itmh_src_sys_port = 0;
    xd->itmh_src_local_port = 0;
    xd->xd_ptch_src_port = 0;
    xd->xd_itmh_present = 0;
    xd->itmh_pph_type = 0;
    xd->itmh_in_mirr_flag = 0;
    xd->itmh_isq_flow = 0;
    xd->itmh_out_lif = 0;
    xd->itmh_multicast_flow = 0;
}

#define TX_LOAD_MAX             4096

#ifndef NO_FILEIO

/** ************************
* \brief
* Discard a single line from input file
***************************/
static int
diag_dnx_discard_line(
    FILE * fp)
{
    int c;
    do
    {
        if ((c = getc(fp)) == EOF)
        {
            return c;
        }
    }
    while (c != '\n');
    return c;
}

/** ************************
* \brief
* Load a single byte from input file
***************************/
static int
diag_dnx_tx_load_byte(
    FILE * fp,
    uint8 * byte)
{
    int c, d;

    do
    {
        if ((c = getc(fp)) == EOF)
        {
            return -1;
        }
        else if (c == '#')
        {
            if ((c = diag_dnx_discard_line(fp)) == EOF)
            {
                return -1;
            }
        }
    }
    while (!isxdigit(c));

    do
    {
        if ((d = getc(fp)) == EOF)
        {
            return -1;
        }
        else if (d == '#')
        {
            if ((d = diag_dnx_discard_line(fp)) == EOF)
            {
                return -1;
            }
        }
    }
    while (!isxdigit(d));

    *byte = (xdigit2i(c) << 4) | xdigit2i(d);

    return 0;
}

/** ************************
* \brief
* Load full packet from file
***************************/
static uint8 *
diag_dnx_tx_load_packet(
    int unit,
    char *fname,
    int *length)
{
    uint8 *p;
    FILE *fp;
    int i;

    if ((p = soc_cm_salloc(unit, TX_LOAD_MAX, "tx_packet")) == NULL)
        return p;

    if ((fp = sal_fopen(fname, "r")) == NULL)
    {
        soc_cm_sfree(unit, p);
        return NULL;
    }

    for (i = 0; i < TX_LOAD_MAX; i++)
    {
        if (diag_dnx_tx_load_byte(fp, &p[i]) < 0)
            break;
    }

    *length = i;

    sal_fclose(fp);

    return p;
}
#endif /* NO_FILEIO */

/** ************************
* \brief
* Parse packet information from bcm shell
***************************/
static uint8 *
diag_dnx_parse_user_packet_payload(
    int unit,
    char *packet_data,
    int *length)
{
    uint8 *p;
    char tmp, data_iter;
    int data_len, i, pkt_len, data_base;

    /*
     * If string data starts with 0x or 0X, skip it 
     */
    if ((packet_data[0] == '0') && ((packet_data[1] == 'x') || (packet_data[1] == 'X')))
    {
        data_base = 2;
    }
    else
    {
        data_base = 0;
    }

    data_len = strlen(packet_data) - data_base;

    pkt_len = data_len > *length ? data_len : *length;
    if (pkt_len < UNTAGGED_PACKET_LENGTH)
    {
        pkt_len = UNTAGGED_PACKET_LENGTH;
    }

    if ((p = soc_cm_salloc(unit, pkt_len, "tx_string_packet")) == NULL)
        return p;

    sal_memset(p, 0, pkt_len);

    /*
     * Convert char to value 
     */
    i = 0;
    while (i < data_len)
    {
        data_iter = packet_data[data_base + i];
        if (('0' <= data_iter) && (data_iter <= '9'))
        {
            tmp = data_iter - '0';
        }
        else if (('a' <= data_iter) && (data_iter <= 'f'))
        {
            tmp = data_iter - 'a' + 10;
        }
        else if (('A' <= data_iter) && (data_iter <= 'F'))
        {
            tmp = data_iter - 'A' + 10;
        }
        else
        {
            cli_out("Unexpected char: %c\n", data_iter);
            return NULL;
        }

        /*
         * String input is in 4b unit. Below we're filling in 8b:
         * offset is /2, and we shift by 4b if the input char is odd 
         */
        p[i / 2] |= tmp << (((i + 1) % 2) * 4);
        ++i;
    }

    *length = pkt_len;
    return p;
}

/** ************************
* \brief
* If data was read from file, extract to XD structure
***************************/
static void
diag_dnx_check_pkt_fields(
    xd_t * xd)
{
    /*
     * Ethernet packet header 
     */
    enet_hdr_t *ep;
    bcm_pkt_t *pkt_info = &xd->pkt_info;

    ep = (enet_hdr_t *) BCM_PKT_IEEE(pkt_info);

    if (XD_FILE(xd) || XD_STRING(xd))
    {   /* Loaded from file or string */
        /*
         * Also set parameters to file data so incrementing works 
         */

        ENET_COPY_MACADDR(&ep->en_dhost, xd->xd_mac_dst);
        ENET_COPY_MACADDR(&ep->en_shost, xd->xd_mac_src);

        if (!ENET_TAGGED(ep))
        {
            LOG_WARN(BSL_LOG_MODULE, (BSL_META("Warning:  Untagged packet read from file or string for tx.\n")));
            xd->xd_vlan = VLAN_ID_NONE;
        }
        else
        {
            xd->xd_vlan = VLAN_CTRL_ID(bcm_ntohs(ep->en_tag_ctrl));
            xd->xd_prio = VLAN_CTRL_PRIO(bcm_ntohs(ep->en_tag_ctrl));
        }
    }
    else
    {
        if (xd->xd_vlan)
        {
            /*
             * Tagged format 
             */
            ep->en_tag_ctrl = bcm_htons(VLAN_CTRL(xd->xd_prio, 0, xd->xd_vlan));
            ep->en_tag_len = bcm_htons(xd->xd_pkt_len) - ENET_TAGGED_HDR_LEN - ENET_CHECKSUM_LEN;
            ep->en_tag_tpid = bcm_htons(0x8100);
        }
        else
        {
            /*
             * Untagged format 
             */
            LOG_WARN(BSL_LOG_MODULE, (BSL_META("Warning:  Sending untagged packet.\n")));
            ep->en_untagged_len = bcm_htons(xd->xd_pkt_len) - ENET_UNTAGGED_HDR_LEN - ENET_CHECKSUM_LEN;
        }
        ENET_SET_MACADDR(ep->en_dhost, xd->xd_mac_dst);
        ENET_SET_MACADDR(ep->en_shost, xd->xd_mac_src);
    }
}

/** ************************
* \brief
* Get the first port in a bitmap
***************************/
static void
diag_dnx_tx_first_pbm(
    int unit,
    bcm_pbmp_t pbm,
    bcm_pbmp_t * rpbm)
{
    soc_port_t p, dport;

    COMPILER_REFERENCE(unit);

    BCM_PBMP_CLEAR(*rpbm);
    DPORT_BCM_PBMP_ITER(unit, pbm, dport, p)
    {
        LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "First to port %d\n"), p));
        BCM_PBMP_PORT_ADD(*rpbm, p);
        return;
    }

    LOG_INFO(BSL_LOG_MODULE, (BSL_META_U(unit, "Warning: first pbm null\n")));
}

/** ************************
* \brief
* Initialize tx/rx struct with itmh parameters
***************************/
static void
diag_dnx_tx_itmh_dest_port_set(
    int unit,
    uint8 * itmh_header,
    uint32 dest_port)
{
    xd_t *xd = xd_units[unit];
    bcm_pkt_t *pkt_info = &xd->pkt_info;

    xd->xd_prio_int = -1;
    xd->itmh_snoop_cmd = 0;
    xd->itmh_traffic_class = xd->xd_prio;
    xd->itmh_dp = 0;
    xd->itmh_fwd_type = 0;
    xd->itmh_dest_sys_port = dest_port;
    xd->itmh_pph_type = 0;
    xd->itmh_in_mirr_flag = 0;
    
    sal_memcpy(itmh_header, &pkt_info->_dpp_hdr[0], 4);
    xd->itmh_traffic_class = 0;

    return;
}

/** ************************
* \brief
* Strip VLAN tag from packet marked by parameter "data".
***************************/
static void
diag_dnx_tx_tag_strip(
    int unit,
    uint8 * data)
{
    xd_t *xd = xd_units[unit];
    bcm_pkt_t *pkt_info = &xd->pkt_info;
    enet_hdr_t *ep;

    ep = (enet_hdr_t *) data;

    if (!ENET_TAGGED(ep))
    {
        return;
    }
    sal_memcpy(data + 2 * ENET_MAC_SIZE, data + 2 * ENET_MAC_SIZE + ENET_TAG_SIZE, xd->xd_pkt_len - 16);
    xd->xd_pkt_len -= ENET_TAG_SIZE;
    pkt_info->pkt_data[0].len -= ENET_TAG_SIZE;

    return;
}

/** ************************
* \brief
* Recover VLAN tag from packet marked by parameter "data".
***************************/
static void
diag_dnx_tx_tag_recover(
    int unit,
    uint8 * data)
{
    xd_t *xd = xd_units[unit];
    bcm_pkt_t *pkt_info = &xd->pkt_info;
    enet_hdr_t *ep;

    ep = (enet_hdr_t *) data;
    if (!xd->xd_vlan)
    {
        return;
    }
    sal_memcpy(data + 2 * ENET_MAC_SIZE + ENET_TAG_SIZE, data + 2 * ENET_MAC_SIZE, xd->xd_pkt_len - 2 * ENET_MAC_SIZE);
    ep->en_tag_ctrl = bcm_htons(VLAN_CTRL(xd->xd_prio, 0, xd->xd_vlan));
    ep->en_tag_tpid = bcm_htons(ENET_DEFAULT_TPID);
    xd->xd_pkt_len += ENET_TAG_SIZE;
    pkt_info->pkt_data[0].len += ENET_TAG_SIZE;

    return;
}

/** ************************
* \brief
* Get the next port in a bitmap
***************************/
static int
diag_dnx_tx_next_port(
    int unit,
    bcm_pbmp_t * newpbm,
    bcm_pbmp_t allpbm)
{
    soc_port_t p, dport;
    soc_port_t np, ndport;
    int found = FALSE;
    char pfmt[SOC_PBMP_FMT_LEN];

    DPORT_BCM_PBMP_ITER(unit, *newpbm, ndport, np)
    {
        DPORT_BCM_PBMP_ITER(unit, allpbm, dport, p)
        {
            if (found)
            {
                LOG_INFO(BSL_LS_APPL_TX, (BSL_META_U(unit, "Next to port %d\n"), p));
                BCM_PBMP_CLEAR(*newpbm);
                BCM_PBMP_PORT_ADD(*newpbm, p);
                return 0;
            }
            if (np == p)
                found = TRUE;
        }
    }

    /*
     * If we get here, must be resetting. 
     */
    diag_dnx_tx_first_pbm(unit, allpbm, newpbm);
    LOG_INFO(BSL_LS_APPL_TX, (BSL_META_U(unit, "Resetting to pbm %s\n"), SOC_PBMP_FMT(*newpbm, pfmt)));
    return 1;
}

/**
 * \brief
 *   Transmit packet from BCM shell using bcm_tx API.
 *
 *  \par DIRECT INPUT:
 *    \param [in] *xd -
 *      TX/RX parameters struct.
 *  \par DIRECT OUTPUT:
 *    cmd_result_t - \n
 *      Return value of tx call.
 *  \remark
 *    None
 *  \see
 *    None
 *****************************************************/
static cmd_result_t
diag_dnx_do_tx(
    xd_t * xd)
{
    uint8 *pkt_data;            /* Packet */
    uint8 *payload;             /* data - packet payload */
    enet_hdr_t *ep;             /* Ethernet packet header */
    bcm_pkt_t *pkt_info = &xd->pkt_info;
    int rv = BCM_E_INTERNAL;
    int payload_len;
    int bitmap_not_null = FALSE;
    uint32 port_ndx;
    /*
     * bcm_gport_t        sysport_gport; 
     */
    int header_size = 0;

    pkt_data = NULL;

    /*
     * Allocate the packet; use tx_load if reading from file 
     */
    if (XD_FILE(xd))
    {
#ifdef NO_FILEIO
        cli_out("no filesystem\n");
        return CMD_FAIL;
#else
        if ((pkt_data = diag_dnx_tx_load_packet(xd->xd_unit, xd->xd_file, &xd->xd_pkt_len)) == NULL)
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META("Unable to load packet from file %s\n"), xd->xd_file));
            return CMD_FAIL;
        }
#endif
    }

    /*
     * use parse_user_packet_payload if reading from user string input 
     */
    if (XD_STRING(xd))
    {
        if ((pkt_data = diag_dnx_parse_user_packet_payload(xd->xd_unit, xd->xd_data, &xd->xd_pkt_len)) == NULL)
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META("Unable to allocate memory for packet %s\n"), xd->xd_data));
            return CMD_FAIL;
        }
    }

    /*
     * Make sure packet allocation size is right size 
     */
    if (pkt_info->pkt_data[0].len != xd->xd_pkt_len)
    {
        soc_cm_sfree(xd->xd_unit, pkt_info->pkt_data[0].data);
        if ((pkt_info->pkt_data[0].data = (uint8 *) soc_cm_salloc(xd->xd_unit, xd->xd_pkt_len, "TX")) == NULL)
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META("Unable to allocate packet memory\n")));
            return (CMD_FAIL);
        }
        pkt_info->pkt_data[0].len = xd->xd_pkt_len;
    }

    sal_memset(pkt_info->pkt_data[0].data, 0, pkt_info->pkt_data[0].len);
    /*
     * Data was read from file or string.  Copy into pkt_info 
     */
    if (pkt_data)
    {
        sal_memcpy(pkt_info->pkt_data[0].data, pkt_data, xd->xd_pkt_len);
        soc_cm_sfree(xd->xd_unit, pkt_data);
    }

    /*
     * Setup the packet 
     */
    pkt_info->flags &= ~BCM_TX_CRC_FLD;
    pkt_info->flags |= (xd->xd_crc == 1 ? BCM_TX_CRC_REGEN : 0) | (xd->xd_crc == 2 ? BCM_TX_CRC_APPEND : 0);

    pkt_info->flags &= ~BCM_TX_NO_PAD;
    if (xd->xd_pkt_len < (xd->xd_vlan != 0 ? TAGGED_PACKET_LENGTH : UNTAGGED_PACKET_LENGTH))
    {
        pkt_info->flags |= BCM_TX_NO_PAD;
    }

    

    diag_dnx_check_pkt_fields(xd);

    /*
     * setup the headers 
     */
    /*
     * itmh header 
     */
    

    /*
     * ptch header 
     */
    ep = (enet_hdr_t *) (pkt_info->pkt_data[0].data);

    payload = &pkt_info->pkt_data[0].data[sizeof(enet_hdr_t)];
    payload_len = pkt_info->pkt_data[0].len - sizeof(enet_hdr_t) - sizeof(uint32) /* CRC */ ;
    payload_len = payload_len - header_size;

    /*
     * save base info. setup first port. 
     */
    if (xd->xd_ppsm)
    {
        LOG_INFO(BSL_LS_APPL_TX, (BSL_META("Per port source is active\n")));
        xd->xd_ppsm_pbm = pkt_info->tx_pbmp;
        diag_dnx_tx_first_pbm(xd->xd_unit, pkt_info->tx_pbmp, &pkt_info->tx_pbmp);
        ENET_COPY_MACADDR(xd->xd_mac_src, xd->xd_mac_src_base);
    }

    /*
     * XMIT all the required packets 
     */
    for (xd->xd_cur_cnt = 0; xd->xd_cur_cnt < xd->xd_tot_cnt; xd->xd_cur_cnt++)
    {

        if (xd->xd_state != XD_RUNNING)
        {
            break;
        }

        /*
         * Generate pattern on first time through, or every time if
         * pattern is incrementing.
         */

        if (xd->xd_mac_dst_inc != 0)
        {
            ENET_SET_MACADDR(ep->en_dhost, xd->xd_mac_dst);
        }
        if (xd->xd_mac_src_inc != 0)
        {
            ENET_SET_MACADDR(ep->en_shost, xd->xd_mac_src);
        }

        /*
         * Store pattern 
         */
        if (!XD_FILE(xd) && !XD_STRING(xd) && xd->xd_pat_random)
        {
            packet_random_store(payload, payload_len);
        }
        else if (!XD_FILE(xd) && !XD_STRING(xd) && (xd->xd_cur_cnt == 0 || xd->xd_pat_inc != 0))
        {
            xd->xd_pat = packet_store(payload, payload_len, xd->xd_pat, xd->xd_pat_inc);
        }

        /*
         * put packet to member port of tx_pbmp one by one via ITMH header
         */
        BCM_PBMP_ITER(pkt_info->tx_pbmp, port_ndx)
        {
            
            /*
             * set destination port in ITMH header with target port extracted from tx_pbmp
             */
            diag_dnx_tx_itmh_dest_port_set(xd->xd_tx_unit, pkt_info->pkt_data[0].data + 2 /*DPP_HDR_PTCH_TYPE2_LEN */ ,
                                           0 /*BCM_GPORT_SYSTEM_PORT_ID_GET(sysport_gport) */ );
            if (BCM_PBMP_MEMBER(pkt_info->tx_upbmp, port_ndx))
            {   /*strip VLAN tag for member of tx_upbmp */
                diag_dnx_tx_tag_strip(xd->xd_tx_unit, pkt_info->pkt_data[0].data + 2 /*DPP_HDR_ITMH_BASE_LEN */  +
                                      4 /*DPP Constant */ );
            }
            rv = bcm_tx(xd->xd_tx_unit, pkt_info, NULL);
            if (BCM_PBMP_MEMBER(pkt_info->tx_upbmp, port_ndx))
            {
                /*
                 * add VLAN tag back after packet transmisstion for member of tx_upbmp
                 */
                diag_dnx_tx_tag_recover(xd->xd_tx_unit, pkt_info->pkt_data[0].data + 2 /*DPP Constant */  +
                                        4 /*DPP Constant */ );
            }
            if (rv != BCM_E_NONE)
            {
                LOG_ERROR(BSL_LOG_MODULE,
                          (BSL_META("bcm_tx failed: TX Unit %d: %s\n"), xd->xd_tx_unit, bcm_errmsg(rv)));
                break;
            }
        }
        if (FALSE == bitmap_not_null)
        {
            if ((rv = bcm_tx(xd->xd_tx_unit, pkt_info, NULL)) != BCM_E_NONE)
            {
                LOG_ERROR(BSL_LOG_MODULE,
                          (BSL_META("bcm_tx failed: TX Unit %d: %s\n"), xd->xd_tx_unit, bcm_errmsg(rv)));
                break;
            }
        }

        increment_macaddr(xd->xd_mac_dst, xd->xd_mac_dst_inc);
        increment_macaddr(xd->xd_mac_src, xd->xd_mac_src_inc);

        /*
         * per port source mac 
         */
        if (xd->xd_ppsm)
        {
            /*
             * change xd_pbm to next port. reset src mac if at base port 
             */
            if (diag_dnx_tx_next_port(xd->xd_unit, &pkt_info->tx_pbmp, xd->xd_ppsm_pbm))
            {
                LOG_INFO(BSL_LOG_MODULE, (BSL_META("resetting mac\n")));
                ENET_COPY_MACADDR(xd->xd_mac_src_base, xd->xd_mac_src);
            }
        }
    }

    /*
     * Replace original params 
     */
    if (xd->xd_ppsm)
    {
        ENET_COPY_MACADDR(xd->xd_mac_src_base, xd->xd_mac_src);
        pkt_info->tx_pbmp = xd->xd_ppsm_pbm;
    }

    return (rv == BCM_E_NONE ? CMD_OK : CMD_FAIL);
}

/**
 * \brief
 *   Process a tx command from the bcm shell
 *
 *  \par DIRECT INPUT:
 *    \param [in] u -
 *      Unit number.
 *    \param [in] *a -
 *      Pointer to struct with command content.
 *  \par DIRECT OUTPUT:
 *    cmd_result_t - \n
 *      Return value of tx call.
 *  \remark
 *    None
 *  \see
 *    None
 *****************************************************/
cmd_result_t
cmd_dnx_tx(
    int u,
    args_t * a)
{
    xd_t *xd;
    volatile cmd_result_t rv;
    jmp_buf ctrl_c;

    if (!sh_check_attached(ARG_CMD(a), u))
    {
        return (CMD_FAIL);
    }

    /*
     * Init the tx/rx struct 
     */
    _XD_INIT(u, xd);
    if (xd->xd_state == XD_RUNNING)
    {
        cli_out("%s: Error: already active\n", ARG_CMD(a));
        return (CMD_FAIL);
    }

    /*
     * Parse the argument line 
     */
    if (CMD_OK != (rv = diag_dnx_tx_parse(u, a, xd)))
    {
        return (rv);
    }

#ifndef NO_CTRL_C
    if (!setjmp(ctrl_c))
    {
#endif
        sh_push_ctrl_c(&ctrl_c);
        xd->xd_state = XD_RUNNING;
        /*
         * Perform the TX operation 
         */
        rv = diag_dnx_do_tx(xd);
#ifndef NO_CTRL_C
    }
    else
    {
        rv = CMD_INTR;
    }
#endif

    sh_pop_ctrl_c();
    xd->xd_state = XD_IDLE;

    return (rv);
}
