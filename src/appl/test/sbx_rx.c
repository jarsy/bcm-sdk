/** 
 * 
 *
 * $Id: sbx_rx.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        sbx_rx.c
 * Purpose:     sbxpkt rx support functions
 *
 */

#include <sal/core/libc.h>
#include <sal/core/sync.h>
#include <sal/core/alloc.h>
#include <sal/appl/io.h>
#include <soc/mem.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/pkt.h>
#include <bcm/tx.h>
#include <bcm/rx.h>
#include <bcm_int/control.h>

#include <appl/test/sbx_rx.h>

/*
  Get an array of up to 'len' bytes from packet data memory in 'pkt',
  starting at offset 'idx' into memory pointed to by 'dst'. Returns
  number of bytes copied.
  
  pkt    (IN)  - packet structure
  offset (IN)  - packet data starting offset
  dst    (OUT) - data
  len    (IN)  - requested length

*/
int
sbxpkt_data_memget(bcm_pkt_t *pkt, int offset, uint8 *dst, int len)
{
    int left_in_blk, segment, copied;
    uint8 *src;
    int rv;

    copied = 0;
    while (len > 0) {
        rv = bcm_pkt_byte_index(pkt, offset, &left_in_blk, NULL, &src);
        if (!(rv)) {
            segment = (len > left_in_blk) ? left_in_blk : len;
            sal_memcpy(dst, src, segment);
            dst    += segment;
            offset += segment;
            len    -= segment;
            copied += segment;
        } else {
            break;
        }
    }

    return copied;
}

/* Initialize packet data buffers */
void
sbxpkt_data_clear(bcm_pkt_t *pkt)
{
    int i;

    if (pkt) {
        for (i=0; i<pkt->blk_count; i++) {
            sal_memset(pkt->pkt_data[i].data, 0, pkt->pkt_data[i].len);
        }
    }
}


/* NOTE: this function isn't useful on real devices unless the buffer
   is DMAable */

void
sbxpkt_one_buf_setup(bcm_pkt_t *pkt, unsigned char *buf, int len)
{
    unsigned char *buf2 = sal_alloc(len, "sbxpkt_rx"); 
    /* ase_calloc(1, len); */
    sal_memcpy(buf2, buf, len);
    /* bcm_pkt_one_buf_setup(pkt, buf2, len); */
    BCM_PKT_ONE_BUF_SETUP(pkt, buf2, len);
}

void
sbxpkt_one_buf_free(bcm_pkt_t *pkt)
{
    sal_free(pkt->_pkt_data.data);
}

typedef struct {
    int         unit;           /* Unit this is running on */
    int         rx_running;     /* TRUE if RX was running */
    bcm_pkt_t   *list;          /* receive packet list */
    int         in;             /* Packets coming in from RX callback */
    int         out;            /* Packets going out */
    int         length;         /* Queue length */
    sal_sem_t   sem;            /* Reception semaphore */
} sbxpkt_rx_sync_t;

sbxpkt_rx_sync_t *sbxpkt_rx[BCM_UNITS_MAX];
int sbxpkt_rx_sync_pri = BCM_RX_PRIO_MAX;

int
sbxpkt_rx_sync_set_priority(int pri)
{
    int old_priority = sbxpkt_rx_sync_pri;

    sbxpkt_rx_sync_pri = pri;

    return old_priority;
}

/*

  Put a received packet into a a queue.

  Lock free algorithm: Read 'in' and 'out' once. Only a single
  producer modifies 'in' via 'put', and only a single consumer
  modifies 'out' via 'get'.

  If the queue is not full, only a 'put' can make a queue go from
  !full to full, so any 'get's that occur after the read of 'out' will
  still leave the queue state as !full.

  Copying the data to the queue is done after the queue state is
  determined to be !full and before the queue state is updated. The
  data put into the queue is not visible to the the 'get' until 'in'
  is updated.
  
  If the queue is full, then this 'put' will not modify 'in' (and
  throw the packet away), but any subsequent 'get's will make the
  queue !full, and any successive 'put's will then succeed.

  The order of operation must always be:

    READ the queue state
    COPY data to the queue
    UPDATE the queue state
  

 */
bcm_rx_t
sbxpkt_rx_sync_cb(int unit, bcm_pkt_t *pkt, void *cookie)
{
    sbxpkt_rx_sync_t *data = NULL;
    bcm_rx_t       rv   = BCM_RX_HANDLED;
    int next;
    int full;
    int in;
    int out;

    COMPILER_REFERENCE(cookie);

    data = sbxpkt_rx[unit];

    if (data != NULL) {
        /* Read queue state */
        in   = data->in;
        out  = data->out;

        /* Advance in */
        next = ((in + 1) % data->length);
        full = (out == next);

        if (!full) {
            /* Copy data to queue */
            data->list[in] = *pkt;  /* Copy pkt data to output array */
            if (pkt->pkt_data == &pkt->_pkt_data) {
                /* Update pkt_data if necessary */
                data->list[in].pkt_data = &data->list[in]._pkt_data;
            }

            /* Update */
            data->in = next;

            /* Signal receiver and tell RX we own the packet now */
            sal_sem_give(data->sem);
            rv = BCM_RX_HANDLED_OWNED;
            
        }
    }

    return rv;
}

/**
  Waits for 'timeout' microseconds to get a packet from the queue in 'pkt'.
  
  @param unit    (IN) Receive Unit
  @param pkt     (OUT) Received packet
  @param timeout (IN) Time to wait for packet

  @return a BCM error code.

  Lock free algorithm: Read 'in' and 'out' once. Only a single
  producer modifies 'in' via 'put', and only a single consumer
  modifies 'out' via 'get'.

  If the queue is not empty, only a 'get' can make a queue go from
  !empty to empty, so any 'put's that occur after the read of 'in'
  will still leave the queue state as !empty.

  Copying the data from the queue is done after the queue state is
  determined to be !empty and before the queue state is updated. The
  data copied out from the queue will not be overwritten by a
  subsequent 'put' until 'out' is updated, because it would only be
  overwritten by 'in' equalling the current value of 'out', which is
  precisely the 'queue full' condition until 'out' is advanced to
  'next'.
  
  If the queue is empty, then this 'get' will not modify 'out' (and
  fail), but any subsequent 'put's will make the queue !empty, and any
  successive 'get's will then succeed.

  The order of operation must always be:

    READ the queue state
    COPY data from the queue
    UPDATE the queue state

  
*/
int
sbxpkt_rx_sync(int unit, bcm_pkt_t *pkt, int timeout)
{
    int rv = BCM_E_INTERNAL;
    sbxpkt_rx_sync_t *data = NULL;
    int next, empty, in, out;

    if (!BCM_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    
    if ((data=sbxpkt_rx[unit]) == NULL) {
        return BCM_E_PARAM;
    }

    if (timeout < 0) {
        timeout = sal_sem_FOREVER;
    }

    do {
        /* Read queue state */
        in = data->in;
        out = data->out;

        empty = (in == out);
        if (!empty) {
            next = (out + 1) % data->length;
            /* copy packet from queue */
            *pkt = data->list[out];
            if (data->list[out].pkt_data == &data->list[out]._pkt_data) {
                /* Update pkt_data if necessary */
                pkt->pkt_data = &pkt->_pkt_data;
            }

            /* update */
            data->out = next;

            rv = BCM_E_NONE;
            break;
        } else {
            /* Wait for a packet to arrive */
            if (sal_sem_take(data->sem, timeout) < 0) {
                rv = BCM_E_TIMEOUT;
                break;
            }
        }
            
    } while (empty);

    return rv;
}

int
sbxpkt_rxs_sync(int unit, bcm_pkt_t **pkt, int timeout, int *num_rx)
{
    int rv = BCM_E_INTERNAL;
    sbxpkt_rx_sync_t *data = NULL;
    int next, empty, in, out;

    *num_rx = 0;

    if (!BCM_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    
    if ((data=sbxpkt_rx[unit]) == NULL) {
        return BCM_E_PARAM;
    }

    if (timeout < 0) {
        timeout = sal_sem_FOREVER;
    }

    do {
        /* Read queue state */
        in = data->in;
        out = data->out;

        empty = (in == out);
        if (!empty) {

	    do {
	        next = (out + 1) % data->length;
	        /* copy packet from queue */
	        *pkt[*num_rx] = data->list[out];
	        if (data->list[out].pkt_data == &data->list[out]._pkt_data) {
		    /* Update pkt_data if necessary */
		    pkt[*num_rx]->pkt_data = &pkt[*num_rx]->_pkt_data;
		}
		*num_rx = *num_rx + 1;

	        out = next;
	    } while (out != in);


	    /* update */
	    data->out = next;

            rv = BCM_E_NONE;
	    break;
        } else {
            /* Wait for a packet to arrive */
            if (sal_sem_take(data->sem, timeout) < 0) {
                rv = BCM_E_TIMEOUT;
                break;
            }
        }
          

    } while (empty);

    return rv;
}


/* Default queue size */
#define QSIZE 128

int _sbxpkt_rx_sync_start_qsize = QSIZE;

/* Set the default queue size (if > 0) and return current size */
int
sbxpkt_rx_sync_queue_size(int size)
{
    int old_size = _sbxpkt_rx_sync_start_qsize;

    if (size > 0) {
        _sbxpkt_rx_sync_start_qsize = size;
    }

    return old_size;
}


#if defined(BROADCOM_DEBUG)
#define FILL_DATA 0xc9
#else
#define FILL_DATA 0
#endif

/**
  \brief Starts packet reception on unit for packets.

  \param unit (IN) receive Unit

  \return Returns a BCM error code.
*/
int
sbxpkt_rx_sync_start(int unit)
{
    int rv = BCM_E_INTERNAL;
    int lsize;
    sbxpkt_rx_sync_t *data;

    if (!BCM_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (sbxpkt_rx[unit] != NULL) {
        return BCM_E_BUSY;
    }
    
    data = sal_alloc(sizeof(sbxpkt_rx_sync_t), "sbxpkt_rx_sync_start");
    if (data == NULL) {
        return BCM_E_MEMORY;
    }

    data->unit = unit;
    data->in = 0;
    data->out = 0;
    data->rx_running = FALSE;
    data->length = _sbxpkt_rx_sync_start_qsize;
    sbxpkt_rx[unit] = data;
 
    lsize = _sbxpkt_rx_sync_start_qsize * sizeof(bcm_pkt_t);
    data->list = sal_alloc(lsize, "sbxpkt_rx_sync_start");
    if (data->list == NULL) {
        sbxpkt_rx_sync_stop(unit);
        return BCM_E_MEMORY;
    }
    sal_memset(data->list, FILL_DATA, lsize);

    if (bcm_rx_active(unit)) {
        data->rx_running = TRUE;
    } else {
        rv = bcm_rx_cfg_init(unit);
        if (BCM_FAILURE(rv)) {
            sbxpkt_rx_sync_stop(unit);
            return rv;
        }
        rv = bcm_rx_start(unit, NULL);
        if (BCM_FAILURE(rv)) {
            sbxpkt_rx_sync_stop(unit);
            return rv;
        }
    }
        
    data->sem = sal_sem_create("sbxpkt_rx_sync", sal_sem_BINARY, 0);

    if (!data->sem) {
        sbxpkt_rx_sync_stop(unit);
        rv = BCM_E_MEMORY;
        return rv;
    }

    /* register rx callback */
    rv = bcm_rx_register(unit, "sbxpkt_rx_sync",
                         sbxpkt_rx_sync_cb, sbxpkt_rx_sync_pri,
                         data, BCM_RCO_F_ALL_COS);
    
    if (BCM_FAILURE(rv)) {
        sbxpkt_rx_sync_stop(unit);
    }
        
    return rv;
}

/**
  \brief Stops packet reception on unit.

  \param unit (IN) receive Unit

  \return Returns a BCM error code.
*/
int
sbxpkt_rx_sync_stop(int unit)
{
    int rv = BCM_E_NONE;
    sbxpkt_rx_sync_t *data;
    int in;
    int out;
    int next;
    int empty;
    bcm_pkt_t *pkt;

    if (!BCM_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    
    if ((data=sbxpkt_rx[unit]) != NULL) {
        /* unregister */
        bcm_rx_unregister(data->unit, sbxpkt_rx_sync_cb, sbxpkt_rx_sync_pri);
        if (!data->rx_running) {
            rv = bcm_rx_stop(data->unit, NULL);
        }

        for (;;) {
            in = data->in;
            out = data->out;
            empty = (in == out);
            if (empty) {
                break;
            }
            next = (out + 1) % data->length;
            pkt = &data->list[out];
            rv = bcm_rx_free(unit, pkt->alloc_ptr);
            data->out = next;
        }

        if (data->sem) {
            sal_sem_destroy(data->sem);
        }

        sbxpkt_rx[unit] = NULL;

        if (data->list) {
            sal_free(data->list);
        }

        sal_free(data);
    } else {
        rv = BCM_E_PARAM;
    }

    return rv;
}


