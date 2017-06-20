/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _DNXC_DEV_FEATURE_MANAGER_H_
#define _DNXC_DEV_FEATURE_MANAGER_H_

#include <soc/error.h>

#define INVALID_DEV_ID -1
typedef enum dnxc_feature_e {
    DNXC_INVALID_FEATURE=-1,
    DNXC_NO_METRO_FEATURE,
    DNXC_NO_DC_FEATURE,/*Data Center*/
    DNXC_NO_OTN_FEATURE,
    DNXC_JER2_JER_NIF_24_44_FEATURE,/*DNXC_<device>_NIF_<num of 25G>_<num of 12.5G>_FEATURE*/
    DNXC_JER2_JER_NIF_24_48_FEATURE,/*DNXC_<device>_NIF_<num of 25G>_<num of 12.5G>_FEATURE*/
    DNXC_JER2_JER_NIF_44_16_FEATURE,/*DNXC_<device>_NIF_<num of 25G>_<num of 12.5G>_FEATURE*/
    DNXC_JER2_JER_NIF_48_4_FEATURE,/*DNXC_<device>_NIF_<num of 25G>_<num of 12.5G>_FEATURE*/
    DNXC_JER2_JER_NIF_40_4_FEATURE,/*DNXC_<device>_NIF_<num of 25G>_<num of 12.5G>_FEATURE*/
    DNXC_JER2_JER_NIF_12_32_FEATURE,/*DNXC_<device>_NIF_<num of 25G>_<num of 12.5G>_FEATURE*/
    DNXC_JER2_QAX_NIF_10_32_FEATURE,/*DNXC_<device>_NIF_<num of 25G>_<num of 12.5G>_FEATURE*/
    DNXC_JER2_QAX_NIF_12_24_FEATURE,/*DNXC_<device>_NIF_<num of 25G>_<num of 12.5G>_FEATURE*/
    DNXC_JER2_QAX_NIF_16_24_FEATURE,/*DNXC_<device>_NIF_<num of 25G>_<num of 12.5G>_FEATURE*/
    DNXC_JER2_JER_4_ILKN_PORTS_FEATURE,/*Number of ILKN I/F for device*/
    DNXC_JER2_JER_2_ILKN_PORTS_FEATURE,/*Number of ILKN I/F for device*/
    DNXC_JER2_QAX_3_CAUI4_PORTS_FEATURE,/*Number of CAUI I/F for device*/
    DNXC_JER2_QAX_0_CAUI4_PORTS_FEATURE,/*Number of CAUI I/F for device*/
    DNXC_FABRIC_18_QUADS_FEATURE,/*Device only has 18 Fabric Quads*/
    DNXC_FABRIC_24_QUADS_FEATURE,/*Device only has 24 Fabric Quads*/
    DNXC_FABRIC_12_QUADS_FEATURE,/*Device only has 12 Fabric Quads*/
    DNXC_JER2_JER_24_FABRIC, /*Device has only 24 Fabric links*/
    DNXC_NO_FABRIC_ILKN_FEATURE,/*Fabric links can't be used for ILKN*/
    DNXC_NO_FABRIC_MESH_FEATURE,/*Device can be used only in Single FAP mode*/
    DNXC_SINGLE_STAGE_FE2_FEATURE,/*Device supports only SINGLE_STAGE_FE2 fabric device mode*/
    DNXC_NO_EXT_RAM_FEATURE,
    DNXC_CORE_FREQ_720_FEATURE,/*Maximum core clock frequency is 720MNz*/
    DNXC_CORE_FREQ_325_FEATURE,/*Maximum core clock frequency is 325MNz*/
    DNXC_NO_EXTENDED_LPM_FEATURE,/*No Extended LPM (KAPS)*/
    DNXC_NUM_OF_FEATURES
} dnxc_feature_t;





typedef struct dnxc_range_s {
    int from;
    int to;
} dnxc_range_t;

typedef soc_error_t (*dnxc_propery_value_valid_cb) (int unit, void  *value,int nof_ranges,void *data);

typedef struct dnxc_feature_cb_s
{
    dnxc_feature_t feature;
    dnxc_propery_value_valid_cb cb;
    int nof_ranges;
    void *data;
} dnxc_feature_cb_t;

typedef struct dnxc_property_features_s
{
    char *property;
    dnxc_feature_cb_t  *features_array;
} dnxc_property_features_t;


soc_error_t dnxc_is_property_value_permited(int unit,const char *property,void *value);
soc_error_t dnxc_property_get(int unit, const char *name, uint32 defl,uint32 *value);
soc_error_t dnxc_property_suffix_num_get(int unit, int num, const char *name, const char *suffix, uint32 defl,uint32 *value);
soc_error_t dnxc_property_get_str(int unit, const char *name,char **value);
uint8       dnxc_device_block_for_feature(int unit,dnxc_feature_t feature);






#endif

