/*
 * Misc useful routines to access NIC local SROM/OTP .
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * $Id: bcmsrom.h,v 1.1 Broadcom SDK $
 */

#ifndef	_bcmsrom_h_
#define	_bcmsrom_h_

#include "bcmsrom_fmt.h"

/* Prototypes */
extern int srom_var_init(si_t *sih, uint bus, void *curmap, osl_t *osh,
                         char **vars, uint *count);

extern int srom_read(si_t *sih, uint bus, void *curmap, osl_t *osh,
                     uint byteoff, uint nbytes, uint16 *buf,
                     bool check_crc);

extern int srom_write(si_t *sih, uint bus, void *curmap, osl_t *osh,
                      uint byteoff, uint nbytes, uint16 *buf);

/* parse standard PCMCIA cis, normally used by SB/PCMCIA/SDIO/SPI/OTP
 *   and extract from it into name=value pairs
 */
extern int srom_parsecis(osl_t *osh, uint8 **pcis, uint ciscnt,
                         char **vars, uint *count);

#endif	/* _bcmsrom_h_ */
