/*
 * $Id: port.c,v 1.40 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     port.c
 * Purpose:
 *
 */

#include <shared/bsl.h>

#include <bcm_int/ea/port.h>
#include <bcm_int/ea/tk371x/port.h>
#include <bcm/port.h>
#include <bcm/error.h>
#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/ea/tk371x/onu.h>
#include <soc/ea/tk371x/ea_drv.h>
#include <soc/ea/tk371x/brg.h>

#define HOLDOVER_DEFAULT_TIME	100

static int _user_traffic_status = 1;
static uint32 _mac_learn_mode		= BCM_PORT_LEARN_ARL;
static pon_larse_tx_config_t tx_larse_config;

int _bcm_ea_port_detach(int unit)
{
	_user_traffic_status 	= 1;
	_mac_learn_mode			= BCM_PORT_LEARN_ARL;
	tx_larse_config.flag 	= 0;
	return BCM_E_NONE;
}

/* Get or set various features at the port level. */
int
_bcm_ea_port_control_get(
	    int unit,
	    bcm_port_t port,
	    bcm_port_control_t type,
	    int *value)
{
	int ret_val;

	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "enter _bcm_ea_port_control_get()\n")));
	if (TK371X_PON_PORT_VALID(port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "port type: PON=%d\n"), port));
		switch (type){
			case bcmPortControlPonFecMode:{
				OamExtFECMode fec;
				ret_val = _soc_ea_fec_mode_get(unit, port, &fec);
				if (OK != ret_val){
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "calling _soc_ea_fec_mode_get failed\n")));
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "return value: BCM_E_RESOURCE\n")));
					return BCM_E_FAIL;
				}
				if (fec.rxFEC == 0){
					if (fec.txFEC == 0){
						*value = 0;
					}
					if (fec.txFEC == 1){
						*value = 1;
					}
				}
				if (fec.rxFEC == 1){
					if (fec.txFEC == 0){
						*value = 2;
					}
					if (fec.txFEC == 1){
						*value = 3;
					}
				}
				if (*value < 0  || *value > 4){
					return BCM_E_INTERNAL;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlPonFecMode\n")));
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "fec.rxFEC=%d\n"), fec.rxFEC));
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "fec.txFEC=%d\n"), fec.txFEC));
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "*value=%d\n"), *value));
				return BCM_E_NONE;
			}
			case bcmPortControlPonUserTraffic:
				*value = _user_traffic_status;
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlPonUserTraffic.\n")));
				return BCM_E_NONE;
			case bcmPortControlPonMultiLlid:{
				uint32 llid;
				int state;
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlPonMultiLlid.\n")));
				ret_val = soc_auth_result_get(unit, &state);
				if (ret_val != SOC_E_NONE){
					return BCM_E_FAIL;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "soc_auth_result_get return val=%d, state=%d.\n"), ret_val, state));
				if (state == TRUE){
					*value = -1;
					return BCM_E_NONE;
				}
				ret_val = _soc_ea_real_llid_count_get(unit, &llid);
				if (ret_val != SOC_E_NONE){
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "_soc_ea_real_llid_count_get return BCM_E_RESOURCE, %d\n"), ret_val));
					return BCM_E_FAIL;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "_soc_ea_real_llid_count_get llid_count=%d\n"), llid));
				*value = llid;
				return BCM_E_NONE;
			}
			case bcmPortControlPonEncryptKeyExpiryTime:{
				uint16 time;
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlPonEncryptKeyExpiryTime.\n")));
				ret_val = _soc_ea_encrypt_key_expiry_time_get(unit, port, &time);
				if (OK != ret_val){
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "_soc_ea_encrypt_key_expiry_time_get return BCM_E_RESOURCE, %d\n"), ret_val));
					return BCM_E_FAIL;
				}
				*value = (int)time;
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "_soc_ea_real_llid_count_get time=%d\n"), time));
				return BCM_E_NONE;
			}
			case bcmPortControlPonHoldoverState:{
				uint32 state;
				uint32 time;

				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlPonHoldoverState.\n")));
				ret_val = CtcExtOamGetHoldover(unit, port, &state, &time);
				if (ret_val != OK){
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "CtcExtOamGetHoldover return BCM_E_RESOURCE, %d\n"), ret_val));
					return BCM_E_FAIL;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "CtcExtOamGetHoldover state=%d, time=%d\n"), state, time));
				if (state == HOLDOVERACTIVATED){
					*value = 1;
				}else if (state == HOLDOVERDISACTIVATED){
					*value = 0;
				}else{
					return BCM_E_FAIL;
				}
				return BCM_E_NONE;
			}
			case bcmPortControlPonHoldoverTime:{
				uint32 state;
				uint32 time;
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlPonHoldoverTime.\n")));
				ret_val = CtcExtOamGetHoldover(unit, port, &state, &time);
				if (ret_val != OK){
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "CtcExtOamGetHoldover return BCM_E_RESOURCE, %d\n"), ret_val));
					return BCM_E_FAIL;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "CtcExtOamGetHoldover state=%d, time=%d\n"), state, time));
				*value = time;
				return BCM_E_NONE;
			}
			default:
				return BCM_E_UNAVAIL;
		}
	}else if (TK371X_UNI_PORT_VALID(port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "port type: UNI=%d\n"), port));
		switch (type){
			case bcmPortControlEthPortAutoNegFailureAlarmState:{
				uint8 state;
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlEthPortAutoNegFailureAlarmState.\n")));
				ret_val = _soc_ea_alarm_enable_get(unit, port,
						SOC_EA_EVENT_AUTONEG_FAILURE_ALARM, &state);
				if (ret_val != OK){
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "_soc_ea_alarm_enable_get return BCM_E_RESOURCE, %d\n"), ret_val));
					return BCM_E_FAIL;
				}
				if (state > 0){
					*value = 1;
				}else{
					*value = 0;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "_soc_ea_alarm_enable_get state=%d\n"), state));
				return BCM_E_NONE;
			}
			case bcmPortControlEthPortLosAlarmState:{
				uint8 state;
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlEthPortLosAlarmState.\n")));
				ret_val = _soc_ea_alarm_enable_get(unit, port,
						SOC_EA_EVENT_LOS_ALARM, &state);
				if (ret_val != OK){
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "_soc_ea_alarm_enable_get return BCM_E_RESOURCE, %d\n"), ret_val));
					return BCM_E_FAIL;
				}
				if (state > 0){
					*value = 1;
				}else{
					*value = 0;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "_soc_ea_alarm_enable_get state=%d\n"), state));
				return BCM_E_NONE;
			}
			case bcmPortControlEthPortFailureAlarmState:{
				uint8 state;
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlEthPortFailureAlarmState.\n")));
				ret_val = _soc_ea_alarm_enable_get(unit, port,
						SOC_EA_EVENT_PORT_FAILURE_ALARM, &state);
				if (ret_val != OK){
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "_soc_ea_alarm_enable_get return BCM_E_RESOURCE, %d\n"), ret_val));
					return BCM_E_FAIL;
				}
				if (state > 0){
					*value = 1;
				}else{
					*value = 0;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "_soc_ea_alarm_enable_get state=%d\n"), state));
				return BCM_E_NONE;
			}
			case bcmPortControlEthPortLoopbackAlarmState:{
				uint8 state;
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlEthPortLoopbackAlarmState.\n")));
				ret_val = _soc_ea_alarm_enable_get(unit, port,
						SOC_EA_EVENT_PORT_LOOPBACK_ALARM, &state);

				if (ret_val != OK){
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "_soc_ea_alarm_enable_get return BCM_E_RESOURCE, %d\n"), ret_val));
					return BCM_E_FAIL;
				}
				if (state > 0){
					*value = 1;
				}else{
					*value = 0;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "_soc_ea_alarm_enable_get state=%d\n"), state));
				return BCM_E_NONE;
			}
			case bcmPortControlEthPortCongestionAlarmState:{
				uint8 state;
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlEthPortCongestionAlarmState.\n")));
				ret_val = _soc_ea_alarm_enable_get(unit, port,
						SOC_EA_EVENT_PORT_CONGESTION_ALARM, &state);
				if (ret_val != OK){
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "_soc_ea_alarm_enable_get return BCM_E_RESOURCE, %d\n"), ret_val));
					return BCM_E_FAIL;
				}
				if (state > 0){
					*value = 1;
				}else{
					*value = 0;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "_soc_ea_alarm_enable_get state=%d\n"), state));
				return BCM_E_NONE;
			}
			default:
				return BCM_E_UNAVAIL;
		}
	}else if (TK371X_LLID_PORT_VALID(port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "port type: LLID=%d\n"), port));
		return BCM_E_UNAVAIL;
	}else{
		return BCM_E_PORT;
	}

	return BCM_E_NONE;
}

/* Get or set various features at the port level. */
int
_bcm_ea_port_control_set(
	    int unit,
	    bcm_port_t port,
	    bcm_port_control_t type,
	    int value)
{
	int ret_val = BCM_E_NONE;

	if (TK371X_PON_PORT_VALID(port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "port type: PON=%d\n"), port));
		switch (type){
			case bcmPortControlPonFecMode:{
				OamExtFECMode fec;

				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlPonFecMode\n")));
				switch (value){
				case 0:
					fec.rxFEC = fec.txFEC = 0;
					break;
				case 1:
					fec.rxFEC = 1;
					fec.txFEC = 0;
					break;
				case 2:
					fec.rxFEC = 0;
					fec.txFEC = 1;
					break;
				case 3:
					fec.rxFEC = 1;
					fec.txFEC = 1;
					break;
				case 4:
					return BCM_E_NONE;
				default:
					return BCM_E_PARAM;
				}
				ret_val = _soc_ea_fec_mode_set(unit, port, &fec);
				if (OK != ret_val){
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "calling _soc_ea_fec_mode_set failed\n")));
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "return value: BCM_E_RESOURCE\n")));
					return BCM_E_FAIL;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlPonFecMode\n")));
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "fec.rxFEC=%d\n"), fec.rxFEC));
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "fec.txFEC=%d\n"), fec.txFEC));
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "*value=%d\n"), value));
				return BCM_E_NONE;
			}
			case bcmPortControlPonUserTraffic:
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlPonUserTraffic\n")));
				if (value != 0 && value != 1){
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "return BCM_E_PARAM, invalid value: %d\n"), value));
					return BCM_E_PARAM;
				}
				_user_traffic_status = value;
				if (_user_traffic_status == 0){
					ret_val = TkExtOamSetDisUserTraffic(unit, port);
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "calling TkExtOamSetEnaUserTraffic, ret_val=%d\n"), ret_val));
					if (ret_val != OK){
						return BCM_E_FAIL;
					}
				}
				if (_user_traffic_status == 1){
					ret_val = TkExtOamSetEnaUserTraffic(unit, port);
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "calling TkExtOamSetEnaUserTraffic, ret_val=%d\n"), ret_val));
					if (ret_val != OK){
						return BCM_E_FAIL;
					}
				}
				return BCM_E_NONE;
			case bcmPortControlPonMultiLlid:{
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlPonMultiLlid\n")));
				if (value > _BCM_TK371X_MAX_LLID_PORT_NUM + 1){
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "return BCM_E_PARAM, invalid value: %d\n"), value));
					return BCM_E_PARAM;
				}
				if (value == -1){
					/* do something */
					if (SOC_E_NONE != soc_auth_result_set(unit, TRUE)){
						return BCM_E_FAIL;
					}
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "soc_auth_result_set fail return value SOC_E_NONE.\n")));
					return BCM_E_NONE;

				}else if (value == ( _BCM_TK371X_MAX_LLID_PORT_NUM + 1)){
					/* do something */
					if (SOC_E_NONE != soc_auth_result_set(unit, FALSE)){
						return BCM_E_FAIL;
					}
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "soc_auth_result_set success return value SOC_E_NONE.\n")));
					return BCM_E_NONE;
				}else if (value >=0 && value <= _BCM_TK371X_MAX_LLID_PORT_NUM){
					ret_val = CtcExtOamSetMulLlidCtrl(unit, value);
				}
				if (ret_val != OK){
					return BCM_E_FAIL;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "calling CtcExtOamSetMulLlidCtrl, ret_val=%d\n"), ret_val));
				return BCM_E_NONE;
			}
			case bcmPortControlPonEncryptKeyExpiryTime: {
				uint16 time;

				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlPonEncryptKeyExpiryTime\n")));
				time = (uint16)value;
				ret_val = TkExtOamSetEncryptKeyExpiryTime(unit, port, time);
				if (ret_val != OK){
					return BCM_E_FAIL;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "calling TkExtOamSetEncryptKeyExpiryTime time=%d, ret_val=%d\n"),time, ret_val));
				return BCM_E_NONE;
			}
			case bcmPortControlPonHoldoverState:{
				uint32 state;
				uint32 time;

				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlPonHoldoverState\n")));
				ret_val = CtcExtOamGetHoldover(unit, port, &state, &time);
				if (ret_val != OK){
					return BCM_E_FAIL;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "calling CtcExtOamGetHoldoverret_val=%d\n"), ret_val));
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "state=%d\n"),state));
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "time=%d\n"), time));
				state = value;
				if (value == 0){
					state = HOLDOVERDISACTIVATED;
				}else if (value == 1){
					state = HOLDOVERACTIVATED;
					if (time == 0){
						time = HOLDOVER_DEFAULT_TIME;
					}
				}else{
					return BCM_E_PARAM;
				}
				ret_val = CtcExtOamSetHoldover(unit, port, state, time);
				if (ret_val != OK){
					return BCM_E_FAIL;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "calling CtcExtOamSetHoldover=%d\n"), ret_val));
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "state=%d\n"),state));
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "time=%d\n"), time));
				return BCM_E_NONE;
			}

			case bcmPortControlPonHoldoverTime:{
				uint32 state;
				uint32 time;
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlPonHoldoverTime\n")));
				ret_val = CtcExtOamGetHoldover(unit, port, &state, &time);
				if (ret_val != OK){
					return BCM_E_FAIL;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "calling CtcExtOamGetHoldoverret_val=%d\n"), ret_val));
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "state=%d\n"),state));
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "time=%d\n"), time));
				time = value;
				ret_val = CtcExtOamSetHoldover(unit, port, state, time);
				if (ret_val != OK){
					return BCM_E_FAIL;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "calling CtcExtOamSetHoldover=%d\n"), ret_val));
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "state=%d\n"),state));
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "time=%d\n"), time));
				return BCM_E_NONE;
				}
			default:
				return BCM_E_UNAVAIL;
		}
	}else if (TK371X_UNI_PORT_VALID(port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "Port TYPE: UNI=%d\n"), port));
		switch (type){
			case bcmPortControlEthPortAutoNegFailureAlarmState: {
				uint8 state;
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlEthPortAutoNegFailureAlarmState\n")));
				if (value != 0 && value != 1){
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "return BCM_E_PARAM, invalid value: %d\n"), value));
					return BCM_E_PARAM;
				}
				state = value;
				ret_val = _soc_ea_alarm_enable_set(unit, port,
						SOC_EA_EVENT_AUTONEG_FAILURE_ALARM, state);
				if (ret_val != OK){
					return BCM_E_FAIL;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "calling _soc_ea_alarm_enable_set=%d\n"), ret_val));
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "state=%d\n"),state));
				return BCM_E_NONE;
			}
			case bcmPortControlEthPortLosAlarmState:{
				uint8 state;
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlEthPortLosAlarmState\n")));
				if (value != 0 && value != 1){
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "return BCM_E_PARAM, invalid value: %d\n"), value));
					return BCM_E_PARAM;
				}
				state = value;
				ret_val = _soc_ea_alarm_enable_set(unit, port,
						SOC_EA_EVENT_LOS_ALARM, state);
				if (ret_val != OK){
					return BCM_E_FAIL;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "calling _soc_ea_alarm_enable_set=%d\n"), ret_val));
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "state=%d\n"),state));
				return BCM_E_NONE;
			}
			case bcmPortControlEthPortFailureAlarmState:{
				uint8 state;
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlEthPortFailureAlarmState\n")));
				if (value != 0 && value != 1){
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "return BCM_E_PARAM, invalid value: %d\n"), value));
					return BCM_E_PARAM;
				}
				state = value;
				ret_val = _soc_ea_alarm_enable_set(unit, port,
						SOC_EA_EVENT_PORT_FAILURE_ALARM, state);
				if (ret_val != OK){
					return BCM_E_FAIL;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "calling _soc_ea_alarm_enable_set=%d\n"), ret_val));
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "state=%d\n"),state));
				return BCM_E_NONE;
			}
			case bcmPortControlEthPortLoopbackAlarmState:{
				uint8 state;
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlEthPortLoopbackAlarmState\n")));
				if (value != 0 && value != 1){
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "return BCM_E_PARAM, invalid value: %d\n"), value));
					return BCM_E_PARAM;
				}
				state = value;
				ret_val = _soc_ea_alarm_enable_set(unit, port,
						SOC_EA_EVENT_PORT_LOOPBACK_ALARM, state);
				if (ret_val != OK){
					return BCM_E_FAIL;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "calling _soc_ea_alarm_enable_set=%d\n"), ret_val));
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "state=%d\n"),state));
				return BCM_E_NONE;
			}
			case bcmPortControlEthPortCongestionAlarmState:{
				uint8 state;
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "type=bcmPortControlEthPortLoopbackAlarmState\n")));
				if (value != 0 && value != 1){
					LOG_INFO(BSL_LS_BCM_PORT,
                                                 (BSL_META_U(unit,
                                                             "return BCM_E_PARAM, invalid value: %d\n"), value));
					return BCM_E_PARAM;
				}
				state = value;
				ret_val = _soc_ea_alarm_enable_set(unit, port,
						SOC_EA_EVENT_PORT_CONGESTION_ALARM, state);
				if (ret_val != OK){
					return BCM_E_FAIL;
				}
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "calling _soc_ea_alarm_enable_set=%d\n"), ret_val));
				LOG_INFO(BSL_LS_BCM_PORT,
                                         (BSL_META_U(unit,
                                                     "state=%d\n"),state));
				return BCM_E_NONE;
			}
			default:
				return BCM_E_UNAVAIL;
		}
	}
	else if (TK371X_LLID_PORT_VALID(port)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "port type: LLID=%d\n"), port));
		switch (type){
			case bcmPortControlPonFecMode:
				return BCM_E_UNAVAIL;
			default:
				return BCM_E_UNAVAIL;
		}
	}else{
		return BCM_E_PORT;
	}
	return BCM_E_NONE;
}


/* Enable or disable a port. */
int
_bcm_ea_port_enable_get(
	    int unit,
	    bcm_port_t port,
	    int *enable)
{
	int rv = BCM_E_NONE;
	uint8 state;

	if (port < _BCM_TK371X_PON_PORT_BASE ||
			port > (_BCM_TK371X_LLID_PORT_BASE + _BCM_TK371X_MAX_LLID_PORT_NUM)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "_bcm_ea_port_enable_get return BCM_E_PORT, invalid port=%d\n"), port));
		return BCM_E_PORT;
	}else if (port < _BCM_TK371X_UNI_PORT_BASE){
		rv = TkExtOamGetEponAdmin(unit, port, &state);
		/* change the pon state some as port status*/
		if (rv == RcOk)
			{
			state = (state == 0) ? 1 : 0;
			}
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "TkExtOamGetEponAdmin:unit=%d, port=%d, state=%d, rv=%d\n"), unit, port, state, rv));
	}else if (port < _BCM_TK371X_LLID_PORT_BASE){
		/* getting the UNI port status */
		rv = _soc_ea_phy_admin_state_get((uint8)unit, 0, port, &state);
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "_soc_ea_phy_admin_state_get:unit=%d, port=%d, state=%d,rv=%d\n"), unit, port, state, rv));
	}else {
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "_bcm_ea_port_enable_get return BCM_E_UNAVAIL, invalid port=%d\n"), port));
		return BCM_E_UNAVAIL;
	}

	if (rv != OK){
		return BCM_E_FAIL;
	}
	*enable = (int)state;
	return BCM_E_NONE;
}

/* Enable or disable a port. */
int
_bcm_ea_port_enable_set(
	    int unit,
	    bcm_port_t port,
	    int enable)
{
	int rv;
	int state;

	if (port < _BCM_TK371X_PON_PORT_BASE ||
			port > (_BCM_TK371X_LLID_PORT_BASE + _BCM_TK371X_MAX_LLID_PORT_NUM)){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "_bcm_ea_port_enable_set return BCM_E_PORT, invalid port=%d\n"), port));
		return BCM_E_PORT;
	}else if (port < _BCM_TK371X_UNI_PORT_BASE){
		state = (enable == 1) ? 0 : 1;
		rv = TkExtOamSetEponAdmin(unit, port, state);
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "TkExtOamSetEponAdmin:unit=%d, port=%d, enable=%d, rv=%d\n"), unit, port, enable, rv));
	}else if (port < _BCM_TK371X_LLID_PORT_BASE){
		/* getting the UNI port status */
		rv = _soc_ea_phy_admin_state_set((uint8)unit, 0, port, enable);
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "_soc_ea_phy_admin_state_set:unit=%d, port=%d, enable=%d, rv=%d\n"), unit, port, enable, rv));
	}else {
		return BCM_E_UNAVAIL;
	}
	if (rv != OK){
		return BCM_E_FAIL;
	}
	return BCM_E_NONE;
}

/* Set or retrieve the current maximum packet size permitted on a port. */
int
_bcm_ea_port_frame_max_get(
	    int unit,
	    bcm_port_t port,
	    int *size)
{
	int rv = 0;
	uint16 max_frame = 0;

	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "_bcm_ea_port_frame_max_get..., port=%d\n"), port));
	rv = _soc_ea_mtu_get((uint8)unit, 0, (uint8)port, &max_frame);
	if (rv != OK){
		return BCM_E_FAIL;
	}
	*size = max_frame;
	return BCM_E_NONE;
}

/* Set or retrieve the current maximum packet size permitted on a port. */
int
_bcm_ea_port_frame_max_set(
	    int unit,
	    bcm_port_t port,
	    int size)
{
	int rv = BCM_E_INTERNAL;
	uint16 framesize;

	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "_bcm_ea_port_frame_max_set...\n")));

	framesize = size;
	if (size > 65535){
		framesize = 65535;
	}
	rv = _soc_ea_mtu_set((uint8)unit, 0, (uint8)port, framesize);
	if (rv != OK){
		return BCM_E_FAIL;
	}
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "_soc_ea_mtu_set: UNI Port=%d, rv=%d, max_frame=%d"), port, rv, framesize));
	return BCM_E_NONE;
}


/* Get the hardware and software learning support on a port. */
int
_bcm_ea_port_learn_get(
		int unit,
	    bcm_port_t port,
	    uint32 *flags)
{
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "_bcm_ea_port_learn_get...\n")));
	if (port < _BCM_TK371X_UNI_PORT_BASE || port >= _BCM_TK371X_LLID_PORT_BASE){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "Return BCM_E_UNAVAIL, invalid port=%d\n"), port));
		return BCM_E_UNAVAIL;
	}
	if ((_mac_learn_mode & BCM_PORT_LEARN_ARL) ||
			(_mac_learn_mode & BCM_PORT_LEARN_FWD)){
		*flags = _mac_learn_mode;
	}else if(0 == _mac_learn_mode){
		*flags = 0;
	}else{
		return BCM_E_FAIL;
	}
	return BCM_E_NONE;
}

/* Control the hardware and software learning support on a port. */
int
_bcm_ea_port_learn_modify(
	    int unit,
	    bcm_port_t port,
	    uint32 add,
	    uint32 remove)
{
	int rv = BCM_E_NONE;
	uint32 flags = 0;

	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "_bcm_ea_port_learn_modify...\n")));
	if (port < _BCM_TK371X_UNI_PORT_BASE || port >= _BCM_TK371X_LLID_PORT_BASE){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "Return BCM_E_UNAVAIL, invalid port=%d\n"), port));
		return BCM_E_UNAVAIL;
	}

	rv = _bcm_ea_port_learn_get(unit, port, &flags);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "_bcm_ea_port_learn_get:unit=%d port=%d rv=%d flags=%d\n"),
              unit, port, rv, flags));
	if (BCM_E_NONE != rv){
		return rv;
	}
	flags |= add;
	flags &= ~remove;
	rv = _bcm_ea_port_learn_set(unit, port, add);
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "_bcm_ea_port_learn_set:unit=%d port=%d rv=%d add=%d\n"),
                  unit, port, rv, add));
	return rv;
}

/* Control the hardware and software learning support on a port. */
int
_bcm_ea_port_learn_set(
	    int unit,
	    bcm_port_t port,
	    uint32 flags)
{
	int rv1 = BCM_E_NONE;
	int rv2 = BCM_E_NONE;
	_bcm_tk371x_port_learn_mode_t mode;
	ForwardingMode fwd_mode;

	if (port < _BCM_TK371X_UNI_PORT_BASE || port >= _BCM_TK371X_LLID_PORT_BASE){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "Return BCM_E_UNAVAIL, invalid port=%d\n"), port));
		return BCM_E_UNAVAIL;
	}

	if (flags & BCM_PORT_LEARN_ARL){
		mode = bcmTk371xPortArlHwLearning;
	}else{
		mode = bcmTk371xPortArlDisLearning;
	}
	rv1 = _soc_ea_mac_learning_set((uint8)unit, 0,
			port, (uint8)mode);
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "_soc_ea_mac_learning_set:unit=%d port=%d rv=%d mode=%d\n"),
                  unit, port, rv1, mode));
	if (rv1 != OK){
		return BCM_E_FAIL;
	}

	if (flags & BCM_PORT_LEARN_FWD){
		fwd_mode = ForwardFloodUnknown;
	}else{
		fwd_mode = ForwardDropUnknown;
	}
	rv2 = _soc_ea_forward_mode_set(unit, 0,  port, fwd_mode);
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "_soc_ea_forward_mode_set:unit=%d port=%d rv=%d mode=%d\n"),
                  unit, port, rv2, fwd_mode));
	if (rv2 != OK){
		return BCM_E_FAIL;
	}
	_mac_learn_mode = flags;
	return BCM_E_NONE;
}

STATIC int
_bcm_tk371x_port_laser_para_update(
		int unit,
		int port,
		_bcm_tk371x_port_laser_para_t *para){
	int ret_val;
	CtcOamTlvPowerMonDiag diag;

	ret_val = _soc_ea_phy_control_opt_tran_diag_get(unit, &diag);
	if (ret_val != OK){
		return BCM_E_FAIL;
	}
	para->temp = diag.temp;
	para->vcc = diag.vcc;
	para->tx_bias = diag.txBias;
	para->tx_power = diag.txPower;
	para->rx_power = diag.rxPower;
	return BCM_E_NONE;
}
/* Set/Get PHY specific configurations. */
int
_bcm_ea_port_phy_control_get(
	    int unit,
	    bcm_port_t port,
	    bcm_port_phy_control_t type,
	    uint32 *value)
{
	int ret_val;
	uint8 state;
	_soc_ea_alarm_threshold_t threshold;
	_bcm_tk371x_port_laser_para_t port_laser_para;

	if (port != 0){
		return BCM_E_UNAVAIL;
	}
	switch(type){
		case BCM_PORT_PHY_CONTROL_INTERFACE:
			ret_val = _soc_ea_phy_control_if_get(unit, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_LASER_TRANCEIVER_TEMP:
			ret_val = _bcm_tk371x_port_laser_para_update(unit, port, &port_laser_para);
			if (ret_val != BCM_E_NONE){
				return ret_val;
			}
			*value = port_laser_para.temp;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_LASER_SUPPLY_VOLTAGE:
			ret_val = _bcm_tk371x_port_laser_para_update(unit, port, &port_laser_para);
			if (ret_val != BCM_E_NONE){
				return ret_val;
			}
			*value = port_laser_para.vcc;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_LASER_TX_BIAS:
			ret_val = _bcm_tk371x_port_laser_para_update(unit, port, &port_laser_para);
			if (ret_val != BCM_E_NONE){
				return ret_val;
			}
			*value = port_laser_para.tx_bias;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_LASER_TX_POWER:
			ret_val = _bcm_tk371x_port_laser_para_update(unit, port, &port_laser_para);
			if (ret_val != BCM_E_NONE){
				return ret_val;
			}
			*value = port_laser_para.tx_power;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_LASER_RX_POWER:
			ret_val = _bcm_tk371x_port_laser_para_update(unit, port, &port_laser_para);
			if (ret_val != BCM_E_NONE){
				return ret_val;
			}
			*value = port_laser_para.rx_power;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_LASER_TX_POWER_TIME:
			*value = tx_larse_config.txpws_control.Action;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_LASER_TX_POWER_MODE:
			*value = tx_larse_config.txpws_control.OpticalTransmitterID;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_LASER_TX_STATUS:{
			EponTxLaserStatus tx_larser_status;
			ret_val = TkExtOamGetLaserOn(unit, &tx_larser_status);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = (int)tx_larser_status;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_LASER_RX_STATE:{
			TkEponStatus status;
			ret_val = TkExtOamGetMLLIDLinkStatus(unit, 0, &status);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = (int)status.rxOptState;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_HIGH_ALARM_STATE:{
		/* Transceiver Receive power high alarm state */
			ret_val = _soc_ea_alarm_enable_get(unit, port,
				SOC_EA_EVENT_RX_POWER_HIGH_ALARM, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_HIGH_ALARM_REPORT_THRESHOLD:
		/* Transceiver Receive power high alarm report threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_RX_POWER_HIGH_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.raiseThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_HIGH_ALARM_CLEAR_THRESHOLD:
		/* Transceiver Receive power high alarm clear threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_RX_POWER_HIGH_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.clearThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_LOW_ALARM_STATE:{
		/* Transceiver Receive power low alarm state */
			ret_val = _soc_ea_alarm_enable_get(unit, port,
					SOC_EA_EVENT_RX_POWER_LOW_ALARM, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_LOW_ALARM_REPORT_THRESHOLD:
		/* Transceiver Receive power low alarm report threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_RX_POWER_LOW_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.raiseThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_LOW_ALARM_CLEAR_THRESHOLD:
		/* Transceiver Receive power low alarm clear threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_RX_POWER_LOW_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.clearThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_HIGH_ALARM_STATE:{
		/* Transceiver Transmit power high alarm state */
			ret_val = _soc_ea_alarm_enable_get(unit, port,
					SOC_EA_EVENT_TX_POWER_HIGH_ALARM, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_HIGH_ALARM_REPORT_THRESHOLD:
		/* Transceiver Transmit power high alarm report threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_POWER_HIGH_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.raiseThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_HIGH_ALARM_CLEAR_THRESHOLD:
		/* Transceiver Transmit power high alarm clear threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_POWER_HIGH_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.clearThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_LOW_ALARM_STATE: {
		/* Transceiver Transmit power low alarm state */
			ret_val = _soc_ea_alarm_enable_get(unit, port,
					SOC_EA_EVENT_TX_POWER_LOW_ALARM, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_LOW_ALARM_REPORT_THRESHOLD:
		/* Transceiver Transmit power low alarm report threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_POWER_LOW_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.raiseThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_LOW_ALARM_CLEAR_THRESHOLD:
		/* Transceiver Transmit power low alarm clear threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_POWER_LOW_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.clearThreshold;
			return BCM_E_NONE;

		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_HIGH_ALARM_STATE: {
			/* Transceiver Transmit bias high alarm state */
			ret_val = _soc_ea_alarm_enable_get(unit, port,
					SOC_EA_EVENT_TX_BIAS_HIGH_ALARM, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_HIGH_ALARM_REPORT_THRESHOLD:
		/* Transceiver Transmit bias high alarm report threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_BIAS_HIGH_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.raiseThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_HIGH_ALARM_CLEAR_THRESHOLD:
		/* Transceiver Transmit bias high alarm clear threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_BIAS_HIGH_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.clearThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_LOW_ALARM_STATE: {
		/* Transceiver Transmit bias low alarm state */
			ret_val = _soc_ea_alarm_enable_get(unit, port,
					SOC_EA_EVENT_TX_BIAS_LOW_ALARM, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_LOW_ALARM_REPORT_THRESHOLD:
		/* Transceiver Transmit bias low alarm report threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_BIAS_LOW_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.raiseThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_LOW_ALARM_CLEAR_THRESHOLD:
		/* Transceiver Transmit bias low alarm clear threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_BIAS_LOW_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.clearThreshold;

			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_VCC_HIGH_ALARM_STATE: {
		/* Transceiver supply voltage high alarm state*/
			ret_val = _soc_ea_alarm_enable_get(unit, port,
					SOC_EA_EVENT_VCC_HIGH_ALARM, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_VCC_HIGH_ALARM_REPORT_THRESHOLD:
		/*Transceiver supply voltage high alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_VCC_HIGH_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.raiseThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_VCC_HIGH_ALARM_CLEAR_THRESHOLD:
		/* Transceiver supply voltage high alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_VCC_HIGH_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.clearThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_VCC_LOW_ALARM_STATE: {
		/*Transceiver supply voltage low alarm state*/
			ret_val = _soc_ea_alarm_enable_get(unit, port,
					SOC_EA_EVENT_VCC_LOW_ALARM, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_VCC_LOW_ALARM_REPORT_THRESHOLD:
		/*Transceiver supply voltage low alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_VCC_LOW_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.raiseThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_VCC_LOW_ALARM_CLEAR_THRESHOLD:
		/*Transceiver supply voltage low alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_VCC_LOW_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.clearThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TEMP_HIGH_ALARM_STATE: {
		/*Transceiver temperature high alarm state*/
			ret_val = _soc_ea_alarm_enable_get(unit, port,
					SOC_EA_EVENT_TEMP_HIGH_ALARM, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TEMP_HIGH_ALARM_REPORT_THRESHOLD:
		/*Transceiver temperature high alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TEMP_HIGH_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.raiseThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TEMP_HIGH_ALARM_CLEAR_THRESHOLD:
		/*Transceiver temperature high alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TEMP_HIGH_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.clearThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TEMP_LOW_ALARM_STATE: {
		/*Transceiver temperature low alarm state*/
			ret_val = _soc_ea_alarm_enable_get(unit, port,
					SOC_EA_EVENT_TEMP_LOW_ALARM, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TEMP_LOW_ALARM_REPORT_THRESHOLD:
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TEMP_LOW_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.raiseThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TEMP_LOW_ALARM_CLEAR_THRESHOLD:
		/*Transceiver temperature low alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TEMP_LOW_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.clearThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_HIGH_WARNING_STATE: {
		/*Transceiver Receive power high warning alarm state*/
			ret_val = _soc_ea_alarm_enable_get(unit, port,
					SOC_EA_EVENT_RX_POWER_HIGH_WARNING, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_HIGH_WARNING_REPORT_THRESHOLD:
		/*Transceiver Receive power high warning alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_RX_POWER_HIGH_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.raiseThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_HIGH_WARNING_CLEAR_THRESHOLD:
			/*Transceiver Receive power high warning alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_RX_POWER_HIGH_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.clearThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_LOW_WARNING_STATE: {
			/*Transceiver Receive power low warning alarm state*/
			ret_val = _soc_ea_alarm_enable_get(unit, port,
					SOC_EA_EVENT_RX_POWER_LOW_WARNING, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_LOW_WARNING_REPORT_THRESHOLD:
			/*Transceiver Receive power low warning alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_RX_POWER_LOW_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.raiseThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_LOW_WARNING_CLEAR_THRESHOLD:
			/*Transceiver Receive power low warning alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_RX_POWER_LOW_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.clearThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_HIGH_WARNING_STATE: {
			/*Transceiver Transmit power high warning alarm state*/
			ret_val = _soc_ea_alarm_enable_get(unit, port,
					SOC_EA_EVENT_TX_POWER_HIGH_WARNING, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_HIGH_WARNING_REPORT_THRESHOLD:
			/*Transceiver Transmit power high warning alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_POWER_HIGH_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.raiseThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_HIGH_WARNING_CLEAR_THRESHOLD:
			/*Transceiver Transmit power high warning alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_POWER_HIGH_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.clearThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_LOW_WARNING_STATE: {
			/*Transceiver Transmit power low warning alarm state*/
			ret_val = _soc_ea_alarm_enable_get(unit, port,
					SOC_EA_EVENT_TX_POWER_LOW_WARNING, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_LOW_WARNING_REPORT_THRESHOLD:
			/*Transceiver Transmit power low warning alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_POWER_LOW_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.raiseThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_LOW_WARNING_CLEAR_THRESHOLD:
			/*Transceiver Transmit power low warning alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_POWER_LOW_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.clearThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_HIGH_WARNING_STATE: {
			/*Transceiver Transmit bias high warning alarm state*/
			ret_val = _soc_ea_alarm_enable_get(unit, port,
					SOC_EA_EVENT_TX_BIAS_HIGH_WARNING, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_HIGH_WARNING_REPORT_THRESHOLD:
			/*Transceiver Transmit bias high warning alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_BIAS_HIGH_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.raiseThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_HIGH_WARNING_CLEAR_THRESHOLD:
			/*Transceiver Transmit bias high warning alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_BIAS_HIGH_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.clearThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_LOW_WARNING_STATE: {
			/*Transceiver Transmit bias low warning alarm state*/
			ret_val = _soc_ea_alarm_enable_get(unit, port,
					SOC_EA_EVENT_TX_BIAS_LOW_WARNING, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_LOW_WARNING_REPORT_THRESHOLD:
			/*Transceiver Transmit bias low warning alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_BIAS_LOW_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.raiseThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_LOW_WARNING_CLEAR_THRESHOLD:
			/*Transceiver Transmit bias low warning alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_BIAS_LOW_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.clearThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_VCC_HIGH_WARNING_STATE: {
			/*Transceiver supply voltage high warning alarm state*/
			ret_val = _soc_ea_alarm_enable_get(unit, port,
					SOC_EA_EVENT_VCC_HIGH_WARNING, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_VCC_HIGH_WARNING_REPORT_THRESHOLD:
			/*Transceiver supply voltage high warning alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_VCC_HIGH_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.raiseThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_VCC_HIGH_WARNING_CLEAR_THRESHOLD:
			/*Transceiver supply voltage high warning alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_VCC_HIGH_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.clearThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_VCC_LOW_WARNING_STATE: {
			/*Transceiver supply voltage low warning alarm state*/
			ret_val = _soc_ea_alarm_enable_get(unit, port,
					SOC_EA_EVENT_VCC_LOW_WARNING, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_VCC_LOW_WARNING_REPORT_THRESHOLD:
			/*Transceiver supply voltage low warning alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_VCC_LOW_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.raiseThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_VCC_LOW_WARNING_CLEAR_THRESHOLD:
			/*Transceiver supply voltage low warning alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_VCC_LOW_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.clearThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TEMP_HIGH_WARNING_STATE: {
			/*Transceiver temperature high warning alarm state*/
			ret_val = _soc_ea_alarm_enable_get(unit, port,
					SOC_EA_EVENT_TEMP_HIGH_WARNING, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TEMP_HIGH_WARNING_REPORT_THRESHOLD:
			/*Transceiver temperature high warning alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TEMP_HIGH_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.raiseThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TEMP_HIGH_WARNING_CLEAR_THRESHOLD:
			/*Transceiver temperature high warning alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TEMP_HIGH_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.clearThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TEMP_LOW_WARNING_STATE: {
			/*Transceiver temperature low warning alarm state*/
			ret_val = _soc_ea_alarm_enable_get(unit, port,
					SOC_EA_EVENT_TEMP_LOW_WARNING, &state);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = state;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TEMP_LOW_WARNING_REPORT_THRESHOLD:
			/*Transceiver temperature low warning alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TEMP_LOW_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.raiseThreshold;
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TEMP_LOW_WARNING_CLEAR_THRESHOLD:
			/*Transceiver temperature low warning alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TEMP_LOW_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			*value = threshold.clearThreshold;
			return BCM_E_NONE;
		default:
			return BCM_E_UNAVAIL;
	}
}





/* Set/Get PHY specific configurations. */
int
_bcm_ea_port_phy_control_set(
	    int unit,
	    bcm_port_t port,
	    bcm_port_phy_control_t type,
	    uint32 value)
{
	int ret_val;
	_soc_ea_alarm_threshold_t threshold;

	if (port != 0){
		return BCM_E_UNAVAIL;
	}
	switch(type){
		case BCM_PORT_PHY_CONTROL_INTERFACE:
			ret_val = _soc_ea_phy_control_if_set(unit, (uint8)value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_LASER_TX_POWER_TIME:{
			soc_base_mac_t base_mac;
			ret_val = soc_base_mac_get(unit, &base_mac);
			if (ret_val != SOC_E_NONE){
				return BCM_E_FAIL;
			}
			tx_larse_config.flag |= PORT_PON_LASER_TX_CONFIG_TIME;
			tx_larse_config.txpws_control.Action = value;
			sal_memcpy((void*)(tx_larse_config.txpws_control.ONUID),
							(void*)base_mac.pon_base_mac, sizeof(bcm_mac_t));
			if (tx_larse_config.flag == PORT_PON_LASER_TX_CONFIG_TIME_MODE){
				ret_val = TKCTCExtOamSetLaserTxPowerAdminCtl(unit, 0,
							&tx_larse_config.txpws_control);
				if (ret_val != OK){
					return BCM_E_FAIL;
				}
			}
			tx_larse_config.flag = 0;
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_LASER_TX_POWER_MODE:{
			soc_base_mac_t base_mac;
			ret_val = soc_base_mac_get(unit, &base_mac);
			if (ret_val != SOC_E_NONE){
				return BCM_E_FAIL;
			}
			tx_larse_config.flag |= PORT_PON_LASER_TX_CONFIG_MODE;
			tx_larse_config.txpws_control.OpticalTransmitterID = value;
			sal_memcpy((void*)(tx_larse_config.txpws_control.ONUID),
								(void*)base_mac.pon_base_mac, sizeof(bcm_mac_t));
			if (tx_larse_config.flag == PORT_PON_LASER_TX_CONFIG_TIME_MODE){
				ret_val = TKCTCExtOamSetLaserTxPowerAdminCtl(unit, 0,
							&tx_larse_config.txpws_control);
				if (ret_val != OK){
					return BCM_E_FAIL;
				}
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_LASER_TX_STATUS:{			
			EponTxLaserStatus status;
			switch (value){
				case 0:
				case 1:
				case 2:
					status = value;
					break;
				default:
					return BCM_E_PARAM;
			}
			ret_val = TkExtOamSetLaserOn(unit, status);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
			
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_HIGH_ALARM_STATE:{
		/* Transceiver Receive power high alarm state */
			if (value != 0 && value != 1){
				return BCM_E_PARAM;
			}
			ret_val = _soc_ea_alarm_enable_set(unit, port,
				SOC_EA_EVENT_RX_POWER_HIGH_ALARM, value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_HIGH_ALARM_REPORT_THRESHOLD:
		/* Transceiver Receive power high alarm report threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_RX_POWER_HIGH_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.raiseThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_RX_POWER_HIGH_ALARM, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_HIGH_ALARM_CLEAR_THRESHOLD:
		/* Transceiver Receive power high alarm clear threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_RX_POWER_HIGH_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.clearThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_RX_POWER_HIGH_ALARM, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_LOW_ALARM_STATE:{
		/* Transceiver Receive power low alarm state */
			if (value != 0 && value != 1){
				return BCM_E_PARAM;
			}
			ret_val = _soc_ea_alarm_enable_set(unit, port,
					SOC_EA_EVENT_RX_POWER_LOW_ALARM, value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_LOW_ALARM_REPORT_THRESHOLD:
		/* Transceiver Receive power low alarm report threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_RX_POWER_LOW_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.raiseThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_RX_POWER_LOW_ALARM, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_LOW_ALARM_CLEAR_THRESHOLD:
		/* Transceiver Receive power low alarm clear threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_RX_POWER_LOW_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.clearThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_RX_POWER_LOW_ALARM, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_HIGH_ALARM_STATE:{
		/* Transceiver Transmit power high alarm state */
			if (value != 0 && value != 1){
				return BCM_E_PARAM;
			}
			ret_val = _soc_ea_alarm_enable_set(unit, port,
					SOC_EA_EVENT_TX_POWER_HIGH_ALARM, value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_HIGH_ALARM_REPORT_THRESHOLD:
		/* Transceiver Transmit power high alarm report threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_POWER_HIGH_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.raiseThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TX_POWER_HIGH_ALARM, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_HIGH_ALARM_CLEAR_THRESHOLD:
		/* Transceiver Transmit power high alarm clear threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_POWER_HIGH_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.clearThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TX_POWER_HIGH_ALARM, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_LOW_ALARM_STATE: {
		/* Transceiver Transmit power low alarm state */
			if (value != 0 && value != 1){
				return BCM_E_PARAM;
			}
			ret_val = _soc_ea_alarm_enable_set(unit, port,
					SOC_EA_EVENT_TX_POWER_LOW_ALARM, value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_LOW_ALARM_REPORT_THRESHOLD:
		/* Transceiver Transmit power low alarm report threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_POWER_LOW_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.raiseThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TX_POWER_LOW_ALARM, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_LOW_ALARM_CLEAR_THRESHOLD:
		/* Transceiver Transmit power low alarm clear threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_POWER_LOW_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.clearThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TX_POWER_LOW_ALARM, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;

		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_HIGH_ALARM_STATE: {
			/* Transceiver Transmit bias high alarm state */
			if (value != 0 && value != 1){
				return BCM_E_PARAM;
			}
			ret_val = _soc_ea_alarm_enable_set(unit, port,
					SOC_EA_EVENT_TX_BIAS_HIGH_ALARM, value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_HIGH_ALARM_REPORT_THRESHOLD:
		/* Transceiver Transmit bias high alarm report threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_BIAS_HIGH_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.raiseThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TX_BIAS_HIGH_ALARM, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_HIGH_ALARM_CLEAR_THRESHOLD:
		/* Transceiver Transmit bias high alarm clear threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_BIAS_HIGH_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.clearThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TX_BIAS_HIGH_ALARM, threshold);
			if (ret_val != OK){
				return BCM_E_INTERNAL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_LOW_ALARM_STATE: {
		/* Transceiver Transmit bias low alarm state */
			if (value != 0 && value != 1){
				return BCM_E_PARAM;
			}
			ret_val = _soc_ea_alarm_enable_set(unit, port,
					SOC_EA_EVENT_TX_BIAS_LOW_ALARM, value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_LOW_ALARM_REPORT_THRESHOLD:
		/* Transceiver Transmit bias low alarm report threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_BIAS_LOW_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_INTERNAL;
			}
			threshold.raiseThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TX_BIAS_LOW_ALARM, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_LOW_ALARM_CLEAR_THRESHOLD:
		/* Transceiver Transmit bias low alarm clear threshold */
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_BIAS_LOW_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.clearThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TX_BIAS_LOW_ALARM, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_VCC_HIGH_ALARM_STATE: {
		/* Transceiver supply voltage high alarm state*/
			if (value != 0 && value != 1){
				return BCM_E_PARAM;
			}
			ret_val = _soc_ea_alarm_enable_set(unit, port,
					SOC_EA_EVENT_VCC_HIGH_ALARM, value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_VCC_HIGH_ALARM_REPORT_THRESHOLD:
		/*Transceiver supply voltage high alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_VCC_HIGH_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.raiseThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_VCC_HIGH_ALARM, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_VCC_HIGH_ALARM_CLEAR_THRESHOLD:
		/* Transceiver supply voltage high alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_VCC_HIGH_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.clearThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_VCC_HIGH_ALARM, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_VCC_LOW_ALARM_STATE: {
		/*Transceiver supply voltage low alarm state*/
			if (value != 0 && value != 1){
				return BCM_E_PARAM;
			}
			ret_val = _soc_ea_alarm_enable_set(unit, port,
					SOC_EA_EVENT_VCC_LOW_ALARM, value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_VCC_LOW_ALARM_REPORT_THRESHOLD:
		/*Transceiver supply voltage low alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_VCC_LOW_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.raiseThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_VCC_LOW_ALARM, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_VCC_LOW_ALARM_CLEAR_THRESHOLD:
		/*Transceiver supply voltage low alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_VCC_LOW_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_INTERNAL;
			}
			threshold.clearThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_VCC_LOW_ALARM, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TEMP_HIGH_ALARM_STATE: {
		/*Transceiver temperature high alarm state*/
			if (value != 0 && value != 1){
				return BCM_E_PARAM;
			}
			ret_val = _soc_ea_alarm_enable_set(unit, port,
					SOC_EA_EVENT_TEMP_HIGH_ALARM, value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TEMP_HIGH_ALARM_REPORT_THRESHOLD:
		/*Transceiver temperature high alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TEMP_HIGH_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.raiseThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TEMP_HIGH_ALARM, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TEMP_HIGH_ALARM_CLEAR_THRESHOLD:
		/*Transceiver temperature high alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TEMP_HIGH_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.clearThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TEMP_HIGH_ALARM, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TEMP_LOW_ALARM_STATE: {
		/*Transceiver temperature low alarm state*/
			if (value != 0 && value != 1){
				return BCM_E_PARAM;
			}
			ret_val = _soc_ea_alarm_enable_set(unit, port,
					SOC_EA_EVENT_TEMP_LOW_ALARM, value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TEMP_LOW_ALARM_REPORT_THRESHOLD:
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TEMP_LOW_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.raiseThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TEMP_LOW_ALARM, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TEMP_LOW_ALARM_CLEAR_THRESHOLD:
		/*Transceiver temperature low alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TEMP_LOW_ALARM, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.clearThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TEMP_LOW_ALARM, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_HIGH_WARNING_STATE: {
		/*Transceiver Receive power high warning alarm state*/
			if (value != 0 && value != 1){
				return BCM_E_PARAM;
			}
			ret_val = _soc_ea_alarm_enable_set(unit, port,
					SOC_EA_EVENT_RX_POWER_HIGH_WARNING, value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_HIGH_WARNING_REPORT_THRESHOLD:
		/*Transceiver Receive power high warning alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_RX_POWER_HIGH_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.raiseThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_RX_POWER_HIGH_WARNING, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_HIGH_WARNING_CLEAR_THRESHOLD:
			/*Transceiver Receive power high warning alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_RX_POWER_HIGH_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.clearThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_RX_POWER_HIGH_WARNING, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;

		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_LOW_WARNING_STATE: {
			/*Transceiver Receive power low warning alarm state*/
			if (value != 0 && value != 1){
				return BCM_E_PARAM;
			}
			ret_val = _soc_ea_alarm_enable_set(unit, port,
					SOC_EA_EVENT_RX_POWER_LOW_WARNING, value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_LOW_WARNING_REPORT_THRESHOLD:
			/*Transceiver Receive power low warning alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_RX_POWER_LOW_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.raiseThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_RX_POWER_LOW_WARNING, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_RX_POWER_LOW_WARNING_CLEAR_THRESHOLD:
			/*Transceiver Receive power low warning alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_RX_POWER_LOW_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.clearThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_RX_POWER_LOW_WARNING, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_HIGH_WARNING_STATE: {
			/*Transceiver Transmit power high warning alarm state*/
			if (value != 0 && value != 1){
				return BCM_E_PARAM;
			}
			ret_val = _soc_ea_alarm_enable_set(unit, port,
					SOC_EA_EVENT_TX_POWER_HIGH_WARNING, value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_HIGH_WARNING_REPORT_THRESHOLD:
			/*Transceiver Transmit power high warning alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_POWER_HIGH_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.raiseThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TX_POWER_HIGH_WARNING, threshold);
			if (ret_val != OK){
				return BCM_E_INTERNAL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_HIGH_WARNING_CLEAR_THRESHOLD:
			/*Transceiver Transmit power high warning alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_POWER_HIGH_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.clearThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TX_POWER_HIGH_WARNING, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_LOW_WARNING_STATE: {
			/*Transceiver Transmit power low warning alarm state*/
			if (value != 0 && value != 1){
				return BCM_E_PARAM;
			}
			ret_val = _soc_ea_alarm_enable_set(unit, port,
					SOC_EA_EVENT_TX_POWER_LOW_WARNING, value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_LOW_WARNING_REPORT_THRESHOLD:
			/*Transceiver Transmit power low warning alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_POWER_LOW_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.raiseThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TX_POWER_LOW_WARNING, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_POWER_LOW_WARNING_CLEAR_THRESHOLD:
			/*Transceiver Transmit power low warning alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_POWER_LOW_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.clearThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TX_POWER_LOW_WARNING, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_HIGH_WARNING_STATE: {
			/*Transceiver Transmit bias high warning alarm state*/
			if (value != 0 && value != 1){
				return BCM_E_PARAM;
			}
			ret_val = _soc_ea_alarm_enable_set(unit, port,
					SOC_EA_EVENT_TX_BIAS_HIGH_WARNING, value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_HIGH_WARNING_REPORT_THRESHOLD:
			/*Transceiver Transmit bias high warning alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_BIAS_HIGH_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.raiseThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TX_BIAS_HIGH_WARNING, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_HIGH_WARNING_CLEAR_THRESHOLD:
			/*Transceiver Transmit bias high warning alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_BIAS_HIGH_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.clearThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TX_BIAS_HIGH_WARNING, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_LOW_WARNING_STATE: {
			/*Transceiver Transmit bias low warning alarm state*/
			if (value != 0 && value != 1){
				return BCM_E_PARAM;
			}
			ret_val = _soc_ea_alarm_enable_set(unit, port,
					SOC_EA_EVENT_TX_BIAS_LOW_WARNING, value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_LOW_WARNING_REPORT_THRESHOLD:
			/*Transceiver Transmit bias low warning alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_BIAS_LOW_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.raiseThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TX_BIAS_LOW_WARNING, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TX_BIAS_LOW_WARNING_CLEAR_THRESHOLD:
			/*Transceiver Transmit bias low warning alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TX_BIAS_LOW_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.clearThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TX_BIAS_LOW_WARNING, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_VCC_HIGH_WARNING_STATE: {
			/*Transceiver supply voltage high warning alarm state*/
			if (value != 0 && value != 1){
				return BCM_E_PARAM;
			}
			ret_val = _soc_ea_alarm_enable_set(unit, port,
					SOC_EA_EVENT_VCC_HIGH_WARNING, value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_VCC_HIGH_WARNING_REPORT_THRESHOLD:
			/*Transceiver supply voltage high warning alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_VCC_HIGH_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.raiseThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_VCC_HIGH_WARNING, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_VCC_HIGH_WARNING_CLEAR_THRESHOLD:
			/*Transceiver supply voltage high warning alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_VCC_HIGH_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.clearThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_VCC_HIGH_WARNING, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_VCC_LOW_WARNING_STATE: {
			/*Transceiver supply voltage low warning alarm state*/
			if (value != 0 && value != 1){
				return BCM_E_PARAM;
			}
			ret_val = _soc_ea_alarm_enable_set(unit, port,
					SOC_EA_EVENT_VCC_LOW_WARNING, value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_VCC_LOW_WARNING_REPORT_THRESHOLD:
			/*Transceiver supply voltage low warning alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_VCC_LOW_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.raiseThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_VCC_LOW_WARNING, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_VCC_LOW_WARNING_CLEAR_THRESHOLD:
			/*Transceiver supply voltage low warning alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_VCC_LOW_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.clearThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_VCC_LOW_WARNING, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TEMP_HIGH_WARNING_STATE: {
			/*Transceiver temperature high warning alarm state*/
			if (value != 0 && value != 1){
				return BCM_E_PARAM;
			}
			ret_val = _soc_ea_alarm_enable_set(unit, port,
					SOC_EA_EVENT_TEMP_HIGH_WARNING, value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TEMP_HIGH_WARNING_REPORT_THRESHOLD:
			/*Transceiver temperature high warning alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TEMP_HIGH_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.raiseThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TEMP_HIGH_WARNING, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TEMP_HIGH_WARNING_CLEAR_THRESHOLD:
			/*Transceiver temperature high warning alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TEMP_HIGH_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.clearThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TEMP_HIGH_WARNING, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TEMP_LOW_WARNING_STATE: {
			/*Transceiver temperature low warning alarm state*/
			if (value != 0 && value != 1){
				return BCM_E_PARAM;
			}
			ret_val = _soc_ea_alarm_enable_set(unit, port,
					SOC_EA_EVENT_TEMP_LOW_WARNING, value);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		}
		case BCM_PORT_PHY_CONTROL_PON_TEMP_LOW_WARNING_REPORT_THRESHOLD:
			/*Transceiver temperature low warning alarm report threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TEMP_LOW_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.raiseThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TEMP_LOW_WARNING, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		case BCM_PORT_PHY_CONTROL_PON_TEMP_LOW_WARNING_CLEAR_THRESHOLD:
			/*Transceiver temperature low warning alarm clear threshold*/
			ret_val = _soc_ea_alarm_threshold_get(unit, port,
					SOC_EA_EVENT_TEMP_LOW_WARNING, &threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			threshold.clearThreshold = value;
			ret_val = _soc_ea_alarm_threshold_set(unit, port,
					SOC_EA_EVENT_TEMP_LOW_WARNING, threshold);
			if (ret_val != OK){
				return BCM_E_FAIL;
			}
			return BCM_E_NONE;
		default:
			return BCM_E_UNAVAIL;
	}
}

#define PortSpeed10M		10
#define PortSpeed100M	 	100
#define PortSpeed1G			1000
#define PortSpeed2DOT5G		2500
#define PortSpeed10G		10000
/* Get or set the current operating speed of a port. */
int
_bcm_ea_port_speed_get(
	    int unit,
	    bcm_port_t port,
	    int *speed)
{
	_soc_ea_oam_auto_neg_admin_state_t auto_neg_state;
	uint16 _speed;
	_soc_ea_oam_mac_duplex_status_t duplex_state;
	int rc = BCM_E_NONE;

	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "_bcm_ea_port_speed_get...\n")));
	if (port < _BCM_TK371X_UNI_PORT_BASE || port >= _BCM_TK371X_LLID_PORT_BASE){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "return BCM_E_PORT, invalid port is %d\n"), port));
		return BCM_E_PORT;
	}

	rc = _soc_ea_auto_neg_get((uint8)unit, 0,
			(uint8)port, &auto_neg_state, &_speed, &duplex_state);
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "_soc_ea_auto_neg_get, rv=%d,"
                             "unit = %d, port = %d\n"
                             "autoneg_state=%d, speed=%d duplex=%d\n"),
                  rc, unit, port,
                  (uint8)auto_neg_state, (uint16)_speed, (uint8)duplex_state));
	if (rc != OK){
		return BCM_E_FAIL;
	}
	switch (_speed){
		case PortSpeed10M:
			*speed = DRV_PORT_STATUS_SPEED_10M;
			break;
		case PortSpeed100M:
			*speed = DRV_PORT_STATUS_SPEED_100M;
			break;
		case PortSpeed1G:
			*speed = DRV_PORT_STATUS_SPEED_1G;
			break;
		case PortSpeed2DOT5G:
			*speed = DRV_PORT_STATUS_SPEED_2500M;
			break;
		case PortSpeed10G:
			*speed = DRV_PORT_STATUS_SPEED_10G;
			break;
		default:
			*speed = _speed;
	}
	return BCM_E_NONE;
}

/* Get or set the current operating speed of a port. */
#define PortAutoNegDisabled 1
#define PortAutoNegEnabled 	2
#define PortMacDuplexModeHalf  		1
#define PortMacDuplexModeFull 		2
#define PortMacDuplexModeUnknown 	3

int
_bcm_ea_port_speed_set(
	    int unit,
	    bcm_port_t port,
	    int speed)
{
	int rv = BCM_E_NONE;
	_soc_ea_oam_auto_neg_admin_state_t auto_neg_state;
	_soc_ea_oam_mac_duplex_status_t duplex_state;
	uint16 drv_speed;

	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "_bcm_ea_port_speed_set...\n")));
	if (port < _BCM_TK371X_UNI_PORT_BASE || port >= _BCM_TK371X_LLID_PORT_BASE){
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "return BCM_E_PORT, invalid port is %d\n"), port));
		return BCM_E_PORT;
	}

	if (speed > 65535){
		drv_speed = 65535;
	}
	switch (speed){
		case DRV_PORT_STATUS_SPEED_10M:
			drv_speed = PortSpeed10M;
			break;
		case DRV_PORT_STATUS_SPEED_100M:
			drv_speed = PortSpeed100M;
			break;
		case DRV_PORT_STATUS_SPEED_1G:
			drv_speed = PortSpeed1G;
			break;
		case DRV_PORT_STATUS_SPEED_2500M:
			drv_speed = PortSpeed2DOT5G;
			break;
		case DRV_PORT_STATUS_SPEED_10G:
			drv_speed = PortSpeed10G;
			break;
		default:
			drv_speed = speed;
			break;
	}
	_soc_ea_auto_neg_get((uint8)unit, 0,
							  (uint8)port, &auto_neg_state,
							  &drv_speed, &duplex_state);
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "_soc_ea_auto_neg_get,"
                             "unit = %d, port = %d\n"
                             "autoneg_state=%d, speed=%d duplex=%d\n"),
                  unit, port,
                  (uint8)auto_neg_state, (uint16)drv_speed, (uint8)duplex_state));

	rv = _soc_ea_auto_neg_set((uint8)unit, 0, (uint8)port,
							auto_neg_state, drv_speed, duplex_state);
	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "_soc_ea_auto_neg_set, rv=%d,"
                             "unit = %d, port = %d\n"
                             "autoneg_state=%d, drv_speed=%d duplex=%d\n"),
                  rv, unit, port,
                  (uint8)auto_neg_state, (uint16)drv_speed, (uint8)duplex_state));
	if (rv != OK){
		return BCM_E_FAIL;
	}
	return BCM_E_NONE;
}

int
_bcm_ea_port_link_status_get(int unit, bcm_port_t port, int *up){
	int rv = BCM_E_NONE;
	uint8 status;
	TkEponStatus pon_status;

	LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "_bcm_ea_port_link_status_get..\n")));
	sal_memset(&pon_status, 0, sizeof(TkEponStatus));
	if (port == _BCM_TK371X_PON_PORT_BASE){
		rv = TkExtOamGetMLLIDLinkStatus(unit, port, &pon_status);
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "TkExtOamGetMLLIDLinkStatus: unit=%d, port=%d, rv=%d\n"), unit, port, rv));
		if (rv != OK){
			return BCM_E_FAIL;
		}
		*up = pon_status.connection;
		return BCM_E_NONE;
	}else if (port > _BCM_TK371X_PON_PORT_BASE && port < _BCM_TK371X_LLID_PORT_BASE){
		rv = _soc_ea_eth_link_state_get((uint8)unit, 0, (uint8)port, &status);
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "_soc_ea_eth_link_state_get: unit=%d, port=%d, rv=%d\n"), unit, port, rv));
		if (rv != OK){
			return BCM_E_FAIL;
		}
		*up = status;
		return BCM_E_NONE;
	}else if ((port >= _BCM_TK371X_LLID_PORT_BASE) &&
			(port < _BCM_TK371X_LLID_PORT_BASE + _BCM_TK371X_MAX_LLID_PORT_NUM)){
		rv = TkExtOamGetMLLIDLinkStatus(unit, 0, &pon_status);
		LOG_INFO(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "TkExtOamGetMLLIDLinkStatus: unit=%d, port=%d, rv=%d\n"), unit, 0, rv));
		if (rv != OK){
			return BCM_E_FAIL;
		}
		*up = pon_status.linkInfo[port - _BCM_TK371X_LLID_PORT_BASE].authorizationState;
		return BCM_E_NONE;
	}else{
		return BCM_E_PORT;
	}
	return rv;
}


int _bcm_ea_port_advert_get(int unit,
        bcm_port_t port,
        bcm_port_abil_t *ability){

	return BCM_E_UNAVAIL;
}

int
_bcm_ea_port_advert_remote_get(int unit, bcm_port_t port,
               bcm_port_abil_t *ability_mask){
	return BCM_E_UNAVAIL;
}

