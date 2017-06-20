/*
 * $Id: vm.c,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <assert.h>
#include <soc/types.h>
#include <soc/error.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/drv_if.h>
#include <soc/vm.h>


#define FIX_MEM_ORDER_E(v,m) (((m)->flags & SOC_MEM_FLAG_BE) ? BYTES2WORDS((m)->bytes)-1-(v) : (v))

static int ivm_qset[] = { DRV_VM_QUAL_IVM_PORT_PROFILE,
                                  DRV_VM_QUAL_IVM_PORT_ID,
                                  DRV_VM_QUAL_IVM_INGRESS_STAG_STAUS,
                                  DRV_VM_QUAL_IVM_INGRESS_CTAG_STAUS,
                                  DRV_VM_QUAL_IVM_INGRESS_SVID,
                                  DRV_VM_QUAL_IVM_INGRESS_CVID_RANGE,
                                  DRV_VM_QUAL_IVM_INGRESS_CVID,
                                  DRV_VM_QUAL_IVM_INGRESS_SPCP,
                                  DRV_VM_QUAL_IVM_INGRESS_CPCP,
                                  DRV_VM_QUAL_IVM_INGRESS_FRAME_TYPE,
                                  DRV_VM_QUAL_IVM_INGRESS_ETHER_TYPE,
                                  DRV_VM_QUAL_INVALID};

static int evm_qset[] = { DRV_VM_QUAL_EVM_INGRESS_PORT_ID,
                                  DRV_VM_QUAL_EVM_INGRESS_VPORT_ID,
                                  DRV_VM_QUAL_EVM_FLOW_ID,
                                  DRV_VM_QUAL_EVM_EGRESS_PORT_ID,
                                  DRV_VM_QUAL_EVM_EGRESS_VPORT_ID,
                                  DRV_VM_QUAL_INVALID};

/*
 * Function: _drv_tbx_vm_field_mapping
 *
 * Purpose:
 *     Translate the driver field type to chip field index.
 *
 * Parameters:
 *     unit - BCM device number
 *     field_type - driver field value
 *     field_id (OUT) - chip field index
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_BADID - unknown driver field value
 */
STATIC int
_drv_tbx_vm_field_mapping(int unit, uint32 field_type, uint32 *field_id)
{
    int rv = SOC_E_NONE;

    switch (field_type) {
        /* Key fields */
        case DRV_VM_FIELD_IVM_VALID:
            *field_id = INDEX(VALIDf);
            break;
        case DRV_VM_FIELD_IVM_INGRESS_PORT_PROFILE:
            *field_id = INDEX(INGRESS_PORT_PROFILEf);
            break;
        case DRV_VM_FIELD_IVM_INGRESS_PORT_ID:
            *field_id = INDEX(INGRESS_PORT_IDf);
            break;
        case DRV_VM_FIELD_IVM_INGRESS_STAG_STAUS:
            *field_id = INDEX(INGRESS_STAG_STSf);
            break;
        case DRV_VM_FIELD_IVM_INGRESS_CTAG_STATUS:
            *field_id = INDEX(INGRESS_CTAG_STSf);
            break;
        case DRV_VM_FIELD_IVM_INGRESS_SVID:
            *field_id = INDEX(INGRESS_SVIDf);
            break;
        case DRV_VM_FIELD_IVM_INGRESS_CVID:
            *field_id = INDEX(INGRESS_CVIDf);
            break;
        case DRV_VM_FIELD_IVM_INGRESS_CVID_RANGE:
            *field_id = INDEX(INGRESS_CVID_RANGEf);
            break;
        case DRV_VM_FIELD_IVM_INGRESS_SPCP:
            *field_id = INDEX(INGRESS_SPCPf);
            break;
        case DRV_VM_FIELD_IVM_INGRESS_CPCP:
            *field_id = INDEX(INGRESS_CPCPf);
            break;
        case DRV_VM_FIELD_IVM_FRAME_TYPE:
            *field_id = INDEX(FRAMING_TYPEf);
            break;
        case DRV_VM_FIELD_IVM_ETHER_TYPE:
            *field_id = INDEX(ETH_TYPEf);
            break;
        case DRV_VM_FIELD_IVM_RESERVED:
            *field_id = INDEX(RESERVED_2Rf);
            break;

        case DRV_VM_FIELD_EVM_VALID:
            *field_id = INDEX(VALIDf);
            break;
        case DRV_VM_FIELD_EVM_INGRESS_PORT_ID:
            *field_id = INDEX(INGRESS_PORT_IDf);
            break;
        case DRV_VM_FIELD_EVM_INGRESS_VPORT_ID:
            *field_id = INDEX(INGRESS_VPORT_IDf);
            break;
        case DRV_VM_FIELD_EVM_FLOW_ID:
            *field_id = INDEX(FLOW_IDf);
            break;
        case DRV_VM_FIELD_EVM_EGRESS_PORT_ID:
            *field_id = INDEX(EGRESS_PORT_IDf);
            break;
        case DRV_VM_FIELD_EVM_EGRESS_VPORT_ID:
            *field_id = INDEX(EGRESS_VPORT_IDf);
            break;

        /* Action fields */
        case DRV_VM_FIELD_IVM_SPCP_MARKING_POLICY:
            *field_id = INDEX(SPCP_MARKf);
            break;
        case DRV_VM_FIELD_IVM_CPCP_MARKING_POLICY:
            *field_id = INDEX(CPCP_MARKf);
            break;
        case DRV_VM_FIELD_IVM_VLAN_ID:
            *field_id = INDEX(NEW_VIDf);
            break;
        case DRV_VM_FIELD_IVM_FLOW_ID:
            *field_id = INDEX(FLOW_IDf);
            break;
        case DRV_VM_FIELD_IVM_VPORT_ID:
            *field_id = INDEX(VPORT_IDf);
            break;
        case DRV_VM_FIELD_IVM_VPORT_SPCP:
            *field_id = INDEX(VPORT_SPCPf);
            break;
        case DRV_VM_FIELD_IVM_VPORT_CPCP:
            *field_id = INDEX(VPORT_CPCPf);
            break;
        case DRV_VM_FIELD_IVM_VPORT_DP:
            *field_id = INDEX(VPORT_DPf);
            break;
        case DRV_VM_FIELD_IVM_VPORT_TC:
            *field_id = INDEX(VPORT_TCf);
            break;

        case DRV_VM_FIELD_EVM_STAG_ACT:
            *field_id = INDEX(STAG_ACTf);
            break;
        case DRV_VM_FIELD_EVM_CTAG_ACT:
            *field_id = INDEX(CTAG_ACTf);
            break;
        case DRV_VM_FIELD_EVM_NEW_SVID:
            *field_id = INDEX(NEW_SVIDf);
            break;
        case DRV_VM_FIELD_EVM_NEW_CVID:
            *field_id = INDEX(NEW_CVIDf);
            break;
        case DRV_VM_FIELD_EVM_OUTER_PCP_REPLACE:
            *field_id = INDEX(CHG_SPCPf);
            break;
        case DRV_VM_FIELD_EVM_NEW_OUTER_PCP:
            *field_id = INDEX(NEW_SPCPf);
            break;
        case DRV_VM_FIELD_EVM_INNER_PCP_REPLACE:
            *field_id = INDEX(CHG_CPCPf);
            break;
        case DRV_VM_FIELD_EVM_NEW_INNER_PCP:
            *field_id = INDEX(NEW_CPCPf);
            break;
        default:
            rv = SOC_E_BADID;
    }

    return rv;
}

/*
 * Function: _drv_tbx_vm_read
 *
 * Purpose:
 *     Read the IVM/EVM raw data by ram type from chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     ram_type - ram type (KEY/ACT)
 *     index -entry index
 *     entry(OUT) -IVM/EVM entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
STATIC int
_drv_tbx_vm_read(int unit, uint32 ram_type, uint32 index, drv_vm_entry_t *entry)
{
    int rv = SOC_E_NONE;
    ivm_key_data_mask_entry_t vm;
    uint64 fld_data;

    assert(entry);

    COMPILER_64_ZERO(fld_data);
    
    switch (ram_type) {
    case DRV_VM_RAM_IVM_KEY_DATA:
    case DRV_VM_RAM_EVM_KEY_DATA:
        /*
          * Use DRV_MEM_IVM/EVM_KEY_DATA_MASK memory format, which
          * is a general format for IVM and EVM, to get the data and mask 
          * fields seperately.
          *
          */

        if (ram_type == DRV_VM_RAM_IVM_KEY_DATA) {
            rv = MEM_READ_IVM_KEY_DATA_MASKm(unit, index, (void *)&vm);
            SOC_IF_ERROR_RETURN(rv);
            rv = soc_IVM_KEY_DATA_MASKm_field_get(unit, (void *)&vm, 
                VM_KEY_DATAf, (void *)&fld_data);
            SOC_IF_ERROR_RETURN(rv);
        } else {
            rv = MEM_READ_EVM_KEY_DATA_MASKm(unit, index, (void *)&vm);
            SOC_IF_ERROR_RETURN(rv);
            rv = soc_IVM_KEY_DATA_MASKm_field_get(unit, (void *)&vm, 
                VM_KEY_DATAf, (void *)&fld_data);
            SOC_IF_ERROR_RETURN(rv);

        }

        entry->key_data[1] = COMPILER_64_HI(fld_data);
        entry->key_data[0] = COMPILER_64_LO(fld_data);

        if (ram_type == DRV_VM_RAM_IVM_KEY_DATA) {
            rv = soc_IVM_KEY_DATA_MASKm_field_get(unit, (void *)&vm, 
                VM_KEY_MASKf, (void *)&fld_data);
            SOC_IF_ERROR_RETURN(rv);
        } else {
            rv = soc_EVM_KEY_DATA_MASKm_field_get(unit, (void *)&vm, 
                VM_KEY_MASKf, (void *)&fld_data);
            SOC_IF_ERROR_RETURN(rv);
        }

        entry->key_mask[1] = COMPILER_64_HI(fld_data);
        entry->key_mask[0] = COMPILER_64_LO(fld_data);
        break;

    case DRV_VM_RAM_IVM_ACT:
    case DRV_VM_RAM_EVM_ACT:
        if (ram_type == DRV_VM_RAM_IVM_ACT) {
            rv = MEM_READ_IVM_ACTm(unit, index, (void *)&fld_data);
            SOC_IF_ERROR_RETURN(rv);
        } else {
            rv = MEM_READ_EVM_ACTm(unit, index, (void *)&fld_data);
            SOC_IF_ERROR_RETURN(rv);
        }
#ifdef BE_HOST
        entry->act_data[0] = COMPILER_64_HI(fld_data);
        entry->act_data[1] = COMPILER_64_LO(fld_data);
#else
        entry->act_data[0] = COMPILER_64_LO(fld_data);
        entry->act_data[1] = COMPILER_64_HI(fld_data);
#endif
        break;
    default:
        rv = SOC_E_UNAVAIL;
        break;
    }

    return rv;
}

/*
 * Function: _drv_tbx_vm_write
 *
 * Purpose:
 *     Write the IVM/EVM raw data by ram type to chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     ram_type - ram type (KEY/ACT)
 *     index -entry index
 *     entry -IVM/EVM entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
STATIC int
_drv_tbx_vm_write(int unit, uint32 ram_type, uint32 index, drv_vm_entry_t *entry)
{
    int rv = SOC_E_NONE;
    ivm_key_data_mask_entry_t vm;
    uint64 fld_data;
    uint32 val32_hi, val32_lo;
    
    assert(entry);

    sal_memset(&vm, 0, sizeof(ivm_key_data_mask_entry_t));
    
    /*
     * Perform TCAM write operation 
     */
    switch (ram_type) {
    case DRV_VM_RAM_IVM_KEY_DATA:
    case DRV_VM_RAM_EVM_KEY_DATA:
        /*
          * Use DRV_MEM_IVM/EVM_KEY_DATA_MASK memory format, which
          * is a general format for IVM and EVM include both data and mask 
          * fields, to create the memory entry that going to be set to registers.
          */

        val32_hi = entry->key_data[1];
        val32_lo = entry->key_data[0];

        COMPILER_64_SET(fld_data, val32_hi, val32_lo);
        if (ram_type == DRV_VM_RAM_IVM_KEY_DATA) {          
            rv = soc_IVM_KEY_DATA_MASKm_field_set(unit, (void *)&vm, 
                VM_KEY_DATAf, (void *)&fld_data);
            SOC_IF_ERROR_RETURN(rv);
        } else {
            rv = soc_EVM_KEY_DATA_MASKm_field_set(unit, (void *)&vm, 
                VM_KEY_DATAf, (void *)&fld_data);
            SOC_IF_ERROR_RETURN(rv);
        }

        val32_hi = entry->key_mask[1];
        val32_lo = entry->key_mask[0];

        COMPILER_64_SET(fld_data, val32_hi, val32_lo);

        if (ram_type == DRV_VM_RAM_IVM_KEY_DATA) {          
            rv = soc_IVM_KEY_DATA_MASKm_field_set(unit, (void *)&vm, 
                VM_KEY_MASKf, (void *)&fld_data);
            SOC_IF_ERROR_RETURN(rv);
            rv = MEM_WRITE_IVM_KEY_DATA_MASKm(unit, index, (void *)&vm);

        } else {
            rv = soc_EVM_KEY_DATA_MASKm_field_set(unit, (void *)&vm, 
                VM_KEY_MASKf, (void *)&fld_data);
            SOC_IF_ERROR_RETURN(rv);
            rv = MEM_WRITE_EVM_KEY_DATA_MASKm(unit, index, (void *)&vm);
        }
        break;

    case DRV_VM_RAM_IVM_ACT:
    case DRV_VM_RAM_EVM_ACT:
#ifdef BE_HOST
        val32_hi = entry->act_data[0];
        val32_lo = entry->act_data[1];
#else
        val32_hi = entry->act_data[1];
        val32_lo = entry->act_data[0];
#endif
        COMPILER_64_SET(fld_data, val32_hi, val32_lo);
        if (ram_type == DRV_VM_RAM_IVM_ACT) {
            rv = MEM_WRITE_IVM_ACTm(unit, index, (void *)&fld_data);
        } else {
            rv = MEM_WRITE_EVM_ACTm(unit, index, (void *)&fld_data);
        }
        break;
    default:
        rv = SOC_E_PARAM;
    }

    return rv;
}

STATIC int
_drv_tbx_vm_field_set(int unit, uint32 mem_type, uint32 field_type, 
            drv_vm_entry_t* entry, uint32* fld_val)
{
    int rv = SOC_E_NONE;
    int mem_id;
    uint32  fld_id;
    uint32  mask, mask_hi, mask_lo;
    soc_mem_info_t  *meminfo;
    soc_field_info_t    *fieldinfo;
    int i, wp, bp, len;
    uint32  *data_p;

    assert(entry);
    assert(fld_val);

    switch (mem_type) {
    case DRV_VM_RAM_IVM_KEY_DATA:
    case DRV_VM_RAM_IVM_KEY_MASK:
        mem_id = INDEX(IVM_KEY_DATA_2m);
        if (mem_type == DRV_VM_RAM_IVM_KEY_DATA) {
            data_p = entry->key_data;
        } else { /* mask data */
            data_p = entry->key_mask;
        }
        break;
    case DRV_VM_RAM_IVM_ACT:
        mem_id = INDEX(IVM_ACTm);
        data_p = entry->act_data;
        break;

    case DRV_VM_RAM_EVM_KEY_DATA:
    case DRV_VM_RAM_EVM_KEY_MASK:
        mem_id = INDEX(EVM_KEY_DATAm);

        if (mem_type == DRV_VM_RAM_EVM_KEY_DATA) {
            data_p = entry->key_data;
        } else { /* mask data */
            data_p = entry->key_mask;
        }
        break;
    case DRV_VM_RAM_EVM_ACT:
        mem_id = INDEX(EVM_ACTm);
        data_p = entry->act_data;
        break;

    default:
        rv = SOC_E_PARAM;
        return rv;
    }
    
    if (( rv = _drv_tbx_vm_field_mapping(unit, field_type, &fld_id)) < 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "drv_vm_field_set : UNKNOW FIELD ID. \n")));
        return rv;
    }

    assert(SOC_MEM_IS_VALID(unit, mem_id));
    meminfo = &SOC_MEM_INFO(unit, mem_id);
    SOC_FIND_FIELD(fld_id, meminfo->fields,
                             meminfo->nFields, fieldinfo);
    assert(fieldinfo);

    bp = fieldinfo->bp;
    if (fieldinfo->flags & SOCF_LE) {
        wp = bp / 32;
        bp = bp & (32 - 1);
        i = 0;

        for (len = fieldinfo->len; len > 0; len -= 32) {
            /* mask covers all bits in field. */
            /* if the field is wider than 32, takes 32 bits in each iteration */
            if (len >= 32) {
                mask = 0xffffffff;
            } else {
                mask = (1 << len) - 1;
            }
            /* the field may be splited across a 32-bit word boundary. */
            /* assume bp=0 to start with. Therefore, mask for higer word is 0 */
            mask_lo = mask;
            mask_hi = 0;

            /* if field is not aligned with 32-bit word boundary */
            /* adjust hi and lo masks accordingly. */
            if (bp) {
                mask_lo = mask << bp;
                mask_hi = mask >> (32 - bp);
            }

            /* set field value --- 32 bits each time */
            data_p[FIX_MEM_ORDER_E(wp, meminfo)] &= ~mask_lo;
            data_p[FIX_MEM_ORDER_E(wp++, meminfo)] |= 
                ((fld_val[i] << bp) & mask_lo);
            if (mask_hi) {
                data_p[FIX_MEM_ORDER_E(wp, meminfo)] &= ~(mask_hi);
                data_p[FIX_MEM_ORDER_E(wp, meminfo)] |= 
                    ((fld_val[i] >> (32 - bp)) & mask_hi);
            }

            i++;
        }
    } else {                   
        /* Big endian: swap bits */
        len = fieldinfo->len;

        while (len > 0) {
            len--;
            data_p[FIX_MEM_ORDER_E(bp / 32, meminfo)] &= ~(1 << (bp & (32-1)));
            data_p[FIX_MEM_ORDER_E(bp / 32, meminfo)] |=
            (fld_val[len / 32] >> (len & (32-1)) & 1) << (bp & (32-1));
            bp++;
        }
    }

    return rv;
}

STATIC int
_drv_tbx_vm_field_get(int unit, uint32 mem_type, uint32 field_type, 
            drv_vm_entry_t* entry, uint32* fld_val)
{
    int rv = SOC_E_NONE;
    int mem_id;
    uint32  fld_id;
    uint32  mask = -1;
    uint32  mask_hi, mask_lo;
    soc_mem_info_t  *meminfo;
    soc_field_info_t    *fieldinfo;
    int i, wp, bp, len;
    uint32  *data_p;
    
    assert(entry);
    assert(fld_val);

    switch (mem_type) {
    case DRV_VM_RAM_IVM_KEY_DATA:
    case DRV_VM_RAM_IVM_KEY_MASK:
        mem_id = INDEX(IVM_KEY_DATA_2m);
        if (mem_type == DRV_VM_RAM_IVM_KEY_DATA) {
            data_p = entry->key_data;
        } else { /* mask data */
            data_p = entry->key_mask;
        }
        break;
    case DRV_VM_RAM_IVM_ACT:
        mem_id = INDEX(IVM_ACTm);
        data_p = entry->act_data;
        break;

    case DRV_VM_RAM_EVM_KEY_DATA:
    case DRV_VM_RAM_EVM_KEY_MASK:
        mem_id = INDEX(EVM_KEY_DATAm);

        if (mem_type == DRV_VM_RAM_EVM_KEY_DATA) {
            data_p = entry->key_data;
        } else { /* mask data */
            data_p = entry->key_mask;
        }
        break;
    case DRV_VM_RAM_EVM_ACT:
        mem_id = INDEX(EVM_ACTm);
        data_p = entry->act_data;
        break;

    default:
        rv = SOC_E_PARAM;
        return rv;
    }
    if (( rv = _drv_tbx_vm_field_mapping(unit, field_type, &fld_id)) < 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "drv_vm_field_get : UNKNOW FIELD ID. \n")));
        return rv;
    }

    assert(SOC_MEM_IS_VALID(unit, mem_id));
    meminfo = &SOC_MEM_INFO(unit, mem_id);
    SOC_FIND_FIELD(fld_id, meminfo->fields,
                             meminfo->nFields, fieldinfo);
    assert(fieldinfo);
    bp = fieldinfo->bp;

    wp = bp / 32;
    bp = bp & (32 - 1);
    len = fieldinfo->len;

    /* field is 1-bit wide */
    if (len == 1) {
        fld_val[0] = ((data_p[FIX_MEM_ORDER_E(wp, meminfo)] >> bp) & 1);
    } else {
        if (fieldinfo->flags & SOCF_LE) {
            for (i = 0; len > 0; len -= 32) {
            /* mask covers all bits in field. */
            /* if the field is wider than 32, takes 32 bits in each iteration */
                if (len >= 32) {
                    mask = 0xffffffff;
                } else {
                    mask = (1 << len) - 1;
                }
            /* the field may be splited across a 32-bit word boundary. */
            /* assume bp=0 to start with. Therefore, mask for higer word is 0 */
                mask_lo = mask;
                mask_hi = 0;
                /* if field is not aligned with 32-bit word boundary */
                /* adjust hi and lo masks accordingly. */
                if (bp) {
                    mask_lo = mask << bp;
                    mask_hi = mask >> (32 - bp);
                }
                /* get field value --- 32 bits each time */
                fld_val[i] = (data_p[FIX_MEM_ORDER_E(wp++, meminfo)] 
                    & mask_lo) >> bp;
                if (mask_hi) {
                    fld_val[i] |= (data_p[FIX_MEM_ORDER_E(wp, meminfo)] 
                        & mask_hi) << (32 - bp);
                }
                i++;
            }
        } else {
            i = (len - 1) / 32;

            while (len > 0) {
                assert(i >= 0);
                fld_val[i] = 0;
                do {
                    fld_val[i] = (fld_val[i] << 1) |
                    ((data_p[FIX_MEM_ORDER_E(bp / 32, meminfo)] >>
                    (bp & (32 - 1))) & 1);
                    len--;
                    bp++;
                } while (len & (32 - 1));
                i--;
            }
        }
    }
    return rv;
}

/*
 * Function: drv_tbx_vm_init
 *
 * Purpose:
 *     Initialize the IVM/EVM module. 
 *
 * Parameters:
 *     unit - BCM device number
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 */
int 
drv_tbx_vm_init(int unit)
{
    
    /* Clear HW TABLE */
    SOC_IF_ERROR_RETURN(DRV_MEM_CLEAR(unit, DRV_MEM_IVM_KEY));
    SOC_IF_ERROR_RETURN(DRV_MEM_CLEAR(unit, DRV_MEM_IVM_ACT));
    SOC_IF_ERROR_RETURN(DRV_MEM_CLEAR(unit, DRV_MEM_EVM_KEY));
    SOC_IF_ERROR_RETURN(DRV_MEM_CLEAR(unit, DRV_MEM_EVM_ACT));

    return SOC_E_NONE;
}

/*
 * Function: drv_tbx_vm_deinit
 *
 * Purpose:
 *     De-initialize the IVM/EVM module. 
 *
 * Parameters:
 *     unit - BCM device number
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 */
int 
drv_tbx_vm_deinit(int unit)
{

    return SOC_E_NONE;
}

/*
 * Function: drv_tbx_vm_qset_get
 *
 * Purpose:
 *     Get the qualify bit value from IVM/EVM entry. 
 *
 * Parameters:
 *     unit - BCM device number
 *     qual - qualify ID
 *     entry -IVM/EVM entry
 *     val(OUT) -TRUE/FALSE
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv_tbx_vm_qset_get(int unit, uint32 qual, drv_vm_entry_t *entry, uint32 *val)
{
    int rv = SOC_E_NONE;
    uint32  wp, bp;

    assert(entry);
    
    wp = qual / 32;
    bp = qual & (32 - 1);
    if (entry->w[wp] & (1 << bp)) {
        *val = TRUE;
    } else {
        *val = FALSE;
    }

    return rv;
}

/*
 * Function: drv_tbx_vm_qset_set
 *
 * Purpose:
 *     Set/Reset the qualify bit value to IVM/EVM entry. 
 *
 * Parameters:
 *     unit - BCM device number
 *     qual - qualify ID
 *     entry -IVM/EVM entry
 *     val -TRUE/FALSE
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv_tbx_vm_qset_set(int unit, uint32 qual, drv_vm_entry_t *entry, uint32 val)
{
    int rv = SOC_E_NONE;
    uint32  wp, bp, temp = 0;

    assert(entry);
    
    wp = qual / 32;
    bp = qual & (32 - 1);
    if (val) {
        temp = 1;
    }

    if (temp) {
        entry->w[wp] |= (1 << bp);
    } else {
        entry->w[wp] &= ~(1 << bp);
    }
    
    return rv;
}

/*
 * Function: drv_tbx_vm_format_id_select
 *
 * Purpose:
 *     According to this entry's fields to select which format id used for this entry.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - driver IVM/EVM entry
 *     id(OUT) - format id for this entry
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_RESOURCE - Can't found suitable format id for this entry.
 */
int
drv_tbx_vm_format_id_select(int unit, drv_vm_entry_t *entry, uint32 *id, uint32 flags)
{

   *id = 0;

   return SOC_E_NONE;
}

/*
 * Function: drv_tbx_vm_format_to_qset
 *
 * Purpose:
 *     According to slice id used for this entry.
 *
 * Parameters:
 *     unit - BCM device number
 *     mem_type - driver ram type. Available options are DRV_VM_RAM_IVM and DRV_VM_RAM_EVM
 *     id - format id 
 *     entry(OUT) - driver IVM/EVM entry
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - un-support format id.
 */
int
drv_tbx_vm_format_to_qset(int unit, uint32 mem_type, uint32 id, drv_vm_entry_t *entry)
{
    uint32 i;
    uint32  format[(DRV_VM_QUAL_COUNT / 32) + 1];
    int     *qset;


    switch(mem_type) {
        case DRV_VM_RAM_IVM:
            qset = ivm_qset;
            break;
        case DRV_VM_RAM_EVM:
            qset = evm_qset;
            break;
        default:
            return SOC_E_PARAM;
    }

    for (i=0; i < (DRV_VM_QUAL_COUNT / 32) + 1; i++) {
        format[i] = 0;
    }
    i = 0;
    while (qset[i] != DRV_VM_QUAL_INVALID) {
        format[(qset[i]/32)] |= (0x1 << (qset[i] % 32));
        i++;
    }
    for (i = 0; i < (DRV_VM_QUAL_COUNT / 32) + 1; i++) {
        entry->w[i] = format[i];
    }

    return SOC_E_NONE;
}

/*
 * Function: drv_tbx_vm_field_get
 *
 * Purpose:
 *     Get the field value from the IVM/EVM entry raw data.
 *
 * Parameters:
 *     unit - BCM device number
 *     mem_type - driver ram type (Key/Action)
 *     field_type -driver IVM/EVM field type
 *     entry -IVM/EVM entry
 *     fld_val(OUT) - field value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv_tbx_vm_field_get(int unit, uint32 mem_type, uint32 field_type, 
            drv_vm_entry_t* entry, uint32* fld_val)
{
    int rv = SOC_E_NONE;
    uint32  temp;
    
    assert(entry);
    assert(fld_val);

    switch (mem_type) {
        case DRV_VM_RAM_IVM_KEY_DATA:
        case DRV_VM_RAM_IVM_KEY_MASK:
            /* Process combined fields */
            if (field_type == DRV_VM_FIELD_IVM_INGRESS_SVID) {
                /* 
                 * Ingress_SVID and RESERVED_2R+Ingress_SPCP+Ingress_CPCP+Frame_Type shares
                 * the same field in IVM Key. Aassemble Ingress_SVID from the 4 corresponding 
                 * fields of IVM_KEY_DATAm.
                 */
                /* Get bit 0~1 of Ingress SVID from FrameType field */
                /* coverity[callee_ptr_arith] */
                 rv = _drv_tbx_vm_field_get(unit, mem_type, DRV_VM_FIELD_IVM_FRAME_TYPE, entry, &temp);
                 if (rv < 0) {
                    *fld_val = 0;
                     return rv;
                 }
                *fld_val = temp;
                /* Get bit 2~4 of Ingress SVID from IngressCPCP field */
                /* coverity[callee_ptr_arith] */
                 rv = _drv_tbx_vm_field_get(unit, mem_type, DRV_VM_FIELD_IVM_INGRESS_CPCP, entry, &temp);
                 if (rv < 0) {
                    *fld_val = 0;
                     return rv;
                 }
                *fld_val |= temp << 2;
                /* Get bit 5~7 of Ingress SVID from IngressSPCP field */
                /* coverity[callee_ptr_arith] */
                 rv = _drv_tbx_vm_field_get(unit, mem_type, DRV_VM_FIELD_IVM_INGRESS_SPCP, entry, &temp);
                 if (rv < 0) {
                    *fld_val = 0;
                     return rv;
                 }
                *fld_val |= temp << 5;
                /* Get bit 8~11 of Ingress SVID from IVMReserved field */
                /* coverity[callee_ptr_arith] */
                 rv = _drv_tbx_vm_field_get(unit, mem_type, DRV_VM_FIELD_IVM_RESERVED, entry, &temp);
                 if (rv < 0) {
                    *fld_val = 0;
                     return rv;
                 }
                *fld_val |= temp << 8;

                 /* Done assemble process, return */
                 return rv;
            } else if (field_type == DRV_VM_FIELD_IVM_ETHER_TYPE) {
                /* 
                 * Ether_Type and Ingress_CVID Range+Ingress_CVID shares
                 * the same field in IVM Key. Assemble Ether_Type from the 2 corresponding 
                 * fields of IVM_KEY_DATAm.
                 */
                /* Get bit 0~11 of Ether_Type from Ingress_CVID field */
                /* coverity[callee_ptr_arith] */
                 rv = _drv_tbx_vm_field_get(unit, mem_type, DRV_VM_FIELD_IVM_INGRESS_CVID, entry, &temp);
                 if (rv < 0) {
                    *fld_val = 0;
                     return rv;
                 }
                *fld_val = temp;
                /* Get bit 12~15 of Ether_Type from Ingress_CVID Range field */
                /* coverity[callee_ptr_arith] */
                 rv = _drv_tbx_vm_field_get(unit, mem_type, DRV_VM_FIELD_IVM_INGRESS_CVID_RANGE, entry, &temp);
                 if (rv < 0) {
                    *fld_val = 0;
                     return rv;
                 }
                *fld_val |= temp << 12;

                 /* Done assemble process, return */
                 return rv;
            }
            break;
        default:
            break;
    }

    rv = _drv_tbx_vm_field_get(unit, mem_type, field_type, entry, fld_val);
    return rv;
}

/*
 * Function: drv_tbx_vm_field_set
 *
 * Purpose:
 *     Set the field value to the IVM/EVM entry raw data.
 *
 * Parameters:
 *     unit - BCM device number
 *     mem_type - driver ram type (Key/Action)
 *     field_type -driver IVM/EVM field type
 *     entry(OUT) -IVM/EVM entry
 *     fld_val - field value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv_tbx_vm_field_set(int unit, uint32 mem_type, uint32 field_type, 
            drv_vm_entry_t* entry, uint32* fld_val)
{
    int rv = SOC_E_NONE;
    uint32  temp;

    assert(entry);
    assert(fld_val);

    switch (mem_type) {
        case DRV_VM_RAM_IVM_KEY_DATA:
        case DRV_VM_RAM_IVM_KEY_MASK:
            /* Process combined fields */
            if (field_type == DRV_VM_FIELD_IVM_INGRESS_SVID) {
                /* 
                 * Ingress_SVID and RESERVED_2R+Ingress_SPCP+Ingress_CPCP+Frame_Type shares
                 * the same field in IVM Key. Disassemble Ingress_SVID into 4 pieces and
                 * set into the corresponding fields of IVM_KEY_DATAm.
                 */
                /* Set bit 0~1 of Ingress SVID to FrameType field */
                temp = *fld_val & 0x3;
                /* coverity[callee_ptr_arith] */
                 rv = _drv_tbx_vm_field_set(unit, mem_type, DRV_VM_FIELD_IVM_FRAME_TYPE, entry, &temp);
                 if (rv < 0) {
                     return rv;
                 }
                /* Set bit 2~4 of Ingress SVID to IngressCPCP field */
                temp = (*fld_val >> 2) & 0x7;
                /* coverity[callee_ptr_arith] */
                 rv = _drv_tbx_vm_field_set(unit, mem_type, DRV_VM_FIELD_IVM_INGRESS_CPCP, entry, &temp);
                 if (rv < 0) {
                     return rv;
                 }
                /* Set bit 5~7 of Ingress SVID to IngressSPCP field */
                temp = (*fld_val >> 5) & 0x7;
                /* coverity[callee_ptr_arith] */
                 rv = _drv_tbx_vm_field_set(unit, mem_type, DRV_VM_FIELD_IVM_INGRESS_SPCP, entry, &temp);
                 if (rv < 0) {
                     return rv;
                 }
                /* Set bit 8~11 of Ingress SVID to IVMReserved field */
                temp = (*fld_val >> 8) & 0xf;
                /* coverity[callee_ptr_arith] */
                 rv = _drv_tbx_vm_field_set(unit, mem_type, DRV_VM_FIELD_IVM_RESERVED, entry, &temp);

                 /* Done disassemble process, return */
                 return rv;
            } else if (field_type == DRV_VM_FIELD_IVM_ETHER_TYPE) {
                /* 
                 * Ether_Type and Ingress_CVID Range+Ingress_CVID shares
                 * the same field in IVM Key. Disassemble Ether_Type into 2 pieces and
                 * set into the corresponding fields of IVM_KEY_DATAm.
                 */
                /* Set bit 0~11 of Ether_Type to Ingress_CVID field */
                temp = *fld_val & 0xfff;
                /* coverity[callee_ptr_arith] */
                 rv = _drv_tbx_vm_field_set(unit, mem_type, DRV_VM_FIELD_IVM_INGRESS_CVID, entry, &temp);
                 if (rv < 0) {
                     return rv;
                 }
                /* Set bit 12~15 of Ether_Type to Ingress_CVID Range field */
                temp = (*fld_val >> 12) & 0xf;
                /* coverity[callee_ptr_arith] */
                 rv = _drv_tbx_vm_field_set(unit, mem_type, DRV_VM_FIELD_IVM_INGRESS_CVID_RANGE, entry, &temp);

                 /* Done disassemble process, return */
                 return rv;
            }
            break;
        default:
            break;
    }

    rv = _drv_tbx_vm_field_set(unit, mem_type, field_type, entry, fld_val);
    
    return rv;
}

/*
 * Function: drv_tbx_vm_action_get
 *
 * Purpose:
 *     Get the IVM/EVM action type and parameters value from 
 *     the raw data of ACTION ram.
 *
 * Parameters:
 *     unit - BCM device number
 *     action - driver action type
 *     entry -IVM/EVM entry
 *     act_param(OUT) - action paramter (if need)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - unknown action type
 *
 * Note:
 */
int
drv_tbx_vm_action_get(int unit, uint32 action, 
            drv_vm_entry_t* entry, uint32* act_param)
{
    int rv = SOC_E_NONE;
    uint32  fld_val = 0;

    switch (action) {
        case DRV_VM_ACT_IVM_SPCP_MARKING_POLICY:
        case DRV_VM_ACT_IVM_CPCP_MARKING_POLICY:
            if (action == DRV_VM_ACT_IVM_SPCP_MARKING_POLICY) {
                DRV_VM_FIELD_GET(unit, DRV_VM_RAM_IVM_ACT, 
                    DRV_VM_FIELD_IVM_SPCP_MARKING_POLICY, entry, &fld_val);
            } else {
                DRV_VM_FIELD_GET(unit, DRV_VM_RAM_IVM_ACT, 
                    DRV_VM_FIELD_IVM_CPCP_MARKING_POLICY, entry, &fld_val);
            }
            switch(fld_val) {
                case 0:
                    *act_param = DRV_VM_ACTION_PCP_MARK_MAPPED;
                    break;
                case 1:
                    *act_param = DRV_VM_ACTION_PCP_MARK_INGRESS_INNER_PCP;
                    break;
                case 2:
                    *act_param = DRV_VM_ACTION_PCP_MARK_INGRESS_OUTER_PCP;
                    break;
                case 3:
                    *act_param = DRV_VM_ACTION_PCP_MARK_PORT_DEFAULT;
                    break;
            }
            break;
        case DRV_VM_ACT_IVM_VLAN_ID:
            DRV_VM_FIELD_GET(unit, DRV_VM_RAM_IVM_ACT, 
                DRV_VM_FIELD_IVM_VLAN_ID, entry, &fld_val);
            *act_param = fld_val;
            break;
        case DRV_VM_ACT_IVM_FLOW_ID:
            DRV_VM_FIELD_GET(unit, DRV_VM_RAM_IVM_ACT, 
                DRV_VM_FIELD_IVM_FLOW_ID, entry, &fld_val);
            *act_param = fld_val;
            break;
        case DRV_VM_ACT_IVM_VPORT_ID:
            DRV_VM_FIELD_GET(unit, DRV_VM_RAM_IVM_ACT, 
                DRV_VM_FIELD_IVM_VPORT_ID, entry, &fld_val);
            *act_param = fld_val;
            break;
        case DRV_VM_ACT_IVM_VPORT_SPCP:
            DRV_VM_FIELD_GET(unit, DRV_VM_RAM_IVM_ACT, 
                DRV_VM_FIELD_IVM_VPORT_SPCP, entry, &fld_val);
            *act_param = fld_val;
            break;
        case DRV_VM_ACT_IVM_VPORT_CPCP:
            DRV_VM_FIELD_GET(unit, DRV_VM_RAM_IVM_ACT, 
                DRV_VM_FIELD_IVM_VPORT_CPCP, entry, &fld_val);
            *act_param = fld_val;
            break;
        case DRV_VM_ACT_IVM_VPORT_DP:
            DRV_VM_FIELD_GET(unit, DRV_VM_RAM_IVM_ACT, 
                DRV_VM_FIELD_IVM_VPORT_DP, entry, &fld_val);
            *act_param = fld_val;
            break;
        case DRV_VM_ACT_IVM_VPORT_TC:
            DRV_VM_FIELD_GET(unit, DRV_VM_RAM_IVM_ACT, 
                DRV_VM_FIELD_IVM_VPORT_TC, entry, &fld_val);
            *act_param = fld_val;
            break;

        case DRV_VM_ACT_EVM_STAG:
        case DRV_VM_ACT_EVM_CTAG:
            if (action == DRV_VM_ACT_EVM_STAG) {
                DRV_VM_FIELD_GET(unit, DRV_VM_RAM_EVM_ACT, 
                    DRV_VM_FIELD_EVM_STAG_ACT, entry, &fld_val);
            } else {
                DRV_VM_FIELD_GET(unit, DRV_VM_RAM_EVM_ACT, 
                    DRV_VM_FIELD_EVM_CTAG_ACT, entry, &fld_val);
            }
            switch(fld_val) {
                case 0:
                    *act_param = DRV_VM_ACTION_TAG_AS_RECEIVED;
                    break;
                case 1:
                    *act_param = DRV_VM_ACTION_TAG_AS_NORMALIZED;
                    break;
                case 2:
                    *act_param = DRV_VM_ACTION_TAG_AS_COPY;
                    break;
                case 3:
                    *act_param = DRV_VM_ACTION_TAG_AS_REMOVE;
                    break;
                case 4:
                    *act_param = DRV_VM_ACTION_TAG_AS_REPLACE;
                    break;
            }
            break;
        case DRV_VM_ACT_EVM_NEW_SVID:
            DRV_VM_FIELD_GET(unit, DRV_VM_RAM_EVM_ACT, 
                DRV_VM_FIELD_EVM_NEW_SVID, entry, &fld_val);
            *act_param = fld_val;
            break;
        case DRV_VM_ACT_EVM_NEW_CVID:
            DRV_VM_FIELD_GET(unit, DRV_VM_RAM_EVM_ACT, 
                DRV_VM_FIELD_EVM_NEW_CVID, entry, &fld_val);
            *act_param = fld_val;
            break;
        case DRV_VM_ACT_EVM_OUTER_PCP_REPLACE:
            if (SOC_IS_TB_AX(unit)) {
                rv = SOC_E_PARAM;
            } else {
                DRV_VM_FIELD_GET(unit, DRV_VM_RAM_EVM_ACT, 
                    DRV_VM_FIELD_EVM_OUTER_PCP_REPLACE, entry, &fld_val);
                *act_param = fld_val;
            }
            break;
        case DRV_VM_ACT_EVM_NEW_OUTER_PCP:
            if (SOC_IS_TB_AX(unit)) {
                rv = SOC_E_PARAM;
            } else {
                DRV_VM_FIELD_GET(unit, DRV_VM_RAM_EVM_ACT, 
                    DRV_VM_FIELD_EVM_NEW_OUTER_PCP, entry, &fld_val);
                *act_param = fld_val;
            }
            break;
        case DRV_VM_ACT_EVM_INNER_PCP_REPLACE:
            if (SOC_IS_TB_AX(unit)) {
                rv = SOC_E_PARAM;
            } else {
                DRV_VM_FIELD_GET(unit, DRV_VM_RAM_EVM_ACT, 
                    DRV_VM_FIELD_EVM_INNER_PCP_REPLACE, entry, &fld_val);
                *act_param = fld_val;
            }
            break;
        case DRV_VM_ACT_EVM_NEW_INNER_PCP:
            if (SOC_IS_TB_AX(unit)) {
                rv = SOC_E_PARAM;
            } else {
                DRV_VM_FIELD_GET(unit, DRV_VM_RAM_EVM_ACT, 
                    DRV_VM_FIELD_EVM_NEW_INNER_PCP, entry, &fld_val);
                *act_param = fld_val;
            }
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 * Function: drv_tbx_vm_action_set
 *
 * Purpose:
 *     Set the IVM/EVM action type and parameters value to 
 *     the raw data of ACTION ram.
 *
 * Parameters:
 *     unit - BCM device number
 *     action - driver action type
 *     entry(OUT) -IVm/EVM entry
 *     act_param - action paramter (if need)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - unknown action type
 *
 * Note:
 */
int
drv_tbx_vm_action_set(int unit, uint32 action, 
            drv_vm_entry_t* entry, uint32 act_param)
{
    int rv = SOC_E_NONE;
    uint32  fld_val;

    assert(entry);
    
    switch (action) {
        case DRV_VM_ACT_IVM_SPCP_MARKING_POLICY:
        case DRV_VM_ACT_IVM_CPCP_MARKING_POLICY:
            switch(act_param) {
                case DRV_VM_ACTION_PCP_MARK_MAPPED:
                    fld_val = 0x0;
                    break;
                case DRV_VM_ACTION_PCP_MARK_INGRESS_INNER_PCP:
                    fld_val = 0x1;
                    break;
                case DRV_VM_ACTION_PCP_MARK_INGRESS_OUTER_PCP:
                    fld_val = 0x2;
                    break;
                case DRV_VM_ACTION_PCP_MARK_PORT_DEFAULT:
                    fld_val = 0x3;
                    break;
                default:
                    return SOC_E_PARAM;
                    break;
            }
            if (action == DRV_VM_ACT_IVM_SPCP_MARKING_POLICY) {
                DRV_VM_FIELD_SET(unit, DRV_VM_RAM_IVM_ACT, 
                    DRV_VM_FIELD_IVM_SPCP_MARKING_POLICY, entry, &fld_val);
            } else {
                DRV_VM_FIELD_SET(unit, DRV_VM_RAM_IVM_ACT, 
                    DRV_VM_FIELD_IVM_CPCP_MARKING_POLICY, entry, &fld_val);
            }
            break;
        case DRV_VM_ACT_IVM_VLAN_ID:
            fld_val = act_param;
            DRV_VM_FIELD_SET(unit, DRV_VM_RAM_IVM_ACT, 
                DRV_VM_FIELD_IVM_VLAN_ID, entry, &fld_val);
            break;
        case DRV_VM_ACT_IVM_FLOW_ID:
            fld_val = act_param;
            DRV_VM_FIELD_SET(unit, DRV_VM_RAM_IVM_ACT, 
                DRV_VM_FIELD_IVM_FLOW_ID, entry, &fld_val);
            break;
        case DRV_VM_ACT_IVM_VPORT_ID:
            fld_val = act_param;
            DRV_VM_FIELD_SET(unit, DRV_VM_RAM_IVM_ACT, 
                DRV_VM_FIELD_IVM_VPORT_ID, entry, &fld_val);
            break;
        case DRV_VM_ACT_IVM_VPORT_SPCP:
            fld_val = act_param;
            DRV_VM_FIELD_SET(unit, DRV_VM_RAM_IVM_ACT, 
                DRV_VM_FIELD_IVM_VPORT_SPCP, entry, &fld_val);
            break;
        case DRV_VM_ACT_IVM_VPORT_CPCP:
            fld_val = act_param;
            DRV_VM_FIELD_SET(unit, DRV_VM_RAM_IVM_ACT, 
                DRV_VM_FIELD_IVM_VPORT_CPCP, entry, &fld_val);
            break;
        case DRV_VM_ACT_IVM_VPORT_DP:
            fld_val = act_param;
            DRV_VM_FIELD_SET(unit, DRV_VM_RAM_IVM_ACT, 
                DRV_VM_FIELD_IVM_VPORT_DP, entry, &fld_val);
            break;
        case DRV_VM_ACT_IVM_VPORT_TC:
            fld_val = act_param;
            DRV_VM_FIELD_SET(unit, DRV_VM_RAM_IVM_ACT, 
                DRV_VM_FIELD_IVM_VPORT_TC, entry, &fld_val);
            break;

        case DRV_VM_ACT_EVM_STAG:
        case DRV_VM_ACT_EVM_CTAG:
            switch(act_param) {
                case DRV_VM_ACTION_TAG_AS_RECEIVED:
                    fld_val = 0x0;
                    break;
                case DRV_VM_ACTION_TAG_AS_NORMALIZED:
                    fld_val = 0x1;
                    break;
                case DRV_VM_ACTION_TAG_AS_COPY:
                    fld_val = 0x2;
                    break;
                case DRV_VM_ACTION_TAG_AS_REMOVE:
                    fld_val = 0x3;
                    break;
                case DRV_VM_ACTION_TAG_AS_REPLACE:
                    fld_val = 0x4;
                    break;
                default:
                    return SOC_E_PARAM;
                    break;
            }
            if (action == DRV_VM_ACT_EVM_STAG) {
                DRV_VM_FIELD_SET(unit, DRV_VM_RAM_EVM_ACT, 
                    DRV_VM_FIELD_EVM_STAG_ACT, entry, &fld_val);
            } else {
                DRV_VM_FIELD_SET(unit, DRV_VM_RAM_EVM_ACT, 
                    DRV_VM_FIELD_EVM_CTAG_ACT, entry, &fld_val);
            }
            break;
        case DRV_VM_ACT_EVM_NEW_SVID:
            fld_val = act_param;
            DRV_VM_FIELD_SET(unit, DRV_VM_RAM_EVM_ACT, 
                DRV_VM_FIELD_EVM_NEW_SVID, entry, &fld_val);
            break;
        case DRV_VM_ACT_EVM_NEW_CVID:
            fld_val = act_param;
            DRV_VM_FIELD_SET(unit, DRV_VM_RAM_EVM_ACT, 
                DRV_VM_FIELD_EVM_NEW_CVID, entry, &fld_val);
            break;
        case DRV_VM_ACT_EVM_OUTER_PCP_REPLACE:
            if (SOC_IS_TB_AX(unit)) {
                rv = SOC_E_PARAM;
            } else {
                fld_val = act_param;
                DRV_VM_FIELD_SET(unit, DRV_VM_RAM_EVM_ACT, 
                    DRV_VM_FIELD_EVM_OUTER_PCP_REPLACE, entry, &fld_val);
            }
            break;
        case DRV_VM_ACT_EVM_NEW_OUTER_PCP:
            if (SOC_IS_TB_AX(unit)) {
                rv = SOC_E_PARAM;
            } else {
                fld_val = act_param;
                DRV_VM_FIELD_SET(unit, DRV_VM_RAM_EVM_ACT, 
                    DRV_VM_FIELD_EVM_NEW_OUTER_PCP, entry, &fld_val);
            }
            break;
        case DRV_VM_ACT_EVM_INNER_PCP_REPLACE:
            if (SOC_IS_TB_AX(unit)) {
                rv = SOC_E_PARAM;
            } else {
                fld_val = act_param;
                DRV_VM_FIELD_SET(unit, DRV_VM_RAM_EVM_ACT, 
                    DRV_VM_FIELD_EVM_INNER_PCP_REPLACE, entry, &fld_val);
            }
            break;
        case DRV_VM_ACT_EVM_NEW_INNER_PCP:
            if (SOC_IS_TB_AX(unit)) {
                rv = SOC_E_PARAM;
            } else {
                fld_val = act_param;
                DRV_VM_FIELD_SET(unit, DRV_VM_RAM_EVM_ACT, 
                    DRV_VM_FIELD_EVM_NEW_INNER_PCP, entry, &fld_val);
            }
            break;
        default:
            rv = SOC_E_PARAM;
    }

    return rv;
}

/*
 * Function: drv_tbx_vm_entry_read
 *
 * Purpose:
 *     Read the KEY/ACTION raw data from chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     index - IVM/EVM entry index
 *     ram_type -KEY and ACTION
 *     entry(OUT) - chip entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - unknown ram type
 *
 */
int
drv_tbx_vm_entry_read(int unit, uint32 index, uint32 ram_type, 
            drv_vm_entry_t *entry)
{
    int rv = SOC_E_NONE;

    switch (ram_type) {
        case DRV_VM_RAM_IVM_ALL:
            if ((rv = _drv_tbx_vm_read(unit, DRV_VM_RAM_IVM_KEY_DATA, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_vm_entry_read : failed to read IVM Key with index = 0x%x, rv = %d. \n"), 
                           index, rv));
                return rv;
            }
            if ( (rv = _drv_tbx_vm_read(unit, DRV_VM_RAM_IVM_ACT, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_vm_entry_read : failed to read IVM Action with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index;
            break;
        case DRV_VM_RAM_IVM_KEY_DATA:
            if ((rv = _drv_tbx_vm_read(unit, DRV_VM_RAM_IVM_KEY_DATA, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_vm_entry_read : failed to read IVM Key with index = 0x%x, rv = %d. \n"), 
                           index, rv));
                return rv;
            }
            entry->id = index;
            break;
        case DRV_VM_RAM_IVM_ACT:
            if ( (rv = _drv_tbx_vm_read(unit, DRV_VM_RAM_IVM_ACT, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_vm_entry_read : failed to read IVM Action with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index;
            break;

        case DRV_VM_RAM_EVM_ALL:
            if ((rv = _drv_tbx_vm_read(unit, DRV_VM_RAM_EVM_KEY_DATA, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_vm_entry_read : failed to read EVM Key with index = 0x%x, rv = %d. \n"), 
                           index, rv));
                return rv;
            }
            if ( (rv = _drv_tbx_vm_read(unit, DRV_VM_RAM_EVM_ACT, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_vm_entry_read : failed to read EVM Action with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index;
            break;
        case DRV_VM_RAM_EVM_KEY_DATA:
            if ((rv = _drv_tbx_vm_read(unit, DRV_VM_RAM_EVM_KEY_DATA, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_vm_entry_read : failed to read EVM Key with index = 0x%x, rv = %d. \n"), 
                           index, rv));
                return rv;
            }
            entry->id = index;
            break;
        case DRV_VM_RAM_EVM_ACT:
            if ( (rv = _drv_tbx_vm_read(unit, DRV_VM_RAM_EVM_ACT, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_vm_entry_read : failed to read EVM Action with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index;
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 * Function: drv_tbx_vm_entry_write
 *
 * Purpose:
 *     Write the KEY/ACTION raw data to chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     index - IVM/EVM entry index
 *     ram_type -KEY and ACTION
 *     entry - chip entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - unknown ram type
 *
 * Note:
 *     
 */
int
drv_tbx_vm_entry_write(int unit, uint32 index, uint32 ram_type, 
            drv_vm_entry_t *entry)
{
    int rv = SOC_E_NONE;

    switch (ram_type) {
        case DRV_VM_RAM_IVM_ALL:
            if ((rv = _drv_tbx_vm_write(unit, DRV_VM_RAM_IVM_KEY_DATA, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_vm_entry_write : failed to write IVM Key with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            if ((rv = _drv_tbx_vm_write(unit, DRV_VM_RAM_IVM_ACT, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_vm_entry_write:failed to write IVM Action with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index;
            break;
        case DRV_VM_RAM_IVM_KEY_DATA:
            if ((rv = _drv_tbx_vm_write(unit, DRV_VM_RAM_IVM_KEY_DATA, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_vm_entry_write : failed to write IVM Key with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index; 
            break;
        case DRV_VM_RAM_IVM_ACT:
            if ((rv = _drv_tbx_vm_write(unit, DRV_VM_RAM_IVM_ACT, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_vm_entry_write:failed to write IVM Action with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index; 
            break;

        case DRV_VM_RAM_EVM_ALL:
            if ((rv = _drv_tbx_vm_write(unit, DRV_VM_RAM_EVM_KEY_DATA, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_vm_entry_write : failed to write EVM Key with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            if ((rv = _drv_tbx_vm_write(unit, DRV_VM_RAM_EVM_ACT, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_vm_entry_write:failed to write EVM Action with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index;
            break;
        case DRV_VM_RAM_EVM_KEY_DATA:
            if ((rv = _drv_tbx_vm_write(unit, DRV_VM_RAM_EVM_KEY_DATA, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_vm_entry_write : failed to write EVM Key with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index; 
            break;
        case DRV_VM_RAM_EVM_ACT:
            if ((rv = _drv_tbx_vm_write(unit, DRV_VM_RAM_EVM_ACT, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_vm_entry_write:failed to write EVM Action with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index; 
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 * Function: drv_tbx_vm_range_set
 *
 * Purpose:
 *     Configure IVM Range CVID bounds.
 *
 * Parameters:
 *     unit - BCM device number
 *     id - ranger's id 
 *     min - minimum value 
 *     max - maximum value 
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 */
int 
drv_tbx_vm_range_set(int unit, uint32 id, uint32 min, uint32 max)
{
    int rv = SOC_E_NONE;
    uint32 reg_val, fld_val;

    /* Check supported value */
    if ((min >= VLAN_ID_INVALID) || (max >= VLAN_ID_INVALID)) {
        return SOC_E_PARAM;
    }

    /* set min and max range */
    rv = REG_READ_VID_RANGE_CHECKERr(unit, id, &reg_val);
    if (rv < 0) {
        return rv;
    }
    fld_val = min;
    soc_VID_RANGE_CHECKERr_field_set(unit, &reg_val, SMALL_VALUEf, &fld_val);

    fld_val = max;
    soc_VID_RANGE_CHECKERr_field_set(unit, &reg_val, LARGE_VALUEf, &fld_val);

    rv = REG_WRITE_VID_RANGE_CHECKERr(unit, id, &reg_val);

    return rv;
}

/*
 * Function: drv_tbx_vm_flow_alloc
 *
 * Purpose:
 *     Get a free flow id for IVM/EVM entry creation.
 *
 * Parameters:
 *     unit - BCM device number
 *     type - Get from head or tail 
 *     flow_id - available flow id
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 */
int 
drv_tbx_vm_flow_alloc(int unit, uint32 type, int *flow_id)
{
    if (type == VM_FLOW_ID_GET_CPU_DEFAULT) {
        return DRV_FP_ID_CONTROL(unit, _DRV_FP_ID_VM_FLOW_ID, 
            _DRV_FP_ID_CTRL_GET_CPU_DEFAULT, 0, 
            flow_id, NULL);        
    } else if (type == VM_FLOW_ID_GET_FROM_HEAD) {
        return DRV_FP_ID_CONTROL(unit, _DRV_FP_ID_VM_FLOW_ID, 
            _DRV_FP_ID_CTRL_ALLOC, _DRV_FP_ID_GET_FROM_HEAD, 
            flow_id, NULL);        
    } else {
        return DRV_FP_ID_CONTROL(unit, _DRV_FP_ID_VM_FLOW_ID, 
            _DRV_FP_ID_CTRL_ALLOC, _DRV_FP_ID_GET_FROM_TAIL, 
            flow_id, NULL);
    }
}

/*
 * Function: drv_tbx_vm_flow_free
 *
 * Purpose:
 *    Free the flow id.
 *
 * Parameters:
 *     unit - BCM device number
 *     flow_id - available flow id
 *
 * Returns:
 *     SOC_E_NONE
 *
 */
int 
drv_tbx_vm_flow_free(int unit, int flow_id)
{
    return DRV_FP_ID_CONTROL(unit, _DRV_FP_ID_VM_FLOW_ID, 
        _DRV_FP_ID_CTRL_FREE, 0, &flow_id, NULL);
}

/*
 * Function: drv_tbx_vm_ranger_inc
 *
 * Purpose:
 *     Increase applied count of the ranger_id.
 *
 * Parameters:
 *     unit - BCM device number
 *     ranger_id - Ranger id
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 */
int 
drv_tbx_vm_ranger_inc(int unit, int ranger_id)
{
    /* Increase applied count of the ranger_id. */
    return DRV_FP_ID_CONTROL(unit, _DRV_FP_ID_VM_RANGE_ID, 
        _DRV_FP_ID_CTRL_INC, 0, &ranger_id, NULL);
}

/*
 * Function: drv_tbx_vm_ranger_dec
 *
 * Purpose:
 *    Decrease applied count of the ranger_id.
 *
 * Parameters:
 *     unit - BCM device number
 *     flow_id - available flow id
 *
 * Returns:
 *     SOC_E_NONE
 *
 */
int 
drv_tbx_vm_ranger_dec(int unit, int ranger_id)
{
    return DRV_FP_ID_CONTROL(unit, _DRV_FP_ID_VM_RANGE_ID, 
        _DRV_FP_ID_CTRL_DEC, 0, &ranger_id, NULL);
}

/*
 * Function: drv_tbx_vm_ranger_count_get
 *
 * Purpose:
 *    Get the applied count of the ranger_id.
 *
 * Parameters:
 *     unit - BCM device number
 *     flow_id - available flow id
 *     count - number of entries use the ranger id
 *
 * Returns:
 *     SOC_E_NONE
 *
 */
int 
drv_tbx_vm_ranger_count_get(int unit, int ranger_id, uint32 *count)
{
    return DRV_FP_ID_CONTROL(unit, _DRV_FP_ID_VM_RANGE_ID, 
        _DRV_FP_ID_CTRL_DEC, 0, &ranger_id, count);
  
}


