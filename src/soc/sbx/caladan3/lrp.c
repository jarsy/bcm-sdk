/*
 * $Id: lrp.c,v 1.42 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    lrp.c
 * Purpose: Caladan3 on LRP drivers
 * Requires:
 */

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3.h>
#include <soc/sbx/caladan3/lrp.h>
#include <soc/sbx/caladan3/list.h>
#include <soc/sbx/caladan3/util.h>
#include <soc/sbx/caladan3/ucodemgr.h>
#include <soc/sbx/caladan3/asm3/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/cm.h>
#include <soc/mem.h>

#define SOC_SBX_CALADAN3_LR_OTHER_BANK(b) \
          (((b) + 1) % SOC_SBX_CALADAN3_LR_INST_NUM_BANKS)

#define PACK_INST(a, b, c, d) ((a) << 24 | (b) << 16 | (c) << 8 | (d))

#define UNPACK_INST(p, i) \
           do { \
               (p)[0] = ((i) >> 24) & 0xFF;  \
               (p)[1] = ((i) >> 16) & 0xFF;  \
               (p)[2] = ((i) >>  8) & 0xFF;  \
               (p)[3] = ((i) & 0xff);        \
           } while(0);

#define PACK_TASK_MAP(a, b, c, d) ((d) << 24 | (c) << 16 | (b) << 8 | (a))

lra_inst_b0_mem0_entry_t readback[SOC_SBX_CALADAN3_LR_NUM_OF_INSTRS];

/* Stream inst memory on banks */
soc_mem_t lrmem[][SOC_SBX_CALADAN3_LR_INST_NUM_BANKS] = {
    { LRA_INST_B0_MEM0m, LRA_INST_B1_MEM0m },  /* Stream 0 */
    { LRA_INST_B0_MEM0m, LRA_INST_B1_MEM0m },  /* Stream 1 */
    { LRA_INST_B0_MEM1m, LRA_INST_B1_MEM1m },  /* Stream 2 */
    { LRA_INST_B0_MEM1m, LRA_INST_B1_MEM1m },  /* Stream 3 */
    { LRA_INST_B0_MEM2m, LRA_INST_B1_MEM2m },  /* Stream 4 */
    { LRA_INST_B0_MEM2m, LRA_INST_B1_MEM2m },  /* Stream 5 */
    { LRA_INST_B0_MEM3m, LRA_INST_B1_MEM3m },  /* Stream 6 */
    { LRA_INST_B0_MEM3m, LRA_INST_B1_MEM3m },  /* Stream 7 */
    { LRA_INST_B0_MEM4m, LRA_INST_B1_MEM4m },  /* Stream 8 */
    { LRA_INST_B0_MEM4m, LRA_INST_B1_MEM4m },  /* Stream 9 */
    { LRA_INST_B0_MEM5m, LRA_INST_B1_MEM5m },  /* Stream 10 */
    { LRA_INST_B0_MEM5m, LRA_INST_B1_MEM5m },  /* Stream 11 */
    { LRA_INST_DEBUGm,   LRA_INST_DEBUGm   }   /* Debug, only 1 */
};

#define SOC_SBX_CALADAN3_LR_INST_MEM(b, s) lrmem[(s)][(b)];


STATIC int
_soc_sbx_caladan3_lr_init_bubble(int unit, soc_sbx_caladan3_lrp_t *lrp);

/*
 *
 * Function:
 *     soc_sbx_caladan3_lr_bypass_init
 * Purpose:
 *     Bring up LRP on Bypass mode
 */
int soc_sbx_caladan3_lrp_bypass_init(int unit) 
{
    uint32 regval; 

    regval = 0; 
    soc_reg_field_set(unit, LRA_CONFIG0r, &regval, SOFT_RESET_Nf, 1); 
    WRITE_LRA_CONFIG0r(unit, regval);
    soc_reg_field_set(unit, LRA_CONFIG0r, &regval, BYPASSf, 1); 
    soc_reg_field_set(unit, LRA_CONFIG0r, &regval, ENABLEf, 1); 
    soc_reg_field_set(unit, LRA_CONFIG0r, &regval, LOAD_ENABLEf, 1); 
    WRITE_LRA_CONFIG0r(unit, regval);
    
    READ_LRB_CONFIG0r(unit, &regval); 
    soc_reg_field_set(unit, LRB_CONFIG0r, &regval, SOFT_RESET_Nf, 1); 
    WRITE_LRB_CONFIG0r(unit, regval);

    READ_LRA_CONFIG1r(unit, &regval); 
    soc_reg_field_set(unit, LRA_CONFIG1r, &regval, UPDATEf, 1); 
    soc_reg_field_set(unit, LRA_CONFIG1r, &regval, FRAMES_PER_CONTEXTf, 128); 
    soc_reg_field_set(unit, LRA_CONFIG1r, &regval, CONTEXTSf, 3); 
    soc_reg_field_set(unit, LRA_CONFIG1r, &regval, DUPLEXf, 1); 
    WRITE_LRA_CONFIG1r(unit, regval);
    
    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_lr_shutdown
 * Purpose:
 *     shutdown the LRP
 */
int
soc_sbx_caladan3_lr_shutdown(int unit)
{
    uint32 regval = 0;
    int rv;
    soc_sbx_caladan3_lrp_t *lrp;

    lrp = &(SOC_SBX_CFG_CALADAN3(unit)->lrp_cfg);

    if (!lrp->init_done) {
        return SOC_E_INIT;
    }
    
    READ_LRA_CONFIG0r(unit, &regval);
    soc_reg_field_set(unit, LRA_CONFIG0r, &regval, SHUTDOWNf, 1); 
    rv = WRITE_LRA_CONFIG0r(unit, regval);
    if (SOC_SUCCESS(rv)) {
        rv = soc_sbx_caladan3_reg32_expect_field_timeout(unit, LRA_EVENTr, 
                                                         -1, 0, -1, 
                                                         OFFLINEf, 
                                                         1, -1/*default*/);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_lr_disable: unit %d LR Shutdown failed (%d) \n"), unit, rv));
        }
    }

    return rv;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_lr_enable
 * Purpose:
 *     Enable the LRP
 */
int
soc_sbx_caladan3_lr_enable(int unit, soc_sbx_caladan3_lrp_t *lrp)
{

    uint32 regval = 0;
    uint32 event_data;
    int rv;

    if (!lrp->init_done) {
        /* Reset LRA */
        regval = 0;
        soc_reg_field_set(unit, LRA_CONFIG0r, &regval, SOFT_RESET_Nf, 1); 
        WRITE_LRA_CONFIG0r(unit, regval);
        /* Reset LRB */
        regval = 0;
        soc_reg_field_set(unit, LRB_CONFIG0r, &regval, SOFT_RESET_Nf, 1); 
        WRITE_LRB_CONFIG0r(unit, regval);
     
        if (soc_property_get(unit, spn_LRP_BYPASS, 0)) {
            soc_sbx_caladan3_lrp_bypass_init(unit);
        }

        /* reset result ram */
        regval = 0;
        soc_reg_field_set(unit, LRB_RESULT_RAM_INITr, &regval, INIT_RCEf, 1); 
        soc_reg_field_set(unit, LRB_RESULT_RAM_INITr, &regval, INIT_ETUf, 1); 
        soc_reg_field_set(unit, LRB_RESULT_RAM_INITr, &regval, INIT_SK0f, 1); 
        soc_reg_field_set(unit, LRB_RESULT_RAM_INITr, &regval, INIT_SK1f, 1);       
        soc_reg_field_set(unit, LRB_RESULT_RAM_INITr, &regval, INIT_DM0f, 1); 
        soc_reg_field_set(unit, LRB_RESULT_RAM_INITr, &regval, INIT_DM1f, 1);      
        soc_reg_field_set(unit, LRB_RESULT_RAM_INITr, &regval, INIT_DM2f, 1); 
        soc_reg_field_set(unit, LRB_RESULT_RAM_INITr, &regval, INIT_DM3f, 1); 
        SOC_IF_ERROR_RETURN(WRITE_LRB_RESULT_RAM_INITr(unit, regval));

        sal_udelay(1);
     
        if (soc_property_get(unit, spn_LRP_BYPASS, 0)) {
            soc_sbx_caladan3_lrp_bypass_init(unit);
        }

        if (!lrp->ucode_done) {
            return SOC_E_NONE;
        }
    }
    /* Clear left over events */
    event_data = 0xFFFFFFFF;
    rv = WRITE_LRA_EVENTr( unit, event_data);


    /* Update LRA CONFIG1 */
    if (SOC_SUCCESS(rv)) {

        READ_LRA_CONFIG1r(unit, &regval);
        soc_reg_field_set(unit, LRA_CONFIG1r, &regval, FRAMES_PER_CONTEXTf, 
                          lrp->frames_per_context);


        /* Check detected contexts are same */
        if (lrp->detected_context[1] != lrp->detected_context[0]) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_lr_enable: unit %d Detected contexts Bank0(%d) is not same as Bank1(%d)\n"),
                       unit, lrp->detected_context[0], lrp->detected_context[1]));
            return SOC_E_PARAM;
        }
        if (lrp->num_context != lrp->detected_context[0]) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "soc_sbx_caladan3_lr_enable: unit %d Total num of contexts(%d) is not same as detected(%d)\n"),
                      unit, lrp->num_context, lrp->detected_context[0]));
            lrp->num_context = lrp->detected_context[0];
        }

        /* Final check: Account for reserved contexts */
        if (lrp->num_context > SOC_SBX_CALADAN3_LR_CONTEXTS_USER_MAX) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_lr_enable: unit %d Total num of contexts(%d) is > max (%d)\n"),
                       unit, lrp->num_context, SOC_SBX_CALADAN3_LR_CONTEXTS_MAX));
            return SOC_E_PARAM;
        }
        
        soc_reg_field_set(unit, LRA_CONFIG1r, &regval, CONTEXTSf, 
                          lrp->num_context + SOC_SBX_CALADAN3_LR_CONTEXTS_RSVD);
        soc_reg_field_set(unit, LRA_CONFIG1r, &regval, DUPLEXf,
                          lrp->duplex);
        soc_reg_field_set(unit, LRA_CONFIG1r, &regval, EPOCH_LENGTHf,
                          lrp->epoch_length - 17);
        soc_reg_field_set(unit, LRA_CONFIG1r, &regval, UPDATEf, 1);

        rv = WRITE_LRA_CONFIG1r(unit, regval);
        if (SOC_SUCCESS(rv) && !(lrp->bypass)) {
            rv = soc_sbx_caladan3_reg32_expect_field_timeout(unit, LRA_EVENTr, 
                                                             -1, 0, -1, 
                                                             UPDATEf, 
                                                             1, -1/*default*/);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_sbx_caladan3_lr_enable: unit %d LRA.CONFIG1 update failed (%d) \n"), unit, rv));
                return rv;
            } else {
                /* Clear */
                event_data = 0;
                soc_reg_field_set(unit, LRA_EVENTr, &event_data, UPDATEf, 1);
                WRITE_LRA_EVENTr( unit, event_data);
            }
        }
    }

    if (SOC_SUCCESS(rv)) {

        /* Update LRA CONFIG0 */
        READ_LRA_CONFIG0r(unit, &regval);
        soc_reg_field_set(unit, LRA_CONFIG0r, &regval, SOFT_RESET_Nf, 1); 

        /* Start the loader and the enable lrp only if there 
         * exists a valid ucode or we are in bypass 
         */
        if (lrp->ucode_done || lrp->bypass) {
            soc_reg_field_set(unit, LRA_CONFIG0r, &regval, ENABLEf, 1);
            soc_reg_field_set(unit, LRA_CONFIG0r, &regval, LOAD_ENABLEf, 
                              lrp->loader_enable);
        }
        soc_reg_field_set(unit, LRA_CONFIG0r, &regval, BYPASSf, lrp->bypass);
        if (lrp->bypass) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "soc_sbx_caladan3_lr_enable: unit %d LRP in Bypass mode\n"), unit));
            soc_reg_field_set(unit, LRA_CONFIG0r, &regval, ONLINEf, 0);
        } else {
            soc_reg_field_set(unit, LRA_CONFIG0r, &regval, ONLINEf, 
                              lrp->streams_online);
        }
        rv = WRITE_LRA_CONFIG0r(unit, regval);
        if (SOC_SUCCESS(rv) && !(lrp->bypass) && lrp->streams_online) {
            rv = soc_sbx_caladan3_reg32_expect_field_timeout(unit, LRA_EVENTr, 
                                                           -1, 0, -1, 
                                                           ONLINEf, 
                                                           lrp->streams_online,
                                                           -1/*default*/);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_sbx_caladan3_lr_enable: unit %d LRA.CONFIG0 timeout waiting for stream online (%d) \n"), unit, rv));
                return rv;
            } else {
                /* Clear */
                event_data = 0;
                soc_reg_field_set(unit, LRA_EVENTr, &event_data, UPDATEf, 1);
                WRITE_LRA_EVENTr( unit, event_data);
            }
        }
 
    }

    if (SOC_SUCCESS(rv)) {
        READ_LRB_DEBUGr(unit, &regval);
        /* OCM Latency disabled for in-circuit emulation work-around */
        soc_reg_field_set(unit, LRB_DEBUGr, &regval, OCM_LATENCYf, 1);
        /* SVP Latency set to adapt to the latency of on-chip memory */
        soc_reg_field_set(unit, LRB_DEBUGr, &regval, SVP_LATENCYf, 15);
        WRITE_LRB_DEBUGr(unit, regval);
    }

    if (SOC_SUCCESS(rv)) {
	/* config LRP to stall when TMU (or other lookup engine) bandwidth exceeded 
	 * and can not return lookup result within 1 epoch
	 */
	READ_LRA_RESULT_TIMERr(unit, &regval);
	soc_reg_field_set(unit, LRA_RESULT_TIMERr, &regval, TIMERf, 0x7fffFFFF);
	WRITE_LRA_RESULT_TIMERr(unit, regval);
    }

    if (SOC_SUCCESS(rv)) {
        rv = _soc_sbx_caladan3_lr_init_bubble(unit, lrp);
    }

#if 0
    if (SOC_SUCCESS(rv)) {
        /* create wheel list on svp=2.0 with 1024 entries, increment 2^0 */
        rv = soc_sbx_caladan3_lr_list_manager_init(unit, 2, 0, SOC_SBX_CALADAN3_LRP_LIST_MODE_WHEEL, 1024, 0);
    }
#endif

    if (SOC_SUCCESS(rv)) {
        /* LRP enabled */
        lrp->init_done = 1;
    }
    return rv;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_lr_driver_initdefaults
 * Purpose:
 *     Setup default parameters
 */
int soc_sbx_caladan3_lr_driver_initdefaults(int unit) 
{
    soc_sbx_caladan3_lrp_t *lrp;

    lrp = &(SOC_SBX_CFG_CALADAN3(unit)->lrp_cfg);
    if (!lrp) {
        return SOC_E_INIT;
    }

    /* Default parameters */

    lrp->bypass = soc_property_get(unit, spn_LRP_BYPASS, 0);
    lrp->num_context = SOC_SBX_CALADAN3_LR_CONTEXTS_RSVD;
    lrp->epoch_length = SOC_SBX_CALADAN3_LR_EPOCH_LENGTH_MIN;
    lrp->frames_per_context = SOC_SBX_CALADAN3_LR_FRAMES_PER_CONTEXT_MAX;
    lrp->duplex = SOC_SBX_CALADAN3_DUPLEX_MODE;
    lrp->loader_enable = 1;

    /* lrp->oam_num_endpoints initialized in sbx_drv.c dependent upon ucode */

    /* Allocted DMA-able mem */
    if (lrp->s01 == NULL) {
        lrp->s01 = soc_cm_salloc(unit, 
                        SOC_SBX_CALADAN3_LR_NUM_OF_INSTRS * sizeof(lra_inst_b0_mem0_entry_t),
                        "LRP inst mem");
    }
    if (!lrp->s01) {
        return SOC_E_INIT;
    }

    return SOC_E_NONE;      
}

/***
 ***  LRP Bubble
 ***/

/*
 *
 * Function:
 *     _soc_sbx_caladan3_lr_init_bubble
 * Purpose:
 *     Initialize the bubble generator
 * Notes: 
 * 1.  Channel 3 of the ts clock goes to the IEEE1588 block and it is set to 125MHz or 8ns period
 *     based upon the ts_ppl_channel_3 mdiv setting of 24.
 * 
 * 2.  IEEE1588 clock matches because of the time_freq_control frequency setting.  Upper 6 bits
 *     define the time in ns to increment each clock cycle, lower 26 bits are fractional.  We are
 *     setting this to 8ns meaning every clock cycle is equal to 8ns.
 *
 * 3.  The LRP bubble timer timeout field is initialized with the current_time + interval during 
 *     initialization of the timer.
 *  
 * 4.  LRP gets current time from IEEE1588 time of day upper 32 seconds, lower 32 nseconds.  Note 
 *     that lower 32 bits are in nanoseconds.
 * 
 * 5.  Every time bit 9 of the lower time of day toggles, then the lrp current time is incremented.  
 *      This should give a 512ns clock.
 */

#define SOC_SBX_CALADAN3_BUBBLE_INTERVAL_TABLE_SIZE 128 
#define SOC_SBX_CALADAN3_BUBBLE_INTERVAL_TABLE_SIZE_USED 8
static int _soc_sbx_caladan3_bubble_interval[SOC_SBX_CALADAN3_BUBBLE_INTERVAL_TABLE_SIZE_USED] = {0,            /* invalid     */
                                                                                                  (6503),       /* 3.329536 ms */
                                                                                                  (19531),      /* 9.999872 ms */
                                                                                                  (195312),     /* 99.999744 ms */
                                                                                                  (1953125),    /* 1s           */
                                                                                                  (19531250),   /* 10s          */
                                                                                                  (117187500),  /* 1 min        */
                                                                                                  (1171875000)};/* 10 min       */

STATIC int
_soc_sbx_caladan3_lr_init_bubble(int unit, soc_sbx_caladan3_lrp_t *lrp)
{
    uint32 index = 0;
    uint32 regval = 0;
    int rc = SOC_E_NONE;
    char *s;

    s = soc_property_get_str(unit, spn_BCM88030_UCODE);

    
    /*  bubble descriptor host specific data bytes 0-29        */
    /* p 480 of spec.                                          */
    /* The first 8 words of data are the first 8 words of the  */
    /* packet header descriptor. p417 of spec                  */
    /* see definition caladan_ucode/g3p1/doc/g3p1_oam.txt      */
    if (s != NULL && sal_strcmp("g3p1a", s) == 0) {
      /* Arad mode */
      SOC_IF_ERROR_RETURN(WRITE_LRA_BUBBLE_DATAr(unit, 0, 0x6d310102));
      SOC_IF_ERROR_RETURN(WRITE_LRA_BUBBLE_DATAr(unit, 1, 0x033f0000));
      SOC_IF_ERROR_RETURN(WRITE_LRA_BUBBLE_DATAr(unit, 2, 0x00000000));
      SOC_IF_ERROR_RETURN(WRITE_LRA_BUBBLE_DATAr(unit, 3, 0x00000000));
      SOC_IF_ERROR_RETURN(WRITE_LRA_BUBBLE_DATAr(unit, 4, 0x40505c60));
      SOC_IF_ERROR_RETURN(WRITE_LRA_BUBBLE_DATAr(unit, 5, 0x62000000));
      SOC_IF_ERROR_RETURN(WRITE_LRA_BUBBLE_DATAr(unit, 6, 0x00000000));
      SOC_IF_ERROR_RETURN(WRITE_LRA_BUBBLE_DATAr(unit, 7, 0x00000000));
    }
    else {
      /* Sirius mode */
      SOC_IF_ERROR_RETURN(WRITE_LRA_BUBBLE_DATAr(unit, 0, 0x69300102));
      SOC_IF_ERROR_RETURN(WRITE_LRA_BUBBLE_DATAr(unit, 1, 0x033f0000));
      SOC_IF_ERROR_RETURN(WRITE_LRA_BUBBLE_DATAr(unit, 2, 0x00000000));
      SOC_IF_ERROR_RETURN(WRITE_LRA_BUBBLE_DATAr(unit, 3, 0x00000000));
      SOC_IF_ERROR_RETURN(WRITE_LRA_BUBBLE_DATAr(unit, 4, 0x404c585c));
      SOC_IF_ERROR_RETURN(WRITE_LRA_BUBBLE_DATAr(unit, 5, 0x5e000000));
      SOC_IF_ERROR_RETURN(WRITE_LRA_BUBBLE_DATAr(unit, 6, 0x00000000));
      SOC_IF_ERROR_RETURN(WRITE_LRA_BUBBLE_DATAr(unit, 7, 0x00000000));
    }

    /* clear table */
    regval = 0;
    for (index = 0; index<SOC_SBX_CALADAN3_BUBBLE_INTERVAL_TABLE_SIZE; index++) {
        SOC_IF_ERROR_RETURN(soc_mem_write(unit, LRB_BUBBLE_INTERVAL_TABLEm, MEM_BLOCK_ALL,
                                          index, &regval));
    }

    /* Interval is in 512ns ticks  */
    for (index = 0; index<SOC_SBX_CALADAN3_BUBBLE_INTERVAL_TABLE_SIZE_USED; index++) {
        SOC_IF_ERROR_RETURN(soc_mem_write(unit, LRB_BUBBLE_INTERVAL_TABLEm, MEM_BLOCK_ALL,
                                          index, &_soc_sbx_caladan3_bubble_interval[index]));
    }

    SOC_IF_ERROR_RETURN(READ_CX_TS_PLL_CHANNEL_3r(unit, &regval));
    soc_reg_field_set(unit, CX_TS_PLL_CHANNEL_3r, &regval, HOLDf, 1);
    SOC_IF_ERROR_RETURN(WRITE_CX_TS_PLL_CHANNEL_3r(unit, regval));

    SOC_IF_ERROR_RETURN(READ_CX_TS_PLL_CHANNEL_3r(unit, &regval));
    soc_reg_field_set(unit, CX_TS_PLL_CHANNEL_3r, &regval, MDIVf, 12); /* 125MHz if 24, 250MHz if 12 */
    SOC_IF_ERROR_RETURN(WRITE_CX_TS_PLL_CHANNEL_3r(unit, regval));

    SOC_IF_ERROR_RETURN(READ_CX_PLL_CTRLr(unit, &regval));
    soc_reg_field_set(unit, CX_PLL_CTRLr, &regval, TS_BOND_OVERRIDEf, 1);
    SOC_IF_ERROR_RETURN(WRITE_CX_PLL_CTRLr(unit, regval));

    SOC_IF_ERROR_RETURN(READ_CX_TS_PLL_CHANNEL_3r(unit, &regval));
    soc_reg_field_set(unit, CX_TS_PLL_CHANNEL_3r, &regval, HOLDf, 0);
    SOC_IF_ERROR_RETURN(WRITE_CX_TS_PLL_CHANNEL_3r(unit, regval));

    SOC_IF_ERROR_RETURN(READ_CX_TS_PLL_CHANNEL_3r(unit, &regval));
    soc_reg_field_set(unit, CX_TS_PLL_CHANNEL_3r, &regval, LOAD_ENf, 1);
    SOC_IF_ERROR_RETURN(WRITE_CX_TS_PLL_CHANNEL_3r(unit, regval));

    SOC_IF_ERROR_RETURN(READ_CX_TS_PLL_CHANNEL_3r(unit, &regval));
    soc_reg_field_set(unit, CX_TS_PLL_CHANNEL_3r, &regval, LOAD_ENf, 0);
    SOC_IF_ERROR_RETURN(WRITE_CX_TS_PLL_CHANNEL_3r(unit, regval));

    SOC_IF_ERROR_RETURN(READ_IEEE1588_TIME_FREQ_CONTROLr(unit, &regval));
    /* 4ns tick with 4ns clock (250MHz) - lower 26 bits are fraction of ns upper 6 bits are in ns */
    /* 0001-00 ns upper 00-0000-0000-0000-0000-0000-0000 fraction lower */                          
    soc_reg_field_set(unit, IEEE1588_TIME_FREQ_CONTROLr, &regval, FREQUENCYf, 0x10000000);
    SOC_IF_ERROR_RETURN(WRITE_IEEE1588_TIME_FREQ_CONTROLr(unit, regval));
    
    SOC_IF_ERROR_RETURN(READ_IEEE1588_TIME_CONTROLr(unit, &regval));
    soc_reg_field_set(unit, IEEE1588_TIME_CONTROLr, &regval, LOAD_ENABLEf, 0x1f);
    SOC_IF_ERROR_RETURN(WRITE_IEEE1588_TIME_CONTROLr(unit, regval));

    SOC_IF_ERROR_RETURN(READ_IEEE1588_TIME_CONTROLr(unit, &regval));
    soc_reg_field_set(unit, IEEE1588_TIME_CONTROLr, &regval, LOAD_ENABLEf, 0x0);
    SOC_IF_ERROR_RETURN(WRITE_IEEE1588_TIME_CONTROLr(unit, regval));

    SOC_IF_ERROR_RETURN(READ_IEEE1588_TIME_CONTROLr(unit, &regval));
    soc_reg_field_set(unit, IEEE1588_TIME_CONTROLr, &regval, COUNT_ENABLEf, 0x1);
    SOC_IF_ERROR_RETURN(WRITE_IEEE1588_TIME_CONTROLr(unit, regval));

    rc = soc_sbx_caladan3_lr_bubble_enable(unit, FALSE, 0);
    return rc;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_lr_bubble_enable
 * Purpose:
 *     Enable bubble table generation
 */
int
soc_sbx_caladan3_lr_bubble_enable(int unit, uint32 enable, uint32 size_in_bytes)
{
    uint32 regval = 0;

    /* Set size to be 0 for now and leave disabled until bcm_oam_init() */
    SOC_IF_ERROR_RETURN(READ_LRB_BUBBLE_TABLE_CONFIGr(unit, &regval));
    soc_reg_field_set(unit, LRB_BUBBLE_TABLE_CONFIGr, &regval, SIZEf, size_in_bytes);
    SOC_IF_ERROR_RETURN(WRITE_LRB_BUBBLE_TABLE_CONFIGr(unit, regval));

    SOC_IF_ERROR_RETURN(READ_LRB_BUBBLE_TABLE_CONFIGr(unit, &regval));
    if (enable) {
        soc_reg_field_set(unit, LRB_BUBBLE_TABLE_CONFIGr, &regval, ENABLEf, 1);
    } else {
        soc_reg_field_set(unit, LRB_BUBBLE_TABLE_CONFIGr, &regval, ENABLEf, 0);
    }
    SOC_IF_ERROR_RETURN(WRITE_LRB_BUBBLE_TABLE_CONFIGr(unit, regval));

    return SOC_E_NONE;
}

/*
 *   Function
 *     sbx_caladan3_lr_host_bubble
 *   Purpose
 *      generate Host Bubble
 *   Parameters
 *      (IN) unit      : unit number of the device
 *      (IN) stream    : stream to send bubble
 *      (IN) task      : ingress=0, egress=1
 *      (IN) id        : id of generated bubble
 *   Returns
 *       SOC_E_NONE    - success
 *       SOC_E_TIMEOUT - command timed out
 */
int soc_sbx_caladan3_lr_host_bubble(int unit,
                                    int stream,
                                    int task,
                                    uint32 id)
{
    int    result = SOC_E_NONE;
    uint32 uRegValue = 0;

    soc_reg_field_set(unit, LRA_BUBBLEr, &uRegValue, INGRESSf, ((task == 0) ? 1 : 0));
    soc_reg_field_set(unit, LRA_BUBBLEr, &uRegValue, EGRESSf, ((task == 1) ? 1 : 0));
    soc_reg_field_set(unit, LRA_BUBBLEr, &uRegValue, STRf, (uint32) stream);
    soc_reg_field_set(unit, LRA_BUBBLEr, &uRegValue, IDf, id);

    result = WRITE_LRA_BUBBLEr(unit, uRegValue);
    if (SOC_FAILURE(result)) {
        return result; 
    }
 
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s lr host bubble generation stream 0x%x, task 0x%x, id 0x%8x unit %d.\n"),
                 FUNCTION_NAME(), uRegValue, task, id, unit));

    return result;
}

/*
 * Function:
 *     soc_sbx_caladan3_lr_driver_init
 * Purpose:
 *     Bring up LRP drivers
 */
int soc_sbx_caladan3_lr_driver_init(int unit) 
{
    soc_sbx_caladan3_lrp_t *lrp;
    int rv = SOC_E_NONE;
   
    lrp = &(SOC_SBX_CFG_CALADAN3(unit)->lrp_cfg);
    if (!lrp) {
        return SOC_E_INIT;
    }
    if (!lrp->init_done) {
        
        /* Initialize with defaults */
        soc_sbx_caladan3_lr_driver_initdefaults(unit);

        /* Initialize List in LRB */
        soc_sbx_caladan3_lr_list_init(unit);

        /* Enable LRA/LRB block */
        rv = soc_sbx_caladan3_lr_enable(unit, lrp);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_lr_driver_init: unit %d Init failed (%d) \n"), unit, rv));
        }
    }
    return rv;
}

/*
 * Function:
 *    soc_sbx_caladan3_lr_bank_swap
 * Purpose
 *    Swap the instruction memory banks
 */
int
soc_sbx_caladan3_lr_bank_swap(int unit) 
{
    uint32 config_data; 
    uint32 event_data;
    int rv;
    int active, bank;

    READ_LRA_CONFIG0r( unit, &config_data );
    active = soc_reg_field_get( unit, LRA_CONFIG0r, config_data, ENABLEf);

    /* Fake a bank swap if LRP is not active */
    if (!active) {

        /* Update the ucodemgr about bank swap */
        soc_sbx_caladan3_ucodemgr_bank_swapped(unit);

        return SOC_E_NONE;
    }

    /* Prevent a bank swap if bank is yet loaded */
    bank = soc_sbx_caladan3_ucodemgr_standby_bank_get(unit);
    if (soc_sbx_caladan3_ucodemgr_bank_dirty_get(unit, bank) == BANK_INVALID) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "\nCannot swap LRP to uninitialized bank %d"), bank));
        return SOC_E_INIT;
    }
    
    /* Clear any left over event */
    event_data = 0;
    soc_reg_field_set(unit, LRA_EVENTr, &event_data, BANK_SWAP_DONEf, 1);
    WRITE_LRA_EVENTr( unit, event_data); 

 
    READ_LRA_CONFIG0r( unit, &config_data );
    soc_reg_field_set( unit, LRA_CONFIG0r, &config_data, BANK_SWAPf, 1 );
    rv = WRITE_LRA_CONFIG0r( unit, config_data );

    if (SOC_SUCCESS(rv)) {
        rv = soc_sbx_caladan3_reg32_expect_field_timeout(unit, LRA_EVENTr, 
                                                         -1, 0, -1, 
                                                         BANK_SWAP_DONEf, 
                                                         1, -1);
    }
    if (SOC_SUCCESS(rv)) {
        /* Clear the event */
        event_data = 0;
        soc_reg_field_set(unit, LRA_EVENTr, &event_data, BANK_SWAP_DONEf, 1);
        WRITE_LRA_EVENTr( unit, event_data);

        /* Update the ucodemgr about bank swap */
        soc_sbx_caladan3_ucodemgr_bank_swapped(unit);
    }
    return rv;
}

/*
 * Function: 
 *    soc_sbx_caladan3_lr_download_all
 * Description:
 *    Routine to download image to LRP memory
 */

int
soc_sbx_caladan3_lr_download_all(int unit, soc_sbx_caladan3_ucode_pkg_t *ucode)
{
    int inum, snum, bp;
    uint32 so, se, odd, even; 
    soc_mem_t memA, memB;
    uint32 *ip1, *ip0, w;
    uint8 *p1, *p0; 
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_lrp_t *lrp;
    int gcnt=0, cnt = 0;
    int i,j;
    uint8 *p;
    uint32 taskmap, maxi;
    
    lrp = &(SOC_SBX_CFG_CALADAN3(unit)->lrp_cfg);
    if (!lrp) {
        return SOC_E_INIT;
    }
    
    for (snum=0; ((snum < SOC_SBX_CALADAN3_LR_NUM_OF_STREAMS) &&
                  (snum < ucode->m_snum)); snum+=2)
    {
        cnt = 0; 
        so = ucode->m_inum * (snum + 1) * ucode->m_onum;
        se = ucode->m_inum * snum * ucode->m_onum;
        memA = SOC_SBX_CALADAN3_LR_INST_MEM(0, snum);
        memB = SOC_SBX_CALADAN3_LR_INST_MEM(1, snum);
        sal_memset(&lrp->s01[0], 0, 
               sizeof(lrp->s01[0])* SOC_SBX_CALADAN3_LR_NUM_OF_INSTRS);
        for (inum=0; ((inum < SOC_SBX_CALADAN3_LR_NUM_OF_INSTRS) && 
                  (inum < ucode->m_inum)); inum++) 
        {
            odd = inum * ucode->m_onum + so;
            even = inum * ucode->m_onum + se;
            ip0 = &lrp->s01[inum].entry_data[0];
            ip1 = ip0 + (ucode->m_onum / 4);
            p0 = &ucode->m_code[even];
            p1 = &ucode->m_code[odd]; 
            for (w=ucode->m_onum/4 - 1, bp = 0; bp < ucode->m_onum; w--, bp += 4) {
                ip1[w] = 
                    PACK_INST(p1[bp], p1[bp+1], p1[bp+2], p1[bp+3]);
                ip0[w] =
                    PACK_INST(p0[bp] ,p0[bp+1], p0[bp+2], p0[bp+3]);
                if ((w == ucode->m_onum/4) && (ip1[w] == 0xffc00000)) cnt++; 
            }
        }
        if (gcnt) {
           if (gcnt != cnt) {
               LOG_CLI((BSL_META_U(unit,
                                   "\nContext Compare failed when downloading stream %d: this bank(%d) this stream(%d)"), snum, gcnt, cnt));
               return SOC_E_PARAM;
           }
        } else {
            gcnt = cnt;
        }
        /* Push streams 
         *   We write all the inst even though lesser may be valid
         *   The unused ones are zeroed. Zero encodes to nop.
         */
        /*    coverity[negative_returns : FALSE]    */
        rv = soc_mem_write_range(unit, memA, MEM_BLOCK_ALL, 0, 
                                 (SOC_SBX_CALADAN3_LR_NUM_OF_INSTRS - 1), 
                                 (void*)&lrp->s01[0]);
        if (SOC_SUCCESS(rv)) {
            /*    coverity[negative_returns : FALSE]    */
            rv = soc_mem_write_range(unit, memB, MEM_BLOCK_ALL, 0, 
                                     (SOC_SBX_CALADAN3_LR_NUM_OF_INSTRS - 1), 
                                     (void*)&lrp->s01[0]);
        }
        if (SOC_FAILURE(rv)) {
            LOG_CLI((BSL_META_U(unit,
                                "\nFailed while downloading ucode image for stream(%d) rv(%d)"), snum, rv));
            return rv;
        } else {
            lrp->streams_online |= 0x3 << snum;
        }
        

    }
    soc_sbx_caladan3_ucodemgr_bank_dirty_set(unit, 0, BANK_DIRTY);
    soc_sbx_caladan3_ucodemgr_bank_dirty_set(unit, 1, BANK_DIRTY);
    lrp->detected_context[1] = gcnt;
    lrp->detected_context[0] = gcnt;

    /* Update the taskmap */
    p = ucode->m_tmap;
    maxi = (ucode->m_inum + 31)/32;
    for (j=0,i=0; j < soc_mem_index_max(unit, LRA_INST_TASK_MAPm); j++,i+=4) {
        if (j < maxi) {
            taskmap = PACK_TASK_MAP(p[i], p[i+1], p[i+2], p[i+3]);
        } else {
            taskmap = 0;
        } 
        rv = soc_mem_write(unit, LRA_INST_TASK_MAPm, MEM_BLOCK_ALL, j, &taskmap);
        if (SOC_FAILURE(rv)) {
            LOG_CLI((BSL_META_U(unit,
                                "\nFailed while updating Taskmap index %d (%d)"), j, rv));
            return rv;
        }
    }
 

    return SOC_E_NONE;
}

/*
 * Function: 
 *    soc_sbx_caladan3_lr_download_img
 * Description:
 *    Routine to download image to LRP memory
 */
int gdebug = 0;

int
soc_sbx_caladan3_lr_download_img(int unit, int bank,
                                 soc_sbx_caladan3_ucode_pkg_t *ucode)
{
    int inum, snum, bp;
    uint32 so, se, odd, even; 
    soc_mem_t mem;
    uint32 *ip1, *ip0, w;
    uint8 *p1, *p0; 
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_lrp_t *lrp;
    int gcnt=0, cnt = 0;
    
    lrp = &(SOC_SBX_CFG_CALADAN3(unit)->lrp_cfg);
    if (!lrp) {
        return SOC_E_INIT;
    }
#if 0
    LOG_CLI((BSL_META_U(unit,
                        "\nDownloading img to bank %d"), bank));
#endif
    for (snum=0; ((snum < SOC_SBX_CALADAN3_LR_NUM_OF_STREAMS) &&
                  (snum < ucode->m_snum)); snum+=2)
    {
        cnt = 0; 
        so = ucode->m_inum * (snum + 1) * ucode->m_onum;
        se = ucode->m_inum * snum * ucode->m_onum;
        mem = SOC_SBX_CALADAN3_LR_INST_MEM(bank, snum);
        if (gdebug == 0) {
            sal_memset(&lrp->s01[0], 0, 
               sizeof(lrp->s01[0])* SOC_SBX_CALADAN3_LR_NUM_OF_INSTRS);
        } else {
            sal_memset(&lrp->s01[0], 0, 
               sizeof(lrp->s01[0])* ucode->m_inum);
        }
        for (inum=0; ((inum < SOC_SBX_CALADAN3_LR_NUM_OF_INSTRS) && 
                  (inum < ucode->m_inum)); inum++) 
        {
            odd = inum * ucode->m_onum + so;
            even = inum * ucode->m_onum + se;
            ip0 = &lrp->s01[inum].entry_data[0];
            ip1 = ip0 + (ucode->m_onum / 4);
            p0 = &ucode->m_code[even];
            p1 = &ucode->m_code[odd]; 
            for (w=ucode->m_onum/4 - 1, bp = 0; bp < ucode->m_onum; w--, bp += 4) {
                ip1[w] = 
                    PACK_INST(p1[bp], p1[bp+1], p1[bp+2], p1[bp+3]);
                ip0[w] =
                    PACK_INST(p0[bp] ,p0[bp+1], p0[bp+2], p0[bp+3]);
                if (ip1[w] == 0xffc00000) cnt++;
            }
        }
        if (gcnt) {
           if (gcnt != cnt) {
               LOG_CLI((BSL_META_U(unit,
                                   "\nContext Compare failed when downloading stream %d: this bank(%d) this stream(%d)"), snum, gcnt, cnt));
               return SOC_E_PARAM;
           }
        } else {
            gcnt = cnt;
        }
        /* Push streams 
         *   We write all the inst even though lesser may be valid
         *   The unused ones are zeroed. Zero encodes to nop.
         */
              
        /*    coverity[negative_returns : FALSE]    */
        rv = soc_mem_write_range(unit, mem, MEM_BLOCK_ALL, 0, 
                                 (SOC_SBX_CALADAN3_LR_NUM_OF_INSTRS - 1), 
                                 (void*)&lrp->s01[0]);
        if (SOC_FAILURE(rv)) {
            LOG_CLI((BSL_META_U(unit,
                                "\nFailed while downloading ucode image for bank(%d) stream(%d) rv(%d)"), bank, snum, rv));
            return rv;
        } else {
            lrp->streams_online |= 0x3 << snum;
        }
        

#if 0
        rv = soc_mem_read_range(unit, mem, MEM_BLOCK_ANY, 0, 
                                 (SOC_SBX_CALADAN3_LR_NUM_OF_INSTRS - 1), 
                                 (void*)&readback[0]);
        if (SOC_FAILURE(rv)) {
            LOG_CLI((BSL_META_U(unit,
                                "\nFailed while reading back ucode image for bank(%d) stream(%d) rv(%d)"), bank, snum, rv));
            return rv;
        }
        for (inum=0; ((inum < SOC_SBX_CALADAN3_LR_NUM_OF_INSTRS) && 
                  (inum < ucode->m_inum)); inum++) {
            if (sal_memcmp(&readback[inum], &lrp->s01[inum], 2*ucode->m_onum)!=0) {
                LOG_CLI((BSL_META_U(unit,
                                    "\nReadback failed: Stream: %d Inst: %d "), snum, inum));
                LOG_CLI((BSL_META_U(unit,
                                    "\nWrote: %x %x %x %x %x %x %x"), 
                         lrp->s01[inum].entry_data[0],
                         lrp->s01[inum].entry_data[1],
                         lrp->s01[inum].entry_data[2],
                         lrp->s01[inum].entry_data[3],
                         lrp->s01[inum].entry_data[4],
                         lrp->s01[inum].entry_data[5],
                         lrp->s01[inum].entry_data[6]));
                LOG_CLI((BSL_META_U(unit,
                                    "\nReadback: %x %x %x %x %x %x %x"), 
                         readback[inum].entry_data[0],
                         readback[inum].entry_data[1],
                         readback[inum].entry_data[2],
                         readback[inum].entry_data[3],
                         readback[inum].entry_data[4],
                         readback[inum].entry_data[5],
                         readback[inum].entry_data[6]));
                return SOC_E_INIT;
            }
        }
        if (SOC_FAILURE(rv)) break;
#endif
    }

    soc_sbx_caladan3_ucodemgr_bank_dirty_set(unit, bank, BANK_DIRTY);
    lrp->detected_context[bank] = gcnt;

    /*
     *  Load the debug stream if not yet loaded 
     */
    if (!(lrp->debug_load) && (snum == 12) && (snum < ucode->m_snum)) {
        cnt = 0; 
        se = ucode->m_inum * snum * ucode->m_onum;
        mem = SOC_SBX_CALADAN3_LR_INST_MEM(0, snum);
        sal_memset(&lrp->s01[0], 0, 
               sizeof(lrp->s01[0])* SOC_SBX_CALADAN3_LR_NUM_OF_INSTRS);
        for (inum=0; ((inum < SOC_SBX_CALADAN3_LR_NUM_OF_INSTRS) && 
                  (inum < ucode->m_inum)); inum++) 
        {
            even = inum * ucode->m_onum + se;
            ip0 = &lrp->s01[inum].entry_data[0];
            ip1 = ip0 + (ucode->m_onum / 4);
            p0 = &ucode->m_code[even];
            for (w=ucode->m_onum/4 - 1, bp = 0; bp < ucode->m_onum; w--, bp += 4) {
                ip0[w] =
                    PACK_INST(p0[bp] ,p0[bp+1], p0[bp+2], p0[bp+3]);
            }
        }
        /*    coverity[negative_returns : FALSE]    */
        rv = soc_mem_write_range(unit, mem, MEM_BLOCK_ALL, 0, 
                                 (SOC_SBX_CALADAN3_LR_NUM_OF_INSTRS - 1), 
                                 (void*)&lrp->s01[0]);
        if (SOC_FAILURE(rv)) {
            LOG_CLI((BSL_META_U(unit,
                                "\nFailed while downloading ucode image for bank(%d) stream(%d) rv(%d)"), bank, snum, rv));
            return rv;
        }
        lrp->debug_load = 1;
        lrp->debug_len = ucode->m_inum;
    }
#if 0
    LOG_CLI((BSL_META_U(unit,
                        " -> OK\n")));
#endif
    return SOC_E_NONE;
}

/*
 * Function: 
 *    soc_sbx_caladan3_lr_download
 * Description:
 *    Routine to download image to LRP memory
 *    Swaps and updates all memories if required
 */
int
soc_sbx_caladan3_lr_download(int unit, 
                                 soc_sbx_caladan3_ucode_pkg_t *ucode)
{
    int i,j, bank;
    uint8 *p;
    uint32 taskmap, maxi;
    int rv = SOC_E_NONE;
    
    /* Download image to the standby bank */
    bank = soc_sbx_caladan3_ucodemgr_standby_bank_get(unit);
    rv = soc_sbx_caladan3_lr_download_img(unit, bank, ucode);
    if (SOC_FAILURE(rv)) {
        LOG_CLI((BSL_META_U(unit,
                            "\nFailed downloading to bank %d"), bank));
        return rv;
    }

    /* Update the taskmap */
    p = ucode->m_tmap;
    maxi = (ucode->m_inum + 31)/32;
    for (j=0,i=0; j < soc_mem_index_max(unit, LRA_INST_TASK_MAPm); j++,i+=4) {
        if (j < maxi) {
            taskmap = PACK_TASK_MAP(p[i], p[i+1], p[i+2], p[i+3]);
        } else {
            taskmap = 0;
        } 
        rv = soc_mem_write(unit, LRA_INST_TASK_MAPm, MEM_BLOCK_ALL, j, &taskmap);
        if (SOC_FAILURE(rv)) {
            LOG_CLI((BSL_META_U(unit,
                                "\nFailed while updating Taskmap index %d (%d)"), j, rv));
            return rv;
        }
    }
 
    /* if the other bank needs to be updated, do it now */
    if (soc_sbx_caladan3_ucodemgr_sync_all_banks(unit)) {
        rv = soc_sbx_caladan3_lr_bank_swap(unit);
        if (SOC_SUCCESS(rv)) {
            bank = SOC_SBX_CALADAN3_LR_OTHER_BANK(bank);
            rv = soc_sbx_caladan3_lr_download_img(unit, bank, ucode);
            if (SOC_FAILURE(rv)) {
                LOG_CLI((BSL_META_U(unit,
                                    "\nFailed downloading to other bank %d"), bank));
            }
        }
    }

    return rv;
}

/*
 * Function:
 *    soc_sbx_caladan3_lr_read
 * Purpose
 *    Read a single instruction from the instruction memory
 */
int
soc_sbx_caladan3_lr_iread(void *handle, uint32 snum, uint32 inum, uint8 *inst) 
{
    int bank, unit;
    uint8 *p;
    lra_inst_b0_mem0_entry_t packedinst;
    int b = (snum & 1);
    soc_mem_t mem;
    int rv = SOC_E_NONE;
    int i;
    uint32 *pi;

    unit = PTR_TO_INT(handle);
    bank = soc_sbx_caladan3_ucodemgr_standby_bank_get(unit);
    mem = SOC_SBX_CALADAN3_LR_INST_MEM(bank, snum);
    if (inst) {
        rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY, inum, &packedinst);
        if (SOC_SUCCESS(rv)) {
            p = (uint8*)&packedinst +
                           b * SOC_SBX_CALADAN3_LR_INST_MEM_INST_BYTES;
            soc_sbx_caladan3_cmic_endian(p, 
                    SOC_SBX_CALADAN3_LR_INST_MEM_INST_BYTES);
            pi = (uint32*)p;
            for (i=0; i<SOC_SBX_CALADAN3_LR_INST_MEM_INST_BYTES/4;i++) {
                UNPACK_INST(inst, pi[i]);
                inst+=4;
            }
        }
    }
    return rv;
}

/*
 * Function:
 *    soc_sbx_caladan3_lr_iwrite_bank
 * Purpose
 *    Read a single instruction from the instruction memory bank given
 */
int
soc_sbx_caladan3_lr_iwrite_bank(int unit, int bank, uint32 snum, uint32 inum, uint8 *inst) 
{
    uint8 *p;
    lra_inst_b0_mem0_entry_t packedinst;
    int b = (snum & 1);
    soc_mem_t mem;
    int rv = SOC_E_NONE;
    int i, j;
    uint32 *pi;
 
    mem = SOC_SBX_CALADAN3_LR_INST_MEM(bank, snum);
    if (inum > SOC_SBX_CALADAN3_LR_NUM_OF_INSTRS) {
        return SOC_E_PARAM;
    }
    if (inst) {
        rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY, inum, &packedinst);
        if (SOC_SUCCESS(rv)) {
            p = (uint8*)&packedinst + 
                           b * SOC_SBX_CALADAN3_LR_INST_MEM_INST_BYTES;
            pi = (uint32*)p;
            for (i=0, j=0; i<SOC_SBX_CALADAN3_LR_INST_MEM_INST_BYTES/4;j+=4,i++)
                pi[i]=PACK_INST(inst[j+0], inst[j+1], inst[j+2], inst[j+3]);
            soc_sbx_caladan3_cmic_endian(p, 
                    SOC_SBX_CALADAN3_LR_INST_MEM_INST_BYTES);
            rv = soc_mem_write(unit, mem, MEM_BLOCK_ALL, inum, &packedinst);
            if (SOC_SUCCESS(rv)) {
                soc_sbx_caladan3_ucodemgr_bank_dirty_set(unit, bank,
                                                         BANK_DIRTY);
            }
        }
    }
    return rv;
}

/*
 * Function:
 *    soc_sbx_caladan3_lr_iwrite
 * Purpose
 *    Write a single instruction from the instruction memory
 *    Bank swap and write to the other bank if sync is required
 */
int
soc_sbx_caladan3_lr_iwrite(void* handle, uint32 snum, uint32 inum, uint8 *inst) 
{
    int rv, bank, unit;
 
    unit = PTR_TO_INT(handle);
#ifdef BCM_WARM_BOOT_SUPPORT
		if(SOC_WARM_BOOT(unit)) {
            return SOC_E_NONE;
        }
#endif
    bank = soc_sbx_caladan3_ucodemgr_standby_bank_get(unit);
    if (inum > SOC_SBX_CALADAN3_LR_NUM_OF_INSTRS) {
        return SOC_E_PARAM;
    }
    rv = soc_sbx_caladan3_lr_iwrite_bank(unit, bank, snum, inum, inst);
    if (SOC_SUCCESS(rv)) {
        if (soc_sbx_caladan3_ucodemgr_sync_all_banks(unit)) {
            rv = soc_sbx_caladan3_lr_bank_swap(unit);
            if (SOC_SUCCESS(rv)) {
                bank = SOC_SBX_CALADAN3_LR_OTHER_BANK(bank);
                soc_sbx_caladan3_lr_iwrite_bank(unit, bank, snum, inum, inst);
            } 
        }
    }
    return rv;
}

/*
 * Function:
 *     soc_sbx_caladan3_lr_ucode_prepare
 * Purpose:
 *     Prepare LR for download. Ideally nothing is required
 * since the download uses the standby bank.  
 */
int soc_sbx_caladan3_lr_ucode_prepare(int unit, 
                                      soc_sbx_caladan3_ucode_pkg_t *ucode) 
{
    uint32 regval; 
    uint32 contexts;
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_lrp_t *lrp;
    
    lrp = &(SOC_SBX_CFG_CALADAN3(unit)->lrp_cfg);
    if (!lrp) {
        return SOC_E_INIT;
    }
    if (!lrp->init_done) {
        regval = 0;
        soc_reg_field_set(unit, LRA_CONFIG0r, &regval, SOFT_RESET_Nf, 1); 
        WRITE_LRA_CONFIG0r(unit, regval);
        soc_reg_field_set(unit, LRB_CONFIG0r, &regval, SOFT_RESET_Nf, 1); 
        WRITE_LRB_CONFIG0r(unit, regval);

    }
    lrp->ucode_reload = 0;
    if (lrp->ucode_done) {
        /* 
         * New image is being downloaded on the already active lrp
         * We can do seamless transition only if the switches match
         * Prudent thing is to disable loader and then load
         */
#ifdef LR_SHUTDOWN
         rv = soc_sbx_caladan3_lr_shutdown(unit);
         if (SOC_FAILURE(rv)) {
             return rv;
         }
#else
        /*
         * alternative to controlled shutdown path
         * 1) Set LRA_CONFIG0.LOAD_ENABLE to 0.
         *    This will prevent any further requests to the SWS and the LRP will finish 
         *    processing whatever packets are already in flight.
         * 2) Set LRA_EPOCH_CONTROL.EPOCHS to (NCONTEXTS + 4) and LRA_EPOCH_CONTROL.ENABLE to 1.
         *    This is to ensure that all packets have exited and idle the instruction controller.
         * 3) Wait for the amount of time indicated by step (2)
         * 4) Modify ucode
         * 5) Set LRA_EPOCH_CONTROL.ENABLE to 0
         * 6) Set LRA_CONFIG0.LOAD_ENABLE to 1
         */

         lrp->ucode_reload = 1;

         READ_LRA_CONFIG0r(unit, &regval);
         soc_reg_field_set(unit, LRA_CONFIG0r, &regval, LOAD_ENABLEf, 0);
         WRITE_LRA_CONFIG0r(unit, regval);

         READ_LRA_CONFIG1r(unit, &regval);
         contexts = soc_reg_field_get(unit, LRA_CONFIG1r, regval, CONTEXTSf);

         READ_LRA_EPOCH_COUNT0r(unit, &regval);
         soc_reg_field_set(unit, LRA_EPOCH_COUNT0r, &regval, EPOCHSf, contexts + 4); 
         WRITE_LRA_EPOCH_COUNT0r(unit, regval);

         READ_LRA_EPOCH_CONTROLr(unit, &regval);
         soc_reg_field_set(unit, LRA_EPOCH_CONTROLr, &regval, ENABLEf, 1);
         WRITE_LRA_EPOCH_CONTROLr(unit, regval);

         sal_udelay(1000);
#endif
    }

    /* ucode download in progress */
    lrp->ucode_done = 0;

    return rv;
}

/*
 * Function:
 *     soc_sbx_caladan3_lr_ucode_done
 * Purpose:
 *     Enable LRP to run the new ucode
 */
int soc_sbx_caladan3_lr_ucode_done(int unit, 
                                   soc_sbx_caladan3_ucode_pkg_t *ucode) 
{
    soc_sbx_caladan3_lrp_t *lrp;
    int rv;
    uint32 regval;
    
    lrp = &(SOC_SBX_CFG_CALADAN3(unit)->lrp_cfg);
    if (!lrp && !lrp->init_done) {
        return SOC_E_INIT;
    }

    /* Ucode download completed */
    lrp->ucode_done = 1;

#ifndef LR_SHUTDOWN
    if (lrp->ucode_reload) {
        /*
         * alternative to controlled shutdown path
         * - post ucode download
         * 5) Set LRA_EPOCH_CONTROL.ENABLE to 0
         * 6) Set LRA_CONFIG0.LOAD_ENABLE to 1
         *   complete (LRP is enabled)
         */

         READ_LRA_EPOCH_CONTROLr(unit, &regval);
         soc_reg_field_set(unit, LRA_EPOCH_CONTROLr, &regval, ENABLEf, 0);
         WRITE_LRA_EPOCH_CONTROLr(unit, regval);

         READ_LRA_CONFIG0r(unit, &regval);
         soc_reg_field_set(unit, LRA_CONFIG0r, &regval, LOAD_ENABLEf, 1);
         WRITE_LRA_CONFIG0r(unit, regval);

         return SOC_E_NONE;
    }
#endif

    /* Enable LRP */
    rv = soc_sbx_caladan3_lr_enable(unit, lrp);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_lr_driver_init: unit %d Init failed (%d) \n"), unit, rv));
    }

    return rv;
}

/***
 ***  LRP Shared Registers
 ***/

/*
 *   Function
 *     sbx_caladan3_lr_shared_register_iaccess
 *   Purpose
 *      access LRP SVP Shared Registers
 *   Parameters
 *      (IN) unit      : unit number of the device
 *      (IN) operation : operation type (read=1/write=0)
 *      (IN) address   : address - register address
 *      (IN/OUT)data   : data
 *   Returns
 *       SOC_E_NONE    - success
 *       SOC_E_TIMEOUT - command timed out
 */
int soc_sbx_caladan3_lr_shared_register_iaccess(int unit,
                                        int operation,
                                        uint32 address,
                                        uint32 *data)
{
    int    result = SOC_E_NONE;
    uint32 uRegValue = 0;

    /* sanity checks */
    if (data == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s lr shared register - data = NULL on unit %d\n"),
                   FUNCTION_NAME(), unit));
        return SOC_E_PARAM;
    }

    WRITE_LRB_SHARED_DATAr(unit, *data);

    /* initiate the operation */
    uRegValue = 0;
    soc_reg_field_set(unit, LRB_SHARED_DATA_ACCr, &uRegValue, ACCf, 1);
    soc_reg_field_set(unit, LRB_SHARED_DATA_ACCr, &uRegValue, REG_IDf, address);
    switch (operation) {
        case 0:
            soc_reg_field_set(unit, LRB_SHARED_DATA_ACCr, &uRegValue, RW_Nf, 0);
            break;
        case 1:
            soc_reg_field_set(unit, LRB_SHARED_DATA_ACCr, &uRegValue, RW_Nf, 1);
            break;
        default:
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s lr unsupported operation %d on unit %d\n"),
                       FUNCTION_NAME(), operation, unit));
            return SOC_E_PARAM;
    }
    result = WRITE_LRB_SHARED_DATA_ACCr(unit, uRegValue);
    if (SOC_FAILURE(result)) {
        return result;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s lr shared register access 0x%x, data 0x%8x unit %d.\n"),
                 FUNCTION_NAME(), uRegValue, *data, unit));

    /* poll for done */

    /* There is no ack on this access ! */

    /* return data */

    READ_LRB_SHARED_DATAr(unit, data);

    return result;
}


int soc_sbx_lrp_setup_tmu_program(int unit, 
				  int table_index,
				  uint32 program, 
                                  int update,
				  uint8 key0valid,
                                  uint8 key1valid ) {
  uint32 mementry=0, field=0, tmu_update;


  SOC_IF_ERROR_RETURN(soc_mem_read( unit, LRB_PROGRAM_TRANSLATIONm,
                                    MEM_BLOCK_ANY, table_index, &mementry) );
  field =  (key0valid ) ? 1 : 0;
  field |= (key1valid ) ? 2 : 0;
  tmu_update = (update) ? 1 : 0;
  soc_mem_field_set( unit, LRB_PROGRAM_TRANSLATIONm,
                     &mementry, TMUf, &field );
  soc_mem_field_set( unit, LRB_PROGRAM_TRANSLATIONm,
                     &mementry, TPROGf, &program );
  soc_mem_field_set( unit, LRB_PROGRAM_TRANSLATIONm,
                     &mementry, TMU_UPDATEf, &tmu_update );
  SOC_IF_ERROR_RETURN(soc_mem_write(unit, LRB_PROGRAM_TRANSLATIONm,
                                    MEM_BLOCK_ANY, table_index, (void *) &mementry) );

  return 0;
}

int
soc_sbx_lrp_setup_etu_program(int unit,
                              int table_index,
                              uint32 program)
{
    uint32 mementry=0;
    SOC_IF_ERROR_RETURN(
        soc_mem_read(unit, LRB_PROGRAM_TRANSLATIONm,
                     MEM_BLOCK_ANY, table_index, &mementry));
    soc_mem_field32_set(unit, LRB_PROGRAM_TRANSLATIONm,
                        &mementry, ETUf, 1);
    soc_mem_field_set(unit, LRB_PROGRAM_TRANSLATIONm,
                    &mementry, EPROGf, &program);
    SOC_IF_ERROR_RETURN(
        soc_mem_write(unit, LRB_PROGRAM_TRANSLATIONm,
                      MEM_BLOCK_ANY, table_index, (void *) &mementry));
    return 0;
}


void 
soc_sbx_caladan3_lr_isr(void *handle, void *d1, void *d2, void *d3, void *d4)
{
    uint32 regval = 0;
    int unit = PTR_TO_INT(handle);
    READ_LRA_ERRORr(unit, &regval);
    /* Currently handling only debug, expand if required */
    if (soc_reg_field_get(unit, LRA_ERRORr, regval, DEBUGf)) {
        soc_sbx_caladan3_ucode_debug_isr(PTR_TO_INT(handle));
        soc_reg_field_set(unit, LRA_ERRORr, &regval, DEBUGf, 1);
    } else {    
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_lr_isr: unit %d unexpected handle (%x) \n"), unit, regval));
        soc_sbx_caladan3_ucode_debug_isr(PTR_TO_INT(handle));
    }
    soc_reg_field_set(unit, LRA_ERRORr, &regval, DIAG_ATTNf, 1);
    WRITE_LRA_ERRORr(unit, regval);
    
}

int
soc_sbx_caladan3_lr_isr_enable(int unit) 
{
    uint32 regval = 0;
    READ_LRA_ERROR_MASKr(unit, &regval);
    soc_reg_field_set(unit, LRA_ERROR_MASKr, &regval, DEBUG_DISINTf, 0);
    WRITE_LRA_ERROR_MASKr(unit, regval);

    return (soc_cmicm_intr3_enable(unit, (1<<SOC_SBX_CALADAN3_LRA_INTR_POS)));
}

int
soc_sbx_caladan3_lrb_isr_enable(int unit) 
{
    return (soc_cmicm_intr3_enable(unit, (1<<SOC_SBX_CALADAN3_LRB_INTR_POS)));
}


/*
 * function:
 *  soc_sbx_caladan3_lrb_isr
 * Purpose:
 *  Process LRB List enqueue and dqueue events
 */
void 
soc_sbx_caladan3_lrb_isr(void *handle, void *d1, void *d2, void *d3, void *d4)
{
    uint32 regval = 0;
    int idx  = 0;
    int unit = PTR_TO_INT(handle);

    READ_LRB_LIST_ENQ_EVENTr(unit, &regval);
    while (regval != 0) {
        if (regval & 1) {
            soc_sbx_caladan3_lr_list_manager_enqueue_done(unit, idx);
        }
        regval >>= 1;
        idx++;
    }
    regval = 0; idx = 0;
    READ_LRB_LIST_DEQ_EVENTr(unit, &regval);
    while (regval != 0) {
        if (regval & 1) {
            soc_sbx_caladan3_lr_list_manager_dequeue_done(unit, idx);
        }
        regval >>= 1;
        idx++;
    }
    soc_sbx_caladan3_lrb_isr_enable(unit);
}

#endif
