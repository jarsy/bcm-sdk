/*
 * $Id: sflash.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    sflash.h
 */

#ifndef _sflash_h_
#define _sflash_h_

#include <vxWorks.h>

struct sflash {
	UINT32 blocksize;		/* Block size */
	UINT32 numblocks;		/* Number of blocks */
	UINT32 type;		/* Type */
	UINT32 size;		/* Total size in bytes */
};

/* Utility functions */
extern int sflash_read(UINT32 offset, UINT32 len, unsigned char *buf);
extern int sflash_write(UINT32 offset, UINT32 len, const unsigned char *buf);
extern int sflash_erase(UINT32 offset);
extern struct sflash *sflash_init(void);
extern STATUS sysFlashGet(char *string, int strLen, int offset);
extern STATUS sysFlashSet(char *string, int byteLen, int offset);
#endif /* _sflash_h_ */
