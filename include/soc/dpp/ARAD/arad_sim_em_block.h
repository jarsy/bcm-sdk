/* $Id: arad_sim_em_block.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifndef _ARAD_SIM_EM_BLOCK_H_
/* { */
#define _ARAD_SIM_EM_BLOCK_H_

#ifndef UINT32
  #define UINT32 unsigned int
#endif

typedef struct
{
  UINT32          read_result_address; /* for consistency with indirect - marks the end of blocks.*/
  UINT32          offset;
  UINT32          table_size;
  UINT32          key_size;
  UINT32          data_nof_bytes;
  UINT32          start_address;
  UINT32          end_address;
  SOC_SAND_MULTI_SET_PTR    multi_set;
  PARSER_HINT_ARR uint8*          base;
} CHIP_SIM_EM_BLOCK;

/* } _ARAD_SIM_EM_BLOCK_H_*/
#endif
