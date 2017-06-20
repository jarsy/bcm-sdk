/*
 * $Id: l2.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <appl/diag/parse.h>
#include <appl/diag/system.h>
#include <appl/diag/ea/ea.h>


#include <soc/debug.h>
#include <soc/hash.h>
#include <soc/l2u.h>
#include <soc/l2x.h>

#include <bcm/debug.h>
#include <bcm/error.h>
#include <bcm/l2.h>
#include <bcm/mcast.h>
#include <bcm/stack.h>

static int
_l2_addr_tranverse_callback(
    int unit, 
    bcm_l2_addr_t *entry, 
    void *user_data)
{
    int port = * (int *)user_data;
    if (port != -1){
        if (port != entry->port) {
            return 0;
        }
    }
    cli_out("%02x:%02x:%02x:%02x:%02x:%02x",
            entry->mac[0],entry->mac[1],entry->mac[2],
            entry->mac[3],entry->mac[4],entry->mac[5]);    
    cli_out("  port_idx:%2d ", entry->port);
    if ((entry->flags & BCM_L2_STATIC) == BCM_L2_STATIC) {
        cli_out("static\n");
    } else {
        cli_out("dyn\n"); 
    }
    return 0;
}



cmd_result_t
if_ea_l2(int unit, args_t *a)
{
	char *subcmd = NULL;
    parse_table_t pt;
    bcm_l2_addr_t l2addr;
    bcm_mac_t arg_macaddr = {0,0,0,0,0,0};
    bcm_mac_t def_macaddr = {0,0,0,0,0,0};
    int arg_port = -1;
    int arg_newport = -1;
    cmd_result_t ret_code = 0;
    int rv = 0;
    
    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }
    /* Check valid device to operation on ...*/
    if (! sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }
        
    if (!sal_strcasecmp(subcmd, "replace")) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Port",            PQ_DFL|PQ_PORT,
                0, &arg_port,   NULL);
        parse_table_add(&pt, "MACaddress", 	    PQ_DFL|PQ_MAC,
                0, &arg_macaddr, NULL);
        parse_table_add(&pt, "NewPort",         PQ_DFL|PQ_PORT,
                0, &arg_newport,   NULL);
                
        if (ARG_CNT(a) < 3) {
            return CMD_USAGE;
        }
        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }
        if ((arg_port == -1) || (arg_newport == -1) || 
            (sal_memcmp(arg_macaddr, def_macaddr, sizeof(bcm_mac_t)) == 0)) {
            return CMD_USAGE;
        }
        rv = bcm_l2_addr_delete_by_mac_port(unit, arg_macaddr, 0, arg_port,
                    BCM_L2_DELETE_STATIC);
	    if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        bcm_l2_addr_t_init(&l2addr, arg_macaddr, 0);
        l2addr.port = arg_newport;
        rv = bcm_l2_addr_add(unit, &l2addr);
        if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;        
    } else if (! sal_strcasecmp(subcmd, "add") || 
            ! sal_strcasecmp(subcmd, "+")) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Port",            PQ_DFL|PQ_PORT,
                0, &arg_port,   NULL);
        parse_table_add(&pt, "MACaddress", 	    PQ_DFL|PQ_MAC,
                0, &arg_macaddr, NULL);       
        if (ARG_CNT(a) < 2) {
            return CMD_USAGE;
        }
        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }
        if ((arg_port == -1) || 
            (sal_memcmp(arg_macaddr, def_macaddr, sizeof(bcm_mac_t)) == 0)) {
            return CMD_USAGE;
        }
        bcm_l2_addr_t_init(&l2addr, arg_macaddr, 0);
        l2addr.port = arg_port;
        rv = bcm_l2_addr_add(unit, &l2addr);
        if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;       
    } else if (! sal_strcasecmp(subcmd, "del") 
            || ! sal_strcasecmp(subcmd, "-")) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "MACaddress", PQ_DFL|PQ_MAC,
                0, &arg_macaddr, NULL);
        parse_table_add(&pt, "Port", PQ_DFL|PQ_PORT,
                0, &arg_port, NULL);
                
        if (ARG_CNT(a) < 1) {
            return CMD_USAGE;
        }
        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }
        if (sal_memcmp(arg_macaddr, def_macaddr, sizeof(bcm_mac_t)) == 0) {
            return CMD_USAGE;
        }
        if (arg_port >= 0) {
            rv = bcm_l2_addr_delete_by_mac_port(unit, arg_macaddr,
                     0, arg_port, BCM_L2_DELETE_STATIC);
            if (BCM_FAILURE(rv)) {
                cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }
            
        } else {        
            rv = bcm_l2_addr_delete(unit, arg_macaddr, 0);
            if (BCM_FAILURE(rv)) {
                cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }
        }
        return CMD_OK;       
    } else if (! sal_strcasecmp(subcmd, "learn")){  
        
        char    *learncmd = NULL;
        
        if ((learncmd = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }
        
        if (! sal_strcasecmp(learncmd, "control")) {
            bcm_l2_learn_limit_t limit;       
            int arg_limit = -1;
            int arg_age = -1;
            parse_table_init(unit, &pt); 
            parse_table_add(&pt, "Port", PQ_DFL|PQ_PORT,
                0, &arg_port, NULL);
            parse_table_add(&pt, "Limit", PQ_DFL|PQ_INT,
                0, &arg_limit, NULL);
            parse_table_add(&pt, "Age", PQ_DFL|PQ_INT,
                0, &arg_age, NULL);
            
            if (ARG_CNT(a) == 0) {
                return CMD_USAGE;
            }
            if (!parseEndOk(a, &pt, &ret_code)) {
                return ret_code;
            } 
            if (arg_port == -1) {
               return CMD_USAGE; 
            }
            if ((arg_limit == -1) && (arg_age == -1)) {
                limit.port = arg_port;
                rv = bcm_l2_learn_limit_get(unit, &limit);
                if (BCM_FAILURE(rv)) {
                    cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                    return CMD_FAIL;
                }
                rv = bcm_l2_age_timer_get(unit, &arg_age);
                if (BCM_FAILURE(rv)) {
                    cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                    return CMD_FAIL;
                } 
                cli_out("port_idx:%2d  limit:%4d  age:%4d\n", 
                        arg_port, limit.limit, arg_age);
                
                return CMD_OK;
            }
            
            if (arg_limit > 0) {
                limit.port = arg_port;
                limit.flags = BCM_L2_LEARN_LIMIT_PORT;
                rv = bcm_l2_learn_limit_set(unit, &limit);
                if (BCM_FAILURE(rv)) {
                    cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                    return CMD_FAIL;
                } 
            }
            
            if (arg_age > 0) {
                rv = bcm_l2_age_timer_set(unit, arg_age);
                if (BCM_FAILURE(rv)) {
                    cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                    return CMD_FAIL;
                } 
            }
            return  CMD_OK;
            
        } else {
            return CMD_USAGE;
        }
        
        return CMD_OK;
         
    } else if (! sal_strcasecmp(subcmd, "show") 
            || ! sal_strcasecmp(subcmd, "-d")) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Port",            PQ_DFL|PQ_PORT,
                0, &arg_port,   NULL);
        if (ARG_CNT(a) != 0) {
            if (!parseEndOk(a, &pt, &ret_code)) {
                return ret_code;
            }
        }
        rv = bcm_l2_traverse(unit, _l2_addr_tranverse_callback, 
                (void *)&arg_port);
        if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;       
    } else if (! sal_strcasecmp(subcmd, "clear") 
            || ! sal_strcasecmp(subcmd, "clr")) {       
        rv = bcm_l2_detach(unit);
        if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;       
    } else if (! sal_strcasecmp(subcmd, "dump")) {
        rv = bcm_l2_traverse(unit, _l2_addr_tranverse_callback, 
                (void *)&arg_port);
        if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;             
    } else {
        return CMD_USAGE;
    }
}


/*
 * Multicast command
 */

cmd_result_t
if_ea_mcast(int unit, args_t *a)
{
	return CMD_OK;
}
