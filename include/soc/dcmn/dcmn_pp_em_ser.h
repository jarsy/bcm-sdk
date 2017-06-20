/* $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef __JER_PP_EM_SER_INCLUDED__
#define __JER_PP_EM_SER_INCLUDED__

/*************
 * INCLUDES  *
 *************/
#ifdef BCM_DPP_SUPPORT
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

/*************
 * TYPE DEFS *
 *************/

typedef enum
{
    JER_PP_EM_SER_BLOCK_IHB,
    JER_PP_EM_SER_BLOCK_PPDB_A,
    JER_PP_EM_SER_BLOCK_PPDB_B,
    JER_PP_EM_SER_BLOCK_EDB,
    JER_PP_EM_SER_BLOCK_OAMP,
    JER_PP_EM_SER_NOF_BLOCKS
}JER_PP_EM_SER_BLOCK;

typedef enum
{
    JER_PP_EM_SER_TYPE_LEM_KEYT_PLDT,
    JER_PP_EM_SER_TYPE_ISEM_KEYT_PLDT,
    JER_PP_EM_SER_TYPE_GLEM_KEYT_PLDT,
    JER_PP_EM_SER_TYPE_ESEM_KEYT_PLDT,
    JER_PP_EM_SER_TYPE_OEMA_KEYT_PLDT,
    JER_PP_EM_SER_TYPE_OEMB_KEYT_PLDT,
    JER_PP_EM_SER_TYPE_RMAPEM_KEYT_PLDT,
    JER_PP_EM_SER_TYPE_LAST
}JER_PP_EM_SER_TYPE;

/*************
 * FUNCTIONS *
 *************/

int dcmn_pp_em_ser(int unit, soc_mem_t mem, unsigned array_index, int copyno, int index);
#include <soc/dpp/SAND/Utils/sand_footer.h>

#endif

#endif /* __JER_PP_EM_SER_INCLUDED__ */



