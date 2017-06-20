/*
 * $Id: ramon_rx.c,v 1.8.64.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC RAMON RX
 */


#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_RX

#include <bcm/types.h>

#include <shared/bsl.h>

#include <soc/mcm/allenum.h>
#include <soc/mcm/memregs.h>
#include <soc/defs.h>
#include <soc/types.h>
#include <soc/error.h>

#include <soc/dnxc/legacy/error.h>

#include <soc/dnxf/cmn/dnxf_drv.h>
#include <soc/dnxf/cmn/dnxf_warm_boot.h>

#include <soc/dnxf/ramon/ramon_rx.h>
#include <soc/dnxf/ramon/ramon_stack.h>

#define LINKS_ARRAY_SIZE (SOC_DNXF_MAX_NOF_LINKS/32 + (SOC_DNXF_MAX_NOF_LINKS%32? 1:0) + 1 /*for ECC*/)

soc_error_t
soc_ramon_rx_cpu_address_modid_set(int unit, int num_of_cpu_addresses, int *cpu_addresses)
{
    int i;
    bcm_module_t alrc_max_base_index, update_base_index;
    uint32 mem_val[LINKS_ARRAY_SIZE];
    uint32 reg_val32;
    soc_dnxf_fabric_isolate_type_t isolate_type = soc_dnxf_fabric_isolate_type_none;
    soc_dnxc_isolation_status_t isolation_status = soc_dnxc_isolation_status_active;

    DNXC_INIT_FUNC_DEFS;

    /* Check for valid num of cpu addresses */
    if ((num_of_cpu_addresses < 0) || (SOC_RAMON_RX_MAX_NOF_CPU_BUFFERS > SOC_RAMON_RX_MAX_NOF_CPU_BUFFERS))
    {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("num_of_cpu_addresses invalid \n")));
    }

    /* Check that traffic is disabled, e.g. FE is isolated or under shutdown. Writing to ECI_GLOBAL registers under traffic can cause undefined behavior. */
    if (SOC_DNXF_IS_FE13(unit))
    {
        DNXC_IF_ERR_EXIT(SOC_DNXF_WARM_BOOT_VAR_GET(unit, ISOLATE_TYPE, &isolate_type));
    } else {
        DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL(unit,mbcm_dnxf_fabric_topology_isolate_get,(unit, &isolation_status)));
    }
    if (isolate_type == soc_dnxf_fabric_isolate_type_none && isolation_status == soc_dnxc_isolation_status_active) {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("bcm_rx_start/stop should not be called when traffic is enabled.\nTo disable traffic use bcm_fabric_control_set with bcmFabricIsolate/bcmFabricShutdown parameter.\n ")));
    }

    /* issue a warning if a cpu address is not in the interval [AlrcMaxBaseIndex,UpdateBaseIndex]*/

    DNXC_IF_ERR_EXIT(soc_ramon_stk_module_max_all_reachable_get(unit, &alrc_max_base_index));
    DNXC_IF_ERR_EXIT(soc_ramon_stk_module_max_fap_get(unit, &update_base_index));

    for (i = 0 ; i < num_of_cpu_addresses ; i++)
    {
        if (!((cpu_addresses[i] >= alrc_max_base_index) && (cpu_addresses[i] <= update_base_index)))
        {
            LOG_CLI((BSL_META_U(unit,
                "warning: cpu address is not in the interval [AlrcMaxBaseIndex,UpdateBaseIndex] (the ignored interval for calculating all reachable vector)\n")));
        }
    }


    DNXC_IF_ERR_EXIT(READ_ECI_GLOBAL_FE_DEST_IDS_1r(unit, &reg_val32));
    /* updating first cpu address */
    if (num_of_cpu_addresses >= 1)
    {
        soc_reg_field_set(unit, ECI_GLOBAL_FE_DEST_IDS_1r, &reg_val32, FE_DEST_ID_0f, cpu_addresses[0]);
    }
    soc_reg_field_set(unit, ECI_GLOBAL_FE_DEST_IDS_1r, &reg_val32, FE_DEST_ID_0_ENf, num_of_cpu_addresses >= 1 ? 1:0);

    if (num_of_cpu_addresses >= 2)
    {
        soc_reg_field_set(unit, ECI_GLOBAL_FE_DEST_IDS_1r, &reg_val32, FE_DEST_ID_1f, cpu_addresses[1]);
    }
    soc_reg_field_set(unit, ECI_GLOBAL_FE_DEST_IDS_1r, &reg_val32, FE_DEST_ID_1_ENf, num_of_cpu_addresses >= 2 ? 1:0);

    DNXC_IF_ERR_EXIT(WRITE_ECI_GLOBAL_FE_DEST_IDS_1r(unit, reg_val32));

    DNXC_IF_ERR_EXIT(READ_ECI_GLOBAL_FE_DEST_IDS_2r(unit, &reg_val32));

    if (num_of_cpu_addresses >= 3)
    {
        soc_reg_field_set(unit, ECI_GLOBAL_FE_DEST_IDS_2r, &reg_val32, FE_DEST_ID_2f, cpu_addresses[2]);
    }

    soc_reg_field_set(unit, ECI_GLOBAL_FE_DEST_IDS_2r, &reg_val32, FE_DEST_ID_2_ENf, num_of_cpu_addresses >= 3 ? 1:0);

    if (num_of_cpu_addresses >= 4)
    {
        soc_reg_field_set(unit, ECI_GLOBAL_FE_DEST_IDS_2r, &reg_val32, FE_DEST_ID_3f, cpu_addresses[3]);
    }

    soc_reg_field_set(unit, ECI_GLOBAL_FE_DEST_IDS_2r, &reg_val32, FE_DEST_ID_3_ENf, num_of_cpu_addresses >= 4 ? 1:0);

    DNXC_IF_ERR_EXIT(WRITE_ECI_GLOBAL_FE_DEST_IDS_2r(unit, reg_val32));

    if (SOC_DNXF_IS_FE13(unit) && !SOC_DNXF_CONFIG(unit).fabric_local_routing_enable)
    {
        for (i=0 ; i < SOC_DNXF_DEFS_GET(unit, nof_instances_dch)/2; i++) { /*FE1*/
            DNXC_IF_ERR_EXIT(READ_DCH_GLOBAL_FE_DEST_IDS_1r(unit, i, &reg_val32));
            soc_reg_field_set(unit, DCH_GLOBAL_FE_DEST_IDS_1r, &reg_val32, FE_DEST_ID_0_ENf, 0X0);
            soc_reg_field_set(unit, DCH_GLOBAL_FE_DEST_IDS_1r, &reg_val32, FE_DEST_ID_1_ENf, 0x0);
            DNXC_IF_ERR_EXIT(WRITE_DCH_GLOBAL_FE_DEST_IDS_1r(unit, i, reg_val32));
        }

        /* 
         * FE13 ignores cpu destinations - 
         * shouold set reachablity tables to -1 on order to indicate that the cpu is reachable -                                                       
         */
        SHR_BITSET_RANGE(mem_val, 0, SOC_DNXF_DEFS_GET(unit, nof_links));
        for (i = 0 ; i < num_of_cpu_addresses ; i++)
        {
            DNXC_IF_ERR_EXIT(WRITE_RTP_RMHMTm(unit,MEM_BLOCK_ALL,cpu_addresses[i], mem_val));
            DNXC_IF_ERR_EXIT(WRITE_RTP_DUCTPm(unit, 0, MEM_BLOCK_ALL,cpu_addresses[i], mem_val));
            DNXC_IF_ERR_EXIT(WRITE_RTP_DUCTPm(unit, 1, MEM_BLOCK_ALL,cpu_addresses[i], mem_val));
            DNXC_IF_ERR_EXIT(WRITE_RTP_DUCTPm(unit, 2, MEM_BLOCK_ALL,cpu_addresses[i], mem_val));
            DNXC_IF_ERR_EXIT(WRITE_RTP_DUCTPm(unit, 3, MEM_BLOCK_ALL,cpu_addresses[i], mem_val));
        }                  
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_ramon_rx_cpu_address_modid_init(int unit)
{
    uint32 reg_val;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(READ_ECI_GLOBAL_FE_DEST_IDS_1r(unit, &reg_val));
    soc_reg_field_set(unit, ECI_GLOBAL_FE_DEST_IDS_1r, &reg_val, FE_DEST_ID_0_ENf, 0);
    soc_reg_field_set(unit, ECI_GLOBAL_FE_DEST_IDS_1r, &reg_val, FE_DEST_ID_1_ENf, 0);
    DNXC_IF_ERR_EXIT(WRITE_ECI_GLOBAL_FE_DEST_IDS_1r(unit, reg_val));

    DNXC_IF_ERR_EXIT(READ_ECI_GLOBAL_FE_DEST_IDS_2r(unit, &reg_val));
    soc_reg_field_set(unit, ECI_GLOBAL_FE_DEST_IDS_2r, &reg_val, FE_DEST_ID_2_ENf, 0);
    soc_reg_field_set(unit, ECI_GLOBAL_FE_DEST_IDS_2r, &reg_val, FE_DEST_ID_3_ENf, 0);
    DNXC_IF_ERR_EXIT(WRITE_ECI_GLOBAL_FE_DEST_IDS_2r(unit, reg_val));

exit:
    DNXC_FUNC_RETURN;
}


#undef _ERR_MSG_MODULE_NAME
