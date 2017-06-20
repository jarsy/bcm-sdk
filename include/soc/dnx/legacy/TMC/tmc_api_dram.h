/* $Id: jer2_jer2_jer2_tmc_api_dram.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_jer2_jer2_tmc/src/soc_petra_api_dram.h
*
* MODULE PREFIX:  soc_jer2_jer2_jer2_tmcdram
*
* FILE DESCRIPTION:
*
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement
******************************************************************/

#ifndef __DNX_TMC_API_DRAM_INCLUDED__
/* { */
#define __DNX_TMC_API_DRAM_INCLUDED__

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define DNX_TMC_BIST_NOF_PATTERNS 8  

#define DNX_TMC_BIST_NOF_SEEDS 8

   /*  
    * DRAM BIST FLAGS    
    */
#define DNX_TMC_DRAM_BIST_FLAGS_CONS_ADDR_8_BANKS            (0x0001)
#define DNX_TMC_DRAM_BIST_FLAGS_ADDRESS_SHIFT_MODE           (0X0002)
#define DNX_TMC_DRAM_BIST_FLAGS_INFINITE                     (0x0004)
#define DNX_TMC_DRAM_BIST_FLAGS_ALL_ADDRESS                  (0x0008)
#define DNX_TMC_DRAM_BIST_FLAGS_STOP                         (0x0010)
#define DNX_TMC_DRAM_BIST_FLAGS_GET_DATA                     (0x0020)
#define DNX_TMC_DRAM_BIST_FLAGS_TWO_ADDRESS_MODE             (0x0040)
#define DNX_TMC_DRAM_BIST_FLAGS_BG_INTERLEAVE                (0x0080)
#define DNX_TMC_DRAM_BIST_FLAGS_SINGLE_BANK_TEST             (0x0100)
#define DNX_TMC_DRAM_BIST_FLAGS_MPR_STAGGER_INCREMENT_MODE   (0x0400)
#define DNX_TMC_DRAM_BIST_FLAGS_MPR_READOUT_MODE             (0x0800)
#define DNX_TMC_DRAM_BIST_FLAGS_ADDRESS_PRBS_MODE            (0x1000)
#define DNX_TMC_DRAM_BIST_FLAGS_USE_RANDOM_DATA_SEED         (0x2000)    
#define DNX_TMC_DRAM_BIST_FLAGS_USE_RANDOM_DBI_SEED          (0x4000) 


/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

typedef enum
{
  /*
   *  DRAM type - GDDR3
   */
  DNX_TMC_DRAM_TYPE_GDDR3=1,
  /*
   *  DRAM type - DDR2
   */
  DNX_TMC_DRAM_TYPE_DDR2=2,
  /*
   *  DRAM type - DDR3
   */
  DNX_TMC_DRAM_TYPE_DDR3=3,
  /*
   *  DRAM type - DDR4
   */
  DNX_TMC_DRAM_TYPE_DDR4=4,
  /*
   *  DRAM type - GDDR5
   */
  DNX_TMC_DRAM_TYPE_GDDR5=5,
  /*
   *  Total number of DRAM types
   */
  DNX_TMC_DRAM_NOF_TYPES

}DNX_TMC_DRAM_TYPE;

typedef enum
{
  /*
  *  DRAM Number Of Columns - 256
  */
  DNX_TMC_DRAM_NUM_COLUMNS_256=0,
  /*
  *  DRAM Number Of Columns - 512
  */
  DNX_TMC_DRAM_NUM_COLUMNS_512=1,
  /*
  *  DRAM Number Of Columns - 1024
  */
  DNX_TMC_DRAM_NUM_COLUMNS_1024=2,
  /*
  *  DRAM Number Of Columns - 2048
  */
  DNX_TMC_DRAM_NUM_COLUMNS_2048=3,
  /*
  *  DRAM Number Of Columns - 4096
  */
  DNX_TMC_DRAM_NUM_COLUMNS_4096=4,
  /*
  *  DRAM Number Of Columns - 8192
  */
  DNX_TMC_DRAM_NUM_COLUMNS_8192=5,
  /*
  *  Total number of DRAM types
  */
  DNX_TMC_NOF_DRAM_NUMS_COLUMNS

}DNX_TMC_DRAM_NUM_COLUMNS;

typedef enum
{
 /*
  *  DRAM Number Of Rows - 8192
  */
  DNX_TMC_DRAM_NUM_ROWS_8192=8192,
 /*
  *  DRAM Number Of Rows - 16384
  */
  DNX_TMC_DRAM_NUM_ROWS_16384=16384,
  /*
  *  Total number of DRAM types
  */
  DNX_TMC_NOF_DRAM_NUM_ROWS

}DNX_TMC_DRAM_NUM_ROWS;

typedef enum
{
  /*
   *  Number of DRAM banks - 4 (valid for DDR2)
   */
  DNX_TMC_DRAM_NUM_BANKS_4=4,
  /*
   *  Number of DRAM banks - 8
   */
  DNX_TMC_DRAM_NUM_BANKS_8=8,
  /*
   *  Total number of DRAM number of banks configurations
   */
  DNX_TMC_NOF_DRAM_NUM_BANKS

}DNX_TMC_DRAM_NUM_BANKS;

typedef enum
{
  /*
   *  AP is placed on addr[ 8]
   */
  DNX_TMC_DRAM_AP_POSITION_08=0,
  /*
   *  AP is placed on addr[ 9]
   */
   DNX_TMC_DRAM_AP_POSITION_09=1,
  /*
   *  AP is placed on addr[10]
   */
   DNX_TMC_DRAM_AP_POSITION_10=2,
  /*
   *  AP is placed on addr[11]
   */
   DNX_TMC_DRAM_AP_POSITION_11=3,
  /*
   *  AP is placed on addr[12]
   */
   DNX_TMC_DRAM_AP_POSITION_12=4,
  /*
   *  Total number of DRAM number of banks configurations
   */
  DNX_TMC_NOF_DRAM_AP_POSITIONS

}DNX_TMC_DRAM_AP_POSITION;

typedef enum
{
  /*
   *  16B burst size
   */
  DNX_TMC_DRAM_BURST_SIZE_16=4,
  /*
   *  32B burst size
   */
  DNX_TMC_DRAM_BURST_SIZE_32=8,
  /*
   * Number of dram burst sizes
   */
  DNX_TMC_DRAM_NOF_BURST_SIZES

}DNX_TMC_DRAM_BURST_SIZE;

typedef enum {
  /*
   * both of the drams are disabled
   */
  DNX_TMC_DRAM_CLAM_SHELL_MODE_DISABLED = 0,
  /*
   * only dram 0 is enabled
   */
  DNX_TMC_DRAM_CLAM_SHELL_MODE_DRAM_0 = 1,
  /*
   * only dram 1 is enabled
   */
  DNX_TMC_DRAM_CLAM_SHELL_MODE_DRAM_1 = 2,
  /*
   *  Number of types in DNX_TMC_DDR_CLAM_SHELL_MODE
   */
  DNX_TMC_NOF_DRAM_CLAM_SHELL_MODE = 3

} DNX_TMC_DRAM_CLAM_SHELL_MODE;


typedef enum
{
    /*
     * costum pattern. Use the pattern as is.
     */
    DNX_TMC_DRAM_BIST_DATA_PATTERN_CUSTOM = 0,
    /*
     *  The PRBS will be used to generate the pattern towards
     *  the DRAM.
     */
    DNX_TMC_DRAM_BIST_DATA_PATTERN_RANDOM_PRBS = 1,
    /*
     *  Fill the data to write by 1010101... (Bits). The DATA_MODE
     *  may use this data pattern in different ways see
     *  DNX_TMC_DRAM_BIST_DATA_MODE. 
     */
    DNX_TMC_DRAM_BIST_DATA_PATTERN_DIFF = 2,
    /*
     *  Fill the data to write by 11111111... (Bits). The DATA_MODE
     *  may use this data pattern in different ways see
     *  DNX_TMC_DRAM_BIST_DATA_MODE. Random mode ignores these
     *  values.
     */
    DNX_TMC_DRAM_BIST_DATA_PATTERN_ONE = 3,
    /*
     *  Fill the data to write by 00000000... (Bits). The DATA_MODE
     *  may use this data pattern in different ways see
     *  DNX_TMC_DRAM_BIST_DATA_MODE. Random mode ignores these
     *  values.
     */
    DNX_TMC_DRAM_BIST_DATA_PATTERN_ZERO = 4,
    /*
     * A different bit is selected from pattern0 - pattern7 in an incremental manner.
     * The selected bit is duplicated on all of the dram data bus.
     */
    DNX_TMC_DRAM_BIST_DATA_PATTERN_BIT_MODE = 5,
    /*
     * data shift mode.
     * Every write/read command the data will be shifted 1 bit to the left in a cyclic manner.
     * The initial pattern is pattern0 which is duplicated 8 times.
     * This DATA_MODE create 8 consecutive dram transactions.
     */
    DNX_TMC_DRAM_BIST_DATA_PATTERN_SHIFT_MODE = 6,
    /*
     * data address mode.
     * Every command data will be equal to the address it is written to.
     */
    DNX_TMC_DRAM_BIST_DATA_PATTERN_ADDR_MODE = 7,
    /*
     * nof data pattern modes
     */
    DNX_TMC_DRAM_BIST_NOF_DATA_PATTERN_MODES = 8
}DNX_TMC_DRAM_BIST_DATA_PATTERN_MODE;


typedef struct {
    /* Number of Write commands per cycle. Range 0-225. */
    uint32 write_weight;
    /* Number of Read commands per cycle. Range 0-225. */
    uint32 read_weight;
    /* number of actions */
    uint32 bist_num_actions;
    /* Start address */
    uint32 bist_start_address;
    /*  End address */
    uint32 bist_end_address;
    /* pattern info */
    DNX_TMC_DRAM_BIST_DATA_PATTERN_MODE pattern_mode;
    /*  patterns */
    uint32 pattern[DNX_TMC_BIST_NOF_PATTERNS];  
    /* PRGS seed */
    uint32 data_seed[DNX_TMC_BIST_NOF_SEEDS]; 
    /* flags */
    uint32 jer2_jer2_jer2_arad_bist_flags;

}DNX_TMC_DRAM_BIST_INFO;




#endif /* __DNX_TMC_API_DRAM_INCLUDED__ */
