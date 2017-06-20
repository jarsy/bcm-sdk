/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: tdm.c,v 1.5.16.6 Broadcom SDK $
 *
 * File:    sws.c
 * Purpose: Caladan3 SWS drivers, TDM determination
 * Requires:
 */



#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3/port.h>
#include <soc/sbx/caladan3/sws_params.h>
#include <soc/sbx/caladan3/sws.h>
#include <soc/sbx/caladan3/util.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/mem.h>
#include <soc/mcm/allenum.h>
#include <soc/sbx/caladan3/ped.h>
#include <soc/sbx/caladan3/sws_configs/c3_null.h>
#include <soc/sbx/caladan3/sws_configs/c3_1x100g_1xhg127.h>
#include <soc/sbx/caladan3/sws_configs/c3_1x100g_1xil100.h>
#include <soc/sbx/caladan3/sws_configs/c3_12x10g_1xhg127.h>
#include <soc/sbx/caladan3/sws_configs/c3_12x10g_1xil100.h>
#include <soc/sbx/caladan3/sws_configs/c3_48x1g_2xhg25.h>
#include <soc/sbx/caladan3/sws_configs/c3_48x1g_1xil100.h>
#include <soc/sbx/caladan3/sws_configs/c3_8x1g_4xhg10_ilkn50n.h>
#include <soc/sbx/caladan3/sws_configs/c3_48x1g_1xhg127.h>
#include <soc/sbx/caladan3/sws_configs/c3_3x40g_1xil100.h>
#include <soc/sbx/caladan3/sws_configs/c3_8x10g_2xhg10_1xil100.h>
#include <soc/sbx/caladan3/sws_configs/c3_4x10g_2xhg10_1xil100.h>
#include <soc/sbx/caladan3/sws_configs/c3_4x1g_2xhg10_1xil100.h>
#include <soc/sbx/caladan3/sws_configs/c3_3x40g_3xhg42.h>
#include <soc/sbx/caladan3/sws_configs/c3_8x10g_16x1g_1xhg127.h>
#include <soc/sbx/caladan3/sws_configs/c3_16x1g_8x10g_1xhg127.h>
#include <soc/sbx/caladan3/sws_configs/c3_1x40g_8x10g.h>
#include <soc/sbx/caladan3/sws_configs/c3_8x10g_4xhg10_1xil100.h>
#include <soc/sbx/caladan3/sws_configs/c3_4cos_1x100g_1xil100.h>
#include <soc/sbx/caladan3/sws_configs/c3_4cos_12x10g_1xil100.h>
#include <soc/sbx/caladan3/sws_configs/c3_4x10g_2xhg10_16x1g_1xil50.h>
#include <soc/sbx/caladan3/sws_configs/c3_4x10g_18x1g_1xil50.h>
#include <soc/sbx/caladan3/sws_configs/c3_nj_12x10g_1xil100.h>
#include <soc/sbx/caladan3/sws_configs/c3_nj_1x100g_1xil100.h>
#include <soc/sbx/caladan3/sws_configs/c3_4x10g_2xXAUI_1xhg127.h>
#include <soc/sbx/caladan3/sws_configs/c3_4x10g_20x1g_1xhg127.h>
#include <soc/sbx/caladan3/sws_configs/c3_1xil100_1xhg127.h>
#include <soc/sbx/caladan3/sws_configs/c3_12x1g_1xhg42.h>
#include <soc/sbx/caladan3/sws_configs/c3_4x10g_6xhg10_1xil100.h>
#include <soc/sbx/caladan3/sws_configs/c3_20x1g_1xhg42.h>
#include <soc/sbx/caladan3/sws_configs/c3_8x10g_4xhg10.h>
#include <soc/sbx/caladan3/sws_configs/c3_1xil100_channelized_1xhg127.h>
#include <soc/sbx/caladan3/sws_configs/c3_10x10g_2xhg10.h>
#include <soc/sbx/caladan3/sws_configs/c3_4x10g_2xhg10_4x1g_1xil50.h>

#include <soc/sbx/caladan3/sws_configs/c3_2x10g_32x1g_1xhg127.h>


#include <soc/sbx/caladan3/sws_configs/c3_clport_based_common_mode_ilkn50.h>

typedef struct  tdm_registry_s {
    tdm_identifier_t tdm_id;
    sws_config_t *sws_cfg;
} tdm_registry_t;

tdm_registry_t _tdm_id_registered[SOC_SBX_CALADAN3_SWS_MAX_TDM];

/* TDM Database */

/* 
 * turns out to be much simple if the end user specifies the tdm
 * But caladan uses the ucodeport map in the config.bcm to specify 
 * portmap.
 */
typedef struct tdm_dbase_s {
    int num_line_ports;
    int num_fabric_ports;
    int num_100g;
    int num_hg127;
    int num_hg42;
    int num_hg25;
    int num_hg10;
    int num_40g;
    int num_10g;
    int num_1g;
    int num_ilkn_100;
    int num_ilkn_50;
    int num_xt0;
    int num_xt1;
    int num_xt2;
} tdm_dbase_t;

tdm_dbase_t tdm_database[] = {

 /* nl, nf, 100g, hg127, hg42, hg25, hg10, 40g, 10g, 1g, i100, i50, xt0, xt1, xt2 */
  { 0,  0,   0,    0,     0,    0,    0,    0,   0,   0,   0,   0,    0,   0,   0 }, /* TDMNULL */
  { 1,  1,   1,    1,     0,    0,    0,    0,   0,   0,   0,   0,    0,   0,   0 }, /* TDM1 */
  { 3,  3,   0,    0,     3,    0,    0,    3,   0,   0,   0,   0,    0,   0,   0 }, /* TDM2 */
  {12,  1,   0,    1,     0,    0,    0,    0,  12,   0,   0,   0,    0,   0,   0 }, /* TDM3 */
  { 9,  1,   0,    1,     1,    0,    0,    0,   8,   0,   0,   0,    0,   0,   0 }, /* TDM4 */
  {12,  1,   0,    0,     0,    1,    0,    0,   0,  12,   0,   0,    0,   0,   0 }, /* TDM5 */
  {48,  2,   0,    0,     0,    2,    0,    0,   0,  48,   0,   0,   12,  12,  12 }, /* TDM6 */
  { 5,  3,   0,    0,     0,    3,    0,    0,   5,   0,   0,   0,    0,   0,   0 }, /* TDM7 */
  { 8,  4,   0,    0,     0,    4,    0,    0,   8,   0,   0,   0,    0,   0,   0 }, /* TDM8 */
  { 1,  1,   0,    0,     0,    0,    0,    0,   0,   0,   0,   2,    0,   0,   0 }, /* TDM9 */
  { 1,  1,   1,    0,     0,    0,    0,    0,   0,   0,   1,   0,    0,   0,   0 }, /* TDM10 */

 /* nl, nf, 100g, hg127, hg42, hg25, hg10, 40g, 10g, 1g, i100, i50, xt0, xt1, xt2 */
  { 2,  2,   0,    0,     2,    0,    0,    2,   0,   0,   0,   0,    0,   0,   0 }, /* TDM11 */
  {12,  1,   0,    0,     0,    0,    0,    0,  12,   0,   1,   0,    0,   0,   0 }, /* TDM12 */
  { 3,  1,   0,    0,     0,    0,    0,    3,   0,   0,   1,   0,    0,   0,   0 }, /* TDM13 */
  {10,  1,   0,    0,     0,    0,    2,    0,   8,   0,   1,   0,    0,   0,   0 }, /* TDM14 */
  { 6,  1,   0,    0,     0,    0,    2,    0,   4,   0,   1,   0,    0,   0,   0 }, /* TDM15 */
  { 6,  1,   0,    0,     0,    0,    2,    0,   0,   4,   1,   0,    0,   0,   0 }, /* TDM16 */
  {24,  1,   0,    1,     0,    0,    0,    0,   8,  16,   0,   0,    0,   4,  12 }, /* TDM17 */
  {24,  1,   0,    1,     0,    0,    0,    0,   8,  16,   0,   0,    8,   8,   0 }, /* TDM18 */
  { 9,  1,   0,    0,     0,    0,    0,    1,   8,   0,   1,   0,    0,   0,   0 }, /* TDM19 */
  { 9,  1,   0,    1,     0,    0,    0,    1,   8,   0,   0,   0,    0,   0,   0 }, /* TDM20 */

 /* nl, nf, 100g, hg127, hg42, hg25, hg10, 40g, 10g, 1g, i100, i50, xt0, xt1, xt2 */
  {48,  1,   0,    0,     0,    0,    0,    0,   0,  48,   1,   0,   12,  12,  12 }, /* TDM21 */
  {48,  1,   0,    1,     0,    0,    0,    0,   0,  48,   0,   0,   12,  12,  12 }, /* TDM22 */
  { 6,  1,   0,    0,     0,    0,    2,    0,   2,   2,   1,   0,    0,   0,   0 }, /* TDM23 */
  { 1,  1,   1,    1,     0,    0,    0,    0,   0,   0,   0,   0,    0,   0,   0 }, /* TDM24 */
  {12,  1,   0,    1,     0,    0,    0,    0,  12,   0,   0,   0,    0,   0,   0 }, /* TDM25 */
  {22,  1,   0,    0,     0,    0,    2,    0,   4,  16,   0,   1,    0,   4,  12 }, /* TDM26 */
  {22,  1,   0,    0,     0,    0,    0,    0,   4,  18,   0,   1,    0,   4,  12 }, /* TDM27 */
  {12,  1,   0,    1,     0,    0,    0,    0,  12,   0,   0,   0,    0,   0,   0 }, /* TDM28 */
  { 1,  1,   1,    1,     0,    0,    0,    0,   0,   0,   0,   0,    0,   0,   0 }, /* TDM29 */
  { 6,  1,   0,    1,     0,    0,    0,    0,   6,   0,   0,   0,    0,   0,   0 }, /* TDM30 */

 /* nl, nf, 100g, hg127, hg42, hg25, hg10, 40g, 10g, 1g, i100, i50, xt0, xt1, xt2 */
  { 24, 1,   0,    1,     0,    0,    0,    0,   4,  20,   0,   0,    8,   8,   0 }, /* TDM31 */

  {  1, 1,   0,    0,/*XXXTTT was "1" but comment out for now to 
                       avoid this overly simplistic matching scheme!
                     since colliding with TDM37*/     0,    0,    0,    0,   0,   0,   1,   0,    0,   0,   0 }, /* TDM32 */

  {12,  1,   0,    0,     1,    0,    0,    0,   0,  12,   0,   0,    0,   0,   0 }, /* TDM33 */
  {10,  1,   0,    0,     0,    0,    6,    0,   4,   0,   1,   0,    0,   0,   0 }, /* TDM34 */
  {20,  1,   0,    0,     1,    0,    0,    0,   0,  20,   0,   0,    0,   0,   0 }, /* TDM35 */
  {12,  1,   0,    1,     0,    0,    4,    0,   8,   0,   0,   0,    0,   0,   0 }, /* TDM36 */
  { 1,  1,   0,    1,     0,    0,    0,    0,   0,   0,   1,   0,    0,   0,   0 }, /* TDM37 */
  { 7,  1,   0,    0,     0,    0,    0,    1,   6,   0,   1,   0,    0,   0,   0 }, /* TDM38 */
  {12,  1,   0,    0,     0,    0,    2,    0,  10,   0,   1,   0,    0,   0,   0 }, /* TDM39 */
  {12,  1,   0,    0,     0,    0,    4,    0,   0,   8,   0,   1,    0,   0,   0 }, /* TDM40 */

 /*nl, nf, 100g, hg127, hg42, hg25, hg10,  40g, 10g, 1g, i100, i50,  xt0, xt1, xt2 */
  {12,  1,   0,    0,     0,    0,    2,    0,   6,   4,   0,   1,    0,   0,   0 }, /* TDM41 */
  { 7,  1,   0,    1,     0,    0,    0,    1,   6,   0,   0,   0,    0,   0,   0 }, /* TDM42 */
  {36,  1,   0,    1,     0,    0,    0,    0,   4,   32,  0,   0,    8,   12, 12 }, /* TDM43 */

  /* TDM44 is a "direct lookup" TDM. It is overloaded; meaning it is a 
   * common baseline TDM for specific variations. We configure line and fabric ports but
   * purposely leave out the rest.
   */
  {12,  1,   0,    0,     0,    0,    0,    0,   0,   0,   0,   1,    0,   0,   0 }, /* TDM44 */

  { 0,  0,   0,    0,     0,    0,    0,    0,   0,   0,   0,   0,    0,   0,   0 }, /* TDMLAST */

};



/*
 */
int soc_sbx_caladan3_sws_tdm_is_oversubscribed(int unit, int tdm_id)
{
    int idx;
    if ((tdm_id >= TDMNULL) && (tdm_id < TDMLAST)) {
        for (idx = 0;idx < SOC_SBX_CALADAN3_SWS_MAX_TDM; idx++) {
            if (_tdm_id_registered[idx].tdm_id.id == tdm_id) {
                return (_tdm_id_registered[idx].sws_cfg->global_cfg.tdm_is_oversubscribed);
            }
        }
    } 
    return 0;
}


char *
soc_sbx_caladan3_sws_tdm_name(int tdmid) {
    if ((tdmid >= TDMNULL) && (tdmid < TDMLAST)) {
        return (_tdm_id_registered[tdmid].tdm_id.name);
    } 
    return NULL;
}

/* TDM helpers */
int
soc_sbx_caladan3_sws_tdm_lookup(int unit, int *tdmid, sws_config_t **sws_cfg)
{
    int port;
    int blk, idx, cfgid;
    tdm_dbase_t tdm_data = {0};
    soc_info_t *si = &SOC_INFO(unit);

    soc_sbx_caladan3_port_map_t *port_map  COMPILER_ATTRIBUTE((unused));

    int phy_port, xt0, xt1, xt2;
    int found = 0;
    int ucode_num_port_override = 0;
    int status = SOC_E_NONE;
    int is_chan = FALSE, dummy = 0;

    int explicit_tdm_configured = 0;
    int ret = SOC_E_NONE;

    if ((!tdmid) || (!sws_cfg)) {
        return SOC_E_PARAM;
    }

    /* Direct load TDM (aka explicit TDM specification)*/
    cfgid = soc_property_get(unit, spn_BCM88030_CONFIG, -1);
    if (cfgid > 0) {
        *tdmid = cfgid;
        *sws_cfg = NULL;
        for (idx = 0; idx < SOC_SBX_CALADAN3_SWS_MAX_TDM; idx++) {
            if (_tdm_id_registered[idx].tdm_id.id == cfgid) {
                *sws_cfg = _tdm_id_registered[idx].sws_cfg;
                break;
            }
        }
        explicit_tdm_configured = 1;

        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Explicit TDM Configured  TDM ID:%d  for Unit:%d \n"), *tdmid, unit));
    }

    /* Parse ports and find matching TDM */
    xt0 = soc_sbx_block_find(unit, SOC_BLK_XTPORT, 0);
    xt1 = soc_sbx_block_find(unit, SOC_BLK_XTPORT, 1);
    xt2 = soc_sbx_block_find(unit, SOC_BLK_XTPORT, 2);

    port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    /* Override the number of ports to match given tdms and support channelized ports */
    ucode_num_port_override = soc_property_get(unit, "ucode_num_port_override", 0);

    /* Approximate the config to TDM */
    SOC_PBMP_ITER(PBMP_PORT_ALL(unit), port) {

        /* Assume the port is no channelized and determine only if required */
        if (ucode_num_port_override) {
            is_chan = FALSE;
            status = soc_sbx_caladan3_port_is_channelized_subport(unit, port, &is_chan, &dummy);
            if (status != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d soc_sbx_caladan3_port_is_channelized_subport port %d failed: %s\n"),
                           unit, port, soc_errmsg(status)));
                    return status;
            } else if (is_chan) {
                /* Skipping */
                continue;
            }
        }
        if (soc_feature(unit, soc_feature_logical_port_num)) {
            phy_port = si->port_l2p_mapping[port];
        } else {
            phy_port = port;
        }
        blk = SOC_PORT_IDX_BLOCK(unit, phy_port, 0);
        if (soc_sbx_caladan3_is_line_port(unit, port)) {
            tdm_data.num_line_ports++;
        } else {
            tdm_data.num_fabric_ports++;
        }
        if (PBMP_MEMBER(si->il.bitmap, port)) {
            if (si->port_speed_max[port] == SOC_SBX_CALADAN3_ILKN50w_SPEED) {
                   tdm_data.num_ilkn_50++;
            } else {
                if (si->port_num_lanes[port] < 10) {
                   tdm_data.num_ilkn_50++;
                } else {
                   tdm_data.num_ilkn_100++;
                }
            }
        } else if (si->port_speed_max[port] == 100*SOC_SBX_CALADAN3_GIG_SPEED) {
            tdm_data.num_100g++;
        } else if (si->port_speed_max[port] == 127*SOC_SBX_CALADAN3_GIG_SPEED) {
            tdm_data.num_hg127++;
        } else if (si->port_speed_max[port] == 42*SOC_SBX_CALADAN3_GIG_SPEED) {
            tdm_data.num_hg42++;
        } else if (si->port_speed_max[port] == 40*SOC_SBX_CALADAN3_GIG_SPEED) {
            tdm_data.num_40g++;
        } else if (si->port_speed_max[port] == 25*SOC_SBX_CALADAN3_GIG_SPEED) {
            tdm_data.num_hg25++;
        } else if (si->port_speed_max[port] == 10*SOC_SBX_CALADAN3_GIG_SPEED) {
            if (IS_HG_PORT(unit, port)) {
                tdm_data.num_hg10++;
            } else {
                tdm_data.num_10g++;
            }
        } else if (si->port_speed_max[port] == 1*SOC_SBX_CALADAN3_GIG_SPEED) {
            if (blk == xt0) tdm_data.num_xt0++;
            if (blk == xt1) tdm_data.num_xt1++;
            if (blk == xt2) tdm_data.num_xt2++;
            tdm_data.num_1g++;
        }
    }

    if (tdm_data.num_line_ports > 0) {
        tdm_data.num_1g -= SOC_SBX_CALADAN3_MAX_XLPORT_PORT;
        tdm_data.num_line_ports -= SOC_SBX_CALADAN3_MAX_XLPORT_PORT;
    }

    for (idx = 0; 
         idx < (sizeof(tdm_database)/sizeof(tdm_database[0])); idx++) {
        if (sal_memcmp(&tdm_data, &tdm_database[idx], sizeof(tdm_data)) == 0) {
            found = 1; 
            break;
        }
    }

    if (found && !explicit_tdm_configured) {
        for (cfgid = 0; cfgid < SOC_SBX_CALADAN3_SWS_MAX_TDM; cfgid++) {
           if (_tdm_id_registered[cfgid].tdm_id.id == idx) {
               *tdmid = idx;
               *sws_cfg = _tdm_id_registered[cfgid].sws_cfg;
               return SOC_E_NONE;
           }
        }
    }
        
    if (!found && !explicit_tdm_configured) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, Failed locating the TDM data\n"), unit));
        ret = SOC_E_NOT_FOUND;
    }

    /* Dump debug info for either TDM !found or for explicit TDM configured */

    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "Port Config Specifies Total of:  %d Line Ports  %d Fabric Ports  for Unit:%d\n"), 
               tdm_data.num_line_ports, tdm_data.num_fabric_ports, unit));

    if (tdm_data.num_line_ports > 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Detailed Port Config: ")));

        if (tdm_data.num_100g) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%dx100G"), tdm_data.num_100g));
        }

        if (tdm_data.num_40g) {
            if (tdm_data.num_100g) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_")));
            }
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%dx40G"), tdm_data.num_40g));
        }
        if (tdm_data.num_10g) {
            if (tdm_data.num_40g) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_")));
            }
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%dx10G"), tdm_data.num_10g));
        }
        if (tdm_data.num_hg10) {
            if (tdm_data.num_10g) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_")));
            }
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%dxHG10"), tdm_data.num_hg10));
        }
        if (tdm_data.num_1g) {
            if (tdm_data.num_hg10) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_")));
            }
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%dx1G"), tdm_data.num_1g));
        }
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "-")));

        if (tdm_data.num_ilkn_100) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%dxILKN100 "), tdm_data.num_ilkn_100));
        }
        if (tdm_data.num_ilkn_50) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%dxILKN50 "), tdm_data.num_ilkn_50));
        }
        if (tdm_data.num_hg127) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%dxHG127 "), tdm_data.num_hg127));
        }
        if (tdm_data.num_hg42) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%dxHG42 "), tdm_data.num_hg42));
        }
        if (tdm_data.num_hg25) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%dxHG25 "), tdm_data.num_hg25));
        }

        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "\n")));
    }

    return ret;
}

tdm_identifier_t *
soc_sbx_caladan3_tdm_identifier_get(int unit, int tdmid)
{
    int idx;

    if ((unit < 0) || (unit > SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_tdm_select: Invalid unit\n")));
        return NULL;
    }
    if ((tdmid < TDMNULL) || (tdmid >= SOC_SBX_CALADAN3_SWS_MAX_TDM)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_tdm_select: Invalid TDM\n")));
        return NULL;
    }
    for (idx = 0; idx < SOC_SBX_CALADAN3_SWS_MAX_TDM; idx++) {
        if (_tdm_id_registered[idx].tdm_id.id == tdmid) {
            return &(_tdm_id_registered[idx].tdm_id);
        }
    }
            
    return NULL;
}


int
soc_sbx_caladan3_sws_tdm_register(int unit, int id, char *name, sws_config_t *sws_cfg)
{
    int idx;

    if ((unit < 0) || (unit > SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_tdm_register: Invalid unit\n")));
        return SOC_E_PARAM;
    }
    for (idx = 0; idx < SOC_SBX_CALADAN3_SWS_MAX_TDM; idx++) {
        if (_tdm_id_registered[idx].tdm_id.id == id) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "sws_tdm_register: TDM %d already registered\n"), id));
            return SOC_E_EXISTS;
        } else if (_tdm_id_registered[idx].tdm_id.id == TDMNONE) {
            _tdm_id_registered[idx].tdm_id.id = id;
            _tdm_id_registered[idx].tdm_id.name = name;
            _tdm_id_registered[idx].sws_cfg = sws_cfg;
            return SOC_E_NONE;
        }
    }
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "sws_tdm_register: TDM database full, increase limits\n")));
    return SOC_E_FULL;
}

int
soc_sbx_caladan3_sws_tdm_unregister(int unit, int id, char *name, sws_config_t **sws_cfg)
{

    int idx;
 
    if ((unit < 0) || (unit > SOC_MAX_NUM_DEVICES)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sws_tdm_unregister: Invalid unit\n")));
        return SOC_E_PARAM;
    }
    if (!sws_cfg) {
        return SOC_E_PARAM; 
    }

    for (idx = 0; idx < SOC_SBX_CALADAN3_SWS_MAX_TDM; idx++) {
        if ((_tdm_id_registered[idx].tdm_id.id == id) ||
                 (sal_memcmp(_tdm_id_registered[idx].tdm_id.name, name, strlen(name)) == 0)) {
            *sws_cfg = _tdm_id_registered[idx].sws_cfg;
            _tdm_id_registered[idx].tdm_id.id = TDMNONE;
            _tdm_id_registered[idx].tdm_id.name = NULL;
            _tdm_id_registered[idx].sws_cfg = NULL;
            return SOC_E_NONE;
        }
    }
    return SOC_E_NOT_FOUND;
}


/*
 *
 * Function:
 *     soc_sbx_caladan3_sws_tdm_config_init
 * Purpose:
 *     Initialize SWS TDM config
 */
int
soc_sbx_caladan3_sws_tdm_config_init(int unit)
{
    int     i;

    for (i = 0; i < SOC_SBX_CALADAN3_SWS_MAX_TDM; i++) {
        _tdm_id_registered[i].tdm_id.id = TDMNONE;
    }

    soc_sbx_caladan3_sws_tdm_register(unit, TDMNULL,  "NULL",                 &c3_null_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM1,  "1x100GE-1xHG127",         &c3_1x100g_1xhg127_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM2,  "3x40GE-3xHG42",           &c3_3x40g_3xhg42_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM3,  "12x10GE-1xHG127",         &c3_12x10g_1xhg127_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM6,  "48x1GE-2xHG25",           &c3_48x1g_2xhg25_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM10, "1x100GE-1xILKN100",       &c3_1x100g_1xil100_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM12, "12x10GE-1xILKN100",       &c3_12x10g_1xil100_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM13, "3x40GE-1xILKN100",        &c3_3x40g_1xil100_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM14, "8x10GE+2xHG10-1xILKN100", &c3_8x10g_4xhg10_1xil100_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM15, "4x10GE+2xHG10-1xILKN100", &c3_8x10g_4xhg10_1xil100_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM16, "4x1GE+2xHG10-1xILKN100",  &c3_8x10g_4xhg10_1xil100_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM17, "8x10GE+16xGE-1xHG127",    &c3_8x10g_16x1g_1xhg127_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM18, "16xGE+8x10GE-1xHG127",    &c3_16x1g_8x10g_1xhg127_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM19, "1x40GE+8x10GE-1xILKN100", &c3_1x40g_8x10g_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM20, "1x40GE+8x10GE-1xHG127",   &c3_1x40g_8x10g_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM21, "48x1GE-1xILKN100",          &c3_48x1g_1xil100_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM22, "48x1GE-1xHG127",          &c3_48x1g_1xhg127_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM23, "2x10GE+2x1GE+2xHG10-1xILKN100",  &c3_8x10g_4xhg10_1xil100_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM24, "4Cos-1x100GE-1xILKN100",  &c3_4cos_1x100g_1xil100_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM25, "4Cos-12x10GE-1xILKN100",  &c3_4cos_12x10g_1xil100_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM26, "4x10GE+16x1GE+2xHG10-1xILKN50",  &c3_4x10g_2xhg10_16x1g_1xil50_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM27, "4x10GE+18x1GE-1xILKN50",  &c3_4x10g_18x1g_1xil50_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM28, "NJ_12x10G-1xILKN100",     &c3_nj_12x10g_1xil100_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM29, "NJ_1x100G-1xILKN100",     &c3_nj_1x100g_1xil100_cfg);

    soc_sbx_caladan3_sws_tdm_register(unit, TDM30,  "4x10GE+2xXAUI-1xHG127", &c3_4x10g_2xXAUI_1xhg127_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM31,  "4x10G_20x1G-1xHG127", &c3_4x10g_20x1g_1xhg127_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM32,  "1xILKN100-1xHG127", &c3_1xilkn100_1xhg127_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM33,  "12x1G-1xHG42", &c3_12x1g_1xhg42_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM34,  "4x10G_6xHG10-1xILKN100", &c3_4x10g_6xhg10_1xil100_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM35,  "20x1G-1xHG42", &c3_20x1g_1xhg42_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM36,  "8x10GE_4xHG10", &c3_8x10g_4xhg10_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM37,  "1xILKN100-channelized-1xHG127", &c3_1xil100_channelized_1xhg127_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM38,  "1x40G_6x10GE-ILKN100", &c3_1x40g_8x10g_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM39,  "10x10GE_2xHG10", &c3_10x10g_2xhg10_cfg);

    soc_sbx_caladan3_sws_tdm_register(unit, TDM40,  "8x1GE_4xHG10-1xILKN50N",          &c3_8x1g_4xhg10_ilkn50n_cfg);

    soc_sbx_caladan3_sws_tdm_register(unit, TDM41,  "4x10G_2xHG10_4x1G-1xILKN50", &c3_4x10g_2xhg10_4x1g_1xil50_cfg);

    soc_sbx_caladan3_sws_tdm_register(unit, TDM42,  "1x40G_6x10GE-1x1xHG127", &c3_1x40g_8x10g_cfg);
    soc_sbx_caladan3_sws_tdm_register(unit, TDM43,  "2x10G_32x1G-1xHG127", &c3_2x10g_32x1g_1xhg127_cfg);

    soc_sbx_caladan3_sws_tdm_register(unit, TDM44,  "CLPORT_BASED_COMMON_MODE_1xil50", &c3_clport_based_common_mode_cfg);

    return SOC_E_NONE;
}

int
soc_sbx_caladan3_sws_tdm_is_swapable(int unit)
{
    int current, prev;
    current = soc_sbx_caladan3_sws_tdm_id_current(unit);
    prev = soc_sbx_caladan3_sws_tdm_id_last(unit);

    if ((current < 0) || (prev < 0)) {
        /* Unknown */
        return FALSE;
    }
    if (((current == TDM17) || (current == TDM18)) &&
             ((prev == TDM17) || (prev == TDM18))) {
        return TRUE;
    }

    return FALSE;
}


int
soc_sbx_caladan3_sws_tdm_show_all(int unit) 
{
    int idx;
    LOG_CLI((BSL_META_U(unit,
                        "\nList of Supported TDMs \n")));
    LOG_CLI((BSL_META_U(unit,
                        "+---------------------------------------------+\n")));
    LOG_CLI((BSL_META_U(unit,
                        "| TDM id   |       TDM name                   |\n")));
    LOG_CLI((BSL_META_U(unit,
                        "+---------------------------------------------+\n")));
    for (idx=0; idx < SOC_SBX_CALADAN3_SWS_MAX_TDM; idx++) {
        if (_tdm_id_registered[idx].tdm_id.name) {
            LOG_CLI((BSL_META_U(unit,
                                "| %3d      | %32s |\n"),
                     _tdm_id_registered[idx].tdm_id.id,
                     _tdm_id_registered[idx].tdm_id.name));
        }
    }
    LOG_CLI((BSL_META_U(unit,
                        "+---------------------------------------------+\n")));
    return SOC_E_NONE;
}



#endif
