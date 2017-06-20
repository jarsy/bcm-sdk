/*
 * $Id: l2.c,v 1.46 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#include <soc/arl.h>
#include <soc/l2x.h>
#include <soc/l2u.h>
#include <soc/debug.h>
#include <soc/hash.h>

#include <bcm/error.h>
#include <bcm/l2.h>
#include <bcm/mcast.h>
#include <bcm/debug.h>
#include <bcm/stack.h>

#ifdef  BCM_TB_SUPPORT
#include <bcm_int/robo/subport.h>
#include <bcm_int/robo/l2.h>
#include <bcm_int/robo/port.h>
#endif  /* BCM_TB_SUPPORT */

const sal_mac_addr_t _mac_all_zeroes =
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* CB function to print L2 Entry */
int _robo_l2addr_dump(int unit, bcm_l2_addr_t *l2addr, void *user_data)
{
    bcm_module_t        local_modid;
    char                pfmt[SOC_PBMP_FMT_LEN];
    char                bmstr[FORMAT_PBMP_MAX];

    if (bcm_stk_my_modid_get(unit, &local_modid) != BCM_E_NONE) {
        local_modid = -1;
    } 

    cli_out("mac=%02x:%02x:%02x:%02x:%02x:%02x vlan=%d",
            l2addr->mac[0], l2addr->mac[1], l2addr->mac[2],
            l2addr->mac[3], l2addr->mac[4], l2addr->mac[5], l2addr->vid);


    if (!(l2addr->flags & BCM_L2_MCAST)) {              
        if (BCM_GPORT_IS_SET(l2addr->port)){
                uint32  gport_id, mod_id, port_id;
#ifdef  BCM_TB_SUPPORT
                uint32  vp_id;
#endif  /* BCM_TB_SUPPORT */
            if (BCM_GPORT_IS_LOCAL(l2addr->port)) {
                gport_id = BCM_GPORT_LOCAL_GET(l2addr->port);
                cli_out(" modid=%d port=%d%s%s", l2addr->modid, gport_id, 
                        ((local_modid) == l2addr->modid) ? "/" : " ",
                        ((local_modid == l2addr->modid) ? 
                        SOC_PORT_NAME(unit, gport_id) : " "));
            } else if (BCM_GPORT_IS_MODPORT(l2addr->port)) {
                mod_id = BCM_GPORT_MODPORT_MODID_GET(l2addr->port);
                if (mod_id != l2addr->modid){
                    cli_out(" (l2.modid=%d)", l2addr->modid);
                }
                port_id = BCM_GPORT_MODPORT_PORT_GET(l2addr->port);
                cli_out(" modid=%d port=%d%s%s", mod_id, port_id, 
                        ((local_modid) == mod_id) ? "/" : " ",
                        ((local_modid == mod_id) ? 
                        SOC_PORT_NAME(unit, port_id) : " "));
            } else if (BCM_GPORT_IS_LOCAL_CPU(l2addr->port)) {
                cli_out(" port=CPU");
            } else if (BCM_GPORT_IS_SUBPORT_GROUP(l2addr->port)) {
                gport_id = BCM_GPORT_SUBPORT_GROUP_GET(l2addr->port);
                cli_out(" port=%d(vp_group)", l2addr->port);
            } else if (BCM_GPORT_IS_SUBPORT_PORT(l2addr->port)) {
                gport_id = BCM_GPORT_SUBPORT_PORT_GET(l2addr->port);
                if (SOC_IS_TBX(unit)){
#ifdef  BCM_TB_SUPPORT
                    port_id = _TB_SUBPORT_SYSTEM_ID_2PORT(gport_id);
                    vp_id = _TB_SUBPORT_SYSTEM_ID_2VPORT(gport_id);
                    cli_out(" port=%d vport=%d", port_id, vp_id);
#endif  /* BCM_TB_SUPPORT */
                } else {
                    cli_out(" port=%d(subport_port)", gport_id);
                }
            } else {
                cli_out(" port=%d(UNEXPECTED GPORT)", l2addr->port);
            }
        } else {
            cli_out(" modid=%d port=%d%s%s", l2addr->modid, l2addr->port, 
                    ((local_modid) == l2addr->modid) ? "/" : " ",
                    ((local_modid == l2addr->modid) ? 
                    SOC_PORT_NAME(unit, l2addr->port) : " "));        
        }
    }
    if (l2addr->flags & BCM_L2_TRUNK_MEMBER) {
        cli_out(" Trunk=%d", l2addr->tgid);
    }

    if (l2addr->flags & BCM_L2_STATIC) {
        cli_out(" Static");
    }

    if (l2addr->flags & BCM_L2_HIT) {
        cli_out(" Hit");
    }

    if (l2addr->flags & BCM_L2_PENDING) {
        cli_out(" Pending");
    }

    if (l2addr->cos_src != 0 || l2addr->cos_dst != 0) {
#ifdef BCM_ROBO_SUPPORT        
        if(SOC_IS_ROBO(unit)) {
            if (SOC_IS_ROBO_GE_SWITCH(unit)) {
                cli_out(" Priority=%d", l2addr->cos_dst);
            } else {
                cli_out(" COS=%d", l2addr->cos_dst);
            }
        } else {
#endif /* BCM_ROBO_SUPPORT */        
            cli_out(" COS(src=%d,dst=%d)", l2addr->cos_src, l2addr->cos_dst);
#ifdef BCM_ROBO_SUPPORT
        }
#endif /* BCM_ROBO_SUPPORT */        
    }

    if (l2addr->flags & BCM_L2_COS_SRC_PRI) {
        cli_out(" SCP");
    }

    if ((l2addr->flags & BCM_L2_COPY_TO_CPU) ||
        (l2addr->port == CMIC_PORT(unit))) {
        cli_out(" CPU");
    }

    if (l2addr->flags & BCM_L2_L3LOOKUP) {
        cli_out(" L3");
    }

    if (l2addr->flags & BCM_L2_DISCARD_SRC) {
        cli_out(" DiscardSrc");
    }

    if (l2addr->flags & BCM_L2_DISCARD_DST) {
        cli_out(" DiscardDest");
    }

    if (l2addr->flags & BCM_L2_MCAST) {
        cli_out(" MCast=%d", l2addr->l2mc_group);
    }

    if (SOC_PBMP_NOT_NULL(l2addr->block_bitmap)) {
        format_pbmp(unit, bmstr, sizeof (bmstr), l2addr->block_bitmap);
        cli_out(" MAC blocked port bitmap=%s: %s",
                SOC_PBMP_FMT(l2addr->block_bitmap, pfmt), bmstr);
    }

    if (l2addr->group) {
        cli_out(" Group=%d", l2addr->group);
    }

    cli_out("\n");

    return BCM_E_NONE;
}

static void
_robo_dump_l2_cache_addr(int unit, char *pfx, bcm_l2_cache_addr_t *l2caddr)
{
    cli_out("%sMport_mac=%02x:%02x:%02x:%02x:%02x:%02x",
            pfx,
            l2caddr->mac[0], l2caddr->mac[1], l2caddr->mac[2],
            l2caddr->mac[3], l2caddr->mac[4], l2caddr->mac[5]);

    cli_out(" Mport_vctr=0x%x", SOC_PBMP_WORD_GET(l2caddr->dest_ports, 0));

    cli_out("\n");
}

/*
 * AGE timer
 */

cmd_result_t
cmd_robo_age(int unit, args_t *a)
{
    int seconds;
    int r;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }


    if (!ARG_CNT(a)) {          /* Display settings */
        if ((r = bcm_l2_age_timer_get(unit, &seconds)) != BCM_E_NONE) {
            cli_out("%s ERROR: could not get age time: %s\n",
                    ARG_CMD(a), bcm_errmsg(r));
            return CMD_FAIL;
        }

        cli_out("Current age timer is %d.\n", seconds);

        return CMD_OK;
    }

    seconds = sal_ctoi(ARG_GET(a), 0);

    if ((r = bcm_l2_age_timer_set(unit, seconds)) != BCM_E_NONE) {
        cli_out("%s ERROR: could not set age time: %s\n",
                ARG_CMD(a), bcm_errmsg(r));
        return CMD_FAIL;
    }

    cli_out("Set age timer to %d. %s\n", seconds, seconds ? "":"(disabled)");

    return CMD_OK;
}

cmd_result_t
if_robo_bpdu(int unit, args_t *a)
{
    int idx, count;
    char *subcmd = NULL;
    static int initted = 0;
    static sal_mac_addr_t arg_macaddr, maczero;
    bcm_l2_cache_addr_t addr;
    static int arg_index = 0;
    int idx_used = 0;
    int rv;
    parse_table_t pt;
    char buf[SAL_MACADDR_STR_LEN];
    
    /*
     * Initialize MAC address field for the user to the first real
     * address which does not conflict
     */
    if (! initted) {
        maczero[0] = 0;
        maczero[1] = 0;
        maczero[2] = 0;
        maczero[3] = 0;
        maczero[4] = 0;
        maczero[5] = 0;
        arg_macaddr[0] = 0x01;
        arg_macaddr[1] = 0x80;
        arg_macaddr[2] = 0xc2;
        arg_macaddr[3] = 0x00;
        arg_macaddr[4] = 0x00;
        arg_macaddr[5] = 0x00;
        initted = 1;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    /* Check valid device to operation on ...*/
    if (! sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if (!sal_strcasecmp(subcmd, "add") ||
            !sal_strcasecmp(subcmd, "+")) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Index", PQ_DFL|PQ_INT,
                (void *)(0), &arg_index, NULL);
        parse_table_add(&pt, "MACaddress", PQ_DFL|PQ_MAC,
                0, &arg_macaddr,NULL);

        if (!parseEndOk(a, &pt, &rv)) {
            return rv;
        }

        rv = DRV_DEV_PROP_GET(unit, 
                DRV_DEV_PROP_BPDU_NUM, (uint32 *) &count);
        if (rv != BCM_E_NONE) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        if (arg_index > (count - 1) ) {       
            cli_out("%s ERROR: index=%d invalid (support %d BUPUs)\n", 
                    ARG_CMD(a), arg_index, count);
            return CMD_FAIL;
        }
        if (BCM_MAC_IS_ZERO(arg_macaddr)){
            cli_out("Zero MAC address is invalid for BPDU ADD\n");
            return CMD_FAIL;
        }
        bcm_l2_cache_addr_t_init(&addr);
        ENET_COPY_MACADDR(arg_macaddr, addr.mac);
        addr.flags |= BCM_L2_CACHE_BPDU;
        rv = bcm_l2_cache_set(unit, arg_index, &addr, &idx_used);
        if (rv != BCM_E_NONE) {
            if (rv == BCM_E_FULL){
                cli_out("\t No Free Space for new BPDU addr\n");
            }
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        } else {
            cli_out("\t New BPDU added!(id=%d)\n", idx_used);
        }
        return CMD_OK;
    }

    if (!sal_strcasecmp(subcmd, "del") ||
            !sal_strcasecmp(subcmd, "-")) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Index", PQ_DFL|PQ_INT,
                    (void *)(0), &arg_index, NULL);

        if (!parseEndOk(a, &pt, &rv)) {
            return rv;
        }
        /* the CLI usage for 'del' allow valid id only, the address will be 
         * ignored in this sub-command.
         */
        if (arg_index == -1){
            cli_out("\t BPDU DEL allowed existed BPDU index only.\n");
            return CMD_FAIL;
        }
        rv = DRV_DEV_PROP_GET(unit, 
                DRV_DEV_PROP_BPDU_NUM, (uint32 *) &count);
        if (rv != BCM_E_NONE) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        if (arg_index > (count - 1) ) {       
            cli_out("%s ERROR: support %d BUPUs\n", ARG_CMD(a),count);
            return CMD_FAIL;
        }
        bcm_l2_cache_addr_t_init(&addr);
        ENET_COPY_MACADDR(maczero, addr.mac);
        addr.flags |= BCM_L2_CACHE_BPDU;
        rv = bcm_l2_cache_set(unit, arg_index, &addr, &idx_used);
        if (rv != BCM_E_NONE) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;
    }
    
    if (! sal_strcasecmp(subcmd, "show") || 
            ! sal_strcasecmp(subcmd, "-d")) {
        rv = DRV_DEV_PROP_GET(unit, 
                DRV_DEV_PROP_BPDU_NUM, (uint32 *) &count);
        if (rv != BCM_E_NONE) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("unit %d supports %d BPDU entries\n", unit, count);
        bcm_l2_cache_addr_t_init(&addr);
        for (idx = 0; idx < count; idx++) {
            addr.flags |= BCM_L2_CACHE_BPDU;
            rv = bcm_l2_cache_get(unit, idx, &addr);
            if (rv != BCM_E_NONE) {
                /* return BCM_E_BADID is the special case for bcm53242 */ 
                if (rv == BCM_E_NOT_FOUND || rv == BCM_E_BADID) {
                    cli_out("\tBPDU %d: -- Invalid --\n", idx);
                } else {
                    cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                    return CMD_FAIL;
                }
            } else {
                format_macaddr(buf, addr.mac);
                cli_out("\tBPDU %d: %s\n", idx, buf);
            }
        }
        return CMD_OK ;
    }
    return CMD_USAGE;
}

extern void 
_bcm_robo_l2_from_arl(int unit, bcm_l2_addr_t *l2addr, l2_arl_sw_entry_t *arl_entry);

/*
 * Used with the 'l2 watch' command below.
 */
static void
_l2_watch_cb(int unit,
         bcm_l2_addr_t *l2addr,
         int insert,
         void *userdata)
{
    int s;
    sal_thread_t main_thread;

    s = sal_splhi();

    main_thread = sal_thread_main_get();
    sal_thread_main_set(sal_thread_self());

    if(insert) {
        cli_out("L2 ADD: ");
        if(_robo_l2addr_dump(unit, l2addr, NULL)){
            cli_out("\n");
        }
    }
    else {
        cli_out("L2 DEL: ");
        if(_robo_l2addr_dump(unit, l2addr, NULL)){
            cli_out("\n");
        }
    }

    sal_thread_main_set(main_thread);

    sal_spl(s);
}


cmd_result_t
if_robo_l2(int unit, args_t *a)
{
    soc_control_t   *soc = SOC_CONTROL(unit);
    int idx = 0;
    char *subcmd = NULL;
    static pbmp_t arg_pbmp;
    pbmp_t pbmp;
    static int arg_static = 0, arg_trunk = 0, arg_l3if = 0,
        arg_scp = 0, arg_ds = 0, arg_dd = 0, arg_count = 1, arg_modid = 0,
        arg_vlan = VLAN_ID_DEFAULT, arg_tgid = 0, arg_cbit = 0, arg_hit = 0,
        arg_port, arg_cos = -1, arg_replace = 0, arg_pending = 0,
        arg_stonly = 0, arg_mcast = 0, arg_mconly = 0;
    int arg_newmodid = 0, arg_newport = 0, arg_newtgid=0, arg_del=0, 
        arg_vport = 0;
    soc_port_t p;
    int rv = CMD_OK;
    parse_table_t pt;
    cmd_result_t ret_code = 0;
    bcm_l2_addr_t l2addr;
    uint32  port = 0, fld_value = 0;
    uint64  temp_mac_field;
    uint8   temp_mac_addr[6];
    l2_arl_sw_entry_t l2_sw_entry, output, output1;
    
    /*
     * Initialize MAC address field for the user to the first real
     * address which does not conflict
     */
    sal_mac_addr_t default_macaddr = {0, 0, 0, 0, 0, 0X1};
    sal_mac_addr_t arg_macaddr     = {0, 0, 0, 0, 0, 0X1};
    
    COMPILER_64_ZERO(temp_mac_field);

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    /* Check valid device to operation on ...*/
    if (! sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if (!sal_strcasecmp(subcmd, "replace")) {
        uint32 flags = 0;

        arg_modid = 0;
        arg_port = arg_vlan = -1;
        arg_tgid = arg_newport = arg_newtgid = -1;
        arg_newmodid = arg_static = 0;
        arg_del = 1;
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Module",          PQ_DFL|PQ_INT,
                0, &arg_modid,      NULL);
        parse_table_add(&pt, "Port",            PQ_DFL|PQ_PORT,
                0, &arg_port,       NULL);
        parse_table_add(&pt, "MACaddress",      PQ_DFL|PQ_MAC,
                0, &arg_macaddr,    NULL);
        parse_table_add(&pt, "Vlanid",          PQ_DFL|PQ_HEX,
                0, &arg_vlan,       NULL);
        parse_table_add(&pt, "Trunk",           PQ_DFL|PQ_BOOL,
                0, &arg_trunk,      NULL);
        parse_table_add(&pt, "TrunkGroupID",    PQ_DFL|PQ_INT,
                0, &arg_tgid,       NULL);
        parse_table_add(&pt, "STatic",          PQ_DFL|PQ_BOOL,
                0, &arg_static,     NULL);
        parse_table_add(&pt, "PENDing",         PQ_DFL|PQ_BOOL,
                0, &arg_pending,    NULL);
        parse_table_add(&pt, "Delete",          PQ_DFL|PQ_BOOL,
                0, &arg_del,        NULL);
        parse_table_add(&pt, "NewModule",       PQ_DFL|PQ_INT,
                0, &arg_newmodid,   NULL);
        parse_table_add(&pt, "NewPort",         PQ_DFL|PQ_PORT,
                0, &arg_newport,    NULL);
        parse_table_add(&pt, "NewTrunkGroupID", PQ_DFL|PQ_INT,
                0, &arg_newtgid,    NULL);
                
        parse_table_add(&pt, "STOnly",          PQ_DFL|PQ_BOOL,
                0, &arg_stonly,     NULL);
        parse_table_add(&pt, "MCast",           PQ_DFL|PQ_BOOL,
                0, &arg_mcast,      NULL);
        parse_table_add(&pt, "MCOnly",          PQ_DFL|PQ_BOOL,
                0, &arg_mconly,     NULL);
        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }

        bcm_l2_addr_t_init(&l2addr, arg_macaddr, arg_vlan);
        if (arg_static) {
            flags |= BCM_L2_REPLACE_MATCH_STATIC;
#ifdef BCM_TB_SUPPORT
            flags |=  _BCM_TB_L2_REPLACE_MATCH_STATIC;
#endif  /* BCM_TB_SUPPORT */
        }
        if (arg_del) {
            flags |= BCM_L2_REPLACE_DELETE;
        }
        if (ENET_CMP_MACADDR(arg_macaddr, default_macaddr)) {
            flags |= BCM_L2_REPLACE_MATCH_MAC;
        }
        if (arg_vlan != -1) {
            flags |= BCM_L2_REPLACE_MATCH_VLAN;
        }
        if (arg_newtgid != - 1) {
            flags |= BCM_L2_REPLACE_NEW_TRUNK; 
        }
        if ((arg_port != - 1) && (arg_modid != - 1)) {
            flags |= BCM_L2_REPLACE_MATCH_DEST;
            l2addr.modid = arg_modid;
            l2addr.port = arg_port;
        }
        if (arg_pending) {
            flags |= BCM_L2_REPLACE_PENDING;
        }

#ifdef BCM_TB_SUPPORT        
        if (arg_stonly) {
            flags |= _BCM_TB_L2_REPLACE_MATCH_STATIC_ONLY;
        }
        if (arg_mcast) {
            flags |= _BCM_TB_L2_REPLACE_MATCH_MCAST;
        }
        if (arg_mconly) {
            flags |= _BCM_TB_L2_REPLACE_MATCH_MCAST_ONLY;
        }
#else   /* BCM_TB_SUPPORT */
        if (arg_stonly || arg_mcast || arg_mconly){
            cli_out(" Message: STOnly|MCast|MCOnly are not available!\n");
        }
#endif  /* BCM_TB_SUPPORT */

        if (arg_trunk && (arg_tgid != -1)) {
            l2addr.tgid = arg_tgid;
        }

        rv = bcm_l2_replace(unit, flags, &l2addr, arg_newmodid, 
                                arg_newport, arg_newtgid); 

        if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }

        return CMD_OK;
    } else if (! sal_strcasecmp(subcmd, "add") || 
            ! sal_strcasecmp(subcmd, "+")) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                (void *)(0), &arg_pbmp, NULL);
        parse_table_add(&pt, "MACaddress", PQ_DFL|PQ_MAC,
                0, &arg_macaddr, NULL);
        parse_table_add(&pt, "Vlanid", PQ_DFL|PQ_HEX,
                0, &arg_vlan, NULL);
        parse_table_add(&pt, "PRIority", PQ_DFL|PQ_HEX,
                0, &arg_cos, NULL);
        parse_table_add(&pt, "STatic", PQ_DFL|PQ_BOOL,
                0, &arg_static, NULL);
        parse_table_add(&pt, "HIT", PQ_DFL|PQ_BOOL,
                0, &arg_hit, NULL);
        parse_table_add(&pt, "SourceCosPriority", PQ_DFL|PQ_BOOL,
                0, &arg_scp, NULL);
        parse_table_add(&pt, "DiscardSource", PQ_DFL|PQ_BOOL,
                0, &arg_ds, NULL);
        parse_table_add(&pt, "DiscardDest", PQ_DFL|PQ_BOOL,
                0, &arg_dd, NULL);
        parse_table_add(&pt, "CPUmirror", PQ_DFL|PQ_BOOL,
                0, &arg_cbit, NULL);
        parse_table_add(&pt, "PENDing", PQ_DFL|PQ_BOOL,
                0, &arg_pending, NULL);
        parse_table_add(&pt, "VPort", PQ_DFL|PQ_INT,
                0, &arg_vport, NULL);
        parse_table_add(&pt, "Replace", PQ_DFL|PQ_BOOL,
                0, &arg_replace, NULL);

        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }
        pbmp = arg_pbmp;

        if (BCM_PBMP_IS_NULL(pbmp)) {
            cli_out("%s ERROR: empty port bitmap\n", ARG_CMD(a));
            return CMD_FAIL;
        }

        /*
         * If we are setting the range, the MAC address is incremented by
         * 1 for each port.
         */
        PBMP_ITER(pbmp, p) {
            bcm_l2_addr_init(&l2addr, arg_macaddr, arg_vlan);

            l2addr.tgid = arg_tgid;
            
            if (SOC_IS_ROBO_GE_SWITCH(unit)) {
                if (arg_cos < 0) {
                    arg_cos = 0;
                }
            } else {
                if ((arg_cos < 0) || (arg_cos > 3)) {
                      arg_cos = 0; /* If out of range, set to 0 */
                }
            }
            l2addr.cos_dst = arg_cos;
            /* Configure flags for SDK call */
            if (arg_static)
                l2addr.flags |= BCM_L2_STATIC;
            if (arg_hit){
                l2addr.flags |= BCM_L2_HIT;
            }
            if (arg_scp)
                l2addr.flags |= BCM_L2_COS_SRC_PRI;
            if (arg_dd)
                l2addr.flags |= BCM_L2_DISCARD_DST;
            if (arg_ds)
                l2addr.flags |= BCM_L2_DISCARD_SRC;
            if (arg_l3if)
                l2addr.flags |= BCM_L2_L3LOOKUP;
            if (arg_trunk)
                l2addr.flags |= BCM_L2_TRUNK_MEMBER;
            if (arg_cbit)
                l2addr.flags |= BCM_L2_COPY_TO_CPU;
            if (arg_replace)
                l2addr.flags |= BCM_L2_REPLACE_DYNAMIC;
            if (arg_pending){
                l2addr.flags |= BCM_L2_PENDING;
            }
            
#ifdef  BCM_TB_SUPPORT
            if (SOC_IS_TBX(unit)){
                _TB_SUBPORT_GPORT_ID_SET(l2addr.port, p, 0, arg_vport);
            } else {
                l2addr.port = p;
            }
#else
            l2addr.port = p;
#endif  /* BCM_TB_SUPPORT */
            cli_out("ADD: ");
            if(_robo_l2addr_dump(unit, &l2addr, NULL)){
                cli_out("\n");
            }

            if ((rv = bcm_l2_addr_add(unit, &l2addr))!= BCM_E_NONE) {
                cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }

            /* Set up for next call */
            increment_macaddr(arg_macaddr, 1);
        }

        return CMD_OK;
    } else if (! sal_strcasecmp(subcmd, "del") 
            || ! sal_strcasecmp(subcmd, "-")) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "MACaddress",      PQ_DFL|PQ_MAC,
                0, &arg_macaddr, NULL);
        parse_table_add(&pt, "Count",           PQ_DFL|PQ_INT,
                (void *)(1), &arg_count, NULL);
        parse_table_add(&pt, "Vlanid",          PQ_DFL|PQ_HEX,
                0, &arg_vlan, NULL);
        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }

        for (idx = 0; idx < arg_count; idx++) {
            rv = bcm_l2_addr_get(unit, arg_macaddr, arg_vlan, &l2addr);

            if (rv < 0) {
                cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }

            cli_out("DEL : ");
            if(_robo_l2addr_dump(unit, &l2addr, NULL)){
                cli_out("\n");
            }

            if ((rv = bcm_l2_addr_delete(unit, arg_macaddr, arg_vlan)) < 0) {
                cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }

            increment_macaddr(arg_macaddr, 1);
        }

        return CMD_OK;

    } else if (! sal_strcasecmp(subcmd, "learn")){
        char    *learncmd = NULL, *showcmd = NULL;
        
        if ((learncmd = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }
         
        BCM_PBMP_CLEAR(arg_pbmp);   
            
        if (!sal_strcasecmp(learncmd, "control")){
            uint32  cfg_flags = 0;
            int     show_config = FALSE;
            int     arg_salearn = 1, arg_swlearn = 0;
            int     arg_cpu = 0, arg_forward = 1;
            
            if ((showcmd= ARG_GET(a)) == NULL) {
                return CMD_USAGE;
            } else {
                if (!sal_strcasecmp(showcmd, "show")){
                    show_config = TRUE;
                } else if (!sal_strcasecmp(showcmd, "set")){
                    show_config = FALSE;
                }
            }
            
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                    (void *)(0), &arg_pbmp, NULL);
            parse_table_add(&pt, "SALearn", PQ_DFL|PQ_BOOL,
                    0, &arg_salearn, NULL);
            parse_table_add(&pt, "CPU", PQ_DFL|PQ_BOOL,
                    0, &arg_cpu, NULL);
            parse_table_add(&pt, "FWD", PQ_DFL|PQ_BOOL,
                    0, &arg_forward, NULL);
            parse_table_add(&pt, "SWLearn", PQ_DFL|PQ_BOOL,
                    0, &arg_swlearn, NULL);
            
            if (!parseEndOk( a, &pt, &ret_code)) {
                cli_out("PortBitMap must be assigned!\n");
                return ret_code;
            }
            pbmp = arg_pbmp;
            
            if (BCM_PBMP_IS_NULL(pbmp)) {
                cli_out("%s ERROR: empty port bitmap\n", ARG_CMD(a));
                return CMD_FAIL;
            }
            
            if (show_config){
                cli_out("\nCurrent learning mode configuration :\n");
            } else {
                cfg_flags = ((arg_salearn) ? BCM_PORT_LEARN_ARL : 0) | 
                        ((arg_cpu) ? BCM_PORT_LEARN_CPU : 0) | 
                        ((arg_forward) ? BCM_PORT_LEARN_FWD : 0) | 
                        ((arg_swlearn) ? BCM_PORT_LEARN_PENDING : 0);
            }
            
            PBMP_ITER(pbmp, port) {
                if (show_config){
                    cli_out("\t Port %d : ", port);
                    rv = bcm_port_learn_get(unit, port, &cfg_flags);
                } else {
                    cli_out("\t Port %d : set to ", port);
                    rv = bcm_port_learn_set(unit, port, cfg_flags);
                }
                if (rv){
                    cli_out("ERROR! %s\n", bcm_errmsg(rv));
                } else {
                    cli_out("(0x%x):", cfg_flags);
                    cli_out("SA_Learn(%d) SLF2CPU(%d) ",
                            (cfg_flags & BCM_PORT_LEARN_ARL) ? 1 : 0,
                            (cfg_flags & BCM_PORT_LEARN_CPU) ? 1 : 0);
                    cli_out("SLF_FWD(%d) SW_Learn(%d)\n",
                            (cfg_flags & BCM_PORT_LEARN_FWD) ? 1 : 0,
                            (cfg_flags & BCM_PORT_LEARN_PENDING) ? 1 : 0);
                }
            }
            
        } else if (!sal_strcasecmp(learncmd, "limit")){
            cli_out("\n\t Pleaes set Learn limimt through 'AUTH' command!\n");
        } else {
            return CMD_USAGE;
        }
            
    } else if (! sal_strcasecmp(subcmd, "show") 
            || ! sal_strcasecmp(subcmd, "-d")) {
        /*
         * If no port specified, show all ports.
         */

        if (ARG_CNT(a) == 0) {
            pbmp = PBMP_ALL(unit);
        } else {
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "PortBitMap",     PQ_DFL|PQ_PBMP,
                    (void *)(0), &arg_pbmp, NULL);
        
            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }
        
            SOC_PBMP_ASSIGN(pbmp, arg_pbmp);
            SOC_PBMP_AND(pbmp, PBMP_ALL(unit));
        }
        if(soc->arl_table){
            LOG_INFO(BSL_LS_APPL_ARL,
                     (BSL_META_U(unit,
                                 "showing SW ARL table.. \n")));
            rv = bcm_l2_traverse(unit, _robo_l2addr_dump, NULL);

            if (rv < 0) {
                cli_out("%s: ERROR: bcm_l2_traverse failed %s\n",
                        ARG_CMD(a), bcm_errmsg(rv));
            }
            
        }else {
            int entry_id = 0;
            
            LOG_INFO(BSL_LS_APPL_ARL,
                     (BSL_META_U(unit,
                                 "showing HW ARL table..\n")));
            sal_memset(&l2_sw_entry, 0, sizeof(l2_arl_sw_entry_t));
            PBMP_ITER(pbmp, port) {
                /* step 1: start the search of valid ARL entries */
                soc_arl_search_valid(unit, _ARL_SEARCH_VALID_OP_START, 
                    NULL, NULL, NULL);
                /* step 2: get the valid ARL entries if found */
                /* write the port number into arl entry */
                rv = SOC_E_EXISTS;

                while (rv == SOC_E_EXISTS) {
                    sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
                    sal_memset(&output1, 0, sizeof(l2_arl_sw_entry_t));
                    rv = soc_arl_search_valid(unit, _ARL_SEARCH_VALID_OP_GET, 
                        (uint32 *)&entry_id, (uint32 *)&output, 
                        (uint32 *)&output1);
                    if (rv == SOC_E_EXISTS) { 
                        if (SOC_IS_ROBO_ARCH_VULCAN(unit) || 
                            SOC_IS_DINO16(unit)) {
                        
                            /* Entry 0 */
                            DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                                    DRV_MEM_FIELD_VALID, (uint32 *)&output, 
                                    &fld_value);
                            if (fld_value){
                                _bcm_robo_l2_from_arl(unit, &l2addr, &output);
                                if (port != l2addr.port){
                                    soc_arl_search_valid(unit, _ARL_SEARCH_VALID_OP_NEXT, 
                                            NULL, NULL, NULL);
                                    continue;
                                }
                                cli_out(" id(%05d):", entry_id *2);
                                if(_robo_l2addr_dump(unit, &l2addr, NULL)){
                                    cli_out("\n");
                                }
                            }
                            /* Entry 1 */
                            DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                                    DRV_MEM_FIELD_VALID, (uint32 *)&output1, 
                                    &fld_value);
                            if (fld_value){
                                _bcm_robo_l2_from_arl(unit,&l2addr, &output1);
                                if (port != l2addr.port){
                                    soc_arl_search_valid(unit, _ARL_SEARCH_VALID_OP_NEXT, 
                                            NULL, NULL, NULL);
                                    continue;
                                }
                                cli_out(" id(%05d):", entry_id*2+1);
                                if(_robo_l2addr_dump(unit, &l2addr, NULL)){
                                    cli_out("\n");
                                }
                            }
                        } else {

                            bcm_port_t  p = 0;                               

                            _bcm_robo_l2_from_arl(unit, &l2addr, &output);


                            if (l2addr.flags & BCM_L2_MCAST) {
                                if(port != 0) {
                                    soc_arl_search_valid(unit, _ARL_SEARCH_VALID_OP_NEXT, 
                                            NULL, NULL, NULL);
                                    continue;
                                }
                            } else {
                                if (SOC_IS_TBX(unit)){
                                    if (BCM_GPORT_IS_SET(l2addr.port)) {
                                        if (bcm_port_local_get(
                                                unit, l2addr.port, &p)){
                                            cli_out("GPORT not parsed!\n");
                                            soc_arl_search_valid(unit, _ARL_SEARCH_VALID_OP_NEXT, 
                                                    NULL, NULL, NULL);
                                            continue;
                                        }
                                    } else {
                                        p = l2addr.port;
                                    }
                                }

                                if (p != port){
                                    soc_arl_search_valid(unit, _ARL_SEARCH_VALID_OP_NEXT, 
                                            NULL, NULL, NULL);
                                    continue;
                                }
                            }
                            cli_out(" id(%05d):", entry_id);
                            if(_robo_l2addr_dump(unit, &l2addr, NULL)){
                                cli_out("\n");
                            }                                            
                        }
                        soc_arl_search_valid(unit, _ARL_SEARCH_VALID_OP_NEXT, 
                                NULL, NULL, NULL);
                    }
                }
            }
        }
        cli_out("\n");
        return CMD_OK;
    } else if (! sal_strcasecmp(subcmd, "clear") 
            || ! sal_strcasecmp(subcmd, "clr")) {

        char *static_str;
        int old_modid, old_port, old_vlan, old_tgid, old_static;
        enum {
            MAC_PRESENT         = 0x00000001,
            MODID_PRESENT       = 0x00000002,
            PORT_PRESENT        = 0x00000004,
            VLANID_PRESENT      = 0x00000008,
            TGID_PRESENT        = 0x00000010
        } arg_combination;
        
        /*
         * Clear the ARL on a per port or per switch basis. Walk ARL
         * memory finding matching ports such as the user specified,
         * issue BCM L2 SDK calls for clearing the entries.
         */

        /* Save current arguments */
        old_modid = arg_modid;
        old_port = arg_port;
        old_vlan = arg_vlan;
        old_tgid = arg_tgid;
        old_static = arg_static;
        
        arg_modid = -1;
        arg_port = -1;
        arg_vlan = -1;
        arg_tgid = -1;
        arg_static = TRUE;
        ENET_SET_MACADDR(arg_macaddr, _mac_all_zeroes);
        arg_combination = 0;

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Module",          PQ_DFL|PQ_INT,
            0, &arg_modid,  NULL);
        parse_table_add(&pt, "Port",            PQ_DFL|PQ_PORT,
            0, &arg_port,   NULL);
        parse_table_add(&pt, "Vlanid",             PQ_DFL|PQ_HEX,
            0, &arg_vlan,    NULL);
        parse_table_add(&pt, "TrunkGroupID",     PQ_DFL|PQ_INT,
            0, &arg_tgid,    NULL);
        parse_table_add(&pt, "MACaddress",      PQ_DFL|PQ_MAC,
                        0, &arg_macaddr,NULL);
        parse_table_add(&pt, "STatic",             PQ_DFL|PQ_BOOL,
            0, &arg_static,    NULL);
        parse_table_add(&pt, "PENDing",         PQ_DFL|PQ_BOOL,
                0, &arg_pending,    NULL);

        ret_code = CMD_OK;

        if (!ARG_CNT(a)) {
            /*
             * Restore arguments for parseEndOk below to print
             * correct settings.
             */
            if (arg_modid == -1) arg_modid = old_modid;
            if (arg_port == -1) arg_port = old_port;
            if (arg_vlan == -1) arg_vlan = old_vlan;
            if (arg_tgid == -1) arg_tgid = old_tgid;
            if (arg_static == -1) arg_static = old_static;
        }

        if (!parseEndOk(a, &pt, &ret_code)) {
            goto done;
        }

        /*
         * Notice which arguments were supplied
         */
        if (arg_modid >=0) {
            arg_combination |= MODID_PRESENT;
        }

        if (arg_port >= 0) {
            arg_combination |= PORT_PRESENT;
        }

        if (ENET_CMP_MACADDR(arg_macaddr, _mac_all_zeroes)) {
            arg_combination |= MAC_PRESENT;
        }

        if (arg_tgid >= 0) {
            arg_combination |= TGID_PRESENT;
        }

        if (arg_vlan >= 0) {
            arg_combination |= VLANID_PRESENT;
        }

        if (arg_pending){
            arg_combination |= BCM_L2_PENDING;
        }

        arg_static = arg_static ? BCM_L2_DELETE_STATIC : 0;
        if (arg_pending == TRUE) {
            static_str = arg_static ? "static and pending" : "pending";
            arg_static |= BCM_L2_DELETE_PENDING;
        } else {
            static_str = arg_static ? "static and non-static" : "non-static";
        }
        switch ((int)arg_combination) {
            case PORT_PRESENT:
                cli_out("%s: Deleting %s addresses by port, local module ID\n",
                        ARG_CMD(a), static_str);
                rv = bcm_l2_addr_delete_by_port(unit,
                                -1, arg_port,
                                arg_static);
            break;
    
            case PORT_PRESENT | MODID_PRESENT:
                cli_out("%s: Deleting %s addresses by module/port\n",
                        ARG_CMD(a), static_str);
                rv = bcm_l2_addr_delete_by_port(unit,
                                arg_modid, arg_port,
                                arg_static);
            break;
    
            case VLANID_PRESENT:
                cli_out("%s: Deleting %s addresses by VLAN\n",
                        ARG_CMD(a), static_str);
                rv = bcm_l2_addr_delete_by_vlan(unit,
                                arg_vlan,
                                arg_static);
                break;
    
            case TGID_PRESENT:
                cli_out("%s: Deleting %s addresses by trunk ID\n",
                        ARG_CMD(a), static_str);
                rv = bcm_l2_addr_delete_by_trunk(unit,
                                 arg_tgid,
                                 arg_static);
    
            break;
            
            case MAC_PRESENT:
                cli_out("%s: Deleting %s addresses by MAC\n",
                        ARG_CMD(a), static_str);
                rv = bcm_l2_addr_delete_by_mac(unit, 
                                               arg_macaddr, 
                                               arg_static);
                break;
    
            case MAC_PRESENT | VLANID_PRESENT:
                cli_out("%s: Deleting an address by MAC and VLAN\n",
                        ARG_CMD(a));
                rv = bcm_l2_addr_delete(unit, 
                                        arg_macaddr, 
                                        arg_vlan);
    
                break;
    
            case MAC_PRESENT | PORT_PRESENT:
                cli_out("%s: Deleting %s addresses by MAC and port\n",
                        ARG_CMD(a), static_str);
                rv = bcm_l2_addr_delete_by_mac_port(unit, 
                                                    arg_macaddr, 
                                                    -1, arg_port,
                                                    arg_static);
                break;
    
            case MAC_PRESENT | PORT_PRESENT | MODID_PRESENT:
                cli_out("%s: Deleting %s addresses by MAC and module/port\n",
                        ARG_CMD(a), static_str);
                rv = bcm_l2_addr_delete_by_mac_port(unit, 
                                                    arg_macaddr, 
                                                    arg_modid, arg_port,
                                                    arg_static);
                break;
    
            case VLANID_PRESENT | PORT_PRESENT:
                cli_out("%s: Deleting %s addresses by VLAN and port\n",
                        ARG_CMD(a), static_str);
                rv = bcm_l2_addr_delete_by_vlan_port(unit,
                                 arg_vlan,
                                 -1, arg_port,
                                 arg_static);
                break;
    
            case VLANID_PRESENT | PORT_PRESENT | MODID_PRESENT:
                cli_out("%s: Deleting %s addresses by VLAN and module/port\n",
                        ARG_CMD(a), static_str);
                rv = bcm_l2_addr_delete_by_vlan_port(unit,
                                 arg_vlan,
                                 arg_modid, arg_port,
                                 arg_static);
                break;
    
            case VLANID_PRESENT |TGID_PRESENT:
                cli_out("%s: Deleting %s addresses by trunk ID\n",
                        ARG_CMD(a), static_str);
                rv = bcm_l2_addr_delete_by_vlan_trunk(unit,
                                arg_vlan,
                                arg_tgid,
                                arg_static);
        
            break;
            default:
                cli_out("%s: Unknown argument combination\n", ARG_CMD(a));
                ret_code = CMD_USAGE;
            break;
        }

done:
        /* Restore unused arguments */
        if (arg_modid == -1) arg_modid = old_modid;
        if (arg_port == -1) arg_port = old_port;
        if (arg_vlan == -1) arg_vlan = old_vlan;
        if (arg_tgid == -1) arg_tgid = old_tgid;
        if (arg_static == -1) arg_static = old_static;

        if ((ret_code == CMD_OK) && (rv < 0)) {
            cli_out("ERROR: %s\n", bcm_errmsg(rv));
            return CMD_FAIL;
        }

        return ret_code;
    } else if (! sal_strcasecmp(subcmd, "dump")) {
        if(soc->arl_table){
            int index_min = 0,index_count = 0;
            uint32 idx = 0, valid = 0;
            uint32 count=0;            

            index_min = SOC_MEM_BASE(unit, INDEX(L2_ARLm));
            index_count = SOC_MEM_SIZE(unit, INDEX(L2_ARLm));
            for (idx = index_min; idx < index_count; idx++) {
                sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
                if(!ARL_ENTRY_NULL(&soc->arl_table[idx])) {
                    ARL_SW_TABLE_LOCK(soc);
                    sal_memcpy(&output, &soc->arl_table[idx], 
                            sizeof(l2_arl_sw_entry_t));
                    ARL_SW_TABLE_UNLOCK(soc);
                } else{
                    continue;
                }
                DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                        (uint32 *)&output, &valid);
                /* for TB device, could be valid or pending status */
                if (valid){
                    count++;
                    DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                            (uint32 *)&output, (void *)&temp_mac_field);
                    SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                    cli_out("\nMac = %02x-%02x-%02x-%02x-%02x-%02x ",
                            temp_mac_addr[0], temp_mac_addr[1],
                            temp_mac_addr[2], temp_mac_addr[3],
                            temp_mac_addr[4], temp_mac_addr[5]);
                    DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                            (uint32 *)&output, &fld_value);
                    cli_out("VLAN ID = %d, ",fld_value);
                    if (temp_mac_addr[0] & 0x01) { /* mcast address */
                        /* The the multicast format didn't define, we need 
                            collect 3 fields to get the multicast index value.
                            multicast index : bit 55~48
                            age : bit 55
                            priority : bit 54~53
                            port id : bit 52~48
                        */
                        DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                                DRV_MEM_FIELD_DEST_BITMAP, 
                                (uint32 *)&output, &fld_value);
                        cli_out("Multicast Index = %d, ", fld_value);
                        DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                                DRV_MEM_FIELD_STATIC, 
                                (uint32 *)&output, &fld_value);
                        if (fld_value) {
                            cli_out("STATIC");
                        }
                    } else { /* unicast address */
                        DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                                DRV_MEM_FIELD_SRC_PORT, 
                                (uint32 *)&output, &fld_value);
                        cli_out("Port = %d, ", fld_value);
                        if (!SOC_IS_TBX(unit)){
                            DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                                    DRV_MEM_FIELD_PRIORITY, 
                                    (uint32 *)&output, &fld_value);
                            if (SOC_IS_ROBO_GE_SWITCH(unit)) {
                                cli_out("Priority = %d", fld_value);
                            } else {
                                cli_out("COS Queue = %d", fld_value);
                            }
                        }
                    
                        DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                                DRV_MEM_FIELD_STATIC, 
                                (uint32 *)&output, &fld_value);
                        if (fld_value) {
                            cli_out(", STATIC");
                        }
                        DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                                DRV_MEM_FIELD_AGE, 
                                (uint32 *)&output, &fld_value);
                        /* TB has different hit value, but no specific parsing here */
                        if (fld_value) {  
                            cli_out(", Hit");
                        }
                    }
                }
            }
            LOG_INFO(BSL_LS_APPL_L2TABLE,
                     (BSL_META_U(unit,
                                 "\n dump %d entries\n"),count));
        }else {
            int entry_id = 0;
            /* step 1: start the search of valid ARL entries */
            soc_arl_search_valid(unit, _ARL_SEARCH_VALID_OP_START, 
                NULL, NULL, NULL);
            /* step 2: get valid ARL entries */
            rv = SOC_E_EXISTS;
            while (rv == SOC_E_EXISTS) {
                sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
                sal_memset(&output1, 0, sizeof(l2_arl_sw_entry_t));
                rv = soc_arl_search_valid(unit, 
                    _ARL_SEARCH_VALID_OP_GET, 
                    (uint32 *)&entry_id, (uint32 *)&output, 
                    (uint32 *)&output1);
                if (rv == SOC_E_EXISTS) {
                    if (SOC_IS_ROBO_ARCH_VULCAN(unit) || SOC_IS_DINO16(unit)) {

                        /* Entry 0 */
                        DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                            DRV_MEM_FIELD_VALID, 
                                (uint32 *)&output, &fld_value);
                        if (fld_value){
                            _bcm_robo_l2_from_arl(unit, &l2addr, &output);
                            cli_out(" id(%05d):", entry_id*2);
                            if(_robo_l2addr_dump(unit, &l2addr, NULL)){
                                cli_out("\n");
                            }
                        }
                        /* Entry 1 */
                        DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                                (uint32 *)&output1, &fld_value);
                        if (fld_value){
                            _bcm_robo_l2_from_arl(unit, &l2addr, &output1);
                            cli_out(" id(%05d):", entry_id*2+1);
                            if(_robo_l2addr_dump(unit, &l2addr, NULL)){
                                cli_out("\n");
                            }
                        }
                    } else {
                        _bcm_robo_l2_from_arl(unit, &l2addr, &output);
                        cli_out(" id(%05d):", entry_id);
                        if (_robo_l2addr_dump(unit, &l2addr, NULL)){
                            cli_out("\n");
                        }
                    }
                    soc_arl_search_valid(unit, _ARL_SEARCH_VALID_OP_NEXT,
                        NULL, NULL, NULL);
                }
            }
        }

        cli_out("\n");
        return CMD_OK;
    } else if (!sal_strcasecmp(subcmd, "conflict")) {
        bcm_l2_addr_t        addr;
        bcm_l2_addr_t        cf[ROBO_MAX_L2_BUCKET_SIZE];
        int            cf_count, i;

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "MACaddress",      PQ_DFL|PQ_MAC,
                0, &arg_macaddr, NULL);
        parse_table_add(&pt, "Vlanid",          PQ_DFL|PQ_HEX,
                0, &arg_vlan,    NULL);
        if (!parseEndOk(a, &pt, &ret_code))
            return ret_code;

        bcm_l2_addr_t_init(&addr, arg_macaddr, arg_vlan);
        
        if ((rv = bcm_l2_conflict_get(unit, &addr,
                cf, sizeof (cf) / sizeof (cf[0]),
                &cf_count)) < 0) {
            cli_out("%s: bcm_l2_conflict_get failed: %s\n",
                    ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        
        for (i = 0; i < cf_count; i++) {
            cli_out("conflict: ");
            if(_robo_l2addr_dump(unit, &cf[i], NULL)){
                cli_out("\n");
            }
        }
        
        return CMD_OK;
    } else if (!sal_strcasecmp(subcmd, "watch")) {
        static int watch = 0;
        char* opt = ARG_GET(a);
        if(opt == NULL) {
            cli_out("L2 watch is%srunning.\n",
                    (watch) ? " " : " not ");
            return CMD_OK;
        } else if(!sal_strcasecmp(opt, "start")) {
            watch = 1;
            bcm_l2_addr_register(unit, _l2_watch_cb, NULL);
            return CMD_OK;
        } else if(!sal_strcasecmp(opt, "stop")) {
            watch = 0;
            bcm_l2_addr_unregister(unit, _l2_watch_cb, NULL);
            return CMD_OK;
        } else {
            return CMD_USAGE;
        }
    } else if (!sal_strcasecmp(subcmd, "cache") ||
            !sal_strcasecmp(subcmd, "c") ) {
        int cidx;
        char *cachecmd = NULL;
        char str[16];
        sal_mac_addr_t arg_macaddr;
        int arg_cidx, arg_ccount, idx_max, arg_dstports = 0;
        bcm_l2_cache_addr_t l2caddr;

        if ((cachecmd = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }

        sal_memset(arg_macaddr, 0, sizeof (sal_mac_addr_t));
        BCM_PBMP_CLEAR(arg_pbmp);

        arg_cidx = -1;
        arg_ccount = 1;

        if (!sal_strcasecmp(cachecmd, "add") ||
                !sal_strcasecmp(cachecmd, "+") ) {

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "CacheIndex",          PQ_DFL|PQ_INT,
                            0, &arg_cidx,       NULL);
            parse_table_add(&pt, "MACaddress",          PQ_DFL|PQ_MAC,
                            0, &arg_macaddr,    NULL);
            parse_table_add(&pt, "pbmp",                PQ_DFL|PQ_PBMP,
                            0, &arg_pbmp,      NULL);
            parse_table_add(&pt, "DestPorts",         PQ_DFL|PQ_BOOL,
                            0, &arg_dstports,    NULL);

            if (!parseEndOk(a, &pt, &ret_code)) {
                return ret_code;
            }

            bcm_l2_cache_addr_t_init(&l2caddr);

            ENET_COPY_MACADDR(arg_macaddr, l2caddr.mac);

            /* Configure flags for SDK call */
            if (arg_dstports) {
                l2caddr.flags |= BCM_L2_CACHE_DESTPORTS;
                l2caddr.dest_ports = arg_pbmp;
            }

            _robo_dump_l2_cache_addr(unit, "ADD CACHE: ", &l2caddr);

            if ((rv = bcm_l2_cache_set(unit, arg_cidx, &l2caddr, 
                                       &cidx)) != BCM_E_NONE) {
                cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }

            cli_out(" => using index %d\n", cidx);

            return CMD_OK;
        } else if (!sal_strcasecmp(cachecmd, "del") ||
                 !sal_strcasecmp(cachecmd, "-")) {

            if (ARG_CNT(a)) {
                parse_table_init(unit, &pt);
                parse_table_add(&pt, "CacheIndex",      PQ_DFL|PQ_INT,
                                0, &arg_cidx,   NULL);
                parse_table_add(&pt, "Count",           PQ_DFL|PQ_INT,
                                0, &arg_ccount, NULL);
                if (!parseEndOk(a, &pt, &ret_code)) {
                    return ret_code;
                }
            }

            if (arg_cidx == -1) {
                cli_out("%s ERROR: no index specified\n", ARG_CMD(a));
                return CMD_FAIL;
            }
            
            bcm_l2_cache_addr_t_init(&l2caddr);

            for (idx = 0; idx < arg_ccount; idx++) {
                l2caddr.flags |= BCM_L2_CACHE_DESTPORTS;
                if ((rv = bcm_l2_cache_get(unit, arg_cidx, &l2caddr)) < 0) {
                    cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                    return CMD_FAIL;
                }

                _robo_dump_l2_cache_addr(unit, "DEL CACHE: ", &l2caddr);

                cli_out(" => index %d\n", arg_cidx);

                if ((rv = bcm_l2_cache_delete(unit, arg_cidx)) != BCM_E_NONE) {
                    cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                    return CMD_FAIL;
                }
                arg_cidx++;
            }

            return CMD_OK;
        } else if (!sal_strcasecmp(cachecmd, "show") ||
                 !sal_strcasecmp(cachecmd, "-d")) {

            if ((rv = bcm_l2_cache_size_get(unit, &idx_max)) < 0) {
                cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }

            bcm_l2_cache_addr_t_init(&l2caddr);
            for (idx = 0; idx <= idx_max; idx++) {
                l2caddr.flags |= BCM_L2_CACHE_DESTPORTS;
                if (bcm_l2_cache_get(unit, idx, &l2caddr) == BCM_E_NONE) {
                    sal_sprintf(str, " %4d : ", idx);
                    _robo_dump_l2_cache_addr(unit, str, &l2caddr);
                }
            }
            
            return CMD_OK;
        } else if (!sal_strcasecmp(cachecmd, "clear") ||
                 !sal_strcasecmp(cachecmd, "clr")) {

            if (ARG_CNT(a)) {
                parse_table_init(unit, &pt); 
                if (!parseEndOk(a, &pt, &ret_code)) {
                    return ret_code;
                }
            }

            if ((rv = bcm_l2_cache_delete_all(unit)) != BCM_E_NONE) {
                cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            } 

            return CMD_OK;
        } else {
            return CMD_USAGE;
        }
    } else {
        return CMD_USAGE;
    }

    return CMD_FAIL;
}


/*
 * Multicast command
 */

cmd_result_t
if_robo_mcast(int unit, args_t *a)
{
    char *subcmd;
    int r;
    static sal_mac_addr_t mac_address;
    static int initted = 0;
    static int vid=1;
    static int mcindex = -1;
    static pbmp_t pbmp;
    parse_table_t pt;
    cmd_result_t ret_code;
    bcm_mcast_addr_t mcaddr;
    pbmp_t rtrpbmp;
    int port;

    if (! initted) {
        mac_address[0] = 0x1;
        mac_address[1] = 0;
        mac_address[2] = 0;
        mac_address[3] = 0;
        mac_address[4] = 0;
        mac_address[5] = 0x1;
        initted = 1;
    }

    if (! sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    SOC_PBMP_AND(pbmp, PBMP_ALL(unit));
    mcindex = -1;
    if (sal_strcasecmp(subcmd, "add") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "MACaddress", PQ_DFL|PQ_MAC,
                0, &mac_address,NULL);
        parse_table_add(&pt, "Vlanid", PQ_DFL|PQ_HEX,
                0, &vid, NULL);
        parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                (void *)(0), &pbmp, NULL);
        parse_table_add(&pt, "Index",     PQ_DFL|PQ_INT,
            0, &mcindex,    NULL);
        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }
        bcm_mcast_addr_init(&mcaddr, mac_address, vid);
        mcaddr.pbmp = pbmp;
        mcaddr.l2mc_index = mcindex;
        if (mcindex == -1) {
            r = bcm_mcast_addr_add(unit, &mcaddr);
        } else {
            r = bcm_mcast_addr_add_w_l2mcindex(unit, &mcaddr);
        }
        if (r < 0) {
            cli_out("ERROR: %s\n", bcm_errmsg(r));
            return CMD_FAIL;
        }
        return CMD_OK;
    }

    if (sal_strncasecmp(subcmd, "del", 3) == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "MACaddress", PQ_DFL|PQ_MAC,
                0, &mac_address,NULL);
        parse_table_add(&pt, "Vlanid", PQ_DFL|PQ_HEX,
                0, &vid, NULL);
        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }
        if ((r = bcm_mcast_addr_remove(unit, mac_address, vid)) < 0) {
            cli_out("bcm_mcast_addr_remove failed ERROR: %s\n",
                    bcm_errmsg(r));
            return CMD_FAIL;
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "join") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "MACaddress", PQ_DFL|PQ_MAC,
                0, &mac_address,NULL);
        parse_table_add(&pt, "Vlanid", PQ_DFL|PQ_HEX,
                0, &vid, NULL);
        parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                (void *)(0), &pbmp, NULL);
        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }
        PBMP_ITER(pbmp, port) {
            r = bcm_mcast_join(unit, mac_address, vid, port,
                       &mcaddr, &rtrpbmp);
            if (r < 0) {
                cli_out("ERROR: %s %s port %s failed: %s\n",
                        ARG_CMD(a), subcmd, SOC_PORT_NAME(unit, port),
                        bcm_errmsg(r));
                return CMD_FAIL;
            }
            switch (r) {
                case BCM_MCAST_JOIN_ADDED:
                    cli_out("port %s added\n", SOC_PORT_NAME(unit, port));
                    break;
                case BCM_MCAST_JOIN_UPDATED:
                    cli_out("port %s updated\n", SOC_PORT_NAME(unit, port));
                    break;
                default:
                    cli_out("port %s complete, return %d\n",
                            SOC_PORT_NAME(unit, port), r);
                    break;
            }
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "leave") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "MACaddress", PQ_DFL|PQ_MAC,
                0, &mac_address,NULL);
        parse_table_add(&pt, "Vlanid", PQ_DFL|PQ_HEX,
                0, &vid, NULL);
        parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                (void *)(0), &pbmp, NULL);
        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }
        PBMP_ITER(pbmp, port) {
            r = bcm_mcast_leave(unit, mac_address, vid, port);
            if (r < 0) {
                cli_out("ERROR: %s %s port %s failed: %s\n",
                        ARG_CMD(a), subcmd, SOC_PORT_NAME(unit, port),
                        bcm_errmsg(r));
                return CMD_FAIL;
            }
            switch (r) {
                case BCM_MCAST_LEAVE_NOTFOUND:
                    cli_out("port %s not found\n", SOC_PORT_NAME(unit, port));
                    break;
                case BCM_MCAST_LEAVE_DELETED:
                    cli_out("port %s deleted\n", SOC_PORT_NAME(unit, port));
                    break;
                case BCM_MCAST_LEAVE_UPDATED:
                    cli_out("port %s updated\n", SOC_PORT_NAME(unit, port));
                    break;
                default:
                    cli_out("port %s complete, return %d\n",
                            SOC_PORT_NAME(unit, port), r);
                    break;
            }
        }
        return CMD_OK;
    }
    if (sal_strcasecmp(subcmd, "padd") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "MACaddress",      PQ_DFL|PQ_MAC,
              0, &mac_address,  NULL);
        parse_table_add(&pt, "Vlanid",          PQ_DFL|PQ_HEX,
              0, &vid,          NULL);
        parse_table_add(&pt, "PortBitMap",      PQ_DFL|PQ_PBMP,
              (void *)(0), &pbmp, NULL);
        if (!parseEndOk(a, &pt, &ret_code))
            return ret_code;
        bcm_mcast_addr_t_init(&mcaddr, mac_address, vid);
        SOC_PBMP_ASSIGN(mcaddr.pbmp, pbmp);
        r = bcm_mcast_port_add(unit, &mcaddr);
        if (r < 0) {
            cli_out("ERROR: %s %s ports failed: %s\n",
                    ARG_CMD(a), subcmd, bcm_errmsg(r));
            return CMD_FAIL;
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "premove") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "MACaddress",      PQ_DFL|PQ_MAC,
                0, &mac_address, NULL);
        parse_table_add(&pt, "Vlanid",          PQ_DFL|PQ_HEX,
                0, &vid,    NULL);
        parse_table_add(&pt, "PortBitMap",      PQ_DFL|PQ_PBMP,
                (void *)(0), &pbmp, NULL);
        if (!parseEndOk(a, &pt, &ret_code))
            return ret_code;
        bcm_mcast_addr_t_init(&mcaddr, mac_address, vid);
        SOC_PBMP_ASSIGN(mcaddr.pbmp, pbmp);
        r = bcm_mcast_port_remove(unit, &mcaddr);
        if (r < 0) {
            cli_out("ERROR: %s %s ports failed: %s\n",
                    ARG_CMD(a), subcmd, bcm_errmsg(r));
            return CMD_FAIL;
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "pget") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "MACaddress",      PQ_DFL|PQ_MAC,
                        0, &mac_address, NULL);
        parse_table_add(&pt, "Vlanid",          PQ_DFL|PQ_HEX,
                        0, &vid,    NULL);
        if (!parseEndOk(a, &pt, &ret_code))
            return ret_code;
        bcm_mcast_addr_t_init(&mcaddr, mac_address, vid);
        SOC_PBMP_ASSIGN(mcaddr.pbmp, pbmp);
        r = bcm_mcast_port_get(unit, mac_address, vid, &mcaddr);
        if (r < 0) {
            cli_out("ERROR: %s %s ports failed: %s\n",
                    ARG_CMD(a), subcmd, bcm_errmsg(r));
            return CMD_FAIL;
        }

        cli_out("mac=%02x:%02x:%02x:%02x:%02x:%02x vlan=%d",
                mcaddr.mac[0], mcaddr.mac[1], mcaddr.mac[2],
                mcaddr.mac[3], mcaddr.mac[4], mcaddr.mac[5], mcaddr.vid);

        if (SOC_INFO(unit).port_num > 32) {
            cli_out(" mc_pbmp=0x%x 0x%x\n", SOC_PBMP_WORD_GET(mcaddr.pbmp, 1), 
                    SOC_PBMP_WORD_GET(mcaddr.pbmp, 0));
        } else {
            cli_out(" mc_pbmp=0x%x\n", SOC_PBMP_WORD_GET(mcaddr.pbmp, 0));
        }
    
        return CMD_OK;
    }

    return CMD_USAGE;
}

/*
 * L2 Mode : handling the ARL thread for syncronize SW/HW l2 table.
 */

char if_robo_l2mode_usage[] =
    "Parameters: [on|off]\n\t"
    "Starts or stop the L2 shadow table update thread.\n\t"
    "The task reads the entire L2 table and issues updates to the L2 shadow\n\t"
    "table and associated callbacks.\n\t"
    "If <on/off> is omitted, prints current setting.\n";

cmd_result_t
if_robo_l2mode(int unit, args_t *a)
{
    soc_control_t    *soc = SOC_CONTROL(unit);
    sal_usecs_t        usec;
    parse_table_t    pt;
    int            r;
    uint32        flags = ARL_ROBO_POLL_INTERVAL;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    usec = soc->arl_interval;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "Interval", PQ_DFL|PQ_INT,
            (void *) 0, &usec, NULL);

    if (!ARG_CNT(a)) {            /* Display settings */
        cli_out("Current settings:\n");
        parse_eq_format(&pt);
        parse_arg_eq_done(&pt);
        return CMD_OK;
    }

    if (parse_arg_eq(a, &pt) < 0) {
        cli_out("%s: Error: Unknown option: %s\n", ARG_CMD(a), ARG_CUR(a));
        parse_arg_eq_done(&pt);
        return CMD_FAIL;
    }
    parse_arg_eq_done(&pt);
    
    if (ARG_CNT(a) > 0){
        if (!sal_strcasecmp(_ARG_CUR(a), "on")){
            flags = ARL_MODE_ROBO_POLL;
        } else if (!sal_strcasecmp(_ARG_CUR(a), "off")){
            flags = ARL_MODE_NONE;
        } else {
            return CMD_USAGE;
        } 
        ARG_NEXT(a);
    }
    
    if (ARG_CNT(a) > 0) {
        return CMD_USAGE;
    }
    
    r = soc_robo_arl_mode_set(unit, flags);

    if (r < 0) {
        cli_out("%s: Error: Could not set L2X mode: %s\n",
                ARG_CMD(a), soc_errmsg(r));
        return CMD_FAIL;
    }
    return CMD_OK;
}




