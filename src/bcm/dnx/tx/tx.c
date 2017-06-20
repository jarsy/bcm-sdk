/*! \file tx.c
 * $Id$
 *
 * TX procedures for DNX.
 *
 * Here add DESCRIPTION.
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#  error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_TX
/*
 * Include files which are specifically for DNX. Final location.
 * {
 */
#include <shared/shrextend/shrextend_debug.h>
#include <shared/shrextend/shrextend_error.h>
/*
 * }
 */
/*
 * Include files currently used for DNX. To be modified and moved to
 * final location.
 * {
 */
#include <shared/bslenum.h>
#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/tx.h>
#include <soc/drv.h>
#include <soc/dnx/dnx_data/dnx_data_cmodel.h>
#ifdef CMODEL_SERVER_MODE
#include <soc/dnx/cmodel/cmodel_reg_access.h>
#endif
/*
 * }
 */

int
bcm_dnx_tx(
  int unit, bcm_pkt_t *pkt, void *cookie)
{
#ifdef CMODEL_SERVER_MODE
  uint32 cmodel_loopback_enable;
  cmodel_ms_id_e ms_id = CMODEL_MS_ID_FIRST_MS;
#endif

  SHR_FUNC_INIT_VARS(unit);

#ifdef CMODEL_SERVER_MODE

  /** Gets the value of the cmodel_loopback_enable from the dnx_data */
  cmodel_loopback_enable = dnx_data_cmodel.tx.loopback_enable_get(unit);

  if (cmodel_loopback_enable == 1)
  {
      ms_id = CMODEL_MS_ID_LOOPBACK;
  }

  /*
   * Use a different flow for sending a packet in C model.
   * The first two entries in the packet data contain a hard coded value for verification and the tx port
   * We therefor pass the data starting at index 2 and use index 1 to pass the tx port
   */

  SHR_IF_ERR_EXIT(cmodel_send_buffer(unit, (uint32)ms_id, (uint32)(pkt->src_port),
                                     pkt->pkt_data->len, pkt->pkt_data->data, 1));

#else

  
  SHR_ERR_EXIT(BCM_E_NONE, "Regular TX handling");

/** CMODEL_SERVER_MODE  */
#endif

exit:
  SHR_FUNC_EXIT;
}
