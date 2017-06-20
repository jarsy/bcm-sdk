/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    failover.c
 * Purpose: Handle TH2 specific failover APIs
 */


#include <shared/bsl.h>

#include <soc/defs.h>
#include <sal/core/libc.h>

#if defined(INCLUDE_L3)
#if defined(BCM_TOMAHAWK2_SUPPORT)

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/util.h>
#include <soc/debug.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/failover.h>
#include <bcm/extender.h>

#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/tomahawk2.h>
#include <bcm_int/common/multicast.h>
#include <bcm_int/esw/failover.h>
#include <bcm_int/esw/triumph2.h>

#define _bcm_th2_failover_bk_info _bcm_failover_bk_info
typedef _bcm_failover_bookkeeping_t _bcm_th2_failover_bookkeeping_t; 

/*
 * Function:
 *      _bcm_th2_failover_check_init
 * Purpose:
 *      Check if Failover is initialized
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXX
 */
STATIC int 
_bcm_th2_failover_check_init (int unit)
{
    if ((unit < 0) || (unit >= BCM_MAX_NUM_UNITS)) {
        return BCM_E_UNIT;
    }

    if (!_bcm_th2_failover_bk_info[unit].initialized) {
        return BCM_E_INIT;
    } else {
        return BCM_E_NONE;
    }
}

/*
 * Function:
 *      bcm_failover_lock
 * Purpose:
 *      Take the Failover Lock Sempahore
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXX
 */
int 
bcm_th2_failover_lock (int unit)
{
    int rv;

    rv = _bcm_th2_failover_check_init(unit);

    if ( rv == BCM_E_NONE ) {
        sal_mutex_take(FAILOVER_INFO((unit))->failover_mutex,
                sal_mutex_FOREVER);
    }
    return rv; 
}

/*
 * Function:
 *      bcm_failover_unlock
 * Purpose:
 *      Release  the Failover Lock Semaphore
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXX
 */
void
bcm_th2_failover_unlock (int unit)
{
    int rv;

    rv = _bcm_th2_failover_check_init(unit);

    if ( rv == BCM_E_NONE ) {
        sal_mutex_give(FAILOVER_INFO((unit))->failover_mutex);
    }
}

/*
 * Function:
 *      _bcm_th2_failover_free_resource
 * Purpose:
 *      Free all allocated software resources 
 * Parameters:
 *      unit - SOC unit number
 * Returns:
 *      Nothing
 */
STATIC void
_bcm_th2_failover_free_resource(int unit,
        _bcm_th2_failover_bookkeeping_t *failover_info)
{
    if (!failover_info) {
        return;
    }

    if (failover_info->prot_group_bitmap) {
        sal_free(failover_info->prot_group_bitmap);
        failover_info->prot_group_bitmap = NULL;
    }

    if (failover_info->prot_nhi_bitmap) {
        sal_free(failover_info->prot_nhi_bitmap);
        failover_info->prot_nhi_bitmap = NULL;
    }

}

#ifdef BCM_WARM_BOOT_SUPPORT
/* 
 * Function:
 *     _bcm_th2_failover_reinit
 * Purpose:
 *     Reinit for warm boot.
 * Parameters:
 *      unit    : (IN) Device Unit Number
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_th2_failover_reinit(int unit)
{
    int idx, index_min, index_max, i , bit_index;
    tx_initial_prot_group_table_entry_t prot_grp_entry;
    uint32 buf[4];
    int value;

    index_min = soc_mem_index_min(unit, TX_INITIAL_PROT_GROUP_TABLEm);
    index_max = soc_mem_index_max(unit, TX_INITIAL_PROT_GROUP_TABLEm);

    for (idx = index_min; idx <= index_max; idx++) {

        SOC_IF_ERROR_RETURN(soc_mem_read(unit, TX_INITIAL_PROT_GROUP_TABLEm,
                    MEM_BLOCK_ANY, idx, &prot_grp_entry));

        soc_mem_field_get(unit, TX_INITIAL_PROT_GROUP_TABLEm,
                (uint32 *)&prot_grp_entry, REPLACE_ENABLE_BITMAPf, buf);
        for (i = 0; i < BCM_TH2_FAILOVER_PROT_GROUPS_PER_ENTRY ; i++) {
            value = ((buf[i / 32] >> (i % 32)) & 0x1);
            if (value != 0) {
                bit_index = (((idx & 0x7) << 7) | (i & 0x7F));
                _BCM_FAILOVER_PROT_GROUP_MAP_USED_SET(unit, bit_index);
            }
        }
    }

    return BCM_E_NONE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

/*
 * Function:
 *      bcm_th2_failover_init
 * Purpose:
 *      Initialize the Failover software module
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_th2_failover_init (int unit)
{
    int rv = BCM_E_NONE;
    int num_prot_group, num_prot_nhi;
    _bcm_th2_failover_bookkeeping_t *failover_info = FAILOVER_INFO(unit);

    /*
     * allocate resources
     */
    if (failover_info->initialized) {
        BCM_IF_ERROR_RETURN(bcm_th2_failover_cleanup(unit));
    }

    num_prot_group = (soc_mem_index_count(unit, TX_INITIAL_PROT_GROUP_TABLEm)
                      * BCM_TH2_FAILOVER_PROT_GROUPS_PER_ENTRY);
    num_prot_nhi = soc_mem_index_count(unit, INITIAL_PROT_NHI_TABLEm);

    failover_info->prot_group_bitmap =
        sal_alloc(SHR_BITALLOCSIZE(num_prot_group), "prot_group_bitmap");
    if (failover_info->prot_group_bitmap == NULL) {
        _bcm_th2_failover_free_resource(unit, failover_info);
        return BCM_E_MEMORY;
    }

    failover_info->prot_nhi_bitmap =
        sal_alloc(SHR_BITALLOCSIZE(num_prot_nhi), "prot_nhi_bitmap");
    if (failover_info->prot_nhi_bitmap == NULL) {
        _bcm_th2_failover_free_resource(unit, failover_info);
        return BCM_E_MEMORY;
    }

    sal_memset(failover_info->prot_group_bitmap, 0,
               SHR_BITALLOCSIZE(num_prot_group));
    sal_memset(failover_info->prot_nhi_bitmap, 0,
               SHR_BITALLOCSIZE(num_prot_nhi));

    /* Create Failover  protection mutex. */
    failover_info->failover_mutex = sal_mutex_create("failover_mutex");
    if (!failover_info->failover_mutex) {
        _bcm_th2_failover_free_resource(unit, failover_info);
        return BCM_E_MEMORY;
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    if(SOC_WARM_BOOT(unit)) {
        rv = _bcm_th2_failover_reinit(unit);
        if (BCM_FAILURE(rv)){
            _bcm_th2_failover_free_resource(unit, failover_info);
            return rv;
        }
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    /* Mark the state as initialized */
    failover_info->initialized = TRUE;

    return rv;
}

/*
 * Function:
 *      _bcm_th2_failover_hw_clear
 * Purpose:
 *     Perform hw tables clean up for failover module. 
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_th2_failover_hw_clear (int unit)
{
    int i, rv, rv_error = BCM_E_NONE;
    int num_entry;

    num_entry = (soc_mem_index_count(unit, TX_INITIAL_PROT_GROUP_TABLEm) *
                 BCM_TH2_FAILOVER_PROT_GROUPS_PER_ENTRY);

    /* Index-0 is reserved */      
    for (i = 1; i < num_entry; i++) {
        if (_BCM_FAILOVER_PROT_GROUP_MAP_USED_GET(unit, i)) {
            rv = bcm_th2_failover_destroy(unit, i);
            if (rv < 0) {
                rv_error = rv;
            }
        }
    }

    return rv_error;
}

/*
 * Function:
 *      bcm_th2_failover_cleanup
 * Purpose:
 *      DeInit  the Failover software module
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_th2_failover_cleanup (int unit)
{
    _bcm_th2_failover_bookkeeping_t *failover_info = FAILOVER_INFO(unit);
    int rv = BCM_E_UNAVAIL;

    if (FALSE == failover_info->initialized) {
        return (BCM_E_NONE);
    } 

    rv = bcm_th2_failover_lock(unit);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    if (0 == SOC_HW_ACCESS_DISABLE(unit)) { 
        rv = _bcm_th2_failover_hw_clear(unit);
    }

    /* Free software resources */
    (void) _bcm_th2_failover_free_resource(unit, failover_info);

    bcm_th2_failover_unlock(unit);

    /* Destroy protection mutex. */
    sal_mutex_destroy(failover_info->failover_mutex);

    /* Mark the state as uninitialized */
    failover_info->initialized = FALSE;

    return rv;
}


/*
 * Function:
 *		_bcm_th2_failover_get_prot_group_table_index
 * Purpose:
 *		Obtain Index into  INITIAL_PROT_GROUP_TABLE
 * Parameters:
 *		IN :  Unit
 *           OUT : prot_index
 * Returns:
 *		BCM_E_XXX
 */
STATIC int
_bcm_th2_failover_get_prot_group_table_index (int unit, int *prot_index)
{
    int  i;
    int  num_entry;

    num_entry = (soc_mem_index_count(unit, TX_INITIAL_PROT_GROUP_TABLEm) *
                 BCM_TH2_FAILOVER_PROT_GROUPS_PER_ENTRY);

    /* Index-0 is reserved */      
    for (i = 1; i < num_entry; i++) {
        if (!_BCM_FAILOVER_PROT_GROUP_MAP_USED_GET(unit, i)) {
            break;
        }
    }

    if (i == num_entry) {
        return BCM_E_RESOURCE;
    }
    _BCM_FAILOVER_PROT_GROUP_MAP_USED_SET(unit, i);
    *prot_index = i;	 

    return BCM_E_NONE;
}

/*
 * Function:
 *		_bcm_th2_failover_clear_prot_group_table_entry
 * Purpose:
 *		Clear INITIAL_PROT_GROUP_TABLE entry pointed by Index
 * Parameters:
 *		IN :  Unit
 *           IN : prot_index
 * Returns:
 *		BCM_E_XXX
 */
STATIC void
_bcm_th2_failover_clear_prot_group_table_entry (int unit,
        bcm_failover_t prot_index)
{
   _BCM_FAILOVER_PROT_GROUP_MAP_USED_CLR(unit, prot_index);
}

/*
 * Function:
 *		_bcm_th2_failover_set_prot_group_table_entry
 * Purpose:
 *		Set INITIAL_PROT_GROUP_TABLE entry pointed by Index
 * Parameters:
 *		IN :  Unit
 *           IN : prot_index
 * Returns:
 *		BCM_E_XXX
 */
STATIC void
_bcm_th2_failover_set_prot_group_table_entry (int unit,
        bcm_failover_t prot_index)
{
   _BCM_FAILOVER_PROT_GROUP_MAP_USED_SET(unit, prot_index);
}

/*
 * Function:
 *		_bcm_th2_failover_id_validate
 * Purpose:
 *		Validate the failover ID
 * Parameters:
 *           IN : failover_id
 * Returns:
 *		BCM_E_XXX
 */
STATIC int
_bcm_th2_failover_id_validate (int unit, bcm_failover_t failover_id)
{
    int num_entry;

    num_entry = (soc_mem_index_count(unit, TX_INITIAL_PROT_GROUP_TABLEm) *
                 BCM_TH2_FAILOVER_PROT_GROUPS_PER_ENTRY);

    if ((failover_id < 1) || (failover_id > num_entry)) {
        return BCM_E_PARAM;
    } else if (failover_id == num_entry) {
        return  BCM_E_RESOURCE;
    }
    if (!_BCM_FAILOVER_PROT_GROUP_MAP_USED_GET(unit, failover_id)) {
        return BCM_E_NOT_FOUND;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *		bcm_th2_failover_create
 * Purpose:
 *		Create a failover object
 * Parameters:
 *		IN :  unit
 *           IN :  flags
 *           OUT :  failover_id
 * Returns:
 *		BCM_E_XXX
 */
int 
bcm_th2_failover_create (int unit, uint32 flags, bcm_failover_t *failover_id)
{
    int rv = BCM_E_UNAVAIL;
    tx_initial_prot_group_table_entry_t  prot_group_entry;
    rx_prot_group_table_entry_t  rx_prot_group_entry;
    int  num_entry;
    uint32 table_index;
    uint32 bit_index;
    uint32 buf[4];

    if (failover_id == NULL) {
        return BCM_E_PARAM;
    }

    /* Check for unsupported Flag */
    if (flags & (~(BCM_FAILOVER_REPLACE |
                   BCM_FAILOVER_WITH_ID))) {
        return BCM_E_PARAM;
    }
    /*TH2 does not support 1:1 MC protection*/
    if (SOC_IS_TOMAHAWK2(unit) && (flags & BCM_FAILOVER_ENCAP)) {
        return BCM_E_PARAM;
    }

    if ((flags & BCM_FAILOVER_WITH_ID) || (flags & BCM_FAILOVER_REPLACE)) {
        /* make sure the given id is valid */
        num_entry = (soc_mem_index_count(unit, TX_INITIAL_PROT_GROUP_TABLEm) *
                     BCM_TH2_FAILOVER_PROT_GROUPS_PER_ENTRY);
        if ((*failover_id < 1) || (*failover_id >= num_entry)) {
            return BCM_E_PARAM;
        }
        if (flags & BCM_FAILOVER_WITH_ID) {
            if (_BCM_FAILOVER_PROT_GROUP_MAP_USED_GET(unit, *failover_id)) {
                return BCM_E_EXISTS;
            } else {
                _BCM_FAILOVER_PROT_GROUP_MAP_USED_SET(unit, *failover_id);
            }
        }
        if (flags & BCM_FAILOVER_REPLACE) {
            if (!_BCM_FAILOVER_PROT_GROUP_MAP_USED_GET(unit, *failover_id)) {
                return BCM_E_NOT_FOUND;
            }
        }
        rv = BCM_E_NONE;
    } else if (!flags) {
        /* Get Index */
        rv = _bcm_th2_failover_get_prot_group_table_index(unit, failover_id);
    }

    if (BCM_SUCCESS(rv)) {
        /* Init table entry */
        table_index = ((*failover_id >> 7) & 0x7);
        bit_index = (*failover_id & 0x7F);
        BCM_IF_ERROR_RETURN (soc_mem_read(unit, TX_INITIAL_PROT_GROUP_TABLEm,
                    MEM_BLOCK_ANY, table_index, &prot_group_entry));
        sal_memcpy(buf, &prot_group_entry, sizeof(buf));

        buf[bit_index / 32] &= ~(0x1 << (bit_index % 32));

        soc_mem_field_set(unit, TX_INITIAL_PROT_GROUP_TABLEm,
                (uint32 *)&prot_group_entry, REPLACE_ENABLE_BITMAPf, buf);

        rv = soc_mem_write(unit, TX_INITIAL_PROT_GROUP_TABLEm,
                MEM_BLOCK_ALL, table_index, &prot_group_entry);
        if (rv < 0) {
            _bcm_th2_failover_clear_prot_group_table_entry(unit, *failover_id);
            return BCM_E_RESOURCE;
        }

        /* Init table entry */
        table_index = ((*failover_id >> 7) & 0xF);
        bit_index = (*failover_id & 0x7F);
        BCM_IF_ERROR_RETURN (soc_mem_read(unit, RX_PROT_GROUP_TABLEm,
                    MEM_BLOCK_ANY, table_index, &rx_prot_group_entry));

        sal_memcpy(buf, &rx_prot_group_entry, sizeof(buf));

        buf[bit_index / 32] &= ~(0x1 << (bit_index % 32));
        soc_mem_field_set(unit, RX_PROT_GROUP_TABLEm,
                (uint32 *)&rx_prot_group_entry, DROP_DATA_ENABLE_BITMAPf, buf);

        rv = soc_mem_write(unit, RX_PROT_GROUP_TABLEm,
                MEM_BLOCK_ALL, table_index, &rx_prot_group_entry);
        if (rv < 0) {
            _bcm_th2_failover_clear_prot_group_table_entry(unit, *failover_id);
            return BCM_E_RESOURCE;
        }
    }

    return rv;
}

/*
 * Function:
 *		bcm_th2_failover_destroy
 * Purpose:
 *		Destroy a failover object
 * Parameters:
 *		IN :  unit
 *           IN :  failover_id
 * Returns:
 *		BCM_E_XXX
 */
int 
bcm_th2_failover_destroy (int unit, bcm_failover_t failover_id)
{
    int rv = BCM_E_UNAVAIL;
    int type = _BCM_FAILOVER_DEFAULT_MODE;
    tx_initial_prot_group_table_entry_t  prot_group_entry;
    rx_prot_group_table_entry_t  rx_prot_group_entry;
    uint32 table_index;
    uint32 bit_index;
    uint32 buf[4];

    /*Check failover id type Ingress or Egress Encap*/
    _BCM_GET_FAILOVER_TYPE(failover_id, type);
    if (type & _BCM_FAILOVER_1_1_MC_PROTECTION_MODE) {
        return BCM_E_PARAM;
    }
    rv = _bcm_th2_failover_id_validate(unit, failover_id);
    BCM_IF_ERROR_RETURN(rv);

    if (!_BCM_FAILOVER_PROT_GROUP_MAP_USED_GET(unit, failover_id)) {
        return BCM_E_NOT_FOUND;
    }

    /*Release index*/
    _bcm_th2_failover_clear_prot_group_table_entry(unit, failover_id);

    /* Clear entry */
    table_index = ((failover_id >> 7) & 0x7);
    bit_index = (failover_id & 0x7F);
    BCM_IF_ERROR_RETURN (soc_mem_read(unit, TX_INITIAL_PROT_GROUP_TABLEm,
                MEM_BLOCK_ANY, table_index, &prot_group_entry));

    sal_memcpy(buf, &prot_group_entry, sizeof(buf));

    buf[bit_index / 32] &= ~(0x1 << (bit_index % 32));
    soc_mem_field_set(unit, TX_INITIAL_PROT_GROUP_TABLEm,
            (uint32 *)&prot_group_entry, REPLACE_ENABLE_BITMAPf, buf);

    rv = soc_mem_write(unit, TX_INITIAL_PROT_GROUP_TABLEm, MEM_BLOCK_ALL,
            table_index, &prot_group_entry);
    if (rv < 0) {
        _bcm_th2_failover_set_prot_group_table_entry(unit, failover_id);
        return BCM_E_RESOURCE;
    }

    /* Init table entry */
    table_index = ((failover_id >> 7) & 0xF);
    bit_index = (failover_id & 0x7F);
    BCM_IF_ERROR_RETURN (soc_mem_read(unit, RX_PROT_GROUP_TABLEm,
                MEM_BLOCK_ANY, table_index, &rx_prot_group_entry));

    sal_memcpy(buf, &rx_prot_group_entry, sizeof(buf));

    buf[bit_index / 32] &= ~(0x1 << (bit_index % 32));
    soc_mem_field_set(unit, RX_PROT_GROUP_TABLEm,
            (uint32 *)&rx_prot_group_entry, DROP_DATA_ENABLE_BITMAPf, buf);

    rv = soc_mem_write(unit, RX_PROT_GROUP_TABLEm,
            MEM_BLOCK_ALL, table_index, &rx_prot_group_entry);

    if (rv < 0) {
        _bcm_th2_failover_set_prot_group_table_entry(unit, failover_id);
        return BCM_E_RESOURCE;
    }
    
    return rv;
}

STATIC int
_bcm_th2_failover_nhi_get(int unit, bcm_gport_t port, int *nh_index)
{
    int vp=0xFFFF;
    ing_dvp_table_entry_t dvp;

    if (!BCM_GPORT_IS_MPLS_PORT(port) &&
            !BCM_GPORT_IS_MIM_PORT(port) ) {
        return BCM_E_PARAM;
    }

    if ( BCM_GPORT_IS_MPLS_PORT(port)) {
        vp = BCM_GPORT_MPLS_PORT_ID_GET(port);
    } else if ( BCM_GPORT_IS_MIM_PORT(port)) {
        vp = BCM_GPORT_MIM_PORT_ID_GET(port);
    }

    if (vp >= soc_mem_index_count(unit, SOURCE_VPm)) {
        return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN (READ_ING_DVP_TABLEm(unit, MEM_BLOCK_ANY, vp, &dvp));

    /* Next-hop index is used for multicast replication */
    *nh_index = soc_ING_DVP_TABLEm_field32_get(unit, &dvp, NEXT_HOP_INDEXf);
    return BCM_E_NONE;
}

/*
 * Function:
 *        bcm_th2_failover_status_set
 * Purpose:
 *        Set the parameters for a failover object
 * Parameters:
 *        IN :  unit
 *           IN :  failover_id
 *           IN :  value
 * Returns:
 *        BCM_E_XXX
 */
int
bcm_th2_failover_status_set (int unit,
        bcm_failover_element_t *failover,
        int value)
{
    int rv = BCM_E_UNAVAIL;
    int nh_index;
    uint32 table_index = 0;
    uint32 bit_index = 0;
    uint32 buf[4];
    tx_initial_prot_group_table_entry_t  prot_group_entry;
    rx_prot_group_table_entry_t  rx_prot_group_entry;
    initial_prot_nhi_table_1_entry_t   prot_nhi_1_entry;

    if ((value < 0) || (value > 1)) {
        return BCM_E_PARAM;
    }
    if ((failover->flags & BCM_FAILOVER_ENCAP) && SOC_IS_TOMAHAWK2(unit)) {
            return BCM_E_PARAM;
    }
    if (failover->failover_id != BCM_FAILOVER_INVALID) {
        /* Group protection for Port and Tunnel: Egress and Ingress */
        table_index = ((failover->failover_id >> 7) & 0x7);
        bit_index = (failover->failover_id & 0x7F);

        BCM_IF_ERROR_RETURN(
                _bcm_th2_failover_id_validate(unit, failover->failover_id));

        BCM_IF_ERROR_RETURN (soc_mem_read(unit, TX_INITIAL_PROT_GROUP_TABLEm,
                    MEM_BLOCK_ANY, table_index, &prot_group_entry));

        sal_memcpy(buf, &prot_group_entry, sizeof(buf));
        if (value == 0) {
            buf[bit_index / 32] &= ~(0x1 << (bit_index % 32));
        } else {
            buf[bit_index / 32] |= (0x1 << (bit_index % 32));
        }

        soc_mem_field_set(unit, TX_INITIAL_PROT_GROUP_TABLEm,
                (uint32 *)&prot_group_entry, REPLACE_ENABLE_BITMAPf, buf);

        BCM_IF_ERROR_RETURN(
                soc_mem_write(unit, TX_INITIAL_PROT_GROUP_TABLEm,
                    MEM_BLOCK_ALL, table_index, &prot_group_entry));

        table_index = ((failover->failover_id >> 7) & 0xF);
        bit_index = (failover->failover_id & 0x7F);

        BCM_IF_ERROR_RETURN (soc_mem_read(unit, RX_PROT_GROUP_TABLEm,
                    MEM_BLOCK_ANY, table_index, &rx_prot_group_entry));

        sal_memcpy(buf, &rx_prot_group_entry, sizeof(buf));
        if (value == 0) {
            buf[bit_index / 32] &= ~(0x1 << (bit_index % 32));
        } else {
            buf[bit_index / 32] |= (0x1 << (bit_index % 32));
        }

        soc_mem_field_set(unit, RX_PROT_GROUP_TABLEm,
                (uint32 *)&rx_prot_group_entry, DROP_DATA_ENABLE_BITMAPf, buf);

        rv = soc_mem_write(unit, RX_PROT_GROUP_TABLEm,
                MEM_BLOCK_ALL, table_index,  &rx_prot_group_entry);
    

    } else if (failover->intf != BCM_IF_INVALID) {
        /* Only Egress is applicable */
        if (BCM_XGS3_L3_EGRESS_IDX_VALID(unit, failover->intf)) {
            nh_index = failover->intf - BCM_XGS3_EGRESS_IDX_MIN;
        } else {
            nh_index = failover->intf  - BCM_XGS3_DVP_EGRESS_IDX_MIN;
        }

        table_index = ((nh_index >> 7) & 0x1FF);
        bit_index = (nh_index & 0x7F);

        BCM_IF_ERROR_RETURN (soc_mem_read(unit, INITIAL_PROT_NHI_TABLE_1m,
                    MEM_BLOCK_ANY, table_index, &prot_nhi_1_entry));

        sal_memcpy(buf, &prot_nhi_1_entry, sizeof(buf));
        if (value == 0) {
            buf[bit_index / 32] &= ~(0x1 << (bit_index % 32));
        } else {
            buf[bit_index / 32] |= (0x1 << (bit_index % 32));
        }

        soc_mem_field_set(unit, INITIAL_PROT_NHI_TABLE_1m,
                (uint32 *)&prot_nhi_1_entry, REPLACE_ENABLE_BITMAPf, buf);

        rv = soc_mem_write(unit, INITIAL_PROT_NHI_TABLE_1m,
                MEM_BLOCK_ALL, table_index, &prot_nhi_1_entry);
        if (rv < 0) {
            return rv;
        }

    } else if (failover->port != BCM_GPORT_INVALID) {
        /* Individual protection for Pseudo-wire: Egress and Ingress */
        BCM_IF_ERROR_RETURN (
                _bcm_th2_failover_nhi_get(unit, failover->port , &nh_index));

        table_index = ((nh_index >> 7) & 0x1FF);
        bit_index = (nh_index & 0x7F);

        BCM_IF_ERROR_RETURN (soc_mem_read(unit, INITIAL_PROT_NHI_TABLE_1m,
                    MEM_BLOCK_ANY, table_index, &prot_nhi_1_entry));

        sal_memcpy(buf, &prot_nhi_1_entry, sizeof(buf));
        if (value == 0) {
            buf[bit_index / 32] &= ~(0x1 << (bit_index % 32));
        } else {
            buf[bit_index / 32] |= (0x1 << (bit_index % 32));
        }

        soc_mem_field_set(unit, INITIAL_PROT_NHI_TABLE_1m,
                (uint32 *)&prot_nhi_1_entry, REPLACE_ENABLE_BITMAPf, buf);

        rv = soc_mem_write(unit, INITIAL_PROT_NHI_TABLE_1m,
                MEM_BLOCK_ALL, table_index, &prot_nhi_1_entry);

    }
    return rv;
}


/*
 * Function:
 *        bcm_th2_failover_status_get
 * Purpose:
 *        Get the parameters for a failover object
 * Parameters:
 *        IN :  unit
 *           IN :  failover_id
 *           OUT :  value
 * Returns:
 *        BCM_E_XXX
 */
int
bcm_th2_failover_status_get (int unit,
        bcm_failover_element_t *failover,
        int  *value)
{
    tx_initial_prot_group_table_entry_t  prot_group_entry;
    rx_prot_group_table_entry_t  rx_prot_group_entry;
    initial_prot_nhi_table_1_entry_t   prot_nhi_1_entry;
    int nh_index;
    uint32 table_index = 0;
    uint32 bit_index = 0;
    uint32 buf[4];

    if ((failover->flags & BCM_FAILOVER_ENCAP) && SOC_IS_TOMAHAWK2(unit)) {
            return BCM_E_PARAM;
    }
    if (failover->failover_id != BCM_FAILOVER_INVALID) {
        BCM_IF_ERROR_RETURN(
                _bcm_th2_failover_id_validate(unit, failover->failover_id));

        table_index = ((failover->failover_id >> 7) & 0x7);
        bit_index = (failover->failover_id & 0x7F);

        BCM_IF_ERROR_RETURN (soc_mem_read(unit, TX_INITIAL_PROT_GROUP_TABLEm,
                    MEM_BLOCK_ANY, table_index, &prot_group_entry));

        soc_mem_field_get(unit, TX_INITIAL_PROT_GROUP_TABLEm,
                (uint32 *)&prot_group_entry, REPLACE_ENABLE_BITMAPf, buf);
        *value = ((buf[bit_index / 32] >> (bit_index % 32)) & 0x1);

        table_index = ((failover->failover_id >> 7) & 0xF);
        bit_index = (failover->failover_id & 0x7F);

        BCM_IF_ERROR_RETURN(soc_mem_read(unit, RX_PROT_GROUP_TABLEm,
                    MEM_BLOCK_ANY, table_index, &rx_prot_group_entry));

        soc_mem_field_get(unit, RX_PROT_GROUP_TABLEm,
                (uint32 *)&rx_prot_group_entry, DROP_DATA_ENABLE_BITMAPf, buf);

        *value &= ((buf[bit_index / 32] >> (bit_index % 32)) & 0x1);
    } else if (failover->intf != BCM_IF_INVALID) {

        if (BCM_XGS3_L3_EGRESS_IDX_VALID(unit, failover->intf)) {
            nh_index = failover->intf - BCM_XGS3_EGRESS_IDX_MIN;
        } else {
            nh_index = failover->intf  - BCM_XGS3_DVP_EGRESS_IDX_MIN;
        }

        table_index = ((nh_index >> 7) & 0x1FF);
        bit_index = (nh_index & 0x7F);

        BCM_IF_ERROR_RETURN (soc_mem_read(unit, INITIAL_PROT_NHI_TABLE_1m,
                    MEM_BLOCK_ANY, table_index, &prot_nhi_1_entry));

        soc_mem_field_get(unit, INITIAL_PROT_NHI_TABLE_1m,
                (uint32 *)&prot_nhi_1_entry, REPLACE_ENABLE_BITMAPf, buf);

        *value = ((buf[bit_index / 32] >> (bit_index % 32)) & 0x1);

    } else if (failover->port != BCM_GPORT_INVALID) {

        BCM_IF_ERROR_RETURN (
                _bcm_th2_failover_nhi_get(unit, failover->port , &nh_index));

        table_index = ((nh_index >> 7) & 0x1FF);
        bit_index = (nh_index & 0x7F);

        BCM_IF_ERROR_RETURN (soc_mem_read(unit, INITIAL_PROT_NHI_TABLE_1m,
                    MEM_BLOCK_ANY, table_index, &prot_nhi_1_entry));

        soc_mem_field_get(unit, INITIAL_PROT_NHI_TABLE_1m,
                (uint32 *)&prot_nhi_1_entry, REPLACE_ENABLE_BITMAPf, buf);

        *value = ((buf[bit_index / 32] >> (bit_index % 32)) & 0x1);
    }
    return BCM_E_NONE;
}

#endif /* defined(BCM_TOMAHAWK2_SUPPORT) */
#endif /* defined(INCLUDE_L3) */
