/*
 * $Id: vlan.c,v 1.24 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: VLAN management
 */

#include <shared/bsl.h>

#include <soc/drv.h>

#include <bcm/error.h>
#include <bcm/vlan.h>
#include <bcm/types.h>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/lock.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbStatus.h>

int
bcm_sbx_vlan_init(int unit)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_vlan_init == NULL) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_init(unit);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_create(int unit,
                    bcm_vlan_t vid)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_vlan_create == NULL) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_create(unit, vid);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_destroy(int unit,
                     bcm_vlan_t vid)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_vlan_destroy == NULL) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_destroy(unit, vid);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_destroy_all(int unit)
{
    int rc = BCM_E_NONE;


    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if ((mbcm_sbx_driver[unit] == NULL) || (mbcm_sbx_driver[unit]->mbcm_vlan_destroy_all == NULL)) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_destroy_all(unit);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_port_add(int unit,
                      bcm_vlan_t vid,
                      pbmp_t pbmp,
                      pbmp_t ubmp)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_vlan_port_add == NULL) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_port_add(unit, vid, pbmp, ubmp);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_port_remove(int unit,
                         bcm_vlan_t vid,
                         pbmp_t pbmp)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_vlan_port_remove == NULL) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_port_remove(unit, vid, pbmp);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_port_get(int unit,
                      bcm_vlan_t vid,
                      pbmp_t *pbmp,
                      pbmp_t *ubmp)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_vlan_port_get == NULL) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_port_get(unit, vid, pbmp, ubmp);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_list(int unit,
                  bcm_vlan_data_t **listp,
                  int *countp)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_vlan_list == NULL) {
    BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_list(unit, listp, countp);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_list_by_pbmp(int unit,
                          pbmp_t pbmp,
                          bcm_vlan_data_t **listp,
                          int *countp)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_vlan_list_by_pbmp == NULL) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_list_by_pbmp(unit, pbmp, listp, countp);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_list_destroy(int unit,
                          bcm_vlan_data_t *list,
                          int count)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_vlan_list_destroy == NULL) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_list_destroy(unit, list, count);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_default_get(int unit,
                         bcm_vlan_t *vid_ptr)
{
    int rc = BCM_E_NONE;


    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if ((mbcm_sbx_driver[unit] == NULL) || (mbcm_sbx_driver[unit]->mbcm_vlan_default_get == NULL)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_default_get(unit, vid_ptr);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_vlan_default_set(int unit,
                         bcm_vlan_t vid)
{
    int rc = BCM_E_NONE;


    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if ((mbcm_sbx_driver[unit] == NULL) || (mbcm_sbx_driver[unit]->mbcm_vlan_default_set == NULL)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_vlan_default_set(unit, vid);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}
