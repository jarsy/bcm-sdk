/* 
 * $Id: ipmc.c,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        ipmc.c
 * Purpose:     IPMC CLI Commands
 *
 */
#include <shared/bsl.h>

#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#include <bcm/error.h>
#include <bcm/ipmc.h>
#include <bcm/debug.h>

STATIC cmd_result_t
_cmd_sbx_ipmc_sg_flow(int unit, args_t *args);

#define CMD_IPMC_FLOW_USAGE                                     \
    "    sg-flow [add | replace | remove | delete-all | find | get]\n"       \
    "         [s=<sip>] [g=<dip>]\n"                            \
    "         [vid=<vid>] [ipmc-index=<ipmc-index>] [valid=<1 or 0>] [modid=<modid>]\n"     \
    "         [ipmc-index=<ipmc-index>] [port=<port>] [untag=<1|0>]\n"        \
    "         [intf-id=<intf-id>] \n" \
    "         [if-count=<if-count> if1=<intf-id> if2=<intf-id> if3=<intf-id>]\n"

static cmd_t _cmd_sbx_ipmc_list[] = {
    {"sg-flow",   _cmd_sbx_ipmc_sg_flow,   "\n" CMD_IPMC_FLOW_USAGE, NULL},
};

char cmd_sbx_ipmc_usage[] =
    "\n"
#ifdef COMPILER_STRING_CONST_LIMIT
    "    ipmc <option> [args...]\n"
#else
    CMD_IPMC_FLOW_USAGE  "\n"
#endif
    ;
/*
 * Local global variables to remember last values in arguments.
 */

STATIC cmd_result_t
_cmd_sbx_ipmc_sg_flow(int unit, args_t *args)
{
    cmd_result_t       retCode;
    int                rv, vid;
    parse_table_t      pt;
    char	      *subcmd;
    bcm_ipmc_addr_t    data;

    if ((subcmd = ARG_GET(args)) == NULL) {
        cli_out("%s ERROR: empty cmd\n", FUNCTION_NAME());
        return CMD_FAIL;
    }

    vid = 0;
    
    bcm_ipmc_addr_t_init(&data);
    
    parse_table_init(unit, &pt);

    /* Parse command option arguments */
    parse_table_add(&pt, "s", PQ_DFL|PQ_IP,
                    0, &data.s_ip_addr, NULL);
    parse_table_add(&pt, "g", PQ_DFL|PQ_IP,
                    0, &data.mc_ip_addr, NULL);
    parse_table_add(&pt, "vid",  PQ_DFL|PQ_INT,
                    0, &vid, NULL);
    parse_table_add(&pt, "ipmc-index", PQ_DFL|PQ_HEX,
                    0, &data.group, NULL);
    parse_table_add(&pt, "valid", PQ_DFL|PQ_INT,
                    0, &data.v, NULL);
    parse_table_add(&pt, "modid", PQ_DFL|PQ_INT,
                    0, &data.mod_id, NULL);
    parse_table_add(&pt, "port_tgid", PQ_DFL|PQ_INT,
                    0, &data.port_tgid, NULL);
    
    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    data.vid = vid;
    
    if (sal_strcasecmp(subcmd, "add") == 0) {

        rv = bcm_ipmc_add(unit, &data);
        if (rv == BCM_E_NONE) {
            cli_out("Added (0x%x, 0x%x, %d) with ipmc-index(0x%x)\n",
                    data.s_ip_addr, data.mc_ip_addr,
                    data.vid, data.group);
        } else {
            cli_out("Error(%s) adding (0x%x, 0x%x, %d) ipmc-index(0x%x)\n",
                    bcm_errmsg(rv), data.s_ip_addr, data.mc_ip_addr,
                    data.vid, data.group);
            return CMD_FAIL;
        }
        
    } else if (sal_strcasecmp(subcmd, "replace") == 0) {

        data.flags |= BCM_IPMC_REPLACE;
        
        rv = bcm_ipmc_add(unit, &data);
        if (rv == BCM_E_NONE) {
            cli_out("Replace (0x%x, 0x%x, %d) with ipmc-index(0x%x)\n",
                    data.s_ip_addr, data.mc_ip_addr,
                    data.vid, data.group);
        } else {
            cli_out("Error(%s) replacing (0x%x, 0x%x, %d) ipmc-index(0x%x)\n",
                    bcm_errmsg(rv), data.s_ip_addr, data.mc_ip_addr,
                    data.vid, data.group);
            return CMD_FAIL;
        }
        
    } else if (sal_strcasecmp(subcmd, "find") == 0) {

        rv = bcm_ipmc_find(unit, &data);
        if (rv == BCM_E_NONE) {
            cli_out("(0x%x, 0x%x, %d) --> ipmc-index(0x%x), cos(0x%x)\n",
                    data.s_ip_addr, data.mc_ip_addr, data.vid,
                    data.group, data.cos);
        } else {
            cli_out("error(%s) bcm_ipmc_find()\n",
                    bcm_errmsg(rv));
            return CMD_FAIL;
        }
    } else if (sal_strcasecmp(subcmd, "get") == 0) {
        /* bcm_ipmc_get_by_index and bcm_ipmc_get are deprecated */
        rv = BCM_E_UNAVAIL;
        cli_out("error(%s) bcm_ipmc_get() or bcm_ipmc_get_by_index()\n",
                bcm_errmsg(rv));
        return CMD_FAIL;
    } else if (sal_strcasecmp(subcmd, "delete-all") == 0) {

        rv = bcm_ipmc_remove_all(unit);
        if (rv == BCM_E_NONE) {
            cli_out("Successfully deleted all ipmc sg\n");
        } else {
            cli_out("Error(%s) deleting all ipmc sg\n",
                    bcm_errmsg(rv));
            return CMD_FAIL;
        }
    } else if (sal_strcasecmp(subcmd, "remove") == 0) {

        rv = bcm_ipmc_remove(unit, &data);
        if (rv == BCM_E_NONE) {
            cli_out("Removed (0x%x, 0x%x, %d) with ipmc-index(0x%x)\n",
                    data.s_ip_addr, data.mc_ip_addr,
                    data.vid, data.group);
        } else {
            cli_out("Error(%s) removing (0x%x, 0x%x, %d) ipmc-index(0x%x)\n",
                    bcm_errmsg(rv), data.s_ip_addr, data.mc_ip_addr,
                    data.vid, data.group);
            return CMD_FAIL;
        }
    } else {
        cli_out("invalid sub-command\n");
        return CMD_FAIL;
    }
    
    return CMD_OK;
}


cmd_result_t
cmd_sbx_ipmc(int unit, args_t *args)
{
    return subcommand_execute(unit,
                              args, 
                              _cmd_sbx_ipmc_list,
                              COUNTOF(_cmd_sbx_ipmc_list));
}

/*-----------------------------------------------------------
  TEST RUN:: Convert to tcl script and work with packets(TBD)
 
  pbmp ge0, ge12, cpu
  
  vlan create 2 PortBitMap=0x000000000000000080001001
  vlan create 3 PortBitMap=0x000000000000000080001001
  vlan create 4 PortBitMap=0x000000000000000080001001
  vlan create 5 PortBitMap=0x000000000000000080001001
  vlan create 6 PortBitMap=0x000000000000000080001001

  l3 intf add SMAC=00:00:22:22:22:22 vid=0x2 ttl=1
  l3 intf add SMAC=00:00:33:33:33:33 vid=0x3 ttl=1
  l3 intf add SMAC=00:00:44:44:44:44 vid=0x4 ttl=1
  l3 intf add SMAC=00:00:55:55:55:55 vid=0x5 ttl=1
  l3 intf add SMAC=00:00:66:66:66:66 vid=0x6 ttl=1

  [0xca000020 0xca000021 0xca000022 0xca000023 0xca000024]
  ipmc sg-flow add s=2.2.2.2 g=224.1.1.1 vid=2 ipmc-index=4096
  ipmc sg-flow add s=3.3.3.3 g=224.1.1.1 vid=3 ipmc-index=4097
  ipmc sg-flow add s=4.4.4.4 g=224.1.1.1 vid=4 ipmc-index=4098
  ipmc sg-flow add s=5.5.5.5 g=224.1.1.1 vid=5 ipmc-index=4099
  ipmc sg-flow add s=6.6.6.6 g=224.1.1.1 vid=6 ipmc-index=4100

  ipmc sg-flow get ipmc-index=4096
  ipmc sg-flow get ipmc-index=4097
  ipmc sg-flow get ipmc-index=4098
  ipmc sg-flow get ipmc-index=4099
  ipmc sg-flow get ipmc-index=4100

  ipmc sg-flow find ipmc-index=4096
  ipmc sg-flow find s=2.2.2.2 g=224.1.1.1 vid=2

  #add egress
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000021
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf get ipmc-index=4096 port=ge1

  #add duplicate intf
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000021

  #delete egress with 1
  ipmc egr-intf delete ipmc-index=4096 port=ge0 intf-id=0xca000021
  ipmc egr-intf get ipmc-index=4096 port=ge0

  # delete egress when > 1 egr present
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000021
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000022
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000023
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf delete ipmc-index=4096 port=ge0 intf-id=0xca000022
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf delete ipmc-index=4096 port=ge0 intf-id=0xca000023
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf delete ipmc-index=4096 port=ge0 intf-id=0xca000021
  ipmc egr-intf get ipmc-index=4096 port=ge0

  # delete when intf not present
  ipmc egr-intf delete ipmc-index=4096 port=ge0 intf-id=0xca000021

  # delete-all when egr is null
  ipmc egr-intf delete-all ipmc-index=4096 port=ge0

  # delete-all when egr is 1
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000021
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf delete-all ipmc-index=4096 port=ge0
  ipmc egr-intf get ipmc-index=4096 port=ge0

  # delete-all when egr is > 1
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000021
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000022
  ipmc egr-intf add ipmc-index=4096 port=ge12 untag=0 intf-id=0xca000022
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf get ipmc-index=4096 port=ge12
  ipmc egr-intf delete-all ipmc-index=4096 port=ge0
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf get ipmc-index=4096 port=ge12
  ipmc egr-intf delete-all ipmc-index=4096 port=ge12
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf get ipmc-index=4096 port=ge12

  # delete 1 intf when egr > 1
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000021
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000022
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000023
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf delete ipmc-index=4096 port=ge0 intf-id=0xca000022
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf delete ipmc-index=4096 port=ge0 intf-id=0xca000023
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf delete ipmc-index=4096 port=ge0 intf-id=0xca000021
  ipmc egr-intf get ipmc-index=4096 port=ge0

  # remove without egr
  ipmc sg-flow remove ipmc-index=4096
  ipmc sg-flow get ipmc-index=4096
  ipmc sg-flow add s=2.2.2.2 g=224.1.1.1 vid=2 ipmc-index=4096

  # remove with 1 egr 
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000021
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc sg-flow remove ipmc-index=4096
  ipmc sg-flow get ipmc-index=4096
  ipmc sg-flow add s=2.2.2.2 g=224.1.1.1 vid=2 ipmc-index=4096
  ipmc sg-flow get ipmc-index=4096
  ipmc egr-intf get ipmc-index=4096 port=ge0

  # remove with > 1 egr 
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000021
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000022
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000023
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc sg-flow remove ipmc-index=4096
  ipmc sg-flow get ipmc-index=4096
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc sg-flow add s=2.2.2.2 g=224.1.1.1 vid=2 ipmc-index=4096
  ipmc sg-flow get ipmc-index=4096
  ipmc egr-intf get ipmc-index=4096 port=ge0

  # egress_set
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000021
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000022
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000023
  ipmc egr-intf add ipmc-index=4096 port=ge12 untag=0 intf-id=0xca000021
  ipmc egr-intf add ipmc-index=4096 port=ge12 untag=0 intf-id=0xca000022
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf get ipmc-index=4096 port=ge12
  ipmc egr-intf set ipmc-index=4096 port=ge0 if-count=3 if1=0xca000021 if2=0xca000024 if3=0xca000020
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf get ipmc-index=4096 port=ge12

  # final removal of all s,g state
  ipmc sg-flow delete-all ipmc-index=0
  ipmc sg-flow get ipmc-index=4096
  ipmc sg-flow get ipmc-index=4097
  ipmc sg-flow get ipmc-index=4098
  ipmc sg-flow get ipmc-index=4099
  ipmc sg-flow get ipmc-index=4100

  *-------------------------------------------------------*/
