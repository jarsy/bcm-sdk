/* 
 * $Id: diag_field.c,v 1.42 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        diag.c
 * Purpose:     Device diagnostics commands.
 *
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_APPL_SHELL
#define BSL_LOG_MODULE BSL_LS_APPL_SHELL

#include <shared/bsl.h>

#include <appl/diag/diag.h>
#include <appl/diag/diag_field.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/dpp/PPD/ppd_api_fp.h>
#include <bcm_int/common/debug.h>
#include <bcm_int/dpp/field_int.h>
#include <bcm_int/dpp/utils.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_dbal.h>
#include <soc/dpp/SAND/Utils/sand_sorted_list.h>
#include <appl/diag/sand/diag_sand_prt.h>
#include <appl/diag/sand/diag_sand_utils.h>
#include <sal/appl/sal.h>
#include <sal/appl/io.h>

/* { */
/******************************************************************** 
 *  Function handler: packet_diag_get (section fp)
 ********************************************************************/
cmd_result_t 
  cmd_ppd_api_fp_packet_diag_get(int unit, args_t* a)  
{   
  uint32 
    ret;   
  SOC_PPC_FP_PACKET_DIAG_INFO   
    *prm_info;
  uint32
    soc_sand_dev_id;
  int
    core;

  prm_info = sal_alloc(sizeof(SOC_PPC_FP_PACKET_DIAG_INFO), "cmd_ppd_api_fp_packet_diag_get.prm_info");
  if(!prm_info) {
      cli_out("Memory allocation failure\n");
      return CMD_FAIL;
  }
  soc_sand_dev_id = (unit);

  SOC_DPP_CORES_ITER(SOC_CORE_ALL, core) {

      SOC_PPC_FP_PACKET_DIAG_INFO_clear(prm_info);
      cli_out("======================================== core %d ========================================\n", core);
      /* Get parameters */

      /* Call function */
      ret = soc_ppd_fp_packet_diag_get(
              soc_sand_dev_id,
              core,
              prm_info
            );
      if (soc_sand_get_error_code_from_error_word(ret) != SOC_SAND_OK)
      {
          sal_free(prm_info);
          return CMD_FAIL;
      }

      SOC_PPC_FP_PACKET_DIAG_INFO_print(prm_info);
  }
  sal_free(prm_info);
  return CMD_OK; 
} 
/* { */
static
  cmd_result_t 
    diag_soc_sand_print_indices(int unit, args_t *a)  
{   
  cmd_result_t 
    ret ;   
  char
    *arg_mode ;
  SOC_SAND_SORTED_LIST_PTR
    sorted_list ;
  int
    val ;

  ret = CMD_OK ;
  /* Get parameters */ 
  arg_mode = ARG_GET(a) ;
  val = sal_ctoi(arg_mode,0) ;
  sorted_list = (SOC_SAND_SORTED_LIST_PTR)val ;
  soc_sand_print_indices(unit,sorted_list) ;
  return (ret) ; 
}
static
  cmd_result_t 
    diag_soc_sand_print_list(int unit, args_t *a)  
{   
  cmd_result_t 
    ret ;   
  char
    *arg_mode ;
  SOC_SAND_SORTED_LIST_PTR
    sorted_list ;
  int
    val ;

  ret = CMD_OK ;
  /* Get parameters */ 
  arg_mode = ARG_GET(a) ;
  val = sal_ctoi(arg_mode,0) ;
  sorted_list = (SOC_SAND_SORTED_LIST_PTR)val ;
  soc_sand_print_list(unit,sorted_list) ;
  return (ret) ; 
}
/* } */


/******************************************************************** 
 *  Function handler: dbs_action_info_show (section fp)
 ********************************************************************/

cmd_result_t 
  cmd_ppd_api_fp_dbs_action_info_show(int unit, args_t* a)  
{   
  uint32 
    ret;   

  /* Call function */
  ret = soc_ppd_fp_dbs_action_info_show(
          unit
        );
  if (soc_sand_get_error_code_from_error_word(ret) != SOC_SAND_OK) 
  {
      return CMD_FAIL; 
  } 
  
  return CMD_OK; 
}


/******************************************************************** 
 *  Function handler: action_info_show (section fp)
 ********************************************************************/

cmd_result_t 
  cmd_ppd_api_fp_action_info_show(int unit, args_t* a)  
{   
  uint32 
    ret;   

  /* Call function */
  ret = soc_ppd_fp_action_info_show(
          unit
        );
  if (soc_sand_get_error_code_from_error_word(ret) != SOC_SAND_OK) 
  {
      return CMD_FAIL; 
  } 
  
  return CMD_OK; 
}

/******************************************************************** 
 *  Function handler: print_all_fems_for_stage (section fp)
 *
 * Note:
 *   We use, below, _shr_ctoi() instead of the standard atoi() (sal_atoi())
 *   because, otherwise, compilation fails for '--kernel'
 ********************************************************************/

cmd_result_t 
  cmd_ppd_api_fp_print_all_fems_for_stage(int unit, args_t* a)  
{   
  uint32
    val,
    ret,
    stage,
    is_for_tm ;   
  char
    *arg_mode ;

  ret = CMD_USAGE ;
  arg_mode = ARG_GET(a) ;
  /*
   * First parameter is 'stage'. See SOC_PPC_FP_DATABASE_STAGE.
   */
  if (arg_mode) {
    stage = (uint32)_shr_ctoi(arg_mode) ;
  } else {
    cli_out("\r\n") ;
    cli_out("%s: Missing first parameter. Quit.\r\n",__FUNCTION__) ;
    cli_out("\r\n") ;
    goto exit ;
  }
  arg_mode = ARG_GET(a) ;
  /*
   * Second parameter 'is_for_tm':
   *   Set 1 for TM processing (packets with ITMH header)
   *   Set 0 for stacking or TDM processing (packets with FTMH header)
   */
  if (arg_mode) {
    is_for_tm = (uint32)_shr_ctoi(arg_mode) ;
  } else {
    cli_out("\r\n") ;
    cli_out("%s: Missing second parameter. Quit.\r\n",__FUNCTION__) ;
    cli_out("\r\n") ;
    goto exit ;
  }
  ret = CMD_FAIL ; 
  /* Call function */
  val = soc_ppd_fp_print_all_fems_for_stage(unit,stage,is_for_tm) ;
  if (soc_sand_get_error_code_from_error_word(val) != SOC_SAND_OK) 
  {
    goto exit ;
  } 
  ret = CMD_OK ; 
exit:
  return (ret) ;
}

/******************************************************************** 
 *  Function handler: print_fes_info_for_stage (section fp)
 *
 * Note:
 *   We use, below, _shr_ctoi() instead of the standard atoi() (sal_atoi())
 *   because, otherwise, compilation fails for '--kernel'
 ********************************************************************/

cmd_result_t 
  cmd_ppd_api_fp_print_fes_info_for_stage(int unit, args_t* a)  
{   
  uint32
    val,
    ret,
    stage,
    pmf_pgm_ndx ;   
  char
    *arg_mode ;

  ret = CMD_USAGE ;
  arg_mode = ARG_GET(a) ;
  /*
   * First parameter is 'stage'. See SOC_PPC_FP_DATABASE_STAGE.
   */
  if (arg_mode) {
    stage = (uint32)_shr_ctoi(arg_mode) ;
  } else {
    cli_out("\r\n") ;
    cli_out("%s: Missing first parameter. Quit.\r\n",__FUNCTION__) ;
    cli_out("\r\n") ;
    goto exit ;
  }
  /*
   * Second parameter is 'PMF program'. This is a programmable parameter.
   * See SOC_DPP_DEFS_MAX(NOF_INGRESS_PMF_PROGRAMS).
   */
  arg_mode = ARG_GET(a) ;
  if (arg_mode) {
    pmf_pgm_ndx = (uint32)_shr_ctoi(arg_mode) ;
  } else {
    cli_out("\r\n") ;
    cli_out("%s: Missing second parameter. Quit.\r\n",__FUNCTION__) ;
    cli_out("\r\n") ;
    goto exit ;
  }
  ret = CMD_FAIL ; 
  /* Call function */
  val = soc_ppd_fp_print_fes_info_for_stage(unit,stage,pmf_pgm_ndx) ;
  if (soc_sand_get_error_code_from_error_word(val) != SOC_SAND_OK) 
  {
    goto exit ;
  } 
  ret = CMD_OK ; 
exit:
  return (ret) ;
}

/******************************************************************** 
 *  Function handler: resource_diag_get (section fp)
 ********************************************************************/

cmd_result_t 
  cmd_ppd_api_fp_resource_diag_get(int unit, args_t* a)  
{   
  uint32 
    soc_sand_dev_id,
    mode,
    ret;   
  SOC_PPC_FP_RESOURCE_DIAG_INFO   
    *prm_info;
  char *arg_mode;

  prm_info = sal_alloc(sizeof(SOC_PPC_FP_RESOURCE_DIAG_INFO), "cmd_ppd_api_fp_resource_diag_get.prm_info");
  if(prm_info == NULL) {
    cli_out("Memory allocation failure\n");
    return CMD_FAIL;
  }
  
  SOC_PPC_FP_RESOURCE_DIAG_INFO_clear(prm_info);
  soc_sand_dev_id = (unit);

  /* Get parameters */ 
  arg_mode = ARG_GET(a);
  if (! arg_mode ) {
      mode = 0;
  } else if (! sal_strncasecmp(arg_mode, "1", strlen(arg_mode))) {
      mode = 1;
  } else if (! sal_strncasecmp(arg_mode, "2", strlen(arg_mode))) {
      mode = 2;
  } else if (! sal_strncasecmp(arg_mode, "3", strlen(arg_mode))) {
      mode = 3;
  } else {
      mode = 0;
  } 

  /* Call function */
  ret = soc_ppd_fp_resource_diag_get(
          soc_sand_dev_id,
          mode,
          prm_info
        );
  if (soc_sand_get_error_code_from_error_word(ret) != SOC_SAND_OK) 
  {
      sal_free(prm_info); 
      return CMD_FAIL; 
  } 

  SOC_PPC_FP_RESOURCE_DIAG_INFO_print(unit, prm_info);
  sal_free(prm_info);
  
  return CMD_OK; 
}


extern cmd_result_t _bcm_petra_field_test_qualify_set(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_warmboot);
extern cmd_result_t _bcm_petra_field_test_action_set(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_field_group(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
extern cmd_result_t _bcm_petra_field_test_field_group_2(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
extern cmd_result_t _bcm_petra_field_test_field_group_destroy(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_field_group_destroy_with_traffic(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_entry(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_entry_traffic(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_entry_traffic_perf(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_entry_priority(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
extern cmd_result_t _bcm_petra_field_test_entry_priority_2(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
extern cmd_result_t _bcm_petra_field_test_shared_bank(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_field_group_priority(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_field_group_destroy_with_traffic_and_de_fg(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_cascaded(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_presel(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_presel_set(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_field_group_presel(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_field_group_presel_1(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_data_qualifiers(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_data_qualifier_set(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_data_qualifiers_entry(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_data_qualifiers_entry_traffic(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_predefined_data_qualifiers_entry_traffic(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_field_group_direct_extraction(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_de_entry(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_de_entry_traffic(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_de_entry_traffic_large(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_field_group_direct_table(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_direct_table_entry(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_direct_table_entry_traffic(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_full_tcam(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_full_tcam_perf(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_full_tcam_diff_prio(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_full_tcam_diff_prio_perf(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_compress(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_itmh_field_group(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_itmh_field_group_traffic(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_itmh_parsing_test(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_itmh_parsing_test_pb(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_resend_last_packet(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
extern cmd_result_t _bcm_petra_field_test_user_header(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_watmboot);
STATIC cmd_result_t
cmd_diag_test(int unit, args_t* a)
{
    cmd_result_t res;
    cmd_result_t(*all_tests_func_ingress[])(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_warmboot) = {
        _bcm_petra_field_test_qualify_set,
        _bcm_petra_field_test_action_set,
        _bcm_petra_field_test_field_group,
        _bcm_petra_field_test_field_group_destroy,
        _bcm_petra_field_test_field_group_destroy_with_traffic,
        _bcm_petra_field_test_entry,
        _bcm_petra_field_test_entry_traffic,
        _bcm_petra_field_test_entry_traffic_perf,
        _bcm_petra_field_test_entry_priority,
        _bcm_petra_field_test_shared_bank,
        _bcm_petra_field_test_field_group_priority,
        _bcm_petra_field_test_cascaded,
        _bcm_petra_field_test_presel,
        _bcm_petra_field_test_presel_set,
        _bcm_petra_field_test_field_group_presel,
        _bcm_petra_field_test_data_qualifiers,
        _bcm_petra_field_test_data_qualifier_set,
        _bcm_petra_field_test_data_qualifiers_entry,
        _bcm_petra_field_test_data_qualifiers_entry_traffic,
        _bcm_petra_field_test_predefined_data_qualifiers_entry_traffic,
        _bcm_petra_field_test_field_group_direct_extraction,
        _bcm_petra_field_test_de_entry,
        _bcm_petra_field_test_de_entry_traffic,
        _bcm_petra_field_test_de_entry_traffic_large,
        _bcm_petra_field_test_field_group_direct_table,
        _bcm_petra_field_test_full_tcam,
        _bcm_petra_field_test_full_tcam_perf,
        _bcm_petra_field_test_full_tcam_diff_prio,
        _bcm_petra_field_test_full_tcam_diff_prio_perf,
        _bcm_petra_field_test_compress,
        _bcm_petra_field_test_itmh_field_group_traffic,
        _bcm_petra_field_test_itmh_parsing_test,
        _bcm_petra_field_test_itmh_parsing_test_pb,
        _bcm_petra_field_test_user_header
    };

    cmd_result_t(*all_tests_func_egress[])(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_warmboot) = {
        _bcm_petra_field_test_qualify_set,
        _bcm_petra_field_test_action_set,
        _bcm_petra_field_test_field_group,
        _bcm_petra_field_test_field_group_destroy,
        _bcm_petra_field_test_entry,
        _bcm_petra_field_test_entry_traffic,
        _bcm_petra_field_test_entry_traffic_perf,
        _bcm_petra_field_test_presel,
        _bcm_petra_field_test_presel_set,
        _bcm_petra_field_test_data_qualifiers,
        _bcm_petra_field_test_data_qualifier_set,
        _bcm_petra_field_test_data_qualifiers_entry,
        _bcm_petra_field_test_predefined_data_qualifiers_entry_traffic
    };
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    cmd_result_t(*all_tests_func_external[])(int unit, uint8 stage, uint32 x, uint32 mode, uint8 do_warmboot) = {
        _bcm_petra_field_test_qualify_set,
        _bcm_petra_field_test_action_set,
        _bcm_petra_field_test_field_group,
        _bcm_petra_field_test_field_group_2,
        _bcm_petra_field_test_entry,
        _bcm_petra_field_test_entry_traffic,
        _bcm_petra_field_test_entry_traffic_perf,
        _bcm_petra_field_test_entry_priority,
        _bcm_petra_field_test_entry_priority_2,
        _bcm_petra_field_test_data_qualifiers,
        _bcm_petra_field_test_data_qualifier_set,
        _bcm_petra_field_test_data_qualifiers_entry,
        _bcm_petra_field_test_data_qualifiers_entry_traffic,
        _bcm_petra_field_test_predefined_data_qualifiers_entry_traffic,
        _bcm_petra_field_test_full_tcam,
        _bcm_petra_field_test_full_tcam_perf,
        _bcm_petra_field_test_full_tcam_diff_prio,
        _bcm_petra_field_test_full_tcam_diff_prio_perf
    };
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */

    uint32 i;
    uint32 x, i_mode;
    uint8 i_stage;
    char *mode;
    char *stage;
    parse_table_t pt;
    char *option;
    uint32 do_warmboot = 0;
    int core=0;

    uint32 is_deterministic;
    uint32 srand_value;

    option = ARG_GET(a);
    parse_table_init(unit, &pt);

    parse_table_add(&pt, "x", PQ_INT, (void *) (0),
            &x, NULL);
    parse_table_add(&pt, "Mode", PQ_STRING, (void *) "FAST",
          &mode, NULL);
    parse_table_add(&pt, "Stage", PQ_STRING, (void *) "INGRESS",
          &stage, NULL);
    parse_table_add(&pt, "core", PQ_INT, 0,
          &core, NULL);
    parse_table_add(&pt, "is_deterministic", PQ_INT, (void *) (0),
          &is_deterministic, NULL);
    parse_table_add(&pt, "Srand", PQ_INT, (void *) (5),
          &srand_value, NULL);

#ifdef BCM_WARM_BOOT_SUPPORT
    parse_table_add(&pt, "Warmboot", PQ_INT, (void *) (0),
          &do_warmboot, NULL);
#endif
    if (parse_arg_eq(a, &pt) < 0) {
        cli_out("%s: Invalid option: %s\n",
                ARG_CMD(a), ARG_CUR(a));
        return CMD_USAGE;
    }

    if (ARG_CNT(a) != 0) {
        cli_out("%s: extra options starting with \"%s\"\n",
                ARG_CMD(a), ARG_CUR(a));
        return CMD_USAGE;
    }
    
    if(is_deterministic) {
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META_U(unit,
                             "The test is deterministic\n")));
        sal_srand(srand_value);
    } else {
        srand_value = sal_time();
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META_U(unit,
                             "The test is not deterministic. srand_value = %d.\n"), srand_value));
        sal_srand(srand_value);

    }
    
#ifndef NO_SAL_APPL
    if(! sal_strncasecmp(mode, "SUPER_FAST", strlen(mode)) ) {
        i_mode = 0;
    } else if(! sal_strncasecmp(mode, "FAST", strlen(mode)) ) {
        i_mode = 1;
    } else if(! sal_strncasecmp(mode, "MEDIUM", strlen(mode)) ) {
        i_mode = 2;
    } else if(! sal_strncasecmp(mode, "SLOW", strlen(mode)) ) {
        i_mode = 3;
    } else if(! sal_strncasecmp(mode, "SCAN", strlen(mode)) ) {
        i_mode = 4;
    } else {
        cli_out("%s: Invalid option: %s\n",
                ARG_CMD(a), ARG_CUR(a));
        return CMD_USAGE;
    }

    if(! sal_strncasecmp(stage, "INGRESS", strlen(stage)) ) {
        i_stage = 0;
    } else if(! sal_strncasecmp(stage, "EGRESS", strlen(stage)) ) {
        i_stage = 1;
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    } else if(! sal_strncasecmp(stage, "EXTERNAL", strlen(stage)) ) {
        i_stage = 2;
#endif /*defined(INCLUDE_KBP) && !defined(BCM_88030) */
    } else {
        cli_out("%s: Invalid option: %s\n",
                ARG_CMD(a), ARG_CUR(a));
        return CMD_USAGE;
    }
    if(!option) {
        return CMD_USAGE;
    } else if(! sal_strncasecmp(option, "all", strlen(option)) ){
        if(i_stage == 0) {
            /* ingress */
            for(i=0; i < sizeof(all_tests_func_ingress)/sizeof(all_tests_func_ingress[0]); ++i) {
                res = all_tests_func_ingress[i](unit, i_stage, x, i_mode, do_warmboot);
                if(res != CMD_OK) {
                    return res;
                }
            }
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
        } else if(i_stage == 2) {
            /* external */
            for(i=0; i < sizeof(all_tests_func_external)/sizeof(all_tests_func_external[0]); ++i) {
                res = all_tests_func_ingress[i](unit, i_stage, x, i_mode, do_warmboot);
                if(res != CMD_OK) {
                    return res;
                }
            }
#endif /*defined(INCLUDE_KBP) && !defined(BCM_88030) */
        } else {
            /* egress */
            for(i=0; i < sizeof(all_tests_func_egress)/sizeof(all_tests_func_egress[0]); ++i) {
                res = all_tests_func_egress[i](unit, i_stage, x, i_mode, do_warmboot);
                if(res != CMD_OK) {
                    return res;
                }
            }
        }            
    } else if(! sal_strncasecmp(option, "qset", strlen(option)) ){
        return _bcm_petra_field_test_qualify_set(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "aset", strlen(option)) ){
        return _bcm_petra_field_test_action_set(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "field_group", strlen(option)) ){
        return _bcm_petra_field_test_field_group(unit, i_stage, x, i_mode, do_warmboot);
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    } else if(! sal_strncasecmp(option, "field_group_2", strlen(option)) ){
        return _bcm_petra_field_test_field_group_2(unit, i_stage, x, i_mode, do_warmboot);
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
    } else if(! sal_strncasecmp(option, "field_group_destroy", strlen(option)) ){
        return _bcm_petra_field_test_field_group_destroy(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "field_group_destroy_traffic", strlen(option)) ){
        return _bcm_petra_field_test_field_group_destroy_with_traffic(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "field_group_destroy_traffic_de", strlen(option)) ){
        return _bcm_petra_field_test_field_group_destroy_with_traffic_and_de_fg(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "entry", strlen(option)) ){
        return _bcm_petra_field_test_entry(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "entry_traffic", strlen(option)) ){
        return _bcm_petra_field_test_entry_traffic(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "entry_traffic_perf", strlen(option)) ){
        return _bcm_petra_field_test_entry_traffic_perf(unit, i_stage, 0x01, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "entry_priority", strlen(option)) ){
        return _bcm_petra_field_test_entry_priority(unit, i_stage, x, i_mode, do_warmboot);
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    } else if(! sal_strncasecmp(option, "entry_priority_2", strlen(option)) ){
        return _bcm_petra_field_test_entry_priority_2(unit, i_stage, x, i_mode, do_warmboot);
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
    } else if(! sal_strncasecmp(option, "shared_bank", strlen(option)) ){
        return _bcm_petra_field_test_shared_bank(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "field_group_priority", strlen(option)) ){
        return _bcm_petra_field_test_field_group_priority(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "cascaded", strlen(option)) ){
        return _bcm_petra_field_test_cascaded(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "presel", strlen(option)) ){
        return _bcm_petra_field_test_presel(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "presel_set", strlen(option)) ){
        return _bcm_petra_field_test_presel_set(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "field_group_presel", strlen(option)) ){
        return _bcm_petra_field_test_field_group_presel(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "field_group_presel_1", strlen(option)) ){
        return _bcm_petra_field_test_field_group_presel_1(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "data_qualifiers", strlen(option)) ){
        return _bcm_petra_field_test_data_qualifiers(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "data_qualifier_set", strlen(option)) ){
        return _bcm_petra_field_test_data_qualifier_set(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "data_qualifiers_entry", strlen(option)) ){
        return _bcm_petra_field_test_data_qualifiers_entry(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "data_qualifiers_entry_traffic", strlen(option)) ){
        return _bcm_petra_field_test_data_qualifiers_entry_traffic(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "predefined_data_qualifiers_entry_traffic", strlen(option)) ){
        return _bcm_petra_field_test_predefined_data_qualifiers_entry_traffic(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "direct_extraction_field_group", strlen(option)) ){
        return _bcm_petra_field_test_field_group_direct_extraction(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "direct_extraction_entry", strlen(option)) ){
        return _bcm_petra_field_test_de_entry(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "direct_extraction_entry_traffic", strlen(option)) ){
        return _bcm_petra_field_test_de_entry_traffic(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "direct_extraction_entry_traffic_large", strlen(option)) ){
        return _bcm_petra_field_test_de_entry_traffic_large(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "direct_table_field_group", strlen(option)) ){
        return _bcm_petra_field_test_field_group_direct_table(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "direct_table_entry", strlen(option)) ){
        return _bcm_petra_field_test_direct_table_entry(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "direct_table_entry_traffic", strlen(option)) ){
        return _bcm_petra_field_test_direct_table_entry_traffic(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "full_tcam", strlen(option)) ){
        return _bcm_petra_field_test_full_tcam(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "full_tcam_perf", strlen(option)) ){
        return _bcm_petra_field_test_full_tcam_perf(unit, i_stage, 0x01, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "full_tcam_diff_prio", strlen(option)) ){
        return _bcm_petra_field_test_full_tcam_diff_prio(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "full_tcam_diff_prio_perf", strlen(option)) ){
        return _bcm_petra_field_test_full_tcam_diff_prio_perf(unit, i_stage, 0x01, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "compress", strlen(option)) ){
        return _bcm_petra_field_test_compress(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "itmh_field_group", strlen(option)) ){
        return _bcm_petra_field_test_itmh_field_group(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "itmh_field_group_traffic", strlen(option)) ){
        return _bcm_petra_field_test_itmh_field_group_traffic(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "itmh_parsing_test", strlen(option)) ){
        return _bcm_petra_field_test_itmh_parsing_test(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "itmh_parsing_test_pb", strlen(option)) ){
        return _bcm_petra_field_test_itmh_parsing_test_pb(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "user_header", strlen(option)) ){
        return _bcm_petra_field_test_user_header(unit, i_stage, x, i_mode, do_warmboot);
    } else if(! sal_strncasecmp(option, "resend_last_packet", strlen(option)) ){
        return _bcm_petra_field_test_resend_last_packet(unit, i_stage, x, i_mode, do_warmboot);
    } else {
        return CMD_USAGE;
    }
    return CMD_OK;
#else
     cli_out("Option parsing is not supported when NO_SAL_APPL is defined\n");
     return CMD_USAGE;
#endif
}

static void
cmd_dpp_field_get_qual_32bit(char *str_data, char *str_mask, int data_size, uint32 data, uint32 mask)
{
    uint32 number = 0;
    int i;

    for(i = 0; i < data_size; i++)
        number |= 1 << i;

    mask &= number;
    data &= number;

    if(mask == 0) {
        /* Qualifier fully masked */
        SET_EMPTY(str_data);
        SET_EMPTY(str_mask);
    } else if(mask == number) {
        /* Qualifier fully unmasked */
        sprintf(str_data, "%X", data);
        SET_EMPTY(str_mask);
    } else {
        /* Partial mask */
        sprintf(str_data, "%X", data);
        sprintf(str_mask, "%X", mask);
    }

    return;
}

static void
_dpp_field_get_qual_str(SOC_PPC_FP_QUAL_VAL *qual_val, char *str, int data_size)
{
    char str_data[36], str_mask[36];

    if(data_size <= 32) {
        cmd_dpp_field_get_qual_32bit(str_data, str_mask,
                                     data_size, qual_val->val.arr[0], qual_val->is_valid.arr[0]);
    } else if(data_size <= 64) {
        cmd_dpp_field_get_qual_32bit(str_data, str_mask,
                                     data_size - 32, qual_val->val.arr[1], qual_val->is_valid.arr[1]);
        cmd_dpp_field_get_qual_32bit(str_data + strlen(str_data), str_mask + strlen(str_mask),
                                     32, qual_val->val.arr[0], qual_val->is_valid.arr[0]);
    } else {
        cli_out("Qualifier size:%d\n", data_size);
    }

    if(ISEMPTY(str_mask))
        sprintf(str, "%s", str_data);
    else
        sprintf(str, "%s/%s", str_data, str_mask);

    return;
}

static void
_dpp_field_get_qual_list(SOC_PPC_FP_DATABASE_INFO *dbInfo_p, char **qual_str, int *qual_num_p)
{
    int i;
    int qual_num = 0;
    sal_memset(qual_str, 0, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX * sizeof(char *));
    for (i = 0; i < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; i++) {
        if (_BCM_DPP_FIELD_PPD_QUAL_VALID(dbInfo_p->qual_types[i])) {
            qual_str[qual_num] = (char *)SOC_PPC_FP_QUAL_TYPE_to_string(dbInfo_p->qual_types[i]);
            qual_num++;
        }
    }

    *qual_num_p = qual_num;
    return;
}

static void
_dpp_field_get_action_list(SOC_PPC_FP_DATABASE_INFO *dbInfo_p, char **action_str, int *action_num_p)
{
    int i;
    int action_num = 0;
    sal_memset(action_str, 0, SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX * sizeof(char *));
    for (i = 0; i < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; i++) {
        if (_BCM_DPP_FIELD_PPD_ACTION_VALID(dbInfo_p->action_types[i])) {
            action_str[action_num] = (char *)SOC_PPC_FP_ACTION_TYPE_to_string(dbInfo_p->action_types[i]);
            action_num++;
        }
    }

    *action_num_p = action_num;
    return;
}

static void _dpp_get_range_flags_str(int flags, char *flags_str)
{
    SET_EMPTY(flags_str);

    if(flags & BCM_FIELD_RANGE_TCP) {
        strcat(flags_str, "TCP");
    }
    if(flags & BCM_FIELD_RANGE_UDP) {
        strcat(flags_str, " UDP");
    }
    if(flags & BCM_FIELD_RANGE_SRCPORT) {
        strcat(flags_str, " SrcPort");
    }
    if(flags & BCM_FIELD_RANGE_DSTPORT) {
        strcat(flags_str, " DstPort");
    }
    if(flags & BCM_FIELD_RANGE_PACKET_LENGTH) {
        strcat(flags_str, " PacketLength");
    }
}

static cmd_result_t
cmd_dpp_field_ranges_dump(int unit)
{
    bcm_field_range_t range;
    int index, actual, offset;
    int result;
    uint32 flags[_BCM_DPP_FIELD_RANGE_CHAIN_MAX];
    uint32 rangeMin[_BCM_DPP_FIELD_RANGE_CHAIN_MAX];
    uint32 rangeMax[_BCM_DPP_FIELD_RANGE_CHAIN_MAX];
    char flags_str[40];
    bcm_dpp_field_info_OLD_t *unitData;
    const _bcm_dpp_field_device_range_info_t *ranges;

    PRT_INIT_VARS;
    SHR_FUNC_INIT_VARS(unit);

    unitData = _bcm_dpp_field_unit_info[unit];
    ranges = unitData->devInfo->ranges;

    PRT_TITLE_SET("Range Dump");

    PRT_COLUMN_ADD("Qualifier");
    PRT_COLUMN_ADD("Range");
    PRT_COLUMN_ADD("Flags");
    PRT_COLUMN_ADD("Min");
    PRT_COLUMN_ADD("Max");

    for (index = 0; ranges[index].qualifier < bcmFieldQualifyCount; index++) {
        for (range = ranges[index].rangeBase; range < ranges[index].rangeBase + ranges[index].count; range++) {
            result = _bcm_dpp_field_range_get(unitData, range, _BCM_DPP_FIELD_RANGE_CHAIN_MAX,
                                              &(flags[0]), &(rangeMin[0]), &(rangeMax[0]), &actual);
            if (BCM_E_NONE == result) {
                PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                PRT_CELL_SET("%s(%d)", _bcm_dpp_field_qual_name[ranges[index].qualifier], ranges[index].qualifier);
                PRT_CELL_SET("%d", range);
                for (offset = 0; offset < actual; offset++) {
                    if(offset) {
                        PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                        PRT_CELL_SKIP(2);
                    }

                    _dpp_get_range_flags_str(flags[offset], flags_str);
                    PRT_CELL_SET("%s", flags_str);
                    PRT_CELL_SET("%d", rangeMin[offset]);
                    PRT_CELL_SET("%d", rangeMax[offset]);
                }
            }
        } /* for (all ranges of this type) */
    } /* for (all types of ranges) */

    /* Coverity: We are not going to change the macros to aviod such defects */
    /* coverity[dead_error_line]*/
    PRT_COMMIT;
exit:
    return SHR_ERR_TO_SHELL;
}

static cmd_result_t
cmd_dpp_field_table_list(int unit)
{
    uint32 soc_sandResult;
    int result;
    uint32 priority, key_size = 0, entry_size;
    SOC_PPC_FP_DATABASE_INFO    dbInfo;
    SOC_PPC_FP_DATABASE_STAGE   stage;
    ARAD_FP_ENTRY               fp_entry;

    int i;
    char *qual_str[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int qual_num;
    char *action_str[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX];
    int action_num;
    int line_num;
    int db_id, tcam_db_id;

    PRT_INIT_VARS;
    SHR_FUNC_INIT_VARS(unit);

    PRT_TITLE_SET("Table List");

    PRT_COLUMN_ADD("ID");
    PRT_COLUMN_ADD("Type");
    PRT_COLUMN_ADD("Stage");
    PRT_COLUMN_ADD("Priority");
    PRT_COLUMN_ADD("Key Size");
    PRT_COLUMN_ADD("NOF");
    PRT_COLUMN_ADD("Qualifiers");
    PRT_COLUMN_ADD("Actions");

    for (db_id = 0; db_id < SOC_PPC_FP_NOF_DBS; db_id++) {
        sal_memset(&dbInfo, 0, sizeof(SOC_PPC_FP_DATABASE_INFO));
        soc_sandResult = soc_ppd_fp_database_get(unit, db_id, &dbInfo);
        result = handle_sand_result(soc_sandResult);
        if (BCM_E_NONE != result) {
            LOG_ERROR(BSL_LS_APPL_SHELL, (BSL_META_U(unit, "DB %3d: unable to read %d (%s)\n"),
                      db_id, result, _SHR_ERRMSG(result)));
            continue;
        }
        if (SOC_PPC_NOF_FP_DATABASE_TYPES == dbInfo.db_type) {
            continue;
        }
        else if (SOC_PPC_NOF_FP_DATABASE_TYPES < dbInfo.db_type) {
            LOG_ERROR(BSL_LS_APPL_SHELL, (BSL_META_U(unit,"DB %3d: unexpected type %d\n"),
                      db_id, dbInfo.db_type));
            continue;
        }

        soc_sandResult = arad_pp_fp_db_stage_get(unit, db_id, &stage);
        result = handle_sand_result(soc_sandResult);
        if (BCM_E_NONE != result) {
            LOG_ERROR(BSL_LS_APPL_SHELL, (BSL_META_U(unit, "DB %3d: unable to get stage %d (%s)\n"),
                      db_id, result, _SHR_ERRMSG(result)));
            continue;
        }

        sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_entries.get(unit, stage, db_id, &fp_entry);
        sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.prio.get(unit, stage, db_id, &priority);

        /* Key size varies according to DB type */
        if(dbInfo.db_type == SOC_PPC_FP_DB_TYPE_TCAM || dbInfo.db_type == SOC_PPC_FP_DB_TYPE_EGRESS) {
            tcam_db_id = ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id);
            sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(unit,tcam_db_id, &entry_size);
            key_size = ARAD_SW_DB_ENTRY_SIZE_ID_TO_BITS(entry_size);
        } else if(dbInfo.db_type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE) {
            key_size = (dbInfo.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS) ? 19 : SOC_DPP_DEFS_GET(unit, tcam_big_bank_key_nof_bits);
        } else if(dbInfo.db_type == SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION) {
            key_size = 32;
        }

        _dpp_field_get_qual_list(&dbInfo, qual_str, &qual_num);
        _dpp_field_get_action_list(&dbInfo, action_str, &action_num);
        line_num = action_num > qual_num ? action_num : qual_num;
        for(i = 0; i < line_num; i++) {
            if(i == line_num - 1) {
                PRT_ROW_ADD(PRT_ROW_SEP_UNDERSCORE);
            }
            else {
                PRT_ROW_ADD(PRT_ROW_SEP_NONE);
            }
            if(i == 0) {
                PRT_CELL_SET("%d", db_id);
                PRT_CELL_SET("%s", SOC_PPC_FP_DATABASE_TYPE_to_string(dbInfo.db_type));
                PRT_CELL_SET("%s", SOC_PPC_FP_DATABASE_STAGE_to_string(stage));
                PRT_CELL_SET("%d", priority);
                PRT_CELL_SET("%d", key_size);
                PRT_CELL_SET("%d", fp_entry.nof_db_entries);
            }
            else {
                PRT_CELL_SKIP(6);
            }

            if(qual_str[i] != NULL)
            {
                PRT_CELL_SET("%s", qual_str[i]);
            }
            else
            {
                PRT_CELL_SKIP(1);
            }
            if(action_str[i] != NULL)
            {
                PRT_CELL_SET("%s", action_str[i]);
            }
            else
            {
                PRT_CELL_SKIP(1);
            }
        }
    }

    /* Coverity: We are not going to change the macros to aviod such defects */
    /* coverity[dead_error_line]*/
    PRT_COMMIT;
exit:
    return SHR_ERR_TO_SHELL;
}

static cmd_result_t
cmd_dpp_field_table_dump(int unit, int table_id, int start_entry_id, int output_num)
{
    uint32 soc_sandResult;
    uint32 soc_sandOffset;
    uint8 okay;
    SOC_PPC_FP_DATABASE_INFO    dbInfo;
    SOC_PPC_FP_DATABASE_STAGE   stage;
    int i, k;
    int count;
    int result;
    uint32 qual_size[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    char *qual_str[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int qual_num;
    char *action_str[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX];
    int action_num;
    char qual_val_str[36];
    SOC_PPC_FP_DIR_EXTR_ENTRY_INFO *entInfoDe = NULL;
    SOC_PPC_FP_ENTRY_INFO *entInfoTc = NULL;

    _bcm_dpp_field_ent_idx_t entryTcLimit;
    _bcm_dpp_field_ent_idx_t entryDeLimit;

    PRT_INIT_VARS;
    SHR_FUNC_INIT_VARS(unit);

    FIELD_ACCESS.entryTcLimit.get(unit, &entryTcLimit);
    FIELD_ACCESS.entryDeLimit.get(unit, &entryDeLimit);

    soc_sandResult = arad_pp_fp_db_stage_get(unit, table_id, &stage);
    result = handle_sand_result(soc_sandResult);
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_APPL_SHELL, (BSL_META_U(unit, "DB %3d: unable to get stage %d (%s)\n"),
                  table_id, result, _SHR_ERRMSG(result)));
        return CMD_FAIL;
    }

    sal_memset(&dbInfo, 0, sizeof(SOC_PPC_FP_DATABASE_INFO));
    soc_sandResult = soc_ppd_fp_database_get(unit, table_id, &dbInfo);
    result = handle_sand_result(soc_sandResult);
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_APPL_SHELL, (BSL_META_U(unit, "DB %3d: unable to read %d (%s)\n"),
                  table_id, result, _SHR_ERRMSG(result)));
        return CMD_FAIL;
    }

    if (SOC_PPC_NOF_FP_DATABASE_TYPES == dbInfo.db_type) {
        return CMD_FAIL;
    }
    else if (SOC_PPC_NOF_FP_DATABASE_TYPES < dbInfo.db_type) {
        LOG_ERROR(BSL_LS_APPL_SHELL, (BSL_META_U(unit,"DB %3d: unexpected type %d\n"),
                  table_id, dbInfo.db_type));
        return CMD_FAIL;
    }

    _dpp_field_get_qual_list(&dbInfo, qual_str, &qual_num);
    _dpp_field_get_action_list(&dbInfo, action_str, &action_num);

    PRT_TITLE_SET("Show Table:%d", table_id);

    PRT_COLUMN_ADD("ID");
    PRT_COLUMN_ADD("Qualifiers");
    for(i = 0; i < qual_num - 1; i++)
        PRT_COLUMN_ADD("");
    PRT_COLUMN_ADD("");
    PRT_COLUMN_ADD("Pr");
    PRT_COLUMN_ADD("");
    PRT_COLUMN_ADD("Actions");
    for(i = 0; i < action_num - 1; i++)
        PRT_COLUMN_ADD("");

    PRT_ROW_ADD(PRT_ROW_SEP_EQUAL);

    /*
     * First line of data will be actual header, but we have already assigned the right number of columns above
     */
    PRT_CELL_SET("%d", table_id);
    for (i = 0; i < qual_num; i++) {
        PRT_CELL_SET("%s", qual_str[i]);
    }

    PRT_CELL_SKIP(1);
    PRT_CELL_SKIP(1); /* for priority */
    PRT_CELL_SKIP(1);
    for (i = 0; i < action_num; i++) {
        PRT_CELL_SET("%s", action_str[i]);
    }

    for (i = 0; i < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; i++) {
        if (_BCM_DPP_FIELD_PPD_QUAL_VALID(dbInfo.qual_types[i])) {
            arad_pp_fp_key_length_get_unsafe(unit, stage, dbInfo.qual_types[i], 0 , &qual_size[i]);
        }
    }

    if ((SOC_PPC_FP_DB_TYPE_TCAM == dbInfo.db_type) ||
        (SOC_PPC_FP_DB_TYPE_DIRECT_TABLE == dbInfo.db_type) ||
        (SOC_PPC_FP_DB_TYPE_FLP == dbInfo.db_type) ||
        (SOC_PPC_FP_DB_TYPE_EGRESS == dbInfo.db_type)) {
        if(start_entry_id >= entryTcLimit) {
            PRT_FREE;
            SHR_ERR_EXIT(_SHR_E_INTERNAL,"Entry ID exceeded entryTcLimit:%d\n", entryTcLimit);
        }

        if(output_num <= 0)
            count = entryTcLimit;
        else
            count = output_num;

        if((entInfoTc = sal_alloc(sizeof(SOC_PPC_FP_ENTRY_INFO), "TCAM info")) == NULL) {
            SHR_ERR_EXIT(_SHR_E_MEMORY, "Failed to allocate SOC_PPC_FP_ENTRY_INFO\n");
        }

        for (soc_sandOffset = start_entry_id; soc_sandOffset < entryTcLimit; soc_sandOffset++) {
            sal_memset(entInfoTc, 0, sizeof(SOC_PPC_FP_ENTRY_INFO));
            soc_sandResult = soc_ppd_fp_entry_get(unit,
                                                  table_id,
                                                  soc_sandOffset,
                                                  &okay,
                                                  entInfoTc);
            result = handle_sand_result(soc_sandResult);
            if ((BCM_E_NONE == result) && okay) {
                PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                PRT_CELL_SET("%d", soc_sandOffset);
                for (i = 0; i < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; i++) {
                    if (_BCM_DPP_FIELD_PPD_QUAL_VALID(entInfoTc->qual_vals[i].type)) {
                        _dpp_field_get_qual_str(&entInfoTc->qual_vals[i], qual_val_str, qual_size[i]);
                        PRT_CELL_SET("%s", qual_val_str);
                    } else if (_BCM_DPP_FIELD_PPD_QUAL_VALID(dbInfo.qual_types[i])) {
                        PRT_CELL_SKIP(1);
                    }
                }
                PRT_CELL_SKIP(1);
                PRT_CELL_SET("%d", entInfoTc->priority);
                PRT_CELL_SKIP(1);
                for (i = 0; i < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; i++) {
                    if (_BCM_DPP_FIELD_PPD_ACTION_VALID(entInfoTc->actions[i].type)) {
                        PRT_CELL_SET("%X", entInfoTc->actions[i].val);
                    }
                }
                if(!(--count))
                    break;
            } /* if ((BCM_E_NONE == result) && okay) */
        } /* for (soc_sandOffset = 0; soc_sandOffset < 4K; soc_sandOffset++) */
    } else if (SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION == dbInfo.db_type) {
        if(start_entry_id >= entryDeLimit) {
            PRT_FREE;
            SHR_ERR_EXIT(_SHR_E_INTERNAL,"Entry ID exceeded entryDeLimit:%d\n", entryDeLimit);
        }

        if(output_num <= 0)
            count = entryDeLimit;
        else
            count = output_num;

        if((entInfoDe = sal_alloc(sizeof(SOC_PPC_FP_DIR_EXTR_ENTRY_INFO), "DE info")) == NULL) {
            SHR_ERR_EXIT(_SHR_E_MEMORY, "Failed to allocate SOC_PPC_FP_DIR_EXTR_ENTRY_INFO\n");
        }

        for (soc_sandOffset = start_entry_id; soc_sandOffset < entryDeLimit; soc_sandOffset++) {
            sal_memset(entInfoDe, 0, sizeof(SOC_PPC_FP_DIR_EXTR_ENTRY_INFO));
            soc_sandResult = soc_ppd_fp_direct_extraction_entry_get(unit,
                                                                    table_id,
                                                                    soc_sandOffset,
                                                                    &okay,
                                                                    entInfoDe);
            result = handle_sand_result(soc_sandResult);
            if ((BCM_E_NONE == result) && okay) {
                PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                PRT_CELL_SET("%d", soc_sandOffset);
                for (i = 0; i < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; i++) {
                    if (_BCM_DPP_FIELD_PPD_QUAL_VALID(entInfoDe->qual_vals[i].type)) {
                        _dpp_field_get_qual_str(&entInfoDe->qual_vals[i], qual_val_str, qual_size[i]);
                        PRT_CELL_SET("%s", qual_val_str);
                    } else if (_BCM_DPP_FIELD_PPD_QUAL_VALID(dbInfo.qual_types[i])) {
                        PRT_CELL_SKIP(1);
                    }
                }
                PRT_CELL_SKIP(1);
                PRT_CELL_SET("%d", entInfoDe->priority);
                PRT_CELL_SKIP(1);
                for (i = 0; i < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; i++) {
                    if (_BCM_DPP_FIELD_PPD_ACTION_VALID(entInfoDe->actions[i].type)) {
                        PRT_CELL_SET("base  %08X", entInfoDe->actions[i].base_val);
                        for (k = 0; k < entInfoDe->actions[i].nof_fields; k++) {
                            PRT_ROW_ADD(PRT_ROW_SEP_NONE);
                            PRT_CELL_SKIP(qual_num + 4);
                            if ((entInfoDe->actions[i].fld_ext[k].cst_val) ||
                                (BCM_FIELD_ENTRY_INVALID == entInfoDe->actions[i].fld_ext[k].type)) {
                                PRT_CELL_SET("const %08X [%d-0]",
                                         entInfoDe->actions[i].fld_ext[k].cst_val,
                                         entInfoDe->actions[i].fld_ext[k].nof_bits - 1);
                            } else {
                                PRT_CELL_SET("qual  %s [%d-%d]",
                                         SOC_PPC_FP_QUAL_TYPE_to_string(entInfoDe->actions[i].fld_ext[k].type),
                                         entInfoDe->actions[i].fld_ext[k].nof_bits + entInfoDe->actions[i].fld_ext[k].fld_lsb - 1,
                                         entInfoDe->actions[i].fld_ext[k].fld_lsb);
                            }
                        } /* for (all extractions this action) */
                    } /* if (this action is valid) */
                } /* for (all actions this entry) */
                if(!(--count))
                    break;
            } /* if ((BCM_E_NONE == result) && okay) */
        } /* for (soc_sandOffset = 0; soc_sandOffset < 4K; soc_sandOffset++) */
    }

    /* Coverity: We are not going to change the macros to aviod such defects */
    /* coverity[dead_error_line]*/
    PRT_COMMIT;
exit:
    if(entInfoDe != NULL)
        sal_free(entInfoDe);
    if(entInfoTc != NULL)
        sal_free(entInfoTc);

    return SHR_ERR_TO_SHELL;
}

static int
cmd_dpp_field_tables_dump(int unit, args_t* a)
{
    cmd_result_t res = CMD_OK;
    parse_table_t pt;
    int db_id = -1;
    int start_entry_id = 0, entry_num = 0;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "db", PQ_DFL|PQ_INT, &db_id,  &db_id, NULL);
    parse_table_add(&pt, "start", PQ_DFL|PQ_INT, 0,  &start_entry_id, NULL);
    parse_table_add(&pt, "num", PQ_DFL|PQ_INT, 0,  &entry_num, NULL);

    if (parse_arg_eq(a, &pt) < 0) {
        cli_out("%s: Invalid option: %s\n", ARG_CMD(a), ARG_CUR(a));
        return CMD_USAGE;
    }

    if (ARG_CNT(a) != 0) {
        cli_out("%s: extra options starting with \"%s\"\n", ARG_CMD(a), ARG_CUR(a));
        return CMD_USAGE;
    }

    if(db_id == -1) {
        res = cmd_dpp_field_table_list(unit);
    } else if((db_id  >= 0) && (db_id < SOC_PPC_FP_NOF_DBS)) {
        res = cmd_dpp_field_table_dump(unit, db_id, start_entry_id, entry_num);
    } else {
        cli_out("Illegal db id:%d\n", db_id);
    }

    return res;
}

cmd_result_t
cmd_dpp_diag_field(int unit, args_t* a)
{
    char      *function;

    function = ARG_GET(a);
    if (! function ) {
        return CMD_USAGE;
    } else if (DIAG_FUNC_STR_MATCH(function, "LAST_packet_get", "LAST")) {
        return cmd_ppd_api_fp_packet_diag_get(unit, a);
    } else if (DIAG_FUNC_STR_MATCH(function, "RESource_get", "RES")) {
        return cmd_ppd_api_fp_resource_diag_get(unit, a);
    } else if (DIAG_FUNC_STR_MATCH(function, "ACtion_info", "AC")) {
        return cmd_ppd_api_fp_action_info_show(unit, a);
    } else if (DIAG_FUNC_STR_MATCH(function, "DB_action_info", "DB")) {
        return cmd_ppd_api_fp_dbs_action_info_show(unit, a);
    } else if (DIAG_FUNC_STR_MATCH(function, "Tables", "T")) {
        return cmd_dpp_field_tables_dump(unit, a);
    } else if (DIAG_FUNC_STR_MATCH(function, "Ranges", "R")) {
        return cmd_dpp_field_ranges_dump(unit);
    } else if (DIAG_FUNC_STR_MATCH(function, "FEMS_for_stage", "FEMS")) {
        return cmd_ppd_api_fp_print_all_fems_for_stage(unit, a);
    } else if (DIAG_FUNC_STR_MATCH(function, "FESS_for_stage", "FESS")) {
        return cmd_ppd_api_fp_print_fes_info_for_stage(unit, a);
    } else if (! sal_strncasecmp(function, "test", strlen(function))){
        return cmd_diag_test(unit, a);
    } else if (! sal_strncasecmp(function, "pr_indices", strlen(function))){
        return diag_soc_sand_print_indices(unit, a) ;
    } else if (! sal_strncasecmp(function, "pr_lists", strlen(function))){
        return diag_soc_sand_print_list(unit, a) ;
    }
    else {
        return CMD_USAGE;
    } 
}

void
print_field_usage(int unit)
{
    char cmd_dpp_diag_field_usage[] =
    "Usage (DIAG field):"
    "\n\tDIAGnotsics field commands\n\t"
    "Usages:\n\t"
    "DIAG field [OPTION] <parameters> ..."
#ifdef COMPILER_STRING_CONST_LIMIT
    "\nFull documentation cannot be displayed with -pendantic compiler\n";
#else
    "OPTION can be:"
    "\nLAST_packet_get - Field ACL results (Key built, TCAM hit and actions done) for the last packet"
    "\n"
    "\nRESource_get <0/1/2> - Diagnostics to reflect which HW resources are used" 
    "\n                       0 - resource usage"
    "\n                       1 - include consistency between SW and HW"
    "\n                       2 - include validation of bank entries"
    "\n"
    "\nACtion_info -          Diagnostics to display internal actions and corresponding BCM identifiers" 
    "\n" 
    "\nDB_action_info -       Diagnostics to display internal actions and corresponding data basess" 
    "\n" 
    "\nTables [db=<table id>] [start=<entry id>] [num=<number>] - Table show facility"
    "\n                       Using without parameters will present all tables with qualifiers and actions"
    "\n                         use db in order to present specific table"
    "\n                         use start and num parameters if the table has a lot of entries"
    "\n"
    "\nRanges          -      Show all ranges used by qualifiers"
    "\n"
    "\nFEMS_for_stage <stage> <is_for_tm> -"
    "\n                       Diagnostics to display all FEMs assigned to specified stage" 
    "\n                       First parameter (stage) may be:" 
    "\n                         0 - SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF" 
    "\n                         1 - SOC_PPC_FP_DATABASE_STAGE_EGRESS" 
    "\n                         etc." 
    "\n                       Second parameter (is_for_tm):" 
    "\n                         Set 1 for TM processing (packets with ITMH header)" 
    "\n                         Set 0 for stacking or TDM processing (packets with FTMH header)" 
    "\nFESS_for_stage <stage> <program> -"
    "\n                       Diagnostics to display all FESs assigned to specified stage and program" 
    "\n                       First parameter (stage) may be:" 
    "\n                         0 - SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF" 
    "\n                         1 - SOC_PPC_FP_DATABASE_STAGE_EGRESS" 
    "\n                         etc." 
    "\n                       Second parameter (program):" 
    "\n                         Currently 0 - 31" 
    "\n" 
    "\ntest <test name> (or \"all\") "
    "\n\t\tx - key number."
    "\n\t\tmode - <Fast/Medium/Slow/Scan>. \n"
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    "\n\t\tstage - <Ingress/Egress/External>. \n"
#else
    "\n\t\tstage - <Ingress/Egress>. \n"
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
#ifdef BCM_WARM_BOOT_SUPPORT
    "\n\t\twarmboot <0/1>. Do warmboot after field group and entries install. The default is FALSE.\n"
#endif
    "\npr_indices <sorted list id> - Print contents of direct indices created for specified sorted list"
    "\n"
    "\npr_lists <sorted list id>   - Print contents of specified sorted list"
    "\n"
        ;
#endif   /*COMPILER_STRING_CONST_LIMIT*/

    cli_out(cmd_dpp_diag_field_usage);
}







cmd_result_t
cmd_dpp_diag_dbal(int unit, args_t* a)
{
    char    *function, *function1, *function2;
    int     val = -1, val1, val2;

    function = ARG_GET(a);

    if (! function ) {
        return CMD_USAGE;
    } else if (DIAG_FUNC_STR_MATCH(function, "Tables_Info", "ti")) {
        if (arad_pp_dbal_tables_dump(unit, 0)) {
            return CMD_FAIL;
        }
        return CMD_OK;

    } else if (DIAG_FUNC_STR_MATCH(function, "Prefix_Info", "pi")) {
        if (arad_pp_dbal_lem_prefix_table_dump(unit)) {
            return CMD_FAIL;
        }
        if (arad_pp_dbal_isem_prefix_table_dump(unit)) {
            return CMD_FAIL;
        }
        if (arad_pp_dbal_tcam_prefix_table_dump(unit)) {
            return CMD_FAIL;
        }
        return CMD_OK;

    } else if (DIAG_FUNC_STR_MATCH(function, "Table", "t")) {
        function = ARG_GET(a);
        if (function) {
            val = sal_ctoi(function,0);
            if (arad_pp_dbal_table_info_dump(unit, val)) {
                return CMD_FAIL;
            }
        } else {
            return CMD_USAGE;
        }
    }else if (DIAG_FUNC_STR_MATCH(function, "DB_Dump", "dbd")) {
        function = ARG_GET(a);
        if (function) {
            val = sal_ctoi(function,0);
            if (arad_pp_dbal_phisycal_db_dump(unit, val)) {
                return CMD_FAIL;
            }
        } else {
            return CMD_USAGE;
        }
    } else if (DIAG_FUNC_STR_MATCH(function, "Last_Packet", "lp")) {
        int core_id;
        for (core_id = 0; core_id < SOC_DPP_DEFS_GET(unit, nof_cores); core_id++) {
            if (arad_pp_dbal_last_packet_dump(unit, core_id)) {
                return CMD_FAIL;
            }
        }
    } else if (DIAG_FUNC_STR_MATCH(function, "INstraction", "in")) {
        function = ARG_GET(a);
        function1 = ARG_GET(a);
        function2 = ARG_GET(a);
        if (function && function1 && function2) {
            val = sal_ctoi(function,0);
            val1 = sal_ctoi(function1,0);
            val2 = sal_ctoi(function2,0);
            if (arad_pp_dbal_qualifier_to_instruction_dump(unit, val, val1, 0, val2)) {
                return CMD_FAIL;
            }
        } else {
            return CMD_USAGE;
        }
    } else if (DIAG_FUNC_STR_MATCH(function, "Table_Entries", "te")) {
        function = ARG_GET(a);
        if (function) {
            val = sal_ctoi(function,0);
            if (arad_pp_dbal_table_print(unit, val)) {
                return CMD_FAIL;
            }
        } else {
            return CMD_USAGE;
        }
    } else if (DIAG_FUNC_STR_MATCH(function, "Tables_Info_Full", "tif")) {
        if (arad_pp_dbal_tables_dump(unit, 1)) {
            return CMD_FAIL;
        }
    } else if (DIAG_FUNC_STR_MATCH(function, "PRogram_Info", "pri")) {
        function = ARG_GET(a);
        function1 = ARG_GET(a);
        if (function && function1) {
            val = sal_ctoi(function,0);
            val1 = sal_ctoi(function1,0);
            if (arad_pp_dbal_ce_per_program_dump(unit, val, val1)) {
                return CMD_FAIL;
            }
        } else {
            return CMD_USAGE;
        }
    } else if (DIAG_FUNC_STR_MATCH(function, "entry_ADD", "add")) {
        function = ARG_GET(a);
        if (!function) {
            return CMD_USAGE;
        }
        val = sal_ctoi(function, 0);
        if (arad_pp_dbal_diag_entry_manage(unit, SOC_DPP_DBAL_DIAG_ENTRY_MANAGE_ADD, val, a)) {
            return CMD_FAIL;
        }
    } else if (DIAG_FUNC_STR_MATCH(function, "entry_DELete", "del")) {
        function = ARG_GET(a);
        if (!function) {
            return CMD_USAGE;
        }
        val = sal_ctoi(function, 0);
        if (arad_pp_dbal_diag_entry_manage(unit, SOC_DPP_DBAL_DIAG_ENTRY_MANAGE_DELETE, val, a)) {
            return CMD_FAIL;
        }
    } else {
        return CMD_USAGE;
    }

    return CMD_OK;
}

void
print_dbal_usage(int unit)
{
    char cmd_dpp_diag_dbal_usage[] =
    "Usage (DIAG dbal):"
    "\n\tDIAGnotsics dbal commands\n\t"
    "Usages:\n\t"
    "DIAG dbal [OPTION] <parameters> ..."
#ifdef COMPILER_STRING_CONST_LIMIT
    "\nFull documentation cannot be displayed with -pendantic compiler\n";
#else
    "OPTION can be:"
    "\nTables_Info                           - Returns brief information about all existing tables."
    "\nTable <ID>                            - Returns full information for a specific table ID." 
    "\nTables_Info_Full                      - Returns full information for all existing tables."
    "\nTable_Entries <ID>                    - Returns all entries in a table."
    "\nDB_Dump <DB>                          - Returns all entries in physical DB, LEM=0, TCAM=1, KBP=2, SEM A=3, SEM B=4, "
    "                                          6=ESEM, OAM1=7, OAM2=8, RMEP=9, GLEM=10."
	/*"\nentry_ADD <ID> <q1>..<qn> <payload>   - Adds an entry with <q1>..<qn> => <payload> to table <ID>."
    "\nentry_DELete <ID> <q1>..<qn>          - Deletes the entry identified with <q1>..<qn> from table <ID>.\n"
	"                                          Leave <q1>..<qn> and <payload> empty to list required qualifiers.\n"
	"                                          Use ':' to seperate between words of a multi-world value [ms]:..:[ls]"*/
    "\nLast_Packet <core>                    - Returns full information for the last packet lookups from the VT, TT and FLP. if core not add core=0"
    "\nPrefix_Info                           - Returns prefix allocation mapping for exact match databases."
    "\nPRogram_Info <programID> <stage>      - Returns program information for a specific program and stage.\n"
    "                                          Stage selected according to the following: PMF = 0, FLP = 2, SLB = 3, VT = 4, TT = 5\n";
#endif   /*COMPILER_STRING_CONST_LIMIT*/

    cli_out(cmd_dpp_diag_dbal_usage);
}
/* } */
