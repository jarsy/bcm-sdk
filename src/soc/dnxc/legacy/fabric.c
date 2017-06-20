/*
 * $Id: fabric.c Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT
#include <shared/bsl.h>
#include <sal/core/libc.h>
#include <soc/dnxc/legacy/fabric.h>
#include <soc/dnxc/legacy/error.h>

    
#ifdef FIXME_DNXF_LEGACY
#include <soc/dnxf/cmn/dnxf_drv.h>
#endif
#ifdef BCM_DNX_SUPPORT
#include <soc/dnx/legacy/mbcm.h>
#endif


int
soc_dnxc_fabric_pipe_map_initalize_valid_configurations(int unit, int min_tdm_priority, soc_dnxc_fabric_pipe_map_t* fabric_pipe_map_valid_config)
{
    int *config_uc;
    int *config_mc;
    int i, index = 0;
    DNXC_INIT_FUNC_DEFS;

    /* UC, MC, TDM configuration - uc={0,0,0,2} mc={1,1,1,2}  ([uc/mc x priority]-> pipe_num )*/
    sal_strncpy(fabric_pipe_map_valid_config[index].name ,"UC,MC,TDM", sizeof(fabric_pipe_map_valid_config[0].name));
    fabric_pipe_map_valid_config[index].nof_pipes=3;
    fabric_pipe_map_valid_config[index].mapping_type = soc_dnxc_fabric_pipe_map_triple_uc_mc_tdm;
    config_uc = fabric_pipe_map_valid_config[index].config_uc;
    config_mc = fabric_pipe_map_valid_config[index].config_mc;
    config_uc[0]=config_uc[1]=config_uc[2]=config_uc[3]=0;
    config_mc[0]=config_mc[1]=config_mc[2]=config_mc[3]=1;
    for (i = min_tdm_priority; i < SOC_DNXC_FABRIC_PIPE_MAX_NUM_OF_PRIORITIES; i++)
    {
        config_uc[i] = config_mc[i] = 2;
    }
    
    index++;

    /* UC, HP MC (3), LP MC (0, 1, 2) Configuration -  uc={0,0,0,0} mc={2,2,2,1} ([uc/mc x priority] -> pipe_num )*/
    sal_strncpy(fabric_pipe_map_valid_config[index].name,"UC,HP MC (3),LP MC", sizeof(fabric_pipe_map_valid_config[index].name));
    fabric_pipe_map_valid_config[index].nof_pipes=3;
    fabric_pipe_map_valid_config[index].mapping_type = soc_dnxc_fabric_pipe_map_triple_uc_hp_mc_lp_mc;
    config_uc = fabric_pipe_map_valid_config[index].config_uc;
    config_mc = fabric_pipe_map_valid_config[index].config_mc;
    config_uc[0]=config_uc[1]=config_uc[2]=config_uc[3]=0;
    config_mc[0]=config_mc[1]=config_mc[2]=2; config_mc[3]=1;

    index++;

    /* UC, HP MC (2, 3), LP MC (0, 1) (Configuration -  uc={0,0,0,0} mc={2,2,1,1} ([uc/mc x priority] -> pipe_num )*/
    sal_strncpy(fabric_pipe_map_valid_config[index].name,"UC,HP MC (2,3),LP MC", sizeof(fabric_pipe_map_valid_config[index].name));
    fabric_pipe_map_valid_config[index].nof_pipes=3;
    fabric_pipe_map_valid_config[index].mapping_type = soc_dnxc_fabric_pipe_map_triple_uc_hp_mc_lp_mc;
    config_uc = fabric_pipe_map_valid_config[index].config_uc;
    config_mc = fabric_pipe_map_valid_config[index].config_mc;
    config_uc[0]=config_uc[1]=config_uc[2]=config_uc[3]=0;
    config_mc[0]=config_mc[1]=2;config_mc[2]=config_mc[3]=1;

    index++;

    /* UC, HP MC (1, 2, 3), LP MC (0) Configuration -  uc={0,0,0,0} mc={2,2,1,1} ([uc/mc x priority] -> pipe_num )*/
    sal_strncpy(fabric_pipe_map_valid_config[index].name,"UC,HP MC (1,2,3),LP MC", sizeof(fabric_pipe_map_valid_config[index].name));
    fabric_pipe_map_valid_config[index].nof_pipes=3;
    fabric_pipe_map_valid_config[index].mapping_type = soc_dnxc_fabric_pipe_map_triple_uc_hp_mc_lp_mc;
    config_uc = fabric_pipe_map_valid_config[index].config_uc;
    config_mc = fabric_pipe_map_valid_config[index].config_mc;
    config_uc[0]=config_uc[1]=config_uc[2]=config_uc[3]=0;
    config_mc[0]=2;config_mc[1]=config_mc[2]=config_mc[3]=1;

    index++;

    /* NON_TDM , TDM Configuration - uc={0,0,0,1} mc={0,0,0,1} ([uc/mc x priority] -> pipe_num )*/
    sal_strncpy(fabric_pipe_map_valid_config[index].name, "NON_TDM,TDM", sizeof(fabric_pipe_map_valid_config[index].name));
    fabric_pipe_map_valid_config[index].nof_pipes=2;
    fabric_pipe_map_valid_config[index].mapping_type = soc_dnxc_fabric_pipe_map_dual_tdm_non_tdm;
    config_uc = fabric_pipe_map_valid_config[index].config_uc;
    config_mc = fabric_pipe_map_valid_config[index].config_mc;
    config_uc[0]=config_uc[1]=config_uc[2]=config_uc[3]=0;
    config_mc[0]=config_mc[1]=config_mc[2]=config_mc[3]=0;
    for (i = min_tdm_priority; i < SOC_DNXC_FABRIC_PIPE_MAX_NUM_OF_PRIORITIES; i++)
    {
        config_uc[i] = config_mc[i] = 1;
    }

    index++;

    /* UC,MC Configuration - uc={0,0,0,0} mc={1,1,1,1} ([uc/mc x priority] -> pipe_num) */
    sal_strncpy(fabric_pipe_map_valid_config[index].name, "UC,MC", sizeof(fabric_pipe_map_valid_config[index].name));
    fabric_pipe_map_valid_config[index].nof_pipes=2;
    fabric_pipe_map_valid_config[index].mapping_type = soc_dnxc_fabric_pipe_map_dual_uc_mc;
    config_uc = fabric_pipe_map_valid_config[index].config_uc;
    config_mc = fabric_pipe_map_valid_config[index].config_mc;
    config_uc[0]=config_uc[1]=config_uc[2]=config_uc[3]=0;
    config_mc[0]=config_mc[1]=config_mc[2]=config_mc[3]=1;

    DNXC_FUNC_RETURN;
}

int
soc_dnxc_fabric_mesh_topology_diag_get(int unit, soc_dnxc_fabric_mesh_topology_diag_t *mesh_topology_diag)
{
    DNXC_INIT_FUNC_DEFS;

    
#ifdef FIXME_DNXF_LEGACY
    if (SOC_IS_DNXF(unit))
    {

        DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_diag_mesh_topology_get, (unit, mesh_topology_diag)));
    } else
#endif 
#ifdef BCM_DNX_SUPPORT
    if (SOC_IS_DNX(unit))
    {
        if (!soc_feature(unit, soc_feature_no_fabric)) {
            DNXC_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_fabric_mesh_topology_get, (unit, mesh_topology_diag)));
        }
    } else
#endif /*BCM_DNX_SUPPORT*/
    {
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("feature unavail")));
    }

exit:
    DNXC_FUNC_RETURN;
}

