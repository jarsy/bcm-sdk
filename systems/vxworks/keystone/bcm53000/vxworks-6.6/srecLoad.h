/* $Id: srecLoad.h,v 1.2 2011/07/21 16:14:28 yshtil Exp $
 * srecLoad
 *
 * by Curt McDowell, 09-05-99
 */

#ifndef _SRECLOAD_H
#define _SRECLOAD_H

#include <stdio.h>

/*
 * Return codes for srecLoad()
 */

#define SREC_ERROR_NONE		-0	/* Success */
#define SREC_ERROR_FORMAT	-1	/* File format error */
#define SREC_ERROR_CHECKSUM	-2	/* Checksum mismatch */
#define SREC_ERROR_NRECORDS	-3	/* Incorrect number of records */
#define SREC_ERROR_TRUNC	-4	/* Truncated line */
#define SREC_ERROR_ADDR		-5	/* Address out of range */

extern int srecLoad(FILE *fp, char *buf, int bufLen, int *entry);
extern char *srecErrmsg(int code);

#endif /* _SRECLOAD_H */
