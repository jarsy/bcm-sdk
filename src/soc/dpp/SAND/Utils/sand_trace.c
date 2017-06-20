/* $Id: sand_trace.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/




#include <shared/bsl.h>
#include <soc/dpp/drv.h>



#include <soc/dpp/SAND/Utils/sand_trace.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>

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

} SOC_SAND_TRACE;

/* $Id: sand_trace.c,v 1.4 Broadcom SDK $
 * Number of entries in table.
 * May change in run time.
 */
#define SOC_SAND_TRACE_DEFAULT_TABLE_SIZE 50

/*
 * Hold the table.
 * Allocated in run time.
 */
static SOC_SAND_TRACE
  Soc_sand_trace_table[SOC_SAND_TRACE_DEFAULT_TABLE_SIZE];

/*
 * index of the current place in table.
 * This table is cyclic.
 */
static uint32
  Soc_sand_trace_table_index = 0;

/*
 * Did or did not made turn over
 */
static uint32
  Soc_sand_trace_table_turn_over = FALSE;

/* } */


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
  )
{
  SOC_SAND_RET
    ex;

  ex = SOC_SAND_OK;

  Soc_sand_trace_table_index = 0;
  Soc_sand_trace_table_turn_over = FALSE;
  soc_sand_os_memset(Soc_sand_trace_table,
                 0,
                 sizeof(SOC_SAND_TRACE)*SOC_SAND_TRACE_DEFAULT_TABLE_SIZE);

  return ex;
}

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
  )
{
  return SOC_SAND_OK;
}

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
  )
{
  SOC_SAND_RET
    ex;
  ex = soc_sand_trace_init();
  return ex;
}


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
    SOC_SAND_IN uint32    identifier,
    SOC_SAND_IN char* const     str /*pointer to const string*/
  )
{
  SOC_SAND_INTERRUPT_INIT_DEFS;

  SOC_SAND_INTERRUPTS_STOP;

  Soc_sand_trace_table[Soc_sand_trace_table_index].identifier = identifier;
  Soc_sand_trace_table[Soc_sand_trace_table_index].str        = str;
  Soc_sand_trace_table[Soc_sand_trace_table_index].time_signature =
    soc_sand_os_get_time_micro();
  Soc_sand_trace_table_index ++;
  if( Soc_sand_trace_table_index >= SOC_SAND_TRACE_DEFAULT_TABLE_SIZE )
  {
    Soc_sand_trace_table_index = 0;
    Soc_sand_trace_table_turn_over = TRUE;
  }

  SOC_SAND_INTERRUPTS_START_IF_STOPPED;
  return SOC_SAND_OK;
}



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
  )
{
  int
    index;
  uint32
    micro_time_diff;

  LOG_CLI((BSL_META("soc_sand_trace_print():\n\r")));
  LOG_CLI((BSL_META("Soc_sand_trace_table_index:%u  Soc_sand_trace_table_turn_over:%u\n\r"),
Soc_sand_trace_table_index,
           Soc_sand_trace_table_turn_over));
  for(index=0; index<SOC_SAND_TRACE_DEFAULT_TABLE_SIZE; ++index)
  {
    if(index>0)
    {
      micro_time_diff =
        Soc_sand_trace_table[index].time_signature -
          Soc_sand_trace_table[index-1].time_signature;

    }
    else
    {
      micro_time_diff =
        Soc_sand_trace_table[0].time_signature -
          Soc_sand_trace_table[SOC_SAND_TRACE_DEFAULT_TABLE_SIZE-1].time_signature;
    }
    LOG_CLI((BSL_META("%3d:  micro_time_diff(%5u), identifier(%5u), str(%s)\n\r"),
index,
             micro_time_diff,
             Soc_sand_trace_table[index].identifier,
             Soc_sand_trace_table[index].str
             ));
  }


  return SOC_SAND_OK;
}
