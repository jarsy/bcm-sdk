/* $Id: jer2_arad_api_ingress_scheduler.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef __JER2_ARAD_API_INGRESS_SCHEDULER_INCLUDED__
/* { */
#define __JER2_ARAD_API_INGRESS_SCHEDULER_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_api_general.h>
#include <soc/dnx/legacy/TMC/tmc_api_ingress_scheduler.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */
#define JER2_ARAD_ING_SCH_MAX_WEIGHT_VALUE                        63
#define JER2_ARAD_ING_SCH_MAX_MAX_CREDIT_VALUE                    65535
#define JER2_ARAD_ING_SCH_MAX_NOF_ENTRIES                         JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS
#define JER2_ARAD_ING_SCH_MAX_ID_VALUE                            JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS

#define JER2_ARAD_ING_SCH_CLOS_NOF_HP_SHAPERS       3
#define JER2_ARAD_ING_SCH_CLOS_NOF_GLOBAL_SHAPERS   2
#define JER2_ARAD_ING_SCH_CLOS_NOF_LP_SHAPERS       2
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


#define JER2_ARAD_ING_SCH_MESH_LOCAL                           DNX_TMC_ING_SCH_MESH_LOCAL
#define JER2_ARAD_ING_SCH_MESH_CON1                            DNX_TMC_ING_SCH_MESH_CON1
#define JER2_ARAD_ING_SCH_MESH_CON2                            DNX_TMC_ING_SCH_MESH_CON2
#define JER2_ARAD_ING_SCH_MESH_CON3                            DNX_TMC_ING_SCH_MESH_CON3
#define JER2_ARAD_ING_SCH_MESH_CON4                            DNX_TMC_ING_SCH_MESH_CON4
#define JER2_ARAD_ING_SCH_MESH_CON5                            DNX_TMC_ING_SCH_MESH_CON5
#define JER2_ARAD_ING_SCH_MESH_CON6                            DNX_TMC_ING_SCH_MESH_CON6
#define JER2_ARAD_ING_SCH_MESH_CON7                            DNX_TMC_ING_SCH_MESH_CON7
#define JER2_ARAD_ING_SCH_MESH_LAST                            DNX_TMC_ING_SCH_MESH_LAST

#define JER2_ARAD_ING_SCH_DONT_TOUCH							  DNX_TMC_ING_SCH_DONT_TOUCH						  	   

typedef DNX_TMC_ING_SCH_MESH_CONTEXTS                          JER2_ARAD_ING_SCH_MESH_CONTEXTS;

typedef DNX_TMC_ING_SCH_SHAPER                                 JER2_ARAD_ING_SCH_SHAPER;
typedef DNX_TMC_ING_SCH_MESH_CONTEXT_INFO                      JER2_ARAD_ING_SCH_MESH_CONTEXT_INFO;
typedef DNX_TMC_ING_SCH_MESH_INFO                              JER2_ARAD_ING_SCH_MESH_INFO;
typedef DNX_TMC_ING_SCH_CLOS_WFQ_ELEMENT                       JER2_ARAD_ING_SCH_CLOS_WFQ_ELEMENT;
typedef DNX_TMC_ING_SCH_CLOS_WFQS                              JER2_ARAD_ING_SCH_CLOS_WFQS;
typedef DNX_TMC_ING_SCH_CLOS_HP_SHAPERS                        JER2_ARAD_ING_SCH_CLOS_HP_SHAPERS;
typedef DNX_TMC_ING_SCH_CLOS_LP_SHAPERS                        JER2_ARAD_ING_SCH_CLOS_LP_SHAPERS;
typedef DNX_TMC_ING_SCH_CLOS_SHAPERS                           JER2_ARAD_ING_SCH_CLOS_SHAPERS;
typedef DNX_TMC_ING_SCH_CLOS_INFO                              JER2_ARAD_ING_SCH_CLOS_INFO;

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
*     jer2_arad_ingress_scheduler_mesh_set
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
*     configuration, the JER2_ARAD_ING_SCH_MESH_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  JER2_ARAD_ING_SCH_MESH_INFO   *mesh_info -
*     mesh_info pointer to configuration structure.
*  DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_INFO   *exact_mesh_info -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_mesh_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_ING_SCH_MESH_INFO   *mesh_info,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_INFO   *exact_mesh_info
  );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_mesh_bandwidth_set
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
*     configuration, the JER2_ARAD_ING_SCH_MESH_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 gport -
*  DNX_SAND_IN  uint32              rate -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  jer2_arad_ingress_scheduler_mesh_bandwidth_set(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_IN  uint32              rate
    );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_mesh_sched_set
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
*     configuration, the JER2_ARAD_ING_SCH_MESH_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 gport -
*  DNX_SAND_IN  int                 weight -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  jer2_arad_ingress_scheduler_mesh_sched_set(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_IN  int                 weight
    );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_mesh_burst_set
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
*     configuration, the JER2_ARAD_ING_SCH_MESH_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 gport -
*  DNX_SAND_IN  int                 burst -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  jer2_arad_ingress_scheduler_mesh_burst_set(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_IN  int                 burst
    );
/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_mesh_get
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
*     configuration, the JER2_ARAD_ING_SCH_MESH_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_INFO   *mesh_info -
*     mesh_info pointer to configuration structure.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_mesh_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_INFO   *mesh_info
  );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_mesh_get
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
*     configuration, the JER2_ARAD_ING_SCH_MESH_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*    DNX_SAND_IN  int                 unit, 
*    DNX_SAND_IN  int                 gport, 
*    DNX_SAND_OUT uint32              *rate
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  jer2_arad_ingress_scheduler_mesh_bandwidth_get(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_OUT uint32              *rate
    );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_mesh_sched_get
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
*     configuration, the JER2_ARAD_ING_SCH_MESH_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*    DNX_SAND_IN  int                 unit, 
*    DNX_SAND_IN  int                 gport, 
*    DNX_SAND_OUT int                 *weight
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  jer2_arad_ingress_scheduler_mesh_sched_get(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_OUT int                 *weight
    );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_mesh_burst_get
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
*     configuration, the JER2_ARAD_ING_SCH_MESH_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*    DNX_SAND_IN  int                 unit, 
*    DNX_SAND_IN  int                 gport, 
*    DNX_SAND_OUT int                 *burst
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  jer2_arad_ingress_scheduler_mesh_burst_get(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_OUT int                 *burst
    );
/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_clos_set
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     This procedure configure the ingress scheduler when
*     working with DNX_SAND CLOS fabric (that is DNX_SAND_FE200/DNX_SAND_FE600). The
*     configuration includes:[local/fabric]-shaper-rates,
*     [local/fabric]-weights. NOTES:1. The 'Verify' function -
*     includes Verification of weight and shaper values range.
*     2. The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Clos
*     configuration, the JER2_ARAD_ING_SCH_CLOS_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info -
*     clos_info pointer to configuration structure.
*  DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *exact_clos_info -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  jer2_arad_ingress_scheduler_clos_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info
  );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_clos_set
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     This procedure configure the ingress scheduler rates when
*     working with DNX_SAND CLOS fabric (that is DNX_SAND_FE200/DNX_SAND_FE600). The
*     configuration includes:[local/fabric]-shaper-rates,
*     [local/fabric]-weights. NOTES:1. The 'Verify' function -
*     includes Verification of weight and shaper values range.
*     2. The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Clos
*     configuration, the JER2_ARAD_ING_SCH_CLOS_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 port -
*  DNX_SAND_IN  uint32              rate -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  jer2_arad_ingress_scheduler_clos_bandwidth_set(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_IN  uint32              rate
    );
/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_clos_sched_set
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     This procedure configure the ingress scheduler rates when
*     working with DNX_SAND CLOS fabric (that is DNX_SAND_FE200/DNX_SAND_FE600). The
*     configuration includes:[local/fabric]-shaper-rates,
*     [local/fabric]-weights. NOTES:1. The 'Verify' function -
*     includes Verification of weight and shaper values range.
*     2. The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Clos
*     configuration, the JER2_ARAD_ING_SCH_CLOS_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 port -
*  DNX_SAND_IN  int                 weight -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  jer2_arad_ingress_scheduler_clos_sched_set(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_IN  int                 weight
    );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_clos_burst_set
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     This procedure configure the ingress scheduler rates when
*     working with DNX_SAND CLOS fabric (that is DNX_SAND_FE200/DNX_SAND_FE600). The
*     configuration includes:[local/fabric]-shaper-rates,
*     [local/fabric]-weights. NOTES:1. The 'Verify' function -
*     includes Verification of weight and shaper values range.
*     2. The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Clos
*     configuration, the JER2_ARAD_ING_SCH_CLOS_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 port -
*  DNX_SAND_IN  int                 burst -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  jer2_arad_ingress_scheduler_clos_burst_set(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_IN  int                 burst
    );
/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_clos_sched_set
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     This procedure configure the ingress scheduler rates when
*     working with DNX_SAND CLOS fabric (that is DNX_SAND_FE200/DNX_SAND_FE600). The
*     configuration includes:[local/fabric]-shaper-rates,
*     [local/fabric]-weights. NOTES:1. The 'Verify' function -
*     includes Verification of weight and shaper values range.
*     2. The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Clos
*     configuration, the JER2_ARAD_ING_SCH_CLOS_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 port -
*  DNX_SAND_OUT int                 *weight -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  jer2_arad_ingress_scheduler_clos_sched_get(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_OUT int                 *weight
    );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_clos_burst_get
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     This procedure configure the ingress scheduler rates when
*     working with DNX_SAND CLOS fabric (that is DNX_SAND_FE200/DNX_SAND_FE600). The
*     configuration includes:[local/fabric]-shaper-rates,
*     [local/fabric]-weights. NOTES:1. The 'Verify' function -
*     includes Verification of weight and shaper values range.
*     2. The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Clos
*     configuration, the JER2_ARAD_ING_SCH_CLOS_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 port -
*  DNX_SAND_OUT int                 *burst -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  jer2_arad_ingress_scheduler_clos_burst_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_OUT int                 *burst
  );
/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_clos_get
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     This procedure configure the ingress scheduler when
*     working with DNX_SAND CLOS fabric (that is DNX_SAND_FE200/DNX_SAND_FE600). The
*     configuration includes:[local/fabric]-shaper-rates,
*     [local/fabric]-weights. NOTES:1. The 'Verify' function -
*     includes Verification of weight and shaper values range.
*     2. The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Clos
*     configuration, the JER2_ARAD_ING_SCH_CLOS_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info -
*     clos_info pointer to configuration structure.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  jer2_arad_ingress_scheduler_clos_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info
  );

/*********************************************************************
* NAME:
*       jer2_arad_ingress_scheduler_clos_bandwidth_get
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     This procedure configure the ingress scheduler when
*     working with DNX_SAND CLOS fabric (that is DNX_SAND_FE200/DNX_SAND_FE600). The
*     configuration includes:[local/fabric]-shaper-rates,
*     [local/fabric]-weights. NOTES:1. The 'Verify' function -
*     includes Verification of weight and shaper values range.
*     2. The 'Get' function - The implementation of the 'get'
*     function of the structure for the Ing Sch Clos
*     configuration, the JER2_ARAD_ING_SCH_CLOS_INFO structure is
*     a 'Fill All' function. Meaning it will get the entire
*     information of the structure from the registers, instead
*     of information for a specific context.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 gport -
*  DNX_SAND_OUT uint32              *rate -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
soc_error_t
  jer2_arad_ingress_scheduler_clos_bandwidth_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_OUT uint32              *rate
  );

void
  jer2_arad_JER2_ARAD_ING_SCH_SHAPER_clear(
    DNX_SAND_OUT JER2_ARAD_ING_SCH_SHAPER *info
  );

void
  jer2_arad_JER2_ARAD_ING_SCH_MESH_CONTEXT_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_CONTEXT_INFO *info
  );

void
  jer2_arad_JER2_ARAD_ING_SCH_MESH_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_INFO *info
  );

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_WFQ_ELEMENT_clear(
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_WFQ_ELEMENT *info
  );

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_WFQS_clear(
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_WFQS *info
  );

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_HP_SHAPERS_clear(
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_HP_SHAPERS *info
  );

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_SHAPERS_clear(
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_SHAPERS *info
  );

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO *info
  );

#if JER2_ARAD_DEBUG_IS_LVL1

const char*
  jer2_arad_JER2_ARAD_ING_SCH_MESH_CONTEXTS_to_string(
    DNX_SAND_IN JER2_ARAD_ING_SCH_MESH_CONTEXTS enum_val
  );

void
  jer2_arad_JER2_ARAD_ING_SCH_SHAPER_print(
    DNX_SAND_IN JER2_ARAD_ING_SCH_SHAPER *info
  );

void
  jer2_arad_JER2_ARAD_ING_SCH_MESH_CONTEXT_INFO_print(
    DNX_SAND_IN JER2_ARAD_ING_SCH_MESH_CONTEXT_INFO *info
  );

void
  jer2_arad_JER2_ARAD_ING_SCH_MESH_INFO_print(
    DNX_SAND_IN JER2_ARAD_ING_SCH_MESH_INFO *info
  );

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_WFQ_ELEMENT_print(
    DNX_SAND_IN JER2_ARAD_ING_SCH_CLOS_WFQ_ELEMENT *info
  );

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_WFQS_print(
    DNX_SAND_IN JER2_ARAD_ING_SCH_CLOS_WFQS *info
  );

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_HP_SHAPERS_print(
    DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_HP_SHAPERS *info
  );

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_SHAPERS_print(
    DNX_SAND_IN JER2_ARAD_ING_SCH_CLOS_SHAPERS *info
  );

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_INFO_print(
    DNX_SAND_IN JER2_ARAD_ING_SCH_CLOS_INFO *info
  );

void
  jer2_arad_JER2_ARAD_ING_SCH_CLOS_INFO_SHAPER_dont_touch(
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO *info
  );

void
  jer2_arad_JER2_ARAD_ING_SCH_MESH_INFO_SHAPERS_dont_touch(
    DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_INFO *info
  );

#endif /* JER2_ARAD_DEBUG_IS_LVL1 */

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_conversion_test_api
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
*   DNX_SAND_IN uint8 is_regression - if true less values are checked.
* RETURNS:
*   DNX_SAND_DIRECT: uint8 pass - whether the test has passed or not.
*
*
*********************************************************************/

uint8
  jer2_arad_ingress_scheduler_conversion_test_api(
    DNX_SAND_IN uint8 is_regression,
    DNX_SAND_IN uint8 silent
  );

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __JER2_ARAD_API_INGRESS_SCHEDULER_INCLUDED__*/
#endif
