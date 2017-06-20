/* $Id: sand_trace.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/




#include <shared/bsl.h>
#include <soc/dnx/legacy/drv.h>



#include <soc/dnx/legacy/SAND/Utils/sand_trace.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>

/* { Structure of the trace mechanism*/
typedef struct
{
  /*
   * Time in micro seconds.
   */
  uint32  time_signature;

  /*
   * Identifier of location of the trace.
   * The user of
   */
  uint32      identifier;
  const char*       str;

} DNX_SAND_TRACE;

/* $Id: sand_trace.c,v 1.4 Broadcom SDK $
 * Number of entries in table.
 * May change in run time.
 */
#define DNX_SAND_TRACE_DEFAULT_TABLE_SIZE 50

/*
 * Hold the table.
 * Allocated in run time.
 */
static DNX_SAND_TRACE
  Dnx_soc_sand_trace_table[DNX_SAND_TRACE_DEFAULT_TABLE_SIZE];

/*
 * index of the current place in table.
 * This table is cyclic.
 */
static uint32
  Dnx_soc_sand_trace_table_index = 0;

/*
 * Did or did not made turn over
 */
static uint32
  Dnx_soc_sand_trace_table_turn_over = FALSE;

/* } */


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
  )
{
  DNX_SAND_RET
    ex;

  ex = DNX_SAND_OK;

  Dnx_soc_sand_trace_table_index = 0;
  Dnx_soc_sand_trace_table_turn_over = FALSE;
  dnx_sand_os_memset(Dnx_soc_sand_trace_table,
                 0,
                 sizeof(DNX_SAND_TRACE)*DNX_SAND_TRACE_DEFAULT_TABLE_SIZE);

  return ex;
}

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
  )
{
  return DNX_SAND_OK;
}

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
  )
{
  DNX_SAND_RET
    ex;
  ex = dnx_sand_trace_init();
  return ex;
}


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
    DNX_SAND_IN uint32    identifier,
    DNX_SAND_IN char* const     str /*pointer to const string*/
  )
{
  DNX_SAND_INTERRUPT_INIT_DEFS;

  DNX_SAND_INTERRUPTS_STOP;

  Dnx_soc_sand_trace_table[Dnx_soc_sand_trace_table_index].identifier = identifier;
  Dnx_soc_sand_trace_table[Dnx_soc_sand_trace_table_index].str        = str;
  Dnx_soc_sand_trace_table[Dnx_soc_sand_trace_table_index].time_signature =
    dnx_sand_os_get_time_micro();
  Dnx_soc_sand_trace_table_index ++;
  if( Dnx_soc_sand_trace_table_index >= DNX_SAND_TRACE_DEFAULT_TABLE_SIZE )
  {
    Dnx_soc_sand_trace_table_index = 0;
    Dnx_soc_sand_trace_table_turn_over = TRUE;
  }

  DNX_SAND_INTERRUPTS_START_IF_STOPPED;
  return DNX_SAND_OK;
}



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
  )
{
  int
    index;
  uint32
    micro_time_diff;

  LOG_CLI((BSL_META("dnx_sand_trace_print():\n\r")));
  LOG_CLI((BSL_META("Dnx_soc_sand_trace_table_index:%u  Dnx_soc_sand_trace_table_turn_over:%u\n\r"),
Dnx_soc_sand_trace_table_index,
           Dnx_soc_sand_trace_table_turn_over));
  for(index=0; index<DNX_SAND_TRACE_DEFAULT_TABLE_SIZE; ++index)
  {
    if(index>0)
    {
      micro_time_diff =
        Dnx_soc_sand_trace_table[index].time_signature -
          Dnx_soc_sand_trace_table[index-1].time_signature;

    }
    else
    {
      micro_time_diff =
        Dnx_soc_sand_trace_table[0].time_signature -
          Dnx_soc_sand_trace_table[DNX_SAND_TRACE_DEFAULT_TABLE_SIZE-1].time_signature;
    }
    LOG_CLI((BSL_META("%3d:  micro_time_diff(%5u), identifier(%5u), str(%s)\n\r"),
index,
             micro_time_diff,
             Dnx_soc_sand_trace_table[index].identifier,
             Dnx_soc_sand_trace_table[index].str
             ));
  }


  return DNX_SAND_OK;
}
