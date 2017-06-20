/*
 * $Id:$
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_BCM_INTR



#include <shared/bsl.h>
#include <shared/util.h>

#include <soc/error.h>
#include <soc/drv.h>
#include <soc/dnxc/legacy/error.h>

/* 
 *  include  
 */ 



#include <soc/dnxc/legacy/dnxc_dev_feature_manager.h>



static soc_error_t dnxc_keywords_cb (int unit,void  *value,int nof_ranges,void *data)
{
    char **keywords = (char **)data;
    int i;

    DNXC_INIT_FUNC_DEFS;

/*    v = _shr_ctoi((char *)value); */
    if (value==NULL ||  !sal_strlen(value)) {
        SOC_EXIT;
    }
    for (i = 0; keywords[i]; i++) {
        if (!sal_strcmp(value,keywords[i]))
        {
            SOC_EXIT;
        }
    }
    DNXC_IF_ERR_EXIT(SOC_E_PARAM);

exit:
    DNXC_FUNC_RETURN;

}


static soc_error_t dnxc_range_cb (int unit,void  *value,int nof_ranges,void *data)
{
    uint32 v;
    int i;
    dnxc_range_t *ranges = (dnxc_range_t *) data;


    DNXC_INIT_FUNC_DEFS;

/*    v = _shr_ctoi((char *)value); */

    v = *(uint32 *)value;
    for (i=0;i<nof_ranges;i++) {
        if (ranges[i].from <=v && v<=ranges[i].to) {
            SOC_EXIT;
        }
    }
    DNXC_IF_ERR_EXIT(SOC_E_PARAM);

exit:
    DNXC_FUNC_RETURN;

}



static soc_error_t dnxc_otn_enable_cb (int unit,void  *value,int nof_ranges,void *data)
{
    uint16 dev_id; 
    uint8 rev_id;

    DNXC_INIT_FUNC_DEFS;
    soc_cm_get_id(unit, &dev_id, &rev_id);

    if (dev_id==BCM88675_DEVICE_ID  || dev_id==BCM88375_DEVICE_ID) {
        uint32 val;
        DNXC_IF_ERR_EXIT(READ_ECI_OGER_1004r_REG32(unit,&val));
        /* Register ECI_OGER_1004 - Bit  9
        * If the bit is "1" it means that OTN is disabled in the device.
        */
        if (!(val & 1<<8)) {
            SOC_EXIT;
        }
    }


    DNXC_IF_ERR_EXIT(dnxc_keywords_cb(unit,value,nof_ranges,data));

exit:
    DNXC_FUNC_RETURN;

}



static dnxc_range_t range_0_0[] = {{0,0}};
static dnxc_range_t range_0_720000[] = {{0,720000}};
static dnxc_range_t range_0_325000[] = {{0,325000}};
static char *none_keywords[]={"0","none",0};
static char *fabric_mesh_keywords[]={"SINGLE_FAP",0};

static dnxc_feature_cb_t  dc_feature_range[] = {
    {
        DNXC_NO_DC_FEATURE,
        dnxc_range_cb,  /*  cb function*/
        sizeof(range_0_0)/sizeof(dnxc_range_t), /*num of ranges*/
        range_0_0
    },
    {
        DNXC_INVALID_FEATURE,
        NULL,
        0,
        NULL
    }
};

static dnxc_feature_cb_t  metro_feature_range[] = {
    {
        DNXC_NO_METRO_FEATURE,
        dnxc_range_cb,  /*  cb function*/
        sizeof(range_0_0)/sizeof(dnxc_range_t), /*num of ranges*/
        range_0_0
    },
    {
        DNXC_INVALID_FEATURE,
        NULL,
        0,
        NULL
    }
};
static dnxc_feature_cb_t  otn_feature_none[] = {
    {
        DNXC_NO_OTN_FEATURE,
        dnxc_otn_enable_cb,  /*  cb function*/
        0, /*not relevant*/
        none_keywords
    },
    {
        DNXC_INVALID_FEATURE,
        NULL,
        0,
        NULL
    }
};

static dnxc_feature_cb_t  fabric_24_feature_range[] = {
    {
        DNXC_JER2_JER_24_FABRIC,
        dnxc_range_cb,  /*  cb function*/
        sizeof(range_0_0)/sizeof(dnxc_range_t), /*num of ranges*/
        range_0_0
    },
    {
        DNXC_INVALID_FEATURE,
        NULL,
        0,
        NULL
    }
};

static dnxc_feature_cb_t  fabric_ilkn_feature_range[] = {
    {
        DNXC_NO_FABRIC_ILKN_FEATURE,
        dnxc_range_cb,  /*  cb function*/
        sizeof(range_0_0)/sizeof(dnxc_range_t), /*num of ranges*/
        range_0_0
    },
    {
        DNXC_INVALID_FEATURE,
        NULL,
        0,
        NULL
    }
};

static dnxc_feature_cb_t  fabric_mesh_feature_kwywords[] = {
    {
        DNXC_NO_FABRIC_MESH_FEATURE,
        dnxc_keywords_cb,  /*  cb function*/
        0, /*not relevant*/
        fabric_mesh_keywords
    },
    {
        DNXC_INVALID_FEATURE,
        NULL,
        0,
        NULL
    }
};
static dnxc_feature_cb_t  ext_ram_feature_range[] = {
    {
        DNXC_NO_EXT_RAM_FEATURE,
        dnxc_range_cb,  /*  cb function*/
        sizeof(range_0_0)/sizeof(dnxc_range_t), /*num of ranges*/
        range_0_0
    },
    {
        DNXC_INVALID_FEATURE,
        NULL,
        0,
        NULL
    }
};
static dnxc_feature_cb_t  core_freq_720_feature_range[] = {
    {
        DNXC_CORE_FREQ_720_FEATURE,
        dnxc_range_cb,  /*  cb function*/
        sizeof(range_0_720000)/sizeof(dnxc_range_t), /*num of ranges*/
        range_0_720000
    },
    {
        DNXC_INVALID_FEATURE,
        NULL,
        0,
        NULL
    }
};
static dnxc_feature_cb_t  core_freq_325_feature_range[] = {
    {
        DNXC_CORE_FREQ_325_FEATURE,
        dnxc_range_cb,  /*  cb function*/
        sizeof(range_0_325000)/sizeof(dnxc_range_t), /*num of ranges*/
        range_0_325000
    },
    {
        DNXC_INVALID_FEATURE,
        NULL,
        0,
        NULL
    }
};
static dnxc_feature_cb_t  fabric_18_quads_feature_range[] = {
    {
        DNXC_FABRIC_18_QUADS_FEATURE,
        dnxc_range_cb,  /*  cb function*/
        sizeof(range_0_0)/sizeof(dnxc_range_t), /*num of ranges*/
        range_0_0
    },
    {
        DNXC_INVALID_FEATURE,
        NULL,
        0,
        NULL
    }
};
static dnxc_feature_cb_t  fabric_24_quads_feature_range[] = {
    {
        DNXC_FABRIC_24_QUADS_FEATURE,
        dnxc_range_cb,  /*  cb function*/
        sizeof(range_0_0)/sizeof(dnxc_range_t), /*num of ranges*/
        range_0_0
    },
    {
        DNXC_INVALID_FEATURE,
        NULL,
        0,
        NULL
    }
};
static dnxc_feature_cb_t  fabric_12_quads_feature_range[] = {
    {
        DNXC_FABRIC_12_QUADS_FEATURE,
        dnxc_range_cb,  /*  cb function*/
        sizeof(range_0_0)/sizeof(dnxc_range_t), /*num of ranges*/
        range_0_0
    },
    {
        DNXC_INVALID_FEATURE,
        NULL,
        0,
        NULL
    }
};
/* feature propert map we connect to each property list of features that the property should blocked if feature disabled*/
static dnxc_property_features_t  properies_feature_map[] = {
                                                        {
                                                            spn_TRILL_MODE,
                                                            dc_feature_range
                                                        },
                                                        {
                                                            spn_BCM886XX_VXLAN_ENABLE,
                                                            dc_feature_range
                                                        },
                                                        {
                                                            spn_BCM886XX_L2GRE_ENABLE ,
                                                            dc_feature_range
                                                        },
                                                        {
                                                            spn_BCM886XX_FCOE_SWITCH_MODE  ,
                                                            dc_feature_range
                                                        },
                                                        {
                                                            spn_NUM_OAMP_PORTS ,
                                                            metro_feature_range
                                                        },
                                                        {
                                                            spn_FAP_TDM_BYPASS  ,
                                                            otn_feature_none
                                                        },
                                                        {
                                                            spn_FABRIC_CONNECT_MODE  ,
                                                            fabric_mesh_feature_kwywords
                                                        },
                                                        {
                                                            spn_USE_FABRIC_LINKS_FOR_ILKN_NIF  ,
                                                            fabric_ilkn_feature_range
                                                        },
                                                        {
                                                             spn_EXT_RAM_PRESENT  ,
                                                             ext_ram_feature_range
                                                        },
                                                        {
                                                             spn_CORE_CLOCK_SPEED  ,
                                                             core_freq_720_feature_range
                                                        },
                                                        {
                                                             spn_CORE_CLOCK_SPEED  ,
                                                             core_freq_325_feature_range
                                                        },
                                                        {
                                                             spn_SERDES_QRTT_ACTIVE  ,
                                                             fabric_18_quads_feature_range
                                                        },
                                                        {
                                                             spn_SERDES_QRTT_ACTIVE  ,
                                                             fabric_24_quads_feature_range
                                                        },
                                                        {
                                                             spn_SERDES_QRTT_ACTIVE  ,
                                                             fabric_12_quads_feature_range
                                                        },
                                                        {
                                                             spn_SERDES_QRTT_ACTIVE  ,
                                                             fabric_24_feature_range
                                                        }

                                                    };



/*    feature device map. for each feature we connect list of devices for them the feature disabled*/
static int no_metro_devices[] = 
    {
         BCM88674_DEVICE_ID,
         BCM88674_DEVICE_ID,
         BCM88676_DEVICE_ID,
         BCM88676M_DEVICE_ID,
         BCM88376_DEVICE_ID,
         BCM88376M_DEVICE_ID,
         INVALID_DEV_ID
    }; /* DNXC_NO_METRO_FEATURE*/
static int no_dc_devices[] = 
    {
        BCM88671_DEVICE_ID,
        BCM88671M_DEVICE_ID,
        BCM88673_DEVICE_ID,
        BCM88674_DEVICE_ID,
        BCM88677_DEVICE_ID,
        BCM88678_DEVICE_ID,
        BCM88371_DEVICE_ID,
        BCM88371M_DEVICE_ID,
        BCM88377_DEVICE_ID,
        BCM88378_DEVICE_ID,
        BCM88381_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_NO_DC_FEATURE*/

static int no_otn_devices[] = 
    {
        BCM88671_DEVICE_ID,
        BCM88671M_DEVICE_ID,
        BCM88675_DEVICE_ID,
        BCM88675M_DEVICE_ID,
        BCM88676_DEVICE_ID,
        BCM88676M_DEVICE_ID,
        BCM88370_DEVICE_ID,
        BCM88371_DEVICE_ID,
        BCM88371M_DEVICE_ID,
        BCM88375_DEVICE_ID,
        BCM88376_DEVICE_ID,
        BCM88376M_DEVICE_ID,
        BCM88477_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_NO_OTN_FEATURE*/

static int jer2_jer_nif_24_44_devices[] =
    {
        BCM88670_DEVICE_ID,
        BCM88673_DEVICE_ID,
        BCM88674_DEVICE_ID,
        BCM88377_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_JER2_JER_NIF_24_44_FEATURE*/

static int jer2_jer_nif_24_48_devices[] =
    {
        BCM88682_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_JER2_JER_NIF_24_48_FEATURE*/

int jer2_jer_nif_44_16_devices[] =
    {
        BCM88683_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_JER2_JER_NIF_44_16_FEATURE*/

static int jer2_jer_nif_48_4_devices[] =
    {
        BCM88677_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_JER2_JER_NIF_48_4_FEATURE*/

static int jer2_jer_nif_40_4_devices[] =
    {
        BCM88471_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_JER2_JER_NIF_40_4_FEATURE*/

static int jer2_jer_nif_12_32_devices[] =
    {
        BCM88474_DEVICE_ID,
        BCM88474H_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_JER2_JER_NIF_12_32_FEATURE*/

static int jer2_qax_nif_10_32_devices[] =
    {
        BCM88477_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_JER2_QAX_NIF_10_32_FEATURE*/

static int jer2_qax_nif_12_24_devices[] =
    {
        BCM88474_DEVICE_ID,
        BCM88474H_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_JER2_QAX_NIF_12_24_FEATURE*/

static int jer2_qax_nif_16_24_devices[] =
    {
        BCM88477_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_JER2_QAX_NIF_16_24_FEATURE*/

static int jer2_jer_ilkn_4_ports_devices[] =
    {
        BCM88670_DEVICE_ID,
        BCM88672_DEVICE_ID,
        BCM88673_DEVICE_ID,
        BCM88674_DEVICE_ID,
        BCM88677_DEVICE_ID,
        BCM88377_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_JER2_JER_4_ILKN_PORTS_FEATURE*/

static int jer2_jer_ilkn_2_ports_devices[] =
    {
        INVALID_DEV_ID
    }; /* DNXC_JER2_JER_2_ILKN_PORTS_FEATURE*/

static int jer2_qax_caui4_3_ports_devices[] =
    {
        BCM88474H_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_JER2_QAX_2_CAUI4_PORTS_FEATURE*/

static int jer2_qax_caui4_0_ports_devices[] =
    {
        BCM88471_DEVICE_ID,
        BCM88474_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_JER2_QAX_0_CAUI4_PORTS_FEATURE*/

static int fabric_18_quads_devices[] = 
    {
        BCM88777_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_FABRIC_18_QUADS_FEATURE*/

static int fabric_24_quads_devices[] = 
    {
        BCM88773_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_FABRIC_24_QUADS_FEATURE*/

static int fabric_12_quads_devices[] = 
    {
        BCM88774_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_FABRIC_12_QUADS_FEATURE*/
	
static int jer2_jer_24_fabric_devices[] = 
    {
       INVALID_DEV_ID
    }; /* DNXC_JER2_JER_24_FABRIC*/

static int no_fabric_ilkn_devices[] = 
    {
        BCM88670_DEVICE_ID,
        BCM88672_DEVICE_ID,
        BCM88673_DEVICE_ID,
        BCM88674_DEVICE_ID,
        BCM88378_DEVICE_ID,
       INVALID_DEV_ID
    }; /* DNXC_NO_FABRIC_ILKN_FEATURE*/

static int no_fabric_mesh_devices[] = 
    {
        BCM88671M_DEVICE_ID,
        BCM88676M_DEVICE_ID,
        BCM88677_DEVICE_ID,
        BCM88370_DEVICE_ID,
        BCM88371_DEVICE_ID,
        BCM88376_DEVICE_ID,
        BCM88378_DEVICE_ID,
        BCM88381_DEVICE_ID,
        BCM88471_DEVICE_ID,
        BCM88474_DEVICE_ID,
        BCM88474H_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_NO_FABRIC_MESH_FEATURE*/

static int single_stage_fe2_devices[] =
    {
        BCM88773_DEVICE_ID,
        BCM88774_DEVICE_ID,
        BCM88775_DEVICE_ID,
        BCM88776_DEVICE_ID,
        BCM88777_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_SINGLE_STAGE_FE2_FEATURE*/

static int no_ext_ram_devices[] =
    {
        BCM88677_DEVICE_ID,
        BCM88683_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_NO_EXT_RAM_FEATURE*/

static int core_freq_720_devices[] =
    {
        BCM88474_DEVICE_ID,
        INVALID_DEV_ID
    }; /* DNXC_CORE_FREQ_720_FEATURE*/

static int core_freq_325_devices[] =
    {
         BCM88471_DEVICE_ID,
         BCM88474_DEVICE_ID,
         INVALID_DEV_ID
    }; /* DNXC_CORE_FREQ_325_FEATURE*/

static int no_extended_lpm_devices[] =
    {
         BCM88681_DEVICE_ID,
         BCM88682_DEVICE_ID,
         BCM88683_DEVICE_ID,
         INVALID_DEV_ID
    }; /* DNXC_NO_EXTENDED_LPM_FEATURE*/

/*This array must aligned with the ENUM dnxc_feature_e in file dnxc_dev_feature_manager.h*/
static int   *device_features_map[DNXC_NUM_OF_FEATURES] = {
    no_metro_devices,
    no_dc_devices,
    no_otn_devices,
    jer2_jer_nif_24_44_devices,
    jer2_jer_nif_24_48_devices,
    jer2_jer_nif_44_16_devices,
    jer2_jer_nif_48_4_devices,
    jer2_jer_nif_40_4_devices,
    jer2_jer_nif_12_32_devices,
    jer2_qax_nif_10_32_devices,
    jer2_qax_nif_12_24_devices,
    jer2_qax_nif_16_24_devices,
    jer2_jer_ilkn_4_ports_devices,
    jer2_jer_ilkn_2_ports_devices,
    jer2_qax_caui4_3_ports_devices,
    jer2_qax_caui4_0_ports_devices,
    fabric_18_quads_devices,
    fabric_24_quads_devices,
    fabric_12_quads_devices,
    jer2_jer_24_fabric_devices,
    no_fabric_ilkn_devices,
    no_fabric_mesh_devices,
    single_stage_fe2_devices,
    no_ext_ram_devices,
    core_freq_720_devices,
    core_freq_325_devices,
    no_extended_lpm_devices
    };


soc_error_t dnxc_is_property_value_permited(int unit,const char *property,void *value) {
    int i,j,k;
    int len = sizeof(properies_feature_map)/sizeof(dnxc_property_features_t);
    uint16 dev_id; 
    uint8 rev_id;
    dnxc_feature_cb_t  *features_array;
    dnxc_feature_t feature;
    int rc;

    DNXC_INIT_FUNC_DEFS;

    soc_cm_get_id(unit, &dev_id, &rev_id);

    for (i=0;i<len;i++) {

        /*  continue if not the soc property entry*/
        if (sal_strcmp(property,properies_feature_map[i].property)) {
            continue;
        }
        /*  getting the features entry for soc property*/
        features_array = properies_feature_map[i].features_array;
        for (j=0;features_array[j].feature!=DNXC_INVALID_FEATURE;j++) {
            feature = features_array[j].feature;
            /* scanning device list for feature to see if our device belong */
            for (k=0;device_features_map[feature][k]!=INVALID_DEV_ID && device_features_map[feature][k]!=dev_id;k++) {
            }
            if (device_features_map[feature][k]!=dev_id) {
                continue;
            }
            /* at this point it turn out that our device  is block to some of the soc property values so we need to check*/
            rc = features_array[j].cb(unit,value,features_array[j].nof_ranges,features_array[j].data);
            if (rc!= SOC_E_NONE) {
                DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Property:%s  blocked for device %s\n"), property, soc_dev_name(unit)));  
            }

            DNXC_IF_ERR_EXIT(rc);


        }
    }

exit:
    DNXC_FUNC_RETURN;




}


soc_error_t dnxc_property_get(int unit, const char *name, uint32 defl,uint32 *value)
{
    DNXC_INIT_FUNC_DEFS;
    *value = soc_property_get(unit,name, defl);

    DNXC_IF_ERR_EXIT(dnxc_is_property_value_permited(unit,name,value));

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t dnxc_property_suffix_num_get(int unit, int num, const char *name, const char *suffix, uint32 defl,uint32 *value)
{
    DNXC_INIT_FUNC_DEFS;
    *value = soc_property_suffix_num_get(unit, num, name, suffix, defl);

    DNXC_IF_ERR_EXIT(dnxc_is_property_value_permited(unit,name,value));

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t dnxc_property_get_str(int unit, const char *name,char **value)
{
    DNXC_INIT_FUNC_DEFS;
    *value = soc_property_get_str(unit,name);
    DNXC_IF_ERR_EXIT(dnxc_is_property_value_permited(unit,name,*value));

exit:
    DNXC_FUNC_RETURN;

}

uint8       dnxc_device_block_for_feature(int unit,dnxc_feature_t feature)
{
    uint32 i;
    uint16 dev_id;
    uint8 rev_id;
    soc_cm_get_id(unit, &dev_id, &rev_id);
    for (i=0;device_features_map[feature][i]!=INVALID_DEV_ID && device_features_map[feature][i]!=dev_id;i++) {
    }

    return device_features_map[feature][i]==dev_id;

}

