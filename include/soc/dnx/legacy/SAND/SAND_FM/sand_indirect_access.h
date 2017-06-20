/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/
#ifndef DNX_SAND_INDIRECT_ACCESS_H
#define DNX_SAND_INDIRECT_ACCESS_H
#ifdef  __cplusplus
extern "C" {
#endif

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>


/* $Id$
 * Definitions related to 'indirect_access_addresss'. {
 */

#define DNX_SAND_MODULE_MS_BIT           31
#define DNX_SAND_MODULE_NUM_BITS         5
#define DNX_SAND_MODULE_LS_BIT           (DNX_SAND_MODULE_MS_BIT + 1 - DNX_SAND_MODULE_NUM_BITS)
#define DNX_SAND_MODULE_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_SAND_MODULE_MS_BIT) - DNX_SAND_BIT(DNX_SAND_MODULE_LS_BIT)) + (uint32)DNX_SAND_BIT(DNX_SAND_MODULE_MS_BIT))
#define DNX_SAND_MODULE_SHIFT            DNX_SAND_MODULE_LS_BIT

#define DNX_SAND_RD_NOT_WR_MS_BIT        31
#define DNX_SAND_RD_NOT_WR_NUM_BITS      1
#define DNX_SAND_RD_NOT_WR_LS_BIT        (DNX_SAND_RD_NOT_WR_MS_BIT + 1 - DNX_SAND_RD_NOT_WR_NUM_BITS)
#define DNX_SAND_RD_NOT_WR_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_SAND_RD_NOT_WR_MS_BIT) - DNX_SAND_BIT(DNX_SAND_RD_NOT_WR_LS_BIT)) + DNX_SAND_BIT(DNX_SAND_RD_NOT_WR_MS_BIT))
/*
 * } Definitions related to 'indirect_access_triggers'. {
 */
/*
 * } Definition of indirect option varable {
 */

#define DNX_SAND_TBL_READ_NO_RVRS 0

/* This field is dedicated for offset */
#define DNX_SAND_TBL_READ_RVRS_BITS_WORD_MSB    31
#define DNX_SAND_TBL_READ_RVRS_BITS_WORD_LSB    30
/* This field is dedicated for the table width */
#define DNX_SAND_TBL_READ_SIZE_BITS_WORD_MSB    11
#define DNX_SAND_TBL_READ_SIZE_BITS_WORD_LSB     0

#define DNX_SAND_GET_TBL_READ_RVRS_BITS_FROM_WORD(word)                                               \
          DNX_SAND_GET_FLD_FROM_PLACE(                                                                     \
            word,                                                                                 \
            DNX_SAND_TBL_READ_RVRS_BITS_WORD_LSB,                                                     \
            DNX_SAND_BITS_MASK(DNX_SAND_TBL_READ_RVRS_BITS_WORD_MSB, DNX_SAND_TBL_READ_RVRS_BITS_WORD_LSB)    \
          )


#define DNX_SAND_GET_TBL_READ_SIZE_BITS_FROM_WORD(word)                                               \
          DNX_SAND_GET_FLD_FROM_PLACE(                                                                     \
            word,                                                                                 \
            DNX_SAND_TBL_READ_SIZE_BITS_WORD_LSB,                                                     \
            DNX_SAND_BITS_MASK(DNX_SAND_TBL_READ_SIZE_BITS_WORD_MSB, DNX_SAND_TBL_READ_SIZE_BITS_WORD_LSB)    \
          )


#define DNX_SAND_TBL_WRITE_NO_RVRS 0

/* This field is dedicated for offset */
#define DNX_SAND_TBL_WRITE_RVRS_BITS_WORD_MSB   31
#define DNX_SAND_TBL_WRITE_RVRS_BITS_WORD_LSB   30
/* This is dedicated for the nof repetitions */
#define DNX_SAND_TBL_WRITE_REPT_BITS_WORD_MSB   29
#define DNX_SAND_TBL_WRITE_REPT_BITS_WORD_LSB   12
/* This field is dedicated for the table width */
#define DNX_SAND_TBL_WRITE_SIZE_BITS_WORD_MSB   11
#define DNX_SAND_TBL_WRITE_SIZE_BITS_WORD_LSB    0

#define DNX_SAND_GET_TBL_WRITE_REPT_BITS_FROM_WORD(word)                                              \
          DNX_SAND_GET_FLD_FROM_PLACE(                                                                     \
            word,                                                                                 \
            DNX_SAND_TBL_WRITE_REPT_BITS_WORD_LSB,                                                    \
            DNX_SAND_BITS_MASK(DNX_SAND_TBL_WRITE_REPT_BITS_WORD_MSB, DNX_SAND_TBL_WRITE_REPT_BITS_WORD_LSB)  \
          )

#define DNX_SAND_GET_TBL_WRITE_SIZE_BITS_FROM_WORD(word)                                              \
          DNX_SAND_GET_FLD_FROM_PLACE(                                                                     \
            word,                                                                                 \
            DNX_SAND_TBL_WRITE_SIZE_BITS_WORD_LSB,                                                    \
            DNX_SAND_BITS_MASK(DNX_SAND_TBL_WRITE_SIZE_BITS_WORD_MSB, DNX_SAND_TBL_WRITE_SIZE_BITS_WORD_LSB)  \
          )

#define DNX_SAND_GET_TBL_WRITE_RVRS_BITS_FROM_WORD(word)                                              \
          DNX_SAND_GET_FLD_FROM_PLACE(                                                                     \
            word,                                                                                 \
            DNX_SAND_TBL_WRITE_RVRS_BITS_WORD_LSB,                                                    \
            DNX_SAND_BITS_MASK(DNX_SAND_TBL_WRITE_RVRS_BITS_WORD_MSB, DNX_SAND_TBL_WRITE_RVRS_BITS_WORD_LSB)  \
          )

#define DNX_SAND_SET_TBL_WRITE_REPT_BITS_IN_WORD(word)                                                \
          DNX_SAND_SET_FLD_IN_PLACE(                                                                       \
            word,                                                                                 \
            DNX_SAND_TBL_WRITE_REPT_BITS_WORD_LSB,                                                    \
            DNX_SAND_BITS_MASK(DNX_SAND_TBL_WRITE_REPT_BITS_WORD_MSB, DNX_SAND_TBL_WRITE_REPT_BITS_WORD_LSB)  \
          )

#define DNX_SAND_SET_TBL_WRITE_SIZE_BITS_IN_WORD(word)                                                \
          DNX_SAND_SET_FLD_IN_PLACE(                                                                       \
            word,                                                                                 \
            DNX_SAND_TBL_WRITE_SIZE_BITS_WORD_LSB,                                                    \
            DNX_SAND_BITS_MASK(DNX_SAND_TBL_WRITE_SIZE_BITS_WORD_MSB, DNX_SAND_TBL_WRITE_SIZE_BITS_WORD_LSB)  \
          )

#define DNX_SAND_SET_TBL_WRITE_RVRS_BITS_IN_WORD(word)                                                \
          DNX_SAND_SET_FLD_IN_PLACE(                                                                       \
            word,                                                                                 \
            DNX_SAND_TBL_WRITE_RVRS_BITS_WORD_LSB,                                                    \
            DNX_SAND_BITS_MASK(DNX_SAND_TBL_WRITE_RVRS_BITS_WORD_MSB, DNX_SAND_TBL_WRITE_RVRS_BITS_WORD_LSB)  \
          )

void 
  dnx_sand_indirect_set_nof_repetitions_unsafe(
    DNX_SAND_IN  int          unit,
    DNX_SAND_IN  uint32           nof_repetitions  
  );

/*
 * } DNX_SAND_ONE_INDIRECT_MODULE_BLOCK is one block of memory accessed by the device (address and size) {
 */
typedef struct
{
  uint32   offset;
  uint32    size; /* in longs */
  /**/
} DNX_SAND_INDIRECT_MEMORY_MAP;


/*
 * Indicator.
 */
typedef enum
{
  /*
   *     Do not touch the offset.
   *     All DNX_SAND_FE200, DNX_SAND_FAP10M modules.
   */
  DNX_SAND_INDIRECT_DONT_TOUCH_MODULE_BITS = 0,

  /*
   * DNX_SAND_INDIRECT_ERASE_MODULE_BITS -
   *     Set ZERO to the module part bits 27:30,
   *     when writing the offset part to the device.
   *     It applicable in some DNX_SAND_FAP20V/DNX_SAND_FAP21V modules.
   */
  DNX_SAND_INDIRECT_ERASE_MODULE_BITS = 1
} DNX_SAND_INDIRECT_MODULE_BITS;

/*
 * DNX_SAND_ONE_INDIRECT_MODULE_INFO is the information
 * we keep for every indirectly accessed module
 * in a DNX_SAND chip.
 * This way we can work differently with each module
 */
typedef struct
{
  uint32    module_index;
  /*
   * This is offset to the module read result
   * buffer (each module has a different one)
   * this register is directly accessed.
   * each module might have different read result size
   */
  uint32   read_result_offset;
  uint32    word_size; /* in bytes */

  /*
   * Module offset to the indirect access trigger
   */
  uint32 access_trigger;
  /*
   * Module offset to the indirect access address
   */
  uint32 access_address;
  /*
   * Module offset to  the indirect write buffer
   */
  uint32 write_buffer_offset;

  /*
   * Indicator.
   * DNX_SAND_INDIRECT_ERASE_MODULE_BITS -
   *     Set ZERO to the module part bits 27:30,
   *     when writing the offset part to the device.
   *     It applicable in some DNX_SAND_FAP20V modules.
   * DNX_SAND_INDIRECT_DONT_TOUCH_MODULE_BITS -
   *     Do not touch the offset.
   *     All DNX_SAND_FE200, DNX_SAND_FAP10M modules.
   */
  DNX_SAND_INDIRECT_MODULE_BITS  module_bits;

} DNX_SAND_INDIRECT_MODULE_INFO;


typedef struct
{
  /*
   * When the software will search for the relevant
   * indirect table information, it should find the
   * longest prefix that match the indirect address.
   * NOTE!!!:
   *   If a tables_prefix A is a subset of tables_prefix B,
   *   The structure that contain tables_prefix A, must
   *   come before the structure that contains tables_prefix B
   */
  uint32   tables_prefix;
  uint32   tables_prefix_nof_bits;
  /*
   * This is offset to the module read result
   * buffer (each module has a different one)
   * this register is directly accessed.
   * each module might have different read result size
   */
  uint32    word_size; /* in bytes */
  uint32   read_result_offset;
   /*
   * Module offset to  the indirect write buffer
   */
  uint32   write_buffer_offset;

}DNX_SAND_INDIRECT_TABLES_INFO;
/*
 * Structure hold all Indirect info.
 */
typedef struct
{
  /*
   * Indirect definitions of the device.
   * This will hold a pointer to an information array.
   * No allocation is done. That is, The one taht set this
   * pointer need to give a array that "live" in memeory.
   */
  DNX_SAND_INDIRECT_MODULE_INFO const* info_arr;

  /*
   * In some cases, the module information is not enough.
   * In those cases this will hold the device information.
   */
  DNX_SAND_INDIRECT_TABLES_INFO const* tables_info;
  /*
   * Indirect definitions of the device.
   * This will hold a pointer to an information array.
   * No allocation is done. That is, The one that set this
   * pointer need to give a array that "live" in memeory.
   */
  DNX_SAND_INDIRECT_MEMORY_MAP const* memory_map_arr;
  /*
   * Number of enrtries in Indirect_module_info[] array.
   */
  uint32  info_arr_max_index;

}DNX_SAND_INDIRECT_MODULE;

extern DNX_SAND_INDIRECT_MODULE
  Dnx_soc_indirect_module_arr[DNX_SAND_MAX_DEVICE] ;
extern uint32
  Dnx_soc_sand_nof_repetitions[DNX_SAND_MAX_DEVICE];


/*****************************************************
*NAME
* DNX_SAND_INDIRECT_WRITE_ACCESS_PTR
*TYPE:
*  PROC
*DATE:
*  11-Apr-06
*FUNCTION:
*  DNX_SAND low level indirect write function.
*  assumes that the parameters are checked and device
*  semaphore is taken.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN   int   unit -
*      Identifier of device to access.
*    DNX_SAND_IN   uint32* data_ptr -
*      Pointer to buffer for this procedure to load
*      from the data. Size of buffer must be at
*      least 'size'.
*    DNX_SAND_IN   uint32  offset -
*      Offset of first 32-bits register to read, relative to
*      'indirect' offset.
*    DNX_SAND_IN   uint32  size -
*      Number of bytes to read, beginning at 'offset'.
*      This must be a multiple of four (integral number
*      of uint32s).
*    DNX_SAND_IN   uint32   module_bits -
*      Each indirect module has module bits that specify the
*       indirect module ID.
*       Use module_bits == DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET,
*       to get those bits from the offset (normal operation)
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 -
*      See formatting rules in ERROR RETURN VALUES above.
*      If error code is not FAP10M_NO_ERR then
*        specific error codes:
*          None.
*      Otherwise, no error has been detected and device
*        has been written.
*  DNX_SAND_INDIRECT:
*    NONE
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
typedef
  DNX_SAND_RET
    (*DNX_SAND_INDIRECT_WRITE_ACCESS_PTR)(
      DNX_SAND_IN     int   unit,
      DNX_SAND_IN     uint32* data_ptr,
      DNX_SAND_IN     uint32  offset,
      DNX_SAND_IN     uint32  size,
      DNX_SAND_IN     uint32   module_bits
    );

/*****************************************************
*NAME
* DNX_SAND_INDIRECT_READ_ACCESS_PTR
*TYPE:
*  PROC
*DATE:
*  11-Apr-06
*FUNCTION:
*  DNX_SAND low level indirect read function.
*  assumes that the parameters are checked and device
*  semaphore is taken.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN     int   unit -
*      Identifier of device to access.
*    DNX_SAND_INOUT  uint32* result_ptr -
*      Pointer to buffer for this procedure to load
*      with read data. Size of buffer must be at
*      least 'size'.
*    DNX_SAND_IN     uint32  offset -
*      Offset of first 32-bits register to read, relative to
*      'indirect' offset.
*    DNX_SAND_IN     uint32  size -
*      Number of bytes to read, beginning at 'offset'.
*      This must be a multiple of four (integral number
*      of uint32s).
*    DNX_SAND_IN   uint32   module_bits -
*      Each indirect module has module bits that specify the
*       indirect module ID.
*       Use module_bits == DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET,
*       to get those bits from the offset (normal operation)

*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    unsigned short -
*      error indication
*  DNX_SAND_INDIRECT:
*    NONE
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
typedef
  DNX_SAND_RET
    (*DNX_SAND_INDIRECT_READ_ACCESS_PTR)(
      DNX_SAND_IN     int   unit,
      DNX_SAND_INOUT  uint32* result_ptr,
      DNX_SAND_IN     uint32  offset,
      DNX_SAND_IN     uint32  size,
      DNX_SAND_IN     uint32   module_bits
    );

typedef struct
{
  /*
   * Indirect Write function pointer to the device.
   */
  DNX_SAND_INDIRECT_WRITE_ACCESS_PTR indirect_write;

  /*
   * Indirect Read function pointer to the device.
   */
  DNX_SAND_INDIRECT_READ_ACCESS_PTR  indirect_read;
} DNX_SAND_INDIRECT_ACCESS;

/*****************************************************
*NAME
* dnx_sand_indirect_set_access_hook
*TYPE:
*  PROC
*DATE:
*  11-Apr-06
*FUNCTION:
*  Sets DNX_SAND private 'DNX_SAND_INDIRECT_ACCESS'.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN DNX_SAND_INDIRECT_ACCESS* indirect_access -
*      Pointer to indirect access function to the devices.
*      To be supplied by the BSP implementor.
*      If any of the specified indirect access
*      functions is set to NULL, the system's default
*      access function is used.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET.
*  DNX_SAND_INDIRECT:
*    None
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_indirect_set_access_hook(
    DNX_SAND_IN DNX_SAND_INDIRECT_ACCESS* indirect_access
  );
/*****************************************************
*NAME
* dnx_sand_indirect_get_access_hook
*TYPE:
*  PROC
*DATE:
*  11-Apr-06
*FUNCTION:
*  Gets DNX_SAND private 'DNX_SAND_INDIRECT_ACCESS'.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN DNX_SAND_INDIRECT_ACCESS* indirect_access -
*      Loaded with the indirect access function to the devices.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET
*  DNX_SAND_INDIRECT:
*    None
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_indirect_get_access_hook(
    DNX_SAND_OUT DNX_SAND_INDIRECT_ACCESS* indirect_access
  );


/*****************************************************
*NAME
* DNX_SAND_TBL_WRITE_PTR
*TYPE:
*  PROC
*DATE:
*  11-Apr-07
*FUNCTION:
*  DNX_SAND low level indirect write function.
*  assumes that the parameters are checked and device
*  semaphore is taken.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN   int   unit -
*      Identifier of device to access.
*    DNX_SAND_IN   uint32    *data_ptr -
*      Pointer to buffer for this procedure to load
*      from the data. Size of buffer must be at
*      least 'size'.
*    DNX_SAND_IN   uint32  offset -
*      Offset of first 32-bits register to read, relative to
*      'indirect' offset.
*    DNX_SAND_IN   uint32  size -
*      Number of bytes to read, beginning at 'offset'.
*      This must be a multiple of four (integral number
*      of uint32s).
*    DNX_SAND_IN   uint32   module_id -
*      Each indirect module has module bits that specify the
*       indirect module ID.
*       Use module_bits == DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET,
*       to get those bits from the offset (normal operation)
*    DNX_SAND_IN     uint32   entry_nof_bytes -
*      the size of the table to write to (in bytes).
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 -
*      See formatting rules in ERROR RETURN VALUES above.
*      If error code is not DNX_SAND_NO_ERR then
*        specific error codes:
*          None.
*      Otherwise, no error has been detected and device
*        has been written.
*  DNX_SAND_INDIRECT:
*    NONE
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
typedef
  DNX_SAND_RET
    (*DNX_SAND_TBL_WRITE_PTR)(
      DNX_SAND_IN     int   unit,
      DNX_SAND_IN     uint32    *data_ptr,
      DNX_SAND_IN     uint32    offset,
      DNX_SAND_IN     uint32    size,
      DNX_SAND_IN     uint32   module_id,
      DNX_SAND_IN     uint32    indirect_options
    );


/*****************************************************
*NAME
* DNX_SAND_TBL_READ_PTR
*TYPE:
*  PROC
*DATE:
*  11-Apr-06
*FUNCTION:
*  DNX_SAND low level indirect read function.
*  assumes that the parameters are checked and device
*  semaphore is taken.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN     int   unit -
*      Identifier of device to access.
*    DNX_SAND_INOUT  uint32    *result_ptr -
*      Pointer to buffer for this procedure to load
*      with read data. Size of buffer must be at
*      least 'size'.
*    DNX_SAND_IN     uint32  offset -
*      Offset of first 32-bits register to read, relative to
*      'indirect' offset.
*    DNX_SAND_IN     uint32  size -
*      Number of bytes to read, beginning at 'offset'.
*      This must be a multiple of four (integral number
*      of uint32s).
*    DNX_SAND_IN   uint32   module_id -
*      Each indirect module has module bits that specify the
*       indirect module ID.
*       Use module_bits == DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET,
*       to get those bits from the offset (normal operation)
*    DNX_SAND_IN     uint32   entry_nof_bytes -
*      the size of the table to write to (in bytes).
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    unsigned short -
*      error indication
*  DNX_SAND_INDIRECT:
*    NONE
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
typedef
  DNX_SAND_RET
    (*DNX_SAND_TBL_READ_PTR)(
      DNX_SAND_IN     int   unit,
      DNX_SAND_INOUT  uint32    *result_ptr,
      DNX_SAND_IN     uint32    offset,
      DNX_SAND_IN     uint32    size,
      DNX_SAND_IN     uint32   module_id,
      DNX_SAND_IN     uint32    indirect_options
    );

typedef struct
{
  /*
   * Write to Table function pointer.
   */
  DNX_SAND_TBL_WRITE_PTR write;

  /*
   * Read from Table function pointer.
   */
  DNX_SAND_TBL_READ_PTR  read;
} DNX_SAND_TBL_ACCESS;

/*****************************************************
*NAME
* dnx_sand_tbl_hook_set
*TYPE:
*  PROC
*DATE:
*  11-Apr-06
*FUNCTION:
*  Sets DNX_SAND private 'DNX_SAND_TBL_ACCESS'.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN DNX_SAND_TBL_ACCESS* tbl -
*      Pointer to indirect access function to the devices.
*      To be supplied by the BSP implementor.
*      If any of the specified indirect access
*      functions is set to NULL, the system's default
*      access function is used.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET.
*  DNX_SAND_INDIRECT:
*    None
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_tbl_hook_set(
    DNX_SAND_IN DNX_SAND_TBL_ACCESS* tbl
  );
/*****************************************************
*NAME
* dnx_sand_tbl_hook_get
*TYPE:
*  PROC
*DATE:
*  11-Apr-06
*FUNCTION:
*  Gets DNX_SAND private 'DNX_SAND_TBL_ACCESS'.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN DNX_SAND_TBL_ACCESS* tbl -
*      Loaded with the indirect access function to the devices.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET
*  DNX_SAND_INDIRECT:
*    None
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_tbl_hook_get(
    DNX_SAND_OUT DNX_SAND_TBL_ACCESS* tbl
  );

/*
 * Set the device specific indirect parameters.
 */
DNX_SAND_RET
  dnx_sand_indirect_set_info(
    DNX_SAND_IN int          unit,
    DNX_SAND_IN DNX_SAND_INDIRECT_MODULE* indirect_module
  );

/*
 * Clears unit indirect information.
 */
DNX_SAND_RET
  dnx_sand_indirect_clear_info(
    DNX_SAND_IN int  unit
  );

/*
 * Clears all dervices indirect information.
 */
DNX_SAND_RET
  dnx_sand_indirect_clear_info_all(
    void
  );

/*
 * Indirect access services
 * {
 */
DNX_SAND_RET
  dnx_sand_indirect_check_request_legal(
    DNX_SAND_IN int  unit,
    DNX_SAND_IN uint32 offset,
    DNX_SAND_IN uint32 size
  );
/*
 */
DNX_SAND_RET
  dnx_sand_indirect_verify_trigger_0(
    int  unit,
    uint32 offset,
    uint32 timeout /* in nano seconds */
  );
/*
 */
DNX_SAND_RET
  dnx_sand_indirect_write_address(
    int  unit,
    uint32 offset,
    uint32  read_not_write, /* 0-write, 1-read*/
    uint32  module_bits
  );
/*
 */
DNX_SAND_RET
  dnx_sand_indirect_write_value(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    uint32   offset,
    DNX_SAND_IN    uint32   *result_ptr,
    DNX_SAND_IN    uint32   size /* in bytes */
  );
/*
 */
DNX_SAND_RET
  dnx_sand_indirect_assert_trigger_1(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    uint32   offset
  );
/*
 */
DNX_SAND_RET
  dnx_sand_indirect_read_result(
    int  unit,
    uint32 *result_ptr,
    uint32 offset
  );
/*
 *
 */
DNX_SAND_RET
  dnx_sand_indirect_read_from_chip(
    DNX_SAND_IN     int   unit,
    DNX_SAND_INOUT  uint32* result_ptr,
    DNX_SAND_IN     uint32  offset,
    DNX_SAND_IN     uint32  size,
    DNX_SAND_IN     uint32   module_bits
  );


/*****************************************************
*NAME
* dnx_sand_tbl_read
*TYPE:
*  PROC
*DATE:
*  15/10/2007
*FUNCTION:
*  Read indirect data given the size of the table.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN int  unit -
*    DNX_SAND_IN uint32 *result_ptr -
*      The value that was read.
*    DNX_SAND_IN uint32 offset -
*      Offset in device / indirect table
*    DNX_SAND_IN uint32  size -
*      Size of data to write
*    DNX_SAND_IN  uint32     module_id -
*      id of the module that includes the table to read from.
*    DNX_SAND_IN uint32  entry_nof_bytes -
*      Width of table to read from (in bytes)
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    error indication
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*   The difference between this functions and the previous:
*   - Size of the table (tbl_width) is sent to the functions
*     as parameters rather than reading it from the tables info.
*   - The module bits are sent in a separated parameter, and not
*     in the offset parameter, so the offset doesn't include the
*     module bits in its higher bits.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_tbl_read(
    DNX_SAND_IN  int     unit,
    DNX_SAND_OUT uint32      *result_ptr,
    DNX_SAND_IN  uint32      offset,
    DNX_SAND_IN  uint32     size,
    DNX_SAND_IN  uint32     module_id,
    DNX_SAND_IN  uint32     entry_nof_bytes
  );


/*****************************************************
*NAME
* dnx_sand_tbl_read_unsafe
*TYPE:
*  PROC
*DATE:
*  01/SEP/2007
*FUNCTION:
*  Write direct / indirect data - unsafe procedure .
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN int  unit -
*    DNX_SAND_IN uint32 *result_ptr -
*      The value that was read.
*    DNX_SAND_IN uint32 offset -
*      Offset in device / indirect table
*    DNX_SAND_IN uint32  size -
*      Size of data to write
*    DNX_SAND_IN  uint32     module_id -
*      id of the module that includes the table to read from.
*    DNX_SAND_IN uint32  entry_nof_bytes -
*      Width of table to read from (in bytes)
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    error indication
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*   The difference between this functions and the previous:
*   - Size of the table (tbl_width) is sent to the functions
*     as parameters rather than reading it from the tables info.
*   - The module bits are sent in a separated parameter, and not
*     in the offset parameter, so the offset doesn't include the
*     module bits in its higher bits.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_tbl_read_unsafe(
    DNX_SAND_IN  int     unit,
    DNX_SAND_OUT uint32      *result_ptr,
    DNX_SAND_IN  uint32      offset,
    DNX_SAND_IN  uint32     size,
    DNX_SAND_IN  uint32     module_id,
    DNX_SAND_IN  uint32     entry_nof_bytes
  );


/*****************************************************
*NAME
* dnx_sand_tbl_write
*TYPE:
*  PROC
*DATE:
*  21/02/2007
*FUNCTION:
*  Write direct / indirect data.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN int  unit -
*    DNX_SAND_IN uint32 *data_ptr -
*      The value to write.
*    DNX_SAND_IN uint32 offset -
*      Offset in device / indirect table
*    DNX_SAND_IN uint32  size -
*      Size of data to write
*    DNX_SAND_IN  uint32     module_id -
*      id of the module that includes the table to read from.
*    DNX_SAND_IN uint32  entry_nof_bytes -
*      Width of table to write to  (in bytes)
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    error indication
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*   The difference between this functions and the previous:
*   - Size of the table (tbl_width) is sent to the functions
*     as parameters rather than reading it from the tables info.
*   - The module bits are sent in a separated parameter, and not
*     in the offset parameter, so the offset doesn't include the
*     module bits in its higher bits.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_tbl_write(
    DNX_SAND_IN  int     unit,
    DNX_SAND_IN  uint32      *data_ptr,
    DNX_SAND_IN  uint32      offset,
    DNX_SAND_IN  uint32     size,
    DNX_SAND_IN  uint32     module_id,
    DNX_SAND_IN  uint32     entry_nof_bytes
  );


/*****************************************************
*NAME
* dnx_sand_tbl_write_unsafe
*TYPE:
*  PROC
*DATE:
*  21/02/2007
*FUNCTION:
*  Write direct / indirect data - unsafe procedure.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN int  unit -
*    DNX_SAND_IN uint32 *data_ptr -
*      The value to write.
*    DNX_SAND_IN uint32 offset -
*      Offset in device / indirect table
*    DNX_SAND_IN uint32  size -
*      Size of data to write
*    DNX_SAND_IN  uint32     module_id -
*      id of the module that includes the table to write to.
*    SAMD_IN   uint32   indirect_options
*      bits 0 :11 - width of the table in longs.
*      bits 12:27 - number of desired repetitions.
*      bits 28:31 - number of indirect write registers of the odule suppose
*                   LSB should be written to the highest write register,
*                   otherwise 0.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    error indication
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*   The difference between this functions and the previous:
*   - Size of the table (tbl_width) is sent to the functions
*     as parameters rather than reading it from the tables info.
*   - The module bits are sent in a separated parameter, and not
*     in the offset parameter, so the offset doesn't include the
*     module bits in its higher bits.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_tbl_write_unsafe(
    DNX_SAND_IN  int     unit,
    DNX_SAND_IN  uint32      *data_ptr,
    DNX_SAND_IN  uint32      offset,
    DNX_SAND_IN  uint32     size,
    DNX_SAND_IN  uint32     module_id,
    DNX_SAND_IN  uint32      indirect_options
  );


/*
 *
 */
DNX_SAND_RET
  dnx_sand_indirect_write_value_ind_info(
    DNX_SAND_IN  int              unit,
    DNX_SAND_IN  uint32             reverse_order,
    DNX_SAND_IN  uint32             word_size,
    DNX_SAND_IN  uint32             *result_ptr,
    DNX_SAND_IN  DNX_SAND_INDIRECT_MODULE_INFO *ind_info,
             uint32             *device_base_address
  );


/*****************************************************
*NAME
* dnx_sand_tbl_read_from_chip
*TYPE:
*  PROC
*DATE:
*  03-Jun-03
*FUNCTION:
*  DNX_SAND low level indirect read function.
*  To be called from 'dnx_sand_mem_read()' or any other
*  driver calls after parameters are checked and device
*  semaphore is taken.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN     int   unit -
*      Identifier of device to access.
*    DNX_SAND_INOUT  uint32* result_ptr -
*      Pointer to buffer for this procedure to load
*      with read data. Size of buffer must be at
*      least 'size'.
*    DNX_SAND_IN     uint32  offset -
*      Offset of first 32-bits register to read, relative to
*      'indirect' offset.
*    DNX_SAND_IN     uint32  size -
*      Number of bytes to read, beginning at 'offset'.
*      This must be a multiple of four (integral number
*      of uint32s).
*    DNX_SAND_IN   uint32   module_bits -
*      Each indirect module has module bits that specify the
*       indirect module ID.
*       Use module_bits == DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET,
*       to get those bits from the offset (normal operation)

*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN int  unit -
*    DNX_SAND_IN uint32 *result_ptr -
*      The value that was read.
*    DNX_SAND_IN uint32 offset -
*      Offset in device / indirect table
*    DNX_SAND_IN uint32  size -
*      Size of data to write
*    DNX_SAND_IN  uint32     module_id -
*      id of the module that includes the table to read from.
*    DNX_SAND_IN uint32  entry_nof_bytes -
*      Width of table to read from
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  Calls to a callback function
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_tbl_read_from_chip(
    DNX_SAND_IN  int     unit,
    DNX_SAND_OUT uint32      *result_ptr,
    DNX_SAND_IN  uint32      offset,
    DNX_SAND_IN  uint32     size,
    DNX_SAND_IN  uint32     module_id,
    DNX_SAND_IN  uint32     entry_nof_bytes
  );


/*****************************************************
*NAME
* dnx_sand_tbl_write_to_chip
*TYPE:
*  PROC
*DATE:
*  03-Jun-03
*FUNCTION:
*  DNX_SAND low level indirect read function.
*  To be called from 'dnx_sand_mem_read()' or any other
*  driver calls after parameters are checked and device
*  semaphore is taken.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN int  unit -
*    DNX_SAND_IN uint32 *data_ptr -
*      The value to write.
*    DNX_SAND_IN uint32 offset -
*      Offset in device / indirect table
*    DNX_SAND_IN uint32  size -
*      Size of data to write
*    DNX_SAND_IN  uint32     module_id -
*      id of the module that includes the table to write to.
*    DNX_SAND_IN uint32  entry_nof_bytes -
*      Width of table to write to.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  Calls to a callback function
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_tbl_write_to_chip(
    DNX_SAND_IN  int     unit,
    DNX_SAND_OUT uint32      *data_ptr,
    DNX_SAND_IN  uint32      offset,
    DNX_SAND_IN  uint32     size,
    DNX_SAND_IN  uint32     module_id,
    DNX_SAND_IN  uint32     entry_nof_bytes
  );

/*****************************************************
*NAME
* DNX_SAND_INDIRECT_READ_ACCESS_PTR
*TYPE:
*  PROC
*DATE:
*  11-Apr-06
*FUNCTION:
*  DNX_SAND low level indirect read function.
*  assumes that the parameters are checked and device
*  semaphore is taken.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN     int   unit -
*      Identifier of device to access.
*    DNX_SAND_INOUT  uint32* result_ptr -
*      Pointer to buffer for this procedure to load
*      with read data. Size of buffer must be at
*      least 'size'.
*    DNX_SAND_IN     uint32  offset -
*      Offset of first 32-bits register to read, relative to
*      'indirect' offset.
*    DNX_SAND_IN     uint32  size -
*      Number of bytes to read, beginning at 'offset'.
*      This must be a multiple of four (integral number
*      of uint32s).
*    DNX_SAND_IN   uint32   module_bits -
*      Each indirect module has module bits that specify the
*       indirect module ID.
*       Use module_bits == DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET,
*       to get those bits from the offset (normal operation)

*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    unsigned short -
*      error indication
*  DNX_SAND_INDIRECT:
*    NONE
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_indirect_read_low(
    DNX_SAND_IN     int   unit,
    DNX_SAND_INOUT  uint32* result_ptr,
    DNX_SAND_IN     uint32  offset,
    DNX_SAND_IN     uint32  size,
    DNX_SAND_IN     uint32   module_bits
  );


/*****************************************************
*NAME
* dnx_sand_tbl_read_low
*TYPE:
*  PROC
*DATE:
*  11-Apr-06
*FUNCTION:
*  DNX_SAND low level indirect read function.
*  assumes that the parameters are checked and device
*  semaphore is taken.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN     int   unit -
*      Identifier of device to access.
*    DNX_SAND_INOUT  uint32    *result_ptr -
*      Pointer to buffer for this procedure to load
*      with read data. Size of buffer must be at
*      least 'size'.
*    DNX_SAND_IN     uint32  offset -
*      Offset of first 32-bits register to read, relative to
*      'indirect' offset.
*    DNX_SAND_IN     uint32  size -
*      Number of bytes to read, beginning at 'offset'.
*      This must be a multiple of four (integral number
*      of uint32s).
*    DNX_SAND_IN   uint32   module_bits -
*      Each indirect module has module bits that specify the
*       indirect module ID.
*       Use module_bits == DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET,
*       to get those bits from the offset (normal operation)
*    DNX_SAND_IN   uint32   entry_nof_bytes
*      width of the table in longs
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    unsigned short -
*      error indication
*  DNX_SAND_INDIRECT:
*    NONE
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_tbl_read_low(
    DNX_SAND_IN     int   unit,
    DNX_SAND_INOUT  uint32    *result_ptr,
    DNX_SAND_IN     uint32    offset,
    DNX_SAND_IN     uint32    size,
    DNX_SAND_IN     uint32   module_id,
    DNX_SAND_IN     uint32    indirect_options
  );


/*
 *
 */
#define DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET 0xFF

DNX_SAND_RET
  dnx_sand_indirect_write_to_chip(
    DNX_SAND_IN   int   unit,
    DNX_SAND_IN   uint32* data_ptr,
    DNX_SAND_IN   uint32  offset,
    DNX_SAND_IN   uint32  size,
    DNX_SAND_IN   uint32   module_bits
  ) ;
/*
 *
 */
/*****************************************************
*NAME
* DNX_SAND_INDIRECT_WRITE_ACCESS_PTR
*TYPE:
*  PROC
*DATE:
*  11-Apr-06
*FUNCTION:
*  DNX_SAND low level indirect write function.
*  assumes that the parameters are checked and device
*  semaphore is taken.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN   int   unit -
*      Identifier of device to access.
*    DNX_SAND_IN   uint32* data_ptr -
*      Pointer to buffer for this procedure to load
*      from the data. Size of buffer must be at
*      least 'size'.
*    DNX_SAND_IN   uint32  offset -
*      Offset of first 32-bits register to read, relative to
*      'indirect' offset.
*    DNX_SAND_IN   uint32  size -
*      Number of bytes to read, beginning at 'offset'.
*      This must be a multiple of four (integral number
*      of uint32s).
*    DNX_SAND_IN   uint32   module_bits -
*      Each indirect module has module bits that specify the
*       indirect module ID.
*       Use module_bits == DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET,
*       to get those bits from the offset (normal operation)
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 -
*      See formatting rules in ERROR RETURN VALUES above.
*      If error code is not DNX_SAND_NO_ERR then
*        specific error codes:
*          None.
*      Otherwise, no error has been detected and device
*        has been written.
*  DNX_SAND_INDIRECT:
*    NONE
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_indirect_write_low(
    DNX_SAND_IN     int   unit,
    DNX_SAND_IN     uint32* data_ptr,
    DNX_SAND_IN     uint32  offset,
    DNX_SAND_IN     uint32  size,
    DNX_SAND_IN     uint32   module_bits
  );
/*****************************************************
*NAME
* dnx_sand_tbl_write_low
*TYPE:
*  PROC
*DATE:
*  11-Apr-06
*FUNCTION:
*  DNX_SAND low level indirect write function.
*  assumes that the parameters are checked and device
*  semaphore is taken.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN   int   unit -
*      Identifier of device to access.
*    DNX_SAND_IN   uint32    *data_ptr -
*      Pointer to buffer for this procedure to load
*      from the data. Size of buffer must be at
*      least 'size'.
*    DNX_SAND_IN   uint32    offset -
*      Offset of first 32-bits register to read, relative to
*      'indirect' offset.
*    DNX_SAND_IN   uint32  size -
*      Number of bytes to read, beginning at 'offset'.
*      This must be a multiple of four (integral number
*      of uint32s).
*    DNX_SAND_IN   uint32   module_bits -
*      Each indirect module has module bits that specify the
*       indirect module ID.
*       Use module_bits == DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET,
*       to get those bits from the offset (normal operation)
*    SAMD_IN   uint32   indirect_options
*      bits 0 :11 - width of the table in longs.
*      bits 12:27 - number of desired repetitions.
*      bits 28:31 - number of indirect write registers of the odule suppose
*                   LSB should be written to the highest write register,
*                   otherwise 0.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 -
*      See formatting rules in ERROR RETURN VALUES above.
*      If error code is not DNX_SAND_NO_ERR then
*        specific error codes:
*          None.
*      Otherwise, no error has been detected and device
*        has been written.
*  DNX_SAND_INDIRECT:
*    NONE
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_tbl_write_low(
    DNX_SAND_IN     int   unit,
    DNX_SAND_IN     uint32    *data_ptr,
    DNX_SAND_IN     uint32    offset,
    DNX_SAND_IN     uint32    size,
    DNX_SAND_IN     uint32   module_id,
    DNX_SAND_IN     uint32    indirect_options
  );

/*
 *
 */
DNX_SAND_RET
  dnx_sand_indirect_read_modify_write(
    DNX_SAND_IN     int  unit,
    DNX_SAND_IN     uint32 offset,
    DNX_SAND_IN     uint32 shift,
    DNX_SAND_IN     uint32 mask,
    DNX_SAND_IN     uint32 data_to_write
  ) ;

/*
 * Indirect access services
 * }
 */

/*
 * { Support in user defined Indirect access
 */

DNX_SAND_RET
  dnx_sand_indirect_get_access_trigger(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32 offset,
    DNX_SAND_OUT uint32 *access_trigger
  );

DNX_SAND_RET
  dnx_sand_indirect_get_word_size(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32 offset,
    DNX_SAND_OUT uint32  *entry_nof_bytes
  );

DNX_SAND_RET
  dnx_sand_indirect_get_inner_struct(
    int unit,
    uint32  offset,
    uint32 module_bits,
    DNX_SAND_INDIRECT_MODULE_INFO *ind_info
  );

DNX_SAND_RET
  dnx_sand_indirect_write_address_ind_info(
    DNX_SAND_IN  int              unit,
    DNX_SAND_IN  uint32             offset,
    DNX_SAND_IN  uint32              read_not_write,
    DNX_SAND_IN  DNX_SAND_INDIRECT_MODULE_INFO *ind_info,
    DNX_SAND_IN  uint32              module_bits,
             uint32             *device_base_address
  );


DNX_SAND_RET
  dnx_sand_indirect_read_result_ind_info(
    DNX_SAND_OUT uint32             *result_ptr,
    DNX_SAND_IN  uint32             word_size,
    DNX_SAND_IN  uint32             reverse_order,
    DNX_SAND_IN  DNX_SAND_INDIRECT_MODULE_INFO *ind_info,
             uint32             *device_base_address
  );

#if DNX_SAND_DEBUG
void
  dnx_sand_indirect_write_to_chip_print_when_write(
    DNX_SAND_IN   int   unit,
    DNX_SAND_IN   uint32* data_ptr,
    DNX_SAND_IN   uint32  offset,
    DNX_SAND_IN   uint32  size,
    DNX_SAND_IN   uint32  word_size
  );
#endif
/*
 * } Support in user defined Indirect access
 */
#ifdef  __cplusplus
}
#endif

#endif
