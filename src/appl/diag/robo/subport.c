/*
 * $Id: subport.c,v 1.13 Broadcom SDK $ 
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
#ifdef  BCM_TB_SUPPORT
#include <bcm_int/robo/subport.h>
#endif  /* BCM_TB_SUPPORT */

char if_robo_subport_usage[] = 
    "Usages:\n\n"
    "  subport show <port_id>\n"
    "  subport group create <mcrep_id> <port_id> <svid> <stag_type>\n"
    "  subport group destroy <mcrep_id> <port_id>\n"
    "  subport vport add <mcrep_id> <port_id> <cvid> <pri> <ctag_type> <spcp_remark>\n"
    "  subport vport remove <mcrep_id> <port_id> <vp_id>\n\n"
#ifndef COMPILER_STRING_CONST_LIMIT
    "  Supported paramter are:\n\t"
    "  <port_id>    - Physical port id.\n\t"
    "  <vp_id>      - Vertial port id.\n\t"
    "  <mcrep_id>   - Multicast Replication group id.\n\t"
    "  <svid>       - Service provider VLAN id(valid value between 1-4095).\n\t"
    "  <cvid>       - Customer VLAN id and the valid value between 0-4095.\n\t" 
    "                 (0 for pri-tag and untag vp).\n\t"
    "  <pri>        - user assigning priorty. There are two priority foramts:\n\t"
    "                 (1) 1p priority (valid value from 0 to 7)\n\t"
    "                 (2) Internal priority: bit5-bit4(DP), bit3-bit0(TC)\n\t"
    "  <stag_type>  - Tag status on NNI.\n\t"
    "                 (0 for Single S-Tagged and 1 for Double Tagged)\n\t"
    "  <ctag_type>  - Tag status on UNI.\n\t"
    "                 (0 for 1Q-Tagged, 1 for Pri-Tagged and 2 for Untagged)\n\t"
    "  <spcp_remark> - Options for the egress SPCP remarking. \n\t"
    "                 (1) value at 0 for COPY incomming CPCP\n\t"
    "                 (2) value at 1 for user assigned 1p priority\n\t"
    "                 (3) value at 2 for user assigned DP+TC mapping priority\n\t"
    "\n"
#endif
    ;

cmd_result_t
if_robo_subport(int unit, args_t *a)
{
    char            *subcmd, *c;
    int             rv = 0;
#ifdef	BCM_TB_SUPPORT
    int             p = 0, i, j, tag_type = 0, pcp_mark = 0;
#endif /* BCM_TB_SUPPORT */
    vlan_id_t       svid;
    soc_port_t      port;
    int             vp_id, mcrep_id, cvid;
    int             pri = 0;
    uint32          prop_flags = 0;
    pbmp_t          pbmp;
    bcm_gport_t     gport;
    bcm_subport_group_config_t  vpg_cfg;
    bcm_subport_config_t        vp_cfg;

    if (!soc_feature(unit, soc_feature_subport)) {
        cli_out("%s: MSG: Feature not supported!\n", ARG_CMD(a));
        return CMD_FAIL;
    }
    
    BCM_PBMP_CLEAR(pbmp);
    sal_memset(&vpg_cfg, 0, sizeof(bcm_subport_group_config_t));
    sal_memset(&vp_cfg, 0, sizeof(bcm_subport_config_t));
    
    
    if (! sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }
    
    if (sal_strcasecmp(subcmd, "show") == 0) {
        if ((c = ARG_GET(a)) == NULL) {
            pbmp = PBMP_ALL(unit);
        } else {
            port = parse_integer(c);
            if (IS_E_PORT(unit, port)){
                BCM_PBMP_PORT_ADD(pbmp, port);
            }
        }
    
#ifdef  BCM_TB_SUPPORT
        if (!SOC_IS_TBX(unit)){
            cli_out(" 'Subport' on this chip is not designed yet!\n");
            return CMD_OK;
        }
        
        PBMP_ITER(pbmp, p){
            for (i = 0; i < _TB_SUBPORT_NUM_GROUP_PER_PORT; i++){
                mcrep_id = ((p & _TB_SUPORT_GROUP_ID_MASK_PORT) << 
                        _TB_SUPORT_GROUP_ID_SHIFT_PORT) | 
                        (i & _TB_SUPORT_GROUP_ID_MASK_GROUP );
                BCM_GPORT_SUBPORT_GROUP_SET(gport, mcrep_id);
                rv = bcm_subport_group_get(unit, gport, &vpg_cfg);
                
                if (rv != BCM_E_NONE){
                    if (rv == BCM_E_NOT_FOUND){
                        continue;
                    } else {
                        cli_out(" %s,ERROR:port=0x%x,mc_group=0x%x, %s\n", 
                                ARG_CMD(a), p, i, bcm_errmsg(rv));
                        goto bcm_err;
                    }
                }
                cli_out("   SubportGroup ID=0x%04x(GPORT=0x%08x) : \n",
                        mcrep_id, gport);
                cli_out("\t Port=0x%x,VLAN(SP)=0x%x,prop_flag=0x%08x\n", 
                        p, vpg_cfg.vlan, vpg_cfg.prop_flags);
                
                for (j = 0; j < _TB_SUBPORT_NUM_VPORT_PER_GROUP; j++){
                    vp_id = _TB_SUBPORT_SYSTEM_ID_GET(mcrep_id, j);
                    BCM_GPORT_SUBPORT_PORT_SET(gport, vp_id);
                    rv = bcm_subport_port_get(unit, gport, &vp_cfg);
                    if (rv != BCM_E_NONE){
                        if (rv == BCM_E_NOT_FOUND){
                            continue;
                        } else {
                            cli_out(" %s,ERROR:port=0x%x,vp_id=0x%x, %s\n", 
                                    ARG_CMD(a), p, j, bcm_errmsg(rv));
                            goto bcm_err;
                        }
                    }
                    prop_flags = vp_cfg.prop_flags;
                    cli_out("    >> VPort%d(GPORT=0x%08x),prop_flags=0x%08x\n",
                            j, gport, prop_flags);
                    cli_out("\t  - priority=0x%x, ", vp_cfg.int_pri);
                    if (vp_cfg.pkt_vlan == 0) {
                        if (prop_flags & _BCM_SUBPORT_FLAG_UP_ING_UNTAG){
                            cli_out("Untag VLAN\n");
                        } else if (prop_flags & 
                                _BCM_SUBPORT_FLAG_UP_ING_PRITAG){
                            cli_out("Pri-tag VLAN(vid=0)\n");
                        }
                    } else {
                        cli_out("VLAN(C)%d\n", vp_cfg.pkt_vlan);
                    }
                }
            }
        }
#else   /* BCM_TB_SUPPORT */
        cli_out(" 'Subport' on this chip is not designed yet!\n");
        return CMD_OK;
#endif  /* BCM_TB_SUPPORT */
            
        return CMD_OK;
    } else if (sal_strcasecmp(subcmd, "group") == 0) {
        subcmd = ARG_GET(a);
        if (subcmd == NULL) {
            cli_out("%s: ERROR: Missing group subcommand\n", ARG_CMD(a));
            return CMD_USAGE;
        }

        if (sal_strcasecmp(subcmd, "create") == 0 ) {

            if ((c = ARG_GET(a)) == NULL) {
                cli_out("  ERROR: Missing mcrep_id!\n\n");
                return CMD_USAGE;
            }
            mcrep_id = parse_integer(c);
            
            if ((c = ARG_GET(a)) == NULL) {
                cli_out("  ERROR: Missing port_id!\n\n");
                return CMD_USAGE;
            }
            port = parse_integer(c);
                
            if ((c = ARG_GET(a)) == NULL) {
                cli_out("  ERROR: Missing svid!\n\n");
                return CMD_USAGE;
            }
            svid = parse_integer(c);

#ifdef BCM_TB_SUPPORT
            if ((c = ARG_GET(a)) == NULL) {
                cli_out("  Default stag_type set to 0!\n\n");
                tag_type = 0;
            } else {
                tag_type = parse_integer(c);
            }
            
            /* parsing flags for group creation */
            if (tag_type == 0){
                prop_flags |= _BCM_SUBPORT_FLAG_DOWN_ING_SOT;
            } else if (tag_type == 1){
                prop_flags |= _BCM_SUBPORT_FLAG_DOWN_ING_DT;
            } else {
                cli_out("  Incorrect tag_type(%d) assignment!\n\n",tag_type);
                return CMD_USAGE;
            }
#endif  /* BCM_TB_SUPPORT */
            
            /* preparing the config data to call API */
            vpg_cfg.flags = BCM_SUBPORT_GROUP_WITH_ID;
            BCM_GPORT_LOCAL_SET(vpg_cfg.port, port); 
            vpg_cfg.vlan = svid;
            vpg_cfg.prop_flags = prop_flags;
            BCM_GPORT_SUBPORT_GROUP_SET(gport, mcrep_id);
            rv = bcm_subport_group_create(unit, &vpg_cfg, &gport);
            if (rv){
                goto bcm_err;
            }
            
            /* retrieve the system basis group_id */
            mcrep_id = BCM_GPORT_SUBPORT_GROUP_GET(gport);
            cli_out("  Subport Group created!\n\t"
                    "System basis group id is 0x%x(prop_flags=0x%08x)\n", 
                    mcrep_id, prop_flags);
            return CMD_OK;
        } else if (sal_strcasecmp(subcmd, "destroy") == 0 ) {
            if ((c = ARG_GET(a)) == NULL) {
                cli_out("  ERROR: Missing mcrep_id!\n\n");
                return CMD_USAGE;
            }
            mcrep_id = parse_integer(c);
            
            if ((c = ARG_GET(a)) == NULL) {
                cli_out("  ERROR: Missing port_id!\n\n");
                return CMD_USAGE;
            }
            port = parse_integer(c);
                
#ifdef  BCM_TB_SUPPORT
            mcrep_id = ((port & _TB_SUPORT_GROUP_ID_MASK_PORT) << 
                    _TB_SUPORT_GROUP_ID_SHIFT_PORT) | 
                    (mcrep_id & _TB_SUPORT_GROUP_ID_MASK_GROUP );
#else  /* BCM_TB_SUPPORT */
            cli_out("  ERROR: Internal problem on parsing !\n\n");
#endif  /* BCM_TB_SUPPORT */

            BCM_GPORT_SUBPORT_GROUP_SET(gport, mcrep_id);
            rv = bcm_subport_group_destroy(unit, gport);
            if (rv){
                goto bcm_err;
            }
            cli_out("  System basis subport group id 0x%x destroied\n",
                    mcrep_id);
            return CMD_OK;

        } else {
            return CMD_USAGE;
        }
    } else if (sal_strcasecmp(subcmd, "vport") == 0) {
        subcmd = ARG_GET(a);
        if (subcmd == NULL) {
            cli_out("%s: ERROR: Missing vport subcommand\n", ARG_CMD(a));
            return CMD_USAGE;
        }

        if (sal_strcasecmp(subcmd, "add") == 0 ) {
            if ((c = ARG_GET(a)) == NULL) {
                cli_out("  ERROR: Missing mcrep_id!\n\n");
                return CMD_USAGE;
            }
            mcrep_id = parse_integer(c);
            
            if ((c = ARG_GET(a)) == NULL) {
                cli_out("  ERROR: Missing port_id!\n\n");
                return CMD_USAGE;
            }
            port = parse_integer(c);

            
            if ((c = ARG_GET(a)) == NULL) {
                cli_out("  ERROR: Missing CVID(0-4095)!\n\n");
                return CMD_USAGE;
            }
            cvid = parse_integer(c);

            if ((c = ARG_GET(a)) == NULL) {
                cli_out("  ERROR: Missing pri(priority)!\n\n");
                return CMD_USAGE;
            }
            pri = parse_integer(c);

#ifdef BCM_TB_SUPPORT
            if ((c = ARG_GET(a)) == NULL) {
                cli_out("  Default ctag_type and pcp_remark set to 0!\n\n");
                tag_type = 0;
                pcp_mark = 0;
            } else {
                tag_type = parse_integer(c);
                
                if ((c = ARG_GET(a)) == NULL) {
                    cli_out("  Default pcp_remark set to 0!\n\n");
                    pcp_mark = 0;
                } else {
                    pcp_mark = parse_integer(c);
                }
                
            }
            
            /* parsing flags for group creation */
            if (tag_type == 0){
                prop_flags |= _BCM_SUBPORT_FLAG_UP_ING_SIT;
            } else if (tag_type == 1){
                prop_flags |= _BCM_SUBPORT_FLAG_UP_ING_PRITAG;
            } else if (tag_type == 2){
                prop_flags |= _BCM_SUBPORT_FLAG_UP_ING_UNTAG;
            } else {
                cli_out("  Incorrect tag_type(%d) assignment!\n\n",tag_type);
                return CMD_USAGE;
            }
            
            if (pcp_mark == 0){
                prop_flags |= _BCM_SUBPORT_FLAG_EGR_SPCP_ING_CPCP;
            } else if (pcp_mark == 1){
                prop_flags |= _BCM_SUBPORT_FLAG_EGR_SPCP_VP_SPCP;
            } else if (pcp_mark == 2){
                prop_flags |= _BCM_SUBPORT_FLAG_EGR_SPCP_MAPPED;
            } else {
                cli_out("  Incorrect pcp_remark(%d) assignment!\n\n",
                        pcp_mark);
                return CMD_USAGE;
            }
#endif  /* BCM_TB_SUPPORT */

            /* preparing the configuration data */
#ifdef  BCM_TB_SUPPORT
            if (SOC_IS_TBX(unit)){
                mcrep_id = ((port & _TB_SUPORT_GROUP_ID_MASK_PORT) << 
                        _TB_SUPORT_GROUP_ID_SHIFT_PORT) | 
                        (mcrep_id & _TB_SUPORT_GROUP_ID_MASK_GROUP );
                BCM_GPORT_SUBPORT_GROUP_SET(gport, mcrep_id);
                vp_cfg.group = gport;
            } else {
                vp_cfg.group = BCM_GPORT_INVALID;
            }
#else  /* BCM_TB_SUPPORT */
            vp_cfg.group = BCM_GPORT_INVALID;
#endif  /* BCM_TB_SUPPORT */
            vp_cfg.pkt_vlan = cvid;
            vp_cfg.int_pri = pri;     /* ignored for ROBO */
            vp_cfg.prop_flags = prop_flags;
            
            rv = bcm_subport_port_add(unit, &vp_cfg, &gport);

            vp_id = BCM_GPORT_SUBPORT_PORT_GET(gport);
            if (rv){
                goto bcm_err;
            }
#ifdef  BCM_TB_SUPPORT
            cli_out("  vport add into subport group 0x%x with ID=%d\n\t"
                    "(system basis vp_id is 0x%x, prop_flags is 0x%08x)\n\n",
                    mcrep_id, _TB_SUBPORT_SYSTEM_ID_2VPORT(vp_id), 
                    vp_id, prop_flags);
#endif  /* BCM_TB_SUPPORT */
            return CMD_OK;
                            
        } else if (sal_strcasecmp(subcmd, "remove") == 0 ) {
            if ((c = ARG_GET(a)) == NULL) {
                cli_out("  ERROR: Missing mcrep_id!\n\n");
                return CMD_USAGE;
            }
            mcrep_id = parse_integer(c);
            
            if ((c = ARG_GET(a)) == NULL) {
                cli_out("  ERROR: Missing port_id!\n\n");
                return CMD_USAGE;
            }
            port = parse_integer(c);

            /* set the system basis group id */
#ifdef  BCM_TB_SUPPORT
            mcrep_id = ((port & _TB_SUPORT_GROUP_ID_MASK_PORT) << 
                    _TB_SUPORT_GROUP_ID_SHIFT_PORT) | 
                    (mcrep_id & _TB_SUPORT_GROUP_ID_MASK_GROUP );
#else  /* BCM_TB_SUPPORT */
            cli_out("  ERROR: Internal problem on parsing !\n\n");
#endif  /* BCM_TB_SUPPORT */
                    
            if ((c = ARG_GET(a)) == NULL) {
                cli_out("  ERROR: Missing vport_id(0-15)!\n\n");
                return CMD_USAGE;
            }
            vp_id = parse_integer(c);
            
#ifdef  BCM_TB_SUPPORT
            /* retrieve the system based VPort_ID :
             *  - The id binding the subport group id and vp_id(0-15)
             */
            vp_id = _TB_SUBPORT_SYSTEM_ID_GET(mcrep_id, vp_id);
            BCM_GPORT_SUBPORT_PORT_SET(gport, vp_id);
#else  /* BCM_TB_SUPPORT */
            gport = BCM_GPORT_INVALID;
#endif  /* BCM_TB_SUPPORT */

            rv = bcm_subport_port_delete(unit, gport);
            if (rv){
                goto bcm_err;
            }
            cli_out("  System basis vport id 0x%x removed!!\n", vp_id);
            return CMD_OK;
        } else {
            return CMD_USAGE;
        }
    } else {
        return CMD_USAGE;
    }
    
 bcm_err:
    cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
    return CMD_FAIL;
}

