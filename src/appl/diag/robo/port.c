/*
 * $Id: port.c,v 1.88 Broadcom SDK $
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

#include <bcm_int/robo/port.h>
#include <bcm_int/robo/cosq.h>

static int prev_robo_linkscan_interval[BCM_MAX_NUM_UNITS];

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
*robo_if_fmt_speed(char *b, int speed)
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
char    *robo_discard_mode[] = {
    "None", "All", "Untag", "Tag", "Ingress", "Egress", NULL
};
/* Note:  See link.h, bcm_linkscan_mode_e */
char            *robo_linkscan_mode[] = {
    "None", "SW", "HW", "OFF", "ON", NULL
};
/* Note:  See portmode.h, soc_port_if_e */
char            *robo_interface_mode[] = SOC_PORT_IF_NAMES_INITIALIZER;
/* Note:  See portmode.h, soc_port_ms_e */
char            *robo_phymaster_mode[] = {
    "Slave", "Master", "Auto", "None", NULL
};
/* Note:  See port.h, bcm_port_loopback_e */
char            *robo_loopback_mode[] = {
    "NONE", "MAC", "PHY", NULL
};
/* Note:  See port.h, bcm_port_stp_e */
char            *robo_forward_mode[] = {
    "Disable", "Block", "LIsten", "LEarn", "Forward", NULL
};
/* Note: See port.h, bcm_port_encap_e */
char            *robo_encap_mode[] = {
    "IEEE", "HIGIG", "B5632", "Reserved", NULL
};
/* Note: See port.h, bcm_port_medium_t */
char           *robo_medium_status[] = {
    "None", "Copper", "Fiber", NULL
};


void
robo_brief_port_info_header(int unit)
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
    "%7s  "          /* discard mode */
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

#define _CHECK_PRINT(flags, mask, str, val) do {                       \
    if ((flags) & (mask)) {                                            \
        cli_out(\
                str, val);                                          \
    } else {                                                           \
        cli_out(\
                str, "");                                           \
    }                                                                  \
} while (0)

int
robo_brief_port_info(char *port_ref, bcm_port_info_t *info, uint32 flags)
{
    char        *spt_str, *discrd_str;
    char        sbuf[6];
    char        lrn_str[4];

    spt_str = ROBO_FORWARD_MODE(info->stp_state);
    discrd_str = ROBO_DISCARD_MODE(info->discard);

    /* port number (4)
     * enabled (3)
     * link state (3)
     * speed/duplex (6)
     * link scan mode (4)
     * auto negotiate? (4)
     * spantree state (7)
     * pause tx/rx (5)
     * discard mode (7)
     * learn to CPU, ARL, FWD or discard (3)
     */
    cli_out("%4s  %3s %4s  ", port_ref, 
            info->enable ? "En" : "DIS", 
            info->linkstatus ? " up " : "down");
    _CHECK_PRINT(flags, BCM_PORT_ATTR_SPEED_MASK,
                 "%5s ", robo_if_fmt_speed(sbuf, info->speed));
    _CHECK_PRINT(flags, BCM_PORT_ATTR_DUPLEX_MASK,
                 "%2s ",  info->speed == 0 ? "" : info->duplex ? "FD" : "HD");
    _CHECK_PRINT(flags, BCM_PORT_ATTR_LINKSCAN_MASK,
                 " %4s  ", robo_linkscan_mode[info->linkscan]);
    _CHECK_PRINT(flags, BCM_PORT_ATTR_AUTONEG_MASK,
                 "%4s  ", info->autoneg ? " Yes" : " No ");
    _CHECK_PRINT(flags, BCM_PORT_ATTR_STP_STATE_MASK,
                 "%7s  ", spt_str);
    _CHECK_PRINT(flags, BCM_PORT_ATTR_PAUSE_MASK,
                 "%2s ", info->pause_tx ? "TX" : " ");
    _CHECK_PRINT(flags, BCM_PORT_ATTR_PAUSE_MASK,
                 "%2s ", info->pause_rx ? "RX" : " ");
    _CHECK_PRINT(flags, BCM_PORT_ATTR_DISCARD_MASK,
                 "%7s   ", discrd_str);

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
        _CHECK_PRINT(flags, BCM_PORT_ATTR_INTERFACE_MASK,
                     "%6s  ", robo_interface_mode[info->interface]);
    }

    cli_out("\n");
    return 0;
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
if_robo_port_stat(int unit, args_t *a)
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
                         "if_robo_port_stat");
    if (info_all == NULL) {
        cli_out("Insufficient memory.\n");
        return CMD_FAIL;
    }

    PBMP_ITER(pbm, p) {
        robo_port_info_init(unit, p, &info_all[p], mask);
        if ((r = bcm_port_selective_get(unit, p, &info_all[p])) < 0) {
            cli_out("%s: Error: Could not get port %s information: %s\n",
                    ARG_CMD(a), SOC_PORT_NAME(unit, p), bcm_errmsg(r));
            sal_free(info_all);
            return (CMD_FAIL);
        }
    }

    robo_brief_port_info_header(unit);

#define _call_bpi(pbm, pbm_mask) \
    tmp_pbm = pbm_mask; \
    BCM_PBMP_AND(tmp_pbm, pbm); \
    PBMP_ITER(tmp_pbm, p) { \
        robo_brief_port_info(SOC_PORT_NAME(unit, p), &info_all[p], mask); \
    }

    _call_bpi(pbm, PBMP_FE_ALL(unit));
    _call_bpi(pbm, PBMP_GE_ALL(unit));
    _call_bpi(pbm, PBMP_XE_ALL(unit));
    _call_bpi(pbm, PBMP_HG_ALL(unit));

    sal_free(info_all);

    return CMD_OK;
}

/*
 * Function:
 *  if_port_rate
 * Purpose:
 *  Set/display of port rate metering characteristics
 * Parameters:
 *  unit - SOC unit #
 *  a - pointer to args
 * Returns:
 *  CMD_OK/CMD_FAIL
 */
cmd_result_t
if_robo_port_rate(int unit, args_t *a)
{
    pbmp_t          pbm;
    soc_port_t      p;
    int             operation = 0;
    int             rv;
    int             header;
    uint32          rate = 0xFFFFFFFF;
    uint32          burst = 0xFFFFFFFF;
    uint32          bw_flags = 0;
    char           *c;
    uint32  reg_value = 0, temp = 0;
    int queue_n = -1;
#ifdef BCM_TB_SUPPORT
    int rate_type = 0;
#endif

#define SHOW    1
#define INGRESS 2
#define EGRESS  4
#define PAUSE   8
#define PER_QUEUE_EGRESS  0x10

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if ((c = ARG_GET(a)) == NULL) {
        BCM_PBMP_ASSIGN(pbm, PBMP_ALL(unit));
    } else if (parse_pbmp(unit, c, &pbm) < 0) {
        cli_out("%s: Error: unrecognized port bitmap: %s\n",
                ARG_CMD(a), c);
        return CMD_FAIL;
    }

    /* Apply PortRate only to those ports which support it */
    BCM_PBMP_AND(pbm, PBMP_ALL(unit));
    if (BCM_PBMP_IS_NULL(pbm)) {
        cli_out("No ports specified.\n");
        return CMD_OK;
    }
    
    /* Ingress, egress or show both */
    if ((c = ARG_GET(a)) == NULL) {
        operation = (SHOW | INGRESS | EGRESS | PER_QUEUE_EGRESS);
    }
    else if (!sal_strncasecmp(c, "ingress", strlen(c))) {
        operation = INGRESS;
    }
    else if (!sal_strncasecmp(c, "egress", strlen(c))) {
        operation = EGRESS;
    }
    else if (!sal_strncasecmp(c, "queue_egress", strlen(c))) {
        operation = PER_QUEUE_EGRESS;
    }
    else if (!sal_strncasecmp(c, "pause", strlen(c))) {
        operation = PAUSE;
    }
    else {
        cli_out("%s: Error: unrecognized port rate type: %s\n",
                ARG_CMD(a), c);
        return CMD_FAIL;
    }

    /* Set or get */
    if (operation == PER_QUEUE_EGRESS) {
        if ((c = ARG_GET(a)) != NULL) {
            queue_n = parse_integer(c);
            if ((c = ARG_GET(a)) != NULL) {
                rate = parse_integer(c);
                if ((c = ARG_GET(a)) != NULL) {
                    burst = parse_integer(c);
                }
                else {
                    cli_out("%s: Error: missing port burst size\n",
                            ARG_CMD(a));
                    return CMD_FAIL;
                }
            }
            else {
                operation |= SHOW; /* show specific queue */
            }
        }
        else {
            operation |= SHOW; /* show all queues */
        }
    } else {
        if ((c = ARG_GET(a)) != NULL) {
            rate = parse_integer(c);
            if ((c = ARG_GET(a)) != NULL) {
                burst = parse_integer(c);
            }
            else {
                cli_out("%s: Error: missing port burst size\n",
                        ARG_CMD(a));
                return CMD_FAIL;
            }
            if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                if ((c = ARG_GET(a)) != NULL) {
                    rate_type = parse_integer(c);
                } else {
                    rate_type = 0; /* default is kbps */
                }
#endif
            }
        }
        else {
            operation |= SHOW;
        }
    }

    PBMP_ITER(pbm, p) {
        if (operation & SHOW) {
            /* Display current setting */
            header = 0;
            if (operation & (INGRESS | PAUSE) ) {
                rv = bcm_port_rate_ingress_get(unit, p, &rate, &burst); 
                if (rv < 0) {
                    cli_out("%4s:", SOC_PORT_NAME(unit, p));
                    cli_out("\tIngress meter:  %s.\n",bcm_errmsg(rv));
                    continue;
                }
                if (rate) {
                    cli_out("%4s:", SOC_PORT_NAME(unit, p));
                    header = 1;

                    cli_out("\tIngress meter: "
                            "%8d kbps %8d kbits max burst.\n",
                            rate, burst);
                    rv = bcm_port_rate_pause_get(unit, p, &rate, &burst); 
                    if (rv < 0) {
                        cli_out("\tbcm_port_rate_pause_get:  %s.\n",bcm_errmsg(rv));
                        continue;
                    }
                    if (burst) { 
                        header = 1;

                        cli_out("\tPause frames: Pause = %8d kbits, "
                                "Resume = %8d kbits.\n", rate, burst);

                    }
                } else {
                    cli_out("%4s:", SOC_PORT_NAME(unit, p));
                    header = 1;
                    cli_out("\tIngress meter:  value not set yet.\n");
                }
            }
            
            if (operation & EGRESS) { 
                if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                    SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
                    rv = bcm_port_rate_egress_get(unit, p, &rate, &burst);
                if (rv < 0) {
                    if (!header) {
                        cli_out("%4s:", SOC_PORT_NAME(unit, p));
                    }
                    cli_out("\tEgress meter:  %s.\n",bcm_errmsg(rv));
                        continue;
                    }
                    if (rate) { 
                        if (!header) {
                            cli_out("%4s:", SOC_PORT_NAME(unit, p));
                        }
                        /* Rate for dual IMP0/IMP1 in terms of Packets Per Second(PPS) */
                        if (IS_CPU_PORT(unit, p)) {
                            cli_out("%4s:", "(IMP0)");
                            cli_out("\tEgress meter:  %8d pps.\n", rate);
                        } else {
                            SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
                                (unit, &reg_value));

                            SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
                                (unit, &reg_value, FRM_MNGPf, &temp));
                     
                            /* if enable Dual-IMP port (IMP0 and IMP1) */
                            if ((temp == 0x3) && (p == 5)) {
                                cli_out("%4s:", "(IMP1)");
                                cli_out("\tEgress meter:  %8d pps.\n", rate);
                            } else {
                                cli_out("\tEgress meter:  %8d kbps %8d kbits max burst.\n",
                                        rate, burst);
                            }
                        }
                    } else {
                        if (!header) {
                            cli_out("%4s:", SOC_PORT_NAME(unit, p));
                        }
                        cli_out("\tEgress meter:  value not set yet.\n");
                    }
               } else {
                    rv = bcm_port_rate_egress_get(unit, p, &rate, &burst);
                    if (rv < 0) {
                       if (!header) {
                           cli_out("%4s:", SOC_PORT_NAME(unit, p));
                       }
                       cli_out("\tEgress meter:  %s.\n",bcm_errmsg(rv));
                       continue;
                    }
                    if (rate) { 
                        if (!header) {
                            cli_out("%4s:", SOC_PORT_NAME(unit, p));
                        }
                        if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                            rv = bcm_cosq_control_get
                                (unit, p, -1, bcmCosqControlEgressRateType, &rate_type);
                            if (rv < 0) {
                                cli_out("\tEgress meter:  %s.\n",bcm_errmsg(rv));
                                continue;
                            }
                            cli_out("\tEgress meter:  %8d %s %8d kbits max burst.\n", 
                                    rate, (rate_type) ? "pps" : "kbps", burst);
#endif
                        } else {
                            cli_out("\tEgress meter:  %8d kbps %8d kbits max burst.\n",
                                    rate, burst);
                        }
                    } else {
                        if (!header) {
                            cli_out("%4s:", SOC_PORT_NAME(unit, p));
                        }
                        cli_out("\tEgress meter:  value not set yet.\n");
                    }
                }
            }

            if (operation & PER_QUEUE_EGRESS) { 
                if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                rv = bcm_cosq_control_get
                    (unit, p, -1, bcmCosqControlEgressRateType, &rate_type);
                if (rv < 0) {
                    if (!header) {
                        cli_out("%4s:", SOC_PORT_NAME(unit, p));
                    }
                    cli_out("\tQueue #%d Egress meter:  %s.\n",queue_n, bcm_errmsg(rv));
                    continue;
                }
#endif
                }
                if (queue_n >= 0) {
                    rv = bcm_cosq_port_bandwidth_get(unit, p, queue_n, &rate, &burst, &bw_flags);
                    if (rv < 0) {
                       if (!header) {
                           cli_out("%4s:", SOC_PORT_NAME(unit, p));
                       }
                       cli_out("\tQueue #%d Egress meter:  %s.\n",queue_n, bcm_errmsg(rv));
                       continue;
                    }
                    if (rate) { 
                        if (!header) {
                           cli_out("%4s:", SOC_PORT_NAME(unit, p));
                        }
                        if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                            cli_out("\tEgress meter:  %8d %s %8d kbits max burst.\n", 
                                    rate, (rate_type) ? "pps" : "kbps", burst);
#endif
                        } else {
                            cli_out("\tQueue #%d Egress meter:  %8d kbps %8d kbits max burst.\n",
                                    queue_n, rate, burst);
                        }
                    } else {
                        if (!header) {
                          cli_out("%4s:", SOC_PORT_NAME(unit, p));
                        }
                        cli_out("\tQueue #%d Egress meter:  value not set yet.\n", queue_n);
                    }
                } else {
                    int i, current_q;
                    rv = bcm_cosq_config_get(unit, &current_q);
                    if (rv < 0) {
                       if (!header) {
                           cli_out("%4s:", SOC_PORT_NAME(unit, p));
                       }
                       cli_out("\tPer Queue Egress meter:  %s.\n",bcm_errmsg(rv));
                       continue;
                    }

                    if (!header) {
                       cli_out("%4s:", SOC_PORT_NAME(unit, p));
                    }

                    for (i = 0; i < current_q; i++) {
                        rv = bcm_cosq_port_bandwidth_get(unit, p, i, &rate, &burst, &bw_flags);
                        if (rv < 0) {
                           cli_out("\tQueue #%d Egress meter:  %s.\n",i,bcm_errmsg(rv));
                           continue;
                        }
                        if (rate) { 
                            if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                                cli_out("\tQueue #%d Egress meter:  %8d %s %8d kbits max burst.\n",
                                        i, rate, (rate_type) ? "pps" : "kbps", burst);
#endif
                            } else {
                                cli_out("\tQueue #%d Egress meter:  %8d kbps %8d kbits max burst.\n",
                                        i, rate, burst);
                            }
                        } else {
                            cli_out("\tQueue #%d Egress meter:  value not set yet.\n", i);
                        }
                    }
                }
            }
        } else {
            /* New setting */
            if (!rate || !burst) {
                rate = burst = 0; /* Disable port metering */
            }
            if (operation & INGRESS) {
                rv = bcm_port_rate_ingress_set(unit, p, rate, burst); 
                if (rv < 0) {
                    cli_out("%s: Error: bcm_port_rate_ingress_set() failed: %s\n",
                            ARG_CMD(a), bcm_errmsg(rv));
                    return CMD_OK;
                }
            } else if (operation & PAUSE) {
                rv = bcm_port_rate_pause_set(unit, p, rate, burst); 
                if (rv < 0) {
                    cli_out("%s: ERROR: bcm_port_rate_pause_set: %s\n",
                            ARG_CMD(a), bcm_errmsg(rv));
                    return CMD_OK;
                }
            } else if (operation & EGRESS) {
                if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                    rv = bcm_cosq_control_set
                        (unit, p, -1, bcmCosqControlEgressRateType, (rate_type) ? 1 : 0);
                    if (rv < 0) {
                        cli_out("%s: Error: bcm_cosq_control_set() failed: %s\n",
                                ARG_CMD(a), bcm_errmsg(rv));
                        return CMD_OK;
                    }
#endif
                }
                rv = bcm_port_rate_egress_set(unit, p, rate, burst);
                if (rv < 0) {
                    cli_out("%s: Error: bcm_port_rate_egress_set() failed: %s\n",
                            ARG_CMD(a), bcm_errmsg(rv));
                    return CMD_OK;
                }
            } else if (operation & PER_QUEUE_EGRESS) {
                rv = bcm_cosq_port_bandwidth_set(unit, p, queue_n, rate, burst, 0); 
                if (rv < 0) {
                    cli_out("%s: Error: bcm_cosq_port_bandwidth_set() failed: %s\n",
                            ARG_CMD(a), bcm_errmsg(rv));
                    return CMD_OK;
                }
            }
        }
    }

#undef SHOW
#undef INGRESS
#undef EGRESS
#undef PAUSE
#undef PER_QUEUE_EGRESS
    return CMD_OK;
}

/*
 * Function:
 *      if_robo_port_samp_rate
 * Purpose:
 *      Set/display of sflow port sampling rates.
 * Parameters:
 *      unit - SOC unit #
 *      args - pointer to comand line arguments
 * Returns:
 *      CMD_OK/CMD_FAIL
 */
char if_robo_port_samp_rate_usage[] =
    "Set/Display port sampling rate characteristics.\n"
    "Parameters: <pbm> [ingress_rate] [egress_rate]\n"
    "\tOn average, every 1/ingress_rate packets will be sampled.\n"
    "\tA rate of 0 indicates no sampling.\n"
    "\tA rate of 1 indicates all packets sampled.\n";

cmd_result_t
if_robo_port_samp_rate(int unit, args_t *args)
{
#define SHOW    0x01
#define SET     0x02
    pbmp_t          pbm;
    char           *ch;
    int             operation    = SET; /* Set or Show */
    int             ingress_rate = -1;
    int             egress_rate  = -1;
    soc_port_t      soc_port;
    int             retval;

    if (!sh_check_attached(ARG_CMD(args), unit)) {
        return CMD_FAIL;
    }

    /* get port bitmap */
    if ((ch = ARG_GET(args)) == NULL) {
        BCM_PBMP_ASSIGN(pbm, PBMP_PORT_ALL(unit));
    } else if (parse_pbmp(unit, ch, &pbm) < 0) {
        cli_out("%s: Error: unrecognized port bitmap: %s\n",
                ARG_CMD(args), ch);
        return CMD_FAIL;
    }

    /* read in ingress_rate and egress_rate if given */
    if ((ch = ARG_GET(args)) != NULL) {
        ingress_rate = parse_integer(ch);
        if ((ch = ARG_GET(args)) != NULL) {
            egress_rate = parse_integer(ch);
        }
        else {
            cli_out("%s: Error: missing egress rate \n", ARG_CMD(args));
            return CMD_FAIL;
        }
    }
    else {
        operation = SHOW;
    }

    /* Iterate through port bitmap and perform 'operation' on them. */
    PBMP_ITER(pbm, soc_port) {
        if (operation == SHOW) {
        /* Show port sflow sample rate(s) */
            retval = bcm_port_sample_rate_get(unit, soc_port, &ingress_rate,
                                              &egress_rate);
            if (retval != BCM_E_NONE) {
                cli_out("%s port %s: ERROR: bcm_port_sample_rate_get: "
                        "%s\n", ARG_CMD(args),
                        SOC_PORT_NAME(unit, soc_port), bcm_errmsg(retval));
                return CMD_FAIL;
            }

            cli_out("%4s:", SOC_PORT_NAME(unit, soc_port));
 
            if ( ingress_rate == 0 ) {
                cli_out("\tingress: not sampling,");
            }
            else {
                cli_out("\tingress: 1 out of %d packets,", ingress_rate);
            }
            if ( egress_rate == 0 ) {
                cli_out("\tegress: not sampling,");
            }
            else {
                cli_out("\tegress: 1 out of %d packets,", egress_rate);
            }
            cli_out("\n");
        }
        else {
        /* Set port sflow sample rate(s) */
            retval = bcm_port_sample_rate_set(unit, soc_port, ingress_rate, egress_rate);
            if (retval != BCM_E_NONE) {
                cli_out("%s port %s: ERROR: bcm_port_sample_rate_set: "
                        "%s\n", ARG_CMD(args),
                        SOC_PORT_NAME(unit, soc_port), bcm_errmsg(retval));
                return CMD_FAIL;
            }
        }
    }

#undef SHOW
#undef SET
    return CMD_OK;
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
robo_disp_port_info(char *port_ref, bcm_port_info_t *info, int hg_port,
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
            cli_out("LS(%s) ", robo_linkscan_mode[info->linkscan]);
        }
    }

    if (hg_port) {
        cli_out("%s/XAUI(", robo_encap_mode[info->encap_mode & 0x3]);
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
            cli_out("%s", robo_if_fmt_speed(speed_buf, info->speed));
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
                        robo_if_fmt_speed(speed_buf, BCM_PORT_ABIL_SPD_MAX(mode)), 
                        (mode & BCM_PORT_ABIL_FD) ? "FD" : "HD",
                        (mode & BCM_PORT_ABIL_PAUSE_TX) ? "+TXpau" : "",
                        (mode & BCM_PORT_ABIL_PAUSE_RX) ? "+RXpau" : "");
            }

            if ((flags & BCM_PORT_ATTR_REMOTE_ADVERT_MASK) &&
                (info->remote_advert_valid && info->linkstatus)) {
                mode = info->remote_advert;
                cli_out("PeerAdv(%s%s%s%s) ",
                        robo_if_fmt_speed(speed_buf, BCM_PORT_ABIL_SPD_MAX(mode)), 
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
        cli_out("STP(%s) ", robo_forward_mode[info->stp_state]);
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
            cli_out("IF(%s) ", robo_interface_mode[info->interface]);
        }
    }

    if (flags & BCM_PORT_ATTR_PHY_MASTER_MASK) {
        if (info->phy_master >= 0 &&
        info->phy_master < SOC_PORT_MS_COUNT &&
        info->phy_master != SOC_PORT_MS_NONE) {
            cli_out("PH(%s) ", robo_phymaster_mode[info->phy_master]);
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
 *  port_parse_setup
 * Purpose:
 *  Setup the parse table for a port command
 * Parameters:
 *  pt  - the table
 *  info    - port info structure to hold parse results
 * Returns:
 *  Nothing
 */

void
robo_port_parse_setup(int unit, parse_table_t *pt, bcm_port_info_t *info)
{
    int i;

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
                    0, &info->linkscan, robo_linkscan_mode);
    parse_table_add(pt, "LeaRN",   PQ_INT | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->learn, 0);
    parse_table_add(pt, "DISCard", PQ_MULTI | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->discard, robo_discard_mode);
    parse_table_add(pt, "VlanFilter", PQ_BOOL | PQ_NO_EQ_OPT,
                    0, &info->vlanfilter, 0);
    parse_table_add(pt, "PRIOrity", PQ_INT | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->untagged_priority, 0);
    parse_table_add(pt, "PortFilterMode", PQ_INT | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->pfm, 0);
    parse_table_add(pt, "PHymaster", PQ_MULTI | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->phy_master, robo_phymaster_mode);
    parse_table_add(pt, "InterFace", PQ_MULTI | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->interface, robo_interface_mode);
    parse_table_add(pt, "LoopBack", PQ_MULTI | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->loopback, robo_loopback_mode);
    parse_table_add(pt, "SpanningTreeProtocol",
                    PQ_MULTI | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->stp_state, robo_forward_mode);
    parse_table_add(pt, "STationADdress", PQ_MAC | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->pause_mac, 0);
    parse_table_add(pt, "TxPAUse",     PQ_BOOL | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->pause_tx, 0);
    parse_table_add(pt, "RxPAUse",     PQ_BOOL | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->pause_rx, 0);
    parse_table_add(pt, "ENCapsulation", PQ_MULTI | PQ_DFL,
                    0, &info->encap_mode, robo_encap_mode);
    parse_table_add(pt, "FrameMaxsize",       PQ_INT | PQ_DFL | PQ_NO_EQ_OPT,
                    0, &info->frame_max, 0);

    if (SOC_IS_HERCULES(unit)) {
        /* For Hercules, ignore some StrataSwitch attributes */
        for (i = 0; i < pt->pt_cnt; i++) {
            if (~BCM_PORT_HERC_ATTRS & port_attr_map[i]) {
                pt->pt_entries[i].pq_type |= PQ_IGNORE;
            }
        }
    } else if (!SOC_IS_XGS(unit)) {
    /* For all non-XGS chips, ignore special XGS attributes */
        for (i = 0; i < pt->pt_cnt; i++) {
            if (BCM_PORT_XGS_ATTRS & port_attr_map[i]) {
                pt->pt_entries[i].pq_type |= PQ_IGNORE;
            }
        }
    }
}

/*
 * Function:
 *  port_parse_mask_get
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
robo_port_parse_mask_get(parse_table_t *pt, uint32 *seen, uint32 *parsed)
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

/* Invalid unit number ( < 0) is permitted */
void
robo_port_info_init(int unit, int port, bcm_port_info_t *info, uint32 actions)
{
    info->action_mask = actions;

    /* We generally need to get link state and autoneg and adverts */
    info->action_mask |= BCM_PORT_ATTR_LINKSTAT_MASK;
    info->action_mask |= BCM_PORT_ATTR_LOCAL_ADVERT_MASK;
    info->action_mask |= BCM_PORT_ATTR_REMOTE_ADVERT_MASK;
    info->action_mask |= BCM_PORT_ATTR_AUTONEG_MASK;
    /* Pickup XGS specific fields
     * NOTE: On Draco, you will only want this on the uplink port.
     */
    if (unit >= 0 && SOC_IS_HERCULES(unit)) {
        info->action_mask |= BCM_PORT_ATTR_ENCAP_MASK;
    }

    /* Clear rate for HG ports */
    if (unit >= 0 && IS_HG_PORT(unit, port)) {
        info->action_mask &= ~(BCM_PORT_ATTR_RATE_MCAST_MASK |
                               BCM_PORT_ATTR_RATE_BCAST_MASK |
                               BCM_PORT_ATTR_RATE_DLFBC_MASK);
    }
}


/*
 * Function:
 *  port_parse_port_info_set
 * Purpose:
 *  Set/change values in a destination according to parsing
 * Parameters:
 *  src - Where to get info from
 *  dest    - Where to put info
 *  flags   - What values to change
 * Returns:
 *  -1 on error.  0 on success
 * Notes:
 *  The speed_max and abilities values must be
 *      set in the src port info structure before this is called.
 *
 *      Assumes linkstat and autoneg are valid in the dest structure
 *      If autoneg is specified in flags, assumes local advert
 *      is valid in the dest structure.
 */

int
robo_port_parse_port_info_set(uint32 flags, bcm_port_info_t *src,
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

/* Iterate thru a port bitmap with the given mask; display info */
STATIC int
_robo_port_disp_iter(int unit, bcm_port_info_t *info, pbmp_t pbm,
                pbmp_t pbm_mask, uint32 seen)
{
    soc_port_t port;
    int r;

    BCM_PBMP_AND(pbm, pbm_mask);
    BCM_PBMP_ITER(pbm, port) {
        robo_port_info_init(unit, port, info, seen);

        if ((r = bcm_port_selective_get(unit, port, info)) < 0) {
            cli_out("Error: Could not get port %s information: %s\n",
                    SOC_PORT_NAME(unit, port), bcm_errmsg(r));
            return (CMD_FAIL);
        }
        robo_disp_port_info(SOC_PORT_NAME(unit, port), info,
                       IS_HG_PORT(unit, port), seen);
    }

    return CMD_OK;
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
if_robo_port(int u, args_t *a)
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
                         "if_robo_port");
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
        robo_port_parse_setup(u, &pt, &given);

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
        robo_port_parse_mask_get(&pt, &seen, &parsed);
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
        if (_robo_port_disp_iter(u, i, p, mp, s) != CMD_OK) { \
             sal_free(info_all); \
             return CMD_FAIL; \
        }
        _call_pdi(u, &info, pbm, PBMP_FE_ALL(u), seen);
        _call_pdi(u, &info, pbm, PBMP_GE_ALL(u), seen);
        _call_pdi(u, &info, pbm, PBMP_XE_ALL(u), seen);
        _call_pdi(u, &info, pbm, PBMP_HG_ALL(u), seen);
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

    if (SOC_IS_HERCULES(u)) {
        parsed &= BCM_PORT_HERC_ATTRS;
    }

    /*
     * Retrieve all requested port information first, then later write
     * back all port information.  That prevents a problem with loopback
     * cables where setting one port's info throws another into autoneg
     * causing it to return info in flux (e.g. suddenly go half duplex).
     */

    PBMP_ITER(pbm, p) {
        robo_port_info_init(u, p, &info_all[p], parsed);
        if ((r = bcm_port_selective_get(u, p, &info_all[p])) < 0) {
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

        if ((r = robo_port_parse_port_info_set(parsed,
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

        if ((r = bcm_port_selective_set(u, p, &info_all[p])) < 0) {
            cli_out("%s: Error: Could not set port %s information: %s\n",
                    ARG_CMD(a), SOC_PORT_NAME(u, p), bcm_errmsg(r));
            cmd_rv = CMD_FAIL;
            continue;
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

char if_robo_egress_usage[] =
    "Usages:\n\t"
    "  egress set [<Port=port#>] [<Modid=modid>] <PBmp=val>\n\t"
    "        - Set allowed egress bitmap for (modid,port) or all\n\t"
    "  egress show [<Port=port#>] [<Module=modid>]\n\t"
    "        - Show allowed egress bitmap for (modid,port)\n";

/*
 * Note:
 *
 * Since these port numbers are likely on different modules, we cannot
 * use PBMP qualifiers by this unit.
 */

cmd_result_t
if_robo_egress(int unit, args_t *a)
{
    char *subcmd, *c;
    int port, arg_port = -1;
    int modid, arg_modid = -1, mod_min = 0;
    int mod_max = SOC_MODID_MAX(unit);
    bcm_pbmp_t pbmp;
    int r;
    bcm_pbmp_t arg_pbmp;
    parse_table_t pt;
    cmd_result_t ret_code;
    char buf[FORMAT_PBMP_MAX];

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    BCM_PBMP_CLEAR(pbmp);
    BCM_PBMP_CLEAR(arg_pbmp);

    /* Egress show command */
    if (sal_strcasecmp(subcmd, "show") == 0) {

        if ((c = ARG_CUR(a)) != NULL) {
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "Port", PQ_DFL|PQ_INT, 0, &arg_port, NULL);
            parse_table_add(&pt, "Modid",   PQ_DFL|PQ_INT,  0, &arg_modid,
                            NULL);
            if (!parseEndOk(a, &pt, &ret_code)) {
                return ret_code;
            }

            /* the module id is 0 : standalone for ROBO */
            if ((arg_modid >= 0) || (arg_port >= 0)) {
                mod_min = mod_max = 0;
            }
    }

    for (modid = mod_min; modid <= mod_max; modid++) {
            PBMP_ITER(PBMP_ALL(unit), port) {
                r = bcm_port_egress_get(unit, port, modid, &pbmp);
                if (r < 0) {
                    cli_out("%s: egress (modid=%d, port=%d) get failed: %s\n",
                            ARG_CMD(a), modid, port, bcm_errmsg(r));
                    return CMD_FAIL;
                }

                format_pbmp(unit, buf, sizeof (buf), pbmp);
                cli_out("Module %d, port %d:  Enabled egress ports %s\n",
                        modid, port, buf);
            }
       }

    return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "set") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Port", PQ_DFL|PQ_INT, 0, &arg_port, NULL);
        parse_table_add(&pt, "Modid",   PQ_DFL|PQ_INT,  0, &arg_modid,  NULL);
        parse_table_add(&pt, "Pbmp", PQ_DFL|PQ_PBMP, 0, &arg_pbmp, NULL);
        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }

        SOC_PBMP_ASSIGN(pbmp, arg_pbmp);

        r = bcm_port_egress_set(unit, arg_port, arg_modid, pbmp);

    } else {
        return CMD_USAGE;
    }

    if (r < 0) {
        cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(r));
        return CMD_FAIL;
    }

    return CMD_OK;
}

cmd_result_t
if_robo_dtag(int unit, args_t *a)
{
    char        *subcmd, *c;
    bcm_pbmp_t      pbmp;
    bcm_port_t      port;
    int         mode, r;
    uint16      tpid;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
    return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
    subcmd = "show";
    }

    c = ARG_GET(a);
    if (c == NULL) {
    BCM_PBMP_ASSIGN(pbmp, PBMP_E_ALL(unit));
    } else {
    if (parse_pbmp(unit, c, &pbmp) < 0) {
        cli_out("%s: ERROR: unrecognized port bitmap: %s\n",
                ARG_CMD(a), c);
        return CMD_FAIL;
    }
    }

    r = 0;
    if (sal_strcasecmp(subcmd, "show") == 0) {
    PBMP_ITER(pbmp, port) {
        r = bcm_port_dtag_mode_get(unit, port, &mode);
        if (r < 0) {
        goto bcm_err;
        }
        r = bcm_port_tpid_get(unit, port, &tpid);
        if (r < 0) {
        goto bcm_err;
        }
        switch (mode) {
        case BCM_PORT_DTAG_MODE_NONE:
        c = "none (disabled)";
        break;
        case BCM_PORT_DTAG_MODE_INTERNAL:
        c = "internal (service provider)";
        break;
        case BCM_PORT_DTAG_MODE_EXTERNAL:
        c = "external (customer)";
        break;
        default:
        c = "unknown";
        break;
        }
        cli_out("port %d:%s\tdouble tag mode %s, tpid 0x%x\n",
                unit, SOC_PORT_NAME(unit, port), c, tpid);
    }
    return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "mode") == 0) {
    c = ARG_GET(a);
    if (c == NULL) {
        PBMP_ITER(pbmp, port) {
        r = bcm_port_dtag_mode_get(unit, port, &mode);
        if (r < 0) {
            goto bcm_err;
        }
        switch (mode) {
        case BCM_PORT_DTAG_MODE_NONE:
            c = "none (disabled)";
            break;
        case BCM_PORT_DTAG_MODE_INTERNAL:
            c = "internal (service provider)";
            break;
        case BCM_PORT_DTAG_MODE_EXTERNAL:
            c = "external (customer)";
            break;
        default:
            c = "unknown";
            break;
        }
        cli_out("port %d:%s\tdouble tag mode %s\n",
                unit, SOC_PORT_NAME(unit, port), c);
        }
        return CMD_OK;
    }
    if (sal_strcasecmp(c, "none") == 0) {
        mode = BCM_PORT_DTAG_MODE_NONE;
    } else if (sal_strcasecmp(c, "internal") == 0) {
        mode = BCM_PORT_DTAG_MODE_INTERNAL;
    } else if (sal_strcasecmp(c, "external") == 0) {
        mode = BCM_PORT_DTAG_MODE_EXTERNAL;
    } else {
        return CMD_USAGE;
    }
    PBMP_ITER(pbmp, port) {
        r = bcm_port_dtag_mode_set(unit, port, mode);
        if (r < 0) {
        goto bcm_err;
        }
    }
    return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "tpid") == 0) {
    c = ARG_GET(a);
    if (c == NULL) {
        PBMP_ITER(pbmp, port) {
        r = bcm_port_tpid_get(unit, port, &tpid);
        if (r < 0) {
            goto bcm_err;
        }
        cli_out("port %d:%s\ttpid 0x%x\n",
                unit, SOC_PORT_NAME(unit, port), tpid);
        }
    } else {
        tpid = parse_integer(c);
        PBMP_ITER(pbmp, port) {
        r = bcm_port_tpid_set(unit, port, tpid);
        if (r < 0) {
            goto bcm_err;
        }
        }
    }
    return CMD_OK;
    }

    return CMD_USAGE;

 bcm_err:
    cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(r));
    return CMD_FAIL;
}
/***********************************************************************
 *
 * Combo port support
 *
 ***********************************************************************/


/*
 * Function:    robo_if_combo_dump
 * Purpose: Dump the contents of a bcm_phy_config_t
 */

STATIC int
robo_if_combo_dump(args_t *a, int u, int p, int medium)
{
    char        pm_str[80];
    bcm_port_medium_t   active_medium;
    int         r;
    bcm_phy_config_t    cfg;

    /*
     * Get active medium so we can put an asterisk next to the status if
     * it is active.
     */

    if ((r = bcm_port_medium_get(u, p, &active_medium)) < 0) {
    return r;
    }
    if ((r = bcm_port_medium_config_get(u, p, medium, &cfg)) < 0) {
    return r;
    }

    cli_out("%s:\t%s medium%s\n",
            SOC_PORT_NAME(u, p),
            ROBO_MEDIUM_STATUS(medium),
            (medium == active_medium) ? " (active)" : "");

    format_port_mode(pm_str, sizeof (pm_str), cfg.autoneg_advert, TRUE);

    cli_out("\tenable=%d preferred=%d "
            "force_speed=%d force_duplex=%d master=%s\n",
            cfg.enable, cfg.preferred,
            cfg.force_speed, cfg.force_duplex,
            ROBO_PHYMASTER_MODE(cfg.master));
    cli_out("\tautoneg_enable=%d autoneg_advert=%s(0x%x)\n",
            cfg.autoneg_enable, pm_str, cfg.autoneg_advert);

    return BCM_E_NONE;
}

static int combo_watch[SOC_MAX_NUM_DEVICES][SOC_MAX_NUM_PORTS];

static void
if_combo_watch(int unit, bcm_port_t port, bcm_port_medium_t medium, void *arg) 
{
    cli_out("Unit %d: %s: Active medium switched to %s\n",
            unit, SOC_PORT_NAME(unit, port), ROBO_MEDIUM_STATUS(medium));

    /* 
     * Increment the number of medium changes. Remember, that we pass the 
     * address of combo_watch[unit][port] when we register this callback
     */
    (*((int *)arg))++; 
}

/*
 * Function:    if_combo
 * Purpose: Control combo ports
 * Parameters:  u - SOC unit #
 *      a - pointer to args
 * Returns: CMD_OK/CMD_FAIL/
 */
cmd_result_t
if_robo_combo(int u, args_t *a)
{
    pbmp_t      pbm;
    soc_port_t      p;
    int         specified_medium = BCM_PORT_MEDIUM_COUNT;
    int         r, rc, rf;
    char        *c;
    parse_table_t   pt;
    bcm_phy_config_t    cfg, cfg_opt;

    enum if_combo_cmd_e {
        COMBO_CMD_DUMP,
        COMBO_CMD_SET,
        COMBO_CMD_WATCH,
        CONBO_CMD_COUNT
    } cmd;

    enum if_combo_watch_arg_e {
        COMBO_CMD_WATCH_SHOW,
        COMBO_CMD_WATCH_ON,
        COMBO_CMD_WATCH_OFF,
        COMBO_CMD_WATCH_COUNT
    } watch_arg = COMBO_CMD_WATCH_SHOW;

    cfg_opt.enable = -1;
    cfg_opt.preferred = -1;
    cfg_opt.autoneg_enable = -1;
    cfg_opt.autoneg_advert = 0xffffffff;
    cfg_opt.force_speed = -1;
    cfg_opt.force_duplex = -1;
    cfg_opt.master = -1;
    cfg_opt.mdix = -1;

    if (!sh_check_attached(ARG_CMD(a), u)) {
    return CMD_FAIL;
    }

    if ((c = ARG_GET(a)) == NULL) {
    return CMD_USAGE;
    }

    if (parse_pbmp(u, c, &pbm) < 0) {
    cli_out("%s: ERROR: unrecognized port bitmap: %s\n",
            ARG_CMD(a), c);
    return CMD_FAIL;
    }

    SOC_PBMP_AND(pbm, PBMP_PORT_ALL(u));

    c = ARG_GET(a);     /* NULL or media type or command */

    if (c == NULL) {
        cmd = COMBO_CMD_DUMP;
        specified_medium = BCM_PORT_MEDIUM_COUNT;
    } else if (sal_strcasecmp(c, "copper") == 0 || 
               sal_strcasecmp(c, "c") == 0) {
        cmd = COMBO_CMD_SET;
        specified_medium = BCM_PORT_MEDIUM_COPPER;
    } else if (sal_strcasecmp(c, "fiber") == 0 || 
               sal_strcasecmp(c, "f") == 0) {
        cmd = COMBO_CMD_SET;
        specified_medium = BCM_PORT_MEDIUM_FIBER;
    } else if (sal_strcasecmp(c, "watch") == 0 || 
               sal_strcasecmp(c, "w") == 0) {
        cmd = COMBO_CMD_WATCH;
    } else {
        return CMD_USAGE;
    }

    switch (cmd) {
    case COMBO_CMD_SET:
    if ((c = ARG_CUR(a)) != NULL) {
        if (c[0] == '=') {
        return CMD_USAGE;   /* '=' unsupported */
        }

        parse_table_init(u, &pt);
        parse_table_add(&pt, "Enable", PQ_DFL|PQ_BOOL, 0,
                &cfg_opt.enable, 0);
        parse_table_add(&pt, "PREFerred", PQ_DFL|PQ_BOOL, 0,
                &cfg_opt.preferred, 0);
        parse_table_add(&pt, "Autoneg_Enable", PQ_DFL|PQ_BOOL, 0,
                &cfg_opt.autoneg_enable, 0);
        parse_table_add(&pt, "Autoneg_Advert", PQ_DFL|PQ_PORTMODE, 0,
                &cfg_opt.autoneg_advert, 0);
        parse_table_add(&pt, "Force_Speed", PQ_DFL|PQ_INT, 0,
                &cfg_opt.force_speed, 0);
        parse_table_add(&pt, "Force_Duplex", PQ_DFL|PQ_BOOL, 0,
                &cfg_opt.force_duplex, 0);
        parse_table_add(&pt, "MAster", PQ_DFL|PQ_BOOL, 0,
                &cfg_opt.master, 0);

        if (parse_arg_eq(a, &pt) < 0) {
        parse_arg_eq_done(&pt);
        return CMD_USAGE;
        }
        parse_arg_eq_done(&pt);

        if (ARG_CUR(a) != NULL) {
        return CMD_USAGE;
        }
    } else {
            cmd = COMBO_CMD_DUMP;
        }

        break;
        
    case COMBO_CMD_WATCH:
        c = ARG_GET(a);

        if (c == NULL) {
            watch_arg = COMBO_CMD_WATCH_SHOW;
        } else if (sal_strcasecmp(c, "on") == 0) {
            watch_arg = COMBO_CMD_WATCH_ON;
        } else if (sal_strcasecmp(c, "off") == 0) {
            watch_arg = COMBO_CMD_WATCH_OFF;
        } else {
            return CMD_USAGE;
        }
        break;

    default:
        break;
    }

    PBMP_ITER(pbm, p) {
    switch (cmd) {
        case COMBO_CMD_DUMP:
        cli_out("Port %s:\n", SOC_PORT_NAME(u, p));
            
            rc = rf = BCM_E_UNAVAIL;
            if (specified_medium == BCM_PORT_MEDIUM_COPPER ||
                specified_medium == BCM_PORT_MEDIUM_COUNT) {
                rc = robo_if_combo_dump(a, u, p, BCM_PORT_MEDIUM_COPPER);
            } 

            if (specified_medium == BCM_PORT_MEDIUM_FIBER ||
                specified_medium == BCM_PORT_MEDIUM_COUNT) {
                rf = robo_if_combo_dump(a, u, p, BCM_PORT_MEDIUM_FIBER);
                if (rf != BCM_E_NONE && rf != BCM_E_UNAVAIL) {
                    cli_out("%s:\tERROR(fiber): %s\n",
                            SOC_PORT_NAME(u, p),
                            bcm_errmsg(rc));
                }
            }

            /*
             * If there were problems getting medium-specific info on 
             * individual mediums, then they will be printed above. However,
             * if BCM_E_UNAVAIL is returned for both copper and fiber mediums
             * we'll print only one error message
             */
            if (rc == BCM_E_UNAVAIL && rf == BCM_E_UNAVAIL) {
                cli_out("%s:\tmedium info unavailable\n",
                        SOC_PORT_NAME(u, p));
            } 
           break;

        case COMBO_CMD_SET:
            /*
             * Update the medium operating mode.
             */
            r = bcm_port_medium_config_get(u, p,
                                           specified_medium,
                                           &cfg);

            if (r < 0) {
                cli_out("%s: port %s: Error getting medium config: %s\n",
                        ARG_CMD(a), SOC_PORT_NAME(u, p), bcm_errmsg(r));
                return CMD_FAIL;
            }

            if (cfg_opt.enable != -1) {
                cfg.enable = cfg_opt.enable;
            }
            
            if (cfg_opt.preferred != -1) {
                cfg.preferred = cfg_opt.preferred;
            }
            
            if (cfg_opt.autoneg_enable != -1) {
                cfg.autoneg_enable = cfg_opt.autoneg_enable;
            }
            
            if (cfg_opt.autoneg_advert != 0xffffffff) {
                cfg.autoneg_advert = cfg_opt.autoneg_advert;
            }
            
            if (cfg_opt.force_speed != -1) {
                cfg.force_speed = cfg_opt.force_speed;
            }
            
            if (cfg_opt.force_duplex != -1) {
                cfg.force_duplex = cfg_opt.force_duplex;
            }
            
            if (cfg_opt.master != -1) {
                cfg.master = cfg_opt.master;
            }
            
            if (cfg_opt.mdix != -1) {
                cfg.mdix = cfg_opt.mdix;
            }
            
            r = bcm_port_medium_config_set(u, p,
                                           specified_medium,
                                           &cfg);
        
            if (r < 0) {
                cli_out("%s: port %s: Error setting medium config: %s\n",
                        ARG_CMD(a), SOC_PORT_NAME(u, p), bcm_errmsg(r));
                return CMD_FAIL;
            }

            break;

        case COMBO_CMD_WATCH:
            switch (watch_arg) {
            case COMBO_CMD_WATCH_SHOW:
                if (combo_watch[u][p]) {
                    cli_out("Port %s: Medium status change watch is  ON. "
                            "Medim changed %d times\n",
                            SOC_PORT_NAME(u, p),
                            combo_watch[u][p] - 1);
                } else {
                    cli_out("Port %s: Medium status change watch is OFF.\n",
                            SOC_PORT_NAME(u, p));
                }
                break;

            case COMBO_CMD_WATCH_ON:
                if (!combo_watch[u][p]) {
                    r = bcm_port_medium_status_register(u, p, 
                                                        if_combo_watch,
                                                        &combo_watch[u][p]);
                    if (r < 0) {
                        cli_out("Error registerinig medium status change "
                                "callback for %s: %s\n",
                                SOC_PORT_NAME(u, p), soc_errmsg(r));
                        return (CMD_FAIL);
                    }

                    combo_watch[u][p] = 1;
                }

                cli_out("Port %s: Medium change watch is ON\n",
                        SOC_PORT_NAME(u, p));

                break;

            case COMBO_CMD_WATCH_OFF:
                if (combo_watch[u][p]) {
                    r = bcm_port_medium_status_unregister(u, p, 
                                                          if_combo_watch,
                                                          &combo_watch[u][p]);
                    if (r < 0) {
                        cli_out("Error unregisterinig medium status change "
                                "callback for %s: %s\n",
                                SOC_PORT_NAME(u, p), soc_errmsg(r));
                        return (CMD_FAIL);
                    }

                    combo_watch[u][p] = 0;
                }

                cli_out("Port %s: Medium change watch is OFF\n",
                        SOC_PORT_NAME(u, p));

                break;

            default:
                return CMD_FAIL;
            }
           
            break;
        /* coverity[dead_error_begin] */
        default:
            return CMD_FAIL;
        }
    }

    return CMD_OK;
}


/*
 * Function:
 *  cmd_cablediag
 * Purpose:
 *  Run cable diagnostics (if available)
 */

cmd_result_t
cmd_robo_cablediag(int unit, args_t *a)
{
    char    *s;
    bcm_pbmp_t  pbm;
    bcm_port_t  port;
    int     rv, i;
    bcm_port_cable_diag_t   cd;
    static char *statename[] = _SHR_PORT_CABLE_STATE_NAMES_INITIALIZER;
    char pfmt[SOC_PBMP_FMT_LEN];

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if ((s = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    BCM_PBMP_CLEAR(pbm);
    if (parse_pbmp(unit, s, &pbm) < 0) {
        cli_out("%s: ERROR: unrecognized port bitmap: %s\n",
                ARG_CMD(a), s);
        return CMD_FAIL;
    }

    if ((rv = bcm_linkscan_mode_set_pbm(unit, pbm, BCM_LINKSCAN_MODE_NONE)) < 0) {
        cli_out("%s: Failed to disable link scanning: PBM=%s: %s\n",
                ARG_CMD(a), SOC_PBMP_FMT(pbm, pfmt), bcm_errmsg(rv));
        return(CMD_FAIL);
    }

    PBMP_ITER(pbm, port) {
        if (SOC_IS_TB_B0(unit) && IS_FE_PORT(unit, port)) {
            cli_out("Waiting 6 seconds for phy status update.\n");
            sal_usleep(6000000);
        }
        rv = bcm_port_cable_diag(unit, port, &cd);
        if (rv < 0) {
            cli_out("%s: ERROR: port %s: %s\n",
                    ARG_CMD(a), SOC_PORT_NAME(unit, port), bcm_errmsg(rv));
            continue;
        }
        if (cd.fuzz_len == 0) {
            cli_out("port %s: cable (%d pairs)\n",
                    SOC_PORT_NAME(unit, port), cd.npairs);
        } else {
            cli_out("port %s: cable (%d pairs, length +/- %d meters)\n",
                    SOC_PORT_NAME(unit, port), cd.npairs, cd.fuzz_len);
        }
        for (i = 0; i < cd.npairs; i++) {
            if (cd.pair_state[i] != BCM_PORT_CABLE_STATE_OK) {
                 cli_out("\tpair %c %s, length %d meters\n",
                         'A' + i, statename[cd.pair_state[i]], cd.pair_len[i]);
            } else {
                 cli_out("\tpair %c %s\n",
                         'A' + i, statename[cd.pair_state[i]]);
            }
        }
    }

    if ((rv = bcm_linkscan_mode_set_pbm(unit, pbm, BCM_LINKSCAN_MODE_SW)) < 0) {
        cli_out("%s: Failed to recover link scanning: PBM=%s: %s\n",
                ARG_CMD(a), SOC_PBMP_FMT(pbm, pfmt), bcm_errmsg(rv));
        return(CMD_FAIL);
    }
    return CMD_OK;
}

/* Must stay in sync with bcm_color_t enum (bcm/types.h) */
const char *diag_robo_parse_color[] = {
    "Green",
    "Yellow",
    "Red",
    "Preserve",
    NULL
};

char cmd_robo_color_usage[] =
    "Usages:\n\t"
    "  color set Port=<port> Prio=<prio>|CFI=<cfi>\n\t"
    "        Color=<Green|Yellow|Red|Preserve>\n\t"
    "  color show Port=<port>\n\t"
    "  color map Port=<port> PktPrio=<prio> CFI=<cfi>\n\t"
    "        IntPrio=<prio> Color=<Green|Yellow|Red|Preserve>\n\t"
    "  color unmap Port=<port> IntPrio=<prio> Color=<Green|Yellow|Red|Preserve>\n\t"
    "        PktPrio=<prio> CFI=<cfi>\n"
    "  Note: Color=Preserve is for TB only\n";


cmd_result_t
cmd_robo_color(int unit, args_t *a)
{
    int  port = 0, prio = -1, cfi = -1, color_parse = bcmColorRed;
    bcm_color_t  color;
    char  *subcmd;
    int  r;
    parse_table_t  pt;
    cmd_result_t  retCode;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
	return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    if (sal_strcasecmp(subcmd, "set") == 0) {

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Port", PQ_DFL|PQ_PORT, 0, &port, NULL);
        parse_table_add(&pt, "PRio", PQ_INT|PQ_DFL, 0, &prio, NULL);
        parse_table_add(&pt, "CFI", PQ_INT|PQ_DFL, 0, &cfi, NULL);
        parse_table_add(&pt, "Color", PQ_MULTI|PQ_DFL, 0,
                        &color_parse, diag_robo_parse_color);

        if (!parseEndOk( a, &pt, &retCode)) {
            return retCode;
        }
        if (!SOC_PORT_VALID(unit,port)) {
            cli_out("%s: ERROR: Invalid port selection %d\n",
                    ARG_CMD(a), port);
            return CMD_FAIL;
        }

        color = (bcm_color_t) color_parse;

        if (prio < 0) {
            if (cfi < 0) {
                /* No selection to assign color */
                cli_out("%s: ERROR: No parameter to assign color\n",
                        ARG_CMD(a));
                return CMD_FAIL;
            } else {
                if ((r = bcm_port_cfi_color_set(unit, port,
                                                cfi, color)) < 0) {
                    goto bcm_err;
                }
            }
        } else {
            if (cfi < 0) {
                if (prio > BCM_PRIO_MAX) {
                    cli_out("%s: ERROR: Priority %d exceeds maximum\n",
                            ARG_CMD(a), prio);
                    return CMD_FAIL;
                } else {
                    if ((r = bcm_port_priority_color_set(unit, port, prio,
                                                         color)) < 0) {
                        goto bcm_err;
                    }
                }
            } else {
                cli_out(\
                        "%s: ERROR: Either configure cfi or prio value.\
                        use the 'color map ' cmd to configure both\n",
                        ARG_CMD(a));
                return CMD_FAIL;
            }

        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "show") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Port", PQ_DFL|PQ_PORT, 0, &port, NULL);
        if (!parseEndOk( a, &pt, &retCode)) {
            return retCode;
        }
        if (!SOC_PORT_VALID(unit,port)) {
            cli_out("%s: ERROR: Invalid port selection %d\n",
                    ARG_CMD(a), port);
            return CMD_FAIL;
        }

        cli_out("Color settings for port %s\n", BCM_PORT_NAME(unit, port));
        for (prio = BCM_PRIO_MIN; prio <= BCM_PRIO_MAX; prio++) {
            if ((r = bcm_port_priority_color_get(unit, port, prio,
                                                 &color)) < 0) {
                goto bcm_err;
            }
            cli_out("Priority %d\t%s\n", prio, diag_robo_parse_color[color]);
        }

        if ((r = bcm_port_cfi_color_get(unit, port, FALSE, &color)) < 0) {
            goto bcm_err;
        }
        cli_out("No CFI     \t%s\n", diag_robo_parse_color[color]);

        if ((r = bcm_port_cfi_color_get(unit, port, TRUE, &color)) < 0) {
            goto bcm_err;
        }
        cli_out("CFI        \t%s\n", diag_robo_parse_color[color]);
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "map") == 0) {
        int pkt_prio, int_prio;

        pkt_prio = int_prio = -1;

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Port", PQ_DFL|PQ_PORT, 0, &port, NULL);
        parse_table_add(&pt, "PktPrio", PQ_INT|PQ_DFL, 0, &pkt_prio, NULL);
        parse_table_add(&pt, "CFI", PQ_INT|PQ_DFL, 0, &cfi, NULL);
        parse_table_add(&pt, "IntPrio", PQ_INT|PQ_DFL, 0, &int_prio, NULL);
        parse_table_add(&pt, "Color", PQ_MULTI|PQ_DFL, 0,
                        &color_parse, diag_robo_parse_color);

        if (!parseEndOk( a, &pt, &retCode)) {
            return retCode;
        }

        if (!SOC_PORT_VALID(unit,port)) {
            cli_out("%s: ERROR: Invalid port selection %d\n",
                    ARG_CMD(a), port);
            return CMD_FAIL;
        }

        if (pkt_prio < 0 || cfi < 0 || int_prio < 0) {
            cli_out("Color map settings for port %s\n", 
                    BCM_PORT_NAME(unit, port));

            for (prio = BCM_PRIO_MIN; prio <= BCM_PRIO_MAX; prio++) {
                for (cfi = 0; cfi <= 1; cfi++) {
                    if ((r = bcm_port_vlan_priority_map_get(unit, port, 
                              prio, cfi, &int_prio, &color)) < 0) {
                        goto bcm_err; 
                    }

                    cli_out("Packet Prio=%d, CFI=%d, Internal Prio=%d, "
                            "Color=%s\n",
                            prio, cfi, int_prio, diag_robo_parse_color[color]);
                } 
            }
         } else {
             color = (bcm_color_t) color_parse;
             if ((r = bcm_port_vlan_priority_map_set(unit, port, pkt_prio, cfi, 
                       int_prio, color)) < 0) {
                 goto bcm_err;
             }
         }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "unmap") == 0) {
        int pkt_prio, int_prio;

        pkt_prio = int_prio = -1;

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Port", PQ_DFL | PQ_PORT, 0, &port, NULL);
        parse_table_add(&pt, "PktPrio", PQ_INT|PQ_DFL, 0, &pkt_prio, NULL);
        parse_table_add(&pt, "CFI", PQ_INT|PQ_DFL, 0, &cfi, NULL);
        parse_table_add(&pt, "IntPrio", PQ_INT|PQ_DFL, 0, &int_prio, NULL);
        parse_table_add(&pt, "Color", PQ_MULTI|PQ_DFL, 0,
                        &color_parse, diag_robo_parse_color);

        if (!parseEndOk( a, &pt, &retCode)) {
            return retCode;
        }

        if (!SOC_PORT_VALID(unit,port)) {
            cli_out("%s: ERROR: Invalid port selection %d\n",
                    ARG_CMD(a), port);
            return CMD_FAIL;
        }

        if (pkt_prio < 0 || cfi < 0 || int_prio < 0) {
            cli_out("Color unmap settings for port %s\n", 
                    BCM_PORT_NAME(unit, port));

            for (prio = BCM_PRIO_MIN; prio <= _BCM_PRIO_MAX(unit); prio++) {
                pkt_prio = 0;
                for (color = bcmColorGreen; 
                     color <= bcmColorPreserve; 
                     color++) {
                    if (!SOC_IS_TBX(unit)){
                        if (color == bcmColorPreserve){
                            continue;
                        }
                    } 

                    if (color == bcmColorBlack) {
                        continue;
                    }
                    if ((r = bcm_port_vlan_priority_unmap_get(unit, port, 
                            prio, color, &pkt_prio, &cfi)) < 0) {
                        goto bcm_err; 
                    }
                    cli_out("Internal Prio=%d, Color=%s, Packet Prio=%d, "
                            "CFI=%d\n",
                            prio, diag_robo_parse_color[color], pkt_prio, cfi);
                } 
            }
         } else {
             color = (bcm_color_t) color_parse;
             if ((r = bcm_port_vlan_priority_unmap_set(unit, port, int_prio, 
                                             color, pkt_prio, cfi)) < 0) {
                 goto bcm_err;
             }
         }

        return CMD_OK;
 
    }

    cli_out("%s: ERROR: Unknown color subcommand: %s\n",
            ARG_CMD(a), subcmd);

    return CMD_USAGE;

 bcm_err:

    cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(r));

    return CMD_FAIL;
}

/*
 * Function:    if_ipg
 * Purpose: Configure default IPG values.
 * Parameters:  unit - SOC unit #
 *      a - arguments
 * Returns: CMD_OK/CMD_FAIL
 */
cmd_result_t
if_robo_ipg(int unit, args_t *a)
{
    parse_table_t      pt;
    pbmp_t             arg_pbmp;
    int                arg_speed, speed;
    bcm_port_duplex_t  arg_duplex, duplex;
    cmd_result_t       ret_code;
    int                real_ifg;
    int                rv;
    int                i;
    bcm_port_t         port;

    static const char *header = "        "
                                "    10HD"
                                "    10FD"
                                "   100HD"
                                "   100FD"
                                "  1000HD"
                                "  1000FD";

    static const int speeds[] = {10, 100, 1000};
    static const int num_speeds = sizeof(speeds) / sizeof(int);

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    /*
     * Assign the defaults
     */
    BCM_PBMP_ASSIGN(arg_pbmp, PBMP_PORT_ALL(unit));
    arg_speed  = 0;
    arg_duplex = BCM_PORT_DUPLEX_COUNT; 

    /*
     * Parse the arguments
     */
    if (ARG_CNT(a)) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PortBitMap", PQ_DFL | PQ_PBMP,
                        0, &arg_pbmp, NULL);
        parse_table_add(&pt, "SPeed", PQ_DFL | PQ_INT,
                        0, &arg_speed, NULL);
        parse_table_add(&pt, "FullDuplex", PQ_DFL | PQ_BOOL,
                        0, &arg_duplex, NULL);
        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }
    }

    cli_out("%s\n", header);
    /*
     * Display IPG settings for all the specified ports
     */
    PBMP_ITER(arg_pbmp, port) {
        cli_out("%-8.8s", SOC_PORT_NAME(unit, port));
        for (i = 0; i < num_speeds; i++) {
            speed = speeds[i];
            
            for (duplex = BCM_PORT_DUPLEX_HALF; 
                 duplex < BCM_PORT_DUPLEX_COUNT;
                 duplex++) {
                /*
                 * Skip an entry if the speed has been explicitly specified
                 */
                if (arg_speed != 0 && speed != arg_speed) {
                    cli_out("%8.8s", " ");
                    continue;
                }
            
                /*
                 * Skip an entry if duplex has been explicitly specified
                 * and the entry doesn't match
                 */
                if (arg_duplex != BCM_PORT_DUPLEX_COUNT &&
                    arg_duplex != duplex) {
                    cli_out("%8.8s", " ");
                    continue;
                }
                
                rv = bcm_port_ifg_get(unit, port, speed, duplex, &real_ifg);
                
                if (rv == BCM_E_NONE) {
                    cli_out("%8d", real_ifg);
                } else {
                    cli_out("%8.8s", "n/a");
                }
            }
        }
        cli_out("\n");
    }

    return(CMD_OK);
}

cmd_result_t
if_robo_linkscan(int unit, args_t *a)
{
    parse_table_t pt;
    char *c;
    int us, rv;
    pbmp_t pbm_sw, pbm_hw, pbm_force;
    pbmp_t pbm_none, pbm_none_fe, pbm_none_ge;
    pbmp_t pbm_temp;
    soc_port_t port;
    char pfmt[SOC_PBMP_FMT_LEN];
    int linkscan_interval_default;

    linkscan_interval_default = soc_property_get(unit,
        spn_BCM_LINKSCAN_INTERVAL, BCM_LINKSCAN_INTERVAL_DEFAULT);

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

    if ((rv = bcm_linkscan_enable_get(unit, &us)) < 0) {
        cli_out("%s: Error: Failed to recover enable status: %s\n",
                ARG_CMD(a), bcm_errmsg(rv));
        return(CMD_FAIL);
    }

    BCM_PBMP_CLEAR(pbm_sw);
    BCM_PBMP_CLEAR(pbm_hw);
    BCM_PBMP_CLEAR(pbm_force);

    PBMP_PORT_ITER(unit, port) {
        int mode;
    
        if ((rv = bcm_linkscan_mode_get(unit, port, &mode)) < 0) {
            cli_out("%s: Error: Could not get linkscan state for port %s\n",
                    ARG_CMD(a), SOC_PORT_NAME(unit, port));
        } else {
            switch (mode) {
                case BCM_LINKSCAN_MODE_SW:
                    SOC_PBMP_PORT_ADD(pbm_sw, port);
                    break;
                case BCM_LINKSCAN_MODE_HW:
                    SOC_PBMP_PORT_ADD(pbm_hw, port);
                    break;
                default:
                    break;
            }
        }
    }

    /*
     * If there are no arguments, just display the status.
     */

    if (ARG_CNT(a) == 0) {
        char buf[80];
        pbmp_t pbm;
    
        if (us) {
            cli_out("%s: Linkscan enabled\n", ARG_CMD(a));
            cli_out("%s:   Software polling interval: %d usec\n",
                    ARG_CMD(a), us);
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
        } else {
            cli_out("%s: Linkscan disabled\n", ARG_CMD(a));
        }
    
        return(CMD_OK);
    }

    us = linkscan_interval_default;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "SwPortBitMap", PQ_PBMP | PQ_DFL, 0, &pbm_sw, 0);
    parse_table_add(&pt, "HwPortBitMap", PQ_PBMP | PQ_DFL, 0, &pbm_hw, 0);
    parse_table_add(&pt, "Force", PQ_PBMP | PQ_DFL, 0, &pbm_force, 0);
    parse_table_add(&pt, "Interval", PQ_INT | PQ_DFL, 0, &us, 0);

    if (parse_arg_eq(a, &pt) < 0) {
        cli_out("%s: Invalid argument: %s\n", ARG_CMD(a), ARG_CUR(a));
        parse_arg_eq_done(&pt);
        return(CMD_FAIL);
    }
    parse_arg_eq_done(&pt);

    /*
     * Handle backward compatibility, allowing a raw interval to be
     * specified directly on the command line, as well as "on" or "off".
     */

    if (ARG_CUR(a) != NULL) {
        c = ARG_GET(a);
    
        if (!sal_strcasecmp(c, "off") ||
            !sal_strcasecmp(c, "disable") ||
            !sal_strcasecmp(c, "no")) {
            us = 0;
        } else if (!sal_strcasecmp(c, "on") ||
               !sal_strcasecmp(c, "enable") ||
               !sal_strcasecmp(c, "yes")) {
            us = prev_robo_linkscan_interval[unit] ?
                 prev_robo_linkscan_interval[unit] : linkscan_interval_default;
        } else if (isint(c)) {
            us = parse_integer(c);
        } else {
            return(CMD_USAGE);
        }
    }

    if (us == 0) {
        int prev_interval;

        rv = bcm_linkscan_enable_get(unit, &prev_interval);
        if (rv < 0 || prev_interval <= 0) {
            prev_interval = linkscan_interval_default;
        }
        prev_robo_linkscan_interval[unit] = prev_interval;
        /* Turn off linkscan */
        if ((rv = bcm_linkscan_enable_set(unit, 0)) < 0) {
            cli_out("%s: Error: Failed to disable linkscan: %s\n",
                    ARG_CMD(a), bcm_errmsg(rv));
            return(CMD_FAIL);
        }
    
        return(CMD_OK);
    }

    SOC_PBMP_AND(pbm_sw, PBMP_PORT_ALL(unit));
    SOC_PBMP_AND(pbm_hw, PBMP_PORT_ALL(unit));
    SOC_PBMP_ASSIGN(pbm_none_fe, PBMP_FE_ALL(unit));
    SOC_PBMP_ASSIGN(pbm_none_ge, PBMP_GE_ALL(unit));
    SOC_PBMP_OR(pbm_none_fe, pbm_none_ge);
    pbm_none = pbm_none_fe;
    SOC_PBMP_ASSIGN(pbm_temp, pbm_sw);
    SOC_PBMP_OR(pbm_temp, pbm_hw);
    SOC_PBMP_XOR(pbm_none, pbm_temp);
    SOC_PBMP_AND(pbm_force, PBMP_PORT_ALL(unit));

    SOC_PBMP_ASSIGN(pbm_temp, pbm_sw);
    SOC_PBMP_AND(pbm_temp, pbm_hw);
    if (SOC_PBMP_NOT_NULL(pbm_temp)) {
        cli_out("%s: Error: Same port can't use both "
                "software and hardware linkscan\n",
                ARG_CMD(a));
        return(CMD_FAIL);
    }

    if ((rv = bcm_linkscan_mode_set_pbm(unit, pbm_sw,
                    BCM_LINKSCAN_MODE_SW)) < 0) {
        cli_out("%s: Failed to set software link scanning: PBM=%s: %s\n",
                ARG_CMD(a), SOC_PBMP_FMT(pbm_sw, pfmt), bcm_errmsg(rv));
        return(CMD_FAIL);
    }

    if ((rv = bcm_linkscan_mode_set_pbm(unit, pbm_hw,
                    BCM_LINKSCAN_MODE_HW)) < 0) {
        cli_out("%s: Failed to set hardware link scanning: PBM=%s: %s\n",
                ARG_CMD(a), SOC_PBMP_FMT(pbm_hw, pfmt), bcm_errmsg(rv));
        return(CMD_FAIL);
    }

    if ((rv = bcm_linkscan_mode_set_pbm(unit, pbm_none,
                    BCM_LINKSCAN_MODE_NONE)) < 0) {
        cli_out("%s: Failed to disable link scanning: PBM=%s: %s\n",
                ARG_CMD(a), SOC_PBMP_FMT(pbm_none, pfmt), bcm_errmsg(rv));
        return(CMD_FAIL);
    }

    if ((rv = bcm_linkscan_enable_set(unit, us)) < 0) {
        cli_out("%s: Error: Failed to enable linkscan: %s\n",
                ARG_CMD(a), bcm_errmsg(rv));
        return(CMD_FAIL);
    }

    if ((rv = bcm_link_change(unit, pbm_force)) < 0) {
        cli_out("%s: Failed to force link scan: PBM=%s: %s\n",
                ARG_CMD(a), SOC_PBMP_FMT(pbm_force, pfmt), bcm_errmsg(rv));
        return(CMD_FAIL);
    }

    return(CMD_OK);
}


#define DUMP_PHY_COLS   4

void _robo_print_timesync_egress_message_mode(char *message, bcm_port_phy_timesync_event_message_egress_mode_t mode) 
{

    cli_out("%s (no,uc,rc) - ", message);

    switch (mode) {
    case bcmPortPhyTimesyncEventMessageEgressModeNone:
        cli_out("NOne\n");
        break;
    case bcmPortPhyTimesyncEventMessageEgressModeUpdateCorrectionField:
        cli_out("Update_Correctionfield\n");
        break;
    case bcmPortPhyTimesyncEventMessageEgressModeReplaceCorrectionFieldOrigin:
        cli_out("Replace_Correctionfield_origin\n");
        break;
    default:
        cli_out("\n");
        break;
    }

}

bcm_port_phy_timesync_event_message_egress_mode_t _robo_convert_timesync_egress_message_str(char *str, 
    bcm_port_phy_timesync_event_message_egress_mode_t def) 
{
    int i;
    struct s_array {
        char *s;
        bcm_port_phy_timesync_event_message_egress_mode_t value;
    } data[] = {
    {"no",bcmPortPhyTimesyncEventMessageEgressModeNone},
    {"uc",bcmPortPhyTimesyncEventMessageEgressModeUpdateCorrectionField},
    {"rc",bcmPortPhyTimesyncEventMessageEgressModeReplaceCorrectionFieldOrigin} };

    for (i = 0; i < (sizeof(data)/sizeof(data[0])); i++) {
        if (!strncmp(str, data[i].s, 2)) {
            return data[i].value;
        }
    }
    return def;
}

void _robo_print_timesync_ingress_message_mode(char *message, bcm_port_phy_timesync_event_message_ingress_mode_t mode)
{

    cli_out("%s (no,uc,it,id) - ", message);

    switch (mode) {
    case bcmPortPhyTimesyncEventMessageIngressModeNone:
        cli_out("NOne\n");
        break;
    case bcmPortPhyTimesyncEventMessageIngressModeUpdateCorrectionField:
        cli_out("Update_Correctionfield\n");
        break;
    case bcmPortPhyTimesyncEventMessageIngressModeInsertTimestamp:
        cli_out("Insert_Timestamp\n");
        break;
    case bcmPortPhyTimesyncEventMessageIngressModeInsertDelaytime:
        cli_out("Insert_Delaytime\n");
        break;
    default:
        cli_out("\n");
        break;
    }

}

bcm_port_phy_timesync_event_message_ingress_mode_t _robo_convert_timesync_ingress_message_str(char *str, 
    bcm_port_phy_timesync_event_message_ingress_mode_t def) 
{
    int i;
    struct s_array {
        char *s;
        bcm_port_phy_timesync_event_message_ingress_mode_t value;
    } data[] = {
    {"no",bcmPortPhyTimesyncEventMessageIngressModeNone},
    {"uc",bcmPortPhyTimesyncEventMessageIngressModeUpdateCorrectionField},
    {"it",bcmPortPhyTimesyncEventMessageIngressModeInsertTimestamp},
    {"id",bcmPortPhyTimesyncEventMessageIngressModeInsertDelaytime} };

    for (i = 0; i < (sizeof(data)/sizeof(data[0])); i++) {
        if (!strncmp(str, data[i].s, 2)) {
            return data[i].value;
        }
    }
    return def;
}

void _robo_print_timesync_gmode(char *message, bcm_port_phy_timesync_global_mode_t mode)
{

    cli_out("%s (fr,si,cp) - ", message);

    switch (mode) {
    case bcmPortPhyTimesyncModeFree:
        cli_out("FRee\n");
        break;
    case bcmPortPhyTimesyncModeSyncin:
        cli_out("SyncIn\n");
        break;
    case bcmPortPhyTimesyncModeCpu:
        cli_out("CPu\n");
        break;
    default:
        cli_out("\n");
        break;
    }

}

bcm_port_phy_timesync_global_mode_t _robo_convert_timesync_gmode_str(char *str, 
    bcm_port_phy_timesync_global_mode_t def) 
{
    int i;
    struct s_array {
        char *s;
        bcm_port_phy_timesync_global_mode_t value;
    } data[] = {
    {"fr",bcmPortPhyTimesyncModeFree},
    {"si",bcmPortPhyTimesyncModeSyncin},
    {"cp",bcmPortPhyTimesyncModeCpu} };

    for (i = 0; i < (sizeof(data)/sizeof(data[0])); i++) {
        if (!strncmp(str, data[i].s, 2)) {
            return data[i].value;
        }
    }
    return def;
}

void _robo_print_framesync_mode(char *message, bcm_port_phy_timesync_framesync_mode_t  mode)
{

    cli_out("%s (fno,fs0,fs1,fss,fsc) - ", message);

    switch (mode) {
    case bcmPortPhyTimesyncFramesyncNone:
        cli_out("FramesyncNOne\n");
        break;
    case bcmPortPhyTimesyncFramesyncSyncin0:
        cli_out("FramesyncSyncIn0\n");
        break;
    case bcmPortPhyTimesyncFramesyncSyncin1:
        cli_out("FramesyncSyncIn1\n");
        break;
    case bcmPortPhyTimesyncFramesyncSyncout:
        cli_out("FrameSyncSyncout\n");
        break;
    case bcmPortPhyTimesyncFramesyncCpu:
        cli_out("FrameSyncCpu\n");
        break;
    default:
        cli_out("\n");
        break;
    }

}

bcm_port_phy_timesync_framesync_mode_t _robo_convert_framesync_mode_str(char *str, 
    bcm_port_phy_timesync_framesync_mode_t def) 
{
    int i;
    struct s_array {
        char *s;
        bcm_port_phy_timesync_framesync_mode_t value;
    } data[] = {
    {"fno",bcmPortPhyTimesyncFramesyncNone},
    {"fs0",bcmPortPhyTimesyncFramesyncSyncin0},
    {"fs1",bcmPortPhyTimesyncFramesyncSyncin1},
    {"fss",bcmPortPhyTimesyncFramesyncSyncout},
    {"fsc",bcmPortPhyTimesyncFramesyncCpu} };

    for (i = 0; i < (sizeof(data)/sizeof(data[0])); i++) {
        if (!strncmp(str, data[i].s, 3)) {
            return data[i].value;
        }
    }
    return def;
}

void _robo_print_syncout_mode(char *message, bcm_port_phy_timesync_syncout_mode_t mode)
{

    cli_out("%s (sod,sot,spt,sps) - ", message);

    switch (mode) {
    case bcmPortPhyTimesyncSyncoutDisable:
        cli_out("SyncOutDisable\n");
        break;
    case bcmPortPhyTimesyncSyncoutOneTime:
        cli_out("SyncoutOneTime\n");
        break;
    case bcmPortPhyTimesyncSyncoutPulseTrain:
        cli_out("SyncoutPulseTrain\n");
        break;
    case bcmPortPhyTimesyncSyncoutPulseTrainWithSync:
        cli_out("SyncoutPulsetrainSync\n");
        break;
    default:
        cli_out("\n");
        break;
    }

}

bcm_port_phy_timesync_syncout_mode_t _robo_convert_syncout_mode_str(char *str, 
    bcm_port_phy_timesync_syncout_mode_t def) 
{
    int i;
    struct s_array {
        char *s;
        bcm_port_phy_timesync_syncout_mode_t value;
    } data[] = {
    {"sod",bcmPortPhyTimesyncSyncoutDisable},
    {"sot",bcmPortPhyTimesyncSyncoutOneTime},
    {"spt",bcmPortPhyTimesyncSyncoutPulseTrain},
    {"sps",bcmPortPhyTimesyncSyncoutPulseTrainWithSync} };

    for (i = 0; i < (sizeof(data)/sizeof(data[0])); i++) {
        if (!strncmp(str, data[i].s, 3)) {
            return data[i].value;
        }
    }
    return def;
}

void _robo_print_timesync_config(bcm_port_phy_timesync_config_t *conf)
{

    cli_out("ENable (Y or N) - %s\n", conf->flags & BCM_PORT_PHY_TIMESYNC_ENABLE ? "Yes" : "No");

    cli_out("CaptureTS (Y or N) - %s\n", conf->flags & BCM_PORT_PHY_TIMESYNC_CAPTURE_TS_ENABLE ? "Yes" : "No");

    cli_out("HeartbeatTS (Y or N) - %s\n", conf->flags & BCM_PORT_PHY_TIMESYNC_HEARTBEAT_TS_ENABLE ? "Yes" : "No");

    cli_out("RxCrc (Y or N) - %s\n", conf->flags & BCM_PORT_PHY_TIMESYNC_RX_CRC_ENABLE ? "Yes" : "No");

    cli_out("AS (Y or N) - %s\n", conf->flags & BCM_PORT_PHY_TIMESYNC_8021AS_ENABLE ? "Yes" : "No");

    cli_out("L2 (Y or N) - %s\n", conf->flags & BCM_PORT_PHY_TIMESYNC_L2_ENABLE ? "Yes" : "No");

    cli_out("IP4 (Y or N) - %s\n", conf->flags & BCM_PORT_PHY_TIMESYNC_IP4_ENABLE ? "Yes" : "No");

    cli_out("IP6 (Y or N) - %s\n", conf->flags & BCM_PORT_PHY_TIMESYNC_IP6_ENABLE ? "Yes" : "No");

    cli_out("ExtClock (Y or N) - %s\n", conf->flags & BCM_PORT_PHY_TIMESYNC_CLOCK_SRC_EXT ? "Yes" : "No");

    cli_out("ITpid = 0x%04x\n", conf->itpid);

    cli_out("OTpid = 0x%04x\n", conf->otpid);

    cli_out("OriginalTimecodeSeconds = 0x%08x%08x\n",
            COMPILER_64_HI(conf->original_timecode.seconds),
            COMPILER_64_LO(conf->original_timecode.seconds));

    cli_out("OriginalTimecodeNanoseconds = %u\n", conf->original_timecode.nanoseconds);

    _robo_print_timesync_gmode("GMode", conf->gmode);

    _robo_print_framesync_mode("FramesyncMode", conf->framesync.mode);

    _robo_print_syncout_mode("SyncoutMode", conf->syncout.mode);

    cli_out("TxOffset = %d\n", conf->tx_timestamp_offset);

    cli_out("RxOffset = %d\n", conf->rx_timestamp_offset);

    _robo_print_timesync_egress_message_mode("TxSync", conf->tx_sync_mode);
    _robo_print_timesync_egress_message_mode("TxDelayReq", conf->tx_delay_request_mode);
    _robo_print_timesync_egress_message_mode("TxPdelayReq", conf->tx_pdelay_request_mode);
    _robo_print_timesync_egress_message_mode("TxPdelayreS", conf->tx_pdelay_response_mode);
                                            
    _robo_print_timesync_ingress_message_mode("RxSync", conf->rx_sync_mode);
    _robo_print_timesync_ingress_message_mode("RxDelayReq", conf->rx_delay_request_mode);
    _robo_print_timesync_ingress_message_mode("RxPdelayReq", conf->rx_pdelay_request_mode);
    _robo_print_timesync_ingress_message_mode("RxPdelayreS", conf->rx_pdelay_response_mode);
                                            
}

void _robo_print_heartbeat_ts(int unit, bcm_port_t port)
{
    int rv;
    uint64 time;

    rv = bcm_port_control_phy_timesync_get(unit, port, bcmPortControlPhyTimesyncHeartbeatTimestamp, &time);

    if (rv != BCM_E_NONE) {
       cli_out("bcm_port_control_phy_timesync_get failed with error  u=%d p=%d %s\n", unit, port, bcm_errmsg(rv));
    }

    cli_out("Heartbeat TS = %08x%08x\n",
            COMPILER_64_HI(time), COMPILER_64_LO(time));
}

void _robo_print_capture_ts(int unit, bcm_port_t port)
{
    int rv;
    uint64 time;

    rv = bcm_port_control_phy_timesync_get(unit, port, bcmPortControlPhyTimesyncCaptureTimestamp, &time);

    if (rv != BCM_E_NONE) {
       cli_out("bcm_port_control_phy_timesync_get failed with error  u=%d p=%d %s\n", unit, port, bcm_errmsg(rv));
    }

    cli_out("Capture   TS = %08x%08x\n",
            COMPILER_64_HI(time), COMPILER_64_LO(time));
}

/*
 * Function:    if_phy
 * Purpose: Show/configure phy registers.
 * Parameters:  u - SOC unit #
 *              a - pointer to args
 * Returns: CMD_OK/CMD_FAIL/
 */
cmd_result_t
if_robo_phy(int u, args_t *a)
{
    pbmp_t pbm, pbm_phys, pbm_temp;
    soc_port_t p, dport = 0;
    char *c, drv_name[64];
    uint16 phy_data, phy_reg, phy_devad = 0;
    uint16 phy_addr;
    int intermediate = 0;
    int rv = BCM_E_NONE;
    char pfmt[SOC_PBMP_FMT_LEN];
    int i;
    int p_devad[] = {PHY_C45_DEV_PMA_PMD,
                        PHY_C45_DEV_PCS,
                        PHY_C45_DEV_PHYXS};
    char *p_devad_str[] = {"DEV_PMA_PMD",
                         "DEV_PCS",
                         "DEV_PHYXS"};

    if (! sh_check_attached(ARG_CMD(a), u)) {
        return CMD_FAIL;
    }

    c = ARG_GET(a);

    if (c != NULL && sal_strcasecmp(c, "info") == 0) {
        cli_out("Phy mapping dump:\n");
        cli_out("%10s %5s %5s %5s %5s %16s %10s\n",
                "port", "id0", "id1", "addr", "iaddr", "name", "timeout");
        PBMP_PORT_ITER(u, p) {
            cli_out("%5s(%3d) %5x %5x %5x %5x %16s %10d\n",
                    SOC_PORT_NAME(u, p), p,
                    soc_phy_id0reg_get(u, p),
                    soc_phy_id1reg_get(u, p),
                    soc_phy_addr_of_port(u, p),
                    soc_phy_addr_int_of_port(u, p),
                    soc_phy_name_get(u, p),
                    soc_phy_an_timeout_get(u, p));
        }

        return CMD_OK;
    }

    if (c != NULL && sal_strcasecmp(c, "timesync") == 0) {
        bcm_port_phy_timesync_config_t conf;
        uint64 val64;

        if (((c = ARG_GET(a)) == NULL) || (parse_bcm_pbmp(u, c, &pbm) < 0)) {
            cli_out("%s: ERROR: unrecognized port bitmap: %s\n", ARG_CMD(a), c);
            return CMD_FAIL;
        }
        if ((c = ARG_CUR(a)) != NULL) {
            parse_table_t    pt;
            uint32 enable, capture_ts, heartbeat_ts, rx_crc, as, l2, ip4, ip6, ec, /* bool */
                   itpid, otpid, tx_offset, rx_offset, original_timecode_seconds, original_timecode_nanoseconds,
                   load_all, frame_sync, flags; 
            char   *gmode_str=NULL, *tx_sync_mode_str=NULL, *tx_delay_request_mode_str=NULL, 
                   *tx_pdelay_request_mode_str=NULL, *tx_pdelay_response_mode_str=NULL,
                   *rx_sync_mode_str=NULL, *rx_delay_request_mode_str=NULL,
                   *rx_pdelay_request_mode_str=NULL, *rx_pdelay_response_mode_str=NULL,
                   *framesync_mode_str=NULL, *syncout_mode_str=NULL; 

            parse_table_init(u, &pt);
            parse_table_add(&pt, "ENable", PQ_BOOL | PQ_DFL | PQ_NO_EQ_OPT, /* index 0 */
                            0, &enable, 0);
            parse_table_add(&pt, "CaptureTS", PQ_BOOL | PQ_DFL | PQ_NO_EQ_OPT, /* index 1 */
                            0, &capture_ts, 0);
            parse_table_add(&pt, "HeartbeatTS", PQ_BOOL | PQ_DFL | PQ_NO_EQ_OPT, /* index 2 */
                            0, &heartbeat_ts, 0);
            parse_table_add(&pt, "RxCrc", PQ_BOOL | PQ_DFL | PQ_NO_EQ_OPT, /* index 3 */
                            0, &rx_crc, 0);
            parse_table_add(&pt, "AS", PQ_BOOL | PQ_DFL | PQ_NO_EQ_OPT, /* index 4 */
                            0, &as, 0);
            parse_table_add(&pt, "L2", PQ_BOOL | PQ_DFL | PQ_NO_EQ_OPT, /* index 5 */
                            0, &l2, 0);
            parse_table_add(&pt, "IP4", PQ_BOOL | PQ_DFL | PQ_NO_EQ_OPT, /* index 6 */
                            0, &ip4, 0);
            parse_table_add(&pt, "IP6", PQ_BOOL | PQ_DFL | PQ_NO_EQ_OPT, /* index 7 */
                            0, &ip6, 0);
            parse_table_add(&pt, "ExtClock", PQ_BOOL | PQ_DFL | PQ_NO_EQ_OPT, /* index 8 */
                            0, &ec, 0);
            parse_table_add(&pt, "ITpid", PQ_DFL | PQ_INT, /* index 9 */
                            0, &itpid, 0);
            parse_table_add(&pt, "OTpid", PQ_DFL | PQ_INT, /* index 10 */
                            0, &otpid, 0);
            parse_table_add(&pt, "GMode", PQ_DFL | PQ_STRING, /* index 11 */
                            0, &gmode_str, 0);
            parse_table_add(&pt, "TxOffset", PQ_DFL | PQ_INT, /* index 12 */
                            0, &tx_offset, 0);
            parse_table_add(&pt, "RxOffset", PQ_DFL | PQ_INT, /* index 13 */
                            0, &rx_offset, 0);
            parse_table_add(&pt, "TxSync", PQ_DFL | PQ_STRING, /* index 14 */
                            0, &tx_sync_mode_str, 0);
            parse_table_add(&pt, "TxDelayReq", PQ_DFL | PQ_STRING, /* index 15 */
                            0, &tx_delay_request_mode_str, 0);
            parse_table_add(&pt, "TxPdelayReq", PQ_DFL | PQ_STRING, /* index 16 */
                            0, &tx_pdelay_request_mode_str, 0);
            parse_table_add(&pt, "TxPdelayreS", PQ_DFL | PQ_STRING, /* index 17 */
                            0, &tx_pdelay_response_mode_str, 0);
            parse_table_add(&pt, "RxSync", PQ_DFL | PQ_STRING, /* index 18 */
                            0, &rx_sync_mode_str, 0);
            parse_table_add(&pt, "RxDelayReq", PQ_DFL | PQ_STRING, /* index 19 */
                            0, &rx_delay_request_mode_str, 0);
            parse_table_add(&pt, "RxPdelayReq", PQ_DFL | PQ_STRING, /* index 20 */
                            0, &rx_pdelay_request_mode_str, 0);
            parse_table_add(&pt, "RxPdelayreS", PQ_DFL | PQ_STRING, /* index 21 */
                            0, &rx_pdelay_response_mode_str, 0);
            parse_table_add(&pt, "OriginalTimecodeSeconds", PQ_DFL | PQ_INT, /* index 22 */
                            0, &original_timecode_seconds, 0);
            parse_table_add(&pt, "OriginalTimecodeNanoseconds", PQ_DFL | PQ_INT, /* index 23 */
                            0, &original_timecode_nanoseconds, 0);

            parse_table_add(&pt, "FramesyncMode", PQ_DFL | PQ_STRING, /* index 24 */
                            0, &framesync_mode_str, 0);
            parse_table_add(&pt, "SyncoutMode", PQ_DFL | PQ_STRING, /* index 25 */
                            0, &syncout_mode_str, 0);
            parse_table_add(&pt, "LoadAll", PQ_BOOL | PQ_DFL | PQ_NO_EQ_OPT, /* index 26 */
                            0, &load_all, 0);
            parse_table_add(&pt, "FrameSync", PQ_BOOL | PQ_DFL | PQ_NO_EQ_OPT, /* index 27 */
                            0, &frame_sync, 0);

            if (parse_arg_eq(a, &pt) < 0) {
                parse_arg_eq_done(&pt);
                return CMD_USAGE;
            }
            if (ARG_CNT(a) > 0) {
                cli_out("%s: Unknown argument %s\n", ARG_CMD(a), ARG_CUR(a));
                parse_arg_eq_done(&pt);
                return CMD_USAGE;
            }

            flags=0;

            for (i = 0; i < pt.pt_cnt; i++) {
                if (pt.pt_entries[i].pq_type & PQ_PARSED) {
                    flags |= (1 << i);
                }
            }

            DPORT_SOC_PBMP_ITER(u, pbm, dport, p) {
                conf.validity_mask = 0xffffffff & ~BCM_PORT_PHY_TIMESYNC_VALID_MPLS_CONTROL;
                if ((rv = bcm_port_phy_timesync_config_get(u,p,&conf)) == BCM_E_FAIL) {
                    cli_out("bcm_port_phy_timesync_config_get() failed, u=%d, p=%d\n", u, p);
                    return CMD_FAIL;
                }

                if (flags & (1U << 0)) {
                    conf.flags &= ~BCM_PORT_PHY_TIMESYNC_ENABLE;
                    conf.flags |= enable ? BCM_PORT_PHY_TIMESYNC_ENABLE : 0;
                }

                if (flags & (1U << 1)) {
                    conf.flags &= ~BCM_PORT_PHY_TIMESYNC_CAPTURE_TS_ENABLE;
                    conf.flags |= capture_ts ? BCM_PORT_PHY_TIMESYNC_CAPTURE_TS_ENABLE : 0;
                }

                if (flags & (1U << 2)) {
                    conf.flags &= ~BCM_PORT_PHY_TIMESYNC_HEARTBEAT_TS_ENABLE;
                    conf.flags |= heartbeat_ts ? BCM_PORT_PHY_TIMESYNC_HEARTBEAT_TS_ENABLE : 0;
                }

                if (flags & (1U << 3)) {
                    conf.flags &= ~BCM_PORT_PHY_TIMESYNC_RX_CRC_ENABLE;
                    conf.flags |= rx_crc ? BCM_PORT_PHY_TIMESYNC_RX_CRC_ENABLE : 0;
                }

                if (flags & (1U << 4)) {
                    conf.flags &= ~BCM_PORT_PHY_TIMESYNC_8021AS_ENABLE;
                    conf.flags |= as ? BCM_PORT_PHY_TIMESYNC_8021AS_ENABLE : 0;
                }

                if (flags & (1U << 5)) {
                    conf.flags &= ~BCM_PORT_PHY_TIMESYNC_L2_ENABLE;
                    conf.flags |= l2 ? BCM_PORT_PHY_TIMESYNC_L2_ENABLE : 0;
                }

                if (flags & (1U << 6)) {
                    conf.flags &= ~BCM_PORT_PHY_TIMESYNC_IP4_ENABLE;
                    conf.flags |= ip4 ? BCM_PORT_PHY_TIMESYNC_IP4_ENABLE : 0;
                }

                if (flags & (1U << 7)) {
                    conf.flags &= ~BCM_PORT_PHY_TIMESYNC_IP6_ENABLE;
                    conf.flags |= ip6 ? BCM_PORT_PHY_TIMESYNC_IP6_ENABLE : 0;
                }

                if (flags & (1U << 8)) {
                    conf.flags &= ~BCM_PORT_PHY_TIMESYNC_CLOCK_SRC_EXT;
                    conf.flags |= ec ? BCM_PORT_PHY_TIMESYNC_CLOCK_SRC_EXT : 0;
                }

                if (flags & (1U << 9)) {
                    conf.itpid = itpid;
                }

                if (flags & (1U << 10)) {
                    conf.otpid = otpid;
                }

                if (flags & (1U << 11)) {
                    conf.gmode = _robo_convert_timesync_gmode_str(gmode_str, conf.gmode);
                }

                if (flags & (1U << 12)) {
                    conf.tx_timestamp_offset = tx_offset;
                }

                if (flags & (1U << 13)) {
                    conf.rx_timestamp_offset = rx_offset;
                }

                if (flags & (1U << 14)) {
                    conf.tx_sync_mode = _robo_convert_timesync_egress_message_str(tx_sync_mode_str, conf.tx_sync_mode);
                }

                if (flags & (1U << 15)) {
                    conf.tx_delay_request_mode = _robo_convert_timesync_egress_message_str(tx_delay_request_mode_str,
                                                  conf.tx_delay_request_mode);
                }

                if (flags & (1U << 16)) {
                    conf.tx_pdelay_request_mode = _robo_convert_timesync_egress_message_str(tx_pdelay_request_mode_str,
                                                   conf.tx_pdelay_request_mode);
                }

                if (flags & (1U << 17)) {
                    conf.tx_pdelay_response_mode = _robo_convert_timesync_egress_message_str(tx_pdelay_response_mode_str,
                                                    conf.tx_pdelay_response_mode);
                }

                if (flags & (1U << 18)) {
                    conf.rx_sync_mode = _robo_convert_timesync_ingress_message_str(rx_sync_mode_str, conf.rx_sync_mode);
                }

                if (flags & (1U << 19)) {
                    conf.rx_delay_request_mode = _robo_convert_timesync_ingress_message_str(rx_delay_request_mode_str,
                                                  conf.rx_delay_request_mode);
                }

                if (flags & (1U << 20)) {
                    conf.rx_pdelay_request_mode = _robo_convert_timesync_ingress_message_str(rx_pdelay_request_mode_str,
                                                   conf.rx_pdelay_request_mode);
                }

                if (flags & (1U << 21)) {
                    conf.rx_pdelay_response_mode = _robo_convert_timesync_ingress_message_str(rx_pdelay_response_mode_str,
                                                    conf.rx_pdelay_response_mode);
                }

                if (flags & (1U << 22)) {
                    COMPILER_64_SET(conf.original_timecode.seconds, 0,
                                    original_timecode_seconds);
                }

                if (flags & (1U << 23)) {
                    conf.original_timecode.nanoseconds = original_timecode_nanoseconds;
                }

                if (flags & (1U << 24)) {
                    conf.framesync.mode = _robo_convert_framesync_mode_str(framesync_mode_str, conf.framesync.mode);
                }

                if (flags & (1U << 25)) {
                    conf.syncout.mode = _robo_convert_syncout_mode_str(syncout_mode_str, conf.syncout.mode);
                }

                conf.validity_mask = 0xffffffff & ~BCM_PORT_PHY_TIMESYNC_VALID_MPLS_CONTROL;
                if ((rv = bcm_port_phy_timesync_config_set(u,p,&conf)) == BCM_E_FAIL) {
                    cli_out("bcm_port_phy_timesync_config_set() failed, u=%d, p=%d\n", u, p);
                    return CMD_FAIL;
                }

                if (flags & (1U << 26)) {
                    COMPILER_64_SET(val64, 0, 0xaaaaaaaa);
                    rv = bcm_port_control_phy_timesync_set(u, p, bcmPortControlPhyTimesyncLoadControl, val64);
                    if (rv != BCM_E_NONE) {
                       cli_out("bcm_port_control_phy_timesync_set failed with error  u=%d p=%d %s\n", u, p, bcm_errmsg(rv));
                    }
                }
                if (flags & (1U << 27)) {
                    COMPILER_64_SET(val64, 0, 1);
                    rv = bcm_port_control_phy_timesync_set(u, p, bcmPortControlPhyTimesyncFrameSync, val64);
                    if (rv != BCM_E_NONE) {
                       cli_out("bcm_port_control_phy_timesync_set failed with error  u=%d p=%d %s\n", u, p, bcm_errmsg(rv));
                    }
                }
            }

            /* free allocated memory from arg parsing */
            parse_arg_eq_done(&pt);

        } else {

            DPORT_SOC_PBMP_ITER(u, pbm, dport, p) {
                cli_out("\n\nIEEE1588 settings for %s(%3d)\n",
                        SOC_PORT_NAME(u, p), p);
                
                conf.validity_mask = 0xffffffff & ~BCM_PORT_PHY_TIMESYNC_VALID_MPLS_CONTROL;
                if ((rv = bcm_port_phy_timesync_config_get(u,p,&conf)) == BCM_E_FAIL) {
                    cli_out("bcm_port_phy_timesync_config_get() failed, u=%d, p=%d\n", u, p);
                    return CMD_FAIL;
                }
                _robo_print_timesync_config(&conf);
                _robo_print_heartbeat_ts(u, p);
                _robo_print_capture_ts(u, p);
            }

        }
        return CMD_OK;
    }

    /* Raw access to an MII register */
    if (c != NULL && sal_strcasecmp(c, "raw") == 0) {
        if ((c = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }
        phy_addr = strtoul(c, NULL, 0);
        if ((c = ARG_GET(a)) == NULL) { /* Get register number */
            return CMD_USAGE;
        }
        phy_reg = strtoul(c, NULL, 0);
        if ((c = ARG_GET(a)) == NULL) { /* Read register */
            rv = soc_miim_read(u, phy_addr, phy_reg, &phy_data);
            if (rv < 0) {
                cli_out("ERROR: MII Addr %d: soc_miim_read failed: %s\n",
                        phy_addr, soc_errmsg(rv));
                return CMD_FAIL;
            }
            cli_out("%s\t0x%02x: 0x%04x\n",
                    "", phy_reg, phy_data);
        } else { /* write */
            phy_data = strtoul(c, NULL, 0);
            rv = soc_miim_write(u, phy_addr, phy_reg, phy_data);
            if (rv < 0) {
                cli_out("ERROR: MII Addr %d: soc_miim_write failed: %s\n",
                        phy_addr, soc_errmsg(rv));
                return CMD_FAIL;
            }
        }
        return CMD_OK;
    }

    if (c != NULL && sal_strcasecmp(c, "int") == 0) {
        intermediate = 1;
        c = ARG_GET(a);
    }

    if (c != NULL && sal_strcasecmp(c, "power") == 0){
        parse_table_t    pt;
        char *mode_type = 0;
        uint32 mode_value;
        int sleep_time = -1;
        int wake_time = -1;

        if (((c = ARG_GET(a)) == NULL) || (parse_bcm_pbmp(u, c, &pbm) < 0)) {
            cli_out("%s: ERROR: unrecognized port bitmap: %s\n", ARG_CMD(a), c);
            return CMD_FAIL;
        }

        if ((c = ARG_CUR(a)) != NULL) {

            if (c[0] == '=') {
                return CMD_USAGE;        /* '=' unsupported */
            }

            parse_table_init(u, &pt);
            parse_table_add(&pt, "mode", PQ_DFL | PQ_STRING,0,
                    &mode_type, NULL);

            parse_table_add(&pt, "Sleep_Time", PQ_DFL | PQ_INT,
                        0, &sleep_time, NULL);

            parse_table_add(&pt, "Wake_Time", PQ_DFL | PQ_INT,
                        0, &wake_time, NULL);

            if (parse_arg_eq(a, &pt) < 0) {
                parse_arg_eq_done(&pt);
                return CMD_USAGE;
            }
            if (ARG_CNT(a) > 0) {
                cli_out("%s: Unknown argument %s\n", ARG_CMD(a), ARG_CUR(a));
                parse_arg_eq_done(&pt);
                return CMD_USAGE;
            }
            if (!mode_type) {
                cli_out("%s: Mode not assigned\n", ARG_CMD(a));
                parse_arg_eq_done(&pt);
                return CMD_USAGE;
            }
        } else {
            char * str;
            cli_out("Phy Power Mode dump:\n");
            cli_out("%10s %16s %14s %14s %14s\n",
                    "port", "name", "power_mode","sleep_time(ms)","wake_time(ms)");
            DPORT_SOC_PBMP_ITER(u, pbm, dport, p) {
                mode_value = 0;
                sleep_time = 0;
                wake_time  = 0;
                rv = bcm_port_phy_control_get(u,p,BCM_PORT_PHY_CONTROL_POWER,
                                          &mode_value);
                if (rv == SOC_E_NONE) {
                    if (mode_value == BCM_PORT_PHY_CONTROL_POWER_AUTO) {
                        str = "auto_down";
                        if ((rv = bcm_port_phy_control_get(u,p,
                                BCM_PORT_PHY_CONTROL_POWER_AUTO_SLEEP_TIME,
                                (uint32*) &sleep_time)) != SOC_E_NONE) {
                            sleep_time = 0;
                        }
                        if ((rv = bcm_port_phy_control_get(u,p,
                                BCM_PORT_PHY_CONTROL_POWER_AUTO_WAKE_TIME,
                                (uint32*) &wake_time)) != SOC_E_NONE) {
                            wake_time = 0;
                        }
                         
                    } else if (mode_value == BCM_PORT_PHY_CONTROL_POWER_LOW) {
                        str = "low";
                    } else {
                       str = "full";
                    }
                } else {
                    str = "unavail";
                }                
                cli_out("%5s(%3d) %16s %14s ",
                        SOC_PORT_NAME(u, p), p,
                        soc_phy_name_get(u, p),
                        str);
                if (sleep_time && wake_time) {
                    cli_out("%10d %14d\n", sleep_time,wake_time);
                } else {
                    cli_out("%10s %14s\n", "N/A","N/A");
                }
            }
            return CMD_OK;
        }

        if (sal_strcasecmp(mode_type, "full") == 0) {
            DPORT_SOC_PBMP_ITER(u, pbm, dport, p) {
                (void)bcm_port_phy_control_set(u,p,BCM_PORT_PHY_CONTROL_POWER,
                        BCM_PORT_PHY_CONTROL_POWER_FULL);
            }
        } else if (sal_strcasecmp(mode_type, "auto_down") == 0) {
            DPORT_SOC_PBMP_ITER(u, pbm, dport, p) {
                (void)bcm_port_phy_control_set(u,p,BCM_PORT_PHY_CONTROL_POWER,
                        BCM_PORT_PHY_CONTROL_POWER_AUTO);
                if (sleep_time >= 0) {
                    (void)bcm_port_phy_control_set(u,p,
                            BCM_PORT_PHY_CONTROL_POWER_AUTO_SLEEP_TIME,
                            sleep_time);
                }
                if (wake_time >= 0) {
                    (void)bcm_port_phy_control_set(u,p,
                            BCM_PORT_PHY_CONTROL_POWER_AUTO_WAKE_TIME,
                            wake_time);
                }
            }
        }

        /* free allocated memory from arg parsing */
        parse_arg_eq_done(&pt);
        return CMD_OK;
    }


    if (c == NULL) {
        return(CMD_USAGE);
    }

    /* Parse the bitmap. */
    if (parse_pbmp(u, c, &pbm) < 0) {
        cli_out("%s: ERROR: unrecognized port bitmap: %s\n",
                ARG_CMD(a), c);
        return CMD_FAIL;
    }

    SOC_PBMP_ASSIGN(pbm_phys, pbm);
    SOC_PBMP_AND(pbm_phys, PBMP_ALL(u));
    if (SOC_PBMP_IS_NULL(pbm_phys)) {
        cli_out("Ports specified do not have PHY drivers.\n");
    } else {
        SOC_PBMP_ASSIGN(pbm_temp, pbm);
        SOC_PBMP_REMOVE(pbm_temp, PBMP_ALL(u));
        if (SOC_PBMP_NOT_NULL(pbm_temp)) {
            cli_out("Not all ports given have PHY drivers.  Using %s\n",
                    SOC_PBMP_FMT(pbm_phys, pfmt));
        }
    }
    SOC_PBMP_ASSIGN(pbm, pbm_phys);

    if (ARG_CNT(a) == 0) {  /*  show information for all registers */
        PBMP_ITER(pbm, p) {
            phy_addr = (intermediate ?
                PORT_TO_PHY_ADDR_INT(u, p) :
                PORT_TO_PHY_ADDR(u, p));
            if (phy_addr == 0xff) {
                cli_out("Port %s: No %sPHY address assigned\n",
                        SOC_PORT_NAME(u, p),
                        intermediate ? "intermediate " : "");
                continue;
            }
            if (intermediate) {
                cli_out("Port %s (intermediate PHY addr 0x%02x):",
                        SOC_PORT_NAME(u, p), phy_addr);
            } else {
                SOC_IF_ERROR_RETURN(
                        bcm_port_phy_drv_name_get(u, p, drv_name, 64));
                cli_out("Port %s (PHY addr 0x%02x): %s (%s)",
                        SOC_PORT_NAME(u, p), phy_addr,
                        soc_phy_name_get(u, p), drv_name);
            }
            if ((!intermediate) && (soc_phy_is_c45_miim(u, p))) {
                for(i = 0; i < sizeof(p_devad)/sizeof(p_devad[0]); i++) {
                    phy_devad = p_devad[i];
                    cli_out("\nDevAd = %d(%s)", phy_devad, p_devad_str[i]);
                    for (phy_reg = PHY_MIN_REG; phy_reg <= PHY_MAX_REG; \
                       phy_reg++) {
                        rv = soc_miimc45_read(u, phy_addr,
                                phy_devad, phy_reg, &phy_data);
                        if (rv < 0) {
                            cli_out("\n");
                            cli_out("ERROR: Port %s: soc_miim_read failed: %s\n",
                                    SOC_PORT_NAME(u, p), soc_errmsg(rv));
                            return CMD_FAIL;
                        }
                        cli_out("%s\t0x%04x: 0x%04x",
                                ((phy_reg % DUMP_PHY_COLS) == 0) ? "\n" : "",
                                phy_reg, phy_data);
                    }
                }
            } else {
#if defined(BCM_NORTHSTAR_SUPPORT)
                if (IS_ROBO_CPU_MDIOBUS(u, p)) {
                    phy_addr |= PHY_ADDR_ROBO_CPU_MDIOBUS;
                }
#endif				
                for (phy_reg = PHY_MIN_REG; phy_reg <= PHY_MAX_REG; phy_reg++) {
                    rv = soc_miim_read(u, phy_addr, phy_reg, &phy_data);
                    if (rv < 0) {
                        cli_out("\nERROR: Port %s: soc_miim_read failed: %s\n",
                                SOC_PORT_NAME(u, p), soc_errmsg(rv));
                        return CMD_FAIL;
                    }
                    cli_out("%s\t0x%02x: 0x%04x",
                            ((phy_reg % DUMP_PHY_COLS) == 0) ? "\n" : "",
                            phy_reg, phy_data);
                }
            }
            cli_out("\n");
        }
    } else {                /* get register argument */
        c = ARG_GET(a);
        phy_reg = sal_ctoi(c, 0);

        if (ARG_CNT(a) == 0) {  /* no more args; show this register */
            PBMP_ITER(pbm, p) {
                phy_addr = (intermediate ?
                        PORT_TO_PHY_ADDR_INT(u, p) :
                        PORT_TO_PHY_ADDR(u, p));
                if (phy_addr == 0xff) {
                    cli_out("Port %s: No %sPHY address assigned\n",
                            SOC_PORT_NAME(u, p),
                            intermediate ? "intermediate " : "");
                    continue;
                }
                if ((!intermediate) && (soc_phy_is_c45_miim(u, p))) {
                    for(i = 0; i < sizeof(p_devad)/sizeof(p_devad[0]); i++) {
                        phy_devad = p_devad[i];
                        cli_out("Port %s (PHY addr 0x%02x) DevAd %d(%s) Reg %d:",
                                SOC_PORT_NAME(u, p), phy_addr, phy_devad,
                                p_devad_str[i], phy_reg);
                        rv = soc_miimc45_read(u, phy_addr,
                                phy_devad, phy_reg, &phy_data);
                        if (rv < 0) {
                            cli_out("\n");
                            cli_out("ERROR: Port %s: soc_miim_read failed: %s\n",
                                    SOC_PORT_NAME(u, p), soc_errmsg(rv));
                            return CMD_FAIL;
                        }
                        cli_out("0x%04x\n", phy_data);
                    }
                } else {
                    cli_out("Port %s (PHY addr 0x%02x) Reg %d: ",
                            SOC_PORT_NAME(u, p), phy_addr, phy_reg);
                    rv = soc_miim_read(u, phy_addr, phy_reg, &phy_data);
                    if (rv < 0) {
                        cli_out("\nERROR: Port %s: soc_miim_read failed: %s\n",
                                SOC_PORT_NAME(u, p), soc_errmsg(rv));
                        return CMD_FAIL;
                    }
                    cli_out("0x%04x\n", phy_data);
                }
            }
        } else {    /* set the reg to given value for the indicated phys */
            c = ARG_GET(a);
            phy_data = sal_ctoi(c, 0);
        
            PBMP_ITER(pbm, p) {
                phy_addr = (intermediate ?
                        PORT_TO_PHY_ADDR_INT(u, p) :
                        PORT_TO_PHY_ADDR(u, p));
                if (phy_addr == 0xff) {
                    cli_out("Port %s: No %sPHY address assigned\n",
                            SOC_PORT_NAME(u, p),
                            intermediate ? "intermediate " : "");
                    return CMD_FAIL;
                }
                if ((!intermediate) && (soc_phy_is_c45_miim(u, p))) {
                    phy_devad = phy_data;
                    i = ((phy_devad == p_devad[0]) ? 0 :
                        ((phy_devad == p_devad[1]) ? 1 :
                        ((phy_devad == p_devad[2]) ? 2 : -1)));
                    if (i == -1) {
                        cli_out("\nERROR: Port %s: Invalid DevAd %d\n",
                                SOC_PORT_NAME(u, p), phy_devad);
                        continue;
                    }
                    if (ARG_CNT(a) == 0) {  
                        /* no more args; show this register */
                        cli_out("Port %s (PHY addr 0x%02x) DevAd %d(%s) Reg %d:",
                                SOC_PORT_NAME(u, p), phy_addr, phy_devad,
                                p_devad_str[i], phy_reg);
                        rv = soc_miimc45_read(u, phy_addr,
                                phy_devad, phy_reg, &phy_data);
                        if (rv < 0) {
                            cli_out("\n");
                            cli_out("ERROR: Port %s: soc_miim_read failed: %s\n",
                                    SOC_PORT_NAME(u, p), soc_errmsg(rv));
                            return CMD_FAIL;
                        }
                        cli_out("0x%04x\n", phy_data);
                    } else { /* write */
                        c = ARG_GET(a);
                        phy_data = sal_ctoi(c, 0);
                        rv = soc_miimc45_write(u, phy_addr,
                                phy_devad, phy_reg, phy_data);
                        if (rv < 0) {
                            cli_out("ERROR: Port %s: soc_miim_write failed:%s\n",
                                    SOC_PORT_NAME(u, p), soc_errmsg(rv));
                            return CMD_FAIL;
                        }
                    }
                } else {
                    rv = soc_miim_write(u, phy_addr, phy_reg, phy_data);
                    if (rv < 0) {
                        cli_out("ERROR: Port %s: soc_miim_write failed: %s\n",
                                SOC_PORT_NAME(u, p), soc_errmsg(rv));
                        return CMD_FAIL;
                    }
                }
            }
        }
    }
    return CMD_OK;
}

cmd_result_t
if_robo_dscp(int unit, args_t *a)
{
    int  rv, port, srccp, mapcp, prio, cng, mode, count, i, map;
    char  *s;
    parse_table_t  pt;
    char 		*subcmd;
    int pkt_dscp, int_prio, dscp, ecn;
    int prio_min, prio_max, color_min, color_max;
    int color, color_parse = bcmColorGreen;
    cmd_result_t	retCode;    


    if (! sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }
    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }
    port = -1;
    if (sal_strcasecmp(subcmd, "unmap") == 0) {        
        pkt_dscp = int_prio = -1;

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "IntPrio", PQ_INT|PQ_DFL, 0, &int_prio, NULL);
        parse_table_add(&pt, "Color", PQ_MULTI|PQ_DFL, 0,
                        &color_parse, diag_robo_parse_color);
        parse_table_add(&pt, "dscp", PQ_INT|PQ_DFL, 0, &pkt_dscp, NULL);

        if (ARG_CNT(a)) { 
            if (!parseEndOk( a, &pt, &retCode)) {
                return retCode;
            }
    }
        if (pkt_dscp < 0 || int_prio < 0) {
            cli_out("DSCP unmap settings \n");

            if (SOC_IS_TBX(unit)){
                prio_min = 0;
                prio_max =_BCM_PRIO_MAX(unit);
                color_min = bcmColorGreen;
                color_max = bcmColorPreserve;                    
            } else {
                prio_min = prio_max = 0;
                color_min = color_max = 0;
            }
            for (prio = prio_min; prio <= prio_max; prio++) {
                for (color = color_min; 
                     color <= color_max; 
                     color++) {
                    if ((rv = bcm_port_dscp_unmap_get(unit, port, 
                            prio,
                            color, 
                            &pkt_dscp)) < 0) {
                        goto bcm_err; 
                    }
                    if (pkt_dscp & BCM_DSCP_ECN) {
                        dscp = (pkt_dscp & ~BCM_DSCP_ECN) >> 0x2;
                        ecn = (pkt_dscp & ~BCM_DSCP_ECN) & 0x3;
                        cli_out("Internal Prio=%d, Color=%s, Packet dscp=%d ecn=%d\n ",
                                prio, diag_robo_parse_color[color], dscp, ecn);

                    } else {
                        cli_out("Internal Prio=%d, Color=%s, Packet dscp=%d \n ",
                                prio, diag_robo_parse_color[color], pkt_dscp);
                    }
                } 
            }
        } else {
             color = (bcm_color_t) color_parse;
             if ((rv = bcm_port_dscp_unmap_set(unit, port, int_prio, 
                                             color, pkt_dscp)) < 0) {
                 goto bcm_err;
             }
         }
        return CMD_OK;
 bcm_err:
        cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
        return CMD_FAIL;

    }

    if (sal_strcasecmp(subcmd, "map") == 0) {     
    srccp = -1;
    s = ARG_GET(a);
    if (s) {
        srccp = parse_integer(s);
    }

    if ((s = ARG_GET(a)) == NULL) {
        if (srccp < 0) {
            srccp = 0;
            if (SOC_IS_TBX(unit) && !SOC_IS_TB_AX(unit)) {
                count = 256;
                srccp |= BCM_DSCP_ECN;
            } else {
                count = 64;
            }
        } else {
            count = 1;
        }
            port = -1;
            rv = bcm_port_dscp_map_get(unit, port, 0, &mapcp, &prio);
            if (rv == BCM_E_PORT) {
                port = -1;
                cli_out("%d: dscp map:\n", unit);
            }
            for (i = 0; i < count; i++) {
                rv = bcm_port_dscp_map_get(unit, port, srccp + i, &mapcp, &prio);
                if (rv < 0) {
                    cli_out("ERROR: dscp map get %d failed: %s\n",
                            srccp + i, bcm_errmsg(rv));
                    return CMD_FAIL;
                }
                if (SOC_IS_TBX(unit) && !SOC_IS_TB_AX(unit)) {
                cli_out(" pkt dscp %d  ecn %d prio=%d cng=%s\n",
                        ((srccp + i)&0xFC)>>2, ((srccp +i)&0x3),
                        prio & BCM_PRIO_MASK,
                        (prio & BCM_PRIO_GREEN) ? diag_robo_parse_color[0] :
                        (prio & BCM_PRIO_YELLOW) ?diag_robo_parse_color[1]   :
                        (prio & BCM_PRIO_RED) ? diag_robo_parse_color[2]  : 
                        (prio & BCM_PRIO_PRESERVE) ? diag_robo_parse_color[3]  : 
                        diag_robo_parse_color[0] );
                } else {
                    cli_out(" pkt dscp %d prio=%d cng=%s\n",
                            srccp + i, 
                            prio & BCM_PRIO_MASK,
                            (prio & BCM_PRIO_GREEN) ? diag_robo_parse_color[0] :
                            (prio & BCM_PRIO_YELLOW) ?diag_robo_parse_color[1]   :
                            (prio & BCM_PRIO_RED) ? diag_robo_parse_color[2]  : 
                            (prio & BCM_PRIO_PRESERVE) ? diag_robo_parse_color[3]  : 
                            diag_robo_parse_color[0] );
                }
            }
            return CMD_OK;
        } else {
            cng = -1;
	    prio = parse_integer(s);
    	    if ((s = ARG_GET(a)) != NULL) {
	        cng = parse_integer(s);
            }

        if (cng == bcmColorGreen) {
            prio |= BCM_PRIO_GREEN;
        } else if (cng == bcmColorYellow) {
            prio |= BCM_PRIO_YELLOW;
        } else if (cng == bcmColorRed) {
            prio |= BCM_PRIO_RED;
        } else if (cng == bcmColorPreserve) {
            prio |= BCM_PRIO_DROP_LAST;
        }

        /*  dscp mapping */
        port = -1;
        if (SOC_IS_TBX(unit) && !SOC_IS_TB_AX(unit)) {
            if (srccp & BCM_DSCP_ECN) {
                mapcp = (srccp &~ BCM_DSCP_ECN) >> 2;
            } else {
                mapcp = srccp;
            }
        } else {
            mapcp = srccp;
        }
        rv = bcm_port_dscp_map_set(unit, port, srccp, mapcp, prio);
        if (rv < 0) {
            cli_out("%d: ERROR: "
                    "dscp %d map prio=0x%x cng=%d failed: %s\n",
                    unit, srccp, prio, cng, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;
        }
    }
    if (sal_strcasecmp(subcmd, "mode") == 0) {     
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Mode", PQ_INT, (void *)-1, &mode, NULL);
        rv = parse_arg_eq(a, &pt);
        parse_arg_eq_done(&pt);
        if (BCM_FAILURE(rv)) {
            return CMD_FAIL;
        }
            if (mode != -1) {
                rv = bcm_port_dscp_map_mode_set(unit, port, mode);
                if (rv < 0) {
                    cli_out("%d:%s ERROR: dscp mode set mode=%d: %s\n",
                            unit, (port == -1) ? "" : BCM_PORT_NAME(unit, port),
                            mode, bcm_errmsg(rv));
                    return CMD_FAIL;
                }
        } else {
            map = -1;
            rv = bcm_port_dscp_map_mode_get(unit, port, &map);    
            if (rv < 0) {
                cli_out("fail to get dscp mode!\n");
                return CMD_FAIL;
            }
            if (map == BCM_PORT_DSCP_MAP_ALL) {
                cli_out("DSCP MAP ALL mode \n");
            } else {
                cli_out("DSCP MAP Disabled !\n");
            }
        }
        return CMD_OK;
    }
    return CMD_USAGE;
}

char if_stat_usage[] =
    "Usages:\n\t"
    "  stat <port>  - Show SNMP statistics on the given port.\n";
/*
    "  stat <port> [interval]  - Show SNMP statistics on the given port\n\t"
    "                            If interval is specified (in seconds),\n\t"
    "                            the statistics are displayed continuously\n\t"
    "                            after time period has elapsed.\n";
*/
cmd_result_t
if_stat(int unit, args_t* a)
{
    char *c;
    soc_port_t port = 0;
    bcm_stat_val_t s;
    char *sname;
    char *_stat_names[] = BCM_STAT_NAME_INITIALIZER;
    int rv;
    uint64 val;
    int f_parse_fail = TRUE;

    if ((c = ARG_GET(a)) != NULL) {     /* Ports specified? */
        if (isint(c)) {
            port = parse_integer(c);
            if ((port < SOC_ROBO_MAX_NUM_PORTS) && (port >= 0)) {
                f_parse_fail = FALSE;
            }
        }    
    }
    
    if (f_parse_fail) {
        cli_out("%s: Error: no specified port number.\n",ARG_CMD(a));
        return CMD_FAIL;
    }

    cli_out("%s: Statistics for Unit %d port %s\n",
            ARG_CMD(a), unit, SOC_PORT_NAME(unit, port));
    for (s = 0; s < snmpValCount; s++) {
        sname = _stat_names[s];
        rv = bcm_stat_get(unit, port, s, &val);;
        if (rv < 0) {
            cli_out("%8s\t%s (stat %d): %s\n",
                    "-", sname, s, bcm_errmsg(rv));
            continue;
        }

        if (COMPILER_64_HI(val) == 0) {
            cli_out("%8u\t%s (stat %d)\n",
                    COMPILER_64_LO(val), sname, s);
        } else {
            cli_out("0x%08x%08x\t%s (stat %d)\n",
                    COMPILER_64_HI(val),
                    COMPILER_64_LO(val),
                    sname, s);
        }
    }
    return CMD_OK;
}


static struct {			/* match enum from bcm/port.h */
    int		type;
    char	*name;
} pcontrol_names[] = {
    { bcmPortControlPreservePacketPriority, "preserve_pktpri" },
    { bcmPortControlIngressRateControlDrop, "ingressratecontrol_drop" },
    { bcmPortControlIngressTcSelectTable,   "ingress_tcselect" },
    { bcmPortControlEgressVlanPriUsesPktPri,"egresspri_usepktpri" },
    { bcmPortControlEgressVlanCfiUsesPktPri,"egresscfi_usepktcfi" },
    { bcmPortControlEgressModifyDscp,       "egress_modifydscp" },
    { bcmPortControlEgressModifyECN,        "egress_modifyecn" },
    { bcmPortControlEgressModifyOuterPktPri,         "egress_modify_outerpri" },
    { bcmPortControlEgressModifyInnerPktPri,         "egress_modify_innerpri" },
    { bcmPortControlTCPriority,             "tcpriority" },
    { bcmPortControlDropPrecedence,         "droppercedence" },
    { bcmPortControlEEEEnable,         "eee_enable" },
    { bcmPortControlEEETransmitIdleTime,         "eee_tx_idle_time" },
    { bcmPortControlEEETransmitIdleTimeHund,         "eee_tx_idle_time_100m" },
    { bcmPortControlEEETransmitWakeTime,         "eee_tx_wake_time" },
    { bcmPortControlEEETransmitWakeTimeHund,         "eee_tx_wake_time_100m" },
    { bcmPortControlEEETransmitMinLPITime,         "eee_tx_min_lpi_time" },
    { bcmPortControlEEETransmitMinLPITimeHund,         "eee_tx_min_lpi_time_100m" },
    { bcmPortControlEEETransmitEventCount,         "eee_tx_event" },
    { bcmPortControlEEETransmitDuration,         "eee_tx_duration" },
    { bcmPortControlFilterTcpUdpPktEnable,         "filtertcpudppktenable" },
    { bcmPortControlFloodPktDropTc,         "floodpktdrop_tc" },
    { 0, NULL }      /* LAST ENTRY */
};

char if_robo_port_control_usage[] =
    "Set/Display port control types.\n"
    "Parameters: <pbm> [type_name] [value]\n";
/*
 * Function:    if_robo_port_control
 * Purpose: Configure default IPG values.
 * Parameters:  unit - SOC unit #
 *      a - arguments
 * Returns: CMD_OK/CMD_FAIL
 */
cmd_result_t
if_robo_port_control(int unit, args_t *a)
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
            cli_out("\n Port %s:\n", SOC_PORT_NAME(unit, p));
            for (i = 0; pcontrol_names[i].name != NULL; i++) {
                type = pcontrol_names[i].type;
                rv = bcm_port_control_get(unit, p, type, &val);
                cli_out("\t %-45s= ", pcontrol_names[i].name);
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
            cli_out("\n Port %s:\n", SOC_PORT_NAME(unit, p));
            for (i = 0; pcontrol_names[i].name != NULL; i++) {
                if (sal_strcasecmp(c, pcontrol_names[i].name) == 0){
                    
                    type = pcontrol_names[i].type;

                    if (val_str != NULL){   
                        /* set process */
                        val = parse_integer(val_str);
                        rv = bcm_port_control_set(unit, p, type, val);
                        if (rv) {
                            cli_out("\t Failed on setting type=%s(%d),%s\n", 
                                    pcontrol_names[i].name, 
                                    pcontrol_names[i].type, 
                                    bcm_errmsg(rv));
                        } else {
                            cli_out("\t %-45s(%d) set value=%d\n", 
                                    pcontrol_names[i].name, 
                                    pcontrol_names[i].type, 
                                    val);
                        }
                    } else {
                        /* get process */
                        rv = bcm_port_control_get(unit, p, type, &val);
                        cli_out("\t %-45s= ", pcontrol_names[i].name);
                        if (rv){
                            cli_out("%s\n", bcm_errmsg(rv));
                        } else {
                            cli_out("0x%x\n", val);
                        }
                    }
                    
                    break;
                }
            }
            
            if (pcontrol_names[i].name == NULL){
                cli_out("\t Unknown Port Control type at %s!\n", c);
                return CMD_FAIL;
            }
        }
    }
        
    return CMD_OK;
}

char if_robo_port_cross_connect_usage[] =
    "Set/Display port cross connect information.\n"
    "Parameters: <port> <enable/disable> <egress_port>\n";
/*
 * Function:    if_robo_port_cross_connect
 * Purpose: Configure port cross connect values.
 * Parameters:  unit - SOC unit #
 *      a - arguments
 * Returns: CMD_OK/CMD_FAIL
 */
cmd_result_t
if_robo_port_cross_connect(int unit, args_t *a)
{
    char        *c;
    bcm_port_t  p, egr_p;
    int         rv, val = 0;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if ((c = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    if (parse_port(unit, c, &p) < 0) {
        cli_out("%s: ERROR: unrecognized port number: %s\n",
                ARG_CMD(a), c);
        return CMD_FAIL;
    }

    c = ARG_GET(a);     /* NULL or media type or command */

    if (c == NULL) {
        /* Display the port cross connect info */
        rv = bcm_port_force_forward_get(unit, p,  &egr_p, &val);
        cli_out("\n Port %s: Cross Connect ", SOC_PORT_NAME(unit, p));
        if (rv) {
            cli_out("%s\n", bcm_errmsg(rv));
        } else {
            if (val) {
                cli_out("Enabled, Egress port is %s\n", SOC_PORT_NAME(unit, egr_p));
            } else {
                cli_out("Disabled.\n");
            }
        }
    } else {
        if (!sal_strcasecmp(c, "disable")) {
            c = ARG_GET(a);
            if (parse_port(unit, c, &egr_p) < 0) {
                cli_out("%s: ERROR: unrecognized egress port number: %s\n",
                        ARG_CMD(a), c);
                return CMD_FAIL;
            }
            val = 0;
            rv = bcm_port_force_forward_set(unit, p, egr_p, val);
            if (rv) {
                cli_out("%s\n", bcm_errmsg(rv));
            }
            return CMD_OK;
            
        } else if (!sal_strcasecmp(c, "enable")) {
            c = ARG_GET(a);
            if (parse_port(unit, c, &egr_p) < 0) {
                cli_out("%s: ERROR: unrecognized egress port number: %s\n",
                        ARG_CMD(a), c);
                return CMD_FAIL;
            }
            val = 1;
            rv = bcm_port_force_forward_set(unit, p, egr_p, val);
            if (rv) {
                cli_out("%s\n", bcm_errmsg(rv));
            }
            return CMD_OK;
        } else {
            return CMD_USAGE;
        }
    }
        
    return CMD_OK;
}



#ifdef IMP_SW_PROTECT
#define IMP_PROT_GET_NUMB(numb, str, args) \
    if (((str) = ARG_GET(args)) == NULL) { \
        return CMD_USAGE; \
    } \
    (numb) = parse_integer(str);

cmd_result_t
if_robo_imp_protect(int unit, args_t * args)
{
    char *c;
    int rv;
    int min_rate, mid_rate, max_rate;

    if (ARG_CUR(args) != NULL) {
        IMP_PROT_GET_NUMB(min_rate, c, args);
        IMP_PROT_GET_NUMB(mid_rate, c, args);
        IMP_PROT_GET_NUMB(max_rate, c, args);

        if ((min_rate > mid_rate) || (min_rate > max_rate)) {
            cli_out("The Min rate is larger than Midlle rate or Max rate.\n");
            return CMD_FAIL;
        }
        if (mid_rate > max_rate) {
            cli_out("The Middle rate is larger than Max rate.\n");
            return CMD_FAIL;
        }

        rv = soc_imp_prot_set(unit, min_rate, mid_rate, max_rate);
        if (rv < 0) {
            cli_out("soc_imp_prot_set : failed! \n");
            return CMD_FAIL;
        }
    } else {
        soc_imp_prot_dump(unit);
    }

    return CMD_OK;
}

#endif /* IMP_SW_PROTECT */

char if_dos_attack_usage[] =
    "DoS Attack enable/disable\n"
    "Usage :\n\t"
    "   dos [pbm] enable [0|1|2|...|15]\n\t"
    "   dos [pbm] disable [0|1|2|...|15]\n\t"
    "   dos [pbm] show\n\t"
#ifndef COMPILER_STRING_CONST_LIMIT
    "0: Source IP equal Destination IP\n\t"
    "1: TCP flags\n\t"
    "2: Source Port equal Destination Port\n\t"
    "3: smaller than defined TCP Header Size\n\t"
    "4: Ping Flood\n\t"
    "5: SYN Flood\n\t"
    "6: TCP SMURF\n\t"
    "7: TCP Source Port equal Destination Port\n\t"
    "8: UDP Source Port equal Destination Port\n\t"
    "9: TCP flags : All TCP flags = 0\n\t"
    "10: TCP flags : FIN = 1 & URG = 1 & PSH = 1\n\t"
    "11: TCP flags : SYN = 1 & FIN = 1\n\t"
    "12: TCP flags : SYN = 1 & ACK = 0 & SRC_Port < 1024\n\t"
    "13: Fragmented ICMP packets check\n\t"
    "14: Enable ICMP size check\n\t"
    "15: V4 first fragment check\n"
#endif
    ;

cmd_result_t
if_dos_attack(int unit, args_t * args)
{
    char *c;
    int rv = CMD_OK;
    int x, enable, type = 0;
    bcm_port_t port = 0;
    pbmp_t pbm, all_pbm;
    int temp_type = 0;

    if ((c = ARG_GET(args)) == NULL) {
        return(CMD_USAGE);
    }

    if (parse_pbmp(unit, c, &pbm) < 0) {
        cli_out("%s: Error: unrecognized port bitmap: %s\n",
                ARG_CMD(args), c);
        return CMD_FAIL;
    }

    BCM_PBMP_AND(pbm, PBMP_PORT_ALL(unit));
    BCM_PBMP_REMOVE(pbm, PBMP_CMIC(unit));

    BCM_PBMP_ASSIGN(all_pbm, PBMP_PORT_ALL(unit));
    BCM_PBMP_REMOVE(all_pbm, PBMP_CMIC(unit));

    if (BCM_PBMP_IS_NULL(pbm)) {
        ARG_DISCARD(args);
        return CMD_USAGE;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    c = ARG_GET(args);
    /* Enable Gloable setting by hardware */
    if (BCM_PBMP_EQ(pbm, all_pbm)) {
        if (!sal_strcasecmp(c, "show")) {
            for (x = 0; x < 16; x++) {
                cli_out("Item[%2d] is ", x);
                switch (x) {
                    case 0:
                        type = bcmSwitchDosAttackSipEqualDip;
                        break;
                    case 1:
                        type = bcmSwitchDosAttackTcpFlags;
                        break;
                    case 2:
                        type = bcmSwitchDosAttackL4Port;
                        break;
                    case 3:
                        type = bcmSwitchDosAttackTcpFrag;
                        break;
                    case 4:
                        type = bcmSwitchDosAttackPingFlood;
                        break;
                    case 5:
                        type = bcmSwitchDosAttackSynFlood;
                        break;
                    case 6:
                        type = bcmSwitchDosAttackTcpSmurf;
                        break;
                    case 7:
                        type = bcmSwitchDosAttackTcpPortsEqual;
                        break;
                    case 8:
                        type = bcmSwitchDosAttackUdpPortsEqual;
                        break;
                    case 9:
                        type = bcmSwitchDosAttackFlagZeroSeqZero;
                        break;
                    case 10:
                        type = bcmSwitchDosAttackTcpFlagsFUP;
                        break;
                    case 11:
                        type = bcmSwitchDosAttackTcpFlagsSF;
                        break;
                    case 12:
                        type = bcmSwitchDosAttackSynFrag;
                        break;
                    case 13:
                        type = bcmSwitchDosAttackIcmpFragments;
                        break;
                    case 14:
                        type = bcmSwitchDosAttackIcmp;
                        break;
                    case 15:
                        type = bcmSwitchDosAttackV4FirstFrag;
                        break;
                }

                rv = bcm_switch_control_get(unit, type, &enable);
                if (rv < 0) {
                    cli_out("%s\n",soc_errmsg(rv));
                } else {
                    if (enable) {
                        cli_out("enabled\n");
                    } else {
                        cli_out("disabled\n");
                    }
                }
            }
        } else if (!sal_strcasecmp(c, "enable")) {
            /* Enable DoS Attack Cases */
            c = ARG_GET(args);
            if (c == 0) {
                return CMD_USAGE;
            }
            x = parse_integer(c);
            switch (x) {
                case 0:
                    temp_type = bcmSwitchDosAttackSipEqualDip;
                    break;
                case 1:
                    temp_type = bcmSwitchDosAttackTcpFlags;
                    break;
                case 2:
                    temp_type = bcmSwitchDosAttackL4Port;
                    break;
                case 3:
                    temp_type = bcmSwitchDosAttackTcpFrag;
                    break;
                case 4:
                    temp_type = bcmSwitchDosAttackPingFlood;
                    break;
                case 5:
                    temp_type = bcmSwitchDosAttackSynFlood;
                    break;
                case 6:
                    temp_type = bcmSwitchDosAttackTcpSmurf;
                    break;
                case 7:
                    temp_type = bcmSwitchDosAttackTcpPortsEqual;
                    break;
                case 8:
                    temp_type = bcmSwitchDosAttackUdpPortsEqual;
                    break;
                case 9:
                    temp_type = bcmSwitchDosAttackFlagZeroSeqZero;
                    break;
                case 10:
                    temp_type = bcmSwitchDosAttackTcpFlagsFUP;
                    break;
                case 11:
                    temp_type = bcmSwitchDosAttackTcpFlagsSF;
                    break;
                case 12:
                    temp_type = bcmSwitchDosAttackSynFrag;
                    break;
                case 13:
                    temp_type = bcmSwitchDosAttackIcmpFragments;
                    break;
                case 14:
                    temp_type = bcmSwitchDosAttackIcmp;
                    break;
                case 15:
                    temp_type = bcmSwitchDosAttackV4FirstFrag;
                    break;
                default:
                    cli_out("Case Number Invalid\n");
                    return CMD_FAIL;
                
            }

            rv = bcm_switch_control_set(unit, temp_type, 1);
            if (rv < 0) {
                cli_out("SET ENABLE FAIL: rv = %s\n",soc_errmsg(rv));
                return CMD_FAIL;
            }
        } else if (!sal_strcasecmp(c, "disable")) {
            c = ARG_GET(args);
            if (c == 0) {
                return CMD_USAGE;
            }
            x = parse_integer(c);
            switch (x) {
                case 0:
                    type = bcmSwitchDosAttackSipEqualDip;
                    break;
                case 1:
                    type = bcmSwitchDosAttackTcpFlags;
                    break;
                case 2:
                    type = bcmSwitchDosAttackL4Port;
                    break;
                case 3:
                    type = bcmSwitchDosAttackTcpFrag;
                    break;
                case 4:
                    type = bcmSwitchDosAttackPingFlood;
                    break;
                case 5:
                    type = bcmSwitchDosAttackSynFlood;
                    break;
                case 6:
                    type = bcmSwitchDosAttackTcpSmurf;
                    break;
                case 7:
                    type = bcmSwitchDosAttackTcpPortsEqual;
                    break;
                case 8:
                    type = bcmSwitchDosAttackUdpPortsEqual;
                    break;
                case 9:
                    type = bcmSwitchDosAttackFlagZeroSeqZero;
                    break;
                case 10:
                    type = bcmSwitchDosAttackTcpFlagsFUP;
                    break;
                case 11:
                    type = bcmSwitchDosAttackTcpFlagsSF;
                    break;
                case 12:
                    type = bcmSwitchDosAttackSynFrag;
                    break;
                case 13:
                    type = bcmSwitchDosAttackIcmpFragments;
                    break;
                case 14:
                    type = bcmSwitchDosAttackIcmp;
                    break;
                case 15:
                    type = bcmSwitchDosAttackV4FirstFrag;
                    break;
                default:
                    cli_out("Case Number Invalid\n");
                    return CMD_FAIL;
                
            }

            rv = bcm_switch_control_set(unit, type, 0);
            if (rv < 0) {
                cli_out("SET DISABLE FAIL: rv = %s\n",soc_errmsg(rv));
                return CMD_FAIL;
            }
        }  else {
            return CMD_USAGE;
        }
    } else {
        /* Enable per-port setting by CFP */
        if (!sal_strcasecmp(c, "show")) {
            PBMP_ITER(pbm, port) {
                cli_out("Port %s\n", SOC_PORT_NAME(unit, port));
                for (x = 0; x < 16; x++) {
                    cli_out("Item[%2d] is ", x);
                    switch (x) {
                        case 0:
                            type = bcmSwitchDosAttackSipEqualDip;
                            break;
                        case 1:
                            type = bcmSwitchDosAttackTcpFlags;
                            break;
                        case 2:
                            type = bcmSwitchDosAttackL4Port;
                            break;
                        case 3:
                            type = bcmSwitchDosAttackTcpFrag;
                            break;
                        case 4:
                            type = bcmSwitchDosAttackPingFlood;
                            break;
                        case 5:
                            type = bcmSwitchDosAttackSynFlood;
                            break;
                        case 6:
                            type = bcmSwitchDosAttackTcpSmurf;
                            break;
                        case 7:
                            type = bcmSwitchDosAttackTcpPortsEqual;
                            break;
                        case 8:
                            type = bcmSwitchDosAttackUdpPortsEqual;
                            break;
                        case 9:
                            type = bcmSwitchDosAttackFlagZeroSeqZero;
                            break;
                        case 10:
                            type = bcmSwitchDosAttackTcpFlagsFUP;
                            break;
                        case 11:
                            type = bcmSwitchDosAttackTcpFlagsSF;
                            break;
                        case 12:
                            type = bcmSwitchDosAttackSynFrag;
                            break;
                        case 13:
                            type = bcmSwitchDosAttackIcmpFragments;
                            break;
                        case 14:
                            type = bcmSwitchDosAttackIcmp;
                            break;
                        case 15:
                            type = bcmSwitchDosAttackV4FirstFrag;
                            break;
                    }

                    rv = bcm_switch_control_port_get(unit, port, type, &enable);
                    if (rv < 0) {
                        cli_out("%s\n",soc_errmsg(rv));
                    } else {
                        if (enable) {
                            cli_out("enabled\n");
                        } else {
                            cli_out("disabled\n");
                        }
                    }
                }
            }
        } else if (!sal_strcasecmp(c, "enable")) {
            /* Enable DoS Attack Cases */
            c = ARG_GET(args);
            if (c == 0) {
                return CMD_USAGE;
            }
            x = parse_integer(c);
            switch (x) {
                case 0:
                    temp_type = bcmSwitchDosAttackSipEqualDip;
                    break;
                case 1:
                    temp_type = bcmSwitchDosAttackTcpFlags;
                    break;
                case 2:
                    temp_type = bcmSwitchDosAttackL4Port;
                    break;
                case 3:
                    temp_type = bcmSwitchDosAttackTcpFrag;
                    break;
                case 4:
                    temp_type = bcmSwitchDosAttackPingFlood;
                    break;
                case 5:
                    temp_type = bcmSwitchDosAttackSynFlood;
                    break;
                case 6:
                    temp_type = bcmSwitchDosAttackTcpSmurf;
                    break;
                case 7:
                    temp_type = bcmSwitchDosAttackTcpPortsEqual;
                    break;
                case 8:
                    temp_type = bcmSwitchDosAttackUdpPortsEqual;
                    break;
                case 9:
                    temp_type = bcmSwitchDosAttackFlagZeroSeqZero;
                    break;
                case 10:
                    temp_type = bcmSwitchDosAttackTcpFlagsFUP;
                    break;
                case 11:
                    temp_type = bcmSwitchDosAttackTcpFlagsSF;
                    break;
                case 12:
                    temp_type = bcmSwitchDosAttackSynFrag;
                    break;
                case 13:
                    temp_type = bcmSwitchDosAttackIcmpFragments;
                    break;
                case 14:
                    temp_type = bcmSwitchDosAttackIcmp;
                    break;
                case 15:
                    temp_type = bcmSwitchDosAttackV4FirstFrag;
                    break;
                default:
                    cli_out("Case Number Invalid\n");
                    return CMD_FAIL;
                
            }

            PBMP_ITER(pbm, port) {
                rv = bcm_switch_control_port_set(unit, port, temp_type, 1);
                if (rv < 0) {
                    cli_out("SET ENABLE FAIL: rv = %s\n",soc_errmsg(rv));
                    return CMD_FAIL;
                }
            }
        } else if (!sal_strcasecmp(c, "disable")) {
            c = ARG_GET(args);
            if (c == 0) {
                return CMD_USAGE;
            }
            x = parse_integer(c);
            switch (x) {
                case 0:
                    type = bcmSwitchDosAttackSipEqualDip;
                    break;
                case 1:
                    type = bcmSwitchDosAttackTcpFlags;
                    break;
                case 2:
                    type = bcmSwitchDosAttackL4Port;
                    break;
                case 3:
                    type = bcmSwitchDosAttackTcpFrag;
                    break;
                case 4:
                    type = bcmSwitchDosAttackPingFlood;
                    break;
                case 5:
                    type = bcmSwitchDosAttackSynFlood;
                    break;
                case 6:
                    type = bcmSwitchDosAttackTcpSmurf;
                    break;
                case 7:
                    type = bcmSwitchDosAttackTcpPortsEqual;
                    break;
                case 8:
                    type = bcmSwitchDosAttackUdpPortsEqual;
                    break;
                case 9:
                    type = bcmSwitchDosAttackFlagZeroSeqZero;
                    break;
                case 10:
                    type = bcmSwitchDosAttackTcpFlagsFUP;
                    break;
                case 11:
                    type = bcmSwitchDosAttackTcpFlagsSF;
                    break;
                case 12:
                    type = bcmSwitchDosAttackSynFrag;
                    break;
                case 13:
                    type = bcmSwitchDosAttackIcmpFragments;
                    break;
                case 14:
                    type = bcmSwitchDosAttackIcmp;
                    break;
                case 15:
                    type = bcmSwitchDosAttackV4FirstFrag;
                    break;
                default:
                    cli_out("Case Number Invalid\n");
                    return CMD_FAIL;
                
            }

            PBMP_ITER(pbm, port) {
                rv = bcm_switch_control_port_set(unit, port, type, 0);
                if (rv < 0) {
                    cli_out("SET DISABLE FAIL: rv = %s\n",soc_errmsg(rv));
                    return CMD_FAIL;
                }
            }
        } else {
            return CMD_USAGE;
        }
    }
    return rv;
}



