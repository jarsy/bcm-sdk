#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)

/* $Id: arad_pp_ce_instruction.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_COMMON

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dcmn/error.h>

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>
#include <soc/dpp/SAND/Utils/sand_bitstream.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_ce_instruction.h>
#include <soc/dpp/ARAD/arad_general.h>


/* } */
/*************
 * DEFINES   *
 *************/
/* { */



#define ARAD_PP_CE_16_BIT_FIELD_INST_SIZE_BITS_LSB        (12)
#define ARAD_PP_CE_32_BIT_FIELD_INST_SIZE_BITS_LSB        (11)
#define ARAD_PP_CE_16_BIT_FIELD_INST_SHIFT_BITS_LSB       (4)
#define ARAD_PP_CE_32_BIT_FIELD_INST_SHIFT_BITS_LSB       (4)
#define ARAD_PP_CE_16_BIT_FIELD_INST_INPUT_SEL_BIT_LSB    (3)
#define ARAD_PP_CE_32_BIT_FIELD_INST_INPUT_SEL_BIT_LSB    (3)
#define ARAD_PP_CE_16_BIT_FIELD_INST_OFFSET_SEL_SIZE_LSB  (0)
#define ARAD_PP_CE_32_BIT_FIELD_INST_OFFSET_SEL_BITS_LSB  (0)

#define ARAD_PP_CE_16_BIT_FIELD_INST_SIZE_BITS_MSB        (15)
#define ARAD_PP_CE_32_BIT_FIELD_INST_SIZE_BITS_MSB        (15)
#define ARAD_PP_CE_16_BIT_FIELD_INST_SHIFT_BITS_MSB       (11)
#define ARAD_PP_CE_32_BIT_FIELD_INST_SHIFT_BITS_MSB       (10)
#define ARAD_PP_CE_16_BIT_FIELD_INST_INPUT_SEL_BIT_MSB    (3)
#define ARAD_PP_CE_32_BIT_FIELD_INST_INPUT_SEL_BIT_MSB    (3)
#define ARAD_PP_CE_16_BIT_FIELD_INST_OFFSET_SEL_SIZE_MSB  (2)
#define ARAD_PP_CE_32_BIT_FIELD_INST_OFFSET_SEL_BITS_MSB  (2)

/* get field defines } */

void 
  arad_pp_ce_instruction_composer_checks (
    uint32 field_size_in_bits,
    uint32 header_index,
    uint32 offest_from_header_start_in_bits,
    uint32 ce_16_or_32)
{
  /* checks*/
  if (header_index > 7)
  {
  	/*ARAD_PP_INIT_PRINT_ADVANCE("header_index 0..7, header_index: %d", header_index);*/
  }

  if (ce_16_or_32 == ARAD_PP_CE_IS_CE16)
  {
  	if (field_size_in_bits > 16)
    {
  	  /*ARAD_PP_INIT_PRINT_ADVANCE("ARAD_PP_CE16 support field size 0..16, requested field size: %d", field_size_in_bits);*/
    }
  	if ((offest_from_header_start_in_bits % 4) != 0)
    {
      /*ARAD_PP_INIT_PRINT_ADVANCE("ARAD_PP_CE16 support offest_from_header_start_in_bits in nibble resolution, requested offest_from_header_start_in_bits: %d", offest_from_header_start_in_bits);*/
    }
  }
  if (ce_16_or_32 == ARAD_PP_CE_IS_CE32)
  {
  	if (field_size_in_bits > 32)
    {
  	  /*ARAD_PP_INIT_PRINT_ADVANCE("ARAD_PP_CE32 support field size 0..32, requested field size: %d", field_size_in_bits);*/
    }
  	if ((offest_from_header_start_in_bits % 8) != 0)
    {
      /*ARAD_PP_INIT_PRINT_ADVANCE("ARAD_PP_CE16 support offest_from_header_start_in_bits in byte resolution, requested offest_from_header_start_in_bits: %d", offest_from_header_start_in_bits);  	  */
    }
  }
}

/*
 * Function description 
 * the copy engine work as following: 
 * the actual offset = the copy engine size + offest_from_header_start_in_bits 
 * the field size = number of bits from the offset 
 *  
 * for example 
 *  
 * if we desire to copy 20 LSB bits from the DA we will use the following: 
 *  copy engine of 32 bit. 
 *  field_size_in_bits = 20
 *  offest_from_header_start_in_bits = 15
 *  
 *  so the actual offset will be: 15 + 32 = 47.
 *  the field size will be: 20
 *  
 *  
 *  0               DA                 47               SA
 *  |----------------------------------|----------------------------------|
 *                  |   copied part    | 
 *                  15                 0
*/
uint32 
   arad_pp_ce_instruction_composer (
     uint32 field_size_in_bits,
	   uint32 header_index,
	   uint32 offest_from_header_start_in_bits,
	   uint32 ce_16_or_32
	 )
{
  uint32 inst16=0;
  uint32 inst32=0;
  uint32 tmp=0;
  arad_pp_ce_instruction_composer_checks(field_size_in_bits,header_index,offest_from_header_start_in_bits,ce_16_or_32);
  
  if (ce_16_or_32 == ARAD_PP_CE_IS_CE16)
  {
    tmp = field_size_in_bits-1;
    SHR_BITCOPY_RANGE(&inst16, ARAD_PP_CE_16_BIT_FIELD_INST_SIZE_BITS_LSB, &tmp, 0, (ARAD_PP_CE_16_BIT_FIELD_INST_SIZE_BITS_MSB-ARAD_PP_CE_16_BIT_FIELD_INST_SIZE_BITS_LSB+1));
    tmp = offest_from_header_start_in_bits /4;
    SHR_BITCOPY_RANGE(&inst16, ARAD_PP_CE_16_BIT_FIELD_INST_SHIFT_BITS_LSB, &tmp, 0, (ARAD_PP_CE_16_BIT_FIELD_INST_SHIFT_BITS_MSB-ARAD_PP_CE_16_BIT_FIELD_INST_SHIFT_BITS_LSB+1));
    tmp = 0;
    SHR_BITCOPY_RANGE(&inst16, ARAD_PP_CE_16_BIT_FIELD_INST_INPUT_SEL_BIT_LSB, &tmp, 0, (ARAD_PP_CE_16_BIT_FIELD_INST_INPUT_SEL_BIT_MSB-ARAD_PP_CE_16_BIT_FIELD_INST_INPUT_SEL_BIT_LSB+1));
    tmp = header_index;
    SHR_BITCOPY_RANGE(&inst16, ARAD_PP_CE_16_BIT_FIELD_INST_OFFSET_SEL_SIZE_LSB, &tmp, 0, (ARAD_PP_CE_16_BIT_FIELD_INST_OFFSET_SEL_SIZE_MSB-ARAD_PP_CE_16_BIT_FIELD_INST_OFFSET_SEL_SIZE_LSB+1));
    tmp = inst16;
  }
  else
  {
    tmp = field_size_in_bits-1;
    SHR_BITCOPY_RANGE(&inst32, ARAD_PP_CE_32_BIT_FIELD_INST_SIZE_BITS_LSB, &tmp, 0, (ARAD_PP_CE_32_BIT_FIELD_INST_SIZE_BITS_MSB-ARAD_PP_CE_32_BIT_FIELD_INST_SIZE_BITS_LSB+1));
    tmp = offest_from_header_start_in_bits /8;
    SHR_BITCOPY_RANGE(&inst32, ARAD_PP_CE_32_BIT_FIELD_INST_SHIFT_BITS_LSB, &tmp, 0, (ARAD_PP_CE_32_BIT_FIELD_INST_SHIFT_BITS_MSB-ARAD_PP_CE_32_BIT_FIELD_INST_SHIFT_BITS_LSB+1));
    tmp = 0;
    SHR_BITCOPY_RANGE(&inst32, ARAD_PP_CE_32_BIT_FIELD_INST_INPUT_SEL_BIT_LSB, &tmp, 0, (ARAD_PP_CE_32_BIT_FIELD_INST_INPUT_SEL_BIT_MSB-ARAD_PP_CE_32_BIT_FIELD_INST_INPUT_SEL_BIT_LSB+1));
    tmp = header_index;
    SHR_BITCOPY_RANGE(&inst32, ARAD_PP_CE_32_BIT_FIELD_INST_OFFSET_SEL_BITS_LSB, &tmp, 0, (ARAD_PP_CE_32_BIT_FIELD_INST_OFFSET_SEL_BITS_MSB-ARAD_PP_CE_32_BIT_FIELD_INST_OFFSET_SEL_BITS_LSB+1));
    tmp = inst32;
  }
  
  return tmp;
}


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88650_A0) */

