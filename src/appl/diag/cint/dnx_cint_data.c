/** \file dnx_cint_data.c 
 * 
 * Expose DNX internal functions, structures, constans, ...  to cint.
 *
 */
 /*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
  #error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SOCDNX_DATA

int dnx_cint_data_not_empty;

#if defined(INCLUDE_LIB_CINT) && defined(BCM_DNX_SUPPORT)

#include <cint_config.h>
#include <cint_types.h>
#include <cint_porting.h>

#include <soc/dnx/dnx_data/dnx_data.h> 

CINT_FWRAPPER_CREATE_RP5(const uint32*,uint32, 1, 0,
                         dnx_data_utils_generic_data_get,
                         int,int,unit,0,0,
                         char *,char,module, 1,0,
                         char *,char,submodule, 1,0,
                         char *,char,data, 1,0,
                         char *,char,member, 1,0);    

static cint_function_t __cint_dnx_functions[] = 
    {
        CINT_FWRAPPER_NENTRY("dnx_data_get", dnx_data_utils_generic_data_get),
        
        CINT_ENTRY_LAST
    };  

cint_data_t dnx_cint_data = 
{
    NULL,
    __cint_dnx_functions,
    NULL,
    NULL, 
    NULL, 
    NULL, 
    NULL
}; 

#endif /* INCLUDE_LIB_CINT && BCM_DNX_SUPPORT*/

