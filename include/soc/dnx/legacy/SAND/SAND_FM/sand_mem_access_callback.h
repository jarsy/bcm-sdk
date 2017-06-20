/* $Id: sand_mem_access_callback.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __DNX_SAND_MEM_ACCESS_CALLBACK_H_INCLUDED__
/* { */
#define __DNX_SAND_MEM_ACCESS_CALLBACK_H_INCLUDED__

#ifdef  __cplusplus
extern "C" {
#endif

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>

#include <soc/dnx/legacy/SAND/SAND_FM/sand_user_callback.h>

/* $Id: sand_mem_access_callback.h,v 1.3 Broadcom SDK $
 * DNX_SAND_MEM_READ_CALLBACK_STRUCT is a structure used by
 * the API method dnx_sand_mem_read_callback();
 * This structure is passed from the API method to the
 * callback method holding the specific read request information
 */
typedef struct
{
  /*
   * the unit to read from
   */
  int      unit;
  /*
   * the address of the user supplied buffer, to write the
   * information read from the chip into.
   */
  uint32     *result_ptr;
  /*
   * This buffer is for the internal use of the callback mechanism
   * we copy the last result of the callback, and call the user
   * supplied callback, only if there is a change in that result
   * The reason it's a (huge) array and a specially allocated buffer
   * for that specific buffer (result_ptr) size, is that we wouldn't
   * be able to free it, when the callback item gets kicked out of
   * the system.
   */
  uint32     copy_of_last_result_ptr[DNX_SAND_CALLBACK_BUF_SIZE>>2];
  /*
   * only after copying a value into this buffer it's contect becomes valid
   */
  uint32      copy_of_result_is_valid;
  /*
   * address, within the chip, to read from
   */
  uint32     offset;
  /*
   * the size (in bytes) of the block of information to read from
   * the chip (the read block will be [offset, offset+size]
   */
  uint32      size;
  /*
   * whether this is a direct or indirect access to the chip.
   * offset should be interpreted as a direct or indirext address
   */
  uint32      indirect;
  /*
   * the transaction_id of the requested action, necessary for
   * killing the job later
   */
  uint32     dnx_sand_tcm_callback_id;
  /*
   * user supplied parameter, that is passed back, to help the
   * user to distinguish between the different callbacks.
   */
  uint32     user_callback_id;
  /*
   * user supplied parameter. The user callback method to call
   * if the data read from the chip changed.
   */
  DNX_SAND_USER_CALLBACK_FUNC  user_callback_proc;
} DNX_SAND_MEM_READ_CALLBACK_STRUCT;


/*****************************************************
*NAME
*  dnx_sand_mem_read_callback
*TYPE:
*  PROC
*DATE:
*  23/OCT/2002
*FUNCTION:
*  This procedure physically reads internal chip registers
*  as per specified offset.
*  This procedure may, depending on input, result in
*  activation of specified callback when target registers
*  contents has changed.
*CALLING SEQUENCE:
*  dnx_sand_mem_read_callback(buffer, size)
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32      *buffer -
*      This is actually a DNX_SAND_MEM_READ_CALLBACK_STRUCT
*      pointer. see above for the definition of this structure.
*    uint32      size    -
*       sizeof(DNX_SAND_MEM_READ_CALLBACK_STRUCT)
*  DNX_SAND_INDIRECT:
*    FE200_TRIGGER_TIMEOUT .
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 -
*      See formatting rules in ERROR RETURN VALUES above.
*      If error code is not FE200_NO_ERR some error occured
*  DNX_SAND_INDIRECT:
*    the chip is read, information is loaded to the user
*    supplied buffer, and the user supplied function is called.
*REMARKS:
*  static (local, private) method, unavailable to other modules.
*  This is a private callback used by the API service
*  fe200_mem_read()
*  if the API call requested for a polling job, this is the
*  callback being registered for that job. It does the actual
*  reading from the chip, and calls the user supplied method
*  if the read buffer contain different content, then what
*  was already at the user supplied buffer.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_mem_read_callback(
    uint32 *buffer,
    uint32 size
  );


#ifdef  __cplusplus
}
#endif

/* } __DNX_SAND_MEM_ACCESS_CALLBACK_H_INCLUDED__*/
#endif
