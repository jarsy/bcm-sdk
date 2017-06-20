/*
 * $Id: g3p1_dummy_ext.c,v 1.32.10.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT)

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/cm.h>
#include <soc/sbx/caladan3/lrp.h>
#include <soc/sbx/caladan3/cop.h>
#include <soc/sbx/g3p1/g3p1_ppe_rule_encode.h>

#define G3P1_LSM_CAM         0
#define G3P1_LSM_ENTRY_BASE 16
#define G3P1_LSM_COUNT      64
#define G3P1_EXC_STREAM      0

#define G3P1_TPID_CAM         1
#define G3P1_TPID_ENTRY_BASE 10
#define G3P1_TPID_COUNT       4
#define G3P1_TPID_INDEX       4

#define G3P1_ELSM_CAM         1
#define G3P1_ELSM_ENTRY_BASE 32
#define G3P1_ELSM_COUNT      64

#define G3P1_EGR_ERH_MPLS_CAM            0
#define G3P1_EGR_ERH_MPLS_ARAD_ENTRY   109
#define G3P1_EGR_ERH_MPLS_SIRIUS_ENTRY 111



typedef struct g3p1_tpid_rule_s {
    uint8  entry[G3P1_TPID_COUNT];
    int    count;
} g3p1_tpid_rule_t;

g3p1_tpid_rule_t _rule[G3P1_TPID_INDEX] =
 { { {2,0,0,0}, 1},
   { {0,0,0,0}, 1},
   { {1,0,0,0}, 1},
   { {3,0,0,0}, 1} };

typedef struct g3p1_rule_id_s {
    uint8 stage;
    uint8 entry;
} g3p1_rule_id_t;

static g3p1_rule_id_t vid_two_rules[] = { { 1, 10 }, { 1, 11 }, { 1, 12 } };
static g3p1_rule_id_t vid_one_rules[] = { { 1, 16 }, { 1, 17 }, { 1, 18 }, { 1, 19 } };
static g3p1_rule_id_t ipv4_rules[] = { { 4, 0 } };
static g3p1_rule_id_t ipv6_rules[] = { { 4, 1 } };
static g3p1_rule_id_t port_rules[] = { { 5, 0 }, { 5, 1 }, { 5, 5}, {5, 6} };


uint32 *_bubble_shadow_table[SOC_MAX_NUM_DEVICES];
/*
 *  Routine: soc_sbx_g3p1_ppe_entry_enable_set
 *  Description:
 *     Update PPE rule to set valid state
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      valid - (0/1)
 *  Outputs:
 *      one of SOC_E* status types
 */
int
soc_sbx_g3p1_ppe_entry_enable_set(int unit, uint8 camid, uint8 entry, uint8 valid) {

    int rv;
    soc_sbx_caladan3_ppe_tcamdata_t cam; 
    uint8 org_valid;
    uint8 new_valid;

    rv = soc_sbx_caladan3_ppe_tcam_entry_read(unit, camid, entry, &cam, &org_valid);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d ppe_tcam_entry_read %d\n"),
                   unit, rv));
        return (rv);
    }

    if (valid) {
        new_valid = SOC_SBX_CALADAN3_PPE_TCAM_ENTRY_VALID;
    } else {
        new_valid = SOC_SBX_CALADAN3_PPE_TCAM_ENTRY_INVALID;
    }

    rv = soc_sbx_caladan3_ppe_tcam_entry_write(unit, camid, entry, &cam, new_valid);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d ppe_tcam_entry_write %d\n"),
                   unit, rv));
        return (rv);
    }

    return SOC_E_NONE;
}  


extern soc_sbx_g3p1_ppe_ptable_cfg_t soc_sbx_g3p1_ppe_ptable_cfg;

static int soc_sbx_g3p1_ppe_property_table_init(int unit)
{
  int rv = SOC_E_NONE;
  int i;
  /*
   * Call soc API with configurated parameters
   */

   rv = soc_sbx_caladan3_ppe_property_table_init(unit,
                                                 soc_sbx_g3p1_ppe_ptable_cfg.mode,
                                                 soc_sbx_g3p1_ppe_ptable_cfg.portA,
                                                 soc_sbx_g3p1_ppe_ptable_cfg.portB,
                                                 soc_sbx_g3p1_ppe_ptable_cfg.portC,
                                                 soc_sbx_g3p1_ppe_ptable_cfg.portD);
   if (rv != SOC_E_NONE)
     return rv;

   /*
    * Init segments
    */
   for (i = 0;i < SOC_SBX_G3P1_PPE_PROPERTY_TABLE_SEGMENT_MAX;i++)
   {
     if (soc_sbx_g3p1_ppe_ptable_cfg.segment[i].segment != -1)
     {
       rv = soc_sbx_caladan3_ppe_property_table_segment_init(unit,
                                                             soc_sbx_g3p1_ppe_ptable_cfg.segment[i].segment,
                                                             soc_sbx_g3p1_ppe_ptable_cfg.segment[i].seg_id,
                                                             soc_sbx_g3p1_ppe_ptable_cfg.segment[i].start);
       if (rv != SOC_E_NONE)
         return rv;
     }
   }
   return rv;
}

/*
 *   Function
 *     soc_sbx_g3p1_ppe_property_table_segment_get
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
int soc_sbx_g3p1_ppe_property_table_segment_get(int unit, int seg_id, uint32 offset, uint8 *data) {
  int rv = SOC_E_NONE;
  int i;
  uint32 entry[2] = {0, 0};
  uint32 address;
  int shift;
  unsigned int width, mask;
  soc_sbx_g3p1_ppe_ptable_segment_t *segment = NULL;

  /*
   * Check seg_id is valid
   */
  for (i = 0;i < SOC_SBX_G3P1_PPE_PROPERTY_TABLE_SEGMENT_MAX;i++) {
    if (soc_sbx_g3p1_ppe_ptable_cfg.segment[i].seg_id == seg_id) {
      segment = &soc_sbx_g3p1_ppe_ptable_cfg.segment[i];
      break;
    }
  }

  if (segment == NULL) {
    return SOC_E_PARAM;
  }

  /*
   * Calculate address based on base address, mode and offset.
   */
  width = 1 << (soc_sbx_g3p1_ppe_ptable_cfg.mode + 1);
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
 *     soc_sbx_g3p1_ppe_property_table_segment_set
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
int soc_sbx_g3p1_ppe_property_table_segment_set(int unit, int seg_id, uint32 offset, uint8 data) {
  int rv = SOC_E_NONE;
  int i;
  uint32 entry[2] = {0, 0};
  uint32 address;
  int shift;
  unsigned int width, mask;
  soc_sbx_g3p1_ppe_ptable_segment_t *segment = NULL;

  /*
   * Check seg_id is valid
   */
  for (i = 0;i < SOC_SBX_G3P1_PPE_PROPERTY_TABLE_SEGMENT_MAX;i++) {
    if (soc_sbx_g3p1_ppe_ptable_cfg.segment[i].seg_id == seg_id) {
      segment = &soc_sbx_g3p1_ppe_ptable_cfg.segment[i];
      break;
    }
  }

  if (segment == NULL) {
    return SOC_E_PARAM;
  }

  /*
   * Calculate address based on base address, mode and offset.
   */
  width = 1 << (soc_sbx_g3p1_ppe_ptable_cfg.mode + 1);
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


#define G3P1_MAC_IS_ZERO(_mac_)  \
    (((_mac_)[0] | (_mac_)[1] | (_mac_)[2] | \
      (_mac_)[3] | (_mac_)[4] | (_mac_)[5]) == 0)

/* #define PVV_TAPS_ENABLE 1 */

#ifdef PVV_TAPS_ENABLE
#define PVV2E_TABLE_SIZE 65536
#else
#define PVV2E_TABLE_SIZE 8192 
#endif
#define PVV2E_INVALID_INDEX 0x1FFFF
#define PORT_LENGTH 6
#define PORT_MASK  ((1<<PORT_LENGTH)-1)
#define VID_LENGTH 12
#define VID_MASK  ((1<<VID_LENGTH)-1)
#define SOC_PVV_KEY(port, ovid, ivid) port, ovid, ivid
    
#ifdef G3P1_PVV_MODE_0_ENABLE
static int pvv2e_id_table[PVV2E_TABLE_SIZE] = {0};
static int pvv2e_id_first = PVV2E_INVALID_INDEX;
static int pvv2e_id_size  = 0;


static int _soc_sbx_g3p1_util_entry_init(int unit)
{
    int rv = SOC_E_NONE;
    int i = 0;
    soc_sbx_g3p1_pvv2edata_t pvv2edata;

    /* Leave entry 0. It is loaded when TAPS lookup fails and returns 0 */
    for (i = 1; i < PVV2E_TABLE_SIZE; i++) {
        pvv2e_id_table[i] = i+1;
    }
    pvv2e_id_first = 1;

    /* initialize entry 0 */
    soc_sbx_g3p1_pvv2edata_t_init(&pvv2edata);
    soc_sbx_g3p1_pvv2edata_set(unit, 0, &pvv2edata);
  
    return rv;
}

static int _soc_sbx_g3p1_util_entry_alloc(int unit, int *id)
{
    int rv = SOC_E_NONE;

    /* check whether all entries are allocated,
     * i.e., check whether there are free entries.
     * Don't count the entry 0
     */
    if (pvv2e_id_size >= (PVV2E_TABLE_SIZE - 1)) {
        return SOC_E_NOT_FOUND;
    }
    if (PVV2E_INVALID_INDEX == pvv2e_id_first) {
        _soc_sbx_g3p1_util_entry_init(unit);
    }

    *id = pvv2e_id_first;
    pvv2e_id_first = pvv2e_id_table[*id];
    pvv2e_id_table[*id] = PVV2E_INVALID_INDEX;
    pvv2e_id_size++;
  
    return rv;
}

static int _soc_sbx_g3p1_util_entry_free(int id)
{
    int rv = SOC_E_NONE;

    if ((pvv2e_id_size <= 0) || (PVV2E_INVALID_INDEX != pvv2e_id_first)) {
        return SOC_E_NOT_FOUND;
    }
    pvv2e_id_table[id] = pvv2e_id_first;
    pvv2e_id_first = id;
    pvv2e_id_size--;
    
    return rv;
}
#endif

int soc_sbx_g3p1_util_pvv2e_get(int unit, int port, int ovid, int ivid, soc_sbx_g3p1_pvv2edata_t *e)
{
    int rv = SOC_E_NONE;
    soc_sbx_g3p1_pvv2e_t tmu_pvv;

#ifdef PVV_TAPS_ENABLE
    rv = soc_sbx_g3p1_pvv2e_get
        (unit, 0, port, ovid, ivid, 32, &tmu_pvv);
#else
    rv = soc_sbx_g3p1_pvv2e_get(unit, ivid, ovid, port, &tmu_pvv);
#endif
#ifdef G3P1_PVV_MODE_0_ENABLE
    if (SOC_FAILURE(rv) || PVV2E_INVALID_INDEX == tmu_pvv.pvv2e_idx) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "\nsoc_sbx_g3p1_pvv2e_get failed: unit=%d, port=%d, ovid=%d, ivid=%d\n"),
                     unit, port, ovid, ivid));
        return rv;
    }
    rv = soc_sbx_g3p1_pvv2edata_get(unit, tmu_pvv.pvv2e_idx, e);
#else
    e->vpws     = tmu_pvv.vpws;
    e->stpstate = tmu_pvv.stpstate;
    e->vlan     = tmu_pvv.vlan;
    e->lpi      = tmu_pvv.lpi;
    e->hit      = tmu_pvv.hit;
    e->keeporstrip = tmu_pvv.keeporstrip;
    e->replace    = tmu_pvv.replace;
    e->vid        = tmu_pvv.vid;
#ifdef PVV_TAPS_ENABLE
    e->vidop      = tmu_pvv.vidop;
#endif
#endif

    return rv;
}


/* 
 * soc_sbx_g3p1_util_pvv2e_set
 *
 * if (entry exists)
 *     update
 * else
 *     add new entry
 *
 */

int soc_sbx_g3p1_util_pvv2e_set(int unit, int port, int ovid, int ivid, soc_sbx_g3p1_pvv2edata_t *e)
{
    int rv = SOC_E_NONE;

    rv = soc_sbx_g3p1_util_pvv2e_update(unit, port, ovid, ivid, e);
    if (SOC_FAILURE(rv)) {
        rv = soc_sbx_g3p1_util_pvv2e_add(unit, port, ovid, ivid, e);
    }
  
    return rv;
}


/*
 * soc_sbx_g3p1_util_pvv2e_update
 *
 *  If (entry exists)
 *      update
 *  else
 *      report error
 *
 */

int soc_sbx_g3p1_util_pvv2e_update(int unit, int port, int ovid, int ivid, soc_sbx_g3p1_pvv2edata_t *e)
{
    int rv = SOC_E_NONE;
    soc_sbx_g3p1_pvv2e_t tmu_pvv;
  
#ifdef PVV_TAPS_ENABLE
    rv = soc_sbx_g3p1_pvv2e_get
        (unit, 0, SOC_PVV_KEY(port, ovid, ivid), 32, &tmu_pvv);
#else
    rv = soc_sbx_g3p1_pvv2e_get
        (unit, ivid, ovid, port, &tmu_pvv);

#endif

    if (SOC_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "\nsoc_sbx_g3p1_pvv2e_get failed: unit=%d, port=%d, ovid=%d, ivid=%d\n"),
                     unit, port, ovid, ivid));
        return rv;
    }
#ifdef G3P1_PVV_MODE_0_ENABLE
    if (PVV2E_INVALID_INDEX == tmu_pvv.pvv2e_idx) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "\nInvalid index for pvv2edata :  unit=%d, port=%d, ovid=%d, ivid=%d\n"),
                     unit, port, ovid, ivid));
        return SOC_E_INTERNAL;
    }
    /* When adding or updating, set the hit bit */
    e.hit = 1;
    rv = soc_sbx_g3p1_pvv2edata_set(unit, tmu_pvv.pvv2e_idx, e);
#else
    /* Get is successful. So update the entry */
    tmu_pvv.vpws      = e->vpws;
    tmu_pvv.stpstate  = e->stpstate;
    tmu_pvv.vlan      = e->vlan;
    tmu_pvv.lpi       = e->lpi;
    /* When adding or updating, set the hit bit */
    tmu_pvv.hit       = 1;
    tmu_pvv.keeporstrip = e->keeporstrip;
    tmu_pvv.replace  = e->replace;
    tmu_pvv.vid      = e->vid;
#ifdef PVV_TAPS_ENABLE
    tmu_pvv.vidop    = e->vidop;
#endif

#ifdef PVV_TAPS_ENABLE
    rv = soc_sbx_g3p1_pvv2e_set
        (unit, 0, SOC_PVV_KEY(port, ovid, ivid), 32, &tmu_pvv);
#else
    rv = soc_sbx_g3p1_pvv2e_set
         (unit, ivid, ovid, port, &tmu_pvv);
#endif
    if (SOC_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "\nsoc_sbx_g3p1_pvv2e_set failed: unit=%d, port=%d, ovid=%d, ivid=%d\n"),
                     unit, port, ovid, ivid));
    }
#endif
  
    return rv;
}

/* soc_sbx_g3p1_util_pvv2e_add
 *     Add new entry
 */

int soc_sbx_g3p1_util_pvv2e_add(int unit, int port, int ovid, int ivid, soc_sbx_g3p1_pvv2edata_t *e)
{
    int rv = SOC_E_NONE;
#ifdef G3P1_PVV_MODE_0_ENABLE
    int index = 0;
#endif
    soc_sbx_g3p1_pvv2e_t tmu_pvv;

    soc_sbx_g3p1_pvv2e_t_init(&tmu_pvv);
#ifdef G3P1_PVV_MODE_0_ENABLE
    rv = _soc_sbx_g3p1_util_entry_alloc(&index);
    if (SOC_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "\nsoc_util_entry_alloc failed\n")));
        return rv;
    }
    tmu_pvv.pvv2e_idx = index;
#else
    tmu_pvv.vpws      = e->vpws;
    tmu_pvv.stpstate  = e->stpstate;
    tmu_pvv.vlan      = e->vlan;
    tmu_pvv.lpi       = e->lpi;
    /* When adding or updating, set the hit bit */
    tmu_pvv.hit       = 1;
    tmu_pvv.keeporstrip = e->keeporstrip;
    tmu_pvv.replace  = e->replace;
    tmu_pvv.vid      = e->vid;
#ifdef PVV_TAPS_ENABLE
    tmu_pvv.vidop    = e->vidop;
#endif
#endif
#ifdef PVV_TAPS_ENABLE
    rv = soc_sbx_g3p1_pvv2e_set
        (unit, 0, SOC_PVV_KEY(port, ovid, ivid), 32, &tmu_pvv);
#else
    rv = soc_sbx_g3p1_pvv2e_set
         (unit, ivid, ovid, port, &tmu_pvv);
#endif
    if (SOC_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "\nsoc_sbx_g3p1_pvv2e_set failed: unit=%d, port=%d, ovid=%d, ivid=%d\n"),
                     unit, port, ovid, ivid));
        return rv;
    }
#ifdef G3P1_PVV_MODE_0_ENABLE
    /* When adding or updating, set the hit bit */
    e.hit = 1;
    rv = soc_sbx_g3p1_pvv2edata_set(unit, tmu_pvv.pvv2e_idx, e);
#endif
    
    return rv;
}

int soc_sbx_g3p1_util_pvv2e_remove(int unit, int port, int ovid, int ivid)
{
    int rv = SOC_E_NONE;
#ifdef G3P1_PVV_MODE_0_ENABLE
    int index = 0;
    soc_sbx_g3p1_pvv2e_t tmu_pvv;
#endif
    
#ifdef G3P1_PVV_MODE_0_ENABLE
#ifdef PVV_TAPS_ENABLE
    rv = soc_sbx_g3p1_pvv2e_get
        (unit, 0, SOC_PVV_KEY(port, ovid, ivid), 32, &tmu_pvv);
#else
    rv = soc_sbx_g3p1_pvv2e_get
        (unit, ivid, ovid, port, &tmu_pvv);
#endif

    if (SOC_FAILURE(rv) || (PVV2E_INVALID_INDEX == tmu_pvv.pvv2e_idx)) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "\nsoc_sbx_g3p1_pvv2e_get failed: unit=%d, port=%d, ovid=%d, ivid=%d\n"),
                     unit, port, ovid, ivid));
        return rv;
    }
    index = tmu_pvv.pvv2e_idx;
#endif
#ifdef PVV_TAPS_ENABLE
    rv = soc_sbx_g3p1_pvv2e_remove
        (unit, 0, SOC_PVV_KEY(port, ovid, ivid), 32);
#else
    rv = soc_sbx_g3p1_pvv2e_remove
        (unit, ivid, ovid, port);
#endif
    if (SOC_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "\nsoc_sbx_g3p1_pvv2e_remove failed: unit=%d, port=%d, ovid=%d, ivid=%d\n"),
                     unit, port, ovid, ivid));
        return rv;
    }
#ifdef G3P1_PVV_MODE_0_ENABLE
    rv = _soc_sbx_g3p1_util_entry_free(index);
#endif

    return rv;
}

int soc_sbx_g3p1_util_pvv2e_first(int unit, int *port, int *ovid, int *ivid)
{
    return SOC_E_UNAVAIL;
}

int soc_sbx_g3p1_util_pvv2e_next(int unit, int port, int ovid, int ivid, int *nport, int *novid, int *nivid)
{
    return SOC_E_UNAVAIL;
}


/*
 *  Routine: soc_sbx_g3p1_ppe_entry_lsm_init
 *  Description:
 *     Initialize PPE rules to set local station match
 *  Inputs:
 *      camid - cam state (0-13) of base entry
 *      entry - index of the base entry (0-126)
 *      count - number of entries to initialize 
 *  Outputs:
 *      one of SOC_E* status types
 */
int
soc_sbx_g3p1_ppe_entry_lsm_init(int unit, uint8 camid, uint8 entry, uint8 count) {

    int rv;
    int i;
    soc_sbx_caladan3_ppe_tcamdata_t cam;
    soc_sbx_caladan3_ppe_camram_t   camram;
    uint8 valid;

    rv = soc_sbx_caladan3_ppe_tcam_entry_read(unit, camid, entry, &cam, &valid);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d ppe_tcam_entry_read %d\n"),
                   unit, rv));
        return (rv);
    }
    rv = soc_sbx_caladan3_ppe_camram_entry_read(unit, camid, entry, &camram);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d ppe_camram_entry_read %d\n"),
                   unit, rv));
        return (rv);
    }

    valid = SOC_SBX_CALADAN3_PPE_TCAM_ENTRY_INVALID;

    sal_memset(&cam.data[0], 0, 6);

    for (i=1; i<count; i++) {
        rv = soc_sbx_caladan3_ppe_tcam_entry_write(unit, camid, entry+i, &cam, valid);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "g3p1 unit %d ppe_tcam_entry_write %d\n"),
                       unit, rv));
            return (rv);
        }
        rv = soc_sbx_caladan3_ppe_camram_entry_write(unit, camid, entry+i, &camram);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "g3p1 unit %d ppe_camram_entry_write %d\n"),
                       unit, rv));
            return (rv);
        }
    }

    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_g3p1_ppe_entry_lsmac_set
 *  Description:
 *     Update PPE rule to set local station match
 *  Inputs:
 *      lsi   - index of the entry
 *      lsm   - LSM MAC address and port information
 *  Outputs:
 *      one of SOC_E* status types
 */
int
soc_sbx_g3p1_ppe_entry_lsmac_set(int unit, int lsi, soc_sbx_g3p1_lsmac_t *lsm) {

    int rv;
    soc_sbx_caladan3_ppe_tcamdata_t cam;
    uint8 valid;

    rv = soc_sbx_caladan3_ppe_tcam_entry_read(unit, G3P1_LSM_CAM, G3P1_LSM_ENTRY_BASE+lsi, &cam, &valid);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d ppe_tcam_entry_read %d\n"),
                   unit, rv));
        return (rv);
    }

    if (G3P1_MAC_IS_ZERO(lsm->mac)) {
        valid = SOC_SBX_CALADAN3_PPE_TCAM_ENTRY_INVALID;
    } else {
        valid = SOC_SBX_CALADAN3_PPE_TCAM_ENTRY_VALID;
    }

    cam.state[2] = lsm->portid;
    if (lsm->useport) {
        cam.state_mask[2] = 0xff;
    } else {
        cam.state_mask[2] = 0;
    }
    sal_memcpy(&cam.data[0], lsm->mac, 6);

    rv = soc_sbx_caladan3_ppe_tcam_entry_write(unit, G3P1_LSM_CAM, G3P1_LSM_ENTRY_BASE+lsi, &cam, valid);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d ppe_tcam_entry_write %d\n"),
                   unit, rv));
        return (rv);
    }

    return SOC_E_NONE;
} 

/*
 *  Routine: soc_sbx_g3p1_ppe_entry_lsmac_get
 *  Description:
 *     Update PPE rule to get local station match
 *  Inputs:
 *      lsi   - index of the entry
 *      lsm   - LSM MAC address and port information
 *  Outputs:
 *      one of SOC_E* status types
 */
int
soc_sbx_g3p1_ppe_entry_lsmac_get(int unit, int lsi, soc_sbx_g3p1_lsmac_t *lsm) {

    int rv;
    soc_sbx_caladan3_ppe_tcamdata_t cam;
    uint8 valid;

    rv = soc_sbx_caladan3_ppe_tcam_entry_read(unit, G3P1_LSM_CAM, G3P1_LSM_ENTRY_BASE+lsi, &cam, &valid);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d ppe_tcam_entry_read %d\n"),
                   unit, rv));
        return (rv);
    }

    lsm->portid = cam.state[2];
    if (cam.state_mask[2]) {
        lsm->useport = 1;
    } else {
        lsm->useport = 0;
    }

    if (valid) {
        sal_memcpy(lsm->mac, &cam.data[0], 6);
    } else {
        sal_memset(lsm->mac, 0, 6);
    }

    return SOC_E_NONE;
}


/*
 *  Routine: soc_sbx_g3p1_elsmac_init
 *  Description:
 *     Initialize PPE rules to set egress local station match
 *  Inputs:
 *      camid - cam state (0-13) of base entry
 *      entry - index of the base entry (0-126)
 *      count - number of entries to initialize 
 *  Outputs:
 *      one of SOC_E* status types
 */
int
soc_sbx_g3p1_elsmac_init(int unit, uint8 camid, uint8 entry, uint8 count) {

    int rv;
    int i;
    soc_sbx_caladan3_ppe_tcamdata_t cam;
    soc_sbx_caladan3_ppe_camram_t   camram;
    uint8 valid;

    rv = soc_sbx_caladan3_ppe_tcam_entry_read(unit, camid, entry, &cam, &valid);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d ppe_tcam_entry_read %d\n"),
                   unit, rv));
        return (rv);
    }
    rv = soc_sbx_caladan3_ppe_camram_entry_read(unit, camid, entry, &camram);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d ppe_camram_entry_read %d\n"),
                   unit, rv));
        return (rv);
    }

    valid = SOC_SBX_CALADAN3_PPE_TCAM_ENTRY_INVALID;

    sal_memset(&cam.data[0], 0, 6);

    for (i=1; i<count; i++) {
        rv = soc_sbx_caladan3_ppe_tcam_entry_write(unit, camid, entry+i, &cam, valid);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "g3p1 unit %d ppe_tcam_entry_write %d\n"),
                       unit, rv));
            return (rv);
        }
        rv = soc_sbx_caladan3_ppe_camram_entry_write(unit, camid, entry+i, &camram);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "g3p1 unit %d ppe_camram_entry_write %d\n"),
                       unit, rv));
            return (rv);
        }
    }

    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_g3p1_ppe_entry_set
 *  Description:
 *     Update PPE rule to set egress local station match
 *  Inputs:
 *      lsi   - index of the entry
 *      oamupmac   - LSM MAC address and port information
 *  Outputs:
 *      one of SOC_E* status types
 */
int
soc_sbx_g3p1_ppe_entry_elsmac_set(int unit, int lsi, soc_sbx_g3p1_elsmac_t *elsmac) {

    int rv;
    soc_sbx_caladan3_ppe_tcamdata_t cam;
    uint8 valid;

    rv = soc_sbx_caladan3_ppe_tcam_entry_read(unit, G3P1_ELSM_CAM, G3P1_ELSM_ENTRY_BASE+lsi, &cam, &valid);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d ppe_tcam_entry_read %d\n"),
                   unit, rv));
        return (rv);
    }

    if (G3P1_MAC_IS_ZERO(elsmac->mac)) {
        valid = SOC_SBX_CALADAN3_PPE_TCAM_ENTRY_INVALID;
    } else {
        valid = SOC_SBX_CALADAN3_PPE_TCAM_ENTRY_VALID;
    }

    cam.state[2] = elsmac->portid;
    if (elsmac->useport) {
        cam.state_mask[2] = 0xff;
    } else {
        cam.state_mask[2] = 0;
    }
    sal_memcpy(&cam.data[0], elsmac->mac, 6);

    rv = soc_sbx_caladan3_ppe_tcam_entry_write(unit, G3P1_ELSM_CAM, G3P1_ELSM_ENTRY_BASE+lsi, &cam, valid);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d ppe_tcam_entry_write %d\n"),
                   unit, rv));
        return (rv);
    }

    return SOC_E_NONE;
} 

/*
 *  Routine: soc_sbx_g3p1_ppe_entry_elsmac_get_ext
 *  Description:
 *     Update PPE rule to get egress local station match
 *  Inputs:
 *      lsi   - index of the entry
 *      elsmac   - LSM MAC address and port information
 *  Outputs:
 *      one of SOC_E* status types
 */
int
soc_sbx_g3p1_ppe_entry_elsmac_get(int unit, int lsi, soc_sbx_g3p1_elsmac_t *elsmac) {

    int rv;
    soc_sbx_caladan3_ppe_tcamdata_t cam;
    uint8 valid;

    rv = soc_sbx_caladan3_ppe_tcam_entry_read(unit, G3P1_ELSM_CAM, G3P1_ELSM_ENTRY_BASE+lsi, &cam, &valid);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d ppe_tcam_entry_read %d\n"),
                   unit, rv));
        return (rv);
    }

    elsmac->portid = cam.state[2];
    if (cam.state_mask[2]) {
        elsmac->useport = 1;
    } else {
        elsmac->useport = 0;
    }

    if (valid) {
        sal_memcpy(elsmac->mac, &cam.data[0], 6);
    } else {
        sal_memset(elsmac->mac, 0, 6);
    }

    return SOC_E_NONE;
}



/*
 *  Routine: soc_sbx_g3p1_ppe_entry_tpid_set
 *  Description:
 *     Update PPE rule to set tpid 
 *  Inputs:
 *      entry - index of the tpid stack
 *      tpid  - tpid value
 *  Outputs:
 *      one of SOC_E* status types
 */
int
soc_sbx_g3p1_ppe_entry_tpid_set(int unit, uint8 entry, soc_sbx_g3p1_tpid_t *tpid) {

    int rv;
    soc_sbx_caladan3_ppe_tcamdata_t cam;
    int   i, rule;
    uint8 valid;
    uint8 tpid_data[2];

    for (i=0; i < _rule[entry].count; i++) {
        rule = G3P1_TPID_ENTRY_BASE + _rule[entry].entry[i];

        rv = soc_sbx_caladan3_ppe_tcam_entry_read(unit, G3P1_TPID_CAM, rule, &cam, &valid);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "g3p1 unit %d ppe_tcam_entry_read %d\n"),
                       unit, rv));
            return (rv);
        }

        tpid_data[0] = (tpid->tpid >> 8) & 0xff;
        tpid_data[1] = (tpid->tpid >> 0) & 0xff;
        sal_memcpy(&cam.data[0], &tpid_data[0], 2);

        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "%s on unit %d rule=%d tpid= 0x%02x%02x.\n"),
                     FUNCTION_NAME(), unit, rule, tpid_data[0], tpid_data[1]));

        rv = soc_sbx_caladan3_ppe_tcam_entry_write(unit, G3P1_TPID_CAM, rule, &cam, valid);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "g3p1 unit %d ppe_tcam_entry_write %d\n"),
                       unit, rv));
            return (rv);
        }
    }

    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_g3p1_ppe_entry_tpid_get
 *  Description:
 *     Update PPE rule to get tpid
 *  Inputs:
 *      entry - index of the tpid stack
 *      tpid  - tpid value
 *  Outputs:
 *      one of SOC_E* status types
 */
int
soc_sbx_g3p1_ppe_entry_tpid_get(int unit, uint8 entry, soc_sbx_g3p1_tpid_t *tpid) {

    int rv;
    soc_sbx_caladan3_ppe_tcamdata_t cam;
    int   rule;
    uint8 valid;

    rule = G3P1_TPID_ENTRY_BASE + _rule[entry].entry[0];

    rv = soc_sbx_caladan3_ppe_tcam_entry_read(unit, G3P1_TPID_CAM, rule, &cam, &valid);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d ppe_tcam_entry_read %d\n"),
                   unit, rv));
        return (rv);
    }

    tpid->tpid = ((cam.data[0] << 8) | cam.data[1]);

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s on unit %d rule=%d tpid= 0x%04x.\n"),
                 FUNCTION_NAME(), unit, rule, tpid->tpid));

    return SOC_E_NONE;
}


/*
 *  Routine: soc_sbx_g3p1_ppe_iqsm_arad_init
 *  Description:
 *     Initialize PPE egress rules for ARAD to set shift to 16
 *  Inputs:
 *  Outputs:
 *      one of SOC_E* status types
 */
int soc_sbx_g3p1_ppe_iqsm_arad_init(int unit) {
    uint8 i = 0;
    int rv;
    uint32 tmh_type, tmh_len;
    soc_sbx_caladan3_ppe_iqsm_t isqm;

    if (SOC_SBX_G3P1_ERH_ARAD != soc_sbx_configured_ucode_erh_get(unit)) {
        return SOC_E_NONE;
    }

    rv = soc_sbx_g3p1_htype_tmh_get(unit, &tmh_type);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d soc_sbx_g3p1_htype_tmh_get %d\n"),
                   unit, rv));
        return (rv);
    }
    rv = soc_sbx_g3p1_hlen_tmh_get(unit, &tmh_len);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d soc_sbx_g3p1_hlen_tmh_get %d\n"),
                   unit, rv));
        return (rv);
    }

    for (i = 64; i < 127; i++) {
        rv = soc_sbx_caladan3_ppe_iqsm_entry_read(unit, i, &isqm);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "g3p1 unit %d soc_sbx_caladan3_ppe_iqsm_entry_read %d\n"),
                       unit, rv));
            return (rv);
        }

        isqm.initial_type = tmh_type;
        isqm.shift = tmh_len;

        rv = soc_sbx_caladan3_ppe_iqsm_entry_write(unit, i, &isqm);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "g3p1 unit %d soc_sbx_caladan3_ppe_iqsm_entry_write %d\n"),
                       unit, rv));
            return (rv);
        }
    }

    return SOC_E_NONE;
}

/* 
 * soc_sbx_g3p1_ppe_entry_eerh_set
 * Description:
 *    One of the Egress PPE rule checks  - eerh.hdrcompr.
 *    The location of this bit varies depending upon Arad or Sirius.
 *    So, added two rules one for Arad and one for Sirius. Enable only one of them.
 */
int soc_sbx_g3p1_ppe_entry_eerh_set(int unit) {
    int rv;
    soc_sbx_caladan3_ppe_tcamdata_t cam;
    uint8 valid;
    uint8 camid;
    uint8 ruleid;
    camid = G3P1_EGR_ERH_MPLS_CAM;
    /* Both the rules are enabled by default, disable one of them */    
    if (SOC_SBX_CONTROL(unit)->ucode_erh == SOC_SBX_G3P1_ERH_ARAD) {
         /* rule id to disable */
        ruleid = G3P1_EGR_ERH_MPLS_SIRIUS_ENTRY;
    } else {
        ruleid = G3P1_EGR_ERH_MPLS_ARAD_ENTRY;
    }
    rv = soc_sbx_caladan3_ppe_tcam_entry_read(unit, camid, ruleid, &cam, &valid);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d ppe_tcam_entry_read %d\n"),
                   unit, rv));
        return rv;
    }
    valid = 0;
    rv = soc_sbx_caladan3_ppe_tcam_entry_write(unit, camid, ruleid, &cam, valid);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d ppe_tcam_entry_write %d\n"),
                   unit, rv));
        return rv;
    }
    return SOC_E_NONE;
}

/* 
 * soc_sbx_g3p1_ppe_entry_mc_set
 * Description:
 *    A egress rule detects whether MC bit is set or not.
 *    The MC bit location is different depending on whether it's in HG mode or
 *    Interlaken mode. By default it's set for Interlaken mode. 
 *    If it's in HG mode, call this function to reprogram the entry.
 */
int soc_sbx_g3p1_ppe_entry_mc_set(int unit)
{
  uint32 interlaken_offset;
  int camid = 0;
  int ruleid = 112 /*SOC_CALADAN3_PPE_RULE_rS0_EGR_ERH_MC_ENTRY*/;
  int rv;
  soc_sbx_caladan3_ppe_tcamdata_t cam;
  uint8 valid;

  rv = soc_sbx_g3p1_interlaken_offset_get(unit, &interlaken_offset);
  if (SOC_FAILURE(rv)) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "g3p1 unit %d can't get interlaken_offset %d\n"),
               unit, rv));
    return rv;
  }  
  rv = soc_sbx_caladan3_ppe_tcam_entry_read(unit, camid, ruleid, &cam, &valid);
  if (SOC_FAILURE(rv)) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "g3p1 unit %d ppe_tcam_entry_read %d\n"),
               unit, rv));
    return rv;
  }
  if (interlaken_offset) {
    cam.mask[0] = 0x08;
    cam.mask[1] = 0x00;
    cam.data[0] = 0x08;
    cam.data[1] = 0x00;
  } else {
    cam.mask[0] = 0xFF;
    cam.mask[1] = 0x08;
    cam.data[0] = 0xFB;
    cam.data[1] = 0x08;
  }
  rv = soc_sbx_caladan3_ppe_tcam_entry_write(unit, camid, ruleid, &cam, valid);
  if (SOC_FAILURE(rv)) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "g3p1 unit %d ppe_tcam_entry_write %d\n"),
               unit, rv));
    return rv;
  }
  return SOC_E_NONE;
}


int soc_sbx_g3p1_ppe_init_ext(int unit) {

    int rv;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s on unit %d Handcoded initialization.\n"),
                 FUNCTION_NAME(), unit));

    rv = soc_sbx_g3p1_ppe_entry_lsm_init(unit, G3P1_LSM_CAM, G3P1_LSM_ENTRY_BASE, G3P1_LSM_COUNT);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d soc_sbx_g3p1_ppe_entry_lsm_init %d\n"),
                   unit, rv));
        return (rv);
    }

    rv = soc_sbx_g3p1_elsmac_init(unit, G3P1_ELSM_CAM, G3P1_ELSM_ENTRY_BASE, G3P1_ELSM_COUNT);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d soc_sbx_g3p1_elsmac_init %d\n"),
                   unit, rv));
        return (rv);
    }

    rv = soc_sbx_g3p1_ppe_iqsm_arad_init(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d soc_sbx_g3p1_ppe_iqsm_arad_init %d\n"),
                   unit, rv));
        return (rv);
    }

    soc_sbx_caladan3_ppe_exception_stream_set(unit, G3P1_EXC_STREAM, TRUE);

    rv = soc_sbx_g3p1_ppe_entry_eerh_set(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d soc_sbx_g3p1_ppe_entry_eerh_set %d\n"),
                   unit, rv));
        return (rv);
    }

    rv = soc_sbx_g3p1_ppe_entry_mc_set(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d soc_sbx_g3p1_ppe_entry_mc_set %d\n"),
                   unit, rv));
        return (rv);
    }

    rv = soc_sbx_g3p1_ppe_property_table_init(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d soc_sbx_g3p1_ppe_property_table_init %d\n"),
                   unit, rv));
        return (rv);
    }

    return SOC_E_NONE;
}

#define G3P1_MAX_BUBBLE_IDX (64 * 1024)
/*
 * Function:
 *     soc_sbx_g3p1_bubble_table_init
 * Purpose:
 *     Set up the shadow table for read access due to hardware bug
 */
int soc_sbx_g3p1_bubble_table_init(int unit)
{
    int rv = SOC_E_NONE;
    
    _bubble_shadow_table[unit] = sal_alloc(sizeof(uint32) * (G3P1_MAX_BUBBLE_IDX), "bubble shadow table");
    if (!_bubble_shadow_table[unit]) {
        rv = SOC_E_MEMORY;
    }
    sal_memset(_bubble_shadow_table[unit], 0, (sizeof(uint32) * (G3P1_MAX_BUBBLE_IDX)));
    return rv;
}

/*
 * Function:
 *     soc_sbx_g3p1_bubble_entry_set
 * Purpose:
 *     Set up a timer bubble entry.
 * Notes:
 *     There is a hardware bug in the LRP whereby r/w accesses to the bubble
 *     table from the host are not guaranteed while the bubble table is enabled.
 *     This is because there is a bug in arbitration to the table - the LRP hardware
 *     always wins - and since the LRP can be updating the table constantly (updating the current
 *     time value per enabled entry), the host may not get access.  Because of this 
 *     a workaround is implemented.  The host will update shared ucode register space
 *     with the data to be written to the bubble table and then alert the ucode to 
 *     update the table using a single bubble generated via the LRA_BUBBLE. 
*/
int soc_sbx_g3p1_bubble_entry_set(int unit,  uint32 bubble_idx, uint8 count, uint8 interval_index, 
                                  uint8 task, uint8 stream, uint8 init, uint8 jitter_enable, uint8 update_mode)
{
    int rv = SOC_E_NONE;
    uint32 operation;
    uint32 address;
    uint32 data;

    if (interval_index > 0x7f) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc bubble entry set param error: unit %d interval index(%d) > 127\n"), unit, interval_index));
        return SOC_E_PARAM;
    }    
    if ((task !=0) && (task != 1)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc bubble entry set param error: unit %d task(%d) > 1\n"), unit, task));
        return SOC_E_PARAM;
    } 
    if (stream > 0xf) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc bubble entry set param error: unit %d stream(%d) > 15\n"), unit, stream));
        return SOC_E_PARAM;
    } 
    if ((update_mode > SOC_SBX_G3P1_BUBBLE_UPDATE_MODE_LAST) || (update_mode == SOC_SBX_G3P1_BUBBLE_UPDATE_MODE_RESERVED)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc bubble entry set param error: unit %d mode(%d) out of range or =1 (reserved)\n"), unit, update_mode));
        return SOC_E_PARAM;
    } 
    if ((init !=0) && (init != 1)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc bubble entry set param error: unit %d init(%d) > 1\n"), unit, jitter_enable));
        return SOC_E_PARAM;
    } 
    if ((jitter_enable !=0) && (jitter_enable != 1)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc bubble entry set param error: unit %d jitter_enable(%d) > 1\n"), unit, jitter_enable));
        return SOC_E_PARAM;
    }
    if (bubble_idx > G3P1_MAX_BUBBLE_IDX) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc bubble entry set param error: unit %d bubble_idx(%d) > %d\n"), unit, jitter_enable, G3P1_MAX_BUBBLE_IDX));
        return SOC_E_PARAM;
    }
 
    operation = 0; /* write */
    address = 32;  
    data = bubble_idx;

    rv = soc_sbx_caladan3_lr_shared_register_iaccess(unit, operation, address, &data);
    SOC_IF_ERROR_RETURN(rv);

    operation = 0; /* write */
    address = 33;  
    /* p481 of caladan3_chip_v1.0.pdf Table bubble entry */
    /* 31-------------24 23-------------16 15-------------8 7----------------0 */
    /*        Count             rsvd           intv_idx   T  stream I JE Mode  */
    /* 31-------------24 23-------------16 15-------------8 7----------------0 */

    data = (count << 24) | (interval_index << 9) | (task << 8) | 
           (stream << 4) | (init << 3) | (jitter_enable << 2) | update_mode;

    rv = soc_sbx_caladan3_lr_shared_register_iaccess(unit, operation, address, &data);
    SOC_IF_ERROR_RETURN(rv);

    rv = soc_sbx_caladan3_lr_host_bubble(unit, stream, task, 0xffff);
    SOC_IF_ERROR_RETURN(rv);

    /* update bubble shadow table, if initialized, otherwise ignore */
    if (_bubble_shadow_table[unit]) {
        _bubble_shadow_table[unit][bubble_idx] = data; 
    }
    return rv;
}
int soc_sbx_g3p1_bubble_entry_get(int unit,  uint32 bubble_idx, uint8 *count, uint8 *interval_index, 
                                  uint8 *task, uint8 *stream, uint8 *init, uint8 *jitter_enable, uint8 *update_mode)
{
    int rv = SOC_E_NONE;
    uint32 data;

    if (bubble_idx > G3P1_MAX_BUBBLE_IDX) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc bubble entry set param error: unit %d bubble_idx(%d) > %d\n"), unit, (int)jitter_enable, (int)G3P1_MAX_BUBBLE_IDX));
        return SOC_E_PARAM;
    }
    if (!_bubble_shadow_table[unit]) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc bubble shadow table not initialized, get function not available")));
        return SOC_E_INIT;
    }
    if (!count || !interval_index || !task || !stream || !init || !jitter_enable || !update_mode) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "return parameters not provided")));
        return SOC_E_PARAM;
    }

    /* p481 of caladan3_chip_v1.0.pdf Table bubble entry */
    /* 31-------------24 23-------------16 15-------------8 7----------------0 */
    /*        Count             rsvd           intv_idx   T  stream I JE Mode  */
    /* 31-------------24 23-------------16 15-------------8 7----------------0 */

    
    data              = _bubble_shadow_table[unit][bubble_idx];
    *count            = (uint8)((data & 0xff000000) >> 24);
    *interval_index   = (uint8)((data & 0x0000fe00) >> 9);
    *task             = (uint8)((data & 0x100) >> 8);
    *stream           = (uint8)((data & 0xf0) >> 4);
    *init             = (uint8)((data & 0x8) >> 3);
    *jitter_enable    = (uint8)((data & 0x4) >> 2);
    *update_mode      = (uint8) (data & 3); 
    
    return rv;
}
static void *g3p1_util_timer_cb_cookie;
static soc_sbx_g3p1_util_timer_event_callback_f g3p1_util_timer_cb;

/* Current code only works for a single timer segment   
 * If additional timer segments are required, need callback
 * function from g3p1_cop.c to get the segment id 
 */
static 
void g3p1_util_timer_callback(int unit, int cop) {
    int rv;
    soc_sbx_caladan3_cop_timer_expire_event_t timer_event;
    soc_sbx_g3p1_util_timer_event_t event;
    
    rv = SOC_E_NONE;
    while (!SOC_FAILURE(rv)){
        rv = soc_sbx_caladan3_cop_timer_event_dequeue(unit, cop, &timer_event);
        if (!SOC_FAILURE(rv)) {
            event.timer_segment            = timer_event.uSegment;
            event.id                       = timer_event.uTimer;
            event.forced_timeout           = timer_event.bForced;
            event.timer_active_when_forced = timer_event.bActiveWhenForced;                
            g3p1_util_timer_cb(unit, &event, g3p1_util_timer_cb_cookie);
        } else if (rv != SOC_E_EMPTY) {
            LOG_CLI((BSL_META_U(unit,
                                "unit(%d) dequeue failure of timer event cop(%d)\n"), unit, cop));
        }
    }
}

int soc_sbx_g3p1_util_register_timer_callback(int unit, soc_sbx_g3p1_util_timer_event_callback_f cb, void *user_cookie) 
{
    int rv;

    rv = soc_sbx_caladan3_cop_timer_event_callback_register(unit, g3p1_util_timer_callback);
    if (SOC_FAILURE(rv)) {
        LOG_CLI((BSL_META_U(unit,
                            "C3 %d failed to register util timer event callback\n"), unit));
        return rv;
    }

    g3p1_util_timer_cb = cb;
    g3p1_util_timer_cb_cookie = user_cookie;
    return SOC_E_NONE;
}

static uint32 soc_sbx_g3p1_crc_table[] = {
    0x00000000U, 0x04c11db7U, 0x09823b6eU, 0x0d4326d9U,
    0x130476dcU, 0x17c56b6bU, 0x1a864db2U, 0x1e475005U,
    0x2608edb8U, 0x22c9f00fU, 0x2f8ad6d6U, 0x2b4bcb61U,
    0x350c9b64U, 0x31cd86d3U, 0x3c8ea00aU, 0x384fbdbdU,
    0x4c11db70U, 0x48d0c6c7U, 0x4593e01eU, 0x4152fda9U,
    0x5f15adacU, 0x5bd4b01bU, 0x569796c2U, 0x52568b75U,
    0x6a1936c8U, 0x6ed82b7fU, 0x639b0da6U, 0x675a1011U,
    0x791d4014U, 0x7ddc5da3U, 0x709f7b7aU, 0x745e66cdU,
    0x9823b6e0U, 0x9ce2ab57U, 0x91a18d8eU, 0x95609039U, 
    0x8b27c03cU, 0x8fe6dd8bU, 0x82a5fb52U, 0x8664e6e5U,
    0xbe2b5b58U, 0xbaea46efU, 0xb7a96036U, 0xb3687d81U,
    0xad2f2d84U, 0xa9ee3033U, 0xa4ad16eaU, 0xa06c0b5dU,
    0xd4326d90U, 0xd0f37027U, 0xddb056feU, 0xd9714b49U,
    0xc7361b4cU, 0xc3f706fbU, 0xceb42022U, 0xca753d95U,
    0xf23a8028U, 0xf6fb9d9fU, 0xfbb8bb46U, 0xff79a6f1U,
    0xe13ef6f4U, 0xe5ffeb43U, 0xe8bccd9aU, 0xec7dd02dU,
    0x34867077U, 0x30476dc0U, 0x3d044b19U, 0x39c556aeU,
    0x278206abU, 0x23431b1cU, 0x2e003dc5U, 0x2ac12072U,
    0x128e9dcfU, 0x164f8078U, 0x1b0ca6a1U, 0x1fcdbb16U,
    0x018aeb13U, 0x054bf6a4U, 0x0808d07dU, 0x0cc9cdcaU,
    0x7897ab07U, 0x7c56b6b0U, 0x71159069U, 0x75d48ddeU,
    0x6b93dddbU, 0x6f52c06cU, 0x6211e6b5U, 0x66d0fb02U,
    0x5e9f46bfU, 0x5a5e5b08U, 0x571d7dd1U, 0x53dc6066U,
    0x4d9b3063U, 0x495a2dd4U, 0x44190b0dU, 0x40d816baU,
    0xaca5c697U, 0xa864db20U, 0xa527fdf9U, 0xa1e6e04eU,
    0xbfa1b04bU, 0xbb60adfcU, 0xb6238b25U, 0xb2e29692U,
    0x8aad2b2fU, 0x8e6c3698U, 0x832f1041U, 0x87ee0df6U,
    0x99a95df3U, 0x9d684044U, 0x902b669dU, 0x94ea7b2aU,
    0xe0b41de7U, 0xe4750050U, 0xe9362689U, 0xedf73b3eU,
    0xf3b06b3bU, 0xf771768cU, 0xfa325055U, 0xfef34de2U,
    0xc6bcf05fU, 0xc27dede8U, 0xcf3ecb31U, 0xcbffd686U,
    0xd5b88683U, 0xd1799b34U, 0xdc3abdedU, 0xd8fba05aU,
    0x690ce0eeU, 0x6dcdfd59U, 0x608edb80U, 0x644fc637U,
    0x7a089632U, 0x7ec98b85U, 0x738aad5cU, 0x774bb0ebU,
    0x4f040d56U, 0x4bc510e1U, 0x46863638U, 0x42472b8fU,
    0x5c007b8aU, 0x58c1663dU, 0x558240e4U, 0x51435d53U,
    0x251d3b9eU, 0x21dc2629U, 0x2c9f00f0U, 0x285e1d47U,
    0x36194d42U, 0x32d850f5U, 0x3f9b762cU, 0x3b5a6b9bU,
    0x0315d626U, 0x07d4cb91U, 0x0a97ed48U, 0x0e56f0ffU,
    0x1011a0faU, 0x14d0bd4dU, 0x19939b94U, 0x1d528623U,
    0xf12f560eU, 0xf5ee4bb9U, 0xf8ad6d60U, 0xfc6c70d7U,
    0xe22b20d2U, 0xe6ea3d65U, 0xeba91bbcU, 0xef68060bU,
    0xd727bbb6U, 0xd3e6a601U, 0xdea580d8U, 0xda649d6fU,
    0xc423cd6aU, 0xc0e2d0ddU, 0xcda1f604U, 0xc960ebb3U,
    0xbd3e8d7eU, 0xb9ff90c9U, 0xb4bcb610U, 0xb07daba7U,
    0xae3afba2U, 0xaafbe615U, 0xa7b8c0ccU, 0xa379dd7bU,
    0x9b3660c6U, 0x9ff77d71U, 0x92b45ba8U, 0x9675461fU,
    0x8832161aU, 0x8cf30badU, 0x81b02d74U, 0x857130c3U,
    0x5d8a9099U, 0x594b8d2eU, 0x5408abf7U, 0x50c9b640U,
    0x4e8ee645U, 0x4a4ffbf2U, 0x470cdd2bU, 0x43cdc09cU,
    0x7b827d21U, 0x7f436096U, 0x7200464fU, 0x76c15bf8U,
    0x68860bfdU, 0x6c47164aU, 0x61043093U, 0x65c52d24U,
    0x119b4be9U, 0x155a565eU, 0x18197087U, 0x1cd86d30U,
    0x029f3d35U, 0x065e2082U, 0x0b1d065bU, 0x0fdc1becU,
    0x3793a651U, 0x3352bbe6U, 0x3e119d3fU, 0x3ad08088U,
    0x2497d08dU, 0x2056cd3aU, 0x2d15ebe3U, 0x29d4f654U,
    0xc5a92679U, 0xc1683bceU, 0xcc2b1d17U, 0xc8ea00a0U,
    0xd6ad50a5U, 0xd26c4d12U, 0xdf2f6bcbU, 0xdbee767cU,
    0xe3a1cbc1U, 0xe760d676U, 0xea23f0afU, 0xeee2ed18U,
    0xf0a5bd1dU, 0xf464a0aaU, 0xf9278673U, 0xfde69bc4U,
    0x89b8fd09U, 0x8d79e0beU, 0x803ac667U, 0x84fbdbd0U,
    0x9abc8bd5U, 0x9e7d9662U, 0x933eb0bbU, 0x97ffad0cU,
    0xafb010b1U, 0xab710d06U, 0xa6322bdfU, 0xa2f33668U,
    0xbcb4666dU, 0xb8757bdaU, 0xb5365d03U, 0xb1f740b4U,
};

uint32 soc_sbx_g3p1_util_crc32_word(uint32 x)
{
  uint32 i;

  for (i = 0; i < 4; i++)
      x = (x << 8) ^ soc_sbx_g3p1_crc_table[x >> 24];
  return x;
}


/*
 *  Routine: soc_sbx_g3p1_ppe_queue_psc_set
 *  Description:
 *     Update Queue trunk selection criteria by setting hash template disable state.
 *     soc_sbx_g3p1_ppe_entry_psc_set() set global rules, this function toggle queue
 *     specific template enable/disable state. Overall effect for a perticular queue
 *     is intersection of two PSC settings.
 *  Inputs:
 *      queueid - index of the queue
 *      psc     - trunk selection criteria, see SB_G3P1_PSC_*
 *  Outputs:
 *      one of SOC_E* status types
 */
int soc_sbx_g3p1_ppe_queue_psc_set(int unit, uint8 queueid, uint32 psc)
{
    int rv;
    soc_sbx_caladan3_ppe_iqsm_t iqsm;

    rv = soc_sbx_caladan3_ppe_iqsm_entry_read(unit, queueid, &iqsm);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d soc_sbx_caladan3_ppe_iqsm_entry_read %d\n"),
                   unit, rv));
        return (rv);
    }

    iqsm.hash_template_disable = 0;

    if ((psc & (SB_G3P1_PSC_MAC_SA | SB_G3P1_PSC_MAC_DA)) == 0) {
        /* MAC not set, disable MAC templace */
        iqsm.hash_template_disable |= (1 << SOC_SBX_G3P1_PPE_HASHT_hETH_MAC);
    } else if ((psc & SB_G3P1_PSC_MAC_DA) == 0) {
        /* DMAC not set, disable DMAC template */
        iqsm.hash_template_disable |= (1 << SOC_SBX_G3P1_PPE_HASHT_hETH_DMAC);
    } else if ((psc & SB_G3P1_PSC_MAC_SA) == 0) {
        /* SMAC not set, disable SMAC template */
        iqsm.hash_template_disable |= (1 << SOC_SBX_G3P1_PPE_HASHT_hETH_SMAC);
    }

    if ((psc & SB_G3P1_PSC_VID) && (psc & SB_G3P1_PSC_VID_INNER)) {
        /* VID two set, don't disable templace */
    } else if (psc & SB_G3P1_PSC_VID) {
        /* Outer VID */
        iqsm.hash_template_disable |= (1 << SOC_SBX_G3P1_PPE_HASHT_hVLAN_VID_TWO);
        iqsm.hash_template_disable |= (1 << SOC_SBX_G3P1_PPE_HASHT_hVLAN_VID_C);
    } else if (psc & SB_G3P1_PSC_VID_INNER) {
        /* Inner VID not set, disable VID templace */
        iqsm.hash_template_disable |= (1 << SOC_SBX_G3P1_PPE_HASHT_hVLAN_VID_TWO);
        /* Has Outer VID tempalte for single stack */
    } else {
        /* Disable all VID template */
        iqsm.hash_template_disable |= (1 << SOC_SBX_G3P1_PPE_HASHT_hVLAN_VID_TWO);
        iqsm.hash_template_disable |= (1 << SOC_SBX_G3P1_PPE_HASHT_hVLAN_VID);
        iqsm.hash_template_disable |= (1 << SOC_SBX_G3P1_PPE_HASHT_hVLAN_VID_C);
    }

    if ((psc & (SB_G3P1_PSC_IP_SA | SB_G3P1_PSC_IP_DA)) == 0) {
        /* IP not set, disable IP templace */
        iqsm.hash_template_disable |= (1 << SOC_SBX_G3P1_PPE_HASHT_hIPV4); /* IPv4 */
        iqsm.hash_template_disable |= (1 << SOC_SBX_G3P1_PPE_HASHT_hIPV6); /* IPv6 */
    } else if ((psc & SB_G3P1_PSC_IP_DA) == 0) {
        /* DIP not set, disable DIP template */
        iqsm.hash_template_disable |= (1 << SOC_SBX_G3P1_PPE_HASHT_hIPV4_DA); /* IPv4 */
        iqsm.hash_template_disable |= (1 << SOC_SBX_G3P1_PPE_HASHT_hIPV6_DA); /* IPv6 */
    } else if ((psc & SB_G3P1_PSC_IP_SA) == 0) {
        /* SIP not set, disable SIP template */
        iqsm.hash_template_disable |= (1 << SOC_SBX_G3P1_PPE_HASHT_hIPV4_SA); /* IPv4 */
        iqsm.hash_template_disable |= (1 << SOC_SBX_G3P1_PPE_HASHT_hIPV4_SA); /* IPv6 */
    }

    if ((psc & (SB_G3P1_PSC_L4SS | SB_G3P1_PSC_L4DS)) == 0) {
        /* Port not set, disable port templace */
        iqsm.hash_template_disable |= (1 << SOC_SBX_G3P1_PPE_HASHT_hL4_PORT);
    } else if ((psc & SB_G3P1_PSC_L4DS) == 0) {
        /* dport not set, disable dport template */
        iqsm.hash_template_disable |= (1 << SOC_SBX_G3P1_PPE_HASHT_hL4_DPORT);
    } else if ((psc & SB_G3P1_PSC_L4SS) == 0) {
        /* sport not set, disable sport template */
        iqsm.hash_template_disable |= (1 << SOC_SBX_G3P1_PPE_HASHT_hL4_SPORT);
    }

    rv = soc_sbx_caladan3_ppe_iqsm_entry_write(unit, queueid, &iqsm);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d soc_sbx_caladan3_ppe_iqsm_entry_write %d\n"),
                   unit, rv));
    }

    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_g3p1_ppe_queue_psc_get
 *  Description:
 *     Get Queue trunk selection criteria.
 *     soc_sbx_g3p1_ppe_entry_psc_get() get global rules, this function get queue
 *     specific template enable/disable state.
 *  Inputs:
 *      queueid - index of the queue
 *      psc     - trunk selection criteria, see SB_G3P1_PSC_*
 *  Outputs:
 *      one of SOC_E* status types
 */
int soc_sbx_g3p1_ppe_queue_psc_get(int unit, uint8 queueid, uint32* psc)
{
    int rv;
    soc_sbx_caladan3_ppe_iqsm_t iqsm;

    *psc = 0;

    rv = soc_sbx_caladan3_ppe_iqsm_entry_read(unit, queueid, &iqsm);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d soc_sbx_caladan3_ppe_iqsm_entry_read %d\n"),
                   unit, rv));
        return (rv);
    }

    if ((iqsm.hash_template_disable & (1 << SOC_SBX_G3P1_PPE_HASHT_hETH_MAC)) == 0) {
        /* MAC templace */
        *psc |= (SB_G3P1_PSC_MAC_SA | SB_G3P1_PSC_MAC_DA);
    } else if ((iqsm.hash_template_disable & (1 << SOC_SBX_G3P1_PPE_HASHT_hETH_DMAC)) == 0) {
        /* DMAC template */
        *psc |= (SB_G3P1_PSC_MAC_DA);
    } else if ((iqsm.hash_template_disable & (1 << SOC_SBX_G3P1_PPE_HASHT_hETH_SMAC)) == 0) {
        /* SMAC template */
        *psc |= (SB_G3P1_PSC_MAC_SA);
    }

    if ((iqsm.hash_template_disable & (1 << SOC_SBX_G3P1_PPE_HASHT_hVLAN_VID_TWO)) == 0) {
        /* VID double templace */
        *psc |= (SB_G3P1_PSC_VID | SB_G3P1_PSC_VID_INNER);
    } else if ((iqsm.hash_template_disable & (1 << SOC_SBX_G3P1_PPE_HASHT_hVLAN_VID)) == 0) {
        /* VID single template */
        *psc |= (SB_G3P1_PSC_VID);
    } else if ((iqsm.hash_template_disable & (1 << SOC_SBX_G3P1_PPE_HASHT_hVLAN_VID_C)) == 0) {
        /* VID inner template */
        *psc |= (SB_G3P1_PSC_VID_INNER);
    }

    if ((iqsm.hash_template_disable & (1 << SOC_SBX_G3P1_PPE_HASHT_hIPV4)) == 0) {
        /* IP templace */
        *psc |= (SB_G3P1_PSC_IP_SA | SB_G3P1_PSC_IP_DA);
    } else if ((iqsm.hash_template_disable & (1 << SOC_SBX_G3P1_PPE_HASHT_hIPV4_DA)) == 0) {
        /* DIP template */
        *psc |= (SB_G3P1_PSC_IP_DA);
    } else if ((iqsm.hash_template_disable & (1 << SOC_SBX_G3P1_PPE_HASHT_hIPV4_SA)) == 0) {
        /* SIP template */
        *psc |= (SB_G3P1_PSC_IP_SA);
    }

    if ((iqsm.hash_template_disable & (1 << SOC_SBX_G3P1_PPE_HASHT_hL4_PORT)) == 0) {
        /* port templace */
        *psc |= (SB_G3P1_PSC_L4DS | SB_G3P1_PSC_L4SS);
    } else if ((iqsm.hash_template_disable & (1 << SOC_SBX_G3P1_PPE_HASHT_hL4_DPORT)) == 0) {
        /* dport template */
        *psc |= (SB_G3P1_PSC_L4DS);
    } else if ((iqsm.hash_template_disable & (1 << SOC_SBX_G3P1_PPE_HASHT_hL4_SPORT)) == 0) {
        /* sport template */
        *psc |= (SB_G3P1_PSC_L4SS);
    }

    return SOC_E_NONE;
}

/* Batch set PPE rule hash template */
static int _soc_sbx_g3p1_ppe_entry_hasher_set(int unit, g3p1_rule_id_t* rules, int count, int template)
{
    int rv, i;
    soc_sbx_caladan3_ppe_camram_t camram;

    for (i = 0; i < count; i++) {
        rv = soc_sbx_caladan3_ppe_camram_entry_read(unit, rules[i].stage, rules[i].entry, &camram);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "g3p1 unit %d soc_sbx_caladan3_ppe_camram_entry_read %d\n"),
                       unit, rv));
            return (rv);
        }
        if ((template != -1 && (camram.hasher.enable != 1 || camram.hasher.hash_id != template))
            || (template == -1 && camram.hasher.enable !=0)) {
            if (template != -1) {
                camram.hasher.hash_id = template;
                camram.hasher.enable = 1;
            } else {
                camram.hasher.enable = 0;
            }
            rv = soc_sbx_caladan3_ppe_camram_entry_write(unit, rules[i].stage, rules[i].entry, &camram);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "g3p1 unit %d soc_sbx_caladan3_ppe_camram_entry_write %d\n"),
                           unit, rv));
                return (rv);
            }
        }
    }

    return SOC_E_NONE;
}


/* Set ext for p2e */
int
soc_sbx_g3p1_p2e_set_ext(int unit, int iport  , soc_sbx_g3p1_p2e_t *e)
{
    e->port = iport;
    e->portid = iport;

    if (SAL_BOOT_BCMSIM) {
        return(soc_sbx_g3p1_p2e_set_ext_sim(unit, iport  , e));
    }
    else {
      return (soc_sbx_g3p1_ppe_entry_p2e_set(unit, iport  , e));
    }
}

/* Get ext for p2e */
int
soc_sbx_g3p1_p2e_get_ext(int unit, int iport  , soc_sbx_g3p1_p2e_t *e)
{
    if (SAL_BOOT_BCMSIM) {
        return(soc_sbx_g3p1_p2e_get_ext_sim(unit, iport  , e));
    }
    else {
      return (soc_sbx_g3p1_ppe_entry_p2e_get(unit, iport  , e));
    }
} 


/* Set ext for ep2e */
int
soc_sbx_g3p1_ep2e_set_ext(int unit, int iport  , soc_sbx_g3p1_ep2e_t *e)
{
    e->port = iport;
    e->portid = iport;

    if (SAL_BOOT_BCMSIM) {
        return(soc_sbx_g3p1_ep2e_set_ext_sim(unit, iport  , e));
    }
    else {
      return (soc_sbx_g3p1_ppe_entry_ep2e_set(unit, iport  , e));
    }
}

/* Get ext for ep2e */
int
soc_sbx_g3p1_ep2e_get_ext(int unit, int iport  , soc_sbx_g3p1_ep2e_t *e)
{
    if (SAL_BOOT_BCMSIM) {
        return(soc_sbx_g3p1_ep2e_get_ext_sim(unit, iport  , e));
    }
    else {
      return (soc_sbx_g3p1_ppe_entry_ep2e_get(unit, iport  , e));
    }
} 


/* Set ext for tpid */
int
soc_sbx_g3p1_tpid_set_ext(int unit, int itpidi  , soc_sbx_g3p1_tpid_t *e)
{
    if (SAL_BOOT_BCMSIM) {
        return(soc_sbx_g3p1_tpid_set_ext_sim(unit, itpidi  , e));
    }
    else {
      return (soc_sbx_g3p1_ppe_entry_tpid_set(unit, itpidi  , e));
    }
}

/* Get ext for tpid */
int
soc_sbx_g3p1_tpid_get_ext(int unit, int itpidi  , soc_sbx_g3p1_tpid_t *e)
{
    if (SAL_BOOT_BCMSIM) {
        return(soc_sbx_g3p1_tpid_get_ext_sim(unit, itpidi  , e));
    }
    else {
      return (soc_sbx_g3p1_ppe_entry_tpid_get(unit, itpidi  , e));
    }
} 

/* Set ext for oam_rx */
int
soc_sbx_g3p1_oam_rx_set_ext(int unit, int irulenum  , soc_sbx_g3p1_oam_rx_t *e)
{
    if (SAL_BOOT_BCMSIM) {
        return(soc_sbx_g3p1_oam_rx_set_ext_sim(unit, irulenum  , e));
    }
    else {
      return (soc_sbx_g3p1_ppe_entry_oam_rx_set(unit, irulenum  , e));
    }
}

/* Get ext for oam_rx */
int
soc_sbx_g3p1_oam_rx_get_ext(int unit, int irulenum  , soc_sbx_g3p1_oam_rx_t *e)
{
    if (SAL_BOOT_BCMSIM) {
        return(soc_sbx_g3p1_oam_rx_get_ext_sim(unit, irulenum  , e));
    }
    else {
      return (soc_sbx_g3p1_ppe_entry_oam_rx_get(unit, irulenum  , e));
    }
} 

/* Set ext for oam_tx */
int
soc_sbx_g3p1_oam_tx_set_ext(int unit, int irulenum  , soc_sbx_g3p1_oam_tx_t *e)
{
    if (SAL_BOOT_BCMSIM) {
        return(soc_sbx_g3p1_oam_tx_set_ext_sim(unit, irulenum  , e));
    }
    else {
      return (soc_sbx_g3p1_ppe_entry_oam_tx_set(unit, irulenum  , e));
    }
}

/* Get ext for oam_tx */
int
soc_sbx_g3p1_oam_tx_get_ext(int unit, int irulenum  , soc_sbx_g3p1_oam_tx_t *e)
{
    if (SAL_BOOT_BCMSIM) {
        return(soc_sbx_g3p1_oam_tx_get_ext_sim(unit, irulenum  , e));
    }
    else {
      return (soc_sbx_g3p1_ppe_entry_oam_tx_get(unit, irulenum  , e));
    }
} 


/* Set ext for lsmac */
int
soc_sbx_g3p1_lsmac_set_ext(int unit, int ilsi  , soc_sbx_g3p1_lsmac_t *e)
{
    if (SAL_BOOT_BCMSIM) {
        return(soc_sbx_g3p1_lsmac_set_ext_sim(unit, ilsi  , e));
    }
    else {
      return (soc_sbx_g3p1_ppe_entry_lsmac_set(unit, ilsi  , e));
    }
}

/* Get ext for lsmac */
int
soc_sbx_g3p1_lsmac_get_ext(int unit, int ilsi  , soc_sbx_g3p1_lsmac_t *e)
{
    if (SAL_BOOT_BCMSIM) {
        return(soc_sbx_g3p1_lsmac_get_ext_sim(unit, ilsi  , e));
    }
    else {
      return (soc_sbx_g3p1_ppe_entry_lsmac_get(unit, ilsi  , e));
    }
} 

/* Set ext for l2cpmac */
int
soc_sbx_g3p1_l2cpmac_set_ext(int unit, int ilsi  , soc_sbx_g3p1_l2cpmac_t *e)
{
    if (SAL_BOOT_BCMSIM) {
        return(soc_sbx_g3p1_l2cpmac_set_ext_sim(unit, ilsi  , e));
    }
    else {
      return (soc_sbx_g3p1_ppe_entry_l2cpmac_set(unit, ilsi  , e));
    }
}

/* Get ext for l2cpmac */
int
soc_sbx_g3p1_l2cpmac_get_ext(int unit, int ilsi  , soc_sbx_g3p1_l2cpmac_t *e)
{
    if (SAL_BOOT_BCMSIM) {
        return(soc_sbx_g3p1_l2cpmac_get_ext_sim(unit, ilsi  , e));
    }
    else {
      return (soc_sbx_g3p1_ppe_entry_l2cpmac_get(unit, ilsi  , e));
    }
} 

/* Set ext for elsmac */
int
soc_sbx_g3p1_elsmac_set_ext(int unit, int ilsi  , soc_sbx_g3p1_elsmac_t *e)
{
    if (SAL_BOOT_BCMSIM) {
        return(soc_sbx_g3p1_elsmac_set_ext_sim(unit, ilsi  , e));
    }
    else {
      return (soc_sbx_g3p1_ppe_entry_elsmac_set(unit, ilsi  , e));
    }
}

/* Get ext for elsmac */
int
soc_sbx_g3p1_elsmac_get_ext(int unit, int ilsi  , soc_sbx_g3p1_elsmac_t *e)
{
    if (SAL_BOOT_BCMSIM) {
        return(soc_sbx_g3p1_elsmac_get_ext_sim(unit, ilsi  , e));
    }
    else {
      return (soc_sbx_g3p1_ppe_entry_elsmac_get(unit, ilsi  , e));
    }
} 


/*
 *  Routine: soc_sbx_g3p1_ppe_entry_psc_set
 *  Description:
 *     Update PPE entry trunk selection criteria
 *  Inputs:
 *      psc     - trunk selection criteria, see SB_G3P1_PSC_*
 *  Outputs:
 *      one of SOC_E* status types
 */
int soc_sbx_g3p1_ppe_entry_psc_set(int unit, uint32 psc)
{
    int i;
    int rv;
    int template = 0;
    int template1 = 0;
    soc_sbx_caladan3_ppe_iqsm_t iqsm;

    /* MAC Address */
    if (psc & SB_G3P1_PSC_MAC_SA && psc & SB_G3P1_PSC_MAC_DA) {
        template = SOC_SBX_G3P1_PPE_HASHT_hETH_MAC;
    } else if (psc & SB_G3P1_PSC_MAC_DA) {
        template = SOC_SBX_G3P1_PPE_HASHT_hETH_DMAC;
    } else if (psc & SB_G3P1_PSC_MAC_SA) {
        template = SOC_SBX_G3P1_PPE_HASHT_hETH_SMAC;
    } else {
        template = -1;
    }

    /* Replace all mac template MAC|DMAC|SMAC with tempalte
     * MAC template only exists in PP_IQSM */
    for (i = 0; i < 127; i++) {
        rv = soc_sbx_caladan3_ppe_iqsm_entry_read(unit, i, &iqsm);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "g3p1 unit %d soc_sbx_caladan3_ppe_iqsm_entry_read %d\n"),
                       unit, rv));
            return (rv);
        }
        if (iqsm.hasher.hash_id >= SOC_SBX_G3P1_PPE_HASHT_hETH_MAC && iqsm.hasher.hash_id <= SOC_SBX_G3P1_PPE_HASHT_hETH_SMAC /* is MAC tempalte */
            && ((template != -1 && (iqsm.hasher.enable == 0 || iqsm.hasher.hash_id != template))
                || (template == -1 && iqsm.hasher.enable == 1))) {
            if (template != -1) {
                iqsm.hasher.hash_id = template;
                iqsm.hasher.enable = 1;
            } else {
                iqsm.hasher.enable = 0;
            }
            rv = soc_sbx_caladan3_ppe_iqsm_entry_write(unit, i, &iqsm);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "g3p1 unit %d soc_sbx_caladan3_ppe_iqsm_entry_write %d\n"),
                           unit, rv));
                return (rv);
            }
        }
    }


    /* VID */
    if (psc & SB_G3P1_PSC_VID && psc & SB_G3P1_PSC_VID_INNER) {
        template = SOC_SBX_G3P1_PPE_HASHT_hVLAN_VID_TWO;
        template1 = SOC_SBX_G3P1_PPE_HASHT_hVLAN_VID;
    } else if (psc & SB_G3P1_PSC_VID) {
        template = SOC_SBX_G3P1_PPE_HASHT_hVLAN_VID;
        template1 = SOC_SBX_G3P1_PPE_HASHT_hVLAN_VID;
    } else if (psc & SB_G3P1_PSC_VID_INNER) {
        template = SOC_SBX_G3P1_PPE_HASHT_hVLAN_VID_C;
        template1 = SOC_SBX_G3P1_PPE_HASHT_hVLAN_VID;
    } else {
        template = -1;
        template1 = -1;
    }

    /* Toggle all VID hash templates */
    if (sizeof(vid_two_rules) > 0) {
        rv = _soc_sbx_g3p1_ppe_entry_hasher_set(unit, vid_two_rules, COUNTOF(vid_two_rules), template);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "g3p1 unit %d _soc_sbx_g3p1_ppe_entry_hasher_set %d\n"),
                       unit, rv));
            return (rv);
        }
    }
    if (sizeof(vid_one_rules) > 0) {
        rv = _soc_sbx_g3p1_ppe_entry_hasher_set(unit, vid_one_rules, COUNTOF(vid_one_rules), template1);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "g3p1 unit %d _soc_sbx_g3p1_ppe_entry_hasher_set %d\n"),
                       unit, rv));
            return (rv);
        }
    }

    /* IP Address */
    if (psc & SB_G3P1_PSC_IP_SA && psc & SB_G3P1_PSC_IP_DA) {
        template = SOC_SBX_G3P1_PPE_HASHT_hIPV4; /* IPv4 */
        template1 = SOC_SBX_G3P1_PPE_HASHT_hIPV6; /* IPv6 */
    } else if (psc & SB_G3P1_PSC_IP_DA) {
        template = SOC_SBX_G3P1_PPE_HASHT_hIPV4_DA; /* IPv4 */
        template1 = SOC_SBX_G3P1_PPE_HASHT_hIPV6_DA; /* IPv6 */
    } else if (psc & SB_G3P1_PSC_IP_SA) {
        template = SOC_SBX_G3P1_PPE_HASHT_hIPV4_SA; /* IPv4 */
        template1 = SOC_SBX_G3P1_PPE_HASHT_hIPV6_SA; /* IPv6 */
    } else {
        template = -1; /* IPv4 */
        template1 = -1; /* IPv6 */
    }

    if (sizeof(ipv4_rules) > 0) {
        /* Toggle all IPv4 hash templates */
        rv = _soc_sbx_g3p1_ppe_entry_hasher_set(unit, ipv4_rules, COUNTOF(ipv4_rules), template);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "g3p1 unit %d _soc_sbx_g3p1_ppe_entry_hasher_set %d\n"),
                       unit, rv));
            return (rv);
        }
    }

    if (sizeof(ipv6_rules) > 0) {
        /* Toggle all IPv6 hash templates */
        rv = _soc_sbx_g3p1_ppe_entry_hasher_set(unit, ipv6_rules, COUNTOF(ipv6_rules), template1);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "g3p1 unit %d _soc_sbx_g3p1_ppe_entry_hasher_set %d\n"),
                       unit, rv));
            return (rv);
        }
    }

    /* IP Port */
    if (sizeof(port_rules) > 0) {
        if (psc & SB_G3P1_PSC_L4SS && psc & SB_G3P1_PSC_L4DS) {
            template = SOC_SBX_G3P1_PPE_HASHT_hL4_PORT;
        } else if (psc & SB_G3P1_PSC_L4DS) {
            template = SOC_SBX_G3P1_PPE_HASHT_hL4_DPORT;
        } else if (psc & SB_G3P1_PSC_L4SS) {
            template = SOC_SBX_G3P1_PPE_HASHT_hL4_SPORT;
        } else {
            template = -1;
        }

        /* Toggle all port hash templates */
        rv = _soc_sbx_g3p1_ppe_entry_hasher_set(unit, port_rules, COUNTOF(port_rules), template);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "g3p1 unit %d _soc_sbx_g3p1_ppe_entry_hasher_set %d\n"),
                       unit, rv));
            return (rv);
        }
    }

    return SOC_E_NONE;
}

static void _hash_template_to_psc(int tempalte, uint32* psc)
{
    switch (tempalte) {
    case SOC_SBX_G3P1_PPE_HASHT_hETH_MAC:
        *psc |= (SB_G3P1_PSC_MAC_SA | SB_G3P1_PSC_MAC_DA);
        break;
    case SOC_SBX_G3P1_PPE_HASHT_hETH_DMAC:
        *psc |= SB_G3P1_PSC_MAC_DA;
        break;
    case SOC_SBX_G3P1_PPE_HASHT_hETH_SMAC:
        *psc |= SB_G3P1_PSC_MAC_SA;
        break;
    case SOC_SBX_G3P1_PPE_HASHT_hVLAN_VID_TWO:
        *psc |= (SB_G3P1_PSC_VID | SB_G3P1_PSC_VID_INNER);
        break;
    case SOC_SBX_G3P1_PPE_HASHT_hVLAN_VID:
        *psc |= SB_G3P1_PSC_VID;
        break;
    case SOC_SBX_G3P1_PPE_HASHT_hVLAN_VID_C:
        *psc |= SB_G3P1_PSC_VID_INNER;
        break;
    case SOC_SBX_G3P1_PPE_HASHT_hIPV4:
        *psc |= (SB_G3P1_PSC_IP_SA | SB_G3P1_PSC_IP_DA);
        break;
    case SOC_SBX_G3P1_PPE_HASHT_hIPV4_DA:
        *psc |= SB_G3P1_PSC_IP_DA;
        break;
    case SOC_SBX_G3P1_PPE_HASHT_hIPV4_SA:
        *psc |= SB_G3P1_PSC_IP_SA;
        break;
    case SOC_SBX_G3P1_PPE_HASHT_hL4_PORT:
        *psc |= (SB_G3P1_PSC_L4SS | SB_G3P1_PSC_L4DS);
        break;
    case SOC_SBX_G3P1_PPE_HASHT_hL4_DPORT:
        *psc |= SB_G3P1_PSC_L4DS;
        break;
    case SOC_SBX_G3P1_PPE_HASHT_hL4_SPORT:
        *psc |= SB_G3P1_PSC_L4SS;
        break;
    default:
        break;
    }

}

/*
 *  Routine: soc_sbx_g3p1_ppe_entry_psc_get
 *  Description:
 *     Get PPE entry trunk selection criteria
 *  Inputs:
 *      psc     - trunk selection criteria, see SB_G3P1_PSC_*
 *  Outputs:
 *      one of SOC_E* status types
 */
int soc_sbx_g3p1_ppe_entry_psc_get(int unit, uint32* psc)
{
    int rv;
    soc_sbx_caladan3_ppe_iqsm_t iqsm;
    soc_sbx_caladan3_ppe_camram_t camram;

    *psc = 0;

    /* Check hash template in PP_IQSM */
    rv = soc_sbx_caladan3_ppe_iqsm_entry_read(unit, 0, &iqsm);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d soc_sbx_caladan3_ppe_iqsm_entry_read %d\n"),
                   unit, rv));
        return (rv);
    }
    if (iqsm.hasher.enable) {
        _hash_template_to_psc(iqsm.hasher.hash_id, psc);
    }

    /* VID */
    if (sizeof(vid_two_rules) > 0) {
        rv = soc_sbx_caladan3_ppe_camram_entry_read(unit, vid_two_rules[0].stage, vid_two_rules[0].entry, &camram);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "g3p1 unit %d soc_sbx_caladan3_ppe_camram_entry_read %d\n"),
                       unit, rv));
            return (rv);
        }
        if (camram.hasher.enable) {
            _hash_template_to_psc(camram.hasher.hash_id, psc);
        }
    }

    /* IP Address */
    if (sizeof(ipv4_rules) > 0) {
        rv = soc_sbx_caladan3_ppe_camram_entry_read(unit, ipv4_rules[0].stage, ipv4_rules[0].entry, &camram);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "g3p1 unit %d soc_sbx_caladan3_ppe_camram_entry_read %d\n"),
                       unit, rv));
            return (rv);
        }
        if (camram.hasher.enable) {
            _hash_template_to_psc(camram.hasher.hash_id, psc);
        }
    }

    /* IP Port */
    if (sizeof(port_rules) > 0) {
        rv = soc_sbx_caladan3_ppe_camram_entry_read(unit, port_rules[0].stage, port_rules[0].entry, &camram);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "g3p1 unit %d soc_sbx_caladan3_ppe_camram_entry_read %d\n"),
                       unit, rv));
            return (rv);
        }
        if (camram.hasher.enable) {
            _hash_template_to_psc(camram.hasher.hash_id, psc);
        }
    }

    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_g3p1_iqsm_stream_get
 *  Description:
 *  Inputs:
 *      queueid - index of the queue
 *  Outputs:
 *      one of SOC_E* status types
 */
int soc_sbx_g3p1_iqsm_stream_get(int unit, uint8 queueid, uint32 *str)
{
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_ppe_iqsm_t iqsm;

    rv = soc_sbx_caladan3_ppe_iqsm_entry_read(unit, queueid, &iqsm);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d soc_sbx_caladan3_ppe_iqsm_entry_read %d\n"),
                   unit, rv));
        return (rv);
    }
    *str = iqsm.initial_stream;

    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_g3p1_iqsm_stream_set
 *  Description:
 *  Inputs:
 *      queueid - index of the queue
 *  Outputs:
 *      one of SOC_E* status types
 */
int soc_sbx_g3p1_iqsm_stream_set(int unit, uint8 queueid, uint32 str)
{
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_ppe_iqsm_t iqsm;

    rv = soc_sbx_caladan3_ppe_iqsm_entry_read(unit, queueid, &iqsm);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d soc_sbx_caladan3_ppe_iqsm_entry_read %d\n"),
                   unit, rv));
        return (rv);
    }
    iqsm.initial_stream = str;
    rv = soc_sbx_caladan3_ppe_iqsm_entry_write(unit, queueid, &iqsm);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d soc_sbx_caladan3_ppe_iqsm_entry_read %d\n"),
                   unit, rv));
        return (rv);
    }

    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_g3p1_iqsm_checker_get
 *  Description:
 *  Inputs:
 *      queueid - index of the queue
 *  Outputs:
 *      one of SOC_E* status types
 */
int soc_sbx_g3p1_iqsm_checker_get(int unit, uint8 queueid, uint32 *checker)
{
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_ppe_iqsm_t iqsm;

    rv = soc_sbx_caladan3_ppe_iqsm_entry_read(unit, queueid, &iqsm);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d soc_sbx_caladan3_ppe_iqsm_entry_read %d\n"),
                   unit, rv));
        return (rv);
    }
    *checker = iqsm.checker.enable;

    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_g3p1_iqsm_checker_set
 *  Description:
 *  Inputs:
 *      queueid - index of the queue
 *  Outputs:
 *      one of SOC_E* status types
 */
int soc_sbx_g3p1_iqsm_checker_set(int unit, uint8 queueid, uint32 checker)
{
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_ppe_iqsm_t iqsm;

    rv = soc_sbx_caladan3_ppe_iqsm_entry_read(unit, queueid, &iqsm);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d soc_sbx_caladan3_ppe_iqsm_entry_read %d\n"),
                   unit, rv));
        return (rv);
    }
    iqsm.checker.enable = checker;
    rv = soc_sbx_caladan3_ppe_iqsm_entry_write(unit, queueid, &iqsm);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d soc_sbx_caladan3_ppe_iqsm_entry_read %d\n"),
                   unit, rv));
        return (rv);
    }

    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_g3p1_mac_bulk_delete 
 *  Description:
 *     Bulk delete of MAC table based on filter
 *  Inputs:
 *      key and key mask - MAC Address and VLAN
 *      value and value mask - MAC Associated Data
 *  Outputs:
 *      one of SOC_E* status types
 */
int
soc_sbx_g3p1_mac_bulk_delete(int unit, uint32 *filter_key,
                                       uint32 *filter_key_mask,
                                       uint32 *filter_value,
                                       uint32 *filter_value_mask) {

    int rv;

    soc_sbx_g3p1_state_t *fe =
        (soc_sbx_g3p1_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g3p1_tmu_table_manager_t *tm = fe->tmu_mgr;
    soc_sbx_tmu_hash_handle_t handle = (soc_sbx_tmu_hash_handle_t)
        tm->tables[SOC_SBX_G3P1_TMU_MAC_TABLE_ID].handle;
    
    rv = soc_sbx_caladan3_tmu_hash_bulk_delete(unit,
                                               handle,
                                               filter_key,
                                               filter_key_mask,
                                               filter_value,
                                               filter_value_mask);

    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "g3p1 unit %d soc_sbx_g3p1_mac_bulk_delete %d\n"),
                   unit, rv));
        return (rv);
    }

    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_g3p1_ppe_entry_p2e_get
 *  Description:
 *     
 *  Inputs:
 *      
 *      
 *  Outputs:
 *      
 */
int soc_sbx_g3p1_ppe_entry_p2e_get(int unit, int iport, soc_sbx_g3p1_p2e_t *e)
{
    uint32 data[16];
    soc_sbx_caladan3_ppe_iqsm_t iqsm;
    soc_sbx_caladan3_ppe_sqdm_t sqdm;
    uint8 *dp, *sp;
    int queue;
    int s = SOC_E_NONE;
    soc_sbx_g3p1_state_t *fe =
        (soc_sbx_g3p1_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g3p1_ppe_table_manager_t *tm = fe->ppe_mgr;
    soc_sbx_g3p1_entry_desc_t *ed =
        &tm->entries[SOC_SBX_G3P1_PPE_P2E_ID];
   
    
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
    s = soc_sbx_g3p1_p2e_unpack(unit, e, dp, ed->epsize);
    if (SOC_SUCCESS(s)) {
        e->port = iport;
        e->portid = iport;
        e->hdrtype = iqsm.initial_type;
    }
    
    return s;
}

/*
 *  Routine: soc_sbx_g3p1_ppe_entry_p2e_set
 *  Description:
 *     
 *  Inputs:
 *      
 *      
 *  Outputs:
 *      
 */
int soc_sbx_g3p1_ppe_entry_p2e_set(int unit, int iport, soc_sbx_g3p1_p2e_t *e)
{

    uint32 data[16];
    soc_sbx_caladan3_ppe_iqsm_t iqsm;
    soc_sbx_caladan3_ppe_sqdm_t sqdm;
    uint8 *dp, *sp;
    int queue;
    int s = SOC_E_NONE;
    soc_sbx_g3p1_state_t *fe =
        (soc_sbx_g3p1_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g3p1_ppe_table_manager_t *tm = fe->ppe_mgr;
    soc_sbx_g3p1_entry_desc_t *ed =
        &tm->entries[SOC_SBX_G3P1_PPE_P2E_ID];

    s = soc_sbx_caladan3_get_squeue_from_port(unit, iport, 0, 0, &queue);
    if (SOC_FAILURE(s)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "P2E entry set: port to queue get failed %d, for port %d\n"), s, iport));
        return s;
    }
    
       
    dp = (uint8*)&data[0];
    sp = (uint8*)&sqdm;
    sal_memset(&sqdm, 0, sizeof(sqdm));
    sal_memset(&iqsm, 0, sizeof(iqsm));
    sal_memset(data, 0, sizeof(data));
    s = soc_sbx_g3p1_p2e_pack(unit, e, dp, ed->epsize);
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
 *  Routine: soc_sbx_g3p1_ppe_entry_ep2e_set
 *  Description:
 *     
 *  Inputs:
 *      
 *      
 *  Outputs:
 *      
 */
int soc_sbx_g3p1_ppe_entry_ep2e_set(int unit, int iport, soc_sbx_g3p1_ep2e_t *e)
{
    uint32 data[16];
    soc_sbx_caladan3_ppe_iqsm_t iqsm;
    soc_sbx_caladan3_ppe_sqdm_t sqdm;
    uint8 *dp, *sp;
    int queue;
    int s = SOC_E_NONE;
    soc_sbx_g3p1_state_t *fe = (soc_sbx_g3p1_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g3p1_ppe_table_manager_t *tm = fe->ppe_mgr;
    soc_sbx_g3p1_entry_desc_t *ed = &tm->entries[SOC_SBX_G3P1_PPE_EP2E_ID];

    s = soc_sbx_caladan3_get_squeue_from_port(unit, iport, 1, 0, &queue);
    if (SOC_FAILURE(s)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "EP2E set: port 2 queue get failed %d for port %d\n"), s, iport));
        return s;
    }
            
    dp = (uint8*)&data[0];
    sp = (uint8*)&sqdm;
    
    sal_memset(&sqdm, 0, sizeof(sqdm));
    sal_memset(&iqsm, 0, sizeof(iqsm));
    sal_memset(data, 0, sizeof(data));
    s = soc_sbx_g3p1_ep2e_pack(unit, e, dp, ed->epsize);
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
 *  Routine: soc_sbx_g3p1_ppe_entry_ep2e_get
 *     
 *  Inputs:
 *      
 *      
 *  Outputs:
 *      
 */
int soc_sbx_g3p1_ppe_entry_ep2e_get(int unit, int iport, soc_sbx_g3p1_ep2e_t *e)
{
    uint32 data[16];
    soc_sbx_caladan3_ppe_iqsm_t iqsm;
    soc_sbx_caladan3_ppe_sqdm_t sqdm;
    uint8 *dp, *sp;
    int queue = 0;
    int s = SOC_E_NONE;
    soc_sbx_g3p1_state_t *fe = (soc_sbx_g3p1_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g3p1_ppe_table_manager_t *tm = fe->ppe_mgr;
    soc_sbx_g3p1_entry_desc_t *ed =
        &tm->entries[SOC_SBX_G3P1_PPE_EP2E_ID];
    
    
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
    s = soc_sbx_g3p1_ep2e_unpack(unit, e, dp, ed->epsize);
    if (SOC_SUCCESS(s)) {
        e->port = iport;
        e->hdrtype = iqsm.initial_type;
    }
    
    return s;
}

/*
 *  Routine: soc_sbx_g3p1_ppe_entry_oam_rx_set
 *     
 *  Inputs:
 *      
 *      
 *  Outputs:
 *      
 */
int soc_sbx_g3p1_ppe_entry_oam_rx_set(int unit, int irulenum , soc_sbx_g3p1_oam_rx_t *e) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "Dummy soc_sbx_g3p1_ppe_entry_oam_rx_set()")));
    return SOC_E_NOT_FOUND;
}

/*
 *  Routine: soc_sbx_g3p1_ppe_entry_oam_rx_get
 *     
 *  Inputs:
 *      
 *      
 *  Outputs:
 *      
 */
int soc_sbx_g3p1_ppe_entry_oam_rx_get(int unit, int irulenum , soc_sbx_g3p1_oam_rx_t *e){
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "Dummy soc_sbx_g3p1_ppe_entry_oam_rx_get()")));
    return SOC_E_NOT_FOUND;
}

/*
 *  Routine: soc_sbx_g3p1_ppe_entry_oam_tx_set
 *     
 *  Inputs:
 *      
 *      
 *  Outputs:
 *      
 */
int soc_sbx_g3p1_ppe_entry_oam_tx_set(int unit, int irulenum , soc_sbx_g3p1_oam_tx_t *e){
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "Dummy soc_sbx_g3p1_ppe_entry_oam_tx_set()")));
    return SOC_E_NOT_FOUND;
}

/*
 *  Routine: soc_sbx_g3p1_ppe_entry_oam_tx_get
 *     
 *  Inputs:
 *      
 *      
 *  Outputs:
 *      
 */
int soc_sbx_g3p1_ppe_entry_oam_tx_get(int unit, int irulenum , soc_sbx_g3p1_oam_tx_t *e){
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "Dummy soc_sbx_g3p1_ppe_entry_oam_tx_get()")));
    return SOC_E_NOT_FOUND;
}

/*
 *  Routine: soc_sbx_g3p1_ppe_entry_l2cpmac_set
 *     
 *  Inputs:
 *      
 *      
 *  Outputs:
 *      
 */
int soc_sbx_g3p1_ppe_entry_l2cpmac_set(int unit, int ilsi, soc_sbx_g3p1_l2cpmac_t *e){
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "Dummy soc_sbx_g3p1_ppe_entry_l2cpmac_set()")));
    return SOC_E_NOT_FOUND;
}

/*
 *  Routine: soc_sbx_g3p1_ppe_entry_l2cpmac_get
 *     
 *  Inputs:
 *      
 *      
 *  Outputs:
 *      
 */
int soc_sbx_g3p1_ppe_entry_l2cpmac_get(int unit, int ilsi, soc_sbx_g3p1_l2cpmac_t *e){
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "Dummy soc_sbx_g3p1_ppe_entry_l2cpmac_get()")));
    return SOC_E_NOT_FOUND;
}

#endif

