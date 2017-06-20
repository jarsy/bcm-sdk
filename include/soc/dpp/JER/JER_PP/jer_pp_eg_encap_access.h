
/* $Id: jer_pp_eg_encap_access.h,v 1.20 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __JER_PP_EG_ENCAP_ACCESS_INCLUDED__
/* { */
#define __JER_PP_EG_ENCAP_ACCESS_INCLUDED__


/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

 


/* format of eedb entry (88b):
 * type[87:85] (3b)  pcp-dei-profile[84:83] (2b) drop[82] (1b) roo-link-format-identifier[81] (1b) DA[80:33] (48b) SA-LSB[32:21] (12b)
 *    Ether-Type-Index[20:17] (4b) Number of tags[16:15] (2b) Remark-Profile[14:12] (3b) Outer-tag-MSB[11:0] (12b)
*/
typedef struct 
{
  uint32 pcp_dei_profile;
  uint32 drop; 
  uint32 roo_link_format_identifier; 
  uint32 dest_mac[2];
  uint32 sa_lsb; 
  uint32 ether_type_index; 
  uint32 nof_tags; 
  uint32 remark_profile; 
  uint32 outer_tag_pcp_dei; 
  /* Outer-tag-PCP-DEI[11:8] Outer_tag_msb [7:0] */
  uint32 outer_tag_msb;  
} JER_PP_EG_ENCAP_ACCESS_ROO_LL_ENTRY_FORMAT; 


/* format of ROO Link Layer additional quarter entry (20b):
 * roo-outer-vlan-lsbs[19:16] pcp-dei-inner-vlan[15:12] roo-inner-vlan[11:0] */
typedef struct 
{
    uint32 roo_inner_vlan; 
    uint32 pcp_dei_inner_vlan; 
    uint32 roo_outer_vlan_lsbs; 
} JER_PP_EG_ENCAP_ACCESS_ROO_LL_QUARTER_ENTRY_FORMAT; 

/* inner vlan */
#define JER_PP_EG_ENCAP_ACCESS_ROO_LL_QUARTER_ENTRY_INNER_VLAN_LSB              0
#define JER_PP_EG_ENCAP_ACCESS_ROO_LL_QUARTER_ENTRY_INNER_VLAN_NOF_BITS         SOC_SAND_PP_VID_NOF_BITS
/* pcp dei inner vlan */    
#define JER_PP_EG_ENCAP_ACCESS_ROO_LL_QUARTER_ENTRY_PCP_DEI_INNER_VLAN_LSB      (JER_PP_EG_ENCAP_ACCESS_ROO_LL_QUARTER_ENTRY_INNER_VLAN_LSB + JER_PP_EG_ENCAP_ACCESS_ROO_LL_QUARTER_ENTRY_INNER_VLAN_NOF_BITS)
#define JER_PP_EG_ENCAP_ACCESS_ROO_LL_QUARTER_ENTRY_PCP_DEI_INNER_VLAN_NOF_BITS (SOC_SAND_PP_PCP_NOF_BITS + SOC_SAND_PP_CFI_NOF_BITS)
/* outer vlan lsbs */
#define JER_PP_EG_ENCAP_ACCESS_ROO_LL_QUARTER_ENTRY_OUTER_VLAN_LSBS_LSB         (JER_PP_EG_ENCAP_ACCESS_ROO_LL_QUARTER_ENTRY_PCP_DEI_INNER_VLAN_LSB + JER_PP_EG_ENCAP_ACCESS_ROO_LL_QUARTER_ENTRY_PCP_DEI_INNER_VLAN_NOF_BITS)
#define JER_PP_EG_ENCAP_ACCESS_ROO_LL_QUARTER_ENTRY_OUTER_VLAN_LSBS_NOF_BITS    4



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



soc_error_t
  soc_jer_eg_encap_access_roo_link_layer_format_tbl_set(
    SOC_SAND_IN  int             unit,
    SOC_SAND_IN  uint32              outlif,
    SOC_SAND_IN  JER_PP_EG_ENCAP_ACCESS_ROO_LL_ENTRY_FORMAT  *tbl_data
  );

soc_error_t
  soc_jer_eg_encap_access_roo_link_layer_format_tbl_get(
    SOC_SAND_IN  int             unit,
    SOC_SAND_IN  uint32              outlif,
    SOC_SAND_OUT JER_PP_EG_ENCAP_ACCESS_ROO_LL_ENTRY_FORMAT  *tbl_data
  ); 

 

soc_error_t 
  soc_jer_eg_encap_access_roo_link_layer_quarter_entry_format_tbl_get(
     SOC_SAND_IN  int             unit,
     SOC_SAND_IN  uint32          outlif,
     SOC_SAND_OUT  JER_PP_EG_ENCAP_ACCESS_ROO_LL_QUARTER_ENTRY_FORMAT  *tbl_data
   ); 


soc_error_t 
  jer_pp_eg_encap_access_init_outrif_max(
        SOC_SAND_IN  int             unit
   ); 


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __JER_PP_EG_ENCAP_ACCESS_INCLUDED__*/
#endif

