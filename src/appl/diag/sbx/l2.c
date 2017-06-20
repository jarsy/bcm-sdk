/* 
 * $Id: l2.c,v 1.18 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        l2.c
 * Purpose:     L2 CLI commands
 *
 */
#include <shared/bsl.h>

#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#if defined(BCM_FE2000_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)

#include <soc/sbx/sbx_drv.h>
#include <bcm_int/sbx/l2.h>
#include <bcm/error.h>
#include <bcm/l2.h>
#include <bcm/debug.h>

STATIC cmd_result_t _cmd_sbx_l2_add(int unit, args_t *args);
STATIC cmd_result_t _cmd_sbx_l2_clear(int unit, args_t *args);
STATIC cmd_result_t _cmd_sbx_l2_del(int unit, args_t *args);
STATIC cmd_result_t _cmd_sbx_l2_init(int unit, args_t *args);
STATIC cmd_result_t _cmd_sbx_l2_redirect(int unit, args_t *args);
STATIC cmd_result_t _cmd_sbx_l2_show(int unit, args_t *args);


#define CMD_L2_USAGE_ADD \
    "    l2 add [MACaddress=<mac>] [Vlanid=<id>]\n" \
    "           [Module=<n>] [PortBitMap=<pbmp>]\n" \
    "           [Trunk=true|false] [TrunkGroupId=<id>]\n" \
    "           [DiscardSource=true|false] [DiscardDest]=true|false]\n" \
    "           [STatic=true|false]\n" \
    "        - Add incrementing L2 addresses associated with port(s)\n"

#define CMD_L2_USAGE_CLEAR \
    "    l2 clear [MACaddress=<mac>] [Vlanid=<id>]\n" \
    "             [Module=<n>] [Port=<port>]\n" \
    "             [TrunkGroupID=<id>] [Static=true|false]\n" \
    "        - Remove all L2 entries on the given module/port, MAC,\n" \
    "          VLAN, or trunk group ID\n"

#define CMD_L2_USAGE_DEL \
    "    l2 del [MACaddress=<mac>] [Vlanid=<id>] [Count=<value>]\n" \
    "        - Delete L2 address(s)\n"

#define CMD_L2_USAGE_INIT \
    "    l2 init\n" \
    "        - Initialize L2\n"

#define CMD_L2_USAGE_SHOW \
    "    l2 show [MACaddress=<mac> Vlanid=<id>] [Count=<value>]\n" \
    "        - Show L2 addresses starting with given VLAN MAC key\n"

#define CMD_L2_USAGE_REDIRECT \
    "    l2 redirect [MACaddress=<mac>] [Vlanid=<id>]\n" \
    "           [qidunion=<n>]\n" 

char cmd_sbx_l2_usage[] =
    "\n"
#ifdef COMPILER_STRING_CONST_LIMIT
    "    l2 <option> [args...]\n"
#else
    CMD_L2_USAGE_ADD   "\n"
    CMD_L2_USAGE_CLEAR "\n"
    CMD_L2_USAGE_DEL   "\n"
    CMD_L2_USAGE_INIT  "\n"
    CMD_L2_USAGE_REDIRECT "\n"
    CMD_L2_USAGE_SHOW  "\n"
#endif
    ;

static cmd_t _cmd_sbx_l2_list[] = {
    {"ADD",    _cmd_sbx_l2_add,   "\n" CMD_L2_USAGE_ADD,   NULL},
    {"+",      _cmd_sbx_l2_add,   "\n" CMD_L2_USAGE_ADD,   NULL},
    {"CLEAR",  _cmd_sbx_l2_clear, "\n" CMD_L2_USAGE_CLEAR, NULL},
    {"DELete", _cmd_sbx_l2_del,   "\n" CMD_L2_USAGE_DEL,   NULL},
    {"-",      _cmd_sbx_l2_del,   "\n" CMD_L2_USAGE_DEL,   NULL},
    {"INIT",   _cmd_sbx_l2_init,  "\n" CMD_L2_USAGE_INIT,  NULL},
    {"Redirect", _cmd_sbx_l2_redirect, "\n" CMD_L2_USAGE_REDIRECT, NULL},
    {"SHOW",   _cmd_sbx_l2_show,  "\n" CMD_L2_USAGE_SHOW,  NULL}
};



/*
 * Local global variables to remember last values in arguments.
 *
 * Notes:
 * Initialize MAC address field for the user to the first real
 * address which does not conflict.
 */
static sal_mac_addr_t   _l2_macaddr = {0x0, 0x0, 0x0, 0x0, 0x0, 0x1};

static int _l2_vlan = VLAN_ID_DEFAULT;

static int _l2_modid = 0, _l2_port, _l2_trunk = 0, _l2_tgid = 0, 
           _l2_dsrc = 0, _l2_ddest = 0, _l2_static = 0;

static pbmp_t  _l2_pbmp;


STATIC cmd_result_t
_cmd_sbx_l2_add(int unit, args_t *args)
{
    cmd_result_t   retCode;
    int            rv;
    parse_table_t  pt;
    pbmp_t         pbmp;
    bcm_l2_addr_t  l2addr;
    int            port = BCM_GPORT_INVALID;

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "MACaddress", PQ_DFL|PQ_MAC, 0, &_l2_macaddr, NULL);
    parse_table_add(&pt, "Vlanid", PQ_DFL|PQ_HEX, 0, &_l2_vlan, NULL);
    parse_table_add(&pt, "Module", PQ_DFL|PQ_INT, 0, &_l2_modid, NULL);
    parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP, (void *)(0),
                    &_l2_pbmp, NULL);
    parse_table_add(&pt, "Trunk", PQ_DFL|PQ_BOOL, 0, &_l2_trunk, NULL);
    parse_table_add(&pt, "TrunkGroupID", PQ_DFL|PQ_INT, 0, &_l2_tgid, NULL);
    parse_table_add(&pt, "DiscardSource", PQ_DFL|PQ_BOOL, 0, &_l2_dsrc, NULL);
    parse_table_add(&pt, "DiscardDest",  PQ_DFL|PQ_BOOL, 0, &_l2_ddest, NULL);
    parse_table_add(&pt, "STatic", PQ_DFL|PQ_BOOL, 0, &_l2_static, NULL);

    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    pbmp = _l2_pbmp;
    if (BCM_PBMP_IS_NULL(pbmp) && !_l2_trunk) {
        cli_out("%s ERROR: empty port bitmap\n", ARG_CMD(args));
        return CMD_FAIL;
    }

    /*
     * If we are setting the range, the MAC address is incremented by
     * 1 for each port.
     */
    if (!_l2_trunk) {
    PBMP_ITER(pbmp, port) {
        bcm_l2_addr_t_init(&l2addr, _l2_macaddr, _l2_vlan);

        /* Configure L2 addr flags */
        if (_l2_static) {
            l2addr.flags |= BCM_L2_STATIC;
        }
        if (_l2_ddest) {
            l2addr.flags |= BCM_L2_DISCARD_DST;
        }
        if (_l2_dsrc) {
            l2addr.flags |= BCM_L2_DISCARD_SRC;
        }
        if (_l2_trunk) {
            l2addr.flags |= BCM_L2_TRUNK_MEMBER;
        }

        l2addr.modid = _l2_modid;
        l2addr.port  = port;
        l2addr.tgid  = _l2_tgid;

        if (LOG_CHECK(BSL_LS_BCM_L2 | BSL_VERBOSE)) {
            dump_l2_addr(unit, "ADD: ", &l2addr);
        }

        rv = bcm_l2_addr_add(unit, &l2addr);
        if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: %s\n", ARG_CMD(args), bcm_errmsg(rv));
            return CMD_FAIL;
        }

        /* Set up for next call */
        increment_macaddr(_l2_macaddr, 1);
        }
    } else {
        bcm_l2_addr_t_init(&l2addr, _l2_macaddr, _l2_vlan);

        /* Configure L2 addr flags */
        if (_l2_static) {
            l2addr.flags |= BCM_L2_STATIC;
        }
        if (_l2_ddest) {
            l2addr.flags |= BCM_L2_DISCARD_DST;
        }
        if (_l2_dsrc) {
            l2addr.flags |= BCM_L2_DISCARD_SRC;
        }
        if (_l2_trunk) {
            l2addr.flags |= BCM_L2_TRUNK_MEMBER;
        }

        l2addr.modid = _l2_modid;
        l2addr.port  = port;
        l2addr.tgid  = _l2_tgid;

        if (LOG_CHECK(BSL_LS_BCM_L2 | BSL_VERBOSE)) {
            dump_l2_addr(unit, "ADD: ", &l2addr);
        }

        rv = bcm_l2_addr_add(unit, &l2addr);
        if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: %s\n", ARG_CMD(args), bcm_errmsg(rv));
            return CMD_FAIL;
        }
    }

    return CMD_OK;
}

/*
 * This function redirects this UC mac,vid to a different mod/port (FTE)
 */
STATIC cmd_result_t
_cmd_sbx_l2_redirect(int unit, args_t *args)
{
    cmd_result_t   retCode;
    int            rv = BCM_E_UNAVAIL;
    parse_table_t  pt;
    bcm_l2_addr_t  l2addr;
    int            qidunion;

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "MACaddress", PQ_DFL|PQ_MAC, 0, &_l2_macaddr, NULL);
    parse_table_add(&pt, "Vlanid", PQ_DFL|PQ_HEX, 0, &_l2_vlan, NULL);
    parse_table_add(&pt, "QidUnion", PQ_DFL|PQ_INT, 0, &qidunion, NULL);

    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    bcm_l2_addr_t_init(&l2addr, _l2_macaddr, _l2_vlan);

    l2addr.modid = _l2_modid;
    l2addr.tgid  = _l2_tgid;

    if (LOG_CHECK(BSL_LS_BCM_L2 | BSL_VERBOSE)) {
        dump_l2_addr(unit, "REDIRECT: ", &l2addr);
    }
    
    if (SOC_IS_SBX_CALADAN3(unit)){
#ifdef BCM_CALADAN3_SUPPORT
      extern int _bcm_caladan3_l2_addr_update_dest(int unit, bcm_l2_addr_t *l2addr, int qidunion);
        rv = _bcm_caladan3_l2_addr_update_dest(unit, &l2addr, qidunion);
        if (BCM_FAILURE(rv)) {
	    cli_out("%s ERROR: %s\n", ARG_CMD(args), bcm_errmsg(rv));
	    return CMD_FAIL;
        }
#endif
    }
 else {
      cli_out("%s ERROR: %s: redirect only supported on FE2K or CALADAN3\n",
              ARG_CMD(args), bcm_errmsg(rv));
      return CMD_FAIL;
    }
    return CMD_OK;
}

/* CLEAR command options to delete MAC entries 'by' these arguments */
#define DELETE_BY_MAC     0x00000001
#define DELETE_BY_VLAN    0x00000002
#define DELETE_BY_MODID   0x00000004
#define DELETE_BY_PORT    0x00000008
#define DELETE_BY_TGID    0x00000010

STATIC cmd_result_t
_cmd_sbx_l2_clear(int unit, args_t *args)
{
    cmd_result_t   retCode;
    int            rv;
    parse_table_t  pt;
    char           *static_str;
    bcm_mac_t      macaddr;
    int            vlan = -1, modid = -1, port = -1, tgid = -1;
    int            mac_static = TRUE;
    int            delete_by = 0;


    ENET_SET_MACADDR(macaddr, _soc_mac_all_zeroes);

    if (!ARG_CNT(args)) {
        /*
         * Use current global L2 argument values if none arguments given
         */
        ENET_SET_MACADDR(macaddr, _l2_macaddr);
        vlan  = _l2_vlan;
        modid = _l2_modid;
        port  = _l2_port;
        tgid  = _l2_tgid;
        mac_static = _l2_static;

    } else {
        /* Parse command option arguments */
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "MACaddress", PQ_DFL|PQ_MAC, 0, &macaddr, NULL);
        parse_table_add(&pt, "Vlanid", PQ_DFL|PQ_HEX, 0, &vlan, NULL);
        parse_table_add(&pt, "Module", PQ_DFL|PQ_INT, 0, &modid, NULL);
        parse_table_add(&pt, "Port", PQ_DFL|PQ_PORT, 0, &port, NULL);
        parse_table_add(&pt, "TrunkGroupID", PQ_DFL|PQ_INT, 0, &tgid, NULL);
        parse_table_add(&pt, "STatic", PQ_DFL|PQ_BOOL, 0, &mac_static, NULL);

        if (!parseEndOk(args, &pt, &retCode)) {
            return retCode;
        }
    }
    
    /*
     * Determine which arguments were supplied
     */
    if (ENET_CMP_MACADDR(macaddr, _soc_mac_all_zeroes)) {
        delete_by |= DELETE_BY_MAC;
    }
    if (vlan >= 0) {
        delete_by |= DELETE_BY_VLAN;
    }
    if (modid >=0) {
        delete_by |= DELETE_BY_MODID;
    }
    if (port >= 0) {
        delete_by |= DELETE_BY_PORT;
    }
    if (tgid >= 0) {
        delete_by |= DELETE_BY_TGID;
    }

    static_str = mac_static ? "static and non-static" : "non-static";
    mac_static = mac_static ? BCM_L2_DELETE_STATIC : 0;

    /* Delete 'by' */
    switch (delete_by) {
    case DELETE_BY_MAC:
        cli_out("%s: Deleting %s addresses by MAC\n",
                ARG_CMD(args), static_str);
        rv = bcm_l2_addr_delete_by_mac(unit, macaddr, mac_static);
        break;

    case DELETE_BY_VLAN:
        cli_out("%s: Deleting %s addresses by VLAN\n",
                ARG_CMD(args), static_str);
        rv = bcm_l2_addr_delete_by_vlan(unit, vlan, mac_static);
        break;
        
    case DELETE_BY_MAC | DELETE_BY_VLAN:
        cli_out("%s: Deleting an address by MAC and VLAN\n",
                ARG_CMD(args));
        rv = bcm_l2_addr_delete(unit, macaddr, vlan);
        break;

    case DELETE_BY_PORT:
        cli_out("%s: Deleting %s addresses by port, local module ID\n",
                ARG_CMD(args), static_str);
        rv = bcm_l2_addr_delete_by_port(unit, -1, port, mac_static);
        break;

    case DELETE_BY_PORT | DELETE_BY_MODID:
        cli_out("%s: Deleting %s addresses by module/port\n",
                ARG_CMD(args), static_str);
        rv = bcm_l2_addr_delete_by_port(unit, modid, port, mac_static);
        break;

    case DELETE_BY_TGID:
        cli_out("%s: Deleting %s addresses by trunk ID\n",
                ARG_CMD(args), static_str);
        rv = bcm_l2_addr_delete_by_trunk(unit, tgid, mac_static);
        break;

    case DELETE_BY_MAC | DELETE_BY_PORT:
        cli_out("%s: Deleting %s addresses by MAC and port\n",
                ARG_CMD(args), static_str);
        rv = bcm_l2_addr_delete_by_mac_port(unit, macaddr, -1, port,
                                            mac_static);
        break;

    case DELETE_BY_MAC | DELETE_BY_PORT | DELETE_BY_MODID:
        cli_out("%s: Deleting %s addresses by MAC and module/port\n",
                ARG_CMD(args), static_str);
        rv = bcm_l2_addr_delete_by_mac_port(unit, macaddr, modid, port,
                                            mac_static);
        break;

    case DELETE_BY_VLAN | DELETE_BY_PORT:
	    cli_out("%s: Deleting %s addresses by VLAN and port\n",
                    ARG_CMD(args), static_str);
	    rv = bcm_l2_addr_delete_by_vlan_port(unit, vlan, -1, port,
                                             mac_static);
        break;

    case DELETE_BY_VLAN | DELETE_BY_PORT | DELETE_BY_MODID:
	    cli_out("%s: Deleting %s addresses by VLAN and module/port\n",
                    ARG_CMD(args), static_str);
	    rv = bcm_l2_addr_delete_by_vlan_port(unit, vlan, modid, port,
                                             mac_static);
        break;

    case DELETE_BY_VLAN | DELETE_BY_TGID:
	    cli_out("%s: Deleting %s addresses by VLAN and trunk ID\n",
                    ARG_CMD(args), static_str);
	    rv = bcm_l2_addr_delete_by_vlan_trunk(unit, vlan, tgid, mac_static);
        break;

    default:
	    cli_out("%s: Unknown argument combination\n", ARG_CMD(args));
        return CMD_USAGE;
        break;
	}

    if (BCM_FAILURE(rv)) {
        cli_out("%s ERROR: %s\n", ARG_CMD(args), bcm_errmsg(rv));
        return CMD_FAIL;
    }

	return CMD_OK;
}


STATIC cmd_result_t
_cmd_sbx_l2_del(int unit, args_t *args)
{
    cmd_result_t   retCode;
    int            rv;
    parse_table_t  pt;
    bcm_l2_addr_t  l2addr;
    int            idx;
    int            count = 1;

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "MACaddress", PQ_DFL|PQ_MAC, 0, &_l2_macaddr, NULL);
    parse_table_add(&pt, "Vlanid", PQ_DFL|PQ_HEX, 0, &_l2_vlan, NULL);
    parse_table_add(&pt, "Count", PQ_DFL|PQ_INT, (void *)(1), &count, NULL);
    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

	for (idx = 0; idx < count; idx++) {
	    rv = bcm_l2_addr_get(unit, _l2_macaddr, _l2_vlan, &l2addr);
        if (BCM_FAILURE(rv)) {
            cli_out("%s: ERROR: %s\n", ARG_CMD(args), bcm_errmsg(rv));
            return CMD_FAIL;
	    }

        if (LOG_CHECK(BSL_LS_BCM_L2 | BSL_VERBOSE)) {
            dump_l2_addr(unit, "DEL: ", &l2addr);
        }

        rv = bcm_l2_addr_delete(unit, _l2_macaddr, _l2_vlan);
        if (BCM_FAILURE(rv)) {
            cli_out("%s: ERROR: %s\n", ARG_CMD(args), bcm_errmsg(rv));
            return CMD_FAIL;
	    }

	    increment_macaddr(_l2_macaddr, 1);
	}

	return CMD_OK;
}


STATIC cmd_result_t
_cmd_sbx_l2_init(int unit, args_t *args)
{
    int  rv;

    if (ARG_CNT(args) > 0) {
        return CMD_USAGE;
    }

    if (BCM_FAILURE(rv = bcm_l2_init(unit))) {
        cli_out("%s ERROR: %s\n", ARG_CMD(args), bcm_errmsg(rv));
        return CMD_FAIL;
    }

    return CMD_OK;
}


STATIC cmd_result_t
_cmd_sbx_l2_show(int unit, args_t *args)
{
  cmd_result_t   ret_code;
  int            rv;
  bcm_mac_t      mac = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
  int            vid = 0;
  int            max_count = 0;
  
  if (ARG_CNT(args) > 0) {
    parse_table_t  pt;
    
    /* Parse option */
    parse_table_init(0, &pt);
    parse_table_add(&pt, "MACaddress", PQ_DFL|PQ_MAC, 0, &mac, NULL);
    parse_table_add(&pt, "Vlanid", PQ_DFL|PQ_HEX, 0, &vid, NULL);
    parse_table_add(&pt, "COUNT", PQ_DFL | PQ_INT, 0, &max_count,  NULL);
    if (!parseEndOk(args, &pt, &ret_code)) {
      return ret_code;
    }
  }
  
  if (SOC_IS_SBX_CALADAN3(unit)){
#ifdef BCM_CALADAN3_SUPPORT
    rv = _bcm_caladan3_l2_addr_dump(unit, mac,vid, max_count);
    if (BCM_FAILURE(rv)) {
      cli_out("%s ERROR: %s\n", ARG_CMD(args), bcm_errmsg(rv));
      return CMD_FAIL;
    }
#endif
  } 
  return CMD_OK;
  
}

cmd_result_t
cmd_sbx_l2(int unit, args_t *args)
{
    return subcommand_execute(unit, args, 
                              _cmd_sbx_l2_list, COUNTOF(_cmd_sbx_l2_list));
}


/*
 * L2 Age
 *
 * Note:
 *     The CLI to manipulate aging is set in a command group different
 *     from L2.  This maintains the same CLI command menu as the
 *     current XGS/ROBO CLI.
 */
char cmd_sbx_age_usage[] =
    "Parameters:  [<seconds>]\n\t"
    "   Set the L2 age timer to the indicated number of seconds.\n\t"
    "   With no parameter, displays current value.\n\t"
    "   Setting to 0 disables L2 aging.\n";

cmd_result_t
cmd_sbx_age(int unit, args_t *args)
{
    int  seconds;
    int  rv;

    if (!ARG_CNT(args)) {    /* Display settings */
        rv = bcm_l2_age_timer_get(unit, &seconds);
        if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: could not get age time: %s\n",
                    ARG_CMD(args), bcm_errmsg(rv));
            return CMD_FAIL;
        }

        cli_out("Current age timer is %d %s\n",
                seconds, seconds ? "seconds" : "(disabled)");

        return CMD_OK;
    }

    seconds = sal_ctoi(ARG_GET(args), 0);

    rv = bcm_l2_age_timer_set(unit, seconds);
    if (BCM_FAILURE(rv)) {
        cli_out("%s ERROR: could not set age time: %s\n",
                ARG_CMD(args), bcm_errmsg(rv));
        return CMD_FAIL;
    }

    cli_out("Set age timer to %d %s\n",
            seconds, seconds ? "seconds" : "(disabled)");

    return CMD_OK;
}

char cmd_sbx_l2_cache_usage[] =
    "    l2cache add|del|show [MACaddress=<mac>] [MACaddressMask=<mask>] [Vlanid=<id>] [VlanidMask=<mask>] [CacheIndex=<i>]\n" 
    "       [SourcePort=<port>] [SourcePortMask=<mask>] [BPDU=<0|1>]\n";
cmd_result_t
cmd_sbx_l2_cache(int unit, args_t *a)
{
        int cidx;
        char *cachecmd = NULL;
        char str[16];
        sal_mac_addr_t arg_macaddr, arg_macaddr_mask;
        int arg_setpri = 0, arg_bpdu = 0;
        int arg_prio = -1, arg_lrndis = FALSE, arg_tunnel = FALSE;
        int arg_vlan_mask, arg_cidx, arg_ccount, arg_reinit, idx_max;
        int arg_sport = -1, arg_sport_mask;
        int arg_lookup_class = 0;
        bcm_l2_cache_addr_t l2caddr;
        int		arg_trunk = 0, arg_l3if = 0,
	    arg_ds = 0, arg_dd = 0, arg_modid = 0,
        arg_vlan = VLAN_ID_DEFAULT, arg_tgid = 0, 
        arg_cbit = 0, arg_port = 0, 
        arg_mirror = 0;
        parse_table_t	pt;
        int          arg_rpe = TRUE;
        cmd_result_t	retCode;
        int			rv = CMD_OK;
        int                 idx;

        if ((cachecmd = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }

        /* Masks always default to full mask */
        arg_vlan_mask = BCM_L2_VID_MASK_ALL;
        arg_sport_mask = BCM_L2_SRCPORT_MASK_ALL;
        sal_memset(arg_macaddr, 0, sizeof (sal_mac_addr_t));
        sal_memset(arg_macaddr_mask, 0xff, sizeof (sal_mac_addr_t));

        arg_cidx = -1;
        arg_ccount = 1;

        if (!sal_strcasecmp(cachecmd, "add") ||
            !sal_strcasecmp(cachecmd, "+") ) {

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "LookupClass",         PQ_DFL|PQ_INT,
                            0, &arg_lookup_class, NULL);
            parse_table_add(&pt, "CacheIndex",          PQ_DFL|PQ_INT,
                            0, &arg_cidx,       NULL);
            parse_table_add(&pt, "MACaddress",          PQ_DFL|PQ_MAC,
                            0, &arg_macaddr,    NULL);
            parse_table_add(&pt, "MACaddressMask",      PQ_DFL|PQ_MAC,
                            0, &arg_macaddr_mask, NULL);
            parse_table_add(&pt, "Vlanid",              PQ_DFL|PQ_HEX,
                            0, &arg_vlan,       NULL);
            parse_table_add(&pt, "VlanidMask",          PQ_DFL|PQ_HEX,
                            0, &arg_vlan_mask,  NULL);
            parse_table_add(&pt, "SourcePort",          PQ_DFL|PQ_PORT|PQ_BCM,
                            0, &arg_sport,      NULL);
            parse_table_add(&pt, "SourcePortMask",      PQ_DFL|PQ_HEX,
                            0, &arg_sport_mask, NULL);
            parse_table_add(&pt, "Module",              PQ_DFL|PQ_INT,
                            0, &arg_modid,      NULL);
            parse_table_add(&pt, "Port",                PQ_DFL|PQ_PORT,
                            (void *)(0), &arg_port, NULL);
            parse_table_add(&pt, "Trunk",               PQ_DFL|PQ_BOOL,
                            0, &arg_trunk,	NULL);
            parse_table_add(&pt, "TrunkGroupID",        PQ_DFL|PQ_INT,
                            0, &arg_tgid,	NULL);
            parse_table_add(&pt, "SetPriority",         PQ_DFL|PQ_BOOL,
                            0, &arg_setpri,	NULL);
            parse_table_add(&pt, "PRIOrity",            PQ_DFL|PQ_INT,
                            0, &arg_prio,       NULL);
            parse_table_add(&pt, "DiscardDest",         PQ_DFL|PQ_BOOL,
                            0, &arg_dd,	NULL);
            parse_table_add(&pt, "L3",                  PQ_DFL|PQ_BOOL,
                            0, &arg_l3if,       NULL);
            parse_table_add(&pt, "CPUmirror",           PQ_DFL|PQ_BOOL,
                            0, &arg_cbit,       NULL);
            parse_table_add(&pt, "MIRror",              PQ_DFL|PQ_BOOL,
                            0, &arg_mirror,     NULL);
            parse_table_add(&pt, "BPDU",                PQ_DFL|PQ_BOOL,
                            0, &arg_bpdu,       NULL);
            parse_table_add(&pt, "ReplacePriority",  PQ_DFL|PQ_BOOL,
                            0, &arg_rpe,	NULL);
            if (SOC_IS_FIREBOLT2(unit) || SOC_IS_TRX(unit)) {
                parse_table_add(&pt, "LearnDisable",   PQ_DFL|PQ_BOOL,
                                0, &arg_lrndis,	NULL);
            }
            parse_table_add(&pt, "TunnelTermination",  PQ_DFL|PQ_BOOL,
                            0, &arg_tunnel,	NULL);

            if (!parseEndOk(a, &pt, &retCode)) {
                return retCode;
            }

            if (arg_setpri && arg_prio == -1) {
                cli_out("%s ERROR: no priority specified\n", ARG_CMD(a));
                return CMD_FAIL;
            }
            if (!arg_setpri) {
                arg_prio = -1;
            }

            bcm_l2_cache_addr_t_init(&l2caddr);

            l2caddr.vlan = arg_vlan_mask ? arg_vlan : 0;
            l2caddr.vlan_mask = arg_vlan_mask;

            ENET_COPY_MACADDR(arg_macaddr, l2caddr.mac);
            ENET_COPY_MACADDR(arg_macaddr_mask, l2caddr.mac_mask);

            if (arg_sport >= 0) {
                l2caddr.src_port = arg_sport;
                l2caddr.src_port_mask = arg_sport_mask;
            }

            l2caddr.lookup_class = arg_lookup_class;

            l2caddr.dest_modid = arg_modid;
            l2caddr.dest_port = arg_port;
            l2caddr.prio = arg_prio;
            l2caddr.dest_trunk = arg_tgid;

            /* Configure flags for SDK call */
            if (arg_dd || arg_ds)
                l2caddr.flags |= BCM_L2_CACHE_DISCARD;
            if (arg_l3if)
                l2caddr.flags |= BCM_L2_CACHE_L3;
            if (arg_trunk)
                l2caddr.flags |= BCM_L2_CACHE_TRUNK;
            if (arg_cbit)
                l2caddr.flags |= BCM_L2_CACHE_CPU;
            if (arg_bpdu)
                l2caddr.flags |= BCM_L2_CACHE_BPDU;
            if (arg_mirror)
                l2caddr.flags |= BCM_L2_CACHE_MIRROR;
	        if (arg_rpe || arg_setpri)
		        l2caddr.flags |= BCM_L2_CACHE_SETPRI;
	        if (arg_lrndis)
		        l2caddr.flags |= BCM_L2_CACHE_LEARN_DISABLE;
	        if (arg_tunnel)
		        l2caddr.flags |= BCM_L2_CACHE_TUNNEL;

            dump_l2_cache_addr(unit, "ADD CACHE: ", &l2caddr);

            if ((rv = bcm_l2_cache_set(unit, arg_cidx, &l2caddr, 
                                       &cidx)) != BCM_E_NONE) {
                cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }

            if (arg_cidx == -1) {
                cli_out(" => using index %d\n", cidx);
            }

	    /* Set up for next call */
	    increment_macaddr(arg_macaddr, 1);
            if (arg_cidx >= 0) {
                arg_cidx++;
            }

            return CMD_OK;
        }

        else if (!sal_strcasecmp(cachecmd, "del") ||
                 !sal_strcasecmp(cachecmd, "-")) {

            if (ARG_CNT(a)) {
                parse_table_init(unit, &pt);
                parse_table_add(&pt, "CacheIndex",      PQ_DFL|PQ_INT,
                                0, &arg_cidx,   NULL);
                parse_table_add(&pt, "Count",           PQ_DFL|PQ_INT,
                                0, &arg_ccount, NULL);
                if (!parseEndOk(a, &pt, &retCode)) {
                    return retCode;
                }
            }

            if (arg_cidx == -1) {
                cli_out("%s ERROR: no index specified\n", ARG_CMD(a));
                return CMD_FAIL;
            }

            for (idx = 0; idx < arg_ccount; idx++) {
                if ((rv = bcm_l2_cache_get(unit, arg_cidx, &l2caddr)) < 0) {
                    cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                    return CMD_FAIL;
                }

                if (LOG_CHECK(BSL_LS_BCM_L2 | BSL_VERBOSE)) {
                    dump_l2_cache_addr(unit, "DEL CACHE: ", &l2caddr);
                }

                if ((rv = bcm_l2_cache_delete(unit, arg_cidx)) != BCM_E_NONE) {
                    cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                    return CMD_FAIL;
                }
                arg_cidx++;
            }

            return CMD_OK;
        }

        else if (!sal_strcasecmp(cachecmd, "show") ||
                 !sal_strcasecmp(cachecmd, "-d")) {

	    if ((rv = bcm_l2_cache_size_get(unit, &idx_max)) < 0) {
		cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
		return CMD_FAIL;
	    }

	    for (idx = 0; idx < idx_max; idx++) {
            
            if (bcm_l2_cache_get(unit, idx, &l2caddr) == BCM_E_NONE) {
                    sal_sprintf(str, " %4d : ", idx);
                    dump_l2_cache_addr(unit, str, &l2caddr);
            }
        }

            return CMD_OK;
        }

        else if (!sal_strcasecmp(cachecmd, "clear") ||
                 !sal_strcasecmp(cachecmd, "clr")) {

            arg_reinit = 0;

            if (ARG_CNT(a)) {
                parse_table_init(unit, &pt);
                parse_table_add(&pt, "ReInit",              PQ_DFL|PQ_BOOL,
                                0, &arg_reinit,     NULL);
                if (!parseEndOk(a, &pt, &retCode)) {
                    return retCode;
                }
            }

            if ((rv = bcm_l2_cache_delete_all(unit)) != BCM_E_NONE) {
                cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }

            if (arg_reinit && (rv = bcm_l2_cache_init(unit)) != BCM_E_NONE) {
                cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }

            return CMD_OK;
        }

        else {
            return CMD_USAGE;
        }
    }
#if 0 /* code to test MAC deletion performance */
#include <time.h>
#include <sal/core/time.h>
void l2_addr_del_by_test()
{
    int ret=0;
    int unit=1;
    bcm_l2_addr_t  l2Addr; 
    int g_DrvMacTstAddSucc;
    int g_DrvMacTstAddFail;
    sal_usecs_t start,end;

    g_DrvMacTstAddSucc=0;
    g_DrvMacTstAddFail=0;

    sal_srand((unsigned )time(NULL));
    while(1)
    {
        memset(&l2Addr, 0, sizeof(bcm_l2_addr_t));

        l2Addr.mac[0] = (sal_rand()%256)&0xFE;
        l2Addr.mac[1] = sal_rand()%256;
        l2Addr.mac[2] = sal_rand()%256;
        l2Addr.mac[3] = sal_rand()%256;
        l2Addr.mac[4] = sal_rand()%256;
        l2Addr.mac[5] = sal_rand()%256;
        l2Addr.vid = 0x2;
        l2Addr.modid = 0;
        l2Addr.port = sal_rand()%24;

        ret = bcm_l2_addr_add(unit, &l2Addr);
        if( BCM_E_NONE == ret )
        {
            if (g_DrvMacTstAddSucc++ > 350000) ; /*break;*/
            g_DrvMacTstAddFail=0;
            printf("%7d\r", g_DrvMacTstAddSucc);
        }
        else if(BCM_E_MEMORY== ret)
        {
            g_DrvMacTstAddFail++;
            if(g_DrvMacTstAddFail > 1000 )
            {
                break;
            }
        }
    }

    printf("added g_DrvMacTstAddSucc=%d MACs\n", g_DrvMacTstAddSucc);
    
    start = sal_time_usecs();

    ret = bcm_l2_addr_delete_by_vlan(unit, 0x2, 0x0);

 end = sal_time_usecs();
    
 cli_out("bcm_l2_addr_delete_by_vlan return %d at %d us\n", ret, (end-start));
}
#endif /* l2_addr_del_by_test */

#endif /* BCM_FE2000_SUPPORT */
