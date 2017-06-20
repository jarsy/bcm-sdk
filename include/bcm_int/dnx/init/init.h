/** \file bcm_int/dnx/init/init.h
 * 
 * Internal DNX INIT APIs 
 * 
 */

/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _BCMINT_DNX_INIT_INIT_H_INCLUDED
/*
 * { 
 */
#define _BCMINT_DNX_INIT_INIT_H_INCLUDED

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

#include <shared/shrextend/shrextend_debug.h>
#include <sal/core/time.h>


/**
 * Memory Leak detection Activated.
 */
#define BCM_INIT_ADVANCED_F_MEM_LEAK_DETECT   0x1
/**
 * Time Stamp saving Activated.
 */
#define BCM_INIT_ADVANCED_F_TIME_STAMP        0x2
/**
 * Partial Init Activated.
 */
#define BCM_INIT_ADVANCED_F_PARTIAL_INIT      0x4

/*
 * Dynamic or internal flag which are recieved by the flag CB. developer can use them to write his flag CBs.
 * when inserting a new sub-list to the init sequence the STEP_IS_LIST flag should be used. in each flag CB the developer
 * is expected to use the SKIP flag according to a logic which is internal to his module, the flag CB should assign those
 * flags according to SOC properties (for example if soc property trunk_disable is set, the CB should set the skip flag to dynamic_flags).
 */
/**
 * Skip step
 */
#define DNX_INIT_STEP_F_SKIP_DYNAMIC                  0x1
/**
 * Skip Step due to WB
 */
#define DNX_INIT_STEP_F_WB_SKIP_DYNAMIC               0x2

/**
 * \brief Describes the IDs for the different steps and 
 *        sub-steps of the Init. each entry here needs to be
 *        aligned with DNX_INIT_STEP_NAME_STR below.
 */
typedef enum
{
    /**
     * Invalid Value, used to indicate to APIs that this input is
     * not relevant
     */
    DNX_INIT_STEP_INVALID_STEP = 0,
    /**
     * Sub-list containing all TM modules 
     */
    DNX_INIT_STEP_TM_MODULES = 1,
    /**
     * Sub-list containing all PP modules 
     */
    DNX_INIT_STEP_PP_MODULES = 2,
    /**
     * Init Schan
     */
    DNX_INIT_STEP_SCHAN = 3,
    /**
     * Init c model reg access
     */
    DNX_INIT_STEP_CMODEL_REG_ACC = 4,
    /**
     * Init device feature list
     */
    DNX_INIT_STEP_FEATURE = 5,
    /**
     * Init Info config 
     */
    DNX_INIT_STEP_INFO_CONFIG = 6,
    /**
     * Init contorl structures. 
     */
    DNX_INIT_STEP_CONTROL = 7,
    /**
     * Init needed mutexes. 
     */
    DNX_INIT_STEP_MUTEXES = 8,
    /**
     * Init memory mutexes. 
     */
    DNX_INIT_STEP_MEM_MUTEX = 9,
    /**
     * Init the dnx_defines - legacy step should be removed in the future. 
     */
    DNX_INIT_STEP_DEFINES_INIT = 10,
    /**
     * Sub-list containing all Legacy Soc Steps, 
     * eventually this should be empty and canceled. 
     */
    DNX_INIT_STEP_LEGACY_SOC = 11,
    /**
     * Init Dnx Data. 
     */
    DNX_INIT_STEP_DNX_DATA = 12,
    /**
     * Init DBAL. 
     */
    DNX_INIT_STEP_DBAL = 13,
    /**
     * Init Common Modules. 
     */
    DNX_INIT_STEP_COMMON_MODULES = 14,
    /**
     * Init RX. 
     */
    DNX_INIT_STEP_RX = 15,
    /**
     * Init Algo Res. 
     */
    DNX_INIT_STEP_ALGO_RES = 16,
    /**
     * Restore Soc Control.
     */
    DNX_INIT_STEP_RESTORE_SOC_CONTROL = 17,
    /**
     * Init utils and sw state - legacy step, should be removed once
     * real step is done.
     */
    DNX_INIT_STEP_UTILS_SW_STATE = 18,
    /**
     * init Done step, used to update relevant modules that the init
     * was done
     */
    DNX_INIT_STEP_INIT_DONE = 19,
    /**
     * init access step list
     */
    DNX_INIT_STEP_ACCESS = 20,
    /**
     * init Vlan Algo Res
     */
    DNX_INIT_STEP_VLAN_ALGO_RES = 21,
    /**
     * Init algo template.
     */
    DNX_INIT_STEP_ALGO_TEMPLATE = 22,
    /**
     * init HW overwrites
     */
    DNX_INIT_STEP_HW_OVERWRITE = 23,
    /**
     * init PEMLA
     */
    DNX_INIT_STEP_PEMLA = 24,
    /**
     * Init Utilties modules
     */
    DNX_INIT_STEP_UTILITIES = 25,
    /**
     * Init Algo modules
     */
    DNX_INIT_STEP_ALGO = 26,
    /**
     * Per module step that overrides HW defaults (regs and mems)
     */
    DNX_INIT_STEP_DEFAULTS = 27,
    /**
     * Legacy soc modules list
     */
    DNX_INIT_STEP_SOC_MODULES = 28,
    /**
     * init L3 Algo Res
     */
    DNX_INIT_STEP_L3_ALGO = 29,
    /**
     * init L3 module
     */
    DNX_INIT_STEP_L3_MODULE = 30,
    /**
     * init MIRROR module
     */
    DNX_INIT_STEP_MIRROR_MODULE = 31,
    /** 
     * Init LIF module.
     */
    DNX_INIT_STEP_LIF = 32,
    /** 
     * Init LIF module.
     */
    DNX_INIT_STEP_ALGO_MIRROR = 33,
    /** 
     * Appl properties - used to set properties values through APIs.
     */
    DNX_INIT_STEP_APPL_PROPERTIES = 34,
    /**
     * init PP PORT module
     */
    DNX_INIT_STEP_PP_PORT = 35,
    /**
     * init misc. access data structures - signals, regs/mems mcm ad-ons
     */
    DNX_INIT_STEP_AUX_ACCESS = 36,
    /**
     * Dummy step, used to indicate the last step for a step list.
     */
    DNX_INIT_STEP_LAST_STEP,
    /**
     * Number of step IDs
     */
    DNX_INIT_STEP_NUM_OF_STEPS
} dnx_init_step_id_e;

/**
 * \brief Const array to provide string format for each step name for printing purpuses.
 *
 * The Indexes in this array need to be alligned to the Enum \ref dnx_init_step_id_e
 */
#define DNX_INIT_STEP_NAME_STR              \
{                                           \
    "Invalid",                              \
    "TM Modules",                           \
    "PP Modules",                           \
    "Schan",                                \
    "C-Model",                              \
    "Feature",                              \
    "Info Config",                          \
    "Soc Control",                          \
    "Mutexes",                              \
    "Memory Mutexes",                       \
    "Defines",                              \
    "Legacy Soc",                           \
    "DNX Data",                             \
    "DBAL",                                 \
    "Common Modules",                       \
    "RX",                                   \
    "Algo Res",                             \
    "Restore Soc Control",                  \
    "Utils and SW State",                   \
    "Init Done",                            \
    "Access",                               \
    "Vlan Algo Res",                        \
    "Algo Template",                        \
    "HW Overwrites",                        \
    "PEMLA",                                \
    "Utilities",                            \
    "Algo",                                 \
    "Defaults",                             \
    "SOC Modules",                          \
    "L3 Algo",                              \
    "L3",                                   \
    "Mirror",                               \
    "Lif",                                  \
    "Algo Mirror",                          \
    "Appl Properties",                      \
    "PP PORT",                              \
    "Aux Access&Signals",                   \
    "Last Step",                            \
    "Number of steps"                       \
}

/** 
 * \brief ptr to step cb function returning shr_error_e, to be 
 * used in each init/deinit step. the step_list_info and the 
 * flags are used for internal purposes, and are managed by the 
 * mechanism. 
 */
typedef shr_error_e(
    *dnx_step_cb) (
    int unit);

/**
 * \brief ptr to step flag cb function returning 
 * shr_error_e, to be used in each init step to decode flags 
 * according to a given step logic ( for example Step X should 
 * be skipped if SOC property Y is set to value Z etc). to do 
 * so, one would need to return in dynamic flags the 
 * DNX_INIT_STEP_F_SKIP_DYNAMIC flag. 
 */
typedef shr_error_e(
    *dnx_step_flag_cb) (
    int unit,
    int *dynamic_flags);

/**
 * \brief dnx init arguments structure.
 *
 * This structure should contain user's information.
 * most of the needed info however should be passed as SOC properties. 
 * currently it is used to define a stopping and continuation points 
 */
typedef struct
{
  /**
   * this is used to start the init sequence from a certain step.
   * this will be used after stoping the init sequence with last
   * step. the default for this should be the 
   * DNX_INIT_STEP_INVALID_STEP step id. 
   */
    dnx_init_step_id_e first_step;

  /**
   * this is used to stop the init sequence at a certain step. 
   * to continue use the init function with first step defined as 
   * next step of this. the default for this should be the 
   * DNX_INIT_STEP_INVALID_STEP step id. 
   */
    dnx_init_step_id_e last_step;
} dnx_init_info_t;

/*
 * } 
 */
#endif /* _BCMINT_DNX_INIT_INIT_H_INCLUDED */
