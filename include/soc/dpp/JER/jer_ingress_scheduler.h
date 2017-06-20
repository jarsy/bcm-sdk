/* $Id: jer_fabric.h,v 1.30 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __JER_INGRESS_SCHEDULER_INCLUDED__
/* { */

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>


#include <soc/dpp/SAND/Utils/sand_framework.h>
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
*     jer_ingress_scheduler_init
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure init ingress scheduler
*     links.
* INPUT:
*  SOC_SAND_IN  int                 unit -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer_ingress_scheduler_init(
    SOC_SAND_IN  int                 unit
  );
/*********************************************************************  
* NAME:
*     jer_ingress_scheduler_clos_slow_start_get
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure init returns slow start config for clos
*     links.
* INPUT:
*  SOC_SAND_IN  int                 unit -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
jer_ingress_scheduler_clos_slow_start_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_OUT SOC_TMC_ING_SCH_CLOS_INFO   *clos_info
    );


/*********************************************************************  
* NAME:
*     jer_ingress_scheduler_clos_slow_start_set
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure configures slow start config for clos
*     links.
* INPUT:
*  SOC_SAND_IN  int                 unit -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
jer_ingress_scheduler_clos_slow_start_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN SOC_TMC_ING_SCH_CLOS_INFO   *clos_info
    );


/*********************************************************************  
* NAME:
*     jer_ingress_scheduler_mesh_slow_start_get
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure returns slow start config for mesh
*     links.
* INPUT:
*  SOC_SAND_IN  int                 unit -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
jer_ingress_scheduler_mesh_slow_start_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_OUT SOC_TMC_ING_SCH_SHAPER   *shaper_info
    );


/*********************************************************************  
* NAME:
*     jer_ingress_scheduler_mesh_slow_start_set
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure configures slow start config for mesh
*     links.
* INPUT:
*  SOC_SAND_IN  int                 unit -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
jer_ingress_scheduler_mesh_slow_start_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN SOC_TMC_ING_SCH_SHAPER   *shaper_info
    );


/*********************************************************************  
* NAME:
*     jer_ingress_scheduler_clos_bandwidth_get
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure returns rate for clos
*     links.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 gport -  
*  SOC_SAND_IN  uint32              rate -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer_ingress_scheduler_clos_bandwidth_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_OUT uint32              *rate
  );

/*********************************************************************  
* NAME:
*     jer_ingress_scheduler_clos_bandwidth_set
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure configures rate for clos
*     links.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 gport -  
*  SOC_SAND_IN  uint32              rate -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer_ingress_scheduler_clos_bandwidth_set(
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_IN  uint32              rate
    );

/*********************************************************************  
* NAME:
*     jer_ingress_scheduler_mesh_bandwidth_get
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure returns rate for mesh
*     links.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 gport -  
*  SOC_SAND_IN  uint32              rate -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer_ingress_scheduler_mesh_bandwidth_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_OUT uint32              *rate
  );

/*********************************************************************  
* NAME:
*     jer_ingress_scheduler_mesh_bandwidth_set
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure configures rate for mesh
*     links.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 gport -  
*  SOC_SAND_IN  uint32              rate -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer_ingress_scheduler_mesh_bandwidth_set(
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_IN  uint32              rate
    );


/*********************************************************************  
* NAME:
*     jer_ingress_scheduler_clos_burst_get
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure returns burst for clos
*     links.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 gport -  
*  SOC_SAND_IN  uint32              burst -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer_ingress_scheduler_clos_burst_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_OUT int                 *burst
  );

/*********************************************************************  
* NAME:
*     jer_ingress_scheduler_clos_burst_set
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure configures burst for clos
*     links.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 gport -  
*  SOC_SAND_IN  uint32              burst -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer_ingress_scheduler_clos_burst_set(
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_IN  int                 burst
    );

/*********************************************************************  
* NAME:
*     jer_ingress_scheduler_mesh_burst_get
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure returns burst for mesh
*     links.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 gport -  
*  SOC_SAND_IN  uint32              burst -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer_ingress_scheduler_mesh_burst_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_OUT int                 *burst
  );

/*********************************************************************  
* NAME:
*     jer_ingress_scheduler_mesh_burst_set
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure configures burst for mesh
*     links.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 gport -  
*  SOC_SAND_IN  uint32              burst -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer_ingress_scheduler_mesh_burst_set(
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_IN  int                 burst
    );

/*********************************************************************  
* NAME:
*     jer_ingress_scheduler_clos_sched_get
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure returns weight for clos
*     links.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 gport -  
*  SOC_SAND_IN  uint32              weight -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer_ingress_scheduler_clos_sched_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_OUT int                 *weight
  );

/*********************************************************************  
* NAME:
*     jer_ingress_scheduler_clos_sched_set
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure configures weight for clos
*     links.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 gport -  
*  SOC_SAND_IN  uint32              weight -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer_ingress_scheduler_clos_sched_set(
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_IN  int                 weight
    );

/*********************************************************************  
* NAME:
*     jer_ingress_scheduler_mesh_sched_get
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure returns weight for mesh
*     links.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 gport -  
*  SOC_SAND_IN  uint32              weight -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer_ingress_scheduler_mesh_sched_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_OUT int                 *weight
  );

/*********************************************************************  
* NAME:
*     jer_ingress_scheduler_mesh_sched_set
* TYPE:
*   PROC
* DATE:
*   Jun 15 2013
* FUNCTION:
*     This procedure configures weight for mesh
*     links.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  int                 gport -  
*  SOC_SAND_IN  uint32              weight -
* REMARKS:
*     None.
*********************************************************************/
soc_error_t
  jer_ingress_scheduler_mesh_sched_set(
    SOC_SAND_IN  int                 unit, 
    SOC_SAND_IN  int                 gport, 
    SOC_SAND_IN  int                 weight
    );



#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __JER_INGRESS_SCHEDULER_INCLUDED__*/
#endif
