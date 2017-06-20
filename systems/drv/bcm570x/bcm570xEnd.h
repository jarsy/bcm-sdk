/*
 * $Id: bcm570xEnd.h,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef __BCM570X_END__H
#define __BCM570X_END__H

#include "vxWorks.h"
#include "wdLib.h"
#include "iv.h"
#include "errno.h"
#include "memLib.h"
#include "intLib.h"
#include "vxLib.h"
#include "net/mbuf.h"
#include "net/unixLib.h"
#include "net/protosw.h"
#include "sys/socket.h"
#include "sys/ioctl.h"
#include "net/route.h"
#include "iosLib.h"
#include "errnoLib.h"

#include "cacheLib.h"
#include "logLib.h"
#include "netLib.h"
#include "stdio.h"
#include "stdlib.h"
#include "sysLib.h"

#include "etherLib.h"
#include "net/systm.h"
#include "sys/times.h"
#include "net/if_subr.h"
#include "drv/pci/pciConfigLib.h"
#undef	ETHER_MAP_IP_MULTICAST
#include "etherMultiLib.h"
#include "end.h"
#include "endLib.h"
#include "m2Lib.h"
#include "lstLib.h"
#include "semLib.h"



#ifdef BCM570X_DEBUG_TASK
/*
 * Configuration options.
 */
#define BCM570X_BIG_ENDIAN    1             /* Big endian processor */
#define PRIO_BCM570X_TASK     150           /* Priority of monitor task*/
#define BCM570X_PCI_MEMADDR   "0x80000000"  /* String for PCI mem base addr */
#define BCM570X_DEFAULT_UNIT  "0"           /* Default unit to bind to */
#define BCM570X_DEFAULT_IRQ   "1"           /* IRQ number */
#define BCM570X_DEFAULT_OFFSET "0"         /* Frame shift to get IP alignment*/
#define BCM570X_INIT_STR       BCM570X_DEFAULT_UNIT##":"##\
           BCM570X_PCI_MEMADDR##":"##BCM570X_DEFAULT_IRQ##":"##\
           BCM570X_DEFAULT_OFFSET
#endif
#define	BCM570X_END_STRING     "Broadcom BCM570x Gigabit Ethernet SENS Driver"

/* User-configurable */
#define	BCM570X_END_CLBUFS     (512)	    /* Cluster Buffers */
#define	BCM570X_END_CLBLKS     (512)	    /* Total Cluster Headers */
#define	BCM570X_END_MBLKS      (512)	    /* Total Packet Headers */
#define	BCM570X_END_PK_SZ      2048	    /* Maximum Packet Size */


/* Tagged and Untagged Frame Header lengths */
#define	BCM570X_ENET_UNTAGGED_HDR_LEN		14
#define	BCM570X_ENET_TAGGED_HDR_LEN		18
#define	BCM570X_END_SPEED			10000000	/* 10 Mb/s */
#define	BCM570X_END_SPEED2	                1000000000	/* 1 Gb/s */


/* Define the default tagged protocol id */
#define BCM570X_ENET_DEFAULT_TPID		0x8100
#define BCM570X_ENET_TAG_SIZE                   4
#define BCM570X_VLAN_TAG_OFFSET                 12 /* DA+SA */

/*
 * Extern lib code.
 */

IMPORT STATUS hostAdd
         (
         char  *hostName,  /* host name                             */
         char  *hostAddr   /* host addr in standard Internet format */
         );

IMPORT  STATUS ipAttach
         (
         int   unit,     /* Unit number                     */
         char  *pDevice  /* Device name (i.e. ln, ei etc.). */
         );


IMPORT STATUS ifMaskSet
         (
         char  *ifName,  /* name of interface to set mask for, i.e. ei0 */
         int   netMask   /* subnet mask (e.g. 0xff000000)               */
         );

IMPORT  STATUS ifAddrSet
         (
         char  *ifName,    /* name of interface to configure, i.e. ei0 */
         char  *ifAddr  /* Internet address to assign to interface  */
         );

void icmpstatShow (void);
void ipstatShow(BOOL zero_stats);
void tcpstatShow (void);
void ifShow(char  *ifName);

IMPORT STATUS (* _func_bcm570xEnetAddrGet)
    (char *dev,int unit,unsigned char *pMac);

/* length of time in microsecs to delay */
IMPORT void sysUsecDelay(UINT delay);


/*
 * Number of seconds in one microsecond.
 */
#define SECOND_USEC			(1000000)

/*
 * PCI definitions
 */
#define PCI_CMD_MASK	0xffff0000	/* mask to save status bits */
#define PCI_ANY_ID (~0)

/* See mm.h for more defines */



#endif /* __BCM570X_END_H */
