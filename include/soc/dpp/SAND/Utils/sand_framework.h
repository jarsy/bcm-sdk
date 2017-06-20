/* $Id: sand_framework.h,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/
#ifndef  SOC_SAND_DRIVER_FRAMEWORK_H
#define SOC_SAND_DRIVER_FRAMEWORK_H
#ifdef  __cplusplus
extern "C" {
#endif

#include <sal/types.h>
#include <sal/limits.h>
#include <sal/core/libc.h>
#include <sal/compiler.h>
#include <soc/defs.h>

/* $Id: sand_framework.h,v 1.13 Broadcom SDK $
 *  Print and debug possibilities:
 */

/*
 *  The debug and printing features are disabled. 
 *  If this level is selected, the relevant functions,
 *  such as enum-to-string conversion functions,
 *  printing functions etc., are compiled - out.
 *  This level is recommended only for systems without printing
 *  capabilities
 */
/*
 *  Allow printing structures and converting enumerators to strings.
 *  The enum-to-string and printing functions are compiled in
 *  (the library size is affected accordingly).
 *  Unless explicitly requested by calling the relevant function 
 *  e.g. string conversion or printing, does not have any run-time impact.
 *  The driver does not initiate printing (error messages or any other).
 *  The printing is available if called explicitly.
 *  Recommended for systems in production, if TCM is used for
 *  error messaging, and the extra-information available 
 *  when setting higher printing/debug levels is not required
 */
#define SOC_SAND_DBG_LVL1 1
  /*
   *  In addition to the functionality supplied by SOC_SAND_DBG_LVL1,
   *  implicitly prints additional information upon error (only):
   *  1. Prints the values of the input parameters of APIs - i.e., 
   *  parameters with a type 'SOC_SAND_IN' (for new devices and driver versions).
   *  2. For some error conditions, calls a relevant diagnostics function
   *  and prints the output of this function.
   *  Recommended for systems in bring-up stage, if printing functionality
   *  is supported.
   *  Imposes additional increase of the library size
   */
#define SOC_SAND_DBG_LVL2 2
  /*
   *  In addition to the functionality supplied by SOC_SAND_DBG_LVL2,
   *  the driver can print debug information, even if not under an error condition. 
   *  (log-printing mode).
   */
#define SOC_SAND_DBG_LVL3 3

  /*
   *	Recommended for systems in bring-up phase.
   *  refer to the documentation on printing levels for 
   *  a more detailed explanation
   */
#define SOC_SAND_DEBUG SOC_SAND_DBG_LVL2

/*
 * When set, the driver will print error messages, in the driver context.
 * For a production-stage system, this flag is typically unset ('0'),
 * and the error printing is done by the time callback module (TCM),
 * after getting a message from the driver, through a message queue.
 */
#define SOC_SAND_ALLOW_DRIVER_TO_PRINT_ERRORS (SOC_SAND_DEBUG >= SOC_SAND_DBG_LVL2)



/*
 * NULL pointer
 */
#ifndef NULL
  #define NULL (void *)0
#endif
/*
 * boolean true
 */
#ifndef TRUE
  #define TRUE 1
#endif
/*
 * boolean false
 */
#ifndef FALSE
  #define FALSE 0
#endif

/*
 * basic types definition
 */

#define SOC_SAND_BOOL2NUM(b_val) ((b_val) == FALSE?0x0:0x1)
#define SOC_SAND_NUM2BOOL(n_val) ((uint8)((n_val) == 0x0?FALSE:TRUE))

/*
 *  Invert the result - TRUE if value is 0, FALSE otherwise
 */
#define SOC_SAND_BOOL2NUM_INVERSE(b_val) ((b_val) == TRUE?0x0:0x1)
#define SOC_SAND_NUM2BOOL_INVERSE(n_val) (uint8)((n_val) == 0x0?TRUE:FALSE)

/*
 *  Range-related, unsigned.
 *  Get count of entities, first and last entity in range.
 */
#define SOC_SAND_RNG_COUNT(n_first, n_last) (((n_last) >= (n_first))?((n_last) - (n_first) + 1):((n_first) - (n_last) + 1))
#define SOC_SAND_RNG_FIRST(n_last, n_cnt)   ((((n_last) + 1) >= (n_cnt))?(((n_last) + 1) - (n_cnt)):0)
#define SOC_SAND_RNG_LAST(n_first, n_cnt)   ((n_first) + (n_cnt) - 1)
#define SOC_SAND_RNG_NEXT(n_curr)           ((n_curr) + 1)
#define SOC_SAND_RNG_PREV(n_curr)           (((n_curr) > 0)?((n_curr) - 1):0)

/*
 * basic return type of basic driver methods
 */
typedef
  unsigned short SOC_SAND_RET ;

/*
 * the basic SOC_SAND_RET for error
 */
#define SOC_SAND_ERR 1
/*
 * the basic SOC_SAND_RET for success
 */
#define SOC_SAND_OK  0
/*
 * all input params to a method must be one of these 3
 */
#define SOC_SAND_IN   const
#define SOC_SAND_OUT
#define SOC_SAND_INOUT

#define SOC_SAND_NOF_BITS_IN_BYTE 8

#define SOC_SAND_BIT_BYTE_SHIFT 3

#define SOC_SAND_TRANSLATE_BITS_TO_BYTES(_var)   \
           ((_var) >> SOC_SAND_BIT_BYTE_SHIFT)

/*
 * A generic indication for an invalid value, 32-bit.
 * May be used by internal driver functions
 */
#define SOC_SAND_INTERN_VAL_INVALID_32  0xffffffff

/*
 *	A value marking invalid register in the device
 */
#define SOC_SAND_REG_VAL_INVALID   0xDEADBEAF

/* 
 * TRUE if the internal value is the "invalid-value" indication
 */
#define SOC_SAND_IS_INTERN_VAL_INVALID(intern_val)  \
  SOC_SAND_NUM2BOOL(intern_val == SOC_SAND_INTERN_VAL_INVALID_32)

/*
 * Byte swapping MACRO.
 */
#define SOC_SAND_BYTE_SWAP(x) ((((x) << 24)) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | (((x) >> 24)))

/*
 * read / write from the chip must take one of this forms
 */
typedef enum
{
  SOC_SAND_DIRECT,
  SOC_SAND_INDIRECT
} SOC_SAND_ACCESS_METHOD ;
/*
 * sets the x bit in a word, and only it
 */
#ifndef SOC_SAND_BIT
  #define SOC_SAND_BIT(x) (1UL<<(x))
#endif
/*
 * resets the x bit in a word, and only it
 */
#ifndef SOC_SAND_RBIT
  #define SOC_SAND_RBIT(x) (~(1UL<<(x)))
#endif
/*
 * The number of devices this infrastructure
 * is willing to manage.
 * User which system supports more than 8 devices
 * per CPU should change this parameter.
 */
#ifndef SOC_SAND_MAX_DEVICE
  #define SOC_SAND_MAX_DEVICE SOC_MAX_NUM_DEVICES
#endif

/*
 */
#define SOC_SAND_OFFSETOF(x,y)  ((uint32)((char*)(&(((x *)0)->y)) - (char*)0))
/*
 * If 'true' then 'val_max' is meaningful.
 */
/*
 * If 'true' then 'val_min' is meaningful.
 */
/*
 */

typedef enum
{
  SOC_SAND_PRINT_FLAVORS_SHORT =0,
  SOC_SAND_PRINT_FLAVORS_NO_ZEROS,
  SOC_SAND_PRINT_FLAVORS_ERRS_ONLY
}SOC_SAND_PRINT_FLAVORS;

/*
 * General peruse OPERATION enumerator.
 */
typedef enum
{
  /*
   * No operation indicator.
   */
  SOC_SAND_NOP = 0,

  /*
   * 'AND' indicator.
   */
  SOC_SAND_OP_AND,

  /*
   * 'OR' indicator.
   */
  SOC_SAND_OP_OR,

  /*
   * Last one.
   * Count the number of possible operations.
   */
  SOC_SAND_NOF_SAND_OP

} SOC_SAND_OP;

typedef enum
{
  /*
   *  Operation ended with success                            
   */
  SOC_SAND_SUCCESS = 0,
  /*
   *  Operation failed due to lack of resources, e.g., there 
   *  is no space in the Exact match that map the key to the 
   *  table index or data                                     
   */
  SOC_SAND_FAILURE_OUT_OF_RESOURCES = 1,
  /*
   *  Operation failed due to lack of resources.When there are 
   *  two resources, this indicates that the second resource 
   *  type is missing                                         
   */
  SOC_SAND_FAILURE_OUT_OF_RESOURCES_2 = 2,
  /*
   *  Operation failed due to lack of resources.When there are 
   *  three resource types, this indicates that the third 
   *  resource type is missing                                
   */
  SOC_SAND_FAILURE_OUT_OF_RESOURCES_3 = 3,
  /*
   *  Entries in some databases cannot be updated on the fly. 
   *  This value indicates that the item under configuration 
   *  was already added, and the user is expected to remove it 
   *  before using the table index or key                     
   */
  SOC_SAND_FAILURE_REMOVE_ENTRY_FIRST = 10,
  /*
   *  Operation failed due to an internal error.                    
   */
  SOC_SAND_FAILURE_INTERNAL_ERR = 12,
  /*
   *	Operation failed, unspecified error cause. 
   */
  SOC_SAND_FAILURE_UNKNOWN_ERR = 100
  
}SOC_SAND_SUCCESS_FAILURE;

/*
 *  Number of types in SOC_SAND_SUCCESS_FAILURE
 */
#define SOC_SAND_NOF_SUCCESS_FAILURES (6)

/*
 *	Converts from SOC_SAND_SUCCESS_FAILURE
 *  to uint8, TRUE if SUCCESS
 */
#define SOC_SAND_SUCCESS2BOOL(success_status_) \
  (((success_status_) == SOC_SAND_SUCCESS)?TRUE:FALSE)
/*
 *	Converts from SOC_SAND_SUCCESS_FAILURE
 *  to uint8, TRUE if SUCCESS
 */
#define SOC_SAND_BOOL2SUCCESS(is_success_) \
  (((is_success_) == TRUE)?SOC_SAND_SUCCESS:SOC_SAND_FAILURE_UNKNOWN_ERR)

typedef enum
{
  /*
   *  Don't Perform any update/change
   */
  SOC_SAND_OPERATION_TYPE_NONE = 0,
  /*
   *  Add Operation
   */
  SOC_SAND_OPERATION_TYPE_ADD = 1,
  /*
   *  Remove Operation
   */
  SOC_SAND_OPERATION_TYPE_REMOVE = 2,
  /*
   *  Update Operation
   */
  SOC_SAND_OPERATION_TYPE_UPDATE = 3,
  /*
   *  Number of types in SOC_SAND_SUCCESS_FAILURE
   */
  SOC_SAND_NOF_OPERATION_TYPES = 4
}SOC_SAND_OPERATION_TYPE;

/*
 * Macros related to handling of SOC_SAND registers.
 * {
 */

#define SOC_SAND_REG_MAX_BIT    31
#define SOC_SAND_REG_SIZE_BITS  32
#define SOC_SAND_REG_SIZE_BYTES 4

/*
 * Take value and put it in its proper location within a 'long'
 * register (and make sure it does not effect bits outside its
 * predefined mask).
 */
#define SOC_SAND_SET_FLD_IN_PLACE(val,shift,mask) (uint32)(((uint32)(val) << ((uint32)shift)) & (uint32)(mask))
/*
 * Get a value out of location within a 'long' register (and make sure it
 * is not effected by bits outside its predefined mask).
 */
#define SOC_SAND_GET_FLD_FROM_PLACE(val,shift,mask) (((uint32)(val) & (mask)) >> (shift))

#define SOC_SAND_BITS_MASK(ms_bit,ls_bit) \
    ( ((uint32)SOC_SAND_BIT(ms_bit)) - ((uint32)SOC_SAND_BIT(ls_bit)) + ((uint32)SOC_SAND_BIT(ms_bit)) )


#define SOC_SAND_ZERO_BITS_MASK(ms_bit,ls_bit)  (~(SOC_SAND_BITS_MASK(ms_bit, ls_bit)))

#define SOC_SAND_RBITS_MASK(ms_bit,ls_bit)      (SOC_SAND_ZERO_BITS_MASK(ms_bit, ls_bit))

#define SOC_SAND_SET_BITS_RANGE(val,ms_bit,ls_bit) \
 (((uint32)(val) << (ls_bit)) & (SOC_SAND_BITS_MASK(ms_bit,ls_bit)))

#define SOC_SAND_GET_BITS_RANGE(val,ms_bit,ls_bit) \
 (((uint32)(val) & (SOC_SAND_BITS_MASK(ms_bit,ls_bit))) >> (ls_bit))

#define SOC_SAND_GET_BIT(val,bit) (SOC_SAND_GET_BITS_RANGE(val,bit,bit)?(uint32)0x1:(uint32)0x0)

/*
 * Set/Get of bytes.
 * Byte-0: Bits 24-31
 * Byte-1: Bits 16-23
 * Byte-2: Bits 8 -15
 * Byte-3: Bits 0 -7
 */
#define SOC_SAND_GET_BYTE_3(val) SOC_SAND_GET_FLD_FROM_PLACE(val,0,  0x000000FF)
#define SOC_SAND_GET_BYTE_2(val) SOC_SAND_GET_FLD_FROM_PLACE(val,8,  0x0000FF00)
#define SOC_SAND_GET_BYTE_1(val) SOC_SAND_GET_FLD_FROM_PLACE(val,16, 0x00FF0000)
#define SOC_SAND_GET_BYTE_0(val) SOC_SAND_GET_FLD_FROM_PLACE(val,24, 0xFF000000)

#define SOC_SAND_SET_BYTE_2(val) SOC_SAND_SET_FLD_IN_PLACE(val,8,  0x0000FF00)
#define SOC_SAND_SET_BYTE_1(val) SOC_SAND_SET_FLD_IN_PLACE(val,16, 0x00FF0000)

#define SOC_SAND_SET_BIT(reg,val,bit_i)         \
  reg = (val ? (reg | SOC_SAND_BIT(bit_i)) : (reg & SOC_SAND_RBIT(bit_i)));

#define SOC_SAND_APPLY_MASK(_reg, _enable, _mask)   \
    _reg = (_enable ? (_reg | _mask) : (_reg & ~_mask));

/*
 * Basic SOC_SAND types limits
 */
#define SOC_SAND_U8_MAX 0xff

#define SOC_SAND_U16_MAX 0xffff
#define SOC_SAND_I16_MAX 0x7fff

#define SOC_SAND_U32_MAX 0xffffffff

#define SOC_SAND_UINT_MAX SAL_UINT32_MAX

/*
 * Macro to remove all the compiler
 * warnings on unreferred variables
 */
#define SOC_SAND_IGNORE_UNUSED_VAR(p)     (void)(p)

/*
 *  Infinite loop
 */
#define SOC_SAND_LOOP_FOREVER for(;;)

/*
 * Minimal time between two activations of TCM task, in system
 * ticks. This limits the CPU load due to TCM task and, accordingly,
 * the precision of time periods requested by the user in commands
 * involved with tcm, and interrupt handling latency.
 */
#define SOC_SAND_MIN_TCM_ACTIVATION_PERIOD  1
/*
 * This definition is only related to indirect access.
 * Maximal time between asserting the 'trigger' bit and
 * getting response from device, in nanoseconds.
 */
#define SOC_SAND_TRIGGER_TIMEOUT            1000
/*
 * This definition is only related to DRAM init access.
 * Maximal time between asserting the 'trigger' bit and
 * getting response from device, in nanoseconds.
 */
/*
 * This definition is only related to sending fabric cells.
 * Maximal time between asserting the transmit 'trigger'
 * bit and getting response from device (transmit 'trigger'
 * bit deasserted), in nanoseconds.
 */
/*
 * Size of buffer handed over to driver, to put results in,
 * for callback procedure. Driver may use part or all
 * buffer to store results in.
 */
#define SOC_SAND_CALLBACK_BUF_SIZE          500
/*
 * Pointer to error handler procedure which may be
 * called when driver detects an error condition.
 * Error handler returns int and gets, as parameters,
 * uint32, char *, char **,
 *     uint32,uint32,uint32,
 *     uint32,uint32,uint32
 *
 *
 *    uint32 err_id -
 *      Identifier of error. See ERROR RETURN VALUES
 *      in soc_sand_error_code.h
 *    char          *err_desc -
 *      Null terminated string describing error.
 *      This is the buffer the user gave last time the
 *      function was called.
 *    char          **new_buf -
 *      New buffer to be used next time this function
 *      is to be called.
 *      Case NULL is loaded, the buffer is not replaced.
 *    uint32 param_01 -
 *      General parameters related to error description.
 *    uint32 param_02 -
 *      General parameters related to error description.
 *    uint32 param_03 -
 *      General parameters related to error description.
 *    uint32 param_04 -
 *      General parameters related to error description.
 *    uint32 param_05 -
 *      General parameters related to error description.
 *    uint32 param_06 -
 *      General parameters related to error description.
 */
typedef int (*SOC_SAND_ERROR_HANDLER_PTR)(
  uint32,char *,char **,uint32,
  uint32,uint32,uint32,uint32,uint32) ;
/*
 * local error registering and handling
 * {
 */
typedef struct
{
  uint32 err_id ;
  char          error_txt[SOC_SAND_CALLBACK_BUF_SIZE] ;
  uint32 param_01 ;
  uint32 param_02 ;
  uint32 param_03 ;
  uint32 param_04 ;
  uint32 param_05 ;
  uint32 param_06 ;
} SOC_SAND_ERRORS_QUEUE_MESSAGE ;

typedef struct
{
  /*
   *  Start of the range.                                     
   */
  uint32 start;
  /*
   *  End of the range [start, end]: May be closed or open 
   *  range according to what is specified for the specific 
   *  use.                                                    
   */
  uint32 end;

}SOC_SAND_U32_RANGE;

#define SOC_SAND_TBL_ITER_SCAN_ALL        (0xFFFFFFFF)
#define SOC_SAND_TBL_ITER_SET_BEGIN(iter) ((*(iter)) = 0)
#define SOC_SAND_TBL_ITER_SET_END(iter) ((*(iter)) = 0xFFFFFFFF)
#define SOC_SAND_TBL_ITER_IS_BEGIN(iter) (*(iter) == 0)
#define SOC_SAND_TBL_ITER_IS_END(iter) (*(iter) == 0xFFFFFFFF)


typedef struct
{
  /*
   *  Iterator indicates the place to start to act from 
   *  (read/delete/modify).                                   
   */
  uint32 iter;
  /*
   *  The number of entries to scan.Stop after scanning this 
   *  number of entries. set to SOC_SAND_TBL_ITER_SCAN_ALL to scan all entries
   */
  uint32 entries_to_scan;
  /*
   *  Number of entries to act on.Stop after acting on this 
   *  number of entries.                                      
   */
  uint32 entries_to_act;

}SOC_SAND_TABLE_BLOCK_RANGE;

/*
 *  Maximal number of messages in 'errors' queue
 *  which contains information concerning system
 *  wide events/errors and which is handled by
 *  TCM task. TCM task sends those reports to a
 *  user-supplied callback for system-wide
 *  error handling.
 */
#define ERRORS_MSG_QUEUE_NUM_MSGS       30
/*
 *  Number of system ticks to wait for 'errors' queue
 *  which contains information concerning system
 *  wide events/errors and which is handled by
 *  TCM task.
 */
/*
 * Size of one message in 'errors' queue
 */
#define SOC_SAND_ERRORS_MSG_QUEUE_SIZE_MSG      sizeof(SOC_SAND_ERRORS_QUEUE_MESSAGE)



  extern uint32
      Soc_sand_errors_msg_queue_flagged;
  extern char  *Soc_sand_supplied_error_buffer ;
  extern uint32
    Soc_sand_supplied_error_handler_is_on;



SOC_SAND_RET
  soc_sand_error_handler(
    uint32 err_id,
    const char          *error_txt,
    uint32 param_01,
    uint32 param_02,
    uint32 param_03,
    uint32 param_04,
    uint32 param_05,
    uint32 param_06
  ) ;

SOC_SAND_RET
  soc_sand_invoke_user_error_handler(
    uint32 err_id,
    const char    *error_txt,
    uint32 param_01,
    uint32 param_02,
    uint32 param_03,
    uint32 param_04,
    uint32 param_05,
    uint32 param_06
  ) ;

/*
 */
SOC_SAND_RET
  soc_sand_set_user_error_handler(
    SOC_SAND_ERROR_HANDLER_PTR   user_error_handler,
    char                    *user_error_buffer
  ) ;

SOC_SAND_RET
  soc_sand_set_user_error_state(
    uint32 onFlag
  );
/*
 * }
 * local error registering and handling
 */

/*
 * return TRUE / FALSE
 */
#define SOC_SAND_UINT32_ALIGN_MASK (SOC_SAND_BIT(0) | SOC_SAND_BIT(1))

/* allocate memory dynamically for a given type 'DS_ELEM_TYPE' of a data structure
 *NOTE: DS_ELEM_TYPE must be statically-sized type
 */

/* allocate memory for an element of 'DS_ELEM_TYPE',
 *  placed at DS_NAME[unit].
 *if the memory have already been allocated - report ERROR
 *NOTE1: 'DEVICE_ID' is assumed to be set to the current device value
 */


/* free the memory allocated for 'DS_ELEM'*/

/*free the memory allocated for 'unit' entry of DS_NAME data structure*/
/*
 */
int
  soc_sand_is_long_aligned(
    uint32 word_to_check
  ) ;

void
  soc_sand_check_driver_and_device(
    int  unit,
    uint32 *error_word
  ) ;

uint32
  soc_sand_get_index_of_max_member_in_array(
    SOC_SAND_IN     uint32                     array[],
    SOC_SAND_IN     uint32                    len
  );

SOC_SAND_RET
  soc_sand_set_field(
    SOC_SAND_INOUT  uint32    *reg_val,
    SOC_SAND_IN  uint32       ms_bit,
    SOC_SAND_IN  uint32       ls_bit,
    SOC_SAND_IN  uint32       field_val
  );

SOC_SAND_RET
  soc_sand_U8_to_U32(
    SOC_SAND_IN uint8     *u8_val,
    SOC_SAND_IN uint32    nof_bytes,
    SOC_SAND_OUT uint32   *u32_val
  );

SOC_SAND_RET
  soc_sand_U32_to_U8(
    SOC_SAND_IN uint32  *u32_val,
    SOC_SAND_IN uint32  nof_bytes,
    SOC_SAND_OUT uint8  *u8_val
  );

/*
 * }
 */

/*
 * {  SOC_SAND_SYNC_IOS
 * Macro for IO-memory barrier synchronize
 * When accessing to the device "very soon" after writing a value and
 * expecting it to do something. Example indirect trigger.
 */
#if (defined(CPU) && ((CPU)==(PPC860)) && defined(__GNUC__))
  #define SOC_SAND_SYNC_IOS   __asm__ __volatile__ ("   eieio")
#elif defined(SOC_PPC_CPU) && defined(__GNUC__)
  #define SOC_SAND_SYNC_IOS   __asm__ __volatile__ ("   eieio")
#else
  #define SOC_SAND_SYNC_IOS
#endif

#ifndef SOC_SAND_SYNC_IOS
  #error "SOC_SAND_SYNC_IOS need to be defined." \
         "  Specifically, in Dune Reference System we use PPC860" \
         "  Consult with the CPU Data Sheet, or your BSP expert, for a similar ASM code."
#endif

void
  soc_sand_SAND_U32_RANGE_clear(
    SOC_SAND_OUT SOC_SAND_U32_RANGE *info
  );

void
  soc_sand_SAND_TABLE_BLOCK_RANGE_clear(
    SOC_SAND_OUT SOC_SAND_TABLE_BLOCK_RANGE *info
  );

#if SOC_SAND_DEBUG
/* { */
int
  soc_sand_general_display_err(
    uint32 err_id,
    const char    *error_txt
  );

/*
 * Printing utility.
 * Convert from enumerator to string.
 */
const char*
  soc_sand_SAND_OP_to_str(
    SOC_SAND_IN SOC_SAND_OP      soc_sand_op,
    SOC_SAND_IN uint32 short_format
  ) ;

/*
 * Print HEX buffer.
 */
void
  soc_sand_print_hex_buff(
    SOC_SAND_IN char*        buff,
    SOC_SAND_IN uint32 buff_byte_size,
    SOC_SAND_IN uint32 nof_bytes_per_line
  );

/*
 * Prints Band-Width.
 */
void
  soc_sand_print_bandwidth(
    SOC_SAND_IN uint32 bw_kbps,
    SOC_SAND_IN uint32  short_format
  );




const char*
  soc_sand_SAND_SUCCESS_FAILURE_to_string(
    SOC_SAND_IN  SOC_SAND_SUCCESS_FAILURE enum_val
  );

void
  soc_sand_SAND_U32_RANGE_print(
    SOC_SAND_IN  SOC_SAND_U32_RANGE *info
  );

void
  soc_sand_SAND_TABLE_BLOCK_RANGE_print(
    SOC_SAND_IN  SOC_SAND_TABLE_BLOCK_RANGE *info
  );



/* } */
#endif

/*
 * } End SOC_SAND_SYNC_IOS
 */
#ifdef  __cplusplus
}
#endif

#endif
