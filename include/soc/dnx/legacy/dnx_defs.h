/*
* $Id$
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
*
* 
*/
#ifndef SOC_DNX_DNX_DEFS_H
#define SOC_DNX_DNX_DEFS_H


typedef enum soc_dnx_stat_path_drop_stage_e {
    soc_dnx_stat_path_drop_stage_none = 0,
    soc_dnx_stat_path_drop_stage_ingress_no_packet = 1,
    soc_dnx_stat_path_drop_stage_ingress_tm = 2,
    soc_dnx_stat_path_drop_stage_egress_tm = 3,

    /*must be last*/
    soc_dnx_stat_path_drop_stage_nof = 4
} soc_dnx_stat_path_drop_stage_t;


typedef struct soc_dnx_stat_path_info_s {
    int ingress_core;
    int egress_core;
    soc_dnx_stat_path_drop_stage_t drop;
} soc_dnx_stat_path_info_t;




#endif /* SOC_DNX_DNX_DEFS_H */

