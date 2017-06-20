/*
 * $Id: mirror.c,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#include <bcm/error.h>
#include <bcm/mirror.h>
#include <bcm/stack.h>
#include <bcm/debug.h>
#include <bcm/switch.h>
#include <bcm_int/control.h>

cmd_result_t
if_robo_mirror(int unit, args_t *a)
{
    int rv;
    soc_port_t port;

    static char *mode_list[] = {"Off", "L2", NULL};

    static int arg_mode = 0;     /* Off*/
    static pbmp_t arg_ing_bmp, arg_eng_bmp, arg_mport_bmp;
    
    parse_table_t pt;
    cmd_result_t ret_code;

    char *c;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    BCM_PBMP_CLEAR(arg_mport_bmp);
    BCM_PBMP_CLEAR(arg_ing_bmp);
    BCM_PBMP_CLEAR(arg_eng_bmp);

    if ((c = ARG_CUR(a)) != NULL) {
        if (sal_strcasecmp(c, "mtp") == 0) {
            char pbmp_str[80];
            char pfmt[SOC_PBMP_FMT_LEN];
            char *subcmd;

            c = ARG_GET(a);
            if ((subcmd = ARG_GET(a)) == NULL) {
                return CMD_USAGE;
            }
            
            if (sal_strcasecmp(subcmd, "dump_sw") == 0) {
                rv = bcm_mirror_to_pbmp_get(unit, 0, &arg_mport_bmp);
                if (rv < 0) {
                    cli_out("%s: bcm_mirror_to_pbmp_get failed: %s\n", ARG_CMD(a),
                            bcm_errmsg(rv));
                    return CMD_FAIL;
                }

                format_pbmp(unit, pbmp_str, sizeof(pbmp_str), arg_mport_bmp);
                cli_out("MPortBitMap (SW) = %s ==> %s\n", 
                        SOC_PBMP_FMT(arg_mport_bmp, pfmt), pbmp_str);

                return CMD_OK;
            } else if (sal_strcasecmp(subcmd, "dump_hw") == 0) {
                rv = DRV_MIRROR_GET
                    (unit, (uint32 *)&arg_mode, &arg_mport_bmp, &arg_ing_bmp, &arg_eng_bmp);
                if (rv < 0) {
                    cli_out("%s: DRV_MIRROR_GET failed: %s\n", ARG_CMD(a),
                            bcm_errmsg(rv));
                    return CMD_FAIL;
                }

                format_pbmp(unit, pbmp_str, sizeof(pbmp_str), arg_mport_bmp);
                cli_out("MPortBitMap (HW) = %s ==> %s\n", 
                        SOC_PBMP_FMT(arg_mport_bmp, pfmt), pbmp_str);

                return CMD_OK;
            } else {
                return CMD_USAGE;
            }
        }
    }

    rv = bcm_mirror_to_pbmp_get(unit, 0, &arg_mport_bmp);
    if (rv < 0) {
        cli_out("%s: bcm_mirror_to_pbmp_get failed: %s\n", ARG_CMD(a),
                bcm_errmsg(rv));
        return CMD_FAIL;
    }

    rv = bcm_mirror_mode_get(unit, &arg_mode);
    if (rv < 0) {
        cli_out("%s: bcm_mirror_mode_get failed: %s\n", ARG_CMD(a),
                bcm_errmsg(rv));
        return CMD_FAIL;
    }

    PBMP_ALL_ITER(unit, port) {
        int enable = 0;

        rv = bcm_mirror_ingress_get(unit, port, &enable);
        if (rv < 0 && !IS_CPU_PORT(unit, port)) {
		cli_out("%s: bcm_mirror_ingress_get failed: %s\n", ARG_CMD(a),
                        bcm_errmsg(rv));
		return CMD_FAIL;
	 }

	 if (enable) {
            BCM_PBMP_PORT_ADD(arg_ing_bmp, port);
	 }

        rv = bcm_mirror_egress_get(unit, port, &enable);
	 if (rv < 0 && !IS_CPU_PORT(unit, port)) {
            cli_out("%s: bcm_mirror_egress_get failed: %s\n", ARG_CMD(a),
                    bcm_errmsg(rv));
            return CMD_FAIL;
        }

        if (enable) {
		BCM_PBMP_PORT_ADD(arg_eng_bmp, port);
        }
    }
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "Mode"         , \
                    PQ_DFL|PQ_MULTI,0, &arg_mode, mode_list);
    parse_table_add(&pt, "MPortBitMap"         , \
                    PQ_DFL|PQ_PBMP,(void *)( 0), &arg_mport_bmp, NULL);
    parse_table_add(&pt, "IngressBitMap", \
                    PQ_DFL|PQ_PBMP,(void *)(0), &arg_ing_bmp, NULL);
    parse_table_add(&pt, "EgressBitMap" , \
                    PQ_DFL|PQ_PBMP,(void *)(0), &arg_eng_bmp, NULL);    
    
    if (!parseEndOk( a, &pt, &ret_code)) {
        return ret_code;
    }
    rv = bcm_mirror_to_pbmp_set(unit, 0, arg_mport_bmp);
    
    if (rv < 0) {
        cli_out("%s: bcm_mirror_to_pbmp_set failed: %s\n", ARG_CMD(a),
                bcm_errmsg(rv));
        if (BCM_PBMP_IS_NULL(arg_mport_bmp)) {
            cli_out("mirror to port should be set\n");
        }
        return CMD_FAIL;
    }

    switch (arg_mode) {
        case 0:
            rv = bcm_mirror_mode(unit, BCM_MIRROR_DISABLE);
            break;
        case 1:
            rv = bcm_mirror_mode(unit, BCM_MIRROR_L2);
            break;
        default:
            break;
    }
    
    if (rv < 0) {
        cli_out("%s: bcm_mirror_mode failed: %s\n", ARG_CMD(a), bcm_errmsg(rv));
        return CMD_FAIL;
    }
    if (arg_mode == 0) {/* skip ingress/egress for disable case */
        return CMD_OK;
    }

    PBMP_ALL_ITER(unit, port) {
        rv = bcm_mirror_ingress_set(unit, port,\
                                    PBMP_MEMBER(arg_ing_bmp, port) ? 1 : 0);
        if (rv < 0) {
            cli_out(\
                    "%s: bcm_mirror_ingress_set failed: %s\n", \
                    ARG_CMD(a),bcm_errmsg(rv));
            return CMD_FAIL;
        }
        rv = bcm_mirror_egress_set(unit, port,\
                                   PBMP_MEMBER(arg_eng_bmp, port) ? 1 : 0);
        if (rv < 0) {
            cli_out(\
                    "%s: bcm_mirror_egress_set failed: %s\n", \
                    ARG_CMD(a),bcm_errmsg(rv));
            return CMD_FAIL;
        }
    }
                    
    return CMD_OK;
}

static void
_robo_dmirror_show(int unit, int port, int dstmod, int dstport, uint32 flags, int mymodid)
{
    int rv, show_dest, count = 0;
    char *mstr;
    uint16 tpid, vlan;
    bcm_pbmp_t  mport_pbmp;
    char pbmp_str[80];
    char pfmt[SOC_PBMP_FMT_LEN];

    BCM_PBMP_CLEAR(mport_pbmp);

    show_dest = 1;
    if (flags & BCM_MIRROR_PORT_INGRESS) {
        if (flags & BCM_MIRROR_PORT_EGRESS) {
            mstr = "Mirror all";
        } else {
            mstr = "Mirror ingress";
        }
    } else if (flags & BCM_MIRROR_PORT_EGRESS) {
        mstr = "Mirror egress";
    } else if (flags & BCM_MIRROR_PORT_ENABLE) {
        mstr = "Mirror";
    } else {
        mstr = "-";
        show_dest = 0;
    }
    cli_out("%4s: %s", SOC_PORT_NAME(unit, port), mstr);

    bcm_mirror_to_pbmp_get(unit, 0, &mport_pbmp);
    BCM_PBMP_COUNT(mport_pbmp, count);
    format_pbmp(unit, pbmp_str, sizeof(pbmp_str), mport_pbmp);

    if (show_dest) {
        if (flags & BCM_MIRROR_PORT_DEST_TRUNK) {
            cli_out(" to trunk %d", dstport);
        } else if (dstmod == mymodid) {
            if (count >= 2) {
                cli_out(" to local port bitmap %s ==> %s", 
                        SOC_PBMP_FMT(mport_pbmp, pfmt), pbmp_str);
            } else {
                cli_out(" to local port %s", SOC_PORT_NAME(unit, dstport));
            }
        } else {
            cli_out(" to module %d, port %d", dstmod, dstport+1);
        }
        if (dstmod == mymodid) {
            rv = bcm_mirror_vlan_get(unit, port, &tpid, &vlan);
            if (rv == BCM_E_NONE && vlan > 0) {
                cli_out(" (TPID=%d, VLAN=%d)", tpid, vlan);
            }
        }
    }
    cli_out("\n");
}

char if_robo_dmirror_usage[] =
    "Parameters: <ports> [Mode=<Off|On|Ingress|Egress|All>]\n"
    "\t[DestPortBitMap=<pbmp>] [DestModule=<modid>] [DestTrunk=<tgid>]\n"
    "\t[MirrorTagProtocolID=<tpid>] [MirrorVlanID=<vid>\n"
    "\tDestTrunk overrides DestModule/DestPort.\n"
    "\tTPID and VLAN are set only if DestPort is a local port.\n";

cmd_result_t
if_robo_dmirror(int unit, args_t *a)
{
    int	rv;
    int port, dstmod, dstport;
    int mymodid;
    uint32 flags;
    parse_table_t pt;
    cmd_result_t retCode;
    bcm_pbmp_t pbm, tmp_pbm, argDestPbmp, dstportbmp;
    char *mstr;
    int argDestMod = -1, argDestTrunk = -1;
    int argMode = -1, argTpid = -1, argVlan = -1;
    int mirror_mode = 0;
    int isGport = 0;
    bcm_pbmp_t mport_bmp;

    char *modeList[] = {"OFF", "ON", "Ingress", "Egress", "All", NULL};

    if (!sh_check_attached(ARG_CMD(a), unit)) {
	return CMD_FAIL;
    }

    rv = bcm_stk_my_modid_get(unit, &mymodid);
    if (rv < 0) {
        cli_out("%s: bcm_stk_my_modid_get: %s\n", ARG_CMD(a),
                bcm_errmsg(rv));
        return CMD_FAIL;
    }

    if ((mstr = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    if (parse_pbmp(unit, mstr, &pbm) < 0) {
        cli_out("%s: Error: unrecognized port bitmap: %s\n",
                ARG_CMD(a), mstr);
        return CMD_FAIL;
    }

    BCM_PBMP_AND(pbm, PBMP_E_ALL(unit));
    if (BCM_PBMP_IS_NULL(pbm)) {
        cli_out("%s: Error: unsupported port bitmap: %s\n",
                ARG_CMD(a), mstr);
        return CMD_FAIL;
    }

    BCM_PBMP_CLEAR(argDestPbmp);
    BCM_PBMP_CLEAR(mport_bmp);

    if (ARG_CNT(a) == 0) {
        rv = bcm_switch_control_get(unit, bcmSwitchUseGport, &isGport);
        if (rv < 0) {
            cli_out("%s: bcm_switch_control_get (bcmSwitchUseGport) failed: %s\n",
                    ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }

        /* Get mirror-to port bitmap to check is NULL ro not */
        rv = bcm_mirror_to_pbmp_get(unit, 0, &mport_bmp);
        if (rv < 0) {
            cli_out("%s: bcm_mirror_to_pbmp_get failed: %s\n",
                    ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }

        /* Show FE ports separately (for Tucana) */
        tmp_pbm = pbm;
        BCM_PBMP_AND(tmp_pbm, PBMP_FE_ALL(unit));
        PBMP_ITER(tmp_pbm, port) {
            rv = bcm_mirror_port_get(unit, port, &dstmod, &dstport, &flags);
            if (rv < 0) {
                cli_out("%s: bcm_mirror_port_get: %s\n", ARG_CMD(a),
                        bcm_errmsg(rv));
                return CMD_FAIL;
            }

            /* Check mirror-to port bitmap is not NULL */
            if (isGport && BCM_PBMP_NOT_NULL(mport_bmp)) {
                rv = bcm_port_local_get(unit, dstport, &dstport);
                if (rv < 0) {
                    cli_out("%s: bcm_port_local_get: %s\n", ARG_CMD(a),
                            bcm_errmsg(rv));
                    return CMD_FAIL;
                }
            }

            _robo_dmirror_show(unit, port, dstmod, dstport, flags, mymodid);
        }
        /* Show non-FE ports */
        tmp_pbm = pbm;
        BCM_PBMP_REMOVE(tmp_pbm, PBMP_FE_ALL(unit));
        PBMP_ITER(tmp_pbm, port) {
            rv = bcm_mirror_port_get(unit, port, &dstmod, &dstport, &flags);
            if (rv < 0) {
                cli_out("%s: bcm_mirror_port_get: %s\n", ARG_CMD(a),
                        bcm_errmsg(rv));
                return CMD_FAIL;
            }

            /* Check mirror-to port bitmap is not NULL */
            if (isGport && BCM_PBMP_NOT_NULL(mport_bmp)) {
                rv = bcm_port_local_get(unit, dstport, &dstport);
                if (rv < 0) {
                    cli_out("%s: bcm_port_local_get: %s\n", ARG_CMD(a),
                            bcm_errmsg(rv));
                    return CMD_FAIL;
                }
            }

            _robo_dmirror_show(unit, port, dstmod, dstport, flags, mymodid);
        }
        return CMD_OK;
    }

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "DestPortBitMap", PQ_DFL|PQ_PBMP,
                    (void *)(0), &argDestPbmp, NULL);
    parse_table_add(&pt, "DestModule", PQ_DFL|PQ_INT,
                    (void *)(0), &argDestMod, NULL);
    parse_table_add(&pt, "DestTrunk", PQ_DFL|PQ_INT,
                    (void *)(0), &argDestTrunk, NULL);
    parse_table_add(&pt, "MirrorTagProtocolID", PQ_DFL|PQ_INT,
                    (void *)(0), &argTpid, NULL);
    parse_table_add(&pt, "MirrorVlanID", PQ_DFL|PQ_INT,
                    (void *)(0), &argVlan, NULL);
    parse_table_add(&pt, "Mode", PQ_DFL|PQ_MULTI,
                    (void *)(0), &argMode, modeList);
    if (!parseEndOk( a, &pt, &retCode)) {
        return retCode;
    }

    flags = 0;

    if (argMode < 0) {
        return CMD_USAGE;
    }

    if ((argMode > 0) && (BCM_PBMP_IS_NULL(argDestPbmp))) {
        return CMD_USAGE;
    }

    if (argDestTrunk < 0) {
        BCM_PBMP_ASSIGN(dstportbmp, argDestPbmp);
    } else {
        BCM_PBMP_PORT_SET(dstportbmp, argDestTrunk);
        flags |= BCM_MIRROR_PORT_DEST_TRUNK;
    }

    switch (argMode) {
    case 0:
        flags = BCM_MIRROR_DISABLE;
        break;
    case 1:
        flags |= BCM_MIRROR_PORT_ENABLE;
        break;
    case 2:
        flags |= BCM_MIRROR_PORT_INGRESS;
        break;
    case 3:
        flags |= BCM_MIRROR_PORT_EGRESS;
        break;
    case 4:
        flags |= (BCM_MIRROR_PORT_INGRESS | BCM_MIRROR_PORT_EGRESS);
        break;
    default:
        break;
    }

    if (flags == BCM_MIRROR_DISABLE) {
        mirror_mode = BCM_MIRROR_DISABLE;
    } else {
        mirror_mode = BCM_MIRROR_L2;
    }

    rv = bcm_mirror_mode_set(unit, mirror_mode);
    if (rv < 0) {
        cli_out("%s: bcm_mirror_mode_set failed: %s\n", ARG_CMD(a),
                bcm_errmsg(rv));
        return CMD_FAIL;
    }


    PBMP_ITER(dstportbmp, dstport) {
        break;
    }

    PBMP_ITER(pbm, port) {
        rv = bcm_mirror_port_set(unit, port, argDestMod, dstport, flags);

        if (rv < 0) {
            cli_out("%s: bcm_mirror_port_set: %s\n", ARG_CMD(a),
                    bcm_errmsg(rv));
            return CMD_FAIL;
        }

        if (argDestMod < 0 || argDestMod == mymodid) {
            if (argTpid >= 0 && argVlan >= 0 && argDestTrunk < 0) {
                rv = bcm_mirror_vlan_set(unit, port, argTpid, argVlan);

                if (rv < 0) {
                    cli_out("%s: bcm_mirror_vlan_set failed: %s\n", ARG_CMD(a),
                            bcm_errmsg(rv));
                    return CMD_FAIL;
                }
            }
        }
    }

    rv = bcm_mirror_to_pbmp_set(unit, 0, dstportbmp);

    if (rv < 0) {
        cli_out("%s: bcm_mirror_to_pbmp_set: %s\n", ARG_CMD(a),
                bcm_errmsg(rv));
        return CMD_FAIL;
    }

    return CMD_OK;
}

