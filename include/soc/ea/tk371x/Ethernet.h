/*
 * $Id: Ethernet.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     Ethernet.h
 * Purpose: 
 *
 */
 
#ifndef _SOC_EA_ETHERNET_H
#define _SOC_EA_ETHERNET_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkConfig.h>


/* pack structures */
#if defined(UNIX) || defined(VXWORKS) || defined(LINUX)
#pragma pack (1)
#endif

#ifdef TK_NEED_PKT_HEAD     
    #define TK_PK_HEAD_LEN      0
#endif

#define EthMinFrameSize             64
#define EthMinFrameSizeWithoutCrc   (EthMinFrameSize - 4)

/* Ethernet protocol type codes */
typedef enum {
    EthertypeArp        = 0x0806,
    EthertypeEapol      = 0x888E,
    EthertypeEapolOld   = 0x8180,
    EthertypeLoopback   = 0x9000,
    EthertypeMpcp       = 0x8808,
    EthertypeOam        = 0x8809,
    EthertypePppoeDisc  = 0x8863,
    EthertypePppoeSess  = 0x8864,
    EthertypeRarp       = 0x8035,
    EthertypeVlan       = 0x8100,
    EthertypeIp         = 0x0800,
} Ethertype;


/* MAC address */
typedef union {
    uint8   byte[6];
    uint16  word[3];
    uint8   u8[6];
} PACK MacAddr;

/* Basic Ethernet frame format */
typedef struct {
    MacAddr da;
    MacAddr sa;
#ifdef TK_NEED_PKT_HEAD
    uint8   pktDataHead[TK_PK_HEAD_LEN];
#endif
#ifdef TK_NEED_MGNT_VLAN
    uint16  vlanType;
    uint16  vlanInfo;
#endif
    uint16  type;
} PACK EthernetFrame;

/* VLAN tag */
typedef uint16 VlanTag;

/* VLAN header */
typedef struct {
    uint16  type;       /* EthertypeVlan */
    VlanTag tag;        /* priority + CFI + VID */
} PACK EthernetVlanData;


typedef struct {
    MacAddr Dst;
    MacAddr Src;
    uint16  VlanType;
    uint16  Vlan;
    uint16  Type;
} PACK VlanTaggedEthernetFrame;

#if defined(UNIX) || defined(VXWORKS) || defined(LINUX)
#pragma pack()
#endif

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_ETHERNET_H */
