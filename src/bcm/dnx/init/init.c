/** \file init.c
 * DNX init sequence functions.
 * 
 * The DNX init sequence should be constructed of a series of well defined steps.
 * each step should have a seperate init and deinit functions, where the deinit function
 * should not access the HW but just free allocated resources.
 * 
 *
 * each step should have a built in mechanism to test for freeing all allocated resources.
 * each step should have a built in time stamping mechanism to provide an option to test for deviation from time frames.
 */

/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_INITSEQDNX

/*
 * Include files.
 * {
 */
#include <shared/bsl.h>
#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm_int/control.h>
#include <bcm_int/dnx/init/init.h>
#include <bcm_int/dnx/vlan/vlan.h>
#include <bcm_int/dnx/mirror/mirror.h>
#include <soc/drv.h>
#include <soc/dnx/dnx_data/dnx_data.h>
#include <shared/shrextend/shrextend_debug.h>
#include <soc/dnx/access.h>
#include <soc/dnx/drv.h>
#include <soc/shared/access_pack.h>

#include <soc/dnx/dbal/dbal.h>
#include <soc/dnx/legacy/dnx_config_defs.h>
#include <soc/dnx/algo/res_mngr/res_mngr_internal.h>
#include <bcm_int/dnx/algo/template_mngr/template_mngr_api.h>
#include <bcm_int/dnx/algo/mirror/algo_mirror.h>
#include <bcm_int/dnx/algo/lif_mngr/lif_mngr.h>

#include "init_cb_wrappers.h"

/*
 * }
 */

/**
 * \brief Const array to provide string format for each step name for printing purpuses.
 *  
 * The Indexes in this array need to be alligned to the Enum \ref dnx_init_step_id_e
 */
const char *dnx_init_step_str_name[] = DNX_INIT_STEP_NAME_STR;

/**
 * \brief step/sub-step structure.
 *
 * each step in the Init/Deinit process should be well defined.
 * each step should have an Init and Deinit function.
 * the Deinit function should not access the HW, just free SW resources.
 * a flag CB can also be used, so each step can activate flags according
 * to dedicated SOC properties or other logic.
 */
typedef struct dnx_init_step_s dnx_init_step_t;
struct dnx_init_step_s
{
  /** 
   * Step ID, used to uniquely identify a step. 
   */
    dnx_init_step_id_e step_id;

  /** 
   * Init function CB that will run during the Init Sequence. 
   */
    dnx_step_cb init_function;

  /** 
   * Deinit function CB that will run during the Deinit Sequence.
   */
    dnx_step_cb deinit_function;

  /** 
   * Flag function CB that will run prior to the init function of 
   * this step to determine which flags are needed to the step 
   * according to the CB logic - could be looking for certain SOC 
   * properties for example. 
   */
    dnx_step_flag_cb flag_function;

  /** 
   * Step Flags, internal flags used by the system's logic
   */
    int step_flags;

  /** 
   * Pointer to a sub list of steps to be used if current step 
   * represents a sub list of steps. 
   */
    dnx_init_step_t *step_sub_array;
};

/**
 * \brief meta data to save for each step.
 */
typedef struct
{
  /** 
   * step starting time stamp
   */
    sal_usecs_t time_stamp_start;

  /** 
   * step ending time stamp
   */
    sal_usecs_t time_stamp_end;

  /** 
   * step starting memory alocation 
   */
    int total_memory_alocation_start;

  /** 
   * step ending memory alocation 
   */
    int total_memory_alocation_end;

} dnx_init_step_meta_data_t;

/**
 * \brief dnx init step list info structure.
 *
 * each step that represents a list of steps should have one such structure.
 * this structure is used to pass onwards down the cascaded lists the needed information
 * to maintain the init sequence system's managment.
 * the step_list_ptr is changed on each new sub level to the relevant list pointer.
 * the meta_data_arr points in all the steps to the array allocated on the higher level.
 * same goes for first/last_is_done_ptr, they point to the same place through all the levels.
 */
typedef struct dnx_init_step_list_info_s dnx_init_step_list_info_t;
struct dnx_init_step_list_info_s
{
  /**
   * this is used to start the init sequence from a certain step. 
   * this will be used after stoping the init sequence with last 
   * step. 
   */
    dnx_init_step_id_e first_step;

  /**
   * this is used to stop the init sequence at a certain step. 
   * to continue use the init function with first step defined as 
   * next step of this. 
   */
    dnx_init_step_id_e last_step;

  /**
   * pointer to current step list.
   */
    dnx_init_step_t *step_list_ptr;

  /**
   * pointer to meta_data_arr, this is used to save time stamps and detecet memory leaks.
   */
    dnx_init_step_meta_data_t *meta_data_arr;

  /**
   * pointer to veriable that marks the first step was done.
   */
    int *first_is_done_ptr;

  /**
   * pointer to veriable that marks the last step was done.
   */
    int *last_is_done_ptr;

   /**
    * List Depth - used for log indent 
    */
    int depth;
};

/*
 * Static functions forward declerations
 * {
 */
static shr_error_e dnx_init_step_list_count_steps(
    int unit,
    dnx_init_step_t * step_list_ptr,
    int *nof_steps);

static shr_error_e dnx_init_internal_verify(
    int unit,
    int flags,
    dnx_init_info_t * dnx_init_info);

static shr_error_e dnx_init_managment_logic_init(
    int unit,
    dnx_init_step_id_e step_id,
    dnx_init_step_list_info_t * step_list_info,
    int step_flags,
    int *dynamic_flags,
    dnx_init_step_t * step_sub_array);

static shr_error_e dnx_init_managment_logic_deinit(
    int unit,
    dnx_init_step_list_info_t * step_list_info,
    int step_flags,
    dnx_init_step_t * step_sub_array);

static shr_error_e dnx_init_run_step_list_init(
    int unit,
    dnx_init_step_list_info_t * step_list_info,
    int flags);

static shr_error_e dnx_init_run_step_list_deinit(
    int unit,
    dnx_init_step_list_info_t * step_list_info,
    int flags);

static shr_error_e dnx_init_internal_init(
    int unit,
    int flags,
    dnx_init_info_t * dnx_init_info);

static shr_error_e dnx_init_internal_deinit(
    int unit);

static shr_error_e dnx_init_info_t_init(
    int unit,
    dnx_init_info_t * dnx_init_info);

/*
 * }
 */

/*
 * Init Deinit step lists, Indent needs to ignore the format here 
 * {  
 */
/**
 * So how does this mechanism work? 
 *    The init process start at the begining of the high level sequence list and performs
 *    each step according to order. a step can expand to a list of sub steps. the deinint
 *    process is done in the reverse order of the init process and is used to free all allocated
 *    resources from the init.
 *  
 * General guidelines regarding how to add a new Step: 
 *  1.  Implement the Init, Deinit and flag CB. good practice will be to implement both init and deinit steps even if one of them is an empty function
 *      for future profing.
 *  2.  if your module already has an Init/Deinit functions, you can only use a wrapper to match the needed call for the CB.
 *  3.  in init.h add a step id to the enum to match your step, all step IDs must be unique and match only a single step.
 *  4.  in init.c add a step string name to show whenever the step is initiated (should be matching to the enum).
 *  5.  add an entry to the relevant list below with your CBs, step ID and other needed info.
 *  
 * Specific guidelines regarding how to add a new sub-step list: 
 *  1.  it is possible to add as many sub-steps list as one desires and in as many cascaded level as needed,
 *      common since however dictates that no more than 5 cascaded levels are really needed (and that also an over-kill).
 *  2.  to add a new sub-step list create in the origin list a new step.
 *      use pointer to the sub-step list for the SUB_STEP_LIST_PTR.
 *      you may add a flag CB function with logic when to skip this list (not mandatory).
 *  3.  you can find an example for this with either one of the existing sub-step lists ( TM, PP etc..)
 */
/* *INDENT-OFF* */
/**
 * \brief Init-Deinit SOC Modules sequence.
 *  Used for legacy modules ported from DPP
 * For specific info on steps look for each function decription
 */
dnx_init_step_t dnx_init_deinit_soc_modules_sequence[] = {
  /*STEP_ID,                      STEP_INIT,                    STEP_DEINIT,     FLAG_CB          STEP_FLAGS     SUB_STEP_LIST_PTR */
  {DNX_INIT_STEP_AUX_ACCESS,      shr_access_device_init,       NULL,            NULL,            0,             NULL},
  {DNX_INIT_STEP_LAST_STEP,       NULL,                         NULL,            NULL,            0,             NULL}
};

/**
 * \brief Init-Deinit common modules sequence.
 *
 * For specific info on steps look for each function decription
 */
dnx_init_step_t dnx_init_deinit_common_sequence[] = {
  /*STEP_ID,                      STEP_INIT,                    STEP_DEINIT,                    FLAG_CB           STEP_FLAGS      SUB_STEP_LIST_PTR */ 
  {DNX_INIT_STEP_RX,              dnx_init_rx_init,             dnx_init_rx_deinit,             NULL,             0,              NULL},
  {DNX_INIT_STEP_LAST_STEP,       NULL,                         NULL,                           NULL,             0,              NULL}
};
/**
 * \brief Init-Deinit TM sequence.
 *
 * For specific info on steps look for each function decription
 */
dnx_init_step_t dnx_init_deinit_tm_sequence[] = {
  /*STEP_ID,                      STEP_INIT,                    STEP_DEINIT,        FLAG_CB          STEP_FLAGS     SUB_STEP_LIST_PTR */
  {DNX_INIT_STEP_MIRROR_MODULE,   dnx_mirror_init,              dnx_mirror_deinit,  NULL,            0,             NULL},
  {DNX_INIT_STEP_LAST_STEP,       NULL,                         NULL,               NULL,            0,             NULL}
};

/**
 * \brief Init-Deinit PP sequence.
 *
 * For specific info on steps look for each function decription
 */
dnx_init_step_t dnx_init_deinit_pp_sequence[] = {
  /*STEP_ID,                      STEP_INIT,                    STEP_DEINIT,                    FLAG_CB          STEP_FLAGS     SUB_STEP_LIST_PTR */
  
  {DNX_INIT_STEP_LIF,             dnx_algo_lif_mngr_init,       dnx_algo_lif_mngr_deinit,       NULL,            0,             NULL},
  {DNX_INIT_STEP_VLAN_ALGO_RES,   dnx_vlan_algo_res_init,       NULL,                           NULL,            0,             NULL},
  {DNX_INIT_STEP_L3_ALGO,         dnx_init_l3_algo_init,        dnx_init_l3_algo_deinit,        NULL,            0,             NULL},
  {DNX_INIT_STEP_HW_OVERWRITE,    dnx_init_hw_overwrite_init,   NULL,                           NULL,            0,             NULL},
  {DNX_INIT_STEP_L3_MODULE,       dnx_init_l3_module_init,      dnx_init_l3_module_deinit,      NULL,            0,             NULL},
  {DNX_INIT_STEP_PP_PORT,         dnx_init_pp_port_init,        dnx_init_pp_port_deinit,        NULL,            0,             NULL},
  {DNX_INIT_STEP_PEMLA,           dnx_init_pemla_init,          NULL,                           NULL,            0,             NULL},
  {DNX_INIT_STEP_LAST_STEP,       NULL,                         NULL,                           NULL,            0,             NULL}
};

/**
 * \brief Init-Deinit legacy soc sequence.
 *
 * For specific info on steps look for each function decription
 */
dnx_init_step_t dnx_init_deinit_legacy_soc[] = {
  /*STEP_ID,                      STEP_INIT,                    STEP_DEINIT,                    FLAG_CB           STEP_FLAGS     SUB_STEP_LIST_PTR */
  {DNX_INIT_STEP_DEFINES_INIT,    soc_dnx_defines_init,         soc_dnx_defines_deinit,         NULL,             0,             NULL},
  {DNX_INIT_STEP_UTILS_SW_STATE,  soc_dnx_sw_state_utils_init,  soc_dnx_sw_state_utils_deinit,  NULL,             0,             NULL},
  {DNX_INIT_STEP_INFO_CONFIG,     soc_dnx_info_config,          NULL,                           NULL,             0,             NULL},  
  {DNX_INIT_STEP_LAST_STEP,       NULL,                         NULL,                           NULL,             0,             NULL}
};


/**
 * \brief Init-Deinit access sequence.
 *
 * For specific info on steps look for each function decription
 */
dnx_init_step_t dnx_init_deinit_access_sequence[] = {
  /*STEP_ID,                      STEP_INIT,                        STEP_DEINIT,                        FLAG_CB           STEP_FLAGS      SUB_STEP_LIST_PTR */ 
  {DNX_INIT_STEP_MEM_MUTEX,       dnx_access_mem_mutex_init,        dnx_access_mem_mutex_deinit,          NULL,           0,              NULL},
  {DNX_INIT_STEP_SCHAN,           soc_schan_init,                   soc_schan_deinit,                   NULL,             0,              NULL}, 
#ifdef CMODEL_SERVER_MODE
  {DNX_INIT_STEP_CMODEL_REG_ACC,  dnx_init_cmodel_reg_access_init,  dnx_init_cmodel_reg_access_deinit,  NULL,             0,              NULL},
#endif 
  {DNX_INIT_STEP_LAST_STEP,       NULL,                         NULL,                                   NULL,             0,              NULL}
};

/**
 * \brief Init-Deinit Algo sequence.
 *
 * For specific info on steps look for each function decription
 */
dnx_init_step_t dnx_init_deinit_algo_sequence[] = {
  /*STEP_ID,                      STEP_INIT,                    STEP_DEINIT,                        FLAG_CB             STEP_FLAGS      SUB_STEP_LIST_PTR */
  {DNX_INIT_STEP_ALGO_RES,        dnx_algo_res_init,            dnx_algo_res_deinit,                NULL,               0,              NULL},
  {DNX_INIT_STEP_ALGO_TEMPLATE,   dnx_algo_template_init,       dnx_algo_template_deinit,           NULL,               0,              NULL},
  {DNX_INIT_STEP_ALGO_MIRROR,     dnx_algo_mirror_init,         dnx_algo_mirror_deinit,             NULL,               0,              NULL},
  {DNX_INIT_STEP_LAST_STEP,       NULL,                         NULL,                               NULL,               0,              NULL}
};

/**
 * \brief Init-Deinit Utilities sequence.
 *
 * For specific info on steps look for each function decription
 */
dnx_init_step_t dnx_init_deinit_utilities_sequence[] = {
  /*STEP_ID,                          STEP_INIT,                        STEP_DEINIT,                        FLAG_CB           STEP_FLAGS                SUB_STEP_LIST_PTR */   
  {DNX_INIT_STEP_FEATURE,             dnx_init_feature_init,            dnx_init_feature_deinit,            NULL,             0,                        NULL},
  {DNX_INIT_STEP_MUTEXES,             soc_dnx_mutexes_init,             soc_dnx_mutexes_deinit,             NULL,             0,                        NULL},
  {DNX_INIT_STEP_LAST_STEP,           NULL,                             NULL,                               NULL,             0,                        NULL}
};

/**
 * \brief Appl properties sequence.
 *
 * Used for dynamic soc properties (i.e. properties that can be set by both API and soc property) 
 * In order to avoid from code duplication, those properties should be set as last step of init sequence. 
 * The expected implementaion will be to get the relevant values from dnx data, and to set those values 
 * through BCM APIs. 
 */
dnx_init_step_t dnx_init_deinit_appl_properties_sequence[] = {
  /*STEP_ID,                          STEP_INIT,                        STEP_DEINIT,                        FLAG_CB           STEP_FLAGS                SUB_STEP_LIST_PTR */   
  {DNX_INIT_STEP_LAST_STEP,           NULL,                             NULL,                               NULL,             0,                        NULL}
};


/**
 * \brief Init-Deinit Utilities sequence.
 *
 * For specific info on steps look for each function decription
 */
dnx_init_step_t dnx_init_deinit_defaults_sequence[] = {
  /*STEP_ID,                          STEP_INIT,                        STEP_DEINIT,                        FLAG_CB           STEP_FLAGS                SUB_STEP_LIST_PTR */   
  {DNX_INIT_STEP_LAST_STEP,           NULL,                             NULL,                               NULL,             0,                        NULL}
};

/**
 * \brief Init-Deinit High level sequence.
 *
 * For specific info on steps look for each function decription
 */
dnx_init_step_t dnx_init_deinit_sequence[] = {
  /*STEP_ID,                          STEP_INIT,                        STEP_DEINIT,                        FLAG_CB           STEP_FLAGS                SUB_STEP_LIST_PTR */   
  {DNX_INIT_STEP_RESTORE_SOC_CONTROL, NULL,                             soc_dnx_restore,                    NULL,             0,                        NULL},
  {DNX_INIT_STEP_CONTROL,             soc_dnx_control_init,             soc_dnx_control_deinit,             NULL,             0,                        NULL},
  {DNX_INIT_STEP_LEGACY_SOC,          NULL,                             NULL,                               NULL,             0,                        dnx_init_deinit_legacy_soc},
  {DNX_INIT_STEP_DNX_DATA,            dnx_data_init,                    dnx_data_deinit,                    NULL,             0,                        NULL},
  {DNX_INIT_STEP_UTILITIES,           NULL,                             NULL,                               NULL,             0,                        dnx_init_deinit_utilities_sequence},
  {DNX_INIT_STEP_ACCESS,              NULL,                             NULL,                               NULL,             0,                        dnx_init_deinit_access_sequence},
  {DNX_INIT_STEP_DEFAULTS,            NULL,                             NULL,                               NULL,             0,                        dnx_init_deinit_defaults_sequence},
  {DNX_INIT_STEP_SOC_MODULES,         NULL,                             NULL,                               NULL,             0,                        dnx_init_deinit_soc_modules_sequence},
  {DNX_INIT_STEP_DBAL,                dbal_init,                        dbal_deinit,                        NULL,             0,                        NULL},
  {DNX_INIT_STEP_ALGO,                NULL,                             NULL,                               NULL,             0,                        dnx_init_deinit_algo_sequence},
  {DNX_INIT_STEP_COMMON_MODULES,      NULL,                             NULL,                               NULL,             0,                        dnx_init_deinit_common_sequence},
  {DNX_INIT_STEP_TM_MODULES,          NULL,                             NULL,                               NULL,             0,                        dnx_init_deinit_tm_sequence},
  {DNX_INIT_STEP_PP_MODULES,          NULL,                             NULL,                               NULL,             0,                        dnx_init_deinit_pp_sequence},
  {DNX_INIT_STEP_APPL_PROPERTIES,     NULL,                             NULL,                               NULL,             0,                        dnx_init_deinit_appl_properties_sequence},
  {DNX_INIT_STEP_INIT_DONE,           dnx_init_done_init,               dnx_init_done_deinit,               NULL,             0,                        NULL},
  {DNX_INIT_STEP_LAST_STEP,           NULL,                             NULL,                               NULL,             0,                        NULL}
};
/* *INDENT-ON* */
/*
 * }
 */

/**
 * \brief Init \ref dnx_init_info_t with default values. 
 *  
 */
static shr_error_e
dnx_init_info_t_init(
    int unit,
    dnx_init_info_t * dnx_init_info)
{
    SHR_FUNC_INIT_VARS(unit);
    SHR_NULL_CHECK(dnx_init_info, _SHR_E_MEMORY, "dnx_init_info");

    dnx_init_info->first_step = DNX_INIT_STEP_INVALID_STEP;
    dnx_init_info->last_step = DNX_INIT_STEP_INVALID_STEP;

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - backward competible init function. 
 * Wrapper to \ref dnx_init_internal_init with default init info
 * and flags. 
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e
bcm_dnx_init(
    int unit)
{
    int flags = 0;
    dnx_init_info_t dnx_init_info;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * set defaults for init info and flags. 
     * currently no flags used by default. 
     */
    SHR_IF_ERR_EXIT(dnx_init_info_t_init(unit, &dnx_init_info));

    /*
     * run internal init which is capable of getting arguments and flags. 
     * we keep the bcm_dnx_init just for backward compitability, the new 
     * function can use init info and flags. 
     */
    SHR_IF_ERR_EXIT(dnx_init_internal_init(unit, flags, &dnx_init_info));

exit:
    SHR_FUNC_EXIT;
}


/**
 * \brief - new init function. wrapper to \ref dnx_init_internal_init. 
 * caller can decide himself on flags and init info.
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   \param [in] flags - possible flags are: BCM_INIT_ADVANCED_F_...
 *   \param [in] dnx_init_info - additional user information for init sequence, should be kept minimal.
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e
bcm_dnx_init_advanced(
    int unit,
    int flags,
    dnx_init_info_t * dnx_init_info)
{
    SHR_FUNC_INIT_VARS(unit);
    SHR_IF_ERR_EXIT(dnx_init_internal_init(unit, flags, dnx_init_info));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - counts NOF members in step list
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   \param [in] step_list_ptr - pointer to step list
 *   \param [out] nof_steps - returned result
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   list MUST contain at least one member (last member) with
 *   step id == DNX_INIT_STEP_LAST_STEP.
 * \see
 *   * None
 */
static shr_error_e
dnx_init_step_list_count_steps(
    int unit,
    dnx_init_step_t * step_list_ptr,
    int *nof_steps)
{
    int step_index;
    int count = 0;

    SHR_FUNC_INIT_VARS(unit);
    SHR_NULL_CHECK(step_list_ptr, _SHR_E_PARAM, "step_list_ptr");
    SHR_NULL_CHECK(nof_steps, _SHR_E_PARAM, "nof_steps");

    /*
     * all lists must have at least one member - the last one - with step_id == DNX_INIT_STEP_LAST_STEP
     */
    for (step_index = 0; step_list_ptr[step_index].step_id != DNX_INIT_STEP_LAST_STEP; ++step_index)
    {
        ++count;
    }

    /*
     * Add last member
     */
    ++count;

    *nof_steps = count;

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Internal Init input verification function.
 *   returns error if contredicting flags or input data 
 *   is used.
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   \param [in] flags - possible flags are: BCM_INIT_ADVANCED_F_...
 *   \param [in] dnx_init_info - additional user information for init sequence, should be kept minimal.
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
static shr_error_e
dnx_init_internal_verify(
    int unit,
    int flags,
    dnx_init_info_t * dnx_init_info)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(dnx_init_info, _SHR_E_MEMORY, "dnx_init_info");

    /**
     * check that MEM_LEAK and TIEM_STAMP flags are not used when
     * PARTIAL_INIT is used.
     */
    if ((flags & (BCM_INIT_ADVANCED_F_MEM_LEAK_DETECT | BCM_INIT_ADVANCED_F_TIME_STAMP)) &&
        (flags & BCM_INIT_ADVANCED_F_PARTIAL_INIT))
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "MEM_LEAK or TIME_STAMP flags are set with PARTIAL_INIT, invalid configuration\n");
    }

    /**
     * check that PARTIAL_INIT flag is used only when given valid
     * last_step or first_step.
     */
    if (((dnx_init_info->first_step != DNX_INIT_STEP_INVALID_STEP) ||
         (dnx_init_info->last_step != DNX_INIT_STEP_INVALID_STEP)) && ((flags & BCM_INIT_ADVANCED_F_PARTIAL_INIT) == 0))
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "PARTIAL_INIT flag is not set when it should be set\n");
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Managment function for running step list (init 
 *        direction).
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   \param [in] step_id - current step_id.
 *   \param [in,out] step_list_info - ptr to relevant
 *          step_list_info.
 *   \param [in] step_flags - possible flags are: DNX_INIT_STEP_F_..._INTERNAL
 *   \param [in] dynamic_flags - result dynamic flags according to internal logic
 *   \param [in] step_sub_array - pointer to sub-step list.
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   makes the needed decisions regarding the dynamic flags,
 *   contorls the stop at and continue from step options. makes the
 *   needed modifications for step_list_info to be taken to the next sub_steps.
 * \see
 *   * None
 */
static shr_error_e
dnx_init_managment_logic_init(
    int unit,
    dnx_init_step_id_e step_id,
    dnx_init_step_list_info_t * step_list_info,
    int step_flags,
    int *dynamic_flags,
    dnx_init_step_t * step_sub_array)
{

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Am I First? 
     */
    if (step_id == step_list_info->first_step)
    {
        *step_list_info->first_is_done_ptr = 1;
    }

    /*
     * Am I list 
     */
    if (step_sub_array != NULL)
    {
        /*
         * Change list ptr to new list, if step is list - it must have a valid sub-list ptr 
         */
        step_list_info->step_list_ptr = step_sub_array;
    }
    /*
     * Is first Not done? 
     */
    else if (*step_list_info->first_is_done_ptr == 0)
    {
        /*
         * Skip this step 
         */
        *dynamic_flags |= DNX_INIT_STEP_F_SKIP_DYNAMIC;
    }

    /*
     * Is last done? 
     */
    if (*step_list_info->last_is_done_ptr == 1)
    {
        /*
         * Skip this step 
         */
        *dynamic_flags |= DNX_INIT_STEP_F_SKIP_DYNAMIC;
    }

    /*
     * Am I Last? 
     */
    if (step_id == step_list_info->last_step)
    {
        *step_list_info->last_is_done_ptr = 1;
    }

    SHR_FUNC_EXIT;
}

/**
 * \brief - Managment function for running step list (deinit 
 *        direction).
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   \param [in,out] step_list_info - ptr to relevant
 *          step_list_info.
 *   \param [in] step_flags - possible flags are:
 *          DNX_INIT_STEP_F_..._INTERNAL
 *   \param [in] step_sub_array - pointer to sub-step list.
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
static shr_error_e
dnx_init_managment_logic_deinit(
    int unit,
    dnx_init_step_list_info_t * step_list_info,
    int step_flags,
    dnx_init_step_t * step_sub_array)
{
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Am I list 
     */
    if (step_sub_array != NULL)
    {
        /*
         * Change list ptr to new list, if step is list - it must have a valid sub-list ptr 
         */
        step_list_info->step_list_ptr = step_sub_array;
    }

    SHR_FUNC_EXIT;
}

/**
 * \brief - function to run a list of steps (init direction)
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   \param [in] step_list_info - ptr to relevant step_list_info.
 *   \param [in] flags - possible flags are: BCM_INIT_ADVANCED_F_...
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   the funtion is also responsible for making decisions regarding
 *   time stamping, memory leak detections and skipping steps according
 *   to input flags and step_list_info.
 * \see
 *   * None
 */
static shr_error_e
dnx_init_run_step_list_init(
    int unit,
    dnx_init_step_list_info_t * step_list_info,
    int flags)
{
    int step_index;
    int dynamic_flags = 0;
    int nof_steps_in_list = 0;
    dnx_init_step_t *current_step;
    dnx_init_step_id_e current_step_id;
    dnx_init_step_list_info_t step_list_info_internal;
    int depth_index;

    SHR_FUNC_INIT_VARS(unit);
    SHR_NULL_CHECK(step_list_info, _SHR_E_MEMORY, "step_list_info");

    /*
     * Find NOF steps in list 
     */
    SHR_IF_ERR_EXIT(dnx_init_step_list_count_steps(unit, step_list_info->step_list_ptr, &nof_steps_in_list));

    /*
     * Copy step_list_info incase it is needed to be changed internally 
     */
    step_list_info_internal = *step_list_info;
    step_list_info_internal.depth++;
    for (step_index = 0; step_index < nof_steps_in_list; ++step_index)
    {
        current_step = &(step_list_info->step_list_ptr[step_index]);
        current_step_id = current_step->step_id;

        /*
         * Save time stamp and mem allocation at begining of step 
         */
        if (flags & BCM_INIT_ADVANCED_F_TIME_STAMP)
        {
            step_list_info->meta_data_arr[current_step_id].time_stamp_start = sal_time_usecs();
        }

        if (flags & BCM_INIT_ADVANCED_F_MEM_LEAK_DETECT)
        {
            
            step_list_info->meta_data_arr[current_step_id].total_memory_alocation_start = 0;
        }

        /*
         * Run Flag function if exists 
         */
        dynamic_flags = 0;
        if (current_step->flag_function != NULL)
        {
            SHR_IF_ERR_EXIT(current_step->flag_function(unit, &dynamic_flags));
        }

        /*
         * Decisions made by the managment logic are stronger then ones made by the flag cb function 
         */
        SHR_IF_ERR_EXIT(dnx_init_managment_logic_init
                        (unit, current_step_id, &step_list_info_internal, current_step->step_flags, &dynamic_flags,
                         current_step->step_sub_array));
        if ((dynamic_flags & DNX_INIT_STEP_F_SKIP_DYNAMIC) != 0)
        {
            /*
             * Skip 
             */
            LOG_VERBOSE_EX(BSL_LOG_MODULE, "Step %s is skipped%s%s%s\n", dnx_init_step_str_name[current_step_id], EMPTY,
                           EMPTY, EMPTY);
        }
        else if ((dynamic_flags & DNX_INIT_STEP_F_WB_SKIP_DYNAMIC) != 0)
        {
            /*
             * Skip due to WB 
             */
            LOG_VERBOSE_EX(BSL_LOG_MODULE, "Step %s is skipped during WB%s%s%s\n",
                           dnx_init_step_str_name[current_step_id], EMPTY, EMPTY, EMPTY);
        }
        else
        {
            /*
             * Run Step INIT function if exists 
             */
            if (current_step->init_function != NULL)
            {
                /*
                 * Log info
                 */
                LOG_INFO_EX(BSL_LOG_MODULE, "INIT: %s%s%s%s", EMPTY, EMPTY, EMPTY, EMPTY);
                for (depth_index = 0; depth_index < step_list_info->depth; depth_index++)
                {
                    LOG_INFO_EX(BSL_LOG_MODULE, "   - %s%s%s%s", EMPTY, EMPTY, EMPTY, EMPTY);
                }
                LOG_INFO_EX(BSL_LOG_MODULE, "%s\n%s%s%s", dnx_init_step_str_name[current_step_id], EMPTY, EMPTY, EMPTY);

                /*
                 * Run init function 
                 */
                SHR_IF_ERR_EXIT(current_step->init_function(unit));
            }

            /*
             * If list - init list steps
             */
            if (current_step->step_sub_array != NULL)
            {
                /*
                 * Log info
                 */
                LOG_INFO_EX(BSL_LOG_MODULE, "INIT: %s%s%s%s", EMPTY, EMPTY, EMPTY, EMPTY);
                for (depth_index = 0; depth_index < step_list_info->depth; depth_index++)
                {
                    LOG_INFO_EX(BSL_LOG_MODULE, "    - %s%s%s%s", EMPTY, EMPTY, EMPTY, EMPTY);
                }
                LOG_INFO_EX(BSL_LOG_MODULE, "%s:\n%s%s%s", dnx_init_step_str_name[current_step_id], EMPTY, EMPTY,
                            EMPTY);

                /*
                 * Run init list function
                 */
                SHR_IF_ERR_EXIT(dnx_init_run_step_list_init(unit, &step_list_info_internal, flags));
            }
        }

        /*
         * Save time stamp and mem allocation at the end of the step 
         */
        if (flags & BCM_INIT_ADVANCED_F_TIME_STAMP)
        {
            step_list_info->meta_data_arr[current_step_id].time_stamp_end = sal_time_usecs();
        }

        if (flags & BCM_INIT_ADVANCED_F_MEM_LEAK_DETECT)
        {
            
            step_list_info->meta_data_arr[current_step_id].total_memory_alocation_end = 0;
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Function to run a list of steps (deinit direction)
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   \param [in] step_list_info - ptr to relevant step_list_info.
 *   \param [in] flags - possible flags are: BCM_INIT_ADVANCED_F_...
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   the funtion is also responsible for making decisions regarding
 *   memory leak detections according to input flags and step_list_info.
 * \see
 *   * None
 */
static shr_error_e
dnx_init_run_step_list_deinit(
    int unit,
    dnx_init_step_list_info_t * step_list_info,
    int flags)
{
    int step_index;
    int nof_steps_in_list = 0;
    int total_memory_alocation_end;
    int total_memory_alocation_start;
    int memory_freed_during_deinit;
    int memory_alocation_during_init;
    dnx_init_step_t *current_step;
    dnx_init_step_id_e current_step_id;
    dnx_init_step_list_info_t step_list_info_internal;
    int depth_index;

    SHR_FUNC_INIT_VARS(unit);
    SHR_NULL_CHECK(step_list_info, _SHR_E_MEMORY, "step_list_info");

    /*
     * Find NOF steps in list 
     */
    SHR_IF_ERR_EXIT(dnx_init_step_list_count_steps(unit, step_list_info->step_list_ptr, &nof_steps_in_list));

    /*
     * Copy step_list_info incase it is needed to be changed internally 
     */
    step_list_info_internal = *step_list_info;
    step_list_info_internal.depth++;
    for (step_index = nof_steps_in_list - 1; step_index >= 0; --step_index)
    {
        current_step = &(step_list_info->step_list_ptr[step_index]);
        current_step_id = current_step->step_id;

        if (flags & BCM_INIT_ADVANCED_F_MEM_LEAK_DETECT)
        {
            
            total_memory_alocation_end = 0;
        }

        /*
         * At deinit, the only reason to run the managment logic is inorder to switch the step_list_ptr to the sub-list 
         */
        SHR_IF_ERR_EXIT(dnx_init_managment_logic_deinit
                        (unit, &step_list_info_internal, current_step->step_flags, current_step->step_sub_array));
        /*
         * If list - deinit list steps
         */
        if (current_step->step_sub_array != NULL)
        {
            /*
             * Log info
             */
            LOG_INFO_EX(BSL_LOG_MODULE, "DEINIT: %s%s%s%s", EMPTY, EMPTY, EMPTY, EMPTY);
            for (depth_index = 0; depth_index < step_list_info->depth; depth_index++)
            {
                LOG_INFO_EX(BSL_LOG_MODULE, "    - %s%s%s%s", EMPTY, EMPTY, EMPTY, EMPTY);
            }
            LOG_INFO_EX(BSL_LOG_MODULE, "%s: \n%s%s%s", dnx_init_step_str_name[current_step_id], EMPTY, EMPTY, EMPTY);

            /*
             * Run deinit list 
             */
            SHR_IF_ERR_EXIT(dnx_init_run_step_list_deinit(unit, &step_list_info_internal, flags));
        }

        /*
         * Run Step DEINIT function if exists 
         */
        if (current_step->deinit_function != NULL)
        {
            /*
             * Log info
             */
            LOG_INFO_EX(BSL_LOG_MODULE, "DEINIT: %s%s%s%s", EMPTY, EMPTY, EMPTY, EMPTY);
            for (depth_index = 0; depth_index < step_list_info->depth; depth_index++)
            {
                LOG_INFO_EX(BSL_LOG_MODULE, "    - %s%s%s%s", EMPTY, EMPTY, EMPTY, EMPTY);
            }
            LOG_INFO_EX(BSL_LOG_MODULE, "%s:\n%s%s%s", dnx_init_step_str_name[current_step_id], EMPTY, EMPTY, EMPTY);

            /*
             * Run deinit function 
             */
            SHR_IF_ERR_EXIT(current_step->deinit_function(unit));
        }
        if (flags & BCM_INIT_ADVANCED_F_MEM_LEAK_DETECT)
        {
            
            total_memory_alocation_start = 0;
            memory_alocation_during_init =
                step_list_info->meta_data_arr[current_step_id].total_memory_alocation_end -
                step_list_info->meta_data_arr[current_step_id].total_memory_alocation_start;
            memory_freed_during_deinit = total_memory_alocation_end - total_memory_alocation_start;
            if (memory_freed_during_deinit != memory_alocation_during_init)
            {
                SHR_ERR_EXIT(_SHR_E_MEMORY, "Memory leak detected in step %s, terminating driver\n",
                             dnx_init_step_str_name[current_step_id]);
            }
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Internal Init function.
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   \param [in] flags - BCM_INIT_ADVANCED_F_...
 *   \param [in] dnx_init_info - additional user information for init sequence, should be kept minimal.
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   This abstraction was requiered due to the need to maintain backwards compatability
 *   with older BCM API but provide option for new BCM API to get flags and user info.
 * \see
 *   * None
 */
static shr_error_e
dnx_init_internal_init(
    int unit,
    int flags,
    dnx_init_info_t * dnx_init_info)
{
    int first_is_done = 0;
    int last_is_done = 0;
    dnx_init_step_meta_data_t steps_meta_data[DNX_INIT_STEP_NUM_OF_STEPS];
    dnx_init_step_list_info_t step_list_info;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Verify info - make sure no contredicting flags co-exist 
     */
    SHR_IF_ERR_EXIT(dnx_init_internal_verify(unit, flags, dnx_init_info));

    /*
     * Initialize meta data array to zero 
     */
    sal_memset(steps_meta_data, 0, sizeof(steps_meta_data));

    /*
     * Init step_list_info 
     */
    step_list_info.first_step = dnx_init_info->first_step;
    step_list_info.last_step = dnx_init_info->last_step;;
    step_list_info.step_list_ptr = dnx_init_deinit_sequence;
    step_list_info.meta_data_arr = steps_meta_data;
    step_list_info.first_is_done_ptr = &first_is_done;
    step_list_info.last_is_done_ptr = &last_is_done;
    step_list_info.depth = 0;
    /*
     * if not doing partial init, or if doing partial init with defining only last step 
     */
    if (((flags & BCM_INIT_ADVANCED_F_PARTIAL_INIT) == 0) || (dnx_init_info->first_step == DNX_INIT_STEP_INVALID_STEP))
    {
        /*
         * mark first is done 
         */
        first_is_done = 1;
    }

    SHR_IF_ERR_EXIT(dnx_init_run_step_list_init(unit, &step_list_info, flags));

    /*
     * Copy time stamps and mem_leak info to memory 
     */
    if (flags & (BCM_INIT_ADVANCED_F_MEM_LEAK_DETECT | BCM_INIT_ADVANCED_F_TIME_STAMP))
    {
        
    }

    if (flags & BCM_INIT_ADVANCED_F_MEM_LEAK_DETECT)
    {
        
    }

    if (flags & BCM_INIT_ADVANCED_F_TIME_STAMP)
    {
        
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Internal Deinit function
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
static shr_error_e
dnx_init_internal_deinit(
    int unit)
{
    int flags = 0;
    dnx_init_step_list_info_t step_list_info;
    dnx_init_step_meta_data_t steps_meta_data[DNX_INIT_STEP_NUM_OF_STEPS];

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Initialize meta data array to zero 
     */
    sal_memset(steps_meta_data, 0, sizeof(steps_meta_data));

    /*
     * Copy from memory information regarding memory leak and set flag accordingly 
     */
    if (0  )
    {
        

        /*
         * set mem leak flag 
         */
        flags |= BCM_INIT_ADVANCED_F_MEM_LEAK_DETECT;
    }

    /*
     * Init step_list_info 
     */
    step_list_info.first_step = DNX_INIT_STEP_INVALID_STEP;
    step_list_info.last_step = DNX_INIT_STEP_INVALID_STEP;
    step_list_info.step_list_ptr = dnx_init_deinit_sequence;
    step_list_info.meta_data_arr = steps_meta_data;
    step_list_info.first_is_done_ptr = NULL;
    step_list_info.last_is_done_ptr = NULL;
    step_list_info.depth = 0;

    SHR_IF_ERR_EXIT(dnx_init_run_step_list_deinit(unit, &step_list_info, flags));

exit:
    SHR_FUNC_EXIT;
}

/*
 * IMPORTANT:
 *   The code here is empty and meaningless.
 *   It will probably need to be rewritten!
 *   See bcm/dpp/init.c (_bcm_petra_attach) as example.
 *  
 *   those are probably the minimal steps needed, this should be done, need to verify
 */
int
_bcm_dnx_attach(
    int unit,
    char *subtype)
{
    int dunit;
    SHR_FUNC_INIT_VARS(unit);

    BCM_CONTROL(unit)->capability |= BCM_CAPA_LOCAL;
    dunit = BCM_CONTROL(unit)->unit;
    if (SOC_UNIT_VALID(dunit))
    {
        BCM_CONTROL(unit)->chip_vendor = SOC_PCI_VENDOR(dunit);
        BCM_CONTROL(unit)->chip_device = SOC_PCI_DEVICE(dunit);
        BCM_CONTROL(unit)->chip_revision = SOC_PCI_REVISION(dunit);
        BCM_CONTROL(unit)->capability |= BCM_CAPA_SWITCH;
    }

    SHR_EXIT();
exit:
    SHR_FUNC_EXIT;
}

int
dnx_info_get_verify(
    int unit,
    bcm_info_t * info)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(info, _SHR_E_PARAM, "info");

exit:
    SHR_FUNC_EXIT;
}

int
bcm_dnx_info_get(
    int unit,
    bcm_info_t * info)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_info_get_verify(unit, info));

    info->vendor = SOC_PCI_VENDOR(unit);
    info->device = SOC_PCI_DEVICE(unit);
    info->revision = SOC_PCI_REVISION(unit);
    info->capability = 0;

    

exit:
    SHR_FUNC_EXIT;
}

int
_bcm_dnx_detach(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);
    SHR_IF_ERR_EXIT(dnx_init_internal_deinit(unit));

exit:
    SHR_FUNC_EXIT;
}

int
_bcm_dnx_match(
    int unit,
    char *subtype_a,
    char *subtype_b)
{
    int r_rv;
    r_rv = _SHR_E_UNAVAIL;
    return r_rv;
}
