/*
 * $Id$ 
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __NEMO_REGISTERS_H__
#define __NEMO_REGISTERS_H__

                       
#include "nd_platform.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CES16_BCM_VERSION

typedef unsigned short field_t;


/* */
/* Global / Host interface registers */
/* */

typedef union
{
    struct AgNdRegFpgaId_S
    {
#ifdef BE_HOST
        field_t n_architecture  : 1,
	    n_fpga_code_id  : 7,
	    n_fpga_revision : 8;
#else
	field_t  n_fpga_revision  :  8, 
	    n_fpga_code_id  :  7, 
	    n_architecture  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegFpgaId;



typedef union
{
    struct AgNdRegGlobalControl_S
    {
#ifdef BE_HOST
        field_t n_reserved                    : 1,
	    b_lan_dce_mode_enable_h       : 1, /*ISTTFIX add high bit*/
	    b_lan_dce_mode_enable         : 1,
	    b_wan_dce_mode_enable         : 1,
	    b_rclr_on_host_access         : 1,
	    b_tdm_local_loopback          : 1,
	    b_ucode_memory_locked         : 1,
	    n_one_second_pulse_direction  : 1,
	    n_bit_order                   : 1,
	    b_tdm_remote_loopback         : 1,
	    b_dechannelizer_loopback      : 1,
	    b_channelizer_loopback        : 1,
	    b_packet_classifier_loopback  : 1,
	    b_packet_interface_loopback   : 1,
	    b_packet_local_loopback       : 1,
	    b_packet_remote_loopback      : 1;
#else
	field_t  b_packet_remote_loopback  :  1, 
	    b_packet_local_loopback  :  1, 
	    b_packet_interface_loopback  :  1, 
	    b_packet_classifier_loopback  :  1, 
	    b_channelizer_loopback  :  1, 
	    b_dechannelizer_loopback  :  1, 
	    b_tdm_remote_loopback  :  1, 
	    n_bit_order  :  1, 
	    n_one_second_pulse_direction  :  1, 
	    b_ucode_memory_locked  :  1, 
	    b_tdm_local_loopback  :  1, 
	    b_rclr_on_host_access  :  1, 
	    b_wan_dce_mode_enable  :  1, 
	    b_lan_dce_mode_enable  :  1, 
	    b_lan_dce_mode_enable_h  :  1,  /*ISTTFIX  add  high  bit*/ 
	    n_reserved  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegGlobalControl;




typedef union
{
    struct AgNdRegChannelizerLoopbackChannelSelect_S
    {
#ifdef BE_HOST
        field_t n_reserved       : 5,
	    n_channel_select : 11;
#else
	field_t  n_channel_select  :  11, 
	    n_reserved  :  5; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegChannelizerLoopbackChannelSelect;





typedef union
{
    struct AgNdRegGlobalClkActivityMonitoring_S
    {
#ifdef BE_HOST
        field_t n_oscillator_type             : 1,
	    n_reserved                    : 7,
	    b_one_second_pulse            : 1,
	    b_nomad_brg_clk               : 1,
	    b_miir_clk                    : 1,
	    b_miit_clk                    : 1,
	    b_reference_clk_input2        : 1,
	    b_reference_clk_input1        : 1,
	    b_external_clk_synchronizer   : 1,
	    b_internal_cclk               : 1;
#else
	field_t  b_internal_cclk  :  1, 
	    b_external_clk_synchronizer  :  1, 
	    b_reference_clk_input1  :  1, 
	    b_reference_clk_input2  :  1, 
	    b_miit_clk  :  1, 
	    b_miir_clk  :  1, 
	    b_nomad_brg_clk  :  1, 
	    b_one_second_pulse  :  1, 
	    n_reserved  :  7, 
	    n_oscillator_type  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegGlobalClkActivityMonitoring;




typedef union
{
    struct AgNdRegTestBusControl_S
    {
#ifdef BE_HOST
        field_t n_reserved     : 8,    
	    n_block_select : 4,
	    n_group_select : 4;
#else
	field_t  n_group_select  :  4, 
	    n_block_select  :  4, 
	    n_reserved  :  8; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegTestBusControl;



typedef union
{
    struct AgNdRegExtendedAddress_S
    {
#ifdef BE_HOST
        field_t b_internal_memory     : 1,
	    n_reserved            : 11,
	    n_bank                : 4;
#else
	field_t  n_bank  :  4, 
	    n_reserved  :  11, 
	    b_internal_memory  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegExtendedAddress;

typedef union
{
    struct AgNdRegMemoryConfiguration_S
    {
#ifdef BE_HOST
        field_t n_reserved3			          : 1,
	    n_bank_0_size                 : 3,
	    n_reserved2  		          : 1,
	    n_bank_1_size                 : 3,
	    n_reserved1                   : 5,
	    n_max_pw                      : 3;
#else
	field_t  n_max_pw  :  3, 
	    n_reserved1  :  5, 
	    n_bank_1_size  :  3, 
	    n_reserved2  		  :  1, 
	    n_bank_0_size  :  3, 
	    n_reserved3			  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegMemoryConfiguration;



/* */
/* Interrupt handling registers */
/* */

typedef union
{
    struct AgNdRegGlobalInterruptEnable_S
    {
#ifdef BE_HOST
        field_t n_reserved               : 11,
	    b_performance_monitoring : 1,
	    b_tpp                    : 1,
	    b_payload_buffer         : 1,
	    b_ces_control_word       : 1,
	    b_packet_sync            : 1;
#else
	field_t  b_packet_sync  :  1, 
	    b_ces_control_word  :  1, 
	    b_payload_buffer  :  1, 
	    b_tpp  :  1, 
	    b_performance_monitoring  :  1, 
	    n_reserved  :  11; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegGlobalInterruptEnable;





typedef union
{
    struct AgNdRegGlobalInterruptStatus_S
    {
#ifdef BE_HOST
        field_t n_reserved               : 11,
	    b_performance_monitoring : 1,
	    b_tpp                    : 1,
	    b_payload_buffer         : 1,
	    b_ces_control_word       : 1,
	    b_packet_sync            : 1;
#else
	field_t  b_packet_sync  :  1, 
	    b_ces_control_word  :  1, 
	    b_payload_buffer  :  1, 
	    b_tpp  :  1, 
	    b_performance_monitoring  :  1, 
	    n_reserved  :  11; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegGlobalInterruptStatus;




typedef union
{
    struct AgNdRegEventQueueEntry_S
    {
#ifdef BE_HOST
        field_t b_valid_entry           : 1,
	    b_last_valid_entry      : 1,
	    n_reserved              : 3,
	    n_pending_event_channel : 11;
#else
	field_t  n_pending_event_channel  :  11, 
	    n_reserved  :  3, 
	    b_last_valid_entry  :  1, 
	    b_valid_entry  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegEventQueueEntry;  /* describes all interrupts event queue entry registers */




typedef union
{
    struct AgNdRegEventStatus_S
    {
#ifdef BE_HOST
        field_t n_reserved     : 5,
	    n_queue_status : 11;
#else
	field_t  n_queue_status  :  11, 
	    n_reserved  :  5; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegEventStatus; /* describes all interrupts event queue status registers */



/* */
/* Nemo TDM interface registers */
/* */

typedef union
{
    struct AgNdRegT1E1CClkMux1_S
    {
#ifdef BE_HOST
        field_t n_reference_clk1_port_select : 4,
	    n_reference_clk1_brg_select  : 4,
	    n_reference_clk2_port_select : 4,
	    n_reference_clk2_brg_select  : 4;
#else
	field_t  n_reference_clk2_brg_select  :  4, 
	    n_reference_clk2_port_select  :  4, 
	    n_reference_clk1_brg_select  :  4, 
	    n_reference_clk1_port_select  :  4; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegT1E1CClkMux1;





typedef union
{
    struct AgNdRegT1E1CClkMux2_S
    {
#ifdef BE_HOST
        field_t n_reference_clk_t1_e1_select    : 1,
	    b_cclk_out_enable               : 1,
	    n_cclk_source_select            : 2,
	    n_reference_clk1_select         : 3,
	    n_reference_clk2_select         : 3,
	    n_external_clk1_dir             : 1,
	    n_external_clk2_dir             : 1,
	    n_ptp_clk_out1_select           : 2,
	    n_ptp_clk_out2_select           : 2;
#else
	field_t  n_ptp_clk_out2_select  :  2, 
	    n_ptp_clk_out1_select  :  2, 
	    n_external_clk2_dir  :  1, 
	    n_external_clk1_dir  :  1, 
	    n_reference_clk2_select  :  3, 
	    n_reference_clk1_select  :  3, 
	    n_cclk_source_select  :  2, 
	    b_cclk_out_enable  :  1, 
	    n_reference_clk_t1_e1_select  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegT1E1CClkMux2;





typedef union
{
    struct
    {
#ifdef BE_HOST
        field_t n_missing_structured    : 2,
	    n_missing_unstructured  : 2,
	    n_l_bit_structured      : 2,
	    n_l_bit_unstructured    : 2,
	    n_lops_structured       : 2,
	    n_lops_unstructured     : 2,
	    n_rai_structured        : 2,
	    n_rai_unstructured      : 2;
#else
	field_t  n_rai_unstructured  :  2, 
	    n_rai_structured  :  2, 
	    n_lops_unstructured  :  2, 
	    n_lops_structured  :  2, 
	    n_l_bit_unstructured  :  2, 
	    n_l_bit_structured  :  2, 
	    n_missing_unstructured  :  2, 
	    n_missing_structured  :  2; 
#endif

    } x_fields;

    AG_U16      n_reg;

} AgNdRegPayloadReplacementPolicy;




typedef union
{
    struct AgNdRegPayloadReplacementBytePattern_S
    {
#ifdef BE_HOST
        field_t n_filler_byte_pattern       : 8,
	    n_idle_byte_pattern         : 8;
#else
	field_t  n_idle_byte_pattern  :  8, 
	    n_filler_byte_pattern  :  8; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegPayloadReplacementBytePattern;





typedef union
{
    struct AgNdRegT1E1PortConfiguration_S
    {
#ifdef BE_HOST
        field_t b_port_enable                          : 1,
	    n_reserved_1                           : 5,
	    b_prime_rtp_timestamp_mode_enable      : 1,
	    b_t1_d4_framing_mode_enable            : 1,
	    b_cas_signaling_enable                 : 1,
	    b_loopback_enable                      : 1,
	    n_t1_e1_port_selection                 : 1,
	    b_structured_service                   : 1,
	    b_octet_aligned_mode_in_satop_t1       : 1,
	    n_receive_clk_and_sync_source_select   : 1,
	    n_transmit_clk_source_select           : 2;
#else
	field_t  n_transmit_clk_source_select  :  2, 
	    n_receive_clk_and_sync_source_select  :  1, 
	    b_octet_aligned_mode_in_satop_t1  :  1, 
	    b_structured_service  :  1, 
	    n_t1_e1_port_selection  :  1, 
	    b_loopback_enable  :  1, 
	    b_cas_signaling_enable  :  1, 
	    b_t1_d4_framing_mode_enable  :  1, 
	    b_prime_rtp_timestamp_mode_enable  :  1, 
	    n_reserved_1  :  5, 
	    b_port_enable  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegT1E1PortConfiguration;



/* */
/* Neptune TDM interface registers */
/* */

typedef union
{
    struct AgNdRegTdmBusHierarchyConfiguration_S
    {
#ifdef BE_HOST
        field_t n_tdm_bus_type                          : 1,
	    n_vc4_circuit_selection                 : 1,
	    n_spe_3_mode                            : 2,
	    n_spe_2_mode                            : 2,
	    n_spe_1_mode                            : 2,
	    n_reserved_1                            : 8;
#else
	field_t  n_reserved_1  :  8, 
	    n_spe_1_mode  :  2, 
	    n_spe_2_mode  :  2, 
	    n_spe_3_mode  :  2, 
	    n_vc4_circuit_selection  :  1, 
	    n_tdm_bus_type  :  1; 
#endif

    } x_fields;

    AG_U16      n_reg;

} AgNdRegTdmBusHierarchyConfiguration;


typedef union
{
    struct AgNdRegTributaryCircuitConfiguration_S
    {
#ifdef BE_HOST
        field_t b_circuit_enable                            : 1,
	    b_cas_signaling_enable                      : 1,
	    b_clock_master                              : 1,
	    b_tsx_link_rate_disable                     : 1,
	    n_reserved_1                                : 2,
	    b_prime_rtp_timestamp_mode_enable           : 1,
	    n_tu_vt_cep_emulation                       : 1,
	    b_include_vt_pointer_bytes_in_cep_emulation : 1,
	    n_reserved_2                                : 1,
	    b_t1_d4_framing_mode_enable                 : 1,
	    b_structured_service                        : 1,
	    b_octet_aligned_mode_in_satop_t1            : 1,
	    n_reserved_3                                : 3;
#else
	field_t  n_reserved_3  :  3, 
	    b_octet_aligned_mode_in_satop_t1  :  1, 
	    b_structured_service  :  1, 
	    b_t1_d4_framing_mode_enable  :  1, 
	    n_reserved_2  :  1, 
	    b_include_vt_pointer_bytes_in_cep_emulation  :  1, 
	    n_tu_vt_cep_emulation  :  1, 
	    b_prime_rtp_timestamp_mode_enable  :  1, 
	    n_reserved_1  :  2, 
	    b_tsx_link_rate_disable  :  1, 
	    b_clock_master  :  1, 
	    b_cas_signaling_enable  :  1, 
	    b_circuit_enable  :  1; 
#endif

    } x_fields;

    AG_U16      n_reg;

} AgNdRegTributaryCircuitConfiguration;


typedef union 
{
    struct AgNdRegJustificationRequestControl_S
    {
#ifdef BE_HOST
        field_t b_explicit_justification_request        : 1,
	    n_explicit_justification_polarity       : 1,
	    n_explicit_justification_type           : 1,
	    b_bsg_enable                            : 1,
	    n_bsg_request_polarity                  : 1,
	    n_bsg_request_type                      : 1,
	    b_bsg_count_enable						: 1,
	    n_reserved_1                            : 9;
#else
	field_t  n_reserved_1  :  9, 
	    b_bsg_count_enable						:  1, 
	    n_bsg_request_type  :  1, 
	    n_bsg_request_polarity  :  1, 
	    b_bsg_enable  :  1, 
	    n_explicit_justification_type  :  1, 
	    n_explicit_justification_polarity  :  1, 
	    b_explicit_justification_request  :  1; 
#endif

    } x_fields;

    AG_U16      n_reg;

} AgNdRegJustificationRequestControl;


/* */
/* CAS registers */
/*  */
typedef union
{
    struct AgNdRegCasIdlePattern_S
    {
#ifdef BE_HOST
        field_t n_reserved_1               : 8,
	    n_e1_idle_pattern          : 4,
	    n_t1_idle_pattern          : 4; 
#else
	field_t  n_t1_idle_pattern  :  4, 
	    n_e1_idle_pattern  :  4, 
	    n_reserved_1  :  8; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegCasIdlePattern; 


typedef union
{
    struct AgNdRegPbfCasConfiguration_S
    {
#ifdef BE_HOST
        field_t b_signaling_enable         : 1,
	    n_reserved_1               : 4,
	    n_packet_payload_size      : 11;
#else
	field_t  n_packet_payload_size  :  11, 
	    n_reserved_1  :  4, 
	    b_signaling_enable  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegPbfCasConfiguration; 


typedef union
{
    struct AgNdRegPbfCasBufferStatus_S
    {
#ifdef BE_HOST
        field_t n_cas_state                : 2,
	    n_reserved_1               : 14;
#else
	field_t  n_reserved_1  :  14, 
	    n_cas_state  :  2; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegPbfCasBufferStatus; 


typedef union
{
    struct AgNdRegTransmitCasHeaderConfiguration_S
    {
#ifdef BE_HOST
        field_t n_reserved_1               : 9,
	    n_header_memory_size       : 7;
#else
	field_t  n_header_memory_size  :  7, 
	    n_reserved_1  :  9; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegTransmitCasHeaderConfiguration; 


typedef union
{
    struct AgNdRegTransmitCasHeaderAccessControl_S
    {
#ifdef BE_HOST
        field_t n_reserved_1                    : 2,
	    n_header_memory_block_size      : 2,
	    n_header_memory_base_address    : 12;
#else
	field_t  n_header_memory_base_address  :  12, 
	    n_header_memory_block_size  :  2, 
	    n_reserved_1  :  2; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegTransmitCasHeaderAccessControl; 


typedef union
{
    struct AgNdRegTransmitCasPacketDelay_S
    {
#ifdef BE_HOST
        field_t n_no_change_delay               : 3,
	    n_change_delay                  : 3,
	    n_reserved_1                    : 10;
#else
	field_t  n_reserved_1  :  10, 
	    n_change_delay  :  3, 
	    n_no_change_delay  :  3; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegTransmitCasPacketDelay; 


typedef union
{
    struct AgNdRegJbfCasConfiguration_S
    {
#ifdef BE_HOST
        field_t n_reserved_1               : 5,
	    n_packet_payload_size      : 11;
#else
	field_t  n_packet_payload_size  :  11, 
	    n_reserved_1  :  5; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegJbfCasConfiguration; 


typedef union
{
    struct AgNdRegJbfCasSequenceNumberWindow_S
    {
#ifdef BE_HOST
        field_t n_reserved_1               : 8,
	    n_cas_window_limit         : 8; 
#else
	field_t  n_cas_window_limit  :  8, 
	    n_reserved_1  :  8; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegJbfSequenceNumberWindow; 


/* */
/* CHI/CHE channelizer registers */
/* */
typedef union
{
    struct AgNdRegCircuitToChannelMap_S
    {
#ifdef BE_HOST
        field_t b_timeslot_enable          : 1,
	    b_first_timeslot_in_packet : 1,
	    n_reserved                 : 3,
	    n_target_channel_id        : 11;
#else
	field_t  n_target_channel_id  :  11, 
	    n_reserved  :  3, 
	    b_first_timeslot_in_packet  :  1, 
	    b_timeslot_enable  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegCircuitToChannelMap; 


typedef union
{
    struct AgNdRegChannelControl_S
    {
#ifdef BE_HOST
        field_t b_channel_enable           : 1,
	    n_reserved                 : 15;
#else
	field_t  n_reserved  :  15, 
	    b_channel_enable  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegChannelControl;


typedef union
{
    struct AgNdRegTimeslotAllocationControl_S
    {
#ifdef BE_HOST
        field_t b_dbm_enable                    : 1,
	    n_dbmux                         : 2,
	    n_reserved                      : 2,
	    n_dbmux_status                  : 2,
	    : 8;
#else
    field_t  :  8, 
	    n_dbmux_status  :  2, 
	    n_reserved  :  2, 
	    n_dbmux  :  2, 
	    b_dbm_enable  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegTimeslotAllocationControl;




/* */
/* PBF   Transmit payload buffer registers */
/* */

typedef union
{
    struct AgNdRegPayloadBufferChannelConfiguration_S
    {
#ifdef BE_HOST
        field_t b_interrupt_enable          : 1,
	    b_packet_redundancy_enable  : 1,
	    n_payload_buffer_queue_size : 3,
	    n_packet_payload_size       : 11;
#else
	field_t  n_packet_payload_size  :  11, 
	    n_payload_buffer_queue_size  :  3, 
	    b_packet_redundancy_enable  :  1, 
	    b_interrupt_enable  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegPayloadBufferChannelConfiguration;







typedef union
{
    struct AgNdRegPayloadBufferStatus_S
    {
#ifdef BE_HOST
        field_t b_buffer_overflow            : 1,
	    n_reserved                   : 2,
	    n_buffer_maximum_utilization : 13;
#else
	field_t  n_buffer_maximum_utilization  :  13, 
	    n_reserved  :  2, 
	    b_buffer_overflow  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegPayloadBufferStatus;







typedef union
{
    struct AgNdRegTransmitHeaderConfiguration_S
    {
#ifdef BE_HOST
        field_t n_reserved                : 6,
	    b_auto_r_bit_insertion    : 1,
	    b_dba_packet              : 1,
	    b_header_select_alternate : 1,
	    n_header_memory_size      : 7;
#else
	field_t  n_header_memory_size  :  7, 
	    b_header_select_alternate  :  1, 
	    b_dba_packet  :  1, 
	    b_auto_r_bit_insertion  :  1, 
	    n_reserved  :  6; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegTransmitHeaderConfiguration;







typedef union
{
    struct AgNdRegTransmitHeaderAccessControl_S
    {
#ifdef BE_HOST
        field_t n_reserved                   : 2,        
	    n_header_memory_block_size   : 2,
	    n_header_memory_base_address : 12;
#else
	field_t  n_header_memory_base_address  :  12, 
	    n_header_memory_block_size  :  2, 
	    n_reserved  :  2; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegTransmitHeaderAccessControl;





/* */
/* JBF   Jitter buffer registers */
/* */

typedef union
{
    struct AgNdRegJitterBufferChannelConfiguration1_S
    {
#ifdef BE_HOST
        field_t n_jitter_buffer_ring_size       : 3,
	    n_window_size_limit             : 3,
	    n_jitter_buffer_break_out_point : 10;
#else
	field_t  n_jitter_buffer_break_out_point  :  10, 
	    n_window_size_limit  :  3, 
	    n_jitter_buffer_ring_size  :  3; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegJitterBufferChannelConfiguration1;






typedef union
{
    struct AgNdRegJitterBufferChannelConfiguration2
    {
#ifdef BE_HOST
        field_t b_packet_sync_interrupt_enable   : 1,
	    n_packet_sync_thresholds_pointer : 3,
	    b_drop_on_valid                  : 1,
	    n_packet_payload_size            : 11;
#else
	field_t  n_packet_payload_size  :  11, 
	    b_drop_on_valid  :  1, 
	    n_packet_sync_thresholds_pointer  :  3, 
	    b_packet_sync_interrupt_enable  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegJitterBufferChannelConfiguration2;






typedef union
{
    struct AgNdRegJitterBufferQueueBaseAddress_S
    {
#ifdef BE_HOST
	field_t	b_drop_on_valid : 1,
	    n_reserved   : 3,
	    n_base_address : 12;
#else
	field_t  n_base_address  :  12, 
	    n_reserved  :  3, 
	    b_drop_on_valid :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegJitterBufferQueueBaseAddress;






typedef union
{
    struct AgNdRegJitterBufferValidBitArrayConfiguration_S
    {
#ifdef BE_HOST
        field_t b_drop_on_valid                : 1,
	    n_reserved                     : 2,
	    n_valid_bit_array_base_address : 13;
#else
	field_t  n_valid_bit_array_base_address  :  13, 
	    n_reserved  :  2, 
	    b_drop_on_valid  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegJitterBufferValidBitArrayConfiguration;






typedef union
{
    struct AgNdRegJitterBufferPerformanceCounters_S
    {
#ifdef BE_HOST
        field_t n_buffer_underrun_counter : 8,
	    n_missing_packet_counter  : 8;
#else
	field_t  n_missing_packet_counter  :  8, 
	    n_buffer_underrun_counter  :  8; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegJitterBufferPerformanceCounters;






typedef union
{
    struct AgNdRegBadLengthPacketCounter_S
    {
#ifdef BE_HOST
        field_t n_reserved                   : 8,
	    n_bad_length_packet_counter  : 8;
#else
	field_t  n_bad_length_packet_counter  :  8, 
	    n_reserved  :  8; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegBadLengthPacketCounter;






typedef union
{
    struct AgNdRegOutOfOrderPacketCounters_S
    {
#ifdef BE_HOST
        field_t n_dropped_out_of_order_packet_count   : 8,
	    n_reordered_out_of_order_packet_count : 8;
#else
	field_t  n_reordered_out_of_order_packet_count  :  8, 
	    n_dropped_out_of_order_packet_count  :  8; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegOutOfOrderPacketCounters;






typedef union
{
    struct AgNdRegJitterBufferChannelStatus_S
    {
#ifdef BE_HOST
        field_t n_reserved_1                    : 2,
	    n_jitter_buffer_state           : 2,
	    n_reserved_2                    : 1,
	    b_trimming_mode                 : 1,
	    n_reserved_3                    : 10;
#else
	field_t  n_reserved_3  :  10, 
	    b_trimming_mode  :  1, 
	    n_reserved_2  :  1, 
	    n_jitter_buffer_state  :  2, 
	    n_reserved_1  :  2; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegJitterBufferChannelStatus;






typedef union
{
    struct AgNdRegJitterBufferSlipCommand_S
    {
#ifdef BE_HOST
        field_t n_byte_slip_command_status   : 1,
	    n_packet_slip_command_status : 1,
	    n_packet_slip_direction      : 1,
	    b_global_trimming_enable     : 1,
	    n_reserved                   : 1,
	    n_byte_slip_size             : 11;
#else
	field_t  n_byte_slip_size  :  11, 
	    n_reserved  :  1, 
	    b_global_trimming_enable  :  1, 
	    n_packet_slip_direction  :  1, 
	    n_packet_slip_command_status  :  1, 
	    n_byte_slip_command_status  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegJitterBufferSlipCommand;






typedef union
{
    struct AgNdRegJitterBufferSlipConfiguration_S
    {
#ifdef BE_HOST
        field_t n_packet_slip_size          : 5,
	    n_slip_channel_select       : 11;
#else
	field_t  n_slip_channel_select  :  11, 
	    n_packet_slip_size  :  5; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegJitterBufferSlipConfiguration;







/*  */
/* describes entry in the aquisition/loss packet sync treshold tables */
/* */
typedef union
{
    struct AgNdRegSyncThreshold_S
    {
#ifdef BE_HOST
        field_t n_reserved              : 3,
	    n_packet_sync_threshold : 13;
#else
	field_t  n_packet_sync_threshold  :  13, 
	    n_reserved  :  3; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegSyncThreshold;



typedef union
{
    struct AgNdRegPacketSyncStatus_S
    {
#ifdef BE_HOST
        field_t n_packet_sync_interrupt_status  : 1,
	    n_packet_sync_state             : 1,
	    n_reserved_1                    : 14;
#else
	field_t  n_reserved_1  :  14, 
	    n_packet_sync_state  :  1, 
	    n_packet_sync_interrupt_status  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegPacketSyncStatus;



/* */
/* TPE   Receive packet classifier registers */
/* */

typedef union
{
    struct AgNdRegTransmitHeaderFormatDescriptor_S
    {
#ifdef BE_HOST
        field_t b_ipv4_exists                   : 1,
	    n_ipv4_header_pointer           : 7,
	    b_udp_exists                    : 1,
	    n_udp_header_pointer            : 7,
	    b_rtp_or_ptp_exists             : 1,
	    n_rtp_or_ptp_header_pointer     : 7,
	    b_cep_or_ptp_sec_exists         : 1,
	    n_ces_header_pointer            : 7;
#else
	field_t  n_ces_header_pointer  :  7, 
	    b_cep_or_ptp_sec_exists  :  1, 
	    n_rtp_or_ptp_header_pointer  :  7, 
	    b_rtp_or_ptp_exists  :  1, 
	    n_udp_header_pointer  :  7, 
	    b_udp_exists  :  1, 
	    n_ipv4_header_pointer  :  7, 
	    b_ipv4_exists  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg[2];

} AgNdRegTransmitHeaderFormatDescriptor;


/* */
/* RPC   Receive packet classifier registers */
/* */


/*ORI*/
/*update reg to support l2tpv3 */

typedef union
{
    struct AgNdRegReceiveChannelConfiguration_S
    {
#ifdef BE_HOST
        field_t b_channel_enable                : 1,
	    b_control_word_interrupt_enable : 1, 
	    n_reserved_1                    : 10,
	    n_l2tpv3_cookie_num             : 2,
	    b_rtp_follows_ces               : 1,
	    n_reserved_2                    : 1;
#else
	field_t  n_reserved_2  :  1, 
	    b_rtp_follows_ces  :  1, 
	    n_l2tpv3_cookie_num  :  2, 
	    n_reserved_1  :  10, 
	    b_control_word_interrupt_enable  :  1, 
	    b_channel_enable  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegReceiveChannelConfiguration;


typedef union
{
    struct AgNdRegGlobalCounterPolicySelection_S
    {
#ifdef BE_HOST
        field_t n_global_8_policy_selection     : 2,
	    n_global_7_policy_selection     : 2,
	    n_global_6_policy_selection     : 2,
	    n_global_5_policy_selection     : 2,
	    n_global_4_policy_selection     : 2,
	    n_global_3_policy_selection     : 2,
	    n_global_2_policy_selection     : 2,
	    n_global_1_policy_selection     : 2;
#else
	field_t  n_global_1_policy_selection  :  2, 
	    n_global_2_policy_selection  :  2, 
	    n_global_3_policy_selection  :  2, 
	    n_global_4_policy_selection  :  2, 
	    n_global_5_policy_selection  :  2, 
	    n_global_6_policy_selection  :  2, 
	    n_global_7_policy_selection  :  2, 
	    n_global_8_policy_selection  :  2; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegGlobalCounterPolicySelection;



typedef union
{
    struct AgNdRegPrioterizedCounterSelectionMask_S
    {
#ifdef BE_HOST
        field_t n_reserved_1                    : 4,
	    b_per_channel_4_priority_enable : 1,
	    b_per_channel_3_priority_enable : 1,
	    b_per_channel_2_priority_enable : 1,
	    b_per_channel_1_priority_enable : 1,
	    b_global_8_priority_enable      : 1,
	    b_global_7_priority_enable      : 1,
	    b_global_6_priority_enable      : 1,
	    b_global_5_priority_enable      : 1,
	    b_global_4_priority_enable      : 1,
	    b_global_3_priority_enable      : 1,
	    b_global_2_priority_enable      : 1,
	    b_global_1_priority_enable      : 1;
#else
	field_t  b_global_1_priority_enable  :  1, 
	    b_global_2_priority_enable  :  1, 
	    b_global_3_priority_enable  :  1, 
	    b_global_4_priority_enable  :  1, 
	    b_global_5_priority_enable  :  1, 
	    b_global_6_priority_enable  :  1, 
	    b_global_7_priority_enable  :  1, 
	    b_global_8_priority_enable  :  1, 
	    b_per_channel_1_priority_enable  :  1, 
	    b_per_channel_2_priority_enable  :  1, 
	    b_per_channel_3_priority_enable  :  1, 
	    b_per_channel_4_priority_enable  :  1, 
	    n_reserved_1  :  4; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegPrioterizedCounterSelectionMask;


typedef union
{
    struct AgNdRegUcodeInstruction_S
    {
#ifdef BE_HOST
        field_t b_stop                          : 1,
	    n_comparison_source             : 2,
	    b_use_mask                      : 1,
	    n_freeze_condition_code         : 2,
	    b_branch_condition_code         : 2,
	    n_branch_address                : 8,

	    n_set_flasg_condition           : 2,
	    n_reserved_2                    : 2,
	    n_flag                          : 4,
	    n_opcode_exec_condition_code    : 2,
	    n_reserved_3                    : 1,
	    n_opcode                        : 5,

	    n_value                         : 16,

	    n_mask                          : 16;
#else
	field_t  n_mask  :  16, 

	    n_value  :  16, 

	    n_opcode  :  5, 
	    n_reserved_3  :  1, 
	    n_opcode_exec_condition_code  :  2, 
	    n_flag  :  4, 
	    n_reserved_2  :  2, 
	    n_set_flasg_condition  :  2, 

	    n_branch_address  :  8, 
	    b_branch_condition_code  :  2, 
	    n_freeze_condition_code  :  2, 
	    b_use_mask  :  1, 
	    n_comparison_source  :  2, 
	    b_stop  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg[4];

} AgNdRegUcodeInstruction;


typedef union
{
    struct AgNdRegLstreeEntry_S
    {
#ifdef BE_HOST
        field_t n_opcode                        : 3,
	    n_path_tag                      : 8,
	    n_next_record_address           : 13,
	    n_next_path_tag                 : 8;
#else
	field_t  n_next_path_tag  :  8, 
	    n_next_record_address  :  13, 
	    n_path_tag  :  8, 
	    n_opcode  :  3; 
#endif

    } x_fields;

    AG_U16      n_reg[2];

} AgNdRegLstreeEntry;


typedef union
{
    struct AgNdRegRaiConfiguration_S
    {
#ifdef BE_HOST
        field_t n_reserved            : 14,
	    b_m_bit_detection     : 1,
	    b_r_bit_detection     : 1;
#else
	field_t  b_r_bit_detection  :  1, 
	    b_m_bit_detection  :  1, 
	    n_reserved  :  14; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegRaiConfiguration;


/* */
/* TPP   Timing pre-processor registers */
/* */

typedef union
{
    struct AgNdRegClkSourceSelection_S
    {
#ifdef BE_HOST
        field_t n_reserved            : 8,
	    b_clk_source_enable   : 1,
	    n_target_rcr_channel  : 7;
#else
	field_t  n_target_rcr_channel  :  7, 
	    b_clk_source_enable  :  1, 
	    n_reserved  :  8; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegClkSourceSelection;




typedef union
{
    struct AgNdRegRcrChannelConfiguration_S
    {
#ifdef BE_HOST
        field_t n_reserved_1                    : 5,
	    b_timestamp_sampling_period     : 1,
	    b_pll_peak_detector_enable      : 1,
	    b_fll_peak_detector_enable      : 1,
	    b_phase_slope_limit_enable      : 1,
	    n_reserved_2                    : 3,
	    n_sampling_period_size          : 4;
#else
	field_t  n_sampling_period_size  :  4, 
	    n_reserved_2  :  3, 
	    b_phase_slope_limit_enable  :  1, 
	    b_fll_peak_detector_enable  :  1, 
	    b_pll_peak_detector_enable  :  1, 
	    b_timestamp_sampling_period  :  1, 
	    n_reserved_1  :  5; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegRcrChannelConfiguration;

typedef union
{
    struct AgNdRegTppTimeStampSelect_S
    {
#ifdef BE_HOST
        field_t n_tsr_125             : 2,
	    n_tsr_77              : 2,
	    n_reserved            : 12;
#else
	field_t  n_reserved  :  12, 
	    n_tsr_77  :  2, 
	    n_tsr_125  :  2; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegTppTimeStampSelect;


/* */
/* PIF registers */
/* */

typedef union
{
    struct AgNdRegMACCmdConfiguration1_S
    {
#ifdef BE_HOST
        field_t n_reserved_1               : 5,
	    b_rx_err_disc              : 1,
	    b_ena_10                   : 1,
	    b_no_lgth_check            : 1,
	    b_cntl_frm_ena             : 1,
	    n_reserved_2               : 1,
	    b_wakeup                   : 1,
	    b_sleep                    : 1,
	    b_magic_ena                : 1,
	    n_tx_addr_sel              : 3;
#else
	field_t  n_tx_addr_sel  :  3, 
	    b_magic_ena  :  1, 
	    b_sleep  :  1, 
	    b_wakeup  :  1, 
	    n_reserved_2  :  1, 
	    b_cntl_frm_ena  :  1, 
	    b_no_lgth_check  :  1, 
	    b_ena_10  :  1, 
	    b_rx_err_disc  :  1, 
	    n_reserved_1  :  5; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegMACCmdConfiguration1;





typedef union
{
    struct AgNdRegMACCmdConfiguration2_S
    {
#ifdef BE_HOST
        field_t b_loop_ena                 : 1,
	    b_mhash_sel                : 1,
	    b_sw_reset                 : 1,
	    b_late_col                 : 1,
	    b_excess_col               : 1,
	    b_hd_ena                   : 1,
	    b_tx_addr_ins              : 1,
	    b_pause_ignore             : 1,
	    b_pause_fwd                : 1,
	    b_crc_fwd                  : 1,
	    b_pad_en                   : 1,
	    b_promis_en                : 1,
	    b_eth_speed                : 1,
	    n_reserved_1               : 1,
	    b_rx_ena                   : 1,
	    b_tx_ena                   : 1;
#else
	field_t  b_tx_ena  :  1, 
	    b_rx_ena  :  1, 
	    n_reserved_1  :  1, 
	    b_eth_speed  :  1, 
	    b_promis_en  :  1, 
	    b_pad_en  :  1, 
	    b_crc_fwd  :  1, 
	    b_pause_fwd  :  1, 
	    b_pause_ignore  :  1, 
	    b_tx_addr_ins  :  1, 
	    b_hd_ena  :  1, 
	    b_excess_col  :  1, 
	    b_late_col  :  1, 
	    b_sw_reset  :  1, 
	    b_mhash_sel  :  1, 
	    b_loop_ena  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegMACCmdConfiguration2;


/* */
/* PTP registers */
/*  */
typedef union
{
    struct AgNdRegPtpTimestampGeneratorsConfiguration_S
    {
#ifdef BE_HOST
        field_t b_timestamp_generator_enable    : 1,
	    b_fast_phase_enable             : 1,
	    b_error_correction_enable       : 1,
	    n_reserved_1                    : 13;
#else
	field_t  n_reserved_1  :  13, 
	    b_error_correction_enable  :  1, 
	    b_fast_phase_enable  :  1, 
	    b_timestamp_generator_enable  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegPtpTimestampGeneratorsConfiguration;


typedef union
{
    struct AgNdRegPtpTimestampClkSourceSelection_S
    {
#ifdef BE_HOST
        field_t n_reserved_1                    : 8,
	    n_reference_clock_source_select : 3,
	    n_port_select                   : 5;
#else
	field_t  n_port_select  :  5, 
	    n_reference_clock_source_select  :  3, 
	    n_reserved_1  :  8; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegPtpTimestampClkSourceSelection;


typedef union
{
    struct AgNdRegPtpBaudRateGeneratorCorrection_S
    {
#ifdef BE_HOST
        field_t n_reserved_1                    : 14,
	    n_positive_adjustment           : 1,
	    n_negative_adjustment           : 1;
#else
	field_t  n_negative_adjustment  :  1, 
	    n_positive_adjustment  :  1, 
	    n_reserved_1  :  14; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegPtpBaudRateGeneratorCorrection;



typedef union
{
    struct AgNdRegPbfPtpConfiguration_S
    {
#ifdef BE_HOST
        field_t n_reserved_1                : 2,
	    n_header_1_memory_size      : 7,
	    n_header_0_memory_size      : 7;
#else
	field_t  n_header_0_memory_size  :  7, 
	    n_header_1_memory_size  :  7, 
	    n_reserved_1  :  2; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegPbfPtpConfiguration;


typedef union
{
    struct AgNdRegPtpTrasmitAccessControl_S
    {
#ifdef BE_HOST
        field_t n_reserved_1                 : 2,
	    n_header_memory_block_size   : 2,
	    n_header_memory_base_address : 12;
#else
	field_t  n_header_memory_base_address  :  12, 
	    n_header_memory_block_size  :  2, 
	    n_reserved_1  :  2; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegPtpTrasmitAccessControl;


typedef union
{
    struct AgNdRegPtpConfiguration_S
    {
#ifdef BE_HOST
        field_t b_auto_tx_sync_enable               : 1,
	    b_auto_tx_delay_request_enable      : 1,
	    b_auto_tx_delay_response_enable     : 1,
	    b_auto_tx_pdelay_request_enable     : 1,
	    b_auto_tx_pdelay_response_enable    : 1,
	    n_clk_select						: 1,
	    b_random_tx_enable					: 1,
	    n_reserved_2						: 1,
	    b_count_in_delay_req				: 1,
	    n_reserved_1						: 1,
	    n_rx_response_rate					: 3,
	    n_tx_sync_rate						: 3;
#else
	field_t  				n_tx_sync_rate						:  3, 
	    n_rx_response_rate					:  3, 
	    n_reserved_1						:  1, 
	    b_count_in_delay_req				:  1, 
	    n_reserved_2						:  1, 
	    b_random_tx_enable					:  1, 
	    n_clk_select						:  1, 
	    b_auto_tx_pdelay_response_enable  :  1, 
	    b_auto_tx_pdelay_request_enable  :  1, 
	    b_auto_tx_delay_response_enable  :  1, 
	    b_auto_tx_delay_request_enable  :  1, 
	    b_auto_tx_sync_enable  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegPtpConfiguration;

typedef union
{
    struct AgNdRegPtpOutPacketConfiguration_S
    {
#ifdef BE_HOST
        field_t n_reserved_1		                : 15,
	    b_count_out_delay_req				: 1;
#else
	field_t  				b_count_out_delay_req				:  1, 
	    n_reserved_1		  :  15; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegPtpOutPacketConfiguration;



typedef union
{
    struct AgNdRegPtpFilter_S
    {
#ifdef BE_HOST
        field_t n_reserved_1                        : 8,
	    b_rx_sync_enable                    : 1,
	    b_rx_delay_request_enable           : 1,
	    b_rx_delay_response_enable          : 1,
	    b_rx_pdelay_request_enable          : 1,
	    b_rx_pdelay_response_enable         : 1,
	    n_clk_select                        : 1,
	    n_reserved_2                        : 2;
#else
	field_t  n_reserved_2  :  2, 
	    n_clk_select  :  1, 
	    b_rx_pdelay_response_enable  :  1, 
	    b_rx_pdelay_request_enable  :  1, 
	    b_rx_delay_response_enable  :  1, 
	    b_rx_delay_request_enable  :  1, 
	    b_rx_sync_enable  :  1, 
	    n_reserved_1  :  8; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegPtpFilter;


typedef union
{
    struct AgNdRegReceivePtpChannelConfiguration_S
    {
#ifdef BE_HOST
        field_t b_channel_enable                    : 1,
	    n_clk_select                        : 1,
	    n_reserved_1                        : 14;
#else
	field_t  n_reserved_1  :  14, 
	    n_clk_select  :  1, 
	    b_channel_enable  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegReceivePtpChannelConfiguration;


/* */
/* HPL registers */
/* */
typedef union
{
    struct AgNdRegHostPacketLinkConfiguration_S
    {
#ifdef BE_HOST
        field_t b_hpl_tx_enable            : 1,
	    b_hpl_rx_enable            : 1,
	    b_pme_export_enable        : 1,
	    n_reserved_1               : 1,
	    n_pme_channels_1           : 4,
	    n_pme_channels_2           : 4,
	    n_pme_channels_3           : 4;
#else
	field_t  n_pme_channels_3  :  4, 
	    n_pme_channels_2  :  4, 
	    n_pme_channels_1  :  4, 
	    n_reserved_1  :  1, 
	    b_pme_export_enable  :  1, 
	    b_hpl_rx_enable  :  1, 
	    b_hpl_tx_enable  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegHostPacketLinkConfiguration;

typedef union
{
    struct AgNdRegPmeStructureAssembly_S
    {
#ifdef BE_HOST
        field_t b_auto_trigger_enable      : 1,
	    b_activation               : 1,
	    n_reserved_1               : 6,
	    n_counter_select           : 8;
#else
	field_t  n_counter_select  :  8, 
	    n_reserved_1  :  6, 
	    b_activation  :  1, 
	    b_auto_trigger_enable  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegPmeStructureAssembly;


typedef union
{
    struct AgNdRegPmCounterExportChannnelSelection_S
    {
#ifdef BE_HOST
        field_t b_pme_group_1_select       : 1,
	    b_pme_group_2_select       : 1,
	    b_pme_group_3_select       : 1,
	    n_reserved_1               : 13;
#else
	field_t  n_reserved_1  :  13, 
	    b_pme_group_3_select  :  1, 
	    b_pme_group_2_select  :  1, 
	    b_pme_group_1_select  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegPmCounterExportChannnelSelection;


typedef union
{
    struct AgNdRegDiffRecoveredLinkRatePhaseIntegration_S
    {
#ifdef BE_HOST
        field_t n_reserved                  : 6,
	    n_output_phase_shift        : 10;
#else
	field_t  n_output_phase_shift  :  10, 
	    n_reserved  :  6; 
#endif
    } x_fields;

    AG_U16      n_reg;
} AgNdRegDiffRecoveredLinkRatePhaseIntegration;


typedef union
{
    struct AgNdRegDiffRTPTimestampGeneratorConfiguration_S
    {
#ifdef BE_HOST
        field_t b_timestamp_generation_enable       : 1,
	    b_bit_justification_toggle_enable	: 1,
	    n_reserved                          : 2,
	    n_prime_timestamp_sampling_period   : 12;
#else
	field_t  n_prime_timestamp_sampling_period  :  12, 
	    n_reserved  :  2, 
	    b_bit_justification_toggle_enable	:  1, 
	    b_timestamp_generation_enable  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;
} AgNdRegDiffRTPTimestampGeneratorConfiguration;


typedef union
{
    struct AgNdRegDiffRTPTimestampFrameStepSize_S
    {
#ifdef BE_HOST
        field_t n_reserved                          : 4,
	    n_frame_step_size                   : 12;
#else
	field_t  n_frame_step_size  :  12, 
	    n_reserved  :  4; 
#endif
    } x_fields;

    AG_U16      n_reg;
} AgNdRegDiffRTPTimestampFrameStepSize;


/* */
/* nemo dcr registers */
/* */

typedef union
{
    struct AgNdRegRtpTimestampGeneratorConfiguration_S
    {
#ifdef BE_HOST
        field_t b_timestamp_generation_enable          : 1,
	    b_fast_phase_enable                    : 1,
	    b_error_correction_enable              : 1,
	    n_reserved_1                           : 1,
	    n_prime_timestamp_step_size            : 12;
#else
	field_t  n_prime_timestamp_step_size  :  12, 
	    n_reserved_1  :  1, 
	    b_error_correction_enable  :  1, 
	    b_fast_phase_enable  :  1, 
	    b_timestamp_generation_enable  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegRtpTimestampGeneratorConfiguration;

typedef union
{
    struct AgNdRegDcrReferenceClockSourceSelection_S
    {
#ifdef BE_HOST
        field_t n_reserved_1						   : 8,
	    n_ref_clock                            : 3,
	    n_port_select						   : 5;
#else
	field_t  				n_port_select						  :  5, 
	    n_ref_clock  :  3, 
	    n_reserved_1						  :  8; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegDcrReferenceClockSourceSelection;

typedef union
{
    struct AgNdRegDcrReferenceSystemClockSourceSelection_S
    {
#ifdef BE_HOST
        field_t n_clock_source_select                  : 4,
	    n_prime_timestamp_step_size			   : 12;
#else
	field_t n_prime_timestamp_step_size  :  12, 
	    n_clock_source_select  :  4; 
#endif
    } x_fields;

    AG_U16      n_reg;

} AgNdRegDcrReferenceSystemClockSourceSelection;

typedef union
{
    struct AgNdRegDcrSystemTsoMode_S
    {
#ifdef BE_HOST
        field_t n_mode : 1,
	    n_reserved : 15;
#else
	field_t n_reserved :  15, 
	    n_mode  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;
}AgNdRegDcrSystemTsoMode;

typedef union
{
    struct AgNdRegDcrSystemTsoRefClockConfiguration_S
    {
#ifdef BE_HOST
        field_t n_reserved : 14,
	    n_ref_select   : 2;
#else
	field_t	n_ref_select :  2, 
	    n_reserved :  14; 
#endif
    } x_fields;

    AG_U16      n_reg;
}AgNdRegDcrSystemTsoRefClockConfiguration;

/*ORI*/
#ifdef CES16_BCM_VERSION
/*ORI*/
/*this msg to complete session gs(pm) */
typedef union
{
    struct AgNdRegJitterBufferMissingPacketCount_S
    {
#ifdef BE_HOST
        field_t n_missing_packet_count			        : 16;
#else
	field_t  n_missing_packet_count			  :  16;
#endif
	    } x_fields;

    AG_U16      n_reg;
}AgNdRegJitterBufferMissingPacketCount;

typedef union
{
    struct AgNdRegJitterBufferUnderrunPacketCount_S
    {
#ifdef BE_HOST
        field_t n_underrun_packet_count			        : 16;
#else
	field_t  n_underrun_packet_count			  :  16;
#endif
	    } x_fields;

    AG_U16      n_reg;
}AgNdRegJitterBufferUnderrunPacketCount;

typedef union
{
    struct AgNdRegJitterBufferRestartCount_S
    {
#ifdef BE_HOST
    	field_t n_reserved			                   : 8,
	    n_restart_count			           : 8;
#else
	field_t  		n_restart_count			  :  8, 
	    n_reserved			  :  8; 
#endif
    } x_fields;

    AG_U16      n_reg;
}AgNdRegJitterBufferRestartCount;




typedef union
{
    struct AgRegFramerPortConfiguration_S
    {
#ifdef BE_HOST
        field_t n_reserved			                   : 1,
	    b_b7_stuffing_enable				   : 1,
	    b_auto_prm_enable				       : 1,
	    n_sing_mode_select					   : 1,
	    n_manul_reframe						   : 1,	
	    n_master_slave_select				   : 1,
	    n_codec_mode_select                    : 1,
	    n_zero_count                           : 1,
	    n_e1_line_code_select                  : 1,
	    n_t1_line_code_select                  : 1,
	    b_rec_crc_enable                       : 1,
	    b_tran_crc_enable                      : 1,
	    n_japanse_mode_select                  : 1,
	    n_t1_sing_format_select                : 2,
	    n_t1_j1_fram_mode					   : 1;
#else
	field_t  				n_t1_j1_fram_mode					  :  1, 
	    n_t1_sing_format_select  :  2, 
	    n_japanse_mode_select  :  1, 
	    b_tran_crc_enable  :  1, 
	    b_rec_crc_enable  :  1, 
	    n_t1_line_code_select  :  1, 
	    n_e1_line_code_select  :  1, 
	    n_zero_count  :  1, 
	    n_codec_mode_select  :  1, 
	    n_master_slave_select				  :  1, 
	    n_manul_reframe						  :  1,	 
	    n_sing_mode_select					  :  1, 
	    b_auto_prm_enable				  :  1, 
	    b_b7_stuffing_enable				  :  1, 
	    n_reserved			  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;
}AgRegFramerPortConfiguration;

typedef union
{
    struct AgRegFramerCommonConfigurationLower_S
    {
#ifdef BE_HOST
        field_t b_prbs_mode			                   : 1,
	    b_prbs_inter						   : 1,
	    n_prbs_test_mode_select				   : 2,
	    n_prbs_port_select					   : 3,
	    b_inter_trig_config					   : 1,
	    b_port8_enable                         : 1,
	    b_port7_enable                         : 1,
	    b_port6_enable                         : 1,
	    b_port5_enable                         : 1,
	    b_port4_enable                         : 1,
	    b_port3_enable                         : 1,
	    b_port2_enable                         : 1,
	    b_port1_enable                         : 1;
#else
	field_t b_port1_enable  :  1, 
	    b_port2_enable  :  1, 
	    b_port3_enable  :  1, 
	    b_port4_enable  :  1, 
	    b_port5_enable  :  1, 
	    b_port6_enable  :  1, 
	    b_port7_enable  :  1, 
	    b_port8_enable  :  1, 
	    b_inter_trig_config					  :  1, 
	    n_prbs_port_select					  :  3, 
	    n_prbs_test_mode_select				  :  2, 
	    b_prbs_inter						  :  1, 
	    b_prbs_mode			  :  1; 
#endif
    } x_fields;

    AG_U16      n_reg;
}AgRegFramerCommonConfigurationLower;
typedef union
{
    struct AgRegFramerPortStatusRegister1_S
    {
#ifdef BE_HOST
	field_t n_loopback_deactivation_code 		   : 1,
	    n_loopback_activation_code             : 1,
	    n_sa6_change_unlatched_status 		   : 1,
	    n_prm_status						   : 1,
	    n_excessive_zeroes_status 			   : 1,
	    n_cas_ts_16_ais_status				   : 1,
	    n_cas_ts_16_los_status 				   : 1,
	    n_recive_bits_status				   : 1,
	    n_signal_multiframe_error_status       : 1,
	    n_crc_error_status					   : 1,
	    n_frame_alarm_status				   : 1,
	    n_remote_multiframe_error_status       : 1,
	    n_remote_alarm_indec_status			   : 1,
	    n_oof_status						   : 1,
	    n_ais_status						   : 1,
	    n_los_status						   : 1;
#else
	field_t  					n_los_status						  :  1, 
	    n_ais_status						  :  1, 
	    n_oof_status						  :  1, 
	    n_remote_alarm_indec_status			  :  1, 
	    n_remote_multiframe_error_status  :  1, 
	    n_frame_alarm_status				  :  1, 
	    n_crc_error_status					  :  1, 
	    n_signal_multiframe_error_status  :  1, 
	    n_recive_bits_status				  :  1, 
	    n_cas_ts_16_los_status  				  :  1, 
	    n_cas_ts_16_ais_status				  :  1, 
	    n_excessive_zeroes_status  			  :  1, 
	    n_prm_status						  :  1, 
	    n_sa6_change_unlatched_status  		  :  1, 
	    n_loopback_activation_code  :  1, 
	    n_loopback_deactivation_code  		  :  1; 
#endif
    } x_fields;
	
    AG_U16		n_reg;
}AgRegFramerPortStatusRegister1;

typedef union
{
    struct AgRegFramerPortAlarmControlRegister_S
    {
#ifdef BE_HOST
	field_t n_t1_oof_config 						: 2,
	    n_reserved						   		: 3,
	    n_force_fram_alig_word_error 			: 1,
	    n_force_bip_code				   		: 1,
	    n_force_crc_error 				   		: 1,
	    n_rai_insert_signal_bus				   	: 1,
	    n_signal_bus_ais_los       				: 1,
	    n_signal_bus_ais_oof					: 1,
	    n_signal_bus_ais_ais				   	: 1,
	    n_forward_rai       					: 1,
	    n_forward_ais			   				: 1,
	    n_force_rai						   		: 1,
	    n_force_ais						   		: 1;
#else
	field_t  					n_force_ais						  		:  1, 
	    n_force_rai						  		:  1, 
	    n_forward_ais			  				:  1, 
	    n_forward_rai  					:  1, 
	    n_signal_bus_ais_ais				  	:  1, 
	    n_signal_bus_ais_oof					:  1, 
	    n_signal_bus_ais_los  				:  1, 
	    n_rai_insert_signal_bus				  	:  1, 
	    n_force_crc_error  				  		:  1, 
	    n_force_bip_code				  		:  1, 
	    n_force_fram_alig_word_error  			:  1, 
	    n_reserved						  		:  3, 
	    n_t1_oof_config  						:  2; 
#endif
    }x_fields;
    AG_U16		n_reg;	
}AgRegFramerPortAlarmControlRegister;

typedef union
{
    struct AgRegFramerPortCRCErrorCounterShadow_S
    {
#ifdef BE_HOST
        field_t n_reserved			                   : 6,
	    n_crc_error_counter                    : 10;
#else
	field_t  				n_crc_error_counter  :  10, 
	    n_reserved			  :  6; 
#endif
    } x_fields;

    AG_U16      n_reg;
}AgRegFramerPortCRCErrorCounterShadow;

typedef union
{
    struct AgRegFramerPortLineCodeViolationErrorCounterShadow_S
    {
#ifdef BE_HOST
        field_t n_line_code_voi_error                  : 16;
#else
	field_t  n_line_code_voi_error  :  16; 
#endif
	    } x_fields;

    AG_U16      n_reg;
}AgRegFramerPortLineCodeViolationErrorCounterShadow;


typedef union
{
    struct AgRegFramerPortEBitErrorCounterShadow_S
    {
#ifdef BE_HOST
        field_t n_e_bit_error_count                    : 16;
#else
	field_t  n_e_bit_error_count  :  16;
#endif
	    } x_fields;

    AG_U16      n_reg;
}AgRegFramerPortEBitErrorCounterShadow;


typedef union
{
    struct AgRegFramerPortFrameAlignmentErrorSlipCountersShadow_S
    {
#ifdef BE_HOST
        field_t n_reserved			                   : 2,
	    n_transmit_slip_count                  : 3,
	    n_recive_slip_count                    : 3,
	    n_frame_alim_error_count               : 8;
#else
	field_t  				n_frame_alim_error_count  :  8, 
	    n_recive_slip_count  :  3, 
	    n_transmit_slip_count  :  3, 
	    n_reserved			  :  2; 
#endif
    } x_fields;

    AG_U16      n_reg;
}AgRegFramerPortFrameAlignmentErrorSlipCountersShadow;

typedef union
{
    struct AgRegFramerPortSeverelyErroredFrameCOFACounterShadow_S
    {
#ifdef BE_HOST
        field_t n_change_framer_alim_count			   : 8,
	    n_sev_frame_error_count                : 8;
#else
	field_t  				n_sev_frame_error_count  :  8, 
	    n_change_framer_alim_count			  :  8; 
#endif
    } x_fields;

    AG_U16      n_reg;
}AgRegFramerPortSeverelyErroredFrameCOFACounterShadow;

typedef union
{
    struct AgRegFramerPortTimeSlotLoopbackControlRegisters_S
    {
#ifdef BE_HOST
        field_t n_time_slot_mask                    : 16;
#else
	field_t  n_time_slot_mask  :  16;
#endif
	    } x_fields;

    AG_U16      n_reg;
}AgRegFramerPortTimeSlotLoopbackControlRegisters;


typedef union
{
    struct AgRegFramerPortMaintenanceSlipControl_S
    {
#ifdef BE_HOST
        field_t n_reserved			                   : 6,
	    b_time_slot_loopback_enable			   : 1,
	    b_freeze_state_tran_sign_sorce_sel	   : 1,
	    b_trans_slip_buffer_re_center		   : 1,
	    b_receive_slip_buffer_re_center		   : 1,
	    b_trans_sign_freeze                    : 1,
	    b_receive_sign_freeze                  : 1,
	    b_framer_payload_loopback_enable       : 1,
	    b_remote_e1_t1_loopback_enable         : 1,
	    b_local_e1_t1_loopback_enable          : 1,
	    b_transmit_slip_buffer_enable          : 1;
#else
	field_t  				b_transmit_slip_buffer_enable  :  1, 
	    b_local_e1_t1_loopback_enable  :  1, 
	    b_remote_e1_t1_loopback_enable  :  1, 
	    b_framer_payload_loopback_enable  :  1, 
	    b_receive_sign_freeze  :  1, 
	    b_trans_sign_freeze  :  1, 
	    b_receive_slip_buffer_re_center		  :  1, 
	    b_trans_slip_buffer_re_center		  :  1, 
	    b_freeze_state_tran_sign_sorce_sel	  :  1, 
	    b_time_slot_loopback_enable			  :  1, 
	    n_reserved			  :  6; 
#endif
    } x_fields;

    AG_U16      n_reg;
}AgRegFramerPortMaintenanceSlipControl;
#endif


#define AG_ND_CHANNEL(id)     ((unsigned long)(id) << 12)
#define AG_ND_CIRCUIT(id)     ((unsigned long)(id) << 12)
#define AG_ND_PTP_IDX(id)     ((unsigned long)(id) << 12)
#define AG_ND_TIMESLOT(id)    ((unsigned long)(id) << 1)
#define AG_ND_16BIT_ENTRY(id) ((unsigned long)(id) << 1)
#define AG_ND_32BIT_ENTRY(id) ((unsigned long)(id) << 2)

/* */
/* Global / Host interface registers */
/* */
#define AG_REG_FPGA_ID                                                  0x000000
#define AG_REG_GLOBAL_CONTROL                                           0x000004
#define AG_REG_CHANNELIZER_LOOPBACK_CHANNEL_SELECT                      0x000006
#define AG_REG_GLOBAL_CLK_ACTIVITY_MONITORING                           0x000008
#define AG_REG_TEST_BUS_CONTROL                                         0x00000A
#define AG_REG_EXTENDED_ADDRESS                                         0x00000C
#ifndef CES16_BCM_VERSION
 #define AG_REG_MEMORY_CONFIGURATION_ADDRESS                             0x00000E
#endif
/* */
/* Interrupt handling registers */
/* */
#define AG_REG_GLOBAL_INTERRUPT_ENABLE                                  0x000010
#define AG_REG_GLOBAL_INTERRUPT_STATUS                                  0x000012

/*hdlc debug */
#ifndef CES16_BCM_VERSION
 #define AG_REG_HDLC_SW_RESET                                            0x000018
#endif
/* */
/* Nemo TDM interface registers */
/* */
#define AG_REG_T1_E1_CCLK_MUX_1                                         0x000200                                                  
#define AG_REG_T1_E1_CCLK_MUX_2                                         0x000202                                                  
#define AG_REG_TDM_PORT_ACTIVITY_MONITORING                             0x000204                                                  
#define AG_REG_PAYLOAD_REPLACEMENT_POLICY                               0x000206
#define AG_REG_PACKET_PAYLOAD_REPLACEMENT_BYTE_PATTERN                  0x000208                                                  
#define AG_REG_T1_E1_PORT_CONFIGURATION(circuit)                       (0x000210 + AG_ND_CIRCUIT(circuit))                              
#define AG_REG_PORT_CLK_BAUD_RATE_GENERATOR1(circuit)                  (0x000214 + AG_ND_CIRCUIT(circuit))                              
#define AG_REG_PORT_CLK_BAUD_RATE_GENERATOR2(circuit)                  (0x000216 + AG_ND_CIRCUIT(circuit))                              
#define AG_REG_RTP_TIMESTAMP_GENERATOR_CONFIGURATION                    0x000220
#define AG_REG_DCR_COMMON_REF_CLK_SOURCE_SELECTION                      0x000222
#define AG_REG_RTP_TIMESTAMP_STEP_SIZE_1                                0x000224
#define AG_REG_RTP_TIMESTAMP_STEP_SIZE_2                                0x000226
#define AG_REG_RTP_TIMESTAMP_FAST_PHASE_CONFIGURATION                   0x000228
#define AG_REG_RTP_TIMESTAMP_ERROR_CORRECTION_PERIOD                    0x00022A
#define AG_REG_RECOVERED_CLK_PRIME_LOCAL_TIMESTAMP_1(circuit)          (0x00022C + AG_ND_CIRCUIT(circuit))
#define AG_REG_RECOVERED_CLK_PRIME_LOCAL_TIMESTAMP_2(circuit)          (0x00022E + AG_ND_CIRCUIT(circuit))
#define AG_REG_RECOVERED_CLK_TIMESTAMP_SAMPLING_PERIOD_1(circuit)      (0x000230 + AG_ND_CIRCUIT(circuit))
#define AG_REG_RECOVERED_CLK_TIMESTAMP_SAMPLING_PERIOD_2(circuit)      (0x000232 + AG_ND_CIRCUIT(circuit))
#define AG_REG_RECOVERED_CLK_LOCAL_TIMESTAMP_1(circuit)                (0x000234 + AG_ND_CIRCUIT(circuit))
#define AG_REG_RECOVERED_CLK_LOCAL_TIMESTAMP_2(circuit)                (0x000236 + AG_ND_CIRCUIT(circuit))
#define AG_REG_RECOVERED_CLK_FAST_PHASE_TIMESTAMP(circuit)             (0x000238 + AG_ND_CIRCUIT(circuit))

/* */
/* Neptune TDM interface registers */
/* */
#define AG_REG_TDM_BUS_INTERFACE_CONFIGURATION                          0x000240

#define AG_REG_DCR_SYSTEM_TSO_CLK_SOURCE_SELECTION(circuit)            (0x000240 + AG_ND_CIRCUIT(circuit))

#define AG_REG_TDM_BUS_HIERARCHY_CONFIGURATION                          0x000242
#define AG_REG_TRIBUTARY_CIRCUIT_TYPE_PER_VC_2_1                        0x000244
#define AG_REG_TRIBUTARY_CIRCUIT_TYPE_PER_VC_2_2                        0x000246
#define AG_REG_VC2_HIERARCHY_CONFIGURATION_1                            0x000248
#define AG_REG_VC2_HIERARCHY_CONFIGURATION_2                            0x00024A
#define AG_REG_TRIBUTARY_CIRCUIT_CONFIGURATION(circuit)                (0x000250 + AG_ND_CIRCUIT(circuit))
#define AG_REG_BIT_STUFFING_GENERATOR_1(circuit)                       (0x000254 + AG_ND_CIRCUIT(circuit))
#define AG_REG_BIT_STUFFING_GENERATOR_2(circuit)                       (0x000256 + AG_ND_CIRCUIT(circuit))
#define AG_REG_JUSTIFICATION_REQUEST_CONTROL(circuit)                  (0x000258 + AG_ND_CIRCUIT(circuit))
#define AG_REG_RECOVERED_LINK_RATE_PHASE_INTEGRATOR(circuit)           (0x00025A + AG_ND_CIRCUIT(circuit))
#define AG_REG_RTP_TIMESTAMP_GENERATOR_CONFIGURATION_OC3                0x000260
#define AG_REG_RTP_TIMESTAMP_FRAME_STEP_SIZE                            0x000262
#define AG_REG_RTP_TIMESTAMP_BIT_STEP_SIZE                              0x000264
#define AG_REG_RTP_TIMESTAMP_RATE										0x00026C

/* */
/* CAS registers */
/* */
#ifndef CES16_BCM_VERSION
 #define AG_REG_CAS_INGRESS_DATA_BUFFER(circuit,entry)                  (0x000270 + AG_ND_CIRCUIT(circuit) + AG_ND_16BIT_ENTRY(entry))
 #define AG_REG_CAS_EGRESS_DATA_BUFFER(circuit,entry)                   (0x000280 + AG_ND_CIRCUIT(circuit) + AG_ND_16BIT_ENTRY(entry))
#endif
#define AG_REG_CAS_INGRESS_CHANGE_DETECTION(circuit,entry)             (0x000290 + AG_ND_CIRCUIT(circuit) + AG_ND_16BIT_ENTRY(entry))
#define AG_REG_CAS_DATA_REPLACEMENT(circuit,entry)                     (0x000294 + AG_ND_CIRCUIT(circuit) + AG_ND_16BIT_ENTRY(entry))
#define AG_REG_CAS_IDLE_PATTERN                                         0x000298

#define AG_REG_DCR_SYSTEM_REF_CLK_SOURCE_SELECTION(circuit)            (0x00020C + AG_ND_CIRCUIT(circuit))

/*ORI*/
#ifndef CES16_BCM_VERSION
#define AG_REG_BSG_ONE_SHOT_COUNTER(circuit)            			   (0x0002A0 + AG_ND_CIRCUIT(circuit))
#else
#define AG_REG_PTP_BAUD_RATE_GENERATOR(ptp,entry)                      (0x0002A0 + AG_ND_PTP_IDX(ptp) + AG_ND_16BIT_ENTRY(entry))
#endif	

/*ORI*/
#ifdef CES16_BCM_VERSION
#define AG_REG_RECOVERED_CLK_SYSTEM_LOCAL_TIMESTAMP_1(sys_clk)         (0x0002c0 + AG_ND_CIRCUIT(sys_clk))
#define AG_REG_RECOVERED_CLK_SYSTEM_LOCAL_TIMESTAMP_2(sys_clk)         (0x0002c2 + AG_ND_CIRCUIT(sys_clk))
#define AG_REG_RECOVERED_CLK_SYSTEM_FAST_PHASE_TIMESTAMP(sys_clk)      (0x0002c4 + AG_ND_CIRCUIT(sys_clk))
#define AG_REG_RECOVERED_CLK_SYSTEM_TIMESTAMP_SAMPLING_PERIOD_1(circuit)      (0x0002c8 + AG_ND_CIRCUIT(circuit))
#define AG_REG_RECOVERED_CLK_SYSTEM_TIMESTAMP_SAMPLING_PERIOD_2(circuit)      (0x0002cA + AG_ND_CIRCUIT(circuit))

#endif


/* */
/* PIF   Packet interface configuration registers */
/* */
#ifndef CES16_BCM_VERSION
#define AG_REG_PIF_REV                                                  0x000300
#define AG_REG_PIF_SCRATCH                                              0x000304
#define AG_REG_PIF_COMMAND_CONFIG_1                                     0x000308
#define AG_REG_PIF_COMMAND_CONFIG_2                                     0x00030A
#define AG_REG_PIF_MAC_0                                                0x00030C
#define AG_REG_PIF_MAC_1                                                0x000310
#define AG_REG_PIF_FRM_LENGTH                                           0x000314
#define AG_REG_PIF_PAUSE_QUANT                                          0x000318
#define AG_REG_PIF_RX_SECTION_EMPTY                                     0x00031C
#define AG_REG_PIF_RX_SECTION_FULL                                      0x000320
#define AG_REG_PIF_TX_SECTION_EMPTY                                     0x000324
#define AG_REG_PIF_TX_SECTION_FULL                                      0x000328
#define AG_REG_PIF_RX_ALMOST_EMPTY                                      0x00032C
#define AG_REG_PIF_RX_ALMOST_FULL                                       0x000330
#define AG_REG_PIF_TX_ALMOST_EMPTY                                      0x000334
#define AG_REG_PIF_TX_ALMOST_FULL                                       0x000338
#define AG_REG_PIF_MDIO_ADDR_0                                          0x00033C
#define AG_REG_PIF_MDIO_ADDR_1                                          0x000340
#define AG_REG_PIF_REG_STAT                                             0x000358
#define AG_REG_PIF_TX_IPG_LENGTH                                        0x00035C
/* */
/* PIF   Packet interface statistics registers */
/* */
#define AG_REG_PIF_MACID_0                                              0x000360
#define AG_REG_PIF_MACID_1                                              0x000364
#define AG_REG_PIF_FRAMES_TRANSMITTED_OK                                0x000368
#define AG_REG_PIF_FRAMES_RECEIVED_OK                                   0x00036C
#define AG_REG_PIF_FRAME_CHECK_SEQUENCE_ERRORS                          0x000370
#define AG_REG_PIF_ALIGNMENT_ERRORS                                     0x000374
#define AG_REG_PIF_OCTETS_TRANSMITTED_OK                                0x000378
#define AG_REG_PIF_OCTETS_RECEIVED_OK                                   0x00037C
#define AG_REG_PIF_TX_PAUSE_MAC_CTRL_FRAMES                             0x000380
#define AG_REG_PIF_RX_PAUSE_MAC_CTRL_FRAMES                             0x000384
#define AG_REG_PIF_IN_ERRORS                                            0x000388
#define AG_REG_PIF_OUT_ERRORS                                           0x00038C
#define AG_REG_PIF_IN_UNICAST_PACKETS                                   0x000390
#define AG_REG_PIF_IN_MULTICAST_PACKETS                                 0x000394
#define AG_REG_PIF_IN_BROADCAST_PACKETS                                 0x000398
#define AG_REG_PIF_OUT_DISCARDS                                         0x00039C
#define AG_REG_PIF_OUT_UNICAST_PACKETS                                  0x0003A0
#define AG_REG_PIF_OUT_MULTICAST_PACKETS                                0x0003A4
#define AG_REG_PIF_OUT_BROADCAST_PACKETS                                0x0003A8
#define AG_REG_PIF_STATS_DROP_EVENTS                                    0x0003AC
#define AG_REG_PIF_STATS_OCTETS                                         0x0003B0
#define AG_REG_PIF_STATS_PACKETS                                        0x0003B4
#define AG_REG_PIF_STATS_UNDERSIZE_PACKETS                              0x0003B8
#define AG_REG_PIF_STATS_OVERSIZE_PACKETS                               0x0003BC
#define AG_REG_PIF_STATS_PACKETS_64_OCTETS                              0x0003C0
#define AG_REG_PIF_STATS_PACKETS_65_TO_127_OCTETS                       0x0003C4
#define AG_REG_PIF_STATS_PACKETS_128_TO_255_OCTETS                      0x0003C8
#define AG_REG_PIF_STATS_PACKETS_256_TO_511_OCTETS                      0x0003CC
#define AG_REG_PIF_STATS_PACKETS_512_TO_1023_OCTETS                     0x0003D0
#define AG_REG_PIF_STATS_PACKETS_1024_TO_1518_OCTETS                    0x0003D4

#define AG_REG_PIF_PHY_DEVICE_0_MDIO(entry)                            (0x002300 + AG_ND_32BIT_ENTRY(entry))

#else
/*This is for BCM*/
#define AG_REG_PIF_PIFCSR                                               0x000300   /*PIF Control and status register*/
#define AG_REG_PIF_TXFILLTH                                             0x000310   /*Transmit Fill Threshold*/
#define AG_REG_PIF_TXAFULLTH                                            0x000312   /*Transmit Almost Full Threshold Register*/
#define AG_REG_PIF_TXAEMPTH                                             0x000314   /*Transmit Almost Empty Threshold Register*/
#define AG_REG_PIF_TXPREAMBLE                                           0x000316   /*Transmit Preamble Register*/
#define AG_REG_PIF_RXFILLTH                                             0x000320   /*Receive Fill Threshold Register*/
#endif                                                                       


/* */
/* CHI   Ingress channelizer registers */
/* */
#define AG_REG_INGRESS_CIRCUIT_TO_CHANNEL_MAP(circuit,timeslot)        (0x000400 + AG_ND_CIRCUIT(circuit) + AG_ND_TIMESLOT(timeslot))
#define AG_REG_INGRESS_CHANNEL_CONTROL(channel)                        (0x000440 + AG_ND_CHANNEL(channel))
#ifndef CES16_BCM_VERSION
 #define AG_REG_INGRESS_TIMESLOT_ALLOCATION_CONTROL(channel)            (0x000442 + AG_ND_CHANNEL(channel))
#endif                                                                                                                                  
/* */
/* CHE   Egress channelizer registers */
/* */
#define AG_REG_EGRESS_CIRCUIT_TO_CHANNEL_MAP(circuit,timeslot)         (0x000500 + AG_ND_CIRCUIT(circuit) + AG_ND_TIMESLOT(timeslot))         
#define AG_REG_EGRESS_CHANNEL_CONTROL(channel)                         (0x000540 + AG_ND_CHANNEL(channel))
#ifndef CES16_BCM_VERSION
 #define AG_REG_EGRESS_TIMESLOT_ALLOCATION_CONTROL(channel)             (0x000542 + AG_ND_CHANNEL(channel))
#endif                                                                                                                                  
/* */
/* PBF   Transmit payload buffer registers */
/* */
#define AG_REG_PAYLOAD_BUFFER_CHANNEL_CONFIGURATION(channel)           (0x000800 + AG_ND_CHANNEL(channel)) 
#ifndef CES16_BCM_VERSION                             
 #define AG_REG_PAYLOAD_BUFFER_QUEUE_BASE_ADDRESS(channel)              (0x000802 + AG_ND_CHANNEL(channel))                              
 #define AG_REG_PAYLOAD_BUFFER_STATUS(channel)                          (0x000804 + AG_ND_CHANNEL(channel))                              
#endif
#define AG_REG_TRANSMIT_HEADER_CONFIGURATION(channel)                  (0x000806 + AG_ND_CHANNEL(channel))                              
#ifndef CES16_BCM_VERSION
 #define AG_REG_TRANSMIT_HEADER_ACCESS_CONTROL                           0x000810
#endif
#define AG_REG_PBF_CAS_CONFIGURATION(channel)                          (0x000820 + AG_ND_CHANNEL(channel))                              
#define AG_REG_PAYLOAD_CAS_BUFFER_STATUS(channel)                      (0x000824 + AG_ND_CHANNEL(channel))                              
#define AG_REG_TRANSMIT_CAS_HEADER_CONFIGURATION(channel)              (0x000826 + AG_ND_CHANNEL(channel))                              
#ifndef CES16_BCM_VERSION
 #define AG_REG_TRANSMIT_CAS_HEADER_ACCESS_CONTROL                       0x000828
#endif
#define AG_REG_TRANSMIT_CAS_PACKET_DELAY                                0x00082A
#ifndef CES16_BCM_VERSION
 #define AG_REG_PBF_PTP_CONFIGURATION_REGISTER(channel)                 (0x000830 + AG_ND_CHANNEL(channel))
 #define AG_REG_PTP_TRANSMIT_HEADER_ACCESS_CONTROL                       0x000832
#endif
/* */
/* JBF   Jitter buffer registers */
/* */
#define AG_REG_JITTER_BUFFER_CHANNEL_CONFIGURATION_1(channel)          (0x000900 + AG_ND_CHANNEL(channel))
#define AG_REG_JITTER_BUFFER_CHANNEL_CONFIGURATION_2(channel)          (0x000902 + AG_ND_CHANNEL(channel))
#ifndef CES16_BCM_VERSION
 #define AG_REG_JITTER_BUFFER_QUEUE_BASE_ADDRESS(channel)               (0x000904 + AG_ND_CHANNEL(channel))
 #define AG_REG_JITTER_BUFFER_VALID_BIT_ARRAY_CONFIGURATION(channel)    (0x000906 + AG_ND_CHANNEL(channel))
#endif
#define AG_REG_CAS_MISORDERED_DROPPED_PACKET_COUNT(channel)            (0x000908 + AG_ND_CHANNEL(channel))
#define AG_REG_PWE3_IN_BAD_LENGTH_PACKET_COUNT(channel)                (0x00090A + AG_ND_CHANNEL(channel))
#define AG_REG_MISORDERED_DROPPED_PACKET_COUNT(channel)                (0x00090C + AG_ND_CHANNEL(channel))
#define AG_REG_REORDERED_GOOD_PACKET_COUNT(channel)                    (0x00090E + AG_ND_CHANNEL(channel))
#define AG_REG_MINIMUM_JITTER_BUFFER_DEPTH_1(channel)                  (0x000910 + AG_ND_CHANNEL(channel))
#define AG_REG_MINIMUM_JITTER_BUFFER_DEPTH_2(channel)                  (0x000912 + AG_ND_CHANNEL(channel))
#define AG_REG_MAXIMUM_JITTER_BUFFER_DEPTH_1(channel)                  (0x000914 + AG_ND_CHANNEL(channel))
#define AG_REG_MAXIMUM_JITTER_BUFFER_DEPTH_2(channel)                  (0x000916 + AG_ND_CHANNEL(channel))
#define AG_REG_JITTER_BUFFER_MISSING_PACKET_COUNT(channel)             (0x000918 + AG_ND_CHANNEL(channel))
#define AG_REG_JITTER_BUFFER_UNDERRUN_PACKET_COUNT(channel)            (0x00091A + AG_ND_CHANNEL(channel))
#define AG_REG_JITTER_BUFFER_RESTART_COUNT(channel)                    (0x00091C + AG_ND_CHANNEL(channel))
#define AG_REG_JITTER_BUFFER_CHANNEL_STATUS(channel)                   (0x00091E + AG_ND_CHANNEL(channel))
#define AG_REG_JITTER_BUFFER_SLIP_COMMAND                               0x000920
#ifndef CES16_BCM_VERSION
 #define AG_REG_JITTER_BUFFER_SLIP_CONFIGURATION                         0x000922
#endif
#define AG_REG_PACKET_SYNCHRONIZATION_INTERRUPT_EVENT_QUEUE             0x000924
#define AG_REG_PACKET_SYNCHRONIZATION_INTERRUPT_QUEUE_STATUS            0x000926
#define AG_REG_PACKET_SYNC_STATUS(channel)                             (0x000928 + AG_ND_CHANNEL(channel))
#define AG_REG_LOSS_OF_PACKET_SYNC_THRESHOLD_TABLE(entry)              (0x000940 + AG_ND_16BIT_ENTRY(entry))
#define AG_REG_ACQUISITION_OF_PACKET_SYNC_THRESHOLD_TABLE(entry)       (0x000950 + AG_ND_16BIT_ENTRY(entry))
#define AG_REG_JITTER_BUFFER_CAS_CONFIGURATION(channel)                (0x000960 + AG_ND_CHANNEL(channel))
#define AG_REG_JITTER_BUFFER_CAS_SEQUENCE_NUMBER_WINDOW                 0x000962

/* */
/* TPE   Transmit packet encapsulator registers */
/* */
#define AG_REG_TPE_GLOBAL_CONFIGURATION                                 0x000A00
#ifndef CES16_BCM_VERSION
 #define AG_REG_PTP_OUTGOING_COUNTERS_CONFIGURATION(channel)            (0x000A06 + AG_ND_CHANNEL(channel))
#endif
#define AG_REG_TRANSMITTED_RTP_TIMESTAMP_1(channel)                    (0x000A08 + AG_ND_CHANNEL(channel))
#define AG_REG_TRANSMITTED_RTP_TIMESTAMP_2(channel)                    (0x000A0A + AG_ND_CHANNEL(channel))
#define AG_REG_PWE_OUT_TRANSMITTED_BYTES_1(channel)                    (0x000A10 + AG_ND_CHANNEL(channel))
#define AG_REG_PWE_OUT_TRANSMITTED_BYTES_2(channel)                    (0x000A12 + AG_ND_CHANNEL(channel))
#define AG_REG_PWE_OUT_TRANSMITTED_PACKETS(channel)                    (0x000A14 + AG_ND_CHANNEL(channel))
#define AG_REG_CAS_OUT_TRANSMITTED_PACKETS(channel)                    (0x000A16 + AG_ND_CHANNEL(channel))
#ifndef CES16_BCM_VERSION
 #define AG_REG_PTP_OUT_PACKETS_1_2(channel)                    		   (0x000A18 + AG_ND_CHANNEL(channel))
#endif
#define AG_REG_PW_OUT_SEQUENCE_NUMBER(channel)                         (0x000A20 + AG_ND_CHANNEL(channel))
#define AG_REG_CAS_OUT_SEQUENCE_NUMBER(channel)                        (0x000A22 + AG_ND_CHANNEL(channel))

/* */
/* RPC   Receive packet classifier registers */
/* */
#define AG_REG_RECEIVE_CHANNEL_CONFIGURATION(channel)                  (0x000B00 + AG_ND_CHANNEL(channel))
#ifndef CES16_BCM_VERSION
 #define AG_REG_LABEL_SEARCH_TABLE_BASE_ADDRESS                          0x000B02
 #define AG_REG_STRICT_CHECK_TABLE_BASE_ADDRESS                          0x000B04
 #define AG_REG_PTP_CHANNELS_STRICT_CHECK_TABLE_BASE_ADDRESS             0x000B06
#endif
#define AG_REG_CES_CONTROL_WORD_INTERRUPT_MASK                          0x000B08                                                  
#define AG_REG_CES_CONTROL_WORD_INTERRUPT_STATUS(channel)              (0x000B0A + AG_ND_CHANNEL(channel))
#define AG_REG_RECEIVE_CES_CONTROL_WORD(channel)                       (0x000B0C + AG_ND_CHANNEL(channel))
#define AG_REG_PWE_IN_RECEIVED_BYTES_1(channel)                        (0x000B10 + AG_ND_CHANNEL(channel))
#define AG_REG_PWE_IN_RECEIVED_BYTES_2(channel)                        (0x000B12 + AG_ND_CHANNEL(channel))
#define AG_REG_PWE_IN_RECEIVED_PACKETS(channel)                        (0x000B14 + AG_ND_CHANNEL(channel))
#define AG_REG_CLASSIFIER_DROPPED_PACKET_COUNT                          0x000B16
#ifndef CES16_BCM_VERSION
 #define AG_REG_HIGH_PRIORITY_HOST_PACKET_COUNT                          0x000B18
 #define AG_REG_LOW_PRIORITY_HOST_PACKET_COUNT                           0x000B1A
#endif
#define AG_REG_RAI_CONFIGURATION                                        0x000B1C
#define AG_REG_CES_CONTROL_WORD_INTERRUPT_EVENT_QUEUE                   0x000B20
#define AG_REG_CES_CONTROL_WORD_INTERRUPT_QUEUE_STATUS                  0x000B22
#ifndef CES16_BCM_VERSION
 #define AG_REG_RECEIVE_PTP_CHANNEL_CONFIGURATION(channel)              (0x000B24 + AG_ND_CHANNEL(channel))
 #define AG_REG_POLICY_PTP(entry)                                       (0x000B30 + AG_ND_16BIT_ENTRY(entry))
#endif
#define AG_REG_PRIORITIZED_COUNTER_SELECTION_MASK                       0x000B3A
#define AG_REG_GLOBAL_COUNTER_POLICY_SELECTION                          0x000B38
#define AG_REG_POLICY_STATUS_POLARITY(entry)                           (0x000B3C + AG_ND_16BIT_ENTRY(entry))
#define AG_REG_POLICY_DROP_UNCONDITIONAL(entry)                        (0x000B40 + AG_ND_16BIT_ENTRY(entry))
#define AG_REG_POLICY_DROP_IF_NOT_FORWARD(entry)                       (0x000B44 + AG_ND_16BIT_ENTRY(entry))
#define AG_REG_POLICY_FORWARD_HIGH(entry)                              (0x000B48 + AG_ND_16BIT_ENTRY(entry))
#define AG_REG_POLICY_FORWARD_LOW(entry)                               (0x000B4C + AG_ND_16BIT_ENTRY(entry))
#define AG_REG_POLICY_PER_CHANNEL_COUNTERS(entry)                      (0x000B50 + AG_ND_16BIT_ENTRY(entry))
#define AG_REG_POLICY_GLOBAL_COUNTERS(entry)                           (0x000B60 + AG_ND_16BIT_ENTRY(entry))
#define AG_REG_CLASSIFIER_GLOBAL_PM_COUNTER(entry)                     (0x000B80 + AG_ND_16BIT_ENTRY(entry))
#define AG_REG_CLASSIFIER_CHANNEL_SPECIFIC_PM_COUNTER(channel,entry)   (0x000B90 + AG_ND_CHANNEL(channel) + AG_ND_16BIT_ENTRY(entry))

/* */
/* TPP   Timing pre-processor registers */
/* */
#define AG_REG_TPP_CHANNEL_STATUS(entry)                               (0x000C00 + AG_ND_16BIT_ENTRY(entry))                                                  
#define AG_REG_TPP_CONFIG_TIMESTAMP_RATE                                0x000C0C
#define AG_REG_CLK_SOURCE_SELECTION(channel)                           (0x000C10 + AG_ND_CHANNEL(channel))                              
#define AG_REG_TIMING_CHANNEL_CONFIGURATION(channel)                   (0x000C20 + AG_ND_CHANNEL(channel))                              
#define AG_REG_PHASE_SLOPE_LIMIT_THRESHOLD(channel)                    (0x000C22 + AG_ND_CHANNEL(channel))                              
#define AG_REG_PHASE_SLOPE_LIMIT_PM_COUNTER(channel)                   (0x000C24 + AG_ND_CHANNEL(channel))                              
#define AG_REG_PEAK_DETECTOR_PM_COUNTER(channel)                       (0x000C26 + AG_ND_CHANNEL(channel))                              
#define AG_REG_ARRIVAL_TIMESTAMP_BASE_1(channel)                       (0x000C30 + AG_ND_CHANNEL(channel))                              
#define AG_REG_ARRIVAL_TIMESTAMP_BASE_2(channel)                       (0x000C32 + AG_ND_CHANNEL(channel))                              
#define AG_REG_ARRIVAL_TIMESTAMP_SIGMA_1(channel)                      (0x000C34 + AG_ND_CHANNEL(channel))                              
#define AG_REG_ARRIVAL_TIMESTAMP_SIGMA_2(channel)                      (0x000C36 + AG_ND_CHANNEL(channel))                              
#define AG_REG_ARRIVAL_TIMESTAMP_COUNT(channel)                        (0x000C38 + AG_ND_CHANNEL(channel))                              
#define AG_REG_SQN_FILLER_SIGMA_1(channel)                             (0x000C3C + AG_ND_CHANNEL(channel))                              
#define AG_REG_SQN_FILLER_SIGMA_2(channel)                             (0x000C3E + AG_ND_CHANNEL(channel))                              
#define AG_REG_JITTER_BUFFER_DEPTH_PACKET_COUNT(channel)               (0x000C40 + AG_ND_CHANNEL(channel))                              
#define AG_REG_JITTER_BUFFER_DEPTH_ACCUMULATION_1(channel)             (0x000C44 + AG_ND_CHANNEL(channel))                              
#define AG_REG_JITTER_BUFFER_DEPTH_ACCUMULATION_2(channel)             (0x000C46 + AG_ND_CHANNEL(channel))                              
#define AG_REG_TIMING_INTERFACE_JITTER_BUFFER_MAXIMUM_DEPTH_1(channel) (0x000C48 + AG_ND_CHANNEL(channel))                              
#define AG_REG_TIMING_INTERFACE_JITTER_BUFFER_MAXIMUM_DEPTH_2(channel) (0x000C4A + AG_ND_CHANNEL(channel))                              
#define AG_REG_RECEIVED_PACKET_RTP_TIMESTAMP_1(channel)                (0x000C50 + AG_ND_CHANNEL(channel))                              
#define AG_REG_RECEIVED_PACKET_RTP_TIMESTAMP_2(channel)                (0x000C52 + AG_ND_CHANNEL(channel))                              
#define AG_REG_125MHZ_ORIGINATED_TPP_TIMESTAMP_1                        0x000C54
#define AG_REG_125MHZ_ORIGINATED_TPP_TIMESTAMP_2                        0x000C56
#define AG_REG_77MHZ_ORIGINATED_TPP_TIMESTAMP_1                         0x000C58
#define AG_REG_77MHZ_ORIGINATED_TPP_TIMESTAMP_2                         0x000C5A

/* */
/* PTP registers */
/* */
#ifndef CES16_BCM_VERSION
#define AG_REG_PTP_TIMESTAMP_GENERATORS_CONFIGURATION                   0x000D00
#define AG_REG_PTP_TIMESTAMP_CLOCK_SOURCE_SELECTION(ptp)                0x000D02 + AG_ND_PTP_IDX(ptp)
#define AG_REG_PTP_TIMESTAMP_STEP_SIZE_INTEGER                          0x000D04
#define AG_REG_PTP_TIMESTAMP_STEP_SIZE_FRACTION(entry)                  0x000D06 + AG_ND_16BIT_ENTRY(entry)
#define AG_REG_PTP_TIMESTAMP_FAST_PHASE_CONFIGURATION                   0x000D0a
#define AG_REG_PTP_TIMESTAMP_ERROR_CORRECTION_PERIOD                    0x000D0c
#define AG_REG_PTP_TIMESTAMP_CORRECTION_VALUE(ptp,entry)               (0x000D10 + AG_ND_PTP_IDX(ptp) + AG_ND_16BIT_ENTRY(entry))
#define AG_REG_PTP_BAUD_RATE_GENERATOR(ptp,entry)                      (0x000D20 + AG_ND_PTP_IDX(ptp) + AG_ND_16BIT_ENTRY(entry))
#define AG_REG_PTP_BAUD_RATE_GENERATOR_CORRECTION(ptp)                 (0x000D24 + AG_ND_PTP_IDX(ptp))
#define AG_REG_PTP_CONFIGURATION(channel)                              (0x000D30 + AG_ND_CHANNEL(channel))
#define AG_REG_PTP_FILTER                                               0x000D32
/* */
/* System diff elemetns */
/* */
/*ORI*/
/*THIS REGS MOVED FOR B002*/
#ifndef CES16_BCM_VERSION
#define AG_REG_RECOVERED_CLK_SYSTEM_LOCAL_TIMESTAMP_1(sys_clk)         (0x000D40 + AG_ND_CIRCUIT(sys_clk))
#define AG_REG_RECOVERED_CLK_SYSTEM_LOCAL_TIMESTAMP_2(sys_clk)         (0x000D42 + AG_ND_CIRCUIT(sys_clk))
#define AG_REG_RECOVERED_CLK_SYSTEM_FAST_PHASE_TIMESTAMP(sys_clk)      (0x000D44 + AG_ND_CIRCUIT(sys_clk))
#define AG_REG_RECOVERED_CLK_SYSTEM_TIMESTAMP_SAMPLING_PERIOD_1(circuit)      (0x000D48 + AG_ND_CIRCUIT(circuit))
#define AG_REG_RECOVERED_CLK_SYSTEM_TIMESTAMP_SAMPLING_PERIOD_2(circuit)      (0x000D4A + AG_ND_CIRCUIT(circuit))
#endif
#define AG_REG_PTP_IN_PACKETS_1_2(channel)                 			   		  (0x000D50 + AG_ND_CHANNEL(channel))
#define AG_REG_PTP_IN_PACKETS_3(channel)   	               			   		  (0x000D52 + AG_ND_CHANNEL(channel))
#endif
/* */
/* HPL   Host-Packet link registers */
/* */
#ifndef CES16_BCM_VERSION
 #define AG_REG_HOST_PACKET_LINK_CONFIGURATION                           0x000E00
 #define AG_REG_RECEIVED_HOST_PACKET_DROP_COUNT                          0x000E04                                                  
 #define AG_REG_PM_EXPORT_STRUCTURE_ASSEMBLY_REGISTER(entry)            (0x000E08 + AG_ND_16BIT_ENTRY(entry))
 #define AG_REG_PM_COUNTER_EXPORT_CHANNEL_SELECTION(channel)            (0x000E10 + AG_ND_CHANNEL(channel))                           
#endif

/*ORI*/
/*regs for framer */
#ifdef CES16_BCM_VERSION
#define AG_REG_FRAMER_COMMON_CONFIGURATION_LOWER						0x000104
#define AG_REG_FRAMER_COMMON_CONFIGURATION_UPPER						0x0008104

#define AG_REG_FRAMER_PORT_CONFIGURATION(circuit)									(0x000118 + AG_ND_CIRCUIT(circuit))
#define AG_REG_FRAMER_PORT_ALARM_CONTROL_REGISTER(circuit) 					(0X00011A + AG_ND_CIRCUIT(circuit))

#define AG_REG_FRAMER_PORT_MAINRENANCE_SLIP_CONTROL_REGISTERS(circuit)				(0x00011E + AG_ND_CIRCUIT(circuit))
#define AG_REG_FRAMER_PORT_TIME_SLOT_LOOPBACK_CONTROL_REGISTERS1(circuit)			(0x000120 + AG_ND_CIRCUIT(circuit))
#define AG_REG_FRAMER_PORT_TIME_SLOT_LOOPBACK_CONTROL_REGISTERS2(circuit)			(0x000122 + AG_ND_CIRCUIT(circuit))

#define AG_REG_FRAMER_PORT_STATUS_1(circuit)							(0x00015C + AG_ND_CIRCUIT(circuit)) 

/*regs fpr pm of  fpga */
#define AG_REG_FRAMER_PORT_CRC_ERROR_COUNTER_SHADOW_REGISTER(circuit)      				(0x000166 + AG_ND_CIRCUIT(circuit))
#define AG_REG_FRAMER_PORT_LINE_CODE_VIOLATION_ERROR_COUNTER_SHADOW_REGISTER(circuit)      (0x000168 + AG_ND_CIRCUIT(circuit))
#define AG_REG_FRAMER_PORT_EBIT_ERROR_COUNTER_SHADOW_REGISTER(circuit)      				(0x00016A + AG_ND_CIRCUIT(circuit))
#define AG_REG_FRAMER_PORT_FRAME_ALIGNMENT_ERROR_SLIP_COUNTERS_SHADOW_REGISTER(circuit)    (0x00016C + AG_ND_CIRCUIT(circuit))
#define AG_REG_FRAMER_PORT_SEVERELY_ERRORED_FRAME_COFA_COUNTERS_SHADOW_REGISTER(circuit)   (0x00016E + AG_ND_CIRCUIT(circuit))





#endif

#ifdef __cplusplus
}
#endif

#endif /* _NEMO_REGISTERS_H_ */


