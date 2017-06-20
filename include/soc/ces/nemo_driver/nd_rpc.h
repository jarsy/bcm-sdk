/*
 * $Id$ 
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __NEMO_RPC_H__
#define __NEMO_RPC_H__

#include "pub/nd_api.h"
#include "nd_protocols.h"

#ifdef __cplusplus
extern "C"
{
#endif

void
ag_nd_rpc_init(AgNdDevice *p_device);

AgResult ag_nd_opcode_read_config_rpc_ucode(AgNdDevice *p_device, AgNdMsgConfigRpcUcode *p_ucode);
AgResult ag_nd_opcode_write_config_rpc_ucode(AgNdDevice *p_device, AgNdMsgConfigRpcUcode *p_ucode);

AgResult ag_nd_opcode_write_config_rpc_port_fwd(AgNdDevice *p_device, AgNdMsgConfigRpcPortFwd *p_msg);

AgResult ag_nd_opcode_write_config_rpc_map_label2chid(AgNdDevice *p_device, AgNdMsgConfigRpcMapLabelToChid *p_msg);

AgResult ag_nd_opcode_read_config_rpc_policy(AgNdDevice *p_device, AgNdMsgConfigRpcPolicy *p_msg);
AgResult ag_nd_opcode_write_config_rpc_policy(AgNdDevice *p_device, AgNdMsgConfigRpcPolicy *p_msg);

/* */
/* special channel id values for forwarding traffic to host via */
/* high/low priority queues based on UDP destination port */
/* */
#define AG_ND_RPC_UDP_FWD_HPQ_CHID          0x1000
#define AG_ND_RPC_UDP_FWD_LPQ_CHID          0x800

/* */
/* these MSB nibbles should be added to multiplexing */
/* labels in order maintain uniqueness   */
/* */
#define AG_ND_RPC_LSTREE_HIGH_NIBBLE_MPLS   0xe
#define AG_ND_RPC_LSTREE_HIGH_NIBBLE_ECID   0xd
#define AG_ND_RPC_LSTREE_HIGH_NIBBLE_UDP    0xc
#define AG_ND_RPC_LSTREE_HIGH_NIBBLE_PTP    0xf
/*ORI*/
/*L2TP */
#define AG_ND_RPC_LSTREE_HIGH_NIBBLE_L2TP 0xa

#ifndef CES16_BCM_VERSION
  #define AG_ND_RPC_LSTREE_TAG_BASE           0x100000
#else
  #define AG_ND_RPC_LSTREE_TAG_BASE           0x100000
#endif
/* */
/* classification path object: holds classification */
/* builder path */
/* */
#define AG_ND_RPC_LSTREE_PATH_MAX           11

typedef struct
{
    AG_U8   a_data[AG_ND_RPC_LSTREE_PATH_MAX];
    AG_U8   a_opcode[AG_ND_RPC_LSTREE_PATH_MAX];
    AG_U32  n_size;

} AgNdLsTreePath;

/* */
/* this type defines function that converts ECID/MPLS/L2TP/UDP label  */
/* to classification path */
/* */

typedef struct
{
    AG_U32  n_label;
    AG_U8   *p_ptp_port;

} AgNdClsData;

typedef void (*AgNdPathBuilder)(AgNdLsTreePath*, AgNdClsData*);

void ag_nd_rpc_build_mpls_path(AgNdLsTreePath *p_path, AgNdClsData *p_cls_data);
void ag_nd_rpc_build_ecid_path(AgNdLsTreePath *p_path, AgNdClsData *p_cls_data);
void ag_nd_rpc_build_l2tp_path(AgNdLsTreePath *p_path, AgNdClsData *p_cls_data);
void ag_nd_rpc_build_udp_path(AgNdLsTreePath *p_path, AgNdClsData *p_cls_data);
void ag_nd_rpc_build_ptp_path(AgNdLsTreePath *p_path, AgNdClsData *p_cls_data);

/* */
/* label search tree service functions */
/* */
AgResult
ag_nd_rpc_query_label(
    AgNdDevice          *p_device, 
    AgNdEncapsulation   e_encapsulation,
    AgNdClsData         *p_cls_data,
    AgNdChannel         *n_channel_id);

AgResult 
ag_nd_rpc_label_to_chid_map(
    AgNdDevice          *p_device, 
    AgNdEncapsulation   e_encapsulation,
    AgNdClsData         *p_cls_data,
    AgNdChannel         n_channel_id);

AgResult 
ag_nd_rpc_label_to_chid_unmap(
    AgNdDevice          *p_device, 
    AgNdEncapsulation   e_encapsulation,
    AgNdClsData         *p_cls_data);


#ifdef __cplusplus
}
#endif

#endif  /* __NEMO_RPC_H__ */

