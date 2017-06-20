/* $Id: uc.h,v 1.5 Broadcom SDK $
 * $Id: 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        uc.h
 */

#ifndef _SOC_UC_H_
#define _SOC_UC_H_

#include <soc/types.h>

extern int soc_uc_init(int unit);
extern int soc_uc_reset(int unit, int uC);
extern int soc_uc_in_reset(int unit, int uC);
extern int soc_uc_preload(int unit, int uC);
extern int soc_uc_load(int unit, int uC, uint32 addr, int len,
                       unsigned char *data);
extern int soc_uc_start(int unit, int uC, uint32 addr);

extern char *soc_uc_firmware_version(int unit, int uC);

extern uint32 soc_uc_mem_read(int unit, uint32 addr);
extern int soc_uc_mem_write(int unit, uint32 addr, uint32 value);

extern int soc_uc_sram_extents(int unit, uint32 *addr, uint32 *size);

extern char *soc_uc_firmware_thread_info(int unit, int uC);

extern int soc_uc_stats_reset(int unit, int uC);

extern int soc_uc_status(int unit, int uC, int *status);

#endif
