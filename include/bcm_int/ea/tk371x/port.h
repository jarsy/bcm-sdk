/*
 * $Id: port.h,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains port module definitions internal to
 * the BCM library.
 */
#ifndef _BCM_INT_TK371X_PORT_H
#define _BCM_INT_TK371X_PORT_H
#include <soc/ea/tk371x/CtcMiscApi.h>

#define _BCM_TK371X_PON_PORT_BASE		0
#define _BCM_TK371X_MAX_PON_PORT_NUM	1
#define _BCM_TK371X_UNI_PORT_BASE		1
#define _BCM_TK371X_MAX_UNI_PORT_NUM	2
#define _BCM_TK371X_LLID_PORT_BASE		3
#define _BCM_TK371X_MAX_LLID_PORT_NUM	8

#define TK371X_PON_PORT_VALID(port) ((port) == _BCM_TK371X_PON_PORT_BASE)
#define TK371X_UNI_PORT_VALID(port) ((port) >= _BCM_TK371X_UNI_PORT_BASE && \
								(port) < _BCM_TK371X_LLID_PORT_BASE)
#define TK371X_GE_PORT_VALID(port)  ((port) == _BCM_TK371X_UNI_PORT_BASE)
#define TK371X_FE_PORT_VALID(port)	 ((port) == (_BCM_TK371X_UNI_PORT_BASE + 1))
#define TK371X_LLID_PORT_VALID(port)	(((port) >= _BCM_TK371X_LLID_PORT_BASE) && \
								((port) < (_BCM_TK371X_LLID_PORT_BASE + _BCM_TK371X_MAX_LLID_PORT_NUM)))
#define TK371X_PORT_VALID(port)	(TK371X_PON_PORT_VALID(port) || \
									TK371X_UNI_PORT_VALID(port) || \
									TK371X_LLID_PORT_VALID(port))

typedef enum {
	bcmTk371xPortArlDisLearning = 0,
	bcmTk371xPortArlHwLearning = 1,
	bcmTk371xPortFwdModeDisable = 0,
	bcmTk371xPortFwdModeEnable = 1,
}_bcm_tk371x_port_learn_mode_t;

typedef struct _bcm_tk371x_port_laser_para_s{
	uint16 temp;
	uint16 vcc;
	uint16 tx_bias;
	uint16 tx_power;
	uint16 rx_power;
}_bcm_tk371x_port_laser_para_t;

#define PORT_PON_LASER_TX_CONFIG_TIME		0x01
#define PORT_PON_LASER_TX_CONFIG_MODE		0x02
#define PORT_PON_LASER_TX_CONFIG_TIME_MODE	0x03

typedef struct pon_larse_tx_config_s{
	uint8 flag;
	CtcExtONUTxPowerSupplyControl txpws_control;
}pon_larse_tx_config_t;

extern int _bcm_tk371x_port_detach(int unit);

#endif
