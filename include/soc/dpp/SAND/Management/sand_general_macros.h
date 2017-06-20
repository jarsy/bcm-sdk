/* $Id: sand_general_macros.h,v 1.20 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifndef  SOC_SAND_DRIVER_SHORTCUTS_H
#define SOC_SAND_DRIVER_SHORTCUTS_H
#ifdef  __cplusplus
extern "C"
{
#endif

#include <soc/dpp/SAND/Utils/sand_framework.h>
#include <bcm/debug.h>

/* $Id: sand_general_macros.h,v 1.20 Broadcom SDK $
 * MACROs for validation code.
 */
#define SOC_SAND_GLOBAL_EXIT_PLACE_BASE       10000

/*
 * soc_sand_check_driver_and_device failed.
 */
#define SOC_SAND_EXIT_PLACE_DRIVER_OR_DEVICE  (1  + SOC_SAND_GLOBAL_EXIT_PLACE_BASE)
/*
 * got NULL as input parameter.
 */
#define SOC_SAND_EXIT_PLACE_NULL_INPUT        (2  + SOC_SAND_GLOBAL_EXIT_PLACE_BASE)

/*
 * semaphore taking failed.
 */
#define SOC_SAND_EXIT_PLACE_SEM_TAKE_FAIL     (3  + SOC_SAND_GLOBAL_EXIT_PLACE_BASE)

/*
* semaphore giving failed.
*/
#define SOC_SAND_EXIT_PLACE_SEM_GIVE_FAIL     (4  + SOC_SAND_GLOBAL_EXIT_PLACE_BASE)

/*
 * When calling RevA only function while running over RevB+ chip
 */
/*
 * When calling RevB+ function while running over RevA chip
 */

/*
 * memory allocation failure
 */
#define SOC_SAND_EXIT_PLACE_MALLOC_FAIL       (13  + SOC_SAND_GLOBAL_EXIT_PLACE_BASE)

/*
* Magic Number validation
*/
#define SOC_SAND_EXIT_PLACE_MAGIC_NUM         (21  + SOC_SAND_GLOBAL_EXIT_PLACE_BASE)

#define _BSL_SOCDNX_SAND_VVERBOSE_MSG(string) _ERR_MSG_MODULE_NAME, unit, "%s[%d]%s   " string "\n", __FILE__, __LINE__, FUNCTION_NAME()
#ifdef _ERR_MSG_MODULE_NAME
/** For macros printing in modular debug interface, using the
 *  SOCDNX module 
 *  soc/dpp/error.h should be #included */
    #define _SOCDNX_SAND_MSG(string) string "\n"
    #define _SOCDNX_SAND_MSG_TWO_VARS(string, _a, _b) string "\n", _a, _b

    #define _BSL_SOCDNX_SAND_MSG(string) _ERR_MSG_MODULE_NAME, unit, string "\n"
    #define _BSL_SOCDNX_SAND_MSG_TWO_VARS(string, _a, _b) _ERR_MSG_MODULE_NAME, unit, string "\n", _a, _b

    #define INIT_PRINT _bsl_vverbose(_BSL_SOCDNX_SAND_VVERBOSE_MSG("Enter"));
    #define EXIT_AND_ERR_PRINT(error_name,var_a, var_b) _bsl_error(_BSL_SOCDNX_SAND_MSG_TWO_VARS("Function returned an error (var_a=%d, var_b=%d)", var_a, var_b));
    #define EXIT_VOID_AND_ERR_PRINT(error_name,var_a, var_b)  _bsl_error(_BSL_SOCDNX_SAND_MSG_TWO_VARS("Function returned an error (var_a=%d, var_b=%d)", var_a, var_b));
    #define DRIVER_AND_DEVICE_PRINT  _bsl_error(_BSL_SOCDNX_SAND_MSG("Driver and device error"));
    #define ERROR_CODE_PRINT(error)    if (error!=0) _bsl_error(_BSL_SOCDNX_SAND_MSG(#error));
    #define TAKE_SEM_PRINT  _bsl_error(_BSL_SOCDNX_SAND_MSG("Take semaphore error" ));
    #define GIVE_SEM_PRINT   _bsl_error(_BSL_SOCDNX_SAND_MSG("Give semaphore error"));
    #define FUNC_RESULT_PRINT  _bsl_error(_BSL_SOCDNX_SAND_MSG("soc_sand function returned error"));
    #define FUNC_RESULT_SOC_PRINT(f_res) _bsl_error(_BSL_SOCDNX_SAND_MSG(" %s\n"), soc_errmsg(f_res)); 


/** Under SOCDNX module, ussage of following mcros is
 *  preferable over  SOC_SAND_EXIT_AND_SEND_ERROR and
 *  SOC_SAND_VOID_EXIT_AND_SEND_ERROR */
        /* 
     * Example usage: 
     * SOC_SAND_EXIT_AND_SEND_ERROR(("error in unit %d" , unit)); 
     * or 
     *  SOC_SAND_EXIT_AND_SEND_ERROR(( _SOCDNX_MSG("error with number %d") , number));
     */
    #define SOC_SAND_EXIT_AND_SEND_ERROR_SOCDNX(stuff)    \
          if (ex != no_err)                                         \
          {                                                         \
                _bsl_error stuff ;   \
          }                                                         \
          return ex;   

    /* For documentation and usage see SOC_SAND_EXIT_AND_SEND_ERROR above*/
    #define SOC_SAND_VOID_EXIT_AND_SEND_ERROR_SOCDNX(stuff)    \
        if (ex != no_err)                                         \
        {                                                         \
            _bsl_error stuff ;   \
        }                                                         \
        return;

    /*
     * Used like the macros above. 
     */
    #define SOC_SAND_SET_ERROR_MSG(stuff)   \
      {                                                           \
          soc_sand_set_error_code_into_error_word(SOC_SAND_GEN_ERR, &ex);     \
          exit_place = SOC_SAND_GEN_ERR;           \
          _bsl_error stuff ;  \
          goto exit;                                      \
      }



#else /*def _ERR_MSG_MODULE_NAME */
/** For macros printing with the old debug interface or using
 *  soc_sand_os_printf() or not printing at all.
 *  Should never be used in code for ARAD and onward.
 *  */

    #define INIT_PRINT
    #define EXIT_AND_ERR_PRINT(error_name,var_a, var_b)  soc_sand_error_handler(ex, error_name, exit_place,var_a,var_b,0,0,0 ); 
    #define EXIT_VOID_AND_ERR_PRINT(error_name,var_a, var_b)  soc_sand_error_handler(ex, error_name, exit_place,var_a,var_b,0,0,0 ); 
    #define DRIVER_AND_DEVICE_PRINT 
    #define ERROR_CODE_PRINT(error)    
    #define  TAKE_SEM_PRINT  
    #define GIVE_SEM_PRINT   
    #define FUNC_RESULT_PRINT
    #define FUNC_RESULT_SOC_PRINT(f_res) _bsl_error(BSL_FILE, BSL_LINE, BSL_FUNC, bslLayerCount, bslSourceCount,\
        BSL_UNIT_UNKNOWN, " %s\n", soc_errmsg(f_res)); 
    #define SOC_SAND_SOC_ERR(stuff)   _bsl_error(BSL_FILE, BSL_LINE, BSL_FUNC, bslLayerCount, bslSourceCount,\
        BSL_UNIT_UNKNOWN, " %s\n", soc_errmsg(f_res));


#endif /*def _ERR_MSG_MODULE_NAME */

/* Must be placed at the end of the local variables declaration */
#define SOC_SAND_INIT_ERROR_DEFINITIONS(func_name)              \
  uint32                                                    \
    ex = 0,                                                 \
    no_err,                                                 \
    exit_place=0;     (void)exit_place;             \
      INIT_PRINT \
  soc_sand_initialize_error_word(func_name, 0, &ex);            \
  no_err = ex;

/*PCID LITE VERSION - skip code not relevant for arrakis in order to improve init time*/
#ifdef BCM_PCID_LITE
#define SOC_SAND_PCID_LITE_SKIP(unit)                         \
    goto exit
#else
#define SOC_SAND_PCID_LITE_SKIP(unit)                         
#endif

#define SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(func_name)              \
  int unit = BSL_UNIT_UNKNOWN;                              \
  uint32                                                    \
    ex = 0,                                                 \
    no_err,                                                 \
    exit_place=0;     (void)exit_place;             \
  (void)unit;                          \
      INIT_PRINT \
  soc_sand_initialize_error_word(func_name, 0, &ex);            \
  no_err = ex;

#define SOC_SAND_EXIT_AND_SEND_ERROR(error_name,var_a,var_b)    \
  if (ex != no_err)                                         \
  {                                                         \
    EXIT_AND_ERR_PRINT(error_name,var_a, var_b) \
  }                                                         \
  return ex;

#define SOC_SAND_VOID_EXIT_AND_SEND_ERROR(error_name,var_a,var_b)    \
  if (ex != no_err)                                         \
  {                                                         \
    EXIT_VOID_AND_ERR_PRINT(error_name,var_a, var_b) \
  }                                                         \
  return;


#define SOC_SAND_CHECK_DRIVER_AND_DEVICE                        \
  soc_sand_check_driver_and_device(unit, &ex);             \
  if (ex != no_err)                                         \
  {                                                         \
    DRIVER_AND_DEVICE_PRINT \
    exit_place = SOC_SAND_EXIT_PLACE_DRIVER_OR_DEVICE;          \
    goto exit;                                              \
  }

#define SOC_SAND_CHECK_NULL_PTR(ptr, err_num, err_exit_label) \
  if(!ptr)\
  { \
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_NULL_POINTER_ERR, err_num, err_exit_label); \
  }

#define SOC_SAND_CHECK_NULL_INPUT(p_input)                      \
  SOC_SAND_CHECK_NULL_PTR(p_input, SOC_SAND_EXIT_PLACE_NULL_INPUT, exit);

#define SOC_SAND_TAKE_DEVICE_SEMAPHORE                             \
  if (SOC_SAND_OK != soc_sand_take_chip_descriptor_mutex(unit) )  \
  {                                                            \
    soc_sand_set_error_code_into_error_word(                       \
      SOC_SAND_SEM_TAKE_FAIL,                                      \
      &ex                                                      \
    );                                                         \
    TAKE_SEM_PRINT \
    exit_place = SOC_SAND_EXIT_PLACE_SEM_TAKE_FAIL;                \
    goto exit;                                                 \
  }


#define SOC_SAND_GIVE_DEVICE_SEMAPHORE                              \
  if (SOC_SAND_OK != soc_sand_give_chip_descriptor_mutex(unit) )   \
  {                                                             \
    soc_sand_set_error_code_into_error_word(SOC_SAND_SEM_GIVE_FAIL,&ex);\
    exit_place = SOC_SAND_EXIT_PLACE_SEM_GIVE_FAIL;                 \
    GIVE_SEM_PRINT \
    goto exit;                                                  \
  }




/* 
 * When  _ERR_MSG_MODULE_NAME is defined (presumably in module SOCDNX)
 * first parameter must be SOC_SAND_ERROR_CODE directly, not through a variable. 
 * i.e. 
 *     SOC_SAND_ERROR_CODE error_var = SOC_SAND_GEN_ERR; 
 *     SOC_SAND_ERROR_CODE (error_var, 10, exit); 
 *  Will print "error_var" whereas
 *      SOC_SAND_ERROR_CODE (SOC_SAND_GEN_ERR, 10, exit);
 *  will print "SOC_SAND_GEN_ERR" as expected.
*/ 
#define SOC_SAND_SET_ERROR_CODE(soc_sand_err_e,err_num,err_exit_label)\
  {                                                           \
       ERROR_CODE_PRINT(soc_sand_err_e) \
    soc_sand_set_error_code_into_error_word(soc_sand_err_e, &ex);     \
    exit_place = err_num;                                     \
    goto err_exit_label;                                      \
  }

#define SOC_SAND_CHECK_FUNC_RESULT(f_res,err_num,err_exit_label)\
  if(soc_sand_update_error_code(f_res, &ex ) != no_err)         \
  {                                                         \
    exit_place = err_num;                                   \
    FUNC_RESULT_PRINT   \
    goto err_exit_label;                                    \
  }


#define SOC_SAND_SOC_CHECK_FUNC_RESULT_ERR_VAL(f_res, err_num, err_exit_label, err_val) \
  do {                                                           \
    if(f_res != SOC_E_NONE) {                                    \
        FUNC_RESULT_SOC_PRINT(f_res) \
        f_res = (err_val);                                    \
        SOC_SAND_SET_ERROR_CODE(f_res,err_num,err_exit_label)        \
    }                                                            \
  } while(0)


#define SOC_SAND_SOC_CHECK_FUNC_RESULT(f_res,err_num,err_exit_label) \
  SOC_SAND_SOC_CHECK_FUNC_RESULT_ERR_VAL(f_res,err_num,err_exit_label, SOC_SAND_SOC_ERR)

#define SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, err_num, exit, err_val, op) \
    do { res = (op); SOC_SAND_SOC_CHECK_FUNC_RESULT_ERR_VAL(res, err_num, exit, err_val); } while(0)

#define SOC_SAND_SOC_IF_ERROR_RETURN(res, err_num, exit, op) \
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, err_num, exit, SOC_SAND_SOC_ERR, op)

#define SOC_SAND_MALLOC(snd_ptr, snd_size, str)        \
{                                             \
    snd_ptr = soc_sand_os_malloc(snd_size, str);       \
    if(snd_ptr == NULL)                       \
    {                                         \
      soc_sand_set_error_code_into_error_word(    \
      SOC_SAND_MALLOC_FAIL,                       \
      &ex                                     \
    );                                        \
    exit_place = SOC_SAND_EXIT_PLACE_MALLOC_FAIL; \
    goto exit;                                \
    }                                         \
}



/*
 * for save functions
 *  assume the following params: cur_size, total_size, buffer_size_bytes, res 
 *  defined locally
 */
#define SOC_SAND_COPY_TO_BUFF_AND_INC(var_dest_ptr, var_src_ptr, type, count)        \
  {                                                                       \
    if ((var_src_ptr == NULL) || (var_dest_ptr == NULL))                  \
    {                                                                     \
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_NULL_POINTER_ERR, SOC_SAND_EXIT_PLACE_NULL_INPUT, exit); \
    }                                                                     \
    cur_size = (count) * sizeof(type);                       \
    total_size += cur_size;                                               \
    if (total_size > buffer_size_bytes)                                  \
    {                                                                     \
      SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR,SOC_SAND_EXIT_PLACE_MALLOC_FAIL,exit);  \
    }                                                                     \
    res = soc_sand_os_memcpy(                                                 \
            var_dest_ptr,                                                 \
            var_src_ptr,                                                  \
            cur_size                                                      \
          );                                                              \
    SOC_SAND_CHECK_FUNC_RESULT(res, SOC_SAND_NULL_POINTER_ERR, exit);           \
    var_dest_ptr += cur_size;                                             \
  }

/*
 * for use in the exit part only
 */
#define SOC_SAND_FREE(snd_ptr) \
    {                             \
      if (snd_ptr)                \
      {                           \
        soc_sand_os_free(snd_ptr);    \
        snd_ptr = NULL;           \
      }                           \
    }

/*
 * for use in the exit part only
 */

#define SOC_SAND_INTERRUPT_INIT_DEFS                      \
  uint32          __sand_macro_int_flags = 0;           \
  uint32  __sand_macro_is_int_stopped = FALSE

/*
 *	This MACRO is used to stop all interrupts.
 *  It is often used to prevent context switch in
 *  the areas where per-device semaphore cannot be taken 
 *  (e.g. before device registration).
 *  Please note that in multi-core systems it is not enough 
 *  to stop interrupts to prevent context-switch.
 *  In this case, the macro implementation should also take
 *  a global OS semaphore preventing any task-switching.
 */
#define SOC_SAND_INTERRUPTS_STOP                        \
{                                                   \
  soc_sand_os_stop_interrupts(&__sand_macro_int_flags); \
  __sand_macro_is_int_stopped = TRUE;               \
}

/*
 *	Release interrupt lock as defined in "SOC_SAND_INTERRUPTS_STOP"
 *  Please note that if a global OS semaphore was also taken here
 *  to prevent task-switching, it should be release here.
 */
#define SOC_SAND_INTERRUPTS_START_IF_STOPPED              \
{                                                     \
  if (__sand_macro_is_int_stopped == TRUE)            \
  {                                                   \
    soc_sand_os_start_interrupts(__sand_macro_int_flags); \
    __sand_macro_is_int_stopped = FALSE ;             \
  }                                                   \
}

#define SOC_SAND_EXIT_NO_ERROR                                \
{                                                         \
  soc_sand_set_error_code_into_error_word(SOC_SAND_NO_ERR, &ex);  \
  goto exit;                                              \
}



#define SOC_SAND_TODO_IMPLEMENT_WARNING \


#define   SOC_SAND_MAGIC_NUM_ENABLE
#ifdef    SOC_SAND_MAGIC_NUM_ENABLE
#define SOC_SAND_MAGIC_NUM_VAL    0x69
#define SOC_SAND_MAGIC_NUM_VAR    char soc_sand_magic_num;
#define SOC_SAND_MAGIC_NUM_SET    info->soc_sand_magic_num = SOC_SAND_MAGIC_NUM_VAL;
#define SOC_SAND_MAGIC_NUM_VERIFY(struct_name) \
      if(struct_name->soc_sand_magic_num != SOC_SAND_MAGIC_NUM_VAL) \
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_MAGIC_NUM_ERR,SOC_SAND_EXIT_PLACE_MAGIC_NUM,exit);
#define SOCDNX_MAGIC_NUM_VERIFY(struct_name) \
      if(struct_name->soc_sand_magic_num != SOC_SAND_MAGIC_NUM_VAL) \
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOC_MSG("SOC_SAND_MAGIC_NUM_ERR")));
#else
#define SOC_SAND_MAGIC_NUM_VAL
#define SOC_SAND_MAGIC_NUM_VAR
#define SOC_SAND_MAGIC_NUM_SET
#define SOC_SAND_MAGIC_NUM_VERIFY
#define SOCDNX_MAGIC_NUM_VERIFY
#endif

/* 
 * For the following 7 Macros,  When  _ERR_MSG_MODULE_NAME is defined (presuimably in module SOCDNX)
 * err_e parameter must be SOC_SAND_ERROR_CODE directly, not through a variable. 
 * i.e. 
 *     SOC_SAND_ERROR_CODE error_var = SOC_SAND_GEN_ERR; 
 *     SOC_SAND_ERR_IF_BELOW_MIN (val_to_check, min_val, error_val,err_num, exit); 
 *  Will print "error_var" whereas
 *     SOC_SAND_ERR_IF_BELOW_MIN (val_to_check, min_val, SOC_SAND_GEN_ERR,err_num, exit); 
 *  will print "SOC_SAND_GEN_ERR" as expected.
*/ 
#define SOC_SAND_ERR_IF_BELOW_MIN(val_to_check, min_val,err_e,err_num,err_exit_label) \
{                                                                             \
  if ((val_to_check) < (min_val))                                                 \
  {                                                                           \
    SOC_SAND_SET_ERROR_CODE(err_e, err_num, err_exit_label);   \
  }                                                                           \
}

/*see documentation for SOC_SAND_ERR_IF_BELOW_MIN() above.*/
#define SOC_SAND_ERR_IF_ABOVE_MAX(val_to_check, max_val,err_e,err_num,err_exit_label) \
{                                                                             \
  if ((val_to_check) > (max_val))                                                 \
  {                                                                           \
    SOC_SAND_SET_ERROR_CODE(err_e, err_num, err_exit_label);                      \
  }                                                                           \
}

/*see documentation for SOC_SAND_ERR_IF_BELOW_MIN() above.*/
#define SOC_SAND_ERR_IF_ABOVE_MAX_AND_NOT_NULL(val_to_check, max_val, null_val, err_e,err_num,err_exit_label) \
{                                                                             \
  if (((val_to_check) != (null_val)) && ((val_to_check) > (max_val)))         \
  {                                                                           \
  SOC_SAND_SET_ERROR_CODE(err_e, err_num, err_exit_label);                      \
  }                                                                           \
}

/*see documentation for SOC_SAND_ERR_IF_BELOW_MIN() above.*/
#define SOC_SAND_ERR_IF_ABOVE_NOF(val_to_check, nof,err_e,err_num,err_exit_label) \
{                                                                             \
  if ((val_to_check) >= (nof))                                                 \
  {                                                                           \
  SOC_SAND_SET_ERROR_CODE(err_e, err_num, err_exit_label);                      \
  }                                                                           \
}

/*see documentation for SOC_SAND_ERR_IF_BELOW_MIN() above.*/
#define SOC_SAND_ERR_IF_OUT_OF_RANGE(val_to_check, min_val, max_val,err_e,err_num,err_exit_label) \
{                                                                         \
  if (((val_to_check) < (min_val)) || ((val_to_check) > (max_val)))       \
  {                                                                       \
    SOC_SAND_SET_ERROR_CODE(err_e, err_num, err_exit_label);                  \
  }                                                                       \
}

/*see documentation for SOC_SAND_ERR_IF_BELOW_MIN() above.*/
#define SOC_SAND_ERR_IF_EQUALS_VALUE(val_to_check, val, err_e, err_num, err_exit_label) \
{                                                                         \
  if ((val_to_check) == (val))                                                \
  {                                                                       \
    SOC_SAND_SET_ERROR_CODE(err_e, err_num, err_exit_label);                  \
  }                                                                       \
}

/*see documentation for SOC_SAND_ERR_IF_BELOW_MIN() above.*/
#define SOC_SAND_ERR_IF_NOT_EQUALS_VALUE(val_to_check, val, err_e, err_num, err_exit_label) \
{                                                                         \
  if ((val_to_check) != (val))                                                \
  {                                                                       \
    SOC_SAND_SET_ERROR_CODE(err_e, err_num, err_exit_label);                  \
  }                                                                       \
}

#define SOC_SAND_IS_VAL_IN_RANGE(val_to_check, min_val, max_val) \
  (((val_to_check) >= (min_val)) && ((val_to_check) <= (max_val)))

#define SOC_SAND_IS_VAL_OUT_OF_RANGE(val_to_check, min_val, max_val) \
  !(SOC_SAND_IS_VAL_IN_RANGE((val_to_check), (min_val), (max_val)))

#define SOC_SAND_LIMIT_FROM_ABOVE(val, limit) \
{                                         \
  if (val>limit)                          \
  {                                       \
    val=limit;                            \
  }                                       \
}

#define SOC_SAND_LIMIT_FROM_BELOW(val, limit) \
{                                         \
  if (val<limit)                          \
  {                                       \
    val=limit;                            \
  }                                       \
}

#define SOC_SAND_LIMIT_VAL(val, min_limit, max_limit) \
{                                                 \
  SOC_SAND_LIMIT_FROM_ABOVE(val, max_limit);          \
  SOC_SAND_LIMIT_FROM_BELOW(val, min_limit);          \
};

/* Max value for bit length up to 32 bits */
#define SOC_SAND_MAX_VAL_FOR_BIT_LEN(val, len)                          \
{                                                                       \
  if (SOC_SAND_IS_VAL_IN_RANGE((int)len, 0, 32))                        \
  {                                                                     \
    if (len == 32)                                                      \
    {                                                                   \
      val = SOC_SAND_U32_MAX;                                           \
    }                                                                   \
    else                                                                \
    {                                                                   \
      val = ((1 << len) - 1);                                           \
    }                                                                   \
  } else {                                                              \
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_VALUE_OUT_OF_RANGE_ERR,0,exit);    \
  }                                                                     \
}

#ifdef  __cplusplus
}
#endif

#endif /*SOC_SAND_DRIVER_SHORTCUTS_H*/
