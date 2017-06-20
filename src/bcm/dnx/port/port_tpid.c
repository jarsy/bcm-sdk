/*! \file port_tpid.c
 * $Id$
 *
 * Port TPID procedures for DNX.
 *
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_PORT
/*
 * Include files.
 * {
 */
/*#include <shared/bslenum.h>*/
#include <shared/shrextend/shrextend_debug.h>
#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/port.h>
#include <bcm_int/dnx/algo/algo_gpm.h>
#include <soc/dnx/dbal/dbal.h>
#include <bcm_int/dnx/port/port_pp.h>
#include <bcm_int/dnx/port/port_tpid.h>
#include <bcm_int/dnx/switch/switch_tpid.h>
/*
 * }
 */

shr_error_e
dnx_port_tpid_class_set_verify(
    int unit,
    bcm_port_tpid_class_t *tpid_class)
{
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Verify that untagged packets are classified only to tag format 0
     */
    if ((tpid_class->tpid1 == BCM_PORT_TPID_CLASS_TPID_INVALID) &&
        (tpid_class->tpid2 == BCM_PORT_TPID_CLASS_TPID_INVALID) &&
        (tpid_class->tag_format_class_id != 0)) {
        /*BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("Untagged packets can be classified only to tag format 0")));*/
    }

    

    /*
     * check tpid_class->tag_format_class_id value range of:
     * tag_format_class_id
     * vlan_translation_action_id
     * vlan_translation_qos_map_id
     */
    if (tpid_class->tag_format_class_id > DNX_PORT_TPID_TAG_FORMAT_MAX_ID)
    {
        /*BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("tag_format_class_id is out of range")));*/
    }

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_port_tpid_class_set_tpid_class_to_buff(
    int unit,
    bcm_port_tpid_class_t *tpid_class,
    uint32 *ingress_buff,
    uint32 *egress_buff)
{
    uint32 temp_value = 0;

    SHR_FUNC_INIT_VARS(unit);

    *ingress_buff = 0;
    *egress_buff = 0;

    /*
     * For ingress, all buffer bits are needed.
     * For egress, only TagFormat and CEP bits are needed.
     */

    /*
     * 18:18 inner-cep
     */
    temp_value = (tpid_class->flags & BCM_PORT_TPID_CLASS_INNER_C)?1:0;
    temp_value = temp_value << 18;
    *ingress_buff |= temp_value;
    *egress_buff |= temp_value;
    /*
     * 17:17 outer-cep
     */
    temp_value = (tpid_class->flags & BCM_PORT_TPID_CLASS_OUTER_C)?1:0;
    temp_value = temp_value << 17;
    *ingress_buff |= temp_value;
    *egress_buff |= temp_value;
    /*
     * 16:16 discard
     */
    temp_value = (tpid_class->flags & BCM_PORT_TPID_CLASS_DISCARD)?1:0;
    *ingress_buff |= temp_value << 16;

    /*
     * 15:11 IncomingTagStructure
     */
    temp_value = tpid_class->tag_format_class_id;
    temp_value = temp_value << 11;
    *ingress_buff |= temp_value;
    *egress_buff |= temp_value;

    /*
     * 10:4 IVEC
     */
    temp_value = tpid_class->vlan_translation_action_id;
    *ingress_buff |= temp_value << 4;

    /*
     * 3:0  PCP-DEI-PROFILE
     */
    temp_value = tpid_class->vlan_translation_qos_map_id;
    *ingress_buff |= temp_value;

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_port_tpid_class_set_buff_to_tpid_class(
    int unit,
    uint32 buff,
    bcm_port_tpid_class_t *tpid_class)
{
    SHR_FUNC_INIT_VARS(unit);

    /*
     * 4 bit PCP-DEI-PROFILE
     */
    tpid_class->vlan_translation_qos_map_id = buff & 0xF;
    buff = buff >> 4;

    /*
     * 7 bit IVEC
     */
    tpid_class->vlan_translation_action_id = buff & 0x7F;
    buff = buff >> 7;

    /*
     * 5 bit IncomingTagStructure
     */
     tpid_class->tag_format_class_id = buff & 0x1F;
     buff = buff >> 5;

    /*
     * 1 bit discard
     */
     tpid_class->flags |= (buff & 0x1)?BCM_PORT_TPID_CLASS_DISCARD:0;
     buff = buff >> 1;

    /*
     * 1 bit outer-cep
     */
    tpid_class->flags |= (buff & 0x1)?BCM_PORT_TPID_CLASS_OUTER_C:0;
    buff = buff >> 1;

    /*
     * 1 bit inner-cep
     */
    tpid_class->flags |= (buff & 0x1)?BCM_PORT_TPID_CLASS_INNER_C:0;

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}


shr_error_e
dnx_port_tpid_class_set_tag_priority_get(
    int unit,
    bcm_port_tpid_class_t *tpid_class,
    uint32 *tag_priority_val,
    uint32 *nof_tag_priority_vals)
{
    SHR_FUNC_INIT_VARS(unit);

    if (tpid_class->flags & BCM_PORT_TPID_CLASS_OUTER_IS_PRIO)
    {
        /*
         * BCM_PORT_TPID_CLASS_OUTER_IS_PRIO - outer tag is a priority tag
         */
        tag_priority_val[0] = 1;
        *nof_tag_priority_vals = 1;
    } 
    else if (tpid_class->flags & BCM_PORT_TPID_CLASS_OUTER_NOT_PRIO)
    {
        /*
         * BCM_PORT_TPID_CLASS_OUTER_NOT_PRIO - outer tag is not a priority tag
         */
        tag_priority_val[0] = 0; 
        *nof_tag_priority_vals = 1;
    }
    else
    {
        /*
         * No notion whether outer tag is priority tag or not.
         * In this case create two entries, for outer tag is priority tag
         * and outer tag is not a priority tag.
         */
        tag_priority_val[0] = 0;
        tag_priority_val[1] = 1;
        *nof_tag_priority_vals = 2;
    }

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_port_tpid_class_set_parser_info_to_llvp_index(
    int unit,
    dnx_port_tpid_llvp_parser_info_t llvp_parser_info,
    int *tag_structure_index)
{
    SHR_FUNC_INIT_VARS(unit);

    *tag_structure_index  = (llvp_parser_info.is_outer_prio & 0x1) << 6;
    *tag_structure_index |= (llvp_parser_info.outer_tpid & 0x7) << 3;
    *tag_structure_index |= (llvp_parser_info.inner_tpid & 0x7);

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_port_tpid_class_set_tpid_class_buffer_update(
    int unit,
    dnx_port_tpid_llvp_parser_info_t llvp_parser_info,
    uint32 tag_buff,
    uint32 *tpid_class_buffer)
{
    int tag_structure_index;
    SHR_FUNC_INIT_VARS(unit);

    /* get LLVP key for a LLVP profile (Reminder: LLVP is divided by LLVP profile) */
    SHR_IF_ERR_EXIT(dnx_port_tpid_class_set_parser_info_to_llvp_index
                    (unit, llvp_parser_info, &tag_structure_index));

    SHR_BITCOPY_RANGE(tpid_class_buffer,
                      DNX_PORT_TPID_BUFFER_BITS_PER_TAG_STRCT * tag_structure_index,
                      &tag_buff,
                      0,
                      DNX_PORT_TPID_BUFFER_BITS_PER_TAG_STRCT);

exit:
    SHR_FUNC_EXIT;
}


shr_error_e
dnx_port_tpid_class_set_ingress_llvp_entry_to_dbal(
    int unit,
    int llvp_profile,
    dnx_port_tpid_llvp_parser_info_t *llvp_parser_info,
    dnx_port_tpid_ingress_llvp_entry_t *dnx_port_tpid_ingress_llvp_entry)
{
    uint32 entry_handle_id;
    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_INGRESS_LLVP_CLASSIFICATION, &entry_handle_id));

    /*
     * Set key fields
     */
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_LLVP_PROFILE, INST_SINGLE, llvp_profile);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_IS_OUTER_PRIO,INST_SINGLE, llvp_parser_info->is_outer_prio);
    /* in case tpid is -1, then */
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_OUTER_TPID,   INST_SINGLE, llvp_parser_info->outer_tpid);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_INNER_TPID,   INST_SINGLE, llvp_parser_info->inner_tpid);

    /*
     * Set data fields
     */
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_INCOMING_VID_EXIST, INST_SINGLE,
                                 dnx_port_tpid_ingress_llvp_entry->incoming_vid_exist);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_INCOMING_TAG_EXIST, INST_SINGLE, 
                                 dnx_port_tpid_ingress_llvp_entry->incoming_tag_exist);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_INCOMING_S_TAG_EXIST, INST_SINGLE, 
                                 dnx_port_tpid_ingress_llvp_entry->incoming_s_tag_exist);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_ACCEPTABLE_FRAME_TYPE_ACTION, INST_SINGLE, 
                                 dnx_port_tpid_ingress_llvp_entry->frame_type_action);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_LLVP_INCOMING_TAG_STRUCTURE, INST_SINGLE, 
                                 dnx_port_tpid_ingress_llvp_entry->llvp_tag_format);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_IVEC_INDEX, INST_SINGLE, 
                                 dnx_port_tpid_ingress_llvp_entry->ivec_index);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_LLVP_PCP_DEI_PROFILE, INST_SINGLE, 
                                 dnx_port_tpid_ingress_llvp_entry->pcp_dei_profile);

    /*
     * write to HW and release handle
     */
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));

exit:
    SHR_FUNC_EXIT;
}

/*
 * Write single entry to LLVP table
 */
shr_error_e
dnx_port_tpid_class_set_egress_llvp_entry_to_dbal(
    int unit,
    int llvp_profile,
    dnx_port_tpid_llvp_parser_info_t *llvp_parser_info,
    dnx_port_tpid_egress_llvp_entry_t *egress_llvp_entry)
{
    uint32 entry_handle_id;
    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_EGRESS_LLVP_CLASSIFICATION, &entry_handle_id));

    /*
     * Set key fields
     */
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_LLVP_PROFILE, INST_SINGLE, llvp_profile);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_IS_OUTER_PRIO,INST_SINGLE, llvp_parser_info->is_outer_prio);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_OUTER_TPID,   INST_SINGLE, llvp_parser_info->outer_tpid);
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_INNER_TPID,   INST_SINGLE, llvp_parser_info->inner_tpid);

    /*
     * Set data fields
     */
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_C_TAG_OFFSET, INST_SINGLE,
                                 egress_llvp_entry->c_tag_offset);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_PACKET_HAS_C_TAG, INST_SINGLE,
                                 egress_llvp_entry->packet_has_c_tag);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_PACKET_HAS_PCP_DEI, INST_SINGLE,
                                 egress_llvp_entry->packet_has_pcp_dei);
    dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_LLVP_INCOMING_TAG_STRUCTURE, INST_SINGLE,
                                 egress_llvp_entry->llvp_tag_format);

    /*
     * write to HW and release handle
     */
    SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));


exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_port_tpid_class_set_ingress_llvp_entry_get(
    int unit,
    dnx_port_tpid_llvp_parser_info_t llvp_parser_info,
    uint32 ingress_tag_buff,
    dnx_port_tpid_ingress_llvp_entry_t *dnx_port_tpid_ingress_llvp_entry)
{
    bcm_port_tpid_class_t tpid_class;

    SHR_FUNC_INIT_VARS(unit);
    bcm_port_tpid_class_t_init(&tpid_class);

    /* 
     * Convert tag buffer to tpid class object 
     */ 
    SHR_IF_ERR_EXIT(dnx_port_tpid_class_set_buff_to_tpid_class
                    (unit, ingress_tag_buff, &tpid_class));

    /*
     * incoming tag exist if tag format is not zero
     */
    dnx_port_tpid_ingress_llvp_entry->incoming_tag_exist = (tpid_class.tag_format_class_id != 0);

    /*
     * vid exist if incoming tag exist and it's not priority
     */
    dnx_port_tpid_ingress_llvp_entry->incoming_vid_exist = 
        (dnx_port_tpid_ingress_llvp_entry->incoming_tag_exist && !llvp_parser_info.is_outer_prio);

    /*
     * S-Tag exist if tagged and not C-Tag
     */
    dnx_port_tpid_ingress_llvp_entry->incoming_s_tag_exist = 
        (dnx_port_tpid_ingress_llvp_entry->incoming_tag_exist && !(tpid_class.flags & BCM_PORT_TPID_CLASS_OUTER_C));

    /*
     * Set Acceptable-Frame-Type-Action
     */
    if (tpid_class.flags & BCM_PORT_TPID_CLASS_DISCARD)
    {
        dnx_port_tpid_ingress_llvp_entry->frame_type_action = DNX_PORT_TPID_TRAP_CODE_ACCEPTABLE_FRAME_TYPE_DROP;
    }
    else
    {
        dnx_port_tpid_ingress_llvp_entry->frame_type_action = DNX_PORT_TPID_TRAP_CODE_ACCEPTABLE_FRAME_TYPE_ACCEPT;
    }

    dnx_port_tpid_ingress_llvp_entry->llvp_tag_format = tpid_class.tag_format_class_id;
    dnx_port_tpid_ingress_llvp_entry->ivec_index = tpid_class.vlan_translation_action_id;
    dnx_port_tpid_ingress_llvp_entry->pcp_dei_profile = tpid_class.vlan_translation_qos_map_id;

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_port_tpid_class_set_egress_llvp_entry_get(
    int unit,
    uint32 egress_tag_buff,
    dnx_port_tpid_egress_llvp_entry_t *dnx_port_tpid_egress_llvp_entry)
{
    int is_outer_c_tag;
    int is_inner_c_tag;
    int is_outer_s_tag;
    bcm_port_tpid_class_t tpid_class;


    SHR_FUNC_INIT_VARS(unit);
    bcm_port_tpid_class_t_init(&tpid_class);

    SHR_IF_ERR_EXIT(dnx_port_tpid_class_set_buff_to_tpid_class
                    (unit, egress_tag_buff, &tpid_class));

    /*
     * c-tag according to user indication
     */
    is_outer_c_tag = (tpid_class.flags & BCM_PORT_TPID_CLASS_OUTER_C) ? 1 : 0;
    is_inner_c_tag = (tpid_class.flags & BCM_PORT_TPID_CLASS_INNER_C) ? 1 : 0;
    /*
     * s-tag, if tagged and not ctag
     */
    is_outer_s_tag = (tpid_class.tag_format_class_id != 0) && (!is_outer_c_tag);

    
    dnx_port_tpid_egress_llvp_entry->c_tag_offset = 0;
    dnx_port_tpid_egress_llvp_entry->packet_has_pcp_dei = 0;
    dnx_port_tpid_egress_llvp_entry->llvp_tag_format = 0;
    dnx_port_tpid_egress_llvp_entry->packet_has_c_tag = 0;

    if (is_outer_c_tag)
    {
        dnx_port_tpid_egress_llvp_entry->c_tag_offset = 0;
        /* is outer C-tag */
        dnx_port_tpid_egress_llvp_entry->packet_has_c_tag = TRUE;
    }
    else if (is_inner_c_tag)
    {
        dnx_port_tpid_egress_llvp_entry->c_tag_offset = 1;
        /* is outer C-tag */
        dnx_port_tpid_egress_llvp_entry->packet_has_c_tag = TRUE;
    }

    /* if outer is S-Tag then has PCP-DEI */
    if (is_outer_s_tag)
    {
        dnx_port_tpid_egress_llvp_entry->packet_has_pcp_dei = TRUE;
    }

    dnx_port_tpid_egress_llvp_entry->llvp_tag_format = tpid_class.tag_format_class_id;

exit:
    SHR_FUNC_EXIT;
}


shr_error_e
dnx_port_tpid_class_set_egress_llvp_entry_write(
    int unit,
    int llvp_profile,
    dnx_port_tpid_llvp_parser_info_t llvp_parser_info,
    uint32 egress_tag_buff)
{
    dnx_port_tpid_egress_llvp_entry_t llvp_entry;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Convert buffer and parser info to egress LLVP entry 
     */
    SHR_IF_ERR_EXIT(dnx_port_tpid_class_set_egress_llvp_entry_get
                    (unit, egress_tag_buff, &llvp_entry));

    /*
     * Write the entry to HW 
     */
    SHR_IF_ERR_EXIT(dnx_port_tpid_class_set_egress_llvp_entry_to_dbal
                    (unit, llvp_profile, &llvp_parser_info, &llvp_entry));
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_port_tpid_class_set_ingress_llvp_entry_write(
    int unit,
    int llvp_profile,
    dnx_port_tpid_llvp_parser_info_t llvp_parser_info,
    uint32 ingress_tag_buff)
{
    dnx_port_tpid_ingress_llvp_entry_t llvp_entry;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Convert buffer and parser info to ingress LLVP entry 
     */
    SHR_IF_ERR_EXIT(dnx_port_tpid_class_set_ingress_llvp_entry_get
        (unit, llvp_parser_info, ingress_tag_buff, &llvp_entry));

    /*
     * Write the entry to HW
     */
    SHR_IF_ERR_EXIT(dnx_port_tpid_class_set_ingress_llvp_entry_to_dbal
                    (unit, llvp_profile, &llvp_parser_info, &llvp_entry));
exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_port_tpid_class_set_ingress_llvp_block_write(
    int unit,
    int llvp_profile,
    uint32 *tpid_class_buffer)
{
    int idx;
    uint32 entry_handle_id = DNX_PORT_TPID_LLVP_BLOCK_SIZE;
    uint32 temp_buffer_entry;
    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_INGRESS_LLVP_CLASSIFICATION, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_LLVP_PROFILE, INST_SINGLE, llvp_profile);

    /*
     * Go over all entries
     */
    for(idx = 0; idx < DNX_PORT_TPID_LLVP_BLOCK_SIZE; idx++)
    {
        int temp;
        dnx_port_tpid_llvp_parser_info_t llvp_parser_info;
        dnx_port_tpid_ingress_llvp_entry_t llvp_entry;
        temp = idx;
        llvp_parser_info.inner_tpid = temp * 0x7;
        temp = temp >> 3;

        llvp_parser_info.outer_tpid = temp * 0x7;
        temp = temp >> 3;

        llvp_parser_info.is_outer_prio = temp * 0x1;

        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_IS_OUTER_PRIO, INST_SINGLE, llvp_parser_info.is_outer_prio);
        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_OUTER_TPID,    INST_SINGLE, llvp_parser_info.outer_tpid);
        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_INNER_TPID,    INST_SINGLE, llvp_parser_info.inner_tpid);

        SHR_BITCOPY_RANGE(&temp_buffer_entry,
                          0,
                          tpid_class_buffer,
                          DNX_PORT_TPID_BUFFER_BITS_PER_TAG_STRCT * idx,
                          DNX_PORT_TPID_BUFFER_BITS_PER_TAG_STRCT);

        SHR_IF_ERR_EXIT(dnx_port_tpid_class_set_ingress_llvp_entry_get
                        (unit, llvp_parser_info, temp_buffer_entry, &llvp_entry));

        /*
         * Set data fields
         */
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_INCOMING_VID_EXIST, INST_SINGLE, 
                                     llvp_entry.incoming_vid_exist);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_INCOMING_TAG_EXIST, INST_SINGLE, 
                                     llvp_entry.incoming_tag_exist);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_INCOMING_S_TAG_EXIST, INST_SINGLE, 
                                     llvp_entry.incoming_s_tag_exist);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_ACCEPTABLE_FRAME_TYPE_ACTION, INST_SINGLE, 
                                     llvp_entry.frame_type_action);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_LLVP_INCOMING_TAG_STRUCTURE, INST_SINGLE, 
                                     llvp_entry.llvp_tag_format);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_IVEC_INDEX, INST_SINGLE, 
                                     llvp_entry.ivec_index);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_LLVP_PCP_DEI_PROFILE, INST_SINGLE, 
                                     llvp_entry.pcp_dei_profile);

        /*
         * write to HW and don't release handle
         */
        SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_KEEP_HANDLE));
    }

exit:
    dbal_entry_handle_release(unit, entry_handle_id);
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_port_tpid_class_set_egress_llvp_block_write(
    int unit,
    int llvp_profile,
    uint32 *tpid_class_buffer)
{
    int idx;
    int temp;
    uint32 entry_handle_id = 128;
    uint32 temp_buffer_entry;
    dnx_port_tpid_llvp_parser_info_t llvp_parser_info;
    dnx_port_tpid_egress_llvp_entry_t llvp_entry;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Go over all entries
     */
    SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_EGRESS_LLVP_CLASSIFICATION, &entry_handle_id));
    dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_LLVP_PROFILE, INST_SINGLE, llvp_profile);

    for(idx = 0; idx < DNX_PORT_TPID_LLVP_BLOCK_SIZE; idx++)
    {
        temp = idx;
        llvp_parser_info.inner_tpid = temp * 0x7;
        temp = temp >> 3;

        llvp_parser_info.outer_tpid = temp * 0x7;
        temp = temp >> 3;

        llvp_parser_info.is_outer_prio = temp * 0x1;

        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_IS_OUTER_PRIO, INST_SINGLE, llvp_parser_info.is_outer_prio);
        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_OUTER_TPID, INST_SINGLE, llvp_parser_info.outer_tpid);
        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_INNER_TPID, INST_SINGLE, llvp_parser_info.inner_tpid);

        SHR_BITCOPY_RANGE(&temp_buffer_entry,
                          0,
                          tpid_class_buffer,
                          DNX_PORT_TPID_BUFFER_BITS_PER_TAG_STRCT * idx,
                          DNX_PORT_TPID_BUFFER_BITS_PER_TAG_STRCT);

        SHR_IF_ERR_EXIT(dnx_port_tpid_class_set_egress_llvp_entry_get
                        (unit, temp_buffer_entry, &llvp_entry));

        /*
         * Set data fields
         */
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_C_TAG_OFFSET, INST_SINGLE, 
                                     llvp_entry.c_tag_offset);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_PACKET_HAS_C_TAG, INST_SINGLE, 
                                     llvp_entry.packet_has_c_tag);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_PACKET_HAS_PCP_DEI, INST_SINGLE, 
                                     llvp_entry.packet_has_pcp_dei);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_LLVP_INCOMING_TAG_STRUCTURE, INST_SINGLE, 
                                     llvp_entry.llvp_tag_format);

        /*
         * write to HW and don't release handle
         */
        SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_KEEP_HANDLE));
    }

exit:
    dbal_entry_handle_release(unit, entry_handle_id);
    SHR_FUNC_EXIT;
}


/** 
 *  \brief
 *  Resolve tpid value to global tpid index.
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit.
 *    \param [in] tpid_val -
 *     tpid value. 
 * \par INDIRECT INPUT
 *     None.
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *  \param tpid_indexes -
 *    global tpid index, 3b
 *    when tpid_val has special value "invalid" then tpid index has a special value: untag
 *  \param nof_tpid_indexes -
 *    number of tpid index found.   
 *  when tpid_val has special value "any" then number of tpid index found will be all the
 *  global tpid index. 
 *  otherwise 1 if tpid index found and 0 if not found
 */ 
shr_error_e
dnx_port_tpid_class_set_tpid_indexes_get(int unit, 
                    uint32 tpid_val, 
                    int tpid_indexes[BCM_DNX_SWITCH_TPID_NUM_OF_VALID_GLOBALS],
                    int *nof_tpid_indexes)
{
    uint32 tpid_indx=0;

    SHR_FUNC_INIT_VARS(unit);

    *nof_tpid_indexes = 0;

    /* tpid val has special value invalid
       We make sure it's 3b length */
    if (tpid_val == BCM_PORT_TPID_CLASS_TPID_INVALID) {
        tpid_indexes[tpid_indx++] = BCM_DNX_SWITCH_TPID_INDEX_INVALID & 0x7;
        *nof_tpid_indexes = tpid_indx;
        SHR_EXIT();
    }

    

    /* search for the tpid value in global tpids and return the index */
    SHR_IF_ERR_EXIT(dnx_switch_tpid_index_get(unit, tpid_val, &(tpid_indexes[0])));
    /* update nof tpids */
    /* tpid index not found */
    if (tpid_indexes[0] == BCM_DNX_SWITCH_TPID_INDEX_INVALID) 
    {
        nof_tpid_indexes = 0;
    } 
    /* tpid index found */
    else 
    {
        *nof_tpid_indexes = 1;
    }

exit:
    SHR_FUNC_EXIT;
}


shr_error_e
bcm_dnx_port_tpid_class_get(
    int unit,
    bcm_port_tpid_class_t *tpid_class)
{
    return -1;
}

shr_error_e
bcm_dnx_port_tpid_class_set(
    int unit,
    bcm_port_tpid_class_t *tpid_class)
{
    int idx1;
    int idx2;
    int idx3;
    dnx_algo_gpm_gport_phy_info_t gport_info;

    /* key of the llvp table: tpid indexes, tag priority, llvp profile */
    int nof_port_tpid_indexes[2];
    int port_tpid_indexes[2][7];
    uint32 tag_priority_vals[2];
    uint32 nof_tag_priority_vals;
    int old_llvp_profile = 0;
    int new_llvp_profile = 0;

    /* compressed ingress/egress llvp entry information */
    uint32 ingress_tag_buff;
    uint32 egress_tag_buff;

    /* compressed ingress/egress llvp table per llvp profile. Used by template manager. */
    uint32 tpid_class_buffer[DNX_PORT_TPID_CLASS_TEMPLATE_SIZE_NUM_OF_UINT32];


    uint8 is_ingress, is_egress;

    SHR_FUNC_INIT_VARS(unit);

    sal_memset(nof_port_tpid_indexes, 0x0, sizeof(nof_port_tpid_indexes)); 
    sal_memset(port_tpid_indexes, 0x0, sizeof(port_tpid_indexes)); 

    /*
     * Verify user input
     */
    SHR_INVOKE_VERIFY_DNX(dnx_port_tpid_class_set_verify(unit, tpid_class));

    is_ingress = _SHR_IS_FLAG_SET(tpid_class->flags, BCM_PORT_TPID_CLASS_EGRESS_ONLY) ? FALSE : TRUE;
    is_egress =  _SHR_IS_FLAG_SET(tpid_class->flags, BCM_PORT_TPID_CLASS_INGRESS_ONLY) ? FALSE : TRUE;


    /*
     * map gport to ppd-port
     */
    SHR_IF_ERR_EXIT(dnx_algo_gpm_gport_phy_info_get
                    (unit, tpid_class->port, DNX_ALGO_GPM_GPORT_TO_PHY_OP_LOCAL_IS_MANDATORY, &gport_info));

    /*
     * calculate TPID indexes for given two TPIDs
     */
    SHR_IF_ERR_EXIT(dnx_port_tpid_class_set_tpid_indexes_get
                    (unit, tpid_class->tpid1, port_tpid_indexes[0], &(nof_port_tpid_indexes[0])));
    SHR_IF_ERR_EXIT(dnx_port_tpid_class_set_tpid_indexes_get
                    (unit, tpid_class->tpid2, port_tpid_indexes[1], &(nof_port_tpid_indexes[1])));

    
    SHR_IF_ERR_EXIT(dnx_port_tpid_class_set_tpid_class_to_buff
                    (unit, tpid_class, &ingress_tag_buff , &egress_tag_buff));

    /*
     * Calculate outer tag is priority tag LLVP key value
     * possible cases: 1. outer tag is priority tag.
     *                 2. outer tag is not priority tag.
     *                 3. don't care - create 1 and 2.
     */
    SHR_IF_ERR_EXIT(dnx_port_tpid_class_set_tag_priority_get
                    (unit, tpid_class, tag_priority_vals, &nof_tag_priority_vals));

    /*
     * Update ingress LLVP table
     */
    if (is_ingress)
    {
        
        sal_memset(tpid_class_buffer, 0, sizeof(tpid_class_buffer));

        /*
         * Update template with new LLVP fields 
         */
        for (idx1 = 0; idx1 < nof_port_tpid_indexes[0]; idx1++)
        {
            for (idx2 = 0; idx2 < nof_port_tpid_indexes[1]; idx2++)
            {
                for (idx3 = 0; idx3 < nof_tag_priority_vals; idx3++)
                {
                    dnx_port_tpid_llvp_parser_info_t llvp_parser_info;
                    llvp_parser_info.outer_tpid = port_tpid_indexes[0][idx1];
                    llvp_parser_info.inner_tpid = port_tpid_indexes[1][idx2];
                    llvp_parser_info.is_outer_prio = tag_priority_vals[idx3];

                    /*
                     * Update the tpid class buffer with tag buffer
                     */
                    SHR_IF_ERR_EXIT(dnx_port_tpid_class_set_tpid_class_buffer_update
                                    (unit, llvp_parser_info, ingress_tag_buff, tpid_class_buffer));
                }
            }
        }

        

        
        if (TRUE)
        {
            
            if (old_llvp_profile == new_llvp_profile)
            {
                /*
                 * if new template equal to old template we only need to update the
                 * new LLVP entries
                 */
                for (idx1 = 0; idx1 < nof_port_tpid_indexes[0]; idx1++)
                {
                    for (idx2 = 0; idx2 < nof_port_tpid_indexes[1]; idx2++)
                    {
                        for (idx3 = 0; idx3 < nof_tag_priority_vals; idx3++)
                        {
                            dnx_port_tpid_llvp_parser_info_t llvp_parser_info;
                            llvp_parser_info.outer_tpid = port_tpid_indexes[0][idx1];
                            llvp_parser_info.inner_tpid = port_tpid_indexes[1][idx2];
                            llvp_parser_info.is_outer_prio = tag_priority_vals[idx3];

                            /*
                             * Write to HW
                             */
                            SHR_IF_ERR_EXIT(dnx_port_tpid_class_set_ingress_llvp_entry_write
                                    (unit, new_llvp_profile, llvp_parser_info, ingress_tag_buff));
                        }
                    }
                }
            }
            else
            {
                /*
                 * a new template was created, we copy all buffer to the new locations
                 */
                SHR_IF_ERR_EXIT(dnx_port_tpid_class_set_ingress_llvp_block_write
                                (unit, new_llvp_profile, tpid_class_buffer));
            }
        }

        
    }

    /*
     * Update egress LLVP table
     */
    if (is_egress)
    {
        
        sal_memset(tpid_class_buffer, 0, sizeof(tpid_class_buffer));

        /*
         * Update template with new LLVP fields
         */
        for (idx1 = 0; idx1 < nof_port_tpid_indexes[0]; idx1++)
        {
            for (idx2 = 0; idx2 < nof_port_tpid_indexes[1]; idx2++)
            {
                for (idx3 = 0; idx3 < nof_tag_priority_vals; idx3++)
                {
                    dnx_port_tpid_llvp_parser_info_t llvp_parser_info;
                    llvp_parser_info.outer_tpid = port_tpid_indexes[0][idx1];
                    llvp_parser_info.inner_tpid = port_tpid_indexes[1][idx2];
                    llvp_parser_info.is_outer_prio = tag_priority_vals[idx3];

                    /*
                     * Update the buffer
                     */
                    SHR_IF_ERR_EXIT(dnx_port_tpid_class_set_tpid_class_buffer_update
                                    (unit, llvp_parser_info, egress_tag_buff, tpid_class_buffer));
                }
            }
        }

        

        
        if (TRUE)
        {
            
            if (old_llvp_profile == new_llvp_profile)
            {
                /*
                 * if new template equal to old template we only need to update the
                 * new LLVP entries
                 */
                for (idx1 = 0; idx1 < nof_port_tpid_indexes[0]; idx1++)
                {
                    for (idx2 = 0; idx2 < nof_port_tpid_indexes[1]; idx2++)
                    {
                        for (idx3 = 0; idx3 < nof_tag_priority_vals; idx3++)
                        {
                            dnx_port_tpid_llvp_parser_info_t llvp_parser_info;
                            llvp_parser_info.outer_tpid = port_tpid_indexes[0][idx1];
                            llvp_parser_info.inner_tpid = port_tpid_indexes[1][idx2];
                            llvp_parser_info.is_outer_prio = tag_priority_vals[idx3];

                            /*
                             * Write to HW
                             */
                            SHR_IF_ERR_EXIT(dnx_port_tpid_class_set_egress_llvp_entry_write
                                            (unit, new_llvp_profile, llvp_parser_info, egress_tag_buff));
                        }
                    }
                }
            }
            else
            {
                /*
                 * a new template was created, we copy all buffer to the new locations
                 */
                SHR_IF_ERR_EXIT(dnx_port_tpid_class_set_egress_llvp_block_write
                                (unit, new_llvp_profile, tpid_class_buffer));
            }
        }

        
    }

exit:
    SHR_FUNC_EXIT;
}
