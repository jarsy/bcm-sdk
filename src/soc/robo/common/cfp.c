/*
 * $Id: cfp.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * CFP driver service
 */
#include <assert.h>
#include <soc/types.h>
#include <soc/error.h>
#include <soc/cfp.h>


_fp_id_info_t _robo_fp_id_info[SOC_MAX_NUM_DEVICES];

/*
 * Function: drv_cfp_qset_get
 *
 * Purpose:
 *     Get the qualify bit value from CFP entry. 
 *
 * Parameters:
 *     unit - BCM device number
 *     qual - qualify ID
 *     entry -CFP entry
 *     val(OUT) -TRUE/FALSE
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv_cfp_qset_get(int unit, uint32 qual, drv_cfp_entry_t *entry, uint32 *val)
{
    int rv = SOC_E_NONE;
    uint32  wp, bp;

    assert(entry);
    
    wp = qual / 32;
    bp = qual & (32 - 1);
    if (entry->w[wp] & (1 << bp)) {
        *val = TRUE;
    } else {
        *val = FALSE;
    }

    return rv;
}

/*
 * Function: drv_cfp_qset_set
 *
 * Purpose:
 *     Set/Reset the qualify bit value to CFP entry. 
 *
 * Parameters:
 *     unit - BCM device number
 *     qual - qualify ID
 *     entry -CFP entry
 *     val -TRUE/FALSE
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv_cfp_qset_set(int unit, uint32 qual, drv_cfp_entry_t *entry, uint32 val)
{
    int rv = SOC_E_NONE;
    uint32  wp, bp, temp = 0;

    assert(entry);
    
    wp = qual / 32;
    bp = qual & (32 - 1);
    if (val) {
        temp = 1;
    }

    if (temp) {
        entry->w[wp] |= (1 << bp);
    } else {
        entry->w[wp] &= ~(1 << bp);
    }
    
    return rv;
}


