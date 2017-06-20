/* configNetParams.h - header for network stack configuration parameters */

/* $Id: configNetParams.h,v 1.3 2011/07/21 16:14:20 yshtil Exp $
 * Copyright (c) 2006-2007 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
02k,06mar07,kch  Added support for dynamic address, IPIKE_MAX_EXCHANGES and
                 IPIKE_ALLOW_DUPLICATE_SA configuration parameters 
                 to INCLUDE_INTERNET_KEY_EXCHANGE. Als changed default value
                 for IPNAT_ICMP_MAPPING_TIMEOUT from 5 to 10 seconds 
                 (WIND00089895).
02j,27feb07,kch  Removed FTPS_VERSION since it is an internal version string
                 that should not be made configurable. Changed default value
                 for IPF_MAX_STATEFUL_MAPPINGS from 100 to 1000. Also added
                 MIPMN_IPSEC_PROTECTED parameter for mobile node and renamed
                 MIPMN_IFNAME_LIST macro to MIPMN_IFNAME since only one
                 interface can be configured at any time in the mobile node
                 (WIND00089286).
02i,13feb07,dlk  Added NETBUF_LEADING_CLSPACE_DRV. 
02h,24jan07,kch  Added missing IPSSH configuration macros (WIND00086323).
02g,23jan07,kch  Added DHCPC_RFC2131_INIT_DELAY, DHCPC_RFC2131_EXP_BACKOFF,
                 DHCPC_DISCOVER_RETRIES and DHCPC_OFFER_TIMEOUT configuration
                 macros (WIND00086146).
02f,22jan07.kch  Added configuration parameters for PPP username and password
                 (WIND00085576).
02e,22jan07,kch  Renamed radius client configuration macros for ipcom 
                 authentication (WIND00083711).
02d,10jan07,kch  Updated default value for PPP_IPCP_PEER_IPV4_ADDRESS and
                 PPP_IPCP_PEER_IPV4_ADDRESS_POOL.
02c,03jan07,kch  Updated IPNET default values based on Jan 03 code drop. Also
                 removed PPPOE_IF_MAX_SESSIONS_LIST pppoe configuration param.
02b,20dec06,kch  Changed INET_MAX_MULTICAST_SOLICIT default value from 0 to 9.
02a,19dec06,kch  Changed default telnet port to 23 (was 2323).
01z,11dec06,kch  Added configuration parameters for ipftps, ipdhcpc, ipdhcpr
                 and ipdhcps callback hooks. Also added DHCPS_NETCONF_SYSVAR,
                 DHCPR_NETCONF_SYSVAR and IPMPLS_FWCONF_SYSVAR.
01y,01dec06,kch  Added additional configuration parameters for dhcp6 client,
                 dns client, sntp client/server and tcp. Also changed
                 INET_MIN_MTU_SIZE default value to 68 and added configuration
                 parameter for ipppp action callback hook.
01x,22nov06,kch  Added IPCOM_TELNET_AUTH_ENABLED and RADIUS_NAS_IDENTIFIER.
01w,22nov06,kch  Removed coreip telnet server.
01v,20oct06,tkf  Added packet pool configration components and redefined
                 the values of the existing ones.
01u,27sep06,kch  Added IPMPLS configuration parameters.
01t,27sep06,tkf  Set the default value of the IFCONFIG_1 consistent with
                 that defined in CDF.
01s,23sep06,kch  Renamed INCLUDE_IPNET_USE_VRRPD to INCLUDE_IPVRRPD.
01r,22sep06,jfb  Added ipsec, ike, and ssh params
01q,19sep06,kch  Added mobility support.
01p,18sep06,kch  Added IPPPP and IPPPPOE configuration support.
01o,13sep06,tkf  Added INCLUDE_IPDHCPC6 and INCLUDE_IPDHCPS6 dependenecies
01n,09sep06,jmt  Merge updates together
01m,09sep06,dlk  Increase number of 128-byte clusters in data pool, decrease
                 number of 64-byte clusters.
01l,08sep06,dlk  Restore INCLUDE_NET_POOL parameters, with decreased numbers
                 of M_BLKs/CL_BLKs/clusters, for endEtherAddressForm().
01m,08sep06,kch  Removed all defines for the old BOOTCONFIG.
01l,08sep06,kch  Restore IP_MAX_UNITS for backward compatibility.
01k,06sep06,dlk  Support taking the IPnet interface name from the END device
                 name.
01j,02sep06,kch  Update based on Aug 22, 2006 code drop - added INCLUDE_IPAIP,
                 INCLUDE_IPPROXYARP and INCLUDE_IPNET6_AUTOCONF_PRIVACY_EXT
                 configuration parameters (global and per-interface support).
                 Updated default value for INET_BASE_REACHABLE_TIME and
                 INET_MAX_MULTICAST_SOLICIT. Also added configuration
                 parameters for INCLUDE_IPNET_USE_NAT, IPF_FWMAC_RULE_FILE,
                 DHCPS_DO_ICMP_ADDRESS_CHECK and DHCPS_AUTHORIZED_AGENTS.
01i,28aug06,kch  Corrected IPCom authentication configuration support.
01h,24aug06,kch  Added INCLUDE_TELNET configuration parameters.
01g,12aug06,kch  Added INCLUDE_IPNET_USE_VRRPD configuration parameters. Also
                 updated INCLUDE_IPRADVD configuration parameters.
01f,09aug06,kch  Updated ipsntp, ipdhcpc and ipdnsc configuration parameters.
                 Guarded iptftp configuration parameters with 
                 INCLUDE_IPTFTP_COMMON instead of INCLUDE_IPTFTPS.
01e,09aug06,kch  Modified to define configuration parameter for a component
                 only if it is not yet defined.
01d,31jul06,kch  Added INCLUDE_IFCONFIG and INCLUDE_ROUTECMD configuration
                 parameters for backward compatibility. Also removed the
                 unsupported INCLUDE_GETADDRINFO configuration parameter.
01c,28jul06,tlu  Added RIPng configuration parameters
01b,22jul06,kch  Added RIP configuration parameters.
01a,18jul06,kch  Added network components configuration parameters (moved
                 from configAllNetwork.h).
*/

#ifndef INCconfigNetParamsh
#define INCconfigNetParamsh

/******************************************************************************/
/*                                                                            */
/*                   NETWORK STACK CONFIGURATION PARAMETERS                   */
/*                                                                            */
/******************************************************************************/

/*
 * IP_MAX_UNITS - IP Driver Control Limit
 * This parameter defines the maximum number of interfaces attached to the
 * IP layer. This parameter remains here for backward compatibility since
 * BSPs still using it. The IPNet stack do not use this configuration 
 * parameter at all.
 */
#ifndef IP_MAX_UNITS
#define IP_MAX_UNITS            4
#endif /* IP_MAX_UNITS */

#ifdef INCLUDE_NET_DAEMON
#ifndef NET_JOB_NUM_CFG
#define NET_JOB_NUM_CFG         85
#endif
#ifndef NET_TASK_PRIORITY
#define NET_TASK_PRIORITY       50
#endif
#ifndef NET_TASK_STACKSIZE
#define NET_TASK_STACKSIZE      10000
#endif
#ifndef NET_TASK_OPTIONS
#define NET_TASK_OPTIONS        VX_SUPERVISOR_MODE | VX_UNBREAKABLE
#endif
#endif /* INCLUDE_NET_DAEMON */

#ifdef INCLUDE_GTF
#ifndef GTF_NBRTIMERUNIT_CFG
#define GTF_NBRTIMERUNIT_CFG            7       /*   nbrTimerUnit, */
#endif
#ifndef GTF_BINPOWTIMERSLOT_CFG
#define GTF_BINPOWTIMERSLOT_CFG         9       /*   binPowTimerSlot, */
#endif
#ifndef GTF_BINPOWGRANSCALE_CFG
#define GTF_BINPOWGRANSCALE_CFG         2       /*   binPowGranScale, */
#endif
#ifndef GTF_SIZETIMERFREEQUEUE_CFG
#define GTF_SIZETIMERFREEQUEUE_CFG      100     /*   sizeTimerFreeQueue, */
#endif
#ifndef GTF_HZ_CFG
#define GTF_HZ_CFG                      10
#endif
#endif /* INCLUDE_GTF */

#ifdef INCLUDE_REMLIB
#ifndef RSH_STDERR_SETUP_TIMEOUT
#define RSH_STDERR_SETUP_TIMEOUT        -1
#endif
#endif /* INCLUDE_REMLIB */

#ifdef INCLUDE_MUX_L2
#ifndef MUX_L2_NUM_PORTS_CFG
#define MUX_L2_NUM_PORTS_CFG   16   /* number of ports that the device has */
#endif
#ifndef MUX_L2_MAX_VLANS_CFG
#define MUX_L2_MAX_VLANS_CFG   16   /* max number of 802.1Q VLANs supports */
#endif
#endif /* INCLUDE_MUX_L2 */

#ifdef INCLUDE_QOS_INGRESS
/* 
 * The following parameter specifies the Job queue that will be used to 
 * schedule the deferred queue processing job.
 */
#ifndef QOS_JOBQ
#define QOS_JOBQ      netJobQueueId  /* Ingress QoS Job Queue */
#endif

/*
 * The following parameter secifies the the priority at which the deferred
 * queue processing job will be scheduled on the Job queue defined by QOS_JOBQ.
 * Valid values range from 0 to 31. For correct operation, this value should
 * be less than the priority at which the driver receive routine executes.
 */
#ifndef QOS_JOBQ_PRI
#define QOS_JOBQ_PRI  NET_TASK_QJOB_PRI - 1  /* Ingress QoS Job Queue priority*/
#endif

/* 
 * The following parameters specifies the the priority queue on which the packet
 * is deferred for processing, if the packet classification routine returns 
 * QOS_DEFER_PKT with -1 in the priority field. Valid values range from 0 to 31.
 */
#ifndef QOS_DEFAULT_PRI
#define QOS_DEFAULT_PRI		0  /* default priority at which to defer pkts*/
#endif

#endif /* INCLUDE_QOS_INGRESS */

#ifdef INCLUDE_NETBUFLIB
/* Leading cluster pad space for qualifying 'driver' pools. */
#define NETBUF_LEADING_CLSPACE_DRV  64
#endif

#ifdef INCLUDE_NET_POOL
                                /* System pool */
#ifndef NUM_SYS_MBLKS
#define NUM_SYS_MBLKS		2
#endif
#ifndef NUM_SYS_CLBLKS
#define NUM_SYS_CLBLKS		2
#endif
#ifndef PMA_SYSPOOL
#define PMA_SYSPOOL		NULL
#endif
#ifndef PMS_SYSPOOL
#define PMS_SYSPOOL		0
#endif
#ifndef SIZ_SYS_16
#define SIZ_SYS_16		20
#endif
#ifndef NUM_SYS_16
#define NUM_SYS_16		2
#endif
#ifndef PMA_SYS_16
#define PMA_SYS_16		NULL
#endif
#ifndef PMS_SYS_16
#define PMS_SYS_16		0
#endif
#ifndef SIZ_SYS_32
#define SIZ_SYS_32		44
#endif
#ifndef NUM_SYS_32
#define NUM_SYS_32		0
#endif
#ifndef PMA_SYS_32
#define PMA_SYS_32		NULL
#endif
#ifndef PMS_SYS_32
#define PMS_SYS_32		0
#endif
#ifndef SIZ_SYS_64
#define SIZ_SYS_64		96
#endif
#ifndef NUM_SYS_64
#define NUM_SYS_64		0
#endif
#ifndef PMA_SYS_64
#define PMA_SYS_64		NULL
#endif
#ifndef PMS_SYS_64
#define PMS_SYS_64		0
#endif
#ifndef SIZ_SYS_128
#define SIZ_SYS_128		172
#endif
#ifndef NUM_SYS_128
#define NUM_SYS_128		0
#endif
#ifndef PMA_SYS_128
#define PMA_SYS_128		NULL
#endif
#ifndef PMS_SYS_128
#define PMS_SYS_128		0
#endif
#ifndef SIZ_SYS_256
#define SIZ_SYS_256		292
#endif
#ifndef NUM_SYS_256
#define NUM_SYS_256		0
#endif
#ifndef PMA_SYS_256
#define PMA_SYS_256		NULL
#endif
#ifndef PMS_SYS_256
#define PMS_SYS_256		0
#endif
#ifndef SIZ_SYS_512
#define SIZ_SYS_512		664
#endif
#ifndef NUM_SYS_512
#define NUM_SYS_512		0
#endif
#ifndef PMA_SYS_512
#define PMA_SYS_512		NULL
#endif
#ifndef PMS_SYS_512
#define PMS_SYS_512		0
#endif
#ifndef SIZ_SYS_1024
#define SIZ_SYS_1024		1144
#endif
#ifndef NUM_SYS_1024
#define NUM_SYS_1024		2
#endif
#ifndef PMA_SYS_1024
#define PMA_SYS_1024		NULL
#endif
#ifndef PMS_SYS_1024
#define PMS_SYS_1024		0
#endif
#ifndef SIZ_SYS_2048
#define SIZ_SYS_2048		2048
#endif
#ifndef NUM_SYS_2048
#define NUM_SYS_2048		0
#endif
#ifndef PMA_SYS_2048
#define PMA_SYS_2048		NULL
#endif
#ifndef PMS_SYS_2048
#define PMS_SYS_2048		0
#endif

                                /* Data pool */
#ifndef NUM_DAT_MBLKS
#define NUM_DAT_MBLKS		NUM_DAT_CLBLKS
#endif
#ifndef NUM_DAT_CLBLKS
#define NUM_DAT_CLBLKS		(NUM_DAT_64 + NUM_DAT_128 + NUM_DAT_256 + \
                                 NUM_DAT_512 + NUM_DAT_1024 + NUM_DAT_2048 + \
                                 NUM_DAT_4096 + NUM_DAT_8192 + NUM_DAT_16384 \
				 + NUM_DAT_32768 + NUM_DAT_65536)
#endif
#ifndef PMA_DATPOOL
#define PMA_DATPOOL		NULL
#endif
#ifndef PMS_DATPOOL
#define PMS_DATPOOL		0
#endif
#ifndef NUM_DAT_64
#define NUM_DAT_64		0
#endif
#ifndef PMA_DAT_64
#define PMA_DAT_64		NULL
#endif
#ifndef PMS_DAT_64
#define PMS_DAT_64		0
#endif
#ifndef NUM_DAT_128
#define NUM_DAT_128		128
#endif
#ifndef PMA_DAT_128
#define PMA_DAT_128		NULL
#endif
#ifndef PMS_DAT_128
#define PMS_DAT_128		0
#endif
#ifndef NUM_DAT_256
#define NUM_DAT_256		4
#endif
#ifndef PMA_DAT_256
#define PMA_DAT_256		NULL
#endif
#ifndef PMS_DAT_256
#define PMS_DAT_256		0
#endif
#ifndef NUM_DAT_512
#define NUM_DAT_512		4
#endif
#ifndef PMA_DAT_512
#define PMA_DAT_512		NULL
#endif
#ifndef PMS_DAT_512
#define PMS_DAT_512		0
#endif
#ifndef NUM_DAT_1024
#define NUM_DAT_1024		4
#endif
#ifndef PMA_DAT_1024
#define PMA_DAT_1024		NULL
#endif
#ifndef PMS_DAT_1024
#define PMS_DAT_1024		0
#endif
#ifndef NUM_DAT_2048
#define NUM_DAT_2048		4
#endif
#ifndef PMA_DAT_2048
#define PMA_DAT_2048		NULL
#endif
#ifndef PMS_DAT_2048
#define PMS_DAT_2048		0
#endif
#ifndef NUM_DAT_4096
#define NUM_DAT_4096		0
#endif 
#ifndef PMA_DAT_4096
#define PMA_DAT_4096		0
#endif
#ifndef PMS_DAT_4096
#define PMS_DAT_4096		0
#endif
#ifndef NUM_DAT_8192
#define NUM_DAT_8192		0
#endif 
#ifndef PMA_DAT_8192
#define PMA_DAT_8192		0
#endif 
#ifndef PMS_DAT_8192
#define PMS_DAT_8192		0
#endif       
#ifndef NUM_DAT_16384
#define NUM_DAT_16384		0
#endif
#ifndef PMA_DAT_16384
#define PMA_DAT_16384		0
#endif
#ifndef PMS_DAT_16384
#define PMS_DAT_16384		0
#endif       
#ifndef NUM_DAT_32768
#define NUM_DAT_32768		0
#endif
#ifndef PMA_DAT_32768
#define PMA_DAT_32768		0
#endif
#ifndef PMS_DAT_32768
#define PMS_DAT_32768		0
#endif
#ifndef NUM_DAT_65536
#define NUM_DAT_65536		0
#endif
#ifndef PMA_DAT_65536
#define PMA_DAT_65536		0
#endif
#ifndef PMS_DAT_65536
#define PMS_DAT_65536		0
#endif

#endif /* INCLUDE_NET_POOL */

/* SNMP configuration parameters */
#ifndef SNMP_TRACE_LEVEL
#define SNMP_TRACE_LEVEL            0    /* Must be >= 0 and <= 3 with higher */
                                         /* values giving more info and 0     */
                                         /* giving no info                    */
#endif /* SNMP_TRACE_LEVEL */


/* INCLUDE_SNS     || INCLUDE_SNS_MP
|| INCLUDE_SNS_RTP || INCLUDE_SNS_MP_RTP */
#if defined(INCLUDE_SNS_RTP) || defined(INCLUDE_SNS_MP_RTP)
#ifndef  SNS_PATHNAME
#define SNS_PATHNAME  "/romfs/snsServer.vxe"
#endif
#endif /* INCLUDE_SNS_RTP || INCLUDE_SNS_MP_RTP */

#if defined(INCLUDE_SNS) || defined(INCLUDE_SNS_MP) || \
    defined(INCLUDE_SNS_RTP) || defined(INCLUDE_SNS_MP_RTP)
#ifndef  SNS_PRIORITY
#define SNS_PRIORITY  50
#endif
#ifndef SNS_STACK_SIZE
#define SNS_STACK_SIZE  20000
#endif
#ifndef SNS_LISTEN_BACKLOG
#define SNS_LISTEN_BACKLOG  5
#endif
#endif /* INCLUDE_SNS || INCLUDE_SNS_MP || INCLUDE_SNS_RTP ... */

#if defined(INCLUDE_SNS_MP) || defined(INCLUDE_SNS_MP_RTP)
#ifndef SNS_DISTRIBUTED_SERVER_TYPE
#define SNS_DISTRIBUTED_SERVER_TYPE 70
#endif
#ifndef SNS_DISTRIBUTED_SERVER_INSTANCE
#define SNS_DISTRIBUTED_SERVER_INSTANCE 1
#endif
#endif /* INCLUDE_SNS_MP || INCLUDE_SNS_MP_RTP */


/* INCLUDE_DSI_POOL buffer pool */
#ifdef INCLUDE_DSI_POOL
#ifndef DSI_NUM_SOCKETS
#define DSI_NUM_SOCKETS    200
#endif
#ifndef DSI_DATA_32
#define DSI_DATA_32         50
#endif
#ifndef DSI_DATA_64
#define DSI_DATA_64        100
#endif
#ifndef DSI_DATA_128
#define DSI_DATA_128       200
#endif
#ifndef DSI_DATA_256
#define DSI_DATA_256        40
#endif
#ifndef DSI_DATA_512
#define DSI_DATA_512        40
#endif
#ifndef DSI_DATA_1K
#define DSI_DATA_1K         10
#endif
#ifndef DSI_DATA_2K
#define DSI_DATA_2K         10
#endif
#ifndef DSI_DATA_4K
#define DSI_DATA_4K         10
#endif
#ifndef DSI_DATA_8K
#define DSI_DATA_8K         10
#endif
#ifndef DSI_DATA_16K
#define DSI_DATA_16K         4
#endif
#ifndef DSI_DATA_32K
#define DSI_DATA_32K         0
#endif
#ifndef DSI_DATA_64K
#define DSI_DATA_64K         0
#endif
#endif /* INCLUDE_DSI_POOL */

/* INCLUDE_TIPC_SOCKET and INCLUDE_TIPC_MEMPOOL buffer pools */
#ifdef INCLUDE_TIPC_SOCKET
#ifndef TIPC_NUM_SOCKETS
#define TIPC_NUM_SOCKETS    200
#endif
#ifndef TIPC_SOCK_RXQ_LIMIT
#define TIPC_SOCK_RXQ_LIMIT   2500
#endif
#endif /* INCLUDE_TIPC_SOCKET */

#ifdef INCLUDE_TIPC_MEMPOOL
#ifndef TIPC_DATA_00064
#define TIPC_DATA_00064       120
#endif
#ifndef TIPC_DATA_00128
#define TIPC_DATA_00128       200
#endif
#ifndef TIPC_DATA_00256
#define TIPC_DATA_00256        40
#endif
#ifndef TIPC_DATA_00512
#define TIPC_DATA_00512        40
#endif
#ifndef TIPC_DATA_01024
#define TIPC_DATA_01024        50
#endif
#ifndef TIPC_DATA_02048
#define TIPC_DATA_02048        20
#endif
#ifndef TIPC_DATA_04096
#define TIPC_DATA_04096         2
#endif
#ifndef TIPC_DATA_08192
#define TIPC_DATA_08192         0
#endif
#ifndef TIPC_DATA_16384
#define TIPC_DATA_16384         0
#endif
#ifndef TIPC_DATA_32768
#define TIPC_DATA_32768         0
#endif
#ifndef TIPC_DATA_65536
#define TIPC_DATA_65536         0
#endif
#endif /* INCLUDE_TIPC_MEMPOOL */

/* INCLUDE_TIPC_CONFIG_STR */
#ifdef INCLUDE_TIPC_CONFIG_STR
#ifndef TIPC_CONFIG_STR
#define TIPC_CONFIG_STR ""
#endif
#endif

/* INCLUDE_TIPC_MEDIA_SM */
#ifndef SM_TIPC_PKT_Q_LEN
#define SM_TIPC_PKT_Q_LEN   0
#endif
#ifndef SM_TIPC_SM_PKT_SIZE
#define SM_TIPC_SM_PKT_SIZE 0
#endif
#ifndef SM_TIPC_NUM_BUF
#define SM_TIPC_NUM_BUF     60
#endif

#if 0
/* INCLUDE_WDB_COMM_END */
#ifndef WDB_END_MTU
#define WDB_END_MTU             1500
#endif
#ifndef WDB_END_DEVICE_NAME
#define WDB_END_DEVICE_NAME     NULL
#endif
#ifndef WDB_END_DEVICE_UNIT
#define WDB_END_DEVICE_UNIT     NULL
#endif
#endif /* #if 0 */

/******************************************************************************/
/*                                                                            */
/*                          IPNET STACK CONFIGURATION                         */
/*                                                                            */
/******************************************************************************/
#ifdef INCLUDE_IPCOM_SYSLOGD_CMD
#ifndef IPCOM_SYSLOGD_DEFAULT_PRIORITY
#define IPCOM_SYSLOGD_DEFAULT_PRIORITY   IPCOM_LOG_ERR  /* Default priority */
#endif
#ifndef IPCOM_SYSLOGD_QUEUE_MAX
#define IPCOM_SYSLOGD_QUEUE_MAX          256        /* maximum queue depth */
#endif
#ifndef IPCOM_SYSLOGD_LOG_FILE
#define IPCOM_SYSLOGD_LOG_FILE IPCOM_FILE_ROOT"syslog" /* log file */
#endif
#endif /* INCLUDE_IPCOM_SYSLOGD_CMD */

#ifdef INCLUDE_IPCOM_AUTH_1
#ifndef AUTH_USER_NAME1
#define AUTH_USER_NAME1     NULL  /* User name #1 */
#endif
#ifndef AUTH_USER_PASSWD1
#define AUTH_USER_PASSWD1   NULL  /* Password #1 */
#endif
#ifndef AUTH_USER_ID1
#define AUTH_USER_ID1       0     /* Userid #1 */
#endif
#endif /* INCLUDE_IPCOM_AUTH_1 */

#ifdef INCLUDE_IPCOM_AUTH_2
#ifndef AUTH_USER_NAME2
#define AUTH_USER_NAME2     NULL  /* User name #2 */ 
#endif
#ifndef AUTH_USER_PASSWD2
#define AUTH_USER_PASSWD2   NULL  /* Password #2 */ 
#endif
#ifndef AUTH_USER_ID2
#define AUTH_USER_ID2       0     /* Userid #2 */
#endif
#endif /* INCLUDE_IPCOM_AUTH_2 */

#ifdef INCLUDE_IPCOM_AUTH_3
#ifndef AUTH_USER_NAME3
#define AUTH_USER_NAME3     NULL  /* User name #3 */ 
#endif
#ifndef AUTH_USER_PASSWD3
#define AUTH_USER_PASSWD3   NULL  /* Password #3 */
#endif
#ifndef AUTH_USER_ID3
#define AUTH_USER_ID3       0     /* Userid #3 */
#endif
#endif /* INCLUDE_IPCOM_AUTH_3 */

#ifdef INCLUDE_IPCOM_AUTH_4
#ifndef AUTH_USER_NAME4
#define AUTH_USER_NAME4     NULL  /* User name #4 */ 
#endif
#ifndef AUTH_USER_PASSWD4
#define AUTH_USER_PASSWD4   NULL  /* Password #4 */
#endif
#ifndef AUTH_USER_ID4
#define AUTH_USER_ID4       0     /* Userid #4 */
#endif
#endif /* INCLUDE_IPCOM_AUTH_4 */

#ifdef INCLUDE_IPCOM_AUTH_5
#ifndef AUTH_USER_NAME5
#define AUTH_USER_NAME5     NULL  /* User name #5 */ 
#endif
#ifndef AUTH_USER_PASSWD5
#define AUTH_USER_PASSWD5   NULL  /* Password #5 */
#endif
#ifndef AUTH_USER_ID5
#define AUTH_USER_ID5       0     /* Userid #5 */
#endif
#endif /* INCLUDE_IPCOM_AUTH_5 */

#ifdef INCLUDE_IPCOM_AUTH_6
#ifndef AUTH_USER_NAME6
#define AUTH_USER_NAME6     NULL  /* User name #6 */
#endif
#ifndef AUTH_USER_PASSWD6
#define AUTH_USER_PASSWD6   NULL  /* Password #6 */
#endif
#ifndef AUTH_USER_ID6
#define AUTH_USER_ID6       0     /* Userid #6 */
#endif
#endif /* INCLUDE_IPCOM_AUTH_6 */

#ifdef INCLUDE_IPCOM_USE_AUTH_RADIUS
/*  RADIUS client configuration for IPCOM authentication */
#ifndef IPCOM_AUTH_RADIUS_SERVER_DOMAIN
/* RADIUS server domain, IP_AF_INET=2 or IP_AF_INET6=24. */
#define IPCOM_AUTH_RADIUS_SERVER_DOMAIN         2
#endif
#ifndef IPCOM_AUTH_RADIUS_SERVER_IPV4_ADDRESS
#define IPCOM_AUTH_RADIUS_SERVER_IPV4_ADDRESS   "10.1.2.1"
#endif
#ifndef IPCOM_AUTH_RADIUS_SERVER_IPV6_ADDRESS
#define IPCOM_AUTH_RADIUS_SERVER_IPV6_ADDRESS   "3ffe:300::5"
#endif
#ifndef IPCOM_AUTH_RADIUS_SERVER_PORT
#define IPCOM_AUTH_RADIUS_SERVER_PORT           1812
#endif
#ifndef IPCOM_AUTH_RADIUS_SERVER_SHARE_SECRET
#define IPCOM_AUTH_RADIUS_SERVER_SHARE_SECRET   "kallekula123"
#endif
#ifndef IPCOM_AUTH_RADIUS_SERVER_PASSWD_TYPE
/* RADIUS server password type PAP=1, CHAP=2. */
#define IPCOM_AUTH_RADIUS_SERVER_PASSWD_TYPE    1
#endif
#endif /* INCLUDE_IPCOM_USE_AUTH_RADIUS */

#ifdef INCLUDE_IPRADIUS
#ifndef RADIUS_NAS_IDENTIFIER
#define RADIUS_NAS_IDENTIFIER       "Wind River example NAS"
#endif
#endif /* INCLUDE_IPRADIUS */

#ifdef INCLUDE_USE_IPCOM_SHELL
#ifndef IPCOM_SHELL_PROMPT
#define IPCOM_SHELL_PROMPT "\\i$\\V>"           /* IPCOM shell prompt */
#endif
#endif /* INCLUDE_USE_IPCOM_SHELL */
          
/* 
 * Loopback interface configuration:
 * Specifies the loopback interface parameters to be configured when the
 * stack is initialized, e.g. "ifname lo0","inet 127.0.0.1/32",
 * "inet6 ::1/128","inet6 FF01::1/16".
 * The following keywords are recognized.
 * (1) "ifname" - specifies the name of loopback interface, e.g. "ifname eth0"
 * (2) "inet" - specifies loopback IPv4 address and subnet, 
 *     e.g. "inet 127.0.0.1/32". 
 * (3) "inet6" - specifies loopback IPv6 address and subnet,
 *     e.g. "inet6 ::1/128".
 */
#ifdef INCLUDE_IPNET_LOOPBACK_CONFIG
#ifndef IFCONFIG_LOOPBACK
#define IFCONFIG_LOOPBACK "ifname lo0","inet 127.0.0.1/32","inet6 ::1/128","inet6 FF01::1/16"
#endif
#endif /* INCLUDE_IPNET_LOOPBACK_CONFIG */

/*
 * IPNet Interface configuration:
 * Specifies the interface parameters to be configured when the stack is
 * initialized, as a list of attribute-value strings, for example:
 * "ifname eth0","devname fei0","gateway driver","inet 10.1.2.100/24",
 * "inet6 3ffe:1:2:3::4/64".
 * The following keywords are recognized.
 * (1) "ifname" - specifies the name of the ethernet interface,
 * e.g. "ifname eth0". If the interface name is missing after "ifname",
 * the vxWorks END device name will be used.
 * (2) "devname" - specifies which driver this interface should
 * attach itself, "fei0".
 * The default is set to "driver" so that device name is retrieved from
 * device boot parameters.
 * (3) "inet" - specifies the interface IPv4 address and subnet,
 * e.g. "inet 10.1.2.100/24". Intead of IPv4 address, there also exists a
 * few special syntaxes: "inet driver" means address and mask should be
 * read from the BSP. "inet dhcp" means address and mask should be received
 * from a DHCP server. The gateway might also be received from that server
 * (depends on the DHCP server configuration). "inet rarp" means address
 * and mask should be received from a RARP server. The default is set to
 * "driver" so that device name is retrieved from device boot parameters.
 * (4) "gateway" - specifies the default gateway used for IPv4, e.g.
 * "gateway 10.1.2.1". Only ONE default gateway can be specified.
 * "gateway driver" may be used to take the gateway from the boot parameters.
 * (5) "inet6" - specifies the interface IPv6 address and subnet, e.g.
 * "inet6 3ffe:1:2:3::4/64". The 'tentative' keyword can be inserted before
 * the address if the stack should do duplicate address detection on the
 * address before assigning it to the interface, e.g. 
 * "tentative 3ffe:1:2:3::4/64".
 * (6) "gateway6" - specifies the default gateway used for IPv6. Only ONE
 * default gateway can be specified.
 */
#ifdef INCLUDE_IPNET_IFCONFIG_1
#ifndef IFCONFIG_1
#define IFCONFIG_1 "ifname", "devname driver","inet driver","gateway driver", \
                   "inet6 3ffe:1:2:3::4/64"
#endif
#endif /* INCLUDE_IPNET_IFCONFIG_1 */

#ifdef INCLUDE_IPNET_IFCONFIG_2
#ifndef IFCONFIG_2
#define IFCONFIG_2                          NULL
#endif
#endif /* INCLUDE_IPNET_IFCONFIG_2 */

#ifdef INCLUDE_IPNET_IFCONFIG_3
#ifndef IFCONFIG_3
#define IFCONFIG_3                          NULL
#endif
#endif /* INCLUDE_IPNET_IFCONFIG_3 */

#ifdef INCLUDE_IPNET_IFCONFIG_4
#ifndef IFCONFIG_4
#define IFCONFIG_4                          NULL
#endif
#endif /* INCLUDE_IPNET_IFCONFIG_4 */

/*
 * IPNET packet pool configuration:
 * Defines the number of packets of each size that IPNET will add to its
 * packet pool in term of count-size-priority entries. The size given is
 * for IP and higher layer protocols, i.e. the link layer header is not
 * included = the MTU.
 * The size of packet pool is defined in bytes.
 * The priority level for the packet pool defines the lowest priority the
 * caller must have to be able to allocate packet out of a given pool.
 * Acceptable priority level is 0 (min) to 10 (max).
 * 
 */
#ifdef INCLUDE_IPNET_PACKET_POOL_1
#ifndef NUM_POOL_1
#define NUM_POOL_1                          10
#endif
#ifndef SIZE_POOL_1
#define SIZE_POOL_1                         64
#endif
#ifndef MIN_PRIO_POOL_1
#define MIN_PRIO_POOL_1                     0
#endif
#endif /* INCLUDE_IPNET_PACKET_POOL_1 */

#ifdef INCLUDE_IPNET_PACKET_POOL_2
#ifndef NUM_POOL_2
#define NUM_POOL_2                          40
#endif
#ifndef SIZE_POOL_2
#define SIZE_POOL_2                         128
#endif
#ifndef MIN_PRIO_POOL_2
#define MIN_PRIO_POOL_2                     0
#endif
#endif /* INCLUDE_IPNET_PACKET_POOL_2 */

#ifdef INCLUDE_IPNET_PACKET_POOL_3
#ifndef NUM_POOL_3
#define NUM_POOL_3                          25
#endif
#ifndef SIZE_POOL_3
#define SIZE_POOL_3                         256
#endif
#ifndef MIN_PRIO_POOL_3
#define MIN_PRIO_POOL_3                     0
#endif
#endif /* INCLUDE_IPNET_PACKET_POOL_3 */

#ifdef INCLUDE_IPNET_PACKET_POOL_4
#ifndef NUM_POOL_4
#define NUM_POOL_4                          50
#endif
#ifndef SIZE_POOL_4
#define SIZE_POOL_4                         512
#endif
#ifndef MIN_PRIO_POOL_4
#define MIN_PRIO_POOL_4                     0
#endif
#endif /* INCLUDE_IPNET_PACKET_POOL_4 */

#ifdef INCLUDE_IPNET_PACKET_POOL_5
#ifndef NUM_POOL_5
#define NUM_POOL_5                          90
#endif
#ifndef SIZE_POOL_5
#define SIZE_POOL_5                         1500
#endif
#ifndef MIN_PRIO_POOL_5
#define MIN_PRIO_POOL_5                     0
#endif
#endif /* INCLUDE_IPNET_PACKET_POOL_5 */

#ifdef INCLUDE_IPNET_PACKET_POOL_6
#ifndef NUM_POOL_6
#define NUM_POOL_6                          5
#endif
#ifndef SIZE_POOL_6
#define SIZE_POOL_6                         (10000)
#endif
#ifndef MIN_PRIO_POOL_6
#define MIN_PRIO_POOL_6                     0
#endif
#endif /* INCLUDE_IPNET_PACKET_POOL_6 */

#ifdef INCLUDE_IPNET_PACKET_POOL_7
#ifndef NUM_POOL_7
#define NUM_POOL_7                          1
#endif
#ifndef SIZE_POOL_7
#define SIZE_POOL_7                         (20000)
#endif
#ifndef MIN_PRIO_POOL_7
#define MIN_PRIO_POOL_7                     0
#endif
#endif /* INCLUDE_IPNET_PACKET_POOL_7 */

#ifdef INCLUDE_IPNET_PACKET_POOL_8
#ifndef NUM_POOL_8
#define NUM_POOL_8                          1
#endif
#ifndef SIZE_POOL_8
#define SIZE_POOL_8                         (30000)
#endif
#ifndef MIN_PRIO_POOL_8
#define MIN_PRIO_POOL_8                     0
#endif
#endif /* INCLUDE_IPNET_PACKET_POOL_8 */

#ifdef INCLUDE_IPNET_PACKET_POOL_9
#ifndef NUM_POOL_9
#define NUM_POOL_9                          1
#endif
#ifndef SIZE_POOL_9
#define SIZE_POOL_9                         (40000)
#endif
#ifndef MIN_PRIO_POOL_9
#define MIN_PRIO_POOL_9                     0
#endif
#endif /* INCLUDE_IPNET_PACKET_POOL_9 */

#ifdef INCLUDE_IPNET_PACKET_POOL_10
#ifndef NUM_POOL_10
#define NUM_POOL_10                          1
#endif
#ifndef SIZE_POOL_10
#define SIZE_POOL_10                         (50000)
#endif
#ifndef MIN_PRIO_POOL_10
#define MIN_PRIO_POOL_10                     0
#endif
#endif /* INCLUDE_IPNET_PACKET_POOL_10 */

#ifdef INCLUDE_IPNET_PACKET_POOL_11
#ifndef NUM_POOL_11
#define NUM_POOL_10                          1
#endif
#ifndef SIZE_POOL_11
#define SIZE_POOL_11                         (66000)
#endif
#ifndef MIN_PRIO_POOL_11
#define MIN_PRIO_POOL_11                     0
#endif
#endif /* INCLUDE_IPNET_PACKET_POOL_11 */

#ifdef INCLUDE_IPNET
#ifndef IPNET_MAX_INPUT_PKTS
#define IPNET_MAX_INPUT_PKTS                32
#endif
#ifndef IPNET_REASSEMBLY_TIMEOUT
#define IPNET_REASSEMBLY_TIMEOUT            15
#endif
#ifndef IPNET_CACHE_BUFSIZE
#define IPNET_CACHE_BUFSIZE                 0
#endif
#endif /* INCLUDE_IPNET */

#ifdef INCLUDE_IPCOM_USE_INET
#ifndef INET_MIN_MTU_SIZE
#define INET_MIN_MTU_SIZE                 68  /* Minimum mtu size */
#endif
#ifndef INET_BASE_HOP_LIMIT
#define INET_BASE_HOP_LIMIT               "64" /* Base hop limit */
#endif
#ifndef INET_MAX_PKTS_PENDING
#define INET_MAX_PKTS_PENDING             "3"  /* Maximum pending packets */
#endif
#ifndef INET_MAX_UNICAST_SOLICIT
#define INET_MAX_UNICAST_SOLICIT          "1"  /* Maximum unicast solicit */
#endif
#ifndef INET_MAX_MULTICAST_SOLICIT
#define INET_MAX_MULTICAST_SOLICIT        "9"  /* Maximum multicast solicit */
#endif
#ifndef INET_MAX_APP_SOLICIT
#define INET_MAX_APP_SOLICIT              "1"  /* Maximum application solicit */
#endif
#ifndef INET_BASE_REACHABLE_TIME
#define INET_BASE_REACHABLE_TIME          "3600" /* Base reachable time */
#endif
#ifndef INET_BASE_RETRANSMIT_TIME
#define INET_BASE_RETRANSMIT_TIME         "1"  /* Base retransmit time */
#endif
#ifndef INET_DELAY_FIRST_PROBE_TIME
#define INET_DELAY_FIRST_PROBE_TIME       "5"  /* Delay first probe time */
#endif
#ifndef INET_NBR_CACHE_TO_LIVE_TIME
#define INET_NBR_CACHE_TO_LIVE_TIME       "1200" /* Neighbor cache to live */
#endif
#ifndef INET_DST_CACHE_TO_LIVE_TIME
#define INET_DST_CACHE_TO_LIVE_TIME       "300"  /* Destination cache to live */
#endif
#ifndef INET_ICMP_RATE_LIMIT_BUCKET_SIZE
#define INET_ICMP_RATE_LIMIT_BUCKET_SIZE  "10" /* Icmp rate limit bucket size */
#endif
#ifndef INET_ICMP_RATE_LIMIT_INTERVAL
#define INET_ICMP_RATE_LIMIT_INTERVAL     "100"  /* Icmp rate limit interval */
#endif
#ifndef INET_ICMP_REDIRECT_SEND
#define INET_ICMP_REDIRECT_SEND           "2"  /* Icmp redirect send */
#endif
#ifndef INET_ICMP_REDIRECT_RECEIVE
#define INET_ICMP_REDIRECT_RECEIVE        "1"  /* Icmp redirect receive */
#endif
#ifndef INET_ICMP_IGNORE_ECHO_REQ
#define INET_ICMP_IGNORE_ECHO_REQ         "0"  /* Icmp ignore echo request */
#endif
#ifndef INET_ICMP_IGNORE_TIMESTAMP_REQ
#define INET_ICMP_IGNORE_TIMESTAMP_REQ  "0" /* Icmp ignore timestamp reqeust */
#endif
#ifndef INET_ICMP_SEND_TIME_EXCEEDED
#define INET_ICMP_SEND_TIME_EXCEEDED      "1"  /* Icmp send time exceeded */
#endif
#ifndef INET_ICMP_SEND_DST_UNREACHABLE
#define INET_ICMP_SEND_DST_UNREACHABLE    "1" /* Icmp send dest unreachable */
#endif

#ifdef INCLUDE_IPPROXYARP
#ifndef INET_ENABLE_PROXY_ARP
#define INET_ENABLE_PROXY_ARP             "0"  /* Enable network proxy ARP */
#endif
#ifndef INET_AUTO_PROXY_ARP
#define INET_AUTO_PROXY_ARP               "0"  /* Auto proxy ARP */
#endif

/* 
 * != 0 if the stack shall do proxy arping for network routes tagged with
 * the proxy arp flag. This parameter can be configure on a per-interface
 * basis in the form of "<ifparam>=<value>". Each pair of
 * "<ifparam>=<value>" is semicolon separated. E.g, "eth0=1".
 */
#ifndef INET_IFLIST_ENABLE_PROXY_ARP
#define INET_IFLIST_ENABLE_PROXY_ARP      ""
#endif

/* 
 * != 0 if the stack automatically shall tag all interface address network
 * routes as proxy arp. This parameter can be configure on a per-interface
 * basis in the form of "<ifparam>=<value>". Each pair of 
 * "<ifparam>=<value>" is semicolon separated. E.g, "eth0=1"
 */
#ifndef INET_IFLIST_AUTO_PROXY_ARP
#define INET_IFLIST_AUTO_PROXY_ARP        ""
#endif
#endif /* INCLUDE_IPPROXYARP */

#ifdef INCLUDE_IPAIP

/* 
 * INET_IPAIP_IFNAME_LIST - autoIP interface list
 * A space-separated list of interfaces that can configure a link-local
 * IPv4, e.g. "eth0 eth1".
 */
#ifndef INET_IPAIP_IFNAME_LIST
#define INET_IPAIP_IFNAME_LIST         ""
#endif

/*
 * The following configuration parameters support RFC3927, Dynamic
 * Configuration of IPv4 Link-Local Addresses. These configuration
 * parameters are only meaningful if IPNET is compiled with the
 * IPNET_USE_RFC3927 directive.
 */
#ifdef INCLUDE_IPAIP_GLOBAL_CONFIGS
#ifndef INET_IPAIP_PROBE_WAIT
#define INET_IPAIP_PROBE_WAIT         "1"
#endif
#ifndef INET_IPAIP_PROBE_NUM
#define INET_IPAIP_PROBE_NUM          "3"
#endif
#ifndef INET_IPAIP_PROBE_MIN
#define INET_IPAIP_PROBE_MIN          "1"
#endif
#ifndef INET_IPAIP_PROBE_MAX
#define INET_IPAIP_PROBE_MAX          "3"
#endif
#ifndef INET_IPAIP_ANNOUNCE_WAIT
#define INET_IPAIP_ANNOUNCE_WAIT      "2"
#endif
#ifndef INET_IPAIP_ANNOUNCE_NUM
#define INET_IPAIP_ANNOUNCE_NUM       "2"
#endif
#ifndef INET_IPAIP_ANNOUNCE_INTERVAL
#define INET_IPAIP_ANNOUNCE_INTERVAL  "2"
#endif
#ifndef INET_IPAIP_MAX_CONFLICTS
#define INET_IPAIP_MAX_CONFLICTS      "10"
#endif
#ifndef INET_IPAIP_RATE_LIMIT_INTERVAL
#define INET_IPAIP_RATE_LIMIT_INTERVAL  "60"
#endif
#ifndef INET_IPAIP_DEFEND_INTERVAL
#define INET_IPAIP_DEFEND_INTERVAL     "10"
#endif
#endif /* INCLUDE_IPAIP_GLOBAL_CONFIGS */

#ifdef INCLUDE_IPAIP_INTERFACE_CONFIGS
/*
 * INET_IPAIP_IFLIST_PROBE_WAIT - autoIP probe wait time
 * Time (in seconds) the host will wait before creating a link local IPv4
 * and start checking it for uniqueness. This parameter can be specified
 * for each interface specified in the INET_IPAIP_IFNAME_LIST in the form
 * of  "<ifparam>=<value>". Each pair of "<ifparam>=<value>" is semicolon
 * separated. E.g, "eth0=1".
 */                
#ifndef INET_IPAIP_IFLIST_PROBE_WAIT
#define INET_IPAIP_IFLIST_PROBE_WAIT    ""
#endif

/*
 * INET_IPAIP_IFLIST_PROBE_NUM - autoIP probe count
 * Number of times a link local address is checked for uniqness before being
 * assigned. This parameter can be specified for each interface specified in
 * the INET_IPAIP_IFNAME_LIST in the form of  "<ifparam>=<value>". Each pair
 * of "<ifparam>=<value>" is semicolon separated. E.g, "eth0=3".
 */
#ifndef INET_IPAIP_IFLIST_PROBE_NUM
#define INET_IPAIP_IFLIST_PROBE_NUM     ""
#endif

/*
 * INET_IPAIP_IFLIST_PROBE_MIN - autoIP min problem time
 * The minimum time (in seconds) where the next probe is sent. This parameter
 * can be specified for each interface specified in the INET_IPAIP_IFNAME_LIST
 * in the form of "<ifparam>=<value>". Each pair of "<ifparam>=<value>" is
 * semicolon separated. E.g, "eth0=1".
 */
#ifndef INET_IPAIP_IFLIST_PROBE_MIN
#define INET_IPAIP_IFLIST_PROBE_MIN     ""
#endif

/*
 * INET_IPAIP_IFLIST_PROBE_MAX - autoIP max problem time
 * The maximum time (in seconds) where the next probe is sent. This parameter
 * can be specified for each interface specified in the INET_IPAIP_IFNAME_LIST
 * in the form of "<ifparam>=<value>". Each pair of "<ifparam>=<value>" is
 * semicolon separated. E.g, "eth0=3".
 */
#ifndef INET_IPAIP_IFLIST_PROBE_MAX
#define INET_IPAIP_IFLIST_PROBE_MAX     ""
#endif

/*
 * INET_IPAIP_IFLIST_ANNOUNCE_WAIT - autoIP announce wait time
 * The time to wait (in seconds) before the assigned IPv4 address is
 * announced. This parameter can be specified for each interface specified in
 * the INET_IPAIP_IFNAME_LIST in the form of "<ifparam>=<value>". Each pair
 * of "<ifparam>=<value>" is semicolon separated. E.g, "eth0=2".
 */
#ifndef INET_IPAIP_IFLIST_ANNOUNCE_WAIT
#define INET_IPAIP_IFLIST_ANNOUNCE_WAIT ""
#endif

/*
 * INET_IPAIP_IFLIST_ANNOUNCE_NUM - autoIP number of announcements
 * The number of announcements to send. This parameter can be specified for
 * each interface specified in the INET_IPAIP_IFNAME_LIST in the form of
 * "<ifparam>=<value>". Each pair of "<ifparam>=<value>" is semicolon
 * separated. E.g, "eth0=2".
 */
#ifndef INET_IPAIP_IFLIST_ANNOUNCE_NUM
#define INET_IPAIP_IFLIST_ANNOUNCE_NUM  ""
#endif

/*
 * INET_IPAIP_IFLIST_ANNOUNCE_INTERVAL - autoIP announcements interval
 * The number of seconds between announcements. This parameter can be
 * specified for each interface specified in the INET_IPAIP_IFNAME_LIST in
 * the form of "<ifparam>=<value>". Each pair of "<ifparam>=<value>" is
 * semicolon separated. E.g, "eth0=2".
 */
#ifndef INET_IPAIP_IFLIST_ANNOUNCE_INTERVAL
#define INET_IPAIP_IFLIST_ANNOUNCE_INTERVAL ""
#endif

/*
 * INET_IPAIP_IFLIST_MAX_CONFLICTS - autoIP max conflicts
 * Max conflicts before rate limiting. This parameter can be specified for
 * each interface specified in the INET_IPAIP_IFNAME_LIST in the form of
 * "<ifparam>=<value>". Each pair of "<ifparam>=<value>" is semicolon
 * separated. E.g, "eth0=10".
 */
#ifndef INET_IPAIP_IFLIST_MAX_CONFLICTS
#define INET_IPAIP_IFLIST_MAX_CONFLICTS     ""
#endif

/*
 * INET_IPAIP_IFLIST_RATE_LIMIT_INTERVAL - autoIP rate limit interval
 * Delay between successive attempts. This parameter can be specified for
 * each interface specified in the INET_IPAIP_IFNAME_LIST in the form of
 * "<ifparam>=<value>". Each pair of "<ifparam>=<value>" is semicolon
 * separated. E.g, "eth0=60".
 */
#ifndef INET_IPAIP_IFLIST_RATE_LIMIT_INTERVAL
#define INET_IPAIP_IFLIST_RATE_LIMIT_INTERVAL   ""
#endif

/*
 * INET_IPAIP_IFLIST_DEFEND_INTERVAL - autoIP defensive interval
 * Minimum interval between defensive ARPs. This parameter can be specified
 * for each interface specified in the INET_IPAIP_IFNAME_LIST in the form
 * of "<ifparam>=<value>". Each pair of "<ifparam>=<value>" is semicolon
 * separated. E.g, "eth0=10".
 */
#ifndef INET_IPAIP_IFLIST_DEFEND_INTERVAL
#define INET_IPAIP_IFLIST_DEFEND_INTERVAL   ""
#endif
#endif /* INCLUDE_IPAIP_INTERFACE_CONFIGS */
#endif /* INCLUDE_IPAIP */

#endif /* INCLUDE_IPCOM_USE_INET */

#ifdef INCLUDE_IPCOM_USE_INET6
#ifndef INET6_MIN_MTU_SIZE
#define INET6_MIN_MTU_SIZE                1280  /* Minimum mtu size */
#endif
#ifndef INET6_BASE_HOP_LIMIT
#define INET6_BASE_HOP_LIMIT              "64"  /* Base hop limit */
#endif
#ifndef INET6_MAX_PKTS_PENDING
#define INET6_MAX_PKTS_PENDING            "3"  /* Maximum pending packets */
#endif
#ifndef INET6_MAX_UNICAST_SOLICIT
#define INET6_MAX_UNICAST_SOLICIT         "3"  /* Maximum unicast solicit */
#endif
#ifndef INET6_MAX_MULTICAST_SOLICIT
#define INET6_MAX_MULTICAST_SOLICIT       "0"  /* Maximum multicast solicit */
#endif
#ifndef INET6_MAX_APP_SOLICIT
#define INET6_MAX_APP_SOLICIT             "1"  /* Maximum application solicit */
#endif
#ifndef INET6_DAD_TRANSMITS
#define INET6_DAD_TRANSMITS               "1"  /* Number DAD transmits */
#endif
#ifndef INET6_BASE_REACHABLE_TIME
#define INET6_BASE_REACHABLE_TIME         "30" /* Base reachable time */
#endif
#ifndef INET6_BASE_RETRANSMIT_TIME
#define INET6_BASE_RETRANSMIT_TIME        "1"  /* Base retransmit time */
#endif
#ifndef INET6_DELAY_FIRST_PROBE_TIME
#define INET6_DELAY_FIRST_PROBE_TIME      "5"  /* Delay first probe time */
#endif
#ifndef INET6_NBR_CACHE_TO_LIVE_TIME
#define INET6_NBR_CACHE_TO_LIVE_TIME      "1200"  /* Neighbor cache to live */
#endif
#ifndef INET6_DST_CACHE_TO_LIVE_TIME
#define INET6_DST_CACHE_TO_LIVE_TIME      "300"  /* Destination cache to live */
#endif
#ifndef INET6_ROUTER_LIFETIME
#define INET6_ROUTER_LIFETIME             "1800"  /* Router lifetime */
#endif
#ifndef INET6_ICMP_RATE_LIMIT_BUCKET_SIZE
#define INET6_ICMP_RATE_LIMIT_BUCKET_SIZE "10" /* Icmp rate limit bucket size */
#endif
#ifndef INET6_ICMP_RATE_LIMIT_INTERVAL
#define INET6_ICMP_RATE_LIMIT_INTERVAL    "1000" /* Icmp rate limit interval */
#endif
#ifndef INET6_ICMP_REDIRECT_SEND
#define INET6_ICMP_REDIRECT_SEND          "2"   /* Icmp redirect send */
#endif
#ifndef INET6_ICMP_REDIRECT_RECEIVE
#define INET6_ICMP_REDIRECT_RECEIVE       "1"   /* Icmp redirect receive */
#endif
#ifndef INET6_ICMP_IGNORE_ECHO_REQ
#define INET6_ICMP_IGNORE_ECHO_REQ        "0"   /* Icmp ignore echo request */
#endif
#ifndef INET6_ICMP_SEND_TIME_EXCEEDED
#define INET6_ICMP_SEND_TIME_EXCEEDED     "1"   /* Icmp send time exceeded */
#endif
#ifndef INET6_ICMP_SEND_DST_UNREACHABLE
#define INET6_ICMP_SEND_DST_UNREACHABLE   "1"  /* Icmp send dest unreachable */
#endif
#ifndef INET6_ACCEPT_RTADV
#define INET6_ACCEPT_RTADV                "1"   /* Accept router adv message*/
#endif
#ifndef INET6_AUTO_CONFIG
#define INET6_AUTO_CONFIG                 "1"   /* Auto config IPv6 address */
#endif

#ifdef INCLUDE_IPNET6_AUTOCONF_PRIVACY_EXT

/*
 * The following configuration parameters support RFC3041, Privacy Extensions
 * for Stateless Address Autoconfiguration in IPv6. These configuration
 * parameters are only meaningful if IPNET is compiled with the 
 * IPNET_USE_RFC3401 directive.
 */

#ifdef INCLUDE_IPNET6_PRIVACY_GLOBAL_CONFIGS
#ifndef INET6_ENABLE_PRIVACY_EXTENSONS
#define INET6_ENABLE_PRIVACY_EXTENSONS    "1"      /* enabled */
#endif
#ifndef INET6_TEMP_VALID_LIFETIME
#define INET6_TEMP_VALID_LIFETIME         "604800"  /* 1 week */
#endif
#ifndef INET6_TEMP_PREFERRED_LIFETIME
#define INET6_TEMP_PREFERRED_LIFETIME     "86400"   /* 1 day */
#endif
#ifndef INET6_REGEN_ADVANCE
#define INET6_REGEN_ADVANCE               "30"   /* 30 seconds */
#endif
#endif /* INCLUDE_IPNET6_PRIVACY_GLOBAL_CONFIGS */

#ifdef INCLUDE_IPNET6_PRIVACY_INTERFACE_CONFIGS

/* 
 * Enable/disable privacy extensions. This parameter can be configure on
 * a per-interface basis in the form of "<ifparam>=<value>". Each pair
 * of "<ifparam>=<value>" is semicolon separated. E.g, "eth0=1". 
 */
#ifndef INET6_IFLIST_ENABLE_PRIVACY_EXTENSONS
#define INET6_IFLIST_ENABLE_PRIVACY_EXTENSONS       ""
#endif

/* 
 * Maximum lifetime, in seconds, of temporary addresses. default: 1 week
 * This parameter can be configure on a per-interface basis in the form
 * of "<ifparam>=<value>". Each pair of "<ifparam>=<value>" is semicolon
 * separated. E.g, "eth0=604800".
 */
#ifndef INET6_IFLIST_TEMP_VALID_LIFETIME
#define INET6_IFLIST_TEMP_VALID_LIFETIME        ""
#endif

/* 
 * Maximum preferred time, in seconds, of temporary addresses. 
 * default: 1 day 
 * This parameter can be configure on a per-interface basis in the form
 * of "<ifparam>=<value>". Each pair of "<ifparam>=<value>" is semicolon
 * separated. E.g, "eth0=86400".
 */
#ifndef INET6_IFLIST_TEMP_PREFERRED_LIFETIME
#define INET6_IFLIST_TEMP_PREFERRED_LIFETIME       ""
#endif

/*  
 * Number of seconds before the current temprary address goes non-preferred,
 * that a new address will be generated.
 * default: 5 seconds by RFC, but that is very tight so 30 seconds is
 * used instead.
 * This parameter can be configure on a per-interface basis in the form
 * of "<ifparam>=<value>". Each pair of "<ifparam>=<value>" is semicolon
 * separated. E.g, "eth0=30".
 */
#ifndef INET6_IFLIST_REGEN_ADVANCE
#define INET6_IFLIST_REGEN_ADVANCE      ""
#endif
#endif /* INCLUDE_IPNET6_PRIVACY_INTERFACE_CONFIGS */

#endif /* INCLUDE_IPNET6_AUTOCONF_PRIVACY_EXT */

#endif /* INCLUDE_IPCOM_USE_INET6 */

#ifdef INCLUDE_IPMCP
#ifndef MCP_UPSTREAM_IFNAME
#define MCP_UPSTREAM_IFNAME    NULL /* Upstream interface name */
#endif
#ifndef MCP_DOWNSTREAM_IFNAMES
#define MCP_DOWNSTREAM_IFNAMES NULL /* Downstream interface names */
#endif
#ifndef MCP_ROBUSTNESS_VAR
#define MCP_ROBUSTNESS_VAR     "2"  /* Multicast router robustness var */
#endif
#ifndef MCP_QUERY_INTERVAL
#define MCP_QUERY_INTERVAL     "125" /* Multicast router query interval, sec*/
#endif
#ifndef MCP_QUERY_RESP_INTERVAL
#define MCP_QUERY_RESP_INTERVAL "10000"  /* Multicast router query resp, msec*/
#endif
#ifndef MCP_LAST_LISTERNER_QUERY_INTERVAL
#define MCP_LAST_LISTERNER_QUERY_INTERVAL   "1000"  /* Multicast last listener query, msec */
#endif
#endif /* INCLUDE_IPMCP */

#ifdef INCLUDE_IPNET_USE_NAT
#ifndef IPNAT_ICMP_MAPPING_TIMEOUT
#define IPNAT_ICMP_MAPPING_TIMEOUT      "10"
#endif
#ifndef IPNAT_UDP_MAPPING_TIMEOUT
#define IPNAT_UDP_MAPPING_TIMEOUT       "60"
#endif
#ifndef IPNAT_TCP_MAPPING_TIMEOUT
#define IPNAT_TCP_MAPPING_TIMEOUT       "432000"
#endif
#ifndef IPNAT_OTHER_MAPPING_TIMEOUT
#define IPNAT_OTHER_MAPPING_TIMEOUT     "60"
#endif
#ifndef IPNAT_MAX_MAPPING
#define IPNAT_MAX_MAPPING               "1000"
#endif
#ifndef IPNAT_AUTOPORT_START_INTERVAL
#define IPNAT_AUTOPORT_START_INTERVAL   "29000"
#endif
#ifndef IPNAT_AUTOPORT_END_INTERVAL
#define IPNAT_AUTOPORT_END_INTERVAL     "29999"
#endif
#endif /* INCLUDE_IPNET_USE_NAT */

#ifdef INCLUDE_IPPPP

#ifndef PPP_DEFAULT_BAUDRATE
#define PPP_DEFAULT_BAUDRATE    "9600"    /* Initial baudrate */
#endif
#ifndef PPP_RUNMODE
#define PPP_RUNMODE    "start,passive,wincompat"   /* runmode */
#endif
#ifndef PPP_FLAGS
#define PPP_FLAGS     "proxyarp"    /* PPP flags */
#endif
#ifndef PPP_AUTH_MODES
#define PPP_AUTH_MODES  "noauth"     /* Authentication mode */
#endif
#ifndef PPP_LCP_MRU
#define PPP_LCP_MRU     "1500"     /* LCP maximum receive unit */
#endif
#ifndef PPP_LCP_MTU
#define PPP_LCP_MTU     "1500"     /* LCP maximum transmit unit */
#endif
#ifndef PPP_LCP_ECHO_REQ_INTERVAL
#define PPP_LCP_ECHO_REQ_INTERVAL   "60"    /* LCP echo requests interval */
#endif
#ifndef PPP_LCP_ECHO_REQ_FAILURE
#define PPP_LCP_ECHO_REQ_FAILURE    "5"    /* Echo failure */
#endif
#ifndef PPP_IPCP_IPV4_ADDRESS
#define PPP_IPCP_IPV4_ADDRESS       "0.0.0.0"     /* Local IPv4 address */
#endif
#ifndef PPP_IPCP_PEER_IPV4_ADDRESS
#define PPP_IPCP_PEER_IPV4_ADDRESS  "10.1.4.1"    /* Peer IPv4 address */
#endif
#ifndef PPP_IPCP_PEER_IPV4_POOL_NAME
#define PPP_IPCP_PEER_IPV4_POOL_NAME  "default" /* Peer IPv4 addr pool name */
#endif
#ifndef PPP_IPCP_PEER_IPV4_ADDRESS_POOL
#define PPP_IPCP_PEER_IPV4_ADDRESS_POOL "10.1.3.1-10.1.3.255"    /* Peer IPv4 address pool */
#endif
#ifndef PPP_USERNAME
#define PPP_USERNAME        "ppp"
#endif
#ifndef PPP_PASSWD
#define PPP_PASSWD          "kallekula"
#endif
#ifndef PPP_IPCP_PRIMARY_DNS_ADDRESS
#define PPP_IPCP_PRIMARY_DNS_ADDRESS    ""   /* Primary DNS Address */
#endif
#ifndef PPP_IPCP_SECONDARY_DNS_ADDRESS
#define PPP_IPCP_SECONDARY_DNS_ADDRESS  ""    /* Secondary DNS Address */
#endif
#ifndef PPP_IPCP_PRIMARY_NBNS_ADDRESS
#define PPP_IPCP_PRIMARY_NBNS_ADDRESS    ""  /* Primary NBNS Address */
#endif
#ifndef PPP_IPCP_SECONDARY_NBNS_ADDRESS
#define PPP_IPCP_SECONDARY_NBNS_ADDRESS   ""  /* Secondary NBNS Address */
#endif

/*
 * PPP_INSTALL_CALLBACK_HOOK - install user-defined PPP callback routine
 * Determine if the user-defined PPP action callback routine as specified
 * by the macro PPP_ACTION_CALLBACK_HOOK should be installed. If FALSE, the
 * PPP_ACTION_CALLBACK_HOOK configuration is not applicable.
 */
#ifndef PPP_INSTALL_CALLBACK_HOOK
#define PPP_INSTALL_CALLBACK_HOOK   FALSE
#endif

/*
 * PPP_ACTION_CALLBACK_HOOK - user-defined PPP action callback routine
 * User-defined PPP action callback routine. In order for PPP to function,
 * this callback function used by PPP must be implemented to handle
 * various PPP actions like init, startup, peer IPv4 address requests,
 * Peer Authentication login etc. If not specified, the internal PPP
 * action callback routine will be used. The PPP_INSTALL_CALLBACK_HOOK
 * must also be set to TRUE in order to install this callback hook.
 */
#ifndef PPP_ACTION_CALLBACK_HOOK
#define PPP_ACTION_CALLBACK_HOOK     NULL  /* PPP action callback routine */
#endif

#ifdef INCLUDE_IPPPP_INTERFACE_CONFIG

/*
 * PPP_IF_DEFAULT_BAUDRATE_LIST - Initial baudrate
 * Interface initial baudrate. This parameter is configurable on a
 * per-interface basis in the form of "<ifparam>=<value>", each separated
 * from the other by a semicolon. E.g, "ppp0=38400;ppp1=38400".
 */
#ifndef PPP_IF_DEFAULT_BAUDRATE_LIST
#define PPP_IF_DEFAULT_BAUDRATE_LIST    ""
#endif

/*
 * PPP_IF_RUNMODE_LIST - Interface runmode
 * Specifies one or more runmode options on an interface. Possible runmode
 * options are "start", "passive" and "exit". This parameter is configurable
 * on a per-interface basis in the form of "<ifparam>=<value>", each separated
 * from the other by a semicolon. E.g, "ppp0=start,passive".
 */
#ifndef PPP_IF_RUNMODE_LIST
#define PPP_IF_RUNMODE_LIST     ""
#endif

/*
 * PPP_IF_FLAGS_LIST - Interface PPP flags.
 * Specifies one or more flag behavioural options on an interface. Possible
 * flags options are "defaultroute" and "proxyarp". This parameter is
 * configurable on a per-interface basis in the form of "<ifparam>=<value>",
 * each separated from the other by a semicolon.
 * E.g, "ppp0=proxyarp;ppp1=proxyarp".
 */
#ifndef PPP_IF_FLAGS_LIST
#define PPP_IF_FLAGS_LIST       ""
#endif

/*
 * PPP_IF_AUTH_MODES_LIST - Authentication mode
 * Configures the required authentication modes. There are currently six
 * authentication options defined - "auth", "noauth", "pap", "refuse-pap",
 * "chap", and "refuse-chap". This parameter is configurable on a 
 * per-interface basis in the form of "<ifparam>=<value>", each separated
 * from the other by a semicolon. E.g. "ppp0=chap;ppp1=pap".
 */
#ifndef PPP_IF_AUTH_MODES_LIST
#define PPP_IF_AUTH_MODES_LIST  ""
#endif

/*
 * PPP_IF_LCP_MRU_LIST - LCP maximum receive unit
 * This parameter is configurable on a per-interface basis in the form of
 * "<ifparam>=<value>", each separated from the other by a semicolon.
 * E.g, "ppp0=1500;ppp1=1500".
 */
#ifndef PPP_IF_LCP_MRU_LIST
#define PPP_IF_LCP_MRU_LIST     ""
#endif

/*
 * PPP_IF_LCP_MTU_LIST - Interface LCP maximum transmit unit.
 * This parameter is configurable on a per-interface basis in the form of
 * "<ifparam>=<value>", each separated from the other by a semicolon.
 * E.g, "ppp0=1500;ppp1=1500".
 */
#ifndef PPP_IF_LCP_MTU_LIST
#define PPP_IF_LCP_MTU_LIST     ""
#endif

/*
 * PPP_IF_LCP_ECHO_REQ_INTERVAL_LIST - Interface LCP echo requests interval
 * LCP echo request output interval in seconds. 0 to disable. This parameter
 * is configurable on a per-interface basis in the form of 
 * "<ifparam>=<value>", each separated from the other by a semicolon. 
 * E.g, "ppp0=60;ppp1=60".
 */
#ifndef PPP_IF_LCP_ECHO_REQ_INTERVAL_LIST
#define PPP_IF_LCP_ECHO_REQ_INTERVAL_LIST   ""
#endif

/*
 * PPP_IF_LCP_ECHO_REQ_FAILURE_LIST - Echo failure
 * Max echo request failures before link termination. This parameter can be
 * configured on a per-interface basis in the form of "<ifparam>=<value>",
 * each separated from the other by a semicolon. E.g, "eth0=5"
 */
#ifndef PPP_IF_LCP_ECHO_REQ_FAILURE_LIST
#define PPP_IF_LCP_ECHO_REQ_FAILURE_LIST    ""
#endif

/*
 * PPP_IF_IPCP_IPV4_ADDRESS_LIST - Local IPv4 address list
 * Specifies the desired local IPv4 address. Setting local IPv4 address to 0
 * means asking the peer to suggest our IPv4 address. This parameter is
 * configurable on a per-interface basis in the form of "<ifparam>=<value>",
 * each separated from the other by a semicolon.
 * E.g, "ppp0=12.0.0.100;ppp1=12.1.0.100".
 */
#ifndef PPP_IF_IPCP_IPV4_ADDRESS_LIST
#define PPP_IF_IPCP_IPV4_ADDRESS_LIST       ""
#endif

/*
 * PPP_IF_IPCP_PEER_IPV4_ADDRESS_LIST - Peer IPv4 address list
 * Suggest peer IPv4 address if asked for. Note that the peer IPv4 address
 * will only get suggested to the peer if it ask for it by requesting IP
 * address 0. This parameter is configurable on a per-interface basis in the
 * form of "<ifparam>=<value>", each separated from the other by a semicolon.
 * E.g, "ppp0=10.1.4.1;ppp1=10.2.4.1".
 */
#ifndef PPP_IF_IPCP_PEER_IPV4_ADDRESS_LIST
#define PPP_IF_IPCP_PEER_IPV4_ADDRESS_LIST      ""
#endif

/*
 * PPP_IF_IPCP_PEER_IPV4_ADDRESS_POOL_LIST - Interface IPv4 peer address pool
 * list
 * Configure a IPv4 address pool used for the interface to supply peers with
 * IPv4 addresses when running as a PPP or PPPoE server. This parameter is 
 * configurable on a per-interface basis in the form of "<ifparam>=<value>", 
 * each separated from the other by a semicolon. 
 * E.g., "ppp0=10.1.3.1-10.1.3.255".
 */
#ifndef PPP_IF_IPCP_PEER_IPV4_ADDRESS_POOL_LIST
#define PPP_IF_IPCP_PEER_IPV4_ADDRESS_POOL_LIST     ""
#endif

/*
 * PPP_IF_IPCP_PRIMARY_DNS_ADDRESS_LIST - Primary DNS Address list
 * Configures the PPP client IPv4 primary DNS address. The configured IPv4
 * address will be given to the peer (the client) if the peer requests a
 * primary DNS address. This parameter is configurable on a per-interface
 * basis in the form of "<ifparam>=<value>", each separated from the other
 * by a semicolon. E.g. "ppp0=10.1.2.3;ppp1=10.2.2.3".
 */
#ifndef PPP_IF_IPCP_PRIMARY_DNS_ADDRESS_LIST
#define PPP_IF_IPCP_PRIMARY_DNS_ADDRESS_LIST    ""
#endif

/*
 * PPP_IF_IPCP_SECONDARY_DNS_ADDRESS_LIST - Secondary DNS Address list
 * Configures the PPP client IPv4 secondary DNS address. The configured IPv4
 * address will be given to the peer (the client) if the peer requests a
 * secondary DNS address. This parameter is configurable on a per-interface
 * basis in the form of "<ifparam>=<value>", each separated from the other by
 * a semicolon. E.g. "ppp0=195.42.198.5;ppp1=195.43.198.5".
 */
#ifndef PPP_IF_IPCP_SECONDARY_DNS_ADDRESS_LIST
#define PPP_IF_IPCP_SECONDARY_DNS_ADDRESS_LIST  ""
#endif

#endif /* INCLUDE_IPPPP_INTERFACE_CONFIG */

#ifdef INCLUDE_IPPPP_USERS_CONFIG
/*
 * PPP_USERS_LCP_ECHO_REQ_INTERVAL_LIST - Users LCP echo requests interval
 * LCP echo request output interval in seconds. 0 to disable. This parameter
 * is configurable on a per-user basis in the form of "<username>=<value>",
 * each separated from the other by a semicolon. E.g, "user1=60;user2=60".
 */
#ifndef PPP_USERS_LCP_ECHO_REQ_INTERVAL_LIST
#define PPP_USERS_LCP_ECHO_REQ_INTERVAL_LIST    ""
#endif

/*
 * PPP_USERS_IPCP_IPV4_ADDRESS_LIST - Users local IPv4 address list
 * Specifies the desired local IPv4 address. Setting local IPv4 address to
 * 0 means asking the peer to suggest our IPv4 address. This parameter is
 * configurable on a per-user basis in the form of "<username>=<value>".
 * Each pair of "<username>=<value>" is semicolon separated. 
 * E.g, "user1=12.0.0.100;user2=12.1.0.100".
 */
#ifndef PPP_USERS_IPCP_IPV4_ADDRESS_LIST
#define PPP_USERS_IPCP_IPV4_ADDRESS_LIST    ""
#endif

/*
 * PPP_USERS_IPCP_PEER_IPV4_ADDRESS_LIST - Users Peer IPv4 address list
 * Suggest peer IPv4 address if asked for. Note that the peer IPv4 address
 * will only get suggested to the peer if it ask for it by requesting IP
 * address 0. This parameter is configurable on a per-user basis in the form
 * of "<username>=<value>". Each pair of "<username>=<value>" is semicolon
 * separated. E.g, "user1=10.1.4.1;user2=10.2.4.1".
 */
#ifndef PPP_USERS_IPCP_PEER_IPV4_ADDRESS_LIST
#define PPP_USERS_IPCP_PEER_IPV4_ADDRESS_LIST   ""
#endif

/*
 * PPP_USERS_IPCP_PEER_IPV4_ADDRESS_POOL_LIST - Users IPv4 peer address pool
 * list
 * Configure a IPv4 address pool used for a user  to supply peers with IPv4
 * addresses when running as a PPP or PPPoE server. This parameter is
 * configurable on a per-user basis in the form of "<username>=<value>".
 * Each pair of "<username>=<value>" is semicolon separated.
 * E.g., "user1=10.1.3.1-10.1.3.255;user2=10.2.3.1-10.2.3.255".
 */
#ifndef PPP_USERS_IPCP_PEER_IPV4_ADDRESS_POOL_LIST
#define PPP_USERS_IPCP_PEER_IPV4_ADDRESS_POOL_LIST  ""
#endif

/*
 * PPP_USERS_IPCP_PRIMARY_DNS_ADDRESS_LIST - Users primary DNS Address list
 * Configures the PPP client IPv4 primary DNS address. The configured IPv4
 * address will be given to the peer (the client) if the peer requests a
 * primary DNS address. This parameter is configurable on a per-user basis
 * in the form of "<ifparam>=<value>", each separated from the other by a
 * semicolon. E.g. "user1=10.1.2.3;user2=10.2.2.3".
 */
#ifndef PPP_USERS_IPCP_PRIMARY_DNS_ADDRESS_LIST
#define PPP_USERS_IPCP_PRIMARY_DNS_ADDRESS_LIST     ""
#endif

/*
 * PPP_USERS_IPCP_SECONDARY_DNS_ADDRESS_LIST - Users secondary DNS Address list
 * Configures the PPP client IPv4 secondary DNS address. The configured IPv4
 * address will be given to the peer (the client) if the peer requests a
 * secondary DNS address. This parameter is configurable on a per-user basis
 * in the form of "<ifparam>=<value>", each separated from the other by a
 * semicolon. E.g. "user1=195.42.198.5;user2=195.43.198.5".
 */
#ifndef PPP_USERS_IPCP_SECONDARY_DNS_ADDRESS_LIST
#define PPP_USERS_IPCP_SECONDARY_DNS_ADDRESS_LIST   ""
#endif

#endif /* INCLUDE_IPPPP_USERS_CONFIG */

#endif /* INCLUDE_IPPPP */

#ifdef INCLUDE_IPPPPOE
#ifndef PPPOE_SERVER
#define PPPOE_SERVER    "0"    /* Enable(1) /disable(0) PPPoE server */
#endif
#ifndef PPPOE_SECRET_NAME
#define PPPOE_SECRET_NAME   "puttoanythinghere"    /* PPPoE Secret */
#endif
#ifndef PPPOE_MAX_SESSIONS
#define PPPOE_MAX_SESSIONS   "16"     /* Maximum PPPoE Sessions */
#endif
#ifndef PPPOE_MAX_ETH_SESSIONS
#define PPPOE_MAX_ETH_SESSIONS  "8"  /* Maximum PPPoE Sessions per Interface */
#endif
#ifndef PPPOE_AC_NAME
#define PPPOE_AC_NAME   "Windriver PPPoE Server"    /* Access Concentrator */
#endif
#ifndef PPPOE_SERVICE_NAME
#define PPPOE_SERVICE_NAME  "myservice"    /* Service name */
#endif

/*
 * PPPOE_IF_MAX_ETH_SESSIONS_LIST - PPPoE Sessions list per Interface
 * Maximum number of PPPoE sessions per Ethernet interface. Note that the
 * interface name used is the Ethernet interface name and not the pppoe 
 * interface name. This parameter is configurable on a per-interface basis
 * in the form of "<ifparam>=<value>", each separated from the other by
 * a semicolon. E.g. "eth0=8;eth1=8".
 */
#ifndef PPPOE_IF_MAX_ETH_SESSIONS_LIST
#define PPPOE_IF_MAX_ETH_SESSIONS_LIST  ""
#endif

/*
 * PPPOE_IF_SERVICE_NAME_LIST - Service name list
 * Service name to accept. Do not set the service name to accept any
 * service name. This parameter is configurable on a per-interface basis
 * in the form of "<ifparam>=<value>", each separated from the other by
 * a semicolon. E.g. "eth0=myservicename1;eth1=myservicename2".
 */
#ifndef PPPOE_IF_SERVICE_NAME_LIST
#define PPPOE_IF_SERVICE_NAME_LIST      ""
#endif

#endif /* INCLUDE_IPPPPOE */

#ifdef INCLUDE_IPTCP
#ifndef TCP_CONN_TIMEOUT
#define TCP_CONN_TIMEOUT        "30"   /* Connection timeout */
#endif
#ifndef TCP_MAX_RETRANSMITS
#define TCP_MAX_RETRANSMITS     "10"   /* Max retransmits */
#endif
#ifndef TCP_MSL
#define TCP_MSL                 "30"   /* Maximum segment lifetime */
#endif
#ifndef TCP_MAX_MSS
#define TCP_MAX_MSS             "0"    /* Maximum segment size */
#endif
#ifndef TCP_USE_TIMESTAMP
#define TCP_USE_TIMESTAMP       "0"    /* Use Timestamp */
#endif
#ifndef TCP_USE_RFC1122_URGENT_DATA
#define TCP_USE_RFC1122_URGENT_DATA   "0"   /* Use RFC1122 urgent data */
#endif
#ifndef TCP_SEGMENT_MULTIPLIER
#define TCP_SEGMENT_MULTIPLIER      "2"   /* Segment size multiplie */
#endif
#endif /* INCLUDE_IPTCP */

#ifdef INCLUDE_IPVRRPD

/*
 * VRRP_IFNAME_LIST - List of interfaces where this router should do VRRP
 * operation. The format is a space-separated list of interface names,
 * e.g. "eth0 eth1". Default: empty list (i.e. the daemon will just shutdown
 * if it is started).
 */
#ifndef VRRP_IFNAME_LIST
#define VRRP_IFNAME_LIST        ""      /* Interface list */
#endif

/*
 * VRRP_IFLIST_VRIDS - List of virtual ID's defined on this interface. Must
 *  be >= 1 and >= 255. The vrids list is specified using the format 
 *  <ifparam>=<value>. Each pair of <ifparam>=<value> is semicolon separated,
 *  e.g. "eth0=1;eth1=2".
 */
#ifndef VRRP_IFLIST_VRIDS
#define VRRP_IFLIST_VRIDS       NULL    /* vrids list */
#endif

/*
 * Configuration parameters associated with each interface/VRID list.
 */

/*
 * VRRP_IFLIST_VRIDS_PRIORITY - Priority value to be used in Master election
 * by this virtual router for each interface/VRID pair. 1-254 is available
 * for virtual routers backing up the master virtual router. 255 means this
 * router owns the address and starts as master. Default priority: 100. The
 * priority list is specified using the format <ifparam>=<value>. Each pair
 * of <ifparam>=<value> is semicolon separated, e.g. "eth0.1=100;eth1.2=255".
 */
#ifndef VRRP_IFLIST_VRIDS_PRIORITY
#define VRRP_IFLIST_VRIDS_PRIORITY      NULL
#endif

/*
 * VRRP_IFLIST_VRIDS_IPADDR - One or more address associated with this virtual
 * router for each interface/VRID pair. The IP Address list is specified using
 * the format <ifparam>=<value>. Each pair of <ifparam>=<value> is semicolon
 * separated, e.g. "eth0.1=10.130.2.254;eth1.2=10.130.3.254"
 */
#ifndef VRRP_IFLIST_VRIDS_IPADDR
#define VRRP_IFLIST_VRIDS_IPADDR        NULL
#endif

/*
 * VRRP_IFLIST_VRIDS_ADV_INTERVAL - Time interval (in seconds) between VRRP
 * advertisement messages for each interface/VRID pair. Default: 1. The
 * advertisement interval list is specified using the format
 * <ifparam>=<value>. Each pair of <ifparam>=<value> is semicolon separated,
 * e.g. "eth0.1=1;eth1.2=1".
 */
#ifndef VRRP_IFLIST_VRIDS_ADV_INTERVAL
#define VRRP_IFLIST_VRIDS_ADV_INTERVAL  NULL
#endif

/*
 *VRRP_IFLIST_VRIDS_PREEMPT_MODE - Controls whether a higher priority backup
 * router preempts a lower priority master for each interface/VRID pair.
 * Default: 1 (i.e. true). The preempt mode list is specified using the format
 * <ifparam>=<value>. Each pair of <ifparam>=<value> is semicolon separated,
 * e.g. "eth0.1=1;eth1.2=1".
 */
#ifndef VRRP_IFLIST_VRIDS_PREEMPT_MODE
#define VRRP_IFLIST_VRIDS_PREEMPT_MODE   NULL
#endif
#endif /* INCLUDE_IPVRRPD */

#ifdef INCLUDE_IPNET_SOCKET
#ifndef IPNET_SOCK_MAX
#define IPNET_SOCK_MAX                   1024   /* Max number of sockets */
#endif
#ifndef IPNET_SOCK_ANON_PORT_MIN
#define IPNET_SOCK_ANON_PORT_MIN         "49152"  /* AnonPortMin */
#endif
#ifndef IPNET_SOCK_ANON_PORT_MAX
#define IPNET_SOCK_ANON_PORT_MAX         "65535"  /* AnonPortMax */
#endif
#ifndef IPNET_SOCK_DEFAULT_SEND_BUFSIZE
#define IPNET_SOCK_DEFAULT_SEND_BUFSIZE  "10000"  /* Def sock send bufsize */
#endif
#ifndef IPNET_SOCK_DEFAULT_RECV_BUFSIZE
#define IPNET_SOCK_DEFAULT_RECV_BUFSIZE  "10000"  /* Def sock recv bufsize */
#endif
#ifndef IPNET_SOCK_ADDR_NOTIFY
#define IPNET_SOCK_ADDR_NOTIFY           "1"    /* Address Notify */
#endif
#endif /* INCLUDE_IPNET_SOCKET */

#ifdef INCLUDE_IPEAP
#ifndef EAP_IDENTITY
#define EAP_IDENTITY   "kalle"  /* EAP identity sent in response message */
#endif

#ifdef INCLUDE_IPEAP_TLS_COMMON

/*
 * EAP_INSTALL_CALLBACK_HOOK - install user-defined eap callback routines
 * Determine if the user-defined EAP callback routines as specified
 * by EAP_TLS_CERT_VERIFY_CALLBACK_HOOK, EAP_LOAD_TLS_KEY_CALLBACK_HOOK, 
 * EAP_LOAD_TLS_CERT_CALLBACK_HOOK and EAP_LOAD_CA_CERT_CALLBACK_HOOK 
 * should be installed. If FALSE, those callback routines are not applicable.
 */
#ifndef EAP_INSTALL_CALLBACK_HOOK
#define EAP_INSTALL_CALLBACK_HOOK       FALSE
#endif

/* 
 * EAP_TLS_CERT_VERIFY_CALLBACK_HOOK - TLS verification callback routine.
 * The EAP_INSTALL_CALLBACK_HOOK must also be set to TRUE in order to install
 * this callback hook.
 */
#ifndef EAP_TLS_CERT_VERIFY_CALLBACK_HOOK
#define EAP_TLS_CERT_VERIFY_CALLBACK_HOOK   NULL
#endif

/* 
 * EAP_LOAD_TLS_KEY_CALLBACK_HOOK - TLS key load callback routine.
 * The EAP_INSTALL_CALLBACK_HOOK must also be set to TRUE in order to install
 * this callback hook. 
 */
#ifndef EAP_LOAD_TLS_KEY_CALLBACK_HOOK
#define EAP_LOAD_TLS_KEY_CALLBACK_HOOK      NULL
#endif

/* 
 * EAP_LOAD_TLS_CERT_CALLBACK_HOOK - X509 certificate load callback routine.
 * The EAP_INSTALL_CALLBACK_HOOK must also be set to TRUE in order to install
 * this callback hook.
 */
#ifndef EAP_LOAD_TLS_CERT_CALLBACK_HOOK
#define EAP_LOAD_TLS_CERT_CALLBACK_HOOK     NULL
#endif

/* 
 * EAP_LOAD_CA_CERT_CALLBACK_HOOK - X509 CA certificates callback routine.
 * The EAP_INSTALL_CALLBACK_HOOK must also be set to TRUE in order to install
 * this callback hook.
 */
#ifndef EAP_LOAD_CA_CERT_CALLBACK_HOOK
#define EAP_LOAD_CA_CERT_CALLBACK_HOOK      NULL
#endif

#ifndef EAP_TLS_KEY_FILE
#define EAP_TLS_KEY_FILE    "default"   /* TLS key file */
#endif
#ifndef EAP_TLS_CERT_FILE
#define EAP_TLS_CERT_FILE   "default"   /* TLS certificate file */
#endif
#ifndef EAP_TLS_CA_CERT_FILE
#define EAP_TLS_CA_CERT_FILE  "default"     /* TLS CA certifcate file */
#endif

#endif /* INCLUDE_IPEAP_TLS_COMMON */

#ifdef INCLUDE_IPEAP_USE_EAP_MD5
#ifndef EAP_MD5
#define EAP_MD5     "0"         /* Enable EAP-MD5 */
#endif
#ifndef EAP_MD5_SECRET
#define EAP_MD5_SECRET  "this_should_be_secret"     /* EAP-MD5 secret */
#endif  
#endif /* INCLUDE_IPEAP_USE_EAP_MD5 */

#ifdef INCLUDE_IPEAP_USE_EAP_MSCHAPV2
#ifndef EAP_MSCHAPV2
#define EAP_MSCHAPV2    "0"     /* Enable EAP-MSCHAPv2 */
#endif
#ifndef EAP_MSCHAPV2_USER_ID
#define EAP_MSCHAPV2_USER_ID   "kalle"   /* EAP-MSCHAPV2 user id */
#endif
#ifndef EAP_MSCHAPV2_PASSWD
#define EAP_MSCHAPV2_PASSWD "anka"      /* EAP-MSCHAPV2 password */
#endif
#endif /* INCLUDE_IPEAP_USE_EAP_MSCHAPV2 */

#ifdef INCLUDE_IPEAP_USE_EAP_PEAP
#ifndef EAP_PEAP
#define EAP_PEAP        "0"     /* Enable PEAP */
#endif
#endif /* INCLUDE_IPEAP_USE_EAP_PEAP */

#ifdef INCLUDE_IPEAP_USE_EAP_SIM
#ifndef EAP_SIM
#define EAP_SIM         "0"     /* Enable EAP-SIM */
#endif
#ifndef EAP_SIM_TEST
#define EAP_SIM_TEST    "0"     /* Enable EAP-SIM test */
#endif
#endif /* INCLUDE_IPEAP_USE_EAP_SIM */

#ifdef INCLUDE_IPEAP_USE_EAP_TLS
#ifndef EAP_TLS
#define EAP_TLS     "0"     /* Enable EAP-TLS */
#endif
#endif /* INCLUDE_IPEAP_USE_EAP_TLS */

#ifdef INCLUDE_IPEAP_USE_EAP_TTLS
#ifndef EAP_TTLS
#define EAP_TTLS    "0"     /* Enable EAP-TTLS */
#endif
#ifndef EAP_TTLS_PHASE2
#define EAP_TTLS_PHASE2 "mschapv2"  /* EAP-TTLS Phase 2 auth type */
#endif
#ifndef EAP_TTLS_USER_ID
#define EAP_TTLS_USER_ID    "user"  /* EAP-TTLS user id */
#endif
#ifndef EAP_TTLS_PASSWD
#define EAP_TTLS_PASSWD     "password"    /* EAP-TTLS password */
#endif
#endif /* INCLUDE_IPEAP_USE_EAP_TTLS */

#endif /* INCLUDE_IPEAP */

#ifdef INCLUDE_IPMPLS
/*
 * IPMPLS_FWCONF_SYSVAR - MPLS network pre-configuration
 * MPLS network pre-configuration array is used to statically configure 
 * MPLS forwarding with an initial setup when IPMPLS is initialized. Each 
 * system variable in this array must be named according to the rule 
 * ipmpls.fwconf.<index>, where <index> is a unique line indicator. The value
 * of each system variable must be composed of the IPMPLS shell command 
 * mplsctl together with the flags/paramters that normally are invoked from 
 * the IPCOM shell. The commands are executed in the order they are defined.
 * It is important that the "silent" flag "-s" is included on each statement. 
 * See the ipmpls_fwdconf_sysvar Ipcom_sysvar array in ipmpls_config.c 
 * configlette for the example usage.
 */
#ifndef IPMPLS_FWCONF_SYSVAR
#define IPMPLS_FWCONF_SYSVAR    NULL
#endif
#endif /* INCLUDE_IPMPLS */

/******************************************************************************/
/*                                                                            */
/*                    IPv4/v6 MOBILITY CONFIGURATION                          */
/*                                                                            */
/******************************************************************************/
#ifdef INCLUDE_IPMIPFA

/* MIPFA_IFNAME_LIST - Specifies the network interface which should be
 * managed by the mobile IP foreign agent implementation. This is a
 * space-separated list of one or more interfaces, i.e. "eth0"
 */
#ifndef MIPFA_IFNAME_LIST
#define MIPFA_IFNAME_LIST    ""
#endif

/*
 * MIPFA_REG_LIFETIME - Specifies how long a mobile node's binding is
 * kept active before it is deleted, unless the mobile node have reregistered
 * before the time is up. Value given in seconds.
 */ 
#ifndef MIPFA_REG_LIFETIME
#define MIPFA_REG_LIFETIME    "10"
#endif

/*
 * MIPFA_HA_IF_SPIS_LIST - Foreign - Home authentication.
 * List of SPIs to use between FA and HA. This parameter can be specified
 * for each HA in the form of "<HA address>=<SPI>", each separated from the
 * other by a semicolon. Set "<HA address>" to "any" to enable foreign-home
 * authentication to all home agents. If the HA IP address is not found and
 * no "any" entry exists, no foreign-home authentication will be used. For
 * example, setting "10.1.2.42=1002" enables foreign-home authentication
 * using SPI 1002 between this FA and HA using IP Address 10.1.2.42. Setting
 * "any=1002" enables foreign-home authentication to all home agents using
 * SPI 1002.
 */
#ifndef MIPFA_HA_IF_SPIS_LIST
#define MIPFA_HA_IF_SPIS_LIST   ""
#endif

/*
 * MIPFA_FA_IF_SPIS_LIST Foreign - Foreign authentication.
 * List of SPIs to use between FA and FA. This parameter can be specified
 * for each FA in the form of "<FA address>=<SPI>", each separated from the
 * other by a semicolon. Set "<FA address>" to "any" to enable foreign-foreign
 * authentication to all foreign agents. If the FA IP address is not found
 * and no "any" entry exists, no foreign-foreign authentication will be used.
 * For example, setting "10.1.2.42=1002" enable foreign-foreign authentication
 * using SPI 1002 between this FA and FA using IP Address 10.1.2.42. Setting
 * "any=1002" enables foreign-foreign authentication to all foreign
 * agents using SPI 1002.
 */
#ifndef MIPFA_FA_IF_SPIS_LIST
#define MIPFA_FA_IF_SPIS_LIST   ""
#endif

/*
 * MIPFA_SPI_LIST - Interface default Security Parameter Index (SPI) list.
 * Foreign agents can configure multiple SPIs for use with many different
 * home agents. Each SPI have exactly one secret. The secret length may be
 * up to 16 bytes and need not be printable. The SPI is specifed in the form
 * of "<SPI>=<value>", each separated from the other by a semicolon. 
 * E.g. "1000=test0", "1000=test0;1001=test1".
 */
#ifndef MIPFA_SPI_LIST
#define MIPFA_SPI_LIST        ""
#endif

/*
 * MIPFA_CVSE_LIST - Critical vendor specific extensions (CVSE)
 * Specifies the critical vendor specific extensions (CVSE) that should be
 * attached to communication with mobile nodes and home agents according to
 * RFC 3115. Note that any node encountering a NVSE that it does not
 * understand will skip that particular extension. A NVSE is of the form
 * "<vendor_id>.<vendor_type>=<vendor_data>" where vendor_id is the SMI
 * number of the company, vendor_type is the vendor administrated vendor
 * subtype and vendor_data is the vendor specific opaque information. Each
 * pair of cvse is semicolon separated. Values are either plain text or a
 * hexadecimal number prepended with '0x'. That is, "vendor value", and
 * "0xdeadbeef" are both valid values. E.g. "0.1=Testing",
 * "0.1=Testing;0.2=Testing2".
 */
#ifndef MIPFA_CVSE_LIST
#define MIPFA_CVSE_LIST     ""
#endif

/*
 * MIPFA_NVSE_LIST - Normal vendor specific extensions (NVSE)
 * Specifies the normal vendor specific extensions (NVSE) that should be
 * attached to the communications as according to RFC 3115. A NVSE extension
 * that is not understood by transit nodes or the HA are silently discarded.
 * The NVSE is specified in the form of
 * "<vendor_id>.<vendor_type>=<vendor_data>", where vendor_id is the SMI
 * number, vendor_type is the vendor administrated vendor subtype and
 * vendor_data is the vendor specific opaque information. Each pair of nvse
 * is semicolon separated. The value is specified either as plain text, or
 * as a hexadecimal number, prepended with '0x'. That is, "vendor value",
 * and "0xdeadbeef" are both valid values. E.g. "0.1=Testing",
 * "0.1=Testing;0.2=Testing2".
 */
#ifndef MIPFA_NVSE_LIST
#define MIPFA_NVSE_LIST     ""
#endif

/*
 * MIPFA_IF_ADDRESS_LIST - Interface FA address
 * Interface FA address in the form of "<ifname>=<address>", each separated
 * from the other by a semicolon.  Set to "0.0.0.0" to read IPv4 address at
 * boot. If set to "255.255.255.255", no RtAdv will be sent but registration
 * replies will be processed if matching a sent request. E.g., "eth0=0.0.0.0".
 */
#ifndef MIPFA_IF_ADDRESS_LIST
#define MIPFA_IF_ADDRESS_LIST  ""
#endif

/*
 * MIPFA_IF_ADV_INTERVAL_LIST - Interface Router Advertisement Interval
 * The interval (in seconds) at which the agent will send router
 * advertisements on this interface specified in the form of
 * "<ifname>=<value>", each separated from the other by a semicolon. If set
 * to 0, the agent will not send any router advertisements, also, none of
 * the otheradv_ sysvars need to be set. The default value of 10 is good for
 * testing purposes, whereas the RFC suggests about a value of a third of
 * the ICMP router advertisement's lifetime. This value can be set to 0 or
 * high if mobile nodes have other means of detecting entry to new subnets,
 * such as wireless AP indication. E.g. "eth0=10".
 */
#ifndef MIPFA_IF_ADV_INTERVAL_LIST
#define MIPFA_IF_ADV_INTERVAL_LIST     ""
#endif

/*
 * MIPFA_IF_ADV_ADDRESS_LIST - RtAdv destination IP address
 * Specifies the destination address of outgoing router advertisement
 * messages from the agent. This parameter is configurable on a per-interface
 * basis in the form of "<ifname>=<value>", each separated from the other by
 * a semicolon. E.g. "eth0=224.0.0.11".
 */
#ifndef MIPFA_IF_ADV_ADDRESS_LIST
#define MIPFA_IF_ADV_ADDRESS_LIST      ""
#endif

/*
 * MIPFA_IF_ADV_LIFETIME_LIST - Interface Router Advertisement Lifetime
 * Specifies the lifetime (in seconds) of advertisement messages from the
 * agent. This parameter is configurable on a per-interface basis in the form
 * of "<ifname>=<value>", each separated from the other by a semicolon. 
 * E.g. "eth0=300".
 */
#ifndef MIPFA_IF_ADV_LIFETIME_LIST
#define MIPFA_IF_ADV_LIFETIME_LIST     ""
#endif

/*
 * MIPFA_IF_CHALLENGE_LIST - Interface Challenge
 * Enable(1)/disable(0) RFC3012 challenges for an interface. This parameter
 * is configurable on a per-interface basis in the form of "<ifname>=<value>".
 * Each pair of "<ifname>=<value>" is semicolon separated. E.g. "eth0=1".
 */
#ifndef MIPFA_IF_CHALLENGE_LIST
#define MIPFA_IF_CHALLENGE_LIST        ""
#endif

/*
 * MIPFA_IF_MN_AUTH_ENABLED_LIST - Interface Mobile Node authentication
 * Enable(1)/Disable(0) forcing of mobile nodes to use Mobile-Foreign 
 * authentication. This parameter is configurable on a per-interface basis in
 * the form of "<ifname>=<value>", each separated from the other by a 
 * semicolon. E.g. "eth0=0".
 */
#ifndef MIPFA_IF_MN_AUTH_ENABLED_LIST
#define MIPFA_IF_MN_AUTH_ENABLED_LIST      ""
#endif

/*
 * MIPFA_IF_LLH_ENABLED_LIST - Enable pre-registration
 * Enables(1) or Disables(0) PRE-REGISTRATION Low Latency Handoffs for an
 * interface. This parameter is configurable on a per-interface basis in the
 * form of "<ifname>=<value>". Each pair of "<ifname>=<value>" is semicolon
 * separated. E.g. "eth0=1". 
 */
#ifndef MIPFA_IF_LLH_PRE_ENABLED_LIST
#define MIPFA_IF_LLH_PRE_ENABLED_LIST          ""
#endif

/*
 * MIPFA_IF_LLH_POST_ENABLED_LIST - Enable post-registration
 * Enables(1) or Disables(0) POST-REGISTRATION Low Latency Handoffs for an
 * interface. This parameter is configurable on a per-interface basis in 
 * the form of "<ifname>=<value>", each separated from the other by a 
 * semicolon. E.g. "eth0=0".
 */
#ifndef MIPFA_IF_LLH_POST_ENABLED_LIST
#define MIPFA_IF_LLH_POST_ENABLED_LIST         ""
#endif

/*
 * MIPFA_IF_LLH_FA_AUTH_ENABLED_LIST - post-registration FF Authentication
 * Enables(1) or Disables(0) POST-REGISTRATION Foreign-Foreign authentication
 * during Low Latency Handoffs for an interface. This parameter can be 
 * configured on a per-interface basis in the form of "<ifname>=<value>",
 * each separated from the other by a semicolon. E.g. "eth0=1".
 *
 */
#ifndef MIPFA_IF_LLH_FA_AUTH_ENABLED_LIST
#define MIPFA_IF_LLH_FA_AUTH_ENABLED_LIST      ""
#endif

/*
 * MIPFA_IF_LLH_BET_LIFETIME_LIST - BET registration lifetime
 * Specifies the default value of the BET lifetime when POST registration
 * is in effect. The BET lifetime is the value the FA will try to negotiate
 * for the BET tunnel, and it is also used as the default value when the
 * tunnel lifetime is being extended. This parameter is configurable on a
 * per-interface basis in the form of "<ifname>=<value>", each separated from
 * the other by a semicolon. E.g. "eth0=5".
 *
 */
#ifndef MIPFA_IF_LLH_BET_LIFETIME_LIST
#define MIPFA_IF_LLH_BET_LIFETIME_LIST         ""
#endif

/*
 * MIPFA_IF_LLH_NBR_SOL_INTERVAL_LIST - Neighbor Solicitation Interval
 * Specifies neighbor solicitation interval for Low-Latency pre-registration
 * handoffs. This parameter is configurable on a per-interface basis in
 * the form of "<ifname>=<value>", each separated from the other by a 
 * semicolon. E.g. "eth0=10". 
 */
#ifndef MIPFA_IF_LLH_NBR_SOL_INTERVAL_LIST
#define MIPFA_IF_LLH_NBR_SOL_INTERVAL_LIST     ""
#endif

/*
 * MIPFA_IF_LLH_NBR_ADVERT_TIMEOUT_LIST - Neighbor Advertisement Timeout
 * Specifies neighbor advertisement timeout for Low-Latency Pre-registration
 * handoffs. This timeout determines the interval for which a proxied router
 * advertisement is considered valid. This parameter is configurable on 
 * a per-interface basis in the form of "<ifname>=<value>", each separated
 * from the other by a semicolon. E.g. "eth0=20". 
 *
 */
#ifndef MIPFA_IF_LLH_NBR_ADVERT_TIMEOUT_LIST
#define MIPFA_IF_LLH_NBR_ADVERT_TIMEOUT_LIST   ""
#endif

/*
 * MIPFA_IF_LLH_NBR_ADDRESS_ANY_LIST - Wildcard Neighbor configuration
 * These wildcard keys configure known LLH entities (neighboring FA's)
 * that we may communicate with when it comes to Low-Latency Handoffs.
 * The value is the solicitation interval/advertisement timeout to use for
 * any node which responds to our solicitations may be used. If
 * unspecified, the default values will be used. This parameter can be
 * configured on a per-interface basis in the form of
 * "<ifname>=<sol interface/adv interface>", each separated from the other
 * by a semicolon. E.g. "eth0=20/30"
 */
#ifndef MIPFA_IF_LLH_NBR_ADDRESS_ANY_LIST
#define MIPFA_IF_LLH_NBR_ADDRESS_ANY_LIST      ""
#endif

/*
 * MIPFA_IF_NAT_T_ENABLED_LIST - Enable NAT traversal
 * Enables or disables NAT Traversal according to RFC3519. This parameter can
 * be configured on a per-interface basis in the form of "<ifname>=<value>",
 * each separated from the other by a semicolon. E.g. "eth0=1".
 */
#ifndef MIPFA_IF_NAT_T_ENABLED_LIST
#define MIPFA_IF_NAT_T_ENABLED_LIST    ""
#endif

/*
 * MIPFA_IF_FORCED_NAT_T_ENABLED_LIST - Enable forced NAT traversal
 * Enables forcing of NAT traversal (RFC 3519), regardless what the home
 * agent NAT detection decides. This parameter is configurable on a 
 * per-interface basis in the form of "<ifname>=<value>", each separated 
 * from the other by a semicolon. E.g. "eth0=0"
 */
#ifndef MIPFA_IF_FORCED_NAT_T_ENABLED_LIST
#define MIPFA_IF_FORCED_NAT_T_ENABLED_LIST     ""
#endif

/*
 * MIPFA_IF_NAT_T_KEEPALIVE_LIST - NAT traversal keepalive
 * Decides the keepalive timeout (in seconds) to use for NAT Traversal
 * ICMP keepalive messages in order to maintain a NAT device's UDP port
 * mapping. This parameter is configurable on a per-interface basis in the 
 * form of "<ifname>=<timeout>", each separated from the other by a semicolon. 
 * E.g. "eth0=120"
 */
#ifndef MIPFA_IF_NAT_T_KEEPALIVE_LIST
#define MIPFA_IF_NAT_T_KEEPALIVE_LIST      ""
#endif

/*
 * MIPFA_IF_NAT_T_TUNNEL_TYPE_LIST - NAT-T forced tunnel type
 * Allows for specifying a different tunnel type if NAT traversal is used.
 * Ordinarily, the tunnel type specified by the mobile node is used, but
 * the foreign agent may override it for NAT Traversal tunneling, if for
 * example only IPIP is specified to work. Allowed values are 'min', 'gre'
 * and 'ipip'. If not specified or empty, the mobile nodes configuration
 * will take presedence. This parameter is  configured on a per-interface
 * basis in the form of "<ifname>=<type>", each separated from the other by 
 * a semicolon. E.g. "eth0=ipip"
 */
#ifndef MIPFA_IF_NAT_T_TUNNEL_TYPE_LIST
#define MIPFA_IF_NAT_T_TUNNEL_TYPE_LIST    ""
#endif

/*
 * MIPFA_IF_TUNNEL_REVERSE_LIST - Interface Reverse Tunneling
 * Disabled(0)/Optional(1)/Required(2) usage of reverse tunneling. If it's
 * disabled, any node trying to request reverse tunneling will be disallowed.
 * If optional, the node itself will be allowed to decide. If required the
 * reverse tunneling bit must be set. This parameter is configurable on a
 * per-interface basis in the form of "<ifname>=<value>", each separated 
 * from the other by a semicolon. E.g. "eth0=1".
 */
#ifndef MIPFA_IF_TUNNEL_REVERSE_LIST
#define MIPFA_IF_TUNNEL_REVERSE_LIST       ""
#endif

/*
 * MIPFA_IF_GRE_TUNNEL_ENABLED_LIST - Interface GRE tunneling
 * Enable(1)/Disable(0) usage of GRE tunneling for an interface. This
 * parameter is configurable on a per-interface basis in the form of
 * "<ifname>=<value>", each separated from the other by a semicolon.
 * E.g. "eth0=1".
 */
#ifndef MIPFA_IF_GRE_TUNNEL_ENABLED_LIST
#define MIPFA_IF_GRE_TUNNEL_ENABLED_LIST   ""
#endif

/*
 * MIPFA_IF_MINENC_TUNNEL_ENABLED_LIST - Interface MIN Encap tunneling
 * Enable(1)/Disable(0) usage of Minimal encapsulation tunneling for an
 * interface. This parameter is configurable on a per-interface basis in the
 * form of "<ifname>=<value>", each separated from the other by a semicolon.
 * E.g. "eth0=1".
 */
#ifndef MIPFA_IF_MINENC_TUNNEL_ENABLED_LIST
#define MIPFA_IF_MINENC_TUNNEL_ENABLED_LIST    ""
#endif

#endif /* INCLUDE_IPMIPFA */

#ifdef INCLUDE_IPMIPHA

/*
 * MIPHA_IFNAME_LIST - Home agent Interface list
 * Space-separated list of network interface which should be managed by the
 * mobile IP home agent implementation. E.g., "eth0 eth1"
 */
#ifndef MIPHA_IFNAME_LIST
#define MIPHA_IFNAME_LIST           ""
#endif
#ifndef MIPHA_REG_LIFETIME
#define MIPHA_REG_LIFETIME          "30"    /* Registration lifetime */
#endif
#ifndef MIPHA_REPLAY_PROTECTION
#define MIPHA_REPLAY_PROTECTION     "7"     /* Replay protection */
#endif
#ifndef MIPHA_HA_NAI
#define MIPHA_HA_NAI                ""      /* AAH Server Network access id */
#endif
#ifndef MIPHA_AAAH_NAI_1
#define MIPHA_AAAH_NAI_1            ""      /* AAH Server Network access id */
#endif
#ifndef MIPHA_AAAH_NAI_2
#define MIPHA_AAAH_NAI_2            ""      /* */
#endif

/*
 * MIPHA_CVSE_LIST - Critical vendor specific extensions
 * Specifies the critical vendor specific extensions (CVSE) that should be
 * attached to all replies sent by this home agent as according to RFC 3115.
 * Note that any transit node or mobile node receiving a CVSE that it does
 * not understand, will discard the entire reply. A CVSE is specified in the
 * form of "<vendor_id>.<vendor_type>=<vendor_data>" where vendor_id is the
 * SMI number of the vendor, vendor_type is the vendor administrated vendor
 * subtype and vendor_data is the vendor specific opaque information. Each
 * pair of cvse is semicolon separated. The value is specified either as plain
 * text, or as a hexadecimal number, prepended with '0x'. That is, "vendor
 * value", and "0xdeadbeef" are both valid values. E.g. "0.1=Testing",
 * "0.1=Testing;0.2=Testing2".
 */ 
#ifndef MIPHA_CVSE_LIST
#define MIPHA_CVSE_LIST         ""
#endif

/*
 * MIPHA_NVSE_LIST - Normal vendor specific extensions
 * Specifies the normal vendor specific extensions (NVSE) that should be
 * should be attached to all replies sent by this home agent as according
 * to RFC 3115. Note that any transit node or mobile node receiving a NVSE
 * that it does not understand, will skip the extension. A NVSE is specified
 * in the form of "<vendor_id>.<vendor_type>=<vendor_data>" where vendor_id
 * is the SMI number of the vendor, vendor_type is the vendor administrated
 * vendor and vendor_data is the vendor specific opaque information. Each pair
 * of cvse is semicolon separated. The value is specified either as plain
 * text, or as a hexadecimal number, prepended with '0x'. That is, "vendor
 * value", and "0xdeadbeef" are both valid values. E.g. "0.1=Testing",
 * "0.1=Testing;0.2=Testing2".
 */
#ifndef MIPHA_NVSE_LIST
#define MIPHA_NVSE_LIST         ""
#endif

/*
 * MIPHA_SPI_LIST - Security Parameter Index (SPI) list.
 * Home agents can configure multiple SPIs for use with many different home
 * agents. Each SPI have exactly one secret. The secret length may be up to
 * 16 bytes and need not be printable. The SPI is specifed in the form of
 * "<SPI>=<value>", each separated from the other by a semicolon. 
 * E.g. "1000=test0", "1000=test0;1001=test1".
 */
#ifndef MIPHA_SPI_LIST
#define MIPHA_SPI_LIST          ""
#endif

/*
 * MIPHA_IF_HOME_ADDRESS_LIST - Home agent interface address
 * The IPv4 addresses of the home agent in the form of "<ifname>=<address>",
 * each separated from the other by a semicolon. This address must be
 * reachable from all foreign nets the mobile node wishes to visit. If this
 * value is set "0.0.0.0" for an interface, the interface will not act home
 * agent on the subnet it is configured for, but only sniff incoming packets
 * for possible forwarding to proxied mobile nodes. This is typically useful
 * if the home agent is being run on a router. E.g., "eth0=0.0.0.0".
 */
#ifndef MIPHA_IF_HOME_ADDRESS_LIST
#define MIPHA_IF_HOME_ADDRESS_LIST  ""
#endif

/*
 * MIPHA_IF_HOME_MASK_LIST - Home agent netmask
 * The home agent's IPv4 netmask(s) in the form of "<ifname>=<mask>", each
 * separated from the other by a semicolon. E.g. "eth0=255.255.0.0".
 */
#ifndef MIPHA_IF_HOME_MASK_LIST
#define MIPHA_IF_HOME_MASK_LIST     ""
#endif

/*
 * MIPHA_IF_AUTH_SPI_LIST - Interface default SPI list.
 * Default Security Parameter Index (SPI) for the interface which will be used
 * if a mobile node has not been given a unique SPI to work with. The SPI is
 * specifed in the form of "<SPI>=<value>", each separated from the other by
 * a semicolon. The SPI must be greater than 255. E.g. "eth0=1000",
 * "eth=1000;eth1=1001".
 */
#ifndef MIPHA_IF_AUTH_SPI_LIST
#define MIPHA_IF_AUTH_SPI_LIST      ""
#endif

/*
 * MIPHA_IF_FA_AUTH_LIST - Foreign agent security requirements
 * Specifies if Foreign Agent to Home Agent security association is required
 * or not. Enabled(1), Disabled(0). This parameter is configurable on a
 * per-interface basis in the form of "<ifname>=<value>", each separated from 
 * the other by a semicolon. E.g. "eth0=1".
 */
#ifndef MIPHA_IF_FA_AUTH_LIST
#define MIPHA_IF_FA_AUTH_LIST       ""
#endif

/*
 * MIPHA_IF_ADV_INTERVAL_LIST - Advertisement interval
 * The interval at which the home agent will send ICMP router advertisements on
 * this interface specified in the form of "<ifname>=<value>", each separated
 * from the other by a semicolon.  If set to 0, the home agent will not send
 * any router advertisements, also, none of the other adv_ sysvars need to be
 * set. The default value of 3 is good for testing purposes, whereas the RFC
 * suggests about a value of a third of the ICMP router advertisement's
 * lifetime. This value can be set high if mobile nodes are configured to use
 * solicitations upon entering new subnets. E.g. "eth0=10".
 */
#ifndef MIPHA_IF_ADV_INTERVAL_LIST
#define MIPHA_IF_ADV_INTERVAL_LIST  ""
#endif

/*
 * MIPHA_IF_ADV_ADDRESS_LIST - Router advertisement address
 * The destination address of outgoing router advertisement messages from the
 * home agent. This parameter is configurable on a per-interface basis in the
 * form of "<ifname>=<value>", each separated from the other by a semicolon.
 * E.g. "eth0=224.0.0.11".
 */
#ifndef MIPHA_IF_ADV_ADDRESS_LIST
#define MIPHA_IF_ADV_ADDRESS_LIST   ""
#endif

/*
 * MIPHA_IF_ADV_LIFETIME_LIST - Router advertisement lifetime
 * The lifetime of advertisement messages from the home agent, in seconds. 
 * This parameter is configurable on a per-interface basis in the form of
 * "<ifname>=<value>", each separated from the other by a semicolon.
 * E.g. "eth0=300".
 */
#ifndef MIPHA_IF_ADV_LIFETIME_LIST
#define MIPHA_IF_ADV_LIFETIME_LIST  ""
#endif

/*
 * MIPHA_IF_NAT_T_ENABLED_LIST - Enable NAT-T
 * Enables or disables NAT Traversal according to RFC3519. This parameter is
 * configured on a per-interface basis in the form of "<ifname>=<value>", 
 * each separated from the other by a semicolon. E.g. "eth0=1".
 */
#ifndef MIPHA_IF_NAT_T_ENABLED_LIST
#define MIPHA_IF_NAT_T_ENABLED_LIST     ""
#endif

/*
 * MIPHA_IF_NAT_T_KEEPALIVE_LIST - NAT-T keepalive
 * Decides the keepalive timeout (in seconds) to use for NAT Traversal ICMP
 * keepalive messages in order to maintain a NAT device's UDP port mapping.
 * This parameter is configurable on a per-interface basis in the form of
 * "<ifname>=<timeout>", each separated from the other by a semicolon. 
 * E.g. "eth0=120"
 */
#ifndef MIPHA_IF_NAT_T_KEEPALIVE_LIST
#define MIPHA_IF_NAT_T_KEEPALIVE_LIST   ""
#endif

#endif /* INCLUDE_IPMIPHA */

#ifdef INCLUDE_IPMIPMN

#ifndef MIPMN_IFNAME
#define MIPMN_IFNAME        ""     /* Mobile node interface */
#endif
#ifndef MIPMN_HOME_AGENT
#define MIPMN_HOME_AGENT    ""     /* Home agent Ipv4 address */
#endif
#ifndef MIPMN_MTU_REDUCTION
#define MIPMN_MTU_REDUCTION   "0"  /* Reduce the MTU of tunneled routes */
#endif
#ifndef MIPMN_HOME_ADDRESS
#define MIPMN_HOME_ADDRESS  "0.0.0.0" /* Home address, "0.0.0.0" means any */
#endif
#ifndef MIPMN_HOME_MASK
#define MIPMN_HOME_MASK     ""       /* Home netmask */
#endif
#ifndef  MIPMN_HOME_GATEWAY
#define MIPMN_HOME_GATEWAY  ""       /* Home gateway */
#endif
#ifndef MIPMN_SOL_ADDRESS
#define MIPMN_SOL_ADDRESS   ""       /* Router solicitation address */
#endif
#ifndef MIPMN_HA_AUTH_SECRET
#define MIPMN_HA_AUTH_SECRET ""      /* Home agent shared secret */
#endif
#ifndef MIPMN_HA_AUTH_SPI
#define MIPMN_HA_AUTH_SPI    ""      /* Home agent SPI */
#endif
#ifndef MIPMN_RUN_MODE
#define MIPMN_RUN_MODE       "fa"    /* Home agent run mode */
#endif
#ifndef MIPMN_FA_AUTH_ENABLED
#define MIPMN_FA_AUTH_ENABLED  FALSE /* Enable Mobile-foreign authentication */
#endif
#ifndef MIPMN_FA_AUTH_SECRET
#define MIPMN_FA_AUTH_SECRET  ""      /* Foreign agent shared secret */
#endif
#ifndef MIPMN_FA_AUTH_SPI
#define MIPMN_FA_AUTH_SPI     ""       /* Foreign agent SPI */
#endif
#ifndef MIPMN_REG_LIFETIME
#define MIPMN_REG_LIFETIME    "30"     /* Registration lifetime */
#endif
#ifndef MIPMN_RECV_BROADCASTS
#define MIPMN_RECV_BROADCASTS "0"      /* Receive broadcasts */
#endif
#ifndef MIPMN_TUNNEL_TYPE
#define MIPMN_TUNNEL_TYPE     "ipip"   /* Tunnel type */
#endif
#ifndef MIPMN_REVERSE_TUNNELING
#define MIPMN_REVERSE_TUNNELING  "1"   /* Reverse tunneling */
#endif
#ifndef MIPMN_NAI
#define MIPMN_NAI             ""        /* Network access identifier */
#endif
#ifndef MIPMN_HA_NAI
#define MIPMN_HA_NAI          ""        /* HA Network access identifier */
#endif
#ifndef MIPMN_AAAH_NAI
#define MIPMN_AAAH_NAI        ""  /* AAAH Server Network access identifier */
#endif
#ifndef MIPMN_SOLICIT
#define MIPMN_SOLICIT         "1"          /* Solicitations */
#endif
#ifndef MIPMN_IPSEC_PROTECTED
#define MIPMN_IPSEC_PROTECTED    "0"  /* IPSEC protected CoA only */
#endif 
#ifndef MIPMN_IPIKE_RECONFIGURE
#define MIPMN_IPIKE_RECONFIGURE  "0"  /* IPIKE reconfiguration on movement */
#endif
#ifndef MIPMN_IPIKE_SECURE
#define MIPMN_IPIKE_SECURE  "0"     /* use IPIKE secure address only */
#endif
#ifndef MIPMN_NAT_T_ENABLED
#define MIPMN_NAT_T_ENABLED   "1"   /* NAT Traversal */
#endif
#ifndef MIPMN_NAT_T_FORCED
#define MIPMN_NAT_T_FORCED    "0"   /* Forced NAT Traversal */
#endif
#ifndef MIPMN_NAT_T_KEEPALIVE
#define MIPMN_NAT_T_KEEPALIVE  "120"  /* NAT Traversal keepalive */
#endif
#ifndef MIPMN_NAT_T_TUNNEL_TYPE
#define MIPMN_NAT_T_TUNNEL_TYPE ""   /* NAT Traversal tunnel type */
#endif

/*
 * MIPMN_CVSE_LIST - Critical vendor specific extensions
 * Specifies the critical vendor specific extensions (CVSE) that should
 * be attached to the communications as according to RFC 3115. Note however
 * that using a CVSE that is not understood by either the FA or HA will
 * terminate the request. The CVSE is specified in the form of
 * "<vendor_id>.<vendor_type>=<vendor_data>", where vendor_id is the SMI
 * number, vendor_type is the vendor administrated vendor subtype and
 * vendor_data is the vendor specific opaque information. Each pair of cvse
 * is semicolon separated. The value is specified either as plain text, or
 * as a hexadecimal number, prepended with '0x'. That is, "vendor value",
 * and "0xdeadbeef" are both valid values. E.g. "0.1=Testing", 
 * "0.1=Testing;0.2=Testing2".
 */
#ifndef MIPMN_CVSE_LIST
#define MIPMN_CVSE_LIST     ""
#endif

/*
 * MIPMN_CVSE_LIST - Normal vendor specific extensions
 * Specifies the normal vendor specific extensions (NVSE) that should be
 * attached to the communications as according to RFC 3115. A NVSE extension
 * that is not understood by transit nodes or the HA are silently discarded.
 * The NVSE is specified in the form of
 * "<vendor_id>.<vendor_type>=<vendor_data>", where vendor_id is the SMI
 * number, vendor_type is the vendor administrated vendor subtype and
 * vendor_data is the vendor specific opaque information. Each pair of nvse
 * is semicolon separated. The value is specified either as plain text, or
 * as a hexadecimal number, prepended with '0x'. That is, "vendor value",
 * and "0xdeadbeef" are both valid values. E.g. "0.1=Testing", 
 * "0.1=Testing;0.2=Testing2".
 */
#ifndef MIPMN_NVSE_LIST
#define MIPMN_NVSE_LIST     ""
#endif

#endif /* INCLUDE_IPMIPMN */

#ifdef INCLUDE_IPMIP6MN

#ifndef MIP6MN_NETWORK_ENUM
#define MIP6MN_NETWORK_ENUM     "windriver"     /* Network enumeration */
#endif

/*
 * MIP6MN_IF_NAME_LIST - Interface enumeration list
 * The interface enumeration list specifies what interfaces should be used
 * for a particular network configuration. This parameter is configurable on
 * a per-interface basis in the form of "<ifname>=<value>", each separated
 * from the other by a semicolon. E.g. "0=eth0;1=eth1"
 */
#ifndef MIP6MN_IF_NAME_LIST
#define MIP6MN_IF_NAME_LIST     ""
#endif

/*
 * MIP6MN_IF_HOMEAGENT_LIST - Home Agent enumeration
 * Specifies what home agents should be used for a particular network
 * configuration in the form of "<enum>=<value>", each separated from the
 * other by a semicolon. If no home agents are enumerated, the Mobile Node
 * will resort to Dynamic Home Agent Address Discovery (DHAAD). The <value>
 * is split into two parts separated by "/". Prior to the "/" is a number
 * specifying the priority of the given home agent. After the "/" is the
 * IPv6 address. A home agent with higher priority is always used before
 * one with lower. In the case of equal priorities it will be randomly
 * decided. E.g. "0=5/3ffe:302:111:3::1;1=4/3ffe:302:111:3::6"
 */
#ifndef MIP6MN_IF_HOMEAGENT_LIST
#define MIP6MN_IF_HOMEAGENT_LIST    ""
#endif

#ifndef MIP6MN_HOME_ADDRESS
#define MIP6MN_HOME_ADDRESS     ""        /* address the mobile node  */
#endif
#ifndef MIP6MN_LIFETIME
#define MIP6MN_LIFETIME         "120"     /* Requested binding lifetime */
#endif
#ifndef MIP6MN_REG_TIMEOUT
#define MIP6MN_REG_TIMEOUT      "1500"    /* Initial Registration Timeout */
#endif
#ifndef MIP6MN_SA_AUTH_MODE 
#define MIP6MN_SA_AUTH_MODE     "SHA1"    /* Security association auth mode */
#endif
#ifndef MIP6MN_SA_AUTH_KEY
#define MIP6MN_SA_AUTH_KEY  "HMACSHA196 AUTH PADN"  /* SA auth key */
#endif
#ifndef MIP6MN_SA_ENC_MODE
#define MIP6MN_SA_ENC_MODE  "3DES"      /* Security association enc mode */
#endif
#ifndef MIP6MN_SA_ENC_KEY
#define MIP6MN_SA_ENC_KEY  "3DES-CBC Private Enc PAD"   /* SA enc key */
#endif

/* 
 * MIP6MN_SA_AUTH_MODE_LIST - Security association auth mode list
 * Specifies the security association configuration that the mobile node 
 * should use for a particular mobile node and traffic. This parameter is 
 * specified in the form of "<spi_value>=<value>", each separated from the 
 * other by a semicolon. E.g. "0=SHA1;1=PADL"
 */
#ifndef MIP6MN_SA_AUTH_MODE_LIST
#define MIP6MN_SA_AUTH_MODE_LIST    ""
#endif

/*
 * MIP6MN_SA_AUTH_KEY_LIST - Security association auth key list
 * Specifies the security association configuration that the mobile node
 * should use for a particular mobile node and traffic. This parameter is
 * specified in the form of "<spi_value>=<value>", each separated from
 * the other by a semicolon. E.g. "0=HMACSHA196;1=Private".
 */
#ifndef MIP6MN_SA_AUTH_KEY_LIST 
#define MIP6MN_SA_AUTH_KEY_LIST     ""
#endif

/*
 * MIP6MN_SA_ENC_MODE_LIST - Security association enc mode list
 * Specifies the security association configuration that the mobile node
 * should use for a particular mobile node and traffic. This parameter is
 * specified in the form of "<spi_value>=<value>", each separated from the
 * other by a semicolon. E.g. "0=3DES;1=3DES".
 */
#ifndef MIP6MN_SA_ENC_MODE_LIST
#define MIP6MN_SA_ENC_MODE_LIST     ""
#endif

/*
 * MIP6MN_SA_ENC_KEY_LIST - Security association enc key list
 * Specifies the security association configuration that the mobile node
 * should use for a particular mobile node and traffic. This parameter is
 * specified in the form of "<spi_value>=<value>", each separated from the
 * other by a semicolon. E.g. "0=3DES-CBC".
 */
#ifndef MIP6MN_SA_ENC_KEY_LIST
#define MIP6MN_SA_ENC_KEY_LIST      ""
#endif

#ifndef MIP6MN_IPSEC_MH_ENABLED
#define MIP6MN_IPSEC_MH_ENABLED     "1"  /* Enable mobile header signaling */
#endif
#ifndef MIP6MN_IPSEC_MH_SPI_IN
#define MIP6MN_IPSEC_MH_SPI_IN      "200"   /* IPSec MH SPI In */
#endif
#ifndef MIP6MN_IPSEC_MH_SPI_OUT
#define MIP6MN_IPSEC_MH_SPI_OUT      "201"   /* IPSec MH SPI Out */
#endif
#ifndef MIP6MN_IPSEC_PAYLOAD_ENABLED
#define MIP6MN_IPSEC_PAYLOAD_ENABLED "0"   /* Enable payload data protection */
#endif
#ifndef MIP6MN_IPSEC_PAYLOAD_SPI_IN
#define MIP6MN_IPSEC_PAYLOAD_SPI_IN  "204"  /* IPSec Payload SPI In */
#endif
#ifndef MIP6MN_IPSEC_PAYLOAD_SPI_OUT
#define MIP6MN_IPSEC_PAYLOAD_SPI_OUT  "205"  /* IPSec Payload SPI Out */
#endif

#endif /* INCLUDE_IPMIP6MN */


/******************************************************************************/
/*                                                                            */
/*                     NETWORK APPLICATIONS CONFIGURATION                     */
/*                                                                            */
/******************************************************************************/

#ifdef INCLUDE_IPDHCPC
#ifndef DHCPC_CLIENT_PORT
#define DHCPC_CLIENT_PORT       "68"  /* DHCP client port */
#endif
#ifndef DHCPC_SERVER_PORT
#define DHCPC_SERVER_PORT       "67"  /* DHCP server port */
#endif
#ifndef DHCPC_REQ_OPTS
#define DHCPC_REQ_OPTS          NULL  /* Global list of req DHCP options */
#endif
#ifndef DHCPC_CLIENT_ID
#define DHCPC_CLIENT_ID         NULL  /* Global client identifier */
#endif
#ifndef DHCPC_RFC2131_INIT_DELAY
#define DHCPC_RFC2131_INIT_DELAY    "1"
#endif
#ifndef DHCPC_RFC2131_EXP_BACKOFF
#define DHCPC_RFC2131_EXP_BACKOFF   "1"
#endif
#ifndef DHCPC_DISCOVER_RETRIES
#define DHCPC_DISCOVER_RETRIES      "4"
#endif
#ifndef DHCPC_OFFER_TIMEOUT
#define DHCPC_OFFER_TIMEOUT         "2000"
#endif

/*
 * DHCPC_IF_REQ_OPTS_LIST - Interface specific list of requested DHCP options
 * An interface name followed by comma separated list of DHCP options numbers
 * that the DHCP client wants the server to provide in the format
 * <ifparam>=<opts>. Each pair of <ifparam>=<opts> is semicolon separated,
 * e.g. "eth0=1,2,3,4;eth1=2,3,4,5".
 */
#ifndef DHCPC_IF_REQ_OPTS_LIST
#define DHCPC_IF_REQ_OPTS_LIST    ""
#endif

/*
 * DHCPC_IF_CLIENT_ID_LIST - Interface specific list of client identifier
 * A string of characters that the DHCP client wants the server to identify
 * it by in the format <ifparam>=<id>. Each pair of <ifparam>=<id> is
 * semicolon separated, e.g. "eth0=my_red_computer_eth0" or
 * "eth0=my_red_computer_eth0;eth1=my_blue_computer_eth1".
 */
#ifndef DHCPC_IF_CLIENT_ID_LIST
#define DHCPC_IF_CLIENT_ID_LIST  ""
#endif

/*
 * DHCPC_INSTALL_CALLBACK_HOOK - install dhcp client callback hook
 * Determine if the user-defined dhcp client callback routine as specified
 * by the macro DHCPC_OPTION_CALLBACK_HOOK should be installed. If FALSE,
 * the DHCPC_OPTION_CALLBACK_HOOK configuration is not applicable.
 */
#ifndef DHCPC_INSTALL_CALLBACK_HOOK
#define DHCPC_INSTALL_CALLBACK_HOOK     FALSE
#endif

/*
 * DHCPC_OPTION_CALLBACK_HOOK - DHCP client option callback routine
 * This user-defined dhcp option processing callback routine is called once
 * for each option prior to IPDHCPC is acting on the option, i.e. this
 * callback can be used to change the contents of the DHCP server reply
 * (caution!). If not specified, the internal option callback routine will
 * be used. The DHCPC_INSTALL_CALLBACK_HOOK must also be set to TRUE in order
 * to install this callback routine.
 */
#ifndef DHCPC_OPTION_CALLBACK_HOOK
#define DHCPC_OPTION_CALLBACK_HOOK  NULL
#endif

#endif /* INCLUDE_IPDHCPC */

#ifdef INCLUDE_IPDHCPR
#ifndef DHCPR_SERVER_PORT
#define DHCPR_SERVER_PORT   "67"  /* UDP port used by the DHCP server */
#endif
#ifndef DHCPR_CLIENT_PORT
#define DHCPR_CLIENT_PORT   "68"  /* UDP port used by the dhcp/bootp clients */
#endif
#ifndef DHCPR_MAX_PKT_SIZE
#define DHCPR_MAX_PKT_SIZE  "576" /* Max DHCP packet size */
#endif
#ifndef DHCPR_HOPS_THRESHOLD
#define DHCPR_HOPS_THRESHOLD  "3" /* Maximum number of hops */
#endif

/*
 * DHCPR_NETCONF_SYSVAR - dhcp relay network pre-configuration
 * DHCP relay network pre-configuratio array is used to statically configure
 * the dhcp relay with a preconfigured setup of networks and hosts when 
 * initiated. Each system variable in this array must be named according to
 * the syntax ipdhcpr.netconf.<index>, where <index> is a unique line
 * indicator. The value of each system variable must consist of a IPDHCPR
 * shell command (without the preceeding dhcpr that is used only when
 * executing from a shell). The commands are executed in the order they are
 * defined. See ipdhcpr_netconf_sysvar Ipcom_sysvar array in the 
 * ipdhcps_config.c configletee for the example usage.
 */
#ifndef DHCPR_NETCONF_SYSVAR
#define DHCPR_NETCONF_SYSVAR        NULL
#endif

/*
 * DHCPR_INSTALL_CALLBACK_HOOK - Install dhcp relay callback routine
 * Determine if the user-defined dhcp relay callback routines as specified
 * by the macro DHCPR_START_CALLBACK_HOOK should be installed. If FALSE,
 * the DHCPR_START_CALLBACK_HOOK configuration is not applicable.
 */
#ifdef DHCPR_INSTALL_CALLBACK_HOOK
#define DHCPR_INSTALL_CALLBACK_HOOK     FALSE
#endif

/*
 * DHCPR_START_CALLBACK_HOOK - dhcp relay startup callback routine
 * DHCP relay startup callback routine. This routine, if provided, will be
 * called by DHCP relay at startup. It is intended to be used for user
 * supplied startup configuration of the DHCP relay. The
 * DHCPR_INSTALL_CALLBACK_HOOK must also be set to TRUE in order to install
 * this callback hook.
 */
#ifndef DHCPR_START_CALLBACK_HOOK
#define DHCPR_START_CALLBACK_HOOK       NULL
#endif

#endif /* INCLUDE_IPDHCPR */

#ifdef INCLUDE_IPDHCPS
#ifndef DHCPS_SERVER_PORT
#define DHCPS_SERVER_PORT  "67"  /* UDP port used by the DHCP server */
#endif
#ifndef DHCPS_CLIENT_PORT
#define DHCPS_CLIENT_PORT  "68"  /* UDP port used by the dhcp/bootp clients */
#endif
#ifndef DHCPS_DEFAULT_LEASE_TIME
#define DHCPS_DEFAULT_LEASE_TIME  "864000"  /* Default lease time */
#endif
#ifndef DHCPS_MAX_LEASE_TIME
#define DHCPS_MAX_LEASE_TIME      "8640000" /* Maximum lease time */ 
#endif
#ifndef DHCPS_MIN_LEASE_TIME
#define DHCPS_MIN_LEASE_TIME      "60"   /* Minimum lease time */
#endif
#ifndef DHCPS_RENEWAL_TIME
#define DHCPS_RENEWAL_TIME        "disabled"  /* Lease renewal time */
#endif
#ifndef DHCPS_REBINDING_TIME
#define DHCPS_REBINDING_TIME      "disabled"  /* Lease rebinding time */
#endif
#ifndef DHCPS_ABANDONED_STATE_MAX_TIME
#define DHCPS_ABANDONED_STATE_MAX_TIME  "8640000" /* Abandoned state max time */
#endif
#ifndef DHCPS_EXPIRED_STATE_MAX_TIME
#define DHCPS_EXPIRED_STATE_MAX_TIME    "forever" /* Expired state max time */
#endif
#ifndef DHCPS_RELEASED_STATE_MAX_TIME
#define DHCPS_RELEASED_STATE_MAX_TIME   "forever" /* Released state max time */
#endif
#ifndef DHCPS_OFFERED_STATE_MAX_TIME
#define DHCPS_OFFERED_STATE_MAX_TIME    "10"      /* Offered state max time */
#endif
#ifndef DHCPS_LEASE_BOOTPC_MAX_TIME
#define DHCPS_LEASE_BOOTPC_MAX_TIME "forever" /* bootp client lease max time */
#endif
#ifndef DHCPS_ALLOW_DECLINE
#define DHCPS_ALLOW_DECLINE       "true"  /* Allow decline */
#endif
#ifndef DHCPS_PKT_SIZE
#define DHCPS_PKT_SIZE            "576"   /* Packet size */
#endif
#ifndef DHCPS_ALLOW_BOOTP
#define DHCPS_ALLOW_BOOTP         "true"  /* Allow bootp */
#endif
#ifndef DHCPS_ALLOW_DYNAMIC_BOOTP
#define DHCPS_ALLOW_DYNAMIC_BOOTP "false" /* Allow dynamic bootp */
#endif
#ifndef DHCPS_DO_ICMP_ADDRESS_CHECK
#define DHCPS_DO_ICMP_ADDRESS_CHECK  "false" /* Check address */
#endif
#ifndef DHCPS_AUTHORIZED_AGENTS
#define DHCPS_AUTHORIZED_AGENTS     "any"   /* authorized dhcp relay agent */
#endif

/*
 * DHCPS_NETCONF_SYSVAR - dhcp server network configuration array
 * DHCP server network pre-configuratio array is used to statically configure
 * the dhcp server with a preconfigured setup of networks and hosts when
 * initiated. Each system variable in this array must be named according to
 * the syntax ipdhcps.netconf.<index>, where <index> is a unique line
 * indicator. The value of each system variable must consist of a IPDHCPS
 * shell command (without the preceeding dhcps that is used only when
 * executing from a shell). The commands are executed in the order they are
 * defined. See ipdhcps_netconf_sysvar Ipcom_sysvar array in 
 * dhcps_config.c configlette for the example usage.
 */
#ifndef DHCPS_NETCONF_SYSVAR
#define DHCPS_NETCONF_SYSVAR      NULL
#endif

/*
 * DHCPS_INSTALL_CALLBACK_HOOK - install dhcp server callback routines
 * Determine if the user-defined dhcp server callback routines as specified
 * by the macro DHCPS_START_CALLBACK_HOOK and DHCPS_STOP_CALLBACK_HOOK 
 * should be installed. If FALSE, the DHCPS_START_CALLBACK_HOOK and 
 * DHCPS_STOP_CALLBACK_HOOK configurations are not applicable.
 *
 */
#ifndef DHCPS_INSTALL_CALLBACK_HOOK
#define DHCPS_INSTALL_CALLBACK_HOOK     FALSE
#endif

/*
 * DHCPS_START_CALLBACK_HOOK - dhcp server startup callback routine
 * This callback routine, if provided, will be called by DHCP server at
 * startup. It is intended to be used for static configuration of DHCP server,
 * i.e. to install a lease 'dump to file' routine and/or to restore a lease
 * database from a file. The DHCPS_INSTALL_CALLBACK_HOOK must also 
 * be set to TRUE in order to install this callback hook.
 */
#ifndef DHCPS_START_CALLBACK_HOOK
#define DHCPS_START_CALLBACK_HOOK   NULL
#endif

/*
 * DHCPS_STOP_CALLBACK_HOOK - dhcp server termination callback routine
 * This callback routine, if provided, will be called by DHCP server upon
 * termination of the DHCP server. It is intended to be used for static
 * configuration of IPDHCPS, for instance to save the lease database at
 * shutdown so that the lease can later be restored. The 
 * DHCPS_INSTALL_CALLBACK_HOOK must also be set to TRUE in order to install
 * this callback hook.
 */
#ifndef DHCPS_STOP_CALLBACK_HOOK
#define DHCPS_STOP_CALLBACK_HOOK     NULL
#endif

#endif /* INCLUDE_IPDHCPS */

#ifdef INCLUDE_IPDHCPC6
#ifndef DHCPC6_DUID_TYPE
#define DHCPC6_DUID_TYPE    "ll"   /* DUID type: ll, llt, or en */
#endif
#ifndef DHCPC6_DUID_IF
#define DHCPC6_DUID_IF      ""      /* DUID interface: eth0, etc. */
#endif
#ifndef DHCPC6_DUID_EN_NUM
#define DHCPC6_DUID_EN_NUM "1000" /* DUID EN number; must specify for en */
#endif
#ifndef DHCPC6_DUID_EN_VAL
#define DHCPC6_DUID_EN_VAL "0xabcdef" /* DUID EN value; must specify for en */
#endif
/*
 * DHCPC6_IF_ENUM_LIST is a list of interface/Status pairs, each separated
 * from the other by a semi-colon as follows:
 * "eth0=enable;vlan21=automatic"
 */
#ifndef DHCPC6_IF_ENUM_LIST
#define DHCPC6_IF_ENUM_LIST ""
#endif
/*
 * DHCPC6_IF_RAPID_COMMIT_LIST is a list of interface/Rapid Commit Status
 * pairs, each separated from the other by a semi-colon as follows:
 * "eth0=enable;vlan21=automatic"
 */
#ifndef DHCPC6_IF_RAPID_COMMIT_LIST
#define DHCPC6_IF_RAPID_COMMIT_LIST ""
#endif
/*
 * DHCPC6_IF_INFORMATION_ONLY_LIST is a list of interface/Information Only
 * Status pairs, each separated from the other by a semi-colon as follows:
 * "eth0=enable;vlan21=automatic"
 */
#ifndef DHCPC6_IF_INFORMATION_ONLY_LIST
#define DHCPC6_IF_INFORMATION_ONLY_LIST ""
#endif
/*
 * DHCPC6_IF_DNS_LIST is a list of interface/DNS Status pairs, each separated
 * from the other by a semi-colon as follows:
 * "eth0=enable;vlan21=automatic"
 */
#ifndef DHCPC6_IF_DNS_LIST
#define DHCPC6_IF_DNS_LIST ""
#endif
/*
 * DHCPC6_IF_SNTP_LIST is a list of interface/SNTP Status pair, each separated
 * from the other by a semi-colon as follows:
 * "eth0=enable;vlan21=enable"
 */
#ifndef DHCPC6_IF_SNTP_LIST
#define DHCPC6_IF_SNTP_LIST ""
#endif
/*
 * DHCPC6_IF_INFO_REFRESH_LIST is a lis of interface/Information Refresh
 * Status pair, each separated from the other by a semi-colon as follows:
 * "eth0=enable;vlan21=enable".
 */
#ifndef DHCPC6_IF_INFO_REFRESH_LIST
#define DHCPC6_IF_INFO_REFRESH_LIST ""
#endif
/*
 * DHCPC6_IF_INFO_REFRESH_MIN_LIST is a list of interface/Information
 * Refresh Minimum Value pair, each separated from the other by a
 * semi-colon as follows:
 * "eth0=600;vlan21=600".
 */
#ifndef DHCPC6_IF_INFO_REFRESH_MIN_LIST
#define DHCPC6_IF_INFO_REFRESH_MIN_LIST ""
#endif
/*
 * DHCPC6_IF_INFO_REFRESH_DEFAULT_LIST is a list of interface/Information
 * Refresh Default Value pair, each separated from the other by a 
 * semi-colon as follows:
 * "eth0=86400;vlan21=86400".
 */
#ifndef DHCPC6_IF_INFO_REFRESH_DEFAULT_LIST
#define DHCPC6_IF_INFO_REFRESH_DEFAULT_LIST ""
#endif
/*
 * DHCPC6_IF_INFO_REFRESH_MAX_LIST is a list of interface/Information
 * Refresh Maximum Value pair, each separated from the other by a 
 * semi-colon as follows:
 * "eth0=0;vlan21=0".
 */
#ifndef DHCPC6_IF_INFO_REFRESH_MAX_LIST
#define DHCPC6_IF_INFO_REFRESH_MAX_LIST ""
#endif
/*
 * DHCPC6_IF_IA_NA_DEFAULT_LIST a list of interface/IA_NA Default Status pairs,
 * each separated from the other by a semi-colon as follows:
 * "eth0=enable;vlan21=automatic"
 */
#ifndef DHCPC6_IF_IA_NA_DEFAULT_LIST
#define DHCPC6_IF_IA_NA_DEFAULT_LIST ""
#endif
/*
 * DHCPC6_IF_IA_NA_DEFAULT_IAID_LIST is list of interface/IA_NA Default IAID
 * pairs, each separated from the other by a semi-colon as follows:
 * "eth0=1;vlan21=2"
 */
#ifndef DHCPC6_IF_IA_NA_DEFAULT_IAID_LIST
#define DHCPC6_IF_IA_NA_DEFAULT_IAID_LIST ""
#endif
/* Hints for this IA  */
/*
 * DHCPC6_IF_HINTS_DEFAULT_ENUM_LIST is a list of interface/Default Hints
 * Status pairs, each separated from the other by a semi-colon as follows:
 * "eth0=enable;vlan21=automatic"
 */
#ifndef DHCPC6_IF_HINTS_DEFAULT_ENUM_LIST
#define DHCPC6_IF_HINTS_DEFAULT_ENUM_LIST ""
#endif
/*
 * DHCPC6_IF_HINTS_DEFAULT_PREFIX_LIST is a list of interface/Default Prefix
 * Hints pairs, each separated from the other by a semi-colon as follows:
 * "eth0=3FFE:01;vlan21=3FFE:02"
 */
#ifndef DHCPC6_IF_HINTS_DEFAULT_PREFIX_LIST
#define DHCPC6_IF_HINTS_DEFAULT_PREFIX_LIST ""
#endif
/*
 * DHCPC6_IF_HINTS_DEFAULT_VALID_LIST is a list of interface/Default Valid
 * Hints pairs, each separated from the other by a semi-colon as follows:
 * "eth0=8000;vlan21=7000"
 */
#ifndef DHCPC6_IF_HINTS_DEFAULT_VALID_LIST
#define DHCPC6_IF_HINTS_DEFAULT_VALID_LIST ""
#endif
/*
 * DHCPC6_IF_HINTS_DEFAULT_PREFERRED_LIST is a list of interface/Default Preferred
 * Hints pairs, each separated from the other by a semi-colon as follows:
 * "eth0=8000;vlan21=7000"
 */
#ifndef DHCPC6_IF_HINTS_DEFAULT_PREFERRED_LIST
#define DHCPC6_IF_HINTS_DEFAULT_PREFERRED_LIST ""
#endif
/*
 * DHCPC6_IF_HINTS_DEFAULT_RENEW_LIST is a list of interface/Default Renew
 * Hints pairs, each separated from the other by a semi-colon as follows:
 * "eth0=8000;vlan21=4000"
 */
#ifndef DHCPC6_IF_HINTS_DEFAULT_RENEW_LIST
#define DHCPC6_IF_HINTS_DEFAULT_RENEW_LIST ""
#endif
/*
 * DHCPC6_IF_HINTS_DEFAULT_REBIND_LIST is a list of interface/Default Rebind
 * Hints pairs, each separated from the other by a semi-colon as follows:
 * "eth0=8000;vlan21=4000"
 */
#ifndef DHCPC6_IF_HINTS_DEFAULT_REBIND_LIST
#define DHCPC6_IF_HINTS_DEFAULT_REBIND_LIST ""
#endif

#endif /* INCLUDE_IPDHCPC6 */

#ifdef INCLUDE_IPDHCPS6
/*
 * The mode the DHCPv6 daemon is running in, can be 'server' or 'relay'.
 * Default is 'server'
 */
#ifndef DHCPS6_MODE
#define DHCPS6_MODE "server"
#endif

/* The interface the server will use to generate the server ID */
#ifndef DHCPS6_DUID_IFNAME
#define DHCPS6_DUID_IFNAME "eth0"
#endif

/* Specifies if the server allows rapid commit or not valid values [yes|no] */
#ifndef DHCPS6_ALLOW_RAPID_COMMIT
#define DHCPS6_ALLOW_RAPID_COMMIT "yes"
#endif

/* Max hop count in a Relay-forward message, default is 32 */
#ifndef DHCPS6_MAXHOP_COUNT
#define DHCPS6_MAXHOP_COUNT "32"
#endif

/*
 * Specifies which DHCPv6 server or relay agent messages received
 * on interface specified after 'ipdhcps6.relay.map.' should be sent to
 * Leave the address field empty to use the all-dhcp-servers (ff05::1:3)
 * multicast address
 *
 * DHCPS6_RELAY_MAP is a list of Source Interface/Destination Interface
 * pairs, each separated from the other with a semi-colon as follows:
 * "eth0=eth1;vlan21=eth2"
 */
#ifndef DHCPS6_IF_RELAY_MAP_LIST
#define DHCPS6_IF_RELAY_MAP_LIST ""
#endif

#endif /* INCLUDE_IPDHCPS6 */

#ifdef INCLUDE_IPDNSC
#ifndef DNSC_SERVER_PORT
#define DNSC_SERVER_PORT          "53"   /* server listening port */
#endif
#ifndef DNSC_TIMEOUT
#define DNSC_TIMEOUT              "10"   /* query timeout */
#endif
#ifndef DNSC_RETRIES
#define DNSC_RETRIES              "2"    /* query retries */
#endif
#ifndef DNSC_DOMAIN_NAME
#define DNSC_DOMAIN_NAME          "windriver.com"   /* domain name  */
#endif
#ifndef DNSC_PRIMARY_NAME_SERVER
#define DNSC_PRIMARY_NAME_SERVER    ""     /* primary name server */
#endif
#ifndef DNSC_SECONDARY_NAME_SERVER
#define DNSC_SECONDARY_NAME_SERVER  ""   /* secondary name server */
#endif
#ifndef DNSC_TERTIARYNS_NAME_SERVER
#define DNSC_TERTIARYNS_NAME_SERVER ""   /* tertiaryns name server */
#endif
#ifndef DNSC_QUATERNARYNS_NAME_SERVER
#define DNSC_QUATERNARYNS_NAME_SERVER   ""    /* quaternaryns name server */
#endif
#ifndef DNSC_IP4_ZONE
#define DNSC_IP4_ZONE          "in-addr.arpa"  /* ipv4 zone for name lookup */
#endif
#ifndef DNSC_IP6_ZONE
#define DNSC_IP6_ZONE          "ip6.int"   /* ipv6 zone for name lookup */
#endif
#endif /* INCLUDE_IPDNSC */

#ifdef INCLUDE_IPFIREWALL
#ifndef IPF_ICMP_TIMEOUT
#define IPF_ICMP_TIMEOUT          "60"      /* ICMP stateful mapping timeout */
#endif
#ifndef IPF_UDP_TIMEOUT
#define IPF_UDP_TIMEOUT           "60"      /* UDP stateful mapping timeout */
#endif
#ifndef IPF_TCP_TIMEOUT
#define IPF_TCP_TIMEOUT           "432000"  /* TCP stateful mapping timeout */
#endif
#ifndef IPF_OTHER_TIMEOUT
#define IPF_OTHER_TIMEOUT         "60"      /* Other stateful mapping timeout */
#endif
#ifndef IPF_MAX_STATEFUL_MAPPINGS
#define IPF_MAX_STATEFUL_MAPPINGS  "1000"   /* Max num of stateful mappings */
#endif
#ifndef IPF_IPV4_RULE_FILE
#define IPF_IPV4_RULE_FILE      IPCOM_FILE_ROOT"fw4.cfg"  /* IPv4 rule file */ 
#endif
#ifndef IPF_IPV6_RULE_FILE
#define IPF_IPV6_RULE_FILE      IPCOM_FILE_ROOT"fw6.cfg"  /* Ipv6 rule file */
#endif
#ifndef IPF_FWMAC_RULE_FILE
#define IPF_FWMAC_RULE_FILE  IPCOM_FILE_ROOT"fwmac.cfg"  /* MAC rule file */
#endif 
#endif /* INCLUDE_IPFIREWALL */

#ifdef INCLUDE_IPFTPS
#ifndef FTPS_AUTH_ATTEMPTS
#define FTPS_AUTH_ATTEMPTS        "3"  /* authentication attempts */
#endif
#ifndef FTPS_SLEEP_TIME
#define FTPS_SLEEP_TIME           "5"  /* sleep time after auth fail */
#endif
#ifndef FTPS_LOCAL_PORT_BASE
#define FTPS_LOCAL_PORT_BASE      "49151" /* Local port base number */
#endif
#ifndef FTPS_PEER_PORT_BASE
#define FTPS_PEER_PORT_BASE       "1023"  /* Peer port base number */
#endif
#ifndef FTPS_ENABLE_PROXY
#define FTPS_ENABLE_PROXY         "0"  /* Enable proxy FTP support */
#endif
#ifndef FTPS_MODE
#define FTPS_MODE                 "0"  /* Read/write mode */
#endif
#ifndef FTPS_ROOT_DIR
#define FTPS_ROOT_DIR             "flash:/"  /* FTP root directory */
#endif
#ifndef FTPS_INITIAL_DIR
#define FTPS_INITIAL_DIR          "flash:/"  /* FTP initial directory */
#endif
#ifndef FTPS_MAX_SESSIONS
#define FTPS_MAX_SESSIONS         "8"    /* Max simultaneous sessions */
#endif
#ifndef FTPS_PORT_NUM
#define FTPS_PORT_NUM             "21"   /* Server port number */
#endif
#ifndef FTPS_RECV_TIMEOUT
#define FTPS_RECV_TIMEOUT         "30"   /* Data receive timeout */
#endif
#ifndef FTPS_SEND_TIMEOUT
#define FTPS_SEND_TIMEOUT         "30"   /* Data send timeout */
#endif 
#ifndef FTPS_INACTIVITY_TIMEOUT
#define FTPS_INACTIVITY_TIMEOUT   "300"  /* User inactivity timeout */
#endif
#ifndef FTPS_SYS_TYPE
#define FTPS_SYS_TYPE             "UNIX"   /* Reported system type */
#endif

/*
 * FTPS_INSTALL_CALLBACK_HOOK - install ftp server callback routine
 * Determine if the user-defined ftp server authentication callback routine
 * as specified by the macro FTPS_AUTH_CALLBACK_HOOK should be installed.
 * If FALSE, the FTPS_AUTH_CALLBACK_HOOK configuration is not applicable.
 */
#ifndef FTPS_INSTALL_CALLBACK_HOOK
#define FTPS_INSTALL_CALLBACK_HOOK  TRUE
#endif

/*
 * FTPS_AUTH_CALLBACK_HOOK - Authentication callback routine.
 * User-defined authentication callback routine used by FTP server. If not
 * specified, internal authenticaion callback routine will be used. The
 * FTPS_INSTALL_CALLBACK_HOOK must also be set to TRUE in order to install
 * this callback hook.
 */
#ifndef FTPS_AUTH_CALLBACK_HOOK
#define FTPS_AUTH_CALLBACK_HOOK     usrAuthenticateCallback
#endif

#endif /* INCLUDE_IPFTPS */

#ifdef INCLUDE_IPRADVD
/*
 * RADVD_IFNAME_LIST - The name of the interface radvd will send router
 * advertisement and listen for router solicitation messages. The format is
 * a space-separated list of interface names, e.g. "eth0 eth1". Default:
 * empty list.
 */
#ifndef RADVD_IFNAME_LIST
#define RADVD_IFNAME_LIST             ""
#endif

/*
 * Interface configurations
 * The following configuration parameters can be specified for each interface
 * that are listed in the RADVD_IFNAME_LIST in the form of "<ifparam>=<value>".
 * Each pair of "<ifparam>=<value>" is semicolon separated.
 */

/*
 * RADVD_IFLIST_ADV_MANAGED_FLAG - The TRUE(=1) /FALSE(=0) value to be placed
 * in the "Managed address configuration" flag field in the Router
 * Advertisement. This parameter can be specified for each interface that are
 * listed in the RADVD_IFNAME_LIST in the form of "<ifparam>=<value>". Each
 * pair of "<ifparam>=<value>" is semicolon separated. Default: "0" (=FALSE).
 * E.g. "eth0=0" or "eth0=0;eth1=0".
 */
#ifndef RADVD_IFLIST_ADV_MANAGED_FLAG
#define RADVD_IFLIST_ADV_MANAGED_FLAG        ""
#endif

/*
 * RADVD_IFLIST_ADV_OTHER_CONFIG_FLAG - The TRUE(=1) /FALSE(=0) value to be
 * placed in the "Other stateful configuration" flag field in the Router
 * Advertisement. This parameter can be specified for each interface that are
 * listed in the RADVD_IFNAME_LIST in the form of "<ifparam>=<value>". Each
 * pair of "<ifparam>=<value>" is semicolon separate. Default: "0" (=FALSE).
 * E.g. "eth0=0" or "eth0=0;eth1=0".
 */
#ifndef RADVD_IFLIST_ADV_OTHER_CONFIG_FLAG
#define RADVD_IFLIST_ADV_OTHER_CONFIG_FLAG   ""
#endif

/*
 * RADVD_IFLIST_ADV_LINK_MTU - The value to be placed in MTU options sent by
 * the router. A value of zero indicates that no MTU options are sent, which
 * means that the MTU of the interface will be used by all hosts. This
 * parameter can be specified for each interface that are listed in the
 * RADVD_IFNAME_LIST in the form of "<ifparam>=<value>". Each pair of
 * "<ifparam>=<value>" is semicolon separated. Default: "0".
 * E.g. "eth0=0" or "eth0=0;eth1=0".
 */
#ifndef RADVD_IFLIST_ADV_LINK_MTU
#define RADVD_IFLIST_ADV_LINK_MTU            ""
#endif

/*
 * RADVD_IFLIST_ADV_REACHABLE_TIME - The value to be placed in the Reachable
 * Time field in the Router Advertisement messages sent by the router. The
 * value zero means unspecified (by this router). MUST be no greater than
 * 3,600,000 millisecon ds (1 hour). This parameter can be specified for each
 * interface that are listed in the RADVD_IFNAME_LIST in the form of
 * "<ifparam>=<value>". Each pair of "<ifparam>=<value>" is semicolon
 * separated. Default: "0". E.g. "eth0=0" or "eth0=0;eth1=0".
 */
#ifndef RADVD_IFLIST_ADV_REACHABLE_TIME
#define RADVD_IFLIST_ADV_REACHABLE_TIME      ""
#endif

/*
 * RADVD_IFLIST_ADV_RETRANS_TIMER - The value to be placed in the Retrans
 * Timer field in the Router Advertisement messages sent by the router. The
 * value zero means unspecified (by this router). This parameter can be
 * specified for each interface that are listed in the RADVD_IFNAME_LIST in
 * the form of "<ifparam>=<value>". Each pair of "<ifparam>=<value>" is
 * semicolon separated. Default: "0". E.g. "eth0=0" or "eth0=0;eth1=0".
 */
#ifndef RADVD_IFLIST_ADV_RETRANS_TIMER
#define RADVD_IFLIST_ADV_RETRANS_TIMER       ""
#endif

/*
 * RADVD_IFLIST_ADV_CUR_HOP_LIMIT - The default value to be placed in the
 * Cur Hop Limit field in the Router Advertisement messages sent by the
 * router. The value should be set to that current diameter of the Internet.
 * The value zero means unspecified (by this router). This parameter can be
 * specified for each interface that are listed in the RADVD_IFNAME_LIST in
 * the form of "<ifparam>=<value>". Each pair of "<ifparam>=<value>" is
 * semicolon separated. Default: "64". E.g. "eth0=64" or "eth0=64;eth1=64".
 */
#ifndef RADVD_IFLIST_ADV_CUR_HOP_LIMIT
#define RADVD_IFLIST_ADV_CUR_HOP_LIMIT       "" 
#endif

/*
 * RADVD_IFLIST_ADV_HA_FLAG - The TRUE(=1) /FALSE(=0) value to be placed in
 * the "Home Agent Flag" flag field in the Router Advertisement. If this flag
 * is set, it indicates that the router in question also serves as a Home
 * Agent. This parameter can be specified for each interface that are listed
 * in the RADVD_IFNAME_LIST in the form of "<ifparam>=<value>". Each pair of
 * "<ifparam>=<value>" is semicolon separated. Default: "0" (=FALSE).  
 * E.g. "eth0=0" or "eth0=0;eth1=0".
 */
#ifndef RADVD_IFLIST_ADV_HA_FLAG
#define RADVD_IFLIST_ADV_HA_FLAG             ""
#endif

/*
 * RADVD_IFLIST_ADV_INTERVAL_OPT - If TRUE(=1), the Router Advertisment
 * daemon will append a Router Advertisement Interval option to the router
 * advertisement. This option is specified in RFC 3775. This parameter can
 * be specified for each interface that are listed in the RADVD_IFNAME_LIST
 * in the form of "<ifparam>=<value>". Each pair of "<ifparam>=<value>" is
 * semicolon separated. Default: "0" (=FALSE). E.g. "eth0=0" or 
 * "eth0=0;eth1=0".
 */
#ifndef RADVD_IFLIST_ADV_INTERVAL_OPT
#define RADVD_IFLIST_ADV_INTERVAL_OPT        ""
#endif

/*
 * RADVD_IFLIST_ADV_HA_OPT - If TRUE(=1), the Router Advertisment daemon will
 * append a home agent information option to its router advertisement. This
 * parameter can be specified for each interface that are listed in the
 * RADVD_IFNAME_LIST in the form of "<ifparam>=<value>". Each pair of
 * "<ifparam>=<value>" is semicolon separated. Default: "0" (=FALSE).
 *  E.g. "eth0=0" or "eth0=0;eth1=0".
 */
#ifndef RADVD_IFLIST_ADV_HA_OPT
#define RADVD_IFLIST_ADV_HA_OPT              ""
#endif

/*
 * RADVD_IFLIST_HA_OPT_LIFETIME - Specifies the Home Agents lifetime in
 * milliseconds. This parameter can be specified for each interface that are
 * listed in the RADVD_IFNAME_LIST in the form of "<ifparam>=<value>". Each
 * pair of "<ifparam>=<value>" is semicolon separated. Default: "1800000"
 * (RADVD_IFLIST_ADV_DEFAULT_LIFETIME). E.g. "eth0=1800000" or 
 * "eth0=1800000;eth1=1800000".
 */
#ifndef RADVD_IFLIST_HA_OPT_LIFETIME
#define RADVD_IFLIST_HA_OPT_LIFETIME         ""
#endif

/*
 * RADVD_IFLIST_HA_OPT_PREFERENCE - Specifies the Home Agents preference.
 * This parameter can be specified for each interface that are listed in the
 * RADVD_IFNAME_LIST in the form of "<ifparam>=<value>". Each pair of
 * "<ifparam>=<value>" is semicolon separated. Default: "0".  E.g. "eth0=0"
 * or "eth0=0;eth1=0".
 */
#ifndef RADVD_IFLIST_HA_OPT_PREFERENCE
#define RADVD_IFLIST_HA_OPT_PREFERENCE       ""
#endif

/*
 * RADVD_IFLIST_MIN_RTR_ADV_INTERVAL - The minimum time allowed (in seconds)
 * between sending unsolicited multicast Router Advertisements from the
 * interface. MUST be no less than 30 milliseconds and no greater than 75%
 * of MaxRtrAdvInterval. The interval limitations are set according to
 * RFC3775. This parameter can be specified for each interface that are
 * listed in the RADVD_IFNAME_LIST in the form of "<ifparam>=<value>". Each
 * pair of "<ifparam>=<value>" is semicolon separated. Default: "200000"
 * (MaxRtrAdvInterval / 3). E.g "eth0=200000" or "eth0=200000;eth1=200000".
 */
#ifndef RADVD_IFLIST_MIN_RTR_ADV_INTERVAL
#define RADVD_IFLIST_MIN_RTR_ADV_INTERVAL    ""
#endif

/*
 * RADVD_IFLIST_MAX_RTR_ADV_INTERVAL - The maximum time (in seconds) allowed
 * between sending unsolicited multicast Router Advertisements from the
 * interface. MUST be no less than 70 milliseconds and no greater than
 * 1800000 seconds. This parameter can be specified for each interface that
 * are listed in the RADVD_IFNAME_LIST in the form of "<ifparam>=<value>".
 * Each pair of "<ifparam>=<value>" is semicolon separated. Default: "600000".
 * E.g. "eth0=600000" or "eth0=600000;eth1=600000".
 */
#ifndef RADVD_IFLIST_MAX_RTR_ADV_INTERVAL
#define RADVD_IFLIST_MAX_RTR_ADV_INTERVAL    ""
#endif

/*
 * RADVD_IFLIST_MIN_DELAY_BETWEEN_RAS - The min delay between solicited RA's.
 * Will default to MinRtrAdvInterval if not specified. This configuration
 * variable is defined in RFC 3775 and can be specified for each interface
 * that are listed in the the RADVD_IFNAME_LIST in the form of
 * "<ifparam>=<value>". Each pair of "<ifparam>=<value>" is semicolon
 * separated. Default: "200000" (MinRtrAdvInterval). E.g. "eth0=200000" or
 * "eth0=200000;eth1=200000".
 */
#ifndef RADVD_IFLIST_MIN_DELAY_BETWEEN_RAS
#define RADVD_IFLIST_MIN_DELAY_BETWEEN_RAS   ""
#endif

/*
 * RADVD_IFLIST_ADV_DEFAULT_LIFETIME - The value (in seconds) to be placed in
 * the Router Lifetime field of Router Advertisements sent from the interface.
 * Must be either zero or between 1000 and 9000000 milliseconds. A value of
 * zero indicates that the router is not to be used as a default router. This
 * parameter can be specified for each interface that are listed in the
 * RADVD_IFNAME_LIST in the form of "<ifparam>=<value>". Each pair of
 * "<ifparam>=<value>" is semicolon separated. Default: 3 * MaxRtrAdvInterval
 * but at least 1000 milliseconds. 
 * E.g. "eth0=1800000" or "eth0=1800000;eth1=1800000".
 */
#ifndef RADVD_IFLIST_ADV_DEFAULT_LIFETIME
#define RADVD_IFLIST_ADV_DEFAULT_LIFETIME    ""
#endif

/*
 * The following prefix configuration parameters configure prefix that is
 * added to the router advertisement message for each interface.
 */

/* 
 * RADVD_IFLIST_ADV_PREFIX - List of prefixes that should be added to router
 * advertisement messages. The elements are an identifier for the prefix, not
 * the actual prefix. This parameter can be specified for each interface that
 * are listed in the RADVD_IFNAME_LIST in the form of "<ifparam>=<value>".
 * Each pair of "<ifparam>=<value>" is semicolon separated. Default: Empty
 * list. E.g."eth0=6to4net" or "eth0=6to4net;eth1=devnet".
 */
#ifndef RADVD_IFLIST_ADV_PREFIX
#define RADVD_IFLIST_ADV_PREFIX            ""
#endif

/*
 * RADVD_IFLIST_PREFIX - Specifies the prefix and prefix length. The prefix
 * length must be between 1 and 127 and must be 64 for Ethernet if it should
 * be used for automatic address configuration. This parameter can be
 * specified for each interface that are listed in the RADVD_IFNAME_LIST in
 * the form of "<ifparam>=<value>". Each pair of "<ifparam>=<value>" is
 * semicolon separated. Default: "" (not specified). E.g. 
 * "eth0.prefix.6to4net=2002:0a01:0264::/64" or 
 * "eth0.prefix.6to4net=2002:0a01:0264::/64;eth1.prefix.devnet=
 * 2002:a01:201:1::/64".
 */
#ifndef RADVD_IFLIST_PREFIX
#define RADVD_IFLIST_PREFIX               ""
#endif

/*
 * RADVD_IFLIST_PREFIX_ADV_ONLINK_FLAG - The value to be placed in the
 * On-Link Flag field in the Prefix Information option. This parameter can
 * be specified for each interface that are listed in the RADVD_IFNAME_LIST
 * in the form of "<ifparam>=<value>". Each pair of "<ifparam>=<value>" is
 * semicolon separated. Default: 1 (TRUE). E.g. "eth0.prefix.6to4net=1" or
 * "eth0.prefix.6to4net=1;eth1.prefix.devnet=1".
 */
#ifndef RADVD_IFLIST_PREFIX_ADV_ONLINK_FLAG
#define RADVD_IFLIST_PREFIX_ADV_ONLINK_FLAG  ""
#endif

/*
 * RADVD_IFLIST_PREFIX_ADV_AUTONOMOUS_FLAG - The value to be placed in the
 * Autonomus Flag field in the Prefix Information option. This parameter can
 * be specified for each interface that are listed in the RADVD_IFNAME_LIST
 * in the form of "<ifparam>=<value>". Each pair of "<ifparam>=<value>" is
 * semicolon separated. Default: 1 (TRUE). E.g. "eth0.prefix.6to4net=1" or
 * "eth0.prefix.6to4net=1;eth1.prefix.devnet=1".
 */
#ifndef RADVD_IFLIST_PREFIX_ADV_AUTONOMOUS_FLAG
#define RADVD_IFLIST_PREFIX_ADV_AUTONOMOUS_FLAG ""
#endif

/*
 * RADVD_IFLIST_PREFIX_ADV_RTR_ADDR_FLAG - The value to be placed in the
 * Router Address flag field in the Prefix Information option. This indicates
 * that the specified prefix is also a complete, global address for the
 * router. This flag is specified in RFC3775 and can be specified for each
 * interface that are listed in the RADVD_IFNAME_LIST in the form of
 * "<ifparam>=<value>". Each pair of "<ifparam>=<value>" is semicolon
 * separated. Default: 0 (FALSE). E.g. "eth0.prefix.6to4net=0" or
 * "eth0.prefix.6to4net=0;eth1.prefix.devnet=0".
 */
#ifndef RADVD_IFLIST_PREFIX_ADV_RTR_ADDR_FLAG
#define RADVD_IFLIST_PREFIX_ADV_RTR_ADDR_FLAG   ""
#endif

/*
 * RADVD_IFLIST_PREFIX_ADV_VALID_LIFETIME - The value to be placed in the
 * Valid Lifetime in the Prefix Information option, in seconds. The
 * designated value of all 1's ( -1) represents infinity. This parameter
 * can be specified for each interface that are listed in the
 * RADVD_IFNAME_LIST in the form of "<ifparam>=<value>". Each pair of
 * "<ifparam>=<value>" is semicolon separated. Default: "-1" (infinite).
 * E.g. "eth0.prefix.6to4net=-1" or 
 * "eth0.prefix.6to4net=-1;eth1.prefix.devnet=-1".
 */
#ifndef RADVD_IFLIST_PREFIX_ADV_VALID_LIFETIME
#define RADVD_IFLIST_PREFIX_ADV_VALID_LIFETIME  ""
#endif

/*
 * RADVD_IFLIST_PREFIX_ADV_PREFERRED_LIFETIME - The value to be placed in
 * the Preferred Lifetime in the Prefix Information option, in seconds.
 * The designated value of all 1' s ( -1) represent s infinity. This
 * parameter can be specified for each interface that are listed in the
 * RADVD_IFNAME_LIST in the form of "<ifparam>=<value>". Each pair of
 * "<ifparam>=<value>" is semicolon separated. Default: "-1" (infinite).
 * E.g. "eth0.prefix.6to4net=-1" or 
 * "eth0.prefix.6to4net=-1;eth1.prefix.devnet=-1".
 */
#ifndef RADVD_IFLIST_PREFIX_ADV_PREFERRED_LIFETIME
#define RADVD_IFLIST_PREFIX_ADV_PREFERRED_LIFETIME ""
#endif
#endif /* INCLUDE_IPRADVD */

#ifdef INCLUDE_IPTELNETS
#ifndef IPCOM_TELNET_PORT
#define IPCOM_TELNET_PORT      "23"      /* IPCCOM telnet port */
#endif
#ifndef IPCOM_TELNET_AUTH_ENABLED
#define IPCOM_TELNET_AUTH_ENABLED   "0"  /* telnet authentication */
#endif
#endif /* INCLUDE_IPTELNETS */

#ifdef INCLUDE_IPTFTP_COMMON
#ifndef TFTPS_DIRS
#define TFTPS_DIRS   IPCOM_FILE_ROOT "tftpdir"   /* TFTP server working dir */
#endif
#ifndef TFTPS_RETRANSMIT_TIMEOUT
#define TFTPS_RETRANSMIT_TIMEOUT  "5"  /* TFTP retransmit timeout (secs) */
#endif
#ifndef TFTPS_RETRIES
#define TFTPS_RETRIES             "2"  /* TFTP number of retries */
#endif
#endif /* INCLUDE_IPTFTP_COMMON */

#ifdef INCLUDE_IPSNTP_COMMON
#ifndef SNTP_LISTENING_PORT
#define SNTP_LISTENING_PORT        "123"  /* SNTP port */
#endif
#endif /* INCLUDE_IPSNTP_COMMON */

#ifdef INCLUDE_IPSNTPS
#ifndef SNTPS_STRATUM
#define SNTPS_STRATUM   "9"         /* SNTP server stratum */
#endif
#ifndef SNTPS_PRECISION
#define SNTPS_PRECISION   "-6"     /* SNTP server precision */
#endif
#ifndef SNTPS_MULTICAST_INTERVAL
#define SNTPS_MULTICAST_INTERVAL "3600"  /* SNTP multicast mode send interval*/
#endif
#ifndef SNTPS_MULTICAST_TTL
#define SNTPS_MULTICAST_TTL      "1"
#endif
#ifndef SNTPS_IPV4_MULTICAST_ADDR
#define SNTPS_IPV4_MULTICAST_ADDR  "10.1.255.255"  /* SNTP mcast IPv4 address */
#endif
#ifndef SNTPS_IPV6_MULTICAST_ADDR
#define SNTPS_IPV6_MULTICAST_ADDR  "FF05::1"  /* SNTP mcast IPv6 address */
#endif
#endif /* INCLUDE_IPSNTPS */

#ifdef INCLUDE_IPSNTPC
#ifndef SNTPC_POLL_INTERVAL
#define SNTPC_POLL_INTERVAL       "1024"  /* poll interval */
#endif
#ifndef SNTPC_POLL_COUNT
#define SNTPC_POLL_COUNT          "3"     /* Number of retransmissions */
#endif
#ifndef SNTPC_POLL_TIMEOUT
#define SNTPC_POLL_TIMEOUT        "2"     /* Seconds between retransmissions */
#endif
#ifndef SNTPC_PRIMARY_IPV4_ADDR
#define SNTPC_PRIMARY_IPV4_ADDR   "10.1.2.90"  /* Primary server IPv4 address */
#endif
#ifndef SNTPC_BACKUP_IPV4_ADDR
#define SNTPC_BACKUP_IPV4_ADDR    "10.1.2.40"  /* Backup server IPv4 address */
#endif
#ifndef SNTPC_MULTICAST_MODE_IF
#define SNTPC_MULTICAST_MODE_IF   ""
#endif
#ifndef SNTPC_MULTICAST_GROUP_ADDR
#define SNTPC_MULTICAST_GROUP_ADDR  "224.0.1.1"
#endif
#ifndef SNTPC_PRIMARY_IPV6_ADDR
#define SNTPC_PRIMARY_IPV6_ADDR   "2001::90"   /* Primary server IPv6 address */
#endif
#ifndef SNTPC_BACKUP_IPV6_ADDR
#define SNTPC_BACKUP_IPV6_ADDR    "2001::28"   /* Backup server IPv6 address */
#endif
#ifndef SNTPC_MULTICAST_MODE_IPV6_IF
#define SNTPC_MULTICAST_MODE_IPV6_IF    ""
#endif
#ifndef SNTPC_MULTICAST_GROUP_IPV6_ADDR
#define SNTPC_MULTICAST_GROUP_IPV6_ADDR "ff05::101"
#endif
#endif /* INCLUDE_IPSNTPC */

#ifdef INCLUDE_IPRIP
#ifndef IPRIP_UPDATE_INTERVAL
#define IPRIP_UPDATE_INTERVAL     "20"   /* RIP Update Interval */
#endif
#ifndef IPRIP_UPDATE_DELTA
#define IPRIP_UPDATE_DELTA        "20"   /* RIP Update delta Interval */
#endif
#ifndef IPRIP_EXPIRE_INTERVAL
#define IPRIP_EXPIRE_INTERVAL     "180"  /* RIP Expire Interval */
#endif
#ifndef IPRIP_GARBAGE_INTERVAL
#define IPRIP_GARBAGE_INTERVAL    "120"  /* RIP Garbage Interval */
#endif
#ifndef IPRIP_FLASH_DELAY
#define IPRIP_FLASH_DELAY         "3"  /* RIP Flash Interval */
#endif
#ifndef IPRIP_AUTH_ENABLED
#define IPRIP_AUTH_ENABLED        "0"    /* Authenticate RIP requests */
#endif
#endif /* INCLUDE_IPRIP */

#ifdef INCLUDE_IPRIPNG
#ifndef IPRIPNG_PRIORITY
#define IPRIPNG_PRIORITY	  "0"
#endif
#ifndef IPRIPNG_OPTIONS_STRING
#define IPRIPNG_OPTIONS_STRING	  ""
#endif
#endif /* INCLUDE_IPRIPNG */
/*
 * RIP interface configuration:
 * Specifies parameters to configure RIP on an interface. The syntax must
 * begin with the name of the interface followed by options and their values
 * if required. The wildcard interface name any is used to match any 
 * interface, effectively starting RIP on all interfaces. If a specific entry
 * is find, that entry is used instead of the any entry, e.g. eth0 is chosen
 * before any.
 * Example #1 - "all broadcast input-multicast"
 * Example #2 - "eth0 broadcast input-multicast input=v2"
 * Example #3 - "ppp0 silent metric=2 input=no" 
 *
 * The following options are available to configure RIP on an interface:
 * (1) broadcast - Use subnet broadcast output.
 * (2) multicast - Use multicast output (224.0.0.9). 
 * (3) silent - Do not output RIP requests/responses. 
 * (4) auth-md5=<password> - Enable GateD style md5 authentication with
 * <password>.
 * (5) auth-simple=<password] - Enable simple authentication with <password>.
 * (6) input=<no|v1|v2> - Change input mode (no,v1 or v2 only). No means that
 * no input RIP packets are parsed.
 * (7) input-multicast - Accept multicast input (224.0.0.9).
 * (8) metric=<num> - Change default metric from 1 to <num>.
 * (9) version-1 - Enable RIPv1 (default is RIPv2)
 * (10) nopoison - Use simple Split Horizon instead of poisonous. 
 *
 * Note - Only use one of broadcast, multicast or silent options.
 */

#ifdef INCLUDE_IPRIP_IFCONFIG_1
#ifndef IPRIP_IFCONFIG_1
#define IPRIP_IFCONFIG_1                    "all broadcast input-multicast"
#endif
#endif /* INCLUDE_IPRIP_IFCONFIG_1 */

#ifdef INCLUDE_IPRIP_IFCONFIG_2
#ifndef IPRIP_IFCONFIG_2
#define IPRIP_IFCONFIG_2                    NULL
#endif
#endif /* INCLUDE_IPRIP_IFCONFIG_2 */

#ifdef INCLUDE_IPRIP_IFCONFIG_3
#ifndef IPRIP_IFCONFIG_2
#define IPRIP_IFCONFIG_2                    NULL
#endif
#endif /* INCLUDE_IPRIP_IFCONFIG_3 */

#ifdef INCLUDE_IPRIP_IFCONFIG_4
#ifndef IPRIP_IFCONFIG_2
#define IPRIP_IFCONFIG_2                    NULL
#endif
#endif /* INCLUDE_IPRIP_IFCONFIG_4 */

/*
 * RIP static route configuration:
 * Configures initial static RIP routes which may be useful before the system
 * has configured the network using RIP. The syntax is:
 * "dst=<a.b.c.d> mask=<a.b.c.d> gw=<a.b.c.d> metric=<num>".
 *
 * Example: "dst=136.35.0.0 mask=255.255.0.0 gw=10.1.2.100 metric=3"
 */
#ifdef INCLUDE_IPRIP_STATIC_ROUTE_1
#ifndef IPRIP_STATIC_ROUTE_1
#define IPRIP_STATIC_ROUTE_1                NULL
#endif
#endif /* INCLUDE_IPRIP_STATIC_ROUTE_1 */

#ifdef INCLUDE_IPRIP_STATIC_ROUTE_2
#ifndef IPRIP_STATIC_ROUTE_2
#define IPRIP_STATIC_ROUTE_2                NULL
#endif
#endif /* INCLUDE_IPRIP_STATIC_ROUTE_2 */

#ifdef INCLUDE_IPRIP_STATIC_ROUTE_3
#ifndef IPRIP_STATIC_ROUTE_3
#define IPRIP_STATIC_ROUTE_3                NULL
#endif
#endif /* INCLUDE_IPRIP_STATIC_ROUTE_3 */

/****************************************************
 *     SECURITY APPLICATIONS CONFIGURATION          *
 ****************************************************/

#ifdef INCLUDE_IP_SECURITY
#ifndef IPIPSEC_LARVAL_SA_TMO_SECONDS
#define IPIPSEC_LARVAL_SA_TMO_SECONDS "600"  /* Default larval SA (created by getspi) timeout in seconds.  */
#endif

#ifndef IPIPSEC_CONF_ENABLE_OUTFILTER
#define IPIPSEC_CONF_ENABLE_OUTFILTER 0 /* Enable output filter */
#endif

#ifndef IPIPSEC_CONF_ENABLE_INFILTER
#define IPIPSEC_CONF_ENABLE_INFILTER  1 /* Enable input filter */
#endif

#ifndef IPIPSEC_CONF_ESP_ENABLE
#define IPIPSEC_CONF_ESP_ENABLE  		1 /* Enable ESP */
#endif

#ifndef IPIPSEC_CONF_AH_ENABLE
#define IPIPSEC_CONF_AH_ENABLE  		1 /* Enable AH */
#endif

#ifndef IPIPSEC_CONF_IPIP_ALLOW
#define IPIPSEC_CONF_IPIP_ALLOW  		1 /* WARNING, IPIP IS A SECURITY RISK! See programmer's guide. */
#endif

#ifndef INCLUDE_IPSECCTRL_CMD
#define INCLUDE_IPSECCTRL_CMD
#endif

#ifdef INCLUDE_KEYADM_CMD
#define INCLUDE_KEYADM_CMD
#endif
#endif /* INCLUDE_IP_SECURITY */

#ifdef INCLUDE_INTERNET_KEY_EXCHANGE

/* 
 * ISAKMP socket buffer size (in bytes), this is the largest ISAKMP message 
 * the IKE daemon can send and receive. 
 */
#ifndef IPIKE_SOCK_BUF_SIZE
#define IPIKE_SOCK_BUF_SIZE          "10000"
#endif

#ifndef IPIKE_MAX_EXCHANGES
#define IPIKE_MAX_EXCHANGES          "32"
#endif

/* 
 * Setting this to 1 will configure the system to use IKEv1 only. Setting it 
 * to 2 will enable IKEv1 and IKEv2. 
 */
#ifndef IPIKE_VERSION
#define IPIKE_VERSION          2
#endif

/* The max number of clients connected to the ike daemon at any one time. */
#ifndef IPIKE_MAX_CLIENTS
#define IPIKE_MAX_CLIENTS          8 
#endif

/* 
 * Specifies the UDP port on which to listen for ISAKMP packets. Should be 
 * 500 in most cases. 
 */
#ifndef IPIKE_PORT
#define IPIKE_PORT          "500"
#endif

/* 
 * Specifies the UDP port on which to listen for ISAKMP NATTraversal packets. 
 * Must be 4500 to interoperate with other clients. 
 */
#ifndef IPIKE_NAT_PORT
#define IPIKE_NAT_PORT          "4500"
#endif

/* 
 * Specifies the number of seconds between keep alive packets when 
 * NAT-Traversal is used. 
 */
#ifndef IPIKE_NAT_KEEPALIVE
#define IPIKE_NAT_KEEPALIVE          "20"
#endif

/* 
 * The time (in seconds) the IKE daemon will wait before doing the first 
 * resend of a packet. IKE will wait two times this value between the first 
 * and second resend, three times the value until the third resend, etc. 
 */
#ifndef IPIKE_RESEND_DELAY
#define IPIKE_RESEND_DELAY          "5"
#endif

#ifndef IPIKE_USE_AGGRESSIVE_MODE
#define IPIKE_USE_AGGRESSIVE_MODE    1 /* Enables IKE aggressive mode. */
#endif

/* 
 * The name of the file that will be used to configure the IKE daemon on 
 * startup. 
 */
#ifndef IPIKE_CONFIG_FILE_NAME
#define IPIKE_CONFIG_FILE_NAME          "ike.cfg"
#endif

/* The path the IKE daemon will use to locate the configuration file. */
#ifndef IPIKE_CONFIG_FILE_PATH
#define IPIKE_CONFIG_FILE_PATH          "/ram/"
#endif

/* 
 * The size (in bytes) of the nonce (random number) that the IKE daemon will 
 * generate during phase 1 exchange. Must be between 8 and 256 (inclusive) 
 * bytes. 
 */
#ifndef IPIKE_NONCE_SIZE
#define IPIKE_NONCE_SIZE          "64"
#endif

/* 
 * IPIKE will create a default configuration file for acting as a responder 
 * using preshared secret if this variable is set to 1 and there is no 
 * configuration file with the name specified by ipike.filename present. 
 */
#ifndef IPIKE_CREATE_DEFAULT_CONFIG_FILE
#define IPIKE_CREATE_DEFAULT_CONFIG_FILE     "1"
#endif

/*
 * Set this to 1 to allow IPIKE to create duplicate IKE SAs.
 */
#ifndef IPIKE_ALLOW_DUPLICATE_SA
#define IPIKE_ALLOW_DUPLICATE_SA     "0"
#endif

/* 
 * This flag is used to indicate to the IKE daemon to flush the keys at 
 * startup. 
 */
#ifndef IPIKE_FLUSH_STARTUP
#define IPIKE_FLUSH_STARTUP          "0"
#endif

/* 
 * This flag is used to indicate to the IKE daemon to flush the keys at 
 * shutdown. 
 */
#ifndef IPIKE_FLUSH_SHUTDOWN
#define IPIKE_FLUSH_SHUTDOWN          "0"
#endif

/* 
 * This flag is used to indicate to the IKE daemon to flush the keys when 
 * reconfigured. 
 */
#ifndef IPIKE_FLUSH_RECONFIGURE
#define IPIKE_FLUSH_RECONFIGURE         "0"
#endif

/*
 * Specifies ip network for IPv4 dynamic address pool (responder).
 */
#ifndef IPIKE_DYNAMIC_ADDRESS_POOL_NETWORK4
#define IPIKE_DYNAMIC_ADDRESS_POOL_NETWORK4  "10.1.111.1/24"
#endif

/*
 * Specifies dns ip address for IPv4 dynamic address pool (responder).
 */
#ifndef IPIKE_DYNAMIC_ADDRESS_POOL_DNS4
#define IPIKE_DYNAMIC_ADDRESS_POOL_DNS4      "10.1.1.1"
#endif

/*
 * Specifies expire time for IPv4 dynamic address pool (responder).
 */
#ifndef IPIKE_DYNAMIC_ADDRESS_POOL_EXPIRE4
#define IPIKE_DYNAMIC_ADDRESS_POOL_EXPIRE4     "3600"
#endif

/*
 * Specifies dynamic address mode for IKE version 1 protocol (initiator).
 */
#ifndef IPIKE_DYNAMIC_ADDRESS_MODE
#define IPIKE_DYNAMIC_ADDRESS_MODE         "1"
#endif
#endif /* INCLUDE_INTERNET_KEY_EXCHANGE */

#ifdef INCLUDE_SSH

/* 
 * The TCP port the IPSSH daemon listens on for incoming connections. 
 * Additional ports are specified using ipssh.port2, ipssh.port3, etc. in 
 * the ipssh_config.c file. Note that ipssh.port1 is not a valid system 
 * variable. 
 */
#ifndef SSH_PORT
#define SSH_PORT          "22" 
#endif

#ifndef SSH_BIND_ADDR
#define SSH_BIND_ADDR   "0.0.0.0" /* The address the IPSSH daemon binds to. */
#endif

/* 
 * The number of seconds before an idle client is disconnected. 0 means 
 * infinite timeout. 
 */
#ifndef SSH_IDLE_TIMEOUT
#define SSH_IDLE_TIMEOUT        "0" 
#endif

#ifndef SSH_MAX_CLIENTS
#define SSH_MAX_CLIENTS  "20" /* The maximum number of simultaneous clients. */
#endif

/* 
 * The maximum number of shells that can be started per connection. The IPCOM 
 * SHELL only supports one shell per ssh client. 
 */
#ifndef SSH_MAX_SHELLS
#define SSH_MAX_SHELLS          "1"
#endif

/* 
 * Controls whether password authentication can be used. 1 means on, 0 means 
 * off. 
 */
#ifndef SSH_AUTH_PW_ALLOWED
#define SSH_AUTH_PW_ALLOWED          "1"
#endif

/* 
 * Controls whether password authentication must be performed before a client 
 * is considered logged in. 1 means on, 0 means off. 
 */
#ifndef SSH_AUTH_PW_REQUIRED
#define SSH_AUTH_PW_REQUIRED          "0"
#endif

/* 
 * Controls whether public key authentication can be used. 1 means on, 0 means 
 * off. 
 */
#ifndef SSH_AUTH_PUB_KEY_ALLOWED
#define SSH_AUTH_PUB_KEY_ALLOWED          "1"
#endif

/* 
 * Controls whether public key authentication must be performed before a 
 * client is considered logged in. 1 means on, 0 means off. 
 */
#ifndef SSH_AUTH_PUB_KEY_REQUIRED
#define SSH_AUTH_PUB_KEY_REQUIRED          "0"
#endif

/* 
 * Controls whether public key authentication shall be performed first, if both
 * password and public key authentications are required. 1 means that public 
 * key authentication must performed first, 0 means that the client can 
 * choose in which order to authenticate. 
 */
#ifndef SSH_AUTH_PUB_KEY_FIRST
#define SSH_AUTH_PUB_KEY_FIRST          "1"
#endif

/* 
 * Controls the maximum number of authentication attempts before a client is 
 * disconnected. 
 */
#ifndef SSH_AUTH_MAX_FAIL
#define SSH_AUTH_MAX_FAIL          "3"
#endif

/* 
 * Public key operations during the handshake are CPU intensive. It is a good 
 * idea to run these tasks at a lower priority. Possible values are 1-7. 
 */
#ifndef SSH_SPAWN_TASK_PRIORITY
#define SSH_SPAWN_TASK_PRIORITY         2
#endif

/* Enables the use of the AES cipher. 1 means on, 0 means off. */
#ifndef SSH_USE_AES
#define SSH_USE_AES         "1"
#endif

/* Enables the use of the DES cipher. 1 means on, 0 means off. */
#ifndef SSH_USE_DES
#define SSH_USE_DES         "1"
#endif

/* Enables the use of the 3DES cipher. 1 means on, 0 means off. */
#ifndef SSH_USE_3DES
#define SSH_USE_3DES        "1"
#endif

/* Enables the use of the Blowfish cipher. 1 means on, 0 means off. */
#ifndef SSH_USE_BLOWFISH
#define SSH_USE_BLOWFISH    "1"
#endif

/* Enables the use of the CAST cipher. 1 means on, 0 means off. */
#ifndef SSH_USE_CAST
#define SSH_USE_CAST        "1"
#endif

/* Enables the use of the RC4 cipher. 1 means on, 0 means off. */
#ifndef SSH_USE_ARCFOUR
#define SSH_USE_ARCFOUR     "1"
#endif

/* Enables the use of no cipher. 1 means on, 0 means off. */
#ifndef SSH_USE_NO_CIPHER
#define SSH_USE_NO_CIPHER   "0"
#endif

/* Enables the use of SHA1 for authentication. 1 means on, 0 means off. */
#ifndef SSH_USE_SHA1
#define SSH_USE_SHA1        "1"
#endif

/* Enables the use of SHA1-96 for authentication. 1 means on, 0 means off. */
#ifndef SSH_USE_SHA1_96
#define SSH_USE_SHA1_96     "1"
#endif

/* Enables the use of MD5 for authentication.  1 means on, 0 means off. */
#ifndef SSH_USE_MD5
#define SSH_USE_MD5         "1"
#endif

/* Enables the use of MD5-96 for authentication.  1 means on, 0 means off. */
#ifndef SSH_USE_MD5_96
#define SSH_USE_MD5_96      "1"
#endif

/* Enables the use of no authentication.  1 means on, 0 means off. */
#ifndef SSH_USE_NO_MAC
#define SSH_USE_NO_MAC      "0"
#endif

/* Enables SSH v1.  1 means on, 0 means off. */
#ifndef SSH_USE_V1
#define SSH_USE_V1      "1"
#endif

/* Enables SSH v2.  1 means on, 0 means off. */
#ifndef SSH_USE_V2
#define SSH_USE_V2      "1"
#endif

/* Enables SSH Terminal Service.  1 means on, 0 means off. */
#ifndef SSH_USE_TERMINAL
#define SSH_USE_TERMINAL    "1"
#endif

/* Enables SSH SFTP Service.  1 means on, 0 means off. */
#ifndef SSH_USE_SFTP
#define SSH_USE_SFTP    "1"
#endif

/* Enables SSH Port Forwarding.  1 means on, 0 means off. */
#ifndef SSH_USE_PORT_FWD
#define SSH_USE_PORT_FWD    "1"
#endif

/* Name of DSA Key File stored at IPCOM_FILE_ROOT */
#ifndef SSH_DSA_KEY
#define SSH_DSA_KEY     "dsa_key.pem"
#endif

/* Name of RSA Key File stored at IPCOM_FILE_ROOT */
#ifndef SSH_RSA_KEY
#define SSH_RSA_KEY     "rsa_key.pem"
#endif

#endif /* INCLUDE_SSH */

/******************************************************************************/
/*                                                                            */
/*           COREIP DUAL STACK NETWORK APPLICATIONS CONFIGURATION             */
/*                                                                            */
/******************************************************************************/

#ifdef INCLUDE_CORE_NFS_SERVER
#ifndef NFS_MAXPATH
#define NFS_MAXPATH         255
#endif
#ifndef NFS_MAXFILENAME
#define NFS_MAXFILENAME     40
#endif
#endif /* INCLUDE_CORE_NFS_SERVER */

#ifdef INCLUDE_NFS2_CLIENT
#ifndef NFS2_CLIENT_CACHE_DEFAULT_NUM_LINES
#define NFS2_CLIENT_CACHE_DEFAULT_NUM_LINES  16
#endif
#ifndef NFS2_CLIENT_CACHE_DEFAULT_OPTIONS
#define NFS2_CLIENT_CACHE_DEFAULT_OPTIONS    0
#endif
#ifndef NFS2_CLIENT_CACHE_DEFAULT_LINE_SIZE
#define NFS2_CLIENT_CACHE_DEFAULT_LINE_SIZE  16384
#endif
#endif /* INCLUDE_NFS2_CLIENT */

#ifdef INCLUDE_NFS3_CLIENT
#ifndef NFS3_CLIENT_CACHE_DEFAULT_NUM_LINES
#define NFS3_CLIENT_CACHE_DEFAULT_NUM_LINES  16
#endif
#ifndef NFS3_CLIENT_CACHE_DEFAULT_OPTIONS
#define NFS3_CLIENT_CACHE_DEFAULT_OPTIONS    0
#endif
#ifndef NFS3_CLIENT_CACHE_DEFAULT_LINE_SIZE
#define NFS3_CLIENT_CACHE_DEFAULT_LINE_SIZE  16384
#endif
#endif /* INCLUDE_NFS3_CLIENT */

#ifdef INCLUDE_BPF
#ifndef BPF_BUFSIZE_CFG
#define BPF_BUFSIZE_CFG 4096
#endif
#endif /* INCLUDE_BPF */

#ifdef INCLUDE_FASTPATH
/* 
 * Define the FF_IPV4_INIT_RTN and FF_IPV6_INIT_RTN  macros to
 * your fastpath module init routine. If you don't want to initialize
 * the module, set it to -1. Set the FF_IPV4_FIB_DISP_TBL and
 * FF_IPV6_FIB_DISP_TBL macros to the dispatch table for the FIB you want
 * to use with the fastpath module. You must define a valid dispatch
 * table if you set the FF_IPVX_INIT_RTN macro to other than -1
 */
#ifndef FF_IPV4_INIT_RTN
#define FF_IPV4_INIT_RTN ipFFInit
#endif
#ifndef FF_IPV6_INIT_RTN
#define FF_IPV6_INIT_RTN ipFFInit 
#endif
#ifndef FF_IPV4_FIB_DISP_TBL
#define FF_IPV4_FIB_DISP_TBL ptRibDispatchTable
#endif
#ifndef FF_IPV6_FIB_DISP_TBL
#define FF_IPV6_FIB_DISP_TBL avlRibDispatchTable
#endif
#endif /* INCLUDE_FASTPATH */

#ifdef INCLUDE_APPL_LOG_UTIL
#ifndef APPL_LOG_FD_CFG
#define APPL_LOG_FD_CFG         STD_OUT
#endif
#ifndef APPL_LOG_LEVEL_CFG
#define APPL_LOG_LEVEL_CFG      (_LOG_DEC_DFLT | _LOG_LVL_DFLT)
#endif
#ifndef APPL_KERN_LOG_FD_CFG
#define APPL_KERN_LOG_FD_CFG    KERN_LOG_FD
#endif
#ifndef APPL_KERN_LOG_LEVEL_CFG
#define APPL_KERN_LOG_LEVEL_CFG (_LOG_DEC_DFLT | _LOG_LVL_DFLT)
#endif
#endif /* INCLUDE_APPL_LOG_UTIL */

#ifdef INCLUDE_FTP
#ifndef FTP_TRANSIENT_MAX_RETRY_COUNT
#define FTP_TRANSIENT_MAX_RETRY_COUNT   100
#endif
#ifndef FTP_TRANSIENT_RETRY_INTERVAL
#define FTP_TRANSIENT_RETRY_INTERVAL    0
#endif
#ifndef FTP_TRANSIENT_FATAL
#define FTP_TRANSIENT_FATAL             ftpTransientFatal
#endif
#ifndef FTP_DEBUG_OPTIONS
#define FTP_DEBUG_OPTIONS               0
#endif
#ifndef FTP_TIMEOUT
#define FTP_TIMEOUT                     0
#endif
#endif /* INCLUDE_FTP */

/******************************************************************************/
/*                                                                            */
/*                EXCLUDED NETWORK STACK CONFIGURATION PARAMETERS             */
/*                                                                            */
/******************************************************************************/
#if FALSE

#ifdef INCLUDE_IFCONFIG
#ifndef IFCONFIG_ENTRY_1
#define IFCONFIG_ENTRY_1  NULL
#endif
#ifndef IFCONFIG_ENTRY_2
#define IFCONFIG_ENTRY_2  NULL
#endif
#ifndef IFCONFIG_ENTRY_3
#define IFCONFIG_ENTRY_3  NULL
#endif
#ifndef IFCONFIG_ENTRY_4
#define IFCONFIG_ENTRY_4  NULL
#endif
#ifndef IFCONFIG_ENTRY_5
#define IFCONFIG_ENTRY_5  NULL
#endif
#ifndef IFCONFIG_ENTRY_6
#define IFCONFIG_ENTRY_6  NULL
#endif
#ifndef IFCONFIG_ENTRY_7
#define IFCONFIG_ENTRY_7  NULL
#endif
#ifndef IFCONFIG_ENTRY_8
#define IFCONFIG_ENTRY_8  NULL
#endif
#ifndef IFCONFIG_ENTRY_9
#define IFCONFIG_ENTRY_9  NULL
#endif
#ifndef IFCONFIG_ENTRY_10
#define IFCONFIG_ENTRY_10 NULL
#endif
#endif /* INCLUDE_IFCONFIG */

#ifdef INCLUDE_ROUTECMD
#ifndef ROUTE_ENTRY_1
#define ROUTE_ENTRY_1 NULL
#endif
#ifndef ROUTE_ENTRY_2
#define ROUTE_ENTRY_2 NULL
#endif
#ifndef ROUTE_ENTRY_3
#define ROUTE_ENTRY_3 NULL
#endif
#ifndef ROUTE_ENTRY_4
#define ROUTE_ENTRY_4 NULL
#endif
#ifndef ROUTE_ENTRY_5
#define ROUTE_ENTRY_5 NULL
#endif
#endif /* INCLUDE_ROUTECMD */

#ifdef INCLUDE_ARP_API
#ifndef ARP_ENTRY_1_HOST
#define ARP_ENTRY_1_HOST NULL
#endif
#ifndef ARP_ENTRY_1_ADDR
#define ARP_ENTRY_1_ADDR NULL
#endif
#ifndef ARP_ENTRY_2_HOST
#define ARP_ENTRY_2_HOST NULL
#endif
#ifndef ARP_ENTRY_2_ADDR
#define ARP_ENTRY_2_ADDR NULL
#endif
#ifndef ARP_ENTRY_3_HOST
#define ARP_ENTRY_3_HOST NULL
#endif
#ifndef ARP_ENTRY_3_ADDR
#define ARP_ENTRY_3_ADDR NULL
#endif
#ifndef ARP_ENTRY_4_HOST
#define ARP_ENTRY_4_HOST NULL
#endif
#ifndef ARP_ENTRY_4_ADDR
#define ARP_ENTRY_4_ADDR NULL
#endif
#ifndef ARP_ENTRY_5_HOST
#define ARP_ENTRY_5_HOST NULL
#endif
#ifndef ARP_ENTRY_5_ADDR
#define ARP_ENTRY_5_ADDR NULL
#endif
#endif /* INCLUDE_ARP_API */

#endif /* FALSE */

#endif /* INCconfigNetParamsh */
