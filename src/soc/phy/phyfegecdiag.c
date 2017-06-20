/*
 * $Id: phyfegecdiag.c,v 1.4.88.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        phycdiag.c
 * Purpose:	Cable diagnostic algorithm for default phy.
 *
 */
#if defined(BCM_ROBO_SUPPORT)
#include <sal/types.h>
#include <sal/core/thread.h>

#include <soc/phy.h>
#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/error.h>
#include <soc/phyreg.h>

#include <soc/ll.h>
#include <soc/phy/phyctrl.h>

#include "phyreg.h"
#include "phyfege.h"
#include "phy54xx.h"

int phy_cdMain(int unit,soc_port_t l1,soc_port_cable_diag_t*l2){uint16 l3;
uint16 l4;uint8 l5;phy_ctrl_t*l6;l6 = EXT_PHY_SW_STATE(unit,l1);
SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit,l6,MII_BRCM_TEST_REG,
MII_BRCM_TEST_REG_DEFAULT|MII_BRCM_TEST_ENSHA));for(l4 = 0;l4<1000;l4++){l2->
npairs = 2;}SOC_IF_ERROR_RETURN(READ_PHY_REG(unit,l6,MII_SHA_CD_SEL_REG,&l3))
;if(l3&0x0040){SOC_IF_ERROR_RETURN(READ_PHY_REG(unit,l6,MII_SHA_AUX_STAT2_REG
,&l3));l5 = ((l3&MII_SHA_AUX_STAT2_LEN)>>12)*20;if(l5>= GOOD_CABLE_LEN_TUNING
){l5-= GOOD_CABLE_LEN_TUNING;}else{l5 = 0;}l2->fuzz_len = CABLE_FUZZY_LEN1;l2
->state = l2->pair_state[0] = l2->pair_state[1] = SOC_PORT_CABLE_STATE_OK;l2
->pair_len[0] = l2->pair_len[1] = CABLE_FUZZY_LEN1+l5;SOC_IF_ERROR_RETURN(
WRITE_PHY_REG(unit,l6,MII_BRCM_TEST_REG,MII_BRCM_TEST_REG_DEFAULT));return
SOC_E_NONE;}SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit,l6,MII_SHA_CD_SEL_REG,0x0)
);SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit,l6,MII_SHA_CD_CTRL_REG,0x0));
SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit,l6,MII_SHA_CD_SEL_REG,0x0200));
SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit,l6,MII_SHA_CD_CTRL_REG,0x4824));
SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit,l6,MII_SHA_CD_CTRL_REG,0x0400));
SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit,l6,MII_SHA_CD_CTRL_REG,0x0500));
SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit,l6,MII_SHA_CD_CTRL_REG,0xC404));
SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit,l6,MII_SHA_CD_CTRL_REG,0x0100));
SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit,l6,MII_SHA_CD_CTRL_REG,0x004F));
SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit,l6,MII_SHA_CD_SEL_REG,0x0));
SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit,l6,MII_SHA_CD_CTRL_REG,0x8006));do{for
(l4 = 0;l4<1000;l4++){l2->npairs = 2;}SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit,
l6,MII_SHA_CD_SEL_REG,0x0));SOC_IF_ERROR_RETURN(READ_PHY_REG(unit,l6,
MII_SHA_CD_CTRL_REG,&l3));}while(l3&MII_SHA_CD_CTRL_START);if((l3&0x03c0)!= 
0x0300){SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit,l6,MII_BRCM_TEST_REG,
MII_BRCM_TEST_REG_DEFAULT));return SOC_E_FAIL;}l2->fuzz_len = 
CABLE_FUZZY_LEN2;switch((l3&MII_SHA_CD_CTRL_PA_STAT)>>10){case 0:l2->
pair_state[0] = SOC_PORT_CABLE_STATE_OK;break;case 1:l2->pair_state[0] = 
SOC_PORT_CABLE_STATE_OPEN;break;case 2:l2->pair_state[0] = 
SOC_PORT_CABLE_STATE_SHORT;break;default:l2->pair_state[0] = 
SOC_PORT_CABLE_STATE_UNKNOWN;break;}switch((l3&MII_SHA_CD_CTRL_PB_STAT)>>12){
case 0:l2->pair_state[1] = SOC_PORT_CABLE_STATE_OK;break;case 1:l2->
pair_state[1] = SOC_PORT_CABLE_STATE_OPEN;break;case 2:l2->pair_state[1] = 
SOC_PORT_CABLE_STATE_SHORT;break;default:l2->pair_state[1] = 
SOC_PORT_CABLE_STATE_UNKNOWN;break;}l2->state = l2->pair_state[0];if(l2->
pair_state[1]>l2->state){l2->state = l2->pair_state[1];}SOC_IF_ERROR_RETURN(
READ_PHY_REG(unit,l6,MII_SHA_CD_CTRL_REG,&l3));l2->pair_len[0] = ((l3&
MII_SHA_CD_CTRL_PA_LEN)*80)/100;l2->pair_len[1] = (((l3&
MII_SHA_CD_CTRL_PB_LEN)>>8)*80)/100;SOC_IF_ERROR_RETURN(WRITE_PHY_REG(unit,l6
,MII_BRCM_TEST_REG,MII_BRCM_TEST_REG_DEFAULT));return SOC_E_NONE;}

#endif /* BCM_ROBO_SUPPORT */
int _phy_fege_cdiag_not_empty;
