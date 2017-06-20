/*
 * $Id: tx.c,v 1.20 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        tx.c
 * Purpose:
 * Requires:
 */

#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbx_txrx.h>
#ifdef BCM_QE2000_SUPPORT
#include <soc/sbx/qe2000_util.h>
#endif

#include <bcm/error.h>
#include <bcm/tx.h>

#include <bcm_int/control.h>
#include <bcm_int/sbx/lock.h>
#include <bcm_int/common/tx.h>

#define TX_UNIT_VALID_CHECK(unit) \
    if (((unit) < 0) || ((unit) >= BCM_MAX_NUM_UNITS)) { return BCM_E_UNIT; }


#define IF_GOTO(t, e, l) do { if((t)) { rv=e; goto l;} } while(0)
#define IF_GOTO_E(t, e) IF_GOTO(t, e, error)

#define SBX_TX_INIT_DONE  (pkt_tx_sem != NULL)


#define DEFAULT_TX_THREAD_PRI    50
#define MAX_RETRY                50

/* timeout per request, needed to catch memory leaks on failed soc_sbx_txrx
 */
#define MAX_TIMEOUT              (30 * 1000000)


typedef struct async_context_s {
    int           count;
    int           unit;
    bcm_pkt_cb_f  cb;
    void         *cookie;
    bcm_pkt_t    *first_pkt;
    sal_time_t    start_time;

    struct async_context_s *next;
} async_context_t;

typedef struct tx_req_list_s {
    async_context_t *head;
    async_context_t *tail;
    int              count;
} tx_req_list_t;


typedef struct tx_state_s {
    sal_mutex_t      mtx;

    tx_req_list_t    active_list;
    tx_req_list_t    done_list;
} tx_state_t;


/*
 *************************************************************
 *                                                           *
 * SECTION: STATIC                                           *
 *                                                           *
 * Local (static) variables and functions should go into the *
 * section below.                                            *
 *                                                           *
 *************************************************************
 */
static sal_sem_t      pkt_tx_sem = NULL;
static sal_thread_t   pkt_tx_thrd_id = NULL;
static tx_state_t     tx_state;

STATIC void   _sbx_tx_thread(void *param);
STATIC void   _sbx_tx_done (int unit, soc_sbx_txrx_active_t *done);

STATIC int    _sbx_tx_generic(int unit, bcm_pkt_t **pkt, int count, 
                              bcm_pkt_cb_f all_done_cb, void *cookie);

STATIC int    _sbx_tx_req_add(tx_req_list_t   *list, 
                              async_context_t *entry,
                              sal_mutex_t     *mtx);

STATIC int    _sbx_tx_req_is_member(tx_req_list_t   *list,
                                    async_context_t *entry,
                                    sal_mutex_t     *mtx);

STATIC int    _sbx_tx_req_remove(tx_req_list_t   *list, 
                                 async_context_t **entry,
                                 sal_mutex_t     *mtx);

STATIC int    _sbx_tx_req_age(tx_req_list_t *list, sal_mutex_t *mtx);

STATIC void   _sbx_tx_cleanup(void);

extern int sbx_qe2000_tx_pkt_setup(int unit, bcm_pkt_t *tx_pkt);
extern int sbx_sirius_tx_pkt_setup(int unit, bcm_pkt_t *tx_pkt);

/*
 *************************************************************
 *                                                           *
 * SECTION: extern                                           *
 *                                                           *
 * All external APIs should go into the section below.       *
 * See the h-file for the external function prototypes,      *
 * description of parameters and return values.              *
 *                                                           *
 *************************************************************
 */

/*
 * Function: bcm_sbx_tx_pkt_setup
 *
 * Purpose: Fill in the device specific route header.  This routine should
 *          be called after the caller has established packet dest mod, port,
 *          and other packet attributes.  
 *
 * Parameters:
 *   tx_pkt   - pointer to packet to initialize.
 *
 * Returns:
 *   BCM_E_NONE - success
 *   BCM_E_xxx  - error
 */
int
bcm_sbx_tx_pkt_setup(int unit, bcm_pkt_t *tx_pkt)
{
#if defined(BCM_QE2000_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
    int rv;
#endif
    int i, pkt_len;

    TX_UNIT_VALID_CHECK(unit);

    if (tx_pkt == NULL) {
        return BCM_E_PARAM;
    }

    if (!BCM_IS_LOCAL(unit)) {
        /* Don't need to do special setup for tunneled packets */
        return BCM_E_NONE;
    }
    
    for (i=0, pkt_len=0; i < tx_pkt->blk_count; i++) {
        pkt_len += tx_pkt->pkt_data[i].len;
    }

    /* To comply with the api - bcm_tx expects all packets to contain room
     * for CRC in the packet data.  When BCM_TX_CRC_ALLOC flag is set, that
     * informs the driver the given packet does not have room for a CRC, and
     * should allocate additional space for one.  For SBX, that means do 
     * nothing.
     * With the BCM_TX_CRC_ALLOC clear, that informs the driver the given
     * packet has allocated space for the CRC in the packet data.  For SBX, 
     * that means reduce the length by 4, to remove the CRC.
     */
    if ((tx_pkt->flags & BCM_TX_CRC_ALLOC)) {
        tx_pkt->tot_len = pkt_len;
    } else {
        tx_pkt->tot_len = pkt_len - ENET_FCS_SIZE;
    }

#ifdef BCM_SIRIUS_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit))) {
        rv = sbx_sirius_tx_pkt_setup(unit, tx_pkt);
	return rv;
    }
#endif

#if defined(BCM_QE2000_SUPPORT)
    if((SOC_IS_SBX_QE2000(unit))) {
        rv = sbx_qe2000_tx_pkt_setup(unit, tx_pkt);
	return rv;
    }
#endif

    return BCM_E_NONE;
}


/*
 * Function: bcm_sbx_tx_init
 *
 * Purpose: Initialize internal data structures and bcmTx thread for
 *          asynchronous transmit.
 *
 * Parameters: none
 *
 *
 */
int
bcm_sbx_tx_init(int unit)
{
    char* msg = NULL;
    int   rv = BCM_E_NONE;

    TX_UNIT_VALID_CHECK(unit);

    BCM_SBX_LOCK(unit);

    if (!BCM_IS_LOCAL(unit)) {
        /* Set up TX tunnel receiver and transmitter */
        /* Currently this is done elsewhere */
	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
	rv = bcm_common_tx_init(unit);
	BCM_SBX_UNLOCK(unit);
        return rv;
    }    
#endif

    if (SBX_TX_INIT_DONE) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }

    sal_memset(&tx_state, 0, sizeof(tx_state));

    /* create semaphore and thread to handle user callbacks for 
     * asynchronous tx; create mutex for request list protection
     */
    if (pkt_tx_sem == NULL) {
        pkt_tx_sem = sal_sem_create("pkt tx sem", sal_sem_BINARY, 0);
        msg = "sem create";
        IF_GOTO_E((pkt_tx_sem == NULL), BCM_E_MEMORY);
    }

    if (tx_state.mtx == NULL) {
        tx_state.mtx = sal_mutex_create("sbx async Tx Mutex");
        msg = "mutex create";
        IF_GOTO_E((tx_state.mtx == NULL), BCM_E_MEMORY);
    }

    pkt_tx_thrd_id = sal_thread_create("bcmTx", SAL_THREAD_STKSZ,
                                       soc_property_get(unit,
                                                        spn_BCM_TX_THREAD_PRI,
                                                        DEFAULT_TX_THREAD_PRI),
                                       _sbx_tx_thread, NULL);
    msg = "thread create";
    IF_GOTO_E((pkt_tx_thrd_id == NULL), BCM_E_MEMORY);

    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;

error:
    _sbx_tx_cleanup();

    if (rv) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "bcmTx: Error=%d %s\n"), rv, msg));
    }

    
    BCM_SBX_UNLOCK(unit);
    return rv;
}


/*
 * Function: bcm_sbx_tx_detach
 *
 * Purpose:  Teardown bcmTx thread, clear internal data structures.
 *
 * Parameters: none
 *
 */
int
bcm_sbx_tx_detach(int unit)
{
    TX_UNIT_VALID_CHECK(unit);
    BCM_SBX_LOCK(unit);

    _sbx_tx_cleanup();

    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;
}


/*
 * Function: bcm_sbx_tx_array
 *
 * Purpose:  Send an array of bcm_pkts, synchronously or asynchronously.
 *
 * Parameters:
 *   pkt     Array of pointers to packets to send
 *   count   number of packets pkt points to - size of array
 *   all_done_cb  user callback function called after all packets are sent
 *   cookie       user defined cookied passed to all_done_cb
 *
 */
int
bcm_sbx_tx_array(int unit, bcm_pkt_t **pkt, int count, 
                 bcm_pkt_cb_f all_done_cb, void *cookie)
{
    int rv;

    if (count == 0) {
        return BCM_E_PARAM;
    }

    TX_UNIT_VALID_CHECK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
	rv = bcm_common_tx_array(unit, pkt, count, all_done_cb, cookie);
	BCM_SBX_UNLOCK(unit);
        return rv;
    }    
#endif

    BCM_SBX_LOCK(unit);
    rv = _sbx_tx_generic(unit, pkt, count, all_done_cb, cookie);
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/*
 * Function: bcm_sbx_tx
 *
 * Purpose: Send a single bcm_ptk_t pkt, synchronously.
 *
 * Parameters:
 *   pkt     packet to send
 *   cookie  ignored
 */
int
bcm_sbx_tx(int unit, bcm_pkt_t *pkt, void *cookie)
{
    int rv = BCM_E_PARAM;

    TX_UNIT_VALID_CHECK(unit);

    if (!BCM_IS_LOCAL(unit)) {
        /* Set up TX tunnel receiver and transmitter */
        /* Currently this is done elsewhere */
        return BCM_E_NONE;
    }

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
	rv = bcm_common_tx(unit, pkt, cookie);
        return rv;
    }    
#endif

    /* supporting only one dma buffer for now */
    if( pkt->blk_count == 0 || pkt->blk_count > 1 ) {
        return BCM_E_PARAM;
    }
 
    BCM_SBX_LOCK(unit);
 
    rv = bcm_sbx_tx_array(unit, &pkt, 1, NULL, NULL);

    BCM_SBX_UNLOCK(unit);
    return rv;
}

/*
 * Function: bcm_sbx_tx_list
 *
 * Purpose:  Send a linked list of bcm_pkt_t, linked via the 'next'
 *           bcm_pkt_t data member.
 *
 * Parameters:
 *   pkt -  pointer to head of list.
 *   all_done_cb  - user callback function called after all packets 
 *                  have been sent
 *   cookie       - user cookie passed to user callback
 */
int
bcm_sbx_tx_list(int unit, bcm_pkt_t *pkt, 
                bcm_pkt_cb_f all_done_cb, void *cookie)
{
    int rv;

    TX_UNIT_VALID_CHECK(unit);
    BCM_SBX_LOCK(unit);

#ifdef BCM_SBX_CMIC_SUPPORT
    if((SOC_IS_SBX_SIRIUS(unit)) || (SOC_IS_SBX_CALADAN3(unit))) {
	rv = bcm_common_tx_list(unit, pkt, all_done_cb, cookie);
	BCM_SBX_UNLOCK(unit);
        return rv;
    }    
#endif

    /* coverity[callee_ptr_arith] */
    rv = _sbx_tx_generic(unit, &pkt, 0, all_done_cb, cookie);

    BCM_SBX_UNLOCK(unit);
    return rv;
}

STATIC int
_sbx_tx_generic(int unit, bcm_pkt_t **pkt, int count, 
                bcm_pkt_cb_f all_done_cb, void *cookie)
{    
    int rv = BCM_E_NONE;
    int warn_rv = BCM_E_NONE;
    bcm_pkt_t *tx_pkt;
    int pkt_idx = 1;
    int retry_count = 0;
    char* sbx_hdr_p;
    char sbx_hdr[24]; 
    int  sbx_hdr_len;

    if (!pkt) {
        return BCM_E_PARAM;
    }

    TX_UNIT_VALID_CHECK(unit);

    if (!BCM_IS_LOCAL(unit)) { /* Tunnel the packet to the remote CPU */
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "bcm_tx_list ERROR:  Cannot tunnel\n")));
        return BCM_E_PARAM;
    }

    if (!SBX_TX_INIT_DONE) {
        rv = bcm_sbx_tx_init(unit);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }

    /* age the list to try to catch memory leaks from failed attempts,
     * this should not be the normal case
     */
    _sbx_tx_req_age(&tx_state.active_list, &tx_state.mtx);


    /* no callback supplied, send synchronously */
    tx_pkt = *pkt;
    if (all_done_cb == NULL) {

        while (tx_pkt) {

	    if (!(tx_pkt->flags & BCM_TX_SBX_READY)) {
	      bcm_tx_pkt_setup(unit, tx_pkt);
	    }

            if ((tx_pkt->flags & BCM_TX_HG_READY)) {
                sbx_hdr_p = &sbx_hdr[0];
                sal_memcpy(&sbx_hdr[0], tx_pkt->_sbx_rh, tx_pkt->_sbx_hdr_len);
                sal_memcpy(&sbx_hdr[tx_pkt->_sbx_hdr_len], tx_pkt->_higig, 12); 
                sbx_hdr_len = tx_pkt->_sbx_hdr_len + 12;
            } else {
                sbx_hdr_p = (char*)tx_pkt->_sbx_rh;
                sbx_hdr_len = tx_pkt->_sbx_hdr_len;
            }

            if (tx_pkt->tot_len < (ENET_MIN_PKT_SIZE - ENET_FCS_SIZE)) {
                LOG_WARN(BSL_LS_BCM_TX,
                         (BSL_META_U(unit,
                                     "Packet too small: pktLen=%d min=%d, skipping\n"),
                          tx_pkt->tot_len, ENET_MIN_PKT_SIZE - ENET_FCS_SIZE));
                warn_rv = BCM_E_PARAM;

            } else {
                retry_count = 0;
                do {
                    /* Use tot_len to allow for CRC_ALLOC adjustment */
                    int buf_len = tx_pkt->tot_len;

                    rv = soc_sbx_txrx_sync_tx(unit, sbx_hdr_p,
                                              sbx_hdr_len,
                                              (char *) tx_pkt->pkt_data[0].data,
                                              buf_len,
                                              ((SAL_BOOT_BCMSIM) ? 10000000 : 
                                               1000000));
                    if (BCM_FAILURE(rv)) {
                        if (rv == SOC_E_RESOURCE) {
                            retry_count++;
                        } else {
                            return rv;
                        }
                    } else {
                        break;
                    }
                } while (retry_count < MAX_RETRY);
            }

            /* Get next packet */
            /* count == 0 means list
             * count non-zero, means array
             */
            if (count == 0) {                
                tx_pkt = tx_pkt->next;
            } else {
                if (pkt_idx == count) {
                    tx_pkt = NULL;
                } else {
                    tx_pkt = *(pkt + pkt_idx++);
                }
            }
        }

    } else {

        /* send array asynchrounously */
        async_context_t  *tx_send_ctx;
        int pkt_count = 0;

        tx_send_ctx = sal_alloc(sizeof(async_context_t), "tx_context");
        if (tx_send_ctx == NULL) {
            return BCM_E_MEMORY;
        }
        
        if (count == 0) {
            tx_pkt = *pkt;
            while (++pkt_count && (tx_pkt = tx_pkt->next)) ;
        } else{
            pkt_count = count;
        }   

        
        tx_send_ctx->count = pkt_count;
        tx_send_ctx->cb = all_done_cb;
        tx_send_ctx->cookie = cookie;
        tx_send_ctx->unit = unit;
        tx_send_ctx->first_pkt = *pkt;
        tx_send_ctx->next = NULL;
        tx_send_ctx->start_time = sal_time_usecs();
        
        _sbx_tx_req_add(&tx_state.active_list, tx_send_ctx, &tx_state.mtx);

        tx_pkt = *pkt;
        while (tx_pkt) {

            if ((tx_pkt->flags & BCM_TX_HG_READY)) {
                sbx_hdr_p = &sbx_hdr[0];
                sal_memcpy(&sbx_hdr[0], tx_pkt->_sbx_rh, tx_pkt->_sbx_hdr_len);
                sal_memcpy(&sbx_hdr[tx_pkt->_sbx_hdr_len], tx_pkt->_higig, 12);
                sbx_hdr_len = tx_pkt->_sbx_hdr_len + 12;
            } else {
                sbx_hdr_p = (char*)tx_pkt->_sbx_rh;
                sbx_hdr_len = tx_pkt->_sbx_hdr_len;
            }
            
            if (tx_pkt->tot_len < (ENET_MIN_PKT_SIZE - ENET_FCS_SIZE)) {
                LOG_WARN(BSL_LS_BCM_TX,
                         (BSL_META_U(unit,
                                     "Packet too small: pktLen=%d min=%d, skipping\n"),
                          tx_pkt->tot_len, ENET_MIN_PKT_SIZE - ENET_FCS_SIZE));
                warn_rv = BCM_E_PARAM;
                
            } else {
                retry_count = 0;
                do {
                    /* Use tot_len to allow for CRC_ALLOC adjustment */
                    int buf_len = tx_pkt->tot_len;

                    rv = soc_sbx_txrx_tx(unit, sbx_hdr_p, 
                                         sbx_hdr_len,
                                         1,
                                         (void **) &tx_pkt->pkt_data[0].data,
                                         &buf_len,
                                         _sbx_tx_done, tx_send_ctx);
                    
                    if (BCM_FAILURE(rv)) {
                        if (rv == SOC_E_RESOURCE) {
                            retry_count++;
                        } else {
                            return rv;
                        }
                    } else {
                        break;
                    }
                } while (retry_count < MAX_RETRY);
            }

            if (count == 0) {
                tx_pkt = tx_pkt->next;
            } else {
                if (pkt_idx == count) {
                    tx_pkt = NULL;
                } else {
                    tx_pkt = *(pkt + pkt_idx++);
                }
            }
        }
    }

    if (warn_rv != BCM_E_NONE) {
        rv = warn_rv;
    }
    return rv;
}

/*
 * Function: _sbx_tx_thread
 *
 * Purpose:  Process list of completed transmit requests from async processing.
 *
 */
STATIC void
_sbx_tx_thread(void *param)
{
    char thread_name[SAL_THREAD_NAME_MAX_LEN];
    sal_thread_t	thread;

    thread = sal_thread_self();
    thread_name[0] = 0;
    sal_thread_name(thread, thread_name, sizeof (thread_name));
  
    while(1) {
        int rv = BCM_E_NONE;
        async_context_t *entry;
        
        if (sal_sem_take(pkt_tx_sem, sal_sem_FOREVER) < 0) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META("AbnormalThreadExit:%s\n"), thread_name));
            break;
        }
        
        while (BCM_SUCCESS(rv)) {
            entry = NULL;
            rv = _sbx_tx_req_remove(&tx_state.done_list, 
                                    &entry, 
                                    &tx_state.mtx);

            if (BCM_SUCCESS(rv) && entry) {
                LOG_VERBOSE(BSL_LS_BCM_TX,
                            (BSL_META("tx done first_pkt=0x%08x cookie=0x%08x\n"),
                             (uint32)entry->first_pkt, (uint32)entry->cookie));
                entry->cb(entry->unit, entry->first_pkt, entry->cookie);
                sal_free(entry);
            }
        }
    }
}


/*
 * Function: _sbx_tx_done
 *
 * Purpose:  Callback function passed to SOC layer for async transactions
 *
 */
STATIC void
_sbx_tx_done (int unit, soc_sbx_txrx_active_t *done)
{
    async_context_t *ctx = (async_context_t*)done->cookie;
    int err;

    err = sal_mutex_take(tx_state.mtx, sal_mutex_FOREVER);
    if (BCM_FAILURE(err)) {
        return;
    }

    if (_sbx_tx_req_is_member(&tx_state.active_list, ctx, NULL))
    {
        /* soc_sbx_txrx_tx returns when one or more requests have been processed.
         * Reconcile the number of requests handled with the number of requests
         * sent.
         */
        for (;done; done = done->next) {
            ctx->count--;
        }

        if (ctx->count == 0) {
            int rv;

            rv = _sbx_tx_req_remove(&tx_state.active_list, &ctx, NULL);

            /* wake the Tx thread to process the user callback and free the
             * send context
             */
            if (BCM_SUCCESS(rv)) {
                _sbx_tx_req_add(&tx_state.done_list, ctx, NULL);
                sal_sem_give(pkt_tx_sem);
            }
        }
    }

    sal_mutex_give(tx_state.mtx);
}

/*
 * Function: _sbx_tx_req_add
 *
 * Purpose: Add an async context to the given list.  Used to track active
 *          and completed transactions.
 *
 */
STATIC int
_sbx_tx_req_add(tx_req_list_t *list, async_context_t *entry, sal_mutex_t *mtx)
{
    int rv = BCM_E_NONE;

    if (mtx) {
        rv = sal_mutex_take(*mtx, sal_mutex_FOREVER);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }

    if (list->head == NULL) {
        list->head = list->tail = entry;
    } else {
        list->tail->next = entry;
        list->tail = entry;
    }

    list->count++;

    if (mtx) {
        rv = sal_mutex_give(*mtx);
    }
    return rv;
}

/*
 * Function: _sbx_tx_req_is_member
 *
 * Purpose: Determine if the given entry exists in the list
 */
STATIC int  
_sbx_tx_req_is_member(tx_req_list_t   *list,
                      async_context_t *entry,
                      sal_mutex_t     *mtx)
{
    int err = BCM_E_NONE;
    async_context_t *cur;

    if (mtx) {
        err = sal_mutex_take(*mtx, sal_mutex_FOREVER);
        if (BCM_FAILURE(err)) {
            return 0;
        }
    }

    cur = list->head;
    while(cur) {
        if (cur == entry) {
            if (mtx) {
                sal_mutex_give(*mtx);                
            }
            return 1;
        }
        cur = cur->next;
    }

    if (mtx) {
        sal_mutex_give(*mtx);
    }

    return 0;
}

/*
 * Function: _sbx_tx_req_remove
 *
 * Purpose: Remove an entry from the given async context list.  If entry
 *          points to a NULL entry, the routine removes the head, otherwise
 *          it scans the list for *entry.
 */
STATIC int
_sbx_tx_req_remove(tx_req_list_t *list, 
                   async_context_t **entry, 
                   sal_mutex_t *mtx)
{
    int rv = BCM_E_NONE;
    async_context_t *cur, *one_back;

    if (mtx) {
        rv = sal_mutex_take(*mtx, sal_mutex_FOREVER);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }
    
    /* list empty - error */
    IF_GOTO((list->head == NULL), BCM_E_EMPTY, error);

    if (*entry == NULL) {
        *entry = list->head;
    }

    /* find the entry */
    one_back = NULL;
    cur = list->head;
    
    while (cur && cur != *entry) {
        one_back = cur;
        cur      = cur->next;
    }

    IF_GOTO((cur != *entry), BCM_E_FAIL, error);

    if (cur == list->head) {
        if (cur == list->tail) {
            list->head = list->tail = NULL;
        } else {
            list->head = cur->next;
        }
    } else {
        IF_GOTO((one_back == NULL), BCM_E_FAIL, error);
        
        one_back->next = cur->next;

        if (cur == list->tail) {
            list->tail = one_back;
            list->tail->next = NULL;
        }
    }

    cur->next = NULL;
    list->count--;
    
error:
    if (mtx) {
        sal_mutex_give(*mtx);
    }
    return rv;
}


/*
 * Function: _sbx_tx_req_age
 *
 * Purpose: Age out entries in the request list that fail the 
 *          soc_timeout_check.  This age routine is used to catch 
 *          any memory leak due to failed soc_sbx_txrx calls when
 *          the callback is not called.  It is not a compelete
 *          solution, but should be sufficient enough to catch most
 *          problems, and avoid running out of memory in a system
 * 
 */
STATIC int
_sbx_tx_req_age(tx_req_list_t *list, sal_mutex_t *mtx)
{
    int rv = BCM_E_NONE;
    async_context_t *cur;

    if (mtx) {
        rv = sal_mutex_take(*mtx, sal_mutex_FOREVER);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }

    cur = list->head;

    while (cur) {
        async_context_t *age_entry = NULL;
        async_context_t *next = cur->next;

        if (sal_time_usecs() > SAL_USECS_ADD(cur->start_time, MAX_TIMEOUT)) {
            /* Entry aged out, remove it from the list, assume the tx failed
             * in the soc layer
             */
            rv = _sbx_tx_req_remove(list, &cur, NULL);
            if (BCM_SUCCESS(rv)) {
                age_entry = cur;
            }
        }

        cur = next;

        if (age_entry) {           
            sal_free(age_entry); 
        }
    }

    if (mtx) {
        sal_mutex_give(*mtx);
    }
    return rv;
}


/* 
 * Function: _sbx_tx_cleanup
 *
 * Purpose:  Cleanup internal data structures, shut down bcmTx thread.
 *
 */
STATIC void
_sbx_tx_cleanup(void)
{
    if (pkt_tx_sem) {
        sal_sem_destroy(pkt_tx_sem);
        pkt_tx_sem = NULL;
    }

    if (tx_state.mtx) {
        sal_mutex_destroy(tx_state.mtx);
    }

    if (pkt_tx_thrd_id) {
        sal_thread_destroy(pkt_tx_thrd_id);
    }

    sal_memset(&tx_state, 0, sizeof(tx_state));
}

