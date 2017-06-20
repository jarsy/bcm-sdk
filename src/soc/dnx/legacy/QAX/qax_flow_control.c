#include <shared/bsl.h>

#include <soc/mcm/memregs.h>
/* $Id: jer2_qax_flow_control.c,v 1.59 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FLOWCONTROL

/*************
 * INCLUDES  *
 *************/
#include <shared/swstate/access/sw_state_access.h>
#include <soc/mem.h>
/* { */

#include <soc/dnxc/legacy/error.h>

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/drv.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/ARAD/arad_api_framework.h>
#include <soc/dnx/legacy/ARAD/arad_flow_control.h>
#include <soc/dnx/legacy/JER/jer_flow_control.h>
#include <soc/dnx/legacy/QAX/qax_flow_control.h>
#include <soc/dnx/legacy/ARAD/arad_reg_access.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_api_mgmt.h>
#include <soc/dnx/legacy/TMC/tmc_api_egr_queuing.h>
#include <soc/dnx/legacy/port_sw_db.h>
#include <soc/portmod/portmod.h>
#include <soc/dnx/legacy/JER/jer_egr_queuing.h>
#include <soc/mcm/allenum.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* } */
/*************
 * MACROS    *
 *************/
/* { */


/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

/* } */
/*************
 * GLOBALS   *
 *************/
/* { */
static uint32
        pfc_map_reg[SOC_DNX_DEFS_MAX(NOF_FC_PFC_GENERIC_BITMAPS)] = {
        CFC_PFC_GENERIC_BITMAP_0r, CFC_PFC_GENERIC_BITMAP_1r, CFC_PFC_GENERIC_BITMAP_2r, CFC_PFC_GENERIC_BITMAP_3r,
        CFC_PFC_GENERIC_BITMAP_4r, CFC_PFC_GENERIC_BITMAP_5r, CFC_PFC_GENERIC_BITMAP_6r, CFC_PFC_GENERIC_BITMAP_7r,
        CFC_PFC_GENERIC_BITMAP_8r, CFC_PFC_GENERIC_BITMAP_9r, CFC_PFC_GENERIC_BITMAP_10r, CFC_PFC_GENERIC_BITMAP_11r,
        CFC_PFC_GENERIC_BITMAP_12r, CFC_PFC_GENERIC_BITMAP_13r, CFC_PFC_GENERIC_BITMAP_14r, CFC_PFC_GENERIC_BITMAP_15r,
        CFC_PFC_GENERIC_BITMAP_16r, CFC_PFC_GENERIC_BITMAP_17r, CFC_PFC_GENERIC_BITMAP_18r, CFC_PFC_GENERIC_BITMAP_19r,
        CFC_PFC_GENERIC_BITMAP_20r, CFC_PFC_GENERIC_BITMAP_21r, CFC_PFC_GENERIC_BITMAP_22r, CFC_PFC_GENERIC_BITMAP_23r,
        CFC_PFC_GENERIC_BITMAP_24r, CFC_PFC_GENERIC_BITMAP_25r, CFC_PFC_GENERIC_BITMAP_26r, CFC_PFC_GENERIC_BITMAP_27r,
        CFC_PFC_GENERIC_BITMAP_28r, CFC_PFC_GENERIC_BITMAP_29r, CFC_PFC_GENERIC_BITMAP_30r, CFC_PFC_GENERIC_BITMAP_31r,
        CFC_PFC_GENERIC_BITMAP_32r, CFC_PFC_GENERIC_BITMAP_33r, CFC_PFC_GENERIC_BITMAP_34r, CFC_PFC_GENERIC_BITMAP_35r,
        CFC_PFC_GENERIC_BITMAP_36r, CFC_PFC_GENERIC_BITMAP_37r, CFC_PFC_GENERIC_BITMAP_38r, CFC_PFC_GENERIC_BITMAP_39r,
        };
/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

soc_error_t
  jer2_qax_fc_pfc_generic_bitmap_verify(
    DNX_SAND_IN   int   unit,
    DNX_SAND_IN   int       priority,
    DNX_SAND_IN   uint32    pfc_bitmap_index
  )
{
    DNXC_INIT_FUNC_DEFS;

    if ((priority < 0) || (priority > DNX_TMC_EGR_NOF_Q_PRIO_JER2_ARAD - 1)) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    if(pfc_bitmap_index >= SOC_DNX_DEFS_GET(unit, nof_fc_pfc_generic_bitmaps)) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_qax_fc_pfc_generic_bitmap_set(
    DNX_SAND_IN   int                              unit,
    DNX_SAND_IN   int                              priority,
    DNX_SAND_IN   uint32                           pfc_bitmap_index,
    DNX_SAND_IN   DNX_TMC_FC_PFC_GENERIC_BITMAP    *pfc_bitmap
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    uint32
        reg_offset;
    soc_reg_above_64_val_t  
        pfc_map_data;

    DNXC_INIT_FUNC_DEFS;

    rv = jer2_qax_fc_pfc_generic_bitmap_verify(unit, priority, pfc_bitmap_index);
    DNXC_IF_ERR_EXIT(rv);

    SOC_REG_ABOVE_64_CLEAR(pfc_map_data);

    /* Read the selected Generic Bitmap */
    rv = soc_reg_above_64_get(unit, pfc_map_reg[pfc_bitmap_index], REG_PORT_ANY, 0, pfc_map_data);
    DNXC_IF_ERR_EXIT(rv);
    
    /* Update the Generic Bitmap */
    for(reg_offset = 0; reg_offset < DNX_TMC_FC_PFC_GENERIC_BITMAP_SIZE / 32; reg_offset++)
    {
        pfc_map_data[reg_offset] = pfc_bitmap->bitmap[reg_offset];
    }

    /* Write the updated Generic Bitmap */
    rv = soc_reg_above_64_set(unit, pfc_map_reg[pfc_bitmap_index], REG_PORT_ANY, 0, pfc_map_data);
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_qax_fc_pfc_generic_bitmap_get(
    DNX_SAND_IN   int   unit,
    DNX_SAND_IN   int       priority,
    DNX_SAND_IN   uint32    pfc_bitmap_index,
    DNX_SAND_OUT   DNX_TMC_FC_PFC_GENERIC_BITMAP    *pfc_bitmap
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    uint32
        reg_offset;
    soc_reg_above_64_val_t  
        pfc_map_data;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(pfc_bitmap);
    rv = jer2_qax_fc_pfc_generic_bitmap_verify(unit, priority, pfc_bitmap_index);
    DNXC_IF_ERR_EXIT(rv);

    SOC_REG_ABOVE_64_CLEAR(pfc_map_data);

    /* Read the selected Generic Bitmap */
    rv = soc_reg_above_64_get(unit, pfc_map_reg[pfc_bitmap_index], REG_PORT_ANY, 0, pfc_map_data);
    DNXC_IF_ERR_EXIT(rv);
  
    /* Get the requested bitmap */
    pfc_bitmap->core = 0;
    for(reg_offset = 0; reg_offset < DNX_TMC_FC_PFC_GENERIC_BITMAP_SIZE / 32; reg_offset++)
    {
        pfc_bitmap->bitmap[reg_offset] = pfc_map_data[reg_offset];
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_qax_fc_glb_rcs_mask_verify(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN int                           core,
      DNX_SAND_IN int                           is_high_prio,
      DNX_SAND_IN uint32                        glb_res_src_bitmap
  )
{
    DNXC_INIT_FUNC_DEFS;

    /* Place your code here */
    if (core > SOC_DNX_DEFS_GET(unit, nof_cores)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("core is out of range")));
    }

    if (glb_res_src_bitmap && (!(glb_res_src_bitmap & (1<<DNX_TMC_FC_GLB_RES_TYPE_OCB)))
                            && (!(glb_res_src_bitmap & (1<<DNX_TMC_FC_GLB_RES_TYPE_DRAM)))
                            && (!(glb_res_src_bitmap & (1<<DNX_TMC_FC_GLB_RES_TYPE_POOL0)))
                            && (!(glb_res_src_bitmap & (1<<DNX_TMC_FC_GLB_RES_TYPE_POOL1)))
                            && (!(glb_res_src_bitmap & (1<<DNX_TMC_FC_GLB_RES_TYPE_OCB_HEADROOM)))) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("global resouce src flag is error")));
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_qax_fc_glb_rcs_mask_set(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN int                           core,
      DNX_SAND_IN int                           is_high_prio,
      DNX_SAND_IN DNX_TMC_FC_GLB_RES_TYPE       glb_res_dst,
      DNX_SAND_IN uint32                        glb_res_src_bitmap
    )
{
    soc_error_t res = DNX_SAND_OK;
    soc_reg_above_64_val_t reg_data;
    soc_reg_t reg = CFC_GLB_RSC_CGM_MASKr;
    soc_field_t field = INVALIDf;
    uint32 field_value = 0;

    DNXC_INIT_FUNC_DEFS;

    res = jer2_qax_fc_glb_rcs_mask_verify(unit, core, is_high_prio, glb_res_src_bitmap);
    DNXC_IF_ERR_EXIT(res);
    if (is_high_prio) {
        field_value = 2;   /* Bit 1 for HP mask */
    }
    else {
        field_value = 1;   /* Bit 0 for LP mask */
    }

    res = soc_reg_above_64_get(unit, reg, REG_PORT_ANY, 0, reg_data);
    DNXC_IF_ERR_EXIT(res);

    if (glb_res_src_bitmap & (1<<DNX_TMC_FC_GLB_RES_TYPE_OCB)) {
        field = GLB_RSC_SRAM_MASKf;
        soc_reg_above_64_field32_set(unit, reg, reg_data, field, field_value); 
    }
    if (glb_res_src_bitmap & (1<<DNX_TMC_FC_GLB_RES_TYPE_DRAM)) {
        field = GLB_RSC_MIX_MASKf;
        soc_reg_above_64_field32_set(unit, reg, reg_data, field, field_value); 
    }
    if (glb_res_src_bitmap & (1<<DNX_TMC_FC_GLB_RES_TYPE_POOL0)) {
        field = GLB_RSC_POOL_0_MASKf;
        soc_reg_above_64_field32_set(unit, reg, reg_data, field, field_value); 
    }
    if (glb_res_src_bitmap & (1<<DNX_TMC_FC_GLB_RES_TYPE_POOL1)) {
        field = GLB_RSC_POOL_1_MASKf;
        soc_reg_above_64_field32_set(unit, reg, reg_data, field, field_value); 
    } 
    if (glb_res_src_bitmap & (1<<DNX_TMC_FC_GLB_RES_TYPE_OCB_HEADROOM)) {
        field_value = 1; /* Headroom only one threshold */
        field = GLB_RSC_HDRM_MASKf;
        soc_reg_above_64_field32_set(unit, reg, reg_data, field, field_value);
    }

    res = soc_reg_above_64_set(unit, reg, REG_PORT_ANY, 0, reg_data);
    DNXC_IF_ERR_EXIT(res);
  
exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_qax_fc_glb_rcs_mask_get(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN int                           core,
      DNX_SAND_IN int                           is_high_prio,
      DNX_SAND_IN DNX_TMC_FC_GLB_RES_TYPE       glb_res_dst,
      DNX_SAND_OUT uint32                       *glb_res_src_bitmap
    )
{
    soc_error_t res = DNX_SAND_OK;
    soc_reg_above_64_val_t reg_data;
    soc_reg_t reg = CFC_GLB_RSC_CGM_MASKr;
    soc_field_t field = INVALIDf;
    uint32 field_value = 0;

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(glb_res_src_bitmap);

    *glb_res_src_bitmap = 0;
    res = jer2_qax_fc_glb_rcs_mask_verify(unit, core, is_high_prio, *glb_res_src_bitmap);
    DNXC_IF_ERR_EXIT(res);

    res = soc_reg_above_64_get(unit, reg, REG_PORT_ANY, 0, reg_data);
    DNXC_IF_ERR_EXIT(res);

    field = GLB_RSC_SRAM_MASKf;
    field_value = soc_reg_above_64_field32_get(unit, reg, reg_data, field);
    if (field_value) {
        (*glb_res_src_bitmap) |= (1<<DNX_TMC_FC_GLB_RES_TYPE_OCB);
    }

    field = GLB_RSC_MIX_MASKf;
    field_value = soc_reg_above_64_field32_get(unit, reg, reg_data, field);
    if (field_value) {
        (*glb_res_src_bitmap) |= (1<<DNX_TMC_FC_GLB_RES_TYPE_DRAM);
    }

    field = GLB_RSC_POOL_0_MASKf;
    field_value = soc_reg_above_64_field32_get(unit, reg, reg_data, field);
    if (field_value) {
        (*glb_res_src_bitmap) |= (1<<DNX_TMC_FC_GLB_RES_TYPE_POOL0);
    }

    field = GLB_RSC_POOL_1_MASKf;
    field_value = soc_reg_above_64_field32_get(unit, reg, reg_data, field);
    if (field_value) {
        (*glb_res_src_bitmap) |= (1<<DNX_TMC_FC_GLB_RES_TYPE_POOL1);
    }
	
    field = GLB_RSC_HDRM_MASKf;
    field_value = soc_reg_above_64_field32_get(unit, reg, reg_data, field);
    if (field_value) {
        (*glb_res_src_bitmap) |= (1<<DNX_TMC_FC_GLB_RES_TYPE_OCB_HEADROOM);
    }

exit:
    DNXC_FUNC_RETURN;
}

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

