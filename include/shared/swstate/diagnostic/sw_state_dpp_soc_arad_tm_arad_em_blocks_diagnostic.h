/* $Id: $
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * 
 * DO NOT EDIT THIS FILE!
 * This file is auto-generated.
 * Edits to this file will be lost when it is regenerated.
 * search for 'sw_state_diagnostic_cbs_t' for the root of the struct
 */

#ifndef _SHR_SW_STATE_DPP_SOC_ARAD_TM_ARAD_EM_BLOCKS_DIAGNOSTIC_H_
#define _SHR_SW_STATE_DPP_SOC_ARAD_TM_ARAD_EM_BLOCKS_DIAGNOSTIC_H_

#ifdef BCM_WARM_BOOT_API_TEST
/********************************* diagnostic calbacks definitions *************************************/
/* this set of callbacks, are the callbacks used in the diagnostic calbacks struct 'sw_state_cbs_t' to */
/* access the data in 'sw_state_t'.                                                                */
/* the calbacks are inserted into the diagnostic struct by 'sw_state_diagnostic_cb_init'.                  */
/***************************************************************************************************/

#if defined(BCM_PETRA_SUPPORT)
#ifdef BCM_PETRA_SUPPORT
/* implemented by: sw_state_dpp_soc_arad_tm_arad_em_blocks_dump */
typedef int (*sw_state_dpp_soc_arad_tm_arad_em_blocks_dump_cb)(
    int unit, int tm_idx_0);
#endif /* BCM_PETRA_SUPPORT*/ 
#endif /* defined(BCM_PETRA_SUPPORT)*/ 

#if defined(BCM_PETRA_SUPPORT)
#ifdef BCM_PETRA_SUPPORT
/* implemented by: sw_state_dpp_soc_arad_tm_arad_em_blocks_read_result_address_dump */
typedef int (*sw_state_dpp_soc_arad_tm_arad_em_blocks_read_result_address_dump_cb)(
    int unit, int tm_idx_0, int arad_em_blocks_idx_0);
#endif /* BCM_PETRA_SUPPORT*/ 
#endif /* defined(BCM_PETRA_SUPPORT)*/ 

#if defined(BCM_PETRA_SUPPORT)
#ifdef BCM_PETRA_SUPPORT
/* implemented by: sw_state_dpp_soc_arad_tm_arad_em_blocks_offset_dump */
typedef int (*sw_state_dpp_soc_arad_tm_arad_em_blocks_offset_dump_cb)(
    int unit, int tm_idx_0, int arad_em_blocks_idx_0);
#endif /* BCM_PETRA_SUPPORT*/ 
#endif /* defined(BCM_PETRA_SUPPORT)*/ 

#if defined(BCM_PETRA_SUPPORT)
#ifdef BCM_PETRA_SUPPORT
/* implemented by: sw_state_dpp_soc_arad_tm_arad_em_blocks_table_size_dump */
typedef int (*sw_state_dpp_soc_arad_tm_arad_em_blocks_table_size_dump_cb)(
    int unit, int tm_idx_0, int arad_em_blocks_idx_0);
#endif /* BCM_PETRA_SUPPORT*/ 
#endif /* defined(BCM_PETRA_SUPPORT)*/ 

#if defined(BCM_PETRA_SUPPORT)
#ifdef BCM_PETRA_SUPPORT
/* implemented by: sw_state_dpp_soc_arad_tm_arad_em_blocks_key_size_dump */
typedef int (*sw_state_dpp_soc_arad_tm_arad_em_blocks_key_size_dump_cb)(
    int unit, int tm_idx_0, int arad_em_blocks_idx_0);
#endif /* BCM_PETRA_SUPPORT*/ 
#endif /* defined(BCM_PETRA_SUPPORT)*/ 

#if defined(BCM_PETRA_SUPPORT)
#ifdef BCM_PETRA_SUPPORT
/* implemented by: sw_state_dpp_soc_arad_tm_arad_em_blocks_data_nof_bytes_dump */
typedef int (*sw_state_dpp_soc_arad_tm_arad_em_blocks_data_nof_bytes_dump_cb)(
    int unit, int tm_idx_0, int arad_em_blocks_idx_0);
#endif /* BCM_PETRA_SUPPORT*/ 
#endif /* defined(BCM_PETRA_SUPPORT)*/ 

#if defined(BCM_PETRA_SUPPORT)
#ifdef BCM_PETRA_SUPPORT
/* implemented by: sw_state_dpp_soc_arad_tm_arad_em_blocks_start_address_dump */
typedef int (*sw_state_dpp_soc_arad_tm_arad_em_blocks_start_address_dump_cb)(
    int unit, int tm_idx_0, int arad_em_blocks_idx_0);
#endif /* BCM_PETRA_SUPPORT*/ 
#endif /* defined(BCM_PETRA_SUPPORT)*/ 

#if defined(BCM_PETRA_SUPPORT)
#ifdef BCM_PETRA_SUPPORT
/* implemented by: sw_state_dpp_soc_arad_tm_arad_em_blocks_end_address_dump */
typedef int (*sw_state_dpp_soc_arad_tm_arad_em_blocks_end_address_dump_cb)(
    int unit, int tm_idx_0, int arad_em_blocks_idx_0);
#endif /* BCM_PETRA_SUPPORT*/ 
#endif /* defined(BCM_PETRA_SUPPORT)*/ 

#if defined(BCM_PETRA_SUPPORT)
#ifdef BCM_PETRA_SUPPORT
/* implemented by: sw_state_dpp_soc_arad_tm_arad_em_blocks_multi_set_dump */
typedef int (*sw_state_dpp_soc_arad_tm_arad_em_blocks_multi_set_dump_cb)(
    int unit, int tm_idx_0, int arad_em_blocks_idx_0);
#endif /* BCM_PETRA_SUPPORT*/ 
#endif /* defined(BCM_PETRA_SUPPORT)*/ 

#if defined(BCM_PETRA_SUPPORT)
#ifdef BCM_PETRA_SUPPORT
/* implemented by: sw_state_dpp_soc_arad_tm_arad_em_blocks_base_dump */
typedef int (*sw_state_dpp_soc_arad_tm_arad_em_blocks_base_dump_cb)(
    int unit, int tm_idx_0, int arad_em_blocks_idx_0);
#endif /* BCM_PETRA_SUPPORT*/ 
#endif /* defined(BCM_PETRA_SUPPORT)*/ 

/*********************************** diagnostic calbacks struct ****************************************/
/* this set of structs, rooted at 'sw_state_cbs_t' define the diagnostic layer for the entire SW state.*/
/* use this tree to dump fields in the sw state rooted at 'sw_state_t'.              */
/* NOTE: 'sw_state_t' data should not be accessed directly.                                        */
/***************************************************************************************************/

#if defined(BCM_PETRA_SUPPORT)
#ifdef BCM_PETRA_SUPPORT
typedef struct sw_state_dpp_soc_arad_tm_arad_em_blocks_read_result_address_diagnostic_cbs_s {
    sw_state_dpp_soc_arad_tm_arad_em_blocks_read_result_address_dump_cb dump;
} sw_state_dpp_soc_arad_tm_arad_em_blocks_read_result_address_diagnostic_cbs_t;
#endif /* BCM_PETRA_SUPPORT*/ 
#endif /* defined(BCM_PETRA_SUPPORT)*/ 

#if defined(BCM_PETRA_SUPPORT)
#ifdef BCM_PETRA_SUPPORT
typedef struct sw_state_dpp_soc_arad_tm_arad_em_blocks_offset_diagnostic_cbs_s {
    sw_state_dpp_soc_arad_tm_arad_em_blocks_offset_dump_cb dump;
} sw_state_dpp_soc_arad_tm_arad_em_blocks_offset_diagnostic_cbs_t;
#endif /* BCM_PETRA_SUPPORT*/ 
#endif /* defined(BCM_PETRA_SUPPORT)*/ 

#if defined(BCM_PETRA_SUPPORT)
#ifdef BCM_PETRA_SUPPORT
typedef struct sw_state_dpp_soc_arad_tm_arad_em_blocks_table_size_diagnostic_cbs_s {
    sw_state_dpp_soc_arad_tm_arad_em_blocks_table_size_dump_cb dump;
} sw_state_dpp_soc_arad_tm_arad_em_blocks_table_size_diagnostic_cbs_t;
#endif /* BCM_PETRA_SUPPORT*/ 
#endif /* defined(BCM_PETRA_SUPPORT)*/ 

#if defined(BCM_PETRA_SUPPORT)
#ifdef BCM_PETRA_SUPPORT
typedef struct sw_state_dpp_soc_arad_tm_arad_em_blocks_key_size_diagnostic_cbs_s {
    sw_state_dpp_soc_arad_tm_arad_em_blocks_key_size_dump_cb dump;
} sw_state_dpp_soc_arad_tm_arad_em_blocks_key_size_diagnostic_cbs_t;
#endif /* BCM_PETRA_SUPPORT*/ 
#endif /* defined(BCM_PETRA_SUPPORT)*/ 

#if defined(BCM_PETRA_SUPPORT)
#ifdef BCM_PETRA_SUPPORT
typedef struct sw_state_dpp_soc_arad_tm_arad_em_blocks_data_nof_bytes_diagnostic_cbs_s {
    sw_state_dpp_soc_arad_tm_arad_em_blocks_data_nof_bytes_dump_cb dump;
} sw_state_dpp_soc_arad_tm_arad_em_blocks_data_nof_bytes_diagnostic_cbs_t;
#endif /* BCM_PETRA_SUPPORT*/ 
#endif /* defined(BCM_PETRA_SUPPORT)*/ 

#if defined(BCM_PETRA_SUPPORT)
#ifdef BCM_PETRA_SUPPORT
typedef struct sw_state_dpp_soc_arad_tm_arad_em_blocks_start_address_diagnostic_cbs_s {
    sw_state_dpp_soc_arad_tm_arad_em_blocks_start_address_dump_cb dump;
} sw_state_dpp_soc_arad_tm_arad_em_blocks_start_address_diagnostic_cbs_t;
#endif /* BCM_PETRA_SUPPORT*/ 
#endif /* defined(BCM_PETRA_SUPPORT)*/ 

#if defined(BCM_PETRA_SUPPORT)
#ifdef BCM_PETRA_SUPPORT
typedef struct sw_state_dpp_soc_arad_tm_arad_em_blocks_end_address_diagnostic_cbs_s {
    sw_state_dpp_soc_arad_tm_arad_em_blocks_end_address_dump_cb dump;
} sw_state_dpp_soc_arad_tm_arad_em_blocks_end_address_diagnostic_cbs_t;
#endif /* BCM_PETRA_SUPPORT*/ 
#endif /* defined(BCM_PETRA_SUPPORT)*/ 

#if defined(BCM_PETRA_SUPPORT)
#ifdef BCM_PETRA_SUPPORT
typedef struct sw_state_dpp_soc_arad_tm_arad_em_blocks_multi_set_diagnostic_cbs_s {
    sw_state_dpp_soc_arad_tm_arad_em_blocks_multi_set_dump_cb dump;
} sw_state_dpp_soc_arad_tm_arad_em_blocks_multi_set_diagnostic_cbs_t;
#endif /* BCM_PETRA_SUPPORT*/ 
#endif /* defined(BCM_PETRA_SUPPORT)*/ 

#if defined(BCM_PETRA_SUPPORT)
#ifdef BCM_PETRA_SUPPORT
typedef struct sw_state_dpp_soc_arad_tm_arad_em_blocks_base_diagnostic_cbs_s {
    sw_state_dpp_soc_arad_tm_arad_em_blocks_base_dump_cb dump;
} sw_state_dpp_soc_arad_tm_arad_em_blocks_base_diagnostic_cbs_t;
#endif /* BCM_PETRA_SUPPORT*/ 
#endif /* defined(BCM_PETRA_SUPPORT)*/ 

#if defined(BCM_PETRA_SUPPORT)
#ifdef BCM_PETRA_SUPPORT
typedef struct sw_state_dpp_soc_arad_tm_arad_em_blocks_diagnostic_cbs_s {
    sw_state_dpp_soc_arad_tm_arad_em_blocks_dump_cb dump;
    sw_state_dpp_soc_arad_tm_arad_em_blocks_read_result_address_diagnostic_cbs_t read_result_address;
    sw_state_dpp_soc_arad_tm_arad_em_blocks_offset_diagnostic_cbs_t offset;
    sw_state_dpp_soc_arad_tm_arad_em_blocks_table_size_diagnostic_cbs_t table_size;
    sw_state_dpp_soc_arad_tm_arad_em_blocks_key_size_diagnostic_cbs_t key_size;
    sw_state_dpp_soc_arad_tm_arad_em_blocks_data_nof_bytes_diagnostic_cbs_t data_nof_bytes;
    sw_state_dpp_soc_arad_tm_arad_em_blocks_start_address_diagnostic_cbs_t start_address;
    sw_state_dpp_soc_arad_tm_arad_em_blocks_end_address_diagnostic_cbs_t end_address;
    sw_state_dpp_soc_arad_tm_arad_em_blocks_multi_set_diagnostic_cbs_t multi_set;
    sw_state_dpp_soc_arad_tm_arad_em_blocks_base_diagnostic_cbs_t base;
} sw_state_dpp_soc_arad_tm_arad_em_blocks_diagnostic_cbs_t;

#endif /* BCM_PETRA_SUPPORT*/ 
#endif /* defined(BCM_PETRA_SUPPORT)*/ 

int sw_state_dpp_soc_arad_tm_arad_em_blocks_diagnostic_cb_init(int unit);

#endif /* BCM_WARM_BOOT_API_TEST */

#endif /* _SHR_SW_STATE_DPP_SOC_ARAD_TM_ARAD_EM_BLOCKS_DIAGNOSTIC_H_ */