/*
 * $Id: mcastrep.c,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <appl/diag/diag.h>

#include <bcm/error.h>
#include <bcm/subport.h>
#include <bcm/debug.h>
#include <bcm/multicast.h>
#include <bcm/subport.h>
#ifdef  BCM_TB_SUPPORT
#include <bcm_int/robo/subport.h>
#endif  /* BCM_TB_SUPPORT */
#include <bcm_int/robo/mcast.h>
#include <bcm_int/common/multicast.h>
        
char if_robo_mcastrep_usage[] = 
    "Usages:\n\n"
#ifdef COMPILER_STRING_CONST_LIMIT
    "  MCastREP <subcmd> [args...]\n"
#else
    "  MCastREP group create [<mcrep_id>]\n"
    "       - No <cmrep_id> for group created with Auto selected ID\n"
    "  MCastREP group destroy <mcrep_id>\n"
    "  MCastREP egress add <mcrep_id> Port=<port_id>\n"
    "  MCastREP egress delete <mcrep_id> Port=<port_id>\n"
    "  MCastREP egress clear <mcrep_id>\n"
    "  MCastREP set <mcrep_id> Port=<port_id> vlan=[<vid1>,<vid2>,... | clear]\n\t"
    "  - set replicated vlan id(s) to a particular(index, port) pair.\n"
    "  - \"vlan=clear\" to empty the vlan ids of a particular(index, port) pair.\n"
    "  MCastREP show <mcrep_id> Port=<port_id>\n\t"
    "  - display the replication vlan list of a particular(index, port) pair.\n\n"
    "  MCastREP paramter are:\n\t"
    "  <port_id>    - Physical port id\n\t"
    "  <mcrep_id>   - Multicast group id\n\t"
    "                 (id between 0 to 255 is for Mcast Replication)\n"
#endif
    ;

cmd_result_t
if_robo_mcastrep(int unit, args_t *a)
{
    char            *subcmd, *c;
    int             rv = 0;
    soc_port_t      port;
    int             vp_id, mcrep_id = 0, encap_id = -1;
    uint32          type = 0, flags = 0;
    parse_table_t   pt;
    cmd_result_t    ret_code = CMD_OK;
    bcm_gport_t     gport, gport_subport;
    bcm_multicast_t l2mc_id;
    char *vlan_list = NULL;
    char *ts = NULL;
    int i = 0;
    int vid = 0;
    bcm_vlan_vector_t vlan_vec;

    if (! sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }
    
    if (sal_strcasecmp(subcmd, "group") == 0) {
        
        subcmd = ARG_GET(a);
        if (subcmd == NULL) {
            cli_out("%s: ERROR: Missing group subcommand\n", ARG_CMD(a));
            return CMD_USAGE;
        }

        if (sal_strcasecmp(subcmd, "create") == 0 ) {

            if ((c = ARG_GET(a)) != NULL) {
                flags = BCM_MULTICAST_WITH_ID;
                mcrep_id = parse_integer(c);
            } else {
                mcrep_id = -1;
            }
            
            if (SOC_IS_TBX(unit)){
                if (flags == BCM_MULTICAST_WITH_ID){
#ifdef  BCM_TB_SUPPORT
                    type = (mcrep_id < (_TB_SUBPORT_NUM_GROUP_PER_PORT - 1))?
                            BCM_MULTICAST_TYPE_SUBPORT: BCM_MULTICAST_TYPE_L2;
#else  /* BCM_TB_SUPPORT */
                    type = BCM_MULTICAST_TYPE_L2;
#endif  /* BCM_TB_SUPPORT */
                } else {
                    /* default set to subport type for this CLI is used for 
                     * mcast replication.
                     */
                    type = BCM_MULTICAST_TYPE_SUBPORT;
                }
            } else {
                type = BCM_MULTICAST_TYPE_L2;
            }
            
            /* assign BCM layer type into control flag */ 
            flags |= type;
            
            /* reassign multicast type to ROBO type formate */ 
            type = (type == BCM_MULTICAST_TYPE_SUBPORT) ? 
                    _BCM_MULTICAST_TYPE_SUBPORT : 
                    _BCM_MULTICAST_TYPE_L2;
            
            /* preparing the config data to call API */
            _BCM_MULTICAST_GROUP_SET(l2mc_id, type, mcrep_id);
            rv = bcm_multicast_create(unit, flags, &l2mc_id);
            if (rv){
                goto bcm_err;
            }
            
            /* retrieve the system basis group_id */
            mcrep_id = _BCM_MULTICAST_ID_GET(l2mc_id);
            cli_out(" Mcast Group %d created! %s for %s\n", mcrep_id, 
                    ((flags & BCM_MULTICAST_WITH_ID) ? 
                    "Forced-ID" : "AutoSelected-ID"), 
                    ((type == _BCM_MULTICAST_TYPE_SUBPORT) ? 
                    "Subport Multicast" : "L2 Multicast"));
            return CMD_OK;
        } else if (sal_strcasecmp(subcmd, "destroy") == 0 ) {
            if ((c = ARG_GET(a)) == NULL) {
                cli_out("  ERROR: Missing mcrep_id!\n\n");
                return CMD_USAGE;
            }
            mcrep_id = parse_integer(c);
            
            if (SOC_IS_TBX(unit)){
#ifdef  BCM_TB_SUPPORT
                type = (mcrep_id < (_TB_SUBPORT_NUM_GROUP_PER_PORT - 1)) ?
                        BCM_MULTICAST_TYPE_SUBPORT : BCM_MULTICAST_TYPE_L2;
#else  /* BCM_TB_SUPPORT */
                type = BCM_MULTICAST_TYPE_L2;
#endif  /* BCM_TB_SUPPORT */
            } else {
                type = BCM_MULTICAST_TYPE_L2;
            }

            /* reassign multicast type to ROBO type formate */ 
            type = (type == BCM_MULTICAST_TYPE_SUBPORT) ? 
                    _BCM_MULTICAST_TYPE_SUBPORT : 
                    _BCM_MULTICAST_TYPE_L2;
                    
            /* preparing the config data to call API */
            _BCM_MULTICAST_GROUP_SET(l2mc_id, type, mcrep_id);
            rv = bcm_multicast_destroy(unit, l2mc_id);
            if (rv){
                goto bcm_err;
            }

            cli_out(" McastRep group(0x%x) destroied!\n", mcrep_id);
            return CMD_OK;

        } else {
            return CMD_USAGE;
        }
    } else if (sal_strcasecmp(subcmd, "encapid") == 0) {
        subcmd = ARG_GET(a);
        if (subcmd == NULL) {
            cli_out("%s: ERROR: Missing vport subcommand\n", ARG_CMD(a));
            return CMD_USAGE;
        }

        if (sal_strcasecmp(subcmd, "show") == 0 ) {
            if ((c = ARG_GET(a)) == NULL) {
                cli_out("  ERROR: Missing mcrep_id!\n\n");
                return CMD_USAGE;
            }
            mcrep_id = parse_integer(c);

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "Port",        PQ_DFL|PQ_PORT,
                        0, &port,   NULL);
            parse_table_add(&pt, "VPort",       PQ_DFL|PQ_INT,
    			        0, &vp_id,   NULL);
            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }
    			        
            if ((port == -1) || (vp_id == -1)){
                cli_out("  ERROR: Missing port/vport id!\n\n");
                return CMD_USAGE;
            }
            
            /* parsing to proper valud type to call API */
            _BCM_MULTICAST_GROUP_SET(l2mc_id, 
                    _BCM_MULTICAST_TYPE_SUBPORT, mcrep_id);
            rv = bcm_port_gport_get(unit, port, &gport);
            BCM_GPORT_SUBPORT_PORT_SET(gport_subport, vp_id);
            rv |= bcm_multicast_subport_encap_get(
                    unit, l2mc_id, gport, gport_subport, 
                    (bcm_if_t *)&encap_id);
            if (rv){
                goto bcm_err;
            }
            cli_out(" Mcast group 0x%x,port 0x%x and subport 0x%x "
                    ">> EncapID=0x%x\n",
                    mcrep_id, port, vp_id, encap_id);

            return CMD_OK;
        } else {
            return CMD_USAGE;
        }
    } else if (sal_strcasecmp(subcmd, "egress") == 0) {
        subcmd = ARG_GET(a);
        if (subcmd == NULL) {
            cli_out("%s: ERROR: Missing subcommand. (add|delete)\n", 
                    ARG_CMD(a));
            return CMD_USAGE;
        }
        if (sal_strcasecmp(subcmd, "add") == 0) {
            if ((c = ARG_GET(a)) == NULL) {
                cli_out("  ERROR: Missing mcrep_id!\n\n");
                return CMD_USAGE;
            }
            mcrep_id = parse_integer(c);
            
            /* default set to L2 type. Once encap_id is valid the type will 
             *  be assigned to subport type.
             */
            type = BCM_MULTICAST_TYPE_L2;
            
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "Port",    PQ_DFL|PQ_PORT,
                    0, &port,   NULL);
            parse_table_add(&pt, "EncapID", PQ_DFL|PQ_HEX,
                    0, &encap_id,   NULL);
            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }
    			        
            if (port == -1){
                cli_out("  ERROR: Missing port id!\n\n");
                return CMD_USAGE;
            }
            
            if (encap_id != -1){
                cli_out("  Message: Encap-ID will be ignored!\n\n");
            }

            if (SOC_IS_TBX(unit)){
                type = (encap_id == -1) ?
                        _BCM_MULTICAST_TYPE_L2 : _BCM_MULTICAST_TYPE_SUBPORT;
            } else {
                type = _BCM_MULTICAST_TYPE_L2;
            }

            /* parsing to proper valud type to call API */
            _BCM_MULTICAST_GROUP_SET(l2mc_id, type, mcrep_id);
            rv = bcm_port_gport_get(unit, port, &gport);
            rv |= bcm_multicast_egress_add(unit, l2mc_id, gport, encap_id);
            if (rv){
                goto bcm_err;
            }

            cli_out(" Port=%d has added to Mcast group 0x%x \n", 
                    port, mcrep_id);
            return CMD_OK;
        } else if (sal_strcasecmp(subcmd, "delete") == 0) {
            if ((c = ARG_GET(a)) == NULL) {
                cli_out("  ERROR: Missing mcrep_id!\n\n");
                return CMD_USAGE;
            }
            mcrep_id = parse_integer(c);
            
            /* default set to L2 type. Once encap_id is valid the type will 
             *  be assigned to subport type.
             */
            type = BCM_MULTICAST_TYPE_L2;
            
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "Port",        PQ_DFL|PQ_PORT,
                        0, &port,   NULL);
            parse_table_add(&pt, "EncapID",     PQ_DFL|PQ_HEX,
    			        0, &encap_id,   NULL);
            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }
    			        
            if (port == -1){
                cli_out("  ERROR: Missing port id!\n\n");
                return CMD_USAGE;
            }

            if (encap_id != -1){
                cli_out("  Message: Encap-ID will be ignored!\n\n");
            }

            if (SOC_IS_TBX(unit)){
                type = (encap_id == -1) ?
                        _BCM_MULTICAST_TYPE_L2 : _BCM_MULTICAST_TYPE_SUBPORT;
            } else {
                type = _BCM_MULTICAST_TYPE_L2;
            }

            /* parsing to proper valud type to call API */
            _BCM_MULTICAST_GROUP_SET(l2mc_id, type, mcrep_id);
            rv = bcm_port_gport_get(unit, port, &gport);
            rv |= bcm_multicast_egress_delete(
                        unit, l2mc_id, gport, encap_id);
            if (rv){
                goto bcm_err;
            }

            cli_out(" Port=%d has deleted from Mcast group 0x%x \n", 
                    port, mcrep_id);
            return CMD_OK;
        } else if (sal_strcasecmp(subcmd, "clear") == 0) {
            if ((c = ARG_GET(a)) == NULL) {
                cli_out("  ERROR: Missing mcrep_id!\n\n");
                return CMD_USAGE;
            }
            mcrep_id = parse_integer(c);

            /* default set to L2 type to clear all replication behavior on 
             *  user assigned group.
             */
            _BCM_MULTICAST_GROUP_SET(
                    l2mc_id, _BCM_MULTICAST_TYPE_L2, mcrep_id);
            rv = bcm_multicast_egress_delete_all(unit, l2mc_id);
            if (rv){
                goto bcm_err;
            }

            cli_out(" McastRep group(0x%x) was cleared.\n", mcrep_id);

            return CMD_OK;
        } else {
            return CMD_USAGE;
        }
    } else if (sal_strcasecmp(subcmd, "set") == 0) {
        if ((c = ARG_GET(a)) == NULL) {
            cli_out("  ERROR: Missing mcrep_id!\n\n");
            return CMD_USAGE;
        }
        mcrep_id = parse_integer(c);
        
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Port",    PQ_DFL|PQ_PORT,
                0, &port,   NULL);
        parse_table_add(&pt, "vlan", PQ_DFL | PQ_STRING,0,
                &vlan_list, NULL);
        if (parse_arg_eq(a, &pt) < 0) {
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
        }

        if (port == -1){
            cli_out("  ERROR: Missing port id!\n\n");
            return CMD_USAGE;
        }

        /* create vlan vector to be set */
        i = 0;
        BCM_VLAN_VEC_ZERO(vlan_vec);

        if (sal_strcasecmp(vlan_list, "clear")) {
            if (strchr(vlan_list, ',')) {
                do {
                    if (i >= BCM_VLAN_INVALID) {
                        /* Set a check point to avoid infinite loop */
                        break;
                    }
                    vid = sal_ctoi(vlan_list, &ts);
                    i++;
        
                    if (vid < 0) {
                        return(-1);
                    } 
    
                    BCM_VLAN_VEC_SET(vlan_vec, vid);
        
                    if (*ts != ',') {   /* End of string */
                        break;
                    }
                    vlan_list = ts + 1;
                } while(1);
            } else  {
                /* only one vid is set */
                vid = _shr_ctoi(vlan_list);
                if (vid < 0) {
                    return(-1);
                } 
                BCM_VLAN_VEC_SET(vlan_vec, vid);
            }
        }

        rv = bcm_multicast_repl_set(unit, mcrep_id, port, vlan_vec);
        if (rv < 0) {
            goto bcm_err;
        }

        return CMD_OK;
    } else if (sal_strcasecmp(subcmd, "show") == 0) {
        if ((c = ARG_GET(a)) == NULL) {
            cli_out("  ERROR: Missing mcrep_id!\n\n");
            return CMD_USAGE;
        }
        mcrep_id = parse_integer(c);
        
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Port",    PQ_DFL|PQ_PORT,
                0, &port,   NULL);
        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }
			        
        if (port == -1){
            cli_out("  ERROR: Missing port id!\n\n");
            return CMD_USAGE;
        }

        BCM_VLAN_VEC_ZERO(vlan_vec);
        rv = bcm_multicast_repl_get(unit, mcrep_id, port, vlan_vec);
        if (rv < 0) {
            goto bcm_err;
        }
        cli_out("Replicated vlan(s): ");
        for (vid = BCM_VLAN_MIN; vid <= BCM_VLAN_MAX; vid++) {
        	
            if (BCM_VLAN_VEC_GET(vlan_vec, vid)) {
                if (vid == BCM_VLAN_UNTAG) {
                    cli_out("untagged ");
                } else {
                    cli_out("%d ",vid);
                }
            }
        }
        cli_out("\n");
        return CMD_OK;

    } else {
        return CMD_USAGE;
    }
    
 bcm_err:
    cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
    return CMD_FAIL;
}

