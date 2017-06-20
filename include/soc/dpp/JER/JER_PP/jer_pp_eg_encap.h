
/* $Id: jer_pp_eg_encap.h,v 1.20 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __JER_PP_EG_ENCAP_INCLUDED__
/* { */
#define __JER_PP_EG_ENCAP_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define JER_PP_EG_ENCAP_ROO_LINK_LAYER_SA_LSBS_NOF_BITS                   12
#define JER_PP_EG_ENCAP_ROO_LINK_LAYER_SA_LSBS_MASK                       ((1 << JER_PP_EG_ENCAP_ROO_LINK_LAYER_SA_LSBS_NOF_BITS) -1)

#define JER_PP_EG_ENCAP_ROO_LINK_LAYER_OUTER_TAG_MSB_OUTER_TAG_LSB        0
#define JER_PP_EG_ENCAP_ROO_LINK_LAYER_OUTER_TAG_MSB_PCP_DEI_LSB          8
#define JER_PP_EG_ENCAP_ROO_LINK_LAYER_PCP_DEI_NOF_BITS                   (SOC_SAND_PP_PCP_NOF_BITS + SOC_SAND_PP_CFI_NOF_BITS)
#define JER_PP_EG_ENCAP_ROO_LINK_LAYER_PCP_DEI_MASK                      ((1 << JER_PP_EG_ENCAP_ROO_LINK_LAYER_PCP_DEI_NOF_BITS) - 1)
/* outer vlan: msbs from linker layers, lsbs from 1/4 entry: */
/* nbr of lsbs from outer vlan id for 1/4 entry */
#define JER_PP_EG_ENCAP_ROO_LINK_LAYER_OUTER_TAG_LSB_NOF_BITS             JER_PP_EG_ENCAP_ACCESS_ROO_LL_QUARTER_ENTRY_OUTER_VLAN_LSBS_NOF_BITS
#define JER_PP_EG_ENCAP_ROO_LINK_LAYER_OUTER_TAG_LSB_MASK                 ((1 << JER_PP_EG_ENCAP_ROO_LINK_LAYER_OUTER_TAG_LSB_NOF_BITS) -1)
/* nbr of msbs from outer vlan id for outer tag msbs field */
#define JER_PP_EG_ENCAP_ROO_LINK_LAYER_OUTER_TAG_MSB_NOF_BITS             (SOC_SAND_PP_VID_NOF_BITS - JER_PP_EG_ENCAP_ROO_LINK_LAYER_OUTER_TAG_LSB_NOF_BITS)
#define JER_PP_EG_ENCAP_ROO_LINK_LAYER_OUTER_TAG_MSB_OUTER_TAG_MSB_MASK   ((1 << JER_PP_EG_ENCAP_ROO_LINK_LAYER_OUTER_TAG_MSB_NOF_BITS) -1)


/* get 4 lsbs of vlan. For 1/4 eedb entry */
#define JER_PP_EG_ENCAP_OVERLAY_ARP_DATA_ENTRY_OUT_VID_LSBS(vid, lsbs) (lsbs) = (vid & JER_PP_EG_ENCAP_ROO_LINK_LAYER_OUTER_TAG_LSB_MASK) 

/* lsb for outer tag  */

 /* Outer-tag-MSB:
    in roo ll entry format, this field is composed of: outer-tag-PCP-DEI[11:8]; outer-tag-msbs[7:0] */
#define ROO_LINK_LAYER_ENTRY_OUTER_TAG_MSB_SET(outer_tag_msb, outer_tag, pcp_dei) \
    (outer_tag_msb) = (((pcp_dei & JER_PP_EG_ENCAP_ROO_LINK_LAYER_PCP_DEI_MASK)     \
                             << JER_PP_EG_ENCAP_ROO_LINK_LAYER_OUTER_TAG_MSB_PCP_DEI_LSB)         \
                       | ((outer_tag >> (JER_PP_EG_ENCAP_ROO_LINK_LAYER_OUTER_TAG_LSB_NOF_BITS))           \
                          & JER_PP_EG_ENCAP_ROO_LINK_LAYER_OUTER_TAG_MSB_OUTER_TAG_MSB_MASK))

/* get outer tag msbs from field "outer tag msbs"  */
#define JER_PP_EG_ENCAP_ROO_LINK_LAYER_ENTRY_OUTER_TAG_MSB_OUTER_TAG_GET(outer_tag_msb, outer_tag) \
   SHR_BITCOPY_RANGE(&outer_tag, JER_PP_EG_ENCAP_ROO_LINK_LAYER_OUTER_TAG_LSB_NOF_BITS,                     \
                     &outer_tag_msb, JER_PP_EG_ENCAP_ROO_LINK_LAYER_OUTER_TAG_MSB_OUTER_TAG_LSB,    \
                     JER_PP_EG_ENCAP_ROO_LINK_LAYER_OUTER_TAG_MSB_NOF_BITS) 

/* get pcp dei from field "outer tag msbs" */
#define JER_PP_EG_ENCAP_ROO_LINK_LAYER_ENTRY_OUTER_TAG_MSB_PCP_DEI_GET(outer_tag_msb, pcp_dei) \
   SHR_BITCOPY_RANGE(&pcp_dei, 0,                                                              \
                     &outer_tag_msb, JER_PP_EG_ENCAP_ROO_LINK_LAYER_OUTER_TAG_MSB_PCP_DEI_LSB, \
                     JER_PP_EG_ENCAP_ROO_LINK_LAYER_PCP_DEI_NOF_BITS)


/* This define is temporary, it disables the tunnel new mode and make sure we're working in the old mode for Jericho B0 */
#define SOC_JER_PP_EG_ENCAP_IP_TUNNEL_SIZE_PROTOCOL_TEMPLATE_ENABLE (1)

                     

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

/*********************************************************************
* NAME:
 *   soc_jer_pp_eg_encap_overlay_arp_data_entry_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *    overlay arp eedb entry changes between arad+ and jericho 
 *    Build data info to get overlay arp encap info.
 *    Build eedb entry of type "linker layer" / jericho ROO link layer format:
 *          eedb entry format:
 *            type (3b)  pcp-dei-profile (2b) drop (1b) roo-link-format-identifier (1b) DA (48b)  SA-LSB (12b)
 *            Ether-Type-Index (4b) Number of tags (2b) Remark-Profile (3b) Outer-tag-PCP-DEI (4b) Outer-tag-MSB (8b)
 *    Ether-Type-Index point to table: CfgEtherTypeIndex
 *          Format of CfgEtherTypeIndex entry:
 *          Ethernet-type(16b)  tpid_0(16b) tpid_1(16b)
 *    Additional 1/4 entry for EVE.
 *          1/4 EEDB entry format: LSB's for ROO-Outer_VLAN(16b) PCP-DEI-Inner_Vlan (4b) ROO-Inner-VLAN(16b) 
 * INPUT:
 *   SOC_SAND_IN  int                                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  ll_eep_ndx,
 *      eedb entry index
 *   SOC_SAND_OUT SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO ll_encap_info -
 *      To include egress encapsulation entries.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
soc_jer_pp_eg_encap_overlay_arp_data_entry_add(
     SOC_SAND_IN  int                                    unit,
     SOC_SAND_IN  uint32                                 ll_eep_ndx, 
     SOC_SAND_INOUT  SOC_PPC_EG_ENCAP_OVERLAY_ARP_ENCAP_INFO *ll_encap_info
   ); 

/*********************************************************************
* NAME:
 *   soc_jer_eg_encap_roo_ll_entry_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *    overlay arp eedb entry changes between arad+ and jericho
 *    Parse eedb entry to get overlay encap info.
 *    Parse eedb entry of type "linker layer" / jericho ROO link layer format:
 *          eedb entry format:
 *            type (3b)  pcp-dei-profile (2b) drop (1b) roo-link-format-identifier (1b) DA (48b)  SA-LSB (12b)
 *            Ether-Type-Index (4b) Number of tags (2b) Remark-Profile (3b) Outer-tag-PCP-DEI (4b) Outer-tag-MSB (8b)
 *    Ether-Type-Index point to table: CfgEtherTypeIndex
 *          Format of CfgEtherTypeIndex entry:
 *          Ethernet-type(16b)  tpid_0(16b) tpid_1(16b)
 *    Additional 1/4 entry for EVE.
 *          1/4 EEDB entry format: LSB's for ROO-Outer_VLAN(16b) PCP-DEI-Inner_Vlan (4b) ROO-Inner-VLAN(16b) 
 *
 * Note: Parse only linker layer eedb entry.
 *       CfgEtherTypeIndex parsing is not done here (see soc_jer_eg_encap_ether_type_index_get)
 * INPUT:
 *   SOC_SAND_IN  int                         unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_INFO ll_encap_info -
 *      To include egress encapsulation entries.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/

soc_error_t
soc_jer_eg_encap_roo_ll_entry_get(
     SOC_SAND_IN  int                         unit,
     SOC_SAND_IN  uint32                      eep_ndx, 
     SOC_SAND_OUT SOC_PPC_EG_ENCAP_ENTRY_INFO *encap_info
   ); 






/*********************************************************************
* NAME:
 *   soc_jer_eg_encap_ether_type_index_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *    add entry in ether type index table.
      Build entry.
      Add it in HW.
      Ether type index table is an additional table for jericho roo link layer. 
 * INPUT:
 *   SOC_SAND_IN  int                         unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  int                         eth_type_index -
 *      Index for the ether_type_index table
 *   SOC_SAND_IN  SOC_PPC_EG_ENCAP_ETHER_TYPE_INDEX_INFO *eth_type_index_entry -
 *      eth type index entry.      
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
soc_jer_eg_encap_ether_type_index_set(
   SOC_SAND_IN  int                                        unit, 
   SOC_SAND_IN  int                                        eth_type_index,
   SOC_SAND_IN  SOC_PPC_EG_ENCAP_ETHER_TYPE_INDEX_INFO *eth_type_index_entry
   ); 



/*********************************************************************
* NAME:
 *   soc_jer_eg_encap_ether_type_index_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *    get in ether type index table entry.
 *     Ether type index table is an additional table for jericho roo link layer.
 * 
 * INPUT:
 *   SOC_SAND_IN  int                         unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  int                         eth_type_index -
 *      Index for the ether_type_index table
 *   SOC_SAND_OUT  SOC_PPC_EG_ENCAP_ETHER_TYPE_INDEX_INFO *eth_type_index_entry -
 *      eth type index entry.      
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
soc_jer_eg_encap_ether_type_index_get(
   SOC_SAND_IN  int                                        unit, 
   SOC_SAND_IN  int                                        eth_type_index,
   SOC_SAND_OUT  SOC_PPC_EG_ENCAP_ETHER_TYPE_INDEX_INFO *eth_type_index_entry
   ); 

/*********************************************************************
* NAME:
 *   soc_jer_eg_encap_ether_type_index_clear
 * TYPE:
 *   PROC
 * FUNCTION:
 *    Clear ether type index entry 
 * INPUT:
 *   SOC_SAND_IN  int                         unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  int                         eth_type_index -
 *      Index for the ether_type_index table 
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
soc_jer_eg_encap_ether_type_index_clear(
   SOC_SAND_IN  int unit, 
   SOC_SAND_IN  int eth_type_index); 


/*********************************************************************
* NAME:
 *   soc_jer_eg_encap_direct_bank_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *    Given an EEDB bank id, the bank is set to either mapped lif mode, or direct lif mode.
 * INPUT:
 *      unit        - (IN) Identifier of the device to access.
 *      bank        - (IN) Identifier of the bank to be set.
 *      is_mapped   - (IN)  TRUE: Set the bank to be mapped bank.
                           FALSE: Set the bank to be direct bank.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
soc_jer_eg_encap_direct_bank_set(int unit, int bank, uint8 is_mapped);


/*********************************************************************
* NAME:
 *   soc_jer_eg_encap_extension_mapping_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *    Given an EEDB bank id, the bank is set if extnded, .
 * INPUT:
 *      unit        - (IN) Identifier of the device to access.
 *      bank        - (IN) Identifier of the bank to be set (0-21).
 *      is_extended   - (IN)  TRUE: Set the bank to be extended.
 *                            FALSE: Set the bank to be not extended.
 *      extesnion_bank - (IN)  extension bank id (0-7) 
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
soc_jer_eg_encap_extension_mapping_set(int unit, int bank, uint32 is_extended, uint32 extesnion_bank);

/*********************************************************************
* NAME:
 *   soc_jer_eg_encap_extension_mapping_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *    Given an EEDB bank id, the bank is set if extnded, .
 * INPUT:
 *      unit        - (IN) Identifier of the device to access.
 *      bank        - (IN) Identifier of the bank to be set (0-21).
 *      is_extended   - (OUT)  TRUE: Set the bank to be extended.
 *                            FALSE: Set the bank to be not extended.
 *      extesnion_bank - (OUT)  extension bank id (0-7) 
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
soc_jer_eg_encap_extension_mapping_get(int unit, int bank, uint32 *is_extended, uint32 *extesnion_bank);


soc_error_t
soc_jer_eg_encap_extension_type_set(int unit, int bank, uint8 is_ext_data);

soc_error_t
soc_jer_eg_encap_extension_type_get(int unit, int bank, uint8 *is_ext_data);


soc_error_t
soc_jer_eg_encap_map_encap_intpri_color_set(int unit, int index, SOC_PPC_EG_MAP_ENCAP_INTPRI_COLOR_INFO *entry_info);

soc_error_t
soc_jer_eg_encap_map_encap_intpri_color_get(int unit, int index, SOC_PPC_EG_MAP_ENCAP_INTPRI_COLOR_INFO *entry_info);

soc_error_t 
soc_jer_eg_encap_ip_tunnel_size_protocol_template_set (int unit, int encapsulation_mode, SOC_PPC_EG_ENCAP_IP_TUNNEL_SIZE_PROTOCOL_TEMPLATE_INFO *ip_tunnel_size_protocol_template); 

soc_error_t 
soc_jer_eg_encap_ip_tunnel_size_protocol_template_init(int unit); 

soc_error_t soc_jer_eg_encap_null_value_set(int unit, uint32 *value);

soc_error_t soc_jer_eg_encap_null_value_get(int unit, uint32 *value);

soc_error_t soc_jer_eg_encap_push_2_swap_init(int unit);

/* } */


#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __JER_PP_EG_ENCAP_INCLUDED__*/
#endif

