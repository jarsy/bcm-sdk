/* usrNetwork.c - network initialization */

/* $Id: usrNetwork.c,v 1.5 2011/07/21 16:14:49 yshtil Exp $
 * Copyright (c) 1992-2005 Wind River Systems, Inc.
 *
 * The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement. No license to Wind River intellectual property rights
 * is granted herein. All rights not licensed by Wind River are reserved
 * by Wind River.
 */

/*
modification history
--------------------
08n,13oct05,ebh  Eliminate TIPC initialization debug statement
08m,28sep05,tlu  add checking for ipv6_forwarding state, exit MN start up if true
		 and change MIP6 init before RTSOL
08m,28sep05,dlk  Add initialization code for unsupported INCLUDE_HOSTCACHE
                 component.
08l,13sep05,dlk  Added ICMPV4_TIMESTAMP_REQ_SUPPORT_CFG support.
08l,10sep05,dsk  Added new TIPC infrastructure initialization for sockets and buffers
08k,02sep05,rp   set mld version correctly
08j,26aug05,spm  updated sysctl dependencies (SPR #106702)
08i,26aug05,dlk  Remove inaccurate ether_output() declaration resulting in
                 compiler warning.
08h,26aug05,ijm  define usrNetworkSymbols for INCLUDE_SCTP
08g,23aug05,xli  Added MIP6 init calls
08e,23aug05,d_c  Remove usrAtaConfig (0, 0, "/ata0a") when INCLUDE_ATA defined
08f,20aug05,dlk  Added IP_MAXFRAGPACKETS_CFG and IP_MAXFRAGSPERPACKET_CFG.
08e,18aug05,kch  Added l2config initialization.
08d,14aug05,niq  Add Ingress QoS support
08c,11aug05,kch  Added mux layer 2, subnet-based vlan and ifclone 
                 initialization.
08b,10aug05,rp   fixed warning for addrctlString
08a,04aug05,wap  Changes for microstack
07z,03aug05,vvv  combined INCLUDE_SCTPV4 and INCLUDE_SCTPV6 into single
                 component
07y,03aug05,vvv  added symbolic reference for SCTP for linking
07x,29jul05,xli  Added mip6CommandInit to pull in MIP6 module
07w,25jul05,dlk  Support new INCLUDE_NETBUFADVLIB and INCLUDE_NETPOOLSHOW
                 components.
07v,20jul05,kch  added SCTP initialization
07u,19jul05,xli  change mnInitialize() into mip6InitializeMn()
07t,13jul05,tlu  Added INCLUDE_MIP6_MN
07s,26jun05,dlk  Restore IPINTRQ_IFQ_MAXLEN_CFG and IP6INTRQ_IFQ_MAXLEN_CFG
		 for loopback use.
		 Add INCLUDE_JOB_QUEUE and INCLUDE_JOB_QUEUE_UTIL.
07r,12jul05,ijm  added DHCPv6 authentication parameters
07q,06jul05,kch  Fixed mld6 router initialization.
07p,05jul05,dlk  Added parameters TCPV6_MTU_MIN_CFG, ICMPV6_MTU_MIN_CFG,
		 ICMPV4_VALIDATION_REQ_CFG, and ICMPV4_MTU_MIN_CFG.
07o,29jun05,wap  Add ifmedia support
07n,28jun05,vvv  added IGNORE_LINKLOCAL
07m,24jun05,rp   added INCLUDE_ADDRCTL_POLICY
07l,24may05,kch  Added IPv6 configuration parameters for ip6_prefer_tempaddr,
                 ip6_mcast_pmtu and _nd6_maxqueuelen. Also added mobility
                 initialization.
07k,20may05,niq  Fix virtual stack initialization error
07j,17may05,spm  fixed virtual stack debug messages (SPR #190404)
07i,17may05,spm  fixed sysctl startup (SPR #108824)
07h,04may05,dlk  Use the configlette usrNetApplUtil.c rather than defining
		 a special version of usrNetApplUtilInit() here. SPR #108879.
		 Replace #include "..." with #include <...> where appropriate.
07g,29apr05,rp   virtualization and sysctl changes for mld
07f,27apr05,rp   merged from comp_wn_ipv6_mld_interim-dev
07f,15apr05,als  removed redundant declaration of tipc_init()
07e,26apr05,vvv  fixed SNTP crash with virtual stacks
07d,13apr05,kch  Corrected tcp debug and net sysctl virtualization issues.
07c,13apr05,niq  Fix fastpath init routine prototype
07b,08apr05,vvv  fixed warnings (SPR #104973)
07a,25mar05,ijm  usrDhcpcLeaseSave stores netmask incorrectly,SPR#106517 
06z,23mar05,niq  Initialize CIDR MIB
06y,09mar05,dlk  Initialize semaphore for INCLUDE_APPL_LOG_UTIL component
                 here rather than in applUtilLib.c for better scalability
		 (SPR #104991).
06x,07mar05,bwa  removed "Net" part of DSI routines to match configlettes
06w,02mar05,vvv  fixed warnings when shared-memory network is included (SPR
                 #106582)
06v,25feb05,niq  mroute changes
06u,24feb05,spm  removed unneeded route format values and macros (SPR #100995)
06t,24feb05,spm  performance updates and code cleanup (SPR #100995)
06s,18feb05,wap  Allow sysctl init routines to be scaled out
06r,11feb05,ijm  removed support for PMP interfaces, SPR#106317
06t,29mar05,bwa  added INCLUDE_TIPC_MEDIA_SM initialization.
06s,21feb05,ebh  update manually (zipply merge was not running)
06r,31jan05,niq  added virtual stack support for sysctl
06q,26jan05,vvv  removed IPv4-only FTP server
06p,04jan05,bwa  removed 'Net' part of DSI routines to match configlettes.
06o,14dec04,syy  Fixed SPR#104035: support for guest login
06n,11nov04,kc   Fixed usrProxyServerInit() virtualization problem (SPR #104110)
06m,20oct04,vvv  added initialization for SM show routine (SPR #102309)
06l,14oct04,dlk  Set default SELECT_NET_ROUTE_SHOW_BUFFERING
                 component if none is selected (SPR #102522).
06k,12oct04,ann  merged from BASE6_ITER6_FRZ11 to move domaininit into 
                 usrNetworkCommon.c.
06j,04oct04,wap  Remove debug message
06i,04oct04,wap  Add missing polling stats hook (SPR #101616)
06h,01oct04,kc   Fixed SPR#102219 - ftpd6 security not enabled for virtual
                 stack in usrFtp6Start().
06g,30sep04,dlk  Fix typo.
06f,30sep04,dlk  Change path to usrNetRouteShow.c to one working for both
                 Base 6 and Medusa.
06e,30sep04,dlk  Add INCLUDE_NET_ROUTE_SHOW changes.
06d,30sep04,syy  Disable code referring to RUNATBOOT
06c,27sep04,niq  Fix the issue with default route
06b,24sep04,spm  build update: fixed include files for virtual stack build
                 and moved network symbols from usrExtra.c file (SPR #98312)
06a,21sep04,kc   Moved igmp router dependency to usrDepend.c
05z,20sep04,vvv  fixed warning
05y,20sep04,spm  updated virtual stack startup for core network: stop
                 packing and unpacking configuration parameters
05x,20sep04,niq  Complete rtsock scaling work
05w,20sep04,spm  fixed virtual stack component build errors (SPR #98312)
05v,20sep04,kc   Fixed warnings.
05u,20sep04,syy  Added INCLUDE_DHCP6C_SHOW
05t,19sep04,spm  updated virtual stack startup and shutdown for TCP and UDP
                 show routines: restored use of published initialization
                 routines and removed unused destructors
05s,19sep04,spm  updated virtual stack startup and shutdown routines for
                 MIB-II components and ICMP utilities: restored use of
                 published routines and stopped packing/unpacking parameters
05r,19sep04,spm  cleanup hostname initialization for virtual stacks; use
                 separate routines for host table setup and hostname assigment
05q,19sep04,rae  update for IGMP router
05p,19sep04,ijm  vxWorks 5x PPP equivalent changes
05o,19sep04,nee  Adding TIPC initialization support
05n,19sep04,dlk  Correct name INCLUDE_APPL_UTIL -> INCLUDE_APPL_LOG_UTIL.
05m,09sep04,dlk  Add INCLUDE_APPL_UTIL.
05l,17sep04,niq  Scale out routing sockets
05k,13sep04,kc   Undo vlan tag library virtualization.
05j,08sep04,vvv  backed out previous checkin
05i,07sep04,ann  fixed the function usrDnsInit
05h,01sep04,nee  Fixing extern misspelled as exttern for OSPF initialization
05g,31aug04,vvv  changed INCLUDE_NET_SYSCTL_API to INCLUDE_NET_SYSCTL
05f,27aug04,vvv  fixed initialization order
05e,26aug04,vvv  fixed sysctl initialization
05d,17aug04,vvv  added vsData.h to fix build issues for other components;
		 fixed compile warnings
05c,13aug04,kc   Fixed telnet client virtualization issue.
05b,11aug04,rae  add INCLUDE_IGMP_ROUTER
05a,06aug04,spm  cleanup virtual stack routine names
04z,03aug04,kc   Fixed ftp server virtualization issue.
04y,28jul04,jwl  Implement virtualization for sntpc, sntps and telnetc
04x,22jul04,kc   Fixed typos for tftp server.
04w,21jul04,spm  removed extra routine from FTP and TFTP server's virtual
                 stack startup; added FTP client virtual stack support
04v,19jul04,niq  Fix calling of component sysctlInit routines
04u,19jul04,kc   Fixed networkinit() so that usrNetHostInit() is called before 
                 MIB2 SYSTEM is inited (SPR #99674). Also added forward
                 declaration for ospfv2/v3 prototypes to remove warnings.
04t,15jul04,vvv  modified INCLUDE_ADDIF for backward compatibility
04s,12jul04,vvv  fixed warnings
04r,09jul04,vvv  fixed GNU warnings
04q,07jul04,dlk  Remove obsolete SOCK_FD_MAX parameter to sockLibInit().
04p,07jul04,vvv  fixed warnings
04o,07jul04,kc   Fixed TCP and UDP show facilities virtualization problems.
04n,06jul04,syy  Using global variables for DHCPv6 init routines in VS
04m,01jul04,syy  Fix DHCPv6 related compiler warnings
04l,28jun04,niq  Move fastpath initialization to until after the interfaces are
                 initialized + Move splSemInit to netCoreInstInit
04k,28jun04,syy  Fix typos
04j,28jun04,kc   Added missing "extern" keyword to aipRegistrationNum.
04i,28jun04,rp   fixed compilation errors for dhcp relay
04h,26jun04,kc   Fixed warning.
04g,24jun04,kc   Added usrNetAutoIpInit() to support IPv4 address auto config
                 for virtual stack.
04f,25jun04,syy  Add virtualization for DHCPv6
04e,23jun04,vvv  fixed warning
04d,22jun04,niq  Fix the calling of rtsockSysctlInit
04c,22jun04,niq  Add rtsol to list of virtual stack apps
04b,21jun04,vvv  added virtual stack support for NFS server
04a,18jun04,kc   Fixed SPR#98401 - added usrNetVlanTagLibInit() to fix vlan 
                 tagging library init issue for virtual stack support.
03z,09jun04,rp   fixed compilation error when dhcp server is included
03y,09jun04,spm  fixed fastpath setup for virtual stacks
03x,05jun04,kc   Corrected the vlanTagLib init sequence.
03w,04jun04,rp   use appropriate login verify routine for ftpdInit
03v,01jun04,rp   merged from iteration5
03u,23may04,nee  Adding OSPF and Auto IP intialization functions
03t,21may04,kc   Added INCLUDE_VLAN_TAG to support layer 2 vlan tagging.
03s,09apr04,nee  Fixing the prototype for usrTelnetdStart
03r,02apr04,spm  merged virtual stack support for applications from Orion
                 project; fixed sysctl startup order to support virtual stacks
03q,01mar04,ann  merged from /main/vdt/base6_itn4-int/2.
03p,25feb04,nee  Setup if_resolve and if6_resolve for ethernet
03o,25feb04,ijm  bootrom/runtime starting ports should differ, SPR#93950 
03n,19feb04,dlk  Add INCLUDE_ETHER_PMP support.
03m,20feb04,vvv  fixed sysctl initialization for ARP parameters (SPR #94091)
03l,20feb04,vvv  made changes for socket/sysctl separation from stack (moved
		 common code to usrNetworkCommon.c)
03l,03feb04,asr  1) ported FTP server enhancements from RS1.3. 
                 2) made security non-optional when FTP server is included
                 3) added protection around loginUserAdd
03k,10feb04,vvv  fixed NFS definitions (SPR #93507)
03j,21jan04,zhr  Added the initialization call for telnet client 
03i,08jan04,rp   fixed SPR 92684
03h,15dec03,ijm  if boot device is not configured, try "other", SPR#70284;
                 fixed Diab warnings
03g,08dec03,pas  fixed diab warnings in usrNetIcmpv4Init and usrNetBootConfig
03f,08dec03,vvv  added hostlib RTP support
03e,06dec03,niq  SPR #75839/83022 fix (RIP merge from router stack)
03d,05dec03,vvv  modified to include INCLUDE_SC_SYSCTL automatically
03c,02dec03,vvv  added socket library system call support
03b,26nov03,rp   ppplite changes
03a,26nov03,wap  Merge in changes from Snowflake
02z,24nov03,ann  added the initialization calls for NFS
02z,21nov03,bwa  added AF_LOCAL/COMP initialization
02y,20nov03,ppp  added inetLibSysctlInit for inetLib initialisation
02x,20nov03,vvv  modified to not include INCLUDE_SC_SYSCTL automatically
02w,20nov03,vvv  added RTP support; fixed warnings
02v,17nov03,rp   header file path update
02u,31oct03,dbt  Removed initialization of _func_wdbIsNowExternal pointer. This 
                 one does no longer exists.
02t,17oct03,rp   vxSim fixes
02s,25sep03,asr  Fix compile error when SNTP is included
02r,16sep03,vvv  updated include paths for new files; removed references to
		 INCLUDE_IPPROTO; fixed some compilation warnings; backed out
		 accidental merge of application code in ver 02o
02q,15sep03,niq  removed hostname because of replacement with gethostname()
02p,11sep03,vvv  merged from ACCORDION_BASE6_MERGE_BASELINE
02o,05sep03,htm  Added code for enabling SNTP client and server.
02n,23jul03,vvv  added initialization for scope6_id (SPR #88481)
02m,20jun03,t_m  added pccardlib
02l,17jun03,vvv  fixed DHCP lease renewal (SPR #88786); add PF_ROUTE socket
		 library unconditionally (SPR #88623)
02k,16may03,vvv  added configuration parameter MIB2IF_LONG_COUNTERS
02j,15may03,spm  updated to Tornado 2.2 FCS release (added SPR #73762 fix)
02i,14may03,vvv  removed adv_net.h
02h,08may03,vvv  added registration for IPv6 functions (SPR #85982)
02g,25apr03,vvv  moved INCLUDE_MIB2_ALL and INCLUDE_MIB2_IPV6ALL dependency
                 completion checks from configAll.h
02f,10apr03,vvv  added missing init routines (SPR #87544)
02e,08apr03,vvv  added call to nd6SysctlInit() (SPR #87360)
02d,03apr03,vvv  modified for netstat componentization (SPR #87352)
02c,07mar03,ann  re-organized due to more componentization.
02b,06feb03,vvv  added DHCPC_DISCOVER_RETRIES (SPR #83246)
02a,18oct02,hsh  make hostLib.h unconditional
01z,18oct02,ham  fixed conflicts between DHCPS and DHCPR (SPR 82650).
                 Initialize structures to eliminate diab error (SPR #76331,
                 SPR #76377)
01y,17oct02,ham  fixed vxWorks.st build error due to WDB symbols (SPR 83160)
01x,16oct02,ham  fixed improper default gateway configuration (SPR 83122).
01w,11oct02,ham  fixed improper netIoctl return value check.
                 recovered the copyright.
		 called m2LibInit and m2Ipv6LibInit (SPR 83041).
01v,04oct02,ham  fixed improper ioctl return value check (SPR 82778).
01u,01oct02,ham  added MIB2_SYSTEM.
01t,18sep02,nee  removing ifr_type for SIOCSIFFLAGS
01s,26aug02,ham  moved hostname[] to kern_sysctl.c, used #ifdef
                 INCLUDE_NETWORK where INCLUDE_NET_INIT used be used
01r,26aug02,wap  Add startup hook for DHCP relay agent
01q,23aug02,kal  added RTSOL, RTADV, RIPNG and RIP
01p,14aug02,ppp  removing the inclusion of usrTor22Init.h
		 added initilisation call for usrNetUtilsInit
		 added a call to ndpInit to pull in the ndp module
		 added a call to arpLibInit inorder to pull in the arp library
01o,13aug02,ham  rewrote the initialization code in response to SPR#80540.
01n,06aug02,ppp  adding INCLUDE_PREFIXCMD
01m,05aug02,ham  merged with TeamF1 version(01i,rvr->01l,pas)
01l,30jul02,pas  bzero after DS_MALLOC
01i,29jul02,rvr  revised as per new clarinet code
01k,25jul02,rvr  made TOR22 spefici changes
01j,27jun02,ppp  merge from t202 for removal of DS_CONF
01i,20jun02,rvr  added rlogInit()
01h,22may02,ham  netBufLib rollback.
01g,10may02,mk   changed ripng/rtadvd.h to rtadvd.h
01f,03may02,mk   merged Hamas changes
                 removed IPSEC added multicast forwarding
01e,02may02,mk   changed usrNetInit to usrNetInitClarinet
01d,02may02,mk   replace KHEAP_ALLOC with malloc
01c,21apr02,mk   included ip.h before icmpLib.h to remove warnings
01b,21apr02,mk   removed compilation warning
01a,13apr02,mk   created.
*/


/*
DESCRIPTION
This file is used to configure and initialize the VxWorks networking 
stack support.
This file is included by usrConfig.c.

SEE ALSO: usrExtra.c

NOMANUAL
*/


/* includes */
#ifndef  __INCusrNetworkc
#define  __INCusrNetworkc


#include <errnoLib.h>
#include <private/excLibP.h>
#include <cplusLib.h>

#ifdef INCLUDE_VIRTUAL_STACK
#include <netinet/vsLib.h>
#endif

#include <netLib.h>
#include <netCore.h>

#include <net/route.h>

#ifdef INCLUDE_NET_BOOT
#include <string.h>
#include <bootLib.h>
#endif /* INCLUDE_NET_BOOT */

#include <hostLib.h>

#ifdef INCLUDE_DNS_RESOLVER
#include <resolvLib.h>
#endif

#include <remLib.h>
#include <netDrv.h>
#include <m2Ipv6Lib.h>

#ifdef INCLUDE_END
#include "configNet.h"
#include <end.h>
#include <endLib.h>
#include <m2IfLib.h>

#ifdef INCLUDE_END_POLLED_STATS
#ifndef END_POLL_STATS_ROUTINE_HOOK
#define END_POLL_STATS_ROUTINE_HOOK     m2PollStatsIfPoll
#endif
#endif

#endif 

#include <ipProto.h>

#ifdef INCLUDE_MUX
#include <muxLib.h>
#endif

#ifdef INCLUDE_MUX_L2
#include <muxL2Lib.h>
#endif /* INCLUDE_MUX_L2 */

#ifdef INCLUDE_SUBNET_VLAN
#include <net/if_vlan_var.h>
#endif /* INCLUDE_SUBNET_VLAN */

#ifdef INCLUDE_IFCLONE
#include <net/if_clone.h>
#endif /* INCLUDE_IFCLONE */

#ifdef INCLUDE_IPV6
#include <ip6Lib.h>
#include <netinet6/nd6.h>
#include <protos/nd6Lib.h>
#endif

#ifdef INCLUDE_MIPV6
#include <netinet6/mip6.h>
#endif /* INCLUDE_MIPV6 */

#ifdef INCLUDE_ICMPV6
#include <protos/icmpv6Lib.h>
#endif

#ifdef INCLUDE_MLD
#include <protos/mldLib.h>
#endif

#ifdef INCLUDE_NDP
#include <net/utils/ndp.h>
#endif

#ifdef INCLUDE_ADDRCTL_POLICY
#include <net/utils/ip6addrctl.h>
#endif

#ifdef INCLUDE_IFCONFIG
#include <net/utils/ifconfig.h>
#endif

#ifdef INCLUDE_L2CONFIG
#include <net/utils/l2config.h>
#endif 

#ifdef INCLUDE_ROUTECMD
#include <net/utils/routeCmd.h>
#endif

#ifdef INCLUDE_PREFIXCMD
#include <net/utils/prefixcmd.h>
#endif

#ifdef INCLUDE_ARP_API
#include <arpLib.h>
#endif

#ifdef INCLUDE_INETLIB
#include <inetLib.h>
#endif

#ifdef INCLUDE_IGMP
#include <protos/igmpLib.h>
#endif

#ifdef INCLUDE_FTP
#include <ftpLib.h>
#include <private/ftpLibP.h>
#endif 

#ifdef INCLUDE_PROXY_SERVER
#include <proxyArpLib.h>
#endif

#ifdef INCLUDE_SNTPS
#include <sntpsLib.h>

/* Include this declaration if hook is defined. */

/* IMPORT STATUS SNTPS_TIME_HOOK (int, void *); */

#endif

#ifdef INCLUDE_SNTPC
#include <sntpcLib.h>
#endif

#ifdef INCLUDE_FTP6
#include <ftp6Lib.h>
#endif 

#ifdef INCLUDE_HOSTCACHE
extern int inhc_init (void);
#endif /* INCLUDE_HOSTCACHE */

#if (defined (INCLUDE_NFS2_SERVER) || defined (INCLUDE_NFS3_SERVER) \
    || defined (INCLUDE_NFS_SERVER_ALL))
#include <nfsd.h>
#include <mountd.h>

#ifndef NFS_USER_ID
#define NFS_USER_ID 2001
#endif

#ifndef NFS_GROUP_ID
#define NFS_GROUP_ID  100
#endif

#ifndef NFS_MAXPATH
#define NFS_MAXPATH 255
#endif

#ifndef NFS_MAXFILENAME
#define NFS_MAXFILENAME 40
#endif

#endif   /* NFS SERVER */

#if (defined (INCLUDE_NFS2_CLIENT) || defined (INCLUDE_NFS3_CLIENT) \
   || defined (INCLUDE_NFS_CLIENT_ALL))

#include <nfsCommon.h>
#include <nfs2Drv.h>
#include <nfs3Drv.h>

#ifndef NFS_USER_ID
#define NFS_USER_ID 2001
#endif

#ifndef NFS_GROUP_ID
#define NFS_GROUP_ID  100
#endif

#ifndef NFS_MAXPATH
#define NFS_MAXPATH 255
#endif

#ifdef __GNUC__
# ifndef alloca
#  define alloca __builtin_alloca
# endif
#endif

#if VX_VERSION == 64
extern UINT32 nfs3MaxCacheLines;
extern UINT32 nfs3BytesPerCacheLine;
extern UINT32 nfs3CacheOptions;
extern UINT32 nfs2MaxCacheLines;
extern UINT32 nfs2BytesPerCacheLine;
extern UINT32 nfs2CacheOptions;
#endif /* VX_VERSION == 64 */

#endif /* (INCLUDE_NFS2_CLIENT) || defined (INCLUDE_NFS3_CLIENT)
          || defined (INCLUDE_NFS_CLIENT_ALL)) */


#if defined (INCLUDE_NFS_MOUNT_ALL)

#ifndef GROUP_EXPORTS
#define GROUP_EXPORTS 0
#endif
#endif

#ifdef INCLUDE_TELNET
#include <ptyDrv.h>
#include <telnetLib.h>
#endif

#ifdef INCLUDE_REMLIB
#include <remLib.h>
#endif 

#ifdef INCLUDE_BPF
#include <bpfDrv.h>
#endif 

#if (defined (INCLUDE_FTPD6_SECURITY))
#include <loginLib.h>
#endif 

#ifdef INCLUDE_MIP6_MN
#include <mip6/mip6.h>
static int prefixcmd_getinet6sysctl (int code);
#endif /* INCLUDE_MIP6_MN */

#ifdef INCLUDE_RTADV
#include <rtadvLib.h>
#endif

#ifdef INCLUDE_RTSOL
#include <rtsolLib.h>
#endif

#ifdef INCLUDE_RIP
#include <rip/ripLib.h>
#endif

#ifdef INCLUDE_RIPNG
#include <ripngLib.h>
#endif

#ifdef INCLUDE_DHCPS
#include <dhcpsLib.h>
#endif

#ifdef INCLUDE_DHCPR
#include <dhcprLib.h>
#endif /* INCLUDE_DHCPR */


#ifdef INCLUDE_DHCPC_SHOW
#include <dhcp/dhcpcShow.h>
#endif 

#ifdef INCLUDE_DHCPC_LEASE_GET
#include <sysLib.h>
#include <dhcp/dhcpcCommonLib.h>
#include <dhcp/dhcpcInit.h>
#endif 

#ifdef INCLUDE_DHCPC
#include <dhcp/dhcpcInit.h>
#include <sysLib.h>
#endif

#ifdef INCLUDE_DHCPC_BOOT
#include <dhcpcBootLib.h>
#endif

#ifdef INCLUDE_DHCPC_SHARE
#include <dhcp/dhcpcCommonLib.h>
#endif 

#ifdef INCLUDE_DHCP6C_SHOW
#include <dhcp6cShow.h>
#ifndef INCLUDE_DHCP6C
#define INCLUDE_DHCP6C
#endif
#endif

#if defined (INCLUDE_DHCP6S) || defined (INCLUDE_DHCP6C) \
    || defined (INCLUDE_DHCP6R)
#include <dhcp6Lib.h>
#endif

#ifdef INCLUDE_IFINDEX
#include <ifIndexLib.h>
#endif

#ifdef INCLUDE_IFLIB
#include <ifLib.h>
#endif 

#ifdef INCLUDE_IF6LIB
#include <if6Lib.h>
#endif 

#if (defined (INCLUDE_TCP_SHOW) || defined (INCLUDE_UDP_SHOW) ||       \
     defined (INCLUDE_ICMP_SHOW) || defined (INCLUDE_IGMP_SHOW) ||     \
     defined (INCLUDE_NET_SHOW) || defined (INCLUDE_NET_ROUTE_SHOW) || \
     defined (INCLUDE_NET_IF_SHOW) || defined (INCLUDE_NET_HOST_SHOW))
#include <netShow.h>
#endif 

#ifdef INCLUDE_DHCPC_LEASE_TEST
#include <dhcp/dhcpcCommonLib.h>
#endif

#ifdef INCLUDE_FTP6
#include <ftp6Lib.h>
#endif 

#ifdef INCLUDE_FTP6_SERVER
#include <ftpd6Lib.h>
#endif

#if (defined (INCLUDE_ICMPLIB) || defined (INCLUDE_ICMPV4))
#include <icmpLib.h>
#endif

#ifdef INCLUDE_TFTP_SERVER
#include <tftpdLib.h>
#endif 

#ifdef INCLUDE_MROUTE
#include <netinet/ip_mroute.h>
#endif

#ifdef INCLUDE_MROUTEV6
#include <netinet6/ip6_mroute.h>
#endif

#ifdef INCLUDE_IGMP_ROUTER
#include <IGMPv2/igmpRouterLib.h>
#endif

#ifdef INCLUDE_RDISC
#include <rdiscLib.h>
#endif

#ifdef INCLUDE_RARP 
#include <protos/rarpLib.h>
#endif

#ifdef INCLUDE_RLOGIN
#include <rlogLib.h>
#endif

#ifdef INCLUDE_FASTUDP
#include <net/fastUdpLib.h>
#endif

#ifdef INCLUDE_FASTUDP6
#include <net/fastUdp6Lib.h>
#endif

#ifdef INCLUDE_FASTPATH
#include <fastPath/fastPathLib.h>
#endif 

#ifdef INCLUDE_ROUTE_STORAGE
#include <route/ipRouteLib.h>
#endif

#ifdef INCLUDE_IFMEDIA
#include <net/if_media.h>
#endif

#ifdef INCLUDE_IPV4
#include <netinet/if_ether.h>
#include <ipLib.h>
#include <route/ipRouteNodeLib.h>
#endif /* INCLUDE_IPV4 */

#ifdef INCLUDE_TCPV4
#include <protos/tcpLib.h>
#endif

#ifdef INCLUDE_SCTP
#include <protos/sctpLib.h>
#include <netinet/sctp.h>
#endif

#ifdef INCLUDE_UDPV4
#include <protos/udpLib.h>
#endif

#ifdef INCLUDE_OLDROUTE
#include <routeLib.h>    
#endif

#ifdef INCLUDE_TUNNELLIB
#include <dlink/tunnelLib.h>
#endif

#ifdef INCLUDE_TCP_DEBUG
#include <netinet/tcp_debug.h>
#endif

#ifdef INCLUDE_VIRTUAL_STACK
#include <netinet/vsLib.h>
#include <netinet/vsData.h>
#endif

#ifdef INCLUDE_SM_NET
#include <drv/end/smEnd.h>
#endif /* INCLUDE_SM_NET */

#ifdef INCLUDE_VLAN_TAG
#include <dlink/vlanTagLib.h>
#endif /* INCLUDE_VLAN_TAG */


#ifdef INCLUDE_QOS_INGRESS
#include <dlink/qosIngressLib.h>
#endif /* INCLUDE_QOS_INGRESS */

/*
 * Extra networking modules to include in VxWorks image.
 * For backward compatibility reasons, the startup routine
 * in this file does not initialize the TFTP client directly,
 * or reference any of its symbols, so this array provides a
 * mechanism to link it automatically. All other network components
 * currently link through their executed initialization routines.
 */

#if (defined (INCLUDE_TFTP_CLIENT) || defined (INCLUDE_SCTP))

#ifdef INCLUDE_TFTP_CLIENT
#include <tftpLib.h>
#endif 

LOCAL VOIDFUNCPTR usrNetworkSymbols [2];
   /*
    * Array contents:
    * 
    * {
    * (VOIDFUNCPTR) tftpCopy,
    * (VOIDFUNCPTR) sctp_bindx
    * };
    *
    * Ideally, this definition would assign the array contents
    * directly. However, that behavior generates a warning since
    * the array is actually unused. Instead, the tftpCopy routine
    * is added to the array later.
    */
#endif /* INCLUDE_TFTP_CLIENT || INCLUDE_SCTP */

#ifdef INCLUDE_MIB2_ALL
#ifndef INCLUDE_MIB2_ICMP
#define INCLUDE_MIB2_ICMP
#endif
#ifndef INCLUDE_MIB2_IF
#define INCLUDE_MIB2_IF
#endif
#ifndef INCLUDE_MIB2_IFMGT
#define INCLUDE_MIB2_IFMGT
#endif
#ifndef INCLUDE_MIB2_IP
#define INCLUDE_MIB2_IP
#endif
#ifndef INCLUDE_MIB2_SYSTEM
#define INCLUDE_MIB2_SYSTEM
#endif
#ifndef INCLUDE_MIB2_TCP
#define INCLUDE_MIB2_TCP
#endif
#ifndef INCLUDE_MIB2_UDP
#define INCLUDE_MIB2_UDP
#endif
#endif /* INCLUDE_MIB2_ALL */

#ifdef INCLUDE_MIB2_IPV6ALL
#ifndef INCLUDE_MIB2_ICMPV6
#define INCLUDE_MIB2_ICMPV6
#endif
#ifndef INCLUDE_MIB2_IPV6
#define INCLUDE_MIB2_IPV6
#endif
#ifndef INCLUDE_MIB2_MLD
#define INCLUDE_MIB2_MLD
#endif
#ifndef INCLUDE_MIB2_TCPV6
#define INCLUDE_MIB2_TCPV6
#endif
#ifndef INCLUDE_MIB2_UDPV6
#define INCLUDE_MIB2_UDPV6
#endif
#endif /* INCLUDE_MIB2_IPV6ALL */

/*
 * The following section provides backward compatibility for the system
 * MIB parameters. This will allow old parameter definitions using
 * MIB2_SYS_* to work. The correct way to define the parameters is to
 * use the MIB2SYS_* definitions.
 */

#ifdef  MIB2_SYS_DESCR
#undef  MIB2SYS_DESCR
#define MIB2SYS_DESCR MIB2_SYS_DESCR
#endif

#ifdef  MIB2_SYS_CONTACT
#undef  MIB2SYS_CONTACT
#define MIB2SYS_CONTACT MIB2_SYS_CONTACT
#endif

#ifdef  MIB2_SYS_LOCATION
#undef  MIB2SYS_LOCATION
#define MIB2SYS_LOCATION MIB2_SYS_LOCATION
#endif

#ifdef  MIB2_SYS_OID_LEN
#undef  MIB2SYS_OID_LEN
#define MIB2SYS_OID_LEN MIB2_SYS_OID_LEN
#endif

#ifdef  MIB2_SYS_OID
#undef  MIB2SYS_OID
#define MIB2SYS_OID MIB2_SYS_OID
#endif


#if (defined (INCLUDE_NFS2_SERVER) || defined (INCLUDE_NFS3_SERVER) \
    || defined (INCLUDE_NFS_SERVER_ALL))

int nfsServMaxPath = NFS_MAXPATH + 1;
int nfsMaxFileName = NFS_MAXFILENAME;
int    *pnfsdRebootTime = NULL;
FUNCPTR pNlmdInit = NULL;

#ifdef VIRTUAL_STACK
FUNCPTR pNlmdStop = NULL;
#endif

#if defined (INCLUDE_NFS_SERVER_ALL) || defined (INCLUDE_NFS2_SERVER)
extern void nfs2dRequestEnqueue (); 
extern NFSDCALLBACK nfsv2CallBack;
extern MOUNTCALLBACK mountv1CallBack;    /* call back function for MOUNT v1 */
extern void mount1dRequestProcess ();
extern NFS2_SERVER_STATUS nfs2ServerStatus;
extern NFS2_SERVER_STATUS *pnfs2ServerStat;
STATUS usrNfsdStart (void);
#endif

#if defined (INCLUDE_NFS_SERVER_ALL) || defined (INCLUDE_NFS3_SERVER)
extern void nfs3dRequestEnqueue (); 
extern NFSDCALLBACK nfsv3CallBack;
extern MOUNTCALLBACK mountv3CallBack;    /* call back function for MOUNT v3 */
extern void mount3dRequestProcess ();
extern void mountv1dRequestProcess ();
extern STATUS nlmdInit (int, int, int);

#ifdef VIRTUAL_STACK
extern STATUS nlmdStop (int);
#endif

extern int  nfsdRebootTime;
extern NFS3_SERVER_STATUS *pnfs3ServerStat;
extern NFS3_SERVER_STATUS nfs3ServerStatus;
STATUS usrNfsdStart (void);
#endif

#endif

#if (defined (INCLUDE_NFS2_CLIENT) || defined (INCLUDE_NFS3_CLIENT) \
    || defined (INCLUDE_NFS_CLIENT_ALL))
const int nfsMaxPath = NFS_MAXPATH + 1;
STATUS usrNfsInit (int version);

#if VX_VERSION == 64
#ifdef INCLUDE_NFS2_CLIENT
void usrNfs2CacheInit (UINT32 maxLines, UINT32 lineSize, UINT32 options);
#endif

#ifdef INCLUDE_NFS3_CLIENT
void usrNfs3CacheInit (UINT32 maxLines, UINT32 lineSize, UINT32 options);
#endif
#endif /* VX_VERSION == 64 */

#endif /* (INCLUDE_NFS_SERVER_ALL) || defined (INCLUDE_NFS2_SERVER) */


#if defined (INCLUDE_NFS_MOUNT_ALL)

IMPORT BOOL group_export;
void usrNetNfsMountAll(void);

#endif

#ifdef INCLUDE_DHCP6C
void   usrNetDhcp6cInit(void);
#ifdef VIRTUAL_STACK
extern VS_REG_ID dhcp6cRegistrationNum;
extern STATUS dhcp6cInstInit(void *);
extern STATUS dhcp6cDestructor (VSNUM);
struct dhcp6cInitParams dh6cParams;
#endif /* VIRTUAL_STACK */
#endif /* INCLUDE_DHCP6C */

#ifdef INCLUDE_DHCP6S
void   usrNetDhcp6sInit(void);
#ifdef VIRTUAL_STACK
extern VS_REG_ID dhcp6sRegistrationNum;
extern STATUS dhcp6sInstInit(void *);
extern STATUS dhcp6sDestructor (VSNUM);
struct dhcp6sInitParams dh6sParams;
#endif /* VIRTUAL_STACK */
#endif /* INCLUDE_DHCP6S */

#ifdef INCLUDE_DHCP6R
void   usrNetDhcp6rInit(void);
#ifdef VIRTUAL_STACK
extern VS_REG_ID dhcp6rRegistrationNum;
extern STATUS dhcp6rInstInit(void *);
extern STATUS dhcp6rDestructor (VSNUM);
struct dhcp6rInitParams dh6rParams;
#endif /* VIRTUAL_STACK */
#endif /* INCLUDE_DHCP6R */

#ifdef INCLUDE_MLD6_ROUTER
STATUS usrNetMld6rInit (void);
extern STATUS mldrLibInit (void); 
#ifdef VIRTUAL_STACK
extern VS_REG_ID mldrRegistrationNum;
extern STATUS mldrDestructor (VSNUM);
#endif /* VIRTUAL_STACK */
#endif /* INCLUDE_MLD6_ROUTER */

/* globals */


#ifdef INCLUDE_END
IMPORT END_TBL_ENTRY endDevTbl[];
#endif /* INCLUDE_END */

#if defined(BROADCOM_BSP)
IMPORT STATUS sysEnetAddrSet(unsigned char *pMac);
#endif


#ifdef INCLUDE_NET_BOOT
/* externals */

IMPORT BOOT_PARAMS sysBootParams;

/* locals */

#ifdef INCLUDE_BOOT_LINE_INIT
#ifdef INCLUDE_NET_BOOT_CONFIG
LOCAL BOOL diskBoot = FALSE;
#endif /* INCLUDE_NET_BOOT_CONFIG */
#endif /* INCLUDE_BOOT_LINE_INIT */

LOCAL BOOL netDevBootFlag = FALSE;  /* 
                                     * Set TRUE once a device is started
                                     * or if processing can't continue.
                                     */
#ifndef INCLUDE_NET_MICRO_STACK
LOCAL BOOL netAttachFlag = FALSE;   /* BSD or END device available? */
LOCAL int       netmask;            /* Device netmask. */
#endif	/* INCLUDE_NET_MICRO_STACK */ 
LOCAL char * 	pAddrString;        /* Selects address field for device. */
LOCAL char * 	pDevName;           /* Selects name for device. */
LOCAL char *    pTgtName;           /* Selects name for the target */
LOCAL int       uNum;               /* Selects the unit number */

#ifdef INCLUDE_SM_NET
LOCAL BOOL      backplaneBoot = FALSE;
#endif

#endif /* INCLUDE_NET_BOOT */

#ifdef INCLUDE_NET_DAEMON
extern unsigned int netJobNum;
extern int      netTaskId;
extern int      netTaskPriority;
extern int      netTaskOptions;
extern int      netTaskStackSize;
extern void     netTask (void);
extern STATUS	netLibInit (void);

extern STATUS bdall (int);
extern FUNCPTR _func_bdall;

#endif /* INCLUDE_NET_DAEMON */

#ifdef  INCLUDE_NETSTAT_IF   
extern void netstatIfInit ();
#endif /* INCLUDE_NETSTAT_IF */

#ifdef  INCLUDE_NETSTAT_IPV4   
extern void netstatIpInit ();
#endif /* INCLUDE_NETSTAT_IPV4 */

#ifdef  INCLUDE_NETSTAT_IPV6   
extern void netstatIp6Init ();
#endif /* INCLUDE_NETSTAT_IPV6 */

#ifdef  INCLUDE_NETSTAT_TCP   
extern void netstatTcpInit ();
#endif /* INCLUDE_NETSTAT_TCP */

#ifdef  INCLUDE_NETSTAT_SCTP   
extern void netstatSctpInit ();
#endif /* INCLUDE_NETSTAT_SCTP */

#ifdef  INCLUDE_NETSTAT_UDP   
extern void netstatUdpInit ();
#endif /* INCLUDE_NETSTAT_UDP */

#ifdef  INCLUDE_NETSTAT_RAWV6   
extern void netstatRip6Init ();
#endif /* INCLUDE_NETSTAT_RAWV6 */

#ifdef  INCLUDE_NETSTAT_ICMPV4   
extern void netstatIcmpInit ();
#endif /* INCLUDE_NETSTAT_ICMPV4 */

#ifdef  INCLUDE_NETSTAT_ICMPV6   
extern void netstatIcmp6Init ();
#endif /* INCLUDE_NETSTAT_ICMPV6 */

#ifdef  INCLUDE_NETSTAT_ROUTE   
extern void netstatRouteInit ();
#endif /* INCLUDE_NETSTAT_ROUTE */

#ifdef  INCLUDE_NETSTAT_IGMP   
extern void netstatIgmpInit ();
#endif /* INCLUDE_NETSTAT_IGMP */

#ifdef  INCLUDE_NETSTAT_MROUTE   
extern void netstatMrouteInit ();
#endif /* INCLUDE_NETSTAT_MROUTE */

#ifdef  INCLUDE_NETSTAT_MROUTEV6
extern void netstatMroute6Init ();
#endif /* INCLUDE_NETSTAT_MROUTEV6 */

#ifdef  INCLUDE_NETSTAT_UN_COMP
extern void netstatUncompInit (void);
#endif /* INCLUDE_NETSTAT_UN_COMP */

#ifdef INCLUDE_ZBUF_SOCK
STATUS zbufSockLibInit (void);
#endif /* INCLUDE_ZBUF_SOCK */

#ifdef INCLUDE_NDP
extern void ndpInit();
#endif

#ifdef INCLUDE_GIF
extern void gifInit ();
#endif

#ifdef INCLUDE_STF
extern void stfInit ();
#endif

#ifdef VIRTUAL_STACK
VS_REG_ID netCoreRegistrationNum;
#endif

#ifdef INCLUDE_TCP_SHOW
#ifdef VIRTUAL_STACK
VS_REG_ID tcpShowRegistrationNum;
#endif /*VIRTUAL_STACK*/
#endif /* INCLUDE_TCP_SHOW */

#ifdef INCLUDE_UDP_SHOW
#ifdef VIRTUAL_STACK
VS_REG_ID udpShowRegistrationNum;
#endif /*VIRTUAL_STACK*/
#endif /* INCLUDE_UDP_SHOW */

#ifdef INCLUDE_UN_SHOW
extern void unShowInit (void);
#endif

#ifdef INCLUDE_UN_COMP_SHOW
extern void uncompShowInit (void);
#endif

#ifdef INCLUDE_RIP
extern BOOL initInterfaces;
extern int  recvBufSize;
#endif  /* INCLUDE_RIP */

#ifdef INCLUDE_OSPF
extern void ospfInstInit ();
#endif /* INCLUDE_OSPF */

#ifdef INCLUDE_OSPFV3
extern STATUS ospfv3InstInit ();
#endif /* INCLUDE_OSPFV3 */

#ifdef INCLUDE_AIP
STATUS usrNetAutoIpInit ();    /* IPV4 address auto configuration */
extern STATUS aipLibInit(void);
#ifdef VIRTUAL_STACK
extern VS_REG_ID aipRegistrationNum;
extern STATUS aipInstInit (void * pParams);
extern STATUS aipInstDestroy (VSNUM vsnum);
#endif /* VIRTUAL_STACK */
#endif

#ifdef INCLUDE_GETNAMEINFO
#ifdef INCLUDE_RTP
extern STATUS   rtpGetnameinfoInit ();
#ifdef VIRTUAL_STACK
VS_REG_ID rtpgetnameRegistrationNum;
#endif /* VIRTUAL_STACK */
#endif
#endif

#ifdef   INCLUDE_DNS_RESOLVER 
extern STATUS dnsInstInit (void *InitValues);
extern DNS_CONFIG_PARAMS dnsDefaultConfigParams;
#ifdef VIRTUAL_STACK
extern STATUS dnsDestructor (VSNUM vsnum);
extern VS_REG_ID dnsRegistrationNum;
#endif
#endif /* INCLUDE_DNS_RESOLVER */

#ifdef  INCLUDE_NET_HOST_SETUP
extern STATUS hostnameSetup (char * pLocalName);
#ifdef INCLUDE_NET_SYSCTL
extern void hostnameSysctlSetup (void);
#endif /* INCLUDE_NET_SYSCTL */
#endif /* INCLUDE_NET_HOST_SETUP */

#ifdef INCLUDE_HOST_TBL
#ifdef VIRTUAL_STACK
VS_REG_ID hostRegistrationNum;
extern STATUS   hostDestructor (VSNUM);
#endif

LOCAL STATUS usrNetHostTblSetup (void);

extern STATUS hostTblSetup (char * pRemoteName, char * pRemoteAddr,
                            char * pLocalName, char * pLocalAddr);

#ifdef INCLUDE_RTP
extern STATUS   rtpHostLibInit ();
#endif
#endif /* INCLUDE_HOST_TBL */


#ifdef INCLUDE_ROUTING_SOCKET
#ifdef VIRTUAL_STACK
extern VS_REG_ID rtsockRegistrationNum;
extern STATUS rtsockDestructor (VSNUM vsnum);
#endif
extern STATUS rtsockInstInit (void);
#endif /* ROUTING_SOCKET */


#ifdef INCLUDE_IPV4
#ifdef VIRTUAL_STACK
extern VS_REG_ID arpRegistrationNum;
extern VS_REG_ID ipRegistrationNum;
extern VS_REG_ID rawRegistrationNum;

extern STATUS arpDestructor (VSNUM);
extern STATUS ipDestructor (VSNUM vsnum);
extern STATUS rawDestructor (VSNUM);
#endif
extern ARP_CONFIG_PARAMS arpDefaultConfigParams;
extern IP_CONFIG_PARAMS ipDefaultConfigParams;
extern struct ipRouteDispatchTable IPV4_RT_DISP_TBL;
extern BOOL ipForwardCfg;

extern STATUS arpInstInit (void *);
extern STATUS ipInstInit (void * InitValues);
extern STATUS rawInstInit (void *);
extern STATUS netCoreInstInit (void *);
#endif /* INCLUDE_IPV4 */

#ifdef INCLUDE_ICMPV4
#ifdef VIRTUAL_STACK
extern VS_REG_ID icmpRegistrationNum;
extern STATUS icmpDestructor (VSNUM vsnum);
#endif
extern ICMP_CONFIG_PARAMS icmpDefaultConfigParams;

extern STATUS icmpInstInit (void *);
#endif

#ifdef INCLUDE_IGMP
#ifdef VIRTUAL_STACK
extern VS_REG_ID igmpRegistrationNum;
extern STATUS igmpDestructor (VSNUM vsnum);
#endif
extern IGMP_CONFIG_PARAMS igmpDefaultConfigParams;
IMPORT STATUS igmpInstInit (void *);
#endif

#ifdef INCLUDE_IGMP_ROUTER
#ifdef VIRTUAL_STACK
VS_REG_ID igmpRouterRegistrationNum;
extern STATUS igmpRouterDestructor (VSNUM vsnum);
#endif
IMPORT STATUS igmpRouterLibInit ();
#endif

#ifdef INCLUDE_TCP_DEBUG
#ifdef VIRTUAL_STACK
extern VS_REG_ID tcpDebugRegistrationNum;
#endif /* VIRTUAL_STACK */
#endif

#ifdef INCLUDE_TCPV4
#ifdef VIRTUAL_STACK
extern VS_REG_ID tcpRegistrationNum;
extern STATUS tcpDestructor (VSNUM);
#endif
extern TCP_CONFIG_PARAMS tcpDefaultConfigParams;

extern STATUS tcpInstInit (void *);
#endif

#ifdef INCLUDE_SCTP
#ifdef VIRTUAL_STACK
extern VS_REG_ID sctpRegistrationNum;
#endif /* VIRTUAL_STACK */
SCTP_CONFIG_PARAMS sctpDefaultConfigParams;

#ifdef INCLUDE_IPV6
#ifdef VIRTUAL_STACK
extern VS_REG_ID sctpv6RegistrationNum;
#endif /* VIRTUAL_STACK */
extern SCTP6_CONFIG_PARAMS sctp6DefaultConfigParams;
#endif /* INCLUDE_IPV6 */
#endif /* INCLUDE_SCTP */

#ifdef INCLUDE_UDPV4
#ifdef VIRTUAL_STACK
extern VS_REG_ID udpRegistrationNum;
extern STATUS udpDestructor (VSNUM);
#endif
extern UDP_CONFIG_PARAMS udpDefaultConfigParams;

extern STATUS udpInstInit (void *);
#endif

#ifdef INCLUDE_MROUTE
#ifdef VIRTUAL_STACK
VS_REG_ID mrouteRegistrationNum;
extern STATUS mrouteDestructor (VSNUM vsnum);
#endif
extern STATUS mrouteInstInit (void *);
extern IPM_CONFIG_PARAMS ipmDefaultConfigParams;
#endif

#ifdef INCLUDE_ICMPV6
#ifdef VIRTUAL_STACK
extern VS_REG_ID icmpv6RegistrationNum;
extern STATUS icmpv6Destructor (VSNUM);
#endif
extern ICMPV6_CONFIG_PARAMS icmpv6DefaultConfigParams;

extern STATUS icmpv6InstInit (void *);
#endif


#ifdef INCLUDE_IPV6
#ifdef VIRTUAL_STACK
extern VS_REG_ID ipv6RegistrationNum;
extern VS_REG_ID nd6RegistrationNum;

extern STATUS ipv6Destructor (VSNUM);
extern STATUS nd6Destructor (VSNUM);
#endif
extern IPV6_CONFIG_PARAMS IPv6DefaultConfigParams;
extern ND6_CONFIG_PARAMS nd6DefaultConfigParams;
extern struct ipRouteDispatchTable IPV6_RT_DISP_TBL;

extern STATUS ipv6InstInit (void *);
extern STATUS nd6InstInit (void *);
#endif /* INCLUDE_IPV6 */

#ifdef INCLUDE_MIPV6
#ifdef VIRTUAL_STACK
extern VS_REG_ID mipv6RegistrationNum;
extern STATUS mipv6Destructor (VSNUM);
#endif /* VIRTUAL_STACK */
extern STATUS mipv6InstInit (void *);
extern MIPV6_CONFIG_PARAMS mipv6DefaultConfigParams;
#endif /* INCLUDE_MIPV6 */

#ifdef INCLUDE_MLD
#ifdef VIRTUAL_STACK
extern VS_REG_ID mldRegistrationNum;
extern STATUS mldDestructor(VSNUM);
#endif /* VIRTUAL_STACK */
extern STATUS mldInstInit (void *);
extern MLD_CONFIG_PARAMS mldDefaultConfigParams;
#endif /* INCLUDE_MLD */

#ifdef INCLUDE_UDPV6
#ifdef VIRTUAL_STACK
extern VS_REG_ID udpv6RegistrationNum;
#endif
extern UDP6_CONFIG_PARAMS udp6DefaultConfigParams;
extern STATUS udpv6InstInit (void *);
#endif

#ifdef INCLUDE_TCPV6
#ifdef VIRTUAL_STACK
extern VS_REG_ID tcpv6RegistrationNum;
#endif
extern TCP6_CONFIG_PARAMS tcp6DefaultConfigParams;
extern STATUS tcpv6InstInit (void *);
#endif

#ifdef INCLUDE_MROUTEV6
#ifdef VIRTUAL_STACK
VS_REG_ID mroutev6RegistrationNum;
extern STATUS mroutev6Destructor (int vsNum);
#endif /* VIRTUAL_STACK */
extern STATUS mroutev6InstInit (void *InitValues);
extern IPM6_CONFIG_PARAMS ipm6DefaultConfigParams;
#endif /* INCLUDE_MROUTEV6 */

#ifdef INCLUDE_IFCONFIG
extern STATUS ifconfigInit (void);
#endif /* INCLUDE_IFCONFIG */

#ifdef INCLUDE_IFCONFIG_PREFIX
extern STATUS prefixLibInit (void);
#endif /* INCLUDE_IFCONFIG_PREFIX */

#ifdef INCLUDE_LOOPBACK
IMPORT STATUS loopbackInstInit (void * cfgParams);
#ifdef INCLUDE_IPV6
STATUS usrVsWrapperIp6_init2 (void * cfgParams);
extern void ip6_init2 ();
#endif /* INCLUDE_IPV6 */
#endif /* INCLUDE_LOOPBACK */

#ifdef INCLUDE_ETHERNET
extern int ipEtherResolvRtn ();
#ifdef INCLUDE_IPV6
extern int ip6EtherResolvRtn ();
#endif
#include <net/if_var.h>
extern int ether_resolvemulti ();
#endif /* INCLUDE_ETHERNET */

#if defined INCLUDE_DHCPR 
#ifdef VIRTUAL_STACK
extern VS_REG_ID dhcprRegistrationNum;
#endif /* VIRTUAL_STACK */
extern DHCPR_CONFIG_PARAMS dhcprDefaultConfigParams;

extern STATUS dhcprInstInit (void *InitValues);

/* 
 * This table contains a list of interfaces that the relay agent
 * will listen to. The boot device is used when the table's	
 * first entry is the null string.
 */

DHCPR_IF_DESC dhcprIfTbl [] =
    {
    {""}			/* Use primary interface */
/* Sample interface entries. */
/*  {"fei1"}, */
/*  {"ln0"} */
    };

#endif /* INCLUDE_DHCPR */

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
    { NULL } /* List terminator, must be last */
     };
#endif  /* INCLUDE_DHCPS || INCLUDE_DHCPR */

#ifdef INCLUDE_DHCPS
#ifndef VIRTUAL_STACK
IMPORT STATUS dhcpsLeaseHookAdd (FUNCPTR);
IMPORT STATUS dhcpsAddressHookAdd (FUNCPTR);
#else
extern VS_REG_ID dhcpsRegistrationNum;
#endif /* VIRTUAL_STACK */

extern DHCPS_CFG_PARAMS dhcpsDefaultConfigParams;

extern STATUS dhcpsInstInit (void *InitValues);

/* Include these declarations if hooks are defined. */

/* IMPORT STATUS DHCPS_LEASE_HOOK (int, char *, int); */

/* IMPORT STATUS DHCPS_ADDRESS_HOOK (int, char *, int); */

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

/*
 * This table contains a list of interfaces that the server
 * will listen to. The boot device is used when the table's
 * first entry is the null string.
 */

DHCPS_IF_DESC dhcpsIfTbl [] =
    {
    {""}                        /* Use primary interface */
/* Sample interface entries. */
/*  {"fei1"}, */
/*  {"ln0"} */
    };

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

/* {"ent1", "90.11.42.24", "90.11.42.24", "clid=\"1:0x08003D21FE90\":maxl=90:dfll=6
0"},   */
/* {"ent2", "90.11.42.25", "90.11.42.26", "snmk=255.255.255.0:maxl=90:dfll=70:file=
/vxWorks"}, */
/* {"ent3", "90.11.42.27", "90.11.42.27", "maxl=0xffffffff:file=/vxWorks"}, */
/* {"entry4", "90.11.42.28", "90.11.42.29", "albp=true:file=/vxWorks"}      */
    };

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
    { NULL,                           NULL } /* List terminator, must be last */
    };

#endif /* INCLUDE_DHCPS */

#ifdef INCLUDE_ADDIF
extern STATUS addifInit (int, char *, char *, ULONG);
#endif

#ifdef INCLUDE_FASTUDP
#ifdef VIRTUAL_STACK
extern VS_REG_ID fastUdpRegistrationNum;
extern STATUS fastUdpDestructor (VSNUM);
#endif

extern FASTUDP_CONFIG_PARAMS fastUdpDefaultConfigParams;

extern STATUS fastUdpInstInit (void *);

/*
 * This is the configuration table for Fast UDP. It is used to reserve some
 * of the UDP ports for Fast UDP. Each entry indicates a range of ports which
 * can be used by a Fast UDP application. These ports are not available to
 * normal UDP applications. Also, these ports cannot be shared by applications.
 */

PORT_TBL portCfgTbl [] =
    {
    /*
     * Start port     End Port
     * ----------     --------
     */

    /* Default dummy entry (required if there are no other entries). */

    { 0, 0 },

    /* Sample entries */

    /* { 1030, 1060 }, */
    /* { 9000, 9000 }, */
    };
#endif /* INCLUDE_FASTUDP */

#ifdef INCLUDE_FASTUDP6
#ifdef VIRTUAL_STACK
extern VS_REG_ID fastUdp6RegistrationNum;
extern STATUS fastUdp6Destructor (VSNUM);
#endif

extern FASTUDP6_CONFIG_PARAMS fastUdp6DefaultConfigParams;

extern STATUS fastUdp6InstInit (void *);

/*
 * This is the configuration table for Fast UDPv6. It is used to reserve some
 * of the UDP ports for Fast UDPv6. Each entry indicates a range of ports which
 * can be used by a Fast UDPv6 application. These ports are not available to
 * normal UDPv6 applications. Also, these ports cannot be shared by applications.
 */

LOCAL PORT6_TBL port6CfgTbl [] =
    {
    /*
     * Start port     End Port
     * ----------     --------
     */

    /* Default dummy entry (required if there are no other entries). */

    { 0, 0 },

    /* Sample entries */

    /* { 1030, 1060 }, */
    /* { 9000, 9000 }, */
    };
#endif /* INCLUDE_FASTUDP6 */

#ifdef INCLUDE_DHCPC_LEASE_CLEAN
/* XXX */

LOCAL STATUS dhcpcBootLineClean (char *);

#endif /* INCLUDE_DHCPC_LEASE_CLEAN */

#ifdef INCLUDE_IP6ATTACH
LOCAL STATUS do_autoconfig (char *, int);
#endif

#ifdef INCLUDE_ICMPLIB
#ifdef VIRTUAL_STACK
VS_REG_ID icmpUtilRegistrationNum;
extern STATUS icmpUtilDestructor (VSNUM);
#endif

extern STATUS icmpUtilLibInit (void);
#endif /* INCLUDE_ICMPLIB */


/* forward declarations */

STATUS usrBootLineParse (char *);

#ifdef INCLUDE_TFTP_SERVER

#ifdef VIRTUAL_STACK
VS_REG_ID tftpdRegistrationNum;

extern STATUS tftpdDestructor (VSNUM vsnum);
#endif /* VIRTUAL_STACK */

LOCAL STATUS usrTftpdInit (void);
#endif /* INCLUDE_TFTP_SERVER */

#ifdef INCLUDE_FTP6_SERVER
LOCAL STATUS   usrFtpd6Start (void); /* File Transfer Protocol v4/v6 Server */
#endif /* INCLUDE_FTP6_SERVER */

#ifdef INCLUDE_FTP

#ifdef VIRTUAL_STACK
LOCAL int ftpvsnum;

VS_REG_ID ftpRegistrationNum;
#endif /* VIRTUAL_STACK */

STATUS   usrFtpInit (void);
STATUS ftpLibInit (long timeout);
#endif

#ifdef INCLUDE_RPC
STATUS usrNetRpcInit ();
#endif

#ifdef INCLUDE_DOMAIN_INIT
void usrNetDomainInit (void);
#endif

#ifdef INCLUDE_IPV4
void usrNetIpv4Init (void);
#endif /* INCLUDE_IPV4 */

#ifdef INCLUDE_ICMPV4
void usrNetIcmpv4Init (void);
#endif /* INCLUDE_ICMPV4 */

#ifdef INCLUDE_IGMP
void usrNetIgmpInit (void);
#endif /* INCLUDE_IGMP */

#ifdef INCLUDE_TCPV4
void usrNetTcpv4Init (void);
#endif /* INCLUDE_TCPV4 */

#ifdef INCLUDE_TCP_DEBUG
void usrNetTcpv4DebugInit (void);
#endif /* INCLUDE_TCP_DEBUG */

#ifdef INCLUDE_SCTP
void usrNetSctpInit (void);
#endif /* INCLUDE_SCTP */

#ifdef INCLUDE_RAWV4
void usrNetRawv4Init (void);
#endif /* INCLUDE_RAWV4 */

#ifdef INCLUDE_UDPV4
void usrNetUdpv4Init (void);
#endif /* INCLUDE_UDPV4 */

#ifdef INCLUDE_MROUTE
void usrNetIpMrouteInit (void);
#endif /* INCLUDE_MROUTE */

#ifdef INCLUDE_ND
void usrNetNdInit (void);
#endif /* INCLUDE_ND */

#ifdef INCLUDE_IPV6_FW
void usrNetIpv6FwInit (void);
#endif /* INCLUDE_IPV6_FW */

#ifdef INCLUDE_ICMPV6
void usrNetIcmpv6Init (void);
#endif /* INCLUDE_ICMPV6 */

#ifdef INCLUDE_MLD
void usrNetMldInit (void);
#endif /* INCLUDE_MLD */

#ifdef INCLUDE_UDPV6
void usrNetUdpv6Init (void);
#endif /* INCLUDE_UDPV6 */

#ifdef INCLUDE_TCPV6
void usrNetTcpv6Init (void);
#endif /* INCLUDE_TCPV6 */

#ifdef INCLUDE_MROUTEV6
void usrNetIpMroutev6Init (void);
#endif /* INCLUDE_MROUTEV6 */

#ifdef INCLUDE_ROUTING_SOCKET
STATUS usrNetRtSockInit (void);  /* XXX: general VS routing support */
#endif /* ROUTING_SOCKET */

#ifdef INCLUDE_IPV6
void usrNetIpv6Init (void);
#endif /* INCLUDE_IPV6 */

#ifdef INCLUDE_MIPV6
void usrNetMipv6Init (void);
#endif /* INCLUDE_MIPV6 */

#ifdef  INCLUDE_NET_HOST_SETUP
#ifdef VIRTUAL_STACK
VS_REG_ID hostnameRegistrationNum;
extern STATUS hostnameDestroy (VSNUM);
#endif

STATUS usrNetHostAdd (void);
#endif

#ifdef INCLUDE_NET_REM_IO
void usrNetRemoteCreate (void);          /* Allows access to file system on boot host */
#endif /*  INCLUDE_NET_REM_IO */

STATUS usrNetIfInit (void);

#ifdef INCLUDE_IFCONFIG
void usrNetIfconfigInit (void);
#endif /* INCLUDE_IFCONFIG */

#ifdef INCLUDE_LOOPBACK
void usrNetLoopbackStart (void);
#endif
    
#ifdef INCLUDE_ETHERNET
STATUS usrNetEther (void);
#endif

#ifdef INCLUDE_END
STATUS usrNetEndLibInit();
#endif

#ifdef INCLUDE_IPATTACH  
void usrNetIpAttachCommon (char *,int,char *,FUNCPTR);
#endif

#ifdef INCLUDE_IP6ATTACH
STATUS usrNetIpv6Attach (char *, int);
#endif /* INCLUDE_IP6ATTACH */

#ifdef INCLUDE_ROUTE
void usrNetRoute (void);
#endif
    
#ifdef INCLUDE_NDP
void  usrNetNdpInit ();                  
#endif /* INCLUDE_NDP */

#ifdef INCLUDE_ADDRCTL_POLICY
void  usrNetAddrCtlInit ();                  
#endif /* INCLUDE_ADDRCTL_POLICY */

#ifdef INCLUDE_ROUTECMD
void usrNetRouteCmdInit (void);
#endif 

#ifdef INCLUDE_MIP6_MN
STATUS mip6CommandInit();
#endif /* INCLUDE_MIP6_MN */

#ifdef INCLUDE_RTSOL
STATUS usrNetRtsolInit (void);
#endif

#ifdef INCLUDE_RTADV
STATUS usrNetRtadvInit (void);
#endif

#ifdef INCLUDE_RIP
LOCAL STATUS usrNetRipInit
    (
    BOOL supplier,	    /* operate in silent mode? */
    BOOL gateway,	    /* act as gateway to the Internet? */
    BOOL multicast,	    /* use multicast or broadcast addresses? */
    int version,            /* 1 or 2: selects format of outgoing messages */
    int timerRate,          /* update frequency for internal routing table */
    int supplyInterval,     /* update frequency for neighboring routers */
    int expire,             /* maximum interval for renewing learned routes */
    int garbage,            /* elapsed time before deleting stale route */
    int authType            /* default authentication type to use */
    );
#endif

#ifdef INCLUDE_RIPNG
LOCAL STATUS usrripngInit 
    (
    );
#endif    
 

#ifdef INCLUDE_RDISC
LOCAL STATUS usrRdiscInit
    (
    int     priority,
    int     options,
    int     stacksize
    );
#endif 

#ifdef INCLUDE_SECURITY
void usrSecurity (void);
#endif

#if (defined(INCLUDE_TCPV4) && defined(INCLUDE_NET_SYM_TBL))
void advanceTcpPorts (void);
#endif

#ifdef INCLUDE_NET_BOOT_CONFIG
STATUS usrNetBootConfig
    (
    char *      devName,                /* device name e.g. "fei" */
    int         unit,			/* unit number */
    char *      addr,                   /* target ip address */
    int         netmask,                /* subnet mask */
    char *	gateway			/* default gateway */
    );
#endif

#ifdef INCLUDE_PING
STATUS pingLibInit (void);
#endif


#ifdef INCLUDE_PING6
STATUS ping6LibInit(void);                     /* ICMP (ping6) client */
#endif 


#ifdef  INCLUDE_DNS_RESOLVER 
LOCAL STATUS usrDnsInit (void);
#endif

#ifdef INCLUDE_NET_BOOT
void  usrNetBoot(void);
#endif 

#ifdef  INCLUDE_NET_DAEMON
STATUS usrNetDaemonInit
    (
    unsigned int	jobNum,
    unsigned int	pri,
    int			opts,
    unsigned int	stack
    );
#endif 

#ifdef INCLUDE_ARP_API
void    usrNetArpLibInit ();    /* ARP table manipulation library */
#endif  /* INCLUDE_ARP_API */

#ifdef INCLUDE_TELNET
LOCAL STATUS usrTelnetdStart (void);
#endif 

#ifdef INCLUDE_TELNET_CLIENT
LOCAL STATUS  usrTelnetcStart(void);
#endif

#ifdef INCLUDE_SNTPC
LOCAL STATUS usrSntpcInit (void);
#endif

#ifdef INCLUDE_SNTPS
LOCAL STATUS usrSntpsInit (void);
#endif

#ifdef INCLUDE_DHCPR
STATUS usrDhcprInit (void);
#endif 

#ifdef INCLUDE_DHCPS
STATUS usrDhcpsInit (void);
#endif

#ifdef INCLUDE_DHCPC
    void usrDhcpcStart (void);
#endif 

#ifdef INCLUDE_DHCPC_LEASE_CLEAN
    void usrDhcpcLeaseClean(void);
#endif 

#ifdef INCLUDE_DHCPC_LEASE_SAVE
void   usrDhcpcLeaseSave (void);
#endif

#ifdef INCLUDE_DHCPC_LEASE_TEST
void   usrDhcpcLeaseTest (void);
#endif
    
#ifdef INCLUDE_DHCPC_LEASE_GET
void   usrDhcpcLeaseGet (void);
#endif

#if defined(INCLUDE_NETWORK) && !defined(INCLUDE_NET_MICRO_STACK)
STATUS  usrNetworkInit ();         /* Initialize tables for network stack */
IMPORT STATUS netCoreSetup (UINT, UINT, UINT, UINT);
#endif

#ifdef INCLUDE_ROUTE_STORAGE
void usrNetRouteStorageInit (void);
#endif

#ifdef INCLUDE_MUX_L2
STATUS usrNetMuxL2Init();
#endif

#ifdef INCLUDE_QOS_INGRESS
STATUS usrNetQosIngressLibInit (void);
#endif /* INCLUDE_QOS_INGRESS */

#ifdef INCLUDE_QOS_INGRESS_HOOKS
IMPORT STATUS qosIngressHooksInit(void);
#endif /* INCLUDE_QOS_INGRESS_HOOKS */

#ifdef INCLUDE_BPF 	
STATUS usrNetBpfDrvInit (void);
#endif     

#ifdef INCLUDE_TUNNELLIB
STATUS usrNetTunnelLibInit (void);
#endif

#ifdef INCLUDE_TUNNEL_MUX_REG
STATUS usrNetTunnelMuxReg (void);
#endif

#ifdef INCLUDE_MIB2_IF
#ifdef VIRTUAL_STACK
VS_REG_ID m2IfRegistrationNum;
extern STATUS m2IfDestructor (VSNUM vsnum);
#endif /* VIRTUAL_STACK */

STATUS usrNetM2IfInit (void); /* MIB2 Variables for Interface Layer */
#endif

#ifdef INCLUDE_MIB2_IP
#ifdef VIRTUAL_STACK
VS_REG_ID m2IpRegistrationNum;
#endif /* VIRTUAL_STACK */

LOCAL STATUS usrNetM2IpInit (void); /* MIB2 Variables for IP Layer */
#endif

#ifdef INCLUDE_MIB2_IPV6
#ifdef VIRTUAL_STACK
VS_REG_ID m2Ipv6IfRegistrationNum;
extern STATUS m2Ipv6IfDestructor (VSNUM vsnum);
#endif /* VIRTUAL_STACK */

LOCAL STATUS usrNetM2Ipv6IfInit (); /* MIB2 Variables for IP6 Layer */
#endif

#ifdef INCLUDE_MIB2_SYSTEM
#ifdef VIRTUAL_STACK
VS_REG_ID m2SysRegistrationNum;
extern STATUS m2SysDestructor (VSNUM vsnum);
#endif /* VIRTUAL_STACK */

LOCAL STATUS usrNetM2SysInit (void); /* MIB2 Variables for System */
#endif

#ifdef INCLUDE_RLOGIN
STATUS usrRlogindStart (void);      /* Allows remote access to target */
#endif    

#ifdef INCLUDE_PROXY_SERVER
STATUS usrProxyServerInit (void);
#endif

#ifdef INCLUDE_RARP 
STATUS usrNetRarpInit (void);
#endif

#ifdef INCLUDE_FASTPATH
STATUS usrNetFFInit (void);
#endif

#ifdef INCLUDE_FASTUDP
void usrNetFastUdpInit (void);
#endif

#ifdef INCLUDE_FASTUDP6
void usrNetFastUdp6Init (void);
#endif

#ifdef INCLUDE_VIRTUAL_STACK   
STATUS usrVirtualStackInit (void);  

STATUS usrVirtualStackCoreInit (void);

STATUS usrVirtualStackAppInit (void);
#endif /* INCLUDE_VIRTUAL_STACK */

#ifdef INCLUDE_ICMPLIB
void usrNetIcmpUtilInit (void);         /* Initialize the ICMP show facility */
#endif

#ifdef INCLUDE_APPL_LOG_UTIL
#include <applUtilLib.h>
#include "../comps/src/net/coreip/usrNetApplUtil.c"
#endif

#ifdef INCLUDE_VXWORKS_5_X_EQUIV_PPP
#include "../comps/src/net/ppp/usrBasicPPPFrameworkInit.c"
#endif /* INCLUDE_VXWORKS_5_X_EQUIV_PPP */


#ifdef INCLUDE_TIPC
#ifdef INCLUDE_TIPC_MEDIA_SM
#include <tipc/sm_media.h>
#include <smUtilLib.h>
#include "../../config/comps/src/usrTipcSm.c"
#endif
#ifdef INCLUDE_TIPC_SOCKET
#include "../../config/comps/src/tipc/usrTipcSocket.c"
#endif

#ifdef INCLUDE_TIPC_MEMPOOL
#include "../../config/comps/src/tipc/usrTipcPool.c"
#endif
#endif /* INCLUDE_TIPC */

#if defined (INCLUDE_NET_ROUTE_SHOW)

#if !defined (INCLUDE_NET_ROUTE_SHOW_SNPRINTF) && \
    !defined (INCLUDE_NET_ROUTE_SHOW_NBIO) && \
    !defined (INCLUDE_NET_ROUTE_SHOW_UNBUF)
#define INCLUDE_NET_ROUTE_SHOW_SNPRINTF
#endif

#if defined (INCLUDE_NET_ROUTE_SHOW_SNPRINTF)
#include <stdarg.h>
#elif !defined (INCLUDE_NET_ROUTE_SHOW_UNBUF)
#include <private/nbioLogLibP.h>
#endif /* !defined (INCLUDE_NET_ROUTE_SHOW_UNBUF) */

#include <../comps/src/net/coreip/usrNetRouteShow.c>

#endif /* INCLUDE_NET_ROUTE_SHOW */


/* code */

/*****************************************************************************
*
* usrNetHostSetup - initialize local host table and lookup methods
*
* RETURNS: N/A
*/

void usrNetHostSetup (void)
    {
#ifdef INCLUDE_HOST_TBL
#ifdef VIRTUAL_STACK
    /*
     * Register with vsmanager to allow the setup of the internal
     * host table for any virtual stack at the appropriate time.
     */

    vsComponentRegister (&hostRegistrationNum, "HOST",
                         VS_SC_ENABLED, VS_SC_OPTIONAL,
                         usrNetHostTblSetup, 0, NULL, hostDestructor);
#else
    /*
     * When only one network stack exists, just add the local and
     * remote hostnames to the internal host table immediately.
     */

    usrNetHostTblSetup ();
#endif /* VIRTUAL_STACK */
#endif

#ifdef INCLUDE_GETADDRINFO
    gethostbynameInit (IGNORE_LINKLOCAL);
#endif

#ifdef INCLUDE_GETNAMEINFO
    gethostbyaddrInit();

#ifdef INCLUDE_RTP
#ifdef VIRTUAL_STACK
    /*
     * Register with vsmanager to allow the setup of the getnameinfo
     * component for any virtual stack at the appropriate time.
     */

    vsComponentRegister (&rtpgetnameRegistrationNum, "RTP getname",
                         VS_SC_ENABLED, VS_SC_OPTIONAL,
                         rtpGetnameinfoInit, 0, NULL, NULL);
#else
    /*
     * When only one network stack exists, just setup the getnameinfo
     * sysctl node immediately
     */

    rtpGetnameinfoInit ();
#endif /* VIRTUAL_STACK */
#endif /* INCLUDE_RTP */
#endif  /* INCLUDE_GETNAMEINFO */

#ifdef INCLUDE_NET_HOST_SETUP
#ifdef VIRTUAL_STACK
    /*
     * Register with vsmanager to allow the assignment of the
     * local hostname for any virtual stack at the appropriate time.
     */

    vsComponentRegister (&hostnameRegistrationNum, "HOSTNAME SETUP",
                         VS_SC_ENABLED, VS_SC_NON_OPTIONAL,
                         usrNetHostAdd, 0, NULL, hostnameDestroy);
#else
    /*
     * When only one network stack exists, just assign the
     * local hostname immediately.
     */

    usrNetHostAdd (); 
#endif /* VIRTUAL_STACK */
#endif
    }

/******************************************************************************
*
* usrNetSocketInit - Initialize Socket Layer
*
* RETURNS:N/A
*/

void usrNetSocketInit (void)
    {
#ifdef INCLUDE_SOCKLIB
    sockLibInit ();                     /* Socket API */
#endif /* INCLUDE_SOCKLIB */

#ifdef INCLUDE_SC_SOCKLIB
    sockScLibInit ();                   /* Socket API system call support */
#endif

#ifdef INCLUDE_BSD_SOCKET
    usrNetBsdSocket(); /* Network components for BSD socket */
#endif /* INCLUDE_BSD_SOCKET */

#ifdef INCLUDE_DSI_SOCKET
    usrDsiSocket ();
#endif /* INCLUDE_DSI_SOCKET */

#ifdef INCLUDE_ZBUF_SOCK 
    zbufSockLibInit ();
#endif /* INCLUDE_ZBUF_SOCK */
    }

void usrNetMib2Init (void)
    {
#ifdef INCLUDE_MIB2_ALL
    m2LibInit ();
#endif

#ifdef INCLUDE_MIB2_IF
    m2If64BitCounters = MIB2IF_LONG_COUNTERS;

#ifdef VIRTUAL_STACK
    /*
     * Register with vsmanager to allow the MIB2 Interface Layer
     * to start in every virtual stack at the appropriate time.
     */

    vsComponentRegister (&m2IfRegistrationNum, "MIB2_IF",
                         VS_SC_ENABLED, VS_SC_NON_OPTIONAL,
                         usrNetM2IfInit, 0, NULL, m2IfDestructor);
#else
    /*
     * When only one network stack exists, just
     * start the MIB2 Interface Layer immediately.
     */

    usrNetM2IfInit (); /* MIB2 Variables for Interface Layer */
#endif
#endif

#ifdef INCLUDE_MIB2_IFMGT
    m2IfMgtLibInit();
#endif

#ifdef INCLUDE_MIB2_SYSTEM
#ifdef VIRTUAL_STACK
    /*
     * Register with vsmanager to allow the MIB2 System group
     * to start in every virtual stack at the appropriate time.
     */

    vsComponentRegister (&m2SysRegistrationNum, "MIB2_SYS",
                         VS_SC_ENABLED, VS_SC_NON_OPTIONAL,
                         usrNetM2SysInit, 0, NULL, m2SysDestructor);
#else
    /*
     * When only one network stack exists, just
     * start the MIB2 System group immediately.
     */

    usrNetM2SysInit ();

#endif /* VIRTUAL_STACK */
#endif

#ifdef INCLUDE_MIB2_IP
#ifdef VIRTUAL_STACK
    /*
     * Register with vsmanager to allow the MIB2 IP Layer
     * to start in every virtual stack at the appropriate time.
     */

    vsComponentRegister (&m2IpRegistrationNum, "MIB2_IP",
                         VS_SC_ENABLED, VS_SC_NON_OPTIONAL,
                         usrNetM2IpInit, 0, NULL, NULL);
#else
    /*
     * When only one network stack exists, just
     * start the MIB2 IP Layer immediately.
     */

    usrNetM2IpInit ();

#endif /* VIRTUAL_STACK */
#endif

#ifdef INCLUDE_MIB2_UDP
    m2UdpInit ();
#endif

#ifdef INCLUDE_MIB2_TCP
    m2TcpInit ();
#endif
#ifdef INCLUDE_MIB2_ICMP
    m2IcmpInit ();
#endif

#ifdef INCLUDE_MIB2_IPV6ALL
    m2Ipv6LibInit ();
#endif

#ifdef INCLUDE_MIB2_IPV6
#ifdef VIRTUAL_STACK
    /*
     * Register with vsmanager to allow the MIB2 IPv6 Layer
     * to start in every virtual stack at the appropriate time.
     */

    vsComponentRegister (&m2IpRegistrationNum, "MIB2_IPV6",
                         VS_SC_ENABLED, VS_SC_NON_OPTIONAL,
                         usrNetM2Ipv6IfInit, 0, NULL, m2Ipv6IfDestructor);
#else
    /*
     * When only one network stack exists, just
     * start the MIB2 IPv6 Layer immediately.
     */

    usrNetM2Ipv6IfInit ();

#endif /* VIRTUAL_STACK */
#endif

#ifdef INCLUDE_MIB2_UDPV6
    m2Ipv6UdpInit ();
#endif

#ifdef INCLUDE_MIB2_TCPV6
    m2Ipv6TcpInit ();
#endif
#ifdef INCLUDE_MIB2_ICMPV6
    m2Ipv6IfIcmpInit ();
#endif
#ifdef INCLUDE_MIB2_MLD
    m2Ipv6MldInit ();
#endif    
#ifdef INCLUDE_CIDR_METHODS
    rfc2096init ();
#endif    
    }

/******************************************************************************
*
* usrNetProtoInit - Initialize the network protocol stacks
*
* RETURNS:N/A
*/

void usrNetProtoInit (void)
    {
    usrNetIfInit ();

#ifdef INCLUDE_IFCLONE
    if_clone_init ();
#endif /* INCLUDE_IFCLONE */

#ifdef INCLUDE_SUBNET_VLAN
    vlanIfLibInit();
#endif /* INCLUDE_SUBNET_VLAN */

#ifdef INCLUDE_IPV4
    usrNetIpv4Init ();                  /* IP v4 */
#endif /* INCLUDE_IPV4 */

#ifdef INCLUDE_ICMPV4
    usrNetIcmpv4Init ();                /* ICMP v4 */
#endif

#ifdef INCLUDE_UDPV4
    usrNetUdpv4Init ();                 /* UDP v4 */
#endif

#ifdef INCLUDE_UN
    usrUnInit ();                    /* AF_LOCAL domain (Unix) */
#ifdef INCLUDE_UN_SHOW
    unShowInit ();
#endif
#endif

#ifdef INCLUDE_UN_COMP
    usrUnCompInit ();                /* AF_LOCAL/COMP */
#ifdef INCLUDE_UN_COMP_SHOW
    uncompShowInit ();
#endif
#endif

#ifdef INCLUDE_TCPV4
    usrNetTcpv4Init ();                 /* TCP v4 */
#endif

#ifdef INCLUDE_TCP_DEBUG
    usrNetTcpv4DebugInit ();           /* TCP v4 debug */
#endif

#ifdef INCLUDE_RAWV4
    usrNetRawv4Init ();                 /* RAW v4 */
#endif
               
#ifdef INCLUDE_IGMP
    usrNetIgmpInit ();                  /* IGMP */
#endif

#ifdef INCLUDE_MROUTE
    usrNetIpMrouteInit ();              /* Multicast routing */
#endif

#ifdef INCLUDE_ROUTING_SOCKET
    usrNetRtSockInit ();                /* XXX: rename accurately */
#endif /* ROUTING_SOCKET */

#ifdef INCLUDE_IPV6
    usrNetIpv6Init ();                  /* IP v6 */
#endif 

#ifdef INCLUDE_MIPV6
    usrNetMipv6Init ();                  /* Mobility v6 */
#endif /* INCLUDE_MIPV6 */

#ifdef INCLUDE_ICMPV6
    usrNetIcmpv6Init ();                /* ICMP v6 */
#endif

#ifdef INCLUDE_MLD
    usrNetMldInit ();                      /* MLD */
#endif

#ifdef INCLUDE_UDPV6
    usrNetUdpv6Init ();                 /* UDP v6 */
#endif

#ifdef INCLUDE_TCPV6
    usrNetTcpv6Init ();                 /* TCP v6 */
#endif

#ifdef INCLUDE_SCTP                     /* SCTP */
    usrNetSctpInit ();
    usrNetworkSymbols [1] = (VOIDFUNCPTR) sctp_bindx;
#endif

#ifdef INCLUDE_MROUTEV6
    usrNetIpMroutev6Init ();           /* Multicast routing v6 */
#endif /* INCLUDE_MROUTEV6 */

#ifdef INCLUDE_ND
    usrNetNdInit ();                    /* Neigbour Discovery */
#endif

#ifdef INCLUDE_IPV6_FW
    usrNetIpv6FwInit ();
#endif

#ifdef INCLUDE_TUNNELLIB
    usrNetTunnelLibInit ();
#endif


#ifdef INCLUDE_RARP 
    usrNetRarpInit ();
#endif

    usrNetMib2Init();
    
#ifdef INCLUDE_FASTUDP
    usrNetFastUdpInit ();
#endif

#ifdef INCLUDE_FASTUDP6
    usrNetFastUdp6Init ();
#endif
    }


/******************************************************************************
*
* usrNetRemoteInit -
*
*  RETURNS:N/A
*/

void usrNetRemoteInit (void)
    {
#ifdef INCLUDE_REMLIB
    remLibInit(RSH_STDERR_SETUP_TIMEOUT); /* Remote Command Library */
#ifdef INCLUDE_NET_SYSCTL
    remLibSysctlInit();
#endif
#endif 

    /* Allows access to file system on boot host */
#ifdef INCLUDE_NET_REM_IO
    usrNetRemoteCreate ();              /* Allows access to file system on boot host */
#endif /*  INCLUDE_NET_REM_IO */
    }


/******************************************************************************
*
* usrNetUtilsInit - Initialize network utility routines
*
* RETURNS: N/A
*/

void usrNetUtilsInit (void)
    {
#ifdef INCLUDE_L2CONFIG
    l2configInit ();          /* l2config */
#endif /* INCLUDE_L2CONFIG */

#ifdef INCLUDE_IFCONFIG
    usrNetIfconfigInit ();              /* ifconfig */
#endif /* INCLUDE_IFCONFIG */

#ifdef INCLUDE_IFCONFIG_PREFIX
    prefixLibInit ();
#endif /* INCLUDE_IFCONFIG_PREFIX */
  
#ifdef INCLUDE_ARP_API
    usrNetArpLibInit ();                /* Address Resolution Protocol (ARP) table manipulation library  */
#endif  /*INCLUDE_ARP_API */

#ifdef INCLUDE_IFLIB
    ifLibInit(); /* Network interface library */
#endif /* INCLUDE_IFLIB */
    
#ifdef INCLUDE_IF6LIB
    if6LibInit();
#endif /* INCLUDE_IF6LIB */
    
#ifdef INCLUDE_NDP
    usrNetNdpInit ();                   /* ndp */
#endif /* INCLUDE_NDP */
    
#ifdef INCLUDE_ADDRCTL_POLICY
    usrNetAddrCtlInit ();               /* address selection */
#endif /* INCLUDE_ADDRCTL_POLICY */
    
#ifdef INCLUDE_OLDROUTE                 /* routeAdd, mRouteAdd etc. */
    oldRouteLibInit ();
#endif

#ifdef INCLUDE_ROUTECMD
    usrNetRouteCmdInit ();              /* routec command */
#endif /* INCLUDE_ROUTECMD */

#ifdef  INCLUDE_NETSTAT_IF   
    netstatIfInit ();
#endif /* INCLUDE_NETSTAT_IF */

#ifdef  INCLUDE_NETSTAT_IPV4   
    netstatIpInit ();
#endif /* INCLUDE_NETSTAT_IPV4 */

#ifdef  INCLUDE_NETSTAT_IPV6   
    netstatIp6Init ();
#endif /* INCLUDE_NETSTAT_IPV6 */

#ifdef  INCLUDE_NETSTAT_TCP   
    netstatTcpInit ();
#endif /* INCLUDE_NETSTAT_TCP */

#ifdef  INCLUDE_NETSTAT_UDP   
    netstatUdpInit ();
#endif /* INCLUDE_NETSTAT_UDP */

#ifdef  INCLUDE_NETSTAT_RAWV6   
    netstatRip6Init ();
#endif /* INCLUDE_NETSTAT_RAWV6 */

#ifdef  INCLUDE_NETSTAT_ICMPV4   
    netstatIcmpInit ();
#endif /* INCLUDE_NETSTAT_ICMPV4 */

#ifdef  INCLUDE_NETSTAT_ICMPV6   
    netstatIcmp6Init ();
#endif /* INCLUDE_NETSTAT_ICMPV6 */

#ifdef  INCLUDE_NETSTAT_ROUTE   
    netstatRouteInit ();
#endif /* INCLUDE_NETSTAT_ROUTE */

#ifdef  INCLUDE_NETSTAT_IGMP   
    netstatIgmpInit ();
#endif /* INCLUDE_NETSTAT_IGMP */

#ifdef  INCLUDE_NETSTAT_SCTP   
    netstatSctpInit ();
#endif /* INCLUDE_NETSTAT_SCTP */

#ifdef  INCLUDE_NETSTAT_MROUTE   
    netstatMrouteInit ();
#endif /* INCLUDE_NETSTAT_MROUTE */

#ifdef  INCLUDE_NETSTAT_MROUTEV6   
    netstatMroute6Init ();
#endif /* INCLUDE_NETSTAT_MROUTEV6 */

#ifdef  INCLUDE_NETSTAT_UN_COMP
    netstatUncompInit ();
#endif /* INCLUDE_NETSTAT_UN_COMP */

#ifdef INCLUDE_NET_SHOW
    netShowInit();                      /* Initialize the IP show facility */
#endif /* INCLUDE_NET_SHOW */

#ifdef INCLUDE_NET_IF_SHOW
    ifShowInit();                   /* Initialize the interface show facility */
#endif /* INCLUDE_NET_IF_SHOW */

#ifdef INCLUDE_ICMPLIB
#ifdef VIRTUAL_STACK
    /*
     * Register with vsmanager to allow the ICMP utilities
     * to start in every virtual stack at the appropriate time.
     */

    vsComponentRegister (&icmpUtilRegistrationNum, "ICMP UTIL",
                         VS_SC_ENABLED, VS_SC_OPTIONAL,
                         icmpUtilLibInit, 0, NULL, icmpUtilDestructor);
#else
    /*
     * When only one network stack exists, just
     * setup the ICMP utilites immediately.
     */

    icmpUtilLibInit ();
#endif /* VIRTUAL_STACK */
#endif

#ifdef INCLUDE_NET_ROUTE_SHOW
    routeShowInit();                    /* Initialize the route show facility */
#endif /* INCLUDE_NET_ROUTE_SHOW */

#ifdef INCLUDE_NET_HOST_SHOW
    hostShowInit();                /* Initialize the host table show facility */
#endif /* INCLUDE_NET_HOST_SHOW */

#ifdef  INCLUDE_ICMP_SHOW
    icmpShowInit();                     /* Initialize the ICMP show facility */
#endif /*  INCLUDE_ICMP_SHOW */

#ifdef INCLUDE_IGMP_SHOW    
    igmpShowInit();                     /* Initialize the IGMP show facility */
#endif /* INCLUDE_IGMP_SHOW    */

#ifdef INCLUDE_TCP_SHOW
#ifdef VIRTUAL_STACK
    /*
     * Register with vsmanager to allow the TCP show routines
     * to start in every virtual stack at the appropriate time.
     */

    vsApplicationRegister (&tcpShowRegistrationNum,
                           "TCPSHOW", VS_SC_ENABLED,
                           tcpShowInit, 0, NULL, NULL);
#else
    /*
     * When only one network stack exists, just
     * start the TCP show routines immediately.
     */

    tcpShowInit ();
#endif
#endif /* INCLUDE_TCP_SHOW */

#ifdef INCLUDE_UDP_SHOW
#ifdef VIRTUAL_STACK
    /*
     * Register with vsmanager to allow the UDP show routines
     * to start in every virtual stack at the appropriate time.
     */

    vsApplicationRegister (&udpShowRegistrationNum,
                           "UDPSHOW", VS_SC_ENABLED,
                           udpShowInit, 0, NULL, NULL);
#else
    /*
     * When only one network stack exists, just
     * start the UDP show routines immediately.
     */

    udpShowInit ();
#endif
#endif /* INCLUDE_UDP_SHOW */

#ifdef INCLUDE_PREFIXCMD
    prefixCmdInit();
#endif /* INCLUDE_PREFIXCMD */
 
#ifdef INCLUDE_INETLIB
#ifdef INCLUDE_NET_SYSCTL
    inetLibSysctlInit();
#endif
#endif /* INCLUDE_INETLIB */

#ifdef INCLUDE_IGMP_ROUTER
#ifdef VIRTUAL_STACK
    /*
     * Register with vsmanager to allow the ICMP utilities
     * to start in every virtual stack at the appropriate time.
     */

    vsApplicationRegister (&igmpRouterRegistrationNum, "IGMP ROUTER",
                           VS_SC_ENABLED, igmpRouterLibInit,
                           0, NULL, igmpRouterDestructor);

#else
    /*
     * When only one network stack exists, just
     * setup the ICMP utilites immediately.
     */

    igmpRouterLibInit ();
#endif /* VIRTUAL_STACK */
#ifdef INCLUDE_MIB2_IGMP
    m2IgmpInit (); 
#ifdef INCLUDE_WINDMANAGE_SNMP_CORE
    if (m2IgmpTreeAdd () == ERROR)
        NET_DIAG(("Unable to add MIB fragment for IGMP to tree\n"));
#endif   /* INCLUDE_WINDMANAGE_SNMP_CORE */
#endif   /* INCLUDE_MIB2_IGMP */
#endif   /* INCLUDE_IGMP_ROUTER */


    }

/******************************************************************************
*
* usrNetAddrInit - initialize the network address for a device
*
* RETURNS: N/A
*/

void usrNetAddrInit (void)
    {
#ifdef INCLUDE_DHCPC_LEASE_GET
    usrDhcpcLeaseGet ();
#endif
    
#ifdef INCLUDE_DHCPC_LEASE_CLEAN
    usrDhcpcLeaseClean ();
#endif
    }

/******************************************************************************
*
* usrNetAddrCheck - verify the IP address if assigned dynamically
*
* RETURNS: N/A
*/

void usrNetAddrCheck (void)
    {
#ifdef INCLUDE_DHCPC
    usrDhcpcStart();
#endif

#ifdef INCLUDE_DHCPC_LEASE_TEST
    usrDhcpcLeaseTest ();
#endif

#ifdef INCLUDE_DHCPC_LEASE_SAVE
    usrDhcpcLeaseSave ();
#endif
    }

#ifdef INCLUDE_NETMASK_GET
/*******************************************************************************
*
* usrNetmaskGet - remove DHCP timestamp information from address
*
* This routine stores any value for the netmask found in the client 
* address string in the bootline, if needed. It is the initialization
* routine for the INCLUDE_NETMASK_GET component which is automatically
* added when the INCLUDE_NET_INIT component is used. The netmask
* value is not stored if further processing was disabled by an earlier 
* routine.
*
* RETURNS: N/A
*
* NOMANUAL
*/

void usrNetmaskGet (void)
    {

    /* 
     * Extract the netmask if processing is not complete. The flag will be
     * set to disable further processing if an error is detected in the
     * address string. It will also be set if the PPP or SLIP device have 
     * been created, since those devices handle any netmask value 
     * independently.
     */

    if (!netDevBootFlag)
        {
        netmask = 0;
        bootNetmaskExtract (pAddrString, &netmask);
        }
    }
#endif /* INCLUDE_NETMASK_GET */


/******************************************************************************
*
* usrNetDevStart - Network device start
*
* RETURNS: N/A
*/

void usrNetDevStart (void)
    {
#ifdef INCLUDE_ETHERNET
    usrNetEther (); /* Ethernet interface support */
#endif /*INCLUDE_ETHERNET */

#ifdef INCLUDE_END
    usrNetEndLibInit();                 /* Support for network devices using MUX/END interface */
#endif  /* INCLUDE_END */

#ifdef INCLUDE_BPF
    usrNetBpfDrvInit ();                /* Berkeley Packet Filter Support */
#endif 
 
#ifdef INCLUDE_TUNNEL_MUX_REG
    usrNetTunnelMuxReg ();
#endif

#ifdef INCLUDE_STF
    stfInit ();
#endif

#ifdef INCLUDE_GIF
    gifInit ();
#endif

#ifdef INCLUDE_VXWORKS_5_X_EQUIV_PPP
    usrBasicPPPFrameworkInit ();
#endif

    if (strncmp (sysBootParams.bootDev, "ppp", 3) == 0)
        {
        /* booting via ppp */
#ifdef INCLUDE_VXWORKS_5_X_EQUIV_PPP

        printf ("Attaching network interface ppp...\n");

        pppBootHandle = usrWindNetPPPInit (sysBootParams.bootDev,
                                           sysBootParams.ead,
                                           ((sysBootParams.gad[0] == EOS)?
                                            sysBootParams.had:
                                            sysBootParams.gad));
        if (pppBootHandle == NULL)
            return;

        rebootHookAdd ((FUNCPTR) pppRebootHook);
#else
        printf ("\nError: PPP not included.\n");
        return;
#endif /* INCLUDE_VXWORKS_5_X_EQUIV_PPP */
        }
    else
        {
#ifdef INCLUDE_BOOT_LINE_INIT
	if (pDevName != NULL)
	    {
#ifdef INCLUDE_IPATTACH	
	    /* Attach boot END device to IPv4 */
	    usrNetIpAttachCommon (pDevName, uNum,"IPv4",ipAttach);
#endif /* INCLUDE_IPATTACH */

#ifdef INCLUDE_IP6ATTACH
	    /* Attach boot END device to IPv6 */
	    usrNetIpv6Attach (pDevName, uNum);
#endif /* INCLUDE_IP6ATTACH */

#ifdef INCLUDE_NET_BOOT_CONFIG
    diskBoot = FALSE;

    /* find network boot device configuration */
    if ( (strncmp (sysBootParams.bootDev, "scsi", 4) == 0) ||
        (strncmp (sysBootParams.bootDev, "ide", 3) == 0) ||
        (strncmp (sysBootParams.bootDev, "ata", 3) == 0) ||
        (strncmp (sysBootParams.bootDev, "fd", 2) == 0)  ||
        (strncmp (sysBootParams.bootDev, "tffs", 4) == 0) )
          diskBoot = TRUE;

    /*
     * If we are booting from disk, then the target IP address
     * string need not be present. Make sure DHCP client option
     * is activated 
     */

    if ((diskBoot) && (sysBootParams.flags & SYSFLG_AUTOCONFIG)) 
        {
        if((pAddrString != NULL) && (pAddrString[0] != EOS))
	    usrNetBootConfig (pDevName, uNum, pAddrString,
			      netmask, sysBootParams.gad);
        }
    else
        usrNetBootConfig (pDevName, uNum, pAddrString,
                              netmask, sysBootParams.gad);
#endif
            }
#endif /* INCLUDE_BOOT_LINE_INIT */
        }

#ifdef INCLUDE_SM_NET
    if (!backplaneBoot)
        {
        int bpmask = 0;
        END_OBJ*   pEnd;

        pEnd = endFindByName ("sm", 0);
        if (pEnd == NULL)
	    {
            printf ("Can not find the shared memory END device.");
            printf (" Check address assignments for SM master.\n");
            }
        else 
	    {
#ifdef INCLUDE_IPATTACH  
            /* Attach shared memory END device to IPv4 */

            usrNetIpAttachCommon ("sm", 0, "IPv4", ipAttach);
#endif /* INCLUDE_IPATTACH */

#ifdef INCLUDE_IP6ATTACH
            /* Attach shared memory END device to IPv6 */

            usrNetIpAttachCommon ("sm", 0,"IPv6", ip6Attach);

#if (IFF_INET6_UP_CFG > 0)
            do_autoconfig ("sm", 0);
#endif
#endif /* INCLUDE_IP6ATTACH */
            }

        /* 
         * The load routine for the shared memory END device determines
         * an IP address for the shared memory master using the boot
         * parameter values. If none is available, the load fails and
         * the previous endFindByName() call will prevent this configuration
         * attempt.
         *
         * The shared memory slaves do not require an address from the boot
         * parameters. If the backplane address is not given, those devices
         * will retrieve an address from the shared memory master.
         */ 

        if (sysBootParams.bad[0] == EOS)
            { 
            if (smEndInetAddrGet ("sm", 0, NONE, sysBootParams.bad) != OK)
                printf ("Error: sm0 addressing is not setup correctly\n");
            }
        else
            {
            bootNetmaskExtract (sysBootParams.bad, &bpmask);
            }

        /* SM configuration with boot parameters */

        usrNetBootConfig ("sm", 0, sysBootParams.bad,
                          bpmask, sysBootParams.gad); 
        }

#ifdef INCLUDE_SM_NET_SHOW
    smEndShowInit ();
#endif /* INCLUDE_SM_NET_SHOW */
#endif /* INCLUDE_SM_NET */

#ifdef INCLUDE_ADDIF 
    addifInit (ADDIF_NUM, ADDIF_NAME, ADDIF_ADDR, ADDIF_MASK);
#endif

#ifdef _WRS_ARCH_IS_SIMULATOR
	/* configure VxSim command line additional devices */
	sysNetIfConfig ();
#endif /* _WRS_ARCH_IS_SIMULATOR */
    }

/******************************************************************************
*
* usrNetAppInit - Intializes the network application modules
*  
*/
void usrNetAppInit (void)
    {
#ifdef INCLUDE_RPC
    if (usrNetRpcInit () != OK)
	printf ("RPC initialization failed\n");  /* Remote Procedure Call system */
#endif /* INCLUDE_RPC */
    
#ifdef INCLUDE_AIP
    usrNetAutoIpInit ();    /* IPV4 address auto configuration */
#endif /* INCLUDE_AIP */

#ifdef INCLUDE_TELNET
    usrTelnetdStart ();
#endif  /* INCLUDE_TELNET */

#ifdef INCLUDE_TELNET_CLIENT
    usrTelnetcStart ();
#endif  /* INCLUDE_TELNET_CLIENT */

#ifdef INCLUDE_SNTPC
    usrSntpcInit ();
#endif

#ifdef INCLUDE_SNTPS
    usrSntpsInit ();
#endif

#ifdef INCLUDE_SECURITY
    usrSecurity();
#endif

#ifdef INCLUDE_TFTP_CLIENT
   /*
    * Add a symbolic reference to link the client code,
    * since the initialization routine is not called, and
    * no other direct reference exists in this startup code.
    */

    usrNetworkSymbols[0] = (VOIDFUNCPTR)tftpCopy;
#endif /* INCLUDE_TFTP_CLIENT */

#ifdef INCLUDE_TFTP_SERVER

#ifdef VIRTUAL_STACK
    /*
     * Register at vsmanager to allow the tftp server to start in a 
     * chosen virtual stack; it can only run in one stack.
     */

    if (vsApplicationRegister (&tftpdRegistrationNum, "tftpd",
                               VS_SC_DISABLED, usrTftpdInit,
                               0, NULL, tftpdDestructor) == VS_OK)
	{
#if (TFTP_SERVER_IN_VS0 == TRUE)   /* Startup in initial network stack. */
        vsComponentStatusSet (0, tftpdRegistrationNum, VS_SC_ENABLED, NULL);
#endif
        }
#else
    /*
     * When only one network stack exists, just
     * start the TFTP server immediately.
     */

    usrTftpdInit ();

#endif /* VIRTUAL_STACK */

#endif /* INCLUDE_TFTP_SERVER */ 

#ifdef INCLUDE_FTP6_SERVER
    usrFtpd6Start (); /* File Transfer Protocol6 Server Library and Task */
#endif /* INCLUDE_FTP6_SERVER */

#ifdef INCLUDE_FTP

#ifdef VIRTUAL_STACK
    /*
     * Register at vsmanager to allow ftp to be started in a 
     * chosen virtual stack; it can only be started in one stack.
     */

    ftpvsnum = -1;    /* Currently not active in any stack. */

    if (vsApplicationRegister (&ftpRegistrationNum, "ftpClient",
                               VS_SC_DISABLED, usrFtpInit,
                               0, NULL, NULL) == VS_OK)
	{
#if (FTP_CLIENT_IN_VS0 == TRUE)   /* it must be enabled for instance 0 */
        vsComponentStatusSet (0, ftpRegistrationNum, VS_SC_ENABLED, NULL);
#endif
        }
#else
    /*
     * When only one network stack exists, just
     * start the FTP client immediately.
     */

    usrFtpInit ();

#endif /* VIRTUAL_STACK */
#endif /* INCLUDE_FTP */

#ifdef INCLUDE_FTP6
    ftp6LibInit (FTP6_REPLYTIMEOUT);
#endif /* INCLUDE_FTP6 */


    /* initialize NFS server if specified */
#if (defined (INCLUDE_NFS_SERVER_ALL) || defined (INCLUDE_NFS2_SERVER) \
    || defined (INCLUDE_NFS3_SERVER))
    usrNfsdStart ();
#endif

#if (defined (INCLUDE_NFS2_CLIENT))
    usrNfsInit(NFS_V2);
#if VX_VERSION == 64
    usrNfs2CacheInit (NFS2_CLIENT_CACHE_DEFAULT_NUM_LINES,
                      NFS2_CLIENT_CACHE_DEFAULT_LINE_SIZE,
                      NFS2_CLIENT_CACHE_DEFAULT_OPTIONS);
#endif /* VX_VERSION == 64 */
#endif /* INCLUDE_NFS2_CLIENT) */

#if (defined (INCLUDE_NFS3_CLIENT))
    usrNfsInit(NFS_V3);
#if VX_VERSION == 64
    usrNfs3CacheInit (NFS3_CLIENT_CACHE_DEFAULT_NUM_LINES,
                      NFS3_CLIENT_CACHE_DEFAULT_LINE_SIZE,
                      NFS3_CLIENT_CACHE_DEFAULT_OPTIONS);
#endif /* VX_VERSION == 64 */
#endif /* INCLUDE_NFS3_CLIENT) */

#if (defined (INCLUDE_NFS_CLIENT_ALL))
    usrNfsInit (NFS_V2);
    usrNfsInit (NFS_V3);
#endif

#if defined (INCLUDE_NFS_MOUNT_ALL)
    usrNetNfsMountAll();
#endif

#ifdef INCLUDE_DHCPC_SHOW
    dhcpcShowInit();
#endif 
    
#ifdef INCLUDE_DHCPC
    usrDhcpcStart ();
#endif

#ifdef INCLUDE_DHCPS
    usrDhcpsInit (); /* Dynamic host configuration protocol server */
#endif

#ifdef INCLUDE_DHCPR
    usrDhcprInit ();
#endif
    
#ifdef INCLUDE_PING
    pingLibInit();                      /* ICMP (ping) client */
#endif /* INCLUDE_PING */
#ifdef INCLUDE_PING6
    ping6LibInit();                     /* ICMP (ping6) client */
#endif /* INCLUDE_PING6 */
    
#ifdef   INCLUDE_DNS_RESOLVER 
    usrDnsInit ();                      /* Domain name service client */
#endif /* INCLUDE_DNS_RESOLVER */

#ifdef INCLUDE_RLOGIN
    usrRlogindStart ();                 /* Allows remote access to target */
#endif    

#ifdef INCLUDE_RDISC
    usrRdiscInit (RDISC_PRIORITY, RDISC_OPTIONS, RDISC_STACKSIZE);
#endif


#ifdef INCLUDE_MIP6_MN

#ifdef MIP6_INIT_AT_BOOT
         /* check ipv6 forwarding state before proceed */
         if (prefixcmd_getinet6sysctl (IPV6CTL_FORWARDING) == 1) {
            printf("\nKernel is configured as a router, exit MN start up\n");
            return ERROR;
         }

        ifconfig(MN_IFCONFIG_OPTIONS);
        Sysctl("-w net.inet6.ip6.accept_rtadv=1");
	Sysctl("-w net.inet6.ip6.forwarding=0");
	ip6Attach(IP6ATTACH_IFUNIT, IP6ATTACH_IFNAME);
	ifconfig(IFCONFIG_OPTIONS);
        mnd(MN_DAEMON_TASK_OPTIONS);
        mdd(MN_MOVEMENT_DETECTION_TASK_OPTIONS);
#else 
    mip6CommandInit();
#endif

#endif /* INCLUDE_MIP6_MN */


#ifdef INCLUDE_RTSOL
    usrNetRtsolInit ();
#endif

#ifdef INCLUDE_RTADV
    usrNetRtadvInit ();
#endif

#ifdef INCLUDE_RIP
    initInterfaces = RIP_IF_INIT;          /* XXX: virtualize! */
    recvBufSize = RIP_ROUTING_SOCKET_BUF_SIZE;
    usrNetRipInit (RIP_SUPPLIER, RIP_GATEWAY, RIP_MULTICAST, RIP_VERSION, 
                   RIP_TIMER_RATE,  RIP_SUPPLY_INTERVAL, RIP_EXPIRE_TIME, 
                   RIP_GARBAGE_TIME, RIP_AUTH_TYPE);
#endif

#ifdef INCLUDE_RIPNG
    usrripngInit ();
#endif
    
#ifdef INCLUDE_OSPFV3
    ospfv3InstInit();
#endif
  
#ifdef INCLUDE_OSPF
    ospfInstInit();
#endif
  
#ifdef INCLUDE_PROXY_SERVER
    usrProxyServerInit ();
#endif /* INCLUDE_PROXY_SERVER */

#ifdef INCLUDE_DHCP6C_SHOW
    dh6cShowInit();
#endif
    
#ifdef INCLUDE_DHCP6C
    usrNetDhcp6cInit ();
#endif /* INCLUDE_DHCP6C */    

#ifdef INCLUDE_DHCP6S
    usrNetDhcp6sInit ();
#endif /* INCLUDE_DHCP6S */    

#ifdef INCLUDE_DHCP6R
    usrNetDhcp6rInit ();
#endif /* INCLUDE_DHCP6R */    

#ifdef INCLUDE_MLD6_ROUTER
    usrNetMld6rInit ();
#endif /* INCLUDE_MLD6_ROUTER */
    }

/******************************************************************************
*
* networkinit - The network intialization routine 
*
* The network intialization routine for the clarinet stack
* has been derived from network part of  the prjConfig.c file.
* Basic network intialization is performed by default,  data
* structures , socket library intialization , ipv4 and ipv6
* attach to the end devices
* 
* Optional parmeters include network utility and application
* modules , examples are ping cleint , ifconfig utility
* show routines etc 
*
* RETURNS : N/A
*
*/

void networkinit(void)
    {
#ifdef INCLUDE_VIRTUAL_STACK
    usrVirtualStackInit ();  
#endif

#ifdef INCLUDE_APPL_LOG_UTIL
    usrNetApplUtilInit ();
#endif

#ifdef INCLUDE_VIRTUAL_STACK
#ifdef INCLUDE_VIRTUAL_STACK_OLD_INTERFACE 
    vsOldInit ();
#endif

#ifdef INCLUDE_VIRTUAL_STACK_DEBUG	
    vsDebInit ();
#endif

#ifdef INCLUDE_VIRTUAL_STACK_SHOW_FUNCTIONS 
    vsShowInit ();
#endif

#ifdef INCLUDE_VIRTUAL_STACK_EXAMPLE	
    vsExampleInit ();
#endif
#endif /* INCLUDE_VIRTUAL_STACK */

#ifdef INCLUDE_NET_SYSCTL
    usrNetSysctlInit ();                /* SYSCTL API support */

#endif /* INCLUDE_NET_SYSCTL */


/*
 * sysctl CLI initialization is not required here in VxWorks 6.x since
 * that has already been done as part of kernel initialization.
 */

#if (!defined(_WRS_VXWORKS_MAJOR) || (_WRS_VXWORKS_MAJOR < 6))
#ifdef INCLUDE_SYSCTL_CLI
    sysctlLibInit ();                 /* SYSCTL CLI support */
#endif
#endif

#ifdef     INCLUDE_NET_BOOT
    usrNetBoot();
#endif /* INCLUDE_NET_BOOT */
   
#if defined(INCLUDE_NETWORK) && !defined(INCLUDE_NET_MICRO_STACK)
#ifdef VIRTUAL_STACK
    /*
     * Register at vsmanager to setup the core network in the initial
     * virtual stack, prior to starting all other components.
     */

    vsComponentRegister (&netCoreRegistrationNum, "Net Core",
                         VS_SC_ENABLED, VS_SC_NON_OPTIONAL,
                         usrNetworkInit, 0, NULL, netCoreDestructor);
#else
    /*
     * When only one network stack exists, just
     * setup the core network information immediately.
     */

    usrNetworkInit ();
#endif /* VIRTUAL_STACK */
#endif /* INCLUDE_NETWORK && !INCLUDE_NET_MICRO_STACK */

#ifdef INCLUDE_NETBUFLIB
    netBufLibInit ();                   /* Network Buffer Library */

#ifdef INCLUDE_NETBUFADVLIB
    netBufAdvLibInit();
#endif
#ifdef INCLUDE_NETPOOLSHOW
    _func_netPoolShow = netPoolShow;
#endif
#ifdef INCLUDE_NETBUFPOOL
    netBufPoolInit ();			/* netBufLib Generic Pool */
#endif

#ifdef INCLUDE_LINKBUFPOOL
    linkBufPoolInit ();			/* Network Buffer Pool */
#endif
#endif	/* INCLUDE_NETBUFLIB */

#ifdef INCLUDE_JOB_QUEUE
    jobQueueLibInit();
#endif

#ifdef INCLUDE_JOB_QUEUE_UTIL
    jobQueueUtilInit();
#endif

#ifdef INCLUDE_NET_DAEMON
    usrNetDaemonInit(NET_JOB_NUM_CFG,NET_TASK_PRIORITY, NET_TASK_OPTIONS,
                     NET_TASK_STACKSIZE); /* Network Daemon Support */
#endif /* INCLUDE_NET_DAEMON */

#ifdef INCLUDE_GTF
    usrNetGtfInit ();                   /* General Timer Facility Support */
#endif /* INCLUDE_GTF */
    
#ifdef INCLUDE_NET_POOL     
    usrNetPoolConfig ();                /* Network Stack Memory Pool Configuration */
#endif /* INCLUDE_NET_POOL */

#ifdef INCLUDE_DSI_POOL
    usrDsiPoolConfig ();
#endif /* INCLUDE_DSI_POOL */

#ifdef INCLUDE_VLAN_TAG   
    vlanTagLibInit ();    /* layer 2 vlan tagging library support */
#endif /* INCLUDE_VLAN_TAG */

#ifdef INCLUDE_QOS_INGRESS
    usrNetQosIngressLibInit ();    /* Ingress QoS support */
#endif /* INCLUDE_QOS_INGRESS */

#ifdef INCLUDE_QOS_INGRESS_HOOKS
    qosIngressHooksInit ();    /* Ingress QoS hooks */
#endif /* INCLUDE_QOS_INGRESS_HOOKS */

#ifdef INCLUDE_ROUTE_STORAGE
    usrNetRouteStorageInit ();
#endif

    usrNetSocketInit();

    /*
     * NOTE: The usrNetHostSetup() routine must execute after the
     *       usrNetAddrInit() and usrNetmaskGet() routines, but
     *       before the MIB2 system initialization. 
     *
     * The usrNetAddrInit() and usrNetmaskGet() routines remove any
     * DHCP lease information and the netmask from the address string.
     * These steps guarantee the host table setup will only receive
     * the address string. It can't handle the additional information
     * contained in the bootline.
     *
     * The usrNetHostAdd() routine executes within the host setup routine
     * to provide the required local host name with the sethostname() routine
     * before the system MIB initializes. The system MIB initialization uses
     * the gethostname() routine to retrieve this information.
     */

    usrNetAddrInit ();
#ifdef INCLUDE_NETMASK_GET         /* extract netmask from address string */
    usrNetmaskGet ();
#endif

    usrNetHostSetup ();    /* create host table and add the local hostname */
    usrNetProtoInit();

#ifdef INCLUDE_DOMAIN_INIT
    usrNetDomainInit ();
#endif /* INCLUDE_DOMAIN_INIT */

#ifdef  INCLUDE_ROUTE 
    usrNetRoute ();
#endif /* INCLUDE_ROUTE */

#ifdef INCLUDE_GTF_TIMER_START
    usrNetGtfTimerStart();
#endif /* INCLUDE_GTF_TIMER_START */

#ifdef INCLUDE_MUX
    muxLibInit();
#endif 

#ifdef INCLUDE_MUX_L2
    usrNetMuxL2Init();
#endif /* INCLUDE_MUX_L2 */

#ifdef INCLUDE_IFINDEX
    ifIndexLibInit ();      /* replace with ifIndexLibInit2 if needed */
#endif

#ifdef INCLUDE_IFMEDIA
    ifmediaLibInit ();
#endif

#ifdef INCLUDE_LOOPBACK
    usrNetLoopbackStart ();             /* loopback interface */
#endif /* INCLUDE_LOOPBACK */

#ifdef INCLUDE_FASTPATH
    usrNetFFInit ();
#endif

#ifdef INCLUDE_VIRTUAL_STACK
    usrVirtualStackCoreInit ();
#endif /* INCLUDE_VIRTUAL_STACK */

#ifdef INCLUDE_HOSTCACHE
    inhc_init();
#endif /* INCLUDE_HOSTCACHE */

    usrNetRemoteInit ();	/* initialize network remote I/O access */
    usrNetDevStart ();		/* Network device start */

    usrNetUtilsInit ();
    usrNetAddrCheck ();

    usrNetAppInit();

#ifdef INCLUDE_VIRTUAL_STACK
    usrVirtualStackAppInit ();
#endif

#if (defined(INCLUDE_TCPV4) && defined(INCLUDE_NET_SYM_TBL))
    advanceTcpPorts ();
#endif
 
/* TIPC initialization if INCLUDE_NETWORK is defined */
#ifdef INCLUDE_TIPC

#ifdef INCLUDE_TIPC_MEDIA_SM
    usrTipcSmInit (BOOT_LINE_ADRS);
#endif

#ifdef INCLUDE_TIPC_MEMPOOL
   usrTipcPoolConfig();
#endif

#ifdef INCLUDE_TIPC_SOCKET
    usrTipcSocket();
#endif
    tipc_init (TIPC_CONFIG_STR, TIPC_CONFIG_HOOK);
#endif /* INCLUDE_TIPC */
    }

/******************************************************************************
*
* usrNetInit - network intialization routine called from usrConfig.c
* 
*/
STATUS usrNetInit
    (
    char *bootString
    )
    {
#ifdef INCLUDE_BOOT_LINE_INIT
    usrBootLineParse(NULL);
#endif /* INCLUDE_BOOT_LINE_INIT */ 
    networkinit ();
    return (OK);
    }

#ifdef INCLUDE_NET_BOOT
/******************************************************************************
*
* usrNetDevNameGet - use alternate device name field if necessary
*
* This routine sets the network-related boot parameter processing to read
* the network device name from the "other" field instead of the "boot device"
* field when booting with a block device.
*
* RETURNS: N/A
*
* NOMANUAL
*/

void usrNetDevNameGet (void)
    {
    /*
     * Do nothing if another device is already initialized or an 
     * error was detected in the boot parameters.
     */

    if (netDevBootFlag)
        return;

    pDevName = sysBootParams.bootDev;

    /* If booting from disk, configure additional network device, if any. */

    if ( (strncmp (sysBootParams.bootDev, "scsi", 4) == 0) ||
        (strncmp (sysBootParams.bootDev, "ide", 3) == 0) ||
        (strncmp (sysBootParams.bootDev, "ata", 3) == 0) ||
        (strncmp (sysBootParams.bootDev, "fd", 2) == 0)  ||
	(strncmp (sysBootParams.bootDev, "tffs", 4) == 0) )
        {
        if (sysBootParams.other [0] != EOS)
            {
            char *pStr;
            int unit;

            /*
             * Fix up the unitNum correctly. If the boot device
             * was specified in the other field, we need to derive
             * the unit number from here, rather than the one
             * derived from bootDev, since bootDev was a disk
             * device of some kind.
             */

            pStr = (char *)&sysBootParams.other;
            while (!isdigit (*pStr) && *pStr != EOS)
                pStr++;

            if (pStr != EOS && sscanf (pStr, "%d", &unit) == 1)
                {
                sysBootParams.unitNum = unit;
                uNum = unit;
                *pStr = EOS;
                }

            pDevName = sysBootParams.other;
            }
        else
            {
	    pDevName = NULL;
	    if ( _func_printErr )
		(* _func_printErr) ("usrNetDevNameGet: no network device\n");
	    }
        }
    else if (strncmp (sysBootParams.bootDev, "passDev", 7) == 0)
    	{
        pDevName = NULL;
        }
    return;
    }

/*******************************************************************************
*
* usrNetBoot - device independent network boot handling
*
* This routine prepares for the network-related boot parameter processing.
* It is the initialization routine for the INCLUDE_NET_INIT component. 
*
* RETURNS: N/A
*
* NOMANUAL
*/

void usrNetBoot (void)
    {
    /* Set to access the bootline params by default. */

    pAddrString = sysBootParams.ead;
    pTgtName    = sysBootParams.targetName;
    uNum        = sysBootParams.unitNum;

    usrNetDevNameGet ();

#ifdef INCLUDE_SM_NET
    if ((pDevName != NULL) &&
	((strncmp (pDevName, "bp", 2) == 0) ||
	 (strncmp (pDevName, "sm", 2) == 0)))
        {
        /* booting via backplane or SM networking */

        pAddrString = sysBootParams.bad;

        backplaneBoot = TRUE;        
        }
#endif /* INCLUDE_SM_NET */

    return;
    }

#endif /* INCLUDE_NET_BOOT */

#if defined(INCLUDE_NETWORK) && !defined(INCLUDE_NET_MICRO_STACK)

STATUS usrNetworkInit (void)
    {
    STATUS result;

    /* Setup the minimum initial network stack information. */

    result = netCoreSetup (MAX_LINKHDR_CFG, MAX_PROTOHDR_CFG,
                           NMBCLUSTERS_CFG, IP_MAX_UNITS);

    return (result);
    }
#endif /* INCLUDE_NETWORK && !INCLUDE_NET_MICRO_STACK */ 


#ifdef  INCLUDE_NET_DAEMON

STATUS usrNetDaemonInit
    (
    unsigned int	jobNum,
    unsigned int	pri,
    int			opts,
    unsigned int	stack
    )
    {
    netJobNum = jobNum;
    netTaskPriority = pri;
    netTaskOptions = opts;
    netTaskStackSize = stack;

    if ( !(netTaskOptions & VX_UNBREAKABLE) )
	{
#ifdef INCLUDE_SHELL_DEBUG
        _func_bdall = (FUNCPTR)bdall;
#endif
	}

    return (netLibInit());    
    }
#endif /* INCLUDE_NET_DAEMON */


#ifdef INCLUDE_DOMAIN_INIT
#ifdef VIRTUAL_STACK
extern VS_REG_ID domainRegistrationNum;
STATUS usrVsWrapperDomainInit (void *cfgParams);
STATUS domainDestruct(VSNUM vsnum);
#endif

void usrNetDomainInit (void)
    {
#ifdef VIRTUAL_STACK
    vsComponentRegister (&domainRegistrationNum,
                         "Domain Init",
                         VS_SC_ENABLED,
                         VS_SC_NON_OPTIONAL,
                         usrVsWrapperDomainInit,
                         0,
                         NULL,
                         domainDestruct);
#else
    domaininit (NULL);
#endif /* VIRTUAL_STACK */
    }

#ifdef VIRTUAL_STACK
STATUS usrVsWrapperDomainInit (void *cfgParams)
    {
    domaininit(NULL);
    return (OK);
    }
#endif /* VIRTUAL_STACK */
#endif /* INCLUDE_DOMAIN_INIT */

#ifdef INCLUDE_MUX_L2
STATUS usrNetMuxL2Init (void)
    {
    STATUS  rc;

    bzero ((char *)&muxL2DefaultConfigParams, sizeof (MUX_L2_CONFIG_PARAMS));
    muxL2DefaultConfigParams.cfgh.len = sizeof (MUX_L2_CONFIG_PARAMS);
    muxL2DefaultConfigParams.cfg_priv_muxL2NumPorts = MUX_L2_NUM_PORTS_CFG;
    muxL2DefaultConfigParams.cfg_priv_muxL2MaxVlanSupport = 
                                                    MUX_L2_MAX_VLANS_CFG;

    rc = muxL2LibInit (&muxL2DefaultConfigParams);
    if (rc == ERROR)
        {
	    if (_func_printErr)
	        (*_func_printErr) ("usrNetMuxL2Init: muxL2LibInit failed\n");
        return ERROR;
        }

    return OK;
    }
#endif /* INCLUDE_MUX_L2 */

#ifdef INCLUDE_IPV4

void usrNetIpv4Init (void)
    {
    /* ARP */

    bzero ((char *)&arpDefaultConfigParams, sizeof (ARP_CONFIG_PARAMS));
    arpDefaultConfigParams.cfgh.len = sizeof (ARP_CONFIG_PARAMS);
    arpDefaultConfigParams.cfg_priv_arpt_prune = ARP_PRUNE_INTVL_CFG;
    arpDefaultConfigParams.cfg_priv_arpt_keep = ARP_MAX_AGE_CFG;
    arpDefaultConfigParams.cfg_priv_arpt_down = ARP_HOST_DOWN_TIME_CFG;
    arpDefaultConfigParams.cfg_priv_arpMaxEntries = ARP_MAX_ENTRIES_CFG;
    arpDefaultConfigParams.cfg_priv_arp_maxtries = ARP_MAXTRIES_CFG;
    arpDefaultConfigParams.cfg_priv_useloopback = ARP_USELOOPBACK_CFG;
    arpDefaultConfigParams.cfg_priv_arp_proxyall = ARP_PROXYALL_CFG;
    arpDefaultConfigParams.cfg_arp_gratuituous = ARP_GRATUITUOUS_CFG;
    arpDefaultConfigParams.cfg_priv_log_arp_wrong_iface =
        LOG_ARP_WRONG_IFACE_CFG;

#ifdef INCLUDE_NET_SYSCTL
    /* Initialize the sysctl node */

    arpDefaultConfigParams.cfg_privInitSysctl = ifEtherSysctlInit;
#else
    arpDefaultConfigParams.cfg_privInitSysctl = NULL;
#endif

#ifdef VIRTUAL_STACK
    vsComponentRegister (&arpRegistrationNum,
                         "ARP",
                         VS_SC_ENABLED,
                         VS_SC_NON_OPTIONAL,
                         arpInstInit,
                         sizeof (ARP_CONFIG_PARAMS),
                         &arpDefaultConfigParams,
                         arpDestructor);
#else
    arpInstInit (&arpDefaultConfigParams);
#endif /* VIRTUAL_STACK */

    /* set up the default configuration parameters */

    bzero ((char *) &ipDefaultConfigParams, sizeof (IP_CONFIG_PARAMS));
    ipDefaultConfigParams.cfgh.len = sizeof (IP_CONFIG_PARAMS);

    /* definitions from in.c */

    ipDefaultConfigParams.cfg_priv_subnetsarelocal = SUBNETS_ARE_LOCAL_CFG;

    /* definitions from in_pcb.c */

    ipDefaultConfigParams.cfg_ipport_lowfirstauto = IPPORT_RESERVED_CFG - 1;
    ipDefaultConfigParams.cfg_ipport_lowlastauto = IPPORT_RESERVEDSTART_CFG;
    ipDefaultConfigParams.cfg_ipport_firstauto = IPPORT_RESERVED_CFG;
    ipDefaultConfigParams.cfg_ipport_lastauto = IPPORT_USERRESERVED_CFG;
    ipDefaultConfigParams.cfg_ipport_hifirstauto = IPPORT_HIFIRSTAUTO_CFG;
    ipDefaultConfigParams.cfg_ipport_hilastauto = IPPORT_HILASTAUTO_CFG;

    /* definitions from in_rmx.c */

    ipDefaultConfigParams.cfg_priv_rtq_reallyold = RTEXPIRE_CFG;
    ipDefaultConfigParams.cfg_priv_rtq_minreallyold = RTMINEXPIRE_CFG;
    ipDefaultConfigParams.cfg_priv_rtq_toomany = RTMAXCACHE_CFG;
    ipDefaultConfigParams.cfg_priv_rtq_timeout = RTQ_TIMEOUT;

#if VX_VERSION == 62
    /* definitions from in_flow.c */
    ipDefaultConfigParams.cfg_priv_ipflow_active = FASTFORWARDING_CFG;
#endif /* VX_VERSION */

    /* definitions from ip_input.c */

    ipDefaultConfigParams.cfg_priv_ipsendredirects = IPSENDREDIRECTS_CFG;
    ipDefaultConfigParams.cfg_priv_ip_dosourceroute = IP_DOSOURCEROUTE_CFG;
    ipDefaultConfigParams.cfg_priv_ip_acceptsourceroute =
	IP_ACCEPTSOURCEROUTE_CFG;
    ipDefaultConfigParams.cfg_priv_ip_checkinterface = IP_CHECKINTERFACE_CFG;
    ipDefaultConfigParams.cfg_priv_ipqmaxlen = IPINTRQ_IFQ_MAXLEN_CFG;
    ipDefaultConfigParams.cfg_priv_maxnipq = IP_MAXFRAGPACKETS_CFG;
    ipDefaultConfigParams.cfg_priv_maxfragsperpacket = 
	IP_MAXFRAGSPERPACKET_CFG;

    /* was ifdef IPSTEALTH but not avail in configlettes */

    ipDefaultConfigParams.cfg_priv_ipstealth = IPSTEALTH_CFG;
    ipDefaultConfigParams.cfg_ipforwarding = IPFORWARDING_CFG;
    ipDefaultConfigParams.cfg_ip_defttl = IP_DEFTTL_CFG;

    /* definitions from raw_in.c */

    ipDefaultConfigParams.cfg_rip_sendspace = RAW_MAXDGRAM_CFG;
    ipDefaultConfigParams.cfg_rip_recvspace = RAW_RECVSPACE_CFG;

    ipDefaultConfigParams.cfg_ipv4RtDispTable = &IPV4_RT_DISP_TBL;

#ifdef INCLUDE_NET_SYSCTL
    /* Initialize the sysctl node */
    ipDefaultConfigParams.cfg_privInitSysctl = ipSysctlInit;
#else
    ipDefaultConfigParams.cfg_privInitSysctl = NULL;
#endif

#ifdef VIRTUAL_STACK
    vsComponentRegister (&ipRegistrationNum,
                         "IPv4",
                         VS_SC_ENABLED,
                         VS_SC_NON_OPTIONAL,
                         ipInstInit,
                         sizeof (IP_CONFIG_PARAMS),
                         &ipDefaultConfigParams,
                         ipDestructor);
#else
    ipInstInit (&ipDefaultConfigParams);
#endif /* VIRTUAL_STACK */
    }

#endif /*INCLUDE_IPV4 */



#ifdef INCLUDE_ICMPV4

/*
 * Minimum acceptable IPv4 MTU. Should be at least 68.
 * Presently unvirtualized, same value used in all stacks.
 * UNVIRTUALIZED.
 */
int priv_icmpv4_mtu_min = ICMPV4_MTU_MIN_CFG;

/*
 * Default to reject ICMPv4 Destination Unreachable / Needs
 * Fragmentation errors unless the protocol validates them.
 * Presently, only TCP knows how to validate such error
 * messages, and (with exceptions which we neglect) only
 * TCP sends IP packets with the IP_DF flag set, so we
 * only expect such errors back for TCP segments we send.
 * With this default, for any other protocol we reject
 * Needs Fragmentation errors.
 * Set priv_icmpsrc_result_default to ICMP4_RESULT_ACCEPT
 * if the stack should accept Needs Fragmentation ICMPv4
 * errors bearing non-TCP segments.
 * UNVIRTUALIZED.
 */
int priv_icmpsrc_result_default = ICMPV4_VALIDATION_REQ_CFG;

/*
 * If false, do not respond to ICMP timestamp requests.
 * Value initialized in usrNetIcmpv4.c.
 * Unvirtualized, same policy used in all virtual stacks.
 */
BOOL priv_icmpv4_timestamp_support = ICMPV4_TIMESTAMP_REQ_SUPPORT_CFG;


void usrNetIcmpv4Init (void)
    {
#if (ICMPERRORLEN_CFG < 0) || (ICMPERRORLEN_CFG > 488)
#error usrNetIcmpv4: wrong value for ICMPERRORLEN_CFG (0-488)
#endif

    /* set up the default configuration parameters */

    bzero ((char *) &icmpDefaultConfigParams, sizeof (ICMP_CONFIG_PARAMS));
    icmpDefaultConfigParams.cfgh.len = sizeof (ICMP_CONFIG_PARAMS);
    icmpDefaultConfigParams.cfg_priv_icmpmaskrepl = ICMPMASKREPL_CFG;
    icmpDefaultConfigParams.cfg_priv_drop_redirect = ICMP_DROP_REDIRECT_CFG;
    icmpDefaultConfigParams.cfg_priv_log_redirect = ICMP_LOG_REDIRECT_CFG;
    icmpDefaultConfigParams.cfg_priv_icmplim = ICMPLIM_CFG;
    icmpDefaultConfigParams.cfg_priv_icmpbmcastecho = ICMPBMCASTECHO_CFG;
    icmpDefaultConfigParams.cfg_icmpErrorLen   = ICMPERRORLEN_CFG;

#ifdef INCLUDE_NET_SYSCTL
    /* Initialize the sysctl node */
    icmpDefaultConfigParams.cfg_privInitSysctl   = icmpv4SysctlInit;
#else
    icmpDefaultConfigParams.cfg_privInitSysctl   = NULL;
#endif

#ifdef VIRTUAL_STACK
    vsComponentRegister (&icmpRegistrationNum,
                         "ICMP",
                         VS_SC_ENABLED,
                         VS_SC_NON_OPTIONAL,
                         icmpInstInit,
                         sizeof (ICMP_CONFIG_PARAMS),
                         &icmpDefaultConfigParams,
                         icmpDestructor);
#else
    icmpInstInit (&icmpDefaultConfigParams);
#endif /* VIRTUAL_STACK */

    return;
    }

#endif    /* INCLUDE_ICMPV4 */


#ifdef INCLUDE_IGMP

void usrNetIgmpInit (void)
    {
    /* set up the default configuration parameters */

    bzero ((char *) &igmpDefaultConfigParams, sizeof (IGMP_CONFIG_PARAMS));
    igmpDefaultConfigParams.cfgh.len = sizeof (IGMP_CONFIG_PARAMS);

#ifdef INCLUDE_NET_SYSCTL
    /* Initialize the sysctl node */
    igmpDefaultConfigParams.cfg_privInitSysctl   = igmpSysctlInit;
#else
    igmpDefaultConfigParams.cfg_privInitSysctl   = NULL;
#endif

#ifdef VIRTUAL_STACK
    vsComponentRegister (&igmpRegistrationNum,
                        "IGMP",
                        VS_SC_ENABLED,
                        VS_SC_NON_OPTIONAL,
                        (STATUS (*) (void *)) igmpInstInit,
                        sizeof (IGMP_CONFIG_PARAMS),
                        &igmpDefaultConfigParams,
                        igmpDestructor);
#else
    igmpInstInit (&igmpDefaultConfigParams);
#endif /* VIRTUAL_STACK */

    return;
    }

#endif    /* INCLUDE_IGMP */


#ifdef INCLUDE_TCP_DEBUG

void usrNetTcpv4DebugInit (void)
    {
#ifdef VIRTUAL_STACK
    vsComponentRegister (&tcpDebugRegistrationNum, "TCP DEBUG",
                         VS_SC_ENABLED, VS_SC_OPTIONAL,
                         tcp_debugInit, 0, NULL, NULL);
#else
    tcp_debugInit ();
#endif

    return;    
    }

#endif /* INCLUDE_TCP_DEBUG */

#ifdef INCLUDE_TCPV4

void usrNetTcpv4Init (void)
    {
    bzero ((char *) &tcpDefaultConfigParams, sizeof (TCP_CONFIG_PARAMS));
    tcpDefaultConfigParams.cfgh.len = sizeof (TCP_CONFIG_PARAMS);

    /* tcp_subr */

    tcpDefaultConfigParams.cfg_tcp_mssdflt = TCP_MSSDFLT_CFG;
    tcpDefaultConfigParams.cfg_priv_tcp_do_rfc1323 = TCP_DO_RFC1323_CFG;
    tcpDefaultConfigParams.cfg_priv_tcp_do_rfc1644 = TCP_DO_RFC1644_CFG;
    tcpDefaultConfigParams.cfg_priv_do_tcpdrain = DO_TCPDRAIN_CFG;
    tcpDefaultConfigParams.cfg_priv_icmp_may_rst = ICMP_MAY_RST_CFG;
    tcpDefaultConfigParams.cfg_tcp_isn_reseed_interval =
                                             TCP_SEQ_ISNSEED_INTERVAL;
    tcpDefaultConfigParams.cfg_tcp_inflight_enable = TCP_INFLIGHT_ENABLE;
    tcpDefaultConfigParams.cfg_tcp_inflight_debug = TCP_INFLIGHT_DEBUG;
    tcpDefaultConfigParams.cfg_tcp_inflight_min = TCP_INFLIGHT_MIN;
    tcpDefaultConfigParams.cfg_tcp_inflight_max = TCP_INFLIGHT_MAX;

    /* tcp_timer */

    tcpDefaultConfigParams.cfg_tcp_keepinit = TCP_KEEPINIT_CFG;
    tcpDefaultConfigParams.cfg_tcp_keepidle = TCP_KEEPIDLE_CFG;
    tcpDefaultConfigParams.cfg_tcp_keepintvl = TCP_KEEPINTVL_CFG;
    tcpDefaultConfigParams.cfg_priv_tcp_keepcnt = TCP_KEEPCNT_CFG;
    tcpDefaultConfigParams.cfg_tcp_delacktime = TCP_DELACKTIME_CFG;
    tcpDefaultConfigParams.cfg_tcp_msl = TCP_MSL_CFG;
    tcpDefaultConfigParams.cfg_priv_always_keepalive = TCP_ALWAYS_KEEPALIVE_CFG;

    /* tcp_input */

    tcpDefaultConfigParams.cfg_priv_tcp_log_in_vain = TCP_LOG_IN_VAIN_CFG;
    tcpDefaultConfigParams.cfg_priv_tcp_blackhole = TCP_BLACKHOLE_CFG;
    tcpDefaultConfigParams.cfg_tcp_delack_enabled = TCP_DELACK_ENABLED_CFG;

    /* was ifdef TCP_DROP_SYNFIN but not avail in configlette */
    tcpDefaultConfigParams.cfg_priv_drop_synfin = TCP_DROP_SYNFIN_CFG;
    tcpDefaultConfigParams.cfg_tcp_reass_global_max = TCP_REASS_GLOBAL_MAX_CFG;
    tcpDefaultConfigParams.cfg_tcp_reass_endpt_max = TCP_REASS_ENDPT_MAX_CFG;

    /* tcp_output */

    tcpDefaultConfigParams.cfg_priv_path_mtu_discovery = TCP_PATH_MTU_DISCOVERY_CFG;
    tcpDefaultConfigParams.cfg_ss_fltsz = TCP_SLOWSTART_FLIGHTSIZE_CFG;
    tcpDefaultConfigParams.cfg_ss_fltsz_local = TCP_LOCAL_SLOWSTART_FLIGHTSIZE_CFG;
    tcpDefaultConfigParams.cfg_tcp_do_newreno = TCP_DO_NEWRENO_CFG;

    /* tcp_usrreqs */

    tcpDefaultConfigParams.cfg_tcp_sendspace = TCP_SENDSPACE_CFG;
    tcpDefaultConfigParams.cfg_tcp_recvspace = TCP_RECVSPACE_CFG;

#ifdef INCLUDE_NET_SYSCTL
    /* Initialize the sysctl node */
    tcpDefaultConfigParams.cfg_privInitSysctl   = tcpv4SysctlInit;
#else
    tcpDefaultConfigParams.cfg_privInitSysctl   = NULL;
#endif

#ifdef VIRTUAL_STACK
    vsComponentRegister (&tcpRegistrationNum,
                         "TCP",
                         VS_SC_ENABLED,
                         VS_SC_NON_OPTIONAL,
                         tcpInstInit,
                         sizeof (TCP_CONFIG_PARAMS),
                         &tcpDefaultConfigParams,
                         tcpDestructor);
#else
    tcpInstInit (&tcpDefaultConfigParams);
#endif

    return;
    }

#endif    /* INCLUDE_TCPV4 */

#ifdef INCLUDE_SCTP
void usrNetSctpInit (void)
    {
    bzero ((char *) &sctpDefaultConfigParams, sizeof (SCTP_CONFIG_PARAMS));
    sctpDefaultConfigParams.cfgh.len = sizeof (SCTP_CONFIG_PARAMS);

    sctpDefaultConfigParams.cfg_sctp_sendspace = SCTP_SENDSPACE_CFG;
    sctpDefaultConfigParams.cfg_sctp_recvspace = SCTP_RECVSPACE_CFG;
    sctpDefaultConfigParams.cfg_sctp_auto_asconf = SCTP_AUTO_ASCONF_CFG;
    sctpDefaultConfigParams.cfg_sctp_ecn = SCTP_ECN_CFG;
    sctpDefaultConfigParams.cfg_sctp_ecn_nonce = SCTP_ECN_NONCE_CFG;
    sctpDefaultConfigParams.cfg_sctp_strict_sacks = SCTP_STRICT_SACKS_CFG;
    sctpDefaultConfigParams.cfg_sctp_no_csum_on_loopback = 
                                            SCTP_NO_CKSUM_ON_LOOPBACK_CFG;
    sctpDefaultConfigParams.cfg_sctp_strict_init = SCTP_STRICT_INIT_CFG;
    sctpDefaultConfigParams.cfg_sctp_peer_chunk_oh = SCTP_PEER_CHUNK_OH_CFG;
    sctpDefaultConfigParams.cfg_sctp_max_burst_default = 
                                            SCTP_MAX_BURST_DEF_CFG;
    sctpDefaultConfigParams.cfg_sctp_max_chunks_on_queue = 
                                            SCTP_MAX_CHUNKS_ON_QUEUE_CFG;
    sctpDefaultConfigParams.cfg_sctp_delayed_sack_time_default = 
                                            SCTP_DELAYED_SACK_TIME_DEF_CFG;
    sctpDefaultConfigParams.cfg_sctp_heartbeat_interval_default =
                                            SCTP_HEARTBEAT_INTERVAL_DEF_CFG;
    sctpDefaultConfigParams.cfg_sctp_pmtu_raise_time_default = 
                                            SCTP_DEF_PMTU_RAISE_SEC_CFG;
    sctpDefaultConfigParams.cfg_sctp_shutdown_guard_time_default =
                                            SCTP_DEF_MAX_SHUTDOWN_SEC_CFG;
    sctpDefaultConfigParams.cfg_sctp_secret_lifetime_default =
                                            SCTP_DEF_SECRET_LIFE_SEC_CFG;
    sctpDefaultConfigParams.cfg_sctp_rto_max_default = 
                                            SCTP_RTO_UPPER_BOUND_CFG;
    sctpDefaultConfigParams.cfg_sctp_rto_min_default =
                                            SCTP_RTO_LOWER_BOUND_CFG;
    sctpDefaultConfigParams.cfg_sctp_rto_initial_default =
                                            SCTP_RTO_INITIAL_CFG;
    sctpDefaultConfigParams.cfg_sctp_init_rto_max_default =
                                            SCTP_RTO_UPPER_BOUND_CFG;
    sctpDefaultConfigParams.cfg_sctp_valid_cookie_life_default =
                                            SCTP_DEFAULT_COOKIE_LIFE_CFG;
    sctpDefaultConfigParams.cfg_sctp_init_rtx_max_default =
                                            SCTP_DEF_MAX_INIT_CFG;
    sctpDefaultConfigParams.cfg_sctp_assoc_rtx_max_default =
                                            SCTP_DEF_MAX_SEND_CFG;
    sctpDefaultConfigParams.cfg_sctp_path_rtx_max_default =
                                            SCTP_DEF_MAX_SEND_CFG/2;
    sctpDefaultConfigParams.cfg_sctp_nr_outgoing_streams_default =
                                            SCTP_OSTREAM_INITIAL_CFG;
    sctpDefaultConfigParams.cfg_sctp_debug_on = SCTP_DEBUG_CFG;

#ifdef INCLUDE_NET_SYSCTL
    /* Initialize the sysctl node */
    sctpDefaultConfigParams.cfg_privInitSysctl = sctpSysctlInit;
#else
    sctpDefaultConfigParams.cfg_privInitSysctl = NULL;
#endif /* INCLUDE_NET_SYSCTL */

    #ifdef VIRTUAL_STACK
    vsComponentRegister (&sctpRegistrationNum,
                         "SCTPv4",
                         VS_SC_ENABLED,
                         VS_SC_NON_OPTIONAL,
                         sctpInstInit,
                         sizeof (SCTP_CONFIG_PARAMS),
                         &sctpDefaultConfigParams,
                         sctpDestructor);
#else
    sctpInstInit (&sctpDefaultConfigParams);
#endif

#ifdef INCLUDE_IPV6
    bzero ((char *) &sctp6DefaultConfigParams, sizeof (SCTP6_CONFIG_PARAMS));
    sctp6DefaultConfigParams.cfgh.len = sizeof (SCTP6_CONFIG_PARAMS);

#ifdef INCLUDE_NET_SYSCTL
    sctp6DefaultConfigParams.cfg_privInitSysctl = sctp6SysctlInit;
#else
    sctp6DefaultConfigParams.cfg_privInitSysctl = NULL;
#endif /* INCLUDE_NET_SYSCTL */

#ifdef VIRTUAL_STACK
    vsComponentRegister (&sctpv6RegistrationNum, "SCTPv6", VS_SC_ENABLED,
                         VS_SC_NON_OPTIONAL, sctpv6InstInit,
                         sizeof (SCTP6_CONFIG_PARAMS),
                         &sctp6DefaultConfigParams, NULL);
#else
    sctpv6InstInit(&sctp6DefaultConfigParams);
#endif /* VIRTUAL_STACK */
#endif /* INCLUDE_IPV6 */

    return;
    }
#endif /* INCLUDE_SCTP */


#ifdef INCLUDE_RAWV4

void usrNetRawv4Init (void)
    {
#ifdef VIRTUAL_STACK
    vsComponentRegister (&rawRegistrationNum,
                         "RAWCB",
                         VS_SC_ENABLED,
                         VS_SC_NON_OPTIONAL,
                         rawInstInit,
                         0,
                         NULL,
                         rawDestructor);
#else
    rawInstInit (NULL);
#endif

    return;
    }

#endif    /* INCLUDE_RAWV4 */



#ifdef INCLUDE_UDPV4

void usrNetUdpv4Init (void)
    {
    bzero ((char *) &udpDefaultConfigParams, sizeof (UDP_CONFIG_PARAMS));
    udpDefaultConfigParams.cfgh.len = sizeof (UDP_CONFIG_PARAMS);
    udpDefaultConfigParams.cfg_udpcksum = UDPCKSUM_CFG;
    udpDefaultConfigParams.cfg_udp_log_in_vain = UDP_LOG_IN_VAIN_CFG;
    udpDefaultConfigParams.cfg_priv_udp_blackhole = UDP_BLACKHOLE_CFG;
    udpDefaultConfigParams.cfg_udp_sendspace = UDP_MAXDGRAM_CFG;
    udpDefaultConfigParams.cfg_udp_recvspace = UDP_RECVSPACE_CFG;

#ifdef INCLUDE_NET_SYSCTL
    /* Initialize the sysctl node */
    udpDefaultConfigParams.cfg_privInitSysctl   = udpv4SysctlInit;
#else
    udpDefaultConfigParams.cfg_privInitSysctl   = NULL;
#endif

#ifdef VIRTUAL_STACK
    vsComponentRegister (&udpRegistrationNum,
                         "UDP",
                         VS_SC_ENABLED,
                         VS_SC_NON_OPTIONAL,
                         udpInstInit,
                         sizeof (UDP_CONFIG_PARAMS),
                         &udpDefaultConfigParams,
                         udpDestructor);
#else
    udpInstInit (&udpDefaultConfigParams);
#endif

    return;
    }

#endif    /* INCLUDE_UDPV4 */


#ifdef INCLUDE_MROUTE
void usrNetIpMrouteInit (void)
    {
    bzero ((char *) &ipmDefaultConfigParams, sizeof (IPM_CONFIG_PARAMS));
    ipmDefaultConfigParams.cfgh.len = sizeof (IPM_CONFIG_PARAMS);

#ifdef INCLUDE_NET_SYSCTL
    /* Initialize the sysctl node */

    ipmDefaultConfigParams.cfg_privInitSysctl = ipMrouteSysctlInit;
#else
    ipmDefaultConfigParams.cfg_privInitSysctl = NULL;
#endif

#ifdef VIRTUAL_STACK
    vsComponentRegister (&mrouteRegistrationNum,
                         "MROUTE",
                         VS_SC_ENABLED,
                         VS_SC_NON_OPTIONAL,
                         mrouteInstInit,
                         sizeof (IPM_CONFIG_PARAMS),
                         &ipmDefaultConfigParams,
                         mrouteDestructor);
#else
    mrouteInstInit (&ipmDefaultConfigParams);
#endif /* VIRTUAL_STACK */

    return;
    }

#endif    /* INCLUDE_MROUTE */


#ifdef INCLUDE_ROUTING_SOCKET
STATUS usrNetRtSockInit (void)
    {
#ifdef VIRTUAL_STACK
    return vsComponentRegister (&rtsockRegistrationNum,
                                         "RTSOCK",
                                         VS_SC_ENABLED,
                                         VS_SC_NON_OPTIONAL,
                                         rtsockInstInit,
                                         0, NULL,
                                         rtsockDestructor);
#else
    return rtsockInstInit ();
#endif /* VIRTUAL_STACK */
    }
#endif /* ROUTING_SOCKET */

#ifdef INCLUDE_ND

void usrNetNdInit (void)
    {
    /* set up the default configuration parameters */
    bzero ((char *)&nd6DefaultConfigParams, sizeof (ND6_CONFIG_PARAMS));
    nd6DefaultConfigParams.cfgh.len = sizeof (ND6_CONFIG_PARAMS);
    nd6DefaultConfigParams.cfg_nd6_prune = ND6_PRUNE_CFG;
    nd6DefaultConfigParams.cfg_nd6_delay = ND6_DELAY_CFG;
    nd6DefaultConfigParams.cfg_nd6_umaxtries = ND6_UMAXTRIES_CFG;
    nd6DefaultConfigParams.cfg_nd6_mmaxtries = ND6_MMAXTRIES_CFG;
    nd6DefaultConfigParams.cfg_nd6_useloopback = ND6_USELOOPBACK_CFG;
    nd6DefaultConfigParams.cfg_nd6_maxnudhint = ND6_MAXNUDHINT_CFG;
    nd6DefaultConfigParams.cfg_nd6_maxqueuelen = ND6_MAXQUEUELEN_CFG;
    nd6DefaultConfigParams.cfg_nd6_debug = ND6_DEBUG_CFG; 
    nd6DefaultConfigParams.cfg_ip6_use_tempaddr = ND6_USETEMPADDR_CFG;
    nd6DefaultConfigParams.cfg_ip6_temp_preferred_lifetime = ND6_TEMPPLTIME_CFG;
    nd6DefaultConfigParams.cfg_ip6_temp_valid_lifetime = ND6_TEMPVLTIME_CFG;

#ifdef INCLUDE_NET_SYSCTL
    /* Initialize the sysctl node */
    nd6DefaultConfigParams.cfg_privInitSysctl   = nd6SysctlInit;
#else
    nd6DefaultConfigParams.cfg_privInitSysctl   = NULL;
#endif

#ifdef VIRTUAL_STACK
    vsComponentRegister (&nd6RegistrationNum,
                         "ND6",
                         VS_SC_ENABLED,
                         VS_SC_NON_OPTIONAL,
                         nd6InstInit,
                         sizeof (ND6_CONFIG_PARAMS),
                         &nd6DefaultConfigParams,
                         nd6Destructor);
#else
    nd6InstInit (&nd6DefaultConfigParams);
#endif
    }

#endif    /* INCLUDE_ND */



#ifdef INCLUDE_IPV6_FW

void usrNetIpv6FwInit (void)
    {
    }

#endif    /* INCLUDE_IPV6_FW */



#ifdef INCLUDE_ICMPV6

/*
 * Ignore ICMP packet too big error messages with next hop MTU less
 * than this value.  By default, we use the value 0.
 * UNVIRTUALIZED, use same policy for all virtual stacks.
 */
unsigned int priv_icmp6_mtu_min = ICMPV6_MTU_MIN_CFG;

void usrNetIcmpv6Init (void)
    {
    /* set up the default configuration parameters */

    icmpv6DefaultConfigParams.cfgh.len = sizeof (ICMPV6_CONFIG_PARAMS);
    icmpv6DefaultConfigParams.cfg_icmp6_rediraccept = ICMPV6CTL_REDIRACCEPT_CFG; /*1;*/
    icmpv6DefaultConfigParams.cfg_icmp6_redirtimeout = ICMPV6CTL_REDIRTIMEOUT_CFG; /*10 * 60;*/
    icmpv6DefaultConfigParams.cfg_icmp6errppslim = ICMPV6CTL_ERRPPSLIMIT_CFG; /*100;*/
    icmpv6DefaultConfigParams.cfg_icmp6nodeinfo = ICMPV6CTL_NODEINFO_CFG; /*3;*/
    
#ifdef INCLUDE_NET_SYSCTL
    /* Initialize the sysctl node */
    icmpv6DefaultConfigParams.cfg_privInitSysctl   = icmp6SysctlInit;
#else
    icmpv6DefaultConfigParams.cfg_privInitSysctl   = NULL;
#endif /* INCLUDE_NET_SYSCTL */

#ifdef VIRTUAL_STACK
    vsComponentRegister (&icmpv6RegistrationNum, "ICMPv6", VS_SC_ENABLED,
                         VS_SC_NON_OPTIONAL, icmpv6InstInit,
                         sizeof (ICMPV6_CONFIG_PARAMS),
                         &icmpv6DefaultConfigParams, icmpv6Destructor);
#else
    icmpv6InstInit (&icmpv6DefaultConfigParams);
#endif /* VIRTUAL_STACK */
    }

#endif   /* INCLUDE_ICMPV6 */

#ifdef INCLUDE_MLD

void usrNetMldInit (void)
    {

    mldDefaultConfigParams.cfgh.len = sizeof (MLD_CONFIG_PARAMS);
    mldDefaultConfigParams.cfg_mldmaxsrcfilter = ICMPV6CTL_MLD_MAXSRCFILTER_CFG;
    mldDefaultConfigParams.cfg_mldsomaxsrc     = ICMPV6CTL_MLD_SOMAXSRC_CFG;
    mldDefaultConfigParams.cfg_mld_version     = ICMPV6CTL_MLD_VERSION_CFG;

#ifdef INCLUDE_NET_SYSCTL
    /* Initialize the sysctl node */
    mldDefaultConfigParams.cfg_privInitSysctl   = mldSysctlInit;
#else
    mldDefaultConfigParams.cfg_privInitSysctl   = NULL;
#endif /* INCLUDE_NET_SYSCTL */

#ifdef VIRTUAL_STACK
    vsComponentRegister (&mldRegistrationNum,
                         "MLD",
                         VS_SC_ENABLED,
                         VS_SC_NON_OPTIONAL,
                         mldInstInit,
                         sizeof (MLD_CONFIG_PARAMS),
                         &mldDefaultConfigParams,
                         mldDestructor);
#else
    mldInstInit(&mldDefaultConfigParams);
#endif /* VIRTUAL_STACK */

    }
#endif /* INCLUDE_MLD */
    
#ifdef INCLUDE_IPV6

void usrNetIpv6Init (void)
    {
#ifdef VIRTUAL_STACK     
    VS_STATUS   	registered;
#endif /*VIRTUAL_STACK*/

    IPv6DefaultConfigParams.cfgh.len		= sizeof (IPV6_CONFIG_PARAMS);
    IPv6DefaultConfigParams.cfg_ip6_auto_linklocal = 
        IPV6CTL_AUTO_LINKLOCAL_CFG;
    IPv6DefaultConfigParams.cfg_ip6_forwarding	= 
        IPV6CTL_FORWARDING_CFG;
    IPv6DefaultConfigParams.cfg_ip6_sendredirects	= 
        IPV6CTL_SENDREDIRECTS_CFG;
    IPv6DefaultConfigParams.cfg_ip6_defhlim		= 
        IPV6CTL_DEFHLIM_CFG;
    IPv6DefaultConfigParams.cfg_ip6_defmcasthlim	= 
        IPV6CTL_DEFMCASTHLIM_CFG;
    IPv6DefaultConfigParams.cfg_ip6_accept_rtadv	= 
        IPV6CTL_ACCEPT_RTADV_CFG;
    IPv6DefaultConfigParams.cfg_ip6_maxfragpackets	= 
        IPV6CTL_MAXFRAGPACKETS_CFG;
    IPv6DefaultConfigParams.cfg_ip6_maxfrags		=
        IPV6CTL_MAXFRAGPACKETS_CFG;
    IPv6DefaultConfigParams.cfg_ip6_log_interval	= 
        IPV6CTL_LOG_INTERVAL_CFG;
    IPv6DefaultConfigParams.cfg_ip6_hdrnestlimit	= 
        IPV6CTL_HDRNESTLIMIT_CFG;
    IPv6DefaultConfigParams.cfg_ip6_dad_count	= 
        IPV6CTL_DAD_COUNT_CFG;
    IPv6DefaultConfigParams.cfg_ip6_auto_flowlabel	= 
        IPV6CTL_AUTO_FLOWLABEL_CFG;
    IPv6DefaultConfigParams.cfg_ip6_gif_hlim	= 
        IPV6CTL_GIF_HLIM_CFG;
    IPv6DefaultConfigParams.cfg_ip6_use_deprecated	= 
        IPV6CTL_USE_DEPRECATED_CFG;
    IPv6DefaultConfigParams.cfg_ip6_rr_prune	= 
        IPV6CTL_RR_PRUNE_CFG;
    IPv6DefaultConfigParams.cfg_ip6_prefer_tempaddr     = 
        IPV6CTL_PREFER_TEMPADDR_CFG;
    IPv6DefaultConfigParams.cfg_ip6_mcast_pmtu		= 
        IPV6CTL_MCAST_PMTU_CFG;
    IPv6DefaultConfigParams.cfg_ip6_v6only		= 
        IPV6CTL_V6ONLY_CFG;
    IPv6DefaultConfigParams.cfg_ip6_keepfaith	= 
        IPV6CTL_KEEPFAITH_CFG;
    IPv6DefaultConfigParams.cfg_rtq_reallyold	= 
        IPV6CTL_RTEXPIRE_CFG;
    IPv6DefaultConfigParams.cfg_rtq_minreallyold	= 
        IPV6CTL_RTMINEXPIRE_CFG;
    IPv6DefaultConfigParams.cfg_rtq_toomany		= 
        IPV6CTL_RTMAXCACHE_CFG;
    IPv6DefaultConfigParams.cfg_rtq_timeout		= 
        IPV6CTL_RTTIMEOUT_CFG;
    IPv6DefaultConfigParams.cfg_ipv6RtDispTable	= &IPV6_RT_DISP_TBL;
    IPv6DefaultConfigParams.cfg_ip6qmaxlen		= 
	IP6INTRQ_IFQ_MAXLEN_CFG;

#ifdef INCLUDE_NET_SYSCTL
    /* Initialize the sysctl node */
    IPv6DefaultConfigParams.cfg_privInitSysctl   = in6ProtoSysctlInit;
#else
    IPv6DefaultConfigParams.cfg_privInitSysctl   = NULL;
#endif

#ifdef VIRTUAL_STACK     
    registered = vsComponentRegister
        (
        &ipv6RegistrationNum,
        "IPv6",
        VS_SC_ENABLED,
        VS_SC_NON_OPTIONAL,
        ipv6InstInit,
        sizeof (IPV6_CONFIG_PARAMS),
        &IPv6DefaultConfigParams,
        ipv6Destructor
        );
 
#else 

    ipv6InstInit (&IPv6DefaultConfigParams);

#endif /*VIRTUAL_STACK*/
    }

#endif /* INCLUDE_IPV6 */

#ifdef INCLUDE_MIPV6

void usrNetMipv6Init (void)
    {
    mipv6DefaultConfigParams.cfgh.len = sizeof(MIPV6_CONFIG_PARAMS);
    mipv6DefaultConfigParams.cfg_mip6ctl_debug = MIPV6CTL_DEBUG_CFG;
    mipv6DefaultConfigParams.cfg_mip6ctl_use_ipsec = MIPV6CTL_USE_IPSEC_CFG;
    mipv6DefaultConfigParams.cfg_mip6ctl_rr_hint_ppslim =
                                             MIPV6CTL_RR_HINT_PPSLIM_CFG;

#ifdef INCLUDE_NET_SYSCTL
    /* Initialize the sysctl node */
    mipv6DefaultConfigParams.cfg_privInitSysctl = mip6ProtoSysctlInit;
#else
    mipv6DefaultConfigParams.cfg_privInitSysctl = NULL;
#endif

#ifdef VIRTUAL_STACK     
    (void)vsComponentRegister (&mipv6RegistrationNum, "MIPv6",
                               VS_SC_ENABLED, VS_SC_NON_OPTIONAL,
                               mipv6InstInit, sizeof (MIPV6_CONFIG_PARAMS),
                               &mipv6DefaultConfigParams, mipv6Destructor);
 
#else 
    mipv6InstInit (&mipv6DefaultConfigParams);
#endif /*VIRTUAL_STACK*/
    }

#endif /* INCLUDE_MIPV6 */

#ifdef INCLUDE_UDPV6

void usrNetUdpv6Init (void)
    {
    bzero ((char *)&udp6DefaultConfigParams, sizeof (UDP6_CONFIG_PARAMS));
    udp6DefaultConfigParams.cfgh.len = sizeof (UDP6_CONFIG_PARAMS);

#ifdef INCLUDE_NET_SYSCTL
    /* Initialize the sysctl node */
    udp6DefaultConfigParams.cfg_privInitSysctl   = udpv6SysctlInit;
#else
    udp6DefaultConfigParams.cfg_privInitSysctl   = NULL;
#endif /* INCLUDE_NET_SYSCTL */

#ifdef VIRTUAL_STACK
    vsComponentRegister (&udpv6RegistrationNum, "UDPv6", VS_SC_ENABLED,
                         VS_SC_NON_OPTIONAL, udpv6InstInit,
                         sizeof (UDP6_CONFIG_PARAMS),
                         &udp6DefaultConfigParams, NULL);
#else
    udpv6InstInit (&udp6DefaultConfigParams);
#endif /* VIRTUAL_STACK */
    }

#endif    /* INCLUDE_UDPV6 */



#ifdef INCLUDE_TCPV6

/*
 * priv_tcp6_min_mtu should be a non-negative
 * multiple of 4 no greater than IPV6_MMTU (1280).
 * Setting priv_tcp6_min_mtu to IPV6_MMTU will prevent TCP
 * from decreasing its MSS estimate below that appropriate
 * to the IPv6 minimum MTU.  Instead, a fragment header will
 * be included and fragmentation may occur at IPv6->IPv4 translating
 * routers where the egress MTU is less than 1280.
 * See tcp_mtudisc().
 * UNVIRTUALIZED, use the same policy in all virtual stacks.
 */
int priv_tcp6_min_mtu = TCPV6_MTU_MIN_CFG;

void usrNetTcpv6Init (void)
    {
    bzero ((char *)&tcp6DefaultConfigParams, sizeof (TCP6_CONFIG_PARAMS));
    tcp6DefaultConfigParams.cfgh.len = sizeof (TCP6_CONFIG_PARAMS);
    tcp6DefaultConfigParams.cfg_tcp_v6mssdflt	= TCP_V6MSSDFLT_CFG;

#ifdef INCLUDE_NET_SYSCTL
    /* Initialize the sysctl node */
    tcp6DefaultConfigParams.cfg_privInitSysctl   = tcpv6SysctlInit;
#else
    tcp6DefaultConfigParams.cfg_privInitSysctl   = NULL;
#endif /* INCLUDE_NET_SYSCTL */

#ifdef VIRTUAL_STACK
    vsComponentRegister (&tcpv6RegistrationNum, "TCPv6", VS_SC_ENABLED,
                         VS_SC_NON_OPTIONAL, tcpv6InstInit,
                         sizeof (TCP6_CONFIG_PARAMS),
                         &tcp6DefaultConfigParams, NULL);
#else
    tcpv6InstInit(&tcp6DefaultConfigParams);
#endif /* VIRTUAL_STACK */

    }

#endif    /* INCLUDE_TCPV6 */


#ifdef INCLUDE_MROUTEV6
void usrNetIpMroutev6Init (void)
    {
    bzero ((char *) &ipm6DefaultConfigParams, sizeof (IPM6_CONFIG_PARAMS));
    ipm6DefaultConfigParams.cfgh.len = sizeof (IPM6_CONFIG_PARAMS);
    
#ifdef INCLUDE_NET_SYSCTL
    /* Initialize the sysctl node */
    ipm6DefaultConfigParams.cfg_privInitSysctl = ip6MrouteSysctlInit;
#else
    ipm6DefaultConfigParams.cfg_privInitSysctl = NULL;
#endif /* INCLUDE_NET_SYSCTL */

#ifdef VIRTUAL_STACK
    vsComponentRegister (&mroutev6RegistrationNum, "MROUTEv6", VS_SC_ENABLED,
                         VS_SC_NON_OPTIONAL, mroutev6InstInit,
                         sizeof (IPM6_CONFIG_PARAMS),
                         &ipm6DefaultConfigParams, mroutev6Destructor);
#else
    (void)mroutev6InstInit (&ipm6DefaultConfigParams);
#endif /* VIRTUAL_STACK */

    return;
    }
#endif /* INCLUDE_MROUTEV6 */

#ifdef INCLUDE_FTP6_SERVER

#ifdef VIRTUAL_STACK

/*  variables to be set to values from cdf params in the configlette */

extern char *ftpd6optionsstring;
extern FUNCPTR ftpd6loginrtn;
extern int ftpd6ssize;

extern VS_REG_ID ftpd6RegistrationNum;

extern STATUS ftpd6InstInit (void *noparams);
extern STATUS ftpd6Destructor (VSNUM vsnum);
#endif /* VIRTUAL_STACK */
extern STATUS ftpd6GuestAllow (const char *, const char *, BOOL);
extern STATUS ftpd6GuestIdSet (const char *, u_int);

LOCAL STATUS usrFtpd6Start (void)
    {
    STATUS  rc = OK;
    
#ifdef VIRTUAL_STACK
    /* first set global variables to the CDF param values for later use */

    ftpd6optionsstring = FTPD6_OPTIONS_STRING;
    ftpd6loginrtn = FTPD6_LOGINRTN;
    ftpd6ssize = FTPD6_SSIZE;
    
    /*
     * Register at vsmanager to allow ftp6 server to be started in
     * a chosen virtual stack; it can only be started in one stack.
     */
    
    rc = vsApplicationRegister (&ftpd6RegistrationNum, "ftpd6",
                               VS_SC_DISABLED, ftpd6InstInit,
                               0, NULL, ftpd6Destructor);
    if (rc == VS_OK)

        {
#if (FTP6_SERVER_IN_VS0 == TRUE) /* it must be enabled for instance 0 */
        rc = vsComponentStatusSet (0, ftpd6RegistrationNum, VS_SC_ENABLED,
                                   NULL);
#endif
        }
    else
        return (ERROR);			
#else
    ftpd6Init (FTPD6_OPTIONS_STRING, FTPD6_LOGINRTN, FTPD6_SSIZE);
#endif /* VIRTUAL_STACK */

#if defined (INCLUDE_FTPD6_SECURITY)
    ftpd6EnableSecurity ();
#endif /* INCLUDE_FTPD6_SECURITY */

#ifdef INCLUDE_FTPD6_GUEST_LOGIN
    
    ftpd6GuestIdSet (FTPD6_GUEST_1, 0);
    ftpd6GuestIdSet (FTPD6_GUEST_2, 1);
    ftpd6GuestIdSet (FTPD6_GUEST_3, 2);

#if defined (FTPD6_GUEST_ROOT_DIR) && defined (FTPD6_GUEST_UPLOAD_DIR)
    ftpd6GuestAllow(FTPD6_GUEST_ROOT_DIR, FTPD6_GUEST_UPLOAD_DIR,
                    FTPD6_GUEST_DIR_VALIDATE);
#else
    #error Guest login enabled, but directories not defined
#endif   /* FTPD6_GUEST_ROOT_DIR and FTPD6_GUEST_UPLOAD_DIR */

#endif /* INCLUDE_FTPD6_GUEST_LOGIN */
  
    return (rc);
    }
#endif /* INCLUDE_FTP6_SERVER */

#ifdef INCLUDE_RDISC

RDISC_CFG_PARAMS rdiscDefaultConfigParams;

#ifdef VIRTUAL_STACK
VS_REG_ID rdiscRegNum;
extern STATUS rdiscDestructor (VSNUM vsnum);
#endif /* VIRTUAL_STACK */

LOCAL STATUS usrRdiscInit
    (
    int priority,
    int options,
    int stacksize
    )
    {
#ifdef VIRTUAL_STACK        
    rdiscLibInit();
#endif    
	
    bzero ((char *) &rdiscDefaultConfigParams, sizeof (rdiscDefaultConfigParams));
    rdiscDefaultConfigParams.cfgh.len = sizeof(rdiscDefaultConfigParams);
    rdiscDefaultConfigParams.cfg_rdisc_priority     = priority;
    rdiscDefaultConfigParams.cfg_rdisc_options      = options;
    rdiscDefaultConfigParams.cfg_rdisc_stacksize    = stacksize;
    
#ifdef VIRTUAL_STACK
    if (vsApplicationRegister (&rdiscRegNum, "RDISC",
                               VS_SC_DISABLED, rdiscInstInit,
                               sizeof (rdiscDefaultConfigParams),
                               &rdiscDefaultConfigParams,
                               rdiscDestructor) == VS_OK)
        {
#if (RDISC_IN_VS0 == TRUE)   /* it must be enabled for instance 0 */
        return vsComponentStatusSet (0, rdiscRegNum, VS_SC_ENABLED,
                                     &rdiscDefaultConfigParams);
#else
        return (OK);
#endif
        }
    else
        {
        if (_func_printErr)
            (*_func_printErr) ("usrRdiscInit: VS Registration failed\n");
        return (ERROR);
        }
#else
    if (rdiscInstInit (&rdiscDefaultConfigParams) == ERROR)
        {
        if (_func_printErr)
            (*_func_printErr) ("usrRdiscInit: rdiscInstInit failed\n");

        return (ERROR);
        }
    return (OK);
#endif /* VIRTUAL_STACK */
    }
#endif /* INCLUDE_RDISC*/

#ifdef  INCLUDE_NET_HOST_SETUP
/*******************************************************************************
*
* usrNetHostAdd - assign local hostname to target
*
* This routine stores the hostname for the local vxWorks target and
* initializes the sysctl interface to retrieve that information.
*
* The MIB-II system group uses this name as a public identifier.
*
* RETURNS: OK, or ERROR if setup failed.
*
* NOMANUAL
*/

STATUS usrNetHostAdd (void)
    {
    STATUS result;

    result = hostnameSetup (pTgtName);

#ifdef INCLUDE_NET_SYSCTL

#ifdef VIRTUAL_STACK
    /*
     * If using virtual stacks, then don't register the sysctl variables
     * unless starting the management stack since the kernel node is
     * global and not virtualized.
     */
 
    if (myStackNum != VS_MGMT_STACK)
        return (result);
#endif /* VIRTUAL_STACK */

    if (result == OK)
        hostnameSysctlSetup();
#endif /* INCLUDE_NET_SYSCTL */

    return (result);    
    }
#endif   /* INCLUDE_NET_HOST_SETUP */

/*
  DESCRIPTION
  This file is used to configure and initialize the VxWorks networking support
  for remote I/O access. The file contents are added to the project 
  configuration file when FOLDER_NET_REM_IO is included.  
  
  NOMANUAL

*/


#ifdef INCLUDE_NET_REM_IO

extern int remLastResvPort;
/*******************************************************************************
*
* usrNetRemoteCreate - enable remote I/O access over the network
*
* This routine creates a device for network remote I/O access if
* INCLUDE_NET_REM_IO. It is called by the configuration tool as the 
* initialization routine for that component.
* 
* RETURNS: N/A
*
* NOMANUAL
*/



void usrNetRemoteCreate (void)
    {
#if (!(defined(INCLUDE_PASSFS)) && !(defined(INCLUDE_NTPASSFS)))
    char        devName [MAX_FILENAME_LENGTH];  /* device name */
    int 	protocol;
#endif	/* !INCLUDE_PASSFS & !INCLUDE_NTPASSFS */

    remLastResvPort = 1010;     /* pick an unused port number so we don't *
                                 * have to wait for the one used by the *
                                 * by the bootroms to time out */

#ifdef INCLUDE_FTP
    _func_ftpLs = ftpLs;               /* init ptr to ftp dir listing routine */
#else
    _func_ftpLs = NULL; 
#endif

#ifdef INCLUDE_REMLIB
#ifdef INCLUDE_PCCARD
    csClientLoad (CS_END_ENABLER);
#endif /* INCLUDE_PCCARD */

    _func_remCurIdGet = (FUNCPTR) remCurIdGet;
    _func_remCurIdSet = (FUNCPTR) remCurIdSet;
#else
    _func_remCurIdGet = NULL;
    _func_remCurIdSet = NULL;
#endif

    /* initialize the network remote file driver */

    netDrv ();

#if (!(defined(INCLUDE_PASSFS)) && !(defined(INCLUDE_NTPASSFS)))
    /*
     * Create netDrv device.
     * NOTE: This is not done when PASSFS is included. Moreover, the device
     * names for passFs and netDrv are the same, e.g. <hostname>, this
     * would bring conflicts.
     */

    _func_netLsByName = netLsByName;	/* init ptr to netDrv listing routine */
    sprintf (devName, "%s:", sysBootParams.hostName);	/* make dev name */
    protocol = (sysBootParams.passwd[0] == EOS) ? 0 : 1;/* pick protocol */

    /* Create device. */

    netDevCreate (devName, sysBootParams.hostName, protocol);
#endif	/* !INCLUDE_PASSFS && !INCLUDE_NTPASSFS */
    
#ifdef INCLUDE_REMLIB
    /* Set the user id and current directory. */

    iam (sysBootParams.usr, sysBootParams.passwd);
#endif

#if (!(defined(INCLUDE_PASSFS)) && !(defined(INCLUDE_NTPASSFS)))   
    /* Set the current default path, for PassFs this done by usrPassFsInit() */

    ioDefPathSet (devName);
#endif	/* !INCLUDE_PASSFS & !INCLUDE_NTPASSFS */
    return;    
    }

#endif /* INCLUDE_NET_REM_IO */


#ifdef INCLUDE_ARP_API

void usrNetArpLibInit (void)
    {
	
    arpLibInit();

    /* Add 5 static arp entries */

#if (defined (ARP_ENTRY_1_HOST) && defined (ARP_ENTRY_1_ADDR))
    {
    char * arpHost = ARP_ENTRY_1_HOST;
    char * arpAddr = ARP_ENTRY_1_ADDR;

    if (arpHost && arpAddr)
        arpAdd (ARP_ENTRY_1_HOST, ARP_ENTRY_1_ADDR, ATF_PERM);
    }
#endif

#if (defined ARP_ENTRY_2_HOST && defined ARP_ENTRY_2_ADDR)
    {
    char * arpHost = ARP_ENTRY_2_HOST;
    char * arpAddr = ARP_ENTRY_2_ADDR;

    if (arpHost && arpAddr)
        arpAdd (ARP_ENTRY_2_HOST, ARP_ENTRY_2_ADDR, ATF_PERM);
    }
#endif

#if (defined ARP_ENTRY_3_HOST && defined ARP_ENTRY_3_ADDR)
    {
    char * arpHost = ARP_ENTRY_3_HOST;
    char * arpAddr = ARP_ENTRY_3_ADDR;

    if (arpHost && arpAddr)
        arpAdd (ARP_ENTRY_3_HOST, ARP_ENTRY_3_ADDR, ATF_PERM);
    }
#endif

#if (defined ARP_ENTRY_4_HOST && defined ARP_ENTRY_4_ADDR)
    {
    char * arpHost = ARP_ENTRY_4_HOST;
    char * arpAddr = ARP_ENTRY_4_ADDR;

    if (arpHost && arpAddr)
        arpAdd (ARP_ENTRY_4_HOST, ARP_ENTRY_4_ADDR, ATF_PERM);
    }
#endif

#if (defined ARP_ENTRY_5_HOST && defined ARP_ENTRY_5_ADDR)
    {
    char * arpHost = ARP_ENTRY_5_HOST;
    char * arpAddr = ARP_ENTRY_5_ADDR;

    if (arpHost && arpAddr)
        arpAdd (ARP_ENTRY_5_HOST, ARP_ENTRY_5_ADDR, ATF_PERM);
    }
#endif
    }

#endif /* INCLUDE_ARP_API */


#ifdef INCLUDE_LOOPBACK

/*******************************************************************************
*
* usrNetLoopbackStart - init the loopback device
*
* This routine attaches the loopback driver with IP address 127.0.0.1 and
* host name "localhost".
*
* RETURNS: N/A
*
* NOMANUAL
*/

void usrNetLoopbackStart (void)
    {

#ifndef VIRTUAL_STACK

    /* Non-virtual stack */

    if (_func_printErr)
	    (*_func_printErr) ("Attaching interface lo0... ");

    loopbackInstInit (NULL);
#ifdef INCLUDE_IPV6
    ip6_init2(0);
#endif

    if (_func_printErr)
	    (*_func_printErr) ("done\n");

    return;

#else /* Is VIRTUAL_STACK */
    VS_REG_ID loVsRegNum;
#ifdef INCLUDE_IPV6
    VS_REG_ID ip6init2VsRegNum;
#endif /* INCLUDE_IPV6 */

    vsComponentRegister (&loVsRegNum,
                         "loopback",
                         VS_SC_ENABLED,
                         VS_SC_NON_OPTIONAL,
                         loopbackInstInit,
                         0,
                         NULL,
                         NULL);

#ifdef INCLUDE_IPV6
    vsComponentRegister (&ip6init2VsRegNum,
                         "ip6_init2",
                         VS_SC_ENABLED,
                         VS_SC_NON_OPTIONAL,
                         usrVsWrapperIp6_init2,
                         0,
                         NULL,
                         NULL);
                           
    return;
#endif /* INCLUDE_IPV6 */               
                                      
#endif /* VIRTUAL_STACK */
    }


#ifdef VIRTUAL_STACK
#ifdef INCLUDE_IPV6
/*******************************************************************************
*
* usrVsWrapperIp6_Init2 - Virtualisation wrapper for ip6_init2().
*
* This is used to provide a VS-friendly interface to ip6_init2() so that it
* can be registered with vsComponentRegister().
*
* RETURNS: OK
*
* NOMANUAL
*/
STATUS usrVsWrapperIp6_init2 (void *cfgParams)
    {
    ip6_init2(0);
    return (OK);
    }
#endif /* INCLUDE_IPV6 */
#endif /* VIRTUAL_STACK */

#endif /*  INCLUDE_LOOPBACK */


#ifdef INCLUDE_ETHERNET

STATUS usrNetEther (void)
    {
    /* Register ADDR_RES_FUNC */

    if (muxIfFuncAdd (M2_ifType_ethernet_csmacd, 0x800, ADDR_RES_FUNC,
                      (FUNCPTR)ipEtherResolvRtn) != OK)
	{
        if (_func_printErr)
            (* _func_printErr) ("usrNetEther: can't register ADDR_RES_FUNC for IPv4\n");
        return (ERROR);
	}

#ifdef INCLUDE_IPV6
    /* Register IPv6 ADDR_RES_FUNC */

    if (muxIfFuncAdd (M2_ifType_ethernet_csmacd, 0x86dd, ADDR_RES_FUNC,
                      (FUNCPTR)ip6EtherResolvRtn) != OK)
	{
        if (_func_printErr)
            (* _func_printErr) ("usrNetEther: can't register ADDR_RES_FUNC for IPv6\n");
        return (ERROR);
	}
#endif
    /* Register IPv4 IF_OUTPUT_FUNC */

    if (muxIfFuncAdd (M2_ifType_ethernet_csmacd, 0x800, IF_OUTPUT_FUNC,
		      (FUNCPTR)ether_output) != OK)
	{
        if (_func_printErr)
            (* _func_printErr) ("usrNetEther: can't register IF_OUTPUT_FUNC\n");
	return (ERROR);
	}

    /* Register MULTI_ADDR_RES_FUNC */
	
    if (muxIfFuncAdd (M2_ifType_ethernet_csmacd, 0x800, MULTI_ADDR_RES_FUNC,
                      (FUNCPTR)ether_resolvemulti) != OK)
	{
        if (_func_printErr)
            (* _func_printErr) ("usrNetEther: can't register MULTI_ADDR_RES_FUNC\n");
	return (ERROR);
	}

#ifdef INCLUDE_IPV6
    /* Register IPv6 IF_OUTPUT_FUNC */

    if (muxIfFuncAdd (M2_ifType_ethernet_csmacd, 0x86dd, IF_OUTPUT_FUNC,
		      (FUNCPTR)ether_output) != OK)
	{
        if (_func_printErr)
            (* _func_printErr) ("usrNetEther: can't register IPv6 IF_OUTPUT_FUNC\n");
	return (ERROR);
	}

    /* register IPv6 MULTI_ADDR_RES_FUNC */
	
    if (muxIfFuncAdd (M2_ifType_ethernet_csmacd, 0x86dd, MULTI_ADDR_RES_FUNC,
                      (FUNCPTR)ether_resolvemulti) != OK)
	{
        if (_func_printErr)
            (* _func_printErr) ("usrNetEther: can't register IPv6 MULTI_ADDR_RES_FUNC\n");
	return (ERROR);
	}
#endif /* INCLUDE_IPV6 */

    return (OK);
    }
#endif /* INCLUDE_ETHERNET */


#ifdef INCLUDE_END


STATUS usrNetEndLibInit()
    {
    int			count;
    END_TBL_ENTRY *	pDevTbl;
    void *		pCookie = NULL;

    /* Add in mux ENDs. */
    for (count = 0, pDevTbl = endDevTbl; pDevTbl->endLoadFunc != END_TBL_END;
         pDevTbl++, count++)
        {
        /* Make sure that WDB has not already installed the device. */
        if (!pDevTbl->processed)
            {
            pCookie = muxDevLoad (pDevTbl->unit,
                                  pDevTbl->endLoadFunc,
                                  pDevTbl->endLoadString,
                                  pDevTbl->endLoan, pDevTbl->pBSP);
            if (pCookie == NULL)
		{
	       	if (_func_printErr)
                    (* _func_printErr)
			("muxDevLoad failed for device entry %d!\n", count);
		}
            else
                {
                if (muxDevStart(pCookie) == ERROR)
		    {
		    if (_func_printErr)
                        (* _func_printErr)
		            ("muxDevStart failed for entry %d!\n", count);
		    }
		else
                    {
                    pDevTbl->processed = TRUE;
#ifdef INCLUDE_END_POLLED_STATS
                    endPollStatsInit (pCookie, END_POLL_STATS_ROUTINE_HOOK);
#endif /* INCLUDE_END_POLLED_STATS */
                    }
                }
            }
        }
    netDevBootFlag = TRUE;
    return (OK);    
    }

#endif /* INCLUDE_END */


#ifdef INCLUDE_IPATTACH  

/*******************************************************************************
*
* usrNetIpAttachCommon - attach a network device
*
* RETURNS: N/A
*
* NOMANUAL
*/

void usrNetIpAttachCommon
    (
    char *    pDeviceName,	/* Device name */
    int       unitNum,		/* unit number */
    char *    pStr,		/* e.g. "IPv4", "IPv6" */
    FUNCPTR   pAttachRtn	/* attach routine */
    )
    {
    int unitNumber = unitNum;


    if ((pDeviceName == NULL) || (*pDeviceName == 0))
        return;

    if (endFindByName (pDeviceName, unitNum) == NULL)
        {
        if (sysBootParams.other [0] != EOS)
            {
            char *pStr;

            /*
             * the other parameter may or may not contain a
             * unit number.  If there is one, use it 
             * and update sysBootParams.unitNum.
             */

            pStr = (char *)&sysBootParams.other;
            while (!isdigit (*pStr) && *pStr != EOS)
                pStr++;

            if (pStr != EOS && sscanf (pStr, "%d", &unitNumber) == 1)
                {
                sysBootParams.unitNum = unitNumber;
                uNum = unitNumber;
                *pStr = EOS;
                }

            pDevName = sysBootParams.other;
            }
        else
            pDevName = NULL;

        pDeviceName = pDevName;
        }

    if (pDeviceName == NULL || pAttachRtn == NULL)
        {
        if ( _func_printErr && pDeviceName == NULL)
            (* _func_printErr) ("usrNetIpAttachCommon: unknown network device\n");
        return;
        }

    if ((*pAttachRtn) (unitNumber, pDeviceName) != OK)
        {
        if ( _func_printErr)
            (*_func_printErr) ("Failed to attach to device %s%d",
	                        pDeviceName, unitNumber);
        return;
        }

    netAttachFlag = TRUE;		/* usrNetBoot.c */

    if (_func_printErr)
        (*_func_printErr)
	    ("Attached %s interface to %s unit %d\n",pStr,pDeviceName,unitNumber);

    return;
    }



#endif /* INCLUDE_IPATTACH */



#ifdef INCLUDE_IP6ATTACH
STATUS usrNetIpv6Attach
    (
    char *	pDeviceName,
    int		unitNum
    )
    {
    usrNetIpAttachCommon (pDeviceName, unitNum,"IPv6", ip6Attach);

#if (IFF_INET6_UP_CFG > 0)
    do_autoconfig (pDeviceName, unitNum);
#endif

    return OK;
    }


LOCAL STATUS  do_autoconfig(char * devName, int unit){

    char ifname [20];
    int                 s;
    struct ifreq        ifr;
    
    bzero (ifname, sizeof (ifname));
    sprintf (ifname, "%s%d", devName, unit);
    if (ifname == NULL) 
        return (EINVAL);
    if ((s = socket (AF_INET6, SOCK_DGRAM, 0)) < 0)
        {
	if (_func_printErr)
	    (*_func_printErr) ("do_autoconfig: failed to create a socket\n");
        return (ERROR);
        }

    bzero ((caddr_t)&ifr, sizeof (ifr));
    strcpy (ifr.ifr_name, ifname);

    if (ioctl(s, (int) SIOCGIFFLAGS, (int)&ifr) < 0) {
      if (_func_printErr)
	    (*_func_printErr) ("do_autoconfig: ioctl (SIOCGIFFLAGS)\n");
      close (s);
      return (ERROR);
    }
    ifr.ifr_flags |= IFF_INET6_UP;
    if(ioctl (s, (int) SIOCSIFFLAGS, (int)&ifr) < 0) {
      if (_func_printErr)
	    (*_func_printErr) ("do_autoconfig: ioctl (SIOCSIFFLAGS)\n");
      close (s);
      return (ERROR);
    }

    close (s);
    return (OK);
}

#endif /* INCLUDE_IP6ATTACH */


#ifdef INCLUDE_NET_BOOT_CONFIG
/* extern */
extern u_long random (void);

#ifndef SIN
#define SIN(s) ((struct sockaddr_in *)s)
#endif


/*******************************************************************************
*
* addGateway - add default gateway for IPv4 network
*
* RETURNS: N/A
*
* NOMANUAL
*/

LOCAL STATUS addGateway
    (
    char *	pDst,
    char *	pGtw
    )
    {
    struct sockaddr_in	dst, mask, gate;

    /* initialize the sockaddrs */
    bzero ((char *)&dst, sizeof (dst));
    bzero ((char *)&gate, sizeof (gate));
    bzero ((char *)&mask, sizeof (mask));

    dst.sin_family      = AF_INET;
    dst.sin_len         = sizeof (struct sockaddr_in);
    dst.sin_addr.s_addr = inet_addr (pDst);
    if (dst.sin_addr.s_addr == ERROR)
	return (ERROR);

   /* initialize gateway */

    gate.sin_family      = AF_INET;
    gate.sin_len         = sizeof (struct sockaddr_in);
    gate.sin_addr.s_addr = inet_addr (pGtw);
    if (gate.sin_addr.s_addr == ERROR)
	return (ERROR);

   /* initialize netmask */

    mask.sin_family      = AF_INET;
    mask.sin_len         = sizeof (struct sockaddr_in);
    mask.sin_addr.s_addr = 0;

    return (ipRouteCreate ((struct sockaddr *)&dst, (struct sockaddr *)&mask, 
                           (struct sockaddr *)&gate, 0, 0, 
                           M2_ipRouteProto_other,
                           ROUTE_KERNEL, ROUTE_USAGE_IPV4,
                           0, 0, NULL, NULL, NULL));
    }


/*******************************************************************************
*
* netIoctl - configure the network boot device
*
* This routine is the initialization routine for the INCLUDE_NET_BOOT_CONFIG
* component. It assigns the IP address, netmask and default gateway if
* specified to the boot device.
*
* RETURNS: N/A
*
* NOMANUAL
*/


LOCAL int netIoctl
    (
    char *      ifname,
    u_int       cmd,
    int         addr            /* must be network byte order */
    )
    {
    int                 s, err;
    struct ifreq        ifr;

    if (ifname == NULL || cmd == 0)
        return (EINVAL);

    if ((s = socket (AF_INET, SOCK_RAW, 0)) < 0)
        {
	if (_func_printErr)
	    (*_func_printErr) ("netIoctl: failed to create a socket\n");
        return (ERROR);
        }
    bzero ((caddr_t)&ifr, sizeof (ifr));
    strcpy (ifr.ifr_name, ifname);
    SIN(&ifr.ifr_addr)->sin_len = sizeof (struct sockaddr_in);
    SIN(&ifr.ifr_addr)->sin_family = AF_INET;
    SIN(&ifr.ifr_addr)->sin_addr.s_addr = addr;

    err = ioctl (s, cmd, (int)&ifr);
    close (s);
    return (err);
    }


/*******************************************************************************
*
* usrNetBootConfig - configure the network boot device
*
* This routine is the initialization routine for the INCLUDE_NET_BOOT_CONFIG
* component. It assigns the IP address, netmask and default gateway if
* specified to the boot device.
*
* RETURNS: N/A
*
* NOMANUAL
*/

STATUS usrNetBootConfig
    (
    char *      devName,                /* device name e.g. "fei" */
    int         unit,			/* unit number */
    char *      addr,                   /* target ip address */
    int         netmask,                /* subnet mask */
    char *	gateway			/* default gateway */
    )
    {
    char ifname [20];
    int		ipaddr;
    int s;
    struct ifreq ifr;
    /*
     * Do nothing if another device is already configured or an
     * error was detected in the boot parameters.
     */

    if (devName == NULL || devName[0] == EOS || addr == NULL ||
	addr[0] == EOS)
    	{
	if (_func_printErr)
	    (*_func_printErr) ("usrNetBootConfig: Invalid Argument\n");
	return (ERROR);
	}

    /* build interface name */

    bzero (ifname, sizeof (ifname));
    sprintf (ifname, "%s%d", devName, unit);
    
    /* set subnet mask, if any specified */

    if (netmask &&
	netIoctl (ifname, SIOCSIFNETMASK, htonl(netmask)) != 0)
	{
	if (_func_printErr)
	    (*_func_printErr) ("usrNetBootConfig: Failed SIOCSIFNETMASK for %s\n", ifname);
	return (ERROR);
	}
    
    /* set inet addr */

    ipaddr = inet_addr(addr);      /* htonl is done inside inet_addr */
    if (netIoctl(ifname, SIOCSIFADDR, ipaddr) != 0)
	{
	if (_func_printErr)
	    (*_func_printErr) ("usrNetBootConfig: Failed SIOCSIFADDR for %s\n", ifname);
	return (ERROR);
	}

	/* Set IFF_INET_UP */
	if ((s = socket (AF_INET, SOCK_RAW, 0)) < 0)
	        {
		if (_func_printErr)
		    (*_func_printErr) ("usrNetBootConfig: failed to create a socket\n");
	        return (ERROR);
	        }
	
	    bzero ((caddr_t)&ifr, sizeof (ifr));
	    strcpy (ifr.ifr_name, ifname);
	
	    if (ioctl(s, (int)SIOCGIFFLAGS, (int)&ifr) != 0) {
	      if (_func_printErr)
		    (*_func_printErr) ("usrNetBootConfig: ioctl (SIOCGIFFLAGS)\n");
	      close (s);
	      return (ERROR);
	    }
	    ifr.ifr_flags |= IFF_INET_UP;
	    if(ioctl (s, (int)SIOCSIFFLAGS, (int)&ifr) != 0) {
	      if (_func_printErr)
		    (*_func_printErr) ("usrNetBootConfig: ioctl (SIOCSIFFLAGS)\n");
	      close (s);
	      return (ERROR);
	}
	close (s);


    /* set default gateway from give boot parameter */

    if (gateway != NULL && gateway[0] != EOS)
        {
	if (addGateway ("0.0.0.0", gateway) == ERROR)
	    {
	    if (_func_printErr)
	        (*_func_printErr) ("usrNetBootConfig: Failed addGateway\n");
	    return (ERROR);
            }
        }

    return (OK);
    }

#endif /* INCLUDE_NET_BOOT_CONFIG */



#ifdef   INCLUDE_DNS_RESOLVER 

LOCAL STATUS usrDnsInit (void)
    {
    bzero ( (char *)&dnsDefaultConfigParams, sizeof (DNS_CONFIG_PARAMS));
    dnsDefaultConfigParams.cfgh.len = sizeof (DNS_CONFIG_PARAMS);

    /* resolvInit() */
    dnsDefaultConfigParams.cfg_pNameServer = RESOLVER_DOMAIN_SERVER;
    dnsDefaultConfigParams.cfg_pDefaultDomainName = RESOLVER_DOMAIN;
    dnsDefaultConfigParams.cfg_debugFlag = DNS_DEBUG;
    /* resolvLib.c; Resolver state default settings. _res */
    dnsDefaultConfigParams.cfg_res.retrans = RES_TIMEOUT_CFG;
    dnsDefaultConfigParams.cfg_res.retry = RETRY_CFG;
    dnsDefaultConfigParams.cfg_res.options = RES_OPTIONS_CFG;
    dnsDefaultConfigParams.cfg_res.nscount = NSCOUNT_CFG;
	
#ifdef VIRTUAL_STACK
    if (vsApplicationRegister (&dnsRegistrationNum, "DNS",
                               VS_SC_DISABLED, dnsInstInit,
                               sizeof (DNS_CONFIG_PARAMS),
                               &dnsDefaultConfigParams,
                               dnsDestructor) == VS_OK)
        {
#if (DNS_CLIENT_IN_VS0 == TRUE)  /* it must be enabled for instance 0 */
        return vsComponentStatusSet (0, dnsRegistrationNum, VS_SC_ENABLED,
				     &dnsDefaultConfigParams);
#else
        return (OK);
#endif
        }
    else
        return (ERROR);
#else
    return dnsInstInit (&dnsDefaultConfigParams);
#endif
    }
#endif /* INCLUDE_DNS_RESOLVER */

#ifdef INCLUDE_BOOT_LINE_INIT

extern int sysStartType;

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
    if ((startType & BOOT_CLEAR) || (*BOOT_LINE_ADRS == EOS))
        {
        /*
         * Either cold boot or empty boot line --
         * initialize boot line if not given params by '$' boot command
         */

        if ( *BOOT_LINE_ADRS == EOS )
            {
            if ((sysNvRamGet (BOOT_LINE_ADRS, BOOT_LINE_SIZE, 0) == ERROR))
                {
                /* no non-volatile RAM -- use default boot line */

                strcpy (BOOT_LINE_ADRS, DEFAULT_BOOT_LINE);
                }
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

#ifdef	INCLUDE_NET_CLARINET
    /* check inet addresses */

    if ((checkInetAddrField (pParams->ead, TRUE) != OK) ||
	(checkInetAddrField (pParams->bad, TRUE) != OK) ||
	(checkInetAddrField (pParams->had, FALSE) != OK) ||
	(checkInetAddrField (pParams->gad, FALSE) != OK))
	{
	return (ERROR);
	}
#endif

    return (OK);
    }

#ifdef	INCLUDE_NET_CLARINET
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
#endif

/******************************************************************************
*
* usrBootLineParse - parse the boot line
*/ 

STATUS usrBootLineParse
    (
    char* bootString
    )
    {
    BOOT_PARAMS	params;

    usrBootLineInit (sysStartType);     /* crack the bootline */

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

    if ((params.startupScript [0] == EOS) &&	/* startup script (s) */
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

    return OK;
    }


#endif /* INCLUDE_BOOT_LINE_INIT */

#ifdef VIRTUAL_STACK
extern STATUS ifDestructor(VSNUM);
extern VS_REG_ID ifRegistrationNum;
#endif

extern STATUS ifInstInit (void *ifInitValues);

extern IF_CONFIG_PARAMS ifDefaultConfigParams;

STATUS usrNetIfInit (void)
    {
    /* set up the default configuration parameters */

#ifdef INCLUDE_IF
    ifDefaultConfigParams.cfgh.len = sizeof (IF_CONFIG_PARAMS);
    ifDefaultConfigParams.if_index_limit = IF_INDEXLIM_CFG;
    ifDefaultConfigParams.ifq_max_len = IFQ_MAXLEN_CFG;

#ifdef INCLUDE_NET_SYSCTL
    /* Initialize the sysctl node */

    ifDefaultConfigParams.cfg_privInitSysctl = ifSysctlInit;
#else
    ifDefaultConfigParams.cfg_privInitSysctl = NULL;
#endif

#ifdef VIRTUAL_STACK
    return vsComponentRegister (&ifRegistrationNum,
                                         "IF",
                                         VS_SC_ENABLED,
                                         VS_SC_NON_OPTIONAL,
                                         ifInstInit,
                                         sizeof (IF_CONFIG_PARAMS),
                                         &ifDefaultConfigParams,
                                         ifDestructor);
#else
    return ifInstInit (&ifDefaultConfigParams);
#endif /* VIRTUAL_STACK */
#else 	/* INCLUDE_IF */
    return OK;
#endif	/* INCLUDE_IF */ 
    }

#ifdef INCLUDE_IFCONFIG
void usrNetIfconfigInit (void)
    {
    /* Configure up to 10 interfaces with ifconfig */

#ifdef IFCONFIG_ENTRY_1 
    {
    char * ifString = IFCONFIG_ENTRY_1;
    if (ifString)
        ifconfig (IFCONFIG_ENTRY_1);
    }
#endif

#ifdef IFCONFIG_ENTRY_2 
    {
    char * ifString = IFCONFIG_ENTRY_2;
    if (ifString)
        ifconfig (IFCONFIG_ENTRY_2);
    }
#endif

#ifdef IFCONFIG_ENTRY_3 
    {
    char * ifString = IFCONFIG_ENTRY_3;
    if (ifString)
        ifconfig (IFCONFIG_ENTRY_3);
    }
#endif

#ifdef IFCONFIG_ENTRY_4
    {
    char * ifString = IFCONFIG_ENTRY_4;
    if (ifString)
        ifconfig (IFCONFIG_ENTRY_4);
    }
#endif

#ifdef IFCONFIG_ENTRY_5
    {
    char * ifString = IFCONFIG_ENTRY_5;
    if (ifString)
        ifconfig (IFCONFIG_ENTRY_5);
    }
#endif

#ifdef IFCONFIG_ENTRY_6 
    {
    char * ifString = IFCONFIG_ENTRY_6;
    if (ifString)
        ifconfig (IFCONFIG_ENTRY_6);
    }
#endif

#ifdef IFCONFIG_ENTRY_7
    {
    char * ifString = IFCONFIG_ENTRY_7;
    if (ifString)
        ifconfig (IFCONFIG_ENTRY_7);
    }
#endif

#ifdef IFCONFIG_ENTRY_8 
    {
    char * ifString = IFCONFIG_ENTRY_8;
    if (ifString)
        ifconfig (IFCONFIG_ENTRY_8);
    }
#endif

#ifdef IFCONFIG_ENTRY_9
    {
    char * ifString = IFCONFIG_ENTRY_9;
    if (ifString)
        ifconfig (IFCONFIG_ENTRY_9);
    }
#endif

#ifdef IFCONFIG_ENTRY_10
    {
    char * ifString = IFCONFIG_ENTRY_10;
    if (ifString)
        ifconfig (IFCONFIG_ENTRY_10);
    }
#endif

    ifconfigInit ();

    }

#endif /* INCLUDE_IFCONFIG */


#ifdef INCLUDE_TELNET

#ifdef VIRTUAL_STACK

/*  variables to be set to values from cdf params in the configlette */

extern int telnetdmaxclients;
extern BOOL telnetdtaskflag;
extern FUNCPTR telnetdparserhook;
extern int telnetdport;
extern int telnetvsnum;
 
extern VS_REG_ID telnetdRegistrationNum;

extern STATUS telnetdInstInit (void *noparams);
extern STATUS telnetdDestructor (VSNUM vsnum);
#endif /* VIRTUAL_STACK */

#if defined(__STDC__) || defined(__cplusplus)
#if (!defined (_WRS_VXWORKS_MAJOR) || (_WRS_VXWORKS_MAJOR < 6))
IMPORT STATUS TELNETD_PARSER_HOOK (UINT32, UINT32, UINT32);
#else
IMPORT STATUS TELNETD_PARSER_HOOK (UINT32, UINT32, UINT32, VOIDFUNCPTR);
#endif
#else   /* __STDC__ */

IMPORT STATUS TELNETD_PARSER_HOOK ();

#endif  /* __STDC__ */

LOCAL STATUS usrTelnetdStart (void)
    {
#ifdef VIRTUAL_STACK

    /* first set global variables to the CDF param values for later use */

    telnetdmaxclients = TELNETD_MAX_CLIENTS;
    telnetdtaskflag   = TELNETD_TASKFLAG;
    telnetdparserhook = TELNETD_PARSER_HOOK;
    telnetdport       = TELNETD_PORT;
    telnetvsnum       = -1;
    
    /*
     * Register at vsmanager to allow ftp6 server to be started in
     * a chosen virtual stack; it can only be started in one stack.
     */

    if (vsApplicationRegister (&telnetdRegistrationNum, "telnetd",
                               VS_SC_DISABLED, telnetdInstInit,
                               0, NULL, telnetdDestructor) == VS_OK)
        {
#if (TELNET_SERVER_IN_VS0 == TRUE)  /* it must be enabled for instance 0 */
        return vsComponentStatusSet (0, telnetdRegistrationNum, VS_SC_ENABLED,
				     NULL);
#else
	return (OK);
#endif
        }
    else
        return (ERROR);
#else
    STATUS result = ERROR;

    /* Specify command interpreter for telnet server. */

    if (telnetdParserSet (TELNETD_PARSER_HOOK) == ERROR)
        printf ("Error %x: unable to add command interpreter to telnet server.\n", errno);
    else if (telnetdInit (TELNETD_MAX_CLIENTS, TELNETD_TASKFLAG) == ERROR)
        printf ("Error %x: unable to start telnet server.\n", errno);
    else if (telnetdStart (TELNETD_PORT) == ERROR)
        printf ("Error %x: unable to start telnet server.\n", errno);
    else
        result = OK;

    return (result);
#endif /* VIRTUAL_STACK */
    }
#endif /* INCLUDE_TELNET */


#ifdef INCLUDE_TELNET_CLIENT
#ifdef VIRTUAL_STACK

extern VS_REG_ID telnetcRegistrationNum;

extern STATUS telnetcInstInit (void *noparams);
extern STATUS telnetcDestructor (VSNUM vsnum);
#endif /*VIRTUAL_STACK*/

LOCAL STATUS usrTelnetcStart (void)
    {
    
#ifdef VIRTUAL_STACK
    /* now register at vsmanager to allow telnet to be started in a 
       chosen virtual stack; it can only be started in one stack*/ 
       
    if (vsApplicationRegister (&telnetcRegistrationNum,
                               "telnetc",
                               VS_SC_DISABLED,
                               telnetcInstInit,
                               0,
                               NULL,
                               telnetcDestructor) == VS_OK)
        {
#if (TELNET_CLIENT_IN_VS0 == TRUE)   /* it must be enabled for instance 0 */
        return vsComponentStatusSet (0,	telnetcRegistrationNum,	VS_SC_ENABLED,
				     NULL);
#else
	return (OK);
#endif
        }
    else 
        return (ERROR);
#else
    telnetcInit ();
    return (OK);
#endif /* VIRTUAL_STACK */
    }
#endif /*INCLUDE_TELNET_CLIENT*/

#ifdef INCLUDE_SNTPC
#ifdef VIRTUAL_STACK
extern int sntpcvsnum ;
extern VS_REG_ID sntpcRegistrationNum;
extern int sntpcport;
extern char sntpcpdevname [BOOT_DEV_LEN];

extern STATUS sntpcInstInit (void *noparams);
extern STATUS sntpcDestructor (VSNUM vsnum);
#endif /* VIRTUAL_STACK */

LOCAL STATUS usrSntpcInit (void)
    {
    char	devName [MAX_FILENAME_LENGTH];	/* device name */

    devName [0] = EOS;
    sprintf(devName, "%s%d", pDevName, sysBootParams.unitNum);

#ifdef VIRTUAL_STACK
    sntpcvsnum = -1;
    sntpcport = SNTP_PORT;
    strcpy (sntpcpdevname, devName);

    /* now register at vsmanager to allow sntp to be started in a 
       chosen virtual stack; it can only be started in one stack*/ 
       

    if (vsApplicationRegister (&sntpcRegistrationNum,
                               "sntpc",
                               VS_SC_DISABLED,
                               sntpcInstInit,
                               0,
                               NULL,
                               sntpcDestructor) == VS_OK)
        {
#if (SNTP_CLIENT_IN_VS0 == TRUE)   /* it must be enabled for instance 0 */
        return vsComponentStatusSet (0, sntpcRegistrationNum, VS_SC_ENABLED,
				     NULL);
#else
        return (OK);
#endif
        }
    else 
        return (ERROR);
#else
    /* Call the init rtn for SNTP client. */
    if (sntpcInit (SNTP_PORT, devName) == ERROR)
        {
        printf ("Unable to initialize SNTP client; errno = 0x%x\n", errno);
        return ERROR;
        }
    return OK;
#endif /* VIRTUAL_STACK */
    }
#endif /* INCLUDE_SNTPC */

#ifdef INCLUDE_SNTPS
#ifdef VIRTUAL_STACK
extern int sntpsvsnum;
extern VS_REG_ID sntpsRegistrationNum;
extern char sntpspdevname [BOOT_DEV_LEN];
extern u_char sntpsmode;
extern char * sntpspdstaddr;
extern short sntpsinterval;
extern u_short sntpsport;
extern FUNCPTR sntpsptimehookrtn;

extern STATUS sntpsInstInit (void *noparams);
extern STATUS sntpsDestructor (VSNUM vsnum);
#endif /* VIRTUAL_STACK */

LOCAL STATUS usrSntpsInit ()
    {
    char	devName [MAX_FILENAME_LENGTH];	/* device name */

    devName [0] = EOS;
    sprintf(devName, "%s%d", pDevName, sysBootParams.unitNum);

#ifdef VIRTUAL_STACK
    sntpsvsnum = -1;
    strcpy (sntpspdevname, devName);
    sntpsmode = SNTPS_MODE;
    sntpspdstaddr = SNTPS_DSTADDR;
    sntpsinterval = SNTPS_INTERVAL;
    sntpsport = SNTP_PORT;
    sntpsptimehookrtn = SNTPS_TIME_HOOK;

    /* now register at vsmanager to allow sntp to be started in a 
       chosen virtual stack; it can only be started in one stack*/ 
       

    if (vsApplicationRegister (&sntpsRegistrationNum,
                               "sntps",
                               VS_SC_DISABLED,
                               sntpsInstInit,
                               0,
                               NULL,
                               sntpsDestructor) == VS_OK)
        {
#if (SNTP_SERVER_IN_VS0 == TRUE)   /* it must be enabled for instance 0 */
        return vsComponentStatusSet (0, sntpsRegistrationNum, VS_SC_ENABLED,
				     NULL);
#else
        return (OK);
#endif
        }
    else
        return (ERROR);
#else
    /*
     * For now, set server to listen on primary interface.
     * Eventually, need to allow for multiple network devices.
     */

    if (sntpsInit (devName, SNTPS_MODE, SNTPS_DSTADDR, SNTPS_INTERVAL, 
		   SNTP_PORT, SNTPS_TIME_HOOK) == ERROR)
        {
        printf ("Unable to start SNTP server; errno = 0x%x\n", errno);
        return (ERROR);
        }
    return (OK);
#endif /* VIRTUAL_STACK */
    }
#endif /* INCLUDE_SNTPS */


#ifdef INCLUDE_RIP

extern RIP_CONFIG_PARAMS ripDefaultConfigParams;
extern STATUS ripInstInit (void *InitValues);

#ifdef VIRTUAL_STACK
extern STATUS ripDestructor (VSNUM vsnum);
extern VS_REG_ID ripRegistrationNum;
extern void ripTblInit(void);
#endif /* VIRTUAL_STACK */

LOCAL STATUS usrNetRipInit
    (
    BOOL supplier,	    /* operate in silent mode? */
    BOOL gateway,	    /* act as gateway to the Internet? */
    BOOL multicast,	    /* use multicast or broadcast addresses? */
    int version,            /* 1 or 2: selects format of outgoing messages */
    int timerRate,          /* update frequency for internal routing table */
    int supplyInterval,     /* update frequency for neighboring routers */
    int expire,             /* maximum interval for renewing learned routes */
    int garbage,            /* elapsed time before deleting stale route */
    int authType            /* default authentication type to use */
    )
    {
#ifdef VIRTUAL_STACK    
    ripTblInit();
#endif    
    
    bzero ((char *) &ripDefaultConfigParams, sizeof (ripDefaultConfigParams));
    ripDefaultConfigParams.cfgh.len = sizeof(ripDefaultConfigParams);
    ripDefaultConfigParams.cfg_supplier = supplier;
    ripDefaultConfigParams.cfg_gateway = gateway;
    ripDefaultConfigParams.cfg_multicast = multicast;
    ripDefaultConfigParams.cfg_version = version;
    ripDefaultConfigParams.cfg_timerRate = timerRate;
    ripDefaultConfigParams.cfg_supplyInterval = supplyInterval;
    ripDefaultConfigParams.cfg_expire = expire;
    ripDefaultConfigParams.cfg_garbage = garbage;
    ripDefaultConfigParams.cfg_authType = authType;
    

#ifdef VIRTUAL_STACK
    if (vsApplicationRegister (&ripRegistrationNum, "RIP",
                               VS_SC_DISABLED, ripInstInit,
                               sizeof (ripDefaultConfigParams),
                               &ripDefaultConfigParams,
                               ripDestructor) == VS_OK)
        {
#if (RIP_IN_VS0 == TRUE)  /* it must be enabled for instance 0 */
        return vsComponentStatusSet (0, ripRegistrationNum, VS_SC_ENABLED,
                                     &ripDefaultConfigParams);
#else
        return (OK);
#endif
        }
    else 
        return (ERROR);
#else
    if (ripInstInit (&ripDefaultConfigParams) == ERROR)
        return (ERROR);
    return (OK);
#endif
    }

#endif /* INCLUDE_RIP */

#ifdef INCLUDE_RIPNG

extern RIPNG_CFG_PARAMS ripngDefaultConfigParams;
extern STATUS ripngInstInit (void *InitValues);

#ifdef VIRTUAL_STACK
extern VS_REG_ID ripngRegistrationNum;
extern STATUS ripngInstDestroy (VSNUM vsnum);
extern STATUS ripngLibInit(void);
#endif /* VIRTUAL_STACK */

LOCAL STATUS usrripngInit (void)
    {
#ifdef VIRTUAL_STACK        
    ripngLibInit();
#endif    
	
    bzero ((char *) &ripngDefaultConfigParams, sizeof (ripngDefaultConfigParams));
    ripngDefaultConfigParams.cfgh.len = sizeof(ripngDefaultConfigParams);
    ripngDefaultConfigParams.cfg_ripng_priority = RIPNG_PRIORITY;
    ripngDefaultConfigParams.cfg_ripng_options  = RIPNG_OPTIONS_STRING;
    
#ifdef VIRTUAL_STACK
    if (vsApplicationRegister (&ripngRegistrationNum, "RIPNG",
                               VS_SC_DISABLED, ripngInstInit,
                               sizeof (ripngDefaultConfigParams),
                               &ripngDefaultConfigParams,
                               ripngInstDestroy) == VS_OK)
        {
#if (RIPNG_IN_VS0 == TRUE)  /* it must be enabled for instance 0 */
        return vsComponentStatusSet (0, ripngRegistrationNum, VS_SC_ENABLED,
				     &ripngDefaultConfigParams);
#else
	return (OK);
#endif
        }
    else 
        return (ERROR);
#else
    if (ripngInstInit (&ripngDefaultConfigParams) == ERROR)
        return (ERROR);
    return (OK);
#endif
    }
#endif /* INCLUDE_RIPNG */


#ifdef INCLUDE_RTSOL
IMPORT RTSOL_CONFIG_PARAMS rtsolDefaultConfigParams;

IMPORT STATUS rtsolInstInit (void *values);

#ifdef VIRTUAL_STACK
IMPORT VS_REG_ID rtsolRegistrationNum;

IMPORT STATUS rtsolDestructor (VSNUM);
#endif

STATUS usrNetRtsolInit (void)
    {
    bzero((char *)&rtsolDefaultConfigParams, sizeof (RTSOL_CONFIG_PARAMS));
    rtsolDefaultConfigParams.cfgh.len = sizeof (RTSOL_CONFIG_PARAMS);

    if ((rtsolDefaultConfigParams.cfg_pParameterStr = 
                malloc (strlen(RTSOL_COMMAND) + 1)) == NULL)
	 return (ERROR);

    sprintf(rtsolDefaultConfigParams.cfg_pParameterStr, RTSOL_COMMAND);    

#ifdef VIRTUAL_STACK
    if (vsApplicationRegister (&rtsolRegistrationNum,
                          "RTSOL",
                          VS_SC_DISABLED,
                          rtsolInstInit,
                          sizeof (RTSOL_CONFIG_PARAMS),
                          &rtsolDefaultConfigParams,
                          rtsolDestructor)==VS_OK)
        {
#if (RTSOL_IN_VS0 == TRUE)   /* it must be enabled for instance 0 */
	return vsComponentStatusSet (0, rtsolRegistrationNum, VS_SC_ENABLED,
				     &rtsolDefaultConfigParams);
#else
	return (OK);
#endif
	}
    else 
        return (ERROR);
#else
    rtsolInstInit (&rtsolDefaultConfigParams);
    return (OK);
#endif /* VIRTUAL_STACK */
    }
#endif /* INCLUDE_RTSOL */


#ifdef INCLUDE_RTADV

IMPORT RTADV_CONFIG_PARAMS rtadvDefaultConfigParams;

#ifdef VIRTUAL_STACK
IMPORT VS_REG_ID rtadvRegistrationNum;
#endif

IMPORT STATUS rtadvInstInit (void *values);

#ifdef VIRTUAL_STACK
IMPORT STATUS rtadvDestructor (VSNUM);
#endif

STATUS usrNetRtadvInit (void)
    {
    bzero ( (char *)&rtadvDefaultConfigParams, sizeof (RTADV_CONFIG_PARAMS));
    rtadvDefaultConfigParams.cfgh.len = sizeof (RTADV_CONFIG_PARAMS);

    if ((rtadvDefaultConfigParams.cfg_pParameterStr = 
                malloc (sizeof (RTADV_COMMAND))) == NULL)
	 return (ERROR);

    sprintf (rtadvDefaultConfigParams.cfg_pParameterStr, RTADV_COMMAND);    

#ifdef VIRTUAL_STACK
    if (vsApplicationRegister (&rtadvRegistrationNum, "RTADV",
                               VS_SC_DISABLED, rtadvInstInit,
                               sizeof (RTADV_CONFIG_PARAMS),
                               &rtadvDefaultConfigParams,
                               rtadvDestructor) == VS_OK)
        {
#if (RTADV_IN_VS0 == TRUE)  /* it must be enabled for instance 0 */
        return vsComponentStatusSet (0, rtadvRegistrationNum, VS_SC_ENABLED, 
                                     &rtadvDefaultConfigParams);
#else
	return (OK);
#endif
        }
    else
        return (ERROR);
#else
    rtadvInstInit (&rtadvDefaultConfigParams);
    return (OK);
#endif
    }
#endif /* INCLUDE_RTADV */

#ifdef INCLUDE_DHCPS

STATUS usrDhcpsInit (void)
    {
    /* set up the default configuration parameters */

    bzero ( (char *)&dhcpsDefaultConfigParams, sizeof (DHCPS_CFG_PARAMS));
    dhcpsDefaultConfigParams.cfgh.len = sizeof (DHCPS_CFG_PARAMS);
    dhcpsDefaultConfigParams.dhcpMaxHops = DHCP_MAX_HOPS;			
    dhcpsDefaultConfigParams.dhcpSPort = DHCPS_SPORT;			
    dhcpsDefaultConfigParams.dhcpCPort = DHCPS_CPORT;			
    dhcpsDefaultConfigParams.dhcpMaxMsgSize = DHCPS_MAX_MSGSIZE;			
    dhcpsDefaultConfigParams.dhcpsDfltLease = DHCPS_DEFAULT_LEASE;		
    dhcpsDefaultConfigParams.dhcpsMaxLease = DHCPS_MAX_LEASE;			
    dhcpsDefaultConfigParams.pDhcpsLeaseFunc = DHCPS_LEASE_HOOK;		
    dhcpsDefaultConfigParams.pDhcpsAddrFunc = DHCPS_ADDRESS_HOOK;		
    dhcpsDefaultConfigParams.pDhcpsIfTbl = dhcpsIfTbl;		
    dhcpsDefaultConfigParams.numDev = NELEMENTS(dhcpsIfTbl);				
    dhcpsDefaultConfigParams.pDhcpsLeaseTbl = dhcpsLeaseTbl;	
    dhcpsDefaultConfigParams.dhcpsLeaseTblSize = NELEMENTS(dhcpsLeaseTbl);		
    dhcpsDefaultConfigParams.pDhcpsTargetTbl = dhcpTargetTbl;	
    dhcpsDefaultConfigParams.dhcpTargetTblSize = NELEMENTS(dhcpTargetTbl);		
    dhcpsDefaultConfigParams.pDhcpsRelayTbl = dhcpsRelayTbl;	
    dhcpsDefaultConfigParams.dhcpsRelayTblSize = NELEMENTS(dhcpsRelayTbl);
        
#ifdef VIRTUAL_STACK

    if (vsApplicationRegister (&dhcpsRegistrationNum, "DHCPS",
                               VS_SC_DISABLED, dhcpsInstInit,
                               sizeof (DHCPS_CFG_PARAMS),
                               &dhcpsDefaultConfigParams,
                               dhcpsDestructor) == VS_OK)
        {
#if (DHCPS_IN_VS0 == TRUE)  /* it must be enabled for instance 0 */
        if (vsComponentStatusSet (0, dhcpsRegistrationNum, VS_SC_ENABLED,
                                  &dhcpsDefaultConfigParams) != VS_OK)
            return (ERROR);
#endif
        }
    else
        return (ERROR);		
#else
    if (dhcpsInstInit (&dhcpsDefaultConfigParams) == ERROR)
        return (ERROR);
#endif /* VIRTUAL_STACK */

    return (OK);
    }
#endif /* INCLUDE_DHCPS */

#ifdef INCLUDE_NDP
extern void ndpInit(void);

void usrNetNdpInit (void)
    {
    /* Configure up to 5 ND entries */

    ndpInit();

#ifdef NDP_ENTRY_1
    {
    char * ndpString = NDP_ENTRY_1;
    if (ndpString)
        ndp (NDP_ENTRY_1);
    }
#endif

#ifdef NDP_ENTRY_2
    {
    char * ndpString = NDP_ENTRY_2;
    if (ndpString)
        ndp (NDP_ENTRY_2);
    }
#endif

#ifdef NDP_ENTRY_3
    {
    char * ndpString = NDP_ENTRY_3;
    if (ndpString)
        ndp (NDP_ENTRY_3);
    }
#endif

#ifdef NDP_ENTRY_4
    {
    char * ndpString = NDP_ENTRY_4;
    if (ndpString)
        ndp (NDP_ENTRY_4);
    }
#endif

#ifdef NDP_ENTRY_5
    {
    char * ndpString = NDP_ENTRY_5;
    if (ndpString)
        ndp (NDP_ENTRY_5);
    }
#endif
    }

#endif /* INCLUDE_NDP */

#ifdef INCLUDE_ADDRCTL_POLICY
void usrNetAddrCtlInit (void)
    {

    /* Configure up to 3 address control policy entries */

    ip6addrctlInit ();

#ifdef ADDRCTL_POLICY_ENTRY_1
    {
    char * addrctlString = ADDRCTL_POLICY_ENTRY_1;
    if ((addrctlString != NULL))
        ip6addrctl (ADDRCTL_POLICY_ENTRY_1);
    }
#endif

#ifdef ADDRCTL_POLICY_ENTRY_2
    {
    char * addrctlString = ADDRCTL_POLICY_ENTRY_2;
    if ((addrctlString != NULL))
        ip6addrctl (ADDRCTL_POLICY_ENTRY_2);
    }
#endif

#ifdef ADDRCTL_POLICY_ENTRY_3
    {
    char * addrctlString = ADDRCTL_POLICY_ENTRY_3;
    if ((addrctlString != NULL))
        ip6addrctl (ADDRCTL_POLICY_ENTRY_3);
    }
#endif
    }
#endif /* INCLUDE_ADDRCTL_POLICY */

#ifdef INCLUDE_ROUTE 

IMPORT int route_init (void);

#ifdef VIRTUAL_STACK
extern VS_REG_ID routeRegistrationNum;
STATUS usrVsWrapperRoute_init (void *cfgParams);
#endif

void usrNetRoute (void)
    {
#ifdef VIRTUAL_STACK
    vsComponentRegister (&routeRegistrationNum,
                         "Route",
                         VS_SC_ENABLED,
                         VS_SC_NON_OPTIONAL,
                         usrVsWrapperRoute_init,
                         0,
                         NULL,
                         NULL);
#else
    route_init ();
#endif /* VIRTUAL_STACK */
    }

#ifdef VIRTUAL_STACK
STATUS usrVsWrapperRoute_init (void *cfgParams)
    {
    route_init();
    return (OK);
    }
#endif /* VIRTUAL_STACK */

#endif /* INCLUDE_ROUTE */

#ifdef INCLUDE_ROUTECMD

IMPORT STATUS routeCmdInit (void);

void usrNetRouteCmdInit (void)
    {

    routeCmdInit();

    /* Add 5 static routing entries */

#ifdef ROUTE_ENTRY_1 
    {
    char * routeString = ROUTE_ENTRY_1;
    if (routeString)
        routec (ROUTE_ENTRY_1);
    }
#endif

#ifdef ROUTE_ENTRY_2
    {
    char * routeString = ROUTE_ENTRY_2;
    if (routeString)
        routec (ROUTE_ENTRY_2);
    }
#endif

#ifdef ROUTE_ENTRY_3
    {
    char * routeString = ROUTE_ENTRY_3; 
    if (routeString)
        routec (ROUTE_ENTRY_3);
    }
#endif

#ifdef ROUTE_ENTRY_4
    {
    char * routeString = ROUTE_ENTRY_4; 
    if (routeString)
        routec (ROUTE_ENTRY_4);
    }
#endif

#ifdef ROUTE_ENTRY_5
    {
    char * routeString = ROUTE_ENTRY_5; 
    if (routeString)
        routec (ROUTE_ENTRY_5);
    }
#endif

    }

#endif /* INCLUDE_ROUTECMD */


#ifdef INCLUDE_DHCPC_LEASE_CLEAN

/******************************************************************************
*
* usrDhcpcLeaseClean - remove DHCP timestamp information from address
*
* This routine deletes any values for the DHCP lease origin and lease duration
* in the selected address string. It is the initialization routine for the 
* INCLUDE_DHCPC_LEASE_CLEAN component which is automatically added when the 
* INCLUDE_NET_INIT component is used, unless the INCLUDE_DHCPC component 
* is also included. If DHCP timestamp values are found, processing can't 
* continue because the corresponding active DHCP lease cannot be maintained.
*
* RETURNS: N/A
*
* NOMANUAL
*/

void usrDhcpcLeaseClean (void)
    {
    /* Check address for DHCP lease information. */ 

    if (sysBootParams.flags & SYSFLG_AUTOCONFIG)
        if (dhcpcBootLineClean (pAddrString) == OK)
            {
            printf ("Can't use dynamic address %s without DHCP.\n",
                    pAddrString);
            netDevBootFlag = TRUE;    /* Prevents further processing. */
            }

    return;
    }

/******************************************************************************
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

    offset = (char *)index (pAddrString, ':');  /* Find netmask field. */
    if (offset != NULL)
        {
        pDelim = offset + 1;
        offset = (char *)index (pDelim, ':');
        if (offset != NULL)
            {
            /* 
             * Lease duration found - for active DHCP leases,
             * the lease origin field is also present.
             */

            pDelim = offset + 1;
            pDelim = (char *)index (pDelim, ':');
            if (pDelim != NULL)
                 result = OK;     /* Active DHCP lease found. */

            *offset = EOS;    /* Remove DHCP lease information. */
            }
        }
    return (result);
    }

#endif /* INCLUDE_DHCPC_LEASE_CLEAN */


#ifdef INCLUDE_DHCPC_LEASE_TEST

LOCAL BOOL dhcpBootFlag = FALSE;  /* Set TRUE if a lease timestamp is found. */

/*******************************************************************************
*
* usrDhcpcLeaseGet - extract DHCP timestamp information from address
*
* This routine stores any values for the DHCP lease origin and lease duration
* in the dhcpcBootLease variable and sets the dhcpBoot flag to indicate
* if those values were found. It is the initialization routine for the 
* INCLUDE_DHCPC_LEASE_GET component which is automatically added when both the 
* INCLUDE_DHCPC and INCLUDE_NET_INIT components are used.
*
* RETURNS: N/A
*
* NOMANUAL
*/

void usrDhcpcLeaseGet (void)
    {
    /* 
     * Remove the timestamp from the address string if automatic configuration
     * (using the DHCP client) is active.
     */

    if (sysBootParams.flags & SYSFLG_AUTOCONFIG)
        if (dhcpcLeaseGet (pAddrString, &dhcpBootFlag) == ERROR)
            {
            printf ("Unable to verify assigned DHCP address.\n");
            netDevBootFlag = TRUE;    /* Prevents further processing. */
            }
    return;
    }

/*******************************************************************************
*
* usrDhcpcLeaseTest - renew DHCP lease stored in boot parameters
*
* This routine attempts to renew any existing DHCP lease created at boot
* time. It is the initialization routine for the INCLUDE_DHCPC_LEASE_TEST 
* component which is automatically added when both the INCLUDE_DHCPC and 
* INCLUDE_NET_INIT components are used. If no lease is available and no
* target address is specified, the routine will attempt to establish a
* new lease.
*
* RETURNS: N/A
*
* NOMANUAL
*/

void usrDhcpcLeaseTest (void)
    {
    /*
     * Renew or obtain a lease as needed. If successful, the pDhcpcBootCookie
     * global is set to allow user access to the lease and the timing
     * information is stored in the dhcpcBootLease global for later storage
     * in the boot line. The new or verified target IP address is stored in
     * the boot parameters immediately.
     */

    if (dhcpcConfigSet (&sysBootParams, pAddrString, &netmask,
                        &dhcpBootFlag, netAttachFlag) == ERROR)
        netDevBootFlag = TRUE;

    return;
    }

/*******************************************************************************
*
* usrDhcpcLeaseSave - preserve new DHCP lease in boot parameters
*
* This routine stores the timestamp information for a new DHCP lease
* established during the runtime startup. It is the initialization
* routine for the INCLUDE_DHCPC_LEASE_SAVE component which is
* automatically added when both the INCLUDE_DHCPC and INCLUDE_NET_INIT
* components are used.
*
* RETURNS: N/A
*
* NOMANUAL
*/

void usrDhcpcLeaseSave (void)
    {
    char numString [30];    /* buffer for numeric string with timestamps */

    /*
     * If the target address is not manually assigned, add timing
     * information so later reboots will detect the DHCP lease.
     */

    if (dhcpBootFlag == TRUE)
        {
        if (netmask == 0)
            sprintf (numString, "::%lx:%lx", dhcpcBootLease.lease_duration,
                                             dhcpcBootLease.lease_origin);
        else
            {
            /*
             * dhcpcConfigSet() called from usrDhcpcLeaseGet()
             * returns netmask in host order.
             */
            sprintf (numString, ":%x:%lx:%lx", netmask,
                                               dhcpcBootLease.lease_duration,
                                               dhcpcBootLease.lease_origin);
            }

#ifdef INCLUDE_SM_NET
        if (backplaneBoot)
            strcat (sysBootParams.bad, numString);
        else
#endif /* INCLUDE_SM_NET */
            strcat (sysBootParams.ead, numString);

        bootStructToString (BOOT_LINE_ADRS, &sysBootParams);
        }

    return;
    }


#endif /* INCLUDE_DHCPC_LEASE_TEST */

#ifdef INCLUDE_BPF 	
extern unsigned int bpfBufSize;

STATUS usrNetBpfDrvInit (void)
    {
    bpfBufSize = BPF_BUFSIZE_CFG;
    return (bpfDrv());
    }

#endif /* INCLUDE_BPF */


#ifdef  INCLUDE_DHCPR

STATUS usrDhcprInit (void)
    {
    /* set up the default configuration parameters */

    bzero ((char *)&dhcprDefaultConfigParams, sizeof (DHCPR_CONFIG_PARAMS));
    dhcprDefaultConfigParams.cfgh.len = sizeof (DHCPR_CONFIG_PARAMS);
    dhcprDefaultConfigParams.cfg_dhcpTargetTbl = dhcpTargetTbl;
    dhcprDefaultConfigParams.cfg_dhcpTargetTblSize = (NELEMENTS(dhcpTargetTbl));
    dhcprDefaultConfigParams.cfg_dhcprIfTbl = dhcprIfTbl;		
    dhcprDefaultConfigParams.cfg_numDev = NELEMENTS(dhcprIfTbl);
    dhcprDefaultConfigParams.cfg_dhcpCPort = DHCPS_CPORT;    
    dhcprDefaultConfigParams.cfg_dhcpSPort = DHCPS_SPORT;
    dhcprDefaultConfigParams.cfg_dhcpMaxHops = DHCP_MAX_HOPS;
    dhcprDefaultConfigParams.cfg_dhcpsMaxMsgSize = DHCPS_MAX_MSGSIZE;
    
#ifdef VIRTUAL_STACK

    if (vsApplicationRegister (&dhcprRegistrationNum, "DHCPR",
                               VS_SC_DISABLED, dhcprInstInit,
                               sizeof (DHCPR_CONFIG_PARAMS),
                               &dhcprDefaultConfigParams,
                               dhcprDestructor) == VS_OK)
        {
#if (DHCPR_IN_VS0 == TRUE)  /* it must be enabled for instance 0 */
        if (vsComponentStatusSet (0, dhcprRegistrationNum, VS_SC_ENABLED, 
                                  &dhcprDefaultConfigParams) != VS_OK)
            return (ERROR);
#endif
        }
    else 
        return (ERROR);
#else
    if (dhcprInstInit (&dhcprDefaultConfigParams) == ERROR)
        return (ERROR);
#endif /* VIRTUAL_STACK */

    return (OK);
    }
#endif /* INCLUDE_DHCPR */

#ifdef INCLUDE_DHCPC

void usrDhcpcStart (void)
    {
#ifdef INCLUDE_SM_NET
    if (backplaneBoot)
        {
        if ((sysBootParams.flags & SYSFLG_AUTOCONFIG) &&
                 !(sysBootParams.flags & SYSFLG_PROXY))
            {
            printf ("Warning: DHCP over backplane might require proxy arp.\n");
            }
        }
#endif /* INCLUDE_SM_NET */

    if (dhcpcLibInit (DHCPC_SPORT, DHCPC_CPORT, DHCPC_MAX_LEASES,
                      DHCPC_MAX_MSGSIZE, DHCPC_OFFER_TIMEOUT,
                      DHCPC_DEFAULT_LEASE, DHCPC_MIN_LEASE) == ERROR)
        printf ("Error in initializing dhcpcLib\n");

#if (DHCPC_DISCOVER_RETRIES)
    dhcpcDiscoverRetries = DHCPC_DISCOVER_RETRIES;
#endif
    }

#endif /* INCLUDE_DHCPC */

#ifdef INCLUDE_TFTP_SERVER

LOCAL STATUS usrTftpdInit (void)
    {
    STATUS result;

#if (TFTP_SERVER_NOCONTROL == FALSE)
    char * pDirlist;
    char * pDirName;
#endif

    result = tftpdInit (TFTP_SERVER_STACKSIZE,
                        0, NULL, TFTP_SERVER_NOCONTROL,
                        TFTP_SERVER_MAXCONNECTIONS);
    if (result == ERROR)
        {
        printf ("Error %x: unable to start tftp server.\n", errnoGet());
        return (ERROR);
        }

#if (TFTP_SERVER_NOCONTROL == FALSE)

    /*
     * Add any permitted directories if access control is enabled.
     * Otherwise, the list remains empty, allowing access to any
     * directory.
     */

    if (TFTP_SERVER_DIRECTORYNAMES == NULL)
        return (OK);

    /* Copy the provided list to avoid changing it. */

    pDirlist = malloc (strlen (TFTP_SERVER_DIRECTORYNAMES) + 1);
    if (pDirlist == NULL)
        return (ERROR);

    strcpy (pDirlist, TFTP_SERVER_DIRECTORYNAMES);

    /* Add individual names to the list of permitted directories. */

    pDirName = strtok (pDirlist, ",");
    if (pDirName != NULL)
        {
        while (pDirName != NULL)
            {
            tftpdDirectoryAdd (pDirName);
            pDirName = strtok (NULL, ",");
            }
        }

    free (pDirlist);
#endif /* TFTP_SERVER_NO_CONTROL */

    return (OK);
    }
#endif /* INCLUDE_TFTP_SERVER */

#ifdef INCLUDE_SECURITY
void usrSecurity (void)
    {
    /* initialize login table */
    loginInit ();                          

    /* add additional users here as required */

#if defined (LOGIN_USER_NAME) && defined (LOGIN_PASSWORD)
    loginUserAdd (LOGIN_USER_NAME, LOGIN_PASSWORD);
#endif
    }
#endif /* INCLUDE_SECURITY */

#ifdef INCLUDE_FTP

extern UINT32  ftplTransientMaxRetryCount;
extern UINT32  ftplTransientRetryInterval;

LOCAL BOOL ftpTransientFatal
    (
    UINT32 reply /* Three digit code defined in RFC #959 */
    )
    {
    switch (reply)
        {
        case (421): /* Service not available */
        case (450): /* File unavailable */
        case (451): /* error in processing */
        case (452): /* insufficient storage */
            {
            /* yes, these are actually non-recoverable replies */
            return (TRUE);
            break;
            }
            /* attempt to retry the last command */
        default:
        return (FALSE);
        }
    }

STATUS usrFtpInit (void)
    {
#ifdef VIRTUAL_STACK

    /* Return error if component is already started in any virtual stack. */

    if (ftpvsnum != -1)
        {
        errnoSet (EALREADY);
        return (ERROR);
        }

    /* Save stack number to prevent startup in other virtual stacks. */

    ftpvsnum = myStackNum;

#endif /* VIRTUAL_STACK */

    ftpLibInit (FTP_TIMEOUT);
    ftplTransientMaxRetryCount = FTP_TRANSIENT_MAX_RETRY_COUNT;
    ftplTransientRetryInterval = FTP_TRANSIENT_RETRY_INTERVAL;
    ftpTransientFatalInstall ( FTP_TRANSIENT_FATAL );
    ftpLibDebugOptionsSet ( FTP_DEBUG_OPTIONS );
    return OK;
    }
#endif /* INCLUDE_FTP */

#ifdef INCLUDE_RPC
#ifdef VIRTUAL_STACK
VS_REG_ID  rpcRegistrationNum;

extern STATUS rpcInstInit (void *);
extern STATUS rpcDestructor (VSNUM vsnum);
#endif /* VIRTUAL_STACK */

STATUS usrNetRpcInit ()
    {
#ifdef VIRTUAL_STACK
    if (vsNonCoreComponentRegister (&rpcRegistrationNum, "RPC",
				    VS_SC_DISABLED, rpcInstInit,
			            0, NULL, rpcDestructor) == VS_OK)
        {
#if (RPC_IN_VS0 == TRUE)
        return (vsComponentStatusSet (0, rpcRegistrationNum, VS_SC_ENABLED,
				      NULL));
#else
        return (OK);
#endif
	}
    else
	return (ERROR);
#else
    return (rpcInit ());
#endif /* VIRTUAL_STACK */
    }
#endif /* INCLUDE_RPC */


#if (defined (INCLUDE_NFS_SERVER_ALL) || defined (INCLUDE_NFS2_SERVER) \
    || defined (INCLUDE_NFS3_SERVER))

#ifdef VIRTUAL_STACK
VS_REG_ID        nfsdRegistrationNum;
NFSD_CFG_PARAMS  nfsdDefaultConfigParams;

extern STATUS nfsdInstInit (void * initValues);
extern STATUS nfsdDestructor (VSNUM vsnum);
#endif

STATUS usrNfsdStart ()
   {
    int version = 0;

    if (nfsServMaxPath < 1)
        {
        printf ("Error initializing NFS server, invalid NFS_MAXPATH\n");
        return (ERROR);
        }

    if (nfsServMaxPath > (PATH_MAX +1))
        nfsServMaxPath = (PATH_MAX +1);

    if ((nfsMaxFileName < 1) || (nfsMaxFileName > NAME_MAX))
        {
        printf ("Error initializing NFS server, invalid NFS_MAXFILENAME\n");
        return (ERROR);
        }


#if defined (INCLUDE_NFS_SERVER_ALL)
    nfsv2CallBack = (NFSDCALLBACK) nfs2dRequestEnqueue;
    mountv3CallBack = mount3dRequestProcess;
    nfsv3CallBack = (NFSDCALLBACK) nfs3dRequestEnqueue;
    mountv1CallBack = mount1dRequestProcess;
    pnfs2ServerStat = &nfs2ServerStatus;
    pnfs3ServerStat = &nfs3ServerStatus;
    pnfsdRebootTime = &nfsdRebootTime;
    pNlmdInit = nlmdInit;
#ifdef VIRTUAL_STACK
    pNlmdStop = nlmdStop;
#endif

    /* Set up call statistics */
    memset (&nfs3ServerStatus, 0, sizeof (nfs3ServerStatus));
    /* Set up call statistics */
    memset (&nfs2ServerStatus, 0, sizeof (nfs2ServerStatus));
#else
#if defined (INCLUDE_NFS3_SERVER)
       mountv1CallBack = mountdv1RequestProcess;
       mountv3CallBack = mount3dRequestProcess;
       nfsv3CallBack = (NFSDCALLBACK) nfs3dRequestEnqueue;
       pnfs3ServerStat = &nfs3ServerStatus;
       pnfsdRebootTime = &nfsdRebootTime;
       pNlmdInit = nlmdInit;
#ifdef VIRTUAL_STACK
       pNlmdStop = nlmdStop;
#endif

       /* Set up call statistics */
       memset (&nfs3ServerStatus, 0, sizeof (nfs3ServerStatus));
#endif
#if defined (INCLUDE_NFS2_SERVER)
    nfsv2CallBack = (NFSDCALLBACK) nfs2dRequestEnqueue;
    mountv1CallBack = mount1dRequestProcess;
    /* Set up call statistics */
    memset (&nfs2ServerStatus, 0, sizeof (nfs2ServerStatus));
    pnfs2ServerStat = &nfs2ServerStatus;
#endif   /* else NFS2_SERVER */
#endif   /* else NFS_SERVER_ALL */



#ifdef INCLUDE_NFS_SERVER_ALL
    version = 0;
#else 
#ifdef INCLUDE_NFS2_SERVER
    version |= 0x01;
#endif
#ifdef INCLUDE_NFS3_SERVER
    version |= 0x02;
    if ((version == 0x03))
        version = 0;
#endif  /* NFS3 */
#endif  /* NFS_SERVER_ALL */

#ifdef VIRTUAL_STACK
   bzero ((char *) &nfsdDefaultConfigParams,
	  sizeof (nfsdDefaultConfigParams));
   nfsdDefaultConfigParams.cfgh.len    = sizeof (nfsdDefaultConfigParams);
   nfsdDefaultConfigParams.cfg_options = version;

   /*
    * Register with the VS manager to allow the NFS server to be
    * started in a virtual stack. It can only be started in one stack.
    */

   if (vsApplicationRegister (&nfsdRegistrationNum, "nfsd",
			      VS_SC_DISABLED, nfsdInstInit,
			      sizeof (nfsdDefaultConfigParams),
			      &nfsdDefaultConfigParams,
			      nfsdDestructor) == VS_OK)
       {
#if (NFS_SERVER_IN_VS0 == TRUE) /* it must be enabled in stack 0 */
       return (vsComponentStatusSet (0, nfsdRegistrationNum, VS_SC_ENABLED,
				     &nfsdDefaultConfigParams));
#else
       return (OK);
#endif
       }
   else
       return (ERROR);
#else
   if (nfsdInit (0, 0, 0, 0, 0, version) == ERROR)
       {
       printf ("NFS server initialization failed!\n");
       return (ERROR);
       }
   return (OK);
#endif /* VIRTUAL_STACK */
   }
#endif /* INCLUDE_NFS_SERVER_ALL...... */


/* Initialize NFS client */

#if (defined (INCLUDE_NFS2_CLIENT) || defined (INCLUDE_NFS3_CLIENT) \
    || defined (INCLUDE_NFS_CLIENT_ALL))

STATUS usrNfsInit (int version)
    {
    char *devName;

    if (nfsMaxPath < 1)
        {
        printf ("Error initializing NFS, invalid NFS_MAXPATH\n");
        return (ERROR);
        }

    if ((devName = (char *) alloca (nfsMaxPath)) == NULL)
       {
       printf ("Error initializing NFS, out of memory\n");
       return (ERROR);
       }
#if (CPU==SIMHPPA)
    nfsMaxMsgLen   = 512;
#endif

    nfsAuthUnixSet (sysBootParams.hostName,
        NFS_USER_ID,
        NFS_GROUP_ID,
        0,
        (int *) 0);
    if(version == NFS_V2)
        {
        if (nfs2Drv () == ERROR)     /* initialize nfs driver */
            printf ("Error initializing NFS, errno = %#x\n", errno);
        }
    if(version == NFS_V3)
        {
        if (nfs3Drv () == ERROR)     /* initialize nfs driver */
            printf ("Error initializing NFS, errno = %#x\n", errno);
        }

#if defined (_WRS_VXWORKS_MAJOR) && (_WRS_VXWORKS_MAJOR >= 6) /* VxWorks 6.x */
    iosDevFind (sysBootParams.bootFile, (const char **) &pAddrString);
#else
    iosDevFind (sysBootParams.bootFile, (char **) &pAddrString);
#endif
    if (pAddrString != sysBootParams.bootFile)
        {
        devName[0] = EOS;
        strncat (devName, sysBootParams.bootFile, pAddrString - sysBootParams.bootFile);
        ioDefPathSet (devName);
        }
    return OK;
    }

#if VX_VERSION == 64
/*******************************************************************************
*
* usrNfs2CacheInit - initialize the NFS v2 cache parameters
*
* This routine initializes the NFS v2 cache parameters.
*
* RETURNS: N/A
*
* NOMANUAL
*/

void usrNfs2CacheInit
    (
    UINT32   defaultNumLines,
    UINT32   defaultLineSize,
    UINT32   defaultOptions
    )
    {
    nfs2MaxCacheLines     = defaultNumLines;
    nfs2BytesPerCacheLine = defaultLineSize;
    nfs2CacheOptions      = defaultOptions;
    }

/***************************************************************************
*
* usrNfs3CacheInit - initialize the NFS v3 cache parameters
*
* This routine initializes the NFS v3 cache parameters.
*
* RETURNS: N/A
*
* NOMANUAL
*/

void usrNfs3CacheInit
    (
    UINT32   defaultNumLines,
    UINT32   defaultLineSize,
    UINT32   defaultOptions
    )
    {
    nfs3MaxCacheLines	  = defaultNumLines;
    nfs3BytesPerCacheLine = defaultLineSize;
    nfs3CacheOptions      = defaultOptions;
    }

#endif /* VX_VERSION == 64 */

#endif /* (INCLUDE_NFS2_CLIENT) || defined (INCLUDE_NFS3_CLIENT) 
          || defined (INCLUDE_NFS_CLIENT_ALL) */

#if defined (INCLUDE_NFS_MOUNT_ALL)

void usrNetNfsMountAll ()
    {
    group_export = GROUP_EXPORTS;
    printf ("Mounting NFS file systems from host %s", sysBootParams.hostName);
    if (sysBootParams.targetName[0] != EOS)\
       printf (" for target %s:\n", sysBootParams.targetName);\
    else
       printf (":\n");

    if (nfsMountAll (sysBootParams.hostName, sysBootParams.targetName, FALSE) !=OK)
        printf ("... nfs mount all failed\n");
    else
        printf ("...done\n");
    }
#endif

#ifdef INCLUDE_TUNNELLIB

/* globals */

TUNNEL_CONFIG_PARAMS tunnelDefaultConfigParams;

#ifdef VIRTUAL_STACK
VS_REG_ID   tunRegNum;   
#endif

/*
 * Note that usrNetTunnelMuxReg() must also be called to register with the MUX.
 */

STATUS usrNetTunnelLibInit (void)
    {
    STATUS error;
    TUNNEL_CONFIG_PARAMS * pCfg;
    
    /* tunnelLibInit - Common initialisation for Virtual Stack and non-VS */

    error = tunnelLibInit();
    if (error != OK)
        {
        if (_func_printErr)
            (*_func_printErr) ("tunnelLibInit() failed\n");
        return (ERROR);
        }
    pCfg = &tunnelDefaultConfigParams;

    bzero ((char *)pCfg, sizeof (TUNNEL_CONFIG_PARAMS));
    pCfg->cfgh.len = sizeof (TUNNEL_CONFIG_PARAMS);
    pCfg->mtuRefreshDelay = TUNNEL_PMTU_REFRESH_CFG;
    pCfg->numXin4Protos = 2;
    pCfg->numXin6Protos = 2;
    pCfg->Xin4ProtoArr = (TUN_PROTO *)malloc (pCfg->numXin4Protos *
                                              sizeof(TUN_PROTO));
    if (pCfg->Xin4ProtoArr == NULL)
        {
        if (_func_printErr)
            (*_func_printErr) ("usrNetTunnelLibInit() failed (Xin4 malloc)\n");
        return (ERROR);
        }

    pCfg->Xin6ProtoArr = (TUN_PROTO *)malloc (pCfg->numXin6Protos *
                                              sizeof(TUN_PROTO));
    if (pCfg->Xin6ProtoArr == NULL)
        {
        if (_func_printErr)
            (*_func_printErr) ("usrNetTunnelLibInit() failed (Xin6 malloc)\n");
        free (pCfg->Xin4ProtoArr);
        return (ERROR);
        }

    pCfg->Xin4ProtoArr[0].pr_protocol  = IPPROTO_IPV4;
    pCfg->Xin4ProtoArr[0].pr_input     = (FUNCPTR)tunnelIPv4Input;
    pCfg->Xin4ProtoArr[0].pr_ctlinput  = (FUNCPTR)tunnelIPv4inIPv4_ctlinput;
    
    pCfg->Xin4ProtoArr[1].pr_protocol  = IPPROTO_IPV6;
    pCfg->Xin4ProtoArr[1].pr_input     = (FUNCPTR)tunnelIPv4Input;
    pCfg->Xin4ProtoArr[1].pr_ctlinput  = (FUNCPTR)tunnelIPv6inIPv4_ctlinput;

    pCfg->Xin6ProtoArr[0].pr_protocol  = IPPROTO_IPV4;
    pCfg->Xin6ProtoArr[0].pr_input     = (FUNCPTR)tunnelIPv6Input;
    pCfg->Xin6ProtoArr[0].pr_ctlinput  = (FUNCPTR)tunnelIPv4inIPv6_ctlinput;
    
    pCfg->Xin6ProtoArr[1].pr_protocol  = IPPROTO_IPV6;
    pCfg->Xin6ProtoArr[1].pr_input     = (FUNCPTR)tunnelIPv6Input;
    pCfg->Xin6ProtoArr[1].pr_ctlinput  = (FUNCPTR)tunnelIPv6inIPv6_ctlinput;

#ifdef VIRTUAL_STACK
    
    /* vsComponentRegister() with tunnelInstInit() */
    
    return vsComponentRegister (&tunRegNum, "tunnelLib",
                                VS_SC_ENABLED,
                                VS_SC_OPTIONAL,
                                tunnelInstInit,
                                sizeof (TUNNEL_CONFIG_PARAMS),
                                &tunnelDefaultConfigParams,
                                tunnelInstDestroy);
#else
    error = tunnelInstInit (&tunnelDefaultConfigParams);
    if (error != OK)
        {
        if (_func_printErr)
            (*_func_printErr) ("tunnelInstInit() failed\n");
        return (ERROR);
        }
    return (OK);
#endif /* VIRTUAL_STACK */
    }
#endif /* INCLUDE_TUNNELLIB */

#ifdef INCLUDE_TUNNEL_MUX_REG

STATUS usrNetTunnelMuxReg (void)
    {
    STATUS error;
    
    /* Add if_output function for tunnel interfaces */
    
    error = muxIfFuncAdd (M2_ifType_tunnel, ETHERTYPE_IP, IF_OUTPUT_FUNC, 
                          tunnelIfOutput);
    if (error != OK)
        {
        if (_func_printErr)
            (*_func_printErr) ("First muxIfFuncAdd() failed\n");
        return (ERROR);
        }
                  
    error = muxIfFuncAdd (M2_ifType_tunnel, ETHERTYPE_IPV6, IF_OUTPUT_FUNC, 
                          tunnelIfOutput);
    if (error != OK)
        {
        if (_func_printErr)
            (*_func_printErr) ("Second muxIfFuncAdd() failed\n");
        return (ERROR);
        }

    return (OK);
    }
    
#endif /* INCLUDE_TUNNEL_MUX_REG */ 


#ifdef INCLUDE_MIB2_IF
STATUS usrNetM2IfInit (void)
    {
    return m2IfInit (MIB2IF_TRAP_RTN, MIB2IF_TRAP_ARG);
    }
#endif

#ifdef INCLUDE_MIB2_IP
LOCAL STATUS usrNetM2IpInit (void)
    {
    return m2IpInit (MIB2IP_MAX_ROUTE_TBL_SIZE);
    }
#endif

#ifdef INCLUDE_MIB2_IPV6
LOCAL STATUS usrNetM2Ipv6IfInit (void)
    {
    return m2Ipv6IfInit (MIB2IPV6_MAX_ROUTE_TBL_SIZE);
    }
#endif

#ifdef INCLUDE_MIB2_SYSTEM

LOCAL STATUS usrNetM2SysInit (void)
    {
    static M2_OBJECTID sysObjId = { MIB2SYS_OID_LEN, MIB2SYS_OID };

    return (m2SysInit (MIB2SYS_DESCR, MIB2SYS_CONTACT, MIB2SYS_LOCATION,
                       &sysObjId));
    }
#endif

#ifdef INCLUDE_PROXY_SERVER
extern PROXY_ARP_CONFIG_PARAMS proxy_arpDefaultConfigParams;
extern STATUS proxy_arpInstInit (void *InitValues); 
#ifdef VIRTUAL_STACK
extern VS_REG_ID proxyArpRegistrationNum;
extern STATUS proxy_arpDestructor (VSNUM vsnum);
#endif /* VIRTUAL_STACK */

STATUS usrProxyServerInit (void)
{
    bzero ((char *) &proxy_arpDefaultConfigParams,
	   sizeof(PROXY_ARP_CONFIG_PARAMS));
    proxy_arpDefaultConfigParams.cfgh.len = sizeof (PROXY_ARP_CONFIG_PARAMS);

/* Parameters specify the internal hash table sizes. Each must be equal 
   to a power of two, or zero to use a default size value. */
    proxy_arpDefaultConfigParams.cfg_clnt_tbl_sz = CLNT_TBL_SZ_CFG;
    proxy_arpDefaultConfigParams.cfg_port_tbl_sz = PORT_TBL_SZ_CFG;

    proxy_arpDefaultConfigParams.cfg_proxyd_main_addr = PROXYD_MAIN_ADDRESS;
    proxy_arpDefaultConfigParams.cfg_proxyd_proxy_addr = PROXYD_PROXY_ADDRESS; 

    proxy_arpDefaultConfigParams.cfg_arpDebug = ARP_DEBUG_CFG;
    proxy_arpDefaultConfigParams.cfg_proxyArpVerbose = PROXY_ARP_VERBOSE_CFG;
    proxy_arpDefaultConfigParams.cfg_proxyBroadcastVerbose = PROXY_BROADCAST_VERBOSE_CFG;
    proxy_arpDefaultConfigParams.cfg_proxyBroadcastFwd = PROXY_BROADCAST_FWD_CFG;
    proxy_arpDefaultConfigParams.cfg_arpRegister = ARP_REGISTER_CFG;

#ifdef VIRTUAL_STACK
    if (vsApplicationRegister (&proxyArpRegistrationNum,
                                "PROXY_ARP",
                                VS_SC_DISABLED,
                                proxy_arpInstInit,
                                sizeof(PROXY_ARP_CONFIG_PARAMS),
                                &proxy_arpDefaultConfigParams,
                                proxy_arpDestructor) == VS_OK)
	{
#if (PROXY_ARP_SERVER_IN_VS0 == TRUE)   /* it must be enabled for instance 0 */
	return vsComponentStatusSet (0, proxyArpRegistrationNum, VS_SC_ENABLED,
				     &proxy_arpDefaultConfigParams);
#else
        return (OK);
#endif
	}
    else 
	return (ERROR);
#else
    return proxy_arpInstInit(&proxy_arpDefaultConfigParams);
#endif
}

#endif /* INCLUDE_PROXY_SERVER */


#ifdef INCLUDE_RARP 
extern RARP_CONFIG_PARAMS  rarpDefaultConfigParams;
extern STATUS rarpInstInit (void *InitValues); 
#ifdef VIRTUAL_STACK
extern VS_REG_ID   rarpRegistrationNum;
extern STATUS rarpDestructor (VSNUM vsnum);
#endif

STATUS usrNetRarpInit (void)
{
    bzero ((char *)&rarpDefaultConfigParams, sizeof(RARP_CONFIG_PARAMS));
    rarpDefaultConfigParams.cfgh.len = sizeof(RARP_CONFIG_PARAMS);
    rarpDefaultConfigParams.cfg_rarpState.rarpDebug = RARP_DEBUG;

#ifdef VIRTUAL_STACK
    return vsComponentRegister  (&rarpRegistrationNum,
                                 "RARP",
                                 VS_SC_ENABLED,
                                 VS_SC_OPTIONAL,
                                 rarpInstInit,
                                 sizeof(RARP_CONFIG_PARAMS),
                                 &rarpDefaultConfigParams,
                                 rarpDestructor);
#else
    return rarpInstInit (&rarpDefaultConfigParams);
#endif

}
#endif /* INCLUDE_RARP */

#ifdef     INCLUDE_VIRTUAL_STACK  
VSID mgmtStackId;	/* Management stack identifier */


STATUS usrVirtualStackInit ()
    {
	
	VS_MGR_CFG_DATA cfgData;

        bzero ( (char *)&cfgData, sizeof (VS_MGR_CFG_DATA));

	cfgData.VsDeleteTimeo = VS_DELTIMEO_CFG;
	cfgData.VsSocketTimeo = VS_SOCKTIMEO_CFG;
	cfgData.VsApplTimeo   = VS_APPLTIMEO_CFG;

#ifdef INCLUDE_VIRTUAL_STACK_DEBUG
    cfgData.VsDebEnabled = TRUE;
#else
    cfgData.VsDebEnabled = FALSE;
#endif /* INCLUDE_VIRTUAL_STACK_DEBUG */

    if (vsLibInit (&cfgData) != VS_OK)
        {
        logMsg ("usrVirtualStackInit: unable to initialize virtual stack support.\n",
	 0, 0, 0, 0, 0, 0);
        return (ERROR);
        }
	
    if (vsCreate ("stack0", &mgmtStackId) != VS_OK)
       {
       logMsg ("usrVirtualStackInit: Unable to setup management stack.\n", 
	0, 0, 0, 0, 0, 0);
       return (ERROR);
       }
     
    return (OK);
    }

STATUS usrVirtualStackCoreInit ()
    {
    /* 
     * Initialise Non-Optional and Optional Components.
     *
     * Use stack num 0 (Management Stack)
     */
    
    vsStart (0, VS_COMP);

    vsStatusSet (VS_RUNNING);

    return (OK);
    }

STATUS usrVirtualStackAppInit ()
    {
    /* 
     * Initialise Applications
     *
     * Use stack num 0 (Management Stack)
     */
    
    vsStart (0, VS_APPL);

    return (OK);
    }
#endif /* INCLUDE_VIRTUAL_STACK */

#ifdef INCLUDE_AIP

STATUS usrNetAutoIpInit (void)
    {
    STATUS  rc = OK;

    /* setup the autoIP */
    aipLibInit ();

#ifdef VIRTUAL_STACK
    /* register the AutoIP as an application to the virtual stack */
    rc = vsApplicationRegister (&aipRegistrationNum, "AIP",
                                VS_SC_ENABLED, aipInstInit,  
                                0, NULL, aipInstDestroy);
#endif /* VIRTUAL_STACK */

    return rc;
    }

#endif /* INCLUDE_AIP */

#ifdef INCLUDE_RLOGIN
#ifdef VIRTUAL_STACK
extern int rlogindvsnum;

extern VS_REG_ID rlogindRegistrationNum;

extern STATUS rlogindInstInit (void *noparams);
extern STATUS rlogindDestructor (VSNUM vsnum);
#endif /*VIRTUAL_STACK*/

STATUS usrRlogindStart (void)
    {
    
#ifdef VIRTUAL_STACK
    
    rlogindvsnum       = -1;
    
    /* now register at vsmanager to allow ftp to be started in a 
       chosen virtual stack; it can only be started in one stack*/ 
       

    if (vsApplicationRegister (&rlogindRegistrationNum,
                               "rlogin",
                               VS_SC_DISABLED,
                               rlogindInstInit,
                               0,
                               NULL,
                               rlogindDestructor) ==VS_OK)
	{ 
#if (RLOGIN_IN_VS0 == TRUE)   /* it must be enabled for instance 0 */
	return vsComponentStatusSet (0, rlogindRegistrationNum, VS_SC_ENABLED,
				     NULL);
#else
	return (OK);
#endif
	}
    else 
	return (ERROR);
#else
    rlogInit();
    return (OK);
#endif /* VIRTUAL_STACK */
    }                       /* Allows remote access to target */
#endif   

#ifdef INCLUDE_FASTPATH
#ifdef VIRTUAL_STACK
extern STATUS ffDestructor (VSNUM);
extern STATUS ffMonDestructor (VSNUM);
extern VS_REG_ID ffRegistrationNum;
extern VS_REG_ID ffMonRegistrationNum;
#endif

extern STATUS ffInstInit (void *params);
extern STATUS ffMonInstInit (void *params);
extern FF_CONFIG_PARAMS ffDefaultConfigParams;

#ifndef NONE
#define NONE    -1
#endif

#if FF_IPV4_INIT_RTN != NONE
extern struct ffObj * FF_IPV4_INIT_RTN (int, int, FIB_DISPATCH_TBL *); 
extern FIB_DISPATCH_TBL FF_IPV4_FIB_DISP_TBL;
#endif

#if FF_IPV6_INIT_RTN != NONE
extern struct ffObj * FF_IPV6_INIT_RTN (int, int, FIB_DISPATCH_TBL *);
extern FIB_DISPATCH_TBL FF_IPV6_FIB_DISP_TBL;
#endif

STATUS usrNetFFInit(void)
  {
  STATUS status;

  bzero ((char *)&ffDefaultConfigParams, sizeof (FF_CONFIG_PARAMS));

  ffDefaultConfigParams.cfgh.len = sizeof (FF_CONFIG_PARAMS);

#if FF_IPV4_INIT_RTN != NONE
  ffDefaultConfigParams.pIPv4FFInit = FF_IPV4_INIT_RTN;
  ffDefaultConfigParams.pFfIPv4FibDispTable = &FF_IPV4_FIB_DISP_TBL;
#else
  ffDefaultConfigParams.pIPv4FFInit = NULL;
  ffDefaultConfigParams.pFfIPv4FibDispTable = NULL;
#endif

#if FF_IPV6_INIT_RTN != NONE
  ffDefaultConfigParams.pIPv6FFInit = FF_IPV6_INIT_RTN;
  ffDefaultConfigParams.pFfIPv6FibDispTable = &FF_IPV6_FIB_DISP_TBL;
#else
  ffDefaultConfigParams.pIPv6FFInit = NULL;
  ffDefaultConfigParams.pFfIPv6FibDispTable = NULL;
#endif

#ifdef VIRTUAL_STACK
  status =  vsApplicationRegister (&ffRegistrationNum,
			      "Fastpath",
			      VS_SC_ENABLED,
			      ffInstInit,
			      sizeof (FF_CONFIG_PARAMS),
			      &ffDefaultConfigParams,
			      ffDestructor);

  status |= vsApplicationRegister (&ffMonRegistrationNum,
				"Fastpath RTM Mon",
				VS_SC_ENABLED,
				ffMonInstInit,
				0,
				NULL,
				ffMonDestructor);
#else
  status = ffInstInit(&ffDefaultConfigParams);
  status |= ffMonInstInit (NULL);
#endif /* VIRTUAL_STACK */
  return (status);
}
#endif /*INCLUDE_FASTPATH*/

#ifdef INCLUDE_QOS_INGRESS
IMPORT QOS_CONFIG_PARAMS	qosDefaultConfigParams;
IMPORT JOB_QUEUE_ID             QOS_JOBQ;

STATUS usrNetQosIngressLibInit (void)
    {
    /* Initialize parameters for the qosIngressLib */

    qosDefaultConfigParams.cfg_pJobQueue = QOS_JOBQ;
    qosDefaultConfigParams.cfg_jobQPri = QOS_JOBQ_PRI;
    qosDefaultConfigParams.cfg_qosDefaultPri = QOS_DEFAULT_PRI;

    return (qosIngressLibInit(&qosDefaultConfigParams));
    }
#endif /* INCLUDE_QOS_INGRESS */

#ifdef INCLUDE_ROUTE_STORAGE
IMPORT STATUS ipRouteLibInstInit (void *);
#ifdef VIRTUAL_STACK
IMPORT STATUS ipRouteLibDestructor (VSNUM);

extern VS_REG_ID ipRouteRegistrationNum;
#endif


void usrNetRouteStorageInit (void)
    {
    /* Initialize parameters for the ipRouteLib */

    ipRouteLibInit();

    bzero ((char *)&ipRouteDefaultConfigParams, 
           sizeof (IP_ROUTE_CONFIG_PARAMS_T));
#ifdef VIRTUAL_STACK
    ipRouteDefaultConfigParams.cfgh.len = sizeof (IP_ROUTE_CONFIG_PARAMS_T);
#endif /* VIRTUAL_STACK */
    ipRouteDefaultConfigParams.routeDefaultWeight = ROUTE_DEFAULT_WEIGHT;

#ifdef VIRTUAL_STACK
    vsComponentRegister (&ipRouteRegistrationNum,
                         "ROUTE_STORAGE",
                         VS_SC_ENABLED,
                         VS_SC_NON_OPTIONAL,
			 ipRouteLibInstInit,
                         sizeof (IP_ROUTE_CONFIG_PARAMS_T),
                         &ipRouteDefaultConfigParams,
			 ipRouteLibDestructor);
#else
    ipRouteLibInstInit (&ipRouteDefaultConfigParams);

#endif

    return;
    }

#endif /*INCLUDE_ROUTE_STORAGE*/

#ifdef INCLUDE_FASTUDP
void usrNetFastUdpInit (void)
    {
    bzero ((char *)&fastUdpDefaultConfigParams, sizeof (FASTUDP_CONFIG_PARAMS));
    fastUdpDefaultConfigParams.cfgh.len = sizeof (FASTUDP_CONFIG_PARAMS);
    fastUdpDefaultConfigParams.cfg_priv_fastUdpDebug = FASTUDP_DEBUG_CFG;
    fastUdpDefaultConfigParams.cfg_pPortCfgTbl = portCfgTbl;
    fastUdpDefaultConfigParams.cfg_portCfgTblEntries = sizeof (portCfgTbl)/
                                                       sizeof (portCfgTbl[0]);

#ifdef VIRTUAL_STACK
    vsComponentRegister (&fastUdpRegistrationNum,
                         "FASTUDP",
                         VS_SC_ENABLED,
                         VS_SC_OPTIONAL,
                         fastUdpInstInit,
                         sizeof (FASTUDP_CONFIG_PARAMS),
                         &fastUdpDefaultConfigParams,
                         fastUdpDestructor);
#else
    fastUdpInstInit (&fastUdpDefaultConfigParams);
#endif
						       
    }
#endif /*INCLUDE_FASTUDP*/

#ifdef INCLUDE_FASTUDP6
void usrNetFastUdp6Init (void)
    {
    bzero ((char *)&fastUdp6DefaultConfigParams, sizeof (FASTUDP6_CONFIG_PARAMS));
    fastUdp6DefaultConfigParams.cfgh.len = sizeof (FASTUDP6_CONFIG_PARAMS);
    fastUdp6DefaultConfigParams.cfg_priv_fastUdp6Debug = FASTUDP6_DEBUG_CFG;
    fastUdp6DefaultConfigParams.cfg_pPortCfgTbl = port6CfgTbl;
    fastUdp6DefaultConfigParams.cfg_portCfgTblEntries = sizeof (port6CfgTbl)/
                                                       sizeof (port6CfgTbl[0]);

#ifdef VIRTUAL_STACK
    vsComponentRegister (&fastUdp6RegistrationNum,
                         "FASTUDP6",
                         VS_SC_ENABLED,
                         VS_SC_OPTIONAL,
                         fastUdp6InstInit,
                         sizeof (FASTUDP6_CONFIG_PARAMS),
                         &fastUdp6DefaultConfigParams,
                         fastUdp6Destructor);
#else
    fastUdp6InstInit (&fastUdp6DefaultConfigParams);
#endif
						       
    }
#endif /*INCLUDE_FASTUDP*/

#ifdef INCLUDE_HOST_TBL
LOCAL STATUS usrNetHostTblSetup (void)
    {
    STATUS result;

    if (hostTblInit () == ERROR)
        return (ERROR);

#ifdef INCLUDE_RTP
    /* Allow access to internal host table with system calls. */

    rtpHostLibInit ();
#endif

    /*
     * Create initial host table entries for the loopback address,
     * and for the local and remote IP addresses, if available.
     * The remote host information is required for the installation
     * of any network remote I/O device. The remaining startup sequence
     * may use a network I/O device, if it downloads a symbol table.
     */

    result = hostTblSetup (sysBootParams.hostName, sysBootParams.had,
                           pTgtName, pAddrString);

    return (result);
    }
#endif /*INCLUDE_HOST_TBL*/

#ifdef INCLUDE_DHCP6C
void usrNetDhcp6cInit (void)
    {
#ifdef VIRTUAL_STACK
    dh6cParams.uplinkIfName   = DHCP6C_UPLINK_IF;
    dh6cParams.downlinkIfName = DHCP6C_DOWNLINK_IF;
    dh6cParams.downStreamPort = DHCP6C_DPORT;
    dh6cParams.upStreamPort   = DHCP6C_UPORT;
    dh6cParams.priority       = DHCP6C_PRIORITY;
    dh6cParams.dhcp6cSize     = DHCP6C_SIZE;
    dh6cParams.dhcp6cKeyRealm  = DHCP6C_KEY_REALM_STRING;
    dh6cParams.dhcp6cKeySecret = DHCP6C_KEY_SECRET_STRING;
    dh6cParams.dhcp6cKeyExpire = DHCP6C_KEY_EXPIRE_STRING;
        
    /* now register at vsmanager to allow dhcp6 server to be started in a 
       chosen virtual stack; it can only be started in one stack*/ 
    
    if (vsApplicationRegister (&dhcp6cRegistrationNum,
                               "dhcp6c",
                               VS_SC_DISABLED,
                               dhcp6cInstInit,
                               sizeof (struct dhcp6cInitParams),
                               &dh6cParams,
                               dhcp6cDestructor) == VS_OK)
	{
#if (DHCP6C_IN_VS0 == TRUE)   /* it must be enabled for instance 0 */
        if (vsComponentStatusSet(0, dhcp6cRegistrationNum, VS_SC_ENABLED,
                                 &dh6cParams) != OK)
            printf ("vsComponentStatusSet() failed for dhcp6c \n");
#endif
        }
    else 
        printf ("VS Application Register failed for dhcp6c \n");
#else
    dhcp6cLibInit (DHCP6C_UPLINK_IF, DHCP6C_DOWNLINK_IF,
                   DHCP6C_DPORT, DHCP6C_UPORT, DHCP6C_PRIORITY,
                   DHCP6C_SIZE, DHCP6C_KEY_REALM_STRING,
                   DHCP6C_KEY_SECRET_STRING, DHCP6C_KEY_EXPIRE_STRING);
#if FALSE  /* temporary disabled */
    if (DHCP6C_RUNATBOOT)
        {
        if (dhcp6c (DHCP6C_UPLINK_IF, DHCP6C_OPTION, DHCP6C_VERBOSE) != OK)
            printf ("usrNetDhcp6cInit(): calling dhcp6c() failed \n");
        }
#endif
#endif /* VIRTUAL_STACK */
    return;
    }
#endif /* INCLUDE_DHCP6C */

#ifdef INCLUDE_DHCP6S
void usrNetDhcp6sInit (void)
    {
#ifdef VIRTUAL_STACK
    dh6sParams.ramDiskSize    = DHCP6S_RAMDISK_SIZE;
    dh6sParams.downStreamPort = DHCP6S_DPORT;
    dh6sParams.upStreamPort   = DHCP6S_UPORT;
    dh6sParams.priority       = DHCP6S_PRIORITY;
    dh6sParams.dhcp6sSize     = DHCP6S_SIZE;
        
    /* now register at vsmanager to allow dhcp6 server to be started in a 
       chosen virtual stack; it can only be started in one stack*/ 
    
    if (vsApplicationRegister (&dhcp6sRegistrationNum,
                               "dhcp6s",
                               VS_SC_DISABLED,
                               dhcp6sInstInit,
                               sizeof (struct dhcp6sInitParams),
                               &dh6sParams,
                               dhcp6sDestructor) == VS_OK)
	{
#if (DHCP6S_IN_VS0 == TRUE)   /* it must be enabled for instance 0 */
        if (vsComponentStatusSet(0, dhcp6sRegistrationNum, VS_SC_ENABLED,
                                 &dh6sParams) != OK)
            printf ("vsComponentStatusSet() failed for dhcp6s \n");
#endif
	}
    else 
        printf ("VS Application Register failed for dhcp6s \n");
#else
    if (dhcp6sLibInit (DHCP6S_RAMDISK_SIZE, DHCP6S_DPORT, DHCP6S_UPORT,
                       DHCP6S_PRIORITY, DHCP6S_SIZE) != OK)
        printf ("dhcp6sLibInit() failed \n");
#if FALSE  /* temporary disabled */
    else
        {
        if (DHCP6S_RUNATBOOT)
            if (dhcp6s (DHCP6S_CONF_FILE_NAME, DHCP6S_DOWNLINK_IF,
                        DHCP6S_VERBOSE) != OK)
                printf ("dhcp6s() feailed \n");
        }
#endif
#endif /* VIRTUAL_STACK */
    return;
    }
#endif /* INCLUDE_DHCP6S */

#ifdef INCLUDE_DHCP6R
void usrNetDhcp6rInit (void)
    {
#ifdef VIRTUAL_STACK
    dh6rParams.srcAddr        = DHCP6R_SRC_ADDR;
    dh6rParams.servAddr       = DHCP6R_SERVER_ADDR;
    dh6rParams.uIfName        = DHCP6R_UPLINK_IF;
    dh6rParams.hopLimit       = DHCP6R_HOP_LIMIT;
    dh6rParams.downStreamPort = DHCP6R_DPORT;
    dh6rParams.upStreamPort   = DHCP6R_UPORT;
    dh6rParams.priority       = DHCP6R_PRIORITY;
    dh6rParams.dhcp6rSize     = DHCP6R_SIZE;
        
    /* now register at vsmanager to allow dhcp6 server to be started in a 
       chosen virtual stack; it can only be started in one stack*/ 
    
    if (vsApplicationRegister (&dhcp6rRegistrationNum,
                               "dhcp6r",
                               VS_SC_DISABLED,
                               dhcp6rInstInit,
                               sizeof (struct dhcp6rInitParams),
                               &dh6rParams,
                               dhcp6rDestructor)==VS_OK)
	{
#if (DHCP6R_IN_VS0 == TRUE)   /* it must be enabled for instance 0 */
        if (vsComponentStatusSet(0, dhcp6rRegistrationNum, VS_SC_ENABLED,
                                 &dh6rParams) != OK)
            printf ("vsComponentStatusSet() failed for dhcp6r\n");
#endif
	}
    else 
        printf ("VS Application Register failed for dhcp6r \n");
#else
    if (dhcp6rLibInit (DHCP6R_SRC_ADDR, DHCP6R_SERVER_ADDR, DHCP6R_UPLINK_IF,
                       DHCP6R_HOP_LIMIT, DHCP6R_DPORT, DHCP6R_UPORT,
                       DHCP6R_PRIORITY, DHCP6R_SIZE) != OK)
        printf ("dhcp6rLibInit() failed \n");
#if FALSE  /* temporary disabled */
    else
        {
        if (DHCP6R_RUNATBOOT)
            if (dhcp6relay (DHCP6R_DOWNLINK_IF, DHCP6R_VERBOSE) != OK)
                printf ("dhcp6relay() feailed \n");
        }
#endif
#endif /* VIRTUAL_STACK */
    return;
    }
#endif /* INCLUDE_DHCP6R */

#ifdef INCLUDE_MLD6_ROUTER
STATUS usrNetMld6rInit (void)
    {
#ifdef VIRTUAL_STACK
    /*
     * Register at vsmanager to allow mldr daemon to be started in a 
     * chosen virtual stack; it can only be started in one stack.
     */
    if (vsApplicationRegister (&mldrRegistrationNum, "MLDR",
                               VS_SC_ENABLED, mldrLibInit, 0,
                               NULL, mldrDestructor) == VS_OK)
        {
#if (MLDR_IN_VS0 == TRUE)   /* it must be enabled for instance 0 */
        vsComponentStatusSet (0, mldrRegistrationNum, VS_SC_ENABLED, NULL);
#endif 
        }
#else
    mldrLibInit();
#endif /* VIRTUAL_STACK */

    return (OK);
    }
#endif /* INCLUDE_MLD6_ROUTER */

#if (defined(INCLUDE_TCPV4) && defined(INCLUDE_NET_SYM_TBL))
/******************************************************************************
*
* advanceTcpPorts - advance TCP ports for runtime image
*
* This function advances the starting port for the runtime image by 2 to
* avoid delaying the loading of the symbol table.
* The bootrom FTP opens 2 sockets to download the runtime image.
* If the runtime image downloads the symbol table, the starting port,
* must be IPPORT_RESERVED_CFG + 2 or larger since
* IPPORT_RESERVED_CFG and IPPORT_RESERVED_CFG + 1 ports are in
* TIME_WAIT at the boot host.
*
* RETURNS: N/A
*
* NOMANUAL
*/

void advanceTcpPorts (void)
    {
    int fd;
    struct sockaddr_in sockaddr;
    int i;

    for (i=0; i<=1;++i)
        {
        if ((fd = socket (AF_INET, SOCK_STREAM, 0)) == ERROR)
            return;

        bzero ((char *)&sockaddr, sizeof (sockaddr));
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port = 0;
        sockaddr.sin_addr.s_addr = htonl (INADDR_ANY);

        bind (fd, (struct sockaddr *) &sockaddr, sizeof (sockaddr));
        close (fd);
        }

    return;

    }
#endif /* INCLUDE_NET_SYM_TBL */

#ifdef INCLUDE_MIP6_MN
/*
 *  Utility function, duplicated from coreip/common/utilslib/prefix.c, to be
 *  removed after the one in prefix.c is made global
 */
static int prefixcmd_getinet6sysctl
    (
    int code
    )
    {
    int mib[] = { CTL_NET, PF_INET6, IPPROTO_IPV6, 0 };
    int value;
    size_t size;

    mib [3] = code;
    size = sizeof (value);
    if (sysctl (mib, sizeof (mib) / sizeof (mib [0]), &value, &size, NULL, 0) < 0)
        return -1;
    else
        return value;
    }
#endif /* INCLUDE_MIP6_MN */

#endif /* __INCusrNetworkc */
