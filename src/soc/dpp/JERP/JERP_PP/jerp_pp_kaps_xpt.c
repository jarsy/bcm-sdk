/* $Id: jerp_pp_kaps_xpt.c, hagayco Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/

#include <soc/mem.h>

#if defined(BCM_88680_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FORWARD
#include <soc/mem.h>

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <shared/bsl.h>
#include <soc/dcmn/error.h>
#include <soc/dcmn/dcmn_dev_feature_manager.h>
#include <soc/dpp/ARAD/arad_general.h>
#include <soc/dpp/ARAD/arad_kbp.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_sw_db.h>

#include <soc/dpp/JER/JER_PP/jer_pp_kaps.h>
#include <soc/dpp/JER/JER_PP/jer_pp_kaps_xpt.h>

#include <soc/dpp/JER/JER_PP/jer_pp_kaps_entry_mgmt.h>

#include <soc/dpp/SAND/Management/sand_low_level.h>
#include <soc/dpp/QAX/QAX_PP/qax_pp_kaps_xpt.h>

#include <soc/dpp/JER/jer_sbusdma_desc.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

#define JERP_RPB_FIRST_BLK_ID  1
#define JERP_RPB_LAST_BLK_ID   4
#define JERP_BB_FIRST_BLK_ID   5
#define JERP_BB_LAST_BLK_ID    12
#define JERP_BBS_FIRST_BLK_ID  13
#define JERP_BBS_LAST_BLK_ID   36

#define JERP_NUMELS_BBS        8

#define JERP_JER_MODE_MAX_IDX  1024

/* } */

/*************
 *  MACROS   *
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
                                                          int       *blk)
{
    uint32 rv = KBP_OK;

    *mem = INVALIDm;
    *reg = INVALIDr;
    *array_index = 0;

    if (blk_id >= JERP_BBS_FIRST_BLK_ID && blk_id <= JERP_BBS_LAST_BLK_ID) {
        *array_index = (blk_id - JERP_BBS_FIRST_BLK_ID) % JERP_NUMELS_BBS;
        *blk = KAPS_BBS_BLOCK(unit, ((blk_id - JERP_BBS_FIRST_BLK_ID) / JERP_NUMELS_BBS));

        switch (func)
        {
           case KAPS_FUNC0:
               if (offset == KAPS_BB_GLOBAL_CONFIG_OFFSET) {
                   *reg = KAPS_BBS_BB_GLOBAL_CONFIGr;
                   *blk = ((blk_id - JERP_BBS_FIRST_BLK_ID) / JERP_NUMELS_BBS) | SOC_REG_ADDR_INSTANCE_MASK;
               } else{
                   LOG_CLI((BSL_META_U(0, "%s():  unsupported BB register offset: %d\n"),
                                    FUNCTION_NAME(), offset));
                   rv = KBP_FATAL_TRANSPORT_ERROR;
               }
               break;

            case KAPS_FUNC1:
               *mem = KAPS_BUCKET_MAP_MEMORYm;
               LOG_CLI((BSL_META_U(0, "%s():  BB, unsupported func: %d. KAPS_BUCKET_MAP_MEMORYm is not  supported in Jericho Plus!\n"),
                                    FUNCTION_NAME(), func));
               rv = KBP_FATAL_TRANSPORT_ERROR;
               break;
            case KAPS_FUNC2:
            case KAPS_FUNC5:
               *mem = KAPS_BBS_BUCKET_MEMORYm;
               break;

            default:
               LOG_CLI((BSL_META_U(0, "%s():  BB, unsupported func: %d\n"),
                                    FUNCTION_NAME(), func));
               rv = KBP_FATAL_TRANSPORT_ERROR;
               break;
        }
    } else if (blk_id >= JERP_BB_FIRST_BLK_ID && blk_id <= JERP_BB_LAST_BLK_ID) {
        *array_index = blk_id - JERP_BB_FIRST_BLK_ID;
        *blk = KAPS_BLOCK(unit);
        switch (func)
        {
           case KAPS_FUNC0:
               if (offset == KAPS_BB_GLOBAL_CONFIG_OFFSET) {
                   *reg = KAPS_BB_GLOBAL_CONFIGr;
               } else{
                   LOG_CLI((BSL_META_U(0, "%s():  unsupported BB register offset: %d\n"),
                                    FUNCTION_NAME(), offset));
                   rv = KBP_FATAL_TRANSPORT_ERROR;
               }
               break;

            case KAPS_FUNC1:
               *mem = KAPS_BUCKET_MAP_MEMORYm;
               LOG_CLI((BSL_META_U(0, "%s():  BB, unsupported func: %d. KAPS_BUCKET_MAP_MEMORYm is not  supported in Jericho Plus!\n"),
                                    FUNCTION_NAME(), func));
               rv = KBP_FATAL_TRANSPORT_ERROR;
               break;
            case KAPS_FUNC2:
            case KAPS_FUNC5:
               *mem = KAPS_BUCKET_MEMORYm;
               break;

            default:
               LOG_CLI((BSL_META_U(0, "%s():  BB, unsupported func: %d\n"),
                                    FUNCTION_NAME(), func));
               rv = KBP_FATAL_TRANSPORT_ERROR;
               break;
        }
    } else if (blk_id >= JERP_RPB_FIRST_BLK_ID && blk_id <= JERP_RPB_LAST_BLK_ID) {
        *array_index = blk_id - JERP_RPB_FIRST_BLK_ID;
        *blk = KAPS_BLOCK(unit);
        switch (func)
        {
           case KAPS_FUNC0:
               if (offset == KAPS_RPB_CAM_BIST_CONTROL_OFFSET) {
                   *reg = KAPS_RPB_CAM_BIST_CONTROLr;
               } else if (offset == KAPS_RPB_CAM_BIST_STATUS_OFFSET) {
                   *reg = KAPS_RPB_CAM_BIST_STATUSr;
               } else if (offset == KAPS_RPB_GLOBAL_CONFIG_OFFSET) {
                   *reg = KAPS_RPB_GLOBAL_CONFIGr;
               } else{
                   LOG_CLI((BSL_META_U(0, "%s():  unsupported RPB register offset: %d\n"),
                                    FUNCTION_NAME(), offset));
                   rv = KBP_FATAL_TRANSPORT_ERROR;
               }
               break;

            case KAPS_FUNC1:
               *mem = KAPS_RPB_TCAM_CPU_COMMANDm;
               break;

            case KAPS_FUNC4:
                *mem = KAPS_RPB_ADSm;
                break;

            default:
               LOG_CLI((BSL_META_U(0, "%s():  RPB, unsupported func: %d\n"),
                                    FUNCTION_NAME(), func));
               rv = KBP_FATAL_TRANSPORT_ERROR;
               break;
        }


    } else {
        LOG_CLI((BSL_META_U(0,"%s(), unrecognized blk_id = %d.\n"),FUNCTION_NAME(), blk_id));
        return KBP_FATAL_TRANSPORT_ERROR;
    }

    return rv;
}

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

#endif /* defined(BCM_88680_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030) */
