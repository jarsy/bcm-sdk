/*
* $Id: dcmn_gen_err.h,v 1.0 Broadcom SDK $
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
*
* This file includes the generation of fake error for partial init.
*/

#ifndef _SOC_DCMN_GEN_ERR_H
#define _SOC_DCMN_GEN_ERR_H

#include <soc/defs.h>
#include <soc/error.h>
#include <sal/core/sync.h>

#define GEN_ERR_NO_UNIT    (-1)

/* 
 * generate error test type
 * 
 * Generate the "fake" error on init, deinit or init & deinit sequence.
 */
typedef enum {
    GEN_ERR_TEST_NONE      = 0,
    GEN_ERR_TEST_INIT      = 1,
    GEN_ERR_TEST_DEINIT    = 2,
    GEN_ERR_TEST_ALL       = 3,
    GEN_ERR_TEST_CLEANUP   = 4,
    GEN_ERR_TEST_NUMS      = 5
} _gen_err_test_t;

/* 
 * generate error type
 * 
 * Generate the "fake" error on soc or bcm layer.
 */
typedef enum {
    GEN_ERR_TYPE_SOC       = 0,
    GEN_ERR_TYPE_BCM       = 1,
    GEN_ERR_TYPE_NUMS      = 2
} _gen_err_type_t;

#if defined(BCM_SAND_SUPPORT) && defined(BCM_GEN_ERR_MECHANISM)
/* 
 * "fake" generation test setup to create the mutex.
 */
#define GEN_ERR_TEST_SETUP(_unit) \
    do { \
        if ((unit != GEN_ERR_NO_UNIT) && (gen_err_lock[_unit] == NULL)) { \
            gen_err_lock[_unit] = sal_mutex_create("gen_err_lock"); \
        } \
    } while(0)

/* 
 * "fake" generation test cleanup to destory the mutex.
 */
#define GEN_ERR_TEST_CLEANUP(_unit) \
    do { \
        if ((unit != GEN_ERR_NO_UNIT) && gen_err_lock[_unit]) { \
            sal_mutex_destroy(gen_err_lock[_unit]); \
            gen_err_lock[_unit] = NULL; \
        } \
	} while(0)

/* 
 * "fake" generation test set to set the "fake" generation in init, deinit or init & deinit.
 */
#define GEN_ERR_TEST_SET(_unit, _test) \
    do { \
        if ((unit != GEN_ERR_NO_UNIT) && gen_err_lock[_unit]) { \
            sal_mutex_take(gen_err_lock[_unit], sal_mutex_FOREVER); \
            gen_err_test[_unit] = _test; \
            sal_mutex_give(gen_err_lock[_unit]); \
        } \
    } while(0)

/* 
 * "fake" generation run set to set the current init or deinit stage.
 */
#define GEN_ERR_RUN_SET(_unit, _stage) \
    do { \
        if ((unit != GEN_ERR_NO_UNIT) && gen_err_lock[_unit]) { \
            sal_mutex_take(gen_err_lock[_unit], sal_mutex_FOREVER); \
            gen_err_running[_unit] = _stage; \
            sal_mutex_give(gen_err_lock[_unit]); \
        } \
    } while(0)

/* 
 * "fake" generation compare set to set the "fake" generation compare value.
 */
#define GEN_ERR_CMP_SET(_unit, _type, _cmp) \
    do { \
        if ((unit != GEN_ERR_NO_UNIT) && gen_err_lock[_unit]) { \
            sal_mutex_take(gen_err_lock[_unit], sal_mutex_FOREVER); \
            gen_err_cmp[_unit][_type] = _cmp; \
            sal_mutex_give(gen_err_lock[_unit]); \
        } \
    } while(0)
   
/* 
 * "fake" generation compare set to set the "fake" generation current value.
 */ 
#define GEN_ERR_CNT_SET(_unit, _type, _cnt) \
    do { \
        if ((unit != GEN_ERR_NO_UNIT) && gen_err_lock[_unit]) { \
            sal_mutex_take(gen_err_lock[_unit], sal_mutex_FOREVER); \
            gen_err_cnt[_unit][_type] = _cnt; \
            sal_mutex_give(gen_err_lock[_unit]); \
        } \
    } while(0)
     
/* 
 * "fake" generation check to check whether to generate the "fake" error.
 */
#define GEN_ERR_CHK(_unit, _type, _rv) \
    do { \
        static int gen_err_has_checked[SOC_MAX_NUM_DEVICES] = { 0 }; \
        if ((unit != GEN_ERR_NO_UNIT) && gen_err_lock[_unit]) { \
            sal_mutex_take(gen_err_lock[_unit], sal_mutex_FOREVER); \
            if ((gen_err_running[_unit] & gen_err_test[_unit]) != 0) { \
                if (_rv != SOC_E_NONE) { \
                    sal_mutex_give(gen_err_lock[_unit]); \
                    break; \
                } \
                if (gen_err_cmp[_unit][_type] != 0) { \
                    if (!gen_err_has_checked[_unit]) { \
                        gen_err_has_checked[_unit] = 1; \
                        if (++gen_err_cnt[_unit][_type] == gen_err_cmp[_unit][_type]) { \
                            _rv = SOC_E_INIT; \
                            gen_err_cmp[_unit][_type] = 0; \
                        } \
                    } \
                } \
            } else if (gen_err_test[_unit] == GEN_ERR_TEST_CLEANUP) { \
                gen_err_has_checked[_unit] = 0; \
            } \
            sal_mutex_give(gen_err_lock[_unit]); \
        } \
    } while(0)

extern sal_mutex_t gen_err_lock[SOC_MAX_NUM_DEVICES];
extern int gen_err_test[SOC_MAX_NUM_DEVICES];
extern int gen_err_running[SOC_MAX_NUM_DEVICES];
extern int gen_err_cmp[SOC_MAX_NUM_DEVICES][GEN_ERR_TYPE_NUMS];
extern int gen_err_cnt[SOC_MAX_NUM_DEVICES][GEN_ERR_TYPE_NUMS]; 
#else
#define GEN_ERR_TEST_SET(_unit, _test);
#define GEN_ERR_RUN_SET(_unit, _stage);
#define GEN_ERR_CMP_SET(_unit, _type, _cmp);
#define GEN_ERR_CNT_SET(_unit, _type, _cnt);
#define GEN_ERR_CHK(_unit, _type, _rv);
#endif

#endif /*_SOC_DCMN_GEN_ERR_H*/

