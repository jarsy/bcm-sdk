/*! \file bcm_int/dnx/switch_tpid/switch_tpid.h
 * 
 * Internal DNX SWITCH-TPID APIs 
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef SWITCH_TPID_H_INCLUDED
/* { */
#  define SWITCH_TPID_H_INCLUDED

#  ifndef BCM_DNX_SUPPORT
#    error "This file is for use by DNX (JR2) family only!"
#  endif

/** TPID value 0x0000 is not valid:
 *  See the Programming Guide PP document for full details */
#  define BCM_DNX_SWITCH_TPID_VALUE_INVALID   (0x0000)

/** TPID invalid index: Table index is [0..7] */
#  define BCM_DNX_SWITCH_TPID_INDEX_INVALID   (-1)

/** There are 8 globals TPIDs:
 *  See the Programming Guide PP document for full details   */
#  define BCM_DNX_SWITCH_TPID_NUM_OF_GLOBALS  (8)

/** There are 7 valid globals TPIDs:
 *  See the Programming Guide PP document for full details   */
#  define BCM_DNX_SWITCH_TPID_NUM_OF_VALID_GLOBALS  (7)

/** TPID7 is reserved:
 *  See the Programming Guide PP document for full details   */
#  define BCM_DNX_SWITCH_TPID_RESERVED_ENTRY  (7)


#  define DBAL_TABLE_SWITCH_TPID    DBAL_TABLE_SW_STATE_TABLE_SWITCH_TPID_TEMP


shr_error_e
dnx_switch_tpid_value_get(
   int unit,
   int tpid_index,
   uint16 *tpid_value);

shr_error_e
dnx_switch_tpid_value_set(
   int unit,
   int tpid_index,
   uint16 tpid_value);

shr_error_e
dnx_switch_tpid_index_get(
   int unit,
   uint16 tpid_value,
   int *tpid_index);


/* } */
#endif/*SWITCH_TPID_H_INCLUDED*/
