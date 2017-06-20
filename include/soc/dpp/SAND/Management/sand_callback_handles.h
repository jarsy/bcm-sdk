/* $Id: sand_callback_handles.h,v 1.4 Broadcom SDK $
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
 * $Copyright
 * $
*/
#ifndef SOC_SAND_CALLBACK_HANDLES_H
#define SOC_SAND_CALLBACK_HANDLES_H
#ifdef  __cplusplus
extern "C" {
#endif

#include <soc/dpp/SAND/Utils/sand_delta_list.h>

/* $Id: sand_callback_handles.h,v 1.4 Broadcom SDK $
 * Handles utils, for registering and unregistering all device services
 * (both polling and interrupts).
 * {
 */
typedef struct
{
  /*
   * valid word - a mark telling us this is actually an FE_CALLBACK_HANDLE
   */
  uint32 valid_word;
  /*
   * flag indicating whether this callback weas registerd for
   * an interrupt (0) or for polling (1).
   */
  uint32  int_or_poll;
  /*
   * The device id of the service. Very useful when unregistering a whole device
   */
  int  unit;
  /*
   * The procedure id of the service. useful when services shouldn't register twice
   */
  uint32  proc_id;
  /*
   * the handle returned by the callback engine delta queue.
   */
  uint32 soc_sand_polling_handle;
  /*
   * upto 2 interrupt handles.
   */
  uint32 soc_sand_interrupt_handle;
} SOC_SAND_CALLBACK_HANDLE;
/*
 */
#define SOC_SAND_CALLBACK_HANDLE_VALID_WORD 0xBCBCBCBC


extern SOC_SAND_DELTA_LIST
    *Soc_sand_handles_list  ;


SOC_SAND_RET
  soc_sand_handles_init_handles(
    void
  );
/*
 */
SOC_SAND_RET
  soc_sand_handles_shut_down_handles(
    void
  );
/*
 */
SOC_SAND_RET
  soc_sand_handles_register_handle(
    uint32  int_or_poll,
    int  unit,
    uint32  proc_id,
    uint32 soc_sand_polling_handle,
    uint32 soc_sand_interrupt_handle,
    uint32 *public_handle
  );
/*
 */
SOC_SAND_RET
  soc_sand_handles_unregister_handle(
    uint32 fe_service_handle
  );
/*
 */
SOC_SAND_RET
  soc_sand_handles_unregister_all_device_handles(
    int unit
  );
/*
 */
SOC_SAND_CALLBACK_HANDLE
  *soc_sand_handles_search_next_handle(
    int        unit,
    uint32        proc_id,
    SOC_SAND_CALLBACK_HANDLE  *current
);
/*
 */
SOC_SAND_RET
  soc_sand_handles_is_handle_exist(
    SOC_SAND_IN  int  unit,
    SOC_SAND_IN  uint32  proc_id,
    SOC_SAND_OUT uint32* callback_exist
  );


SOC_SAND_RET
  soc_sand_handles_delta_list_take_mutex(
    void
  );
SOC_SAND_RET
  soc_sand_handles_delta_list_give_mutex(
    void
  );
sal_thread_t
  soc_sand_handles_delta_list_get_owner(
    void
  );

/*
 * }
 * End of device serivecs handle utils
 */
#ifdef  __cplusplus
}
#endif
#endif
