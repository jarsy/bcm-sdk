/*
 * $Id: sirius_ddr23.c,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        sirius_ddr.c
 * Purpose:     Sirius ddr functions
 * Requires:
 */

#include <shared/bsl.h>

#include <soc/error.h>
#include <soc/debug.h>
#include <soc/types.h>
#include <soc/cm.h>
#include <sal/appl/sal.h>
#include <soc/phy/ddr23.h>
#include <soc/sbx/sirius_ddr23.h>
#include <soc/sbx/sirius.h>



/*
 * Mutex lock
 */
#ifdef DDR_TRAINING_LOCK
#undef DDR_TRAINING_LOCK
#endif
#ifdef DDR_TRAINING_UNLOCK
#undef DDR_TRAINING_UNLOCK
#endif

#define SIRIUS_DDR23_TIMEOUT (1000)

static  sal_mutex_t           _mlock[SOC_MAX_NUM_DEVICES];
#define DDR_TRAINING_LOCK(unit)    sal_mutex_take(_mlock[unit], sal_mutex_FOREVER)
#define DDR_TRAINING_UNLOCK(unit)  sal_mutex_give(_mlock[unit])

int gCiTrainMask = 0;  /* CI's requiring SW training */

/*
 * DDR Training information
 *
 * Contains information on the DDR training thread
 */
typedef struct _ddr23_training_thread_s {
  VOL sal_thread_t  thread_id;        /* ddr training thread task id */
  char              thread_name[16];  /* ddr training thread task name */
  VOL int           interval;         /* Train interval in usec */
                                      /* (Zero when thread trains continously) */
                                      /* (-1) training thread is not running */
  VOL sal_sem_t     trigger;          /* Trigger training thread attn */
} _ddr23_training_thread_t;

/*
 * DDR Training Control
 *
 * Contains information on the ddr training module on a given unit.
 */
typedef struct _ddr23_training_control_s {
  VOL uint32              flags;    
  _ddr23_training_thread_t  train;           /* ddr training thread */
  /* add other control params here */
} _ddr23_training_control_t;

static _ddr23_training_control_t    *_ddr23_training_control[SOC_MAX_NUM_DEVICES];

#define DDR_TRAINING_CONTROL(_unit)             (_ddr23_training_control[_unit])
#define DDR_TRAINING_THREAD(_unit)             (DDR_TRAINING_CONTROL(_unit)->train)

/*
 * Function:
 *     soc_sbx_sirius_sw_ddr23_train_init
 * Purpose:
 *     Initialize ddr training module.
 * Parameters:
 *     unit        - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *
 */

int
soc_sbx_sirius_sw_ddr23_train_init(int unit, siriusInitParams_t *pInitParams)
{
  int rv = SOC_E_NONE;
  int interval_us = 0;
  int env_interval = 0;
  int flags = 0;
  int ci = 0;
  uint32 uData = 0;

  UNIT_VALID_CHECK(unit);
  
  gCiTrainMask = 0;
  for (ci = 0; ci < pInitParams->uDdr3NumMemories; ci++ ) {
    if (pInitParams->ci[ci].bHwRunTimeDDRCalibration == FALSE) {
      gCiTrainMask |= (1 << ci);
      /* be sure HW autocalibration is off */
      SOC_IF_ERROR_RETURN(READ_DDR23_PHY_BYTE_LANE0_VDL_CALIBRATEr(unit,ci,&uData));
      DDR23_SET_FIELD(uData,DDR23_PHY_BYTE_LANE0,VDL_CALIBRATE,CALIB_ALWAYS,0x0); 
      SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE0_VDL_CALIBRATEr(unit,ci,uData));

      SOC_IF_ERROR_RETURN(READ_DDR23_PHY_BYTE_LANE1_VDL_CALIBRATEr(unit,ci,&uData));
      DDR23_SET_FIELD(uData,DDR23_PHY_BYTE_LANE0,VDL_CALIBRATE,CALIB_ALWAYS,0x0); 
      SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE1_VDL_CALIBRATEr(unit,ci,uData));
    }
  }

  if (gCiTrainMask == 0) {
    /* doing autocalibration in hw for all CI's */
    return SOC_E_NONE;
  }

  if((env_interval = soc_property_get(unit, spn_SIRIUS_DDR_SW_DDR_TRAIN_INTERVAL,0)) != 0) {
    interval_us = env_interval;
  } else {
    /* use some default value here (tbd) */
    /* use zero to run continuously with mimimal delay */
    interval_us = 0;
  }

    /* Create Mutex lock */
    if (_mlock[unit] == NULL) {
        if ((_mlock[unit] = sal_mutex_create("soc_sirius_ddr23_training_lock")) == NULL) {
            return SOC_E_MEMORY;
        }
    }

    DDR_TRAINING_LOCK(unit);

  /*
   * DDR Training handler
   *
   * If handler is null, allocate handler
   * Else, stop training thread and reset fields
   */
  if (DDR_TRAINING_CONTROL(unit) == NULL) {
    DDR_TRAINING_CONTROL(unit) = sal_alloc(sizeof(_ddr23_training_control_t),
					   "ddr_training_control");
    if (DDR_TRAINING_CONTROL(unit) == NULL) {
      DDR_TRAINING_UNLOCK(unit);
      return SOC_E_MEMORY;
    }

    sal_memset(DDR_TRAINING_CONTROL(unit), 0, sizeof(_ddr23_training_control_t));

    DDR_TRAINING_THREAD(unit).thread_id = SAL_THREAD_ERROR;
    DDR_TRAINING_THREAD(unit).interval  = -1;
    DDR_TRAINING_THREAD(unit).trigger   = NULL;

  } else {
    /*
     * Stop training thread, if running. Do any cleanup here required before
     * re-starting thread.
     */
    rv = soc_sbx_sirius_sw_ddr23_train_stop(unit);
    if (SOC_FAILURE(rv)) {
      DDR_TRAINING_UNLOCK(unit);
      LOG_CLI((BSL_META_U(unit,
                          "Error stopping training thread\n")));
      return rv;
    }
  }  

  LOG_VERBOSE(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "Starting ddr training thread flags 0x%x, Interval %d\n"),flags,interval_us));

  /* start */
  rv = soc_sbx_sirius_sw_ddr23_train_start(unit,flags,interval_us);
  if (SOC_FAILURE(rv)) {
      DDR_TRAINING_UNLOCK(unit);
      LOG_CLI((BSL_META_U(unit,
                          "Error starting DDR training thread\n")));
      return rv;
  }
  LOG_VERBOSE(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "soc_sbx_sirius_sw_ddr23_train_init: unit=%d rv=%d\n"),
               unit, rv));

  DDR_TRAINING_UNLOCK(unit);
  return rv;
}

/*
 * Function:
 *     soc_sbx_sirius_sw_ddr23_train_detach
 * Purpose:
 *     Stop and deallocate SW training module.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 */

int
soc_sbx_sirius_sw_ddr23_train_detach(int unit)
{

    UNIT_INIT_CHECK(unit);

    SOC_IF_ERROR_RETURN(soc_sbx_sirius_sw_ddr23_train_stop(unit));

    DDR_TRAINING_LOCK(unit);

    /* Destroy semaphores */
    if ((DDR_TRAINING_THREAD(unit).trigger) != NULL) {
        sal_sem_destroy(DDR_TRAINING_THREAD(unit).trigger);
    }

    sal_free(DDR_TRAINING_CONTROL(unit));
    DDR_TRAINING_CONTROL(unit) = NULL;

    DDR_TRAINING_UNLOCK(unit);

    sal_mutex_destroy(_mlock[unit]);
    _mlock[unit] = NULL;

    return SOC_E_NONE;
}

/*
 * Function:
 *     _soc_sbx_sirius_sw_ddr23_dynamic_check
 * Purpose:
 *     Program dynamic vdl settings, test if they work.
 * Parameters:
 *     unit           - Device number
 *     ci             - CI interface being calibrated
 *     uOverrideData  - Dynamic data to use
 * Returns:
 *     nErrors - No of errors for this tap settings
 * Notes:
 *
 */

STATIC int
_soc_sbx_sirius_sw_ddr23_dynamic_check(int unit,
				 int ci,
    			         uint32 uOverrideData) 
{

  uint32 nErrors = 0;
  uint32 uAddr = 0x0;
  uint32 wr_status = BCM_E_NONE;
  uint32 rd_status = BCM_E_NONE;
  uint32 uDataWR[8] = {0};
  uint32 uDataRD[8] = {0};
  int i = 0;
  int bank = 0;

#ifndef __KERNEL__
  int rseed = 0x12345678;
  sal_srand(rseed);
#endif

  /*
   *  Use random data
   */

  for (i = 0; i < 8; i++ ) {
#ifndef __KERNEL__
    /* random data in user mode */
    uDataWR[i] = sal_rand() & 0xffffffff;
#else
    /* fixed pattern in kernel mode */
    uDataWR[i] = (0xf << i);
#endif
  }

  /*
   *  Write the data to each bank
   */  

  for (bank = 0; bank < 8; bank++ ) {

    uAddr = bank; 

    wr_status = soc_sbx_sirius_ddr23_write(unit, ci,uAddr,
					   uDataWR[0],uDataWR[1],uDataWR[2],
					   uDataWR[3],uDataWR[4],uDataWR[5],
					   uDataWR[6],uDataWR[7]);

    /*
     *  Read back and compare
     */  
  
    rd_status = soc_sbx_sirius_ddr23_read(unit,ci,uAddr,&uDataRD[0],
					  &uDataRD[1],&uDataRD[2],&uDataRD[3],
					  &uDataRD[4],&uDataRD[5],&uDataRD[6],
					  &uDataRD[7]);

    if (wr_status || 
	rd_status ||
	(uDataWR[0] != uDataRD[0]) ||
	(uDataWR[1] != uDataRD[1]) ||
	(uDataWR[2] != uDataRD[2]) ||
	(uDataWR[3] != uDataRD[3]) ||
	(uDataWR[4] != uDataRD[4]) ||
	(uDataWR[5] != uDataRD[5]) ||
	(uDataWR[6] != uDataRD[6]) ||
	(uDataWR[7] != uDataRD[7])) {

      nErrors++;
    }
  }

  return nErrors;

}

/*
 * Function:
 *     _soc_sbx_sirius_sw_ddr23_best_delay_tap
 * Purpose:
 *     Finds the optimal delay settings to use
 * Parameters:
 *     ovr_step_results - results from calibration
 * Returns:
 *     best_tap setting
 * Notes:
 * Return best delay tap, or the tap with least errors in the event
 * that all taps had errors. If there are identical longest_spans    
 * the first span will be used, this will result in minimal delay.
 * ex. ovr_step_results[0-4] = 0, and ovr_step_results[11-15] = 0. best_tap = 2
 */

STATIC int
_soc_sbx_sirius_sw_ddr23_best_delay_tap(uint32 ovr_step_results[] )
{

#define NO_ERRORS 0
  uint32 longest_span = 0;
  uint32 cur_span = 0;
  uint32 start = 0;
  uint32 end = 0;
  uint32 min_errors = 0;
  uint32 tap = 0;
  uint32 tap_least_errors = 0;
  uint32 best_tap = 0;

  min_errors = ovr_step_results[0];
  for (tap = 0; tap < 128; tap++ ) {
    if (ovr_step_results[tap] == NO_ERRORS) {
      cur_span++;
    } else {
      if (cur_span > longest_span) {
	longest_span = cur_span;
	end = tap - 1; 
      }
      cur_span = 0; /* reset */
    }

    /* find min, save tap where it occurs */
    if (ovr_step_results[tap] < min_errors) {
      tap_least_errors = tap;
      min_errors = ovr_step_results[tap];
    }
  }

  /* if longest working span was at 
   * the end account for it here
   */

  if (cur_span > longest_span) {
    longest_span = cur_span;
    end = 127;
  }

  if (longest_span == 0) {

    /* no delay settings worked, use the 
     * tap with least errors
     */
    LOG_CLI((BSL_META("No delay settings worked using tap=%d least errors(%d)\n"),
             tap_least_errors,ovr_step_results[tap_least_errors]));

    best_tap =  tap_least_errors;

  } else {   /* use midpoint of longest span */
    start = end - longest_span + 1;
    best_tap = (end-start)/2 + start;
  }
  
  return best_tap;
}

/*
 * Function:
 *     _soc_sbx_sirius_sw_ddr23_train
 * Purpose:
 *     Does the re-training (re-calibration) in SW
 * Parameters:
 *     unit - Device
 *     ci   - CI interface to train
 * Returns:
 *     
 * Notes:
 *  SW training steps
 *  1 - Iterate over each dynamic control register
 *
 *    Registers used in SW calibration
 *    DDR23_PHY_BYTE_LANE_VDL_OVERRIDE_4 (DQSP)
 *    DDR23_PHY_BYTE_LANE_VDL_OVERRIDE_5 (DQSN)
 *    DDR23_PHY_BYTE_LANE_VDL_OVERRIDE_6 (Read Enable)
 *    DDR23_PHY_BYTE_LANE_VDL_OVERRIDE_7 (Write Data)
 *    DDR23_PHY_ADDR_CTL_DYNAMIC_VDL_OVERRIDE
 *
 *   Notes from RSchumann:
 *   The ovr_step field sets the number of full steps, i.e. the delay = ovr_step * step_size, where step_size is approx. 60ps (slow case)
 *   The ovr_fine_fall field sets the number of quarter steps, 
 *   i.e. the fine_delay = ovr_fine_fall * quarter_step_size, where quarter_step_size is approx. 15ps (slow case)
 *
 *   Then total_delay = delay + fine_delay
 *     
 */

STATIC int
_soc_sbx_sirius_sw_ddr23_train(int unit, int ci)
{

    uint32 uOverrideData = 0;
  uint32 ovr_step = 0;
  uint32 ovr_fine_fall = 0;
  uint32 ovr_fine_rise = 0;
  uint32 best_tap = 0;
  uint32 tap = 0;
  uint32 ovr_step_results[128] = {0};
  uint32 nErrors = 0;
  uint32 uData = 0;

  /*
   *  Be sure following is set.
   */

  SOC_IF_ERROR_RETURN(READ_CI_CONFIG3r(unit,ci,&uData));
  soc_reg_field_set(unit,CI_CONFIG3r,&uData,PHY_UPD_VDL_ADDRf,1);
  soc_reg_field_set(unit,CI_CONFIG3r,&uData,PHY_UPD_VDL_BL1f,1);
  soc_reg_field_set(unit,CI_CONFIG3r,&uData,PHY_UPD_VDL_BL0f,1);
  SOC_IF_ERROR_RETURN(WRITE_CI_CONFIG3r(unit,ci,uData));

  for ( tap = 0; tap < 128; tap++ ) {

    ovr_step = tap >> 2;   /* 0 -> 31 */
    ovr_fine_fall = tap & 3;

    /*
     *  Uniform step sizes are achieved by keeping rise/fall values the same.
     */

    ovr_fine_rise = ovr_fine_fall;

    /*
     * Reserved values must be written with 0x0.
     */

    SOC_IF_ERROR_RETURN(READ_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_4r(unit,ci,&uOverrideData)); /* Read DQSP */
    DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_4,RESERVED0,0x0);
    DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_4,OVR_EN,0x1);
    DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_4,RESERVED1,0x0);
    DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_4,OVR_FINE_FALL,ovr_fine_fall);
    DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_4,RESERVED2,0x0);
    DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_4,OVR_FINE_RISE,ovr_fine_rise);
    DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_4,RESERVED3,0x0);
    DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_4,OVR_STEP,ovr_step);
    SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_4r(unit,ci,uOverrideData));

    /*
     *   All dynamic override registers have the same format
     */

    SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_5r(unit,ci,uOverrideData));  /* Read DQSN */
    SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_6r(unit,ci,uOverrideData));  /* Read enable */
    SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_7r(unit,ci,uOverrideData));  /* Write Data and mask VDL */

    SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE1_VDL_OVERRIDE_5r(unit,ci,uOverrideData));  
    SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE1_VDL_OVERRIDE_6r(unit,ci,uOverrideData));  
    SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE1_VDL_OVERRIDE_7r(unit,ci,uOverrideData));  
    SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_ADDR_CTL_DYNAMIC_VDL_OVERRIDEr(unit,ci,uOverrideData)); /* Address/Control */

    /*
     *  See if these dynamic values work
     */

    nErrors =  _soc_sbx_sirius_sw_ddr23_dynamic_check(unit,ci,uOverrideData);
    ovr_step_results[tap] = nErrors;

  } 

  /*
   *   Find the longest span of steps that worked(no errors), middle of the span 
   *   will result in optimal delay. If no delay settings worked use the one
   *   with the least number of errors. This like FE2k DDR training.
   */

  best_tap = _soc_sbx_sirius_sw_ddr23_best_delay_tap(ovr_step_results);
  ovr_step = best_tap >> 2;
  ovr_fine_fall = best_tap & 3;
  ovr_fine_rise = ovr_fine_fall;

  /*
   *  Use the best_delay tap settings
   */

  LOG_VERBOSE(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit:%d ci%d using ovr_step=%d, ovr_fine_fall=%d, ovr_fine_rise=%d\n"),
               FUNCTION_NAME(),
               unit,ci,ovr_step,
               ovr_fine_fall,
               ovr_fine_rise));

  SOC_IF_ERROR_RETURN(READ_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_0r(unit,ci,&uOverrideData));       /* Read DQSP */
  DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_0,RESERVED0,0x0);
  DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_0,OVR_EN,0x1);
  DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_0,RESERVED1,0x0);
  DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_0,OVR_FINE_FALL,ovr_fine_fall);
  DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_0,RESERVED2,0x0);
  DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_0,OVR_FINE_RISE,ovr_fine_rise);
  DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_0,RESERVED3,0x0);
  DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_0,OVR_STEP,ovr_step);
  SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_0r(unit,ci,uOverrideData));

  /* load the remaining dynamic data into the static registers */
  SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_1r(unit,ci,uOverrideData));      /* Read DQSN */
  SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_2r(unit,ci,uOverrideData));      /* Read enable */
  SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_3r(unit,ci,uOverrideData));      /* Write Data and mask VDL */
  SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_ADDR_CTL_STATIC_VDL_OVERRIDEr(unit,ci,uOverrideData));   /* Address/Control */  

  /* turn off using dynamic values */

  SOC_IF_ERROR_RETURN(READ_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_4r(unit,ci,&uData));
  DDR23_SET_FIELD(uData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_4,OVR_EN,0x0);
  SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_4r(unit,ci,uData));

  SOC_IF_ERROR_RETURN(READ_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_5r(unit,ci,&uData));
  DDR23_SET_FIELD(uData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_4,OVR_EN,0x0);
  SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_5r(unit,ci,uData));

  SOC_IF_ERROR_RETURN(READ_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_6r(unit,ci,&uData));
  DDR23_SET_FIELD(uData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_6,OVR_EN,0x0);
  SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_6r(unit,ci,uData));

  SOC_IF_ERROR_RETURN(READ_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_7r(unit,ci,&uData));
  DDR23_SET_FIELD(uData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_7,OVR_EN,0x0);
  SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_7r(unit,ci,uData));

  SOC_IF_ERROR_RETURN(READ_DDR23_PHY_ADDR_CTL_STATIC_VDL_OVERRIDEr(unit,ci,&uData));
  DDR23_SET_FIELD(uData,DDR23_PHY_ADDR_CTL_STATIC,VDL_OVERRIDE,OVR_EN,0x0);
  SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_ADDR_CTL_STATIC_VDL_OVERRIDEr(unit,ci,uData));

  return 0;
  
}

/*
 * Function:
 *     _soc_sbx_sirius_sw_ddr23_train_thread
 * Purpose:
 *     Master training thread
 * Parameters:
 *     unit_vp - Device unit number
 * Returns:
 *     Nothing, does not return.
 * Notes:
 *     
 */

STATIC void
_soc_sbx_ddr23_training_thread(void *unit_vp)
{
    int                   unit = PTR_TO_INT(unit_vp);
    int                   interval = 0;
    int ci = 0;
    _ddr23_training_thread_t  *train;

    train = &DDR_TRAINING_THREAD(unit);
    
    while ((interval = train->interval) != -1) {
      
        /*
         * Use a semaphore timeout instead of a sleep to achieve the
         * desired delay between scans.  This allows this thread to exit
         * immediately when soc_sbx_sirius_sw_ddr23_train_stop wants it to.
         */

      LOG_VERBOSE(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s  sleep %d\n"), FUNCTION_NAME(),interval));

        (void)sal_sem_take(train->trigger, interval + INTERVAL_TOLERANCE);

        if (train->interval == -1) {       /* Exit signaled */
            break;
        }

	/* SW Training */
	for (ci = 0; ci < SOC_SIRIUS_MAX_NUM_CI_BLKS; ci++ ) {
	  if (gCiTrainMask & (1 << ci)) {
	    _soc_sbx_sirius_sw_ddr23_train(unit,ci);
	  }
	}

	/* for debug only */
	/* train->interval = -1; */

    } /* while thread running */

    train->thread_id = SAL_THREAD_ERROR;
    train->interval = -1;

    sal_thread_exit(0);

}

int
soc_sbx_sirius_sw_ddr23_train_start(int unit,
			       uint32 flags,
			       int interval)
{

  int rv = SOC_E_NONE;
  _ddr23_training_thread_t *train;
  LOG_VERBOSE(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit=%d flags=0x%x interval=%d\n"),
               FUNCTION_NAME(),unit,flags,interval));

  UNIT_INIT_CHECK(unit);

  DDR_TRAINING_LOCK(unit);
  
  /* stop if already running */
  rv = soc_sbx_sirius_sw_ddr23_train_stop(unit);
  if (SOC_FAILURE(rv)) {
    DDR_TRAINING_UNLOCK(unit);
    return rv;
  }

  /* interval of (-1) just stops thread */
  if (interval == -1) {
    DDR_TRAINING_UNLOCK(unit);
    LOG_CLI((BSL_META_U(unit,
                        "train interval (-1) not starting\n")));
    return SOC_E_NONE;
  }

  train = &DDR_TRAINING_THREAD(unit);

  /* create semaphores */
  if (train->trigger == NULL) {
    train->trigger = sal_sem_create("ddr_training_trigger",
				     sal_sem_BINARY, 0);
  }

  if (train->trigger == NULL) {
    LOG_CLI((BSL_META_U(unit,
                        "%s: sem create failed\n"),FUNCTION_NAME()));
    DDR_TRAINING_UNLOCK(unit);
    return SOC_E_INTERNAL;
  }

  /* start ddr training thread */
  if (interval >= 0) {
    train->interval = interval;

    sal_snprintf(train->thread_name,sizeof(train->thread_name),
		 "DDRTRAIN.%d",unit);

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "start DDR Train Thread %s. Interval %d \n"),
                 train->thread_name, train->interval));    

    train->thread_id = sal_thread_create(train->thread_name,
					   SAL_THREAD_STKSZ,
					   soc_property_get
					   (unit,
					    spn_SIRIUS_SW_DDR_TRAIN_THREAD_PRI, 50),
					 _soc_sbx_ddr23_training_thread,
					   INT_TO_PTR(unit));
  }
  
  DDR_TRAINING_UNLOCK(unit);
  return rv;

}


/*
 * Function:
 *     soc_sbx_sirius_sw_ddr23_train_stop
 * Purpose:
 *     Terminate the ddr training
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Returns:
 *     SOC_E_XXX
 */

int
soc_sbx_sirius_sw_ddr23_train_stop(int unit)
{

  int rv = SOC_E_NONE;
  _ddr23_training_thread_t *train;
  sal_thread_t   sample_pid;
  sal_usecs_t    timeout = 0;
  soc_timeout_t  to;
  
  LOG_VERBOSE(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit=%d\n"),FUNCTION_NAME(),unit));
  DDR_TRAINING_LOCK(unit);
  train = &DDR_TRAINING_THREAD(unit);
  
  /* stop thread if running */
  if (train->interval != -1) {

    timeout = 500;

    train->interval = -1;
    sal_sem_give(train->trigger);

    soc_timeout_init(&to, timeout, 0);
    
    while ((sample_pid = train->thread_id) != SAL_THREAD_ERROR) {
      if (soc_timeout_check(&to)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: thread did not exit\n"),FUNCTION_NAME()));
	train->thread_id = SAL_THREAD_ERROR;
	rv = SOC_E_INTERNAL;
	break;
      }

      sal_usleep(10000);
    }    
  }

  LOG_VERBOSE(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit=%d rv=%d\n"),
               FUNCTION_NAME(),unit, rv));

  DDR_TRAINING_UNLOCK(unit); 
  return rv;
}

/* clear all external DDR memory */
int
soc_sbx_sirius_ddr23_clear(int unit)
{

  uint32 uData = 0;
  uint32 uDone = 0;
  uint32 uDoneMask = 0;
  int ci;
  soc_timeout_t to;
  int ddr_timeout_usec;
  uint32 uStatus = BCM_E_NONE;

  /* Run standard DDR test with 0x0 to clear mem */
  soc_reg_field_set(unit,CI_DDR_TESTr,&uData,RAM_TEST_FAILf,1); /* w1tc */
  soc_reg_field_set(unit,CI_DDR_TESTr,&uData,RAM_DONEf,1);      /* w1tc */
  soc_reg_field_set(unit,CI_DDR_TESTr,&uData,MODEf,0);          /* standard DDR test */

  for (ci = 0; ci < SOC_SIRIUS_MAX_NUM_CI_BLKS; ci++ ) {
    SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TESTr(unit,ci,uData));
    SOC_IF_ERROR_RETURN(WRITE_CI_DEBUGr(unit,ci,0x1));          /* ci to auto inject refresh */
    SOC_IF_ERROR_RETURN(WRITE_CI_DDR_STARTr(unit,ci,0x0));      /* start address = 0x0 */
    SOC_IF_ERROR_RETURN(WRITE_CI_DDR_BURSTr(unit,ci,0x40));     /* burst writes */
    SOC_IF_ERROR_RETURN(WRITE_CI_DDR_ITERr(unit,ci,0x1));       /* run test once */
    /* use data = 0x0 */
    SOC_IF_ERROR_RETURN(WRITE_CI_TEST_DATA0r(unit,ci,0x0));
    SOC_IF_ERROR_RETURN(WRITE_CI_TEST_DATA1r(unit,ci,0x0));
    SOC_IF_ERROR_RETURN(WRITE_CI_TEST_DATA2r(unit,ci,0x0));
    SOC_IF_ERROR_RETURN(WRITE_CI_TEST_DATA3r(unit,ci,0x0));
    SOC_IF_ERROR_RETURN(WRITE_CI_TEST_DATA4r(unit,ci,0x0));
    SOC_IF_ERROR_RETURN(WRITE_CI_TEST_DATA5r(unit,ci,0x0));
    SOC_IF_ERROR_RETURN(WRITE_CI_TEST_DATA6r(unit,ci,0x0));
    SOC_IF_ERROR_RETURN(WRITE_CI_TEST_DATA7r(unit,ci,0x0));
    SOC_IF_ERROR_RETURN(WRITE_CI_TEST_ALT_DATA0r(unit,ci,0x0));
    SOC_IF_ERROR_RETURN(WRITE_CI_TEST_ALT_DATA1r(unit,ci,0x0));
    SOC_IF_ERROR_RETURN(WRITE_CI_TEST_ALT_DATA2r(unit,ci,0x0));
    SOC_IF_ERROR_RETURN(WRITE_CI_TEST_ALT_DATA3r(unit,ci,0x0));
    SOC_IF_ERROR_RETURN(WRITE_CI_TEST_ALT_DATA4r(unit,ci,0x0));
    SOC_IF_ERROR_RETURN(WRITE_CI_TEST_ALT_DATA5r(unit,ci,0x0));
    SOC_IF_ERROR_RETURN(WRITE_CI_TEST_ALT_DATA6r(unit,ci,0x0));
    SOC_IF_ERROR_RETURN(WRITE_CI_TEST_ALT_DATA7r(unit,ci,0x0));

    /* start the test */
    uData = 0;
    SOC_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit,ci,&uData));
    soc_reg_field_set(unit,CI_DDR_TESTr,&uData,RAM_TESTf,1);
    SOC_IF_ERROR_RETURN(WRITE_CI_DDR_TESTr(unit,ci,uData));
			
  }

  /* wait for done */
  ddr_timeout_usec = 1000000;
  soc_timeout_init(&to,ddr_timeout_usec,0);
  while (!soc_timeout_check(&to)) {
    for (ci = 0; ci < SOC_SIRIUS_MAX_NUM_CI_BLKS; ci++ ) {
      SOC_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit,ci,&uData));
      uDone = soc_reg_field_get(unit,CI_DDR_TESTr,uData,RAM_DONEf);
      if (uDone) {
	uDoneMask |= (1<<ci);
	if (uDoneMask == 0x3ff) break;
      }
    }
  }

  if (uDoneMask != 0x3ff) {
    for(ci = 0; ci < SOC_SIRIUS_MAX_NUM_CI_BLKS; ci++) {
      if (!(uDoneMask & ( 1 << ci ))) {
	LOG_CLI((BSL_META_U(unit,
                            "CI%d did not finish mem clear\n"),ci));
	uStatus = -1;
      }
    }
  }

  /* check for errors */
  for (ci = 0; ci < SOC_SIRIUS_MAX_NUM_CI_BLKS; ci++ ) {
    if (uDoneMask & ( 1 << ci)) {
      SOC_IF_ERROR_RETURN(READ_CI_DDR_TESTr(unit,ci,&uData));
      if (soc_reg_field_get(unit,CI_DDR_TESTr,
			    uData,RAM_TEST_FAILf)) {
	LOG_CLI((BSL_META_U(unit,
                            "CI%d finished mem clear with errors\n"),ci));
	uStatus = -1;
      }
    }
  }

  return uStatus;
}





/* ddr23 r/w for bringup debug */
int
soc_sbx_sirius_ddr23_write(int unit, int ci, uint32 addr,
			   uint32 data0, uint32 data1, uint32 data2,
			   uint32 data3, uint32 data4, uint32 data5,
			   uint32 data6, uint32 data7)
{

  int rv = BCM_E_NONE;
  uint32 uCmd = 0;

  /* setup the data */
  SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA0r(unit,ci,data0));
  SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA1r(unit,ci,data1));
  SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA2r(unit,ci,data2));
  SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA3r(unit,ci,data3));
  SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA4r(unit,ci,data4));
  SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA5r(unit,ci,data5));
  SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA6r(unit,ci,data6));
  SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA7r(unit,ci,data7));

  /* insert 200us delay */
  sal_udelay(200);

  uCmd = 0;
  soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_ACKf,1);
  soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_REQf,1);
  soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_RD_WR_Nf,0);
  soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_USE_DYN_VDLf,0);
  soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_ADDRf,addr);

  SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_CTRLr(unit,ci,uCmd));

  if (soc_sbx_sirius_ddr23_done(unit,
				ci,
				SIRIUS_DDR23_TIMEOUT) != TRUE) {

    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "Error timeout writing to CI:%d addr:0x%x\n"),ci,addr));
    return SOC_E_TIMEOUT;
  }

  return rv;

}

int
soc_sbx_sirius_ddr23_read(int unit, int ci, uint32 addr,
			  uint32 *pData0, uint32 *pData1, uint32 *pData2,
			  uint32 *pData3, uint32 *pData4, uint32 *pData5,
			  uint32 *pData6, uint32 *pData7)
{

  uint32 uStatus = BCM_E_NONE;
  uint32 uCmd = 0;

  /* clear data registers */
  SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA0r(unit,ci,0x0));
  SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA1r(unit,ci,0x0));
  SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA2r(unit,ci,0x0));
  SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA3r(unit,ci,0x0));
  SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA4r(unit,ci,0x0));
  SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA5r(unit,ci,0x0));
  SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA6r(unit,ci,0x0));
  SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_DATA7r(unit,ci,0x0));

  soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_ACKf,1);
  soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_REQf,1);
  soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_RD_WR_Nf,1);
  soc_reg_field_set(unit,CI_MEM_ACC_CTRLr,&uCmd,MEM_ACC_ADDRf,addr);

  SOC_IF_ERROR_RETURN(WRITE_CI_MEM_ACC_CTRLr(unit,ci,uCmd));

  if (soc_sbx_sirius_ddr23_done(unit,
				ci,
				SIRIUS_DDR23_TIMEOUT) != TRUE) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "Error timeout reading from CI:%d addr:0x%x\n"),ci,addr));
    return SOC_E_TIMEOUT;
  }

  /* insert 200us delay */
  sal_udelay(200);

  SOC_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA0r(unit,ci,pData0));
  SOC_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA1r(unit,ci,pData1));
  SOC_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA2r(unit,ci,pData2));
  SOC_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA3r(unit,ci,pData3));
  SOC_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA4r(unit,ci,pData4));
  SOC_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA5r(unit,ci,pData5));
  SOC_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA6r(unit,ci,pData6));
  SOC_IF_ERROR_RETURN(READ_CI_MEM_ACC_DATA7r(unit,ci,pData7));

  return uStatus;

}

/*
 * Function:
 *     soc_sbx_sirius_ddr23_phy_tune_best_setting
 * Purpose:
 *     Determine optimal values from DDR Tuning results
 * Parameters:
 *      first_window           (first tread_en window)
 *      ci_tune_params_t *pCi  (ci tunning params to fill in for each ci)
 *      uint8 *p               (pointer to results array from running tuning)
 * Returns:
 * Notes:
 * 
 * Auto Tuning (DDRPhyTuneAuto) will run tunning test on a pair of tread_en windows
 * the best result is determined like below example:
 *
 * tread_en - amount of cycles to wait once sirius initiates a read to get data from phy
 * It has been measured that there is a gap of approx (78) between the two tread_en windows
 *
 *
 *                (first_window)
 *
 *                 tread_en=13              gap                 tread_en=14
 *                                |-------- (78) ----------|
 * Read En VDL   0  1  2  3  ... 63                         0 1 2 3 4 ...          63
 * Read VDL    0 -  -  -  - * * * * * * * * * * * * * * * * * * * * * * * - - - - - -
 * Read VDL    1 -  -  -  - - * * * * * * * * * * * * * * * * * * * * * * * - - - - -  
 * Read VDL    2 -  -  -  - - - * * * * * * * * * * * * * * * * * * * * * * * - - - -
 * Read VDL    3 -  -  -  - - - - * * * * * * * * * * * * * * * * * * * * * * * - - -
 * ...
 * Read VDL    63 - - - - - - - - * * * * * * * * * * * * * * * * * * * * * * * * * -
 *
 *
 * For each Read VDL, the longest span of passing results is determined (* == passing in above),
 * then the midpoint of span is calculated. The tread_en window that is closest to the midpoint
 * gets saved into ci_tune_params for each ci, along with the Read VDL that had the longest span.
 *
 * When results are passing at the end of one tread_en window, and the beginning of the next
 * (like above). The midpoint will be somewhere between the two windows, the tread_en window that
 * is closest to the midpoint is determined, and the Read En VDL setting use will either be at the far
 * end of the first window(Read En VDL 63) or at the beginning of the next window. (Read En VDL 0)
 *
 *
 * The DDRPhyTuneAuto command will then print optimal soc properties to use for each ci interface
 * 
 */

void
soc_sbx_sirius_ddr23_phy_tune_best_setting(int first_window, ci_tune_params_t *pCiTune, uint8 *p) {
  uint32 longest_span = 0;
  uint32 cur_span = 0;
  uint32 midpoint = 0;
  uint32 span_end_read_vdl_en = 0;
  uint32 best_read_vdl = 0;
  uint32 read_vdl;
  uint32 read_vdl_en;
  uint32 middle_to_first = 0;
  uint32 middle_to_second = 0;

  /* first window width + gap + second window width */
  int max_read_en_vdl_width = DDR23_READ_EN_VDL_WIDTH + TREAD_EN_GAP_WIDTH + DDR23_READ_EN_VDL_WIDTH;

  for (read_vdl = 0; read_vdl < DDR23_READ_EN_VDL_WIDTH; read_vdl++) {
    for (read_vdl_en = 0; read_vdl_en < max_read_en_vdl_width; read_vdl_en++) {
      if (*p++ == 1) { /* passing result */
	cur_span++;
	if (cur_span > longest_span) {
	  longest_span = cur_span;
	  span_end_read_vdl_en = read_vdl_en;
	  best_read_vdl = read_vdl;
	}
      } else {
	cur_span = 0;
      }
    }
  }

  /* if longest working span was at 
   * the end account for it here
   */

  if (cur_span > longest_span) {
    longest_span = cur_span;
    span_end_read_vdl_en = max_read_en_vdl_width;
  }

  /*  longest span of passing can only be as wide as both windows put together 
   *  plus center window, this is not likely to happen
   */

  if (longest_span > max_read_en_vdl_width ) longest_span=max_read_en_vdl_width;

  if (longest_span <= TREAD_EN_GAP_WIDTH ) {
    LOG_CLI((BSL_META("no passing results found from one tread_en window into the next\n")));
    pCiTune->valid = 0;
  } else {   
    pCiTune->valid       = 1;
    pCiTune->read_vdl    = best_read_vdl;

   /* use midpoint of longest span and determine which tread_en window to use that 
    * results in closest setting to midpoint 
    */

    midpoint = span_end_read_vdl_en - longest_span/2;
    middle_to_first = midpoint - (DDR23_READ_EN_VDL_WIDTH - 1);
    middle_to_second = (DDR23_READ_EN_VDL_WIDTH + TREAD_EN_GAP_WIDTH)  - midpoint;
    if (middle_to_first < middle_to_second) {
      /* midpoint is closer to first tread_en window */
      pCiTune->tread_en    = first_window;
      /* use end of first window */
      pCiTune->read_en_vdl = (DDR23_READ_EN_VDL_WIDTH-1); 
    } else {
      /* use beginning of second window */
      pCiTune->read_en_vdl = 0; 
      pCiTune->tread_en    = first_window+1;
    }
  }
}


int
soc_sbx_sirius_ddr23_vdl_set(int unit, int ci, uint block, uint vdl, 
			     uint tap, uint fine) {
  int rv = SOC_E_NONE;
  uint uOverrideData = 0;

  if ((tap > 63) || (fine > 3)) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: tap or fine out of range \n"),FUNCTION_NAME()));
    return SOC_E_PARAM;
  }

  switch (block) {
  case 0:
    /* addr/ctrl */
    DDR23_SET_FIELD(uOverrideData,DDR23_PHY_ADDR_CTL,STATIC_VDL_OVERRIDE,OVR_EN,        0x1);
    DDR23_SET_FIELD(uOverrideData,DDR23_PHY_ADDR_CTL,STATIC_VDL_OVERRIDE,OVR_FINE_FALL, fine);
    DDR23_SET_FIELD(uOverrideData,DDR23_PHY_ADDR_CTL,STATIC_VDL_OVERRIDE,OVR_FINE_RISE, fine);
    DDR23_SET_FIELD(uOverrideData,DDR23_PHY_ADDR_CTL,STATIC_VDL_OVERRIDE,OVR_STEP,      tap);
    SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_ADDR_CTL_STATIC_VDL_OVERRIDEr(unit,ci,uOverrideData));   
    break;
  case 1:
    /* byte lane 0 */
    switch (vdl) {
    case 0:
      /* override 0, read DQSP */
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_0,OVR_EN,0x1);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_0,OVR_FINE_FALL,fine);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_0,OVR_FINE_RISE,fine);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_0,OVR_STEP,tap);
      SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_0r(unit,ci,uOverrideData));
      break;
    case 1:
      /* override 1, read DQSN */
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_1,OVR_EN,0x1);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_1,OVR_FINE_FALL,fine);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_1,OVR_FINE_RISE,fine);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_1,OVR_STEP,tap);
      SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_1r(unit,ci,uOverrideData));
      break;
    case 2:
      /* override 2, read en */
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_2,OVR_EN,0x1);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_2,OVR_FINE_FALL,fine);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_2,OVR_FINE_RISE,fine);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_2,OVR_STEP,tap);
      SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_2r(unit,ci,uOverrideData));
      break;
    case 3:
      /* override 3, write VDL */
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_3,OVR_EN,0x1);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_3,OVR_FINE_FALL,fine);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_3,OVR_FINE_RISE,fine);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE0,VDL_OVERRIDE_3,OVR_STEP,tap);
      SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE0_VDL_OVERRIDE_3r(unit,ci,uOverrideData));
      break;
    default:
      LOG_ERROR(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s: unknown ddr phy vdl %d\n"),FUNCTION_NAME(), vdl));
      return SOC_E_PARAM;
    }
    break;
  case 2:
    /* byte lane 1 */
    switch (vdl) {
    case 0:
      /* override 0, read DQSP */
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE1,VDL_OVERRIDE_0,OVR_EN,0x1);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE1,VDL_OVERRIDE_0,OVR_FINE_FALL,fine);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE1,VDL_OVERRIDE_0,OVR_FINE_RISE,fine);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE1,VDL_OVERRIDE_0,OVR_STEP,tap);
      SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE1_VDL_OVERRIDE_0r(unit,ci,uOverrideData));
      break;
    case 1:
      /* override 1, read DQSN */
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE1,VDL_OVERRIDE_1,OVR_EN,0x1);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE1,VDL_OVERRIDE_1,OVR_FINE_FALL,fine);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE1,VDL_OVERRIDE_1,OVR_FINE_RISE,fine);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE1,VDL_OVERRIDE_1,OVR_STEP,tap);
      SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE1_VDL_OVERRIDE_1r(unit,ci,uOverrideData));
      break;
    case 2:
      /* override 2, read en */
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE1,VDL_OVERRIDE_2,OVR_EN,0x1);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE1,VDL_OVERRIDE_2,OVR_FINE_FALL,fine);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE1,VDL_OVERRIDE_2,OVR_FINE_RISE,fine);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE1,VDL_OVERRIDE_2,OVR_STEP,tap);
      SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE1_VDL_OVERRIDE_2r(unit,ci,uOverrideData));
      break;
    case 3:
      /* override 3, write VDL */
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE1,VDL_OVERRIDE_3,OVR_EN,0x1);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE1,VDL_OVERRIDE_3,OVR_FINE_FALL,fine);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE1,VDL_OVERRIDE_3,OVR_FINE_RISE,fine);
      DDR23_SET_FIELD(uOverrideData,DDR23_PHY_BYTE_LANE1,VDL_OVERRIDE_3,OVR_STEP,tap);
      SOC_IF_ERROR_RETURN(WRITE_DDR23_PHY_BYTE_LANE1_VDL_OVERRIDE_3r(unit,ci,uOverrideData));
      break;
    default:
      LOG_ERROR(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s: unknown ddr phy vdl %d\n"),FUNCTION_NAME(), vdl));
      return SOC_E_PARAM;
    }
    break;
  default:
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unknown ddr phy register block %d \n"),FUNCTION_NAME(), block));
    return SOC_E_PARAM;
  }

  return rv;
}

int soc_sbx_sirius_ddr23_done(int unit,
			      int ci,
			      uint32 uTimeout)
{
  uint32 i;
  uint32 data = 0;
  uint32 uAck = 0;

  /* wait for the ACK to indicate rd/wr op is finished */
  for(i = 0; i < uTimeout; i++ ) {
    SOC_IF_ERROR_RETURN(READ_CI_MEM_ACC_CTRLr(unit,ci,&data));  
    uAck = soc_reg_field_get(unit,CI_MEM_ACC_CTRLr,data,MEM_ACC_ACKf);
    if (uAck) {
      return TRUE;
    }
    sal_udelay(10);
  }
  
  /* timed out */
  return ( FALSE );
}
