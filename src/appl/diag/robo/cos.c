/*
 * $Id: cos.c,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#include <bcm/error.h>
#include <bcm/cosq.h>
#include <bcm/debug.h>

#include <bcm_int/robo/port.h>

static int robo_cos_first_setting_flag = 0;

/*
 * Manage classes of service
 */

cmd_result_t
cmd_robo_cos(int unit, args_t *a)
{
    int r;
    char *subcmd, *c;
    int fabric;
    bcm_cos_queue_t cosq;
    static int weights[BCM_COS_COUNT];
    static int tmp_weights[BCM_COS_COUNT];
    int delay = 0;
    int numq, mode = BCM_COSQ_STRICT;
    bcm_cos_t prio;
    static bcm_cos_t lastPrio = 0;
    parse_table_t pt;
    cmd_result_t ret_code;
    pbmp_t              pbmp;
    soc_port_t          p;
    int                 discard_ena, discard_cap_avg, gain, drop_prob;
    int                 discard_start, discard_end, discard_slope, discard_time;
    char *              discard_color;

    if (robo_cos_first_setting_flag == 0) {
        sal_memset(weights, 0, BCM_COS_COUNT * sizeof(int));
        sal_memset(tmp_weights, 0, BCM_COS_COUNT * sizeof(int));
    }

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    fabric = SOC_IS_XGS(unit) && !SOC_IS_XGS_SWITCH(unit);

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    c = ARG_CUR(a);

    if ((c) == NULL) {
        if (sal_strcasecmp(subcmd, "clear") == 0) {
            if ((r = bcm_cosq_init(unit)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "strict") == 0) {
            if ((r = bcm_cosq_sched_set(unit, BCM_COSQ_STRICT,
                        weights, 0)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }

    }

    if ((r = bcm_cosq_config_get(unit, &numq)) < 0) {
        goto bcm_err;
    }

    if (!fabric &&
        (r = bcm_cosq_sched_get(unit, &mode, weights, &delay)) < 0) {
        goto bcm_err;
    }

    if (sal_strcasecmp(subcmd, "port") == 0) {
        if ((subcmd = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }

        if (sal_strcasecmp(subcmd, "show") == 0) { 
            char *mode_name;
            int weight_max;

            BCM_PBMP_CLEAR(pbmp);
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                            (void *)(0), &pbmp, NULL);

            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }
            if (BCM_PBMP_IS_NULL(pbmp)) {
                cli_out("%s: ERROR: must specify valid port bitmap.\n",
                        ARG_CMD(a));
                return CMD_FAIL;
            } else {
                PBMP_ITER(pbmp, p) {
                    if (IS_CPU_PORT(unit, p)) {
                        cli_out("%s: ERROR: No support for CPU port.\n",
                                ARG_CMD(a));
                        return CMD_FAIL;
                    }

                    switch (mode) {
                        case BCM_COSQ_STRICT:
                            mode_name = "strict";
                            break;
                        case BCM_COSQ_WEIGHTED_ROUND_ROBIN:
                            mode_name = "weighted round-robin";
                            break;
                        case BCM_COSQ_DEFICIT_ROUND_ROBIN:
                            mode_name = "deficit round-robin";
                            break;
                        default:
                            mode_name = "?";
                            break;
                    }

                    if ((r = bcm_cosq_sched_weight_max_get(unit,
                                                           mode, &weight_max)) < 0) {
                        goto bcm_err;
                    }
                            
                    cli_out("\n  Port %s COS configuration:\n", SOC_PORT_NAME(unit, p));
                    cli_out(" ------------------------------\n");
                    cli_out("  Config (max queues): %d\n", numq);
                    cli_out("  Schedule mode: %s\n", mode_name);

                    if (mode != BCM_COSQ_STRICT) {
                        cli_out("  Weighting (in packets, max_weight = %d):\n", weight_max);

                        for (cosq = 0; cosq < numq; cosq++) {
                            if (!robo_cos_first_setting_flag) {
                                /* show init cos queues weights status */
                                cli_out("    COSQ %d = %d packets\n", cosq, weights[cosq]);
                            } else {
                                /* show "Strict Priority" if setting cos queue weight = 0 */
                                if (tmp_weights[cosq] == BCM_COSQ_WEIGHT_STRICT) {
                                    cli_out("    COSQ %d = Strict Priority\n", cosq);
                                } else {
                                    cli_out("    COSQ %d = %d %s\n", cosq, weights[cosq],
                                            (mode ==BCM_COSQ_DEFICIT_ROUND_ROBIN) ?
                                            "n-bytes(n:the unit of weight)" : "packets");
                                }
                            }
                        }
                    }

                    cli_out("  Priority to queue mappings:\n");

                    for (prio = 0; prio < 8; prio++) {
                        if ((r = bcm_cosq_port_mapping_get(unit, p, prio, &cosq)) < 0) {
                            goto bcm_err;
                        }
                        cli_out("    PRIO %d ==> COSQ %d\n", prio, cosq);
                    } 

                    /* Only BCM53280 on ROBO chips support more than 8 priorities */
                    for (prio = 8; prio < 16; prio++) {
                        if ((r = bcm_cosq_port_mapping_get(unit, p, prio, &cosq)) < 0) {
                            break;
                        }
                        cli_out("    PRIO %d ==> COSQ %d\n", prio, cosq);
                    }
                }
            }
            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "map") == 0) {
            BCM_PBMP_CLEAR(pbmp);
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                            (void *)(0), &pbmp, NULL);
            parse_table_add(&pt, "Pri",     PQ_DFL|PQ_INT,
                            (void *)(0), &lastPrio, NULL);
            parse_table_add(&pt, "Queue",   PQ_DFL|PQ_INT,
                            (void *)(0), &cosq, NULL);

            if ((c != NULL) && !parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }
            if (BCM_PBMP_IS_NULL(pbmp)) {
                cli_out("%s: ERROR: must specify valid port bitmap.\n",
                        ARG_CMD(a));
                return CMD_FAIL;
            } else {
                PBMP_ITER(pbmp, p) {
                    r = bcm_cosq_port_mapping_set(unit, p, lastPrio, cosq);
                    if (r < 0) {
                        cli_out("%s: ERROR: port %s: %s\n",
                                ARG_CMD(a),
                                SOC_PORT_NAME(unit, p),
                                bcm_errmsg(r));
                        return CMD_FAIL;
                    }
                }
            }
            return CMD_OK;
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "show") == 0) {
        char *mode_name;
        int weight_max;

        if (c != NULL) {
            return CMD_USAGE;
        }

        if (fabric) {
            cli_out("COS configuration:\n");
            cli_out("  Config (max queues): %d\n", numq);

            return CMD_OK;
        }

        switch (mode) {
            case BCM_COSQ_STRICT:
                mode_name = "strict";
                break;
            case BCM_COSQ_WEIGHTED_ROUND_ROBIN:
                mode_name = "weighted round-robin";
                break;
            case BCM_COSQ_DEFICIT_ROUND_ROBIN:
                mode_name = "deficit round-robin";
                break;
            default:
                mode_name = "?";
                break;
        }

        if ((r = bcm_cosq_sched_weight_max_get(unit,
                                               mode, &weight_max)) < 0) {
            goto bcm_err;
        }
                
        cli_out("COS configuration:\n");
        cli_out("  Config (max queues): %d\n", numq);
        cli_out("  Schedule mode: %s\n", mode_name);

        if (mode != BCM_COSQ_STRICT) {
            cli_out("  Weighting (in packets, max_weight = %d):\n", weight_max);

            for (cosq = 0; cosq < numq; cosq++) {
                if (!robo_cos_first_setting_flag) {
                    /* show init cos queues weights status */
                    cli_out("    COSQ %d = %d packets\n", cosq, weights[cosq]);
                } else {
                    /* show "Strict Priority" if setting cos queue weight = 0 */
                    if (tmp_weights[cosq] == BCM_COSQ_WEIGHT_STRICT) {
                        cli_out("    COSQ %d = Strict Priority\n", cosq);
                    } else {
                        cli_out("    COSQ %d = %d %s\n", cosq, weights[cosq],
                                (mode ==BCM_COSQ_DEFICIT_ROUND_ROBIN) ? 
                                "n-bytes(n:the unit of weight)" : "packets");
                    }
                }
            }
        }

        cli_out("  Priority to queue mappings:\n");

        for (prio = 0; prio < 8; prio++) {
            if ((r = bcm_cosq_mapping_get(unit, prio, &cosq)) < 0) {
                goto bcm_err;
            }
            cli_out("    PRIO %d ==> COSQ %d\n", prio, cosq);
        }

        /* Only BCM53280 on ROBO chips support more than 8 priorities */
        for (prio = 8; prio < 16; prio++) {
            if ((r = bcm_cosq_mapping_get(unit, prio, &cosq)) < 0) {
                break;
            }
            cli_out("    PRIO %d ==> COSQ %d\n", prio, cosq);
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "config") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Numcos", PQ_DFL|PQ_INT,
            (void *)( 0), &numq, NULL);

        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }

        if ((r = bcm_cosq_config_set(unit, numq)) < 0) {
            goto bcm_err;
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "map") == 0) {
        if ((r = bcm_cosq_mapping_get(unit, lastPrio, &cosq)) < 0) {
            goto bcm_err;
        }

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Pri", PQ_DFL|PQ_INT,
            (void *)(0), &lastPrio, NULL);
        parse_table_add(&pt, "Queue", PQ_DFL|PQ_INT,
            (void *)(0), &cosq, NULL);

        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }

        if ((r = bcm_cosq_mapping_set(unit, lastPrio, cosq)) < 0) {
            goto bcm_err;
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "weight") == 0 ||
        sal_strcasecmp(subcmd, "drr") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "W0",  PQ_DFL|PQ_INT,
            (void *)( 0), &weights[0], NULL);
        parse_table_add(&pt, "W1",  PQ_DFL|PQ_INT,
            (void *)( 0), &weights[1], NULL);
        parse_table_add(&pt, "W2",  PQ_DFL|PQ_INT,
            (void *)( 0), &weights[2], NULL);
        parse_table_add(&pt, "W3",  PQ_DFL|PQ_INT,
            (void *)( 0), &weights[3], NULL);
        parse_table_add(&pt, "W4",  PQ_DFL|PQ_INT,
            (void *)( 0), &weights[4], NULL);
        parse_table_add(&pt, "W5",  PQ_DFL|PQ_INT,
            (void *)( 0), &weights[5], NULL);
        parse_table_add(&pt, "W6",  PQ_DFL|PQ_INT,
            (void *)( 0), &weights[6], NULL);
        parse_table_add(&pt, "W7",  PQ_DFL|PQ_INT,
            (void *)( 0), &weights[7], NULL);

        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }

        if (sal_strcasecmp(subcmd, "drr") == 0) {
	     mode = BCM_COSQ_DEFICIT_ROUND_ROBIN;
	 } else {
            mode = BCM_COSQ_WEIGHTED_ROUND_ROBIN;
	 }

        if ((r = bcm_cosq_sched_set(unit, mode, weights, 0)) < 0) {
            goto bcm_err;
        }

        /* check first time to set cos queues weights status */
        if (!robo_cos_first_setting_flag) {
            robo_cos_first_setting_flag = 1;
        }
        /* record each time to set cos queues weights value */
        for (cosq = 0 ; cosq < numq ; cosq++) {
            tmp_weights[cosq] = weights[cosq];
        }
        
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "bandwidth") == 0) {
        uint32 kbits_sec_min, kbits_max_burst, bw_flags;

        BCM_PBMP_CLEAR(pbmp);
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                        (void *)(0), &pbmp, NULL);
	parse_table_add(&pt, "Queue", PQ_INT,
			(void *)( 0), &cosq, NULL);
	parse_table_add(&pt, "KbpsMIn", PQ_INT,
			(void *)( 0), &kbits_sec_min, NULL);
	parse_table_add(&pt, "KbitsMaxBurst", PQ_INT,
			(void *)( 0), &kbits_max_burst, NULL);
	parse_table_add(&pt, "Flags", PQ_INT,
			(void *)( 0), &bw_flags, NULL);

        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }

        if (BCM_PBMP_IS_NULL(pbmp)) {
            cli_out("%s ERROR: empty port bitmap\n", ARG_CMD(a));
            return CMD_FAIL;
        }

        PBMP_ITER(pbmp, p) {
	    if ((r = bcm_cosq_port_bandwidth_set(unit, p, cosq, 
                                                 kbits_sec_min,
                                                 kbits_max_burst,
                                                 bw_flags)) < 0) {
	        goto bcm_err;
	    }
        }

	return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "bandwidth_show") == 0) {
        uint32 kbits_sec_min, kbits_max_burst, bw_flags;

        if (c != NULL) {
	    return CMD_USAGE;
	}

        BCM_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));
        cli_out("  COSQ bandwith configuration:\n");

        cli_out("    port | q | KbpsMin  | KbitsMaxBurst  | Flags\n");

        PBMP_ITER(pbmp, p) {
            cli_out("    -----+---+----------+----------------+-------\n");
            /* Queue = 0 ~ numq for Non EAV queue  */
            for (cosq = 0; cosq < numq; cosq++) {
                /* There is no bw_flags for Queue 0 ~ numq */
                bw_flags = -1;
                if ((r = bcm_cosq_port_bandwidth_get(unit, p, cosq, 
                      &kbits_sec_min, &kbits_max_burst, &bw_flags)) == 0) {
                    cli_out("    %4s | %d | %8d | %14d | %6d\n",
                            SOC_PORT_NAME(unit, p), cosq, kbits_sec_min,
                            kbits_max_burst, bw_flags);
                }
            }
            if (soc_feature(unit, soc_feature_eav_support)) {
                /* Queue = 4 ~ 5 for EAV queue*/
                for (cosq = 4; cosq < 6; cosq++) {
                    if ((r = bcm_cosq_port_bandwidth_get(unit, p, cosq, 
                               &kbits_sec_min, &kbits_max_burst, &bw_flags)) == 0) {
                        cli_out("    %4s | %d | %8d | %14d | %6d\n",
                                SOC_PORT_NAME(unit, p), cosq, kbits_sec_min,
                                kbits_max_burst, bw_flags);
                    }
                }
            }
        }
	return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "dpcontrol") == 0) {
        int  dlf_dp_value, dlf_dp_en, xoff_dp_en;

        if ((r = bcm_cosq_control_get
                (unit, -1, -1, bcmCosqControlDpValueDlf, &dlf_dp_value)) < 0) {
            goto bcm_err;
        }  

        if ((r = bcm_cosq_control_get
                (unit, -1, -1, bcmCosqControlDpChangeDlf, &dlf_dp_en)) < 0) {
            goto bcm_err;
        }  

        if ((r = bcm_cosq_control_get
                (unit, -1, -1, bcmCosqControlDpChangeXoff, &xoff_dp_en)) < 0) {
            goto bcm_err;
        }  

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "DlfDpValue",  PQ_DFL|PQ_INT,
            (void *)( 0), &dlf_dp_value, NULL);
        parse_table_add(&pt, "DlfDpEn",  PQ_DFL|PQ_INT,
            (void *)( 0), &dlf_dp_en, NULL);
        parse_table_add(&pt, "XoffDpEn",  PQ_DFL|PQ_INT,
            (void *)( 0), &xoff_dp_en, NULL);

        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }

        if ((r = bcm_cosq_control_set
                (unit, -1, -1, bcmCosqControlDpValueDlf, 
                _BCM_COLOR_DECODING(unit, dlf_dp_value))) < 0) {
            goto bcm_err;
        }  

        if ((r = bcm_cosq_control_set
                (unit, -1, -1, bcmCosqControlDpChangeDlf, dlf_dp_en)) < 0) {
            goto bcm_err;
        }  

        if ((r = bcm_cosq_control_set
                (unit, -1, -1, bcmCosqControlDpChangeXoff, xoff_dp_en)) < 0) {
            goto bcm_err;
        }  

	return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "discard") == 0) {
        uint32 discard_flags = 0;

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Enable", PQ_BOOL,
                        0, &discard_ena, NULL);
        parse_table_add(&pt, "CapAvg", PQ_BOOL,
                        0, &discard_cap_avg, NULL);
        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }

        if (discard_ena) {
            discard_flags |= BCM_COSQ_DISCARD_ENABLE;
        }
        if (discard_cap_avg) {
            discard_flags |= BCM_COSQ_DISCARD_CAP_AVERAGE;
        }

	    if ((r = bcm_cosq_discard_set(unit, discard_flags)) < 0) {
	        goto bcm_err;
	    }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "discard_port") == 0) {
        uint32 color_flags = 0;
        bcm_gport_t sched_gport;

        BCM_PBMP_CLEAR(pbmp);
        discard_color = NULL;
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP|PQ_BCM,
                        (void *)(0), &pbmp, NULL);
	    parse_table_add(&pt, "Queue", PQ_INT,
			(void *)( 0), &cosq, NULL);
        parse_table_add(&pt, "Color", PQ_STRING, 
                        "green", &discard_color, NULL);
	    parse_table_add(&pt, "DropSTart", PQ_INT,
			(void *)( 0), &discard_start, NULL);
	    parse_table_add(&pt, "DropSLope", PQ_INT,
			(void *)( 0), &discard_slope, NULL);
	    parse_table_add(&pt, "AvgTime", PQ_INT,
			(void *)( 0), &discard_time, NULL);
        parse_table_add(&pt, "Gport", PQ_INT, 
                        (void *)BCM_GPORT_INVALID,
                        &sched_gport, 0);

        /* Parse remaining arguments */
        if (0 > parse_arg_eq(a, &pt)) {
            cli_out("%s: Error: Invalid option or malformed expression: %s\n",
                    ARG_CMD(a), ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        if (discard_color != NULL) {
            if (!strncmp(discard_color, "r", 1)) {
                color_flags = BCM_COSQ_DISCARD_COLOR_RED;
            } else if (!strncmp(discard_color, "y", 1)) {
                color_flags = BCM_COSQ_DISCARD_COLOR_YELLOW;
            } else if (!strncmp(discard_color, "g", 1)) {
                color_flags = BCM_COSQ_DISCARD_COLOR_GREEN;
            } else {
                cli_out("%s ERROR: Invalid color value\n", ARG_CMD(a));
                parse_arg_eq_done(&pt);
                return CMD_FAIL;
            }
        } else {
            color_flags = BCM_COSQ_DISCARD_COLOR_GREEN;
        }

        parse_arg_eq_done(&pt);

        if (sched_gport == BCM_GPORT_INVALID) {
            if (BCM_PBMP_IS_NULL(pbmp)) {
                cli_out("%s ERROR: empty port bitmap\n", ARG_CMD(a));
                return CMD_FAIL;
            }

            BCM_PBMP_ITER(pbmp, p) {
	            if ((r = bcm_cosq_discard_port_set(unit, p, cosq, color_flags,
                                                   discard_start, discard_slope,
                                                   discard_time)) < 0) {
                                                   cli_out("%s, %d\n",FUNCTION_NAME(),__LINE__);
	                goto bcm_err;
	            }
            }
        } else {
	        if ((r = bcm_cosq_discard_port_set(unit, sched_gport, cosq, 
                                               color_flags, discard_start, 
                                         discard_slope, discard_time)) < 0) {
                                         cli_out("%s, %d\n",FUNCTION_NAME(),__LINE__);
	            goto bcm_err;
	        }
        }

	    return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "discard_show") == 0) {
        uint32          discard_flags = 0;
        bcm_gport_t     gport = 0;
        bcm_cos_queue_t cosq = 0;
        bcm_cosq_gport_discard_t discard;
        uint64 val;

        if (ARG_CNT(a)) {
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "Gport", PQ_INT, (void *)( 0), &gport, 0);
	    parse_table_add(&pt, "Queue", PQ_INT, (void *)( 0), &cosq, NULL);

            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }

            if (gport != 0) {
                cli_out(" Discard configuration GPORT=%d (%08x), COSQ %d\n",
                        gport, gport, cosq);
    
                discard.flags = BCM_COSQ_DISCARD_COLOR_GREEN | BCM_COSQ_DISCARD_BYTES;
                if (bcm_cosq_gport_discard_get(unit, gport, cosq, &discard) == 0) {
                    cli_out("    GREEN  (bytes) : Min %d, Max %d, Gain %d, DropProb %d, Ena %d\n",
                            discard.min_thresh, discard.max_thresh, discard.gain,
                            discard.drop_probability,
                            (discard.flags & BCM_COSQ_DISCARD_ENABLE) ? 1 : 0);
                }
                discard.flags = BCM_COSQ_DISCARD_COLOR_YELLOW | BCM_COSQ_DISCARD_BYTES;
                if (bcm_cosq_gport_discard_get(unit, gport, cosq, &discard) == 0) {
                    cli_out("    YELLOW (bytes) : Min %d, Max %d, Gain %d, DropProb %d, Ena %d\n",
                            discard.min_thresh, discard.max_thresh, discard.gain,
                            discard.drop_probability,
                            (discard.flags & BCM_COSQ_DISCARD_ENABLE) ? 1 : 0);
                }
                discard.flags = BCM_COSQ_DISCARD_COLOR_RED | BCM_COSQ_DISCARD_BYTES;
                if (bcm_cosq_gport_discard_get(unit, gport, cosq, &discard) == 0) {
                    cli_out("    RED    (bytes) : Min %d, Max %d, Gain %d, DropProb %d, Ena %d\n",
                            discard.min_thresh, discard.max_thresh, discard.gain,
                            discard.drop_probability,
                            (discard.flags & BCM_COSQ_DISCARD_ENABLE) ? 1 : 0);
                }
                return CMD_OK;
            }
        }

        if ((r = bcm_cosq_discard_get(unit, &discard_flags)) < 0) {
            if (r != BCM_E_UNAVAIL) {
                goto bcm_err;
            }
        }

        cli_out("  Discard (WRED): discard %s, cap-average %s\n",
                (discard_flags & BCM_COSQ_DISCARD_ENABLE) ? 
                "enabled" : "disabled",
                (discard_flags & BCM_COSQ_DISCARD_CAP_AVERAGE) ? 
                "enabled" : "disabled");

        BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
        cli_out("  Discard (WRED) port configuration:\n");
        cli_out("         |   | gr     | gr | gr    | ye     |"
                " ye | ye    | re     | re | re\n");
        cli_out("    port | q | strt   | sl | time  | strt   |"
                " sl | time  | strt   | sl | time\n");
        BCM_PBMP_ITER(pbmp, p) {
            cli_out("    -----+---+--------+----+-------+--------+"
                    "----+-------+--------+----+-------\n");
            for (cosq = 0; cosq < numq; cosq++) {
                if ((r = bcm_cosq_discard_port_get(unit, p, cosq, 
                        BCM_COSQ_DISCARD_COLOR_GREEN,
                        &discard_start, &discard_slope,
                        &discard_time)) < 0) {
                    cli_out("    %4s | %d |  ----  | -- | ----- ",
                            BCM_PORT_NAME(unit, p), cosq);
                } else {
                    cli_out("    %4s | %d | %6d | %2d | %5d ",
                            BCM_PORT_NAME(unit, p), cosq, discard_start, 
                            discard_slope, discard_time);
                }
                if ((r = bcm_cosq_discard_port_get(unit, p, cosq, 
                        BCM_COSQ_DISCARD_COLOR_YELLOW,
                        &discard_start, &discard_slope,
                        &discard_time)) < 0) {
                    cli_out("|  ----  | -- | ----- ");
                } else {
                    cli_out("| %6d | %2d | %5d ", 
                            discard_start, discard_slope, discard_time);
                }
                if ((r = bcm_cosq_discard_port_get(unit, p, cosq, 
                        BCM_COSQ_DISCARD_COLOR_RED,
                        &discard_start, &discard_slope,
                        &discard_time)) < 0) {
                    cli_out("|  ----  | -- | ----- \n");
                } else {
                    cli_out("| %6d | %2d | %5d\n", 
                            discard_start, discard_slope, discard_time);
                }
            }
        }

        BCM_PBMP_ITER(pbmp, p) {
            /* Per-port Egress WRED drop counter */
            cli_out("     Discard drop counter for port=%d\n", p);
            BCM_GPORT_LOCAL_SET(gport, p);
            COMPILER_64_ZERO(val);
            if (bcm_cosq_gport_stat_get(unit,  gport, -1, 
                    bcmCosqGportDiscardDroppedPkts, &val) == 0) {
                cli_out("        drop   ( pkts) : %8u\n", COMPILER_64_LO(val));    
            }
            if (bcm_cosq_gport_stat_get(unit, gport, -1, 
                    bcmCosqGportDiscardDroppedBytes, &val) == 0) {
                cli_out("        drop   ( bytes): 0x%08x%08x\n", 
                        COMPILER_64_HI(val), COMPILER_64_LO(val));    
            }
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "discard_gport") == 0) {
        uint32 color_flags = 0;
        bcm_gport_t sched_gport;
        bcm_cosq_gport_discard_t discard;

        BCM_PBMP_CLEAR(pbmp);
        discard_color = NULL;
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "GPort", PQ_INT, 
                        (void *)BCM_GPORT_INVALID,
                        &sched_gport, 0);
	parse_table_add(&pt, "Queue", PQ_INT,
			(void *)( 0), &cosq, NULL);
        parse_table_add(&pt, "Color", PQ_STRING, 
                        "green", &discard_color, NULL);
	parse_table_add(&pt, "DropStart", PQ_INT,
			(void *)( 0), &discard_start, NULL);
	parse_table_add(&pt, "DropEnd", PQ_INT,
			(void *)( 0), &discard_end, NULL);
	parse_table_add(&pt, "DropProbability", PQ_INT,
			(void *)( 0), &drop_prob, NULL);
	parse_table_add(&pt, "GAin", PQ_INT,
			(void *)( 0), &gain, NULL);
        parse_table_add(&pt, "Enable", PQ_BOOL,
                        0, &discard_ena, NULL);
        parse_table_add(&pt, "CapAvg", PQ_BOOL,
                        0, &discard_cap_avg, NULL);

        /* Parse remaining arguments */
        if (0 > parse_arg_eq(a, &pt)) {
            cli_out("%s: Error: Invalid option or malformed expression: %s\n",
                    ARG_CMD(a), ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        if (discard_color != NULL) {
            if (!strncmp(discard_color, "r", 1)) {
                color_flags = BCM_COSQ_DISCARD_COLOR_RED;
            } else if (!strncmp(discard_color, "y", 1)) {
                color_flags = BCM_COSQ_DISCARD_COLOR_YELLOW;
            } else if (!strncmp(discard_color, "g", 1)) {
                color_flags = BCM_COSQ_DISCARD_COLOR_GREEN;
            } else {
                cli_out("%s ERROR: Invalid color value\n", ARG_CMD(a));
                parse_arg_eq_done(&pt);
                return CMD_FAIL;
            }
        } else {
            color_flags = BCM_COSQ_DISCARD_COLOR_GREEN;
        }
        if (discard_ena) {
            color_flags |= BCM_COSQ_DISCARD_ENABLE;
        }
        if (discard_cap_avg) {
            color_flags |= BCM_COSQ_DISCARD_CAP_AVERAGE;
        }

        parse_arg_eq_done(&pt);

        discard.flags = color_flags;
        discard.min_thresh = discard_start;
        discard.max_thresh = discard_end;
        discard.drop_probability = drop_prob;
        discard.gain = gain;
	if ((r = bcm_cosq_gport_discard_set(unit, sched_gport, cosq, &discard)) < 0) {
	    goto bcm_err;
	}

	return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "discard_default") == 0) {
        bcm_cosq_gport_discard_t discard;
        uint32 en_flags = 0;

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "DropStart", PQ_INT,
			(void *)( 0), &discard_start, NULL);
        parse_table_add(&pt, "DropEnd", PQ_INT,
			(void *)( 0), &discard_end, NULL);
        parse_table_add(&pt, "DropProbability", PQ_INT,
			(void *)( 0), &drop_prob, NULL);
        parse_table_add(&pt, "GAin", PQ_INT,
			(void *)( 0), &gain, NULL);
        parse_table_add(&pt, "Enable", PQ_BOOL,
                        0, &discard_ena, NULL);

        /* Parse remaining arguments */
        if (0 > parse_arg_eq(a, &pt)) {
            cli_out("%s: Error: Invalid option or malformed expression: %s\n",
                    ARG_CMD(a), ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        /* Default WRED configuration that is used for IFP action */
        en_flags = BCM_COSQ_DISCARD_IFP;
        if (discard_ena) {
            en_flags |= BCM_COSQ_DISCARD_ENABLE;
        }

        parse_arg_eq_done(&pt);

        discard.flags = en_flags;
        discard.min_thresh = discard_start;
        discard.max_thresh = discard_end;
        discard.drop_probability = drop_prob;
        discard.gain = gain;
	    if ((r = bcm_cosq_gport_discard_set(unit, -1, -1, &discard)) < 0) {
	        goto bcm_err;
	    }

	    return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "bypass_wred") == 0) {
        int enable = 0;

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Enable", PQ_BOOL,
                        0, &discard_ena, NULL);
        parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP|PQ_BCM,
                        (void *)(0), &pbmp, NULL);
        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }

        if (discard_ena) {
            enable = 1;
        }

        BCM_PBMP_ITER(pbmp, p) {
    	    if ((r = bcm_cosq_control_set(unit, p, -1, 
                    bcmCosqControlEgressWredDropCancel, enable)) < 0) {
    	        goto bcm_err;
    	    }
        }
        return CMD_OK;
    }

    return CMD_USAGE;

bcm_err:
    cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(r));
    return CMD_FAIL;
}


