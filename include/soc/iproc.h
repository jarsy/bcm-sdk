/*
 * $Id: iproc.h,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef _SOC_IPROC_H
#define _SOC_IPROC_H

#include <soc/register.h>
#include <ibde.h>

#define PAXB_0_BASE           0x18012000
#define PAXB_1_BASE           0x18013000
#define PAXB_0_ENDIANESS      (PAXB_0_BASE + 0x30)
#define PAXB_1_ENDIANESS      (PAXB_1_BASE + 0x30)

extern int soc_iproc_init(int unit);
extern int soc_iproc_deinit(int unit);

extern int soc_iproc_reset(int unit, int uC);
extern int soc_iproc_shutdown(int unit, uint32 cpu_mask, int level);
extern int soc_iproc_preload(int unit, int uC);
extern int soc_iproc_load(int unit, int uC,
                          uint32 iproc_addr, int len, unsigned char *data);
extern int soc_iproc_start(int unit, int uC, uint32 iproc_addr);

extern int soc_iproc_getreg(int unit, uint32 addr, uint32 *data);
extern int soc_iproc_setreg(int unit, uint32 addr, uint32 data);

extern int soc_iproc_ddr_init(int unit);

#endif /* _SOC_IPROC_H */
