/* $Id: soc_ppc_api_fp_fem.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_PPC_FP_FEM_INCLUDED__
/* { */
#define __SOC_PPC_FP_FEM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>
#include <soc/dpp/PPC/ppc_api_fp.h>

/* } */
/*************
 * DEFINES   *
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

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If True, this FEM is for a Direct Extraction entry.
   */
  uint8 is_for_entry;
  /*
   *  Database strength of this FEM
   */
  uint32 db_strength;
  /*
   *  Database-ID of this FEM.
   */
  uint32 db_id;
  /*
   *  Entry strength of this FEM. Relevant only if
   *  'is_for_entry' is True.
   */
  uint32 entry_strength;
  /*
   *  Entry-ID of this FEM (relevant only if 'is_for_entry' is
   *  True).
   */
  uint32 entry_id;
  /*
   *  Required action type. Needed to know the minimal FEM
   *  size.
   */
  SOC_PPC_FP_ACTION_TYPE action_type[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX];
  /*
   * If TRUE, then Base-Offset is positive
   */
  uint8 is_base_positive[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX];

} SOC_PPC_FP_FEM_ENTRY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If True, then the cycle-id is fixed (e.g., for TCAM
   *  Databases)
   */
  uint8 is_cycle_fixed;
  /*
   *  Cycle-ID required for this FEM
   */
  uint8 cycle_id;

} SOC_PPC_FP_FEM_CYCLE;


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
  SOC_PPC_FP_FEM_ENTRY_clear(
    SOC_SAND_OUT SOC_PPC_FP_FEM_ENTRY *info
  );

void
  SOC_PPC_FP_FEM_CYCLE_clear(
    SOC_SAND_OUT SOC_PPC_FP_FEM_CYCLE *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

void
  SOC_PPC_FP_FEM_ENTRY_print(
    SOC_SAND_IN  SOC_PPC_FP_FEM_ENTRY *info
  );

void
  SOC_PPC_FP_FEM_CYCLE_print(
    SOC_SAND_IN  SOC_PPC_FP_FEM_CYCLE *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_FP_FEM_INCLUDED__*/
#endif

