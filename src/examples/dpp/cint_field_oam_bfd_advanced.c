
/*
 * $id cint_end_tlv.c $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * End TLV workaround:
 * Add a value to the ACE VAR to use the End TLV workaround for OAM CCM packets with
 * port/interface status TLV fields.
 *
 * In order to use port/interface status TLV in received OAM CCM packets, call:
 * > cint utility/cint_utils_field.c
 * > cint cint_field_oam_bfd_advanced.c
 * > cint
 * > field_oam_advanced_features(unit);
 *
 * In order to use BFD flags events when the C-Flag may be set in received PDUs, call:
 * > cint utility/cint_utils_field.c
 * > cint cint_field_oam_bfd_advanced.c
 * > cint
 * > field_bfd_advanced_features(unit,<hw-trap-code>);
 * where <hw-trap-code> is dependent on the traffic type (BFDoIPv4, BFDoPWE, etc.)
 * and can be found in the traps application notes under appendix A.
 *
 */


/*
 * Sets the key to be [OAM-Trap-code, OAMP-dest-port] and sets the value for that key
 * That value is used to identify packets trapped to the OAMP and apply different workarounds if necessary.
 */
int field_oam_advanced_features(int unit)
{
    int result;
    int trap_id;
    bcm_field_group_t grp_tcam = 1;
    int group_priority = 10;
    int qual_id = 1;
    int flags = 0;

    /*Get trap_id value for OAM trap*/
    result = bcm_rx_trap_type_get(unit,flags,bcmRxTrapOamEthAccelerated,&trap_id);
    if (BCM_E_NONE != result) {
        printf("Error in bcm_rx_trap_type_get\n");
        return result;
    }

    /* Create field group for OAM EndTLV WA */
    result = field__prge_action_create_field_group(unit, group_priority, grp_tcam, qual_id);
    if (BCM_E_NONE != result) {
        printf("Error in prge_end_tlv_create_field_group\n");
        return result;
    }

    /* Set action code for the created group */
    printf("Adding entry to mark OAM packets for advances processing\n");
    result = field__prge_action_code_add_entry(unit, grp_tcam, trap_id, 232/*dest_port (OAMP PP port)*/,
                                               qual_id, prge_action_code_oam_end_tlv_wa);
    if (BCM_E_NONE != result) {
        printf("Error in prge_end_tlv_add_entry\n");
        return result;
    }

    return result;
}

/*
 * Sets the key to be [Trap-code, OAMP-dest-port] and sets the value for that key
 * That value is used to identify packets trapped to the OAMP and apply different workarounds if necessary.
 */
int field_bfd_advanced_features(int unit, uint8 trap_code)
{
    int result;
    bcm_field_group_t grp_tcam = 1;
    int group_priority = 10;
    int qual_id = 1;
    char *proc_name  ;

    proc_name = "field_bfd_advanced_features" ;
    printf("%s(): Enter\n",proc_name);

    /* Create field group for OAM EndTLV WA */
    result = field__prge_action_create_field_group(unit, group_priority, grp_tcam, qual_id);
    if (BCM_E_NONE != result) {
        printf("Error in prge_end_tlv_create_field_group\n");
        return result;
    }

    /* Set action code for the created group */
    result = field__prge_action_code_add_entry(unit, grp_tcam, trap_code, 232/*dest_port (OAMP PP port)*/,
                                               qual_id, prge_action_code_bfd_c_bit_clear_wa);
    if (BCM_E_NONE != result) {
        printf("Error in prge_end_tlv_add_entry\n");
        return result;
    }

    printf("%s(): Exit OK\n",proc_name);
    return result;
}
