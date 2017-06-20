/*
 * $Id: jer2_jer_drv.c Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT

/*
 * Includes
 */
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
/* SAL includes */
#include <sal/appl/sal.h>

/* SOC includes */
#include <soc/uc.h>
#include <soc/error.h>
#include <soc/iproc.h>
#include <soc/ipoll.h>
#include <soc/linkctrl.h>

/*SOC DNXC includes*/
#include <soc/dnxc/legacy/dnxc_cmic.h>
#include <soc/dnxc/legacy/dnxc_iproc.h>
#include <soc/dnxc/legacy/dnxc_intr.h>
#include <soc/dnxc/legacy/dnxc_dev_feature_manager.h>

/* SOC DNX includes */
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/mbcm.h>
#include <soc/dnxc/legacy/dnxc_mem.h>

/* SOC DNX JER2_JER includes */
#include <soc/dnx/legacy/JER/jer_drv.h>
#include <soc/dnx/legacy/JER/jer_init.h>
#include <soc/dnx/legacy/JER/jer_regs.h>
#include <soc/dnx/legacy/JER/jer_egr_queuing.h>
#include <soc/dnx/legacy/JER/jer_reg_access.h>
#include <soc/dnx/legacy/JER/jer_link.h>
#include <soc/dnx/legacy/JER/jer_mgmt.h>
#include <soc/dnx/legacy/JER/jer_sbusdma_desc.h>
/* SOC DNX Arad includes */
#include <soc/dnx/legacy/ARAD/arad_chip_regs.h>
#include <soc/dnx/legacy/ARAD/arad_drv.h>
#include <soc/dnx/legacy/ARAD/arad_init.h>
#include <soc/dnx/legacy/port_sw_db.h>
#ifdef CMODEL_SERVER_MODE
#include <soc/dnx/cmodel/cmodel_reg_access.h>
#endif

/*
 * Configures soc data structures specific to Jericho
 */
int soc_dnx_get_default_config_jer2_jer(
    int unit)
{

    soc_dnx_config_jer2_jer_t *jer2_jer;

    DNXC_INIT_FUNC_DEFS;

    jer2_jer = SOC_DNX_JER2_JER_CONFIG(unit);

    /* Call special jer2_jer/jer2_qax functions to fix soc_dnx_defines_t
     * (in case of qmx (jer2_jer) or kalia (jer2_qax) */
    DNXC_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_fabric_link_config_ovrd,(unit)));



    /* Already reset in SOC attach */
    sal_memset(jer2_jer, 0, sizeof(soc_dnx_config_jer2_jer_t));

exit:
    DNXC_FUNC_RETURN;
}

/* Configure CMIC. */
int soc_jer2_jer_init_reset_cmic_regs(
    int unit)
{
    uint32 core_freq = 0x0;
    uint32 rval = 0;
    int schan_timeout = 0x0;
    int dividend, divisor;
    int mdio_int_freq, mdio_delay;

    DNXC_INIT_FUNC_DEFS;

    /*
     * Map the blocks to their Sbus rings.
     * SBUS ring map:
     * Ring 2:
     * Ring 3:
     * Ring 5:
     * Ring 7:
     */
    if (SOC_IS_QUX(unit)) {
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_0_7r(unit, 0x00222227));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_8_15r(unit, 0x24442220));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_16_23r(unit, 0x22222222));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_24_31r(unit, 0x53333222));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_32_39r(unit, 0x55555555));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_40_47r(unit, 0x32333335));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_48_55r(unit, 0x00000022));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_56_63r(unit, 0x00000000));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_64_71r(unit, 0x00000000));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_72_79r(unit, 0x00000000));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_80_87r(unit, 0x00000000));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_88_95r(unit, 0x00000000));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_96_103r(unit, 0x00000000));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_104_111r(unit, 0x00000000));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_112_119r(unit, 0x00000000));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_120_127r(unit, 0x00000000));
    } else if (SOC_IS_QAX(unit)) {
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_0_7r(unit, 0x00022227));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_8_15r(unit, 0x00333220));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_16_23r(unit, 0x22022000));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_24_31r(unit, 0x22222022));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_32_39r(unit, 0x00222222));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_40_47r(unit, 0x02000000));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_48_55r(unit, 0x30330002));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_56_63r(unit, 0x33033333));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_64_71r(unit, 0x55555553));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_72_79r(unit, 0x55555555));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_80_87r(unit, 0x55555555));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_88_95r(unit, 0x55555555));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_96_103r(unit, 0x00005555));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_104_111r(unit, 0x22222000));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_112_119r(unit, 0x02044402));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_120_127r(unit, 0x00000002));

    } else if (SOC_IS_JERICHO_PLUS_A0(unit)) {
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_0_7r(unit, 0x04444447));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_8_15r(unit, 0x22222334));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_16_23r(unit, 0x33433222));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_24_31r(unit, 0x33333333));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_32_39r(unit, 0x22222233));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_40_47r(unit, 0x22222222));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_48_55r(unit, 0x55555522));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_56_63r(unit, 0x44444665));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_64_71r(unit, 0x44444444));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_72_79r(unit, 0x66666666));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_80_87r(unit, 0x66666666));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_88_95r(unit, 0x66666666));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_96_103r(unit, 0x66666666));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_104_111r(unit, 0x66226666));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_112_119r(unit, 0x44555066));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_120_127r(unit, 0x00004633));

        } else {

        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_0_7r(unit, 0x04444447)); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_8_15r(unit, 0x22222334));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_16_23r(unit, 0x33433222));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_24_31r(unit, 0x33333333));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_32_39r(unit, 0x22222233));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_40_47r(unit, 0x55222222));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_48_55r(unit, 0x46655555));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_56_63r(unit, 0x42444444));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_64_71r(unit, 0x66666664));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_72_79r(unit, 0x66666666));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_80_87r(unit, 0x66666666));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_88_95r(unit, 0x66666666));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_96_103r(unit, 0x66666666));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_104_111r(unit, 0x55220666));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_112_119r(unit, 0x62633445));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_120_127r(unit, 0x20000000));

        /* LED PROCESSOR */  

        /* setting Jericho ports status scan values */  
        /* In order to match the phisical order in led data memory, */
        /* i.e. port1 resides in index 1, port2 in index2 etc...,  */
        /* LED processor 0 - will handle ports  1-24.  port 1  resides in index 1 in data ram */
        /* LED processor 1 - will handle ports 25-84.  port 25 resides in index 1 in data ram */
        /* LED processor 2 - will handle ports 85-144. port 85 resides in index 1 in data ram */

        /* PMH ports chain is connected to LED0, 24 ports */  
        /* Phisical ports status chain to LED0 is following, port4 resides in port0 place:*/        
        /* 21->22->23->24->13->14->15->16->17->18->19->20->9->10->11->12->5->6->7->8->1->2->3->4 */
        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_0f, 4); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_1f, 3); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_2f, 2); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_3f, 1); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_0_3r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_4f, 8); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_5f, 7); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_6f, 6); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_7f, 5); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_4_7r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_8f,  12); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_9f,  11); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_10f, 10); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_11f,  9); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_8_11r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_12f, 20); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_13f, 19); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_14f, 18); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_15f, 17); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_12_15r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_16f, 16); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_17f, 15); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_18f, 14); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_19f, 13); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_16_19r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_20f, 24); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_21f, 23); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_22f, 22); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_23f, 21); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_20_23r(unit,   rval));

        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_24_27r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_28_31r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_32_35r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_36_39r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_40_43r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_44_47r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_48_51r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_52_55r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_56_59r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_60_63r(unit, 0x0));

        /* PML0 ports chain is connected to LED1, 24 ports */  
        /* Phisical ports status chain to LED1 is following, port36 resides in port0 place:*/        
        /* 69->73->77->81->53->57->61->65->37->41->45->49->29->30->31->32->25->26->27->28->33->34->35->36 */
        /* index in phisical LED DRAM memory port should reside (reduce 24 from phisical port number): */        
        /* 45->49->53->57->29->33->37->41->13->17->21->25-> 5-> 6-> 7-> 8-> 1-> 2-> 3-> 4-> 9->10->11->12 */
        
        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_0f, 12); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_1f, 11); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_2f, 10); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_3f,  9); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_0_3r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_4f, 4); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_5f, 3); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_6f, 2); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_7f, 1); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_4_7r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_8f,  8); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_9f,  7); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_10f, 6); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_11f, 5); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_8_11r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_12f, 25); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_13f, 21); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_14f, 17); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_15f, 13); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_12_15r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_16f, 41); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_17f, 37); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_18f, 33); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_19f, 29); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_16_19r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_20f, 57); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_21f, 53); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_22f, 49); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_23f, 45); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_20_23r(unit,   rval));

        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_24_27r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_28_31r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_32_35r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_36_39r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_40_43r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_44_47r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_48_51r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_52_55r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_56_59r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_60_63r(unit, 0x0));

        /* PML1 ports chain is connected to LED2, 24 ports */  
        /* Phisical ports status chain to LED1 is following, port36 resides in port0 place:*/        
        /* 129->133->137->141->113->117->121->125->97->101->105->109->89->90->91->92->85->86->87->88->93->94->95->96 */
        /* index in phisical LED DRAM memory port should reside (reduce 84 from phisical port number): */        
        /*  45-> 49-> 53-> 57-> 29-> 33-> 37-> 41->13-> 17-> 21-> 25-> 5-> 6-> 7-> 8-> 1-> 2-> 3-> 4-> 9->10->11->12 */

        
        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_0f, 12); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_1f, 11); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_2f, 10); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_3f,  9); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_0_3r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_4f, 4); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_5f, 3); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_6f, 2); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_7f, 1); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_4_7r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_8f,  8); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_9f,  7); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_10f, 6); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_11f, 5); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_8_11r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_12f, 25); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_13f, 21); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_14f, 17); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_15f, 13); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_12_15r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_16f, 41); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_17f, 37); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_18f, 33); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_19f, 29); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_16_19r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_20f, 57); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_21f, 53); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_22f, 49); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_23f, 45); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_20_23r(unit,   rval));

        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_24_27r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_28_31r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_32_35r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_36_39r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_40_43r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_44_47r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_48_51r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_52_55r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_56_59r(unit, 0x0));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_60_63r(unit, 0x0));

        /* setting Jericho ports status scan values */  
        /*               bits 9-4 (scan delay); bits 3-1 (scan port delay); bit 0 (enable) */    
        /* processor 0 :        32                              5               0          */
        /* processor 1 :        40                              5               0          */
        /* processor 2 :        40                              5               0          */
   
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_CTRLr(unit, 0x20a));  
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_CTRLr(unit, 0x28a));  
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_CTRLr(unit, 0x28a));  


        /* Required led clock period is 200 ns and switch clock runs at 720MHz.  */    
        /* value = Clk freq * (period/2) = (720 * 10^6)*(100 * 10^-9) = 72       */
        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP0_CLK_DIVr, &rval, LEDCLK_HALF_PERIODf, 72); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_CLK_DIVr(unit, rval)); 
        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP1_CLK_DIVr, &rval, LEDCLK_HALF_PERIODf, 72); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_CLK_DIVr(unit, rval)); 
        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP2_CLK_DIVr, &rval, LEDCLK_HALF_PERIODf, 72); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_CLK_DIVr(unit, rval)); 


        /* Required refresh period is 30 ms and switch clock period is ~1.388ns(720MHz) */  
        /* Then the value should be: (30*10^-3)/(1.388...*10^-9) = 21600000 = 0x1499700 Timeout value in switch clocks */
        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP0_CLK_PARAMSr, &rval, REFRESH_CYCLE_PERIODf, 0x1499700); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_CLK_PARAMSr(unit, rval)); 
        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP1_CLK_PARAMSr, &rval, REFRESH_CYCLE_PERIODf, 0x1499700); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_CLK_PARAMSr(unit, rval)); 
        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP2_CLK_PARAMSr, &rval, REFRESH_CYCLE_PERIODf, 0x1499700); 
        DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_CLK_PARAMSr(unit, rval)); 
         
    }

    /* Set SBUS timeout */
    DNXC_IF_ERR_EXIT(soc_jer2_arad_core_frequency_config_get(unit, SOC_JER2_JER_CORE_FREQ_KHZ_DEFAULT, &core_freq));
    DNXC_IF_ERR_EXIT(soc_jer2_arad_schan_timeout_config_get(unit, &schan_timeout));
    DNXC_IF_ERR_EXIT(soc_dnxc_cmic_sbus_timeout_set(unit, core_freq, schan_timeout));

    /* Mdio - internal*/

    /*Dividend values*/

    dividend = soc_property_get(unit, spn_RATE_INT_MDIO_DIVIDEND, -1); 
    if (dividend == -1) 
    {
        /*default value*/
        dividend =  SOC_DNX_IMP_DEFS_GET(unit, mdio_int_dividend_default);

    }

    divisor = soc_property_get(unit, spn_RATE_INT_MDIO_DIVISOR, -1); 
    if (divisor == -1) 
    {
        /*Calc default dividend and divisor*/
        mdio_int_freq = SOC_DNX_IMP_DEFS_GET(unit, mdio_int_freq_default);
        divisor = core_freq * dividend / (2* mdio_int_freq);

    }

    mdio_delay = SOC_DNX_IMP_DEFS_GET(unit, mdio_int_out_delay_default);

    DNXC_IF_ERR_EXIT(soc_dnxc_cmic_mdio_config(unit,dividend,divisor,mdio_delay));

    /* Clear SCHAN_ERR */
    DNXC_IF_ERR_EXIT(WRITE_CMIC_CMC0_SCHAN_ERRr(unit, 0));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_CMC1_SCHAN_ERRr(unit, 0));
    DNXC_IF_ERR_EXIT(WRITE_CMIC_CMC2_SCHAN_ERRr(unit, 0));

    /* MDIO configuration */
    DNXC_IF_ERR_EXIT(soc_dnxc_cmic_mdio_set(unit));

exit:
    DNXC_FUNC_RETURN;
}

int soc_jer2_jer_init_reset(
    int unit,
    int reset_action)
{
    int disable_hard_reset = 0x0;

    DNXC_INIT_FUNC_DEFS;

    /* Configure PAXB, enabling the access of CMIC */
    DNXC_IF_ERR_EXIT(soc_dnxc_iproc_config_paxb(unit));

    /* Arad CPS Reset */
    disable_hard_reset = soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "init_without_device_hard_reset", 0);
    if (disable_hard_reset == 0) {
        DNXC_IF_ERR_EXIT(soc_dnxc_cmic_device_hard_reset(unit, reset_action));
    }

    DNXC_IF_ERR_EXIT(soc_dnxc_iproc_config_paxb(unit));

    /* Config Endianess */
    soc_endian_config(unit);
    soc_pci_ep_config(unit, 0);

    /* Config Default/Basic cmic registers */
    if (soc_feature(unit, soc_feature_cmicm)) {
        DNXC_IF_ERR_EXIT(soc_jer2_jer_init_reset_cmic_regs(unit));
    }

exit:
    DNXC_FUNC_RETURN;
}

int soc_jer2_jer_rcpu_base_q_pair_init(int unit, int port_i)
{
    uint32 base_q_pair = 0, rval = 0;
    soc_error_t rv;
    
    DNXC_INIT_FUNC_DEFS;
    
    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.base_q_pair.get(unit, port_i, &base_q_pair);
    DNXC_IF_ERR_EXIT(rv);

    if (base_q_pair < 32) 
    {
        DNXC_IF_ERR_EXIT(READ_CMIC_PKT_PORTS_0r(unit, &rval));
        rval |= 0x1 << base_q_pair;
        DNXC_IF_ERR_EXIT(WRITE_CMIC_PKT_PORTS_0r(unit, rval));
    } else if (base_q_pair < 64) 
    {
        DNXC_IF_ERR_EXIT(READ_CMIC_PKT_PORTS_1r(unit, &rval));
        rval |= 0x1 << (base_q_pair - 32);
        DNXC_IF_ERR_EXIT(WRITE_CMIC_PKT_PORTS_1r(unit, rval));
    } else if (base_q_pair < 96) 
    {
        DNXC_IF_ERR_EXIT(READ_CMIC_PKT_PORTS_2r(unit, &rval));
        rval |= 0x1 << (base_q_pair - 64);
        DNXC_IF_ERR_EXIT(WRITE_CMIC_PKT_PORTS_2r(unit, rval));
    } else if (base_q_pair == 96) 
    {
        rval = 0x1;
        DNXC_IF_ERR_EXIT(WRITE_CMIC_PKT_PORTS_3r(unit, rval));
    } else 
    {
        LOG_ERROR(BSL_LS_SOC_INIT, (BSL_META_U(unit, "Error: RCPU base_q_pair range is 0 - 96\n")) );
        DNXC_IF_ERR_EXIT(SOC_E_INTERNAL);
    }              
    exit:
         DNXC_FUNC_RETURN;
}

int soc_jer2_jer_rcpu_init(int unit, soc_dnx_config_t *dnx)
{
    int port_i = 0;
    DNXC_INIT_FUNC_DEFS;

    SOC_PBMP_ITER(dnx->jer2_arad->init.rcpu.slave_port_pbmp, port_i) 
    {
        DNXC_IF_ERR_EXIT(soc_jer2_jer_rcpu_base_q_pair_init(unit, port_i));
    }

    exit:
         DNXC_FUNC_RETURN;
}
