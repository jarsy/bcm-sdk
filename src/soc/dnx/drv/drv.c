/** \file drv.c
 * Slim SoC module to allow bcm actions.
 * 
 * This file contains structure and routine declarations for the
 * Switch-on-a-Chip Driver.
 *
 */

/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SOC_INIT

/*
 * legacy, should be removed once done porting 
 * { 
 */
#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT
/*
 * }
 */

/*
 * INCLUDE FILES:
 * {
 */

#include <shared/shrextend/shrextend_debug.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/dnx/drv.h>
#include <soc/mcm/driver.h>
#include <shared/swstate/sw_state_res_tag_bitmap.h>
#include "include/soc/dnx/cmodel/cmodel_reg_access.h"

/*
 * }
 */

shr_error_e
soc_dnx_chip_driver_find(
    int unit,
    uint16 pci_dev_id,
    uint8 pci_rev_id,
    soc_driver_t ** found_driver)
{
    uint16 driver_dev_id;
    uint8 driver_rev_id;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * get handler to locate the device driver, 
     * the result will usually be the same as the input, 
     * however it will be modified in cases of running SKUs 
     */
    SHR_IF_ERR_EXIT(soc_cm_get_id_driver(pci_dev_id, pci_rev_id, &driver_dev_id, &driver_rev_id));

    switch (driver_dev_id)
    {
        case BCM88690_DEVICE_ID:
        {
            *found_driver = &soc_driver_bcm88690_a0;
            break;
        }
        default:
        {
            SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_NOT_FOUND, "a suitable driver was not found for specified "
                                     "device_id %u and revision_id %u\n%s", driver_dev_id, driver_rev_id, EMPTY);
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
soc_dnx_chip_type_set(
    int unit,
    uint16 dev_id)
{
    soc_info_t *si;

    SHR_FUNC_INIT_VARS(unit);

    si = &SOC_INFO(unit);

    /*
     * Used to implement the SOC_IS_*(unit) macros
     */
    switch (dev_id)
    {
        case JERICHO_2_DEVICE_ID:
        {
            si->chip_type = SOC_INFO_CHIP_TYPE_JERICHO_2;
            SOC_CHIP_STRING(unit) = "jericho_2";
            break;
        }
        default:
        {

            si->chip_type = 0;
            SOC_CHIP_STRING(unit) = "???";
            LOG_VERBOSE_EX(BSL_LOG_MODULE, "soc_dnx_chip_type_set: driver device %04x unexpected\n%s%s%s",
                           dev_id, EMPTY, EMPTY, EMPTY);
            SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_NOT_FOUND,
                                     "soc_dnx_chip_type_set: cannot find a match for driver device\n%s%s%s", EMPTY,
                                     EMPTY, EMPTY);
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
soc_dnx_restore(
    int unit)
{
    int rv;
    uint16 device_id;
    uint8 revision_id;
    soc_control_t *soc;

    SHR_FUNC_INIT_VARS(unit);
    soc = SOC_CONTROL(unit);

    sal_memset(soc, 0, sizeof(soc_control_t));

    /*
     * the MCM generates a driver from each supported register file 
     * this driver is linked here to the running device. SKUs will 
     * recieve the super set driver. 
     */
    rv = soc_cm_get_id(unit, &device_id, &revision_id);
    SHR_IF_ERR_EXIT(rv);
    rv = soc_dnx_chip_driver_find(unit, device_id, revision_id, &(soc->chip_driver));
    SHR_IF_ERR_EXIT_WITH_LOG(rv, "Couldn't find driver for unit %d (device 0x%04x, revision 0x%02x)\n",
                             unit, device_id, revision_id);

    rv = soc_dnx_chip_type_set(unit, device_id);
    SHR_IF_ERR_EXIT(rv);

    soc->soc_flags |= SOC_F_ATTACHED;

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
soc_dnx_attach(
    int unit)
{
    soc_control_t *soc;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Allocate soc_control.
     */
    soc = SOC_CONTROL(unit);
    soc = sal_alloc(sizeof(soc_control_t), "soc_control");
    if (soc == NULL)
    {
        SHR_IF_ERR_EXIT(_SHR_E_MEMORY);
    }
    SOC_CONTROL(unit) = soc;
    SHR_IF_ERR_EXIT(soc_dnx_restore(unit));

exit:
    if (SHR_FUNC_ERR())
    {
        LOG_ERROR_EX(BSL_LOG_MODULE, "soc_dnx_attach: unit %d failed\n%s%s%s", unit, EMPTY, EMPTY, EMPTY);
        soc_dnx_detach(unit);
    }

    SHR_FUNC_EXIT;
}

shr_error_e
soc_dnx_detach(
    int unit)
{
    soc_control_t *soc;

    SHR_FUNC_INIT_VARS(unit);

    if (!SOC_UNIT_NUM_VALID(unit))
    {
        SHR_IF_ERR_EXIT(_SHR_E_UNIT);
    }

    soc = SOC_CONTROL(unit);
    if (soc == NULL)
    {
        SHR_EXIT();
    }

    sal_free(soc);
    SOC_CONTROL(unit) = NULL;

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
soc_dnx_control_init(
    int unit)
{
    soc_dnx_control_t *dnx;
    SHR_FUNC_INIT_VARS(unit);

    dnx = SOC_DNX_CONTROL(unit);
    if (dnx != NULL)
    {
        SHR_IF_ERR_EXIT(SOC_E_INIT);
    }

    dnx = (soc_dnx_control_t *) sal_alloc(sizeof(soc_dnx_control_t), "soc_dnx_control");
    if (dnx == NULL)
    {
        SHR_IF_ERR_EXIT(SOC_E_MEMORY);
    }
    sal_memset(dnx, 0, sizeof(soc_dnx_control_t));
    SOC_CONTROL(unit)->drv = dnx;

    dnx->cfg = (soc_dnx_config_t *) sal_alloc(sizeof(soc_dnx_config_t), "soc_dnx_cofig");
    if (dnx->cfg == NULL)
    {
        SHR_IF_ERR_EXIT(SOC_E_MEMORY);
    }
    sal_memset(dnx->cfg, 0, sizeof(soc_dnx_config_t));

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
soc_dnx_control_deinit(
    int unit)
{
    soc_dnx_config_t *dnx_cfg;
    SHR_FUNC_INIT_VARS(unit);

    if (SOC_DNX_CONTROL(unit) != NULL)
    {
        dnx_cfg = SOC_DNX_CONFIG(unit);
        if (dnx_cfg != NULL)
        {
            sal_free(SOC_DNX_CONFIG(unit));
            SOC_DNX_CONFIG(unit) = NULL;
        }
        sal_free(SOC_DNX_CONTROL(unit));
        SOC_CONTROL(unit)->drv = NULL;
    }

    SHR_FUNC_EXIT;
}

shr_error_e
soc_dnx_mutexes_init(
    int unit)
{
    int cmc;
    soc_control_t *soc;
    SHR_FUNC_INIT_VARS(unit);

    soc = SOC_CONTROL(unit);
    SOC_PCI_CMCS_NUM(unit) = soc_property_uc_get(unit, 0, spn_PCI_CMCS_NUM, 1);

    /*
     * Create mutexes.
     */
    if ((soc->schan_wb_mutex = sal_mutex_create("SchanWB")) == NULL)
    {
        SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_MEMORY, "Failed to allocate SchanWB\n%s%s%s", EMPTY, EMPTY, EMPTY);
    }

    if ((soc->socControlMutex = sal_mutex_create("SOC_CONTROL")) == NULL)
    {
        SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_MEMORY, "Failed to allocate soc_control lock\n%s%s%s", EMPTY, EMPTY, EMPTY);
    }

    if ((soc->counterMutex = sal_mutex_create("Counter")) == NULL)
    {
        SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_MEMORY, "Failed to allocate counter Lock\n%s%s%s", EMPTY, EMPTY, EMPTY);
    }

    if ((soc->miimMutex = sal_mutex_create("MIIM")) == NULL)
    {
        SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_MEMORY, "Failed to allocate MIIM lock\n%s%s%s", EMPTY, EMPTY, EMPTY);
    }

    if ((soc->miimIntr = sal_sem_create("MIIM interrupt", sal_sem_BINARY, 0)) == NULL)
    {
        SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_MEMORY, "Failed to allocate MIIM interrup Sem\n%s%s%s", EMPTY, EMPTY, EMPTY);
    }

    if ((soc->schanMutex = sal_mutex_create("SCHAN")) == NULL)
    {
        SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_MEMORY, "Failed to allocate Schan Lock\n%s%s%s", EMPTY, EMPTY, EMPTY);
    }

    for (cmc = 0; cmc < SOC_PCI_CMCS_NUM(unit) + 1; cmc++)
    {
        if ((soc->schanIntr[cmc] = sal_sem_create("SCHAN interrupt", sal_sem_BINARY, 0)) == NULL)
        {
            SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_MEMORY, "Failed to allocate Schan interrupt Sem\n%s%s%s", EMPTY, EMPTY,
                                     EMPTY);
        }
    }

#ifdef BCM_CMICM_SUPPORT
    soc->fschanMutex = NULL;
    if ((soc->fschanMutex = sal_mutex_create("FSCHAN")) == NULL)
    {
        SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_MEMORY, "Failed to allocate fSchan Lock\n%s%s%s", EMPTY, EMPTY, EMPTY);
    }
#endif /* BCM_CMICM_SUPPORT */

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
soc_dnx_mutexes_deinit(
    int unit)
{
    soc_control_t *soc;
    int cmc;
    SHR_FUNC_INIT_VARS(unit);

    if (!SOC_UNIT_VALID(unit))
    {
        SHR_IF_ERR_EXIT(SOC_E_UNIT);
    }

    soc = SOC_CONTROL(unit);
    if (soc == NULL)
    {
        SHR_EXIT();
    }

    /*
     * Destroy Sem/Mutex 
     */
#ifdef BCM_CMICM_SUPPORT
    if (soc->fschanMutex != NULL)
    {
        sal_mutex_destroy(soc->fschanMutex);
        soc->fschanMutex = NULL;
    }
#endif /* BCM_CMICM_SUPPORT */

    for (cmc = 0; cmc < SOC_PCI_CMCS_NUM(unit) + 1; cmc++)
    {
        if (soc->schanIntr[cmc])
        {
            sal_sem_destroy(soc->schanIntr[cmc]);
            soc->schanIntr[cmc] = NULL;
        }
    }
    if (soc->schanMutex != NULL)
    {
        sal_mutex_destroy(soc->schanMutex);
        soc->schanMutex = NULL;
    }
    if (soc->miimIntr != NULL)
    {
        sal_sem_destroy(soc->miimIntr);
        soc->miimIntr = NULL;
    }

    if (soc->miimMutex != NULL)
    {
        sal_mutex_destroy(soc->miimMutex);
        soc->miimMutex = NULL;
    }

    if (soc->counterMutex != NULL)
    {
        sal_mutex_destroy(soc->counterMutex);
        soc->counterMutex = NULL;
    }

    if (soc->socControlMutex != NULL)
    {
        sal_mutex_destroy(soc->socControlMutex);
        soc->socControlMutex = NULL;
    }

    if (soc->schan_wb_mutex != NULL)
    {
        sal_mutex_destroy(soc->schan_wb_mutex);
        soc->schan_wb_mutex = NULL;
    }

exit:
    SHR_FUNC_EXIT;
}


shr_error_e
soc_dnx_sw_state_utils_deinit(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);
    SHR_FUNC_EXIT;
}


shr_error_e
soc_dnx_sw_state_utils_init(
    int unit)
{
    int rv = SOC_E_NONE;
    soc_control_t *soc;
    uint32 sw_state_size;
    uint32 stable_size;
#if (0)
#if defined(BCM_WARM_BOOT_SUPPORT) && !defined(__KERNEL__) && defined (LINUX)
    int stable_location;
    uint32 stable_flags;
#endif
#endif

    stable_size = soc_property_get(unit, spn_STABLE_SIZE, SHR_SW_STATE_MAX_DATA_SIZE_IN_BYTES);

    soc = SOC_CONTROL(unit);
    soc->soc_flags &= ~SOC_F_INITED;

    sw_state_size = soc_property_get(unit, spn_SW_STATE_MAX_SIZE, 0);

    if (sw_state_size == 0)
    {
        LOG_WARN(BSL_LS_SOC_INIT,
                 (BSL_META_U(unit,
                             "sw_state_max_size SOC property is not defined, deriving size from stable_size.\n")));
        sw_state_size = stable_size - SHR_SW_STATE_SCACHE_RESERVE_SIZE;
    }

#if (0)
#if !defined(__KERNEL__) && defined (LINUX)
#ifdef BCM_WARM_BOOT_SUPPORT
    /*
     * create new file if not SOC_WARM_BOOT 
     */
    SOCDNX_IF_ERR_RETURN(soc_stable_get(unit, &stable_location, &stable_flags));
    if (stable_location == 4)
    {
        /*
         * init HA only if stable location is shared memory (4) 
         */
        SOCDNX_IF_ERR_RETURN(ha_init(unit, 1 /* HA enabled */ , "HA", stable_size, SOC_WARM_BOOT(unit) ? 0 : 1));
    }
#endif
#endif
#endif

    rv = shr_sw_state_init(unit, 0x0,
                           SOC_WARM_BOOT(unit) ? socSwStateDataBlockRestoreAndOveride : socSwStateDataBlockRegularInit,
                           sw_state_size);
    SOCDNX_IF_ERR_RETURN(rv);
    
    if (!SOC_WARM_BOOT(unit))
    {
        SOCDNX_IF_ERR_RETURN(utilex_occ_bm_init(unit, MAX_NOF_DSS_FOR_DNX));
        SOCDNX_IF_ERR_RETURN(utilex_hash_table_init(unit, MAX_NOF_HASHS_FOR_DNX));
        SOCDNX_IF_ERR_RETURN(utilex_sorted_list_init(unit, MAX_NOF_LISTS_FOR_DNX));
        SOCDNX_IF_ERR_RETURN(utilex_multi_set_init(unit, MAX_NOF_MULTIS_FOR_DNX));
        SOCDNX_IF_ERR_RETURN(sw_state_res_tag_bitmap_init(unit, MAX_NOF_RES_TAG_BITMAPS_FOR_DNX));
    }

    soc->soc_flags |= SOC_F_INITED;
    return rv;
}

int
soc_dnx_info_config(
    int unit)
{
    soc_info_t *si;
    soc_control_t *soc;
    int mem, blk;

    SOCDNX_INIT_FUNC_DEFS;

    soc = SOC_CONTROL(unit);

    si = &SOC_INFO(unit);
    si->driver_type = soc->chip_driver->type;
    si->driver_group = soc_chip_type_map[si->driver_type];

    SOCDNX_IF_ERR_EXIT(soc_dnx_info_config_blocks(unit));

    /*
     * Calculate the mem_block_any array for this configuration
     * The "any" block is just the first one enabled
     */
    for (mem = 0; mem < NUM_SOC_MEM; mem++)
    {
        si->mem_block_any[mem] = -1;
        if (SOC_MEM_IS_VALID(unit, mem))
        {
            SOC_MEM_BLOCK_ITER(unit, mem, blk)
            {
                si->mem_block_any[mem] = blk;
                break;
            }
        }
    }

    /*
     * use of gports is true for all SAND devices 
     */
    SOC_USE_GPORT_SET(unit, TRUE);

exit:
SOCDNX_FUNC_RETURN}

int
soc_dnx_info_config_blocks(
    int unit)
{
    soc_info_t *si;
    int blk, blktype, blknum;
    char instance_string[3];

    SOCDNX_INIT_FUNC_DEFS;

    si = &SOC_INFO(unit);

    sal_memset(si->has_block, 0, sizeof(soc_block_t) * COUNTOF(si->has_block));
    sal_memset(si->broadcast_blocks, 0, sizeof(si->broadcast_blocks));
    sal_memset(si->broadcast_blocks_size, 0, sizeof(si->broadcast_blocks_size));

    for (blk = 0; blk < SOC_MAX_NUM_BLKS; blk++)
    {
        si->block_port[blk] = REG_PORT_ANY;
        si->block_valid[blk] = 0;
    }

    si->cmic_block = -1;
    si->iproc_block = -1;

    /*
     * setting broadcast blocks as "regular blocks" by default 
     */
    for (blk = 0; SOC_BLOCK_INFO(unit, blk).type >= 0; blk++)
    {
        blktype = SOC_BLOCK_INFO(unit, blk).type;
        switch (blktype)
        {
            case SOC_BLK_CGM:
                si->brdc_cgm_block = blk;
                break;
            case SOC_BLK_EGQ:
                si->brdc_egq_block = blk;
                break;
            case SOC_BLK_EPNI:
                si->brdc_epni_block = blk;
                break;
            case SOC_BLK_IHB:
                si->brdc_ihb_block = blk;
                break;
            case SOC_BLK_IHP:
                si->brdc_ihp_block = blk;
                break;
            case SOC_BLK_IPS:
                si->brdc_ips_block = blk;
                break;
            case SOC_BLK_IQM:
                si->brdc_iqm_block = blk;
                break;
            case SOC_BLK_SCH:
                si->brdc_sch_block = blk;
                break;
        }
    }

    for (blk = 0; SOC_BLOCK_INFO(unit, blk).type >= 0; blk++)
    {
        blktype = SOC_BLOCK_INFO(unit, blk).type;
        blknum = SOC_BLOCK_INFO(unit, blk).number;
        if (blknum < 0)
        {
            SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                 (_BSL_SOCDNX_MSG("soc_dpp_info_config_blocks: illegal block instance number %d"),
                                  blknum));
        }

        if (blk >= SOC_MAX_NUM_BLKS)
        {
            SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                 (_BSL_SOCDNX_MSG("soc_dpp_info_config_blocks: too much blocks for device")));
        }

        si->has_block[blk] = blktype;
        sal_snprintf(instance_string, sizeof(instance_string), "%d", blknum);

        si->block_valid[blk] = 1;

        switch (blktype)
        {
            case SOC_BLK_IPS:
                if (blknum >= SOC_MAX_NUM_IPS_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many IPS blocks")));
                }
                si->ips_blocks[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_PQP:
                if (blknum >= SOC_MAX_NUM_PQP_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many PQP blocks")));
                }
                si->pqp_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_CRPS:
                si->crps_block = blk;
                break;
            case SOC_BLK_EPRE:
                if (blknum >= SOC_MAX_NUM_EPRE_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many EPRE blocks")));
                }
                si->epre_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_FDTL:
                si->fdtl_block = blk;
                break;
            case SOC_BLK_MRPS:
                if (blknum >= SOC_MAX_NUM_MRPS_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many MRPS blocks")));
                }
                si->mrps_blocks[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_IRE:
                si->ire_block = blk;
                break;
            case SOC_BLK_IPT:
                si->ipt_block = blk;
                break;
		    case SOC_BLK_OCB:
                if (blknum >= SOC_MAX_NUM_OCB_BLKS) {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("soc_dpp_info_config_blocks: soc_dpp_info_config: too many OCB blocks")));
                }
                si->ocb_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_FMAC:
                if (blknum >= SOC_MAX_NUM_FMAC_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many FMAC blocks")));
                }
                si->fmac_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_FDA:
                si->fda_block = blk;
                break;
            case SOC_BLK_RTP:
                si->rtp_block = blk;
                break;
            case SOC_BLK_FCR:
                si->fcr_block = blk;
                break;
            case SOC_BLK_FCT:
                si->fct_block = blk;
                break;
            case SOC_BLK_FDT:
                si->fdt_block = blk;
                break;
            case SOC_BLK_FDR:
                si->fdr_block = blk;
                break;
            case SOC_BLK_EDB:
                si->edb_block = blk;
                break;
            case SOC_BLK_EPNI:
                if (blknum >= SOC_MAX_NUM_EPNI_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many EPNI blocks")));
                }
                si->epni_blocks[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_SCH:
                if (blknum >= SOC_MAX_NUM_SCH_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many SCH blocks")));
                }
                si->sch_blocks[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_ECI:
                si->eci_block = blk;
                break;
            case SOC_BLK_CFC:
                if (blknum >= SOC_MAX_NUM_CFC_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many CFC blocks")));
                }
                si->cfc_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
        case SOC_BLK_KAPS:
            if (blknum >= SOC_MAX_NUM_KAPS_MI_BLKS)
            {
                SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                     (_BSL_SOCDNX_MSG
                                      ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many KAPS blocks")));
            }
            si->kaps_mi_block[blknum] = blk;
            si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
            break;
            case SOC_BLK_OAMP:
                si->oamp_block = blk;
                break;
            case SOC_BLK_OLP:
                si->olp_block = blk;
                break;
            case SOC_BLK_SIF:
                if (blknum >= SOC_MAX_NUM_SIF_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many SIF blocks")));
                }
                si->sif_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_MCP:
                if (blknum >= SOC_MAX_NUM_MCP_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many MCP blocks")));
                }
                si->mcp_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_ITPP:
                if (blknum >= SOC_MAX_NUM_ITPP_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many ITPP blocks")));
                }
                si->itpp_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_PDM:
                if (blknum >= SOC_MAX_NUM_PDM_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many PDM blocks")));
                }
                si->pdm_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_BDM:
                if (blknum >= SOC_MAX_NUM_BDM_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many BDM blocks")));
                }
                si->bdm_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_CDU:
                if (blknum >= SOC_MAX_NUM_CDU_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many CDU blocks")));
                }
                si->cdu_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;

            case SOC_BLK_CDUM:
                if (blknum >= SOC_MAX_NUM_CDUM_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many CDUM blocks")));
                }
                si->cdum_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;

            case SOC_BLK_DDHA:
                if (blknum >= SOC_MAX_NUM_DDHA_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many DDHA blocks")));
                }
                si->ddha_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;

            case SOC_BLK_DDHB:
                if (blknum >= SOC_MAX_NUM_DDHB_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many DDHB blocks")));
                }
                si->ddhb_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;

            case SOC_BLK_DHC:
                if (blknum >= SOC_MAX_NUM_DHC_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many DHC blocks")));
                }
                si->dhc_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;

            case SOC_BLK_DMU:
                if (blknum >= SOC_MAX_NUM_DMU_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many DMU blocks")));
                }
                si->dmu_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;

            case SOC_BLK_ETPPC:
                if (blknum >= SOC_MAX_NUM_ETPPC_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many ETPPC blocks")));
                }
                si->etppc_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;

            case SOC_BLK_EVNT:
                if (blknum >= SOC_MAX_NUM_EVNT_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many EVNT blocks")));
                }
                si->evnt_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;

            case SOC_BLK_HBC:
                if (blknum >= SOC_MAX_NUM_HBC_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many HBC blocks")));
                }
                si->hbc_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;

            case SOC_BLK_ILE:
                if (blknum >= SOC_MAX_NUM_ILE_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many ILE blocks")));
                }
                si->ile_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_ILU:
                if (blknum >= SOC_MAX_NUM_ILU_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many ILU blocks")));
                }
                si->ilu_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_NMG:
                if (blknum >= SOC_MAX_NUM_NMG_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many NMG blocks")));
                }
                si->nmg_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;

            case SOC_BLK_IPPE:
                if (blknum >= SOC_MAX_NUM_IPPE_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many IPPE blocks")));
                }
                si->ippe_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;

            case SOC_BLK_TCAM:
                if (blknum >= SOC_MAX_NUM_TCAM_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many TCAM blocks")));
                }
                si->tcam_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;

            case SOC_BLK_TDU:
                if (blknum >= SOC_MAX_NUM_TDU_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many TDU blocks")));
                }
                si->tdu_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_EPS:
                if (blknum >= SOC_MAX_NUM_EPS_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many EPS blocks")));
                }
                si->eps_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_HBMC:
                if (blknum >= SOC_MAX_NUM_HBMC_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many HBMC blocks")));
                }
                si->hbmc_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;

            case SOC_BLK_MTM:
                si->mtm_block = blk;
                break;
            case SOC_BLK_HBM:
                if (blknum >= SOC_MAX_NUM_HBM_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many HBM blocks")));
                }
                si->hbm_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_RQP:
                if (blknum >= SOC_MAX_NUM_RQP_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many RQP blocks")));
                }
                si->rqp_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_FQP:
                if (blknum >= SOC_MAX_NUM_FQP_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many FQP blocks")));
                }
                si->fqp_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_MDB:
                si->mdb_block = blk;
                break;
            case SOC_BLK_ERPP:
                if (blknum >= SOC_MAX_NUM_ERPP_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many ERPP blocks")));
                }
                si->erpp_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_ETPPA:
                if (blknum >= SOC_MAX_NUM_ETPPA_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many ETPPA blocks")));
                }
                si->etppa_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_ETPPB:
                if (blknum >= SOC_MAX_NUM_ETPPB_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many ETPPB blocks")));
                }
                si->etppb_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_MACT:
                si->mact_block = blk;
                break;
            case SOC_BLK_IPPA:
                if (blknum >= SOC_MAX_NUM_IPPA_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many IPPA blocks")));
                }
                si->ippa_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_IPPB:
                if (blknum >= SOC_MAX_NUM_IPPB_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many IPPB blocks")));
                }
                si->ippb_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_IPPC:
                if (blknum >= SOC_MAX_NUM_IPPC_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many IPPC blocks")));
                }
                si->ippc_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_IPPD:
                if (blknum >= SOC_MAX_NUM_IPPD_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many IPPD blocks")));
                }
                si->ippd_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_CMIC:
                si->cmic_block = blk;
                break;
            case SOC_BLK_XLP:
                if (blknum >= SOC_MAX_NUM_XLP_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many XLP blocks")));
                }
                si->xlp_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;      /* will be override if port is defined */
                break;
            case SOC_BLK_CLP:
                if (blknum >= SOC_MAX_NUM_CLP_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many CLP blocks %d"),
                                          blknum));
                }
                si->clp_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;      /* will be override if port is defined */
                break;
            case SOC_BLK_GPORT:
                if (blknum >= SOC_MAX_NUM_GPORT_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many GPORT blocks")));
                }
                si->gport_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;      /* will be override if port is defined */
                break;
            case SOC_BLK_PMQ:
                if (blknum >= SOC_MAX_NUM_PMQ_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many PMQ blocks")));
                }
                si->pmq_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;      /* will be override if port is defined */
                break;
            case SOC_BLK_FSRD:
                if (blknum >= SOC_MAX_NUM_FSRD_BLKS)
                {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,
                                         (_BSL_SOCDNX_MSG
                                          ("soc_dpp_info_config_blocks: soc_dpp_info_config: too many FSRD blocks")));
                }
                si->fsrd_block[blknum] = blk;
                si->block_port[blk] = blknum | SOC_REG_ADDR_INSTANCE_MASK;
                break;
            case SOC_BLK_SPB:
                si->spb_block = blk;
                break;
            case SOC_BLK_DDP:
                si->ddp_block = blk;
                break;
            case SOC_BLK_SQM:
                si->sqm_block = blk;
                break;
            case SOC_BLK_DQM:
                si->dqm_block = blk;
                break;
            case SOC_BLK_ECGM:
                si->ecgm_block = blk;
                break;
            case SOC_BLK_GCK:
                si->gck_block = blk;
                break;
            case SOC_BLK_PEM:
                si->pem_block = blk;
                break;
            default:
                break;
        }

        si->block_valid[blk] += 1;

        sal_snprintf(si->block_name[blk], sizeof(si->block_name[blk]),
                     "%s%s", soc_block_name_lookup_ext(blktype, unit), instance_string);
    }
    si->block_num = blk;

exit:
    SOCDNX_FUNC_RETURN;

}
