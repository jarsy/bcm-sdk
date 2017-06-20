/* $Id: jer2_tmc_api_flow_control.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

/*************
 * INCLUDES  *
 *************/
/* { */


#include <shared/bsl.h>

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>

#include <soc/dnx/legacy/TMC/tmc_api_flow_control.h>
#include <soc/dnx/legacy/TMC/tmc_api_framework.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

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

void DNX_TMC_FC_PFC_GENERIC_BITMAP_clear(DNX_SAND_OUT DNX_TMC_FC_PFC_GENERIC_BITMAP *generic_bm)
{
  uint32
    i;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(generic_bm);

  sal_memset(generic_bm, 0x0, sizeof(DNX_TMC_FC_PFC_GENERIC_BITMAP));

  for(i = 0; i<DNX_TMC_FC_PFC_GENERIC_BITMAP_SIZE / 32; i++)
  {
    generic_bm->bitmap[i] = 0;
  }

exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void DNX_TMC_FC_CAL_IF_INFO_clear(DNX_SAND_OUT DNX_TMC_FC_CAL_IF_INFO *cal_info)
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(cal_info);

  sal_memset(cal_info, 0x0, sizeof(DNX_TMC_FC_CAL_IF_INFO));

  cal_info->enable = 0;
  cal_info->cal_len = 0;
  cal_info->cal_reps = 0;


exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_FC_GEN_INBND_CB_clear(
    DNX_SAND_OUT DNX_TMC_FC_GEN_INBND_CB *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_FC_GEN_INBND_CB));
  info->inherit = DNX_TMC_FC_NOF_INBND_CB_INHERITS;
  info->glbl_rcs_low = 0;
  info->cnm_intercept_enable = 0;
  info->nif_cls_bitmap = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}


void
  DNX_TMC_FC_GEN_INBND_LL_clear(
    DNX_SAND_OUT DNX_TMC_FC_GEN_INBND_LL *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_FC_GEN_INBND_LL));
  info->cnm_enable = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_FC_REC_INBND_CB_clear(
    DNX_SAND_OUT DNX_TMC_FC_REC_INBND_CB *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_FC_REC_INBND_CB));
  info->inherit = DNX_TMC_FC_NOF_INBND_CB_INHERITS;
  info->sch_hr_bitmap = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_FC_GEN_INBND_INFO_clear(
    DNX_SAND_OUT DNX_TMC_FC_GEN_INBND_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_FC_GEN_INBND_INFO));
  info->mode = DNX_TMC_FC_NOF_INBND_MODES;
  DNX_TMC_FC_GEN_INBND_CB_clear(&(info->cb));
  DNX_TMC_FC_GEN_INBND_LL_clear(&(info->ll));
  DNX_TMC_FC_GEN_INBND_PFC_clear(&(info->pfc));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_FC_GEN_INBND_PFC_clear(
    DNX_SAND_OUT DNX_TMC_FC_GEN_INBND_PFC *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  dnx_sand_os_memset(info, 0x0, sizeof(DNX_TMC_FC_GEN_INBND_PFC));
  info->inherit = DNX_TMC_FC_NOF_INBND_PFC_INHERITS;
  info->glbl_rcs_low = 0;
  info->cnm_intercept_enable = 0;
  info->nif_cls_bitmap = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_FC_REC_CALENDAR_clear(
    DNX_SAND_OUT DNX_TMC_FC_REC_CALENDAR *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_FC_REC_CALENDAR));
  info->destination = DNX_TMC_FC_NOF_REC_CAL_DESTS;
  info->id = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_FC_GEN_CALENDAR_clear(
    DNX_SAND_OUT DNX_TMC_FC_GEN_CALENDAR *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_FC_GEN_CALENDAR));
  info->source = DNX_TMC_FC_NOF_GEN_CAL_SRCS;
  info->id = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_FC_REC_INBND_INFO_clear(
    DNX_SAND_OUT DNX_TMC_FC_REC_INBND_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_FC_REC_INBND_INFO));
  info->mode = DNX_TMC_FC_NOF_INBND_MODES;
  DNX_TMC_FC_REC_INBND_CB_clear(&(info->cb));
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_FC_REC_OFP_MAP_INFO_clear(
    DNX_SAND_OUT DNX_TMC_FC_REC_OFP_MAP_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_FC_REC_OFP_MAP_INFO));
  info->react_point = DNX_TMC_FC_NOF_REC_OFP_RPS;
  info->ofp_ndx = 0;
  info->priority = DNX_TMC_FC_NOF_OFP_PRIORITYS;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_FC_ILKN_LLFC_INFO_clear(
    DNX_SAND_OUT DNX_TMC_FC_ILKN_LLFC_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_FC_ILKN_LLFC_INFO));
  info->multi_use_mask = 0;
  info->cal_channel = DNX_TMC_FC_NOF_ILKN_CAL_LLFCS;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>
