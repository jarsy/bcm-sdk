/*
 * $Id: qe2000_cmds.c,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        qe2000_cmds.c
 * Purpose:     QE-2000-specific diagnostic shell commands
 * Requires:
 */


#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <soc/defs.h>

#ifdef BCM_QE2000_SUPPORT
#include <appl/diag/system.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_user.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/qe2000.h>
#include <soc/sbx/qe2000_mvt.h>
#include <soc/sbx/qe2000_util.h>
#include <appl/diag/sbx/qe2000_cmds.h>
#include <appl/diag/shell.h>
#include <appl/diag/sbx/sbx.h>
#include <appl/diag/sbx/register.h>
#include <appl/diag/sbx/field.h>
#include <appl/diag/sbx/gu2.h>
#include <bcm/error.h>
#include <bcm/init.h>
#include <bcm/rx.h>
#include <bcm/vlan.h>
#include <bcm/stg.h>
#include <bcm/mcast.h>
#include <bcm/trunk.h>
#include <bcm/stack.h>
#include <bcm/cosq.h>

#include <shared/idxres_fl.h>
#include <shared/idxres_afl.h>

int
cmd_sbx_qe2000_print_queue_info(int unit, int queue)
{
    int rv, mc, node, port, cos, baseq, enabled;
    
    rv = soc_qe2000_queue_info_get(unit, queue, &mc, &node, &port,
                                &cos, &baseq, &enabled);
    if(rv) {
        cli_out("Failed to get queue info for Queue=%d status=%d\n",
                queue, rv);
        return CMD_FAIL;
    } else {
        cli_out("q=%d (base q=%d) is %sABLED:\nNode=%d Port=%d cos=%d mc=%d\n",
                queue, baseq, enabled ? "EN":"DIS",
                node, port, cos, mc);
    }
    return CMD_OK;
}

char cmd_sbx_qe2000_qsindirect_usage[] = "\n"
"QSINdirect <memory> [<range> [value(s)]]\n"
#ifndef COMPILER_STRING_CONST_LIMIT
"  Only applicable for QE-2000\n"
"  memory: \n"
"    ratea, rateb, credit, depth_hplen, q2ec, queue_para, shape_rate,\n"
"    shape_max_burst, shape, age, age_thresh_key, age_thresh, pri_lut, priority, e2q, lastsentpri\n"
"  If the memory only contains a single field, you can directly specify the value to write\n"
"  For multi-field memories, specify the value to write in <field>=<value> pairs and\n"
"  make sure you specify all fields\n"
"  range: specify as min-max for a range or a single number for a single entry\n"
#endif
;

cmd_result_t
cmd_sbx_qe2000_qsindirect(int unit, args_t *args)
{
    char *mem;
    char *range;
    char *lo;
    char *hi = NULL;
    char *value;
    int idx;
    int start_idx;
    int end_idx;
    int val = 0;
    int ret_code;

    cli_out("cdp cmd_sbx_qe2000_qsindirect\n"); 

    if(!SOC_IS_SBX_QE2000(unit)) {
        return CMD_USAGE;
    }

    if (!(mem = ARG_GET(args))) {
        return CMD_USAGE;
    }

    if((lo = range = ARG_GET(args)) != NULL) {
        if((hi = strchr(range, '-')) != NULL) {
            hi++;
        } else {
            hi = lo;
        }
    }

    if(!sal_strcasecmp(mem, "ratea")) {
        if(diag_parse_range(lo, hi, &start_idx, &end_idx, 0, 16383)) {
            cli_out("Invalid range. Valid range is : 0-16383\n");
            return CMD_FAIL;
        }
        if(!(value = ARG_GET(args))) {
            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_rate_a_get(unit, idx, &val)) {
                    cli_out("Error getting Rate A for queue %d\n", idx);
                    return CMD_FAIL;
                }
                cli_out("Queue: %5d\tRate A: 0x%08X\n", idx, val);
            }
        } else {
            val = sal_ctoi(value, 0);
            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_rate_a_set(unit, idx, val)) {
                    cli_out("Error setting Rate A for queue %d\n", idx);
                    return CMD_FAIL;
                }
            }
        }
    } else if(!sal_strcasecmp(mem, "rateb")) {
        if(diag_parse_range(lo, hi, &start_idx, &end_idx, 0, 16383)) {
            cli_out("Invalid range. Valid range is : 0-16383\n");
            return CMD_FAIL;
        }
        if(!(value = ARG_GET(args))) {
            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_rate_b_get(unit, idx, (uint32 *)&val)) {
                    cli_out("Error getting Rate B for queue %d\n", idx);
                    return CMD_FAIL;
                }
                cli_out("Queue: %5d\tRate B: 0x%08X\n", idx, val);
            }
        } else {
            val = sal_ctoi(value, 0);
            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_rate_b_set(unit, idx, (uint32)val)) {
                    cli_out("Error setting Rate A for queue %d\n", idx);
                    return CMD_FAIL;
                }
            }
        }
    } else if(!sal_strcasecmp(mem, "credit")) {
        if(diag_parse_range(lo, hi, &start_idx, &end_idx, 0, 16383)) {
            cli_out("Invalid range. Valid range is : 0-16383\n");
            return CMD_FAIL;
        }
        if(!(value = ARG_GET(args))) {
            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_credit_get(unit, idx, (uint32*)&val)) {
                    cli_out("Error getting credit for queue %d\n", idx);
                    return CMD_FAIL;
                }
                cli_out("Queue: %5d\tCredit: 0x%08X\n", idx, val);
            }
        } else {
            val = sal_ctoi(value, 0);
            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_credit_set(unit, idx, (uint32)val)) {
                    cli_out("Error setting Credit for queue %d\n", idx);
                    return CMD_FAIL;
                }
            }
        }
    } else if(!sal_strcasecmp(mem, "depth_hplen")) {
        int depth = 0;
        int pktlen = 0;

        if(diag_parse_range(lo, hi, &start_idx, &end_idx, 0, 16383)) {
            cli_out("Invalid range. Valid range is : 0-16383\n");
            return CMD_FAIL;
        }
        if(!ARG_CNT(args)) {
            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_depth_length_get(unit, idx, &depth, &pktlen)) {
                    cli_out("Error getting depth_hplen for queue %d\n", idx);
                    return CMD_FAIL;
                }
                cli_out("Queue: %5d\tDepth: 0x%X\tPktlen: 0x%X\n", idx,
                        depth, pktlen);
            }
        } else {
            parse_table_t pt;

            parse_table_init(0, &pt);
            parse_table_add(&pt, "PktLen",  PQ_DFL | PQ_INT,
                            0, &pktlen, NULL);
            parse_table_add(&pt, "Depth",   PQ_DFL | PQ_INT,
                            0, &depth, NULL);
            if (!parseEndOk(args, &pt, &ret_code)) {
                return ret_code;
            }

            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_depth_length_set(unit, idx, depth, pktlen)) {
                    cli_out("Error setting depth_hplen for queue %d\n", idx);
                    return CMD_FAIL;
                }
            }
        }
    } else if(!sal_strcasecmp(mem, "q2ec")) {
        int mc = 0;
        int node = 0;
        int port = 0;
        int cos = 0;

        if(diag_parse_range(lo, hi, &start_idx, &end_idx, 0, 16383)) {
            cli_out("Invalid range. Valid range is : 0-16383\n");
            return CMD_FAIL;
        }
        if(!ARG_CNT(args)) {
            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_q2ec_get(unit, idx, &mc, &node, &port, &cos)) {
                    cli_out("Error getting q2ec for queue %d\n", idx);
                    return CMD_FAIL;
                }
                cli_out("Queue: %5d\tMc=%d  Node=0x%02X  Port=%d  Cos=%d\n", idx,
                        mc, node, port, cos);
            }
        } else {
            parse_table_t pt;

            parse_table_init(0, &pt);
            parse_table_add(&pt, "Mc",  PQ_DFL | PQ_INT,
                            0, &mc, NULL);
            parse_table_add(&pt, "Node",   PQ_DFL | PQ_INT,
                            0, &node, NULL);
            parse_table_add(&pt, "Port",   PQ_DFL | PQ_INT,
                            0, &port, NULL);
            parse_table_add(&pt, "Cos",   PQ_DFL | PQ_INT,
                            0, &cos, NULL);
            if (!parseEndOk(args, &pt, &ret_code)) {
                return ret_code;
            }

            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_q2ec_set(unit, idx, mc, node, port, cos)) {
                    cli_out("Error setting q2ec for queue %d\n", idx);
                    return CMD_FAIL;
                }
            }
        }
    } else if(!sal_strcasecmp(mem, "queue_para")) {
        int local = 0;
        int hold_ts = 0;
        int q_type = 0;

        if(diag_parse_range(lo, hi, &start_idx, &end_idx, 0, 16383)) {
            cli_out("Invalid range. Valid range is : 0-16383\n");
            return CMD_FAIL;
        }
        if(!ARG_CNT(args)) {
            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_queue_para_get(unit, idx, &local, &hold_ts, &q_type)) {
                    cli_out("Error getting queue_para for queue %d\n", idx);
                    return CMD_FAIL;
                }
                cli_out("Queue: %5d\tlocal=%d  hold_ts=0x%X  q_type=0x%X\n", idx,
                        local, hold_ts, q_type);
            }
        } else {
            parse_table_t pt;

            parse_table_init(0, &pt);
            parse_table_add(&pt, "Local",  PQ_DFL | PQ_INT,
                            0, &local, NULL);
            parse_table_add(&pt, "Hold_ts",   PQ_DFL | PQ_INT,
                            0, &hold_ts, NULL);
            parse_table_add(&pt, "Q_type",   PQ_DFL | PQ_INT,
                            0, &q_type, NULL);
            if (!parseEndOk(args, &pt, &ret_code)) {
                return ret_code;
            }

            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_queue_para_set(unit, idx, local, hold_ts, q_type)) {
                    cli_out("Error setting queue_para for queue %d\n", idx);
                    return CMD_FAIL;
                }
            }
        }
    } else if(!sal_strcasecmp(mem, "shape_rate")) {
        int shape_mode = 0;
        int shape_rate = 0;
        cli_out("cdp shape_rate\n");
        if(diag_parse_range(lo, hi, &start_idx, &end_idx, 0, 16383)) {
            cli_out("Invalid range. Valid range is : 0-16383\n");
            return CMD_FAIL;
        }
        if(!ARG_CNT(args)) {
            for(idx = start_idx; idx <= end_idx; idx++) {
                cli_out("cdp shape_rate get\n");
                if(soc_qe2000_shape_rate_get(unit, idx, &shape_mode, &shape_rate)) {
                    cli_out("Error getting shape_rate for queue %d\n", idx);
                    return CMD_FAIL;
                }
                cli_out("Queue: %5d\tShape Mode=%d  shape_rates=0x%X\n", idx,
                        shape_mode, shape_rate);
            }
        } else {
            parse_table_t pt;
            cli_out("cdp shape_rate set\n");

            parse_table_init(0, &pt);
            parse_table_add(&pt, "Mode",  PQ_DFL | PQ_INT,
                            0, &shape_mode, NULL);
            parse_table_add(&pt, "Rate",   PQ_DFL | PQ_INT,
                            0, &shape_rate, NULL);
            if (!parseEndOk(args, &pt, &ret_code)) {
                return ret_code;
            }

            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_shape_rate_set(unit, idx, shape_mode, shape_rate)) {
                    cli_out("Error setting shape_rate for queue %d\n", idx);
                    return CMD_FAIL;
                }
            }
        }
    } else if(!sal_strcasecmp(mem, "shape_max_burst")) {
        int shape_enable = 0;
        int shape_maxburst = 0;

        if(diag_parse_range(lo, hi, &start_idx, &end_idx, 0, 16383)) {
            cli_out("Invalid range. Valid range is : 0-16383\n");
            return CMD_FAIL;
        }
        if(!ARG_CNT(args)) {
            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_shape_maxburst_get(unit, idx, &shape_enable, &shape_maxburst)) {
                    cli_out("Error getting shape_max_burst for queue %d\n", idx);
                    return CMD_FAIL;
                }
                cli_out("Queue: %5d\tShape Enable=%d  Maxburst=0x%X\n", idx,
                        shape_enable, shape_maxburst);
            }
        } else {
            parse_table_t pt;

            parse_table_init(0, &pt);
            parse_table_add(&pt, "Enable",  PQ_DFL | PQ_INT,
                            0, &shape_enable, NULL);
            parse_table_add(&pt, "Maxburst",   PQ_DFL | PQ_INT,
                            0, &shape_maxburst, NULL);
            if (!parseEndOk(args, &pt, &ret_code)) {
                return ret_code;
            }

            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_shape_maxburst_set(unit, idx, shape_enable, shape_maxburst)) {
                    cli_out("Error setting shape_max_burst for queue %d\n", idx);
                    return CMD_FAIL;
                }
            }
        }
    } else if(!sal_strcasecmp(mem, "shape")) {
        if(diag_parse_range(lo, hi, &start_idx, &end_idx, 0, 16383)) {
            cli_out("Invalid range. Valid range is : 0-16383\n");
            return CMD_FAIL;
        }
        if(!(value = ARG_GET(args))) {
            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_shape_bucket_get(unit, idx, &val)) {
                    cli_out("Error getting shape bucket for queue %d\n", idx);
                    return CMD_FAIL;
                }
                cli_out("Queue: %5d\tShape_bucket: 0x%08X\n", idx, val);
            }
        } else {
            val = sal_ctoi(value, 0);
            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_shape_bucket_set(unit, idx, val)) {
                    cli_out("Error setting Shape bucket for queue %d\n", idx);
                    return CMD_FAIL;
                }
            }
        }
    } else if(!sal_strcasecmp(mem, "age")) {
        int nonempty = 0;
        int anemic_event = 0;
        int ef_event = 0;
        int cnt = 0;
        
        if(diag_parse_range(lo, hi, &start_idx, &end_idx, 0, 16383)) {
            cli_out("Invalid range. Valid range is : 0-16383\n");
            return CMD_FAIL;
        }
        if(!ARG_CNT(args)) {
            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_age_get(unit, idx, &nonempty, &anemic_event, &ef_event, &cnt)) {
                    cli_out("Error getting age for queue %d\n", idx);
                    return CMD_FAIL;
                }
                cli_out("Queue: %5d\tNonempty=%d anemic_event=%d ef_event=%d cnt=0x%02X\n", idx,
                        nonempty, anemic_event, ef_event, cnt);
            }
        } else {
            parse_table_t pt;

            parse_table_init(0, &pt);
            parse_table_add(&pt, "Nonempty",  PQ_DFL | PQ_INT,
                            0, &nonempty, NULL);
            parse_table_add(&pt, "Anemicevent",   PQ_DFL | PQ_INT,
                            0, &anemic_event, NULL);
            parse_table_add(&pt, "Efevent",   PQ_DFL | PQ_INT,
                            0, &ef_event, NULL);
            parse_table_add(&pt, "Cnt",   PQ_DFL | PQ_INT,
                            0, &cnt, NULL);
            if (!parseEndOk(args, &pt, &ret_code)) {
                return ret_code;
            }

            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_age_set(unit, idx, nonempty, anemic_event, ef_event, cnt)) {
                    cli_out("Error setting age for queue %d\n", idx);
                    return CMD_FAIL;
                }
            }
        }
    } else if(!sal_strcasecmp(mem, "age_thresh_key")) {
        if(diag_parse_range(lo, hi, &start_idx, &end_idx, 0, 16383)) {
            cli_out("Invalid range. Valid range is : 0-16383\n");
            return CMD_FAIL;
        }
        if(!(value = ARG_GET(args))) {
            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_age_thresh_key_get(unit, idx, &val)) {
                    cli_out("Error getting age_thresh_key for queue %d\n", idx);
                    return CMD_FAIL;
                }
                cli_out("Queue: %5d\tage_thresh_key: 0x%02X\n", idx, val);
            }
        } else {
            val = sal_ctoi(value, 0);
            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_age_thresh_key_set(unit, idx, val)) {
                    cli_out("Error setting age_thresh_key for queue %d\n", idx);
                    return CMD_FAIL;
                }
            }
        }
    } else if(!sal_strcasecmp(mem, "age_thresh")) {
        int anemic_thresh = 0;
        int ef_thresh = 0;
        
        if(diag_parse_range(lo, hi, &start_idx, &end_idx, 0, 31)) {
            cli_out("Invalid range. Valid range is : 0-31\n");
            return CMD_FAIL;
        }
        if(!ARG_CNT(args)) {
            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_age_thresh_get(unit, idx, &anemic_thresh, &ef_thresh)) {
                    cli_out("Error getting age_thres for idx %d\n", idx);
                    return CMD_FAIL;
                }
                cli_out("Idx: %5d\tanemic_thresh=%d ef_thresh=%d\n", idx,
                        anemic_thresh, ef_thresh);
            }
        } else {
            parse_table_t pt;

            parse_table_init(0, &pt);
            parse_table_add(&pt, "Anemicthresh",   PQ_DFL | PQ_INT,
                            0, &anemic_thresh, NULL);
            parse_table_add(&pt, "Efthresh",   PQ_DFL | PQ_INT,
                            0, &ef_thresh, NULL);
            if (!parseEndOk(args, &pt, &ret_code)) {
                return ret_code;
            }

            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_age_thresh_set(unit, idx, anemic_thresh, ef_thresh)) {
                    cli_out("Error setting age_thres for idx %d\n", idx);
                    return CMD_FAIL;
                }
            }
        }
    } else if(!sal_strcasecmp(mem, "pri_lut")) {
        int pri = 0;
        int next_pri = 0;
        
        if(diag_parse_range(lo, hi, &start_idx, &end_idx, 0, 8191)) {
            cli_out("Invalid range. Valid range is : 0-8191\n");
            return CMD_FAIL;
        }
        if(!ARG_CNT(args)) {
            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_pri_lut_get(unit, idx, &pri, &next_pri)) {
                    cli_out("Error getting pri_lut for idx %d\n", idx);
                    return CMD_FAIL;
                }
                cli_out("Idx: %5d\tpri=%d next_pri=%d\n", idx,
                        pri, next_pri);
            }
        } else {
            parse_table_t pt;

            parse_table_init(0, &pt);
            parse_table_add(&pt, "Pri",   PQ_DFL | PQ_INT,
                            0, &pri, NULL);
            parse_table_add(&pt, "Next_pri",   PQ_DFL | PQ_INT,
                            0, &next_pri, NULL);
            if (!parseEndOk(args, &pt, &ret_code)) {
                return ret_code;
            }

            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_pri_lut_set(unit, idx, pri, next_pri)) {
                    cli_out("Error setting pri_lut for idx %d\n", idx);
                    return CMD_FAIL;
                }
            }
        }
    } else if(!sal_strcasecmp(mem, "priority")) {
        int shaped = 0;
        int pri = 0;
        int next_pri = 0;
        
        if(diag_parse_range(lo, hi, &start_idx, &end_idx, 0, 16*4224-1)) {
            cli_out("Invalid range. Valid range is : 0-%d\n", 16*4224-1);
            return CMD_FAIL;
        }
        if(!ARG_CNT(args)) {
            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_priority_get(unit, idx, &shaped, &pri, &next_pri)) {
                    cli_out("Error getting priority for idx %d\n", idx);
                    return CMD_FAIL;
                }
                cli_out("Idx: %5d\tshaped=%d pri=%d next_pri=%d\n", idx,
                        shaped, pri, next_pri);
            }
        } else {
            parse_table_t pt;

            parse_table_init(0, &pt);
            parse_table_add(&pt, "Shaped",   PQ_DFL | PQ_INT,
                            0, &shaped, NULL);
            parse_table_add(&pt, "Pri",   PQ_DFL | PQ_INT,
                            0, &pri, NULL);
            parse_table_add(&pt, "Next_pri",   PQ_DFL | PQ_INT,
                            0, &next_pri, NULL);
            if (!parseEndOk(args, &pt, &ret_code)) {
                return ret_code;
            }

            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_priority_set(unit, idx, shaped, pri, next_pri)) {
                    cli_out("Error setting priority for idx %d\n", idx);
                    return CMD_FAIL;
                }
            }
        }
    } else if(!sal_strcasecmp(mem, "e2q")) {
        int mc, node, port;
        int queue = 0;
        int enable = 0;
        
        if(diag_parse_range(lo, hi, &start_idx, &end_idx, 0, 4223)) {
            cli_out("Invalid range. Valid range is : 0-%d\n", 4223);
            return CMD_FAIL;
        }
        if(!ARG_CNT(args)) {
            for(idx = start_idx; idx <= end_idx; idx++) {
                mc = (idx >> 14) & 1;
                node = (idx >> 6) & 0x1F;
                port = (idx & 0xFFF);
                if(soc_qe2000_e2q_get(unit, mc, node, port, &enable, &queue)) {
                    cli_out("Error getting e2q for idx %d\n", idx);
                    return CMD_FAIL;
                }
                cli_out("Idx: %5d\tqueue=%d enable=%d\n", idx,
                        queue, enable);
            }
        } else {
            parse_table_t pt;

            parse_table_init(0, &pt);
            parse_table_add(&pt, "Enable",   PQ_DFL | PQ_INT,
                            0, &enable, NULL);
            parse_table_add(&pt, "Queue",   PQ_DFL | PQ_INT,
                            0, &queue, NULL);
            if (!parseEndOk(args, &pt, &ret_code)) {
                return ret_code;
            }

            for(idx = start_idx; idx <= end_idx; idx++) {
                mc = (idx >> 14) & 1;
                node = (idx >> 6) & 0x1F;
                port = (idx & 0xFFF);
                if(soc_qe2000_e2q_set(unit, mc, node, port, queue, enable)) {
                    cli_out("Error setting e2q for idx %d\n", idx);
                    return CMD_FAIL;
                }
            }
        }
    } else if(!sal_strcasecmp(mem, "lastsentpri")) {
        int pri = 0;
        int next_pri = 0;
        
        if(diag_parse_range(lo, hi, &start_idx, &end_idx, 0, 4223)) {
            cli_out("Invalid range. Valid range is : 0-%d\n", 4223);
            return CMD_FAIL;
        }
        if(!ARG_CNT(args)) {
            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_lastsentpri_get(unit, idx, &pri, &next_pri)) {
                    cli_out("Error getting lastsentpri for idx %d\n", idx);
                    return CMD_FAIL;
                }
                cli_out("Idx: %5d\tpri=%d next_pri=%d\n", idx,
                        pri, next_pri);
            }
        } else {
            parse_table_t pt;

            parse_table_init(0, &pt);
            parse_table_add(&pt, "Pri",   PQ_DFL | PQ_INT,
                            0, &pri, NULL);
            parse_table_add(&pt, "Nextpri",   PQ_DFL | PQ_INT,
                            0, &next_pri, NULL);
            if (!parseEndOk(args, &pt, &ret_code)) {
                return ret_code;
            }

            for(idx = start_idx; idx <= end_idx; idx++) {
                if(soc_qe2000_lastsentpri_set(unit, idx, pri, next_pri)) {
                    cli_out("Error setting lastsentpri for idx %d\n", idx);
                    return CMD_FAIL;
                }
            }
        }
    }

    return CMD_OK;
}


char cmd_sbx_qe2000_mvtget_usage[] = "\n"
"mvtget <index>\n"
"  dumps a QE-2000 MVT entry\n"
;

cmd_result_t
cmd_sbx_qe2000_mvtget(int unit, args_t *args)
{
    HW_QE2000_MVT_ENTRY_ST sbMvtEntry;
    sbhandle sbh;
    char *lo;
    char *hi = NULL;
    char *range;
    sbStatus_t sbxRv;
    int start_idx, end_idx;
    int i, j, ports;
    sbx_qe2000_mvt_entry_t  mvtEntry;
    int     ohi, vid;
    uint data, mcgroup_size;

    if(!SOC_IS_SBX_QE2000(unit)) {
        return CMD_USAGE;
    }

    if((lo = range = ARG_GET(args)) != NULL) {
        if((hi = strchr(range, '-')) != NULL) {
            hi++;
        } else {
            hi = lo;
        }
    }
    
    if(diag_parse_range(lo, hi, &start_idx, &end_idx, 0, (48 * 1024) - 1)) {
        cli_out("Invalid range. Valid range is : 0-%d\n", (48 * 1024) - 1);
        return CMD_FAIL;
    }

    data = SAND_HAL_READ(unit, KA, EG_MC_CONFIG0);
    mcgroup_size = SAND_HAL_GET_FIELD(KA, EG_MC_CONFIG0, MCGROUP_SIZE, data);

    switch (mcgroup_size) {
      case 0:
	mcgroup_size = (12*1024) - 1;
	break;

      case 1:
	mcgroup_size = (24*1024) - 1;
	break;

      case 2:
      default:
	mcgroup_size = (48*1024) -1;
	break;
    }

    if (end_idx > mcgroup_size) {
        cli_out("Invalid high range. MVT entry max configured entry is %d\n", mcgroup_size);
        return CMD_FAIL;
    }

    sbh = SOC_SBX_CONTROL(unit)->sbhdl;
    for (i = start_idx; i <= end_idx; i++) {
        sbxRv = hwQe2000MVTGet(sbh, &sbMvtEntry, i, NULL, NULL, 0, 0, NULL);
        if(sbxRv) {
            cli_out("MVTGet Failed:(%d)\n", sbxRv);
            return CMD_FAIL;
        }
        cli_out("MVT entry %d: mvta=0x%x, mvtb=0x%x, ko=%d, next=0x%x\n",
                i, sbMvtEntry.ulMvtdA, sbMvtEntry.ulMvtdB,
                sbMvtEntry.bSourceKnockout, sbMvtEntry.ulNext);

        if (sbMvtEntry.ulMvtdA != 0) {      /* check if this is a "valid" entry */
            /* this is the essential part of soc_qe2000_mvt_entry_get() */
            mvtEntry.egress_data_a   = sbMvtEntry.ulMvtdA;
            mvtEntry.egress_data_b   = sbMvtEntry.ulMvtdB;
            mvtEntry.source_knockout = sbMvtEntry.bSourceKnockout;
            mvtEntry.next            = sbMvtEntry.ulNext;
            if (SBX_MVT_IS_LI(&mvtEntry)) {
                SBX_MVT_GET_LI_OHI(&mvtEntry, ohi);
                cli_out(" ===> ohi = %d (0x%05X)\n", ohi, ohi);
            } else {
                SBX_MVT_GET_VID(&mvtEntry, vid);
                cli_out(" ===> vid = %d\n", vid);
            }
        }

        ports = 0;
        for (j = 0; j < HW_QE2000_MAX_NUM_PORTS_K; j++) {
            if (sbMvtEntry.bPortEnable[j]) {
                if (ports % 8 == 7) {
                    cli_out("\n");
                }
                cli_out("port %2d ", j);
                ports++;
            }
        }
        if (ports % 8) {
            cli_out("\n");
        }
    }
    cli_out("\n");

    return CMD_OK;
}

char cmd_sbx_qe2000_mvtset_usage[] = "\n"
"mvtset <idx=index>  [a=<value>] [b=<value>] [setportmask=<vlaue>] [portmasklo=<vlaue>] [portmaskhi=<value>]\n"
"  sets a QE-2000 MVT entry\n"
;
 
cmd_result_t
cmd_sbx_qe2000_mvtset(int unit, args_t *args)
{
    HW_QE2000_MVT_ENTRY_ST sbMvtEntry;
    sbhandle sbh;
    sbStatus_t sbRv;
    uint32 data_a = ~0, data_b = ~0;
    int knockout = -1;
    uint32 setportmask=0, portmaskhi = 0, portmasklo = 0;
    uint32 next = 0xFFFFFFFF;
    uint32 idx;
 
    if(!SOC_IS_SBX_QE2000(unit)) {
        return CMD_USAGE;
    }
 
    if (ARG_CNT(args)) {
        parse_table_t pt;
        int ret_code;
 
        parse_table_init(0, &pt);
        parse_table_add(&pt, "a", PQ_INT, 0, &data_a, NULL);
        parse_table_add(&pt, "b", PQ_INT, 0, &data_b, NULL);
        parse_table_add(&pt, "knockout", PQ_INT, 0, &knockout, NULL);
        parse_table_add(&pt, "setportmask", PQ_INT, 0, &setportmask, NULL);
        parse_table_add(&pt, "portmaskhi", PQ_INT, 0, &portmaskhi, NULL);
        parse_table_add(&pt, "portmasklo", PQ_INT, 0, &portmasklo, NULL);
        parse_table_add(&pt, "next", PQ_INT, 0, &next, NULL);
        parse_table_add(&pt, "idx", PQ_DFL | PQ_INT, 0, &idx, NULL);
        
        if (!parseEndOk(args, &pt, &ret_code)) {
            return ret_code;
        }
 
    } else {
        return CMD_USAGE;
    }
 
    sbh = SOC_SBX_CONTROL(unit)->sbhdl;
    sbRv = hwQe2000MVTGet(sbh, &sbMvtEntry, idx, NULL, NULL, 0, 0, NULL);
    if(sbRv) {
        cli_out("MVTGet Failed:(%d)\n", sbRv);
        return CMD_FAIL;
    }

    if (data_a != ~0) {
        sbMvtEntry.ulMvtdA = data_a;
    }

    if (data_b != ~0) {
        sbMvtEntry.ulMvtdB = data_b;
    }

    if (knockout != -1)
      sbMvtEntry.bSourceKnockout = knockout;

    if (setportmask) {
      int j;
      for (j = 0; j < 32; j++) {
	sbMvtEntry.bPortEnable[j] = (portmasklo>>j) & 1;
      }
      for (j = 0; j < HW_QE2000_MAX_NUM_PORTS_K-32; j++) {
	sbMvtEntry.bPortEnable[32+j] = (portmaskhi >> j) & 1;
      }
    }

    if (next != 0xFFFFFFFF) {
      sbMvtEntry.ulNext = next;
    }

    sbRv = hwQe2000MVTSet(sbh, sbMvtEntry, idx, NULL, NULL, 0, 0, NULL);
    if (sbRv) {
        cli_out("MVTSet Failed: %d\n", sbRv);
    }

    cli_out("mvt set ok\n");

    return CMD_OK;
}

char cmd_sbx_qe2000_port_rate_egress_usage[] = "\n"
"PortRate <index>\n"
"  dumps a QE-2000 PortRate entry\n"
;

cmd_result_t
cmd_sbx_qe2000_port_rate_egress(int unit, args_t *args)
{
    char *lo;
    char *hi = NULL;
    char *range;
    int start_idx, end_idx;

    if(!SOC_IS_SBX_QE2000(unit)) {
        return CMD_USAGE;
    }

    if((lo = range = ARG_GET(args)) != NULL) {
        if((hi = strchr(range, '-')) != NULL) {
            hi++;
        } else {
            hi = lo;
        }
    }
    
    if(diag_parse_range(lo, hi, &start_idx, &end_idx, 0,  256)) {
        cli_out("Invalid range. Valid range is : 0-%d\n",  256);
        return CMD_FAIL;
    }

    return CMD_OK;
}

#endif /* BCM_QE2000_SUPPORT */
