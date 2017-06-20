/*
 * $Id$ 
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __NEMO_HEADER_H__
#define __NEMO_HEADER_H__

#include "pub/nd_hw.h"
#include "nd_util.h"
#include "nd_registers.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define AG_ND_PROTO_VLAN_TAG_MAX       2
#define AG_ND_PROTO_MPLS_LABEL_MAX     2

#define AG_ND_PROTO_ETH_IP4            0x0800
#define AG_ND_PROTO_ETH_IP6            0x86DD
#define AG_ND_PROTO_ETH_MPLS           0x8847 /* unicast */
#define AG_ND_PROTO_ETH_CES            0x88d8
#define AG_ND_PROTO_ETH_VLAN           0x8100
#define AG_ND_PROTO_ETH_PTP            0x88F7

#define AG_ND_PROTO_VLAN_CFI           0x0000

#define AG_ND_PROTO_IP4_VERSION        0x0004
#define AG_ND_PROTO_IP4_TOS            0x0000
#define AG_ND_PROTO_IP4_ID             0x0000
#define AG_ND_PROTO_IP4_RSV            0x0000
#define AG_ND_PROTO_IP4_DONT_FRAG      0x0001
#define AG_ND_PROTO_IP4_MORE_FRAG      0x0000
#define AG_ND_PROTO_IP4_FRAG_OFFSET    0x0000
#define AG_ND_PROTO_IP4_FRAG_TTL       0x0002
#define AG_ND_PROTO_IP4_PROTO          0x0011 /* UDP */
#define AG_ND_PROTO_IP4_SRC            0x0000 
#define AG_ND_PROTO_IP4_DST            0x0000 
#define AG_ND_PROTO_IP4_HDR_LEN        0x0014
/*ORI*/
/*add protocol to l2tpv3 */
#define AG_ND_PROTO_IP4_L2TPV3          0x0073 /* L2TPV3 */

#define AG_ND_PROTO_IP6_VERSION        0x0006
#define AG_ND_PROTO_IP6_TRAFFIC_CLS    0x0000
#define AG_ND_PROTO_IP6_FLOW_LABEL     0x0000
#define AG_ND_PROTO_IP6_NEXT_HEADER    0x0011 /* UDP */
#define AG_ND_PROTO_IP6_HOP_LIMIT      0x0002

#define AG_ND_PROTO_UDP_SRC            0x0000 
#define AG_ND_PROTO_UDP_DST            0x0000 
#define AG_ND_PROTO_UDP_HDR_LEN        0x0008

#define AG_ND_PROTO_MPLS_LABEL         0x0000
#define AG_ND_PROTO_MPLS_EXP           0x0000
#define AG_ND_PROTO_MPLS_TTL           0x0002
#define AG_ND_PROTO_MPLS_BOS           0x0000

#define AG_ND_PROTO_VCLABEL_LABEL      0x0000
#define AG_ND_PROTO_VCLABEL_EXP        0x0000
#define AG_ND_PROTO_VCLABEL_TTL        0x0002
#define AG_ND_PROTO_VCLABEL_BOS        0x0001

#define AG_ND_PROTO_RTP_VERSION        0x0002 
#define AG_ND_PROTO_RTP_PADDING        0x0000 
#define AG_ND_PROTO_RTP_EXTENTION      0x0000 
#define AG_ND_PROTO_RTP_CSRC_COUNT     0x0000 
#define AG_ND_PROTO_RTP_MARKER         0x0000 
#define AG_ND_PROTO_RTP_PT             0x0000 
#define AG_ND_PROTO_RTP_SQN            0x0000 
#define AG_ND_PROTO_RTP_TIMESTAMP      0x0000 
#define AG_ND_PROTO_RTP_SSRC           0x0000
#define AG_ND_PROTO_RTP_HDR_LEN        0x000c
#define AG_ND_PROTO_RTP_TIMESTAMP_LEN  0x000a 
#define AG_ND_PROTO_RTP_PORT_LEN       0x000a 

#define AG_ND_PROTO_CES_SQN            0x0000
#define AG_ND_PROTO_CES_HDR_LEN        0x0004

#define AG_ND_PROTO_PTP_MESSAGE_TYPE   0x0000
#define AG_ND_PROTO_PTP_RESERVED       0x0000
#define AG_ND_PROTO_PTP_SEQUENCE_ID    0x0000
#define AG_ND_PROTO_PTP_CONTROL        0x0000
#define AG_ND_PROTO_PTP_LOG_MEAN       0x007f
#define AG_ND_PROTO_PTP_TIMESTAMP      0x0000
#define AG_ND_PROTO_PTP_TIMESTAMP_LEN  0x000a
#define AG_ND_PROTO_PTP_DST_PORT_LEN   0x000a
#define AG_ND_PROTO_PTP_HDR_LEN        0x0022 

typedef struct
{
    AG_U8   a_source[6];
    AG_U8   a_destination[6];

} AgNdProtoEth;

typedef struct
{
    AG_U32  n_priority;
    AG_U32  n_vid;

} AgNdProtoVlan;

typedef struct
{
    AG_U32  n_tos;
    AG_U32  n_ttl;
    AG_U32  n_source;
    AG_U32  n_destination;

} AgNdProtoIp4;

typedef struct
{
    AG_U32  n_traffic_class;
    AG_U32  n_flow_label;
    AG_U32  n_hop_limit;
    AG_U8   a_source[16];
    AG_U8   a_destination[16];

} AgNdProtoIp6;

typedef struct
{
    AG_U32  n_source;
    AG_U32  n_destination;

} AgNdProtoUdp;

typedef struct
{
    AG_U32  n_label;
    AG_U32  n_expiremental;
    AG_U32  n_ttl;

} AgNdProtoMpls;

typedef struct
{
    AG_U32  n_label;

} AgNdProtoVcLabel;

typedef struct
{
    AG_U32  n_pt;
    AG_U32  n_ssrc;

} AgNdProtoRtp;

typedef struct
{
    AG_U8   n_ts;
    AG_U8   n_ver;
    AG_U8   n_domain;
    AG_U16  n_flags;
    AG_U8   a_correction[8];
    AG_U8   a_src_port[10];
    AG_BOOL b_dst_port;
    AG_U8   a_dst_port[10];
	AG_U8   n_log_mean_interval;
} AgNdProtoPtp;

/*ORI*/
typedef struct
{
	AG_BOOL b_udp_mode;
  	AG_U32 n_header;
	AG_U32 n_session_local_id;
	AG_U32 n_session_peer_id;
	AG_U32 n_local_cookie1;
	AG_U32 n_local_cookie2;
	AG_U32 n_peer_cookie1;
	AG_U32 n_peer_cookie2;
}AgNdProtoL2tpv3;

/*ORI*/
/*changed to add L2tpv3 protcol*/
typedef struct
{
    AgNdEncapsulation       e_encapsulation;

    AgNdProtoEth            x_eth;
    AgNdProtoVlan           a_vlan[AG_ND_PROTO_VLAN_TAG_MAX];

    AgNdProtoIp4            x_ip4;
    AgNdProtoIp6            x_ip6;
    AgNdProtoMpls           a_mpls[AG_ND_PROTO_MPLS_LABEL_MAX];
    AgNdProtoVcLabel        x_vc_label;
    AgNdProtoUdp            x_udp;
    AgNdProtoRtp            x_rtp;
    AgNdProtoPtp            x_ptp;
	AgNdProtoL2tpv3			x_l2tpv3;

    AG_U32                  n_ip_version;
    AG_U32                  n_vlan_count;
    AG_U32                  n_mpls_count;
    AG_BOOL                 b_rtp_exists;
    AG_BOOL                 b_udp_chksum;
	AG_U32					n_l2tpv3_count;

} AgNdPacketHeader;


void ag_nd_header_build(
        AgNdPacketHeader    *p_hdr,             
        AgNdBitwise         *p_bw,              
        AgNdRegTransmitHeaderFormatDescriptor   
                            *p_tx_hdr_fmt,      
        AG_U32              *p_udp_chksum,      
        AG_U16              *p_udp_len,         
        AG_U32              *p_ip_chksum,       
        AG_U16              *p_ip_len);

void ag_nd_header_update_cw(
        AgNdDevice          *p_device,
        AgNdMemUnit         *p_mem_unit,
        AG_U32              n_offset,
        AG_U16              n_set_cw_mask,
        AG_U16              n_set_cw_value,
        AgNdChannelIngressInfo
                            *p_channel_info);

void 
ag_nd_header_update_ptp(
        AgNdDevice          *p_device,
        AgNdMemUnit         *p_mem_unit,
        AG_U32              n_offset,
        AG_U32              n_udp_chksum, 
        AG_U16              n_udp_len,    
        AG_U32              n_ip_chksum,  
        AG_U16              n_ip_len);

void ag_nd_strict_build(
        AgNdDevice          *p_device, 
        AgNdPacketHeader    *p_hdr, 
        AgNdChannel         n_channel_id);


void ag_nd_strict_update_vlan_id(AgNdDevice* 	p_device,
								 AG_U32 		n_vlan_count,
							     AgNdProtoVlan* p_vlan,
							     AgNdChannel 	n_channel_id,
							     AG_BOOL		b_ptp,
							     AG_U32* 		p_addr);


#ifdef __cplusplus
}
#endif

#endif /* __NEMO_HEADER_H__ */

