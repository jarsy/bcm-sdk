/*
 * $Id: techsupport.c Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    techsupport.c
 * Purpose: This file contains the generic infrastructure code
 *          for techsupport utility.
 *
 */

#include <appl/diag/parse.h>
#include <shared/bsl.h>
#include <bcm/error.h>
#include <appl/diag/shell.h>
#include <sal/core/time.h>
#include <appl/diag/techsupport.h>
#include <soc/drv.h>
#include <soc/ea/allenum.h>
#include <sal/appl/sal.h>
#include <bcm/types.h>

char command_techsupport_usage[] =
     "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "techsupport <option> [args...]\n"
#else
    "techsupport basic - collects basic config/setup information from switch\n\t"
    "techSupport <feature_name> [diag] [reg] "
#ifndef BCM_SW_STATE_DUMP_DISABLE
    "[mem] [list] [sw] [verbose]- collects feature specific debug information\n\t"
#else
    "[mem] [list] [verbose]- collects feature specific debug information\n\t"
#endif /* BCM_SW_STATE_DUMP_DISABLE */
    "When <feature_name> is "COMMAND_COS" - collects cos related debug information\n\t"
    "When <feature_name> is "COMMAND_EFP" - collects efp related debug information\n\t"
    "When <feature_name> is "COMMAND_IFP" - collects ifp related debug information\n\t"
    "When <feature_name> is "COMMAND_LOAD_BALANCE" - collects RTAG7,DLB,ECMP and "
    "trunk related debug information\n\t"
    "When <feature_name> is "COMMAND_L3UC" - collects L3 unicast related debug information\n\t"
    "When <feature_name> is "COMMAND_L3MC" - collects L3 multicast related debug information\n\t"
    "When <feature_name> is "COMMAND_MPLS" - collects mpls related debug information\n\t"
    "When <feature_name> is "COMMAND_MMU" - collects mmu related debug information\n\t"
    "When <feature_name> is "COMMAND_NIV" - collects niv related debug information\n\t"
    "When <feature_name> is "COMMAND_VFP" - collects vfp related debug information\n\t"
    "When <feature_name> is "COMMAND_RIOT" - collects riot related debug information\n\t"
    "When <feature_name> is "COMMAND_OAM" - collects oam related debug information\n\t"
    "When <feature_name> is "COMMAND_VXLAN" - collects vxlan related debug information\n\t"
    "When <feature_name> is "COMMAND_VLAN" - collects vlan related debug information\n\t"
    "When <feature_name> is "COMMAND_TCB" - collects tcb related debug information\n\t"
    "When <feature_name> is "COMMAND_DGM" - collects dgm related debug information\n\t"
    "When <feature_name> is "COMMAND_PSTAT" - collects pstat related debug information\n"
#endif /* COMPILER_STRING_CONST_LIMIT */
    ;
extern cmd_result_t command_techsupport(int unit, args_t *a);

mbcm_techsupport_t *mbcm_techsupport[BCM_MAX_NUM_UNITS];

extern techsupport_data_t techsupport_l3uc_trident2plus_data;
extern techsupport_data_t techsupport_l3mc_trident2plus_data;
extern techsupport_data_t techsupport_mmu_trident2plus_data;
extern techsupport_data_t techsupport_mpls_trident2plus_data;
extern techsupport_data_t techsupport_niv_trident2plus_data;
extern techsupport_data_t techsupport_riot_trident2plus_data;
extern techsupport_data_t techsupport_vxlan_trident2plus_data;
extern techsupport_data_t techsupport_vlan_trident2plus_data;
extern techsupport_data_t techsupport_ifp_trident2plus_data;
extern techsupport_data_t techsupport_vfp_trident2plus_data;
extern techsupport_data_t techsupport_efp_trident2plus_data;
extern techsupport_data_t techsupport_cos_trident2plus_data;
extern techsupport_data_t techsupport_loadbalance_trident2plus_data;
extern techsupport_data_t techsupport_oam_trident2plus_data;

/* Global structure that maintains different features for "Trident2plus" chipset */
mbcm_techsupport_t mbcm_trident2plus_techsupport[] = {
    {COMMAND_L3MC, &techsupport_l3mc_trident2plus_data},
    {COMMAND_L3UC, &techsupport_l3uc_trident2plus_data},
    {COMMAND_MMU, &techsupport_mmu_trident2plus_data},
    {COMMAND_MPLS, &techsupport_mpls_trident2plus_data},
    {COMMAND_NIV, &techsupport_niv_trident2plus_data},
    {COMMAND_RIOT, &techsupport_riot_trident2plus_data},
    {COMMAND_OAM, &techsupport_oam_trident2plus_data},
    {COMMAND_VXLAN, &techsupport_vxlan_trident2plus_data},
    {COMMAND_VLAN, &techsupport_vlan_trident2plus_data},
    {COMMAND_IFP, &techsupport_ifp_trident2plus_data},
    {COMMAND_VFP, &techsupport_vfp_trident2plus_data},
    {COMMAND_EFP, &techsupport_efp_trident2plus_data},
    {COMMAND_COS, &techsupport_cos_trident2plus_data},
    {COMMAND_LOAD_BALANCE, &techsupport_loadbalance_trident2plus_data},
    {NULL, NULL} /* Must be the last element in this structure */
  };

#ifdef BCM_TOMAHAWK_SUPPORT
extern techsupport_data_t techsupport_vxlan_tomahawk_data;
extern techsupport_data_t techsupport_ifp_tomahawk_data;
extern techsupport_data_t techsupport_vfp_tomahawk_data;
extern techsupport_data_t techsupport_efp_tomahawk_data;
extern techsupport_data_t techsupport_mmu_tomahawk_data;
extern techsupport_data_t techsupport_oam_tomahawk_data;
extern techsupport_data_t techsupport_vlan_tomahawk_data;
extern techsupport_data_t techsupport_l3uc_tomahawk_data;
extern techsupport_data_t techsupport_l3mc_tomahawk_data;
extern techsupport_data_t techsupport_mpls_tomahawk_data;
extern techsupport_data_t techsupport_niv_tomahawk_data;
extern techsupport_data_t techsupport_loadbalance_tomahawk_data;
extern techsupport_data_t techsupport_cos_tomahawk_data;

mbcm_techsupport_t mbcm_tomahawk_techsupport[] = {
    {COMMAND_VXLAN, &techsupport_vxlan_tomahawk_data},
    {COMMAND_MMU, &techsupport_mmu_tomahawk_data},
    {COMMAND_OAM, &techsupport_oam_tomahawk_data},
    {COMMAND_IFP, &techsupport_ifp_tomahawk_data},
    {COMMAND_VFP, &techsupport_vfp_tomahawk_data},
    {COMMAND_EFP, &techsupport_efp_tomahawk_data},
    {COMMAND_VLAN, &techsupport_vlan_tomahawk_data},
    {COMMAND_L3UC, &techsupport_l3uc_tomahawk_data},
    {COMMAND_L3MC, &techsupport_l3mc_tomahawk_data},
    {COMMAND_MPLS, &techsupport_mpls_tomahawk_data},
    {COMMAND_NIV, &techsupport_niv_tomahawk_data},
    {COMMAND_LOAD_BALANCE, &techsupport_loadbalance_tomahawk_data},
    {COMMAND_COS, &techsupport_cos_tomahawk_data},
    {NULL, NULL} /* Must be the last element in this structure */
  };
#endif

#ifdef BCM_TOMAHAWK2_SUPPORT
extern techsupport_data_t techsupport_efp_tomahawk2_data;
extern techsupport_data_t techsupport_tcb_tomahawk2_data;
extern techsupport_data_t techsupport_dgm_tomahawk2_data;
extern techsupport_data_t techsupport_pstat_tomahawk2_data;

mbcm_techsupport_t mbcm_tomahawk2_techsupport[] = {
    {COMMAND_IFP, &techsupport_ifp_tomahawk_data},
    {COMMAND_VFP, &techsupport_vfp_tomahawk_data},
    {COMMAND_EFP, &techsupport_efp_tomahawk2_data},
    {COMMAND_TCB, &techsupport_tcb_tomahawk2_data},
    {COMMAND_DGM, &techsupport_dgm_tomahawk2_data},
    {COMMAND_PSTAT, &techsupport_pstat_tomahawk2_data},
    {NULL, NULL} /* Must be the last element in this structure */
  };
#endif


#ifdef BCM_TRIDENT2_SUPPORT
extern techsupport_data_t techsupport_l3uc_trident2_data;
extern techsupport_data_t techsupport_l3mc_trident2_data;
extern techsupport_data_t techsupport_mmu_trident2_data;
extern techsupport_data_t techsupport_mpls_trident2_data;
extern techsupport_data_t techsupport_niv_trident2_data;
extern techsupport_data_t techsupport_vxlan_trident2_data;
extern techsupport_data_t techsupport_vlan_trident2_data;
extern techsupport_data_t techsupport_cos_trident2_data;
extern techsupport_data_t techsupport_loadbalance_trident2_data;
extern techsupport_data_t techsupport_efp_trident2_data;
extern techsupport_data_t techsupport_ifp_trident2_data;
extern techsupport_data_t techsupport_oam_trident2_data;

mbcm_techsupport_t mbcm_trident2_techsupport[] = {
    {COMMAND_L3MC, &techsupport_l3mc_trident2_data},
    {COMMAND_L3UC, &techsupport_l3uc_trident2_data},
    {COMMAND_MMU, &techsupport_mmu_trident2_data},
    {COMMAND_OAM, &techsupport_oam_trident2_data},
    {COMMAND_COS, &techsupport_cos_trident2_data},
    {COMMAND_LOAD_BALANCE, &techsupport_loadbalance_trident2_data},
    {COMMAND_MPLS, &techsupport_mpls_trident2_data},
    {COMMAND_NIV, &techsupport_niv_trident2_data},
    {COMMAND_VXLAN, &techsupport_vxlan_trident2_data},
    {COMMAND_VLAN, &techsupport_vlan_trident2_data},
    /*Intentionally using td2plus data set for td2
     * as there is no change in data for IFP/VFP*/
    {COMMAND_VFP, &techsupport_vfp_trident2plus_data},
    {COMMAND_IFP, &techsupport_ifp_trident2_data},
    {COMMAND_EFP, &techsupport_efp_trident2_data},
    {NULL, NULL} /* Must be the last element in this structure */
   };
#endif

static struct techsupport_cmd {
    const char *techsupport_cmd;
    int (*techsupport_func) (int unit, args_t *a,
                             techsupport_data_t *techsupport_feature_data);
} techsupport_cmds[] = {
    {COMMAND_BASIC, techsupport_basic},
    {COMMAND_L3MC, techsupport_l3mc},
    {COMMAND_L3UC, techsupport_l3uc},
    {COMMAND_MMU, techsupport_mmu},
    {COMMAND_MPLS, techsupport_mpls},
    {COMMAND_NIV, techsupport_niv},
    {COMMAND_RIOT, techsupport_riot},
    {COMMAND_VLAN, techsupport_vlan},
    {COMMAND_VXLAN, techsupport_vxlan},
    {COMMAND_IFP, techsupport_ifp},
    {COMMAND_VFP, techsupport_vfp},
    {COMMAND_EFP, techsupport_efp},
    {COMMAND_COS, techsupport_cos},
    {COMMAND_LOAD_BALANCE, techsupport_loadbalance},
    {COMMAND_OAM, techsupport_oam},
    {COMMAND_TCB, techsupport_tcb},
    {COMMAND_DGM, techsupport_dgm},
    {COMMAND_PSTAT, techsupport_pstat},
    {NULL, NULL } /* Must be the last element in this structure */
  };

/* Function:  techsupport_calc_and_print_time_taken
 * Purpose :  Calculate and print the take taken
 * Parameters: start_time - start time in seconds
 *             end_time -   end time in seconds
 *             str      - string.
 *             print_header_footer_string - 1 or 0
*/
void techsupport_calc_and_print_time_taken(sal_time_t start_time,
                        sal_time_t end_time, char *str,
                        int print_header_footer_string)
{
    unsigned long hrs, mins, secs;
    sal_time_t time_diff;

    if (end_time < start_time) {
      return ;
    }

    time_diff = end_time - start_time;
    secs = time_diff % 60;
    time_diff = time_diff / 60;
    mins = time_diff % 60;
    hrs = time_diff / 60;

    if ((0 != secs) || (0 != mins) || (0 != hrs)) {
        if (1 == print_header_footer_string) {
          cli_out("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
        }
        cli_out("\nTime taken to execute \"%s\""
                " is %02d:%02d:%02d (hh:mm:ss).\n", str, (int)hrs, (int)mins, (int)secs);
        if (1 == print_header_footer_string) {
          cli_out(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
        }
    }
    return;
}
/* Function:  techsupport_command_execute
 * Purpose :  Executes the specified "command" and prints the time
 *            taken to execute the specified "command".
 * Parameters: unit - Unit number
 *             command - pointer to command.
*/
void
techsupport_command_execute(int unit, char *command)
{
    int  diff = 0, diff1 = 0, j = 0;
    char str[MAX_STR_LEN] = {0};
    char *pattern1 = "Output of \"";
    char *pattern2 = "\" start";
    char *pattern3 = "\" end";
    sal_time_t  start_time, end_time;
    /* Before actually dumping the command output, head string is
     * printed. Example of head string is something like below.
     * >>>>>>>>>>>>>>>>>>>>>>Output of "config show" start>>>>>>>>>>>>>>>>>>>
     * The below logic froms the head string.
    */
    memset(str, 0, MAX_STR_LEN);
    diff = MAX_STR_LEN - (strlen(command) + strlen(pattern1) + strlen(pattern2));
    diff = (diff % 2) == 0  ?  diff : ( diff + 1 );

    for (j=0; j < (diff / 2); j++) {
        str[j]='>';
    }

    cli_out("\n%s%s%s%s%s\n\n", str, pattern1, command, pattern2,str);
    start_time = sal_time();
    sh_process_command(unit, command);
    end_time = sal_time();

    /* After dumping the command output, tail string is
     * printed. Example of tail string is something like below.
     * <<<<<<<<<<<<<<<<<<<<<<Output of "config show" end<<<<<<<<<<<<<<<<<<<<<<<
     * The below logic froms the tail string.
    */

    memset(str, 0, MAX_STR_LEN);
    diff1 = MAX_STR_LEN - (strlen(command) + strlen(pattern1) + strlen(pattern3));
    diff1 = (diff1 % 2) == 0  ?  diff1 : ( diff1 + 1 );

    for (j=0; j < (diff1 / 2); j++) {
        str[j]='<';
    }
    cli_out("\n%s%s%s%s%s\n", str, pattern1, command, pattern3, str);

    techsupport_calc_and_print_time_taken(start_time, end_time, command, 0);
}
/* Function:   techsupport_feature_process
 *  Purpose :  Executes the following
 *             1) diag shell comands(techsupport_feature_data->techsupport_data_diag_cmdlist[])
 *             2) dumps the memories(techsupport_feature_data->techsupport_data_mem_list[])
 *             3) dumps the registers(techsupport_feature_data->techsupport_data_reg_list[])
 * Parameters: unit - Unit number
 *             a - pointer to argument.
 *             techsupport_data_t -structure that maintains per chip per feature debug info.
 * Returns:    CMD_OK :done
 *             CMD_FAIL : INVALID INPUT
 */
int
techsupport_feature_process(int unit, args_t *a,
                          techsupport_data_t *techsupport_feature_data)
{
    int i=0;
    char *arg1;
    char command_str[MAX_STR_LEN] ={0}, command_options_flag = 0, dump_all = 1;
    char dont_execute = 0;
    /* By default, dump only the changes */
    command_options_flag |= DUMP_TABLE_CHANGED;

    /* Parse the options */
    arg1 = ARG_GET(a);
    for (;;) {
        if (arg1 != NULL && !sal_strcasecmp(arg1, VERBOSE)) {
            command_options_flag &= ~(DUMP_TABLE_CHANGED);
            arg1 = ARG_GET(a);
        } else if (arg1 != NULL && !sal_strcasecmp(arg1, REG)) {
            command_options_flag |= DUMP_REGISTERS;
            arg1 = ARG_GET(a);
            dump_all = 0;
        } else if (arg1 != NULL && !sal_strcasecmp(arg1, MEM)) {
            command_options_flag |= DUMP_MEMORIES;
            arg1 = ARG_GET(a);
            dump_all = 0;
        } else if (arg1 != NULL && !sal_strcasecmp(arg1, DIAG)) {
            command_options_flag |= DUMP_DIAG_CMDS;
            arg1 = ARG_GET(a);
            dump_all = 0;
#ifndef BCM_SW_STATE_DUMP_DISABLE
        } else if (arg1 != NULL && !sal_strcasecmp(arg1, SW)) {
            command_options_flag |= DUMP_SW;
            arg1 = ARG_GET(a);
            dump_all = 0;
#endif /* BCM_SW_STATE_DUMP_DISABLE */
        } else if (arg1 != NULL && !sal_strcasecmp(arg1, LIST)) {
            command_options_flag |= DUMP_LIST;
            arg1 = ARG_GET(a);
            /* if "list" option is specified, just list out the following
             * with out executing.
             * 1)diag commands
             * 2)Memory names
             * 3)Register names
             * 4)software dump commands
             */
            dont_execute = 1;
        }
        else {
            break;
        }
    }

    /* Displays the output of feature specific diag commands */
    if (dump_all == 1 || (command_options_flag & DUMP_DIAG_CMDS)) {
        if (1 == dont_execute) {
            cli_out("\nList of Diag commands:\n");
            cli_out(">>>>>>>>>>>>>>>>>>>>>>\n");
        }
        for(i = 0; techsupport_feature_data->techsupport_data_diag_cmdlist[i] != NULL; i++) {
            if (1 == dont_execute) {
                cli_out("%s\n", techsupport_feature_data->techsupport_data_diag_cmdlist[i]);
            } else {
                techsupport_command_execute(unit, techsupport_feature_data->techsupport_data_diag_cmdlist[i]);
            }
        }

        for(i = 0; techsupport_feature_data->techsupport_data_device_diag_cmdlist[i] != NULL; i++) {
            if (1 == dont_execute) {
                cli_out("%s\n", techsupport_feature_data->techsupport_data_device_diag_cmdlist[i]);
            } else {
                techsupport_command_execute(unit, techsupport_feature_data->techsupport_data_device_diag_cmdlist[i]);
            }

        }

    }

    /* Dumps feature specific memory tables */
    if (dump_all == 1 || (command_options_flag & DUMP_MEMORIES)) {
        if (1 == dont_execute) {
            cli_out("\nList of Memory Table names:\n");
            cli_out(">>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
        }
        for (i=0; techsupport_feature_data->techsupport_data_mem_list[i] != INVALIDm ; i++) {
            if (1 == dont_execute) {
                cli_out("%s\n", SOC_MEM_UFNAME(unit, techsupport_feature_data->techsupport_data_mem_list[i]));
            } else {
                sal_strncpy(command_str, DUMP, sizeof(DUMP));

                if (command_options_flag & DUMP_TABLE_CHANGED) {
                    sal_strncat(command_str, CHANGE, (MAX_STR_LEN - strlen(command_str) - 1));
                }
                sal_strncat(command_str,
                        SOC_MEM_UFNAME(unit, techsupport_feature_data->techsupport_data_mem_list[i]),
                        (MAX_STR_LEN - strlen(command_str) - 1));
                techsupport_command_execute(unit, command_str);
            }
        }
    }

#ifndef BCM_SW_STATE_DUMP_DISABLE
    /* Displays the output of sofware dump  diag commands */
    if (dump_all == 1 || (command_options_flag & DUMP_SW)) {
        if (1 == dont_execute) {
            cli_out("\nSoftware dump commands:\n");
            cli_out(">>>>>>>>>>>>>>>>>>>>>>\n");
        }
        for(i = 0; techsupport_feature_data->techsupport_data_sw_dump_cmdlist[i] != NULL; i++) {
            if (1 == dont_execute) {
                cli_out("%s\n", techsupport_feature_data->techsupport_data_sw_dump_cmdlist[i]);
            } else {
                techsupport_command_execute(unit, techsupport_feature_data->techsupport_data_sw_dump_cmdlist[i]);
            }
        }

        for(i = 0; techsupport_feature_data->techsupport_data_device_sw_dump_cmdlist[i] != NULL; i++) {
            if (1 == dont_execute) {
                cli_out("%s\n", techsupport_feature_data->techsupport_data_device_sw_dump_cmdlist[i]);
            } else {
                techsupport_command_execute(unit, techsupport_feature_data->techsupport_data_device_sw_dump_cmdlist[i]);
            }

        }

    }
#endif /* BCM_SW_STATE_DUMP_DISABLE */

    /* Dumps feature specific registers */
    if (dump_all == 1 || (command_options_flag & DUMP_REGISTERS)) {
        if (1 == dont_execute) {
            cli_out("\nList of Registers:\n");
            cli_out(">>>>>>>>>>>>>>>>>>\n");
        }
        for (i = 0; techsupport_feature_data->techsupport_data_reg_list[i].reg != INVALIDr; i++) {
            if (1 == dont_execute) {
                cli_out("%s\n", SOC_REG_NAME(unit, techsupport_feature_data->techsupport_data_reg_list[i].reg));
            } else {
                sal_strncpy(command_str, GET_REG, sizeof(GET_REG));
                if (command_options_flag & DUMP_TABLE_CHANGED){
                    sal_strncat(command_str, CHANGE, (MAX_STR_LEN - strlen(command_str) - 1));
                }
                sal_strncat(command_str,  SOC_REG_NAME(unit, techsupport_feature_data->techsupport_data_reg_list[i].reg),
                                      (MAX_STR_LEN - strlen(command_str) - 1));
                techsupport_command_execute(unit, command_str);
            }
        }
    }
    return CMD_OK;
}
/* Function:    command_techsupport
 * Purpose :    Displays all the debug info for a given subfeature.
 * Parameters:  unit - Unit number
 *              a - pointer to argument.
 * Returns:     CMD_OK :done
 *              CMD_FAIL : INVALID INPUT
 */
cmd_result_t
command_techsupport(int unit, args_t *a)
{
    int rc = CMD_OK;
    sal_time_t  start_time = 0, end_time = 0;
    char *feature_name = ARG_GET(a);
    char feature_name_supported_flag = 0;
    struct techsupport_cmd *command;
    techsupport_data_t *techsupport_feature_data;
    mbcm_techsupport_t *mbcm_techsupport_ptr;
    char str[MAX_STR_LEN] ={0};
    if (feature_name == NULL) {
      return CMD_USAGE;
    }

    if (sal_strcasecmp(feature_name, "basic") == 0) {
        start_time = sal_time();
        rc  = techsupport_basic(unit, a, NULL);
        end_time = sal_time();
    } else {
        if (NULL == mbcm_techsupport[unit]) {
            if (SOC_IS_TRIDENT2PLUS(unit)) {
                mbcm_techsupport[unit] = mbcm_trident2plus_techsupport;
        }
#ifdef BCM_TRIDENT2_SUPPORT
            else if (SOC_IS_TRIDENT2(unit)) {
                mbcm_techsupport[unit] = mbcm_trident2_techsupport;
            }
#endif
#ifdef BCM_TOMAHAWK2_SUPPORT
            else if (SOC_IS_TOMAHAWK2(unit)) {
            mbcm_techsupport[unit] = mbcm_tomahawk2_techsupport;
        }
#endif
#ifdef BCM_TOMAHAWK_SUPPORT
            else if (SOC_IS_TOMAHAWK(unit)) {
            mbcm_techsupport[unit] = mbcm_tomahawk_techsupport;
        }
#endif
        else {
                cli_out("command \"techSupport %s\" not supported on chip %s.\n", feature_name, SOC_UNIT_NAME(unit));
                return CMD_FAIL;
            }
        }

        mbcm_techsupport_ptr = mbcm_techsupport[unit];
        for (; mbcm_techsupport_ptr->techsupport_feature_name; mbcm_techsupport_ptr++) {
            if (sal_strcasecmp(feature_name, mbcm_techsupport_ptr->techsupport_feature_name) == 0) {
                techsupport_feature_data = mbcm_techsupport_ptr->techsupport_data;
                feature_name_supported_flag = 1;
                break;
            }
        }

        if (feature_name_supported_flag == 0) {
            cli_out("command \"techSupport %s\" not supported on chip %s.\n", feature_name, SOC_UNIT_NAME(unit));
            return CMD_FAIL;
        }

        for (command = techsupport_cmds; command->techsupport_cmd; command++) {
            if (sal_strcasecmp(feature_name, command->techsupport_cmd) == 0) {
                start_time = sal_time();
                rc = (command->techsupport_func(unit, a, techsupport_feature_data));
                end_time = sal_time();
                break;
            }
        }
    }
    sal_strncpy(str, TECHSUPPORT, sizeof(TECHSUPPORT));
    sal_strncat(str, feature_name, (MAX_STR_LEN - strlen(str) - 1));
    sal_strncat(str, " [options..]", (MAX_STR_LEN - strlen(str) - 1));
    techsupport_calc_and_print_time_taken(start_time, end_time, str, 1);
    return rc;
}
