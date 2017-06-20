/*! \file rx.c
 * $Id$
 *
 * RX procedures for DNX.
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
#define BSL_LOG_MODULE BSL_LS_BCMDNX_RX
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
#include <bcm/pkt.h>

/*#include <bcm/rx.h>*/
#include <bcm_int/common/rx.h>

#include <soc/dcmn/dcmn_wb.h>

#ifdef CMODEL_SERVER_MODE
#include <soc/dnx/cmodel/cmodel_reg_access.h>
#include <fcntl.h>
#include <unistd.h>
#endif
/*
 * }
 */

/*
 * Macros and defines for rx thread management
 * {
 */
#ifdef CMODEL_SERVER_MODE

/* 
 * This is the default time between minimum token refreshes. It is also
 * the maximum time between RX thread wake-ups.
 */
#define BCM_RX_TOKEN_CHECK_US_DEFAULT   100000     /* 10 times/sec. */

#define BASE_SLEEP_VAL   500000                                            \

/* Set sleep to base value */
#define INIT_SLEEP    rx_control.sleep_cur = BASE_SLEEP_VAL

/* Lower sleep time if val is < current */
#define SLEEP_MIN_SET(val)                                           \
    (rx_control.sleep_cur = ((val) < rx_control.sleep_cur) ?         \
     (val) : rx_control.sleep_cur)
#endif
/*
 * }
 */

int bcm_cmodel_rx_start(int unit, bcm_rx_cfg_t *cfg);

int
bcm_dnx_rx_start(
  int unit, bcm_rx_cfg_t *cfg)
{

  SHR_FUNC_INIT_VARS(unit);

#ifdef CMODEL_SERVER_MODE

  SHR_IF_ERR_EXIT(bcm_cmodel_rx_start(unit, cfg));

#else

  
  SHR_EXIT();

#endif /*CMODEL_SERVER_MODE*/

exit:
  SHR_FUNC_EXIT;
}

int
bcm_dnx_rx_free(
  int unit, void *pkt_data)
{

  SHR_FUNC_INIT_VARS(unit);

  SHR_IF_ERR_EXIT(_bcm_common_rx_free(unit,pkt_data));

exit:
  SHR_FUNC_EXIT;
}

/*
 * Function:    bcm_dnx_rx_register
 * Purpose:     Register an upper layer driver
 * Parameters:  unit (IN)   - device number.
 *              name (IN)   - constant character string for debug purposes.
 *              cb_f (IN)   - callback function when packet arrives
 *              pri (IN)    - priority of handler in list (0:lowest priority)
 *              cookie (IN) - cookie passed to driver when packet arrives.
 *              flags (IN)  - Register for interrupt or non-interrupt callback
 */
int bcm_dnx_rx_register(int unit, const char *name, bcm_rx_cb_f cb_f, uint8 pri, 
                      void *cookie, uint32 flags)
{
  SHR_FUNC_INIT_VARS(unit);

  SHR_IF_ERR_EXIT(_bcm_common_rx_register(unit, name, cb_f, pri, cookie, flags));

exit:
  SHR_FUNC_EXIT;
}

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
int bcm_dnx_rx_unregister(int unit, bcm_rx_cb_f cb_f, uint8 pri)
{
    SHR_FUNC_INIT_VARS(unit);

    SHR_IF_ERR_EXIT (_bcm_common_rx_unregister(unit, cb_f, pri));
     
exit:
     _DCMN_BCM_WARM_BOOT_API_TEST_MODE_SKIP_WB_SEQUENCE(unit);
    SHR_FUNC_EXIT;
}

int
dnx_rx_packet_parse(
    int unit, 
    bcm_pkt_t *pkt,
    uint8 device_access_allowed)
{
#ifdef BROADCOM_DEBUG
    int i;
#endif /*BROADCOM_DEBUG*/

    SHR_FUNC_INIT_VARS(unit);

#if defined(BROADCOM_DEBUG)
    if (bsl_check(bslLayerBcm, bslSourceRx, bslSeverityVerbose, unit)) {
        LOG_VERBOSE(BSL_LS_BCM_RX,
                    (BSL_META_U(unit,
                                "*************************************RX: (Egress) port %d, cos %d, tot len %d, reason %04x\n"), 
                                pkt->rx_port, pkt->cos, pkt->tot_len, pkt->rx_reason));
    
        {
            char *packet_ptr, *ch_ptr;
            packet_ptr = sal_alloc(3 * pkt->pkt_len, "Packet print");
            if (packet_ptr != NULL) {
                ch_ptr = packet_ptr;
                    /* Dump the packet data block */
                for(i = 0; i < pkt->pkt_len; i++) {
                        if ((i % 4) == 0) {
                            sal_sprintf(ch_ptr," ");
                            ch_ptr++;
                        }
                        if ((i % 32) == 0) {
                            sal_sprintf(ch_ptr,"\n");
                            ch_ptr++;
                        }
                        
                        sal_sprintf(ch_ptr,"%02x", (pkt->_pkt_data.data)[i]); 
                        ch_ptr++;
                        ch_ptr++;
                }
                sal_sprintf(ch_ptr,"\n");
                LOG_VERBOSE(BSL_LS_BCM_RX,
                            (BSL_META_U(unit,
                                        "%s"),packet_ptr));
                sal_free(packet_ptr);
            }
        }
    }
#endif/*BROADCOM_DEBUG*/

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

/* CMODEL functions for RX */
#ifdef CMODEL_SERVER_MODE

void cmodel_convert_rx_data_from_chars(int unit, int data_length_in_bits, unsigned char *data_chars, uint8 *data_binary)
{
  int convert_position = 0;
  int result_index = 0;
  char zero_char = '0';

  while (convert_position + 8 <= data_length_in_bits)
  {
    data_binary[result_index] = (((uint8)(data_chars[convert_position])-(uint8)(zero_char))     << 7 |
                                 ((uint8)(data_chars[convert_position + 1])-(uint8)(zero_char)) << 6 |
                                 ((uint8)(data_chars[convert_position + 2])-(uint8)(zero_char)) << 5 |
                                 ((uint8)(data_chars[convert_position + 3])-(uint8)(zero_char)) << 4 |
                                 ((uint8)(data_chars[convert_position + 4])-(uint8)(zero_char)) << 3 |
                                 ((uint8)(data_chars[convert_position + 5])-(uint8)(zero_char)) << 2 |
                                 ((uint8)(data_chars[convert_position + 6])-(uint8)(zero_char)) << 1 |
                                 ((uint8)(data_chars[convert_position + 7])-(uint8)(zero_char)));
    convert_position += 8;
    result_index++;
  }
}

/* Build the pkt struct */
void cmodel_build_rx_packet(int unit, cmodel_ms_id_e ms_id, uint32 nof_signals, uint32 src_port, int len, unsigned char *buf, bcm_pkt_t *pkt)
{
  /* Currently there are no signals, only the packet's data. Take the data after the signal ID and data size */
  cmodel_convert_rx_data_from_chars(unit, len, &(buf[8]), pkt->pkt_data->data);
  pkt->pkt_data->len = ((len-8) >> 3);    /* Convert from bit to bytes. The first 8 bytes are the signal ID and data length */
  pkt->_pkt_data.len = pkt->pkt_data->len;
  pkt->pkt_len = pkt->pkt_data->len;
  sal_memcpy(pkt->_pkt_data.data, pkt->pkt_data->data, pkt->pkt_data->len);

  pkt->blk_count = 1;                     /* Number of blocks in data array. */
  pkt->unit = unit;                       /* Unit number. */
  /*    pkt->cos; */                          /* The local COS queue to use. */
  /*   pkt->color;   */                      /* Packet color. */
  pkt->src_port = (int16)src_port;                     /* Source port used in header/tag. */
  /* dest_port; */                   /* Destination port used in header/tag. */
  pkt->opcode = BCM_HG_OPCODE_CPU;   /* BCM_HG_OPCODE_xxx. */

  /*   bcm_gport_t dst_gport;   */           /* Destination virtual port */
  /*   bcm_gport_t src_gport;   */           /* Source virtual port */
  /*   bcm_multicast_t multicast_group;  */  /* Destination multicast group. */
}

static void cmodel_rx_pkt_thread(void *param)
{
  int unit = 0;
  unsigned char buf[MAX_PACKET_SIZE_CMODEL];
  uint8 data_buf[MAX_PACKET_SIZE_CMODEL];
  int len;
  int rv = BCM_E_NONE;
  uint32 src_port;
  cmodel_ms_id_e ms_id = 0;
  uint32 nof_signals = 0;
  bcm_pkt_t pkt;
  bcm_pkt_blk_t pkt_data;

  pkt.pkt_data = &pkt_data;
  pkt.pkt_data->data = data_buf;
  pkt._pkt_data.data = data_buf;

  INIT_SLEEP;
  while (rx_control.thread_running)
  {
    /* Lock system rx start/stop mechanism. */
    _BCM_RX_SYSTEM_LOCK;

    /* Wait until a packet is ready and read it */
    rv = cmodel_read_buffer(unit, &ms_id, &nof_signals, &src_port, &len, &(buf[0]));

    /* Packet was read from buffer */
    if (rv == _SHR_E_NONE) {
      /* Build the packets struct */
      cmodel_build_rx_packet(unit, ms_id, nof_signals, src_port, len, &(buf[0]), &pkt);

      /* Process the packet */
      rx_cmodel_process_packet(unit, &pkt);
    }
    else if (rv == _SHR_E_DISABLED)
    {
        /** c model server disconnected. Close Rx thread (this one) in the SDK */
        rx_control.thread_running = FALSE;
    }
    /* Unlock system rx start/stop mechanism. */
    _BCM_RX_SYSTEM_UNLOCK;

    _BCM_RX_CHECK_THREAD_DONE;

    SLEEP_MIN_SET(BASE_SLEEP_VAL);

    sal_sem_take(rx_control.pkt_notify, rx_control.sleep_cur);
    rx_control.pkt_notify_given = FALSE;

    INIT_SLEEP;
  }

  /* Done using self-pipe, close fds */
  close(pipe_fds[0]);
  close(pipe_fds[1]);

  rx_control.thread_exit_complete = TRUE;
  sal_thread_exit(0);
}

/* Init the Rx thread that deals with packets to the CPU */
int bcm_cmodel_rx_start(int unit, bcm_rx_cfg_t *cfg)
{
  int priority = RX_THREAD_PRI_DFLT;
  int flags;

  SHR_FUNC_INIT_VARS(unit);

  /* Timer/Event semaphore thread sleeping on. */
  if (NULL == rx_control.pkt_notify) {
    rx_control.pkt_notify = sal_sem_create("RX pkt ntfy", sal_sem_BINARY, 0);
    if (NULL == rx_control.pkt_notify) {
      SHR_ERR_EXIT(BCM_E_MEMORY, "NULL == rx_control.pkt_notify");
    }
    rx_control.pkt_notify_given = FALSE;
  }

  /* RX start/stop on one of the units protection mutex. */
  if (NULL == rx_control.system_lock) {
    rx_control.system_lock = sal_mutex_create("RX system lock");
    if (NULL == rx_control.system_lock) {
      sal_sem_destroy(rx_control.pkt_notify);
      SHR_ERR_EXIT(BCM_E_MEMORY, "NULL == rx_control.system_lock");
    }
  }

  /* Setup pipe for select exit notification.
     We use a "self-pipe" trick:
        - The write end is maintained by the main thread
        - The read end is selected in the RX thread
     When we need to close the RX thread, we simply write
     to the pipe, and we exit the blocking select call */
  if (pipe(pipe_fds) == -1) {
    SHR_ERR_EXIT(BCM_E_FAIL, "pipe(pipe_fds) == -1");
  }

  /* Make read and write ends of pipe nonblocking:
     Get read end flags */
  flags = fcntl(pipe_fds[0], F_GETFL);
  if (flags == -1) {
    SHR_ERR_EXIT(BCM_E_FAIL, "fcntl(pipe_fds[0], F_GETFL)=-1");
  }

  /* Make read end nonblocking */
  flags |= O_NONBLOCK;
  if (fcntl(pipe_fds[0], F_SETFL, flags) == -1) {
    SHR_ERR_EXIT(BCM_E_FAIL, "fcntl(pipe_fds[0], F_SETFL, flags) == -1");
  }

  /* Get write end flags */
  flags = fcntl(pipe_fds[1], F_GETFL);
  if (flags == -1) {
    SHR_ERR_EXIT(BCM_E_FAIL, "fcntl(pipe_fds[1], F_GETFL)= -1");
  }

  /* Make write end nonblocking */
  flags |= O_NONBLOCK;
  if (fcntl(pipe_fds[1], F_SETFL, flags) == -1) {
    SHR_ERR_EXIT(BCM_E_FAIL, "fcntl(pipe_fds[1], F_SETFL, flags) == -1");
  }

  /* Start rx thread. */
  rx_control.thread_running = TRUE;
  rx_control.cmodel_rx_tid = sal_thread_create("bcmCmodelRX",
                                        SAL_THREAD_STKSZ,
                                        priority,
                                        cmodel_rx_pkt_thread, NULL);

  /* Indicate RX is running */
  rx_ctl[unit]->flags |= BCM_RX_F_STARTED;

exit:
  SHR_FUNC_EXIT;
}

/* End of C model functions */
#endif /*CMODEL_SERVER_MODE*/
