/** \file vlan_translate.c
 *  
 *  VLAN port translate procedures for DNX. 
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#  error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_VLAN
/*
 * Include files.
 * {
 */
#include <soc/dnx/dbal/dbal.h>
#include <bcm/types.h>
#include <bcm/vlan.h>
#include <shared/shrextend/shrextend_debug.h>
#include <bcm_int/dnx/port/port_pp.h>
#include <bcm_int/dnx/algo/algo_gpm.h>
#include <bcm_int/dnx/vlan/vlan.h>
#include <bcm_int/dnx/switch/switch_tpid.h>
#include <soc/dnx/dnx_data/dnx_data_l2.h>
#include <shared/util.h>
#include <bcm/error.h>
#include <bcm_int/dnx/algo/res_mngr/res_mngr_api.h>

/*
 * }
 */

/*
 * MACROs
 * {
 */

/*
 * }
 */

/*
 * Conversions from BCM API to HW DBAL
 */


/*!
 * \brief VLAN Translation VID action information in BCM API format
 *
 * This structure contains VID actioninformation in a BCM API format.
 */
typedef struct
{
    /*
     * !
     * * VID action for an Outer Tag
     */ 
    bcm_vlan_action_t outer_tag_action;
    /*
     * !
     * * VID action for Inner Tag
     */
    bcm_vlan_action_t inner_tag_action;
} dnx_vlan_translate_vid_action_bcm_format_t;

/*!
 * \brief VLAN Translation enumeration for VID Sources.
 *
 * VLAN Translation enumeration for VID Sources. This enumeration is later on mapped to the DBAL
 * enumeration for IVE or EVE.
 */
typedef enum
{
     FIRST_DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_E = 0,
     /**
      *  No VLAN Edit action for the tag
      */
     DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY = FIRST_DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_E,
     /**
      *  VID Source is the AC LIF Outer VID
      */
     DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_OUTER_VID,     
     /**
      *  VID Source is the AC LIF Inner VID
      */
     DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_INNER_VID,
    /**
      *  VID Source is the Incoming packet Outer VID
      */
     DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID,
    /**
      *  VID Source is the Incoming packet Inner VID
      */
     DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID,
     /**
      *  VID Source is the Forward domain (VSI)
      */
     DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN,

     NUM_DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_E
} dnx_vlan_translate_vid_source_type_e;


/*!
 * \brief VLAN Translation enumeration for PCP-DEI Sources.
 *
 * VLAN Translation enumeration for PCP-DEI Sources. This enumeration is later on mapped to the DBAL
 * enumeration for IVE or EVE.
 */
typedef enum
{
     FIRST_DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_E = 0,
     /**
      *  PCP-DEI Source is the AC LIF Outer PCP-DEI mapping
      */
     DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_AC_OUTER_PCP_DEI = FIRST_DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_E,    
     /**
      *  PCP_DEI Source is the AC LIF Inner PCP-DEI mapping
      */
     DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_AC_INNER_PCP_DEI,
     /**
      *  PCP_DEI Source is the Incoming packet Outer PCP-DEI mapping
      */
     DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_INCOMING_OUTER_PCP_DEI,
     /**
      *  PCP_DEI Source is the Incoming packet Inner PCP-DEI mapping
      */
     DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_INCOMING_INNER_PCP_DEI,

     NUM_DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_E
} dnx_vlan_translate_pcp_dei_source_type_e;


/*!
 * \brief VLAN Translation VID Action information in DNX HW format
 *
 * This structure contains VID action information in a DNX HW format.
 */
typedef struct
{
    /*
     * !
     * * Number of VLAN tags to remove during this VLAN Translation or -1 if invalid
     */
    int nof_tags_to_remove;
    /*
     * !
     * * VID Source for added Outer Tag
     */ 
    dnx_vlan_translate_vid_source_type_e vid_source_outer;
    /*
     * !
     * * VID Source for added Inner Tag
     */
    dnx_vlan_translate_vid_source_type_e vid_source_inner;
} dnx_vlan_translate_vid_action_dnx_format_t;


/*!
 * \brief VLAN Translation information in DNX HW format
 *
 * This structure contains VLAN Translation command information in a DNX HW format.
 * It stores the number of tags to remove and the sources from which to build up to two new tags: Outer & inner.
 */
typedef struct
{
    /*
     * !
     * * Number of VLAN tags to remove during this VLAN Translation or -1 if invalid
     */
    int nof_tags_to_remove;
    /*
     * !
     * * VID Source for added Outer Tag
     */ 
    dnx_vlan_translate_vid_source_type_e vid_source_outer;
    /*
     * !
     * * VID Source for added Inner Tag
     */
    dnx_vlan_translate_vid_source_type_e vid_source_inner;
    /*
     * !
     * * PCP-DEI Source for added Outer Tag
     */
    dnx_vlan_translate_pcp_dei_source_type_e pcp_dei_source_outer;
    /*
     * !
     * * PCP-DEI Source for added Inner Tag
     */
    dnx_vlan_translate_pcp_dei_source_type_e pcp_dei_source_inner;
    /*
     * !
     * * Global TPID index for added Outer Tag
     */
    int outer_tpid_idx;
    /*
     * !
     * * Global TPID index for added Inner Tag
     */
    int inner_tpid_idx;
} dnx_vlan_translate_action_dnx_format_t;


/*!
 * \brief VLAN Translation conversion information.
 *
 * This structure contains information that is required in order to convert a VLAN Translation command from
 * BCM API format to DNX HW format.
 *  \see 
 * dnx_xxxxxx
 */
typedef struct
{
    /*
     * !
     * * VLAN Translation action in BCM API format. Comprised of an action for each of the supported tags - 
     *   Outer & Inner.
     */
    dnx_vlan_translate_vid_action_bcm_format_t vlan_action_bcm;
    /*
     * !
     * * VLAN Translation action in DNX HW format. Comprised of the number of tags to remove and Sources from which to
     *   build the content of each of the tags - Outer & Inner.
     */ 
    dnx_vlan_translate_vid_action_dnx_format_t vlan_action_dnx;
    /*
     * !
     * * Flag that determines if the BCM API format is valid for Ingress conversion.
     */
    uint8 is_valid_ingress_conversion;
    /*
     * !
     * * Flag that determines if the BCM API format is valid for Egress conversion.
     */
    uint8 is_valid_egress_conversion;
} dnx_vlan_translate_vid_action_bcm_to_dnx_conversion_t;

/*
 * bcm-outer-vid, bcm-inner-vid, 
 *      tags-to-remove, outer-vid-src, inner-vid-src, ingress-valid-conversion,egress-valid-conversion 
 */ 
static dnx_vlan_translate_vid_action_bcm_to_dnx_conversion_t dnx_vlan_translation_conversion_info[] = {
{{bcmVlanActionNone, bcmVlanActionNone},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 1, 1},
{{bcmVlanActionReplace, bcmVlanActionNone},
    {1, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 1, 1},
{{bcmVlanActionAdd, bcmVlanActionNone},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 1, 1},
{{bcmVlanActionDelete, bcmVlanActionNone},
    {1, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 1, 1},
{{bcmVlanActionCopy, bcmVlanActionNone},
    {1, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 1, 1},
{{bcmVlanActionMappedReplace, bcmVlanActionNone},
    {1, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 0, 1},
{{bcmVlanActionMappedAdd, bcmVlanActionNone},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 0, 1},
{{bcmVlanActionOuterAdd, bcmVlanActionNone},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID ,DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 1, 1},
{{bcmVlanActionInnerAdd, bcmVlanActionNone},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID ,DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 1, 1},
{{bcmVlanActionNone, bcmVlanActionReplace},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_INNER_VID}, 1, 1},
{{bcmVlanActionReplace, bcmVlanActionReplace},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_INNER_VID}, 1, 1},
{{bcmVlanActionAdd, bcmVlanActionReplace},
    {-1, 0, 0}, 0, 0},
{{bcmVlanActionDelete, bcmVlanActionReplace},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 1, 1},
{{bcmVlanActionCopy, bcmVlanActionReplace},
    {2 ,DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_INNER_VID}, 1, 1},
{{bcmVlanActionMappedReplace, bcmVlanActionReplace},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_INNER_VID}, 0, 1},
{{bcmVlanActionMappedAdd, bcmVlanActionReplace},
    {-1, 0, 0}, 0, 0},
{{bcmVlanActionOuterAdd, bcmVlanActionReplace},
    {-1, 0, 0}, 0, 0},
{{bcmVlanActionInnerAdd, bcmVlanActionReplace},
    {-1, 0, 0}, 0, 0},
{{bcmVlanActionNone, bcmVlanActionAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 1, 1},
{{bcmVlanActionReplace, bcmVlanActionAdd},
    {1, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_INNER_VID}, 1, 1},
{{bcmVlanActionAdd, bcmVlanActionAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_INNER_VID}, 1, 1},
{{bcmVlanActionDelete, bcmVlanActionAdd},
    {1, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 1, 1},
{{bcmVlanActionCopy, bcmVlanActionAdd},
    {1, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID}, 1, 1},
{{bcmVlanActionMappedReplace, bcmVlanActionAdd},
    {1, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN}, 0, 1},
{{bcmVlanActionMappedAdd, bcmVlanActionAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_INNER_VID}, 0, 1},
{{bcmVlanActionOuterAdd, bcmVlanActionAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_INNER_VID}, 1, 1},
{{bcmVlanActionInnerAdd, bcmVlanActionAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_INNER_VID}, 1, 1},
{{bcmVlanActionNone, bcmVlanActionDelete},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 1, 1},
{{bcmVlanActionReplace, bcmVlanActionDelete},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 1, 1},
{{bcmVlanActionAdd, bcmVlanActionDelete},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID}, 1, 1},
{{bcmVlanActionDelete, bcmVlanActionDelete},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 1, 1},
{{bcmVlanActionCopy, bcmVlanActionDelete},
    {1, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 1, 1},
{{bcmVlanActionMappedReplace, bcmVlanActionDelete},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 0, 1},
{{bcmVlanActionMappedAdd, bcmVlanActionDelete},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID}, 0, 1},
{{bcmVlanActionOuterAdd, bcmVlanActionDelete},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID ,DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID}, 1, 1},
{{bcmVlanActionInnerAdd, bcmVlanActionDelete},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID ,DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID}, 1, 1},
{{bcmVlanActionNone, bcmVlanActionCopy},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID}, 1, 1},
{{bcmVlanActionReplace, bcmVlanActionCopy},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID}, 1, 1},
{{bcmVlanActionAdd, bcmVlanActionCopy},
    {-1, 0, 0}, 0, 0},
{{bcmVlanActionDelete, bcmVlanActionCopy},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 1, 1},
{{bcmVlanActionCopy, bcmVlanActionCopy},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID}, 1, 1},
{{bcmVlanActionMappedReplace, bcmVlanActionCopy},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID}, 0, 1},
{{bcmVlanActionMappedAdd, bcmVlanActionCopy},
    {-1, 0, 0}, 0, 0},
{{bcmVlanActionOuterAdd, bcmVlanActionCopy},
    {-1, 0, 0}, 0, 0},
{{bcmVlanActionInnerAdd, bcmVlanActionCopy},
    {-1, 0, 0}, 0, 0},
{{bcmVlanActionNone, bcmVlanActionMappedReplace},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN}, 0, 1},
{{bcmVlanActionReplace, bcmVlanActionMappedReplace},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN}, 0, 1},
{{bcmVlanActionAdd, bcmVlanActionMappedReplace},
    {-1, 0, 0}, 0, 0},
{{bcmVlanActionDelete, bcmVlanActionMappedReplace},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 0, 1},
{{bcmVlanActionCopy, bcmVlanActionMappedReplace},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN}, 0, 1},
{{bcmVlanActionMappedReplace, bcmVlanActionMappedReplace},
    {2, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN}, 0, 1},
{{bcmVlanActionMappedAdd, bcmVlanActionMappedReplace},
    {-1, 0, 0}, 0, 0},
{{bcmVlanActionOuterAdd, bcmVlanActionMappedReplace},
    {-1, 0, 0}, 0, 0},
{{bcmVlanActionInnerAdd, bcmVlanActionMappedReplace},
    {-1, 0, 0}, 0, 0},
{{bcmVlanActionNone, bcmVlanActionMappedAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 0, 1},
{{bcmVlanActionReplace, bcmVlanActionMappedAdd},
    {1, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_INNER_VID}, 0, 1},
{{bcmVlanActionAdd, bcmVlanActionMappedAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN}, 0, 1},
{{bcmVlanActionDelete, bcmVlanActionMappedAdd},
    {1, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 0, 1},
{{bcmVlanActionCopy, bcmVlanActionMappedAdd},
    {1, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID}, 0, 1},
{{bcmVlanActionMappedReplace, bcmVlanActionMappedAdd},
    {1, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN}, 0, 1},
{{bcmVlanActionMappedAdd, bcmVlanActionMappedAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN}, 0, 1},
{{bcmVlanActionOuterAdd, bcmVlanActionMappedAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN}, 0, 1},
{{bcmVlanActionInnerAdd, bcmVlanActionMappedAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN}, 0, 1},
{{bcmVlanActionNone, bcmVlanActionOuterAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 1, 1},
{{bcmVlanActionReplace, bcmVlanActionOuterAdd},
    {1, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_INNER_VID}, 1, 1},
{{bcmVlanActionAdd, bcmVlanActionOuterAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID}, 1, 1},
{{bcmVlanActionDelete, bcmVlanActionOuterAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 1, 1},
{{bcmVlanActionCopy, bcmVlanActionOuterAdd},
    {1, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID}, 1, 1},
{{bcmVlanActionMappedReplace, bcmVlanActionOuterAdd},
    {1, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN}, 0, 1},
{{bcmVlanActionMappedAdd, bcmVlanActionOuterAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID}, 0, 1},
{{bcmVlanActionOuterAdd, bcmVlanActionOuterAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID ,DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID}, 1, 1},
{{bcmVlanActionInnerAdd, bcmVlanActionOuterAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID ,DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID}, 1, 1},
{{bcmVlanActionNone, bcmVlanActionInnerAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 1, 1},
{{bcmVlanActionReplace, bcmVlanActionInnerAdd},
    {1, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_INNER_VID}, 1, 1},
{{bcmVlanActionAdd, bcmVlanActionInnerAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_OUTER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID}, 1, 1},
{{bcmVlanActionDelete, bcmVlanActionInnerAdd},
    {1, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY}, 1, 1},
{{bcmVlanActionCopy, bcmVlanActionInnerAdd},
    {1, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID}, 1, 1},
{{bcmVlanActionMappedReplace, bcmVlanActionInnerAdd},
    {1, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN}, 0, 1},
{{bcmVlanActionMappedAdd, bcmVlanActionInnerAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID}, 0, 1},
{{bcmVlanActionOuterAdd, bcmVlanActionInnerAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID ,DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID}, 1, 1},
{{bcmVlanActionInnerAdd, bcmVlanActionInnerAdd},
    {0, DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID ,DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID}, 1, 1},
};

static dbal_enum_value_field_ive_outer_vid_src_e dnx_vlan_translate_ive_vid_action_mapping[] = {
     /**
      *  No VLAN Edit action for the tag - 
      *  DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY
      */
     DBAL_ENUM_FVAL_IVE_OUTER_VID_SRC_EMPTY,
     /**
      *  VID Source is the AC LIF Outer VID -
      *  DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_OUTER_VID
      */
     DBAL_ENUM_FVAL_IVE_OUTER_VID_SRC_AC_OUTER_VID,     
     /**
      *  VID Source is the AC LIF Inner VID -
      *  DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_INNER_VID
      */
     DBAL_ENUM_FVAL_IVE_OUTER_VID_SRC_AC_INNER_VID,
     /**
      *  VID Source is the Incoming packet Outer VID -
      *  DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID
      */
     DBAL_ENUM_FVAL_IVE_OUTER_VID_SRC_INCOMING_OUTER_VID,
     /**
      *  VID Source is the Incoming packet Inner VID -
      *  DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID
      */
     DBAL_ENUM_FVAL_IVE_OUTER_VID_SRC_INCOMING_INNER_VID
};

static dbal_enum_value_field_eve_outer_vid_src_e dnx_vlan_translate_eve_vid_action_mapping[] = {
     /**
      *  No VLAN Edit action for the tag - 
      *  DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY
      */
     DBAL_ENUM_FVAL_EVE_OUTER_VID_SRC_EMPTY,
     /**
      *  VID Source is the AC LIF Outer VID -
      *  DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_OUTER_VID
      */
     DBAL_ENUM_FVAL_EVE_OUTER_VID_SRC_AC_OUTER_VID,     
     /**
      *  VID Source is the AC LIF Inner VID -
      *  DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_AC_INNER_VID
      */
     DBAL_ENUM_FVAL_EVE_OUTER_VID_SRC_AC_INNER_VID,
     /**
      *  VID Source is the Incoming packet Outer VID -
      *  DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_OUTER_VID
      */
     DBAL_ENUM_FVAL_EVE_OUTER_VID_SRC_INCOMING_OUTER_VID,
     /**
      *  VID Source is the Incoming packet Inner VID -
      *  DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_INCOMING_INNER_VID
      */
     DBAL_ENUM_FVAL_EVE_OUTER_VID_SRC_INCOMING_INNER_VID,
     /**
      *  VID Source is the Forward domain (VSI) -
      *  DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_FORWARD_DOMAIN
      */
     DBAL_ENUM_FVAL_EVE_OUTER_VID_SRC_FORWARD_DOMAIN
};

static dbal_enum_value_field_ive_outer_pcp_dei_src_e dnx_vlan_translate_ive_pcp_dei_action_mapping[] = {
     /**
      *  PCP-DEI Source is the AC LIF Outer PCP-DEI mapping -
      *  DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_AC_OUTER_PCP_DEI
      */
     DBAL_ENUM_FVAL_EVE_OUTER_PCP_DEI_SRC_AC_OUTER_PCP_DEI,     
     /**
      *  PCP_DEI Source is the AC LIF Inner PCP-DEI mapping -
      *  DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_AC_INNER_PCP_DEI
      */
     DBAL_ENUM_FVAL_EVE_OUTER_PCP_DEI_SRC_AC_INNER_PCP_DEI,
     /**
      *  PCP_DEI Source is the Incoming packet Outer PCP-DEI mapping
      *  DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_INCOMING_OUTER_PCP_DEI
      */
     DBAL_ENUM_FVAL_EVE_OUTER_PCP_DEI_SRC_INCOMING_OUTER_PCP_DEI,
     /**
      *  PCP_DEI Source is the Incoming packet Inner PCP-DEI mapping
      *  DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_INCOMING_INNER_PCP_DEI
      */
     DBAL_ENUM_FVAL_EVE_OUTER_PCP_DEI_SRC_INCOMING_INNER_PCP_DEI
};


static dbal_enum_value_field_eve_outer_pcp_dei_src_e dnx_vlan_translate_eve_pcp_dei_action_mapping[] = {
     /**
      *  PCP-DEI Source is the AC LIF Outer PCP-DEI mapping -
      *  DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_AC_OUTER_PCP_DEI
      */
     DBAL_ENUM_FVAL_EVE_OUTER_PCP_DEI_SRC_AC_OUTER_PCP_DEI,     
     /**
      *  PCP_DEI Source is the AC LIF Inner PCP-DEI mapping -
      *  DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_AC_INNER_PCP_DEI
      */
     DBAL_ENUM_FVAL_EVE_OUTER_PCP_DEI_SRC_AC_INNER_PCP_DEI,
     /**
      *  PCP_DEI Source is the Incoming packet Outer PCP-DEI mapping
      *  DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_INCOMING_OUTER_PCP_DEI
      */
     DBAL_ENUM_FVAL_EVE_OUTER_PCP_DEI_SRC_INCOMING_OUTER_PCP_DEI,
     /**
      *  PCP_DEI Source is the Incoming packet Inner PCP-DEI mapping
      *  DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_INCOMING_INNER_PCP_DEI
      */
     DBAL_ENUM_FVAL_EVE_OUTER_PCP_DEI_SRC_INCOMING_INNER_PCP_DEI
};


/**
 * \brief - Verify function for
 *        bcm_dnx_vlan_port_translation_set
 */
static shr_error_e
dnx_vlan_port_translation_set_verify(
    int unit,
    bcm_vlan_port_translation_t *vlan_port_translation)
{
    int is_ingrees, is_egrees;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Checking that the vlan_port_translation pointer is not NULL: 
     */
    SHR_NULL_CHECK(vlan_port_translation, _SHR_E_PARAM, "vlan_port_translation");

    /*
     *  Verify vlan_port_translation->flags;
     *    - One of Ingrees/Egrees is set.
     *    - Only one of Ingrees/Egrees is set.
     */
    is_ingrees = (vlan_port_translation->flags & BCM_VLAN_ACTION_SET_INGRESS) ? TRUE : FALSE;
    is_egrees = (vlan_port_translation->flags & BCM_VLAN_ACTION_SET_EGRESS) ? TRUE : FALSE;

    if ((is_ingrees == FALSE) && (is_egrees == FALSE))
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "Wrong flags setting. Neither INGREES nor EGREES are set!!! flags = 0x%08X\n",
                     vlan_port_translation->flags);
    }

    if ((is_ingrees == TRUE) && (is_egrees == TRUE)) {
        SHR_ERR_EXIT(_SHR_E_PARAM, "Wrong flags setting. Both INGREES and EGREES are set!!! flags = 0x%08X\n",
                     vlan_port_translation->flags);
    }

    /* 
     * Verify vlan_port_translation->new_outer_vlan, new_inner_vlan;
     *      - the new VLAN values should be in range
     */
    BCM_DNX_VLAN_CHK_ID(unit, vlan_port_translation->new_outer_vlan);
    BCM_DNX_VLAN_CHK_ID(unit, vlan_port_translation->new_inner_vlan);

    
    /*
    BCM_DPP_VLAN_EDIT_PROFILE_VALID(unit, vlan_port_translation->vlan_edit_class_id,
        (vlan_port_translation->flags & BCM_VLAN_ACTION_SET_INGRESS)); */


    

    
    /*
    if (!(gport_info.flags & DNX_ALGO_GPM_GPORT_INFO_F_IS_LOCAL_PORT))
    {
        SHR_ERR_EXIT(BCM_E_PORT, "The API is supported only for local ports.\r\n");
    } */

    


exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Verify function for
 *        bcm_dnx_vlan_translate_action_id_create
 */
static shr_error_e
dnx_vlan_translate_action_id_create_verify(
    int unit,
    uint32 flags, 
    int *action_id)
{
  return 0;

}

/**
 * \brief - Verify function for
 *        bcm_dnx_vlan_translate_action_id_set
 */
static shr_error_e
dnx_vlan_translate_action_id_set_verify(
    int unit,
    uint32 flags, 
    int action_id,
    bcm_vlan_action_set_t *action)
{
  return 0;
}


/**
 * \brief - Verify function for
 *        bcm_dnx_vlan_translate_action_class_set
 */
static shr_error_e
dnx_vlan_translate_action_class_set_verify(
    int unit,
    bcm_vlan_translate_action_class_t *action_class)
{
  return 0;
}



/**
 * \brief - Conversion function from a combination of VID 
 *        actions (Outer & Inner), to a HW representation that
 *        is comprised of the tags to remove and the VID Source
 *        for each tag.
 *        The conversion is performed by looking up for a
 *        matching entry of VID actions in a dedicated static
 *        table.
 */
static shr_error_e dnx_vlan_translate_vid_action_to_dnx(
    int unit,
    uint32 is_ingress,
    bcm_vlan_action_set_t *bcm_action,
    dnx_vlan_translate_action_dnx_format_t *dnx_vlan_translation_command)
{
    uint32 tbl_idx, is_valid_conversion, nof_conversion_tbl_entries;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Lookup for the BCM API VID-Action combination in a conversion table
     */
    nof_conversion_tbl_entries = sizeof(dnx_vlan_translation_conversion_info) / sizeof(dnx_vlan_translation_conversion_info[0]);
    for (tbl_idx = 0; tbl_idx < nof_conversion_tbl_entries; tbl_idx++)
    {
        if ((dnx_vlan_translation_conversion_info[tbl_idx].vlan_action_bcm.outer_tag_action == bcm_action->dt_outer) &&
            (dnx_vlan_translation_conversion_info[tbl_idx].vlan_action_bcm.inner_tag_action == bcm_action->dt_inner))
        {
            is_valid_conversion = (is_ingress) ? dnx_vlan_translation_conversion_info[tbl_idx].is_valid_ingress_conversion :
                dnx_vlan_translation_conversion_info[tbl_idx].is_valid_egress_conversion;

            if (is_valid_conversion) {
                dnx_vlan_translation_command->nof_tags_to_remove =
                    dnx_vlan_translation_conversion_info[tbl_idx].vlan_action_dnx.nof_tags_to_remove;
                dnx_vlan_translation_command->vid_source_outer =
                    dnx_vlan_translation_conversion_info[tbl_idx].vlan_action_dnx.vid_source_outer;
                dnx_vlan_translation_command->vid_source_inner =
                    dnx_vlan_translation_conversion_info[tbl_idx].vlan_action_dnx.vid_source_inner;
            }
            else {
                SHR_ERR_EXIT(BCM_E_UNAVAIL, "Invalid VLAN Translation configuration with actions %d,%d at %s",
                    bcm_action->dt_outer, bcm_action->dt_inner, (is_ingress)?"Ingress":"Egress");
            }
            SHR_EXIT();
        }
    }

    SHR_ERR_EXIT(BCM_E_INTERNAL, "Missing entry in VLAN Translation conversion table for actions %d,%d",
                    bcm_action->dt_outer, bcm_action->dt_inner);

exit:
    SHR_FUNC_EXIT; 
}


/**
 * \brief - Conversion function for VLAN Translation PCP-DEI 
 *        action in BCM API format to a DNX HW command value.
 *        The conversion is performed by 
 */
static shr_error_e dnx_vlan_translate_pcp_dei_action_to_dnx(
    int unit,
    bcm_vlan_action_set_t *bcm_action,
    dnx_vlan_translate_action_dnx_format_t *dnx_vlan_translation_command)
{

    /*
     * Handle the PCP-DEI action for the Outer tag. 
     * The conversion is relative to the tag position and therefore is different from Inner Tag conversion. 
     */
    if (dnx_vlan_translation_command->vid_source_outer != DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY)
    {
        switch (bcm_action->dt_outer_pkt_prio)
        {
        case bcmVlanActionNone:
        case bcmVlanActionOuterAdd:
            dnx_vlan_translation_command->pcp_dei_source_outer = DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_INCOMING_OUTER_PCP_DEI;
            break;
        case bcmVlanActionAdd:
        case bcmVlanActionReplace:
            
            dnx_vlan_translation_command->pcp_dei_source_outer = DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_AC_OUTER_PCP_DEI;
            break;
        case bcmVlanActionCopy:
        case bcmVlanActionInnerAdd:
        default:
            dnx_vlan_translation_command->pcp_dei_source_outer = DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_INCOMING_INNER_PCP_DEI;
            break;
        }
    }
    else
    {
        /*
         * Any valid value can be set when the tag isn't created as it isn't applicable/
         */
        dnx_vlan_translation_command->pcp_dei_source_outer = DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_INCOMING_INNER_PCP_DEI;
    }

    /*
     * Handle the PCP-DEI action for the Inner tag. 
     * The conversion is relative to the tag position and therefore is different from Outer Tag conversion. 
     */
    if (dnx_vlan_translation_command->vid_source_inner != DNX_VLAN_TRANSLATION_VID_SOURCE_TYPE_EMPTY)
    {
        switch (bcm_action->dt_inner_pkt_prio)
        {
        case bcmVlanActionNone:
        case bcmVlanActionInnerAdd:
            dnx_vlan_translation_command->pcp_dei_source_inner = DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_INCOMING_INNER_PCP_DEI;
            break;
        case bcmVlanActionAdd:
        case bcmVlanActionReplace:
            
            dnx_vlan_translation_command->pcp_dei_source_inner = DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_AC_INNER_PCP_DEI;
            break;
        case bcmVlanActionCopy:
        case bcmVlanActionOuterAdd:
        default:
            dnx_vlan_translation_command->pcp_dei_source_inner = DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_INCOMING_OUTER_PCP_DEI;
            break;
        }
    }
    else
    {
        
        dnx_vlan_translation_command->pcp_dei_source_inner = DNX_VLAN_TRANSLATION_PCP_DEI_SOURCE_TYPE_INCOMING_INNER_PCP_DEI;
    }

    return BCM_E_NONE;
}


/**
 * \brief - Conversion function for VLAN Translation TPID value 
 *        as in BCM API format to a DNX HW command value - TPID
 *        index.
 */
static shr_error_e dnx_vlan_translate_tpid_action_to_dnx(
    int unit,
    bcm_vlan_action_set_t *bcm_action,
    dnx_vlan_translate_action_dnx_format_t *dnx_vlan_translation_command)
{
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Retrieve the Global TPID-Index for a given TPID value, both for the Outer & Inner tags
     */
    SHR_IF_ERR_EXIT(dnx_switch_tpid_index_get(unit, bcm_action->outer_tpid, &(dnx_vlan_translation_command->outer_tpid_idx)));
    if (dnx_vlan_translation_command->outer_tpid_idx == BCM_DNX_SWITCH_TPID_INDEX_INVALID)
    {
        SHR_ERR_EXIT(BCM_E_UNAVAIL, "The specified Outer TPID isn't configured for the device - %04x",
            bcm_action->outer_tpid);
    }

    SHR_IF_ERR_EXIT(dnx_switch_tpid_index_get(unit, bcm_action->inner_tpid, &(dnx_vlan_translation_command->inner_tpid_idx)));
    if (dnx_vlan_translation_command->inner_tpid_idx == BCM_DNX_SWITCH_TPID_INDEX_INVALID)
    {
        SHR_ERR_EXIT(BCM_E_UNAVAIL, "The specified Inner TPID isn't configured for the device - %04x",
            bcm_action->inner_tpid);
    }

exit:
    SHR_FUNC_EXIT; 
}


/**
 * \brief - conversion function from a BCM API VLAN Translation 
 * action to a DNX HW VLAN Translation command info. 
 * The function first converts the VID action. If that's 
 * valid, it proceedes to determine the PCP-DEI and TPID-Index 
 * for each added tag. 
 */
static shr_error_e dnx_vlan_translate_action_to_dnx_command(
   int unit,
   uint32 is_ingress,
   bcm_vlan_action_set_t *action,
   dnx_vlan_translate_action_dnx_format_t *dnx_vlan_translation_command)
{

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Convert the VID actions from BCM API format to DNX HW
     */
    SHR_IF_ERR_EXIT(dnx_vlan_translate_vid_action_to_dnx(unit, is_ingress, action, dnx_vlan_translation_command));

    


    /*
     * Convert the PCP-DEI actions from BCM API format to DNX HW
     */
    SHR_IF_ERR_EXIT(dnx_vlan_translate_pcp_dei_action_to_dnx(unit, action, dnx_vlan_translation_command));

    /*
     * Convert the TPIDs from BCM API format to DNX HW
     */
    SHR_IF_ERR_EXIT(dnx_vlan_translate_tpid_action_to_dnx(unit, action, dnx_vlan_translation_command));

exit:
    SHR_FUNC_EXIT;   
}


/**
 * \brief
 * This API configures VLAN Transation parameters per gport -
 * In-LIF, Out-LIF, ESEM or Egress Default.
 * 
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit.
 *   \param [in] vlan_port_translation -
 *     A structure that holds the VLAN Translation parameters
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error. See shr_error_e, for example: invalid vid.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *   * Update HW according to the supplied gport - In-LIF,
 *     Out-LIF, ESEM or Egress Default.
 */
/* Set translation information for a vlan port */
shr_error_e bcm_dnx_vlan_port_translation_set(
    int unit, 
    bcm_vlan_port_translation_t *vlan_port_translation)
{
    uint32 entry_handle_id;
    bcm_gport_t local_in_lif, local_out_lif;
    uint8 is_ingress;
    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_vlan_port_translation_set_verify(unit, vlan_port_translation));

    is_ingress = _SHR_IS_FLAG_SET(vlan_port_translation->flags, BCM_VLAN_ACTION_SET_INGRESS) ? TRUE : FALSE;

    
    local_in_lif = BCM_GPORT_SUB_TYPE_LIF_VAL_GET(BCM_GPORT_VLAN_PORT_ID_GET(vlan_port_translation->gport));

    /*
     * Handle the Ingress scenario
     */
    if (is_ingress)
    {

        


        /*
         * Set the VLAN Editing values to the local In-LIF HW
         */
        SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_IN_AC_INFO_DB, &entry_handle_id));
        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_IN_LIF, INST_SINGLE, local_in_lif);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_RESULT_TYPE, INST_SINGLE, DBAL_RESULT_TYPE_IN_AC_INFO_DB_IN_LIF_FORMAT_AC_MP);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_PROFILE, INST_SINGLE, vlan_port_translation->vlan_edit_class_id);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_VID_1, INST_SINGLE, vlan_port_translation->new_outer_vlan);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_VID_2, INST_SINGLE, vlan_port_translation->new_inner_vlan);
        SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));
    }
    else
    {
        
        local_out_lif = local_in_lif;

        /*
         * Set the VLAN Editing values to the local Out-LIF HW
         */
        SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_EEDB_OUT_AC, &entry_handle_id));
        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_OUT_LIF, INST_SINGLE, local_out_lif);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_PROFILE, INST_SINGLE, vlan_port_translation->vlan_edit_class_id);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_VID_1, INST_SINGLE, vlan_port_translation->new_outer_vlan);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_VID_2, INST_SINGLE,vlan_port_translation->new_inner_vlan);
        SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));
    }

exit:
    SHR_FUNC_EXIT;   
}

/* Get translation information from a vlan port */
int bcm_dnx_vlan_port_translation_get(
    int unit, 
    bcm_vlan_port_translation_t *vlan_port_translation)
{
    return -1;
}

/**
 * \brief
 * This API allocates an Ingress VLAN Editing(IVE) or Egress 
 * VLAN Editing (EVE) Command ID. 
 * 
 * \par DIRECT INPUT 
 *   \param [in] unit -
 *     Relevant unit.
 *   \param [in] flags -
 *     Flags that are used for controlling the API like WITH_ID
 *     and to determine Ingress or Egress opertaion.
 *   \param [in] action_id -
 *     Pointer to a VLAN Translation Command ID.
 *     \b As \b output - \n
 *     Pointer to an SDK allocated VLAN Translation Command ID.
 *     \b As \b input - \n
 *     Matched with a WITH_ID flag, the pointed value contains a
 *     user supplied VLAN Translation Command ID for allocation.
 * \par INDIRECT INPUT: 
 *   * \b *action_id \n
 *     See 'action_id' in DIRECT INPUT above
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error. See shr_error_e, for example: invalid vid.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *   * SW Allocation of a VLAN Translation Command ID, either
 *     for Ingress or Egress.
 *   * \b *action_id \n
 *     See 'action_id' in DIRECT INPUT above
 */
int bcm_dnx_vlan_translate_action_id_create(
    int unit, 
    uint32 flags, 
    int *action_id)
{
    /* uint8 is_ingress; */
    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_vlan_translate_action_id_create_verify(unit, flags, action_id));

    /* is_ingress = _SHR_IS_FLAG_SET(vlan_port_translation->flags, BCM_VLAN_ACTION_SET_INGRESS) ? TRUE : FALSE; */

    
    *action_id = 5;
         
exit:
    SHR_FUNC_EXIT;   
}

/* Destroy a VLAN translation ID instance */
int bcm_dnx_vlan_translate_action_id_destroy(
    int unit, 
    uint32 flags, 
    int action_id)
{
    return -1;
}

/* Destroy all VLAN translation ID instances */
int bcm_dnx_vlan_translate_action_id_destroy_all(
    int unit, 
    uint32 flags)
{
    return -1;
}

/* Set a VLAN translation ID instance with tag actions */
int bcm_dnx_vlan_translate_action_id_set(
    int unit, 
    uint32 flags, 
    int action_id, 
    bcm_vlan_action_set_t *action)
{
    uint32 entry_handle_id;
    uint8 is_ingress;
    dnx_vlan_translate_action_dnx_format_t dnx_vlan_translation_command;
    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_vlan_translate_action_id_set_verify(unit, flags, action_id, action));

    is_ingress = _SHR_IS_FLAG_SET(flags, BCM_VLAN_ACTION_SET_INGRESS) ? TRUE : FALSE;

    /*
     * Convert the VLAN Translation action from BCM API format to DNX HW info
     */
    SHR_IF_ERR_EXIT(dnx_vlan_translate_action_to_dnx_command(unit, is_ingress, action, &dnx_vlan_translation_command));

    /*
     * Set the modified VLAN Translation command to the HW - Ingress or Egress
     */
    if (is_ingress)
    {
        SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_IVEC, &entry_handle_id));

        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_COMMAND_INDEX, INST_SINGLE, action_id);

        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_OUTER_TPID_INDEX, INST_SINGLE, dnx_vlan_translation_command.outer_tpid_idx);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_IVE_OUTER_VID_SRC, INST_SINGLE,
            dnx_vlan_translate_ive_vid_action_mapping[dnx_vlan_translation_command.vid_source_outer]);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_IVE_OUTER_PCP_DEI_SRC, INST_SINGLE,
            dnx_vlan_translate_ive_pcp_dei_action_mapping[dnx_vlan_translation_command.pcp_dei_source_outer]);

        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_INNER_TPID_INDEX, INST_SINGLE, dnx_vlan_translation_command.inner_tpid_idx);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_IVE_INNER_VID_SRC, INST_SINGLE,
            dnx_vlan_translate_ive_vid_action_mapping[dnx_vlan_translation_command.vid_source_inner]);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_IVE_INNER_PCP_DEI_SRC, INST_SINGLE,
            dnx_vlan_translate_ive_pcp_dei_action_mapping[dnx_vlan_translation_command.pcp_dei_source_inner]);

        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_TAGS_TO_REMOVE, INST_SINGLE, dnx_vlan_translation_command.nof_tags_to_remove);

        SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));
    }
    else
    {
        SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_EVEC_FRWRD_STAGE, &entry_handle_id));

        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_COMMAND_INDEX, INST_SINGLE, action_id);

        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_OUTER_TPID_INDEX, INST_SINGLE, dnx_vlan_translation_command.outer_tpid_idx);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_EVE_OUTER_VID_SRC, INST_SINGLE,
            dnx_vlan_translate_eve_vid_action_mapping[dnx_vlan_translation_command.vid_source_outer]);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_EVE_OUTER_PCP_DEI_SRC, INST_SINGLE,
            dnx_vlan_translate_eve_pcp_dei_action_mapping[dnx_vlan_translation_command.pcp_dei_source_outer]);

        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_INNER_TPID_INDEX, INST_SINGLE, dnx_vlan_translation_command.inner_tpid_idx);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_EVE_INNER_VID_SRC, INST_SINGLE,
            dnx_vlan_translate_eve_vid_action_mapping[dnx_vlan_translation_command.vid_source_inner]);
        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_EVE_INNER_PCP_DEI_SRC, INST_SINGLE,
            dnx_vlan_translate_eve_pcp_dei_action_mapping[dnx_vlan_translation_command.pcp_dei_source_inner]);

        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_TAGS_TO_REMOVE, INST_SINGLE, dnx_vlan_translation_command.nof_tags_to_remove);

        SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));
        
        

    }

exit:
    SHR_FUNC_EXIT;
}

/* Get tag actions from a VLAN translation ID instance */
int bcm_dnx_vlan_translate_action_id_get(
    int unit, 
    uint32 flags, 
    int action_id, 
    bcm_vlan_action_set_t *action)
{
    return -1;
}


/**
 * \brief
 * This API sets a VLAN translation mapping. It maps a 
 * combination of tag-format and VLAN-Edit-Profile to a 
 * Command-ID, either for Ingress or Egress. 
 * In the Egress case, the mapping is used by the HW to index 
 * into the Forwarding stage EVEC table, but not to the Encap 
 * stage EVEC table. 
 * 
 * \par DIRECT INPUT
 *    \param [in] unit -
 *     Relevant unit.
 *   \param [in] action_class -
 *     A pointer to a structure that holds the mapping
 *     parameters: tag-format, VLAN-Edit-Profile and the result
 *     Command-ID.
 *     It also contains flags to determine Ingress or Egress
 *     operation.
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   \retval Negative in case of an error. See shr_error_e, for example: invalid vid.
 *   \retval Zero in case of NO ERROR
 * \par INDIRECT OUTPUT
 *   * HW IVEC-Mapping or EVEC-Mappings are modified with the
 *     supplied mapping.
 */
int bcm_dnx_vlan_translate_action_class_set(
    int unit, 
    bcm_vlan_translate_action_class_t *action_class)
{
    uint32 entry_handle_id;
    uint8 is_ingress;
    SHR_FUNC_INIT_VARS(unit);

    SHR_INVOKE_VERIFY_DNX(dnx_vlan_translate_action_class_set_verify(unit, action_class));

    is_ingress = _SHR_IS_FLAG_SET(action_class->flags, BCM_VLAN_ACTION_SET_INGRESS) ? TRUE : FALSE;

    /*
     * Perform IVEC/EVEC-Mapping configuration according to the selected flag
     */
    if (is_ingress)
    {
        SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_IVEC_MAPPING, &entry_handle_id));

        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_LLVP_INCOMING_TAG_STRUCTURE, INST_SINGLE, action_class->tag_format_class_id);
        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_PROFILE, INST_SINGLE, action_class->vlan_edit_class_id);

        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_COMMAND_INDEX, INST_SINGLE, action_class->vlan_translation_action_id);

        SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));
    }
    else
    {         
        SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_EVEC_MAPPING, &entry_handle_id));

        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_LLVP_INCOMING_TAG_STRUCTURE, INST_SINGLE, action_class->tag_format_class_id);
        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_PROFILE, INST_SINGLE, action_class->vlan_edit_class_id);

        dbal_entry_value_field32_set(unit, entry_handle_id, DBAL_FIELD_VLAN_EDIT_COMMAND_INDEX, INST_SINGLE, action_class->vlan_translation_action_id);

        SHR_IF_ERR_EXIT(dbal_entry_commit(unit, entry_handle_id, DBAL_COMMIT_NORMAL));
    }
             
exit:
    SHR_FUNC_EXIT; 
}


/* 
 * Get the action ID that is configured for a specified combination of
 * packet format ID and VLAN edit profile
 */
int bcm_dnx_vlan_translate_action_class_get(
    int unit, 
    bcm_vlan_translate_action_class_t *action_class)
{
    return -1;
}

