/*
 * $Id: arl.c,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    arl.c
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <sal/core/time.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/robo.h>

#include <soc/robo/mcm/driver.h>
#include <soc/robo/arl.h>
#ifdef BCM_TBX_SUPPORT
#include "../tbx/robo_tbx.h"
#endif

#ifdef BCM_TBX_SUPPORT
int 
_drv_tbx_arl_hw_to_sw_entry (int unit, uint32 *key, 
        l2_arl_entry_t *hw_arl, l2_arl_sw_entry_t  *sw_arl)
{
    int         rv = SOC_E_NONE;
    
    rv = DRV_MEM_READ(unit, DRV_MEM_ARL_HW, *key, 1, (uint32 *)hw_arl);
    if (rv < 0){
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              " %s, SOC driver problem on reading hw_arl!\n"), 
                   FUNCTION_NAME()));
        return rv;
    }
    
    rv = DRV_MEM_READ(unit, DRV_MEM_ARL, *key, 1, (uint32 *)sw_arl);
    if (rv < 0){
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              " %s, SOC driver problem on reading sw_arl!\n"), 
                   FUNCTION_NAME()));
        return rv;
    }
    return rv;
}
#endif

#ifdef BCM_HARRIER_SUPPORT
int _drv_harrier_arl_hw_to_sw_entry (int unit, uint32 *key, l2_arl_entry_t *hw_arl, l2_arl_sw_entry_t  *sw_arl)
{
    uint32        temp;
    uint32      vid_rw, mcast_index;
    uint32        reg_value;
    uint64      rw_mac_field, temp_mac_field;
    uint8          hash_value[6];
    uint8       mac_addr_rw[6], temp_mac_addr[6];
    uint16        hash_result;
    int         rv = SOC_E_NONE;

    SOC_IF_ERROR_RETURN(DRV_MEM_READ
        (unit, DRV_MEM_ARL, *key, 1, (uint32 *)hw_arl));
    /* Read VALID bit */
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
        (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_VALID, 
            (uint32 *)hw_arl, &temp));
    if (!temp) {
        return rv;
    }

    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, (uint32 *)sw_arl, &temp));


    /* Read VLAN ID value */
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
        (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_VLANID, 
            (uint32 *)hw_arl, &vid_rw));

    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, (uint32 *)sw_arl, &vid_rw));
        
    /* Read MAC address bit 12~47 */
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET    
        (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_MAC, 
            (uint32 *)hw_arl, (uint32 *)&rw_mac_field));

    SAL_MAC_ADDR_FROM_UINT64(mac_addr_rw, rw_mac_field);

    /* check HASH enabled ? */
    /* check 802.1q enable */
    if ((rv = REG_READ_VLAN_CTRL0r(unit, &reg_value)) < 0) {
        return rv;
    }

    soc_VLAN_CTRL0r_field_get(unit, &reg_value,
        VLAN_ENf, &temp);

    sal_memset(hash_value, 0, sizeof(hash_value));
    if (temp) {
        soc_VLAN_CTRL0r_field_get(unit, &reg_value,
            VLAN_LEARN_MODEf, &temp);
        if (temp == 3) {
            /* hash value = VID + MAC */
            hash_value[0] = (vid_rw >> 4) & 0xff;
            hash_value[1] = ((vid_rw & 0xf) << 4) + (mac_addr_rw[1] & 0xf);
            hash_value[2] = mac_addr_rw[2];
            hash_value[3] = mac_addr_rw[3];
            hash_value[4] = mac_addr_rw[4];
            hash_value[5] = mac_addr_rw[5];
        } else {
            /* hash value = MAC */
            hash_value[0] = ((mac_addr_rw[1] & 0xf) << 4) 
                + ((mac_addr_rw[2] & 0xf0) >> 4);
            hash_value[1] = ((mac_addr_rw[2] & 0xf) << 4) 
                + ((mac_addr_rw[3] & 0xf0) >> 4);
            hash_value[2] = ((mac_addr_rw[3] & 0xf) << 4) 
                + ((mac_addr_rw[4] & 0xf0) >> 4);
            hash_value[3] = ((mac_addr_rw[4] & 0xf) << 4) 
                + ((mac_addr_rw[5] & 0xf0) >> 4);
            hash_value[4] = ((mac_addr_rw[5] & 0xf) << 4);
        }
    } else {
        /* hash value = MAC value */
        hash_value[0] = ((mac_addr_rw[1] & 0xf) << 4) 
            + ((mac_addr_rw[2] & 0xf0) >> 4);
        hash_value[1] = ((mac_addr_rw[2] & 0xf) << 4) 
            + ((mac_addr_rw[3] & 0xf0) >> 4);
        hash_value[2] = ((mac_addr_rw[3] & 0xf) << 4) 
            + ((mac_addr_rw[4] & 0xf0) >> 4);
        hash_value[3] = ((mac_addr_rw[4] & 0xf) << 4) 
            + ((mac_addr_rw[5] & 0xf0) >> 4);
        hash_value[4] = ((mac_addr_rw[5] & 0xf) << 4);
    }

    /* Get the hash revalue */
    _drv_arl_hash(hash_value, 47, &hash_result);

    /* Recover the MAC bit 0~11 */
    temp = *key;
    temp = temp >> 2; 
    hash_result = (hash_result ^ temp) & 0xfff;

    temp_mac_addr[0] = ((mac_addr_rw[1] & 0xf) << 4) 
        + ((mac_addr_rw[2] & 0xf0) >> 4);
    temp_mac_addr[1] = ((mac_addr_rw[2] & 0xf) << 4) 
        + ((mac_addr_rw[3] & 0xf0) >> 4);
    temp_mac_addr[2] = ((mac_addr_rw[3] & 0xf) << 4) 
        + ((mac_addr_rw[4] & 0xf0) >> 4);
    temp_mac_addr[3] = ((mac_addr_rw[4] & 0xf) << 4) 
        + ((mac_addr_rw[5] & 0xf0) >> 4);
    temp_mac_addr[4] = ((mac_addr_rw[5] & 0xf) << 4) 
        + (hash_result >> 8);
    temp_mac_addr[5] = hash_result & 0xff;
    SAL_MAC_ADDR_TO_UINT64(temp_mac_addr, temp_mac_field);

    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
            (uint32 *)sw_arl, (uint32 *)&temp_mac_field));
    if (temp_mac_addr[0] & 0x01) { /* multicast entry */
        /* Multicast index */
        mcast_index = 0;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_SRC_PORT, 
                (uint32 *)hw_arl, &temp));
        mcast_index = temp;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(        
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_PRIORITY, 
                (uint32 *)hw_arl, &temp));
        mcast_index += (temp << 6);
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_AGE, 
                (uint32 *)hw_arl, &temp));
        mcast_index += (temp << 11);
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(
            unit, DRV_MEM_ARL, DRV_MEM_FIELD_DEST_BITMAP, 
            (uint32 *)sw_arl, &mcast_index));
    } else { /* unicast entry */
        /* Source port number */
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_SRC_PORT, 
                (uint32 *)hw_arl, &temp));
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(
            unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
            (uint32 *)sw_arl, &temp));

        /* Age bit */
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(        
            unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_AGE, 
                (uint32 *)hw_arl, &temp));
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(
            unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, 
            (uint32 *)sw_arl, &temp));
    }

    /* Static bit */
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(
        unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_STATIC, 
           (uint32 *)hw_arl, &temp));
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(
        unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
        (uint32 *)sw_arl, &temp));

    return rv;
}
#endif

#ifdef BCM_DINO16_SUPPORT
static int 
_drv_dino16_arl_hw_to_sw_entry (int unit, uint32 *key, 
    l2_arl_entry_t *hw_arl, l2_arl_sw_entry_t  *sw_arl)
{
    uint32  temp;
    int     rv = SOC_E_NONE;

    SOC_IF_ERROR_RETURN(DRV_MEM_READ
        (unit, DRV_MEM_ARL, *key, 1, (uint32 *)hw_arl));

    /* Read VALID bit */
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
        (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_VALID, 
            (uint32 *)hw_arl, &temp));

    if (!temp) {
        return rv;
    }
    sal_memcpy(sw_arl, hw_arl, sizeof(l2_arl_entry_t));

    return rv;
}
#endif /* BCM_DINO16_SUPPORT */

int
drv_arl_table_process(int unit, uint32 *key, void *hw_arl, void  *sw_arl)
{
    int rv = SOC_E_NONE;

    if (SOC_IS_HARRIER(unit)) {
#ifdef BCM_HARRIER_SUPPORT
        rv = _drv_harrier_arl_hw_to_sw_entry
            (unit, key, (l2_arl_entry_t * )hw_arl, (l2_arl_sw_entry_t *) sw_arl);
#endif
    }

    if (SOC_IS_TBX(unit)) {
#ifdef BCM_TBX_SUPPORT
        rv = _drv_tbx_arl_hw_to_sw_entry(unit, key, 
                (l2_arl_entry_t * )hw_arl, (l2_arl_sw_entry_t *) sw_arl);
#endif
    }

    if (SOC_IS_DINO16(unit)) {
#ifdef BCM_DINO16_SUPPORT
        rv = _drv_dino16_arl_hw_to_sw_entry(unit, key, 
                (l2_arl_entry_t * )hw_arl, (l2_arl_sw_entry_t *) sw_arl);
#endif
    }

    return rv;
}

/* arlsync used compare information :
 *    - log the valid and age field inforamtion on this chip.
 */
typedef struct drv_arlsync_cmp_info_s{
    int     init;           /* TRUE/FALSE, default is FALSE */
    int     word_reversed;  /* l2 entry work order is revrsed or not */
    uint32  valid32_mask;   /* uint32 mask */
    uint16  valid_bp;       /* bit position */
    uint16  valid_cross_boundary;   /* cross the uint32 boundary */
    uint32  age32_mask;     /* uint32 mask */
    uint16  age_bp;         /* bit position */
    uint16  age_cross_boundary;   /* cross the uint32 boundary */
} drv_arlsync_cmp_info_t;

static drv_arlsync_cmp_info_t 
            drv_arlsync_cmp_info[SOC_MAX_NUM_SWITCH_DEVICES];   

#define DRV_ARLCMP_INFO_UINT32_BOUNDARY_TEST(_bp, _len)   \
        ((((_bp) % 32 + (_len - 1)) < 32) ? FALSE : TRUE)
        
#define DRV_ARLSYNC_MASK32_BUILD(_cross_boundary, _start, _len, _mask)    \
    if (!(_cross_boundary)) {               \
        (_mask) = (uint32)-1;               \
        (_mask) &= -1 << ((_start) % 32);   \
        (_mask) &= (1 << (((_start) + (_len) - 1) % 32) << 1) - 1;  \
    } else {            \
        (_mask) = 0;    \
    }


/* arl compare result flags 
 * 
 * Note :
 *  1. DRV_ARLCMP_NODIFF : 
 *        - for both l2 entry are invalid or 
 *        - for both l2 entry are valid with no any data field changed.
 *  2. DRV_ARLCMP_NONE_VALID_AGE_CHANGED :
 *        - means change is observed on the field except 'valid' and 'age'
 */
typedef enum drv_arlcmp_result_flag_e {
    DRV_ARLCMP_NODIFF           = 0x0,
    DRV_ARLCMP_INSERT           = 0x1,
    DRV_ARLCMP_DELETE           = 0x2,
    DRV_ARLCMP_VALID_PENDING    = 0x4,
    DRV_ARLCMP_AGE_CHANGED      = 0x8,
    DRV_ARLCMP_NONE_VALID_AGE_CHANGED    = 0x10,
    DRV_ARLCMP_UNEXPECT         = 0x20
} drv_arlcmp_result_flag_t;


/* macro definition for arlsync compare */
#define DRV_ARLCMP_VALID_CHANGE_MASK   \
        (DRV_ARLCMP_INSERT | DRV_ARLCMP_DELETE | DRV_ARLCMP_VALID_PENDING)

/* SOC driver to build the valid and age field information in l2 entry */
int 
drv_robo_arlsync_cmp_info_build(int unit)
{
    soc_field_info_t    *fieldp;
    soc_mem_info_t      *memp;
    uint32              tmp, valid_fld_id = 0, age_fld_id = 0;
    int                 cross_boundary;
    
    /* early return if init is proceeded already */
    if (drv_arlsync_cmp_info[unit].init == TRUE){
        return SOC_E_NONE;
    }

    /* all ROBO chips use the same table name of SW ARL table */
    memp = &SOC_MEM_INFO(unit, INDEX(L2_ARL_SWm));

    /* assigning the valid and age filed name for most of ROBO chips first */
    valid_fld_id = INDEX(VALID_Rf);
    age_fld_id = INDEX(AGEf);
    
    if (SOC_IS_TBX(unit)){
        valid_fld_id = INDEX(VAf);
    } else if (SOC_IS_STARFIGHTER(unit) || SOC_IS_LOTUS(unit) ||
        SOC_IS_BLACKBIRD2(unit) || SOC_IS_POLAR(unit) || 
        SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {
        valid_fld_id = INDEX(VALIDf);
    }

    /* word ordering effort required per arl entry format :
     */
    drv_arlsync_cmp_info[unit].word_reversed = FALSE;
    
    /* assign the valid field info */
    SOC_FIND_FIELD(valid_fld_id, memp->fields, memp->nFields, fieldp);
    assert(fieldp);
    drv_arlsync_cmp_info[unit].valid_bp = fieldp->bp; 
    cross_boundary = 
            DRV_ARLCMP_INFO_UINT32_BOUNDARY_TEST(fieldp->bp, fieldp->len);
    drv_arlsync_cmp_info[unit].valid_cross_boundary = cross_boundary;
    DRV_ARLSYNC_MASK32_BUILD(cross_boundary, fieldp->bp, fieldp->len, tmp);
    drv_arlsync_cmp_info[unit].valid32_mask = tmp;
            

    /* assign the age field info */
    SOC_FIND_FIELD(age_fld_id, memp->fields, memp->nFields, fieldp);
    assert(fieldp);
    drv_arlsync_cmp_info[unit].age_bp = fieldp->bp; 
    cross_boundary = 
            DRV_ARLCMP_INFO_UINT32_BOUNDARY_TEST(fieldp->bp, fieldp->len);
    drv_arlsync_cmp_info[unit].age_cross_boundary = cross_boundary;
    DRV_ARLSYNC_MASK32_BUILD(cross_boundary, fieldp->bp, fieldp->len, tmp);
    drv_arlsync_cmp_info[unit].age32_mask = tmp;

    drv_arlsync_cmp_info[unit].init = TRUE;

    /* varified! no error! */
    LOG_VERBOSE(BSL_LS_SOC_ARL,
                (BSL_META_U(unit,
                            "%s, SW_ARLSYNC_COMPARE_INFO....init=%d\n"
                            "  reversed=%d,va_mask=0x%08x,va_bp=%d,va_bound=%d,"
                            "  age_mask=0x%08x,age_bp=%d,age_bound=%d\n"),
                 FUNCTION_NAME(),drv_arlsync_cmp_info[unit].init, 
                 drv_arlsync_cmp_info[unit].word_reversed,
                 drv_arlsync_cmp_info[unit].valid32_mask,
                 drv_arlsync_cmp_info[unit].valid_bp, 
                 drv_arlsync_cmp_info[unit].valid_cross_boundary,
                 drv_arlsync_cmp_info[unit].age32_mask, 
                 drv_arlsync_cmp_info[unit].age_bp, 
                 drv_arlsync_cmp_info[unit].age_cross_boundary));
    
    return SOC_E_NONE;
}


/* macro definitions for helping the readable and maintainance of code */
#define _DRV_ARLCMP_ALL_INVALID(_old_va, _new_va)   \
            (!(_old_va) && !(_new_va))

/* old is invalid and new is valid or pending */
#define _DRV_ARLCMP_IS_INSERT(_old_va, _new_va)     \
            (!(_old_va) && (_new_va))

/* old is valid or pending and new is invalid */
#define _DRV_ARLCMP_IS_DELETE(_old_va, _new_va)     \
            ((_old_va) && !(_new_va))

/* old_valid and new_valid both TRUE, check pending status */
#define _DRV_ARLCMP_ALL_VALID(_old_pen, _new_pen)   \
            (!(_old_pen) && !(_new_pen))
#define _DRV_ARLCMP_ALL_PENDING(_old_pen, _new_pen) \
            ((_old_pen) && (_new_pen))
        
/* performing the high performance processing to compare two arl entries */
static int 
_drv_arlsync_compare(int unit, 
        uint32 *old_sw_arl, uint32 *new_sw_arl, 
        drv_arlcmp_result_flag_t *result_flags)
{
    uint32  old_valid = 0, old_age = 0, old_pending = 0;
    uint32  new_valid = 0, new_age = 0, new_pending = 0;
    uint32  temp_bp, temp_mask, temp32, temp_valid;
    int     i, l2_word_size, l2_size;
    l2_arl_sw_entry_t   old_l2, new_l2;

    assert(old_sw_arl && new_sw_arl);   /* NULL check */

    if (drv_arlsync_cmp_info[unit].init != TRUE){
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s, Unexpected condition during ARL thread started!\n"),
                  FUNCTION_NAME()));
        *result_flags = DRV_ARLCMP_NODIFF;
        return SOC_E_INTERNAL;
    }

    l2_size = sizeof(l2_arl_sw_entry_t);
    /* coverity[dead_error_condition : FALSE] */
    l2_word_size = (l2_size / 4) + ((l2_size % 4) ? 1 : 0);

    sal_memcpy(&old_l2, old_sw_arl, l2_size);
    sal_memcpy(&new_l2, new_sw_arl, l2_size);

    /* 1. retrieve the valid status first */
    if (!drv_arlsync_cmp_info[unit].valid_cross_boundary){
        temp_bp = drv_arlsync_cmp_info[unit].valid_bp;
        temp_mask = drv_arlsync_cmp_info[unit].valid32_mask;
        /* get the uint32 data to retrieve old valid */
        if (drv_arlsync_cmp_info[unit].word_reversed){
            i = ((l2_word_size - 1) - (temp_bp / 32));
            temp32 = *(((uint32 *)&old_l2) + i);
        } else {
            temp32 = *(((uint32 *)&old_l2) + (temp_bp / 32));
        }
        
        temp_valid = (temp32 & temp_mask) >> (temp_bp % 32);
        /* valid/pending both treate as valid */
        old_valid = (temp_valid) ? TRUE : FALSE;
        old_pending = FALSE;
        if (SOC_IS_TBX(unit)){
#ifdef  BCM_TBX_SUPPORT
            if (old_valid) {
                if (temp_valid == _TB_ARL_STATUS_PENDING){
                    old_pending = TRUE;
                } else if (temp_valid != _TB_ARL_STATUS_VALID){
                    /* unexpected value */
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s, Unexpected value on valid field!\n"),
                              FUNCTION_NAME()));
                }
            }
#endif  /* BCM_TBX_SUPPORT */
        }
        
        /* get the uint32 data to retrieve new valid */
        if (drv_arlsync_cmp_info[unit].word_reversed){
            i = ((l2_word_size - 1) - (temp_bp / 32));
            temp32 = *(((uint32 *)&new_l2) + i);
        } else {
            temp32 = *(((uint32 *)&new_l2) + (temp_bp / 32));
        }
        temp_valid = (temp32 & temp_mask) >> (temp_bp % 32);

        /* valid/pending both treate as valid */
        new_valid = (temp_valid) ? TRUE : FALSE;
        new_pending = FALSE;
        if (SOC_IS_TBX(unit)){
#ifdef  BCM_TBX_SUPPORT
            if (new_valid) {
                if (temp_valid == _TB_ARL_STATUS_PENDING){
                    new_pending = TRUE;
                } else if (temp_valid != _TB_ARL_STATUS_VALID){
                    /* unexpected value */
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s, Unexpected value on valid field!\n"),
                              FUNCTION_NAME()));
                }
            }
#endif  /* BCM_TBX_SUPPORT */
        }

        /* early return for valid changed 
         *  - insert / delete cases
         *  - changed between valid and pending cases
         */
        if (_DRV_ARLCMP_ALL_INVALID(old_valid, new_valid)){
            /* both invalid, no more compare required */
            *result_flags = DRV_ARLCMP_NODIFF;
            goto arlcmp_finished;
        } else if (_DRV_ARLCMP_IS_INSERT(old_valid, new_valid)){
            /* insert action, no more compare required */
            *result_flags |= DRV_ARLCMP_INSERT;
            goto arlcmp_finished;
        } else if (_DRV_ARLCMP_IS_DELETE(old_valid, new_valid)){
            /* delete action, no more compare required */
            *result_flags |= DRV_ARLCMP_DELETE;
            goto arlcmp_finished;
        } else {
            /* both at valid or pending status,
             *  - assume old_valid == new_valid == TRUE
             *  - check pending status is enough in this section.
             */
            if (_DRV_ARLCMP_ALL_VALID(old_pending, new_pending) || 
                    _DRV_ARLCMP_ALL_PENDING(old_pending, new_pending)){
                /* need more compare later*/
                *result_flags = DRV_ARLCMP_NODIFF;
            } else {
                /* one is valid and the other is pending. */
                /* valid/pending changed, no more compare required */
                *result_flags |= DRV_ARLCMP_VALID_PENDING;
                goto arlcmp_finished;
            }
        }
    } else {
        /* valid_cross_boundary : ROBO chips has no such condition so far */
        *result_flags = DRV_ARLCMP_NODIFF;
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s, Unexpected process for uint32 boundary issue!\n"),
                  FUNCTION_NAME()));
        return SOC_E_INTERNAL;
    }
    
    /* 2. retrieve the age status */
    if (!drv_arlsync_cmp_info[unit].age_cross_boundary){
        temp_bp = drv_arlsync_cmp_info[unit].age_bp;
        temp_mask = drv_arlsync_cmp_info[unit].age32_mask;
        /* get the uint32 data to retrieve old age */
        if (drv_arlsync_cmp_info[unit].word_reversed){
            i = ((l2_word_size - 1) - (temp_bp / 32));
            temp32 = *(((uint32 *)&old_l2) + i);
            old_age = (temp32 & temp_mask) >> (temp_bp % 32);
            /* mask-out age field */
            temp32 &= (uint32)(~temp_mask);
            *(((uint32 *)&old_l2) + i) = temp32;
        } else {
            temp32 = *(((uint32 *)&old_l2) + (temp_bp / 32));
            old_age = (temp32 & temp_mask) >> (temp_bp % 32);
            /* mask-out age field */
            temp32 &= (uint32)(~temp_mask);
            *(((uint32 *)&old_l2) + (temp_bp / 32)) = temp32;
        }
        
        
        /* get the uint32 data to retrieve new age */
        if (drv_arlsync_cmp_info[unit].word_reversed){
            i = ((l2_word_size - 1) - (temp_bp / 32));
            temp32 = *(((uint32 *)&new_l2) + i);
            new_age = (temp32 & temp_mask) >> (temp_bp % 32);
            /* mask-out age field */
            temp32 &= (uint32)(~temp_mask);
            *(((uint32 *)&new_l2) + i) = temp32;
        } else {
            temp32 = *(((uint32 *)&new_l2) + (temp_bp / 32));
            new_age = (temp32 & temp_mask) >> (temp_bp % 32);
            /* mask-out age field */
            temp32 &= (uint32)(~temp_mask);
            *(((uint32 *)&new_l2) + (temp_bp / 32)) = temp32;
        }

        if (old_age != new_age){
            *result_flags |= DRV_ARLCMP_AGE_CHANGED;
        }
    } else {
        /* age_cross_boundary : ROBO chips has no such condition so far */
        *result_flags = DRV_ARLCMP_UNEXPECT;
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s, Unexpected process for uint32 boundary issue!\n"),
                  FUNCTION_NAME()));
        return SOC_E_INTERNAL;
    }
    
    /* 3. ARL entry compare 
     *  - assumed the old and new l2 entry both at valid or pending status  
     *      and the age field were both mask-out already.
     */
    for (i = 0; i < (l2_size / 4); i++){
        temp32 = *((uint32 *)&old_l2 + i) ^ *((uint32 *)&new_l2 + i);
        if (temp32){
            /* means difference observed */
            *result_flags |= DRV_ARLCMP_NONE_VALID_AGE_CHANGED;
            break;
        }
    }

arlcmp_finished:
    
    return SOC_E_NONE;
}

/*  check the valid status on a given SW l2 entry.
 *  
 */
void
_drv_arl_swl2_valid_get(int unit, l2_arl_sw_entry_t *sw_l2, int *valid)
{
    uint32  temp_bp, temp_mask, temp32, temp_valid;
    int     ent_word_size, ent_size;

    *valid = 0;
    if (sw_l2 == NULL){
        return;
    }

    ent_size = sizeof(l2_arl_sw_entry_t);
    /* coverity[dead_error_condition : FALSE] */
    ent_word_size = (ent_size / 4) + ((ent_size % 4) ? 1 : 0);
    if (!drv_arlsync_cmp_info[unit].valid_cross_boundary){
        temp_bp = drv_arlsync_cmp_info[unit].valid_bp;
        temp_mask = drv_arlsync_cmp_info[unit].valid32_mask;
        /* get the uint32 data to retrieve old valid */
        if (drv_arlsync_cmp_info[unit].word_reversed){
            temp32 = *(((uint32 *)sw_l2) + 
                    ((ent_word_size - 1) - (temp_bp / 32)));
        } else {
            temp32 = *(((uint32 *)sw_l2) + (temp_bp / 32));
        }
        temp_valid = (temp32 & temp_mask) >> (temp_bp % 32);

        if (SOC_IS_TBX(unit)){
            /* pending(b01) is treat as valid(b11) as well */
            *valid = (temp_valid) ? 1: 0;
        } else {
            /* other chips have only one valid bit */
            *valid = temp_valid;
        }
    }
}

#define _DRV_HASH_ID_2_L2_ID(_h_id, _bin_cnt, _bin) \
        (((_h_id) * (_bin_cnt)) | ((_bin) % (_bin_cnt)))
#define _DRV_L2_ID_2_HASH_ID(_l2_id, _bin_cnt)  ((_l2_id) / (_bin_cnt))
#define _DRV_L2_ID_2_BIN_ID(_l2_id, _bin_cnt)  ((_l2_id) % (_bin_cnt))

#ifdef  BCM_TBX_SUPPORT
#define _DRV_TB_IS_HASH32_BIN(_bin)  ((_bin) > 1)
#endif  /* BCM_TBX_SUPPORT */

/* Sanity process when add a new l2 entry into SW shadow table :
 *  
 *  1. Remove the duplicated L2 entry which is existed in SW ARL already but 
 *      placed in another l2 index due to hash conflict.
 *  2. Need call back for such removment.
 *  3. return SOC_E_EXIST if found the redundant l2 entry.
 *  
 */
static int 
_drv_arl_add_sanity_process(int unit, uint32 *key, void *new_arl)
{
    int     hash_index = -1, this_bin_id = -1, bin_cnt = 0;
    int     l2_idx = -1, valid = 0, i;
    uint32  vid = 0, chk_vid = 0;
    uint64  mac_field, chk_mac_field;
    l2_arl_sw_entry_t   *sw_l2_entry;
    soc_control_t       *soc = SOC_CONTROL(unit);
#ifdef  BCM_TBX_SUPPORT
    int     hash_is_crc16 = 1, hash2_index = -1;
#endif  /* BCM_TBX_SUPPORT */

    COMPILER_64_ZERO(mac_field);
    COMPILER_64_ZERO(chk_mac_field);

    if (SOC_IS_TBX(unit)){   /* 16K dual hash 4bin (each hash 2 bin) */
        /* dual hash */
        bin_cnt = 4;   
    } else if (SOC_IS_GEX(unit)){
        /* lotus:512 4 bin; others:1K 4bin  */
        bin_cnt = 4;
    } else if (SOC_IS_HARRIER(unit)) {
        /* 4K 2bin */
        bin_cnt = 2;
    } else if (SOC_IS_DINO(unit)) {
        /* 2bin */
        bin_cnt = 2;
    } else {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "_drv_arl_del_reduncant_l2ent(), chip is not handled!\n")));
        return SOC_E_INTERNAL;
    }

    /* remove the redundant MAC+VID from other hashed bin */
    hash_index = _DRV_L2_ID_2_HASH_ID(*key, bin_cnt);
    this_bin_id = _DRV_L2_ID_2_BIN_ID(*key, bin_cnt);
#ifdef  BCM_TBX_SUPPORT
    if (SOC_IS_TBX(unit)){
        hash_is_crc16 = (_DRV_TB_IS_HASH32_BIN(this_bin_id)) ? 0 : 1;
    }
#endif  /* BCM_TBX_SUPPORT */

    /* retrieve MAC+VID */
    DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC,
            (uint32 *)new_arl, (void *)&mac_field);
    DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID,
            (uint32 *)new_arl, (uint32 *)&vid);

#ifdef  BCM_TBX_SUPPORT
    /* for dual hash device, retrieve the other hash index */
    if (SOC_IS_TBX(unit)){
        int rv = SOC_E_NONE;

        rv = _drv_tbx_arl_hash_index_get(unit, 
                !hash_is_crc16, (uint32 *)new_arl, &hash2_index);
        SOC_IF_ERROR_RETURN(rv);
    }
#endif  /* BCM_TBX_SUPPORT */

    /* check the bins in the same hash index */
    for (i = 0; i < bin_cnt; i++){
        if (i == this_bin_id){
            continue;
        }
        
        /* assign the l2 index from hash_id */
        l2_idx = _DRV_HASH_ID_2_L2_ID(hash_index, bin_cnt, i);
#ifdef  BCM_TBX_SUPPORT
        if (SOC_IS_TBX(unit)){
            /* special for TB's dual hash once the checking bin is at the 
             * other hash bank.
             */
            if ((hash_is_crc16 && _DRV_TB_IS_HASH32_BIN(i)) || 
                    (!hash_is_crc16 && !_DRV_TB_IS_HASH32_BIN(i))) {
                l2_idx = _DRV_HASH_ID_2_L2_ID(hash2_index, bin_cnt, i);
            }
        }
#endif  /* BCM_TBX_SUPPORT */

        sw_l2_entry = (l2_arl_sw_entry_t *)(soc->arl_table) + l2_idx;
        _drv_arl_swl2_valid_get(unit, sw_l2_entry, &valid);

        if (valid){
            DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC,
                    (uint32 *)sw_l2_entry, (void *)&chk_mac_field);
            DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID,
                    (uint32 *)sw_l2_entry, (uint32 *)&chk_vid);
            
            if (COMPILER_64_EQ(mac_field, chk_mac_field) && vid == chk_vid) {

                /* issue the delete callback first */
                soc_robo_arl_callback(unit, sw_l2_entry, NULL);

                /* remove this redundant l2 entry from sw arl */
                sal_memset(sw_l2_entry, 0, sizeof(l2_arl_sw_entry_t));
                LOG_INFO(BSL_LS_SOC_ARLMON,
                         (BSL_META_U(unit,
                                     "L2 entry is removed(id=%d) before L2 ADD(id=%d)\n"), 
                          *key, l2_idx));

                *key = l2_idx;
                return SOC_E_EXISTS;
            }
        }
    }

    return SOC_E_NONE;
}

/* SW ARL Callback type definitions :
 *
 *  1. _DRV_SWARL_NO_CB : indicated no callback request.
 *  2. _DRV_SWARL_INSERT_CB : callback for insert action
 *  3. _DRV_SWARL_DELETE_CB : callback for delete action
 *  4. _DRV_SWARL_MODIFY_CB : callback for modify action
 */
#define _DRV_SWARL_NO_CB        0
#define _DRV_SWARL_MODIFY_CB    1
#define _DRV_SWARL_DELETE_CB    2
#define _DRV_SWARL_INSERT_CB    3

/* SW_ARL_UPDATE_HIT force all SW ARL containing no AGE or HIT information.
 *  - This information can speed the arl_sync. process to aviod the case when 
 *      the old and new arl entry is different at age field only.
 *  - If this symbol is defined as FALSE, all sync l2 entry will be kept as 
 *      the first sync age field value.
 */
#define SW_ARL_UPDATE_HIT   FALSE

/*
 * Function:
 *  drv_arl_sync (internal)
 * Purpose:
 *  Compare old ARL contents to new and synchronize shadow table.
 * Parameters:
 *  unit    - RoboSwitch unit #
 *  old_arl - Results of previous ARL entry
 *  new_arl - Results of current ARL entry
 */
int
drv_arl_sync(int unit, uint32 *key, void *old_arl, void *new_arl)
{
    int     rv = SOC_E_NONE;
    uint32  cb_level = _DRV_SWARL_NO_CB;
    sal_usecs_t     stime, etime;
    soc_control_t   *soc = SOC_CONTROL(unit);
    drv_arlcmp_result_flag_t result_flags = 0;
    l2_arl_sw_entry_t   *old, old_arl_temp;

    int     del_hash_duplicate = 0;
    uint32  temp_key = 0;

    ARL_SW_TABLE_LOCK(soc);
    old = (l2_arl_sw_entry_t *)old_arl;
    sal_memcpy(&old_arl_temp, old, sizeof(l2_arl_sw_entry_t));

    stime = sal_time_usecs();
    
    rv = _drv_arlsync_compare(unit, 
            (uint32 *)old_arl, (uint32 *)new_arl,  &result_flags);
    if (rv != SOC_E_NONE){
        ARL_SW_TABLE_UNLOCK(soc);
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "soc_arl_sync: unexpect error! (rv=%d)\n"), rv));
        return SOC_E_INTERNAL;
    }

    if (result_flags != DRV_ARLCMP_NODIFF){
        if (result_flags & DRV_ARLCMP_VALID_CHANGE_MASK){
            if (result_flags == DRV_ARLCMP_INSERT){
                cb_level = _DRV_SWARL_INSERT_CB;

                del_hash_duplicate = 1;
            } else if (result_flags == DRV_ARLCMP_DELETE){
                cb_level = _DRV_SWARL_DELETE_CB;
            } else if (result_flags == DRV_ARLCMP_VALID_PENDING){
                cb_level = _DRV_SWARL_MODIFY_CB;
            } else {
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "%s, unexpected compare result(flags=0x%x)\n"),
                          FUNCTION_NAME(), result_flags));
                cb_level = _DRV_SWARL_MODIFY_CB;
            }
        } else if (result_flags & DRV_ARLCMP_NONE_VALID_AGE_CHANGED){
            cb_level = _DRV_SWARL_MODIFY_CB;

            /* MAC+VID change will be marked as modify here */
             del_hash_duplicate = 1;
        } else {
            if (result_flags & DRV_ARLCMP_AGE_CHANGED){
                /* the age difference will be treated as no change. */
                cb_level = _DRV_SWARL_NO_CB;
            } else if (result_flags & DRV_ARLCMP_UNEXPECT){
                /* possible condition is DRV_ARLCMP_UNEXPECT */ 
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "%s, unexpected compare result(flags=0x%x)\n"),
                          FUNCTION_NAME(), result_flags));
                cb_level = _DRV_SWARL_MODIFY_CB;
            }
        }

        /* for SDK-29690 : Incorrect L2 callback occured when many l2 entries
         *      burst in on the fly during HW L2 fast aging.
         *
         * - Remove the redundant l2 entry with the same MAC+VID in other 
         *    SW ARL address and a callback will be issued for this action.
         */

        if (del_hash_duplicate) {
            temp_key = *key;
            rv = _drv_arl_add_sanity_process(unit, &temp_key, new_arl);
            if (rv == SOC_E_EXISTS){
                LOG_INFO(BSL_LS_SOC_ARLMON,
                         (BSL_META_U(unit,
                                     "SW arl at id=%d is removed to prevent duplicat ADD\n"),
                          temp_key));
                rv = SOC_E_NONE;
            }
        }

        /* SW arl sync :
         *  - Do sync process even only DRV_ARLCMP_AGE_CHANGED flags set.
         */
        sal_memcpy(old_arl, new_arl, sizeof(l2_arl_sw_entry_t));

    } else {
        cb_level = _DRV_SWARL_NO_CB;
    }
    ARL_SW_TABLE_UNLOCK(soc);

    etime = sal_time_usecs();

    if (cb_level == _DRV_SWARL_MODIFY_CB) {
        soc_robo_arl_callback(unit, &old_arl_temp,
                (l2_arl_sw_entry_t *)new_arl);
    } else if (cb_level == _DRV_SWARL_DELETE_CB) {
        soc_robo_arl_callback(unit, &old_arl_temp, NULL);
    } else if (cb_level == _DRV_SWARL_INSERT_CB) {
        soc_robo_arl_callback(unit, NULL, (l2_arl_sw_entry_t *)new_arl);
    } else {
        /* no callback */
    }
    
    /* Use debug source ARLMON to avoid any debug messge for ARL */
    if (cb_level != _DRV_SWARL_NO_CB){
        LOG_INFO(BSL_LS_SOC_ARLMON,
                 (BSL_META_U(unit,
                             "soc_arl_sync: done in %d usec (cb=%d)\n"),
                  SAL_USECS_SUB(etime, stime), cb_level));
    }
    return rv;
}


