/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * TRX image file header format.
 *
 * Copyright (C) 2001 Broadcom Corporation
 *
 * $Id: trxhdr.h,v 1.1 2004/02/24 07:47:00 csm Exp $
 */ 

#define TRX_MAGIC	0x30524448	/* "HDR0" */
#define TRX_VERSION	1
#define TRX_MAX_LEN	0x3A0000

struct trx_header {
	unsigned long magic;		/* "HDR0" */
	unsigned long len;		/* Length of file */
	unsigned long crc32;		/* 32-bit CRC across length of file */
	unsigned long flag_version;	/* 0:15 flags, 16:31 version */
	unsigned long reserved[3];
};

/* Compatibility */
typedef struct trx_header TRXHDR, *PTRXHDR;
