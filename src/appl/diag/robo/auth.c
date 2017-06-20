/*
 * $Id: auth.c,v 1.20 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Auth (802.1x) CLI commands
 */
#include <shared/bsl.h>

#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#include <bcm/error.h>
#include <bcm/auth.h>
#include <bcm/l2.h>
#include <bcm/debug.h>
#include <bcm_int/robo/auth.h>


cmd_result_t
cmd_robo_auth(int unit, args_t *a)
{
    char           *subcmd;
    parse_table_t  pt;
    cmd_result_t   ret_code;
    int            ingress;
    int            l2ena;
    int            rxdapdrop;
    int            port, r;
    static pbmp_t  pbmp;
    static pbmp_t  auth_pbmp,pbm;
    static sal_mac_addr_t mac;
    char           *mac_str;
    int            auth_num, i, sa_num;
    uint32         mode;
    uint32 pbmp_value[SOC_PBMP_WORD_MAX];
    bcm_l2_learn_limit_t limit;

    sal_memset(&limit, 0, sizeof(bcm_l2_learn_limit_t));

    
    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    ingress = 0;
    mode = 0;
    l2ena = 0;
    rxdapdrop = 0;

    /* init the valid portBitMap for authenticating feature */
    ret_code = (DRV_SERVICES(unit)->dev_prop_get
                    (unit, DRV_DEV_PROP_AUTH_PBMP, &pbmp_value[0]));
    for (i=0; i < SOC_PBMP_WORD_MAX; i++){
        SOC_PBMP_WORD_SET(auth_pbmp, i, pbmp_value[i]);
    }

    pbm = PBMP_E_ALL(unit);
    BCM_PBMP_AND(auth_pbmp,pbm);

    if (sal_strcasecmp(subcmd, "mac") == 0) {
        if ((subcmd = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }

        if (sal_strcasecmp(subcmd, "init") == 0) {
            mac_str = soc_property_get_str(unit, spn_STATION_MAC_ADDRESS);
            if (!mac_str) {
                cli_out("station_mac_address not set\n");
                return CMD_FAIL;
            }

            if ((r = parse_macaddr(mac_str, mac)) < 0) {
                cli_out("ERROR: invalid mac string: \"%s\" (error=%d)\n", mac_str, r);
                return CMD_FAIL;
            }

            PBMP_ITER(auth_pbmp, port) {
                r = bcm_auth_mac_add(unit, port, mac);
                if (r < 0) {
                    cli_out("ERROR: %s %s failed: %s\n",
                            ARG_CMD(a), subcmd, bcm_errmsg(r));
                    return CMD_FAIL;
                }
            }
            cli_out("All fe ports set to mac address ");
            cli_out("%02x:%02x:%02x:%02x:%02x:%02x\n",
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "add") == 0) {
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                            (void *)(0), &pbmp, NULL);
            parse_table_add(&pt, "Mac", PQ_DFL | PQ_MAC, 0, 
                            (void *)mac, 0);
            if (!parseEndOk(a, &pt, &ret_code)) {
                return ret_code;
            }

            SOC_PBMP_AND(pbmp, auth_pbmp);
            i=0;
            PBMP_ITER(pbmp, port) {
#ifdef BCM_DINO16_SUPPORT
                if (SOC_IS_DINO16(unit)){
                    if (i >= 1) {
                        cli_out("ERROR : %s %s port %s failed:  \n",
                                ARG_CMD(a), subcmd, SOC_PORT_NAME(unit, port));
                        return CMD_FAIL;
                    }
                }
#endif /* BCM_DINO16_SUPPORT */
                r = bcm_auth_mac_add(unit, port, mac);

                if (r < 0) {
                    cli_out("ERROR: %s %s port %s failed: %s\n",
                            ARG_CMD(a), subcmd, SOC_PORT_NAME(unit, port),
                            bcm_errmsg(r));
                    return CMD_FAIL;
                }
                cli_out("port %s mac address ",SOC_PORT_NAME(unit, port));
                cli_out("%02x:%02x:%02x:%02x:%02x:%02x set.\n",
                        mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
                i++;
            }
            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "del") == 0) {
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                            (void *)(0), &pbmp, NULL);
            parse_table_add(&pt, "Mac", PQ_DFL | PQ_MAC, 0,
                            (void *)mac, 0);
            if (!parseEndOk(a, &pt, &ret_code)) {
                return ret_code;
            }

            SOC_PBMP_AND(pbmp, auth_pbmp);
            PBMP_ITER(pbmp, port) {
                r = bcm_auth_mac_delete(unit, port, mac);

                if (r < 0) {
                    cli_out("ERROR: %s %s port %s failed: %s\n",
                            ARG_CMD(a), subcmd, SOC_PORT_NAME(unit, port),
                            bcm_errmsg(r));
                    return CMD_FAIL;
                }
                cli_out("port %s mac address ",SOC_PORT_NAME(unit, port));
                cli_out("%02x:%02x:%02x:%02x:%02x:%02x deleted.\n",
                        mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
            }
            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "clear") == 0) {
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                            (void *)(0), &pbmp, NULL);
            if (!parseEndOk(a, &pt, &ret_code)) {
                return ret_code;
            }

            SOC_PBMP_AND(pbmp, auth_pbmp);
            PBMP_ITER(pbmp, port) {
                r = bcm_auth_mac_delete_all(unit, port);

                if (r < 0) {
                    cli_out("ERROR: %s %s port %s failed: %s\n",
                            ARG_CMD(a), subcmd, SOC_PORT_NAME(unit, port),
                            bcm_errmsg(r));
                    return CMD_FAIL;
                }
                cli_out("port %s all mac addresses deleted.\n",
                        SOC_PORT_NAME(unit, port));
            }
            return CMD_OK;
        }
    }

    if (sal_strcasecmp(subcmd, "block") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                        (void *)(0), &pbmp, NULL);
        parse_table_add(&pt, "IngressOnly", PQ_BOOL|PQ_DFL,
                        0, &ingress, 0);
        parse_table_add(&pt, "RxEapDrop", PQ_BOOL|PQ_DFL,
                        0, &rxdapdrop, 0);
        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }

        if (ingress) {
            mode = BCM_AUTH_MODE_UNAUTH | BCM_AUTH_BLOCK_IN;
        }
        else {
            mode = BCM_AUTH_MODE_UNAUTH | BCM_AUTH_BLOCK_INOUT;
        }

        if (rxdapdrop) {
            mode |= BCM_AUTH_SEC_RX_EAP_DROP;
        }

        SOC_PBMP_AND(pbmp, auth_pbmp);
        PBMP_ITER(pbmp, port) {
            r = bcm_auth_mode_set(unit, port, mode);

            if (r < 0) {
                cli_out("ERROR: %s %s port %s failed: %s\n",
                        ARG_CMD(a), subcmd, SOC_PORT_NAME(unit, port),
                        bcm_errmsg(r));
                return CMD_FAIL;
            }
            cli_out("port %s blocked in %s direction(s).\n",
                    SOC_PORT_NAME(unit, port), ingress ? "ingress" : "all");
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "unblock") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                        (void *)(0), &pbmp, NULL);
        parse_table_add(&pt, "RxEapDrop", PQ_BOOL|PQ_DFL,
                        0, &rxdapdrop, 0);
        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }

        mode = BCM_AUTH_MODE_UNCONTROLLED;

        if (rxdapdrop) {
            mode |= BCM_AUTH_SEC_RX_EAP_DROP;
        }

        SOC_PBMP_AND(pbmp, auth_pbmp);
        PBMP_ITER(pbmp, port) {
            r = bcm_auth_mode_set(unit, port, mode);

            if (r < 0) {
                cli_out("ERROR: %s %s port %s failed: %s\n",
                        ARG_CMD(a), subcmd, SOC_PORT_NAME(unit, port),
                        bcm_errmsg(r));
                return CMD_FAIL;
            }
            cli_out("port %s unblock - all traffic allowed now.\n",
                    SOC_PORT_NAME(unit, port));
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "enable") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                        (void *)(0), &pbmp, NULL);
        parse_table_add(&pt, "LearnEnable", PQ_BOOL|PQ_DFL,
                        0, &l2ena, 0);
        parse_table_add(&pt, "AuthNum", PQ_INT,
                        0, &auth_num, 0);
        parse_table_add(&pt, "SaNum", PQ_INT,
                        0, &sa_num, 0);
        parse_table_add(&pt, "RxEapDrop", PQ_BOOL|PQ_DFL,
                        0, &rxdapdrop, 0);
        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }

        mode = BCM_AUTH_MODE_AUTH;
        
        if (rxdapdrop) {
            mode |= BCM_AUTH_SEC_RX_EAP_DROP;
        }
        
#define AUTH_SEC_NONE          0 
#define AUTH_SEC_STATIC_ACCEPT 1
#define AUTH_SEC_STATIC_REJECT 2
#define AUTH_SEC_SA_NUM        3 
#define AUTH_SEC_SA_MATCH      4
#define AUTH_SEC_EXTEND_MODE   5 
#define AUTH_SEC_SIMPLFY_MODE  6
#ifdef BCM_TB_SUPPORT
#define AUTH_SEC_SA_MOVE_DROP  7
#define AUTH_SEC_SA_MOVE_CPUCOPY  8
#define AUTH_SEC_SA_UNKNOWN_DROP  9
#define AUTH_SEC_SA_UNKNOWN_CPUCOPY  10
#define AUTH_SEC_SA_OVERLIMIT_DROP  11
#define AUTH_SEC_SA_OVERLIMIT_CPUCOPY  12
#endif

        if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
            switch (auth_num) {
            	case AUTH_SEC_NONE:
                  mode |= BCM_AUTH_SEC_NONE;
                  break;
            	case AUTH_SEC_SA_MOVE_DROP:
                  mode |= BCM_AUTH_SEC_SA_MOVEMENT_DROP;
                  break;
            	case AUTH_SEC_SA_MOVE_CPUCOPY:
                  mode |= BCM_AUTH_SEC_SA_MOVEMENT_CPUCOPY;
                  break;
            	case AUTH_SEC_SA_UNKNOWN_DROP:
                  mode |= BCM_AUTH_SEC_SA_UNKNOWN_DROP;
                  break;
            	case AUTH_SEC_SA_UNKNOWN_CPUCOPY:
                  mode |= BCM_AUTH_SEC_SA_UNKNOWN_CPUCOPY;
                  break;
            	case AUTH_SEC_SA_OVERLIMIT_DROP:
                  mode |= BCM_AUTH_SEC_SA_OVERLIMIT_DROP;
                  break;
            	case AUTH_SEC_SA_OVERLIMIT_CPUCOPY:
                  mode |= BCM_AUTH_SEC_SA_OVERLIMIT_CPUCOPY;
                  break;
            	default:
                  cli_out("ERROR: %s without expected parameter.\n",   
                          ARG_CMD(a));   
                  return CMD_FAIL;
            }
#endif
        } else {
            switch (auth_num) {
            	case AUTH_SEC_NONE:
                  mode |= BCM_AUTH_SEC_NONE;
                  break;
              case AUTH_SEC_STATIC_ACCEPT:
                  mode |= BCM_AUTH_SEC_STATIC_ACCEPT;
                  break;
              case AUTH_SEC_STATIC_REJECT:
                  mode |= BCM_AUTH_SEC_STATIC_REJECT;
                  break;
              case AUTH_SEC_SA_NUM:
                  mode |= BCM_AUTH_SEC_SA_NUM;
                  break;
              case AUTH_SEC_SA_MATCH:
                  mode |= BCM_AUTH_SEC_SA_MATCH;
                  break;
              case AUTH_SEC_EXTEND_MODE:
                  mode |= BCM_AUTH_SEC_EXTEND_MODE;
                  break;
              case AUTH_SEC_SIMPLFY_MODE:
                  mode |= BCM_AUTH_SEC_SIMPLIFY_MODE;
                  break;
              default:
                  cli_out("ERROR: %s without expected parameter.\n",
                          ARG_CMD(a));
                  return CMD_FAIL;
            }
        }
        
        if (l2ena) {
            mode |= BCM_AUTH_LEARN;
        } else {        /* not learning mode */
            mode &= ~BCM_AUTH_LEARN;
        }

        SOC_PBMP_AND(pbmp, auth_pbmp);
        PBMP_ITER(pbmp, port) {
            r = bcm_auth_mode_set(unit, port, mode);
            if (r < 0) {
                cli_out("ERROR: %s %s port %s failed: %s\n",
                        ARG_CMD(a), subcmd, SOC_PORT_NAME(unit, port),
                        bcm_errmsg(r));
                return CMD_FAIL;
            }
            if (mode & BCM_AUTH_SEC_SA_NUM) {
                cli_out("sanum = 0x%x\n", sa_num);
                limit.port = port;
                limit.flags = BCM_L2_LEARN_LIMIT_PORT;
                if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
                    if (mode & BCM_AUTH_SEC_EXTEND_MODE) {
                        limit.flags |= BCM_L2_LEARN_LIMIT_ACTION_DROP;
                    } else if (mode & BCM_AUTH_SEC_SIMPLIFY_MODE) {
                        limit.flags |= BCM_L2_LEARN_LIMIT_ACTION_CPU;
                    }
#endif
                } else {
                    /* Get now action(drop or copy to cpu) for 
                      * SA violation(not in ARL table or exceeding learning limitation) 
                      */
                    bcm_l2_learn_limit_get(unit, &limit);
                }

                limit.limit = sa_num;
                r = bcm_l2_learn_limit_set(unit, &limit);
                if (r < 0) {
                    cli_out("ERROR: %s %s port %s failed: %s\n",
                            ARG_CMD(a), subcmd, SOC_PORT_NAME(unit, port),
                            bcm_errmsg(r));
                    return CMD_FAIL;
                }
            }
            cli_out("port %s authorization enabled.\n",
                    SOC_PORT_NAME(unit, port));
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "disable") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                        (void *)(0), &pbmp, NULL);
        parse_table_add(&pt, "RxEapDrop", PQ_BOOL|PQ_DFL,
                        0, &rxdapdrop, 0);
        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }

        mode = BCM_AUTH_MODE_UNAUTH | BCM_AUTH_BLOCK_INOUT;

        if (rxdapdrop) {
            mode |= BCM_AUTH_SEC_RX_EAP_DROP;
        }

        SOC_PBMP_AND(pbmp, auth_pbmp);
        PBMP_ITER(pbmp, port) {
            r = bcm_auth_mode_set(unit, port, mode);

            if (r < 0) {
                cli_out("ERROR: %s %s port %s failed: %s\n",
                        ARG_CMD(a), subcmd, SOC_PORT_NAME(unit, port),
                        bcm_errmsg(r));
                return CMD_FAIL;
            }
            cli_out("port %s authorization disable - no traffic allowed.\n",
                    SOC_PORT_NAME(unit, port));
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "init") == 0) {
        if ((r = bcm_auth_init(unit)) < 0) {
            cli_out("%s: error initializing: %s\n", ARG_CMD(a), bcm_errmsg(r));
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "detach") == 0) {
        r = bcm_auth_detach(unit);
        if (r < 0) {
            cli_out("ERROR: %s %s failed: %s\n",
                    ARG_CMD(a), subcmd, bcm_errmsg(r));
            return CMD_FAIL;
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "show") == 0) {
        PBMP_ITER(auth_pbmp, port) {
            r = bcm_auth_mode_get(unit, port, &mode);

            if (r < 0) {
                cli_out("ERROR: %s %s port %s failed: %s\n",
                        ARG_CMD(a), subcmd, SOC_PORT_NAME(unit, port),
                        bcm_errmsg(r));
                return CMD_FAIL;
            }
            if (mode & BCM_AUTH_MODE_UNCONTROLLED) {
                cli_out("port %s in uncontrolled state.\n",
                        SOC_PORT_NAME(unit, port));
            } else if (mode & BCM_AUTH_MODE_UNAUTH) {
                cli_out("port %s is unauthorized for %s direction(s).\n",
                        SOC_PORT_NAME(unit, port),
                        (mode & BCM_AUTH_BLOCK_IN) ? "ingress" : "all");
            } else {
                if (!(mode & (BCM_AUTH_LEARN|BCM_AUTH_IGNORE_LINK
                            |BCM_AUTH_IGNORE_VIOLATION))) {
                    cli_out("port %s is authorized.\n",
                            SOC_PORT_NAME(unit, port));
                } else {
                   cli_out("port %s is authorized with condition(s) %s%s%s\n",
                           SOC_PORT_NAME(unit, port),
                           (mode & BCM_AUTH_LEARN) ? "L2LEAERN " : "",
                           (mode & BCM_AUTH_IGNORE_LINK) ? "IGNORE_LINK " : "",
                           (mode & BCM_AUTH_IGNORE_VIOLATION) ? \
                           "IGNORE_VIOLATION " : "");
                }
            }
        }
        return CMD_OK;
    }

    return CMD_USAGE;
}

