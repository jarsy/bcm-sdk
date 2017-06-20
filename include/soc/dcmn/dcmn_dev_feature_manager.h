/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _DCMN_DEV_FEATURE_MANAGER_H_
#define _DCMN_DEV_FEATURE_MANAGER_H_

#include <soc/error.h>

#define INVALID_DEV_ID -1
typedef enum dcmn_feature_e {
    DCMN_INVALID_FEATURE=-1,
    DCMN_NO_METRO_FEATURE,
    DCMN_NO_DC_FEATURE,/*Data Center*/
    DCMN_NO_OTN_FEATURE,
    DCMN_JER_NIF_24_44_FEATURE,/*DCMN_<device>_NIF_<num of 25G>_<num of 12.5G>_FEATURE*/
    DCMN_JER_NIF_40_20_FEATURE,/*DCMN_<device>_NIF_<num of 25G>_<num of 12.5G>_FEATURE*/
    DCMN_JER_NIF_48_0_FEATURE,/*DCMN_<device>_NIF_<num of 25G>_<num of 12.5G>_FEATURE*/
    DCMN_JER_NIF_12_32_FEATURE,/*DCMN_<device>_NIF_<num of 25G>_<num of 12.5G>_FEATURE*/
    DCMN_QAX_NIF_10_32_FEATURE,/*DCMN_<device>_NIF_<num of 25G>_<num of 12.5G>_FEATURE*/
    DCMN_QAX_NIF_12_24_FEATURE,/*DCMN_<device>_NIF_<num of 25G>_<num of 12.5G>_FEATURE*/
    DCMN_QAX_NIF_16_24_FEATURE,/*DCMN_<device>_NIF_<num of 25G>_<num of 12.5G>_FEATURE*/
    DCMN_QUX_NIF_24_6_FEATURE,/*DCMN_<device>_NIF_<num of 2.5G>_<num of 10G>_FEATURE*/
    DCMN_QUX_NIF_12_4_FEATURE,/*DCMN_<device>_NIF_<num of 2.5G>_<num of 10G>_FEATURE*/
    DCMN_JER_4_ILKN_PORTS_FEATURE,/*Number of ILKN I/F for device*/
    DCMN_QAX_3_CAUI4_PORTS_FEATURE,/*Number of CAUI I/F for device*/
    DCMN_QAX_0_CAUI4_PORTS_FEATURE,/*Number of CAUI I/F for device*/
    DCMN_QAX_NO_STAT_FEATURE,/*Device does not support statistics interface*/
    DCMN_QAX_LOW_FALCON_SPEED_FEATURE,/*Falcon S/D capped at 10.9Gb or 12.5Gb, depending on port macro firmware update availability*/
    DCMN_FABRIC_18_QUADS_FEATURE,/*Device only has 18 Fabric Quads*/
    DCMN_FABRIC_24_QUADS_FEATURE,/*Device only has 24 Fabric Quads*/
    DCMN_FABRIC_12_QUADS_FEATURE,/*Device only has 12 Fabric Quads*/
    DCMN_NO_FABRIC_ILKN_FEATURE,/*Fabric links can't be used for ILKN*/
    DCMN_NO_FABRIC_MESH_FEATURE,/*Device can be used only in Single FAP mode*/
    DCMN_ONLY_MESH_FEATURE,/*Device can be used only with Mesh*/
    DCMN_SINGLE_STAGE_REPEATER_FEATURE,/*Device supports only SINGLE_STAGE_FE2 and REPEATER fabric device mode*/
    DCMN_NO_EXT_RAM_FEATURE,/*No external Dram I/F*/
    DCMN_ONE_EXT_RAM_FEATURE,/*One external Dram I/F*/
    DCMN_CORE_FREQ_325_FEATURE,/*Maximum core clock frequency is 325MNz*/
    DCMN_CORE_FREQ_175_FEATURE,/*Maximum core clock frequency is 175MNz*/
    DCMN_NO_EXTENDED_LPM_FEATURE,/*No Extended LPM (KAPS)*/
    DCMN_NUM_OF_FEATURES
} dcmn_feature_t;





typedef struct dcmn_range_s {
    int from;
    int to;
} dcmn_range_t;

typedef soc_error_t (*dcmn_propery_value_valid_cb) (int unit, void  *value,int nof_ranges,void *data);

typedef struct dcmn_feature_cb_s
{
    dcmn_feature_t feature;
    dcmn_propery_value_valid_cb cb;
    int nof_ranges;
    void *data;
} dcmn_feature_cb_t;

typedef struct dcmn_property_features_s
{
    char *property;
    dcmn_feature_cb_t  *features_array;
} dcmn_property_features_t;


soc_error_t dcmn_is_property_value_permited(int unit,const char *property,void *value);
soc_error_t dcmn_property_get(int unit, const char *name, uint32 defl,uint32 *value);
soc_error_t dcmn_property_suffix_num_get(int unit, int num, const char *name, const char *suffix, uint32 defl,uint32 *value);
soc_error_t dcmn_property_get_str(int unit, const char *name,char **value);
uint8       dcmn_device_block_for_feature(int unit,dcmn_feature_t feature);






#endif

