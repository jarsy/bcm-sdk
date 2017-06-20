/* $Id: srecLoad.c,v 1.2 2011/07/21 16:14:28 yshtil Exp $
 * srecLoad
 *
 * by Curt McDowell, 09-05-99
 */

#include <string.h>
#include <stdlib.h>

#include "srecLoad.h"

#if 0
int test_me()
{
}
int test_me1()
{
}
#endif
/*
 * nextByte:
 *
 * Utility routine for srecLoad()
 */

static int nextByte(char **p)
{
    int			d1, d2;

    d1 = **p;
    if (d1 >= '0' && d1 <= '9')
	d1 = d1 - '0';
    else if (d1 >= 'A' && d1 <= 'F')
	d1 = d1 - 'A' + 10;
    else
	return -1;

    d1 *= 16;

    (*p)++;

    d2 = **p;
    if (d2 >= '0' && d2 <= '9')
	d1 += d2 - '0';
    else if (d2 >= 'A' && d2 <= 'F')
	d1 += d2 - 'A' + 10;
    else
	return -1;

    (*p)++;
    return d1;
}

/*
 * srecLoad:
 *
 * Routine to read an S-record format file into a fixed-size buffer.
 *
 * On success, returns number of bytes loaded.
 * On failure, returns negative error code.
 */

int srecLoad(FILE *fp, char *buf, int bufLen, int *entry)
{
    static unsigned char addrLens[] = { 2, 2, 3, 4, 0, 2, 0, 4, 3, 2 };
    char		line[1 + 1 + 2 + 255*2 + 1];
    char		dataBytes[255], *p;
    int			dataLen, dataRecs;
    char		*s;
    int			type, x, addrLen, byteCount, byteTotal;
    int			addr;
    unsigned char	chkSum;

    memset(buf, 0, bufLen);
    *entry = 0;

    dataRecs = 0;
    byteTotal = 0;

    while (fgets(line, sizeof (line), fp) != 0) {
	s = line;
	chkSum = 0;
	if (*s++ != 'S')
	    continue;
	type = *s++;
	if (type < '0' || type > '9')
	    return SREC_ERROR_FORMAT;
	addrLen = addrLens[type - '0'];
	if ((byteCount = nextByte(&s)) < 0)
	    return SREC_ERROR_FORMAT;
	chkSum = (unsigned char) byteCount;
	byteCount -= addrLen;
	if (byteCount < 1)
	    return SREC_ERROR_FORMAT;	/* Need at least addr and checksum */
	addr = 0;
	while (addrLen--) {
	    if ((x = nextByte(&s)) < 0)
		return SREC_ERROR_TRUNC;
	    addr = addr << 8 | x;
	    chkSum += (unsigned char) x;
	}
	p = dataBytes;
	dataLen = byteCount - 1;
	while (byteCount-- > 1) {
	    if ((x = nextByte(&s)) < 0)
		return SREC_ERROR_TRUNC;
	    *p++ = (char) x;
	    chkSum += (unsigned char) x;
	}
	if ((x = nextByte(&s)) < 0)
	    return SREC_ERROR_TRUNC;
	chkSum += (unsigned char) x;
	if (chkSum != (unsigned char) 0xff)
	    return SREC_ERROR_CHECKSUM;
	switch (type) {
	case '0':		/* Header record */
	    dataRecs = 0;
	    break;
	case '5':		/* Record count */
	    if (addr != dataRecs)
		return SREC_ERROR_NRECORDS;
	    break;		/* Ignore */
	case '1':		/* 2-byte address with data */
	case '2':		/* 3-byte address with data */
	case '3':		/* 4-byte address with data */
	    if (addr < 0 || addr + dataLen > bufLen){
		printf("addr=0x%x, addr+dataLen=0x%x, bufLen=%d\n",
		       addr, addr + dataLen, bufLen);
		return SREC_ERROR_ADDR;
	    }
	    memcpy(buf + addr, dataBytes, dataLen);
	    byteTotal += dataLen;
	    dataRecs++;
	    break;
	case '7':		/* Termination with 4-byte address */
	case '8':		/* Termination with 3-byte address */
	case '9':		/* Termination with 2-byte address */
	    *entry = addr;
	    dataRecs = 0;
	    break;
	default:
	    return SREC_ERROR_FORMAT;
	}
    }

    return byteTotal;
}

char *srecErrmsg(int errcode)
{
    switch (errcode) {
    case SREC_ERROR_NONE:
	return "Success";
    case SREC_ERROR_FORMAT:
	return "File format error";
    case SREC_ERROR_CHECKSUM:
	return "Checksum mismatch";
    case SREC_ERROR_NRECORDS:
	return "Incorrect number of records";
    case SREC_ERROR_TRUNC:
	return "Truncated line";
    case SREC_ERROR_ADDR:
	return "Address out of range";
    }

    return "Unknown error";
}

#ifdef TEST_MAIN

/*
 * Test main program
 */

#define PROM_SIZE	(512 * 1024)

main(int argc, char **argv)
{
    FILE		*fp;
    char		buf[PROM_SIZE];
    int			entry, r;

    if (argc != 2) {
	fprintf(stderr, "Usage: srecLoad file.hex > file.bin\n");
	exit(1);
    }

    if ((fp = fopen(argv[1], "r")) == 0) {
	fprintf(stderr, "srecLoad: could not open %s\n", argv[1]);
	exit(1);
    }

    if ((r = srecLoad(fp, buf, sizeof (buf), &entry)) < 0) {
	fprintf(stderr, "srecLoad: load error %d\n", r);
	exit(1);
    }

    fclose(fp);

    write(1, buf, sizeof (buf));

    fprintf(stderr, "Entry address: 0x%08x\n", (unsigned int) entry);
    exit(0);
}

#endif /* TEST_MAIN */

#if 0
int test_me()
{
}
int test_me1()
{
test_me();
}
#endif
