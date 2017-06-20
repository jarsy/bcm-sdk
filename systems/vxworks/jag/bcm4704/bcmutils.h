/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.

    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * Misc useful os-independent macros and functions.
 *
 * Copyright(c) 2001 Broadcom Corporation
 * $Id: bcmutils.h,v 1.1 2004/02/24 07:47:00 csm Exp $
 */

#ifndef	_bcmutils_h_
#define	_bcmutils_h_

#ifndef MIN
#define	MIN(a, b)		(((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define	MAX(a, b)		(((a)>(b))?(a):(b))
#endif

#define CEIL(x, y)		(((x) + ((y)-1)) / (y))
#define	ROUNDUP(x, y)		((((ulong)(x)+((y)-1))/(y))*(y))
#define	ISALIGNED(a, x)		(((uint)(a) & ((x)-1)) == 0)
#define	ISPOWEROF2(x)		((((x)-1)&(x))==0)
#define	OFFSETOF(type, member)	((uint) &((type *)0)->member)
#define ARRAYSIZE(a)		(sizeof(a)/sizeof(a[0]))

/* bit map related macros */
#ifndef setbit
#define	NBBY	8	/* 8 bits per byte */
#define	setbit(a,i)	((a)[(i)/NBBY] |= 1<<((i)%NBBY))
#define	clrbit(a,i)	((a)[(i)/NBBY] &= ~(1<<((i)%NBBY)))
#define	isset(a,i)	((a)[(i)/NBBY] & (1<<((i)%NBBY)))
#define	isclr(a,i)	(((a)[(i)/NBBY] & (1<<((i)%NBBY))) == 0)
#endif

#define	NBITS(type)	(sizeof (type) * 8)

#define _BCM_U	0x01	/* upper */
#define _BCM_L	0x02	/* lower */
#define _BCM_D	0x04	/* digit */
#define _BCM_C	0x08	/* cntrl */
#define _BCM_P	0x10	/* punct */
#define _BCM_S	0x20	/* white space (space/lf/tab) */
#define _BCM_X	0x40	/* hex digit */
#define _BCM_SP	0x80	/* hard space (0x20) */

extern unsigned char bcm_ctype[];
#define __ismask(x) (bcm_ctype[(int)(unsigned char)(x)])

#define bcm_isalnum(c)	((__ismask(c)&(_BCM_U|_BCM_L|_BCM_D)) != 0)
#define bcm_isalpha(c)	((__ismask(c)&(_BCM_U|_BCM_L)) != 0)
#define bcm_iscntrl(c)	((__ismask(c)&(_BCM_C)) != 0)
#define bcm_isdigit(c)	((__ismask(c)&(_BCM_D)) != 0)
#define bcm_isgraph(c)	((__ismask(c)&(_BCM_P|_BCM_U|_BCM_L|_BCM_D)) != 0)
#define bcm_islower(c)	((__ismask(c)&(_BCM_L)) != 0)
#define bcm_isprint(c)	((__ismask(c)&(_BCM_P|_BCM_U|_BCM_L|_BCM_D|_BCM_SP)) != 0)
#define bcm_ispunct(c)	((__ismask(c)&(_BCM_P)) != 0)
#define bcm_isspace(c)	((__ismask(c)&(_BCM_S)) != 0)
#define bcm_isupper(c)	((__ismask(c)&(_BCM_U)) != 0)
#define bcm_isxdigit(c)	((__ismask(c)&(_BCM_D|_BCM_X)) != 0)

/*
 * Spin at most 'us' microseconds while 'exp' is true.
 * Caller should explicitly test 'exp' when this completes
 * and take appropriate error action if 'exp' is still true.
 */
#define SPINWAIT(exp, us) { \
	uint countdown = (us) + 9; \
	while ((exp) && (countdown >= 10)) {\
		OSL_DELAY(10); \
		countdown -= 10; \
	} \
}

/* generic osl packet queue */
struct pktq {
	void *head;
	void *tail;
	uint  len;
	uint  maxlen; 
};
#define DEFAULT_QLEN	128

#define	pktq_len(q)		((q)->len)
#define	pktq_avail(q)	((q)->maxlen - (q)->len)
#define	pktq_head(q)	((q)->head)
#define	pktq_full(q)	((q)->len >= (q)->maxlen)

/* crc defines */
#define CRC8_INIT_VALUE  0xff		/* Initial CRC8 checksum value */
#define CRC8_GOOD_VALUE  0x9f		/* Good final CRC8 checksum value */
#define CRC16_INIT_VALUE 0xffff		/* Initial CRC16 checksum value */
#define CRC16_GOOD_VALUE 0xf0b8		/* Good final CRC16 checksum value */
#define CRC32_INIT_VALUE 0xffffffff /* Initial CRC32 checksum value */
#define CRC32_GOOD_VALUE 0xdebb20e3 /* Good final CRC32 checksum value */

/* externs */
extern uint bcm_atoi(char *s);
extern ulong bcm_strtoul(char *cp, char **endp, uint base);
extern void deadbeef(char *p, uint len);
extern void prhex(char *msg, uchar *buf, uint len);
extern void prpkt(char *msg, void *drv, void *p0);
extern uint pktcopy(void *drv, void *p, uint offset, int len, uchar *buf);
extern uint pkttotlen(void *drv, void *);
extern uchar *bcm_ether_ntoa(char *ea, char *buf);
extern int bcm_ether_atoe(char *p, char *ea);
extern void bcm_mdelay(uint ms);
extern char *getvar(char *vars, char *name);
extern int getintvar(char *vars, char *name);

extern uint8 crc8(uint8 *p, uint nbytes, uint8 crc);
extern uint16 crc16(uint8 *p, uint nbytes, uint16 crc);
extern uint32 crc32(uint8 *p, uint nbytes, uint32 crc);
#ifdef BCMDBG
extern void testcrc32(void);
#endif
extern uint8 *bcm_parse_tlvs(uint8 *buf, int buflen, uint key);
extern void pktqinit(struct pktq *q, int maxlen);
extern void pktenq(struct pktq *q, void *p, bool lifo);
extern void *pktdeq(struct pktq *q);

#define BCMINTERNAL 1
#ifdef BCMINTERNAL
extern void bcmlog(char *fmt, uint a1, uint a2);
extern void bcmdumplog(uchar *buf, int size);
#else
#define	bcmlog(fmt, a1, a2)
#define	bcmdumplog(buf, size)	*buf = '\0'
#endif

#endif	/* _bcmutils_h_ */
