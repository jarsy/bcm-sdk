/* $Id: arad_api_dram.h,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_API_DRAM_INCLUDED__
/* { */
#define __ARAD_API_DRAM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/TMC/tmc_api_dram.h>
#include <soc/dpp/ARAD/arad_framework.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_DRAM_VAL_LSB                                ( 0)
#define ARAD_DRAM_VAL_MSB                                (30)
#define ARAD_DRAM_VAL_SHIFT                              (ARAD_DRAM_VAL_LSB)
#define ARAD_DRAM_VAL_MASK                               (SOC_SAND_BITS_MASK(ARAD_DRAM_VAL_MSB, ARAD_DRAM_VAL_LSB))

#define ARAD_DRAM_VAL_IS_IN_CLOCKS_LSB                   (31)
#define ARAD_DRAM_VAL_IS_IN_CLOCKS_MSB                   (31)
#define ARAD_DRAM_VAL_IS_IN_CLOCKS_SHIFT                 (ARAD_DRAM_VAL_IS_IN_CLOCKS_LSB)
#define ARAD_DRAM_VAL_IS_IN_CLOCKS_MASK                  (SOC_SAND_BITS_MASK(ARAD_DRAM_VAL_IS_IN_CLOCKS_MSB, ARAD_DRAM_VAL_IS_IN_CLOCKS_LSB))

/*     Number of longs in a DRAM pattern of 256 bits.          */
#define  ARAD_DRAM_PATTERN_SIZE_IN_UINT32S (8)

#define ARAD_DRAM_TYPE_DDR3                                SOC_TMC_DRAM_TYPE_DDR3
#define ARAD_DRAM_NOF_TYPES                                SOC_TMC_DRAM_NOF_TYPES
typedef SOC_TMC_DRAM_TYPE                                      ARAD_DRAM_TYPE;

#define ARAD_DRAM_NUM_COLUMNS_256                          SOC_TMC_DRAM_NUM_COLUMNS_256
#define ARAD_DRAM_NUM_COLUMNS_512                          SOC_TMC_DRAM_NUM_COLUMNS_512
#define ARAD_DRAM_NUM_COLUMNS_1024                         SOC_TMC_DRAM_NUM_COLUMNS_1024
#define ARAD_DRAM_NUM_COLUMNS_2048                         SOC_TMC_DRAM_NUM_COLUMNS_2048
#define ARAD_DRAM_NUM_COLUMNS_4096                         SOC_TMC_DRAM_NUM_COLUMNS_4096
#define ARAD_DRAM_NUM_COLUMNS_8192                         SOC_TMC_DRAM_NUM_COLUMNS_8192
#define ARAD_NOF_DRAM_NUMS_COLUMNS                         SOC_TMC_NOF_DRAM_NUMS_COLUMNS
typedef SOC_TMC_DRAM_NUM_COLUMNS                               ARAD_DRAM_NUM_COLUMNS;                              

#define ARAD_DRAM_NUM_ROWS_8192                          SOC_TMC_DRAM_NUM_ROWS_8192
#define ARAD_DRAM_NUM_ROWS_16384                        SOC_TMC_DRAM_NUM_ROWS_16384
#define ARAD_NOF_DRAM_NUMS_ROWS                         SOC_TMC_NOF_DRAM_NUMS_ROWS
typedef SOC_TMC_DRAM_NUM_ROWS                                     ARAD_DRAM_NUM_ROWS;

#define ARAD_DRAM_NUM_BANKS_4                              SOC_TMC_DRAM_NUM_BANKS_4
#define ARAD_DRAM_NUM_BANKS_8                              SOC_TMC_DRAM_NUM_BANKS_8
#define ARAD_NOF_DRAM_NUM_BANKS                            SOC_TMC_NOF_DRAM_NUM_BANKS
typedef SOC_TMC_DRAM_NUM_BANKS                                 ARAD_DRAM_NUM_BANKS;

#define ARAD_DRAM_AP_POSITION_08                           SOC_TMC_DRAM_AP_POSITION_08
#define ARAD_DRAM_AP_POSITION_09                           SOC_TMC_DRAM_AP_POSITION_09
#define ARAD_DRAM_AP_POSITION_10                           SOC_TMC_DRAM_AP_POSITION_10
#define ARAD_DRAM_AP_POSITION_11                           SOC_TMC_DRAM_AP_POSITION_11
#define ARAD_DRAM_AP_POSITION_12                           SOC_TMC_DRAM_AP_POSITION_12
#define ARAD_NOF_DRAM_AP_POSITIONS                         SOC_TMC_NOF_DRAM_AP_POSITIONS
typedef SOC_TMC_DRAM_AP_POSITION                               ARAD_DRAM_AP_POSITION;

#define ARAD_DRAM_BURST_SIZE_16                            SOC_TMC_DRAM_BURST_SIZE_16
#define ARAD_DRAM_BURST_SIZE_32                            SOC_TMC_DRAM_BURST_SIZE_32
#define ARAD_DRAM_NOF_BURST_SIZES                          SOC_TMC_DRAM_NOF_BURST_SIZES
typedef SOC_TMC_DRAM_BURST_SIZE                                ARAD_DRAM_BURST_SIZE;

#define ARAD_DDR_CLAM_SHELL_MODE_DISABLED                  SOC_TMC_DRAM_CLAM_SHELL_MODE_DISABLED
#define ARAD_DDR_CLAM_SHELL_MODE_DRAM_0                    SOC_TMC_DRAM_CLAM_SHELL_MODE_DRAM_0
#define ARAD_DDR_CLAM_SHELL_MODE_DRAM_1                    SOC_TMC_DRAM_CLAM_SHELL_MODE_DRAM_1
#define ARAD_DDR_NOF_CLAM_SHELL_MODES                      SOC_TMC_NOF_DRAM_CLAM_SHELL_MODE
typedef SOC_TMC_DRAM_CLAM_SHELL_MODE                       ARAD_DDR_CLAM_SHELL_MODE;

/* 
 * ARAD DRAM BIST
 */
#define ARAD_DRAM_BIST_NOF_PATTERNS (8)

/* Arad Dram Bist Flags*/
#define ARAD_DRAM_BIST_CONS_ADDR_8_BANKS            SOC_TMC_DRAM_BIST_FLAGS_CONS_ADDR_8_BANKS 
#define ARAD_DRAM_BIST_ADDRESS_SHIFT_MODE           SOC_TMC_DRAM_BIST_FLAGS_ADDRESS_SHIFT_MODE
#define ARAD_DRAM_BIST_INFINITE                     SOC_TMC_DRAM_BIST_FLAGS_INFINITE 
#define ARAD_DRAM_BIST_ALL_ADDRESS                  SOC_TMC_DRAM_BIST_FLAGS_ALL_ADDRESS
#define ARAD_DRAM_BIST_STOP                         SOC_TMC_DRAM_BIST_FLAGS_STOP
#define ARAD_DRAM_BIST_GET_DATA                     SOC_TMC_DRAM_BIST_FLAGS_GET_DATA 
#define ARAD_DRAM_BIST_TWO_ADDRESS_MODE             SOC_TMC_DRAM_BIST_FLAGS_TWO_ADDRESS_MODE
#define ARAD_DRAM_BIST_BG_INTERLEAVE                SOC_TMC_DRAM_BIST_FLAGS_BG_INTERLEAVE
#define ARAD_DRAM_BIST_SINGLE_BANK_TEST             SOC_TMC_DRAM_BIST_FLAGS_SINGLE_BANK_TEST

#define ARAD_DRAM_BUF_NUM_ALL 0xffffffff
#define ARAD_DRAM_MAX_BUFFERS_IN_ERROR_CNTR 1000

/* } */

/*************
 * MACROS    *
 *************/
/* { */

#define ARAD_DRAM_VAL_IN_CLOCKS(val)                 \
          (SOC_SAND_SET_FLD_IN_PLACE(val, ARAD_DRAM_VAL_SHIFT, ARAD_DRAM_VAL_MASK)) | \
          (SOC_SAND_SET_FLD_IN_PLACE(0x1, ARAD_DRAM_VAL_IS_IN_CLOCKS_SHIFT, ARAD_DRAM_VAL_IS_IN_CLOCKS_MASK))

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

typedef struct arad_api_dram_ddr3_s{

    SOC_SAND_MAGIC_NUM_VAR
    /*
     *  DDR3 - MRS0 (1st write)
     */
    uint32 mrs0_wr1;
    /*
     *  DDR3 - MRS0 (2nd write)
     */
    uint32 mrs0_wr2;
    /*
     *  DDR3 - MRS1 (1st write)
     */
    uint32 mrs1_wr1;
    /*
     *  DDR3 - MRS2 (1st write)
     */
    uint32 mrs2_wr1;
    /*
     *  DDR3 - MRS3 (1st write)
     */
    uint32 mrs3_wr1;
} arad_api_dram_ddr3_t;

typedef union
{
    arad_api_dram_ddr3_t ddr3;
} ARAD_DRAM_MODE_REGS_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  T - Automatically calculate the mode registers (according to JEDEK)
   *  F - Use the values specified in 'mode_regs' for the mode registers
   */
  uint8 auto_mode;
  /*
   *  Number of Banks
   */
  ARAD_DRAM_NUM_BANKS nof_banks;
  /*
   *  Number of columns
   */
  ARAD_DRAM_NUM_COLUMNS nof_cols;
  /*
   *  Auto precharge bit position. Determines the position of the Auto
   *  Precharge bit in the address going to the DRAM
   */
  ARAD_DRAM_AP_POSITION ap_bit_pos;
  /*
   *  Dram burst size. May be 16 or 32 bytes. Must be set
   *  according to the dram's burst size
   */
  ARAD_DRAM_BURST_SIZE burst_size;
  /*
   *  Column Address Strobe latency. The period (clocks) between
   *  READ command and valid read data presented on the data
   *  out pins of the dram
   */
  uint32 c_cas_latency;
  /*
   *  The period (clocks) between write command and write data set
   *  on the dram data in pins
   */
  uint32 c_wr_latency;
  /*
   *  Refresh Cycle. Period between the active to the
   *  active/auto refresh commands (tRC)
   */
  uint32 t_rc;
  /*
   *  Row Refresh Cycle. Auto refresh command period. The
   *  minimal period between the refresh command and the
   *  next active command (tRFC)
   *  By default this period is stated in terms of picoseconds. To state
   *  it in terms of number of clocks use the macro ARAD_DRAM_VAL_IN_CLOCKS
   */
  uint32 t_rfc;
  /*
   *  Row Address Strobe. The minimal period needed to
   *  access a certain row of data in RAM between the data
   *  request and the precharge command (tRAS)
   *  By default this period is stated in terms of picoseconds. To state
   *  it in terms of number of clocks use the macro ARAD_DRAM_VAL_IN_CLOCKS
   */
  uint32 t_ras;
  /*
   *  Four Active Window. No more than four banks may be
   *  activated in a rolling window (tFAW)
   *  By default this period is stated in terms of picoseconds. To state
   *  it in terms of number of clocks use the macro ARAD_DRAM_VAL_IN_CLOCKS
   */
  uint32 t_faw;
  /*
   *  Row address to Column address Delay. The minimal period
   *  needed between RAS and CAS. It is the time required
   *  between row activation and read access to the column of
   *  the given memory block (tRcdRd)
   *  By default this period is stated in terms of picoseconds. To state
   *  it in terms of number of clocks use the macro ARAD_DRAM_VAL_IN_CLOCKS
   */
  uint32 t_rcd_rd;
  /*
   *  Row address to Column address Delay. The minimal period
   *  needed between RAS and CAS. It is the time required
   *  between row activation and write access to the column of
   *  the given memory block (tRcdWr)
   *  By default this period is stated in terms of picoseconds. To state
   *  it in terms of number of clocks use the macro ARAD_DRAM_VAL_IN_CLOCKS
   */
  uint32 t_rcd_wr;
  /*
   *  RAS To RAS delay. Active bank a to active bank command (tRRD)
   *  By default this period is stated in terms of picoseconds. To state
   *  it in terms of number of clocks use the macro ARAD_DRAM_VAL_IN_CLOCKS
   */
  uint32 t_rrd;
  /*
   *  Row Precharge. The minimal period between pre-charge
   *  action of a certain Row and the next consecutive action
   *  to the same bank/row (tRP)
   *  By default this period is stated in terms of picoseconds. To state
   *  it in terms of number of clocks use the macro ARAD_DRAM_VAL_IN_CLOCKS
   */
  uint32 t_rp;
  /*
   *  Write Recovery Time. Specifies the period that must
   *  elapse after the completion of a valid write operation,
   *  before a pre-charge command can be issued (tWR)
   *  By default this period is stated in terms of picoseconds. To state
   *  it in terms of number of clocks use the macro ARAD_DRAM_VAL_IN_CLOCKS
   */
  uint32 t_wr;
  /*
   *  Average periodic refresh interval.
   *  By default this period is stated in terms of picoseconds. To state
   *  it in terms of number of clocks use the macro ARAD_DRAM_VAL_IN_CLOCKS
   *  The value 0 disables the auto refresh mechanism.
   */
  uint32 t_ref;
  /*
   *  Write To Read Delay. The minimal period that must
   *  elapse between the last valid write operation and the
   *  next read command to the same internal bank of the DDR
   *  device (tWTR)
   *  By default this period is stated in terms of picoseconds. To state
   *  it in terms of number of clocks use the macro ARAD_DRAM_VAL_IN_CLOCKS
   */
  uint32 t_wtr;
  /*
   *  Read To Precharge Delay (tRTP)
   *  By default this period is stated in terms of picoseconds. To state
   *  it in terms of number of clocks use the macro ARAD_DRAM_VAL_IN_CLOCKS
   */
  uint32 t_rtp;
  /*
   * Jedec - Joint Electron Device Engineering Council
   */
  uint32 jedec;
  /*
   *  Mode registers configuration
   */
  ARAD_DRAM_MODE_REGS_INFO mode_regs;

} ARAD_DRAM_INFO;


typedef  SOC_TMC_DRAM_BIST_DATA_PATTERN_MODE ARAD_DRAM_BIST_DATA_PATTERN_MODE; 

typedef  SOC_TMC_DRAM_BIST_INFO ARAD_DRAM_BIST_TEST_RUN_INFO; 

typedef struct arad_dram_buffer_info_s{
    uint32 buf_num;
    uint32 bank;
    char channel;
    uint32 err_cntr; /* num of detected error */
    uint32 is_deleted; /* is buffer deleted */
} arad_dram_buffer_info_t;

/*************
 * FUNCTIONS *
 *************/
/* { */

/*********************************************************************
* NAME:
*   arad_dram_info_set
* TYPE:
*   PROC
* FUNCTION:
*   This function configures the dram according to the
*   provided sets of parameters. This function is called
*   during the initialization sequence and must not be
*   called afterwards
* INPUT:
*   SOC_SAND_IN  int                 unit -
*     Identifier of the device to access
*   SOC_SAND_IN  uint32                 dram_ndx -
*     Dram index. Range: 0-5
*   SOC_SAND_IN  uint32                 dram_freq -
*     Dram frequency (MHz)
*   SOC_SAND_IN  ARAD_DRAM_TYPE           dram_type -
*     One of three supported dram types (DDR3)
*   SOC_SAND_IN  ARAD_DRAM_INFO           *info -
*     Dram configuration information
* REMARKS:
*   None
* RETURNS:
*   OK or ERROR indication
*********************************************************************/
uint32
  arad_dram_info_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 dram_ndx,
    SOC_SAND_IN  uint32                 dram_freq,
    SOC_SAND_IN  ARAD_DRAM_TYPE           dram_type,
    SOC_SAND_IN  ARAD_DRAM_INFO           *info
  );

/*********************************************************************
* NAME:
*   arad_dram_info_get
* TYPE:
*   PROC
* FUNCTION:
*   This function configures the dram according to the
*   provided sets of parameters. This function is called
*   during the initialization sequence and must not be
*   called afterwards
* INPUT:
*   SOC_SAND_IN  int                 unit -
*     Identifier of the device to access
*   SOC_SAND_IN  uint32                 dram_ndx -
*     Dram index. Range: 0-5
*   SOC_SAND_IN  uint32                 dram_freq -
*     Dram frequency (MHz)
*   SOC_SAND_OUT ARAD_DRAM_TYPE           *dram_type -
*     One of three supported dram types (DDR3)
*   SOC_SAND_OUT ARAD_DRAM_INFO           *info -
*     Dram configuration information
* REMARKS:
*   None
* RETURNS:
*   OK or ERROR indication
*********************************************************************/
uint32
  arad_dram_info_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 dram_ndx,
    SOC_SAND_IN  uint32                 dram_freq,
    SOC_SAND_OUT ARAD_DRAM_TYPE           *dram_type,
    SOC_SAND_OUT ARAD_DRAM_INFO           *info
  );


uint32
    arad_dram_bist_test_start(
      SOC_SAND_IN int unit,
      SOC_SAND_IN uint32 dram_ndx,
      SOC_SAND_IN ARAD_DRAM_BIST_TEST_RUN_INFO *info
    );


/*********************************************************************
* NAME:
 *   arad_dram_mmu_indirect_get_logical_address_full
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
    arad_dram_mmu_indirect_get_logical_address_full(
         SOC_SAND_IN int unit,
         SOC_SAND_IN uint32 buf_num,
         SOC_SAND_IN uint32 index,
         SOC_SAND_OUT uint32* addr_full 
    );

/*********************************************************************
* NAME:
 *   arad_dram_mmu_indirect_read_unsafe/arad_dram_mmu_indirect_write
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
    arad_dram_mmu_indirect_read(
        SOC_SAND_IN int unit,
        SOC_SAND_IN uint32 logical_mod,
        SOC_SAND_IN uint32 addr,
        soc_reg_above_64_val_t* data
    );
    
uint32
    arad_dram_mmu_indirect_write(
        SOC_SAND_IN int unit,
        SOC_SAND_IN uint32 logical_mod,
        SOC_SAND_IN uint32 addr,
        soc_reg_above_64_val_t* data
    );

/*********************************************************************
* NAME:
 *   arad_dram_crc_delete_buffer_enable()
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
    arad_dram_crc_delete_buffer_enable(
        SOC_SAND_IN int unit,
        SOC_SAND_IN uint32 fifo_depth);  
 
/*********************************************************************
* NAME:
 *   arad_dram_crc_del_buffer_max_reclaims_get/set()
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
    arad_dram_crc_del_buffer_max_reclaims_set(
        SOC_SAND_IN int unit,
        SOC_SAND_IN uint32 max_err);

uint32
    arad_dram_crc_del_buffer_max_reclaims_get(
        SOC_SAND_IN int unit,
        SOC_SAND_OUT uint32 *max_err);

/*********************************************************************
* NAME:
 *   arad_dram_buffer_get_info()
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
    arad_dram_buffer_get_info(
            SOC_SAND_IN int unit,
            SOC_SAND_IN  uint32 buf,
            SOC_SAND_OUT arad_dram_buffer_info_t *buf_info);      
            
/*********************************************************************
* NAME:
 *   arad_dram_delete_buffer_read_fifo()
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
    arad_dram_delete_buffer_read_fifo(
           SOC_SAND_IN int unit, 
           SOC_SAND_IN uint32 del_buf_max, 
           SOC_SAND_OUT uint32 *del_buf_array, 
           SOC_SAND_OUT uint32 *del_buf_count);
                   
/*********************************************************************
* NAME:
 *   arad_dram_delete_buffer_test()
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
    arad_dram_delete_buffer_test(
           SOC_SAND_IN int unit, 
           SOC_SAND_IN uint32 del_buf, 
           SOC_SAND_OUT uint32 *is_pass);

/*********************************************************************
* NAME:
 *   arad_dram_delete_buffer_action()
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
    arad_dram_delete_buffer_action(
        SOC_SAND_IN int unit, 
        SOC_SAND_IN uint32 buf, 
        SOC_SAND_IN uint32 should_delete);

/*********************************************************************
* NAME:
 *   arad_dram_init_buffer_error_cntr()
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
    arad_dram_init_buffer_error_cntr(
        SOC_SAND_IN int unit, 
        SOC_SAND_IN uint32 buf);

/*********************************************************************
* NAME:
 *   arad_dram_get_buffer_error_cntr_info()
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
   arad_dram_get_buffer_error_cntr_in_list_index(
        SOC_SAND_IN int unit, 
        SOC_SAND_IN uint32 index_in_list, 
        SOC_SAND_OUT uint32 *is_buf,
        SOC_SAND_OUT uint32 *buf);

void
  arad_ARAD_DRAM_MR_INFO_clear(
    SOC_SAND_OUT ARAD_DRAM_MODE_REGS_INFO  *info
  );

void
  arad_ARAD_DRAM_INFO_clear(
    SOC_SAND_OUT ARAD_DRAM_INFO *info
  );

#if ARAD_DEBUG_IS_LVL1

const char*
  arad_ARAD_DRAM_TYPE_to_string(
    SOC_SAND_IN ARAD_DRAM_TYPE enum_val
  );

const char*
  arad_ARAD_DRAM_BURST_SIZE_to_string(
    SOC_SAND_IN ARAD_DRAM_BURST_SIZE enum_val
  );

const char*
  arad_ARAD_DRAM_NUM_BANKS_to_string(
    SOC_SAND_IN ARAD_DRAM_NUM_BANKS enum_val
  );

const char*
  arad_ARAD_DRAM_NUM_COLUMNS_to_string(
    SOC_SAND_IN ARAD_DRAM_NUM_COLUMNS enum_val
  );

const char*
  arad_ARAD_DRAM_AP_POSITION_to_string(
    SOC_SAND_IN ARAD_DRAM_AP_POSITION enum_val
  );


void
  arad_ARAD_DRAM_MR_INFO_print(
    SOC_SAND_IN ARAD_DRAM_TYPE           dram_type,
    SOC_SAND_IN ARAD_DRAM_MODE_REGS_INFO *info
  );

void
  arad_ARAD_DRAM_INFO_print(
    SOC_SAND_IN ARAD_DRAM_TYPE dram_type,
    SOC_SAND_IN ARAD_DRAM_INFO *info
  );



#endif /* ARAD_DEBUG_IS_LVL1 */
/* } */
#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_API_DRAM_INCLUDED__*/
#endif

