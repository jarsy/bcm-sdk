/*
 * $Id: mem.c,v 1.69 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC Memory (Table) Utilities
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>

#include <soc/mem.h>
#include <soc/debug.h>

#include <soc/l2x.h>
#include <soc/cmic.h>
#include <soc/error.h>
#include <soc/register.h>
#include <soc/robo/mcm/driver.h>
#include <soc/spi.h>
#include <soc/arl.h>


/*
 * Empty (null) table entries
 */
uint32  _soc_robo_mem_entry_null_zeroes[SOC_ROBO_MAX_MEM_WORDS];


/*
 * Function:
 *  _soc_mem_cmp_XXX
 * Purpose:
 *  Compare two memory entries
 * Parameters:
 *  unit, entry A and entry B
 * Returns:
 *  Negative for A < B, zero for A == B, positive for A > B
 * Notes:
 *  Generic compare functions for table entries.
 *  Additional chip-specific compare functions are defined in
 *  the mcm files.
 */

int
_soc_mem_cmp_gm(int unit, void *ent_a, void *ent_b)
{
    return 0;
}



/*
 *  Function : _drv_arl_hash
 *
 *  Purpose :
 *      Get the hash result value.
 *
 *  Parameters :
 *      hash_value   :   48 bits hash value.
 *      length :            number of bytes of the value.
 *      hash_result     :   hash result.
 *
 *  Return :
 *      None
 *
 *  Note :
 *
 */
void 
_drv_arl_hash(uint8 *hash_value, uint8 loop, uint16 *hash_result)
{
    uint16  crc = 0xffff;
    uint16  poly = 0x1021;
    uint8  ch;
    uint16  i, j, v, xor_flag;
    for (i=0; i< ((loop+7)/8); i++) {
        ch = *(hash_value+i);
        v= 0x80;
        for (j=0; j<8; j++) {
            if ( (i*8+j) < loop) {
                if (crc & 0x8000) {
                    xor_flag = 1;
                } else {
                    xor_flag = 0;
                }
                crc = crc << 1;
                if (ch & v) {
                    crc = crc ^ poly;
                }
                if (xor_flag) {
                    crc = crc ^ poly;
                }
                v = v >> 1;
            }
        }
    }
    *hash_result = crc;

}

/* Function : _drv_arl_calc_hash_index
 *  - SW hash algorithm to help SW to retreive the ARL entry index based on
 *      a given sw arl entry(carry the MAC + VID hash key)
 *
 *  Note :
 *  1. TB devices doesn't need this function for the TB chip can report the 
 *      real table index through it dual hash argorithm.
 */
static uint32
_drv_arl_calc_hash_index(int unit, int bin_index, void *entry)
{
    uint8   hash_value[8], mac_addr_rw[6];
    uint16  hash_result, left_over = 0, final_hash;
    uint32  vid_rw;
    uint32  table_index = 0;
    uint64  rw_mac_field;
    int     index_count;
    uint8       loop = 0, bin_num = 0;
    uint32  ivl_mode = 0;
    
    if (SOC_IS_TBX(unit)){
        /* TB chips allowed SW to retrieve the dual hashed table index 
         *  through the given MAC+VID key.
         *
         * In current design for ROBO SDK, there is no request on getting 
         *  hashed index directly. This internal routine for ROBO is left 
         *  as "TBD" status.
         */
         assert(0);
    }
    
    index_count = SOC_MEM_SIZE(unit, INDEX(L2_ARLm));
    (DRV_SERVICES(unit)->mem_field_get)
        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
        (uint32 *)entry, &vid_rw);

    /* Read MAC address bit 12~47 */
    (DRV_SERVICES(unit)->mem_field_get)
        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
        (void *)entry, (void *)&rw_mac_field);
    SAL_MAC_ADDR_FROM_UINT64(mac_addr_rw, rw_mac_field);
    
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->vlan_prop_get)
            (unit, DRV_VLAN_PROP_VLAN_LEARNING_MODE, &ivl_mode));
            
    /* when ROBO device at SVL mode, the VID for hash is "0" */
    if (ivl_mode == FALSE){
        vid_rw = 0;
    }

    /* HASH value */
#if defined(BCM_DINO8_SUPPORT) || defined(BCM_DINO16_SUPPORT)
    if (SOC_IS_DINO(unit)) {
        hash_value[0] = mac_addr_rw[5];
        hash_value[1] = mac_addr_rw[4];
        hash_value[2] = mac_addr_rw[3];
        hash_value[3] = mac_addr_rw[2];
        hash_value[4] = mac_addr_rw[1];
        hash_value[5] = mac_addr_rw[0];
        hash_value[6] = vid_rw & 0xff;
        hash_value[7] = (vid_rw >> 8) & 0xf;
        hash_value[0] = _shr_bit_rev8(hash_value[0]);
        hash_value[1] = _shr_bit_rev8(hash_value[1]);
        hash_value[2] = _shr_bit_rev8(hash_value[2]);
        hash_value[3] = _shr_bit_rev8(hash_value[3]);
        hash_value[4] = _shr_bit_rev8(hash_value[4]);
        hash_value[5] = _shr_bit_rev8(hash_value[5]);
        hash_value[6] = _shr_bit_rev8(hash_value[6]);
        hash_value[7] = _shr_bit_rev8(hash_value[7]);
    } else 
#endif /* BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */
    {    
        hash_value[0] = (vid_rw >> 4) & 0xff;
        hash_value[1] = ((vid_rw & 0xf) << 4) + ((mac_addr_rw[0] & 0xf0) >> 4);
        hash_value[2] = ((mac_addr_rw[0] & 0xf) << 4) + 
            ((mac_addr_rw[1] & 0xf0) >> 4);
        hash_value[3] = ((mac_addr_rw[1] & 0xf) << 4) + 
            ((mac_addr_rw[2] & 0xf0) >> 4);
        hash_value[4] = ((mac_addr_rw[2] & 0xf) << 4) + 
            ((mac_addr_rw[3] & 0xf0) >> 4);
        hash_value[5] = ((mac_addr_rw[3] & 0xf) << 4) + 
            ((mac_addr_rw[4] & 0xf0) >> 4);
        hash_value[6] = ((mac_addr_rw[4] & 0xf) << 4) + 
            ((mac_addr_rw[5] & 0xf0) >> 4);
        hash_value[7] = ((mac_addr_rw[5] & 0xf) << 4);
    }

    if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit) || 
        SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        /* 1K 4bin */
        loop = 50;
        left_over = (((mac_addr_rw[4] & 0x3) << 8) + mac_addr_rw[5]);
        bin_num = 4;
    } else if (SOC_IS_HARRIER(unit)) {
        /* 4K 2bin */
        loop = 47;
        left_over = (((mac_addr_rw[4] & 0x1f) << 8) + mac_addr_rw[5]);
        bin_num = 2;
    } else if (SOC_IS_LOTUS(unit)) {
        /* 512, 4bin */
        loop = 51;
        left_over = (((mac_addr_rw[4] & 0x1) << 8) + mac_addr_rw[5]);
        bin_num = 4;
#if defined(BCM_DINO8_SUPPORT) || defined(BCM_DINO16_SUPPORT)
    } else if (SOC_IS_DINO(unit)) {
        /* 4K, 2bin */
        loop = 60;
        left_over = 0x0;
        bin_num = 2;
#endif /* BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */
    }
    
    _drv_arl_hash(hash_value, loop, &hash_result);
    final_hash = hash_result^left_over;
    table_index = ((final_hash * bin_num) + bin_index) % index_count;
    LOG_INFO(BSL_LS_SOC_ARL,
             (BSL_META_U(unit,
                         "_drv_arl_calc_hash_index : table_index = 0x%x\n"), table_index));
    return table_index;
}

int
_drv_arl_database_delete_by_fastage(int unit, uint32 src_port, 
    uint32 vlanid, uint32 flags)
{
    uint32  temp, port, vlan;
    int     rv = SOC_E_NONE;
    int     index_min, index_count;
    int32   idx,valid;
    soc_control_t   *soc = SOC_CONTROL(unit);

    index_min = SOC_MEM_BASE(unit, INDEX(L2_ARLm));
    index_count = SOC_MEM_SIZE(unit, INDEX(L2_ARLm));
    
    if (flags & DRV_MEM_OP_DELETE_ALL_ARL) {
        if (soc->arl_table != NULL) {
            ARL_SW_TABLE_LOCK(soc);
            sal_memset(&soc->arl_table[0], 0, 
                sizeof (l2_arl_sw_entry_t) * index_count);
            ARL_SW_TABLE_UNLOCK(soc);
        }
        return rv;
    }
    if (soc->arl_table != NULL) {
        ARL_SW_TABLE_LOCK(soc);
        for (idx = index_min; idx < index_count; idx++) {
            if(!ARL_ENTRY_NULL(&soc->arl_table[idx])) {
                if ((rv = DRV_MEM_FIELD_GET(unit, 
                        DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                        (uint32 *)&soc->arl_table[idx], 
                        (uint32 *) &valid)) < 0){
                    ARL_SW_TABLE_UNLOCK(soc);
                    return rv;
                }
                if (valid) {
                    /* Check PORT */
                    if (flags & DRV_MEM_OP_DELETE_BY_PORT){
                        if ((rv = DRV_MEM_FIELD_GET(unit, 
                                DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                                (uint32 *)&soc->arl_table[idx], &port)) < 0){
                            ARL_SW_TABLE_UNLOCK(soc);
                            return rv;
                        }
                        if (port != src_port) {
                            continue;
                        }
                    }
                    /* Check VLAN */
                    if (flags & DRV_MEM_OP_DELETE_BY_VLANID){
                        if ((rv = DRV_MEM_FIELD_GET(unit, 
                                DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                                (uint32 *)&soc->arl_table[idx], &vlan)) < 0){
                            ARL_SW_TABLE_UNLOCK(soc);
                            return rv;
                        }
                        if (vlan !=vlanid) {
                            continue;
                        }
                    }
                    /* Check static/dynamic entry */
                    if (!(flags & DRV_MEM_OP_DELETE_BY_STATIC)){
                        if ((rv = DRV_MEM_FIELD_GET(unit, 
                                DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                                (uint32 *)&soc->arl_table[idx], &temp)) < 0){
                            ARL_SW_TABLE_UNLOCK(soc);
                            return rv;
                        }
                        if (temp != 0) {
                            continue;
                        }
                    }
                    soc_arl_database_delete(unit, idx);
                }
            }
        }
        ARL_SW_TABLE_UNLOCK(soc);
    }
    return rv;
}

/* Function : _drv_arl_database_delete()
 *  - serve the delete request on SW ARL table and execute the registered 
 *      callback routine for such deletion.
 *
 * Note :
 *  1. The 2nd parameter, "index", for all robo chips before TB was designed 
 *      to carry the bin_id of the target entry to delete.
 *      - The early ROBO device before TB can't report the hashed index. Thus 
 *          our SW ARL design implemented a SW basis hash algorithm to 
 *          retrieve this index.
 *          (TB new feature has implemented to report this hash basis index)
 */
int
_drv_arl_database_delete(int unit, int index, void *entry)
{
    int rv;
    soc_control_t   *soc = SOC_CONTROL(unit);
    uint32  valid;
    uint32  table_index = 0;
    l2_arl_sw_entry_t output;
    uint32 cb = 0;
    
    if (SOC_IS_TBX(unit)){
        table_index = index;
    } else {
        table_index = _drv_arl_calc_hash_index(unit, index, entry);
    }
    LOG_INFO(BSL_LS_SOC_TESTS,
             (BSL_META_U(unit,
                         "%s, arl_index=%d\n"), FUNCTION_NAME(), table_index));

    if (soc->arl_table != NULL) {
        ARL_SW_TABLE_LOCK(soc);
        soc_arl_database_dump(unit, table_index, &output);
        if ((rv = DRV_MEM_FIELD_GET(unit, 
                DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                (uint32 *)&output, &valid)) < 0){
            ARL_SW_TABLE_UNLOCK(soc);
            return rv;
        }
        if (valid) {
            cb = 1;
            valid = 0;
            if ((rv = DRV_MEM_FIELD_SET(unit,
                    DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                    (uint32 *)&output, &valid)) < 0){
                ARL_SW_TABLE_UNLOCK(soc);
                return rv;
            }
            sal_memcpy(&(soc->arl_table[table_index]), &output, 
                sizeof(l2_arl_sw_entry_t));
        }
        ARL_SW_TABLE_UNLOCK(soc);
        if (cb) {
            soc_robo_arl_callback(unit, 
                (l2_arl_sw_entry_t *)&output, NULL);
        }
    }
    return SOC_E_NONE;
}

/* Function : _drv_arl_database_insert()
 *  - serve the insert request on SW ARL table and execute the registered 
 *      callback routine for such insertion.
 *
 * Note :
 *  1. The 2nd parameter, "index", for all robo chips before TB was designed 
 *      to carry the bin_id of the target entry to delete.
 *      - The early ROBO device before TB can't report the hashed index. Thus 
 *          our SW ARL design implemented a SW basis hash algorithm to 
 *          retrieve this index.
 *          (TB new feature has implemented to report this hash basis index)
 *  2. This routine currently is called in DRV_MEM_INSERT() routine only.
 */
int
_drv_arl_database_insert(int unit, int index, void *entry)
{
    soc_control_t   *soc = SOC_CONTROL(unit);
    uint32          table_index = 0, valid = 0;
    int             rv = SOC_E_NONE, is_modify = 0, is_pending = 0;
    l2_arl_sw_entry_t   old_entry, output;

    if (SOC_IS_TBX(unit)){
        table_index = index;
    } else {
        table_index = _drv_arl_calc_hash_index(unit, index, entry);
    }
    LOG_INFO(BSL_LS_SOC_TESTS,
             (BSL_META_U(unit,
                         "%s, arl_index=%d\n"), FUNCTION_NAME(), table_index));

    if (soc->arl_table != NULL) {
        
        sal_memcpy(&output, entry, 
                sizeof(l2_arl_sw_entry_t));
        
        /* SDK-32894 : L2 callback issue while SW station movement occurred.
         *
         * Fix process :
         *  - issue an ARL_DELETE callback before ARL_INSERT callback if the 
         *      indicated L2 entry is valid in SW ARL table.
         *      (ARL modify callback process = ARL_DELETE + ARL_INSERT)
         */
        
        ARL_SW_TABLE_LOCK(soc);
        /* to retreive current SW ARL entry first */
        soc_arl_database_dump(unit, table_index, &old_entry);

        /* sw arl insert */
        sal_memcpy(&(soc->arl_table[table_index]), &output, 
                sizeof(l2_arl_sw_entry_t));
        ARL_SW_TABLE_UNLOCK(soc);
        
        rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_VALID, (uint32 *)&old_entry, &valid);
        if (rv){
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s, unexpect error(%d) while retrieving valid status!"),
                      FUNCTION_NAME(), rv));
            return rv;
        }

        if (SOC_IS_TBX(unit)){
            /* The valid field valid at PENDING(0x1) and VALID(0x3) will be 
             *  treated as valid status here.
             */
            is_modify = ((valid == 0x1) || (valid == 0x3)) ? 1 : 0;
            is_pending = (valid == 0x1) ? TRUE : FALSE;
        } else {
            is_modify = (valid) ? 1 : 0;
        }

        /* issue the delete callback before insert callback if original entry 
         * is valid.
         */
        if (is_modify) {
            soc_robo_arl_callback(unit, 
                (l2_arl_sw_entry_t *)&old_entry, NULL);
        } else {
            /* To fix the SDK-37297 about the ARL sync issue while 
             *  fast-aging before the SW/HW ARL is syncronized.
             */
            soc_arl_database_add(unit, table_index, is_pending);
        }
        
        soc_robo_arl_callback(unit, NULL,
              (l2_arl_sw_entry_t *)&output);
    }
    return SOC_E_NONE;
}


/*
 * Function:
 *      soc_mem_debug_set
 * Purpose:
 *      Enable or disable MMU debug mode.
 * Returns:
 *      Previous enable state, or SOC_E_XXX on error.
 */

int
soc_robo_mem_debug_set(int unit, int enable)
{
    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_mem_debug_get
 * Purpose:
 *      Return current MMU debug mode status
 */

int
soc_robo_mem_debug_get(int unit, int *enable)
{
    return SOC_E_NONE;
}


