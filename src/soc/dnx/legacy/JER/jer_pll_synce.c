/*
 * $Id: jer2_jer_pll_synce_init.c, v1 17/09/2014 09:55:39 azarrin $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

/*************
 * INCLUDES  *
 *************/


/* SOC includes */
#include <soc/debug.h>
#include <soc/error.h>
#include <soc/dnxc/legacy/error.h>

/* SOC DNX includes */
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/port_sw_db.h>

#include <soc/dnx/legacy/JER/jer_fabric.h>
#include <soc/dnx/legacy/JER/jer_pll_synce.h>

#if defined PORTMOD_SUPPORT
#include <soc/portmod/portmod.h>
#include <phymod/chip/bcmi_tsce_xgxs_defs.h>
#ifndef PHYMOD_EXCLUDE_CHIPLESS_TYPES
#define PHYMOD_EXCLUDE_CHIPLESS_TYPES
#include <phymod/chip/bcmi_tscf_xgxs_defs.h>
#undef PHYMOD_EXCLUDE_CHIPLESS_TYPES
#endif /* PHYMOD_EXCLUDE_CHIPLESS_TYPES */
#endif

#include <soc/dnx/legacy/ARAD/arad_chip_regs.h>

/*************
 * DEFINES   *
 *************/
#ifdef _ERR_MSG_MODULE_NAME
    #error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT



/*************
 * TYPE DEFS *
 *************/
typedef enum
{
    JER2_QAX_PLL_TYPE_BSPLL0 = 0,
    JER2_QAX_PLL_TYPE_BSPLL1 = 1
}JER2_QAX_BSPLL_TYPE;

typedef enum
{
    QUX_PLL_TYPE_NIF_PML = 0,
    QUX_PLL_TYPE_NIF_PMX = 1
}QUX_NIF_PLL_TYPE;


/*************
 * FUNCTIONS *
 *************/

static int jer2_jer_pll_4_set(
                    int                         unit,
                    JER2_QAX_BSPLL_TYPE                pll_type)
{
    uint32 val;
    soc_reg_above_64_val_t reg_above_64, field_above_64;

    const static soc_reg_t
        eci_srd_pll_config[] = {ECI_BS_0_PLL_CONFIGr, ECI_BS_1_PLL_CONFIGr};

    const static soc_reg_t
        eci_srd_pll_status[] = {ECI_BS_0_PLL_STATUSr, ECI_BS_1_PLL_STATUSr};
    const static soc_field_t
        eci_srd_pll_locked[] = {MISC_PLL_1_LOCKEDf, MISC_PLL_2_LOCKEDf};
    
    soc_field_t  bs_pll_config_field;

    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(reg_above_64);
    SOC_REG_ABOVE_64_CLEAR(field_above_64);

    /* ndiv */
    if(SOC_IS_QAX(unit)) {
       val = 120; /* Table-13,Qumran-doc.docx */
    } else {
       val = 140;
    }

    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_BS_0_PLL_CONFIGr, reg_above_64, BS_0_PLL_FBDIV_NDIV_INTf, field_above_64);

    /* mdiv ch 0 */
    if(SOC_IS_QAX(unit)) {
       val = 150; /* 20MHz BSPLL0/1 is configured instead 25MHz */
    } else {
       val = 140;
    }
    bs_pll_config_field = SOC_IS_QUX(unit) ? ITEM_125_132f : BS_0_PLL_CH_0_MDIVf;
    SHR_BITCOPY_RANGE(field_above_64, 0, &val, 0, sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_BS_0_PLL_CONFIGr, reg_above_64, bs_pll_config_field, field_above_64);

    /* kp */
    val = 3;
    bs_pll_config_field = SOC_IS_QUX(unit) ? ITEM_6_9f : BS_0_PLL_KPf;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_BS_0_PLL_CONFIGr, reg_above_64, bs_pll_config_field, field_above_64);

    /* ki */
    val = 2;
    bs_pll_config_field = SOC_IS_QUX(unit) ? ITEM_10_12f : BS_0_PLL_KIf;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_BS_0_PLL_CONFIGr, reg_above_64, bs_pll_config_field, field_above_64);

    /* ka */
    val = 0;
    bs_pll_config_field = SOC_IS_QUX(unit) ? ITEM_13_15f : BS_0_PLL_KAf;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_BS_0_PLL_CONFIGr, reg_above_64, bs_pll_config_field, field_above_64);

    /* pdiv */
    val = 1;
    bs_pll_config_field = SOC_IS_QUX(unit) ? ITEM_16_19f : BS_0_PLL_PDIVf;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_BS_0_PLL_CONFIGr, reg_above_64, bs_pll_config_field, field_above_64);

    /* control pwm_rate */
    val = 0;
    bs_pll_config_field = SOC_IS_QUX(unit) ? ITEM_82_83f : BS_0_PLL_CTRL_VEC_PWM_RATEf;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_BS_0_PLL_CONFIGr, reg_above_64, bs_pll_config_field, field_above_64);

    /* control post_rst_sel */
    val = 2;
    bs_pll_config_field = SOC_IS_QUX(unit) ? ITEM_84_85f : BS_0_PLL_CTRL_VEC_POST_RESET_SELf;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_BS_0_PLL_CONFIGr, reg_above_64, bs_pll_config_field, field_above_64);

    /* control vco_fb_div2 */
    val = 1;
    bs_pll_config_field = SOC_IS_QUX(unit) ? ITEM_86_86f : BS_0_PLL_CTRL_VEC_VCO_FB_DIV_2f;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_BS_0_PLL_CONFIGr, reg_above_64, bs_pll_config_field, field_above_64);

    /* control vco_range */
    val = 2;
    bs_pll_config_field = SOC_IS_QUX(unit) ? ITEM_90_91f : BS_0_PLL_CTRL_VEC_VCO_RANGEf;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_BS_0_PLL_CONFIGr, reg_above_64, bs_pll_config_field, field_above_64);

    /* control ldo */
    val = 1;
    bs_pll_config_field = SOC_IS_QUX(unit) ? ITEM_92_93f : BS_0_PLL_CTRL_VEC_LDOf;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_BS_0_PLL_CONFIGr, reg_above_64, bs_pll_config_field, field_above_64);

    /* pwron ldo*/
    val = 1;
    bs_pll_config_field = SOC_IS_QUX(unit) ? ITEM_59_59f : BS_0_PLL_PWRON_LDOf;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_BS_0_PLL_CONFIGr, reg_above_64, bs_pll_config_field, field_above_64);

    /* set config register with init values wo pwron */
    DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, ECI_BS_0_PLL_CONFIGr, REG_PORT_ANY, 0, reg_above_64));

    sal_usleep(1);

    /* pwron */
    val = 1;
    bs_pll_config_field = SOC_IS_QUX(unit) ? ITEM_142_142f : BS_0_PLL_PWR_ONf;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_BS_0_PLL_CONFIGr, reg_above_64, bs_pll_config_field, field_above_64);

    /* set config register with init values with pwron */
    DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, eci_srd_pll_config[pll_type-JER2_QAX_PLL_TYPE_BSPLL0], REG_PORT_ANY, 0, reg_above_64));

    sal_usleep(30);

    /* asserting the reset bit */
    DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, eci_srd_pll_config[pll_type-JER2_QAX_PLL_TYPE_BSPLL0], REG_PORT_ANY, 0, reg_above_64));
    SOC_REG_ABOVE_64_CLEAR(field_above_64);
    SOC_REG_ABOVE_64_CREATE_MASK(field_above_64, 1, 0);
    bs_pll_config_field = SOC_IS_QUX(unit) ? ITEM_144_144f : BS_0_PLL_RESETBf;
    soc_reg_above_64_field_set(unit, ECI_BS_0_PLL_CONFIGr, reg_above_64, bs_pll_config_field, field_above_64);

    DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, eci_srd_pll_config[pll_type-JER2_QAX_PLL_TYPE_BSPLL0], REG_PORT_ANY, 0, reg_above_64));

    /* polling on pll_locked */
    if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {
        DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, eci_srd_pll_status[pll_type-JER2_QAX_PLL_TYPE_BSPLL0], REG_PORT_ANY, 0, eci_srd_pll_locked[pll_type-JER2_QAX_PLL_TYPE_BSPLL0], 1));
    }

    /* post reset */
    DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, eci_srd_pll_config[pll_type-JER2_QAX_PLL_TYPE_BSPLL0], REG_PORT_ANY, 0, reg_above_64));
    SOC_REG_ABOVE_64_CLEAR(field_above_64);
    SOC_REG_ABOVE_64_CREATE_MASK(field_above_64, 1, 0);
    bs_pll_config_field = SOC_IS_QUX(unit) ? ITEM_143_143f : BS_0_PLL_POST_RESETBf;
    soc_reg_above_64_field_set(unit, ECI_BS_0_PLL_CONFIGr, reg_above_64, bs_pll_config_field, field_above_64);

    DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, eci_srd_pll_config[pll_type-JER2_QAX_PLL_TYPE_BSPLL0], REG_PORT_ANY, 0, reg_above_64));

exit:
    DNXC_FUNC_RETURN;
}

/*
 * PLL_3 configurations used by both pll and sync init
 */
int jer2_jer_pll_3_set(
                    int                         unit,
                    JER2_JER_PLL_TYPE                pll_type,
                    uint32                      ndiv,
                    uint32                      mdiv,
                    uint32                      pdiv,
                    uint32                      is_bypass)
{
    uint32 val;
    soc_reg_above_64_val_t reg_above_64, field_above_64;

    const static soc_reg_t
        eci_srd_pll_config[] = {ECI_FAB_0_PLL_CONFIGr, ECI_FAB_1_PLL_CONFIGr, ECI_NIF_PMH_PLL_CONFIGr, ECI_NIF_PML_0_PLL_CONFIGr, ECI_NIF_PML_1_PLL_CONFIGr, ECI_SYNCE_MASTER_PLL_CONFIGr, ECI_SYNCE_SLAVE_PLL_CONFIGr};
    const static soc_reg_t
        eci_srd_pll_status[] = {ECI_FAB_0_PLL_STATUSr, ECI_FAB_1_PLL_STATUSr, ECI_PMH_PLL_STATUSr, ECI_PML_0_PLL_STATUSr, ECI_PML_1_PLL_STATUSr, ECI_MASTER_SYNCE_PLL_STATUSr, ECI_SLAVE_SYNCE_PLL_STATUSr};
    const static soc_field_t
        eci_srd_pll_locked[] = {MISC_PLL_2_LOCKEDf, MISC_PLL_3_LOCKEDf, MISC_PLL_4_LOCKEDf, MISC_PLL_5_LOCKEDf, MISC_PLL_6_LOCKEDf, MISC_PLL_7_LOCKEDf, MISC_PLL_8_LOCKEDf};

    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(reg_above_64);
    SOC_REG_ABOVE_64_CLEAR(field_above_64);

    if(!SOC_IS_QAX(unit)) {

       /* ndiv */
       SHR_BITCOPY_RANGE(field_above_64,0,&ndiv,0,sizeof(ndiv)*8);
       soc_reg_above_64_field_set(unit, ECI_FAB_0_PLL_CONFIGr, reg_above_64, FAB_0_PLL_CFG_NDIVf, field_above_64);

       if (pll_type == JER2_JER_PLL_TYPE_FABRIC_0) {
          /* mdiv ch 4 */
          val = 20;
          SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
          soc_reg_above_64_field_set(unit, ECI_FAB_0_PLL_CONFIGr, reg_above_64, FAB_0_PLL_CFG_CH_4_MDIVf, field_above_64);
       }

       /* mdiv ch 0 */
       SHR_BITCOPY_RANGE(field_above_64,0,&mdiv,0,sizeof(mdiv)*8);
       soc_reg_above_64_field_set(unit, ECI_FAB_0_PLL_CONFIGr, reg_above_64, FAB_0_PLL_CFG_CH_0_MDIVf, field_above_64);

       /* kp */
       val = 6;
       SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
       soc_reg_above_64_field_set(unit, ECI_FAB_0_PLL_CONFIGr, reg_above_64, FAB_0_PLL_CFG_KPf, field_above_64);

       /* ki */
       val = 4;
       SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
       soc_reg_above_64_field_set(unit, ECI_FAB_0_PLL_CONFIGr, reg_above_64, FAB_0_PLL_CFG_KIf, field_above_64);

       /* kpp */
       val = 0;
       SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
       soc_reg_above_64_field_set(unit, ECI_FAB_0_PLL_CONFIGr, reg_above_64, FAB_0_PLL_CFG_KPPf, field_above_64);

       /* pdiv */
       SHR_BITCOPY_RANGE(field_above_64,0,&pdiv,0,sizeof(pdiv)*8);
       soc_reg_above_64_field_set(unit, ECI_FAB_0_PLL_CONFIGr, reg_above_64, FAB_0_PLL_CFG_PDIVf, field_above_64);

       /* control stat_reset */
       val = 1;
       SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
       soc_reg_above_64_field_set(unit, ECI_FAB_0_PLL_CONFIGr, reg_above_64, FAB_0_PLL_CFG_PLL_CTRL_STAT_RESETf, field_above_64);

       /* control lc_boost */
       val = 1;
       SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
       soc_reg_above_64_field_set(unit, ECI_FAB_0_PLL_CONFIGr, reg_above_64, FAB_0_PLL_CFG_PLL_CTRL_LC_BOOSTf, field_above_64);

       /* control cmlbuf0_pwron */
       val = 1;
       SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
       soc_reg_above_64_field_set(unit, ECI_FAB_0_PLL_CONFIGr, reg_above_64, FAB_0_PLL_CFG_PLL_CTRL_CMLBUF_0_PWRONf, field_above_64);

       /* control cmlbuf1_pwron */
       val = 1;
       SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
       soc_reg_above_64_field_set(unit, ECI_FAB_0_PLL_CONFIGr, reg_above_64, FAB_0_PLL_CFG_PLL_CTRL_CMLBUF_1_PWRONf, field_above_64);

       /* control pwm_rate */
       val = 2;
       SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
       soc_reg_above_64_field_set(unit, ECI_FAB_0_PLL_CONFIGr, reg_above_64, FAB_0_PLL_CFG_PLL_CTRL_PWM_RATEf, field_above_64);

       /* control post_rst_sel */
       val = 2;
       SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
       soc_reg_above_64_field_set(unit, ECI_FAB_0_PLL_CONFIGr, reg_above_64, FAB_0_PLL_CFG_PLL_CTRL_POST_RST_SELf, field_above_64);

       /* pwron */
       val = 1;
       SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
       soc_reg_above_64_field_set(unit, ECI_FAB_0_PLL_CONFIGr, reg_above_64, FAB_0_PLL_CFG_PWRONf, field_above_64);

       /* pwron ldo*/
       val = 1;
       SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
       soc_reg_above_64_field_set(unit, ECI_FAB_0_PLL_CONFIGr, reg_above_64, FAB_0_PLL_CFG_PWRON_LDOf, field_above_64);

       /* synce source clock from fabric/nif */
       if (is_bypass) {
          /* fabric */
          val = 0;
       } else {
	    /* nif */
	    val = 1;
       }
       if (eci_srd_pll_config[pll_type] == ECI_SYNCE_MASTER_PLL_CONFIGr) {
          SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
          soc_reg_above_64_field_set(unit, eci_srd_pll_config[pll_type], reg_above_64, SYNCE_MASTER_PLL_CFG_REFRENCE_NIF_SELf, field_above_64);

          /* bypass configuration for synce master */
          val = is_bypass;
          SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
          soc_reg_above_64_field_set(unit, eci_srd_pll_config[pll_type], reg_above_64, SYNCE_MASTER_PLL_CFG_BYPASS_PLL_ENf, field_above_64);
       }
       if (eci_srd_pll_config[pll_type] == ECI_SYNCE_SLAVE_PLL_CONFIGr) {
          SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
          soc_reg_above_64_field_set(unit, eci_srd_pll_config[pll_type], reg_above_64, SYNCE_SLAVE_PLL_CFG_REFRENCE_NIF_SELf, field_above_64);

          /* bypass configuration for synce slave */
          val = is_bypass;
          SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
          soc_reg_above_64_field_set(unit, eci_srd_pll_config[pll_type], reg_above_64, SYNCE_SLAVE_PLL_CFG_BYPASS_PLL_ENf, field_above_64);
       }

       /* set config register with init values */
       DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, eci_srd_pll_config[pll_type], REG_PORT_ANY, 0, reg_above_64));

       sal_usleep(30);

       /* asserting the reset bit */
       DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, eci_srd_pll_config[pll_type], REG_PORT_ANY, 0, reg_above_64));
       SOC_REG_ABOVE_64_CLEAR(field_above_64);
       SOC_REG_ABOVE_64_CREATE_MASK(field_above_64, 1, 0);
       soc_reg_above_64_field_set(unit, ECI_FAB_0_PLL_CONFIGr, reg_above_64, FAB_0_PLL_CFG_RESET_Bf, field_above_64);
       DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, eci_srd_pll_config[pll_type], REG_PORT_ANY, 0, reg_above_64));

       /* polling on pll_locked */
       if(!is_bypass){
          if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {
              DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, eci_srd_pll_status[pll_type], REG_PORT_ANY, 0, eci_srd_pll_locked[pll_type], 1));
          }
       }
      /* post reset */
      DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, eci_srd_pll_config[pll_type], REG_PORT_ANY, 0, reg_above_64));
      SOC_REG_ABOVE_64_CLEAR(field_above_64);
      SOC_REG_ABOVE_64_CREATE_MASK(field_above_64, 1, 0);
      soc_reg_above_64_field_set(unit, ECI_FAB_0_PLL_CONFIGr, reg_above_64, FAB_0_PLL_CFG_POST_RESET_Bf, field_above_64);
      DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, eci_srd_pll_config[pll_type], REG_PORT_ANY, 0, reg_above_64));
   } else { /* JER2_QAX Specific */
      /* ndiv */
      SHR_BITCOPY_RANGE(field_above_64,0,&ndiv,0,sizeof(ndiv)*8);
      soc_reg_above_64_field_set(unit, ECI_SYNCE_MASTER_PLL_CONFIGr, reg_above_64, SYNCE_MASTER_PLL_CFG_NDIVf, field_above_64);

      if (pll_type == JER2_JER_PLL_TYPE_FABRIC_0) {
          /* mdiv ch 4 */
          val = 20;
          SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
          soc_reg_above_64_field_set(unit, ECI_SYNCE_MASTER_PLL_CONFIGr, reg_above_64, SYNCE_MASTER_PLL_CFG_CH_4_MDIVf, field_above_64);
      }

      /* mdiv ch 0 */
      SHR_BITCOPY_RANGE(field_above_64,0,&mdiv,0,sizeof(mdiv)*8);
      soc_reg_above_64_field_set(unit, ECI_SYNCE_MASTER_PLL_CONFIGr, reg_above_64, SYNCE_MASTER_PLL_CFG_CH_0_MDIVf, field_above_64);

      /* kp */
      val = 6;
      SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
      soc_reg_above_64_field_set(unit, ECI_SYNCE_MASTER_PLL_CONFIGr, reg_above_64, SYNCE_MASTER_PLL_CFG_KPf, field_above_64);

      /* ki */
      val = 4;
      SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
      soc_reg_above_64_field_set(unit, ECI_SYNCE_MASTER_PLL_CONFIGr, reg_above_64, SYNCE_MASTER_PLL_CFG_KIf, field_above_64);

      /* kpp */
      val = 0;
      SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
      soc_reg_above_64_field_set(unit, ECI_SYNCE_MASTER_PLL_CONFIGr, reg_above_64, SYNCE_MASTER_PLL_CFG_KPPf, field_above_64);

      /* pdiv */
      SHR_BITCOPY_RANGE(field_above_64,0,&pdiv,0,sizeof(pdiv)*8);
      soc_reg_above_64_field_set(unit, ECI_SYNCE_MASTER_PLL_CONFIGr, reg_above_64, SYNCE_MASTER_PLL_CFG_PDIVf, field_above_64);

      /* control stat_reset */
      val = 1;
      SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
      soc_reg_above_64_field_set(unit, ECI_SYNCE_MASTER_PLL_CONFIGr, reg_above_64, SYNCE_MASTER_PLL_CFG_PLL_CTRL_STAT_RESETf, field_above_64);

      /* control lc_boost */
      val = 1;
      SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
      soc_reg_above_64_field_set(unit, ECI_SYNCE_MASTER_PLL_CONFIGr, reg_above_64, SYNCE_MASTER_PLL_CFG_PLL_CTRL_LC_BOOSTf, field_above_64);

      /* control cmlbuf0_pwron */
      val = 1;
      SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
      soc_reg_above_64_field_set(unit, ECI_SYNCE_MASTER_PLL_CONFIGr, reg_above_64, SYNCE_MASTER_PLL_CFG_PLL_CTRL_CMLBUF_0_PWRONf, field_above_64);

      /* control cmlbuf1_pwron */
      val = 1;
      SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
      soc_reg_above_64_field_set(unit, ECI_SYNCE_MASTER_PLL_CONFIGr, reg_above_64, SYNCE_MASTER_PLL_CFG_PLL_CTRL_CMLBUF_1_PWRONf, field_above_64);

      /* control pwm_rate */
      val = 2;
      SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
      soc_reg_above_64_field_set(unit, ECI_SYNCE_MASTER_PLL_CONFIGr, reg_above_64, SYNCE_MASTER_PLL_CFG_PLL_CTRL_PWM_RATEf, field_above_64);

      /* control post_rst_sel */
      val = 2;
      SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
      soc_reg_above_64_field_set(unit, ECI_SYNCE_MASTER_PLL_CONFIGr, reg_above_64, SYNCE_MASTER_PLL_CFG_PLL_CTRL_POST_RST_SELf, field_above_64);

      /* pwron */
      val = 1;
      SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
      soc_reg_above_64_field_set(unit, ECI_SYNCE_MASTER_PLL_CONFIGr, reg_above_64, SYNCE_MASTER_PLL_CFG_PWRONf, field_above_64);

      /* pwron ldo*/
      val = 1;
      SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
      soc_reg_above_64_field_set(unit, ECI_SYNCE_MASTER_PLL_CONFIGr, reg_above_64, SYNCE_MASTER_PLL_CFG_PWRON_LDOf, field_above_64);

      /* synce source clock from fabric/nif */
      if (is_bypass) {
         /* fabric */
         val = 0;
      } else {
         /* nif */
         val = 1;
      }
      if (eci_srd_pll_config[pll_type] == ECI_SYNCE_MASTER_PLL_CONFIGr) {
         SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
         soc_reg_above_64_field_set(unit, eci_srd_pll_config[pll_type], reg_above_64, SYNCE_MASTER_PLL_CFG_REFRENCE_NIF_SELf, field_above_64);

         /* bypass configuration for synce master */
         val = is_bypass;
         SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
         soc_reg_above_64_field_set(unit, eci_srd_pll_config[pll_type], reg_above_64, SYNCE_MASTER_PLL_CFG_BYPASS_PLL_ENf, field_above_64);
      }
      if (eci_srd_pll_config[pll_type] == ECI_SYNCE_SLAVE_PLL_CONFIGr) {
         SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
         soc_reg_above_64_field_set(unit, eci_srd_pll_config[pll_type], reg_above_64, SYNCE_SLAVE_PLL_CFG_REFRENCE_NIF_SELf, field_above_64);

         /* bypass configuration for synce slave */
         val = is_bypass;
         SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
         soc_reg_above_64_field_set(unit, eci_srd_pll_config[pll_type], reg_above_64, SYNCE_SLAVE_PLL_CFG_BYPASS_PLL_ENf, field_above_64);
      }

      /* set config register with init values */
      DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, eci_srd_pll_config[pll_type], REG_PORT_ANY, 0, reg_above_64));

      sal_usleep(30);

      /* asserting the reset bit */
      DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, eci_srd_pll_config[pll_type], REG_PORT_ANY, 0, reg_above_64));
      SOC_REG_ABOVE_64_CLEAR(field_above_64);
      SOC_REG_ABOVE_64_CREATE_MASK(field_above_64, 1, 0);
      soc_reg_above_64_field_set(unit, ECI_SYNCE_MASTER_PLL_CONFIGr, reg_above_64, SYNCE_MASTER_PLL_CFG_RESET_Bf, field_above_64);
      DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, eci_srd_pll_config[pll_type], REG_PORT_ANY, 0, reg_above_64));

      /* polling on pll_locked */
      if(!is_bypass){
         if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {
             DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, eci_srd_pll_status[pll_type], REG_PORT_ANY, 0, eci_srd_pll_locked[pll_type], 1));
          }
      }
      /* post reset */
      DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, eci_srd_pll_config[pll_type], REG_PORT_ANY, 0, reg_above_64));
      SOC_REG_ABOVE_64_CLEAR(field_above_64);
      SOC_REG_ABOVE_64_CREATE_MASK(field_above_64, 1, 0);
      soc_reg_above_64_field_set(unit, ECI_SYNCE_MASTER_PLL_CONFIGr, reg_above_64, SYNCE_MASTER_PLL_CFG_POST_RESET_Bf, field_above_64);
      DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, eci_srd_pll_config[pll_type], REG_PORT_ANY, 0, reg_above_64));


   }

    if(is_bypass){
        
    }

exit:
    DNXC_FUNC_RETURN;
}

/*
 * PLL_2 configurations used by both pll and sync init
 */
static int jer2_jer_pll_2_set(
                    int                         unit,
                    JER2_JER_PLL_TYPE                pll_type,
                    uint32                      ndiv,
                    uint32                      mdiv,
                    uint32                      icp,
                    uint32                      is_bypass)
{
    uint32 val;
    soc_reg_above_64_val_t reg_above_64, field_above_64;

    const static soc_reg_t
        eci_srd_pll_config[] = {ECI_NIF_PMH_PLL_CONFIGr, ECI_NIF_PML_0_PLL_CONFIGr, ECI_NIF_PML_1_PLL_CONFIGr};
    const static soc_reg_t
        eci_srd_pll_status[] = {ECI_PMH_PLL_STATUSr, ECI_PML_0_PLL_STATUSr, ECI_PML_1_PLL_STATUSr};
    const static soc_field_t
        eci_srd_pll_locked[] = {MISC_PLL_4_LOCKEDf, MISC_PLL_5_LOCKEDf, MISC_PLL_6_LOCKEDf};

    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(reg_above_64);
    SOC_REG_ABOVE_64_CLEAR(field_above_64);

    /* CP */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_CPf, field_above_64);

    /* CP_1 */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_CP_1f, field_above_64);

    /* PDIV */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_PDIVf, field_above_64);

    /* CH_0_MDIV */
    SHR_BITCOPY_RANGE(field_above_64,0,&mdiv,0,sizeof(mdiv)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_CH_0_MDIVf, field_above_64);

    /* CH_1_MDIV */
    val = 0x5;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_CH_1_MDIVf, field_above_64);

    /* CH_2_MDIV */
    val = 0x15;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_CH_2_MDIVf, field_above_64);

    /* CH_3_MDIV */
    SHR_BITCOPY_RANGE(field_above_64,0,&mdiv,0,sizeof(mdiv)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_CH_3_MDIVf, field_above_64);

    /* CH_4_MDIV */
    val = 0x5;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_CH_4_MDIVf, field_above_64);

    /* RZ  */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_RZf, field_above_64);

    /* CZ */
    val = 3;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_CZf, field_above_64);

    /* MSC_CTRL_VEC_REF_SEL */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_MSC_CTRL_VEC_REF_SELf, field_above_64);

    /* MSC_CTRL_VEC_CML_EN */
    val = 3;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_MSC_CTRL_VEC_CML_ENf, field_above_64);

    /* LDO_CTRL */
    val = 0x2a;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_LDO_CTRLf, field_above_64);

    /* CTRL_VEC_CPP */
    val = 0x80;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_CTRL_VEC_CPPf, field_above_64);

    /* CTRL_VEC_FREQ_DOUBL_DEL */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_CTRL_VEC_FREQ_DOUBL_DELf, field_above_64);

    /* ICP */
    SHR_BITCOPY_RANGE(field_above_64,0,&icp,0,sizeof(icp)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_ICPf, field_above_64);

    /* RP */
    val = 3;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_RPf, field_above_64);

    /* MSC_CTRL_VEC_TEST_BUF_EN */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_MSC_CTRL_VEC_TEST_BUF_ENf, field_above_64);

    /* FBDIV_NDIV_INT */
    if(SOC_IS_QAX(unit)) {
	val = 0x14; /* Selecting default Fref=156.25MHz for JER2_QAX */
    } else {
	val = 0x19;
    }
    SHR_BITCOPY_RANGE(field_above_64,0,&ndiv,0,sizeof(ndiv)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_FBDIV_NDIV_INTf, field_above_64);

    /* BYP_HOLDOVER */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_BYP_HOLDOVERf, field_above_64);

    /* MSC_CTRL_VEC_D_2C_BIAS */
    val = 4;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_MSC_CTRL_VEC_D_2C_BIASf, field_above_64);

    /* FBDIV_NDIV_INT */
    val = 3;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_CTRL_VEC_POST_RES_SELf, field_above_64);

    /* PWRON */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_PWRONf, field_above_64);

    /* PWRON_LDO */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_PWRON_LDOf, field_above_64);

    /* set config register with init values */
    DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, eci_srd_pll_config[pll_type - JER2_JER_PLL_TYPE_NIF_PMH], REG_PORT_ANY, 0, reg_above_64));

    sal_usleep(30);

    /* asserting the reset bit */
    DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, eci_srd_pll_config[pll_type - JER2_JER_PLL_TYPE_NIF_PMH], REG_PORT_ANY, 0, reg_above_64));
    SOC_REG_ABOVE_64_CLEAR(field_above_64);
    SOC_REG_ABOVE_64_CREATE_MASK(field_above_64, 1, 0);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_RESETBf, field_above_64);
    DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, eci_srd_pll_config[pll_type - JER2_JER_PLL_TYPE_NIF_PMH], REG_PORT_ANY, 0, reg_above_64));

    /* polling on pll_locked */
    if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {
        DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, eci_srd_pll_status[pll_type - JER2_JER_PLL_TYPE_NIF_PMH], REG_PORT_ANY, 0, eci_srd_pll_locked[pll_type - JER2_JER_PLL_TYPE_NIF_PMH], 1));
    }

    /* post reset */
    DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, eci_srd_pll_config[pll_type - JER2_JER_PLL_TYPE_NIF_PMH], REG_PORT_ANY, 0, reg_above_64));
    SOC_REG_ABOVE_64_CLEAR(field_above_64);
    SOC_REG_ABOVE_64_CREATE_MASK(field_above_64, 1, 0);
    soc_reg_above_64_field_set(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_above_64, PMH_PLL_POST_RESETBf, field_above_64);
    DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, eci_srd_pll_config[pll_type - JER2_JER_PLL_TYPE_NIF_PMH], REG_PORT_ANY, 0, reg_above_64));

    if(is_bypass){
        
    }

exit:
    DNXC_FUNC_RETURN;
}
/*
 * PLL_2 configurations used by both pll and sync init for QUX
 */
static int qux_pll_2_set(
                    int                         unit,
                    QUX_NIF_PLL_TYPE            pll_type,
                    uint32                      ndiv,
                    uint32                      mdiv,
                    uint32                      icp,
                    uint32                      is_bypass)
{
    uint32 val;
    soc_reg_above_64_val_t reg_above_64, field_above_64;

    const static soc_reg_t
        eci_srd_pll_config[] = {ECI_NIF_PML_PLL_CONFIGr, ECI_NIF_PMX_PLL_CONFIGr};
    const static soc_reg_t
        eci_srd_pll_status[] = {ECI_PML_PLL_STATUSr, ECI_PMX_PLL_STATUSr};
    const static soc_field_t
        eci_srd_pll_locked[] = {MISC_PLL_4_LOCKEDf, MISC_PLL_5_LOCKEDf};

    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(reg_above_64);
    SOC_REG_ABOVE_64_CLEAR(field_above_64);

    /* CP */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_248_249f, field_above_64);

    /* CP_1 */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_155_156f, field_above_64);

    /* PDIV */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_6_9f, field_above_64);

    /* CH_0_MDIV */
    SHR_BITCOPY_RANGE(field_above_64,0,&mdiv,0,sizeof(mdiv)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_77_84f, field_above_64);

    /* CH_1_MDIV */
    val = 0x5;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_86_93f, field_above_64);

    /* CH_2_MDIV */
    val = ((pll_type == QUX_PLL_TYPE_NIF_PML ) ? 4 : 0x50);
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_95_102f, field_above_64);

    /* CH_4_MDIV */
    val = 0x7;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_113_120f, field_above_64);

    /* RZ  */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_168_172f, field_above_64);

    /* CZ */
    val = 3;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_157_158f, field_above_64);

    /* MSC_CTRL_VEC_REF_SEL */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_191_191f, field_above_64);

    /* MSC_CTRL_VEC_CML_EN */
    val = 3;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_196_197f, field_above_64);

    /* LDO_CTRL */
    val = 0x2a;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_173_188f, field_above_64);

    /* CTRL_VEC_CPP */
    val = 0x80;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_60_71f, field_above_64);

    /* CTRL_VEC_FREQ_DOUBL_DEL */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_58_59f, field_above_64);

    /* ICP */
    SHR_BITCOPY_RANGE(field_above_64,0,&icp,0,sizeof(icp)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_159_164f, field_above_64);

    /* RP */
    val = 3;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_165_167f, field_above_64);

    /* MSC_CTRL_VEC_TEST_BUF_EN */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_201_201f, field_above_64);

    /* FBDIV_NDIV_INT */

    SHR_BITCOPY_RANGE(field_above_64,0,&ndiv,0,sizeof(ndiv)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_294_303f, field_above_64);

    /* BYP_HOLDOVER */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_304_304f, field_above_64);

    /* MSC_CTRL_VEC_D_2C_BIAS */
    val = 4;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_192_194f, field_above_64);

    /* FBDIV_NDIV_INT */
    val = 3;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_55_56f, field_above_64);

    /* PWRON */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_132_132f, field_above_64);

    /* PWRON_LDO */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_133_133f, field_above_64);

    /* set config register with init values */
    DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, eci_srd_pll_config[pll_type - QUX_PLL_TYPE_NIF_PML], REG_PORT_ANY, 0, reg_above_64));

    sal_usleep(30);

    /* asserting the reset bit */
    DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, eci_srd_pll_config[pll_type - QUX_PLL_TYPE_NIF_PML], REG_PORT_ANY, 0, reg_above_64));
    SOC_REG_ABOVE_64_CLEAR(field_above_64);
    SOC_REG_ABOVE_64_CREATE_MASK(field_above_64, 1, 0);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_130_130f, field_above_64);
    DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, eci_srd_pll_config[pll_type - QUX_PLL_TYPE_NIF_PML], REG_PORT_ANY, 0, reg_above_64));

    /* polling on pll_locked */
    if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {
        DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, eci_srd_pll_status[pll_type - QUX_PLL_TYPE_NIF_PML], REG_PORT_ANY, 0, eci_srd_pll_locked[pll_type - QUX_PLL_TYPE_NIF_PML], 1));
    }

    /* post reset */
    DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, eci_srd_pll_config[pll_type - QUX_PLL_TYPE_NIF_PML], REG_PORT_ANY, 0, reg_above_64));
    SOC_REG_ABOVE_64_CLEAR(field_above_64);
    SOC_REG_ABOVE_64_CREATE_MASK(field_above_64, 1, 0);
    soc_reg_above_64_field_set(unit, ECI_NIF_PML_PLL_CONFIGr, reg_above_64, ITEM_131_131f, field_above_64);
    DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, eci_srd_pll_config[pll_type - QUX_PLL_TYPE_NIF_PML], REG_PORT_ANY, 0, reg_above_64));
    

exit:
    DNXC_FUNC_RETURN;
}

/*
 * PLL_3 configurations used by both pll and sync init
 */
static int jer2_jer_pll_1_set(
                    int                         unit,
                    JER2_JER_PLL_TYPE                pll_type)
{
    uint32 val;
    soc_reg_above_64_val_t reg_above_64, field_above_64;

    const static soc_reg_t
        eci_srd_pll_config[] = {ECI_TS_PLL_CONFIGr, ECI_BS_PLL_CONFIGr};
    const static soc_reg_t
        eci_srd_pll_status[] = {ECI_TS_PLL_STATUSr, ECI_BS_PLL_STATUSr};
    const static soc_field_t
        eci_srd_pll_locked[] = {MISC_PLL_0_LOCKEDf, MISC_PLL_1_LOCKEDf};

    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(reg_above_64);
    SOC_REG_ABOVE_64_CLEAR(field_above_64);

    /* ndiv */
    if(SOC_IS_QAX(unit)) {
        val = 120; /* Table-13, Qumran-doc.docx */
    } else {
        val = 140;
    }

    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_TS_PLL_CONFIGr, reg_above_64, TS_PLL_CFG_NDIVf, field_above_64);

    /* mdiv ch 0 */
    if (pll_type == JER2_JER_PLL_TYPE_TS) {
        if(SOC_IS_QAX(unit)) {
           val = 6; /* Table-13,Qumran-doc.docx */
        } else {
           val = 7;
        }
    } else {
        if(SOC_IS_QMX(unit) || SOC_IS_JERICHO(unit)) {
            val = 175; /*For QMX/JER2_JERICHO_a0/b0, BS-PLL out considered 20MHz*/
        } else {
            val = 140;
        }
    }
    SHR_BITCOPY_RANGE(field_above_64, 0, &val, 0, sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_TS_PLL_CONFIGr, reg_above_64, TS_PLL_CFG_CH_0_MDIVf, field_above_64);

    /* mdiv ch 1 */
    if(SOC_IS_QAX(unit)){
        val = 12;

        SHR_BITCOPY_RANGE(field_above_64, 0, &val, 0, sizeof(val)*8);
        soc_reg_above_64_field_set(unit, ECI_TS_PLL_CONFIGr, reg_above_64, TS_PLL_CFG_CH_1_MDIVf, field_above_64);
    }

    /* kp */
    val = 3;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_TS_PLL_CONFIGr, reg_above_64, TS_PLL_CFG_KPf, field_above_64);

    /* ki */
    val = 2;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_TS_PLL_CONFIGr, reg_above_64, TS_PLL_CFG_KIf, field_above_64);

    /* ka */
    val = 0;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_TS_PLL_CONFIGr, reg_above_64, TS_PLL_CFG_KAf, field_above_64);

    /* pdiv */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_TS_PLL_CONFIGr, reg_above_64, TS_PLL_CFG_PDIVf, field_above_64);

    /* control pwm_rate */
    val = 0;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_TS_PLL_CONFIGr, reg_above_64, TS_PLL_CFG_PLL_CTRL_PWM_RATEf, field_above_64);

    /* control post_rst_sel */
    val = 2;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_TS_PLL_CONFIGr, reg_above_64, TS_PLL_CFG_PLL_CTRL_POST_RESET_SELECTf, field_above_64);

    /* control vco_fb_div2 */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_TS_PLL_CONFIGr, reg_above_64, TS_PLL_CFG_PLL_CTRL_VCO_FB_DIV_2f, field_above_64);

    /* control vco_range */
    val = 2;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_TS_PLL_CONFIGr, reg_above_64, TS_PLL_CFG_PLL_CTRL_VCO_RANGEf, field_above_64);

    /* control ldo */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_TS_PLL_CONFIGr, reg_above_64, TS_PLL_CFG_PLL_CTRL_LDOf, field_above_64);

    /* pwron ldo*/
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_TS_PLL_CONFIGr, reg_above_64, TS_PLL_CFG_PWRON_LDOf, field_above_64);

    /*  external clock - (BS gets clock internally) */
    if (pll_type == JER2_JER_PLL_TYPE_TS) {
        soc_reg_above_64_field32_set(unit, ECI_TS_PLL_CONFIGr, reg_above_64, TS_PLL_CFG_PLL_CTRL_FREF_SELf, 1);
    }

    /* set config register with init values wo pwron */
    DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, eci_srd_pll_config[pll_type-JER2_JER_PLL_TYPE_TS], REG_PORT_ANY, 0, reg_above_64));

    sal_usleep(1);

    /* pwron */
    val = 1;
    SHR_BITCOPY_RANGE(field_above_64,0,&val,0,sizeof(val)*8);
    soc_reg_above_64_field_set(unit, ECI_TS_PLL_CONFIGr, reg_above_64, TS_PLL_CFG_PWRONf, field_above_64);

    /* set config register with init values with pwron */
    DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, eci_srd_pll_config[pll_type-JER2_JER_PLL_TYPE_TS], REG_PORT_ANY, 0, reg_above_64));

    sal_usleep(30);

    /* asserting the reset bit */
    DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, eci_srd_pll_config[pll_type-JER2_JER_PLL_TYPE_TS], REG_PORT_ANY, 0, reg_above_64));
    SOC_REG_ABOVE_64_CLEAR(field_above_64);
    SOC_REG_ABOVE_64_CREATE_MASK(field_above_64, 1, 0);
    if (pll_type == JER2_JER_PLL_TYPE_BS) {
        soc_reg_above_64_field_set(unit, ECI_BS_PLL_CONFIGr, reg_above_64, BS_PLL_CFG_RESET_Bf, field_above_64);
    } else {
        soc_reg_above_64_field_set(unit, ECI_TS_PLL_CONFIGr, reg_above_64, TS_PLL_CFG_RESET_Bf, field_above_64);
    }
    DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, eci_srd_pll_config[pll_type-JER2_JER_PLL_TYPE_TS], REG_PORT_ANY, 0, reg_above_64));

    /* polling on pll_locked */
    if (SOC_DNX_CONFIG(unit)->emulation_system == 0) {
        DNXC_IF_ERR_EXIT(soc_dnx_polling(unit, JER2_ARAD_TIMEOUT, JER2_ARAD_MIN_POLLS, eci_srd_pll_status[pll_type-JER2_JER_PLL_TYPE_TS], REG_PORT_ANY, 0, eci_srd_pll_locked[pll_type-JER2_JER_PLL_TYPE_TS], 1));
    }

    /* post reset */
    DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, eci_srd_pll_config[pll_type-JER2_JER_PLL_TYPE_TS], REG_PORT_ANY, 0, reg_above_64));
    SOC_REG_ABOVE_64_CLEAR(field_above_64);
    SOC_REG_ABOVE_64_CREATE_MASK(field_above_64, 1, 0);
    if (pll_type == JER2_JER_PLL_TYPE_BS) {
        soc_reg_above_64_field_set(unit, ECI_BS_PLL_CONFIGr, reg_above_64, BS_PLL_CFG_POST_RESET_Bf, field_above_64);
    } else {
        soc_reg_above_64_field_set(unit, ECI_TS_PLL_CONFIGr, reg_above_64, TS_PLL_CFG_POST_RESET_Bf, field_above_64);
    }
    DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, eci_srd_pll_config[pll_type-JER2_JER_PLL_TYPE_TS], REG_PORT_ANY, 0, reg_above_64));

exit:
    DNXC_FUNC_RETURN;
}
static int jer2_jer_pll_clk_div_get(int unit,
                                   soc_dnxc_init_serdes_ref_clock_t  ref_clock_in,
                                   soc_dnxc_init_serdes_ref_clock_t  ref_clock_out,
                                   uint32 *ndiv, uint32 *mdiv, uint32 *icp)
{
    DNXC_INIT_FUNC_DEFS;

    if (ref_clock_in == soc_dnxc_init_serdes_ref_clock_125) {
        *ndiv = 25;
        *icp = 19;
    } else if (ref_clock_in == soc_dnxc_init_serdes_ref_clock_156_25) {
        *ndiv = 20;
        *icp = 16;
    } else {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
    if (ref_clock_out == soc_dnxc_init_serdes_ref_clock_125) {
        *mdiv = 25;
    } else if (ref_clock_out == soc_dnxc_init_serdes_ref_clock_156_25) {
        *mdiv = 20;
    } else {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
exit:
    DNXC_FUNC_RETURN;

}
/*
 * Configurations
 */
static int jer2_jer_pll_2_3_init(
                    int                         unit,
                    JER2_JER_PLL_TYPE                pll_type,
                    soc_dnxc_init_serdes_ref_clock_t  ref_clock_in,
                    soc_dnxc_init_serdes_ref_clock_t  ref_clock_out)
{
    uint32 ndiv, mdiv, icp;
    uint32 pdiv = 1;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(
        jer2_jer_pll_clk_div_get(unit, ref_clock_in, ref_clock_out,
                            &ndiv, &mdiv, &icp));
    if (SOC_IS_QAX(unit)) {
        DNXC_IF_ERR_EXIT(jer2_jer_pll_2_set(unit, pll_type, ndiv, mdiv, icp, FALSE));
    } else {
        DNXC_IF_ERR_EXIT(jer2_jer_pll_3_set(unit, pll_type, ndiv, mdiv, pdiv, FALSE));
    }

exit:
    DNXC_FUNC_RETURN;
}
/*
 * Configurations
 */
static int qux_pll_2_init(
                    int                         unit,
                    QUX_NIF_PLL_TYPE            pll_type,
                    soc_dnxc_init_serdes_ref_clock_t  ref_clock_in,
                    soc_dnxc_init_serdes_ref_clock_t  ref_clock_out)
{
    uint32 ndiv, mdiv, icp;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(
        jer2_jer_pll_clk_div_get(unit, ref_clock_in, ref_clock_out,
                            &ndiv, &mdiv, &icp));

    DNXC_IF_ERR_EXIT(qux_pll_2_set(unit, pll_type, ndiv, mdiv, icp, FALSE));

exit:
    DNXC_FUNC_RETURN;
}

/*
 * syncE Configurations
 * outclock = vco * over_sampling_factor / SERDES_internal_devider (20/40) /  synce_devider / nif_devider / pll_p * pll_ndiv / pll_mdiv
 */
int jer2_jer_synce_config_set(int unit, int synce_index, soc_dnxc_init_serdes_ref_clock_t  ref_clock_out, soc_port_t port)
{
    uint32 ndiv = 0, mdiv = 0, pdiv = 0;
    uint32 reg_val;
    uint32 devide_value;
    
    /*JER2_ARAD_INIT_SYNCE        *info;*/
    uint32  is_bypass = -1;
    int is_falcon = 0;
    int speed;
    soc_port_if_t interface_type;
    portmod_access_get_params_t params;
    phymod_phy_access_t phy_access;
    int nof_phys;
    uint32 synce_div, synce_div_val;
    
    /*SOC_JER2_JER_NIF_PLL_TYPE pll_type;*/
    soc_reg_above_64_val_t reg_val_above_64;

    DNXC_INIT_FUNC_DEFS;

    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
    info = &(SOC_DNX_CONFIG(unit)->jer2_arad->init.synce);*
    DNXC_IF_ERR_EXIT(soc_jer2_jer_port_pll_type_get(unit, port, &pll_type));

    if (IS_SFI_PORT(unit, port)) {
        is_bypass = 1;
        is_falcon = TRUE;
    } else {
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_first_phy_port_get(unit, port, &phy));
        if((pll_type == SOC_JER2_JER_NIF_PLL_TYPE_PMH) ||
            (SOC_IS_JERICHO_PLUS_ONLY(unit) && (pll_type == SOC_JER2_JER_NIF_PLL_TYPE_PML1))) {
           is_falcon = TRUE;
        } else {
           is_falcon = FALSE;
        }
        is_bypass = 0;
    }
#endif 

    DNXC_IF_ERR_EXIT(portmod_access_get_params_t_init(unit, &params));

    params.phyn = 0;

    DNXC_IF_ERR_EXIT(phymod_phy_access_t_init(&phy_access));

    DNXC_IF_ERR_EXIT(portmod_port_phy_lane_access_get(unit, port, &params, 1, &phy_access, &nof_phys, NULL));
    if (nof_phys != 1) {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("SYNCE: unsupported nof_phys %d\n"), nof_phys));
    }

    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
    DNXC_IF_ERR_EXIT(soc_jer2_jer_portmod_calc_os(unit, &phy_access, &os_int, &os_remainder));
    if (os_int != 1 && os_int != 2 && os_int != 8) {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("SYNCE: unsupported os_int %d\n"), os_int));
    }
#endif 

#if defined PORTMOD_SUPPORT
    
    
    if (is_falcon) {
        BCMI_TSCE_XGXS_MAIN0_SYNCE_CTLr_t synce_ctrl;
        DNXC_IF_ERR_EXIT(BCMI_TSCE_XGXS_READ_MAIN0_SYNCE_CTLr(&phy_access.access, &synce_ctrl));

        if (phy_access.access.lane_mask & 0x1) {
            synce_div = BCMI_TSCE_XGXS_MAIN0_SYNCE_CTLr_SYNCE_MODE_PHY_LN0f_GET(synce_ctrl);
        } else if (phy_access.access.lane_mask & 0x2) {
            synce_div = BCMI_TSCE_XGXS_MAIN0_SYNCE_CTLr_SYNCE_MODE_PHY_LN1f_GET(synce_ctrl);
        } else if (phy_access.access.lane_mask & 0x4) {
            synce_div = BCMI_TSCE_XGXS_MAIN0_SYNCE_CTLr_SYNCE_MODE_PHY_LN2f_GET(synce_ctrl);
        } else if (phy_access.access.lane_mask & 0x8) {
            synce_div= BCMI_TSCE_XGXS_MAIN0_SYNCE_CTLr_SYNCE_MODE_PHY_LN3f_GET(synce_ctrl);
        } else {
            DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("ERROR: Wrong lane\n")));
        }
    } else {
        BCMI_TSCF_XGXS_MAIN0_SYNCE_CTLr_t synce_ctrl;
        DNXC_IF_ERR_EXIT(BCMI_TSCF_XGXS_READ_MAIN0_SYNCE_CTLr(&phy_access.access, &synce_ctrl));

        if (phy_access.access.lane_mask & 0x1) {
            synce_div = BCMI_TSCF_XGXS_MAIN0_SYNCE_CTLr_SYNCE_MODE_PHY_LANE0f_GET(synce_ctrl);
        } else if (phy_access.access.lane_mask & 0x2) {
            synce_div = BCMI_TSCF_XGXS_MAIN0_SYNCE_CTLr_SYNCE_MODE_PHY_LANE1f_GET(synce_ctrl);
        } else if (phy_access.access.lane_mask & 0x4) {
            synce_div = BCMI_TSCF_XGXS_MAIN0_SYNCE_CTLr_SYNCE_MODE_PHY_LANE2f_GET(synce_ctrl);
        } else if (phy_access.access.lane_mask & 0x8) {
            synce_div = BCMI_TSCF_XGXS_MAIN0_SYNCE_CTLr_SYNCE_MODE_PHY_LANE3f_GET(synce_ctrl);
        } else {
            DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("ERROR: Wrong lane\n")));
        }
    }
#endif /*PORTMOD_SUPPORT*/

    if (2 == synce_div) {
        synce_div_val = 11; /* default fout = 125MHz/25MHz considered, So fref = fvco/Ndiv = 23.4375MHz */
        /*synce_div_val = 9 ;  default fout = 156.25MHz considered, So fref = fvco/Ndiv = 28.645833MHz */
    } else if (1 == synce_div) {
        synce_div_val = 7;
    } else {
        synce_div_val = 1;
    }

    if (synce_div_val == 7) {
        DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("SYNCE: unsupported synce_div_val %d\n"), synce_div_val));
    }

    devide_value = 11 / synce_div_val;
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_speed_get(unit, port, &speed));

    if (!is_bypass) {
            DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_type_get(unit, port, &interface_type));
            if (!is_falcon) {

            switch (speed) {
            case 1000:
            case 10312:
            case 10000:
                /* Eagle 10.3125 GHZ */
                pdiv = 3;
                ndiv = 200;
                if (soc_dnxc_init_serdes_ref_clock_125 == ref_clock_out) {
                    mdiv = 25;
                } else if (soc_dnxc_init_serdes_ref_clock_156_25 == ref_clock_out) {
                    mdiv = 20;
                } else if (soc_dnxc_init_serdes_ref_clock_25 == ref_clock_out) {
                    mdiv = 125;
                } else {
                    DNXC_IF_ERR_EXIT(SOC_E_PARAM);
                }
                break;
            case 40000:
                /* Eagle 40000  GHZ */
                if (interface_type == SOC_PORT_IF_XLAUI) {
                    pdiv = 3;
                    ndiv = 200;
                    if (soc_dnxc_init_serdes_ref_clock_125 == ref_clock_out) {
                        mdiv = 25;
                    } else if (soc_dnxc_init_serdes_ref_clock_156_25 == ref_clock_out) {
                        mdiv = 20;
                    } else if (soc_dnxc_init_serdes_ref_clock_25 == ref_clock_out) {
                        mdiv = 125;
                    } else {
                        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
                    }
                    break;
                }
            default:
                DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("SYNCE: unsupported speed %d for port %d, interface_type %d\n"), speed, port, interface_type));
            }
        } else {

            switch (speed) {
            case 10000:
            case 10312:
                    /* Falcon 10.3125  GHZ */
                    pdiv = 1;
                    if (soc_dnxc_init_serdes_ref_clock_125 == ref_clock_out) {
                        mdiv = 24;
                        ndiv = 128;
                    } else if (soc_dnxc_init_serdes_ref_clock_156_25 == ref_clock_out) {
                          mdiv = 18;
                       /* mdiv = 22;  default fout=156.25MHz considered */
			  			  ndiv = 120;
                    } else if (soc_dnxc_init_serdes_ref_clock_25 == ref_clock_out) {
                        mdiv = 120;
                        ndiv = 128;
                    } else {
                        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
                    }
                    break;
            case 25000:
            case 25781:
            case 100000:
                /* Falcon 25.78125 GHZ */
                pdiv = 3;
                ndiv = 160;
                if (soc_dnxc_init_serdes_ref_clock_125 == ref_clock_out) {
                    mdiv = 25;
                } else if (soc_dnxc_init_serdes_ref_clock_156_25 == ref_clock_out) {
                    mdiv = 20;
                } else if (soc_dnxc_init_serdes_ref_clock_25 == ref_clock_out) {
                    mdiv = 125;
                } else {
                    DNXC_IF_ERR_EXIT(SOC_E_PARAM);
                }
                break;
            case 40000:
                /* Falcon 40000  GHZ */
                if (interface_type == SOC_PORT_IF_XLAUI2) {
                    pdiv = 1;
                    if (soc_dnxc_init_serdes_ref_clock_125 == ref_clock_out) {
                        mdiv = 24;
                        ndiv = 64;
                    } else if (soc_dnxc_init_serdes_ref_clock_156_25 == ref_clock_out) {
                        mdiv = 18;
                        ndiv = 60;
                    } else if (soc_dnxc_init_serdes_ref_clock_25 == ref_clock_out) {
                        mdiv = 120;
                        ndiv = 64;
                    } else {
                        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
                    }
                    break;
                }
 
                if (interface_type == SOC_PORT_IF_XLAUI) {
                    pdiv = 1; 
                    if (soc_dnxc_init_serdes_ref_clock_125 == ref_clock_out) {
                        mdiv = 24;
                        ndiv = 128;
                    } else if (soc_dnxc_init_serdes_ref_clock_156_25 == ref_clock_out) {
                        mdiv = 18;
                        ndiv = 120;
                    } else if (soc_dnxc_init_serdes_ref_clock_25 == ref_clock_out) {
                        mdiv = 120;
                        ndiv = 128;
                    } else {
                        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
                    }
                    break;
                }
            default:
                DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("SYNCE: unsupported speed %d for port %d, interface_type %d\n"), speed, port, interface_type));
            }
        }

        /* Configure first port serdes */
        
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACYf
        rv = dnx_handle_sand_result(soc_jer2_jer_port_synce_clk_sel_set(unit, synce_index, port));
        DNXC_IF_ERR_EXIT(rv);
#endif 

        /*Enable synce*/
        DNXC_IF_ERR_EXIT(READ_ECI_GP_CONTROL_9r(unit, reg_val_above_64));
        soc_reg_above_64_field32_set(unit, ECI_GP_CONTROL_9r, reg_val_above_64, PMH_SYNCE_RSTNf, 0x1);
        DNXC_IF_ERR_EXIT(WRITE_ECI_GP_CONTROL_9r(unit, reg_val_above_64));
        
        /* NBI configuration */
        if (!SOC_IS_QUX(unit)) {
            
            DNXC_IF_ERR_EXIT(READ_NBIH_SYNC_ETH_CFGr(unit, synce_index, &reg_val));
            if (devide_value != 1) {
                /* Configure Clock divider */
                soc_reg_field_set(unit, NBIH_SYNC_ETH_CFGr, &reg_val, SYNC_ETH_CLOCK_DIV_Nf, 2);
                soc_reg_field_set(unit, NBIH_SYNC_ETH_CFGr, &reg_val, SYNC_ETH_DIVIDER_PHASE_ZERO_Nf, ((devide_value+1)/2) - 1);
                soc_reg_field_set(unit, NBIH_SYNC_ETH_CFGr, &reg_val, SYNC_ETH_DIVIDER_PHASE_ONE_Nf, (devide_value/2) - 1);
            } else {
                soc_reg_field_set(unit, NBIH_SYNC_ETH_CFGr, &reg_val, SYNC_ETH_CLOCK_DIV_Nf, 1);
            }
    
            /* Configure Squelch mode */
            
            DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
            soc_reg_field_set(unit, NBIH_SYNC_ETH_CFGr, &reg_val, SYNC_ETH_SQUELCH_EN_Nf, info->conf[synce_index].squelch_enable);
#endif 
    
            DNXC_IF_ERR_EXIT(WRITE_NBIH_SYNC_ETH_CFGr(unit, synce_index, reg_val));
        }
    }

    /* pll configuration */
    if (SOC_IS_JERICHO_PLUS_ONLY(unit)) {
        DNXC_IF_ERR_EXIT(jer2_jer_plus_pll_set(unit, synce_index, ndiv, mdiv, pdiv, is_bypass));
    }else {
        DNXC_IF_ERR_EXIT(jer2_jer_pll_3_set(unit, synce_index + JER2_JER_PLL_TYPE_SYNCE_0, ndiv, mdiv, pdiv, is_bypass));
    }

exit:

    DNXC_FUNC_RETURN;
}

uint32 jer2_jer_synce_clk_div_set(int unit, uint32 synce_idx, JER2_JER_PLL_NIF_SYNCE_CLK_DIV   clk_div)
{
    DNXC_INIT_FUNC_DEFS;

    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
    DNXC_IF_ERR_EXIT(soc_jer2_jer_port_synce_clk_sel_get(unit, synce_idx, &port));
    DNXC_IF_ERR_EXIT(jer2_jer_synce_config_set(unit, synce_idx, (soc_dnxc_init_serdes_ref_clock_t)clk_div, port));
#endif 

    DNXC_FUNC_RETURN;
}

uint32 jer2_jer_synce_clk_div_get(int unit, uint32 synce_idx, JER2_JER_PLL_NIF_SYNCE_CLK_DIV*   clk_div)
{
   uint32 mdiv;
   soc_dnxc_init_serdes_ref_clock_t ref_clock_out;

   DNXC_INIT_FUNC_DEFS;

   DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_read(unit, ECI_NIF_PMH_PLL_CONFIGr, REG_PORT_ANY, 0, SOC_IS_QAX(unit) ? PMH_PLL_CH_0_MDIVf : NIF_PMH_PLL_CFG_CH_0_MDIVf, &mdiv));
   switch (mdiv) {
   case 22:
   case 20:
       ref_clock_out = soc_dnxc_init_serdes_ref_clock_156_25;
       break;
   case 24:
   case 25:
       ref_clock_out = soc_dnxc_init_serdes_ref_clock_125;
       break;
   case 125:
   case 120:
       ref_clock_out = soc_dnxc_init_serdes_ref_clock_25;
       break;
   default:
       DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("SYNCE: invalid value for mdiv %d\n"), mdiv));
   }

   *clk_div = (soc_dnxc_init_serdes_ref_clock_t)ref_clock_out;
exit:
    DNXC_FUNC_RETURN;
}

uint32 jer2_jer_synce_clk_port_sel_set(int unit, uint32 synce_idx, soc_port_t port)
{
   soc_dnxc_init_serdes_ref_clock_t  ref_clock_out;
   JER2_JER_PLL_NIF_SYNCE_CLK_DIV synce_div;

   DNXC_INIT_FUNC_DEFS;

   DNXC_IF_ERR_EXIT(jer2_jer_synce_clk_div_get(unit, synce_idx, &synce_div));
   ref_clock_out = (JER2_JER_PLL_NIF_SYNCE_CLK_DIV)synce_div;
   DNXC_IF_ERR_EXIT(jer2_jer_synce_config_set(unit, synce_idx, ref_clock_out, port));

exit:
   DNXC_FUNC_RETURN;
}

/*
 * syncE init function
 */
int jer2_jer_synce_init(int unit)
{
    uint32 reg_val;
    int synce_index;
    soc_dnxc_init_serdes_ref_clock_t  ref_clock_out;
    soc_port_t port;
    
    /*JER2_ARAD_INIT_SYNCE        *info;*/
    int is_master;

    DNXC_INIT_FUNC_DEFS;

    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
    info = &(SOC_DNX_CONFIG(unit)->jer2_arad->init.synce);
#endif 

    for (synce_index = 0; synce_index <= JER2_JER_PLL_TYPE_SYNCE_1 - JER2_JER_PLL_TYPE_SYNCE_0; synce_index++) {
        

        if (/*info->conf[synce_index].enable*/ 0) {
            ref_clock_out = SOC_DNX_CONFIG(unit)->jer2_jer->pll.ref_clk_synce_out[synce_index];
            
            DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
            port = info->conf[synce_index].port_id;
#endif 
            if (!IS_SFI_PORT(unit, port))
            {
                DNXC_IF_ERR_EXIT(jer2_jer_synce_config_set(unit, synce_index, ref_clock_out, port));
                is_master = synce_index? 0 : 1;
                DNXC_IF_ERR_EXIT(soc_jer2_jer_fabric_sync_e_enable_set(unit, is_master, 0));
            }
        }
    }

    /* Configure Pads */
    reg_val = 0x0;
    soc_reg_field_set(unit, ECI_SELECT_OUTPUT_OF_SYNCHRONOUS_ETHERNET_PADSr, &reg_val, SYNC_ETH_PAD_0_OE_Nf, 0x0 /* Output */);
    soc_reg_field_set(unit, ECI_SELECT_OUTPUT_OF_SYNCHRONOUS_ETHERNET_PADSr, &reg_val, SYNC_ETH_PAD_1_OE_Nf, 0x0 /* Output */);
    soc_reg_field_set(unit, ECI_SELECT_OUTPUT_OF_SYNCHRONOUS_ETHERNET_PADSr, &reg_val, SYNC_ETH_PAD_2_OE_Nf, 0x0 /* Output */);
    soc_reg_field_set(unit, ECI_SELECT_OUTPUT_OF_SYNCHRONOUS_ETHERNET_PADSr, &reg_val, SYNC_ETH_PAD_3_OE_Nf, 0x0 /* Output */);

    
    DNXC_LEGACY_FIXME_ASSERT;
    if (/*info->mode == JER2_ARAD_NIF_SYNCE_MODE_TWO_DIFF_CLK*/ 0) {
        soc_reg_field_set(unit, ECI_SELECT_OUTPUT_OF_SYNCHRONOUS_ETHERNET_PADSr, &reg_val, SYNC_ETH_PAD_0_SELECTf, 0x0 /* clk 0 (P)*/);
        soc_reg_field_set(unit, ECI_SELECT_OUTPUT_OF_SYNCHRONOUS_ETHERNET_PADSr, &reg_val, SYNC_ETH_PAD_1_SELECTf, 0x0 /* clk 0 (N) */);
        soc_reg_field_set(unit, ECI_SELECT_OUTPUT_OF_SYNCHRONOUS_ETHERNET_PADSr, &reg_val, SYNC_ETH_PAD_2_SELECTf, 0x1 /* clk 1 (P) */);
        soc_reg_field_set(unit, ECI_SELECT_OUTPUT_OF_SYNCHRONOUS_ETHERNET_PADSr, &reg_val, SYNC_ETH_PAD_3_SELECTf, 0x1 /* clk 1 (N) */);
        
    } else if (/*info->mode == JER2_ARAD_NIF_SYNCE_MODE_TWO_CLK_AND_VALID*/ 0) {
        soc_reg_field_set(unit, ECI_SELECT_OUTPUT_OF_SYNCHRONOUS_ETHERNET_PADSr, &reg_val, SYNC_ETH_PAD_0_SELECTf, 0x0 /* clk 0 (P)*/);
        soc_reg_field_set(unit, ECI_SELECT_OUTPUT_OF_SYNCHRONOUS_ETHERNET_PADSr, &reg_val, SYNC_ETH_PAD_1_SELECTf, 0x4 /* valid 0 */);
        soc_reg_field_set(unit, ECI_SELECT_OUTPUT_OF_SYNCHRONOUS_ETHERNET_PADSr, &reg_val, SYNC_ETH_PAD_2_SELECTf, 0x1 /* clk 1 (P) */);
        soc_reg_field_set(unit, ECI_SELECT_OUTPUT_OF_SYNCHRONOUS_ETHERNET_PADSr, &reg_val, SYNC_ETH_PAD_3_SELECTf, 0x5 /* valid 1 */);
    }
    DNXC_IF_ERR_EXIT(WRITE_ECI_SELECT_OUTPUT_OF_SYNCHRONOUS_ETHERNET_PADSr(unit, reg_val));

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Init Jericho plls including Synce
 */
int jer2_jer_pll_init(int unit)
{
    soc_jer2_jer_pll_config_t        pll;
    DNXC_INIT_FUNC_DEFS;

    pll = SOC_DNX_CONFIG(unit)->jer2_jer->pll;

    if (SOC_IS_QUX(unit)) {
        if (pll.ref_clk_pml_in[0] != JER2_ARAD_INIT_SERDES_REF_CLOCK_DISABLE && pll.ref_clk_pml_out[0] != JER2_ARAD_INIT_SERDES_REF_CLOCK_DISABLE) {
            DNXC_IF_ERR_EXIT(qux_pll_2_init(unit, QUX_PLL_TYPE_NIF_PML, pll.ref_clk_pml_in[0], pll.ref_clk_pml_out[0]));
        }
        if (pll.ref_clk_pmx_in != JER2_ARAD_INIT_SERDES_REF_CLOCK_DISABLE && pll.ref_clk_pmx_out != JER2_ARAD_INIT_SERDES_REF_CLOCK_DISABLE) {
            DNXC_IF_ERR_EXIT(qux_pll_2_init(unit, QUX_PLL_TYPE_NIF_PMX, pll.ref_clk_pmx_in, pll.ref_clk_pmx_out));
        }
    } else {
        if (pll.ref_clk_pml_in[0] != JER2_ARAD_INIT_SERDES_REF_CLOCK_DISABLE && pll.ref_clk_pml_out[0] != JER2_ARAD_INIT_SERDES_REF_CLOCK_DISABLE) {
            DNXC_IF_ERR_EXIT(jer2_jer_pll_2_3_init(unit, JER2_JER_PLL_TYPE_NIF_PML_0, pll.ref_clk_pml_in[0], pll.ref_clk_pml_out[0]));
        }
        if (pll.ref_clk_pml_in[1] != JER2_ARAD_INIT_SERDES_REF_CLOCK_DISABLE && pll.ref_clk_pml_out[1] != JER2_ARAD_INIT_SERDES_REF_CLOCK_DISABLE) {
            DNXC_IF_ERR_EXIT(jer2_jer_pll_2_3_init(unit, JER2_JER_PLL_TYPE_NIF_PML_1, pll.ref_clk_pml_in[1], pll.ref_clk_pml_out[1]));
        }
        if (pll.ref_clk_pmh_in != JER2_ARAD_INIT_SERDES_REF_CLOCK_DISABLE && pll.ref_clk_pmh_out != JER2_ARAD_INIT_SERDES_REF_CLOCK_DISABLE) {
            DNXC_IF_ERR_EXIT(jer2_jer_pll_2_3_init(unit, JER2_JER_PLL_TYPE_NIF_PMH, pll.ref_clk_pmh_in, pll.ref_clk_pmh_out));
        }
    }
    /* ts */
    if (SOC_DNX_CONFIG(unit)->jer2_arad->init.pll.ts_clk_mode == 0x1) {
        DNXC_IF_ERR_EXIT(jer2_jer_pll_1_set(unit, JER2_JER_PLL_TYPE_TS));
    }
    if (!SOC_IS_QAX(unit)) {
        if (pll.ref_clk_fabric_in[0] != JER2_ARAD_INIT_SERDES_REF_CLOCK_DISABLE) {
            DNXC_IF_ERR_EXIT(jer2_jer_pll_2_3_init(unit, JER2_JER_PLL_TYPE_FABRIC_0, pll.ref_clk_fabric_in[0], pll.ref_clk_fabric_out[0]));
        }
        if (pll.ref_clk_fabric_in[1] != JER2_ARAD_INIT_SERDES_REF_CLOCK_DISABLE)
        {
            DNXC_IF_ERR_EXIT(jer2_jer_pll_2_3_init(unit, JER2_JER_PLL_TYPE_FABRIC_1, pll.ref_clk_fabric_in[1], pll.ref_clk_fabric_out[1]));
        }
        /* bs */
        if (SOC_DNX_CONFIG(unit)->jer2_arad->init.pll.bs_clk_mode == 0x1) {
            DNXC_IF_ERR_EXIT(jer2_jer_pll_1_set(unit, JER2_JER_PLL_TYPE_BS));
        }

    }else {
        /* bs */
        if (SOC_DNX_CONFIG(unit)->jer2_arad->init.pll.bs_clk_mode == 0x1) {
            DNXC_IF_ERR_EXIT(jer2_jer_pll_4_set(unit, JER2_QAX_PLL_TYPE_BSPLL1));
	    sal_usleep(30);
	    DNXC_IF_ERR_EXIT(jer2_jer_pll_4_set(unit, JER2_QAX_PLL_TYPE_BSPLL0));

        }
    }

exit:
    DNXC_FUNC_RETURN;
}
