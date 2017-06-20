/*
 * $Id: robo.c,v 1.65 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        robo.c
 * Purpose:
 * Requires:
 */
#include <shared/bsl.h>

#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/robo/mcm/driver.h>
#include <soc/error.h>

#include <soc/debug.h>

#ifdef BCM_ROBO_SUPPORT

/* Will be added and changed later */
/*
 * Robo chip driver functions.  Common across Robo devices for now.
 * These may get broken out by chip in the future, but not needed yet.
 */
soc_functions_t soc_robo_drv_funs = {
    soc_robo_misc_init,
    soc_robo_mmu_init,
    soc_robo_age_timer_get,
    soc_robo_age_timer_max_get,
    soc_robo_age_timer_set
};

int bcm53222_attached = 0;


/*
 * soc_robo_53242_mmu_default_set():
 *   - Do 53242 MMU default Configuration.
 */
int
soc_robo_53242_mmu_default_set(int unit) 
{
    uint32          temp;
    uint32          reg_value = 0;

    /* FCON_Q0_100_TH_CTRL_1r: Page Offset = 0x0A 0x06 */
    SOC_IF_ERROR_RETURN(REG_READ_FCON_Q0_100_TH_CTRL_1r(unit, &reg_value));

    temp = 0x13;
    SOC_IF_ERROR_RETURN(soc_FCON_Q0_100_TH_CTRL_1r_field_set
        (unit, &reg_value, BT100_HYST_THRSf, &temp));
    temp = 0x1c;
    SOC_IF_ERROR_RETURN(soc_FCON_Q0_100_TH_CTRL_1r_field_set
        (unit, &reg_value, BT100_PAUS_THRSf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_FCON_Q0_100_TH_CTRL_1r(unit, &reg_value));

    /* FCON_Q0_100_TH_CTRL_2r: Page Offset = 0x0A 0x08 */
    SOC_IF_ERROR_RETURN(REG_READ_FCON_Q0_100_TH_CTRL_2r(unit, &reg_value));

    temp = 0x98;
    SOC_IF_ERROR_RETURN(soc_FCON_Q0_100_TH_CTRL_2r_field_set
        (unit, &reg_value, BT100_DROP_THRSf, &temp));
    temp = 0x73;
    SOC_IF_ERROR_RETURN(soc_FCON_Q0_100_TH_CTRL_2r_field_set
        (unit, &reg_value, BT100_MCDROP_THRSf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_FCON_Q0_100_TH_CTRL_2r(unit, &reg_value));

    /* FCON_GLOB_TH_CTRL_1r: Page Offset = 0x0A 0x0E */
    SOC_IF_ERROR_RETURN(REG_READ_FCON_GLOB_TH_CTRL_1r(unit, &reg_value));

    temp = 0x44;
    SOC_IF_ERROR_RETURN(soc_FCON_GLOB_TH_CTRL_1r_field_set
        (unit, &reg_value, FCON_GLOB_HYST_THf, &temp));
    temp = 0x7e;
    SOC_IF_ERROR_RETURN(soc_FCON_GLOB_TH_CTRL_1r_field_set
        (unit, &reg_value, FCON_GLOB_PAUSE_THf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_FCON_GLOB_TH_CTRL_1r(unit, &reg_value));

    /* FCON_GLOB_TH_CTRL_2r: Page Offset = 0x0A 0x10 */
    SOC_IF_ERROR_RETURN(REG_READ_FCON_GLOB_TH_CTRL_2r(unit, &reg_value));

    temp = 0x9b;
    SOC_IF_ERROR_RETURN(soc_FCON_GLOB_TH_CTRL_2r_field_set
        (unit, &reg_value, FCON_GLOB_DROP_THf, &temp));
    temp = 0x7e;
    SOC_IF_ERROR_RETURN(soc_FCON_GLOB_TH_CTRL_2r_field_set
        (unit, &reg_value, FCON_GLOB_MCDROP_THf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_FCON_GLOB_TH_CTRL_2r(unit, &reg_value));

    /* FCON_FLOWMIXr: Page Offset = 0x0A 0x30 */
    SOC_IF_ERROR_RETURN(REG_READ_FCON_FLOWMIXr(unit, &reg_value));

    temp = 0x3;
    SOC_IF_ERROR_RETURN(soc_FCON_FLOWMIXr_field_set
        (unit, &reg_value, QOS_RSRV_QUOTA_OPTf, &temp));
    temp = 0x1;
    SOC_IF_ERROR_RETURN(soc_FCON_FLOWMIXr_field_set
        (unit, &reg_value, EN_PARKING_PREVENTIONf, &temp));
    SOC_IF_ERROR_RETURN(soc_FCON_FLOWMIXr_field_set
        (unit, &reg_value, EN_MCAST_BLANCEf, &temp));
    SOC_IF_ERROR_RETURN(soc_FCON_FLOWMIXr_field_set
        (unit, &reg_value, EN_MCAST_DROPf, &temp));
    SOC_IF_ERROR_RETURN(soc_FCON_FLOWMIXr_field_set
        (unit, &reg_value, EN_UCAST_DROPf, &temp));
    SOC_IF_ERROR_RETURN(soc_FCON_FLOWMIXr_field_set
        (unit, &reg_value, EN_TXQ_PAUSEf, &temp));
    temp = 0x0;
    SOC_IF_ERROR_RETURN(soc_FCON_FLOWMIXr_field_set
        (unit, &reg_value, EN_RX_DROPf, &temp));
    SOC_IF_ERROR_RETURN(soc_FCON_FLOWMIXr_field_set
        (unit, &reg_value, EN_RX_PAUSEf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_FCON_FLOWMIXr(unit, &reg_value));

    /* FCON_MISC_TXFLOW_CTRLr: Page Offset = 0x0A 0x4A */
    SOC_IF_ERROR_RETURN(REG_READ_FCON_MISC_TXFLOW_CTRLr(unit, &reg_value));

    temp = 0x1;
    SOC_IF_ERROR_RETURN(soc_FCON_MISC_TXFLOW_CTRLr_field_set
        (unit, &reg_value, RESERVE_BLANCEf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_FCON_MISC_TXFLOW_CTRLr(unit, &reg_value));

    /* FCON_Q1_100_TH_CTRL_1r: Page Offset = 0x0A 0x6A */
    SOC_IF_ERROR_RETURN(REG_READ_FCON_Q1_100_TH_CTRL_1r(unit, &reg_value));

    temp = 0x13;
    SOC_IF_ERROR_RETURN(soc_FCON_Q1_100_TH_CTRL_1r_field_set
        (unit, &reg_value, BT100_HYST_THRSf, &temp));
    temp = 0x1c;
    SOC_IF_ERROR_RETURN(soc_FCON_Q1_100_TH_CTRL_1r_field_set
        (unit, &reg_value, BT100_PAUS_THRSf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_FCON_Q1_100_TH_CTRL_1r(unit, &reg_value));

    /* FCON_Q1_100_TH_CTRL_2r: Page Offset = 0x0A 0x6C */
    SOC_IF_ERROR_RETURN(REG_READ_FCON_Q1_100_TH_CTRL_2r(unit, &reg_value));

    temp = 0x98;
    SOC_IF_ERROR_RETURN(soc_FCON_Q1_100_TH_CTRL_2r_field_set
        (unit, &reg_value, BT100_DROP_THRSf, &temp));
    temp = 0x73;
    SOC_IF_ERROR_RETURN(soc_FCON_Q1_100_TH_CTRL_2r_field_set
        (unit, &reg_value, BT100_MCDROP_THRSf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_FCON_Q1_100_TH_CTRL_2r(unit, &reg_value));

    /* FCON_Q2_100_TH_CTRL_1r: Page Offset = 0x0A 0x78 */
    SOC_IF_ERROR_RETURN(REG_READ_FCON_Q2_100_TH_CTRL_1r(unit, &reg_value));

    temp = 0x13;
    SOC_IF_ERROR_RETURN(soc_FCON_Q2_100_TH_CTRL_1r_field_set
        (unit, &reg_value, BT100_HYST_THRSf, &temp));
    temp = 0x1c;
    SOC_IF_ERROR_RETURN(soc_FCON_Q2_100_TH_CTRL_1r_field_set
        (unit, &reg_value, BT100_PAUS_THRSf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_FCON_Q2_100_TH_CTRL_1r(unit, &reg_value));

    /* FCON_Q2_100_TH_CTRL_2r: Page Offset = 0x0A 0x7A */
    SOC_IF_ERROR_RETURN(REG_READ_FCON_Q2_100_TH_CTRL_2r(unit, &reg_value));

    temp = 0x98;
    SOC_IF_ERROR_RETURN(soc_FCON_Q2_100_TH_CTRL_2r_field_set
        (unit, &reg_value, BT100_DROP_THRSf, &temp));
    temp = 0x73;
    SOC_IF_ERROR_RETURN(soc_FCON_Q2_100_TH_CTRL_2r_field_set
        (unit, &reg_value, BT100_MCDROP_THRSf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_FCON_Q2_100_TH_CTRL_2r(unit, &reg_value));

    /* FCON_Q3_100_TH_CTRL_1r: Page Offset = 0x0A 0x86 */
    SOC_IF_ERROR_RETURN(REG_READ_FCON_Q3_100_TH_CTRL_1r(unit, &reg_value));

    temp = 0x13;
    SOC_IF_ERROR_RETURN(soc_FCON_Q3_100_TH_CTRL_1r_field_set
        (unit, &reg_value, BT100_HYST_THRSf, &temp));
    temp = 0x1c;
    SOC_IF_ERROR_RETURN(soc_FCON_Q3_100_TH_CTRL_1r_field_set
        (unit, &reg_value, BT100_PAUS_THRSf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_FCON_Q3_100_TH_CTRL_1r(unit, &reg_value));

    /* FCON_Q3_100_TH_CTRL_2r: Page Offset = 0x0A 0x88 */
    SOC_IF_ERROR_RETURN(REG_READ_FCON_Q3_100_TH_CTRL_2r(unit, &reg_value));

    temp = 0x98;
    SOC_IF_ERROR_RETURN(soc_FCON_Q3_100_TH_CTRL_2r_field_set
        (unit, &reg_value, BT100_DROP_THRSf, &temp));
    temp = 0x73;
    SOC_IF_ERROR_RETURN(soc_FCON_Q3_100_TH_CTRL_2r_field_set
        (unit, &reg_value, BT100_MCDROP_THRSf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_FCON_Q3_100_TH_CTRL_2r(unit, &reg_value));

    /* FCON_RX_FCON_CTRLr: Page Offset = 0x0A 0x92 */
    SOC_IF_ERROR_RETURN(REG_READ_FCON_RX_FCON_CTRLr(unit, &reg_value));

    temp = 0x1;
    SOC_IF_ERROR_RETURN(soc_FCON_RX_FCON_CTRLr_field_set
        (unit, &reg_value, EN_UNPAUSE_HDLf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_FCON_RX_FCON_CTRLr(unit, &reg_value));

    /* FCON_DLF_TH_CTRLr: Page Offset = 0x0A 0x94 */
    SOC_IF_ERROR_RETURN(REG_READ_FCON_DLF_TH_CTRLr(unit, &reg_value));

    temp = 0x7e;
    SOC_IF_ERROR_RETURN(soc_FCON_DLF_TH_CTRLr_field_set
        (unit, &reg_value, TOTAL_INDV_DLFTH_DROPf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_FCON_DLF_TH_CTRLr(unit, &reg_value));

    /* FCON_BCST_TH_CTRLr: Page Offset = 0x0A 0x96 */
    SOC_IF_ERROR_RETURN(REG_READ_FCON_BCST_TH_CTRLr(unit, &reg_value));

    temp = 0x7e;
    SOC_IF_ERROR_RETURN(soc_FCON_BCST_TH_CTRLr_field_set
        (unit, &reg_value, TOTAL_INDV_BCSTTH_DROPf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_FCON_BCST_TH_CTRLr(unit, &reg_value));

    /* TOTAL_HYST_THRESH_Q1r: Page Offset = 0x0A 0xC0 */
    SOC_IF_ERROR_RETURN(REG_READ_TOTAL_HYST_THRESH_Q1r(unit, &reg_value));

    temp = 0x46;
    SOC_IF_ERROR_RETURN(soc_TOTAL_HYST_THRESH_Q1r_field_set
        (unit, &reg_value, TL_HYST_TH_Q1f, &temp));
    temp = 0x80;
    SOC_IF_ERROR_RETURN(soc_TOTAL_HYST_THRESH_Q1r_field_set
        (unit, &reg_value, TL_PAUSE_TH_Q1f, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_TOTAL_HYST_THRESH_Q1r(unit, &reg_value));

    /* TOTAL_DROP_THRESH_Q1r: Page Offset = 0x0A 0xC2 */
    SOC_IF_ERROR_RETURN(REG_READ_TOTAL_DROP_THRESH_Q1r(unit, &reg_value));

    temp = 0x9d;
    SOC_IF_ERROR_RETURN(soc_TOTAL_DROP_THRESH_Q1r_field_set
        (unit, &reg_value, TL_DROP_TH_Q1f, &temp));
    temp = 0x7e;
    SOC_IF_ERROR_RETURN(soc_TOTAL_DROP_THRESH_Q1r_field_set
        (unit, &reg_value, RESERVED_Rf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_TOTAL_DROP_THRESH_Q1r(unit, &reg_value));

    /* TOTAL_HYST_THRESH_Q2r: Page Offset = 0x0A 0xC4 */
    SOC_IF_ERROR_RETURN(REG_READ_TOTAL_HYST_THRESH_Q2r(unit, &reg_value));

    temp = 0x48;
    SOC_IF_ERROR_RETURN(soc_TOTAL_HYST_THRESH_Q2r_field_set
        (unit, &reg_value, TL_HYST_TH_Q2f, &temp));
    temp = 0x82;
    SOC_IF_ERROR_RETURN(soc_TOTAL_HYST_THRESH_Q2r_field_set
        (unit, &reg_value, TL_PAUSE_TH_Q2f, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_TOTAL_HYST_THRESH_Q2r(unit, &reg_value));

    /* TOTAL_DROP_THRESH_Q2r: Page Offset = 0x0A 0xC6 */
    SOC_IF_ERROR_RETURN(REG_READ_TOTAL_DROP_THRESH_Q2r(unit, &reg_value));

    temp = 0x9f;
    SOC_IF_ERROR_RETURN(soc_TOTAL_DROP_THRESH_Q2r_field_set
        (unit, &reg_value, TL_DROP_TH_Q2f, &temp));
    temp = 0x7e;
    SOC_IF_ERROR_RETURN(soc_TOTAL_DROP_THRESH_Q2r_field_set
        (unit, &reg_value, RESERVED_Rf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_TOTAL_DROP_THRESH_Q2r(unit, &reg_value));

    /* TOTAL_HYST_THRESH_Q3r: Page Offset = 0x0A 0xC8 */
    SOC_IF_ERROR_RETURN(REG_READ_TOTAL_HYST_THRESH_Q3r(unit, &reg_value));

    temp = 0x4a;
    SOC_IF_ERROR_RETURN(soc_TOTAL_HYST_THRESH_Q3r_field_set
        (unit, &reg_value, TL_HYST_TH_Q3f, &temp));
    temp = 0x84;
    SOC_IF_ERROR_RETURN(soc_TOTAL_HYST_THRESH_Q3r_field_set
        (unit, &reg_value, TL_PAUSE_TH_Q3f, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_TOTAL_HYST_THRESH_Q3r(unit, &reg_value));

    /* TOTAL_DROP_THRESH_Q3r: Page Offset = 0x0A 0xCA */
    SOC_IF_ERROR_RETURN(REG_READ_TOTAL_DROP_THRESH_Q3r(unit, &reg_value));

    temp = 0xa1;
    SOC_IF_ERROR_RETURN(soc_TOTAL_DROP_THRESH_Q3r_field_set
        (unit, &reg_value, TL_DROP_TH_Q3f, &temp));
    temp = 0x7e;
    SOC_IF_ERROR_RETURN(soc_TOTAL_DROP_THRESH_Q3r_field_set
        (unit, &reg_value, RESERVED_Rf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_TOTAL_DROP_THRESH_Q3r(unit, &reg_value));

    /* TOTAL_DLF_DROP_THRESH_Q1r: Page Offset = 0x0A 0xD0 */
    SOC_IF_ERROR_RETURN(REG_READ_TOTAL_DLF_DROP_THRESH_Q1r(unit, &reg_value));

    temp = 0x7e;
    SOC_IF_ERROR_RETURN(soc_TOTAL_DLF_DROP_THRESH_Q1r_field_set
        (unit, &reg_value, TOTAL_DLF_DROP_THRESH_Q1f, &temp));
    SOC_IF_ERROR_RETURN(soc_TOTAL_DLF_DROP_THRESH_Q1r_field_set
        (unit, &reg_value, TOTAL_BC_DROP_THRESH_Q1f, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_TOTAL_DLF_DROP_THRESH_Q1r(unit, &reg_value));

    /* TOTAL_DLF_DROP_THRESH_Q2r: Page Offset = 0x0A 0xD2 */
    SOC_IF_ERROR_RETURN(REG_READ_TOTAL_DLF_DROP_THRESH_Q2r(unit, &reg_value));

    temp = 0x7e;
    SOC_IF_ERROR_RETURN(soc_TOTAL_DLF_DROP_THRESH_Q2r_field_set
        (unit, &reg_value, TOTAL_DLF_DROP_THRESH_Q2f, &temp));
    SOC_IF_ERROR_RETURN(soc_TOTAL_DLF_DROP_THRESH_Q2r_field_set
        (unit, &reg_value, TOTAL_BC_DROP_THRESH_Q2f, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_TOTAL_DLF_DROP_THRESH_Q2r(unit, &reg_value));

    /* TOTAL_DLF_DROP_THRESH_Q3r: Page Offset = 0x0A 0xD4 */
    SOC_IF_ERROR_RETURN(REG_READ_TOTAL_DLF_DROP_THRESH_Q3r(unit, &reg_value));

    temp = 0x7e;
    SOC_IF_ERROR_RETURN(soc_TOTAL_DLF_DROP_THRESH_Q3r_field_set
        (unit, &reg_value, TOTAL_DLF_DROP_THRESH_Q3f, &temp));
    SOC_IF_ERROR_RETURN(soc_TOTAL_DLF_DROP_THRESH_Q3r_field_set
        (unit, &reg_value, TOTAL_BC_DROP_THRESH_Q3f, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_TOTAL_DLF_DROP_THRESH_Q3r(unit, &reg_value));

    return SOC_E_NONE;
}

/* soc_misc_init() :
 *  - allowed user to do the init by chip dependant configuration.
 *
 *  Note : 
 *   1. below routine is for all Roob chip related init routine.
 *   2. different robo chip init section may separated by 
 *      "SOC_IS_ROBO53xx(unit)"
 */
int
soc_robo_misc_init(int unit)
{
    uint32          temp;
    uint32          reg_value = 0;
    int             rv = SOC_E_NONE;
    soc_pbmp_t pbmp;
    
    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        soc_robo_53242_mmu_default_set(unit);

        rv = REG_READ_BONDING_PADr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);

        reg_value &= 0x1f;
        reg_value >>= 1;

        if (reg_value == 0xf) {
            bcm53222_attached = 1;
        }
    } else if (SOC_IS_VULCAN(unit) || SOC_IS_LOTUS(unit) ||
        SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) ||
        SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
        SOC_IS_STARFIGHTER3(unit)) {

        /* Configure WAN ports if any */
        pbmp = soc_property_get_pbmp(unit, spn_PBMP_WAN_PORT, 0);
        SOC_PBMP_AND(pbmp, PBMP_ALL(unit));
        if (SOC_PBMP_NOT_NULL(pbmp)) {
            rv = REG_READ_WAN_PORT_SELr(unit, &reg_value);
            SOC_IF_ERROR_RETURN(rv);
            temp = SOC_PBMP_WORD_GET(pbmp, 0);
            soc_WAN_PORT_SELr_field_set(unit, &reg_value, WAN_SELECTf, &temp);
            temp = 1;
            soc_WAN_PORT_SELr_field_set(unit, &reg_value, EN_MAN2WANf, &temp);
    
            rv = REG_WRITE_WAN_PORT_SELr(unit, &reg_value);
            SOC_IF_ERROR_RETURN(rv);
        }

        /* Enable dual-imp mode */
        temp = soc_property_get(unit, spn_DUAL_IMP_ENABLE, 0);

        if (SOC_PBMP_MEMBER(pbmp, 5)) {
            /* Port 5 can be selected WAN port only when dual-imp disabled */
            temp = 0;
        }

        if (temp) {
            /* Dual-IMP */
            temp = 0x3;
        } else {
            /* Single-IMP */
            temp = 0x2;
        }

        rv = REG_READ_GMNGCFGr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);
        soc_GMNGCFGr_field_set(unit, &reg_value, FRM_MNGPf, &temp);

        rv = REG_WRITE_GMNGCFGr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);
    } else if (SOC_IS_BLACKBIRD(unit) ||
        SOC_IS_BLACKBIRD2(unit)){

        /* Configure WAN ports if any */
        pbmp = soc_property_get_pbmp(unit, spn_PBMP_WAN_PORT, 0);
        SOC_PBMP_AND(pbmp, PBMP_ALL(unit));
        if (SOC_PBMP_NOT_NULL(pbmp)) {
#ifdef BCM_53128
            if (SOC_IS_BLACKBIRD2(unit)) {
                rv = REG_READ_MDIO_PORT7_ADDRr(unit, &reg_value);
                SOC_IF_ERROR_RETURN(rv);
                temp = SOC_PBMP_WORD_GET(pbmp, 0);
                soc_MDIO_PORT7_ADDRr_field_set(unit, &reg_value, WAN_SELECTf, &temp);
        
                rv = REG_WRITE_MDIO_PORT7_ADDRr(unit, &reg_value);
                SOC_IF_ERROR_RETURN(rv);
            } else {
#endif /* BCM_53128 */
            rv = REG_READ_WAN_PORT_SELr(unit, &reg_value);
            SOC_IF_ERROR_RETURN(rv);
            temp = SOC_PBMP_WORD_GET(pbmp, 0);
            soc_WAN_PORT_SELr_field_set(unit, &reg_value, WAN_SELECTf, &temp);
    
            rv = REG_WRITE_WAN_PORT_SELr(unit, &reg_value);
            SOC_IF_ERROR_RETURN(rv);
#ifdef BCM_53128
            }
#endif /* BCM_53128 */
        }

    /* appended for next robo chip */
    } else if (SOC_IS_DINO16(unit)) {
        /* enable Exterenal PHY auto-polling */
        rv = REG_READ_EXTPHY_SCAN_CTLr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);

        temp = 1;
        soc_EXTPHY_SCAN_CTLr_field_set(unit, &reg_value, PHY_SCAN_ENf, &temp);

        rv = REG_WRITE_EXTPHY_SCAN_CTLr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);
        
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "misc_init: External PHY auto-polling enabled\n")));
    } else {
    }
    
    /* disable device base PHY auto-scan behavior : 
    *   - checkMe : check if only bcm5324, bcm5348 and bcm5396 has device 
    *           base enable/disable setting for phy auto-scan.
    */
    if  (SOC_IS_ROBO53242(unit)|| SOC_IS_ROBO53262(unit) || 
        SOC_IS_TBX(unit)){
        rv = REG_READ_PHYSCAN_CTLr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);

        temp = 0;            
        soc_PHYSCAN_CTLr_field_set(unit,  &reg_value, EN_PHY_SCANf, &temp);

        rv = REG_WRITE_PHYSCAN_CTLr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);

    } else if (SOC_IS_DINO16(unit)) {  /* bcm5396 only */
        rv = REG_READ_EXTPHY_SCAN_CTLr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);

        temp = 0;
        soc_EXTPHY_SCAN_CTLr_field_set(unit, &reg_value, PHY_SCAN_ENf, &temp);

        rv = REG_WRITE_EXTPHY_SCAN_CTLr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);
    }

        
    /* TB misc_init :
     *  1. Port Mask table reset.
     *  2. Flow Control init :
     *      - global XOFF enable.
     *      - Port basis XOFF disable.
     *  3. User MAC address
     */
    if (SOC_IS_TBX(unit)){
        uint64  reg64_val;
        soc_pbmp_t  mport_vctr;
        uint8       mport_addr[6];
        
        /* Reset PORTMASK */
        SOC_IF_ERROR_RETURN(DRV_MEM_CLEAR(unit, DRV_MEM_PORTMASK));

        /* Flow control init : enable global XOFF */
        temp = 1;
        SOC_IF_ERROR_RETURN(REG_READ_NEW_CONTROLr(unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_NEW_CONTROLr_field_set(
                unit, &reg_value, EN_SW_FLOW_CONf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NEW_CONTROLr(unit, &reg_value));
        
        /* Flow control init : diable XOFF on each port */
        COMPILER_64_ZERO(reg64_val);
        SOC_IF_ERROR_RETURN(REG_WRITE_SW_XOFF_PORT_CTLr(unit, &reg64_val));
        
        /* User MAC addresses init process
         *  1. clear user address(global will be disabled if all user 
         *      addresses were deleted)
         *  2. Set the the VLAN bypass default setting about user address.
         */
        SOC_PBMP_CLEAR(mport_vctr);
        ENET_SET_MACADDR(mport_addr, _soc_mac_all_zeroes);
        SOC_IF_ERROR_RETURN(DRV_MAC_SET(
                unit, mport_vctr, DRV_MAC_MULTIPORT_0, mport_addr, 0));
        SOC_IF_ERROR_RETURN(DRV_MAC_SET(
                unit, mport_vctr, DRV_MAC_MULTIPORT_1, mport_addr, 0));
                
        /* default at VLAN bypass l2 address is for the lagcy support 
         *  - DVAPI for all robo chip will test such bypass behavior
         */
        SOC_IF_ERROR_RETURN(DRV_VLAN_PROP_SET(
                unit, DRV_VLAN_PROP_BYPASS_L2_USER_ADDR, TRUE));
    }
    
    /* reset the Traffic remarking on Non-ResEPacket */
    if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {        
        soc_port_t  port;
        uint32      pri;
        
        /* bcm5395 on this feature is not implemented, so this reset exclude 
         * bcm5395 related setting.
         */
         
        /* reset the control register */
        if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            rv = REG_READ_TRREG_CTRL0r(unit, &reg_value);
        } else {
            rv = REG_READ_TRREG_CTRLr(unit, &reg_value);
        }
        SOC_IF_ERROR_RETURN(rv);
        temp = 0;
        if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            soc_TRREG_CTRL0r_field_set(unit, &reg_value, PCP_RMK_ENf, &temp);
        } else {
            soc_TRREG_CTRLr_field_set(unit, &reg_value, PCP_RMK_ENf, &temp);
        }
        if (!(SOC_IS_LOTUS(unit) || SOC_IS_BLACKBIRD2(unit))) {
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                soc_TRREG_CTRL0r_field_set(unit, &reg_value, 
                                                CFI_RMK_ENf, &temp);
            } else {
                soc_TRREG_CTRLr_field_set(unit, &reg_value, CFI_RMK_ENf, &temp);
            }
        }

        if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            rv = REG_WRITE_TRREG_CTRL0r(unit, &reg_value);
        } else {
            rv = REG_WRITE_TRREG_CTRLr(unit, &reg_value);
        }
        SOC_IF_ERROR_RETURN(rv);
        
        
        /* reset the TC2PCP mapping */
        PBMP_ALL_ITER(unit, port){
            for (pri = 0; pri <= 7; pri++) {
                
                /* the new-pri is formed as {CFI(bit4),PRI(bit3-bit0)} 
                 *  - in the reset value, the CFI is rewrite to 0 always!
                 *      (default is 1 on the RV=1 field)
                 */
                SOC_IF_ERROR_RETURN(DRV_PORT_PRI_MAPOP_SET(unit, port, 
                        DRV_PORT_OP_NORMAL_TC2PCP, pri, 0, pri, 0));
                
                /* outband TC2PCP is supported on bcm53115 only */
                if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                    SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                    SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                    SOC_IF_ERROR_RETURN(DRV_PORT_PRI_MAPOP_SET(unit, port, 
                            DRV_PORT_OP_OUTBAND_TC2PCP, pri, 0, pri, 0));
                    
                }
            }
        }
    }
   
    /* Enable the SA learning of reserved mutilcasts */
    if (SOC_IS_ROBO_ARCH_VULCAN(unit) || SOC_IS_DINO(unit)) {

        temp = 1;
        SOC_IF_ERROR_RETURN(
            REG_READ_RSV_MCAST_CTRLr(unit, &reg_value));
        soc_RSV_MCAST_CTRLr_field_set(unit, &reg_value, 
            EN_RES_MUL_LEARNf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_RSV_MCAST_CTRLr(unit, &reg_value));
    }
    
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
    if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit)) {
        /* Select SOP from 1588 */   
        temp = 0x1FF;
        SOC_IF_ERROR_RETURN(
            REG_READ_P1588_CTRLr(unit, &reg_value));
        soc_P1588_CTRLr_field_set(unit, &reg_value,
            SOP_SELf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_P1588_CTRLr(unit, &reg_value));

        /* Enable RX LINK delay always */
        SOC_IF_ERROR_RETURN(
            REG_READ_TX_PORT_0_TS_OFFSET_MSBr(unit, &reg_value));
        temp = 0xF;
        soc_TX_PORT_0_TS_OFFSET_MSBr_field_set(unit, &reg_value, 
            TS_LDf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_TX_PORT_0_TS_OFFSET_MSBr(unit, &reg_value));
        SOC_IF_ERROR_RETURN(
            REG_WRITE_TX_PORT_1_TS_OFFSET_MSBr(unit, &reg_value));
        SOC_IF_ERROR_RETURN(
            REG_WRITE_TX_PORT_2_TS_OFFSET_MSBr(unit, &reg_value));
        SOC_IF_ERROR_RETURN(
            REG_WRITE_TX_PORT_3_TS_OFFSET_MSBr(unit, &reg_value));
        SOC_IF_ERROR_RETURN(
            REG_WRITE_TX_PORT_4_TS_OFFSET_MSBr(unit, &reg_value));
        SOC_IF_ERROR_RETURN(
            REG_WRITE_TX_PORT_5_TS_OFFSET_MSBr(unit, &reg_value));
        SOC_IF_ERROR_RETURN(
            REG_WRITE_TX_PORT_7_TS_OFFSET_MSBr(unit, &reg_value));
        SOC_IF_ERROR_RETURN(
            REG_WRITE_TX_PORT_8_TS_OFFSET_MSBr(unit, &reg_value));            
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT  || NS+ */

#ifdef INCLUDE_MACSEC
    /* Misc init for Switch-MACSEC */
#if defined(BCM_NORTHSTARPLUS_SUPPORT)
    if (SOC_IS_NORTHSTARPLUS(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_SET(unit, 
                    DRV_DEV_PROP_SWITCHMACSEC_SW_INIT, TRUE));
        /* NS+ specific :
         *  - MACSEC bypass for P4 and P5 must be disabled first for normal 
         *    MACSEC init process about MMI/LMI I/O.
         * Note : Once the macsec_enable is not request! this bypass will be 
         *      enabled later.
         */
        temp = soc_property_get(unit, spn_MACSEC_ENABLE, 0);
        temp = (temp == 0) ? TRUE : FALSE; 
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "%s,FORCE BYPASS-%s due to MACSEC %s!!\n"), FUNCTION_NAME(), 
                     (temp) ? "ON" : "OFF", (!temp) ? "enabling" : "disabling"));
        SOC_IF_ERROR_RETURN(
                soc_robo_macsec_bypass_set(unit, 4, temp));
        SOC_IF_ERROR_RETURN(
                soc_robo_macsec_bypass_set(unit, 5, temp));
    }
#endif /* BCM_NORTHSTARPLUS_SUPPORT */
#endif /* INCLUDE_MACSEC */

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "soc_robo_misc_init: OK\n")));
    
    return rv;
}

int
soc_robo_mmu_init(int unit)
{
    return SOC_E_NONE;
}

int
soc_robo_age_timer_get(int unit, int *age_seconds, int *enabled)
{
    return SOC_E_NONE;
}

int
soc_robo_age_timer_max_get(int unit, int *max_seconds)
{
    return SOC_E_NONE;
}

int
soc_robo_age_timer_set(int unit, int age_seconds, int enable)
{
    return SOC_E_NONE;
}

/*
 * soc_robo_64_val_to_pbmp() :
 *     -- Get port bitmap from unsigned 64-bits integer variable.
 * 
 * unit: unit
 * *pbmp: (OUT) returned port bitmap
 * value64: (IN) data value for transfering into port bitmap
 */
int
soc_robo_64_val_to_pbmp(int unit, soc_pbmp_t *pbmp, uint64 value64)
{
    uint32 value32;

    COMPILER_64_TO_32_LO(value32, value64);
    SOC_PBMP_WORD_SET(*pbmp, 0, value32);
    if (SOC_INFO(unit).port_num > 32) {
        COMPILER_64_TO_32_HI(value32, value64);
        SOC_PBMP_WORD_SET(*pbmp, 1, value32);
    } else {
        SOC_PBMP_WORD_SET(*pbmp, 1, 0);
    }
    
    return SOC_E_NONE;
}

/*
 * soc_robo_64_pbmp_to_val() :
 *     -- Transfer port bitmap into unsigned 64-bits integer variable.
 * 
 * unit: unit
 * *pbmp: (IN) returned port bitmap
 * *value64: (OUT) data value for transfering into port bitmap
 */
int
soc_robo_64_pbmp_to_val(int unit, soc_pbmp_t *pbmp, uint64 *value64)
{
    uint32 value_h, value_l;

    value_l = SOC_PBMP_WORD_GET(*pbmp, 0);
    if (SOC_INFO(unit).port_num > 32) {
        value_h = SOC_PBMP_WORD_GET(*pbmp, 1);
    } else {
        value_h = 0;
    }
    COMPILER_64_SET(*value64, value_h, value_l);

    return SOC_E_NONE;
}

int
soc_robo_loop_detect_enable_set(int unit, int enable)
{
    uint32 reg_value = 0;

    /* enable Loop Detection */
    SOC_IF_ERROR_RETURN(REG_READ_LPDET_CFGr(unit, &reg_value));

    /* enable/disable loop detection */
    SOC_IF_ERROR_RETURN(soc_LPDET_CFGr_field_set
        (unit, &reg_value, EN_LPDETf, (uint32 *)&enable));

    SOC_IF_ERROR_RETURN(REG_WRITE_LPDET_CFGr(unit, &reg_value));

    return SOC_E_NONE;
}

int
soc_robo_loop_detect_enable_get(int unit, int *enable)
{
    uint32 reg_value = 0;

    SOC_IF_ERROR_RETURN(REG_READ_LPDET_CFGr(unit, &reg_value));

    SOC_IF_ERROR_RETURN(soc_LPDET_CFGr_field_get
        (unit, &reg_value, EN_LPDETf, (void *)&enable));

    return SOC_E_NONE;
}


int
soc_robo_loop_detect_pbmp_get(int unit, soc_pbmp_t *pbmp)
{
    uint32 reg_value32, pbmp_value32;

    SOC_IF_ERROR_RETURN(REG_READ_LED_PORTMAPr(unit, &reg_value32)); 

    SOC_IF_ERROR_RETURN(soc_LED_PORTMAPr_field_get
        (unit, &reg_value32, LED_WARNING_PORTMAPf, &pbmp_value32));

    SOC_PBMP_WORD_SET(*pbmp, 0, pbmp_value32);

    return SOC_E_NONE;
}

#endif  /* BCM_ROBO_SUPPORT */
