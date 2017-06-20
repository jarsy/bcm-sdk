/* $Id: tmc_api_pmf_low_level_fem_tag.h,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/tmc/include/soc_tmcapi_pmf_low_level.h
*
* MODULE PREFIX:  soc_tmcpmf
*
* FILE DESCRIPTION:
*
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/

#ifndef __SOC_TMC_API_PMF_LOW_LEVEL_FEM_TAG_INCLUDED__
/* { */
#define __SOC_TMC_API_PMF_LOW_LEVEL_FEM_TAG_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/TMC/tmc_api_ports.h>

#include <soc/dpp/PPC/ppc_api_fp.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     Number of FEM selected bits.                            */
#define  SOC_TMC_PMF_FEM_NOF_SELECTED_BITS (4)

/*     Maximal number of bits in the FEM output result (action
 *     value).                                                 */
#define  SOC_TMC_PMF_FEM_MAX_OUTPUT_SIZE_IN_BITS    (24)

#define SOC_TMC_PMF_LOW_LEVEL_NOF_DATABASES                          (128)

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

typedef enum
{
  /*
   *  Key A [31:0]
   */
  SOC_TMC_PMF_FEM_INPUT_SRC_A_31_0 = 0,
  /*
   *  Key A [47:16]
   */
  SOC_TMC_PMF_FEM_INPUT_SRC_A_47_16 = 1,
  /*
   *  Key A [63:32]
   */
  SOC_TMC_PMF_FEM_INPUT_SRC_A_63_32 = 2,
  /*
   *  Key A [79:48].
   */
  SOC_TMC_PMF_FEM_INPUT_SRC_A_79_48 = 3,
  /*
   *  Key A [95:64].
   */
  SOC_TMC_PMF_FEM_INPUT_SRC_A_95_64 = 4,
  /*
   *  Key B [31:0]
   */
  SOC_TMC_PMF_FEM_INPUT_SRC_B_31_0 = 5,
  /*
   *  Key B [47:16]
   */
  SOC_TMC_PMF_FEM_INPUT_SRC_B_47_16 = 6,
  /*
   *  Key B [63:32]
   */
  SOC_TMC_PMF_FEM_INPUT_SRC_B_63_32 = 7,
  /*
   *  Key B [79:48].
   */
  SOC_TMC_PMF_FEM_INPUT_SRC_B_79_48 = 8,
  /*
   *  Key B [95:64].
   */
  SOC_TMC_PMF_FEM_INPUT_SRC_B_95_64 = 9,
  /*
   *  TCAM lookup result 0
   */
  SOC_TMC_PMF_FEM_INPUT_SRC_TCAM_0 = 10,
  /*
   *  TCAM lookup result 1
   */
  SOC_TMC_PMF_FEM_INPUT_SRC_TCAM_1 = 11,
  /*
   *  TCAM lookup result 2
   */
  SOC_TMC_PMF_FEM_INPUT_SRC_TCAM_2 = 12,
  /*
   *  TCAM lookup result 3
   */
  SOC_TMC_PMF_FEM_INPUT_SRC_TCAM_3 = 13,
  /*
   *  Vector composed of: {12 bits '0'; Direct table result}
   */
  SOC_TMC_PMF_FEM_INPUT_SRC_DIR_TBL = 14,
  /*
   *  FEM returns an invalid action
   */
  SOC_TMC_PMF_FEM_INPUT_SRC_NOP = 15,
  /*
   *  General TCAM before PD/Sel resolution
   */
   SOC_TMC_PMF_FEM_INPUT_TCAM = 16,
  /*
   *  Number of types in SOC_TMC_PMF_FEM_INPUT_SRC
   */
  SOC_TMC_NOF_PMF_FEM_INPUT_SRCS = 17
}SOC_TMC_PMF_FEM_INPUT_SRC;

typedef enum
{
  /*
   *  Statistic-Tag type. In this case, the
   *  'stat_tag_lsb_position' must be also set.
   */
  SOC_TMC_PMF_TAG_TYPE_STAT_TAG = 0,
  /*
   *  LAG Load-Balancing key.
   */
  SOC_TMC_PMF_TAG_TYPE_LAG_LB_KEY = 1,
  /*
   *  ECMP Load-Balancing key.
   */
  SOC_TMC_PMF_TAG_TYPE_ECMP_LB_KEY = 2,
  /*
   *  Stacking Route History.
   */
  SOC_TMC_PMF_TAG_TYPE_STACK_RT_HIST = 3,
  /*
   *  Number of types in SOC_TMC_PMF_TAG_TYPE
   */
  SOC_TMC_NOF_PMF_TAG_TYPES = 4
}SOC_TMC_PMF_TAG_TYPE;

typedef enum
{
  /*
   *  Key A [29:0]
   */
  SOC_TMC_PMF_TAG_VAL_SRC_A_29_0 = 0,
  /*
   *  Key A [63:32]
   */
  SOC_TMC_PMF_TAG_VAL_SRC_A_61_32 = 1,
  /*
   *  Key B [29:0]
   */
  SOC_TMC_PMF_TAG_VAL_SRC_B_29_0 = 2,
  /*
   *  Key B [61:32]
   */
  SOC_TMC_PMF_TAG_VAL_SRC_B_61_32 = 3,
  /*
   *  TCAM lookup result 0
   */
  SOC_TMC_PMF_TAG_VAL_SRC_TCAM_0 = 4,
  /*
   *  TCAM lookup result 1
   */
  SOC_TMC_PMF_TAG_VAL_SRC_TCAM_1 = 5,
  /*
   *  TCAM lookup result 2
   */
  SOC_TMC_PMF_TAG_VAL_SRC_TCAM_2 = 6,
  /*
   *  Direct table result
   */
  SOC_TMC_PMF_TAG_VAL_SRC_DIR_TBL = 7,
  /*
   *  Number of types in SOC_TMC_PMF_TAG_VAL_SRC
   */
  SOC_TMC_NOF_PMF_TAG_VAL_SRCS = 8
}SOC_TMC_PMF_TAG_VAL_SRC;

typedef enum
{
  /*
   *  Action on the Destination field
   */
  SOC_TMC_PMF_FEM_ACTION_TYPE_DEST = 0,
  /*
   *  Action on the Drop Precedence field
   */
  SOC_TMC_PMF_FEM_ACTION_TYPE_DP = 1,
  /*
   *  Action on the Traffic Class field
   */
  SOC_TMC_PMF_FEM_ACTION_TYPE_TC = 2,
  /*
   *  Action on the Forward field
   */
  SOC_TMC_PMF_FEM_ACTION_TYPE_TRAP = 3,
  /*
   *  Action on the Snoop field
   */
  SOC_TMC_PMF_FEM_ACTION_TYPE_SNP = 4,
  /*
   *  Action on the Mirror field
   */
  SOC_TMC_PMF_FEM_ACTION_TYPE_MIRROR = 5,
  /*
   *  Action on the Outbound-Mirror-Disable field
   */
  SOC_TMC_PMF_FEM_ACTION_TYPE_MIR_DIS = 6,
  /*
   *  Action on the Exclude-Source field
   */
  SOC_TMC_PMF_FEM_ACTION_TYPE_EXC_SRC = 7,
  /*
   *  Action on the Ingress Shaping field
   */
  SOC_TMC_PMF_FEM_ACTION_TYPE_IS = 8,
  /*
   *  Action on the Meter field
   */
  SOC_TMC_PMF_FEM_ACTION_TYPE_METER = 9,
  /*
   *  Action on the Counter field
   */
  SOC_TMC_PMF_FEM_ACTION_TYPE_COUNTER = 10,
  /*
   *  Action on the Statistic field
   */
  SOC_TMC_PMF_FEM_ACTION_TYPE_STAT = 11,
  /*
   *  Action on the Second Pass Data - Valid if the Cycle
   *  index is 0.
   */
  SOC_TMC_PMF_FEM_ACTION_TYPE_2ND_PASS_DATA = 12,
  /*
   *  Action on the Second Pass Program - Valid if the Cycle
   *  index is 0.
   */
  SOC_TMC_PMF_FEM_ACTION_TYPE_2ND_PASS_PGM = 13,
  /*
   *  Action on the Outlif field
   */
  SOC_TMC_PMF_FEM_ACTION_TYPE_OUTLIF = 14,
  /*
   *  No action
   */
  SOC_TMC_PMF_FEM_ACTION_TYPE_NOP = 15,
  /*
   *  Number of types in SOC_TMC_PMF_FEM_ACTION_TYPE
   */
  SOC_TMC_NOF_PMF_FEM_ACTION_TYPES = 16
}SOC_TMC_PMF_FEM_ACTION_TYPE;

typedef enum
{
  /*
   *  The FEM output bit value is constant and its value is
   *  indicated in the 'val' field
   */
  SOC_TMC_PMF_FEM_BIT_LOC_TYPE_CST = 0,
  /*
   *  The FEM output bit value equals the bit in the location
   *  'val' in the FEM-Key
   */
  SOC_TMC_PMF_FEM_BIT_LOC_TYPE_KEY = 1,
  /*
   *  The FEM output bit value equals the bit in the location
   *  'val' in the Map-Data field of the Selected-Bit of the
   *  FEM-Key
   */
  SOC_TMC_PMF_FEM_BIT_LOC_TYPE_MAP_DATA = 2,
  /*
   *  Number of types in SOC_TMC_PMF_FEM_BIT_LOC_TYPE
   */
  SOC_TMC_NOF_PMF_FEM_BIT_LOC_TYPES = 3
}SOC_TMC_PMF_FEM_BIT_LOC_TYPE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /* Is invalid source - NOP*/
  uint8 is_nop;

  /*
   *  If Key: A..D, LSB 0/16/../112/128
   *  Otherwise, tcam result index and lookup cycle
   */
  uint8 is_key_src;
  /*
   *  Source-Key / TCAM result
   *  Range: A..D / 0..3
   */
  uint32 key_tcam_id;
  /*
   *  Source-Key-LSB
   *  LSB 0/16/../112/128
   *  For Tcam: 0 - 32b LSB
   *  1 - 32b MSB
   */
  uint32 key_lsb;
  /*
   *  Lookup Cycle: 0 / 1
   */
  uint32 lookup_cycle_id;

  /*
   *  use_kaps : TRUE/FALSE
   */
  uint8 use_kaps;

  /*
   *  For Jericho_Plus above, indicates
   *  that a compare FEM/FES should be used.
   */
  uint8 is_compare;


} SOC_TMC_PMF_FEM_INPUT_SRC_ARAD;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  FEM-Program-Id: The FEM Program ID typically defines the
   *  PCL role, along with the PCL result (e.g.: Filtering /
   *  metering / counting / trapping / Policy based
   *  routing...). Range: 0 - 3. (Soc_petra-B)
   */
  uint8 pgm_id;
  /*
   *  FEM-Input Source. The FEM-Input data can be taken from
   *  the direct table result, directly from the PMF input
   *  vector, or from one of the 4 TCAM lookup results
   *  Soc_petraB only
   */
  SOC_TMC_PMF_FEM_INPUT_SRC src;
  /*
   *  TCAM Database-ID to collect the result from. Valid only
   *  if this Database-ID is valid for this Lookup-Profile
   *  (set via the soc_ppd_pmf_tcam_lookup_set API). Range: 0 -
   *  16K-1.
   */
  uint32 db_id;

  /* ARAD only */
  /*
   *  FEM-Input Source.
   *  If Key: A..D, LSB 0/16/../112/128
   *  Otherwise, tcam result index and lookup cycle
   */
  SOC_TMC_PMF_FEM_INPUT_SRC_ARAD src_arad;
  /*
   *  Is 16 LSB are taken from another place
   */
  uint8 is_16_lsb_overridden;
  /* ARAD only */
  /*
   *  If so, its location
   */
  SOC_TMC_PMF_FEM_INPUT_SRC_ARAD lsb16_src;

} SOC_TMC_PMF_FEM_INPUT_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Shift: Action-Value will be taken according to the shift of the input
   *  Range: 0 - 31.
   */
  uint32 shift;
  /*
   *  Shift: Action-Value will be taken according to the shift of the input
   *  Range: 0 - 31.
   */
  SOC_PPC_FP_ACTION_TYPE action_type;
  /*
   *  If True, then the action is always done (Direct Extraction)
   *  Otherwise, the LSB of the input indicates whether to do it (TCAM)
   */
  uint8 is_action_always_valid;

  /*
   * Amount of valid bits for this FES
   */
  uint32 valid_bits;

} SOC_TMC_PMF_FES_INPUT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Tag value source to get the Tag value from.
   */
  SOC_TMC_PMF_TAG_VAL_SRC val_src;
  /*
   *  Bit position in the Tag value of the lsb of the final
   *  Statistic-Tag. Valid only if the Tag-Type is
   *  Statistic-Tag.
   */
  uint32 stat_tag_lsb_position;

} SOC_TMC_PMF_TAG_SRC_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  FEM-Id. In Soc_petra-B, the FEM ids 0-1 have an output of 4
   *  bits, the ids 2-4 of 14 bits, the ids 5-7of 17
   *  bits. Range: 0 - 7. (Soc_petra-B). Range: 0 - 15. (Arad)
   */
  uint32 id;
  /*
   *  Cycle index in which the access occurs. Range: 0 - 1.
   *  (Soc_petra-B only)
   */
  uint32 cycle_ndx;

} SOC_TMC_PMF_FEM_NDX;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Set the location of the MSB of the consecutive selected
   *  bits. Range: 3 - 31. (Soc_petra-B)
   */
  uint32 sel_bit_msb;

} SOC_TMC_PMF_FEM_SELECTED_BITS_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Action-format Id: One the four actions set by this FEM.
   *  Range: 0 - 3. (Soc_petra-B)
   */
  uint32 action_fomat_id;
  /*
   *  One of the 3 source options during the Field Extraction
   *  (with extracting from the FEM-Key or from a constant
   *  value). Is set per Selected-Bits value (4b) and is not a
   *  segment of the FEM-Key. Range: 0 - 15. (Soc_petra-B)
   */
  uint32 map_data;
  /*
   *  Set if Action type is valid. Arad-only
   */
  uint8 is_action_valid;

} SOC_TMC_PMF_FEM_ACTION_FORMAT_MAP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Bit location type.
   */
  SOC_TMC_PMF_FEM_BIT_LOC_TYPE type;
  /*
   *  Value of the bit location / bit value according to the
   *  'type'. If the type is 'CST', this value indicates the
   *  constant value of the bit (0 or 1). If the type is
   *  'KEY', then the bit value is taken from the bit location
   *  indicated by this field. If the type is 'MAP_DATA', then
   *  the bit value is taken from the 'map_data' value set per
   *  Selected-bits value in the
   *  soc_ppd_pmf_fem_action_format_map_set() API.
   */
  uint32 val;

} SOC_TMC_PMF_FEM_BIT_LOC;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Action type of the FEM.
   */
  uint32 type; /*SOC_TMC_PMF_FEM_ACTION_TYPE in Soc_petra-B, SOC_PPC_FP_ACTION_TYPE in Arad */
  /*
   *  Set the size of the Field-bit-location parameter. The
   *  coherency between the maximum FEM-ID value width (4, 14,
   *  or 17 bits) and the size is checked. Range: 0 - 17.
   *  (Soc_petra-B)
   */
  uint32 size;
  /*
   *  Set the bit location of each bit of the FEM output
   *  result (action value) up to the 'size' bit.
   */
  SOC_TMC_PMF_FEM_BIT_LOC bit_loc[SOC_TMC_PMF_FEM_MAX_OUTPUT_SIZE_IN_BITS];
  /*
   *  Add a constant base-value (offset) to the FEM action
   *  value.
   */
  uint32 base_value;

} SOC_TMC_PMF_FEM_ACTION_FORMAT_INFO;

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
void
  SOC_TMC_PMF_FEM_INPUT_SRC_ARAD_clear(
    SOC_SAND_OUT SOC_TMC_PMF_FEM_INPUT_SRC_ARAD *info
  );

void
  SOC_TMC_PMF_FEM_INPUT_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PMF_FEM_INPUT_INFO *info
  );

void
  SOC_TMC_PMF_FES_INPUT_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PMF_FES_INPUT_INFO *info
  );


void
  SOC_TMC_PMF_TAG_SRC_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PMF_TAG_SRC_INFO *info
  );

void
  SOC_TMC_PMF_FEM_NDX_clear(
    SOC_SAND_OUT SOC_TMC_PMF_FEM_NDX *info
  );

void
  SOC_TMC_PMF_FEM_SELECTED_BITS_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PMF_FEM_SELECTED_BITS_INFO *info
  );

void
  SOC_TMC_PMF_FEM_ACTION_FORMAT_MAP_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PMF_FEM_ACTION_FORMAT_MAP_INFO *info
  );

void
  SOC_TMC_PMF_FEM_BIT_LOC_clear(
    SOC_SAND_OUT SOC_TMC_PMF_FEM_BIT_LOC *info
  );

void
  SOC_TMC_PMF_FEM_ACTION_FORMAT_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PMF_FEM_ACTION_FORMAT_INFO *info
  );

#if SOC_TMC_DEBUG_IS_LVL1

const char*
  SOC_TMC_PMF_FEM_INPUT_SRC_to_string(
    SOC_SAND_IN  SOC_TMC_PMF_FEM_INPUT_SRC enum_val
  );

const char*
  SOC_TMC_PMF_TAG_TYPE_to_string(
    SOC_SAND_IN  SOC_TMC_PMF_TAG_TYPE enum_val
  );

const char*
  SOC_TMC_PMF_TAG_VAL_SRC_to_string(
    SOC_SAND_IN  SOC_TMC_PMF_TAG_VAL_SRC enum_val
  );

const char*
  SOC_TMC_PMF_FEM_ACTION_TYPE_to_string(
    SOC_SAND_IN  SOC_TMC_PMF_FEM_ACTION_TYPE enum_val
  );

const char*
  SOC_TMC_PMF_FEM_BIT_LOC_TYPE_to_string(
    SOC_SAND_IN  SOC_TMC_PMF_FEM_BIT_LOC_TYPE enum_val
  );

void
  SOC_TMC_PMF_FEM_INPUT_INFO_print(
    SOC_SAND_IN  SOC_TMC_PMF_FEM_INPUT_INFO *info
  );

void
  SOC_TMC_PMF_FEM_INPUT_SRC_ARAD_print(
    SOC_SAND_IN  SOC_TMC_PMF_FEM_INPUT_SRC_ARAD *info
  );

void
  SOC_TMC_PMF_TAG_SRC_INFO_print(
    SOC_SAND_IN  SOC_TMC_PMF_TAG_SRC_INFO *info
  );

void
  SOC_TMC_PMF_FEM_NDX_print(
    SOC_SAND_IN  SOC_TMC_PMF_FEM_NDX *info
  );

void
  SOC_TMC_PMF_FEM_SELECTED_BITS_INFO_print(
    SOC_SAND_IN  SOC_TMC_PMF_FEM_SELECTED_BITS_INFO *info
  );

void
  SOC_TMC_PMF_FEM_ACTION_FORMAT_MAP_INFO_print(
    SOC_SAND_IN  SOC_TMC_PMF_FEM_ACTION_FORMAT_MAP_INFO *info
  );

void
  SOC_TMC_PMF_FEM_BIT_LOC_print(
    SOC_SAND_IN  SOC_TMC_PMF_FEM_BIT_LOC *info
  );

void
  SOC_TMC_PMF_FEM_ACTION_FORMAT_INFO_print(
    SOC_SAND_IN  SOC_TMC_PMF_FEM_ACTION_FORMAT_INFO *info
  );

#endif /* SOC_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_API_PMF_LOW_LEVEL_FEM_TAG_INCLUDED__*/
#endif

