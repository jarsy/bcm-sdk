/* $Id: arad_dram.h,v 1.24 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_DRAM_INCLUDED__
/* { */
#define __ARAD_DRAM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/arad_api_dram.h>
#include <soc/dpp/ARAD/arad_api_mgmt.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */
#define SOC_ARAD_DRAM_USER_BUFFER_FLAGS_LOGICAL2PHY_TRANS 0x1

/* the buffer is composed of soc_reg_above_64_val_t, in Arad buff should be able to contain 512b, in Jericho 2048b */
#define SOC_DPP_DRAM_MMU_IND_ACCESS_MAX_BUFF_SIZE 4

/* } */

/*************
 * MACROS    *
 *************/
/* { */

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

typedef struct
{
  /*
   * The list of deleted buffers.
   */
  uint32  dram_deleted_buff_list[ARAD_DRAM_MAX_BUFFERS_IN_ERROR_CNTR];

} ARAD_DRAM;

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */

/*********************************************************************
*  Drc Init
*********************************************************************/
uint32 arad_mgmt_dram_init_drc_soft_init(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint8 *is_valid,
    SOC_SAND_IN uint32  init);

uint32 arad_dram_read_data_dly_get(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint32  dram_freq);

/*********************************************************************
* NAME:
*   arad_dram_info_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   This function configures the dram according to the
*   provided sets of parameters. This function is called
*   during the initialization sequence and must not be
*   called afterwards
* INPUT:
*   SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  uint32                 dram_ndx -
*     Dram index. Range: 0-5
*   SOC_SAND_IN  uint32                 dram_freq -
*     Dram frequency (MHz)
*   SOC_SAND_IN  ARAD_DRAM_TYPE           dram_type -
*     One of the supported dram types (DDR3)
*   SOC_SAND_IN  ARAD_DRAM_INFO           *info -
*     Dram configuration information.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_dram_info_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 dram_ndx,
    SOC_SAND_IN  uint32                 dram_freq,
    SOC_SAND_IN  ARAD_DRAM_TYPE           dram_type,
    SOC_SAND_IN  ARAD_DRAM_INFO           *info
  );

/*********************************************************************
* NAME:
*   arad_dram_info_verify
* TYPE:
*   PROC
* FUNCTION:
*   This function configures the dram according to the
*   provided sets of parameters. This function is called
*   during the initialization sequence and must not be
*   called afterwards
* INPUT:
*   SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  uint32                 dram_freq -
*     Dram frequency (MHz)
*   SOC_SAND_IN  ARAD_DRAM_TYPE           dram_type -
*     One of the supported dram type (DDR3)
*   SOC_SAND_IN  ARAD_DRAM_INFO           *info -
*     Dram configuration information.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_dram_info_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 dram_freq,
    SOC_SAND_IN  ARAD_DRAM_TYPE           dram_type,
    SOC_SAND_IN  ARAD_DRAM_INFO           *info
  );

/*********************************************************************
* NAME:
*   arad_dram_info_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   This function configures the dram according to the
*   provided sets of parameters. This function is called
*   during the initialization sequence and must not be
*   called afterwards
* INPUT:
*   SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  uint32                 dram_ndx -
*     Dram index. Range: 0-5
*   SOC_SAND_IN  uint32                 dram_freq -
*     Dram frequency (MHz)
*   SOC_SAND_OUT ARAD_DRAM_TYPE           *dram_type -
*     One of the supported dram types (DDR3)
*   SOC_SAND_OUT ARAD_DRAM_INFO           *info -
*     Dram configuration information.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_dram_info_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 dram_ndx,
    SOC_SAND_IN  uint32                 dram_freq,
    SOC_SAND_OUT ARAD_DRAM_TYPE           *dram_type,
    SOC_SAND_OUT ARAD_DRAM_INFO           *info
  );

/*********************************************************************
* NAME:
 *   arad_dram_init_drc_phy_register_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   configures the drc registers concerning Dram PHY
 * INPUT:
 *   uint32                                    unit -
 *     Identifier of the device to access.
 *   ARAD_DRAM_TYPE                  dram_type -
 *     One of the supported dram types (DDR3)
 *   uint32                                    dram_freq -
 *     Dram frequency (MHz) 
 *   ARAD_DRAM_INFO                  t_info -
 *     Dram configuration information.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32 arad_dram_init_drc_phy_register_set(int unit, ARAD_DRAM_TYPE dram_type, uint32 dram_freq, ARAD_DRAM_INFO t_info);

/*********************************************************************
* NAME:
 *   arad_dram_rbus_write
 * TYPE:
 *   PROC
 * FUNCTION:
 *   perform indirect write to PHY rbus protocol (inner registers)
 * INPUT:
 *   SOC_SAND_IN  int                                    unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                    addr -
 *     The address to write.
 *   SOC_SAND_IN  uint32                                    dram_ndx -
 *     Dram index. Range: 0 - 7.
 *   SOC_SAND_IN  uint32                                    data -
 *     The data to be writen.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/

uint32 arad_dram_rbus_write(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint32 dram_ndx,
    SOC_SAND_IN uint32 addr,
    SOC_SAND_IN uint32 data
  );

/*********************************************************************
* NAME:
 *   arad_dram_rbus_write_br
 * TYPE:
 *   PROC
 * FUNCTION:
 *   perform indirect write to PHY rbus protocol (inner registers)
 *   broadcasting to all the drams
 * INPUT:
 *   SOC_SAND_IN  int                                    unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                    addr -
 *     The address to write.
 *   SOC_SAND_IN  uint32                                    last_dram_ndx -
 *     The last dram index. Range: 0 - 7.
 *     In order to get the ack from this dram drc register 
 *   SOC_SAND_IN  uint32                                    data -
 *     The data to be writen.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/

uint32 arad_dram_rbus_write_br(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint32 last_dram_ndx,
    SOC_SAND_IN uint32 addr,
    SOC_SAND_IN uint32 data
  );

/*********************************************************************
* NAME:
 *   arad_dram_rbus_read
 * TYPE:
 *   PROC
 * FUNCTION:
 *   perform indirect read from PHY rbus protocol (inner registers)
 * INPUT:
 *   SOC_SAND_IN  int                                    unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                    addr -
 *     The address to read.
 *   SOC_SAND_IN  uint32                                    dram_ndx -
 *     Dram index. Range: 0 - 7.
 *   SOC_SAND_OUT  uint32                                   data -
 *     The data will be get.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32 arad_dram_rbus_read(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  uint32 dram_ndx,
    SOC_SAND_IN  uint32 addr,
    SOC_SAND_OUT uint32 *data
  );

/*********************************************************************
* NAME:
 *   arad_dram_rbus_modify
 * TYPE:
 *   PROC
 * FUNCTION:
 *   perform indirect modify from PHY rbus protocol (inner registers)
 *  function read addrss, mask the relevant bits and write according to the new data
 * INPUT:
 *   SOC_SAND_IN  int                                    unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                    addr -
 *     The address to modify.
 *   SOC_SAND_IN  uint32                                    dram_ndx -
 *     Dram index. Range: 0 - 7.
 *   SOC_SAND_OUT  uint32                                   data -
 *     The data will be set.
 *   SOC_SAND_OUT  uint32                                   mask -
 *     The data masking.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32 arad_dram_rbus_modify(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  uint32 dram_ndx,
    SOC_SAND_IN  uint32 addr,
    SOC_SAND_IN  uint32 data,
    SOC_SAND_IN  uint32 mask
);

/*
 *  Arad Dram Bist
 */

uint32
  arad_dram_configure_bist(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint32 dram_ndx,
    SOC_SAND_IN uint32 write_weight,
    SOC_SAND_IN uint32 read_weight,
    SOC_SAND_IN uint32 pattern_bit_mode,
    SOC_SAND_IN uint32 two_addr_mode,
    SOC_SAND_IN uint32 prbs_mode,
    SOC_SAND_IN uint32 cons_addr_8_banks,
    SOC_SAND_IN uint32 address_shift_mode,
    SOC_SAND_IN uint32 data_shift_mode,
    SOC_SAND_IN uint32 data_addr_mode,
    SOC_SAND_IN uint32 bist_num_actions,
    SOC_SAND_IN uint32 bist_start_address,
    SOC_SAND_IN uint32 bist_end_address,
    SOC_SAND_IN uint32 bist_pattern[ARAD_DRAM_BIST_NOF_PATTERNS]
  );

uint32
  arad_dram_bist_atomic_action(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint32 dram_ndx,
    SOC_SAND_IN uint8  is_infinite,
    SOC_SAND_IN uint8  stop
  );

uint32
  arad_dram_bist_atomic_action_start(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint32 dram_ndx,
    uint8 start
  );

uint32
  arad_dram_bist_atomic_action_polling(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint32 dram_ndx
  );

uint32
  arad_dram_read_bist_err_cnt(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  uint32 dram_ndx,
    SOC_SAND_OUT uint32 *bist_err_occur,
    SOC_SAND_OUT uint32 *bist_full_err_cnt,
    SOC_SAND_OUT uint32 *bist_single_err_cnt,
    SOC_SAND_OUT uint32 *bist_global_err_cnt
  );

uint32
    arad_dram_bist_test_start_verify(
        SOC_SAND_IN int unit,
        SOC_SAND_IN uint32 dram_ndx,
        SOC_SAND_IN ARAD_DRAM_BIST_TEST_RUN_INFO *info
    );

uint32
    arad_dram_bist_test_start_unsafe(
      SOC_SAND_IN int unit,
      SOC_SAND_IN uint32 dram_ndx,
      SOC_SAND_IN ARAD_DRAM_BIST_TEST_RUN_INFO *info
    );

/*
 * Init ddr
 */
uint32
    arad_dram_init_ddr(
        SOC_SAND_IN int                unit,
        SOC_SAND_IN ARAD_INIT_DDR         *init
    );


/*
 * this proc power down unused DPRCs. 
 */
uint32 arad_dram_init_power_down_unused_dprcs(
        SOC_SAND_IN int unit,
        SOC_SAND_IN uint8* is_valid_dram
    );

/*********************************************************************
* NAME:
 *   arad_dram_mmu_indirect_get_logical_address_full_unsafe
 * FUNCTION:
 *   get address for DRAM mmu indirect reading/writing, in logical_mode. 
 * INPUT:
 *   SOC_SAND_IN  int                                    unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                    buf_num -
 *     the buf number. 
 *   SOC_SAND_IN uint32                                           index -
 *      the index refer to the buffer division to 64 bytes entries .
 *      should be smaller than buffer size (in bytes), divided to 64.
 *   SOC_SAND_OUT uint32                                    addr_full -
 *      return the address as it should be given to arad_dram_mmu_indirect_read/write in logical mode.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/

uint32
    arad_dram_mmu_indirect_get_logical_address_full_unsafe(
         SOC_SAND_IN int unit,
         SOC_SAND_IN uint32 buf_num,
         SOC_SAND_IN uint32 index,
         SOC_SAND_OUT uint32* addr_full 
    );

uint32 
arad_dram_logical2physical_addr_mapping(
    int unit, 
    int buf_num,
    int index,
    uint32 *phy_addr); 

/*********************************************************************
* NAME:
 *   arad_dram_mmu_indirect_read_unsafe/arad_dram_mmu_indirect_write_unsafe
 * FUNCTION:
 *   perform mmu indirect reading or writing from the DRAM. 
 * INPUT:
 *   SOC_SAND_IN  int                                    unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                    logical_mod -
 *     if TRUE the address will represent the DRAM logical address, else the address will represent the physical address. 
 *   SOC_SAND_IN uint32                                           addr -
 *      logical or physical address of buffer. 
 *      should be no longer than 21 bits.
 *      in logical_mod - the address will be :
 *          first bits (4 bits if the size DRAM buffer is 1K, or 5 bit if the size of DRAM buffer is 2K) - is the index.
 *          following 21 bits are the index numbers.
 *          can use arad_dram_mmu_indirect_get_logical_address_full() , to get logical address.
 *      in physical mode -
 *          0-2 bits - bank number.
 *          3-5 bits - DRAM index.
 *          6-11 bits - column.
 *          12- - row.
 *   soc_reg_above_64_val_t                                       data -
 *      read from /write on only the first 64 bytes, size of DRAM entry.  
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/


uint32
    arad_dram_mmu_indirect_read_unsafe(
        SOC_SAND_IN int unit,
        SOC_SAND_IN uint32 logical_mod,
        SOC_SAND_IN uint32 addr,
        soc_reg_above_64_val_t* data
    );
    
uint32
    arad_dram_mmu_indirect_write_unsafe(
        SOC_SAND_IN int unit,
        SOC_SAND_IN uint32 logical_mod,
        SOC_SAND_IN uint32 addr,
        soc_reg_above_64_val_t* data
    );
  
uint32
soc_arad_user_buffer_dram_access(
     SOC_SAND_IN        int unit, 
     SOC_SAND_IN        uint32 flags, 
     SOC_SAND_IN        uint32 access_type, 
     SOC_SAND_INOUT     uint8 *buf, 
     SOC_SAND_IN        int offset, 
     SOC_SAND_IN        int nbytes);

uint32
soc_arad_user_buffer_dram_write(
    SOC_SAND_IN         int unit, 
    SOC_SAND_IN         uint32 flags, 
    SOC_SAND_INOUT      uint8 *buf, 
    SOC_SAND_IN         int offset, 
    SOC_SAND_IN         int nbytes);
  
uint32
soc_arad_user_buffer_dram_read(
    SOC_SAND_IN         int unit, 
    SOC_SAND_IN         uint32 flags, 
    SOC_SAND_INOUT      uint8 *buf, 
    SOC_SAND_IN         int offset, 
    SOC_SAND_IN         int nbytes);
  
/*********************************************************************
* NAME:
 *   arad_dram_crc_delete_buffer_enable_unsafe()
 * FUNCTION:
 *   Configure whether DRAM buffers ,that found with error by the IPT , will be deleted .
 *   if deleted buffer was enable:
 *        - A FIFO of deleted buffers can be read by arad_dram_delete_buffer_read_fifo().
 *        - buffer that was deleted can be released by arad_dram_delete_buffer_action() (arg sould_delete =0).
 *        - Interrupt related - CrcDeletedBuffersFifoNotEmpty. asserted when buffer added to the deleted FIFO. 
 *                              if unmasked call_back func will read the deleted buffers FIFO, and will release 
 *                              or released each buffer, decision based on some test. 
 *                              
 * INPUT:
 *   SOC_SAND_IN  int                                    unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                    enable - 
 *     enable/disable deletion of DRAM buffers with crc error.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
 
uint32
    arad_dram_crc_delete_buffer_enable_unsafe(
        SOC_SAND_IN int unit,
        SOC_SAND_IN uint32 fifo_depth);   
  
 
/*********************************************************************
* NAME:
 *   arad_dram_appl_max_buff_crc_err_get_unsafe()
 * FUNCTION:
 *   This function get the max value of crc error allowed by interrupt application.
 *   This value is set by dram_crc_delete_buffer_enable second argument..
 *                              
 * INPUT:
 *   SOC_SAND_IN  int                                    unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT  uint32                                   max_val - 
 *     max value of crc error allowed for buffer by application.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/ 
  
uint32   
    arad_dram_crc_del_buffer_max_reclaims_set_unsafe(
        SOC_SAND_IN int unit,
        SOC_SAND_IN uint32 max_err);  
  
uint32   
    arad_dram_crc_del_buffer_max_reclaims_get_unsafe(
        SOC_SAND_IN int unit,
        SOC_SAND_OUT uint32 *max_err);
 
/*********************************************************************
* NAME:
 *   arad_dram_buffer_get_info_unsafe()
 * FUNCTION:
 *   Get information on DRAM buffer.
 * INPUT:
 *   SOC_SAND_IN  int                                    unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                    buf -
 *     the buffer , as it get from arad_dram_delete_buffer_read_fifo().
 *   SOC_SAND_OUT arad_dram_buffer_info_t                  *buf_info -
 *     array that will be fill with the deleted buffers.                                         
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
    arad_dram_buffer_get_info_unsafe(
            SOC_SAND_IN int unit,
            SOC_SAND_IN  uint32 buf,
            SOC_SAND_OUT arad_dram_buffer_info_t *buf_info);
       
uint32
    arad_dram_buffer_get_info_verify(
            SOC_SAND_IN int unit,
            SOC_SAND_IN  uint32 buf,
            SOC_SAND_OUT arad_dram_buffer_info_t *buf_info);       
         
/*********************************************************************
* NAME:
 *   arad_dram_delete_buffer_read_fifo_unsafe()
 * FUNCTION:
 *   Read the deleted buffers FIFO and empty each value that was reading.
 * INPUT:
 *   SOC_SAND_IN  int                                    unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                    max_entries -
 *     num of entries in del_buf_array.
 *   SOC_SAND_OUT arad_dram_buffer_info_t                  *del_buf_array -
 *     array that will be fill with the deleted buffers.
 *   SOC_SAND_OUT uint32                                   *count_entries -
 *      number of entries del_buf_array that was filled ( MAX(max_entries, num of deleted buffer in the FIFO) ).                                          
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/

uint32
    arad_dram_delete_buffer_read_fifo_unsafe(
           SOC_SAND_IN int unit, 
           SOC_SAND_IN uint32 del_buf_max, 
           SOC_SAND_OUT uint32 *del_buf_array, 
           SOC_SAND_OUT uint32 *del_buf_count);

uint32
    arad_dram_delete_buffer_read_fifo_verify(
           SOC_SAND_IN int unit, 
           SOC_SAND_IN uint32 del_buf_max,
           SOC_SAND_OUT uint32 *del_buf_array, 
           SOC_SAND_OUT uint32 *del_buf_count);
                   
/*********************************************************************
* NAME:
 *   arad_dram_delete_buffer_test_unsafe()
 * FUNCTION:
 *   write known pattern to the DRAM deleted buffer, than read and compare the result.
 *   pass through all the buffer.
 * INPUT:
 *   SOC_SAND_IN  int                                    unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                    del_buf -
 *     the deleted buffer, as it get from arad_dram_delete_buffer_read_fifo().
 *     if buffer was deleted (as a result of error than was found by the IPT or MMU)
 *     get the deleted buffer list by arad_dram_delete_buffer_read_fifo().
 *   SOC_SAND_OUT uint32                                    *is_pass -
 *     set to TRUE if the buffer pass the test .                                       
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/

uint32
    arad_dram_delete_buffer_test_unsafe(
           SOC_SAND_IN int unit, 
           SOC_SAND_IN uint32 del_buf, 
           SOC_SAND_OUT uint32 *is_pass);

uint32
    arad_dram_delete_buffer_test_verify(
           SOC_SAND_IN int unit, 
           SOC_SAND_IN uint32 del_buf,
           SOC_SAND_OUT uint32 *is_pass);

/*********************************************************************
* NAME:
 *   arad_dram_delete_buffer_action_unsafe()
 * FUNCTION:
 *    Delete or release a DRAM buffer.
 * INPUT:
 *   SOC_SAND_IN  int                                    unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                    buf -
 *     the DRAM buffer , as it get from arad_dram_delete_buffer_read_fifo(). 
 *   SOC_SAND_IN uint32                             should_delete -
 *      if TRUE delete the buffer else release the buffer.                                       
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/

uint32
    arad_dram_delete_buffer_action_unsafe(
        SOC_SAND_IN int unit, 
        SOC_SAND_IN uint32 buf, 
        SOC_SAND_IN uint32 should_delete);

uint32
    arad_dram_delete_buffer_action_verify(
        SOC_SAND_IN int unit, 
        SOC_SAND_IN uint32 buf);

/*********************************************************************
* NAME:
 *   arad_dram_init_buffer_error_cntr_unsafe()
 * FUNCTION:
 *    Init the array of buffers error counter.
 *    Should be call in the beginning of the application with ARAD_DRAM_BUF_NUM_ALL,
 *    Or to zero counters, after a while..
 * INPUT:
 *   SOC_SAND_IN  int                                    unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN uint32                                     buf -                                                               
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
    arad_dram_init_buffer_error_cntr_unsafe(
        SOC_SAND_IN int unit, 
        SOC_SAND_IN uint32 buf);

uint32
    arad_dram_init_buffer_error_cntr_verify(
        SOC_SAND_IN int unit, 
        SOC_SAND_IN uint32 buf);

/*********************************************************************
* NAME:
 *   arad_dram_get_buffer_error_cntr_info_unsafe()
 * FUNCTION:
 *    Get info of buffer that is in the error cntr list, by his index in the array.
 *    This function made to get status of all the buffers in the error cntr list.
 * INPUT:
 *   SOC_SAND_IN  int                                    unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN uint32                                     index_in_list -
 *     index of the buffer data info struct in the list of all the buffers that was
 *     detected with errors.
 *     to read the list start with index 0 and stop when is_buff = 0;
 *   SOC_SAND_OUT uint32*                                  is_buf
 *     TRUE if there is buffer info in the array index given.
 *   SOC_SAND_OUT arad_dram_buffer_info_t                   *buf -
 *     if is_buf=TRUE, will be filed with the buffer in the index.                                                                 
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
    arad_dram_get_buffer_error_cntr_in_list_index_unsafe(
        SOC_SAND_IN int unit, 
        SOC_SAND_IN uint32 index_in_list, 
        SOC_SAND_OUT uint32 *is_buf,
        SOC_SAND_OUT uint32 *buf );

uint32
    arad_dram_get_buffer_error_cntr_in_list_index_verify(
        SOC_SAND_IN int unit, 
        SOC_SAND_IN uint32 index_in_list,
        SOC_SAND_OUT uint32 *is_buf,
        SOC_SAND_OUT uint32 *buf);

int soc_arad_validate_dram_address (int unit, uint32 addr);

/*
 * Ardon Dram.  
 * Support DRC Combo28. 
 */
int soc_ardon_dram_info_verify(int unit, soc_dpp_drc_combo28_info_t *drc_info);
int soc_dpp_drc_combo28_dram_init(int unit, soc_dpp_drc_combo28_info_t *drc_info);

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_DRAM_INCLUDED__*/

#endif
