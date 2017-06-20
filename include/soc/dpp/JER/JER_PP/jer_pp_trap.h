/* $Id: arad_pp_trap.h,v 1.27 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __JER_PP_TRAP_INCLUDED__
/* { */
#define __JER_PP_TRAP_INCLUDED__



#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/SAND/Utils/sand_footer.h>

#include <soc/dpp/PPC/ppc_api_eg_mirror.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */


#define MIRROR_STRENGTH_MAX_VALUE  (3)
#define MIRROR_STRENGTH_MIN_VALUE  (0)
#define FWD_STRENGTH_MAX_VALUE     (3)
#define FWD_STRENGTH_MIN_VALUE     (0)
#define MIRROR_COMMAND_MAX_VALUE   (15)
#define MIRROR_COMMAND_MIN_VALUE   (0)



/* static array defines */

/* positions in the table */
#define ETPP_OUT_VPORT_DISCARD_INDEX                       (0)
#define ETPP_STP_STATE_FAIL_INDEX                          (1)
#define ETPP_PROTECTION_PATH_UNEXPECTED_INDEX              (2)
#define ETPP_VPORT_LOOKUP_FAIL_INDEX                       (3)
#define ETPP_MTU_FILTER                                    (4)
#define ETPP_ACC_FRAME_TYPE                                (5)
#define ETPP_SPLIT_HORIZON                                 (6)
#define ETPP_NOF_TRAPS                                     (7)


#define ETPP_MIRROR_CMD_INDEX                              (0)
#define ETPP_FWD_STRENGTH_INDEX                            (1)
#define ETPP_MIRROR_STRENGTH_INDEX                         (2)
#define ETPP_FWD_ENABLE_INDEX                              (3)
#define ETPP_MIRROR_ENABLE_INDEX                           (4)
#define ETPP_NUMBER_TRAP_FIELDS                            (5)

#define ETPP_EPNI_NOF_MTU_PROFILE                          (8)
#define ETPP_EPNI_NOF_MTU_PROFILES_BIT                     (3)


/*********************************************************************
* NAME:
 *   soc_jer_pp_eg_pmf_mirror_params_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set mirror profile table to use by PMF when it matches (on
 *   action) a specific mirror profile.
 * INPUT:
 *   int    unit -
 *       Identifier of the device to access.
 *   uint32 mirror_profile -
 *       Mirror profile to use as index into table
 *   dpp_outbound_mirror_config_t *config -
 *       Parameters to load into table. Note that mirror_enable
 *       and fwd_enable are among these parameters.
 * REMARKS:
 *   For Jericho (and up) only.
 * See also:
 *   EPNI_PMF_MIRROR_PROFILE_TABLE
 *   soc_jer_pp_eg_pmf_mirror_params_set()
 *   bcm_petra_mirror_port_destination_add()
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
soc_jer_pp_eg_pmf_mirror_params_set(int unit, uint32 mirror_profile, dpp_outbound_mirror_config_t *config) ;

soc_error_t
soc_jer_eg_etpp_trap_set(int unit, SOC_PPC_TRAP_ETPP_TYPE trap, SOC_PPC_TRAP_ETPP_INFO *entry_info);

soc_error_t
soc_jer_eg_etpp_trap_get(int unit, SOC_PPC_TRAP_ETPP_TYPE trap, SOC_PPC_TRAP_ETPP_INFO *entry_info);

soc_error_t
soc_jer_eg_etpp_verify_parmas(int unit, SOC_PPC_TRAP_ETPP_INFO *info);

void
soc_jer_eg_etpp_trap_get_array_index( SOC_PPC_TRAP_CODE trap, int *trap_index);

soc_error_t 
soc_jer_eg_etpp_out_lif_mtu_map_set(int unit,uint32 out_lif_profile_bit_mask, uint32 mtu_profile, uint32 mtu_val);

soc_error_t 
soc_jer_eg_etpp_out_lif_mtu_map_get(int unit,uint32 out_lif_profile, uint32 *mtu_val);

soc_error_t 
soc_jer_eg_etpp_out_lif_mtu_check_set(int unit,uint32 enable);




/* }__JER_PP_TRAP_INCLUDED__*/
#endif
