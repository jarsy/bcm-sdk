/*
 * $Id: g3p1.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <soc/sbx/g3p1/g3p1.h>
#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/g3p1/g3p1_defs.h>

#ifdef BCM_CALADAN3_G3P1_SUPPORT

#define PPE_CFG_INIT 1
#define PED_CFG_INIT 1
#define COP_CFG_INIT 1
#define CMU_CFG_INIT 1
#define TMU_CFG_INIT 1

/*
 * Function: g3p1_app_init
 *    Initialize the G3p1 Microcode Application
 * Parameters
 *    unit: Input, unit number
 *    contexts: Output, number of contexts used in application
 *    epoch: Output, epoch len or number of instr in application
 * Returns
 *    error status: one of SOC_E* 
 */
int
g3p1_app_init(int unit, soc_sbx_caladan3_ucode_pkg_t *pkg, uint32 *contexts, uint32 *epoch, int reload)
{
    int rv = SOC_E_NONE;
    soc_sbx_control_t *sbx;

    sbx = SOC_SBX_CONTROL(unit);
    if (!sbx) {
        return SOC_E_INIT;
    }

    if (!reload) {

        if (!sbx->drv) {
            sbx->drv = sal_alloc(sizeof(soc_sbx_g3p1_state_t), "g3p1 ucode state");
            if (!sbx->drv) {
                return SOC_E_MEMORY;
            }
            sal_memset(sbx->drv, 0, sizeof(soc_sbx_g3p1_state_t));
        }

#if COP_CFG_INIT
        rv = soc_sbx_g3p1_cop_init(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Caladan3 unit %d COP init failed %d\n"),
                       unit, rv));
            return (rv);
        }
#endif
#if CMU_CFG_INIT
        rv = soc_sbx_g3p1_cmu_init(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Caladan3 unit %d CMU init failed %d\n"),
                       unit, rv));
            return (rv);
        }
#endif

        rv = soc_sbx_g3p1_init(unit, (void*)pkg);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_ucodemgr_app_init: unit %d microcode application init failed (%d) \n"), unit, rv));
            return rv;
        }

#if PED_CFG_INIT

#ifdef BCM_WARM_BOOT_SUPPORT
	if(!SOC_WARM_BOOT(unit)) {
#endif /* BCM_WARM_BOOT_SUPPORT */
        rv = soc_sbx_g3p1_ped_init(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_ucodemgr_app_init: unit %d PED header init %d \n"), unit, rv));
            return rv;
        }
#ifdef BCM_WARM_BOOT_SUPPORT
	}
	else {
	  LOG_CLI((BSL_META_U(unit,
                              "WARM_BOOT_TODO: Caladan3 soc_sbx_g3p1_ped_init skipped\n")));
        }
#endif /* BCM_WARM_BOOT_SUPPORT */
#endif
#if PPE_CFG_INIT
        rv = soc_sbx_g3p1_ppe_init(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_ucodemgr_app_init: unit %d PPE configuration failed %d \n"), unit, rv));
            return SOC_E_PARAM;
        }
        rv = soc_sbx_g3p1_ppe_tables_init(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Caladan3 unit %d PPE table manager init failed %d\n"),
                       unit, rv));
            return (rv);
        }
        rv = soc_sbx_g3p1_ppe_init_ext(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Caladan3 unit %d PPE hand coded init failed %d\n"),
                       unit, rv));
            return (rv);
        }
#endif

#if TMU_CFG_INIT

        rv = soc_sbx_g3p1_tmu_init(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Caladan3 unit %d TMU table manager init failed %d\n"),
                       unit, rv));
            return (rv);
        }

#endif

        rv = g3p1_fte_partition(unit);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Caladan3 unit %d FTE init failed %d\n"),
                       unit, rv));
            return (rv);
        }

    }

    /*
     * Microcode application initialized successfully
     * Final steps before letting LR execute
     *   1) Set application params to LRP
     *   2) Set SDK overrides
     */

    /*
     * LR parameters:
     * 1) Epoch length,
     *      can this be directly taken from the ucode->m_inum
     * 2) Num of contexts used
     */
    rv = soc_sbx_g3p1_global_get(unit, "elen", epoch);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucodemgr_app_init: unit %d Reading Epoch len failed (%d), \n"), unit, rv));
        return rv;
    }

    rv = soc_sbx_g3p1_global_get(unit, "CONTEXTS", contexts);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucodemgr_app_init: unit %d Reading number of contexts used failed (%d) \n"),
                   unit, rv));
        return rv;
    }

    rv = soc_sbx_g3p1_stpstate_forward_get(unit, &sbx->stpforward);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Caladan3 unit %d stp forward get failed %d\n"),
                   unit, rv));
        return (rv);
    }
    rv = soc_sbx_g3p1_stpstate_block_get(unit, &sbx->stpblock);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Caladan3 unit %d stp block get failed %d\n"),
                   unit, rv));
        return (rv);
    }
    rv = soc_sbx_g3p1_stpstate_learn_get(unit, &sbx->stplearn);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Caladan3 unit %d stp learn get failed %d\n"),
                   unit, rv));
        return (rv);
    }

    /* Set the microcode type */
    SOC_SBX_CONTROL(unit)->ucodetype = SOC_SBX_UCODE_TYPE_G3P1;

    sal_memcpy(sbx->uver_name,
               pkg->m_uver_name,
               strlen(pkg->m_uver_name));
    sbx->uver_maj   = pkg->m_uver_maj;
    sbx->uver_min   = pkg->m_uver_min;
    sbx->uver_patch = pkg->m_uver_patch;

    if (!soc_sbx_ucode_versions[SOC_SBX_CONTROL(unit)->ucodetype]) {
        soc_sbx_ucode_versions[SOC_SBX_CONTROL(unit)->ucodetype] =
            sal_alloc(64, "ucode ver");
        if (!soc_sbx_ucode_versions[SOC_SBX_CONTROL(unit)->ucodetype]) {
            return SOC_E_MEMORY;
        }
    }
    sal_sprintf(soc_sbx_ucode_versions[SOC_SBX_CONTROL(unit)->ucodetype], "%s %d.%d.%d",
                sbx->uver_name,
                sbx->uver_maj,
                sbx->uver_min,
                sbx->uver_patch);

    
    if ((sal_strlen(sbx->uver_name) == 5) &&
         (sal_strcmp(sbx->uver_name, "g3p1a")==0)) {
        soc_sbx_caladan3_sws_pr_icc_program_arad_header(unit, 0);
    } else {
        soc_sbx_caladan3_sws_pr_icc_program_sirius_header(unit, 0, 0, 0, 0);
    }

    return rv;
}

/* pv2e.vlan is used to store the VPWS ft
 * (stores only the offset with respect to vpws uni ft base)
 * The size of the pv2e.vlan restricts the number of VPWS UNI
 */
#define G3P1_VPWS_UNI_MAX (16 * 1024)

/*
 * Function:
 *     g3p1_fte_partition
 * Purpose:
 *     compute the FTE segmentation based on user configurable values; store
 *    base addresses in the caladan3 soc state struct
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit, soc control.
 */
int
g3p1_fte_partition(int unit)
{
    uint32 fteBase, maxFtes, mc_ft_offset, pw_fo_maxgports;
    uint32 numLocalGportFtes, numNodes;
    int    rv;

    maxFtes = soc_sbx_g3p1_ft_table_size_get(unit);

    /* Cache the vsi end param for other layers to reference */
    SOC_SBX_CFG_CALADAN3(unit)->vsiEnd = 
        soc_sbx_g3p1_v2e_table_size_get(unit) - 1;

    SOC_SBX_CFG_CALADAN3(unit)->numUcodePorts = soc_sbx_caladan3_get_max_ports(unit);

#if 0
    if (SOC_SBX_CFG_CALADAN3(unit)->numUcodePorts > 
        SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule) 
    {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: Line card found to have more "
                              "ports (%d) than specified "
                              "num_max_fabric_ports_on_module (%d).\n"),
                   SOC_SBX_CFG_CALADAN3(unit)->numUcodePorts,
                   SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule));
        return SOC_E_CONFIG;
    }
#endif

    numLocalGportFtes = 
        soc_property_get(unit, spn_FTE_NUM_LOCAL_GPORTS, SBX_MAX_GPORTS);

    numNodes = soc_property_get(unit, spn_NUM_MODULES, SBX_MAX_NODES);

    /*    FTE partitions:
     *      +------------------------+
     *      |       PortMesh         |
     *      +------------------------+
     *      |        Drop            |
     *      +------------------------+
     *      |        CPU             |
     *      +------------------------+
     *      |       Trunk            |
     *      +------------------------+
     *      |     HiGig Port         |
     *      +------------------------+
     *      |     VPLS color         |
     *      +------------------------+
     *      |      Local Gport       |
     *      +------------------------+
     *      |      Global Gport      |
     *      +------------------------+--->+------------------------+
     *      |                        |    |     VLAN Flood (vid)   |
     *      |      VSI Flood         |    +------------------------+
     *      |                        |    |     VLAN Flood (vsi)   |
     *      +------------------------+--->+------------------------+
     *      |     mpls vpws uni ft   |
     *      +------------------------+--->+------------------------+
     *      |                        |    |  uknown mc Flood (vid) |
     *      |  unknown MC Flood      |    +------------------------+
     *      |                        |    |  uknown mc Flood (vsi) |
     *      +------------------------+--->+------------------------+
     *      |  MPLS PW FO FTE        |
     *      +------------------------+--->+------------------------+
     *      |      Dynamic FTE       |
     *      +------------------------+
     *      |  Extra FTEs            |
     *      +------------------------+
     *
     *  The VSI and unknown MC Flood FTEs have special meaning.  The first 4K
     * are implied reserved by vlan.c for use per vid.  The remaining are 
     * implied reserved whenever a VSI is allocated.
     */

    fteBase = 0;
    SOC_SBX_CFG_CALADAN3(unit)->fteMap[SOC_SBX_FSEG_PORT_MESH] = fteBase;
    fteBase += (SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule * numNodes);

    SOC_SBX_CFG_CALADAN3(unit)->fteMap[SOC_SBX_FSEG_DROP]    = fteBase++;
    SOC_SBX_CFG_CALADAN3(unit)->fteMap[SOC_SBX_FSEG_CPU]     = fteBase++;
    SOC_SBX_CFG_CALADAN3(unit)->fteMap[SOC_SBX_FSEG_TRUNK]   = fteBase;
    fteBase += SBX_MAX_TRUNKS;

    SOC_SBX_CFG_CALADAN3(unit)->fteMap[SOC_SBX_FSEG_HG_PORT] = fteBase;

    SOC_SBX_CFG_CALADAN3(unit)->fteMap[SOC_SBX_FSEG_VPLS_COLOR] = fteBase;
    fteBase += SBX_MAX_VPLS_COLORS;

    SOC_SBX_CFG_CALADAN3(unit)->fteMap[SOC_SBX_FSEG_LGPORT]  = fteBase;
    fteBase +=  numLocalGportFtes;

    SOC_SBX_CFG_CALADAN3(unit)->fteMap[SOC_SBX_FSEG_GGPORT]  = fteBase;

    /* VSI FTEs are defined by a ucode pushdown variable, the first
     * 4K are for VIDs the remaining are for dynamic VSIs
     */
    rv = soc_sbx_g3p1_vlan_ft_base_get(unit, &fteBase);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Failed to get vlan fte base: %d %s\n"),
                   rv, soc_errmsg(rv)));
        return rv;
    }
    pw_fo_maxgports = fteBase;
    SOC_SBX_CFG_CALADAN3(unit)->fteMap[SOC_SBX_FSEG_VID_VSI] = fteBase;    

    fteBase += SBX_MAX_VID;
    SOC_SBX_CFG_CALADAN3(unit)->fteMap[SOC_SBX_FSEG_DYN_VSI] = fteBase;

    fteBase = (SOC_SBX_CFG_CALADAN3(unit)->fteMap[SOC_SBX_FSEG_VID_VSI] +
               SOC_SBX_CFG_CALADAN3(unit)->vsiEnd + 1);

    SOC_SBX_CFG_CALADAN3(unit)->fteMap[SOC_SBX_FSEG_VPWS_UNI] = fteBase;
    /* move the fteBase (point to the entry after the end of VPWS_UNI fte) */
    fteBase += G3P1_VPWS_UNI_MAX;
    pw_fo_maxgports += G3P1_VPWS_UNI_MAX;

    /* Unknown MC flood FTEs are tied to the VSI FTEs.  That is, when a
     * VSI FTE is allocated, the Unknown MC flood FTE is implied.
     */
    rv = soc_sbx_g3p1_mc_ft_offset_get(unit, &mc_ft_offset);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Failed to get mc fte base: %d %s\n"),
                   rv, soc_errmsg(rv)));
        return rv;
    }

    if (mc_ft_offset) {
        /* Start of Unknown MC FTEs = vid_flood base + mc_ft_offset;
         */
        fteBase = mc_ft_offset + 
                      SOC_SBX_CFG_CALADAN3(unit)->fteMap[SOC_SBX_FSEG_VID_VSI];
    }

    SOC_SBX_CFG_CALADAN3(unit)->fteMap[SOC_SBX_FSEG_UMC] = fteBase;
    /* move the fteBase (point to the entry after the end of UMC fte) */
    fteBase += SOC_SBX_CFG_CALADAN3(unit)->vsiEnd + 1;

    if (fteBase < maxFtes) {
        SOC_SBX_CFG_CALADAN3(unit)->fteMap[SOC_SBX_FSEG_PW_FO] = fteBase;
        /* move the fteBase (point to the entry after the end of PW FO fte) */
        fteBase = fteBase + pw_fo_maxgports;
    }

    if (fteBase < maxFtes) {
        /* Dynamic FTEs - unpriveldege FTEs */
        SOC_SBX_CFG_CALADAN3(unit)->fteMap[SOC_SBX_FSEG_DYNAMIC] = fteBase;            
        SOC_SBX_CFG_CALADAN3(unit)->fteMap[SOC_SBX_FSEG_EXTRA]   = 0;
    } else {
        SOC_SBX_CFG_CALADAN3(unit)->fteMap[SOC_SBX_FSEG_DYNAMIC] = 0;
        SOC_SBX_CFG_CALADAN3(unit)->fteMap[SOC_SBX_FSEG_EXTRA] = 0;
    }

    SOC_SBX_CFG_CALADAN3(unit)->fteMap[SOC_SBX_FSEG_END] = maxFtes;

    return rv;
}

int soc_sbx_g3p1_v4_ena(int unit)
{
    return (SOC_SBX_V4_ENABLE(unit));
}

int soc_sbx_g3p1_v6_ena(int unit)
{
    return (SOC_SBX_V6_ENABLE(unit));
}

int soc_sbx_g3p1_mplstp_ena(int unit)
{
    return (SOC_SBX_MPLSTP_ENABLE(unit));
}

#endif

