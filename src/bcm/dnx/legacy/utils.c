/* 
 * $Id: utils.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        bcm_sand.h
 * Purpose:     Conversion between BCM and DNX_SAND types, and common macros/function for
 *              handling Dune's code.
 */

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_BCM_COMMON

#include <shared/bsl.h>

#include <bcm/error.h>
#include <bcm_int/common/debug.h>
#include <bcm/debug.h>

#include <sal/core/libc.h>

#include <bcm_int/dnx/legacy/utils.h>
#include <bcm_int/dnx/legacy/error.h>
#include <bcm_int/dnx/legacy/cosq.h>

#include <soc/dnx/legacy/port_sw_db.h>
#include <soc/dnx/legacy/mbcm.h>


/*
 *   Function
 *      pbmp_from_ppd_port_bitmap
 *   Purpose
 *      Convert array of port as used by ppd api to bcm_pbmp_t. 
 *
 *   Parameters
 *      (IN)  pbmp         : bcm_pbmp_t to be filled
 *      (IN)  ports        : ports bitmap in dnx_sand format
 *      (IN)  ports_len    : length of ports array, in longs
 *   Returns
 *       BCM_E_NONE - success
 *       BCM_E_*    - failure
 */

int
pbmp_from_ppd_port_bitmap(
    int unit,
    bcm_pbmp_t *pbmp,
    uint32 *ports, 
    int ports_len)
{
    int port_i, bits_per_long;
    BCMDNX_INIT_FUNC_DEFS;
    bits_per_long = sizeof(*ports) * 8;

    BCM_PBMP_CLEAR(*pbmp);
    
    if (ports_len > _SHR_PBMP_WIDTH) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "pbmp_from_ppd_port_bitmap: pbmp size is smaller than 'ports_len'")));
        BCM_RETURN_VAL_EXIT(BCM_E_PARAM);
    }

    for (port_i = 0; port_i < ports_len * bits_per_long; ++port_i) {        
        if ((ports[port_i / bits_per_long] & (1<<(port_i % bits_per_long))) != 0) {

            BCM_PBMP_PORT_ADD(*pbmp, port_i);
        }
    }
    
    BCM_EXIT;
exit:
    BCMDNX_FUNC_RETURN;
}

/*
 *   Function
 *      pbmp_to_ppd_port_bitmap
 *   Purpose
 *      Convert bcm_pbmp_t to array of port as used by ppd api. 
 *
 *   Parameters
 *      (IN)  pbmp         : source bcm_pbmp_t bitmap
 *      (IN)  ports        : ports bitmap in dnx_sand format, to be filled
 *      (IN)  ports_len    : length of ports array, in longs
 *   Returns
 *       BCM_E_NONE - success
 *       BCM_E_*    - failure
 */

int
pbmp_to_ppd_port_bitmap(
    int unit,
    bcm_pbmp_t *pbmp,
    uint32 *ports, 
    int ports_len)
{
    int port_i, offset, bits_per_long;

    BCMDNX_INIT_FUNC_DEFS;
    bits_per_long = sizeof(*ports) * 8;
       
    sal_memset(ports, 0x0, ports_len * sizeof(*ports));

    BCM_PBMP_ITER(*pbmp, port_i) {
        offset = port_i / bits_per_long;

        if (offset >= ports_len) {
            /* Bitmap is longer than the ports array. Stop copying */
            break;
        }

        ports[offset] |= (1<<(port_i % bits_per_long));
    }
    
    BCM_EXIT;
exit:
    BCMDNX_FUNC_RETURN;
}

/*
 *   Function
 *      dnx_pbmp_from_ppd_port_bitmap_convert
 *   Purpose
 *      Convert array of port as used by ppd api to bcm_pbmp_t.
 *      Currently : Converts input from pp_to_local;  
 * 
 *
 *   Parameters
 *      (IN)  core_id      : core id
 *      (IN)  pbmp         : bcm_pbmp_t to be filled
 *      (IN)  ports        : ports bitmap in dnx_sand format
 *      (IN)  ports_len    : length of ports array, in longs
 *   Returns
 *       BCM_E_NONE - success
 *       BCM_E_*    - failure
 */

int
dnx_pbmp_from_ppd_port_bitmap_convert(
    int unit,
    int core_id,
    uint32 *ports, 
    int ports_len,
    _bcm_dnx_convert_flag_t flag,
    bcm_pbmp_t *pbmp
    )
{
    int port_i, bits_per_long;
    bcm_port_t port_from_ppd = 0;
    BCMDNX_INIT_FUNC_DEFS;
    bits_per_long = sizeof(*ports) * 8;

    BCM_PBMP_CLEAR(*pbmp);
    
    if (ports_len > _SHR_PBMP_WIDTH) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "pbmp_from_ppd_port_bitmap: pbmp size is smaller than 'ports_len'")));
        BCM_RETURN_VAL_EXIT(BCM_E_PARAM);
    }

    for (port_i = 0; port_i < ports_len * bits_per_long; ++port_i) {        
        if ((ports[port_i / bits_per_long] & (1<<(port_i % bits_per_long))) != 0) {
            switch (flag) {
            case _BCM_DNX_CONVERT_FLAG_NONE:
                port_from_ppd = port_i;
                break;
            case _BCM_DNX_CONVERT_FLAG_PP_TO_LOCAL:
                if(BCM_E_NOT_FOUND == MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_pp_to_local_port_get, (unit, core_id, port_i, &port_from_ppd))){
                     LOG_ERROR(BSL_LS_BCM_COMMON,(BSL_META_U(unit,"port %d is invalid\n"),port_i));
                     continue;
                }
                break;
            case _BCM_DNX_CONVERT_FLAG_TM_TO_LOCAL:
                BCMDNX_IF_ERR_EXIT( MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_tm_to_local_port_get, (unit, core_id, port_i, &port_from_ppd)));
                break;
            default:
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "Conversion flag not supported")));
                BCM_RETURN_VAL_EXIT(BCM_E_PARAM);
            }

            BCM_PBMP_PORT_ADD(*pbmp, port_from_ppd);
        }
    }
    
    BCM_EXIT;
exit:
    BCMDNX_FUNC_RETURN;
}
/*
 *   Function
 *      pbmp_to_ppd_port_bitmap
 *   Purpose
 *      Convert bcm_pbmp_t to array of port as used by ppd api.
 *      Currently : Converts input from local to pp. Ports with different core are filltered.  
 * 
 *
 *   Parameters
 *      (IN)  core_id      : Expected core.
 *      (IN)  pbmp         : source bcm_pbmp_t bitmap
 *      (IN)  ports        : ports bitmap in dnx_sand format, to be filled
 *      (IN)  ports_len    : length of ports array, in longs
 *   Returns
 *       BCM_E_NONE - success
 *       BCM_E_*    - failure
 */

int
dnx_pbmp_to_ppd_port_bitmap_convert(
    int unit,
    int core_id,
    bcm_pbmp_t *pbmp,
    int ports_len,
    _bcm_dnx_convert_flag_t flag,
    uint32 *ports
    )
{
    int port_i, offset, bits_per_long, core;
    uint32 flags;
    uint32 soc_ppd_port_i;

    BCMDNX_INIT_FUNC_DEFS;
    bits_per_long = sizeof(*ports) * 8;
       
    sal_memset(ports, 0x0, ports_len * sizeof(*ports));

    BCM_PBMP_ITER(*pbmp, port_i) {
        BCMDNX_IF_ERR_EXIT(dnx_port_sw_db_flags_get(unit, port_i, &flags)); 
        if (DNX_PORT_IS_STAT_INTERFACE(flags))
        {
            continue;
        }
        switch (flag) {
        case _BCM_DNX_CONVERT_FLAG_NONE:
            soc_ppd_port_i = port_i;
            core = core_id;
            break;
        case _BCM_DNX_CONVERT_FLAG_LOCAL_TO_PP:
            BCMDNX_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_local_to_pp_port_get, (unit, port_i, &soc_ppd_port_i, &core)));
            if (core != core_id) {
                continue;
            }
            break;
        case _BCM_DNX_CONVERT_FLAG_LOCAL_TO_TM:
            BCMDNX_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_local_to_tm_port_get, (unit, port_i, &soc_ppd_port_i, &core)));
            if (core != core_id) {
                continue;
            }
            break;

        default:
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Conversion flag not supported")));
            BCM_RETURN_VAL_EXIT(BCM_E_PARAM);
        }
        offset = soc_ppd_port_i / bits_per_long;

        if (offset >= ports_len) {
            /* Bitmap is longer than the ports array. Stop copying */
            break;
        }

        ports[offset] |= (1<<(soc_ppd_port_i % bits_per_long));
    }
    
    BCM_EXIT;
exit:
    BCMDNX_FUNC_RETURN;
}

/*
 *   Function
 *      _bcm_dnx_pbmp_filter_by_core
 *   Purpose
 *      Returns only ports of the input core. 
 *
 *   Parameters
 *      (IN)  core_id      : required core
 *      (IN)  pbmp         : bcm_pbmp_t of local ports
 *      (OUT) core_pbmp    : returned pbmp: ports of the core
 *   Returns
 *       BCM_E_NONE - success
 *       BCM_E_*    - failure
 */

int
_bcm_pbmp_filter_by_core(
    int unit,
    int core_id,
    bcm_pbmp_t pbmp,
    bcm_pbmp_t *core_pbmp)
{

    bcm_port_t port_i;
    int        core;
    uint32     soc_ppd_port_i; 

    BCMDNX_INIT_FUNC_DEFS;

    BCM_PBMP_CLEAR(*core_pbmp);
    
    BCM_PBMP_ITER(pbmp, port_i) {
        BCMDNX_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_local_to_pp_port_get, (unit, port_i, &soc_ppd_port_i, &core)));

        if (core == core_id) {
             BCM_PBMP_PORT_ADD(*core_pbmp, port_i);
        }
    }
    
    BCM_EXIT;
exit:
    BCMDNX_FUNC_RETURN;
}



