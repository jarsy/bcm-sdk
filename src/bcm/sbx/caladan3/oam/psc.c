/*
 * $Id: psc.c,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * FE2000 OAM Protection State Coordination function
 */

#if defined(INCLUDE_L3)

#include <bcm/error.h>
#include <bcm/oam.h>

int bcm_caladan3_oam_psc_add(int unit,
                           bcm_oam_psc_t *psc_info)
{
    int status = BCM_E_UNAVAIL;
    return status;
}

int bcm_caladan3_oam_psc_delete(int unit,
                              bcm_oam_psc_t *psc_info)
{
    int status = BCM_E_UNAVAIL;
    return status;
}

int bcm_caladan3_oam_psc_get(int unit,
                           bcm_oam_psc_t *psc_info)
{
    int status = BCM_E_UNAVAIL;
    return status;
}


/* Add an OAM PW Status object */
int bcm_caladan3_oam_pw_status_add(
    int unit, 
    bcm_oam_pw_status_t *pw_status_ptr)
{
  return BCM_E_UNAVAIL;
}


/* Get an OAM PW Status object */
int bcm_caladan3_oam_pw_status_get(
    int unit, 
    bcm_oam_pw_status_t *pw_status_ptr)
{
  return BCM_E_UNAVAIL;
}

/* Delete an OAM PW Status object */
int bcm_caladan3_oam_pw_status_delete(
    int unit, 
    bcm_oam_pw_status_t *pw_status_ptr)
{
  return BCM_E_UNAVAIL;
}


#endif
