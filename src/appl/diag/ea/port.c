/*
 * $Id: port.c,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        port.c
 * Purpose:     Functions to support CLI port commands
 * Requires:
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <sal/types.h>
#include <sal/appl/sal.h>
#include <sal/appl/io.h>
#include <sal/core/libc.h>

#include <assert.h>

#include <soc/debug.h>
#include <soc/phy.h>
#include <soc/phyctrl.h>

#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <appl/diag/diag.h>
#include <appl/diag/dport.h>

#include <bcm/init.h>
#include <bcm/port.h>
#include <bcm/stg.h>
#include <bcm/error.h>
#include <bcm/link.h>
#include <bcm/stat.h>
#include <bcm/cosq.h>

#ifdef BCM_TK371X_SUPPORT

/*
 * Function:
 *  _if_fmt_speed
 * Purpose:
 *  Format a speed as returned from bcm_xxx for display.
 * Parameters:
 *  b     - buffer to format into.
 *  speed - speed as returned from bcm_port_speed_get
 * Returns:
 *  Pointer to buffer (b).
 */
static char
*tk371x_if_fmt_speed(char *b, int speed)
{
    if (speed >= 1000) {                /* Use Gb */
        if (speed % 1000) {             /* Use Decimal */
            sal_sprintf(b, "%d.%dG", speed / 1000, (speed % 1000) / 100);
        } else {
            sal_sprintf(b, "%dG", speed / 1000);
        }
    } else if (speed == 0) {
        sal_sprintf(b, "-");
    } else {                            /* Use Mb */
        sal_sprintf(b, "%dM", speed);
    }
    return(b);
}

/*
 * These are ordered according the corresponding enumerated types.
 * See soc/portmode.h, bcm/port.h and bcm/link.h for more information
 */

/* Note:  See port.h, bcm_port_discard_e */
char *tk371x_discard_mode[] = {
    "None",
    "All",
    "Untag",
    "Tag",
    NULL
};
/* Note:  See link.h, bcm_linkscan_mode_e */
char *tk371x_linkscan_mode[] = {
    "None",
    "SW",
    "HW",
    "OFF",
    "ON",
    NULL
};
/* Note:  See portmode.h, soc_port_if_e */
char *tk371x_interface_mode[] = SOC_PORT_IF_NAMES_INITIALIZER;

/* Note:  See portmode.h, soc_port_ms_e */
char *tk371x_phymaster_mode[] = {
    "Slave",
    "Master",
    "Auto",
    "None",
    NULL
};
/* Note:  See port.h, bcm_port_loopback_e */
char *tk371x_loopback_mode[] = {
    "NONE",
    "MAC",
    "PHY",
    NULL
};
/* Note:  See port.h, bcm_port_stp_e */
char *tk371x_forward_mode[] = {
    "Disable",
    "Block",
    "LIsten",
    "LEarn",
    "Forward",
    NULL
};
/* Note: See port.h, bcm_port_encap_e */
char *tk371x_encap_mode[] = {
    "IEEE",
    "HIGIG",
    "B5632",
    "Reserved",
    NULL
};
/* Note: See port.h, bcm_port_medium_t */
char *tk371x_medium_status[] = {
    "None",
    "Copper",
    "Fiber",
    NULL
};

extern int _bcm_tk371x_port_selective_get(int unit, bcm_port_t port, bcm_port_info_t *info);

/* Invalid unit number ( < 0) is permitted */
void
tk371x_port_info_init(int unit, int port, bcm_port_info_t *info, uint32 actions)
{
    info->action_mask = actions;

    /* We generally need to get link state and autoneg and adverts */
    info->action_mask |= BCM_PORT_ATTR_LINKSTAT_MASK;
    info->action_mask |= BCM_PORT_ATTR_LOCAL_ADVERT_MASK;
    info->action_mask |= BCM_PORT_ATTR_REMOTE_ADVERT_MASK;
    info->action_mask |= BCM_PORT_ATTR_AUTONEG_MASK;
}

void
tk371x_brief_port_info_header(int unit)
{
    const char * const disp_str =
    "%4s  "          /* port number */
    "%3s "           /* enabled */
    "%4s    "        /* link state */
    "%6s  "          /* speed/duplex */
    "%4s  "          /* link scan mode */
    "%4s "           /* auto negotiate? */
    "%7s   "         /* spantree state */
    "%5s  "          /* pause tx/rx */
    "%6s  "          /* discard mode */
    "%3s  "          /* learn to CPU, ARL, FWD or discard */
    "%5s\n";         /* interface */

    cli_out(disp_str,
            " ",          /* port number */
            " ",          /* enabled */
            " ",          /* link state */
            "speed/",     /* speed/duplex */
            "link",       /* link scan mode */
            "auto",       /* auto negotiate? */
            " STP ",      /* spantree state */
            " ",          /* pause tx/rx */
            " ",          /* discard mode */
            "lrn",        /* learn to CPU, ARL, FWD or discard */
            "inter");     /* interface */
    cli_out(disp_str,
            "port",       /* port number */
            "Ena",        /* Enabled */
            "link",       /* link state */
            "duplex",     /* speed/duplex */
            "scan",       /* link scan mode */
            "neg?",       /* auto negotiate? */
            "state",      /* spantree state */
            "pause",      /* pause tx/rx */
            "discrd",     /* discard mode */
            "ops",        /* learn to CPU, ARL, FWD or discard */
            "face");      /* interface */
}
#define _CHECK_PRINT(flags, mask, str, val) do {                \
        if ((flags) & (mask)) {                                 \
            cli_out(\
                    str, val);                               \
        } else {                                                \
            cli_out(\
                    str, "");                                \
        }                                                       \
    } while (0)

int
tk371x_brief_port_info(char *port_ref, bcm_port_info_t *info, uint32 flags)
{
    char        *spt_str, *discrd_str;
    char        sbuf[6];
    char        lrn_str[4];

    spt_str = tk371x_forward_mode[info->stp_state];
    discrd_str = tk371x_discard_mode[info->discard];

    /* port number (4)
     * enabled (3)
     * link state (3)
     * speed/duplex (6)
     * link scan mode (4)
     * auto negotiate? (4)
     * spantree state (7)
     * pause tx/rx (5)
     * discard mode (6)
     * learn to CPU, ARL, FWD or discard (3)
     */
    cli_out("%4s  %3s %4s  ", port_ref,
            info->enable ? "En" : "DIS",
            info->linkstatus ? " up " : "down");
    _CHECK_PRINT(flags, BCM_PORT_ATTR_SPEED_MASK,
                 "%5s ", tk371x_if_fmt_speed(sbuf, info->speed));
    _CHECK_PRINT(flags, BCM_PORT_ATTR_DUPLEX_MASK,
                 "%2s ",  info->speed == 0 ? "" : info->duplex ? "FD" : "HD");
    _CHECK_PRINT(flags, BCM_PORT_ATTR_LINKSCAN_MASK,
                 " %4s  ", tk371x_linkscan_mode[info->linkscan]);
    _CHECK_PRINT(flags, BCM_PORT_ATTR_AUTONEG_MASK,
                 "%4s  ", info->autoneg ? " Yes" : " No ");
    _CHECK_PRINT(flags, BCM_PORT_ATTR_STP_STATE_MASK,
                 "%7s  ", spt_str);
    _CHECK_PRINT(flags, BCM_PORT_ATTR_PAUSE_MASK,
                 "%2s ", info->pause_tx ? "TX" : " ");
    _CHECK_PRINT(flags, BCM_PORT_ATTR_PAUSE_MASK,
                 "%2s ", info->pause_rx ? "RX" : " ");
    _CHECK_PRINT(flags, BCM_PORT_ATTR_DISCARD_MASK,
                 "%6s   ", discrd_str);

    sal_memset(lrn_str, 0, sizeof(lrn_str));
    /*
     * BCM5324M does not support L2 learn and trap function.
     * It's forced at learn and forward states.
     */
    lrn_str[0] = 'F';
    lrn_str[1] = 'A';
    lrn_str[2] = 'C';
    _CHECK_PRINT(flags, BCM_PORT_ATTR_LEARN_MASK,
                 "%3s ", lrn_str);
    if (info->interface >= 0 && info->interface < SOC_PORT_IF_COUNT) {
        /* coverity[overrun-local] */
        _CHECK_PRINT(flags, BCM_PORT_ATTR_INTERFACE_MASK,
                     "%6s  ", tk371x_interface_mode[info->interface]);
    }

    cli_out("\n");
    return 0;
}


/*
 * Function:
 *  disp_port_info
 * Purpose:
 *  Display selected port information
 * Parameters:
 *  info        - pointer to structure with info to display
 *  port_ref    - Port reference to print
 *  hg_port     - Is the port a hi-gig port?
 * Returns:
 *  Nothing
 * Notes:
 *  Assumes link status info always valid
 */

void
tk371x_disp_port_info(char *port_ref, bcm_port_info_t *info, int hg_port,
               uint32 flags)
{
    char            speed_buf[6];
    bcm_port_abil_t     mode;
    int         r;
    int         no_an_props = 0;   /* Do not show AN props */

    /* Assume link status always available. */
    cli_out(" %c%s ", info->linkstatus ? '*' : ' ', port_ref);

    if (flags & BCM_PORT_ATTR_ENABLE_MASK) {
        cli_out("%s", info->enable ? "" : "DISABLED ");
    }

    if (flags & BCM_PORT_ATTR_LINKSCAN_MASK) {
        if (info->linkscan) {
            cli_out("LS(%s) ", tk371x_linkscan_mode[info->linkscan]);
        }
    }

    if (hg_port) {
        cli_out("%s/XAUI(", tk371x_encap_mode[info->encap_mode & 0x3]);
    } else if (flags & BCM_PORT_ATTR_AUTONEG_MASK) {
        if (info->autoneg) {
            if (!info->linkstatus) {
                cli_out("Auto(no link) ");
                no_an_props = 1;
            } else {
                cli_out("Auto(");
            }
        } else {
            cli_out("Forced(");
        }
    }

    /* If AN is enabled, but not complete, don't show port settings */
    if (!no_an_props) {
        if (flags & BCM_PORT_ATTR_SPEED_MASK) {
            cli_out("%s", tk371x_if_fmt_speed(speed_buf, info->speed));
        }
        if (flags & BCM_PORT_ATTR_DUPLEX_MASK) {
            cli_out("%s", info->duplex ? "FD" : "HD");
        }
        if (flags & BCM_PORT_ATTR_PAUSE_MASK) {
            cli_out("%s%s", info->pause_tx ? "+TXpau" : "",
                    info->pause_rx ? "+RXpau" : "");
        }
        cli_out(") ");
    }

    if (flags & BCM_PORT_ATTR_AUTONEG_MASK) {
        if (info->autoneg) {
            if (flags & BCM_PORT_ATTR_LOCAL_ADVERT_MASK) {
                mode = info->local_advert;
                cli_out("Adv(%s%s%s%s) ",
                        tk371x_if_fmt_speed(speed_buf, BCM_PORT_ABIL_SPD_MAX(mode)),
                        (mode & BCM_PORT_ABIL_FD) ? "FD" : "HD",
                        (mode & BCM_PORT_ABIL_PAUSE_TX) ? "+TXpau" : "",
                        (mode & BCM_PORT_ABIL_PAUSE_RX) ? "+RXpau" : "");
            }

            if ((flags & BCM_PORT_ATTR_REMOTE_ADVERT_MASK) &&
                (info->remote_advert_valid && info->linkstatus)) {
                mode = info->remote_advert;

                cli_out("PeerAdv(%s%s%s%s) ",
                        tk371x_if_fmt_speed(speed_buf, BCM_PORT_ABIL_SPD_MAX(mode)),
                        (mode & BCM_PORT_ABIL_FD) ? "FD" : "HD",
                        (mode & BCM_PORT_ABIL_PAUSE_TX) ? "+TXpau" : "",
                        (mode & BCM_PORT_ABIL_PAUSE_RX) ? "+RXpau" : "");
            }
        }
    }

    if (flags & BCM_PORT_ATTR_PAUSE_MAC_MASK) {
        cli_out("Stad(%02x:%02x:%02x:%02x:%02x:%02x) ",
                info->pause_mac[0], info->pause_mac[1],
                info->pause_mac[2], info->pause_mac[3],
                info->pause_mac[4], info->pause_mac[5]);
    }

    if (flags & BCM_PORT_ATTR_STP_STATE_MASK) {
        cli_out("STP(%s) ", tk371x_forward_mode[info->stp_state]);
    }

    if (!hg_port) {
        if (flags & BCM_PORT_ATTR_DISCARD_MASK) {
            switch (info->discard) {
                case BCM_PORT_DISCARD_NONE:
                    break;
                case BCM_PORT_DISCARD_ALL:
                    cli_out("Disc(all) ");
                    break;
                case BCM_PORT_DISCARD_UNTAG:
                    cli_out("Disc(untagged) ");
                    break;
                case BCM_PORT_DISCARD_TAG:
                    cli_out("Disc(tagged) ");
                    break;
                default:
                    cli_out("Disc(?) ");
                    break;
            }
        }

        if (flags & BCM_PORT_ATTR_LEARN_MASK) {
            cli_out("Lrn(");

            r = 0;

            if (info->learn & BCM_PORT_LEARN_ARL) {
                cli_out("ARL");
                r = 1;
            }

            if (info->learn & BCM_PORT_LEARN_CPU) {
                cli_out("%sCPU", r ? "," : "");
                r = 1;
            }

            if (info->learn & BCM_PORT_LEARN_FWD) {
                cli_out("%sFWD", r ? "," : "");
                r = 1;
            }

            if (!r) {
                cli_out("disc");
            }
            cli_out(") ");
        }

        if (flags & BCM_PORT_ATTR_UNTAG_PRI_MASK) {
            cli_out("UtPri(");

            if (info->untagged_priority < 0) {
                cli_out("off");
            } else {
                cli_out("%d", info->untagged_priority);
            }
            cli_out(") ");
        }

        if (flags & BCM_PORT_ATTR_PFM_MASK) {
            cli_out("Pfm(%c) ",
                    info->pfm == BCM_PORT_PFM_MODEA ? 'A' :
                    info->pfm == BCM_PORT_PFM_MODEB ? 'B' :
                    info->pfm == BCM_PORT_PFM_MODEC ? 'C' : '?');
        }
    } /* !hg_port */

    if (flags & BCM_PORT_ATTR_INTERFACE_MASK) {
        if (info->interface >= 0 && info->interface < SOC_PORT_IF_COUNT) {
            /* coverity[overrun-local] */
            cli_out("IF(%s) ", tk371x_interface_mode[info->interface]);
        }
    }

    if (flags & BCM_PORT_ATTR_PHY_MASTER_MASK) {
        if (info->phy_master >= 0 &&
        info->phy_master < SOC_PORT_MS_COUNT &&
        info->phy_master != SOC_PORT_MS_NONE) {
            cli_out("PH(%s) ", tk371x_phymaster_mode[info->phy_master]);
        }
    }

    if (flags & BCM_PORT_ATTR_LOOPBACK_MASK) {
        if (info->loopback == BCM_PORT_LOOPBACK_PHY) {
            cli_out("LB(phy)");
        } else if (info->loopback == BCM_PORT_LOOPBACK_MAC) {
            cli_out("LB(mac)");
        }
    }

    if (flags & BCM_PORT_ATTR_FRAME_MAX_MASK) {
        cli_out("MaxFrmSz(%d) ", info->frame_max);
    }

    cli_out("\n");
}

/* This maps the above list to the masks for the proper attributes
 * Note that the order of this attribute map should match that of
 * the parse-table entry/creation below.
 */
static int port_attr_map[] = {
    BCM_PORT_ATTR_ENABLE_MASK,       /* Enable */
    BCM_PORT_ATTR_AUTONEG_MASK,      /* AutoNeg */
    BCM_PORT_ATTR_SPEED_MASK,        /* SPeed */
    BCM_PORT_ATTR_DUPLEX_MASK,       /* FullDuplex */
    BCM_PORT_ATTR_LINKSCAN_MASK,     /* LinkScan */
    BCM_PORT_ATTR_LEARN_MASK,        /* LeaRN */
    BCM_PORT_ATTR_DISCARD_MASK,      /* DISCard */
    BCM_PORT_ATTR_VLANFILTER_MASK,   /* VlanFilter */
    BCM_PORT_ATTR_UNTAG_PRI_MASK,    /* PRIOrity */
    BCM_PORT_ATTR_PFM_MASK,          /* PortFilterMode */
    BCM_PORT_ATTR_PHY_MASTER_MASK,   /* PHymaster */
    BCM_PORT_ATTR_INTERFACE_MASK,    /* InterFace */
    BCM_PORT_ATTR_LOOPBACK_MASK,     /* LoopBack */
    BCM_PORT_ATTR_STP_STATE_MASK,    /* SpanningTreeProtocol */
    BCM_PORT_ATTR_PAUSE_MAC_MASK,    /* STationADdress */
    BCM_PORT_ATTR_PAUSE_TX_MASK,     /* TxPAUse */
    BCM_PORT_ATTR_PAUSE_RX_MASK,     /* RxPAUse */
    BCM_PORT_ATTR_ENCAP_MASK,        /* Port encapsulation mode */
    BCM_PORT_ATTR_FRAME_MAX_MASK     /* Maximum frame size */
};

/*
 * Function:
 *  tk371x_port_parse_setup
 * Purpose:
 *  Setup the parse table for a port command
 * Parameters:
 *  pt  - the table
 *  info    - port info structure to hold parse results
 * Returns:
 *  Nothing
 */

void
tk371x_port_parse_setup(int unit, parse_table_t *pt, bcm_port_info_t *info)
{                                      

    /*
     * NOTE: ENTRIES IN THIS TABLE ARE POSITION-DEPENDENT!
     * See references to PQ_PARSED below.
     */
    parse_table_init(unit, pt);
    parse_table_add(pt, "Enable", PQ_BOOL | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->enable, 0);
    parse_table_add(pt, "AutoNeg", PQ_BOOL | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->autoneg, 0);
    parse_table_add(pt, "SPeed",       PQ_INT | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->speed, 0);
    parse_table_add(pt, "FullDuplex",  PQ_BOOL | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->duplex, 0);
    parse_table_add(pt, "LinkScan",    PQ_MULTI | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->linkscan, tk371x_linkscan_mode);
    parse_table_add(pt, "LeaRN",   PQ_INT | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->learn, 0);
    parse_table_add(pt, "DISCard", PQ_MULTI | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->discard, tk371x_discard_mode);
    parse_table_add(pt, "VlanFilter", PQ_BOOL | PQ_NO_EQ_OPT,
                    0, &info->vlanfilter, 0);
    parse_table_add(pt, "PRIOrity", PQ_INT | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->untagged_priority, 0);
    parse_table_add(pt, "PortFilterMode", PQ_INT | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->pfm, 0);
    parse_table_add(pt, "PHymaster", PQ_MULTI | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->phy_master, tk371x_phymaster_mode);
    parse_table_add(pt, "InterFace", PQ_MULTI | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->interface, tk371x_interface_mode);
    parse_table_add(pt, "LoopBack", PQ_MULTI | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->loopback, tk371x_loopback_mode);
    parse_table_add(pt, "SpanningTreeProtocol",
                    PQ_MULTI | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->stp_state, tk371x_forward_mode);
    parse_table_add(pt, "STationADdress", PQ_MAC | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->pause_mac, 0);
    parse_table_add(pt, "TxPAUse",     PQ_BOOL | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->pause_tx, 0);
    parse_table_add(pt, "RxPAUse",     PQ_BOOL | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->pause_rx, 0);
    parse_table_add(pt, "ENCapsulation", PQ_MULTI | PQ_DFL,
                    0, &info->encap_mode, tk371x_encap_mode);
    parse_table_add(pt, "FrameMaxsize",       PQ_INT | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->frame_max, 0);         
					

}


/*
 * Function:
 *  tk371x_port_parse_mask_get
 * Purpose:
 *  Given PT has been parsed, set seen and parsed flags
 * Parameters:
 *  pt  - the table
 *  seen    - which parameters occurred w/o =
 *  parsed  - which parameters occurred w =
 * Returns:
 *  Nothing
 */

void
tk371x_port_parse_mask_get(parse_table_t *pt, uint32 *seen, uint32 *parsed)
{
    uint32      were_parsed = 0;
    uint32      were_seen = 0;
    int         i;

    /* Check that either all parameters are parsed or are seen (no =) */

    for (i = 0; i < pt->pt_cnt; i++) {
        if (pt->pt_entries[i].pq_type & PQ_SEEN) {
            were_seen |= port_attr_map[i];
        }

        if (pt->pt_entries[i].pq_type & PQ_PARSED) {
            were_parsed |= port_attr_map[i];
        }
    }

    if (were_seen &
        (BCM_PORT_ATTR_PAUSE_RX_MASK | BCM_PORT_ATTR_PAUSE_TX_MASK)) {
        were_seen |= BCM_PORT_ATTR_PAUSE_MASK;
    }

    if (were_parsed &
        (BCM_PORT_ATTR_PAUSE_RX_MASK | BCM_PORT_ATTR_PAUSE_TX_MASK)) {
        were_parsed |= BCM_PORT_ATTR_PAUSE_MASK;
    }

    *seen = were_seen;
    *parsed = were_parsed;
}

/* Iterate thru a port bitmap with the given mask; display info */
STATIC int
_tk371x_port_disp_iter(int unit, bcm_port_info_t *info, pbmp_t pbm,
                pbmp_t pbm_mask, uint32 seen)
{
    soc_port_t port;
    int r;

    BCM_PBMP_AND(pbm, pbm_mask);
    BCM_PBMP_ITER(pbm, port) {
        tk371x_port_info_init(unit, port, info, seen);

        if ((r = _bcm_tk371x_port_selective_get(unit, port, info)) < 0) {
            cli_out("Error: Could not get port %s information: %s\n",
                    SOC_PORT_NAME(unit, port), bcm_errmsg(r));
            return (CMD_FAIL);
        }
        tk371x_disp_port_info(SOC_PORT_NAME(unit, port), info,
                       IS_HG_PORT(unit, port), seen);
    }

    return CMD_OK;
}

int
tk371x_port_parse_port_info_set(uint32 flags, bcm_port_info_t *src,
                         bcm_port_info_t *dest)
{
    int info_speed_adj;

    if (flags & BCM_PORT_ATTR_AUTONEG_MASK) {
        dest->autoneg = src->autoneg;
    }

    if (flags & BCM_PORT_ATTR_ENABLE_MASK) {
        dest->enable = src->enable;
    }

    if (flags & BCM_PORT_ATTR_STP_STATE_MASK) {
        dest->stp_state = src->stp_state;
    }

    /*
     * info_speed_adj is the same as src->speed except a speed of 0
     * is replaced by the maximum speed supported by the port.
     */

    info_speed_adj = src->speed;

    if ((flags & BCM_PORT_ATTR_SPEED_MASK) && (info_speed_adj == 0)) {
        info_speed_adj = src->speed_max;
    }

    /* This section calculates the local advertisement */
    if (dest->autoneg) {
        int                 cur_speed, cur_duplex;
        int                 cur_pause_tx, cur_pause_rx;
        int                 new_speed, new_duplex;
        int                 new_pause_tx, new_pause_rx;
        bcm_port_abil_t     mode;

        /*
         * Update link advertisements for speed/duplex/pause.  All
         * speeds less than or equal to the requested speed are
         * advertised.
         */

        mode = dest->local_advert;

        cur_speed = BCM_PORT_ABIL_SPD_MAX(mode);
        cur_duplex = ((mode & BCM_PORT_ABIL_FD) ?
                      SOC_PORT_DUPLEX_FULL : SOC_PORT_DUPLEX_HALF);
        cur_pause_tx = (mode & BCM_PORT_ABIL_PAUSE_TX) != 0;
        cur_pause_rx = (mode & BCM_PORT_ABIL_PAUSE_RX) != 0;

        new_speed = (flags & BCM_PORT_ATTR_SPEED_MASK ?
                     info_speed_adj : cur_speed);
        new_duplex = (flags & BCM_PORT_ATTR_DUPLEX_MASK ?
                      src->duplex : cur_duplex);
        new_pause_tx = (flags & BCM_PORT_ATTR_PAUSE_TX_MASK ?
                        src->pause_tx : cur_pause_tx);
        new_pause_rx = (flags & BCM_PORT_ATTR_PAUSE_RX_MASK ?
                        src->pause_rx : cur_pause_rx);

        /* Start with maximum ability and cut down */

        mode = src->ability;

        if (new_duplex == SOC_PORT_DUPLEX_HALF) {
            mode &= ~BCM_PORT_ABIL_FD;
        }

        if (new_speed < 1000) {
            mode &= ~BCM_PORT_ABIL_1000MB;
        }

        if (new_speed < 100) {
            mode &= ~BCM_PORT_ABIL_100MB;
        }

        if (!(mode & BCM_PORT_ABIL_PAUSE_ASYMM) &&
            (new_pause_tx != new_pause_rx)) {
            cli_out("port parse: Error: Asymmetrical pause not available\n");
            return -1;
        }

        if (!new_pause_tx) {
            mode &= ~BCM_PORT_ABIL_PAUSE_TX;
        }

        if (!new_pause_rx) {
            mode &= ~BCM_PORT_ABIL_PAUSE_RX;
        }

        dest->local_advert = mode;
    } else {
        /* Update forced values for speed/duplex/pause */

        if (flags & BCM_PORT_ATTR_SPEED_MASK) {
            dest->speed = info_speed_adj;
        }

        if (flags & BCM_PORT_ATTR_DUPLEX_MASK) {
            dest->duplex = src->duplex;
        }

        if (flags & BCM_PORT_ATTR_PAUSE_TX_MASK) {
            dest->pause_tx = src->pause_tx;
        }

        if (flags & BCM_PORT_ATTR_PAUSE_RX_MASK) {
            dest->pause_rx = src->pause_rx;
        }
    }

    if (flags & BCM_PORT_ATTR_PAUSE_MAC_MASK) {
        sal_memcpy(dest->pause_mac, src->pause_mac, sizeof (sal_mac_addr_t));
    }

    if (flags & BCM_PORT_ATTR_LINKSCAN_MASK) {
        dest->linkscan = src->linkscan;
    }

    if (flags & BCM_PORT_ATTR_LEARN_MASK) {
        dest->learn = src->learn;
    }

    if (flags & BCM_PORT_ATTR_DISCARD_MASK) {
        dest->discard = src->discard;
    }

    if (flags & BCM_PORT_ATTR_VLANFILTER_MASK) {
        dest->vlanfilter = src->vlanfilter;
    }

    if (flags & BCM_PORT_ATTR_UNTAG_PRI_MASK) {
        dest->untagged_priority = src->untagged_priority;
    }

    if (flags & BCM_PORT_ATTR_PFM_MASK) {
        dest->pfm = src->pfm;
    }

    if (flags & BCM_PORT_ATTR_PHY_MASTER_MASK) {
        dest->phy_master = src->phy_master;
    }

    if (flags & BCM_PORT_ATTR_INTERFACE_MASK) {
        dest->interface = src->interface;
    }

    if (flags & BCM_PORT_ATTR_LOOPBACK_MASK) {
        dest->loopback = src->loopback;
    }

    if (flags & BCM_PORT_ATTR_ENCAP_MASK) {
        dest->encap_mode = src->encap_mode;
    }

    return 0;
}

/*
 * Function:
 *  if_port
 * Purpose:
 *  Configure port specific parameters.
 * Parameters:
 *  u   - SOC unit #
 *  a   - pointer to args
 * Returns:
 *  CMD_OK/CMD_FAIL
 */
cmd_result_t
if_ea_port(int u, args_t *a)
{
    pbmp_t pbm;
    bcm_port_info_t *info_all;
    bcm_port_info_t info;               /* Current port's info */
    static bcm_port_info_t given;
    char *c;
    int r;
    int rv;
    int cmd_rv = CMD_OK;
    soc_port_t p;
    parse_table_t pt;
    uint32 seen = 0;
    uint32 parsed = 0;
    char pfmt[SOC_PBMP_FMT_LEN];

    if (!sh_check_attached(ARG_CMD(a), u)) {
        return CMD_FAIL;
    }

    if ((c = ARG_GET(a)) == NULL) {
        return(CMD_USAGE);
    }

    info_all = sal_alloc(SOC_MAX_NUM_PORTS * sizeof(bcm_port_info_t),
                         "if_ea_port");
    if (info_all == NULL) {
        cli_out("Insufficient memory.\n");
        return CMD_FAIL;
    }
    sal_memset(&info, 0, sizeof(info));
    sal_memset(info_all, 0, SOC_MAX_NUM_PORTS * sizeof(bcm_port_info_t));
    sal_memset(&given, 0, sizeof(bcm_port_info_t));

    if (parse_pbmp(u, c, &pbm) < 0) {
        cli_out("%s: Error: unrecognized port bitmap: %s\n",
                ARG_CMD(a), c);
        sal_free(info_all);
        return CMD_FAIL;
    }

    BCM_PBMP_AND(pbm, PBMP_E_ALL(u));

    if (BCM_PBMP_IS_NULL(pbm)) {
        ARG_DISCARD(a);
        sal_free(info_all);
        return CMD_OK;
    }

    if (ARG_CNT(a) == 0) {
        seen = BCM_PORT_ATTR_ALL_MASK;
    } else {
        /*
         * Otherwise, arguments are given.  Use them to determine which
         * properties need to be gotten/set.
         *
         * Probe and detach, hidden commands.
         */
        if (!sal_strcasecmp(_ARG_CUR(a), "detach")) {
            pbmp_t detached;
            bcm_port_detach(u, pbm, &detached);
            cli_out("Detached port bitmap %s\n", SOC_PBMP_FMT(detached, pfmt));
            ARG_GET(a);
            sal_free(info_all);
            return CMD_OK;
        } else if ((!sal_strcasecmp(_ARG_CUR(a), "probe")) ||
                   (!sal_strcasecmp(_ARG_CUR(a), "attach"))) {
            pbmp_t probed;
            bcm_port_probe(u, pbm, &probed);
            cli_out("Probed port bitmap %s\n", SOC_PBMP_FMT(probed, pfmt));
            ARG_GET(a);
            sal_free(info_all);
            return CMD_OK;
        }

        if (!sal_strcasecmp(_ARG_CUR(a), "disable")) {
            bcm_port_t p_port;
            PBMP_ITER(pbm, p_port) {
                bcm_port_enable_set(u, p_port, 0);
            }
            ARG_GET(a);
            sal_free(info_all);
            return CMD_OK;

        } else if (!sal_strcasecmp(_ARG_CUR(a), "enable")) {
            bcm_port_t p_port;
            PBMP_ITER(pbm, p_port) {
                /* coverity[check_return] */
                bcm_port_enable_set(u, p_port, 1);
            }
            ARG_GET(a);
            sal_free(info_all);
            return CMD_OK;
        }

        if (!sal_strcmp(_ARG_CUR(a), "=")) {
            /*
             * For "=" where the user is prompted to enter all the parameters,
             * use the parameters from the first selected port as the defaults.
             */
            if (ARG_CNT(a) != 1) {
                sal_free(info_all);
                return CMD_USAGE;
            }
            PBMP_ITER(pbm, p) {
                break;    /* Find first port in bitmap */
            }
            if ((rv = bcm_port_info_get(u, p, &given)) < 0) {
                cli_out("%s: Error: Failed to get port info\n", ARG_CMD(a));
                sal_free(info_all);
                return CMD_FAIL;
            }
        }

        /*
         * Parse the arguments.  Determine which ones were actually given.
         */
        tk371x_port_parse_setup(u, &pt, &given);

        if (parse_arg_eq(a, &pt) < 0) {
            parse_arg_eq_done(&pt);
            sal_free(info_all);
            return(CMD_FAIL);
        }

        if (ARG_CNT(a) > 0) {
            cli_out("%s: Unknown argument %s\n", ARG_CMD(a), ARG_CUR(a));
            parse_arg_eq_done(&pt);
            sal_free(info_all);
            return(CMD_FAIL);
        }

        /*
         * Find out what parameters specified.  Record values specified.
         */
        tk371x_port_parse_mask_get(&pt, &seen, &parsed);
        parse_arg_eq_done(&pt);
    }

    if (seen && parsed) {
        cli_out("%s: Cannot get and set "
                "port properties in one command\n",
                ARG_CMD(a));
        sal_free(info_all);
        return CMD_FAIL;
    } else if (seen) { /* Show selected information */
        if (SOC_IS_HERCULES(u)) {
            seen &= BCM_PORT_HERC_ATTRS;
        }
        cli_out("%s: Status (* indicates PHY link up)\n", ARG_CMD(a));
        /* Display the information by port type */
#define _call_pdi(u, i, p, mp, s) \
        if (_tk371x_port_disp_iter(u, i, p, mp, s) != CMD_OK) { \
             sal_free(info_all); \
             return CMD_FAIL; \
        }
        _call_pdi(u, &info, pbm, PBMP_FE_ALL(u), seen);
        _call_pdi(u, &info, pbm, PBMP_GE_ALL(u), seen);
        sal_free(info_all);
        return(CMD_OK);
    }

    /* Some set information was given */

    if (parsed & BCM_PORT_ATTR_LINKSCAN_MASK) {
        /* Map ON --> S/W, OFF--> None */
        if (given.linkscan > 2) {
            given.linkscan -= 3;
        }
    }

    /*
     * Retrieve all requested port information first, then later write
     * back all port information.  That prevents a problem with loopback
     * cables where setting one port's info throws another into autoneg
     * causing it to return info in flux (e.g. suddenly go half duplex).
     */

    PBMP_ITER(pbm, p) {
        tk371x_port_info_init(u, p, &info_all[p], parsed);
        if ((r = _bcm_tk371x_port_selective_get(u, p, &info_all[p])) < 0) {
            cli_out("%s: Error: Could not get port %s information: %s\n",
                    ARG_CMD(a), SOC_PORT_NAME(u, p), bcm_errmsg(r));
            sal_free(info_all);
            return (CMD_FAIL);
        }
    }

    /*
     * Loop through all the specified ports, changing whatever field
     * values were actually given.  This avoids copying unaffected
     * information from one port to another and prevents having to
     * re-parse the arguments once per port.
     */

    PBMP_ITER(pbm, p) {
        if ((rv = bcm_port_speed_max(u, p, &given.speed_max)) < 0) {
            cli_out("port parse: Error: Could not get port %s max speed: %s\n",
                    SOC_PORT_NAME(u, p), bcm_errmsg(rv));
            continue;
        }

        if ((rv = bcm_port_ability_get(u, p, &given.ability)) < 0) {
            cli_out("port parse: Error: Could not get port %s ability: %s\n",
                    SOC_PORT_NAME(u, p), bcm_errmsg(rv));
            continue;
        }

        if ((r = tk371x_port_parse_port_info_set(parsed,
                                          &given, &info_all[p])) < 0) {
            cli_out("%s: Error: Could not parse port %s info: %s\n",
                    ARG_CMD(a), SOC_PORT_NAME(u, p), bcm_errmsg(r));
            cmd_rv = CMD_FAIL;
            continue;
        }

        /* If AN is on, do not set speed, duplex, pause */
        if (info_all[p].autoneg) {
            info_all[p].action_mask &= ~(BCM_PORT_ATTR_SPEED_MASK |
                  BCM_PORT_ATTR_DUPLEX_MASK);
        }

        if (given.frame_max) {
            /* Only set when the max frame size is input */
            if ((r = bcm_port_frame_max_set(u, p, given.frame_max)) < 0) {
                cli_out("%s: Error: Could not set port %s max frame size: %s\n",
                        ARG_CMD(a), SOC_PORT_NAME(u, p), bcm_errmsg(r));
                cmd_rv = CMD_FAIL;
                continue;
            }
        }

    }
    sal_free(info_all);
    return(cmd_rv);
}


cmd_result_t
if_ea_linkscan(int unit, args_t *a)
{
	int rv;
    pbmp_t pbm_sw, pbm_hw;
    char buf[80];
    pbmp_t pbm;
    char pfmt[SOC_PBMP_FMT_LEN];
    pbmp_t pbm_temp;

    /*
     * Workaround that allows "linkscan off" at the beginning of rc.soc
     */

    if (ARG_CNT(a) == 1 && sal_strcasecmp(_ARG_CUR(a), "off") == 0) {
		rv = bcm_init_check(unit);
		if (rv == BCM_E_UNIT) {
			(void) ARG_GET(a);
			return(CMD_OK);
		}
    }


    /*
     * First get current linkscan state.  (us == 0 if disabled).
     */
    BCM_PBMP_CLEAR(pbm_sw);
    BCM_PBMP_CLEAR(pbm_hw);
    pbm_sw = PBMP_ALL(unit);
    pbm_hw = PBMP_PORT_ALL(unit);
	cli_out("%s: Linkscan enabled\n", ARG_CMD(a));
	format_pbmp(unit, buf, sizeof (buf), pbm_sw);
	cli_out("%s:   Software Port BitMap %s: %s\n",
                ARG_CMD(a), SOC_PBMP_FMT(pbm_sw, pfmt), buf);
	format_pbmp(unit, buf, sizeof (buf), pbm_hw);
	cli_out("%s:   Hardware Port BitMap %s: %s\n",
                ARG_CMD(a), SOC_PBMP_FMT(pbm_hw, pfmt), buf);
	SOC_PBMP_ASSIGN(pbm_temp, pbm_sw);
	SOC_PBMP_OR(pbm_temp, pbm_hw);
	SOC_PBMP_ASSIGN(pbm, PBMP_PORT_ALL(unit));
	SOC_PBMP_XOR(pbm, pbm_temp);
	format_pbmp(unit, buf, sizeof (buf), pbm);
	cli_out("%s:   Disabled Port BitMap %s: %s\n",
                ARG_CMD(a), SOC_PBMP_FMT(pbm, pfmt), buf);
    return(CMD_OK);
}

static struct {			/* match enum from bcm/port.h */
    int		type;
    char	*name;
} poncontrol_names[] = {
	{bcmPortControlPonFecMode,    	"PON-port-FEC-mode"},
	{bcmPortControlPonUserTraffic,	"PON-Port-User-Traffic"},
	{bcmPortControlPonMultiLlid, 	"PON-Multi-LLID"},
	{bcmPortControlPonEncryptKeyExpiryTime, "PON-Encrypt-Key-Expiry-Time"},
	{bcmPortControlPonHoldoverState, 		"PON-Holdover-State"},
	{bcmPortControlPonHoldoverTime, 		"PON-Holdover-Time"},
    { 0,			NULL }		/* LAST ENTRY */
},
 ethcontrol_names[] = {
	{bcmPortControlEthPortAutoNegFailureAlarmState, "Eth-port-Auto-Neg-Failure-Alarm-State"},
	{bcmPortControlEthPortLosAlarmState, 	"Eth-Port-Los-Alarm-State"},
	{bcmPortControlEthPortFailureAlarmState, "Eth-Port-Failure-Alarm-State"},
	{bcmPortControlEthPortLoopbackAlarmState, "Eth-Port-Loopback-Alarm-State"},
	{bcmPortControlEthPortCongestionAlarmState, "Eth-Port-Congestion-Alarm-State"},
	{ 0, 			NULL }		/* LAST ENTRY */
},
 llidcontrol_names[] = {
	{bcmPortControlPonFecMode,    	"llid-port-FEC-mode"},
	{ 0, 			NULL }		/* LAST ENTRY */

};

char if_ea_port_control_usage[] =
    "Set/Display port control types.\n"
    "Parameters: <pbm> [type_name] [value]\n";
/*
 * Function:    if_ea_port_control
 * Purpose: Configure default IPG values.
 * Parameters:  unit - SOC unit #
 *      a - arguments
 * Returns: CMD_OK/CMD_FAIL
 */
cmd_result_t
if_ea_port_control(int unit, args_t *a)
{
    char        *val_str, *c;
    bcm_pbmp_t  pbm;
    bcm_port_t  p;
    int         i, type, rv, val = 0;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if ((c = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    if (parse_pbmp(unit, c, &pbm) < 0) {
        cli_out("%s: ERROR: unrecognized port bitmap: %s\n",
                ARG_CMD(a), c);
        return CMD_FAIL;
    }

    SOC_PBMP_AND(pbm, PBMP_PORT_ALL(unit));

    c = ARG_GET(a);     /* NULL or media type or command */

    if (c == NULL) {
        /* dump process */
        PBMP_ITER(pbm, p){
        	if (p == 0){
				cli_out("\n EPON Port %s:\n", SOC_PORT_NAME(unit, p));
				for (i = 0; poncontrol_names[i].name != NULL; i++) {
					type = poncontrol_names[i].type;
					rv = bcm_port_control_get(unit, p, type, &val);
					cli_out("\t %-45s= ", poncontrol_names[i].name);
					if (rv){
						cli_out("%s\n", bcm_errmsg(rv));
					} else {
						cli_out("0x%x\n", val);
					}
				}
        	}else if (p ==1 || p == 2){
				cli_out("\n UNI Port %s:\n", SOC_PORT_NAME(unit, p));
				for (i = 0; ethcontrol_names[i].name != NULL; i++) {
					type = ethcontrol_names[i].type;
					rv = bcm_port_control_get(unit, p, type, &val);
					cli_out("\t %-45s= ", ethcontrol_names[i].name);
					if (rv){
						cli_out("%s\n", bcm_errmsg(rv));
					} else {
						cli_out("0x%x\n", val);
					}
				}
        	}else{
				cli_out("\n LLID Port %s:\n", SOC_PORT_NAME(unit, p));
				for (i = 0; llidcontrol_names[i].name != NULL; i++) {
					type = llidcontrol_names[i].type;
					rv = bcm_port_control_get(unit, p, type, &val);
					cli_out("\t %-45s= ", llidcontrol_names[i].name);
					if (rv){
						cli_out("%s\n", bcm_errmsg(rv));
					} else {
						cli_out("0x%x\n", val);
					}
				}
        	}

        }
    } else {
        /* proceeding at specific type */
        val_str = ARG_GET(a);
        PBMP_ITER(pbm, p){
        	if (p == 0){
				cli_out("\n PON Port %s:\n", SOC_PORT_NAME(unit, p));
				for (i = 0; poncontrol_names[i].name != NULL; i++) {
					if (sal_strcasecmp(c, poncontrol_names[i].name) == 0){
						type = poncontrol_names[i].type;
						if (val_str != NULL){
							/* set process */
							val = parse_integer(val_str);
							rv = bcm_port_control_set(unit, p, type, val);
							if (rv) {
								cli_out("\t Failed on setting type=%s(%d),%s\n",
                                                                        poncontrol_names[i].name,
                                                                        poncontrol_names[i].type,
                                                                        bcm_errmsg(rv));
							} else {
								cli_out("\t %-45s(%d) set value=%d\n",
                                                                        poncontrol_names[i].name,
                                                                        poncontrol_names[i].type,
                                                                        val);
							}
						} else {
							/* get process */
							rv = bcm_port_control_get(unit, p, type, &val);
							cli_out("\t %-45s= ", poncontrol_names[i].name);
							if (rv){
								cli_out("%s\n", bcm_errmsg(rv));
							} else {
								cli_out("0x%x\n", val);
							}
						}
						break;
					}
				}
				if (poncontrol_names[i].name == NULL){
					cli_out("\t Unknown Port Control type at %s!\n", c);
					return CMD_FAIL;
				}
            }else if (p == 1 || p == 2){
				cli_out("\n UNI Port %s:\n", SOC_PORT_NAME(unit, p));
				for (i = 0; ethcontrol_names[i].name != NULL; i++) {
					if (sal_strcasecmp(c, ethcontrol_names[i].name) == 0){
						type = ethcontrol_names[i].type;
						if (val_str != NULL){
							/* set process */
							val = parse_integer(val_str);
							rv = bcm_port_control_set(unit, p, type, val);
							if (rv) {
								cli_out("\t Failed on setting type=%s(%d),%s\n",
                                                                        ethcontrol_names[i].name,
                                                                        ethcontrol_names[i].type,
                                                                        bcm_errmsg(rv));
							} else {
								cli_out("\t %-45s(%d) set value=%d\n",
                                                                        ethcontrol_names[i].name,
                                                                        ethcontrol_names[i].type,
                                                                        val);
							}
						} else {
							/* get process */
							rv = bcm_port_control_get(unit, p, type, &val);
							cli_out("\t %-45s= ", ethcontrol_names[i].name);
							if (rv){
								cli_out("%s\n", bcm_errmsg(rv));
							} else {
								cli_out("0x%x\n", val);
							}
						}
						break;
					}
				}
				if (ethcontrol_names[i].name == NULL){
					cli_out("\t Unknown Port Control type at %s!\n", c);
					return CMD_FAIL;
				}
            }else{
				cli_out("\n LLID Port %s:\n", SOC_PORT_NAME(unit, p));
				for (i = 0; llidcontrol_names[i].name != NULL; i++) {
					if (sal_strcasecmp(c, llidcontrol_names[i].name) == 0){
						type = llidcontrol_names[i].type;
						if (val_str != NULL){
							/* set process */
							val = parse_integer(val_str);
							rv = bcm_port_control_set(unit, p, type, val);
							if (rv) {
								cli_out("\t Failed on setting type=%s(%d),%s\n",
                                                                        llidcontrol_names[i].name,
                                                                        llidcontrol_names[i].type,
                                                                        bcm_errmsg(rv));
							} else {
								cli_out("\t %-45s(%d) set value=%d\n",
                                                                        llidcontrol_names[i].name,
                                                                        llidcontrol_names[i].type,
                                                                        val);
							}
						} else {
							/* get process */
							rv = bcm_port_control_get(unit, p, type, &val);
							cli_out("\t %-45s= ", llidcontrol_names[i].name);
							if (rv){
								cli_out("%s\n", bcm_errmsg(rv));
							} else {
								cli_out("0x%x\n", val);
							}
						}
						break;
					}
				}
				if (ethcontrol_names[i].name == NULL){
					cli_out("\t Unknown Port Control type at %s!\n", c);
					return CMD_FAIL;
				}
            }
        }
    }

    return CMD_OK;
}

static struct {
    int		type;
    char	*name;
}ponphycontrol_names[] = {
	{BCM_PORT_PHY_CONTROL_INTERFACE, "PON_INTERFACE"},
	{BCM_PORT_PHY_CONTROL_PON_LASER_TRANCEIVER_TEMP, "PON_LASER_TRANCEIVER_TEMP"},
	{BCM_PORT_PHY_CONTROL_PON_LASER_SUPPLY_VOLTAGE, "PON_LASER_SUPPLY_VOLTAGE"},
	{BCM_PORT_PHY_CONTROL_PON_LASER_TX_BIAS, "PON_LASER_TX_BIAS"},
	{BCM_PORT_PHY_CONTROL_PON_LASER_TX_POWER, "PON_LASER_TX_POWER"},
	{BCM_PORT_PHY_CONTROL_PON_LASER_RX_POWER, "PON_LASER_RX_POWER"},
	{BCM_PORT_PHY_CONTROL_PON_LASER_TX_POWER_TIME, "PON_LASER_TX_POWER_TIME"},
	{BCM_PORT_PHY_CONTROL_PON_LASER_TX_POWER_MODE, "PON_LASER_TX_POWER_MODE"},
	{BCM_PORT_PHY_CONTROL_PON_LASER_TX_STATUS, "PON_LASER_TX_STATUS"},
	{BCM_PORT_PHY_CONTROL_PON_LASER_RX_STATE, "PON_LASER_RX_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_RX_POWER_HIGH_ALARM_STATE, "PON_RX_POWER_HIGH_ALARM_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_RX_POWER_HIGH_ALARM_REPORT_THRESHOLD, "PON_RX_POWER_HIGH_ALARM_REPORT_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_RX_POWER_HIGH_ALARM_CLEAR_THRESHOLD, "PON_RX_POWER_HIGH_ALARM_CLEAR_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_RX_POWER_LOW_ALARM_STATE, "PON_RX_POWER_LOW_ALARM_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_RX_POWER_LOW_ALARM_REPORT_THRESHOLD, "PON_RX_POWER_LOW_ALARM_REPORT_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_RX_POWER_LOW_ALARM_CLEAR_THRESHOLD, "PON_RX_POWER_LOW_ALARM_CLEAR_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TX_POWER_HIGH_ALARM_STATE, "PON_TX_POWER_HIGH_ALARM_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_TX_POWER_HIGH_ALARM_REPORT_THRESHOLD, "PON_TX_POWER_HIGH_ALARM_REPORT_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TX_POWER_HIGH_ALARM_CLEAR_THRESHOLD, "PON_TX_POWER_HIGH_ALARM_CLEAR_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TX_POWER_LOW_ALARM_STATE, "PON_TX_POWER_LOW_ALARM_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_TX_POWER_LOW_ALARM_REPORT_THRESHOLD, "PON_TX_POWER_LOW_ALARM_REPORT_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TX_POWER_LOW_ALARM_CLEAR_THRESHOLD, "PON_TX_POWER_LOW_ALARM_CLEAR_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TX_BIAS_HIGH_ALARM_STATE, "PON_TX_BIAS_HIGH_ALARM_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_TX_BIAS_HIGH_ALARM_REPORT_THRESHOLD, "PON_TX_BIAS_HIGH_ALARM_REPORT_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TX_BIAS_HIGH_ALARM_CLEAR_THRESHOLD, "PON_TX_BIAS_HIGH_ALARM_CLEAR_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TX_BIAS_LOW_ALARM_STATE, "PON_TX_BIAS_LOW_ALARM_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_TX_BIAS_LOW_ALARM_REPORT_THRESHOLD, "PON_TX_BIAS_LOW_ALARM_REPORT_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TX_BIAS_LOW_ALARM_CLEAR_THRESHOLD, "PON_TX_BIAS_LOW_ALARM_CLEAR_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_VCC_HIGH_ALARM_STATE, "PON_VCC_HIGH_ALARM_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_VCC_HIGH_ALARM_REPORT_THRESHOLD, "PON_VCC_HIGH_ALARM_REPORT_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_VCC_HIGH_ALARM_CLEAR_THRESHOLD, "PON_VCC_HIGH_ALARM_CLEAR_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_VCC_LOW_ALARM_STATE, "PON_VCC_LOW_ALARM_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_VCC_LOW_ALARM_REPORT_THRESHOLD, "PON_VCC_LOW_ALARM_REPORT_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_VCC_LOW_ALARM_CLEAR_THRESHOLD, "PON_VCC_LOW_ALARM_CLEAR_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TEMP_HIGH_ALARM_STATE, "PON_TEMP_HIGH_ALARM_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_TEMP_HIGH_ALARM_REPORT_THRESHOLD, "PON_TEMP_HIGH_ALARM_REPORT_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TEMP_HIGH_ALARM_CLEAR_THRESHOLD, "PON_TEMP_HIGH_ALARM_CLEAR_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TEMP_LOW_ALARM_STATE, "PON_TEMP_LOW_ALARM_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_TEMP_LOW_ALARM_REPORT_THRESHOLD, "PON_TEMP_LOW_ALARM_REPORT_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TEMP_LOW_ALARM_CLEAR_THRESHOLD, "PON_TEMP_LOW_ALARM_CLEAR_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_RX_POWER_HIGH_WARNING_STATE, "PON_RX_POWER_HIGH_WARNING_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_RX_POWER_HIGH_WARNING_REPORT_THRESHOLD, "PON_RX_POWER_HIGH_WARNING_REPORT_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_RX_POWER_HIGH_WARNING_CLEAR_THRESHOLD, "PON_RX_POWER_HIGH_WARNING_CLEAR_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_RX_POWER_LOW_WARNING_STATE, "PON_RX_POWER_LOW_WARNING_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_RX_POWER_LOW_WARNING_REPORT_THRESHOLD, "PON_RX_POWER_LOW_WARNING_REPORT_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_RX_POWER_LOW_WARNING_CLEAR_THRESHOLD, "PON_RX_POWER_LOW_WARNING_CLEAR_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TX_POWER_HIGH_WARNING_STATE, "PON_TX_POWER_HIGH_WARNING_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_TX_POWER_HIGH_WARNING_REPORT_THRESHOLD, "PON_TX_POWER_HIGH_WARNING_REPORT_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TX_POWER_HIGH_WARNING_CLEAR_THRESHOLD, "PON_TX_POWER_HIGH_WARNING_CLEAR_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TX_POWER_LOW_WARNING_STATE, "PON_TX_POWER_LOW_WARNING_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_TX_POWER_LOW_WARNING_REPORT_THRESHOLD, "PON_TX_POWER_LOW_WARNING_REPORT_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TX_POWER_LOW_WARNING_CLEAR_THRESHOLD, "PON_TX_POWER_LOW_WARNING_CLEAR_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TX_BIAS_HIGH_WARNING_STATE, "PON_TX_BIAS_HIGH_WARNING_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_TX_BIAS_HIGH_WARNING_REPORT_THRESHOLD, "PON_TX_BIAS_HIGH_WARNING_REPORT_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TX_BIAS_HIGH_WARNING_CLEAR_THRESHOLD, "PON_TX_BIAS_HIGH_WARNING_CLEAR_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TX_BIAS_LOW_WARNING_STATE, "PON_TX_BIAS_LOW_WARNING_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_TX_BIAS_LOW_WARNING_REPORT_THRESHOLD, "PON_TX_BIAS_LOW_WARNING_REPORT_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TX_BIAS_LOW_WARNING_CLEAR_THRESHOLD, "PON_TX_BIAS_LOW_WARNING_CLEAR_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_VCC_HIGH_WARNING_STATE, "PON_VCC_HIGH_WARNING_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_VCC_HIGH_WARNING_REPORT_THRESHOLD, "PON_VCC_HIGH_WARNING_REPORT_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_VCC_HIGH_WARNING_CLEAR_THRESHOLD, "PON_VCC_HIGH_WARNING_CLEAR_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_VCC_LOW_WARNING_STATE, "PON_VCC_LOW_WARNING_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_VCC_LOW_WARNING_REPORT_THRESHOLD, "PON_VCC_LOW_WARNING_REPORT_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_VCC_LOW_WARNING_CLEAR_THRESHOLD, "PON_VCC_LOW_WARNING_CLEAR_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TEMP_HIGH_WARNING_STATE, "PON_TEMP_HIGH_WARNING_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_TEMP_HIGH_WARNING_REPORT_THRESHOLD, "PON_TEMP_HIGH_WARNING_REPORT_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TEMP_HIGH_WARNING_CLEAR_THRESHOLD, "PON_TEMP_HIGH_WARNING_CLEAR_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TEMP_LOW_WARNING_STATE, "PON_TEMP_LOW_WARNING_STATE"},
	{BCM_PORT_PHY_CONTROL_PON_TEMP_LOW_WARNING_REPORT_THRESHOLD, "PON_TEMP_LOW_WARNING_REPORT_THRESHOLD"},
	{BCM_PORT_PHY_CONTROL_PON_TEMP_LOW_WARNING_CLEAR_THRESHOLD, "PON_TEMP_LOW_WARNING_CLEAR_THRESHOLD"},
	{ 0,  NULL}, /* The Last Entry */
};

char if_ea_port_phy_control_usage[] =
	"Set/Display epon port phy control types. (Now only support PON port)\n"
	"Parameters: <pbm> [type_name] [value]\n";
cmd_result_t
if_ea_port_phy_control(int unit, args_t *a){

    char        *val_str, *c;
    bcm_pbmp_t  pbm;
    bcm_port_t  p;
    int         i, type, rv;
    uint32 val;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if ((c = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    if (parse_pbmp(unit, c, &pbm) < 0) {
        cli_out("%s: ERROR: unrecognized port bitmap: %s\n",
                ARG_CMD(a), c);
        return CMD_FAIL;
    }

    SOC_PBMP_AND(pbm, PBMP_PORT_ALL(unit));

    c = ARG_GET(a);     /* NULL or media type or command */

    if (c == NULL) {
        /* dump process */
        PBMP_ITER(pbm, p){
        	if (p !=0 ){
        		return CMD_USAGE;
        	}
            cli_out("\n EPON Port %s:\n", SOC_PORT_NAME(unit, p));
            for (i = 0; ponphycontrol_names[i].name != NULL; i++) {
                type = ponphycontrol_names[i].type;
                rv = bcm_port_phy_control_get(unit, p, type, &val);
                cli_out("\t %-45s= ", ponphycontrol_names[i].name);
                if (rv){
                    cli_out("%s\n", bcm_errmsg(rv));
                } else {
                    cli_out("0x%x\n", val);
                }
            }
        }
    } else {
        /* proceeding at specific type */
        val_str = ARG_GET(a);
        PBMP_ITER(pbm, p){
        	if (p != 0){
        		return CMD_USAGE;
        	}
            cli_out("\n EPON Port %s:\n", SOC_PORT_NAME(unit, p));
            for (i = 0; ponphycontrol_names[i].name != NULL; i++) {
                if (sal_strcasecmp(c, ponphycontrol_names[i].name) == 0){
                    type = ponphycontrol_names[i].type;

                    if (val_str != NULL){
                        /* set process */
                        val = parse_integer(val_str);
                        rv = bcm_port_phy_control_set(unit, p, type, val);
                        if (rv) {
                            cli_out("\t Failed on setting type=%s(%d),%s\n",
                                    ponphycontrol_names[i].name,
                                    ponphycontrol_names[i].type,
                                    bcm_errmsg(rv));
                        } else {
                            cli_out("\t %-45s(%d) set value=%d\n",
                                    ponphycontrol_names[i].name,
                                    ponphycontrol_names[i].type,
                                    val);
                        }
                    } else {
                        /* get process */
                        rv = bcm_port_phy_control_get(unit, p, type, &val);
                        cli_out("\t %-45s= ", ponphycontrol_names[i].name);
                        if (rv){
                            cli_out("%s\n", bcm_errmsg(rv));
                        } else {
                            cli_out("0x%x\n", val);
                        }
                    }
                    break;
                }
            }
            if (ponphycontrol_names[i].name == NULL){
                cli_out("\t Unknown Port Control type at %s!\n", c);
                return CMD_FAIL;
            }
        }
    }

    return CMD_OK;
}


char if_ea_port_cross_connect_usage[] =
    "Set/Display port cross connect information.\n"
    "Parameters: <port> <enable/disable> <egress_port>\n";
/*
 * Function:    if_ea_port_cross_connect
 * Purpose: Configure port cross connect values.
 * Parameters:  unit - SOC unit #
 *      a - arguments
 * Returns: CMD_OK/CMD_FAIL
 */
cmd_result_t
if_ea_port_cross_connect(int unit, args_t *a)
{
	return CMD_OK;
}

/*
 * Function:
 *  if_ea_port_rate
 * Purpose:
 *  Set/display of port rate metering characteristics
 * Parameters:
 *  unit - SOC unit #
 *  a - pointer to args
 * Returns:
 *  CMD_OK/CMD_FAIL
 */
cmd_result_t
if_ea_port_rate(int unit, args_t *a)
{
	return CMD_OK;
}

/*
 * Function:
 *      if_ea_port_samp_rate
 * Purpose:
 *      Set/display of sflow port sampling rates.
 * Parameters:
 *      unit - SOC unit #
 *      args - pointer to comand line arguments
 * Returns:
 *      CMD_OK/CMD_FAIL
 */
char if_ea_port_samp_rate_usage[] =
    "Set/Display port sampling rate characteristics.\n"
    "Parameters: <pbm> [ingress_rate] [egress_rate]\n"
    "\tOn average, every 1/ingress_rate packets will be sampled.\n"
    "\tA rate of 0 indicates no sampling.\n"
    "\tA rate of 1 indicates all packets sampled.\n";

cmd_result_t
if_ea_port_samp_rate(int unit, args_t *args)
{
	return CMD_OK;
}

/*
 * Function:
 *  if_port_stat
 * Purpose:
 *  Table display of port information
 * Parameters:
 *  unit - SOC unit #
 *  a - pointer to args
 * Returns:
 *  CMD_OK/CMD_FAIL
 */
cmd_result_t
if_ea_port_stat(int unit, args_t *a)
{
    pbmp_t          pbm, tmp_pbm;
    bcm_port_info_t *info_all;
    soc_port_t      p;
    int             r;
    char           *c;
    uint32          mask;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if ((c = ARG_GET(a)) == NULL) {
        BCM_PBMP_ASSIGN(pbm, PBMP_PORT_ALL(unit));
    } else if (parse_pbmp(unit, c, &pbm) < 0) {
        cli_out("%s: Error: unrecognized port bitmap: %s\n",
                ARG_CMD(a), c);
        return CMD_FAIL;
    }
    BCM_PBMP_AND(pbm, PBMP_PORT_ALL(unit));
    if (BCM_PBMP_IS_NULL(pbm)) {
        cli_out("No ports specified.\n");
        return CMD_OK;
    }

    if (SOC_IS_HERCULES(unit)) {
        mask = BCM_PORT_HERC_ATTRS;
    } else {
        mask = BCM_PORT_ATTR_ALL_MASK;
    }

    info_all = sal_alloc(SOC_MAX_NUM_PORTS * sizeof(bcm_port_info_t),
                         "if_ea_port_stat");
    if (info_all == NULL) {
        cli_out("Insufficient memory.\n");
        return CMD_FAIL;
    }

    PBMP_ITER(pbm, p) {
        tk371x_port_info_init(unit, p, &info_all[p], mask);
        if ((r = _bcm_tk371x_port_selective_get(unit, p, &info_all[p])) < 0) {
            cli_out("%s: Error: Could not get port %s information: %s\n",
                    ARG_CMD(a), SOC_PORT_NAME(unit, p), bcm_errmsg(r));
            sal_free(info_all);
            return (CMD_FAIL);
        }
    }

    tk371x_brief_port_info_header(unit);

#define _call_bpi(pbm, pbm_mask) \
    tmp_pbm = pbm_mask; \
    BCM_PBMP_AND(tmp_pbm, pbm); \
    PBMP_ITER(tmp_pbm, p) { \
        tk371x_brief_port_info(SOC_PORT_NAME(unit, p), &info_all[p], mask); \
    }

    _call_bpi(pbm, PBMP_FE_ALL(unit));
    _call_bpi(pbm, PBMP_GE_ALL(unit));
    _call_bpi(pbm, PBMP_XE_ALL(unit));
    _call_bpi(pbm, PBMP_HG_ALL(unit));

    sal_free(info_all);

    return CMD_OK;
}                      

#endif /* BCM_TK371X_SUPPORT */
