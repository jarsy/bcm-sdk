/*

 * $Id: dnxf_drv.c,v 1.87 Broadcom SDK $

 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT

#include <shared/bsl.h>


#include <soc/mcm/driver.h>     /* soc_base_driver_table */
#include <soc/error.h>
#include <soc/ipoll.h>
#include <soc/mem.h>
#include <soc/drv.h>
#include <soc/linkctrl.h>

#include <soc/dnxc/legacy/fabric.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnxc/legacy/dnxc_dev_feature_manager.h>

#include <soc/dnxf/cmn/dnxf_drv.h>
#include <soc/dnxf/cmn/mbcm.h>
#include <soc/dnxf/cmn/dnxf_warm_boot.h>
#include <soc/dnxf/cmn/dnxf_config_defs.h>
#include <soc/dnxf/cmn/dnxf_port.h>
#include <soc/dnxc/legacy/dnxc_cmic.h>
#include <soc/dnxc/legacy/dnxc_mem.h>


#ifdef BCM_88750_SUPPORT
#include <soc/dnxf/fe1600/fe1600_drv.h>
#include <soc/dnxf/fe1600/fe1600_link.h>
#include <soc/dnxf/fe1600/fe1600_interrupts.h>
#endif


/* Dfe interrupt CB */
soc_interrupt_fn_t dnxf_intr_fn = soc_intr;


int soc_dnxf_tbl_is_dynamic(int unit, soc_mem_t mem)
{
    return MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_tbl_is_dynamic, (unit, mem));
}

/* 
 * Mark all cacheable tables
 */
void soc_dnxf_tbl_mark_cachable(int unit) {
    soc_mem_t mem;
    for (mem = 0; mem < NUM_SOC_MEM; mem++) {
        if (!SOC_MEM_IS_VALID(unit, mem)) {
            continue;
        };
        /* Skip the Read-only/Write-Only/Signal tables */
        if (soc_mem_is_readonly(unit, mem) || soc_mem_is_writeonly(unit, mem) || soc_mem_is_signal(unit, mem))
        {
            continue;
        }
        /* dynamic tables are not cacheable*/
        if (soc_dnxf_tbl_is_dynamic(unit,mem))
        {
            continue;
        }
        /* uncacheable memories */

        /* this memory is updated from other memory. therefore it can not be cached with current cache mechanism */
        if (mem == RTP_MCTm) {
            continue;
        }
        if (mem == RTP_SLSCTm) {
            if(soc_dnxf_load_balancing_mode_destination_unreachable != SOC_DNXF_CONFIG(unit).fabric_load_balancing_mode) {
                continue;
            }
        }
        if (mem == FSRD_FSRD_WL_EXT_MEMm) {
            continue;
        }

        /*SER tables should be not cacheable*/
        if ((mem == SER_ACC_TYPE_MAPm) ||
            (mem ==  SER_MEMORYm) ||
            (mem == SER_RESULT_0m) ||
            (mem == SER_RESULT_1m) ||
            (mem == SER_RESULT_DATA_0m) ||
            (mem ==  SER_RESULT_DATA_1m) ||
            (mem ==  SER_RESULT_EXPECTED_0m) ||
            (mem == SER_RESULT_EXPECTED_1m)) {
            continue;
        }
        SOC_MEM_INFO(unit, mem).flags |= SOC_MEM_FLAG_CACHABLE;
    }
}

/* Local dnxf soc_intr() */
void dnxf_local_soc_intr(void *_unit)
{
    dnxf_intr_fn(_unit);
}


extern soc_controlled_counter_t soc_dnxf_controlled_counter[];


int
soc_dnxf_misc_init(int unit)
{
    return SOC_E_NONE;
}

int
soc_dnxf_mmu_init(int unit)
{
    return SOC_E_NONE;
}

soc_functions_t soc_dnxf_drv_funs = {
    soc_dnxf_misc_init,
    soc_dnxf_mmu_init,
    NULL,
    NULL,
    NULL,
};

STATIC int
soc_dnxf_info_soc_properties(int unit) 
{

    DNXC_INIT_FUNC_DEFS;    

    DNXC_IF_ERR_EXIT(soc_dnxf_drv_soc_properties_general_read(unit));

    DNXC_IF_ERR_EXIT(soc_dnxf_drv_soc_properties_chip_read(unit));

    DNXC_IF_ERR_EXIT(soc_dnxf_drv_soc_properties_multicast_read(unit));

    DNXC_IF_ERR_EXIT(soc_dnxf_drv_soc_properties_fabric_pipes_read(unit));

    DNXC_IF_ERR_EXIT(soc_dnxf_drv_soc_properties_access_read(unit));

    DNXC_IF_ERR_EXIT(soc_dnxf_drv_soc_properties_port_read(unit));

    DNXC_IF_ERR_EXIT(soc_dnxf_drv_soc_properties_fabric_cell_read(unit));
    
    DNXC_IF_ERR_EXIT(soc_dnxf_drv_soc_properties_fabric_routing_read(unit));

    DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL_NO_ARGS(unit, mbcm_dnxf_drv_soc_properties_validate));
exit:
    DNXC_FUNC_RETURN;
}

int 
soc_dnxf_drv_soc_properties_fabric_pipes_read(int unit)
{    
    int nof_pipes;
    int i;
    int j;
    soc_dnxc_fabric_pipe_map_t fabric_pipe_map_valid_config[SOC_DNXC_FABRIC_PIPE_NUM_OF_FABRIC_PIPE_VALID_CONFIGURATIONS];
    int is_valid=1;
    char *str;
    int rv;

    DNXC_INIT_FUNC_DEFS;

    SOC_DNXF_CONFIG(unit).is_dual_mode = soc_dnxf_property_get(unit, spn_IS_DUAL_MODE,  0, SOC_DNXF_PROPERTY_UNAVAIL);
    SOC_DNXF_CONFIG(unit).system_is_dual_mode_in_system = soc_dnxf_property_get(unit, spn_SYSTEM_IS_DUAL_MODE_IN_SYSTEM,  0, SOC_DNXF_PROPERTY_UNAVAIL);
    SOC_DNXF_CONFIG(unit).system_is_single_mode_in_system = soc_dnxf_property_get(unit, spn_SYSTEM_IS_SINGLE_MODE_IN_SYSTEM,  0, SOC_DNXF_PROPERTY_UNAVAIL);
    SOC_DNXF_CONFIG(unit).system_contains_multiple_pipe_device = soc_dnxf_property_get(unit, spn_SYSTEM_CONTAINS_MULTIPLE_PIPE_DEVICE,  0, SOC_DNXF_PROPERTY_UNAVAIL);
    str = soc_dnxf_property_get_str(unit, spn_FABRIC_TDM_PRIORITY_MIN,0,NULL);
    rv = soc_dnxf_property_str_to_enum(unit, spn_FABRIC_TDM_PRIORITY_MIN, soc_dnxf_property_str_enum_fabric_tdm_priority_min, str, &SOC_DNXF_CONFIG(unit).fabric_tdm_priority_min);
    DNXC_IF_ERR_EXIT(rv);

    rv = soc_dnxc_fabric_pipe_map_initalize_valid_configurations(unit, 
                                                                     SOC_DNXF_CONFIG(unit).fabric_tdm_priority_min == SOC_DNXF_FABRIC_TDM_PRIORITY_NONE ? SOC_DNXC_FABRIC_PIPE_MAX_NUM_OF_PRIORITIES : SOC_DNXF_CONFIG(unit).fabric_tdm_priority_min, /*Set to num of priorities in case no tdm priority*/
                                                                     fabric_pipe_map_valid_config);
    DNXC_IF_ERR_EXIT(rv);
    nof_pipes=soc_dnxf_property_get(unit,spn_FABRIC_NUM_PIPES,0, SOC_DNXF_PROPERTY_UNAVAIL);

    SOC_DNXF_FABRIC_PIPES_CONFIG(unit).nof_pipes=nof_pipes;

    if (nof_pipes == SOC_DNXF_PROPERTY_UNAVAIL) /* soc property isn't supported*/
    {
        SOC_DNXF_FABRIC_PIPES_CONFIG(unit).nof_pipes=(SOC_DNXF_CONFIG(unit).is_dual_mode? 2:1);
        SOC_DNXF_FABRIC_PIPES_CONFIG(unit).mapping_type = (SOC_DNXF_CONFIG(unit).is_dual_mode? soc_dnxc_fabric_pipe_map_dual_tdm_non_tdm:soc_dnxc_fabric_pipe_map_single);

        for (i=0;i<SOC_DNXC_FABRIC_PIPE_MAX_NUM_OF_PRIORITIES;i++)
        {
            SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_uc[i]=SOC_DNXF_PROPERTY_UNAVAIL;
            SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_mc[i]=SOC_DNXF_PROPERTY_UNAVAIL;
        }

        SOC_EXIT; 
    }

    if (nof_pipes == 1)
    {
        /*No need to configure pipe mapping*/
        for (i=0;i<SOC_DNXC_FABRIC_PIPE_MAX_NUM_OF_PRIORITIES;i++)
        {
            SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_uc[i]=0;
            SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_mc[i]=0;
        }
        SOC_EXIT;
    }
    
    SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_uc[0] = soc_dnxf_property_suffix_num_get(unit, 0, spn_FABRIC_PIPE_MAP, "uc",1, SOC_DNXF_PROPERTY_UNAVAIL);
    if (SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_uc[0] == SOC_DNXF_PROPERTY_UNAVAIL)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("unicast priority 0 isn't configued- if number of pipes > 1, all priorities must be configued")));
    }

    SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_uc[1] = soc_dnxf_property_suffix_num_get(unit, 1, spn_FABRIC_PIPE_MAP, "uc",1, SOC_DNXF_PROPERTY_UNAVAIL);
    if (SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_uc[1] == SOC_DNXF_PROPERTY_UNAVAIL)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("unicast priority 1 isn't configued- if number of pipes > 1, all priorities must be configued")));
    }

    SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_uc[2] = soc_dnxf_property_suffix_num_get(unit, 2, spn_FABRIC_PIPE_MAP, "uc",1, SOC_DNXF_PROPERTY_UNAVAIL);
    if (SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_uc[2] == SOC_DNXF_PROPERTY_UNAVAIL)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("unicast priority 2 isn't configued- if number of pipes > 1, all priorities must be configued")));

    }

    SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_uc[3] = soc_dnxf_property_suffix_num_get(unit, 3, spn_FABRIC_PIPE_MAP, "uc",1, SOC_DNXF_PROPERTY_UNAVAIL); /* if we have one pipe then pipe should be 0, if two pipes then pipe should be 1, if three pipes then pipe should be 2 (default) */
    if (SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_uc[3] == SOC_DNXF_PROPERTY_UNAVAIL)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("unicast priority 3 isn't configued- if number of pipes > 1, all priorities must be configued")));

    }

    SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_mc[0] = soc_dnxf_property_suffix_num_get(unit, 0, spn_FABRIC_PIPE_MAP, "mc",1, SOC_DNXF_PROPERTY_UNAVAIL);
    if (SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_mc[0] == SOC_DNXF_PROPERTY_UNAVAIL)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("multicast priority 0 isn't configued- if number of pipes > 1, all priorities must be configued")));

    }

    SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_mc[1] = soc_dnxf_property_suffix_num_get(unit, 1, spn_FABRIC_PIPE_MAP, "mc",1, SOC_DNXF_PROPERTY_UNAVAIL);
    if (SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_mc[1] == SOC_DNXF_PROPERTY_UNAVAIL)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("multicast priority 1 isn't configued- if number of pipes > 1, all priorities must be configued")));

    }

    SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_mc[2] = soc_dnxf_property_suffix_num_get(unit, 2, spn_FABRIC_PIPE_MAP, "mc",1, SOC_DNXF_PROPERTY_UNAVAIL);
    if (SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_mc[2] == SOC_DNXF_PROPERTY_UNAVAIL)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("multicast priority 2 isn't configued- if number of pipes > 1, all priorities must be configued")));

    }

    SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_mc[3] = soc_dnxf_property_suffix_num_get(unit, 3, spn_FABRIC_PIPE_MAP, "mc",1, SOC_DNXF_PROPERTY_UNAVAIL);
    if (SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_mc[3] == SOC_DNXF_PROPERTY_UNAVAIL)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("multicast priority 3 isn't configued- if number of pipes > 1, all priorities must be configued")));

    }


    /* check if invalid pipe was configured (more than number of pipes) */
    for (i=0;i<4;i++)
    {
        if ( (SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_uc[i] >= nof_pipes) || (SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_mc[i] >= nof_pipes) ||
             (SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_uc[i] < 0) || (SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_mc[i] < 0)  )
        {
            DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Invalid pipe number - more than configured")));
        }
    }
    /* check if the configuration is valid*/

    if (nof_pipes!=1)
    {
        
        for (i=0;i<SOC_DNXC_FABRIC_PIPE_NUM_OF_FABRIC_PIPE_VALID_CONFIGURATIONS;i++)
        {
            if (nof_pipes==fabric_pipe_map_valid_config[i].nof_pipes)
            {
                is_valid=1;
                for (j=0;j<SOC_DNXC_FABRIC_PIPE_MAX_NUM_OF_PRIORITIES;j++)
                {
                    is_valid= (is_valid && (SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_uc[j]==fabric_pipe_map_valid_config[i].config_uc[j]) &&
                                           (SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_mc[j]==fabric_pipe_map_valid_config[i].config_mc[j]));
                }
                if (is_valid)
                {
                    SOC_DNXF_FABRIC_PIPES_CONFIG(unit).mapping_type = fabric_pipe_map_valid_config[i].mapping_type;
                    break;
                }
            }
        }
        
    }

    if (!is_valid) /* invalid configuration */
    {
        if (SOC_DNXF_CONFIG(unit).custom_feature_lab==0)
        {
            DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Invalid configuration")));
        }
        else
        {
            SOC_DNXF_FABRIC_PIPES_CONFIG(unit).mapping_type = (nof_pipes == 2) ? soc_dnxc_fabric_pipe_map_dual_custom:soc_dnxc_fabric_pipe_map_triple_custom;
            LOG_CLI((BSL_META_U(unit,
                                "warning: using an invalid configuration for fabric pipes")));
        }
    }

    if (SOC_DNXF_FABRIC_PIPES_CONFIG(unit).mapping_type == soc_dnxc_fabric_pipe_map_triple_uc_hp_mc_lp_mc &&
        SOC_DNXF_CONFIG(unit).fe_mc_priority_map_enable == 1)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Invalid Configuration - cannot configure triple pipe uc,hp-mc,lp-mc mode & mc priority map")));
    }
    


exit:
    DNXC_FUNC_RETURN;
}

int 
soc_dnxf_drv_soc_properties_access_read(int unit)
{
       int rv;
    DNXC_INIT_FUNC_DEFS;

    /*SCHAN*/
    if (SAL_BOOT_QUICKTURN) {
        SOC_CONTROL(unit)->schanTimeout = SCHAN_TIMEOUT_QT;
    } else if (SAL_BOOT_PLISIM) {
        SOC_CONTROL(unit)->schanTimeout = SCHAN_TIMEOUT_PLI;
    } else {
        SOC_CONTROL(unit)->schanTimeout = SCHAN_TIMEOUT;
    }
    SOC_CONTROL(unit)->schanTimeout = soc_dnxf_property_get(unit, spn_SCHAN_TIMEOUT_USEC,1, SOC_CONTROL(unit)->schanTimeout);
    SOC_CONTROL(unit)->schanIntrEnb = soc_dnxf_property_get(unit, spn_SCHAN_INTR_ENABLE, 0, SOC_DNXF_PROPERTY_UNAVAIL);

    /*MIIM access*/
    SOC_CONTROL(unit)->miimTimeout =  soc_dnxf_property_get(unit, spn_MIIM_TIMEOUT_USEC,0, SOC_DNXF_PROPERTY_UNAVAIL);
    SOC_CONTROL(unit)->miimIntrEnb = soc_dnxf_property_get(unit, spn_MIIM_INTR_ENABLE,0, SOC_DNXF_PROPERTY_UNAVAIL);
    SOC_DNXF_CONFIG(unit).mdio_int_dividend = soc_dnxf_property_get(unit, spn_RATE_INT_MDIO_DIVIDEND, 0, SOC_DNXF_PROPERTY_UNAVAIL);
    SOC_DNXF_CONFIG(unit).mdio_int_divisor = soc_dnxf_property_get(unit, spn_RATE_INT_MDIO_DIVISOR, 0, SOC_DNXF_PROPERTY_UNAVAIL);

    /*Memory bist*/
    if ((SOC_DNXF_CONFIG(unit).run_mbist = soc_dnxf_property_get(unit, spn_BIST_ENABLE, 0, SOC_DNXF_PROPERTY_UNAVAIL))) {
        if (SOC_DNXF_CONFIG(unit).run_mbist != 2) {
            SOC_DNXF_CONFIG(unit).run_mbist = 1;
        }
    }

    /*Fabric Cell FIFO DMA*/
    SOC_DNXF_CONFIG(unit).fabric_cell_fifo_dma_enable = soc_dnxf_property_get(unit, spn_FABRIC_CELL_FIFO_DMA_ENABLE, 0, SOC_DNXF_PROPERTY_UNAVAIL);
    if (SOC_DNXF_CONFIG(unit).fabric_cell_fifo_dma_enable != SOC_DNXF_PROPERTY_UNAVAIL && SOC_DNXF_CONFIG(unit).fabric_cell_fifo_dma_enable)
    {
        SOC_DNXF_CONFIG(unit).fabric_cell_fifo_dma_buffer_size = soc_dnxf_property_get(unit, spn_FABRIC_CELL_FIFO_DMA_BUFFER_SIZE, 0, SOC_DNXF_PROPERTY_UNAVAIL);
        SOC_DNXF_CONFIG(unit).fabric_cell_fifo_dma_timeout = soc_dnxf_property_get(unit, spn_FABRIC_CELL_FIFO_DMA_TIMEOUT, 0, SOC_DNXF_PROPERTY_UNAVAIL);
        SOC_DNXF_CONFIG(unit).fabric_cell_fifo_dma_threshold = soc_dnxf_property_get(unit, spn_FABRIC_CELL_FIFO_DMA_THRESHOLD, 0, SOC_DNXF_PROPERTY_UNAVAIL);
        rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_fifo_dma_fabric_cell_validate, (unit));
        DNXC_IF_ERR_EXIT(rv);
    } else {
        SOC_DNXF_CONFIG(unit).fabric_cell_fifo_dma_buffer_size  = SOC_DNXF_PROPERTY_UNAVAIL;
        SOC_DNXF_CONFIG(unit).fabric_cell_fifo_dma_timeout = SOC_DNXF_PROPERTY_UNAVAIL;
        SOC_DNXF_CONFIG(unit).fabric_cell_fifo_dma_threshold = SOC_DNXF_PROPERTY_UNAVAIL;
    }



exit:
    DNXC_FUNC_RETURN;
}

int 
soc_dnxf_drv_soc_properties_chip_read(int unit)
{
    DNXC_INIT_FUNC_DEFS;

    /*core clock + ref clock*/
    if (soc_property_get_str(unit, spn_SYSTEM_REF_CORE_CLOCK) != NULL) {
        /*configure ref clock in MHz*/
        SOC_DNXF_CONFIG(unit).system_ref_core_clock = soc_dnxf_property_get(unit, spn_SYSTEM_REF_CORE_CLOCK, 0, SOC_DNXF_PROPERTY_UNAVAIL) * 1000;
    } else {
        /*configure ref clock in KHz*/
        SOC_DNXF_CONFIG(unit).system_ref_core_clock = soc_dnxf_property_suffix_num_get(unit, 0, spn_SYSTEM_REF_CORE_CLOCK, "khz",1 , SOC_DNXF_PROPERTY_UNAVAIL); 
        if (SOC_DNXF_CONFIG(unit).system_ref_core_clock ==  SOC_DNXF_PROPERTY_UNAVAIL) /* system ref core clock wasn't configued in khz */
        {
            SOC_DNXF_CONFIG(unit).system_ref_core_clock = soc_dnxf_property_get(unit, spn_SYSTEM_REF_CORE_CLOCK, 0, SOC_DNXF_PROPERTY_UNAVAIL) * 1000;
        }
    }

    if (soc_property_get_str(unit, spn_CORE_CLOCK_SPEED) != NULL) {
        /*configure core clock in MHz*/
        SOC_DNXF_CONFIG(unit).core_clock_speed  = soc_dnxf_property_get(unit, spn_CORE_CLOCK_SPEED, 0, SOC_DNXF_PROPERTY_UNAVAIL) * 1000;
    } else {
        /*configure core clock in KHz*/
        SOC_DNXF_CONFIG(unit).core_clock_speed = soc_dnxf_property_suffix_num_get(unit,0, spn_CORE_CLOCK_SPEED, "khz", 1 , SOC_DNXF_PROPERTY_UNAVAIL);
        if (SOC_DNXF_CONFIG(unit).core_clock_speed ==  SOC_DNXF_PROPERTY_UNAVAIL ) /* core clock speed wasn't configured in khz */
        {
            SOC_DNXF_CONFIG(unit).core_clock_speed = soc_dnxf_property_get(unit, spn_CORE_CLOCK_SPEED, 0, SOC_DNXF_PROPERTY_UNAVAIL ) * 1000;
        }
    }

/*exit:*/
    DNXC_FUNC_RETURN;
}

int 
soc_dnxf_drv_soc_properties_multicast_read(int unit)
{
    char *str;
    int rv;

    DNXC_INIT_FUNC_DEFS;

    str = soc_dnxf_property_get_str(unit, spn_FABRIC_MULTICAST_MODE,0,NULL);
    rv = soc_dnxf_property_str_to_enum(unit, spn_FABRIC_MULTICAST_MODE, soc_dnxf_property_str_enum_fabric_multicast_mode, str, &SOC_DNXF_CONFIG(unit).fabric_multicast_mode);
    DNXC_IF_ERR_EXIT(rv);

    if (soc_feature(unit, soc_feature_fe_mc_id_range))
    {
        str = soc_dnxf_property_get_str(unit, spn_FE_MC_ID_RANGE,0,NULL);
        rv = soc_dnxf_property_str_to_enum(unit, spn_FE_MC_ID_RANGE, soc_dnxf_property_str_enum_fe_mc_id_range, str, &SOC_DNXF_CONFIG(unit).fe_mc_id_range);
    }
    else
    {
        SOC_DNXF_CONFIG(unit).fe_mc_id_range = soc_dnxf_multicast_table_mode_64k;
    }
    DNXC_IF_ERR_EXIT(rv);
    if (soc_feature(unit, soc_feature_fe_mc_priority_mode_enable))
    {
        SOC_DNXF_CONFIG(unit).fe_mc_priority_map_enable = soc_dnxf_property_get(unit, spn_FE_MC_PRIORITY_MAP_ENABLE,  0,  SOC_DNXF_PROPERTY_UNAVAIL);
    }
    else
    {
        SOC_DNXF_CONFIG(unit).fe_mc_priority_map_enable = 1; /*default for ramon_fe1600*/
    }

exit:
    DNXC_FUNC_RETURN;
}

int 
soc_dnxf_drv_soc_properties_port_read(int unit)
{
    int lcpll, rc;
    DNXC_INIT_FUNC_DEFS;

    for (lcpll = 0; lcpll < SOC_DNXF_NOF_LCPLL; lcpll++)
    {
        SOC_DNXF_CONFIG(unit).fabric_port_lcpll_in[lcpll] = soc_dnxf_property_suffix_num_get(unit, lcpll, spn_SERDES_FABRIC_CLK_FREQ, "in", 0, SOC_DNXF_PROPERTY_UNAVAIL);
        SOC_DNXF_CONFIG(unit).fabric_port_lcpll_out[lcpll] = soc_dnxf_property_suffix_num_get(unit, lcpll, spn_SERDES_FABRIC_CLK_FREQ, "out", 0, SOC_DNXF_PROPERTY_UNAVAIL);
    }

    if (SOC_DNXF_IS_FE13_ASYMMETRIC(unit) || SOC_DNXF_IS_FE2(unit))
    {
        SOC_DNXF_CONFIG(unit).fabric_clk_freq_in_quad_26 = soc_dnxf_property_suffix_num_get_only_suffix(unit, 26, spn_SERDES_FABRIC_CLK_FREQ, "in_quad", 1, -1); 
        SOC_DNXF_CONFIG(unit).fabric_clk_freq_in_quad_35 = soc_dnxf_property_suffix_num_get_only_suffix(unit, 35, spn_SERDES_FABRIC_CLK_FREQ, "in_quad", 1, -1);
        SOC_DNXF_CONFIG(unit).fabric_clk_freq_out_quad_26 = soc_dnxf_property_suffix_num_get_only_suffix(unit, 26, spn_SERDES_FABRIC_CLK_FREQ, "out_quad", 1, -1); 
        SOC_DNXF_CONFIG(unit).fabric_clk_freq_out_quad_35 = soc_dnxf_property_suffix_num_get_only_suffix(unit, 35, spn_SERDES_FABRIC_CLK_FREQ, "out_quad", 1, -1);
        if (SOC_DNXF_CONFIG(unit).fabric_clk_freq_in_quad_26 != SOC_DNXF_CONFIG(unit).fabric_clk_freq_out_quad_26) {
            DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("Fabric clock in frequency must be equal to out frequency for quad 26")));
        }
        if (SOC_DNXF_CONFIG(unit).fabric_clk_freq_in_quad_35 != SOC_DNXF_CONFIG(unit).fabric_clk_freq_out_quad_35) {
            DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("Fabric clock in frequency must be equal to out frequency for quad 35")));
        }
    }

    SOC_DNXF_CONFIG(unit).serdes_mixed_rate_enable = soc_dnxf_property_get(unit, spn_SERDES_MIXED_RATE_ENABLE,  0, SOC_DNXF_PROPERTY_UNAVAIL);
    if (SOC_DNXF_CONFIG(unit).serdes_mixed_rate_enable !=  SOC_DNXF_PROPERTY_UNAVAIL) /*i.e. supported*/
    {
        if (SOC_IS_FE1600_A0(unit) && SOC_DNXF_CONFIG(unit).serdes_mixed_rate_enable) {
                DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("Property %s is not supported by Fe1600_A0"),spn_SERDES_MIXED_RATE_ENABLE));
        }
    }

    SOC_DNXF_CONFIG(unit).fabric_optimize_patial_links = soc_dnxf_property_get(unit, spn_FABRIC_OPTIMIZE_PARTIAL_LINKS, 0, SOC_DNXF_PROPERTY_UNAVAIL);

    SOC_DNXF_CONFIG(unit).fabric_mac_bucket_fill_rate = soc_dnxf_property_get(unit, spn_FABRIC_MAC_BUCKET_FILL_RATE,  0, SOC_DNXF_PROPERTY_UNAVAIL);

    if (SOC_DNXF_CONFIG(unit).fabric_mac_bucket_fill_rate != SOC_DNXF_PROPERTY_UNAVAIL)
    {
        DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_bucket_fill_rate_validate,(unit, SOC_DNXF_CONFIG(unit).fabric_mac_bucket_fill_rate)));
    }

    rc = _soc_dnxf_drv_soc_property_serdes_qrtt_read(unit);
    DNXC_IF_ERR_EXIT(rc);


exit:
    DNXC_FUNC_RETURN;
}

int 
soc_dnxf_drv_soc_properties_fabric_cell_read(int unit)
{
    DNXC_INIT_FUNC_DEFS;


    SOC_DNXF_CONFIG(unit).system_is_vcs_128_in_system = soc_dnxf_property_get(unit, spn_SYSTEM_IS_VCS_128_IN_SYSTEM,  0, SOC_DNXF_PROPERTY_UNAVAIL);

    SOC_DNXF_CONFIG(unit).system_is_fe600_in_system = soc_dnxf_property_get(unit, spn_SYSTEM_IS_FE600_IN_SYSTEM,  0, SOC_DNXF_PROPERTY_UNAVAIL);
    SOC_DNXF_CONFIG(unit).fabric_merge_cells = soc_dnxf_property_get(unit, spn_FABRIC_MERGE_CELLS,  0, SOC_DNXF_PROPERTY_UNAVAIL);
    SOC_DNXF_CONFIG(unit).fabric_TDM_fragment = soc_dnxf_property_get(unit, spn_FABRIC_TDM_FRAGMENT,  0, SOC_DNXF_PROPERTY_UNAVAIL);

    SOC_DNXF_CONFIG(unit).fabric_TDM_over_primary_pipe = soc_dnxf_property_get(unit, spn_FABRIC_TDM_OVER_PRIMARY_PIPE,  0, SOC_DNXF_PROPERTY_UNAVAIL);

    if (!SOC_DNXF_CONFIG(unit).fabric_TDM_over_primary_pipe) {
        SOC_DNXF_CONFIG(unit).vcs128_unicast_priority = soc_dnxf_property_get(unit, spn_VCS128_UNICAST_PRIORITY, 1,  DNXF_VCS128_UNICAST_PRIORITY_DEFAULT);
        if ( SOC_DNXF_CONFIG(unit).vcs128_unicast_priority != SOC_DNXF_PROPERTY_UNAVAIL) /*if supported*/
        {
            if(!DNXF_IS_PRIORITY_VALID(SOC_DNXF_CONFIG(unit).vcs128_unicast_priority)) {
                DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Illegal priority %d"),SOC_DNXF_CONFIG(unit).vcs128_unicast_priority));
            } else if (SOC_DNXF_CONFIG(unit).vcs128_unicast_priority >= soc_dnxf_fabric_priority_3){
                DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Illegal priority %d - TDM priority"),SOC_DNXF_CONFIG(unit).vcs128_unicast_priority));
            }
        }
    } else {
        SOC_DNXF_CONFIG(unit).vcs128_unicast_priority = soc_dnxf_property_get(unit, spn_VCS128_UNICAST_PRIORITY, 1,  DNXF_VCS128_UNICAST_PRIORITY_TDM_OVER_PRIMARY_PIPE_DEFAULT);
        if ( SOC_DNXF_CONFIG(unit).vcs128_unicast_priority != SOC_DNXF_PROPERTY_UNAVAIL) /*if supported*/
        {
            if(!DNXF_IS_PRIORITY_VALID(SOC_DNXF_CONFIG(unit).vcs128_unicast_priority)) 
            {
                DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Illegal priority %d"),SOC_DNXF_CONFIG(unit).vcs128_unicast_priority));
            } else if (SOC_DNXF_CONFIG(unit).vcs128_unicast_priority >= soc_dnxf_fabric_priority_2) {
                DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Illegal priority %d - TDM priority"),SOC_DNXF_CONFIG(unit).vcs128_unicast_priority));
            }
        }
    }

    if (SOC_DNXF_CONFIG(unit).system_is_vcs_128_in_system !=  SOC_DNXF_PROPERTY_UNAVAIL && SOC_DNXF_CONFIG(unit).system_is_fe600_in_system != SOC_DNXF_PROPERTY_UNAVAIL) /*i.e. not supported*/
    {
        if (SOC_DNXF_CONFIG(unit).system_is_vcs_128_in_system==0 && SOC_DNXF_CONFIG(unit).system_is_fe600_in_system==1) 
        {
            DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Unsupported properties: is_fe600=1 & dpp_arad->init.fabric.is_128_in_system=0\n")));
        }
    }

    if(SOC_DNXF_CONFIG(unit).fabric_TDM_fragment != SOC_DNXF_PROPERTY_UNAVAIL){ /* i.e. supported */
        DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_TDM_fragment_validate,(unit, SOC_DNXF_CONFIG(unit).fabric_TDM_fragment)));
    }

    if (SOC_DNXF_CONFIG(unit).fabric_TDM_over_primary_pipe != SOC_DNXF_PROPERTY_UNAVAIL)
    {
        if (SOC_DNXF_CONFIG(unit).fabric_TDM_over_primary_pipe && !SOC_DNXF_CONFIG(unit).is_dual_mode) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("TDM_OVER_PRIMARY_PIPE: available only in and dual pipe device")));
        }
    }



exit:
    DNXC_FUNC_RETURN;
}


int 
soc_dnxf_drv_soc_properties_general_read(int unit)
{
    char *str;
    int rv;
    DNXC_INIT_FUNC_DEFS;

    rv = dnxc_property_get_str(unit, spn_FABRIC_DEVICE_MODE, &str);
    DNXC_IF_ERR_EXIT(rv);

    rv = soc_dnxf_property_str_to_enum(unit, spn_FABRIC_DEVICE_MODE, soc_dnxf_property_str_enum_fabric_device_mode, str, &SOC_DNXF_CONFIG(unit).fabric_device_mode);
    DNXC_IF_ERR_EXIT(rv);

    str = soc_dnxf_property_get_str(unit, spn_FABRIC_LOAD_BALANCING_MODE,0,NULL);
    rv = soc_dnxf_property_str_to_enum(unit, spn_FABRIC_LOAD_BALANCING_MODE, soc_dnxf_property_str_enum_fabric_load_balancing_mode, str, &SOC_DNXF_CONFIG(unit).fabric_load_balancing_mode);
    DNXC_IF_ERR_EXIT(rv);

    SOC_DNXF_CONFIG(unit).custom_feature_lab=soc_dnxf_property_suffix_num_get_only_suffix(unit,0,spn_CUSTOM_FEATURE,"lab",1,0);

    SOC_DNXF_CONFIG(unit).mesh_topology_fast= soc_dnxf_property_suffix_num_get_only_suffix(unit,0,spn_CUSTOM_FEATURE,"mesh_topology_fast",1,1);


exit:
    DNXC_FUNC_RETURN;
}

int
soc_dnxf_drv_soc_properties_fabric_routing_read(int unit)
{
    DNXC_INIT_FUNC_DEFS;
    
    if (SOC_DNXF_IS_FE13(unit))
    {
        SOC_DNXF_CONFIG(unit).fabric_local_routing_enable = soc_dnxf_property_get(unit, spn_FABRIC_LOCAL_ROUTING_ENABLE, 0, SOC_DNXF_PROPERTY_UNAVAIL);
    }
    else
    {
        SOC_DNXF_CONFIG(unit).fabric_local_routing_enable = 0;
    }

    DNXC_FUNC_RETURN;
}

int
_soc_dnxf_drv_soc_property_serdes_qrtt_read(int unit)
{
    soc_pbmp_t          pbmp_disabled;
    int                 quad, disabled_port, quad_index, port, rc, dis;
    uint32              quad_active;

    DNXC_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(pbmp_disabled);
    for (quad = 0; quad < SOC_DNXF_DEFS_GET(unit, nof_links)/4; quad++) {

       /*
        * SW restriction for SKUs.
        * Check if Quad is disabled for current device.
        */
        rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_quad_disabled, (unit, quad, &dis));
        DNXC_IF_ERR_EXIT(rc);

        if (dis == 1) {
           /*
            * The feature blocking mechanism works with SOC property name and currently has no way of blocking based on suffix.
            * This is why we use it only for the specific Quads that need to be disabled. If serdes_qrtt_active_<quad>=1 return error, else return 0 to disable QUAD.
            */
            DNXC_IF_ERR_EXIT(dnxc_property_suffix_num_get(unit, quad, spn_SERDES_QRTT_ACTIVE, "", 0, &quad_active));
        } else {
            quad_active = soc_property_suffix_num_get(unit, quad, spn_SERDES_QRTT_ACTIVE, "", 1); 
             if (quad_active) {
                 quad_active = soc_property_suffix_num_get(unit, quad, spn_PB_SERDES_QRTT_ACTIVE, "", 1);
             }
        }
         if (!quad_active) {
            for (quad_index = 0; quad_index < 4; quad_index++) {
                SOC_PBMP_PORT_ADD(pbmp_disabled, quad*4 + quad_index);
            }
         }
    }

    for (port = 0; port < SOC_DNXF_DEFS_GET(unit, nof_links) ; port++) {
        disabled_port = FALSE;
        if (PBMP_MEMBER(pbmp_disabled, port)) {
            disabled_port = TRUE;
        }
        
        if (SOC_IS_FE1600_A0(unit) ||  !disabled_port) {
            /* add to enabled bmp*/
            rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_dynamic_port_update, (unit, port, 1, soc_dnxf_port_update_type_sfi));
            DNXC_IF_ERR_EXIT(rc);

            rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_dynamic_port_update, (unit, port, 1, soc_dnxf_port_update_type_port));            
            DNXC_IF_ERR_EXIT(rc);

            rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_dynamic_port_update, (unit, port, 1, soc_dnxf_port_update_type_all));            
            DNXC_IF_ERR_EXIT(rc);

            /* remvoe from disabled bmp */
            rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_dynamic_port_update, (unit, port, 0, soc_dnxf_port_update_type_sfi_disabled));           
            DNXC_IF_ERR_EXIT(rc);

            rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_dynamic_port_update, (unit, port, 0, soc_dnxf_port_update_type_port_disabled));            
            DNXC_IF_ERR_EXIT(rc);

            rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_dynamic_port_update, (unit, port, 0, soc_dnxf_port_update_type_all_disabled));
            DNXC_IF_ERR_EXIT(rc);
            
        }
        if (disabled_port) {
            /* add to disabled bmp */
            rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_dynamic_port_update, (unit, port, 1, soc_dnxf_port_update_type_sfi_disabled));            
            DNXC_IF_ERR_EXIT(rc);

             rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_dynamic_port_update, (unit, port, 1, soc_dnxf_port_update_type_port_disabled));            
            DNXC_IF_ERR_EXIT(rc);

            rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_dynamic_port_update, (unit, port, 1, soc_dnxf_port_update_type_all_disabled));            
            DNXC_IF_ERR_EXIT(rc);

            /* remove from enabled bmp */
            if (!SOC_IS_FE1600_A0(unit))
            {
                rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_dynamic_port_update, (unit, port, 0, soc_dnxf_port_update_type_sfi));            
                DNXC_IF_ERR_EXIT(rc);

                rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_dynamic_port_update, (unit, port, 0, soc_dnxf_port_update_type_port));                
                DNXC_IF_ERR_EXIT(rc);

                rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_dynamic_port_update, (unit, port, 0, soc_dnxf_port_update_type_all));                
                DNXC_IF_ERR_EXIT(rc);
            }
        }
        /* A0: enabled: only 1s, disabled: true value */
    }
exit:
    DNXC_FUNC_RETURN;
}


int
soc_dnxf_control_init(int unit)
{
    int rv;
    uint16 dev_id;
    uint8 rev_id;
    soc_dnxf_control_t    *dnxf = NULL;
    DNXC_INIT_FUNC_DEFS;

    /*dnxf info config*/
    /*prepare config info for the next init sequnace*/
    dnxf = SOC_DNXF_CONTROL(unit);
    if (dnxf == NULL) {
      dnxf = (soc_dnxf_control_t *) sal_alloc(sizeof(soc_dnxf_control_t),
                                            "soc_dnxf_control");
      if (dnxf == NULL) {
          DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("failed to allocate soc_dnxf_control")));
      }
      sal_memset(dnxf, 0, sizeof (soc_dnxf_control_t));

      SOC_CONTROL(unit)->drv = dnxf;
   }

    soc_cm_get_id(unit, &dev_id, &rev_id);
    rv = soc_dnxf_info_config(unit, dev_id);
    DNXC_IF_ERR_CONT(rv);

exit:
    DNXC_FUNC_RETURN;
}

int dnxf_tbl_mem_cache_mem_set(int unit, soc_mem_t mem, void* en)
{
    int rc;
    int enable = *(int *)en;

    DNXC_INIT_FUNC_DEFS;

    SOC_MEM_ALIAS_TO_ORIG(unit,mem);
    if(!SOC_MEM_IS_VALID(unit, mem) || !soc_mem_is_cachable(unit, mem))
    {
        return SOC_E_NONE;
    }

    if ((SOC_MEM_INFO(unit, mem).blocks | SOC_MEM_INFO(unit, mem).blocks_hi) != 0) {

       rc = soc_mem_cache_set(unit, mem, COPYNO_ALL, enable);
       DNXC_IF_ERR_EXIT(rc);

    }

exit:
    DNXC_FUNC_RETURN;

}

int
dnxf_tbl_mem_cache_enable_parity_tbl(int unit, soc_mem_t mem, void* en)
{
    int rc = SOC_E_NONE;

    DNXC_INIT_FUNC_DEFS;

    /* If table has valid PARITY field - it should be cached */
    if (SOC_MEM_FIELD_VALID(unit, mem, PARITYf))
      rc = dnxf_tbl_mem_cache_mem_set(unit, mem, en);

    LOG_INFO(BSL_LS_SOC_MEM, (BSL_META_U(unit, "parity memory %s cache\n"),SOC_MEM_NAME(unit, mem)));
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXC_FUNC_RETURN;

}

int
dnxf_tbl_mem_cache_enable_ecc_tbl(int unit, soc_mem_t mem, void* en)
{
    int rc = SOC_E_NONE;

    DNXC_INIT_FUNC_DEFS;

    /* If table has valid ECC field - it should be cached */
    if (SOC_MEM_FIELD_VALID(unit, mem, ECCf))
      rc = dnxf_tbl_mem_cache_mem_set(unit, mem, en);

    LOG_INFO(BSL_LS_SOC_MEM, (BSL_META_U(unit, "ecc memory %s cache\n"),SOC_MEM_NAME(unit, mem)));
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXC_FUNC_RETURN;
}

int
soc_dnxf_cache_enable_init(int unit)
{
    uint32 cache_enable = 1;
    int enable_all, enable_parity, enable_ecc;

    DNXC_INIT_FUNC_DEFS;

    enable_all = soc_property_suffix_num_get(unit, 0, spn_MEM_CACHE_ENABLE, "all", 0);
    enable_parity         = soc_property_suffix_num_get(unit, 0, spn_MEM_CACHE_ENABLE, "parity", 1);
    enable_ecc             = soc_property_suffix_num_get(unit, 0, spn_MEM_CACHE_ENABLE, "ecc", 1);

    if (enable_all) {
        if (soc_mem_iterate(unit, dnxf_tbl_mem_cache_mem_set, &cache_enable) < 0)
            LOG_ERROR(BSL_LS_APPL_SOCMEM, (BSL_META_U(unit, "soc_dpe_cache_enable_init: unit %d all_cache enable failed\n"), unit));
    }

    if (enable_parity)
    {
        if (soc_mem_iterate(unit, dnxf_tbl_mem_cache_enable_parity_tbl, &cache_enable) < 0)
            LOG_ERROR(BSL_LS_APPL_SOCMEM, (BSL_META_U(unit, "soc_dnxf_cache_enable_init: unit %d parity cache enable failed\n"), unit));
    }

    if (enable_ecc)
    {
        if (soc_mem_iterate(unit, dnxf_tbl_mem_cache_enable_ecc_tbl, &cache_enable) < 0)
            LOG_ERROR(BSL_LS_APPL_SOCMEM, (BSL_META_U(unit, "soc_dnxf_cache_enable_init: unit %d ecc cache enable failed\n"), unit));
    }

    DNXC_FUNC_RETURN;
}

int
soc_dnxf_init_reset(int unit)
{
    int rc = SOC_E_NONE;
    soc_control_t* soc;

    DNXC_INIT_FUNC_DEFS;

    DNXF_UNIT_LOCK_TAKE_DNXC(unit); 

    soc = SOC_CONTROL(unit);

    /*CMIC endianess*/
    soc_endian_config(unit);

    /*properties init*/
    rc = soc_dnxf_info_soc_properties(unit);
    DNXC_IF_ERR_EXIT(rc);

    /* mark cacheble tables */
    soc_dnxf_tbl_mark_cachable(unit);

    rc = soc_dnxf_cache_enable_init(unit);
    DNXC_IF_ERR_EXIT(rc);

    /*warm boot mechanism init*/
    rc = soc_dnxf_warm_boot_init(unit);
    DNXC_IF_ERR_EXIT(rc);

    rc = MBCM_DNXF_DRIVER_CALL_NO_ARGS(unit, mbcm_dnxf_linkctrl_init);
    DNXC_IF_ERR_EXIT(rc);
    
    /*Reset*/
    if (!SOC_WARM_BOOT(unit) && !SOC_IS_RELOADING(unit))
    {
        SOC_DNXF_DRV_INIT_LOG(unit, "Device Reset");
        rc = MBCM_DNXF_DRIVER_CALL_NO_ARGS(unit, mbcm_dnxf_reset_device);
        DNXC_IF_ERR_EXIT(rc);

        /*Make sure chip is isolated*/
        SOC_DNXF_DRV_INIT_LOG(unit, "Device Isolate");
        rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_fabric_topology_isolate_set,(unit, soc_dnxc_isolation_status_isolated)); 
        DNXC_IF_ERR_EXIT(rc);      
        
    } else {
        /*Interrupts init need to be called if it skiped because of WB if above*/
        rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_interrupts_init, (unit));
        DNXC_IF_ERR_CONT(rc);
    }

    /*counter init*/
    rc = soc_counter_attach(unit);
    DNXC_IF_ERR_EXIT(rc);

    rc = MBCM_DNXF_DRIVER_CALL_NO_ARGS(unit, mbcm_dnxf_port_soc_init); 
    DNXC_IF_ERR_EXIT(rc);
    if(!SOC_IS_RAMON(unit)){
        rc = MBCM_DNXF_DRIVER_CALL_NO_ARGS(unit, mbcm_dnxf_drv_sw_ver_set); 
        DNXC_IF_ERR_EXIT(rc);
    }

    if (!SOC_WARM_BOOT(unit) && !SOC_IS_RELOADING(unit))
    {
        /* need to be at the end of soc_init in order to prevent ecc errors from un initialized memories */
        rc = MBCM_DNXF_DRIVER_CALL_NO_ARGS(unit, mbcm_dnxf_ser_init);
        DNXC_IF_ERR_EXIT(rc);
    }

    soc->soc_flags |= SOC_F_INITED;

exit:
    DNXF_UNIT_LOCK_RELEASE_DNXC(unit);
    if (DNXC_FUNC_ERROR)
    {
        soc_dnxf_deinit(unit);
    }
    DNXC_FUNC_RETURN;

}

int
soc_dnxf_init_no_reset(int unit)
{
    int rc = SOC_E_NONE;
    soc_control_t* soc;

    soc_pbmp_t pbmp_enabled, pbmp_unisolated;
    soc_port_t port;
    int enable;
    soc_dnxc_isolation_status_t link_isolation_status, device_isolation_status;


    DNXC_INIT_FUNC_DEFS;

    DNXF_UNIT_LOCK_TAKE_DNXC(unit);


    soc = SOC_CONTROL(unit);
    soc_endian_config(unit);

    if (!SOC_WARM_BOOT(unit) && !SOC_IS_RELOADING(unit))
    {
        SOC_PBMP_CLEAR(pbmp_enabled);
        SOC_PBMP_CLEAR(pbmp_unisolated);
        SOC_PBMP_ITER(PBMP_SFI_ALL(unit), port)
        {
            rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_phy_enable_get, (unit, port, &enable));
            DNXC_IF_ERR_EXIT(rc);
            if (enable) {
                SOC_PBMP_PORT_ADD(pbmp_enabled, port);
            }
            rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_fabric_links_isolate_get, (unit, port, &link_isolation_status));
            DNXC_IF_ERR_EXIT(rc);
            if (link_isolation_status == soc_dnxc_isolation_status_active)
            {
                SOC_PBMP_PORT_ADD(pbmp_unisolated, port);
            }
        }

        rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_fabric_topology_isolate_get, (unit, &device_isolation_status));
        DNXC_IF_ERR_EXIT(rc);

        rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_drv_graceful_shutdown_set, 
                                  (unit, pbmp_enabled, 1, pbmp_unisolated, 0));
        DNXC_IF_ERR_EXIT(rc);
        rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_drv_soft_init, (unit, SOC_DNXC_RESET_ACTION_INOUT_RESET));
        DNXC_IF_ERR_EXIT(rc);

        sal_usleep(10000); /*wait 10 mili sec*/

        rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_drv_graceful_shutdown_set, 
                                  (unit, pbmp_enabled, 0, pbmp_unisolated, device_isolation_status == soc_dnxc_isolation_status_isolated ? 1:0));
        DNXC_IF_ERR_EXIT(rc);
        

    }

    soc->soc_flags |= SOC_F_INITED;

exit:
    DNXF_UNIT_LOCK_RELEASE_DNXC(unit); 
    DNXC_FUNC_RETURN;

}


int
soc_dnxf_init(int unit, int reset) 
{
    int rc = SOC_E_NONE;
    DNXC_INIT_FUNC_DEFS; 
    
    if (reset)
    {
        rc=soc_dnxf_init_reset(unit);
        DNXC_IF_ERR_EXIT(rc);
    }
    else
    {
        rc=soc_dnxf_init_no_reset(unit);
        DNXC_IF_ERR_EXIT(rc);
    }

exit:
     
    DNXC_FUNC_RETURN;
}


soc_driver_t*
soc_dnxf_chip_driver_find(int unit , uint16 pci_dev_id, uint8 pci_rev_id)
{
    uint16              driver_dev_id;
    uint8               driver_rev_id;

    if (soc_cm_get_id_driver(pci_dev_id, pci_rev_id, &driver_dev_id, &driver_rev_id) < 0) {
        return NULL;
    }

     switch(driver_dev_id)
     {
     case BCM88790_DEVICE_ID:
        if (pci_rev_id == BCM88790_A0_REV_ID) {
           return &soc_driver_bcm88790_a0;
        } else {
             LOG_ERROR(BSL_LS_SOC_INIT,
                       (BSL_META_U(unit,
                                   "soc_dnxf_chip_driver_find: driver in devid table "
                                   "not in soc_base_driver_table\n")));
        }
         break;
       default:
            LOG_ERROR(BSL_LS_SOC_INIT,
                      (BSL_META_U(unit,
                                  "soc_dnxf_chip_driver_find: driver in devid table "
                                  "not in soc_base_driver_table\n")));
           break;         
     }
  
    return NULL;
}

int
soc_dnxf_info_config(int unit, int dev_id)
{
    soc_info_t          *si;
    soc_control_t       *soc;
    int                 mem, blk, blktype;
    char                instance_string[3];
    int rv, port, phy_port, bindex;

    DNXC_INIT_FUNC_DEFS;

    soc = SOC_CONTROL(unit);

    /*set chip string*/
    switch (dev_id) {
        case BCM88790_DEVICE_ID:
            SOC_CHIP_STRING(unit) = "ramon";
            break;

        default:
            SOC_CHIP_STRING(unit) = "???";
           LOG_ERROR(BSL_LS_SOC_INIT,
                     (BSL_META_U(unit,
                                 "soc_dnxf_info_config: driver device %04x unexpected\n"),
                                 dev_id));
            DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("failed to find device id")));
    }


    si  = &SOC_INFO(unit);
    si->driver_type = soc->chip_driver->type;
    si->driver_group = soc_chip_type_map[si->driver_type];

    DNXC_IF_ERR_CONT(soc_dnxf_defines_init(unit));

    DNXC_IF_ERR_CONT(soc_dnxf_implementation_defines_init(unit));


    si->fe.min          = si->fe.max          = -1;         si->fe.num = 0;
    si->ge.min          = si->ge.max          = -1;         si->ge.num = 0;
    si->xe.min          = si->xe.max          = -1;         si->xe.num = 0;
    si->hg.min          = si->hg.max          = -1;         si->hg.num = 0;
    si->hg_subport.min  = si->hg_subport.max  = -1;         si->hg_subport.num = 0;
    si->hl.min          = si->hl.max          = -1;         si->hl.num = 0;
    si->st.min          = si->st.max          = -1;         si->st.num = 0;
    si->gx.min          = si->gx.max          = -1;         si->gx.num = 0;
    si->xg.min          = si->xg.max          = -1;         si->xg.num = 0;
    si->spi.min         = si->spi.max         = -1;         si->spi.num = 0;
    si->spi_subport.min = si->spi_subport.max = -1;         si->spi_subport.num = 0;
    si->sci.min         = si->sci.max         = -1;         si->sci.num = 0;
    si->sfi.min         = si->sfi.max         = -1;         si->sfi.num = 0;
    si->port.min        = si->port.max        = -1;         si->port.num = 0;
    si->ether.min       = si->ether.max       = -1;         si->ether.num = 0;
    si->all.min         = si->all.max         = -1;         si->all.num = 0;
    
    
    si->port_num = 0;

    sal_memset(si->has_block, 0, sizeof(soc_block_t) * COUNTOF(si->has_block));

    for (blk = 0; blk < SOC_MAX_NUM_BLKS; blk++) {
        si->block_port[blk] = REG_PORT_ANY;
        si->block_valid[blk] = 0;
    }

    SOC_PBMP_CLEAR(si->cmic_bitmap);

    si->cmic_block = -1;
    for (blk = 0; blk < SOC_MAX_NUM_OTPC_BLKS; blk++) {
        si->otpc_block[blk] = -1;
    }

    for (blk = 0; SOC_BLOCK_INFO(unit, blk).type >= 0; blk++) {
        blktype = SOC_BLOCK_INFO(unit, blk).type;

        if(blk >= SOC_MAX_NUM_BLKS)
        {
              DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("soc_dnxf_info_config: too much blocks for device \n")));
        }
        si->has_block[blk] = blktype;
        sal_snprintf(instance_string, sizeof(instance_string), "%d",
                     SOC_BLOCK_INFO(unit, blk).number);


        rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_drv_block_valid_get, (unit, blktype, SOC_BLOCK_INFO(unit, blk).number, &(si->block_valid[blk])));
        DNXC_IF_ERR_EXIT(rv);
        
        switch(blktype)
        {
            case SOC_BLK_ECI:
                si->eci_block = blk;
                break;
            case SOC_BLK_AVS:
                si->avs_block = blk;
                break;
            case SOC_BLK_MESH_TOPOLOGY:
                si->mesh_topology_block = blk;
                break;
            case SOC_BLK_CMIC:
                si->cmic_block = blk;
                break;
            case SOC_BLK_FMAC:
                if(SOC_BLOCK_INFO(unit, blk).number < SOC_MAX_NUM_FMAC_BLKS) {
                    si->fmac_block[SOC_BLOCK_INFO(unit, blk).number] = blk;
                    si->block_port[blk] = SOC_BLOCK_INFO(unit, blk).number | SOC_REG_ADDR_INSTANCE_MASK;
                }
                else {
                    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("soc_dnxf_info_config: too much FMAC blocks")));
                }
                break;
            case SOC_BLK_OTPC:
                if(SOC_BLOCK_INFO(unit, blk).number < SOC_MAX_NUM_OTPC_BLKS) {
                    si->otpc_block[SOC_BLOCK_INFO(unit, blk).number] = blk;
                }
                else {
                    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("soc_dnxf_info_config: too much OTPC blocks")));
                }
                break;
            case SOC_BLK_FSRD:
                if(SOC_BLOCK_INFO(unit, blk).number < SOC_MAX_NUM_FSRD_BLKS) {
                    si->fsrd_block[SOC_BLOCK_INFO(unit, blk).number] = blk;
                    si->block_port[blk] = SOC_BLOCK_INFO(unit, blk).number | SOC_REG_ADDR_INSTANCE_MASK;
                }
                else {
                    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("soc_dnxf_info_config: too much FSRD blocks")));
                }
                break;
            case SOC_BLK_RTP:
                si->rtp_block = blk;
                break;
            case SOC_BLK_OCCG:
                si->occg_block = blk;
                break;
            case SOC_BLK_DCH:
                if(SOC_BLOCK_INFO(unit, blk).number < SOC_MAX_NUM_DCH_BLKS) {
                    si->dch_block[SOC_BLOCK_INFO(unit, blk).number] = blk;
                    si->block_port[blk] = SOC_BLOCK_INFO(unit, blk).number | SOC_REG_ADDR_INSTANCE_MASK;
                }
                else {
                     DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("soc_dnxf_info_config: too much DCH blocks")));
                }
                break;
            case SOC_BLK_DCML:
                if (SOC_BLOCK_INFO(unit,blk).number < SOC_MAX_NUM_DCML_BLKS) {
                    si->dcml_block[SOC_BLOCK_INFO(unit,blk).number] = blk;
                    si->block_port[blk]= SOC_BLOCK_INFO(unit,blk).number | SOC_REG_ADDR_INSTANCE_MASK;
                }
                else {
                    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("soc_dnxf_info_config: too much DCML blocks")));
                }
                break;
            case SOC_BLK_MCT:
                si->mct_block=blk;
                break;
            case SOC_BLK_QRH:
                if (SOC_BLOCK_INFO(unit,blk).number < SOC_MAX_NUM_QRH_BLKS) {
                    si->qrh_block[SOC_BLOCK_INFO(unit,blk).number] = blk;
                    si->block_port[blk]= SOC_BLOCK_INFO(unit,blk).number | SOC_REG_ADDR_INSTANCE_MASK;
                }
                else {
                    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("soc_dnxf_info_config: too much QRH blocks")));
                }
                break;
            case SOC_BLK_CCH:
                if (SOC_BLOCK_INFO(unit,blk).number < SOC_MAX_NUM_CCH_BLKS) {
                    si->cch_block[SOC_BLOCK_INFO(unit,blk).number] = blk;
                    si->block_port[blk]= SOC_BLOCK_INFO(unit,blk).number | SOC_REG_ADDR_INSTANCE_MASK;
                }
                else {
                    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("soc_dnxf_info_config: too much CCH blocks")));
                }
                break;
            case SOC_BLK_LCM:
                if (SOC_BLOCK_INFO(unit,blk).number < SOC_MAX_NUM_LCM_BLKS) {
                    si->lcm_block[SOC_BLOCK_INFO(unit,blk).number] = blk;
                    si->block_port[blk]= SOC_BLOCK_INFO(unit,blk).number | SOC_REG_ADDR_INSTANCE_MASK;
                }
                else {
                    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("soc_dnxf_info_config: too much LCM blocks")));
                }
                break;
            case SOC_BLK_BRDC_FSRD:
                si->brdc_fsrd_block = blk;
                break;
            case SOC_BLK_BRDC_DCH:
                si->brdc_dch_block= blk;
                break;
            case SOC_BLK_BRDC_FMAC:
                si->brdc_fmac_block=blk;
                break;
            case SOC_BLK_BRDC_CCH:
                si->brdc_cch_block=blk;
                break;
            case SOC_BLK_BRDC_DCML:
                si->brdc_dcml_block=blk;
                break;
            case SOC_BLK_BRDC_LCM:
                si->brdc_lcm_block=blk;
                break;
            case SOC_BLK_BRDC_QRH:
                si->brdc_qrh_block=blk;
                break;
            default:
                DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("soc_dnxf_info_config: unknown block type")));
                break;            
        }

        sal_snprintf(si->block_name[blk], sizeof(si->block_name[blk]),
                     "%s%s",
                     soc_block_name_lookup_ext(blktype, unit),
                     instance_string);
    }
    si->block_num = blk;

    /*
     * Calculate the mem_block_any array for this configuration
     * The "any" block is just the first one enabled
     */
    for (mem = 0; mem < NUM_SOC_MEM; mem++) {
        si->mem_block_any[mem] = -1;
        if (SOC_MEM_IS_VALID(unit, mem)) {
            SOC_MEM_BLOCK_ITER(unit, mem, blk) {
                si->mem_block_any[mem] = blk;
                break;
            }
        }
    }

    for (phy_port = 0; ; phy_port++) {
        
        blk = SOC_DRIVER(unit)->port_info[phy_port].blk;
        bindex = SOC_DRIVER(unit)->port_info[phy_port].bindex;
        if (blk < 0 && bindex < 0) { /* end of list */
            break;
        }
       
        port = phy_port;
        
        if (blk < 0 ) { /* empty slot*/
            blktype = 0; 
        } else {
            blktype = SOC_BLOCK_INFO(unit, blk).type;
        }
        
        if (blktype == 0) {
            sal_snprintf(si->port_name[port], sizeof(si->port_name[port]),
                         "sfi%d", port);
            si->port_offset[port] = port;
            continue;
        }
        
        switch (blktype) {
            case SOC_BLK_CMIC:
                si->cmic_port = port;
                sal_sprintf(SOC_PORT_NAME(unit, port),"CMIC");
                SOC_PBMP_PORT_ADD(si->cmic_bitmap, port);
                break;
        default:
                si->port_num_lanes[port] = 1;
                sal_sprintf(SOC_PORT_NAME(unit, port),"sfi%d",port);
                sal_sprintf(SOC_PORT_NAME_ALTER(unit, port),"fabric%d",port);
                SOC_PORT_NAME_ALTER_VALID(unit, port) = 1;
                DNXF_ADD_PORT(sfi, port);
                DNXF_ADD_PORT(port, port);
                DNXF_ADD_PORT(all, port);
                break;
        }

        si->port_type[phy_port] = blktype;
        
    }

exit:
    DNXC_FUNC_RETURN;
}

int
soc_dnxf_detach(int unit)
{
    soc_control_t       *soc;
    int                  mem;
    int                  cmc;
    int					 rc;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("invalid unit")));
    }

    soc = SOC_CONTROL(unit);
    if (soc == NULL) {
        SOC_EXIT;
    }


    if (soc->miimMutex) {
        sal_mutex_destroy(soc->miimMutex);
        soc->miimMutex = NULL;
    }

    if (soc->counterMutex) {
        sal_mutex_destroy(soc->counterMutex);
        soc->counterMutex = NULL;
    }

    if (soc->schanMutex) {
        sal_mutex_destroy(soc->schanMutex);
        soc->schanMutex = NULL;
    }

    for (cmc = 0; cmc < SOC_PCI_CMCS_NUM(unit) + 1; cmc++) {
        if (soc->schanIntr[cmc]) {
            sal_sem_destroy(soc->schanIntr[cmc]);
            soc->schanIntr[cmc] = NULL;
        }
    }

    (void)soc_sbusdma_lock_deinit(unit);

    /*
    * Memory mutex release
    */
    for (mem = 0; mem < NUM_SOC_MEM; mem++) {
        if (SOC_MEM_IS_VALID(unit, mem)) {
            /*
             * Mutexes created only for valid memories. 
             */
            if (soc->memState[mem].lock) {
                 sal_mutex_destroy(soc->memState[mem].lock);
                 soc->memState[mem].lock = NULL;
            }
        }
    }

    if (soc->miimIntr) {
        sal_sem_destroy(soc->miimIntr);
        soc->miimIntr = NULL;
    }

    if (SOC_PERSIST(unit)) {
        sal_free(SOC_PERSIST(unit));
        SOC_PERSIST(unit) = NULL;
    }

    if (soc->socControlMutex) {
        sal_mutex_destroy(soc->socControlMutex);
        soc->socControlMutex = NULL;
    }

    if (_bcm_lock[unit] != NULL) {
        sal_mutex_destroy(_bcm_lock[unit]);
        _bcm_lock[unit] = NULL;
    }
    if (soc->schan_wb_mutex != NULL) {
        sal_mutex_destroy(soc->schan_wb_mutex);
        soc->schan_wb_mutex = NULL;
    }



    rc = soc_dnxf_implementation_defines_deinit(unit);
    DNXC_IF_ERR_CONT(rc);

    rc = soc_dnxf_defines_deinit(unit);
    DNXC_IF_ERR_CONT(rc);

    if (SOC_CONTROL(unit)->drv != NULL)
    {
    sal_free((soc_dnxf_control_t *)SOC_CONTROL(unit)->drv);
    SOC_CONTROL(unit)->drv = NULL;
    }

    sal_free(soc);
    SOC_CONTROL(unit) = NULL;

exit:
    DNXC_FUNC_RETURN;
}

int
soc_dnxf_chip_type_set(int unit, uint16 dev_id)
{
    soc_info_t           *si; 
    DNXC_INIT_FUNC_DEFS;

    si  = &SOC_INFO(unit);

    /*
     * Used to implement the SOC_IS_*(unit) macros
     */
    switch (dev_id) {
        case BCM88790_DEVICE_ID:
            si->chip_type = SOC_INFO_CHIP_TYPE_RAMON;
            break;

        default:
            si->chip_type = 0;
           LOG_ERROR(BSL_LS_SOC_INIT,
                     (BSL_META_U(unit,
                                 "soc_dnxf_chip_type_set: driver device %04x unexpected\n"),
                                 dev_id));
            DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("failed to find device id")));
    }

exit:  
    DNXC_FUNC_RETURN;
}

int
soc_dnxf_attach(int unit)
{
    soc_control_t        *soc;
    soc_persist_t        *sop;
    soc_info_t           *si; 
    uint16               dev_id;
    uint8                rev_id;
    int                  rc = SOC_E_NONE, mem;
    int                  cmc;
    DNXC_INIT_FUNC_DEFS;

    /* Allocate soc_control and soc_persist if not already. */
    soc = SOC_CONTROL(unit);
    if (soc == NULL) {
        soc = sal_alloc(sizeof (soc_control_t), "soc_control");
        if (soc == NULL) {
            DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("failed to allocate soc_control")));
        }
        sal_memset(soc, 0, sizeof (soc_control_t));
        SOC_CONTROL(unit) = soc;
    } else {
        if (soc->soc_flags & SOC_F_ATTACHED) {
            SOC_EXIT;
        }
    }

    soc->soc_link_pause = 0;

    SOC_PCI_CMCS_NUM(unit) = soc_property_uc_get(unit, 0, spn_PCI_CMCS_NUM, 1);

	#ifdef BCM_SBUSDMA_SUPPORT
    if (!SOC_IS_FE1600(unit)) {
        SOC_CONTROL(unit)->max_sbusdma_channels = SOC_DNXC_MAX_SBUSDMA_CHANNELS;
        SOC_CONTROL(unit)->tdma_ch              = SOC_DNXC_TDMA_CHANNEL;
        SOC_CONTROL(unit)->tslam_ch             = SOC_DNXC_TSLAM_CHANNEL;
        SOC_CONTROL(unit)->desc_ch              = SOC_DNXC_DESC_CHANNEL;
        /* maximum possible memory entry size used for clearing memory, should be a multiple of 32bit words, */
        SOC_MEM_CLEAR_CHUNK_SIZE_SET(unit, 
            soc_property_get(unit, spn_MEM_CLEAR_CHUNK_SIZE, SOC_DNXC_MEM_CLEAR_CHUNK_SIZE));
    }
	#endif


    /* Setup DMA structures when a device is attached */
    DNXC_IF_ERR_EXIT(soc_dma_attach(unit, 1 /* Reset */));

    /* Init cmic_pcie_userif_purge_ctrl */
    DNXC_IF_ERR_EXIT(soc_dnxc_cmic_pcie_userif_purge_ctrl_init(unit));

    /*
     * Create mutexes and semaphores.
     */

    if ((soc->miimMutex = sal_mutex_create("MIIM")) == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("Failed to allocate MIIM lock")));
    }
        
    if ((soc->miimIntr = sal_sem_create("MIIM interrupt", sal_sem_BINARY, 0)) == NULL)  {
         DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("Failed to allocate MIIM semaphore")));
    }

    if (_bcm_lock[unit] == NULL) {
        if ((_bcm_lock[unit] = sal_mutex_create("bcm_dnxf_config_lock")) == NULL) {
            DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("Failed to allocate _bcm_lock")));
        }
    }

    if (_bcm_lock[unit] == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("Failed to allocate DNXF lock")));
    }

    if ((soc->socControlMutex = sal_mutex_create("SOC_CONTROL")) == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("failed to allocate socControlMutex")));
    }

    soc->counterMutex = sal_mutex_create("Counter");
    if (soc->counterMutex == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("failed to allocate counterMutex")));
    }

    soc->schanMutex = sal_mutex_create("SCHAN");
    if (soc->schanMutex == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("failed to allocate schanMutex")));
    }

    for (cmc = 0; cmc < SOC_PCI_CMCS_NUM(unit) + 1; cmc++) {
        if ((soc->schanIntr[cmc] =
             sal_sem_create("SCHAN interrupt", sal_sem_BINARY, 0)) == NULL) {
             DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("failed to allocate schanSem")));
        }
    }
    
    SOC_PERSIST(unit) = sal_alloc(sizeof (soc_persist_t), "soc_persist");
    if (SOC_PERSIST(unit) == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("failed to allocate soc_persist")));
    }
    sal_memset(SOC_PERSIST(unit), 0, sizeof (soc_persist_t));
    sop = SOC_PERSIST(unit);
    sop->version = 1;

    soc_cm_get_id(unit, &dev_id, &rev_id);
    DNXC_IF_ERR_EXIT(soc_dnxf_chip_type_set(unit, dev_id));

    /* Instantiate the driver -- Verify chip revision matches driver
     * compilation revision.
     */
    soc->chip_driver = soc_dnxf_chip_driver_find(unit, dev_id, rev_id);
    if (soc->chip_driver == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("unit has no driver (device 0x%04x rev 0x%02x)"),dev_id, rev_id));
    }

    /*feature init*/
    soc_feature_init(unit);

    si  = &SOC_INFO(unit);
    si->driver_type = soc->chip_driver->type;
    si->driver_group = soc_chip_type_map[si->driver_type];

    
    /* Must call mbcm init first to ensure driver properly installed */
    rc = mbcm_dnxf_init(unit);
    if (rc != SOC_E_NONE) {
         LOG_INFO(BSL_LS_SOC_INIT,
                  (BSL_META_U(unit,
                              "soc_dnxf_init error in mbcm_dnxf_init\n")));
    }
    DNXC_IF_ERR_EXIT(rc);    


    /* allocate counter module resources */


    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_controlled_counter_set,(unit));
    DNXC_IF_ERR_EXIT(rc);


    /*dnxf info config*/
    /*should be at the attach function to enable register access without chip init*/
    rc = soc_dnxf_control_init(unit);
    DNXC_IF_ERR_EXIT(rc);

    /*Required In order to read soc properties*/
    soc->soc_flags |= SOC_F_ATTACHED;

    /*
     * DMA
     */

    /* Initialize TABLEDMA/ SLAMDMA /SBUSDMA Locks */
    rc = soc_sbusdma_lock_init(unit);
    if (rc != SOC_E_NONE) {
        DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("failed to Initialize SBUSDMA Locks")));
    }
#ifdef BCM_SBUSDMA_SUPPORT
    /* Initialize SBSUDMA */
    if (soc_feature(unit, soc_feature_sbusdma)) {
        rc = soc_sbusdma_init(unit, soc_property_get(unit, spn_DMA_DESC_TIMEOUT_USEC, 0),
                                  soc_property_get(unit, spn_DMA_DESC_INTR_ENABLE, 0));
        if (rc != SOC_E_NONE) {
            DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("failed to Initialize SBUSDMA")));
        }
    }
#endif

    /*
     * Initialize memory index_maxes. Chip specific overrides follow.
     */
    for (mem = 0; mem < NUM_SOC_MEM; mem++) {
        if (SOC_MEM_IS_VALID(unit, mem)) {
            sop->memState[mem].index_max = SOC_MEM_INFO(unit, mem).index_max;
            /*
             * should only create mutexes for valid memories. 
             */
            if ((soc->memState[mem].lock =
                 sal_mutex_create(SOC_MEM_NAME(unit, mem))) == NULL) {
                DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("failed to allocate memState lock")));
            }
    
            /* Set cache copy pointers to NULL */
            sal_memset(soc->memState[mem].cache,
                       0,
                       sizeof (soc->memState[mem].cache));
        } else {
            sop->memState[mem].index_max = -1;
        }
    }

    if ((soc->schan_wb_mutex = sal_mutex_create("SchanWB")) == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("Failed to allocate SchanWB")));
    }

    /* Initialize SCHAN */
    rc = soc_schan_init(unit);
    if (rc != SOC_E_NONE) {
       DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("failed to Initialize SCHAN")));
    }

exit:  
    if(DNXC_FUNC_ERROR) {
       LOG_ERROR(BSL_LS_SOC_INIT,
                 (BSL_META_U(unit,
                             "soc_dnxf_attach: unit %d failed (%s)\n"),
                             unit, soc_errmsg(rc)));
        soc_dnxf_detach(unit);
    }

    DNXC_FUNC_RETURN;
}

int
soc_dnxf_dump(int unit, char *pfx)
{
    soc_control_t       *soc;
    soc_persist_t       *sop;
    soc_stat_t          *stat;
    uint16              dev_id;
    uint8               rev_id;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("invalid unit")));
    }

    soc = SOC_CONTROL(unit);
    sop = SOC_PERSIST(unit);

    stat = &soc->stat;

    LOG_CLI((BSL_META_U(unit,
                        "%sUnit %d Driver Control Structure:\n"), pfx, unit));

    soc_cm_get_id(unit, &dev_id, &rev_id);

    LOG_CLI((BSL_META_U(unit,
                        "%sChip=%s Rev=0x%02x Driver=%s\n"),
             pfx,
             soc_dev_name(unit),
             rev_id,
             SOC_CHIP_NAME(soc->chip_driver->type)));
    LOG_CLI((BSL_META_U(unit,
                        "%sFlags=0x%x:"),
             pfx, soc->soc_flags));
    if (soc->soc_flags & SOC_F_ATTACHED) {
        LOG_CLI((BSL_META_U(unit,
                            " attached")));
    }
    if (soc->soc_flags & SOC_F_INITED) {
        LOG_CLI((BSL_META_U(unit,
                            " initialized")));
    }
    if (soc->soc_flags & SOC_F_LSE) {
        LOG_CLI((BSL_META_U(unit,
                            " link-scan")));
    }
    if (soc->soc_flags & SOC_F_SL_MODE) {
        LOG_CLI((BSL_META_U(unit,
                            " sl-mode")));
    }
    if (soc->soc_flags & SOC_F_POLLED) {
        LOG_CLI((BSL_META_U(unit,
                            " polled")));
    }
    if (soc->soc_flags & SOC_F_URPF_ENABLED) {
        LOG_CLI((BSL_META_U(unit,
                            " urpf")));
    }
    if (soc->soc_flags & SOC_F_MEM_CLEAR_USE_DMA) {
        LOG_CLI((BSL_META_U(unit,
                            " mem-clear-use-dma")));
    }
    if (soc->soc_flags & SOC_F_IPMCREPLSHR) {
        LOG_CLI((BSL_META_U(unit,
                            " ipmc-repl-shared")));
    }
    if (soc->remote_cpu) {
        LOG_CLI((BSL_META_U(unit,
                            " rcpu")));
    }
    LOG_CLI((BSL_META_U(unit,
                        "; board type 0x%x"), soc->board_type));
    LOG_CLI((BSL_META_U(unit,
                        "\n")));
    LOG_CLI((BSL_META_U(unit,
                        "%s"), pfx));
    soc_cm_dump(unit);

    LOG_CLI((BSL_META_U(unit,
                        "%sDisabled: reg_flags=0x%x mem_flags=0x%x\n"),
             pfx,
             soc->disabled_reg_flags, soc->disabled_mem_flags));
    LOG_CLI((BSL_META_U(unit,
                        "%sSchanOps=%d MMUdbg=%d LinkPause=%d\n"),
             pfx,
             stat->schan_op,
             sop->debugMode, soc->soc_link_pause));
    LOG_CLI((BSL_META_U(unit,
                        "%sCounter: int=%dus per=%dus dmaBuf=%p\n"),
             pfx,
             soc->counter_interval,
             SAL_USECS_SUB(soc->counter_coll_cur, soc->counter_coll_prev),
             (void *)soc->counter_buf32));
    LOG_CLI((BSL_META_U(unit,
                        "%sTimeout: Schan=%d(%dus) MIIM=%d(%dus)\n"),
             pfx,
             stat->err_sc_tmo, soc->schanTimeout,
             stat->err_mii_tmo, soc->miimTimeout));
    LOG_CLI((BSL_META_U(unit,
                        "%sIntr: Total=%d Sc=%d ScErr=%d MMU/ARLErr=%d\n"
             "%s      LinkStat=%d PCIfatal=%d PCIparity=%d\n"
                        "%s      ARLdrop=%d ARLmbuf=%d ARLxfer=%d ARLcnt0=%d\n"
                        "%s      TableDMA=%d TSLAM-DMA=%d\n"
                        "%s      MemCmd[BSE]=%d MemCmd[CSE]=%d MemCmd[HSE]=%d\n"
                        "%s      ChipFunc[0]=%d ChipFunc[1]=%d ChipFunc[2]=%d\n"
                        "%s      ChipFunc[3]=%d ChipFunc[4]=%d\n"
                        "%s      I2C=%d MII=%d StatsDMA=%d Desc=%d Chain=%d\n"),
             pfx, stat->intr, stat->intr_sc, stat->intr_sce, stat->intr_mmu,
             pfx, stat->intr_ls,
             stat->intr_pci_fe, stat->intr_pci_pe,
             pfx, stat->intr_arl_d, stat->intr_arl_m,
             stat->intr_arl_x, stat->intr_arl_0,
             pfx, stat->intr_tdma, stat->intr_tslam,
             pfx, stat->intr_mem_cmd[0],
             stat->intr_mem_cmd[1], stat->intr_mem_cmd[2],
             pfx, stat->intr_chip_func[0], stat->intr_chip_func[1],
             stat->intr_chip_func[2],
             pfx, stat->intr_chip_func[3], stat->intr_chip_func[4],
             pfx, stat->intr_i2c, stat->intr_mii, stat->intr_stats,
             stat->intr_desc, stat->intr_chain));

        soc_dnxf_drv_soc_properties_dump(unit);
   
exit:
    DNXC_FUNC_RETURN;
} 
  
void 
soc_dnxf_drv_soc_properties_dump(int unit)
{
    int i;
    int lcpll;
    DNXC_INIT_FUNC_DEFS;

    LOG_CLI((BSL_META_U(unit,
                        "\nSoC properties dump\n")));
    LOG_CLI((BSL_META_U(unit,
                        "-------------------\n")));

    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_STR_DUMP(unit, spn_FABRIC_DEVICE_MODE, soc_dnxf_property_str_enum_fabric_device_mode, SOC_DNXF_CONFIG(unit).fabric_device_mode);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_STR_DUMP(unit, spn_FABRIC_MULTICAST_MODE, soc_dnxf_property_str_enum_fabric_multicast_mode, SOC_DNXF_CONFIG(unit).fabric_multicast_mode);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_STR_DUMP(unit, spn_FABRIC_LOAD_BALANCING_MODE, soc_dnxf_property_str_enum_fabric_load_balancing_mode, SOC_DNXF_CONFIG(unit).fabric_load_balancing_mode);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_STR_DUMP(unit, spn_FE_MC_ID_RANGE, soc_dnxf_property_str_enum_fe_mc_id_range, SOC_DNXF_CONFIG(unit).fe_mc_id_range);


    LOG_CLI((BSL_META_U(unit,
                        "\n")));
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_IS_DUAL_MODE, SOC_DNXF_CONFIG(unit).is_dual_mode);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_SYSTEM_IS_VCS_128_IN_SYSTEM, SOC_DNXF_CONFIG(unit).system_is_vcs_128_in_system);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_SYSTEM_IS_DUAL_MODE_IN_SYSTEM, SOC_DNXF_CONFIG(unit).system_is_dual_mode_in_system);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_SYSTEM_IS_SINGLE_MODE_IN_SYSTEM, SOC_DNXF_CONFIG(unit).system_is_single_mode_in_system);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_SYSTEM_CONTAINS_MULTIPLE_PIPE_DEVICE, SOC_DNXF_CONFIG(unit).system_contains_multiple_pipe_device);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_SYSTEM_IS_FE600_IN_SYSTEM, SOC_DNXF_CONFIG(unit).system_is_fe600_in_system);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_FABRIC_MERGE_CELLS, SOC_DNXF_CONFIG(unit).fabric_merge_cells);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_FABRIC_TDM_FRAGMENT, SOC_DNXF_CONFIG(unit).fabric_TDM_fragment);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_STR_DUMP(unit, spn_FABRIC_TDM_PRIORITY_MIN, soc_dnxf_property_str_enum_fabric_tdm_priority_min, SOC_DNXF_CONFIG(unit).fabric_tdm_priority_min);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_VCS128_UNICAST_PRIORITY, SOC_DNXF_CONFIG(unit).vcs128_unicast_priority);

    
    LOG_CLI((BSL_META_U(unit,
                        "\n")));
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_FABRIC_MAC_BUCKET_FILL_RATE, SOC_DNXF_CONFIG(unit).fabric_mac_bucket_fill_rate);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_SERDES_MIXED_RATE_ENABLE, SOC_DNXF_CONFIG(unit).serdes_mixed_rate_enable);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_FABRIC_OPTIMIZE_PARTIAL_LINKS, SOC_DNXF_CONFIG(unit).fabric_optimize_patial_links);

    LOG_CLI((BSL_META_U(unit,
                        "\n")));

    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_FE_MC_PRIORITY_MAP_ENABLE, SOC_DNXF_CONFIG(unit).fe_mc_priority_map_enable);

    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_FABRIC_TDM_OVER_PRIMARY_PIPE, SOC_DNXF_CONFIG(unit).fabric_TDM_over_primary_pipe);

    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_FABRIC_NUM_PIPES, SOC_DNXF_FABRIC_PIPES_CONFIG(unit).nof_pipes);
    for (i=0;i<SOC_DNXF_MAX_NUM_OF_PRIORITIES ;i++)
    {
        SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_SUFFIX_NUM_INT_DUMP(unit, spn_FABRIC_PIPE_MAP, "uc", i,SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_uc[i]);
    }
    for (i=0;i<SOC_DNXF_MAX_NUM_OF_PRIORITIES;i++)
    {
        SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_SUFFIX_NUM_INT_DUMP(unit, spn_FABRIC_PIPE_MAP, "mc", i,SOC_DNXF_FABRIC_PIPES_CONFIG(unit).config_mc[i]);
    }

    LOG_CLI((BSL_META_U(unit,
                        "\n")));
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_SYSTEM_REF_CORE_CLOCK, SOC_DNXF_CONFIG(unit).system_ref_core_clock);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_CORE_CLOCK_SPEED, SOC_DNXF_CONFIG(unit).core_clock_speed);
    for (lcpll = 0; lcpll < SOC_DNXF_NOF_LCPLL; lcpll++)
    {
        SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_SUFFIX_NUM_INT_DUMP(unit, spn_SERDES_FABRIC_CLK_FREQ, "in", lcpll ,SOC_DNXF_CONFIG(unit).fabric_port_lcpll_in[lcpll]);
    }

    for (lcpll = 0; lcpll < SOC_DNXF_NOF_LCPLL; lcpll++)
    {
        SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_SUFFIX_NUM_INT_DUMP(unit, spn_SERDES_FABRIC_CLK_FREQ, "out", lcpll ,SOC_DNXF_CONFIG(unit).fabric_port_lcpll_out[lcpll]);
    }
    
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_BIST_ENABLE, SOC_DNXF_CONFIG(unit).run_mbist);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_SUFFIX_INT_DUMP(unit, spn_CUSTOM_FEATURE, "lab", SOC_DNXF_CONFIG(unit).custom_feature_lab);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_SUFFIX_INT_DUMP(unit, spn_CUSTOM_FEATURE, "mesh_topology_fast", SOC_DNXF_CONFIG(unit).mesh_topology_fast);

    LOG_CLI((BSL_META_U(unit,
                        "\n")));
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_SCHAN_TIMEOUT_USEC, SOC_CONTROL(unit)->schanTimeout);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_SCHAN_INTR_ENABLE, SOC_CONTROL(unit)->schanIntrEnb);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_MIIM_TIMEOUT_USEC, SOC_CONTROL(unit)->miimTimeout);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_MIIM_INTR_ENABLE, SOC_CONTROL(unit)->miimIntrEnb);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_RATE_INT_MDIO_DIVIDEND, SOC_DNXF_CONFIG(unit).mdio_int_dividend);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_RATE_INT_MDIO_DIVISOR, SOC_DNXF_CONFIG(unit).mdio_int_divisor);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_TDMA_TIMEOUT_USEC, SOC_CONTROL(unit)->tableDmaTimeout);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_TDMA_INTR_ENABLE, SOC_CONTROL(unit)->tableDmaIntrEnb);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_TSLAM_TIMEOUT_USEC, SOC_CONTROL(unit)->tslamDmaTimeout);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_TSLAM_INTR_ENABLE, SOC_CONTROL(unit)->tslamDmaIntrEnb);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_FABRIC_CELL_FIFO_DMA_ENABLE, SOC_DNXF_CONFIG(unit).fabric_cell_fifo_dma_enable);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_FABRIC_CELL_FIFO_DMA_BUFFER_SIZE, SOC_DNXF_CONFIG(unit).fabric_cell_fifo_dma_buffer_size);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_FABRIC_CELL_FIFO_DMA_TIMEOUT, SOC_DNXF_CONFIG(unit).fabric_cell_fifo_dma_timeout);
    SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_FABRIC_CELL_FIFO_DMA_THRESHOLD, SOC_DNXF_CONFIG(unit).fabric_cell_fifo_dma_threshold);

    if (SOC_DNXF_IS_FE13(unit))
    {
        SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_INT_DUMP(unit, spn_FABRIC_LOCAL_ROUTING_ENABLE, SOC_DNXF_CONFIG(unit).fabric_local_routing_enable);
    }

    if (SOC_DNXF_IS_FE13_ASYMMETRIC(unit) || SOC_DNXF_IS_FE2(unit))
    {
        SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_SUFFIX_NUM_INT_DUMP(unit, spn_SERDES_FABRIC_CLK_FREQ, "in_quad", 26, SOC_DNXF_CONFIG(unit).fabric_clk_freq_in_quad_26);
        SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_SUFFIX_NUM_INT_DUMP(unit, spn_SERDES_FABRIC_CLK_FREQ, "in_quad", 35, SOC_DNXF_CONFIG(unit).fabric_clk_freq_in_quad_35);
        SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_SUFFIX_NUM_INT_DUMP(unit, spn_SERDES_FABRIC_CLK_FREQ, "out_quad", 26, SOC_DNXF_CONFIG(unit).fabric_clk_freq_out_quad_26);
        SOC_DNXF_DRV_SUPPORTED_SOC_PROPERTY_SUFFIX_NUM_INT_DUMP(unit, spn_SERDES_FABRIC_CLK_FREQ, "out_quad", 35, SOC_DNXF_CONFIG(unit).fabric_clk_freq_out_quad_35);
    }

    LOG_CLI((BSL_META_U(unit,
                        "-------------------\n")));

    DNXC_FUNC_RETURN_VOID;
}

void
soc_dnxf_chip_dump(int unit, soc_driver_t *d)
{
    soc_info_t          *si;
    int                 i, count = 0;
    soc_port_t          port;
    char                pfmt[SOC_PBMP_FMT_LEN];
    uint16              dev_id;
    uint8               rev_id;
    int                 blk, bindex;
    char                instance_string[3], block_name[14];

    if (d == NULL) {
        LOG_CLI((BSL_META_U(unit,
                            "unit %d: no driver attached\n"), unit));
        return;
    }

    LOG_CLI((BSL_META_U(unit,
                        "driver %s (%s)\n"), SOC_CHIP_NAME(d->type), d->chip_string));
    LOG_CLI((BSL_META_U(unit,
                        "\tregsfile\t\t%s\n"), d->origin));
    LOG_CLI((BSL_META_U(unit,
                        "\tpci identifier\t\tvendor 0x%04x device 0x%04x rev 0x%02x\n"),
             d->pci_vendor, d->pci_device, d->pci_revision));
    LOG_CLI((BSL_META_U(unit,
                        "\tclasses of service\t%d\n"), d->num_cos));
    LOG_CLI((BSL_META_U(unit,
                        "\tmaximums\t\tblock %d ports %d mem_bytes %d\n"),
             SOC_MAX_NUM_BLKS, SOC_MAX_NUM_PORTS, SOC_MAX_MEM_BYTES));

    if (unit < 0) {
        return;
    }
    si = &SOC_INFO(unit);
    for (blk = 0; d->block_info[blk].type >= 0; blk++) {
        sal_snprintf(instance_string, sizeof(instance_string), "%d",
                     d->block_info[blk].number);
        if (d->block_info[blk].type == SOC_BLK_PORT_GROUP4 ||
            d->block_info[blk].type == SOC_BLK_PORT_GROUP5) {
            sal_strncpy(instance_string,
                        d->block_info[blk].number ? "_y" : "_x", 2);
            instance_string[2] = '\0';
        }
        sal_snprintf(block_name, sizeof(block_name), "%s%s",
                     soc_block_name_lookup_ext(d->block_info[blk].type, unit),
                     instance_string);
        LOG_CLI((BSL_META_U(unit,
                            "\tblk %d\t\t%-14s schan %d cmic %d\n"),
                 blk,
                 block_name,
                 d->block_info[blk].schan,
                 d->block_info[blk].cmic));
    }
    for (port = 0; ; port++) {
          blk = d->port_info[port].blk;
          bindex = d->port_info[port].bindex;
          if (blk < 0 && bindex < 0) {            /* end of list */
              break;
          }
          if (blk < 0) {                          /* empty slot */
              continue;
          }
          LOG_CLI((BSL_META_U(unit,
                              "\tport %d\t\t%s\tblk %d %s%d.%d\n"),
                   soc_feature(unit, soc_feature_logical_port_num) ?
                   si->port_p2l_mapping[port] : port,
                   soc_block_port_name_lookup_ext(d->block_info[blk].type, unit),
                   blk,
                   soc_block_name_lookup_ext(d->block_info[blk].type, unit),
                   d->block_info[blk].number,
                   bindex));
    }

    soc_cm_get_id(unit, &dev_id, &rev_id);
    LOG_CLI((BSL_META_U(unit,
                        "unit %d:\n"), unit));
    LOG_CLI((BSL_META_U(unit,
                        "\tpci\t\t\tdevice %04x rev %02x\n"), dev_id, rev_id));
    LOG_CLI((BSL_META_U(unit,
                        "\tdriver\t\t\ttype %d (%s) group %d (%s)\n"),
             si->driver_type, SOC_CHIP_NAME(si->driver_type),
             si->driver_group, soc_chip_group_names[si->driver_group]));
    LOG_CLI((BSL_META_U(unit,
                        "\tchip\t\t\t\n")));
    LOG_CLI((BSL_META_U(unit,
                        "\tGE ports\t%d\t%s (%d:%d)\n"),
             si->ge.num, SOC_PBMP_FMT(si->ge.bitmap, pfmt),
             si->ge.min, si->ge.max));
    LOG_CLI((BSL_META_U(unit,
                        "\tXE ports\t%d\t%s (%d:%d)\n"),
             si->xe.num, SOC_PBMP_FMT(si->xe.bitmap, pfmt),
             si->xe.min, si->xe.max));
    LOG_CLI((BSL_META_U(unit,
                        "\tHG ports\t%d\t%s (%d:%d)\n"),
             si->hg.num, SOC_PBMP_FMT(si->hg.bitmap, pfmt),
             si->hg.min, si->hg.max));
    LOG_CLI((BSL_META_U(unit,
                        "\tST ports\t%d\t%s (%d:%d)\n"),
             si->st.num, SOC_PBMP_FMT(si->st.bitmap, pfmt),
             si->st.min, si->st.max));
    LOG_CLI((BSL_META_U(unit,
                        "\tETHER ports\t%d\t%s (%d:%d)\n"),
             si->ether.num, SOC_PBMP_FMT(si->ether.bitmap, pfmt),
             si->ether.min, si->ether.max));
    LOG_CLI((BSL_META_U(unit,
                        "\tPORT ports\t%d\t%s (%d:%d)\n"),
             si->port.num, SOC_PBMP_FMT(si->port.bitmap, pfmt),
             si->port.min, si->port.max));
    LOG_CLI((BSL_META_U(unit,
                        "\tALL ports\t%d\t%s (%d:%d)\n"),
             si->all.num, SOC_PBMP_FMT(si->all.bitmap, pfmt),
             si->all.min, si->all.max));
    LOG_CLI((BSL_META_U(unit,
                        "\tIPIC port\t%d\tblock %d\n"), si->ipic_port, si->ipic_block));
    LOG_CLI((BSL_META_U(unit,
                        "\tCMIC port\t%d\t%s block %d\n"), si->cmic_port,
             SOC_PBMP_FMT(si->cmic_bitmap, pfmt), si->cmic_block));
    LOG_CLI((BSL_META_U(unit,
                        "\tother blocks\t\tARL %d MMU %d MCU %d\n"),
             si->arl_block, si->mmu_block, si->mcu_block));
    LOG_CLI((BSL_META_U(unit,
                        "\t            \t\tIPIPE %d IPIPE_HI %d EPIPE %d EPIPE_HI %d BSAFE %d ESM %d OTPC %d\n"),
             si->ipipe_block, si->ipipe_hi_block,
             si->epipe_block, si->epipe_hi_block, si->bsafe_block, si->esm_block, si->otpc_block[0]));

    for (i = 0; i < COUNTOF(si->has_block); i++) {
        if (si->has_block[i]) {
            count++;
        }
    }
    LOG_CLI((BSL_META_U(unit,
                        "\thas blocks\t%d\t"), count));
    for (i = 0; i < COUNTOF(si->has_block); i++) {
        if (si->has_block[i]) {
            LOG_CLI((BSL_META_U(unit,
                                "%s "), soc_block_name_lookup_ext(si->has_block[i], unit)));
            if ((i) && !(i%6)) {
                LOG_CLI((BSL_META_U(unit,
                                    "\n\t\t\t\t")));
            }
        }
    }

    LOG_CLI((BSL_META_U(unit,
                        "\n")));
    LOG_CLI((BSL_META_U(unit,
                        "\tport names\t\t")));
    for (port = 0; port < si->port_num; port++) {
        if (port > 0 && (port % 5) == 0) {
            LOG_CLI((BSL_META_U(unit,
                                "\n\t\t\t\t")));
        }
        LOG_CLI((BSL_META_U(unit,
                            "%d=%s\t"),
                 port, si->port_name[port]));
    }
    LOG_CLI((BSL_META_U(unit,
                        "\n")));
    i = 0;
    for (blk = 0; SOC_BLOCK_INFO(unit, blk).type >= 0; blk++) {
        if (SOC_PBMP_IS_NULL(si->block_bitmap[blk])) {
            continue;
        }
        if (++i == 1) {
            LOG_CLI((BSL_META_U(unit,
                                "\tblock bitmap\t")));
        } else {
            LOG_CLI((BSL_META_U(unit,
                                "\n\t\t\t")));
        }
        LOG_CLI((BSL_META_U(unit,
                            "%-2d  %-14s %s (%d ports)"),
                 blk,
                 si->block_name[blk],
                 SOC_PBMP_FMT(si->block_bitmap[blk], pfmt),
                 si->block_valid[blk]));
    }
    if (i > 0) {
        LOG_CLI((BSL_META_U(unit,
                            "\n")));
    }

    {
        soc_feature_t f;

        LOG_CLI((BSL_META_U(unit,
                            "\tfeatures\t")));
        i = 0;
        for (f = 0; f < soc_feature_count; f++) {
            if (soc_feature(unit, f)) {
                if (++i > 3) {
                    LOG_CLI((BSL_META_U(unit,
                                        "\n\t\t\t")));
                    i = 1;
                }
                LOG_CLI((BSL_META_U(unit,
                                    "%s "), soc_feature_name[f]));
            }
        }
        LOG_CLI((BSL_META_U(unit,
                            "\n")));
    }
}

int
soc_dnxf_nof_interrupts(int unit, int* nof_interrupts) 
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("invalid unit")));
    }

    DNXC_NULL_CHECK(nof_interrupts);

    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_nof_interrupts,(unit, nof_interrupts));
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXC_FUNC_RETURN;
}

int
soc_dnxf_nof_block_instances(int unit,  soc_block_types_t block_types, int *nof_block_instances) 
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("invalid unit")));
    }

    DNXC_NULL_CHECK(nof_block_instances);

    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_nof_block_instance,(unit, block_types, nof_block_instances));
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXC_FUNC_RETURN;
}

/*
 * soc_dnxf_deinit - 
 * This function main role is to resotore the status to be identical to the status after soc_dnxf_attach. 
 * Important! - update deinit function each time soc_dnxf_attach is updated. 
 */
int
soc_dnxf_deinit(int unit)
{
    int rc;
    soc_control_t* soc;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_UNIT_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("invalid unit")));
    }

    DNXF_UNIT_LOCK_TAKE_DNXC(unit); 


    soc = SOC_CONTROL(unit);

    soc->soc_flags &= ~SOC_F_INITED;

    if(SOC_FAILURE(soc_linkctrl_deinit(unit))){
        LOG_ERROR(BSL_LS_SOC_INIT,
                  (BSL_META_U(unit,
                              "Failed in soc_linkctrl_deinit\n")));
    }
    /*warmboot deinit*/
    rc = soc_dnxf_warm_boot_deinit(unit);
    DNXC_IF_ERR_CONT(rc);

    /*Interrupts deinit*/
    rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_interrupts_deinit, (unit));
    DNXC_IF_ERR_CONT(rc);

    /*counters detach*/
    rc = soc_counter_detach(unit);
    if(SOC_FAILURE(rc)) {
        LOG_ERROR(BSL_LS_SOC_INIT,
                  (BSL_META_U(unit,
                              "Failed to detach counter\n")));
    }


    /*dnxf info config*/
    /*prepare config info for the next init sequnace*/
    rc = soc_dnxf_control_init(unit);
    DNXC_IF_ERR_CONT(rc);
    
exit:

    DNXF_UNIT_LOCK_RELEASE_DNXC(unit); 

    /*inform that detaching device is done*/
    if(SOC_UNIT_NUM_VALID(unit)) {
        SOC_DETACH(unit,0);
    }


    DNXC_FUNC_RETURN;
}

int 
soc_dnxf_avs_value_get(int unit, uint32* avs_val)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;
    
    if (!SOC_UNIT_VALID(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("invalid unit")));
    }
    
    DNXC_NULL_CHECK(avs_val);
    
    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_avs_value_get,(unit, avs_val));
    DNXC_IF_ERR_EXIT(rc);
    
exit:
    DNXC_FUNC_RETURN;    
}

int soc_dnxf_compiler_64_div_32(uint64 x, uint32 y, uint32 *result)
{
    uint64 rem;
    uint64 b;
    uint64 res, d;
    uint32 high;

    COMPILER_64_SET(rem, COMPILER_64_HI(x), COMPILER_64_LO(x));
    COMPILER_64_SET(b, 0, y);
    COMPILER_64_SET(d, 0, 1);

    high = COMPILER_64_HI(rem);

    COMPILER_64_ZERO(res);
    if (high >= y) {
        /* NOTE: Follow code is used to handle 64bits result
         *  high /= y;
         *  res = (uint64_t) (high << 32);
         *  rem -= (uint64_t)((high * y) << 32);
         */
       LOG_ERROR(BSL_LS_SOC_INIT,
                 (BSL_META("soc_dnxf_compiler_64_div_32: result > 32bits\n")));
        return SOC_E_PARAM;
    }

    while ((!COMPILER_64_BITTEST(b, 63)) &&
       (COMPILER_64_LT(b, rem)) ) {
        COMPILER_64_ADD_64(b,b);
        COMPILER_64_ADD_64(d,d);
    }

    do {
    if (COMPILER_64_GE(rem, b)) {
        COMPILER_64_SUB_64(rem, b);
        COMPILER_64_ADD_64(res, d);
    }
        COMPILER_64_SHR(b, 1);
        COMPILER_64_SHR(d, 1);
    } while (!COMPILER_64_IS_ZERO(d));

    *result = COMPILER_64_LO(res);

    return SOC_E_NONE;
}

/*
 * Memory bist test
 */
int
soc_dnxf_drv_mbist(int unit, int skip_errors)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    rc = MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_drv_mbist,(unit, skip_errors));
    DNXC_IF_ERR_EXIT(rc);
        
exit:
    DNXC_FUNC_RETURN;   
}

int soc_dnxf_device_reset(
    int unit,
    int mode,
    int action)
{
    int rv = SOC_E_NONE;

    DNXC_INIT_FUNC_DEFS;

    switch (mode) {
    case SOC_DNXC_RESET_MODE_BLOCKS_RESET:
        rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_drv_blocks_reset, (unit, action , NULL));
        DNXC_IF_ERR_EXIT(rv);
        break;
    case SOC_DNXC_RESET_MODE_BLOCKS_SOFT_RESET:
        rv = soc_dnxf_init(unit, 0);
        DNXC_IF_ERR_EXIT(rv);
        break;
    case SOC_DNXC_RESET_MODE_INIT_RESET:
        rv = soc_dnxf_init(unit, 1);
        DNXC_IF_ERR_EXIT(rv);
        break;
    case SOC_DNXC_RESET_MODE_REG_ACCESS:
        /*read soc properties*/
        rv = soc_dnxf_info_soc_properties(unit);
        DNXC_IF_ERR_EXIT(rv);
        rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_drv_reg_access_only_reset, (unit));
        DNXC_IF_ERR_EXIT(rv);
        break;
    case SOC_DNXC_RESET_MODE_BLOCKS_AND_FABRIC_SOFT_RESET:
        rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_drv_soft_init, (unit, action));
        DNXC_IF_ERR_EXIT(rv);
        break;
    default:
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Unknown/Unsupported Reset Mode")));
    }

exit:
    DNXC_FUNC_RETURN;
}


int
soc_dnxf_drv_link_to_dch_block(int unit, int link, int *block_instance, int *inner_link)
{
    int rv;
    DNXC_INIT_FUNC_DEFS;

    rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_drv_link_to_block_mapping , (unit, link, block_instance, inner_link, SOC_BLK_DCH));
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME

