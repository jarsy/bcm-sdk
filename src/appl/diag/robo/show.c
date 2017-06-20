/*
 * $Id: show.c,v 1.19 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <appl/diag/system.h>

#include <bcm/stat.h>
#include <bcm/error.h>
#include <bcm/init.h>

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/robo/mcm/driver.h>
#include <soc/drv_if.h>


#include <sal/appl/pci.h>

#if defined(VXWORKS)
#include <netShow.h>
#include <muxLib.h>
#if defined (VXWORKS_NETWORK_STACK_6_5)
#include <net/utils/netstat.h>
#endif
#endif /* VXWORKS */


/*
 * Clear something for a user.
 */

cmd_result_t
cmd_robo_clear(int unit, args_t *a)
{
    char *parm = ARG_GET(a);
    soc_mem_t mem;
    pbmp_t pbmp;
    int r, copyno;
    uint32 table_name;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if (!parm) {
        return CMD_USAGE;
    }

    if (!sal_strcasecmp(parm, "counters") ||
        !sal_strcasecmp(parm, "c")) {
        uint64 val;

        if ((parm = ARG_GET(a)) == NULL) {
            pbmp = PBMP_ALL(unit);
        } else if (parse_pbmp(unit, parm, &pbmp) < 0) {
            cli_out("%s: Invalid port bitmap: %s\n", ARG_CMD(a), parm);
            return CMD_FAIL;
        }
        if ((r = soc_robo_counter_set32_by_port(unit, pbmp, 0)) < 0) {
            cli_out("ERROR: Clear counters failed: %s\n", soc_errmsg(r));
            return CMD_FAIL;
        }

        /*
         * Clear the diagnostics' copy of the counters so 'show
         * counters' knows they're clear.
         */

        COMPILER_64_ZERO(val);

        robo_counter_val_set_by_port(unit, pbmp, val);

        return CMD_OK;
    }

    if (!sal_strcasecmp(parm, "stats")) {
        int rv = CMD_OK;
        soc_port_t port;

        if ((parm = ARG_GET(a)) == NULL) {
            pbmp = PBMP_ALL(unit);
        } else if (parse_pbmp(unit, parm, &pbmp) < 0) {
            cli_out("%s: Invalid port bitmap: %s\n", ARG_CMD(a), parm);
            return CMD_FAIL;
        }

        PBMP_ITER(pbmp, port) {
            int rv;

            if ((rv = bcm_stat_clear(unit, port)) != BCM_E_NONE) {
                cli_out("%s: Unit %d Port %d failed to clear stats: %s\n",
                        ARG_CMD(a), unit, port, bcm_errmsg(rv));
                rv = CMD_FAIL;
            }
        }
        return(rv);
    }

    if (!sal_strcasecmp(parm, "dev")) {
        int rv;
        rv = bcm_clear(unit);
        if (rv < 0) {
            cli_out("%s ERROR: Unit %d.  bcm_clear returned %d: %s\n",
                    ARG_CMD(a), unit, rv, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;
    }
    
    do {
        if (parse_memory_name(unit, &mem, parm, &copyno, 0) < 0) {
            cli_out("ERROR: unknown table \"%s\"\n", parm);
            return CMD_FAIL;
        }

        switch(mem) {
    	    case INDEX(GEN_MEMORYm):
    		    table_name = DRV_MEM_GEN;
        		break;
        	case INDEX(L2_ARLm):
        		table_name = DRV_MEM_ARL;
        		break;
        	case INDEX(MARL_PBMPm):
        		table_name = DRV_MEM_MCAST;
        		break;
        	case INDEX(MSPT_TABm):
        		table_name = DRV_MEM_MSTP;
        		break;
        	case INDEX(VLAN_1Qm):
        		table_name = DRV_MEM_VLAN;
        		break;
        	case INDEX(FLOW2VLANm):
        		table_name = DRV_MEM_FLOWVLAN;
        		break;
        	case INDEX(L2_ARL_SWm):
        		table_name = DRV_MEM_ARL;
        		break;
        	case INDEX(L2_MARL_SWm):
        		table_name = DRV_MEM_MARL;
        		break;
        	case INDEX(MAC2VLANm):
        		table_name = DRV_MEM_MACVLAN;
        		break;
        	case INDEX(PROTOCOL2VLANm):
        		table_name = DRV_MEM_PROTOCOLVLAN;
        		break;
        	case INDEX(VLAN2VLANm):
        		table_name = DRV_MEM_VLANVLAN;
        		break;
    		default:
    			cli_out("Unsupport memory table.\n");
    			return -1;
        }

	    if ((r = (DRV_SERVICES(unit)->mem_clear)(unit, table_name)) < 0) {
            cli_out("ERROR: clear table %s failed: %s\n",
                    SOC_ROBO_MEM_UFNAME(unit, mem), soc_errmsg(r));
            return CMD_FAIL;
        }
    } while ((parm = ARG_GET(a)) != NULL);

    return CMD_OK;
}


/*
 * Show something to a user.
 */

/*
 * *** ORDER ***
 *
 * The order of these tables must match the switch statements below.
 */
static parse_key_t show_arg[] = {
    "Pci",                  /* 0 */
    "Counters",             /* 1 */
    "Errors",               /* 2 */
    "Interrupts",           /* 3 */
    "Chips",                /* 4 */
    "Statistics",           /* 5 */
    "MIB",                  /* 6 */
};

static int show_arg_cnt = PARSE_ENTRIES(show_arg);

static parse_key_t show_ctr_arg[] = {
    "Changed",              /* 0 */
    "Same",                 /* 1 */
    "Zero",                 /* 2 */
    "NonZero",              /* 3 */
    "Hex",                  /* 4 */
    "Raw",                  /* 5 */
    "All",                  /* 6 */
    "ErDisc",               /* 7 */
};

static int show_ctr_arg_cnt = PARSE_ENTRIES(show_ctr_arg);

/* extern definition is for bcm_stat_name() is dispatchable */

cmd_result_t
cmd_robo_show(int unit, args_t *a)
{
    soc_control_t *soc = SOC_CONTROL(unit);
    pbmp_t pbmp;
    const parse_key_t *cmd;
    char *c;
    int flags;
    soc_regaddrlist_t alist;
    soc_reg_t ctr_reg;
    soc_port_t port;

    if (!(c = ARG_GET(a))) {   /* Nothing to do */
        return(CMD_USAGE);      /* Print usage line */
    }
#if defined (VXWORKS) && defined(KEYSTONE)
#if defined (VXWORKS_NETWORK_STACK_6_5)
    if (!sal_strcasecmp(c, "ip")) {
        netstat("-p ip -f inet");
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "icmp")) {
        netstat("-p icmp -f inet");
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "arp")) {
        arpShow();
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "udp")) {
        netstat("-p udp -f inet");
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "tcp")) {
        netstat("-p tcp -f inet");
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "mux")) {
        muxShow(NULL, 0);
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "routes")) {
        netstat("-a -r");
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "hosts")){
        hostShow();
        return CMD_OK;
    }
#else /* !VXWORKS_NETWORK_STACK_6_5 */
    if (!sal_strcasecmp(c, "ip")) {
        ipstatShow(FALSE);
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "icmp")) {
        icmpstatShow();
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "arp")) {
        arpShow();
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "udp")) {
        udpstatShow();
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "tcp")) {
        tcpstatShow();
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "mux")) {
#if defined(JUPITER) || defined(IDTRP334) || defined(GENERICPC)
        cli_out("muxShow not available on this BSP\n");
#else
        muxShow(NULL, 0);
#endif
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "routes")) {
        routeShow();
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "hosts")){
        hostShow();
        return CMD_OK;
    }
#endif /* VXWORKS_NETWORK_STACK_6_5 */
#endif
    
#if defined(BROADCOM_DEBUG)
    if (!sal_strcasecmp(c, "feature") || !sal_strcasecmp(c, "features")) {
        int		all;
        soc_feature_t	f;

        c = ARG_GET(a);
        if (c == NULL) {
            all = 0;
        } else {
            all = 1;
        }
        cli_out("Unit %d features:\n", unit);

        for (f = 0; f < soc_feature_count; f++) {
            if (soc_feature(unit, f)) {
                cli_out("\t%s\n", soc_feature_name[f]);
            } else if (all) {
                cli_out("\t[%s]\n", soc_feature_name[f]);
            }
        }
        return CMD_OK;
    }

    if (!sal_strcasecmp(c, "param") || !sal_strcasecmp(c, "params")) {
        if (!(c = ARG_GET(a))) {
            /* Current unit */
            soc_robo_chip_dump(unit, SOC_DRIVER(unit));
        } else {
            int chip, pcidev;
            chip = sal_ctoi(c, 0);
            if (chip >= 0x5600 && chip <= 0x56ff) {
                /* search for driver for pci device id */
                pcidev = chip;
                for (chip = 0; chip < SOC_ROBO_NUM_SUPPORTED_CHIPS; chip++) {
                    if (pcidev == soc_robo_base_driver_table[chip]->pci_device) {
                        soc_robo_chip_dump(-1, soc_robo_base_driver_table[chip]);
                        return CMD_OK;
                    }
                }
                cli_out("Chip device %x not found\n", pcidev);
                return CMD_FAIL;
            }
            /* specific chip requested */
            if (chip >= SOC_ROBO_NUM_SUPPORTED_CHIPS) {
                cli_out("Bad chip parameter:  %d.  Max is %d\n",
                        chip, SOC_ROBO_NUM_SUPPORTED_CHIPS);
            } else if (!SOC_ROBO_DRIVER_ACTIVE(chip)) {
                cli_out("Chip %d is not supported.\n", chip);
            } else {
                soc_robo_chip_dump(-1, soc_robo_base_driver_table[chip]);
            }
        }
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "unit") || !sal_strcasecmp(c, "units")) {
        int this_unit;
        soc_control_t *usoc;

        c = ARG_GET(a);
        if (c != NULL) {	/* specific unit */
            this_unit = sal_ctoi(c, 0);
            if (!SOC_UNIT_VALID(this_unit)) {
                cli_out("Unit %d is not valid\n", this_unit);
                return CMD_FAIL;
            }
            usoc = SOC_CONTROL(this_unit);
            if (!(usoc->soc_flags & SOC_F_ATTACHED)) {
                cli_out("Unit %d (detached)\n", this_unit);
                return CMD_OK;
            }
            cli_out("Unit %d chip %s%s\n",
                    this_unit,
                    soc_dev_name(this_unit),
                    this_unit == unit ? " (current)" : "");
            soc_robo_chip_dump(this_unit, SOC_DRIVER(this_unit));
        } else {		/* all units */
            for (this_unit = 0; this_unit < soc_ndev; this_unit++) {
                if (!SOC_UNIT_VALID(SOC_NDEV_IDX2DEV(this_unit))) {
                    continue;
                }
                usoc = SOC_CONTROL(SOC_NDEV_IDX2DEV(this_unit));
                cli_out("Unit %d chip %s%s\n",
                        SOC_NDEV_IDX2DEV(this_unit),
                        soc_dev_name(SOC_NDEV_IDX2DEV(this_unit)),
                        SOC_NDEV_IDX2DEV(this_unit) == unit ? " (current)" : "");
            }
        }
        return CMD_OK;
    }

#ifdef  BCM_TB_SUPPORT
#if defined(BCM_53280_B0)
    if (!sal_strcasecmp(c, "vm") || !sal_strcasecmp(c, "VlanMapping")) {
        int rv = BCM_E_NONE;
        uint32 reg_value = 0;
        uint32 fld_ivm = 0;
        uint32 fld_evm = 0;
        uint32 hit = 0;

        if (SOC_IS_TBX(unit) && !SOC_IS_TB_AX(unit)) {
            rv = REG_READ_IVM_EVM_HIT_ENTRYr(unit, &reg_value);
            if (rv < 0) {
                cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }
            soc_IVM_EVM_HIT_ENTRYr_field_get(unit, &reg_value, IVM_HIT_ENTRYf, &fld_ivm);
            soc_IVM_EVM_HIT_ENTRYr_field_get(unit, &reg_value, EVM_HIT_ENTRYf, &fld_evm);
            cli_out("Vlan Mapping last hit entries:\n");
            hit = fld_ivm & 0x8000;
            if (!hit) {
                cli_out("IVM is not hit\n");
            } else {
                cli_out("IVM hit entry id #%d\n", fld_ivm & 0x7fff);
            }
            hit = fld_evm & 0x8000;
            if (!hit) {
                cli_out("EVM is not hit\n");
            } else {
                cli_out("EVM hit entry id #%d\n", fld_evm & 0x7fff);
            }
        } else {
            rv = BCM_E_UNAVAIL;
            cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }

        return CMD_OK;
    }
#endif /* defined(BCM_53280_B0) */
#endif /* BCM_TB_SUPPORT */

#endif /* BROADCOM_DEBUG */

    cmd = parse_lookup(c, show_arg, sizeof(show_arg[0]), show_arg_cnt);
    if (!cmd) {
        cli_out("%s: Error: Invalid option %s\n", ARG_CMD(a), c);
        return(CMD_FAIL);
    }

    switch(cmd - show_arg) {
        case 0:     /* PCI */
        	/* pci related functions not available yet. */
            /* 
            pci_print_all();
            */
            break;
        case 1:     /* Counters */
            flags = 0;
    
            while ((c = ARG_GET(a)) != NULL) {
                cmd = parse_lookup(c,
                       show_ctr_arg, sizeof (show_ctr_arg[0]),
                       show_ctr_arg_cnt);
    
                switch (cmd - show_ctr_arg) {
                    case 0:
                        flags |= SHOW_CTR_CHANGED;
                        break;
                    case 1:
                        flags |= SHOW_CTR_SAME;
                        break;
                    case 2:
                        flags |= SHOW_CTR_Z;
                        break;
                    case 3:
                        flags |= SHOW_CTR_NZ;
                        break;
                    case 4:
                        flags |= SHOW_CTR_HEX;
                        break;
                    case 5:
                        flags |= SHOW_CTR_RAW;
                        break;
                    case 6:
                        flags |= (SHOW_CTR_CHANGED | SHOW_CTR_SAME |
                              SHOW_CTR_Z | SHOW_CTR_NZ);
                        break;
                    case 7:
                        flags |= SHOW_CTR_ED;
                        break;
                    default:
                        goto break_for;
                }
            }
            break_for:
    
            /*
             * Supply defaults
             */
        
            if ((flags & (SHOW_CTR_CHANGED | SHOW_CTR_SAME)) == 0) {
                flags |= SHOW_CTR_CHANGED;
            }
        
            if ((flags & (SHOW_CTR_Z | SHOW_CTR_NZ)) == 0) {
                flags |= SHOW_CTR_NZ;
            }
    
            if (c == NULL) {
                ctr_reg = INDEX(INVALID_Rr);     /* All registers */
                pbmp = PBMP_ALL(unit);      /* All ports */
            } else if (parse_pbmp(unit, c, &pbmp) >= 0) {
                ctr_reg = INDEX(INVALID_Rr);     /* All registers, selected ports */
            } else {
                int i;
        
                if (soc_robo_regaddrlist_alloc(&alist) < 0) {
                    cli_out("Could not allocate address list.  Memory error.\n");
                    return CMD_FAIL;
                }
        
                if (parse_symbolic_reference(unit, &alist, c) < 0) {
                    cli_out("Syntax error parsing \"%s\"\n", c);
                    soc_robo_regaddrlist_free(&alist);
                    return CMD_FAIL;
                }
        
                ctr_reg = alist.ainfo[0].reg;
        
                if (!SOC_REG_IS_COUNTER(unit, ctr_reg)) {
                    cli_out(\
                            "%s: Register is not a counter: %s\n", \
                            ARG_CMD(a), c);
                    soc_robo_regaddrlist_free(&alist);
                    return(CMD_FAIL);
                }
        
                BCM_PBMP_CLEAR(pbmp);
        
                for (i = 0; i < alist.count; i++) {
                    BCM_PBMP_PORT_ADD(pbmp, alist.ainfo[i].port);
                }
                soc_robo_regaddrlist_free(&alist);
            }
        
            BCM_PBMP_AND(pbmp, PBMP_ALL(unit));
            robo_do_show_counters(unit, ctr_reg, pbmp, flags);
            break;
        case 2:     /* Errors */
            cli_out("%s: Errors: MII(%d)\n",
                    ARG_CMD(a), soc->stat.err_mii_tmo);
            break;
        case 3:     /* Interrupts */
            break;
        case 4:     /* Chips */
            cli_out("Known chips:\n");
            soc_cm_display_known_devices();
            break;
        case 5:     /* Statistics */
        case 6:     /* MIB */
            flags = 0;
            if ((c = ARG_GET(a)) != NULL) {     /* Ports specified? */
                if (0 != parse_pbmp(unit, c, &pbmp)) {
                    cli_out("%s: Invalid ports: %s\n", ARG_CMD(a), c);
                    return(CMD_FAIL);
                }
                if ((c = ARG_GET(a)) != NULL && sal_strcasecmp(c, "all") == 0) {
                    flags = 1;
                }
            } else {
                pbmp = PBMP_ALL(unit);
            }
        
            PBMP_ITER(pbmp, port) {
                bcm_stat_val_t s;
                char *sname;
                char *_stat_names[] = BCM_STAT_NAME_INITIALIZER;
                int rv;
                uint64 val;
        
                cli_out("%s: Statistics for Unit %d port %s\n",
                        ARG_CMD(a), unit, SOC_PORT_NAME(unit, port));

                bcm_stat_sync(unit);

                for (s = 0; s < snmpValCount; s++) {
                    sname = _stat_names[s];
                    if (!sname) {
                        continue;
                    }
                    rv = bcm_stat_get(unit, port, s, &val);;
                    if (rv < 0) {
                        cli_out("%8s\t%s (stat %d): %s\n",
                                "-", sname, s, bcm_errmsg(rv));
                        continue;
                    }
                    if (flags == 0 && COMPILER_64_IS_ZERO(val)) {
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
            }
            break;
        default:
            return(CMD_FAIL);
    }

    return(CMD_OK);
}

