/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef __QAX_INGRESS_SCHEDULER_INCLUDED__

/*************
 * DEFINES   *
 *************/

/* The actual shaper numbers in PTS block scheme for CLOS/MESH */
#define QAX_INGRESS_SCHEDULER_HP_MC_SHAPER_INDEX    (5)
#define QAX_INGRESS_SCHEDULER_LP_MC_SHAPER_INDEX    (6)

/* Indexes of HP/LP_MC_SHAPERs configurations for slow-start mechanism in PTS_SHAPER_FMC_CFGm for CLOS/MESH */
#define QAX_INGRESS_SCHEDULER_HP_MC_SHAPER_SLOW_START_CONFIG_INDEX    (0)
#define QAX_INGRESS_SCHEDULER_LP_MC_SHAPER_SLOW_START_CONFIG_INDEX    (1)


/*********************************************************************
*     This procedure returns the whole slow start configuration
*     (rates, enable/disable) of the clos scheduling scheme.                                                                                                                                                                                              .
*********************************************************************/
soc_error_t
qax_ingress_scheduler_clos_slow_start_get (
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_OUT SOC_TMC_ING_SCH_CLOS_INFO   *clos_info
    );

/*********************************************************************
*     This procedure configures the slow start configuration
*     (rates/enable/disable) of the clos scheduling scheme                                                                                                                                                                                              .
*********************************************************************/
soc_error_t
qax_ingress_scheduler_clos_slow_start_set (
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN SOC_TMC_ING_SCH_CLOS_INFO   *clos_info
    );

/*********************************************************************
*     This procedure returns the whole slow start configuration
*     (rates, enable/disable) of the mesh scheduling scheme                                                                                                                                                                                              .
*********************************************************************/
soc_error_t
qax_ingress_scheduler_mesh_slow_start_get (
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_OUT SOC_TMC_ING_SCH_SHAPER   *shaper_info
    );

/*********************************************************************
*     This procedure configures the slow start configuration
*     (rates/enable/disable) of the mesh scheduling scheme                                                                                                                                                                                              .
*********************************************************************/
soc_error_t
qax_ingress_scheduler_mesh_slow_start_set (
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN SOC_TMC_ING_SCH_SHAPER   *shaper_info
    );


/* } __QAX_INGRESS_SCHEDULER_INCLUDED__*/
#endif
