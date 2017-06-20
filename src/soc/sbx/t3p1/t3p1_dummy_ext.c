/*
 * $Id: t3p1_dummy_ext.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_T3P1_SUPPORT)

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#include <soc/sbx/t3p1/t3p1_int.h>
#include <soc/sbx/t3p1/t3p1_defs.h>
#include <soc/cm.h>
#include <soc/sbx/caladan3/lrp.h>
#include <soc/sbx/caladan3/cop.h>

#define T3P1_EXC_STREAM      0

uint32 *_bubble_shadow_table[SOC_MAX_NUM_DEVICES];

extern soc_sbx_t3p1_ppe_ptable_cfg_t soc_sbx_t3p1_ppe_ptable_cfg;

/*
 *   Function
 *     soc_sbx_t3p1_ppe_property_table_segment_get
 *   Purpose
 *     Read a property table entry
 *   Parameters
 *      (IN) unit      : unit number of the device
 *      (IN) seg_id    : segment identifier
 *      (IN) offset    : entry offset (i.e. entry index)
 *      (OUT)data      : 1 byte data. Valid bits determined my mode, bits right aligned.
 *   Returns
 *       SOC_E_NONE    - success
 *       SOC_E_TIMEOUT - command timed out
 *       SOC_E_PARAM   - bad parameter value
 */
int soc_sbx_t3p1_ppe_property_table_segment_get(int unit, int seg_id, uint32 offset, uint8 *data) {
  int rv = SOC_E_NONE;
  int i;
  uint32 entry[2] = {0, 0};
  uint32 address;
  int shift;
  unsigned int width, mask;
  soc_sbx_t3p1_ppe_ptable_segment_t *segment = NULL;

  /*
   * Check seg_id is valid
   */
  for (i = 0;i < SOC_SBX_T3P1_PPE_PROPERTY_TABLE_SEGMENT_MAX;i++) {
    if (soc_sbx_t3p1_ppe_ptable_cfg.segment[i].seg_id == seg_id) {
      segment = &soc_sbx_t3p1_ppe_ptable_cfg.segment[i];
      break;
    }
  }

  if (segment == NULL) {
    return SOC_E_PARAM;
  }

  /*
   * Calculate address based on base address, mode and offset.
   */
  width = 1 << (soc_sbx_t3p1_ppe_ptable_cfg.mode + 1);
  address = segment->start + ((offset * width) / 64);

  rv = soc_sbx_caladan3_ppe_property_table_iaccess(unit,
                                                   1, /* Read */
                                                   address,
                                                   entry);
  shift = ((offset * width) % 64);
  if (shift > 31) {
    i = 1;
    shift -= 32;
  } else {
    i = 0;
  }
  mask = (1 << width) - 1;
  *data = (uint8)((entry[i] >> shift) & mask);
  return rv;
}

/*
 *   Function
 *     soc_sbx_t3p1_ppe_property_table_segment_set
 *   Purpose
 *     Write a property table entry
 *   Parameters
 *      (IN) unit      : unit number of the device
 *      (IN) seg_id    : segment identifier
 *      (IN) offset    : entry offset (i.e. entry index)
 *      (IN)data       : 1 byte data. Valid bits determined my mode, bits right aligned.
 *   Returns
 *       SOC_E_NONE    - success
 *       SOC_E_TIMEOUT - command timed out
 */
int soc_sbx_t3p1_ppe_property_table_segment_set(int unit, int seg_id, uint32 offset, uint8 data) {
  int rv = SOC_E_NONE;
  int i;
  uint32 entry[2] = {0, 0};
  uint32 address;
  int shift;
  unsigned int width, mask;
  soc_sbx_t3p1_ppe_ptable_segment_t *segment = NULL;

  /*
   * Check seg_id is valid
   */
  for (i = 0;i < SOC_SBX_T3P1_PPE_PROPERTY_TABLE_SEGMENT_MAX;i++) {
    if (soc_sbx_t3p1_ppe_ptable_cfg.segment[i].seg_id == seg_id) {
      segment = &soc_sbx_t3p1_ppe_ptable_cfg.segment[i];
      break;
    }
  }

  if (segment == NULL) {
    return SOC_E_PARAM;
  }

  /*
   * Calculate address based on base address, mode and offset.
   */
  width = 1 << (soc_sbx_t3p1_ppe_ptable_cfg.mode + 1);
  mask = (1 << width) - 1;
  address = segment->start + ((offset * width) / 64);

  rv = soc_sbx_caladan3_ppe_property_table_iaccess(unit,
                                                   1, /* Read */
                                                   address,
                                                   entry);
  shift = ((offset * width) % 64);
  if (shift > 31) {
    i = 1;
    shift -= 32;
  } else {
    i = 0;
  }

  entry[i] = (entry[i] & ~(mask << shift)) | (((uint32)data) << shift);

  rv = soc_sbx_caladan3_ppe_property_table_iaccess(unit,
                                                   0, /* Write */
                                                   address,
                                                   entry);

  return rv;
}

static void *t3p1_util_timer_cb_cookie;
static soc_sbx_t3p1_util_timer_event_callback_f t3p1_util_timer_cb;

/* Current code only works for a single timer segment   
 * If additional timer segments are required, need callback
 * function from t3p1_cop.c to get the segment id
 */
static 
void t3p1_util_timer_callback(int unit, int cop) {
    int rv;
    soc_sbx_caladan3_cop_timer_expire_event_t timer_event;
    soc_sbx_t3p1_util_timer_event_t event;
    
    rv = SOC_E_NONE;
    while (!SOC_FAILURE(rv)){
        rv = soc_sbx_caladan3_cop_timer_event_dequeue(unit, cop, &timer_event);
        if (!SOC_FAILURE(rv)) {
            event.timer_segment            = timer_event.uSegment;
            event.id                       = timer_event.uTimer;
            event.forced_timeout           = timer_event.bForced;
            event.timer_active_when_forced = timer_event.bActiveWhenForced;                
            t3p1_util_timer_cb(unit, &event, t3p1_util_timer_cb_cookie);
        } else if (rv != SOC_E_EMPTY) {
            LOG_CLI((BSL_META_U(unit,
                                "unit(%d) dequeue failure of timer event cop(%d)\n"), unit, cop));
        }
    }
}

int soc_sbx_t3p1_util_register_timer_callback(int unit, soc_sbx_t3p1_util_timer_event_callback_f cb, void *user_cookie)
{
    int rv;

    rv = soc_sbx_caladan3_cop_timer_event_callback_register(unit, t3p1_util_timer_callback);
    if (SOC_FAILURE(rv)) {
        LOG_CLI((BSL_META_U(unit,
                            "C3 %d failed to register util timer event callback\n"), unit));
        return rv;
    }

    t3p1_util_timer_cb = cb;
    t3p1_util_timer_cb_cookie = user_cookie;
    return SOC_E_NONE;
}

/* Set ext for p2e */
int
soc_sbx_t3p1_p2e_set_ext(int unit, int iport  , soc_sbx_t3p1_p2e_t *e)
{
    e->port = iport;
    e->portid = iport;

    if (SAL_BOOT_BCMSIM) {
        return(soc_sbx_t3p1_p2e_set_ext_sim(unit, iport  , e));
    }
    else {
      return (soc_sbx_t3p1_ppe_entry_p2e_set(unit, iport  , e));
    }
}

/* Get ext for p2e */
int
soc_sbx_t3p1_p2e_get_ext(int unit, int iport  , soc_sbx_t3p1_p2e_t *e)
{
    if (SAL_BOOT_BCMSIM) {
        return(soc_sbx_t3p1_p2e_get_ext_sim(unit, iport  , e));
    }
    else {
      return (soc_sbx_t3p1_ppe_entry_p2e_get(unit, iport  , e));
    }
} 


/* Set ext for ep2e */
int
soc_sbx_t3p1_ep2e_set_ext(int unit, int iport  , soc_sbx_t3p1_ep2e_t *e)
{
    e->port = iport;
    e->portid = iport;

    if (SAL_BOOT_BCMSIM) {
        return(soc_sbx_t3p1_ep2e_set_ext_sim(unit, iport  , e));
    }
    else {
      return (soc_sbx_t3p1_ppe_entry_ep2e_set(unit, iport  , e));
    }
}

/* Get ext for ep2e */
int
soc_sbx_t3p1_ep2e_get_ext(int unit, int iport  , soc_sbx_t3p1_ep2e_t *e)
{
    if (SAL_BOOT_BCMSIM) {
        return(soc_sbx_t3p1_ep2e_get_ext_sim(unit, iport  , e));
    }
    else {
      return (soc_sbx_t3p1_ppe_entry_ep2e_get(unit, iport  , e));
    }
} 

/*
 *  Routine: soc_sbx_t3p1_ppe_entry_p2e_get
 *  Description:
 *     
 *  Inputs:
 *      
 *      
 *  Outputs:
 *      
 */
int soc_sbx_t3p1_ppe_entry_p2e_get(int unit, int iport, soc_sbx_t3p1_p2e_t *e)
{
    uint32 data[16];
    soc_sbx_caladan3_ppe_iqsm_t iqsm;
    soc_sbx_caladan3_ppe_sqdm_t sqdm;
    uint8 *dp, *sp;
    int queue;
    int s = SOC_E_NONE;
    soc_sbx_t3p1_state_t *fe =
        (soc_sbx_t3p1_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_t3p1_ppe_table_manager_t *tm = fe->ppe_mgr;
    soc_sbx_t3p1_entry_desc_t *ed =
        &tm->entries[SOC_SBX_T3P1_PPE_P2E_ID];
   
    
    dp = (uint8*)&data[0];
    sp = (uint8*)&sqdm;
    
    s = soc_sbx_caladan3_get_squeue_from_port(unit, iport, 0, 0, &queue);
    if (SOC_FAILURE(s)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "port 2 queue get failed %d"), s));
        return s;
    }
        
    sal_memset(&sqdm, 0, sizeof(sqdm));
    sal_memset(&iqsm, 0, sizeof(iqsm));
    sal_memset(&data, 0, sizeof(data));
    if (SOC_SUCCESS(s)) {
        s = soc_sbx_caladan3_ppe_sqdm_entry_read(unit, queue, &sqdm);
        if (SOC_FAILURE(s)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "p2e sqdm get failed %d"), s));
            return s;
        }
        sal_memcpy(dp+4, sp, sizeof(sqdm));
        s = soc_sbx_caladan3_ppe_iqsm_entry_read(unit, queue, &iqsm);
        if (SOC_FAILURE(s)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "p2e iqsm get failed %d"), s));
            return s;
        }
        sal_memcpy(dp+1, &iqsm.istate, sizeof(iqsm.istate));
    }
    s = soc_sbx_t3p1_p2e_unpack(unit, e, dp, ed->epsize);
    if (SOC_SUCCESS(s)) {
        e->port = iport;
        e->portid = iport;
        e->hdrtype = iqsm.initial_type;
        e->ppe_variable = iqsm.initial_variable;
    }
    
    return s;
}

/*
 *  Routine: soc_sbx_t3p1_ppe_entry_p2e_set
 *  Description:
 *     
 *  Inputs:
 *      
 *      
 *  Outputs:
 *      
 */
int soc_sbx_t3p1_ppe_entry_p2e_set(int unit, int iport, soc_sbx_t3p1_p2e_t *e)
{

    uint32 data[16];
    soc_sbx_caladan3_ppe_iqsm_t iqsm;
    soc_sbx_caladan3_ppe_sqdm_t sqdm;
    uint8 *dp, *sp;
    int queue;
    int s = SOC_E_NONE;
    soc_sbx_t3p1_state_t *fe =
        (soc_sbx_t3p1_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_t3p1_ppe_table_manager_t *tm = fe->ppe_mgr;
    soc_sbx_t3p1_entry_desc_t *ed =
        &tm->entries[SOC_SBX_T3P1_PPE_P2E_ID];

    s = soc_sbx_caladan3_get_squeue_from_port(unit, iport, 0, 0, &queue);
    if (SOC_FAILURE(s)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "port to queue get failed %d"), s));
        return s;
    }
    
       
    dp = (uint8*)&data[0];
    sp = (uint8*)&sqdm;
    sal_memset(&sqdm, 0, sizeof(sqdm));
    sal_memset(&iqsm, 0, sizeof(iqsm));
    sal_memset(data, 0, sizeof(data));
    s = soc_sbx_t3p1_p2e_pack(unit, e, dp, ed->epsize);
    if (SOC_SUCCESS(s)) {
        sal_memcpy(sp, dp+4, sizeof(sqdm));
        s = soc_sbx_caladan3_ppe_iqsm_entry_read(unit, queue, &iqsm);
        if (SOC_FAILURE(s)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "p2e iqsm get failed %d"), s));
            return s;
        }
        sal_memcpy(&iqsm.istate, dp+1, sizeof(iqsm.istate));
        iqsm.initial_type = e->hdrtype;
        iqsm.initial_variable = e->ppe_variable;
       
        iqsm.initial_load_sq_data = 0xf00;
        s = soc_sbx_caladan3_ppe_iqsm_entry_write(unit, queue, &iqsm);
        if (SOC_FAILURE(s)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "p2e iqsm set failed %d"), s));
            return s;
        }
        s = soc_sbx_caladan3_ppe_sqdm_entry_write(unit, queue, &sqdm);
    }
    
    return s;
}

/*
 *  Routine: soc_sbx_t3p1_ppe_entry_ep2e_set
 *  Description:
 *     
 *  Inputs:
 *      
 *      
 *  Outputs:
 *      
 */
int soc_sbx_t3p1_ppe_entry_ep2e_set(int unit, int iport, soc_sbx_t3p1_ep2e_t *e)
{
    uint32 data[16];
    soc_sbx_caladan3_ppe_iqsm_t iqsm;
    soc_sbx_caladan3_ppe_sqdm_t sqdm;
    uint8 *dp, *sp;
    int queue;
    int s = SOC_E_NONE;
    soc_sbx_t3p1_state_t *fe = (soc_sbx_t3p1_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_t3p1_ppe_table_manager_t *tm = fe->ppe_mgr;
    soc_sbx_t3p1_entry_desc_t *ed = &tm->entries[SOC_SBX_T3P1_PPE_EP2E_ID];

    s = soc_sbx_caladan3_get_squeue_from_port(unit, iport, 1, 0, &queue);
    if (SOC_FAILURE(s)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "port 2 queue get failed %d"), s));
        return s;
    }
            
    dp = (uint8*)&data[0];
    sp = (uint8*)&sqdm;
    
    sal_memset(&sqdm, 0, sizeof(sqdm));
    sal_memset(&iqsm, 0, sizeof(iqsm));
    sal_memset(data, 0, sizeof(data));
    s = soc_sbx_t3p1_ep2e_pack(unit, e, dp, ed->epsize);
    if (SOC_SUCCESS(s)) {
        sal_memcpy(sp, dp+4, sizeof(sqdm));
        s = soc_sbx_caladan3_ppe_iqsm_entry_read(unit, queue, &iqsm);
        if (SOC_FAILURE(s)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "p2e iqsm get failed %d"), s));
            return s;
        }
        sal_memcpy(&iqsm.istate, dp+1, sizeof(iqsm.istate));
        iqsm.initial_type = e->hdrtype;
        iqsm.initial_load_sq_data = 0xfc0;
        iqsm.initial_variable = e->ppe_variable;
        s = soc_sbx_caladan3_ppe_iqsm_entry_write(unit, queue, &iqsm);
        if (SOC_FAILURE(s)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "p2e iqsm set failed %d"), s));
            return s;
        }
        s = soc_sbx_caladan3_ppe_sqdm_entry_write(unit, queue, &sqdm);
    }
    
    return s;
}

/*
 *  Routine: soc_sbx_t3p1_ppe_entry_ep2e_get
 *     
 *  Inputs:
 *      
 *      
 *  Outputs:
 *      
 */
int soc_sbx_t3p1_ppe_entry_ep2e_get(int unit, int iport, soc_sbx_t3p1_ep2e_t *e)
{
    uint32 data[16];
    soc_sbx_caladan3_ppe_iqsm_t iqsm;
    soc_sbx_caladan3_ppe_sqdm_t sqdm;
    uint8 *dp, *sp;
    int queue = 0;
    int s = SOC_E_NONE;
    soc_sbx_t3p1_state_t *fe = (soc_sbx_t3p1_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_t3p1_ppe_table_manager_t *tm = fe->ppe_mgr;
    soc_sbx_t3p1_entry_desc_t *ed =
        &tm->entries[SOC_SBX_T3P1_PPE_EP2E_ID];
    
    
    dp = (uint8*)&data[0];
    sp = (uint8*)&sqdm;
    
    s = soc_sbx_caladan3_get_squeue_from_port(unit, iport, 1, 0, &queue);
    if (SOC_FAILURE(s)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "port 2 queue get failed %d"), s));
        return s;
    }
        
    sal_memset(&sqdm, 0, sizeof(sqdm));
    sal_memset(&iqsm, 0, sizeof(iqsm));
    sal_memset(data, 0, sizeof(data));
    if (SOC_SUCCESS(s)) {
        s = soc_sbx_caladan3_ppe_sqdm_entry_read(unit, queue, &sqdm);
        if (SOC_FAILURE(s)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "p2e sqdm get failed %d"), s));
            return s;
        }
        sal_memcpy(dp+4, sp, sizeof(sqdm));
        s = soc_sbx_caladan3_ppe_iqsm_entry_read(unit, queue, &iqsm);
        if (SOC_FAILURE(s)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "p2e iqsm get failed %d"), s));
            return s;
        }
        sal_memcpy(dp+1, &iqsm.istate, sizeof(iqsm.istate));
    }
    s = soc_sbx_t3p1_ep2e_unpack(unit, e, dp, ed->epsize);
    if (SOC_SUCCESS(s)) {
        e->port = iport;
        e->hdrtype = iqsm.initial_type;
        e->ppe_variable = iqsm.initial_variable;
    }
    
    return s;
}
#endif

