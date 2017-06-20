/*
 * $Id: ramon_stack.h,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * RAMON STACK H
 */
 
#ifndef _SOC_RAMON_STACK_H_
#define _SOC_RAMON_STACK_H_

#include <soc/error.h>
#include <soc/dnxf/cmn/dnxf_stack.h>
#include <soc/defs.h>

#define SOC_RAMON_STK_MAX_MODULE (2047)
#define SOC_RAMON_STK_FAP_GROUP_SIZE (32)
#define SOC_RAMON_STK_MAX_IGNORED_FAP_IDS (16)

soc_error_t soc_ramon_stk_modid_set(int unit, uint32 fe_id);
soc_error_t soc_ramon_stk_modid_get(int unit, uint32* fe_id);
soc_error_t soc_ramon_stk_module_max_all_reachable_verify(int unit, bcm_module_t module);
soc_error_t soc_ramon_stk_module_max_all_reachable_set(int unit, bcm_module_t module);
soc_error_t soc_ramon_stk_module_max_all_reachable_get(int unit, bcm_module_t *module);
soc_error_t soc_ramon_stk_module_max_fap_verify(int unit, bcm_module_t module);
soc_error_t soc_ramon_stk_module_max_fap_set(int unit, bcm_module_t module);
soc_error_t soc_ramon_stk_module_max_fap_get(int unit, bcm_module_t *module);
soc_error_t soc_ramon_stk_module_all_reachable_ignore_id_set(int unit, bcm_module_t module, int arg);
soc_error_t soc_ramon_stk_module_all_reachable_ignore_id_get(int unit, bcm_module_t module, int *arg);
soc_error_t soc_ramon_stk_valid_module_id_verify(int unit, bcm_module_t module, int verify_divisble_by_32);

#endif

