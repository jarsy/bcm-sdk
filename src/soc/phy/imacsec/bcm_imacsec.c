/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 $Id$
 */
#ifdef INCLUDE_PLP_IMACSEC

#include <soc/phy/phyctrl.h>
#include <shared/bsl.h>
#include "bcm_imacsec.h"

/*! \brief imacsec_plp_base_t_secy_init
 *
 * Initializes a SecY device instance identified by macsec_side parameter.
 *
 * API use order:
 *       This function must be executed before any other imacsec_plp_base_t_secy_*()
 *       or imacsec_plp_base_t_secy_*() functions. 
 *
 * @param unit (input) BCM unit number
 * @param port (input) - port number
 * @param macsec_side (input) - selects egress or ingress path
 *  0 - egress path
 *  1 - ingress path
 * @param mutex (input) - User defined mutex function registration. 
 * User, optionally, can pass user data as part of this parameter 
 * that in turn gets passed into the mutex function.
 * @param settings_p (input)
 *      Pointer to the memory location where the device settings are stored.
 *
 * This function is NOT re-entrant for the same macsec_side.
 * This function is re-entrant for different macsec_side's.
 *
 * @return
 *     BCM_PLP_BASE_T_SECY_STATUS_OK 0 : success
 *     BCM_PLP_BASE_T_SECY_ERROR_BAD_PARAMETER : incorrect input parameter
 *     BCM_PLP_BASE_T_SECY_INTERNAL_ERROR : failure
 */
int imacsec_plp_base_t_secy_init(int unit, soc_port_t port,
                               unsigned int macsec_side,
                               bcm_plp_base_t_sec_mutex_t *mutex,
                               bcm_plp_base_t_secy_settings_t *setting_p)
{
    phy_ctrl_t  *pc;
    int rv=0;
    bcm_plp_base_t_sec_access_t *pa;
    EXT_PHY_INIT_CHECK(unit,port);
    pc = EXT_PHY_SW_STATE(unit, port);
    IMACSEC_PLP_BASE_T_SEC_ACCESS_GET(pc,pa,macsec_side);
    pa->mutex.user_data=mutex->user_data;
    pa->mutex.SecY0_mutex_take=mutex->SecY0_mutex_take;
    pa->mutex.SecY1_mutex_take=mutex->SecY1_mutex_take;
    pa->mutex.SecY0_mutex_give=mutex->SecY0_mutex_give;
    pa->mutex.SecY1_mutex_give=mutex->SecY1_mutex_give;
    rv = bcm_plp_base_t_secy_device_init(pa,setting_p);
    if(rv !=0){
        LOG_CLI((BSL_META_U(unit,"bcm_plp_base_t_secy_device_init failed:%d\n "), rv));
    }
   return rv;
}
/*! \brief imacsec_plp_base_t_secy_port_add
 *
 * Adds a new vPort (vPort policy) for one classification device instance
 * identified by macsec_side parameter.
 *
 * Note: If a vPort is added for an SA then it must be added to the same SecY
 *       device (macsec_side) where the SA was added via the imacsec_plp_base_t_secy_sa_add() function.
 *
 * API use order:
 *       A vPort can be added to a classification device instance only after this
 *       device has been initialized via the imacsec_plp_base_t_secy_init() function.
 *
 * @param unit (input) BCM unit number
 * @param port (input) - port number
 * @param macsec_side (input) - selects egress or ingress path
 *  0 - egress path
 *  1 - ingress path
 * @param vport_handle_p (output)
 *      Pointer to the memory location where the vPort handle will be stored.
 *
 * This function is NOT re-entrant for the same vPort of the same macsec_side.
 * This function is re-entrant for different macsec_side's or different vPorts
 * of the same macsec_side.
 *
 * This function can be called concurrently with any other secy API
 * function for the same or different macsec_side provided that the API use order
 * is followed.
 *
 * @return
 *     BCM_PLP_BASE_T_SECY_STATUS_OK : success
 *     BCM_PLP_BASE_T_SECY_ERROR_BAD_PARAMETER : incorrect input parameter
 *     BCM_PLP_BASE_T_SECY_INTERNAL_ERROR : failure
 */
int imacsec_plp_base_t_secy_vport_add(int unit, soc_port_t port, unsigned int macsec_side,
                                    bcm_plp_base_t_secy_vport_handle_t *vport_handle_p)
{
    phy_ctrl_t  *pc;
    int rv=0;
    bcm_plp_base_t_sec_access_t *pa;
    EXT_PHY_INIT_CHECK(unit,port);
    pc = EXT_PHY_SW_STATE(unit, port);
    IMACSEC_PLP_BASE_T_SEC_ACCESS_GET(pc,pa,macsec_side);
    rv = bcm_plp_base_t_secy_vport_add(pa,vport_handle_p);
    if(rv !=0){
        LOG_CLI((BSL_META_U(unit,"bcm_plp_base_t_secy_vport_add  failed:%d\n "), rv));
    }
    return rv;
}

/*! \brief imacsec_plp_base_t_secy_vport_remove
 *
 * Removes an already added vPort (vPort policy) from one classification device
 * instance identified by macsec_side parameter.
 *
 * API use order:
 *       A vPort can be removed from a classification device instance only after
 *       this vPort has been added to the device via the imacsec_plp_base_t_secy_vport_add()
 *       function.
 *
 *       Before this function is called all the rules associated with
 *       this vPort must be removed.
 *
 * @param unit (input) BCM unit number
 * @param port (input) - port number
 * @param macsec_side (input) - selects egress or ingress path
 *  0 - egress path
 *  1 - ingress path
 * @param vport_handle (input)
 *      vPort handle for vPort to be removed.
 *
 * This function is NOT re-entrant for the same vPort of the same macsec_side.
 * This function is re-entrant for different macsec_side's or different vPorts
 * of the same macsec_side.
 *
 * This function cannot be called concurrently with imacsec_plp_base_t_secy_rule_add() and
 * imacsec_plp_base_t_secy_rule_remove() functions for the same macsec_side.
 * This function can be called concurrently with any other secy API
 * function for the same or different macsec_side provided that the API use order
 * is followed.
 *
 * @return
 *     BCM_PLP_BASE_T_SECY_STATUS_OK : success
 *     BCM_PLP_BASE_T_SECY_ERROR_BAD_PARAMETER : incorrect input parameter
 *     BCM_PLP_BASE_T_SECY_INTERNAL_ERROR : failure
 */

int imacsec_plp_base_t_secy_vport_remove(int unit, soc_port_t port, unsigned int macsec_side,
                                      const bcm_plp_base_t_secy_vport_handle_t vport_handle)
{
    phy_ctrl_t  *pc;
    int rv=0;
    bcm_plp_base_t_sec_access_t *pa;
    EXT_PHY_INIT_CHECK(unit,port);
    pc = EXT_PHY_SW_STATE(unit, port);
    IMACSEC_PLP_BASE_T_SEC_ACCESS_GET(pc,pa,macsec_side);
    rv = bcm_plp_base_t_secy_vport_remove(pa,vport_handle);
    if(rv !=0){
        LOG_CLI((BSL_META_U(unit,"imacsec_plp_base_t_secy_vport_remove  failed:%d\n "), rv));
    }
    return rv;
}

/*! \brief imacsec_plp_base_t_secy_uninit
 *
 * Uninitializes a SecY device instance identified by macsec_side parameter.
 *
 * API use order:
 *       This function must be called when the SecY device for this macsec_side
 *       is no longer needed. After this function is called no other imacsec_plp_base_t_secy_*()
 *       or imacsec_plp_base_t_secy_*() functions may be called for this macsec_side except the
 *       imacsec_plp_base_t_secy_init() function.
 *
 * @param unit (input) BCM unit number
 * @param port (input) - port number
 * @param macsec_side (input) - selects egress or ingress path
 *  0 - egress path
 *  1 - ingress path
 * This function is NOT re-entrant for the same macsec_side.
 * This function is re-entrant for different macsec_side's.
 *
 * This function cannot be called concurrently with any other SecY
 * API function for the same macsec_side.
 * This function can be called concurrently with any other SecY API
 * function for the different macsec_side.
 *
 * @return
 *     BCM_PLP_BASE_T_SECY_STATUS_OK : success
 *     BCM_PLP_BASE_T_SECY_ERROR_BAD_PARAMETER : incorrect input parameter
 */

int imacsec_plp_base_t_secy_uninit(int unit, soc_port_t port, unsigned int macsec_side)
{
    phy_ctrl_t  *pc;
    int rv=0;
    bcm_plp_base_t_sec_access_t *pa;
    EXT_PHY_INIT_CHECK(unit,port);
    pc = EXT_PHY_SW_STATE(unit, port);
    IMACSEC_PLP_BASE_T_SEC_ACCESS_GET(pc,pa,macsec_side);
    rv = bcm_plp_base_t_secy_device_uninit(pa);
    if(rv !=0){
        LOG_CLI((BSL_META_U(unit,"imacsec_plp_base_t_secy_uninit  failed:%d\n "), rv));
    }
    return rv;
}
/*! \brief imacsec_plp_base_t_secy_sa_add
 *
 * Adds a new SA for a SecY device instance identified by macsec_side parameter.
 *
 * @param unit (input) BCM unit number
 * @param port (input) - port number
 * @param macsec_side (input) - selects egress or ingress path
 *  0 - egress path
 *  1 - ingress path
 * @param vport_handle (input)
 *      vPort handle for the vPort where the new SA must be added.
 *
 * @param sa_handle_p (output)
 *      Placeholder where the SA handle will be stored. It is used
 *      in the bcm_plp_base_t_secy_sa*() function calls for this macsec_side.
 *
 * @param sa_p (input)
 *      Pointer to the memory location where the data for the new SA is stored.
 *
 * This function is NOT re-entrant for the same macsec_side.
 * This function is re-entrant for different macsec_side's.
 *
 * This function cannot be called concurrently with the imacsec_plp_base_t_secy_sa_remove()
 * function for the same macsec_side.
 * This function can be called concurrently with any other SecY API
 * function for the same or different macsec_side.
 *
 * @return
 *     BCM_PLP_BASE_T_SECY_STATUS_OK : success
 *     BCM_PLP_BASE_T_SECY_ERROR_BAD_PARAMETER : incorrect input parameter
 *     BCM_PLP_BASE_T_SECY_INTERNAL_ERROR : failure
 */

int imacsec_plp_base_t_secy_sa_add(int unit, soc_port_t port, unsigned int macsec_side,
                                 const bcm_plp_base_t_secy_vport_handle_t vport_handle,
                                 bcm_plp_base_t_secy_sa_handle_t *sa_handle_p,
                                 const bcm_plp_base_t_secy_sa_t *sa_p)
{
    phy_ctrl_t  *pc;
    int rv=0;
    bcm_plp_base_t_sec_access_t *pa;
    EXT_PHY_INIT_CHECK(unit,port);
    pc = EXT_PHY_SW_STATE(unit, port);
    IMACSEC_PLP_BASE_T_SEC_ACCESS_GET(pc,pa,macsec_side);
    rv = bcm_plp_base_t_secy_sa_add(pa,vport_handle,sa_handle_p,sa_p);
    if(rv !=0){
        LOG_CLI((BSL_META_U(unit,"imacsec_plp_base_t_secy_sa_add  failed:%d\n "), rv));
    }
    return rv;
}
/*! \brief imacsec_plp_base_t_secy_sa_update
 *
 * Updates SA flow parameters for an already added SA for a SecY device
 * instance identified by macsec_side parameter.
 *
 * Note: This function does not update the SA transform record data and it
 *       cannot be used to update the key in the transform record.
 *
 * API use order:
 *      This function may be called for the sa_handle obtained via
 *      the imacsec_plp_base_t_secy_sa_add() function for the same macsec_side.
 *
 * @param unit (input) BCM unit number
 * @param port (input) - port number
 * @param macsec_side (input) - selects egress or ingress path
 *  0 - egress path
 *  1 - ingress path
 * @param sa_handle (input)
 *      SA handle of the SA to be updated.
 *
 * @param sa_p (input)
 *      Pointer to the memory location where the new parameters for the SA are
 *      stored.
 *
 * This function is re-entrant for the same of different macsec_side's.
 *
 * This function cannot be called concurrently with the imacsec_plp_base_t_secy_sa_remove()
 * function for the same macsec_side.
 * This function can be called concurrently with any other SecY API
 * function for the same or different macsec_side.
 *
 * @return
 *     BCM_PLP_BASE_T_SECY_STATUS_OK : success
 *     BCM_PLP_BASE_T_SECY_ERROR_BAD_PARAMETER : incorrect input parameter
 *     BCM_PLP_BASE_T_SECY_INTERNAL_ERROR : failure
 */

int imacsec_plp_base_t_secy_sa_update(int unit, soc_port_t port, unsigned int macsec_side,
                                    const bcm_plp_base_t_secy_sa_handle_t sa_handle,
                                    const bcm_plp_base_t_secy_sa_t *sa_p)
{
    phy_ctrl_t  *pc;
    int rv=0;
    bcm_plp_base_t_sec_access_t *pa;
    EXT_PHY_INIT_CHECK(unit,port);
    pc = EXT_PHY_SW_STATE(unit, port);
    IMACSEC_PLP_BASE_T_SEC_ACCESS_GET(pc,pa,macsec_side);
    rv=bcm_plp_base_t_secy_sa_update(pa,sa_handle,sa_p);
    if(rv !=0){
        LOG_CLI((BSL_META_U(unit,"imacsec_plp_base_t_secy_sa_update  failed:%d\n "), rv));
    }
    return rv;
}
/*! \brief imacsec_plp_base_t_secy_sa_read
 *
 * Reads (part of) a transform record of an already added SA from a SecY
 * device instance identified by macsec_side parameter.
 *
 * API use order:
 *      This function may be called for the sa_handle obtained via the
 *      imacsec_plp_base_t_secy_sa_add() function for the same macsec_side.
 *
 * @param unit (input) BCM unit number
 * @param port (input) - port number
 * @param macsec_side (input) - selects egress or ingress path
 *  0 - egress path
 *  1 - ingress path
 * @param sa_handle (input)
 *      SA handle of the SA to be read.
 *
 * @param word_offset (input)
 *      Word offset in transform record where the data should be read from.
 *
 * @param word_count (input)
 *      Number of words which must be read from the SA transform record.
 *
 * @param transform_p (output)
 *      Pointer to the memory location where the word_count 32-bit words
 *      of the SA transform record will be stored.
 *
 * This function is re-entrant.
 *
 * This function can be called concurrently with any other SecY API
 * function for the same or different sa_handle of the same or different
 * macsec_side provided that the user order of the API is followed.
 *
 * @return
 *     BCM_PLP_BASE_T_SECY_STATUS_OK : success
 *     BCM_PLP_BASE_T_SECY_ERROR_BAD_PARAMETER : incorrect input parameter
 *     BCM_PLP_BASE_T_SECY_INTERNAL_ERROR : failure
 */

int imacsec_plp_base_t_secy_sa_read(int unit, soc_port_t port, unsigned int macsec_side,
                                  const bcm_plp_base_t_secy_sa_handle_t sa_handle,
                                  const unsigned int word_offset,
                                  const unsigned int word_count,
                                  unsigned int * transform_p)

{
    phy_ctrl_t  *pc;
    int rv=0;
    bcm_plp_base_t_sec_access_t *pa;
    EXT_PHY_INIT_CHECK(unit,port);
    pc = EXT_PHY_SW_STATE(unit, port);
    IMACSEC_PLP_BASE_T_SEC_ACCESS_GET(pc,pa,macsec_side);
    rv=bcm_plp_base_t_secy_sa_read(pa,sa_handle,word_offset,word_count,transform_p);
    if(rv !=0){
        LOG_CLI((BSL_META_U(unit,"imacsec_plp_base_t_secy_sa_read  failed:%d\n "), rv));
    }
    return rv;
}
/*! \brief imacsec_plp_base_t_secy_sa_remove
 *
 * Removes a previously added SA from a SecY device instance identified by
 * macsec_side parameter.
 *
 * @param unit (input) BCM unit number
 * @param port (input) - port number
 * @param macsec_side (input) - selects egress or ingress path
 *  0 - egress path
 *  1 - ingress path
 * @param sa_handle (input)
 *      SA handle of the SA to be removed. After this function returns this
 *      handle should not be used anymore with the imacsec_plp_base_t_secy_sa*() functions.
 *
 * This function is NOT re-entrant for the same macsec_side.
 * This function is re-entrant for different macsec_side's.
 *
 * This function cannot be called concurrently with the imacsec_plp_base_t_secy_sa_add()
 * function for the same macsec_side.
 * This function cannot be called concurrently with the imacsec_plp_base_t_secy_sa_read()
 * function for the same sa_handle.
 * This function can be called concurrently with any other SecY API
 * function for the same or different macsec_side.
 *
 * @return
 *     BCM_PLP_BASE_T_SECY_STATUS_OK : success
 *     BCM_PLP_BASE_T_SECY_ERROR_BAD_PARAMETER : incorrect input parameter
 *     BCM_PLP_BASE_T_SECY_INTERNAL_ERROR : failure
 */

int imacsec_plp_base_t_secy_sa_remove(int unit, soc_port_t port, unsigned int macsec_side,
                                    const bcm_plp_base_t_secy_sa_handle_t sa_handle
                                    )
{
    phy_ctrl_t  *pc;
    int rv=0;
    bcm_plp_base_t_sec_access_t *pa;
    EXT_PHY_INIT_CHECK(unit,port);
    pc = EXT_PHY_SW_STATE(unit, port);
    IMACSEC_PLP_BASE_T_SEC_ACCESS_GET(pc,pa,macsec_side);
    rv = bcm_plp_base_t_secy_sa_remove(pa, sa_handle);
    if(rv !=0){
        LOG_CLI((BSL_META_U(unit,"imacsec_plp_base_t_secy_sa_remove failed:%d\n "), rv));
    }
    return rv;
}
/*! \brief imacsec_plp_base_t_secy_bypass_set
 *
 * Configures port for low-latency bypass.
 *
 * API use order:
 *       This function can be called for a SecY device only after this
 *       device has been initialized via the imacsec_plp_base_t_secy_device_init() function.
 *
 * @param unit (input) BCM unit number
 * @param port (input) - port number
 * @param macsec_side (input) - selects egress or ingress path
 *  0 - egress path
 *  1 - ingress path
 * @param f_static_bypass (input)
 *      Pointer to a unsigned char indication in which the setting must be returned
 *      true if channel is set to static bypass, false if channel is
 *      set for normal operation.
 *
 * @param f_bypass_no_class (input)
 *      Pointer to a unsigned char indication in which the setting must be returned
 *      true if channel is set to bypass no class, false if channel is
 *      set for normal operation.
 *
 * @return
 *     BCM_PLP_BASE_T_SECY_STATUS_OK : success
 *     BCM_PLP_BASE_T_SECY_ERROR_BAD_PARAMETER : incorrect input parameter
 */
int imacsec_plp_base_t_secy_bypass_set(int unit, soc_port_t port, unsigned int macsec_side,
                                    const unsigned char f_static_bypass,
                                    const unsigned char f_bypass_no_class)
{
    phy_ctrl_t  *pc;
    int rv=0;
    bcm_plp_base_t_sec_access_t *pa;
    EXT_PHY_INIT_CHECK(unit,port);
    pc = EXT_PHY_SW_STATE(unit, port);
    IMACSEC_PLP_BASE_T_SEC_ACCESS_GET(pc,pa,macsec_side);
    rv = bcm_plp_base_t_secy_bypass_set(pa, f_static_bypass,f_bypass_no_class);
    if(rv !=0){
        LOG_CLI((BSL_META_U(unit,"imacsec_plp_base_t_secy_bypass_set failed:%d\n "), rv));
    }
    return rv;
}
/*! \brief imacsec_plp_base_t_secy_bypass_get
 *
 * Read the current port low-latency bypass setting.
 *
 * API use order:
 *       This function can be called for a SecY device only after this
 *       device has been initialized via the imacsec_plp_base_t_secy_init() function.
 *
 * @param unit (input) BCM unit number
 * @param port (input) - port number
 * @param macsec_side (input) - selects egress or ingress path
 *  0 - egress path
 *  1 - ingress path
 * @param f_static_bypass (output)
 *      Pointer to an unsigned char in which the setting must be returned
 *      true if channel is set to static bypass, false if channel is
 *      set for normal operation.
 *
 * @param f_bypass_no_class (output)
 *      Pointer to a unsigned char in which the setting must be returned
 *      true if channel is set to bypass no class, false if channel is
 *      set for normal operation.
 *
 * @return
 *     BCM_PLP_BASE_T_SECY_STATUS_OK : success
 *     BCM_PLP_BASE_T_SECY_ERROR_BAD_PARAMETER : incorrect input parameter
 */

int imacsec_plp_base_t_secy_bypass_get(int unit, soc_port_t port, unsigned int macsec_side,
                                    unsigned char *f_static_bypass,
                                    unsigned char *f_bypass_no_class)
{
    phy_ctrl_t  *pc;
    int rv=0;
    bcm_plp_base_t_sec_access_t *pa;
    EXT_PHY_INIT_CHECK(unit,port);
    pc = EXT_PHY_SW_STATE(unit, port);
    IMACSEC_PLP_BASE_T_SEC_ACCESS_GET(pc,pa,macsec_side);
    rv = bcm_plp_base_t_secy_bypass_get(pa, f_static_bypass,f_bypass_no_class);
    if(rv !=0){
        LOG_CLI((BSL_META_U(unit,"imacsec_plp_base_t_secy_bypass_get failed:%d\n "), rv));
    }  
    return rv;
}
/*! \brief imacsec_plp_base_t_secy_rule_add
 *
 * Adds a new rule for matching a packet to a vPort identified by vport_handle
 * for one classification device instance identified by macsec_side parameter.
 *
 * Note: If a rule is added for a vPort then it must be added to the same
 *       device (macsec_side) where the vPort was added via the imacsec_plp_base_t_secy_vport_add()
 *       function.
 *
 * API use order:
 *       A rule can be added to a classification device instance only after
 *       the vPort identified by vport_handle has been added to this device
 *       via the imacsec_plp_base_t_secy_vport_add() function.
 *
 *
 * @param unit (input) BCM unit number
 * @param port (input) - port number
 * @param macsec_side (input) - selects egress or ingress path
 *  0 - egress path
 *  1 - ingress path
 *
 *
 * @param vport_handle (input)
 *      vPort handle for the vPort where the packet matching rule must be added.
 *
 * @param rule_handle_p (output)
 *      Pointer to the memory location where where the rule handle will be stored.
 *
 * @param rule_p (input)
 *      Pointer to the memory location where the data for the rule is stored.
 *
 * This function is NOT re-entrant for the same rule of the same macsec_side.
 * This function is re-entrant for different macsec_side's or different rules
 * of the same macsec_side.
 *
 * This function cannot be called concurrently with imacsec_plp_base_t_secy_vport_remove() and
 * imacsec_plp_base_t_secy_rule_remove() functions for the same macsec_side.
 * This function can be called concurrently with any other secy API
 * function for the same or different macsec_side provided that the API use order
 * is followed.
 *
 * @return
 *     BCM_PLP_BASE_T_SECY_STATUS_OK : success
 *     BCM_PLP_BASE_T_SECY_ERROR_BAD_PARAMETER : incorrect input parameter
 *     BCM_PLP_BASE_T_SECY_INTERNAL_ERROR : failure
 */

int imacsec_plp_base_t_secy_rule_add(int unit, soc_port_t port, unsigned int macsec_side,
                                   const bcm_plp_base_t_secy_vport_handle_t vport_handle,
                                   bcm_plp_base_t_secy_rule_handle_t *rule_handle_p,
                                   const bcm_plp_base_t_secy_rule_t *rule_p)
{
    phy_ctrl_t  *pc;
    int rv=0;
    bcm_plp_base_t_sec_access_t *pa;
    EXT_PHY_INIT_CHECK(unit,port);
    pc = EXT_PHY_SW_STATE(unit, port);
    IMACSEC_PLP_BASE_T_SEC_ACCESS_GET(pc,pa,macsec_side);
    rv=bcm_plp_base_t_secy_rule_add(pa,vport_handle,rule_handle_p,rule_p);
    if(rv !=0){
        LOG_CLI((BSL_META_U(unit,"imacsec_plp_base_t_secy_rule_add failed:%d\n "), rv));
    }
    return rv;
}
/*! \brief imacsec_plp_base_t_secy_rule_remove
 *
 * Removes an already added rule from one classification device
 * instance identified by macsec_side parameter.
 *
 * API use order:
 *       A rule can be removed from a classification device instance only after
 *       this rule has been added to the device via the imacsec_plp_base_t_secy_rule_add()
 *       function.
 *
 * @param unit (input) BCM unit number
 * @param port (input) - port number
 * @param macsec_side (input) - selects egress or ingress path
 *  0 - egress path
 *  1 - ingress path
 * 
 *
 * @param rule_handle (input)
 *      Rule handle for the rule to be removed.
 *
 * This function is NOT re-entrant for the same rule of the same macsec_side.
 * This function is re-entrant for different macsec_side's or different rules
 * of the same macsec_side.
 *
 * This function cannot be called concurrently with imacsec_plp_base_t_secy_vport_remove() and
 * imacsec_plp_base_t_secy_rule_add() functions for the same macsec_side.
 * This function can be called concurrently with any other secy API
 * function for the same or different macsec_side provided that the API use order
 * is followed.
 *
 * @return
 *     BCM_PLP_BASE_T_SECY_STATUS_OK : success
 *     BCM_PLP_BASE_T_SECY_ERROR_BAD_PARAMETER : incorrect input parameter
 *     BCM_PLP_BASE_T_SECY_INTERNAL_ERROR : failure
 */

int imacsec_plp_base_t_secy_rule_remove(int unit, soc_port_t port, unsigned int macsec_side,
                                      const bcm_plp_base_t_secy_rule_handle_t rule_handle)
{
    phy_ctrl_t  *pc;
    int rv=0;
    bcm_plp_base_t_sec_access_t *pa;
    EXT_PHY_INIT_CHECK(unit,port);
    pc = EXT_PHY_SW_STATE(unit, port);
    IMACSEC_PLP_BASE_T_SEC_ACCESS_GET(pc,pa,macsec_side);
    rv = bcm_plp_base_t_secy_rule_remove(pa,rule_handle);
    if(rv !=0){
        LOG_CLI((BSL_META_U(unit,"imacsec_plp_base_t_secy_rule_remove failed:%d\n "), rv));
    } 
    return rv;
}
/*! \brief imacsec_plp_base_t_secy_rule_update
 *
 * Updates a packet matching rule for one classification device instance
 * identified by macsec_side parameter.
 *
 * Note: A rule must be updated to the same device (macsec_side) where
 *       the corresponding vPort was added via the imacsec_plp_base_t_secy_vport_add() function.
 *
 * API use order:
 *       A rule can be updated for a classification device instance only after
 *       the corresponding vPort has been added to this device
 *       via the imacsec_plp_base_t_secy_vport_add() function.
 *
 * @param unit (input) BCM unit number
 * @param port (input) - port number
 * @param macsec_side (input) - selects egress or ingress path
 *  0 - egress path
 *  1 - ingress path
 * 
 * @param rule_handle (input)
 *      Rule handle for rule to be enabled.
 *
 * @param rule_p (input)
 *      Pointer to the memory location where the data for the rule is stored.
 *
 * This function is NOT re-entrant for the same rule of the same macsec_side.
 * This function is re-entrant for different macsec_side's or different rules
 * of the same macsec_side.
 *
 * This function can be called concurrently with any other secy API
 * function for the same or different macsec_side provided that the API use order
 * is followed.
 *
 * @return
 *     BCM_PLP_BASE_T_SECY_STATUS_OK : success
 *     BCM_PLP_BASE_T_SECY_ERROR_BAD_PARAMETER : incorrect input parameter
 *     BCM_PLP_BASE_T_SECY_INTERNAL_ERROR : failure
 */

int imacsec_plp_base_t_secy_rule_update(int unit, soc_port_t port, unsigned int macsec_side,
                                      const bcm_plp_base_t_secy_rule_handle_t rule_handle,
                                      const bcm_plp_base_t_secy_rule_t *rule_p)
{
    phy_ctrl_t  *pc;
    int rv=0;
    bcm_plp_base_t_sec_access_t *pa;
    EXT_PHY_INIT_CHECK(unit,port);
    pc = EXT_PHY_SW_STATE(unit, port);
    IMACSEC_PLP_BASE_T_SEC_ACCESS_GET(pc,pa,macsec_side);
    rv = bcm_plp_base_t_secy_rule_update(pa,rule_handle,rule_p);
    if(rv !=0){
        LOG_CLI((BSL_META_U(unit,"imacsec_plp_base_t_secy_rule_update failed:%d\n "), rv));
    }
    return rv;
}

/*! \brief imacsec_plp_base_t_secy_rule_enable
 *
 * Enables an already added rule for one classification device instance
 * identified by macsec_side parameter.
 *
 * API use order:
 *       A rule can be enabled after it has been added
 *       via the secy API imacsec_plp_base_t_secy_rule_add() function or updated via
 *       the imacsec_plp_base_t_secy_rule_updated() function for the same macsec_side.
 *
 * @param unit (input) BCM unit number
 * @param port (input) - port number
 * @param macsec_side (input) - selects egress or ingress path
 *  0 - egress path
 *  1 - ingress path
 * 
 * @param rule_handle (input)
 *      Rule handle for rule to be enabled.
 *
 * @param fsync (input)
 *      If true then this function will ensure that all the packets
 *      available in the device at the time of the function call are processed
 *      before the rule is enabled.
 *
 * This function is NOT re-entrant for the same Device when fsync is true.
 * This function is re-entrant for the same Device when fsync is false.
 * This function is re-entrant for different Devices.
 *
 * This function can be called concurrently with any other secy API function
 * for the same or different rule_handle of the same Device or
 * different Devices provided that the API use order is followed.
 *
 * If fsync is set to true then this function cannot be called concurrently
 * with any other function of the secy API which also takes fsync parameter set
 * to true for the same Device.
 *
 * @return
 *     BCM_PLP_BASE_T_SECY_STATUS_OK : success
 *     BCM_PLP_BASE_T_SECY_ERROR_BAD_PARAMETER : incorrect input parameter
 *     BCM_PLP_BASE_T_SECY_INTERNAL_ERROR : failure
 */

int imacsec_plp_base_t_secy_rule_enable(int unit, soc_port_t port, unsigned int macsec_side,
                                      const bcm_plp_base_t_secy_rule_handle_t rule_handle,
                                      const unsigned char fsync)
{
    phy_ctrl_t  *pc;
    int rv=0;
    bcm_plp_base_t_sec_access_t *pa;
    EXT_PHY_INIT_CHECK(unit,port);
    pc = EXT_PHY_SW_STATE(unit, port);
    IMACSEC_PLP_BASE_T_SEC_ACCESS_GET(pc,pa,macsec_side);
    rv = bcm_plp_base_t_secy_rule_enable(pa,rule_handle,fsync);
    if(rv !=0){
        LOG_CLI((BSL_META_U(unit,"imacsec_plp_base_t_secy_rule_enable failed:%d\n "), rv));
    }  
    return rv;
}
/*! \brief imacsec_plp_base_t_secy_rule_disable
 *
 * Disables an already added rule for one classification device instance
 * identified by macsec_side parameter.
 *
 * API use order:
 *       A rule can be enabled after it has been added
 *       via the secy API imacsec_plp_base_t_secy_rule_add() function or updated via
 *       the imacsec_plp_base_t_secy_rule_update() function for the same macsec_side.
 *
 * @param unit (input) BCM unit number
 * @param port (input) - port number
 * @param macsec_side (input) - selects egress or ingress path
 *  0 - egress path
 *  1 - ingress path
 * 
 * @param rule_handle (input)
 *      Rule handle for rule to be enabled.
 *
 * @param fsync (input)
 *      If true then this function will ensure that all the packets
 *      available in the device at the time of the function call are processed
 *      before the rule is disabled.
 *
 * This function is NOT re-entrant for the same Device when fsync is true.
 * This function is re-entrant for the same Device when fsync is false.
 * This function is re-entrant for different Devices.
 *
 * This function can be called concurrently with any other secy API function
 * for the same or different rule_handle of the same Device or
 * different Devices provided that the API use order is followed.
 *
 * If fsync is set to true then this function cannot be called concurrently
 * with any other function of the secy API which also takes fsync parameter set
 * to true for the same Device.
 *
 * @return
 *     BCM_PLP_BASE_T_SECY_STATUS_OK : success
 *     BCM_PLP_BASE_T_SECY_ERROR_BAD_PARAMETER : incorrect input parameter
 *     BCM_PLP_BASE_T_SECY_INTERNAL_ERROR : failure
 */

int imacsec_plp_base_t_secy_rule_disable(int unit, soc_port_t port, unsigned int macsec_side,
                                      const bcm_plp_base_t_secy_rule_handle_t rule_handle,
                                      const unsigned char fsync)
{
    phy_ctrl_t  *pc;
    int rv=0;
    bcm_plp_base_t_sec_access_t *pa;
    EXT_PHY_INIT_CHECK(unit,port);
    pc = EXT_PHY_SW_STATE(unit, port);
    IMACSEC_PLP_BASE_T_SEC_ACCESS_GET(pc,pa,macsec_side);
    rv = bcm_plp_base_t_secy_rule_disable(pa,rule_handle,fsync);
    if(rv !=0){
        LOG_CLI((BSL_META_U(unit,"imacsec_plp_base_t_secy_rule_disable failed:%d\n "), rv));
    }
    return rv;
}
/*! \brief imacsec_plp_base_t_secy_rule_enable_disable
 *
 * Enables and/or disables an already added rule from one Classification device
 * instance identified by macsec_side parameter. Can also enable or disable all
 * rules for this device instance at once.
 *
 * API use order:
 *       An rule can be enabled or disabled after its transform record is
 *       installed via the SecY API imacsec_plp_base_t_secy_sa_add() function and the rule is
 *       added via the imacsec_plp_base_t_secy_rule_add() function for the same macsec_side.
 *
 * @param unit (input) BCM unit number
 * @param port (input) - port number
 * @param macsec_side (input) - selects egress or ingress path
 *  0 - egress path
 *  1 - ingress path
 *
 * @param rule_handle_enable (input)
 *      Rule handle for rule to be enabled. Will be ignored if equal to
 *      bcm_plp_base_t_secy_rule_handle_NULL.
 *
 * @param rule_handle_disable (input)
 *      Rule handle for rule to be disabled. Will be ignored if equal to
 *      bcm_plp_base_t_secy_rule_handle_NULL.
 *
 * @param enable_all (input)
 *      When set to true all rules will be enabled. Takes precedence over the
 *      other parameters.
 *
 * @param disable_all (input)
 *      When set to true all rules will be disabled. Takes precedence over the
 *      other parameters, except enable_all.
 *
 * @param fsync (input)
 *      If true then this function will ensure that all the packets
 *      available in the device at the time of the function call are processed
 *      before the rule is enabled or disabled or both.
 *
 * This function is NOT re-entrant for the same Device when fsync is true.
 * This function is re-entrant for the same Device when fsync is false.
 * This function is re-entrant for different Devices.
 *
 * This function can be called concurrently with any other secy API function
 * for the same or different rule_handles of the same Device or
 * different Devices provided that the API use order is followed.
 *
 * If fsync is set to true then this function cannot be called concurrently
 * with any other function of the secy API which also takes fsync parameter set
 * to true for the same Device.
 *
 * @return
 *     BCM_PLP_BASE_T_SECY_STATUS_OK : success
 *     BCM_PLP_BASE_T_SECY_ERROR_BAD_PARAMETER : incorrect input parameter
 *     BCM_PLP_BASE_T_SECY_INTERNAL_ERROR : failure
 */
int imacsec_plp_base_t_secy_rule_enable_disable(int unit, soc_port_t port, unsigned int macsec_side,
                                              const bcm_plp_base_t_secy_rule_handle_t rule_handle_disable,
                                              const bcm_plp_base_t_secy_rule_handle_t rule_handle_enable,
                                              const unsigned char enable_all,
                                              const unsigned char disable_all,
                                              const unsigned char fsync)
{
    phy_ctrl_t  *pc;
    int rv=0;
    bcm_plp_base_t_sec_access_t *pa;
    EXT_PHY_INIT_CHECK(unit,port);
    pc = EXT_PHY_SW_STATE(unit, port);
    IMACSEC_PLP_BASE_T_SEC_ACCESS_GET(pc,pa,macsec_side);
    rv = bcm_plp_base_t_secy_rule_enable_disable(pa,rule_handle_disable,rule_handle_enable,enable_all,disable_all,fsync);
    if(rv !=0){
        LOG_CLI((BSL_META_U(unit,"imacsec_plp_base_t_secy_rule_enable_disable failed:%d\n "), rv));
    }
    return rv;
}

/*! \brief imacsec_plp_base_t_version_get
 * 
 * Get the IMACSEC software version information
 *
 * @param unit (input) BCM unit number
 * @param port (input) - port number
 * @param version_info (output) -  Get the version information
 *    version_info structure has two member variable
 *        major_no  - Indicates major software release
 *        minor_no  - Indicates enhancement after major software release
 * @return
 *   return 0 on success
 */

int imacsec_plp_base_t_version_get(int unit,soc_port_t port,bcm_plp_base_t_version_t* version_info)
{
    int rv=0;
    phy_ctrl_t  *pc;
    bcm_plp_base_t_sec_access_t *pa;
    EXT_PHY_INIT_CHECK(unit,port);
    pc = EXT_PHY_SW_STATE(unit, port);
    pa = (bcm_plp_base_t_sec_access_t *)(((char *)pc->driver_data + pc->size) - sizeof(bcm_plp_base_t_sec_access_t));

    if(pa==NULL){
	    return SOC_E_INTERNAL;
    }

    rv = bcm_plp_base_t_imacsec_version_get(pa, version_info);

    if(rv !=0){
        LOG_CLI((BSL_META_U(unit,"imacsec_plp_base_t_version_get failed:%d\n "), rv));
    }
    return rv;
}


/*! \brief imacsec_plp_addr_read
 * 
 * Reads the register address using mdio bus
 *
 * @param unit (input) BCM unit number
 * @param port (input) - port number
 * @param reg_addr (input)  - Register address
 * @param value (output) -  register value read
 *
 * @return
 *   return 0 on success
 */

int imacsec_plp_addr_read(int unit,
                          unsigned int port,
                          unsigned int reg_addr,
                          unsigned int *value)
{
    phy_ctrl_t  *pc;
    bcm_plp_base_t_sec_access_t *pa;
    EXT_PHY_INIT_CHECK(unit,port);
    pc = EXT_PHY_SW_STATE(unit, port);
    pa = (bcm_plp_base_t_sec_access_t *)(((char *)pc->driver_data + pc->size) - sizeof(bcm_plp_base_t_sec_access_t));
    if(pa==NULL){
	    return SOC_E_INTERNAL;
    }
    pa->phy_info.phy_addr= pa->macsec_dev_addr;
    *value = 0xffffffff;
    return plp_raw_read(&(pa->phy_info),reg_addr,value);
}
/*! \brief imacsec_plp_addr_write
 *
 *  write the register address using mdio bus
 *
 * @param unit (input) BCM unit number
 * @param port (input) - port number
 * @param reg_addr (input)  - Register address
 * @param value (input) -  register value to write
 * @return
 *   return 0 on success
 */

int imacsec_plp_addr_write(int unit,
                           unsigned int port,
                           unsigned int reg_addr,
                           unsigned int value)
{
    phy_ctrl_t  *pc;
    bcm_plp_base_t_sec_access_t *pa;
    EXT_PHY_INIT_CHECK(unit,port);
    pc = EXT_PHY_SW_STATE(unit, port);
    pa = (bcm_plp_base_t_sec_access_t *)(((char *)pc->driver_data + pc->size) - sizeof(bcm_plp_base_t_sec_access_t));
    if(pa==NULL){
	    return SOC_E_INTERNAL;
    }
    pa->phy_info.phy_addr= pa->macsec_dev_addr;
    return plp_raw_write(&(pa->phy_info),reg_addr,value);
}
#endif /* INCLUDE_PLP_IMACSEC */
