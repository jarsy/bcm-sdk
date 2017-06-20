#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_multicast_fabric.c,v 1.20 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_MULTICAST
/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/drv.h>

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_multicast_fabric.h>
#include <soc/dnx/legacy/ARAD/arad_reg_access.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_general.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_chip_tbls.h>
#include <soc/dnx/legacy/ARAD/arad_mgmt.h>
#include <soc/dnx/legacy/port_sw_db.h>


/* } */

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

/*********************************************************************
*     Configure the Enhanced Fabric Multicast Queue
*     configuration: the fabric multicast queues are defined
*     in a configured range, and the credits are coming to
*     these queues according to a scheduler scheme.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_jer_mult_fabric_enhanced_set(
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_IN  int                                 core_id,
    DNX_SAND_IN  DNX_SAND_U32_RANGE                  *queue_range
  )
{
  int
    res;
  uint64
    reg_value;
  uint32 reg32;
  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(queue_range);
  COMPILER_64_SET(reg_value, 0, 0);
  if (core_id != SOC_CORE_ALL && (core_id < 0 || core_id >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores)) {
      LOG_ERROR(BSL_LS_SOC_STAT,(BSL_META_U(unit, "Core %d out of range\n"), core_id));
      DNXC_SAND_IF_ERR_EXIT(SOC_E_PARAM);
  }
  if (queue_range->start > (SOC_DNX_DEFS_GET(unit, nof_queues) - 1)) {
    LOG_ERROR(BSL_LS_SOC_STAT,(BSL_META_U(unit, "Queue start %d out of range\n"), queue_range->start));
    DNXC_SAND_IF_ERR_EXIT(SOC_E_PARAM);
  }
  if (queue_range->end > (SOC_DNX_DEFS_GET(unit, nof_queues) - 1)) {
    LOG_ERROR(BSL_LS_SOC_STAT,(BSL_META_U(unit, "Queue end %d out of range\n"), queue_range->end));
    DNXC_SAND_IF_ERR_EXIT(SOC_E_PARAM);
  }

  if (queue_range->start > queue_range->end) {
      LOG_ERROR(BSL_LS_SOC_STAT,(BSL_META_U(unit, "Queue start %d is higher than Queue end %d\n"), queue_range->start, queue_range->end));
      DNXC_SAND_IF_ERR_EXIT(SOC_E_PARAM);
  }

  if (SOC_IS_QAX(unit)) { /* no cores */
          res = READ_IPS_FMC_QNUM_RANGEr(unit, &reg32);
          DNXC_IF_ERR_EXIT(res);

          soc_reg_field_set(unit, IPS_FMC_QNUM_RANGEr, &reg32, FMC_QNUM_LOWf, queue_range->start);
          soc_reg_field_set(unit, IPS_FMC_QNUM_RANGEr, &reg32, FMC_QNUM_HIGHf, queue_range->end);

          res = WRITE_IPS_FMC_QNUM_RANGEr(unit, reg32);
          DNXC_IF_ERR_EXIT(res);
  } else {

      if (core_id == 0 || core_id == SOC_CORE_ALL) {
          res = READ_ECI_GLOBAL_FMC_0r(unit, &reg_value);
          DNXC_IF_ERR_EXIT(res);

          soc_reg64_field32_set(unit, ECI_GLOBAL_FMC_0r, &reg_value, FMC_QNUM_LOW_0f, queue_range->start);
          soc_reg64_field32_set(unit, ECI_GLOBAL_FMC_0r, &reg_value, FMC_QNUM_HIGH_0f, queue_range->end);

          res = WRITE_ECI_GLOBAL_FMC_0r(unit, reg_value);
          DNXC_IF_ERR_EXIT(res);
      }
      if (core_id == 1 || core_id == SOC_CORE_ALL) {
          res = READ_ECI_GLOBAL_FMC_1r(unit, &reg_value);
          DNXC_IF_ERR_EXIT(res);

          soc_reg64_field32_set(unit, ECI_GLOBAL_FMC_1r, &reg_value, FMC_QNUM_LOW_1f, queue_range->start);
          soc_reg64_field32_set(unit, ECI_GLOBAL_FMC_1r, &reg_value, FMC_QNUM_HIGH_1f, queue_range->end);

          res = WRITE_ECI_GLOBAL_FMC_1r(unit, reg_value);
          DNXC_IF_ERR_EXIT(res);
      }
  }

exit:
  DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Configure the Enhanced Fabric Multicast Queue
*     configuration: the fabric multicast queues are defined
*     in a configured range, and the credits are coming to
*     these queues according to a scheduler scheme.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_jer_mult_fabric_enhanced_get(
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_IN  int                                 core_id,
    DNX_SAND_INOUT DNX_SAND_U32_RANGE                *queue_range
  )
{
  int
    res;
  uint64
    reg_value;
  uint32 reg32;
  
  DNXC_INIT_FUNC_DEFS;
  COMPILER_64_SET(reg_value, 0, 0);
  DNXC_NULL_CHECK(queue_range);
  if (core_id != SOC_CORE_ALL && (core_id < 0 || core_id >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores)) {
      LOG_ERROR(BSL_LS_SOC_STAT,(BSL_META_U(unit, "Core %d out of range\n"), core_id));
      DNXC_SAND_IF_ERR_EXIT(SOC_E_PARAM);
  }


  if (SOC_IS_QAX(unit)) { /* no cores */
      res = READ_IPS_FMC_QNUM_RANGEr(unit, &reg32);
      DNXC_IF_ERR_EXIT(res);

      queue_range->start = soc_reg_field_get(unit, IPS_FMC_QNUM_RANGEr, reg32, FMC_QNUM_LOWf);
      queue_range->end = soc_reg_field_get(unit, IPS_FMC_QNUM_RANGEr, reg32, FMC_QNUM_HIGHf);
  } else {
      if (core_id == 0 || core_id == SOC_CORE_ALL) {
          res = READ_ECI_GLOBAL_FMC_0r(unit, &reg_value);
          DNXC_IF_ERR_EXIT(res);

          queue_range->start = soc_reg64_field32_get(unit, ECI_GLOBAL_FMC_0r, reg_value, FMC_QNUM_LOW_0f);
          queue_range->end = soc_reg64_field32_get(unit, ECI_GLOBAL_FMC_0r, reg_value, FMC_QNUM_HIGH_0f);
      } else if (core_id == 1) {
          res = READ_ECI_GLOBAL_FMC_1r(unit, &reg_value);
          DNXC_IF_ERR_EXIT(res);

          queue_range->start = soc_reg64_field32_get(unit, ECI_GLOBAL_FMC_1r, reg_value, FMC_QNUM_LOW_1f);
          queue_range->end = soc_reg64_field32_get(unit, ECI_GLOBAL_FMC_1r, reg_value, FMC_QNUM_HIGH_1f);
      } 
  }

exit:
  DNXC_FUNC_RETURN;
}
#undef _ERR_MSG_MODULE_NAME

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

#endif /* of #if defined(BCM_88690_A0) */
