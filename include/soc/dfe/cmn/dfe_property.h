/*
 * $Id: dfe_property.h,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * DFE PROPERTY H
 */
 
#ifndef _SOC_DFE_PROPERTY_H_
#define _SOC_DFE_PROPERTY_H_

/**********************************************************/
/*                  Includes                              */
/**********************************************************/
#include <soc/error.h>
#include <soc/drv.h>

/**********************************************************/
/*                  Structures                            */
/**********************************************************/

typedef struct soc_dfe_property_info_s {
    char* str;
    char* def_str;
    int def_int;
    int def_type; /* 1 for string, 2 for int */
} soc_dfe_property_info_t ;

typedef struct soc_dfe_property_str_enum_s {
    int enum_val;
    char *str;
} soc_dfe_property_str_enum_t ;

/**********************************************************/
/*                  Defines                               */
/**********************************************************/

#define SOC_DFE_PROPERTY_DEFAULT_TYPE_STRING 1
#define SOC_DFE_PROPERTY_DEFAULT_TYPE_INT 2
#define SOC_DFE_PROPERTY_UNAVAIL -1

/**********************************************************/
/*                  Functions                             */
/**********************************************************/

int soc_dfe_property_suffix_num_get(int unit,int num, char* soc_property_name,const char* suffix, int force_use_default, int soc_prop_default);
int soc_dfe_property_suffix_num_get_only_suffix(int unit, int num, char* soc_propert_name, const char* suffix, int force_use_default, int soc_prop_default);
int soc_dfe_property_get(int unit,char* soc_property_name,int force_use_default, int soc_prop_default);
char* soc_dfe_property_get_str(int unit,char* soc_property_name,int force_use_default, char* soc_prop_default);
char* soc_dfe_property_port_get_str(int unit, char* soc_property_name, soc_port_t port, int force_use_default, char* soc_prop_default);
int soc_dfe_property_port_get(int unit, char* soc_property_name, soc_port_t port, int force_use_default, int soc_prop_default);

void soc_dfe_check_soc_property(int unit,char* soc_property_name,int* is_supported,soc_dfe_property_info_t* soc_property_info);

/* 
 *Property string values to enum
 *Both function assumes that the last entry is NULL.
 */
soc_error_t soc_dfe_property_str_to_enum(int unit, char *soc_property_name, const soc_dfe_property_str_enum_t *property_info, char *str_val, int *int_val);
soc_error_t soc_dfe_property_enum_to_str(int unit, char *soc_property_name, const soc_dfe_property_str_enum_t *property_info, int int_val, char **str_val);

/**********************************************************/
/*                  Constant                              */
/**********************************************************/
/*Soc property: fabric_device_mode - available values*/
extern const soc_dfe_property_str_enum_t soc_dfe_property_str_enum_fabric_device_mode[];
/*Soc property: fabric_multicast_mode - available values*/
extern const soc_dfe_property_str_enum_t soc_dfe_property_str_enum_fabric_multicast_mode[];
/*Soc property: fabric_load_balancing_mode - available values*/
extern const soc_dfe_property_str_enum_t soc_dfe_property_str_enum_fabric_load_balancing_mode[];
/*Soc property: fabric_cell_format - available values*/
extern const soc_dfe_property_str_enum_t soc_dfe_property_str_enum_fabric_cell_format[];
/*Soc property: fabric_tdm_priority_min - available values*/
extern const soc_dfe_property_str_enum_t soc_dfe_property_str_enum_fabric_tdm_priority_min[];
/*Soc property: fe_mc_id_range - available values*/
extern const soc_dfe_property_str_enum_t soc_dfe_property_str_enum_fe_mc_id_range[];
/*Soc property: backplane_serdes_encoding - available values*/
extern const soc_dfe_property_str_enum_t soc_dfe_property_str_enum_backplane_serdes_encoding[];

#endif /* SOC_DFE_PROPERTY_H */
