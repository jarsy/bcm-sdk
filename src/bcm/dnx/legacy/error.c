/*
 * $Id: error.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: Error translation
 */
 
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_BCM_COMMON

#include <shared/bsl.h>

#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/TMC/tmc_api_framework.h>
#include <soc/dnx/legacy/ARAD/arad_api_framework.h>
#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/debug.h>

int
dnx_translate_sand_success_failure(const DNX_SAND_SUCCESS_FAILURE result)
{
    switch(result) {
        case DNX_SAND_SUCCESS:
            return BCM_E_NONE;
            
        case DNX_SAND_FAILURE_OUT_OF_RESOURCES:
        case DNX_SAND_FAILURE_OUT_OF_RESOURCES_2:
        case DNX_SAND_FAILURE_OUT_OF_RESOURCES_3:
            return BCM_E_FULL;
            
        case DNX_SAND_FAILURE_REMOVE_ENTRY_FIRST:
            return BCM_E_EXISTS;
            
        case DNX_SAND_FAILURE_INTERNAL_ERR:
            return BCM_E_INTERNAL;

        case DNX_SAND_FAILURE_UNKNOWN_ERR:
        default:
             break;
    }
    return BCM_E_PARAM; 
}

int
dnx_handle_sand_result(uint32 dnx_sand_result)
{
#if BROADCOM_DEBUG
    uint32 proc_id, err_id;
    char *err_name, *err_text, *dnx_sand_proc_name, *dnx_sand_module_name;
#endif /* BROADCOM_DEBUG */
    uint16 error_code;

    error_code = dnx_sand_get_error_code_from_error_word(dnx_sand_result);
  
    if (error_code != DNX_SAND_OK) {
#if BROADCOM_DEBUG
        err_id = dnx_sand_error_code_to_string(error_code, &err_name,&err_text) ;

        if (dnx_sand_get_error_code_from_error_word(err_id) != DNX_SAND_OK) {
            err_text = "No error code description (or procedure id) found" ;
        }

        proc_id = dnx_sand_get_proc_id_from_error_word(dnx_sand_result) ;
        dnx_sand_proc_id_to_string((unsigned long)proc_id,&dnx_sand_module_name,&dnx_sand_proc_name)  ;

        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META("DNX_SAND Error Code 0x%X (fail):\r\n"
                            "Text : %s\n\r" /*Error name*/
                            "%s\r\n"        /*Error description*/
                            "Procedure id: 0x%04X (Mod: %s, Proc: %s)\n\r"),
                   err_id,
                   err_name,
                   err_text,
                   proc_id,
                   dnx_sand_module_name,
                   dnx_sand_proc_name));
#endif /* BROADCOM_DEBUG */        
        
        /* map soc error to BCM error */        
        switch (error_code) {
        case (uint16) DNX_TMC_INPUT_OUT_OF_RANGE:
            return BCM_E_PARAM;
        case (uint16) DNX_TMC_CONFIG_ERR:
            return BCM_E_CONFIG;
        case (uint16) JER2_ARAD_CELL_NO_RECEIVED_CELL_ERR:
            return BCM_E_EMPTY;
        default:
            return BCM_E_INTERNAL;        
        }
    }
    
    return BCM_E_NONE;
}
