/*
 * $Id: txrx.c,v 1.34 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * socdiag tx (transmit) and rx (receive) commands
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>

#include <soc/types.h>
#include <soc/debug.h>
#include <soc/cm.h>

#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/net.h>
#include <linux/in.h>
#else
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#endif

#include <soc/robo/mcm/driver.h>
#include <soc/dma.h>
#include <sal/types.h>
#include <sal/appl/sal.h>
#include <sal/appl/io.h>
#include <sal/core/thread.h>

#include <bcm/tx.h>
#include <bcm/pkt.h>
#include <bcm_int/robo/rx.h>
#include <bcm/port.h>
#include <bcm/error.h>

#include <appl/diag/system.h>

/* These modes must correspond with the order of strings below */
#define ENCAP_IEEE  0

static char *hdr_modes[] = {"Ieee",NULL};

typedef struct xd_s {           /* TX/RX description */
    int     xd_init;        /* TRUE --> been here before */
    int     xd_unit;        /* Unit # */
    int     xd_ppsm;        /* true --> per port src macs */
    enum {
    XD_IDLE,            /* Not doing anything */
    XD_RUNNING,         /* Running */
    XD_STOP             /* Stop requested */
    }       xd_state;
    uint32  xd_tot_cnt;     /* # to send/receive */
    uint32  xd_cur_cnt;     /* # sent/received */
    int     xd_pkt_len;     /* Packet length */
    char    *xd_file;       /* File name of packet data */
    sal_mac_addr_t xd_mac_dst; /* Destination mac address */
    sal_mac_addr_t xd_mac_src; /* Source mac address */
    sal_mac_addr_t xd_mac_src_base; /* Source mac address for pps */
                                    /* port source mac */
    uint32  xd_mac_dst_inc;     /* Destination mac increment */
    uint32  xd_mac_src_inc;     /* Source mac increment */
    uint32  xd_pat;         /* XMIT pattern */
    uint32  xd_pat_inc;     /* Pattern Increment */
    pbmp_t  xd_ppsm_pbm;        /* Saved port bit map for per */
                                        /* port source mac */
    uint32  xd_vlan;        /* vlan id (0 if untagged) */
    uint32  xd_prio;        /* vlan prio */
    uint32  xd_crc;

    /* Packet info is now in bcm_pkt_t structure */
    bcm_pkt_t   pkt_info;
    int         hdr_mode;               /* Mode type */
    int     xd_cbk_enable;        /* true: register default tx callback function */
} xd_t;

static xd_t xd_units[SOC_MAX_NUM_DEVICES];

#define XD_FILE(xd) ((xd)->xd_file != NULL && (xd)->xd_file[0] != 0)

static void
_robo_tx_callback(int unit, bcm_pkt_t *pkt, void *cookie)
{
    cli_out("TX callback function: TX pkt = %p done.\n", (void *)pkt);

}

STATIC void
_robo_xd_init(int unit, xd_t *xd)
{
    static sal_mac_addr_t default_mac_src ={ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };
    static sal_mac_addr_t default_mac_dst ={ 0x10, 0x11, 0x12, 0x13, 0x14, 0x15 };

    /*
     * First time through, everything 0 (BSS), so set up defaults.
     */

    xd->hdr_mode = 0;

    xd->xd_unit     = unit;
    xd->xd_init     = TRUE;
    xd->xd_state    = XD_IDLE;
    xd->xd_file = NULL;
    xd->xd_pkt_len  = 68;
    xd->xd_pat  = 0x12345678;
    xd->xd_pat_inc  = 1;
    SOC_PBMP_ASSIGN(xd->pkt_info.tx_pbmp, PBMP_ALL(unit));
    SOC_PBMP_ASSIGN(xd->xd_ppsm_pbm, PBMP_ALL(unit));
    xd->xd_vlan = 0x1;
    xd->xd_prio = 0;
    xd->xd_ppsm = 0;
#if defined(BCM_TB_SUPPORT) || defined(BCM_POLAR_SUPPORT)
    xd->pkt_info.prio_int = 0;
#endif /* BCM_TB_SUPPORT || BCM_POLAR_SUPPORT */
#ifdef BCM_TB_SUPPORT
    xd->pkt_info.multicast_group= -1;
    xd->pkt_info.color = bcmColorGreen;
    xd->pkt_info.flow_id= 0;
    xd->pkt_info.filter_enable= 0;
#endif /* BCM_TB_SUPPORT */

    xd->xd_cbk_enable = 0;
    xd->pkt_info.call_back = NULL;

    ENET_SET_MACADDR(xd->xd_mac_dst, default_mac_dst);
    ENET_SET_MACADDR(xd->xd_mac_src, default_mac_src);
    xd->xd_crc      = 1;        /* Re-gen CRC by default */

    if (xd->pkt_info.pkt_data) { /* Has been setup before */
        soc_cm_sfree(unit, xd->pkt_info.alloc_ptr);
        xd->pkt_info.pkt_data = NULL;
    }
    xd->pkt_info.flags = 0;

    if ((xd->pkt_info.alloc_ptr = (uint8 *)soc_cm_salloc(unit,
                   xd->xd_pkt_len, "TX")) == NULL) {
        cli_out("WARNING:  Could not allocate tx buffer.  Memory error.\n");
        xd->xd_init     = FALSE;
        xd->pkt_info.pkt_data = NULL;
        xd->pkt_info._pkt_data.len = 0;
    } else {
        xd->pkt_info._pkt_data.data = xd->pkt_info.alloc_ptr;
        xd->pkt_info.pkt_data = &xd->pkt_info._pkt_data;
        xd->pkt_info.blk_count = 1;
        xd->pkt_info._pkt_data.len = xd->xd_pkt_len;
    }
}

#if 0 /*David*/
STATIC bcm_pkt_t 
*txrx_pkt_alloc(int unit, int nblocks, int *sizes, int flags)
{
    bcm_pkt_t *pkt;
    int i, j;

    if (!(pkt = sal_alloc(sizeof(bcm_pkt_t), "txrx pkt"))) {
        return NULL;
    }

    pkt->blk_count = nblocks;
    if (nblocks == 1) {
        pkt->pkt_data = &pkt->_pkt_data;
    } else {
        if (!(pkt->pkt_data = sal_alloc(sizeof(bcm_pkt_blk_t) * nblocks,
                                        "tx pdata"))) {
            sal_free(pkt);
            return NULL;
        }
    }

    for (i = 0; i < nblocks; i++) {
        pkt->pkt_data[i].len = sizes[i];
        if (!(pkt->pkt_data[i].data = soc_cm_salloc(unit, sizes[i],
                                                    "txrx data"))) {
            for (j = 0; j < i; j++) {
                soc_cm_sfree(unit, pkt->pkt_data[j].data);
            }
            if (nblocks > 1) {
                sal_free(pkt->pkt_data);
            }
            sal_free(pkt);
            return NULL;
        }
    }

    pkt->unit = unit;
    pkt->flags = flags;
    return pkt;
}

STATIC void
txrx_pkt_free(int unit, bcm_pkt_t *pkt)
{
    int i;

    for (i = 0; i < pkt->blk_count; i++) {
        soc_cm_sfree(unit, pkt->pkt_data[i].data);
    }

    if (pkt->pkt_data != &pkt->_pkt_data) {
        sal_free(pkt->pkt_data);
    }

    sal_free(pkt);
}

#endif

#define _XD_INIT(unit, xd) if (!(xd)->xd_init) _robo_xd_init(unit, xd)

STATIC cmd_result_t
robo_tx_parse(int u, args_t *a, xd_t *xd)
/*
 * Function:    txrx_parse
 * Purpose: Parse input parameters into a xd structure.
 * Parameters:  u - unit #.
 *      a - pointer to arguments
 *      xd - pointer to xd structure to fill in.
 * Returns: 0 - success, -1 failed.
 */
{
    static char *crc_list[] = {"None", "Recompute", "Append", NULL};
    parse_table_t   pt;
    int         min_len, tagged;
    char        *xfile;
#if defined(BCM_TB_SUPPORT) || defined(BCM_POLAR_SUPPORT)
    uint32 traffic_class = 0;
#endif /* BCM_TB_SUPPORT || BCM_POLAR_SUPPORT */

    /* First arg is count */

    if (!ARG_CNT(a) || !isint(ARG_CUR(a))) {
        return(CMD_USAGE);
    }

    _XD_INIT(u, xd);

    xd->xd_tot_cnt = parse_integer(ARG_GET(a));

    parse_table_init(u, &pt);

    /* Add XGS Ethernet/BCM5632/HIGIG Options */
    if (xd->hdr_mode == ENCAP_IEEE) {
        /* Do nothing, already setup */
    }

    parse_table_add(&pt, "PortBitMap",  PQ_DFL|PQ_PBMP, 0,
            &xd->pkt_info.tx_pbmp,      NULL);

    parse_table_add(&pt, "UntagBitMap", PQ_DFL|PQ_PBMP, 0,
            &xd->pkt_info.tx_upbmp,     NULL);

    parse_table_add(&pt, "Filename",    PQ_DFL|PQ_STRING,0,
            &xd->xd_file,       NULL);
    parse_table_add(&pt, "Length",  PQ_DFL|PQ_INT,  0,
            &xd->xd_pkt_len,    NULL);
    parse_table_add(&pt, "VLantag",     PQ_DFL|PQ_HEX,  0,
            &xd->xd_vlan,       NULL);
    parse_table_add(&pt, "VlanPrio",    PQ_DFL|PQ_INT,  0,
            &xd->xd_prio,       NULL);
    parse_table_add(&pt, "Pattern",     PQ_DFL|PQ_HEX,  0,
            &xd->xd_pat,        NULL);
    parse_table_add(&pt, "PatternInc",  PQ_DFL|PQ_INT,  0,
            &xd->xd_pat_inc,    NULL);
    parse_table_add(&pt, "PerPortSrcMac",PQ_DFL|PQ_INT, 0,
            &xd->xd_ppsm,    NULL);
    parse_table_add(&pt, "SourceMac",   PQ_DFL|PQ_MAC,  0,
            &xd->xd_mac_src,    NULL);
    parse_table_add(&pt, "SourceMacInc",PQ_DFL|PQ_INT,  0,
            &xd->xd_mac_src_inc,NULL);
    parse_table_add(&pt, "DestMac",     PQ_DFL|PQ_MAC,  0,
            &xd->xd_mac_dst,    NULL);
    parse_table_add(&pt, "DestMacInc",  PQ_DFL|PQ_INT,  0,
            &xd->xd_mac_dst_inc,NULL);
    parse_table_add(&pt, "CRC",     PQ_DFL|PQ_MULTI,0,
            &xd->xd_crc,        crc_list);
    parse_table_add(&pt, "CallbackFunction",PQ_DFL|PQ_INT, 0,
            &xd->xd_cbk_enable,    NULL);
#if defined(BCM_TB_SUPPORT) || defined(BCM_POLAR_SUPPORT)
    parse_table_add(&pt, "TrafficClass",     PQ_DFL|PQ_INT,     0,
                    &traffic_class, NULL);
#endif /* BCM_TB_SUPPORT || BCM_POLAR_SUPPORT */
#ifdef BCM_TB_SUPPORT
    parse_table_add(&pt, "McastGroupID",     PQ_DFL|PQ_INT,     0,
                    &xd->pkt_info.multicast_group, NULL);
    parse_table_add(&pt, "DropPrecedence",   PQ_DFL|PQ_INT,   0,
                    &xd->pkt_info.color, NULL);
    parse_table_add(&pt, "FlowID",        PQ_DFL|PQ_INT,     0,
                    &xd->pkt_info.flow_id, NULL);
    parse_table_add(&pt, "FilTeRs",             PQ_DFL|PQ_HEX,  0,
                    &xd->pkt_info.filter_enable, NULL);
#endif /* BCM_TB_SUPPORT */

    /* Parse remaining arguments */
    if (0 > parse_arg_eq(a, &pt)) {
        cli_out("%s: Error: Invalid option or malformed expression: %s\n",
                ARG_CMD(a), ARG_CUR(a));
        parse_arg_eq_done(&pt);
        return(CMD_FAIL);
    }

    if (xd->xd_file) {
        xfile = sal_strdup(xd->xd_file);
    } else {
        xfile = NULL;
    }
    parse_arg_eq_done(&pt);
    xd->xd_file = xfile;

    tagged = (xd->xd_vlan != 0);
    min_len = (tagged ? 68 : 64);

    if (!XD_FILE(xd) && xd->xd_pkt_len < min_len) {
        cli_out("%s: Warning: Length %d too small for %s packet (min %d)\n",
                ARG_CMD(a),
                xd->xd_pkt_len,
                tagged ? "tagged" : "untagged",
                min_len);
    }

#if defined(BCM_TB_SUPPORT) || defined(BCM_POLAR_SUPPORT)
    xd->pkt_info.prio_int = traffic_class;
#endif /* BCM_TB_SUPPORT || BCM_POLAR_SUPPORT */

    return(0);
}

/*
 * Function:    _robo_tx_first_pbm(pbmp_t pbm)
 * Purpose: Get the first port in a bitmap.
 * Parameters:  pbm -- the port bitmap
 * Returns: portbitmap with only one port set.
 */
STATIC bcm_pbmp_t
_robo_tx_first_pbm(int unit, bcm_pbmp_t pbm)
{
    soc_port_t p;
    bcm_pbmp_t  rpbm;

    COMPILER_REFERENCE(unit);

    BCM_PBMP_CLEAR(rpbm);
    PBMP_ITER(pbm, p) {
        LOG_INFO(BSL_LS_APPL_TX,
                 (BSL_META_U(unit,
                             "First to port %d\n"), p));
        BCM_PBMP_PORT_ADD(rpbm, p);
        return rpbm;
    }

    LOG_INFO(BSL_LS_APPL_TX,
             (BSL_META_U(unit,
                         "Warning: first pbm null\n")));
    BCM_PBMP_CLEAR(rpbm);
    return rpbm;  /* no port in bit map */
}

/*
 * Function:    int _robo_tx_next_port(pbmp_t *newpbm, pbmp_t allpbm)
 * Purpose: Get the first port in a bitmap.
 * Parameters:  *newpbm -- the current port bitmap (one port)
 *                         set to next port
 *              allpbm  -- all ports being iterated over.
 * Returns: 0 if okay.  1 if reset to first port in allpbm
 */
STATIC int
_robo_tx_next_port(int unit, pbmp_t *newpbm, pbmp_t allpbm)
{
    soc_port_t p;
    soc_port_t np;
    int found = FALSE;
    char pfmt[SOC_PBMP_FMT_LEN];

    PBMP_ITER(*newpbm, np) {
        PBMP_ITER(allpbm, p) {
            if (found) {
                LOG_INFO(BSL_LS_APPL_TX,
                         (BSL_META_U(unit,
                                     "Next to port %d\n"), p));
                BCM_PBMP_CLEAR(*newpbm);
                BCM_PBMP_PORT_ADD(*newpbm, p);
                return 0;
            }
            if (np == p) {
                found = TRUE;
            }
        }
    }

    /* If we get here, must be resetting. */
    *newpbm = _robo_tx_first_pbm(unit, allpbm);
    LOG_INFO(BSL_LS_APPL_TX,
             (BSL_META_U(unit,
                         "Resetting to pbm %s\n"), SOC_PBMP_FMT(*newpbm, pfmt)));
    return 1;
}

#define TX_LOAD_MAX     4096

#ifndef NO_FILEIO

STATIC int
robo_tx_load_byte(FILE *fp, uint8 *byte)
{
    int     c, d;

    do {
        if ((c = getc(fp)) == EOF)
            return -1;
    } while (!isxdigit(c));

    do {
        if ((d = getc(fp)) == EOF)
            return -1;
    } while (!isxdigit(d));

    *byte = (xdigit2i(c) << 4) | xdigit2i(d);

    return 0;
}

STATIC uint8 *
robo_tx_load_packet(int unit, char *fname, int *length)
{
    uint8       *p;
    FILE        *fp;
    int         i;

    if ((p = soc_cm_salloc(unit, TX_LOAD_MAX, "tx_packet")) == NULL) {
        return p;
    }

    if ((fp = sal_fopen(fname, "r")) == NULL) {
        soc_cm_sfree(unit, p);
        return NULL;
    }

    for (i = 0; i < TX_LOAD_MAX; i++) {
        if (robo_tx_load_byte(fp, &p[i]) < 0) {
            break;
        }
    }

    *length = i;

    sal_fclose(fp);

    return p;
}
#endif /* NO_FILEIO */

/* If data was read from file, extract to XD structure */
STATIC void
robo_check_pkt_fields(xd_t *xd)
{
    enet_hdr_t        *ep;      /* Ethernet packet header */
    bcm_pkt_t      *pkt_info = &xd->pkt_info;

    ep = (enet_hdr_t *)BCM_PKT_IEEE(pkt_info);

    if (XD_FILE(xd)) {          /* Loaded from file */
        /* Also set parameters to file data so incrementing works */

        ENET_COPY_MACADDR(xd->xd_mac_dst, &ep->en_dhost);
        ENET_COPY_MACADDR(xd->xd_mac_src, &ep->en_shost);

        if (!ENET_TAGGED(ep)) {
            cli_out("Warning:  Untagged packet read from file for tx.\n");
            xd->xd_vlan = VLAN_ID_NONE;
        } else {
            xd->xd_vlan = VLAN_CTRL_ID(ntohs(ep->en_tag_ctrl));
            xd->xd_prio = VLAN_CTRL_PRIO(ntohs(ep->en_tag_ctrl));
        }
    } else {
        if (xd->xd_vlan) {          /* Tagged format */
            ep->en_tag_ctrl = htons(VLAN_CTRL(xd->xd_prio, 0, xd->xd_vlan));
            ep->en_tag_len  = htons(xd->xd_pkt_len);
            ep->en_tag_tpid = htons(0x8100);
        } else {                /* Untagged format */
            cli_out("Warning:  Sending untagged packet.\n");
            ep->en_untagged_len = htons(xd->xd_pkt_len);
        }
        ENET_SET_MACADDR(ep->en_dhost, xd->xd_mac_dst);
        ENET_SET_MACADDR(ep->en_shost, xd->xd_mac_src);
    }
}

STATIC cmd_result_t
robo_do_tx(xd_t *xd)
/*
 * Function:    robo_do_tx
 * Purpose: Perform actual work for a TX operation.
 * Parameters:  xd - pointer to XD structure already filled in.
 * Returns: CMD_xxx
 */
{
    uint8         *pkt_data;    /* Packet */
    uint8         *payload;     /* data - packet payload */
    enet_hdr_t        *ep;      /* Ethernet packet header */
    bcm_pkt_t         *pkt_info = &xd->pkt_info;
    int            rv = BCM_E_INTERNAL;
    int                payload_len;
#ifdef BCM_TB_SUPPORT
    pbmp_t test_pbmp;
#endif

    pkt_data = NULL;

    /* Allocate the packet; use tx_load if reading from file */
    if (XD_FILE(xd)) {
#ifdef NO_FILEIO
        cli_out("no filesystem\n");
        return CMD_FAIL;
#else
        if ((pkt_data = robo_tx_load_packet(xd->xd_unit,
                                xd->xd_file, &xd->xd_pkt_len)) == NULL) {
            cli_out("Unable to load packet from file %s\n", xd->xd_file);
            return CMD_FAIL;
        }
#endif
    }

    /* Make sure packet allocation size is right size */
    if (pkt_info->pkt_data[0].len != xd->xd_pkt_len) {
        soc_cm_sfree(xd->xd_unit, pkt_info->pkt_data[0].data);
        if ((pkt_info->pkt_data[0].data = (uint8 *)
             soc_cm_salloc(xd->xd_unit, xd->xd_pkt_len, "TX")) == NULL) {
            cli_out("Unable to allocate packet memory\n");
            return(CMD_FAIL);
        }
        pkt_info->pkt_data[0].len = xd->xd_pkt_len;
    }

    sal_memset(pkt_info->pkt_data[0].data, 0, pkt_info->pkt_data[0].len);
    if (pkt_data) { /* Data was read from file.  Copy into pkt_info */
        sal_memcpy(pkt_info->pkt_data[0].data, pkt_data, xd->xd_pkt_len);
        soc_cm_sfree(xd->xd_unit, pkt_data);
    }

    /* Setup the packet */
    pkt_info->opcode = 0; /* initialize opcode */
    pkt_info->flags = 0; /* initialize flags */
    pkt_info->flags &= ~BCM_TX_CRC_FLD;
    pkt_info->flags |= (xd->xd_crc==1 ? BCM_TX_CRC_REGEN : 0) |
        (xd->xd_crc==2 ? BCM_TX_CRC_APPEND: 0);

    if (SOC_IS_TBX(xd->xd_unit)) {
#ifdef BCM_TB_SUPPORT
    if (BCM_PBMP_IS_NULL(pkt_info->tx_pbmp)) {
        /* 
         * Check if tx packet with MGID 
         * tx pbmp is null.
         */
        if (xd->pkt_info.multicast_group >= 0) {
            pkt_info->opcode = BCM_PKT_OPCODE_MC;
        }
    } else {
        /* 
          * Check if tx CPU loopbacked packet 
          * tx pbmp is cpu port only.
          */
        BCM_PBMP_CLEAR(test_pbmp);
        BCM_PBMP_PORT_SET(test_pbmp, CMIC_PORT(xd->xd_unit));
        if (BCM_PBMP_EQ(pkt_info->tx_pbmp, test_pbmp)) {
            pkt_info->flags |= BCM_TX_LOOPBACK;
        }
    }

#endif /* BCM_TB_SUPPORT */
    }

    if (xd->xd_cbk_enable) {
        /* callback function is not required */
        xd->pkt_info.call_back = (bcm_pkt_cb_f)_robo_tx_callback;
    } else {
        xd->pkt_info.call_back = NULL;
    }

    if (xd->xd_pkt_len < (xd->xd_vlan != 0 ? 68 : 64)) {
        pkt_info->flags |= BCM_TX_NO_PAD;
    }

    robo_check_pkt_fields(xd);

    ep = (enet_hdr_t *)(pkt_info->pkt_data[0].data);

    payload = &pkt_info->pkt_data[0].data[sizeof(enet_hdr_t)];
    payload_len = pkt_info->pkt_data[0].len - sizeof(enet_hdr_t) -
        sizeof(uint32) /* CRC */;

    if (xd->xd_ppsm) { /* save base info. setup first port. */
        LOG_INFO(BSL_LS_APPL_TX,
                 (BSL_META("Per port source is active\n")));
        xd->xd_ppsm_pbm = pkt_info->tx_pbmp;
        pkt_info->tx_pbmp = _robo_tx_first_pbm(xd->xd_unit, pkt_info->tx_pbmp);
        ENET_COPY_MACADDR(xd->xd_mac_src, xd->xd_mac_src_base);
    }

    /* XMIT all the required packets */
    for (xd->xd_cur_cnt = 0;
     xd->xd_cur_cnt < xd->xd_tot_cnt;
     xd->xd_cur_cnt++) {
        if (xd->xd_state != XD_RUNNING) {
            break;          /* Abort */
        }
    
        /*
         * Generate pattern on first time through, or every time if
         * pattern is incrementing.
         */
    
        if (xd->xd_mac_dst_inc != 0) {
            ENET_SET_MACADDR(ep->en_dhost, xd->xd_mac_dst);
        }
        if (xd->xd_mac_src_inc != 0) {
            ENET_SET_MACADDR(ep->en_shost, xd->xd_mac_src);
        }
    
            /* Store pattern */
        if (!XD_FILE(xd) &&
            (xd->xd_cur_cnt == 0 || xd->xd_pat_inc != 0)) {
            xd->xd_pat =
            packet_store(payload, payload_len,
                                 xd->xd_pat, xd->xd_pat_inc);
        }
    
        if ((rv = bcm_tx(xd->xd_unit, pkt_info, NULL)) != BCM_E_NONE) {
            cli_out("bcm_tx failed: Unit %d: %s\n",
                    xd->xd_unit, bcm_errmsg(rv));
            break;
        }
    
        increment_macaddr(xd->xd_mac_dst, xd->xd_mac_dst_inc);
        increment_macaddr(xd->xd_mac_src, xd->xd_mac_src_inc);
    
        if (xd->xd_ppsm) { /* per port source mac */
            /* change xd_pbm to next port. reset src mac if at base port */
            if (_robo_tx_next_port(xd->xd_unit, &pkt_info->tx_pbmp,
                              xd->xd_ppsm_pbm)) {
                LOG_INFO(BSL_LS_APPL_TX,
                         (BSL_META("resetting mac\n")));
                ENET_COPY_MACADDR(xd->xd_mac_src_base, xd->xd_mac_src);
            }
        }
    }

    if (xd->xd_ppsm) { /* Replace original params */
        ENET_COPY_MACADDR(xd->xd_mac_src_base, xd->xd_mac_src);
        pkt_info->tx_pbmp = xd->xd_ppsm_pbm;
    }

    return(rv == BCM_E_NONE ? CMD_OK : CMD_FAIL);
}

STATIC void
robo_do_tx_start(void *xdv)
/*
 * Function:    robo_do_tx_start
 * Purpose: Wrapper function for a tx_start thread.
 * Parameters:  xdv - pointer to xd structure for the unit under test.
 * Returns: Nothing, does not return.
 */
{
    cmd_result_t rv;
    xd_t    *xd = (xd_t *)xdv;

    rv = robo_do_tx(xd);

    cli_out("TX Completed %s: TX Requested(%d) Tx Sent(%d)\n",
            rv == CMD_OK ?"successfully" : "with error",
            xd->xd_tot_cnt, xd->xd_cur_cnt);
    xd->xd_state = XD_IDLE;
    sal_thread_exit(rv == CMD_OK ? 0 : 1);
}


/*
 * Function:    tx_count
 * Purpose: Print out status of any currently running TX command.
 * Parameters:  u - unit number.
 *      a - arguments, none expected.
 * Returns: CMD_OK
 */
cmd_result_t
cmd_robo_tx_count(int u, args_t *a)
{
    xd_t    *xd = &xd_units[u];

    if (ARG_CNT(a)) {
        return(CMD_USAGE);
    }
    cli_out("TX (%sRunning) Transmit Req(%d) Sent(%d)\n",
            xd->xd_state == XD_RUNNING ? "" : "Not-",
            xd->xd_tot_cnt, xd->xd_cur_cnt);
    return(CMD_OK);
}

/*
 * Function:    tx_start
 * Purpose: Start off a background TX thread.
 * Parameters:  u - unit number
 *      a - arguments.
 * Returns: CMD_XXX
 */
cmd_result_t
cmd_robo_tx_start(int u, args_t *a)
{
    cmd_result_t rv;
    xd_t    *xd = &xd_units[u];

    if (!sh_check_attached(ARG_CMD(a), u)) {
        return(CMD_FAIL);
    } else if (xd->xd_state == XD_RUNNING) {
        cli_out("%s: Error: tx command running\n", ARG_CMD(a));
        return(CMD_FAIL);
    }

#ifdef BROADCOM_DEBUG
    if (ARG_CUR(a) && !sal_strcasecmp(_ARG_CUR(a), "show")) {
        bcm_tx_show(u);
        return CMD_OK;
    }
#endif /* BROADCOM_DEBUG */

    if (CMD_OK != (rv = robo_tx_parse(u, a, xd))) {
        return(rv);
    }

    /* BLOCK_CNTL_C */
    xd->xd_state = XD_RUNNING;      /* Say GO */

    if (SAL_THREAD_ERROR == sal_thread_create("TX", SAL_THREAD_STKSZ, 100,
                          robo_do_tx_start, (void *)xd)) {
        cli_out("%s: Error: Failed to create background thread\n",ARG_CMD(a));
        xd->xd_state = XD_IDLE;
        rv = CMD_FAIL;
    }
    /* UN-BLOCK_CNTL_C */
    return(rv);
}

/*
 * Function:    tx_stop
 * Purpose: Stop a currently running TX command
 * Parameters:  u - unit number.
 *      a - arguments (none expected).
 * Returns: CMD_OK/CMD_USAGE/CMD_FAIL
 */
cmd_result_t
cmd_robo_tx_stop(int u, args_t *a)
{
    xd_t    *xd = &xd_units[u];

    if (ARG_CNT(a)) {
        return(CMD_USAGE);
    } else if (!sh_check_attached(ARG_CMD(a), u)) {
        return(CMD_FAIL);
    }
    if (XD_RUNNING != xd->xd_state) {
        cli_out("%s: TX not currently running\n", ARG_CMD(a));
        return(CMD_FAIL);
    } else {
        xd->xd_state = XD_STOP;
        cli_out("%s: TX termination requested\n", ARG_CMD(a));
        return(CMD_OK);
    }
}


/*
 * Function:
 * Purpose:
 * Parameters:
 * Returns:
 */
cmd_result_t
cmd_robo_tx(int u, args_t *a)
{
    xd_t *xd = &xd_units[u];
    volatile cmd_result_t rv;
    jmp_buf ctrl_c;

    if (!sh_check_attached(ARG_CMD(a), u)) {
        return(CMD_FAIL);
    } else if (xd->xd_state == XD_RUNNING) {
        cli_out("%s: Error: already active\n", ARG_CMD(a));
        return(CMD_FAIL);
    }

    if (CMD_OK != (rv = robo_tx_parse(u, a, xd))) {
        return(rv);
    }

#ifndef NO_CTRL_C
    if (!setjmp(ctrl_c)) {
#endif
        sh_push_ctrl_c(&ctrl_c);
        xd->xd_state = XD_RUNNING;  /* Say GO */
        rv = robo_do_tx(xd);
#ifndef NO_CTRL_C
    } else {
        rv = CMD_INTR;
    }
#endif

    sh_pop_ctrl_c();
    xd->xd_state = XD_IDLE;

    return(rv);
}

STATIC int rx_cb_count;

STATIC bcm_rx_t
robo_rx_cb_handler(int unit, bcm_pkt_t *info, void *cookie)
{
    int     count, i;
    char *_reason_names[bcmRxReasonCount] = BCM_RX_REASON_NAMES_INITIALIZER;
    COMPILER_REFERENCE(cookie);

    count = ++rx_cb_count;
    if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        cli_out("RX packet %d: unit=%d len=%d rx_port=%d rx_vport=%d cos=%d\n",
                count, unit, info->tot_len, info->rx_port,
                BCM_GPORT_SUBPORT_PORT_GET(info->src_gport),info->cos);
#endif
    } else {
        cli_out("RX packet %d: unit=%d len=%d rx_port=%d cos=%d\n",
                count, unit, info->tot_len, info->rx_port, info->cos);
    }

    for (i = 0; i < bcmRxReasonCount; i++) {
        if (BCM_RX_REASON_GET(info->rx_reasons, i)) {
            cli_out("RX reasons: %s\n", _reason_names[i]);
        }
    }

    if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        int rx_flag_print_cnt = 0;

        cli_out("RX flags: ");
        if (info->flags & BCM_RX_MIRRORED) {
            cli_out("Mirrored packet, ");
            rx_flag_print_cnt++;
        }
        if (info->flags & BCM_RX_LEARN_DISABLED) {
            cli_out("Learning disabled packet");
            rx_flag_print_cnt++;
        }
        if (!rx_flag_print_cnt) {
            cli_out("None");
        }
        cli_out("\n");
        if (info->multicast_group < 0) {
            cli_out("RX flow id: %d\n", info->flow_id);
        } else {
            cli_out("RX Multicast id: %d\n", info->multicast_group);
        }
        cli_out("RX Traffic Class: %d\n", info->prio_int);
        if (info->color == bcmColorPreserve) {
            cli_out("RX Color: Preserve\n");
        } else if (info->color == bcmColorGreen) {
            cli_out("RX Color: Green\n");
        } else if (info->color == bcmColorYellow) {
            cli_out("RX Color: Yellow\n");
        } else {
            cli_out("RX Color: Red\n");
        }
#endif
    }else if (SOC_IS_POLAR(unit)) {
#ifdef BCM_POLAR_SUPPORT
        cli_out("RX Traffic Class: %d\n", info->prio_int);    
#endif /* BCM_POLAR_SUPPORT */
    }
    cli_out("Parsed packet info:\n");
    cli_out("    src mod=%d. src port=%d. op=%d.\n", info->src_mod,
            info->src_port, info->opcode);
    cli_out("    dest mod=%d. dest port=%d. chan=%d.\n", info->dest_mod,
            info->dest_port, info->dma_channel);


    soc_eth_dma_dump_pkt(unit, "Data: ", BCM_PKT_DMAC(info), info->tot_len);

    return BCM_RX_HANDLED;
}

#define BASIC_PRIO 100

static bcm_rx_chan_cfg_t rx_chan_cfg = {
    31,                     /* DV count (number of chains) */
    0,                      /* Default pkt rate */
    0,                     /* No flags */
    0xff                   /* All COS to this channel */
};

/* Default data for configuring RX system */
static bcm_rx_cfg_t rx_cfg = {
    ROBO_RX_PKT_SIZE_DFLT,       /* packet alloc size */
    ROBO_RX_PPC_DFLT,            /* Packets per chain */
    0,                      /* Default pkt rate, global */
    1,                      /* Burst */
    {                       /* Just configure channel 1 */
        {                   /* Only Channel 0, default RX */
            ROBO_RX_CHAINS_DFLT, /* DV count (number of chains) */
            0,              /* Default pkt rate, DEPRECATED */
            0,              /* No flags */
            0xff            /* All COS to this channel */
        }
    },
    NULL,          /* alloc function */
    NULL,           /* free function */
    0                       /* flags */
};

static int free_buffers;

cmd_result_t
cmd_robo_rx_cfg(int unit, args_t *args)
/*
 * Function:    rx
 * Purpose:     Perform simple RX test
 * Parameters:  unit - unit number
 *              args - arguments
 * Returns:     CMD_XX
 */
{
    int chan;
    parse_table_t   pt;
    bcm_cos_queue_t queue_max;
    /* This isn't configurable per unit yet. */
    int cos_pps[BCM_RX_COS];
    uint8 cos_pps_init = 0;
    int rv = BCM_E_NONE;
    int i;
    int system_pps = 0;
    char  *subcmd;
    char *_reason_names[bcmRxReasonCount] = BCM_RX_REASON_NAMES_INITIALIZER;
    uint8 int_prio = 0;
    uint32 pkt_type = 0;
    bcm_rx_reasons_t reasons_q;
    bcm_cos_queue_t mapped_q = 0;

    memset(&reasons_q, 0, sizeof(bcm_rx_reasons_t));

    if (!cos_pps_init) {
        for (i = 0; i < BCM_RX_COS; i++) {
            cos_pps[i] = 100;
        }
        cos_pps_init = 1;
    }

    if (!sh_check_attached(ARG_CMD(args), unit)) {
        return(CMD_FAIL);
    }

    if (BCM_FAILURE(bcm_rx_queue_max_get(unit, &queue_max))) {
        return (CMD_FAIL);
    }

    if (!ARG_CUR(args)) {
        int spps;
        bcm_rx_reasons_t reasons;
        
        /* Display current configuration */
        cli_out("Current RX configuration:\n");
        cli_out("    Pkt Size %d. Pkts/chain %d. Global PPS %d. Burst %d\n",
                rx_cfg.pkt_size, rx_cfg.pkts_per_chain,
                rx_cfg.global_pps, rx_cfg.max_burst);
         /*cli_out("    Weights cos 0 to 7:      %d %d %d %d    %d %d %d %d\n",
                   rx_cfg.cos_wt[0], rx_cfg.cos_wt[1], rx_cfg.cos_wt[2],
                   rx_cfg.cos_wt[3], rx_cfg.cos_wt[4], rx_cfg.cos_wt[5],
                   rx_cfg.cos_wt[6], rx_cfg.cos_wt[7]); */
        for (chan = 0; chan < BCM_ROBO_RX_CHANNELS; chan++) {
            cli_out("    Channel %d:  Chains %d. PPS %d.\n",
                    chan, rx_cfg.chan_cfg[chan].chains,
                    rx_cfg.chan_cfg[chan].rate_pps
                    );
        }
        if ((rv = bcm_rx_cpu_rate_get(unit, &spps)) < 0) {
            cli_out("ERROR getting system rate limit:  %s\n",
                    bcm_errmsg(rv));
        } else {
            cli_out("    System wide rate limit:  %d\n", spps);
        }

        cli_out("\nSupported RX reason codes: ");
        rv = bcm_rx_reasons_get(unit, &reasons);
        if (rv < 0) {
            cli_out("ERROR:  %s\n", bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("\n");
        for (i = 0; i < bcmRxReasonCount; i++) {
            if (BCM_RX_REASON_GET(reasons, i)) {
                cli_out("%s, ", _reason_names[i]);
            }
        }
        cli_out("\n");
        cli_out("\nSupported CosQ mapping reason codes: ");
        rv = bcm_rx_cosq_mapping_reasons_get(unit, &reasons);
        if (rv < 0) {
            cli_out("ERROR:  %s\n", bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("(with mapped CosQ number)\n");
        for (i = 0; i < bcmRxReasonCount; i++) {
            if (BCM_RX_REASON_GET(reasons, i)) {
                BCM_RX_REASON_CLEAR_ALL(reasons_q);
                BCM_RX_REASON_SET(reasons_q, i);
                rv = bcm_rx_cosq_mapping_get
                    (unit, 0, &reasons_q, &reasons_q, &int_prio, &int_prio, 
                    &pkt_type, &pkt_type, &mapped_q);
                if (rv < 0) {
                    cli_out("ERROR:  Get Rx mapped cosq number failed: %s\n", bcm_errmsg(rv));
                    return CMD_FAIL;
                }
                cli_out("%s(%d), ", _reason_names[i], mapped_q);
            }
        }
        /* Get the cosq number of supported packet type */
        cli_out("\n");
        cli_out("\nSupported CosQ mapping packet types: ");
        cli_out("(with mapped CosQ number)\n");
        BCM_RX_REASON_CLEAR_ALL(reasons_q);
        pkt_type = BCM_RX_COSQ_PACKET_TYPE_SWITCHED;
        rv = bcm_rx_cosq_mapping_get
            (unit, 0, &reasons_q, &reasons_q, &int_prio, &int_prio, 
            &pkt_type, &pkt_type, &mapped_q);
        if (rv < 0) {
            cli_out("ERROR:  Get Rx mapped cosq number failed: %s\n", bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("%s(%d), ", "Switched", mapped_q);
        pkt_type = BCM_RX_COSQ_PACKET_TYPE_MIRROR;
        rv = bcm_rx_cosq_mapping_get
            (unit, 0, &reasons_q, &reasons_q, &int_prio, &int_prio, 
            &pkt_type, &pkt_type, &mapped_q);
        if (rv < 0) {
            cli_out("ERROR:  Get Rx mapped cosq number failed: %s\n", bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("%s(%d), ", "Mirrored", mapped_q);

        cli_out("\n");

        return CMD_OK;
    }

    /* Rx reason code and cosq mapping */
    subcmd = ARG_CUR(args);
    if (sal_strcasecmp(subcmd, "reason") == 0) {
        uint32 matched;
        char *value;
        char *pkt_type_name[] = {"Switched", "Non-unicast", "Mirror", NULL};
        uint32 pkt_type_idx = 0;
        int process_reason_code = 0;

        ARG_NEXT(args);
        subcmd = ARG_GET(args);
        value = ARG_GET(args);

        matched = 0;

        /* Process reason codes */
        for (i = 0; i < bcmRxReasonCount; i++) {
            process_reason_code = 0;
            if (subcmd == NULL) {
                process_reason_code = 1;
            } else {
                if (sal_strcasecmp(subcmd, _reason_names[i]) == 0) {
                    process_reason_code = 1;
                }
            }
            if (process_reason_code) {
                matched += 1;
                BCM_RX_REASON_CLEAR_ALL(reasons_q);
                BCM_RX_REASON_SET(reasons_q, i);
                if (value == NULL) {
                    rv = bcm_rx_cosq_mapping_get
                        (unit, 0, &reasons_q, &reasons_q, &int_prio, &int_prio, 
                        &pkt_type, &pkt_type, &mapped_q);
                    if (rv < 0) {
                        cli_out("Get Reason code %s mapped CosQ failed: %s\n", 
                                _reason_names[i], bcm_errmsg(rv));
                        return CMD_FAIL;
                    }
                    cli_out("Get Reason code %s mapped CosQ %d\n", 
                            _reason_names[i], mapped_q);
                } else {
                    mapped_q = parse_integer(value);
                    rv = bcm_rx_cosq_mapping_set
                        (unit, 0, reasons_q, reasons_q, int_prio, int_prio, 
                        pkt_type, pkt_type, mapped_q);
                    if (rv < 0) {
                        cli_out("Set CosQ %d mapped Reason code %s failed: %s\n", 
                                mapped_q,  _reason_names[i], bcm_errmsg(rv));
                        return CMD_FAIL;
                    }
                }
            }
        }

        /* Process pacekt types */
        if (subcmd == NULL) {
            pkt_type = 0;
        } else {
            BCM_RX_REASON_CLEAR_ALL(reasons_q);
            if (sal_strcasecmp(subcmd, "Switched") == 0) {
                pkt_type = BCM_RX_COSQ_PACKET_TYPE_SWITCHED;
                pkt_type_idx = 0;
            } else if (sal_strcasecmp(subcmd, "NonUnicast") == 0) {
                pkt_type = BCM_RX_COSQ_PACKET_TYPE_NON_UNICAST;
                pkt_type_idx = 1;
            } else if (sal_strcasecmp(subcmd, "Mirror") == 0) {
                pkt_type = BCM_RX_COSQ_PACKET_TYPE_MIRROR;
                pkt_type_idx = 2;
            } else {
                pkt_type = 0;
            }
        }

        if (pkt_type != 0) {
            matched += 1;
            if (value == NULL) {
                rv = bcm_rx_cosq_mapping_get
                    (unit, 0, &reasons_q, &reasons_q, &int_prio, &int_prio, 
                    &pkt_type, &pkt_type, &mapped_q);
                if (rv < 0) {
                    cli_out("Get Packet type %s mapped CosQ failed: %s\n", 
                            pkt_type_name[pkt_type_idx], bcm_errmsg(rv));
                    return CMD_FAIL;
                }
                cli_out("Get Packet type %s mapped CosQ %d\n", 
                        pkt_type_name[pkt_type_idx], mapped_q);
            } else {
                mapped_q = parse_integer(value);
                rv = bcm_rx_cosq_mapping_set
                    (unit, 0, reasons_q, reasons_q, int_prio, int_prio, 
                    pkt_type, pkt_type, mapped_q);
                if (rv < 0) {
                    cli_out("Set CosQ %d mapped Packet type %s failed: %s\n", 
                            mapped_q,  pkt_type_name[pkt_type_idx], bcm_errmsg(rv));
                    return CMD_FAIL;
                }
            }
        }

        if (matched == 0) {
            cli_out("%s: ERROR: Unknown reason code or packet type\n", subcmd);
            return CMD_FAIL;
        }
        return CMD_OK;
    }

    if (isint(ARG_CUR(args))) {
        chan = parse_integer(ARG_GET(args));
        if (chan < 0 || chan >= BCM_ROBO_RX_CHANNELS) {
            cli_out("Error: Bad channel %d\n", chan);
            return CMD_FAIL;
        }
    } else {
        chan = -1;
    }

    parse_table_init(unit, &pt);

    parse_table_add(&pt, "SPPS", PQ_DFL|PQ_INT, 0,
                    &system_pps, NULL);
    parse_table_add(&pt, "GPPS", PQ_DFL|PQ_INT, 0,
                    &rx_cfg.global_pps, NULL);
    parse_table_add(&pt, "PKTSIZE", PQ_DFL|PQ_INT, 0,
                    &rx_cfg.pkt_size, NULL);
    parse_table_add(&pt, "PPC", PQ_DFL|PQ_INT, 0,
                    &rx_cfg.pkts_per_chain, NULL);
    parse_table_add(&pt, "BURST", PQ_DFL|PQ_INT, 0,
                    &rx_cfg.max_burst, NULL);
    parse_table_add(&pt, "FREE", PQ_DFL|PQ_BOOL, 0,
                    &free_buffers, NULL);

    for (i = 0; i <= queue_max; i++) {
        char cospps_str[20];

        sal_sprintf(cospps_str, "COSPPS%d", i);
        parse_table_add(&pt, cospps_str, PQ_DFL|PQ_INT, 0,
                        &cos_pps[i], NULL);
    }

    if (chan >= 0) {
        parse_table_add(&pt, "CHAINS", PQ_DFL|PQ_INT, 0,
                        &rx_chan_cfg.chains, NULL);
        parse_table_add(&pt, "PPS", PQ_DFL|PQ_INT, 0,
                        &rx_chan_cfg.rate_pps, NULL);
    }

    /* Parse remaining arguments */
    if (0 > parse_arg_eq(args, &pt)) {
        cli_out("%s: Error: Invalid option or malformed expression: %s\n",
                ARG_CMD(args), ARG_CUR(args));
        parse_arg_eq_done(&pt);
        return(CMD_FAIL);
    }

    /* Check if SPPS was entered; if so do only that */
    if (pt.pt_entries[0].pq_type & PQ_PARSED) {
        rv = bcm_rx_cpu_rate_set(unit, system_pps);
        parse_arg_eq_done(&pt);
        if (rv < 0) {
            cli_out("Warning:  system rate set (to %d) returned %s\n",
                    system_pps, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;
    }
    
    parse_arg_eq_done(&pt);

    for (i = 0; i <= queue_max; i++) {
        rv = bcm_rx_cos_rate_set(unit, i, cos_pps[i]);
        if (rv < 0) {
            cli_out("Warning:  cos rate set(%d to %d) returned %s\n", i,
                    cos_pps[i], bcm_errmsg(rv));
        }
    }


    if (chan >=0 ) { /* Copy external chan cfg to right place */
        sal_memcpy(&rx_cfg.chan_cfg[chan], &rx_chan_cfg,
                   sizeof(bcm_rx_chan_cfg_t));
    }
    return CMD_OK;
}

STATIC int
_robo_init_rx_api(int unit)
{
    int r;

    if (bcm_rx_active(unit)) {
        cli_out("RX is already running\n");
        return -1;
    }

    if (pw_running(unit)) {
        cli_out("rxmon: Error: Cannot start RX with packetwatcher running\n");
        return -1;
    }

    if ((r = bcm_rx_start(unit, &rx_cfg)) < 0) {
        cli_out("rxmon: Error: Cannot start RX: %s.\n", bcm_errmsg(r));
        return -1;
    }

    return 0;
}

cmd_result_t
cmd_robo_rx_mon(int unit, args_t *args)
/*
 * Function:    rx
 * Purpose:     Perform simple RX test
 * Parameters:  unit - unit number
 *              args - arguments
 * Returns:     CMD_XX
 */
{
    char                *c;
    uint32              active;
    int                 r;
    int rv = CMD_OK;

    if (!sh_check_attached(ARG_CMD(args), unit)) {
        return(CMD_FAIL);
    }

    bcm_rx_channels_running(unit,&active);

    c = ARG_GET(args);
    if (c == NULL) {
        cli_out("Active bitmap for RX is %x.\n", active);
        return CMD_OK;
    }

    if (sal_strcasecmp(c, "init") == 0) {
        if (_robo_init_rx_api(unit) < 0) {
            return CMD_FAIL;
        } else {
            return CMD_OK;
        }
    } else if (sal_strcasecmp(c, "start") == 0) {
        rx_cb_count = 0;

        if (!bcm_rx_active(unit)) { /* Try to initialize */
            if (_robo_init_rx_api(unit) < 0) {
                cli_out("Warning:  init failed.  Will attempt register\n");
            }
        }

        /* Register to accept all cos */
        if ((r = bcm_rx_register(unit, "RX CMD", robo_rx_cb_handler,
                    BASIC_PRIO, NULL, BCM_RCO_F_ALL_COS)) < 0) {
            cli_out("%s: bcm_rx_register failed: %s\n",
                    ARG_CMD(args), bcm_errmsg(r));
            return CMD_FAIL;
        }

    } else if (sal_strcasecmp(c, "stop") == 0) {
        if ((r = bcm_rx_stop(unit, &rx_cfg)) < 0) {
            cli_out("%s: Error: Cannot stop RX: %s.  Is it running?\n",
                    ARG_CMD(args), bcm_errmsg(r));
            return CMD_FAIL;
        }
        /* Unregister handler */
        if ((r = bcm_rx_unregister(unit, robo_rx_cb_handler, BASIC_PRIO)) < 0) {
            cli_out("%s: bcm_rx_unregister failed: %s\n",
                    ARG_CMD(args), bcm_errmsg(r));
            return CMD_FAIL;
        }

    } else if (sal_strcasecmp(c, "show") == 0) {
#ifdef  BROADCOM_DEBUG
        bcm_rx_show(unit);
#else
    cli_out("%s: ERROR: cannot show in non-BROADCOM_DEBUG compilation\n",
            ARG_CMD(args));
    return CMD_FAIL;
#endif  /* BROADCOM_DEBUG */
    } else {
        return CMD_USAGE;
    }

    return rv;
}

/****************************************************************
 *
 * RX commands
 *
 ****************************************************************/
cmd_result_t
cmd_robo_rx_init(int unit, args_t *args)
{
    char *ch;
    int override_unit;
    int rv;

    if ((ch = ARG_GET(args)) == NULL) {
        cli_out("RXINIT requires unit specification\n");
        return CMD_USAGE;
    }

    override_unit = strtoul(ch, NULL, 0);
    rv = bcm_rx_init(override_unit);

    if (rv < 0) {
        cli_out("ERROR:  bcm_rx_init(%d) returns %d: %s\n",
                override_unit, rv, bcm_errmsg(rv));
        return CMD_FAIL;
    }
    return CMD_OK;
}


char robo_cmd_hdr_mode_usage[] =
    "Parameters: [mode]\n"
    "  With no mode, shows packet header mode of current unit.\n"
    "  Although the header mode may be set for any device, \n"
    "  it only has an effect for BCM5670 and related devices\n"
    "  Supported modes are:\n"
    "      Ieee   --  no encapusulation\n"
    "      Higig  --  Use parsed HiGig header information\n"
#ifdef BCM_HIGIG2_SUPPORT
    "      Higig2 --  Use parsed HiGig2 header information\n"
#endif /* BCM_HIGIG2_SUPPORT */
    "      B5632  --  Use parsed BCM5632 header information\n"
    "      Raw    --  Use 3 words of raw data for the header\n";

cmd_result_t
robo_cmd_hdr_mode(int unit, args_t *args)
/*
 * Function:
 * Purpose:
 * Parameters:
 * Returns:
 */
{
    xd_t *xd = &xd_units[unit];
    char *arg;
    int i = -1;

    if (!sh_check_attached(ARG_CMD(args), unit)) {
        return(CMD_FAIL);
    }

    _XD_INIT(unit, xd);

    if ((arg = ARG_GET(args)) != NULL) {
        for (i = 0; i < 1; i++) {
            if (!sal_strcasecmp(arg, hdr_modes[i])) {
                xd->hdr_mode = i;
                break;
            }
        }
    }

    if (xd->hdr_mode < 0 || xd->hdr_mode > 0) {
        cli_out("Current header mode for unit %d is invalid (%d).\n",
                unit, xd->hdr_mode);
    } else {
        cli_out("Current header mode for unit %d is%s %s (%d).\n",
                unit, (i != -1) ? " now" : "",
                hdr_modes[xd->hdr_mode], xd->hdr_mode);
    }

    return (i < 1) ? CMD_OK : CMD_USAGE;
}

