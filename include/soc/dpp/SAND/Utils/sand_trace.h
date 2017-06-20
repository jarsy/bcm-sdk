/* $Id: sand_trace.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       soc_sand_trace.h
*
* AUTHOR:         Dune (S.Z.)
*
* FILE DESCRIPTION:
*   Time trace utility.
*   soc_sand_trace_init() -- Initialize the tracing table.
*   soc_sand_trace_end()  -- Currently do nothing.
*   soc_sand_trace_clear() -- Clear the tracing table.
*   soc_sand_trace_print() -- Print trace table.
*                         Prints the time difference between 2 consecutive entries.
*   soc_sand_trace_add_entry() -- Add trace entry to the table.
*                             Add time stamp, except user given data.
* REMARKS:
*   The tracing table is filled continually -- Cyclic.
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
*******************************************************************/


#ifndef __SOC_SAND_TRACE_H_INCLUDED__
/* { */
#define __SOC_SAND_TRACE_H_INCLUDED__

#ifdef  __cplusplus
extern "C" {
#endif

#include <soc/dpp/SAND/Utils/sand_framework.h>

/*****************************************************
*NAME
*  soc_sand_trace_init
*TYPE:
*  PROC
*DATE:
*  15-Jan-03
*FUNCTION:
*  Initialize the trace mechanism.
*INPUT:
*  SOC_SAND_DIRECT:
*    void -
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_RET -
*      SOC_SAND_OK, SOC_SAND_ERR.
*  SOC_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_trace_init(
    void
  );

/*****************************************************
*NAME
*  soc_sand_trace_end
*TYPE:
*  PROC
*DATE:
*  15-Jan-03
*FUNCTION:
*  End the trace mechanism.
*INPUT:
*  SOC_SAND_DIRECT:
*    void -
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_RET -
*      SOC_SAND_OK, SOC_SAND_ERR.
*  SOC_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_trace_end(
    void
  );

/*****************************************************
*NAME
*  soc_sand_trace_clear
*TYPE:
*  PROC
*DATE:
*  15-Jan-03
*FUNCTION:
*  Clear the trace mechanism.
*INPUT:
*  SOC_SAND_DIRECT:
*    void -
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_RET -
*      SOC_SAND_OK, SOC_SAND_ERR.
*  SOC_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_trace_clear(
    void
  );

/*****************************************************
*NAME
*  soc_sand_trace_add_entry
*TYPE:
*  PROC
*DATE:
*  15-Jan-03
*FUNCTION:
*  Add trace entry to the table.
*  Add time stamp, except user given data.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN uint32    identifier -
*         Identifier to be add to trace line.
*         Has no meaning to the tracing process
*    SOC_SAND_IN char*            str
*         String to be add to trace line.
*         Has no meaning to the tracing process
*         XXXXX pointer to const string (not on stack...)  XXXXX
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_RET -
*      SOC_SAND_OK, SOC_SAND_ERR.
*  SOC_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_trace_add_entry(
    SOC_SAND_IN uint32  identifier,
    SOC_SAND_IN char* const   str    /*pointer to const string*/
  );


/*****************************************************
*NAME
*  soc_sand_trace_print
*TYPE:
*  PROC
*DATE:
*  16-Jan-03
*FUNCTION:
*  Print trace table.
*  Prints the time difference between 2 consecutive entries.
*INPUT:
*  SOC_SAND_DIRECT:
*    void -
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_RET -
*      SOC_SAND_OK, SOC_SAND_ERR.
*  SOC_SAND_INDIRECT:
*    NON
*REMARKS:
*  Time difference is in micro-seconds 10^(-6).
*SEE ALSO:
*****************************************************/
SOC_SAND_RET
  soc_sand_trace_print(
    void
  );

#ifdef  __cplusplus
}
#endif

/* } __SOC_SAND_TRACE_H_INCLUDED__*/
#endif
