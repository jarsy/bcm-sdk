/*
 * $Id: jer_trunk.c Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_TRUNK

/* 
 *  INCLUDES
 */

#include <soc/dpp/port_sw_db.h>
#include <soc/dpp/drv.h>

#include <soc/dpp/ARAD/arad_drv.h>
#include <soc/dpp/ARAD/arad_init.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/ARAD/arad_trunk.h>

#include <soc/dpp/JER/jer_trunk.h>

/* 
 *  FUNCTIONS
 */ 

/* 
 * Configuration Functions
 */

/* 
 *  soc_jer_port_direct_lb_key_set - set min & max LB-Key 
 *  if set_min == FALSE  - dosn't set min value.
 *  if set_max == FALSE  - dosn't set max value.
 */
uint32 
  soc_jer_trunk_direct_lb_key_set( 
    SOC_SAND_IN int    unit, 
    SOC_SAND_IN int    core_id, 
    SOC_SAND_IN uint32 local_port,
    SOC_SAND_IN uint32 min_lb_key,
    SOC_SAND_IN uint32 set_min,
    SOC_SAND_IN uint32 max_lb_key,
    SOC_SAND_IN uint32 set_max
   )
{
    uint32  base_q_pair = 0;
    uint32 nof_pairs;
    uint32 curr_q_pair;
    uint32 use_table_2 = 0x0;
    uint32 field_val; 
    ARAD_EGQ_PPCT_TBL_DATA egq_pp_ppct_data;
    uint64 val64;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_tm_port_to_base_q_pair_get(unit, core_id, local_port, &base_q_pair));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_tm_port_to_out_port_priority_get(unit, core_id, local_port, &nof_pairs));

    /* Check if lb-key table 2 (EGQ_PER_PORT_LB_KEY_RANGE) is used  */
    if(SOC_DPP_CONFIG(unit)->arad->init.fabric.ftmh_lb_ext_mode == ARAD_MGMT_FTMH_LB_EXT_MODE_STANDBY_MC_LB) 
    {
        SOCDNX_IF_ERR_EXIT(READ_IHB_LBP_GENERAL_CONFIG_1r(unit,SOC_CORE_ALL,&val64));
        field_val = soc_reg64_field32_get(unit, IHB_LBP_GENERAL_CONFIG_1r, val64, OVERRIDE_FTMH_LB_KEY_MSB_VALUEf);
        if (field_val == 0) 
        {
            use_table_2 = 1;
        }
    }

    /* Run over all match q-pairs and set lb_key min/max */
    for (curr_q_pair = base_q_pair; curr_q_pair - base_q_pair < nof_pairs; curr_q_pair++)
    {
        SOCDNX_IF_ERR_EXIT( arad_egq_ppct_tbl_get_unsafe(unit, core_id, curr_q_pair, &egq_pp_ppct_data));

        if(use_table_2 == 1)
        {
            /* update PPCT */
            if (0x1 == set_min) {
                egq_pp_ppct_data.second_range_lb_key_min = min_lb_key | 0x80; /*  added because msb is 1 */
            }
            if (0x1 == set_max) {
                egq_pp_ppct_data.second_range_lb_key_max = max_lb_key | 0x80; /*  added because msb is 1 */
            }
        } else {
            /* update PPCT */
            if (0x1 == set_min) {
                egq_pp_ppct_data.lb_key_min = min_lb_key;
            }
            if (0x1 == set_max) {
                egq_pp_ppct_data.lb_key_max = max_lb_key;
            }
        }

        SOCDNX_IF_ERR_EXIT(
           arad_egq_ppct_tbl_set_unsafe( unit, core_id, curr_q_pair, &egq_pp_ppct_data));
    }
exit:
  SOCDNX_FUNC_RETURN;
}

/* 
 * soc_jer_port_direct_lb_key_get - get min & max LB-Key 
 *  if set_min == NULL  - doesn't get min value.
 *  if set_max == NULL  - doesn't get max value.
 */
uint32 
    soc_jer_trunk_direct_lb_key_get(
      SOC_SAND_IN int unit, 
      SOC_SAND_IN int core_id, 
      SOC_SAND_IN uint32  local_port,
      SOC_SAND_OUT uint32 *min_lb_key,
      SOC_SAND_OUT uint32 *max_lb_key
   )
{
    uint32 
        base_q_pair;
    ARAD_EGQ_PPCT_TBL_DATA
        egq_pp_ppct_data;
    uint32 
        field_val, 
        use_table_2=0;
    uint64 
        val64;

    SOCDNX_INIT_FUNC_DEFS;  

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_tm_port_to_base_q_pair_get(unit, core_id, local_port, &base_q_pair));

    /* Check if the 2nd lb-key fields are used  */
    if(SOC_DPP_CONFIG(unit)->arad->init.fabric.ftmh_lb_ext_mode == ARAD_MGMT_FTMH_LB_EXT_MODE_STANDBY_MC_LB) {
        SOCDNX_IF_ERR_EXIT(READ_IHB_LBP_GENERAL_CONFIG_1r(unit, 0, &val64));
        field_val = soc_reg64_field32_get(unit, IHB_LBP_GENERAL_CONFIG_1r, val64, OVERRIDE_FTMH_LB_KEY_MSB_VALUEf);
        if(field_val == 1) {
          use_table_2 = 1;
        }
    }

    SOCDNX_IF_ERR_EXIT(
       arad_egq_ppct_tbl_get_unsafe(
          unit,
          core_id,
          base_q_pair,
          &egq_pp_ppct_data));

     if(use_table_2 == 0){
         /* update return value */
         if (min_lb_key != NULL) {
           *min_lb_key = egq_pp_ppct_data.lb_key_min;
         }
         if (max_lb_key != NULL) {
           *max_lb_key = egq_pp_ppct_data.lb_key_max;
         }
     } else {
         if (min_lb_key != NULL) {
           *min_lb_key = egq_pp_ppct_data.second_range_lb_key_min & (~0x80); /* remove bit 7 witch allways set on */
         }
         if (max_lb_key != NULL) {
           *max_lb_key = egq_pp_ppct_data.second_range_lb_key_max & (~0x80); /* remove bit 7 witch allways set on */
         }
     }
exit:
  SOCDNX_FUNC_RETURN;
}

/* 
 * Init functions
 */

/* 
 * #LAG-groups * #LAG-members: 
 * 0x0 - 1K groups of 16
 * 0x1 - 512 groups of 32
 * 0x2 - 256 groups of 64
 * 0x3 - 128 groups of 128
 * 0x4 - 64 groups of 256
 */
uint32
  soc_jer_trunk_mode_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PORT_LAG_MODE  lag_mode
  )
{
  SOCDNX_INIT_FUNC_DEFS;

  /* Same encoding HW and SW */
  SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, ECI_GLOBAL_GENERAL_CFG_3r, REG_PORT_ANY, 0, LAG_MODEf,  lag_mode));

  /* Enable LAG filtering on UM + MC */
  SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_LAG_FILTER_ENABLEr, SOC_CORE_ALL, 0, ENABLE_LAG_FILTER_MCf,  0x1));
  SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_LAG_FILTER_ENABLEr, SOC_CORE_ALL, 0, ENABLE_LAG_FILTER_UCf,  0x0));

exit:
  SOCDNX_FUNC_RETURN;
}

int soc_jer_trunk_init_tables (int unit)
{
    int                 core_index;
    uint32              index;
    uint32              table_entry[128] = {0};
    uint32              egq_data[SOC_DPP_IMP_DEFS_MAX(EGQ_PPCT_NOF_LONGS)];
    uint32              nof_cores;
    soc_mem_t           mem;
    soc_reg_t           reg;

    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_PCID_LITE_SKIP(unit);

    /* PER PORT CONFIGURATION TABLE */
    mem = EGQ_PPCTm;
    for (index = soc_mem_index_min(unit,mem) ; index < soc_mem_index_count(unit,mem) ; index++) {
        SOCDNX_IF_ERR_EXIT(READ_EGQ_PPCTm (unit, EGQ_BLOCK(unit, 0), index, egq_data));
        soc_mem_field32_set(unit, mem, egq_data, LB_KEY_MINf, 0x0);
        soc_mem_field32_set(unit, mem, egq_data, LB_KEY_MAXf, 0xff);
        soc_mem_field32_set(unit, mem, egq_data, SECOND_RANGE_LB_KEY_MINf, 0x0);
        soc_mem_field32_set(unit, mem, egq_data, SECOND_RANGE_LB_KEY_MAXf, 0xff);
        SOCDNX_IF_ERR_EXIT(WRITE_EGQ_PPCTm (unit, EGQ_BLOCK(unit, SOC_CORE_ALL), index, egq_data));
    }

    

    if (SOC_DPP_CONFIG(unit)->emulation_system == 0) {
        /* LAG MAPPING */
        mem = (SOC_IS_QAX(unit)) ? TAR_LAG_MAPPINGm : IRR_LAG_MAPPINGm;
        SOCDNX_IF_ERR_EXIT(arad_fill_table_with_entry(unit, mem, MEM_BLOCK_ANY, table_entry));
    }

    reg = (SOC_IS_QAX(unit)) ? TAR_ENABLE_DYNAMIC_MEMORY_ACCESSr : IRR_ENABLE_DYNAMIC_MEMORY_ACCESSr;

    
    SOCDNX_IF_ERR_EXIT(soc_reg_field32_modify(unit, reg, REG_PORT_ANY, ENABLE_DYNAMIC_MEMORY_ACCESSf, 1));
    
    if (SOC_DPP_CONFIG(unit)->emulation_system == 0) {
        if(SOC_IS_QAX(unit))
        {
            /* LAG NEXT MEMBER */
            SOCDNX_IF_ERR_EXIT(arad_fill_table_with_entry(unit, TAR_LAG_NEXT_MEMBERm, MEM_BLOCK_ANY, table_entry));
            /* LAG TO LAG RANGE */
            SOCDNX_IF_ERR_EXIT(arad_fill_table_with_entry(unit, TAR_LAG_TO_LAG_RANGEm, MEM_BLOCK_ANY, table_entry));
        }
        else
        {
        /* LAG NEXT MEMBER */
        SOCDNX_IF_ERR_EXIT(arad_fill_table_with_entry(unit, IRR_LAG_NEXT_MEMBERm, MEM_BLOCK_ANY, table_entry));

        /* LAG TO LAG RANGE */
        SOCDNX_IF_ERR_EXIT(arad_fill_table_with_entry(unit, IRR_LAG_TO_LAG_RANGEm, MEM_BLOCK_ANY, table_entry));
        }
    }

    /* SMOOTH DIVISION */
    /* Write the smooth division table using DMA. */
    mem = (SOC_IS_QAX(unit)) ?  TAR_SMOOTH_DIVISIONm : IRR_SMOOTH_DIVISIONm;
    nof_cores = SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores;
    for (core_index = 0; core_index < nof_cores; ++core_index) 
    {
        SOCDNX_IF_ERR_EXIT(arad_fill_table_with_variable_values_by_caching(unit, mem, core_index, MEM_BLOCK_ALL, -1, -1, _arad_mgmt_irr_tbls_init_dma_callback, NULL));
    }
    
exit:
    SOCDNX_FUNC_RETURN;
}

uint32
  soc_jer_trunk_resolve_use_msb(int unit)
{
    ARAD_MGMT_INIT *init = &(SOC_DPP_CONFIG(unit)->arad->init);
    uint32
        val;
    uint8
        smooth_division_use_msb = init->ports.smooth_division_resolve_using_msb,
        stack_use_msb = init->ports.stack_resolve_using_msb;

    SOCDNX_INIT_FUNC_DEFS;

    if (smooth_division_use_msb || stack_use_msb) { /* these fields' default values are 0 */     
        if (SOC_IS_QAX(unit))
        {
            SOCDNX_IF_ERR_EXIT(READ_TAR_STATIC_CONFIGURATIONr(unit, &val));
            soc_reg_field_set(unit, TAR_STATIC_CONFIGURATIONr, &val, LAG_LB_KEY_STACK_RESOLVE_USE_MSBf, stack_use_msb);
            soc_reg_field_set(unit, TAR_STATIC_CONFIGURATIONr, &val, LAG_LB_KEY_SMOOTH_DIVISION_USE_MSBf, smooth_division_use_msb);
            SOCDNX_IF_ERR_EXIT(WRITE_TAR_STATIC_CONFIGURATIONr(unit, val));
        }
        else
        {
            SOCDNX_IF_ERR_EXIT(READ_IRR_STATIC_CONFIGURATIONr(unit, &val));
            soc_reg_field_set(unit, IRR_STATIC_CONFIGURATIONr, &val, LAG_LB_KEY_STACK_RESOLVE_USE_MSBf, stack_use_msb);
            soc_reg_field_set(unit, IRR_STATIC_CONFIGURATIONr, &val, LAG_LB_KEY_SMOOTH_DIVISION_USE_MSBf, smooth_division_use_msb);
            SOCDNX_IF_ERR_EXIT(WRITE_IRR_STATIC_CONFIGURATIONr(unit, val));
        }   
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_trunk_resolve_lb_key_ext_mode
 * Purpose:
 *      Set configurations for ftmh lb ext mode
 * Parameters:
 *      unit    - Device Number
 *      fabric  - ptr to Fabric
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer_trunk_resolve_lb_key_ext_mode(int unit, ARAD_INIT_FABRIC *fabric)
{
    uint64 reg64_val;

    SOCDNX_INIT_FUNC_DEFS;

    if(fabric->ftmh_lb_ext_mode == ARAD_MGMT_FTMH_LB_EXT_MODE_STANDBY_MC_LB) 
    {
        /* Each OTM-Port has two ranges */
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_GLOBAL_CONFIGr, SOC_CORE_ALL, 0, USE_SINGLE_LB_RANGEf, 0));
        
        /* the msb bit of the FTMH.LbKey will be overriden by the next register */
        SOCDNX_IF_ERR_EXIT(READ_IHB_LBP_GENERAL_CONFIG_1r(unit, SOC_CORE_ALL, &reg64_val));
        soc_reg64_field32_set(unit, IHB_LBP_GENERAL_CONFIG_1r, &reg64_val, OVERRIDE_FTMH_LB_KEY_MSBf, 1);
        soc_reg64_field32_set(unit, IHB_LBP_GENERAL_CONFIG_1r, &reg64_val, OVERRIDE_FTMH_LB_KEY_MSB_VALUEf, 0);
        SOCDNX_IF_ERR_EXIT(WRITE_IHB_LBP_GENERAL_CONFIG_1r(unit, SOC_CORE_ALL, reg64_val));
    }

   if(fabric->ftmh_lb_ext_mode == ARAD_MGMT_FTMH_LB_EXT_MODE_FULL_HASH) 
   {
        /* set StampLagLbKeyOnUserHeader2 to 1 */
        SOCDNX_IF_ERR_EXIT( soc_reg_above_64_field32_modify(unit, IHB_LBP_GENERAL_CONFIG_0r, SOC_CORE_ALL, 0, STAMP_LAG_LB_KEY_ON_USER_HEADER_2f, 1));
        /* set LbKeyExtUseMsbBits to 1 */
        SOCDNX_IF_ERR_EXIT( soc_reg_above_64_field32_modify(unit, IHB_LBP_GENERAL_CONFIG_0r, SOC_CORE_ALL, 0, LB_KEY_EXT_USE_MSB_BITSf, 1));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_trunk_resolve_hash_key_manipulations
 * Purpose:
 *      Set configurations different hash lb key manipulations
 * Parameters:
 *      unit    - Device Number
 *      fabric  - ptr to Fabric
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer_trunk_resolve_hash_key_manipulations(int unit, ARAD_INIT_FABRIC *fabric)
{
    SOCDNX_INIT_FUNC_DEFS;

    if(fabric->trunk_hash_format == ARAD_MGMT_TRUNK_HASH_FORMAT_INVERTED){
        /* Set SwitchLagLbKeyBits */ 
        SOCDNX_IF_ERR_EXIT( soc_reg_above_64_field32_modify(unit, IHB_LBP_GENERAL_CONFIG_0r, SOC_CORE_ALL, 0, SWITCH_LAG_LB_KEY_BITSf, 1));
     
    } else if(fabric->trunk_hash_format == ARAD_MGMT_TRUNK_HASH_FORMAT_DUPLICATED){
       /* set DuplicateLagLbKeyMsbsToLsbs to 1 */
        SOCDNX_IF_ERR_EXIT( soc_reg_above_64_field32_modify(unit, IHB_LBP_GENERAL_CONFIG_0r, SOC_CORE_ALL, 0, DUPLICATE_LAG_LB_KEY_MSBS_TO_LSBSf, 1));
    }

exit:
    SOCDNX_FUNC_RETURN;
}


int soc_jer_trunk_init (int unit)
{
    ARAD_MGMT_INIT *init = &(SOC_DPP_CONFIG(unit)->arad->init);

    SOCDNX_INIT_FUNC_DEFS;
    
    /* LAG mode configuration */
    SOCDNX_IF_ERR_EXIT(soc_jer_trunk_mode_set_unsafe(unit, init->ports.lag_mode));

    /* Lag Ingress MC Group configuration */
    SOCDNX_IF_ERR_EXIT(soc_arad_trunk_resolve_ingress_mc_destination_method(unit, init->ports.use_trunk_as_ingress_mc_dest));

    /* Using the lb-key's LSB/MSB in smooth-division and stacking resolutions */
    SOCDNX_IF_ERR_EXIT(soc_jer_trunk_resolve_use_msb(unit));

    /* Set Configurations for STANDBY_MC_LB */
    SOCDNX_IF_ERR_EXIT(soc_jer_trunk_resolve_lb_key_ext_mode(unit, &init->fabric));

    /* Set Configurations for Hash-key manipulation */
    SOCDNX_IF_ERR_EXIT(soc_jer_trunk_resolve_hash_key_manipulations(unit, &init->fabric));

    /* Initialization of LAG tables */
    SOCDNX_IF_ERR_EXIT(soc_jer_trunk_init_tables(unit));

    
    /*SOCDNX_IF_ERR_EXIT(soc_jer_trunk_init_stack(unit));*/

exit:
  SOCDNX_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME

