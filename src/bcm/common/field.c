
/*
 * $Id: field.c,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Field - Broadcom StrataSwitch Field Processor switch common API.
 */
#include <soc/types.h>
#include <soc/ll.h>

#include <bcm/field.h>

/*
 * Function:
 *      bcm_field_udf_spec_t_init
 * Purpose:
 *      Initialize a UDF data type.
 */

void 
bcm_field_udf_spec_t_init(bcm_field_udf_spec_t *udf_spec)
{
    if (NULL != udf_spec) {
        sal_memset(udf_spec, 0, sizeof(bcm_field_udf_spec_t));
    }
    return;
}

/*
 * Function:
 *      bcm_field_llc_header_t_init
 * Purpose:
 *      Initialize the bcm_field_llc_header_t structure.
 * Parameters:
 *      llc_header - Pointer to llc header structure.
 * Returns:
 *      NONE
 */
void
bcm_field_llc_header_t_init(bcm_field_llc_header_t *llc_header)
{
    if (llc_header != NULL) {
        sal_memset(llc_header, 0, sizeof (*llc_header));
    }
    return;
}

/*
 * Function:
 *      bcm_field_snap_header_t_init
 * Purpose:
 *      Initialize the bcm_field_snap_header_t structure.
 * Parameters:
 *      snap_header - Pointer to snap header structure.
 * Returns:
 *      NONE 
 */
void
bcm_field_snap_header_t_init(bcm_field_snap_header_t *snap_header)
{
    if (snap_header != NULL) {
        sal_memset(snap_header, 0, sizeof (*snap_header));
    }
    return;
}

/*
 * Function:
 *      bcm_field_qset_t_init
 * Purpose:
 *      Initialize the bcm_field_qset_t structure.
 * Parameters:
 *      qset - Pointer to field qset structure.
 * Returns:
 *      NONE
 */
void
bcm_field_qset_t_init(bcm_field_qset_t *qset)
{
    if (qset != NULL) {
        sal_memset(qset, 0, sizeof (*qset));
    }
    return;
}

/*
 * Function:
 *      bcm_field_aset_t_init
 * Purpose:
 *      Initialize the bcm_field_aset_t structure.
 * Parameters:
 *      aset - Pointer to field aset structure.
 * Returns:
 *      NONE
 */
void
bcm_field_aset_t_init(bcm_field_aset_t *aset)
{
    if (aset != NULL) {
        sal_memset(aset, 0, sizeof (*aset));
    }
    return;
}

/*
 * Function:
 *      bcm_field_presel_set_t_init
 * Purpose:
 *      Initialize the bcm_field_presel_set_t structure.
 * Parameters:
 *      presel_set - Pointer to field aset structure.
 * Returns:
 *      NONE
 */
void
bcm_field_presel_set_t_init(bcm_field_presel_set_t *presel_set)
{
    if (presel_set != NULL) {
        sal_memset(presel_set, 0, sizeof (*presel_set));
    }
    return;
}

/*
 * Function:
 *      bcm_field_group_status_t_init
 * Purpose:
 *      Initialize the Field Group Status structure.
 * Parameters:
 *      fgroup - Pointer to Field Group Status structure.
 * Returns:
 *      NONE
 */
void
bcm_field_group_status_t_init(bcm_field_group_status_t *fgroup)
{
    if (fgroup != NULL) {
        sal_memset(fgroup, 0, sizeof (*fgroup));
    }
    return;
}

/*
 * Function:
 *      bcm_field_class_info_t_init
 * Purpose:
 *      Initialize the Field Class Info structure.
 * Parameters:
 *      fclass - Pointer to Field Class Info structure.
 * Returns:
 *      NONE
 */
void
bcm_field_class_info_t_init(bcm_field_class_info_t *fclass)
{
    if (fclass != NULL) {
        sal_memset(fclass, 0, sizeof (*fclass));
    }
    return;
}

/*
 * Function:
 *      bcm_field_data_qualifier_t
 * Purpose:
 *      Initialize the Field Data Qualifier structure.
 * Parameters:
 *      data_qual - Pointer to field data qualifier structure.
 * Returns:
 *      NONE
 */
void
bcm_field_data_qualifier_t_init(bcm_field_data_qualifier_t *data_qual)
{
    if (data_qual != NULL) {
        sal_memset(data_qual, 0, sizeof (bcm_field_data_qualifier_t));
    }
    return;
}

/*
 * Function:
 *      bcm_field_data_ethertype_t_init
 * Purpose:
 *      Initialize ethertype based field data qualifier. 
 * Parameters:
 *      etype - Pointer to ethertype based data qualifier structure.
 * Returns:
 *      NONE
 */
void
bcm_field_data_ethertype_t_init (bcm_field_data_ethertype_t *etype)
{
    if (etype != NULL) {
        sal_memset(etype, 0, sizeof (bcm_field_data_ethertype_t));
        etype->l2 = BCM_FIELD_DATA_FORMAT_L2_ANY;
        etype->vlan_tag = BCM_FIELD_DATA_FORMAT_VLAN_TAG_ANY; 
    }
    return;
}

/*
 * Function:
 *      bcm_field_data_ip_protocol_t_init
 * Purpose:
 *      Initialize ip protocol based field data qualifier. 
 * Parameters:
 *      etype - Pointer to ip_protocol based data qualifier structure.
 * Returns:
 *      NONE
 */
void
bcm_field_data_ip_protocol_t_init (bcm_field_data_ip_protocol_t *ip_protocol)
{
    if (ip_protocol != NULL) {
        sal_memset(ip_protocol, 0, sizeof (bcm_field_data_ip_protocol_t));
        ip_protocol->flags = BCM_FIELD_DATA_FORMAT_IP_ANY;
        ip_protocol->ip = 0xFF; /* Reserved  IP Protocol ID */
        ip_protocol->l2 = BCM_FIELD_DATA_FORMAT_L2_ANY;
        ip_protocol->vlan_tag = BCM_FIELD_DATA_FORMAT_VLAN_TAG_ANY; 
    }
    return;
}

/*
 * Function:
 *      bcm_field_data_packet_format_t_init
 * Purpose:
 *      Initialize packet format based field data qualifier. 
 * Parameters:
 *      packet_format - Pointer to packet_format based data qualifier structure.
 * Returns:
 *      NONE
 */
void
bcm_field_data_packet_format_t_init (bcm_field_data_packet_format_t *packet_format)
{

    if (packet_format != NULL) {
        sal_memset(packet_format, 0, sizeof (bcm_field_data_packet_format_t));
        packet_format->l2  = BCM_FIELD_DATA_FORMAT_L2_ANY;
        packet_format->vlan_tag  = BCM_FIELD_DATA_FORMAT_VLAN_TAG_ANY;
        packet_format->outer_ip  = BCM_FIELD_DATA_FORMAT_IP_ANY;
        packet_format->inner_ip  = BCM_FIELD_DATA_FORMAT_IP_ANY;
        packet_format->tunnel  = BCM_FIELD_DATA_FORMAT_TUNNEL_ANY;
        packet_format->mpls  = BCM_FIELD_DATA_FORMAT_MPLS_ANY;
        packet_format->fibre_chan_outer  = BCM_FIELD_DATA_FORMAT_FIBRE_CHAN_ANY;
        packet_format->fibre_chan_inner  = BCM_FIELD_DATA_FORMAT_FIBRE_CHAN_ANY;
        packet_format->flags = 0x0;
    }
    return;
}

/*
 * Function:
 *      bcm_field_group_config_t_init
 * Purpose:
 *      Initialize the Field Group Config structure.
 * Parameters:
 *      data_qual - Pointer to field group config structure.
 * Returns:
 *      NONE
 */
void
bcm_field_group_config_t_init(bcm_field_group_config_t *group_config)
{
    if (group_config != NULL) {
        sal_memset(group_config, 0, sizeof (bcm_field_group_config_t));
        group_config->action_res_id = BCM_FIELD_GROUP_ACTION_RES_ID_DEFAULT;
    }
    return;
}

/*
 * Function:
 *     bcm_field_entry_oper_t_init
 * Purpose:
 *     Initialize a field entry operation structure
 * Parameters:
 *     entry_oper - Pointer to field entry operation structure
 * Returns:
 *     NONE
 */ 
void
bcm_field_entry_oper_t_init(bcm_field_entry_oper_t *entry_oper)
{
    if (entry_oper != NULL) {
        sal_memset(entry_oper, 0, sizeof (bcm_field_entry_oper_t));
    }
    return;
}

/*
 * Function:
 *      bcm_field_extraction_action_t_init
 * Purpose:
 *      Initialize the Field Extraction Action structure.
 * Parameters:
 *      action - Pointer to field extraction action structure.
 * Returns:
 *      NONE
 */
void
bcm_field_extraction_action_t_init(bcm_field_extraction_action_t *action)
{
    if (NULL != action) {
        sal_memset(action, 0x00, sizeof(*action));
    }
}

/*
 * Function:
 *      bcm_field_extraction_field_t_init
 * Purpose:
 *      Initialize the Field Extraction Field structure
 * Parameters:
 *      extraction - pointer to field extraction field structure
 * Returns:
 *      NONE
 */
void
bcm_field_extraction_field_t_init(bcm_field_extraction_field_t *extraction)
{
    if (NULL != extraction) {
        sal_memset(extraction, 0x00, sizeof(*extraction));
    }
}

/* Function: bcm_field_hint_t_init
 *
 * Purpose:
 *     Initialize hint data structure.
 * Parameters:
 *     unit - (IN) BCM device number.
 *     hint - (IN) Pointer to bcm_field_hint_t structure
 * Returns:
 *     None.
 *
 */
void bcm_field_hint_t_init(bcm_field_hint_t *hint)
{
    if (hint != NULL) {
        sal_memset (hint, 0, sizeof (bcm_field_hint_t));
    }
    return;
}

/* Function: bcm_field_oam_stat_action_t_init
 *
 * Purpose:
 *     Initialize oam stat action data structure.
 * Parameters:
 *     unit - (IN) BCM device number.
 *     oam_action - (IN) Pointer to bcm_field_oam_stat_action_t structure

 * Returns:
 *     None.
 *
 */
void bcm_field_oam_stat_action_t_init (bcm_field_oam_stat_action_t *oam_action)
{
    if (oam_action != NULL) {
        sal_memset (oam_action, 0, sizeof (bcm_field_oam_stat_action_t));
    }
    return;
}

/* Function: bcm_field_src_class_t_init
 *
 * Purpose:
 *     Initialize source class mode structure.
 * Parameters:
 *     unit - (IN) BCM device number.
 *     src_class - (IN) Pointer to bcm_field_src_class_t structure

 * Returns:
 *     None.
 *
 */
void bcm_field_src_class_t_init (bcm_field_src_class_t *src_class)
{
    if (src_class != NULL) {
        sal_memset (src_class, 0, sizeof (bcm_field_src_class_t));
    }
    return;
}

/* Function: bcm_field_copytocpu_config_t_init
 *
 * Purpose:
 *     Initialize CopyToCpu config structure.
 * Parameters:
 *     unit - (IN) BCM device number.
 *     CopyToCpu_config - (IN) Pointer to bcm_field_CopyToCpu_config_t structure
 * Returns:
 *     None.
 *
 */
void bcm_field_copytocpu_config_t_init (bcm_field_CopyToCpu_config_t *CopyToCpu_config)
{
    if (CopyToCpu_config != NULL) {
        sal_memset (CopyToCpu_config, 0, sizeof (bcm_field_CopyToCpu_config_t));
        CopyToCpu_config->flags = BCM_FIELD_COPYTOCPU_ALL_PACKET;
    }
    return;
}

/* Function: bcm_field_redirect_config_t_init
 *
 * Purpose:
 *     Initialize redirect config structure.
 * Parameters:
 *     unit - (IN) BCM device number.
 *     redirect_config - (IN) Pointer to bcm_field_redirect_config_t structure
 * Returns:
 *     None.
 *
 */
void bcm_field_redirect_config_t_init (bcm_field_redirect_config_t *redirect_config)
{
    if (redirect_config != NULL) {
        sal_memset (redirect_config, 0, sizeof (bcm_field_redirect_config_t));
        redirect_config->flags = BCM_FIELD_REDIRECT_ALL_PACKET;
        redirect_config->destination_type = bcmFieldRedirectDestinationInvalid;
        redirect_config->destination = BCM_GPORT_INVALID;
        redirect_config->source_port = BCM_GPORT_INVALID;
    }
    return;
}

/*
 * Function:
 *      bcm_field_presel_config_t_init
 * Purpose:
 *      Initialize Field Presel Config structure.
 * Parameters:
 *      presel_config - (INOUT) Presel name.
 * Returns:
 *      None.
 * 
 */
extern void bcm_field_presel_config_t_init(
    bcm_field_presel_config_t *presel_config)
{
    if (presel_config != NULL) {
        sal_memset (presel_config, 0, sizeof (bcm_field_presel_config_t));
    }
    return;
}
