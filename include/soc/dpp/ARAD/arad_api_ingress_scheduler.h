/* $Id: arad_api_ingress_scheduler.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef __ARAD_API_INGRESS_SCHEDULER_INCLUDED__
/* { */
#define __ARAD_API_INGRESS_SCHEDULER_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/ARAD/arad_api_general.h>
#include <soc/dpp/TMC/tmc_api_ingress_scheduler.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Utils/sand_integer_arithmetic.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */
#define ARAD_ING_SCH_MAX_WEIGHT_VALUE                        63
#define ARAD_ING_SCH_MAX_MAX_CREDIT_VALUE                    65535
#define ARAD_ING_SCH_MAX_NOF_ENTRIES                         ARAD_ING_SCH_NUM_OF_CONTEXTS
#define ARAD_ING_SCH_MAX_ID_VALUE                            ARAD_ING_SCH_NUM_OF_CONTEXTS

#define ARAD_ING_SCH_CLOS_NOF_HP_SHAPERS       3
#define ARAD_ING_SCH_CLOS_NOF_GLOBAL_SHAPERS   2
#define ARAD_ING_SCH_CLOS_NOF_LP_SHAPERS       2
/* } */

/*************
 * MACROS    *
 *************/
/* { */

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */


#define ARAD_ING_SCH_MESH_LOCAL                           SOC_TMC_ING_SCH_MESH_LOCAL
#define ARAD_ING_SCH_MESH_CON1                            SOC_TMC_ING_SCH_MESH_CON1
#define ARAD_ING_SCH_MESH_CON2                            SOC_TMC_ING_SCH_MESH_CON2
#define ARAD_ING_SCH_MESH_CON3                            SOC_TMC_ING_SCH_MESH_CON3
#define ARAD_ING_SCH_MESH_CON4                            SOC_TMC_ING_SCH_MESH_CON4
#define ARAD_ING_SCH_MESH_CON5                            SOC_TMC_ING_SCH_MESH_CON5
#define ARAD_ING_SCH_MESH_CON6                            SOC_TMC_ING_SCH_MESH_CON6
#define ARAD_ING_SCH_MESH_CON7                            SOC_TMC_ING_SCH_MESH_CON7
#define ARAD_ING_SCH_MESH_LAST                            SOC_TMC_ING_SCH_MESH_LAST

#define ARAD_ING_SCH_DONT_TOUCH							  SOC_TMC_ING_SCH_DONT_TOUCH						  	   

typedef SOC_TMC_ING_SCH_MESH_CONTEXTS                          ARAD_ING_SCH_MESH_CONTEXTS;

typedef SOC_TMC_ING_SCH_SHAPER                                 ARAD_ING_SCH_SHAPER;
typedef SOC_TMC_ING_SCH_MESH_CONTEXT_INFO                      ARAD_ING_SCH_MESH_CONTEXT_INFO;
typedef SOC_TMC_ING_SCH_MESH_INFO                              ARAD_ING_SCH_MESH_INFO;
typedef SOC_TMC_ING_SCH_CLOS_WFQ_ELEMENT                       ARAD_ING_SCH_CLOS_WFQ_ELEMENT;
typedef SOC_TMC_ING_SCH_CLOS_WFQS                              ARAD_ING_SCH_CLOS_WFQS;
typedef SOC_TMC_ING_SCH_CLOS_HP_SHAPERS                        ARAD_ING_SCH_CLOS_HP_SHAPERS;
typedef SOC_TMC_ING_SCH_CLOS_LP_SHAPERS                        ARAD_ING_SCH_CLOS_LP_SHAPERS;
typedef SOC_TMC_ING_SCH_CLOS_SHAPERS                           ARAD_ING_SCH_CLOS_SHAPERS;
typedef SOC_TMC_ING_SCH_CLOS_INFO                              ARAD_ING_SCH_CLOS_INFO;

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */

/*********************************************************************
* NAME:
*     arad_ingress_scheduler_mesh_set
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     MESH fabric (no FE) configure the ingress scheduler. The
*     configuration includes:[per-destination]-shaper-rates,
*     [per-destination]-weights. NOTES:1. The 'Verify'
*     function - includes Verification of weight and shaper
*     values range. For Mesh also verification of the
*     'nof_entries' and 'id' fields in the INFO structure.2.
*     The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Mesh
*     configuration, the ARAD_ING_SCH_MESH_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  ARAD_ING_SCH_MESH_INFO   *mesh_info -
*     mesh_info pointer to configuration structure.
*  SOC_SAND_OUT ARAD_ING_SCH_MESH_INFO   *exact_mesh_info -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_ingress_scheduler_mesh_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ING_SCH_MESH_INFO   *mesh_info,
    SOC_SAND_OUT ARAD_ING_SCH_MESH_INFO   *exact_mesh_info
  );

/*********************************************************************
* NAME:
*     arad_ingress_scheduler_mesh_bandwidth_set
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     MESH fabric (no FE) configure the ingress scheduler. The
*     configuration includes:[per-destination]-shaper-rates,
*     [per-destination]-weights. NOTES:1. The 'Verify'
*     function - includes Verification of weight and shaper
*     values range. For Mesh also verification of the
*     'nof_entries' and 'id' fields in the INFO structure.2.
*     The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Mesh
*     configuration, the ARAD_ING_SCH_MESH_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 gport -
*  SOC_SAND_IN  uint32              rate -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_ingress_scheduler_mesh_bandwidth_set(
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_IN  uint32              rate
    );

/*********************************************************************
* NAME:
*     arad_ingress_scheduler_mesh_sched_set
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     MESH fabric (no FE) configure the ingress scheduler. The
*     configuration includes:[per-destination]-shaper-rates,
*     [per-destination]-weights. NOTES:1. The 'Verify'
*     function - includes Verification of weight and shaper
*     values range. For Mesh also verification of the
*     'nof_entries' and 'id' fields in the INFO structure.2.
*     The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Mesh
*     configuration, the ARAD_ING_SCH_MESH_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 gport -
*  SOC_SAND_IN  int                 weight -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_ingress_scheduler_mesh_sched_set(
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_IN  int                 weight
    );

/*********************************************************************
* NAME:
*     arad_ingress_scheduler_mesh_burst_set
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     MESH fabric (no FE) configure the ingress scheduler. The
*     configuration includes:[per-destination]-shaper-rates,
*     [per-destination]-weights. NOTES:1. The 'Verify'
*     function - includes Verification of weight and shaper
*     values range. For Mesh also verification of the
*     'nof_entries' and 'id' fields in the INFO structure.2.
*     The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Mesh
*     configuration, the ARAD_ING_SCH_MESH_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 gport -
*  SOC_SAND_IN  int                 burst -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_ingress_scheduler_mesh_burst_set(
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_IN  int                 burst
    );
/*********************************************************************
* NAME:
*     arad_ingress_scheduler_mesh_get
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     MESH fabric (no FE) configure the ingress scheduler. The
*     configuration includes:[per-destination]-shaper-rates,
*     [per-destination]-weights. NOTES:1. The 'Verify'
*     function - includes Verification of weight and shaper
*     values range. For Mesh also verification of the
*     'nof_entries' and 'id' fields in the INFO structure.2.
*     The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Mesh
*     configuration, the ARAD_ING_SCH_MESH_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_OUT ARAD_ING_SCH_MESH_INFO   *mesh_info -
*     mesh_info pointer to configuration structure.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_ingress_scheduler_mesh_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT ARAD_ING_SCH_MESH_INFO   *mesh_info
  );

/*********************************************************************
* NAME:
*     arad_ingress_scheduler_mesh_get
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     MESH fabric (no FE) configure the ingress scheduler. The
*     configuration includes:[per-destination]-shaper-rates,
*     [per-destination]-weights. NOTES:1. The 'Verify'
*     function - includes Verification of weight and shaper
*     values range. For Mesh also verification of the
*     'nof_entries' and 'id' fields in the INFO structure.2.
*     The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Mesh
*     configuration, the ARAD_ING_SCH_MESH_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*    SOC_SAND_IN  int                 unit, 
*    SOC_SAND_IN  int                 gport, 
*    SOC_SAND_OUT uint32              *rate
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_ingress_scheduler_mesh_bandwidth_get(
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_OUT uint32              *rate
    );

/*********************************************************************
* NAME:
*     arad_ingress_scheduler_mesh_sched_get
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     MESH fabric (no FE) configure the ingress scheduler. The
*     configuration includes:[per-destination]-shaper-rates,
*     [per-destination]-weights. NOTES:1. The 'Verify'
*     function - includes Verification of weight and shaper
*     values range. For Mesh also verification of the
*     'nof_entries' and 'id' fields in the INFO structure.2.
*     The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Mesh
*     configuration, the ARAD_ING_SCH_MESH_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*    SOC_SAND_IN  int                 unit, 
*    SOC_SAND_IN  int                 gport, 
*    SOC_SAND_OUT int                 *weight
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_ingress_scheduler_mesh_sched_get(
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_OUT int                 *weight
    );

/*********************************************************************
* NAME:
*     arad_ingress_scheduler_mesh_burst_get
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     MESH fabric (no FE) configure the ingress scheduler. The
*     configuration includes:[per-destination]-shaper-rates,
*     [per-destination]-weights. NOTES:1. The 'Verify'
*     function - includes Verification of weight and shaper
*     values range. For Mesh also verification of the
*     'nof_entries' and 'id' fields in the INFO structure.2.
*     The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Mesh
*     configuration, the ARAD_ING_SCH_MESH_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*    SOC_SAND_IN  int                 unit, 
*    SOC_SAND_IN  int                 gport, 
*    SOC_SAND_OUT int                 *burst
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_ingress_scheduler_mesh_burst_get(
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_OUT int                 *burst
    );
/*********************************************************************
* NAME:
*     arad_ingress_scheduler_clos_set
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     This procedure configure the ingress scheduler when
*     working with SOC_SAND CLOS fabric (that is SOC_SAND_FE200/SOC_SAND_FE600). The
*     configuration includes:[local/fabric]-shaper-rates,
*     [local/fabric]-weights. NOTES:1. The 'Verify' function -
*     includes Verification of weight and shaper values range.
*     2. The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Clos
*     configuration, the ARAD_ING_SCH_CLOS_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  ARAD_ING_SCH_CLOS_INFO   *clos_info -
*     clos_info pointer to configuration structure.
*  SOC_SAND_OUT ARAD_ING_SCH_CLOS_INFO   *exact_clos_info -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_ingress_scheduler_clos_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_ING_SCH_CLOS_INFO   *clos_info
  );

/*********************************************************************
* NAME:
*     arad_ingress_scheduler_clos_set
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     This procedure configure the ingress scheduler rates when
*     working with SOC_SAND CLOS fabric (that is SOC_SAND_FE200/SOC_SAND_FE600). The
*     configuration includes:[local/fabric]-shaper-rates,
*     [local/fabric]-weights. NOTES:1. The 'Verify' function -
*     includes Verification of weight and shaper values range.
*     2. The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Clos
*     configuration, the ARAD_ING_SCH_CLOS_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 port -
*  SOC_SAND_IN  uint32              rate -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_ingress_scheduler_clos_bandwidth_set(
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_IN  uint32              rate
    );
/*********************************************************************
* NAME:
*     arad_ingress_scheduler_clos_sched_set
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     This procedure configure the ingress scheduler rates when
*     working with SOC_SAND CLOS fabric (that is SOC_SAND_FE200/SOC_SAND_FE600). The
*     configuration includes:[local/fabric]-shaper-rates,
*     [local/fabric]-weights. NOTES:1. The 'Verify' function -
*     includes Verification of weight and shaper values range.
*     2. The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Clos
*     configuration, the ARAD_ING_SCH_CLOS_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 port -
*  SOC_SAND_IN  int                 weight -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_ingress_scheduler_clos_sched_set(
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_IN  int                 weight
    );

/*********************************************************************
* NAME:
*     arad_ingress_scheduler_clos_burst_set
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     This procedure configure the ingress scheduler rates when
*     working with SOC_SAND CLOS fabric (that is SOC_SAND_FE200/SOC_SAND_FE600). The
*     configuration includes:[local/fabric]-shaper-rates,
*     [local/fabric]-weights. NOTES:1. The 'Verify' function -
*     includes Verification of weight and shaper values range.
*     2. The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Clos
*     configuration, the ARAD_ING_SCH_CLOS_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 port -
*  SOC_SAND_IN  int                 burst -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_ingress_scheduler_clos_burst_set(
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_IN  int                 burst
    );
/*********************************************************************
* NAME:
*     arad_ingress_scheduler_clos_sched_set
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     This procedure configure the ingress scheduler rates when
*     working with SOC_SAND CLOS fabric (that is SOC_SAND_FE200/SOC_SAND_FE600). The
*     configuration includes:[local/fabric]-shaper-rates,
*     [local/fabric]-weights. NOTES:1. The 'Verify' function -
*     includes Verification of weight and shaper values range.
*     2. The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Clos
*     configuration, the ARAD_ING_SCH_CLOS_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 port -
*  SOC_SAND_OUT int                 *weight -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_ingress_scheduler_clos_sched_get(
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_OUT int                 *weight
    );

/*********************************************************************
* NAME:
*     arad_ingress_scheduler_clos_burst_get
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     This procedure configure the ingress scheduler rates when
*     working with SOC_SAND CLOS fabric (that is SOC_SAND_FE200/SOC_SAND_FE600). The
*     configuration includes:[local/fabric]-shaper-rates,
*     [local/fabric]-weights. NOTES:1. The 'Verify' function -
*     includes Verification of weight and shaper values range.
*     2. The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Clos
*     configuration, the ARAD_ING_SCH_CLOS_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 port -
*  SOC_SAND_OUT int                 *burst -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_ingress_scheduler_clos_burst_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_OUT int                 *burst
  );
/*********************************************************************
* NAME:
*     arad_ingress_scheduler_clos_get
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     This procedure configure the ingress scheduler when
*     working with SOC_SAND CLOS fabric (that is SOC_SAND_FE200/SOC_SAND_FE600). The
*     configuration includes:[local/fabric]-shaper-rates,
*     [local/fabric]-weights. NOTES:1. The 'Verify' function -
*     includes Verification of weight and shaper values range.
*     2. The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Clos
*     configuration, the ARAD_ING_SCH_CLOS_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_OUT ARAD_ING_SCH_CLOS_INFO   *clos_info -
*     clos_info pointer to configuration structure.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_ingress_scheduler_clos_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_OUT ARAD_ING_SCH_CLOS_INFO   *clos_info
  );

/*********************************************************************
* NAME:
*       arad_ingress_scheduler_clos_bandwidth_get
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     This procedure configure the ingress scheduler when
*     working with SOC_SAND CLOS fabric (that is SOC_SAND_FE200/SOC_SAND_FE600). The
*     configuration includes:[local/fabric]-shaper-rates,
*     [local/fabric]-weights. NOTES:1. The 'Verify' function -
*     includes Verification of weight and shaper values range.
*     2. The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Clos
*     configuration, the ARAD_ING_SCH_CLOS_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 gport -
*  SOC_SAND_OUT uint32              *rate -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_ingress_scheduler_clos_bandwidth_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_OUT uint32              *rate
  );

void
  arad_ARAD_ING_SCH_SHAPER_clear(
    SOC_SAND_OUT ARAD_ING_SCH_SHAPER *info
  );

void
  arad_ARAD_ING_SCH_MESH_CONTEXT_INFO_clear(
    SOC_SAND_OUT ARAD_ING_SCH_MESH_CONTEXT_INFO *info
  );

void
  arad_ARAD_ING_SCH_MESH_INFO_clear(
    SOC_SAND_OUT ARAD_ING_SCH_MESH_INFO *info
  );

void
  arad_ARAD_ING_SCH_CLOS_WFQ_ELEMENT_clear(
    SOC_SAND_OUT ARAD_ING_SCH_CLOS_WFQ_ELEMENT *info
  );

void
  arad_ARAD_ING_SCH_CLOS_WFQS_clear(
    SOC_SAND_OUT ARAD_ING_SCH_CLOS_WFQS *info
  );

void
  arad_ARAD_ING_SCH_CLOS_HP_SHAPERS_clear(
    SOC_SAND_OUT ARAD_ING_SCH_CLOS_HP_SHAPERS *info
  );

void
  arad_ARAD_ING_SCH_CLOS_SHAPERS_clear(
    SOC_SAND_OUT ARAD_ING_SCH_CLOS_SHAPERS *info
  );

void
  arad_ARAD_ING_SCH_CLOS_INFO_clear(
    SOC_SAND_OUT ARAD_ING_SCH_CLOS_INFO *info
  );

#if ARAD_DEBUG_IS_LVL1

const char*
  arad_ARAD_ING_SCH_MESH_CONTEXTS_to_string(
    SOC_SAND_IN ARAD_ING_SCH_MESH_CONTEXTS enum_val
  );

void
  arad_ARAD_ING_SCH_SHAPER_print(
    SOC_SAND_IN ARAD_ING_SCH_SHAPER *info
  );

void
  arad_ARAD_ING_SCH_MESH_CONTEXT_INFO_print(
    SOC_SAND_IN ARAD_ING_SCH_MESH_CONTEXT_INFO *info
  );

void
  arad_ARAD_ING_SCH_MESH_INFO_print(
    SOC_SAND_IN ARAD_ING_SCH_MESH_INFO *info
  );

void
  arad_ARAD_ING_SCH_CLOS_WFQ_ELEMENT_print(
    SOC_SAND_IN ARAD_ING_SCH_CLOS_WFQ_ELEMENT *info
  );

void
  arad_ARAD_ING_SCH_CLOS_WFQS_print(
    SOC_SAND_IN ARAD_ING_SCH_CLOS_WFQS *info
  );

void
  arad_ARAD_ING_SCH_CLOS_HP_SHAPERS_print(
    SOC_SAND_IN  ARAD_ING_SCH_CLOS_HP_SHAPERS *info
  );

void
  arad_ARAD_ING_SCH_CLOS_SHAPERS_print(
    SOC_SAND_IN ARAD_ING_SCH_CLOS_SHAPERS *info
  );

void
  arad_ARAD_ING_SCH_CLOS_INFO_print(
    SOC_SAND_IN ARAD_ING_SCH_CLOS_INFO *info
  );

void
  arad_ARAD_ING_SCH_CLOS_INFO_SHAPER_dont_touch(
    SOC_SAND_OUT ARAD_ING_SCH_CLOS_INFO *info
  );

void
  arad_ARAD_ING_SCH_MESH_INFO_SHAPERS_dont_touch(
    SOC_SAND_OUT ARAD_ING_SCH_MESH_INFO *info
  );

#endif /* ARAD_DEBUG_IS_LVL1 */

/*********************************************************************
* NAME:
*     arad_ingress_scheduler_conversion_test_api
* TYPE:
*   PROC
* DATE:
*   Oct 12 2007
* FUNCTION:
*     This procedure perform a test that compares the rate values to
*     the exact rate values that are received after the
*     conversion function.
*     The procedure tests 2 main criteria:
*     1. That the exact error percentage from the rate does not exceed limit.
*     2. That the exact is always larger than rate.
* INPUT:
*   SOC_SAND_IN uint8 is_regression - if true less values are checked.
* RETURNS:
*   SOC_SAND_DIRECT: uint8 pass - whether the test has passed or not.
*
*
*********************************************************************/

uint8
  arad_ingress_scheduler_conversion_test_api(
    SOC_SAND_IN uint8 is_regression,
    SOC_SAND_IN uint8 silent
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_API_INGRESS_SCHEDULER_INCLUDED__*/
#endif
