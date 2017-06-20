/*
 * BCM47XX FLASH driver interface
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $Id: flashutl.h,v 1.2 Broadcom SDK $
 */

#ifndef _flashutl_h_
#define _flashutl_h_

#define FLASH_BASE_ADDRESS_FLASH_BOOT   0xBFC00000
#define FLASH_BASE_ADDRESS_ALIAS        0xBC000000

#ifndef _LANGUAGE_ASSEMBLY

int sysFlashInit(char *flash_str);
int sysFlashRead(uint off, uchar *dst, uint bytes);
int sysFlashWrite(uint off, uchar *src, uint bytes);
void nvStartOffset(uint off);
void nvWrite(unsigned short *data, unsigned int len);
void nvWriteChars(unsigned char *data, unsigned int len);

#endif /* _LANGUAGE_ASSEMBLY */

#endif /* _flashutl_h_ */

