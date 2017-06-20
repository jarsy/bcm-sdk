/* $Id: jerp_pp_kaps_xpt.h, hagayco Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __JERP_PP_KAPS_XPT_INCLUDED__
/* { */
#define __JERP_PP_KAPS_XPT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/kbp/alg_kbp/include/kbp_portable.h>
#include <soc/kbp/alg_kbp/include/xpt_kaps.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* } */

/*************
 * ENUMS     *
 *************/
/* { */

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

kbp_status jerp_kaps_translate_blk_func_offset_to_mem_reg(int unit,
                                                          uint8     blk_id,
                                                          uint32     func,
                                                          uint32     offset,
                                                          soc_mem_t *mem,
                                                          soc_reg_t *reg,
                                                          uint32    *array_index,
                                                          int       *blk);

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>


#endif /* } __JERP_PP_KAPS_XPT_INCLUDED__ */

