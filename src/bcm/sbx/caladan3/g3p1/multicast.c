/*
 * $Id: multicast.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Multicast for G2P3
 */

#include <bcm/types.h>
#include <bcm/multicast.h>
#include <bcm/vlan.h>
#include <bcm_int/sbx/error.h>

#include <soc/sbx/sbx_drv.h>

#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>

#include <bcm_int/sbx/caladan3/vlan.h>

int
_bcm_caladan3_g3p1_multicast_l2_encap_get(int              unit,
                                        bcm_multicast_t  group,
                                        bcm_gport_t      gport,
                                        bcm_vlan_t       vlan,
                                        bcm_if_t        *encap_id)
{
    soc_sbx_g3p1_ft_t   ft;
    soc_sbx_g3p1_oi2e_t oie;
    int                 rv = BCM_E_NONE;
    uint32              ftIndex;

    rv = _bcm_caladan3_vlan_fte_gport_get(unit, gport, &ftIndex);
    if (BCM_E_NONE == rv) {
        soc_sbx_g3p1_ft_t_init(&ft);
        rv = soc_sbx_g3p1_ft_get(unit,
                                 ftIndex,
                                 &ft);

    }
    if (BCM_E_NONE == rv) {
        rv = soc_sbx_g3p1_oi2e_get(unit,
                                   (ft.oi - SBX_RAW_OHI_BASE),
                                   &oie);
    }
    if (BCM_E_NONE == rv) {
        *encap_id = SOC_SBX_ENCAP_ID_FROM_OHI(ft.oi);
    }

    return rv;
}
