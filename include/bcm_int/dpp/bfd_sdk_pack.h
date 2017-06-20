/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    bfd_feature.h
 */

#ifndef BFD_SDK_PACK_H_
#define BFD_SDK_PACK_H_

#include <bcm_int/dpp/bfd_sdk_msg.h>

uint8 *
bfd_sdk_dpp_msg_ctrl_sess_set_pack(uint8 *buf,
                                bfd_sdk_msg_ctrl_sess_set_t *msg);

uint8 *
bfd_sdk_dpp_msg_ctrl_sess_get_unpack(uint8 *buf,
                                 bfd_sdk_msg_ctrl_sess_get_t *msg);
uint8 *
bfd_sdk_dpp_msg_ctrl_init_pack(uint8 *buf, bfd_sdk_msg_ctrl_init_t *msg);
#endif /* BFD_SDK_PACK_H_ */

