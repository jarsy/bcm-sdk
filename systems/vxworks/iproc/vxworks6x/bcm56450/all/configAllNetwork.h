/* configAllNetwork.h - default configuration header for network stack */

/*
 * Copyright (c) 2006-2007 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01b,23nov09,rjq  added INCLUDE_IPSNTPS_API and INCLUDE_IPWRAP_SNTPSCLOCKSET
                 and INCLUDE_IPWRAP_SNTPSNSECTOFRACTION.
01a,14jul09,mdk  merge from networking branch
03c,09dec08.ggz  Added INCLUDE_IPWRAP_IFSHOW
03b,17sep08,kjn  Don't define INCLUDE_NETPOOLSHOW by default in VIP
03a,05jul08,dlk  Added INCLUDE_VXMUX_PKT_POOL_MIN, INCLUDE_VXMUX_SLAB,
                 and INCLUDE_VXMUX_CMD_VXSLAB.
02z,16jun08,dlk  Renamed INCLUDE_NULLBUFPOOL as INCLUDE_VXMUX_NULLBUFPOOL.
                 Added INCLUDE_VXMUX_MBLK.
02y,20may08,dlk  MUX2/END2 support.
02x,14oct07,dlk  Added INCLUDE_MUX_IF_FUNC_DBASE for muxIfFuncBase scalability.
02w,18aug07,tkf  Made INCLUDE_NET_DRV and INCLUDE_NET_REM_IO independent of
                 IPv4. Define INCLUDE_FTP6 by default if INET6 is defined and
                 INET is not, as part of the fix for (CQ:WIND00102553).
02v,02aug07,dlk  Added INCLUDE_IPNET_IFQUEUE_FIFO_DEFAULT.
02u,20jul07,mze  removed unused TIPC and SM includes
02t,25apr07,tkf  Add IPv6-Only build support.
02s,28feb07,dlk  Added INCLUDE_IPWRAP_GETHOSTBYNAME,
                 INCLUDE_IPWRAP_GETHOSTBYADDR.
02r,27feb07,kch  Added INCLUDE_IPCOM_USE_TIME_CMD (WIND00089286).
02q,02feb07,jmt  Modify to cut down size of bootroms
02p,25jan07,tkf  Changes to make IPCOM_SHELL and IPTELNETS binary scalable.
02o,19jan07,tkf  Include FS related components only if
                 INCLUDE_IPCOM_USE_RAM_DISK is defined (WIND00085813).
02n,03jan07,dlk  Added INCLUDE_MBUF_UTIL1 and INCLUDE_MBUF_UTIL2.
02m,20dec06,kch  Restore previously removed components to the list of
                 'Excluded Network Facilities' section.
02l,18dec06,tkf  Define INCLUDE_IPTELNETS only if
                 IPCOM_USE_SHELL == IPCOM_SHELL_IPCOM
02k,14dec06,tkf  Include ipcom_config.h instead of ipcom_pconfig.h.
02j,08dec06,kch  Added INCLUDE_IPSYSCTL_CMD to the if FALSE section for
                 reference.
02i,06dec06,kch  Restore INCLUDE_NET_SYSCTL and INCLUDE_SC_SOCKLIB components
                 to the if FALSE section for reference.
02h,24nov06,kch  Added ipeap support.
02g,23nov06,kch  Updated default configurations. Also added
                 INCLUDE_IPWRAP_INETLIB.
02f,22nov06,kch  Removed coreip telnet server.
02e,31oct06,kch  Corrected typo for ipecho components.
02d,27oct06,kch  Restore L2 and NFS components to the 'Excluded Network
                 Facilities' section for references.
02c,23oct06,tkf  Included more packet configure params by default for IPNET.
02b,18oct06,kch  Removed ipldapc since it is a discontinued product.
02a,27sep06,kch  Added ipmpls and ipeap components.
01z,26sep06,kch  Added ipradius and ipldapc components.
01y,22sep06,jfb  Added ipsec, ike, and ssh defines
01x,19sep06,kch  Added support for mobility shell commands.
01w,16sep06,kch  Added IPCOM dependencies for VxWorks file system.
01v,17sep06,kch  Added IPPPP and IPPPPOE support.
01u,13sep06,tkf  Added IPDHCPC6 and IPDHCPS6 support.
01t,12sep06,kch  Added INCLUDE_IPWRAP_IFNAME for backward compatibility.
01s,09sep06,dlk  Restore INCLUDE_NET_POOL. endEtherAddressForm() needs it.
01r,07sep06,tkf  Move INCLUDE_IPCOM_USE_INET6 to if FALSE section.
01q,02sep06,kch  Update based on Aug 22, 2006 code drop - added INCLUDE_IPAIP,
                 INCLUDE_IPPROXYARP and INCLUDE_IPNET6_AUTOCONF_PRIVACY_EXT.
01p,31aug06,tkf  Added INCLUDE_IPWRAP_ARP and INCLUDE_ARP_API for
                 arp APIs backwards compatibility.
01p,24aug06,kch  Added INCLUDE_RLOGIN and INCLUDE_TELNET for backward
                 compatibility.
01o,21aug06,kch  Added support INCLUDE_IPNET_USE_VRRPD component.
01m,11aug06,kch  Renamed INCLUDE_IPSNTP_SERVER to INCLUDE_IPSNTPS and
                 INCLUDE_IPSNTP_CLIENT to INCLUDE_IPSNTPC for consistency.
01l,10aug06,kch  Renamed INCLUDE_IPCOM_USE_SYSLOGD to INCLUDE_IPCOM_SYSLOGD_CMD.
                 Also added INCLUDE_IPCOM_SYSVAR_CMD to default configuration.
01k,03aug06,kch  Renamed INCLUDE_Xxx_CMD to INCLUDE_IPXxx_CMD for consistency.
01j,03aug06,kch  Added INCLUDE_IPWRAP_PING wrapper support. Also added
                 INCLUDE_PING for backward compatibility.
01i,02aug06,kch  Added INCLUDE_IPWRAP_PING6 wrapper support. Also added
                 INCLUDE_PING6 for backward compatibility.
01h,02aug06,kch  Added some common network infrastructure components as
                 default configuration.
01g,31jul06,kch  Added INCLUDE_IFCONFIG, INCLUDE_NETSTAT, INCLUDE_ROUTECMD,
                 INCLUDE_RIPNG, INCLUDE_GETADDRINFO, INCLUDE_GETNAMEINFO,
                 INCLUDE_GETSERVBYNAME and INCLUDE_GETSERVBYPORT for backward
                 compatibility.
01f,31jul06,kch  Added wrapper support for INCLUDE_IPWRAP_GETADDRINFO,
                 INCLUDE_IPWRAP_GETNAMEINFO, INCLUDE_IPWRAP_GETSERVBYNAME
                 and INCLUDE_IPWRAP_GETSERVBYPORT.
01e,31jul06,kch  Added wrapper support for INCLUDE_IPWRAP_ROUTECMD and
                 INCLUDE_IPWRAP_NETSTAT.
01d,28jul06,tlu  Added support for IPRIPNG
01c,22jul06,kch  Added support for IPRIP components.
01b,18jul06,kch  Corrected IPNet dependencies. Added 'Excluded Network
                 Facilities' section for references. Also moved all network
                 configuration parameters to configNetParams.h.
01a,03jul06,kch  Created.
*/

#ifndef INCconfigAllNetworkh
#define INCconfigAllNetworkh

/* IPCOM vxWorks port include header */
#include "ipcom_config.h"

/******************************************************************************/
/*                                                                            */
/*                         INCLUDED NETWORK FACILITIES                        */
/*                                                                            */
/******************************************************************************/
#define INCLUDE_COMMON_NET      /* Common network infrastructure */
#define INCLUDE_NET_BOOT         /* Boot parameter process */
#define INCLUDE_END_COMMON       /* END/END2 common interface support */
#define INCLUDE_MUX_COMMON       /* Common MUX support, all bind styles */
#define INCLUDE_END		 /* END style network drivers */
#define INCLUDE_MUX2		 /* mux2Bind() protocol API support */
#define INCLUDE_MUX              /* muxBind() protocol API support */
#define INCLUDE_MUXTK		 /* muxTkBind() protocol API support */
#define INCLUDE_MUXTK_OVER_END   /* muxTkBind() protocols over END devices */
#define INCLUDE_MUX2_OVER_END       /* mux2Bind() protocol over END device */
#define INCLUDE_END_ETHER_HDR	 /* M_BLK ethernet header build/parse */
#define INCLUDE_NET_DAEMON       /* Network Daemon Support */
#define INCLUDE_NETBUFPOOL       /* netBufLib Generic Pool */
#define INCLUDE_LINKBUFPOOL      /* netBufLib Link Buf Pool */
#define INCLUDE_VXMUX_MBLK	 /* M_BLK <--> Ipcom_pkt conversions */
#define INCLUDE_VXMUX_NULLBUFPOOL   /* minimal buffer pool back end */

#define INCLUDE_IPNET            /* IPNet stack infrastructure */

/*
 * Includes all the default IPNet stack components only if IPNET is included.
 * This allows user to easily exclude the IPNet stack if network support
 * is not required.
 */
#ifdef INCLUDE_IPNET
#define INCLUDE_IPNET_IFCONFIG_1      /* IPNet Interface configuration */
#define INCLUDE_IPNET_PACKET_POOL_3   /* IPNet packet pools #3 configuration */
#define INCLUDE_IPNET_PACKET_POOL_4   /* IPNet packet pools #4 configuration */
#define INCLUDE_IPNET_PACKET_POOL_5   /* IPNet packet pools #5 configuration */
#define INCLUDE_IPNET_PACKET_POOL_6   /* IPNet packet pools #6 configuration */
#define INCLUDE_IPNET_USE_LOOPBACK    /* IPNet loopback interface support */
#define INCLUDE_IPCOM_USE_ETHERNET    /* Ethernet Interface support */
#ifdef INET
#define INCLUDE_IPCOM_USE_INET        /* IPNet IPv4 support */
#endif
#ifdef INET6
#define INCLUDE_IPCOM_USE_INET6       /* IPNet IPv6 support */
#endif
#define INCLUDE_IPNET_USE_SOCK_COMPAT /* Socket backend */
#define INCLUDE_IPNET_USE_ROUTESOCK   /* routing socket support */

/*
 * The INCLUDE_IPAIP component enables the support for RFC3927, Dynamic
 * Configuration of IPv4 Link-Local Addresses. Configuration parameters
 * for INCLUDE_IPAIP are only meaningful if the IPNet stack is compiled
 * with the IPNET_USE_RFC3927 define.
 */
#if defined(INCLUDE_IPCOM_USE_INET) && defined(INCLUDE_IPCOM_USE_ETHERNET)
#define INCLUDE_IPAIP               /* RFC3927 autoIP */
#define INCLUDE_IPAIP_GLOBAL_CONFIGS /* Global IPv4 AutoIP configurations */
#define INCLUDE_IPPROXYARP          /* ProxyARP */
#endif /* INCLUDE_IPCOM_USE_INET && INCLUDE_IPCOM_USE_ETHERNET */

#ifdef INCLUDE_IPCOM_USE_INET6
#define INCLUDE_IPRADVD               /* Router advertisement */
#endif /* INCLUDE_IPCOM_USE_INET6 */
#define INCLUDE_IPTCP                 /* TCP */

/*
 * IPCOM/IPNet dependencies on VxWorks
 */
#if defined(IPCOM_USE_RAM_DISK) && (IPCOM_USE_FILE == IPCOM_FILE_NATIVE) && \
    defined(INCLUDE_IPCOM_USE_RAM_DISK)
#define INCLUDE_DISK_UTIL
#define INCLUDE_DOSFS_MAIN
#define INCLUDE_DOSFS_FMT
#define INCLUDE_XBD_RAMDRV
#endif /* IPCOM_USE_RAM_DISK */

#endif /* INCLUDE_IPNET */

#define INCLUDE_NETMASK_GET      /* network device netmask setup */
#define INCLUDE_BOOT_LINE_INIT	 /* The boot line routines */
#define INCLUDE_MBUF_UTIL1	 /* commonly used legacy mbuf utilities */
#define INCLUDE_REMLIB           /* Remote Command Library */

/*
 * IPv4 Only components
 */
#ifdef INET
#define INCLUDE_FTP              /* FTP client backend */
#endif /* INET */
#define INCLUDE_NET_DRV          /* Access to file system on boot host */
#define INCLUDE_NET_REM_IO       /* network remote I/O access */

#define INCLUDE_HOST_TBL         /* host table support */
#define INCLUDE_NET_HOST_SETUP   /* assign local hostname to target */

/*
 * Only undefine INCLUDE_APPL_LOG_UTIL if you rebuild the coreip-derived
 * components with logging completely disabled (LOG_ENABLE_MASK == 0).
 */
#define  INCLUDE_APPL_LOG_UTIL

/* The TFTP client is now included by default only for the bootrom build */

#ifdef BOOTAPP
#define  INCLUDE_TFTP_CLIENT    /* tftp client */
#endif

#if defined(INET6) && !defined(INET)
/*
 * Define this by default to allow symbol download with IPv6 Only stack.
 */
#define INCLUDE_FTP6             /* FTPv6 client backend */
#endif /* defined(INET6) && !defined(INET) */

/******************************************************************************/
/*                                                                            */
/*                        EXCLUDED NETWORK FACILITIES                         */
/*                                                                            */
/******************************************************************************/
#if FALSE

#define INCLUDE_END2                /* END2 style network driver support */
#define INCLUDE_MUX_OVER_END2	    /* muxBind() protocol over END2 device */
#define INCLUDE_MUXTK_OVER_END2     /* muxTkBind() protcol over END2 device */
#define INCLUDE_END2_LINKBUFPOOL /* linkBufPool for M_BLK protos over END2 */

#define INCLUDE_VXMUX_PKT_POOL_MIN  /* Ipcom_pkt pool in absence of IPNET */
#define INCLUDE_VXMUX_SLAB          /* IPCOM indep. slab memory allocator */
#define INCLUDE_VXMUX_CMD_VXSLAB    /* Display VXMUX slab pools */

#define  INCLUDE_MUX_L2             /* MUX Layer 2 suppport */
#define  INCLUDE_L2CONFIG           /* Layer 2 configuration utility */
#define  INCLUDE_QOS_INGRESS        /* Ingress QOS  support */
#define  INCLUDE_QOS_INGRESS_HOOKS  /* Ingress QOS hooks */

#define INCLUDE_NET_BOOT_CONFIG  /* network boot device configuration */
#define INCLUDE_NETMASK_GET      /* network device netmask setup */
#define INCLUDE_GTF              /* General Timer Facility Support */
#define INCLUDE_GTF_TIMER_START  /* Start GTF Timer Manager */
#define  INCLUDE_NETBUFADVLIB    /* Additional network buffer pool features */

#define INCLUDE_MUX_IF_FUNC_DBASE   /* MUX interface function database */

/*
 * Enable root fifo interface output queues.
 *
 * Note that this component, which may be required for reasonable performance
 * on certain interfaces (generally slower ones), can decrease performance
 * on other interfaces (generally faster ones). The output queue is
 * desirable when the CPU may deliver bursts of packets to be sent to an
 * interface faster than the interface can send them on the wire, when the
 * native TX queueing capacity of the device is not large enough to accomodate
 * the burst.  If the wire is 'faster than the CPU', or if the device has
 * sufficient TX queueing capacity for expected bursts, interface output
 * queue may not be needed.
 */
#define INCLUDE_IPNET_IFQUEUE_FIFO_DEFAULT


#define INCLUDE_MBUF_UTIL2       /* less commonly used legacy mbuf utilities */

#define  INCLUDE_NET_SYSCTL      /* Network sysctl tree support */

#define INCLUDE_IPCOM_USE_INET6  /* IPNet IPv6 support */

#define INCLUDE_IPCOM_AUTH_1     /* IPCom Authentication configuration */
#define INCLUDE_IPCOM_AUTH_2     /* IPCom Authentication configuration */
#define INCLUDE_IPCOM_AUTH_3     /* IPCom Authentication configuration */
#define INCLUDE_IPCOM_AUTH_4     /* IPCom Authentication configuration */
#define INCLUDE_IPCOM_AUTH_5     /* IPCom Authentication configuration */
#define INCLUDE_IPCOM_AUTH_6     /* IPCom Authentication configuration */

#define INCLUDE_IPNET_IFCONFIG_2     /* IPNet Interface configuration */
#define INCLUDE_IPNET_IFCONFIG_3     /* IPNet Interface configuration */
#define INCLUDE_IPNET_IFCONFIG_4     /* IPNet Interface configuration */

#define INCLUDE_IPNET_PACKET_POOL_1  /* IPNet packet pool configuration */
#define INCLUDE_IPNET_PACKET_POOL_2  /* IPNet packet pool configuration */
#define INCLUDE_IPNET_PACKET_POOL_7  /* IPNet packet pool configuration */
#define INCLUDE_IPNET_PACKET_POOL_8  /* IPNet packet pool configuration */
#define INCLUDE_IPNET_PACKET_POOL_9  /* IPNet packet pool configuration */
#define INCLUDE_IPNET_PACKET_POOL_10 /* IPNet packet pool configuration */
#define INCLUDE_IPNET_PACKET_POOL_11 /* IPNet packet pool configuration */

/* IPNet Link Layer */
#define INCLUDE_IPNET_USE_VLAN    /* VLAN Pseudo Interface support */
#define INCLUDE_IPNET_USE_TUNNEL  /* Tunnel Interface support */
#define INCLUDE_IPNET_USE_WLAN    /* Wireless Interface support */
#define INCLUDE_IPMPLS_TUNNEL     /* MPLS Tunnel Interface support */
#define INCLUDE_IPNET_USE_SOCKDEV /* Socket Interface support */

/* PPP support */
#define INCLUDE_IPPPP             /* PPP Interface support */
#define INCLUDE_IPPPPOE           /* PPPoE Interface support */
#define INCLUDE_IPPPP_INTERFACE_CONFIG  /* Per-interface PPP configurations */
#define INCLUDE_IPPPP_USERS_CONFIG   /* Per-user PPP configurations */

/* IPNet Protocols */
#define INCLUDE_IPNET_USE_MCAST_ROUTING  /* IPNet IPv4 Multicast routing */
#define INCLUDE_IPNET6_USE_MCAST_ROUTING  /* IPNet IPv6 Multicast routing */
#define INCLUDE_IPMCP_USE_IGMP   /* IPNet IPv4 IGMP support */
#define INCLUDE_IPMCP_USE_MLD    /* IPNet IPv6 MLD Protocol */
#define INCLUDE_IPNET_USE_NAT    /* IPNet NAT */
#define INCLUDE_IPVRRPD          /* IPNET VRRP */
#define INCLUDE_IPMPLS           /* IPNET MPLS */

/* IPEAP support */
#define INCLUDE_IPEAP                   /* EAP key management framework */
#define INCLUDE_IPEAP_TLS_COMMON        /* EAP-TLS common framework */
#define INCLUDE_IPEAP_USE_EAP_MD5       /* EAP-MD5 */
#define INCLUDE_IPEAP_USE_EAP_MSCHAPV2  /* EAP-MSCHAPV2 */
#define INCLUDE_IPEAP_USE_EAP_TLS       /* EAP-TLS */
#define INCLUDE_IPEAP_USE_EAP_TTLS      /* EAP-TTLS */
#define INCLUDE_IPEAP_USE_EAP_PEAP      /* PEAP */
#define INCLUDE_IPEAP_USE_EAP_SIM       /* EAP-SIM */

/* IPRadius support */
#define INCLUDE_IPRADIUS         /* IPNet Radius client */
#define INCLUDE_IPCOM_USE_AUTH_RADIUS  /* radius authentication support */

/* IPv6 autoconf privacy extensions */
#define INCLUDE_IPNET6_AUTOCONF_PRIVACY_EXT

/* IPv6 autoconf privacy extensions global configurations */
#define INCLUDE_IPNET6_PRIVACY_GLOBAL_CONFIGS

/* Per-interface IPv6 autoconf privacy configurations */
#define INCLUDE_IPNET6_PRIVACY_INTERFACE_CONFIGS

/* IPCOM file system support */
#define INCLUDE_IPCOM_USE_RAM_DISK   /* IPCOM RAM Disk Support */
#define INCLUDE_IPCOM_SYSLOGD_USE_LOG_FILE  /* IPCOM syslogd log file */

/* IPNet Mobility Protocols */
#define INCLUDE_IPMIPFA         /* IP Foreign Agent  */
#define INCLUDE_IPMIPFA_CMD     /* IP Foreign Agent shell command */
#define INCLUDE_IPMIPHA         /* IP Home Agent */
#define INCLUDE_IPMIPMN         /* IP Mobile Node */
#define INCLUDE_IPMIP6MN        /* IPv6 Mobile Node */
#define INCLUDE_IPMIP6MN_CMD    /* IPv6 Mobile Node shell command */

/* IPNet QoS */
#define INCLUDE_IPNET_DIFFSERV  /* Differentiated Services */
#define INCLUDE_IPNET_DS_SM     /* Simple Marker */
#define INCLUDE_IPNET_DS_SRTCM  /* Single Rate Three Color Marker */
#define INCLUDE_IPNET_USE_IFO_HTBC /* Hierarchical Token Bucket Container */

/* IPNet RIP */
#define INCLUDE_IPRIP              /* RIPv1/v2 */
#define INCLUDE_IPRIP_IFCONFIG_1   /* IPRIP interface configuration */
#define INCLUDE_IPRIP_IFCONFIG_2   /* IPRIP interface configuration */
#define INCLUDE_IPRIP_IFCONFIG_3   /* IPRIP interface configuration */
#define INCLUDE_IPRIP_IFCONFIG_4   /* IPRIP interface configuration */
#define INCLUDE_IPRIP_STATIC_ROUTE_1  /* Static RIP routes configuration */
#define INCLUDE_IPRIP_STATIC_ROUTE_2  /* Static RIP routes configuration */
#define INCLUDE_IPRIP_STATIC_ROUTE_3  /* Static RIP routes configuration */

/* RIPNG */
#define INCLUDE_IPRIPNG
#define INCLUDE_RIPNG   /* old coreip stack Routing Information Protocol ng */

/* IPNet Sockets */
#define INCLUDE_IPNET_USE_NETLINKSOCK  /* Netlink socket */

/* IPNet Tunnels */
#define INCLUDE_IPNET_USE_SIT    /* Simple Internet Transition Tunnels  */
#define INCLUDE_IPNET_USE_6TO4   /* 6to4 Tunnels */
#define INCLUDE_IPNET_USE_6OVER4 /* 6over4 Tunnels */
#define INCLUDE_IPNET_USE_GRE    /* Generic Router Encapsulation Tunnels */

/* IPNet Applications */
#define INCLUDE_IPDHCPC          /* DHCP Client */
#define INCLUDE_IPDHCPR          /* DHCP Relay Agent */
#define INCLUDE_IPDHCPS          /* DHCP Server */
#define INCLUDE_IPDHCPC6         /* DHCP6 Client */
#define INCLUDE_IPDHCPS6         /* DHCP6 Server */
#define INCLUDE_IPDNSC           /* DNS Client */
#define INCLUDE_IPFIREWALL       /* Firewall */
#define INCLUDE_IPFTPC           /* FTP Client */
#define INCLUDE_IPFTPS           /* FTP Server */
#define INCLUDE_IPTFTPC          /* TFTP Client */
#define INCLUDE_IPTFTPS          /* TFTP Server */
#define INCLUDE_IPSNTPS          /* SNTP Server */
#define INCLUDE_IPSNTPC          /* SNTP Client */
#define INCLUDE_IPSNTPC_API      /* SNTP Client API */
#define INCLUDE_IPSNTPS_API      /* SNTP Server API */
#define INCLUDE_IPAIP            /* IPv4 AutoIP */
#define INCLUDE_IPAIP_GLOBAL_CONFIGS  /* Global IPv4 AutoIP configs */
#define INCLUDE_IPAIP_INTERFACE_CONFIGS /* Per-interface IPv4 AutoIP configs */
#define INCLUDE_IPTELNETS        /* Telnet Server */

#define INCLUDE_USE_IPCOM_SHELL   /* IPCOM Shell */
#define INCLUDE_USE_NATIVE_SHELL  /* VxWorks native shell */

/* IPNet Shell Commands/Utilities */
#define INCLUDE_IPIFCONFIG_CMD        /* ifconfig */
#define INCLUDE_IPCOM_SYSVAR_CMD      /* IPCOM System variable tool */
#define INCLUDE_IPCOM_SYSLOGD_CMD     /* IPCOM system logging daemon */
#define INCLUDE_IPSYSCTL_CMD       /* IPCOM sysctl commands */
#define INCLUDE_IPARP_CMD          /* arp */
#define INCLUDE_IPECHO_CLIENT_CMD  /* TCP/UDP echo client */
#define INCLUDE_IPECHO_SERVER_CMD  /* TCP/UDP echo server */
#define INCLUDE_IPFTP_CMD          /* ftp client */
#define INCLUDE_IPD_CMD          /* IPD daemon control */
#define INCLUDE_IPFIREWALL_CMD   /* Ipfirewall configuration */
#define INCLUDE_IPNET_CTRL_CMD   /* IPNET runtime debugger utility */
#define INCLUDE_IPMCAST_PROXY_CMD  /* Multicast proxy */
#define INCLUDE_IPMEM_DEBUG_CMD    /* Memory debug tool */
#define INCLUDE_IPNAT_CMD          /* NAT */
#define INCLUDE_IPNDP_CMD          /* ndp */
#define INCLUDE_IPNETSTAT_CMD      /* netstat */
#define INCLUDE_IPNSLOOKUP_CMD     /* nslookup */
#define INCLUDE_IPPING_CMD         /* ping */
#define INCLUDE_IPPING6_CMD        /* ping6 */
#define INCLUDE_IPQUEUE_CONFIG_CMD /* IPNET output queue config utility */
#define INCLUDE_IPQOS_CMD          /* IPNET QoS configuration utility */
#define INCLUDE_IPRADVD_CMD /* IPNET router advertisement config utility */
#define INCLUDE_IPRIP_CTRL_CMD   /* RIP daemon control */
#define INCLUDE_IPRIPNG_CTRL_CMD   /* RIPng daemon control */
#define INCLUDE_IPROUTE_CMD          /* IPNET route table control command */
#define INCLUDE_IPNET_SCALETEST_CMD  /* Algorithm scaling test utility */
#define INCLUDE_IPSNTP_CMD         /* (S)NTP utility */
#define INCLUDE_IPSOCKPERF_CMD     /* Socket performance test tool */
#define INCLUDE_IPSOCKTEST_CMD     /* Socket test tool */
#define INCLUDE_IPTRACE_ROUTE_CMD  /* traceroute utility */
#define INCLUDE_IPTCP_TEST_CMD     /* TCP standard performance test tool */
#define INCLUDE_IPTFTP_CLIENT_CMD  /* tctp client */
#define INCLUDE_IPCOM_USE_TIME_CMD /* IPCOM time commands */
#define INCLUDE_IPVERSION_CMD      /* IPNet product version command */
#define INCLUDE_IPDHCPS6_CMD       /* IPNET DHCPv6 server command */
#define INCLUDE_IPCOM_USE_VXSHELL_CMD  /* IPCOM VxShell Command */


/* IPCOM Wrapper routines - for backwards compatibility */
/*
 * This component provides the backwards compatibility routines ipAttach(),
 * ip6Attach(), ipDetach(), and ip6Detach().
 */
#define INCLUDE_IPWRAP_IPPROTO

/*
 * This component provides the backwards compatibility routine getaddrinfo()
 */
#define INCLUDE_IPWRAP_GETADDRINFO
#define INCLUDE_GETADDRINFO    /* old coreip stack getaddrinfo() */

/*
 * This component provides the backwards compatibility routine getnameinfo()
 */
#define INCLUDE_IPWRAP_GETNAMEINFO
#define INCLUDE_GETNAMEINFO    /* old coreip stack getnameinfo() */

/*
 * This component provides the backwards compatibility routine gethostbyname()
 */
#define INCLUDE_IPWRAP_GETHOSTBYNAME
#define INCLUDE_GETHOSTBYNAME  /* old coreip stack gethostbyname() */

/*
 * This component provides the backwards compatibility routine gethostbyaddr()
 */
#define INCLUDE_IPWRAP_GETHOSTBYADDR
#define INCLUDE_GETHOSTBYADDR  /* old coreip stack gethostbyname() */

/*
 * This component provides the backwards compatibility routine getservbyname()
 */
#define INCLUDE_IPWRAP_GETSERVBYNAME
#define INCLUDE_GETSERVBYNAME  /* old coreip stack getservbyname() */

/*
 * This component provides the backwards compatibility routine getservbyport()
 */
#define INCLUDE_IPWRAP_GETSERVBYPORT
#define INCLUDE_GETSERVBYPORT   /* old coreip stack getservbyport() */

/*
 * This component provides the backwards compatibility routine ifconfig()
 */
#define INCLUDE_IPWRAP_IFCONFIG
#define INCLUDE_IFCONFIG      /* old coreip stack ifconfig command line/API */


/*
 * This component provides the backwards compatibility routine ifShow()
 */
#define INCLUDE_IPWRAP_IFSHOW

/*
 * This component provides the backwards compatibility routine routec()
 */
#define INCLUDE_IPWRAP_ROUTECMD
#define INCLUDE_ROUTECMD     /* old coreip stack routec command-line utility */

/*
 * This component provides the backwards compatibility routine netstat()
 */
#define INCLUDE_IPWRAP_NETSTAT
#define INCLUDE_NETSTAT     /* old coreip stack netstat command-line utility */

/*
 * This component provides the backwards compatibility routine ping()
 */
#define INCLUDE_IPWRAP_PING
#define INCLUDE_PING      /* old coreip stack ping client */

/*
 * This component provides the backwards compatibility routine ping6()
 */
#define INCLUDE_IPWRAP_PING6
#define INCLUDE_PING6     /* old coreip stack ping6 client */

/*
 * This component provides the backwards compatibility routines for arp
 * utility APIs such as arpAdd() and arpDelete().
 */
#define INCLUDE_IPWRAP_ARP
#define INCLUDE_ARP_API

/*
 * This component provides the backwards compatibility routines for
 * internet address manipulation routines such as inet_addr(), inet_ntoa(),
 * inet_aton(), inet_nton(), inet_pton() and etc.
 */
#define INCLUDE_IPWRAP_INETLIB
#define INCLUDE_INETLIB          /* inetLib */

/*
 * This component provides the backward compatibility routines for
 * ifname utilities such as if_nametoindex(), if_indextoname(),
 * if_nameindex() and if_freenameindex().
 */
#define INCLUDE_IPWRAP_IFNAME

/*
 * This component provides the backward compatibility routines for
 * getifaddrs() and freeifaddrs().
 */
#define INCLUDE_IPWRAP_GETIFADDRS

/*
 * This component provides the backward compatibility routine for
 * sntpcTimeGet().
 */
#define INCLUDE_IPWRAP_SNTPCTIMEGET

/*
 * This component provides the backward compatibility routine for
 * sntpsClockSet().
 */
#define INCLUDE_IPWRAP_SNTPSCLOCKSET

/*
 * This component provides the backward compatibility routine for
 * sntpsNsecToFraction().
 */
#define INCLUDE_IPWRAP_SNTPSNSECTOFRACTION


/*
 * The following components are provided for backward compatibility.
 */
#define INCLUDE_RLOGIN          /* Rlogin client/server */
#define INCLUDE_TFTP_CLIENT     /* TFTP client */
#define INCLUDE_TELNET_CLIENT   /* TELNET client */
#define INCLUDE_FTP6            /* FTP6 client backend */
#define INCLUDE_RPC             /* Remote Procedure Call system */

#define INCLUDE_NFS_SERVER_ALL  /* Include all NFS servers */
#define INCLUDE_NFS2_SERVER     /* Include NFS v2 server */
#define INCLUDE_NFS3_SERVER     /* Include NFS v3 server */
#define INCLUDE_NFS_CLIENT_ALL  /* Include All NFS clients */
#define INCLUDE_NFS2_CLIENT     /* Incldue NFS v2 client */
#define INCLUDE_NFS3_CLIENT     /* Include NFS v3 client */
#define INCLUDE_NFS_MOUNT_ALL   /* Include Mount all exported FS */

/* Socket Application Library */
#define INCLUDE_SAL_CLIENT      /* Socket Application Library (Client) */
#define INCLUDE_SAL_SERVER      /* Socket Application Library (Server) */
#define INCLUDE_SNS             /* Socket Name Service */
#define INCLUDE_SNS_RTP         /* Socket Name Service (RTP daemon suport) */
#define INCLUDE_SNS_MP          /* Dist. Socket Name Service */
#define INCLUDE_SNS_MP_RTP      /* Dist. Socket Name Service (RTP daemon) */
#define INCLUDE_SNS_SHOW        /* Socket Name Service show routines */

#define  INCLUDE_UN             /* AF_LOCAL address family */
#define  INCLUDE_UN_COMP        /* AF_LOCAL/COMP protocol  */
#define  INCLUDE_UN_SHOW        /* AF_LOCAL show routine support */
#define  INCLUDE_UN_COMP_SHOW   /* AF_LOCAL/COMP show routines */


#define  INCLUDE_NET_SYM_TBL     /* load symbol table from network */

#define  INCLUDE_SNMPD          /* SNMP Agent */
#define  INCLUDE_SNMPD_DEBUG    /* SNMP Agent debugging */

/* Security */
#define INCLUDE_IP_SECURITY             /* IPSec */
#define INCLUDE_INTERNET_KEY_EXCHANGE   /* IKE */
#define INCLUDE_SSH                     /* SSH */

/* Configure boot net device with IPv6 parameters */
#define INCLUDE_NET_BOOT_IPV6_CFG

/* The following components are available only with RTPs */
#define  INCLUDE_SC_SOCKLIB    /* Socket API System Call support */

#endif /* FALSE */

#endif /* INCconfigAllNetworkh */
