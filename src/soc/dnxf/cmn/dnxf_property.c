/*
 * $Id: dnxf_property.c,v 1.68.6.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_BCM_INIT

#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>

#include <soc/dnxf/cmn/dnxf_drv.h>
#include <soc/dnxf/cmn/mbcm.h>
#include <soc/dnxf/cmn/dnxf_property.h>

void
soc_dnxf_check_soc_property(int unit,char* soc_property_name,int* is_supported,soc_dnxf_property_info_t* soc_property_info)
{
    soc_dnxf_property_info_t* soc_dnxf_property_info_array = NULL;
    int i=0;
    *is_supported=0;

    MBCM_DNXF_DRIVER_CALL_VOID(unit,mbcm_dnxf_get_soc_properties_array,(unit,&soc_dnxf_property_info_array));

    if (soc_dnxf_property_info_array != NULL)
    {
        while (soc_dnxf_property_info_array[i].str != NULL) 
        {
            if (sal_strcmp(soc_dnxf_property_info_array[i].str,soc_property_name)==0)
            {
                *is_supported=1;
                *soc_property_info=soc_dnxf_property_info_array[i];
            }
            i++;
        }
    }
}

char*
soc_dnxf_property_get_str(int unit,char* soc_property_name, int force_use_default, char* soc_prop_default)
{
    return soc_dnxf_property_port_get_str(unit, soc_property_name, -1, force_use_default, soc_prop_default);
}

char*
soc_dnxf_property_port_get_str(int unit,char* soc_property_name, soc_port_t port, int force_use_default, char* soc_prop_default)
{
    char* str;
    char* rv;
    int is_soc_property_supported;
    soc_dnxf_property_info_t soc_property_info;

    soc_dnxf_check_soc_property(unit,soc_property_name ,&is_soc_property_supported,&soc_property_info);
    if (is_soc_property_supported)
    {
        if (port == -1)
        {
            str = soc_property_get_str(unit,soc_property_name);
        } else {
            str = soc_property_port_get_str(unit, port, soc_property_name);
        }
        if (str == NULL)
        {
            if (force_use_default)
            {
                rv = soc_prop_default;
            } else {
                if (soc_property_info.def_type==SOC_DNXF_PROPERTY_DEFAULT_TYPE_STRING)
                {
                    rv = soc_property_info.def_str;
                } else {
                    rv = NULL;
                }
            }

        } else {
            rv = str;
        }
    } else {
        rv = NULL;
    }
    if (port == -1)
    {
         LOG_DEBUG(BSL_LS_SOC_INIT,
                   (BSL_META_U(unit,
                               "unit %d: %s=%s\n"), unit, soc_property_name,(rv!=NULL)? rv:"NULL"));
    } else {
         LOG_DEBUG(BSL_LS_SOC_INIT,
                   (BSL_META_U(unit,
                               "unit %d: %s_%d=%s\n"), unit, soc_property_name, port, (rv!=NULL)? rv:"NULL"));
    }
    return rv;


}

int
soc_dnxf_property_port_get(int unit,  char* soc_property_name, soc_port_t port, int force_use_default, int soc_prop_default)
{
    int is_soc_property_supported;
    soc_dnxf_property_info_t soc_property_info;
    int rv;

    soc_dnxf_check_soc_property(unit,soc_property_name ,&is_soc_property_supported,&soc_property_info);
    if (is_soc_property_supported)
    {
        if (force_use_default)
        {
            rv = soc_property_port_get(unit, port, soc_property_name,soc_prop_default);
        } else {
            if (soc_property_info.def_type==SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT)
            {
                rv = soc_property_port_get(unit, port, soc_property_name, soc_property_info.def_int);
            } else {
                rv = SOC_DNXF_PROPERTY_UNAVAIL;
            }
        }
    } else {
        rv = SOC_DNXF_PROPERTY_UNAVAIL;
    }
     LOG_DEBUG(BSL_LS_SOC_INIT,
                   (BSL_META_U(unit,
                               "unit %d, port %d: %s=%d\n"),unit, port, soc_property_name,rv));
    return rv;

}


int
soc_dnxf_property_get(int unit,char* soc_property_name,int force_use_default, int soc_prop_default)
{
    int is_soc_property_supported;
    soc_dnxf_property_info_t soc_property_info;
    int rv;

    soc_dnxf_check_soc_property(unit,soc_property_name ,&is_soc_property_supported,&soc_property_info);
    if (is_soc_property_supported)
    {
        if (force_use_default)
        {
            rv = soc_property_get(unit, soc_property_name,soc_prop_default);
        } else {
            if (soc_property_info.def_type==SOC_DNXF_PROPERTY_DEFAULT_TYPE_INT)
            {
                rv = soc_property_get(unit,soc_property_name,soc_property_info.def_int);
            } else {
                rv = SOC_DNXF_PROPERTY_UNAVAIL;
            }
        }
    } else {
        rv = SOC_DNXF_PROPERTY_UNAVAIL;
    }
     LOG_DEBUG(BSL_LS_SOC_INIT,
                   (BSL_META_U(unit,
                               "unit %d: %s=%d\n"),unit, soc_property_name,rv));
    return rv;

}

int
soc_dnxf_property_suffix_num_get(int unit,int num, char* soc_property_name,const char* suffix, int force_use_default, int soc_prop_default)
{
    int is_soc_property_supported;
    soc_dnxf_property_info_t soc_property_info;
    int rv;

    soc_dnxf_check_soc_property(unit,soc_property_name,&is_soc_property_supported,&soc_property_info);
    if (is_soc_property_supported)
    {

        if (force_use_default)
        {
            rv= soc_property_suffix_num_get(unit,num,soc_property_name,suffix,soc_prop_default);
        }
        
        else
        {
            rv=soc_property_suffix_num_get(unit,num,soc_property_name,suffix,soc_property_info.def_int);
        }
    }
    else
    {
        rv= SOC_DNXF_PROPERTY_UNAVAIL;
    }

     LOG_DEBUG(BSL_LS_SOC_INIT,
                   (BSL_META_U(unit,
                               "unit %d: %s_%s_%d=%d\n"),unit, soc_property_name,suffix,num,rv));
    return rv;  
}

int
soc_dnxf_property_suffix_num_get_only_suffix(int unit, int num, char* soc_property_name, const char* suffix, int force_use_default, int soc_prop_default)
{
    int is_soc_property_supported;
    soc_dnxf_property_info_t soc_property_info;
    int rv;

    soc_dnxf_check_soc_property(unit,soc_property_name,&is_soc_property_supported,&soc_property_info);
    if (is_soc_property_supported)
    {

        if (force_use_default)
        {
            rv= soc_property_suffix_num_get_only_suffix(unit,num,soc_property_name,suffix,soc_prop_default);
        }
        
        else
        {
            rv=soc_property_suffix_num_get_only_suffix(unit,num,soc_property_name,suffix,soc_property_info.def_int);
        }
    }
    else
    {
        rv= SOC_DNXF_PROPERTY_UNAVAIL;
    }

     LOG_DEBUG(BSL_LS_SOC_INIT,
                   (BSL_META_U(unit,
                               "unit %d: %s_%s_%d=%d\n"),unit, soc_property_name,suffix,num,rv));
    return rv;  
}


soc_error_t
soc_dnxf_property_str_to_enum(int unit, char *soc_property_name, const soc_dnxf_property_str_enum_t *property_info, char *str_val, int *int_val)
{
    int i;
    DNXC_INIT_FUNC_DEFS;

    *int_val = 0;

    /*Looking for the string value at property_info*/
    for (i=0; property_info[i].str != NULL; i++)
    {
        if (str_val != NULL)
        {
            if (sal_strcmp(str_val, property_info[i].str) == 0)
            {
                *int_val = property_info[i].enum_val;
                SOC_EXIT;
            }
        }
    }

    
    if (str_val == NULL)
    {
        /*NULL states the default value*/
        *int_val = property_info[i].enum_val;
    } else {
        /*not found - generating an error*/
        DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Property %s has unrecognized value %s"),soc_property_name, str_val));
    }

exit:
    DNXC_FUNC_RETURN;
}
soc_error_t 
soc_dnxf_property_enum_to_str(int unit, char *soc_property_name, const soc_dnxf_property_str_enum_t *property_info, int int_val, char **str_val)
{
    int i;
    DNXC_INIT_FUNC_DEFS;

    /*Looking for the string value at property_info*/
    for (i=0; property_info[i].str != NULL; i++)
    {
            if (int_val == property_info[i].enum_val)
            {
                *str_val = property_info[i].str;
                SOC_EXIT;
            }
    }

    /*Not found - generating an error*/
    DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Property %s has unrecognized value enum %d"),soc_property_name, int_val));

exit:
    DNXC_FUNC_RETURN;
}

/**********************************************************/
/*          SoC property availbale values                 */
/**********************************************************/


/*fabric_device_mode available values*/
const soc_dnxf_property_str_enum_t soc_dnxf_property_str_enum_fabric_device_mode[] = {
    {soc_dnxf_fabric_device_mode_single_stage_fe2,   "SINGLE_STAGE_FE2"},
    {soc_dnxf_fabric_device_mode_multi_stage_fe2,    "MULTI_STAGE_FE2"},
    {soc_dnxf_fabric_device_mode_multi_stage_fe13,   "MULTI_STAGE_FE13"},
    {soc_dnxf_fabric_device_mode_repeater,           "REPEATER"},
    {soc_dnxf_fabric_device_mode_multi_stage_fe13_asymmetric, "MULTI_STAGE_FE13_ASYMMETRICAL"},
    {DNXF_FABRIC_DEVICE_MODE_DEFAULT,   NULL}
};

/*fabric_multicast_mode available values*/
const soc_dnxf_property_str_enum_t soc_dnxf_property_str_enum_fabric_multicast_mode[] = {
    {soc_dnxf_multicast_mode_direct,     "DIRECT"},
    {soc_dnxf_multicast_mode_indirect,   "INDIRECT"},
    {DNXF_FABRIC_MULTICAST_MODE_DEFAULT, NULL}
};

const soc_dnxf_property_str_enum_t soc_dnxf_property_str_enum_backplane_serdes_encoding[] = {
    {soc_dnxc_port_pcs_8_9_legacy_fec,       "0"},
    {soc_dnxc_port_pcs_8_10,                 "1"},
    {soc_dnxc_port_pcs_64_66_fec,            "2"},
    {soc_dnxc_port_pcs_64_66_bec,            "3"},
    {soc_dnxc_port_pcs_64_66,                "4"},
    {soc_dnxc_port_pcs_64_66_rs_fec,         "5"},
    {soc_dnxc_port_pcs_64_66_ll_rs_fec,      "6"},
    {soc_dnxc_port_pcs_64_66_fec,            "KR_FEC"},
    {soc_dnxc_port_pcs_64_66,                "64_66"},
    {soc_dnxc_port_pcs_64_66_rs_fec,         "RS_FEC"},
    {soc_dnxc_port_pcs_64_66_ll_rs_fec,      "LL_RS_FEC"},
    {SOC_DNXF_PROPERTY_UNAVAIL,              NULL}                 
};

/*fabric_load_balancing_mode available values*/
const soc_dnxf_property_str_enum_t soc_dnxf_property_str_enum_fabric_load_balancing_mode[] = {
    {soc_dnxf_load_balancing_mode_normal,                    "NORMAL_LOAD_BALANCE"},
    {soc_dnxf_load_balancing_mode_destination_unreachable,   "DESTINATION_UNREACHABLE"},
    {soc_dnxf_load_balancing_mode_balanced_input,            "BALANCED_INPUT"},
    {DNXF_FABRIC_LOAD_BALANCING_MODE_DEFAULT,                NULL}
};

/*fabric_load_balancing_mode available values*/
const soc_dnxf_property_str_enum_t soc_dnxf_property_str_enum_fabric_cell_format[] = {
    {soc_dnxf_fabric_link_cell_size_VSC128,                "VSC128"},
    {soc_dnxf_fabric_link_cell_size_VSC256_V1,             "VSC256"}, /*for backward compatibility*/
    {SOC_DNXF_PROPERTY_UNAVAIL,                            NULL}
};

/*fabric_tdm_priority_min available values*/
const soc_dnxf_property_str_enum_t soc_dnxf_property_str_enum_fabric_tdm_priority_min[] = {
    {0,                                                 "0"},
    {1,                                                 "1"},
    {2,                                                 "2"},
    {3,                                                 "3"}, /*for backward compatibility*/
    {SOC_DNXF_FABRIC_TDM_PRIORITY_NONE,                  "NONE"},
    {SOC_DNXF_FABRIC_TDM_PRIORITY_DEFAULT,               NULL}
};

/*fe_mc_id_range available values*/
const soc_dnxf_property_str_enum_t soc_dnxf_property_str_enum_fe_mc_id_range[] = {
    {soc_dnxf_multicast_table_mode_64k,      "64K"},
    {soc_dnxf_multicast_table_mode_128k,     "128K"},
    {soc_dnxf_multicast_table_mode_128k_half,"128K_HALF"}, 
    {soc_dnxf_multicast_table_mode_64k,       NULL}
};


#undef _ERR_MSG_MODULE_NAME
