/*
 * $Id: switchctl.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Switch Control commands
 */

#include <shared/bsl.h>

#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <appl/diag/dport.h>

#include <appl/diag/ea/ea.h>

#include <bcm/error.h>
#include <bcm/switch.h>

char cmd_ea_switch_control_usage[] =
    "\n\tSwitchControl show\n\t"
    "SwitchControl set [<name>=<value>]...\n";

cmd_result_t
cmd_ea_switch_control(int unit, args_t *a)
{
    char *subcmd = NULL;
    int rv = 0;
    cmd_result_t ret_code = 0;
    parse_table_t pt;
    int arg_bcmSwitchPonOamFailsafeState = -1;
    int arg_bcmSwitchEquipmentAlarmState = -1;
    int arg_bcmSwitchPowerAlarmState = -1;
    int arg_bcmSwitchBatteryMissingAlarmState = -1;
    int arg_bcmSwitchBatteryFailureAlarmState = -1;
    int arg_bcmSwitchBatteryVoltLowAlarmState = -1; 
    int arg_bcmSwitchPhysicalIntrusionAlarmState = -1;
    int arg_bcmSwitchSelfTestFailureAlarmState = -1; 
    int arg_bcmSwitchPonIfSwitchAlarmState = -1;
    int arg_bcmSwitchBatteryVoltLowAlarmReportThreshold = -1;
    int arg_bcmSwitchBatteryVoltLowAlarmClearThreshold = -1;
    
    bcm_switch_control_t type;
    int arg;
    
    
    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }
    /* Check valid device to operation on ...*/
    if (! sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }
      
    if (!sal_strcasecmp(subcmd, "show")) {
        type =  bcmSwitchPonOamFailsafeState;
        rv = bcm_switch_control_get(unit, type, &arg);
        if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("bcmSwitchPonOamFailsafeState = %d\n", arg);
        
        type =  bcmSwitchEquipmentAlarmState;
        rv = bcm_switch_control_get(unit, type, &arg);
        if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("bcmSwitchEquipmentAlarmState = %d\n", arg);
        
        
        type =  bcmSwitchPowerAlarmState;
        rv = bcm_switch_control_get(unit, type, &arg);
        if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("bcmSwitchPowerAlarmState = %d\n", arg);
        
        type =  bcmSwitchBatteryMissingAlarmState;
        rv = bcm_switch_control_get(unit, type, &arg);
        if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("bcmSwitchBatteryMissingAlarmState = %d\n", arg);
        
        type =  bcmSwitchBatteryFailureAlarmState;
        rv = bcm_switch_control_get(unit, type, &arg);
        if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("bcmSwitchBatteryFailureAlarmState = %d\n", arg);
        
        type =  bcmSwitchBatteryVoltLowAlarmState;
        rv = bcm_switch_control_get(unit, type, &arg);
        if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("bcmSwitchBatteryVoltLowAlarmState = %d\n", arg);
        
        type =  bcmSwitchPhysicalIntrusionAlarmState;
        rv = bcm_switch_control_get(unit, type, &arg);
        if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("bcmSwitchPhysicalIntrusionAlarmState = %d\n", arg);
        
        type =  bcmSwitchSelfTestFailureAlarmState;
        rv = bcm_switch_control_get(unit, type, &arg);
        if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("bcmSwitchSelfTestFailureAlarmState = %d\n", arg);
        
        type =  bcmSwitchPonIfSwitchAlarmState;
        rv = bcm_switch_control_get(unit, type, &arg);
        if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("bcmSwitchPonIfSwitchAlarmState = %d\n", arg);
        
        type =  bcmSwitchBatteryVoltLowAlarmReportThreshold;
        rv = bcm_switch_control_get(unit, type, &arg);
        if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("bcmSwitchBatteryVoltLowAlarmReportThreshold = %d\n", arg);
        
        type =  bcmSwitchBatteryVoltLowAlarmClearThreshold;
        rv = bcm_switch_control_get(unit, type, &arg);
        if (BCM_FAILURE(rv)) {
            cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("bcmSwitchBatteryVoltLowAlarmClearThreshold = %d\n", arg);
        
        return CMD_OK;
        
    } else if (!sal_strcasecmp(subcmd, "set")) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "bcmSwitchPonOamFailsafeState", PQ_DFL|PQ_INT,
                0, &arg_bcmSwitchPonOamFailsafeState,   NULL);
                
        parse_table_add(&pt, "bcmSwitchEquipmentAlarmState", PQ_DFL|PQ_INT,
                0, &arg_bcmSwitchEquipmentAlarmState,   NULL);
        parse_table_add(&pt, "bcmSwitchPowerAlarmState", PQ_DFL|PQ_INT,
                0, &arg_bcmSwitchPowerAlarmState,   NULL);
        parse_table_add(&pt, "bcmSwitchBatteryMissingAlarmState", PQ_DFL|PQ_INT,
                0, &arg_bcmSwitchBatteryMissingAlarmState,   NULL);
        parse_table_add(&pt, "bcmSwitchBatteryFailureAlarmState", PQ_DFL|PQ_INT,
                0, &arg_bcmSwitchBatteryFailureAlarmState,   NULL);
        parse_table_add(&pt, "bcmSwitchBatteryVoltLowAlarmState", PQ_DFL|PQ_INT,
                0, &arg_bcmSwitchBatteryVoltLowAlarmState,   NULL);
        parse_table_add(&pt, "bcmSwitchPhysicalIntrusionAlarmState", 
                PQ_DFL|PQ_INT,
                0, &arg_bcmSwitchPhysicalIntrusionAlarmState, NULL);
        parse_table_add(&pt, "bcmSwitchSelfTestFailureAlarmState", 
                PQ_DFL|PQ_INT,
                0, &arg_bcmSwitchSelfTestFailureAlarmState,   NULL);
        parse_table_add(&pt, "bcmSwitchPonIfSwitchAlarmState", PQ_DFL|PQ_INT,
                0, &arg_bcmSwitchPonIfSwitchAlarmState,   NULL); 
                
        parse_table_add(&pt, "bcmSwitchBatteryVoltLowAlarmReportThreshold", 
                PQ_DFL|PQ_INT,
                0, &arg_bcmSwitchBatteryVoltLowAlarmReportThreshold,   NULL); 
        parse_table_add(&pt, "bcmSwitchBatteryVoltLowAlarmClearThreshold", 
                PQ_DFL|PQ_INT,
                0, &arg_bcmSwitchBatteryVoltLowAlarmClearThreshold,   NULL);  
        
        if (ARG_CNT(a) < 1) {
            return CMD_USAGE;   
        }
        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }
        if (arg_bcmSwitchPonOamFailsafeState != -1) {
            rv = bcm_switch_control_set(unit, bcmSwitchPonOamFailsafeState,
                arg_bcmSwitchPonOamFailsafeState);
            if (BCM_FAILURE(rv)) {
                cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }
        }
        if (arg_bcmSwitchEquipmentAlarmState != -1) {
            rv = bcm_switch_control_set(unit, bcmSwitchEquipmentAlarmState,
                arg_bcmSwitchEquipmentAlarmState);
            if (BCM_FAILURE(rv)) {
                cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }
        }
        if (arg_bcmSwitchPowerAlarmState != -1) {
            rv = bcm_switch_control_set(unit, bcmSwitchPowerAlarmState,
                arg_bcmSwitchPowerAlarmState);
            if (BCM_FAILURE(rv)) {
                cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }
        }
        if (arg_bcmSwitchBatteryMissingAlarmState != -1) {
            rv = bcm_switch_control_set(unit, bcmSwitchBatteryMissingAlarmState,
                arg_bcmSwitchBatteryMissingAlarmState);
            if (BCM_FAILURE(rv)) {
                cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }
        }        
        if (arg_bcmSwitchBatteryFailureAlarmState != -1) {
            rv = bcm_switch_control_set(unit, bcmSwitchBatteryFailureAlarmState,
                arg_bcmSwitchBatteryFailureAlarmState);
            if (BCM_FAILURE(rv)) {
                cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }
        }
        if (arg_bcmSwitchBatteryVoltLowAlarmState != -1) {
            rv = bcm_switch_control_set(unit, bcmSwitchBatteryVoltLowAlarmState,
                arg_bcmSwitchBatteryVoltLowAlarmState);
            if (BCM_FAILURE(rv)) {
                cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }
        } 
        if (arg_bcmSwitchPhysicalIntrusionAlarmState != -1) {
            rv = bcm_switch_control_set(unit,
                    bcmSwitchPhysicalIntrusionAlarmState,
                    arg_bcmSwitchPhysicalIntrusionAlarmState);
            if (BCM_FAILURE(rv)) {
                cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }
        }
        if (arg_bcmSwitchSelfTestFailureAlarmState != -1) { 
            rv = bcm_switch_control_set(unit,
                    bcmSwitchSelfTestFailureAlarmState,
                    arg_bcmSwitchSelfTestFailureAlarmState);
            if (BCM_FAILURE(rv)) {
                cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }
        }
        if (arg_bcmSwitchPonIfSwitchAlarmState != -1) {
            rv = bcm_switch_control_set(unit,
                    bcmSwitchPonIfSwitchAlarmState,
                    arg_bcmSwitchPonIfSwitchAlarmState);
            if (BCM_FAILURE(rv)) {
                cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }
        }
        if (arg_bcmSwitchBatteryVoltLowAlarmReportThreshold != -1) {
            rv = bcm_switch_control_set(unit,
                    bcmSwitchBatteryVoltLowAlarmReportThreshold,
                    arg_bcmSwitchBatteryVoltLowAlarmReportThreshold);
            if (BCM_FAILURE(rv)) {
                cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }
        }
        if (arg_bcmSwitchBatteryVoltLowAlarmClearThreshold != -1) {
            rv = bcm_switch_control_set(unit,
                    bcmSwitchBatteryVoltLowAlarmClearThreshold,
                    arg_bcmSwitchBatteryVoltLowAlarmClearThreshold);
            if (BCM_FAILURE(rv)) {
                cli_out("%s ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }
        }      
        return CMD_OK;              
    } else {
        return CMD_USAGE;   
    }
    
	return CMD_OK;
}
