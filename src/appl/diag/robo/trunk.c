/*
 * $Id: trunk.c,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#include <bcm/error.h>
#include <bcm/trunk.h>
#include <bcm/port.h>
#include <bcm/stack.h>
#include <bcm/debug.h>
#include <bcm/switch.h>

#include <bcm_int/robo/trunk.h>

/* NOTE:
 * Using PBMP in the add/remove makes the commands useless in stacking
 * where the stack port has to be added multiple times
 */

static char *pscnames[] = BCM_TRUNK_PSC_NAMES_INITIALIZER;

cmd_result_t
if_robo_trunk(int unit, args_t *a)
{
    char  *subcmd, *c;
    bcm_trunk_t  tid = -1;
    pbmp_t  pbmp;
    int  r;
    int  psc;
    static pbmp_t  arg_pbmp;
    static uint32  arg_rtag = 0;
    static uint32  arg_vlan = VLAN_ID_DEFAULT, arg_tid = 0;
    parse_table_t  pt;
    cmd_result_t  ret_code;
    bcm_trunk_info_t  trunk_info;
    bcm_trunk_member_t  *member_array = NULL;
    int  member_count = 0;
    sal_mac_addr_t  mac;
    vlan_id_t  vid;
    char  *s;
    int  i, j;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    sal_memset(&trunk_info, 0, sizeof (trunk_info));

    if (sal_strcasecmp(subcmd, "init") == 0) {
        r = bcm_trunk_init(unit);
        if (r < 0) {
            cli_out("%s: trunk init failed: %s\n", ARG_CMD(a), bcm_errmsg(r));
            return CMD_FAIL;
        }
	    return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "deinit") == 0) {
        r = bcm_trunk_detach(unit);
        if (r < 0) {
            cli_out("%s: trunk deinit failed: %s\n", ARG_CMD(a), bcm_errmsg(r));
            return CMD_FAIL;
        }
        return CMD_OK;
    }

    /* trunk show command */
    if (sal_strcasecmp(subcmd, "show") == 0) {
        bcm_trunk_chip_info_t  ti;
        int  tid_min, tid_max, found, isGport = 0;
    
        if ((r = bcm_trunk_chip_info_get(unit, &ti)) < 0) {
            cli_out("%s: %s\n", ARG_CMD(a), bcm_errmsg(r));
            return CMD_FAIL;
        }
    
        tid_min = ti.trunk_id_min;
        tid_max = ti.trunk_id_max;
    
        if ((c = ARG_CUR(a)) != NULL) {
            parse_table_init(unit, &pt);
                parse_table_add(&pt, "Id", PQ_DFL|PQ_INT, 0, &arg_tid, NULL);
            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }
            tid_min = tid_max = arg_tid;
        }

        r = bcm_switch_control_get(unit, bcmSwitchUseGport, &isGport);
        if (r < 0) {
            cli_out("%s: bcm_switch_control_get (bcmSwitchUseGport) failed: %s\n",
                    ARG_CMD(a), bcm_errmsg(r));
            return CMD_FAIL;
        }
    
        found = 0;
        for (tid = tid_min; tid <= tid_max; tid++) {
            member_array = sal_alloc(sizeof(bcm_trunk_member_t) *
                BCM_TRUNK_MAX_PORTCNT, "member array");
            if (member_array == NULL) {
                cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(BCM_E_MEMORY));
                return CMD_FAIL;
            }

            r = bcm_trunk_get(unit, tid, &trunk_info, 
                    BCM_TRUNK_MAX_PORTCNT, member_array, &member_count);
    
            if (r == BCM_E_NOT_FOUND) {
                /* Only show existing trunk groups */
                sal_free(member_array);
                continue;
            }
    
            if (r < 0) {
                cli_out("%s: trunk %d get failed: %s\n",
                        ARG_CMD(a), tid, bcm_errmsg(r));
                sal_free(member_array);
                return CMD_FAIL;
            }

            if (isGport) {
                r = _bcm_robo_trunk_gport_resolve
                        (unit, member_count, member_array);
                if (r < 0) {
                    cli_out("%s: _bcm_robo_trunk_gport_resolve failed: %s\n",
                            ARG_CMD(a), bcm_errmsg(r));
                    sal_free(member_array);
                    return CMD_FAIL;
                }
            }

            found += 1;
            cli_out("trunk %d: %d ports=", tid, member_count);
    
            for (i = 0; i < member_count; i++) 
            {
                if (!SOC_PORT_VALID(unit, member_array[i].gport)) {
                    cli_out("invalid port!\n");
                    sal_free(member_array);
                    return CMD_FAIL;
                }
                cli_out("%s%s",
                        i == 0 ? "" : ",",
                        SOC_PORT_NAME(unit, member_array[i].gport));
            }
            if ((trunk_info.psc & _BCM_TRUNK_PSC_VALID_VAL) <= 0 || 
                (trunk_info.psc & _BCM_TRUNK_PSC_VALID_VAL) >= 7) 
            {
                s= "unknown";
            } else 
            {
                s = pscnames[(trunk_info.psc & _BCM_TRUNK_PSC_VALID_VAL)];
            }
            cli_out(" psc/rtag=%s (%d or'ed 0x%x)\n", 
                    s, 
                    (trunk_info.psc & _BCM_TRUNK_PSC_VALID_VAL), 
                    (trunk_info.psc & ~_BCM_TRUNK_PSC_VALID_VAL));
    
            if (member_array != NULL) {
                sal_free(member_array);
            }
        }
        if (found == 0) {
            cli_out("[no matching trunks defined]\n");
        }
    
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "add") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Id",   PQ_DFL|PQ_INT,  0, &arg_tid,  NULL);
        parse_table_add(&pt, "Rtag", PQ_DFL|PQ_INT,  0, &arg_rtag, NULL);
        parse_table_add(&pt, "Pbmp", PQ_DFL|PQ_PBMP, 0, &arg_pbmp, NULL);
        if (!parseEndOk(a, &pt, &ret_code)) {
           return ret_code;
        }

        tid = arg_tid;
        psc = arg_rtag;
        pbmp = arg_pbmp;

        member_array = sal_alloc(sizeof(bcm_trunk_member_t) *
            BCM_TRUNK_MAX_PORTCNT, "member array");
        if (member_array == NULL) {
            cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(BCM_E_MEMORY));
            return CMD_FAIL;
        }
	
        r = bcm_trunk_get(unit, tid, &trunk_info, 
                BCM_TRUNK_MAX_PORTCNT, member_array, &member_count);
        if(r >= 0) {  /* not first create */
            SOC_PBMP_ITER(pbmp, i) {
                for (j = 0; j < member_count; j++) {
                    if (i == member_array[j].gport) {
                        break;
                    }
                }

                /* add new ports into trunk member list */
                if (j == member_count) {
                    if (member_count >= BCM_TRUNK_MAX_PORTCNT) {
                        sal_free(member_array);
                        cli_out("member_count >= BCM_TRUNK_MAX_PORTCNT!\n");
                        return CMD_FAIL;
                    }
                    member_array[j].gport = i;
                    member_count++;
                }
            };

            r = bcm_trunk_set(unit, tid, &trunk_info, 
                    member_count, member_array);
        } else {  /* first create */
            j = 0;
            SOC_PBMP_ITER(pbmp, i) { 
                if (j >= BCM_TRUNK_MAX_PORTCNT) {
                    sal_free(member_array);
                    cli_out("member_count >= BCM_TRUNK_MAX_PORTCNT!\n");
                    return CMD_FAIL;
                }
                bcm_trunk_member_t_init(&member_array[j]);
                member_array[j++].gport = i;
            };
    
            member_count = j;
            trunk_info.psc = psc;
            bcm_trunk_create(unit, BCM_TRUNK_FLAG_WITH_ID, &tid);

            r = bcm_trunk_set(unit, tid, &trunk_info, 
                    member_count, member_array);
            if (r < 0) {
                bcm_trunk_destroy(unit, tid);
            }
        }
                
        if (r < 0) {
            sal_free(member_array);
            cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(r));
            return CMD_FAIL;
        }

        if (member_array != NULL) {
            sal_free(member_array);
        }
    } else if (sal_strcasecmp(subcmd, "remove") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Id", PQ_DFL|PQ_INT, 0, &arg_tid, NULL);
        parse_table_add(&pt, "Pbmp", PQ_DFL|PQ_PBMP, \
                       (void *)(0), &arg_pbmp, NULL);
        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }

        member_array = sal_alloc(sizeof(bcm_trunk_member_t) *
            BCM_TRUNK_MAX_PORTCNT, "member array");
        if (member_array == NULL) {
            cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(BCM_E_MEMORY));
            return CMD_FAIL;
        }

        tid = arg_tid;
        r = bcm_trunk_get(unit, tid, &trunk_info, 
                BCM_TRUNK_MAX_PORTCNT, member_array, &member_count);

        if (r >= 0) {
            SOC_PBMP_CLEAR(pbmp);
            for (i = 0; i < member_count; i++) { 
                SOC_PBMP_PORT_ADD(pbmp, member_array[i].gport);
            }
       
            SOC_PBMP_REMOVE(pbmp, arg_pbmp);

            j = 0;
            SOC_PBMP_ITER(pbmp, i) {
            	/*
                 * Check Me : The module id should be get from API.
                 *            But bcm_stk_my_modid_get() is stack API and not 
                 *            designed yet!
                 *            Modify the following line if API ready.
            	 */
                member_array[j++].gport = i;
            }
            member_count = j;
     
            if (j) {
                r = bcm_trunk_set(unit, tid, &trunk_info, 
                        member_count, member_array);
            } else {
                r = bcm_trunk_destroy(unit, tid);
            }

            if (r < 0) {
                sal_free(member_array);
                cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(r));
                return CMD_FAIL;
            }
        }

        if (member_array != NULL) {
            sal_free(member_array);
        }
    } else if (sal_strcasecmp(subcmd, "mcast") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Id", PQ_DFL|PQ_INT, 0, &arg_tid, NULL);
        parse_table_add(&pt, "Mac", PQ_DFL|PQ_MAC, 0, (void *)mac, NULL);
        parse_table_add(&pt, "Vlanid", 	PQ_DFL|PQ_HEX, 0, &arg_vlan, NULL);
        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }

        tid = arg_tid;
        vid = arg_vlan;
        r = bcm_trunk_mcast_join(unit, tid, vid, mac);
    } else if (sal_strcasecmp(subcmd, "psc") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Id",   PQ_DFL|PQ_INT,  0, &arg_tid,  NULL);
        parse_table_add(&pt, "Rtag", PQ_DFL|PQ_INT,  0, &arg_rtag, NULL);
        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }

        tid = arg_tid;
        psc = arg_rtag;

        r = bcm_trunk_psc_set(unit, tid, psc);
    } else {
        return CMD_USAGE;
    }

    if (r < 0) {
        cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(r));
        return CMD_FAIL;
    }

    return CMD_OK;
}

