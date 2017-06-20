/*
 * $Id: robo_srab.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BCM5301X RoboSwitch utility functions
 */

#ifndef _robo_srab_h_
#define _robo_srab_h_

extern void * robo_attach(void *sih);
extern void robo_detach(void *robo);
extern void robo_rreg(void *robo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len);
extern void robo_wreg(void *robo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len);
extern void robo_model_id_adjust_from_otp(void *rinfo, uint16 *model_id);


#endif /* _robo_spi_h_ */

