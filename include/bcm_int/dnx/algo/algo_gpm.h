/** \file algo_gpm.h
 * $Id$
 * 
 * Internal DNX Gport Managment APIs 
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef ALGO_GPM_H_INCLUDED
/*
 * { 
 */
#define ALGO_GPM_H_INCLUDED

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

/*
 * Includes.
 * {
 */
/*
 * }
 */

/**
 * Invalif LIF indication (both ingress, egress, global and
 * local)
 */
#define DNX_ALGO_GPM_LIF_INVALID (-1)
/**
 * Invalif FEC indication
 */
#define DNX_ALGO_GPM_FEC_INVALID (-1)

/**
 * \addtogroup Flags DNX_ALGO_GPM_GPORT_INFO_F_XXX 
 * Flags for dnx_algo_gpm_gport_phy_info_t struct 
 *  
 * @{ 
 */
/*
 * {
 */
/**
 * Gport is Local Port
 */
#define DNX_ALGO_GPM_GPORT_INFO_F_IS_LOCAL_PORT SAL_BIT(0)
/**
 * Gport is LAG
 */
#define DNX_ALGO_GPM_GPORT_INFO_F_IS_LAG        SAL_BIT(1)
/**
 * Gport is system side
 */
#define DNX_ALGO_GPM_GPORT_INFO_F_IS_SYS_SIDE   SAL_BIT(2)
/**
 * Gport is Black Hole (drop)
 */
#define DNX_ALGO_GPM_GPORT_INFO_F_IS_BLACK_HOLE SAL_BIT(3)
/*
 * }
 */
/** @}*/

/**
 * \brief GPORT physical information.
 *
 * This structure contains physical information of a GPORT.
 * Relevant for GPORTs which are of type other than:
 * 1. FEC (like BCM_GPORT_VLAN/MPLS_PORT of subtype BCM_GPORT_SUB_TYPE_FORWARD_GROUP)
 * 2. Lif (like BCM_GPORT_VLAN/MPLS_PORT of subtype BCM_GPORT_SUB_TYPE_LIF)
 * 3. MC pointer (like BCM_GPORT_VLAN/MPLS_PORT of subtype BCM_GPORT_SUB_TYPE_MULTICAST)
 * 4. Push Profile (BCM_GPORT_MPLS_PORT of subtype BCM_GPORT_SUB_TYPE_MPLS_PUSH_PROFILE)
 *  \see 
 * dnx_algo_gpm_gport_phy_info_get
 */
typedef struct
{
    /**
     * Local port
     */
    bcm_port_t local_port;
    /**
     * System port
     */
    uint32 sys_port;
    /**
     * flags DNX_ALGO_GPM_GPORT_INFO_F_XXX, for example 
     * \ref DNX_ALGO_GPM_GPORT_INFO_F_IS_LOCAL_PORT 
     */
    uint32 flags;
    /**
     * Internal information: pp port (not including core)
     */
    int internal_port_pp;
    /**
     * Internal information: tm port (not including core)
     */
    int internal_port_tm;
    /**
     * Internal information: core id
     */
    int internal_core_id;
} dnx_algo_gpm_gport_phy_info_t;

/** 
 * \addtogroup Operations Operations 
 * Field of dnx_algo_gpm_gport_phy_info_get api
 *  
 * @{  
 */
/*
 * {
 */
/**
 * No Operation 
 */
#define DNX_ALGO_GPM_GPORT_TO_PHY_OP_NONE                        0
/**
 * Force system port retrieve 
 */
#define DNX_ALGO_GPM_GPORT_TO_PHY_OP_RETRIVE_SYS_PORT    SAL_BIT(0)
/**
 * Force local port retrieve
 */
#define DNX_ALGO_GPM_GPORT_TO_PHY_OP_LOCAL_IS_MANDATORY  SAL_BIT(1)
/*
 * }
 */
/** @}*/

/**
* \brief
*   Given a gport, return physical information on physical ports
*   Return error on unssuported types, see dnx_algo_gpm_gport_phy_info_t
*   for more information on supported types.
*   dnx_algo_gpm_gport_is_physical can be used to check whether GPORT is physical
* 
*   Physical information includes port (system/pp/tm ports) and core id.
*   Operations flags specify whether system port should be retrieved and
*   whether local port is mandatory.
* 
*   Also supports local port input (not encoded as bcm_gport_t).
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] port -
*      In GPORT given by user
*    \param [in] operation -
*      Operation requested according to DNX_ALGO_GPM_GPORT_TO_PHY_OP_XXX,
*      for example \ref DNX_ALGO_GPM_GPORT_TO_PHY_OP_NONE
*    \param [in] phy_gport_info -
*      Pointer to location to be loaded by physical info. 
*  \par INDIRECT OUTPUT:
*    *phy_gport_info -
*      Physical information in case gport is physical port. 
*      In case gport is LAG, only system port will be retrieved.
*  \par DIRECT OUTPUT:
*    shr_error_e - 
*      Error return value
*  \remark
*    None
*  \see
*    dnx_algo_gpm_gport_phy_info_t
*    dnx_algo_gpm_gport_is_physical
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_gpm_gport_phy_info_get(
    int unit,
    bcm_gport_t port,
    uint32 operation,
    dnx_algo_gpm_gport_phy_info_t * phy_gport_info);

/**
* \brief
*   Given a gport, returns whether the gport is physical port.
*   GPORT is physical in case it's not one of the following types: 
*   VPAN PORT / MPLS PORT / MIM PORT / TRILL PORT / TUNNEL /
*   L2GRE PORT / VXLAN PORT / FORWARD PORT / EXTENDER PORT
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] port -
*      In GPORT given by user
*    \param [in] is_physical_port -
*      Pointer to location to be set as physical info indication. 
*  \par INDIRECT OUTPUT:
*    *is_physical_port -
*      Boolean return value indicating given gport is physical.
*      Meaning the gport is not of type FEC/Lif/MC pointer/Push Profile
*  \par DIRECT OUTPUT:
*    shr_error_e - 
*      Error return value
*  \remark
*    None
*  \see
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_gpm_gport_is_physical(
    int unit,
    bcm_gport_t port,
    uint8 * is_physical_port);

/**
* \brief
*   Perform validity check on a gport.
*   The check includes range verification per gport type.
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] gport -
*      In GPORT given by user
*  \par DIRECT OUTPUT:
*    shr_error_e - 
*      Error return value if check fails
*  \remark
*    None
*  \see
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_gpm_gport_validity_check(
    int unit,
    bcm_gport_t gport);

/*
 * HARDWARE RESOURCES FUNCTIONS
 */
/*
 * {
 */
/**
 * \brief GPORT hardware resources information.
 *
 * This structure contains HW information of a GPORT. 
 * 1. Local and Global LIFs (like BCM_GPORT_VLAN/MPLS_PORT of subtype 
 * BCM_GPORT_SUB_TYPE_LIF) 
 * 2. FEC (like BCM_GPORT_VLAN/MPLS_PORT of subtype
 * BCM_GPORT_SUB_TYPE_FORWARD_GROUP)
 * Each HW resource field can be filled if valid or mark as invalid if does not exist.
 *  \see 
 * dnx_algo_gpm_gport_to_hw_resources
 */
typedef struct
{
    /**
     * Global-LIF, Invalid when does not exist
     */
    int global_in_lif;
    /**
     * Global-LIF, Invalid when does not exist
     */
    int global_out_lif;
    /**
     * Local-LIF, Invalid when does not exist
     */
    int local_in_lif;
    /**
     * Local-LIF, Invaild when does not exist
     */
    int local_out_lif;
    /**
     * FEC, invalid when does not exist
     */
    int fec_id;
} dnx_algo_gpm_gport_hw_resources_t;

/** 
 * \addtogroup flags DNX_ALGO_GPM_GPORT_HW_RESOURCES_XXX 
 *             Information representing which HW resource to
 *             retrieve. Used by
 *             dnx_algo_gpm_gport_to_hw_resources
 *  
 * @{  
 */
/**
 * Retreive FEC resource 
 */
#define DNX_ALGO_GPM_GPORT_HW_RESOURCES_FEC                   SAL_BIT(0)
/**
 * Retreive Ingress Global LIF 
 */
#define DNX_ALGO_GPM_GPORT_HW_RESOURCES_GLOBAL_LIF_INGRESS    SAL_BIT(1)
/**
 * Retreive Egress Global LIF
 */
#define DNX_ALGO_GPM_GPORT_HW_RESOURCES_GLOBAL_LIF_EGRESS     SAL_BIT(2)
/**
 * Retreive Ingress Local LIF
 */
#define DNX_ALGO_GPM_GPORT_HW_RESOURCES_LOCAL_LIF_INGRESS     SAL_BIT(3)
/**
 * Retreive Egress Local LIF
 */
#define DNX_ALGO_GPM_GPORT_HW_RESOURCES_LOCAL_LIF_EGRESS      SAL_BIT(4)
/**
 * Return error in case required and invalid resource
 */
#define DNX_ALGO_GPM_GPORT_HW_RESOURCES_STRICT_CHECK          SAL_BIT(5)
/**
 * Retreive Local LIF resources
 */
#define DNX_ALGO_GPM_GPORT_HW_RESOURCES_LOCAL_LIF \
    (DNX_ALGO_GPM_GPORT_HW_RESOURCES_LOCAL_LIF_INGRESS | DNX_ALGO_GPM_GPORT_HW_RESOURCES_LOCAL_LIF_EGRESS)
/**
 * Retreive Global LIF resources
 */
#define DNX_ALGO_GPM_GPORT_HW_RESOURCES_GLOBAL_LIF \
    (DNX_ALGO_GPM_GPORT_HW_RESOURCES_GLOBAL_LIF_INGRESS | DNX_ALGO_GPM_GPORT_HW_RESOURCES_GLOBAL_LIF_EGRESS)
/**
 * Retreive Local and Global LIF resources
 */
#define DNX_ALGO_GPM_GPORT_HW_RESOURCES_LOCAL_AND_GLOBAL_LIF \
    (DNX_ALGO_GPM_GPORT_HW_RESOURCES_GLOBAL_LIF | DNX_ALGO_GPM_GPORT_HW_RESOURCES_LOCAL_LIF)
/** @}*/

/**
* \brief
*   Given a gport, returns related HW resources - LIF or FEC.
*   In case of LIF, both global and local can be retrieved.
*   Also ingress and egress options are available.
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] gport -
*      GPORT given by user
*    \param [in] flags -
*       flags of options what to retrieve. See for example
*       \ref DNX_ALGO_GPM_GPORT_HW_RESOURCES_FEC in
*            DNX_ALGO_GPM_GPORT_HW_RESOURCES_XXX
*    \param [in] gport_hw_resources -
*      Pointer to location to be set as GPORT HW resources.
*  \par INDIRECT OUTPUT:
*    *gport_hw_resources -
*      GPORT HW resources
*  \par DIRECT OUTPUT:
*    shr_error_e - 
*      Error return value
*  \remark
*    None
*  \see
*    dnx_algo_gpm_gport_hw_resources_t
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_gpm_gport_to_hw_resources(
    int unit,
    bcm_gport_t gport,
    uint32 flags,
    dnx_algo_gpm_gport_hw_resources_t * gport_hw_resources);

/*
 * END OF HARDWARE RESOURCES FUNCTIONS
 */
/*
 * }
 */

/*
 * FORWARD INFORMATION FUNCTIONS
 */
/*
 * {
 */
/**
* \brief
*   Given a gport, returns forward destination encoded by this gport.
*   The destination can be then used for MACT, Cross-Connect and other forwarding databases
*   Also used to store forwarding information in SW state
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] port -
*      GPORT given by user, might be port or gport.
*      Supported gports - physical gports and FEC gports.
*    \param [in] destination -
*       Pointer to destination DBAL field
*  \par INDIRECT OUTPUT:
*    * destination -
*      DBAL encoding for destination field.
*      Can be FEC/PORT/LAG/TRAP/FLOW-ID
*  \par DIRECT OUTPUT:
*    shr_error_e - 
*      Error return value
*  \remark
*    None
*  \see
*    algo_gpm_gport_and_encap_to_forward_information
*    shr_error_e
*****************************************************/
shr_error_e algo_gpm_encode_destination_field_from_gport(
    int unit,
    bcm_gport_t port,
    uint32 * destination);

/**
 * \brief Forward information.
 *
 * This structure contains pre-fec forward information. it's 
 * used when retrieving forward info in 
 * algo_gpm_gport_and_encap_to_forward_information. The type 
 * specifies which fields of forward information are relevant.
 *  \see 
 * algo_gpm_gport_and_encap_to_forward_information
 */
typedef struct
{
    /**
     * type of forwatding information as set by DBAL in forwarding 
     * tables. Can be DEST_ONLY, DEST_OUTLIF, DEST_EEI 
     */
    uint32 fwd_info_result_type;
    /**
     * destination field, as set by DBAL in forwarding tables. must 
     * be valid for all types
     */
    uint32 destination;
    /**
     * outlif field, as set by DBAL in forwarding tables. valid for 
     * DEST_OUTLIF type only
     */
    uint32 outlif;
    /**
     * eei field, as set by DBAL in forwarding tables. valid for 
     * DEST_EEI type only
     */
    uint32 eei;
} dnx_algo_gpm_forward_info_t;

/**
* \brief
*   Given a gport and encap id, returns forward information as
*   used in forwarding stage (MAC table result).
*   encap_id can be invalid (indicated by
*   BCM_FORWARD_ENCAP_ID_INVALID), in that case it won't be
*   used.
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] gport -
*      GPORT given by user
*    \param [in] encap_id -
*       encapsulation id - can be outlif or EEI, encoded with
*       BCM_FORWARD_ENCAP_ID_XXX macros.
*    \param [in] forward_info -
*   	Pointer to forward info structure that was retrieved
*  \par INDIRECT OUTPUT:
*    * forward_info -
*    See 'forward_info' in DIRECT INPUT above
*  \par DIRECT OUTPUT:
*    shr_error_e - 
*      Error return value
*  \remark
*    None
*  \see
*    dnx_algo_gpm_forward_info_t
*    BCM_FORWARD_ENCAP_ID_INVALID
*    shr_error_e
*****************************************************/
shr_error_e algo_gpm_gport_and_encap_to_forward_information(
    int unit,
    bcm_gport_t gport,
    uint32 encap_id,
    dnx_algo_gpm_forward_info_t * forward_info);

/*
 * END OF FORWARD INFORMATION FUNCTIONS
 */
/*
 * }
 */

/*
 * } 
 */
#endif/*_ALGO_GPM_API_INCLUDED__*/
