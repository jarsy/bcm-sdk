/*
 * $Id: dfe_port.c,v 1.13 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC DCMN CMICD IPROC
 */

#ifndef _SOC_DCMN_WB_H_
#define _SOC_DCMN_WB_H_

/**********************************************************/
/*                  Includes                              */
/**********************************************************/
#include <soc/defs.h>
#include <soc/drv.h>
#include <shared/bsl.h>

#include <soc/dcmn/dcmn_defs.h>

#ifdef BCM_WARM_BOOT_API_TEST
#include <appl/diag/shell.h>
#endif

/**********************************************************/
/*                  Defines                               */
/**********************************************************/

#define SOC_WB_API_RESET_ERR_CHECK(disp_rv)\
    if (disp_rv != SOC_E_NONE) {return SOC_E_INTERNAL;}

/* used in field tests */
#ifdef BCM_WARM_BOOT_API_TEST
#define WB_TEST(op) op
#else
#define WB_TEST(op)
#endif

#define _DCMN_BCM_WARM_BOOT_API_TEST_MODE_NONE                          (0) /*don't perform any test*/
#define _DCMN_BCM_WARM_BOOT_API_TEST_MODE_AFTER_EVERY_API               (1) /*perform wb test at end of every api call*/
#define _DCMN_BCM_WARM_BOOT_API_TEST_MODE_END_OF_DVAPIS                 (2) /*perform wb test at end of every dvapi test*/
#define _DCMN_BCM_WARM_BOOT_API_TEST_MODE_CRASH_RECOVERY                (3) /*perform cr test at end of every api call*/


/* in the first DCMN_WARM_BOOT_MIN_DONT_SKIP calls, don't skip the reset sequence */
#define DCMN_WARM_EXP_PARAM 10

/* 
 * field override wb test mode 
 */ 
typedef enum _dcmn_wb_field_falgs_s {
    _dcmnWbFieldFlagDirectExtraction=0,
    _dcmnWbFieldFlagDataQual=1, 
    _dcmnWbFieldFlagNOF=2
} _dcmn_wb_field_falgs_e;

/* general flag, indication whether we are in WB test mode or not */
extern int    _soc_dcmn_wb_warmboot_test_mode[SOC_MAX_NUM_DEVICES];
/* temporary disabling WB test mode */
extern int    _soc_dcmn_wb_override_wb_test[SOC_MAX_NUM_DEVICES];
/* disabling WB test mode for one BCM API call */
extern int    _soc_dcmn_wb_disable_once_wb_test[SOC_MAX_NUM_DEVICES];

/**********************************************************/
/*                  Functions                             */
/**********************************************************/


#ifdef BCM_WARM_BOOT_API_TEST
extern void soc_dcmn_wb_test_mode_set(int unit, int mode);
extern void soc_dcmn_wb_test_mode_get(int unit, int *mode);
extern void soc_dcmn_wb_no_wb_test_set(int unit, int wb_flag);
extern void soc_dcmn_wb_no_wb_test_get(int unit, int *wb_flag);
extern void soc_dcmn_wb_field_test_mode_set(int unit, _dcmn_wb_field_falgs_e type, int wb_flag);
extern void soc_dcmn_wb_field_test_mode_get(int unit, _dcmn_wb_field_falgs_e type, int *wb_flag);
extern void soc_dcmn_wb_disable_once_set(int unit, int wb_flag);
extern void soc_dcmn_wb_disable_once_get(int unit, int *wb_flag);
extern void soc_dcmn_wb_test_counter_set(int unit, int counter);
extern void soc_dcmn_wb_test_counter_get(int unit, int *counter);
extern void soc_dcmn_wb_test_counter_plus_plus(int unit);
extern void dcmn_bcm_warm_boot_api_test_enable_if_disabled_once(int unit);
extern void soc_dcmn_wb_test_counter_reset(int unit);
extern int  dcmn_bcm_warm_boot_api_test_mode_skip_wb_sequence(int unit);
extern int  dcmn_wb_exp_backoff(int test_counter, int *nof_skip);
extern int dcmn_wb_cr_all_reset_test_utils_preconditions(int unit);
extern int dcmn_wb_cr_common_api_preconditions(int unit);
extern int dcmn_wb_api_counter_increase(int unit);
extern int dcmn_wb_api_counter_decrease(int unit);
extern int dcmn_wb_api_counter_get(int unit);
extern int dcmn_wb_api_counter_reset(int unit);
extern int dcmn_wb_api_counter_is_top_level(int unit);
#endif
/*************************************************************************************************/



#ifdef BCM_DPP_SUPPORT
#include <bcm_int/petra_dispatch.h>
#else /* !BCM_DPP_SUPPORT */
#ifdef BCM_DFE_SUPPORT
#include <bcm_int/dfe_dispatch.h>
#endif /* BCM_DFE_SUPPORT */
#endif /* BCM_DPP_SUPPORT */

#define TR_141_COMMAND(is_cr) ((is_cr) ? "tr 141 w=1 NoSync=1 NoDump=1" : "tr 141 W=1 NoDump=2")

#ifdef BCM_WARM_BOOT_API_TEST

int dcmn_bcm_warm_boot_api_test_reset(int unit);

/* WB_TEST_MODE_SKIP_WB_SEQUENCE flag disable the WB test mode for current BCM API call.
 * i.e - if test mode is on (i.e - perform warm rebooot at the end of APIs), when turning this
 * flag on the driver will NOT perform WB sequence when the BCM API finish to run. 
 * for example - to be used by APIs that create a mismatch between SW state and HW state
 * (like in the field instructions that are saved to SW but not yet commited to HW) 
 * also used to exclude some not interesting _get APIs from wb regression */
#define _DCMN_BCM_WARM_BOOT_API_TEST_MODE_SKIP_WB_SEQUENCE(unit)        \
    dcmn_bcm_warm_boot_api_test_mode_skip_wb_sequence(unit)


/* OVERRIDE_WB_TEST_MODE          - temporary disable WB test mode */ 
/* RETRACT_OVERRIDEN_WB_TEST_MODE - re-enable WB test mode */ 
/* for example - can be used by BCM APIs that internally call other BCM APIs after which 
   WB sequence should not be called 
   NOTE: !!! I'ts the caller responsability to call RETRACT for every OVERIDE call!!!
         !!! there is an internal counter here, nof retracts should equal nof ovrides !!! */ 
#define _DCMN_BCM_WARM_BOOT_API_TEST_OVERRIDE_WB_TEST_MODE(unit)        \
    soc_dcmn_wb_no_wb_test_set(unit, 1)
#define _DCMN_BCM_WARM_BOOT_API_TEST_RETRACT_OVERRIDEN_WB_TEST_MODE(unit)\
    soc_dcmn_wb_no_wb_test_set(unit, 0)


#define _DCMN_BCM_WARM_BOOT_API_TEST_FIELD_DIR_EXT_WB_TEST_MODE_STOP(unit);\
    soc_dcmn_wb_field_test_mode_set(unit, _dcmnWbFieldFlagDirectExtraction, 1)
#define _DCMN_BCM_WARM_BOOT_API_TEST_FIELD_DIR_EXT_WB_TEST_MODE_CONTINUE(unit);\
    soc_dcmn_wb_field_test_mode_set(unit, _dcmnWbFieldFlagDirectExtraction, 0)
#define _DCMN_BCM_WARM_BOOT_API_TEST_FIELD_QUAL_WB_TEST_MODE_STOP(unit);\
    soc_dcmn_wb_field_test_mode_set(unit, _dcmnWbFieldFlagDataQual, 1)
#define _DCMN_BCM_WARM_BOOT_API_TEST_FIELD_QUAL_WB_TEST_MODE_CONTINUE(unit);\
    soc_dcmn_wb_field_test_mode_set(unit, _dcmnWbFieldFlagDataQual, 0)
#define _DCMN_BCM_WARM_BOOT_API_TEST_MODE_SKIP_WB_SEQUENCE_NO_SANITY_CHECK(unit)        \
    dcmn_wb_api_counter_increase(unit);\
    _DCMN_BCM_WARM_BOOT_API_TEST_MODE_SKIP_WB_SEQUENCE(unit);\
    dcmn_wb_api_counter_decrease(unit)

#else /* !BCM_WARM_BOOT_API_TEST */


#define _DCMN_BCM_WARM_BOOT_API_TEST_OVERRIDE_WB_TEST_MODE(unit)            do {} while(0)
#define _DCMN_BCM_WARM_BOOT_API_TEST_RETRACT_OVERRIDEN_WB_TEST_MODE(unit)   do {} while(0)
/* in case it is not DPP WB TEST defining the Macro to do nothing */
#define _DCMN_BCM_WARM_BOOT_API_TEST_MODE_SKIP_WB_SEQUENCE(unit)            do {} while(0)

#define _DCMN_BCM_WARM_BOOT_API_TEST_FIELD_DIR_EXT_WB_TEST_MODE_STOP(unit)       do {} while(0)
#define _DCMN_BCM_WARM_BOOT_API_TEST_FIELD_DIR_EXT_WB_TEST_MODE_CONTINUE(unit)   do {} while(0)
#define _DCMN_BCM_WARM_BOOT_API_TEST_FIELD_QUAL_WB_TEST_MODE_STOP(unit)          do {} while(0)
#define _DCMN_BCM_WARM_BOOT_API_TEST_FIELD_QUAL_WB_TEST_MODE_CONTINUE(unit)      do {} while(0)
#define _DCMN_BCM_WARM_BOOT_API_TEST_MODE_SKIP_WB_SEQUENCE_NO_SANITY_CHECK(unit)        \
    _DCMN_BCM_WARM_BOOT_API_TEST_MODE_SKIP_WB_SEQUENCE(unit)

#endif /* BCM_WARM_BOOT_API_TEST */


#endif /*!_SOC_DCMN_WB_H_*/

