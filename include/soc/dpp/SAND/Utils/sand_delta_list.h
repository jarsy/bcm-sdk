/* $Id: sand_delta_list.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifndef SOC_SAND_DELTA_LIST_H
#define SOC_SAND_DELTA_LIST_H
#ifdef  __cplusplus
extern "C" {
#endif
/* $Id: sand_delta_list.h,v 1.4 Broadcom SDK $
 */
#include <sal/core/sync.h>
#include <sal/core/thread.h>
#include <soc/dpp/SAND/Utils/sand_framework.h>
/*
 * the basic node struct of the list. Notice that the list does not
 * know anything about the data it's holding, and therefore it's a
 * void* it's the list user's responsibility to cast it back to
 * the specific type pointer.
 *
 * the "next" field is how the list is linked (one direction - forward)
 * the "num"  field is how this list can implement advanced features.
 * such as priority based sort and insert.
 * In our case it used to maintain delta (in time) to the next item in
 * list, hence giving it the name - delta list.
 */
typedef struct soc_list_node_str
{
  void                 *data;
  uint32         num;
  struct soc_list_node_str *next;
} SOC_SAND_DELTA_LIST_NODE;
/*
 * information we collect on every delta list
 */
typedef struct
{
  uint32   current_size;
  uint32   no_of_pops;
  uint32   no_of_removes;
  uint32   no_of_inserts;
} SOC_SAND_DELTA_LIST_STATISTICS;
/*
 * A pointer to the firts node (head) in the list, and the
 * delta time to it. All methods of this module recieve
 * SOC_SAND_DELTA_LIST as a param, and "work" on it.
 */
typedef struct
{
  SOC_SAND_DELTA_LIST_NODE       *head;
  uint32                         head_time;
  sal_mutex_t                    mutex_id;
  sal_thread_t                   mutex_owner;
  int32                          mutex_counter;
  SOC_SAND_DELTA_LIST_STATISTICS stats;
} SOC_SAND_DELTA_LIST;

/*
 * Zero when has no owner.
 */
sal_thread_t
  soc_sand_delta_list_get_owner(
    SOC_SAND_DELTA_LIST  *plist
  );
/*
 *  goes over the list from head to tail and prints
 *   it to screen using stdio soc_sand_os_printf
 */
void
  soc_sand_delta_list_print(
    SOC_SAND_DELTA_LIST  *plist
  );
/*
 * TRUE   - list is empty
 * FALSE  - list is not empty
 */
int
  soc_sand_delta_list_is_empty(
    SOC_SAND_IN   SOC_SAND_DELTA_LIST      *plist
  );
/*
 * takes the delta list semaphore.
 * allows for multiple taking of the same task
 */
SOC_SAND_RET
  soc_sand_delta_list_take_mutex(
    SOC_SAND_INOUT   SOC_SAND_DELTA_LIST      *plist
  );
/*
 * gives the delta list semaphore.
 * allows for multiple taking of the same task
 */
SOC_SAND_RET
  soc_sand_delta_list_give_mutex(
    SOC_SAND_INOUT   SOC_SAND_DELTA_LIST      *plist
  );
/*
 * gets the time to wait for the head. This is the
 * whole point of delta list - you only have to wait
 * for the head, and the rest of the items are behind it.
 */
uint32
  soc_sand_delta_list_get_head_time(
    SOC_SAND_IN     SOC_SAND_DELTA_LIST      *plist
  );
/*
 * gets the delta list statistics structure
 */
SOC_SAND_DELTA_LIST_STATISTICS
  *soc_sand_delta_list_get_statisics(
    SOC_SAND_INOUT  SOC_SAND_DELTA_LIST      *plist
  );
/*
 * creates an empty list
 */
SOC_SAND_DELTA_LIST
  *soc_sand_delta_list_create(void);
/*
 * destroy an empty list
 */
SOC_SAND_RET
  soc_sand_delta_list_destroy(
    SOC_SAND_INOUT  SOC_SAND_DELTA_LIST      *plist
  );
/*
 * Inserts the item passed in data into the list
 */
SOC_SAND_RET
  soc_sand_delta_list_insert_d(
    SOC_SAND_INOUT  SOC_SAND_DELTA_LIST      *plist,
    SOC_SAND_IN     uint32    nominal_value,
    SOC_SAND_INOUT  void            *data
  );
/*
 * removes the first item in the list and returns it.
 * if the list is empty it returns NULL
 */
void
  *soc_sand_delta_list_pop_d(
    SOC_SAND_INOUT  SOC_SAND_DELTA_LIST      *plist
  );
/*
 * substract time from the head
 */
SOC_SAND_RET
  soc_sand_delta_list_decrease_time_from_head(
    SOC_SAND_INOUT SOC_SAND_DELTA_LIST       *plist,
    SOC_SAND_IN    uint32    time_to_substract
  );
/*
 * substract time from the second item and then from the first
 */
SOC_SAND_RET
  soc_sand_delta_list_decrease_time_from_second_item(
    SOC_SAND_INOUT SOC_SAND_DELTA_LIST       *plist,
    SOC_SAND_INOUT  uint32   time_to_substract
  );
/*
 * remove the item that its handle was passed during insert_d
 */
SOC_SAND_RET
  soc_sand_delta_list_remove(
    SOC_SAND_INOUT SOC_SAND_DELTA_LIST       *plist,
    SOC_SAND_IN    void             *data
  );
/*
 */


#if SOC_SAND_DEBUG
/* { */


/*
 * Print utility of SOC_SAND_DELTA_LIST_STATISTICS
 */
void
  soc_sand_delta_list_print_DELTA_LIST_STATISTICS(
    SOC_SAND_IN SOC_SAND_DELTA_LIST_STATISTICS* delta_list_statistics
  );

/*****************************************************
*NAME
* soc_sand_delta_list_print_DELTA_LIST
*TYPE:
*  PROC
*DATE:
*  07-Apr-04
*FUNCTION:
*  Debug printing utility
*INPUT:
*  SOC_SAND_DIRECT:
*    None.
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    NONE
*  SOC_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
void
  soc_sand_delta_list_print_DELTA_LIST(
    SOC_SAND_IN SOC_SAND_DELTA_LIST* delta_list
  );

/* } */
#endif


#ifdef  __cplusplus
}
#endif

#endif
