/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * Misc useful OS-independent routines.
 *
 * $Id: bcmutils.c,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <shared/et/typedefs.h>
#include <shared/et/osl.h>
#include <shared/et/bcmutils.h>
#include <shared/et/bcmendian.h>

#if !defined(VXWORKS) && !defined(__ECOS)

unsigned char bcm_ctype[] = {
	_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,			/* 0-7 */
	_BCM_C,_BCM_C|_BCM_S,_BCM_C|_BCM_S,_BCM_C|_BCM_S,_BCM_C|_BCM_S,_BCM_C|_BCM_S,_BCM_C,_BCM_C,		/* 8-15 */
	_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,			/* 16-23 */
	_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,_BCM_C,			/* 24-31 */
	_BCM_S|_BCM_SP,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,			/* 32-39 */
	_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,			/* 40-47 */
	_BCM_D,_BCM_D,_BCM_D,_BCM_D,_BCM_D,_BCM_D,_BCM_D,_BCM_D,			/* 48-55 */
	_BCM_D,_BCM_D,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,			/* 56-63 */
	_BCM_P,_BCM_U|_BCM_X,_BCM_U|_BCM_X,_BCM_U|_BCM_X,_BCM_U|_BCM_X,_BCM_U|_BCM_X,_BCM_U|_BCM_X,_BCM_U,	/* 64-71 */
	_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,			/* 72-79 */
	_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,			/* 80-87 */
	_BCM_U,_BCM_U,_BCM_U,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,			/* 88-95 */
	_BCM_P,_BCM_L|_BCM_X,_BCM_L|_BCM_X,_BCM_L|_BCM_X,_BCM_L|_BCM_X,_BCM_L|_BCM_X,_BCM_L|_BCM_X,_BCM_L,	/* 96-103 */
	_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,			/* 104-111 */
	_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,			/* 112-119 */
	_BCM_L,_BCM_L,_BCM_L,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_C,			/* 120-127 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		/* 128-143 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		/* 144-159 */
	_BCM_S|_BCM_SP,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,   /* 160-175 */
	_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,_BCM_P,       /* 176-191 */
	_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,       /* 192-207 */
	_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_P,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_U,_BCM_L,       /* 208-223 */
	_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,       /* 224-239 */
	_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_P,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L,_BCM_L        /* 240-255 */
};


static uchar
bcm_toupper(uchar c)
{
    if (bcm_islower(c))
        c -= 'a'-'A';
    return (c);
}

ulong
bcm_strtoul(char *cp, char **endp, uint base)
{
    ulong result, value;

    if (base == 0) {
        if (cp[0] == '0') {
            if ((cp[1] == 'x') || (cp[1] == 'X')) {
                base = 16;
                cp = &cp[2];
            } else {
                base = 8;
                cp = &cp[1];
            }
        } else
            base = 10;
    }

    result = 0;

    while (bcm_isxdigit(*cp) &&
        (value = bcm_isdigit(*cp) ? *cp-'0' : bcm_toupper(*cp)-'A'+10) < base) {
        result = result*base + value;
        cp++;
    }
    if (endp)
        *endp = (char *)cp;

    return (result);
}

uint
bcm_atoi(char *s)
{
    uint n;

    n = 0;

    while (bcm_isdigit(*s))
        n = (n * 10) + *s++ - '0';
    return (n);
}

void
deadbeef(char *p, uint len)
{
    static uchar meat[] = { 0xde, 0xad, 0xbe, 0xef };

    while (len-- > 0) {
        *p = meat[PTR_TO_INT(p) & 3];
        p++;
    }
}

/* pretty hex print a contiguous buffer */
void
prhex(char *msg, uchar *buf, uint nbytes)
{
    char line[256];
    char* p;
    uint i;

    if (msg && (msg[0] != '\0'))
        LOG_CLI((BSL_META("%s: "), msg));

    p = line;
    for (i = 0; i < nbytes; i++) {
        if (i % 16 == 0) {
            p += sprintf(p, "%04d: ", i);	/* line prefix */
        }
        p += sprintf(p, "%02x ", buf[i]);
        if (i % 16 == 15) {
            LOG_CLI((BSL_META("%s\n"), line));	/* flush line */
            p = line;
        }
    }

    /* flush last partial line */
    if (p != line)
        LOG_CLI((BSL_META("%s\n"), line));
}

/* pretty hex print a pkt buffer chain */
void
prpkt(char *msg, void *dev, void *p0)
{
    void *p;

    if (msg && (msg[0] != '\0'))
        LOG_CLI((BSL_META("%s: "), msg));

    for (p = p0; p; p = ET_PKTNEXT(dev, p))
        prhex(NULL, ET_PKTDATA(dev, p), ET_PKTLEN(dev, p));
}

uchar*
bcm_ether_ntoa(char *ea, char *buf)
{
    sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",
        (uchar)ea[0]&0xff, (uchar)ea[1]&0xff, (uchar)ea[2]&0xff,
        (uchar)ea[3]&0xff, (uchar)ea[4]&0xff, (uchar)ea[5]&0xff);
    return ((uchar *) buf);
}

/* parse a xx:xx:xx:xx:xx:xx format ethernet address */
int
bcm_ether_atoe(char *p, char *ea)
{
    int i = 0;

    for (;;) {
        ea[i++] = (char) bcm_strtoul(p, &p, 16);
        if (!*p++ || i == 6)
            break;
    }

    return (i == 6);
}

/* 
 * Traverse a string of 1-byte tag/1-byte length/variable-length value 
 * triples, returning a pointer to the substring whose first element 
 * matches tag
 */
uint8 *
bcm_parse_tlvs(uint8 *buf, int buflen, uint key)
{
    uint8 *cp;
    int totlen;

    cp = buf;
    totlen = buflen;

    /* find tagged parameter */
    while (totlen > 2) {
        uint tag;
        int len;

        tag = *cp;
        len = *(cp +1);
        
        /* validate remaining totlen */
        if ((tag == key) && (totlen >= (len + 2)))
            return (cp);

        cp += (len + 2);
        totlen -= (len + 2);
    }

    return NULL;
}

void
pktqinit(struct pktq *q, int maxlen)
{
    q->head = q->tail = NULL;
    q->maxlen = maxlen;
    q->len = 0;
}

void
pktenq(struct pktq *q, void *p, bool lifo)
{
    PKTSETLINK(p, NULL);

    if (q->tail == NULL) {
        ASSERT(q->head == NULL);
        q->head = q->tail = p;
    }
    else {
        ASSERT(q->head);
        if (lifo) {
            PKTSETLINK(p, q->head);
            q->head = p;
        } else {
            PKTSETLINK(q->tail, p);
            q->tail = p;
        }
    }
    q->len++;
}

void*
pktdeq(struct pktq *q)
{
    void *p;

    if ((p = q->head)) {
        ASSERT(q->tail);
        /* q->head = PKTLINK(p);*/
        PKTSETLINK(p, NULL);
        q->len--;
        if (q->head == NULL)
            q->tail = NULL;
    }
    else {
        ASSERT(q->tail == NULL);
    }

    return (p);
}

#endif

#ifdef BCMINTERNAL

#define	LOGSIZE	256
static struct {
    uint	cycles;
    char	*fmt;
    uint	a1;
    uint	a2;
} logtab[LOGSIZE];
static uint logi = 0;

#ifndef OSL_GETCYCLES
#define	OSL_GETCYCLES(x)	(x = 0)
#endif

void
bcmlog(char *fmt, uint a1, uint a2)
{
    static uint last = 0;
    uint cycles, i;

    OSL_GETCYCLES(cycles);

    /* wrap? */
    if (cycles < last)
        last = cycles;

    i = logi;
    logtab[i].cycles = cycles - last;
    logtab[i].fmt = fmt;
    logtab[i].a1 = a1;
    logtab[i].a2 = a2;

    logi = ++i % LOGSIZE;

    last = cycles;
}

void
bcmdumplog(uchar *buf, int size)
{
    uchar *limit;
    uint i;
    
    if (buf == NULL) {
        return;
    }

    limit = buf + size - 80;
    *buf = '\0';

    /* print in chronological order */
    for (i = (logi + 1) % LOGSIZE; (i != logi) && (buf < limit); 
        i = (++i % LOGSIZE)) {
        if (logtab[i].fmt == NULL)
            continue;
        buf += sprintf(buf, "%d\t", logtab[i].cycles);
        buf += sprintf(buf, logtab[i].fmt, logtab[i].a1, logtab[i].a2);
        buf += sprintf(buf, "\n");
    }
}
#endif	/* BCMINTERNAL */



