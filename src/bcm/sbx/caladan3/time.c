/*
 * $Id: time.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Time - Broadcom Time BroadSync API (Caladan3).
 */

#include <shared/bsl.h>

#include <bcm/time.h>
#include <bcm/error.h>

#include <bcm_int/common/time.h>
#include <bcm_int/common/time-mbox.h>
#include <bcm_int/common/mbox.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw_dispatch.h>

/* Whole file only applies to Caladan3 */
#if defined(BCM_CALADAN3_SUPPORT)

#define BROAD_SYNC_TIME_CAPTURE_TIMEOUT      (10) /* useconds */
#define BROAD_SYNC_OUTPUT_TOGGLE_TIME_DELAY  (3)  /* seconds */

#define SYNT_TIME_SECONDS(unit, id) \
        TIME_INTERFACE_CONFIG(unit, id).time_capture.syntonous.seconds
#define SYNT_TIME_NANOSECONDS(unit, id) \
        TIME_INTERFACE_CONFIG(unit, id).time_capture.syntonous.nanoseconds

/****************************************************************************/
/*                      LOCAL VARIABLES DECLARATION                         */
/****************************************************************************/
static _bcm_time_config_p _bcm_time_config[BCM_MAX_NUM_UNITS] = {NULL};

#if 0
static bcm_time_spec_t _bcm_time_accuracy_arr[TIME_ACCURACY_CLK_MAX] = {
      {0, COMPILER_64_INIT(0,0),  25},        /* HW value = 32, accuracy up tp 25 nanosec */
      {0, COMPILER_64_INIT(0,0),  100},       /* HW value = 33, accuracy up to 100 nanosec */
      {0, COMPILER_64_INIT(0,0),  250},       /* HW value = 34, accuracy up to 250 nanosec */
      {0, COMPILER_64_INIT(0,0),  1000},      /* HW value = 35, accuracy up to 1 microsec */
      {0, COMPILER_64_INIT(0,0),  2500},      /* HW value = 36, accuracy up to 2.5 microsec */
      {0, COMPILER_64_INIT(0,0),  10000},     /* HW value = 37, accuracy up to 10 microsec */
      {0, COMPILER_64_INIT(0,0),  25000},     /* HW value = 38, accuracy up to 25 microsec */
      {0, COMPILER_64_INIT(0,0),  100000},    /* HW value = 39, accuracy up to 100 microsec */
      {0, COMPILER_64_INIT(0,0),  250000},    /* HW value = 40, accuracy up to 250 microsec */
      {0, COMPILER_64_INIT(0,0),  1000000},   /* HW value = 41, accuracy up to 1 milisec */
      {0, COMPILER_64_INIT(0,0),  2500000},   /* HW value = 42, accuracy up to 2.5 milisec */
      {0, COMPILER_64_INIT(0,0),  10000000},  /* HW value = 43, accuracy up to 10 milisec */
      {0, COMPILER_64_INIT(0,0),  25000000},  /* HW value = 44, accuracy up to 25 milisec */
      {0, COMPILER_64_INIT(0,0),  100000000}, /* HW value = 45, accuracy up to 100 milisec */
      {0, COMPILER_64_INIT(0,0),  250000000}, /* HW value = 46, accuracy up to 250 milisec */
      {0, COMPILER_64_INIT(0,1),  0},         /* HW value = 47, accuracy up to 1 sec */
      {0, COMPILER_64_INIT(0,10), 0},         /* HW value = 48, accuracy up to 10 sec */
      /* HW value = 49, accuracy greater than 10 sec */
      {0, COMPILER_64_INIT(0,TIME_ACCURACY_INFINITE), TIME_ACCURACY_INFINITE},
      /* HW value = 254 accuracy unknown */
      {0, COMPILER_64_INIT(0,TIME_ACCURACY_UNKNOWN), TIME_ACCURACY_UNKNOWN}
};
#endif


/****************************************************************************/
/*                      Internal functions implmentation                    */
/****************************************************************************/
int
_bcm_caladan3_time_capture_get (int unit, bcm_time_if_t id, bcm_time_capture_t *time);
int
_bcm_caladan3_time_interface_offset_get(int unit, bcm_time_if_t id, bcm_time_spec_t *offset);
STATIC int
_bcm_caladan3_time_bs_init(int unit, bcm_time_interface_t *intf);


/*
 * Function:
 *    _bcm_caladan3_time_hw_clear
 * Purpose:
 *    Internal routine used to clear all HW registers and table to default values
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    intf_id        - (IN) Time interface identifier
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_caladan3_time_hw_clear(int unit, bcm_time_if_t intf_id)
{
    return (BCM_E_NONE);
}

/*
 * Function:
 *    _bcm_caladan3_time_deinit
 * Purpose:
 *    Internal routine used to free time software module
 *    control structures.
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    time_cfg_pptr  - (IN) Pointer to pointer to time config structure.
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_caladan3_time_deinit(int unit, _bcm_time_config_p *time_cfg_pptr)
{
    int                 idx;
    _bcm_time_config_p  time_cfg_ptr;
    soc_control_t       *soc = SOC_CONTROL(unit);

    /* Sanity checks. */
    if (NULL == time_cfg_pptr) {
        return (BCM_E_PARAM);
    }

    time_cfg_ptr = *time_cfg_pptr;
    /* If time config was not allocated we are done. */
    if (NULL == time_cfg_ptr) {
        return (BCM_E_NONE);
    }

    /* Free time interface */
    if (NULL != time_cfg_ptr->intf_arr) {
        for (idx = 0; idx < NUM_TIME_INTERFACE(unit); idx++) {
            if (NULL !=  time_cfg_ptr->intf_arr[idx].user_cb) {
                sal_free(time_cfg_ptr->intf_arr[idx].user_cb);
            }
        }
        sal_free(time_cfg_ptr->intf_arr);
    }

    /* Destroy protection mutex. */
    if (NULL != time_cfg_ptr->mutex) {
        sal_mutex_destroy(time_cfg_ptr->mutex);
    }

    /* If any registered function - deregister */
    soc->time_call_ref_count = 0;
    soc->soc_time_callout = NULL;

    /* Free module configuration structue. */
    sal_free(time_cfg_ptr);
    *time_cfg_pptr = NULL;

    return (BCM_E_NONE);
}


#ifdef BCM_WARM_BOOT_SUPPORT
/*
 * Function:
 *    _bcm_caladan3_time_reinit
 * Purpose:
 *    Internal routine used to reinitialize time module based on HW settings
 *    during Warm boot
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    intf_id        - (IN) Time interface identifier
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_caladan3_time_reinit(int unit, bcm_time_if_t intf_id)
{
    return (BCM_E_NONE);
}
#endif /* BCM_WARM_BOOT_SUPPORT */


/*
 * Function:
 *    _bcm_caladan3_time_interface_id_validate
 * Purpose:
 *    Internal routine used to validate interface identifier
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    id             - (IN) Interface id to validate
 * Returns:
 *    BCM_E_XXX
 */
int
_bcm_caladan3_time_interface_id_validate(int unit, bcm_time_if_t id)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *    _bcm_caladan3_time_interface_input_validate
 * Purpose:
 *    Internal routine used to validate interface input
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    intf           - (IN) Interface to validate
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_caladan3_time_interface_input_validate(int unit, bcm_time_interface_t *intf)
{
    /* Sanity checks. */
    if (NULL == intf) {
        return (BCM_E_PARAM);
    }
    if (intf->flags & BCM_TIME_WITH_ID) {
        if (intf->id < 0 || intf->id > TIME_INTERFACE_ID_MAX(unit) ) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Bad time interface ID (%d)\n"), intf->id));
            return (BCM_E_PARAM);
        }
    }

    if (intf->flags & BCM_TIME_DRIFT) {
        if (intf->drift.nanoseconds > TIME_DRIFT_MAX) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Bad time interface drift (%d ppb)\n"), (int)intf->drift.nanoseconds));
            return BCM_E_PARAM;
        }
    }

    if (intf->flags & BCM_TIME_OFFSET) {
        if (intf->offset.nanoseconds > TIME_NANOSEC_MAX) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Bad time interface offset nanoseconds (%d)\n"), (int)intf->offset.nanoseconds));
            return BCM_E_PARAM;
        }
    }

    return (BCM_E_NONE);
}


/*
 * Function:
 *    _bcm_caladan3_time_interface_allocate_id
 * Purpose:
 *    Internal routine used to allocate time interface id
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    id             - (OUT) Interface id to be allocated
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_caladan3_time_interface_allocate_id(int unit, bcm_time_if_t *id)
{
    int                              idx;  /* Time interfaces iteration index.*/
    _bcm_time_interface_config_p     intf; /* Time interface description.     */

    /* Input parameters check. */
    if (NULL == id) {
        return (BCM_E_PARAM);
    }

    /* Find & allocate time interface. */
    for (idx = 0; idx < TIME_CONFIG(unit)->intf_count; idx++) {
        intf = TIME_CONFIG(unit)->intf_arr + idx;
        if (intf->ref_count) {  /* In use interface */
            continue;
        }
        intf->ref_count++;  /* If founf mark interface as in use */
        *id = intf->time_interface.id; /* Assign ID */
        return (BCM_E_NONE);
    }

    /* No available interfaces */
    return (BCM_E_FULL);
}


/*
 * Function:
 *    _bcm_caladan3_time_interface_heartbeat_install
 * Purpose:
 *    Internal routine used to install interface heartbeat rate into a HW
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    id             - (IN) Interface id to be installed into a HW
 * Returns:
 *    BCM_E_XXX
 */
int
_bcm_caladan3_time_interface_heartbeat_install(int unit, bcm_time_if_t id)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *    _bcm_caladan3_time_interface_accuracy_time2hw
 * Purpose:
 *    Internal routine used to compute HW accuracy value from interface
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    id             - (IN) Interface id
 *    accuracy       - (OUT) HW value to be programmed
 * Returns:
 *    BCM_E_XXX
 */
int
_bcm_caladan3_time_interface_accuracy_time2hw(int unit, bcm_time_if_t id,
                                         uint32 *accuracy)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *    _bcm_caladan3_time_interface_drift_install
 * Purpose:
 *    Internal routine used to install interface drift into a HW
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    id             - (IN) Interface id to be installed into a HW
 * Returns:
 *    BCM_E_XXX
 */
int
_bcm_caladan3_time_interface_drift_install(int unit, bcm_time_if_t id)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *    _bcm_caladan3_time_interface_offset_install
 * Purpose:
 *    Internal routine used to install interface offset into a HW
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    id             - (IN) Interface id to be installed into a HW
 * Returns:
 *    BCM_E_XXX
 */
int
_bcm_caladan3_time_interface_offset_install(int unit, bcm_time_if_t id)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *    _bcm_caladan3_time_interface_ref_clock_install
 * Purpose:
 *    Internal routine to install timesync clock divisor to
 *    enable broadsync reference clock.
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    id             - (IN) Interface id to be installed into a HW
 * Returns:
 *    BCM_E_XXX
 */
int
_bcm_caladan3_time_interface_ref_clock_install(int unit, bcm_time_if_t id)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *    _bcm_caladan3_time_interface_install
 * Purpose:
 *    Internal routine used to install interface settings into a HW
 * Parameters:
 *    unit           - (IN) BCM device number.
 *    intf_id        - (IN) Interface id to be installed into a HW
 * Returns:
 *    BCM_E_XXX
 */
int
_bcm_caladan3_time_interface_install(int unit, bcm_time_if_t intf_id)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_caladan3_time_accuracy_parse
 * Purpose:
 *      Internal routine to parse accuracy hw value into bcm_time_spec_t format
 * Parameters:
 *      unit        - (IN) StrataSwitch Unit #.
 *      accuracy    - (IN) Accuracy HW value
 *      time        - (OUT) bcm_time_spec_t structure to contain accuracy
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
_bcm_caladan3_time_accuracy_parse(int unit, uint32 accuracy, bcm_time_spec_t *time)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_caladan3_time_input_parse
 * Purpose:
 *      Internal routine to parse input time information stored in 3 registeres
 * Parameters:
 *      unit    - (IN) StrataSwitch Unit #.
 *      data0   - (IN) Data stored in register 0 conrain input time information
 *      data1   - (IN) Data stored in register 1 conrain input time information
 *      data2   - (IN) Data stored in register 2 conrain input time information
 *      time    - (OUT) Structure to contain input time information
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
_bcm_caladan3_time_input_parse(int unit, uint32 data0, uint32 data1, uint32 data2,
                          bcm_time_capture_t *time)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_caladan3_time_capture_counter_read
 * Purpose:
 *      Internal routine to read HW clocks
 * Parameters:
 *      unit    - (IN) StrataSwitch Unit #.
 *      id      - (IN) Time interface identifier
 *      time    - (OUT) Structure to contain HW clocks values
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
_bcm_caladan3_time_capture_counter_read(int unit, bcm_time_if_t id,
                                   bcm_time_capture_t *time)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_caladan3_time_capture_get
 * Purpose:
 *      Internal routine to read HW clocks
 * Parameters:
 *      unit    - (IN) StrataSwitch Unit #.
 *      id      - (IN) Time interface identifier
 *      time    - (OUT) Structure to contain HW clocks values
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
_bcm_caladan3_time_capture_get (int unit, bcm_time_if_t id, bcm_time_capture_t *time)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_caladan3_time_interface_offset_get
 * Purpose:
 *      Internal routine to read HW offset value and convert it into
 *      bcm_time_spec_t structure
 * Parameters:
 *      unit    -  (IN) StrataSwitch Unit #.
 *      id      -  (IN) Time interface identifier
 *      offset  - (OUT) Time interface  offset
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
_bcm_caladan3_time_interface_offset_get(int unit, bcm_time_if_t id,
                                   bcm_time_spec_t *offset)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_caladan3_time_interface_drift_get
 * Purpose:
 *      Internal routine to read HW drift value and convert it into
 *      bcm_time_spec_t structure
 * Parameters:
 *      unit    -  (IN) StrataSwitch Unit #.
 *      id      -  (IN) Time interface identifier
 *      drift   - (OUT) Time interface  drift
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
_bcm_caladan3_time_interface_drift_get(int unit, bcm_time_if_t id,
                                  bcm_time_spec_t *drift)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_caladan3_time_interface_accuracy_get
 * Purpose:
 *      Internal routine to read HW accuracy value and convert it into
 *      bcm_time_spec_t structure
 * Parameters:
 *      unit        -  (IN) StrataSwitch Unit #.
 *      id          -  (IN) Time interface identifier
 *      accuracy    - (OUT) Time interface  accuracy
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
_bcm_caladan3_time_interface_accuracy_get(int unit, bcm_time_if_t id,
                                     bcm_time_spec_t *accuracy)
{
    uint32 regval;

    READ_CMIC_BS_OUTPUT_TIME_0r(unit, &regval);
    soc_reg_field_get(unit, CMIC_BS_OUTPUT_TIME_0r, regval, ACCURACYf);

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_caladan3_time_interface_get
 * Purpose:
 *      Internal routine to get a time sync interface by id
 * Parameters:
 *      unit -  (IN) StrataSwitch Unit #.
 *      id   -  (IN) Time interface identifier
 *      intf - (IN/OUT) Time Sync Interface to get
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
_bcm_caladan3_time_interface_get(int unit, bcm_time_if_t id, bcm_time_interface_t *intf)
{
    uint32                  regval;
    bcm_time_interface_t    *intf_ptr;
    uint32                  orig_flags;

    intf_ptr = TIME_INTERFACE(unit, id);
    orig_flags = intf_ptr->flags;
    intf_ptr->flags = intf->flags;
    intf_ptr->id = id;
    READ_CMIC_BS_CONFIGr(unit, &regval);

    /* Update output flags */
    if (TIME_MODE_INPUT == soc_reg_field_get(unit, CMIC_BS_CONFIGr,
                                             regval, MODEf)) {
        intf_ptr->flags |= BCM_TIME_INPUT;
    } else {
        intf_ptr->flags &= ~BCM_TIME_INPUT;
    }

    if (soc_reg_field_get(unit, CMIC_BS_CONFIGr,
                          regval, BS_CLK_OUTPUT_ENABLEf)) {
        intf_ptr->flags |= BCM_TIME_ENABLE;
    } else {
        intf_ptr->flags &= ~BCM_TIME_ENABLE;
    }

    READ_CMIC_BS_OUTPUT_TIME_0r(unit, &regval);
    if (soc_reg_field_get(unit, CMIC_BS_OUTPUT_TIME_0r,
                          regval, LOCKf)) {
        intf_ptr->flags |= BCM_TIME_LOCKED;
    } else {
        intf_ptr->flags &= ~BCM_TIME_LOCKED;
    }

    if (intf->flags & BCM_TIME_ACCURACY) {
        BCM_IF_ERROR_RETURN(
            _bcm_caladan3_time_interface_accuracy_get(unit, id,
                                                 &(intf_ptr->accuracy)));
    }

    if (intf->flags & BCM_TIME_OFFSET) {
        if (orig_flags & BCM_TIME_SYNC_STAMPER) {
            
        } else {
            /* Get value from hardware */
            BCM_IF_ERROR_RETURN(
                _bcm_caladan3_time_interface_offset_get(unit, id, &(intf_ptr->offset)));
        }
    }

    if (intf->flags & BCM_TIME_DRIFT) {
        if (orig_flags & BCM_TIME_SYNC_STAMPER) {
            
        } else {
            /* Get value from hardware */
            BCM_IF_ERROR_RETURN(
                _bcm_caladan3_time_interface_drift_get(unit, id, &(intf_ptr->drift)));
        }
    }

    if (intf->flags & BCM_TIME_REF_CLOCK) {
        /* No reporting of ref clock */
    }

    if (orig_flags & BCM_TIME_SYNC_STAMPER) {
        intf_ptr->flags = orig_flags;
    }

    *intf = *(TIME_INTERFACE(unit, id));

    return (BCM_E_NONE);
}


/*
 * Function:
 *      _bcm_caladan3_time_hw_interrupt_dflt
 * Purpose:
 *      Default handler for broadsync heartbeat interrupt
 * Parameters:
 *      unit - StrataSwitch unit #.
 * Returns:
 *      Nothing
 */
void
_bcm_caladan3_time_hw_interrupt_dflt(int unit)
{
}


/*
 * Function:
 *      _bcm_caladan3_time_hw_interrupt
 * Purpose:
 *      Handles broadsync heartbeat interrupt
 * Parameters:
 *      unit - StrataSwitch unit #.
 * Returns:
 *      Nothing
 */
void
_bcm_caladan3_time_hw_interrupt(int unit)
{
}


/****************************************************************************/
/*                      API functions implmentation                         */
/****************************************************************************/


/*
 * Function:
 *      bcm_caladan3_time_init
 * Purpose:
 *      Initialize time module
 * Parameters:
 *      unit - (IN) StrataSwitch Unit #.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
bcm_caladan3_time_init (int unit)
{
    _bcm_time_config_p      time_cfg_ptr;   /* Pointer to Time module config */
    bcm_time_interface_t    *intf;          /* Time interfaces iterator.     */
    int                     alloc_sz;       /* Memory allocation size.       */
    int                     idx;            /* Time interface array iterator */
    int                     rv;             /* Return Value                  */
    soc_control_t *soc = SOC_CONTROL(unit); /* Soc control structure         */

    /* Check if time feature is supported on a device */
    if (!soc_feature(unit, soc_feature_time_support)) {
        return (BCM_E_UNAVAIL);
    }

    /* If already initialized then deinitialize time module */
    if (TIME_INIT(unit)) {
        _bcm_caladan3_time_deinit(unit, &TIME_CONFIG(unit));
    }

    /* Allocate time config structure. */
    alloc_sz = sizeof(_bcm_time_config_t);
    time_cfg_ptr = sal_alloc(alloc_sz, "Time module");
    if (NULL == time_cfg_ptr) {
        return (BCM_E_MEMORY);
    }
    sal_memset(time_cfg_ptr, 0, alloc_sz);

    /* Currently only one interface per unit */
    time_cfg_ptr->intf_count = NUM_TIME_INTERFACE(unit);

    /* Allocate memory for all time interfaces, supported */
    alloc_sz = time_cfg_ptr->intf_count * sizeof(_bcm_time_interface_config_t);
    time_cfg_ptr->intf_arr = sal_alloc(alloc_sz, "Time Interfaces");
    if (NULL == time_cfg_ptr->intf_arr) {
        _bcm_caladan3_time_deinit(unit, &time_cfg_ptr);
        return (BCM_E_MEMORY);
    }
    sal_memset(time_cfg_ptr->intf_arr, 0, alloc_sz);
    for (idx = 0; idx < time_cfg_ptr->intf_count; idx++) {
        intf = &time_cfg_ptr->intf_arr[idx].time_interface;
        intf->id = idx;
    }

    /* For each time interface allocate memory for tuser_cb */
    alloc_sz = sizeof(_bcm_time_user_cb_t);
    for (idx = 0; idx < NUM_TIME_INTERFACE(unit); idx++) {
        time_cfg_ptr->intf_arr[idx].user_cb =
            sal_alloc(alloc_sz, "Time Interface User Callback");
        if (NULL == time_cfg_ptr->intf_arr[idx].user_cb) {
            _bcm_caladan3_time_deinit(unit,  &time_cfg_ptr);
            return (BCM_E_MEMORY);
        }
        sal_memset(time_cfg_ptr->intf_arr[idx].user_cb, 0, alloc_sz);
    }

    /* Interrupt handling function initialization */
    soc->time_call_ref_count = 0;
    soc->soc_time_callout = NULL;

    /* Create protection mutex. */
    time_cfg_ptr->mutex = sal_mutex_create("Time mutex");
    if (NULL == time_cfg_ptr->mutex) {
        _bcm_caladan3_time_deinit(unit, &time_cfg_ptr);
        return (BCM_E_MEMORY);
    }

    sal_mutex_take(time_cfg_ptr->mutex, sal_mutex_FOREVER);

    TIME_CONFIG(unit) = time_cfg_ptr;

    /* Clear memories/registers on Cold boot only */
    if (!SOC_WARM_BOOT(unit)) {
        for (idx = 0; idx < NUM_TIME_INTERFACE(unit); idx++) {
            rv  = _bcm_caladan3_time_hw_clear(unit, idx);
            if (BCM_FAILURE(rv)) {
                TIME_UNLOCK(unit);
                _bcm_caladan3_time_deinit(unit, &time_cfg_ptr);
                TIME_CONFIG(unit) = NULL;
                return (BCM_E_MEMORY);
            }
        }
    } else {
        /* If Warm boot reinitialize module based on HW state */
#ifdef BCM_WARM_BOOT_SUPPORT
        for (idx = 0; idx < NUM_TIME_INTERFACE(unit); idx++) {
            rv = _bcm_caladan3_time_reinit(unit, idx);
            if (BCM_FAILURE(rv)) {
                TIME_UNLOCK(unit);
                _bcm_caladan3_time_deinit(unit, &time_cfg_ptr);
                TIME_CONFIG(unit) = NULL;
                return (rv);
            }
        }

#endif /* BCM_WARM_BOOT_SUPPORT */
    }

    TIME_UNLOCK(unit);

    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_caladan3_time_deinit
 * Purpose:
 *      Uninitialize time module
 * Parameters:
 *      unit - (IN) StrataSwitch Unit #.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
bcm_caladan3_time_deinit (int unit)
{
    /* Chek if time feature is supported on a device */
    if (!soc_feature(unit, soc_feature_time_support)) {
        return (BCM_E_UNAVAIL);
    }

    if (0 == TIME_INIT(unit)) {
        return (BCM_E_INIT);
    }

    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_caladan3_time_interface_add
 * Purpose:
 *      Adding a time sync interface to a specified unit
 * Parameters:
 *      unit - (IN) StrataSwitch Unit #.
 *      intf - (IN/OUT) Time Sync Interface
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
bcm_caladan3_time_interface_add (int unit, bcm_time_interface_t *intf)
{
    int             rv;     /* Return value */
    int replacing = 0;

    /* Chek if time feature is supported on a device */
    if (!soc_feature(unit, soc_feature_time_support)) {
        return (BCM_E_UNAVAIL);
    }

    BCM_IF_ERROR_RETURN(
        _bcm_caladan3_time_interface_input_validate(unit, intf));

    if (TIME_CONFIG(unit) == NULL) {
        return (BCM_E_INIT);
    }

    TIME_LOCK(unit);
    if (intf->flags & BCM_TIME_WITH_ID) {
        /* If interface already been in use */
        if (0 != TIME_INTERFACE_CONFIG_REF_COUNT(unit, intf->id)) {
            if (0 == (intf->flags & BCM_TIME_REPLACE)) {
                TIME_UNLOCK(unit);
                return BCM_E_EXISTS;
            }
            replacing = 1;
        } else {
            TIME_INTERFACE_CONFIG_REF_COUNT(unit, intf->id) = 1;
        }
    } else {
        rv = _bcm_caladan3_time_interface_allocate_id(unit, &(intf->id));
        if (BCM_FAILURE(rv)) {
            TIME_UNLOCK(unit);
            return rv;
        }
    }

    if (intf->flags & BCM_TIME_SYNC_STAMPER) {
        int reset_hardware = 0;

        /* Set time interface configuration. */
        if (replacing) {
            if (((TIME_INTERFACE(unit, intf->id))->flags &
                 (BCM_TIME_ENABLE | BCM_TIME_INPUT)) !=
                (intf->flags & (BCM_TIME_ENABLE | BCM_TIME_INPUT))) {
                /* either master or enabled state changed, so copy those bits in flags */
                (TIME_INTERFACE(unit, intf->id))->flags &= ~(BCM_TIME_ENABLE | BCM_TIME_INPUT);
                (TIME_INTERFACE(unit, intf->id))->flags |=
                    intf->flags & (BCM_TIME_ENABLE | BCM_TIME_INPUT);
                /* either master or enabled state changed, so re-init */
                reset_hardware = 1;
            }
            if (intf->flags & BCM_TIME_DRIFT) {
                (TIME_INTERFACE(unit, intf->id))->drift = intf->drift;
            }
            if (intf->flags & BCM_TIME_OFFSET) {
                (TIME_INTERFACE(unit, intf->id))->offset = intf->offset;
            }
            if (intf->flags & BCM_TIME_ACCURACY) {
                (TIME_INTERFACE(unit, intf->id))->accuracy = intf->accuracy;
            }
            if (intf->flags & BCM_TIME_HEARTBEAT) {
                if ((TIME_INTERFACE(unit, intf->id))->heartbeat_hz != intf->heartbeat_hz) {
                    (TIME_INTERFACE(unit, intf->id))->heartbeat_hz = intf->heartbeat_hz;
                    /* Also use heartbeat flag to indicate whether to replace bitclock freq */
                    (TIME_INTERFACE(unit, intf->id))->bitclock_hz = intf->bitclock_hz;
                    reset_hardware = 1;
                }
            }
        } else {
            /* Is new, so just copy wholesale */
            *(TIME_INTERFACE(unit, intf->id)) = *intf;
            reset_hardware = 1;
        }

        if (reset_hardware) {
            /* Enable/Setup TIME API BroadSync on the unit */
            rv = _bcm_caladan3_time_bs_init(unit, TIME_INTERFACE(unit, intf->id));
            if (BCM_FAILURE(rv)) {
                TIME_UNLOCK(unit);
                return rv;
            }
        }

        if (!replacing) {
            /* Enable BS firmware on the unit */
            rv = _bcm_mbox_comm_init(unit, MOS_MSG_MBOX_APPL_BS);
            if (BCM_FAILURE(rv)) {
                LOG_CLI((BSL_META_U(unit,
                                    "mbox_comm_init failure\n")));
                TIME_UNLOCK(unit);
                return rv;
            }
        }

        /* Set specified offsets */
        if (intf->flags & BCM_TIME_DRIFT) {
            rv = _bcm_time_bs_frequency_offset_set(unit, intf->drift);
            if (BCM_FAILURE(rv)) {
                TIME_UNLOCK(unit);
                return rv;
            }
        }

        if (intf->flags & BCM_TIME_OFFSET) {
            rv = _bcm_time_bs_phase_offset_set(unit, intf->offset);
            if (BCM_FAILURE(rv)) {
                TIME_UNLOCK(unit);
                return rv;
            }
        }
    }

    TIME_UNLOCK(unit);

    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_caladan3_time_interface_delete
 * Purpose:
 *      Deleting a time sync interface from unit
 * Parameters:
 *      unit    - (IN) StrataSwitch Unit #.
 *      intf_id - (IN) Time Sync Interface id to remove
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
bcm_caladan3_time_interface_delete (int unit, bcm_time_if_t intf_id)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_caladan3_time_interface_get
 * Purpose:
 *      Get a time sync interface on a specified unit
 * Parameters:
 *      unit -  (IN) StrataSwitch Unit #.
 *      intf - (IN/OUT) Time Sync Interface to get
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
bcm_caladan3_time_interface_get (int unit, bcm_time_interface_t *intf)
{
    int rv;

    /* Chek if time feature is supported on a device */
    if (!soc_feature(unit, soc_feature_time_support)) {
        return (BCM_E_UNAVAIL);
    }

    /* Validation checks */
    if (NULL == intf) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(
        _bcm_caladan3_time_interface_id_validate(unit, intf->id));

    TIME_LOCK(unit);
    rv = _bcm_caladan3_time_interface_get(unit, intf->id, intf);
    TIME_UNLOCK(unit);

    return (rv);
}


/*
 * Function:
 *      bcm_caladan3_time_interface_delete_all
 * Purpose:
 *      Deleting all time sync interfaces on a unit
 * Parameters:
 *      unit    - (IN) StrataSwitch Unit #.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
bcm_caladan3_time_interface_delete_all (int unit)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_caladan3_time_interface_traverse
 * Purpose:
 *      Itterates over all time sync interfaces and calls given callback
 * Parameters:
 *      unit        - (IN) StrataSwitch Unit #.
 *      cb          - (IN) Call back function
 *      user_data   - (IN) void pointer to store any user information
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
bcm_caladan3_time_interface_traverse (int unit, bcm_time_interface_traverse_cb cb,
                                 void *user_data)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_caladan3_time_capture_get
 * Purpose:
 *      Gets a time captured by HW clock
 * Parameters:
 *      unit    - (IN) StrataSwitch Unit #.
 *      id      - (IN) Time interface identifier
 *      time    - (OUT) Structure to contain HW clocks values
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
bcm_caladan3_time_capture_get (int unit, bcm_time_if_t id, bcm_time_capture_t *time)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_caladan3_time_heartbeat_enable_set
 * Purpose:
 *      Enables/Disables interrupt handling for each heartbeat provided by a
 *      HW clock
 * Parameters:
 *      unit    - (IN) StrataSwitch Unit #.
 *      id      - (IN) Time Sync Interface Id
 *      enable  - (IN) Enable/Disable parameter
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
bcm_caladan3_time_heartbeat_enable_set (int unit, bcm_time_if_t id, int enable)
{
    /* Chek if time feature is supported on a device */
    if (!soc_feature(unit, soc_feature_time_support)) {
        return (BCM_E_UNAVAIL);
    }

    /* Param validation check. */
    BCM_IF_ERROR_RETURN(
        _bcm_caladan3_time_interface_id_validate(unit, id));

    TIME_LOCK(unit);

    if ((TIME_INTERFACE(unit, id))->flags & BCM_TIME_SYNC_STAMPER) {
        int rv = _bcm_time_bs_debug_1pps_set(unit, enable);
        if (BCM_FAILURE(rv)) {
            TIME_UNLOCK(unit);
            return rv;
        }

        TIME_UNLOCK(unit);
        return BCM_E_NONE;
    }

    TIME_UNLOCK(unit);

    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_caladan3_time_heartbeat_enable_get
 * Purpose:
 *      Gets interrupt handling status for each heartbeat provided by a
 *      HW clock
 * Parameters:
 *      unit    - (IN) StrataSwitch Unit #.
 *      id      - (IN) Time Sync Interface Id
 *      enable  - (OUT) Enable status of interrupt handling
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
bcm_caladan3_time_heartbeat_enable_get (int unit, bcm_time_if_t id, int *enable)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_caladan3_time_trigger_enable_set
 * Purpose:
 *      Enables/Disables interrupt handling for external triggers
 * Parameters:
 *      unit        - (IN) StrataSwitch Unit #.
 *      id          - (IN) Time Sync Interface Id
 *      mode_flags  - (IN) Enable/Disable parameter
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
bcm_caladan3_time_trigger_enable_set (int unit, bcm_time_if_t id, uint32 mode_flags)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_caladan3_time_trigger_enable_get
 * Purpose:
 *      Gets interrupt handling status for each heartbeat provided by a
 *      HW clock
 * Parameters:
 *      unit    - (IN) StrataSwitch Unit #.
 *      id      - (IN) Time Sync Interface Id
 *      enable  - (OUT) Enable status of interrupt handling
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
bcm_caladan3_time_trigger_enable_get (int unit, bcm_time_if_t id, uint32 *mode_flags)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_caladan3_time_heartbeat_register
 * Purpose:
 *      Registers a call back function to be called on each heartbeat
 * Parameters:
 *      unit        - (IN) StrataSwitch Unit #.
 *      id          - (IN) Time Sync Interface Id
 *      f           - (IN) Function to register
 *      user_data   - (IN) void pointer to store any user information
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
bcm_caladan3_time_heartbeat_register (int unit, bcm_time_if_t id, bcm_time_heartbeat_cb f,
                                 void *user_data)
{
    return (BCM_E_NONE);
}


/*
 * Function:
 *      bcm_caladan3_time_heartbeat_unregister
 * Purpose:
 *      Unregisters a call back function to be called on each heartbeat
 * Parameters:
 *      unit        - (IN) StrataSwitch Unit #.
 *      id          - (IN) Time Sync Interface Id
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 *
 */
int
bcm_caladan3_time_heartbeat_unregister (int unit, bcm_time_if_t id)
{
    return (BCM_E_NONE);
}

STATIC
int
_bcm_caladan3_bspll_init(int unit)
{
    uint32 regval;
    int count = 0;

    /* coverity[check_return] */
    READ_CX_BS_PLL_RESETr(unit, &regval);
    soc_reg_field_set(unit, CX_BS_PLL_RESETr, &regval, RESET_Nf, 0);
    soc_reg_field_set(unit, CX_BS_PLL_RESETr, &regval, POST_RESET_Nf, 0);
    /* coverity[check_return] */
    WRITE_CX_BS_PLL_RESETr(unit, regval);

    /* m CX_BS_PLL_GAIN KP=6 KI=2 KA=2 */
    /* coverity[check_return] */
    READ_CX_BS_PLL_GAINr(unit, &regval);
    soc_reg_field_set(unit, CX_BS_PLL_GAINr, &regval, KPf, 6);
    soc_reg_field_set(unit, CX_BS_PLL_GAINr, &regval, KIf, 2);
    soc_reg_field_set(unit, CX_BS_PLL_GAINr, &regval, KAf, 2);
    /* coverity[check_return] */
    WRITE_CX_BS_PLL_GAINr(unit, regval);

    /* coverity[check_return] */
    READ_CX_BS_PLL_NDIV_INTEGERr(unit, &regval);
    soc_reg_field_set(unit, CX_BS_PLL_NDIV_INTEGERr, &regval, NDIV_INTf, 99);
    /* coverity[check_return] */
    WRITE_CX_BS_PLL_NDIV_INTEGERr(unit, regval);

    /* coverity[check_return] */
    READ_CX_BS_PLL_NDIV_FRACTIONr(unit, &regval);
    soc_reg_field_set(unit, CX_BS_PLL_NDIV_FRACTIONr, &regval, NDIV_FRACf, 629146);
    /* coverity[check_return] */
    WRITE_CX_BS_PLL_NDIV_FRACTIONr(unit, regval);

    /* coverity[check_return] */
    READ_CX_BS_PLL_PREDIVr(unit, &regval);
    soc_reg_field_set(unit, CX_BS_PLL_PREDIVr, &regval, PDIVf, 1);
    /* coverity[check_return] */
    WRITE_CX_BS_PLL_PREDIVr(unit, regval);

    /* coverity[check_return] */
    READ_CX_BS_PLL_CHANNEL_0r(unit, &regval);
    soc_reg_field_set(unit, CX_BS_PLL_CHANNEL_0r, &regval, MDIVf, 249);
    /* coverity[check_return] */
    WRITE_CX_BS_PLL_CHANNEL_0r(unit, regval);

    /* m CX_BS_PLL_CONTROL REFCLK_SEL=1 (default) */

    /* m CX_BS_PLL_RESET RESET_N=1 POST_RESET_N=0 */
    /* coverity[check_return] */
    READ_CX_BS_PLL_RESETr(unit, &regval);
    soc_reg_field_set(unit, CX_BS_PLL_RESETr, &regval, RESET_Nf, 1);
    soc_reg_field_set(unit, CX_BS_PLL_RESETr, &regval, POST_RESET_Nf, 0);
    /* coverity[check_return] */
    WRITE_CX_BS_PLL_RESETr(unit, regval);

    /* # sleep 1 */
    /* # waiting until "g CX_BS_PLL_STATUS" */
    /* #     returns "CX_BS_PLL_STATUS.cx0[0x2001200]=0x1102: <STATUS=0x102,LOCK=1>" */
    do {
        int status, lock;
        /* coverity[check_return] */
        READ_CX_BS_PLL_STATUSr(unit, &regval);
        status = soc_reg_field_get(unit, CX_BS_PLL_STATUSr, regval, STATUSf);
        lock = soc_reg_field_get(unit, CX_BS_PLL_STATUSr, regval, LOCKf);

        if ( (status == 0x102) && (lock == 1) ) {
            break;
        }

        sal_usleep(100);
        count++;
    } while (count < 2000);

    if (count == 2000) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "BSPLL failed to lock: CX_BS_PLL_STATUS: 0x%08x\n"), regval));
        return (BCM_E_INTERNAL);
    }

    /* m CX_BS_PLL_RESET RESET_N=1 POST_RESET_N=1 */
    /* coverity[check_return] */
    READ_CX_BS_PLL_RESETr(unit, &regval);
    soc_reg_field_set(unit, CX_BS_PLL_RESETr, &regval, RESET_Nf, 1);
    soc_reg_field_set(unit, CX_BS_PLL_RESETr, &regval, POST_RESET_Nf, 1);
    /* coverity[check_return] */
    WRITE_CX_BS_PLL_RESETr(unit, regval);

    return BCM_E_NONE;
}

/********************************* Caladan3 - BroadSync Time Support **********************************/

STATIC
int
_bcm_caladan3_time_bs_init(int unit, bcm_time_interface_t *intf)
{
    const int bspll_freq = 20000000; /* 20 MHz */
    uint32 regval = 0;

    int master = ((intf->flags & BCM_TIME_INPUT) == 0);
    int output_enable = master && ((intf->flags & BCM_TIME_ENABLE) != 0);

    int bitclk_divisor = bspll_freq / intf->bitclock_hz;
    int bitclk_high = bitclk_divisor / 2;
    int bitclk_low = bitclk_divisor - bitclk_high;
    int hb_divisor = intf->bitclock_hz / intf->heartbeat_hz;
    int hb_up = (hb_divisor > 200) ? 100 : (hb_divisor / 2);
    int hb_down = hb_divisor - hb_up;

    uint32 freq_ctrl_lower = 0x40000000;  /* 4ns increment, for 250MHz TSPLL output */

    if (master) {
        /* Only support 10MHz operation currently */
        if (intf->bitclock_hz != 10000000) {
            return BCM_E_PARAM;
        }

        BCM_IF_ERROR_RETURN(_bcm_caladan3_bspll_init(unit));
    }

    /* m CMIC_BS_CLK_CTRL LOW_DURATION=1 HIGH_DURATION=1 */
    /* coverity[check_return] */
    READ_CMIC_BS_CLK_CTRLr(unit, &regval);
    soc_reg_field_set(unit, CMIC_BS_CLK_CTRLr, &regval, LOW_DURATIONf, bitclk_low);
    soc_reg_field_set(unit, CMIC_BS_CLK_CTRLr, &regval, HIGH_DURATIONf, bitclk_high);
    /* coverity[check_return] */
    WRITE_CMIC_BS_CLK_CTRLr(unit, regval);

    /* m CMIC_BS_CLK_CTRL ENABLE=1 */
    /* coverity[check_return] */
    READ_CMIC_BS_CLK_CTRLr(unit, &regval);
    soc_reg_field_set(unit, CMIC_BS_CLK_CTRLr, &regval, ENABLEf, 1);
    /* coverity[check_return] */
    WRITE_CMIC_BS_CLK_CTRLr(unit, regval);

    /* m CMIC_BS_CONFIG BS_CLK_OUTPUT_ENABLE=1 BS_HB_OUTPUT_ENABLE=1 BS_TC_OUTPUT_ENABLE=1 */
    /* coverity[check_return] */
    READ_CMIC_BS_CONFIGr(unit, &regval);
    soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, BS_CLK_OUTPUT_ENABLEf, output_enable);
    soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, BS_HB_OUTPUT_ENABLEf, output_enable);
    soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, BS_TC_OUTPUT_ENABLEf, output_enable);
    /* coverity[check_return] */
    WRITE_CMIC_BS_CONFIGr(unit, regval);

    /* m CMIC_TS_FREQ_CTRL_LOWER FRAC=0x40000000 */
    /* coverity[check_return] */
    READ_CMIC_TS_FREQ_CTRL_LOWERr(unit, &regval);
    soc_reg_field_set(unit, CMIC_TS_FREQ_CTRL_LOWERr, &regval, FRACf, freq_ctrl_lower);
    /* coverity[check_return] */
    WRITE_CMIC_TS_FREQ_CTRL_LOWERr(unit, regval);

    /* m CMIC_TS_FREQ_CTRL_UPPER ENABLE=1 */
    /* coverity[check_return] */
    READ_CMIC_TS_FREQ_CTRL_UPPERr(unit, &regval);
    soc_reg_field_set(unit, CMIC_TS_FREQ_CTRL_UPPERr, &regval, ENABLEf, 1);
    /* coverity[check_return] */
    WRITE_CMIC_TS_FREQ_CTRL_UPPERr(unit, regval);

    /* m CMIC_BS_HEARTBEAT_CTRL ENABLE=1 if we are master*/
    /* coverity[check_return] */
    READ_CMIC_BS_HEARTBEAT_CTRLr(unit, &regval);
    soc_reg_field_set(unit, CMIC_BS_HEARTBEAT_CTRLr, &regval, ENABLEf, master ? 1 : 0);
    /* coverity[check_return] */
    WRITE_CMIC_BS_HEARTBEAT_CTRLr(unit, regval);

    /* m CMIC_BS_HEARTBEAT_DOWN_DURATION DOWN_DURATION=2400 */
    /* coverity[check_return] */
    READ_CMIC_BS_HEARTBEAT_DOWN_DURATIONr(unit, &regval);
    soc_reg_field_set(unit, CMIC_BS_HEARTBEAT_DOWN_DURATIONr, &regval, DOWN_DURATIONf, hb_down);
    /* coverity[check_return] */
    WRITE_CMIC_BS_HEARTBEAT_DOWN_DURATIONr(unit, regval);

    /* m CMIC_BS_HEARTBEAT_UP_DURATION UP_DURATION=100 */
    /* coverity[check_return] */
    READ_CMIC_BS_HEARTBEAT_UP_DURATIONr(unit, &regval);
    soc_reg_field_set(unit, CMIC_BS_HEARTBEAT_UP_DURATIONr, &regval, UP_DURATIONf, hb_up);
    /* coverity[check_return] */
    WRITE_CMIC_BS_HEARTBEAT_UP_DURATIONr(unit, regval);

    /* m CMIC_BS_CLK_CTRL ENABLE=0 */
    /* coverity[check_return] */
    READ_CMIC_BS_CLK_CTRLr(unit, &regval);
    soc_reg_field_set(unit, CMIC_BS_CLK_CTRLr, &regval, ENABLEf, 0);
    /* coverity[check_return] */
    WRITE_CMIC_BS_CLK_CTRLr(unit, regval);

    /* m CMIC_BS_REF_CLK_GEN_CTRL ENABLE=0 */
    /* coverity[check_return] */
    READ_CMIC_BS_REF_CLK_GEN_CTRLr(unit, &regval);
    soc_reg_field_set(unit, CMIC_BS_REF_CLK_GEN_CTRLr, &regval, ENABLEf, 0);
    /* coverity[check_return] */
    WRITE_CMIC_BS_REF_CLK_GEN_CTRLr(unit, regval);

    /* m CX_PLL_CTRL TS_BIT_CLK_SEL=1 if we are master*/
    /* coverity[check_return] */
    READ_CX_PLL_CTRLr(unit, &regval);
    soc_reg_field_set(unit, CX_PLL_CTRLr, &regval, TS_BIT_CLK_SELf, master ? 1 : 0);
    /* coverity[check_return] */
    WRITE_CX_PLL_CTRLr(unit, regval);

    /* m CMIC_BS_CLK_CTRL ENABLE=1 LOW_DURATION=1 HIGH_DURATION=1 */
    /* coverity[check_return] */
    READ_CMIC_BS_CLK_CTRLr(unit, &regval);
    soc_reg_field_set(unit, CMIC_BS_CLK_CTRLr, &regval, ENABLEf, 1);
    soc_reg_field_set(unit, CMIC_BS_CLK_CTRLr, &regval, LOW_DURATIONf, bitclk_low);
    soc_reg_field_set(unit, CMIC_BS_CLK_CTRLr, &regval, HIGH_DURATIONf, bitclk_high);
    /* coverity[check_return] */
    WRITE_CMIC_BS_CLK_CTRLr(unit, regval);

    /* m CMIC_BS_CONFIG MODE=1 BS_CLK_OUTPUT_ENABLE=1 BS_HB_OUTPUT_ENABLE=1 */
    /* coverity[check_return] */
    READ_CMIC_BS_CONFIGr(unit, &regval);
    soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, MODEf, master ? TIME_MODE_OUTPUT : TIME_MODE_INPUT);
    soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, BS_CLK_OUTPUT_ENABLEf, output_enable);
    soc_reg_field_set(unit, CMIC_BS_CONFIGr, &regval, BS_HB_OUTPUT_ENABLEf, output_enable);
    /* coverity[check_return] */
    WRITE_CMIC_BS_CONFIGr(unit, regval);

    return BCM_E_NONE;
}

#endif /* #if defined(BCM_CALADAN3_SUPPORT) */
