/* 
 * $Id: mpls.c,v 1.19 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        mpls.c
 * Purpose:     mpls vpn CLI commands
 *
 */
#include <shared/bsl.h>

#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#include <shared/gport.h>
#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/l3.h>
#include <bcm/debug.h>
#include <bcm/ipmc.h>
#include <bcm/tunnel.h>
#include <bcm/stack.h>
#include <bcm/cosq.h>
#include <bcm/trunk.h>
#include <bcm/mpls.h>
#include <bcm/failover.h>

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/qe2000_util.h>

#include <bcm_int/sbx/fe2000/allocator.h>

STATIC cmd_result_t _cmd_sbx_mpls_vpn_id(int unit, args_t *args);
STATIC cmd_result_t _cmd_sbx_mpls_vpn_port(int unit, args_t *args);
STATIC cmd_result_t _cmd_sbx_mpls_switch(int unit, args_t *args);
STATIC cmd_result_t _cmd_sbx_mpls_vpls_fo_test(int unit, args_t *args);

int
_cmd_sbx_mpls_route_print(int unit, 
                          bcm_mpls_tunnel_switch_t *info,
                          void *cookie);
int
_cmd_sbx_mpls_vpn_print(int unit, 
                        bcm_mpls_vpn_config_t *info,
                        void *cookie);

#define CMD_MPLS_SWITCH_USAGE                                           \
    "    switch [ add | delete | delete-all | get | get-all]\n"                  \
    "           [in-label=<in-label>] [gport=<in-port>]\n"              \
    "           [exp-map=<value>] [int-pri=value]\n"                    \
    "           [action=<SWAP|PHP|POP|POP-DIRECT]\n"                    \
    "           [vpn=<vpn>] [egress-if=<fte>]\n"

#define CMD_MPLS_VPN_ID_USAGE                                  \
    "    vpn-id [create | destroy | destroy-all | get | get-all]\n"      \
    "         [type=<L3|PWE|VPLS>] [lookup-id=<id>]\n"         \
    "         [broadcast-group=<mcg>] [vpn-id=<vpn>]\n"        \

#define CMD_MPLS_VPN_PORT_USAGE                                         \
    "    vpn-port  [add | delete | delete-all | get | get-all]\n"       \
    "         [vpn=<vpn>]\n"                                            \
    "         [mpls-port-id=<mpls-gport>] \n"                           \
    "         [exp-map=<value>] [int-pri=value]\n"                      \
    "         [port=<port>] [module=<module>]\n"                        \
    "         [match-criteria=<NONE|PORT|PORT-VLAN|PORT-SVLAN|LABEL|PORT-LABEL|LABEL-VLAN]\n" \
    "         [match-vlan=<vlan>][match-inner-vlan=<vlan>]\n"           \
    "         [match-label=<label>]\n"                                  \
    "         [service-tpid=<value>][service-vlan=<value>]\n"           \
    "         [egress-if=<fte>] [egress-label-val=<label-out>]\n"       \
    "         [egress-label-exp=<exp>][egress-label-ttl=<ttl>]\n"       \
    "         [encap-id=<ohi>]\n"
#define CMD_MPLS_VPLS_FO_TEST_USAGE                                     \
    "     vpls-fo-test [config | fo-switch | unconfig | show]\n"

static cmd_t _cmd_sbx_mpls_list[] = {
#ifndef COMPILER_STRING_CONST_LIMIT
    {"vpn-id",        _cmd_sbx_mpls_vpn_id,        "\n"  CMD_MPLS_VPN_ID_USAGE, NULL},
    {"vpn-port",      _cmd_sbx_mpls_vpn_port,      "\n"  CMD_MPLS_VPN_PORT_USAGE, NULL},
    {"switch",        _cmd_sbx_mpls_switch,        "\n"  CMD_MPLS_SWITCH_USAGE, NULL},
#endif
    {"vpls-fo-test",  _cmd_sbx_mpls_vpls_fo_test,  "\n"  CMD_MPLS_VPLS_FO_TEST_USAGE, NULL}
#
};

char cmd_sbx_mpls_usage[] =
    "\n"
#ifdef COMPILER_STRING_CONST_LIMIT
    "    mpls  <option> [args...]\n"
#else
    CMD_MPLS_VPN_ID_USAGE     "\n"
    CMD_MPLS_VPN_PORT_USAGE   "\n"
    CMD_MPLS_SWITCH_USAGE     "\n"
    CMD_MPLS_VPLS_FO_TEST_USAGE     "\n"
#endif
    ;

#define SBX_DISPLAY_MPLS_PORT(info)                                     \
    do {                                                                \
        cli_out(\
                "mpls_port_id(0x%x) flags(0x%x) if_class(0x%x) exp_map(0x%x)\n", \
                (info)->mpls_port_id, (info)->flags, (info)->if_class, (info)->exp_map); \
        cli_out(\
                "int_pri(0x%x) service_tpid(0x%x) port(0x%x) criteria(%d) match_vlan(0x%x)\n", \
                (info)->int_pri, (info)->service_tpid, (info)->port, (info)->criteria, (info)->match_vlan); \
        cli_out(\
                "match_inner_vlan(0x%x) match_label(0x%x) egress_tunnel_if(0x%x) egress_label(0x%x)\n", \
                (info)->match_inner_vlan, (info)->match_label, (info)->egress_tunnel_if, (info)->egress_label.label); \
        cli_out(\
                "egress_service_vlan(0x%x) encap_id(0x%x) failover_id(0x%x) failover_port_id(0x%x) policer_id(0x%x)\n", \
                (info)->egress_service_vlan, (info)->encap_id, (info)->failover_id, \
                (info)->failover_port_id, (info)->policer_id);           \
    } while(0)


STATIC cmd_result_t
_cmd_sbx_mpls_switch(int unit, args_t *args)
{
    int                      rv;
    cmd_result_t             retCode;
    parse_table_t            pt;
    char	            *subcmd;
    char                    *action_str;
    bcm_mpls_tunnel_switch_t info;
    
    if ((subcmd = ARG_GET(args)) == NULL) {
        cli_out("%s ERROR: empty cmd\n", FUNCTION_NAME());
        return CMD_FAIL;
    }

    rv             = CMD_OK;
    retCode        = CMD_OK;
    sal_memset(&info, 0, sizeof(info));
    parse_table_init(unit, &pt);
    
    if (sal_strcasecmp(subcmd, "add") == 0) {
        parse_table_add(&pt, "in-label",  PQ_DFL | PQ_HEX,    0, &info.label,   NULL);
        parse_table_add(&pt, "gport",     PQ_DFL | PQ_HEX,    0, &info.port,    NULL);
        parse_table_add(&pt, "exp-map",   PQ_DFL | PQ_INT,    0, &info.exp_map, NULL);
        parse_table_add(&pt, "int-pri",   PQ_DFL | PQ_INT,    0, &info.int_pri, NULL);
        parse_table_add(&pt, "vpn",       PQ_DFL | PQ_HEX,    0, &info.vpn,     NULL);
        parse_table_add(&pt, "action",             PQ_STRING, 0, &action_str,   NULL);
        parse_table_add(&pt, "egress-if", PQ_DFL | PQ_HEX,    0, &info.egress_if, NULL);
        if (!parse_arg_eq(args, &pt)) {
            cli_out("%s: Unknown options: %s\n",
                    ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        if (sal_strcasecmp(action_str, "pop") == 0) {
            info.action = BCM_MPLS_SWITCH_ACTION_POP;
        } else if (sal_strcasecmp(action_str, "swap") == 0) {
            info.action = BCM_MPLS_SWITCH_ACTION_SWAP;
        } else if (sal_strcasecmp(action_str, "php") == 0) {
            info.action = BCM_MPLS_SWITCH_ACTION_PHP;
        } else if (sal_strcasecmp(action_str, "pop-direct") == 0) {
            info.action = BCM_MPLS_SWITCH_ACTION_POP_DIRECT;
        } else {
            parse_arg_eq_done(&pt);
            cli_out("%s ERROR: Invalid action %s", FUNCTION_NAME(), action_str);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

        rv = bcm_mpls_tunnel_switch_add(unit, &info);

    } else if (sal_strcasecmp(subcmd, "delete") == 0) {
        parse_table_add(&pt, "in-label",  PQ_DFL | PQ_HEX,    0, &info.label, NULL);
        if (!parseEndOk(args, &pt, &retCode)) {
            return retCode;
        }
        cli_out("Deleting label 0x%x\n", info.label);

        rv = bcm_mpls_tunnel_switch_delete(unit, &info);
        
    } else if (sal_strcasecmp(subcmd, "delete-all") == 0) {
        parse_arg_eq_done(&pt);

        rv = bcm_mpls_tunnel_switch_delete_all(unit);
    } else if (sal_strcasecmp(subcmd, "get") == 0) {
        parse_table_add(&pt, "in-label",  PQ_DFL | PQ_HEX,    0, &info.label, NULL);
        if (!parseEndOk(args, &pt, &retCode)) {
            return retCode;
        }
        rv = bcm_mpls_tunnel_switch_get(unit, &info);
        _cmd_sbx_mpls_route_print(unit, &info, NULL);
    } else if (sal_strcasecmp(subcmd, "get-all") == 0) {
        parse_arg_eq_done(&pt);
        cli_out("Label\t port\t action\t egr_label\t egr_intf\n");

        rv = bcm_mpls_tunnel_switch_traverse(unit, _cmd_sbx_mpls_route_print, NULL);
    } else {
        parse_arg_eq_done(&pt);
        cli_out("%s ERROR: Invalid option %s", FUNCTION_NAME(), subcmd);
        return CMD_FAIL;
    }

    /* coverity [mixed_enums] */
    if (rv == BCM_E_NONE) {
        rv = CMD_OK;
        cli_out("Command Successfull\n");
    } else {
        rv = CMD_FAIL;
        cli_out("Command failed\n");
    }
    
    return rv;
}

int
_cmd_sbx_mpls_route_print(int unit, bcm_mpls_tunnel_switch_t *info, void *cookie)
{
    cli_out("%05x    %07x  %02x      %05x      %05x\n",
            (uint32) info->label, (uint32) info->port, info->action,
            (uint32) info->egress_label.label, (uint32) info->egress_if);
    

    return BCM_E_NONE;
}

STATIC cmd_result_t
_cmd_sbx_mpls_vpn_id(int unit, args_t *args)
{
    int                       rv;
    parse_table_t             pt;
    char	             *subcmd;
    char                     *type_str;
    bcm_mpls_vpn_config_t     info;
    int                       vpn_id;
    
    if ((subcmd = ARG_GET(args)) == NULL) {
        cli_out("%s ERROR: empty cmd\n", FUNCTION_NAME());
        return CMD_FAIL;
    }

    rv        = BCM_E_NONE;
    vpn_id    = 0;
    sal_memset(&info, 0,  sizeof(bcm_mpls_vpn_config_t));
    
    parse_table_init(unit, &pt);
    if (sal_strcasecmp(subcmd, "create") == 0) {
        parse_table_add(&pt, "type",             PQ_STRING,     0, &type_str,       NULL);
        parse_table_add(&pt, "lookup-id",        PQ_DFL|PQ_INT, 0, &info.lookup_id, NULL);
        parse_table_add(&pt, "vpn-id",           PQ_DFL|PQ_HEX, 0, &vpn_id,         NULL);
        parse_table_add(&pt, "broadcast-group",  PQ_DFL|PQ_HEX, 0, &info.broadcast_group, NULL);
        if (!parse_arg_eq(args, &pt)) {
            cli_out("%s: Unknown options: %s\n",
                    ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        if (sal_strcasecmp(type_str, "L3") == 0) {
            info.flags |= BCM_MPLS_VPN_L3;
        } else if (sal_strcasecmp(type_str, "VPLS") == 0) {
            info.flags |= BCM_MPLS_VPN_VPLS;
        } else if (sal_strcasecmp(type_str, "PWE") == 0) {
            info.flags |= BCM_MPLS_VPN_VPWS;
        } else {
            parse_arg_eq_done(&pt);
            cli_out("%s ERROR: Invalid action %s", FUNCTION_NAME(), type_str);
            return CMD_FAIL;
        }
        
        parse_arg_eq_done(&pt);
        info.vpn = (bcm_vpn_t) vpn_id;
        if (info.vpn) {
            info.flags |= BCM_MPLS_VPN_WITH_ID;
        }

        info.unknown_unicast_group = info.unknown_multicast_group =
            info.broadcast_group;
        
        cli_out("vpn-id create: lookup-id= %d type= %s vpn= 0x%x "
                "broadcast-group=0x%x flags= 0x%x\n",
                info.lookup_id, type_str, info.vpn, info.broadcast_group,
                info.flags);

        rv = bcm_mpls_vpn_id_create(unit, &info);
        if (rv == BCM_E_NONE) {
            cli_out("vpn-id 0x%x created successfully\n", info.vpn);
        } else {
            cli_out("could not create vpn-id error (%s)\n", bcm_errmsg(rv));
        }
        
    } else if (sal_strcasecmp(subcmd, "destroy") == 0) {
        parse_table_add(&pt, "vpn-id",    PQ_DFL|PQ_HEX, 0, &vpn_id, NULL);
        if (!parse_arg_eq(args, &pt)) {
            cli_out("%s: Unknown options: %s\n",
                    ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }
        parse_arg_eq_done(&pt);
        info.vpn = (bcm_vpn_t) vpn_id;
        rv = bcm_mpls_vpn_id_destroy(unit, info.vpn);
        if (rv == BCM_E_NONE) {
            cli_out("vpn-id 0x%x destroyed successfully\n",
                    info.vpn);
        } else {
            cli_out("error(%s) destroying vpn-id(0x%x)\n",
                    bcm_errmsg(rv), info.vpn);
        }
        
    } else if (sal_strcasecmp(subcmd, "destroy-all") == 0) {

        rv = bcm_mpls_vpn_id_destroy_all(unit);
        if (rv == BCM_E_NONE) {
            cli_out("all vpn-id destroyed successfully\n");
        } else {
            cli_out("error(%s) destroying all vpn(s)\n",
                    bcm_errmsg(rv));
        }
        
    } else if (sal_strcasecmp(subcmd, "get") == 0) {
        parse_table_add(&pt, "vpn-id",    PQ_DFL|PQ_HEX, 0, &vpn_id, NULL);
        if (!parse_arg_eq(args, &pt)) {
            cli_out("%s: Unknown options: %s\n",
                    ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }
        parse_arg_eq_done(&pt);

        rv = bcm_mpls_vpn_id_get(unit, (bcm_vpn_t)vpn_id, &info);
        if (rv == BCM_E_NONE) {
            cli_out("\nVPN Id= 0x%x\n", info.vpn);
            cli_out("\tVPN Lookup Id= 0x%x\n", info.lookup_id);
            if (info.flags & BCM_MPLS_VPN_L3) {
                cli_out("vpn-id(0x%x) lookup-id(0x%x) VPN Type= L3\n",
                        info.vpn, info.lookup_id);
            } else if (info.flags & BCM_MPLS_VPN_VPWS) {
                cli_out("vpn-id(0x%x) lookup-id(0x%x) VPN Type= PWE\n",
                        info.vpn, info.lookup_id);
            } else if (info.flags & BCM_MPLS_VPN_VPLS) {
                cli_out("vpn-id(0x%x) lookup-id(0x%x) VPN Type= VPLS\n",
                        info.vpn, info.lookup_id);
            } else {
                cli_out("vpn-id(0x%x) lookup-id(0x%x) unknown VPN type(0x%x)\n",
                        info.vpn, info.lookup_id, info.flags);
            }
        } else {
            cli_out("could not get vpn-info for vpn-id(0x%x) error (%s)\n",
                    (int)vpn_id, bcm_errmsg(rv));
        }
    } else if (sal_strcasecmp(subcmd, "get-all") == 0) {
        parse_arg_eq_done(&pt);
        rv = bcm_mpls_vpn_traverse(unit, _cmd_sbx_mpls_vpn_print, NULL);
        if (rv != BCM_E_NONE) {
            cli_out("could not get vpn-info error (%s)\n", bcm_errmsg(rv));
        }
    } else {
        cli_out("%s ERROR: Invalid option %s", FUNCTION_NAME(), subcmd);
        parse_arg_eq_done(&pt);
        return CMD_FAIL;
    }
    
    parse_arg_eq_done(&pt);

    return CMD_OK;
}

int
_cmd_sbx_mpls_vpn_print(int unit, 
                          bcm_mpls_vpn_config_t *info,
                          void *cookie)
{
    if (info->flags & BCM_MPLS_VPN_L3) {
        cli_out("vpn-id(0x%x) lookup-id(0x%x) VPN Type= L3\n",
                info->vpn, info->lookup_id);
    } else if (info->flags & BCM_MPLS_VPN_VPWS) {
        cli_out("vpn-id(0x%x) lookup-id(0x%x) VPN Type= PWE\n",
                info->vpn, info->lookup_id);
    } else if (info->flags & BCM_MPLS_VPN_VPLS) {
        cli_out("vpn-id(0x%x) lookup-id(0x%x) VPN Type= VPLS\n",
                info->vpn, info->lookup_id);
    } else {
        cli_out("vpn-id(0x%x) lookup-id(0x%x) unknown VPN type(0x%x)\n",
                info->vpn, info->lookup_id, info->flags);
    }
    return BCM_E_NONE;

}

STATIC cmd_result_t
_cmd_sbx_mpls_vpn_port(int unit, args_t *args)
{
    int                rv;
    parse_table_t      pt;
    char	      *subcmd;
    bcm_mpls_port_t    info;
    int                vpn_int;
    bcm_vpn_t          vpn;
    bcm_port_t         port;
    bcm_module_t       module;
    bcm_gport_t        mpls_port_id = BCM_GPORT_INVALID;
    bcm_mpls_port_t   *all_ports;
    int                i, port_count;
    int                match_vlan, match_inner_vlan;
    char              *match_str;
    
    if ((subcmd = ARG_GET(args)) == NULL) {
        cli_out("%s ERROR: empty cmd\n", FUNCTION_NAME());
        return CMD_FAIL;
    }

    match_vlan = match_inner_vlan = BCM_VLAN_INVALID;
    rv      = BCM_E_NONE;
    bcm_mpls_port_t_init(&info);
    parse_table_init(unit, &pt);
    
    if (sal_strcasecmp(subcmd, "add") == 0) {
    
        parse_table_add(&pt, "vpn",              PQ_DFL|PQ_HEX, 0, &vpn_int, NULL);
        parse_table_add(&pt, "mpls-port-id",     PQ_DFL|PQ_HEX, 0, &info.mpls_port_id,     NULL);
        parse_table_add(&pt, "port",             PQ_DFL|PQ_INT, 0, &port,                  NULL);
        parse_table_add(&pt, "module",           PQ_DFL|PQ_INT, 0, &module,                NULL);
        parse_table_add(&pt, "match-criteria",   PQ_STRING,     0, &match_str,             NULL);
        parse_table_add(&pt, "match-vlan",       PQ_DFL|PQ_INT, 0, &match_vlan,            NULL);
        parse_table_add(&pt, "match-inner-vlan", PQ_DFL|PQ_INT, 0, &match_inner_vlan,      NULL);
        parse_table_add(&pt, "match-label",      PQ_DFL|PQ_INT, 0, &info.match_label,      NULL);
        parse_table_add(&pt, "service-tpid",     PQ_DFL|PQ_INT, 0, &info.service_tpid,     NULL);
        parse_table_add(&pt, "service-vlan",     PQ_DFL|PQ_INT, 0, &info.egress_service_vlan, NULL);
        parse_table_add(&pt, "egress-if",        PQ_DFL|PQ_HEX, 0, &info.egress_tunnel_if, NULL);
        parse_table_add(&pt, "egress-label-val", PQ_DFL|PQ_HEX, 0, &info.egress_label.label, NULL);
        parse_table_add(&pt, "egress-label-exp", PQ_DFL|PQ_HEX, 0, &info.egress_label.exp, NULL);
        parse_table_add(&pt, "egress-label-ttl", PQ_DFL|PQ_HEX, 0, &info.egress_label.ttl, NULL);
        parse_table_add(&pt, "encap-id",         PQ_DFL|PQ_HEX, 0, &info.encap_id,         NULL);
        if (!parse_arg_eq(args, &pt)) {
            cli_out("%s: Unknown options: %s\n",
                    ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }
        
        if (vpn_int > 0xFFFF) {
            return (CMD_FAIL);
        }
        vpn = vpn_int;
        _SHR_GPORT_MODPORT_SET(info.port, module, port);
        if (sal_strcasecmp(match_str, "none") == 0) {
            info.criteria = BCM_MPLS_PORT_MATCH_NONE;
        } else if (sal_strcasecmp(match_str, "port") == 0) {
            info.criteria = BCM_MPLS_PORT_MATCH_PORT;
        } else if (sal_strcasecmp(match_str, "port-vlan") == 0) {
            info.criteria = BCM_MPLS_PORT_MATCH_PORT_VLAN;
        } else if (sal_strcasecmp(match_str, "port-svlan") == 0) {
            info.criteria = BCM_MPLS_PORT_MATCH_PORT_VLAN_STACKED;
        } else if (sal_strcasecmp(match_str, "label") == 0) {
            info.criteria = BCM_MPLS_PORT_MATCH_LABEL;
        } else if (sal_strcasecmp(match_str, "port-label") == 0) {
            info.criteria = BCM_MPLS_PORT_MATCH_LABEL_PORT;
        } else if (sal_strcasecmp(match_str, "label-vlan") == 0) {
            info.criteria = BCM_MPLS_PORT_MATCH_LABEL_VLAN;
        } else {
            parse_arg_eq_done(&pt);
            cli_out("%s ERROR: Invalid action %s", FUNCTION_NAME(), match_str);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);
       
        info.match_vlan       = match_vlan;
        info.match_inner_vlan = match_inner_vlan; 

        rv = bcm_mpls_port_add(unit, vpn, &info);
        if (rv == BCM_E_NONE) {
            cli_out("mpls vpn(0x%x) port added: mpls-port-id(0x%x), encap-id(0x%x)\n",
                    vpn, info.mpls_port_id, info.encap_id);
        } else {
            cli_out("error(%s) adding port to vpn(0x%x)\n", bcm_errmsg(rv), vpn);
        }
        
    } else if (sal_strcasecmp(subcmd, "delete") == 0) {
        parse_table_add(&pt, "vpn",              PQ_DFL|PQ_HEX, 0, &vpn_int,          NULL);
        parse_table_add(&pt, "mpls-port-id",     PQ_DFL|PQ_HEX, 0, &mpls_port_id,     NULL);
        if (!parse_arg_eq(args, &pt)) {
            cli_out("%s: Unknown options: %s\n",
                    ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }
        parse_arg_eq_done(&pt);

        if (vpn_int > 0xFFFF) {
            return (CMD_FAIL);
        }
        vpn = vpn_int;
        
        rv = bcm_mpls_port_delete(unit, vpn, mpls_port_id);
        if (rv == BCM_E_NONE) {
            cli_out("mpls-port-id(0x%x) deleted from vpn(0x%x)\n",
                    mpls_port_id, vpn);
        } else {
            cli_out("error(%s) deleting mpls-port-id(0x%x) from vpn(0x%x)\n",
                    bcm_errmsg(rv), mpls_port_id, vpn);
        }
    } else if (sal_strcasecmp(subcmd, "delete-all") == 0) {
        parse_table_add(&pt, "vpn",              PQ_DFL|PQ_HEX, 0, &vpn_int,              NULL);
        if (!parse_arg_eq(args, &pt)) {
            cli_out("%s: Unknown options: %s\n",
                    ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }
        parse_arg_eq_done(&pt);

        if (vpn_int > 0xFFFF) {
            return (CMD_FAIL);
        }
        vpn = vpn_int;
        
        rv = bcm_mpls_port_delete_all(unit, vpn);
        if (rv == BCM_E_NONE) {
            cli_out("all ports successfully deleted from vpn(0x%x)\n",
                    vpn);
        } else {
            cli_out("error(%s) deleting all ports from vpn(0x%x)\n",
                    bcm_errmsg(rv), vpn);
        }
    } else if (sal_strcasecmp(subcmd, "get") == 0) {
        int port=0, module=0;
        info.mpls_port_id = 0;

        parse_table_add(&pt, "vpn",              PQ_DFL|PQ_HEX, 0, &vpn_int,               NULL);
        parse_table_add(&pt, "mpls-port-id",     PQ_DFL|PQ_HEX, 0, &info.mpls_port_id,     NULL);
        parse_table_add(&pt, "match-criteria",   PQ_DFL|PQ_STRING, 0, &match_str, NULL);
        parse_table_add(&pt, "match-vlan",   PQ_DFL|PQ_HEX, 0, &info.match_vlan, NULL);
        parse_table_add(&pt, "match-label",   PQ_DFL|PQ_HEX, 0, &info.match_label, NULL);
        parse_table_add(&pt, "port",   PQ_DFL|PQ_HEX, 0, &port, NULL);
        parse_table_add(&pt, "module",   PQ_DFL|PQ_HEX, 0, &module, NULL);

        if (!parse_arg_eq(args, &pt)) {
            cli_out("%s: Unknown options: %s\n",
                    ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        if (sal_strcasecmp(match_str, "port") == 0) {
            info.criteria = BCM_MPLS_PORT_MATCH_PORT;
        } else if (sal_strcasecmp(match_str, "port-vlan") == 0) {
            info.criteria = BCM_MPLS_PORT_MATCH_PORT_VLAN;
        } else if (sal_strcasecmp(match_str, "label") == 0) {
            info.criteria = BCM_MPLS_PORT_MATCH_LABEL;
        } else {
            parse_arg_eq_done(&pt);
            cli_out("%s ERROR: Invalid action %s", FUNCTION_NAME(), match_str);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

        if (vpn_int > 0xFFFF) {
            return (CMD_FAIL);
        }

        vpn = vpn_int;
        BCM_GPORT_MODPORT_SET(info.port, module, port);

        rv = bcm_mpls_port_get(unit, vpn, &info);
        if (rv == BCM_E_NONE) {
            cli_out("mpls-port-id(0x%x) successfully got from vpn(0x%x)\n",
                    info.mpls_port_id, vpn);
            SBX_DISPLAY_MPLS_PORT(&info);
        } else {
            cli_out("error(%s) port-get for mpls-port-id(0x%x) in vpn(0x%x)\n",
                    bcm_errmsg(rv), mpls_port_id, vpn);
        }
    } else if (sal_strcasecmp(subcmd, "get-all") == 0) {
        parse_table_add(&pt, "vpn",              PQ_DFL|PQ_HEX, 0, &vpn_int,                   NULL);
        if (!parse_arg_eq(args, &pt)) {
            cli_out("%s: Unknown options: %s\n",
                    ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }
        parse_arg_eq_done(&pt);

        if (vpn_int > 0xFFFF) {
            return (CMD_FAIL);
        }

        vpn = vpn_int;
        
        all_ports = sal_alloc(sizeof(bcm_mpls_port_t) * SBX_MAX_PORTS, "tmp-mpls-ports");
        if (all_ports == NULL) {
            return CMD_FAIL;
        }
        
        port_count = 0;
        rv = bcm_mpls_port_get_all(unit, vpn, SBX_MAX_PORTS, all_ports, &port_count);
        if (rv == BCM_E_NONE) {
            cli_out("successfully got %d entries from vpn(0x%x)\n",
                    port_count, vpn);
            for (i = 0; i < port_count; i++) {
                SBX_DISPLAY_MPLS_PORT(&all_ports[i]);
            }
        } else {
            cli_out("error(%s) port-get-all for vpn(0x%x)\n",
                    bcm_errmsg(rv), vpn);
        }
        sal_free(all_ports);
    } else {
        cli_out("%s ERROR: Invalid option %s", FUNCTION_NAME(), subcmd);
        return CMD_FAIL;
    }
    
    return (rv == BCM_E_NONE) ? CMD_OK : CMD_FAIL;
}

 
STATIC cmd_result_t
_cmd_sbx_mpls_vpls_fo_test(int unit, args_t *args)
{
    int                      rv;
    parse_table_t            pt;
    char	            *subcmd;
    static int               isConfigured = 0;
    static int               isFoSwitched = 0;
    bcm_mpls_tunnel_switch_t info;
    static int               ac_port = 0;
    static int               pw_port = 1;
    static int               pw_fo_port = 2;
    bcm_mpls_vpn_config_t    vpn_config;
    static int               mc_flags = 0;
    static int               bcastid = 0;
    bcm_mpls_port_t          acport, pwport, foport;
    int                      femod, i;
    static int               vpnid;
    bcm_l3_egress_t          l3eg, l3eg_fo;
    static bcm_l3_intf_t     l3intf, l3intf_fo;
    static bcm_if_t          l3egid, l3egid_fo;
    bcm_mpls_egress_label_t  label_array;
    sal_mac_addr_t           _mac_addr;
    static bcm_failover_t    foid;
    static bcm_if_t          encap_id[256]; 
    static bcm_if_t          encap_id_fo[256]; 
    static bcm_gport_t       fabric_gport[256];
    static bcm_gport_t       fabric_gport_fo[256];
    static int               qe_unit = 0;
    bcm_gport_t              *fg_add, *fg_delete;
    bcm_if_t                 *encap_add, *encap_delete;
    bcm_mpls_label_t         pw_label = 0x20000;
    sal_usecs_t               t;

    if ((subcmd = ARG_GET(args)) == NULL) {
        cli_out("%s ERROR: empty cmd\n", FUNCTION_NAME());
        return CMD_FAIL;
    }

    rv             = CMD_OK;
    sal_memset(&info, 0, sizeof(info));
    parse_table_init(unit, &pt);

    rv = bcm_stk_modid_get(unit, &femod);
    if (rv != BCM_E_NONE) {
        cli_out("Modid get failed: error (%s)\n", bcm_errmsg(rv));
    }
    if (sal_strcasecmp(subcmd, "config") == 0) {
        if (isConfigured) {
            cli_out("Test already configured");
            return CMD_FAIL;
        }
        /*  Create Multicast entry in QE */
        mc_flags = BCM_MULTICAST_TYPE_VPLS | BCM_MULTICAST_DISABLE_SRC_KNOCKOUT;
        rv = bcm_multicast_create(qe_unit, mc_flags, &bcastid); 
        if (rv != BCM_E_NONE) {
            cli_out("Multicast create failed: error (%s)\n", bcm_errmsg(rv));
        }
        /*  Create VPLS VPN */
        bcm_mpls_vpn_config_t_init(&vpn_config);
        vpn_config.vpn = 0;
        vpn_config.broadcast_group = bcastid;
        vpn_config.unknown_unicast_group = bcastid;
        vpn_config.unknown_multicast_group = bcastid;
        vpn_config.lookup_id = 0;

        vpn_config.flags = vpn_config.flags | BCM_MPLS_VPN_VPLS;
        rv = bcm_mpls_vpn_id_create(unit, &vpn_config);
        if (rv != BCM_E_NONE) {
            cli_out("VPN Id create failed: error (%s)\n", bcm_errmsg(rv));
        }
        vpnid = vpn_config.vpn;
        /* Add AC port */
        bcm_mpls_port_t_init(&acport);
        BCM_GPORT_MODPORT_SET(acport.port, femod, ac_port);
        acport.criteria = BCM_MPLS_PORT_MATCH_PORT_VLAN;
        acport.flags    = BCM_MPLS_PORT_CONTROL_WORD;
        acport.match_vlan = 0x123;
        acport.policer_id = 0;
        rv = bcm_mpls_port_add(unit, vpnid, &acport);
        if (rv != BCM_E_NONE) {
            cli_out("mpls port add - ac failed: error (%s)\n", bcm_errmsg(rv));
        }


    
       /*  4. Create failover LSP */
        bcm_l3_intf_t_init(&l3intf_fo);
        _mac_addr[0] = 0x0;
        _mac_addr[1] = 0x0;
        _mac_addr[2] = 0x1;
        _mac_addr[3] = 0x0;
        _mac_addr[4] = 0x0;
        _mac_addr[5] = 0x5;
        sal_memcpy(l3intf_fo.l3a_mac_addr, _mac_addr, 6);
        l3intf_fo.l3a_vid = 0xfff;
        l3intf_fo.l3a_ttl = 0x5;
        rv = bcm_l3_intf_create(unit, &l3intf_fo);
        if (rv != BCM_E_NONE) {
            cli_out("Interface create failed: error (%s)\n", bcm_errmsg(rv));
        }

        bcm_l3_egress_t_init(&l3eg_fo);
        l3eg_fo.intf = l3intf_fo.l3a_intf_id;
        _mac_addr[2] = 0xc;
        sal_memcpy(l3eg_fo.mac_addr, _mac_addr, 6);
        l3eg_fo.module = femod;
        l3eg_fo.port = pw_fo_port;
        rv = bcm_l3_egress_create(unit, 0, &l3eg_fo, &l3egid_fo);
        if (rv != BCM_E_NONE) {
            cli_out("l3egress create failed: error (%s)\n", bcm_errmsg(rv));
        }

        bcm_mpls_egress_label_t_init(&label_array);
        label_array.flags = BCM_MPLS_EGRESS_LABEL_TTL_DECREMENT | BCM_MPLS_EGRESS_LABEL_EXP_SET;
        label_array.exp = 3;
        label_array.label = 0xa0000;
        rv = bcm_mpls_tunnel_initiator_set(unit, l3intf_fo.l3a_intf_id, 1, &label_array);
        if (rv != BCM_E_NONE) {
            cli_out("Tunnel initiator set failed: error (%s)\n", bcm_errmsg(rv));
        }

       /* Create failover id */
       rv = bcm_failover_create(unit, 0, &foid);
        if (rv != BCM_E_NONE) {
            cli_out("Failover create failed: error (%s)\n", bcm_errmsg(rv));
        }

       /* Create LSP for working */
        bcm_l3_intf_t_init(&l3intf);
        _mac_addr[0] = 0x0;
        _mac_addr[1] = 0x0;
        _mac_addr[2] = 0x0;
        _mac_addr[3] = 0x0;
        _mac_addr[4] = 0x0;
        _mac_addr[5] = 0x5;
        sal_memcpy(l3intf.l3a_mac_addr, _mac_addr, 6);
        l3intf.l3a_vid = 0xfff;
        l3intf.l3a_ttl = 0x5;
        rv = bcm_l3_intf_create(unit, &l3intf);
        if (rv != BCM_E_NONE) {
            cli_out("Interface create failed: error (%s)\n", bcm_errmsg(rv));
        }

        bcm_l3_egress_t_init(&l3eg);
        l3eg.intf = l3intf.l3a_intf_id;
        _mac_addr[2] = 0xb;
        sal_memcpy(l3eg.mac_addr, _mac_addr, 6);
        l3eg.module = femod;
        l3eg.port = pw_port;
        l3eg.failover_id = foid;
        l3eg.failover_if_id = l3egid_fo;
        rv = bcm_l3_egress_create(unit, 0, &l3eg, &l3egid);
        if (rv != BCM_E_NONE) {
            cli_out("l3egress create failed: error (%s)\n", bcm_errmsg(rv));
        }

        bcm_mpls_egress_label_t_init(&label_array);
        label_array.flags = BCM_MPLS_EGRESS_LABEL_TTL_DECREMENT | BCM_MPLS_EGRESS_LABEL_EXP_SET;
        label_array.exp = 3;
        label_array.label = 0x10000;
        rv = bcm_mpls_tunnel_initiator_set(unit, l3intf.l3a_intf_id, 1, &label_array);
        if (rv != BCM_E_NONE) {
            cli_out("Tunnel initiator set failed: error (%s)\n", bcm_errmsg(rv));
        }


        for (i=0 ; i< 256; i++) {

            /* Add failover PW */

            bcm_mpls_egress_label_t_init(&label_array);
            label_array.flags = BCM_MPLS_EGRESS_LABEL_TTL_DECREMENT | BCM_MPLS_EGRESS_LABEL_EXP_SET;
            label_array.exp = 3;
            label_array.label = pw_label;

            bcm_mpls_port_t_init(&foport);
            BCM_GPORT_MODPORT_SET(foport.port, femod, pw_fo_port);
            foport.criteria = BCM_MPLS_PORT_MATCH_LABEL;
            foport.match_label = pw_label;
            foport.policer_id = 0;
            foport.flags = BCM_MPLS_PORT_EGRESS_TUNNEL | BCM_MPLS_PORT_CONTROL_WORD \
                         | BCM_MPLS_PORT_NETWORK | BCM_MPLS_PORT_FAILOVER;
            foport.egress_tunnel_if = l3egid_fo;
            foport.egress_label = label_array;
            rv = bcm_mpls_port_add(unit, vpnid, &foport);
            if (rv != BCM_E_NONE) {
                cli_out("mpls port add pw-fo failed: error (%s)\n", bcm_errmsg(rv));
            }

            /* Add working PW */
            bcm_mpls_egress_label_t_init(&label_array);
            label_array.flags = BCM_MPLS_EGRESS_LABEL_TTL_DECREMENT | BCM_MPLS_EGRESS_LABEL_EXP_SET;
            label_array.exp = 3;
            label_array.label = pw_label;

            bcm_mpls_port_t_init(&pwport);
            BCM_GPORT_MODPORT_SET(pwport.port, femod, pw_port);
            pwport.criteria = BCM_MPLS_PORT_MATCH_LABEL;
            pwport.match_label = pw_label;
            pwport.policer_id = 0;
            pwport.flags = BCM_MPLS_PORT_EGRESS_TUNNEL | BCM_MPLS_PORT_CONTROL_WORD \
                          | BCM_MPLS_PORT_NETWORK;
            pwport.egress_tunnel_if = l3egid;
            pwport.egress_label = label_array;
            pwport.failover_id = foid;
            pwport.failover_port_id = foport.mpls_port_id;

            rv = bcm_mpls_port_add(unit, vpnid, &pwport);
            if (rv != BCM_E_NONE) {
                cli_out("mpls port add pw failed: error (%s)\n", bcm_errmsg(rv));
            }

            rv = bcm_multicast_vpls_encap_get(unit, bcastid, pwport.port, pwport.mpls_port_id, &encap_id[i]);
            if (rv != BCM_E_NONE) {
                cli_out("multicast vpls encap get failed: error (%s)\n", bcm_errmsg(rv));
            }
            rv = bcm_multicast_vpls_encap_get(unit, bcastid, foport.port, foport.mpls_port_id, &encap_id_fo[i]);
            if (rv != BCM_E_NONE) {
                cli_out("multicast vpls encap get failed: error (%s)\n", bcm_errmsg(rv));
            }
            rv = bcm_stk_fabric_map_get(unit, pwport.port, &fabric_gport[i]);
            if (rv != BCM_E_NONE) {
                cli_out("stk fabric map pw get failed: error (%s)\n", bcm_errmsg(rv));
            }
            rv = bcm_stk_fabric_map_get(unit, foport.port, &fabric_gport_fo[i]);
            if (rv != BCM_E_NONE) {
                cli_out("stk fabric map get failed: error (%s)\n", bcm_errmsg(rv));
            }
            rv = bcm_multicast_egress_add(qe_unit, bcastid, fabric_gport[i], encap_id[i]);
            if (rv != BCM_E_NONE) {
                cli_out("multicast egress add failed: error (%s)\n", bcm_errmsg(rv));
            }
            pw_label++;
      }
      /*   7. Store the configuraton details, vpn-id, mvt entry, failover id
     */

        isConfigured = 1;
    } else if (sal_strcasecmp(subcmd, "fo-switch") == 0) {
        if (isConfigured == 0) {
            cli_out("Test is not configured\n");
            return CMD_FAIL;
        }
        t = sal_time_usecs();
        if (isFoSwitched == 0) {
            bcm_failover_set(unit, foid, 1);
            fg_delete    = fabric_gport;
            fg_add       = fabric_gport_fo;
            encap_add    = encap_id_fo;
            encap_delete = encap_id;
        } else {
            bcm_failover_set(unit, foid, 0);
            fg_add       = fabric_gport;
            fg_delete    = fabric_gport_fo;
            encap_delete = encap_id_fo;
            encap_add    = encap_id;
        }
    
        for (i = 0; i < 256; i++) {
            rv = bcm_multicast_egress_delete(qe_unit, bcastid, fg_delete[i], encap_delete[i]);
            if (rv != BCM_E_NONE) {
                cli_out("multicast egress delete failed: error (%s)\n", bcm_errmsg(rv));
            }
            rv = bcm_multicast_egress_add(qe_unit, bcastid, fg_add[i], encap_add[i]);
            if (rv != BCM_E_NONE) {
                cli_out("multicast egress add failed: error (%s)\n", bcm_errmsg(rv));
            }
            
        }
        isFoSwitched = isFoSwitched ? 0:1;
        cli_out("fo switch took %u us ", SAL_USECS_SUB(sal_time_usecs(), t));

        
    } else if (sal_strcasecmp(subcmd, "unconfig") == 0) {
        if (isConfigured == 0) {
            cli_out("Test is not configured\n");
            return CMD_FAIL;
        }
        rv = bcm_multicast_destroy(qe_unit, bcastid);
        if (rv != BCM_E_NONE) {
            cli_out("multicast destroy failed: error (%s)\n", bcm_errmsg(rv));
        }
        rv = bcm_mpls_port_delete_all(unit, vpnid);
        if (rv != BCM_E_NONE) {
            cli_out("mpls port delete all - failed: error (%s)\n", bcm_errmsg(rv));
        }
        rv = bcm_mpls_vpn_id_destroy(unit, vpnid);
        if (rv != BCM_E_NONE) {
            cli_out("mpls vpnid destroy failed: error (%s)\n", bcm_errmsg(rv));
        }
        rv = bcm_mpls_tunnel_initiator_clear(unit, l3intf.l3a_intf_id);
        if (rv != BCM_E_NONE) {
            cli_out("Tunnel initiator clear failed: error (%s)\n", bcm_errmsg(rv));
        }
        rv = bcm_mpls_tunnel_initiator_clear(unit, l3intf_fo.l3a_intf_id);
        if (rv != BCM_E_NONE) {
            cli_out("Tunnel initiator clear failed: error (%s)\n", bcm_errmsg(rv));
        }
        rv = bcm_l3_egress_destroy(unit, l3egid);
        if (rv != BCM_E_NONE) {
            cli_out("L3 egress destroy failed: error (%s)\n", bcm_errmsg(rv));
        }
        rv = bcm_l3_egress_destroy(unit, l3egid_fo);
        if (rv != BCM_E_NONE) {
            cli_out("L3 egress destroy failed: error (%s)\n", bcm_errmsg(rv));
        }
        rv = bcm_l3_intf_delete(unit, &l3intf);
        if (rv != BCM_E_NONE) {
            cli_out("L3 intf delete failed: error (%s)\n", bcm_errmsg(rv));
        }
        rv = bcm_l3_intf_delete(unit, &l3intf_fo);
        if (rv != BCM_E_NONE) {
            cli_out("L3 intf delete failed: error (%s)\n", bcm_errmsg(rv));
        }
        isConfigured = 0;
        isFoSwitched = 0;
    } else if (sal_strcasecmp(subcmd, "show") == 0) {
        if (isConfigured == 0) {
            cli_out("Test is not configured\n");
            return CMD_FAIL;
        }
        cli_out("Working : L3 Intf: %x, Egress: %x\n", l3intf.l3a_intf_id, l3egid);
        cli_out("Failover: L3 Intf: %x, Egress: %x\n", l3intf_fo.l3a_intf_id, l3egid_fo);
        cli_out("Vpn Id  : %x, Multicast group: %x\n", vpnid, bcastid);
        cli_out("Failover Id: %x\n", foid);
    } else {
        parse_arg_eq_done(&pt);
        cli_out("%s ERROR: Invalid option %s", FUNCTION_NAME(), subcmd);
        return CMD_FAIL;
    }

    /* coverity [mixed_enums] */
    if (rv == BCM_E_NONE) {
        rv = CMD_OK;
        cli_out("Command Successfull\n");
    } else {
        rv = CMD_FAIL;
        cli_out("Command failed\n");
    }
    
    return rv;
}

   
cmd_result_t
cmd_sbx_mpls(int unit, args_t *args)
{
    return subcommand_execute(unit, args, 
                              _cmd_sbx_mpls_list,
                              COUNTOF(_cmd_sbx_mpls_list));
}

        
/*------------------------------------------------
gdb bcm.user.dbg
b _bcm_fe2000_mpls_tunnel_switch_update
b bcm_fe2000_mpls_tunnel_switch_add
b bcm_fe2000_mpls_vpn_id_create
b bcm_fe2000_mpls_port_add
b bcm_fe2000_mpls_port_delete
b _bcm_fe2000_mpls_vpls_encap_get
handle SIG32 noprint nostop
run

  Test for L1 Ports

  # create vpn and check for proper table
  # programming
  # verify that vlan->dontlearn is set
  # routedvlan->vrf is drop-vrf fte->drop is set
  mpls vpn-id create type=PWE
  gu2get vlan2etc vlan 0xfffe
  gu2get routedvlan2etc idx 0xfffe
  gu2get fte idx 0xfffe

  # get out modid
  tcl
  set modid -1
  bcm_stk_my_modid_get 1 modid
  puts $modid
  exit
 
  # Simple case P (1) <---> P (2) 
  mpls vpn-port add vpn=0xfffe port=1 module=0 match-criteria=PORT
  mpls vpn-port add vpn=0xfffe port=2 module=0 match-criteria=PORT

  gu2get port2etc port 1
  gu2get port2etc port 2
  gu2get pvid2etc port 1 vid 1
  # error out adding > 2 ports to PWE
  mpls vpn-port add vpn=0xfffe port=3 module=0 match-criteria=PORT

  # delete port
  mpls vpn-port delete vpn=0xfffe port=1 module=0 match-criteria=PORT
  mpls vpn-port delete vpn=0xfffe port=2 module=0 match-criteria=PORT


  # Create all flavors of L1 Port PWE
  # Case P,Vid (1,100) <---> P,Vid (1,200)
  mpls vpn-port add vpn=0xfffe port=1 match-vlan=100 module=0 match-criteria=PORT-VLAN
  mpls vpn-port add vpn=0xfffe port=1 match-vlan=200 module=0 match-criteria=PORT-VLAN
  gu2get pvid2etc port 1 vid 100
  gu2get vlan2etc vlan 0x3000
  gu2get routedvlan2etc idx 0x3000
  gu2get fte idx 0x3000
  gu2get pvid2etc port 1 vid 200
  gu2get vlan2etc vlan 0x2fff
  gu2get routedvlan2etc idx 0x2fff
  gu2get fte idx 0x2fff
  ------------------------------------------------*/
