/* bootConfig.c - system configuration module for boot ROMs */

/* $Id: bootConfig.c,v 1.5 2011/07/21 16:14:16 yshtil Exp $
 * Copyright (c) 1984-2006 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
15k,18jul06,md   fixed warning about iosFdValue (WIND00039006)
15j,07jul06,pcs  Updated call to memInit to take the partition options as the
                 third parameter.
15i,08may06,dtr  SPR#120513 - check status of sysNetMacNVRamAddrGet.
15h,29mar06,mdo	 HEnd drivers do not use endDevTbl to display boot devices
15i,31may06,h_k  reverted the change for WIND00055378 in mEnet called by "N"
                 command to keep backward compatibility.
15h,26may06,wap  Don't byte swap MAC addresses (WIND00055378)
15g,09feb06,jlk  Updated iosInit() call to take the IOS_POSIX_PSE52_MODE
                 parameter.
15f,13jan06,agf  move symbols from sysLib to [usr,boot]Config (SPR 114346)
15e,06sep05,h_k  removed compressedEntry. (SPR #111954)
15d,26aug05,dlk  Remove extra call to usrKernelCoreInit().
15c,27aug05,pcm  added xbdBlkDevLibInit()
15b,11aug05,pcm  added conditional directives for INCLUDE_DOSFS_MAIN
15a,10aug05,pcm  added dosFsCacheLibInit()
14z,04aug05,dcc  added DOSFS_DEFAULT_CACHE_SIZE parameter to usrDosfsInit()
14y,03aug05,pcm	 added parameters to usrDosfsInit()
14x,15aug05,vvv  fixed warnings when DHCP is included
14w,13aug05,vvv  fixed bootrom build error
14v,04aug05,wap  add usrKernelCoreInit() to usrRoot() (SPR# 110587)
14u,14jun05,pcm  XBD'ized dos boot process
14t,08jul05,kk   add usrKernelCoreInit() to usrRoot() (SPR# 110587)
14s,04jul05,pcm  fixed scsi compile error
14r,27jun05,pcm  Renamed fsEventUtil.h to fsEventUtilLib.h
14q,23jun05,jlk  Rename of fsPathWaitLibInit() to fsEventUtilLibInit().
14p,23jun05,jlk  Added fsPathWaitLibInit().
14o,16jun05,yvp  Deleted call to jobLibTaskInit.
14n,26may05,ijm  clear bootFile in dhcpcConfigSet(), SPR#109627
14m,11may05,dlk  Use the configlette usrNetApplUtil.c rather than defining
		 a special version of usrNetApplUtilInit() here. SPR #108879.
		 Fix hashLibInit() warning. Replace #include "..." with
		 #include <...> where appropriate..
14l,15mar05,dlk  Do not define INCLUDE_OBJ_OPEN when INCLUDE_APP_LOG_UTIL is
                 defined (SPR #104991).
14k,09feb05,jmt  SPR 105999 & 106000: Fix MIPS Bootrom issue.
                 Added prototype for palInit().
14j,17feb05,tam  Updated Copyright to 2005
14i,09feb05,wap  Don't include net sysctl support in bootrom by default
14h,02feb05,vvv  fixed warning
14g,20nov04,pes  SPR 104358: Add '#define's of INCLUDE_APPL_LOG when
                 INCLUDE_NETWORK is defined, and INCLUDE_SEM_BINARY and
		 INCLUDE_OBJ_OPEN when INCLUDE_APPL_LOG is defined.
14f,29oct04,dlk  Documentation: remove extraneous () from title line of
                 usrNetApplUtilInit() routine. Make routine \NOMANUAL.
14e,13oct04,j_b  fix warnings (SPR 96917)
14d,13oct04,kk   updated kernelBaseLib.h to private/kernelBaseLibP.h
14c,11oct04,j_b  added INCLUDE_JOB_TASK and updated excInit () parameter list
14b,07oct04,pes  Fix Apigen problems caused by 13y. Change registers
                 used in WRS_ASM statements from k0 to $26 to permit
		         compilation with GNU.
14a,04oct04,pes  Add MIPS specific routines used to stub MMU functions
                 when building bootroms with .o files built for mapped kernels.
13z,30sep04,mig  Clear bss segment before cacheLibInit()
13y,30sep04,tam  removed INCLUDE_EDR_KH
13x,28sep04,dbt  Fixed TSFS boot support.
13w,23sep04,ann  merged from networking: App Log Util changes, hostname
                 initialization, vxWorks 5x equivalent PPP changes, scalability
                 changes.
13v,20jul04,rec  implement 'M' command (SPR #93020)
13u,26aug04,mem  obtain usrSysctl.c from config/comps/src instead of src/config
13t,10aug04,kk   renamed rtpBaseLib kernelBaseLib
13t,28jul04,md   removed INCLUDE_EDR_SYMBOLIC, INCLUDE_EDR_POLICIES
13s,29jun04,job  Removed usrHwSysctl.c inclusion
13r,28jun04,job  Changed INCLUDE_SYSCTL_API to INCLUDE_SYSCTL
13q,14jun04,dlk  removed SOCK_FD_MAX argument to sockLibInit(). Fixed
                 some compiler warnings
13p,14jun04,job  renamed usrSysctl2 to usrSysctl
13o,12jun04,to   re-add usrSysctl2.c
13n,03jun04,rec  change usrSysctl.c to usrHwSysctl.c
13m,01jun04,mem  added call to palInit().
13l,08jun04,elg  Fix dependencies issues when including TSFS boot method.
13k,04jun04,job  Added define of INCLUDE_NET_SYSCTL_API to fix network build
13j,10may04,rec  add call for usrSysctl
13i,21may04,job  Updated SYSCTL inclusion (split between base & network)
                 Fixed usrNetworkInit to only call sysctlInit() if
                 INCLUDE_NET_SYSCTL_API is defined.
13h,15mar04,md   Add ED&R options to boot menu
13g,21apr04,spm  added INCLUDE_SM_NET to boot with shared memory slaves
13f,14apr04,pch  SPR 95355: enable interrupts in root task after MMU is set up.
                 Fix warnings re constant conditionals involving NUM_TTY.
13e,07apr04,j_b  add startType argument to entry point called by go()
13d,05apr04,tra  fixed SPR#95129, bootrom image fails to get a lease
                 after rebooting
13c,20feb04,vvv  updated for socket/sysctl separation changes
13b,18feb04,rp   need bpf if dhcp/bootp is included
13a,06jan04,rp   fixed SPR 92684
12z,15dec03,dat  Copyright 2004
12y,05dec03,jn   Remove references to unsupported object module formats
12x,26nov03,rp   ppplite changes
12w,17nov03,rp   header file path update
12w,17nov03,aim  added edrStub
12x,20nov03,rhe  UBM Base 6 Itn 3 Merge from WRAMP branch
12w,17nov03,rhe  Added netBuf Pool files &  init routine
12v,20jun03,t_m  add INCLUDE_PCCARD
01m,15sep03,asr  updated from clarinet VOB after accordion merge
01l,12sep03,vvv  updated include paths for headers and configlettes
01k,08sep03,vvv  updated for merge from Accordion
01j,18aug03,htm  added support for BootP v4 Client
01i,17jun03,vvv  fixed to print messages by default and allow booting even if
                 target name not specified in boot parameters (SPR #88691)
01h,14may03,vvv  removed adv_net.h
01g,23apr03,vvv  updated from T2.2.1, ver12u (SPRs 65782, 83157)
01f,26mar03,vvv  fixed memory pool configuration (SPR #87113); fixed warnings
01e,18mar03,vvv  added DHCPC_DISCOVER_RETRIES (SPR #83246)
01d,06mar03,vvv  fixed netmask problem
01c,06mar03,vvv  added hostTblInit()
01b,06mar03,vvv  call usrNetHostSetup() only if the component is included
01a,28feb03,vvv  copied from T2.2, ver12q; modified for Clarinet
*/

/*
DESCRIPTION
This is the WRS-supplied configuration module for the VxWorks boot ROM.
It is a stripped-down version of usrConfig.c, having no VxWorks shell or
debugging facilities.  Its primary function is to load an object module
over the network with RSH, FTP or TFTP.  Additionally, a simple set of
single letter commands is provided for displaying and modifying memory
contents. Use this module as a starting point for placing applications
in ROM.
*/

#include <vxWorks.h>
#include <bootElfLib.h>
#include <bootLib.h>
#include <bootLoadLib.h>
#include <cacheLib.h>
#include <cbioLib.h>
#include <ctype.h>
#include <dosFsLib.h>
#include <hashLib.h>
#include <dpartCbio.h>
#include <errno.h>
#include <errnoLib.h>
#include <fcntl.h>
#include <fioLib.h>
#include <intLib.h>
#include <ioLib.h>
#include <iosLib.h>
#include <loadLib.h>
#include <logLib.h>
#include <memLib.h>
#include <msgQLib.h>
#include <pipeDrv.h>
#include <proxyLib.h>
#include <qLib.h>
#include <qPriBMapLib.h>
#include <rebootLib.h>
#include <rngLib.h>
#include <semLib.h>
#include <stdio.h>
#include <string.h>
#include <sysLib.h>
#include <sysSymTbl.h>
#include <taskHookLib.h>
#include <taskLib.h>
#include <tickLib.h>
#include <trcLib.h>
#include <unistd.h>
#include <usrFdiskPartLib.h>
#include <version.h>
#include <wdLib.h>

#include <private/excLibP.h>
#include <private/jobLibP.h>
#include <private/kernelLibP.h>
#include <private/workQLibP.h>

#include "rawFsLib.h"
#include "fsMonitor.h"
#include "drv/erf/erfLib.h"
#include "drv/manager/device.h"
#include "drv/xbd/xbd.h"
#include "fsEventUtilLib.h"

#include <netCore.h>
#include <netLib.h>
#include <ipLib.h>
#include <protos/tcpLib.h>
#include <protos/udpLib.h>
#include <arpa/inet.h>
#include <hostLib.h>
#include <ftpLib.h>
#include <tftpLib.h>
#include <private/ftpLibP.h>
#include <netDrv.h>
#include <remLib.h>
#include <sockFunc.h>
#include <sockLib.h>
#include <sys/mem_stru.h>
#include <sys/ds_conf.h>
#include <net/route.h>
#include <route/ipRouteLib.h>
#include <private/kernelBaseLibP.h>	/* for kernelBaseInit() */
#include <private/isrLibP.h>    /* for isrLibInit() */
#include <hookLib.h>        /* for hookLibInit() */
#include <private/vmLibP.h>	/* for VM_PAGE_SIZE_GET() */
#include <pmLib.h>
#include <edrLib.h>

/*
 * The BOOTCONFIG define must be before the inclusion of config.h, as
 * it affects the defaults for inclusion of some components such as TFTP.
 */

#define BOOTCONFIG

#include "config.h"
#include <scsiLib.h>

#ifdef   INCLUDE_NETWORK
#define  INCLUDE_BSD_SOCKET
#define  INCLUDE_SOCKLIB
#define  INCLUDE_DOMAIN_INIT
#define  INCLUDE_SYSCTL
#undef   INCLUDE_NET_SYSCTL
#define  INCLUDE_UNIXLIB
#define  INCLUDE_GTF
#define  INCLUDE_GTF_TIMER_START
#define  INCLUDE_NET_POOL
#define  INCLUDE_NETBUFLIB
#define  INCLUDE_APPL_LOG_UTIL
#endif

#ifdef INCLUDE_DHCPC
#undef INCLUDE_BOOTP
#endif

#ifdef INCLUDE_END
#define INCLUDE_MUX
#endif

#ifdef INCLUDE_ETHERNET
#define INCLUDE_MUX
#endif

#ifdef INCLUDE_BSD_SOCKET
#include <bsdSockLib.h>
#include <net/socketvar.h>
#endif

#ifdef INCLUDE_SYSCTL
#include <sysctlLib.h>
#endif

#ifdef INCLUDE_MUX
#include <muxLib.h>
#endif

#ifdef INCLUDE_END
#include <m2IfLib.h>
#include <endLib.h>
#endif

#if (defined(INCLUDE_DHCPC) || defined(INCLUDE_BOOTP))
#define INCLUDE_BPF
#endif /* INCLUDE_DHCPC.. */

#ifdef INCLUDE_BPF
#include <bpfDrv.h>
#endif

#ifdef  INCLUDE_FLASH_BOOT
#include <bootElfLib.h>
#include <loadElfLib.h>
#include <elf.h>
#include <elftypes.h>

#include "flashDrvLib.h"        /* Flash driver for MBZ */
#include "flashFsLib.h"     /* Flash filesystem driver */
#endif  /* INCLUDE_FLASH_BOOT */

#ifdef INCLUDE_DHCPC
#include <dhcpcBootLib.h>
#include <dhcp/dhcpcCommonLib.h>
#include <inetLib.h>
#endif /* INCLUDE_DHCPC */

#ifdef INCLUDE_BOOTP
#include <bootpLib.h>
#include <inetLib.h>
#endif /* INCLUDE_BOOTP */
#ifdef INCLUDE_PCCARD
#include <drv/pccard/csLib.h>
#endif /* INCLUDE_PCCARD */

#ifdef  INCLUDE_SM_NET
#ifndef  INCLUDE_SMEND
#warning  "INCLUDE_SM_NET requires INCLUDE_SMEND for shared memory driver!"
#endif /* !INCLUDE_SMEND */
#endif /* INCLUDE_SM_NET */

#ifdef INCLUDE_SM_NET
#include <drv/end/smEnd.h>
#endif /* INCLUDE_SM_NET */

#ifdef ETHERNET_MAC_HANDLER
#include "sysNet.h"
#endif /* ETHERNET_MAC_HANDLER */

#ifdef INCLUDE_SYS_HW_INIT_0
extern void SYS_HW_INIT_0(void);
#endif /* INCLUDE_SYS_HW_INIT_0 */
 
#ifdef  INCLUDE_FLASH
extern STATUS  flashFsLibInit(void); 
#endif  /* INCLUDE_FLASH */

/* defines */
#ifdef ETHERNET_MAC_HANDLER
#define MAX_MAC_SHOW_BUF_SIZE 40
#define IFG_MAC_SUPPLIED 1 /* remaining boot line identifies new MAC address */
#define IFG_DEV_SUPPLIED 2 /* operator elects to quit the dialog */
#define IFG_OPR_QUIT 3     /* device is selected, but no MAC address */
#define NOTEOS(arg) (*arg!=0 && (*arg!= (char)EOF))
#define SKIPSPACE(arg) while( NOTEOS(arg) && \
                        (isspace ((int) *arg))) arg++
#endif /* ETHERNET_MAC_HANDLER */

#define STREQ(A, B) (strcmp(A, B) == 0 ? 1 : 0)
#define TIMEOUT     7   /* number of seconds before auto-boot */

/* maximum line length for cmd input to 'm' and 'mEnet' routine */
#define MAX_LINE        160

/* maximum line length for bootCmdLoop() command line input */
#define BOOT_CMD_MAX_LINE        BOOT_LINE_SIZE

#define RSHD        514 /* rshd service */
#define DEC     FALSE   /* getArg parameters */
#define HEX     TRUE
#define OPT     TRUE
#define MAX_ADR_SIZE    6
#define DOS_ID_OFFSET                   3
#define FIRST_PARTITION_SECTOR_OFFSET   (0x1be + 8)
#define VXDOS                           "VXDOS"
#define VXEXT                           "VXEXT"


#ifndef    NUM_PARTITIONS_DISK_BOOT
#   define NUM_PARTITIONS_DISK_BOOT    1 /* one partition on the disk */
#endif  /* NUM_PARTITIONS_DISK_BOOT */

#ifndef    PARTITION_DISK_BOOT
#   define PARTITION_DISK_BOOT         1 /* partition mounted during boot */
#endif  /* PARTITION_DISK_BOOT */

void        usrInit ();
IMPORT void     sysInitAlt ();
#if (CPU_FAMILY==MIPS)
IMPORT void     sysGpInit ();
#endif  /* (CPU_FAMILY==MIPS) */

#if (CPU_FAMILY == PPC)
#include <arch/ppc/vxPpcLib.h>  /* for vxMsrGet() and vxMsrSet() */
IMPORT _RType taskMsrDefault;   /* from taskArchLib.c */
#endif  /* (CPU_FAMILY == PPC) */

#ifdef INCLUDE_DHCPC
LOCAL DHCP_LEASE_DATA dhcpLease;
#endif

#undef  INCLUDE_SHOW_ROUTINES       /* keep out kernel show routines */
#undef  INCLUDE_SM_NET_SHOW
#include <usrKernel.c>          /* kernel configuration facility */

/* ED&R stub functions */

#undef	INCLUDE_RTP			/* RTP support not valid in bootrom's */

#include "../comps/src/edrStub.c"

#ifdef INCLUDE_EDR_PM
#include "../comps/src/usrPmInit.c"
#endif

#ifdef INCLUDE_EDR_ERRLOG
#include "../comps/src/usrEdrInit.c"
#endif

/* imports */

IMPORT char edata [];       /* defined by the loader */
IMPORT char end [];         /* defined by the loader */

#define FREE_MEM_START_ADRS FREE_RAM_ADRS

#ifdef  INCLUDE_INITIAL_MEM_ALLOCATION
#define MEM_POOL_START_ADRS \
        (ROUND_UP(FREE_MEM_START_ADRS, (INITIAL_MEM_ALIGNMENT)) + \
        (INITIAL_MEM_SIZE))
#else   /* INCLUDE_INITIAL_MEM_ALLOCATION */
#define MEM_POOL_START_ADRS FREE_MEM_START_ADRS
#endif  /* INCLUDE_INITIAL_MEM_ALLOCATION */



#ifdef  INCLUDE_TSFS_BOOT       /* boot via Target Server File System */

#if ((WDB_COMM_TYPE == WDB_COMM_SERIAL) && \
    (CONSOLE_TTY == NONE || CONSOLE_TTY == WDB_TTY_CHANNEL))
#define INCLUDE_TSFS_BOOT_VIO_CONSOLE   /* needed for Target Server Console */
#endif

#define INCLUDE_WDB         /* WDB agent needed for TSFS Boot */
#define INCLUDE_WDB_TSFS        /* target-server file system */

#undef  INCLUDE_WDB_BANNER      /* print banner after agent starts */
#undef  INCLUDE_WDB_TTY_TEST        /* test serial line communcation */
#undef  INCLUDE_WDB_START_NOTIFY    /* notify the host of task creation */
#undef  INCLUDE_WDB_USER_EVENT      /* user events handling */
#undef  INCLUDE_WDB_CTXT        /* context control */
#undef  INCLUDE_WDB_FUNC_CALL       /* spawn function as separate task */
#undef  INCLUDE_WDB_GOPHER      /* gopher info gathering */
#undef  INCLUDE_WDB_EXIT_NOTIFY     /* notify the host of task exit */
#undef  INCLUDE_WDB_REG         /* get/set hardware registers */
#undef  INCLUDE_WDB_EVENTPOINTS     /* eventpoints handling */
#undef  INCLUDE_WDB_MEM         /* optional memory services */
#undef  INCLUDE_WDB_BP          /* breakpoint support */
#undef  INCLUDE_WDB_EXC_NOTIFY      /* exception notification */

#include <wdb/wdbEvtLib.h>
#include <usrWdb.c>                     /* WDB agent configuration */

#else   /* INCLUDE_TSFS_BOOT not defined */
#undef INCLUDE_WDB
#endif  /* INCLUDE_TSFS_BOOT */

BOOT_PARAMS sysBootParams;
SYMTAB_ID       statSymTbl;  /* XXX strerror requires it */

/*
 * ################ NETWORK INITIALIZATION #######################
 */

#ifdef INCLUDE_APPL_LOG_UTIL

/*
 * Since bootroms don't include INCLUDE_NBIO_LOG, the
 * default output destination of KERN_LOG_FD (LOG_FD_NBIO)
 * for the KERN_LOG facility will result in no output being
 * seen.  Also, STD_OUT for a VxWorks bootrom is unlikely to
 * be directed to a telnet socket for which logging from the
 * stack inside splnet() would cause a problem. So we reset
 * KERN_LOG facility logging to STD_OUT.
 */
#undef APPL_KERN_LOG_FD_CFG
#define APPL_KERN_LOG_FD_CFG STD_OUT

#undef VIRTUAL_STACK   /* Just to be sure. INCLUDE_RTP is undef'ed above. */
#include <applUtilLib.h>
#include "../comps/src/net/coreip/usrNetApplUtil.c"

#endif /* INCLUDE_APPL_LOG_UTIL */

#ifdef  INCLUDE_NETWORK /* hama */
#ifdef ETHERNET_MAC_HANDLER
void sEnet (char *Num);
void mEnet (char *pNum, int unitNum, char *ifName);
#ifdef ETHERNET_ADR_SET
#error "ETHERNET_MAC_HANDLER and ETHERNET_ADR_SET are mutually exclusive"
#endif  /* ETHERNET_ADR_SET */
#endif /* ETHERNET_MAC_HANDLER */

#ifdef ETHERNET_ADR_SET
void        mEnet (char *);
void        sysEnetAddrGet ();
void        sysEnetAddrSet ();
#ifdef ETHERNET_MAC_HANDLER
#error "ETHERNET_MAC_HANDLER and ETHERNET_ADR_SET are mutually exclusive"
#endif /* ETHERNET_MAC_HANDLER */
#endif  /* ETHERNET_ADR_SET */

#ifdef INCLUDE_END
#include "configNet.h"

IMPORT END_TBL_ENTRY endDevTbl [];

typedef struct cookie_tbl
    {
    int unitNo;
    char devName [END_NAME_MAX];
    void * pCookie;
    } COOKIE_TBL;

COOKIE_TBL cookieTbl [32];

#endif  /* INCLUDE_END */

#ifdef INCLUDE_VXBUS
#include <vxBusLib.h>
IMPORT char * muxDevConnect_desc;
LOCAL void vxbFindHend (VXB_DEVICE_ID pDev, void * pArg);
#endif /* INCLUDE_VXBUS */

#ifndef IP_MAX_UNITS
#define IP_MAX_UNITS 1
#endif

/*
 * mBlk, clBlk configuration table for network stack *SYSTEM* pool.
 * Used for network stack system structures such as routes, sockets,
 * protocol control blocks, interface addresses, mulitcast addresses,
 * and multicast routing entries.
 */
M_CL_CONFIG sysMblkClBlkDesc =
    {
    /*
      no. mBlks        no. clBlks          memArea      memSize
      -----------      ----------          -------      -------
    */
    NUM_SYS_MBLKS_MIN, NUM_SYS_CLBLKS_MIN, PMA_SYSPOOL, PMS_SYSPOOL
    };

/*
 * network stack *SYSTEM* cluster pool configuration table
 * Used for network stack system structures such as routes, sockets,
 * protocol control blocks, interface addresses, mulitcast addresses,
 * and multicast routing entries.
 */
CL_DESC sysClDescTbl [] =
    {
    /*
      clusterSize      num             memArea       memSize
      -----------      ----            -------       -------
    */
    {SIZ_SYS_16,   NUM_SYS_16_MIN,   PMA_SYS_16,   PMS_SYS_16},
    {SIZ_SYS_32,   NUM_SYS_32_MIN,   PMA_SYS_32,   PMS_SYS_32},
    {SIZ_SYS_64,   NUM_SYS_64_MIN,   PMA_SYS_64,   PMS_SYS_64},
    {SIZ_SYS_128,  NUM_SYS_128_MIN,  PMA_SYS_128,  PMS_SYS_128},
    {SIZ_SYS_256,  NUM_SYS_256_MIN,  PMA_SYS_256,  PMS_SYS_256},
    {SIZ_SYS_512,  NUM_SYS_512_MIN,  PMA_SYS_512,  PMS_SYS_512},
    {SIZ_SYS_1024, NUM_SYS_1024_MIN, PMA_SYS_1024, PMS_SYS_1024}
    };

UINT sysClDescTblNumEnt = (NELEMENTS(sysClDescTbl));


/*
 * mBlk, clBlk configuration table for network stack *DATA* pool.
 * Only used for data transfer in the network stack.
 */
M_CL_CONFIG pktMblkClBlkDesc =
    {
    /*
      no. mBlks         no. clBlks         memArea      memSize
      -----------       ----------         -------      -------
    */
    NUM_DAT_MBLKS_MIN, NUM_DAT_CLBLKS_MIN, PMA_DATPOOL, PMS_DATPOOL
    };

/*
 * network stack *DATA* cluster pool configuration table
 * Only used for data transfer in the network stack.
 */
CL_DESC pktClDescTbl [] =
    {
    /*
      clusterSize  num          memArea        memSize
      -----------  ----         -------        -------
    */
    {  64,  NUM_DAT_64_MIN,    PMA_DAT_64,    PMS_DAT_64},
    { 128,  NUM_DAT_128_MIN,   PMA_DAT_128,   PMS_DAT_128},
    { 256,  NUM_DAT_256_MIN,   PMA_DAT_256,   PMS_DAT_256},
    { 512,  NUM_DAT_512_MIN,   PMA_DAT_512,   PMS_DAT_512},
    { 1024, NUM_DAT_1024_MIN,  PMA_DAT_1024,  PMS_DAT_1024},
    { 2048, NUM_DAT_2048_MIN,  PMA_DAT_2048,  PMS_DAT_2048},
    { 4096, NUM_DAT_4096_MIN,  PMA_DAT_4096,  PMS_DAT_4096},
    { 8192, NUM_DAT_8192_MIN,  PMA_DAT_8192,  PMS_DAT_8192},
    {16384, NUM_DAT_16384_MIN, PMA_DAT_16384, PMS_DAT_16384},
    {32768, NUM_DAT_32768_MIN, PMA_DAT_32768, PMS_DAT_32768},
    {65536, NUM_DAT_65536_MIN, PMA_DAT_65536, PMS_DAT_65536}
    };

UINT pktClDescTblNumEnt = (NELEMENTS(pktClDescTbl));


#include "../comps/src/net/coreip/usrNetBoot.c"       /* INCLUDE_NET_BOOT */
#include "../comps/src/net/coreip/usrNetIpAttachCommon.c" /* INCLUDE_IPATTACH */
#include "../comps/src/net/coreip/usrNetBootConfig.c" /* INCLUDE_NET_BOOT_CONFIG */
#include "../comps/src/net/coreip/usrNetBsdSocket.c"    /* INCLUDE_BSD_SOCKET */

#ifdef INCLUDE_END
#include "../comps/src/net/coreip/usrNetEndLib.c"   /* INCLUDE_END */
#endif /* INCLUDE_END */

#ifdef INCLUDE_ETHERNET
#include "../comps/src/net/coreip/usrNetEther.c"    /* INCLUDE_ETHERNET */
#endif /* INCLUDE_ETHERNET */

#include "../comps/src/net/coreip/usrNetLoopback.c" /* INCLUDE_LOOPBACK */
#include "../comps/src/net/coreip/usrNetIf.c"           /* INCLUDE_IF */
#include "../comps/src/net/coreip/usrNetIpv4.c"     /* INCLUDE_IPV4 */
#include "../comps/src/net/coreip/usrNetIcmpv4.c"   /* INCLUDE_ICMPV4 */
#include "../comps/src/net/coreip/usrNetRawv4.c"    /* INCLUDE_RAWV4 */
#include "../comps/src/net/coreip/usrNetDaemon.c"   /* INCLUDE_NET_DAEMON */
#include "../comps/src/net/coreip/usrNetGtf.c"      /* INCLUDE_GTF */
#include "../comps/src/net/coreip/usrNetGtfTimerStart.c" /* INCLUDE_GTF_TIMER_START */
#include "../comps/src/net/coreip/usrNetRouteStorage.c" /* INCLUDE_ROUTE_STORAGE */
#include "../comps/src/net/coreip/usrNetRtSock.c"   /* INCLUDE_ROUTING_SOCKET */

#ifdef INCLUDE_UDPV4
#include "../comps/src/net/coreip/usrNetUdpv4.c"
#endif

#ifdef INCLUDE_TCPV4
#include "../comps/src/net/coreip/usrNetTcpv4.c"
#endif

#ifdef INCLUDE_IGMP
#include "../comps/src/net/coreip/usrNetIgmp.c"
#endif

#ifdef INCLUDE_HOST_TBL
#include "../comps/src/net/coreip/apps/usrNetHostTblCfg.c"
#endif

#ifdef INCLUDE_NET_REM_IO
#include "../comps/src/net/coreip/usrNetRemoteCfg.c"
#endif

#ifdef INCLUDE_BPF
#include "../comps/src/net/coreip/usrNetBpfDrv.c"
#endif

#ifdef INCLUDE_FTP
#include "../comps/src/net/coreip/apps/usrFtp.c"
#endif

#ifdef INCLUDE_SYSCTL
#include "../comps/src/usrSysctl.c"
#endif

/* Wind kernel configuration facility */

/* forward declarations */

IMPORT int  loattach ();
IMPORT struct ifnet *   ifunit ();
IMPORT int  pcmciaattach ();
IMPORT int route_init ();
IMPORT STATUS hostInstInit (void *);
IMPORT STATUS netSysctlInit (void);

extern FUNCPTR _phostAdd;
extern NET_POOL_CONFIG_PARAMS netPoolDefaultConfigParams;

#if defined(INCLUDE_DOSFS_MAIN) || defined(INCLUDE_DOSFS)
#include "usrDosfs.c"
#endif

#ifdef INCLUDE_VXWORKS_5_X_EQUIV_PPP
#include "../comps/src/net/ppp/usrBasicPPPFrameworkInit.c"
#endif /* INCLUDE_VXWORKS_5_X_EQUIV_PPP */

#endif  /* INCLUDE_NETWORK */


/* global variables */

SYMTAB_ID   sysSymTbl;
int     consoleFd;      /* fd of initial console device */
char    consoleName [20];   /* console device name, eg. "/tyCo/0" */
int     sysStartType;       /* BOOT_CLEAR, BOOT_NO_AUTOBOOT, ... */
int     sysFlags;                       /* boot flags */
char    sysBootHost [BOOT_FIELD_LEN];   /* name of boot host */
char    sysBootFile [BOOT_FIELD_LEN];   /* name of boot file */

BOOL    scsiInitialized      = FALSE;
int     bootCmdTaskPriority  = 1;
int     bootCmdTaskOptions   = VX_SUPERVISOR_MODE;
int     bootCmdTaskStackSize = BOOT_CMD_STACK_SIZE;

#if (CPU_FAMILY == MIPS)
IMPORT void palInit ();
#endif  /* (CPU_FAMILY == MIPS) */

/* Two magic cookies used to detect data section misalignment */

#define TRAP_VALUE_1    0x12348765
#define TRAP_VALUE_2    0x5a5ac3c3
LOCAL volatile UINT32   trapValue1  = TRAP_VALUE_1;
LOCAL volatile UINT32   trapValue2  = TRAP_VALUE_2;

#ifdef ETHERNET_MAC_HANDLER
LOCAL char macShowStr[MAX_MAC_ADRS] [MAX_MAC_SHOW_BUF_SIZE];
LOCAL const char macErrStr[] = "** invalid device id\n";

#endif /* ETHERNET_MAC_HANDLER */

/* forward declarations */

#ifdef __STDC__

void        usrRoot (char *pMemPoolStart, unsigned memPoolSize);
void        usrClock (void);
void        usrKernelInit (void);
LOCAL void  bootCmdLoop (void);
LOCAL char  autoboot (int timeout);
LOCAL void  printBootLogo (void);
LOCAL void  bootHelp (void);
LOCAL STATUS    bootLoad (char *bootString, FUNCPTR *pEntry);
LOCAL void  go (FUNCPTR entry);
LOCAL void  m (char *adrs);
LOCAL void  d (char *adrs, int nwords);
LOCAL void  bootExcHandler (int tid);
LOCAL void  skipSpace (char **strptr);
LOCAL void  printExcMsg (char *string);
LOCAL STATUS    getArg (char **ppString, int *pValue, BOOL defaultHex,
            BOOL optional);
LOCAL void  usrBootLineInit (int startType);
LOCAL STATUS    usrBootLineCrack (char *bootString, BOOT_PARAMS *pParams);

#ifdef  INCLUDE_NETWORK
LOCAL STATUS    netLoad (char *hostName, char *fileName, char *usr,
             char *passwd, FUNCPTR *pEntry);
LOCAL void  netifAdrsPrint (char *ifname);
LOCAL STATUS    checkInetAddrField (char *pInetAddr, BOOL subnetMaskOK);
#ifdef INCLUDE_BOOTP
LOCAL STATUS    bootpGet (char *pNetDev, char *pBootDevAddr, char *pBootFile,
              char *pHostAddr, int *pMask);
#endif
#endif  /* INCLUDE_NETWORK */

#ifdef  INCLUDE_FLASH_BOOT
LOCAL STATUS flashLoad(char    *fileName, FUNCPTR *pEntry);
#endif  /* INCLUDE_FLASH_BOOT */

#ifdef  INCLUDE_SCSI_BOOT
LOCAL STATUS    scsiLoad (int bootDevId, int bootDevLUN, char *fileName,
                  FUNCPTR *pEntry);
#endif  /* INCLUDE_SCSI_BOOT */

#ifdef  INCLUDE_FD
LOCAL STATUS    fdLoad (int drive, int type, char *fileName, FUNCPTR *pEntry);
#endif  /* INCLUDE_FD */

#ifdef  INCLUDE_IDE
LOCAL STATUS    ideLoad (int drive, int type, char *fileName, FUNCPTR *pEntry);
#endif  /* INCLUDE_IDE */

#ifdef  INCLUDE_ATA
LOCAL STATUS    ataLoad (int ctrl, int drive, char *fileName, FUNCPTR *pEntry);
#endif  /* INCLUDE_ATA */

#ifdef  INCLUDE_PCMCIA
LOCAL STATUS    pcmciaLoad (int sock, char *fileName, FUNCPTR *pEntry);
#endif  /* INCLUDE_PCMCIA */

#ifdef  INCLUDE_TFFS
LOCAL STATUS    tffsLoad (int drive, int removable, char *fileName,
              FUNCPTR *pEntry);
#endif  /* INCLUDE_TFFS */

#ifdef  INCLUDE_TSFS_BOOT
LOCAL STATUS    tsfsLoad (char * fileName, FUNCPTR * pEntry);
LOCAL void  wdbRebootEventGet (void * pNode, WDB_EVT_DATA * pEvtData);
#endif  /* INCLUDE_TSFS_BOOT */

#ifdef INCLUDE_PCCARD
LOCAL void      usrPccardDevShow (UINT16 type);
#endif /* INCLUDE_PCCARD */

#else

void        usrRoot ();
void        usrClock ();
void        usrKernelInit ();
LOCAL void  bootCmdLoop ();
LOCAL char  autoboot ();
LOCAL void  printBootLogo ();
LOCAL void  bootHelp ();
LOCAL STATUS    bootLoad ();
LOCAL void  go ();
LOCAL void  m ();
LOCAL void  d ();
LOCAL void  bootExcHandler ();
LOCAL void  skipSpace ();
LOCAL void  printExcMsg ();
LOCAL STATUS    getArg ();
LOCAL void  usrBootLineInit ();
LOCAL STATUS    usrBootLineCrack ();

#ifdef  INCLUDE_NETWORK
LOCAL STATUS    netLoad ();
LOCAL void  netifAdrsPrint ();
LOCAL STATUS    checkInetAddrField ();
LOCAL STATUS    usrNetIfConfig ();
#ifdef INCLUDE_BOOTP
LOCAL STATUS    bootpGet ();
#endif
#endif  /* INCLUDE_NETWORK */

#ifdef  INCLUDE_SCSI_BOOT
LOCAL STATUS    scsiLoad();
#endif  /* INCLUDE_SCSI_BOOT */

#ifdef  INCLUDE_FD
LOCAL STATUS    fdLoad ();
#endif  /* INCLUDE_FD */

#ifdef  INCLUDE_IDE
LOCAL STATUS    ideLoad ();
#endif  /* INCLUDE_IDE */

#ifdef  INCLUDE_ATA
LOCAL STATUS    ataLoad ();
#endif  /* INCLUDE_ATA */

#ifdef  INCLUDE_PCMCIA
LOCAL STATUS    pcmciaLoad ();
#endif  /* INCLUDE_PCMCIA */

#ifdef  INCLUDE_TFFS
LOCAL STATUS    tffsLoad ();
#endif  /* INCLUDE_TFFS */

#ifdef  INCLUDE_TSFS_BOOT
LOCAL STATUS    tsfsLoad ();
LOCAL void  wdbRebootEventGet ();
#endif  /* INCLUDE_TSFS_BOOT */

#ifdef INCLUDE_PCCARD
LOCAL void      usrPccardDevShow ();
#endif /* INCLUDE_PCCARD */

#endif  /* __STDC__ */

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
    /*
     * This trap will catch improper loading of the data section.
     * We check the magic cookie values to make sure the data section is
     * in the expected memory location. We do not want
     * to proceed further if the data segment is not correct.
     *
     * It should be easy to detect entry into the trap using an ICE, JTAG,
     * or logic analyzer. Without the trap, the processor is likely to run
     * away out of control.
     *
     * Data section misalignment can occur when there is a change in tool
     * chain, build rules, compiler, host utilities,  etc.
     */

    while (trapValue1 != TRAP_VALUE_1 || trapValue2 != TRAP_VALUE_2)
    {
    /* infinite loop */;
    }

#if (CPU_FAMILY == MIPS)
    palInit ();
#endif  /* (CPU_FAMILY == MIPS) */

#if (CPU_FAMILY == SPARC)
    excWindowInit ();               /* SPARC window management */
#endif

#if (CPU_FAMILY == MIPS)
    sysGpInit ();               /* MIPS global pointer */
#endif  /* (CPU_FAMILY == MIPS) */

#ifdef INCLUDE_SYS_HW_INIT_0
    /*
     * Perform any BSP-specific initialisation that must be done before
     * cacheLibInit() is called and/or BSS is cleared.
     */

    SYS_HW_INIT_0 ();
#endif /* INCLUDE_SYS_HW_INIT_0 */

    /* don't assume bss variables are zero before this call */

    bzero (edata, end - edata);		/* zero out bss variables */

    /* configure data and instruction cache if available and leave disabled */

#ifdef  INCLUDE_CACHE_SUPPORT
    /*
     * SPR 73609:  If a cache is not to be enabled, don't require
     * its mode to be defined.  Instead, default it to disabled.
     */
# if (!defined(USER_D_CACHE_ENABLE) && !defined(USER_D_CACHE_MODE))
#  define USER_D_CACHE_MODE CACHE_DISABLED
# endif /* !USER_D_CACHE_ENABLE && !USER_D_CACHE_MODE */
# if (!defined(USER_I_CACHE_ENABLE) && !defined(USER_I_CACHE_MODE))
#  define USER_I_CACHE_MODE CACHE_DISABLED
# endif /* !USER_I_CACHE_ENABLE && !USER_I_CACHE_MODE */

    cacheLibInit (USER_I_CACHE_MODE, USER_D_CACHE_MODE);
#endif  /* INCLUDE_CACHE_SUPPORT */

#if (CPU == SPARClite)
    cacheLib.textUpdateRtn = NULL;      /* XXX - mod hist 07u */
#endif

    sysStartType = startType;

    intVecBaseSet ((FUNCPTR *) VEC_BASE_ADRS);  /* set vector base table */

#if (CPU_FAMILY == AM29XXX)
    excSpillFillInit ();                        /* am29k stack cache management */
#endif

    excVecInit ();              /* install exception vectors */

    sysHwInit ();               /* initialize system hardware */

    usrKernelInit ();               /* configure the Wind kernel */


#if (CPU==SPARC) || (CPU_FAMILY==I80X86)    /* XXX workaround for sun1e */
#undef USER_I_CACHE_ENABLE  /* XXX disable instruction cache */
#endif  /* (CPU==SPARC) || (CPU_FAMILY==I80X86) */

#ifdef  INCLUDE_CACHE_SUPPORT
#ifdef  USER_I_CACHE_ENABLE
    cacheEnable (INSTRUCTION_CACHE);        /* enable instruction cache */
#endif  /* USER_I_CACHE_ENABLE */
#endif  /* INCLUDE_CACHE_SUPPORT */

    /* start the kernel specifying usrRoot as the root task */

    kernelInit ((FUNCPTR) usrRoot, ROOT_STACK_SIZE,
        (char *) MEM_POOL_START_ADRS,
        sysMemTop (), ISR_STACK_SIZE, INT_LOCK_LEVEL);
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

void usrRoot
    (
    char *      pMemPoolStart,          /* start of system memory partition */
    unsigned    memPoolSize             /* initial size of mem pool */
    )
    {
    char tyName [20];
    int ix;

    /* initialize core kernel facilities */

    usrKernelCoreInit();

    /*
     * Initialize the memory pool before initializing any other package.
     * The memory associated with the root task will be reclaimed at the
     * completion of its activities.
     */

#ifdef INCLUDE_MEM_MGR_FULL
    memInit (pMemPoolStart, memPoolSize,MEM_PART_DEFAULT_OPTIONS); 
#else
    memPartLibInit (pMemPoolStart, memPoolSize);/* initialize memory pool */
#endif /* INCLUDE_MEM_MGR_FULL */

    /* Initialize sysctl if required */

#ifdef INCLUDE_SYSCTL
    usrSysctlInit ();

#ifdef INCLUDE_SYSCTL_HW
    usrHwSysctlInit();                          /* System control h/w access */
#endif /* INCLUDE_SYSCTL_HW */
#endif /* INCLUDE_SYSCTL */

#if (CPU_FAMILY == PPC)
    /* Enable interrupts before setting up clock */
    vxMsrSet(vxMsrGet() | taskMsrDefault);
#endif  /* (CPU_FAMILY == PPC) */

    /* EDR configuration */

#ifdef INCLUDE_EDR_PM
    usrPmInit ();
#endif /* INCLUDE_EDR_PM */

#ifdef INCLUDE_EDR_SYSDBG_FLAG
    edrSystemDebugModeSet (FALSE);	/* always run in deployed mode */
#endif /* INCLUDE_EDR_SYSDBG_FLAG */

#ifdef INCLUDE_EDR_ERRLOG
    /* initialize the ED&R subsystem */
    usrEdrInit ();

#ifdef INCLUDE_EDR_SHOW
    edrShowInit ();
#endif /* INCLUDE_EDR_SHOW */

#endif /* INCLUDE_EDR_ERRLOG */

    /* set up system timer */

    sysClkConnect ((FUNCPTR) usrClock, 0);/* connect clock interrupt routine */
    sysClkRateSet (SYS_CLK_RATE);     /* set system clock rate */
    sysClkEnable ();              /* start it */

    /* initialize I/O and file system */
    iosInit (NUM_DRIVERS, NUM_FILES, "/null", IOS_POSIX_PSE52_MODE);
    consoleFd = NONE;

    /*
     * The select library needs to be initialized before the tyLib module
     * since the _func_selWakeupListInit FUNCPTR is required (SPR #3314).
     * The installation of the select task delete hook is not performed
     * for boot ROMs.
     */

#ifdef INCLUDE_SELECT
    selectInit (NUM_FILES);
#endif  /* INCLUDE_SELECT */

    /* install driver for on-board serial ports and make devices */

#ifdef  INCLUDE_TYCODRV_5_2
#ifdef  INCLUDE_TTY_DEV
    if ((NUM_TTY > 0))
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
    if ((NUM_TTY > 0))
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
    usrWdbInit();		/* configure and initialize the WDB agent */
    vxBootFile[0] = NULL;	/* clear boot line set by usrWdbInit() */

#if defined(INCLUDE_TSFS_BOOT) && defined(INCLUDE_TSFS_BOOT_VIO_CONSOLE)
    consoleFd = open ("/vio/0", O_RDWR, 0); /* for Target Server Console */
#endif
#endif  /* INCLUDE_WDB */

    ioGlobalStdSet (STD_IN,  consoleFd);
    ioGlobalStdSet (STD_OUT, consoleFd);
    ioGlobalStdSet (STD_ERR, consoleFd);

#ifdef  INCLUDE_PIPES
    pipeDrv ();                 /* install pipe driver */
#endif  /* INCLUDE_PIPES */

#if     defined(INCLUDE_EXC_HANDLING) && defined(INCLUDE_EXC_TASK)
#ifdef  INCLUDE_EXC_SHOW
    excShowInit ();         /* init exception show routines */
#endif
    /* initialize exception handling */
    excInit (MAX_ISR_JOBS);
#endif  /* defined(INCLUDE_EXC_HANDLING) && defined(INCLUDE_EXC_TASK) */

#if defined(INCLUDE_JOB_TASK)
    jobLibInit (JOB_TASK_STACK_SIZE);
#endif /* INCLUDE_JOB_TASK */

    excHookAdd ((FUNCPTR) bootExcHandler);  /* install exc handler */
    logInit (consoleFd, 5);         /* initialize logging */

#ifdef INCLUDE_ERF
    erfLibInit (ERF_MAX_USR_CATEGORIES, ERF_MAX_USR_TYPES);
#endif

#ifdef INCLUDE_DEVICE_MANAGER
    devInit (DEVICE_MANAGER_MAX_NUM_DEVICES);
#endif

#ifdef INCLUDE_XBD
    xbdInit ();
#endif

#ifdef INCLUDE_XBD_BLK_DEV
    xbdBlkDevLibInit ();
#endif

#ifdef INCLUDE_FS_MONITOR
    fsMonitorInit ();
#endif

#ifdef INCLUDE_FS_EVENT_UTIL
    fsEventUtilLibInit ();
#endif

#if defined(INCLUDE_DOSFS_MAIN) || defined(INCLUDE_DOSFS)
    usrDosfsInit (DOSFS_DEFAULT_MAX_FILES, DOSFS_DEFAULT_CREATE_OPTIONS);
#endif

#ifdef INCLUDE_DOSFS_CACHE
    dosFsCacheLibInit (DOSFS_DEFAULT_CACHE_SIZE);
#endif

#ifdef INCLUDE_RAWFS
    rawFsInit (NUM_RAWFS_FILES);
#endif

    /* initialize object module loader */

#if defined(INCLUDE_ELF)
    bootElfInit ();             /* use elf format */
#endif

#ifdef  INCLUDE_PCMCIA

    pcmciaInit ();                      /* init PCMCIA Lib */

#endif /* INCLUDE_PCMCIA */

#ifdef INCLUDE_PCCARD
    csInit (CS_PCCARD_STACK);
#endif /* INCLUDE_PCCARD */

/* initialize the MUX */

#ifdef INCLUDE_MUX
    /* initialize the MUX */

    if (muxLibInit() == ERROR)
    return;                              /* can't return ERROR */
#endif /* INCLUDE_MUX */

    taskSpawn ("tBoot", bootCmdTaskPriority, bootCmdTaskOptions,
        bootCmdTaskStackSize, (FUNCPTR) bootCmdLoop,
        0,0,0,0,0,0,0,0,0,0);

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
    tickAnnounce ();    /* announce system tick to kernel */
    }

/*******************************************************************************
*
* bootCmdLoop - read and execute user commands forever (until boot)
*/

LOCAL void bootCmdLoop (void)

    {
    BOOT_PARAMS params;
    char line [BOOT_CMD_MAX_LINE];
    char *pLine;
    int nwords;
    int nbytes;
    int value;
    int adr;
    int adr2;
    FUNCPTR entry;
    char key = 0;

    /* flush standard input to get rid of any garbage;
     * E.g. the Heurikon HKV2F gets junk in USART if no terminal connected.
     */

    (void) ioctl (STD_IN, FIOFLUSH, 0 /*XXX*/);

#if 0
    if (sysStartType & BOOT_CLEAR)
    printBootLogo ();
#endif

    usrBootLineInit (sysStartType);

    /* print out any new exception message -
     * the first byte is zeroed after printing so that we won't print
     * it again automatically.  However, 'e' command will still print out
     * the remainder. */

    printExcMsg (sysExcMsg);
    *sysExcMsg = EOS;       /* indicate exception message is old */

    /* start autoboot, unless no-autoboot specified */

    bootStringToStruct (BOOT_LINE_ADRS, &params);
    sysFlags = params.flags;

#if defined(INCLUDE_TSFS_BOOT) && defined(INCLUDE_TSFS_BOOT_VIO_CONSOLE)

    /* Wait for Target Server connection */

    while (!wdbTargetIsConnected())
        taskDelay (sysClkRateGet());

    /*
     * disable the auto-boot mechanism, because the Target Server may not have
     * time to start its VIO Console before the end of the auto-boot countdown.
     */

    sysStartType |= BOOT_NO_AUTOBOOT;
#endif

    if (!(sysStartType & BOOT_NO_AUTOBOOT) &&
    !(sysFlags & SYSFLG_NO_AUTOBOOT))
    {
    int timeout = TIMEOUT;

    if ((sysStartType & BOOT_QUICK_AUTOBOOT) ||
        (sysFlags & SYSFLG_QUICK_AUTOBOOT))
        {
        timeout = 1;
        }

    key = autoboot (timeout);   /* doesn't return if successful */
    }


    /* If we're here, either we aren't auto-booting, or we got an error
     * auto-booting, or the auto-booting was stopped. */

    /* put console in line mode */

    (void) ioctl (consoleFd, FIOSETOPTIONS, OPT_TERMINAL);

    /* read and execute the ROM commands */

    printf ("\n");

    FOREVER
    {
    if (key == '@')
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

    pLine = line + strlen (line) - 1;       /* point at last char */
    while ((pLine >= line) && (*pLine == ' '))
        {
        *pLine = EOS;
        pLine--;
        }

    pLine = line;
    skipSpace (&pLine);

    switch (*(pLine++))
        {
        case EOS:       /* blank line */
        break;

        case 'd':       /* display */
        if ((getArg (&pLine, &adr, HEX, OPT) == OK) &&
            (getArg (&pLine, &nwords, DEC, OPT) == OK))
            d ((char *) adr, nwords);
        break;

        case 'e':       /* exception */
        printExcMsg (sysExcMsg + 1);
        break;

        case 'f':       /* fill */
        if ((getArg (&pLine, &adr, HEX, !OPT) == OK) &&
            (getArg (&pLine, &nbytes, DEC, !OPT) == OK) &&
            (getArg (&pLine, &value, DEC, !OPT) == OK))
            {
            bfillBytes ((char *) adr, nbytes, value);
            }

        break;

        case 't':       /* transpose(?) (running out of letters!) */
        if ((getArg (&pLine, &adr, HEX, !OPT) == OK) &&
            (getArg (&pLine, &adr2, HEX, !OPT) == OK) &&
            (getArg (&pLine, &nbytes, HEX, !OPT) == OK))
            {
            bcopy ((char *) adr, (char *) adr2, nbytes);
            }
        break;

        case 'm':       /* modify */
        if (getArg (&pLine, &adr, HEX, !OPT) == OK)
            m ((char *) adr);
        break;

#ifdef  TARGET_HK_V2F
        case 's':       /* system controller */
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
#endif  /* TARGET_HK_V2F */

#if defined(TARGET_FRC_30) || defined(TARGET_FRC_31) || defined(TARGET_FRC_33)
        case 's':       /* system controller */
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
#endif  /* TARGET_FRC_30 || TARGET_FRC_31 || TARGET_FRC_33 */

        case 'p':       /* print boot params */
        bootParamsShow (BOOT_LINE_ADRS);
        break;

        case 'c':       /* change boot params */
        bootParamsPrompt (BOOT_LINE_ADRS);
                if (strlen(BOOT_LINE_ADRS) <= BOOT_LINE_SIZE)
                    {
                    if ((sysNvRamSet (BOOT_LINE_ADRS,
                        strlen (BOOT_LINE_ADRS) + 1, 0)) == ERROR)
                        printf("\nNOTE: Bootline not saved to NVRAM\n");
                    }
                else
                    printf("\nWARNING:Bootline longer than BOOT_LINE_SIZE "
               "(%d bytes). Not saved to NVRAM\n",
               (int) BOOT_LINE_SIZE);
                break;

        case 'g':       /* go */
        {
        int entryAddr;
        if (getArg (&pLine, &entryAddr, HEX, !OPT) == OK)
            {
            entry = (FUNCPTR)entryAddr;
            go (entry);
            }
        }
        break;

#ifdef  INCLUDE_NETWORK
        case 'n':
        netifAdrsPrint (pLine);
        break;

#ifdef ETHERNET_ADR_SET
        case 'N': /* this command is replaced by the 'M' command below */
        mEnet (pLine);
        break;
#endif  /* ETHERNET_ADR_SET */

#ifdef ETHERNET_MAC_HANDLER
        case 'M':
        sEnet (pLine);
        break;
#endif  /* ETHERNET_MAC_HANDLER */

#endif  /* INCLUDE_NETWORK */

        case '?':           /* help */
            case 'h':           /* help */
        bootHelp ();
        break;

            case '@':           /* load and go with internal params */
        case '$':           /* load and go with internal params */

        if (bootLoad (pLine, &entry) == OK)
            {
            go (entry);
            }
        else
            {
            taskDelay (sysClkRateGet ());   /* pause a second */
            reboot (BOOT_NO_AUTOBOOT);      /* something is awry */
            }
        break;

        case 'l':           /* load with internal params */

        if (bootLoad (pLine, &entry) == OK)
            {
            printf ("entry = 0x%x\n", (int) entry);
            }
        else
            {
            taskDelay (sysClkRateGet ());   /* pause a second */
            reboot (BOOT_NO_AUTOBOOT);      /* something is awry */
            }
        break;

            case 'v':                   /* print version & logo */

                printBootLogo ();
                break;

#ifdef INCLUDE_EDR_ERRLOG
        case 'P':           /* display ED&R log */
        edrShow (0,0,0,0);
        break;

        case 'C':           /* clear ED&R log */
        edrClear ();
        break;
#endif

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
    int timeout     /* timeout time in seconds */
    )
    {
    ULONG   autoBootTime;
    int     timeLeft;
    UINT    timeMarker;
    int     bytesRead = 0;
    FUNCPTR entry;
    char    key;

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
        go (entry);             /* ... and never return */
    else
        {
        printf ("Can't load boot file!!\n");
        taskDelay (sysClkRateGet ());   /* pause a second */
        reboot (BOOT_NO_AUTOBOOT);      /* something is awry */
        }
    }
    else
    {
    /* read the key that stopped autoboot */

    read (consoleFd, &key, 1);
    return (key & 0x7f);        /* mask off parity in raw mode */
    }

    return ((char)ERROR);           /* for lint - can't really get here */
    }

/******************************************************************************
*
* printBootLogo - print initial boot banner page
*/

LOCAL void printBootLogo (void)

    {
    printf ("\n\n\n\n\n\n\n\n\n\n\n");
    printf ("%28s%s", "","VxWorks System Boot");
    printf ("\n\n\nCopyright 1984-2005  Wind River Systems, Inc.\n\n\n\n\n\n");
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
    "@",                      "- boot (load and go)",
    "p",                      "- print boot params",
    "c",                      "- change boot params",
    "l",                      "- load boot file",
    "g adrs",                 "- go to adrs",
    "d adrs[,n]",             "- display memory",
    "m adrs",                 "- modify memory",
    "f adrs, nbytes, value",  "- fill memory",
    "t adrs, adrs, nbytes",   "- copy memory",
    "e",                      "- print fatal exception",
        "v",                      "- print boot logo with version",
#ifdef INCLUDE_EDR_ERRLOG
        "P",                      "- print error log",
        "C",                      "- clear error log",
#endif /* INCLUDE_EDR_ERRLOG */
#ifdef INCLUDE_NETWORK
    "n netif",        "- print network interface device address",
#if defined(ETHERNET_ADR_SET)
    "N",              "- set ethernet address",
#endif  /* ETHERNET_ADR_SET */
#if defined(ETHERNET_MAC_HANDLER)
        "M [dev][unitNo] [MAC]",  "- set/display ethernet address",
#endif  /* ETHERNET_MAC_HANDLER */
#endif  /* INCLUDE_NETWORK */
#if defined(TARGET_HK_V2F) || defined(TARGET_FRC_30) || \
    defined(TARGET_FRC_31) || defined(TARGET_FRC_33)
    "s [0/1]",                "- system controller 0 = off, 1 = on",
#endif  /* TARGET_HK_V2F/FRC_30/FRC_31/FRC_33 */
    "$dev(0,procnum)host:/file h=# e=# b=# g=# u=usr [pw=passwd] f=#", "",
    "                          tn=targetname s=script o=other", "",

#ifdef  INCLUDE_SCSI_BOOT
    "boot device: scsi=id,lun              file name: /sd0/vxWorks","",
#endif  /*INCLUDE_SCSI_BOOT*/

#ifdef  INCLUDE_FD
    "boot device: fd=drive,fdType          file name: /fd0/vxWorks","",
#endif  /* INCLUDE_FD */

#ifdef  INCLUDE_IDE
    "boot device: ide=drive,configType     file name: /ide0/vxWorks","",
#endif  /* INCLUDE_IDE */

#ifdef  INCLUDE_ATA
    "boot device: ata=ctrl,drive           file name: /ata0/vxWorks","",
#endif  /* INCLUDE_ATA */

#ifdef  INCLUDE_PCMCIA
    "boot device: pcmcia=sock              file name: /pcmcia0/vxWorks","",
#endif  /* INCLUDE_PCMCIA */

#ifdef  INCLUDE_TFFS
    "boot device: tffs=drive,removable     file name: /tffs0/vxWorks","",
#endif  /* INCLUDE_TFFS */

#ifdef INCLUDE_TSFS_BOOT
    "boot device: tsfs                     file name: /tgtsvr/vxWorks","",
#endif /*INCLUDE_TSFS_BOOT */

    "Boot flags:",        "",
#if defined(TARGET_HK_V2F) || defined(TARGET_FRC_30) || \
    defined(TARGET_FRC_31) || defined(TARGET_FRC_33)
    "  0x01  - don't be system controller", "",
#endif  /* TARGET_HK_V2F/FRC_30/FRC_31/FRC_33 */
    "  0x02  - load local system symbols",      "",
    "  0x04  - don't autoboot",         "",
    "  0x08  - quick autoboot (no countdown)",  "",
#ifdef  INCLUDE_NETWORK
    "  0x20  - disable login security",     "",
#ifdef  INCLUDE_DHCPC
    "  0x40  - use dhcp to get boot parameters",    "",
#else
#ifdef INCLUDE_BOOTP
    "  0x40  - use bootp to get boot parameters",   "",
#else
    "  0x40  - autoconfigure: NOT AVAILABLE (no method installed)", "",
#endif /* INCLUDE_BOOTP */
#endif /* INCLUDE_DHCPC */
    "  0x80  - use tftp to get boot image",     "",
    "  0x100 - use proxy arp",          "",
#endif  /* INCLUDE_NETWORK */
    NULL
    };

    FAST char **pMsg;
#ifdef INCLUDE_END
    char       muxDevName[8];
    END_TBL_ENTRY * pDevTbl;
#endif /* INCLUDE_END */

    printf ("\n");

    for (pMsg = helpMsg; *pMsg != NULL; pMsg += 2)
    printf (" %-21s %s\n", *pMsg, *(pMsg + 1));

#ifdef  INCLUDE_NETWORK

    printf ("\navailable boot devices:");

#ifdef INCLUDE_END
    printf ("\nEnhanced Network Devices\n");

    for (pDevTbl = endDevTbl; pDevTbl->endLoadFunc != END_TBL_END; pDevTbl++)
        {
        if (!pDevTbl->processed)
            {
            muxDevName [0] = EOS;
            pDevTbl->endLoadFunc (muxDevName, NULL);
            printf (" %s%d", muxDevName, pDevTbl->unit);
            }
        }
#endif /* INCLUDE_END */
#ifdef INCLUDE_VXBUS
	vxbDevIterate((FUNCPTR)vxbFindHend,(void *)NULL,VXB_ITERATE_INSTANCES);
#endif /* INCLUDE_VXBUS */
#endif  /* INCLUDE_NETWORK */

#ifdef  INCLUDE_SCSI_BOOT
    printf (" scsi");
#endif  /*INCLUDE_SCSI_BOOT*/

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
    printf (" tffs");
#endif  /* INCLUDE_TFFS */

#ifdef  INCLUDE_TSFS_BOOT
    printf (" tsfs");
#endif  /* INCLUDE_TSFS_BOOT */

#ifdef INCLUDE_PCCARD
    printf ("\n\nPC Card boot devices:\n");
    usrPccardDevShow (CS_END_ENABLER);
    usrPccardDevShow (CS_ATA_ENABLER);
#endif /* INCLUDE_PCCARD */

    printf ("\n");
    }

#ifdef INCLUDE_NETWORK

#ifdef INCLUDE_BOOTP
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
    char *pNetDev,      /* boot device */
    char *pBootDevAddr,     /* device address */
    char *pBootFile,        /* file name */
    char *pHostAddr,        /* host address */
    int  *pMask         /* mask */
    )
    {
#ifndef INCLUDE_DHCPC
    struct bootpParams      bootParams;     /* parameter descriptor */
    struct in_addr      clntAddr;   /* client address */
    struct in_addr      hostAddr;   /* server address */
    struct in_addr      subnetMask; /* subnet mask */
    struct ifnet *              pIf;            /* network boot device */

    char    bootServer [INET_ADDR_LEN];/* boot server */

    subnetMask.s_addr         = 0;

    bzero ( (char *)&clntAddr, sizeof (struct in_addr));
    bzero ( (char *)&hostAddr, sizeof (struct in_addr));
    bzero (bootServer, INET_ADDR_LEN);
    bzero ((char *)&bootParams, sizeof (struct bootpParams));

    pIf = ifunit (pNetDev);
    if (pIf == NULL)
    return (ERROR);

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

    bootParams.netmask = &subnetMask;

    printf ("Getting boot parameters via network interface %s", pNetDev);

    if (bootpLibInit (BOOTP_MAX_HDRSIZE) == ERROR)
        return (ERROR);

    if (bootpParamsGet (pIf, BOOTP_MAX_REQUESTS, &clntAddr, &hostAddr,
                NULL, pBootFile, &bootParams) == ERROR)
    return (ERROR);

    inet_ntoa_b (hostAddr, bootServer);

    printf ("\nBootp Server:%s\n", bootServer);

    if (pBootFile [0] == EOS)
    return (ERROR);             /* no bootfile */

    printf ("    Boot file: %s\n", pBootFile);

    /* copies to params.had */

    if (pHostAddr [0] == EOS)           /* fill in host address */
    {
    strncpy (pHostAddr, bootServer, INET_ADDR_LEN);
    printf ("    Boot host: %s\n", pHostAddr);
    }

    /*
     * copies to pBootDevAddr (params.ead or params.bad) if not using bp or sm
     * drivers and address is not already present.
     */

    if (pBootDevAddr [0] == EOS)        /* fill in inet address */
    {
        inet_ntoa_b (clntAddr, pBootDevAddr);
    printf ("    Boot device Addr (%s): %s\n", pNetDev, pBootDevAddr);
    }

    /* copies to netmask */

    if ((*pMask == 0) && (subnetMask.s_addr != 0))
    {
        *pMask = ntohl (subnetMask.s_addr);
    printf ("    Subnet mask: 0x%x\n", *pMask);
    }

    return (OK);
#else
    return (OK);            /* DHCP client used instead. */
#endif /* INCLUDE_DHCPC */
    }
#endif /* INCLUDE_BOOTP */

#ifdef INCLUDE_DHCPC
/******************************************************************************
*
* dhcpGet - get boot parameters with DHCP
*
* This routine retrieves a boot file name, host and target IP addresses, and
* subnet mask from a DHCP or BOOTP server, using the lease negotiation process
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
* DHCP client is included in the boot program.
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
    char *              pNetDev,        /* boot device */
    char *              pBootDevAddr,   /* device IP address */
    char *              pBootFile,      /* boot file name */
    char *              pHostAddr,      /* host IP address */
    int *               pMask,          /* target subnet mask */
    DHCP_LEASE_DATA *   pDhcpLease      /* lease times and addresses */
    )
    {
    STATUS              result;
    struct ifnet *      pIf;            /* pointer to network interface data */
    char                serverAddr [INET_ADDR_LEN];   /* DHCP server address */
    char                bootFile [BOOT_FILE_LEN];       /* name of boot file */
    int                 subnetMask;                     /* subnet mask */
    void *              pCookie;
    unsigned long       duration;

    struct dhcp_param bootParams;

    bzero (serverAddr, INET_ADDR_LEN);
    bzero ((char *) &bootParams, sizeof (struct dhcp_param));
    bootFile [0] = EOS;

    /*
     * Using pBootFile directly only works if all the DHCP servers supply a
     * bootfile. The Windows NT server does not, so we can't do this.
     */
    /* bootParams.file = pBootFile;  - Desired assignment to save memory. */

    bootParams.file = bootFile;
    bootParams.subnet_mask = (struct in_addr *) &subnetMask;

    pIf = ifunit (pNetDev);
    if (pIf == NULL)
        return (ERROR);

    printf ("Getting boot parameters via network interface %s.\n", pNetDev);

    /* Setup client to retrieve address information from a DHCP server. */

    pCookie = dhcpcBootInit (pIf, DHCPC_SPORT, DHCPC_CPORT, DHCPC_MAX_MSGSIZE,
                             DHCPC_OFFER_TIMEOUT, DHCPC_DEFAULT_LEASE,
                             DHCPC_MIN_LEASE);
    if (pCookie == NULL)
        {
        printf ("Error initializing DHCP client.\n");
        return (ERROR);
        }

#if (DHCPC_DISCOVER_RETRIES)
    dhcpcDiscoverRetries = DHCPC_DISCOVER_RETRIES;
#endif

    /*
     * Set requested lease length to value from bootline, and
     * make sure we specify it in network byte order.
     */

    duration = htonl (pDhcpLease->lease_duration);
    dhcpcOptionAdd (pCookie, _DHCP_LEASE_TIME_TAG, sizeof (int),
                    (UCHAR *) &duration);

    if (pBootDevAddr [0] == EOS)
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

        bcopy ((char *) &bootParams.yiaddr, (char *) &pDhcpLease->yiaddr,
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

    if ((*pMask == 0) && (subnetMask != 0))
        {
        *pMask = ntohl (subnetMask);
        printf ("    Subnet mask: 0x%x\n", *pMask);
        }

    return (OK);
    }
#endif /* INCLUDE_DHCPC */

/*******************************************************************************
*
* usrNetPoolConfig - initialize the network memory pools
*
* This routine allocates memory for and initializes the network memory pools.
*
* RETURNS: N/A
*/

LOCAL void usrNetPoolConfig ()
    {
    netPoolDefaultConfigParams.cfgh.len = sizeof (NET_POOL_CONFIG_PARAMS);

    netPoolDefaultConfigParams.sysMblkClBlkConf = &sysMblkClBlkDesc;
    netPoolDefaultConfigParams.sysClDesc = &sysClDescTbl [0];
    netPoolDefaultConfigParams.sysClDescNum = sysClDescTblNumEnt;

    netPoolDefaultConfigParams.pktMblkClBlkConf = &pktMblkClBlkDesc;
    netPoolDefaultConfigParams.pktClDesc = &pktClDescTbl [0];
    netPoolDefaultConfigParams.pktClDescNum = pktClDescTblNumEnt;

    /* initialize _pNetDPool, _pNetSysPool */

    mbinit (&netPoolDefaultConfigParams);

#ifdef INCLUDE_NET_SYSCTL
    mbufSysctlInit ();
#endif
    }

/*******************************************************************************
*
* usrNetInit - initializes static tables for network stack
*
* This routine initializes static tables for the network stack.
*
* RETURNS: OK, or ERROR if initialization fails
*/

LOCAL STATUS usrNetInit ()
    {
    int status;

    /* set up the default configuration parameters */

    status = netCoreSetup (MAX_LINKHDR_CFG, MAX_PROTOHDR_CFG,
			   NMBCLUSTERS_CFG, IP_MAX_UNITS);
    if (status == ERROR)
    return (ERROR);

#ifdef INCLUDE_NET_SYSCTL
    netSysctlInit ();
#endif

    return (OK);
    }

/*******************************************************************************
*
* usrNetworkInit - bootrom version of network initialization
*
* This routine initializes all network components that are necessary for
* booting vxWorks.
*
* The bootrom version of network initialization is slightly different from
* the project facility version of InitGroup usrNetworkInit or the one in
* usrNetwork.c regarding to the following:
*
* RETURNS: OK or ERROR
*/

LOCAL STATUS usrNetworkInit (void)
    {
#ifdef INCLUDE_DHCPC
    char netDev [BOOT_DEV_LEN + 1];
#endif
#ifdef INCLUDE_BOOTP
    char netDev [BOOT_DEV_LEN + 1];
#endif

    /* by default print messages during network initialization */

    _func_printErr = (FUNCPTR) printf;

#ifdef INCLUDE_APPL_LOG_UTIL
    usrNetApplUtilInit (); 
#endif

    usrNetBoot ();				/* INCLUDE_NET_BOOT */

    /* check if booting with shared memory driver */

#ifdef INCLUDE_SM_NET
    if (strncmp (sysBootParams.bootDev, "sm", 2) == 0)
       {
       if (sysProcNumGet () == 0)
      {
          printf ("Error: processor number must be non-zero to boot from sm\n");
      return (ERROR);
          }

    pAddrString = sysBootParams.bad;
    backplaneBoot = TRUE;
        }
#endif /* INCLUDE_SM_NET */

    usrNetmaskGet ();                           /* INCLUDE_NETMASK_GET */

    if (usrNetInit (NULL) == ERROR)     /* INCLUDE_NET_INIT */
    return (ERROR);

    netBufLibInit ();               /* INCLUDE_NETBUFLIB */
#ifdef INCLUDE_NETBUFPOOL
    netBufPoolInit ();              /* INCLUDE_NETBUFPOOL */
#endif
#ifdef INCLUDE_LINKBUFPOOL
    linkBufPoolInit ();             /* INCLUDE_LINKBUFPOOL */
#endif

    if (usrNetDaemonInit (NET_JOB_NUM_CFG,  /* INCLUDE_NET_DAEMON */
                  NET_TASK_PRIORITY,
                  NET_TASK_OPTIONS,
                  NET_TASK_STACKSIZE) == ERROR)
        return (ERROR);

    if (usrNetGtfInit () == ERROR)      /* INCLUDE_GTF */
    return (ERROR);

    usrNetPoolConfig ();                    /* INCLUDE_NET_POOL */

    usrNetRouteStorageInit ();                  /* INCLUDE_ROUTE_STORAGE */

                    /* InitGroup usrNetSocketInit */
    if (sockLibInit () == ERROR)        /* INCLUDE_SOCKLIB */
    return (ERROR);

    if (usrNetBsdSocket () == ERROR)        /* INCLUDE_BSD_SOCKET */
    return (ERROR);

                    /* InitGroup usrNetProtoInit */
    usrNetIfInit ();                            /* INCLUDE_IF */
    usrNetIpv4Init ();              /* INCLUDE_IPV4 */
    usrNetIcmpv4Init ();            /* INCLUDE_ICMPV4 */

#ifdef INCLUDE_UDPV4
    usrNetUdpv4Init ();
#endif

#ifdef INCLUDE_TCPV4
    usrNetTcpv4Init ();
#endif

    usrNetRawv4Init ();                 /* INCLUDE_RAWV4 */

#ifdef INCLUDE_IGMP
    usrNetIgmpInit ();
#endif

    usrNetRtSockInit ();            /* INCLUDE_ROUTING_SOCKET */

                    /* InitGroup usrNetMib2Init */
    if (m2IfInit (MIB2IF_TRAP_RTN,          /* INCLUDE_MIB2_IF */
              (void *) MIB2IF_TRAP_ARG) == ERROR)
        return (ERROR);

    domaininit (NULL);              /* INCLUDE_DOMAIN_INIT */
    route_init ();              /* INCLUDE_ROUTE */
    usrNetGtfTimerStart ();         /* INCLUDE_GTF_TIMER_START */

                    /* InitGroup usrNetDevStart */
    usrNetLoopbackStart ();                 /* INCLUDE_LOOPBACK */

#ifdef INCLUDE_ETHERNET
    if (usrNetEther () == ERROR)        /* INCLUDE_ETHERNET */
    return (ERROR);
#endif

#ifdef INCLUDE_END
    usrNetEndLibInit ();            /* INCLUDE_END */
#endif

#ifdef INCLUDE_BPF
    if (usrNetBpfDrvInit () == ERROR)
    return (ERROR);
#endif

    if (strncmp (sysBootParams.bootDev, "ppp", 3) == 0)
        {
        /* booting via ppp */
#ifdef INCLUDE_VXWORKS_5_X_EQUIV_PPP
        usrBasicPPPFrameworkInit ();

        printf ("Attaching network interface ppp...\n");

        pppBootHandle = usrWindNetPPPInit (sysBootParams.bootDev,
                                           sysBootParams.ead,
                                           ((sysBootParams.gad[0] == EOS)?
                                            sysBootParams.had:
                                            sysBootParams.gad));
        if (pppBootHandle == NULL)
            return (ERROR);

        rebootHookAdd ((FUNCPTR) pppRebootHook);
#else
        printf ("\nError: PPP not included.\n");
        return (ERROR);
#endif /* INCLUDE_VXWORKS_5_X_EQUIV_PPP */
        }
    else
        {
        usrNetIpAttachCommon (pDevName, uNum,   /* INCLUDE_IPATTACH */
                      "IPv4", ipAttach);

#ifdef INCLUDE_SM_NET
        /*
         * Shared memory slaves do not require an address from the boot
         * parameters. If the backplane address is not given, those devices
         * will retrieve an address from the shared memory master.
         */

        if (backplaneBoot)
            {
            if (sysBootParams.bad [0] == EOS &&
                (smEndInetAddrGet ("sm", uNum, NONE, sysBootParams.bad) == OK))
                {
                printf ("SM backplane IP address: %s\n", sysBootParams.bad);
                }

            if (sysBootParams.bad [0] == EOS)
                {
                printf ("Error: no backplane address specified.\n");
                return (ERROR);
                }

            /* get gateway address if no specified */

            if ((sysBootParams.gad [0] == EOS) && !(sysFlags & SYSFLG_PROXY))
                {
                struct in_addr host;    /* Internet Address */
                struct in_addr backpl;  /* Internet Address */

                host.s_addr = inet_addr (sysBootParams.had);
                backpl.s_addr = inet_addr (sysBootParams.bad);

                if (in_netof(host) != in_netof(backpl))
                    {
                    /* Gateway address is available (assumed to be master) */

                    if (smEndInetAddrGet ("sm", uNum,
                                          0, sysBootParams.gad) == OK)
                        printf ("Gateway inet address: %s\n",
                                sysBootParams.gad);
                    }
                }
            }
#endif /* INCLUDE_SM_NET */
                                        /* InitGroup usrNetworkAddrInit */
        if (sysFlags & SYSFLG_AUTOCONFIG)
        {
#ifdef INCLUDE_DHCPC
            sprintf (netDev, "%s%d", pDevName, uNum);

            if (dhcpGet (netDev, pAddrString, sysBootParams.bootFile,
                 sysBootParams.had, &netmask, &dhcpLease) == ERROR)
                return (ERROR);
#else
#ifdef INCLUDE_BOOTP
            sprintf (netDev, "%s%d", pDevName, uNum);

            if (bootpGet (netDev, pAddrString, sysBootParams.bootFile, sysBootParams.had,
                         &netmask) == ERROR)
                return (ERROR);
#else
            if (_func_printErr)
            (*_func_printErr) ("automatic address assignment requested but not included.\n");
            return (ERROR);
#endif
#endif
            }

        if (usrNetBootConfig (pDevName, uNum,   /* INCLUDE_NET_BOOT_CONFIG */
                              pAddrString, netmask,
                              sysBootParams.gad) == ERROR)
            return (ERROR);
        }

#ifdef INCLUDE_HOST_TBL
    usrNetHostTblSetup ();
#endif

/* InitGroup usrNetRemoteInit */

#ifdef INCLUDE_REMLIB
    remLibInit (RSH_STDERR_SETUP_TIMEOUT);
#endif

#ifdef INCLUDE_NET_REM_IO
    usrNetRemoteCreate ();          /* INCLUDE_NET_REMIO, NET_DRV */
#endif
                    /* InitGroup usrNetAppInit */
#ifdef INCLUDE_FTP
    usrFtpStart ();
#endif
    return (OK);
    }

#endif /* INCLUDE_NETWORK */

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
#ifdef  INCLUDE_SCSI_BOOT
#ifdef  INCLUDE_SCSI2
     SCSI_OPTIONS   options;
     UINT       which;
     int        devBusId;
#endif  /* INCLUDE_SCSI2 */
#endif  /* INCLUDE_SCSI_BOOT */
#ifdef  INCLUDE_NETWORK
    int         result;     /* classification of address string */
    unsigned long   leaseLen;   /* lease length field (optional) */
    char                buf [30];
    char        bootDev [BOOT_DEV_LEN];
    char *      pBootAddr;
    IMPORT int          netTaskPriority;
    int                 oldTaskPriority;
    STATUS              status;

#endif  /* INCLUDE_NETWORK */

    /* copy bootString to low mem address, if specified */

    if ((bootString != NULL) && (*bootString != EOS))
        strcpy (BOOT_LINE_ADRS, bootString);

    /* interpret boot command */

    if (usrBootLineCrack (BOOT_LINE_ADRS, &sysBootParams) != OK)
        return (ERROR);

    /* Display boot parameters */

    bootParamsShow (BOOT_LINE_ADRS);

    /* set our processor number: may establish vme access, etc. */

    sysFlags = sysBootParams.flags;
    sysProcNumSet (sysBootParams.procNum);

#ifdef  INCLUDE_SCSI_BOOT

    /*
     * initialize either the SCSI1 or SCSI2 interface; initialize SCSI2 when
     * the SCSI2 interface is available.
     */

#ifndef INCLUDE_SCSI2
    scsi1IfInit ();
#else
    scsi2IfInit ();
#endif

    if (strncmp (sysBootParams.bootDev, "scsi", 4) == 0)
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

        /*
         * check for absence of bus ID and LUN, in which case
         * auto-configure and display results
         */

        if (sysBootParams.bootDev[4] == EOS)
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

        sscanf (sysBootParams.bootDev, "%*4s%*c%d%*c%d",
                &bootDevId, &bootDevLUN);

        if (scsiLoad (bootDevId, bootDevLUN,
                      sysBootParams.bootFile, pEntry) != OK)
            {
            printErr ("\nError loading file: errno = 0x%x.\n", errno);
            return (ERROR);
            }

        return (OK);
        }
#endif  /* INCLUDE_SCSI_BOOT */

#ifdef  INCLUDE_FD
    if (strncmp (sysBootParams.bootDev, "fd", 2) == 0)
        {
        int type = 0;
        int drive = 0;

        if (sysBootParams.bootDev[2] == EOS)     /* was strlen() == 2 */
            return (ERROR);

        sscanf (sysBootParams.bootDev, "%*2s%*c%d%*c%d", &drive, &type);

        if (fdLoad (drive, type, sysBootParams.bootFile, pEntry) != OK)
            {
            printErr ("\nError loading file: errno = 0x%x.\n", errno);
            return (ERROR);
            }

        return (OK);
        }
#endif  /* INCLUDE_FD */

#ifdef  INCLUDE_IDE

    if (strncmp (sysBootParams.bootDev, "ide", 3) == 0)
        {
        int type = 0;
        int drive = 0;

        if (sysBootParams.bootDev[3] == EOS)
            return (ERROR);

        sscanf (sysBootParams.bootDev, "%*3s%*c%d%*c%d", &drive, &type);

        if (ideLoad (drive, type, sysBootParams.bootFile, pEntry) != OK)
            {
            printErr ("\nError loading file: errno = 0x%x.\n", errno);
            return (ERROR);
            }

        return (OK);
        }

#endif  /* INCLUDE_IDE */

#ifdef  INCLUDE_ATA

    if (strncmp (sysBootParams.bootDev, "ata", 3) == 0)
        {
        int ctrl  = 0;
        int drive = 0;

        if (sysBootParams.bootDev[3] == EOS)
            return (ERROR);

        sscanf (sysBootParams.bootDev, "%*3s%*c%d%*c%d", &ctrl, &drive);

        if (ataLoad (ctrl, drive, sysBootParams.bootFile, pEntry) != OK)
            {
            printErr ("\nError loading file: errno = 0x%x.\n", errno);
            return (ERROR);
            }

        return (OK);
        }
#endif  /* INCLUDE_ATA */

#ifdef  INCLUDE_PCMCIA

    if (strncmp (sysBootParams.bootDev, "pcmcia", 6) == 0)
        {
        int sock    = NONE;

        if (sysBootParams.bootDev[6] == EOS)
            return (ERROR);

        sscanf (sysBootParams.bootDev, "%*6s%*c%d", &sock);

        if (pcmciaLoad (sock, sysBootParams.bootFile, pEntry) == OK)
            return (OK);

        /*
         * fall through if the PC card is not a block device.
         * let's try to boot it from an ethernet device.
         */

        printErr ("\nCannot load from PCMCIA block device."
                  "\nTrying alternate PCMCIA devices ... ");
        }

#endif  /* INCLUDE_PCMCIA */

#ifdef  INCLUDE_FLASH_BOOT
    if (strncmp (sysBootParams.bootDev, "flash:", 5) == 0)
        {
        if (flashLoad (sysBootParams.bootFile, pEntry) != OK)
            {
            printErr ("\nError loading file: errno = 0x%x.\n", errno);
            return (ERROR);
            }

        return (OK);
        }
#endif  /* INCLUDE_FLASH_BOOT */

#ifdef  INCLUDE_TFFS

    if (strncmp (sysBootParams.bootDev, "tffs", 4) == 0)
        {
        int drive = 0;
        int removable = 0;

        if (sysBootParams.bootDev[4] == EOS)
            return (ERROR);

        sscanf (sysBootParams.bootDev, "%*4s%*c%d%*c%d", &drive, &removable);

        /* tffsLoad () should be after pcmciaInit () */

        if (tffsLoad (drive, removable, sysBootParams.bootFile, pEntry) != OK)
            {
            printErr ("\nError loading file: errno = 0x%x.\n", errno);
            return (ERROR);
            }

        return (OK);
        }

#endif  /* INCLUDE_TFFS */

#ifdef  INCLUDE_TSFS_BOOT

    if (strncmp (sysBootParams.bootDev, "tsfs", 4) == 0)
        {
        if (tsfsLoad (sysBootParams.bootFile, pEntry) != OK)
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

    /* verify unsupported booting devices */

    if (strncmp (sysBootParams.bootDev, "sl", 2) == 0)
        {
    printf ("booting via slip is unsupported.\n");
        return (ERROR);
        }

    strncpy (bootDev, sysBootParams.bootDev, sizeof (bootDev));

    /* attach and configure boot interface */

    pBootAddr = sysBootParams.ead;

#ifdef INCLUDE_SM_NET
    if ((strncmp (sysBootParams.bootDev, "bp", 2) == 0) ||
    (strncmp (sysBootParams.bootDev, "sm", 2) == 0))
        {
        backplaneBoot = TRUE;
        pBootAddr = sysBootParams.bad;
    }
#endif /* INCLUDE_SM_NET */

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

    /* Handle finite and infinite address assignment. */

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
#ifdef INCLUDE_DHCPC
    /* Set the DHCP lease information, if needed. */

    if (sysFlags & SYSFLG_AUTOCONFIG)
        {
        /* Save the requested lease length if entered by the user. */

        if (result == 1)
            dhcpLease.lease_duration = leaseLen;
        else

#ifdef INCLUDE_PCCARD

        if (!attached)
            {
            csClientLoad (CS_END_ENABLER);

            pCookie = muxTkCookieGet (bootDev, params.unitNum);
            if (pCookie != NULL)
                {
                /* The enabler has started the device - no muxDevStart
                 * necessary.
                 */

                /* Add our default address resolution functions. */

                muxAddrResFuncAdd (M2_ifType_ethernet_csmacd, 0x800,
                                   ipEtherResolvRtn);

                if (ipAttach (params.unitNum, bootDev) != OK)
                    {
                    printf ("Failed to attach TCP/IP to device %s",
                            muxDevName);
                    return (ERROR);
                    }

                printf ("Attached TCP/IP interface to %s%d.\n", bootDev,
                        params.unitNum);

                attached = TRUE;
                }
            }
#endif /* INCLUDE_PCCARD */
            {
            /*
             * The lease length is either not present or left over from
             * an earlier lease. Use the default value.
             */

            dhcpLease.lease_duration = DHCPC_DEFAULT_LEASE;
            }
        dhcpLease.lease_origin = 0;
        }
#endif /* INCLUDE_DHCPC */

    /*
     * Initialize network stack. Once usrNetworkInit is completed,
     * the bootrom can start loading the vxWorks image via network.
     * Note that MUX is already initialized in usrRoot().
     */

    if (usrNetworkInit() == ERROR)
    return (ERROR);

    if (netmask != 0)               /* reconstruct address with mask */
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

        if (netmask == 0)   /* Create empty netmask field. */
            sprintf (buf, "::%lx", leaseLen);
        else                /* Append requested lease length to netmask. */
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
            if (netmask == 0)       /* Create empty netmask field. */
                sprintf (buf, "::%lx:%lx", dhcpLease.lease_duration,
                         dhcpLease.lease_origin);
            else            /* Append lease timestamps to netmask. */
                sprintf (buf, ":%lx:%lx", dhcpLease.lease_duration,
                         dhcpLease.lease_origin);
            strcat (pBootAddr, buf);
            }
        }
#endif    /* INCLUDE_DHCPC */

    bootStructToString (BOOT_LINE_ADRS, &sysBootParams);

    /* load specified file */

    taskPriorityGet (0, &oldTaskPriority);
    taskPrioritySet (0, netTaskPriority + 1);

    if (netLoad (sysBootParams.had, sysBootParams.bootFile, sysBootParams.usr,
         sysBootParams.passwd, pEntry) != OK)
    {
    printf ("\nError loading file: errno = 0x%x.\n", errno);
    status = ERROR;
    }
    else
    status = OK;

    taskPrioritySet (0, oldTaskPriority);

#ifdef INCLUDE_VXWORKS_5_X_EQUIV_PPP
    if ((strncmp (sysBootParams.bootDev, "ppp", 3) == 0) &&
        ((pppBootHandle->status & PPP_LCP_OPENED) != 0))
        {
        if (ppp5xDisconnect (pppBootHandle) == ERROR ||
            pppBootHandle->syncSem == NULL)
            return (ERROR);

        if (semTake (pppBootHandle->syncSem,
            PPP_DISCONNECT_DELAY * sysClkRateGet()) == ERROR)
            {
            printf ("PPP: timeout: could not disconnect the link.\n");
            return (ERROR);
            }

        printf ("PPP: link disconnected OK\n");
        }
#endif /* INCLUDE_VXWORKS_5_X_EQUIV_PPP */

    return (status);
    }

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
    int errFd;      /* for receiving standard error messages from Unix */
    BOOL bootFtp = (passwd[0] != EOS);
    BOOL bootRsh = FALSE;

#ifdef INCLUDE_REMLIB
    char command [BOOT_FILE_LEN + BOOT_HOST_LEN];
#endif

    printf ("Loading... ");

#ifdef INCLUDE_TFTP_CLIENT
    if (sysFlags & SYSFLG_TFTP)     /* use tftp to get image */
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
#ifdef INCLUDE_FTP
        if (ftpXfer (hostName, usr, passwd, "", "RETR %s", "", fileName,
                 &errFd, &fd) == ERROR)
#endif
        return (ERROR);
        }
    else
        {
        bootRsh = TRUE;
#ifdef INCLUDE_REMLIB
        sprintf (command, "cat %s", fileName);

        fd = rcmd (hostName, RSHD, usr, usr, command, &errFd);
        if (fd == ERROR)
#endif
        return (ERROR);
        }
    }

    if (bootLoadModule (fd, pEntry) != OK)
    goto readErr;

#ifdef INCLUDE_TFTP_CLIENT
    /*
     * Successful TFTP transfers don't need any cleanup. The tftpXfer()
     * routine closes all file descriptors once the data has been
     * retrieved from the remote host.
     */

    if (sysFlags & SYSFLG_TFTP) /* used tftp to get image - just exit */
    {
    DEV_HDR *dhp = iosFdDevFind(fd);

    /*
     * Empty the data socket so that the entire file is received from
     * TFTP server - allows for graceful close on server side
     */

        while (dhp && ((iosFdDrvValue(fd, dhp->drvNum) != ERROR))
           && (read (fd, command, sizeof (command)) > 0));
        return (OK);
    }
#endif


    if (bootRsh == FALSE)
    {
#ifdef INCLUDE_FTP
    /* Empty the Data Socket before close. PC FTP server hangs otherwise */

    while ((read (fd, command, sizeof (command))) > 0);

        /* close the data socket before checking for a ftp reply (SPR #77169) */

        close (fd);

    if (bootFtp)
        (void) ftpCommand (errFd, "QUIT",0,0,0,0,0,0);
#endif
    }
    else
        close (fd);

    close (errFd);
    return (OK);

readErr:
    /* check standard error on Unix */

    if (bootRsh == FALSE)
    {
#ifdef INCLUDE_FTP
    /* Empty the Data Socket before close. PC FTP server hangs otherwise */

    while ((read (fd, command, sizeof (command))) > 0);

        /* close the data socket before checking for a ftp reply (SPR #77169) */

        close (fd);

    if (bootFtp)
        {
        (void) ftpReplyGet (errFd, FALSE); /* error message on std. err */
        (void) ftpCommand (errFd, "QUIT",0,0,0,0,0,0);
        }
#endif /* INCLUDE_FTP */
    }
    else
    {
    char buf [BOOT_FILE_LEN + BOOT_HOST_LEN];
    int errBytesRecv = fioRead (errFd, buf, sizeof (buf));

    if (errBytesRecv > 0)
        {
        /* print error message on standard error fd */

        buf [errBytesRecv] = EOS;
        printf ("\n%s:%s: %s\n", hostName, fileName, buf);
        }

        close (fd);

    }

    close (errFd);

    return (ERROR);
    }

#endif  /* INCLUDE_NETWORK */

#ifdef INCLUDE_FLASH_BOOT
LOCAL STATUS flashLoad(char *fileName, FUNCPTR *pEntry)
{
    int                 fd;
    int                 rv;
    int                 err;

    usrDosfsInit (DOSFS_DEFAULT_MAX_FILES, DOSFS_DEFAULT_CREATE_OPTIONS);

    flashFsLibInit();                   /* Mounts flsh: */

    do {
        /*
         * Load the boot file.
         * Support loading deflated files.
         */

        if ((fd = open(fileName, O_RDONLY, 0)) == ERROR) {
            printErr("Cannot open \"%s\".\n", fileName);
            return ERROR;
        }

        printf("Loading %s ... ", fileName);
        rv = bootLoadModule(fd, pEntry);
        if (rv != OK) {
            err = errnoGet();
            err = err & 0x7fff; /* mask module code. */
        }
        close(fd);
    } while (rv != OK && err == EINTR); /* If interrupted, try again */

    printf("\n");

    return rv;
}
#endif  /* INCLUDE_FLASH_BOOT */

#if     (defined (INCLUDE_SCSI_BOOT) || defined (INCLUDE_FD) || \
     defined (INCLUDE_IDE) || defined (INCLUDE_ATA) || \
     defined (INCLUDE_TFFS))

#define SPIN_UP_TIMEOUT 45  /* max # of seconds to wait for spinup */

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
    FAST char *fullFileName,    /* full file name being parsed */
    FAST char *devName      /* result device name */
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
    char          fsmName[16];

#if 0
    CBIO_DEV_ID pCbio;
#endif


    if (!scsiInitialized)       /* skip if this is a retry */
        {
        if (sysScsiInit () == ERROR)
            {
            printErr ("Could not initialize SCSI.\n");
            return (ERROR);
            }
        scsiInitialized = TRUE;
        }

    taskDelay (sysClkRateGet ());   /* delay 1 second after reset */

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
                                               0xffff, 512)) == NULL)
        {
        printErr ("scsiPhysDevCreate failed.\n");
        return (ERROR);
        }

    /* issue two TEST UNIT READY commands to clear reset exception */

    scsiTestUnitRdy (pScsiPhysBootDev);
    scsiTestUnitRdy (pScsiPhysBootDev);

    /*
     * issue a TEST UNIT READY every second for SPIN_UP_TIMEOUT seconds,
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
        char modeData [4];  /* array for floppy MODE SELECT data */

        /*
         * zero modeData array, then set byte 1 to "medium code" (0x1b).
         * NOTE: MODE SELECT data is highly device-specific.  If your device
         * requires configuration via MODE SELECT, please consult the device's
         * Programmer's Reference for the relevant data format.
         */

        bzero (modeData, sizeof (modeData));
        modeData [1] = 0x1b;

        /* issue a MODE SELECT cmd to correctly configure floppy controller */

        scsiModeSelect (pScsiPhysBootDev, 1, 0, modeData, sizeof (modeData));

        /*
         * delete and re-create the SCSI_PHYS_DEV so that INQUIRY will return
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

    /*
     * -----------------------------------------------------------------------
     *
     *  END of OMTI3500 configuration
     *
     * -----------------------------------------------------------------------
     */

    /* create a block device spanning entire disk (non-destructive!) */

    if ((pScsiBlkBootDev = scsiBlkDevCreate (pScsiPhysBootDev, 0, 0)) == NULL)
        {
        printErr ("scsiLoad: scsiBlkDevCreate failed.\n");
        return (ERROR);
        }


    sprintf (fsmName, "/scsi%d:0", bootDevLUN);
    fsmNameInstall (fsmName, bootDir);

    /* split off boot device from boot file */

    devSplit (fileName, bootDir);

    xbdBlkDevCreate (pScsiBlkBootDev, bootDir);

    /* now support booting from partitions on SCSI devices */

#if 0
    pCbio = dpartDevCreate ((CBIO_DEV_ID) pScsiBlkBootDev,
                            NUM_PARTITIONS_DISK_BOOT,
                            usrFdiskPartRead);

    if (NULL == pCbio)
        {
        printErr ("scsiLoad: dpartDevCreate returned NULL.\n");
        return (ERROR);
        }

    /* initialize the boot block device as a dosFs device named <bootDir> */

    if (ERROR == dosFsDevCreate (bootDir,
                                 dpartPartGet(pCbio,PARTITION_DISK_BOOT),
                                 20, NONE))
        {
        printErr ("scsiLoad: dosFsDevCreate returned ERROR.\n");
        return (ERROR);
        }
#endif

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
    close (fd);
    return (ERROR);
    }

#endif  /* INCLUDE_SCSI_BOOT */

#ifdef  INCLUDE_FD

#include <usrFd.c>

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

    /*
     * Initialize DosFs.  When booting from a floppy, it is assumed that it is
     * formatted with DosFs.  Hence the unconditional initialization of the
     * DosFs file system libraries.
     */

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
    close (fd);
    return (ERROR);
    }
#endif  /* INCLUDE_FD */

#ifdef  INCLUDE_IDE

#define IDE_MEM_DOSFS   0x200000
#include <usrIde.c>

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

    close (fd);
    return (ERROR);
    }

#endif  /* INCLUDE_IDE */

#ifdef  INCLUDE_ATA

#define ATA_MEM_DOSFS   0x200000
#include <usrAta.c>

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
    IMPORT ATA_RESOURCE   ataResources[];
    ATA_RESOURCE *        pAtaResource  = &ataResources[ctrl];
    int                   fd;
    char                  tmp[BOOT_FILE_LEN];

#ifdef INCLUDE_PCCARD
    if (pAtaResource->ctrlType == IDE_LOCAL)
        {
#endif /* INCLUDE_PCCARD */
    if (ataDrv (ctrl, pAtaResource->drives, pAtaResource->intVector,
        pAtaResource->intLevel, pAtaResource->configType,
        pAtaResource->semTimeout, pAtaResource->wdgTimeout) == ERROR)
        {
        printErr ("Could not initialize.\n");
        return (ERROR);
        }
#ifdef INCLUDE_PCCARD
        }
    else
        csClientLoad (CS_ATA_ENABLER);
#endif /* INCLUDE_PCCARD */

    printf ("Attaching to ATA disk device... ");

    /*
     * Initialize DosFs.  When booting from an ATA device, it is assumed that
     * it is formatted with DosFs.  Hence the unconditional initialization of
     * the DosFs file system libraries.
     */

    devSplit (fileName, tmp);

    if (usrAtaConfig (ctrl, drive, tmp) == ERROR)
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
    close (fd);
    return (ERROR);
    }

#endif  /* INCLUDE_ATA */

#ifdef  INCLUDE_PCMCIA

#define PCMCIA_MEM_DOSFS    0x200000
#include <usrPcmcia.c>

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

    /*
     * Initialize DosFs.  When booting from a PCMCIA device, it is assumed that
     * it is formatted with DosFs.  Hence the unconditional initialization of
     * the DosFs file system libraries.
     */

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
    close (fd);
    return (ERROR);
    }

#endif  /* INCLUDE_PCMCIA */

#ifdef  INCLUDE_TFFS

#define TFFS_MEM_DOSFS  0x200000
#include <usrTffs.c>
#include "sysTffs.c"      /* the BSP stub file, in the BSP directory */

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
    int     drive,      /* TFFS drive number (0 - (noOfDrives-1)) */
    int     removable,      /* 0 - nonremovable flash media */
    char    * fileName,     /* file name to download */
    FUNCPTR * pEntry
    )
    {
    int fd;

    if (tffsDrv () != OK)
        {
        printErr ("Could not initialize.\n");
        return (ERROR);
        }

    printf ("Attaching to TFFS... ");

    if (usrTffsConfig (drive, removable, fileName) == ERROR)
        {
        printErr ("usrTffsConfig failed.\n");
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
        goto tffsLoadErr;

    close (fd);
    return (OK);

tffsLoadErr:
    close (fd);
    return (ERROR);
    }

#endif  /* INCLUDE_TFFS */

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
    char    * fileName,     /* file name to download */
    FUNCPTR * pEntry
    )
    {
    int fd;
    WDB_EVT_NODE rebootEventNode;
    char corefile [PATH_MAX + 1];

    /* add a leading slash if the filename is a relative path */

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
#endif  /* INCLUDE_TSFS_BOOT_VIO_CONSOLE */

    /* open the core file via tsfs */

    printErr ("\nLoading %s...\n", corefile);

    if ((fd = open (corefile, O_RDONLY, 0)) == ERROR)
        {
        printErr ("\nCannot open \"%s\".\n", corefile);
        return (ERROR);
        }

    /* load the core file */

    if (bootLoadModule (fd, pEntry) != OK)
        goto tsfsLoadErr;

    close (fd);

#if (WDB_COMM_TYPE != WDB_COMM_SERIAL)

    /* Notify the host of the target reboot */

    wdbEventNodeInit (&rebootEventNode, wdbRebootEventGet, NULL, NULL);
    wdbEventPost (&rebootEventNode);

    /* let some time to WDB to post the event */

    taskDelay (sysClkRateGet() / 10);
#endif  /* WDB_COMM_TYPE != WDB_COMM_SERIAL */

    return (OK);

tsfsLoadErr:

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
    void *      pNode,
    WDB_EVT_DATA *  pEvtData
    )
    {
    taskSuspend (0);
    }

#endif  /* INCLUDE_TSFS_BOOT */

#ifdef  INCLUDE_NETWORK

/******************************************************************************
*
* netifAdrsPrint - print MAC address of a network interface
*/

LOCAL void netifAdrsPrint
    (
    char *ifname        /* interface name */
    )
    {
    IMPORT struct ifnet *ifunit ();
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

    taskDelay (sysClkRateGet ());   /* give the network a moment to close */

#ifdef INCLUDE_END
    /* Stop all ENDs to restore to known state for interrupts and DMA */

    (void) muxDevStopAll (0);
#endif  /* INCLUDE_END */


#if (CPU_FAMILY == PPC)
    cacheTextUpdate ((void *) (LOCAL_MEM_LOCAL_ADRS),   /* cache coherency */
             (size_t) (sysMemTop() - LOCAL_MEM_LOCAL_ADRS));
#else
#ifdef  INCLUDE_CACHE_SUPPORT
    cacheClear (DATA_CACHE, NULL, ENTIRE_CACHE);    /* push cache to mem */
#endif  /* INCLUDE_CACHE_SUPPORT */
#endif  /* (CPU_FAMILY == PPC) */

#if (CPU_FAMILY == I80X86)
    sysClkDisable ();           /* disable the system clock interrupt */
    sysIntLock ();          /* lock the used/owned interrupts */
#   if defined (SYMMETRIC_IO_MODE) || defined (VIRTUAL_WIRE_MODE)
    {
    extern void loApicEnable ();
    loApicEnable (FALSE);       /* disable the LOAPIC */
    }
#       if defined (SYMMETRIC_IO_MODE)
    {
    extern BOOL sysBp;          /* TRUE for BP, FALSE for AP */
    extern void ioApicEnable ();
    if (sysBp)
        ioApicEnable (FALSE);       /* disable the IO APIC */
    }
#       endif /* (SYMMETRIC_IO_MODE) */
#   endif /* (SYMMETRIC_IO_MODE) || (VIRTUAL_WIRE_MODE) */
#endif  /* (CPU_FAMILY == I80X86) */

    /* Lock interrupts before jumping out of boot image. The interrupts
     * enabled during system initialization in sysHwInit()
     */

    intLock();

    (entry) (sysStartType);     /* go to entry point - never to return */
    }

/*******************************************************************************
*
* m - modify memory
*
* This routine prompts the user for modifications to memory, starting at the
* specified address.  It prints each address, and the current contents of
* that address, in turn.  The user can respond in one of several ways:
*
*   RETURN   - No change to that address, but continue
*          prompting at next address.
*   <number> - Set the contents to <number>.
*   . (dot)  - No change to that address, and quit.
*   <EOF>    - No change to that address, and quit.
*
* All numbers entered and displayed are in hexadecimal.
* Memory is treated as 16-bit words.
*/

LOCAL void m
    (
    char *adrs      /* address to change */
    )
    {
    char line [MAX_LINE + 1];   /* leave room for EOS */
    char *pLine;        /* ptr to current position in line */
    int value;          /* value found in line */
    char excess;

    /* round down to word boundary */

    for (adrs = (char *) ((int) adrs & 0xfffffffe); /* start on even addr */
         ;                      /* FOREVER */
     adrs = (char *) (((short *) adrs) + 1))    /* bump as short ptr */
    {
    /* prompt for substitution */

    printf ("%06x:  %04x-", (int) adrs, (*(short *)adrs) & 0x0000ffff);

    /* get substitution value:
     *   skip empty lines (CR only);
     *   quit on end of file or invalid input;
     *   otherwise put specified value at address */

    if (fioRdString (STD_IN, line, MAX_LINE) == EOF)
        break;

    line [MAX_LINE] = EOS;  /* make sure input line has EOS */

    /* skip leading spaces*/

    for (pLine = line; isspace ((UINT) * pLine); ++pLine)
        ;

    if (*pLine == EOS)          /* skip field if just CR */
        continue;

    if (sscanf (pLine, "%x%1s", &value, &excess) != 1)
        break;              /* quit if not number */

    * (short *) adrs = value;       /* assign new value */
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
    FAST char *adrs,    /* address to display */
    int        nwords   /* number of words to print. */
    )           /* If 0, print 64 or last specified. */
    {
    static char *last_adrs;
    static int dNbytes = 128;
    char ascii [17];
    FAST int nbytes;
    FAST int byte;

    ascii [16] = EOS;           /* put an EOS on the string */

    nbytes = 2 * nwords;

    if (nbytes == 0)
    nbytes = dNbytes;   /* no count specified: use current byte count */
    else
    dNbytes = nbytes;   /* change current byte count */

    if (adrs == 0)
    adrs = last_adrs;   /* no address specified: use last address */

    adrs = (char *) ((int) adrs & ~1);  /* round adrs down to word boundary */


    /* print leading spaces on first line */

    bfill ((char *) ascii, 16, '.');

    printf ("%06x:  ", (int) adrs & ~0xf);

    for (byte = 0; byte < ((int) adrs & 0xf); byte++)
    {
    printf ("  ");
    if (byte & 1)
        printf (" ");   /* space between words */
    if (byte == 7)
        printf (" ");   /* extra space between words 3 and 4 */

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

        bfill ((char *) ascii, 16, '.');    /* clear out ascii buffer */
        byte = 0;               /* reset word count */
        }


#if  _BYTE_ORDER == _BIG_ENDIAN
    printf ("%02x", *adrs & 0x000000ff);
#else  /* _BYTE_ORDER == _LITTLE_ENDIAN  */
    /* swap odd and even bytes */
    if ( (long)adrs & 1)
        {
        printf ("%02x", *((char *)((long)adrs & 0xfffffffe)) & 0x000000ff);
        }
    else
        {
        printf ("%02x", *((char *)((long)adrs + 1)) & 0x000000ff);
        }
#endif /* _BYTE_ORDER == _BIG_ENDIAN  */

    if (byte & 1)
        printf (" ");   /* space between words */
    if (byte == 7)
        printf (" ");   /* extra space between words 3 and 4 */

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
        printf (" ");   /* space between words */
    if (byte == 7)
        printf (" ");   /* extra space between words 3 and 4 */

    ascii [byte] = ' ';
    }

    printf ("  *%16s*\n", ascii);   /* print out ascii format values */

    last_adrs = adrs;
    }
/*******************************************************************************
*
* bootExcHandler - bootrom exception handling routine
*/

LOCAL void bootExcHandler
    (
    int tid     /* task ID */
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
    FAST char **strptr  /* pointer to pointer to string */
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
    FAST char **ppString,   /* ptr to ptr to current position in line */
    int *   pValue,     /* ptr where to return value */
    BOOL    defaultHex, /* TRUE = arg is hex (even w/o 0x) */
    BOOL    optional    /* TRUE = ok if end of line */
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

    if ((sysNvRamGet (BOOT_LINE_ADRS, BOOT_LINE_SIZE, 0) == ERROR) ||
        (*BOOT_LINE_ADRS == EOS) || (*BOOT_LINE_ADRS == (char) -1))
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
    char *   bootString,
    BOOT_PARAMS *pParams
    )
    {
    FAST char * pS;

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

#ifdef ETHERNET_MAC_HANDLER
/***********************************************************
*
* macAddressShow - Displays the device and MAC address in
*                     string form.
*
*  This routine returns a pointer to a string of the form:
*
*   id device unit address
*
*   where:
*    id is an integer 1..MAX_MAC_ADRS
*    device is a string (e.g. motfcc)
*    unit is unit number
*    address is a six byte hex mac address
*   example:
*    "01  motfcc0 00:A0:1E:00:10:0A
*
*  The function takes index as an argument  where index is
*  range 00..MAX_MAC_ADRS-1
*
*  If the passed index is out of range, the return value
*  points to an error message.
*
* RETURNS: pointer to string
*/

LOCAL char *macAddressShow
    (
    int index
    )
    {
    int devNameIndex;
    int unit;
    UINT8 macAddr[6] = { 0xff,0xff,0xff,0xff,0xff,0xff };

    /* range check passed index */

    if ((index < 0) || (index >= MAX_MAC_ADRS))
        {
        return ((char *) macErrStr);  /* if error, return error string */
	}

    /* convert MAC index to device index and unit number */

    devNameIndex = sysMacIndex2Dev (index);
    unit = sysMacIndex2Unit(index);

    /* get a copy of the current device address */
    /* macAddr initialized so returns all f's if ERROR */
    sysNetMacNVRamAddrGet ((char *)sysNetDevName[devNameIndex], unit,
                           macAddr, MAC_ADRS_LEN);
			
    /* convert to string */
    			
    sprintf (macShowStr[index],
              "%2d  %6s%d  %02x:%02x:%02x:%02x:%02x:%02x\n",
              index+1, sysNetDevName[devNameIndex],
	      unit,
	      macAddr[0],
	      macAddr[1],
	      macAddr[2],
	      macAddr[3],
	      macAddr[4],
	      macAddr[5]
	    );
    return (macShowStr[index]);
    }


/***********************************************************
*
*  ifGet - Get network interface name
*
*  This routine parses the remaining command line (after the command
*  character has been removed) and attempts to
*  identify the requested device and unit using that string.  If the
*  command line does not provide the device name, a dialog is
*  initiated to identify the device.
*
*
* RETURNS: IFG_MAC_SUPPLIED - if remaining boot line identifies new MAC address.
*          IFG_OPR_QUIT     - if the operator elects to quit the dialog
*          IFG_DEV_SUPPLIED - if a device is selected, but no MAC address
*
*/
			
LOCAL int ifGet
    (
    char **   ifName, 	/* interface name */
    int *     unit,     /* interface unit */
    char *    pNum,     /* Command line */
    uchar_t * curArr    /* current MAC address */
    )
    {
    char line [MAX_LINE + 1];
    uchar_t passedMac [MAX_ADR_SIZE];
    int numMacDigiRead = 0;
    char *pLine;		/* ptr to current position in line */
    char excess;
    int nameLen = 0;
    int index;
    int macAddressPassed = 0;
    int macDevPassed = 0;


    pLine = pNum;

    SKIPSPACE(pLine);  /* advance past any leading whitespaces */
    *ifName = pLine;

    /* determine if the bootline passed includes ifName, unit, and MAC */

     while (NOTEOS(pLine) &&  isalpha((int)*pLine))
        {
        nameLen++;   /* count device length */
	pLine++;
	}

    /* if device name found */

    if (nameLen != 0)
        {
        for (index = 0; index < MAX_MAC_DEVS; index++)
	    {
	
	    /* if indexed device matches passed device name */
	
	    if (strncmp (sysNetDevName[sysMacIndex2Dev(index)], *ifName,
	        strlen((char *)sysNetDevName[sysMacIndex2Dev(index)])) == 0)
	        {
		
		/* save return dev name */
		
                *ifName = (char *)sysNetDevName[sysMacIndex2Dev(index)];
		macDevPassed = 1;               /* set flag */
		
		/* was unit number supplied? */
                if (isdigit((int)*pLine))
		    {
		    *unit = strtol(pLine, &pLine, 0); /* extract unit number */
		    }
                else
		    {
		    *unit = 0; /* if not supplied, assume unit 0 */
		    }
		
		/* look for MAC address */
		
                while (NOTEOS(pLine))
		    {
                    SKIPSPACE(pLine);  /* advance to next field */
	            if (!NOTEOS(pLine))
			{
			break; /* end of string */
			}
		    if (isxdigit((int)*pLine))
			{
			passedMac[numMacDigiRead++] =
			    (uchar_t) strtol(pLine, &pLine, 16);
			}
	            else /* some illegal character read */
			{
			break;
			}
		    if (*pLine == ':')
			    pLine++;
			
		    }
		
		/* if we were able to read the MAC address */
		
#if _BYTE_ORDER == _BIG_ENDIAN
                if (numMacDigiRead == 6)  /* if full MAC address */
                    {
		    macAddressPassed = 1;
                    for (index = 0; index < 6; index++)
	                {
	                curArr[index] = passedMac[5-index];
	                }
	            }
                else if (numMacDigiRead == 3) /* if half MAC address */
                    {
		    macAddressPassed = 1;
	            for (index = 0; index < 3; index++)  /* 3 bytes only */
	                {
	                curArr[index] = passedMac[2-index];
	                }
	            curArr[0] = ((ENET_DEFAULT & 0x0000ff00) >> 8);
	            curArr[1] = ((ENET_DEFAULT & 0x00ff0000) >> 16);
	            curArr[2] = ((ENET_DEFAULT & 0xff000000) >> 24);
	            }
#else  /* _BYTE_ORDER == _LITTLE_ENDIAN  */
                if (numMacDigiRead == 6) /* if full MAC address */
                    {
		    macAddressPassed = 1;
	            for (index = 0; index < 6; index++)
	                {
	                curArr[index] = passedMac[index];
	                }
	            }
                else if (numMacDigiRead == 3) /* if half MAC address */
                    {
		    macAddressPassed = 1;
	            for (index = 3; index < 6; index++) /* 3 bytes only */
	                {
	                curArr[index] = passedMac[index];
	                }
	            curArr[0] = ((ENET_DEFAULT & 0x00ff0000) >> 16);
	            curArr[1] = ((ENET_DEFAULT & 0x0000ff00) >> 8);
	            curArr[2] = (ENET_DEFAULT & 0x000000ff);
	            }
#endif
		break; /* matching device found.  Look no further */
		} /* end if indexed device matches passed device name */
	
	    }
	
	} /* end if no device name found */

    if (macDevPassed == 0)  /* if the command line lacks a device name */
        {
	
	/* print known devices */
	
        printf ("Set device MAC address\n\n");
        printf ("  0  quit\n");
        for (index = 0; index < MAX_MAC_ADRS; index++)
            {
            printf (" %s",macAddressShow(index));
            }


        index = -1; /* initialize to invalid index */

	/* query for new network device (1..n) */

	while ((index < 0) || ( index >= MAX_MAC_ADRS))
            {
            printf ("  Selection : ");
	    if (fioRdString (STD_IN, line, MAX_LINE) == EOF) /* end of line */
		{
		continue;
		}

	    line [MAX_LINE] = EOS;	/* make sure input line has EOS */

            /* skip leading spaces*/

	    for (pLine = line; isspace ((int) *pLine); ++pLine)	
		;

	    if (*pLine == EOS)		/* if field is just CR */
		{
		continue;
		}

            /* if not number */

	    if (sscanf (pLine, "%d%1s", &index, &excess) != 1)
		{
        	printf("\ninvalid selection\n");
		index  = -1;
                return (IFG_OPR_QUIT); /* quit */
		}

            if (index == 0)
	        {
		index  = -1;
                return (IFG_OPR_QUIT); /* quit */
                }

  	        index--;   /* set to zero based index */

           }  /* end query for new network device (1..n) */


        /* return device string and unit number */

        *ifName = (char *)sysNetDevName[sysMacIndex2Dev(index)];
	*unit = sysMacIndex2Unit(index);
	}
	
    if (macAddressPassed)
        return (IFG_MAC_SUPPLIED);
    return (IFG_DEV_SUPPLIED);

    }


/***********************************************************
*
* dynamicMacAddrGen - Generate a dynamic MAC address
*
*  This routine generates a 3-byte network interface MAC address.
*  This should be used as the last three bytes of the device address.
*  It attempts to make the address unique, but is not
*  guaranteed to do so.
*
*  This routine should only be called if both sysNetMacAddrGet()
*  and sysNVRamNetMacAddrGet() return ERROR, indicating that
*  there is no other suitable mechanism for generating a
*  MAC address.
*
* RETURNS: 1 for an address which will be reproducible across
* multiple runs, or 0 if the address is likely to change between
* vxWorks and bootrom or between two invocations within either vxWorks
* or bootrom.
*/

LOCAL int dynamicMacAddrGen
    (
    UINT8 *	ifName,
    int		ifUnit,
    UINT8 *	ifMacAddr
    )
    {
    BOOT_PARAMS         params;
    int			i = 0;
    UINT32		sum = 0;
    char 		adrs [BOOT_TARGET_ADDR_LEN];


    /* hash sum of specified interface name */

    while ( EOS != ifName[i] )
        sum += ifName[i++];

    /* check boot command */

    if (usrBootLineCrack (BOOT_LINE_ADRS, &params) == OK)
        {
	if ( ( params.ead[0] != EOS ) && ( params.ead[1] != EOS ) &&
	     ( params.ead[2] != EOS ) && ( params.ead[3] != EOS ) &&
	     ( params.ead[4] != EOS ) && ( params.ead[5] != EOS ) )
	    {
	    /* OK, use target address */
	    *ifMacAddr++ = params.ead[3] + sum + ifUnit;
	    *ifMacAddr++ = params.ead[4];
	    *ifMacAddr++ = params.ead[5];
	    return(1);
	    }

	adrs[0] = params.bad[0] + params.had[0] + params.gad[0];
	adrs[1] = params.bad[1] + params.had[1] + params.gad[1];
	adrs[2] = params.bad[2] + params.had[2] + params.gad[2];
	adrs[3] = params.bad[3] + params.had[3] + params.gad[3];
	adrs[4] = params.bad[4] + params.had[4] + params.gad[4];
	adrs[5] = params.bad[5] + params.had[5] + params.gad[5];

	if ( ( adrs[0] != EOS ) && ( adrs[1] != EOS ) &&
	     ( adrs[2] != EOS ) && ( adrs[3] != EOS ) &&
	     ( adrs[4] != EOS ) && ( adrs[5] != EOS ) )
	    {
	
	    /* use host + gateway + backplane */
	
	    *ifMacAddr++ = adrs[0] + adrs[3] + sum + ifUnit;
	    *ifMacAddr++ = adrs[1] + adrs[4];
	    *ifMacAddr++ = adrs[2] + adrs[5];
	    return(1);
	    }

	/* hash some strings together */

	for ( i = 0 ; i < BOOT_DEV_LEN ; i++ )
	    sum += params.bootDev[i];
	for ( i = 0 ; i < BOOT_HOST_LEN ; i++ )
	    sum += params.hostName[i];
	for ( i = 0 ; i < BOOT_HOST_LEN ; i++ )
	    sum += params.targetName[i];
	for ( i = 0 ; i < BOOT_FILE_LEN ; i++ )
	    sum += params.bootFile[i];
	for ( i = 0 ; i < BOOT_FILE_LEN ; i++ )
	    sum += params.startupScript[i];
	for ( i = 0 ; i < BOOT_USR_LEN ; i++ )
	    sum += params.usr[i];
	for ( i = 0 ; i < BOOT_PASSWORD_LEN ; i++ )
	    sum += params.passwd[i];
	for ( i = 0 ; i < BOOT_OTHER_LEN ; i++ )
	    sum += params.other[i];

	adrs[0] = 0;
	adrs[1] = 4;
	adrs[2] = ( sum >> 24 ) & 0x00ff;
	adrs[3] = ( sum >> 16 ) & 0x00ff;
	adrs[4] = ( sum >>  8 ) & 0x00ff;
	adrs[5] = ( sum >>  0 ) & 0x00ff;

	*ifMacAddr++ = adrs[0] + adrs[3] + sum + ifUnit;
	*ifMacAddr++ = adrs[1] + adrs[4];
	*ifMacAddr++ = adrs[2] + adrs[5];
	return(1);
	}

    /*
     * boot command not available, generate address
     * which is as close to random as we can make it.
     * If we get here, there isn't any reasonable way to
     * make a highly unique number, but we'll do the
     * best we reasonably can.
     *
     * NOTE: This is guaranteed to cause problems when
     * booting, unless rommable vxWorks image is used.
     * If we get here, the vxWorks and bootrom images
     * will use different MAC addresses.  When the interface
     * changes, vxWorks will correctly send a gratuitous
     * ARP packet.  However, some switches will ignore
     * the gratuitous ARP packet which came so quickly
     * after the vxWorks image download.  In this case,
     * the target will be able to send traffic out, but
     * until it has done so, no other computer will be able
     * to contact it.  To resolve this problem, the target
     * should initiate outbound IP traffic after a suitable
     * delay.  This can be a simple ping, or it can be
     * a symbol table download, or whatever.
     */

    sum += tickGet();		/* time component */
    sum += (int)&params;	/* stack addr: runtime mem use component */
    sum += (int)ifMacAddr;	/* data addr: compile-time component */

    adrs[0] = 0;
    adrs[1] = 4;
    adrs[2] = ( sum >> 24 ) & 0x00ff;
    adrs[3] = ( sum >> 16 ) & 0x00ff;
    adrs[4] = ( sum >>  8 ) & 0x00ff;
    adrs[5] = ( sum >>  0 ) & 0x00ff;

    *ifMacAddr++ = adrs[0] + adrs[3] + ifUnit;
    *ifMacAddr++ = adrs[1] + adrs[4];
    *ifMacAddr++ = adrs[2] + adrs[5];
    return(0);
    }

/*******************************************************************************
*
* sEnet - set the device ethernet address
*  This routine checks the passed boot line to see if it can set the
*  MAC address of the requested device.  The command line is parsed to
*  extract device name, unit name, and MAC address.  All three items
*  are optional.  Example command lines follow:
*
*    M
*    M motfcc
*    M motfcc1
*    M motfcc1 00:01:00
*    M motfcc0 00:A0:1E:00:01:00
*
*    The first example will invoke a dialog with the user to display
*    current devices and prompt for which device address to change as below.
*      0  quit
*      1  motfcc0 00:A0:1E:00:01:1A
*      2  motscc0 00:A0:1E:00:01:1B
*    Selection :
*
*    Once a selection is made, the dialog will contine by quiting the dialog
*    if option zero is slected, or prompting for the MAC address for the
*    selected device as show below.
*
*    00- 00
*    A0- A0
*    1E- 1E
*    00-
*
*    The first three bytes of the MAC address are fixed and can't be changed
*    using this method.  The remaining three subsequent values will be
*    displayed for modification.  If no entry is made, the previous value
*    remains unchanged.
*
*    If the MAC address read from configuration memory does not include the
*    three byte vendor code, it is assumed that the address read is invalid.
*    In this case, a pseudo-random address is generated before the dialog
*    commences.  The address generation algorithm uses a hashing method
*    to generate the address based on the sum of various fixed parameters
*    such as the contents of the boot configuration.  This method generates
*    an address that is reproducible, given the same build and boot
*    parameters.  If the boot record is not valid, a random address is
*    generated that is not reproducible.
*
*    The second example shows a method used to avoid the first part of
*    the user dialog.  In this case, the device is specified on the
*    command line.  Since the unit number is not included, unit zero is
*    assumed.  The third example shows the command format that should be
*    used to select other than unit zero.
*
*    The fourth example shows a method used that avoids the user dialog.  In
*    this example, the first three bytes of the MAC address will be the WR
*    vendor code.  The remaining three bytes will be as specified on the
*    line.  This method requires that exactly three bytes of address be
*    supplied.
*
*    The last example shows a method that can be used to set all six bytes
*    of the MAC address, including the vendor code.  If other than the
*    default WR vendor code, care must be taken when using the command
*    shown in the previous examples.  These other forms assume that
*    other vendor codes are not valid and will overwrite.
*
*
* RETURNS: void
*
* NOMANUAL
*/

void sEnet
    (
    char * 	pNum   /* Boot line, including unit number. */
    )
    {
    uchar_t curArr [MAX_ADR_SIZE];	/* array to hold current Enet addr */
    int unitNum;
    char *ifName ="\n";
    int ifGetStatus;

    /* identify the network device */

    ifGetStatus = ifGet (&ifName, &unitNum, pNum, curArr);

    if (ifGetStatus == IFG_OPR_QUIT)
        return; /* operator abort */
	
    if (ifGetStatus == IFG_MAC_SUPPLIED)
        {
        if (sysNetMacAddrSet (ifName, unitNum, curArr, MAC_ADRS_LEN) != OK)
            {
	    printf ("** error setting address for %s%d\n", ifName, unitNum);
	    return;
	    }
        }
    else
	{
	mEnet (pNum, unitNum, ifName);
	}	
    }


/*******************************************************************************
*
* mEnet - modify the last three bytes of the device ethernet address.
*
*  This routine is called if the command line lacks a network address
*  for the selected device.  It initiates a dialog with the user to
*  obtain the address.
*
* RETURNS: void
*
* NOMANUAL
*/
void mEnet
    (
    char * 	pNum,    /* Boot line, including unit number. */
    int         unitNum, /* array to hold current Enet addr */
    char       *ifName
    )
    {
    uchar_t byte [MAX_ADR_SIZE];	/* array to hold new Enet addr */
    uchar_t curArr [MAX_ADR_SIZE];	/* array to hold current Enet addr */
    char line [MAX_LINE + 1];
    char *pEnet;                /* dummy variable */
    int *pOffset = NULL;        /* dummy variable */
    char *pLine;		/* ptr to current position in line */
    int value;			/* value found in line */
    STATUS status;
    char excess;
    int ix;

    /* make sure device is valid */

    if (sysMacOffsetGet (ifName, unitNum, &pEnet, pOffset) != OK)
        {
	printf ("** invalid device\n");
	return;
	}

    /* read current value */

    status = sysNetMacNVRamAddrGet (ifName, unitNum, curArr, MAC_ADRS_LEN);
    /* if at first we don't succeed, try second method */

    if (status !=OK)
        sysNetMacAddrGet (ifName, unitNum, curArr, MAC_ADRS_LEN);
	
    /* if current MAC address is invalid */

#if _BYTE_ORDER == _BIG_ENDIAN
    if ((status != OK) ||
        (curArr[0] != ((ENET_DEFAULT & 0x0000ff00) >> 8)) ||
        (curArr[1] != ((ENET_DEFAULT & 0x00ff0000) >> 16)) ||
        (curArr[2] != ((ENET_DEFAULT & 0xff000000) >> 24)))
	{
	curArr[0] = ((ENET_DEFAULT & 0x0000ff00) >> 8);
	curArr[1] = ((ENET_DEFAULT & 0x00ff0000) >> 16);
	curArr[2] = ((ENET_DEFAULT & 0xff000000) >> 24);
	
#else  /* _BYTE_ORDER == _LITTLE_ENDIAN  */
    if ((status != OK) ||
        (curArr[0] != ((ENET_DEFAULT & 0x00ff0000) >> 16)) ||
        (curArr[1] != ((ENET_DEFAULT & 0x0000ff00) >> 8)) ||
        (curArr[2] != ((ENET_DEFAULT & 0x000000ff) )))
	{
	curArr[0] = ((ENET_DEFAULT & 0x00ff0000) >> 16);
	curArr[1] = ((ENET_DEFAULT & 0x0000ff00) >> 8);
	curArr[2] = (ENET_DEFAULT & 0x000000ff);
#endif /* _BYTE_ORDER == _BIG_ENDIAN */
        
	ix = dynamicMacAddrGen(ifName, unitNum, &curArr[3]);

	printf ("Mac address for %s%d invalid. ", ifName, unitNum);
	if  (ix == 1)
	    printf ("Using a reproducible random address.\n");
	else
	    printf ("Using a non-reproducible random address.\n");
	}

    printf ("%s%d current Ethernet Address is: ", ifName, unitNum);

    printf ("%02x:%02x:%02x:%02x:%02x:%02x\n", curArr[0],
	    curArr[1], curArr[2], curArr[3],
	    curArr[4], curArr[5]);
    byte[5] = curArr[5];
    byte[4] = curArr[4];
    byte[3] = curArr[3];
    byte[2] = curArr[2];
    byte[1] = curArr[1];
    byte[0] = curArr[0];

    printf ("Modify only the last 3 bytes (board unique portion) of Ethernet Address.\n"
            "The first 3 bytes are fixed at manufacturer's default address block.\n");

    for (ix = 0; ix < 3; ix++)
        printf ("%02x- %02x\n", byte[ix], byte[ix]);


    /* start on fourth byte of enet addr */
    for (ix = 3; ix < 6; ix++)	
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

    if (sysNetMacAddrSet (ifName, unitNum, byte, MAC_ADRS_LEN) != OK)
        {
	printf ("** error setting address for %s%d\n", ifName, unitNum);
	return;
	}
    sysNetMacNVRamAddrGet (ifName, unitNum, byte, MAC_ADRS_LEN);

    printf ("New Ethernet Address is: ");
    for (ix = 0; ix < 5; ix++)
        printf ("%02x:", byte[ix]);
    printf ("%02x\n", byte[5]);
    }
#endif  /* ETHERNET_MAC_HANDLER */

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
    char *  pNum   /* Boot line, including unit number. */
    )
    {
    uchar_t byte [MAX_ADR_SIZE];    /* array to hold new Enet addr */
    uchar_t curArr [MAX_ADR_SIZE];  /* array to hold current Enet addr */
    char line [MAX_LINE + 1];
    char *pLine;        /* ptr to current position in line */
    int value;          /* value found in line */
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
    byte[0] = ((ENET_DEFAULT & 0x0000ff00) >> 8);
    byte[1] = ((ENET_DEFAULT & 0x00ff0000) >> 16);
    byte[2] = ((ENET_DEFAULT & 0xff000000) >> 24);
    byte[3] = curArr[2];
    byte[4] = curArr[1];
    byte[5] = curArr[0];
#else  /* _BYTE_ORDER == _LITTLE_ENDIAN  */
    printf ("%02x:%02x:%02x:%02x:%02x:%02x\n", curArr[0],
        curArr[1], curArr[2], curArr[3],
        curArr[4], curArr[5]);
    byte[0] = ((ENET_DEFAULT & 0x00ff0000) >> 16);
    byte[1] = ((ENET_DEFAULT & 0x0000ff00) >> 8);
    byte[2] = (ENET_DEFAULT & 0x000000ff);
    byte[3] = curArr[3];
    byte[4] = curArr[4];
    byte[5] = curArr[5];
#endif /* _BYTE_ORDER == _BIG_ENDIAN */

    printf ("Modify only the last 3 bytes (board unique portion) of Ethernet Address.\n");
    printf ("The first 3 bytes are fixed at manufacturer's default address block.\n");

    for (ix = 0; ix < 3; ix++)
        printf ("%02x- %02x\n", byte[ix], byte[ix]);


    /* start on fourth byte of enet addr */
    for (ix = 3; ix < 6; ix++)
    {
    /* prompt for substitution */

    printf ("%02x- ", byte[ix]);

    /* get substitution value:
     *   skip empty lines (CR only);
     *   quit on end of file or invalid input;
     *   otherwise put specified value at address */

    if (fioRdString (STD_IN, line, MAX_LINE) == EOF)
        break;

    line [MAX_ADR_SIZE] = EOS;  /* make sure input line has EOS */

    for (pLine = line; isspace ((int) *pLine); ++pLine) /* skip leading spaces*/
        ;

    if (*pLine == EOS)          /* skip field if just CR */
        continue;

    if (sscanf (pLine, "%x%1s", &value, &excess) != 1)
        break;              /* quit if not number */

    byte[ix]  = (uchar_t)value;     /* assign new value */
    }

    printf ("\n");

    sysEnetAddrSet (byte[0], byte[1], byte[2], byte[3], byte[4], byte[5]);

    printf ("New Ethernet Address is: ");
    for (ix = 0; ix < 5; ix++)
        printf ("%02x:", byte[ix]);
    printf ("%02x\n", byte[5]);
    }
#endif  /* ETHERNET_ADR_SET */

#ifdef INCLUDE_VXBUS
LOCAL void vxbFindHend
	(
	VXB_DEVICE_ID pDev,
	void * pArg
	)
	{
	char devStr[END_NAME_MAX];
	int strSize;

	/* Is there an HEnd in the device name */

	if (strstr (pDev->pName, "HEnd") == NULL)
		return;

	strSize = strlen(pDev->pName)-4; /* truncate HEnd */

	if (vxbDevMethodGet(pDev, (UINT32)&muxDevConnect_desc) != NULL)	
		{
		strncpy(devStr,pDev->pName,strSize);
		devStr[strSize]=EOS;
		printf(" %s%d",devStr,pDev->unitNumber);
		}
	}
#endif /* INCLUDE_VXBUS */

#endif  /* INCLUDE_NETWORK */

/******************************************************************************
*
* usrPccardDevShow - Show PC Card clients
*
* This routine shows all PC Card clients of the specified type.
*
* RETURNS: OK or ERROR.
*
*/

#ifdef INCLUDE_PCCARD
LOCAL void usrPccardDevShow
    (
    UINT16 type
    )
    {
    CS_CHANDLE client;
    STATUS status;
    CS_CLIENT_INFO info;
    char nameString[80];
    char vendorString[80];
    UINT16 attr = type | CS_CLIENT_TYPE;

    info.nameStringMax = 80;
    info.nameString = nameString;
    info.vendorStringMax = 80;
    info.vendorString = vendorString;

    status = csGetFirstClient (&client, 0, 0, attr);

    while (status != ERROR)
        {
        if (csGetClientInfo (client, &info) == OK)
           printf (" %s", info.nameString);

        status = csGetNextClient (&client, 0, 0, attr);
        }
    }
#endif /* INCLUDE_PCCARD */

#if (CPU_FAMILY == MIPS)
UINT32 mmuMipsTlbVecSize = 0x10;
UINT32 mmuMipsXtlbVecSize = 0x10;

/****************************************************************************
*
* mmuMipsInitialMemoryMap - dummy function
*
* This function is a dummy to prevent the linker from calling in the
* MIPS mmu library for bootroms.
*
* RETURNS: OK always.
*
*/
STATUS mmuMipsInitialMemoryMapSet
    (
    UINT	nTlbEntries,
    ULONG       excTlbPageBaseAddress,
    UINT32	localMemLocalAdrs,		/* LOCAL_MEM_LOCAL_ADRS */
    UINT32	memTop				/* top of memory */
    )
    {
    return OK;
    }

/****************************************************************************
*
* mmuMipsTlbVec - dummy function
*
* This function is a dummy to prevent the linker from calling in the
* MIPS mmu library for bootroms.
*
* RETURNS: OK always.
*
*/

VOID mmuMipsTlbVec(void)
    {
    WRS_ASM (".set noat");
    WRS_ASM ("la $26, excStub");
    WRS_ASM ("jr $26");
    WRS_ASM (".set at");
    }

/****************************************************************************
*
* mmuMipsXTlbVec - dummy function
*
* This function is a dummy to prevent the linker from calling in the
* MIPS mmu library for bootroms.
*
* RETURNS: OK always.
*
*/

VOID mmuMipsXtlbVec(void)
    {
    WRS_ASM (".set noat");
    WRS_ASM ("la $26, excStub");
    WRS_ASM ("jr $26");
    WRS_ASM (".set at");
    }


#endif /* CPU_FAMILY == MIPS */
