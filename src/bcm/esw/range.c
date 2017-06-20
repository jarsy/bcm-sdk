/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    range.c
 * Purpose: Manages RANGE_CHECKER (IFP_RANGE_CHECK)
 *
 */

#include <bcm/range.h>
#include <bcm/error.h>

#include <soc/defs.h>

#include <bcm_int/esw/range.h>

/* Function : bcm_esw_range_init
 *
 * Purpose  : Initialize range module
 *
 * Parameters : unit - (IN) Unit number.
 *
 * Returns:
 *          BCM_E_NONE     : Range Module initialized successfully.
 *          BCM_E_UNIT     : Invalid BCM Unit number.
 *          BCM_E_UNAVAIL  : Feature not supported.
 *          BCM_E_XXX      : Standard error code.
 */
int 
bcm_esw_range_init( int unit ) { 

#if defined (BCM_TOMAHAWK_SUPPORT)
    return bcmi_xgs5_range_init(unit);
#endif

    return BCM_E_UNAVAIL;
}

/* Function : bcm_esw_range_detach
 *
 * Purpose  : Detach range module
 *
 * Parameters : unit - (IN) Unit number.
 *
 * Returns:
 *          BCM_E_NONE     : Range Module detached successfully.
 *          BCM_E_UNIT     : Invalid BCM Unit number.
 *          BCM_E_UNAVAIL  : Feature not supported.
 *          BCM_E_INIT     : Range module not initialized.
 *          BCM_E_XXX      : Standard error code.
 */
int 
bcm_esw_range_detach( int unit ) { 

#if defined (BCM_TOMAHAWK_SUPPORT)
    return bcmi_xgs5_range_detach(unit);
#endif

    return BCM_E_UNAVAIL;
}

/* Function : bcm_esw_range_create
 *
 * Purpose  : This API is to create a Layer 4 TCP/UDP port range checker 
 *            or a VLAN ranger checker or a packet length range checker or 
 *            a UDF range checker and returns an identifier that can be 
 *            passed to "bcm_field_qualify_RangeCheck" API, to qualify 
 *            a Field entry with Range Checker match criteria.
 *
 * Parameters : unit - (IN) Unit number.
 *              flags - (IN) Flags to Range allocator
 *              range_config - (IN) Range Config Structure
 *
 * Returns:
 *          BCM_E_NONE     : Range Module created successfully.
 *          BCM_E_EXISTS   : Entry already exists.
 *          BCM_E_UNAVAIL  : Feature not supported.
 *          BCM_E_INIT     : Range module not initialized.
 *          BCM_E_RESOURCE : Range Ids are all allocated
 *          BCM_E_XXX      : Standard error code.
 */
int 
bcm_esw_range_create(
        int unit,
        int flags, 
        bcm_range_config_t *range_config) 
{

#if defined (BCM_TOMAHAWK_SUPPORT)
    return bcmi_xgs5_range_create(unit, flags, range_config);
#endif

    return BCM_E_UNAVAIL;
}

/* Function :   bcm_esw_range_get 
 * 
 * Purpose  :   This API is to retrieve the operational configuration for the given range id.
 *
 * Parameters : range_config - (IN) Range Config Structure 
 * 
 * Returns  :
 *              BCM_E_NONE      : Range Module get successful
 *              BCM_E_NOT_FOUND : Range not found
 *              BCM_E_INIT      : Range Module not initialized
 *              BCM_E_UNAVAIL   : Feature not supported.
 *              BCM_E_XXX       : Standard error code.
 */
int 
bcm_esw_range_get(
        int unit, 
        bcm_range_config_t *range_config) 
{
#if defined (BCM_TOMAHAWK_SUPPORT)
    return bcmi_xgs5_range_get(unit, range_config);
#endif

    return BCM_E_UNAVAIL;
}

/* 
 * Function : bcm_esw_range_destroy
 *  
 * Purpose  : Destroys a range checker and releases resources allocated to the range checker. 
 *            Hardware resources will also be freed. But this API will return BCM_E_BUSY, 
 *            if the range checker is in use.
 *
 * Parameters   :   unit - (IN) Unit number.
 *                  rid  - (IN) Range ID.
 *
 * Returns:
 *          BCM_E_NONE      :   Range deleted successfully
 *          BCM_E_NOT_FOUND :   Range does not exist
 *          BCM_E_INIT      :   Range module not initialised
 *          BCM_E_UNAVAIL   :   Feature not supported
 *          BCM_E_XXX       :   Standard error code
 */
int 
bcm_esw_range_destroy(
        int unit,
        bcm_range_t rid)
{
#if defined (BCM_TOMAHAWK_SUPPORT)
    return bcmi_xgs5_range_destroy(unit, rid);
#endif

    return BCM_E_UNAVAIL;    
}
/*
 * Function :   bcm_esw_range_traverse
 * 
 * Purpose  :   Traverse all the valid range ids in the system, calling a specified
 *              callback for each one
 *
 * Parameters   :   unit - (IN) Unit number.
 *                  callback - (IN) A pointer to the callback function to call for each range
 *                  user_data - (IN) Pointer to user data to supply in the callback
 * 
 * Returns  :
 *              BCM_E_XXX
 */
int 
bcm_esw_range_traverse(
        int unit,
        bcm_range_traverse_cb callback,
        void *user_data)
{
#if defined (BCM_TOMAHAWK_SUPPORT)
    return bcmi_xgs5_range_traverse(unit, callback, user_data);
#endif
    return BCM_E_UNAVAIL;
}
/*
 * Function :   bcm_esw_range_oper_mode_set
 *
 * Purpose  :   Configure Operational mode of Range Module to either Global or Pipe Local. 
 *
 * Parameters   :
 *          unit        - (IN) BCM Device number.
 *          oper_mode   - (IN) Range Operational Mode enum value.
 * Returns      :
 *          BCM_E_NONE  :   Operation successful.
 *          BCM_E_PARAM :   Invalid operational mode.
 *          BCM_E_UNIT  :   Range module not initialized.
 *          BCM_E_BUSY  :   Ranges are already existing. Hence cannot change 
 *                          operational mode
 */
int 
bcm_esw_range_oper_mode_set(
        int unit,
        bcm_range_oper_mode_t oper_mode)
{
#if defined (BCM_TOMAHAWK_SUPPORT)
    return bcmi_xgs5_range_oper_mode_set(unit, oper_mode);
#endif
    return BCM_E_UNAVAIL;
}

/* 
 * Function :  bcm_esw_range_oper_mode_get
 *
 * Purpose  :  Get current operational mode of the Range Module. 
 *
 * Parameters   :   unit    - (IN)  BCM Device number.
 *                  oper_mode-(OUT) Reference to Range Operational Mode value.
 * Returns  :
 *          BCM_E_NONE  :   Operation successful.
 *          BCM_E_PARAM :   Invalid parameter.
 *          BCM_E_UNIT  :   Range module not initialized.
 */
int bcm_esw_range_oper_mode_get(
        int unit,
        bcm_range_oper_mode_t *oper_mode)
{
#if defined (BCM_TOMAHAWK_SUPPORT)
    return bcmi_xgs5_range_oper_mode_get(unit, oper_mode);
#endif
    return BCM_E_UNAVAIL;
}
/*
 * Function :   _bcm_esw_range_scache_sync
 * 
 * Purpose  :   Update the scache based on the latest range module state
 *
 * Parameters:  unit    - (IN)  BCM Device number.
 *
 * Returns  :   BCM_E_XXX
 *      
 */
int _bcm_esw_range_scache_sync(
        int unit) 
{
#if defined (BCM_TOMAHAWK_SUPPORT)
#if defined  BCM_WARM_BOOT_SUPPORT
    return bcmi_xgs5_range_wb_sync(unit);
#endif
#endif
    return BCM_E_UNAVAIL;
}
