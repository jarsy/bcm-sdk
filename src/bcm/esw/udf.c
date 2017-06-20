/*
 * $Id: $
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <bcm/udf.h>
#include <bcm/error.h>

#include <soc/defs.h>
#include <soc/drv.h>

#include <bcm_int/esw/xgs4.h>


#if defined (BCM_TRIUMPH2_SUPPORT)
#if defined(BCM_WARM_BOOT_SUPPORT)
int
_bcm_esw_udf_scache_sync(int unit)
{
    if (soc_feature(unit, soc_feature_udf_support)) {
        return bcmi_xgs4_udf_wb_sync(unit);
    }

    return BCM_E_NONE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */
#endif /* BCM_TRIUMPH2_SUPPORT */

/*
 * Function:
 *      bcm_udf_init
 * Purpose:
 *      Initialize UDF module
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_NONE          UDF module initialized successfully.
 *      BCM_E_UNIT          Invalid BCM Unit number.
 *      BCM_E_UNAVAIL       Feature not supported.
 *      BCM_E_XXX           Standard error code.
 * Notes:
 */
int
bcm_esw_udf_init(
    int unit)
{
#if defined (BCM_TRIUMPH2_SUPPORT)
    if (soc_feature(unit, soc_feature_udf_support)) {
        return bcmi_xgs4_udf_init(unit);
    }
#endif

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_udf_detach
 * Purpose:
 *      Detach UDF module
 * Parameters:
 *      unit - (IN) Unit number.
 * Returns:
 *      BCM_E_NONE          UDF module detached successfully.
 *      BCM_E_UNIT          Invalid BCM Unit number.
 *      BCM_E_INIT          UDF module not initialized.
 *      BCM_E_UNAVAIL       Feature not supported.
 *      BCM_E_XXX           Standard error code.
 * Notes:
 */
int
bcm_esw_udf_detach(
    int unit)
{
#if defined (BCM_TRIUMPH2_SUPPORT)
    if (soc_feature(unit, soc_feature_udf_support)) {
        return bcmi_xgs4_udf_detach(unit);
    }
#endif

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_udf_create
 * Purpose:
 *      Creates a UDF object
 * Parameters:
 *      unit - (IN) Unit number.
 *      hints - (IN) Hints to UDF allocator
 *      udf_info - (IN) UDF structure
 *      udf_id - (IN/OUT) UDF ID
 * Returns:
 *      BCM_E_NONE          UDF created successfully.
 *      BCM_E_EXISTS        Entry already exists.
 *      BCM_E_FULL          UDF table full.
 *      BCM_E_UNIT          Invalid BCM Unit number.
 *      BCM_E_INIT          UDF module not initialized.
 *      BCM_E_UNAVAIL       Feature not supported.
 *      BCM_E_XXX           Standard error code.
 * Notes:
 */
int
bcm_esw_udf_create(
    int unit,
    bcm_udf_alloc_hints_t *hints,
    bcm_udf_t *udf_info,
    bcm_udf_id_t *udf_id)
{
#if defined (BCM_TRIUMPH2_SUPPORT)
    if (soc_feature(unit, soc_feature_udf_support)) {
        return bcmi_xgs4_udf_create(unit, hints, udf_info, udf_id);
    }
#endif

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_udf_get
 * Purpose:
 *      Fetches the UDF object created in the system
 * Parameters:
 *      unit - (IN) Unit number.
 *      udf_id - (IN) UDF Object ID
 *      udf_info - (OUT) UDF info structure
 * Returns:
 *      BCM_E_NONE          UDF get successful.
 *      BCM_E_NOT_FOUND     UDF does not exist.
 *      BCM_E_UNIT          Invalid BCM Unit number.
 *      BCM_E_INIT          UDF module not initialized.
 *      BCM_E_UNAVAIL       Feature not supported.
 *      BCM_E_XXX           Standard error code.
 * Notes:
 */
int
bcm_esw_udf_get(
    int unit,
    bcm_udf_id_t udf_id,
    bcm_udf_t *udf_info)
{
#if defined (BCM_TRIUMPH2_SUPPORT)
    if (soc_feature(unit, soc_feature_udf_support)) {
        return bcmi_xgs4_udf_get(unit, udf_id, udf_info);
    }
#endif

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_udf_get_all
 * Purpose:
 *      Fetches all existing UDF ids
 * Parameters:
 *      unit - (IN) Unit number.
 *      max - (IN) Max number of UDF IDs
 *      udf_id_list - (OUT) List of UDF IDs
 *      actual - (OUT) Actual udfs retrieved
 * Returns:
 *      BCM_E_NONE          UDF get successful.
 *      BCM_E_UNIT          Invalid BCM Unit number.
 *      BCM_E_INIT          UDF module not initialized.
 *      BCM_E_UNAVAIL       Feature not supported.
 *      BCM_E_XXX           Standard error code.
 * Notes:
 */
int
bcm_esw_udf_get_all(
    int unit,
    bcm_udf_id_t max,
    bcm_udf_id_t *udf_id_list,
    int *actual)
{
#if defined (BCM_TRIUMPH2_SUPPORT)
    if (soc_feature(unit, soc_feature_udf_support)) {
        return bcmi_xgs4_udf_get_all(unit, max, udf_id_list, actual);
    }
#endif

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_udf_destroy
 * Purpose:
 *      Destroys the UDF object
 * Parameters:
 *      unit - (IN) Unit number.
 *      udf_id - (IN) UDF Object ID
 * Returns:
 *      BCM_E_NONE          UDF deleted successfully.
 *      BCM_E_NOT_FOUND     UDF does not exist.
 *      BCM_E_UNIT          Invalid BCM Unit number.
 *      BCM_E_INIT          UDF module not initialized.
 *      BCM_E_UNAVAIL       Feature not supported.
 *      BCM_E_XXX           Standard error code.
 * Notes:
 */
int
bcm_esw_udf_destroy(
    int unit,
    bcm_udf_id_t udf_id)
{
#if defined (BCM_TRIUMPH2_SUPPORT)
    if (soc_feature(unit, soc_feature_udf_support)) {
        return bcmi_xgs4_udf_destroy(unit, udf_id);
    }
#endif

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_udf_pkt_format_create
 * Purpose:
 *      Create a packet format entry
 * Parameters:
 *      unit - (IN) Unit number.
 *      options - (IN) API Options.
 *      pkt_format - (IN) UDF packet format info structure
 *      pkt_format_id - (OUT) Packet format ID
 * Returns:
 *      BCM_E_NONE          UDF packet format entry created
 *                          successfully.
 *      BCM_E_UNIT          Invalid BCM Unit number.
 *      BCM_E_INIT          UDF module not initialized.
 *      BCM_E_UNAVAIL       Feature not supported.
 *      BCM_E_XXX           Standard error code.
 * Notes:
 */
int
bcm_esw_udf_pkt_format_create(
    int unit,
    bcm_udf_pkt_format_options_t options,
    bcm_udf_pkt_format_info_t *pkt_format,
    bcm_udf_pkt_format_id_t *pkt_format_id)
{
#if defined (BCM_TRIUMPH2_SUPPORT)
    if (soc_feature(unit, soc_feature_udf_support)) {
        return bcmi_xgs4_udf_pkt_format_create(unit, options,
                                              pkt_format, pkt_format_id);
    }
#endif

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_udf_pkt_format_info_get
 * Purpose:
 *      Retrieve packet format info given the packet format Id
 * Parameters:
 *      unit - (IN) Unit number.
 *      pkt_format_id - (IN) Packet format ID
 *      pkt_format - (OUT) UDF packet format info structure
 * Returns:
 *      BCM_E_NONE          Packet format get successful.
 *      BCM_E_NOT_FOUND     Packet format entry does not exist.
 *      BCM_E_UNIT          Invalid BCM Unit number.
 *      BCM_E_INIT          UDF module not initialized.
 *      BCM_E_UNAVAIL       Feature not supported.
 *      BCM_E_XXX           Standard error code.
 * Notes:
 */
int
bcm_esw_udf_pkt_format_info_get(
    int unit,
    bcm_udf_pkt_format_id_t pkt_format_id,
    bcm_udf_pkt_format_info_t *pkt_format)
{
#if defined (BCM_TRIUMPH2_SUPPORT)
    if (soc_feature(unit, soc_feature_udf_support)) {
        return bcmi_xgs4_udf_pkt_format_info_get(unit,
                                                pkt_format_id, pkt_format);
    }
#endif

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_udf_pkt_format_destroy
 * Purpose:
 *      Destroy existing packet format entry
 * Parameters:
 *      unit - (IN) Unit number.
 *      pkt_format_id - (IN) Packet format ID
 *      pkt_format - (IN) UDF packet format info structure
 * Returns:
 *      BCM_E_NONE          Destroy packet format entry.
 *      BCM_E_NOT_FOUND     Packet format ID does not exist.
 *      BCM_E_UNIT          Invalid BCM Unit number.
 *      BCM_E_INIT          UDF module not initialized.
 *      BCM_E_UNAVAIL       Feature not supported.
 *      BCM_E_XXX           Standard error code.
 * Notes:
 */
int
bcm_esw_udf_pkt_format_destroy(
    int unit,
    bcm_udf_pkt_format_id_t pkt_format_id)
{
#if defined (BCM_TRIUMPH2_SUPPORT)
    if (soc_feature(unit, soc_feature_udf_support)) {
        return bcmi_xgs4_udf_pkt_format_destroy(unit, pkt_format_id);
    }
#endif

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_udf_pkt_format_add
 * Purpose:
 *      Adds packet format entry to UDF object
 * Parameters:
 *      unit - (IN) Unit number.
 *      udf_id - (IN) UDF ID
 *      pkt_format_id - (IN) Packet format ID
 * Returns:
 *      BCM_E_NONE          Packet format entry added to UDF
 *                          successfully.
 *      BCM_E_NOT_FOUND     UDF Id or packet format entry does not
 *                          exist.
 *      BCM_E_UNIT          Invalid BCM Unit number.
 *      BCM_E_INIT          UDF module not initialized.
 *      BCM_E_UNAVAIL       Feature not supported.
 *      BCM_E_XXX           Standard error code.
 * Notes:
 */
int
bcm_esw_udf_pkt_format_add(
    int unit,
    bcm_udf_id_t udf_id,
    bcm_udf_pkt_format_id_t pkt_format_id)
{
#if defined (BCM_TRIUMPH2_SUPPORT)
    if (soc_feature(unit, soc_feature_udf_support)) {
        return bcmi_xgs4_udf_pkt_format_add(unit, udf_id, pkt_format_id);
    }
#endif

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_udf_pkt_format_get
 * Purpose:
 *      Fetches all UDFs that share the common packet format entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      pkt_format_id - (IN) Packet format ID.
 *      max - (IN) Max number of UDF IDs
 *      udf_id_list - (OUT) List of UDF IDs
 *      actual - (OUT) Actual udfs retrieved
 * Returns:
 *      BCM_E_NONE          Success.
 *      BCM_E_UNIT          Invalid BCM Unit number.
 *      BCM_E_INIT          UDF module not initialized.
 *      BCM_E_UNAVAIL       Feature not supported.
 *      BCM_E_XXX           Standard error code.
 * Notes:
 */
int
bcm_esw_udf_pkt_format_get(
    int unit,
    bcm_udf_pkt_format_id_t pkt_format_id,
    bcm_udf_id_t max,
    bcm_udf_id_t *udf_id_list,
    int *actual)
{
#if defined (BCM_TRIUMPH2_SUPPORT)
    if (soc_feature(unit, soc_feature_udf_support)) {
        return bcmi_xgs4_udf_pkt_format_get(unit, pkt_format_id,
                                            max, udf_id_list, actual);
    }
#endif

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_udf_pkt_format_delete
 * Purpose:
 *      Deletes packet format spec associated with the UDF
 * Parameters:
 *      unit - (IN) Unit number.
 *      udf_id - (IN) UDF ID
 *      pkt_format_id - (IN) Packet format ID
 * Returns:
 *      BCM_E_NONE          Packet format configuration successfully
 *                          deleted from UDF.
 *      BCM_E_NOT_FOUND     UDF Id or packet format entry does not
 *                          exist.
 *      BCM_E_UNIT          Invalid BCM Unit number.
 *      BCM_E_INIT          UDF module not initialized.
 *      BCM_E_UNAVAIL       Feature not supported.
 *      BCM_E_XXX           Standard error code.
 * Notes:
 */
int
bcm_esw_udf_pkt_format_delete(
    int unit,
    bcm_udf_id_t udf_id,
    bcm_udf_pkt_format_id_t pkt_format_id)
{
#if defined (BCM_TRIUMPH2_SUPPORT)
    if (soc_feature(unit, soc_feature_udf_support)) {
        return bcmi_xgs4_udf_pkt_format_delete(unit, udf_id, pkt_format_id);
    }
#endif

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_udf_pkt_format_get_all
 * Purpose:
 *      Retrieves the user defined format specification configuration
 *      from UDF
 * Parameters:
 *      unit - (IN) Unit number.
 *      udf_id - (IN) UDF ID
 *      max - (IN) Max Packet formats attached to a UDF object
 *      pkt_format_id_list - (OUT) List of packet format entries added to udf id
 *      actual - (OUT) Actual number of Packet formats retrieved
 * Returns:
 *      BCM_E_NONE          Success.
 *      BCM_E_NOT_FOUND     Either the UDF or packet format entry does
 *                          not exist.
 *      BCM_E_UNIT          Invalid BCM Unit number.
 *      BCM_E_INIT          UDF module not initialized.
 *      BCM_E_UNAVAIL       Feature not supported.
 *      BCM_E_XXX           Standard error code.
 * Notes:
 */
int
bcm_esw_udf_pkt_format_get_all(
    int unit,
    bcm_udf_id_t udf_id,
    int max,
    bcm_udf_pkt_format_id_t *pkt_format_id_list,
    int *actual)
{
#if defined (BCM_TRIUMPH2_SUPPORT)
    if (soc_feature(unit, soc_feature_udf_support)) {
        return bcmi_xgs4_udf_pkt_format_get_all(unit, udf_id, max,
                                               pkt_format_id_list, actual);
    }
#endif

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_udf_pkt_format_delete_all
 * Purpose:
 *      Deletes all packet format specs associated with the UDF
 * Parameters:
 *      unit - (IN) Unit number.
 *      udf_id - (IN) UDF ID
 * Returns:
 *      BCM_E_NONE          Deletes all packet formats associated with
 *                          the UDF.
 *      BCM_E_NOT_FOUND     UDF Id does not exist.
 *      BCM_E_UNIT          Invalid BCM Unit number.
 *      BCM_E_INIT          UDF module not initialized.
 *      BCM_E_UNAVAIL       Feature not supported.
 *      BCM_E_XXX           Standard error code.
 * Notes:
 */
int
bcm_esw_udf_pkt_format_delete_all(
    int unit,
    bcm_udf_id_t udf_id)
{
#if defined (BCM_TRIUMPH2_SUPPORT)
    if (soc_feature(unit, soc_feature_udf_support)) {
        return bcmi_xgs4_udf_pkt_format_delete_all(unit, udf_id);
    }
#endif

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_esw_udf_hash_config_add
 * Purpose:
 *      Add UDF id into hashing list and configure it
 * Parameters:
 *      unit            (IN) BCM unit number
 *      options         (IN) See bcm_esw_udf_HASH_ADD_O_xxx
 *      config          (IN) UDF hashing configuration info
 * Returns:
 *      BCM_E_NONE      UDF hashing added successfully
 *      BCM_E_BADID     UDF id does not exist or is not for UDF hashing usage
 *      BCM_E_XXX       Standard error code
 * Notes:
 */
int bcm_esw_udf_hash_config_add(
    int unit,
    uint32 options,
    bcm_udf_hash_config_t *config)
{
#if defined (BCM_TRIDENT2_SUPPORT)
    if (soc_feature(unit, soc_feature_udf_hashing)) {
        return bcmi_xgs4_udf_hash_config_add(unit, options, config);
    }
#endif /* BCM_TRIDENT2_SUPPORT */

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_esw_udf_hash_config_delete
 * Purpose:
 *      Delete UDF id from hashing list
 * Parameters:
 *      unit            (IN) BCM unit number
 *      config          (IN) UDF hashing configuration info
 * Returns:
 *      BCM_E_NONE      UDF hashing added successfully
 *      BCM_E_XXX       Standard error code
 * Notes:
 */
int bcm_esw_udf_hash_config_delete(
    int unit,
    bcm_udf_hash_config_t *config)
{
#if defined (BCM_TRIDENT2_SUPPORT)
    if (soc_feature(unit, soc_feature_udf_hashing)) {
        return bcmi_xgs4_udf_hash_config_delete(unit, config);
    }
#endif /* BCM_TRIDENT2_SUPPORT */

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_esw_udf_hash_config_delete_all
 * Purpose:
 *      Delete all UDF id from hashing list
 * Parameters:
 *      unit            (IN) BCM unit number
 * Returns:
 *      BCM_E_NONE      UDF hashing added successfully
 *      BCM_E_XXX       Standard error code
 * Notes:
 */
int bcm_esw_udf_hash_config_delete_all(
    int unit)
{
#if defined (BCM_TRIDENT2_SUPPORT)
    if (soc_feature(unit, soc_feature_udf_hashing)) {
        return bcmi_xgs4_udf_hash_config_delete_all(unit);
    }
#endif /* BCM_TRIDENT2_SUPPORT */

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_esw_udf_hash_config_get
 * Purpose:
 *      Get UDF hashing configuration of a certain id
 * Parameters:
 *      unit            (IN) BCM unit number
 *      config          (INOUT) UDF hashing configuration info
 * Returns:
 *      BCM_E_NONE      UDF hashing added successfully
 *      BCM_E_NOT_FOUND Requested UDF hashing info does not exist
 *      BCM_E_XXX       Standard error code
 * Notes:
 */
int bcm_esw_udf_hash_config_get(
    int unit,
    bcm_udf_hash_config_t *config)
{
#if defined (BCM_TRIDENT2_SUPPORT)
    if (soc_feature(unit, soc_feature_udf_hashing)) {
        return bcmi_xgs4_udf_hash_config_get(unit, config);
    }
#endif /* BCM_TRIDENT2_SUPPORT */

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      bcm_esw_udf_hash_config_get_all
 * Purpose:
 *      Get all added UDF ids from list
 * Parameters:
 *      unit                  (IN) BCM unit number
 *      max                   (IN) Max count of UDF ids in the list. 
 *      udf_hash_config_list  (OUT) List of UDF hashing configuration
 *      actual                (OUT) Actual count of UDF ids
 * Returns:
 *      BCM_E_NONE      UDF hashing added successfully
 *      BCM_E_XXX       Standard error code
 * Notes:
 */
int bcm_esw_udf_hash_config_get_all(
    int unit,
    int max,
    bcm_udf_hash_config_t *udf_hash_config_list,
    int *actual)
{
#if defined (BCM_TRIDENT2_SUPPORT)
    if (soc_feature(unit, soc_feature_udf_hashing)) {
        return bcmi_xgs4_udf_hash_config_get_all(unit, max, udf_hash_config_list, actual);
    }
#endif /* BCM_TRIDENT2_SUPPORT */

    return BCM_E_UNAVAIL;
}


/* Configure udf operation mode. */
int bcm_esw_udf_oper_mode_set(
    int unit, 
    bcm_udf_oper_mode_t oper_mode) 
{
#if defined (BCM_TOMAHAWK_SUPPORT)
    if ((soc_feature(unit, soc_feature_udf_support)) &&
       (soc_feature(unit, soc_feature_udf_multi_pipe_support))) {
        return bcmi_xgs4_udf_oper_mode_set(unit, oper_mode);
    }
#endif

    return BCM_E_UNAVAIL;
}

/* Retrieves current udf operation mode. */
int bcm_esw_udf_oper_mode_get(
    int unit, 
    bcm_udf_oper_mode_t *oper_mode)
{
#if defined (BCM_TOMAHAWK_SUPPORT)
    if ((soc_feature(unit, soc_feature_udf_support)) &&
       (soc_feature(unit, soc_feature_udf_multi_pipe_support))) {
        return bcmi_xgs4_udf_oper_mode_get(unit, oper_mode);
    }
#endif

    return BCM_E_UNAVAIL;
}

