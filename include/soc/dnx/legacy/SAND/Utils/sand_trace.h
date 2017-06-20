/* $Id: sand_trace.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       dnx_sand_trace.h
*
* AUTHOR:         Dune (S.Z.)
*
* FILE DESCRIPTION:
*   Time trace utility.
*   dnx_sand_trace_init() -- Initialize the tracing table.
*   dnx_sand_trace_end()  -- Currently do nothing.
*   dnx_sand_trace_clear() -- Clear the tracing table.
*   dnx_sand_trace_print() -- Print trace table.
*                         Prints the time difference between 2 consecutive entries.
*   dnx_sand_trace_add_entry() -- Add trace entry to the table.
*                             Add time stamp, except user given data.
* REMARKS:
*   The tracing table is filled continually -- Cyclic.
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
*******************************************************************/


#ifndef __DNX_SAND_TRACE_H_INCLUDED__
/* { */
#define __DNX_SAND_TRACE_H_INCLUDED__

#ifdef  __cplusplus
extern "C" {
#endif

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>

/*****************************************************
*NAME
*  dnx_sand_trace_init
*TYPE:
*  PROC
*DATE:
*  15-Jan-03
*FUNCTION:
*  Initialize the trace mechanism.
*INPUT:
*  DNX_SAND_DIRECT:
*    void -
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET -
*      DNX_SAND_OK, DNX_SAND_ERR.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_trace_init(
    void
  );

/*****************************************************
*NAME
*  dnx_sand_trace_end
*TYPE:
*  PROC
*DATE:
*  15-Jan-03
*FUNCTION:
*  End the trace mechanism.
*INPUT:
*  DNX_SAND_DIRECT:
*    void -
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET -
*      DNX_SAND_OK, DNX_SAND_ERR.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_trace_end(
    void
  );

/*****************************************************
*NAME
*  dnx_sand_trace_clear
*TYPE:
*  PROC
*DATE:
*  15-Jan-03
*FUNCTION:
*  Clear the trace mechanism.
*INPUT:
*  DNX_SAND_DIRECT:
*    void -
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET -
*      DNX_SAND_OK, DNX_SAND_ERR.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_trace_clear(
    void
  );

/*****************************************************
*NAME
*  dnx_sand_trace_add_entry
*TYPE:
*  PROC
*DATE:
*  15-Jan-03
*FUNCTION:
*  Add trace entry to the table.
*  Add time stamp, except user given data.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN uint32    identifier -
*         Identifier to be add to trace line.
*         Has no meaning to the tracing process
*    DNX_SAND_IN char*            str
*         String to be add to trace line.
*         Has no meaning to the tracing process
*         XXXXX pointer to const string (not on stack...)  XXXXX
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET -
*      DNX_SAND_OK, DNX_SAND_ERR.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_trace_add_entry(
    DNX_SAND_IN uint32  identifier,
    DNX_SAND_IN char* const   str    /*pointer to const string*/
  );


/*****************************************************
*NAME
*  dnx_sand_trace_print
*TYPE:
*  PROC
*DATE:
*  16-Jan-03
*FUNCTION:
*  Print trace table.
*  Prints the time difference between 2 consecutive entries.
*INPUT:
*  DNX_SAND_DIRECT:
*    void -
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_RET -
*      DNX_SAND_OK, DNX_SAND_ERR.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  Time difference is in micro-seconds 10^(-6).
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_trace_print(
    void
  );

#ifdef  __cplusplus
}
#endif

/* } __DNX_SAND_TRACE_H_INCLUDED__*/
#endif
