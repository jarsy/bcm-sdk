/* bootConfig.c - system configuration module for boot ROMs */

/* Copyright 1984 - 2001 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/* $Id: bootConfig.c,v 1.4 2011/07/21 16:14:57 yshtil Exp $
modification history
--------------------
*/

/*
DESCRIPTION
This is the WRS-supplied configuration module for the VxWorks boot ROM.
It is a stripped-down version of usrConfig.c, having no VxWorks shell or
debugging facilities.  Its primary function is to load an object module
over the network with either RSH or FTP.  Additionally, a simple set of
single letter commands is provided for displaying and modifying memory
contents.  Use this module as a starting point for placing applications 
in ROM.
*/

#include "vxWorks.h"
#include "bootEcoffLib.h"
#include "bootElfLib.h"
#include "bootLib.h"
#include "bootLoadLib.h"
#include "bootpLib.h"
#include "bsdSockLib.h"
#include "cacheLib.h"
#include "ctype.h"
#include "dosFsLib.h"
#include "errno.h"
#include "errnoLib.h"
#include "fcntl.h"
#include "fioLib.h"
#include "ftpLib.h"
#include "hostLib.h"
#include "icmpLib.h"
#include "ifLib.h"
#include "if_sl.h"
#include "inetLib.h"
#include "intLib.h"
#include "ioLib.h"
#include "iosLib.h"
#include "loadAoutLib.h"
#include "loadCoffLib.h"
#include "loadLib.h"
#include "logLib.h"
#include "memLib.h"
#include "msgQLib.h"
#include "netLib.h"
#include "pipeDrv.h"
#include "proxyLib.h"
#include "qLib.h"
#include "qPriBMapLib.h"
#include "rebootLib.h"
#include "remLib.h"
#include "rngLib.h"
#include "routeLib.h"
#include "semLib.h"
#include "sockLib.h"
#include "stdio.h"
#include "string.h"
#include "sysLib.h"
#include "sysSymTbl.h"
#include "taskHookLib.h"
#include "taskLib.h"
#include "tftpLib.h"
#include "tickLib.h"
#include "trcLib.h"
#include "unistd.h"
#include "version.h"
#include "wdLib.h"
#include "net/if.h"
#include "net/mbuf.h"
#include "netinet/if_ether.h"
#include "drv/netif/smNetLib.h"
#include "ipProto.h"
#include "private/kernelLibP.h"
#include "private/workQLibP.h"
#include "config.h"
#include "scsiLib.h"

#if 1
#include "vxLib.h"
#endif
#ifdef BROADCOM_BSP
#include "drv/pci/pciConfigLib.h"
#include "memDrv.h"
#include "ftpXfer2.h"
#include "srecLoad.h"
#endif

#ifdef  INCLUDE_FLASH_BOOT
#include "flashDrvLib.h"        /* Flash driver for MBZ */
#include "flashFsLib.h"     /* Flash filesystem driver */
#endif  /* INCLUDE_FLASH_BOOT */

#ifdef	INCLUDE_PPP
#include "pppLib.h"
#endif	/* INCLUDE_PPP */
#include "bcm56218.h"

#ifdef INCLUDE_DHCPC
#include "dhcpcBootLib.h"
#include "dhcp/dhcpc.h"
#include "dhcp/dhcpcCommonLib.h"
#endif

#if	defined(INCLUDE_STREAMS) || defined(INCLUDE_STREAMS_ALL)
#include "strmLib.h"
#endif	/* INCLUDE_STREAMS */


/* defines */

#define STREQ(A, B) (strcmp(A, B) == 0 ? 1 : 0)
#define TIMEOUT		7	/* number of seconds before auto-boot */
#define MAX_LINE        160	/* max line length for input to 'm' routine */
#define RSHD		514	/* rshd service */
#define DEC		FALSE	/* getArg parameters */
#define HEX		TRUE
#define OPT		TRUE
#define MAX_ADR_SIZE 	6 
#define DOS_ID_OFFSET                   3
#define FIRST_PARTITION_SECTOR_OFFSET   (0x1be + 8)
#define VXDOS                           "VXDOS"
#define VXEXT                           "VXEXT"

/* DO NOT ADD ANYTHING BEFORE THE FIRST ROUTINE compressedEntry() */

void		usrInit ();
IMPORT void 	sysInitAlt ();

#ifdef BROADCOM_BSP
IMPORT void sysLedDsply(const char* msg);
#endif

#if	(CPU_FAMILY==MIPS)
IMPORT void 	sysGpInit ();
#endif	/* (CPU_FAMILY==MIPS) */

#ifdef  INCLUDE_NETWORK
#ifdef ETHERNET_ADR_SET
void		mEnet (char *);
void 		sysEnetAddrGet ();
void 		sysEnetAddrSet ();
#endif  /* ETHERNET_ADR_SET */
#endif  /* INCLUDE_NETWORK */

#ifdef INCLUDE_END
LOCAL void* findCookie(int unitNo, char* devName);
#endif /* INCLUDE_END */

#ifdef BROADCOM_BSP
/* Jimmy: might delete dosFsMemPartInitSize */
extern unsigned int dosFsMemPartInitSize;

#define DEFLATED_EXT            ".Z"
#define DEFLATED_EXT_LEN        (sizeof (DEFLATED_EXT) - 1)
#define DEFLATED_FILE(fileName) \
        (strlen(fileName) >= DEFLATED_EXT_LEN && \
         ! strcmp((fileName) + strlen(fileName) - DEFLATED_EXT_LEN, \
                  DEFLATED_EXT))
#endif  /* BROADCOM_BSP */

/*******************************************************************************
*
* compressedEntry - compressed entry point after decompression 
*
* This routine is the entry point after the bootroms decompress, if
* compression is utilized.  This routine must be the first item of the
* text segment of this file.  With ANSI C, strings will appear in text
* segment so one should avoid entering strings, routines, or anything
* else that might displace this routine from base of the text segment.
*
* It is unwise to add functionality to this routine without due cause.
* We are in the prehistoric period of system initialization.  
*
* NOMANUAL
*/

void compressedEntry 
    (
    int startType
    )
    {

#if (CPU_FAMILY==MIPS)
#if __GNUC__
    __asm volatile (".extern _gp,0; la $gp,_gp");
#endif
#endif

#if	(CPU_FAMILY==I960)
    sysInitAlt (startType);		/* jump to the i960 entry point */
#else
    usrInit (startType);		/* all others procede below */
#endif
    }

#ifdef INCLUDE_END
#include "end.h"
#include "muxLib.h"
#include "muxTkLib.h"
#include "configNet.h"
#include "m2Lib.h"

IMPORT int 	ipAttach ();
IMPORT END_TBL_ENTRY endDevTbl[];

typedef struct cookie_tbl
    {
    int unitNo;
    char devName[END_NAME_MAX];
    void* pCookie;
    }COOKIE_TBL;

COOKIE_TBL cookieTbl[32];

#ifndef IP_MAX_UNITS 
#define IP_MAX_UNITS 1
#endif

#ifndef MUX_MAX_BINDS
#define MUX_MAX_BINDS 8
#endif

IP_DRV_CTRL ipDrvCtrl [IP_MAX_UNITS];
int ipMaxUnits = IP_MAX_UNITS;
END_OBJ*   pEnd;
void*   pCookie;
M2_INTERFACETBL endM2Tbl;
#else	/* INCLUDE_END not defined */
IP_DRV_CTRL  	ipDrvCtrl[1];
#endif 	/* INCLUDE_END */

/* Wind kernel configuration facility */

#undef	INCLUDE_SHOW_ROUTINES		/* keep out kernel show routines */
#include "usrKernel.c"			/* kernel configuration facility */


/* imports */

IMPORT char	edata [];		/* defined by the loader */
IMPORT char	end [];			/* defined by the loader */

#define FREE_MEM_START_ADRS FREE_RAM_ADRS

#ifdef  INCLUDE_INITIAL_MEM_ALLOCATION
#define MEM_POOL_START_ADRS \
	    (ROUND_UP(FREE_MEM_START_ADRS, (INITIAL_MEM_ALIGNMENT)) + \
	    (INITIAL_MEM_SIZE))
#else   /* INCLUDE_INITIAL_MEM_ALLOCATION */
#define MEM_POOL_START_ADRS FREE_MEM_START_ADRS
#endif  /* INCLUDE_INITIAL_MEM_ALLOCATION */



#ifdef  INCLUDE_TSFS_BOOT		/* boot via Target Server File System */

#if ((WDB_COMM_TYPE == WDB_COMM_SERIAL) && \
	(CONSOLE_TTY == NONE || CONSOLE_TTY == WDB_TTY_CHANNEL))
#define INCLUDE_TSFS_BOOT_VIO_CONSOLE	/* needed for Target Server Console */
#endif

#define INCLUDE_WDB			/* WDB agent needed for TSFS Boot */
#define INCLUDE_WDB_TSFS		/* target-server file system */

#undef  INCLUDE_WDB_BANNER		/* print banner after agent starts */
#undef 	INCLUDE_WDB_TTY_TEST		/* test serial line communcation */
#undef  INCLUDE_WDB_START_NOTIFY	/* notify the host of task creation */
#undef  INCLUDE_WDB_USER_EVENT		/* user events handling */
#undef  INCLUDE_WDB_CTXT		/* context control */
#undef  INCLUDE_WDB_FUNC_CALL		/* spawn function as separate task */
#undef  INCLUDE_WDB_GOPHER		/* gopher info gathering */
#undef  INCLUDE_WDB_EXIT_NOTIFY		/* notify the host of task exit */
#undef  INCLUDE_WDB_REG			/* get/set hardware registers */
#undef  INCLUDE_WDB_EVENTPOINTS		/* eventpoints handling */
#undef  INCLUDE_WDB_MEM			/* optional memory services */
#undef  INCLUDE_WDB_BP			/* breakpoint support */
#include "wdb/wdbEvtLib.h"
#include "../../src/config/usrWdb.c"    /* WDB agent configuration */

#else	/* INCLUDE_TSFS_BOOT not defined */
#undef INCLUDE_WDB
#endif  /* INCLUDE_TSFS_BOOT */

#ifdef  INCLUDE_NETWORK

/* forward declarations */
LOCAL STATUS usrNetProtoInit (void);

IMPORT int	lnEbsaattach ();
IMPORT int	oliattach ();
IMPORT int	dcattach ();
IMPORT int	eglattach ();
IMPORT int	eiattach ();
IMPORT int	feiattach ();
IMPORT int	exattach ();
IMPORT int	enpattach ();
IMPORT int	ieattach ();
IMPORT int	ilacattach ();
IMPORT int	lnattach ();
IMPORT int	lnsgiattach ();
IMPORT int	nicattach ();
IMPORT int	nicEvbattach ();
IMPORT int	medattach ();
IMPORT int      elcattach ();
IMPORT int      ultraattach ();
IMPORT int      eexattach ();
IMPORT int      eltattach ();
IMPORT int      eneattach ();
IMPORT int      esmcattach ();
IMPORT int	quattach ();
IMPORT int	loattach ();
IMPORT int	snattach ();
IMPORT int	fnattach ();
IMPORT STATUS	slipInit ();
IMPORT int	ifreset ();
IMPORT void	if_dettach ();
IMPORT u_long	in_netof ();
IMPORT struct ifnet *	ifunit ();
IMPORT int	pcmciaattach ();

#ifdef  NETIF_USR_DECL
    NETIF_USR_DECL 		/* additional declarations, from BSP */
#endif

#ifdef	INCLUDE_IF_USR
IMPORT int IF_USR_ATTACH ();
#endif	/* INCLUDE_IF_USR */

LOCAL NETIF netIf [] =
    {
#ifdef  NETIF_USR_ENTRIES
    NETIF_USR_ENTRIES		/* additional entries, from BSP */
#endif

#ifdef	INCLUDE_IF_USR
        { IF_USR_NAME, IF_USR_ATTACH, IF_USR_ARG1, IF_USR_ARG2, IF_USR_ARG3,
          IF_USR_ARG4, IF_USR_ARG5, IF_USR_ARG6, IF_USR_ARG7, IF_USR_ARG8 },
#endif	/* INCLUDE_IF_USR */
#ifdef  INCLUDE_LNEBSA
	{ "lnEbsa", lnEbsaattach, (char*)IO_ADRS_LNEBSA, INT_VEC_LNEBSA,
	  INT_LVL_LNEBSA, LNEBSA_POOL_ADRS, LNEBSA_POOL_SIZE,
	  LNEBSA_DATA_WIDTH, LNEBSA_MODE, LNEBSA_DMA_CHAN },
#endif  /* INCLUDE_LNEBSA */
#ifdef  INCLUDE_OLI
	{ "oli", oliattach, (char*)IO_ADRS_NISA_BASE,
	  IO_ADRS_NISA_PCMCIA, IO_ADRS_NISA_PCMEM,
	  INT_VEC_PCMCIA_A, INT_LVL_PCMCIA_A,
	  INT_VEC_PCMCIA_B, INT_LVL_PCMCIA_B },
#endif
#ifdef	INCLUDE_DC
	{ "dc", dcattach, (char*)IO_ADRS_DC, INT_VEC_DC, INT_LVL_DC,
	  DC_POOL_ADRS, DC_POOL_SIZE, DC_DATA_WIDTH, DC_RAM_PCI_ADRS,
	  DC_MODE },
#endif	/* INCLUDE_DC */
#ifdef	INCLUDE_EGL
	{ "egl", eglattach, (char*)IO_ADRS_EGL, INT_VEC_EGL, INT_LVL_EGL },
#endif	/* INCLUDE_EGL */
#ifdef	INCLUDE_EI
        { "ei", eiattach, (char*)INT_VEC_EI, EI_SYSBUS, EI_POOL_ADRS, 0, 0},
#endif	/* INCLUDE_EI */
#ifdef	INCLUDE_FEI
        { "fei", feiattach, (char*)FEI_POOL_ADRS, 0, 0, 0, 0},
#endif	/* INCLUDE_FEI */
#ifdef  INCLUDE_EX
        { "ex", exattach, (char*)IO_ADRS_EX, INT_VEC_EX, INT_LVL_EX,
	  IO_AM_EX_MASTER, IO_AM_EX },
#endif	/* INCLUDE_EX */
#ifdef  INCLUDE_ENP
        { "enp", enpattach, (char*)IO_ADRS_ENP, INT_VEC_ENP, INT_LVL_ENP,
	  IO_AM_ENP },
#endif	/* INCLUDE_ENP */
#ifdef  INCLUDE_IE
        { "ie", ieattach, (char*)IO_ADRS_IE, INT_VEC_IE, INT_LVL_IE },
#endif	/* INCLUDE_IE */
#ifdef	INCLUDE_ILAC
	{ "ilac", ilacattach, (char*)IO_ADRS_ILAC, INT_VEC_ILAC},
#endif	/* INCLUDE_ILAC */
#ifdef  INCLUDE_LN
        { "ln", lnattach, (char*)IO_ADRS_LN, INT_VEC_LN, INT_LVL_LN,
          LN_POOL_ADRS, LN_POOL_SIZE, LN_DATA_WIDTH, LN_PADDING,
	  LN_RING_BUF_SIZE },
#endif	/* INCLUDE_LN */
#ifdef  INCLUDE_LNSGI
        { "lnsgi", lnsgiattach, (char*)IO_ADRS_LNSGI, INT_VEC_LNSGI,
          INT_LVL_LNSGI, LNSGI_POOL_ADRS, LNSGI_POOL_SIZE, LNSGI_DATA_WIDTH,
          LNSGI_PADDING, LNSGI_RING_BUF_SIZE },
#endif  /* INCLUDE_LNSGI */
#ifdef  INCLUDE_NIC
        { "nic", nicattach, (char*)IO_ADRS_NIC, INT_VEC_NIC, INT_LVL_NIC },
#endif	/* INCLUDE_NIC */
#ifdef  INCLUDE_NIC_EVB
        { "nicEvb", nicEvbattach, (char*)IO_ADRS_NIC,INT_VEC_NIC,INT_LVL_NIC },
#endif	/* INCLUDE_NIC_EVB */
#ifdef  INCLUDE_MED
        { "med", medattach, (char*)IO_ADRS_DBETH, INT_VEC_DBETH, INT_LVL_DBETH},
#endif	/* INCLUDE_MED */
#ifdef  INCLUDE_ELC
	{ "elc", elcattach, (char*)IO_ADRS_ELC, INT_VEC_ELC, INT_LVL_ELC,
	  MEM_ADRS_ELC, MEM_SIZE_ELC, CONFIG_ELC},
#endif  /* INCLUDE_ELC */
#ifdef  INCLUDE_ULTRA
	{ "ultra", ultraattach, (char*)IO_ADRS_ULTRA, INT_VEC_ULTRA,
	  INT_LVL_ULTRA, MEM_ADRS_ULTRA, MEM_SIZE_ULTRA, CONFIG_ULTRA},
#endif  /* INCLUDE_ULTRA */
#ifdef  INCLUDE_EEX
	{ "eex", eexattach, (char*)IO_ADRS_EEX, INT_VEC_EEX, INT_LVL_EEX,
	  NTFDS_EEX, CONFIG_EEX},
#endif  /* INCLUDE_EEX */
#ifdef  INCLUDE_ELT
	{ "elt", eltattach, (char*)IO_ADRS_ELT, INT_VEC_ELT, INT_LVL_ELT,
	  NRF_ELT, CONFIG_ELT},
#endif  /* INCLUDE_ELT */
#ifdef  INCLUDE_ENE
	{ "ene", eneattach, (char*)IO_ADRS_ENE, INT_VEC_ENE, INT_LVL_ENE},
#endif  /* INCLUDE_ELT */
#ifdef  INCLUDE_ESMC
	{ "esmc", esmcattach, (char*)IO_ADRS_ESMC, INT_VEC_ESMC, INT_LVL_ESMC,
	  CONFIG_ESMC, RX_MODE_ESMC},
#endif  /* INCLUDE_ESMC */
#ifdef  INCLUDE_QU
	{ "qu", quattach, (char*)IO_ADRS_QU_EN, INT_VEC_QU_EN, QU_EN_SCC,
          QU_EN_TX_BD, QU_EN_RX_BD, QU_EN_TX_OFF, QU_EN_RX_OFF, QU_EN_MEM},
#endif  /* INCLUDE_QU */
#ifdef  INCLUDE_SN
        { "sn", snattach, (char*)IO_ADRS_SN, INT_VEC_SN },
#endif	/* INCLUDE_SN */
#ifdef  INCLUDE_FN
        { "fn", fnattach },
#endif  /* INCLUDE_FN */

#ifdef  INCLUDE_SM_NET
        { "sm", smNetAttach, 0, 0, 0, 0,  0, 0 },
#endif  /* INCLUDE_SM_NET */

#ifdef	INCLUDE_PCMCIA
	{ "pcmcia", pcmciaattach, 0, 0, 0, 0, 0, 0 },
#endif	/* INCLUDE_PCMCIA */

#ifdef  INCLUDE_PPP
	{"ppp", 0, 0, 0, 0, 0},
#endif  /* INCLUDE_PPP */
#ifdef  INCLUDE_SLIP
	{ "sl", 0, 0, 0, 0, 0 },
#endif	/* INCLUDE_SLIP */
	{ "lo", loattach, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 },
    };

#endif  /* INCLUDE_NETWORK */

/* global variables */

SYMTAB_ID	sysSymTbl;
int		consoleFd;		/* fd of initial console device */
char		consoleName [20];	/* console device name, eg. "/tyCo/0" */
int		sysStartType;		/* BOOT_CLEAR, BOOT_NO_AUTOBOOT, ... */

BOOL		scsiInitialized      = FALSE;
int		bootCmdTaskPriority  = 1;
int		bootCmdTaskOptions   = VX_SUPERVISOR_MODE;
int		bootCmdTaskStackSize = 7000;
UINT8           bootActiveString = 0;

#ifdef  INCLUDE_NETWORK

/* network protocol configuration parameters */

#ifdef INCLUDE_TCP
TCP_CFG_PARAMS tcpCfgParams =   /* tcp configuration parameters */
    {
    TCP_FLAGS_DFLT,             /* include rfc1323 support */
    TCP_SND_SIZE_DFLT,          /* default send buffer size */
    TCP_RCV_SIZE_DFLT,          /* default recv buffer size */
    TCP_CON_TIMEO_DFLT,         /* initial connection time out */
    TCP_REXMT_THLD_DFLT,        /* retransmit threshold */
    TCP_MSS_DFLT,               /* default maximum segment size */
    TCP_RND_TRIP_DFLT,          /* default round trip time */
    TCP_IDLE_TIMEO_DFLT,        /* idle timeouts before first probe */
    TCP_MAX_PROBE_DFLT          /* max no. probes before dropping */
    };
#endif /* INCLUDE_TCP */

#ifdef INCLUDE_UDP
UDP_CFG_PARAMS udpCfgParams =   /* udp configuration parameters */
    {
    UDP_FLAGS_DFLT,
    UDP_SND_SIZE_DFLT,          /* send buffer size */
    UDP_RCV_SIZE_DFLT           /* recv buffer size */
    };
#endif /* INCLUDE_UDP */

#ifdef INCLUDE_ICMP
ICMP_CFG_PARAMS icmpCfgParams = /* icmp configuration parameters */
    {
    ICMP_FLAGS_DFLT             /* no icmp mask replies by default */
    };
#endif  /* INCLUDE_ICMP */

IP_CFG_PARAMS ipCfgParams =     /* ip configuration parameters */
    {
    IP_FLAGS_DFLT,              /* default ip flags */
    IP_TTL_DFLT,                /* ip default time to live */
    IP_QLEN_DFLT,               /* default ip intr queue len */
    IP_FRAG_TTL_DFLT            /* default ip fragment time to live */
    };

/* network buffers configuration */

/*
 * mBlk, clBlk configuration table for network stack data pool.
 * Only used for data transfer in the network stack.
 */
M_CL_CONFIG mClBlkConfig =
    {
    /* 
    no. mBlks		no. clBlks		memArea		memSize
    -----------		----------		-------		-------
    */
    NUM_NET_MBLKS_MIN, 	NUM_CL_BLKS_MIN,	NULL, 		0
    };

/*
 * network stack data cluster pool configuration table
 * Only used for data transfer in the network stack.
 */
CL_DESC clDescTbl [] =
    {
    /*
    clusterSize     num             	memArea         memSize
    -----------     ----            	-------         -------
    */
    {64,            NUM_64_MIN,		NULL,           0},
    {128,           NUM_128_MIN,        NULL,           0},
    {256,           NUM_256_MIN,        NULL,           0},
    {512,           NUM_512_MIN,        NULL,           0},
    {1024,          NUM_1024_MIN,       NULL,           0},
    };
int clDescTblNumEnt = (NELEMENTS(clDescTbl));

/*
 * mBlk, clBlk configuration table for network stack system pool.
 * Used for network stack system structures such as routes, sockets,
 * protocol control blocks, interface addresses, mulitcast addresses,
 * and multicast routing entries.
 */
M_CL_CONFIG sysMclBlkConfig = 
    {
    /* 
    no. mBlks		no. clBlks		memArea		memSize
    -----------		----------		-------		-------
    */
    NUM_SYS_MBLKS_MIN, 	NUM_SYS_CL_BLKS_MIN,	NULL, 		0
    };

/*
 * network stack system cluster pool configuration table
 * Used for network stack system structures such as routes, sockets,
 * protocol control blocks, interface addresses, mulitcast addresses,
 * and multicast routing entries.
 */
CL_DESC sysClDescTbl [] = 		
    {
    /* 
    clusterSize		num			memArea		memSize
    -----------		----			-------		-------
    */
    {64,		NUM_SYS_64_MIN,		NULL,		0},
    {128,		NUM_SYS_128_MIN,	NULL,		0},
    {256,		NUM_SYS_256_MIN,	NULL,		0},
    {512,		NUM_SYS_512_MIN,	NULL,		0},
    }; 
int sysClDescTblNumEnt = (NELEMENTS(sysClDescTbl));

#if defined(INCLUDE_STREAMS) || defined(INCLUDE_STREAMS_ALL)

#ifdef  STREAMS_PROTO_INIT_RTN
FUNCPTR strmProtoInitRtn = (FUNCPTR) STREAMS_PROTO_INIT_RTN;
#endif  /* STREAMS_PROTO_INIT_RTN */

#endif /* INCLUDE_STREAMS || INCLUDE_STREAMS_ALL */

#ifdef  INCLUDE_PPP

#ifndef PPP_OPTIONS_FLAGS
#define PPP_OPTIONS_FLAGS ((PPP_OPT_NO_ALL << PPP_S_NO_ALL)  | \
    (PPP_OPT_PASSIVE_MODE << PPP_S_PASSIVE_MODE)             | \
    (PPP_OPT_SILENT_MODE << PPP_S_SILENT_MODE)               | \
    (PPP_OPT_DEFAULTROUTE << PPP_S_DEFAULTROUTE)             | \
    (PPP_OPT_PROXYARP << PPP_S_PROXYARP)                     | \
    (PPP_OPT_IPCP_ACCEPT_LOCAL << PPP_S_IPCP_ACCEPT_LOCAL)   | \
    (PPP_OPT_IPCP_ACCEPT_REMOTE << PPP_S_IPCP_ACCEPT_REMOTE) | \
    (PPP_OPT_NO_IP << PPP_S_NO_IP)                           | \
    (PPP_OPT_NO_ACC << PPP_S_NO_ACC)                         | \
    (PPP_OPT_NO_PC << PPP_S_NO_PC)                           | \
    (PPP_OPT_NO_VJ << PPP_S_NO_VJ)                           | \
    (PPP_OPT_NO_VJCCOMP << PPP_S_NO_VJCCOMP)                 | \
    (PPP_OPT_NO_ASYNCMAP << PPP_S_NO_ASYNCMAP)               | \
    (PPP_OPT_NO_MN << PPP_S_NO_MN)                           | \
    (PPP_OPT_NO_MRU << PPP_S_NO_MRU)                         | \
    (PPP_OPT_NO_PAP << PPP_S_NO_PAP)                         | \
    (PPP_OPT_NO_CHAP << PPP_S_NO_CHAP)                       | \
    (PPP_OPT_REQUIRE_PAP << PPP_S_REQUIRE_PAP)               | \
    (PPP_OPT_REQUIRE_CHAP << PPP_S_REQUIRE_CHAP)             | \
    (PPP_OPT_LOGIN << PPP_S_LOGIN)                           | \
    (PPP_OPT_DEBUG << PPP_S_DEBUG)                           | \
    (PPP_OPT_DRIVER_DEBUG << PPP_S_DRIVER_DEBUG))
#endif  /* PPP_OPTIONS_FLAGS */
 
PPP_OPTIONS pppOptions =
    {
    PPP_OPTIONS_FLAGS,		/* flags field */
    PPP_STR_ASYNCMAP,		/* Set the desired async map */
    PPP_STR_ESCAPE_CHARS,	/* Set chars to escape on transmission */
    PPP_STR_VJ_MAX_SLOTS,	/* Set maximum VJ compression header slots */
    PPP_STR_NETMASK,		/* Set netmask value for negotiation */
    PPP_STR_MRU,		/* Set MRU value for negotiation */
    PPP_STR_MTU,		/* Set MTU value for negotiation */
    PPP_STR_LCP_ECHO_FAILURE,	/* Set max # consecutive LCP echo failures */
    PPP_STR_LCP_ECHO_INTERVAL,	/* Set time for LCP echo requests */
    PPP_STR_LCP_RESTART,	/* Set timeout for LCP */
    PPP_STR_LCP_MAX_TERMINATE,	/* Set max # xmits for LCP term-reqs */
    PPP_STR_LCP_MAX_CONFIGURE,	/* Set max # xmits for LCP conf-reqs */
    PPP_STR_LCP_MAX_FAILURE,	/* Set max # conf-naks for LCP */
    PPP_STR_IPCP_RESTART,	/* Set timeout for IPCP */
    PPP_STR_IPCP_MAX_TERMINATE,	/* Set max # xmits for IPCP term-reqs */
    PPP_STR_IPCP_MAX_CONFIGURE,	/* Set max # xmits for IPCP conf-reqs */
    PPP_STR_IPCP_MAX_FAILURE,	/* Set max # conf-naks for IPCP */
    PPP_STR_LOCAL_AUTH_NAME,	/* Set local name for authentication */
    PPP_STR_REMOTE_AUTH_NAME,	/* Set remote name for authentication */
    PPP_STR_PAP_FILE,		/* Set the PAP secrets file */
    PPP_STR_PAP_USER_NAME,	/* Set username for PAP auth with peer */
    PPP_STR_PAP_PASSWD,		/* Set password for PAP auth with peer */
    PPP_STR_PAP_RESTART,	/* Set timeout for PAP */
    PPP_STR_PAP_MAX_AUTHREQ,	/* Set max # xmits for PAP auth-reqs */
    PPP_STR_CHAP_FILE,		/* Set the CHAP secrets file */
    PPP_STR_CHAP_RESTART,	/* Set timeout for CHAP */
    PPP_STR_CHAP_INTERVAL,	/* Set interval for CHAP rechallenge */
    PPP_STR_CHAP_MAX_CHALLENGE	/* Set max # xmits for CHAP challenge */
    };
 
#endif  /* INCLUDE_PPP */

#endif  /* INCLUDE_NETWORK */

/* forward declarations */

#ifdef __STDC__

void 		usrRoot (char *pMemPoolStart, unsigned memPoolSize);
void 		usrClock (void);
void		usrKernelInit (void);
LOCAL void	bootCmdLoop (void);
LOCAL char	autoboot (int timeout);
LOCAL void	printBootLogo (void);
LOCAL void	bootHelp (void);
LOCAL STATUS	bootLoad (char *bootString, FUNCPTR *pEntry);
LOCAL void	go (FUNCPTR entry);
LOCAL void	m (char *adrs);
LOCAL void	d (char *adrs, int nwords);
LOCAL void	bootExcHandler (int tid);
LOCAL void	skipSpace (char **strptr);
LOCAL void	printExcMsg (char *string);
LOCAL STATUS	getArg (char **ppString, int *pValue, BOOL defaultHex,
			BOOL optional);
LOCAL void	usrBootLineInit (int startType);
LOCAL STATUS	usrBootLineCrack (char *bootString, BOOT_PARAMS *pParams);

#ifdef  INCLUDE_NETWORK
LOCAL STATUS	netLoad (char *hostName, char *fileName, char *usr, 
			 char *passwd, FUNCPTR *pEntry);
#ifdef BROADCOM_BSP
LOCAL STATUS    netFlash(char *bootString);
#endif
LOCAL void	netifAdrsPrint (char *ifname);
LOCAL STATUS	checkInetAddrField (char *pInetAddr, BOOL subnetMaskOK);
LOCAL STATUS	usrNetIfAttach (char *devName, int unitNum, char *inetAdrs);
LOCAL STATUS	usrNetIfConfig (char *devName, int unitNum, char *inetAdrs, 
                                char *inetName, int netmask);
LOCAL STATUS	usrBpInit (char *devName, int unitNum, u_long startAddr);
LOCAL STATUS	usrSlipInit (char *pBootDev, int unitNum, char *localAddr, 
                             char *peerAddr);
LOCAL STATUS    usrPPPInit (char *pBootDev, int unitNum, char *localAddr, 
                            char *peerAddr);
LOCAL STATUS	bootpGet (char *pNetDev, char *pBootDevAddr, char *pBootFile,
			  char *pHostAddr, int *pMask);

#ifdef INCLUDE_DHCPC
LOCAL STATUS 	dhcpGet (char *pNetDev, char *pBootDevAddr, char *pBootFile,
                         char *pHostAddr, int  *pMask,
                         DHCP_LEASE_DATA *pDhcpLease);
#endif

#if defined(INCLUDE_STREAMS) || defined(INCLUDE_STREAMS_ALL)
LOCAL STATUS    usrStrmInit (void);
#endif /* INCLUDE_STREAMS || INCLUDE_STREAMS_ALL */
#endif  /* INCLUDE_NETWORK */

#ifdef	INCLUDE_FLASH_BOOT
LOCAL STATUS flashLoad(char    *fileName, FUNCPTR *pEntry);
#endif	/* INCLUDE_FLASH_BOOT */

#ifdef	INCLUDE_SCSI_BOOT
LOCAL STATUS	scsiLoad (int bootDevId, int bootDevLUN, char *fileName,
		          FUNCPTR *pEntry);
#endif	/* INCLUDE_SCSI_BOOT */

#ifdef  INCLUDE_FD
LOCAL STATUS    fdLoad (int drive, int type, char *fileName, FUNCPTR *pEntry);
#endif  /* INCLUDE_FD */

#ifdef  INCLUDE_IDE
LOCAL STATUS    ideLoad (int drive, int type, char *fileName, FUNCPTR *pEntry);
#endif  /* INCLUDE_IDE */

#ifdef	INCLUDE_ATA
LOCAL STATUS	ataLoad (int ctrl, int drive, char *fileName, FUNCPTR *pEntry);
#endif	/* INCLUDE_ATA */

#ifdef	INCLUDE_PCMCIA
LOCAL STATUS	pcmciaLoad (int sock, char *fileName, FUNCPTR *pEntry);
#endif	/* INCLUDE_PCMCIA */

#ifdef	INCLUDE_TFFS
LOCAL STATUS    tffsLoad (int drive, int removable, char *fileName, 
			  FUNCPTR *pEntry);
#endif	/* INCLUDE_TFFS */

#ifdef	INCLUDE_TSFS_BOOT
LOCAL STATUS	tsfsLoad (char * fileName, FUNCPTR * pEntry);
LOCAL void	wdbRebootEventGet (void * pNode, WDB_EVT_DATA *	pEvtData);
#endif	/* INCLUDE_TSFS_BOOT */

#else

void		usrRoot ();
void		usrClock ();
void		usrKernelInit ();
LOCAL void	bootCmdLoop ();
LOCAL char	autoboot ();
LOCAL void	printBootLogo ();
LOCAL void	bootHelp ();
LOCAL STATUS	bootLoad ();
LOCAL void	go ();
LOCAL void	m ();
LOCAL void	d ();
LOCAL void	bootExcHandler ();
LOCAL void	skipSpace ();
LOCAL void	printExcMsg ();
LOCAL STATUS	getArg ();
LOCAL void	usrBootLineInit ();
LOCAL STATUS	usrBootLineCrack ();

#ifdef  INCLUDE_NETWORK
LOCAL STATUS	netLoad ();
LOCAL void	netifAdrsPrint ();
LOCAL STATUS	checkInetAddrField ();
LOCAL STATUS	usrNetIfAttach ();
LOCAL STATUS	usrNetIfConfig ();
LOCAL STATUS	usrBpInit ();
LOCAL STATUS	usrSlipInit ();
LOCAL STATUS	usrPPPInit ();
LOCAL STATUS	bootpGet ();

#if defined(INCLUDE_STREAMS) || defined(INCLUDE_STREAMS_ALL)
LOCAL STATUS	usrStrmInit ();
#endif /* INCLUDE_STREAMS || INCLUDE_STREAMS_ALL */
#endif  /* INCLUDE_NETWORK */

#ifdef	INCLUDE_SCSI_BOOT
LOCAL STATUS	scsiLoad();
#endif	/* INCLUDE_SCSI_BOOT */

#ifdef  INCLUDE_FD
LOCAL STATUS    fdLoad ();
#endif  /* INCLUDE_FD */

#ifdef  INCLUDE_IDE
LOCAL STATUS    ideLoad ();
#endif  /* INCLUDE_IDE */

#ifdef	INCLUDE_ATA
LOCAL STATUS	ataLoad ();
#endif	/* INCLUDE_ATA */

#ifdef	INCLUDE_PCMCIA
LOCAL STATUS	pcmciaLoad ();
#endif	/* INCLUDE_PCMCIA */

#ifdef	INCLUDE_TFFS
LOCAL STATUS    tffsLoad ();
#endif	/* INCLUDE_TFFS */

#ifdef	INCLUDE_TSFS_BOOT
LOCAL STATUS	tsfsLoad ();
LOCAL void	wdbRebootEventGet ();
#endif	/* INCLUDE_TSFS_BOOT */

#endif	/* __STDC__ */

/*******************************************************************************
*
* usrInit - user-defined system initialization routine
*
* This routine is called by the start-up code in romStart().  It is called
* before kernel multi-tasking is enabled, with the interrupts locked out.
*
* It starts by clearing BSS, so all variables are initialized to 0 as per
* the C specification.  Then it sets up exception vectors, initializes the
* hardware by calling sysHwInit(), and finally starts the kernel with the
* usrRoot() task to do the remainder of the initialization.
*
* NOMANUAL
*/

void usrInit 
    (
    int startType
    )
    {
#ifdef BROADCOM_BSP
	/* bringupPrintRtn = sysSerialPrintStringNL;*/
	bringupPrintRtn = sysLedDsply;
#endif

    BPRINT ("CHK6");
#if (CPU_FAMILY == SPARC)
    excWindowInit ();				/* SPARC window management */
#endif

    BPRINT("CHK7");
#if	(CPU_FAMILY == MIPS)
    sysGpInit ();				/* MIPS global pointer */
#endif	/* (CPU_FAMILY == MIPS) */

    BPRINT("CHK8");

#ifdef INCLUDE_SYS_HW_INIT_0
    /*
     * Perform any BSP-specific initialisation that must be done before
     * cacheLibInit() is called and/or BSS is cleared.
     */

    SYS_HW_INIT_0 ();
#endif /* INCLUDE_SYS_HW_INIT_0 */

    /* configure data and instruction cache if available and leave disabled */

#ifdef	INCLUDE_CACHE_SUPPORT
    cacheLibInit (USER_I_CACHE_MODE, USER_D_CACHE_MODE);
#endif	/* INCLUDE_CACHE_SUPPORT */

#if (CPU == SPARClite)
    cacheLib.textUpdateRtn = NULL;		/* XXX - mod hist 07u */
#endif

    BPRINT("CHK9");

    /* don't assume bss variables are zero before this call */

    bzero (edata, end - edata);		/* zero out bss variables */

    sysStartType = startType;

    BPRINT("CHKa");

    intVecBaseSet ((FUNCPTR *) VEC_BASE_ADRS);	/* set vector base table */

#if (CPU_FAMILY == AM29XXX)
    excSpillFillInit ();                        /* am29k stack cache managemt */
#endif

    BPRINT("CHKb");
    excVecInit ();				/* install exception vectors */

    BPRINT("CHKc");
    sysHwInit ();				/* initialize system hardware */

    BPRINT("CHKd");
    usrKernelInit ();				/* configure the Wind kernel */
    BPRINT("CHKe");


#if	(CPU==SPARC) || (CPU_FAMILY==I80X86)	/* XXX workaround for sun1e */
#undef USER_I_CACHE_ENABLE	/* XXX disable instruction cache */
#endif	/* (CPU==SPARC) || (CPU_FAMILY==I80X86)	*/

#ifdef	INCLUDE_CACHE_SUPPORT
#ifdef 	USER_I_CACHE_ENABLE
    cacheEnable (INSTRUCTION_CACHE);		/* enable instruction cache */
#endif	/* USER_I_CACHE_ENABLE */
#endif	/* INCLUDE_CACHE_SUPPORT */

    /* start the kernel specifying usrRoot as the root task */

    BPRINT("CHKf");
    kernelInit ((FUNCPTR) usrRoot, ROOT_STACK_SIZE,
		(char *) MEM_POOL_START_ADRS,
		sysMemTop (), ISR_STACK_SIZE, INT_LOCK_LEVEL);

    BPRINT("ERRr");
    while(1);
    }

/*******************************************************************************
*
* usrRoot - user-defined root task
*
* The root task performs any initialization that should be done
* subsequent to the kernel initialization.
*
* It initializes the I/O system, install drivers, create devices,
* sets up the network, etc., as necessary for the particular configuration.
* It may also create the system symbol table if one is to be included.
* Finally, it spawns the boot command loop task.
*
* NOMANUAL
*/

int piggy (void) {return 1;}

void usrRoot
    (
    char *      pMemPoolStart,          /* start of system memory partition */
    unsigned    memPoolSize             /* initial size of mem pool */
    )

    {
    char tyName [20];
    int ix;

int xxx = 0;

#ifdef INCLUDE_END
    int count;
    END_TBL_ENTRY* pDevTbl;
#endif /* INCLUDE_END */
    /* Initialize the memory pool before initializing any other package.
     * The memory associated with the root task will be reclaimed at the
     * completion of its activities.                                 
     */

while (xxx == 0)
   xxx = piggy();

    BPRINT("CHKg");
    memInit (pMemPoolStart, memPoolSize);/* XXX select between memPartLibInit */

    BPRINT("CLKh");
    /* set up system timer */

    sysClkConnect ((FUNCPTR) usrClock, 0);/* connect clock interrupt routine */
    BPRINT("CHKi");
    sysClkRateSet (60);			/* set system clock rate */
    BPRINT("CHKj");
    sysClkEnable ();			/* start it */
    BPRINT("CHKk");

    /*
     * The select library needs to be initialized before the tyLib module
     * since the _func_selWakeupListInit FUNCPTR is required (SPR #3314).
     * The installation of the select task delete hook is not performed
     * for boot ROMs.
     */

#ifdef  INCLUDE_SELECT
#if VX_VERSION == 55
    selectInit (NUM_FILES);
#endif
#endif

    /* initialize I/O and file system */

    iosInit (NUM_DRIVERS, NUM_FILES, "/null");
    consoleFd = NONE;

    /* install driver for on-board serial ports and make devices */

#ifdef  INCLUDE_TYCODRV_5_2
#ifdef  INCLUDE_TTY_DEV
    if (NUM_TTY > 0)
        {
        tyCoDrv ();                             /* install console driver */

        for (ix = 0; ix < NUM_TTY; ix++)        /* create serial devices */
            {
            sprintf (tyName, "%s%d", "/tyCo/", ix);

            (void) tyCoDevCreate (tyName, ix, 512, 512);

            if (ix == CONSOLE_TTY)
                strcpy (consoleName, tyName);   /* store console name */
            }

        consoleFd = open (consoleName, O_RDWR, 0);

        /* set baud rate */

        (void) ioctl (consoleFd, FIOBAUDRATE, CONSOLE_BAUD_RATE);
        (void) ioctl (consoleFd, FIOSETOPTIONS,
			OPT_ECHO | OPT_CRMOD | OPT_TANDEM | OPT_7_BIT);
        }
#endif  /* INCLUDE_TTY_DEV */

#else   /* !INCLUDE_TYCODRV_5_2 */
#ifdef  INCLUDE_TTY_DEV
    if (NUM_TTY > 0)
        {
        ttyDrv();                               /* install console driver */

        for (ix = 0; ix < NUM_TTY; ix++)        /* create serial devices */
            {
#if (defined(INCLUDE_WDB) && (WDB_COMM_TYPE == WDB_COMM_SERIAL))
	    if (ix == WDB_TTY_CHANNEL)          /* don't use WDBs channel */
	    	continue;
#endif
            sprintf (tyName, "%s%d", "/tyCo/", ix);
            (void) ttyDevCreate (tyName, sysSerialChanGet(ix), 512, 512);

            if (ix == CONSOLE_TTY)              /* init the tty console */
                {
                strcpy (consoleName, tyName);
                consoleFd = open (consoleName, O_RDWR, 0);
                (void) ioctl (consoleFd, FIOBAUDRATE, CONSOLE_BAUD_RATE);
                (void) ioctl (consoleFd, FIOSETOPTIONS,
			OPT_ECHO | OPT_CRMOD | OPT_TANDEM | OPT_7_BIT);
                }
            }
        }
#endif  /* INCLUDE_TTY_DEV */

BPRINT("TDNE");

#ifdef INCLUDE_PC_CONSOLE
    pcConDrv ();
    for (ix = 0; ix < N_VIRTUAL_CONSOLES; ix++)
        {
        sprintf (tyName, "%s%d", "/pcConsole/", ix);
        (void) pcConDevCreate (tyName,ix, 512, 512);
        if (ix == PC_CONSOLE)           /* init the console device */
            {
            strcpy (consoleName, tyName);
            consoleFd = open (consoleName, O_RDWR, 0);
            (void) ioctl (consoleFd, FIOBAUDRATE, CONSOLE_BAUD_RATE);
            (void) ioctl (consoleFd, FIOSETOPTIONS,
			OPT_ECHO | OPT_CRMOD | OPT_TANDEM | OPT_7_BIT);
            }
        }
#endif  /* INCLUDE_PC_CONSOLE */

#endif  /* !INCLUDE_TYCODRV_5_2 */

#ifdef INCLUDE_WDB
    wdbConfig();		/* configure and initialize the WDB agent */
    vxBootFile[0] = NULL;	/* clear boot line set by wdbConfig() */

#if defined(INCLUDE_TSFS_BOOT) && defined(INCLUDE_TSFS_BOOT_VIO_CONSOLE)
    consoleFd = open ("/vio/0", O_RDWR, 0);	/* for Target Server Console */
#endif
#endif	/* INCLUDE_WDB */

    ioGlobalStdSet (STD_IN,  consoleFd);
    ioGlobalStdSet (STD_OUT, consoleFd);
    ioGlobalStdSet (STD_ERR, consoleFd);

    BPRINT("CONS");

    pipeDrv ();					/* install pipe driver */
    BPRINT("EXCE");
    excInit ();					/* init exception handling */
    BPRINT("BEXC");
    excHookAdd ((FUNCPTR) bootExcHandler);	/* install exc handler */
    BPRINT("LOGS");
    logInit (consoleFd, 5);			/* initialize logging */

#ifdef	INCLUDE_DOSFS
    hashLibInit ();				/* hashLib used by dosFS */
#endif

    /* initialize object module loader */

#if	defined(INCLUDE_AOUT)
    bootAoutInit ();				/* use a.out format */
#else	/* coff or ecoff */
#if	defined(INCLUDE_ECOFF)
    bootEcoffInit ();				/* use ecoff format */
#else	/* coff */
#if	defined(INCLUDE_COFF)
    bootCoffInit ();				/* use coff format */
#else   /* coff */
#if	defined(INCLUDE_ELF)
    bootElfInit ();				/* use elf format */
#endif
#endif 						/* mips cpp no elif */
#endif
#endif

#ifdef	INCLUDE_FLASH
    flashFsLibInit();			/* Mounts flsh: */
#endif	/* INCLUDE_FLASH */

    
    BPRINT("ENDS");
#ifdef INCLUDE_END
    /* initialize the MUX */
    muxMaxBinds = MUX_MAX_BINDS;
    if (muxLibInit() == ERROR)
	return;                              /* can't return ERROR */

    /* Initialize all the available devices. */
    for (count = 0, pDevTbl = endDevTbl; pDevTbl->endLoadFunc != END_TBL_END;
         pDevTbl++, count++)
        {
        cookieTbl[count].pCookie = muxDevLoad (pDevTbl->unit, pDevTbl->endLoadFunc,
                                       pDevTbl->endLoadString,
                                       pDevTbl->endLoan, pDevTbl->pBSP);
        if (cookieTbl[count].pCookie == NULL)
            {
            printf ("muxLoad failed!\n");
            }

        cookieTbl[count].unitNo=pDevTbl->unit;
	bzero((void *)cookieTbl[count].devName,END_NAME_MAX);
	pDevTbl->endLoadFunc((char*)cookieTbl[count].devName, NULL);

        }
#endif /* INCLUDE_END */

    
    BPRINT("BOOT");
    taskSpawn ("tBoot", bootCmdTaskPriority, bootCmdTaskOptions,
		bootCmdTaskStackSize, (FUNCPTR) bootCmdLoop,
		0,0,0,0,0,0,0,0,0,0);
    BPRINT("DONE");
    }


/*******************************************************************************
*
* usrClock - user defined system clock interrupt routine
*
* This routine is called at interrupt level on each clock interrupt.  It is
* installed a call to sysClkConnect().  It calls any other facilities that
* need to know about clock ticks, including the kernel itself.
*
* If the application needs anything to happen at clock interrupt level,
* it should be added to this routine.
*
* NOMANUAL
*/

void usrClock (void)

    {
    tickAnnounce ();	/* announce system tick to kernel */
    }

#define PCI_CFG_47XX(d, f, o)  ( 0xAC000000 | (1 << (16+d)) | (f << 8) | o )

#ifdef BROADCOM_BSP
/****************************************************************************
 * MAC address routines
 ***************************************************************************/

/*
 * Convert hex digit to hex character and vice-versa
 */

int i2xdigit(int digit)
{
    digit &= 0xf;
    return (digit > 9) ? digit - 10 + 'a' : digit + '0';
}

int xdigit2i(int digit)
{
    if (digit >= '0' && digit <= '9') return (digit - '0'     );
    if (digit >= 'a' && digit <= 'f') return (digit - 'a' + 10);
    if (digit >= 'A' && digit <= 'F') return (digit - 'A' + 10);
    return 0;
}

/*
 * parse_macaddr will take a string of the form H:H:H:H:H:H where each
 * H is one or two hex digits, or a string of the form 0xN where N may
 * consist of up to 12 hex digits.  The result is returned in a byte
 * array to avoid endian confusion.
 */

int parse_macaddr(char *str, char macaddr[6])
{
    char *s;
    int	colon = FALSE;
    int	i, c1, c2;

    if (strchr(str, ':')) {		/* Colon format */
	colon = TRUE;
    } else if (*str++ != '0' || tolower(*str++) != 'x') {
	return -1;
    } else {
	memset(macaddr, 0, 6);
    }
    /* Start at end and work back */
    s = str + strlen(str);
    for (i = 5; (i >= 0) && (s >= str); i--) {
	c2 = (s > str && isxdigit((unsigned) s[-1])) ? xdigit2i((unsigned) *--s) : 0;
	c1 = (s > str && isxdigit((unsigned) s[-1])) ? xdigit2i((unsigned) *--s) : 0;
	macaddr[i] = c1 * 16 + c2;
	if (colon && (s >= str) && (':' != *--s))
	    break;
    }
    return(((s <= str) && (!colon || (i == 0))) ? 0 : -1);
}

/*
 * format_macaddr requires a buffer of 18 bytes minimum.
 * It does not use sprintf so it can be called from an interrupt context.
 */

void format_macaddr(char buf[18], char macaddr[6])
{
    int i;

    for (i = 0; i <= 5; i++) {
	*buf++ = i2xdigit(macaddr[i] >> 4);
	*buf++ = i2xdigit(macaddr[i]);
	*buf++ = ':';
    }

    *--buf = 0;
}

/*
 * macAddrCmd
 *
 *   Process the boot loader 'M' command.
 *   'M' displays current MAC address.
 *   'M xx:xx:xx:xx:xx:xx' changes the MAC address.
 */
void macAddrCmd(char *s)
{
    char        enetAdrs[6];
    char        buf[80];

    skipSpace(&s);

    if (*s != EOS) {
        if (parse_macaddr(s, enetAdrs) < 0)
            printf("MAC address syntax error\n");
        else {
            sysEnetAddrSet(enetAdrs);
            sysNvRamSet(enetAdrs, 6, NV_OFF_MACADDR);
        }
    }

    /* Get current MAC address from NVRAM */

    sysNvRamGet(enetAdrs, 6, NV_OFF_MACADDR);

    format_macaddr(buf, enetAdrs);

    printf("MAC address: %s\n", buf);
}

/*
 * timeZoneCmd
 *
 *   Process the boot loader 'Z' command.
 *   'Z' displays current time zone.
 *   'Z w:x:y:z changes the time zone (see ansiTime(1) for format)
 */

void timeZoneCmd(char *s)
{
    extern int sysTimeZoneGood(char *tz);       /* sysLib.c */

    skipSpace(&s);

    if (*s != EOS) {
        if (sysTimeZoneGood(s))
            sysNvRamSet(s, 64, NV_OFF_TIMEZONE);
        else
            printf("Time zone syntax error; see ansiTime(1)\n");
    }

    sysNvRamGet(s, 64, NV_OFF_TIMEZONE);
    printf("Time zone: %s\n", sysTimeZoneGood(s) ? s : "not set");
}

/*
 * Command to clear NVRAM
 */

void nvramClearCmd(void)
{
    char        zero[4] = { 0, 0, 0, 0 };
    int         i;

    for (i = 0; i < NV_RAM_SIZE; i += 4)
        sysNvRamSet(zero, 4, i);

    printf("NVRAM cleared\n");
}

/*
 * testMemory
 */
#define MAX_ERRS	10

void fillPat(UINT32 *addr, UINT32 len,
	     UINT32 seed, UINT32 incr, UINT32 xor)
{
    UINT32 *ptr, *limit;

    printf("FILL ");

    ptr = addr;
    limit = addr + len / 4;

    while (ptr < limit) {
	*ptr = seed;
	seed = (seed + incr) ^ xor;
	ptr++;
    }

    cacheFlush(DATA_CACHE, addr, len);
}

STATUS verifyPat(UINT32 *addr, UINT32 len,
		 UINT32 seed, UINT32 incr, UINT32 xor)
{
    UINT32 *ptr, *limit, data;
    int errs = 0;

    printf("VERIFY ");

    cacheInvalidate(DATA_CACHE, addr, len);

    ptr = addr;
    limit = addr + len / 4;

    while (ptr < limit) {
	data = *ptr;
	if (data != seed) {
	    cacheInvalidate(DATA_CACHE, ptr, 4);
	    printf("\nMISCOMPARE: "
		   "addr=%08x wrote=%08x read=%08x reread=%08x\n",
		   (int) ptr, seed, data, *(volatile UINT32 *) ptr);
	    if (++errs == MAX_ERRS)
		break;
	}
	seed = (seed + incr) ^ xor;
	ptr++;
    }

    return (errs == 0) ? OK : ERROR;
}

void writeMemory(volatile UINT32 *addr, UINT32 len)
{
    UINT32 *end = (UINT32 *) addr + len / 4;
    while (addr < end)
	*addr++ = 0;
}

void readMemory(volatile UINT32 *addr, UINT32 len)
{
    UINT32 *end = (UINT32 *) addr + len / 4;
    while (addr < end)
	*addr++;
}

STATUS testMemory(UINT32 *addr, UINT32 len)
{
    printf("Testing: addr=%08x length=%08x", (int) addr, len);

    printf("\nCheckerboard 5/A        ");
    fillPat(addr, len, 0x55555555, 0x00000000, 0xffffffff);
    if (verifyPat(addr, len, 0x55555555, 0x00000000, 0xffffffff) != OK)
	return ERROR;

    printf("\nCheckerboard A/5        ");
    fillPat(addr, len, 0xaaaaaaaa, 0x00000000, 0xffffffff);
    if (verifyPat(addr, len, 0xaaaaaaaa, 0x00000000, 0xffffffff) != OK)
	return ERROR;

    printf("\nData equals address     ");
    fillPat(addr, len, (UINT32) addr, 0x00000004, 0x00000000);
    if (verifyPat(addr, len, (UINT32) addr, 0x00000004, 0x00000000) != OK)
	return ERROR;

    printf("\nPseudo-random           ");
    fillPat(addr, len, 0xbabeface, 0xdeadbeef, 0x00000000);
    if (verifyPat(addr, len, 0xbabeface, 0xdeadbeef, 0x00000000) != OK)
	return ERROR;

    printf("\n");
    return OK;
}
#endif /* BROADCOM_BSP */

/*****************************************************************************
 *
 * Compute NVRAM offset for currently active boot string
 */

static int bootActiveStringNVOFF(void)
{
    switch (bootActiveString) {
    default:
        return NV_OFF_BOOT0;
    case 1:
        return NV_OFF_BOOT1;
    case 2:
        return NV_OFF_BOOT2;
    }
}

/*******************************************************************************
*
* bootCmdLoop - read and execute user commands forever (until boot)
*/

LOCAL void bootCmdLoop (void)

    {
    BOOT_PARAMS params;
    char line [MAX_LINE];
    char *pLine;
    int nwords;
    int nbytes;
    int value;
    int adr;
    int adr2;
    int port, reg, regv;
    FUNCPTR entry;
    char key = 0;


    /* flush standard input to get rid of any garbage;
     * E.g. the Heurikon HKV2F gets junk in USART if no terminal connected.
     */

    (void) ioctl (STD_IN, FIOFLUSH, 0 /*XXX*/);

BPRINT("SFLS");
#ifdef BROADCOM_BSP
    bootActiveString = 0;
   (void) sysNvRamGet((char *) &bootActiveString,
                       sizeof (bootActiveString),
                       NV_OFF_ACTIVEBOOT);
#endif

    if (sysStartType & BOOT_CLEAR)
	printBootLogo ();
BPRINT("LOGO");

    usrBootLineInit (sysStartType);

BPRINT("BLIN");

    /* print out any new exception message -
     * the first byte is zeroed after printing so that we won't print
     * it again automatically.  However, 'e' command will still print out
     * the remainder. */

    printExcMsg (sysExcMsg);
    *sysExcMsg = EOS;		/* indicate exception message is old */
BPRINT("BEXC");


    /* start autoboot, unless no-autoboot specified */
    bootStringToStruct (BOOT_LINE_ADRS, &params);
    sysFlags = params.flags;

#if defined(INCLUDE_TSFS_BOOT) && defined(INCLUDE_TSFS_BOOT_VIO_CONSOLE)

    /* Wait for Target Server connection */

    while (!wdbTargetIsConnected())
    	taskDelay (sysClkRateGet());

    /*
     * disable the auto-boot mechanism, because the Target Server may not have
     * time to start its VIO Console before the end of the auto-boot coutdown.
     */

    sysStartType |= BOOT_NO_AUTOBOOT;
#endif

BPRINT("ABT ");
    if (!(sysStartType & BOOT_NO_AUTOBOOT) &&
	!(sysFlags & SYSFLG_NO_AUTOBOOT))
	{
	int timeout = TIMEOUT;

	if ((sysStartType & BOOT_QUICK_AUTOBOOT) ||
	    (sysFlags & SYSFLG_QUICK_AUTOBOOT))
	    {
	    timeout = 1;
	    }

BPRINT("ABOT");
	key = autoboot (timeout);	/* doesn't return if successful */
	}


    /* If we're here, either we aren't auto-booting, or we got an error
     * auto-booting, or the auto-booting was stopped. */

    /* put console in line mode */
BPRINT("TRML");

    (void) ioctl (consoleFd, FIOSETOPTIONS, OPT_TERMINAL);

    /* read and execute the ROM commands */

    printf ("\n");

BPRINT("BOOT");
    FOREVER
	{
#ifdef BROADCOM_BSP
        if ((key == '!') || (key == '@') || (key >= '0' && key <= '2'))
#else
	if ((key == '!') || (key == '@'))
#endif

	    {
	    line [0] = key;
	    line [1] = EOS;
	    key = 0;
	    }
	else
	    {
	    printf ("[VxWorks Boot]: ");
	    fioRdString (STD_IN, line, sizeof (line));
	    }

	adr = adr2 = 0;
	nwords = 0;

	/* take blanks off end of line */

	pLine = line + strlen (line) - 1;		/* point at last char */
	while ((pLine >= line) && (*pLine == ' '))
	    {
	    *pLine = EOS;
	    pLine--;
	    }

	pLine = line;
	skipSpace (&pLine);

	switch (*(pLine++))
	    {
	    case EOS:		/* blank line */
		break;

	    case 'd':		/* display */
		if ((getArg (&pLine, &adr, HEX, OPT) == OK) &&
		    (getArg (&pLine, &nwords, DEC, OPT) == OK))
		    d ((char *) adr, nwords);
		break;

	    case 'e':		/* exception */
		printExcMsg (sysExcMsg + 1);
		break;

	    case 'f':		/* fill */
		if ((getArg (&pLine, &adr, HEX, !OPT) == OK) &&
		    (getArg (&pLine, &nbytes, DEC, !OPT) == OK) &&
		    (getArg (&pLine, &value, DEC, !OPT) == OK))
		    {
		    bfillBytes ((char *) adr, nbytes, value);
		    }

		break;

	    case 't':		/* transpose(?) (running out of letters!) */
		if ((getArg (&pLine, &adr, HEX, !OPT) == OK) &&
		    (getArg (&pLine, &adr2, HEX, !OPT) == OK) &&
		    (getArg (&pLine, &nbytes, HEX, !OPT) == OK))
		    {
		    bcopy ((char *) adr, (char *) adr2, nbytes);
		    }
		break;

	    case 'm':		/* modify */
		if (getArg (&pLine, &adr, HEX, !OPT) == OK)
		    m ((char *) adr);
		break;

#ifdef	TARGET_HK_V2F
	    case 's':		/* system controller */
		{
		extern ULONG sysBCLSet ();

		if (getArg (&pLine, &value, DEC, !OPT) == OK)
		    {
		    if (value != 0)
			{
			(void) sysBCLSet ((ULONG)HK_BCL_SYS_CONTROLLER,
					  (ULONG)HK_BCL_SYS_CONTROLLER);
			printf ("System controller on.\n");
			}
		    else
			{
			(void) sysBCLSet ((ULONG)HK_BCL_SYS_CONTROLLER,
					  (ULONG)0);
			printf ("System controller off.\n");
			}
		    }
		break;
		}
#endif	/* TARGET_HK_V2F */

#if defined(TARGET_FRC_30) || defined(TARGET_FRC_31) || defined(TARGET_FRC_33)
	    case 's':		/* system controller */
		if (getArg (&pLine, &value, DEC, !OPT) == OK)
		    {
		    if (value != 0)
			{
			*FGA_CTL1 |= FGA_CTL1_SCON;
			printf ("System controller on.\n");
			}
		    else
			{
			*FGA_CTL1 &= ~FGA_CTL1_SCON;
			printf ("System controller off.\n");
			}
		    }
		break;
#endif	/* TARGET_FRC_30 || TARGET_FRC_31 || TARGET_FRC_33 */

	    case 'p':		/* print boot params */
#ifdef BROADCOM_BSP
                printf("\nboot string          : %d", (int) bootActiveString);
#endif
		bootParamsShow (BOOT_LINE_ADRS);
		break;

#ifdef BROADCOM_BSP
            case 'b':           /* print/change active boot string */
                value = -1;
                if (getArg(&pLine, &value, DEC, OPT) == OK) {
                    if (value < 0)
                        printf("Active boot string: %d\n", bootActiveString);
                    else if (value >= 0 && value <= 2) {
                        bootActiveString = value;
                        (void) sysNvRamSet((char *) &bootActiveString,
                                           sizeof (bootActiveString),
                                           NV_OFF_ACTIVEBOOT);
                        usrBootLineInit(BOOT_CLEAR);    /* Reload BOOT_LINE */
                        printf("Active boot string changed to %d\n",
                               bootActiveString);
                    } else
                        printf("Illegal boot string # (0-2)\n");
                }
                break;
#endif

	    case 'c':		/* change boot params */
		bootParamsPrompt (BOOT_LINE_ADRS);
#ifdef BROADCOM_BSP
                (void) sysNvRamSet (BOOT_LINE_ADRS,
                                    strlen (BOOT_LINE_ADRS) + 1,
                                    bootActiveStringNVOFF());
#else
		(void) sysNvRamSet (BOOT_LINE_ADRS,
				    strlen (BOOT_LINE_ADRS) + 1, 0);
#endif
		break;

#ifdef BROADCOM_BSP
            case 'C':           /* clear NVRAM */
                nvramClearCmd();
                break;

            case 'T':           /* Test memory */
                if (getArg(&pLine, (int *) &adr, HEX, !OPT) == OK &&
                    getArg(&pLine, (int *) &nbytes, HEX, !OPT) == OK)
                    testMemory((UINT32 *) adr, nbytes);
                else
                    printf("Usage: T <start addr>,<size>\n");
                break;
            case 'R':           /* Reset */
                printf("Resetting...");
#ifndef QUICK_TURN
                taskDelay (sysClkRateGet ());   /* pause a second */
#endif
                sysReboot();
                /*NOTREACHED*/
                break;

            case 'W':           /* watchdog Reset */
                printf("watchdog Resetting...");
#ifndef QUICK_TURN
                taskDelay (sysClkRateGet ());   /* pause a second */
#endif
                sysWdReboot();
                /*NOTREACHED*/
                break;
#endif

	    case 'g':		/* go */
		if (getArg (&pLine, (int *) &entry, HEX, !OPT) == OK)
		    go (entry);
		break;

#ifdef  INCLUDE_NETWORK
#ifdef BROADCOM_BSP
            case 'F':           /* Flash PROM over network (Broadcom) */
                netFlash(pLine);
                break;

            case 'M':
                macAddrCmd (pLine);
                break;

#endif  /* BROADCOM_BSP */

	    case 'n':
		netifAdrsPrint (pLine);
		break;

#ifdef ETHERNET_ADR_SET
	    case 'N':
		mEnet (pLine);
		break;
#endif  /* ETHERNET_ADR_SET */
#endif  /* INCLUDE_NETWORK */

#ifdef BROADCOM_BSP

            case 'Z':           /* Time Zone (see ansiTime) */
                timeZoneCmd(pLine);
                break;

#endif

	    case '?':			/* help */
            case 'h':			/* help */
		bootHelp ();
		break;
#ifdef BROADCOM_BSP
#ifdef BCM_END_DRV
	    case '5':		/* display */
		if (getArg (&pLine, &port, HEX, OPT) == OK) 
                    dumpStatistic(0, port);

		break;

	    case '6':		/* display */
                {
                    extern void   bcmSystemInit();
                    extern int rx_thread_dv_check(int unit);
                    bcmSystemInit();
                    taskDelay(100);
                    rx_thread_dv_check(0);
                }
                break;
#endif /* BCM_END_DRV */
           case '0':
            case '1':
            case '2':
                bootActiveString = pLine[-1] - '0';
                usrBootLineInit(BOOT_CLEAR);    /* Reload BOOT_LINE */
                /* Fall through */
#endif

            case '@':			/* load and go with internal params */
	    case '$':			/* load and go with internal params */

		if (bootLoad (pLine, &entry) == OK)
		    {
		    go (entry);
		    }
		else
		    {
		    taskDelay (sysClkRateGet ());	/* pause a second */
		    reboot (BOOT_NO_AUTOBOOT);		/* something is awry */
		    }
		break;

	    case 'l':			/* load with internal params */

		if (bootLoad (pLine, &entry) == OK)
		    {
		    printf ("entry = 0x%x\n", (int) entry);
		    }
		else
		    {
		    taskDelay (sysClkRateGet ());	/* pause a second */
		    reboot (BOOT_NO_AUTOBOOT);		/* something is awry */
		    }
		break;


	    default:
		printf ("Unrecognized command. Type '?' for help.\n");
		break;

            } /* switch */
        } /* FOREVER */
    }
/******************************************************************************
*
* autoboot - do automatic boot sequence
*
* RETURNS: Doesn't return if successful (starts execution of booted system).
*/

LOCAL char autoboot 
    (
    int timeout		/* timeout time in seconds */
    )
    {
    ULONG	autoBootTime;
    int		timeLeft;
    UINT	timeMarker;
    int		bytesRead = 0;
    FUNCPTR	entry;
    char	key;

    if (timeout > 0)
	{
	printf ("\nPress any key to stop auto-boot...\n");

	/* Loop looking for a char, or timeout after specified seconds */

	autoBootTime = tickGet () + sysClkRateGet () * timeout;
	timeMarker = tickGet () + sysClkRateGet ();
	timeLeft = timeout;

	printf ("%2d\r", timeLeft);

	 while ((tickGet () < autoBootTime) && (bytesRead == 0))
	    {
	    (void) ioctl (consoleFd, FIONREAD, (int) &bytesRead);

	    if (tickGet () == timeMarker)
		{
		timeMarker = tickGet () + sysClkRateGet ();
		printf ("%2d\r", --timeLeft);
		}
	    }
	}

    if (bytesRead == 0)    /* nothing typed so auto-boot */
	{
	/* put the console back in line mode so it echoes (so's you can bang
	 * on it to see if it's still alive) */

	(void) ioctl (consoleFd, FIOSETOPTIONS, OPT_TERMINAL);

	printf ("\nauto-booting...\n\n");

	if (bootLoad (BOOT_LINE_ADRS, &entry) == OK)
	    go (entry);				/* ... and never return */
	else
	    {
	    printf ("Can't load boot file!!\n");
	    taskDelay (sysClkRateGet ());	/* pause a second */
	    reboot (BOOT_NO_AUTOBOOT);		/* something is awry */
	    }
	}
    else
	{
	/* read the key that stopped autoboot */

	read (consoleFd, &key, 1);
	return (key & 0x7f);		/* mask off parity in raw mode */
	}

    return (ERROR);			/* for lint - can't really get here */
    }
/******************************************************************************
*
* printBootLogo - print initial boot banner page
*/

LOCAL void printBootLogo (void)

    {
    printf ("\n\n\n\n\n\n\n\n\n\n\n");
    printf ("%28s%s", "","VxWorks System Boot");
    printf ("\n\n\nCopyright 1984-2001  Wind River Systems, Inc.\n\n\n\n\n\n");
    printf ("CPU: %s\n", sysModel ());
    printf ("Version: %s\n", vxWorksVersion);
    printf ("BSP version: " BSP_VERSION BSP_REV "\n");
    printf ("Creation date: %s\n\n", creationDate);
    }

/*******************************************************************************
*
* bootHelp - print brief help list
*/

LOCAL void bootHelp (void)

    {
    static char *helpMsg[] =
	{
	"?",                      "- print this list",
        "@",                      "- boot using active boot string",
#ifdef BROADCOM_BSP
        "<n>",                    "- boot using boot string <n> = 0 to 2",
        "b [<n>]",                "- show/change which boot string is active",
        "P",                      "- show devices on the PCI bus",
        "Z [w:x:y:z]",            "- show/set the time zone",
        "M [xx:xx:xx:xx:xx:xx]",  "- show/set the MAC address",
        "Z [w:x:y:z]",            "- show/set the time zone",
        "R",                      "- reboot",
        "T addr, nbytes",         "- test memory",
        "C",                      "- clear NVRAM",
#endif
	"p",                      "- print boot params",
	"c",                      "- change boot params",
        "l",                      "- load boot file using active boot string",
	"g adrs",                 "- go to adrs",
	"d adrs[,n]",             "- display memory",
	"m adrs",                 "- modify memory",
	"f adrs, nbytes, value",  "- fill memory",
	"t adrs, adrs, nbytes",   "- copy memory",
	"e",                      "- print fatal exception",
#ifdef INCLUDE_NETWORK
	"n netif",	  	  "- print network interface device address",
#if defined(ETHERNET_ADR_SET)
	"N",			  "- set ethernet address",
#endif  /* ETHERNET_ADR_SET */
#endif  /* INCLUDE_NETWORK */
#if defined(TARGET_HK_V2F) || defined(TARGET_FRC_30) || \
    defined(TARGET_FRC_31) || defined(TARGET_FRC_33)
	"s [0/1]",                "- system controller 0 = off, 1 = on",
#endif	/* TARGET_HK_V2F/FRC_30/FRC_31/FRC_33 */
	"$dev(0,procnum)host:/file h=# e=# b=# g=# u=usr [pw=passwd] f=#", "",
	"                          tn=targetname s=script o=other", "",

#ifdef	INCLUDE_SCSI_BOOT
	"boot device: scsi=id,lun              file name: /sd0/vxWorks","",
#endif	/*INCLUDE_SCSI_BOOT*/

#ifdef	INCLUDE_FD
	"boot device: fd=drive,fdType          file name: /fd0/vxWorks","",
#endif	/* INCLUDE_FD */

#ifdef	INCLUDE_IDE
	"boot device: ide=drive,configType     file name: /ide0/vxWorks","",
#endif	/* INCLUDE_IDE */

#ifdef	INCLUDE_ATA
	"boot device: ata=ctrl,drive           file name: /ata0/vxWorks","",
#endif	/* INCLUDE_ATA */

#ifdef	INCLUDE_PCMCIA
	"boot device: pcmcia=sock              file name: /pcmcia0/vxWorks","",
#endif	/* INCLUDE_PCMCIA */

#ifdef	INCLUDE_TFFS
#ifdef  BROADCOM_BSP
        "boot device: flash                    file name: flash:vxWorks.st","",
#else
	"boot device: tffs=drive,removable     file name: /tffs0/vxWorks","",
#endif
#endif	/* INCLUDE_TFFS */

#ifdef INCLUDE_TSFS_BOOT
	"boot device: tsfs                     file name: /tgtsvr/vxWorks","",
#endif /*INCLUDE_TSFS_BOOT */

	"Boot flags:",		  "",
#if defined(TARGET_HK_V2F) || defined(TARGET_FRC_30) || \
    defined(TARGET_FRC_31) || defined(TARGET_FRC_33)
	"  0x01  - don't be system controller",	"",
#endif	/* TARGET_HK_V2F/FRC_30/FRC_31/FRC_33 */
	"  0x02  - load local system symbols",		"",
	"  0x04  - don't autoboot",			"",
	"  0x08  - quick autoboot (no countdown)",	"",
#ifdef  INCLUDE_NETWORK
	"  0x20  - disable login security",		"",
#ifdef  INCLUDE_DHCPC
	"  0x40  - use dhcp to get boot parameters",	"",
#else
	"  0x40  - use bootp to get boot parameters",	"",
#endif
	"  0x80  - use tftp to get boot image",		"",
	"  0x100 - use proxy arp",			"",
#endif  /* INCLUDE_NETWORK */
	NULL
	};

    FAST char **pMsg;
#ifdef  INCLUDE_NETWORK
    FAST NETIF *pNif;
#endif  /* INCLUDE_NETWORK */

#ifdef INCLUDE_END
    int count;
    char       muxDevName[8];
    END_TBL_ENTRY* pDevTbl;
#endif /* INCLUDE_END */

    printf ("\n");

    for (pMsg = helpMsg; *pMsg != NULL; pMsg += 2)
	printf (" %-21s %s\n", *pMsg, *(pMsg + 1));

#ifdef  INCLUDE_NETWORK

    printf ("\navailable boot devices:");

#ifdef INCLUDE_END
    printf ("Enhanced Network Devices\n");
    for (count = 0, pDevTbl = endDevTbl; pDevTbl->endLoadFunc != END_TBL_END;
         pDevTbl++, count++)
        {
        if (!pDevTbl->processed)
            {
            muxDevName [0] = (char) NULL;
            pDevTbl->endLoadFunc (muxDevName, NULL);
            printf (" %s%d", muxDevName, pDevTbl->unit);
            }
        }
#endif /* INCLUDE_END */
    
    for (pNif = netIf; pNif->ifName != 0; pNif++)
	{
	if (strncmp (pNif->ifName, "lo", 2) != 0)
	    printf (" %s", pNif->ifName);
	}


#endif  /* INCLUDE_NETWORK */

#ifdef	INCLUDE_SCSI_BOOT
    printf (" scsi");
#endif	/*INCLUDE_SCSI_BOOT*/

#ifdef  INCLUDE_FD
    printf (" fd");
#endif  /* INCLUDE_FD */

#ifdef  INCLUDE_IDE
    printf (" ide");
#endif  /* INCLUDE_IDE */

#ifdef  INCLUDE_ATA
    printf (" ata");
#endif  /* INCLUDE_ATA */

#ifdef  INCLUDE_TFFS
#ifdef BROADCOM_BSP
    printf (" flash");
#else
    printf (" tffs");
#endif
#endif  /* INCLUDE_TFFS */

#ifdef  INCLUDE_TSFS_BOOT
    printf (" tsfs");
#endif  /* INCLUDE_TSFS_BOOT */

    printf ("\n");
    }

#define COMP_BUF_SIZE   0x00300000
#define DECOMP_BUF_SIZE 0x00A00000

STATUS bootLoadModuleInflate(int zfd, FUNCPTR *pEntry)
{
    char		*imageBuf = NULL;
    char		*compBuf = NULL;
    int			fd = -1;
    int			rv = ERROR;
    int			compSize, r;
    extern STATUS	inflate(char *src, char *dst, int src_size);

    if ((compBuf = malloc(COMP_BUF_SIZE)) == NULL) {
	printErr("Not enough memory for image buffer\n");
	goto done;
    }

    compSize = 0;

    while ((r = read(zfd,	/* Read loop required to support network */
		     compBuf + compSize,
		     COMP_BUF_SIZE - compSize)) > 0)
	compSize += r;

    if (r < 0) {
	printErr("Read failed: errno = %d\n", errnoGet());
	goto done;
    }

    if (compSize == COMP_BUF_SIZE) {
	printErr("Compressed image too large\n");
	goto done;
    }

    printErr("Uncompressing %d bytes... ", compSize);

    if ((imageBuf = malloc(DECOMP_BUF_SIZE)) == NULL) {
	printErr("Not enough memory for decompression buffer\n");
	goto done;
    }

    if ((r = inflate(compBuf, imageBuf, compSize)) < 0) {
	printErr("\nUncompress failed\n");
	goto done;
    }

    printErr("\nLoading image... ");

    memDrv();
    memDevCreate("mem:", imageBuf, DECOMP_BUF_SIZE);

    if ((fd = open("mem:0", O_RDONLY, 0)) < 0) {
	printErr("\nCannot open memory device.\n");
	goto done;
    }

    if (bootLoadModule(fd, pEntry) != OK) {
	printErr("\nError loading: errno = %d\n", errnoGet());
	goto done;
    }

    printErr("\n");

    rv = OK;

 done:
    if (fd >= 0)
	close(fd);
    if (imageBuf)
	free(imageBuf);
    if (compBuf)
	free(compBuf);

    return rv;
}

/*******************************************************************************
*
* bootLoad - load a module into memory
*
* RETURNS: OK or ERROR
*/

LOCAL STATUS bootLoad 
    (
    char *   bootString,
    FUNCPTR *pEntry
    )
    {
    BOOT_PARAMS		params;
#ifdef  INCLUDE_SCSI_BOOT
#ifdef  INCLUDE_SCSI2
     SCSI_OPTIONS	options;
     UINT		which;
     int		devBusId;
#endif  /* INCLUDE_SCSI2 */
#endif  /* INCLUDE_SCSI_BOOT */
#ifdef  INCLUDE_NETWORK
    char		nad [20];	/* host's network internet addr */
    int			netmask = 0;	/* temporary storage */
    int 		result; 	/* classification of address string */
    unsigned long 	leaseLen; 	/* lease length field (optional) */

#ifdef INCLUDE_DHCPC
    DHCP_LEASE_DATA dhcpLease;
#endif  /* INCLUDE_DHCPC */
    char		buf [30];	/* string of netmask and timestamps */
    char 		netDev [BOOT_DEV_LEN + 1];
    char 		bootDev [BOOT_DEV_LEN];
    BOOL		backplaneBoot;
    char *		pBootAddr;
    BOOL 		attached = FALSE; /* driver is attached */


#ifdef INCLUDE_END
    char		muxDevName[8];
#endif /* INCLUDE_END */

#endif  /* INCLUDE_NETWORK */

    /* copy bootString to low mem address, if specified */

    if ((bootString != NULL) && (*bootString != EOS))
	strcpy (BOOT_LINE_ADRS, bootString);

    /* interpret boot command */

    if (usrBootLineCrack (BOOT_LINE_ADRS, &params) != OK)
	return (ERROR);

    /* Display boot parameters */

#ifdef BROADCOM_BSP
    printf("\nboot string          : %d", bootActiveString);
#endif
    bootParamsShow (BOOT_LINE_ADRS);

    /* set our processor number: may establish vme access, etc. */

    sysFlags = params.flags;
    sysProcNumSet (params.procNum);

#ifdef	INCLUDE_SCSI_BOOT

    /*
     * initialize either the SCSI1 or SCSI2 interface; initialize SCSI2 when
     * the SCSI2 interface is available.
     */

#ifndef INCLUDE_SCSI2
    scsi1IfInit ();
#else
    scsi2IfInit ();
#endif

    if (strncmp (params.bootDev, "scsi", 4) == 0)
	{
	int bootDevId = NONE;
	int bootDevLUN = NONE;

#ifdef INCLUDE_SCSI2
        /* Set all devices to asynchronous data transfer */

        which = SCSI_SET_OPT_XFER_PARAMS;
        options.maxOffset = 0;
        options.minPeriod = SCSI_SYNC_XFER_MIN_PERIOD;

        for (devBusId = 0; devBusId < 8; devBusId++)
            {
            scsiTargetOptionsSet (pSysScsiCtrl, devBusId, &options, which);
            }
#endif /* INCLUDE_SCSI2 */

	/* check for absence of bus ID and LUN, in which case
	 * auto-configure and display results
	 */

	if (strlen (params.bootDev) == 4)
	    {
	    if (!scsiInitialized)
		{
	    	if (sysScsiInit () == ERROR)
		    {
		    printErr ("Could not initialize SCSI.\n");
		    return (ERROR);
		    }
		scsiInitialized = TRUE;
		}

	    scsiAutoConfig (pSysScsiCtrl);
	    scsiShow (pSysScsiCtrl);

	    /* return ERROR to indicate that no file was loaded */
	    return (ERROR);
	    }

	sscanf (params.bootDev, "%*4s%*c%d%*c%d", &bootDevId, &bootDevLUN);

	if (scsiLoad (bootDevId, bootDevLUN, params.bootFile, pEntry) != OK)
	    {
	    printErr ("\nError loading file: errno = 0x%x.\n", errno);
	    return (ERROR);
	    }

	return (OK);
	}

#endif	/* INCLUDE_SCSI_BOOT */

#ifdef  INCLUDE_FD

    if (strncmp (params.bootDev, "fd", 2) == 0)
	{
	int type = 0;
	int drive = 0;

	if (strlen (params.bootDev) == 2)
	    return (ERROR);
	else
	    sscanf (params.bootDev, "%*2s%*c%d%*c%d", &drive, &type);

	if (fdLoad (drive, type, params.bootFile, pEntry) != OK)
	    {
	    printErr ("\nError loading file: errno = 0x%x.\n", errno);
	    return (ERROR);
	    }

	return (OK);
	}

#endif  /* INCLUDE_FD */

#ifdef	INCLUDE_IDE

    if (strncmp (params.bootDev, "ide", 3) == 0)
	{
	int type = 0;
	int drive = 0;

	if (strlen (params.bootDev) == 3)
	    return (ERROR);
	else
	    sscanf (params.bootDev, "%*3s%*c%d%*c%d", &drive, &type);

	if (ideLoad (drive, type, params.bootFile, pEntry) != OK)
	    {
	    printErr ("\nError loading file: errno = 0x%x.\n", errno);
	    return (ERROR);
	    }

	return (OK);
	}

#endif	/* INCLUDE_IDE */

#ifdef	INCLUDE_ATA

    if (strncmp (params.bootDev, "ata", 3) == 0)
	{
	int ctrl  = 0;
	int drive = 0;

	if (strlen (params.bootDev) == 3)
	    return (ERROR);
	else
	    sscanf (params.bootDev, "%*3s%*c%d%*c%d", &ctrl, &drive);

	if (ataLoad (ctrl, drive, params.bootFile, pEntry) != OK)
	    {
	    printErr ("\nError loading file: errno = 0x%x.\n", errno);
	    return (ERROR);
	    }

	return (OK);
	}

#endif	/* INCLUDE_ATA */

#ifdef	INCLUDE_PCMCIA

    pcmciaInit ();			/* init PCMCIA Lib */

    if (strncmp (params.bootDev, "pcmcia", 6) == 0)
	{
	int sock	= NONE;

	if (strlen (params.bootDev) == 6)
	    return (ERROR);
	else
	    sscanf (params.bootDev, "%*6s%*c%d", &sock);

	if (pcmciaLoad (sock, params.bootFile, pEntry) == OK)
	    return (OK);

	/* fall through if the PC card is not a block device.
	 * let's try to boot it from an ethernet device.
	 */
	}

#endif	/* INCLUDE_PCMCIA */

#ifdef  INCLUDE_FLASH_BOOT
#ifdef DOC_IS_FLASH
#define FLASH_DEV_NAME  "flsh:"
#else
#define FLASH_DEV_NAME  "flash:"
#endif

    if (strncmp (params.bootDev, FLASH_DEV_NAME, 5) == 0)
        {
        if (flashLoad (params.bootFile, pEntry) != OK)
            {
            printErr ("\nError loading file: errno = 0x%x.\n", errno);
            return (ERROR);
            }

        return (OK);
        }
#undef FLASH_DEV_NAME
#endif  /* INCLUDE_FLASH_BOOT */

#ifdef  INCLUDE_TSFS_BOOT

    if (strncmp (params.bootDev, "tsfs", 4) == 0)
	{
	if (tsfsLoad (params.bootFile, pEntry) != OK)
	    {
	    printErr ("\nError loading file: errno = 0x%x.\n", errno);
	    return (ERROR);
	    }

	return (OK);
	}

#endif  /* INCLUDE_TSFS_BOOT */

#ifndef  INCLUDE_NETWORK

    printf ("\nError loading file: networking code not present.\n");
    return (ERROR);
    }

#else  /* INCLUDE_NETWORK */

    /* start the network */

    /* initialize the generic socket library */

    if (sockLibInit (NUM_FILES) == ERROR)
        return (ERROR);

#if defined(INCLUDE_STREAMS) || defined(INCLUDE_STREAMS_ALL)
    if (usrStrmInit() == ERROR)         /* init Streams subsystem */
        return (ERROR);
#endif   

#if defined(INCLUDE_BSD) || defined(INCLUDE_BSD_SOCKET)
    /* add the BSD socket library interface */

    if (sockLibAdd ((FUNCPTR) bsdSockLibInit, AF_INET_BSD, AF_INET) == ERROR)
        return (ERROR);
    if (sockLibAdd ((FUNCPTR) bsdSockLibInit, AF_ROUTE, AF_ROUTE) == ERROR)
        return (ERROR);
#endif  /* INCLUDE_BSD || INCLUDE_BSD_SOCKET */

    /* install default socket library interface */

#ifndef DEFAULT_STREAMS_SOCKET
    if (sockLibAdd ((FUNCPTR) bsdSockLibInit, AF_INET, AF_INET) == ERROR)
        return (ERROR);
#endif  /* DEFAULT_STREAMS_SOCKET */
 
    hostTblInit ();		/* initialize host table */
    usrNetProtoInit ();         /* initialize various protocols */
    netLibInit ();

#ifdef	INCLUDE_PPP
#ifdef	INCLUDE_PPP_CRYPT 
    cryptRtnInit (&pppCryptRtn);	/* install crypt() routine */ 
#endif	/* INCLUDE_PPP_CRYPT */
#endif	/* INCLUDE_PPP */

    /* attach and configure boot interface */

    if (strncmp (params.bootDev, "ppp", 3) == 0)
        {
        /* booting via ppp */

        if (usrPPPInit (params.bootDev, params.unitNum, params.ead, 
                        ((params.gad[0] == EOS)? params.had : params.gad)) 
                        == ERROR)
            return (ERROR);
        }
    else if (strncmp (params.bootDev, "sl", 2) == 0)
	{
  	if (usrSlipInit (params.bootDev, params.unitNum, params.ead, 
                         ((params.gad[0] == EOS)? params.had : params.gad)) 
                         == ERROR)
	    return (ERROR);
        }
    else
	{
        strncpy (bootDev, params.bootDev, sizeof (bootDev));

	if ((strncmp (params.bootDev, "bp", 2) != 0) &&
            (strncmp (params.bootDev, "sm", 2) != 0))
	    {
	    pBootAddr = params.ead;
	    backplaneBoot = FALSE;
	    }
        else
	    {
	    if (sysProcNumGet () == 0)
		{
                printf (
                  "Error: processor number must be non-zero to boot from bp\n");
	        return (ERROR);
                }

	    if (usrBpInit (bootDev, params.unitNum, 0) == ERROR)
		return (ERROR);

	    pBootAddr = params.bad;
	    backplaneBoot = TRUE;
	    }

        /* Save requested lease length, if any. Ignore lease origin value. */

        result = bootLeaseExtract (pBootAddr, &leaseLen, NULL);
        if (result < 0)
            {
            printf ("Error reading target address information.\n");
            return (ERROR);
            }

        /* Handle any lease information attached to the address entry. */

        if (result == 2)
            {
            /* 
             * The current address contains both a duration value and a start
             * time, indicating that it was assigned by a DHCP server.
             */

            if (leaseLen != (ULONG)~0)
                {
                /* Handle a finite address assignment. */

                if (sysFlags & SYSFLG_AUTOCONFIG)
                    *pBootAddr = EOS;    /* Remove for later replacement. */
                else
                    {
                    /* 
                     * Technically, this address is invalid since it contains
                     * a finite interval that requires DHCP for verification
                     * and the automatic configuration flag is not set.
                     * However, this situation can only occur if caused 
                     * deliberately by the user. So, just ignore the timing
                     * information and assign the address permanently.
                     */

                    result = 0;    /* Prevents restoration of time values. */
                    }
                }
            }

#ifdef INCLUDE_DHCPC
        /* Set the DHCP lease information, if needed. */

        if (sysFlags & SYSFLG_AUTOCONFIG)
            {
            /* Save the requested lease length if entered by the user. */

            if (result == 1)
                dhcpLease.lease_duration = leaseLen;
            else
                {
                /* 
                 * The lease length is either not present or left over from
                 * an earlier lease. Use the default value.
                 */

                dhcpLease.lease_duration = DHCPC_DEFAULT_LEASE;
                }
            dhcpLease.lease_origin = 0;
            }
#endif  /* INCLUDE_DHCPC */

        netmask = 0;
        bootNetmaskExtract (pBootAddr, &netmask);

	/* start of network attachment code block */

#ifdef INCLUDE_END

	/* Try an END device first */

        pCookie=findCookie(params.unitNum, bootDev);
	if (!attached && (pCookie != NULL))
	    {
            if (muxDevStart (pCookie) != OK)
                {
                printf("Failed to start device %s\n", bootDev);
                return (ERROR);
                }
#if 0
	    if (muxIoctl (pEnd, EIOCGMIB2, (char *)&endM2Tbl)
		    == ERROR)
		return (ERROR);
#endif

	    /* Add our default address resolution functions. */

	    muxAddrResFuncAdd (M2_ifType_ethernet_csmacd, 0x800, ipEtherResolvRtn);

	    if (ipAttach (params.unitNum, bootDev) != OK)
		{
		printf ("Failed to attach TCP/IP to device %s", muxDevName);
		return (ERROR);
		}

	    printf ("Attached TCP/IP interface to %s%d.\n", bootDev,
		     params.unitNum);

	    attached = TRUE;
	    }
#endif /*INCLUDE_END*/

#ifdef INCLUDE_BSD

	/* Try a BSD 4.4 device attach sequence */

        if (!attached)
	    {
	    if (usrNetIfAttach (bootDev, params.unitNum, pBootAddr) == OK)
		{
		attached = TRUE;
		}
	    else
		return ERROR;
	    }
            
#endif /* INCLUDE_BSD */

	if (!attached)
	    return ERROR;	/* attachment failed */

        /*
         * Now that any extra parameters following an "=" sign have
         * been removed (e.g. the pcmcia socket number), build the
         * network device string by appending the unit number to the
         * device name.
         */

        sprintf (netDev, "%s%d", bootDev, params.unitNum);

        if ( (sysFlags & SYSFLG_AUTOCONFIG) || (sysFlags & SYSFLG_PROXY) ||
            (netmask == 0))
            {
            struct ifnet *	pIf;
                
            /* Initialize the boot device */
                
            if ( (pIf = ifunit (netDev)) == NULL) 
                {
                printf ("invalid device \"%s\"\n", netDev);
                return (ERROR);			/* device not attached */
                }
                
            if (pIf->if_init != NULL)
                {
                if ( (*pIf->if_init) (pIf->if_unit) != 0)
                    {
                    printf ("initialization failed for device \"%s\"\n",netDev);
                    return (ERROR);
                    }
                }
            }

#ifdef INCLUDE_SM_NET
 	if (backplaneBoot)
	    {
	    if ((params.bad [0] == EOS) &&
	    	(strncmp (bootDev, "sm", 2) == 0) &&
		(smNetInetGet (netDev, params.bad, NONE) == OK))
	    	printf ("Backplane inet address: %s\n", params.bad);

	    if (params.bad [0] == EOS) 
	    	{
	    	printf ("no backplane address specified\n");
		return (ERROR);
		}

	    if ((sysFlags & SYSFLG_AUTOCONFIG) && !(sysFlags & SYSFLG_PROXY))
	        {
#ifdef INCLUDE_DHCPC
	        printf ("Warning! DHCP over backplane may need proxy arp\n");
#else
	        printf ("Warning! BOOTP over backplane may need proxy arp\n");
#endif
	        }
	    }
#endif /* INCLUDE_SM_NET */

        if (sysFlags & SYSFLG_PROXY)
            {
#ifdef INCLUDE_PROXY_CLIENT
            printf ("registering proxy client: %s...", pBootAddr);

            if (proxyReg (netDev, pBootAddr) == ERROR)
                {
                printf ("failed: error %x\n", errno);
                return (ERROR);
                }
            printf ("done.\n");
#else /* INCLUDE_PROXY_CLIENT */
            printf ("proxy client requested but not included.\n");
            return (ERROR);
#endif /* INCLUDE_PROXY_CLIENT */
            }

        /* Get boot parameters over the network if requested. */

        if (sysFlags & SYSFLG_AUTOCONFIG)
            {
            if (bootpGet (netDev, pBootAddr, params.bootFile, params.had,
                          &netmask) == ERROR)
                return (ERROR);
#ifdef INCLUDE_DHCPC
            if (dhcpGet (netDev, pBootAddr, params.bootFile, params.had,
                         &netmask, &dhcpLease) == ERROR)
                return (ERROR);
#endif      /* INCLUDE_DHCPC */
            }

        /* configure the device */

        if (usrNetIfConfig (bootDev, params.unitNum, pBootAddr, 
                            (char *) NULL, netmask) != OK)
            return (ERROR);

        if (netmask == 0)
            {
            (void) icmpMaskGet (netDev, pBootAddr, backplaneBoot ?
                                NULL : params.had, &netmask);

            if (netmask != 0)
                {
                sprintf (bootDev, "%s%d", bootDev, params.unitNum);
                ifMaskSet (bootDev, netmask);
                printf ("Subnet Mask: 0x%x\n", netmask);
                }
            }

 	/* get gateway address */

#ifdef INCLUDE_SM_NET
	if (backplaneBoot && (params.gad [0] == EOS) && 
	    !(sysFlags & SYSFLG_PROXY))
	    {
	        struct in_addr host;		/* Internet Address */
	        struct in_addr backpl;		/* Internet Address */

	        host.s_addr = inet_addr (params.had);
	        backpl.s_addr = inet_addr (params.bad);

	        if ( in_netof(host) != in_netof(backpl) )
	        {
	        /* We can get the gateway address (assumed to be master)  */

	        if ((strncmp (bootDev, "sm", 2) == 0) && 
	            (smNetInetGet (netDev, params.gad, 0) == OK))
		    printf ("Gateway inet address: %s\n", params.gad);
		}
	    }
#endif /* INCLUDE_SM_NET */

        if (netmask != 0)		/* reconstruct address with mask */
            {
            sprintf (buf, ":%x", netmask);
            strcat  (pBootAddr, buf);
            }

        /* 
         * If a value was specified for the requested lease length and the 
         * address information was not replaced by automatic configuration, 
         * restore that value for use by later reboots.
         */

        if (!(sysFlags & SYSFLG_AUTOCONFIG) && result == 1)
            {
            /* 
             * The value is ignored if <result> is 2, since that is only
             * possible at this point for permanent DHCP assignments, 
             * which need no special processing by later reboots.
             */

            if (netmask == 0) 	/* Create empty netmask field. */
                sprintf (buf, "::%lx", leaseLen);
            else 		/* Append requested lease length to netmask. */
                sprintf(buf, ":%lx", leaseLen);

            strcat (pBootAddr, buf);
            }

#ifdef INCLUDE_DHCPC
        /* 
         * If the target IP address was assigned by a DHCP server, append
         * the lease times. The presence of those fields in the address string 
         * will cause the runtime image to renew the corresponding lease.
         */

        if (sysFlags & SYSFLG_AUTOCONFIG)
            {
            /* Add lease origin and lease duration if needed. */

            if (dhcpcBindType == DHCP_NATIVE)
                {
                if (netmask == 0) 	/* Create empty netmask field. */
                    sprintf (buf, "::%lx:%lx", dhcpLease.lease_duration, 
                                               dhcpLease.lease_origin);
                else 		/* Append lease timestamps to netmask. */
                    sprintf(buf, ":%lx:%lx", dhcpLease.lease_duration, 
                                             dhcpLease.lease_origin);
                strcat (pBootAddr, buf);
                }
            }
#endif    /* INCLUDE_DHCPC */

        bootStructToString (BOOT_LINE_ADRS, &params);
        }

    usrNetIfAttach ("lo", 0, "127.0.0.1");
    usrNetIfConfig ("lo", 0, "127.0.0.1", "localhost", 0);

    /* if a gateway was specified, extract the network part of the host's
     * address and add a route to this network
     */

    if (params.gad[0] != EOS)
        {
	inet_netof_string (params.had, nad);
#ifdef BROADCOM_BSP
        /* Make the gateway the default route */
        routeAdd ("0.0.0.0", params.gad);
#else
        routeAdd (nad, params.gad);
#endif
        }

    /* associate hostName with the specified host address */

    hostAdd (params.hostName, params.had);

#ifdef BROADCOM_BSP
    {
        extern void sysBindFix (void);
        sysBindFix ();                   /* See sysLib.c */
    }
#endif

    /* load specified file */

    if (netLoad (params.had, params.bootFile, params.usr,
		 params.passwd, pEntry) != OK)
	{
	printf ("\nError loading file: errno = 0x%x.\n", errno);
	return (ERROR);
	}

    return (OK);
    }

#ifdef INCLUDE_END
/*******************************************************************************
*
* findCookie - traverses the cookieTbl to return the right cookie
*
* Given the unit number and the device name this function traverses the cookieTbl
* to return the right cookie. This is a local file.
*
* RETURNS: cookie or NULL
*
*/

LOCAL void* findCookie
    (
    int unitNo,
    char* devName
    )
    {
    int count;
    for(count=0;count<32;count++)
	{
	if((cookieTbl[count].unitNo==unitNo) && 
	    (STREQ(cookieTbl[count].devName,devName)))
        return(cookieTbl[count].pCookie);
	}
    return (NULL);
    }
#endif    /* INCLUDE_END */


#ifdef BROADCOM_BSP

/*****************************************************************************
* netLoadInit - Attach the network to the point where ftpXfer will work.
*/

LOCAL BOOL netLoadInited = FALSE;

LOCAL STATUS netLoadInit(char *bootString)
{
    BOOT_PARAMS		params;
    char		nad [20];	/* host's network internet addr */
    int			netmask = 0;	/* temporary storage */
    int 		result; 	/* classification of address string */
    unsigned long 	leaseLen; 	/* lease length field (optional) */

#ifdef INCLUDE_DHCPC
    DHCP_LEASE_DATA dhcpLease;
#endif  /* INCLUDE_DHCPC */
    char		buf [30];	/* string of netmask and timestamps */
    char 		netDev [BOOT_DEV_LEN + 1];
    char 		bootDev [BOOT_DEV_LEN];
    BOOL		backplaneBoot;
    char *		pBootAddr;

#ifdef INCLUDE_END
    char       muxDevName[8];
#endif /* INCLUDE_END */

    if (netLoadInited)
	return OK;

    netLoadInited = TRUE;

    /* interpret boot command */

    if (usrBootLineCrack (BOOT_LINE_ADRS, &params) != OK)
	return (ERROR);

    /* set our processor number: may establish vme access, etc. */

    sysFlags = params.flags;
    sysProcNumSet (params.procNum);

    /* start the network */

    /* initialize the generic socket library */

    if (sockLibInit (NUM_FILES) == ERROR)
        return (ERROR);

#if defined(INCLUDE_STREAMS) || defined(INCLUDE_STREAMS_ALL)
    if (usrStrmInit() == ERROR)         /* init Streams subsystem */
        return (ERROR);
#endif   

#if defined(INCLUDE_BSD) || defined(INCLUDE_BSD_SOCKET)
    /* add the BSD socket library interface */

    if (sockLibAdd ((FUNCPTR) bsdSockLibInit, AF_INET_BSD, AF_INET) == ERROR)
        return (ERROR);
    if (sockLibAdd ((FUNCPTR) bsdSockLibInit, AF_ROUTE, AF_ROUTE) == ERROR)
        return (ERROR);
#endif  /* INCLUDE_BSD || INCLUDE_BSD_SOCKET */

    /* install default socket library interface */

#ifndef DEFAULT_STREAMS_SOCKET
    if (sockLibAdd ((FUNCPTR) bsdSockLibInit, AF_INET, AF_INET) == ERROR)
        return (ERROR);
#endif  /* DEFAULT_STREAMS_SOCKET */
 
    hostTblInit ();		/* initialize host table */
    usrNetProtoInit ();         /* initialize various protocols */
    netLibInit ();

#ifdef	INCLUDE_PPP
#ifdef	INCLUDE_PPP_CRYPT 
    cryptRtnInit (&pppCryptRtn);	/* install crypt() routine */ 
#endif	/* INCLUDE_PPP_CRYPT */
#endif	/* INCLUDE_PPP */

    /* attach and configure boot interface */

    if (strncmp (params.bootDev, "ppp", 3) == 0)
        {
        /* booting via ppp */

        if (usrPPPInit (params.bootDev, params.unitNum, params.ead, 
                        ((params.gad[0] == EOS)? params.had : params.gad)) 
                        == ERROR)
            return (ERROR);
        }
    else if (strncmp (params.bootDev, "sl", 2) == 0)
	{
  	if (usrSlipInit (params.bootDev, params.unitNum, params.ead, 
                         ((params.gad[0] == EOS)? params.had : params.gad)) 
                         == ERROR)
	    return (ERROR);
        }
    else
	{
        strncpy (bootDev, params.bootDev, sizeof (bootDev));

	if ((strncmp (params.bootDev, "bp", 2) != 0) &&
            (strncmp (params.bootDev, "sm", 2) != 0))
	    {
	    pBootAddr = params.ead;
	    backplaneBoot = FALSE;
	    }
        else
	    {
	    if (sysProcNumGet () == 0)
		{
                printf (
                  "Error: processor number must be non-zero to boot from bp\n");
	        return (ERROR);
                }

	    if (usrBpInit (bootDev, params.unitNum, 0) == ERROR)
		return (ERROR);

	    pBootAddr = params.bad;
	    backplaneBoot = TRUE;
	    }

        /* Save requested lease length, if any. Ignore lease origin value. */

        result = bootLeaseExtract (pBootAddr, &leaseLen, NULL);
        if (result < 0)
            {
            printf ("Error reading target address information.\n");
            return (ERROR);
            }

        /* Handle any lease information attached to the address entry. */

        if (result == 2)
            {
            /* 
             * The current address contains both a duration value and a start
             * time, indicating that it was assigned by a DHCP server.
             */

            if (leaseLen != ~0)
                {
                /* Handle a finite address assignment. */

                if (sysFlags & SYSFLG_AUTOCONFIG)
                    *pBootAddr = EOS;    /* Remove for later replacement. */
                else
                    {
                    /* 
                     * Technically, this address is invalid since it contains
                     * a finite interval that requires DHCP for verification
                     * and the automatic configuration flag is not set.
                     * However, this situation can only occur if caused 
                     * deliberately by the user. So, just ignore the timing
                     * information and assign the address permanently.
                     */

                    result = 0;    /* Prevents restoration of time values. */
                    }
                }
            }

#ifdef INCLUDE_DHCPC
        /* Set the DHCP lease information, if needed. */

        if (sysFlags & SYSFLG_AUTOCONFIG)
            {
            /* Save the requested lease length if entered by the user. */

            if (result == 1)
                dhcpLease.lease_duration = leaseLen;
            else
                {
                /* 
                 * The lease length is either not present or left over from
                 * an earlier lease. Use the default value.
                 */

                dhcpLease.lease_duration = DHCPC_DEFAULT_LEASE;
                }
            dhcpLease.lease_origin = 0;
            }
#endif  /* INCLUDE_DHCPC */

        netmask = 0;
        bootNetmaskExtract (pBootAddr, &netmask);

        (void) sprintf (netDev, "%s%d", bootDev, params.unitNum);

#ifdef INCLUDE_END
        pCookie=findCookie(params.unitNum, bootDev);
        if (muxDevStart (pCookie) != OK)
            {
            printf("Failed to start device %s\n", bootDev);
            return (ERROR);
            }

        /* Find the END_OBJ associated with it. */

        pEnd = endFindByName (bootDev, params.unitNum);
#if 0
        if (muxIoctl (pEnd, EIOCGMIB2, (char *)&endM2Tbl)
                == ERROR)
            return (ERROR);
#endif

        /* Add our default address resolution functions. */
        muxAddrResFuncAdd (M2_ifType_ethernet_csmacd, 0x800, ipEtherResolvRtn);

        if (ipAttach (params.unitNum, bootDev) != OK)
            {
            printf ("Failed to attach TCP/IP to device %s", muxDevName);
            return (ERROR);
            }
        printf ("Attached TCP/IP interface to %s%d.\n", bootDev,
                 params.unitNum);
#else
        if (usrNetIfAttach (bootDev, params.unitNum, pBootAddr) != OK)
            return (ERROR);
            
#endif /* INCLUDE_END */
        if ( (sysFlags & SYSFLG_AUTOCONFIG) || (sysFlags & SYSFLG_PROXY) ||
            (netmask == 0))
            {
            struct ifnet *	pIf;
                
            /* Initialize the boot device */
                
            if ( (pIf = ifunit (netDev)) == NULL) 
                {
                printf ("invalid device \"%s\"\n", netDev);
                return (ERROR);			/* device not attached */
                }
                
            if (pIf->if_init != NULL)
                {
                if ( (*pIf->if_init) (pIf->if_unit) != 0)
                    {
                    printf ("initialization failed for device \"%s\"\n",netDev);
                    return (ERROR);
                    }
                }
            }

#ifdef INCLUDE_SM_NET
 	if (backplaneBoot)
	    {
	    if ((params.bad [0] == EOS) &&
	    	(strncmp (bootDev, "sm", 2) == 0) &&
		(smNetInetGet (netDev, params.bad, NONE) == OK))
	    	printf ("Backplane inet address: %s\n", params.bad);

	    if (params.bad [0] == EOS) 
	    	{
	    	printf ("no backplane address specified\n");
		return (ERROR);
		}

	    if ((sysFlags & SYSFLG_AUTOCONFIG) && !(sysFlags & SYSFLG_PROXY))
	        {
#ifdef INCLUDE_DHCPC
	        printf ("DHCP over backplane only with proxy arp\n");
#else
	        printf ("BOOTP over backplane only with proxy arp\n");
#endif
	        return (ERROR);
	        }

	    if (sysFlags & SYSFLG_PROXY)
		{
#ifdef INCLUDE_PROXY_CLIENT
	    	printf ("registering proxy client: %s", params.bad);

	    	if (proxyReg (netDev, params.bad) == ERROR)
		    {
	    	    printf ("client registered failed %x\n", errno);
		    return (ERROR);
		    }
		printf ("done.\n");

#else /* INCLUDE_PROXY_CLIENT */
    	    	printf ("proxy client referenced but not included.\n");
    	    	return (ERROR);
#endif /* INCLUDE_PROXY_CLIENT */
		}
	    }
#endif /* INCLUDE_SM_NET */

        /* Get boot parameters over the network if requested. */

        if (sysFlags & SYSFLG_AUTOCONFIG)
            {
            if (bootpGet (netDev, pBootAddr, params.bootFile, params.had,
                          &netmask) == ERROR)
                return (ERROR);
#ifdef INCLUDE_DHCPC
            if (dhcpGet (netDev, pBootAddr, params.bootFile, params.had,
                         &netmask, &dhcpLease) == ERROR)
                return (ERROR);
#endif      /* INCLUDE_DHCPC */
            }

        /* configure the device */

        if (usrNetIfConfig (bootDev, params.unitNum, pBootAddr, 
                            (char *) NULL, netmask) != OK)
            return (ERROR);

        if (netmask == 0)
            {
            (void) icmpMaskGet (netDev, pBootAddr, backplaneBoot ?
                                NULL : params.had, &netmask);

            if (netmask != 0)
                {
                sprintf (bootDev, "%s%d", bootDev, params.unitNum);
                ifMaskSet (bootDev, netmask);
                printf ("Subnet Mask: 0x%x\n", netmask);
                }
            }

 	/* get gateway address */

#ifdef INCLUDE_SM_NET
	if (backplaneBoot && (params.gad [0] == EOS) && 
	    !(sysFlags & SYSFLG_PROXY))
	    {
	        struct in_addr host;		/* Internet Address */
	        struct in_addr backpl;		/* Internet Address */

	        host.s_addr = inet_addr (params.had);
	        backpl.s_addr = inet_addr (params.bad);

	        if ( in_netof(host) != in_netof(backpl) )
	        {
	        /* We can get the gateway address (assumed to be master)  */

	        if ((strncmp (bootDev, "sm", 2) == 0) && 
	            (smNetInetGet (netDev, params.gad, 0) == OK))
		    printf ("Gateway inet address: %s\n", params.gad);
		}
	    }
#endif /* INCLUDE_SM_NET */

        if (netmask != 0)		/* reconstruct address with mask */
            {
            sprintf (buf, ":%x", netmask);
            strcat  (pBootAddr, buf);
            }

        /* 
         * If a value was specified for the requested lease length and the 
         * address information was not replaced by automatic configuration, 
         * restore that value for use by later reboots.
         */

        if (!(sysFlags & SYSFLG_AUTOCONFIG) && result == 1)
            {
            /* 
             * The value is ignored if <result> is 2, since that is only
             * possible at this point for permanent DHCP assignments, 
             * which need no special processing by later reboots.
             */

            if (netmask == 0) 	/* Create empty netmask field. */
                sprintf (buf, "::%lx", leaseLen);
            else 		/* Append requested lease length to netmask. */
                sprintf(buf, ":%lx", leaseLen);

            strcat (pBootAddr, buf);
            }

#ifdef INCLUDE_DHCPC
        /* 
         * If the target IP address was assigned by a DHCP server, append
         * the lease times. The presence of those fields in the address string 
         * will cause the runtime image to renew the corresponding lease.
         */

        if (sysFlags & SYSFLG_AUTOCONFIG)
            {
            /* Add lease origin and lease duration if needed. */

            if (dhcpcBindType == DHCP_NATIVE)
                {
                if (netmask == 0) 	/* Create empty netmask field. */
                    sprintf (buf, "::%lx:%lx", dhcpLease.lease_duration, 
                                               dhcpLease.lease_origin);
                else 		/* Append lease timestamps to netmask. */
                    sprintf(buf, ":%lx:%lx", dhcpLease.lease_duration, 
                                             dhcpLease.lease_origin);
                strcat (pBootAddr, buf);
                }
            }
#endif    /* INCLUDE_DHCPC */

        bootStructToString (BOOT_LINE_ADRS, &params);
        }

    usrNetIfAttach ("lo", 0, "127.0.0.1");
    usrNetIfConfig ("lo", 0, "127.0.0.1", "localhost", 0);

    /* if a gateway was specified, extract the network part of the host's
     * address and add a route to this network
     */

    if (params.gad[0] != EOS)
        {
	inet_netof_string (params.had, nad);
#ifdef BROADCOM_BSP
	/* Make the gateway the default route */
	routeAdd ("0.0.0.0", params.gad);
#else
	routeAdd (nad, params.gad);
#endif
        }

    /* associate hostName with the specified host address */

    hostAdd (params.hostName, params.had);

#ifdef BROADCOM_BSP
    {
	extern void sysBindFix(void);
	sysBindFix();			/* See sysLib.c */
    }
#endif

    /* load specified file */

    return OK;
}
#define FLASH_BUF_SIZE		(512 * 1024)

LOCAL STATUS netFlash(char *bootString)
{
    BOOT_PARAMS		params;
    int fd = -1;
    int errFd = -1;
    FILE *fp = 0;
    char command [100];
    BOOL bootFtp;
    BOOL bootRsh = FALSE;
    char *hostName;
    char *fileName;
    char *usr;
    char *passwd;
    char *flashBuf = 0;
    char tmpc = 0;
    int entry, i;
    int			sRecords;

    /* Re-probe in case jumper moved */
    if (flashFsLibInit() == ERROR) {
        return(ERROR);
    }

    if ((flashBuf = malloc(FLASH_BUF_SIZE)) == 0) {
	printf("\nNot enough memory\n");
	goto fail;
    }
    /* copy bootString to low mem address, if specified */

    if ((bootString != NULL) && (*bootString != EOS))
	strcpy (BOOT_LINE_ADRS, bootString);

    /* interpret boot command */

    if (usrBootLineCrack (BOOT_LINE_ADRS, &params) != OK)
	goto fail;

    hostName = params.had;
    fileName = params.bootFile;
    usr = params.usr;
    passwd = params.passwd;

    bootFtp = (passwd[0] != EOS);

    if (strlen(fileName) < 4) {
    bad_fname:
	printf("Illegal file %s, must end in .img or .hex\n", fileName);
	goto fail;
    }

    if (! strcmp(fileName + strlen(fileName) - 4, ".img"))
	sRecords = 0;
    else if (! strcmp(fileName + strlen(fileName) - 4, ".hex"))
	sRecords = 1;
    else
	goto bad_fname;		/* Above */

    if (netLoadInit(bootString) != OK) {
	printf("\nError configuring network\n");
	goto fail;
    }

    printf ("Loading... ");

#ifdef INCLUDE_TFTP_CLIENT
    if (sysFlags & SYSFLG_TFTP)		/* use tftp to get image */
        {
	if (tftpXfer (hostName, 0, fileName, "get", "binary", &fd,
		      &errFd) == ERROR)
	    goto fail;
	}

   else
#endif
       {
	if (bootFtp)
	    {

	    if (ftpXfer2 (hostName, usr, passwd, "", "RETR %s", "", fileName,
		         &errFd, &fd) == ERROR)
		goto fail;
	    }
	else
	    {
	    bootRsh = TRUE;
	    sprintf (command, "cat %s", fileName);

	    fd = rcmd (hostName, RSHD, usr, usr, command, &errFd);
	    if (fd == ERROR)
		goto fail;
	    }
	}

    if ((fp = fdopen(fd, "r")) == 0) {
	printf("\nCould not open fd\n");
	goto readErr;
    }

    if (sRecords) {
	if ((i = srecLoad(fp, flashBuf, FLASH_BUF_SIZE, &entry)) < 0) {
	    printf("\nError reading file: %s\n", srecErrmsg(i));
	    goto readErr;
	}
    } else if ((i = fread(flashBuf, 1, 512 * 1024, fp)) != 512 * 1024) {
	printf("\nFailed reading 512 kB from binary file %s\n",
	       fileName);
	goto readErr;
    }

    printf("%d\n", i);

    if (bootRsh == FALSE)
	{

	/* Empty the Data Socket before close. PC FTP server hangs otherwise */

	while ((read (fd, command, sizeof (command))) > 0);

	/* Close the data connection -- fixes timeout hang with wu-ftp */
	close(fd); 

	if (bootFtp)
	    {
	    if (ftpReplyGet(errFd, TRUE) != FTP_COMPLETE)
		printErr("No FTP completion reply\n");

	    (void) ftpCommand (errFd, "QUIT",0,0,0,0,0,0);
	    }
	}

    fclose (fp);	/* closes fd */
    close (errFd);

#ifdef BROADCOM_BSP
#ifdef BCM_END_DRV
    {
        extern int bcm_stop_activities(); 
        bcm_stop_activities(); 
    }
#endif /* BCM_END_DRV */
#endif /* BROADCOM_BSP */

    printf("%d\nErasing boot area ...", i);

    for (i = FLASH_BOOT_START_SECTOR;
	 i < FLASH_BOOT_START_SECTOR + FLASH_BOOT_SIZE_SECTORS;
	 i++) {

	if (flashEraseBank(i,1) != OK) {
	    printf("\nflashBoot: failed erasing -- PROM DESTROYED\n");
	    return ERROR;
	}

	printf(".");
    }

    printf("done\nWriting boot data ...");

    if (flashBlkWrite(FLASH_BOOT_START_SECTOR, flashBuf, 0, 512*1024) != OK) {
	printf("\nflashBoot: failed writing -- PROM DESTROYED\n");
	return ERROR;
    }
	
    printf("done\n");

    free(flashBuf);

    return (OK);

readErr:
    /* check standard error on Unix */

    if (bootRsh == FALSE)
	{

	/* Empty the Data Socket before close. PC FTP server hangs otherwise */

	while ((read (fd, command, sizeof (command))) > 0);

	/* Close the data connection -- fixes timeout hang with wu-ftp */
	close(fd); 

	if (bootFtp)
	    {
	    (void) ftpReplyGet (errFd, FALSE); /* error message on std. err */
	    (void) ftpCommand (errFd, "QUIT",0,0,0,0,0,0);
	    }
	}
    else
	{
	char buf [100];
	int errBytesRecv = fioRead (errFd, buf, sizeof (buf));

	if (errBytesRecv > 0)
	    {
	    /* print error message on standard error fd */

	    buf [errBytesRecv] = EOS;
	    printf ("\n%s:%s: %s\n", hostName, fileName, buf);
	    }
	}

 fail:
    if (fp)
	fclose(fp);		/* closes fd */
    else if (fd >= 0)
	close (fd);
    if (errFd >= 0)
	close (errFd);
    if (flashBuf)
	free(flashBuf);

    return (ERROR);
}

#endif	/* BROADCOM_BSP */

/*******************************************************************************
*
* netLoad - downLoad a file from a remote machine via the network.
*
* The remote shell daemon on the machine 'host' is used to download
* the given file to the specified previously opened network file descriptor.
* The remote userId should have been set previously by a call to iam().
* If the file does not exist, the error message from the Unix 'host'
* is printed to the VxWorks standard error fd and ERROR is returned.
*
* RETURNS: OK or ERROR
*/

LOCAL STATUS netLoad 
    (
    char *hostName,
    char *fileName,
    char *usr,
    char *passwd,
    FUNCPTR *pEntry
    )
    {
    int fd;
    int errFd;		/* for receiving standard error messages from Unix */
    char command [100];
    BOOL bootFtp = (passwd[0] != EOS);
    BOOL bootRsh = FALSE;

    printf ("Loading... ");

#ifdef INCLUDE_TFTP_CLIENT
    if (sysFlags & SYSFLG_TFTP)		/* use tftp to get image */
        {
	if (tftpXfer (hostName, 0, fileName, "get", "binary", &fd,
		      &errFd) == ERROR)
	    return (ERROR);
	}

   else
#endif
       {
	if (bootFtp)
	    {
#ifdef BROADCOM_BSP
#if 0
printf("ftpXfer2: %s %s %s %s\n",hostName, fileName, usr, passwd);
#endif
            if (ftpXfer2 (hostName, usr, passwd, "", "RETR %s", "", fileName,
                         &errFd, &fd) == ERROR)
#else
	    if (ftpXfer (hostName, usr, passwd, "", "RETR %s", "", fileName,
		         &errFd, &fd) == ERROR)
#endif
		return (ERROR);
	    }
	else
	    {
	    bootRsh = TRUE;
	    sprintf (command, "cat %s", fileName);

	    fd = rcmd (hostName, RSHD, usr, usr, command, &errFd);
	    if (fd == ERROR)
		return (ERROR);
	    }
	}

#ifdef BROADCOM_BSP
    /* Support loading deflated files.  */

#if 0
{
    int			compSize, r;
	char compBuf;
    while ((r = read(fd,	/* Read loop required to support network */
		     &compBuf,
		     1 )) > 0)
	{
	if (compBuf == '\n') sysSerialPutc('\r');
	sysSerialPutc(compBuf);
	}
}
#endif

    if (DEFLATED_FILE(fileName)) {
        if (bootLoadModuleInflate(fd, pEntry) != OK)
            goto readErr;
    } else {
        if (bootLoadModule (fd, pEntry) != OK)
            goto readErr;
    }
#else
    if (bootLoadModule (fd, pEntry) != OK)
	goto readErr;
#endif

#ifdef INCLUDE_TFTP_CLIENT
    /*
     * Successful TFTP transfers don't need any cleanup. The tftpXfer()
     * routine closes all file descriptors once the data has been
     * retrieved from the remote host.
     */

    if (sysFlags & SYSFLG_TFTP) /* used tftp to get image - just exit */
        return (OK);
#endif


    if (bootRsh == FALSE)
	{

	/* Empty the Data Socket before close. PC FTP server hangs otherwise */

	while ((read (fd, command, sizeof (command))) > 0);

	if (bootFtp)
	    (void) ftpCommand (errFd, "QUIT",0,0,0,0,0,0);
	}

    close (fd);
    close (errFd);
    return (OK);

readErr:
    /* check standard error on Unix */

    if (bootRsh == FALSE)
	{

	/* Empty the Data Socket before close. PC FTP server hangs otherwise */

	while ((read (fd, command, sizeof (command))) > 0);

	if (bootFtp)
	    {
	    (void) ftpReplyGet (errFd, FALSE); /* error message on std. err */
	    (void) ftpCommand (errFd, "QUIT",0,0,0,0,0,0);
	    }
	}
    else
	{
	char buf [100];
	int errBytesRecv = fioRead (errFd, buf, sizeof (buf));

	if (errBytesRecv > 0)
	    {
	    /* print error message on standard error fd */

	    buf [errBytesRecv] = EOS;
	    printf ("\n%s:%s: %s\n", hostName, fileName, buf);
	    }
	}

    close (fd);
    close (errFd);

    return (ERROR);
    }

#endif  /* INCLUDE_NETWORK */

#ifdef  INCLUDE_FLASH_BOOT

/******************************************************************************
*
* flashLoad - load a vxWorks image from FLASH memory
*
* RETURNS: OK, or ERROR if file can not be loaded.
*/

LOCAL STATUS flashLoad(char *fileName, FUNCPTR *pEntry)
{
    int			fd;

    dosFsInit(NUM_DOSFS_FILES);	/* initialize DOS-FS */

    flashFsLibInit();			/* Mounts flsh: */

    /*
     * Load the boot file.
     * Support loading deflated files.
     */

    if ((fd = open(fileName, O_RDONLY, 0)) == ERROR) {
	printErr("Cannot open \"%s\".\n", fileName);
	return ERROR;
    }

    if (DEFLATED_FILE(fileName)) {
	if (bootLoadModuleInflate(fd, pEntry) != OK) {
	    /* Error message already printed */
	    close(fd);
	    return ERROR;
	}
    } else {
	printErr("Loading %s ... ", fileName);
	if (bootLoadModule(fd, pEntry) != OK) {
	    printErr("\nError loading file: status = 0x%x.\n",
		     errnoGet());
	    close(fd);
	    return ERROR;
	}
    }

    printf("\n");
    close(fd);

    return OK;
}
#endif	/* INCLUDE_FLASH_BOOT */

#if     (defined (INCLUDE_SCSI_BOOT) || defined (INCLUDE_FD) || \
	 defined (INCLUDE_IDE) || defined (INCLUDE_ATA) || \
	 defined (INCLUDE_TFFS))

#define	SPIN_UP_TIMEOUT	45	/* max # of seconds to wait for spinup */

/******************************************************************************
*
* devSplit - split the device name from a full path name
*
* This routine returns the device name from a valid UNIX-style path name
* by copying until two slashes ("/") are detected.  The device name is
* copied into <devName>.
*
* RETURNS: N/A
*
* NOMANUAL
*/

void devSplit 
    (
    FAST char *fullFileName,	/* full file name being parsed */
    FAST char *devName		/* result device name */
    )
    {
    FAST int nChars = 0;

    if (fullFileName != NULL)
	{
	char *p0 = fullFileName;
	char *p1 = devName;

	while ((nChars < 2) && (*p0 != EOS))
	    {
	    if (*p0 == '/')
		nChars++;

	    *p1++ = *p0++;
	    }
	*p1 = EOS;
	}
    else
	{
	(void) strcpy (devName, "");
	}
    }

#endif  /* (defined (INCLUDE_SCSI_BOOT) || (INCLUDE_FD) || (INCLUDE_IDE)) */

#ifdef  INCLUDE_SCSI_BOOT

/******************************************************************************
*
* scsiLoad - load a vxWorks image from a local SCSI disk
*
* RETURNS: OK, or ERROR if file can not be loaded.
*/

LOCAL STATUS scsiLoad 
    (
    int     bootDevId,
    int     bootDevLUN,
    char    *fileName,
    FUNCPTR *pEntry
    )
    {
    int fd;

    SCSI_PHYS_DEV *pScsiPhysBootDev;
    BLK_DEV       *pScsiBlkBootDev;
    char bootDir  [BOOT_FILE_LEN];
    int           ix;
#ifdef INCLUDE_SCSI2
    SCSI_OPTIONS  options;
    UINT          which;
#endif /* INCLUDE_SCSI2 */


    if (!scsiInitialized)		/* skip if this is a retry */
	{
	if (sysScsiInit () == ERROR)
	    {
	    printErr ("Could not initialize SCSI.\n");
	    return (ERROR);
	    }
	scsiInitialized = TRUE;
	}

    taskDelay (sysClkRateGet ());	/* delay 1 second after reset */

    if ((bootDevId  < SCSI_MIN_BUS_ID) ||
	(bootDevId  > SCSI_MAX_BUS_ID) ||
	(bootDevLUN < SCSI_MIN_LUN)    ||
	(bootDevLUN > SCSI_MAX_LUN))
	{
	printErr ("SCSI device parameters < busId = %d, lun = %d > ",
		  bootDevId, bootDevLUN);
	printErr ("are out of range (0-7).\n");
	printErr ("Check boot device format:\n");
	printErr ("    scsi=<busId>,<lun>  e.g.  scsi=2,0\n");
	return (ERROR);
	}

#ifdef INCLUDE_SCSI2
    /* Set all devices to asynchronous data transfer */

    which = SCSI_SET_OPT_XFER_PARAMS;
    options.maxOffset = 0;
    options.minPeriod = SCSI_SYNC_XFER_MIN_PERIOD;
    scsiTargetOptionsSet (pSysScsiCtrl, bootDevId, &options, which);
#endif /* INCLUDE_SCSI2 */

    /* create device handle for TEST UNIT READY commands */

    if ((pScsiPhysBootDev = scsiPhysDevCreate (pSysScsiCtrl, bootDevId,
					       bootDevLUN, 128, 0, 0,
					       0xffff, 512))
	== NULL)
	{
	printErr ("scsiPhysDevCreate failed.\n");
	return (ERROR);
	}

    /* issue a couple fo TEST UNIT READY commands to clear reset execption */

    scsiTestUnitRdy (pScsiPhysBootDev);
    scsiTestUnitRdy (pScsiPhysBootDev);

    /* issue a TEST UNIT READY every second for SPIN_UP_TIMEOUT seconds,
     * or until device returns OK status.
     */

    if (scsiTestUnitRdy (pScsiPhysBootDev) != OK)
        {
        printf ("Waiting for disk to spin up...");

        for (ix = 0; ix < SPIN_UP_TIMEOUT; ix++)
            {
            if (scsiTestUnitRdy (pScsiPhysBootDev) == OK)
                {
                printf (" done.\n");
                break;
		}
            else
		{
                if (ix != (SPIN_UP_TIMEOUT - 1))
                    printf (".");
                else
                    {
                    printf (" timed out.\n");
                    return (ERROR);
		    }
                taskDelay (sysClkRateGet ());
		}
	    }
	}

    /* delete temporary device handle */

    scsiPhysDevDelete (pScsiPhysBootDev);

    printf ("Attaching to scsi device... ");

    /* recreate a device handle, with polling for actual device parameters */

    taskDelay (sysClkRateGet ());

    if ((pScsiPhysBootDev = scsiPhysDevCreate (pSysScsiCtrl, bootDevId,
                                               bootDevLUN, 0, -1, 0, 0, 0))
         == NULL)
	{
        printErr ("scsiPhysDevCreate failed.\n");
        return (ERROR);
        }

    /*-------------------------------------------------------------------------
     *
     * Configuration of an OMTI3500
     *
     *-----------------------------------------------------------------------*/

    if ((strncmp (pScsiPhysBootDev->devVendorID, "SMS", 3) == 0) &&
	(strncmp (pScsiPhysBootDev->devProductID, "OMTI3500", 8) == 0))
	{
	char modeData [4];	/* array for floppy MODE SELECT data */

	/* zero modeData array, then set byte 1 to "medium code" (0x1b).
	 * NOTE: MODE SELECT data is highly device-specific.  If your device
	 * requires configuration via MODE SELECT, please consult the device's
	 * Programmer's Reference for the relevant data format.
	 */

	bzero (modeData, sizeof (modeData));
	modeData [1] = 0x1b;

	/* issue a MODE SELECT cmd to correctly configure floppy controller */

	scsiModeSelect (pScsiPhysBootDev, 1, 0, modeData, sizeof (modeData));

	/* delete and re-create the SCSI_PHYS_DEV so that INQUIRY will return
	 * the new device parameters, i.e., correct number of blocks
	 */

	scsiPhysDevDelete (pScsiPhysBootDev);

	/* recreate a device handle, polling for actual device parameters */

	if ((pScsiPhysBootDev = scsiPhysDevCreate (pSysScsiCtrl, bootDevId,
						   bootDevLUN, 0, -1, 0, 0, 0))
	    == NULL)
	    {
	    printErr ("scsiPhysDevCreate failed.\n");
	    return (ERROR);
	    }
	}
    /*-------------------------------------------------------------------------
     *
     * END of OMTI3500 configuration
     *
     *-----------------------------------------------------------------------*/

    /*-------------------------------------------------------------------------
     *
     * START OF CODE WHICH ASSUMES A DOS-FS PARTITION BEGINNING AT BLOCK 0
     *
     *-----------------------------------------------------------------------*/

    /* create a block device spanning entire disk (non-distructive!) */

    if ((pScsiBlkBootDev = scsiBlkDevCreate (pScsiPhysBootDev, 0, 0)) == NULL)
	{
        printErr ("scsiBlkDevCreate failed.\n");
        return (ERROR);
	}

    dosFsInit (NUM_DOSFS_FILES);        /* initialize DOS-FS */

    /* split off boot device from boot file */

    devSplit (fileName, bootDir);

    /* initialize the boot block device as a dosFs device named <bootDir> */

    if (dosFsDevInit (bootDir, pScsiBlkBootDev, NULL) == NULL)
	{
        printErr ("dosFsDevInit failed.\n");
        return (ERROR);
	}

    /*-------------------------------------------------------------------------
     *
     * END OF CODE WHICH ASSUMES A DOS-FS PARTITION BEGINNING AT BLOCK 0
     *
     *------------------------------------------------------------------------*/

    printErr ("done.\n");

    /* load the boot file */

    printErr ("Loading %s...", fileName);

    fd = open (fileName, O_RDONLY, 0);

    if (fd == ERROR)
	{
        printErr ("\nCannot open \"%s\".\n", fileName);
        return (ERROR);
	}

    if (bootLoadModule (fd, pEntry) != OK)
        goto readErr;

    close (fd);
    return (OK);

readErr:

    printErr ("\nerror loading file: status = 0x%x.\n", errnoGet ());
    close (fd);
    return (ERROR);
    }

#endif	/* INCLUDE_SCSI_BOOT */

#ifdef	INCLUDE_FD

#include "../../src/config/usrFd.c"

/******************************************************************************
*
* fdLoad - load a vxWorks image from a local floppy disk
*
* RETURNS: OK, or ERROR if file can not be loaded.
*/

LOCAL STATUS fdLoad 
    (
    int     drive,
    int     type,
    char    *fileName,
    FUNCPTR *pEntry
    )
    {
    int fd;

    if (fdDrv (FD_INT_VEC, FD_INT_LVL) != OK)
	{
	printErr ("Could not initialize.\n");
	return (ERROR);
	}

    printf ("Attaching to floppy disk device... ");

    dosFsInit (NUM_DOSFS_FILES);        /* initialize DOS-FS */

    if (usrFdConfig (drive, type, fileName) == ERROR)
	{
        printErr ("usrFdConfig failed.\n");
        return (ERROR);
	}

    printErr ("done.\n");

    /* load the boot file */

    printErr ("Loading %s...", fileName);

    if ((fd = open (fileName, O_RDONLY, 0)) == ERROR)
	{
        printErr ("\nCannot open \"%s\".\n", fileName);
        return (ERROR);
	}

    if (bootLoadModule (fd, pEntry) != OK)
        goto fdLoadErr;

    close (fd);
    return (OK);

fdLoadErr:

    printErr ("\nerror loading file: status = 0x%x.\n", errnoGet ());
    close (fd);
    return (ERROR);
    }

#endif	/* INCLUDE_FD */

#ifdef	INCLUDE_IDE

#define	IDE_MEM_DOSFS	0x200000
#include "../../src/config/usrIde.c"

/******************************************************************************
*
* ideLoad - load a vxWorks image from a local IDE disk
*
* RETURNS: OK, or ERROR if file can not be loaded.
*/

LOCAL STATUS ideLoad 
    (
    int     drive,
    int     type,
    char    *fileName,
    FUNCPTR *pEntry
    )
    {
    int fd;

    if (ideDrv (IDE_INT_VEC, IDE_INT_LVL, type) == ERROR)
	{
	printErr ("Could not initialize.\n");
	return (ERROR);
	}

    printf ("Attaching to IDE disk device... ");

    dosFsInit (NUM_DOSFS_FILES);        /* initialize DOS-FS */

    if (usrIdeConfig (drive, fileName) == ERROR)
	{
        printErr ("usrIdeConfig failed.\n");
        return (ERROR);
	}

    printErr ("done.\n");

    /* load the boot file */

    printErr ("Loading %s...", fileName);

    if ((fd = open (fileName, O_RDONLY, 0)) == ERROR)
	{
        printErr ("\nCannot open \"%s\".\n", fileName);
        return (ERROR);
	}

    if (bootLoadModule (fd, pEntry) != OK)
        goto ideLoadErr;

    close (fd);
    return (OK);

ideLoadErr:

    printErr ("\nerror loading file: status = 0x%x.\n", errno);
    close (fd);
    return (ERROR);
    }

#endif	/* INCLUDE_IDE */

#ifdef	INCLUDE_ATA

#define	ATA_MEM_DOSFS	0x200000
#include "../../src/config/usrAta.c"

/******************************************************************************
*
* ataLoad - load a vxWorks image from a local ATA disk
*
* RETURNS: OK, or ERROR if file can not be loaded.
*/

LOCAL STATUS ataLoad 
    (
    int     ctrl,
    int     drive,
    char    *fileName,
    FUNCPTR *pEntry
    )
    {
    IMPORT ATA_RESOURCE ataResources[];
    ATA_RESOURCE *pAtaResource	= &ataResources[ctrl];
    int fd;

    if (ataDrv (ctrl, pAtaResource->drives, pAtaResource->intVector,
		pAtaResource->intLevel, pAtaResource->configType,
		pAtaResource->semTimeout, pAtaResource->wdgTimeout) == ERROR)
	{
	printErr ("Could not initialize.\n");
	return (ERROR);
	}

    printf ("Attaching to ATA disk device... ");

    dosFsInit (NUM_DOSFS_FILES);        /* initialize DOS-FS */

    if (usrAtaConfig (ctrl, drive, fileName) == ERROR)
	{
        printErr ("usrAtaConfig failed.\n");
        return (ERROR);
	}

    printErr ("done.\n");

    /* load the boot file */

    printErr ("Loading %s...", fileName);

    if ((fd = open (fileName, O_RDONLY, 0)) == ERROR)
	{
        printErr ("\nCannot open \"%s\".\n", fileName);
        return (ERROR);
	}

    if (bootLoadModule (fd, pEntry) != OK)
        goto ataLoadErr;

    close (fd);
    return (OK);

ataLoadErr:

    printErr ("\nerror loading file: status = 0x%x.\n", errno);
    close (fd);
    return (ERROR);
    }

#endif	/* INCLUDE_ATA */

#ifdef	INCLUDE_PCMCIA

#define	PCMCIA_MEM_DOSFS	0x200000
#include "../../src/config/usrPcmcia.c"

/******************************************************************************
*
* pcmciaLoad - load a vxWorks image from a PCMCIA disk device
*
* RETURNS: OK, or ERROR if file can not be loaded.
*/

LOCAL STATUS pcmciaLoad 
    (
    int     sock,
    char    *fileName,
    FUNCPTR *pEntry
    )
    {
    int fd;

    printf ("Attaching to PCMCIA block device... ");

    dosFsInit (NUM_DOSFS_FILES);        /* initialize DOS-FS */

    if (usrPcmciaConfig (sock, fileName) != OK)
        return (ERROR);

    printErr ("done.\n");

    /* load the boot file */

    printErr ("Loading %s...", fileName);

    if ((fd = open (fileName, O_RDONLY, 0)) == ERROR)
	{
        printErr ("\nCannot open \"%s\".\n", fileName);
        return (ERROR);
	}

    if (bootLoadModule (fd, pEntry) != OK)
        goto pcmciaLoadErr;

    close (fd);
    return (OK);

pcmciaLoadErr:

    printErr ("\nerror loading file: status = 0x%x.\n", errno);
    close (fd);
    return (ERROR);
    }

#endif	/* INCLUDE_PCMCIA */

#ifdef	INCLUDE_TFFS

#ifndef BROADCOM_BSP
#define	TFFS_MEM_DOSFS	0x200000
#include "../../src/config/usrTffs.c"
#endif

/******************************************************************************
*
* tffsLoad - load a vxWorks image from a TFFS Flash disk
*
* RETURNS: OK, or ERROR if file can not be loaded.
*
* NOMANUAL
*/

LOCAL STATUS tffsLoad 
    (
    int     drive,		/* TFFS drive number (0 - (noOfDrives-1)) */
    int     removable,		/* 0 - nonremovable flash media */
    char    * fileName,		/* file name to download */
    FUNCPTR * pEntry
    )
    {
    int fd;

#ifndef BROADCOM_BSP
    if (tffsDrv () != OK)
	{
	printErr ("Could not initialize.\n");
	return (ERROR);
	}
#endif

    printf ("Attaching to TFFS... ");

    dosFsInit (NUM_DOSFS_FILES);        /* initialize DOS-FS */

#ifdef BROADCOM_BSP
#else
    if (usrTffsConfig (drive, removable, fileName) == ERROR)
	{
        printErr ("usrTffsConfig failed.\n");
        return (ERROR);
	}
#endif

    printErr ("done.\n");

    /* load the boot file */

    printErr ("Loading %s...", fileName);

    if ((fd = open (fileName, O_RDONLY, 0)) == ERROR)
	{
        printErr ("\nCannot open \"%s\".\n", fileName);
        return (ERROR);
	}

#ifdef BROADCOM_BSP
    if (DEFLATED_FILE(fileName)) {
        if (bootLoadModuleInflate(fd, pEntry) != OK) {
            /* Error message already printed */
            close(fd);
            return ERROR;
        }
    } else {
        printErr("Loading %s ... ", fileName);
        if (bootLoadModule(fd, pEntry) != OK) 
            goto tffsLoadErr;
    }
#else
    if (bootLoadModule (fd, pEntry) != OK)
        goto tffsLoadErr;
#endif

    close (fd);
    return (OK);

tffsLoadErr:

    printErr ("\nerror loading file: status = 0x%x.\n", errnoGet ());
    close (fd);
    return (ERROR);
    }

#endif	/* INCLUDE_TFFS */

#ifdef INCLUDE_TSFS_BOOT
/******************************************************************************
*
* tsfsLoad - load a vxWorks image from a Target Server File System (TSFS).
*
* RETURNS: OK, or ERROR if file can not be loaded.
*
* NOMANUAL
*/

LOCAL STATUS tsfsLoad 
    (
    char    * fileName,		/* file name to download */
    FUNCPTR * pEntry
    )
    {
    int fd;
    WDB_EVT_NODE rebootEventNode;
    char corefile [strlen (fileName) + 8];

    /* add a leading slash if the filename is a relatif path */

    if (fileName[0] != '/' && fileName[0] != '\\')
	sprintf (corefile, "/tgtsvr/%s", fileName);
    else
	sprintf (corefile, "/tgtsvr%s", fileName);

    printf ("Booting using TSFS...\nMake sure that your");
    printf (" Target Server is started with -R[oot] option.\n");

#ifndef INCLUDE_TSFS_BOOT_VIO_CONSOLE
    printf ("Waiting for Target Server connection...");

    /* wait for Target Server connection */

    while (!wdbTargetIsConnected())
    	taskDelay (sysClkRateGet());

    printf (" Done.\n");
#endif	/* INCLUDE_TSFS_BOOT_VIO_CONSOLE */

    /* open the core file via tsfs */

    printErr ("\nLoading %s...\n", corefile);

    if ((fd = open (corefile, O_RDONLY, 0)) == ERROR)
	{
        printErr ("\nCannot open \"%s\".\n", corefile);
        return (ERROR);
	}

    /* load the the core file */

    if (bootLoadModule (fd, pEntry) != OK)
        goto tsfsLoadErr;

    close (fd);

#if (WDB_COMM_TYPE != WDB_COMM_SERIAL)

    /* Notify the host of the target reboot */

    wdbEventNodeInit (&rebootEventNode, wdbRebootEventGet, NULL, NULL);
    wdbEventPost (&rebootEventNode);

    /* let some time to WDB to post the event */

    taskDelay (sysClkRateGet() / 10);
#endif	/* WDB_COMM_TYPE != WDB_COMM_SERIAL */

    return (OK);

tsfsLoadErr:
    printErr ("\nerror loading file: status = 0x%x.\n", errnoGet ());
    close (fd);
    return (ERROR);
    }

/******************************************************************************
*
* wdbRebootEventGet - dummy eventGet routine to force the Target Server restart
*
* suspend the WDB task, so the Target Server will get a RPC_SYSTEMERROR
* will trying to get an event, so it will restart and try to re-attach to
* the target.
*/

LOCAL void wdbRebootEventGet
    (
    void *		pNode,
    WDB_EVT_DATA *	pEvtData
    )
    {
    taskSuspend (0);
    }

#endif	/* INCLUDE_TSFS_BOOT */

#ifdef  INCLUDE_NETWORK

/******************************************************************************
*
* netifAdrsPrint - print MAC address of a network interface
*/

LOCAL void netifAdrsPrint 
    (
    char *ifname		/* interface name */
    )
    {
    IMPORT struct ifnet *ifunit ();
    struct ifnet *ifp;
    char *buf;
    char  devName [10];
    int i, value;

    if (ifname == NULL || *ifname == EOS)
	{
	printf ("Interface not specified\n");
	return;
	}

    while (*ifname == ' ')
	ifname++;       /* skip leading blanks */

    if (*ifname == EOS)
	{
	printf ("Interface not specified\n");
	return;
	}

    /* Search for unit number of network device. */
    i = 0;
    while (!isdigit((int)ifname[i]) && !isspace((int)ifname[i]) && ifname[i] != EOS)
       i++;

    if (ifname[i] == EOS)          /* No unit number given - use 0. */
       value = 0;
 
    buf = &ifname[i];
    if (bootScanNum (&buf, &value, FALSE) != OK)  /* No unit number - use 0. */
       value = 0;

    ifname[i] = EOS;
    sprintf (devName, "%s%d", ifname, value);

    if (strncmp (devName, "bp", 2) == 0)
	{
	/* address for backplane is just processor number */

	printf ("Address for device \"%s\" == 00:00:00:00:00:%02x\n",
		devName,  sysProcNumGet ());
	return;
	}

    /* start the network */

    hostTblInit ();		/* initialize host table */
    netLibInit ();

    if ((ifp = ifunit (devName)) == NULL)
	{
	if ((usrNetIfAttach (ifname, value, "0") != OK) ||
	    (usrNetIfConfig (ifname, value, "0", (char *) NULL, 0) != OK))
	    {
	    printf ("Cannot initialize interface named \"%s\"\n", devName);
	    return;
	    }

	if ((ifp = ifunit (devName)) == NULL)
	    {
	    printf ("Device named \"%s\" doesn't exist.\n", devName);
	    return;
	    }
	}

    if (!(ifp->if_flags & IFF_POINTOPOINT) &&
	!(ifp->if_flags & IFF_LOOPBACK))
	{
        printf ("Address for device \"%s\" == %02x:%02x:%02x:%02x:%02x:%02x\n",
		devName,
		((struct arpcom *)ifp)->ac_enaddr [0],
		((struct arpcom *)ifp)->ac_enaddr [1],
		((struct arpcom *)ifp)->ac_enaddr [2],
		((struct arpcom *)ifp)->ac_enaddr [3],
		((struct arpcom *)ifp)->ac_enaddr [4],
		((struct arpcom *)ifp)->ac_enaddr [5]);
	}

    if_dettach (ifunit (devName));	/* dettach interface for fresh start */
    }

#endif  /* INCLUDE_NETWORK */

/*******************************************************************************
*
* go - start at specified address
*/

LOCAL void go 
    (
    FUNCPTR entry
    )
    {
    printf ("Starting at 0x%x...\n\n", (int) entry);

    taskDelay (sysClkRateGet ());	/* give the network a moment to close */

#ifdef  INCLUDE_NETWORK
    ifreset ();				/* reset network to avoid interrupts */
#ifdef INCLUDE_END
   
    /* Stop all ENDs to restore to known state for interrupts and DMA */

    (void) muxDevStopAll (0);      
#endif  /* INCLUDE_END */
#endif  /* INCLUDE_NETWORK */

#if	(CPU_FAMILY == PPC)
    cacheTextUpdate ((void *) (LOCAL_MEM_LOCAL_ADRS),	/* cache coherency */
		     (size_t) (sysMemTop() - LOCAL_MEM_LOCAL_ADRS));
#else
#ifdef	INCLUDE_CACHE_SUPPORT
    cacheClear (DATA_CACHE, NULL, ENTIRE_CACHE);	/* push cache to mem */
#endif	/* INCLUDE_CACHE_SUPPORT */
#endif	/* (CPU_FAMILY == PPC) */

#if	(CPU_FAMILY == I80X86)
    sysClkDisable ();	/* no clock interrupts are necessary */
#endif	/* (CPU_FAMILY == I80X86) */
    (entry) ();		/* go to entry point - never to return */
    }

/*******************************************************************************
*
* m - modify memory
*
* This routine prompts the user for modifications to memory, starting at the
* specified address.  It prints each address, and the current contents of
* that address, in turn.  The user can respond in one of several ways:
*
*	RETURN   - No change to that address, but continue
*		   prompting at next address.
*	<number> - Set the contents to <number>.
*	. (dot)	 - No change to that address, and quit.
*	<EOF>	 - No change to that address, and quit.
*
* All numbers entered and displayed are in hexadecimal.
* Memory is treated as 16-bit words.
*/

LOCAL void m 
    (
    char *adrs		/* address to change */
    )
    {
    char line [MAX_LINE + 1];	/* leave room for EOS */
    char *pLine;		/* ptr to current position in line */
    int value;			/* value found in line */
    char excess;

    /* round down to word boundary */

    for (adrs = (char *) ((int) adrs & 0xfffffffe);	/* start on even addr */
         ;						/* FOREVER */
	 adrs = (char *) (((short *) adrs) + 1))	/* bump as short ptr */
	{
	/* prompt for substitution */

	printf ("%06x:  %04x-", (int) adrs, (*(short *)adrs) & 0x0000ffff);

	/* get substitution value:
	 *   skip empty lines (CR only);
	 *   quit on end of file or invalid input;
	 *   otherwise put specified value at address */

	if (fioRdString (STD_IN, line, MAX_LINE) == EOF)
	    break;

	line [MAX_LINE] = EOS;	/* make sure input line has EOS */

	/* skip leading spaces*/

	for (pLine = line; isspace ((UINT) * pLine); ++pLine)
	    ;

	if (*pLine == EOS)			/* skip field if just CR */
	    continue;

	if (sscanf (pLine, "%x%1s", &value, &excess) != 1)
	    break;				/* quit if not number */

	* (short *) adrs = value;		/* assign new value */
	}

    printf ("\n");
    }
/*******************************************************************************
*
* d - display memory
*
* Display contents of memory, starting at adrs.  Memory is displayed in
* words.  The number of words displayed defaults to 64.  If
* nwords is non-zero, that number of words is printed, rounded up to
* the nearest number of full lines.  That number then becomes the default.
*/

LOCAL void d 
    (
    FAST char *adrs,	/* address to display */
    int	       nwords	/* number of words to print. */
    )			/* If 0, print 64 or last specified. */
    {
    static char *last_adrs;
    static int dNbytes = 128;
    char ascii [17];
    FAST int nbytes;
    FAST int byte;

    ascii [16] = EOS;			/* put an EOS on the string */

    nbytes = 2 * nwords;

    if (nbytes == 0)
	nbytes = dNbytes;	/* no count specified: use current byte count */
    else
	dNbytes = nbytes;	/* change current byte count */

    if (adrs == 0)
	adrs = last_adrs;	/* no address specified: use last address */

    adrs = (char *) ((int) adrs & ~1);	/* round adrs down to word boundary */


    /* print leading spaces on first line */

    bfill ((char *) ascii, 16, '.');

    printf ("%06x:  ", (int) adrs & ~0xf);

    for (byte = 0; byte < ((int) adrs & 0xf); byte++)
	{
	printf ("  ");
	if (byte & 1)
	    printf (" ");	/* space between words */
	if (byte == 7)
	    printf (" ");	/* extra space between words 3 and 4 */

	ascii [byte] = ' ';
	}


    /* print out all the words */

    while (nbytes-- > 0)
	{
	if (byte == 16)
	    {
	    /* end of line:
	     *   print out ascii format values and address of next line */

	    printf ("  *%16s*\n%06x:  ", ascii, (int) adrs);

	    bfill ((char *) ascii, 16, '.');	/* clear out ascii buffer */
	    byte = 0;				/* reset word count */
	    }

	printf ("%02x", *adrs & 0x000000ff);
	if (byte & 1)
	    printf (" ");	/* space between words */
	if (byte == 7)
	    printf (" ");	/* extra space between words 3 and 4 */

	if (* adrs == ' ' ||
			(isascii ((UINT) * adrs) && isprint ((UINT) * adrs)))
	    ascii [byte] = (UINT) * adrs;

	adrs++;
	byte++;
	}


    /* print remainder of last line */

    for (; byte < 16; byte++)
	{
	printf ("  ");
	if (byte & 1)
	    printf (" ");	/* space between words */
	if (byte == 7)
	    printf (" ");	/* extra space between words 3 and 4 */

	ascii [byte] = ' ';
	}

    printf ("  *%16s*\n", ascii);	/* print out ascii format values */

    last_adrs = adrs;
    }
/*******************************************************************************
*
* bootExcHandler - bootrom exception handling routine
*/

LOCAL void bootExcHandler 
    (
    int tid		/* task ID */
    )
    {
    REG_SET regSet;       /* task's registers */

    /* get registers of task to be traced */

    if (taskRegsGet (tid, &regSet) != ERROR)
        {
        trcStack (&regSet, (FUNCPTR) NULL, tid);
        taskRegsShow (tid);
        }
    else
        printf ("bootExcHandler: exception caught but no valid task.\n");

    taskDelay (sysClkRateGet ());       /* pause a second */

    reboot (BOOT_NO_AUTOBOOT);
    }
/*******************************************************************************
*
* skipSpace - advance pointer past white space
*
* Increments the string pointer passed as a parameter to the next
* non-white-space character in the string.
*/

LOCAL void skipSpace 
    (
    FAST char **strptr	/* pointer to pointer to string */
    )
    {
    while (isspace ((UINT) ** strptr))
	++ * strptr;
    }
/*******************************************************************************
*
* printExcMsg - print exception message
*
* Avoid printing possible control characters in exception message area.
*/

LOCAL void printExcMsg 
    (
    char *string
    )
    {
    printf ("\n");

    while (isascii ((UINT) * string) && (isprint ((UINT) * string) ||
						isspace ((UINT) * string)))
	printf ("%c", * string++);

    printf ("\n");
    }

/******************************************************************************
*
* getArg - get argument from command line
*
* This routine gets the next numerical argument from the command line.
* If the argument is not optional, then an error is reported if no argument
* is found.  <ppString> will be updated to point to the new position in the
* command line.
*
* RETURNS: OK or ERROR
*/

LOCAL STATUS getArg 
    (
    FAST char **ppString,	/* ptr to ptr to current position in line */
    int *	pValue,		/* ptr where to return value */
    BOOL	defaultHex,	/* TRUE = arg is hex (even w/o 0x) */
    BOOL	optional	/* TRUE = ok if end of line */
    )
    {
    skipSpace (ppString);


    /* if nothing left, complain if arg is not optional */

    if (**ppString == EOS)
	{
	if (!optional)
	    {
	    printf ("missing parameter\n");
	    return (ERROR);
	    }
	else
	    return (OK);
	}


    /* scan arg */

    if (bootScanNum (ppString, pValue, defaultHex) != OK)
	{
	printf ("invalid parameter\n");
	return (ERROR);
	}

    skipSpace (ppString);

    /* if we encountered ',' delimiter, step over it */

    if (**ppString == ',')
	{
	++*ppString;
	return (OK);
	}

    /* if end of line, scan is ok */

    if (**ppString == EOS)
	return (OK);

    /* we got stopped by something else */

    printf ("invalid parameter\n");
    return (ERROR);
    }


/* The following routines are common to bootConfig and usrConfig and will
 * eventually be merged
 */

/******************************************************************************
*
* usrBootLineInit - initialize system boot line
*
* Initializes system boot line as per specified start type.
* If this is a COLD boot, i.e., with CLEAR option to clear memory,
* then the boot line is initialized from non-volatile RAM, if any,
* otherwise from the compiled in default boot line.
*/

LOCAL void usrBootLineInit 
    (
    int startType
    )
    {
    if (startType & BOOT_CLEAR)
	{
	/* this is a cold boot so get the default boot line */

	if ((sysNvRamGet (BOOT_LINE_ADRS, 
                          BOOT_LINE_SIZE, 
                          bootActiveStringNVOFF()) == ERROR) ||
	    (*BOOT_LINE_ADRS == EOS))
	    {
	    /* either no non-volatile RAM or empty boot line */

	    strcpy (BOOT_LINE_ADRS, DEFAULT_BOOT_LINE);
	    }
	}
    }
/******************************************************************************
*
* usrBootLineCrack - interpret and verify boot line
*
* This routine interprets the specified boot line and checks the validity
* of certain parameters.  If there are errors, a diagnostic message is
* printed.
*
* RETURNS: OK or ERROR
*/

LOCAL STATUS usrBootLineCrack 
    (
    char *	 bootString,
    BOOT_PARAMS *pParams
    )
    {
    FAST char *	pS;

    pS = bootStringToStruct (bootString, pParams);

    if (*pS != EOS)
	{
	bootParamsErrorPrint (bootString, pS);
	return (ERROR);
	}

#ifdef  INCLUDE_NETWORK
    /* check inet addresses */

    if ((checkInetAddrField (pParams->ead, TRUE) != OK) ||
	(checkInetAddrField (pParams->bad, TRUE) != OK) ||
	(checkInetAddrField (pParams->had, FALSE) != OK) ||
	(checkInetAddrField (pParams->gad, FALSE) != OK))
	{
	return (ERROR);
	}
#endif  /* INCLUDE_NETWORK */

    return (OK);
    }

#ifdef  INCLUDE_NETWORK

/******************************************************************************
*
* checkInetAddrField - check for valid inet address in boot field
*/

LOCAL STATUS checkInetAddrField 
    (
    char *pInetAddr,
    BOOL subnetMaskOK
    )
    {
    char inetAddr [30];
    int netmask;

    /* 
     * The bzero() call corrects SPR 6326. The calls to bootNetmaskExtract()
     * and inet_addr() did not delimit the input string with a '\0'. When
     * inet_addr attempted to print the invalid address, the system would
     * crash or hang.
     */

    bzero (inetAddr, sizeof(inetAddr));

    if (*pInetAddr == EOS)
	return (OK);

    strncpy (inetAddr, pInetAddr, sizeof (inetAddr) - 1);

    if (subnetMaskOK)
	{
	if (bootNetmaskExtract (inetAddr, &netmask) < 0)
	    {
	    printf ("Error: invalid netmask in boot field \"%s\".\n", inetAddr);
	    return (ERROR);
	    }
	}

    if (inet_addr (inetAddr) == (ULONG) ERROR)
	{
	printf ("Error: invalid inet address in boot field \"%s\".\n",inetAddr);
	return (ERROR);
	}

    return (OK);
    }

/******************************************************************************
*
* usrNetIfAttach - attach a network interface
*
* This routine attaches the specified network interface.
*
*	- interface is attached
*	- interface name is constructed as "<devName><unitNum>"
*
* RETURNS: OK or ERROR
*/

LOCAL STATUS usrNetIfAttach 
    (
    char *devName,
    int  unitNum,
    char *inetAdrs
    )
    {
    FAST NETIF *	pNif;
    STATUS 		status;
    char                buf [BOOT_DEV_LEN];     /* network device */

#ifdef  INCLUDE_PCMCIA
    int sock;

    if (strncmp (devName, "pcmcia", 6) == 0)
	{
	if (strlen (devName) == 6)
	    return (ERROR);
	else
	    sscanf (devName, "%*6s%*c%d", &sock);
	*(devName + 6) = '\0';
	}
#endif  /* INCLUDE_PCMCIA */

    /* find interface in table */
    sprintf(buf, "%s%d", devName, unitNum);

    for (pNif = netIf; pNif->ifName != 0; pNif++)
	{
	if (strcmp (buf, pNif->ifName) == 0)
	    break;
	}

    /*
     * For backward compatibility, the device name only is acceptable for
     * unit numbers of 0.
     */
    if (pNif->ifName == 0 && unitNum == 0)
        {
        for (pNif = netIf; pNif->ifName != 0; pNif++)
            {
            if (strcmp (devName, pNif->ifName) == 0)
                break;
            }
        }

    if (pNif->ifName == 0)
	{
	printf ("Network interface %s unknown.\n", devName);
	return (ERROR);
	}

    if (pNif->attachRtn == NULL)
        {
	printf ("Network interface %s has no attach routine.\n", devName);
	return (OK);
	}

    printf ("Attaching network interface %s... ", buf);

#ifdef	INCLUDE_PCMCIA
    if (strncmp (devName, "pcmcia", 6) == 0)
	pNif->arg1 = (char *)sock;
#endif	/* INCLUDE_PCMCIA */

#if defined (TARGET_VIP10)
        /* SGI VIP10 boards are supposed to come with the ethernet address
         * in SGI formated non volatile ram.  We can not be sure where this
         * is so we default the upper 4 bytes of the address to SGI standard
         * and use the bottom two bytes of the internet address for the rest.
         */
        if (strcmp (devName, "lnsgi") == 0)
            {
            IMPORT char lnsgiEnetAddr [];      /* ethernet addr for lance */

            u_long inet = inet_addr (inetAdrs);
            lnsgiEnetAddr [4] = (inet >> 8) & 0xff;
            lnsgiEnetAddr [5] = inet & 0xff;
            }
#endif  /* TARGET_VIP10 */

    status = pNif->attachRtn (unitNum, pNif->arg1, pNif->arg2, pNif->arg3,
			      pNif->arg4, pNif->arg5, pNif->arg6, pNif->arg7, 
                              pNif->arg8);
    if (status != OK)
	{
        if (errno == S_iosLib_CONTROLLER_NOT_PRESENT)
            printf ("failed.\nError: S_iosLib_CONTROLLER_NOT_PRESENT.\n");
        else if (errno == S_iosLib_INVALID_ETHERNET_ADDRESS)
	    printf ("failed: S_iosLib_INVALID_ETHERNET_ADDRESS, use N command\n");
	else
	    printf ("failed: errno = %#x.\n", errno);
	return (ERROR);
	}

    printf ("done.\n");

    return (OK);
    }

/******************************************************************************
*
* usrNetIfConfig - configure a network interface
*	- subnetmask is extracted from inetAdrs and, if present,
*	  set for interface
*	- inet address is set for interface
*	- if present, inet name for interface is added to host table
*/

LOCAL STATUS usrNetIfConfig 
    (
    char *	devName,		/* device name */
    int         unitNum,                /* unit number */
    char *	inetAdrs,		/* inet address */
    char *	inetName,		/* host name */
    int 	netmask			/* subnet mask */
    )
    {
    char 	ifname [20];

#ifdef	INCLUDE_PCMCIA
    if (strncmp (devName, "pcmcia", 6) == 0)
	devName = "pcmcia";
#endif	/* INCLUDE_PCMCIA */

    /* check for empty inet address */

    if (inetAdrs[0] == EOS)
	{
	printf ("No inet address specified for interface %s.\n", devName);
	return (ERROR);
	}

    /* build interface name */

    sprintf (ifname, "%s%d", devName, unitNum);

    /* set subnet mask, if any specified */

    if (netmask != 0)
	ifMaskSet (ifname, netmask);

    /* set inet address */

    if (ifAddrSet (ifname, inetAdrs) != OK)
        {
        printf ("Error setting inet address of %s to %s, errno = %#x\n",
                ifname, inetAdrs, errno);
        return (ERROR);
        }

    /* add host name to host table */

    if ((inetName != NULL) && (*inetName != EOS))
	hostAdd (inetName, inetAdrs);

    return (OK);
    }

/******************************************************************************
*
* usrBpInit - initialize backplane driver
*
* usrBpInit initializes the backplane driver shared memory region
* and sets up the backplane parameters to attach.
*
* RETURNS: OK if successful otherwise ERROR
*/

LOCAL STATUS usrBpInit 
    (
    char *	devName,	/* device name */
    int         unitNum,        /* unit number */
    u_long	startAddr	/* inet address */
    )
    {
#ifdef	INCLUDE_SM_NET
    char *		bpAnchor;	/* anchor address */
    FAST NETIF *	pNif;		/* netif struct */
    STATUS 		status;		/* status */
    int			procNum;	/* proc num */
    char                buf [BOOT_DEV_LEN];     /* network device */

    /*
     * Pick off optional "=<anchorAdrs>" from backplane
     * device.  Also truncates devName to just "bp" or "sm"
     */

    if ((strncmp (devName, "bp=", 3) == 0) ||
        (strncmp (devName, "sm=", 3) == 0))
	{
	if (bootBpAnchorExtract (devName, &bpAnchor) < 0)
	    {
	    printf ("Invalid anchor address specified: \"%s\"\n", devName);
	    return (ERROR);
	    }
	}
    else
	bpAnchor = SM_ANCHOR_ADRS;		/* default anchor */

    procNum = sysProcNumGet ();

    /* if we are master, initialize backplane net */

    if (procNum == 0)
	{
	printf ("Initializing backplane net with anchor at %#x... ",
		(int) bpAnchor);

	status = smNetInit ((SM_ANCHOR *) bpAnchor, (char *) SM_MEM_ADRS,
			   (int) SM_MEM_SIZE, SM_TAS_TYPE, 0, 0, startAddr);

	if (status == ERROR)
  	    {
	    printf ("Error: backplane device %s not initialized\n", devName);
	    return (ERROR);
	    }

	printf ("done.\n");
	}

    /* Locate NETIF structure for backplane */
    sprintf(buf, "%s%d", devName, unitNum);

    for (pNif = netIf; pNif->ifName != 0; pNif++)
   	{
	if (strcmp (buf, pNif->ifName) == 0)
	    break;
	}

    /*
     * For backward compatibility, the device name only is acceptable for
     * unit numbers of 0.
     */
    if (pNif->ifName == 0 && unitNum == 0)
        {
        for (pNif = netIf; pNif->ifName != 0; pNif++)
            {
            if (strcmp (devName, pNif->ifName) == 0)
                break;
            }
        }

    if (pNif->ifName == 0)
	return (ERROR);

    printf ("Backplane anchor at %#x... ", (int) bpAnchor);

    /* configure backplane parameters (most set in NETIF struct) */

    pNif->arg1 = bpAnchor;		/* anchor address */
    pNif->arg3 = SM_INT_NONE;
    pNif->arg4 = 0;
    pNif->arg5 = 0;
    pNif->arg6 = 0;

    return (OK);
#else	/* INCLUDE_SM_NET */

    printf ("\nError: backplane driver referenced but not included.\n");
    return (ERROR);
#endif	/* INCLUDE_SM_NET */
    }

/*******************************************************************************
*
* usrSlipInit - initialize the slip device
*
* RETURNS: OK if successful, otherwise ERROR.
*/

LOCAL STATUS usrSlipInit 
    (
    char *pBootDev,		/* boot device */
    int  unitNum,               /* unit number */
    char *localAddr,		/* local address */
    char *peerAddr		/* peer address */
    )
    {
#ifdef INCLUDE_SLIP
    char 	slTyDev [20];		/* slip device */
    int		netmask;		/* netmask */

#ifndef SLIP_BAUDRATE
#define SLIP_BAUDRATE 0 /* uses previously selected baudrate */
#endif

#ifdef	CSLIP_ENABLE
#undef	CSLIP_ENABLE
#define	CSLIP_ENABLE	TRUE	/* force CSLIP header compression */
#else	/* CSLIP_ENABLE */
#undef	CSLIP_ENABLE
#define	CSLIP_ENABLE	FALSE
#endif	/* CSLIP_ENABLE */

#ifdef	CSLIP_ALLOW
#undef	CSLIP_ALLOW
#define	CSLIP_ALLOW	TRUE	/* enable CSLIP compression on Rx */
#else	/* CSLIP_ALLOW */
#undef	CSLIP_ALLOW
#define	CSLIP_ALLOW	FALSE
#endif	/* CSLIP_ALLOW */

#ifndef SLIP_MTU
#define SLIP_MTU 576
#endif

    if (pBootDev [2] == '=')
	{
	/* get the tty device used for SLIP interface e.g. "sl=/tyCo/1" */
	strcpy (slTyDev, &pBootDev [3]);
	pBootDev [2] = '\0';
	}
    else
	{
	/* construct the default SLIP tty device */
	sprintf (slTyDev, "%s%d", "/tyCo/", SLIP_TTY);
	}

    printf ("Attaching network interface sl%d... ", unitNum);

    bootNetmaskExtract (localAddr, &netmask); /* remove and ignore mask */

    if (slipInit (unitNum, slTyDev, localAddr, peerAddr, SLIP_BAUDRATE,
        CSLIP_ENABLE, CSLIP_ALLOW, SLIP_MTU) == ERROR)
	{
	printf ("\nslipInit failed 0x%x\n", errno);
	return (ERROR);
	}

    printf ("done.\n");
    return (OK);

#else /* INCLUDE_SLIP */
    printf ("\nError: slip not included.\n");
    return (ERROR);
#endif	/* INCLUDE_SLIP */
    }

/*******************************************************************************
*
* usrPPPInit - initialize a ppp channel
*
* RETURNS: OK if successful, otherwise ERROR.
*/

LOCAL STATUS usrPPPInit (pBootDev, unitNum, localAddr, peerAddr)
    char *      pBootDev;               /* boot device */
    int         unitNum;                /* unit number */
    char *      localAddr;              /* local address */
    char *      peerAddr;               /* peer address */
    {
#ifdef INCLUDE_PPP
    PPP_INFO    pppInfo;
    PPP_OPTIONS *pOptions = NULL;
    char        pppTyDev [20];          /* ppp device */
#ifdef PPP_BAUDRATE
    int		pppBaudRate = PPP_BAUDRATE;	/* ppp baud rate */
#else
    int		pppBaudRate = 0;		/* ppp baud rate */
#endif	/* PPP_BAUDRATE */
    char *	pBaudStr;		/* ppp boot string */
    int         netmask;                /* netmask */
    int		sysRate = sysClkRateGet();
    int		ix;

#ifdef  PPP_OPTIONS_STRUCT  
    pOptions = &pppOptions;
#endif  /* PPP_OPTIONS_STRUCT */
     
    if ((pBaudStr = strpbrk (pBootDev, ",")) != NULL)
        {
	*pBaudStr++ = '\0';
	pppBaudRate = atoi (pBaudStr);
	}

    if (pBootDev [3] == '=')
        {
        /* get the tty device used for PPP interface e.g. "ppp=/tyCo/1" */
        strcpy (pppTyDev, &pBootDev [4]);
        pBootDev [3] = '\0';
        }
    else 
        {
        /* construct the default PPP tty device */
        sprintf (pppTyDev, "%s%d", "/tyCo/", PPP_TTY);
        }

    printf ("Attaching network interface ppp%d...\n", unitNum);

    bootNetmaskExtract (localAddr, &netmask); /* remove and ignore mask */

    if (pppInit (unitNum, pppTyDev, localAddr, peerAddr, pppBaudRate,
		 pOptions, PPP_OPTIONS_FILE) == ERROR)
        {
        printf ("\npppInit failed 0x%x\n", errno);
        return (ERROR);
        }

    /* wait for PPP link to be established */

    for (ix = 0; ix < PPP_CONNECT_DELAY; ix++)
	{
	taskDelay (sysRate);

        if ((pppInfoGet (unitNum, &pppInfo) == OK) &&
	    (pppInfo.ipcp_fsm.state == OPENED))
           break;
	}

    if (ix == PPP_CONNECT_DELAY) 
        {
        pppDelete (unitNum);                   /* kill the interface */ 
	printf ("ppp0: timeout: could not establish link with peer.\n");
	return (ERROR);
	}

    printf ("done.\n");
    return (OK);

#else /* INCLUDE_PPP */
    printf ("\nError: ppp not included.\n");
    return (ERROR);
#endif  /* INCLUDE_PPP */
    }

#if defined(INCLUDE_STREAMS) || defined(INCLUDE_STREAMS_ALL)

/*******************************************************************************
*
* usrStrmInit - Streams subsystem initialization
*
* This routine is called at system startup time to create the Streams task
* and install relevant Streams services.
*
* RETURNS: OK, or ERROR if the Streams initialization fails.
*/
 
LOCAL STATUS usrStrmInit (void)
    {
    int strmMemSize = STREAMS_MEM_PART_SIZE;

    if ((strmMemSize == NULL) || (strmMemSize > STREAMS_MEM_MAX))
        strmMemSize = STREAMS_MEM_MAX;
 
    if (strmInit (strmMemSize,
                     STREAMS_MEM_PART_ADDR,
                     STREAMS_MSGSZ_MAX,
                     STREAMS_CTLSZ_MAX,
                     STREAMS_PUSH_MAX) == ERROR)
        {
        printf ("Streams initialization failure\n");
        return (ERROR);
        }
 
#if defined(INCLUDE_STREAMS_SOCKET) || defined(INCLUDE_STREAMS_ALL)
    strmSockInit ();            /* initialize sockmod module */

    if (sockLibAdd ((FUNCPTR) strmSockLibInit, AF_INET_STREAMS, AF_INET) ==
        ERROR)
        return (ERROR);
 
#ifdef  DEFAULT_STREAMS_SOCKET
    if (sockLibAdd ((FUNCPTR) strmSockLibInit, AF_INET, AF_INET) == ERROR)
        return (ERROR);
#endif  /* DEFAULT_STREAMS_SOCKET */
#endif  /* INCLUDE_STREAMS_SOCKET */
 
#if defined(INCLUDE_STREAMS_TLI) || defined(INCLUDE_STREAMS_ALL)
    tliInit ();                 /* initialize timod and tirdwr modules */
#endif  /* INCLUDE_STREAMS_TLI */
 
#if defined(INCLUDE_STREAMS_AUTOPUSH) || defined(INCLUDE_STREAMS_ALL)
    autopushInit ();                 /* Include Autopush facility */
#endif  /* INCLUDE_STREAMS_AUTOPUSH */
 
#if defined(INCLUDE_STREAMS_DLPI) || defined(INCLUDE_STREAMS_ALL)
    dlpiInit ();                /* initialize the /dev/dlb devices */
#endif  /* INCLUDE_STREAMS_DLPI */
 
#if defined(INCLUDE_STREAMS_STRACE) || defined(INCLUDE_STREAMS_ALL)
    strmStraceInit (STREAMS_STRACE_OUTPUT_DIR);  /* init strace utility */
#endif  /* INCLUDE_STREAMS_STRACE */
 
#if defined(INCLUDE_STREAMS_STRERR) || defined(INCLUDE_STREAMS_ALL)
    strmStrerrInit (STREAMS_STRERR_OUTPUT_DIR);  /* init strerr utility */
#endif  /* INCLUDE_STREAMS_STRERR */
                     
#ifdef INCLUDE_STREAMS_DEBUG
    strmDebugInit ();           /* initialize the Streams debug facility */
#endif  /* INCLUDE_STREAMS_DEBUG */
 
    /* call user-provided protocol initialization hook routine */
 
    if ((strmProtoInitRtn != NULL) && ((*strmProtoInitRtn) () == ERROR))
        return (ERROR);
 
    return (OK);                /* Streams initialization complete */
    }
#endif /* INCLUDE_STREAMS */

/******************************************************************************
*
* bootpGet - get boot parameters via BOOTP.
*
* This routine retrieves a boot file name, host and target IP addresses, and 
* subnet mask from a BOOTP server, using the bootstrap protocol defined in
* RFC 1542. The IP address and subnet mask values will only be stored in the 
* boot parameters if not already specified. In order to use BOOTP, the boot 
* device indicated by <pNetDev> must be capable of sending broadcast messages. 
* Currently, only Ethernet devices and the shared-memory network drivers are 
* supported. To use the shared-memory drivers, the target IP address must 
* already be specified.
* .IP
* The routine is called when the SYSFLG_AUTOCONFIG boot flag is set and the 
* BOOTP client is included in the boot program. If the DHCP client is also
* included, that protocol is used instead.
*
* RETURNS: OK if successful, or ERROR otherwise.
*
* ERRNO: N/A
*
* SEE ALSO: bootpLib, RFC 1542, RFC 951
*/

LOCAL STATUS bootpGet 
    (
    char *pNetDev,		/* boot device */
    char *pBootDevAddr,		/* device address */
    char *pBootFile,		/* file name */
    char *pHostAddr,		/* host address */
    int  *pMask 		/* mask */
    )
    {
#ifndef INCLUDE_DHCPC
#ifdef INCLUDE_BOOTP
    struct bootpParams 		bootParams;     /* parameter descriptor */
    struct in_addr 		clntAddr; 	/* client address */
    struct in_addr 		hostAddr; 	/* server address */
    int 			subnetMask;	/* subnet mask */
#if (VX_VERSION == 55)
    struct ifnet                *pIf;
#endif

    char	bootServer [INET_ADDR_LEN];/* boot server */

    subnetMask 	      = 0;

    bzero ( (char *)&clntAddr, sizeof (struct in_addr));
    bzero ( (char *)&hostAddr, sizeof (struct in_addr));
    bzero (bootServer, INET_ADDR_LEN);
    bzero ((char *)&bootParams, sizeof (struct bootpParams));

#if (VX_VERSION == 55)
    pIf = ifunit (pNetDev);
    if (pIf == NULL)
        return (ERROR);
#endif

    /* Need inet address to boot over the backplane */

    if ( (strncmp (pNetDev, "bp", 2) == 0) || 
            (strncmp (pNetDev, "sm", 2) == 0))
	{
	if (pBootDevAddr [0] == EOS)
	    return (ERROR);

        clntAddr.s_addr = inet_addr (pBootDevAddr);
        if (clntAddr.s_addr == (ULONG)ERROR)
            return (ERROR);
	}

    /* Set pointers to retrieve needed boot parameters. */

#if (VX_VERSION == 55)
#else
    bootParams.clientAddr = &clntAddr;    /* pBootDevAddr */
    bootParams.bootHostAddr = &hostAddr;     /* pHostAddr */
    bootParams.bootfile = pBootFile;
#endif

    bootParams.netmask = (struct in_addr *)&subnetMask;

    printf ("Getting boot parameters via network interface %s", pNetDev);

#if (VX_VERSION == 55)
    if (bootpLibInit (BOOTP_MAX_HDRSIZE) == ERROR)
        return (ERROR);
    if (bootpParamsGet (pIf, BOOTP_MAX_REQUESTS, &clntAddr, &hostAddr,
        NULL, pBootFile, &bootParams) == ERROR)
        return (ERROR);

    inet_ntoa_b (hostAddr, bootServer);
#else
    if (bootpParamsGet (pNetDev, 0, 0, &bootParams) == ERROR)
        return (ERROR);

    inet_ntoa_b (*bootParams.bootHostAddr, bootServer);
#endif

    printf ("\nBootp Server:%s\n", bootServer);

    if (pBootFile [0] == EOS)
	return (ERROR);				/* no bootfile */

    printf ("    Boot file: %s\n", pBootFile);

    /* copies to params.had */

    if (pHostAddr [0] == EOS)			/* fill in host address */
	{
	strncpy (pHostAddr, bootServer, INET_ADDR_LEN);
	printf ("    Boot host: %s\n", pHostAddr);
	}

    /*
     * copies to pBootDevAddr (params.ead or params.bad) if not using bp or sm 
     * drivers and address is not already present.
     */

    if (pBootDevAddr [0] == EOS)		/* fill in inet address */
	{
#if (VX_VERSION == 55)
        inet_ntoa_b (clntAddr, pBootDevAddr);
#else
        inet_ntoa_b (*bootParams.clientAddr, pBootDevAddr);
#endif
	printf ("    Boot device Addr (%s): %s\n", pNetDev, pBootDevAddr);
	}

    /* copies to netmask */
    if ((*pMask == 0) && (subnetMask != 0))
	{
        *pMask = ntohl (subnetMask);
	printf ("    Subnet mask: 0x%x\n", *pMask);
	}

    return (OK);
#else
    printf ("automatic address assignment requested but not included.\n");
    return (ERROR);
#endif
#else
    return (OK);            /* DHCP client used instead. */
#endif
    }

#ifdef INCLUDE_DHCPC
/******************************************************************************
*
* dhcpGet - get boot parameters with DHCP 
*
* This routine retrieves a boot file name, host and target IP addresses, and 
* subnet mask from a DHCP or BOOTP server, using the lease negotation process
* defined in RFC 1541. The IP address and subnet mask values will only be 
* stored in the boot parameters if not already specified. The DHCP client will 
* select the longest offered lease which exceeds the DHCPC_MIN_LEASE value. 
* Any DHCP lease will be given preference over BOOTP replies. Unless a 
* specific lease duration is provided in the target IP address entry, the 
* client requests the lease length defined by DHCPC_DEFAULT_LEASE. The client 
* will collect additional DHCP offers until the interval specified by 
* DHCPC_OFFER_TIMEOUT expires.
* .IP
* The <pNetDev> argument indicates the network device which will be used to
* send and receive DHCP messages. The DHCP client only supports devices
* attached to the IP protocol with the MUX/END interface. The MTU size of the
* network interface must be large enough to receive an IP datagram of 576
* bytes and the device also must be capable of sending broadcast messages.
* Finally, the target IP address must already be specified to use the
* shared-memory driver.
* .IP
* This routine executes when the SYSFLG_AUTOCONFIG boot flag is set and the
* DHCP client is included in the boot program, whether or not the BOOTP client
* is also available.
*
* NOTE
* The boot file to be loaded must also contain the DHCP client library in 
* order to continue using the assigned target IP address. In addition, the 
* DHCP server included with Windows NT does not supply boot file names. If 
* that server is used to supply the boot parameters, the boot file name must 
* be entered manually.
*
* RETURNS: OK if successful, or ERROR otherwise.
*
* ERRNO: N/A
*
* SEE ALSO: dhcpcBootLib, RFC 1541
*/

LOCAL STATUS dhcpGet
    (
    char * 		pNetDev, 	/* boot device */
    char * 		pBootDevAddr, 	/* device IP address */
    char * 		pBootFile, 	/* boot file name */
    char * 		pHostAddr, 	/* host IP address */
    int * 		pMask, 		/* target subnet mask */
    DHCP_LEASE_DATA * 	pDhcpLease 	/* lease times and addresses */
    )
    {
    STATUS 		result;
    struct ifnet * 	pIf; 		/* pointer to network interface data */
    char 		serverAddr [INET_ADDR_LEN];   /* DHCP server address */
    char 		bootFile [BOOT_FILE_LEN]; 	/* name of boot file */
    int 		subnetMask; 			/* subnet mask */
    void *              pCookie;

    struct dhcp_param  bootParams;

    bzero (serverAddr, INET_ADDR_LEN);
    bzero ( (char *)&bootParams, sizeof (struct dhcp_param));
 
    /* 
     * Using pBootFile directly only works if all the DHCP servers supply a 
     * bootfile. The Windows NT server does not, so we can't do this.  
     */
    /* bootParams.file = pBootFile;  - Desired assignment to save memory. */

    bootParams.file = bootFile;
    bootParams.subnet_mask = (struct in_addr *)&subnetMask;

    /* Need target IP address to boot over the backplane */

    if ( (strncmp (pNetDev, "bp", 2) == 0) || 
            (strncmp (pNetDev, "sm", 2) == 0))
        {
        if (pBootDevAddr [0] == EOS)
            return (ERROR);
        }

    pIf = ifunit (pNetDev);
    if (pIf == NULL)
        return (ERROR);

    printf ("Getting boot parameters via network interface %s.\n", pNetDev);

    /* Setup client to retrieve address information from a DHCP server. */

    pCookie = dhcpcBootInit (pIf, DHCPC_SPORT, DHCPC_CPORT, DHCPC_MAX_MSGSIZE,
		   DHCPC_OFFER_TIMEOUT,DHCPC_DEFAULT_LEASE,DHCPC_MIN_LEASE );
    if (pCookie == NULL)
        {
        printf ("Error initializing DHCP client.\n");
        return (ERROR);
        }

    /* Set requested lease length to value from bootline. */
 
    dhcpcOptionAdd (pCookie, _DHCP_LEASE_TIME_TAG, sizeof (int),
                    (UCHAR *)&pDhcpLease->lease_duration);

    if (pBootDevAddr[0] == EOS)
        {
        /* Attempt to retrieve address information from a DHCP server. */
 
        result = dhcpcBootBind ();
        if (result != OK)
            return (ERROR);
        }
    else
        {
        /*
         * Externally configured address. Get any additional parameters.
         * Ignore any failure (since the network device can be configured)
         * as long as a boot file is available.
         */
 
        result = dhcpcBootInformGet (pBootDevAddr);
 
        if (result != OK)
            {
            if (pBootFile[0] == EOS)
                return (ERROR);
            else
                return (OK);
            }
        }

    result = dhcpcBootParamsGet (&bootParams);
    if (result == ERROR)
      return (ERROR);

    /* Fill in configuration parameters for storage in bootline. */

    if (pBootDevAddr[0] == EOS)
        {
        /*
         * If the DHCP process established a lease (which includes an IP
         * address assignment), get the assigned address and timestamp
         * values. This information is not available if an address is
         * assigned externally. (A DHCP inform message is sent in that case).
         */
 
        bcopy ( (char *)&bootParams.yiaddr, (char *)&pDhcpLease->yiaddr,
               sizeof (struct in_addr));
        pDhcpLease->lease_duration = bootParams.lease_duration;
        pDhcpLease->lease_origin = bootParams.lease_origin;
        }

    inet_ntoa_b (bootParams.server_id, serverAddr);
    printf ("\nDHCP Server:%s\n", serverAddr);

    if (pBootFile [0] == EOS && bootFile[0] == EOS)
        return (ERROR);                         /* no bootfile */

    if (bootFile[0] != EOS)                     /* Save new bootfile */
        {
        bcopy (bootFile, pBootFile, BOOT_FILE_LEN);
        printf ("    Boot file: %s\n", pBootFile);
        }

    if (pHostAddr [0] == EOS)                   /* fill in host address */
        {
        inet_ntoa_b (bootParams.siaddr, pHostAddr);
        printf ("    Boot host: %s\n", pHostAddr);
        }

    /* 
     * Fill in the target's IP address, if needed. The status 
     * variable indicates the source of the IP address as follows:
     *    DHCP_NATIVE - assigned by a DHCP server
     *    DHCP_BOOTP - issued by a BOOTP server
     *    DHCP_MANUAL - entered in boot parameters
     */

    if (pBootDevAddr [0] == EOS)                /* fill in inet address */
        {
        /*
         * Use the IP address from the DHCP protocol.
         * The status variable has already been set.
         */

        inet_ntoa_b (bootParams.yiaddr, pBootDevAddr);
        printf ("    Boot device Addr (%s): %s\n", pNetDev, pBootDevAddr);
        }

    if ( (*pMask == 0) && (subnetMask != 0))
        {
        *pMask = ntohl (subnetMask);
        printf ("    Subnet mask: 0x%x\n", *pMask);
        }

    return (OK);
    }
#endif             /* INCLUDE_DHCPC */

#ifdef ETHERNET_ADR_SET
/*******************************************************************************
*
* mEnet - modify the last three bytes of the ethernet address
*
* RETURNS: void
*
* NOMANUAL
*/

void mEnet 
    (
    char * 	pNum   /* Boot line, including unit number. */
    )
    {
    uchar_t byte [MAX_ADR_SIZE];	/* array to hold new Enet addr */
    uchar_t curArr [MAX_ADR_SIZE];	/* array to hold current Enet addr */
    char line [MAX_LINE + 1];
    char *pLine;		/* ptr to current position in line */
    int value;			/* value found in line */
    char excess;
    char *buf;
    int unitNum;
    int ix;

    /* Search for unit number of network device. */

    buf = pNum;
    if (bootScanNum (&buf, &unitNum, FALSE) != OK)  /* Use 0 if no unit #. */
       unitNum = 0;

    sysEnetAddrGet (unitNum, curArr);
    printf ("Current Ethernet Address is: ");

#if _BYTE_ORDER == _BIG_ENDIAN
    printf ("%02x:%02x:%02x:%02x:%02x:%02x\n", curArr[5], 
	    curArr[4], curArr[3], curArr[2], 
	    curArr[1], curArr[0]);
    byte[5] = ((ENET_DEFAULT & 0x0000ff00) >> 8);
    byte[4] = ((ENET_DEFAULT & 0x00ff0000) >> 16);
    byte[3] = ((ENET_DEFAULT & 0xff000000) >> 24);
    byte[2] = curArr[2];
    byte[1] = curArr[1];
    byte[0] = curArr[0];
#else  /* _BYTE_ORDER == _LITTLE_ENDIAN  */
    printf ("%02x:%02x:%02x:%02x:%02x:%02x\n", curArr[0], 
	    curArr[1], curArr[2], curArr[3], 
	    curArr[4], curArr[5]);
    byte[5] = ((ENET_DEFAULT & 0x00ff0000) >> 16);
    byte[4] = ((ENET_DEFAULT & 0x0000ff00) >> 8);
    byte[3] = (ENET_DEFAULT & 0x000000ff);
    byte[2] = curArr[3];
    byte[1] = curArr[4];
    byte[0] = curArr[5];
#endif /* _BYTE_ORDER == _BIG_ENDIAN */

    printf ("Modify only the last 3 bytes (board unique portion) of Ethernet Address.\n");
    printf ("The first 3 bytes are fixed at manufacturer's default address block.\n");

    for (ix = 5; ix > 2; ix--)
        printf ("%02x- %02x\n", byte[ix], byte[ix]);


    /* start on fourth byte of enet addr */
    for (ix = 2; ix >= 0; ix --)	
	{
	/* prompt for substitution */

	printf ("%02x- ", byte[ix]);

	/* get substitution value:
	 *   skip empty lines (CR only);
	 *   quit on end of file or invalid input;
	 *   otherwise put specified value at address */

	if (fioRdString (STD_IN, line, MAX_LINE) == EOF)
	    break;

	line [MAX_ADR_SIZE] = EOS;	/* make sure input line has EOS */

	for (pLine = line; isspace ((int) *pLine); ++pLine)	/* skip leading spaces*/
	    ;

	if (*pLine == EOS)			/* skip field if just CR */
	    continue;

	if (sscanf (pLine, "%x%1s", &value, &excess) != 1)
	    break;				/* quit if not number */

	byte[ix]  = (uchar_t)value;		/* assign new value */
	}

    printf ("\n");

    sysEnetAddrSet (byte[5], byte[4], byte[3], byte[2], byte[1], byte[0]);

    printf ("New Ethernet Address is: ");
#if _BYTE_ORDER == _BIG_ENDIAN
    for (ix = 5; ix > 0; ix--)
        printf ("%02x:", byte[ix]);
    printf ("%02x\n", byte[0]);
#else  /* _BYTE_ORDER == _LITTLE_ENDIAN */
    for (ix = 5; ix > 0; ix--)
        printf ("%02x:", byte[ix]);
    printf ("%02x\n", byte[0]);
#endif /* _BYTE_ODER == _BIG_ENDIAN */
    }
#endif  /* ETHERNET_ADR_SET */

/*******************************************************************************
*
* usrNetProtoInit - configure the various protocols
*
* This routine configures various protocols of the network system.
* This function should be called before netLibInit() at the initialization
* time.
*
* RETURNS: OK, or ERROR if the protocol initialization fails
*
* NOMANUAL
*/

LOCAL STATUS usrNetProtoInit (void)
    {
    ipLibInit (&ipCfgParams);           /* has to included by default */
    rawIpLibInit ();                    /* has to included by default */
    rawLibInit ();

#ifdef INCLUDE_UDP
    udpLibInit (&udpCfgParams);         /* udp protocol initialization */
#endif

#ifdef INCLUDE_TCP
    tcpLibInit (&tcpCfgParams);         /* tcp protocol initialization */
#endif

#ifdef INCLUDE_ICMP
    icmpLibInit (&icmpCfgParams);       /* icmp protocol initialization */
#endif

#ifdef INCLUDE_IGMP
    igmpLibInit ();                     /* igmp protocol initialization */
#endif

#ifdef INCLUDE_OSPF
    ospfLibInit ();
#endif

    return (OK);
    }


#endif  /* INCLUDE_NETWORK */
