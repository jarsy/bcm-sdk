/*
 * $Id: error.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _DNXC_ERROR_H
#define _DNXC_ERROR_H

#include <shared/error.h>
#include <shared/bsl.h>
#ifdef BCM_DNX_SUPPORT
#include <soc/drv.h>
#include <soc/dnx/legacy/error.h>
#include <soc/dnx/legacy/debug.h> 
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h> 
#endif /* BCM_DNX_SUPPORT */

#if  defined(BROADCOM_DEBUG)

/*
 * Unified message format for all modules 
 * In order to use this format _ERR_MSG_MODULE_NAME must be defined 
 * For example in Fabric module the following will be in top of the c file: 
 *  #ifdef _ERR_MSG_MODULE_NAME
 *    #error "_ERR_MSG_MODULE_NAME redefined"
 *  #endif
 *  #define _ERR_MSG_MODULE_NAME FABRIC
 * And in the bottom:
 *  #undef _ERR_MSG_MODULE_NAME
*/
    #define _DNXC_MSG(string) string "\n"
    #define _DNXC_MSG_NO_UNIT(string) string "\n"

    #define _BSL_DNXC_MSG(string) _ERR_MSG_MODULE_NAME, unit, string "\n"
    #define _BSL_DNXC_MSG_NO_UNIT(string) _ERR_MSG_MODULE_NAME, BSL_UNIT_UNKNOWN, string "\n"

#else /*  defined(BROADCOM_DEBUG) */
    #define _DNXC_MSG(string) string
    #define _DNXC_MSG_W_UNIT(string) string
    #define _DNXC_MSG_NO_UNIT(string) string

    #define _BSL_DNXC_MSG(string) _ERR_MSG_MODULE_NAME, unit, string
    #define _BSL_DNXC_MSG_NO_UNIT(string) _ERR_MSG_MODULE_NAME, unit, string
    
#endif /*  defined(BROADCOM_DEBUG) */
    #define _BSL_DNXC_MSG_STR(string) _ERR_MSG_MODULE_NAME, unit, string
/*Following macro independent of DNXC_INIT_FUNC_DEFS and related macros.*/
#define DNXC_IF_ERR_RETURN(_rv) \
  do { \
  int __rv__ = _rv; \
  if (__rv__ != SOC_E_NONE) { \
  _bsl_error(_BSL_DNXC_MSG("%s"),soc_errmsg(__rv__)); \
  return __rv__; \
  } \
  } while (0)

#define _BSL_DNXC_VVERBOSE_MSG(string) _ERR_MSG_MODULE_NAME, unit, "%s[%d]%s unit: %d  " string "\n", __FILE__, __LINE__, FUNCTION_NAME(), unit

/* Must appear at each function right after parameters definition */
#define DNXC_INIT_FUNC_DEFS \
    int _rv = SOC_E_NONE, _lock_taken = 0; \
    (void)_lock_taken;  \
    _bsl_vverbose(_BSL_DNXC_VVERBOSE_MSG("enter")); 

#define DNXC_FUNC_ERROR \
    SOC_FAILURE(_rv)

#define DNXC_FUNC_RETURN \
    _bsl_vverbose(_BSL_DNXC_VVERBOSE_MSG("exit")); \
    return _rv;

#define DNXC_FUNC_RETURN_VOID \
    _bsl_vverbose(_BSL_DNXC_VVERBOSE_MSG("exit"));\
    COMPILER_REFERENCE(_rv);

#define DNXC_IF_ERR_CONT(_rc) \
      do { \
        int __err__rc = _rc; \
        if(__err__rc != SOC_E_NONE) { \
            _bsl_warn(_BSL_DNXC_MSG("%s"),soc_errmsg(__err__rc)); \
            _rv = __err__rc; \
        } \
      } while(0)

#define DNXC_SAND_IF_ERR_CONT(_sand_ret) \
    do { \
        int __err__rc;\
        int error_code = soc_sand_get_error_code_from_error_word(_sand_ret);\
        __err__rc = (error_code == SOC_SAND_OK) ? SOC_E_NONE : SOC_E_FAIL;\
        if(__err__rc != SOC_E_NONE) { \
            _bsl_warn(_BSL_DNXC_MSG("%s"),soc_errmsg(__err__rc)); \
            _rv = __err__rc; \
        } \
      } while(0)

#define DNXC_IF_ERR_EXIT(_rc) \
      do { \
        int __err__rc = _rc; \
        if(__err__rc != SOC_E_NONE) { \
            _bsl_error(_BSL_DNXC_MSG("%s"),soc_errmsg(__err__rc)); \
            _rv = __err__rc; \
            SOC_EXIT; \
        } \
      } while(0)

/* meant for use after exit: , In case of error log it and if it is the first error, update _rv */
#define DNXC_IF_ERR_REPORT(_rc) \
      do { \
        int __err__rc = _rc; \
        if(__err__rc != SOC_E_NONE) { \
            _bsl_error(_BSL_DNXC_MSG("%s"),soc_errmsg(__err__rc)); \
            if (_rv !=  SOC_E_NONE) _rv = __err__rc; \
        } \
      } while(0)

#define DNXC_IF_ERR_EXIT_NO_UNIT(_rc) \
      do { \
        int __err__rc = _rc; \
        if(__err__rc != SOC_E_NONE) { \
            _bsl_error(_BSL_DNXC_MSG_NO_UNIT("%s"),soc_errmsg(__err__rc)); \
            _rv = __err__rc; \
            SOC_EXIT; \
        } \
      } while(0)


#define DNXC_SAND_IF_ERR_RETURN(_sand_ret) \
      do { \
      soc_error_t _rv = dnx_handle_sand_result(_sand_ret); \
      if (_rv != SOC_E_NONE) { \
      _bsl_error(_BSL_DNXC_MSG("%s"),soc_errmsg(_rv)); \
      return _rv; \
      } \
      } while (0)

#define DNXC_SAND_IF_ERR_EXIT(_sand_ret) \
    do { \
        int __err__rc = dnx_handle_sand_result(_sand_ret); \
        if(__err__rc != SOC_E_NONE) { \
            _bsl_error(_BSL_DNXC_MSG("%s"),soc_errmsg(_rv)); \
            _rv = __err__rc; \
            SOC_EXIT; \
        } \
      } while(0)

#define DNXC_SAND_IF_ERR_EXIT_MSG(_sand_ret, stuff) \
    do { \
        int __err__rc = dnx_handle_sand_result(_sand_ret); \
        if(__err__rc != SOC_E_NONE) { \
            DNXC_EXIT_WITH_ERR(__err__rc, stuff); \
        } \
      } while(0)


#define DNXC_EXIT_WITH_ERR(_rc, stuff) \
      do { \
            _bsl_error stuff ; \
            _rv = _rc; \
            SOC_EXIT; \
      } while(0)

#define DNXC_EXIT_WITH_ERR_NO_MSG(_rc) \
      do { \
            _rv = _rc; \
            SOC_EXIT; \
      } while(0)

#define DNXC_IF_ERR_EXIT_MSG(_rc, stuff)  \
      do { \
      int __err__rc = _rc; \
      if(__err__rc != SOC_E_NONE) { \
          _bsl_error stuff ; \
          _rv = __err__rc; \
          SOC_EXIT; \
      } \
      /* Coverity: the dead code is empty */    \
      /* coverity[dead_error_line:FALSE] */     \
} while (0); 

#define DNXC_NULL_CHECK(arg) \
    do {   \
        if ((arg) == NULL) \
        { DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("null parameter" ))); } \
    } while (0)

#define DNXC_FREE(arg)   \
  do {   \
      if(arg != NULL) \
      {sal_free(arg);} \
  } while (0)

#define DNXC_ALLOC(var, type, count,str)                                     \
    do {                                                                       \
      if(var != NULL)                                                       \
      {                                                                     \
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Trying to allocate to a non null ptr is forbidden"))); \
      }                                                                     \
      var = (type*)sal_alloc((count) * sizeof(type),str);                  \
      if (var == NULL)                                                      \
      {                                                                     \
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("Allocating failed")));  \
      }                                                                     \
      sal_memset(                                                 \
          var,                                                          \
          0x0,                                                          \
          (size_t) ((count) * sizeof(type))                                        \
        );                                                    \
    } while(0)

#ifdef BROADCOM_DEBUG
#define DNXC_VERIFY(exp) if (!(exp)) { DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("VERIFY FAILED (%s:%d): %s\n"), __FILE__, __LINE__, #exp)); }
#else
#define DNXC_VERIFY(exp) ((void)0)
#endif

#ifdef DNXC_RUNTIME_DEBUG
typedef enum {
    dnxc_runtime_debug_state_loading = 0,
    dnxc_runtime_debug_state_initializing = 1,
    dnxc_runtime_debug_state_running = 2
} dnxc_runtime_debug_state_t;
typedef struct {
    sal_time_t start_time;
    sal_time_t last_time;
    dnxc_runtime_debug_state_t run_stage;
} dnxc_runtime_debug_t;
extern dnxc_runtime_debug_t dnxc_runtime_debug_per_device[SOC_MAX_NUM_DEVICES];

void dnxc_runtime_debug_update_print_time(int unit, const char *string_to_print);

#define DNXC_RUNTIME_DEBUG_PRINT(unit) dnxc_runtime_debug_update_print_time(unit, "")
#define DNXC_RUNTIME_DEBUG_PRINT_LOC(unit, location) dnxc_runtime_debug_update_print_time(unit, (location))
#else /* DNXC_RUNTIME_DEBUG */
#define DNXC_RUNTIME_DEBUG_PRINT(unit)
#define DNXC_RUNTIME_DEBUG_PRINT_LOC(unit, location)
#endif /* DNXC_RUNTIME_DEBUG */

/*PCID LITE VERSION - skip code not relevant for arrakis in order to improve init time*/
#ifdef BCM_PCID_LITE
#define DNXC_PCID_LITE_SKIP(unit)                         \
    SOC_EXIT
#else
#define DNXC_PCID_LITE_SKIP(unit)                         
#endif



#define DNXC_LEGACY_FIXME_ASSERT  do {\
                                    _bsl_error(_BSL_DNXC_MSG_NO_UNIT("code masked during dpp to dnx code merging")); \
                                    assert(0);\
                                  } while(0)
                                  

#endif  /* !_DNXC_ERROR_H */
