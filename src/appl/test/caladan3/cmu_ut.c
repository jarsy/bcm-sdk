/* $Id: cmu_ut.c,v 1.9 Broadcom SDK $ 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>

#if defined(BCM_CALADAN3_SUPPORT)

#include "../c3sw_test.h"
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/caladan3/cmu.h>
#include <soc/error.h>

static int cmu_ut_test_result;

sbx_caladan3_ocm_port_alloc_t _cmu_ut_ocm_desc[] = {
    {SOC_SBX_CALADAN3_OCM_CMU0_PORT, -1, 64, SOC_SBX_CALADAN3_DATUM_SIZE_QUADWORD},
    {SOC_SBX_CALADAN3_OCM_CMU1_PORT, -1, 64, SOC_SBX_CALADAN3_DATUM_SIZE_QUADWORD}
};

#define _C3_CMU_OCM_LIST_SIZE (sizeof(_cmu_ut_ocm_desc)/sizeof(sbx_caladan3_ocm_port_alloc_t))
#define _C3_CMU_COUNTER_TEST_MAXID 4

int c3_ut_cmu_test1_init(c3sw_test_info_t *pc3swTestInfo, void *pUserData)
{
  int rv, unit;
  int segment, index;
  sbx_caladan3_ocm_port_alloc_t *desc;
  soc_sbx_caladan3_cmu_segment_type_e_t type;

  unit = pc3swTestInfo->unit;

  /* bringup ocm */
  rv = soc_sbx_caladan3_ocm_driver_init(pc3swTestInfo->unit);
  if (SOC_FAILURE(rv) && rv != SOC_E_INIT) {
      cli_out("C3 %d OCM driver init failed\n",pc3swTestInfo->unit);
      return rv;
  }

  /* bringup cmu */
  rv = soc_sbx_caladan3_cmu_driver_init(unit);
  if ((rv != SOC_E_INIT) && SOC_FAILURE(rv)) {
      cli_out("C3 %d CMU driver init failed\n",pc3swTestInfo->unit);
      return rv;
  }

  /* allocate ocm memory, register cmu groups */
  for (index=0; index < _C3_CMU_OCM_LIST_SIZE; index++) {
      desc = &_cmu_ut_ocm_desc[index];

      /* allocate memory */
      rv = soc_sbx_caladan3_ocm_port_mem_alloc(unit, desc);

      if (SOC_FAILURE(rv)) {
	  cli_out("C3 %d Allocation failed for port %d\n",unit, desc->port);
	  return rv;
      } else {
	  /* register counter group */
	  segment = index;
	  if (desc->port == SOC_SBX_CALADAN3_OCM_CMU0_PORT) {
	      type = SOC_SBX_CALADAN3_CMU_SEGMENT_TYPE_TURBO_32B;
	  } else {
	      type = SOC_SBX_CALADAN3_CMU_SEGMENT_TYPE_TURBO_64B;
	  }

	  /* set OCM memory size for each CMU port */
	  rv = soc_sbx_caladan3_cmu_ocm_memory_size_set(unit, 
							desc->port,
							desc->size * sizeof(uint64));
	  if (SOC_FAILURE(rv)) {
	      LOG_ERROR(BSL_LS_APPL_COMMON,
                        (BSL_META("%s Failed to register OCM memory for CMU port %d on unit %d\n"),
                         FUNCTION_NAME(), desc->port, unit));
	      return rv;
	  }

	  rv = soc_sbx_caladan3_cmu_counter_group_register(unit,
						    (uint32*)&segment,
						    desc->size,
						    desc->port,
						    type);
	  if (SOC_FAILURE(rv)) {
	      LOG_ERROR(BSL_LS_APPL_COMMON,
                        (BSL_META("%s Failed to register CMU%d segment %d on unit %d\n"),
                         FUNCTION_NAME(), desc->port, segment, unit));
	      return rv;
	  }
      }
  }

  return rv;
}


/* manual flush test */
int
c3_ut_cmu_test1_run(c3sw_test_info_t *pc3swTestInfo, void *pUserData)
{
    int rv, unit, segment, index;
    int counter, num_counters = _C3_CMU_COUNTER_TEST_MAXID;
    uint64 *buff;
    uint64 *counter_buff;
    uint32 write_buf[_C3_CMU_COUNTER_TEST_MAXID*2];
    int failed=FALSE;

    unit = pc3swTestInfo->unit;
    cmu_ut_test_result = -1;

    /* allocate buffer for turbo counters */
    buff = (uint64*) sal_alloc(num_counters * sizeof(uint64) * 2, "cmu counters");
    sal_memset(buff, 0, num_counters * sizeof(uint64) * 2);
    cli_out("counter buffer at 0x%x\n", (uint32)buff);

    /* set OCM memory for the counters */
    cli_out("\n OCM memory write for CMU segment 0\n");
    for (counter = 0; counter < sizeof(write_buf)/sizeof(uint32); counter++) {
        /* hardcode for now for 32 bits turbo counter */
        if (counter < num_counters) {
	    if (counter % 2) { 
		write_buf[counter]=(counter)+((counter) << 19);
	    } else {
		write_buf[counter]=(counter+2)+((counter+2) << 19);
	    }
	} else {
	    write_buf[counter]=0;
	}
    }

    for (counter = 0; counter < sizeof(write_buf)/sizeof(uint32); counter++) {    
        if ((counter % 2) == 0) {
	    rv = soc_sbx_caladan3_ocm_port_mem_write(unit, SOC_SBX_CALADAN3_OCM_CMU0_PORT, -1, 
						     counter/2, counter/2, &write_buf[counter]);
	    if (SOC_FAILURE(rv)) {
	        cli_out("C3 %d write to port %d memory failed\n",unit,
                        SOC_SBX_CALADAN3_OCM_CMU0_PORT);
		return rv;
	    } 
	}
    }

    cli_out("\n OCM memory write for CMU segment 1\n");
    for (counter = 0; counter < sizeof(write_buf)/sizeof(uint32); counter++) {
        /* hardcode for now for 64 bits turbo counter */
	write_buf[counter]=counter/2+1;	
    }

    for (counter = 0; counter < sizeof(write_buf)/sizeof(uint32); counter++) {    
        if ((counter % 2) == 0) {
	    rv = soc_sbx_caladan3_ocm_port_mem_write(unit, SOC_SBX_CALADAN3_OCM_CMU1_PORT, -1, 
						     counter/2, counter/2, &write_buf[counter]);
	    if (SOC_FAILURE(rv)) {
	        cli_out("C3 %d write to port %d memory failed\n",unit,
                        SOC_SBX_CALADAN3_OCM_CMU1_PORT);
		return rv;
	    } 
	}
    }

    /* disable background flush */
    for (index=0; index < _C3_CMU_OCM_LIST_SIZE; index++) {
        segment = index;
        rv = soc_sbx_caladan3_cmu_segment_background_flush_enable(unit, segment, FALSE);
	if (SOC_FAILURE(rv)) {
	    cli_out("failed to disable background flush for segment %d on unit %d\n",
                    segment, unit);
	    return rv;
	}

	/* read counters using manual eject */
	cli_out("\n start manual eject counters 0 - %d on CMU segment %d\n",
                num_counters-1, segment);
	rv = soc_sbx_caladan3_cmu_counter_read(unit, segment, 0, num_counters, 
					       buff, TRUE, FALSE);
	if (SOC_FAILURE(rv)) {
  	    cli_out("failed to read counter on unit %d\n", unit);
	    return rv;	    
	}
	cli_out("\n manual eject done for counters 0 - %d on CMU segment %d\n",
                num_counters-1, segment);

	/* check the counters */
	counter_buff = buff;
	for (counter = 0; counter < num_counters; counter++) {
          if (COMPILER_64_LO(*counter_buff) != counter+1) {
	        LOG_ERROR(BSL_LS_APPL_COMMON,
                          (BSL_META("counter %d byte counter 0x%x%08x not at expected value 0x%x "
                                    "on segment %d unit %d\n"),
                           counter, COMPILER_64_HI(*counter_buff), COMPILER_64_LO(*counter_buff), counter+1, segment, unit));
		failed = TRUE;
	    } else {
	        LOG_VERBOSE(BSL_LS_APPL_COMMON,
	                    (BSL_META("counter %d byte counter 0x%x%08x "),
	                     counter, COMPILER_64_HI(*counter_buff), COMPILER_64_LO(*counter_buff)));
	    }
	    counter_buff++;

 	    if (COMPILER_64_LO(*counter_buff) != counter+1) {
	        LOG_ERROR(BSL_LS_APPL_COMMON,
                          (BSL_META("counter %d packet counter 0x%x%08x not at expected value 0x%x "
                                    "on segment %d unit %d\n"),
                           counter, COMPILER_64_HI(*counter_buff), COMPILER_64_LO(*counter_buff), counter+1, segment, unit));
		failed = TRUE;
	    } else {
	        LOG_VERBOSE(BSL_LS_APPL_COMMON,
	                    (BSL_META("packet counter 0x%x%08x on segment %d unit %d\n"),
	                     COMPILER_64_HI(*counter_buff), COMPILER_64_LO(*counter_buff), segment, unit));
	    }
	    counter_buff++;
	}
    }

    if (failed == FALSE) {
       cmu_ut_test_result = 0;
       cli_out("\n manual eject counters match expected value\n");
    }

    sal_free(buff);
    return 0;
}

/* ring processing thread test */

int
c3_ut_cmu_test1_done(c3sw_test_info_t *pc3swTestInfo, void *pUserData)
{
    int rv, index, unit, segment;
    sbx_caladan3_ocm_port_alloc_t *desc;

    if (cmu_ut_test_result < 0) {
        /* dont cleanup, helps debugging */
        return -1;
    }

    unit = pc3swTestInfo->unit;

    for (index=0; index < _C3_CMU_OCM_LIST_SIZE; index++) {
        desc = &_cmu_ut_ocm_desc[index];
	segment = index;
        cli_out("\n Unregister segment %d", segment);
	rv = soc_sbx_caladan3_cmu_counter_group_unregister(unit, segment);
        if (SOC_FAILURE(rv)) {
            cli_out("C3 %d Unregiser failed for segment %d\n",unit, segment);
            return rv;
        }

        cli_out("\n Freeing port %d memories", desc->port);
	rv = soc_sbx_caladan3_ocm_port_mem_free(unit, desc);
        if (SOC_FAILURE(rv)) {
            cli_out("C3 %d Free failed for port %d\n",unit, desc->port);
            return rv;
        }
    }

    return 0;
}

/* background flush ang ring processing thread test */
int c3_ut_cmu_test2_init(c3sw_test_info_t *pc3swTestInfo, void *pUserData)
{
    return c3_ut_cmu_test1_init(pc3swTestInfo, pUserData);
}

int
c3_ut_cmu_test2_run(c3sw_test_info_t *pc3swTestInfo, void *pUserData)
{
    int rv, unit, segment, index;
    int counter, num_counters = _C3_CMU_COUNTER_TEST_MAXID;
    uint64 *buff;
    uint64 *counter_buff;
    uint32 write_buf[_C3_CMU_COUNTER_TEST_MAXID*2];
    int failed = FALSE;

    unit = pc3swTestInfo->unit;
    cmu_ut_test_result = -1;

    /* allocate buffer for turbo counters */
    buff = (uint64*) sal_alloc(num_counters * sizeof(uint64) * 2, "cmu counters");
    sal_memset(buff, 0, num_counters * sizeof(uint64) * 2);
    cli_out("counter buffer at 0x%x\n", (uint32)buff);

    /* set OCM memory for the counters */
    for (counter = 0; counter < sizeof(write_buf)/sizeof(uint32); counter++) {
        /* hardcode for now for 32 bits turbo counter */
        if (counter < num_counters) {
	    if (counter % 2) { 
		write_buf[counter]=(counter)+((counter) << 19);
	    } else {
		write_buf[counter]=(counter+2)+((counter+2) << 19);
	    }
	} else {
	    write_buf[counter]=0;
	}
    }

    for (counter = 0; counter < sizeof(write_buf)/sizeof(uint32); counter++) {    
        if ((counter % 2) == 0) {
	    rv = soc_sbx_caladan3_ocm_port_mem_write(unit, SOC_SBX_CALADAN3_OCM_CMU0_PORT, -1, 
						     counter/2, counter/2, &write_buf[counter]);
	    if (SOC_FAILURE(rv)) {
	        cli_out("C3 %d write to port %d memory failed\n",unit,
                        SOC_SBX_CALADAN3_OCM_CMU0_PORT);
		return rv;
	    } 
	}
    }

    for (counter = 0; counter < sizeof(write_buf)/sizeof(uint32); counter++) {
        /* hardcode for now for 64 bits turbo counter */
	write_buf[counter]=counter/2+1;	
    }

    for (counter = 0; counter < sizeof(write_buf)/sizeof(uint32); counter++) {    
        if ((counter % 2) == 0) {
	    rv = soc_sbx_caladan3_ocm_port_mem_write(unit, SOC_SBX_CALADAN3_OCM_CMU1_PORT, -1, 
						     counter/2, counter/2, &write_buf[counter]);
	    if (SOC_FAILURE(rv)) {
	        cli_out("C3 %d write to port %d memory failed\n",unit,
                        SOC_SBX_CALADAN3_OCM_CMU1_PORT);
		return rv;
	    } 
	}
    }

    /* enable background flush */
    for (index=0; index < _C3_CMU_OCM_LIST_SIZE; index++) {
        segment = index;
	cli_out("enable background flush for segment %d on unit %d\n", segment, unit);
        rv = soc_sbx_caladan3_cmu_segment_background_flush_enable(unit, segment, TRUE);
	if (SOC_FAILURE(rv)) {
	    cli_out("failed to enable background flush for segment %d on unit %d\n",
                    segment, unit);
	}
    }

    /* wait for the background flush to flush out counter */
    sal_sleep(10);
    cli_out("background flush wait done on unit %d\n", unit);

    /* disable background flush */
    for (index=0; index < _C3_CMU_OCM_LIST_SIZE; index++) {
        segment = index;
	cli_out("disable background flush for segment %d on unit %d\n", segment, unit);
        rv = soc_sbx_caladan3_cmu_segment_background_flush_enable(unit, segment, FALSE);
	if (SOC_FAILURE(rv)) {
	    cli_out("failed to disable background flush for segment %d on unit %d\n",
                    segment, unit);
	}
    }

    /* only test counter on segment 0, otherwise takeing too long */
    for (index=0; index < 1; index++) {
        segment = index;
	/* read counters without manual eject */
	rv = soc_sbx_caladan3_cmu_counter_read(unit, segment, 0, num_counters, 
					       buff, FALSE, FALSE);
	if (SOC_FAILURE(rv)) {
  	    cli_out("failed to read counter on unit %d\n", unit);
	    return rv;	    
	}

	/* check the counters */
	counter_buff = buff;
	for (counter = 0; counter < num_counters; counter++) {
          if (COMPILER_64_LO(*counter_buff) != counter+1) {
	        LOG_ERROR(BSL_LS_APPL_COMMON,
                          (BSL_META("counter %d byte counter 0x%x%08x not at expected value 0x%x "
                                    "on segment %d unit %d\n"),
                           counter, COMPILER_64_HI(*counter_buff), COMPILER_64_LO(*counter_buff), counter+1, segment, unit));
		failed = TRUE;
	    } else {
	        LOG_VERBOSE(BSL_LS_APPL_COMMON,
	                    (BSL_META("counter %d byte counter 0x%x%08x"),
	                     counter, COMPILER_64_HI(*counter_buff), COMPILER_64_LO(*counter_buff)));
	    }
	    counter_buff++;
	    
	    if (COMPILER_64_LO(*counter_buff) != counter+1) {
	        LOG_ERROR(BSL_LS_APPL_COMMON,
                          (BSL_META("counter %d packet counter 0x%x%08x not at expected value 0x%x "
                                    "on segment %d unit %d\n"),
                           counter, COMPILER_64_HI(*counter_buff), COMPILER_64_LO(*counter_buff), counter+1, segment, unit));
		failed = TRUE;
	    } else {
	        LOG_VERBOSE(BSL_LS_APPL_COMMON,
	                    (BSL_META("packet counter 0x%x%08x on segment %d unit %d\n"),
	                     COMPILER_64_HI(*counter_buff), COMPILER_64_LO(*counter_buff), segment, unit));
	    }
	    counter_buff++;
	}
    }

    if (failed == FALSE) {
        cmu_ut_test_result = 0;
	cli_out("\n background eject counters match expected value\n");
    }

    sal_free(buff);
    return 0;
}

int
c3_ut_cmu_test2_done(c3sw_test_info_t *pc3swTestInfo, void *pUserData)
{
    return c3_ut_cmu_test1_done(pc3swTestInfo, pUserData);
}

#endif /* #ifdef BCM_CALADAN3_SUPPORT */
