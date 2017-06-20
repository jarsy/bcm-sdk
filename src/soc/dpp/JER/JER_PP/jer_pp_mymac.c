/* $Id: jer_pp_mymac.c,v 1.29 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_L3


/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/PPC/ppc_api_mymac.h>
#include <soc/dpp/PPC/ppc_api_fp.h>

#include <soc/mcm/memregs.h>
#include <soc/mcm/memacc.h>
#include <soc/mem.h>
#include <soc/dpp/ARAD/arad_api_ports.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define JER_PP_VRID_TO_IP_VERSION_PROTOCOL_MASK    (3)
#define JER_PP_VRID_TO_IP_VERSION_MASK_NOF_BITS    (2)
#define JER_PP_VRID_TO_IP_VERSION_LOWEST_INDEX     (16)
#define JER_PP_VRID_TO_IP_VERSION_HIGHEST_INDEX    (30)


#define JER_PP_MYMAC_VRRP_TCAM_PROTOCOL_GROUP_MASK_BITS (3)

/* } */
/*************
 * MACROS    *
 *************/
/* { */

/* Creates a mask for the bits that represent the protocol at the given index shift. */
#define JER_PP_VRID_TO_IP_VERSION_PROTOCOL_MASK_CREATE(_index_shift) \
            (JER_PP_VRID_TO_IP_VERSION_PROTOCOL_MASK << (_index_shift))

/* Traverses over all protocols of the vrid_to_ip_version register. A block should be placed after this macro. */
#define JER_PP_VRID_TO_IP_VERSION_TRAVERSE(_index_shift)                                       \
    for (_index_shift = JER_PP_VRID_TO_IP_VERSION_LOWEST_INDEX ;                               \
                _index_shift <= JER_PP_VRID_TO_IP_VERSION_HIGHEST_INDEX ;                      \
                            _index_shift += JER_PP_VRID_TO_IP_VERSION_MASK_NOF_BITS)  

/* Returns TRUE if the protocol at index shift is in the given group. */
#define JER_PP_VRID_TO_IP_VERSION_INDEX_IS_IN_GROUP(_reg_val, _index_shift, _group)                  \
            (JER_PP_VRID_TO_IP_VERSION_GET_GROUP_BY_INDEX((_reg_val), (_index_shift)) == (_group))

/* Returns the group to which the protocol at index shift belongs. */
#define JER_PP_VRID_TO_IP_VERSION_GET_GROUP_BY_INDEX(_reg_val, _index_shift)                           \
            ((_reg_val & JER_PP_VRID_TO_IP_VERSION_PROTOCOL_MASK_CREATE(_index_shift)) >> _index_shift)

/* Resets protocol at index shift back to group 0. */
#define JER_PP_VRID_TO_IP_VERSION_RESET_PROTOCOL(_reg_val, _index_shift)                      \
            ((_reg_val) &= ~JER_PP_VRID_TO_IP_VERSION_PROTOCOL_MASK_CREATE(_index_shift));

/* Translates the index shift of vrid_to_ip_version register to the parsed_l2_next_protocol it represents. */
#define MYMAC_VRID_TO_IP_VERSION_INDEX_TO_PARSED_L2_NEXT_PROTOCOL(_index_shift) \
            ((_index_shift) / JER_PP_VRID_TO_IP_VERSION_MASK_NOF_BITS)

/* If the index shift is divisable by the number of bits, it's a legal index shift. */
#define MYMAC_VRID_TO_IP_VERSION_INDEX_TO_PROTOCOL_FLAG(_index_shift) \
    ((_index_shift % JER_PP_VRID_TO_IP_VERSION_MASK_NOF_BITS == 0) ?    \
    SOC_PPC_PARSED_L2_NEXT_PROTOCOL_TO_L3_PROTOCOL_FLAG(MYMAC_VRID_TO_IP_VERSION_INDEX_TO_PARSED_L2_NEXT_PROTOCOL(_index_shift))    \
    :0)



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
 *   soc_jer_mymac_2nd_mymac_init
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Init 2nd mymac configuration.
 * 
 * INPUT:
 *   int            unit        - (IN) Identifier of the device to access.
 * REMARKS:
 * 
 * RETURNS:
 *   SOC_E_***   If there was a problem reading / writing the register
 *   SOC_E_NONE  Otherwise.
*********************************************************************/

/* set mapping register: VttInnerEthernetTerminationAction
 *
 * format:
 * PTC-Profile Compatible-MC Routing-Enable TT-Lookup.Skip-Ethernet My-MAC | Inner-Ethernet-termination  Trap-Enable
 */
/* value nof bits */
#define SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_INNER_ETHERNET_TERMINATION_NOF_BITS 2 
/* index fields lsbs */
#define SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_MY_MAC_LSB         (0)
#define SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_SKIP_ETHERNET_LSB  (SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_MY_MAC_LSB         + 1)
#define SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_ROUTING_ENABLE_LSB (SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_SKIP_ETHERNET_LSB  + 1)
#define SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_COMPATIBLE_MC_LSB  (SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_ROUTING_ENABLE_LSB + 1)
#define SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_PTC_PROFILE_LSB    (SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_COMPATIBLE_MC_LSB  + 1)

/* get the index according to index fields */
#define SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_INDEX_GET(ptc_profile, compatible_mc, routing_enable, skip_ethernet, my_mac) \
    (((my_mac) << SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_MY_MAC_LSB)                   \
     | ((skip_ethernet) << SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_SKIP_ETHERNET_LSB)   \
     | ((routing_enable) << SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_ROUTING_ENABLE_LSB) \
     | ((compatible_mc) << SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_COMPATIBLE_MC_LSB)   \
     | ((ptc_profile) << SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_PTC_PROFILE_LSB)       \
    )

#define SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_OFFSET_GET(ptc_profile, compatible_mc, routing_enable, skip_ethernet, my_mac) \
    (SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_INNER_ETHERNET_TERMINATION_NOF_BITS * SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_INDEX_GET(ptc_profile, compatible_mc, routing_enable, skip_ethernet, my_mac))



/* set mapping register: MAP_NEXT_PROTOCOL_TO_FWD_CODE
 * format:
 * -----------------------------------------------------------------------------
 * | ethernet-qualifier.next-protocol | ip-qualifier.is_mc ||  forwarding code |
 * |    (4b)                          |      (1b)          ||       (4b)       |
 * -----------------------------------------------------------------------------
 */

#define SOC_JER_PP_MYMAC_MAP_NEXT_PROTOCOL_TO_FWD_CODE_IS_MC_LSB          (0)
#define SOC_JER_PP_MYMAC_MAP_NEXT_PROTOCOL_TO_FWD_CODE_NEXT_PROTOCOL_LSB  (SOC_JER_PP_MYMAC_MAP_NEXT_PROTOCOL_TO_FWD_CODE_IS_MC_LSB +1)

#define SOC_JER_PP_MYMAC_MAP_NEXT_PROTOCOL_TO_FWD_CODE_INDEX_GET(next_protocol, is_mc) \
    (((is_mc) << SOC_JER_PP_MYMAC_MAP_NEXT_PROTOCOL_TO_FWD_CODE_IS_MC_LSB) \
     | ((next_protocol) << SOC_JER_PP_MYMAC_MAP_NEXT_PROTOCOL_TO_FWD_CODE_NEXT_PROTOCOL_LSB) \
    )

/* value nof bits*/ 
#define SOC_JER_PP_MYMAC_MAP_NEXT_PROTOCOL_TO_FWD_CODE_OFFSET_GET(next_protocol, is_mc) \
     (ARAD_PP_FWD_CODE_NOF_BITS * SOC_JER_PP_MYMAC_MAP_NEXT_PROTOCOL_TO_FWD_CODE_INDEX_GET(next_protocol, is_mc))


soc_error_t 
soc_jer_mymac_2nd_mymac_init(int unit){
    int rv;
    soc_reg_above_64_val_t
        reg_above64_val;
    uint32 value; 
    /* variables for the index */ 
    int ptc_profile; 
    int is_skip_ethernet; 
    int is_my_mac; 
    int fwd_code; 
    int is_mc; 

    SOCDNX_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(reg_above64_val);

    /* VttInnerEthernetTerminationAction register has 128 bits
     * The 128 bits is divided into 64 termination decision of 2bits as followed.
     * PTC-Profile Compatible-MC Routing-Enable TT-Lookup.Skip-Ethernet My-MAC | Inner-Ethernet-termination  Trap-Enable
     *    X              0            0                  X                 1   |          0                    1
     *    X              0            0                  X                 0   |          0                    0
     *    X              0            1                  X                 0   |          0                    0
     *    X              0            1                  X                 1   |          1                    0
     *    X              1            0                  X                 X   |          0                    0
     *    X              1            1                  X                 X   |          1                    0
     * 
     * 
     * 
     * 
     * The key format:
     *   PTC profile (0:1)
     *   Compatible-MC (2)
     *   Routing-Enable (3)
     *   TT-Lookup.Skip-Ethernet (4)
     *   My-MAC (5)
     *
     * The data format:
     *   Inner Ethernet Trap enable (0)
     *   Inner Ethernet Termination Enable (1)
     */


    for (ptc_profile=0; ptc_profile<ARAD_PORTS_NOF_FLP_PROFILES; ptc_profile++) {
        for (is_skip_ethernet=0; is_skip_ethernet<2; is_skip_ethernet++) {
            /* PTC-Profile Compatible-MC Routing-Enable TT-Lookup.Skip-Ethernet My-MAC | Inner-Ethernet-termination  Trap-Enable
             *    X              0            0                  X                 1   |          0                    1              */
            value = 1; 
            SHR_BITCOPY_RANGE(reg_above64_val, 
                              SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_OFFSET_GET(ptc_profile, 0, 0, is_skip_ethernet, 1), 
                              &value, 
                              0, 
                              SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_INNER_ETHERNET_TERMINATION_NOF_BITS
                              ); 

            /* PTC-Profile Compatible-MC Routing-Enable TT-Lookup.Skip-Ethernet My-MAC | Inner-Ethernet-termination  Trap-Enable
             *    X              0            0                  X                 0   |          0                    0             */
            value = 0; 
            SHR_BITCOPY_RANGE(reg_above64_val, 
                              SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_OFFSET_GET(ptc_profile, 0, 0, is_skip_ethernet, 0), 
                              &value, 
                              0, 
                              SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_INNER_ETHERNET_TERMINATION_NOF_BITS
                              ); 

            /* PTC-Profile Compatible-MC Routing-Enable TT-Lookup.Skip-Ethernet My-MAC | Inner-Ethernet-termination  Trap-Enable
             *    X              0            1                  X                 0   |          0                    0             */
            value = 0; 
            SHR_BITCOPY_RANGE(reg_above64_val, 
                              SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_OFFSET_GET(ptc_profile, 0, 1, is_skip_ethernet, 0), 
                              &value, 
                              0, 
                              SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_INNER_ETHERNET_TERMINATION_NOF_BITS
                              ); 

            /* PTC-Profile Compatible-MC Routing-Enable TT-Lookup.Skip-Ethernet My-MAC | Inner-Ethernet-termination  Trap-Enable
             *    X              0            1                  X                 1   |          1                    0             */
            value = 2; 
            SHR_BITCOPY_RANGE(reg_above64_val, 
                              SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_OFFSET_GET(ptc_profile, 0, 1, is_skip_ethernet, 1), 
                              &value, 
                              0, 
                              SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_INNER_ETHERNET_TERMINATION_NOF_BITS
                              ); 

            /* PTC-Profile Compatible-MC Routing-Enable TT-Lookup.Skip-Ethernet My-MAC | Inner-Ethernet-termination  Trap-Enable
             *    X              1            0                  X                 X   |          0                    0             */
            value = 0; 
            for (is_my_mac=0; is_my_mac<2; is_my_mac++) {
                SHR_BITCOPY_RANGE(reg_above64_val, 
                                  SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_OFFSET_GET(ptc_profile, 1, 0, is_skip_ethernet, is_my_mac), 
                                  &value, 
                                  0, 
                                  SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_INNER_ETHERNET_TERMINATION_NOF_BITS
                                  ); 

            }

            /* PTC-Profile Compatible-MC Routing-Enable TT-Lookup.Skip-Ethernet My-MAC | Inner-Ethernet-termination  Trap-Enable
             *    X              1            1                  X                 X   |          1                    0             */
            value = 2; 
            for (is_my_mac=0; is_my_mac<2; is_my_mac++) {
                SHR_BITCOPY_RANGE(reg_above64_val, 
                                  SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_OFFSET_GET(ptc_profile, 1, 1, is_skip_ethernet, is_my_mac), 
                                  &value, 
                                  0, 
                                  SOC_JER_PP_MYMAC_INNER_ETHERNET_TERMINTATION_ACTION_INNER_ETHERNET_TERMINATION_NOF_BITS
                                  ); 
            }
        }
    }

    rv = WRITE_IHP_VTT_INNER_ETHERNET_TERMINATION_ACTIONr(unit, SOC_CORE_ALL, reg_above64_val);
    SOCDNX_IF_ERR_EXIT(rv);

    /* The 96 bits register is divided into 32 trap indexes of 3bits. 
     * The key to reach each of 32 trap indexes is comprised of 5 bits
     *   4 bits for the protocol over the native Ethernet 
     *   1 bit for the IP-MC notion.
     * Each trap index will be set to 0.
     */
    SOC_REG_ABOVE_64_CLEAR(reg_above64_val);
    rv = WRITE_IHP_INNER_ETHERNET_TRAP_ACTION_PROFILE_VECTORr(unit, SOC_CORE_ALL, reg_above64_val);
    SOCDNX_IF_ERR_EXIT(rv);


    /* 
     *o	Forwarding-code = 
     *Cfg-Next-Protocol-To-Forwarding-Code[Packet-Format-Qualifier[3].Next-Protocol (4b), Packet-Format-Qualifier[4].Is-MC 1b] 
     * 
     *The 128 bit register is divided into 32 forwading code of 4b divided as followed: 
     * 
     * -----------------------------------------------------------------------------
     * | ethernet-qualifier.next-protocol | ip-qualifier.is_mc ||  forwarding code |
     * |    (4b)                          |      (1b)          ||       (4b)       |
     * -----------------------------------------------------------------------------
     * |            IPv4                  |        0           ||        IPv4-UC   |  
     * |            IPv4                  |        1           ||        IPv4-MC   |
     * |            IPv6                  |        0           ||        IPv6-UC   |  
     * |            IPv6                  |        1           ||        IPv6-MC   |
     * |            OTHER                 |        X           ||        ETH       |
     * -----------------------------------------------------------------------------
     * 
     */

    SOC_REG_ABOVE_64_CLEAR(reg_above64_val);
    value = ARAD_PP_FWD_CODE_IPV4_UC; 

    /* init the whole register with ETH */
    for (fwd_code=0; fwd_code<ARAD_PP_FWD_CODE_NOF_FWD_CODE; fwd_code++) {
        for (is_mc=0;is_mc<2; is_mc++) {
            SHR_BITCOPY_RANGE(reg_above64_val, 
                              SOC_JER_PP_MYMAC_MAP_NEXT_PROTOCOL_TO_FWD_CODE_OFFSET_GET(fwd_code, is_mc), 
                              &value, 
                              0, 
                              ARAD_PP_FWD_CODE_NOF_BITS
                              ); 
        }
    }

    /* ipv4 uc */
    SHR_BITCOPY_RANGE(reg_above64_val, 
                      SOC_JER_PP_MYMAC_MAP_NEXT_PROTOCOL_TO_FWD_CODE_OFFSET_GET(SOC_PPC_FP_PARSED_ETHERTYPE_IPV4, 0), 
                      &value, 
                      0, 
                      ARAD_PP_FWD_CODE_NOF_BITS
                      ); 

    /* ipv4 mc */
    value = ARAD_PP_FWD_CODE_IPV4_MC; 
    SHR_BITCOPY_RANGE(reg_above64_val, 
                      SOC_JER_PP_MYMAC_MAP_NEXT_PROTOCOL_TO_FWD_CODE_OFFSET_GET(SOC_PPC_FP_PARSED_ETHERTYPE_IPV4 , 1), 
                      &value, 
                      0, 
                      ARAD_PP_FWD_CODE_NOF_BITS
                      ); 
    /* ipv6 uc */
    value = ARAD_PP_FWD_CODE_IPV6_UC; 
    SHR_BITCOPY_RANGE(reg_above64_val, 
                      SOC_JER_PP_MYMAC_MAP_NEXT_PROTOCOL_TO_FWD_CODE_OFFSET_GET(SOC_PPC_FP_PARSED_ETHERTYPE_IPV6, 0), 
                      &value, 
                      0, 
                      ARAD_PP_FWD_CODE_NOF_BITS
                      ); 

    /* ipv6 mc */
    value = ARAD_PP_FWD_CODE_IPV6_MC; 
    SHR_BITCOPY_RANGE(reg_above64_val, 
                      SOC_JER_PP_MYMAC_MAP_NEXT_PROTOCOL_TO_FWD_CODE_OFFSET_GET(SOC_PPC_FP_PARSED_ETHERTYPE_IPV6, 1), 
                      &value, 
                      0, 
                      ARAD_PP_FWD_CODE_NOF_BITS
                      ); 
    /* other cases, leave eth (which is 0 */



    rv = WRITE_IHP_MAP_NEXT_PROTOCOL_TO_FWD_CODEr(unit, SOC_CORE_ALL, reg_above64_val);
    SOCDNX_IF_ERR_EXIT(rv);

    SOC_EXIT;
exit:
    SOCDNX_FUNC_RETURN;
}


/*********************************************************************
* NAME:
 *   soc_jer_mymac_protocol_group_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Given a set of l3 protocols and a group id, clears all members of the group (assigns them to group 0) and assigns the protocols to the
 *   selected group id.
 * INPUT:
 *   int            unit        - (IN) Identifier of the device to access.
 *   uint32         protocols   - (IN) Flags of the protocols to be assigned to the group (SOC_PPC_L3_VRRP_PROTOCOL_GROUP_...)
 *   uint8          group       - (IN) Group id of the group to which the protocols will be assigned.
 * REMARKS:
 *   Note that an empty protocol group will just empty the group.
 * RETURNS:
 *   SOC_E_PARAM If group_id > NOF_PROTOCOL_GROUPS
 *   SOC_E_***   If there was a problem reading / writing the register
 *   SOC_E_NONE  Otherwise.
*********************************************************************/
soc_error_t 
soc_jer_mymac_protocol_group_set(int unit, uint32 protocols, uint32 group){
    int rv;
    uint32 reg_val;
    int index_shift;
    SOCDNX_INIT_FUNC_DEFS;

    /* Validate input */
    if (group >= SOC_PPC_L3_NOF_PROTOCOL_GROUPS(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Given protocol group too high")));
    }

    /* Read the register */
    rv = READ_IHP_MAP_VRID_TO_IP_VERSIONr(unit, SOC_CORE_ALL, &reg_val);
    SOCDNX_IF_ERR_EXIT(rv);
    
    /* For each legal protocol, if it's in the group, set it to 0. Then, if it should be in the group, add it. */
    JER_PP_VRID_TO_IP_VERSION_TRAVERSE(index_shift) {

        /* If this protocol is in the set, remove it */
        if (JER_PP_VRID_TO_IP_VERSION_INDEX_IS_IN_GROUP(reg_val, index_shift, group)) {
            JER_PP_VRID_TO_IP_VERSION_RESET_PROTOCOL(reg_val, index_shift);
        }

        /* If this protocol should be in the set, reset it, then add it. */
        if (MYMAC_VRID_TO_IP_VERSION_INDEX_TO_PROTOCOL_FLAG(index_shift) & protocols) {
            JER_PP_VRID_TO_IP_VERSION_RESET_PROTOCOL(reg_val, index_shift);
            reg_val |= group << index_shift;
        }
    }

    /* Write the modified register. */
    rv = WRITE_IHP_MAP_VRID_TO_IP_VERSIONr(unit, SOC_CORE_ALL, reg_val);
    SOCDNX_IF_ERR_EXIT(rv);

    SOC_EXIT;
exit:
    SOCDNX_FUNC_RETURN;
}


/*********************************************************************
* NAME:
 *   soc_jer_mymac_protocol_group_get_protocol_by_group
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Given a group id, returns all protocols assigned to this group.
 * INPUT:
 *   int            unit        - (IN) Identifier of the device to access.
 *   uint8          group       - (IN) Group id of the group to which the protocols are assigned.
 *   uint32         protocols   - (OUT) Flags of the protocols assigned to the group (SOC_PPC_L3_VRRP_PROTOCOL_GROUP_...)
 * REMARKS:
 *   This function is not actually in use, since the data can be obtained from the SW DB. 
 * RETURNS:
 *   SOC_E_PARAM If group_id > NOF_PROTOCOL_GROUPS
 *   SOC_E_***   If there was a problem reading the register
 *   SOC_E_NONE  Otherwise.
*********************************************************************/
soc_error_t 
soc_jer_mymac_protocol_group_get_protocol_by_group(int unit, uint32 group, uint32 *protocols){
    int rv;
    uint32 reg_val;
    int index_shift;
    SOCDNX_INIT_FUNC_DEFS;

    /* Validate input */
    if (group >= SOC_PPC_L3_NOF_PROTOCOL_GROUPS(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Given protocol group too high")));
    }

    SOCDNX_NULL_CHECK(protocols);


    /* Read the register */
    rv = READ_IHP_MAP_VRID_TO_IP_VERSIONr(unit, SOC_CORE_ALL, &reg_val);
    SOCDNX_IF_ERR_EXIT(rv);


    /* Traverse the protocols. If they're in the set, return them. */
    JER_PP_VRID_TO_IP_VERSION_TRAVERSE(index_shift) {

        /* If this protocol is in the set, add it to the return value */
        if (JER_PP_VRID_TO_IP_VERSION_INDEX_IS_IN_GROUP(reg_val, index_shift, group)) {
            *protocols |= MYMAC_VRID_TO_IP_VERSION_INDEX_TO_PROTOCOL_FLAG(index_shift);
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
* NAME:
 *   soc_jer_mymac_protocol_group_get_group_by_protocols
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Given a list of protocols, returns the group to which the protocols are assigned.
 * INPUT:
 *   int            unit        - (IN) Identifier of the device to access.
 *   uint32         protocols   - (IN) Flags of the protocols whose group is inquired. (SOC_PPC_L3_VRRP_PROTOCOL_GROUP_...)
 *   uint8          group       - (OUT) Group id of the group to which the protocols are assigned.
 * REMARKS:
 *   This function is not actually in use, since the data can be obtained from the SW DB. 
 *   
 * RETURNS:
 *   SOC_E_PARAM            If no protocols were given
 *   SOC_E_NOT_FOUND        If the prtocols are in more than one group
 *   SOC_E_***              If there was a problem reading the register
 *   SOC_E_NONE             Otherwise.
*********************************************************************/
soc_error_t 
soc_jer_mymac_protocol_group_get_group_by_protocols(int unit, uint32 protocols, uint32 *group){
    int rv;
    uint32 reg_val;
    uint32 curr_group;
    int index_shift;
    SOCDNX_INIT_FUNC_DEFS;

    /* Validate input */
    if (protocols == 0) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("No protocols given")));
    }

    SOCDNX_NULL_CHECK(group);


    /* Read the register */
    rv = READ_IHP_MAP_VRID_TO_IP_VERSIONr(unit, SOC_CORE_ALL, &reg_val);
    SOCDNX_IF_ERR_EXIT(rv);

    /* Reset protocol group */
    *group = SOC_PPC_L3_VRRP_PROTOCOL_GROUP_INVALID;

    /* Traverse the protocols. When encountering a protocol, save its group. If two protocols are in different groups, return error. */
    JER_PP_VRID_TO_IP_VERSION_TRAVERSE(index_shift) {

        /* If this is a requested protocol, test it */
        if (MYMAC_VRID_TO_IP_VERSION_INDEX_TO_PROTOCOL_FLAG(index_shift) & protocols) {
            curr_group = JER_PP_VRID_TO_IP_VERSION_GET_GROUP_BY_INDEX(reg_val,index_shift);

            /* If a group has already been found (*group is not invalid) then make sure it's the same group.
               If a group hasn't been found, save it.*/
            if (*group < SOC_PPC_L3_NOF_PROTOCOL_GROUPS(unit) && curr_group != *group) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_SOCDNX_MSG("Given protocols are in more than one group")));
            } else {
                *group = curr_group;
            }
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
* NAME:
 *   soc_jer_mymac_vrrp_tcam_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Given a tcam index, and a key built of mac address, mac address mask, protocol group and protocol group mask,
 *   Fills table with the key in the relevant index.
 * INPUT:
 *   int                    unit                        - (IN) Identifier of the device to access.
 *  SOC_PPC_VRRP_CAM_INFO*  info                        - (IN)
 *                              -mac_addr               - (IN) Mac address for the entry.
 *                              -mac_mask               - (IN) Mac mask for the entry.
 *                              -protocol_group         - (IN) Protocol group for the entry.
 *                              -protocol_group_mask    - (IN) Protocol group mask for the entry.
 *                              -vrrp_cam_index         - (IN) Index of the entry.
 * REMARKS:
 *     
 * RETURNS:
 *   SOC_E_***              If there was a problem reading the register
 *   SOC_E_NONE             Otherwise.
*********************************************************************/
soc_error_t
soc_jer_mymac_vrrp_tcam_info_set(int unit, SOC_PPC_VRRP_CAM_INFO *info){
    int rv;
    SOC_SAND_PP_MAC_ADDRESS not_mac_addr;
    uint32
      mac_in_longs[SOC_SAND_PP_MAC_ADDRESS_NOF_UINT32S] = {0},
      data[4] = {0},
      entry_offset = info->vrrp_cam_index,
      not_protocol_group_mask = 0,
      i;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(info);

    /* Write the key (mac address and protocol group). */
    rv = soc_sand_pp_mac_address_struct_to_long(&info->mac_addr, mac_in_longs);
    SOCDNX_SAND_IF_ERR_EXIT(rv);

    soc_IHP_VRID_MY_MAC_TCAMm_field_set(unit, data, DAf, mac_in_longs);
    soc_IHP_VRID_MY_MAC_TCAMm_field32_set(unit, data, IP_VERSIONf, info->protocol_group);

    /* 
     *  Write the key mask.
     *  The key mask actually masks the bits to be ignored, so it needs to be NOT before writing.
     */


    sal_memset(&not_mac_addr, 0, sizeof(not_mac_addr));

    /* Set mac mask to be NOT */
    for (i = 0 ; i < SOC_SAND_PP_MAC_ADDRESS_NOF_U8 ; i++) {
        not_mac_addr.address[i] = ~info->mac_mask.address[i];
    }

    /* Set protocol group mask to be NOT */
    not_protocol_group_mask = ~(info->protocol_group_mask) & JER_PP_MYMAC_VRRP_TCAM_PROTOCOL_GROUP_MASK_BITS;

    rv = soc_sand_pp_mac_address_struct_to_long(&not_mac_addr, mac_in_longs);
    SOCDNX_SAND_IF_ERR_EXIT(rv);

    soc_IHP_VRID_MY_MAC_TCAMm_field_set(unit, data, DA_MASKf, mac_in_longs);
    soc_IHP_VRID_MY_MAC_TCAMm_field32_set(unit, data, IP_VERSION_MASKf, not_protocol_group_mask); 

    /* Set entry to valid */
    soc_IHP_VRID_MY_MAC_TCAMm_field32_set(unit, data, VALIDf, 1);      

    soc_IHP_VRID_MY_MAC_TCAMm_field32_set(unit, data, INDEXf, entry_offset);

    /* Write the register. */
    rv = WRITE_IHP_VRID_MY_MAC_TCAMm(unit, MEM_BLOCK_ANY, entry_offset, data);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
* NAME:
 *   soc_jer_mymac_vrrp_tcam_info_delete
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Given a tcam index, deletes the entry from the table. 
 * INPUT:
 *   int                    unit                   - (IN) Identifier of the device to access.
 *   uint8                  vrrp_cam_index         - (IN) Index of the entry.
 * REMARKS:
 *     
 * RETURNS:
 *   SOC_E_***              If there was a problem reading the register
 *   SOC_E_NONE             Otherwise.
*********************************************************************/
soc_error_t
soc_jer_mymac_vrrp_tcam_info_delete(int unit, uint8 cam_index){
    int rv;
    uint32
      empty_data[4] = {0},
      entry_offset = cam_index;
    SOCDNX_INIT_FUNC_DEFS;

    /* Write an empty register into the field */
    rv = WRITE_IHP_VRID_MY_MAC_TCAMm(unit, MEM_BLOCK_ANY, entry_offset, empty_data);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } */

