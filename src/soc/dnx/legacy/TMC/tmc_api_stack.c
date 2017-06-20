/* $Id: jer2_tmc_api_stack.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

/*************
 * INCLUDES  *
 *************/
/* { */



#include <shared/bsl.h>

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>

#include <soc/dnx/legacy/TMC/tmc_api_stack.h>

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

void
  DNX_TMC_STACK_GLBL_INFO_clear(
    DNX_SAND_OUT DNX_TMC_STACK_GLBL_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_STACK_GLBL_INFO));
  info->max_nof_tm_domains = DNX_TMC_STACK_MAX_NOF_TM_DOMAINS_1;
  info->my_tm_domain = 0;
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_STACK_PORT_DISTR_INFO_clear(
    DNX_SAND_OUT DNX_TMC_STACK_PORT_DISTR_INFO *info
  )
{
  uint32
    ind;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(DNX_TMC_STACK_PORT_DISTR_INFO));
  info->peer_tm_domain = 0;
  for (ind = 0; ind < DNX_TMC_STACK_PRUN_BMP_LEN; ++ind)
  {
    info->prun_bmp[ind] = 0;
  }
  DNX_SAND_MAGIC_NUM_SET;
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_STACK_MAX_NOF_TM_DOMAINS_to_string(
    DNX_SAND_IN  DNX_TMC_STACK_MAX_NOF_TM_DOMAINS enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case DNX_TMC_STACK_MAX_NOF_TM_DOMAINS_1:
    str = "1";
  break;
  case DNX_TMC_STACK_MAX_NOF_TM_DOMAINS_8:
    str = "8";
  break;
  case DNX_TMC_STACK_MAX_NOF_TM_DOMAINS_16:
    str = "16";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

void
  DNX_TMC_STACK_GLBL_INFO_print(
    DNX_SAND_IN  DNX_TMC_STACK_GLBL_INFO *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "max_nof_tm_domains %s "), DNX_TMC_STACK_MAX_NOF_TM_DOMAINS_to_string(info->max_nof_tm_domains)));
  LOG_CLI((BSL_META_U(unit,
                      "my_tm_domain: %u\n\r"),info->my_tm_domain));
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  DNX_TMC_STACK_PORT_DISTR_INFO_print(
    DNX_SAND_IN  DNX_TMC_STACK_PORT_DISTR_INFO *info
  )
{
  uint32
    ind;

  DNX_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  DNX_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "peer_tm_domain: %u\n\r"),info->peer_tm_domain));
  for (ind = 0; ind < DNX_TMC_STACK_PRUN_BMP_LEN; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "prun_bmp[%u]: %u\n\r"),ind,info->prun_bmp[ind]));
  }
exit:
  DNX_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

