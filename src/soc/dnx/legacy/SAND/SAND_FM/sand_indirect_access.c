/* $Id: sand_indirect_access.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/
/* $Id: sand_indirect_access.c,v 1.9 Broadcom SDK $
 */


#include <shared/bsl.h>
#include <soc/dnx/legacy/drv.h>



#include <soc/dnx/legacy/SAND/SAND_FM/sand_indirect_access.h>
#include <soc/dnx/legacy/SAND/SAND_FM/sand_trigger.h>

#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>

#include <soc/dnx/legacy/SAND/Management/sand_low_level.h>
#include <soc/dnx/legacy/SAND/Management/sand_chip_descriptors.h>
#include <soc/dnx/legacy/SAND/SAND_FM/sand_chip_defines.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>


#if DNX_SAND_DEBUG
#include <soc/dnx/legacy/SAND/SAND_FM/sand_mem_access.h>
#endif

#define DNX_SAND_TBL_READ_BIT  1
#define DNX_SAND_TBL_WRITE_BIT 0

#define DNX_SAND_INDRCT_READ_MASK  0x80000000 
#define DNX_SAND_INDRCT_WRITE_MASK 0x7fffffff

uint32
  Dnx_soc_sand_nof_repetitions[DNX_SAND_MAX_DEVICE];

void 
  dnx_sand_indirect_set_nof_repetitions_unsafe(
    DNX_SAND_IN  int          unit,
    DNX_SAND_IN  uint32           nof_repetitions  
  )
{
  Dnx_soc_sand_nof_repetitions[unit] = nof_repetitions;
}

uint32 
  dnx_sand_indirect_get_nof_repetitions(
    DNX_SAND_IN  int          unit
  )
{
  return Dnx_soc_sand_nof_repetitions[unit];
}

/*
 * { Specific information about indirect accessing.
 *   This information may vary from one device type to another.
 */
/*
 * The structure 'Dnx_soc_indirect_module_arr' should not be called
 * directly, but only with the module accessories!!!
 */
DNX_SAND_INDIRECT_MODULE
  Dnx_soc_indirect_module_arr[DNX_SAND_MAX_DEVICE] ;

/*
 * 2 function pointers.
 * These are the indirect access hooks user may supply,
 * in case not, the default indirect access functions are used.
 */
DNX_SAND_INDIRECT_ACCESS
  Dnx_soc_sand_indirect_access =
  {
    dnx_sand_indirect_write_low,
    dnx_sand_indirect_read_low
  } ;

/*
 * 2 function pointers.
 * These are the indirect access hooks user may supply,
 * in case not, the default indirect access functions are used.
 * REMARK: the difference between this functions and the previous:
 *   - Size of the table (word_size) is sent to the functions
 *     as parameters rather than reading it from the tables info.
 *   - The module bits are sent in a separated parameter, and not
 *     in the offset parameter, so the offset doesn't include the
 *     module bits in its higher bits.
*/
DNX_SAND_TBL_ACCESS
  Dnx_soc_sand_tbl_access =
  {
    dnx_sand_tbl_write_low,
    dnx_sand_tbl_read_low
  } ;

/*****************************************************
* see remarks & definitions in the dnx_sand_low_level.h
*****************************************************/
DNX_SAND_RET
  dnx_sand_indirect_set_access_hook(
    DNX_SAND_IN DNX_SAND_INDIRECT_ACCESS* indirect_access
  )
{
  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK ;
  if (NULL == indirect_access)
  {
    dnx_sand_ret = DNX_SAND_NULL_POINTER_ERR ;
    goto exit ;
  }
  /*
   * The driver may not run at this stage.
   */
  dnx_sand_os_task_lock() ;
  if (indirect_access->indirect_write)
  {
    Dnx_soc_sand_indirect_access.indirect_write = indirect_access->indirect_write ;
  }
  else
  {
    Dnx_soc_sand_indirect_access.indirect_write = dnx_sand_indirect_write_low ;
  }
  if (indirect_access->indirect_read)
  {
    Dnx_soc_sand_indirect_access.indirect_read = indirect_access->indirect_read ;
  }
  else
  {
    Dnx_soc_sand_indirect_access.indirect_read = dnx_sand_indirect_read_low ;
  }
  dnx_sand_os_task_unlock() ;
exit:
  return dnx_sand_ret ;
}

/*****************************************************
* see remarks & definitions in the dnx_sand_low_level.h
*****************************************************/
DNX_SAND_RET
  dnx_sand_indirect_get_access_hook(
    DNX_SAND_OUT DNX_SAND_INDIRECT_ACCESS* indirect_access
  )
{
  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK ;

  if (NULL != indirect_access)
  {
    indirect_access->indirect_write = Dnx_soc_sand_indirect_access.indirect_write ;
    indirect_access->indirect_read = Dnx_soc_sand_indirect_access.indirect_read ;
  }
  else
  {
    dnx_sand_ret = DNX_SAND_NULL_POINTER_ERR ;
    goto exit ;
  }

exit:
  return dnx_sand_ret ;
}


/*****************************************************
* see remarks & definitions in the dnx_sand_low_level.h
*****************************************************/
DNX_SAND_RET
  dnx_sand_tbl_hook_get(
    DNX_SAND_OUT DNX_SAND_TBL_ACCESS* tbl
  )
{
  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK ;

  if (NULL != tbl)
  {
    tbl->write = Dnx_soc_sand_tbl_access.write ;
    tbl->read = Dnx_soc_sand_tbl_access.read ;
  }
  else
  {
    dnx_sand_ret = DNX_SAND_NULL_POINTER_ERR ;
    goto exit ;
  }

exit:
  return dnx_sand_ret ;
}



/*****************************************************
* see remarks & definitions in the dnx_sand_low_level.h
*****************************************************/
DNX_SAND_RET
  dnx_sand_tbl_hook_set(
    DNX_SAND_IN DNX_SAND_TBL_ACCESS* tbl
  )
{
  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK ;
  if (NULL == tbl)
  {
    dnx_sand_ret = DNX_SAND_NULL_POINTER_ERR ;
    goto exit ;
  }
  /*
   * The driver may not run at this stage.
   */
  dnx_sand_os_task_lock() ;
  if (tbl->write)
  {
    Dnx_soc_sand_tbl_access.write = tbl->write ;
  }
  else
  {
    Dnx_soc_sand_tbl_access.write = dnx_sand_tbl_write_low ;
  }
  if (tbl->read)
  {
    Dnx_soc_sand_tbl_access.read = tbl->read ;
  }
  else
  {
    Dnx_soc_sand_tbl_access.read = dnx_sand_tbl_read_low ;
  }
  dnx_sand_os_task_unlock() ;
exit:
  return dnx_sand_ret ;
}


/*
 * Should only be called from dnx_sand_indirect_get_size()
 */
DNX_SAND_RET
  dnx_sand_indirect_get_info_from_tables_info(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32 offset,
    DNX_SAND_OUT uint32  *word_size,
    DNX_SAND_OUT uint32 *read_result_offset,
    DNX_SAND_OUT uint32 *write_buffer_offset
  )
{
  DNX_SAND_RET
    ex = DNX_SAND_INDIRECT_CANT_GET_INFO_ERR;
  uint32
    table_i=0;
  uint32
    prefix_lsb,
    prefix_nof_bits,
    prefix_msb,
    prefix_mask;



  *word_size = 0;
  if( Dnx_soc_indirect_module_arr[unit].tables_info == NULL)
  {
    ex = DNX_SAND_INDIRECT_CANT_GET_INFO_ERR;
    goto exit;
  }

  while(Dnx_soc_indirect_module_arr[unit].tables_info[table_i].word_size)
  {
    prefix_msb = 31;
    prefix_nof_bits =
      Dnx_soc_indirect_module_arr[unit].tables_info[table_i].tables_prefix_nof_bits;
    prefix_lsb = prefix_msb + 1 - prefix_nof_bits;
    prefix_mask = (((uint32)DNX_SAND_BIT(prefix_msb) - DNX_SAND_BIT(prefix_lsb)) + DNX_SAND_BIT(prefix_msb));

    if((offset & prefix_mask) ==
       Dnx_soc_indirect_module_arr[unit].tables_info[table_i].tables_prefix
      )
    {
      *word_size =
        Dnx_soc_indirect_module_arr[unit].tables_info[table_i].word_size;
      *read_result_offset =
        Dnx_soc_indirect_module_arr[unit].tables_info[table_i].read_result_offset;
      *write_buffer_offset =
        Dnx_soc_indirect_module_arr[unit].tables_info[table_i].write_buffer_offset;

      ex = DNX_SAND_OK;
      goto exit;
    }
    table_i++;
  }

exit:
  return ex;
}
/*
 * Accessories for the 'Dnx_soc_indirect_module_arr' structure.
 */
DNX_SAND_RET
  dnx_sand_indirect_get_word_size(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32 offset,
    DNX_SAND_OUT uint32  *word_size
  )
{
  uint32 module;
  uint32 size = 0;
  uint32 read_offset;
  uint32 write_offset;
  DNX_SAND_RET ex=DNX_SAND_OK;

  module = DNX_SAND_GET_FLD_FROM_PLACE(offset, DNX_SAND_MODULE_SHIFT, DNX_SAND_MODULE_MASK) ;
  if (Dnx_soc_indirect_module_arr[unit].info_arr_max_index < module)
  {
    ex = DNX_SAND_NO_SUCH_INDIRECT_MODULE_ERR;
    goto exit;
  }
  size = Dnx_soc_indirect_module_arr[unit].info_arr[module].word_size;
  if(!size)
  {
    ex =
      dnx_sand_indirect_get_info_from_tables_info(
        unit, offset, &size, &read_offset, &write_offset
      );
  }
exit:
  *word_size = size;
  return ex;
}

/*****************************************************
*NAME:
* dnx_sand_indirect_get_inner_struct
*DATE:
* 01/SEP/2007
*FUNCTION:
*  Get the parameters for internal use by the module.
*  Getting it once helps to optimize memory-access time
*INPUT:
*  DNX_SAND_DIRECT:
*     uint32 unit -
*       device to retrieve information from.
*     uint32  offset -
*       offset of module.
*     DNX_SAND_INDIRECT_MODULE_INFO *ind_info -
*       Pointer to the struct to return information to.
*
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   long  -
*     Non zero if error
*
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/


DNX_SAND_RET
   dnx_sand_indirect_get_inner_struct(
     int unit,
     uint32  offset,
     uint32 module_bits,
     DNX_SAND_INDIRECT_MODULE_INFO *ind_info
   )
{
  uint32
    module;
  uint32
    word_size;
  uint32
    offset_for_module;
  DNX_SAND_RET
    ex=DNX_SAND_OK;

  if(module_bits == DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET)
  {
    offset_for_module = offset;
  }
  else
  {
    offset_for_module = module_bits << DNX_SAND_MODULE_SHIFT;
  }


  module = DNX_SAND_GET_FLD_FROM_PLACE(offset_for_module, DNX_SAND_MODULE_SHIFT, DNX_SAND_MODULE_MASK) ;
  if (Dnx_soc_indirect_module_arr[unit].info_arr_max_index < module)
  {
    ex = DNX_SAND_NO_SUCH_INDIRECT_MODULE_ERR;
    goto exit;
  }

  word_size = Dnx_soc_indirect_module_arr[unit].info_arr[module].word_size;
  if (!word_size)
  {
    ex =
      dnx_sand_indirect_get_info_from_tables_info(
        unit,
        offset,
        &(ind_info->word_size),
        &(ind_info->read_result_offset),
        &(ind_info->write_buffer_offset)
     );
  }
  else
  {
    ind_info->word_size = word_size;
    ind_info->write_buffer_offset = Dnx_soc_indirect_module_arr[unit].info_arr[module].write_buffer_offset;
    ind_info->read_result_offset = Dnx_soc_indirect_module_arr[unit].info_arr[module].read_result_offset;
  }
  if (ex != DNX_SAND_OK)
  {
    goto exit ;
  }

  ind_info->access_trigger = Dnx_soc_indirect_module_arr[unit].info_arr[module].access_trigger;

  ind_info->module_bits = Dnx_soc_indirect_module_arr[unit].info_arr[module].module_bits;

  ind_info->module_index = Dnx_soc_indirect_module_arr[unit].info_arr[module].module_index;

  ind_info->access_address = Dnx_soc_indirect_module_arr[unit].info_arr[module].access_address;

exit:
  return ex;
}

/*
 * Checks that
 *   If a tables_prefix A is a subset of tables_prefix B,
 *   The structure that contain tables_prefix A, must
 *   come before the structure that contains tables_prefix B
 */
DNX_SAND_RET
  dnx_sand_indirect_validate_tables_info(
    DNX_SAND_IN DNX_SAND_INDIRECT_TABLES_INFO *tables_info
  )
{
  uint32 table_i =0, table_j =0;
  uint32
    prefix_msb,
    prefix_i,
    prefix_i_lsb,
    prefix_i_nof_bits,
    prefix_i_mask,
    prefix_j,
    prefix_j_nof_bits;
  DNX_SAND_RET ex = DNX_SAND_OK;

  if(tables_info == NULL)
  {
    ex = DNX_SAND_INDIRECT_CANT_GET_INFO_ERR;
    goto exit;
  }

  /*
  * prefix_i is before prefix_j in the table.
  * Therefore, it is need to be checked,
  * that prefix_j is not a subset of prefix_i
  */
  prefix_msb = 31;
  table_i = 0;
  while(tables_info[table_i].word_size)
  {
    prefix_i          = tables_info[table_i].tables_prefix;
    prefix_i_nof_bits = tables_info[table_i].tables_prefix_nof_bits;
    prefix_i_lsb      = prefix_msb + 1 - prefix_i_nof_bits;
    prefix_i_mask     = (((uint32)DNX_SAND_BIT(prefix_msb) - DNX_SAND_BIT(prefix_i_lsb)) + DNX_SAND_BIT(prefix_msb));
    table_j = ++table_i;
    while(tables_info[table_j].word_size)
    {
      prefix_j          = tables_info[table_j].tables_prefix;
      prefix_j_nof_bits = tables_info[table_j].tables_prefix_nof_bits;
      table_j++;

      if(prefix_j_nof_bits > prefix_i_nof_bits)
      {
        if((prefix_i_mask & prefix_j) == prefix_i)
        {
          ex = DNX_SAND_INDIRECT_TABLES_INFO_ORDER_ERR;
          goto exit;
        }
      }
    }
  }
exit:
  return ex;
}

DNX_SAND_RET
  dnx_sand_indirect_get_read_info(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32 offset,
    DNX_SAND_OUT uint32 *read_result_offset,
    DNX_SAND_OUT uint32  *word_size
  )
{
  uint32 module;
  uint32 size = 0;
  uint32 read_offset = 0;
  uint32 write_offset;
  DNX_SAND_RET ex=DNX_SAND_OK;

  module = DNX_SAND_GET_FLD_FROM_PLACE(offset, DNX_SAND_MODULE_SHIFT, DNX_SAND_MODULE_MASK) ;
  if (Dnx_soc_indirect_module_arr[unit].info_arr_max_index < module)
  {
    ex = DNX_SAND_NO_SUCH_INDIRECT_MODULE_ERR;
    goto exit;
  }
  size = Dnx_soc_indirect_module_arr[unit].info_arr[module].word_size;
  read_offset = Dnx_soc_indirect_module_arr[unit].info_arr[module].read_result_offset;
  if(!size)
  {
    ex =
      dnx_sand_indirect_get_info_from_tables_info(
        unit, offset, &size, &read_offset, &write_offset
      );
  }
exit:
  *word_size = size;
  *read_result_offset = read_offset;
  return ex;
}

DNX_SAND_RET
  dnx_sand_indirect_get_write_info(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32 offset,
    DNX_SAND_OUT uint32 *write_buffer_offset,
    DNX_SAND_OUT uint32 *word_size
  )
{
  uint32 module;
  uint32 size = 0;
  uint32 read_offset = 0;
  uint32 write_offset = 0;
  DNX_SAND_RET ex=DNX_SAND_OK;

  module = DNX_SAND_GET_FLD_FROM_PLACE(offset, DNX_SAND_MODULE_SHIFT, DNX_SAND_MODULE_MASK) ;
  if (Dnx_soc_indirect_module_arr[unit].info_arr_max_index < module)
  {
    ex = DNX_SAND_NO_SUCH_INDIRECT_MODULE_ERR;
    goto exit;
  }
  size = Dnx_soc_indirect_module_arr[unit].info_arr[module].word_size;
  write_offset = Dnx_soc_indirect_module_arr[unit].info_arr[module].write_buffer_offset;
  if(!size)
  {
    ex =
      dnx_sand_indirect_get_info_from_tables_info(
        unit, offset, &size, &read_offset, &write_offset
      );
  }
exit:
  *word_size = size;
  *write_buffer_offset = write_offset;
  return ex;
}

DNX_SAND_RET
  dnx_sand_indirect_get_access_trigger(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32 offset,
    DNX_SAND_OUT uint32 *access_trigger
  )
{
  uint32 module;
  DNX_SAND_RET ex=DNX_SAND_OK;

  module = DNX_SAND_GET_FLD_FROM_PLACE(offset, DNX_SAND_MODULE_SHIFT, DNX_SAND_MODULE_MASK) ;
  if (Dnx_soc_indirect_module_arr[unit].info_arr_max_index < module)
  {
    ex = DNX_SAND_NO_SUCH_INDIRECT_MODULE_ERR;
    goto exit;
  }
  *access_trigger =
    Dnx_soc_indirect_module_arr[unit].info_arr[module].access_trigger;
exit:
  return ex;
}

uint32
  dnx_sand_indirect_get_info_arr_max_index(
    DNX_SAND_IN  int  unit
  )
{
  return Dnx_soc_indirect_module_arr[unit].info_arr_max_index;
}


DNX_SAND_RET
  dnx_sand_indirect_get_access_address(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32 offset,
    DNX_SAND_OUT uint32 *access_address
  )
{
  uint32 module;
  DNX_SAND_RET ex=DNX_SAND_OK;

  module = DNX_SAND_GET_FLD_FROM_PLACE(offset, DNX_SAND_MODULE_SHIFT, DNX_SAND_MODULE_MASK) ;
  if (Dnx_soc_indirect_module_arr[unit].info_arr_max_index < module)
  {
    ex = DNX_SAND_NO_SUCH_INDIRECT_MODULE_ERR;
    goto exit;
  }
  *access_address =
    Dnx_soc_indirect_module_arr[unit].info_arr[module].access_address;
exit:
  return ex;
}

DNX_SAND_RET
  dnx_sand_indirect_get_module_bits(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32 offset,
    DNX_SAND_OUT DNX_SAND_INDIRECT_MODULE_BITS *erase_module_bits
  )
{
  uint32 module;
  DNX_SAND_RET ex=DNX_SAND_OK;

  module = DNX_SAND_GET_FLD_FROM_PLACE(offset, DNX_SAND_MODULE_SHIFT, DNX_SAND_MODULE_MASK) ;
  if (Dnx_soc_indirect_module_arr[unit].info_arr_max_index < module)
  {
    ex = DNX_SAND_NO_SUCH_INDIRECT_MODULE_ERR;
    goto exit;
  }
  *erase_module_bits =
    Dnx_soc_indirect_module_arr[unit].info_arr[module].module_bits;
exit:
  return ex;
}

DNX_SAND_RET
  dnx_sand_indirect_check_offset_is_legal(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32 offset,
    DNX_SAND_IN  uint32 size

  )
{
  uint32 module;
  int iii;
  uint32 cur_offset;
  uint32  cur_size;
  DNX_SAND_RET ex;

  module = DNX_SAND_GET_FLD_FROM_PLACE(offset, DNX_SAND_MODULE_SHIFT, DNX_SAND_MODULE_MASK) ;
  if (Dnx_soc_indirect_module_arr[unit].info_arr_max_index < module)
  {
    ex = DNX_SAND_NO_SUCH_INDIRECT_MODULE_ERR;
    goto exit;
  }
  /* check that offset and size are legal*/
  iii = 0;
  DNX_SAND_LOOP_FOREVER
  {
    cur_size = Dnx_soc_indirect_module_arr[unit].memory_map_arr[iii].size;
    cur_offset = Dnx_soc_indirect_module_arr[unit].memory_map_arr[iii].offset;
    if (!cur_offset && !cur_size)
    {
      ex = DNX_SAND_INDIRECT_OFFSET_SIZE_MISMATCH_ERR;
      break; /* didn't find the offset + size at hand => error */
    }
    if( (offset >= cur_offset) && (offset + size <= cur_offset + cur_size) )
    {
      ex = DNX_SAND_OK;
      break;
    }
    ++iii;
  }
exit:
  return ex;
}



/*
 * Set the device specific indirect parameters.
 */
DNX_SAND_RET
  dnx_sand_indirect_set_info(
    DNX_SAND_IN int  unit,
    DNX_SAND_IN DNX_SAND_INDIRECT_MODULE* indirect_module
  )
{
  DNX_SAND_RET
    ex ;
  ex = DNX_SAND_OK ;
  if (NULL == indirect_module)
  {
  /* Skip verification for BCM: ex = DNX_SAND_NULL_POINTER_ERR ;*/
    goto exit ;
  }
  ex =
    dnx_sand_indirect_validate_tables_info(
      indirect_module->tables_info
    );
  if(ex != DNX_SAND_OK)
  {
    goto exit;
  }
  Dnx_soc_indirect_module_arr[unit] = *indirect_module ;
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_INDIRECT_SET_INFO,
        "General error in dnx_sand_indirect_set_info()",0,0,0,0,0,0) ;
  return ex ;
}
/*
 * Clears unit indirect information.
 */
DNX_SAND_RET
  dnx_sand_indirect_clear_info(
    DNX_SAND_IN int  unit
  )
{
  DNX_SAND_RET
    ex ;
  ex = DNX_SAND_OK ;
  if (unit < DNX_SAND_MAX_DEVICE)
  {
    Dnx_soc_indirect_module_arr[unit].info_arr           = NULL ;
    Dnx_soc_indirect_module_arr[unit].tables_info        = NULL ;
    Dnx_soc_indirect_module_arr[unit].info_arr_max_index = 0 ;
  }
  else
  {
    ex = DNX_SAND_ERR ;
    goto exit ;
  }
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_INDIRECT_CLEAR_INFO,
        "General error in dnx_sand_indirect_clear_info()",0,0,0,0,0,0) ;
  return ex ;
}
/*
 * Clears all devices indirect information.
 */
DNX_SAND_RET
  dnx_sand_indirect_clear_info_all(
    void
  )
{
  DNX_SAND_RET
    ex ;
  uint32
    ii ;
  ex = DNX_SAND_OK ;
  for (ii = 0 ; ii < DNX_SAND_MAX_DEVICE ; ii++)
  {
    dnx_sand_indirect_clear_info(ii) ;
  }
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_INDIRECT_CLEAR_INFO_ALL,
        "General error in dnx_sand_indirect_clear_info_all()",0,0,0,0,0,0) ;
  return ex ;
}
/*
 * }
 */
/*
 * Indirect access services
 * {
 */
/*****************************************************
*NAME:
* dnx_sand_indirect_check_request_legal
*DATE:
* 27/OCT/2002
*FUNCTION:
*  Checks the an indirect request has legal params
*  1. Check the offset and size against the
*     'Dnx_soc_indirect_module_arr[unit].memory_map_arr[]' array.
*  2. Check that the buffer is aligned with the indirect
*     memory table size.
*     Most of the tables are uint32 sized (4 bytes).
*     Few rare tables are longer.
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32  unit -  device id to check.
*    uint32 offset - indirect address to read
*    uint32 size    - in longs
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   DNX_SAND_ERR  -
*     DNX_SAND_OK -  if legal
*     DNX_SAND_ERR - illegal address or size
*  DNX_SAND_INDIRECT:
*REMARKS:
* Assuming checked before reading / writing of one word
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_indirect_check_request_legal(
    DNX_SAND_IN int  unit,
    DNX_SAND_IN uint32 offset,
    DNX_SAND_IN uint32 size
  )
{
  DNX_SAND_RET ex;
  /**/
  ex = DNX_SAND_ERR;

  ex = dnx_sand_indirect_check_offset_is_legal(unit, offset,size);
  if(ex != DNX_SAND_OK)
  {
    goto exit;
  }


exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_INDIRECT_CHECK_REQUEST_LEGAL,
        "General error in dnx_sand_indirect_check_request_legal()",0,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* dnx_sand_indirect_verify_trigger_0
*DATE:
* 27/OCT/2002
*FUNCTION:
*  verify that the indirect trigger is 0
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32  unit,
*      device to check
*    uint32 timeout       -
*      time to wait if trigger is set before giving up
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   long  -
*     DNX_SAND_OK          - trigger is 0
*     DNX_SAND_TRG_TIMEOUT - timeout occurred
*     else error occurred when reading from the chip
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_indirect_verify_trigger_0(
    int  unit,
    uint32 offset,
    uint32 timeout /* in nano seconds */
  )
{
  DNX_SAND_RET
    ex = DNX_SAND_OK ;
  uint32
    err=0,
    access_trigger;


  ex = dnx_sand_indirect_get_access_trigger(unit, offset, &access_trigger);
  if (ex != DNX_SAND_OK)
  {
    goto exit ;
  }
  ex = dnx_sand_trigger_verify_0(
         unit,
         access_trigger,
         timeout,
         DNX_SAND_GENERAL_TRIG_BIT
       ) ;
  if (ex != DNX_SAND_OK)
  {
    goto exit ;
  }
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_INDIRECT_VERIFY_TRIGGER_0,
        "General error in dnx_sand_indirect_verify_trigger_0()", err,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* dnx_sand_indirect_write_address
*DATE:
* 27/OCT/2002
*FUNCTION:
*  writes offset into the address access register
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32  unit,
*      device to write
*    uint32 offset           -
*       the indirect offset to write into the
*       access address register
*    uint32  read_not_write   -
*       0 - if it is a part of a write operation
*       1 - if it is a part of a read operation
*    DNX_SAND_IN   uint32   module_bits -
*      Each indirect module has module bits that specify the
*       indirect module ID.
*       Use module_bits == DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET,
*       to get those bits from the offset (normal operation)
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   long  -
*     Non zero if error
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_indirect_write_address(
    int  unit,
    uint32 offset,
    uint32  read_not_write,
    uint32  module_bits
  )
{
  DNX_SAND_RET
    ex ;
  uint32
      access_address,
      err,
      offset_lcl[1],
      offset_for_module;
  uint32
      *device_base_address ;
  DNX_SAND_INDIRECT_MODULE_BITS
    erase_module_bits;
  err= 0;

  device_base_address = dnx_sand_get_chip_descriptor_base_addr(unit) ;
  ex = DNX_SAND_OK ;

  if(module_bits == DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET)
  {
    offset_for_module = offset;
  }
  else
  {
    offset_for_module = module_bits << DNX_SAND_MODULE_SHIFT;
  }

  ex =
    dnx_sand_indirect_get_module_bits(
      unit,
      offset_for_module,
      &erase_module_bits
    );
  if (ex != DNX_SAND_OK)
  {
    goto exit ;
  }
  ex =
    dnx_sand_indirect_get_access_address(
      unit,
      offset_for_module,
      &access_address
    );
  if (ex != DNX_SAND_OK)
  {
    goto exit ;
  }

  *offset_lcl = offset;

  if(erase_module_bits == DNX_SAND_INDIRECT_ERASE_MODULE_BITS &&
     module_bits == DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET
    )
  {
    /*
     * If needed set ZERO to the module bits.
     */
    *offset_lcl &= (~DNX_SAND_MODULE_MASK);
  }

  if (read_not_write)
  {
    *offset_lcl       |= DNX_SAND_INDRCT_READ_MASK ;
    offset_for_module |= DNX_SAND_INDRCT_READ_MASK ;
  }
  else /* write */
  {
    *offset_lcl       &= DNX_SAND_INDRCT_WRITE_MASK;
    offset_for_module &= DNX_SAND_INDRCT_WRITE_MASK;
  }

  ex = dnx_sand_physical_write_to_chip(
         offset_lcl, device_base_address,
         access_address,
         sizeof(uint32)
       ) ;
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_INDIRECT_WRITE_ADDRESS,
        "General error in dnx_sand_indirect_write_address()", err,0,0,0,0,0) ;
  return ex ;
}

/*****************************************************
*NAME:
* dnx_sand_indirect_write_address_ind_info
*DATE:
* 01/SEP/2007
*FUNCTION:
*  writes offset into the address access register
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32  unit,
*      device to write
*    uint32 offset           -
*       the indirect offset to write into the
*       access address register
*    uint32  read_not_write   -
*       0 - if it is a part of a write operation
*       1 - if it is a part of a read operation
*    DNX_SAND_INDIRECT_MODULE_INFO ind_info -
*       A struct that consists of the information
*    DNX_SAND_IN   uint32   module_bits -
*      Each indirect module has module bits that specify the
*       indirect module ID.
*       Use module_bits == DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET,
*       to get those bits from the offset (normal operation)
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   long  -
*     Non zero if error
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/

DNX_SAND_RET
  dnx_sand_indirect_write_address_ind_info(
    DNX_SAND_IN  int              unit,
    DNX_SAND_IN  uint32             offset,
    DNX_SAND_IN  uint32              read_not_write,
    DNX_SAND_IN  DNX_SAND_INDIRECT_MODULE_INFO *ind_info,
    DNX_SAND_IN  uint32              module_bits,
             uint32             *device_base_address
  )
{
  DNX_SAND_RET
    ex ;
  uint32
    offset_local[1],
    err;

  err = 0;
  ex = DNX_SAND_OK ;

  *offset_local = offset;
  if(
     (ind_info->module_bits == DNX_SAND_INDIRECT_ERASE_MODULE_BITS) &&
     (module_bits == DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET)
    )
  {
    /*
     * If needed set ZERO to the module bits.
     */
    *offset_local &= (~DNX_SAND_MODULE_MASK);
  }

  if (read_not_write)
  {
    *offset_local |= DNX_SAND_INDRCT_READ_MASK ;
  }
  else
  {
    *offset_local &= DNX_SAND_INDRCT_WRITE_MASK;
  }

  ex = dnx_sand_physical_write_to_chip(
         offset_local,
         device_base_address,
         ind_info->access_address,
         sizeof(uint32)
       ) ;
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_INDIRECT_WRITE_ADDRESS,
        "General error in dnx_sand_indirect_write_address()", err,0,0,0,0,0) ;
  return ex ;
}




/*****************************************************
*NAME:
* dnx_sand_indirect_write_value
*DATE:
* 27/OCT/2002
*FUNCTION:
*  writes offset into the writes value access register
*INPUT:true
*  DNX_SAND_DIRECT:
*    uint32 *base_address    -
*       start address of this device
*    uint32   *result_ptr    -
*       supplied user buffer to write to chip
*    uint32   size           -
*       in bytes of the user buffer
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   long  -
*     Non zero if error
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_indirect_write_value(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    uint32   offset,
    DNX_SAND_IN    uint32   *result_ptr,
    DNX_SAND_IN    uint32   size /* in bytes */
  )
{
  DNX_SAND_RET
    ex = DNX_SAND_OK ;
  uint32
    word_size;
  uint32
      err,
      write_buffer_offset,
      *device_base_address ;

  err= 0;

  ex = dnx_sand_indirect_get_write_info(unit, offset,&write_buffer_offset,&word_size);
  if (ex != DNX_SAND_OK)
  {
    goto exit ;
  }

  device_base_address = dnx_sand_get_chip_descriptor_base_addr(unit) ;
  ex = dnx_sand_physical_write_to_chip(
         result_ptr, device_base_address,
         write_buffer_offset,
         size
       ) ;
  if (DNX_SAND_OK != ex)
  {
    goto exit ;
  }
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_INDIRECT_WRITE_VALUE,
        "General error in dnx_sand_indirect_write_value()", err,0,0,0,0,0) ;
  return ex ;
}


/*****************************************************
*NAME:
* dnx_sand_indirect_write_value_ind_info
*DATE:
* 01/SEP/2007
*FUNCTION:
*  writes offset into the writes value access register
*INPUT:true
*  DNX_SAND_DIRECT:
*    uint32 *base_address    -
*       start address of this device
*    uint32   *result_ptr    -
*       supplied user buffer to write to chip
*    DNX_SAND_INDIRECT_MODULE_INFO ind_info           -
*       A struct that consists of the information
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   long  -
*     Non zero if error
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/

DNX_SAND_RET
  dnx_sand_indirect_write_value_ind_info(
    DNX_SAND_IN  int              unit,
    DNX_SAND_IN  uint32             reverse_order,
    DNX_SAND_IN  uint32             word_size,
    DNX_SAND_IN  uint32             *result_ptr,
    DNX_SAND_IN  DNX_SAND_INDIRECT_MODULE_INFO *ind_info,
             uint32             *device_base_address
  )
{
  DNX_SAND_RET
    ex = DNX_SAND_OK ;
  uint32
    err = 0;
#ifndef SAND_LOW_LEVEL_SIMULATION /* { */
  uint32
    ind;

  if (reverse_order)
  {
    /* Both ind and word_size are a multiply of sizeof(uint32) */
    for (ind = 0; ind < word_size; ind += sizeof(uint32))
    {
      ex = dnx_sand_physical_write_to_chip(
             result_ptr + (ind >> 2),
             device_base_address,
             ind_info->write_buffer_offset + ind_info->word_size - (ind + sizeof(uint32)),
             sizeof(uint32)
           ) ;
      if (DNX_SAND_OK != ex)
      {
        err = 1 ;
        goto exit ;
      }
    }
  }
  else
  {
#endif /* SAND_LOW_LEVEL_SIMULATION } */
    ex = dnx_sand_physical_write_to_chip(
           result_ptr,
           device_base_address,
           ind_info->write_buffer_offset,
           word_size
         ) ;
    if (DNX_SAND_OK != ex)
    {
      err = 2 ;
      goto exit ;
    }
#ifndef SAND_LOW_LEVEL_SIMULATION /* { */
  }
#endif /* SAND_LOW_LEVEL_SIMULATION } */

exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_INDIRECT_WRITE_VALUE,
    "General error in dnx_sand_indirect_write_value()", err,0,0,0,0,0) ;
  return ex ;
}


/*****************************************************
*NAME:
* dnx_sand_indirect_assert_trigger_1
*DATE:
* 27/OCT/2002
*FUNCTION:
*  set the indirect trigger to 1
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32 *base_address    -
*       start address of this device
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   long  -
*     Non zero if error
*  DNX_SAND_INDIRECT:
*REMARKS:
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_indirect_assert_trigger_1(
    DNX_SAND_IN    int    unit,
    DNX_SAND_IN    uint32   offset
  )
{
  DNX_SAND_RET
    ex = DNX_SAND_OK ;

  uint32
      access_trigger,
      err;

  err= 0;

  ex = dnx_sand_indirect_get_access_trigger(unit, offset, &access_trigger);
  if (ex != DNX_SAND_OK)
  {
    goto exit ;
  }

  ex = dnx_sand_trigger_assert_1(
         unit,
         access_trigger,
         0,
         DNX_SAND_GENERAL_TRIG_BIT
       ) ;
  if (DNX_SAND_OK != ex)
  {
    goto exit ;
  }
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_INDIRECT_ASSERT_TRIGGER_1,
        "General error in dnx_sand_indirect_assert_trigger_1()", err,0,0,0,0,0) ;
  return ex ;
}
/*****************************************************
*NAME:
* dnx_sand_indirect_read_result
*DATE:
* 27/OCT/2002
*FUNCTION:
*  reads the result of an indirect request  into user supplied buffer
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32 *base_address  -
*       start address of the chip
*    uint32 *result_ptr    -
*       user supplied buffer
*    uint32 offset         -
*       the indirect address to read
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   long  -
*     Non zero if error
*  DNX_SAND_INDIRECT:
*REMARKS:
*   the offset is needed in order to know which
*   read result buffer in the chip holds the result
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_indirect_read_result(
    int  unit,
    uint32 *result_ptr,
    uint32 offset
  )
{
  DNX_SAND_RET
    ex = DNX_SAND_OK ;
  uint32
    word_size;
  uint32
      err,
      read_offset ;
  uint32
      *device_base_address ;
  err = 0 ;

  device_base_address = dnx_sand_get_chip_descriptor_base_addr(unit) ;

  ex = dnx_sand_indirect_get_read_info(unit, offset, &read_offset,&word_size);
  if (ex != DNX_SAND_OK)
  {
    goto exit ;
  }

  ex = dnx_sand_physical_read_from_chip(
         result_ptr,
         device_base_address,
         read_offset,
         word_size
       ) ;
  if (DNX_SAND_OK != ex)
  {
    err = 2 ;
    goto exit ;
  }
  /*
   */
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_INDIRECT_READ_RESULT,
        "General error in dnx_sand_indirect_read_result()",err,0,0,0,0,0) ;
  return ex ;
}

/*****************************************************
*NAME:
* dnx_sand_indirect_read_result_ind_info
*DATE:
* 01/SEP/2007
*FUNCTION:
*  reads the result of an indirect request  into user supplied buffer
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32 *result_ptr    -
*       user supplied buffer
*    uint32 offset         -
*       the indirect address to read
*    DNX_SAND_INDIRECT_MODULE_INFO ind_info -
*        the struct containing of the information:
*        word size and read result offset.
*    uint32 *device_base_address
*       start address of this device
*
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*   long  -
*     Non zero if error
*  DNX_SAND_INDIRECT:
*REMARKS:
*   the offset is needed in order to know which
*   read result buffer in the chip holds the result
*SEE ALSO:
*****************************************************/

DNX_SAND_RET
  dnx_sand_indirect_read_result_ind_info(
    DNX_SAND_OUT uint32             *result_ptr,
    DNX_SAND_IN  uint32             word_size,
    DNX_SAND_IN  uint32             reverse_order,
    DNX_SAND_IN  DNX_SAND_INDIRECT_MODULE_INFO *ind_info,
             uint32             *device_base_address
  )
{
  DNX_SAND_RET
    ex = DNX_SAND_OK ;
  uint32
    err = 0;
#ifndef SAND_LOW_LEVEL_SIMULATION /* { */
  uint32
    ind;

  if (reverse_order)
  {
    for (ind = 0; ind < word_size; ind += sizeof(uint32))
    {
      ex = dnx_sand_physical_read_from_chip(
             result_ptr + (ind >> 2),
             device_base_address,
             ind_info->read_result_offset + ind_info->word_size - (ind + sizeof(uint32)),
             sizeof(uint32)
           ) ;
      if (DNX_SAND_OK != ex)
      {
        err = 1 ;
        goto exit ;
      }
    }
  }
  else
  {
#endif /* SAND_LOW_LEVEL_SIMULATION } */
    ex = dnx_sand_physical_read_from_chip(
           result_ptr,
           device_base_address,
           ind_info->read_result_offset,
           word_size
         ) ;
    if (DNX_SAND_OK != ex)
    {
      err = 2 ;
      goto exit ;
    }
#ifndef SAND_LOW_LEVEL_SIMULATION /* { */
  }
#endif /* SAND_LOW_LEVEL_SIMULATION } */

exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_INDIRECT_READ_RESULT,
        "General error in dnx_sand_indirect_read_result()",err,0,0,0,0,0) ;
  return ex ;
}


/*****************************************************
*NAME
* dnx_sand_indirect_read_ind_info_low
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
*    DNX_SAND_IN   uint32   module_bits -
*      Each indirect module has module bits that specify the
*       indirect module ID.
*       Use module_bits == DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET,
*       to get those bits from the offset (normal operation)
*    DNX_SAND_IN     uint32  offset -
*      Offset of first 32-bits register to read, relative to
*      'indirect' offset.
*    DNX_SAND_INDIRECT_MODULE_INFO ind_info -
*       A struct that consists of the information
*    DNX_SAND_IN     uint32  size -
*      Number of bytes to read, beginning at 'offset'.
*      This must be a multiple of four (integral number
*      of uint32s).
*    DNX_SAND_INOUT  uint32* result_ptr -
*      Pointer to buffer for this procedure to load
*      with read data. Size of buffer must be at
*      least 'size'.
*    DNX_SAND_IN     uint32  size -
*      size of the data to read.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*   OK or ERROR indication.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*
*SEE ALSO:
*****************************************************/


static DNX_SAND_RET
  dnx_sand_indirect_read_ind_info_low(
    DNX_SAND_IN     int                 unit,
    DNX_SAND_IN     uint32                 module_bits,
    DNX_SAND_IN     uint32                  offset,
    DNX_SAND_IN     DNX_SAND_INDIRECT_MODULE_INFO *ind_info,
    DNX_SAND_INOUT  uint32                  *result_ptr,
    DNX_SAND_IN     uint32                  word_size,
    DNX_SAND_IN     uint32                  reverse_order,
    DNX_SAND_IN     uint32                  size
  )
{
  DNX_SAND_RET
    res = DNX_SAND_OK;
  uint32
    *device_base_address,
    nof_accesses,
    accesses_i,
    read_result_long_size;

  device_base_address = dnx_sand_get_chip_descriptor_base_addr(unit);

  read_result_long_size = word_size >> 2;
  nof_accesses = size / word_size;

  /*
   * Verify indirect trigger is ZERO.
   */
  res = dnx_sand_trigger_verify_0_by_base(
          ind_info->access_trigger,
          DNX_SAND_TRIGGER_TIMEOUT,
          DNX_SAND_GENERAL_TRIG_BIT,
          device_base_address
        );
  if (res != DNX_SAND_OK)
  {
    goto exit ;
  }

  for(accesses_i = 0; accesses_i < nof_accesses; ++accesses_i)
  {
    /*
     * Write indirect address.
     */
    res = dnx_sand_indirect_write_address_ind_info(
            unit,
            offset + accesses_i,
            DNX_SAND_TBL_READ_BIT, /* Read */
            ind_info,
            module_bits,
            device_base_address
          );
    if (res != DNX_SAND_OK)
    {
      goto exit ;
    }

    /*
     * Set indirect trigger to ONE and wait for it to get ZERO again.
     */
    res = dnx_sand_trigger_assert_1_by_base(
            ind_info->access_trigger,
            DNX_SAND_TRIGGER_TIMEOUT,
            DNX_SAND_GENERAL_TRIG_BIT,
            device_base_address
          );
    if (res != DNX_SAND_OK)
    {
      goto exit ;
    }

    /*
     * Read the indirect result.
     */
    res = dnx_sand_indirect_read_result_ind_info(
            result_ptr,
            word_size,
            reverse_order,
            ind_info,
            device_base_address
          );
    if (res != DNX_SAND_OK)
    {
      goto exit ;
    }

    result_ptr += read_result_long_size;
  }

exit:
  DNX_SAND_ERROR_REPORT(res,NULL,0,0,DNX_SAND_INDIRECT_READ_IND_INFO_LOW,
    "General error in dnx_sand_indirect_read_ind_info_low()",0,0,0,0,0,0) ;
  return res;
}


/*****************************************************
*NAME
* dnx_sand_indirect_write_ind_info_low
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
*    DNX_SAND_IN   uint32   module_bits -
*      Each indirect module has module bits that specify the
*       indirect module ID.
*       Use module_bits == DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET,
*       to get those bits from the offset (normal operation)
*    DNX_SAND_IN     uint32  offset -
*      Offset of first 32-bits register to read, relative to
*      'indirect' offset.
*    DNX_SAND_INDIRECT_MODULE_INFO ind_info -
*       A struct that consists of the information
*    DNX_SAND_IN     uint32  size -
*      Number of bytes to read, beginning at 'offset'.
*      This must be a multiple of four (integral number
*      of uint32s).
*    DNX_SAND_INOUT  uint32* data_ptr -
*      Pointer to buffer for this procedure to load
*      with read data. Size of buffer must be at
*      least 'size'.
*    DNX_SAND_IN     uint32  size -
*      size of the data to read.
*    DNX_SAND_IN     uint32  word_size -
*      Number of indirect write registers of the module.
*    DNX_SAND_IN     uint32       reverse_order -
*      Number of registers to revert (LSB in highest register). Zero if none.
*    DNX_SAND_IN     uint32  repeat -
*      Number of times to repeat a single write command.
*      The write is performed at least once (even if repeat = zero).
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*   OK or ERROR indication.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*      The write is performed at least once (even if repeat = zero).
*SEE ALSO:
*****************************************************/
static DNX_SAND_RET
  dnx_sand_indirect_write_ind_info_low(
    DNX_SAND_IN     int                     unit,
    DNX_SAND_IN     uint32                     module_bits,
    DNX_SAND_IN     uint32                      offset,
    DNX_SAND_IN     DNX_SAND_INDIRECT_MODULE_INFO     *ind_info,
    DNX_SAND_IN     uint32                      *data_ptr,
    DNX_SAND_IN     uint32                      size,
    DNX_SAND_IN     uint32                      word_size,
    DNX_SAND_IN     uint32                      reverse_order,
    DNX_SAND_IN     uint32                      repeat
  )
{
  DNX_SAND_RET
    res = DNX_SAND_OK ;
  uint32
    nof_accesses = 0,
    repeat_i = 0,
    repeat_offset = 0,
    accesses_i = 0,
    *device_base_address = dnx_sand_get_chip_descriptor_base_addr(unit),
    read_result_long_size = 0;

  if ((word_size % sizeof(uint32)) != 0)
  {
    res = DNX_SAND_ERR;
    goto exit;
  }

  read_result_long_size = word_size >> 2;
  nof_accesses = size / word_size;

  /* Verify indirect trigger is ZERO. */
  res = dnx_sand_trigger_verify_0_by_base(
          ind_info->access_trigger,
          DNX_SAND_TRIGGER_TIMEOUT,
          DNX_SAND_GENERAL_TRIG_BIT,
          device_base_address
        );
  if (res != DNX_SAND_OK)
  {
    goto exit ;
  }

  for(accesses_i = 0; accesses_i < nof_accesses; ++accesses_i)
  {
    /* Write the value. */
    res = dnx_sand_indirect_write_value_ind_info(
            unit,
            reverse_order,
            word_size,
            data_ptr,
            ind_info,
            device_base_address
          );
    if (res != DNX_SAND_OK)
    {
      goto exit ;
    }

    repeat_offset = 0;
    repeat_i      = 0;

    do
    {
      /* Write indirect address. */
       res = dnx_sand_indirect_write_address_ind_info(
               unit,
               offset + accesses_i + repeat_offset,
               DNX_SAND_TBL_WRITE_BIT, /* Write operation */
               ind_info,
               module_bits,
               device_base_address
             );
       if (res != DNX_SAND_OK)
       {
         goto exit ;
       }

      /* Set indirect trigger to ONE and wait it go to ZERO again. */
      res = dnx_sand_trigger_assert_1_by_base(
              ind_info->access_trigger,
              DNX_SAND_TRIGGER_TIMEOUT,
              DNX_SAND_GENERAL_TRIG_BIT,
              device_base_address
            );
      if (res != DNX_SAND_OK)
      {
        goto exit ;
      }

      repeat_offset += nof_accesses;

    } while((++repeat_i) < repeat);

    data_ptr += read_result_long_size;
  }

exit:
  DNX_SAND_ERROR_REPORT(res,NULL,0,0,DNX_SAND_INDIRECT_WRITE_IND_INFO_LOW,
    "General error in dnx_sand_indirect_write_ind_info_low()",0,0,0,0,0,0) ;
  return res;
}

/*****************************************************
*NAME
* dnx_sand_indirect_read_from_chip
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
*    uint32 -
*      See formatting rules in ERROR RETURN VALUES above.
*      If error code is not FAP10M_NO_ERR then
*        specific error codes:
*          None.
*      Otherwise, no error has been detected and device
*        has been written.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  Calls to a callback function
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_indirect_read_from_chip(
    DNX_SAND_IN     int   unit,
    DNX_SAND_INOUT  uint32* result_ptr,
    DNX_SAND_IN     uint32  offset,
    DNX_SAND_IN     uint32  size,
    DNX_SAND_IN     uint32   module_bits
  )
{
  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK ;

#if DNX_SAND_PHYSICAL_PRINT_WHEN_WRITING
  dnx_sand_set_print_when_writing_part_of_indirect_read(TRUE);
#endif
  /*
   */
  if (Dnx_soc_sand_indirect_access.indirect_read != NULL)
  {
    dnx_sand_ret =
      Dnx_soc_sand_indirect_access.indirect_read(
        unit,
        result_ptr,
        offset,
        size,
        module_bits
      );
  }
  else
  {
    dnx_sand_ret = DNX_SAND_NULL_USER_CALLBACK_FUNC ;
    goto exit ;
  }

exit:
#if DNX_SAND_PHYSICAL_PRINT_WHEN_WRITING
  dnx_sand_set_print_when_writing_part_of_indirect_read(FALSE);
#endif
  DNX_SAND_ERROR_REPORT(dnx_sand_ret,NULL,0,0,DNX_SAND_INDIRECT_READ_FROM_CHIP,
        "General error in dnx_sand_indirect_read_from_chip()",
        unit,
        offset,
        size,
        0 ,0, 0) ;
  return dnx_sand_ret ;
}


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
*    DNX_SAND_IN uint32  ind_options -
*      Width of table to read from
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    error indication
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*    None.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_tbl_read(
    DNX_SAND_IN  int     unit,
    DNX_SAND_OUT uint32      *result_ptr,
    DNX_SAND_IN  uint32      offset,
    DNX_SAND_IN  uint32     size,
    DNX_SAND_IN  uint32     module_id,
    DNX_SAND_IN  uint32     ind_options
  )
{
  uint32
    res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_TBL_READ);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(result_ptr);
  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  /*
   * semaphore taken - go on with the actual job of reading
   */

  res = dnx_sand_tbl_read_unsafe(
          unit,
          result_ptr,
          offset,
          size,
          module_id,
          ind_options
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit_semaphore);


exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_tbl_read()",0,0);
}

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
*    DNX_SAND_IN uint32  ind_options -
*      Width of table to read from
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    error indication
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*    None.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_tbl_read_unsafe(
    DNX_SAND_IN  int     unit,
    DNX_SAND_OUT uint32      *result_ptr,
    DNX_SAND_IN  uint32      offset,
    DNX_SAND_IN  uint32     size,
    DNX_SAND_IN  uint32     module_id,
    DNX_SAND_IN  uint32     ind_options
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_TBL_READ_UNSAFE);

  /*
   */
  if (Dnx_soc_sand_tbl_access.read == NULL)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_NULL_USER_CALLBACK_FUNC,10,exit);
  }

  res = Dnx_soc_sand_tbl_access.read(
          unit,
          result_ptr,
          offset,
          size,
          module_id,
          ind_options
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_tbl_read_unsafe()",module_id,offset);
}


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
*      Width of table to write to

*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    error indication
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*    None.
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
  )
{

  uint32
    res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_TBL_WRITE);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  DNX_SAND_CHECK_NULL_INPUT(data_ptr);
  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  /*
   * semaphore taken - go on with the actual job of reading
   */

   res = dnx_sand_tbl_write_unsafe(
           unit,
           data_ptr,
           offset,
           size,
           module_id,
           entry_nof_bytes
         );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit_semaphore);


exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_tbl_write()",0,0);
}



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
*      bits 0 :11 - width of the table in bytes.
*      bits 12:29 - number of desired repetitions.
*      bits 30:31 - number of indirect write registers of the module for which the
*                   LSB should be written to the highest write register
*                   Zero for none.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    error indication
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*    None.
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
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_TBL_WRITE_UNSAFE);

  if (Dnx_soc_sand_tbl_access.write == NULL)
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_NULL_USER_CALLBACK_FUNC,10,exit);
  }

  res = Dnx_soc_sand_tbl_access.write(
          unit,
          data_ptr,
          offset,
          size,
          module_id,
          indirect_options|DNX_SAND_SET_TBL_WRITE_REPT_BITS_IN_WORD(Dnx_soc_sand_nof_repetitions[unit])
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in dnx_sand_tbl_write_unsafe()",0,0);

}

/*****************************************************
*NAME
* DNX_SAND_INDIRECT_READ_LOW
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
  )
{
  DNX_SAND_RET
    res = DNX_SAND_OK;

  uint32
    offset_for_module;

  DNX_SAND_INDIRECT_MODULE_INFO
    ind_info;


  if(module_bits == DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET)
  {
    offset_for_module = offset;
  }
  else
  {
    offset_for_module = module_bits << DNX_SAND_MODULE_SHIFT;
  }

  /*
   * Check for legal indirect block.
   */
  res = dnx_sand_indirect_check_offset_is_legal(
          unit,
          offset_for_module,
          size >> 2 /*method expects size in longs*/
        );

  if (res != DNX_SAND_OK)
  {
    goto exit ;
  }

  res = dnx_sand_indirect_get_inner_struct(
          unit,
          offset,
          module_bits,
          &ind_info
        );
  if (res != DNX_SAND_OK)
  {
    goto exit ;
  }

  res = dnx_sand_indirect_read_ind_info_low(
          unit,
          module_bits,
          offset,
          &ind_info,
          result_ptr,
          ind_info.word_size,
          DNX_SAND_TBL_READ_NO_RVRS,
          size
        );
  if (res != DNX_SAND_OK)
  {
    goto exit ;
  }

exit:
  DNX_SAND_ERROR_REPORT(res,NULL,0,0,DNX_SAND_INDIRECT_READ_LOW,
      "General error in dnx_sand_indirect_read_low()",
      0,0,0,0 ,0, 0) ;

  return res;
}



/*****************************************************
*NAME
* dnx_sand_tbl_read_low
*DATE:
*  20-SEP-07
*FUNCTION:
*  low level indirect read function.
*  assumes that the parameters are checked and device
*  semaphore is taken.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN   int   unit -
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
*    DNX_SAND_IN   uint32   module_id -
*      Each indirect module has module id
*    DNX_SAND_IN   uint32   word_size
*      width of the table in longs
*
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    unsigned short -
*      error indication
*  DNX_SAND_INDIRECT:
*    NONE
*REMARKS:
*  - still to do check if input (offset,size, module id) is legal
*  - size and word_size are given in number of longs
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
  )
{
  DNX_SAND_INDIRECT_MODULE_INFO
    ind_info;
  DNX_SAND_RET
    res;

  
  res = dnx_sand_indirect_get_inner_struct(
          unit,
          offset,
          module_id,
          &ind_info
        );
  if (res != DNX_SAND_OK)
  {
    goto exit ;
  }
#if DNX_SAND_PHYSICAL_PRINT_WHEN_WRITING
  dnx_sand_set_print_when_writing_part_of_indirect_read(TRUE);
#endif

  res = dnx_sand_indirect_read_ind_info_low(
          unit,
          module_id,
          offset,
          &ind_info,
          result_ptr,
          DNX_SAND_GET_TBL_READ_SIZE_BITS_FROM_WORD(indirect_options),
          DNX_SAND_GET_TBL_READ_RVRS_BITS_FROM_WORD(indirect_options),
          size
        );
  if (res != DNX_SAND_OK)
  {
    goto exit ;
  }


exit:
#if DNX_SAND_PHYSICAL_PRINT_WHEN_WRITING
  dnx_sand_set_print_when_writing_part_of_indirect_read(FALSE);
#endif

  DNX_SAND_ERROR_REPORT(res,NULL,0,0,DNX_SAND_TBL_READ_LOW,
    "error in dnx_sand_tbl_read_low()",0,0,0,0,0,0) ;
  return res ;
}

/*****************************************************
*NAME
* dnx_sand_indirect_write_to_chip
*TYPE:
*  PROC
*DATE:
*  03-Jun-03
*FUNCTION:
*  DNX_SAND low level indirect write function.
*  To be called from 'dnx_sand_mem_write()' or any other
*  driver calls after parameters are checked and device
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
*    NON
*REMARKS:
*  Calls to a callback function
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_indirect_write_to_chip(
    DNX_SAND_IN   int   unit,
    DNX_SAND_IN   uint32* data_ptr,
    DNX_SAND_IN   uint32  offset,
    DNX_SAND_IN   uint32  size,
    DNX_SAND_IN   uint32   module_bits
  )
{
  DNX_SAND_RET
    dnx_sand_ret = DNX_SAND_OK ;

#if DNX_SAND_PHYSICAL_PRINT_WHEN_WRITING /* { */
  dnx_sand_indirect_write_to_chip_print_when_write(
    unit,data_ptr,offset,size,4
    );
#endif /* } DNX_SAND_PHYSICAL_PRINT_WHEN_WRITING*/

 if (Dnx_soc_sand_indirect_access.indirect_write != NULL)
  {
    dnx_sand_ret =
      Dnx_soc_sand_indirect_access.indirect_write(
        unit,
        data_ptr,
        offset,
        size,
        module_bits
      ) ;
  }
  else
  {
    dnx_sand_ret = DNX_SAND_NULL_USER_CALLBACK_FUNC ;
    goto exit ;
  }

exit:
#if DNX_SAND_PHYSICAL_PRINT_WHEN_WRITING
  dnx_sand_set_print_when_writing_part_of_indirect_write(FALSE);
#endif
  DNX_SAND_ERROR_REPORT(dnx_sand_ret,NULL,0,0,DNX_SAND_INDIRECT_WRITE_TO_CHIP,
        "General error in dnx_sand_indirect_write_to_chip()",
        unit,
        offset,
        size,
        0 ,0, 0) ;


  return dnx_sand_ret ;
}


/*****************************************************
*NAME
* DNX_SAND_INDIRECT_WRITE_LOW
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
DNX_SAND_RET
  dnx_sand_indirect_write_low(
    DNX_SAND_IN     int   unit,
    DNX_SAND_IN     uint32* data_ptr,
    DNX_SAND_IN     uint32  offset,
    DNX_SAND_IN     uint32  size,
    DNX_SAND_IN     uint32   module_bits
  )
{
  DNX_SAND_INDIRECT_MODULE_INFO
    ind_info;
  uint32
    offset_for_module;
  DNX_SAND_RET
    res = DNX_SAND_OK;

  if (data_ptr == NULL)
  {
    goto exit;
  }
  if(module_bits == DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET)
  {
    offset_for_module = offset;
  }
  else
  {
    offset_for_module = module_bits << DNX_SAND_MODULE_SHIFT;
  }

  /*
   * Check for legal indirect block.
   */
  res = dnx_sand_indirect_check_offset_is_legal(
          unit,
          offset_for_module,
          size>>2
        );
  if (res != DNX_SAND_OK)
  {
    goto exit ;
  }

   res = dnx_sand_indirect_get_inner_struct(
           unit,
           offset,
           module_bits,
           &ind_info
         );
   if (res!=DNX_SAND_OK)
   {
     goto exit;
   }

   res = dnx_sand_indirect_write_ind_info_low(
           unit,
           module_bits,
           offset,
           &ind_info,
           data_ptr,
           size,
           ind_info.word_size,
           DNX_SAND_TBL_WRITE_NO_RVRS, /* No reverse order writing */
           dnx_sand_indirect_get_nof_repetitions(unit)
         );

   if (res != DNX_SAND_OK)
   {
     goto exit ;
   }

exit:

  DNX_SAND_ERROR_REPORT(res,NULL,0,0,DNX_SAND_INDIRECT_WRITE_LOW,
    "General error in dnx_sand_indirect_write_low()",0,0,0,0,0,0) ;

  return res;
}


/*****************************************************
*NAME
* dnx_sand_tbl_write_low
*DATE:
*  20-SEP-07
*FUNCTION:
*  low level indirect read function.
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
*    DNX_SAND_IN   uint32   module_id -
*      Each indirect module has module id
*    SAMD_IN   uint32   indirect_options
*      bits 0 :11 - width of the table in longs.
*      bits 12:27 - number of desired repetitions.
*      bits 28:31 - number of indirect write registers of the odule suppose
*                   LSB should be written to the highest write register,
*                   otherwise 0.
*
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    unsigned short -
*      error indication
*  DNX_SAND_INDIRECT:
*    NONE
*REMARKS:
*  - still to do check if input (offset,size, module id) is legal
*  - size and word_size are given in number of longs
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_tbl_write_low(
    DNX_SAND_IN  int   unit,
    DNX_SAND_IN  uint32    *data_ptr,
    DNX_SAND_IN  uint32    offset,
    DNX_SAND_IN  uint32    size,
    DNX_SAND_IN  uint32   module_id,
    DNX_SAND_IN  uint32    indirect_options
  )
{
  DNX_SAND_INDIRECT_MODULE_INFO
    ind_info;
  DNX_SAND_RET
    res;

  
  res = dnx_sand_indirect_get_inner_struct(
          unit,
          offset,
          module_id,
          &ind_info
        );
  if (res != DNX_SAND_OK)
  {
    goto exit ;
  }

#if DNX_SAND_PHYSICAL_PRINT_WHEN_WRITING /* { */
  dnx_sand_indirect_write_to_chip_print_when_write(
    unit,data_ptr,offset,size,size
    );
#endif /* } DNX_SAND_PHYSICAL_PRINT_WHEN_WRITING*/

  res = dnx_sand_indirect_write_ind_info_low(
          unit,
          module_id,
          offset,
          &ind_info,
          data_ptr,
          size,
          DNX_SAND_GET_TBL_WRITE_SIZE_BITS_FROM_WORD(indirect_options),
          DNX_SAND_GET_TBL_WRITE_RVRS_BITS_FROM_WORD(indirect_options),
          DNX_SAND_GET_TBL_WRITE_REPT_BITS_FROM_WORD(indirect_options)
        );
  if (res != DNX_SAND_OK)
  {
    goto exit ;
  }


exit:
#if DNX_SAND_PHYSICAL_PRINT_WHEN_WRITING
  dnx_sand_set_print_when_writing_part_of_indirect_write(FALSE);
#endif

  DNX_SAND_ERROR_REPORT(res,NULL,0,0,DNX_SAND_TBL_WRITE_LOW,
    "error in dnx_sand_tbl_write_low()",0,0,0,0,0,0) ;
  return res ;
}

/*****************************************************
*NAME:
* dnx_sand_indirect_read_modify_write
*DATE:
* 31/MAY/2005
*FUNCTION:
* reads modify writes information in the indirect memory
* space
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN    int   unit -
*      Identifier of device to access.
*   DNX_SAND_IN     uint32 offset          -
*     offset of the field
*   DNX_SAND_IN     uint32 shift           -
*     shift of the field within 32 bit register
*   DNX_SAND_IN     uint32 mask            -
*     mask of the field within 32 bit register
*   DNX_SAND_IN     uint32 data_to_write      -
*     the new data to write
*  DNX_SAND_INDIRECT:
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-zero in case of an error
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_indirect_read_modify_write(
    DNX_SAND_IN     int  unit,
    DNX_SAND_IN     uint32 offset,
    DNX_SAND_IN     uint32 shift,
    DNX_SAND_IN     uint32 mask,
    DNX_SAND_IN     uint32 data_to_write
  )
{
  DNX_SAND_RET ex ;
  uint32 tmp[1];

  /* Read */
  ex =
    dnx_sand_indirect_read_from_chip(
      unit,
      tmp,
      offset,
      sizeof(uint32),
      DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET
      ) ;
  if (DNX_SAND_OK != ex)
  {
    goto exit ;
  }
  /* Modify */
  *tmp &= ~mask ;
  *tmp |= DNX_SAND_SET_FLD_IN_PLACE(data_to_write, shift, mask) ;

  /* Write */
  ex =
    dnx_sand_indirect_write_to_chip(
      unit,
      tmp,
      offset,
      sizeof(uint32),
      DNX_SAND_IND_GET_MODULE_BIT_FROM_OFFSET
      ) ;
  if (DNX_SAND_OK != ex)
  {
    goto exit ;
  }
  /*
   */
exit:
  DNX_SAND_ERROR_REPORT(ex,NULL,0,0,DNX_SAND_READ_MODIFY_WRITE,
        "error in dnx_sand_indirect_read_modify_write(): Cannot access chip",0,0,0,0,0,0) ;
  return ex ;
}

#if DNX_SAND_DEBUG

void
  dnx_sand_indirect_write_to_chip_print_when_write(
    DNX_SAND_IN   int   unit,
    DNX_SAND_IN   uint32* data_ptr,
    DNX_SAND_IN   uint32  offset,
    DNX_SAND_IN   uint32  size,
    DNX_SAND_IN   uint32  word_size
  )
{
  uint32
    physical_print_when_writing,
    asic_style,
    indirect_write,
    unit_or_base_address;

  dnx_sand_get_print_when_writing(
    &physical_print_when_writing,
    &asic_style,
    &indirect_write
  );

  if(physical_print_when_writing && indirect_write)
  {
    uint32
      print_num = DNX_SAND_DIV_ROUND_UP(size,4),
      print_i=0;
    uint32
      print_info;

    dnx_sand_set_print_when_writing_part_of_indirect_write(TRUE);

    dnx_sand_get_print_when_writing_unit_or_base_address(
      &unit_or_base_address
      );

    while(print_i<print_num)
    {
      print_info = *(data_ptr+print_i);
      if(asic_style)
      {
        LOG_CLI((BSL_META_U(unit,
                            "INDIRECT_WRITE  0x%X 0x%X\n\r"),
                 offset + (print_i/word_size),
                 print_info
                 /**(data_ptr+print_i)*/
                 ));
      }
      else if (unit_or_base_address)
      {
        if(print_i == 0)
        {
          LOG_CLI((BSL_META_U(unit,
                              "%d 0x%08X "),
                   unit,
                   offset + (print_i/word_size)
                   ));
        }
        if(DNX_SAND_DIV_ROUND_UP(word_size, 4) == print_i+1)
        {
          switch(word_size% 4)
          {
          case 1:
            LOG_CLI((BSL_META_U(unit,
                                "0x%02X\n\r"),
                     *(data_ptr+print_i)
                     ));
            break;
          case 2:
            LOG_CLI((BSL_META_U(unit,
                                "0x%04X\n\r"),
                     *(data_ptr+print_i)
                     ));
            break;
          case 3:
            LOG_CLI((BSL_META_U(unit,
                                "0x%06X\n\r"),
                     *(data_ptr+print_i)
                     ));
            break;
          default:
            LOG_CLI((BSL_META_U(unit,
                                "0x%08X \n\r"),
                     *(data_ptr+print_i)
                     ));
          }
        }
        else
        {
          LOG_CLI((BSL_META_U(unit,
                              "0x%08X "),
                   *(data_ptr+print_i)
                   ));
        }
      }
      else
      {
        LOG_CLI((BSL_META_U(unit,
                            "0x%08x 0x%08X\n\r"),
                 offset + (print_i/word_size),
                 *(data_ptr+print_i)
                 ));
      }
      print_i++;
    }
  }
  return;
}
#endif
/*
 * }
 */


