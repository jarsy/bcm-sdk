/*
 * $Id: eav.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Field Processor related CLI commands
 */

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/error.h>
#include <soc/drv_if.h>
#include <soc/drv.h>
#include <soc/debug.h>


#define EAV_GEX_MAX_TICK_INC    63
#define EAV_GEX_MAX_TICK_ADJUST_PERIOD    15
#define EAV_GEX_MAX_SLOT_ADJUST_PERIOD    15
#define EAV_GEX_MAX_TICK_ONE_SLOT    3126
#define EAV_GEX_MAX_SLOT_NUMBER    31

#define EAV_GEX_MAX_PCP_VALUE    0x7

/* Bytes count allowed for EAV Class4/Class5 bandwidth within a slot time */
#define EAV_GEX_MAX_BANDWIDTH_VALUE 16383
#define EAV_GEX_MIN_BANDWIDTH_VALUE 0

#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT)
static int
_drv_vulcan_eav_mmu_init(int unit)
{
    uint32 reg_val;

    /* 1. MMU settings provided by ASIC*/
    /* Hysteresis threshold */
    reg_val = 110; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa10, &reg_val, 2);
    reg_val = 111;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa12, &reg_val, 2);
    reg_val = 111;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa14, &reg_val, 2);
    reg_val = 112;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa16, &reg_val, 2);
    reg_val = 112;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xab0, &reg_val, 2);
    reg_val = 112;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xab2, &reg_val, 2);

    /* Pause threshold */
    reg_val = 232; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa18, &reg_val, 2);
    reg_val = 233;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa1a, &reg_val, 2);
    reg_val = 234;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa1c, &reg_val, 2);
    reg_val = 235;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa1e, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xab4, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xab6, &reg_val, 2);

    /* Drop threshold */
    reg_val = 500; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa20, &reg_val, 2);
    reg_val = 500;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa22, &reg_val, 2);
    reg_val = 500;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa24, &reg_val, 2);
    reg_val = 500;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa26, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xab8, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xaba, &reg_val, 2);

    /* Total reserved threshold */
    reg_val = 1; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa40, &reg_val, 2);
    reg_val = 1;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa42, &reg_val, 2);
    reg_val = 1;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa44, &reg_val, 2);
    reg_val = 1;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa46, &reg_val, 2);
    reg_val = 18;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa48, &reg_val, 2);
    reg_val = 24;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa4a, &reg_val, 2);

    /* IMP Hysteresis threshold */
    reg_val = 122; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd10, &reg_val, 2);
    reg_val = 123;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd12, &reg_val, 2);
    reg_val = 123;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd14, &reg_val, 2);
    reg_val = 124;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd16, &reg_val, 2);

    /* IMP Pause threshold */
    reg_val = 244; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd18, &reg_val, 2);
    reg_val = 245;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd1a, &reg_val, 2);
    reg_val = 246;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd1c, &reg_val, 2);
    reg_val = 247;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd1e, &reg_val, 2);

    /* IMP Drop threshold */
    reg_val = 511; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd20, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd22, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd24, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd26, &reg_val, 2);

    /* Total Hysteresis threshold */
    reg_val = 108; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa28, &reg_val, 2);
    reg_val = 109;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa2a, &reg_val, 2);
    reg_val = 110;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa2c, &reg_val, 2);
    reg_val = 111;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa2e, &reg_val, 2);
    reg_val = 111;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xabc, &reg_val, 2);
    reg_val = 111;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xabe, &reg_val, 2);

    /* Total Pause threshold */
    reg_val = 246; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa30, &reg_val, 2);
    reg_val = 248;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa32, &reg_val, 2);
    reg_val = 250;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa34, &reg_val, 2);
    reg_val = 252;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa36, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xac0, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xac2, &reg_val, 2);

    /* Total Drop threshold */
    reg_val = 378; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa38, &reg_val, 2);
    reg_val = 380;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa3a, &reg_val, 2);
    reg_val = 382;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa3c, &reg_val, 2);
    reg_val = 384;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa3e, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xac4, &reg_val, 2);
    reg_val = 511;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xac6, &reg_val, 2);

    /* Total IMP Hysteresis threshold */
    reg_val = 138; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd28, &reg_val, 2);
    reg_val = 139;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd2a, &reg_val, 2);
    reg_val = 140;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd2c, &reg_val, 2);
    reg_val = 141;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd2e, &reg_val, 2);

    /* Total IMP Pause threshold */
    reg_val = 276; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd30, &reg_val, 2);
    reg_val = 278;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd32, &reg_val, 2);
    reg_val = 280;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd34, &reg_val, 2);
    reg_val = 282;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd36, &reg_val, 2);

    /* Total IMP Drop threshold */
    reg_val = 408; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd38, &reg_val, 2);
    reg_val = 410;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd3a, &reg_val, 2);
    reg_val = 412;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd3c, &reg_val, 2);
    reg_val = 414;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd3e, &reg_val, 2);

    return SOC_E_NONE;
}
#endif /* BCM_VULCAN_SUPPRT || BCM_BLACKBIRD_SUPPORT */

#if defined(BCM_STARFIGHTER_SUPPORT)
static int
_drv_starfighter_eav_mmu_init(int unit)
{
    uint32 reg_val;

    /* 1. MMU settings provided by ASIC*/
    /* Hysteresis threshold */
    reg_val = 110; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Qr(unit, 0, &reg_val));
    reg_val = 111;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Qr(unit, 1, &reg_val));
    reg_val = 111;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Qr(unit, 2, &reg_val));
    reg_val = 112;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Qr(unit, 3, &reg_val));
    reg_val = 112;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q45r(unit, 0, &reg_val));
    reg_val = 112;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q45r(unit, 1, &reg_val));

    /* Pause threshold */
    reg_val = 232; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Qr(unit, 0, &reg_val));
    reg_val = 233;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Qr(unit, 1, &reg_val));
    reg_val = 234;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Qr(unit, 2, &reg_val));
    reg_val = 235;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Qr(unit, 3, &reg_val));
    reg_val = 511;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q45r(unit, 0, &reg_val));
    reg_val = 511;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q45r(unit, 1, &reg_val));

    /* Drop threshold */
    reg_val = 500;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Qr(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Qr(unit, 1, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Qr(unit, 2, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Qr(unit, 3, &reg_val));
    reg_val = 511;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q45r(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q45r(unit, 1, &reg_val));

    /* Total reserved threshold */
    reg_val = 1; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_RSRV_Qr(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_RSRV_Qr(unit, 1, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_RSRV_Qr(unit, 2, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_RSRV_Qr(unit, 3, &reg_val));
    reg_val = 18;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_RSRV_Qr(unit, 4, &reg_val));
    reg_val = 24;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_RSRV_Qr(unit, 5, &reg_val));

    /* IMP Hysteresis threshold */
    reg_val = 122; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q_IMPr(unit, 0, &reg_val));
    reg_val = 123;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q_IMPr(unit, 1, &reg_val));
    reg_val = 123;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q_IMPr(unit, 2, &reg_val));
    reg_val = 124;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q_IMPr(unit, 3, &reg_val));

    /* IMP Pause threshold */
    reg_val = 244; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q_IMPr(unit, 0, &reg_val));
    reg_val = 245;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q_IMPr(unit, 1, &reg_val));
    reg_val = 246;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q_IMPr(unit, 2, &reg_val));
    reg_val = 247;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q_IMPr(unit, 3, &reg_val));

    /* IMP Drop threshold */
    reg_val = 511; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q_IMPr(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q_IMPr(unit, 1, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q_IMPr(unit, 2, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q_IMPr(unit, 3, &reg_val));

    /* Total Hysteresis threshold */
    reg_val = 108; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Qr(unit, 0, &reg_val));
    reg_val = 109;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Qr(unit, 1, &reg_val));
    reg_val = 110;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Qr(unit, 2, &reg_val));
    reg_val = 111;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Qr(unit, 3, &reg_val));
    reg_val = 111;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q45r(unit, 0, &reg_val));
    reg_val = 111;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q45r(unit, 1, &reg_val));

    /* Total Pause threshold */
    reg_val = 246; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Qr(unit, 0, &reg_val));
    reg_val = 248;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Qr(unit, 1, &reg_val));
    reg_val = 250;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Qr(unit, 2, &reg_val));
    reg_val = 252;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Qr(unit, 3, &reg_val));
    reg_val = 511;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q45r(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q45r(unit, 1, &reg_val));

    /* Total Drop threshold */
    reg_val = 378; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Qr(unit, 0, &reg_val));
    reg_val = 380;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Qr(unit, 1, &reg_val));
    reg_val = 382;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Qr(unit, 2, &reg_val));
    reg_val = 384;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Qr(unit, 3, &reg_val));
    reg_val = 511;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q45r(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q45r(unit, 1, &reg_val));

    /* Total IMP Hysteresis threshold */
    reg_val = 138; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q_IMPr(unit, 0, &reg_val));
    reg_val = 139;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q_IMPr(unit, 1, &reg_val));
    reg_val = 140;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q_IMPr(unit, 2, &reg_val));
    reg_val = 141;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q_IMPr(unit, 3, &reg_val));

    /* Total IMP Pause threshold */
    reg_val = 276; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q_IMPr(unit, 0, &reg_val));
    reg_val = 278;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q_IMPr(unit, 1, &reg_val));
    reg_val = 280;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q_IMPr(unit, 2, &reg_val));
    reg_val = 282;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q_IMPr(unit, 3, &reg_val));

    /* Total IMP Drop threshold */
    reg_val = 408; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q_IMPr(unit, 0, &reg_val));
    reg_val = 410;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q_IMPr(unit, 1, &reg_val));
    reg_val = 412;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q_IMPr(unit, 2, &reg_val));
    reg_val = 414;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q_IMPr(unit, 3, &reg_val));

    return SOC_E_NONE;
}

#endif /* BCM_STARFIGHTER_SUPPORT */

#ifdef BCM_BLACKBIRD2_SUPPORT
static int
_drv_blackbird2_eav_mmu_init(int unit)
{
    uint32 reg_val;

    /* 1. MMU settings provided by ASIC*/
    /* Hysteresis threshold */
    reg_val = 50; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Qr(unit, 0, &reg_val));
    reg_val = 54;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Qr(unit, 1, &reg_val));
    reg_val = 58;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Qr(unit, 2, &reg_val));
    reg_val = 62;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Qr(unit, 3, &reg_val));
    reg_val = 62;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q45r(unit, 0, &reg_val));
    reg_val = 62;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q45r(unit, 1, &reg_val));

    /* Pause threshold */
    reg_val = 99; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Qr(unit, 0, &reg_val));
    reg_val = 107;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Qr(unit, 1, &reg_val));
    reg_val = 115;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Qr(unit, 2, &reg_val));
    reg_val = 123;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Qr(unit, 3, &reg_val));
    reg_val = 767;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q45r(unit, 0, &reg_val));
    reg_val = 767;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q45r(unit, 1, &reg_val));

    /* Drop threshold */
    reg_val = 743;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Qr(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Qr(unit, 1, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Qr(unit, 2, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Qr(unit, 3, &reg_val));
    reg_val = 767;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q45r(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q45r(unit, 1, &reg_val));

    /* Total reserved threshold */
    reg_val = 24; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_RSRV_Qr(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_RSRV_Qr(unit, 1, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_RSRV_Qr(unit, 2, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_RSRV_Qr(unit, 3, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_RSRV_Qr(unit, 4, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_RSRV_Qr(unit, 5, &reg_val));

    /* IMP and WAN Hysteresis threshold */
    reg_val = 62; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q_IMPr(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q_WANr(unit, 0, &reg_val));
    reg_val = 66;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q_IMPr(unit, 1, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q_WANr(unit, 1, &reg_val));
    reg_val = 70;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q_IMPr(unit, 2, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q_WANr(unit, 2, &reg_val));
    reg_val = 74;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q_IMPr(unit, 3, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q_WANr(unit, 3, &reg_val));

    /* IMP and WAN Pause threshold */
    reg_val = 123; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q_IMPr(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q_WANr(unit, 0, &reg_val));
    reg_val = 131;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q_IMPr(unit, 1, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q_IMPr(unit, 1, &reg_val));
    reg_val = 139;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q_IMPr(unit, 2, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q_IMPr(unit, 2, &reg_val));
    reg_val = 147;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q_IMPr(unit, 3, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q_IMPr(unit, 3, &reg_val));

    /* IMP Drop threshold */
    reg_val = 767; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q_IMPr(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q_IMPr(unit, 1, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q_IMPr(unit, 2, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q_IMPr(unit, 3, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q_WANr(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q_WANr(unit, 1, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q_WANr(unit, 2, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q_WANr(unit, 3, &reg_val));

    /* Total Hysteresis threshold */
    reg_val = 232; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Qr(unit, 0, &reg_val));
    reg_val = 236;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Qr(unit, 1, &reg_val));
    reg_val = 240;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Qr(unit, 2, &reg_val));
    reg_val = 244;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Qr(unit, 3, &reg_val));
    reg_val = 244;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q45r(unit, 0, &reg_val));
    reg_val = 244;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q45r(unit, 1, &reg_val));

    /* Total Pause threshold */
    reg_val = 463; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Qr(unit, 0, &reg_val));
    reg_val = 471;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Qr(unit, 1, &reg_val));
    reg_val = 479;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Qr(unit, 2, &reg_val));
    reg_val = 487;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Qr(unit, 3, &reg_val));
    reg_val = 767;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q45r(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q45r(unit, 1, &reg_val));

    /* Total Drop threshold */
    reg_val = 591; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Qr(unit, 0, &reg_val));
    reg_val = 599;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Qr(unit, 1, &reg_val));
    reg_val = 607;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Qr(unit, 2, &reg_val));
    reg_val = 615;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Qr(unit, 3, &reg_val));
    reg_val = 767;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q45r(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q45r(unit, 1, &reg_val));

    /* Total IMP Hysteresis threshold */
    reg_val = 244; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q_IMPr(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q_WANr(unit, 0, &reg_val));
    reg_val = 248;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q_IMPr(unit, 1, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q_WANr(unit, 1, &reg_val));
    reg_val = 252;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q_IMPr(unit, 2, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q_WANr(unit, 2, &reg_val));
    reg_val = 256;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q_IMPr(unit, 3, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q_WANr(unit, 3, &reg_val));

    /* Total IMP Pause threshold */
    reg_val = 487; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q_IMPr(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q_WANr(unit, 0, &reg_val));
    reg_val = 495;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q_IMPr(unit, 1, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q_WANr(unit, 1, &reg_val));
    reg_val = 503;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q_IMPr(unit, 2, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q_WANr(unit, 2, &reg_val));
    reg_val = 511;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q_IMPr(unit, 3, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q_WANr(unit, 3, &reg_val));

    /* Total IMP Drop threshold */
    reg_val = 615; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q_IMPr(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q_WANr(unit, 0, &reg_val));
    reg_val = 623;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q_IMPr(unit, 1, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q_WANr(unit, 1, &reg_val));
    reg_val = 631;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q_IMPr(unit, 2, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q_WANr(unit, 2, &reg_val));
    reg_val = 639;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q_IMPr(unit, 3, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q_WANr(unit, 3, &reg_val));

    return SOC_E_NONE;
}

#endif /* BCM_BLACKBIRD2_SUPPORT */

#ifdef BCM_LOTUS_SUPPORT
static int
_drv_lotus_eav_mmu_init(int unit)
{
    uint32 reg_val;

    /* 1. MMU settings provided by ASIC*/
    /* Hysteresis threshold */
    reg_val = 9; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa10, &reg_val, 2);
    reg_val = 9;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa12, &reg_val, 2);
    reg_val = 10;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa14, &reg_val, 2);
    reg_val = 10;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa16, &reg_val, 2);
    reg_val = 10;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xab0, &reg_val, 2);
    reg_val = 10;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xab2, &reg_val, 2);

    /* Pause threshold */
    reg_val = 18; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa18, &reg_val, 2);
    reg_val = 19;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa1a, &reg_val, 2);
    reg_val = 20;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa1c, &reg_val, 2);
    reg_val = 21;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa1e, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xab4, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xab6, &reg_val, 2);

    /* Drop threshold */
    reg_val = 252; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa20, &reg_val, 2);
    reg_val = 252;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa22, &reg_val, 2);
    reg_val = 252;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa24, &reg_val, 2);
    reg_val = 252;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa26, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xab8, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xaba, &reg_val, 2);

    /* Txq reserved threshold */
    reg_val = 6; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa40, &reg_val, 2);
    reg_val = 6;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa42, &reg_val, 2);
    reg_val = 6;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa44, &reg_val, 2);
    reg_val = 6;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa46, &reg_val, 2);
    reg_val = 6;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa48, &reg_val, 2);
    reg_val = 6;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa4a, &reg_val, 2);

    /* IMP Hysteresis threshold */
    reg_val = 10; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd10, &reg_val, 2);
    reg_val = 11;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd12, &reg_val, 2);
    reg_val = 11;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd14, &reg_val, 2);
    reg_val = 12;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd16, &reg_val, 2);

    /* IMP Pause threshold */
    reg_val = 21; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd18, &reg_val, 2);
    reg_val = 22;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd1a, &reg_val, 2);
    reg_val = 23;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd1c, &reg_val, 2);
    reg_val = 24;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd1e, &reg_val, 2);

    /* IMP Drop threshold */
    reg_val = 255; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd20, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd22, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd24, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd26, &reg_val, 2);

    /* Total Hysteresis threshold */
    reg_val = 45; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa28, &reg_val, 2);
    reg_val = 45;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa2a, &reg_val, 2);
    reg_val = 46;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa2c, &reg_val, 2);
    reg_val = 46;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa2e, &reg_val, 2);
    reg_val = 46;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xabc, &reg_val, 2);
    reg_val = 46;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xabe, &reg_val, 2);

    /* Total Pause threshold */
    reg_val = 90; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa30, &reg_val, 2);
    reg_val = 91;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa32, &reg_val, 2);
    reg_val = 92;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa34, &reg_val, 2);
    reg_val = 93;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa36, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xac0, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xac2, &reg_val, 2);

    /* Total Drop threshold */
    reg_val = 118; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa38, &reg_val, 2);
    reg_val = 119;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa3a, &reg_val, 2);
    reg_val = 120;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa3c, &reg_val, 2);
    reg_val = 121;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xa3e, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xac4, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xac6, &reg_val, 2);

    /* Total IMP Hysteresis threshold */
    reg_val = 48; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd28, &reg_val, 2);
    reg_val = 47;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd2a, &reg_val, 2);
    reg_val = 47;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd2c, &reg_val, 2);
    reg_val = 48;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd2e, &reg_val, 2);

    /* Total IMP Pause threshold */
    reg_val = 93; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd30, &reg_val, 2);
    reg_val = 94;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd32, &reg_val, 2);
    reg_val = 95;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd34, &reg_val, 2);
    reg_val = 96;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd36, &reg_val, 2);

    /* Total IMP Drop threshold */
    reg_val = 121; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd38, &reg_val, 2);
    reg_val = 122;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd3a, &reg_val, 2);
    reg_val = 123;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd3c, &reg_val, 2);
    reg_val = 124;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xd3e, &reg_val, 2);

    /* WAN & IMP1 */
    /* IMP1 Hysteresis threshold */
    reg_val = 12; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe10, &reg_val, 2);
    reg_val = 12;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe12, &reg_val, 2);
    reg_val = 13;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe14, &reg_val, 2);
    reg_val = 13;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe16, &reg_val, 2);

    /* IMP1 Pause threshold */
    reg_val = 24; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe18, &reg_val, 2);
    reg_val = 25;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe1a, &reg_val, 2);
    reg_val = 26;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe1c, &reg_val, 2);
    reg_val = 27;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe1e, &reg_val, 2);

    /* IMP1 Drop threshold */
    reg_val = 255; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe20, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe22, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe24, &reg_val, 2);
    reg_val = 255;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe26, &reg_val, 2);

    /* Total IMP1 Hysteresis threshold */
    reg_val = 48; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe28, &reg_val, 2);
    reg_val = 48;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe2a, &reg_val, 2);
    reg_val = 49;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe2c, &reg_val, 2);
    reg_val = 49;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe2e, &reg_val, 2);

    /* Total IMP1 Pause threshold */
    reg_val = 96; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe30, &reg_val, 2);
    reg_val = 97;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe32, &reg_val, 2);
    reg_val = 98;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe34, &reg_val, 2);
    reg_val = 99;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe36, &reg_val, 2);

    /* Total IMP1 Drop threshold */
    reg_val = 124; 
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe38, &reg_val, 2);
    reg_val = 125;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe3a, &reg_val, 2);
    reg_val = 126;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe3c, &reg_val, 2);
    reg_val = 127;
    (DRV_SERVICES(unit)->reg_write)
        (unit, 0xe3e, &reg_val, 2);

    return SOC_E_NONE;
}
#endif /* BCM_LOTUS_SUPPORT */

int 
drv_gex_eav_control_set(int unit, uint32 type, uint32 param)
{
    int rv = SOC_E_NONE;
    uint32 reg_value, temp;
    
    switch (type) {
        case DRV_EAV_CONTROL_TIME_STAMP_TO_IMP:
            SOC_IF_ERROR_RETURN(
                REG_READ_TM_STAMP_RPT_CTRLr(unit, &reg_value));
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            soc_TM_STAMP_RPT_CTRLr_field_set(unit, &reg_value, 
                TSRPT_PKT_ENf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_TM_STAMP_RPT_CTRLr(unit, &reg_value));
            break;
        case DRV_EAV_CONTROL_MAX_AV_SIZE:
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit) ||
                SOC_IS_LOTUS(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT)
                SOC_IF_ERROR_RETURN(
                    REG_READ_RESE_MAX_AV_PKT_SZr(unit, &reg_value));
                temp = param;
                soc_RESE_MAX_AV_PKT_SZr_field_set(unit, &reg_value, 
                    MAX_AV_PKT_SZf, &temp);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_RESE_MAX_AV_PKT_SZr(unit, &reg_value));
#endif                
            } else if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit)) {
#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_BLACKBIRD2_SUPPORT)
                SOC_IF_ERROR_RETURN(
                    REG_READ_AVB_MAX_AV_PKT_SZr(unit, &reg_value));
                temp = param;
                soc_AVB_MAX_AV_PKT_SZr_field_set(
                    unit, &reg_value, MAX_AV_PKT_SZf, &temp);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_AVB_MAX_AV_PKT_SZr(unit, &reg_value));
#endif                
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_EAV_CONTROL_STREAM_CLASSA_PCP:
            if (SOC_IS_BLACKBIRD(unit)|| SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_BLACKBIRD2(unit)) {
#if defined(BCM_BLACKBIRD_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_BLACKBIRD2_SUPPORT)
                SOC_IF_ERROR_RETURN(
                    REG_READ_CLASS_PCPr(unit, &reg_value));

                if (param > EAV_GEX_MAX_PCP_VALUE) {
                    return SOC_E_PARAM;
                }

                soc_CLASS_PCPr_field_set(unit, &reg_value, 
                    CLASSA_PCPf, &param);

                SOC_IF_ERROR_RETURN(
                    REG_WRITE_CLASS_PCPr(unit, &reg_value));
#endif                
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_EAV_CONTROL_STREAM_CLASSB_PCP:
            if (SOC_IS_BLACKBIRD(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_BLACKBIRD2(unit)) {
#if defined(BCM_BLACKBIRD_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_BLACKBIRD2_SUPPORT)               
                SOC_IF_ERROR_RETURN(
                    REG_READ_CLASS_PCPr(unit, &reg_value));

                if (param > EAV_GEX_MAX_PCP_VALUE) {
                    return SOC_E_PARAM;
                }

                soc_CLASS_PCPr_field_set(unit, &reg_value, 
                    CLASSB_PCPf, &param);

                SOC_IF_ERROR_RETURN(
                    REG_WRITE_CLASS_PCPr(unit, &reg_value));
#endif                
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_EAV_CONTROL_MMU_INIT:
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT)
                rv = _drv_vulcan_eav_mmu_init(unit);
#endif
            } else if (SOC_IS_LOTUS(unit)) {
#ifdef BCM_LOTUS_SUPPORT
                rv = _drv_lotus_eav_mmu_init(unit);
#endif
            } else if (SOC_IS_STARFIGHTER(unit)) {
#if defined(BCM_STARFIGHTER_SUPPORT)
                rv = _drv_starfighter_eav_mmu_init(unit);
#endif
            } else if (SOC_IS_BLACKBIRD2(unit)) {
#ifdef BCM_BLACKBIRD2_SUPPORT
                rv = _drv_blackbird2_eav_mmu_init(unit);
#endif
            }
            break;
        default:
            rv = SOC_E_PARAM;
    }

    return rv;
}


int 
drv_gex_eav_control_get(int unit, uint32 type, uint32 *param)
{
    int rv = SOC_E_NONE;
    uint32 reg_value, temp = 0;
    
    switch (type) {
        case DRV_EAV_CONTROL_TIME_STAMP_TO_IMP:
            SOC_IF_ERROR_RETURN(
                REG_READ_TM_STAMP_RPT_CTRLr(unit, &reg_value));
            soc_TM_STAMP_RPT_CTRLr_field_get(unit, &reg_value, 
                TSRPT_PKT_ENf, &temp);
            if (temp) {
                *param = TRUE;
            } else {
                *param = FALSE;
            }
            break;
        case DRV_EAV_CONTROL_MAX_AV_SIZE:
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit) ||
                SOC_IS_LOTUS(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT)            
                SOC_IF_ERROR_RETURN(
                    REG_READ_RESE_MAX_AV_PKT_SZr(unit, &reg_value));
                soc_RESE_MAX_AV_PKT_SZr_field_get(unit, &reg_value, 
                    MAX_AV_PKT_SZf, &temp);
                *param = temp;
#endif                
                } else if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit)) {
#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_BLACKBIRD2_SUPPORT)
                    SOC_IF_ERROR_RETURN(
                        REG_READ_AVB_MAX_AV_PKT_SZr(unit, &reg_value));
                    soc_AVB_MAX_AV_PKT_SZr_field_get(
                        unit, &reg_value, MAX_AV_PKT_SZf, &temp);
                    *param = temp;
#endif
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            break;
        case DRV_EAV_CONTROL_STREAM_CLASSA_PCP:
            if (SOC_IS_BLACKBIRD(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_BLACKBIRD2(unit) ) {
#if defined(BCM_BLACKBIRD_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_BLACKBIRD2_SUPPORT)               
                SOC_IF_ERROR_RETURN(
                    REG_READ_CLASS_PCPr(unit, &reg_value));

                soc_CLASS_PCPr_field_get(unit, &reg_value, 
                    CLASSA_PCPf, &temp);
                *param = temp;
#endif                
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_EAV_CONTROL_STREAM_CLASSB_PCP:
            if (SOC_IS_BLACKBIRD(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_BLACKBIRD2(unit)) {
#if defined(BCM_BLACKBIRD_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_BLACKBIRD2_SUPPORT)               
                SOC_IF_ERROR_RETURN(
                    REG_READ_CLASS_PCPr(unit, &reg_value));

                soc_CLASS_PCPr_field_get(unit, &reg_value, 
                    CLASSB_PCPf, &temp);
                *param = temp;
#endif                
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        default:
            rv = SOC_E_PARAM;
    }

    return rv;
}

int 
drv_gex_eav_enable_set(int unit, uint32 port, uint32 enable)
{
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_BLACKBIRD2_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
    uint32 reg_value;
#endif
    uint32 temp =0;

    /* Set EAV enable register */
    if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit) ||
        SOC_IS_LOTUS(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT)
        SOC_IF_ERROR_RETURN(
            REG_READ_RESE_AV_EN_CTRLr(unit, &reg_value));
        soc_RESE_AV_EN_CTRLr_field_get(unit, &reg_value, 
            AV_ENf, &temp);
#endif        
    } else if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit)) {
#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_BLACKBIRD2_SUPPORT)
        SOC_IF_ERROR_RETURN(
            REG_READ_AVB_AV_EN_CTRLr(unit, &reg_value));
        soc_AVB_AV_EN_CTRLr_field_get(unit, &reg_value, AV_ENf, &temp);
#endif        
    } else if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        SOC_IF_ERROR_RETURN(
            REG_READ_AVB_TIME_STAMP_ENr(unit, &reg_value));
        soc_AVB_TIME_STAMP_ENr_field_get(unit, &reg_value, 
            AVB_TIME_STAMP_ENf, &temp);
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
    }
    if (enable){
        temp |= 0x1 << port;
    } else {
        temp &= ~(0x1 << port);
    }
    if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit) ||
        SOC_IS_LOTUS(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT)        
        soc_RESE_AV_EN_CTRLr_field_set(unit, &reg_value, 
            AV_ENf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_RESE_AV_EN_CTRLr(unit, &reg_value));
#endif        
    } else if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit)) {
#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_BLACKBIRD2_SUPPORT)
        soc_AVB_AV_EN_CTRLr_field_set(unit, &reg_value, AV_ENf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_AVB_AV_EN_CTRLr(unit, &reg_value));
#endif        
    } else if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
         soc_AVB_TIME_STAMP_ENr_field_set(unit, &reg_value, 
            AVB_TIME_STAMP_ENf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_AVB_TIME_STAMP_ENr(unit, &reg_value));
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT*/
    }
    
    return SOC_E_NONE;
}

int 
drv_gex_eav_enable_get(int unit, uint32 port, uint32 *enable)
{
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_BLACKBIRD2_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) ||\
    defined(BCM_STARFIGHTER3_SUPPORT)
    uint32 reg_value;
#endif
    uint32 temp = 0;
    
    if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit) ||
        SOC_IS_LOTUS(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT)
        SOC_IF_ERROR_RETURN(
            REG_READ_RESE_AV_EN_CTRLr(unit, &reg_value));
        soc_RESE_AV_EN_CTRLr_field_get(unit, &reg_value, 
            AV_ENf, &temp);
#endif        
    } else if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit)) {
#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_BLACKBIRD2_SUPPORT)
        SOC_IF_ERROR_RETURN(
            REG_READ_AVB_AV_EN_CTRLr(unit, &reg_value));
        soc_AVB_AV_EN_CTRLr_field_get(unit, &reg_value, AV_ENf, &temp);
#endif
    } else if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        /* Enable per-port time stamp function */
        SOC_IF_ERROR_RETURN(
            REG_READ_AVB_TIME_STAMP_ENr(unit, &reg_value));
        soc_AVB_TIME_STAMP_ENr_field_get(unit, &reg_value, 
            AVB_TIME_STAMP_ENf, &temp);
#endif /* BCM_NORTHSTARPLUS_SUPPORT */
    }
    if (temp & (0x1 << port)){
        *enable = TRUE;
    } else {
        *enable = FALSE;
    }
    return SOC_E_NONE;
}

int 
drv_gex_eav_link_status_set(int unit, uint32 port, uint32 link)
{
    uint32 reg_value, temp = 0;

    /* Set EAV Link register */
    SOC_IF_ERROR_RETURN(
        REG_READ_EAV_LNK_STATUSr(unit, &reg_value));
    soc_EAV_LNK_STATUSr_field_get(unit, &reg_value, 
        PT_EAV_LNK_STATUSf, &temp);
    if (link){
        temp |= 0x1 << port;
    } else {
        temp &= ~(0x1 << port);
    }
    soc_EAV_LNK_STATUSr_field_set(unit, &reg_value, 
        PT_EAV_LNK_STATUSf, &temp);
    SOC_IF_ERROR_RETURN(
        REG_WRITE_EAV_LNK_STATUSr(unit, &reg_value));
    
    return SOC_E_NONE;
}

int 
drv_gex_eav_link_status_get(int unit, uint32 port, uint32 *link)
{
    uint32 reg_value, temp = 0;
    
    SOC_IF_ERROR_RETURN(
        REG_READ_EAV_LNK_STATUSr(unit, &reg_value));
    soc_EAV_LNK_STATUSr_field_get(unit, &reg_value, 
        PT_EAV_LNK_STATUSf, &temp);
    if (temp & (0x1 << port)){
        *link = TRUE;
    } else {
        *link = FALSE;
    }
    return SOC_E_NONE;
}

int
drv_gex_eav_egress_timestamp_get(int unit, uint32 port,
    uint32 *timestamp)
{
    uint32 reg_value, temp = 0;

    /* Check Valid Status */
    SOC_IF_ERROR_RETURN(
        REG_READ_TM_STAMP_STATUSr(unit, &reg_value));
    soc_TM_STAMP_STATUSr_field_get(unit, &reg_value, 
        VALID_STATUSf, &temp);
    
    if ((temp & (0x1 << port)) == 0) {
        return SOC_E_EMPTY;
    }

    /* Get Egress Time STamp Value */
    if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit) ||
        SOC_IS_LOTUS(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT)        
        SOC_IF_ERROR_RETURN(
            REG_READ_RESE_EGRESS_TM_STAMPr(unit, port, &reg_value));
#endif
    } else if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) ||
               SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_BLACKBIRD2_SUPPORT) ||\
    defined(BCM_STARFIGHTER3_SUPPORT)    
        SOC_IF_ERROR_RETURN(
            REG_READ_AVB_EGRESS_TM_STAMPr(unit, port, &reg_value));
#endif
    }
    *timestamp = reg_value;
    return SOC_E_NONE;
}


#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT)
int 
_drv_vulcan_eav_time_sync_set(int unit, uint32 type, uint32 p0, uint32 p1)
{
    int rv = SOC_E_NONE;
    uint32 reg_value, temp;
    
    switch (type) {
        case DRV_EAV_TIME_SYNC_TIME_BASE:
            SOC_IF_ERROR_RETURN(
                REG_READ_RESE_TM_BASEr(unit, &reg_value));
            temp = p0;
            soc_RESE_TM_BASEr_field_set(unit, &reg_value, 
                TM_BASEf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_RESE_TM_BASEr(unit, &reg_value));
            break;
        case DRV_EAV_TIME_SYNC_TIME_ADJUST:
            SOC_IF_ERROR_RETURN(
                REG_READ_RESE_TM_ADJr(unit, &reg_value));
            if ((p0 > EAV_GEX_MAX_TICK_INC) || 
                (p1 > EAV_GEX_MAX_TICK_ADJUST_PERIOD)) {
                return SOC_E_PARAM;
            }
            temp = p0;
            soc_RESE_TM_ADJr_field_set(unit, &reg_value, 
                TM_INCf, &temp);
            temp = p1;
            soc_RESE_TM_ADJr_field_set(unit, &reg_value, 
                TM_ADJ_PRDf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_RESE_TM_ADJr(unit, &reg_value));
            break;
        case DRV_EAV_TIME_SYNC_TICK_COUNTER:
            SOC_IF_ERROR_RETURN(
                REG_READ_RESE_SLOT_TICK_CNTRr(unit, &reg_value));
            if (p0 > EAV_GEX_MAX_TICK_ONE_SLOT) {
                return SOC_E_PARAM;
            }
            temp = p0;
            soc_RESE_SLOT_TICK_CNTRr_field_set(unit, &reg_value, 
                TICK_CNTRf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_RESE_SLOT_TICK_CNTRr(unit, &reg_value));
            break;
        case DRV_EAV_TIME_SYNC_SLOT_NUMBER:
            SOC_IF_ERROR_RETURN(
                REG_READ_RESE_SLOT_TICK_CNTRr(unit, &reg_value));
            if (p0 > EAV_GEX_MAX_SLOT_NUMBER) {
                return SOC_E_PARAM;
            }
            temp = p0;
            soc_RESE_SLOT_TICK_CNTRr_field_set(unit, &reg_value, 
                SLOT_NUMf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_RESE_SLOT_TICK_CNTRr(unit, &reg_value));
            break;
        case DRV_EAV_TIME_SYNC_MACRO_SLOT_PERIOD:
            SOC_IF_ERROR_RETURN(
                REG_READ_RESE_SLOT_ADJr(unit, &reg_value));
            switch (p0) {
                case 1:
                    temp = 0;
                    break;
                case 2:
                    temp = 1;
                    break;
                case 4:
                    temp = 2;
                    break;
                default:
                    rv = SOC_E_PARAM;
                    return rv;
            }
            soc_RESE_SLOT_ADJr_field_set(unit, &reg_value, 
                MCRO_SLOT_PRDf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_RESE_SLOT_ADJr(unit, &reg_value));
            break;
        case DRV_EAV_TIME_SYNC_SLOT_ADJUST:
            
            SOC_IF_ERROR_RETURN(
                REG_READ_RESE_SLOT_ADJr(unit, &reg_value));
            switch (p0) {
                case 3125:
                    temp = 0;
                    break;
                case 3126:
                    temp = 1;
                    break;
                case 3124:
                    temp = 2;
                    break;
                default:
                    rv = SOC_E_PARAM;
                    return rv;
            }
            soc_RESE_SLOT_ADJr_field_set(unit, &reg_value, 
                SLOT_ADJf, &temp);
            if (p1 >= 16) {
                rv =  SOC_E_PARAM;
                return rv;
            }
            temp = p1;
            soc_RESE_SLOT_ADJr_field_set(unit, &reg_value, 
                SLOT_ADJ_PRDf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_RESE_SLOT_ADJr(unit, &reg_value));
            break;
        default:
            rv = SOC_E_PARAM;
    }

    return rv;
}

int 
_drv_vulcan_eav_time_sync_get(int unit, uint32 type, uint32 *p0, uint32 *p1)
{
    int rv = SOC_E_NONE;
    uint32 reg_value, temp = 0;
    
    switch (type) {
        case DRV_EAV_TIME_SYNC_TIME_BASE:
            SOC_IF_ERROR_RETURN(
                REG_READ_RESE_TM_BASEr(unit, &reg_value));
            soc_RESE_TM_BASEr_field_get(unit, &reg_value, 
                TM_BASEf, &temp);
            *p0 = temp;
            break;
        case DRV_EAV_TIME_SYNC_TIME_ADJUST:
            SOC_IF_ERROR_RETURN(
                REG_READ_RESE_TM_ADJr(unit, &reg_value));
            soc_RESE_TM_ADJr_field_get(unit, &reg_value, 
                TM_INCf, &temp);
            *p0 = temp;
            soc_RESE_TM_ADJr_field_get(unit, &reg_value, 
                TM_ADJ_PRDf, &temp);
            *p1 = temp;
            break;
        case DRV_EAV_TIME_SYNC_TICK_COUNTER:
            SOC_IF_ERROR_RETURN(
                REG_READ_RESE_SLOT_TICK_CNTRr(unit, &reg_value));
            soc_RESE_SLOT_TICK_CNTRr_field_get(unit, &reg_value, 
                TICK_CNTRf, &temp);
            *p0 = temp;
            break;
        case DRV_EAV_TIME_SYNC_SLOT_NUMBER:
            SOC_IF_ERROR_RETURN(
                REG_READ_RESE_SLOT_TICK_CNTRr(unit, &reg_value));
            soc_RESE_SLOT_TICK_CNTRr_field_get(unit, &reg_value, 
                SLOT_NUMf, &temp);
            *p0 = temp;
            break;
        case DRV_EAV_TIME_SYNC_MACRO_SLOT_PERIOD:
            SOC_IF_ERROR_RETURN(
                REG_READ_RESE_SLOT_ADJr(unit, &reg_value));
            soc_RESE_SLOT_ADJr_field_get(unit, &reg_value, 
                MCRO_SLOT_PRDf, &temp);
            switch(temp) {
                case 0:
                    *p0 = 1;
                    break;
                case 1:
                    *p0 = 2;
                    break;
                case 2:
                    *p0 = 4;
                    break;
                default:
                    rv = SOC_E_INTERNAL;
                    return rv;
            }
            break;
        case DRV_EAV_TIME_SYNC_SLOT_ADJUST:
            SOC_IF_ERROR_RETURN(
                REG_READ_RESE_SLOT_ADJr(unit, &reg_value));
            soc_RESE_SLOT_ADJr_field_get(unit, &reg_value, 
                SLOT_ADJf, &temp);
            switch (temp) {
                case 0:
                    *p0 = 3125;
                    break;
                case 1:
                    *p0 = 3126;
                    break;
                case 2:
                    *p0 = 3124;
                    break;
                default:
                    return SOC_E_INTERNAL;
            }
            soc_RESE_SLOT_ADJr_field_get(unit, &reg_value, 
                SLOT_ADJ_PRDf, &temp);
            *p1 = temp;
            break;
        default:
            rv = SOC_E_PARAM;
    }

    return rv;
}
#endif

#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_BLACKBIRD2_SUPPORT) ||\
    defined(BCM_STARFIGHTER3_SUPPORT)

int 
_drv_starfighter_eav_time_sync_set(int unit, uint32 type, uint32 p0, uint32 p1)
{
    int rv = SOC_E_NONE;
    uint32 reg_value, temp;
    
    switch (type) {
        case DRV_EAV_TIME_SYNC_TIME_BASE:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_TM_BASEr(unit, &reg_value));
            temp = p0;
            soc_AVB_TM_BASEr_field_set(unit, &reg_value, TM_BASEf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_AVB_TM_BASEr(unit, &reg_value));
            break;
        case DRV_EAV_TIME_SYNC_TIME_ADJUST:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_TM_ADJr(unit, &reg_value));
            if ((p0 > EAV_GEX_MAX_TICK_INC) || 
                (p1 > EAV_GEX_MAX_TICK_ADJUST_PERIOD)) {
                return SOC_E_PARAM;
            }
            temp = p0;
            soc_AVB_TM_ADJr_field_set(unit, &reg_value, TM_INCf, &temp);
            temp = p1;
            soc_AVB_TM_ADJr_field_set(unit, &reg_value, TM_ADJ_PRDf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_AVB_TM_ADJr(unit, &reg_value));
            break;
        case DRV_EAV_TIME_SYNC_TICK_COUNTER:
            if (SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_STARFIGHTER3_SUPPORT)
                SOC_IF_ERROR_RETURN(
                    REG_READ_AVB_SLOT_TICKr(unit, &reg_value));
                if (p0 > EAV_GEX_MAX_TICK_ONE_SLOT) {
                    return SOC_E_PARAM;
                }
                temp = p0;
                soc_AVB_SLOT_TICKr_field_set(
                    unit, &reg_value, TICK_CNTRf, &temp);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_AVB_SLOT_TICKr(unit, &reg_value));

#endif
            } else if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit)) {
#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_BLACKBIRD2_SUPPORT)
                SOC_IF_ERROR_RETURN(
                    REG_READ_AVB_SLOT_TICK_CNTRr(unit, &reg_value));
                if (p0 > EAV_GEX_MAX_TICK_ONE_SLOT) {
                    return SOC_E_PARAM;
                }
                temp = p0;
                soc_AVB_SLOT_TICK_CNTRr_field_set(
                    unit, &reg_value, TICK_CNTRf, &temp);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_AVB_SLOT_TICK_CNTRr(unit, &reg_value));
#endif
            }
            break;
        case DRV_EAV_TIME_SYNC_SLOT_NUMBER:
            if (SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_STARFIGHTER3_SUPPORT)
                SOC_IF_ERROR_RETURN(
                    REG_READ_AVB_SLOT_TICKr(unit, &reg_value));
                if (p0 > EAV_GEX_MAX_SLOT_NUMBER) {
                    return SOC_E_PARAM;
                }
                temp = p0;
                soc_AVB_SLOT_TICKr_field_set(
                    unit, &reg_value, SLOT_NUMf, &temp);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_AVB_SLOT_TICKr(unit, &reg_value));
#endif
            } else if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit)) {
#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_BLACKBIRD2_SUPPORT)
                SOC_IF_ERROR_RETURN(
                    REG_READ_AVB_SLOT_TICK_CNTRr(unit, &reg_value));
                if (p0 > EAV_GEX_MAX_SLOT_NUMBER) {
                    return SOC_E_PARAM;
                }
                temp = p0;
                soc_AVB_SLOT_TICK_CNTRr_field_set(
                    unit, &reg_value, SLOT_NUMf, &temp);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_AVB_SLOT_TICK_CNTRr(unit, &reg_value));
#endif
            }
            break;
        case DRV_EAV_TIME_SYNC_MACRO_SLOT_PERIOD:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_SLOT_ADJr(unit, &reg_value));
            switch (p0) {
                case 1:
                    temp = 0;
                    break;
                case 2:
                    temp = 1;
                    break;
                case 4:
                    temp = 2;
                    break;
                default:
                    rv = SOC_E_PARAM;
                    return rv;
            }
            soc_AVB_SLOT_ADJr_field_set(
                unit, &reg_value, MCRO_SLOT_PRDf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_AVB_SLOT_ADJr(unit, &reg_value));
            break;
        case DRV_EAV_TIME_SYNC_SLOT_ADJUST:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_SLOT_ADJr(unit, &reg_value));
            switch (p0) {
                case 3125:
                    temp = 0;
                    break;
                case 3126:
                    temp = 1;
                    break;
                case 3124:
                    temp = 2;
                    break;
                default:
                    rv = SOC_E_PARAM;
                    return rv;
            }
            soc_AVB_SLOT_ADJr_field_set(
                unit, &reg_value, SLOT_ADJf, &temp);
            if (p1 >= 16) {
                rv =  SOC_E_PARAM;
                return rv;
            }
            temp = p1;
            soc_AVB_SLOT_ADJr_field_set(
                unit, &reg_value, SLOT_ADJ_PRDf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_AVB_SLOT_ADJr(unit, &reg_value));
            break;
        default:
            rv = SOC_E_PARAM;
    }

    return rv;
}

int 
_drv_starfighter_eav_time_sync_get(int unit, uint32 type, uint32 *p0, uint32 *p1)
{
    int rv = SOC_E_NONE;
    uint32 reg_value, temp = 0;
    
    switch (type) {
        case DRV_EAV_TIME_SYNC_TIME_BASE:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_TM_BASEr(unit, &reg_value));
            soc_AVB_TM_BASEr_field_get(unit, &reg_value, TM_BASEf, &temp);
            *p0 = temp;
            break;
        case DRV_EAV_TIME_SYNC_TIME_ADJUST:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_TM_ADJr(unit, &reg_value));
            soc_AVB_TM_ADJr_field_get(unit, &reg_value, TM_INCf, &temp);
            *p0 = temp;
            soc_AVB_TM_ADJr_field_get(unit, &reg_value, TM_ADJ_PRDf, &temp);
            *p1 = temp;
            break;
        case DRV_EAV_TIME_SYNC_TICK_COUNTER:
            if (SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_STARFIGHTER3_SUPPORT)
                SOC_IF_ERROR_RETURN(
                    REG_READ_AVB_SLOT_TICKr(unit, &reg_value));
                soc_AVB_SLOT_TICKr_field_get(
                    unit, &reg_value, TICK_CNTRf, &temp);
                *p0 = temp;
#endif
            } else if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit)) {
#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_BLACKBIRD2_SUPPORT)
                SOC_IF_ERROR_RETURN(
                    REG_READ_AVB_SLOT_TICK_CNTRr(unit, &reg_value));
                soc_AVB_SLOT_TICK_CNTRr_field_get(
                    unit, &reg_value, TICK_CNTRf, &temp);
                *p0 = temp;
#endif
            }
            break;
        case DRV_EAV_TIME_SYNC_SLOT_NUMBER:
            if (SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_STARFIGHTER3_SUPPORT)
                SOC_IF_ERROR_RETURN(
                    REG_READ_AVB_SLOT_TICKr(unit, &reg_value));
                soc_AVB_SLOT_TICKr_field_get(
                    unit, &reg_value, SLOT_NUMf, &temp);
                *p0 = temp;
#endif
            } else if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit)) {
#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_BLACKBIRD2_SUPPORT)
                SOC_IF_ERROR_RETURN(
                    REG_READ_AVB_SLOT_TICK_CNTRr(unit, &reg_value));
                soc_AVB_SLOT_TICK_CNTRr_field_get(
                    unit, &reg_value, SLOT_NUMf, &temp);
                *p0 = temp;
#endif
            }
            break;
        case DRV_EAV_TIME_SYNC_MACRO_SLOT_PERIOD:
             SOC_IF_ERROR_RETURN(
                REG_READ_AVB_SLOT_ADJr(unit, &reg_value));
            soc_AVB_SLOT_ADJr_field_get(
                unit, &reg_value, MCRO_SLOT_PRDf, &temp);
            switch(temp) {
                case 0:
                    *p0 = 1;
                    break;
                case 1:
                    *p0 = 2;
                    break;
                case 2:
                    *p0 = 4;
                    break;
                default:
                    rv = SOC_E_INTERNAL;
                    return rv;
            }
            break;
        case DRV_EAV_TIME_SYNC_SLOT_ADJUST:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_SLOT_ADJr(unit, &reg_value));
            soc_AVB_SLOT_ADJr_field_get(
                unit, &reg_value, SLOT_ADJf, &temp);
            switch (temp) {
                case 0:
                    *p0 = 3125;
                    break;
                case 1:
                    *p0 = 3126;
                    break;
                case 2:
                    *p0 = 3124;
                    break;
                default:
                    return SOC_E_INTERNAL;
            }
            soc_AVB_SLOT_ADJr_field_get(
                unit, &reg_value, SLOT_ADJ_PRDf, &temp);
            *p1 = temp;
            break;
        default:
            rv = SOC_E_PARAM;
    }

    return rv;
}

#endif


int 
drv_gex_eav_time_sync_set(int unit, uint32 type, uint32 p0, uint32 p1)
{
    int rv = SOC_E_NONE;

    if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)|| 
        SOC_IS_LOTUS(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT)        
        rv = _drv_vulcan_eav_time_sync_set(unit, type, p0, p1);
#endif
    } else if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) ||
               SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_BLACKBIRD2_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
        rv = _drv_starfighter_eav_time_sync_set(unit, type, p0, p1);
#endif
    }
    
    return rv;
}

int 
drv_gex_eav_time_sync_get(int unit, uint32 type, uint32 *p0, uint32 *p1)
{
    int rv = SOC_E_NONE;

    if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)|| 
        SOC_IS_LOTUS(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT)        
        rv = _drv_vulcan_eav_time_sync_get(unit, type, p0, p1);
#endif
    } else if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) ||
               SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_BLACKBIRD2_SUPPORT) ||\
    defined(BCM_STARFIGHTER3_SUPPORT)
        rv = _drv_starfighter_eav_time_sync_get(unit, type, p0, p1);
#endif
    }
    
    return rv;
}

#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT) 
int 
_drv_vulcan_eav_queue_control_set(int unit, 
    uint32 port, uint32 type, uint32 param)
{
    int rv = SOC_E_NONE;
    uint32 reg_value, temp;
    
    switch (type) {
        case DRV_EAV_QUEUE_Q4_BANDWIDTH:
            /* Q4 BW maxmum value = 16383(0x3fff) */
            if (param > EAV_GEX_MAX_BANDWIDTH_VALUE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_gex_eav_queue_control_set : BW unsupported. \n")));
                return  SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN(
                REG_READ_RESE_C4_BW_CNTLr(unit, port, &reg_value));
            temp = param;
            soc_RESE_C4_BW_CNTLr_field_set(unit, &reg_value, 
                C4_BWf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_RESE_C4_BW_CNTLr(unit, port, &reg_value));
            break;
        case DRV_EAV_QUEUE_Q5_BANDWIDTH:
            /* Q5 BW maxmum value = 16383(0x3fff) */
    /*    coverity[unsigned_compare]    */
            if (param > EAV_GEX_MAX_BANDWIDTH_VALUE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_gex_eav_queue_control_set : BW unsupported. \n")));
                return  SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN(
                REG_READ_RESE_C5_BW_CNTLr(unit, port, &reg_value));
            temp = param;
            soc_RESE_C5_BW_CNTLr_field_set(unit, &reg_value, 
                C5_BWf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_RESE_C5_BW_CNTLr(unit, port, &reg_value));
            break;
        case DRV_EAV_QUEUE_Q5_WINDOW:
            SOC_IF_ERROR_RETURN(
                REG_READ_RESE_C5_BW_CNTLr(unit, port, &reg_value));
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            soc_RESE_C5_BW_CNTLr_field_set(unit, &reg_value, 
                C5_WNDWf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_RESE_C5_BW_CNTLr(unit, port, &reg_value));
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}
int 
_drv_vulcan_eav_queue_control_get(int unit, 
    uint32 port, uint32 type, uint32 *param)
{
    int rv = SOC_E_NONE;
    uint32 reg_value, temp = 0, max_value = 0;
    soc_field_info_t    *finfop = NULL;
    
    switch (type) {
        case DRV_EAV_QUEUE_Q4_BANDWIDTH:
            SOC_IF_ERROR_RETURN(
                REG_READ_RESE_C4_BW_CNTLr(unit, port, &reg_value));
            soc_RESE_C4_BW_CNTLr_field_get(unit, &reg_value, 
                C4_BWf, &temp);
            *param = temp;
            break;
        case DRV_EAV_QUEUE_Q5_BANDWIDTH:
            SOC_IF_ERROR_RETURN(
                REG_READ_RESE_C5_BW_CNTLr(unit, port, &reg_value));
            soc_RESE_C5_BW_CNTLr_field_get(unit, &reg_value, 
                C5_BWf, &temp);
            *param = temp;
            break;
        case DRV_EAV_QUEUE_Q5_WINDOW:
            SOC_IF_ERROR_RETURN(
                REG_READ_RESE_C5_BW_CNTLr(unit, port, &reg_value));
            soc_RESE_C5_BW_CNTLr_field_get(unit, &reg_value, 
                C5_WNDWf, &temp);
            if (temp){
                *param = TRUE;
            } else {
                *param = FALSE;
            }
            break;
        case DRV_EAV_QUEUE_Q4_BANDWIDTH_MAX_VALUE:
            /*
              * Get the maximum valid bandwidth value for EAV Class 4 (macro slot time = 1)
              *
              * C4_Bandwidth(bytes/slot) = 
              *     Max_value(kbits/sec) * 1024 / (8 * macro slot time * 1000)
              *
              * C4_Bandwidth (14 bits) = 0x3fff
              * Max_value = (((1<<14) * 8 * macro slot time* 1000)/(1024)) - 1
              */
            SOC_FIND_FIELD(INDEX(C4_BWf),
                SOC_REG_INFO(unit, INDEX(RESE_C4_BW_CNTLr)).fields,
                SOC_REG_INFO(unit, INDEX(RESE_C4_BW_CNTLr)).nFields,
                finfop);
            assert(finfop);
            if (!finfop) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_gex_eav_queue_control_get : Invalid field of C4_BW. \n")));
                return  SOC_E_PARAM;
            }

            temp = (1 << finfop->len);
            max_value = ((temp * 8 * 1 * 1000) / (1024)) - 1;
            *param = max_value;
            break;
        case DRV_EAV_QUEUE_Q5_BANDWIDTH_MAX_VALUE:
            /*
              * Get the maximum valid bandwidth value for EAV Class 5
              *
              * Class 5 slot time is 125 us.
              * C5_Bandwidth(bytes/125us) = Max_value(kbits/sec) * 1024 / (8 * 8000)
              *
              * C5_Bandwidth (14 bits) = 0x3fff
              * Max_value = (((1<<14) * 8 * 8000)/(1024)) - 1
              */
            SOC_FIND_FIELD(INDEX(C5_BWf),
                SOC_REG_INFO(unit, INDEX(RESE_C5_BW_CNTLr)).fields,
                SOC_REG_INFO(unit, INDEX(RESE_C5_BW_CNTLr)).nFields,
                finfop);
            assert(finfop);
            if (!finfop) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_gex_eav_queue_control_get : Invalid field of C5_BW. \n")));
                return  SOC_E_PARAM;
            }

            temp = (1 << finfop->len);
            max_value = ((temp * 8 * 8000) / 1024) - 1;
            *param = max_value;
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

#endif /* BCM_VULCAN_SUPPORT || BLACKBIRD || LOTUS */

#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_BLACKBIRD2_SUPPORT)
int 
_drv_starfighter_eav_queue_control_set(int unit, 
    uint32 port, uint32 type, uint32 param)
{
    int rv = SOC_E_NONE;
    uint32 reg_value, temp;
    
    switch (type) {
        case DRV_EAV_QUEUE_Q4_BANDWIDTH:
            /* Q4 BW maxmum value = 16383(0x3fff) */
            if (param > EAV_GEX_MAX_BANDWIDTH_VALUE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_starfighter_eav_queue_control_set : BW unsupported. \n")));
                return  SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_C4_BW_CNTLr(unit, port, &reg_value));
            temp = param;
            soc_AVB_C4_BW_CNTLr_field_set(
                unit, &reg_value, C4_BWf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_AVB_C4_BW_CNTLr(unit, port, &reg_value));
            break;
        case DRV_EAV_QUEUE_Q5_BANDWIDTH:
            /* Q5 BW maxmum value = 16383(0x3fff) */
            /*    coverity[unsigned_compare]    */
            if (param > EAV_GEX_MAX_BANDWIDTH_VALUE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_starfighter_eav_queue_control_set : BW unsupported. \n")));
                return  SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_C5_BW_CNTLr(unit, port, &reg_value));
            temp = param;
            soc_AVB_C5_BW_CNTLr_field_set(
                unit, &reg_value, C5_BWf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_AVB_C5_BW_CNTLr(unit, port, &reg_value));
            break;
        case DRV_EAV_QUEUE_Q5_WINDOW:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_C5_BW_CNTLr(unit, port, &reg_value));
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            soc_AVB_C5_BW_CNTLr_field_set(
                unit, &reg_value, C5_WNDWf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_AVB_C5_BW_CNTLr(unit, port, &reg_value));
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}
int 
_drv_starfighter_eav_queue_control_get(int unit, 
    uint32 port, uint32 type, uint32 *param)
{
    int rv = SOC_E_NONE;
    uint32 reg_value, temp = 0, max_value = 0;
    soc_field_info_t    *finfop = NULL;
    
    switch (type) {
        case DRV_EAV_QUEUE_Q4_BANDWIDTH:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_C4_BW_CNTLr(unit, port, &reg_value));
            soc_AVB_C4_BW_CNTLr_field_get(
                unit, &reg_value, C4_BWf, &temp);
            *param = temp;
            break;
        case DRV_EAV_QUEUE_Q5_BANDWIDTH:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_C5_BW_CNTLr(unit, port, &reg_value));
            soc_AVB_C5_BW_CNTLr_field_get(
                unit, &reg_value, C5_BWf, &temp);
            *param = temp;
            break;
        case DRV_EAV_QUEUE_Q5_WINDOW:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_C5_BW_CNTLr(unit, port, &reg_value));
            soc_AVB_C5_BW_CNTLr_field_get(
                unit, &reg_value, C5_WNDWf, &temp);
            if (temp){
                *param = TRUE;
            } else {
                *param = FALSE;
            }
            break;
        case DRV_EAV_QUEUE_Q4_BANDWIDTH_MAX_VALUE:
            /*
              * Get the maximum valid bandwidth value for EAV Class 4 (macro slot time = 1)
              *
              * C4_Bandwidth(bytes/slot) = 
              *     Max_value(kbits/sec) * 1024 / (8 * macro slot time * 1000)
              *
              * C4_Bandwidth (14 bits) = 0x3fff
              * Max_value = (((1<<14) * 8 * macro slot time* 1000)/(1024)) - 1
              */
            SOC_FIND_FIELD(INDEX(C4_BWf),
                SOC_REG_INFO(unit, INDEX(AVB_C4_BW_CNTLr)).fields,
                SOC_REG_INFO(unit, INDEX(AVB_C4_BW_CNTLr)).nFields,
                finfop);
            assert(finfop);
            if (!finfop) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_gex_eav_queue_control_get : Invalid field of C4_BW. \n")));
                return  SOC_E_PARAM;
            }

            temp = (1 << finfop->len);
            max_value = ((temp * 8 * 1 * 1000) / (1024)) - 1;
            *param = max_value;
            break;
        case DRV_EAV_QUEUE_Q5_BANDWIDTH_MAX_VALUE:
            /*
              * Get the maximum valid bandwidth value for EAV Class 5
              *
              * Class 5 slot time is 125 us.
              * C5_Bandwidth(bytes/125us) = Max_value(kbits/sec) * 1024 / (8 * 8000)
              *
              * C5_Bandwidth (14 bits) = 0x3fff
              * Max_value = (((1<<14) * 8 * 8000)/(1024)) - 1
              */
            SOC_FIND_FIELD(INDEX(C5_BWf),
                SOC_REG_INFO(unit, INDEX(AVB_C5_BW_CNTLr)).fields,
                SOC_REG_INFO(unit, INDEX(AVB_C5_BW_CNTLr)).nFields,
                finfop);
            assert(finfop);
            if (!finfop) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_gex_eav_queue_control_get : Invalid field of C5_BW. \n")));
                return  SOC_E_PARAM;
            }

            temp = (1 << finfop->len);
            max_value = ((temp * 8 * 8000) / 1024) - 1;
            *param = max_value;
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}


#endif /* BCM_STARFIGHTER_SUPPORT || BLACKBIRD2 */

int 
drv_gex_eav_queue_control_set(int unit, 
    uint32 port, uint32 type, uint32 param)
{
    int rv = SOC_E_NONE;
    if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)|| 
        SOC_IS_LOTUS(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT)         
        rv = _drv_vulcan_eav_queue_control_set(unit, port, type, param);
#endif
    } else if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit)) {
#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_BLACKBIRD2_SUPPORT)
        rv = _drv_starfighter_eav_queue_control_set(unit, port, type, param);
#endif
    }

    return rv;
}

int 
drv_gex_eav_queue_control_get(int unit, 
    uint32 port, uint32 type, uint32 *param)
{
    int rv = SOC_E_NONE;
    if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)|| 
        SOC_IS_LOTUS(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT)         
        rv = _drv_vulcan_eav_queue_control_get(unit, port, type, param);
#endif
    } else if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit)) {
#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_BLACKBIRD2_SUPPORT)
        rv = _drv_starfighter_eav_queue_control_get(unit, port, type, param);
#endif
    }

    return rv;
}

int 
drv_gex_eav_time_sync_mac_set(int unit, uint8* mac, uint16 ethertype)
{
    uint32 reg_val;
    uint64 reg_val64, mac_field;
    uint32 temp;
    int rv =  SOC_E_NONE;

    /*
     * For time sync protocol, the mac should be set in Multi-address 0 register
     */

    /* 1. Set MAC and Ethertype value */
    SAL_MAC_ADDR_TO_UINT64(mac, mac_field);

    COMPILER_64_ZERO(reg_val64);
    soc_MULTIPORT_ADDR0r_field_set(unit, (uint32 *)&reg_val64, 
        MPORT_ADDRf, (uint32 *)&mac_field);
    if (ethertype) {
        temp = ethertype;
        soc_MULTIPORT_ADDR0r_field_set(unit, (uint32 *)&reg_val64, 
            MPORT_E_TYPEf, &temp);
    }
    if ((rv = REG_WRITE_MULTIPORT_ADDR0r(
        unit, (uint32 *)&reg_val64)) < 0) {
        return rv;
    }

    /* 2. Set Forward map to CPU only */
    temp  = SOC_PBMP_WORD_GET(PBMP_CMIC(unit), 0);
    reg_val = 0;
    soc_MPORTVEC0r_field_set(unit, &reg_val, 
        PORT_VCTRf, &temp);
    if ((rv = REG_WRITE_MPORTVEC0r(unit, &reg_val)) < 0) {
        return rv;
    }

    /* 3. Enable Multi-address o */
    if ((rv = REG_READ_MULTI_PORT_CTLr(unit, &reg_val)) < 0) {
        return rv;
    }
    /* Set the match condition are MAC/Ethertype */
    if (ethertype) {
        temp = DRV_MULTIPORT_CTRL_MATCH_ETYPE_ADDR;
    } else {
        temp = DRV_MULTIPORT_CTRL_MATCH_ADDR;
    }
    soc_MULTI_PORT_CTLr_field_set(unit, &reg_val, 
        MPORT_CTRL0f, &temp);
    /* Enable time stamped to CPU */
    temp = 1;
    soc_MULTI_PORT_CTLr_field_set(unit, &reg_val, 
        MPORT0_TS_ENf, &temp);   

    if ((rv = REG_WRITE_MULTI_PORT_CTLr(unit, &reg_val)) < 0) {
        return rv;
    }

    return rv;
    
}

int 
drv_gex_eav_time_sync_mac_get(int unit, uint8* mac, uint16 *ethertype)
{
    uint32 reg_val;
    uint64 reg_val64, mac_field;
    uint32 temp = 0;
    int rv =  SOC_E_NONE;

    COMPILER_64_ZERO(mac_field);
    if ((rv = REG_READ_MULTI_PORT_CTLr(unit, &reg_val)) < 0) {
        return rv;
    }
    /* Get the value of time sync enable */
    soc_MULTI_PORT_CTLr_field_get(unit, &reg_val, 
        MPORT0_TS_ENf, &temp);
    if ( temp == 0) {
        rv = SOC_E_DISABLED;
        return rv;
    }
    /* Get the Multi-address control value */
    soc_MULTI_PORT_CTLr_field_get(unit, &reg_val, 
        MPORT_CTRL0f, &temp);
    if (temp == DRV_MULTIPORT_CTRL_DISABLE) {
        rv = SOC_E_DISABLED;
        return rv;
    }

    /* Get the MAC and Ethertype value */
    COMPILER_64_ZERO(reg_val64);
    COMPILER_64_ZERO(mac_field);
    if ((rv = REG_READ_MULTIPORT_ADDR0r(
        unit, (uint32 *)&reg_val64)) < 0) {
        return rv;
    }
    soc_MULTIPORT_ADDR0r_field_get(unit, (uint32 *)&reg_val64, 
        MPORT_ADDRf, (uint32 *)&mac_field);
    SAL_MAC_ADDR_FROM_UINT64(mac, mac_field);

    soc_MULTIPORT_ADDR0r_field_get(unit, (uint32 *)&reg_val64, 
        MPORT_E_TYPEf, &temp);
    *ethertype = temp;

    return rv;
    
}


