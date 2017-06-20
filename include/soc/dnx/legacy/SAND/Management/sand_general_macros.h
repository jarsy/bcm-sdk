/* $Id: sand_general_macros.h,v 1.20 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifndef  DNX_SAND_DRIVER_SHORTCUTS_H
#define DNX_SAND_DRIVER_SHORTCUTS_H
#ifdef  __cplusplus
extern "C"
{
#endif

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <bcm/debug.h>

/* $Id: sand_general_macros.h,v 1.20 Broadcom SDK $
 * MACROs for validation code.
 */
#define DNX_SAND_GLOBAL_EXIT_PLACE_BASE       10000

/*
 * dnx_sand_check_driver_and_device failed.
 */
#define DNX_SAND_EXIT_PLACE_DRIVER_OR_DEVICE  (1  + DNX_SAND_GLOBAL_EXIT_PLACE_BASE)
/*
 * got NULL as input parameter.
 */
#define DNX_SAND_EXIT_PLACE_NULL_INPUT        (2  + DNX_SAND_GLOBAL_EXIT_PLACE_BASE)

/*
 * semaphore taking failed.
 */
#define DNX_SAND_EXIT_PLACE_SEM_TAKE_FAIL     (3  + DNX_SAND_GLOBAL_EXIT_PLACE_BASE)

/*
* semaphore giving failed.
*/
#define DNX_SAND_EXIT_PLACE_SEM_GIVE_FAIL     (4  + DNX_SAND_GLOBAL_EXIT_PLACE_BASE)

/*
 * When calling RevA only function while running over RevB+ chip
 */
/*
 * When calling RevB+ function while running over RevA chip
 */

/*
 * memory allocation failure
 */
#define DNX_SAND_EXIT_PLACE_MALLOC_FAIL       (13  + DNX_SAND_GLOBAL_EXIT_PLACE_BASE)

/*
* Magic Number validation
*/
#define DNX_SAND_EXIT_PLACE_MAGIC_NUM         (21  + DNX_SAND_GLOBAL_EXIT_PLACE_BASE)

#define _BSL_DNX_SAND_VVERBOSE_MSG(string) _ERR_MSG_MODULE_NAME, unit, "%s[%d]%s   " string "\n", __FILE__, __LINE__, FUNCTION_NAME()
#ifdef _ERR_MSG_MODULE_NAME
/** For macros printing in modular debug interface, using the
 *  DNX module 
 *  soc/dnx/legacy/error.h should be #included */
    #define _DNX_SAND_MSG(string) string "\n"
    #define _DNX_SAND_MSG_TWO_VARS(string, _a, _b) string "\n", _a, _b

    #define _BSL_DNX_SAND_MSG(string) _ERR_MSG_MODULE_NAME, unit, string "\n"
    #define _BSL_DNX_SAND_MSG_TWO_VARS(string, _a, _b) _ERR_MSG_MODULE_NAME, unit, string "\n", _a, _b

    #define DNX_SAND_INIT_PRINT _bsl_vverbose(_BSL_DNX_SAND_VVERBOSE_MSG("Enter"));
    #define DNX_SAND_EXIT_AND_ERR_PRINT(error_name,var_a, var_b) _bsl_error(_BSL_DNX_SAND_MSG_TWO_VARS("Function returned an error (var_a=%d, var_b=%d)", var_a, var_b));
    #define DNX_SAND_EXIT_VOID_AND_ERR_PRINT(error_name,var_a, var_b)  _bsl_error(_BSL_DNX_SAND_MSG_TWO_VARS("Function returned an error (var_a=%d, var_b=%d)", var_a, var_b));
    #define DNX_SAND_DRIVER_AND_DEVICE_PRINT  _bsl_error(_BSL_DNX_SAND_MSG("Driver and device error"));
    #define DNX_SAND_ERROR_CODE_PRINT(error)    if (error!=0) _bsl_error(_BSL_DNX_SAND_MSG(#error));
    #define DNX_SAND_TAKE_SEM_PRINT  _bsl_error(_BSL_DNX_SAND_MSG("Take semaphore error" ));
    #define DNX_SAND_GIVE_SEM_PRINT   _bsl_error(_BSL_DNX_SAND_MSG("Give semaphore error"));
    #define DNX_SAND_FUNC_RESULT_PRINT  _bsl_error(_BSL_DNX_SAND_MSG("dnx_sand function returned error"));
    #define DNX_SAND_FUNC_RESULT_SOC_PRINT(f_res) _bsl_error(_BSL_DNX_SAND_MSG(" %s\n"), soc_errmsg(f_res)); 


/** Under DNX module, ussage of following mcros is
 *  preferable over  DNX_SAND_EXIT_AND_SEND_ERROR and
 *  DNX_SAND_VOID_EXIT_AND_SEND_ERROR */
        /* 
     * Example usage: 
     * DNX_SAND_EXIT_AND_SEND_ERROR(("error in unit %d" , unit)); 
     * or 
     *  DNX_SAND_EXIT_AND_SEND_ERROR(( _DNX_MSG("error with number %d") , number));
     */
    #define DNX_SAND_EXIT_AND_SEND_ERROR_DNX(stuff)    \
          if (ex != no_err)                                         \
          {                                                         \
                _bsl_error stuff ;   \
          }                                                         \
          return ex;   

    /* For documentation and usage see DNX_SAND_EXIT_AND_SEND_ERROR above*/
    #define DNX_SAND_VOID_EXIT_AND_SEND_ERROR_DNX(stuff)    \
        if (ex != no_err)                                         \
        {                                                         \
            _bsl_error stuff ;   \
        }                                                         \
        return;

    /*
     * Used like the macros above. 
     */
    #define DNX_SAND_SET_ERROR_MSG(stuff)   \
      {                                                           \
          dnx_sand_set_error_code_into_error_word(DNX_SAND_GEN_ERR, &ex);     \
          exit_place = DNX_SAND_GEN_ERR;           \
          _bsl_error stuff ;  \
          goto exit;                                      \
      }



#else /*def _ERR_MSG_MODULE_NAME */
/** For macros printing with the old debug interface or using
 *  dnx_sand_os_printf() or not printing at all.
 *  Should never be used in code for JER2_ARAD and onward.
 *  */

    #define DNX_SAND_INIT_PRINT
    #define DNX_SAND_EXIT_AND_ERR_PRINT(error_name,var_a, var_b)  dnx_sand_error_handler(ex, error_name, exit_place,var_a,var_b,0,0,0 ); 
    #define DNX_SAND_EXIT_VOID_AND_ERR_PRINT(error_name,var_a, var_b)  dnx_sand_error_handler(ex, error_name, exit_place,var_a,var_b,0,0,0 ); 
    #define DNX_SAND_DRIVER_AND_DEVICE_PRINT 
    #define DNX_SAND_ERROR_CODE_PRINT(error)    
    #define  DNX_SAND_TAKE_SEM_PRINT  
    #define DNX_SAND_GIVE_SEM_PRINT   
    #define DNX_SAND_FUNC_RESULT_PRINT
    #define DNX_SAND_FUNC_RESULT_SOC_PRINT(f_res) _bsl_error(BSL_FILE, BSL_LINE, BSL_FUNC, bslLayerCount, bslSourceCount,\
        BSL_UNIT_UNKNOWN, " %s\n", soc_errmsg(f_res)); 
    #define DNX_SAND_SOC_ERR(stuff)   _bsl_error(BSL_FILE, BSL_LINE, BSL_FUNC, bslLayerCount, bslSourceCount,\
        BSL_UNIT_UNKNOWN, " %s\n", soc_errmsg(f_res));


#endif /*def _ERR_MSG_MODULE_NAME */

/* Must be placed at the end of the local variables declaration */
#define DNX_SAND_INIT_ERROR_DEFINITIONS(func_name)              \
  uint32                                                    \
    ex = 0,                                                 \
    no_err,                                                 \
    exit_place=0;     (void)exit_place;             \
      DNX_SAND_INIT_PRINT \
  dnx_sand_initialize_error_word(func_name, 0, &ex);            \
  no_err = ex;

/*PCID LITE VERSION - skip code not relevant for arrakis in order to improve init time*/
#ifdef BCM_PCID_LITE
#define DNX_SAND_PCID_LITE_SKIP(unit)                         \
    goto exit
#else
#define DNX_SAND_PCID_LITE_SKIP(unit)                         
#endif

#define DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(func_name)              \
  int unit = BSL_UNIT_UNKNOWN;                              \
  uint32                                                    \
    ex = 0,                                                 \
    no_err,                                                 \
    exit_place=0;     (void)exit_place;             \
  (void)unit;                          \
      DNX_SAND_INIT_PRINT \
  dnx_sand_initialize_error_word(func_name, 0, &ex);            \
  no_err = ex;

#define DNX_SAND_EXIT_AND_SEND_ERROR(error_name,var_a,var_b)    \
  if (ex != no_err)                                         \
  {                                                         \
    DNX_SAND_EXIT_AND_ERR_PRINT(error_name,var_a, var_b) \
  }                                                         \
  return ex;

#define DNX_SAND_VOID_EXIT_AND_SEND_ERROR(error_name,var_a,var_b)    \
  if (ex != no_err)                                         \
  {                                                         \
    DNX_SAND_EXIT_VOID_AND_ERR_PRINT(error_name,var_a, var_b) \
  }                                                         \
  return;


#define DNX_SAND_CHECK_DRIVER_AND_DEVICE                        \
  dnx_sand_check_driver_and_device(unit, &ex);             \
  if (ex != no_err)                                         \
  {                                                         \
    DNX_SAND_DRIVER_AND_DEVICE_PRINT \
    exit_place = DNX_SAND_EXIT_PLACE_DRIVER_OR_DEVICE;          \
    goto exit;                                              \
  }

#define DNX_SAND_CHECK_NULL_PTR(ptr, err_num, err_exit_label) \
  if(!ptr)\
  { \
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_NULL_POINTER_ERR, err_num, err_exit_label); \
  }

#define DNX_SAND_CHECK_NULL_INPUT(p_input)                      \
  DNX_SAND_CHECK_NULL_PTR(p_input, DNX_SAND_EXIT_PLACE_NULL_INPUT, exit);

#define DNX_SAND_TAKE_DEVICE_SEMAPHORE                             \
  if (DNX_SAND_OK != dnx_sand_take_chip_descriptor_mutex(unit) )  \
  {                                                            \
    dnx_sand_set_error_code_into_error_word(                       \
      DNX_SAND_SEM_TAKE_FAIL,                                      \
      &ex                                                      \
    );                                                         \
    DNX_SAND_TAKE_SEM_PRINT \
    exit_place = DNX_SAND_EXIT_PLACE_SEM_TAKE_FAIL;                \
    goto exit;                                                 \
  }


#define DNX_SAND_GIVE_DEVICE_SEMAPHORE                              \
  if (DNX_SAND_OK != dnx_sand_give_chip_descriptor_mutex(unit) )   \
  {                                                             \
    dnx_sand_set_error_code_into_error_word(DNX_SAND_SEM_GIVE_FAIL,&ex);\
    exit_place = DNX_SAND_EXIT_PLACE_SEM_GIVE_FAIL;                 \
    DNX_SAND_GIVE_SEM_PRINT \
    goto exit;                                                  \
  }




/* 
 * When  _ERR_MSG_MODULE_NAME is defined (presumably in module DNX)
 * first parameter must be DNX_SAND_ERROR_CODE directly, not through a variable. 
 * i.e. 
 *     DNX_SAND_ERROR_CODE error_var = DNX_SAND_GEN_ERR; 
 *     DNX_SAND_ERROR_CODE (error_var, 10, exit); 
 *  Will print "error_var" whereas
 *      DNX_SAND_ERROR_CODE (DNX_SAND_GEN_ERR, 10, exit);
 *  will print "DNX_SAND_GEN_ERR" as expected.
*/ 
#define DNX_SAND_SET_ERROR_CODE(dnx_sand_err_e,err_num,err_exit_label)\
  {                                                           \
       DNX_SAND_ERROR_CODE_PRINT(dnx_sand_err_e) \
    dnx_sand_set_error_code_into_error_word(dnx_sand_err_e, &ex);     \
    exit_place = err_num;                                     \
    goto err_exit_label;                                      \
  }

#define DNX_SAND_CHECK_FUNC_RESULT(f_res,err_num,err_exit_label)\
  if(dnx_sand_update_error_code(f_res, &ex ) != no_err)         \
  {                                                         \
    exit_place = err_num;                                   \
    DNX_SAND_FUNC_RESULT_PRINT   \
    goto err_exit_label;                                    \
  }


#define DNX_SAND_SOC_CHECK_FUNC_RESULT_ERR_VAL(f_res, err_num, err_exit_label, err_val) \
  do {                                                           \
    if(f_res != SOC_E_NONE) {                                    \
        DNX_SAND_FUNC_RESULT_SOC_PRINT(f_res) \
        f_res = (err_val);                                    \
        DNX_SAND_SET_ERROR_CODE(f_res,err_num,err_exit_label)        \
    }                                                            \
  } while(0)


#define DNX_SAND_SOC_CHECK_FUNC_RESULT(f_res,err_num,err_exit_label) \
  DNX_SAND_SOC_CHECK_FUNC_RESULT_ERR_VAL(f_res,err_num,err_exit_label, DNX_SAND_SOC_ERR)

#define DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, err_num, exit, err_val, op) \
    do { res = (op); DNX_SAND_SOC_CHECK_FUNC_RESULT_ERR_VAL(res, err_num, exit, err_val); } while(0)

#define DNX_SAND_SOC_IF_ERROR_RETURN(res, err_num, exit, op) \
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, err_num, exit, DNX_SAND_SOC_ERR, op)

#define DNX_SAND_MALLOC(snd_ptr, snd_size, str)        \
{                                             \
    snd_ptr = dnx_sand_os_malloc(snd_size, str);       \
    if(snd_ptr == NULL)                       \
    {                                         \
      dnx_sand_set_error_code_into_error_word(    \
      DNX_SAND_MALLOC_FAIL,                       \
      &ex                                     \
    );                                        \
    exit_place = DNX_SAND_EXIT_PLACE_MALLOC_FAIL; \
    goto exit;                                \
    }                                         \
}



/*
 * for save functions
 *  assume the following params: cur_size, total_size, buffer_size_bytes, res 
 *  defined locally
 */
#define DNX_SAND_COPY_TO_BUFF_AND_INC(var_dest_ptr, var_src_ptr, type, count)        \
  {                                                                       \
    if ((var_src_ptr == NULL) || (var_dest_ptr == NULL))                  \
    {                                                                     \
        DNX_SAND_SET_ERROR_CODE(DNX_SAND_NULL_POINTER_ERR, DNX_SAND_EXIT_PLACE_NULL_INPUT, exit); \
    }                                                                     \
    cur_size = (count) * sizeof(type);                       \
    total_size += cur_size;                                               \
    if (total_size > buffer_size_bytes)                                  \
    {                                                                     \
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR,DNX_SAND_EXIT_PLACE_MALLOC_FAIL,exit);  \
    }                                                                     \
    res = dnx_sand_os_memcpy(                                                 \
            var_dest_ptr,                                                 \
            var_src_ptr,                                                  \
            cur_size                                                      \
          );                                                              \
    DNX_SAND_CHECK_FUNC_RESULT(res, DNX_SAND_NULL_POINTER_ERR, exit);           \
    var_dest_ptr += cur_size;                                             \
  }

/*
 * for use in the exit part only
 */
#define DNX_SAND_FREE(snd_ptr) \
    {                             \
      if (snd_ptr)                \
      {                           \
        dnx_sand_os_free(snd_ptr);    \
        snd_ptr = NULL;           \
      }                           \
    }

/*
 * for use in the exit part only
 */

#define DNX_SAND_INTERRUPT_INIT_DEFS                      \
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
#define DNX_SAND_INTERRUPTS_STOP                        \
{                                                   \
  dnx_sand_os_stop_interrupts(&__sand_macro_int_flags); \
  __sand_macro_is_int_stopped = TRUE;               \
}

/*
 *	Release interrupt lock as defined in "DNX_SAND_INTERRUPTS_STOP"
 *  Please note that if a global OS semaphore was also taken here
 *  to prevent task-switching, it should be release here.
 */
#define DNX_SAND_INTERRUPTS_START_IF_STOPPED              \
{                                                     \
  if (__sand_macro_is_int_stopped == TRUE)            \
  {                                                   \
    dnx_sand_os_start_interrupts(__sand_macro_int_flags); \
    __sand_macro_is_int_stopped = FALSE ;             \
  }                                                   \
}

#define DNX_SAND_EXIT_NO_ERROR                                \
{                                                         \
  dnx_sand_set_error_code_into_error_word(DNX_SAND_NO_ERR, &ex);  \
  goto exit;                                              \
}



#define DNX_SAND_TODO_IMPLEMENT_WARNING \


#define   DNX_SAND_MAGIC_NUM_ENABLE
#ifdef    DNX_SAND_MAGIC_NUM_ENABLE
#define DNX_SAND_MAGIC_NUM_VAL    0x69
#define DNX_SAND_MAGIC_NUM_VAR    char dnx_sand_magic_num;
#define DNX_SAND_MAGIC_NUM_SET    info->dnx_sand_magic_num = DNX_SAND_MAGIC_NUM_VAL;
#define DNX_SAND_MAGIC_NUM_VERIFY(struct_name) \
      if(struct_name->dnx_sand_magic_num != DNX_SAND_MAGIC_NUM_VAL) \
        DNX_SAND_SET_ERROR_CODE(DNX_SAND_MAGIC_NUM_ERR,DNX_SAND_EXIT_PLACE_MAGIC_NUM,exit);
#define DNX_MAGIC_NUM_VERIFY(struct_name) \
      if(struct_name->dnx_sand_magic_num != DNX_SAND_MAGIC_NUM_VAL) \
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOC_MSG("DNX_SAND_MAGIC_NUM_ERR")));
#else
#define DNX_SAND_MAGIC_NUM_VAL
#define DNX_SAND_MAGIC_NUM_VAR
#define DNX_SAND_MAGIC_NUM_SET
#define DNX_SAND_MAGIC_NUM_VERIFY
#define DNX_MAGIC_NUM_VERIFY
#endif

/* 
 * For the following 7 Macros,  When  _ERR_MSG_MODULE_NAME is defined (presuimably in module DNX)
 * err_e parameter must be DNX_SAND_ERROR_CODE directly, not through a variable. 
 * i.e. 
 *     DNX_SAND_ERROR_CODE error_var = DNX_SAND_GEN_ERR; 
 *     DNX_SAND_ERR_IF_BELOW_MIN (val_to_check, min_val, error_val,err_num, exit); 
 *  Will print "error_var" whereas
 *     DNX_SAND_ERR_IF_BELOW_MIN (val_to_check, min_val, DNX_SAND_GEN_ERR,err_num, exit); 
 *  will print "DNX_SAND_GEN_ERR" as expected.
*/ 
#define DNX_SAND_ERR_IF_BELOW_MIN(val_to_check, min_val,err_e,err_num,err_exit_label) \
{                                                                             \
  if ((val_to_check) < (min_val))                                                 \
  {                                                                           \
    DNX_SAND_SET_ERROR_CODE(err_e, err_num, err_exit_label);   \
  }                                                                           \
}

/*see documentation for DNX_SAND_ERR_IF_BELOW_MIN() above.*/
#define DNX_SAND_ERR_IF_ABOVE_MAX(val_to_check, max_val,err_e,err_num,err_exit_label) \
{                                                                             \
  if ((val_to_check) > (max_val))                                                 \
  {                                                                           \
    DNX_SAND_SET_ERROR_CODE(err_e, err_num, err_exit_label);                      \
  }                                                                           \
}

/*see documentation for DNX_SAND_ERR_IF_BELOW_MIN() above.*/
#define DNX_SAND_ERR_IF_ABOVE_MAX_AND_NOT_NULL(val_to_check, max_val, null_val, err_e,err_num,err_exit_label) \
{                                                                             \
  if (((val_to_check) != (null_val)) && ((val_to_check) > (max_val)))         \
  {                                                                           \
  DNX_SAND_SET_ERROR_CODE(err_e, err_num, err_exit_label);                      \
  }                                                                           \
}

/*see documentation for DNX_SAND_ERR_IF_BELOW_MIN() above.*/
#define DNX_SAND_ERR_IF_ABOVE_NOF(val_to_check, nof,err_e,err_num,err_exit_label) \
{                                                                             \
  if ((val_to_check) >= (nof))                                                 \
  {                                                                           \
  DNX_SAND_SET_ERROR_CODE(err_e, err_num, err_exit_label);                      \
  }                                                                           \
}

/*see documentation for DNX_SAND_ERR_IF_BELOW_MIN() above.*/
#define DNX_SAND_ERR_IF_OUT_OF_RANGE(val_to_check, min_val, max_val,err_e,err_num,err_exit_label) \
{                                                                         \
  if (((val_to_check) < (min_val)) || ((val_to_check) > (max_val)))       \
  {                                                                       \
    DNX_SAND_SET_ERROR_CODE(err_e, err_num, err_exit_label);                  \
  }                                                                       \
}

/*see documentation for DNX_SAND_ERR_IF_BELOW_MIN() above.*/
#define DNX_SAND_ERR_IF_EQUALS_VALUE(val_to_check, val, err_e, err_num, err_exit_label) \
{                                                                         \
  if ((val_to_check) == (val))                                                \
  {                                                                       \
    DNX_SAND_SET_ERROR_CODE(err_e, err_num, err_exit_label);                  \
  }                                                                       \
}

/*see documentation for DNX_SAND_ERR_IF_BELOW_MIN() above.*/
#define DNX_SAND_ERR_IF_NOT_EQUALS_VALUE(val_to_check, val, err_e, err_num, err_exit_label) \
{                                                                         \
  if ((val_to_check) != (val))                                                \
  {                                                                       \
    DNX_SAND_SET_ERROR_CODE(err_e, err_num, err_exit_label);                  \
  }                                                                       \
}

#define DNX_SAND_IS_VAL_IN_RANGE(val_to_check, min_val, max_val) \
  (((val_to_check) >= (min_val)) && ((val_to_check) <= (max_val)))

#define DNX_SAND_IS_VAL_OUT_OF_RANGE(val_to_check, min_val, max_val) \
  !(DNX_SAND_IS_VAL_IN_RANGE((val_to_check), (min_val), (max_val)))

#define DNX_SAND_LIMIT_FROM_ABOVE(val, limit) \
{                                         \
  if (val>limit)                          \
  {                                       \
    val=limit;                            \
  }                                       \
}

#define DNX_SAND_LIMIT_FROM_BELOW(val, limit) \
{                                         \
  if (val<limit)                          \
  {                                       \
    val=limit;                            \
  }                                       \
}

#define DNX_SAND_LIMIT_VAL(val, min_limit, max_limit) \
{                                                 \
  DNX_SAND_LIMIT_FROM_ABOVE(val, max_limit);          \
  DNX_SAND_LIMIT_FROM_BELOW(val, min_limit);          \
};

/* Max value for bit length up to 32 bits */
#define DNX_SAND_MAX_VAL_FOR_BIT_LEN(val, len)                          \
{                                                                       \
  if (DNX_SAND_IS_VAL_IN_RANGE((int)len, 0, 32))                        \
  {                                                                     \
    if (len == 32)                                                      \
    {                                                                   \
      val = DNX_SAND_U32_MAX;                                           \
    }                                                                   \
    else                                                                \
    {                                                                   \
      val = ((1 << len) - 1);                                           \
    }                                                                   \
  } else {                                                              \
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_VALUE_OUT_OF_RANGE_ERR,0,exit);    \
  }                                                                     \
}

#ifdef  __cplusplus
}
#endif

#endif /*DNX_SAND_DRIVER_SHORTCUTS_H*/
