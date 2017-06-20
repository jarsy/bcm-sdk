/* $Id: cop_ut.c,v 1.10 Broadcom SDK $ 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>

#if defined(BCM_CALADAN3_SUPPORT)

#include "../c3sw_test.h"
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/caladan3/cop.h>
#include <soc/error.h>
#include <soc/debug.h>

static int cop_ut_test_result;
static int cop_ut_test_timer_match_result;
static int32 id[32];
static uint32 handle[32];

sbx_caladan3_ocm_port_alloc_t _cop_ut_ocm_desc[] = {
    {SOC_SBX_CALADAN3_OCM_COP0_PORT, -1, 64*1024,  SOC_SBX_CALADAN3_DATUM_SIZE_QUADWORD},
    {SOC_SBX_CALADAN3_OCM_COP1_PORT, -1, 128*1024, SOC_SBX_CALADAN3_DATUM_SIZE_QUADWORD}
};

typedef struct sbx_caladan3_cop_ut_segment_desc_s {
    int cop;
    int segment;
    int num_entry;
    soc_sbx_caladan3_cop_segment_type_e_t type;
    soc_sbx_caladan3_cop_segment_type_specific_config_t config;
} sbx_caladan3_cop_ut_segment_desc_t;

sbx_caladan3_cop_ut_segment_desc_t _cop_ut_seg_desc[5];

void c3_ut_cop_test_timer_cb(int unit, int cop) {
    int rv, index;
    sbx_caladan3_cop_ut_segment_desc_t *seg_desc;
    soc_sbx_caladan3_cop_timer_expire_event_t timer_event;

    /* dequeue a timer event */ 
    rv = soc_sbx_caladan3_cop_timer_event_dequeue(unit, cop, &timer_event);
    if (SOC_FAILURE(rv)) {
        cli_out("C3 failed to dequeue timer event on COP %d unit %d\n", cop, unit);
	return;
    } else {
        cli_out("timer event dequeueed on COP %d, segment %d, id %d\n",
                timer_event.uCop, timer_event.uSegment, timer_event.uTimer);
        
    }

    /* check the event got is from the right cop/segment/id, assuming there is only
     * 1 timer segment in the test
     */
    for (index = 0; index < sizeof(_cop_ut_seg_desc)/sizeof(sbx_caladan3_cop_ut_segment_desc_t); index++) {
	seg_desc = &_cop_ut_seg_desc[index];

	if (seg_desc->type == SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_TIMER) {
	    if ((seg_desc->cop == timer_event.uCop) &&
		(seg_desc->segment == timer_event.uSegment) &&
		((handle[index] & 0x3ffFFFF) == timer_event.uTimer)) {
	        cop_ut_test_timer_match_result = 0;
	    }
	}
    }    

    return;
}

int c3_ut_cop_test1_init(c3sw_test_info_t *pc3swTestInfo, void *pUserData)
{
    int rv, unit, index;
    int cop;
    sbx_caladan3_ocm_port_alloc_t *ocm_desc;
    
    unit = pc3swTestInfo->unit;

    _cop_ut_seg_desc[0].cop = 0;
    _cop_ut_seg_desc[0].segment = 0;
    _cop_ut_seg_desc[0].num_entry = 16*1024;
    _cop_ut_seg_desc[0].type = SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_POLICER;
    _cop_ut_seg_desc[0].config.sPolicer.bErrorMask = 0;
    _cop_ut_seg_desc[0].config.sPolicer.nErrorColor = 0;
    _cop_ut_seg_desc[0].config.sPolicer.uMaxRateKbps = 100000000;
    _cop_ut_seg_desc[0].config.sPolicer.uMaxBurstBits = 10000000;

    _cop_ut_seg_desc[1].cop = 0;
    _cop_ut_seg_desc[1].segment = 1;
    _cop_ut_seg_desc[1].num_entry = 8*1024;
    _cop_ut_seg_desc[1].type = SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_POLICER;
    _cop_ut_seg_desc[1].config.sPolicer.bErrorMask = 0;
    _cop_ut_seg_desc[1].config.sPolicer.nErrorColor = 0;
    _cop_ut_seg_desc[1].config.sPolicer.uMaxRateKbps = 100000000;
    _cop_ut_seg_desc[1].config.sPolicer.uMaxBurstBits = 10000000;

    _cop_ut_seg_desc[2].cop = 1;
    _cop_ut_seg_desc[2].segment = 0;
    _cop_ut_seg_desc[2].num_entry = 32*1024;
    _cop_ut_seg_desc[2].type = SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_TIMER;
    _cop_ut_seg_desc[2].config.sTimer.bMode64 = 1;
    _cop_ut_seg_desc[2].config.sTimer.nTimerTickUs = 10000;

    _cop_ut_seg_desc[3].cop = 0;
    _cop_ut_seg_desc[3].segment = 2;
    _cop_ut_seg_desc[3].num_entry = 1*1024;
    _cop_ut_seg_desc[3].type = SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_SN_CHECKER;
    _cop_ut_seg_desc[3].config.sChecker.bMode32 = 1;
    _cop_ut_seg_desc[3].config.sChecker.uSequenceRange = 0;

    _cop_ut_seg_desc[4].cop = 1;
    _cop_ut_seg_desc[4].segment = 1;
    _cop_ut_seg_desc[4].num_entry = 4*1024;
    _cop_ut_seg_desc[4].type = SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_COHERENT;
    _cop_ut_seg_desc[4].config.sCoherent.bReturnNext = 0;
    _cop_ut_seg_desc[4].config.sCoherent.nOverflowMode = SOC_SBX_CALADAN3_COP_COHERENT_OVERFLOW_STICKY;
    _cop_ut_seg_desc[4].config.sCoherent.nFormat = SOC_SBX_CALADAN3_COP_COHERENT_FORMAT_4BIT;

    /*
    rv = soc_sbx_caladan3_cop_driver_init(unit);
    if ((rv != SOC_E_INIT) && SOC_FAILURE(rv)) {
	cli_out("C3 %d COP driver init failed\n",pc3swTestInfo->unit);
	return rv;
    }
    */

    rv = soc_sbx_caladan3_cop_timer_event_callback_register(unit, c3_ut_cop_test_timer_cb);
    if (SOC_FAILURE(rv)) {
        cli_out("C3 %d failed to register timer event callback\n",unit);
	return rv;
    }

    /* alloc ocm memory for cop0 and cop1 */
    for (index = 0; index < sizeof(_cop_ut_ocm_desc)/sizeof(sbx_caladan3_ocm_port_alloc_t); index++) {
	ocm_desc = &_cop_ut_ocm_desc[index];
	cop = index;

	rv = soc_sbx_caladan3_ocm_port_mem_alloc(unit, ocm_desc);
	if (SOC_FAILURE(rv)) {
            cli_out("C3 %d OCM memory Allocation failed for port %d\n",unit, ocm_desc->port);
            return rv;
	}

	rv = soc_sbx_caladan3_cop_ocm_memory_size_set(unit, cop, ocm_desc->size*ocm_desc->datum_size/8);
	if (SOC_FAILURE(rv)) {
            cli_out("Failed to config COP %d mem size on unit %d\n", cop, unit);
            return rv;
	}

	rv = soc_sbx_caladan3_cop_timer_event_queue_size_set(unit, cop, 8096);
	if (SOC_FAILURE(rv)) {
            cli_out("Failed to config COP %d timer event queue size on unit %d\n", cop, unit);
            return rv;
	}
    }

    return rv;
}


int
c3_ut_cop_test1_run(c3sw_test_info_t *pc3swTestInfo, void *pUserData)
{
    int rv, unit, index;
    soc_sbx_caladan3_cop_policer_config_t sPolicer, sPolicerRead;
    soc_sbx_caladan3_cop_timer_config_t sTimer, sTimerRead;
    sbx_caladan3_cop_ut_segment_desc_t *seg_desc;

    cop_ut_test_result = -1;
    cop_ut_test_timer_match_result = -1;
    sal_memset(&sTimer, 0, sizeof(soc_sbx_caladan3_cop_timer_config_t));
    
    unit = pc3swTestInfo->unit;

    /* register the segments, create one entry on each segment */
    for (index = 0; index < sizeof(_cop_ut_seg_desc)/sizeof(sbx_caladan3_cop_ut_segment_desc_t); index++) {
	seg_desc = &_cop_ut_seg_desc[index];

	if (SAL_BOOT_QUICKTURN && (seg_desc->type == SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_TIMER)) {
	    seg_desc->config.sTimer.nTimerTickUs /= 100;
	}

	rv = soc_sbx_caladan3_cop_segment_register(unit, seg_desc->cop, seg_desc->segment,
						   seg_desc->num_entry, seg_desc->type,
						   &seg_desc->config);
	if (SOC_FAILURE(rv)) {
            cli_out("Failed to register COP %d segment %d on unit %d\n", seg_desc->cop,
                    seg_desc->segment, unit);
	    return rv;
	}

	id[index] = 10+index;
	switch (seg_desc->type) {
	    case SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_POLICER:
		/* create policer */
		if (seg_desc->segment == 0) {
		    sPolicer.uRfcMode = SOC_SBX_CALADAN3_COP_POLICER_RFC_MODE_2698;
		} else {
		    sPolicer.uRfcMode = SOC_SBX_CALADAN3_COP_POLICER_RFC_MODE_MEF;
		}
		sPolicer.uCIR = 50000000;       /* 50Gbps */
		sPolicer.uCBS = 5*1024*1024;    /* 5Mbits */
		sPolicer.uEIR = 10000000;       /* 10Gbps */ 
		sPolicer.uEBS = 1*1024*1024;    /* 1Mbits */
		sPolicer.bBlindMode = FALSE;
		sPolicer.bDropOnRed = TRUE;
		sPolicer.bCoupling = FALSE;
		sPolicer.bCBSNoDecrement = TRUE;
		sPolicer.bEBSNoDecrement = TRUE;
		sPolicer.bCIRStrict = TRUE;
		sPolicer.bEIRStrict = TRUE;
		sPolicer.nLenAdjust = 10;
		sPolicer.bPktMode = FALSE;

		rv = soc_sbx_caladan3_cop_policer_create(unit, seg_desc->cop, seg_desc->segment,
							 id[index], &sPolicer, &handle[index]);
		if (SOC_FAILURE(rv)) {
		    cli_out("Failed to create policer %d on COP %d segment %d unit %d\n",
                            id[index], seg_desc->cop, seg_desc->segment, unit);
		    return rv;
		}
		break;
	    case SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_TIMER:
		/* create timer */
	        sTimer.uTimeout = 50000;  /* 50ms timer */
	        if (SAL_BOOT_QUICKTURN) {
		    sTimer.uTimeout /= 100;  /* 500us timer */
		}
		sTimer.bInterrupt = TRUE;

		rv = soc_sbx_caladan3_cop_timer_create(unit, seg_desc->cop, seg_desc->segment,
						       id[index], &sTimer, &handle[index]);
		if (SOC_FAILURE(rv)) {
		    cli_out("Failed to create timer %d on COP %d segment %d unit %d\n",
                            id[index], seg_desc->cop, seg_desc->segment, unit);
		    return rv;
		}
		break;
	    case SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_SN_CHECKER:
		/* create seq gen */		
		rv = soc_sbx_caladan3_cop_seq_checker_create(unit, seg_desc->cop, seg_desc->segment,
							     id[index], 0x1F, &handle[index]);
		if (SOC_FAILURE(rv)) {
		    cli_out("Failed to create sequence checker %d on COP %d segment %d unit %d\n",
                            id[index], seg_desc->cop, seg_desc->segment, unit);
		    return rv;
		}
		break;
	    case SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_COHERENT:		
		/* create coherent table */
		rv = soc_sbx_caladan3_cop_coherent_table_create(unit, seg_desc->cop, seg_desc->segment,
								id[index], 1, 0, &handle[index]);
		if (SOC_FAILURE(rv)) {
		    cli_out("Failed to create coherent table %d on COP %d segment %d unit %d\n",
                            id[index], seg_desc->cop, seg_desc->segment, unit);
		    return rv;
		}
		break;
	    default:
		cop_ut_test_result = -1;		
		return -1;
	}
    }
    
    /* how to verify that policer/timer/seq_checker/coherent table is created properly? 
     * for now just read back the configs
     */
    for (index = 0; index < sizeof(_cop_ut_seg_desc)/sizeof(sbx_caladan3_cop_ut_segment_desc_t); index++) {
	seg_desc = &_cop_ut_seg_desc[index];

	switch (seg_desc->type) {
	    case SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_POLICER:
	        /* read policer */
	        rv = soc_sbx_caladan3_cop_policer_read(unit, handle[index], &sPolicerRead);
		if (SOC_FAILURE(rv)) {
		    cli_out("Failed to read policer %d on COP %d segment %d unit %d\n",
                            id[index], seg_desc->cop, seg_desc->segment, unit);
		    return rv;
		}
		break;
	    case SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_TIMER:
	        /* read timer */
	        rv = soc_sbx_caladan3_cop_timer_read(unit, handle[index], &sTimerRead);
		if (SOC_FAILURE(rv)) {
		    cli_out("Failed to read timer %d on COP %d segment %d unit %d\n",
                            id[index], seg_desc->cop, seg_desc->segment, unit);
		    return rv;
		}
		break;
	    case SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_SN_CHECKER:
	    case SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_COHERENT:		
	        /* do nothing for now */
		break;
	    default:
		cop_ut_test_result = -1;		
		return -1;
	}
    }    

    /* wait a while for timer event to get back */
    cli_out("wait for timer event callback\n");
    if (SAL_BOOT_QUICKTURN) {
        sal_sleep(10);
    } else {
        sal_sleep(1);
    }

    /* delete all entries */
    for (index = 0; index < sizeof(_cop_ut_seg_desc)/sizeof(sbx_caladan3_cop_ut_segment_desc_t); index++) {
	seg_desc = &_cop_ut_seg_desc[index];

	switch (seg_desc->type) {
	    case SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_POLICER:
	        /* delete policer */
		rv = soc_sbx_caladan3_cop_policer_delete(unit, handle[index]);
		if (SOC_FAILURE(rv)) {
		    cli_out("Failed to delete policer %d on COP %d segment %d unit %d\n",
                            id[index], seg_desc->cop, seg_desc->segment, unit);
		    return rv;
		}
		break;
	    case SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_TIMER:
	        /* delete timer */
		rv = soc_sbx_caladan3_cop_timer_delete(unit, handle[index]);
		if (SOC_FAILURE(rv)) {
		    cli_out("Failed to delete timer %d on COP %d segment %d unit %d\n",
                            id[index], seg_desc->cop, seg_desc->segment, unit);
		    return rv;
		}
		break;
	    case SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_SN_CHECKER:
	        /* delete seq gen */
		rv = soc_sbx_caladan3_cop_seq_checker_delete(unit, handle[index]);
		if (SOC_FAILURE(rv)) {
		    cli_out("Failed to delete sequence checker %d on COP %d segment %d unit %d\n",
                            id[index], seg_desc->cop, seg_desc->segment, unit);
		    return rv;
		}
		break;
	    case SOC_SBX_CALADAN3_COP_SEGMENT_TYPE_COHERENT:		
	        /* delete coherent table */
		rv = soc_sbx_caladan3_cop_coherent_table_delete(unit, handle[index]);
		if (SOC_FAILURE(rv)) {
		    cli_out("Failed to delete coherent table %d on COP %d segment %d unit %d\n",
                            id[index], seg_desc->cop, seg_desc->segment, unit);
		    return rv;
		}
		break;
	    default:
		cop_ut_test_result = -1;		
		return -1;
	}

	/* unregister all segments */
	rv = soc_sbx_caladan3_cop_segment_unregister(unit, seg_desc->cop, seg_desc->segment);
	if (SOC_FAILURE(rv)) {
            cli_out("Failed to unregister COP %d segment %d on unit %d\n", seg_desc->cop,
                    seg_desc->segment, unit);
	    return rv;
	}
    }

    cop_ut_test_result = 0;
    return 0;
}

int
c3_ut_cop_test1_done(c3sw_test_info_t *pc3swTestInfo, void *pUserData)
{
    int rv, unit, index;
    int cop;
    sbx_caladan3_ocm_port_alloc_t *ocm_desc;

    if ((cop_ut_test_result < 0) || (cop_ut_test_timer_match_result < 0)){
        /* dont cleanup, helps debugging */
        return -1;
    }

    unit = pc3swTestInfo->unit;

    /* free all the ocm memory */
    for (index = 0; index < sizeof(_cop_ut_ocm_desc)/sizeof(sbx_caladan3_ocm_port_alloc_t); index++) {
	ocm_desc = &_cop_ut_ocm_desc[index];
	cop = index;
	
	rv = 0;
	rv = soc_sbx_caladan3_cop_ocm_memory_size_set(unit, cop, 0);
	if (SOC_FAILURE(rv)) {
            cli_out("Failed to config COP %d mem size on unit %d\n", cop, unit);
            return rv;
	}

	rv = soc_sbx_caladan3_ocm_port_mem_free(unit, ocm_desc);
	if (SOC_FAILURE(rv)) {
            cli_out("C3 %d OCM memory free failed for port %d\n",unit, ocm_desc->port);
            return rv;
	}
    }

    return 0;
}

#endif /* #ifdef BCM_CALADAN3_SUPPORT */
