/* $Id: arad_kbp.h,v 1.31 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_KBP_RECOVER_INCLUDED__
/* { */
#define __ARAD_KBP_RECOVER_INCLUDED__

/*************
 * INCLUDES  *
 *************/

int arad_kbp_recover_rx_enable (int unit, int mdio_id);

int arad_kbp_recover_rx_shut_down (int unit, int mdio_id);

int arad_kbp_recover_run_recovery_sequence(int unit, uint32 core, int mdio_id, uint32 retries, void *, int option);


#endif /*__ARAD_KBP_RECOVER_INCLUDED__*/
