/*
 * $Id$
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __FRAMER_H__
#define __FRAMER_H__


#include "ag_basic_types.h"
#include "pub/nd_hw.h"
/*ORI*/
typedef struct 
{
	AgNdCircuit        		n_circuit_id;
	AG_BOOL 				b_enable;
	AG_BOOL					b_master;
	AG_BOOL 				b_t1;

} AgFramerConfig;

typedef enum
{
	ALARM_RX_AIS,
	ALARM_RX_LOS,
	ALARM_RX_OOF,
	ALARM_TX_AIS,
	ALARM_TX_RAI
}TE1LCLBK_ORI;

/*change */
typedef enum
{
	ALARM_TX_UNFRAME_AIS,
	ALARM_TX_T1_J1_RAI,
	ALARM_TX_FRAME_AIS,
	ALARM_TX_FRAME_RAI,
	SIGNAL_TX_AIS,
	SIGNAL_TX_OOF,
	SIGNAL_TX_LOS,
	SIGNAL_TX_RAI
}AgTramerTransmitAlarms;

typedef struct
{
	AgNdCircuit        		n_circuit_id;
	AG_BOOL 				b_loopback_deactivation_code;
	AG_BOOL					b_loopback_activation_code;
	AG_BOOL					b_sa6_change_unlatched_status;
	AG_BOOL                 b_prm_status;
	AG_BOOL 				b_excessive_zeroes_status;
	AG_BOOL 				b_cas_ts_16_ais_status;
	AG_BOOL 				b_cas_ts_16_los_status;
	AG_BOOL 				b_recive_bits_status;
	AG_BOOL 				b_signal_multiframe_error_status;
	AG_BOOL 				b_crc_error_status;
	AG_BOOL 				b_frame_alarm_status;
	AG_BOOL 				b_remote_multiframe_error_status;
	AG_BOOL 				b_remote_alarm_indec_status;
	AG_BOOL 				b_oof_status;
	AG_BOOL 				b_ais_status;
	AG_BOOL 				b_los_status;
	
}AgFramerPortStatus;

typedef struct 
{
		AgNdCircuit        	n_circuit_id;
		AG_U16				n_t1_oof_config;
		AG_BOOL				b_force_fram_alig_word_error;
		AG_BOOL				b_force_bip_code;
		AG_BOOL				b_force_crc_error;
		AG_BOOL				b_rai_insert_signal_bus;
		AG_BOOL				b_signal_bus_ais_los;
		AG_BOOL				b_signal_bus_ais_oof;
		AG_BOOL				b_signal_bus_ais_ais;
		AG_BOOL				b_forward_rai;
		AG_BOOL				b_forward_ais;
		AG_BOOL				b_force_rai;
		AG_BOOL				b_force_ais;
			
}FramerPortAlarmControl;

typedef enum
{
	LOOPBACK_REMOTE_SWITCH =  0x20000001,	/* R1LB bit 2 Remote line*/
	LOOPBACK_PAYLOAD_SWITCH = 0x20000004,	/*tslb bit 9 + reg per slot or Bit3 payload loop*/
	LOOPBACK_FRAMER_SWITCH =  0x20000008,	/* l1lb bit 1 */
	LOOPBACK_TX_ENABLE = 	  0x40000010,	/* Request tx loop  0x210 bit 6: Loop-Back Enable*/
	LOOPBACK_TX_DONE = 	  	  0x40000020,	/* request done     , from driver */
	LOOPBACK_RX_ACTIVATE =    0x40000040,	/* remote requested loop 0x15c bit 14 Loopback Activation code */
	LOOPBACK_RX_DONE = 		  0x40000080	/* remote request    0x15c Loopback Deactivation Code */ 
} AgFramerLoopbackType;


typedef struct 
{
	AgNdCircuit        		n_circuit_id;
	AG_BOOL 				b_loop_enable;
	AgFramerLoopbackType	e_type;
	AG_U32 					n_mask;
	AG_BOOL					b_loopback_activation_code;
	AG_BOOL					b_loopback_deactivation_code;

} AgFramerPortLoopbackControl;

typedef struct 
{
	AgNdCircuit        		n_circuit_id;
	AG_U16					n_crc_error_counter;
	AG_U16					n_line_code_voi_error;
	AG_U16					n_e_bit_error_count;
	AG_U16					n_transmit_slip_count;
	AG_U16					n_recive_slip_count;
	AG_U16					n_frame_alim_error_count;
	AG_U16					n_change_framer_alim_count;
	AG_U16					n_sev_frame_error_count;
} AgFramerPortPm;

typedef struct 
{
	AgNdCircuit        		n_circuit_id;
	AG_U32                  n_framer_port_timeslot_control;
}AgFramerPortTimeSlotLoopbackControl;



void ag_config_framer(AG_U32 n_port,AG_BOOL b_enable,AG_BOOL b_master);
AG_BOOL ag_check_alarms(AG_U32 n_port,AgFramerPortStatus *palarms);

AG_BOOL ag_set_callback_ori(void (*pfunc)(int line,TE1LCLBK_ORI fcode,int b_enable ,...));

void ag_notify_alarm(AG_U32 n_port,AG_BOOL b_enable,AgFramerPortStatus *palarms);

void ag_transmit_alarm_tx(AG_U32 n_port,TE1LCLBK_ORI e_alarm,AG_BOOL b_enable);

void ag_transmit_alarm(AG_U32 n_port,AgTramerTransmitAlarms e_alarm,AG_BOOL b_enable);

void ag_update_frmaer_pm_counters(AG_U32 n_port,AgFramerPortPm* msg);

void ag_set_loopback_mode(AgFramerLoopbackType e_type,AG_U32 n_port,AG_U32 n_mask,AG_BOOL b_enable);




#endif /*__FRAMER_H__ */

