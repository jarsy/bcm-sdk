/* $Id: jer2_jer_fabric.h,v 1.30 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __JER2_JER_INGRESS_SCHEDULER_INCLUDED__
/* { */

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>


#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/error.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */


/* } */

/*************
 * MACROS    *
 *************/
/* { */

/*************
 * TYPE DEFS *
 *************/
/* { */

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
*     jer2_jer_ingress_scheduler_init
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure init ingress scheduler
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer2_jer_ingress_scheduler_init(
    DNX_SAND_IN  int                 unit
  );
/*********************************************************************  
* NAME:
*     jer2_jer_ingress_scheduler_clos_slow_start_get
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure init returns slow start config for clos
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
jer2_jer_ingress_scheduler_clos_slow_start_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_OUT DNX_TMC_ING_SCH_CLOS_INFO   *clos_info
    );


/*********************************************************************  
* NAME:
*     jer2_jer_ingress_scheduler_clos_slow_start_set
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure configures slow start config for clos
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
jer2_jer_ingress_scheduler_clos_slow_start_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN DNX_TMC_ING_SCH_CLOS_INFO   *clos_info
    );


/*********************************************************************  
* NAME:
*     jer2_jer_ingress_scheduler_mesh_slow_start_get
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure returns slow start config for mesh
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
jer2_jer_ingress_scheduler_mesh_slow_start_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_OUT DNX_TMC_ING_SCH_SHAPER   *shaper_info
    );


/*********************************************************************  
* NAME:
*     jer2_jer_ingress_scheduler_mesh_slow_start_set
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure configures slow start config for mesh
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
jer2_jer_ingress_scheduler_mesh_slow_start_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN DNX_TMC_ING_SCH_SHAPER   *shaper_info
    );


/*********************************************************************  
* NAME:
*     jer2_jer_ingress_scheduler_clos_bandwidth_get
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure returns rate for clos
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 gport -  
*  DNX_SAND_IN  uint32              rate -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer2_jer_ingress_scheduler_clos_bandwidth_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_OUT uint32              *rate
  );

/*********************************************************************  
* NAME:
*     jer2_jer_ingress_scheduler_clos_bandwidth_set
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure configures rate for clos
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 gport -  
*  DNX_SAND_IN  uint32              rate -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer2_jer_ingress_scheduler_clos_bandwidth_set(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_IN  uint32              rate
    );

/*********************************************************************  
* NAME:
*     jer2_jer_ingress_scheduler_mesh_bandwidth_get
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure returns rate for mesh
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 gport -  
*  DNX_SAND_IN  uint32              rate -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer2_jer_ingress_scheduler_mesh_bandwidth_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_OUT uint32              *rate
  );

/*********************************************************************  
* NAME:
*     jer2_jer_ingress_scheduler_mesh_bandwidth_set
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure configures rate for mesh
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 gport -  
*  DNX_SAND_IN  uint32              rate -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer2_jer_ingress_scheduler_mesh_bandwidth_set(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_IN  uint32              rate
    );


/*********************************************************************  
* NAME:
*     jer2_jer_ingress_scheduler_clos_burst_get
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure returns burst for clos
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 gport -  
*  DNX_SAND_IN  uint32              burst -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer2_jer_ingress_scheduler_clos_burst_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_OUT int                 *burst
  );

/*********************************************************************  
* NAME:
*     jer2_jer_ingress_scheduler_clos_burst_set
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure configures burst for clos
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 gport -  
*  DNX_SAND_IN  uint32              burst -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer2_jer_ingress_scheduler_clos_burst_set(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_IN  int                 burst
    );

/*********************************************************************  
* NAME:
*     jer2_jer_ingress_scheduler_mesh_burst_get
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure returns burst for mesh
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 gport -  
*  DNX_SAND_IN  uint32              burst -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer2_jer_ingress_scheduler_mesh_burst_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_OUT int                 *burst
  );

/*********************************************************************  
* NAME:
*     jer2_jer_ingress_scheduler_mesh_burst_set
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure configures burst for mesh
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 gport -  
*  DNX_SAND_IN  uint32              burst -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer2_jer_ingress_scheduler_mesh_burst_set(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_IN  int                 burst
    );

/*********************************************************************  
* NAME:
*     jer2_jer_ingress_scheduler_clos_sched_get
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure returns weight for clos
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 gport -  
*  DNX_SAND_IN  uint32              weight -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer2_jer_ingress_scheduler_clos_sched_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_OUT int                 *weight
  );

/*********************************************************************  
* NAME:
*     jer2_jer_ingress_scheduler_clos_sched_set
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure configures weight for clos
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 gport -  
*  DNX_SAND_IN  uint32              weight -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer2_jer_ingress_scheduler_clos_sched_set(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_IN  int                 weight
    );

/*********************************************************************  
* NAME:
*     jer2_jer_ingress_scheduler_mesh_sched_get
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure returns weight for mesh
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 gport -  
*  DNX_SAND_IN  uint32              weight -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer2_jer_ingress_scheduler_mesh_sched_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_OUT int                 *weight
  );

/*********************************************************************  
* NAME:
*     jer2_jer_ingress_scheduler_mesh_sched_set
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure configures weight for mesh
*     links.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  int                 gport -  
*  DNX_SAND_IN  uint32              weight -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer2_jer_ingress_scheduler_mesh_sched_set(
    DNX_SAND_IN  int                 unit, 
    DNX_SAND_IN  int                 gport, 
    DNX_SAND_IN  int                 weight
    );



#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __JER2_JER_INGRESS_SCHEDULER_INCLUDED__*/
#endif
