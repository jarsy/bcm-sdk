/* usrNetwork.c - network initialization */

/* Copyright 1992 - 2001 Wind River Systems, Inc. */

/* $Id: usrNetwork.c,v 1.6 2011/07/21 16:14:21 yshtil Exp $
modification history
--------------------
06a,20dec01,ros  fix for spr 72294
05z,04jun01,rae  move init for MIB2_IF, to make boot interface visible
05y,07feb01,spm  added merge record, corrected changes, and fixed modification
                 history for 30jan01 update from version 05j of tor2_0_x branch
                 (base 04z); general overhaul of telnet server (SPR #28675)
05x,30jan01,ijm  merged SPR# 28602 fixes for proxy ARP (versions 05b and 05i
                 of tor2_0_x branch - SPR #7576, #8993, #28601, and #28632)
05w,16nov00,spm  enabled new DHCP lease for runtime device setup (SPR #20438)
05v,14nov00,rae  removed unused argument from sockLibAdd()
05u,27oct00,spm  fixed DHCP startup update and mod history entry
05t,27oct00,spm  code cleanup; removed incorrect proxy server requirement
05s,23oct00,niq  updated DHCP startup for latest version (from tor3_x branch)
05r,06oct00,spm  fixed device search with non-zero unit numbers (SPR #26154)
05q,03may99,pul  max_linkhdr to be set in usrNetInit 
05p,29apr99,pul  Upgraded NPT phase3 code to tor2.0.0
05o,02apr99,pul  "To make max_linkhdr user configurable"
05n,26mar99,pul  remove muxMCastMapAdd
05m,19mar99,dat  SPR 25877 removed old driver support
05l,18mar99,sj   removed INCLUDE_NPT check
05k,16mar99,dat  SPR 25790, boot from END or netif device
05j,09mar99,ham  rejected the 05e modification.
05i,08mar99,ham  added boot device "pcmcia" entry, SPR#24714.
05h,05mar99,sj   eliminated call to muxTkLibInit. Must set muxMaxBinds
05g,26feb99,dat  changed netIf to usrNetIfTbl and made it global. Added FEI
		 support (23818)
05f,02feb99,sgv  added bitflag parameter to sockLibAdd
05e,20jan99,scb  usrBpInit() modified to use "sm=" before SM_ANCHOR_ADRS (23035)
05d,03Nov98,pul  moved muxBindBase to muxTkLib.c 
05c,30nov98,c_s  20x -> 200 merge
05b,30nov98,gnn  corrected mod history and merged TFFS support
05a,07oct98,sj   added Network Protocol Toolkit(NPT) support 
04z,11sep98,spm  corrected default value of RIP_GARBAGE_TIME (SPR #21908)
04y,25aug98,n_s  fixed interface config when using bootline other field and END.
                 spr #22217.
04x,04may98,cym  added Windows ULIP code.
04w,29apr98,dbt  removed RDB references (no longer supported).
04v,25sep97,jmb  Add a default route for HPSIM SLIP connection.
04u,10jul97,jmb  Make NFS client work on HPSIM by setting nfsMaxMsgLen
		 to a number less than the MTU - 127.
04t,27jan97,jmb  Add SLIP patches for HPSIM (from Mark Mason).
04s,18jul98,spm  merged support for TFFS boot device into release view
04r,05dec97,spm  added DHCP code review modifications
04q,30nov97,vin  added network stack system pool
04p,29oct97,spm  delayed DHCP client startup to prevent failure for fast 
                 processors (SPR #9641)
04o,06oct97,spm  moved DHCP server default parameters to dhcpsLib.h; added
                 stub routine to support delayed startup of DHCP relay agent
04n,06oct97,spm  removed unused constant INCLUDE_BSD; added optional binary 
                 compatibility for BSD 4.3 applications
04m,03oct97,gnn  removed references to endDriver global
04l,25sep97,gnn  SENS beta feedback fixes
04k,19sep97,vin  added cluster blk configuration to network configuration.
		 changed MBLK_CONFIG to M_CL_CONFIG.
04j,26aug97,spm  major overhaul of DHCP: reorganized code and changed user 
                 interface to support multple leases at runtime; improved
                 handling of address information by client; altered all DHCP 
                 components to allow selection of UDP ports
04i,14aug97,vin  changed MBUF_CONFIG to MBLK_CONFIG
04h,12aug97,gnn  changes necessitated by MUX/END update.
04g,11aug97,vin  changed mbufConfig and clDescTbl.
04d,06aug97,cdp  fixed signed/unsigned comparison in checkInetAddrField.
04f,15jul97,spm  added support for SNTP client and server
04e,03jul97,vin  made routing sockets scalable.
04d,03jul97,sgv  fix for spr #8602 changed ftpdInit to check for 
		 authentication when INCLUDE_FTPD_SECURITY is defined
04c,02jun97,spm  allowed DHCP server to start without lease storage hook
04b,20may97,gnn  fixed SPR 8627 so that multiple gn devices are supported.
04a,09may97,spm  increased stack size for DHCP server task
03z,30apr97,spm  made hop limit for relay agent configurable; fixed DHCP bugs
03y,25apr97,gnn  changed INCLUDE_GN to INCLUDE_END
03x,18apr97,spm  eliminated DHCP-related compiler warnings when building BSP
03w,17apr97,gnn  changed network booting to work with END configNet files.
                 fixed two bugs that ANVL found in RIP timers.
03v,17apr97,vin  added ipFilterLibInit() in usrNetProtoInit().
03u,14apr97,vin  added mCastRouteLibInit(), igmpShowInit, icmpShowInit,
		 tcpShowInit, udpShowInit for scalability.
03t,10apr97,jag  added DNS resolver support
03s,07apr97,gnn  fixed a merge bug with Stephen's changes.
03r,07apr97,gnn  added new booting system for END network device.
03q,07apr97,spm  added DHCP server and relay agent support, fixed unit number 
                 support, removed redundant code for DHCP client
03p,12mar97,gnn  added new timer variables for ripLibInit
03o,24feb97,gnn  added RIP_MULTICAST for use by RIP
                 changed routedLibInit to ripLibInit
03n,14feb97,rjc  OSPF_INCLUDE changed to INCLUDE_OSPF
03m,14feb97,rjc  added ospf init calls
03l,07feb97,rjc  deleted routePref, now in routeLib.c
03k,31jan97,vin  added macros for config parameters.
03j,31jan97,spm  added dhcpShowInit() to link DHCP client show routines.
03i,29jan97,gnn  moved the muxDevLoad call to later in the sequence.
03h,21jan97,vin  added scalability of network stack, 
		 moved _func_remCurId[SG]et from netLib.c for scalability.
03g,21jan97,gnn  Added another argument to muxDevLoad().
03d,13jan97,jpd  Added if_lnEbsa driver entry.
03c,07jan97,jpd  Added if_oli driver entry.
03f,20dec96,vin	 removed warnings muxDevLoad().
03e,18dec96,spm  fixed small bug in DHCP client code
03d,03dec96,spm  removed bootLeaseExtract test when DHCP client not included
                 to reduce code size
03c,27nov96,spm  added support for DHCP client.
03b,26nov96,gnn  added INCLUDE_RIP directive for routed/rip.
03a,25nov96,vin	 added new cluster configuration.
02z,15nov96,vin  fixed problems with passive and silent options for PPP.
02y,23oct96,gnn  added END_DEV_NAME define for gnattach routine.
02x,22oct96,gnn  added END_BUFF_LOAN define so upper layer can request buffer
                 loaning.
03d,10dec97,spm  added configurable password authentication for FTP server
                 from SENS branch (SPR #8602)
03c,11apr97,dvs  added routes for SIMSPARCSOLARIS when using ppp instead of ULIP
03b,11dec96,vin  SPR7604 for PPP silent mode and passive mode.
03a,10dec96,yp   added route for ULIP driver so we send packets for our selves
		 to localhost since not all hosts return whats ours
02z,18nov96,dbt  ANSIfied function headers (SPR #7427.)
02y,11nov96,spm  added bzero() call to checkInetAddrField(). (SPR #6326).
02x,01nov96,hdn  added support for PCMCIA.  moved NETIF to sysLib.h.
02w,13aug96,dat  added NETIF_USR_DECL and NETIF_USR_ENTRIES, SPR 6970
02v,23jul96,hdn  added support for ATA driver.
02u,21jun96,jmb  long modhist -- deleted entries prior to 1994. (SPR #6528)
02t,13jun96,hdn  added INCLUDE_ESMC for SMC91c9x Ethernet driver.
02v,12aug96,rjc  snmp init funcs modified to reflect removal of memory partition                 options. 
02u,07aug96,rjc  added resolver changes.
02t,29may96,dat	 fixed SPR 4392 re: network services after scsi (disk) boot
02s,04apr96,rjc  Merged in changes for snmp agent for vxWorks53
02r,07mar96,gnn	 added SLIP_MTU so that we could configure the MTU. 
		 (SPR #4652)
02q,06mar96,sgv  Added error checks in usrStrmInit for tliInit(), dlpiInit()
		 autopushInit() and strmDebugInit.
02p,11nov95,vin  baudrate in bootstring, added pppHookLib.
02n,06sep95,jag  deleted call to fattachInit (), and changed iosStrmInit to
		 strmInit ().
02n,01aug95,dzb  changed STREAMS options to go through iosStrmInit ().
                 added protocol initialization hook routine.
 		 PPP initialization failure message (SPR #4505).
02m,28jul95,jag  changed INCLUDE_STREAMS_LOG to INCLUDE_STREAMS_STRACE and
		 INCLUDE_STREAMS_STRERR. Added the routines to support these
		 options.
02l,25jul95,dzb  tweaked socket library initialization.
02k,25jul95,dzb  added socket library init for BSD vs. STREAMS.
02j,25jul95,jag  Added check for the return value of iosStrmInit.
02i,18jul95,dzb  wait for PPP interface to come up in usrPPPInit().
02h,17jul95,dzb  fixed ordering of boot params search.
02g,26jun95,dzb  removed PPP_S_USEHOSTNAME option.
02f,22jun95,sgv  Added streamsInit() to Initialise STREAMS
02e,21jun95,dzb  Added INCLUDE_PPP_CRYPT for unbundled crypt() support.
02d,20jun95,dzb  changed PPP options structure to set values from macros.
02c,09jan95,dzb  Added PPP support.
02c,29jun95,caf  added nicEvb driver entry.
02b,28mar95,kkk  changed smNetInit() parameters to use macros from 
		 configAll.h (SPR #4130)
02z,27mar95,jag  removed dependency of INCLUDE_NFS_SERVER from INCLUDE_NFS
01z,19mar95,dvs  removed tron references - no longer supported.
01y,30jan95,hdn  included nfsdLib.h if INCLUDE_NFS_SERVER is defined.
		 added 6th argument to nfsdInit ().
02b,31mar95,caf  added dc driver entry.
02a,02jan95,kat  added ilac driver entry.
01x,09dec94,jag  Added INCLUDE_MIB2_AT
01w,13nov94,dzb  fixed placement of INCLUDE_PING.
01v,17aug94,dzb  fixed setting the gateway for a slip connection (SPR #2353).
		 added qu netif device.  added zbuf socket interface init.
		 added ping init.  added CSLIP support.  added tcpTrace init.
01u,11nov94,jag  divided SNMP and MIB-II support.
01t,09feb94,hdn  added support for if_elt 3COM Etherlink III driver.
		 added support for if_ene Eagle NE2000 driver.
*/

/*
DESCRIPTION
This file is used to configure and initialize the VxWorks networking support.
This file is included by usrConfig.c.

SEE ALSO: usrExtra.c

NOMANUAL
*/

#ifndef  __INCusrNetworkc
#define  __INCusrNetworkc

#include "zbufSockLib.h"
#include "pingLib.h"
#include "ipProto.h"

#ifdef INCLUDE_SNTPS
#include "sntpsLib.h"

/* Include this declaration if hook is defined. */

/* IMPORT STATUS SNTPS_TIME_HOOK (int, void *); */

#endif

#define DEF_MAX_LINK_HDR 16 

#ifndef USR_MAX_LINK_HDR
#define USR_MAX_LINK_HDR DEF_MAX_LINK_HDR
#endif

IMPORT int max_linkhdr;

#ifdef INCLUDE_SNTPC
#include "sntpcLib.h"
#endif

#ifdef INCLUDE_SNMPD
#include "snmp/snmpdInit.h"
#endif

#include "m2Lib.h"
#include "private/funcBindP.h"

#ifdef INCLUDE_OSPF
#include "ospf/ospfLib.h"
#endif /* INCLUDE_OSPF */

#ifdef INCLUDE_NFS_SERVER
#include "nfsdLib.h"
#endif

#ifdef INCLUDE_DNS_RESOLVER
#include "resolvLib.h"
#endif

#ifdef INCLUDE_PROXY_CLIENT
#include "proxyLib.h"
#endif  /* INCLUDE_PROXY_CLIENT */

#if (CPU==SIMHPPA)
extern int nfsMaxMsgLen;
#endif
#ifdef INCLUDE_FTPD_SECURITY
#include "loginLib.h"
#endif

#if defined (INCLUDE_NFS) || defined (INCLUDE_NFS_SERVER)
const int nfsMaxPath = NFS_MAXPATH + 1;
#endif

#ifdef INCLUDE_NFS_SERVER
const int nfsMaxFileName = NFS_MAXFILENAME;
#endif

#ifdef INCLUDE_DHCPS
#include "dhcp/dhcp.h"
#include "dhcp/common.h"
#include "dhcpsLib.h"
IMPORT long dhcps_dflt_lease;
IMPORT long dhcps_max_lease;
IMPORT STATUS dhcpsLeaseHookAdd (FUNCPTR);
IMPORT STATUS dhcpsAddressHookAdd (FUNCPTR);

/* Include these declarations if hooks are defined. */

/* IMPORT STATUS DHCPS_LEASE_HOOK (int, char *, int); */

/* IMPORT STATUS DHCPS_ADDRESS_HOOK (int, char *, int); */

#endif

#ifdef INCLUDE_DHCPC
#include "dhcp/dhcpcInit.h"
#include "dhcp/dhcpcCommonLib.h"
#else
LOCAL STATUS dhcpcBootLineClean (char *);
#endif /* INCLUDE_DHCPC */

#ifdef INCLUDE_DHCPR
#ifndef INCLUDE_DHCPS        /* Server is superset of relay agent. */
#include "dhcprLib.h"
#endif
#endif

#ifdef INCLUDE_END
#include "end.h"
#include "muxLib.h"
#include "muxTkLib.h"
#include "configNet.h"
#include "m2Lib.h"
#endif /* INCLUDE_END */

#if (defined (INCLUDE_DHCPC) || defined (INCLUDE_DHCPS) || \
     defined (INCLUDE_RIP) || defined (INCLUDE_OSPF)) \
     && !defined (INCLUDE_ROUTE_SOCK)
#define INCLUDE_ROUTE_SOCK
#endif /* INCLUDE_DHCPC || INCLUDE_DHCPS */

#ifdef INCLUDE_SLIP
#if (CPU==SIMHPPA)
extern void sysSlipSetup ();
#endif /* (CPU=SIMHPPA) */
#endif /* INCLUDE_SLIP */

#ifdef INCLUDE_END
#ifdef INCLUDE_NT_ULIP
extern int ntResolv ();
#endif /* INCLUDE_NT_ULIP */
#endif /* INCLUDE_END */

/* global variables */

/* network protocol configuration parameters */

#ifdef INCLUDE_TCP
/* Workaround Broadcom's PR 17762 */
#undef TCP_SND_SIZE_DFLT
#define TCP_SND_SIZE_DFLT 0x9000
#undef TCP_RCV_SIZE_DFLT
#define TCP_RCV_SIZE_DFLT 0x9000

TCP_CFG_PARAMS tcpCfgParams =	/* tcp configuration parameters */
    {
    TCP_FLAGS_DFLT,		/* include rfc1323 support */
    TCP_SND_SIZE_DFLT,		/* default send buffer size */
    TCP_RCV_SIZE_DFLT,		/* default recv buffer size */
    TCP_CON_TIMEO_DFLT,		/* initial connection time out */
    TCP_REXMT_THLD_DFLT,	/* retransmit threshold */
    TCP_MSS_DFLT,		/* default maximum segment size */
    TCP_RND_TRIP_DFLT,		/* default round trip time */
    TCP_IDLE_TIMEO_DFLT,	/* idle timeouts before first probe */
    TCP_MAX_PROBE_DFLT		/* max no. probes before dropping */
    }; 
#endif /* INCLUDE_TCP */

#ifdef INCLUDE_UDP
UDP_CFG_PARAMS udpCfgParams =	/* udp configuration parameters */
    {
    UDP_FLAGS_DFLT,
    UDP_SND_SIZE_DFLT,		/* send buffer size */
    UDP_RCV_SIZE_DFLT		/* recv buffer size */
    }; 
#endif /* INCLUDE_UDP */

#ifdef INCLUDE_ICMP
ICMP_CFG_PARAMS icmpCfgParams = /* icmp configuration parameters */
    {
    ICMP_FLAGS_DFLT		/* no icmp mask replies by default */
    }; 
#endif	/* INCLUDE_ICMP */

IP_CFG_PARAMS ipCfgParams =	/* ip configuration parameters */
    {
    IP_FLAGS_DFLT,		/* default ip flags */
    IP_TTL_DFLT,		/* ip default time to live */
    IP_QLEN_DFLT,		/* default ip intr queue len */
    IP_FRAG_TTL_DFLT		/* default ip fragment time to live */
    }; 

/* network buffers configuration */

/*
 * mBlk, clBlk configuration table for network stack data pool.
 * Only used for data transfer in the network stack.
 */
M_CL_CONFIG mClBlkConfig = 
    {
    /* 
    no. mBlks		no. clBlks	memArea		memSize
    -----------		----------	-------		-------
    */
    NUM_NET_MBLKS, 	NUM_CL_BLKS,	NULL, 		0
    };

/*
 * network stack data cluster pool configuration table
 * Only used for data transfer in the network stack.
 */
CL_DESC clDescTbl [] = 		
    {
    /* 
    clusterSize		num		memArea		memSize
    -----------		----		-------		-------
    */
    {64,		NUM_64,		NULL,		0},
    {128,		NUM_128,	NULL,		0},
    {256,		NUM_256,	NULL,		0},
    {512,		NUM_512,	NULL,		0},
    {1024,		NUM_1024,	NULL,		0},
    {2048,		NUM_2048,	NULL,		0}
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
    NUM_SYS_MBLKS, 	NUM_SYS_CL_BLKS,	NULL, 		0
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
    clusterSize		num		memArea		memSize
    -----------		----		-------		-------
    */
    {64,		NUM_SYS_64,	NULL,		0},
    {128,		NUM_SYS_128,	NULL,		0},
    {256,		NUM_SYS_256,	NULL,		0},
    {512,		NUM_SYS_512,	NULL,		0},
    }; 
int sysClDescTblNumEnt = (NELEMENTS(sysClDescTbl));

#if defined(INCLUDE_STREAMS) || defined(INCLUDE_STREAMS_ALL)

#ifdef	STREAMS_PROTO_INIT_RTN
FUNCPTR	strmProtoInitRtn = (FUNCPTR) STREAMS_PROTO_INIT_RTN;
#endif	/* STREAMS_PROTO_INIT_RTN */
#endif /* INCLUDE_STREAMS || INCLUDE_STREAMS_ALL */

#ifdef INCLUDE_END	
#ifndef IP_MAX_UNITS 
#define IP_MAX_UNITS 1
#endif

#ifndef MUX_MAX_BINDS
#define MUX_MAX_BINDS 16
#endif /* MUX_MAX_BINDS */

IP_DRV_CTRL ipDrvCtrl [IP_MAX_UNITS]; 
int ipMaxUnits = IP_MAX_UNITS;

/*
 * These parameters are required when using the routines arpShow, routeShow or
 * mRouteShow either through a remote session like telnet or through the host
 * shell using WDB_COMM_NETWORK. Here RT_BUFFERED_DISPLAY should be set to
 * TRUE. This will cause the route entries to be stored in a buffer before
 * being displayed. RT_DISPLAY_MEMORY sets the size of the buffer in bytes.
 * Some entries may not be displayed depending on the value of this parameter.
 */

#ifdef INCLUDE_NET_SHOW
#define RT_DISPLAY_MEMORY   10000
#define RT_BUFFERED_DISPLAY FALSE
#endif

#else	/* INCLUDE_END not defined */
IP_DRV_CTRL  	ipDrvCtrl[1]; 
#endif 	/* INCLUDE_END */


#ifdef	INCLUDE_PPP

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
    PPP_OPTIONS_FLAGS,              /* flags field */
    PPP_STR_ASYNCMAP,               /* Set the desired async map */
    PPP_STR_ESCAPE_CHARS,           /* Set chars to escape on transmission */
    PPP_STR_VJ_MAX_SLOTS,           /* Set max VJ compression header slots */
    PPP_STR_NETMASK,                /* Set netmask value for negotiation */
    PPP_STR_MRU,                    /* Set MRU value for negotiation */
    PPP_STR_MTU,                    /* Set MTU value for negotiation */
    PPP_STR_LCP_ECHO_FAILURE,       /* Set max consecutive LCP echo failures */
    PPP_STR_LCP_ECHO_INTERVAL,      /* Set time for LCP echo requests */
    PPP_STR_LCP_RESTART,            /* Set timeout for LCP */
    PPP_STR_LCP_MAX_TERMINATE,      /* Set max # xmits for LCP term-reqs */
    PPP_STR_LCP_MAX_CONFIGURE,      /* Set max # xmits for LCP conf-reqs */
    PPP_STR_LCP_MAX_FAILURE,        /* Set max # conf-naks for LCP */
    PPP_STR_IPCP_RESTART,           /* Set timeout for IPCP */
    PPP_STR_IPCP_MAX_TERMINATE,     /* Set max # xmits for IPCP term-reqs */
    PPP_STR_IPCP_MAX_CONFIGURE,     /* Set max # xmits for IPCP conf-reqs */
    PPP_STR_IPCP_MAX_FAILURE,       /* Set max # conf-naks for IPCP */
    PPP_STR_LOCAL_AUTH_NAME,        /* Set local name for authentication */
    PPP_STR_REMOTE_AUTH_NAME,       /* Set remote name for authentication */
    PPP_STR_PAP_FILE,               /* Set the PAP secrets file */
    PPP_STR_PAP_USER_NAME,          /* Set username for PAP auth with peer */
    PPP_STR_PAP_PASSWD,             /* Set password for PAP auth with peer */
    PPP_STR_PAP_RESTART,            /* Set timeout for PAP */
    PPP_STR_PAP_MAX_AUTHREQ,        /* Set max # xmits for PAP auth-reqs */
    PPP_STR_CHAP_FILE,              /* Set the CHAP secrets file */
    PPP_STR_CHAP_RESTART,           /* Set timeout for CHAP */
    PPP_STR_CHAP_INTERVAL,          /* Set interval for CHAP rechallenge */
    PPP_STR_CHAP_MAX_CHALLENGE      /* Set max # xmits for CHAP challenge */
    };
#endif  /* INCLUDE_PPP */

/* extern declarations, IMPORTS */
#ifdef INCLUDE_MIB2_IF
IMPORT FUNCPTR  _m2SetIfLastChange;     /* def'd in if.c for scalability */
IMPORT FUNCPTR  _m2IfTableUpdate;
#endif /* INCLUDE_MIB2_IF */

/* Network interface table. */
IMPORT int lnEbsaattach ();
IMPORT int oliattach();
IMPORT int dcattach ();
IMPORT int eglattach ();
IMPORT int eiattach ();
IMPORT int feiattach ();
IMPORT int exattach ();
IMPORT int enpattach ();
IMPORT int ieattach ();
IMPORT int ilacattach ();
IMPORT int lnattach ();
IMPORT int lnsgiattach ();
IMPORT int nicattach ();
IMPORT int nicEvbattach ();
IMPORT int medattach ();
IMPORT int loattach ();
IMPORT int snattach ();
IMPORT int fnattach ();
IMPORT int elcattach ();
IMPORT int ultraattach ();
IMPORT int eexattach ();
IMPORT int eltattach ();
IMPORT int eneattach ();
IMPORT int esmcattach ();
IMPORT int quattach ();
IMPORT int slattach ();
IMPORT int pppattach ();

#ifdef NETIF_USR_DECL	/* Additional declarations, from BSP */
    NETIF_USR_DECL
#endif

/* generic driver */
#ifdef INCLUDE_END
IMPORT int 	ipAttach ();
IMPORT END_TBL_ENTRY endDevTbl[];
#endif /* INCLUDE_END */

#ifdef  INCLUDE_TCP_DEBUG
IMPORT void tcpTraceInit ();
#endif  /* INCLUDE_TCP_DEBUG */

#ifdef	INCLUDE_IF_USR
IMPORT int IF_USR_ATTACH ();
#endif	/* INCLUDE_IF_USR */

#if defined(BROADCOM_BSP)
IMPORT STATUS sysEnetAddrSet(unsigned char *pMac);
#endif


/* local typedefs */


/* variables */

NETIF usrNetIfTbl [] =	/* network interfaces */
    {
#ifdef NETIF_USR_ENTRIES	/* Additional entries, from BSP */
    NETIF_USR_ENTRIES
#endif

#ifdef	INCLUDE_LNEBSA
        { "lnEbsa", lnEbsaattach, (char*)IO_ADRS_LNEBSA, INT_VEC_LNEBSA,
          INT_LVL_LNEBSA, LNEBSA_POOL_ADRS, LNEBSA_POOL_SIZE, LNEBSA_DATA_WIDTH,
          LNEBSA_MODE, LNEBSA_DMA_CHAN },
#endif	/* INCLUDE_LNEBSA */
#ifdef	INCLUDE_OLI
	{ "oli", oliattach, (char*)IO_ADRS_NISA_BASE,
	  IO_ADRS_NISA_PCMCIA, IO_ADRS_NISA_PCMEM,
	  INT_VEC_PCMCIA_A, INT_LVL_PCMCIA_A,
	  INT_VEC_PCMCIA_B, INT_LVL_PCMCIA_B },
#endif	/* INCLUDE_OLI */
#ifdef	INCLUDE_DC
	{ "dc", dcattach, (char*)IO_ADRS_DC, INT_VEC_DC, INT_LVL_DC,
	  DC_POOL_ADRS, DC_POOL_SIZE, DC_DATA_WIDTH, DC_RAM_PCI_ADRS,
	  DC_MODE },
#endif	/* INCLUDE_DC */
#ifdef  INCLUDE_EGL
        { "egl", eglattach, (char*)IO_ADRS_EGL, INT_VEC_EGL, INT_LVL_EGL },
#endif	/* INCLUDE_EGL */
#ifdef	INCLUDE_EI
	{ "ei", eiattach, (char*)INT_VEC_EI, EI_SYSBUS, EI_POOL_ADRS, 0, 0},
#endif	/* INCLUDE_EI */
#ifdef	INCLUDE_FEI
        { "fei", feiattach, (char*)FEI_POOL_ADRS, 0, 0, 0, 0},
#endif	/* INCLUDE_FEI */
#ifdef	INCLUDE_EX
	{ "ex", exattach, (char*)IO_ADRS_EX, INT_VEC_EX, INT_LVL_EX,
	  IO_AM_EX_MASTER, IO_AM_EX },
#endif	/* INCLUDE_EX */
#ifdef	INCLUDE_ENP
	{ "enp", enpattach, (char*)IO_ADRS_ENP, INT_VEC_ENP, INT_LVL_ENP,
	  IO_AM_ENP },
#endif	/* INCLUDE_ENP */
#ifdef	INCLUDE_IE
	{ "ie", ieattach, (char*)IO_ADRS_IE, INT_VEC_IE, INT_LVL_IE },
#endif	/* INCLUDE_IE */
#ifdef	INCLUDE_ILAC
	{ "ilac", ilacattach, (char*)IO_ADRS_ILAC, INT_VEC_ILAC},
#endif	/* INCLUDE_ILAC */
#ifdef	INCLUDE_IF_USR
	{ IF_USR_NAME, IF_USR_ATTACH, IF_USR_ARG1, IF_USR_ARG2, IF_USR_ARG3,
	  IF_USR_ARG4, IF_USR_ARG5, IF_USR_ARG6, IF_USR_ARG7, IF_USR_ARG8 },
#endif	/* INCLUDE_IF_USR */
#ifdef	INCLUDE_LN
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
#endif  /* INCLUDE_NIC_EVB */
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
#ifdef  INCLUDE_QU
	{ "qu", quattach, (char*)IO_ADRS_QU_EN, INT_VEC_QU_EN, QU_EN_SCC,
	  QU_EN_TX_BD, QU_EN_RX_BD, QU_EN_TX_OFF, QU_EN_RX_OFF, QU_EN_MEM},
#endif  /* INCLUDE_QU */
#ifdef  INCLUDE_ENE
	{ "ene", eneattach, (char*)IO_ADRS_ENE, INT_VEC_ENE, INT_LVL_ENE},
#endif  /* INCLUDE_ENE */
#ifdef  INCLUDE_ESMC
	{ "esmc", esmcattach, (char*)IO_ADRS_ESMC, INT_VEC_ESMC, INT_LVL_ESMC,
	  CONFIG_ESMC, RX_MODE_ESMC},
#endif  /* INCLUDE_ESMC */
#ifdef  INCLUDE_SN
        { "sn", snattach, (char*)IO_ADRS_SN, INT_VEC_SN },
#endif	/* INCLUDE_SN */
#ifdef  INCLUDE_FN
        { "fn", fnattach },
#endif	/* INCLUDE_FN */

#ifdef  INCLUDE_SM_NET
        { "sm", smNetAttach, 0, 0, 0, 0, 0, 0 },
#endif  /* INCLUDE_SM_NET */

#ifdef	INCLUDE_PCMCIA
	{ "pcmcia", pcmciaattach, 0, 0, 0, 0, 0, 0 },
#endif	/* INCLUDE_PCMCIA */

#ifdef	INCLUDE_PPP
	{"ppp", 0, 0, 0, 0, 0},
#endif	/* INCLUDE_PPP */
#ifdef	INCLUDE_SLIP
	{"sl", 0, 0, 0, 0, 0},
#endif	/* INCLUDE_SLIP */
	{ "lo", loattach  },
	{ 0, 0, 0, 0, 0, 0 },
    };

/* forward declarations */

STATUS usrNetProtoInit (void); 

#ifdef INCLUDE_DNS_RESOLVER
LOCAL STATUS usrResolvInit (void);
#endif /* INCLUDE_DNS_RESOLVER */

#ifdef INCLUDE_NET_SHOW
const UINT rtMem = RT_DISPLAY_MEMORY;
const BOOL bufferedRtShow = RT_BUFFERED_DISPLAY;
#endif

#if defined (INCLUDE_DHCPS) || defined (INCLUDE_DHCPR)
/*
 * The following table lists the IP addresses which will receive DHCP
 * messages forwarded across network boundaries by a DHCP server or
 * relay agent.
 */
DHCP_TARGET_DESC dhcpTargetTbl [] =
    {
    /*
     IP address of DHCP target servers
     ---------------------------------
     */
     /* {"90.11.42.2"}, */
     };
LOCAL int dhcpTargetTblSize = (NELEMENTS(dhcpTargetTbl));

struct ifnet *devlist[1];

int dhcpMaxHops = DHCP_MAX_HOPS;
int dhcpSPort = DHCPS_SPORT;
int dhcpCPort = DHCPS_CPORT;
#endif

/* 
 * Example cache routine - should store records of active leases 
 *                         across reboots. 
 */

/*
#ifdef INCLUDE_DHCPS

STATUS dhcpsDummyCache (int, char *, int);

STATUS dhcpsDummyCache (int op, char *name, int length)
    {
    printf("Cache called with op %d on %d bytes.\n", op, length);
    if (op == DHCPS_STORAGE_READ)
        return (ERROR);          /@ Simulated end-of-file. @/
    return (OK);
    }
#endif
*/

#ifdef INCLUDE_DHCPS

DHCPS_LEASE_DESC dhcpsLeaseTbl [] =
    {
    /*
    Name        Start IP      End IP          Parameters
    ----        ----------    ------          -----------
                                     see man pages
                                (timers, bootfile, etc.)
    */

    /*
     * Host requirements defaults needed for RFC compliance - DO NOT REMOVE!!
     */

   {"dflt",    NULL, NULL, DHCPS_DEFAULT_ENTRY},

    /* Sample database entries. */

/* {"ent1", "90.11.42.24", "90.11.42.24", "clid=\"1:0x08003D21FE90\":maxl=90:dfll=60"},   */
/* {"ent2", "90.11.42.25", "90.11.42.26", "snmk=255.255.255.0:maxl=90:dfll=70:file=/vxWorks"}, */
/* {"ent3", "90.11.42.27", "90.11.42.27", "maxl=0xffffffff:file=/vxWorks"}, */
/* {"entry4", "90.11.42.28", "90.11.42.29", "albp=true:file=/vxWorks"}      */
    };
LOCAL int dhcpsLeaseTblSize = (NELEMENTS(dhcpsLeaseTbl));

/*
 * If the DHCP server will receive messages from relay agents,
 * the following table must be filled in by the user.
 */

DHCPS_RELAY_DESC dhcpsRelayTbl [] =
    {
    /*
     IP address of agent              Subnet Number
     --------------------             -------------
     */
    /* {"90.11.42.254",               "90.11.42.0"}, */
    };
LOCAL int dhcpsRelayTblSize = (NELEMENTS(dhcpsRelayTbl));

LOCAL int dhcpsTaskPriority  = 56;      /* Priority level of DHCP server */
LOCAL int dhcpsTaskOptions   = 0;       /* Option settings for DHCP server */
LOCAL int dhcpsTaskStackSize = 5000;    /* Stack size for DHCP server task */
IMPORT void dhcpsStart (void);
#endif

#ifdef INCLUDE_DHCPR                   /* Relay agent settings. */
#ifndef INCLUDE_DHCPS                  /* Server is superset of relay agent. */

LOCAL int dhcprTaskPriority  = 56;    /* Priority level of DHCP relay agent */
LOCAL int dhcprTaskOptions   = 0;     /* Option settings for DHCP relay agent */
LOCAL int dhcprTaskStackSize = 2500;  /* Stack size for DHCP relay agent */
IMPORT void dhcprLibInit (void);
IMPORT STATUS dhcprInit (struct ifnet **, int, DHCP_TARGET_DESC *, int);
IMPORT void dhcprStart (void);
#endif
#endif

/*******************************************************************************
*
* usrNetInit - system-dependent network initialization
*
* This routine initializes the network.  The ethernet and backplane drivers
* and the TCP/IP software are configured.  It also adds hosts (analogous to
* the /etc/hosts file in UNIX), gateways (analogous to /etc/gateways), sets
* up our access rights on the host system (with iam()), optionally
* initializes telnet, rlogin, RPC, and NFS support.
*
* The boot string parameter is normally left behind by the boot ROMs,
* at address BOOT_LINE_ADRS.
*
* Unless the STANDALONE option is selected in usrConfig.c, this routine
* will automatically be called by the root task usrRoot().
*
* RETURNS:
* OK, or
* ERROR if there is a problem in the boot string, or initializing network.
*
* SEE ALSO:  "Network" chapter of the VxWorks Programmer's Guide
*
* NOMANUAL
*/

STATUS usrNetInit 
    (
    char *bootString		/* boot parameter string */
    )
    {
    BOOT_PARAMS	params;
    char	numString [30]; 	/* buffer for numeric strings */
    char * 	pNetDev = NULL; 	/* Name of network device to setup */
    char *	pBootString;       /* IP address settings from boot string */
    int		netmask;

#if  defined (INCLUDE_SM_NET) || defined (INCLUDE_PROXY_SERVER)
    int 	proxymask; 	/* mask for backplane and/or proxy net */
#endif

    char	devName [MAX_FILENAME_LENGTH];	/* device name */

#ifdef INCLUDE_NET_REM_IO
    int		protocol;
#endif /* INCLUDE_NET_REM_IO */

#ifdef INCLUDE_DHCPC
    BOOL 	dhcpBoot = FALSE;       /* Lease obtained during startup? */
#endif  /* INCLUDE_DHCPC */

#ifdef INCLUDE_DHCPS
    STATUS dhcpsResult;
#endif

#ifdef INCLUDE_DHCPR
#ifndef INCLUDE_DHCPS              /* Server is superset of relay agent. */
    STATUS dhcprResult;
#endif
#endif

#ifdef INCLUDE_END
    int         count;
    END_TBL_ENTRY* pDevTbl;
    END_OBJ*      pCookie = NULL;
    END_OBJ*   pEnd;
#endif /* INCLUDE_END */

    BOOL	backplaneBoot = FALSE;
    BOOL 	attachFlag = FALSE;   /* Device configuration required? */

    /* interpret boot command */
    if (bootString == NULL)
	bootString = BOOT_LINE_ADRS;

    if (usrBootLineCrack (bootString, &params) != OK)
	return (ERROR);

#if defined(BROADCOM_BSP)
    if (params.other [0] != EOS)
        {
        int intArr[6];
        unsigned char macAddr[6];
        char *pStr;
        int ix;

        /* extract enet MAC address from 'other' parameter */
        if ((pStr = strstr(params.other, ";mac=")) != NULL)
            {
            *pStr = EOS;
            if (sscanf(&pStr[5], "%02x:%02x:%02x:%02x:%02x:%02x",
                       &intArr[0], &intArr[1], &intArr[2], 
                       &intArr[3], &intArr[4], &intArr[5]) == 6)
                {
                for (ix = 0; ix < 6; ix++)
                    {
                    macAddr[ix] = intArr[ix];
                    }
                sysEnetAddrSet(macAddr);
                }
            }
        }
#endif

    /* fill in any default values specified in configAll */

    if ((params.hostName [0] == EOS) &&			/* host name */
        (strcmp (HOST_NAME_DEFAULT, "") != 0))
	{
	strncpy (params.hostName, HOST_NAME_DEFAULT, BOOT_HOST_LEN);
	printf ("Host Name: %s \n", params.hostName);
	}

    if ((params.targetName [0] == EOS) &&		/* targetname */
        (strcmp (TARGET_NAME_DEFAULT, "") != 0))
	{
	strncpy (params.targetName, TARGET_NAME_DEFAULT, BOOT_HOST_LEN);
	printf ("Target Name: %s \n", params.targetName);
	}

    if ((params.usr [0] == EOS) &&			/* user name (u) */
        (strcmp (HOST_USER_DEFAULT, "") != 0))
	{
	strncpy (params.usr, HOST_USER_DEFAULT, BOOT_USR_LEN);
	printf ("User: %s \n", params.usr);
	}

    if ((params.startupScript [0] == EOS) &&		/* startup script (s) */
        (strcmp (SCRIPT_DEFAULT, "") != 0))
	{
	strncpy (params.startupScript, SCRIPT_DEFAULT, BOOT_FILE_LEN);
	printf ("StartUp Script: %s \n", params.startupScript);
	}

    if ((params.other [0] == EOS) &&			/* other (o) */
        (strcmp (OTHER_DEFAULT, "") != 0))
	{
	strncpy (params.other, OTHER_DEFAULT, BOOT_OTHER_LEN);
	printf ("Other: %s \n", params.other);
	}

    if (params.passwd [0] == EOS)			/* password */
	strncpy (params.passwd, HOST_PASSWORD_DEFAULT, BOOT_PASSWORD_LEN);

    /* fill in system-wide variables */

    bcopy ((char *) &params, (char *) &sysBootParams, sizeof (sysBootParams));

    sysFlags = params.flags;
    strcpy (sysBootHost, params.hostName);	/* backwards compatibility */
    strcpy (sysBootFile, params.bootFile);	/* backwards compatibility */

    /* set processor number: may establish vme bus access, etc. */

    if (_procNumWasSet != TRUE)
	{
    	sysProcNumSet (params.procNum);
	_procNumWasSet = TRUE;
	}

    /* start the network */

    /* initialize the generic socket library */

    if (sockLibInit (NUM_FILES) == ERROR)
	return (ERROR);

#if defined(INCLUDE_STREAMS) || defined(INCLUDE_STREAMS_ALL)
    if (usrStrmInit() == ERROR)		/* init Streams subsystem */
        return (ERROR);
#endif
 
#ifdef INCLUDE_BSD_SOCKET

#ifdef BSD43_COMPATIBLE
    bsdSock43ApiFlag = TRUE;
#endif  /* BSD43_COMPATIBLE */

    /* add the BSD socket library interface */
 
    if (sockLibAdd ((FUNCPTR) bsdSockLibInit, AF_INET_BSD, AF_INET) == ERROR)
        return (ERROR);

#if defined (INCLUDE_ROUTE_SOCK) 
    if (sockLibAdd ((FUNCPTR) bsdSockLibInit, AF_ROUTE, AF_ROUTE) == ERROR)
        return (ERROR);

    routeSockLibInit () ;
#endif 	/* INCLUDE_ROUTE_SOCK */    
#endif  /* INCLUDE_BSD_SOCKET */
 
    /* install default socket library interface */
 
#ifndef	DEFAULT_STREAMS_SOCKET
    if (sockLibAdd ((FUNCPTR) bsdSockLibInit, AF_INET, AF_INET) == ERROR)
        return (ERROR);
#endif  /* DEFAULT_STREAMS_SOCKET */

/* set max link header reserved by BSD socket layer */
    max_linkhdr = USR_MAX_LINK_HDR;

    hostTblInit ();		/* initialize host table */
    usrNetProtoInit (); 	/* initialize various protocols */
    netLibInit ();

#if defined(INCLUDE_END)
    muxMaxBinds = MUX_MAX_BINDS;
    if (muxLibInit() == ERROR)
	return (ERROR);

    /* Add our default address resolution functions. */

#ifdef INCLUDE_NT_ULIP
    muxAddrResFuncAdd (M2_ifType_ethernet_csmacd, 0x800, ntResolv);
#else
    muxAddrResFuncAdd (M2_ifType_ethernet_csmacd, 0x800, ipEtherResolvRtn);
#endif

    /* Add in mux ENDs. */
    for (count = 0, pDevTbl = endDevTbl; pDevTbl->endLoadFunc != END_TBL_END;
         pDevTbl++, count++)
        {
        /* Make sure that WDB has not already installed the device. */
        if (!pDevTbl->processed)
            {
            pCookie = muxDevLoad(pDevTbl->unit,
                                 pDevTbl->endLoadFunc,
                                 pDevTbl->endLoadString,
                                 pDevTbl->endLoan, pDevTbl->pBSP);
            if (pCookie == NULL)
                {
                printf("muxDevLoad failed for device entry %d!\n", count);
                }
            else
                {
                pDevTbl->processed = TRUE;
                if (muxDevStart(pCookie) == ERROR)
                    {
                    printf("muxDevStart failed for device entry %d!\n", count);
                    }
                }
            }
        }
#endif /* INCLUDE_END */

#ifdef	INCLUDE_PPP
#ifdef	INCLUDE_PPP_CRYPT
    cryptRtnInit (&pppCryptRtn);	/* install crypt() routine */
#endif	/* INCLUDE_PPP_CRYPT */

    loginInit();			/* for possible login option */
    pppSecretLibInit();			/* initialize secrets library */
    pppHookLibInit ();			/* initialize pppHooks library */
    pppShowInit();
#endif	/* INCLUDE_PPP */

#ifdef INCLUDE_NET_REM_IO
    remLastResvPort = 1010;	/* pick an unused port number so we don't *
				 * have to wait for the one used by the *
				 * by the bootroms to time out */
    _func_ftpLs = ftpLs;	       /* init ptr to ftp dir listing routine */
    _func_netLsByName = netLsByName;   /* init ptr to netDrv listing routine */
    _func_remCurIdGet = (FUNCPTR) remCurIdGet;
    _func_remCurIdSet = (FUNCPTR) remCurIdSet;
#endif /* INCLUDE_NET_REM_IO */

#ifdef INCLUDE_MIB2_IF
    /* add hooks for if.c */
    _m2SetIfLastChange = (FUNCPTR) m2SetIfLastChange;
    _m2IfTableUpdate   = (FUNCPTR) m2IfTableUpdate;
#endif /* INCLUDE_MIB2_IF */

#ifdef INCLUDE_DHCPR
#ifndef INCLUDE_DHCPS
    dhcprLibInit ();
#endif
#endif

    if ((strncmp (params.bootDev, "bp", 2) == 0) ||
            (strncmp (params.bootDev, "sm", 2) == 0))
	{
	/* booting via backplane */

        pBootString = params.bad;
        }
    else
        pBootString = params.ead;

#ifdef INCLUDE_DHCPC
    /* 
     * Remove any values for DHCP lease origin and lease duration from
     * address string and store in dhcpcBootLease global. Set dhcpBoot
     * flag to indicate if DHCP lease values were found.
     */

    if (sysFlags & SYSFLG_AUTOCONFIG)
        if (dhcpcLeaseGet (pBootString, &dhcpBoot) == ERROR)
            {
            printf ("Unable to verify assigned DHCP address.\n");
            return (ERROR);
            }
#else
    /*
     * If network configuration protocols are used, check address 
     * for DHCP lease information. If found (and removed), exit
     * with an error, since the active DHCP lease cannot be maintained.
     */ 

    if (sysFlags & SYSFLG_AUTOCONFIG)
        if (dhcpcBootLineClean (pBootString) == OK)
            {
            printf ("Can't use dynamic address %s without DHCP.\n",
                        pBootString);
            return (ERROR);
            }
#endif    /* INCLUDE_DHCPC */

    /* attach and configure interfaces */
#ifdef INCLUDE_ULIP
    if (strncmp (params.bootDev, "ul", 2) == 0)
	{
        /*
         * Booting with the simulator's interface completely configures the
         * device. No DHCP/BOOTP address assignment is available.
         */

	extern int ulipInit ();

        pNetDev = params.bootDev;

        printf ("Attaching network interface %s%d... ",
                 params.bootDev, params.unitNum);

	bootNetmaskExtract (params.ead, &netmask); /* remove and ignore mask */

	/* XXX last octet of 'ead' == procNum */

	if (ulipInit (params.unitNum, params.ead, params.had, params.procNum) 
                 == ERROR)
	    {
	    if (errno == S_if_ul_NO_UNIX_DEVICE)
		printf ("\nulipInit failed, errno = S_if_ul_NO_UNIX_DEV\n");
	    else
		printf ("\nulipInit failed, errno = 0x%x\n", errno);

	    return (ERROR);
	    }

	printf ("done.\n");
	}
    else
#endif  /* INCLUDE_ULIP */

    if (strncmp (params.bootDev, "ppp", 3) == 0)
        {
        /*
         * Booting via ppp completely configures the device.
         * No automatic DHCP/BOOTP address assignment (not broadcast capable).
         */

        pNetDev = params.bootDev;

        if (usrPPPInit (params.bootDev, params.unitNum, params.ead, 
                        ((params.gad[0] == EOS)? params.had : params.gad))
                == ERROR)
            return (ERROR);
        }
    else if (strncmp (params.bootDev, "sl", 2) == 0)
	{
        /*
         * Booting via slip completely configures the device.
         * No automatic DHCP/BOOTP address assignment (not broadcast capable).
         */

        pNetDev = params.bootDev;

	if (usrSlipInit (params.bootDev, params.unitNum, params.ead, 
                         ((params.gad[0] == EOS)? params.had : params.gad)) 
                == ERROR)
	    return (ERROR);
	}
    else if ((strncmp (params.bootDev, "bp", 2) == 0) ||
            (strncmp (params.bootDev, "sm", 2) == 0))
	{
        pNetDev = params.bootDev;
	backplaneBoot = TRUE;

	if (usrBpInit (params.bootDev, params.unitNum, 0) == ERROR)
	    return (ERROR);

        /*
         * Booting via backplane: protocol attachment is still required
         * and automatic configuration with DHCP or BOOTP is supported.
         */

        attachFlag = TRUE;
	}
    else
	{
        if ((strncmp (params.bootDev, "scsi", 4) == 0) || 
            (strncmp (params.bootDev, "ide", 3) == 0) ||
	    (strncmp (params.bootDev, "ata", 3) == 0) ||
	    (strncmp (params.bootDev, "fd", 2) == 0)  ||
#if defined(BROADCOM_BSP)
            (strncmp (params.bootDev, "fl", 2) == 0) ||
#endif
	    (strncmp (params.bootDev, "tffs", 4) == 0))
	    {
	    /* booting from disk, configure network if requested */

	    if (params.other [0] != EOS)
		{
                /*
                 * Protocol attachment is required and automatic
                 * configuration with DHCP or BOOTP is supported
                 * for all these network devices.
                 */

                attachFlag = TRUE;
                pNetDev = params.other;
#if defined(BROADCOM_BSP)
                switch(params.other[2]) {
                    case '0' : params.unitNum = 0; params.other[2] = EOS; break;
                    case '1' : params.unitNum = 1; params.other[2] = EOS; break;
                    default: break;
                }
#endif
                }
	    }
	else
	    {
            pNetDev = params.bootDev;

            /*
             * Booting via network: protocol attachment is required and
             * automatic configuration with DHCP or BOOTP is supported
             * for all these network devices.
             */

            attachFlag = TRUE;
            }
        }

    if (attachFlag)
        {
        /*
         * Attempt to attach and setup the network boot device
         * (which is capable of automatic configuration).
         */

        BOOL attached = FALSE;

        netmask = 0;
        bootNetmaskExtract (pBootString, &netmask);

#ifdef INCLUDE_END
        /* Check for the END driver, if any. */

        pEnd = endFindByName (pNetDev, params.unitNum);
        if (pEnd != NULL)
            {
            if (ipAttach(params.unitNum, pNetDev) != OK)
                {
                logMsg ("Failed to attach to device %s",
                        (int)pNetDev, 0, 0, 0, 0, 0);
                return (ERROR);
                }
            attached = TRUE;
            }
#if defined(BROADCOM_BSP)
        if (sysIsLM()) {
        /* Check for the END driver, for the other unit */

        pEnd = endFindByName (pNetDev, (params.unitNum) ? 0 : 1);
        if (pEnd != NULL)
            {
            if (ipAttach((params.unitNum) ? 0 :  1, pNetDev) != OK)
                {
                logMsg ("Failed to attach to device %s",
                        (int)pNetDev, 0, 0, 0, 0, 0);
                return (ERROR);
                }
            attached = TRUE;
            }
        }
#endif
#endif /*INCLUDE_END*/

#ifdef INCLUDE_BSD
        if (!attached)
            {
            if ( (usrNetIfAttach (pNetDev, params.unitNum, pBootString) !=OK))
                return (ERROR);
            attached = TRUE;
            }
#endif /*INCLUDE_BSD*/

        if (!attached)
            return (ERROR);

        printf ("Attached TCP/IP interface to %s unit %d\n", pNetDev,
                params.unitNum);
        }

    /* add loop-back interface */

    usrNetIfAttach ("lo", 0, "127.0.0.1");
    usrNetIfConfig ("lo", 0, "127.0.0.1", "localhost", 0);

    /* Attempt to retrieve a target IP address if it is not available. */

#ifdef INCLUDE_DHCPC
    if (dhcpcLibInit (DHCPC_SPORT, DHCPC_CPORT, DHCPC_MAX_LEASES, 
                      DHCPC_MAX_MSGSIZE, DHCPC_OFFER_TIMEOUT, 
		      DHCPC_DEFAULT_LEASE, DHCPC_MIN_LEASE) == ERROR)
        return (ERROR);

#ifdef INCLUDE_NET_SHOW
    dhcpcShowInit ();
#endif

    /*
     * Renew or obtain a lease as needed and store values in boot line.
     * If successful, the pDhcpcBootCookie global is set to allow user
     * access to the lease and the timing information is stored in the
     * dhcpcBootLease global for later use.
     */

    if (dhcpcConfigSet (&params, &netmask, &dhcpBoot, attachFlag) == ERROR)
        return (ERROR);

#endif   /* INCLUDE_DHCPC */

#if defined(BROADCOM_BSP)

    /* Configure the network device using the address/mask information. */
	if ((sysIsLM()) && (strcmp(params.bootDev, "flash") == 0)) {
    char ipaddr[] = {'1', '9', '2', '.', '1', '6', '8', '.','0', '.', '0', ' '};
        ipaddr[10] += sysSlotIdGet() / ((sysSlotIdGet() > 9) ? 10: 1);
        ipaddr[11] = (sysSlotIdGet() > 9) ? '0' + (sysSlotIdGet()) % 10 : ' ';
        if (attachFlag && (usrNetIfConfig (pNetDev, 0, ipaddr,
                        params.targetName, 0xffffff00) != OK))
            return (ERROR);
        ipaddr[8] = '1';
        if (attachFlag && (usrNetIfConfig (pNetDev, 1, ipaddr,
                        params.targetName, 0xffffff00) != OK))
            return (ERROR);
    } else {
        if (attachFlag && (usrNetIfConfig (pNetDev, params.unitNum, pBootString,
                            params.targetName, netmask) != OK))
            return (ERROR);
    }
#else
    if (attachFlag && (usrNetIfConfig (pNetDev, params.unitNum, pBootString,
                        params.targetName, netmask) != OK))
        return (ERROR);
#endif

    /* attach backplane interface (as second interface) */

#ifdef INCLUDE_SM_NET 	
#define BP_ADDR_GIVEN	        (params.bad [0] != EOS)
#define BP_ADDR_NOT_GIVEN	(params.bad [0] == EOS)

    if ( !backplaneBoot )
	{
    	char 		*bpDev;
    	BOOL	        proxyOn = FALSE;       /* Initialize Defaults */
	u_long          startAddr = 0;         /* Default sequential Addr off */
	char            netDev [BOOT_DEV_LEN + 1];
	BOOL            seqAddrOn = FALSE;
	BOOL            configureBp = TRUE;
	BOOL            useEtherAddr = FALSE;
                                             /* Turn switches ON as needed */
#ifdef INCLUDE_SM_SEQ_ADDR
	seqAddrOn = TRUE;
#endif

#ifdef INCLUDE_PROXY_DEFAULT_ADDR
	useEtherAddr = TRUE;
#endif

#ifdef INCLUDE_PROXY_SERVER
	proxyOn = TRUE;
#endif

        bpDev = "sm";
        sprintf(netDev, "%s%d", "sm", params.unitNum);
	bootNetmaskExtract (params.bad, &proxymask);
	if (proxyOn == TRUE)
	    {
	    if (seqAddrOn == TRUE)   /* PROXY WITH SEQ ADDR */
	        {
	        /* Pick address from backplane or ethernet */

		if (BP_ADDR_GIVEN)
	   	    {
	    	     startAddr = ntohl (inet_addr (params.bad));
	    	     netmask = proxymask;
	   	    }
		else if (sysProcNumGet () == 0)
		    {
                    /*
                     * The shared memory master calculates the next available
                     * address, if possible. Slaves will read it directly.
                     */

		    if ( useEtherAddr )
			{
	    	    	startAddr = ntohl (inet_addr (params.ead)) + 1;
                        netmask = 0xffffffff;
			}
		    else		/* Configuration error */
		       {
		       printf ("Error: No address for proxy service.\n");
		       printf ("Backplane IP Address must be specified.\n");
                       configureBp = FALSE;
		       }
		    }
		}
	    else                     /* PROXY WITHOUT SEQ ADDR */
                {
		if (BP_ADDR_NOT_GIVEN)
		    {
                    if (sysProcNumGet () == 0)
                        configureBp = FALSE;    /* Can't start master. */
		    }
		else
		    {   /* startAddr is left as zero */
	    	    netmask = proxymask;
		    }
	        }
	    }
	else
	    {		/* Using Subnet without PROXY Arp */
	    if (BP_ADDR_GIVEN)
	        {
	        if (seqAddrOn == TRUE)
	            {    /* Assign sequential address to backplane */
	    	    startAddr = ntohl (inet_addr (params.bad));
		    }
	    	netmask = proxymask;
		}
            else if (sysProcNumGet () == 0)
                {
                /* Don't start the master if the backplane address is empty. */

                configureBp = FALSE;
                }
	    }

	if (configureBp == TRUE)
	    {
	    if (usrBpInit (bpDev, params.unitNum, startAddr) == ERROR)
	    	return (ERROR);

	    (void) usrNetIfAttach (bpDev, params.unitNum, params.bad);

	    /* Assigned Back Plane Address if needed */
	    if ((BP_ADDR_NOT_GIVEN) &&
		 (smNetInetGet (netDev, params.bad, NONE) == OK))
		 printf ("Backplane address: %s\n", params.bad);

	    (void) usrNetIfConfig (bpDev, params.unitNum, params.bad, 
                                   (char *) NULL, netmask);
	    }
	}

#ifdef INCLUDE_DHCPC
    if (backplaneBoot)
        {
        if ((sysFlags & SYSFLG_AUTOCONFIG) && !(sysFlags & SYSFLG_PROXY))
            {
            printf ("Warning! DHCP over backplane may need proxy arp.\n");
            }
        }
#endif /* INCLUDE_DHCPC */

#ifdef INCLUDE_SHOW_ROUTINES
    smNetShowInit ();
#endif /* INCLUDE_SHOW_ROUTINES */

#endif /* INCLUDE_SM_NET */

    if (sysFlags & SYSFLG_PROXY)
        {
#ifdef INCLUDE_PROXY_CLIENT
        printf ("registering proxy client %s...", pBootString);

        devName [0] = EOS;
        sprintf(devName, "%s%d", params.bootDev, params.unitNum);

        if (proxyReg (devName, pBootString) == ERROR)
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

#ifdef INCLUDE_PROXY_SERVER

#ifndef INCLUDE_SM_NET
    /* Remove netmask from address if not already done. */

    bootNetmaskExtract (params.bad, &proxymask);
#endif

    if ((sysProcNumGet () == 0) && (params.bad [0] != EOS) &&
        (params.ead [0] != EOS))
        {
        hashLibInit ();                 /* make sure hash functions init'd */

        proxyArpLibInit (8, 8);
        printf ("Creating proxy network: %s\n", params.bad);
        if (proxyNetCreate (params.bad, params.ead) == ERROR)
            {
            printf ("proxyNetCreate failed:%x\n", errno);
            return (ERROR);
            }
        }
#endif /* INCLUDE_PROXY_SERVER */

    if (params.targetName[0] != EOS)
        sethostname (params.targetName, strlen (params.targetName));

#ifdef INCLUDE_DHCPC
    /*
     * If the target address is not manually assigned, add timing 
     * information so later reboots will detect the DHCP lease.
     */

    if (dhcpBoot == TRUE)
        {
        if (netmask == 0)
            sprintf (numString, "::%lx:%lx", dhcpcBootLease.lease_duration,
                                             dhcpcBootLease.lease_origin);
        else
            sprintf (numString, ":%x:%lx:%lx", ntohl (netmask),
                                               dhcpcBootLease.lease_duration,
                                               dhcpcBootLease.lease_origin);
        if (backplaneBoot)
            strcat (params.bad, numString);
        else
            strcat (params.ead, numString);

        bootStructToString (BOOT_LINE_ADRS, &params);
        }
#endif   /* INCLUDE_DHCPC */

    /* if a gateway was specified, extract the network part of the host's
     * address and add a route to this network
     */

    if (params.gad[0] != EOS)
        {
	inet_netof_string (params.had, numString);
#ifdef BROADCOM_BSP
        /* Make the gateway the default route */
        routeAdd ("0.0.0.0", params.gad);
#else
	routeAdd (numString, params.gad);
#endif
        }

    /* associate host name with the specified host address */

    hostAdd (params.hostName, params.had);

#ifdef	INCLUDE_ULIP
    /* Set up host names and routes for talking to other vxworks's */
    {
    extern char *vxsim_ip_addr;
    extern char *vxsim_ip_name;
    int vxsim_num_ulips = 16;
    int ix;
    char host [50];
    char hostip [50];

    /* declare other simulated vxWorks' */

    for (ix = 0; ix < vxsim_num_ulips; ix++)
	{
	sprintf (host, vxsim_ip_name, ix);
	sprintf (hostip, vxsim_ip_addr, ix);
	hostAdd (host, hostip);
	}

    /* Add default route thru host */
    routeAdd ("0.0.0.0", params.had);
    routeAdd (params.ead, "localhost");	/* loopback for local addresses */
    }
#endif /* INCLUDE_ULIP */

#ifdef	INCLUDE_PASSFS

    /* The device names for passFs and netDrv are the same, e.g. "host:",
     * therefore, we have and either/or for the two devices.
     * Bulk of work done in usrConfig.c:usrRoot() for passFs installation.
     */
    iam (params.usr, params.passwd);
    devName[0] = EOS;
    protocol = (params.passwd[0] == EOS) ? 0 : 1;	/* pick protocol */

#else
    /* create transparent remote file access device;
     * device name is host name with ':' appended.
     * protocol is rcmd if no password, or ftp if password specified
     */
#ifdef INCLUDE_NET_REM_IO
    netDrv ();					/* init remote file driver */
    sprintf (devName, "%s:", params.hostName);		/* make dev name */
    protocol = (params.passwd[0] == EOS) ? 0 : 1;	/* pick protocol */
    netDevCreate (devName, params.hostName, protocol);	/* create device */

    /* set the user id, and current directory */

    iam (params.usr, params.passwd);
    ioDefPathSet (devName);

    taskDelay (sysClkRateGet () / 4);		/* 1/4 of a second */
#endif /* INCLUDE_NET_REM_IO */
#endif /* INCLUDE_PASSFS */

#ifdef  INCLUDE_ZBUF_SOCK
    zbufSockLibInit ();                 /* initialize zbuf socket interface */
#endif  /* INCLUDE_ZBUF_SOCK */

#ifdef  INCLUDE_TCP_DEBUG
    tcpTraceInit ();                    /* initialize TCP debug facility */
#endif  /* INCLUDE_TCP_DEBUG */

    /* start the rlogin and telnet daemon */

#ifdef	INCLUDE_RLOGIN
    rlogInit ();
#endif	/* INCLUDE_RLOGIN */

#ifdef	INCLUDE_TELNET

#ifndef TELNETD_PORT
#define TELNETD_PORT 23
#endif

#ifndef TELNETD_MAX_CLIENTS
#define TELNETD_MAX_CLIENTS 1
#endif

#ifndef TELNETD_TASKFLAG
#define TELNETD_TASKFLAG FALSE
#endif

    if (telnetdInit (TELNETD_MAX_CLIENTS, TELNETD_TASKFLAG) == ERROR)
        printf ("Error %x: unable to initialize telnet server.\n", errno);

#ifdef INCLUDE_SHELL
     /* Configure telnet server to use VxWorks shell as command interpreter. */

     else if (telnetdParserSet (shellParserControl) == ERROR)
        printf ("Error %x: unable to attach shell to telnet server.\n", errno);
     else if (telnetdStart (TELNETD_PORT) == ERROR)
        printf ("Error %x: unable to start telnet server.\n", errno);
        
#endif /* INCLUDE_SHELL */        
 
#endif	/* INCLUDE_TELNET */

    /* initialize rpc daemon if specified */

#ifdef	INCLUDE_RPC
    rpcInit ();
#endif	/* INCLUDE_RPC */

#if defined(INCLUDE_FTP_SERVER)
#if defined(INCLUDE_FTPD_SECURITY)
    loginInit();
    ftpdInit((FUNCPTR) loginUserVerify, 0);
#else
    ftpdInit ((FUNCPTR) NULL,0);
#endif
#endif

#ifdef INCLUDE_TFTP_SERVER
    tftpdInit (0, 0, 0, FALSE, 0);
#endif

    /* initialize NFS server and client if specified */

#ifdef INCLUDE_NFS_SERVER
	if (nfsdInit (0, 0, 0, 0, 0, 0) == ERROR)
	    return (ERROR);
#endif  /* INCLUDE_NFS_SERVER */
	
#ifdef	INCLUDE_NFS
    /*
     * The following values are the default values used in NFS.
     * They can be reset here if necessary.
     *
     *     nfsMaxMsgLen   = 8192	message size (decrease only)
     *     nfsTimeoutSec  = 5		timeout seconds
     *     nfsTimeoutUSec = 0		timeout microseconds
     */


#if (CPU==SIMHPPA)
    nfsMaxMsgLen   = 512;
#endif
    nfsAuthUnixSet (params.hostName, NFS_USER_ID, NFS_GROUP_ID, 0, (int *) 0);

    if (nfsDrv () == ERROR)	/* initialize nfs driver */
	printf ("Error initializing NFS, errno = %#x\n", errno);
    else
	{
#ifdef	INCLUDE_NFS_MOUNT_ALL
	printf ("Mounting NFS file systems from host %s", params.hostName);
	if (params.targetName[0] != EOS)
	    printf (" for target %s:\n", params.targetName);
	else
	    printf (":\n");

	nfsMountAll (params.hostName, params.targetName, FALSE);
	printf ("...done\n");
#endif	/* INCLUDE_NFS_MOUNT_ALL */

	/* if the boot pathname starts with a device name, e.g. an nfs mount,
	 * then set the current directory to that device
	 */

	(void) iosDevFind (sysBootFile, &pBootString);
	if (pBootString != sysBootFile)
	    {
	    devName[0] = EOS;
	    strncat (devName, sysBootFile, pBootString - sysBootFile);
	    ioDefPathSet (devName);
	    }
	}
#else	/* INCLUDE_NFS */
    printf ("NFS client support not included.\n");
    pBootString = NULL; 	/* dummy use to quiet compiler warning*/
#endif	/* INCLUDE_NFS */

#if defined (INCLUDE_DHCPS) || defined (INCLUDE_DHCPR)
    /*
     * For now, set server or relay agent to listen on boot device.
     * Eventually, need to allow for multiple network devices.
     */

    devName [0] = EOS;
    sprintf(devName, "%s%d", params.bootDev, params.unitNum);
    devlist[0] = ifunit (devName);

#ifdef INCLUDE_DHCPS
#ifdef INCLUDE_DHCPR
    printf ("DHCP Server present. Relay agent not permitted.\n");
#endif

    if (devlist[0] == NULL)
        printf("Network interface %s not found. DHCP server not started.\n",
                devName);
    else
        {
        dhcpsResult = dhcpsLeaseHookAdd (DHCPS_LEASE_HOOK);
        if (dhcpsResult == ERROR)
            {
            printf ("Warning: Required storage hook not installed.\n");
            printf ("Lease records will not be saved.\n");
            }
        dhcpsResult = dhcpsAddressHookAdd (DHCPS_ADDRESS_HOOK);
        if (dhcpsResult == ERROR)
            {
            printf ("Warning: No DHCP server address cache!");
            printf (" Later entries will not be saved.\n");
            }
        dhcps_dflt_lease = DHCPS_DEFAULT_LEASE;
        dhcps_max_lease = DHCPS_MAX_LEASE;

        dhcpsResult = dhcpsInit (devlist, 1, DHCPS_MAX_MSGSIZE,
                                 dhcpsLeaseTbl, dhcpsLeaseTblSize,
                                 dhcpsRelayTbl, dhcpsRelayTblSize,
                                 dhcpTargetTbl, dhcpTargetTblSize);
        if (dhcpsResult == ERROR)
            printf ("Error initializing DHCP server. Not started.\n");
        else
            {
            dhcpsResult = taskSpawn ("tDhcpsTask", dhcpsTaskPriority,
                                     dhcpsTaskOptions, dhcpsTaskStackSize,
                                     (FUNCPTR) dhcpsStart,
                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
            if (dhcpsResult == ERROR)
                printf ("Unable to start DHCP server task.\n");
            printf ("DHCP server started.\n");
            }
        }
#else
    if (devlist[0] == NULL)
      printf("Network interface %s not found. DHCP relay agent not started.\n",
                devName);
    else
        {
        dhcprResult = dhcprInit (devlist, 1, dhcpTargetTbl, dhcpTargetTblSize);
        if (dhcprResult == ERROR)
            printf ("Error initializing DHCP relay agent. Not started.\n");
        else
            {
            dhcprResult = taskSpawn ("tDhcprTask", dhcprTaskPriority,
                                      dhcprTaskOptions, dhcprTaskStackSize,
                                      (FUNCPTR) dhcprStart,
                                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
            if (dhcprResult == ERROR)
                printf ("Unable to start DHCP relay agent task.\n");
            printf ("DHCP relay agent started.\n");
            }
        }
#endif
#endif    /* INCLUDE_DHCPS or INCLUDE_DHCPR */

#ifdef INCLUDE_SNTPC
   sntpcInit (SNTP_PORT);
#endif

#ifdef INCLUDE_SNTPS
    /*
     * For now, set server to listen on boot device.
     * Eventually, need to allow for multiple network devices.
     */

    devName [0] = EOS;
    sprintf(devName, "%s%d", params.bootDev, params.unitNum);
    sntpsInit (devName, SNTPS_MODE, 
               SNTPS_DSTADDR, SNTPS_INTERVAL, SNTP_PORT, SNTPS_TIME_HOOK);
#endif

#ifdef  INCLUDE_PING
    (void) pingLibInit ();              /* initialize the ping utility */
#endif  /* INCLUDE_PING */

#ifdef INCLUDE_SNMPD
    usrSnmpdInit (); 
#endif

#ifdef INCLUDE_RIP

#ifndef RIP_SUPPLIER
#define RIP_SUPPLIER 0
#endif /* RIP_SUPPLIER */

#ifndef RIP_GATEWAY
#define RIP_GATEWAY 0
#endif /* RIP_GATEWAY */

#ifndef RIP_VERSION
#define RIP_VERSION 1
#endif /* RIP_VERSION */

#ifndef RIP_MULTICAST
#define RIP_MULTICAST 0
#endif /* RIP_MULTICAST */

#ifndef RIP_TIMER_RATE
#define RIP_TIMER_RATE 1
#endif /* RIP_TIMER_RATE */

#ifndef RIP_SUPPLY_INTERVAL
#define RIP_SUPPLY_INTERVAL 30
#endif /* RIP_SUPPLY_INTERVAL */

#ifndef RIP_EXPIRE_TIME
#define RIP_EXPIRE_TIME 180
#endif /* RIP_EXPIRE_TIME */

#ifndef RIP_GARBAGE_TIME
#define RIP_GARBAGE_TIME 300
#endif /* RIP_GARBAGE TIME */    

    ripLibInit(RIP_SUPPLIER, RIP_GATEWAY, RIP_MULTICAST, RIP_VERSION,
               RIP_TIMER_RATE, RIP_SUPPLY_INTERVAL, RIP_EXPIRE_TIME,
               RIP_GARBAGE_TIME);
#endif /* INCLUDE_RIP2 */

#ifdef INCLUDE_DNS_RESOLVER
    if (usrResolvInit () == ERROR)
	{
	printf ("Error initializing resolvLib\n");
	return(ERROR);
	}
#endif /* INCLUDE_DNS_RESOLVER */

#ifdef INCLUDE_ARP
    arpLibInit ();
#endif

    return (OK);
    }

/* the next routines are in common with bootConfig.c and will be merged */

/******************************************************************************
*
* usrBootLineInit - initialize system boot line
*
* Initializes system boot line as per specified start type.  If the boot
* line is empty or this is a COLD boot, i.e. with CLEAR option to clear
* memory, then the boot line is initialized from non-volatile ram, if any,
* otherwise from the compiled in default boot line.
*
* NOMANUAL
*/

void usrBootLineInit 
    (
    int startType
    )
    {
    if ((startType & BOOT_CLEAR) || (* BOOT_LINE_ADRS == EOS))
	{
	/* either cold boot or empty boot line -- initialize boot line */

	if ((sysNvRamGet (BOOT_LINE_ADRS, BOOT_LINE_SIZE, 0) == ERROR))
	    {
	    /* no non-volatile RAM -- use default boot line */

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
* NOMANUAL
*/

STATUS usrBootLineCrack 
    (
    char * bootString,
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

    /* check inet addresses */

    if ((checkInetAddrField (pParams->ead, TRUE) != OK) ||
	(checkInetAddrField (pParams->bad, TRUE) != OK) ||
	(checkInetAddrField (pParams->had, FALSE) != OK) ||
	(checkInetAddrField (pParams->gad, FALSE) != OK))
	{
	return (ERROR);
	}

    return (OK);
    }

/******************************************************************************
*
* checkInetAddrField - check for valid inet address in boot field
*
* RETURNS: OK, or ERROR if invalid inet address
*
* NOMANUAL
*/

STATUS checkInetAddrField 
    (
    char *pInetAddr,
    BOOL subnetMaskOK
    )
    {
    char inetAddr [30];
    int netmask;

    bzero (inetAddr, 30);   /* SPR 6326 - prevents error if address invalid. */

    if (*pInetAddr == EOS)
	return (OK);

    strncpy (inetAddr, pInetAddr, sizeof (inetAddr));

    if (subnetMaskOK)
	{
	if (bootNetmaskExtract (inetAddr, &netmask) < 0)
	    {
	    printf ("Error: invalid netmask in boot field \"%s\".\n", inetAddr);
	    return (ERROR);
	    }
	}

    if (inet_addr (inetAddr) == (u_long) ERROR)
	{
	printf ("Error: invalid inet address in boot field \"%s\".\n",inetAddr);
	return (ERROR);
	}

    return (OK);
    }

/******************************************************************************
*
* usrNetIfAttach - attach a  network interface
*
* This routine attaches the specified network interface.
*
*	- interface is attached
*	- interface name is constructed as "<devName>0"
*
* RETURNS: OK or ERROR
*
* NOMANUAL
*/

STATUS usrNetIfAttach 
    (
    char * 	devName,
    int 	unitNum,
    char * 	inetAdrs
    )
    {
    FAST NETIF *	pNif;
    STATUS 		status;
    char 		buf [BOOT_DEV_LEN + 1]; /* device name + unit number */

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

    /* attach interface */

    /* find interface in table */

    sprintf(buf, "%s%d", devName, unitNum);

    for (pNif = usrNetIfTbl; pNif->ifName != 0; pNif++)
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
        for (pNif = usrNetIfTbl; pNif->ifName != 0; pNif++)
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

    printf ("Attaching network interface %s... ", buf);

#ifdef  INCLUDE_PCMCIA
    if (strncmp (devName, "pcmcia", 6) == 0)
	pNif->arg1 = (char *)sock;
#endif  /* INCLUDE_PCMCIA */

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
			      pNif->arg4, pNif->arg5, pNif->arg6,
			      pNif->arg7, pNif->arg8);
    if (status != OK)
	{
        if (errno == S_iosLib_CONTROLLER_NOT_PRESENT)
            printf ("failed.\nError: S_iosLib_CONTROLLER_NOT_PRESENT.\n");
        else
	    printf ("failed: errno = %#x.\n", errno);
	return (ERROR);
	}

    printf ("done.\n");

    return (OK);
    }

/******************************************************************************
*
* usrNetIfConfig - configure network interface
*
* This routine configures the specified network interface with the specified
* boot parameters:
*
*	- subnetmask is extracted from inetAdrs and, if present,
*	  set for interface
*	- inet address is set for interface
*	- if present, inet name for interface is added to host table.
*
* RETURNS: OK or ERROR
*
* NOMANUAL
*/

STATUS usrNetIfConfig 
    (
    char *      devName, 		/* device name */
    int         unitNum, 		/* unit number */
    char *      inetAdrs, 		/* inet address */
    char *      inetName, 		/* host name */
    int         netmask 		/* subnet mask */
    )
    {
    char ifname [20];


#ifdef  INCLUDE_PCMCIA
    if (strncmp (devName, "pcmcia", 6) == 0)
	devName = "pcmcia";
#endif  /* INCLUDE_PCMCIA */

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
* usrBpInit - initailize backplane driver
*
* usrBpInit initializes the backplane driver shared memory region
* and sets up the backplane parameters to attach.
*
* RETURNS: OK if successful otherwise ERROR
*
* NOMANUAL
*/

STATUS usrBpInit 
    (
    char * 	devName, 	/* device name */
    int 	unitNum, 	/* unit number */
    u_long 	startAddr 	/* inet address */
    )
    {
#ifdef	INCLUDE_SM_NET
    char *		bpAnchor;	/* anchor address */
    FAST NETIF *	pNif;		/* netif struct */
    STATUS 		status;		/* status */
    int			procNum;	/* proc num */
    char 		buf [BOOT_DEV_LEN];  /* network device + unit number */

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
			    (int) SM_MEM_SIZE, SM_TAS_TYPE, SM_CPUS_MAX, 
			    SM_PKTS_SIZE, startAddr);

	if (status == ERROR)
  	    {
	    printf ("Error: backplane device %s not initialized\n", devName);
	    return (ERROR);
	    }

	printf ("done.\n");
	}


    /* Locate NETIF structure for backplane */
    sprintf (buf, "%s%d", devName, unitNum);

    for (pNif = usrNetIfTbl; pNif->ifName != 0; pNif++)
   	{
	if (strcmp (devName, pNif->ifName) == 0)
	    break;
	}

    /*
     * For backward compatibility, the device name only is acceptable for
     * unit numbers of 0.
     */
    if (pNif->ifName == 0 && unitNum == 0)
        {
        for (pNif = usrNetIfTbl; pNif->ifName != 0; pNif++)
            {
            if (strcmp (devName, pNif->ifName) == 0)
                break;
            }
        }

    if (pNif->ifName == 0)
	{
        if (strncmp (devName, "bp", 2) == 0)
	    printf ("INCLUDE_BP_5_0 not supported - use 'sm' interface\n");
	return (ERROR);
	}

    printf ("Backplane anchor at %#x... ", (int) bpAnchor);

    /* configure backplane parameters (most set in NETIF struct) */

    pNif->arg1 = bpAnchor;	/* anchor address */
    pNif->arg3 = SM_INT_TYPE;
    pNif->arg4 = SM_INT_ARG1;
    pNif->arg5 = SM_INT_ARG2;
    pNif->arg6 = SM_INT_ARG3;

    return (OK);

#else	/* INCLUDE_SM_NET */
    printf ("\nError: backplane driver referenced but not included.\n");
    return (ERROR);
#endif	/* INCLUDE_SM_NET */
    }

#ifndef INCLUDE_DHCPC
/*******************************************************************************
*
* dhcpcBootLineClean - remove DHCP information from system bootline
*
* This routine removes the DHCP information (lease duration and origin) from 
* the client address string in the bootline. Those values are present
* if the target IP address obtained during system boot was provided by a
* DHCP server. The routine is called if DHCP is not included in a VxWorks 
* image and an automatic configuration protocol was used by the bootstrap
* loader. Under those circumstances, indications of a DHCP lease result are
* treated as an error by the startup code, since the lease cannot be renewed.
*
* RETURNS: OK if DHCP lease data found in bootline, or ERROR otherwise.
*
* NOMANUAL
*/

LOCAL STATUS dhcpcBootLineClean
    (
    char * 	pAddrString    /* client address string from bootline */
    )
    {
    FAST char *pDelim;
    FAST char *offset;
    STATUS result = ERROR;

    /* Check if lease duration field is present. */

    offset = index (pAddrString, ':');     /* Find netmask field. */
    if (offset != NULL)
        {
        pDelim = offset + 1;
        offset = index (pDelim, ':');
        if (offset != NULL)
            {
            /* 
             * Lease duration found - for active DHCP leases,
             * the lease origin field is also present.
             */

            pDelim = offset + 1;
            pDelim = index (pDelim, ':');
            if (pDelim != NULL)
                 result = OK;     /* Active DHCP lease found. */

            *offset = EOS;    /* Remove DHCP lease information. */
            }
        }
    return (result);
    }
#endif

/*******************************************************************************
*
* usrSlipInit - initialize the slip device
*
* RETURNS: OK if successful, otherwise ERROR.
*
* NOMANUAL
*/

STATUS usrSlipInit 
    (
    char * 	pBootDev,		/* boot device */
    int 	unitNum, 		/* unit number */
    char * 	localAddr,		/* local address */
    char * 	peerAddr		/* peer address */
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

#if (CPU==SIMHPPA)

    sysSlipSetup ();

    /* Add a default route through the host end of the SLIP connection */

    routeAdd ("0.0.0.0", peerAddr);

#endif  /* (CPU==SIMHPPA) */

    printf ("done.\n");
    return (OK);

#else /* INCLUDE_SLIP */
    printf ("\nError: slip not included.\n");
    return (ERROR);
#endif	/* INCLUDE_SLIP */
    }

/********************************************************************************
* usrPPPInit - initialize a ppp channel
*
* RETURNS: OK if successful, otherwise ERROR.
*
* NOMANUAL
*/
 
STATUS usrPPPInit 
    (
    char *      pBootDev,		/* boot device */
    int 	unitNum, 		/* unit number */
    char *      localAddr,		/* local address */
    char *      peerAddr		/* peer address */
    )
    {
#ifdef INCLUDE_PPP
    PPP_INFO    pppInfo;
    PPP_OPTIONS *pOptions = NULL;
    char        pppTyDev [20];          	/* ppp device */
#ifdef PPP_BAUDRATE
    int		pppBaudRate = PPP_BAUDRATE;	/* ppp baud rate */
#else
    int		pppBaudRate = 0;		/* ppp baud rate */
#endif	/* PPP_BAUDRATE */
    char *	pBaudStr;			/* ppp boot string */
    int         netmask;                	/* netmask */
    int         sysRate = sysClkRateGet();	/* system clock rate */
    int         ix;

#ifdef	PPP_OPTIONS_STRUCT
    pOptions = &pppOptions;
#endif	/* PPP_OPTIONS_STRUCT */

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
            ((pppInfo.ipcp_fsm.state == OPENED) ||
	     (pppInfo.lcp_wantoptions.silent)	||
	     (pppInfo.lcp_wantoptions.passive)))
           break;
        }

    if (ix == PPP_CONNECT_DELAY)
        {
 	pppDelete (unitNum);			/* kill the interface */
        printf ("ppp0: timeout: could not establish link with peer.\n");
        return (ERROR);
 	}
 
#if CPU==SIMSPARCSOLARIS
     /* Add default route thru ppp gateway */
    routeAdd ("0.0.0.0", peerAddr);
    routeAdd (localAddr, "127.0.0.1"); /* loopback for local addresses */
#endif

    printf ("done.\n");
    return (OK);
     
#else /* INCLUDE_PPP */
    printf ("\nError: ppp not included.\n");
    return (ERROR);
#endif  /* INCLUDE_PPP */
    }


#ifdef INCLUDE_SNMPD

/* 
 * The following structure defines which MIB modules are initialized
 * by the SNMP agent.  The initialization routines are called 
 * during agent start up.  The termination routines are called 
 * after the agent is deleted.  
 */



   MIB_MODULE snmpMibModules []  =

       {   
           /* initializtion, terminate */

           { NULL, usrMib2CleanUp },
           { NULL, NULL },                 /* Last entry both must be NULL */

       };



/*****************************************************************************
*
* usrSnmpdInit - initialize the SNMP agent
* 
* This routine initializes the SNMP agent as well as the MIB-2 libraries.
*
* RETURNS: OK if successful, otherwise ERROR.
*/

STATUS usrSnmpdInit (void)

    {
    int       traceLevel = 0;

#ifdef SNMP_TRACE_LEVEL
    traceLevel = SNMP_TRACE_LEVEL;
#endif


    snmpdInit (snmpMibModules, traceLevel, 0, 0);	

    return (OK);
    }

/*******************************************************************************
*
* usrMib2Init - initialize the MIB-II library 
*
* This routine initializes the MIB-II library interface. 
*
* RETURNS: OK (always)
*/

STATUS usrMib2Init (void)
    {
#ifdef INCLUDE_MIB2_SYSTEM
    static M2_OBJECTID	sysObjectId = { MIB2_SYS_OBJID_LEN,
				        MIB2_SYS_OBJID };

    /* this includes and initializes the MIB-2 interface */

     m2SysInit (MIB2_SYS_DESCR, MIB2_SYS_CONTACT, MIB2_SYS_LOCATION,
	       &sysObjectId);
#endif /* INCLUDE_MIB2_SYSTEM */

#ifdef INCLUDE_MIB2_IF
    m2IfInit ((FUNCPTR) generateTrap, 0);
#endif /* INCLUDE_MIB2_IF */

#ifdef INCLUDE_MIB2_TCP
     m2TcpInit ();			/* the TCP group */
#endif /* INCLUDE_MIB2_TCP */ 

#ifdef INCLUDE_MIB2_UDP
     m2UdpInit ();			/* the UDP group */
#endif /* INCLUDE_MIB2_UDP */

#ifdef INCLUDE_MIB2_ICMP
      m2IcmpInit ();			/* the ICMP group */
#endif /* INCLUDE_MIB2_ICMP */

#if defined(INCLUDE_MIB2_IP) || defined(INCLUDE_MIB2_AT)
      m2IpInit (0);			/* the IP group */
#endif /* MIB_IP */
    return (OK);
    }


/*******************************************************************************
*
* usrMib2CleanUp - clean up MIB-II state
*
* RETURNS: OK (always)
*/

/*******************************************************************************
*
* usrMib2CleanUp - clean up MIB-II state
*
* RETURNS: OK (always)
*/

STATUS usrMib2CleanUp (void)
    {
#ifdef INCLUDE_MIB2_SYSTEM
    m2SysDelete ();			/* system clean up */
#endif /* INCLUDE_MIB2_SYSTEM */

#ifdef INCLUDE_MIB2_IF
    m2IfDelete ();			/* interface clean up */
#endif /* INCLUDE_MIB2_IF */

#if defined(INCLUDE_MIB2_IP) || defined(INCLUDE_MIB2_AT)
    m2IpDelete ();			/* IP clean up */
#endif /* MIB_IP */

#ifdef INCLUDE_MIB2_TCP
    m2TcpDelete ();			/* the TCP group */
#endif /* INCLUDE_MIB2_TCP */ 

#ifdef INCLUDE_MIB2_UDP
    m2UdpDelete ();			/* the UDP group */
#endif /* INCLUDE_MIB2_UDP */

#ifdef INCLUDE_MIB2_ICMP
    m2IcmpDelete ();			/* the ICMP group */
#endif /* INCLUDE_MIB2_ICMP */

    return (OK);
    }

#else					/* Case for MIB-II support only */

#if defined (INCLUDE_MIB2_ALL)
VOIDFUNCPTR _m2files[] =
    {
    (VOIDFUNCPTR) m2Init
    };
#else

#if defined(INCLUDE_MIB2_SYSTEM) || defined(INCLUDE_MIB2_IF)
#define INCLUDE_M2_FUNCS
#endif

#if defined(INCLUDE_MIB2_TCP) || defined(INCLUDE_MIB2_UDP)
#define INCLUDE_M2_FUNCS
#endif

#if defined(INCLUDE_MIB2_ICMP) || defined(INCLUDE_MIB2_IP)
#define INCLUDE_M2_FUNCS
#endif

#if defined(INCLUDE_MIB2_AT)
#define INCLUDE_M2_FUNCS
#endif

#if defined(INCLUDE_M2_FUNCS)
VOIDFUNCPTR _m2files[] =
    {
#if defined(INCLUDE_MIB2_SYSTEM)
    (VOIDFUNCPTR) m2SysInit,
#endif

#if defined(INCLUDE_MIB2_IF)
    (VOIDFUNCPTR) m2IfInit,
#endif

#if defined(INCLUDE_MIB2_TCP)
    (VOIDFUNCPTR) m2TcpInit,
#endif

#if defined(INCLUDE_MIB2_UDP)
    (VOIDFUNCPTR) m2UdpInit,
#endif

#if defined(INCLUDE_MIB2_ICMP)
    (VOIDFUNCPTR) m2IcmpInit,
#endif

#if defined(INCLUDE_MIB2_IP) || defined(INCLUDE_MIB2_AT)
    (VOIDFUNCPTR) m2IpInit,
#endif

    (VOIDFUNCPTR) 0L
    };
#endif /* INCLUDE_M2_FUNCS */
#endif /* INCLUDE_M2_ALL */


#endif /* INCLUDE_SNMPD */




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

STATUS usrStrmInit (void)
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
    strmSockInit ();		/* initialize sockmod module */
 
    if (sockLibAdd ((FUNCPTR) strmSockLibInit, AF_INET_STREAMS, AF_INET) ==
        ERROR)
        return (ERROR);
 
#ifdef  DEFAULT_STREAMS_SOCKET
    if (sockLibAdd ((FUNCPTR) strmSockLibInit, AF_INET, AF_INET) == ERROR)
        return (ERROR);
#endif	/* DEFAULT_STREAMS_SOCKET */
#endif	/* INCLUDE_STREAMS_SOCKET */

#if defined(INCLUDE_STREAMS_TLI) || defined(INCLUDE_STREAMS_ALL)
    /* initialize timod and tirdwr modules */
    if (tliInit () == ERROR)
        return (ERROR);
#endif	/* INCLUDE_STREAMS_TLI */

#if defined(INCLUDE_STREAMS_AUTOPUSH) || defined(INCLUDE_STREAMS_ALL)
    if (autopushInit () == ERROR)
	return (ERROR);         
#endif	/* INCLUDE_STREAMS_AUTOPUSH */
 
#if defined(INCLUDE_STREAMS_DLPI) || defined(INCLUDE_STREAMS_ALL)
    /* initialize the /dev/dlb devices */
    if (dlpiInit () == ERROR)
	return (ERROR);
#endif	/* INCLUDE_STREAMS_DLPI */
 
#if defined(INCLUDE_STREAMS_STRACE) || defined(INCLUDE_STREAMS_ALL) 
    strmStraceInit (STREAMS_STRACE_OUTPUT_DIR);  /* init strace utility */ 
#endif	/* INCLUDE_STREAMS_STRACE */ 
 
#if defined(INCLUDE_STREAMS_STRERR) || defined(INCLUDE_STREAMS_ALL) 
    strmStrerrInit (STREAMS_STRERR_OUTPUT_DIR);  /* init strerr utility */ 
#endif	/* INCLUDE_STREAMS_STRERR */ 
                                    
#ifdef INCLUDE_STREAMS_DEBUG
    /* initialize the Streams debug facility */
    if (strmDebugInit () == ERROR)
	return (ERROR);
#endif	/* INCLUDE_STREAMS_DEBUG */
 
    /* call user-provided protocol initialization hook routine */

    if ((strmProtoInitRtn != NULL) && ((*strmProtoInitRtn) () == ERROR))
        return (ERROR);

    return (OK);		/* Streams initialization complete */
    }
#endif /* INCLUDE_STREAMS */

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

STATUS usrNetProtoInit (void)
    {
    ipLibInit (&ipCfgParams);  		/* has to included by default */
    rawIpLibInit ();			/* has to included by default */
    rawLibInit ();

#ifdef INCLUDE_IP_FILTER
    ipFilterLibInit ();			/* include ip filter library */
#endif /* INCLUDE_IP_FILTER */

#ifdef INCLUDE_UDP
    udpLibInit (&udpCfgParams); 	/* udp protocol initialization */
#ifdef INCLUDE_NET_SHOW
    udpShowInit ();
#endif	/* INCLUDE_NET_SHOW */     
#endif	/* INCLUDE_UDP */

#ifdef INCLUDE_TCP
    tcpLibInit (&tcpCfgParams); 	/* tcp protocol initialization */
#ifdef INCLUDE_NET_SHOW
    tcpShowInit ();
#endif	/* INCLUDE_NET_SHOW */     
#endif	/* INCLUDE_TCP */

#ifdef INCLUDE_ICMP
    icmpLibInit (&icmpCfgParams);	/* icmp protocol initialization */
#ifdef INCLUDE_NET_SHOW
    icmpShowInit ();
#endif	/* INCLUDE_NET_SHOW */     
#endif	/* INCLUDE_ICMP */

#ifdef INCLUDE_IGMP
    igmpLibInit ();			/* igmp protocol initialization */
#ifdef INCLUDE_NET_SHOW
    igmpShowInit ();
#endif /* INCLUDE_NET_SHOW */     
#ifdef INCLUDE_MCAST_ROUTING
    mCastRouteLibInit ();		/* initialize multicast routing */
#endif /* INCLUDE_MCAST_ROUTING */
#endif /* INCLUDE_IGMP */    

#ifdef INCLUDE_OSPF
    ospfLibInit ();
#endif

    return (OK); 
    }


#ifdef INCLUDE_DNS_RESOLVER
LOCAL STATUS usrResolvInit ()
    {
#ifdef INCLUDE_DNS_DEBUG
    extern int dnsDebug (void);
    
    return (resolvInit (RESOLVER_DOMAIN_SERVER, RESOLVER_DOMAIN, dnsDebug));
#else
    return (resolvInit (RESOLVER_DOMAIN_SERVER, RESOLVER_DOMAIN, NULL));

#endif /* INCLUDE_DNS_DEBUG*/
    }
#endif /* INCLUDE_DNS_RESOLVER */


#endif /* __INCusrNetworkc */
