/******************************************************************************
*
* $Id: sbx_txrx.c,v 1.27 Broadcom SDK $
*
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
******************************************************************************/

#include <shared/bsl.h>

#include <soc/error.h>
#include <soc/defs.h>
#include <sal/appl/sal.h>
#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <sal/appl/io.h>
#include <soc/drv.h>
#include <soc/debug.h>
#include <assert.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbWrappers.h>
#include <soc/sbx/sbx_txrx.h>
#include <soc/sbx/hal_user.h>
#include <soc/sbx/hal_ka_auto.h>
#ifdef BCM_FE2000_SUPPORT
#include <soc/sbx/sbG2FeEgrRouteHeader.h>
#include <soc/sbx/sbG2SsFeIngRouteHeader.h>
#include <soc/sbx/sbG2QeSsFeIngRouteHeader.h>
#endif

#ifndef BE_HOST
#define BE_HOST 0
#endif

/* Hardware-specific parameters */
#define TX_RING_ENTRY_SIZE (4 * 4)
#define COMPLETION_RING_ENTRY_SIZE (4)
#define MAX_MAX_RX_BUFFERS 256
#define MAX_TX_IMMEDIATE_LENGTH 12
#define RXBUF_LOAD_REGS 32

#define TX_RING_CTRLLEN_OFFSET          0
#define TX_RING_IMM_TYPE                0x80000000
#define TX_RING_SOP                     0x40000000
#define TX_RING_EOP                     0x20000000
#define TX_RING_LENGTH_SHIFT            8
#define TX_RING_LENGTH_MAX              16384
#define TX_RING_IMMEDIATE_DATA_OFFSET   4
#define TX_RING_POINTER_OFFSET          4

#define COMPLETION_RING_TX              0x80000000
#define COMPLETION_RING_ERROR           0x40000000
#define COMPLETION_RING_BUFS_USED_MASK  0x000fc000
#define COMPLETION_RING_BUFS_USED_SHIFT 14
#define COMPLETION_RING_LENGTH_MASK     0x00003fff

/* Default parameters */
#define DEFAULT_TX_RING_ENTRIES 8
#define DEFAULT_COMPLETION_RING_ENTRIES (2 * DEFAULT_TX_RING_ENTRIES)
#define DEFAULT_RX_BUFFER_SIZE 16384
#define DEFAULT_MAX_RX_BUFFERS MAX_MAX_RX_BUFFERS

static int txrx_spl;
#define TXRX_INTR_LOCK      txrx_spl = sal_splhi()
#define TXRX_INTR_UNLOCK    sal_spl(txrx_spl)

#define HOST_TO_LE(unit, v) \
 (((CMVEC((unit)).big_endian_other && !BE_HOST)        \
   || (!CMVEC((unit)).big_endian_other && BE_HOST))    \
  ? ((((v) >> 24) & 0xff) | (((v) >> 8) & 0xff00)      \
     | (((v) & 0xff00) << 8) | (((v) & 0xff) << 24))   \
  : (v))
#define LE_TO_HOST(unit,v) HOST_TO_LE(unit, v)

#define ACTIVE_ENQ(h, t, e) \
do {                           \
    if (!(t)) {                \
        assert(!(h));          \
        (h) = (e);             \
    } else {                   \
        assert(!(t)->next);    \
        (t)->next = (e);       \
    }                          \
    (t) = (e);                 \
    (e)->next = NULL;          \
} while (0);

#define ACTIVE_DEQ(h, t, e)    \
do {                           \
    assert(h);                 \
    assert(t);                 \
    (e) = (h);                 \
    (h) = (e)->next;           \
    if (!(h)) (t) = NULL;      \
} while (0);

static int
_is_power_of_2(uint v)
{
    return ((v & (v-1)) == 0);
}

#ifdef BCM_FE2000_SUPPORT
static int
soc_sbx_hdr_ss_field_set(int unit, uint8 *hdr, uint8 hdr_len, SBX_rh_fields_t field, uint32 val);
static int
soc_sbx_hdr_qess_field_set(int unit, uint8 *hdr, uint8 hdr_len, SBX_rh_fields_t field, uint32 val);
static int
soc_sbx_hdr_default_field_set(int unit, uint8 *hdr, uint8 hdr_len, SBX_rh_fields_t field, uint32 val);
static int
soc_sbx_hdr_ss_field_get(int unit, uint8 *hdr, uint8 hdr_len, SBX_rh_fields_t field, uint32 *val);
static int
soc_sbx_hdr_qess_field_get(int unit, uint8 *hdr, uint8 hdr_len, SBX_rh_fields_t field, uint32 *val);
static int
soc_sbx_hdr_default_field_get(int unit, uint8 *hdr, uint8 hdr_len, SBX_rh_fields_t field, uint32 *val);
#endif
static int
rx_buffer_size_param (uint v, uint *p)
{
    uint i;

    if (v < 256 || v > 16384) {
        return 0;
    }

    for (i = 0; i < 7; i++) {
        if (v == (1 << (i + 8))) {
            *p = i;
            return 1;
        }
    }

    return 0;
}

#define TXRX_INIT_CHECK(unit) \
if(SOC_SBX_CONTROL(unit)->txrx_mutex == NULL) {  \
    if(soc_sbx_txrx_init(unit)) {                \
        return SOC_E_INIT;                       \
    }                                            \
}                                                \

int
soc_sbx_txrx_init (int unit)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    int rv = SOC_E_MEMORY;
    int i;


    if (!sbx->tx_ring_entries) {
        sbx->tx_ring_entries = DEFAULT_TX_RING_ENTRIES;
    }

    if (!_is_power_of_2(sbx->tx_ring_entries)) {
        return SOC_E_PARAM;
    }

    if (!sbx->completion_ring_entries) {
        sbx->completion_ring_entries = DEFAULT_COMPLETION_RING_ENTRIES;
    }

    if (!_is_power_of_2(sbx->completion_ring_entries)) {
        return SOC_E_PARAM;
    }

    if (!sbx->rx_buffer_size) {
        sbx->rx_buffer_size = DEFAULT_RX_BUFFER_SIZE;
    }


    if (!sbx->max_rx_buffers) {
        sbx->max_rx_buffers = DEFAULT_MAX_RX_BUFFERS;
    }

    if (sbx->max_rx_buffers > MAX_MAX_RX_BUFFERS) {
        /* Exceeds the size of the hardware RX buffer FIFO */
        /* For now, this is a fixed value, eventually it can vary by chip */
        return SOC_E_PARAM;
    }

    sbx->tx_ring_mem =
        soc_cm_salloc(unit, sbx->tx_ring_entries * TX_RING_ENTRY_SIZE * 2,
                      "SBX tx ring");
    if (!sbx->tx_ring_mem) {
        return SOC_E_MEMORY;
    }

    /* Hardware requires ring aligned to a multiple of its size */
    i = (sbx->tx_ring_entries * TX_RING_ENTRY_SIZE)
        - (PTR_TO_INT(sbx->tx_ring_mem)
           % (sbx->tx_ring_entries * TX_RING_ENTRY_SIZE));
    sbx->tx_ring = (uint32 *) (sbx->tx_ring_mem + i);

    if (!(sbx->txrx_active_mem
          = sal_alloc(2 * (sbx->tx_ring_entries + sbx->max_rx_buffers)
                        * sizeof(soc_sbx_txrx_active_t),
                        "SBX TX/RX active free"))) {
        goto err0;
    }

    sbx->completion_ring_mem =
        soc_cm_salloc(unit,
                      sbx->completion_ring_entries
                      * COMPLETION_RING_ENTRY_SIZE * 2,
                      "SBX completion ring");

    if (!sbx->completion_ring_mem) {
        goto err1;
    }

    /* Hardware requires ring aligned to a multiple of its size */
    i = (sbx->completion_ring_entries * COMPLETION_RING_ENTRY_SIZE)
        - (PTR_TO_INT(sbx->completion_ring_mem)
           % (sbx->completion_ring_entries * COMPLETION_RING_ENTRY_SIZE));
    sbx->completion_ring = (uint32 *) (sbx->completion_ring_mem + i);

    if (!(sbx->txrx_mutex = sal_mutex_create("SBX TX/RX mutex"))) {
        goto err2;
    }

    if (!(sbx->txrx_sync_sem = sal_sem_create("SBX TX/RX sync semaphore",
                                              sal_sem_BINARY, 0))) {
        goto err3;
    }

    rv = soc_sbx_txrx_init_hw_only(unit);

    return rv;


 err3:
    sal_mutex_destroy(sbx->txrx_mutex);
    sbx->txrx_mutex = NULL;
 err2:
    soc_cm_sfree(unit, sbx->completion_ring_mem);
    sbx->completion_ring = NULL;
 err1:
    sal_free(sbx->txrx_active_mem);
    sbx->txrx_active_mem = NULL;
 err0:
    soc_cm_sfree(unit, sbx->tx_ring_mem);
    sbx->tx_ring = NULL;

    return rv;
}

int
soc_sbx_txrx_init_hw_only (int unit)
{
    int i;
    uint rxbuf_size;
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    SOC_SBX_WARM_BOOT_DECLARE(int wb);

    if (!rx_buffer_size_param(sbx->rx_buffer_size, &rxbuf_size)) {
        return SOC_E_PARAM;
    }

    for (i = 0; i < sbx->tx_ring_entries + sbx->max_rx_buffers - 1; i++) {
        sbx->txrx_active_mem[i].next = &sbx->txrx_active_mem[i+1];
    }

    sbx->txrx_active_mem[i].next = NULL;
    sbx->txrx_active_free = sbx->txrx_active_mem;
    sbx->completion_ring_consumer = 0;

    sbx->rx_active = 0;
    sbx->rx_active_head = NULL;
    sbx->rx_active_tail = NULL;

    sbx->tx_ring_producer = 0;
    sbx->tx_ring_entries_active = 0;
    sbx->tx_active = 0;
    sbx->tx_active_head = NULL;
    sbx->tx_active_tail = NULL;

    sal_memset(sbx->tx_ring, 0, sbx->tx_ring_entries * TX_RING_ENTRY_SIZE);
    sal_memset(sbx->completion_ring, 0,
               sbx->completion_ring_entries * COMPLETION_RING_ENTRY_SIZE);

    /* always allow the ring buffers to be updated.
     */

    if (SOC_WARM_BOOT(unit)) {
        SOC_SBX_WARM_BOOT_IGNORE(unit, wb);

        /* Recover the RX ring by setting consumer equal to producer */
        sbx->completion_ring_consumer =
            SAND_HAL_READ_OFFS(sbx->sbhdl, sbx->completion_ring_producer_reg);
        SAND_HAL_WRITE_OFFS(sbx->sbhdl, sbx->completion_ring_consumer_reg,
                            sbx->completion_ring_consumer);
        sbx->completion_ring_consumer = sbx->completion_ring_consumer /
            COMPLETION_RING_ENTRY_SIZE;
        SOC_SBX_WARM_BOOT_OBSERVE(unit, wb);
    }

    SOC_SBX_WARM_BOOT_IGNORE(unit, wb);

    SAND_HAL_WRITE_OFFS(sbx->sbhdl , sbx->tx_ring_reg,
                        soc_cm_l2p(unit, sbx->tx_ring));
    SAND_HAL_WRITE_OFFS(sbx->sbhdl, sbx->tx_ring_size_reg,
                        sbx->tx_ring_entries * TX_RING_ENTRY_SIZE - 1);

    SAND_HAL_WRITE_OFFS(sbx->sbhdl , sbx->completion_ring_reg,
                        soc_cm_l2p(unit, sbx->completion_ring));
    SAND_HAL_WRITE_OFFS(sbx->sbhdl, sbx->completion_ring_size_reg,
                        sbx->completion_ring_entries
                        * COMPLETION_RING_ENTRY_SIZE - 1);

    SAND_HAL_WRITE_OFFS(sbx->sbhdl, sbx->rxbuf_size_reg, rxbuf_size);

    SOC_SBX_WARM_BOOT_OBSERVE(unit, wb);

    /* Recover the TX Ring Producer index values from hardware.
     */
    if (SOC_WARM_BOOT(unit)) {
        sbx->tx_ring_producer =
            SAND_HAL_READ_OFFS(sbx->sbhdl, sbx->tx_ring_producer_reg) /
            TX_RING_ENTRY_SIZE;
    }

    return SOC_E_NONE;
}

int
soc_sbx_txrx_uninit_hw_only (int unit)
{

    int i;

    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    /* disable DMA from device
     * -set ring size to zero
     * -set consuemr to zero
     * -write 1 to pc_rxbuf_fifo_debug.pop_fifo  32 times to flush packet data 
     */
    SAND_HAL_WRITE_OFFS(sbx->sbhdl, sbx->completion_ring_size_reg, 0);
    SAND_HAL_WRITE_OFFS(sbx->sbhdl, sbx->completion_ring_consumer_reg, 0);
    sbx->completion_ring_consumer=0;

    for (i=0; i<32; i++) {
        SAND_HAL_WRITE_OFFS(sbx->sbhdl, sbx->rxbufs_pop_reg, sbx->rxbufs_pop_bit);
    }

    return SOC_E_NONE;
}

int
soc_sbx_txrx_give_rx_buffers (int unit, int bufs, void **bufps,
                              soc_sbx_txrx_done_f donecb, void **cookies)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    soc_sbx_txrx_active_t *active;
    int i;

    TXRX_INIT_CHECK(unit);

    sal_mutex_take(sbx->txrx_mutex, sal_mutex_FOREVER);
    TXRX_INTR_LOCK;
    if (sbx->rx_active + bufs > sbx->max_rx_buffers) {
        TXRX_INTR_UNLOCK;
        sal_mutex_give(sbx->txrx_mutex);
        return SOC_E_PARAM;
    }
    TXRX_INTR_UNLOCK;

    for (i = 0; i<  bufs; i++) {
        if (sbx->rx_debug_fill) {
            sal_memset(bufps[i], sbx->rx_debug_fill_val, sbx->rx_buffer_size);
            soc_cm_sflush(unit, bufps[i], sbx->rx_buffer_size);
        }
        TXRX_INTR_LOCK;
        
        /* use PC_RXBUF_LOAD0 over and over to avoid model bug, but load 
         * consecutive addresses when on real hardware for PCI burst
         */

#ifdef PLISIM
        SAND_HAL_WRITE_OFFS(sbx->sbhdl,
                            sbx->rxbuf_load_reg,
                            soc_cm_l2p(unit, bufps[i]));
#else  /* PLISIM */
        SAND_HAL_WRITE_OFFS(sbx->sbhdl,
                            sbx->rxbuf_load_reg + (i % RXBUF_LOAD_REGS) * 4,
                            soc_cm_l2p(unit, bufps[i]));
#endif  /* PLISIM */

        assert(sbx->txrx_active_free);
        active = sbx->txrx_active_free;
        sbx->txrx_active_free = active->next;

        active->status = SOC_E_NONE;
        active->cookie = cookies[i];
        active->donecb = donecb;
        active->entries = 1;

        ACTIVE_ENQ(sbx->rx_active_head, sbx->rx_active_tail, active);
        sbx->rx_active++;
        TXRX_INTR_UNLOCK;
    }
    
    sal_mutex_give(sbx->txrx_mutex);
    return SOC_E_NONE;
}

int
soc_sbx_txrx_remove_rx_buffers (int unit, int bufs)
{
    int i;
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    soc_sbx_txrx_active_t *active;

    TXRX_INTR_LOCK;
    for (i=0; i<bufs && sbx->rx_active_head; i++) {
        SAND_HAL_WRITE_OFFS(sbx->sbhdl, sbx->rxbufs_pop_reg,
                            SAND_HAL_READ_OFFS(sbx->sbhdl,
                                               sbx->rxbufs_pop_reg)
                            | sbx->rxbufs_pop_bit);
        sbx->rx_active--;
        ACTIVE_DEQ(sbx->rx_active_head, sbx->rx_active_tail, active);
        active->next = sbx->txrx_active_free;
        sbx->txrx_active_free = active;
    }

    if (sbx->rx_active < 0) {
        sbx->rx_active = 0;
    }
    TXRX_INTR_UNLOCK;

    return SOC_E_NONE;
}

int
soc_sbx_txrx_tx (int unit, char *hdr, int hdrlen, int bufs, void **bufps,
                 int *buflens, soc_sbx_txrx_done_f donecb, void *cookie)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    uint32 *entryp, *first_entryp;
    soc_sbx_txrx_active_t *active;
    int first_index;
    int entries;
    int i;

    if (hdrlen > 2 * MAX_TX_IMMEDIATE_LENGTH) {
        return SOC_E_PARAM;
    }

    entries = (hdrlen ? 1 : 0) + bufs;
    if (!entries) {
        return SOC_E_PARAM;
    }
    if (hdrlen > MAX_TX_IMMEDIATE_LENGTH) {
       entries++;
    }

    sal_mutex_take(sbx->txrx_mutex, sal_mutex_FOREVER);
    TXRX_INTR_LOCK;
    /* I think we can only give # entries - 1 */
    /* because consumer == producer means empty */
    if (sbx->tx_ring_entries_active + entries + 1 > sbx->tx_ring_entries) {
        TXRX_INTR_UNLOCK;
        sal_mutex_give(sbx->txrx_mutex);
        return SOC_E_RESOURCE;
    }
    TXRX_INTR_UNLOCK;

    first_index = sbx->tx_ring_producer;
    entryp =
            &sbx->tx_ring[sbx->tx_ring_producer
                     * TX_RING_ENTRY_SIZE/sizeof(uint32)];
    first_entryp = entryp;

    if (hdrlen > MAX_TX_IMMEDIATE_LENGTH) {
        entryp[TX_RING_CTRLLEN_OFFSET/sizeof(uint32)] =
            HOST_TO_LE(unit,
                         (TX_RING_IMM_TYPE
                          | (MAX_TX_IMMEDIATE_LENGTH << TX_RING_LENGTH_SHIFT)));
        sal_memcpy(&entryp[TX_RING_IMMEDIATE_DATA_OFFSET/sizeof(uint32)],
                   hdr, MAX_TX_IMMEDIATE_LENGTH);
        sbx->tx_ring_producer =
            (sbx->tx_ring_producer + 1) % sbx->tx_ring_entries;
        hdrlen -= MAX_TX_IMMEDIATE_LENGTH;
        hdr += MAX_TX_IMMEDIATE_LENGTH;
        entryp =
            &sbx->tx_ring[sbx->tx_ring_producer
                     * TX_RING_ENTRY_SIZE/sizeof(uint32)];
    }

    if (hdrlen > 0) {
        /* coverity[result_independent_of_operands] */
        entryp[TX_RING_CTRLLEN_OFFSET/sizeof(uint32)] =
            HOST_TO_LE(unit,
                         (TX_RING_IMM_TYPE
                          | (hdrlen << TX_RING_LENGTH_SHIFT)));
        sal_memcpy(&entryp[TX_RING_IMMEDIATE_DATA_OFFSET/sizeof(uint32)],
                   hdr, hdrlen);
        sbx->tx_ring_producer =
            (sbx->tx_ring_producer + 1) % sbx->tx_ring_entries;
    }

    for (i = 0; i < bufs; i++) {
        entryp =
            &sbx->tx_ring[sbx->tx_ring_producer
                     * TX_RING_ENTRY_SIZE/sizeof(uint32)];
        /* Coverity misinterprets a segment of conversion from Host to LE
           as the operation and flags this issue inappropriately. */
        /* coverity[operator_confusion] */
        entryp[TX_RING_CTRLLEN_OFFSET/sizeof(uint32)] =
            HOST_TO_LE(unit, (buflens[i] << TX_RING_LENGTH_SHIFT));
        entryp[TX_RING_POINTER_OFFSET/sizeof(uint32)] =
            HOST_TO_LE(unit, soc_cm_l2p(unit, bufps[i]));
        sbx->tx_ring_producer =
            (sbx->tx_ring_producer + 1) % sbx->tx_ring_entries;
    }

    first_entryp[TX_RING_CTRLLEN_OFFSET/sizeof(uint32)]
        |= HOST_TO_LE(unit, TX_RING_SOP);
    entryp[TX_RING_CTRLLEN_OFFSET/sizeof(uint32)]
        |= HOST_TO_LE(unit, TX_RING_EOP);

    for (i = 0; i < bufs; i++) {
        soc_cm_sflush(unit, bufps[i], buflens[i]);
    }

    if (first_index < sbx->tx_ring_producer) {
        soc_cm_sflush(unit, first_entryp,
                     (sbx->tx_ring_producer - first_index)
                     * TX_RING_ENTRY_SIZE);
    } else {
        /* Wrapped around the ring, so flush the 2 portions */
        soc_cm_sflush(unit, first_entryp,
                     (sbx->tx_ring_entries - first_index)
                      * TX_RING_ENTRY_SIZE);
        soc_cm_sflush(unit, sbx->tx_ring,
                     sbx->tx_ring_producer * TX_RING_ENTRY_SIZE);
    }

    TXRX_INTR_LOCK;

    SAND_HAL_WRITE_OFFS(sbx->sbhdl, sbx->tx_ring_producer_reg,
                        sbx->tx_ring_producer * TX_RING_ENTRY_SIZE);

    assert(sbx->txrx_active_free);
    active = sbx->txrx_active_free;
    sbx->txrx_active_free = active->next;

    active->status = SOC_E_NONE;
    active->cookie = cookie;
    active->donecb = donecb;
    active->entries = entries;

    ACTIVE_ENQ(sbx->tx_active_head, sbx->tx_active_tail, active);

    sbx->tx_ring_entries_active += entries;
    sbx->tx_active++;

    TXRX_INTR_UNLOCK;
    sal_mutex_give(sbx->txrx_mutex);
    return SOC_E_NONE;
}



static void
sync_txrx_done (int unit, soc_sbx_txrx_active_t *done)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    soc_sbx_txrx_active_t *op = (soc_sbx_txrx_active_t *) done->cookie;

    op->status = done->status;
    op->rxlen = done->rxlen;

    sal_sem_give(sbx->txrx_sync_sem);
}

int
soc_sbx_txrx_sync_tx (int unit, char *hdr, int hdrlen, char *buf,
                      int buflen, int waitusec)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    int rv;
    soc_sbx_txrx_active_t op;
    void *tmpbuf = buf;

    TXRX_INIT_CHECK(unit);

    /* tmpbuf required to make vxworks compiler happy */
    if ((rv = soc_sbx_txrx_tx(unit, hdr, hdrlen, 1, (void **) &tmpbuf, &buflen,
                              sync_txrx_done, &op))) {
        return rv;
    }

    if (sal_sem_take(sbx->txrx_sync_sem, waitusec) != 0) {
        return SOC_E_TIMEOUT;
    }

    return op.status;
}

int
soc_sbx_txrx_sync_rx (int unit, char *buf, int *buflen, int waitusec)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    void *bufs[1];
    void *cookies[1];
    soc_sbx_txrx_active_t op;
    int rv;

    op.status = SOC_E_FAIL;

    assert(*buflen >= sbx->rx_buffer_size);
    if (sbx->rx_active > 0) {
        /*
         * Note, unmutexed check of rx_active; this is actually
         * just a sanity check, as nobody should be calling sync_rx if there
         * is already another client (e.g. bcm rx) active.  If another
         * client IS active, there is some chance this check will miss it
         * (e.g. may be temporarily without buffers) but it's not
         * worth making this check terrifically robust because sync_rx
         * is only used for `light duty' (e.g. diag shell).
         * Also, of course the check is atomic, but it's not protected
         * all the way to the point where we give the buffer below.
         */
        return SOC_E_BUSY;
    }

    bufs[0] = buf;
    cookies[0] = &op;
    if ((rv = soc_sbx_txrx_give_rx_buffers(unit, 1, bufs,
                                           sync_txrx_done, cookies)))
    {
        return rv;
    }

    if (sal_sem_take(sbx->txrx_sync_sem, waitusec) != 0) {

        /* Pop the RX buff; should be the only one, or at least the front */
        soc_sbx_txrx_remove_rx_buffers (unit, 1);

        /* Clean the sem in case the RX actually completed before we popped */
        /* Again, this isn't bulletproof, but close enough */
        sal_sem_take(sbx->txrx_sync_sem, 0);

        return SOC_E_TIMEOUT;
    }

    *buflen = op.rxlen;
    return op.status;
}


void
soc_sbx_txrx_intr (int unit, uint32 unused)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    soc_sbx_txrx_active_t *rx_done_head = NULL;
    soc_sbx_txrx_active_t *rx_done_tail = NULL;
    soc_sbx_txrx_active_t *tx_done_head = NULL;
    soc_sbx_txrx_active_t *tx_done_tail = NULL;
    soc_sbx_txrx_active_t *done_head = NULL;
    soc_sbx_txrx_active_t *done_tail = NULL;
    soc_sbx_txrx_active_t *active;
    int producer, consumer;
    int rv;
    int bufs;
    int i;
    int rxlen;
    uint32 *entryp;
    uint32 entry;

    /* e.g. entry = *entryp below assumes completion is 1 word */
    assert(COMPLETION_RING_ENTRY_SIZE == sizeof(uint32));

    consumer = sbx->completion_ring_consumer;
    while ((producer =
            SAND_HAL_READ_OFFS(sbx->sbhdl, sbx->completion_ring_producer_reg)
            / COMPLETION_RING_ENTRY_SIZE)
           != consumer)
    {
        entryp = &sbx->completion_ring[consumer];
        LOG_VERBOSE(BSL_LS_SOC_TX,
                    (BSL_META_U(unit,
                                "sbx txrx unit %d completion,"
                                " prod=%d, cons=%d\n"), unit, producer,
                     consumer));
        LOG_VERBOSE(BSL_LS_SOC_TX,
                    (BSL_META_U(unit,
                                "    tx active %d, rx active %d\n"),
                     sbx->tx_active, sbx->rx_active));
        if (consumer < producer) {
            soc_cm_sinval(unit, entryp,
                          (producer - consumer) * COMPLETION_RING_ENTRY_SIZE);
        } else {
            /* Wrapped around the ring, so invalidate the 2 portions */
            soc_cm_sinval(unit, entryp,
                          (sbx->completion_ring_entries - consumer)
                          * COMPLETION_RING_ENTRY_SIZE);
            soc_cm_sinval(unit, sbx->completion_ring,
                          producer * COMPLETION_RING_ENTRY_SIZE);
        }
        while (producer != consumer) {
            entry = *entryp;
            entry = LE_TO_HOST(unit, entry);
            LOG_VERBOSE(BSL_LS_SOC_TX,
                        (BSL_META_U(unit,
                                    "sbx txrx unit %d entry %d: 0x%08x\n"),
                         unit, consumer, entry));
            rv = (entry & COMPLETION_RING_ERROR) ? SOC_E_FAIL : SOC_E_NONE;
            if (rv != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "sbx txrx unit %d: Error in completion ring: "
                                      "consumer=%d entry=0x%08x\n"),
                           unit, consumer, (int)entry));
            }

            if (entry & COMPLETION_RING_TX) {
                if (sbx->tx_active_head == NULL) {
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "sbx txrx unit=%d found tx "
                                         "completion with no active tx in "
                                         "progress; skipping.\nconsumer=%d "
                                         "entry=0x%08x\n"),
                              unit, consumer, (int)entry));

                } else {
                    /* assert(sbx->tx_active_head);*/
                    assert(sbx->tx_active_tail);
                    assert(sbx->tx_active > 0);
                    ACTIVE_DEQ(sbx->tx_active_head, sbx->tx_active_tail, active);
                    active->status = rv;
                    ACTIVE_ENQ(tx_done_head, tx_done_tail, active);
                    sbx->tx_active--;
                    assert(sbx->tx_ring_entries_active >= active->entries);
                    sbx->tx_ring_entries_active -= active->entries;
                }
            } else {
                /* Don't currently support scatter receives */
                /* so if the RX runs into multiple buffers, it is an error */
                bufs =
                    ((entry & COMPLETION_RING_BUFS_USED_MASK)
                     >> COMPLETION_RING_BUFS_USED_SHIFT) + 1;
                rxlen = (entry & COMPLETION_RING_LENGTH_MASK);

                if (bufs <= sbx->rx_active) {
                    if (bufs > 1) {
                        rv = SOC_E_INTERNAL;
                    }
                    LOG_VERBOSE(BSL_LS_SOC_TX,
                                (BSL_META_U(unit,
                                            "sbx txrx unit %d completed %d buffer rx\n"),
                                 unit, bufs));
                    for (i = 0; ((i < bufs) && (sbx->rx_active_head) &&
                                (sbx->rx_active_tail)); i++) {
                        ACTIVE_DEQ(sbx->rx_active_head, sbx->rx_active_tail,
                                   active);
                        sbx->rx_active--;
                        active->status = rv;
                        active->rxlen = rxlen;
                        ACTIVE_ENQ(rx_done_head, rx_done_tail, active);
                    }
                }
            }

            consumer = (consumer + 1) % sbx->completion_ring_entries;
            entryp = &sbx->completion_ring[consumer];
        }
    }

    SAND_HAL_WRITE_OFFS(sbx->sbhdl, sbx->completion_ring_consumer_reg,
                        consumer * COMPLETION_RING_ENTRY_SIZE);
    sbx->completion_ring_consumer = consumer;

    LOG_VERBOSE(BSL_LS_SOC_TX,
                (BSL_META_U(unit,
                            "sbx txrx unit %d completion done,"
                            " prod=%d, cons=%d\n"), unit, producer,
                 consumer));
    LOG_VERBOSE(BSL_LS_SOC_TX,
                (BSL_META_U(unit,
                            "    tx active %d, rx active %d\n"),
                 sbx->tx_active, sbx->rx_active));


    while (tx_done_head) {
        ACTIVE_DEQ(tx_done_head, tx_done_tail, active);
        ACTIVE_ENQ(done_head, done_tail, active);
        if (!tx_done_head
            || tx_done_head->donecb != done_head->donecb) {
            (done_head->donecb)(unit, done_head);
            done_tail->next = sbx->txrx_active_free;
            sbx->txrx_active_free = done_head;
            done_head = done_tail = NULL;
        }
    }

    while (rx_done_head) {
        ACTIVE_DEQ(rx_done_head, rx_done_tail, active);
        ACTIVE_ENQ(done_head, done_tail, active);
        if (!rx_done_head
            || rx_done_head->donecb != done_head->donecb) {
            (done_head->donecb)(unit, done_head);
            done_tail->next = sbx->txrx_active_free;
            sbx->txrx_active_free = done_head;
            done_head = done_tail = NULL;
        }
    }
}

#ifdef BCM_FE2000_SUPPORT
int
soc_sbx_hdr_field_set(int unit, uint8 *hdr, uint8 hdr_len,
                      SBX_rh_fields_t field, uint32 val)
{
    switch (SOC_SBX_CFG(unit)->erh_type) {
    case SOC_SBX_G2P3_ERH_SIRIUS:
        return soc_sbx_hdr_ss_field_set(unit, hdr, hdr_len, field, val);
        break;
    case SOC_SBX_G2P3_ERH_QESS:
        return soc_sbx_hdr_qess_field_set(unit, hdr, hdr_len, field, val);
        break;
    case SOC_SBX_G2P3_ERH_DEFAULT:
        return soc_sbx_hdr_default_field_set(unit, hdr, hdr_len, field, val);
        break;
    default:
        return SOC_E_PARAM;
    }
}

static int
soc_sbx_hdr_default_field_set(int unit, uint8 *hdr, uint8 hdr_len,
                      SBX_rh_fields_t field, uint32 val)
{
    if (hdr_len < SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES) {
        return SOC_E_PARAM;
    }

    switch(field)
    {
    case SBX_rhf_queue_id:
        SB_ZF_G2FEEGRROUTEHEADER_SET_QID(val, hdr,
                              SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_sdp:
        SB_ZF_G2FEEGRROUTEHEADER_SET_FDP(val, hdr,
                              SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_ecn:
        SB_ZF_G2FEEGRROUTEHEADER_SET_E(val, hdr,
                              SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_test:
        SB_ZF_G2FEEGRROUTEHEADER_SET_TEST(val, hdr,
                              SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_length:
        SB_ZF_G2FEEGRROUTEHEADER_SET_FRMLEN(val, hdr,
                              SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_length_adjust:
        SB_ZF_G2FEEGRROUTEHEADER_SET_LENADJINDEX(val, hdr,
                              SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_outunion:
        SB_ZF_G2FEEGRROUTEHEADER_SET_OUTUNION(val, hdr,
                              SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_mc:
        SB_ZF_G2FEEGRROUTEHEADER_SET_MC(val, hdr,
                              SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_sid:
        SB_ZF_G2FEEGRROUTEHEADER_SET_SID(val, hdr,
                              SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_ttl:
        SB_ZF_G2FEEGRROUTEHEADER_SET_TTL(val, hdr,
                              SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
        /*
    case SBX_rhf_fcos:
        SB_ZF_G2FEEGRROUTEHEADER_SET_FCOS(val, hdr,
                              SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
        */
    case SBX_rhf_fdp:
        SB_ZF_G2FEEGRROUTEHEADER_SET_FDP(val, hdr,
                              SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
        /*
    case SBX_rhf_fdp2:
        SB_ZF_G2FEEGRROUTEHEADER_SET_FDP2(val, hdr,
                              SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
        */
    case SBX_rhf_rcos:
        SB_ZF_G2FEEGRROUTEHEADER_SET_RCOS(val, hdr,
                              SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_rdp:
        SB_ZF_G2FEEGRROUTEHEADER_SET_RDP(val, hdr,
                              SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
        /*
    case SBX_rhf_ppeswop:
        SB_ZF_G2FEEGRROUTEHEADER_SET_PPESWOP(val, hdr,
                              SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
        */
    default:
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

static int
soc_sbx_hdr_ss_field_set(int unit, uint8 *hdr, uint8 hdr_len,
                      SBX_rh_fields_t field, uint32 val)
{
    if (hdr_len < SB_ZF_G2_SS_FEINGROUTEHEADER_SIZE_IN_BYTES) {
        return SOC_E_PARAM;
    }

    switch(field)
    {
    case SBX_rhf_ksop:
        SB_ZF_G2SSFEINGROUTEHEADER_SET_KSOP(val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_ect:
        SB_ZF_G2SSFEINGROUTEHEADER_SET_ECT(val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_ecn:
        SB_ZF_G2SSFEINGROUTEHEADER_SET_ECN(val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_test:
        SB_ZF_G2SSFEINGROUTEHEADER_SET_TEST(val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_mc:
        SB_ZF_G2SSFEINGROUTEHEADER_SET_MC(val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_length_adjust:
        SB_ZF_G2SSFEINGROUTEHEADER_SET_LENADJ(val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_queue_id:
        SB_ZF_G2SSFEINGROUTEHEADER_SET_QID(val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_outunion:
        SB_ZF_G2SSFEINGROUTEHEADER_SET_OUTUNION(val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_sid:
        SB_ZF_G2SSFEINGROUTEHEADER_SET_SID(val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_sdp:
        SB_ZF_G2SSFEINGROUTEHEADER_SET_FDP(val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_fcos2:
        SB_ZF_G2SSFEINGROUTEHEADER_SET_FCOS2(val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_lbid:
        SB_ZF_G2SSFEINGROUTEHEADER_SET_LBID(val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_rcos:
        SB_ZF_G2SSFEINGROUTEHEADER_SET_RCOS(val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_rdp:
        SB_ZF_G2SSFEINGROUTEHEADER_SET_RDP(val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_s:
        SB_ZF_G2SSFEINGROUTEHEADER_SET_S(val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_ttl:
        SB_ZF_G2SSFEINGROUTEHEADER_SET_TTL(val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    default:
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

static int
soc_sbx_hdr_qess_field_set(int unit, uint8 *hdr, uint8 hdr_len,
                      SBX_rh_fields_t field, uint32 val)
{
    if (hdr_len < SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES) {
        return SOC_E_PARAM;
    }

    switch(field)
    {
    case SBX_rhf_queue_id:
        SB_ZF_G2QESSFEINGROUTEHEADER_SET_QID(val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_sdp:
        SB_ZF_G2QESSFEINGROUTEHEADER_SET_FDP(val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_ecn:
        SB_ZF_G2QESSFEINGROUTEHEADER_SET_ECN(val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_test:
        SB_ZF_G2QESSFEINGROUTEHEADER_SET_TEST(val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_length:
        SB_ZF_G2QESSFEINGROUTEHEADER_SET_FRMLEN(val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_length_adjust:
        SB_ZF_G2QESSFEINGROUTEHEADER_SET_LENADJ(val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_mc:
        SB_ZF_G2QESSFEINGROUTEHEADER_SET_MC(val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_ect:
        SB_ZF_G2QESSFEINGROUTEHEADER_SET_ECT(val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_outunion:
        SB_ZF_G2QESSFEINGROUTEHEADER_SET_OUTUNION(val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_sid:
        SB_ZF_G2QESSFEINGROUTEHEADER_SET_SID(val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_fcos2:
        SB_ZF_G2QESSFEINGROUTEHEADER_SET_FCOS2(val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_lbid:
        SB_ZF_G2QESSFEINGROUTEHEADER_SET_LBID(val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_rcos:
        SB_ZF_G2QESSFEINGROUTEHEADER_SET_RCOS(val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_rdp:
        SB_ZF_G2QESSFEINGROUTEHEADER_SET_RDP(val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_s:
        SB_ZF_G2QESSFEINGROUTEHEADER_SET_S(val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_ttl:
        SB_ZF_G2QESSFEINGROUTEHEADER_SET_TTL(val, hdr,
                              SB_ZF_G2_SS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    default:
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

int
soc_sbx_hdr_field_get(int unit, uint8 *hdr, uint8 hdr_len,
                      SBX_rh_fields_t field, uint32 *val)
{
    switch (SOC_SBX_CFG(unit)->erh_type) {
    case SOC_SBX_G2P3_ERH_SIRIUS:
        return soc_sbx_hdr_ss_field_get(unit, hdr, hdr_len, field, val);
        break;
    case SOC_SBX_G2P3_ERH_QESS:
        return soc_sbx_hdr_qess_field_get(unit, hdr, hdr_len, field, val);
        break;
    case SOC_SBX_G2P3_ERH_DEFAULT:
        return soc_sbx_hdr_default_field_get(unit, hdr, hdr_len, field, val);
        break;
    default:
        return SOC_E_PARAM;
    }
}
static int
soc_sbx_hdr_default_field_get(int unit, uint8 *hdr, uint8 hdr_len,
                      SBX_rh_fields_t field, uint32 *val)
{
    if (hdr_len < SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES) {
        return SOC_E_PARAM;
    }

    switch(field)
    {
    case SBX_rhf_queue_id:
        SB_ZF_G2FEEGRROUTEHEADER_GET_QID(*val, hdr,
                                         SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_sdp:
        SB_ZF_G2FEEGRROUTEHEADER_GET_FDP(*val, hdr,
                                         SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_ecn:
        SB_ZF_G2FEEGRROUTEHEADER_GET_E(*val, hdr,
                                       SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_test:
        SB_ZF_G2FEEGRROUTEHEADER_GET_TEST(*val, hdr,
                                          SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_length:
        SB_ZF_G2FEEGRROUTEHEADER_GET_FRMLEN(*val, hdr,
                                            SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_length_adjust:
        SB_ZF_G2FEEGRROUTEHEADER_GET_LENADJINDEX(*val, hdr,
                                                 SB_ZF_G2_FE_EGRROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    default:
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}
static int
soc_sbx_hdr_ss_field_get(int unit, uint8 *hdr, uint8 hdr_len,
                      SBX_rh_fields_t field, uint32 *val)
{
    if (hdr_len < SB_ZF_G2_SS_FEINGROUTEHEADER_SIZE_IN_BYTES) {
        return SOC_E_PARAM;
    }

    switch(field)
    {
    case SBX_rhf_ksop:
        SB_ZF_G2SSFEINGROUTEHEADER_GET_KSOP(*val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_ect:
        SB_ZF_G2SSFEINGROUTEHEADER_GET_ECT(*val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_ecn:
        SB_ZF_G2SSFEINGROUTEHEADER_GET_ECN(*val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_test:
        SB_ZF_G2SSFEINGROUTEHEADER_GET_TEST(*val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_mc:
        SB_ZF_G2SSFEINGROUTEHEADER_GET_MC(*val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_length_adjust:
        SB_ZF_G2SSFEINGROUTEHEADER_GET_LENADJ(*val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_queue_id:
        SB_ZF_G2SSFEINGROUTEHEADER_GET_QID(*val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_outunion:
        SB_ZF_G2SSFEINGROUTEHEADER_GET_OUTUNION(*val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_sid:
        SB_ZF_G2SSFEINGROUTEHEADER_GET_SID(*val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_sdp:
        SB_ZF_G2SSFEINGROUTEHEADER_GET_FDP(*val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_fcos2:
        SB_ZF_G2SSFEINGROUTEHEADER_GET_FCOS2(*val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_lbid:
        SB_ZF_G2SSFEINGROUTEHEADER_GET_LBID(*val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_rcos:
        SB_ZF_G2SSFEINGROUTEHEADER_GET_RCOS(*val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_rdp:
        SB_ZF_G2SSFEINGROUTEHEADER_GET_RDP(*val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_s:
        SB_ZF_G2SSFEINGROUTEHEADER_GET_S(*val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_ttl:
        SB_ZF_G2SSFEINGROUTEHEADER_GET_TTL(*val, hdr,
                              SB_ZF_G2_SS_FE_INGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    default:
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}
static int
soc_sbx_hdr_qess_field_get(int unit, uint8 *hdr, uint8 hdr_len,
                      SBX_rh_fields_t field, uint32 *val)
{
    if (hdr_len < SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES) {
        return SOC_E_PARAM;
    }

    switch(field)
    {
    case SBX_rhf_queue_id:
        SB_ZF_G2QESSFEINGROUTEHEADER_GET_QID(*val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_sdp:
        SB_ZF_G2QESSFEINGROUTEHEADER_GET_FDP(*val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_ecn:
        SB_ZF_G2QESSFEINGROUTEHEADER_GET_ECN(*val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_test:
        SB_ZF_G2QESSFEINGROUTEHEADER_GET_TEST(*val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_length:
        SB_ZF_G2QESSFEINGROUTEHEADER_GET_FRMLEN(*val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_length_adjust:
        SB_ZF_G2QESSFEINGROUTEHEADER_GET_LENADJ(*val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_mc:
        SB_ZF_G2QESSFEINGROUTEHEADER_GET_MC(*val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_ect:
        SB_ZF_G2QESSFEINGROUTEHEADER_GET_ECT(*val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_outunion:
        SB_ZF_G2QESSFEINGROUTEHEADER_GET_OUTUNION(*val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_sid:
        SB_ZF_G2QESSFEINGROUTEHEADER_GET_SID(*val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_fcos2:
        SB_ZF_G2QESSFEINGROUTEHEADER_GET_FCOS2(*val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_lbid:
        SB_ZF_G2QESSFEINGROUTEHEADER_GET_LBID(*val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_rcos:
        SB_ZF_G2QESSFEINGROUTEHEADER_GET_RCOS(*val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_rdp:
        SB_ZF_G2QESSFEINGROUTEHEADER_GET_RDP(*val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_s:
        SB_ZF_G2QESSFEINGROUTEHEADER_GET_S(*val, hdr,
                              SB_ZF_G2_QESS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    case SBX_rhf_ttl:
        SB_ZF_G2QESSFEINGROUTEHEADER_GET_TTL(*val, hdr,
                              SB_ZF_G2_SS_FEINGROUTEHEADER_SIZE_IN_BYTES - 1);
        break;
    default:
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}
#endif /* BCM_FE2000_SUPPORT */
