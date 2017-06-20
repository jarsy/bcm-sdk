/*
 * $Id: mim.h,v 1.14 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Soc_petra-B Layer 2 Management
 */

#ifndef   _BCM_INT_DPP_MIM_H_
#define   _BCM_INT_DPP_MIM_H_

#include <bcm_int/dpp/error.h>

#include <bcm/mim.h>
#include <bcm/types.h>

#include <soc/dpp/drv.h>


/* 
 * Defines
 */

#define BCM_PETRA_MIM_VPN_INVALID   (0xffff)

#define BCM_PETRA_MIM_BTAG_TPID    (0x81a8)
#define BCM_PETRA_MIM_ITAG_TPID    (0x88e7)

#define BCM_PETRA_MIM_BVID_MC_GROUP_BASE    (12*1024)

#define MIM_ACCESS sw_state_access[unit].dpp.bcm.mim


/*
 * MiM Module Helper functions
 */

uint8
  __dpp_mim_initialized_get(int unit);

SOC_PPC_LIF_ID
  __dpp_mim_lif_ndx_get(int unit);

SOC_PPC_AC_ID
  __dpp_mim_global_out_ac_get(int unit);


int dpp_mim_set_global_mim_tpid(int unit, uint16 tpid);
int dpp_mim_get_global_mim_tpid(int unit, uint16 *tpid);


int
_bcm_dpp_in_lif_mim_match_get(int unit, bcm_mim_port_t *mim_port, int lif);

/* 
 * Macros
 */

#define MIM_INIT(unit)                                    \
    do {                                                  \
        if (!__dpp_mim_initialized_get(unit)) {           \
            return BCM_E_INIT;                            \
        }                                                 \
    } while (0)

#define MIM_IS_INIT(unit)   (__dpp_mim_initialized_get(unit))


typedef struct bcm_dpp_mim_info_s {
    SOC_PPC_LIF_ID    mim_local_lif_ndx; /* default local lif, used only for MiM */
    SOC_PPC_AC_ID     mim_local_out_ac; /* default out-ac, used only for MiM */
} bcm_dpp_mim_info_t;


#endif /* _BCM_INT_DPP_MIM_H_ */
     
