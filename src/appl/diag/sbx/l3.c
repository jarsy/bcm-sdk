/* $Id: l3.c,v 1.18 Broadcom SDK $
 * $Id: 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        l3.c
 * Purpose:     L3 CLI commands
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

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#ifdef BCM_FE2000_SUPPORT
#include <soc/sbx/fe2000.h>
#endif
#ifdef BCM_QE2000_SUPPORT
#include <soc/sbx/qe2000_util.h>
#endif
#include <bcm_int/sbx/fe2000/allocator.h>
#ifdef BCM_FE2000_SUPPORT
#include <bcm_int/sbx/fe2000/l3.h>
#endif

STATIC cmd_result_t _cmd_sbx_l3_intf(int unit, args_t *args);
STATIC cmd_result_t _cmd_sbx_l3_egr(int unit, args_t *args);
STATIC cmd_result_t _cmd_sbx_l3_route(int unit, args_t *args);
#ifdef BCM_FE2000_SUPPORT
STATIC cmd_result_t _cmd_sbx_l3_mpath(int unit, args_t *args);
STATIC cmd_result_t _cmd_sbx_l3_mpls_tunnel(int unit, args_t *args);
#endif

#define CMD_L3_INTF_USAGE                                  \
    "    intf [add | update | delete | find | get ]\n"     \
    "         [SMAC=<mac>] [vid=<id>]\n"                   \
    "         [vrf=<vrf>] [Id=<ifid>]\n"                   \
    "         [ttl=<ttl>] [mtu=<mtu>]\n"                   

#define CMD_L3_EGR_USAGE                                    \
    "    egr  [add | update | delete | find | get ]\n"      \
    "         [Id=<ifid>] [DMAC=<mac>]\n"                   \
    "         [module=<modid>] [port=<port>]\n"             \
    "         [trunk=<tid>] [label=<label>]\n"              \
    "         [ohi=<ohi>] [fte=<fte>]\n"                    

#define CMD_L3_ROUTE_USAGE                                  \
    "    route [add | delete | update | get ]\n"            \
    "          [vrf=<vrf>] ( ip=<ipaddr> mask=<mask>) | \n" \
    "          (v6 ip6=<ipaddr> ip6mask=<mask> )\n"          \
    "          [fte=<fte>]\n"

#ifdef BCM_FE2000_SUPPORT
#define CMD_L3_MPATH_USAGE                                      \
    "    mpath [create | add | delete | destroy | get ]\n"      \
    "          [vrf=<vrf>] [fte1=<fte1>] [fte2=<fte2>]\n"       \
    "          [fte3=<fte3>] [fte4=<fte4>] [mpbase=<mpbase>]\n"

#define CMD_L3_MPLS_TUNNEL_USAGE                                        \
    "    mpls-tunnel [initiator-set | initiator-clear | get]\n"         \
    "                [Id=<ifid>]\n"                                     \
    "                [label1=<label1>] [ttl1=<ttl1>] [exp1=<exp1>] [ttlop1=<SET|COPY|DECR]\n"\
    "                [label2=<label2>] [ttl2=<ttl2>] [exp2=<exp2>] [ttlop2=<SET|COPY|DECR]\n"\
    "                [label3=<label3>] [ttl3=<ttl3>] [exp3=<exp3>] [ttlop3=<SET|COPY|DECR]\n"
#endif


static cmd_t _cmd_sbx_l3_list[] = {
    {"intf",        _cmd_sbx_l3_intf,         "\n" CMD_L3_INTF_USAGE, NULL},
    {"egr",         _cmd_sbx_l3_egr,          "\n" CMD_L3_EGR_USAGE, NULL},
    {"route",       _cmd_sbx_l3_route,        "\n" CMD_L3_ROUTE_USAGE, NULL},
#ifdef BCM_FE2000_SUPPORT
    {"mpath",       _cmd_sbx_l3_mpath,        "\n" CMD_L3_MPATH_USAGE, NULL},
    {"mpls-tunnel", _cmd_sbx_l3_mpls_tunnel,  "\n" CMD_L3_MPLS_TUNNEL_USAGE, NULL}
#endif
};

char cmd_sbx_l3_usage[] =
    "\n"
#ifdef COMPILER_STRING_CONST_LIMIT
    "    l3 <option> [args...]\n"
#else
    CMD_L3_INTF_USAGE         "\n"
    CMD_L3_EGR_USAGE          "\n"
    CMD_L3_ROUTE_USAGE        "\n"
#ifdef BCM_FE2000_SUPPORT
    CMD_L3_MPATH_USAGE        "\n"
    CMD_L3_MPLS_TUNNEL_USAGE  "\n"
#endif
#endif
    ;
/*
 * Local global variables to remember last values in arguments.
 */

static sal_mac_addr_t   _l3_macaddr = {0x0, 0x0, 0x0, 0x0, 0x0, 0x1};

static int _l3_vid = VLAN_ID_DEFAULT;

static int _l3_modid = 0, _l3_label, _l3_ohi, _l3_port, _l3_tgid = 0;
static int _l3_fte;
static int _l3_vrf = 0, _l3_ttl = 0,  _l3_ifid = 0, _l3_mtu = 0;




STATIC cmd_result_t
_cmd_sbx_l3_intf(int unit, args_t *args)
{
    cmd_result_t       retCode;
    int                rv;
    parse_table_t      pt;
    bcm_l3_intf_t      bcm_intf;
    char	      *subcmd;

    if ((subcmd = ARG_GET(args)) == NULL) {
        cli_out("%s ERROR: empty cmd\n", FUNCTION_NAME());
        return CMD_FAIL;
    }

    _l3_vid   = 0;
    _l3_ifid  = 0;
    _l3_vrf   = 0;
    _l3_ttl   = 0;
    _l3_mtu   = 0;

    rv        = BCM_E_NONE;
    sal_memset(_l3_macaddr, 0, 6);
    
    parse_table_init(unit, &pt);
    bcm_l3_intf_t_init(&bcm_intf);
    if ((sal_strcasecmp(subcmd, "add") == 0) ||
        (sal_strcasecmp(subcmd, "update") == 0)) {

        /* Parse command option arguments */
        parse_table_add(&pt, "SMAC", PQ_DFL|PQ_MAC, 0, &_l3_macaddr, NULL);
        parse_table_add(&pt, "vid", PQ_DFL|PQ_HEX, 0, &_l3_vid, NULL);
        parse_table_add(&pt, "Id",  PQ_DFL|PQ_HEX, 0, &_l3_ifid, NULL);
        parse_table_add(&pt, "vrf", PQ_DFL|PQ_INT, 0, &_l3_vrf, NULL);
        parse_table_add(&pt, "ttl", PQ_DFL|PQ_INT, 0, &_l3_ttl, NULL);
        parse_table_add(&pt, "mtu", PQ_DFL|PQ_INT, 0, &_l3_mtu, NULL);

        if (!parseEndOk(args, &pt, &retCode)) {
            return retCode;
        }

        if (sal_strcasecmp(subcmd, "update") == 0) {
            bcm_intf.l3a_flags   |= BCM_L3_REPLACE;
        }
        
        if (_l3_ifid) {
            bcm_intf.l3a_flags   |= BCM_L3_WITH_ID;
            bcm_intf.l3a_intf_id  = _l3_ifid;
        }

        if (_l3_vid) {
            bcm_intf.l3a_vid      = _l3_vid;
        }

        if (_l3_vrf) {
            bcm_intf.l3a_vrf      = _l3_vrf;
        }

        bcm_intf.l3a_ttl          = 2;
        if (_l3_ttl) {
            bcm_intf.l3a_ttl      = _l3_ttl;
        }

        bcm_intf.l3a_mtu         = 9216;
        if (_l3_mtu) {
            bcm_intf.l3a_mtu     = _l3_mtu;
        }
        sal_memcpy(bcm_intf.l3a_mac_addr,_l3_macaddr, 6);
    
        rv = bcm_l3_intf_create(unit, &bcm_intf);
        if (rv == BCM_E_NONE) {
            cli_out("Created if_id 0x%x\n", bcm_intf.l3a_intf_id);
        }
    } else if (sal_strcasecmp(subcmd, "delete") == 0) {
        parse_table_add(&pt, "SMAC", PQ_DFL|PQ_MAC, 0, &_l3_macaddr, NULL);
        parse_table_add(&pt, "vid", PQ_DFL|PQ_HEX, 0, &_l3_vid, NULL);
        parse_table_add(&pt, "Id",  PQ_DFL|PQ_HEX, 0, &_l3_ifid, NULL);
        parse_table_add(&pt, "vrf", PQ_DFL|PQ_INT, 0, &_l3_vrf, NULL);

        if (!parseEndOk(args, &pt, &retCode)) {
            return retCode;
        }
        
        if (_l3_ifid) {
            bcm_intf.l3a_flags   |= BCM_L3_WITH_ID;
            bcm_intf.l3a_intf_id  = _l3_ifid;
        } else {
            bcm_intf.l3a_vid      = _l3_vid;
            sal_memcpy(bcm_intf.l3a_mac_addr,
                       _l3_macaddr, 6);            
        }
        bcm_intf.l3a_vrf      = _l3_vrf;

        rv = bcm_l3_intf_delete(unit, &bcm_intf);
        if (rv == BCM_E_NONE) {
            if (_l3_ifid)
                cli_out("Deleted if_id 0x%x, %s\n",
                        bcm_intf.l3a_intf_id, bcm_errmsg(rv));
            else
                cli_out("Deleted vid=%d mac=0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x %s\n",
                        bcm_intf.l3a_vid,
                        bcm_intf.l3a_mac_addr[0],bcm_intf.l3a_mac_addr[1],
                        bcm_intf.l3a_mac_addr[2],bcm_intf.l3a_mac_addr[3],
                        bcm_intf.l3a_mac_addr[4],bcm_intf.l3a_mac_addr[5],
                        bcm_errmsg(rv));
        }
        
    } else if (sal_strcasecmp(subcmd, "find") == 0) {

        /* Parse command option arguments */
        parse_table_add(&pt, "SMAC", PQ_DFL|PQ_MAC, 0, &_l3_macaddr, NULL);
        parse_table_add(&pt, "vid", PQ_DFL|PQ_HEX, 0, &_l3_vid, NULL);

        if (!parseEndOk(args, &pt, &retCode)) {
            return retCode;
        }
        
        sal_memcpy(bcm_intf.l3a_mac_addr,_l3_macaddr, 6);
        bcm_intf.l3a_vid      = _l3_vid;
        rv = bcm_l3_intf_find(unit, &bcm_intf);
        if (rv == BCM_E_NONE) {
            cli_out("MacAddr=0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x "
                    "Id=0x%x, Vid=%d, vrf=%d, ttl=%d, mtu=%d\n",
                    bcm_intf.l3a_mac_addr[0],bcm_intf.l3a_mac_addr[1],
                    bcm_intf.l3a_mac_addr[2],bcm_intf.l3a_mac_addr[3],
                    bcm_intf.l3a_mac_addr[4],bcm_intf.l3a_mac_addr[5],
                    bcm_intf.l3a_intf_id, bcm_intf.l3a_vid,
                    bcm_intf.l3a_vrf, bcm_intf.l3a_ttl, bcm_intf.l3a_mtu);
        } else {
            cli_out("MacAddr=0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x "
                    "Vid=%d bcm_l3_intf_find() failure: %s\n",
                    bcm_intf.l3a_mac_addr[0],bcm_intf.l3a_mac_addr[1],
                    bcm_intf.l3a_mac_addr[2],bcm_intf.l3a_mac_addr[3],
                    bcm_intf.l3a_mac_addr[4],bcm_intf.l3a_mac_addr[5],
                    bcm_intf.l3a_vid, bcm_errmsg(rv));
        }
    } else if (sal_strcasecmp(subcmd, "get") == 0) {  
        parse_table_add(&pt, "Id",  PQ_DFL|PQ_HEX, 0, &_l3_ifid, NULL);

        if (!parseEndOk(args, &pt, &retCode)) {
            return retCode;
        }
        
        bcm_intf.l3a_flags   |= BCM_L3_WITH_ID;
        bcm_intf.l3a_intf_id  = _l3_ifid;
        rv = bcm_l3_intf_get(unit, &bcm_intf);
        if (rv == BCM_E_NONE) {
            cli_out("MacAddr=0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x "
                    "Id=0x%x, Vid=%d, vrf=%d, ttl=%d, mtu=%d\n",
                    bcm_intf.l3a_mac_addr[0],bcm_intf.l3a_mac_addr[1],
                    bcm_intf.l3a_mac_addr[2],bcm_intf.l3a_mac_addr[3],
                    bcm_intf.l3a_mac_addr[4],bcm_intf.l3a_mac_addr[5],
                    bcm_intf.l3a_intf_id, bcm_intf.l3a_vid,
                    bcm_intf.l3a_vrf, bcm_intf.l3a_ttl, bcm_intf.l3a_mtu);
        } else {
            cli_out("MacAddr=0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x "
                    "Vid=%d bcm_l3_intf_get() failure: %s\n",
                    bcm_intf.l3a_mac_addr[0],bcm_intf.l3a_mac_addr[1],
                    bcm_intf.l3a_mac_addr[2],bcm_intf.l3a_mac_addr[3],
                    bcm_intf.l3a_mac_addr[4],bcm_intf.l3a_mac_addr[5],
                    bcm_intf.l3a_vid, bcm_errmsg(rv));
        }
    } else {
        cli_out("%s ERROR: Invalid option %s\n", FUNCTION_NAME(), subcmd);
	parse_arg_eq_done(&pt);
        return CMD_FAIL;
    }

    return (rv == BCM_E_NONE) ? CMD_OK : CMD_FAIL;
}


STATIC cmd_result_t
_cmd_sbx_l3_egr(int unit, args_t *args)
{
    cmd_result_t       retCode;
    int                rv;
    parse_table_t      pt;
    bcm_l3_egress_t    bcm_egr;
    char	      *subcmd;
    bcm_if_t           fte_idx;
    uint32             flags;
    
    if ((subcmd = ARG_GET(args)) == NULL) {
        cli_out("%s ERROR: empty cmd\n", FUNCTION_NAME());
        return CMD_FAIL;
    }

    flags          = 0;
    _l3_ifid       = 0;
    _l3_tgid       = 0;
    _l3_modid      = 0;
    _l3_port       = 0;
    _l3_ohi        = 0;
    _l3_label      = 0;
    _l3_fte        = 0;
    rv             = BCM_E_NONE;
    sal_memset(_l3_macaddr, 0, 6);
    
    parse_table_init(unit, &pt);
    bcm_l3_egress_t_init(&bcm_egr);
    
    if ((sal_strcasecmp(subcmd, "add") == 0) ||
        (sal_strcasecmp(subcmd, "update") == 0)) {

        /* Parse command option arguments */
        parse_table_add(&pt, "Id",     PQ_DFL|PQ_HEX, 0, &_l3_ifid, NULL);
        parse_table_add(&pt, "DMAC",   PQ_DFL|PQ_MAC, 0, &_l3_macaddr, NULL);
        parse_table_add(&pt, "module", PQ_DFL|PQ_INT, 0, &_l3_modid, NULL);
        parse_table_add(&pt, "port",   PQ_DFL|PQ_INT, 0, &_l3_port, NULL);
        parse_table_add(&pt, "trunk",  PQ_DFL|PQ_INT, 0, &_l3_tgid, NULL);
        parse_table_add(&pt, "label",  PQ_DFL|PQ_HEX, 0, &_l3_label, NULL);
        parse_table_add(&pt, "ohi",    PQ_DFL|PQ_HEX, 0, &_l3_ohi, NULL);
        parse_table_add(&pt, "fte",    PQ_DFL|PQ_HEX, 0, &_l3_fte, NULL);
        if (!parseEndOk(args, &pt, &retCode)) {
            return retCode;
        }

        if (sal_strcasecmp(subcmd, "update") == 0) {
            flags = bcm_egr.flags = BCM_L3_REPLACE;
        }
        
        bcm_egr.intf              = _l3_ifid;
        sal_memcpy(bcm_egr.mac_addr,_l3_macaddr, 6);
        bcm_egr.module            = _l3_modid;
        bcm_egr.port              = _l3_port;
        bcm_egr.trunk             = _l3_tgid;
        bcm_egr.mpls_label        = _l3_label;
        bcm_egr.encap_id          = _l3_ohi;
        
        rv = bcm_l3_egress_create(unit, flags, &bcm_egr, &fte_idx);
        if (rv == BCM_E_NONE) {
            cli_out("Created fte 0x%x ohi=0x%x\n", fte_idx, bcm_egr.encap_id);
        }
    } else if (sal_strcasecmp(subcmd, "get") == 0) {
        parse_table_add(&pt, "fte",    PQ_DFL|PQ_HEX, 0, &_l3_fte, NULL);
        
        if (!parseEndOk(args, &pt, &retCode)) {
            return retCode;
        }
        fte_idx = (uint32)_l3_fte;
        rv = bcm_l3_egress_get (unit,
                                fte_idx,
                                &bcm_egr) ;
        if (rv == BCM_E_NONE) {
            cli_out("\n\tDMAC=0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x "
                    "\n\tInterfaceId=0x%x, module=%d, port=%d, trunk=%d "
                    "\n\tlabel=0x%x ohi=0x%x\n",
                    bcm_egr.mac_addr[0],bcm_egr.mac_addr[1],
                    bcm_egr.mac_addr[2],bcm_egr.mac_addr[3],
                    bcm_egr.mac_addr[4],bcm_egr.mac_addr[5],
                    bcm_egr.intf, bcm_egr.module,
                    bcm_egr.port, bcm_egr.trunk, bcm_egr.mpls_label,
                    bcm_egr.encap_id);
        }
    } else {
        cli_out("%s ERROR: Invalid option %s\n", FUNCTION_NAME(), subcmd);
	parse_arg_eq_done(&pt);
        return CMD_FAIL;
    }

    return (rv == BCM_E_NONE) ? CMD_OK : CMD_FAIL;
}

STATIC cmd_result_t
_cmd_sbx_l3_route(int unit, args_t *args)
{
    cmd_result_t       retCode;
    int                rv;
    parse_table_t      pt;
    bcm_l3_route_t     bcm_route;
    char	      *subcmd;
    ip_addr_t          ipaddr, mask;
    ip6_addr_t         ip6addr, ip6mask;
    int                v6 = 0;
    
    if ((subcmd = ARG_GET(args)) == NULL) {
        cli_out("%s ERROR: empty cmd\n", FUNCTION_NAME());
        return CMD_FAIL;
    }

    _l3_fte        = 0;
    _l3_vrf        = 0;
    rv             = BCM_E_NONE;
    sal_memset(&ipaddr, 0, sizeof(ipaddr));
    sal_memset(&mask, 0, sizeof(mask));
    sal_memset(&ip6addr, 0, sizeof(ip6_addr_t));
    sal_memset(&ip6mask, 0, sizeof(ip6_addr_t));
    
    parse_table_init(unit, &pt);
    bcm_l3_route_t_init(&bcm_route);

    parse_table_add(&pt, "ip",      PQ_DFL | PQ_IP,  0, &ipaddr, NULL);
    parse_table_add(&pt, "ip6",     PQ_DFL | PQ_IP6, 0, &ip6addr, NULL);
    parse_table_add(&pt, "mask",    PQ_DFL | PQ_IP,  0, &mask,   NULL);
    parse_table_add(&pt, "ip6mask", PQ_DFL | PQ_IP6, 0, &ip6mask,   NULL);
    parse_table_add(&pt, "vrf",    PQ_DFL | PQ_INT, 0, &_l3_vrf, NULL);
    parse_table_add(&pt, "fte",    PQ_DFL | PQ_HEX, 0, &_l3_fte, NULL);
    parse_table_add(&pt, "v6",  PQ_DFL|PQ_BOOL|PQ_NO_EQ_OPT, 0, &v6, NULL);
        
    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    if (v6) {
        bcm_route.l3a_flags |= BCM_L3_IP6;
    }
    
    bcm_route.l3a_vrf     = _l3_vrf;
    bcm_route.l3a_subnet  = ipaddr;
    bcm_route.l3a_ip_mask = mask;
    sal_memcpy(&bcm_route.l3a_ip6_net, ip6addr, sizeof(ip6_addr_t));
    sal_memcpy(&bcm_route.l3a_ip6_mask, ip6mask, sizeof(ip6_addr_t));
    
    if (sal_strcasecmp(subcmd, "add") == 0) {
        bcm_route.l3a_intf    = _l3_fte;
        rv = bcm_l3_route_add(unit, &bcm_route);
        
        cli_out("Route add returned: %s\n", bcm_errmsg(rv));

    } else if (sal_strcasecmp(subcmd, "update") == 0) {
        bcm_route.l3a_intf    = _l3_fte;
        bcm_route.l3a_flags  |= BCM_L3_REPLACE;
        rv = bcm_l3_route_add(unit, &bcm_route);

        cli_out("Route update returned: %s\n", bcm_errmsg(rv));

    } else if (sal_strcasecmp(subcmd, "get") == 0) {
	bcm_route.l3a_intf    = _l3_fte;
        rv = bcm_l3_route_get(unit, &bcm_route);
        cli_out("Route get returned: %s\n", bcm_errmsg(rv));

    } else {
        cli_out("%s ERROR: Invalid option %s", FUNCTION_NAME(), subcmd);
        return CMD_FAIL;
    }
    
    return (rv == BCM_E_NONE) ? CMD_OK : CMD_FAIL;
}

#ifdef BCM_FE2000_SUPPORT
STATIC cmd_result_t
_cmd_sbx_l3_mpath(int unit, args_t *args)
{
    cmd_result_t       retCode;
    int                rv, count;
    parse_table_t      pt;
    uint32             i, flags;
    char	      *subcmd;
    bcm_if_t           mpbase, ftes[_FE2K_L3_ECMP_MAX];
    
    if ((subcmd = ARG_GET(args)) == NULL) {
        cli_out("%s ERROR: empty cmd\n", FUNCTION_NAME());
        return CMD_FAIL;
    }
    
    _l3_vrf        = 0;
    mpbase         = 0;
    count          = 0;
    flags          = 0;
    rv             = BCM_E_NONE;
    for (i = 0; i < _FE2K_L3_ECMP_MAX; i++) {
        ftes[i] = -1;
    }
        
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "vrf",    PQ_DFL | PQ_INT, 0, &_l3_vrf, NULL);
    parse_table_add(&pt, "fte1",   PQ_DFL | PQ_HEX, 0, &ftes[0], NULL);
    parse_table_add(&pt, "fte2",   PQ_DFL | PQ_HEX, 0, &ftes[1], NULL);
    parse_table_add(&pt, "fte3",   PQ_DFL | PQ_HEX, 0, &ftes[2], NULL);
    parse_table_add(&pt, "fte4",   PQ_DFL | PQ_HEX, 0, &ftes[3], NULL);
    parse_table_add(&pt, "mpbase", PQ_DFL | PQ_HEX, 0, &mpbase,  NULL);
    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    for (i = 0; i < _FE2K_L3_ECMP_MAX; i++) {
        if (ftes[i] != -1) {
            count++;
        }
    }
    
    if (sal_strcasecmp(subcmd, "create") == 0) {
        /*
         * Initial creation
         */
        rv = bcm_l3_egress_multipath_create(unit,
                                            flags,
                                            count,
                                            ftes,
                                            &mpbase);
        if (rv == BCM_E_NONE) {
            cli_out("Multipath add success: 0x%x\n", mpbase);
        }
    } else if (sal_strcasecmp(subcmd, "add") == 0) {
        /*
         * add a member to the set
         */
        for (i = 0; i < (_FE2K_L3_ECMP_MAX-1); i++) {
            if (ftes[i] != -1) {
                break;
            }
        }
        rv = bcm_l3_egress_multipath_add(unit,
                                         mpbase,
                                         ftes[i]);
        if (rv == BCM_E_NONE) {
            cli_out("multipath add successfull\n");
        }
    } else if (sal_strcasecmp(subcmd, "delete") == 0) {
        /*
         * remove a member from the set
         */
        for (i = 0; i < (_FE2K_L3_ECMP_MAX-1); i++) {
            if (ftes[i] != -1) {
                break;
            }
        }
        rv = bcm_l3_egress_multipath_delete(unit,
                                            mpbase,
                                            ftes[i]);
        if (rv == BCM_E_NONE) {
            cli_out("multipath delete successfull\n");
        }
    } else if (sal_strcasecmp(subcmd, "destroy") == 0) {
        /*
         * destroy the whole group
         */
        rv = bcm_l3_egress_multipath_destroy(unit,
                                             mpbase) ;
        if (rv == BCM_E_NONE) {
            cli_out("multipath destroy success\n");
        }
    } else if (sal_strcasecmp(subcmd, "get") == 0) {
        rv = bcm_l3_egress_multipath_get(unit,
                                         mpbase,
                                         _FE2K_L3_ECMP_MAX,
                                         ftes,
                                         &count);
        if (rv == BCM_E_NONE) {
            cli_out("multipath get successfull\n");
            for (i = 0; i < count; i++) {
                cli_out("fte[%d] = 0x%x\n", i, ftes[i]);
            }
        }
    } else {
        cli_out("%s ERROR: Invalid option %s", FUNCTION_NAME(), subcmd);
        return CMD_FAIL;
    }
    
    return (rv == BCM_E_NONE) ? CMD_OK : CMD_FAIL;
}

STATIC cmd_result_t
_cmd_sbx_l3_mpls_tunnel(int unit, args_t *args)
{
    cmd_result_t            retCode;
    int                     rv, count;
    parse_table_t           pt;
    uint32                  i;
    char	           *subcmd;
    bcm_mpls_egress_label_t label_array[_FE2K_MAX_MPLS_TNNL_LABELS];
    char                   *ttlop_str[_FE2K_MAX_MPLS_TNNL_LABELS];
    
    if ((subcmd = ARG_GET(args)) == NULL) {
        cli_out("%s ERROR: empty cmd\n", FUNCTION_NAME());
        return CMD_FAIL;
    }
    
    count          = 0;
    rv             = CMD_OK;
    _l3_ifid       = 0;
    for (i = 0; i < _FE2K_MAX_MPLS_TNNL_LABELS; i++) {
        sal_memset(&label_array[i], 0, sizeof(bcm_mpls_egress_label_t));
    }
        
    parse_table_init(unit, &pt);
    
    if (sal_strcasecmp(subcmd, "initiator-set") == 0) {
        parse_table_add(&pt, "Id",       PQ_DFL | PQ_HEX, 0, &_l3_ifid, NULL);
        parse_table_add(&pt, "label1",   PQ_DFL | PQ_HEX, 0, &label_array[0].label, NULL);
        parse_table_add(&pt, "ttl1",     PQ_DFL | PQ_INT, 0, &label_array[0].ttl, NULL);
        parse_table_add(&pt, "ttlop1",   PQ_STRING,       0, &ttlop_str[0],       NULL);
        parse_table_add(&pt, "exp1",     PQ_DFL | PQ_INT, 0, &label_array[0].exp, NULL);
        if (_FE2K_MAX_MPLS_TNNL_LABELS > 1) {
            parse_table_add(&pt, "label2",   PQ_DFL | PQ_HEX, 0, &label_array[1].label, NULL);
            parse_table_add(&pt, "ttl2",     PQ_DFL | PQ_INT, 0, &label_array[1].ttl, NULL);
            parse_table_add(&pt, "ttlop2",   PQ_STRING,       0, &ttlop_str[1],       NULL);
            parse_table_add(&pt, "exp2",     PQ_DFL | PQ_INT, 0, &label_array[1].exp, NULL);
            if (_FE2K_MAX_MPLS_TNNL_LABELS > 2) {
                parse_table_add(&pt, "label3",   PQ_DFL | PQ_HEX, 0, &label_array[2].label, NULL);
                parse_table_add(&pt, "ttl3",     PQ_DFL | PQ_INT, 0, &label_array[2].ttl, NULL);
                parse_table_add(&pt, "ttlop3",   PQ_STRING,       0, &ttlop_str[2],       NULL);
                parse_table_add(&pt, "exp3",     PQ_DFL | PQ_INT, 0, &label_array[2].exp, NULL);
            }
        }
        if (!parse_arg_eq(args, &pt)) {
            cli_out("%s: Unknown options: %s\n",
                    ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        count = 0;
        for (i = 0; i < _FE2K_MAX_MPLS_TNNL_LABELS; i++) {
            if (label_array[i].label) {
                count++;
                if (sal_strcasecmp(ttlop_str[i], "set") == 0) {
                    label_array[i].flags |= BCM_MPLS_EGRESS_LABEL_TTL_SET;
                } else if (sal_strcasecmp(ttlop_str[i], "copy") == 0) {
                    label_array[i].flags |= BCM_MPLS_EGRESS_LABEL_TTL_COPY;
                } else if (sal_strcasecmp(ttlop_str[i], "decr") == 0) {
                    label_array[i].flags |= BCM_MPLS_EGRESS_LABEL_TTL_DECREMENT;
                } else {
                    cli_out("ttlop (%s) is invalid, [set|copy|decr] needs to be provided\n", ttlop_str[i]);
                    parse_arg_eq_done(&pt);
                    return(CMD_FAIL);
                }
            } else {
                break;
            }
        }

        parse_arg_eq_done(&pt);
        rv = bcm_mpls_tunnel_initiator_set(unit,
                                           _l3_ifid,
                                           count,
                                           label_array);
        if (rv == BCM_E_NONE) {
            cli_out("mpls tunnel initiator set successfull\n");
        } 
    } else if (sal_strcasecmp(subcmd, "initiator-clear") == 0) {
        parse_table_add(&pt, "Id",       PQ_DFL | PQ_HEX, 0, &_l3_ifid, NULL);
        if (!parseEndOk(args, &pt, &retCode)) {
            return retCode;
        }
        rv = bcm_mpls_tunnel_initiator_clear(unit,
                                             _l3_ifid);
        if (rv == BCM_E_NONE) {
            cli_out("mpls tunnel initiator clear successfull\n");
        }
    } else if (sal_strcasecmp(subcmd, "initiator-get") == 0) {
        parse_table_add(&pt, "Id",       PQ_DFL | PQ_HEX, 0, &_l3_ifid, NULL);
        if (!parseEndOk(args, &pt, &retCode)) {
            return retCode;
        }        
        count = 0;

        rv = bcm_mpls_tunnel_initiator_get (unit,
                                            _l3_ifid,
                                            _FE2K_MAX_MPLS_TNNL_LABELS,
                                            label_array,
                                            &count);
        if (rv == BCM_E_NONE) {
            cli_out("mpls tunnel initiator get successfull\n");
            for (i = 0; i < count; i++) {
                cli_out("Label %d: value 0x%x ttl %d exp %d\n",
                        i, label_array[i].label,
                        label_array[i].ttl,
                        label_array[i].exp);
            }
        }
    } else {
        cli_out("%s ERROR: Invalid option %s\n", FUNCTION_NAME(), subcmd);
	parse_arg_eq_done(&pt);
        return CMD_FAIL;
    }
    if (rv != BCM_E_NONE) {
        return CMD_FAIL;
    }
    
    return CMD_OK;
}
#endif /* BCM_FE2000_SUPPORT */

cmd_result_t
cmd_sbx_l3(int unit, args_t *args)
{
    return subcommand_execute(unit, args, 
                              _cmd_sbx_l3_list, COUNTOF(_cmd_sbx_l3_list));
}

