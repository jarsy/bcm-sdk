/*! \file port_pp.h
 * $Id$
 * 
 * Internal DNX Port APIs 
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef _PORT_TPID_H_INCLUDED_
/* { */
#define _PORT_TPID_H_INCLUDED_

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

#define DNX_ETHERNET_FRAME_VLAN_FORMAT_NONE 0
#define DNX_PORT_TPID_TRAP_CODE_ACCEPTABLE_FRAME_TYPE_DROP 0
#define DNX_PORT_TPID_TRAP_CODE_ACCEPTABLE_FRAME_TYPE_ACCEPT 1
/*
 * The template of each LLVP profile has 19*2^7=2432 bits
 * It is implemented by 76 uint32's (76*32=2432)
 */
#define DNX_PORT_TPID_CLASS_TEMPLATE_SIZE_NUM_OF_UINT32 76
/*
 * The size of each LLVP table entry buffer
 */
#define DNX_PORT_TPID_BUFFER_BITS_PER_TAG_STRCT 19

#define DNX_PORT_TPID_TAG_FORMAT_MAX_ID 7
#define DNX_PORT_TPID_LLVP_BLOCK_SIZE 128

/*
 * Parser fields that are part of LLVP table key
 */
typedef struct dnx_port_tpid_llvp_parser_info_s
{
    uint32 is_outer_prio;
    uint32 outer_tpid;
    uint32 inner_tpid;
} dnx_port_tpid_llvp_parser_info_t;

/*
 * ingress LLVP table payload
 */
typedef struct dnx_port_tpid_ingress_llvp_entry_s
{
    int incoming_vid_exist;
    int incoming_tag_exist;
    int incoming_s_tag_exist;
    int frame_type_action;
    int llvp_tag_format;
    int ivec_index;
    int pcp_dei_profile;
} dnx_port_tpid_ingress_llvp_entry_t;

/*
 * egress LLVP table payload
 */
typedef struct dnx_port_tpid_egress_llvp_entry_s
{
    int c_tag_offset;
    int packet_has_c_tag;
    int packet_has_pcp_dei;
    int llvp_tag_format;
} dnx_port_tpid_egress_llvp_entry_t;

/**
 * \brief - Verify function for bcm_dnx_port_tpid_class_set
 */
shr_error_e
dnx_port_tpid_class_set_verify(
    int unit,
    bcm_port_tpid_class_t *tpid_class);

/**
 * \brief Convert bcm_port_tpid_class_t to LLVP entry internal buffer
 */
shr_error_e
dnx_port_tpid_class_set_tpid_class_to_buff(
    int unit,
    bcm_port_tpid_class_t *tpid_class,
    uint32 *ingress_buff,
    uint32 *egress_buff);

/**
 * \brief Convert LLVP entry internal buffer to bcm_port_tpid_class_t
 */
shr_error_e
dnx_port_tpid_class_set_buff_to_tpid_class(
    int unit,
    uint32 buff,
    bcm_port_tpid_class_t *tpid_class);

/**
 * \brief Calculate outer tag is priority value
 */
shr_error_e
dnx_port_tpid_class_set_tag_priority_get(
    int unit,
    bcm_port_tpid_class_t *tpid_class,
    uint32 *tag_priority_val,
    uint32 *nof_tag_priority_vals);

/**
 * \brief Update a single entry of LLVP profile block
 * which includes 2^7 entries
 */
shr_error_e
dnx_port_tpid_class_set_tpid_class_buffer_update(
    int unit,
    dnx_port_tpid_llvp_parser_info_t llvp_parser_info,
    uint32 tag_buff,
    uint32 *tpid_class_buffer);

/**
 * \brief Convert parser_info to LLVP table index
 */
shr_error_e
dnx_port_tpid_class_set_parser_info_to_llvp_index(
    int unit,
    dnx_port_tpid_llvp_parser_info_t llvp_parser_info,
    int *tag_structure_index);


/**
 * \brief Write single LLVP entry pointed by its offset
 */
shr_error_e
dnx_port_tpid_class_set_ingress_llvp_entry_write(
    int unit,
    int llvp_profile,
    dnx_port_tpid_llvp_parser_info_t llvp_parser_info,
    uint32 ingress_tag_buff);

/**
  * \brief Convert buffer and parser_info to LLVP entry 
  * Convert ingress tag buffer to tpid class object, and convert it to llvp_entry 
  */
shr_error_e
dnx_port_tpid_class_set_ingress_llvp_entry_get(
    int unit,
    dnx_port_tpid_llvp_parser_info_t llvp_parser_info,
    uint32 ingress_tag_buff,
    dnx_port_tpid_ingress_llvp_entry_t *dnx_port_tpid_ingress_llvp_entry);

/**
 * \brief Write single entry to ingress LLVP table
 */
shr_error_e
dnx_port_tpid_class_set_ingress_llvp_entry_to_dbal(
    int unit,
    int llvp_profile,
    dnx_port_tpid_llvp_parser_info_t *llvp_parser_info,
    dnx_port_tpid_ingress_llvp_entry_t *dnx_port_tpid_ingress_llvp_entry);

/** 
  * \brief Write a LLVP block to HW
  */
shr_error_e
dnx_port_tpid_class_set_ingress_llvp_block_write(
    int unit,
    int llvp_profile,
    uint32 *tpid_class_buffer);

/* } */
#endif /*_PORT_TPID_H_INCLUDED_*/
