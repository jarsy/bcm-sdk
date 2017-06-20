/*! \file bcm_int/dnx/rx/rx.h
 * $Id$
 * 
 * Internal DNX RX APIs 
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef _RX_API_INCLUDED__
/* { */
#define _RX_API_INCLUDED__

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

#include <include/bcm/rx.h>

/*!
 * \brief Initialize the rx thread and resources.
 *
 * \par DIRECT INPUT:
 *   \param [in] unit unit gport belongs to
 *   \param [in] cfg pointer to RX configuration structure
 * \par DIRECT OUTPUT:
 *   \retval Error indication according to shr_error_e enum
 */
int bcm_dnx_rx_start(
  int unit,
  bcm_rx_cfg_t * cfg);

/*!
 * \brief Free packet in RX buffer.
 *
 * \par DIRECT INPUT:
 *   \param [in] unit unit gport belongs to
 *   \param [in] pkt_data pointer to RX packet structure
 * \par DIRECT OUTPUT:
 *   \retval Error indication according to shr_error_e enum
 */
int
bcm_dnx_rx_free(
  int unit, void *pkt_data);

int bcm_dnx_rx_register(
   int unit, const char *name, 
   bcm_rx_cb_f cb_f, uint8 pri, 
   void *cookie, uint32 flags);

/**
 * \brief - Unregister a callback from the Rx handlers.
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - device number.
 *   \param [in] cb_f - the callback to unregister
 *   \param [in] pri - priority of the callback
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int bcm_dnx_rx_unregister(int unit, bcm_rx_cb_f cb_f, uint8 pri);

int dnx_rx_packet_parse(
    int unit, 
    bcm_pkt_t *pkt,
    uint8 device_access_allowed);

/* } */
#endif/*_RX_API_INCLUDED__*/
