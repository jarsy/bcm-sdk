
 
/* $Id: arad_pp_api_eg_encap.c,v 1.22 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_EGRESS

/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>
#include <soc/dpp/SAND/Management/sand_chip_descriptors.h>
#include <soc/dpp/SAND/Utils/sand_bitstream.h>

#include <soc/dpp/JER/JER_PP/jer_pp_eg_filter.h>

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

int 
soc_jer_pp_network_group_config_set(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN uint32 source_network_group_id,
    SOC_SAND_IN uint32 dest_network_group_id, 
    SOC_SAND_IN uint32 is_filter)
{
    int res;
    uint32
        relevant_bit = 1 << (source_network_group_id << 2 | dest_network_group_id), /* bit to turn on or off */
        val;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* 
     * the register maps {inlif.orientation(2b), outlif orientation(2b)} to filter (1) or unfilter (0) 
     */
    res = READ_EPNI_CFG_SPLIT_HORIZON_FILTERr(unit, REG_PORT_ANY, &val);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    if (is_filter){
        val |= relevant_bit;
    } else {
        val &= ~relevant_bit;
    }
    res = WRITE_EPNI_CFG_SPLIT_HORIZON_FILTERr(unit, SOC_CORE_ALL, val);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);


exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_jer_pp_network_group_config_set()",0,0);
}

int 
soc_jer_pp_network_group_config_get(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN uint32 source_network_group_id,
    SOC_SAND_IN uint32 dest_network_group_id,
    SOC_SAND_OUT uint32 *is_filter)
{
    int res;
    uint32
        relevant_bit = 1 << (source_network_group_id << 2 | dest_network_group_id), /* bit to check if on or off */
        val;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = READ_EPNI_CFG_SPLIT_HORIZON_FILTERr(unit, REG_PORT_ANY, &val);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    if (val & relevant_bit) {
        *is_filter = 1;
    } else {
        *is_filter = 0;
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_jer_pp_network_group_config_get()",0,0);
}

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>
