/* 
 * $Id: port.c,v 1.30.20.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        port.c
 * Purpose:     PORT CLI commands
 *
 */

#include <shared/bsl.h>

#include <shared/alloc.h>

#include <appl/diag/system.h>
#include <soc/sbx/sbx_drv.h>
#include <appl/diag/shell.h>
#include <appl/diag/parse.h>
#include <appl/diag/diag.h>
#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3.h>
#endif

#include <bcm/error.h>
#include <bcm/debug.h>
#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3.h>
#endif
#include <bcm/port.h>
#include <bcm/init.h>


/* Port attributes to always get */
#define SBX_DIAG_PORT_ATTR_INIT_MASK    \
    (BCM_PORT_ATTR_LINKSTAT_MASK | BCM_PORT_ATTR_AUTONEG_MASK | \
     BCM_PORT_ATTR_ENABLE_MASK)

/* Current supported port attributes */
#define SBX_DIAG_E_PORT_ATTR_MASK    \
    (BCM_PORT_ATTR_ENABLE_MASK       | BCM_PORT_ATTR_AUTONEG_MASK    | \
     BCM_PORT_ATTR_LOCAL_ADVERT_MASK | BCM_PORT_ATTR_SPEED_MASK      | \
     BCM_PORT_ATTR_DUPLEX_MASK       | BCM_PORT_ATTR_LINKSCAN_MASK   | \
     BCM_PORT_ATTR_UNTAG_PRI_MASK    | BCM_PORT_ATTR_PHY_MASTER_MASK | \
     BCM_PORT_ATTR_INTERFACE_MASK    | BCM_PORT_ATTR_LOOPBACK_MASK   | \
     BCM_PORT_ATTR_PAUSE_MAC_MASK    | BCM_PORT_ATTR_PAUSE_MASK      | \
     BCM_PORT_ATTR_ENCAP_MASK        | BCM_PORT_ATTR_FRAME_MAX_MASK  | \
     BCM_PORT_ATTR_MDIX_MASK         | BCM_PORT_ATTR_LINKSTAT_MASK)

#define SBX_DIAG_HG_INTF_ATTR_MASK    \
    (BCM_PORT_ATTR_ENABLE_MASK       | BCM_PORT_ATTR_AUTONEG_MASK    | \
     BCM_PORT_ATTR_DUPLEX_MASK       | BCM_PORT_ATTR_SPEED_MASK      | \
     BCM_PORT_ATTR_INTERFACE_MASK    | BCM_PORT_ATTR_LOOPBACK_MASK   | \
     BCM_PORT_ATTR_ENCAP_MASK        | BCM_PORT_ATTR_FRAME_MAX_MASK  | \
     BCM_PORT_ATTR_LINKSCAN_MASK     | BCM_PORT_ATTR_LINKSTAT_MASK)

#define SBX_DIAG_HG_SUBPORT_INTF_ATTR_MASK    0

#define SBX_DIAG_G2XX_UNSUPPORTED_ATTR_MASK     \
    ( BCM_PORT_ATTR_UNTAG_PRI_MASK | BCM_PORT_ATTR_FRAME_MAX_MASK )  

/* Iterate thru a port bitmap with the given mask; display info */
STATIC int
sbx_port_disp_iter(int unit, pbmp_t pbm, pbmp_t pbm_mask, uint32 seen)
{
    bcm_port_info_t info;
    soc_port_t port;
    int r;

    BCM_PBMP_AND(pbm, pbm_mask);
    BCM_PBMP_ITER(pbm, port) {

        sal_memset(&info, 0, sizeof(info));
        info.action_mask = SBX_DIAG_PORT_ATTR_INIT_MASK | seen;
        if ((r = bcm_port_selective_get(unit, port, &info)) < 0) {
            cli_out("Error: %s Could not get port %s information: %s\n",
                    FUNCTION_NAME(), SOC_PORT_NAME(unit, port), bcm_errmsg(r));
            return (CMD_FAIL);
        }
        disp_port_info(unit, SOC_PORT_NAME(unit, port), port, &info, 
                       IS_ST_PORT(unit, port), seen);
#ifdef BCM_CALADAN3_SUPPORT
        if ((SOC_IS_CALADAN3(unit)) && (IS_IL_PORT(unit, port))) {
            int i = 0, nlanes =0, lane = 0, rv = SOC_E_NONE;
            soc_sbx_caladan3_il_get_lanes(unit, port, &nlanes);
            rv = soc_sbx_caladan3_il_get_lane_status(unit, port, &lane);
            if (SOC_SUCCESS(rv)) {
                cli_out("          ");
                for (i=0; i<nlanes; i++) {
                    cli_out("L%-d:%1s ", i, (lane & (1 << i)) ? "*" : "-");
                }
                cli_out("\n");
            }
        }
#endif
    }
    return CMD_OK;
}


char cmd_sbx_port_usage[] =
    "\n"
#ifdef COMPILER_STRING_CONST_LIMIT
    "    port <option> [args...]\n"
#else
    "    port <pbmp> [ENCap=IEEE|HIGIG|B5632|HIGIG2]\n"
    "                [AutoNeg=on|off] [ADVert=<portmode>]\n"
    "                [LinkScan=on|off|hw|sw] [SPeed=10|100|1000]\n"
    "                [FullDuplex=true|false] [TxPAUse=on|off]\n"
    "                [RxPAUse=on|off] [STationADdr=<macaddr>]\n"
    "                [LeaRN=<learnmode>] [DIScard=none|untag|all]\n"
    "                [VlanFilter=<value>] [PRIOrity=<0-7>]\n"
    "                [PortFilterMode=<value>]\n"
    "                [PHymaster=<Master|Slave|Auto|None>]\n"
    "                [Enable=<true|false>] [FrameMax=<value>]\n"
    "                [MDIX=Auto|ForcedAuto|ForcedNormal|ForcedXover]\n"
    "                [LoopBack=NONE|MAC|PHY]\n"
    "    - If only <ports> is specified, characteristics for that port are\n"
    "      displayed. <ports> is a standard port bitmap (see \"help pbmp\").\n"
    "      If AutoNeg is on, SPeed and DUPlex are the ADVERTISED MAX values.\n"
    "      If AutoNeg is off, SPeed and DUPlex are the FORCED values.\n"
    "      SPeed of zero indicates maximum speed.\n"
    "      LinkScan enables automatic scanning for link changes with updating\n"
    "      of MAC registers, and EPC_LINK (or equivalent)\n"
    "      PAUse enables send/receive of pause frames in full duplex mode.\n"
    "      <learnmode> is a numeric value controlling source lookup failure\n"
    "      packets; it may include bit 0 to enable hardware L2 learn, bit 1\n"
    "      to copy SLF packets to CPU, bit 2 to forward SLF packets.\n"
    "      VlanFilter drops input packets that not tagged with a valid VLAN\n"
    "      that contains the port. For XGS3, VlanFilter takes a value 0/1/2/3\n"
    "      where bit 0 turns on/off ingress filter and bit 1 turns on/off\n"
    "      egress filter.\n"
    "      PRIOrity sets the priority for untagged packets coming on this port.\n"
    "      PortFilterMode takes a value 0/1/2 for mode A/B/C (see register\n"
    "      manual).\n"
    "\n"
    "    port init   - Initialize port module.\n"
#endif
    ;

/*
 * Function:
 *     cmd_sbx_port
 * Purpose:
 *     Port configuration.
 * Parameters:
 *     unit - Device number
 *     args - Pointer to args
 * Returns:
 *     CMD_OK   - Success
 *     CMD_FAIL - Failure
 */
cmd_result_t
cmd_sbx_port(int unit, args_t *args)
{
    pbmp_t                  pbm, sbx_pbm_all;
    bcm_port_info_t         *info_all;
    static bcm_port_info_t  given;
    char                    *c;
    int                     rv;
    int                     cmd_rv = CMD_OK;
    soc_port_t              port;
    parse_table_t           pt;
    uint32                  seen = 0;
    uint32                  parsed = 0;
    char                    pfmt[SOC_PBMP_FMT_LEN];
    int                     attr_default = 0;




    if (!sh_check_attached(ARG_CMD(args), unit)) {
        return CMD_FAIL;
    }

    if ((c = ARG_GET(args)) == NULL) {
        return(CMD_USAGE);
    }

    if (!sal_strcasecmp(c, "init")) {
        if ((rv = bcm_port_init(unit)) < 0) {
            cli_out("%s: Error: Failed to initialize port module unit=%d (%s)\n",
                    ARG_CMD(args), unit, bcm_errmsg(rv));
            return (CMD_FAIL);
        }
        return CMD_OK;
    }

    info_all = sal_alloc(SOC_MAX_NUM_PORTS * sizeof(bcm_port_info_t),
                         "if_port");
    if (info_all == NULL) {
        cli_out("Insufficient memory.\n");
        return CMD_FAIL;
    }

    sal_memset(info_all, 0, SOC_MAX_NUM_PORTS * sizeof(bcm_port_info_t));

    if (parse_pbmp(unit, c, &pbm) < 0) {
        cli_out("%s: Error: unrecognized port bitmap: %s\n",
                ARG_CMD(args), c);
        sal_free(info_all);
        return CMD_FAIL;
    }

    BCM_PBMP_ASSIGN(sbx_pbm_all, PBMP_PORT_ALL(unit));
    BCM_PBMP_OR(sbx_pbm_all, PBMP_SCI_ALL(unit));
    BCM_PBMP_OR(sbx_pbm_all, PBMP_SFI_ALL(unit));
    BCM_PBMP_OR(sbx_pbm_all, PBMP_XG_ALL(unit));
    BCM_PBMP_OR(sbx_pbm_all, PBMP_SPI_SUBPORT_ALL(unit));
    BCM_PBMP_OR(sbx_pbm_all, PBMP_SPI_ALL(unit));

    BCM_PBMP_AND(pbm, sbx_pbm_all);

    if (BCM_PBMP_IS_NULL(pbm)) {
        cli_out("Ports unspecified\n");
        ARG_DISCARD(args);
        sal_free(info_all);
        return CMD_OK;
    }

    if (ARG_CNT(args) == 0) {
        attr_default = 1;
        seen = SBX_DIAG_PORT_ATTR_INIT_MASK;
    } else {
        /*
         * Otherwise, arguments are given.  Use them to determine which
         * properties need to be gotten/set.
         *
         * Probe and detach, hidden commands.
         */
        if (!sal_strcasecmp(_ARG_CUR(args), "detach")) {
            pbmp_t detached;
            bcm_port_detach(unit, pbm, &detached);
            cli_out("Detached port bitmap %s\n", SOC_PBMP_FMT(detached, pfmt));
            ARG_GET(args);
            sal_free(info_all);
            return CMD_OK;
        } else if ((!sal_strcasecmp(_ARG_CUR(args), "probe")) ||
                   (!sal_strcasecmp(_ARG_CUR(args), "attach"))) {
            pbmp_t probed;
            bcm_port_probe(unit, pbm, &probed);
            cli_out("Probed port bitmap %s\n", SOC_PBMP_FMT(probed, pfmt));
            ARG_GET(args);
            sal_free(info_all);
            return CMD_OK;
        }

        if (!sal_strcmp(_ARG_CUR(args), "=")) {
            /*
             * For "=" where the user is prompted to enter all the parameters,
             * use the parameters from the first selected port as the defaults.
             */
            if (ARG_CNT(args) != 1) {
                sal_free(info_all);
                return CMD_USAGE;
            }
            PBMP_ITER(pbm, port) {
                break;    /* Find first port in bitmap */
            }
            if ((rv = bcm_port_info_get(unit, port, &given)) < 0) {
                cli_out("%s: Error: Failed to get port info\n", ARG_CMD(args));
                sal_free(info_all);
                return CMD_FAIL;
            }
        }

        /*
         * Parse the arguments.  Determine which ones were actually given.
         */
        port_parse_setup(unit, &pt, &given);

        if (parse_arg_eq(args, &pt) < 0) {
            parse_arg_eq_done(&pt);
            sal_free(info_all);
            return(CMD_FAIL);
        }

        if (ARG_CNT(args) > 0) {
            cli_out("%s: Unknown argument %s\n", ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            sal_free(info_all);
            return(CMD_FAIL);
        }

        /*
         * Find out what parameters specified.  Record values specified.
         */
        port_parse_mask_get(&pt, &seen, &parsed);
        parse_arg_eq_done(&pt);
    }

    if (seen && parsed) {
        cli_out("%s: Cannot get and set "
                "port properties in one command\n",
                ARG_CMD(args));
        sal_free(info_all);
        return CMD_FAIL;
    } else if (seen) { /* Show selected information */
        cli_out("%s: Status (* indicates PHY link up)\n", ARG_CMD(args));
        /* Display the information by port type */
#undef  _call_pdi
#define _call_pdi(u, p, mp, s) \
        if (sbx_port_disp_iter(u, p, mp, s) != CMD_OK) { \
             sal_free(info_all); \
             return CMD_FAIL; \
        }

	if ( SOC_IS_SBX_SIRIUS(unit) ) {
	    if (attr_default) {
		_call_pdi(unit, pbm, PBMP_XG_ALL(unit),
			  seen | SBX_DIAG_HG_INTF_ATTR_MASK);
	    } else {
		_call_pdi(unit, pbm, PBMP_XG_ALL(unit),
			  SBX_DIAG_HG_INTF_ATTR_MASK);
	    }
	} else if (SOC_IS_CALADAN3(unit)) {
	    if (attr_default) {
	        _call_pdi(unit, pbm, PBMP_IL_ALL(unit),
			  seen | SBX_DIAG_E_PORT_ATTR_MASK);
	        _call_pdi(unit, pbm, PBMP_CE_ALL(unit),
			  seen | SBX_DIAG_E_PORT_ATTR_MASK);
	        _call_pdi(unit, pbm, PBMP_XE_ALL(unit),
			  seen | SBX_DIAG_E_PORT_ATTR_MASK);
		_call_pdi(unit, pbm, PBMP_HG_ALL(unit),
			  seen | SBX_DIAG_E_PORT_ATTR_MASK);
	        _call_pdi(unit, pbm, PBMP_GE_ALL(unit),
			  seen | SBX_DIAG_E_PORT_ATTR_MASK);
	    } else {
	        _call_pdi(unit, pbm, PBMP_IL_ALL(unit), seen);
	        _call_pdi(unit, pbm, PBMP_CE_ALL(unit), seen);
	        _call_pdi(unit, pbm, PBMP_HG_ALL(unit), seen);
		_call_pdi(unit, pbm, PBMP_XE_ALL(unit), seen);
		_call_pdi(unit, pbm, PBMP_GE_ALL(unit), seen);
	    }
	} else {
	    if (attr_default) {
	        _call_pdi(unit, pbm, PBMP_GE_ALL(unit),
			  seen | SBX_DIAG_E_PORT_ATTR_MASK);
		_call_pdi(unit, pbm, PBMP_XE_ALL(unit),
			  seen | SBX_DIAG_E_PORT_ATTR_MASK);
	    } else {
	        _call_pdi(unit, pbm, PBMP_GE_ALL(unit), seen);
		_call_pdi(unit, pbm, PBMP_XE_ALL(unit), seen);
	    }
	}

        _call_pdi(unit, pbm, PBMP_SFI_ALL(unit), seen);
        _call_pdi(unit, pbm, PBMP_SCI_ALL(unit), seen);
        _call_pdi(unit, pbm, PBMP_SPI_SUBPORT_ALL(unit), seen);
        _call_pdi(unit, pbm, PBMP_SPI_ALL(unit), seen);

#undef  _call_pdi
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

    PBMP_ITER(pbm, port) {

        info_all[port].action_mask = (SBX_DIAG_PORT_ATTR_INIT_MASK | parsed);

        if ((rv = bcm_port_selective_get(unit, port, &info_all[port])) < 0) {
            cli_out("%s: Error: Could not get port %s information: %s\n",
                    ARG_CMD(args), SOC_PORT_NAME(unit, port), bcm_errmsg(rv));
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

    PBMP_ITER(pbm, port) {

        if ((rv = bcm_port_speed_max(unit, port, &given.speed_max)) < 0) {
            cli_out("port parse: Error: Could not get port %s max speed: %s\n",
                    SOC_PORT_NAME(unit, port), bcm_errmsg(rv));
            continue;
        }

        if ((rv = bcm_port_ability_get(unit, port, &given.ability)) < 0) {
            cli_out("port parse: Error: Could not get port %s ability: %s\n",
                    SOC_PORT_NAME(unit, port), bcm_errmsg(rv));
            continue;
        }

        info_all[port].action_mask = parsed;
        if ((rv = port_parse_port_info_set(parsed,
                                           &given, &info_all[port])) < 0) {
            cli_out("%s: Error: Could not parse port %s info: %s\n",
                    ARG_CMD(args), SOC_PORT_NAME(unit, port), bcm_errmsg(rv));
            cmd_rv = CMD_FAIL;
            continue;
        }

        if (info_all[port].autoneg) {
            info_all[port].action_mask &= ~BCM_PORT_AN_ATTRS;
        }

        if ((rv = bcm_port_selective_set(unit, port, &info_all[port])) < 0) {
            cli_out("%s: Error: Could not set port %s information: %s\n",
                    ARG_CMD(args), SOC_PORT_NAME(unit, port), bcm_errmsg(rv));
            cmd_rv = CMD_FAIL;
            continue;
        }
    }

    sal_free(info_all);

    return(cmd_rv);
}


char cmd_sbx_port_stat_usage[] =
    "Display info about port status in table format.\n"
    "    Link scan modes:\n"
    "        SW = software\n"
    "        HW = hardware\n"
    "    Learn operations (source lookup failure control):\n"
    "        F = SLF packets are forwarded\n"
    "        C = SLF packets are sent to the CPU\n"
    "        A = SLF packets are learned in L2 table\n"
    "        D = SLF packets are discarded.\n"
    "    Pause:\n"
    "        TX = Switch will transmit pause packets\n"
    "        RX = Switch will obey pause packets\n";

/*
 * Function:
 *     cmd_sbx_port_stat
 * Purpose:
 *     Table display of port status information.
 * Parameters:
 *     unit - Device number
 *     args - Pointer to args
 * Returns:
 *     CMD_OK   - Success
 *     CMD_FAIL - Failure
 */
cmd_result_t
cmd_sbx_port_stat(int unit, args_t *args)
{
    pbmp_t           pbm, tmp_pbm;
    pbmp_t           rxaui_no_probe_ports;
    pbmp_t           channelized_no_probe_ports;
#ifdef BCM_CALADAN3_SUPPORT
    int              requires_phy_setup;
    int              is_channelized;
#endif
    bcm_port_info_t  *info_all;
    soc_port_t       port;
    int              rv;
    char             *c;

    if (!sh_check_attached(ARG_CMD(args), unit)) {
        return CMD_FAIL;
    }

    BCM_PBMP_CLEAR(rxaui_no_probe_ports);
    BCM_PBMP_CLEAR(channelized_no_probe_ports);
    BCM_PBMP_ASSIGN(tmp_pbm, PBMP_PORT_ALL(unit));
    BCM_PBMP_OR(tmp_pbm, PBMP_SPI_SUBPORT_ALL(unit));
    BCM_PBMP_OR(tmp_pbm, PBMP_SPI_ALL(unit));
    BCM_PBMP_OR(tmp_pbm, PBMP_SCI_ALL(unit));
    BCM_PBMP_OR(tmp_pbm, PBMP_SFI_ALL(unit));
    BCM_PBMP_OR(tmp_pbm, PBMP_HG_ALL(unit));
    if ((c = ARG_GET(args)) == NULL) {
        BCM_PBMP_ASSIGN(pbm, tmp_pbm);
    } else if (parse_pbmp(unit, c, &pbm) < 0) {
        cli_out("%s: Error: unrecognized port bitmap: %s\n",
                ARG_CMD(args), c);
        return CMD_FAIL;
    }
    BCM_PBMP_AND(pbm, tmp_pbm);

    if (BCM_PBMP_IS_NULL(pbm)) {
        cli_out("No ports specified.\n");
        return CMD_OK;
    }

    info_all = sal_alloc(SOC_MAX_NUM_PORTS * sizeof(bcm_port_info_t),
                         "if_port_stat");
    if (info_all == NULL) {
        cli_out("Insufficient memory.\n");
        return CMD_FAIL;
    }
    
    sal_memset(info_all, 0, SOC_MAX_NUM_PORTS * sizeof(bcm_port_info_t));

    PBMP_ITER(pbm, port) {

        if (IS_E_PORT(unit, port) || IS_HG_PORT(unit, port) ||
            IS_HG_SUBPORT_PORT(unit, port) || IS_IL_PORT(unit, port)) {
	  if (SOC_IS_SBX_SIRIUS(unit)) {
	    if (IS_HG_SUBPORT_PORT(unit, port)) {
                
	        info_all[port].action_mask = SBX_DIAG_HG_SUBPORT_INTF_ATTR_MASK;
                
            }else{
	        info_all[port].action_mask = SBX_DIAG_HG_INTF_ATTR_MASK;
            }
	  } else {
	    info_all[port].action_mask = SBX_DIAG_E_PORT_ATTR_MASK;
	  }
        } else if (IS_REQ_PORT(unit, port)) {
          info_all[port].action_mask = SBX_DIAG_PORT_ATTR_INIT_MASK;
	} else {
          info_all[port].action_mask = SBX_DIAG_PORT_ATTR_INIT_MASK;
          info_all[port].action_mask |= BCM_PORT_ATTR_SPEED_MASK;
          /* info_all[port].action_mask |= BCM_PORT_ATTR_INTERFACE_MASK;  */
        }

        /* remove unsupported flags for G2XX (custom) ucode */
        if (SOC_IS_SBX_G2XX(unit)) {
            info_all[port].action_mask &= ~SBX_DIAG_G2XX_UNSUPPORTED_ATTR_MASK;
        }

#ifdef BCM_CALADAN3_SUPPORT
        if (SOC_IS_CALADAN3(unit)) {
            /* Remove unsupported odd numbered rxaui xe ports - */
            /* this is required because we support the 12x10G card type */
            /* but for rxaui, 2 lanes are required per 10G port so there are   */
            /* 6 even ports which are supported xe0, xe2, xe4, xe6, xe8, xe10 */
            /* - remove others       */ 
            if (soc_property_port_get(unit, port, spn_SERDES_RXAUI_MODE, 0) && ((port % 2) != 0)) {
                SOC_PBMP_PORT_ADD(rxaui_no_probe_ports, port);
                continue;
            }

            /* Remove channelized subports from port status command */
            rv = soc_sbx_caladan3_port_is_channelized_subport(unit, port, 
                                                              &is_channelized, 
                                                              &requires_phy_setup);
            if (BCM_FAILURE(rv)) {
                cli_out("Error getting port channelization info port(%d) rv(%d)\n", port, rv);
                sal_free(info_all);
                return (CMD_FAIL);
            }

            if (is_channelized) {
                SOC_PBMP_PORT_ADD(channelized_no_probe_ports, port);
                continue;
            }
            info_all[port].action_mask &= ~(BCM_PORT_ATTR_UNTAG_PRI_MASK);
        }
#endif

        if ((rv = bcm_port_selective_get(unit, port, &info_all[port])) < 0) {
            cli_out("%s: Error: Could not get port %s information: %s\n",
                    ARG_CMD(args), SOC_PORT_NAME(unit, port), bcm_errmsg(rv));
            sal_free(info_all);
            return (CMD_FAIL);
        }
    }

    if (SOC_IS_CALADAN3(unit)) {
        BCM_PBMP_REMOVE(pbm, rxaui_no_probe_ports);
        BCM_PBMP_REMOVE(pbm, channelized_no_probe_ports);
    }

    brief_port_info_header(unit);

#undef  _call_bpi
#define _call_bpi(pbm, pbm_mask) \
    tmp_pbm = pbm_mask; \
    BCM_PBMP_AND(tmp_pbm, pbm); \
    PBMP_ITER(tmp_pbm, port) { \
        brief_port_info(unit, port, &info_all[port], \
                        info_all[port].action_mask); \
    }

    _call_bpi(pbm, PBMP_IL_ALL(unit));
    _call_bpi(pbm, PBMP_CE_ALL(unit));
    _call_bpi(pbm, PBMP_XE_ALL(unit));
    _call_bpi(pbm, PBMP_GE_ALL(unit));
    _call_bpi(pbm, PBMP_HG_ALL(unit));
    /* only report sci_sfi port once */
    BCM_PBMP_ASSIGN(tmp_pbm, PBMP_SFI_ALL(unit));
    BCM_PBMP_REMOVE(tmp_pbm, PBMP_SCI_ALL(unit));
    _call_bpi(pbm, tmp_pbm);
    _call_bpi(pbm, PBMP_SCI_ALL(unit));
    _call_bpi(pbm, PBMP_SPI_SUBPORT_ALL(unit));
    _call_bpi(pbm, PBMP_SPI_ALL(unit));
#undef  _call_bpi

    sal_free(info_all);

    return CMD_OK;
}

/*
 * PBMP
 */
char cmd_sbx_pbmp_usage[] =
    "Parameters: <pbmp>\n"
#ifndef COMPILER_STRING_CONST_LIMIT
    "\tConverts a pbmp string into a hardware port bitmap.  A pbmp string\n\t"
    "is a single port, or a group of ports specified in a list using ','\n\t"
    "to separate them and '-' for ranges, e.g. 1-8,25,cpu.  Ports may be\n\t"
    "specified using the one-based port number (1-29) or port type and\n\t"
    "zero-based number (fe0-fe23,ge0-ge7).  'cpu' is the CPU port,\n\t"
    "'fe' is all FE ports, 'ge' is all GE ports, 'e' is all ethernet\n\t"
    "ports, 'all' is all ports, and 'none' is no ports (0x0).\n\t"
    "A '~' may be used to exclude port previously given (e.g. e,~fe19)\n\t"
    "Acceptable strings and values also depend on the chip being used.\n\t"
    "A pbmp may also be given as a raw hex (0x) number, e.g. 0xbffffff.\n"
#endif
    ;

cmd_result_t
cmd_sbx_pbmp(int unit, args_t *a)
{
    pbmp_t              pbmp;
    char                *c;
    soc_port_t          port;
    char                pbmp_str[FORMAT_PBMP_MAX];
    char                pfmt[SOC_PBMP_FMT_LEN];

    COMPILER_REFERENCE(unit);

    c = ARG_GET(a);

    if (!c) {   
        cli_out("Current bitmaps:\n");

#if 0
	cli_out("     FE   ==> %s\n",
                SOC_PBMP_FMT(PBMP_FE_ALL(unit), pfmt));
#endif

        if (SOC_IS_SBX_FE(unit)) {
            cli_out("     GE             ==> %s\n",
                    SOC_PBMP_FMT(PBMP_GE_ALL(unit), pfmt));
            cli_out("     XE             ==> %s\n",
                    SOC_PBMP_FMT(PBMP_XE_ALL(unit), pfmt));
            cli_out("     E              ==> %s\n",
                    SOC_PBMP_FMT(PBMP_E_ALL(unit), pfmt));
        }

#if 0
	cli_out("     HG   ==> %s\n",
                SOC_PBMP_FMT(PBMP_HG_ALL(unit), pfmt));
        cli_out("     HL   ==> %s\n",
                SOC_PBMP_FMT(PBMP_HL_ALL(unit), pfmt));
        cli_out("     ST   ==> %s\n",
                SOC_PBMP_FMT(PBMP_ST_ALL(unit), pfmt));
        cli_out("     GX   ==> %s\n",
                SOC_PBMP_FMT(PBMP_GX_ALL(unit), pfmt));
#endif
#ifdef BCM_CALADAN3_SUPPORT
        if (SOC_IS_CALADAN3(unit)) {
        cli_out("     CE             ==> %s\n",
                SOC_PBMP_FMT(PBMP_CE_ALL(unit), pfmt));
        cli_out("     HG             ==> %s\n",
                SOC_PBMP_FMT(PBMP_HG_ALL(unit), pfmt));
        cli_out("     IL             ==> %s\n",
                SOC_PBMP_FMT(PBMP_IL_ALL(unit), pfmt));
        cli_out("     HG2            ==> %s\n",
                SOC_PBMP_FMT(SOC_HG2_PBM(unit), pfmt));
        cli_out("     XL             ==> %s\n",
                SOC_PBMP_FMT(PBMP_XL_ALL(unit), pfmt));
        cli_out("     XT             ==> %s\n",
                SOC_PBMP_FMT(PBMP_XT_ALL(unit), pfmt));
        cli_out("     CL             ==> %s\n",
                SOC_PBMP_FMT(PBMP_CL_ALL(unit), pfmt));
        } else 
#endif
        if (SOC_IS_SBX_FE(unit) || ( SOC_IS_SBX_QE(unit) && !SOC_IS_SBX_SIRIUS(unit)) ) {
            cli_out("     SPI            ==> %s\n",
                    SOC_PBMP_FMT(PBMP_SPI_ALL(unit), pfmt));
            cli_out("     SPI_SUBPORT    ==> %s\n",
                    SOC_PBMP_FMT(PBMP_SPI_SUBPORT_ALL(unit), pfmt));
        } else if (SOC_IS_SBX_SIRIUS(unit)) {
	    if (NUM_HG_PORT(unit)) {
	        cli_out("     HG             ==> %s\n",
                        SOC_PBMP_FMT(PBMP_HG_ALL(unit), pfmt));
	    }
	    if (NUM_XE_PORT(unit)) {
		/* For emulator bringup effort only, should have no
		 * XE port configured in normal case
		 */
		cli_out("     XE             ==> %s\n",
                        SOC_PBMP_FMT(PBMP_XE_ALL(unit), pfmt));
		cli_out("     E              ==> %s\n",
                        SOC_PBMP_FMT(PBMP_E_ALL(unit), pfmt));
	    }
	    if (NUM_GE_PORT(unit)) {
		/* For emulator bringup effort only, should have no
		 * GE port configured in normal case
		 */
		cli_out("     GE             ==> %s\n",
                        SOC_PBMP_FMT(PBMP_GE_ALL(unit), pfmt));
		cli_out("     E              ==> %s\n",
                        SOC_PBMP_FMT(PBMP_E_ALL(unit), pfmt));
	    }
	    if (NUM_REQ_PORT(unit)) {
		cli_out("     REQ            ==> %s\n",
                        SOC_PBMP_FMT(PBMP_REQ_ALL(unit), pfmt));
	    }
       	}

        if (SOC_IS_SBX_QE(unit) || SOC_IS_SBX_BME(unit)) {
            cli_out("     SFI            ==> %s\n",
                    SOC_PBMP_FMT(PBMP_SFI_ALL(unit), pfmt));
            cli_out("     SCI            ==> %s\n",
                    SOC_PBMP_FMT(PBMP_SCI_ALL(unit), pfmt));
        }

        cli_out("     PORT           ==> %s\n",
                SOC_PBMP_FMT(PBMP_PORT_ALL(unit), pfmt));
        cli_out("     CPU            ==> %s\n",
                SOC_PBMP_FMT(PBMP_CMIC(unit), pfmt));
        cli_out("     ALL            ==> %s\n",
                SOC_PBMP_FMT(PBMP_ALL(unit), pfmt));
        return CMD_OK;
    }

    if (sal_strcasecmp(c, "port") == 0) {
        if ((c = ARG_GET(a)) == NULL) {
            cli_out("ERROR: missing port string\n");
            return CMD_FAIL;
        }
        if (parse_port(unit, c, &port) < 0) {
            cli_out("%s: Invalid port string: %s (MAX allowable %d)\n", ARG_CMD(a), c, SOC_MAX_NUM_PORTS);
            return CMD_FAIL;
        }
        cli_out("    port %s ==> %s (%d)\n",
                c, SOC_PORT_NAME(unit, port), port);
        return CMD_OK;
    }

    if (parse_pbmp(unit, c, &pbmp) < 0) {
        cli_out("%s: Invalid pbmp string (%s); use 'pbmp ?' for more info.\n",
                ARG_CMD(a), c);
        return CMD_FAIL;
    }

    format_pbmp(unit, pbmp_str, sizeof (pbmp_str), pbmp);

    cli_out("    %s ==> %s\n", SOC_PBMP_FMT(pbmp, pfmt), pbmp_str);

    return CMD_OK;
}

/*
 * Port Rate
 */
char cmd_sbx_port_rate_usage[] =
    "portrate port <port#> [egress <rates_kbits/sec> <burst_kbits>]\n" ;

cmd_result_t
cmd_sbx_port_rate(int unit, args_t *a)
{
    char                *c;
    soc_port_t          port;
    uint32              kbits_sec;
    uint32              kbits_burst;
    int                     rv;
    int                 operation = 0;

#define SHOW    1
#define INGRESS 2
#define EGRESS  4

#define QE2000_PORT_COUNT 48


    COMPILER_REFERENCE(unit);

    c = ARG_GET(a);

    if (!c) {   
        cli_out(" %s",cmd_sbx_port_rate_usage);
            return CMD_FAIL;
    } 

    if (sal_strcasecmp(c, "all") == 0) {

        int i;
        for ( i = 0; i < QE2000_PORT_COUNT; i++) {
            if ((rv = bcm_port_rate_egress_get(unit, i, &kbits_sec, &kbits_burst)) < 0) {
                cli_out("%s: Error: Could not get port_rate_egress %s information: %s\n",
                        ARG_CMD(a), SOC_PORT_NAME(unit, i), bcm_errmsg(rv));
                /* sal_free(info_all); */
                return (CMD_FAIL);
            }
            cli_out("Port %d %s. Rate: %d kb/sec. Burst: %d kbits (0x%x 0x%x)\n", i,
                    SOC_PORT_NAME( unit, i ), kbits_sec, kbits_burst, kbits_sec, kbits_burst);
        }
        return CMD_OK;
    }


    if (sal_strcasecmp(c, "port") == 0) {
        if ((c = ARG_GET(a)) == NULL) {
            cli_out("ERROR: missing port string\n");
            return CMD_FAIL;
        }
        if (parse_port(unit, c, &port) < 0) {
            cli_out("%s: Invalid port string: %s\n", ARG_CMD(a), c);
            return CMD_FAIL;
        }
        cli_out("    port %s ==> %s (%d)\n",
                c, SOC_PORT_NAME(unit, port), port);
     /*   return CMD_OK;  */
    }  else {
        /* Other command options would go here */
        cli_out(" command requires \"port <port#>\" argument\n");
        return CMD_FAIL;

    }

  /* Ingress, egress or show both */
    if ((c = ARG_GET(a)) == NULL) {
        cli_out("Display: ");
        operation = (SHOW );
    } else if (!sal_strncasecmp(c, "ingress", strlen(c))) {
        operation = INGRESS;
        cli_out("No ingress shaper support at this time\n");
        return CMD_FAIL;
    } else if (!sal_strncasecmp(c, "egress", strlen(c))) {
        operation = EGRESS;
    } else {
        cli_out("%s: Error: unrecognized port rate type: %s\n",
                ARG_CMD(a), c);
        cli_out("include \"egress\" before rate values kbits_rate kbits_burst\n");
        cli_out("e.g. portrate port 1 egress 10 100\n");
        return CMD_FAIL;
    }

    /* Set or get */
    if ((c = ARG_GET(a)) != NULL) {
        kbits_sec = parse_integer(c);
        if ( kbits_sec == 0 ) {
            cli_out(" kb_sec is zero. Disable shaper\n");
            kbits_burst = 0;
        } else {
            if ((c = ARG_GET(a)) != NULL) {
                kbits_burst = parse_integer(c);
            } else {
                cli_out("%s: Error: missing port burst size\n",
                        ARG_CMD(a));
                return CMD_FAIL;
            }
        }
    } else {
        operation |= SHOW;
    }

                                                                  

     if (operation == SHOW) {

        if ((rv = bcm_port_rate_egress_get(unit, port, &kbits_sec, &kbits_burst)) < 0) {
            cli_out("%s: Error: Could not get port_rate_egress %s information: %s\n",
                    ARG_CMD(a), SOC_PORT_NAME(unit, port), bcm_errmsg(rv));
            /* sal_free(info_all); */
            return (CMD_FAIL);
        }
        cli_out("Port %d %s. Rate: %d kb/sec. Burst: %d kbits (0x%x 0x%x)\n", port,
                SOC_PORT_NAME( unit, port ), kbits_sec, kbits_burst, kbits_sec, kbits_burst);
    } else if (operation == INGRESS) {
	    /* coverity[dead_error_begin] */
            cli_out("Error: ingress not supported for QE2000\n");
            return (CMD_FAIL);

    } else if (operation == EGRESS) {
        cli_out("Set: ");
        /* cli_out("bcm_port_rate_egress_set 0x%x 0x%x\n", kbits_sec, kbits_burst); */
        if ((rv = bcm_port_rate_egress_set(unit, port, kbits_sec, kbits_burst)) < 0) {
            cli_out("%s: Error: Could not set port_rate_egress %s information: %s\n",
                    ARG_CMD(a), SOC_PORT_NAME(unit, port), bcm_errmsg(rv));
            /* sal_free(info_all); */
            return (CMD_FAIL);
        }

    } else {
            cli_out("Error: Unknown command mode for QE2000\n");
            return (CMD_FAIL);
    }
 
    return CMD_OK;
}

