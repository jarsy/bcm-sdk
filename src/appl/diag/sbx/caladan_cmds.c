/*
 * $Id: caladan_cmds.c,v 1.15.20.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        caladan_cmds.c
 * Purpose:     FE-2000/Caladan3-specific diagnostic shell commands
 * Requires:
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <soc/defs.h>

#if defined(BCM_CALADAN3_SUPPORT)
#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <appl/diag/shell.h>
#include <appl/diag/sbx/sbx.h>
#include <appl/diag/sbx/register.h>
#include <appl/diag/sbx/field.h>
#include <appl/diag/sbx/gu2.h>
#include <appl/test/sbx_pkt.h>
#include <soc/sbx/sbx_drv.h>
#include <appl/diag/sbx/caladan3_cmds.h>
#include <soc/mem.h>
#include <soc/mcm/allenum.h>

char cmd_sbx_caladan_hc_usage[] =
"Usage:\n"
#ifndef COMPILER_STRING_CONST_LIMIT
"  with no argument prints the current state of header capture\n"
"  on              - enable header capture\n"
"  off             - disable header capture\n"
"  queue=n         - enable PPE capture for the specified queue\n"
#ifdef BCM_CALADAN3_SUPPORT
"  show            - show header capture configuration (c3 only)\n"
"  clear           - clear header capture buffer (c3 only)\n"
"  drop            - enable PED capture for drop packet (c3 only)\n"
"  stream          - enable PPE capture for packets to specified stream (c3 only)\n"
"  variable        - enable PPE capture for the packets with variable matching given variable & variable_mask (c3 only)\n"
"  variable_mask   - required for variable matching (c3 only) \n"
"  word            - enable PED capture for packets match with specified data and mask, word=0~15 (c3 only)\n"
"  data            - required for word matching (c3 only) \n"
"  data_mask       - required for word matching (c3 only) \n"
"  ppe             - update PPE configuration only (c3 only) \n"
"  ped             - update PED configuration only (c3 only) \n"
#endif
#endif
;

cmd_result_t
cmd_sbx_caladan_hc(int unit, args_t *a)
{
    int on = 0, off = 0, queue = -1, drop = 0, show = -1;
    int var = 0, varmask = 0, str = -1, clear = 0, word = -1, ppe = 0, ped = 0;
    uint32 data = 0, data_mask = 0;
    cmd_result_t result = CMD_OK;

    if (ARG_CNT(a)) {
        int ret_code;
        parse_table_t pt;
        parse_table_init(0, &pt);

        parse_table_add(&pt, "on", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &on, NULL);
        parse_table_add(&pt, "off", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &off, NULL);
        parse_table_add(&pt, "queue", PQ_DFL | PQ_INT,
                        0, &queue, NULL);
        parse_table_add(&pt, "drop", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &drop, NULL);
        parse_table_add(&pt, "clear", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &clear, NULL);
        parse_table_add(&pt, "stream", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &str, NULL);
        parse_table_add(&pt, "variable", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &var, NULL);
        parse_table_add(&pt, "variable_mask", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &varmask, NULL);
        parse_table_add(&pt, "show", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &show, NULL);
        parse_table_add(&pt, "word", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &word, NULL);
        parse_table_add(&pt, "data", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &data, NULL);
        parse_table_add(&pt, "data_mask", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &data_mask, NULL);
        parse_table_add(&pt, "ppe", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &ppe, NULL);
        parse_table_add(&pt, "ped", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &ped, NULL);
        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }
    }

    if (on && off) {
        return CMD_USAGE;
    }

    if ((ped == 0) && (ppe == 0)) {
        ppe = 1;
        ped = 1;
    }

#ifdef BCM_CALADAN3_SUPPORT
    if (SOC_IS_SBX_CALADAN3(unit)) {
    	if (ppe > 0) {
            if (show > 0) {
                result = cmd_sbx_caladan3_ppe_show(unit);
            }
            else if (word >= 0) {
                cli_out("PPE does not support word check.\n");
            }
            else {
                result = cmd_sbx_caladan3_ppe_hc(unit, on, clear,
                                                 queue, str, var, varmask);
            }
    	}
    	if (ped > 0) {
    	    if (show > 0) {
    	        result = cmd_sbx_caladan3_ped_show(unit);
    	    }
    	    else if (clear > 0) {
    	        result = cmd_sbx_caladan3_ped_clear(unit);
    	    }
    	    else if (word >= 0) {
    	        result = cmd_sbx_caladan3_ped_check(unit, word, data, data_mask);
    	    }
    	    else {
    	        result = cmd_sbx_caladan3_pd_hc(unit, queue, on, drop);
    	    }
    	}
    }
#endif
    return result;
}

char cmd_sbx_caladan_hd_usage[] =
"Usage:\n"
"  dump captured headers & clear capture buffer\n"
"  ppe             - dump captured header from PPE\n"
"  pedin           - dump captured header from PED input\n"
"  pedout          - dump captured header from ped output\n"
"  raw             - dump raw captured headers\n"
"  parsed          - dump parsed captured headers\n\n"
"if neither raw nor parsed are specified, both are dumped\n"
"if none of ppe, pedin and pedout are specified, all are dumped\n"
;

cmd_result_t
cmd_sbx_caladan_hd(int unit, args_t *a)
{
    int rv = CMD_OK;
    int ppe = -1;
    int pedin = -1;
    int pedout = -1;
    int raw = -1;
    int parsed = -1;

    if (ARG_CNT(a)) {
        int ret_code;
        parse_table_t pt;
        parse_table_init(0, &pt);

        parse_table_add(&pt, "ppe", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &ppe, NULL);
        parse_table_add(&pt, "pedin", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &pedin, NULL);
        parse_table_add(&pt, "pedout", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &pedout, NULL);
        parse_table_add(&pt, "raw", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &raw, NULL);
        parse_table_add(&pt, "parsed", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &parsed, NULL);

        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }
    }

    if (ppe == -1 && pedin == -1 && pedout == -1) {
        ppe = 1;
        pedin = 1;
        pedout = 1;
    }
    ppe = (ppe == -1) ? 0 : ppe;
    pedin = (pedin == -1) ? 0 : pedin;
    pedout = (pedout == -1) ? 0 : pedout;

    if (raw == -1 && parsed == -1) {
        raw = 0;
	parsed = 1;
    }
    raw = (raw == -1) ? 0 : raw;
    parsed = (parsed == -1) ? 0 : parsed;


#ifdef BCM_CALADAN3_SUPPORT
    if (SOC_IS_SBX_CALADAN3(unit)) {
        if (ppe) {
            rv = cmd_sbx_caladan3_ppe_hd(unit, parsed);
            if (rv == CMD_FAIL) {
                return rv;
            }
        }
        if (pedin || pedout) {
            rv = cmd_sbx_caladan3_ped_hd(unit, pedin, pedout, parsed);
        }
    }
#endif
    return rv;
}

char cmd_sbx_caladan_nic_config_usage[] =
"Usage:\n"
"nicconfig <options>\n"
"  Configure the `steering' of the Caladan PCI CPU packet port\n"
"  ingress  - loop PCI packets through the ingress microcode\n"
"  egress  - loop PCI packets through the eg2ress microcode\n"
#ifdef BCM_CALADAN3_SUPPORT
"  reset    - caladan3 reset loop on pci port\n "
"  port     - caladan3 sets up loop from cmic->port->macloop->cmic\n"
#endif
;

cmd_result_t
cmd_sbx_caladan_nic_config(int unit, args_t *a)
{
    int ingress = -1, egress = -1;
#ifdef BCM_CALADAN3_SUPPORT
    int reset=-1, port=-1, status;
#endif
    cmd_result_t rv=CMD_OK;

    if (ARG_CNT(a)) {
        int ret_code;
        parse_table_t pt;
        parse_table_init(0, &pt);

        parse_table_add(&pt, "ingress", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &ingress, NULL);
        parse_table_add(&pt, "egress", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &egress, NULL);
#ifdef BCM_CALADAN3_SUPPORT
        if (SOC_IS_SBX_CALADAN3(unit)) {
            parse_table_add(&pt, "reset", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                            0, &reset, NULL);
            parse_table_add(&pt, "port", PQ_DFL | PQ_INT,
                            0, &port, NULL);
        }

#endif
        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }
    }

#ifdef BCM_CALADAN3_SUPPORT
    if (SOC_IS_SBX_CALADAN3(unit)) {
        if (port >= 0) {
            status = soc_sbx_caladan3_sws_cmic_to_port_mac_loopback(unit, 
                                                                    port,
                                                                    reset);
        } else if (ingress > 0 || egress > 0 || reset > 0) {
            status = soc_sbx_caladan3_sws_cmic_port_hpp_loopback(unit,
                                                                 ingress,
                                                                 egress,
                                                                 reset);
            if (SOC_FAILURE(status)) {
                rv = CMD_FAIL;
            }
        } else {
            cli_out("Nothing Executed !!!! \n");
        }
    }
#endif

    return rv;
}

#endif
