/*
 * $Id: tcal.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _BCM_INT_SBX_FE2000_TCAL_H_
#define _BCM_INT_SBX_FE2000_TCAL_H_

#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#endif /* def BCM_FE2000_P3_SUPPORT */

#define _BCM_TCAL_ID_MASK         0xFFFF
#define _BCM_TCAL_INVALID_ID      0xFFFF
#define _BCM_TCAL_ID_VALID(x)    \
       (((x) & _BCM_TCAL_ID_MASK) != _BCM_TCAL_INVALID_ID)


typedef uint32 tcal_id_t;

/*
 *   Function
 *      _bcm_tcal_init
 *   Purpose
 *      initialize the timer calendar manager
 *   Parameters
 *      (in)  unit          - BCM device number
 *   Returns
 *       BCM_E_*
 */
int _bcm_tcal_init(int unit);

/*
 *   Function
 *      _bcm_tcal_cleanup
 *   Purpose
 *      free any timer calendar resources
 *   Parameters
 *      (in)  unit          - BCM device number
 *   Returns
 *       BCM_E_*
 */
int _bcm_tcal_cleanup(int unit);

/*
 *   Function
 *      _bcm_tcal_update
 *   Purpose
 *      Allocate a BCM8802x timer calendar entry
 *   Parameters
 *      (in)  unit          - BCM8802x BCM device number
 *      (in)  period        - period in ms, for which a timer should expire
 *      (in)  tx_enable     - enable the timer to intiate transmit
 *      (in)  ep_rec_index  - endpoint record index to transmit
 *      (out) tcal_id       - timer calenedar entry to update
 *   Returns
 *       BCM_E_*
 */
int _bcm_tcal_alloc(int unit, int period, int tx_enable,
                    uint32 ep_rec_index, tcal_id_t *tcal_id);

/*
 *   Function
 *      _bcm_tcal_update
 *   Purpose
 *      Update an existing BCM8802x timer calendar entry, if existing entry
 *      is implemented as multiple entries, they will be freed as required.
 *   Parameters
 *      (in)  unit          - BCM8802x BCM device number
 *      (in)  period        - period in ms, for which a timer should expire
 *      (in)  tx_enable     - enable the timer to intiate transmit
 *      (in)  ep_rec_index  - endpoint record index to transmit
 *      (inout) tcal_id     - timer calenedar entry to update
 *   Returns
 *       BCM_E_*
 */
int _bcm_tcal_update(int unit, int period, int tx_enable, 
                     uint32 ep_rec_index, tcal_id_t *tcal_id);


/*
 *   Function
 *      _bcm_tcal_free
 *   Purpose
 *      Release the resources associated with the given tcalid
 *   Parameters
 *      (in)  unit          - BCM device number
 *      (in/out)  tcal_id   - tcal_id to free
 *   Returns
 *       BCM_E_*
 */
int _bcm_tcal_free(int unit, tcal_id_t *tcal_id);

/*
 *   Function
 *      _bcm_tcal_interval_decode
 *   Purpose
 *      Convert from interval enumeration to interval in ms.
 *   Parameters
 *      (in)  unit          - BCM device number
 *      (in)  encoded       - Interval enumeration
 *      (out) interval_ms   - Interval in ms
 *   Returns
 *       BCM_E_*
 */
int _bcm_tcal_interval_decode(int unit, uint32 encoded, int *interval_ms);

/*
 *   Function
 *      _bcm_tcal_interval_encode
 *   Purpose
 *      Convert from interval in ms to an interval enumeration.
 *   Parameters
 *      (in)  unit          - BCM device number
 *      (in)  interval_ms   - Interval in ms
 *      (out) encoded       - Interval enumeration
 *   Returns
 *       BCM_E_*
 */
int _bcm_tcal_interval_encode(int unit, int interval_ms, uint32 *encoded);

/*
 *   Function
 *      _bcm_tcal_period_get
 *   Purpose
 *      obtains period associated with Tcal
 *   Parameters
 *      (in)  unit          - BCM device number
 *      (in)  tcal_id       - tcal_id
 *      (in/out) period     - timer period
 *      (in/out) tx_enable  - >0 if timer enabled
 *   Returns
 *       BCM_E_*
 */
int
_bcm_tcal_period_get(int unit, tcal_id_t tcal_id, int *period, int *tx_enable);

#endif  /* _BCM_INT_SBX_FE2000_TCAL_H_  */
