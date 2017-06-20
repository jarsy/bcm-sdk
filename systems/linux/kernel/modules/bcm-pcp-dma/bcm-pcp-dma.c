/*
 * $Id: bcm-pcp-dma.c,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>
#include <asm/uaccess.h>    /* for get_user and put_user */
#include <asm/io.h>		      /* memory access */
#include <linux/skbuff.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <asm/pci.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/version.h>

/* Unified BCM - PCP parameters */
#define BCM_PCP_DMA_KM_CHDEV_NAME "bcm-pcp-dma"
#define BCM_PCP_DMA_KM_CHDEV_FULL_FILE_NAME "/dev/bcm-pcp-dma"
  
#define BCM_PCP_DMA_KM_CHDEV_NUM 234

#define BCM_OS_PCIE_LC0_BASE_ADDR 0x0
#define BCM_OS_PCIE_LC1_BASE_ADDR 0x0 /* not supported in PCP */
#define BCM_OS_PCIE_LC2_BASE_ADDR 0x0 /* not supported in PCP */
#define BCM_OS_PCIE_LC3_BASE_ADDR 0x0 /* not supported in PCP */

#define PCP_PCIE_BUS_NUM            0x2 
#define PCP_BDE_UNIT_NUM            0x0 

#define BCM_PCP_DMA_KM_BYTE_SWAP(x) ((((x) << 24)) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | (((x) >> 24)))

/* to compile with debug-related code, set to 1 */
#ifndef BCM_PCP_DMA_KM_DEBUG
#define BCM_PCP_DMA_KM_DEBUG 1
#endif

/* definitions of BMC PCP DMA KM environment */
#define BCM_KM_U32 uint32_t
#define KM_PRINTF_FORMAT_U32 "u"
#define __ATTRIBUTE_PACKED__  __attribute__ ((packed))

#define KM_WORKQUEUE_FN_PARAM struct work_struct

/* bottom half handler for pcp_dma EOP interrupt */
typedef void  (*bcm_pcp_dma_km_isr_rx_bh_fn)(void* arg);

/* global variable set outside the module. 1 if rx interrupt is supported, 0 otherwise */
extern int bcm_pcp_dma_km_isr_rx_support;

/* if enable = 1, enable interrupts for line card line; if enable = 0, disable interrupts */
extern void bcm_km_isr_enable_line(const BCM_KM_U32 line, int enable);

/* set pcp_dma rx interrupt handler working function */
extern void bcm_km_isr_set_pcp_dma_rx_bh_fn(bcm_pcp_dma_km_isr_rx_bh_fn fn);

/* if enable = 1, enable interrupts for edm (rx cahin); if enable = 0, disable interrupts */
extern void bcm_pcp_dma_km_isr_edm_interrupt_enable(BCM_KM_U32 line, int enable);

/* Get edm cause class - get the class of the recived interrupt */
extern void dk_isr_plcf_edm_cause_classes_get(BCM_KM_U32 line, BCM_KM_U32 *edm_interrupt);

/* High level function - recive RX packet */
extern int bcm_pcp_dma_km_dp_on_rx(BCM_KM_U32 line, BCM_KM_U32 class, struct sk_buff* skb);

/* trace level 
 * set to 
 * 0 to disable trace
 * 1 to enable error traces
 * 6 to enable above + reg write
 * 7 to enable above + recurrent traces (pkt in/out)
 * 8 to enable all traces
 */
int bcm_pcp_dma_km_trc_lvl = 1;
module_param_named(trace_lvl, bcm_pcp_dma_km_trc_lvl, int, S_IRUGO | S_IWUSR);
int bcm_pcp_dma_km_rx_trc_lvl = 1;
module_param_named(rx_trace_lvl, bcm_pcp_dma_km_rx_trc_lvl, int, S_IRUGO | S_IWUSR);
int bcm_pcp_dma_km_tx_trc_lvl = 1;
module_param_named(tx_trace_lvl, bcm_pcp_dma_km_tx_trc_lvl, int, S_IRUGO | S_IWUSR);

/* trace macro */
#define BCM_PCP_DMA_KM_RX_TRACE(lvl, format, ...) \
  do { \
    if(bcm_pcp_dma_km_rx_trc_lvl > lvl) printk(KERN_DEBUG format"\n", ##__VA_ARGS__); \
  } while(0)  

/* trace macro */
#define BCM_PCP_DMA_KM_TX_TRACE(lvl, format, ...) \
  do { \
    if(bcm_pcp_dma_km_tx_trc_lvl > lvl) printk(KERN_DEBUG format"\n", ##__VA_ARGS__); \
  } while(0)  

/* pkt dump macro */
#define BCM_PCP_DMA_KM_PKT_TRC(lvl, txt, buf, len) \
  do { \
    int i; \
    printk("%s : dump 0x%p: ", txt, buf);\
    for(i = 0; i < len; i++) { \
      if(i % 4 == 0) { \
        printk("\n %6.6u ",i);\
      } \
      printk("%2.2x ", *((unsigned char*)buf + i));\
    }\
    printk("\n");\
  } while(0)

#define BCM_PCP_DMA_KM_TX_PKT_TRC(lvl, txt, buf, len) \
  if(bcm_pcp_dma_km_tx_trc_lvl > lvl) BCM_PCP_DMA_KM_PKT_TRC(lvl, txt, buf, len)

#define BCM_PCP_DMA_KM_RX_PKT_TRC(lvl, txt, buf, len) \
  if(bcm_pcp_dma_km_rx_trc_lvl > lvl) BCM_PCP_DMA_KM_PKT_TRC(lvl, txt, buf, len)

/* trace macro */
#define BCM_PCP_DMA_KM_TRACE(lvl, format, ...) \
  do { \
    if(bcm_pcp_dma_km_trc_lvl > lvl) printk(KERN_DEBUG "%s(): "format"\n", FUNCTION_NAME(), ##__VA_ARGS__); \
  } while(0)  

/* Isr module is needed */
#define BCM_PCP_DMA_KM_ENABLE_INTERRUPT 0

/* High receving level is needed */
#define BCM_PCP_DMA_KM_FRWRD_PKT_UP_LEVEL 0

#define BCM_KNET_SUPPORT 1
#ifdef BCM_KNET_SUPPORT
/* redefine the higher level forwarding to Knet */
#ifdef BCM_PCP_DMA_KM_FRWRD_PKT_UP_LEVEL
#undef BCM_PCP_DMA_KM_FRWRD_PKT_UP_LEVEL
#define BCM_PCP_DMA_KM_FRWRD_PKT_UP_LEVEL 1
#endif
int bcm_knet_pkt_rx(BCM_KM_U32 line, BCM_KM_U32 class, struct sk_buff* skb);
#endif /* BCM_KNET_SUPPORT */

int lkbde_mem_write(int d, BCM_KM_U32 addr, BCM_KM_U32 *buf);
int lkbde_mem_read(int d, BCM_KM_U32 addr, BCM_KM_U32 *buf);

/********************  PORTING REQUIRED ***************************/
/* number of line cards in a system */
#define PCP_DMA_NUM_LINE_CARDS 1

/* maximal number of petra devices on a line card */
#define PCP_DMA_NUM_PETRAS_ON_LINE_CARD  1

#define PCP_DMA_MAX_PKT_SIZE (10240)

#define PCP_DMA_MAX_BURST (4096)

/* TX direction (from CPU to Petra): chain of TX_NUM_BUFS_PER_PETRA buffers 
 * of TX_BUF_SIZE per Petra */
static long pcp_dma_tx_num_bufs_per_petra = 10;
module_param_named(tx_chain_len, pcp_dma_tx_num_bufs_per_petra, long, S_IRUGO | S_IWUSR);
static const BCM_KM_U32 PCP_DMA_TX_BUF_SIZE= PCP_DMA_MAX_PKT_SIZE;

/* RX direction (to CPU from Petra): chain of RX_NUM_BUFS_PER_CLASS buffers of RX_BUF_SIZE per class */
#define PCP_DMA_RX_NUM_CLASSES 7

#if 0 /* interrupt per end of packet. This also affects KM_ISR_RX_CLASS_MASK_ALL in bcm_km_isr_main.c */
static const BCM_KM_U32 pcp_dma_edm_class_mask[] =
  { (((BCM_KM_U32)1) << 24), (((BCM_KM_U32)1) << 25), (((BCM_KM_U32)1) << 26), (((BCM_KM_U32)1) << 27), (((BCM_KM_U32)1) << 28), (((BCM_KM_U32)1) << 29), (((BCM_KM_U32)1) << 30) };
#else /* interrupt per end of buffer */
static const BCM_KM_U32 pcp_dma_edm_class_mask[] =
  { (((BCM_KM_U32)1) << 16), (((BCM_KM_U32)1) << 17), (((BCM_KM_U32)1) << 18), (((BCM_KM_U32)1) << 19), (((BCM_KM_U32)1) << 20), (((BCM_KM_U32)1) << 21), (((BCM_KM_U32)1) << 22) };
#endif

static long pcp_dma_rx_num_bufs_per_class = 10;
module_param_named(rx_chain_len, pcp_dma_rx_num_bufs_per_class, long, S_IRUGO | S_IWUSR);
static long pcp_dma_rx_buf_size= PCP_DMA_MAX_PKT_SIZE;
module_param_named(rx_buf_size, pcp_dma_rx_buf_size, long, S_IRUGO | S_IWUSR);

static long pcp_dma_rx_task_budget = 1;
module_param_named(rx_task_budget, pcp_dma_rx_task_budget, long, S_IRUGO | S_IWUSR);

#if BCM_PCP_DMA_KM_ENABLE_INTERRUPT
int bcm_pcp_dma_km_pkt_interrupt_support =  1;
#else
int bcm_pcp_dma_km_pkt_interrupt_support =  0;
#endif
module_param_named(interrupt_support, bcm_pcp_dma_km_pkt_interrupt_support, int, S_IRUGO | S_IWUSR);

/* Petra offsets within PCP_DMA */
#if PCP_DMA_NUM_PETRAS_ON_LINE_CARD == 1
static const BCM_KM_U32 pcp_dma_pipe_offset[PCP_DMA_NUM_PETRAS_ON_LINE_CARD] = { 0x00100000 };
#endif
#if PCP_DMA_NUM_PETRAS_ON_LINE_CARD == 4
static const BCM_KM_U32 pcp_dma_pipe_offset[PCP_DMA_NUM_PETRAS_ON_LINE_CARD] = { 0x00100000, 0x00200000, 0x00300000, 0x00400000 };
#endif

/* Line card FPGA base addresses */
#if PCP_DMA_NUM_PETRAS_ON_LINE_CARD == 1
static const void* pcp_dma_offset[PCP_DMA_NUM_LINE_CARDS] =
{ (void*)BCM_OS_PCIE_LC0_BASE_ADDR };
#endif
#if PCP_DMA_NUM_PETRAS_ON_LINE_CARD == 4
static const void* pcp_dma_offset[PCP_DMA_NUM_LINE_CARDS] =
{ (void*)BCM_OS_PCIE_LC0_BASE_ADDR, (void*)BCM_OS_PCIE_LC1_BASE_ADDR, (void*)BCM_OS_PCIE_LC2_BASE_ADDR, (void*)BCM_OS_PCIE_LC3_BASE_ADDR };
#endif

/* polling timeout of rx thread in jiffies */
static int pcp_dma_rx_working_fn_sleep_jiffies = (10 * HZ) / 1000;
module_param_named(pcp_dma_rx_working_fn_sleep_jiffies, pcp_dma_rx_working_fn_sleep_jiffies, int, S_IRUGO | S_IWUSR);

#if BCM_PCP_DMA_KM_FRWRD_PKT_UP_LEVEL
/* forward recvd packet to upper layers
 * NOTE: packet contains TM and PP header, and does not contain CRC
 */
static int pcp_dma_on_rx(BCM_KM_U32 line, BCM_KM_U32 class, struct sk_buff* skb)
{
#ifdef BCM_KNET_SUPPORT
    return bcm_knet_pkt_rx(line, class, skb);
#else
    return bcm_pcp_dma_km_dp_on_rx(line, class, skb);
#endif /* BCM_KNET_SUPPORT */
}
#endif

/* write pcp_dma register
 * return 0 on success, -1 on error */
static inline int pcp_dma_write_reg(void* addr, BCM_KM_U32 data)
{
  int ret;
  BCM_KM_U32 data_swap = 0x0;

  data_swap = BCM_PCP_DMA_KM_BYTE_SWAP(data);
  BCM_PCP_DMA_KM_TRACE(5, "reg write addr %p data %8.8x, data_swap %8.8x", addr, data, data_swap);

  ret = lkbde_mem_write(PCP_BDE_UNIT_NUM,(BCM_KM_U32)addr, &data_swap);
  return ret;  
}

/* read pcp_dma register
 * return 0 on success, -1 on error */
static inline int pcp_dma_read_reg(void* addr, BCM_KM_U32* data)
{
  int ret;
  BCM_KM_U32 data_swap = 0x0;

  ret = lkbde_mem_read(PCP_BDE_UNIT_NUM, (BCM_KM_U32)addr, data);
  data_swap = BCM_PCP_DMA_KM_BYTE_SWAP(*data);

  BCM_PCP_DMA_KM_TRACE(5, "reg read addr %p data %8.8x, data_swap %8.8x", addr, *data, data_swap);
  return ret;  
}

/* Exposed APIs */

/* init module */
int bcm_pcp_dma_km_dp_via_dma_module_init(void);

/* deinit module */
int bcm_pcp_dma_km_dp_via_dma_module_deinit(void);

/* send pkt to the specified pipe (petra device) within a line card */
int bcm_pcp_dma_km_dp_via_dma_tx_raw(BCM_KM_U32 line, BCM_KM_U32 pipe, struct sk_buff* skb);

/* init/deinit DMA chains for line card line.
 *  deinit = 0 for init,
 *  deinit = 1 for deinit
 */
int bcm_pcp_dma_km_dp_via_dma_line_init(int line, int deinit);


/* statistics sysfs variables */
static int pkts_out = 0;
static int pkts_out_discarded = 0;
static int bufs_in = 0;
static int pkts_in = 0;
static int pkts_in_discarded = 0;
static int pkts_in_error = 0;
static int interrupts_rx = 0;
module_param_named(pkts_out, pkts_out, int, S_IRUGO | S_IWUSR);
module_param_named(pkts_out_discarded, pkts_out_discarded, int, S_IRUGO | S_IWUSR);
module_param_named(pkts_in, pkts_in, int, S_IRUGO | S_IWUSR);
module_param_named(bufs_in, bufs_in, int, S_IRUGO | S_IWUSR);
module_param_named(pkts_in_discarded, pkts_in_discarded, int, S_IRUGO | S_IWUSR);
module_param_named(pkts_in_error, pkts_in_error, int, S_IRUGO | S_IWUSR);
module_param_named(interrupts_rx, interrupts_rx, int, S_IRUGO | S_IWUSR);

int buff_alloc_alignment = -1;
module_param_named(buff_alloc_alignment, buff_alloc_alignment, int, S_IRUGO | S_IWUSR);

int strict_prio = 1;
module_param_named(strict_prio, strict_prio, int, S_IRUGO | S_IWUSR);

int rx_delay_iter = 0;
module_param_named(rx_delay_iter, rx_delay_iter, int, S_IRUGO | S_IWUSR);

int tx_recycle_retry = 10000;
module_param_named(tx_recycle_retry, tx_recycle_retry, int, S_IRUGO | S_IWUSR);

#if BCM_PCP_DMA_KM_DEBUG

int dbg1 = 0;
module_param_named(dbg1, dbg1, int, S_IRUGO | S_IWUSR);
int dbg2 = 0;
module_param_named(dbg2, dbg2, int, S_IRUGO | S_IWUSR);
int dbg3 = 0;
module_param_named(dbg3, dbg3, int, S_IRUGO | S_IWUSR);
int dbg4 = 100;
module_param_named(dbg4, dbg4, int, S_IRUGO | S_IWUSR);


static int discard_rx = 0;
module_param_named(discard_rx, discard_rx, int, S_IRUGO | S_IWUSR);

static int echo_rx = 0;
module_param_named(echo_rx, echo_rx, int, S_IRUGO | S_IWUSR);

static int echo_rx_snake = 0;
module_param_named(echo_rx_snake, echo_rx_snake, int, S_IRUGO | S_IWUSR);

static int stop_pcp_dma_on_corrupted_rx_buf = 0;
module_param_named(stop_pcp_dma_on_corrupted_rx_buf, stop_pcp_dma_on_corrupted_rx_buf, int, S_IRUGO | S_IWUSR);

static int pcp_dma_copy_buf_on_echo_rx = 0;
module_param_named(copy_buf_on_echo_rx, pcp_dma_copy_buf_on_echo_rx, int, S_IRUGO | S_IWUSR);

#endif

/* PCP_DMA registers */
static const BCM_KM_U32 IDM_RST                = 0x90470;
static const BCM_KM_U32 IDM_RST_BIT_RST        = (1 << 0);
static const BCM_KM_U32 IDM_RST_BIT_ENABLE     = (1 << 1);
static const BCM_KM_U32 IDM_TRIG = 0x90474;
#define IDM_TRIG_BIT_PETRA0 (1 << 0)
#define IDM_TRIG_BIT_PETRA1 (1 << 4)                    /* not supported in PCP */
#define IDM_TRIG_BIT_PETRA2 (1 << 8)                    /* not supported in PCP */
#define IDM_TRIG_BIT_PETRA3 (1 << 12)                   /* not supported in PCP */
static const BCM_KM_U32 IDM_DESC_HDR_MSB_PETRA0 = 0x90488;
static const BCM_KM_U32 IDM_DESC_HDR_LSB_PETRA0 = 0x90484;
static const BCM_KM_U32 IDM_DESC_HDR_MSB_PETRA1 = 0x0a1c;   /* not supported in PCP */
static const BCM_KM_U32 IDM_DESC_HDR_LSB_PETRA1 = 0x0a20;   /* not supported in PCP */
static const BCM_KM_U32 IDM_DESC_HDR_MSB_PETRA2 = 0x0a24;   /* not supported in PCP */
static const BCM_KM_U32 IDM_DESC_HDR_LSB_PETRA2 = 0x0a28;   /* not supported in PCP */
static const BCM_KM_U32 IDM_DESC_HDR_MSB_PETRA3 = 0x0a2c;   /* not supported in PCP */
static const BCM_KM_U32 IDM_DESC_HDR_LSB_PETRA3 = 0x0a30;   /* not supported in PCP */
static const BCM_KM_U32 ISS_RST                 = 0x9049c;
static const BCM_KM_U32 ISS_RST_BIT_RST         = (1 << 0);
static const BCM_KM_U32 ISS_RST_BIT_ENABLE      = (1 << 1);
static const BCM_KM_U32 EDM_RST                 = 0x904b0;
static const BCM_KM_U32 EDM_RST_BIT_RST         = (1 << 0);
static const BCM_KM_U32 EDM_RST_BIT_ENABLE      = (1 << 1);

static const BCM_KM_U32 EDM_DESC_HEAD_MSB_CLASS1 = 0x904c0;
static const BCM_KM_U32 EDM_DESC_HEAD_LSB_CLASS1 = 0x904c4;
static const BCM_KM_U32 EDM_DESC_HEAD_MSB_CLASS2 = 0x904c8;
static const BCM_KM_U32 EDM_DESC_HEAD_LSB_CLASS2 = 0x904cc;
static const BCM_KM_U32 EDM_DESC_HEAD_MSB_CLASS3 = 0x904d0;
static const BCM_KM_U32 EDM_DESC_HEAD_LSB_CLASS3 = 0x904d4;
static const BCM_KM_U32 EDM_DESC_HEAD_MSB_CLASS4 = 0x904d8;
static const BCM_KM_U32 EDM_DESC_HEAD_LSB_CLASS4 = 0x904dc;
static const BCM_KM_U32 EDM_DESC_HEAD_MSB_CLASS5 = 0x904e0;
static const BCM_KM_U32 EDM_DESC_HEAD_LSB_CLASS5 = 0x904e4;
static const BCM_KM_U32 EDM_DESC_HEAD_MSB_CLASS6 = 0x904e8;
static const BCM_KM_U32 EDM_DESC_HEAD_LSB_CLASS6 = 0x904ec;
static const BCM_KM_U32 EDM_DESC_HEAD_MSB_CLASS7 = 0x904f0;
static const BCM_KM_U32 EDM_DESC_HEAD_LSB_CLASS7 = 0x904f4;

static const BCM_KM_U32 ESS_RST = 0x90538;
static const BCM_KM_U32 ESS_RST_BIT_RST = (1 << 0);
static const BCM_KM_U32 ESS_RST_BIT_ENABLE = (1 << 1);

/* PCP_DMA buffer descriptor is 160 bits::
 * owner - 1 bit , sof - 1 bit, eof - 1 bit, byte_count - 14 bits, petra - 6 bits, error - 4 bits, rfu (reserved) - 5 bits
 * buffer address - 64 bits
 * next descriptor - 64 bits
 */
static const BCM_KM_U32 PCP_DMA_DESC_OWNER_CPU = 0;
static const BCM_KM_U32 PCP_DMA_DESC_OWNER_PCP_DMA = 1;

struct pcp_dma_regs {
  void* iss_rst;
  void* ess_rst;
  void* idm_rst;
  void* idm_petra_trig;
  void* idm_petra_head_desc_lsb[PCP_DMA_NUM_PETRAS_ON_LINE_CARD];
  void* edm_rst;
  void* edm_petra_trig;
  void* edm_petra_head_desc_lsb[PCP_DMA_RX_NUM_CLASSES];
};


struct pcp_dma_desc {
  BCM_KM_U32 hdr;
  BCM_KM_U32 buf_addr_upper;
  BCM_KM_U32 buf_addr_lower;
  BCM_KM_U32 next_desc_addr_upper;
  BCM_KM_U32 next_desc_addr_lower;   
  BCM_KM_U32 pad1;
  BCM_KM_U32 pad2;
  BCM_KM_U32 pad3;
};

struct pcp_dma_buf {
  /* skbuff associated with this buffer */
  struct sk_buff *skb;
  dma_addr_t dma_buf_phys;
};

struct pcp_dma_tx_ring {
  BCM_KM_U32 line;
  BCM_KM_U32 pipe;
  
  /* pointer to the descriptors ring */
  struct pcp_dma_desc *p_desc;
  
  /* physical address of the descriptor ring */
  dma_addr_t desc_phys_base;
  
  /* size of the descriptor ring */
  BCM_KM_U32 size;
  
  /* number of descriptors */
  BCM_KM_U32 desc_count;
  
  /* number of buffers */
  BCM_KM_U32 buf_count;
  
  /* buffer size */
  BCM_KM_U32 buf_size;
  
  /* number of descriptors in use by PCP_DMA*/
  BCM_KM_U32 used_desc_count;

  /* next free descriptor */
  BCM_KM_U32 next_free_desc;
  
  /* next descriptor to clean */
  BCM_KM_U32 next_used_desc;
  
  /* next descriptor to be handled by PCP_DMA simulation */
  BCM_KM_U32 sim_next_desc;
  
  /* array of buffers */
  struct pcp_dma_buf *p_buf;
  
  struct pci_dev *p_pci_dev;
};

struct pcp_dma_rx_ring {
  
  BCM_KM_U32 line;
  BCM_KM_U32 class;
  
  /* pointer to the descriptors ring */
  struct pcp_dma_desc *p_desc;
  
  /* physical address of the descriptor ring */
  dma_addr_t desc_phys_base;
  
  /* size of the descriptor ring */
  BCM_KM_U32 size;
  
  /* number of descriptors */
  BCM_KM_U32 desc_count;
  
  /* number of buffers */
  BCM_KM_U32 buf_count;
  
  /* buffer size */
  BCM_KM_U32 buf_size;
  
  /* next descriptor to clean */
  BCM_KM_U32 next_used_desc;
  
  /* next descriptor to be handled by PCP_DMA simulation */
  BCM_KM_U32 sim_next_desc;
  
  /* array of buffers */
  struct pcp_dma_buf *p_buf;
  
  /* skb for pending packet (packet is on multi-buffers, and only part of it has arrived */
  struct sk_buff *pend_skb;
  
  struct pci_dev *p_pci_dev;
};

struct pcp_dma_dev {
  struct pcp_dma_rx_ring* p_rx_rings;
  struct pcp_dma_tx_ring* p_tx_rings;
  BCM_KM_U32 num_lines;
  BCM_KM_U32 num_classes;
  BCM_KM_U32 num_pipes_per_line; 
  /* do_continue is set to 0 during module deinit to signal scheduled works to finish
   * execution 
   */
  BCM_KM_U32 do_continue;
  /* semaphore used for waiting for scheduled works to finish during module deinit */
  struct semaphore  sem;
  
  struct pcp_dma_regs regs[PCP_DMA_NUM_LINE_CARDS];

  BCM_KM_U32 pcp_dma_line_ready[PCP_DMA_NUM_LINE_CARDS];
};

/* delayed work : polling rx ring to get recieved packets, which were put
 * in the ring by PCP_DMA */
static struct delayed_work rx_work;

static struct pcp_dma_dev dev;

/* edm_interrupt value includes classes which should be handled in pcp_dma_rx_line_class_working_fn */
static BCM_KM_U32 remain_edm_interrupt[PCP_DMA_NUM_LINE_CARDS];

/* next class to handle, per line card. Used for round-robin */
static BCM_KM_U32 next_class[PCP_DMA_NUM_LINE_CARDS];

#if BCM_PCP_DMA_KM_DEBUG

int rx_ring_0_curr_desc_ndx_ptr = 0;
module_param_named(rx_ring_0_curr_desc_ndx_ptr, rx_ring_0_curr_desc_ndx_ptr, int, S_IRUGO | S_IWUSR);

#endif

int bcm_pcp_dma_km_dp_via_dma_tx_raw(BCM_KM_U32 line, BCM_KM_U32 pipe, struct sk_buff* skb);

static void pcp_dma_recycle_tx_ring(struct pcp_dma_tx_ring* p_ring, BCM_KM_U32* p_op_count);
static void pcp_dma_line_stop(BCM_KM_U32 line);

static struct pcp_dma_desc* pcp_dma_tx_ring_desc_get(struct pcp_dma_tx_ring* p_ring, BCM_KM_U32 i)
{
  return &p_ring->p_desc[i];
}

int pcp_dma_tx_trigger(BCM_KM_U32 line, BCM_KM_U32 petra)
{
#if PCP_DMA_NUM_PETRAS_ON_LINE_CARD == 1
  static const BCM_KM_U32 vals[PCP_DMA_NUM_PETRAS_ON_LINE_CARD] = { IDM_TRIG_BIT_PETRA0 };  
#endif
#if PCP_DMA_NUM_PETRAS_ON_LINE_CARD == 4
  static const BCM_KM_U32 vals[PCP_DMA_NUM_PETRAS_ON_LINE_CARD] = { IDM_TRIG_BIT_PETRA0, IDM_TRIG_BIT_PETRA1, IDM_TRIG_BIT_PETRA2, IDM_TRIG_BIT_PETRA3};  
#endif

  return pcp_dma_write_reg(dev.regs[line].idm_petra_trig, vals[petra]);
}

static void pcp_dma_addr_to_upper_and_lower(dma_addr_t addr, BCM_KM_U32 *upper, BCM_KM_U32* lower)
{
  *lower = (BCM_KM_U32)addr;
  *upper = (sizeof(addr) > 32) ? (BCM_KM_U32)(addr >> 32) : 0;
}


static inline BCM_KM_U32 pcp_dma_desc_owner_get(struct pcp_dma_desc* p_desc)
{
  return (p_desc->hdr >> 31) & 0x1;
}

static inline BCM_KM_U32 pcp_dma_desc_sop_get(struct pcp_dma_desc* p_desc)
{
  return (p_desc->hdr >> 30) & 0x1;
}

static inline BCM_KM_U32 pcp_dma_desc_eop_get(struct pcp_dma_desc* p_desc)
{
  return (p_desc->hdr >> 29) & 0x1;
}

static inline BCM_KM_U32 pcp_dma_desc_byte_count_get(struct pcp_dma_desc* p_desc)
{
  return (p_desc->hdr >> 15) & 0x3fff;
}

static inline BCM_KM_U32 pcp_dma_desc_petra_get(struct pcp_dma_desc* p_desc)
{
  return (p_desc->hdr >> 9) & 0x3f;
}

static inline BCM_KM_U32 pcp_dma_desc_error_get(struct pcp_dma_desc* p_desc)
{
  return (p_desc->hdr >> 5) & 0xf;
}


static inline void pcp_dma_desc_owner_set(struct pcp_dma_desc* p_desc, BCM_KM_U32 owner)
{
  if(owner)
  {
    p_desc->hdr |= ((owner & 0x1) << 31);
  }
  else
  {
    p_desc->hdr &= ~(1 << 31);
  }
}

static inline void pcp_dma_desc_sop_set(struct pcp_dma_desc* p_desc, BCM_KM_U32 sop)
{
  p_desc->hdr &= ~(1 << 30);
  p_desc->hdr |= ((sop & 0x1) << 30);
}

static inline void pcp_dma_desc_eop_set(struct pcp_dma_desc* p_desc, BCM_KM_U32 eop)
{
  p_desc->hdr &= ~(1 << 29);
  p_desc->hdr |= ((eop & 0x1) << 29);
}

static inline void pcp_dma_desc_byte_count_set(struct pcp_dma_desc* p_desc, BCM_KM_U32 byte_count)
{
  p_desc->hdr &= ~(0x3fff << 15);
  p_desc->hdr |= ((byte_count & 0x3fff) << 15);
}

static inline void pcp_dma_desc_petra_set(struct pcp_dma_desc* p_desc, BCM_KM_U32 petra)
{
  p_desc->hdr &= ~(0x3f << 9);
  p_desc->hdr |= ((petra & 0x3f) << 9);
}

static inline void pcp_dma_desc_error_set(struct pcp_dma_desc* p_desc, BCM_KM_U32 error)
{
  p_desc->hdr &= ~(0xf << 5);
  p_desc->hdr |= ((error & 0xf) << 5);
}

/* set first 32-bit word of PCP_DMA descriptor */
static inline void pcp_dma_desc_hdr_set(struct pcp_dma_desc* p_desc, BCM_KM_U32 owner, 
  BCM_KM_U32 sop, BCM_KM_U32 eop, BCM_KM_U32 byte_count, BCM_KM_U32 petra, BCM_KM_U32 error)
{
  BCM_PCP_DMA_KM_TX_TRACE(6, "set descr, owner %"KM_PRINTF_FORMAT_U32" sop %"KM_PRINTF_FORMAT_U32" eop %"KM_PRINTF_FORMAT_U32" count %"KM_PRINTF_FORMAT_U32" petra %"KM_PRINTF_FORMAT_U32" error %"KM_PRINTF_FORMAT_U32"", owner, sop, eop, byte_count, petra, error);
  p_desc->hdr = 0;
  pcp_dma_desc_sop_set(p_desc, sop);
  pcp_dma_desc_eop_set(p_desc, eop);
  pcp_dma_desc_byte_count_set(p_desc, byte_count);
  pcp_dma_desc_petra_set(p_desc, petra);
  pcp_dma_desc_error_set(p_desc, error);
  pcp_dma_desc_owner_set(p_desc, owner);
}

static struct pcp_dma_rx_ring* pcp_dma_rx_ring_get(struct pcp_dma_dev* p_dev, BCM_KM_U32 line, BCM_KM_U32 class)
{
  return &p_dev->p_rx_rings[line * p_dev->num_classes + class];
}
static struct pcp_dma_tx_ring* pcp_dma_tx_ring_get(struct pcp_dma_dev* p_dev, BCM_KM_U32 line, BCM_KM_U32 pipe)
{
  return &p_dev->p_tx_rings[line * p_dev->num_pipes_per_line + pipe];
}

void align_skb(struct sk_buff *skb) {
  int reserve;

  if (buff_alloc_alignment >= 0)
  {

    reserve = buff_alloc_alignment - ((BCM_KM_U32)skb->data & (PCP_DMA_MAX_BURST-1));
    if (buff_alloc_alignment < ((BCM_KM_U32)skb->data & (PCP_DMA_MAX_BURST-1)))
    {
      /* Alignment should push use to the next 4K block */
      reserve += PCP_DMA_MAX_BURST;
    }

    BCM_PCP_DMA_KM_RX_TRACE(6, "PCP_DMA_KM: DP: RX: buff_alloc_alignment before: skb=0x%x",
        (BCM_KM_U32)skb->data);
    skb_reserve(skb, reserve);
    BCM_PCP_DMA_KM_RX_TRACE(6,
        "PCP_DMA_KM: DP: RX: buff_alloc_alignment after : skb=0x%x (reserved %db)",
        (BCM_KM_U32)skb->data, reserve);
  }
}

/* initialize rx ring */
static  int init_rx_ring(struct pcp_dma_rx_ring* p_ring, BCM_KM_U32 line, BCM_KM_U32 class)
{
  int ret = 0;
  BCM_KM_U32 i;
  BCM_KM_U32 size = 0;
  dma_addr_t phys_addr = 0;
  
  BCM_PCP_DMA_KM_RX_TRACE(1, "init rx ring line %"KM_PRINTF_FORMAT_U32" class %"KM_PRINTF_FORMAT_U32" p_ring %p chain_length %lu",line, class, p_ring, pcp_dma_rx_num_bufs_per_class);
  memset(p_ring, 0, sizeof(struct pcp_dma_rx_ring));
  p_ring->line = line;
  p_ring->class = class;
  p_ring->next_used_desc = 0;
  p_ring->buf_count = pcp_dma_rx_num_bufs_per_class;
  p_ring->desc_count = p_ring->buf_count;
  p_ring->buf_size = pcp_dma_rx_buf_size;
  p_ring->sim_next_desc = 0;
  p_ring->p_pci_dev = pci_find_slot(PCP_PCIE_BUS_NUM, PCI_DEVFN(0, 0));
  p_ring->pend_skb = NULL;
  memset(remain_edm_interrupt, 0x0, sizeof(remain_edm_interrupt));
  
  for (i = 0; i < PCP_DMA_NUM_LINE_CARDS; ++i) {
    next_class[i] = (PCP_DMA_RX_NUM_CLASSES-1);
  }

  /* setup buffers */
  size = sizeof(struct pcp_dma_buf) * p_ring->buf_count;
  p_ring->p_buf = kmalloc(size, GFP_KERNEL);
  if(unlikely(p_ring->p_buf == NULL)) {
    ret = -1;
    goto exit;
  }
  memset(p_ring->p_buf, 0, size);
    
  /* allocate skbs */
  for(i = 0; i < p_ring->buf_count; i++) {
    struct pcp_dma_buf* p_buf = &p_ring->p_buf[i];
    p_buf->skb = alloc_skb(p_ring->buf_size + (buff_alloc_alignment >= 0 ? PCP_DMA_MAX_BURST : 0), GFP_KERNEL);

    if(unlikely(p_buf->skb == NULL)) {
      ret = -1;
      goto exit;
    }

    align_skb(p_buf->skb);

    p_buf = &p_ring->p_buf[i];
       
    p_buf->dma_buf_phys =  pci_map_single(p_ring->p_pci_dev, p_buf->skb->data, p_ring->buf_size, PCI_DMA_FROMDEVICE);
  }
  
  /* setup descriptors */
  
  /* allocate memory for descriptors */
  size = sizeof(struct pcp_dma_desc) * p_ring->desc_count;
  p_ring->p_desc = dma_alloc_coherent(&p_ring->p_pci_dev->dev, size, &p_ring->desc_phys_base, GFP_KERNEL);
  if(unlikely(p_ring->p_desc == NULL)) {
    ret = -1;
    goto exit;
  }
  memset(p_ring->p_desc, 0, size);
  
  /* fill descriptots */
  for(i = 0; i < p_ring->desc_count; i++) {
    struct pcp_dma_buf* p_buf;
    struct pcp_dma_desc* p_desc = &p_ring->p_desc[i];
    BCM_KM_U32 next_i = i + 1;
    if(unlikely(next_i == p_ring->desc_count)) {
      next_i = 0;
    }
    
    /* next descriptor */
    phys_addr = p_ring->desc_phys_base +  sizeof(struct pcp_dma_desc) * next_i;
    pcp_dma_addr_to_upper_and_lower(phys_addr, &p_desc->next_desc_addr_upper, &p_desc->next_desc_addr_lower);
    
    /* buffer */
    p_buf = &p_ring->p_buf[i];
    pcp_dma_addr_to_upper_and_lower(p_buf->dma_buf_phys, &p_desc->buf_addr_upper, &p_desc->buf_addr_lower);
    
    pcp_dma_desc_byte_count_set(p_desc, p_ring->buf_size);
    pcp_dma_desc_owner_set(p_desc, PCP_DMA_DESC_OWNER_PCP_DMA);
  }
  exit:
    return ret;
}

static int init_tx_ring(struct pcp_dma_tx_ring* p_ring, BCM_KM_U32 line, BCM_KM_U32 pipe)
{
  int ret = 0;
  BCM_KM_U32 i;
  BCM_KM_U32 size = 0;
  dma_addr_t phys_addr = 0;
  
  BCM_PCP_DMA_KM_RX_TRACE(1, "init tx ring line %"KM_PRINTF_FORMAT_U32" pipe %"KM_PRINTF_FORMAT_U32" p_ring %p chain_length %lu",line, pipe, p_ring, pcp_dma_tx_num_bufs_per_petra);
  
  memset(p_ring, 0, sizeof(struct pcp_dma_tx_ring));
  p_ring->line = line;
  p_ring->pipe = pipe;
  p_ring->next_used_desc = 0;
  p_ring->buf_count = pcp_dma_tx_num_bufs_per_petra;
  p_ring->desc_count = p_ring->buf_count;
  p_ring->buf_size = PCP_DMA_TX_BUF_SIZE;
  p_ring->sim_next_desc = 0;
  
  p_ring->p_pci_dev = pci_find_slot(PCP_PCIE_BUS_NUM, PCI_DEVFN(0, 0));
          
  /* setup buffers */
  size = sizeof(struct pcp_dma_buf) * p_ring->buf_count;
  p_ring->p_buf = kmalloc(size, GFP_KERNEL);
  if(unlikely(p_ring->p_buf == NULL)) {
    ret = -1;
    goto exit;
  }
  memset(p_ring->p_buf, 0, size);
  
  /* setup descriptors */
  size = sizeof(struct pcp_dma_desc) * p_ring->desc_count;
  p_ring->p_desc = dma_alloc_coherent(&p_ring->p_pci_dev->dev, size, &p_ring->desc_phys_base, GFP_KERNEL);

  if(unlikely(p_ring->p_desc == NULL)) {
    ret = -1;
    goto exit;
  }
  memset(p_ring->p_desc, 0, size);
  
  for(i = 0; i < p_ring->desc_count; i++) {
    struct pcp_dma_desc* p_desc;
    BCM_KM_U32 next_i;
    
    p_desc = pcp_dma_tx_ring_desc_get(p_ring, i);
    
    next_i = i + 1;
    if(unlikely(next_i == p_ring->desc_count)) {
      next_i = 0;
    }
    
    /* next descriptor */
    phys_addr = p_ring->desc_phys_base +  sizeof(struct pcp_dma_desc) * next_i;
    pcp_dma_addr_to_upper_and_lower(phys_addr, &p_desc->next_desc_addr_upper, &p_desc->next_desc_addr_lower);
    
    /* buffer address is 0 (buffer not assigned) */
    
    pcp_dma_desc_owner_set(p_desc, PCP_DMA_DESC_OWNER_CPU);
  }
  
  /* write barrier */
  wmb();
  
exit:
  return ret;
}

static void deinit_rx_ring(struct pcp_dma_rx_ring* p_ring)
{
  int i;
  
  /* free descriptors */
  for(i = 0; i < p_ring->desc_count; i++) {
    struct pcp_dma_buf* p_buf = &p_ring->p_buf[i];
    if(p_buf->dma_buf_phys) {
      pci_unmap_single(p_ring->p_pci_dev, p_buf->dma_buf_phys, p_ring->buf_size,PCI_DMA_FROMDEVICE );
    }
  }
  
  if(p_ring->p_desc) {
    dma_free_coherent(&p_ring->p_pci_dev->dev, sizeof(struct pcp_dma_desc) * p_ring->desc_count, p_ring->p_desc, p_ring->desc_phys_base);
  }

  /* free skbuffs */
  for(i = 0; i < p_ring->buf_count; i++) {
    struct pcp_dma_buf* p_buf = &p_ring->p_buf[i];
    if(p_buf->skb) {
      kfree_skb(p_buf->skb);
    }
  }

  /* free buffers */
  kfree(p_ring->p_buf);
  
  return;
}

static void deinit_tx_ring(struct pcp_dma_tx_ring* p_ring)
{
  int i;

  /* free descriptors */
  for (i = 0; i < p_ring->desc_count; i++) {
    struct pcp_dma_buf* p_buf = &p_ring->p_buf[i];

    if (p_buf->dma_buf_phys) {
      pci_unmap_single(p_ring->p_pci_dev, p_buf->dma_buf_phys, p_buf->skb->len, PCI_DMA_TODEVICE);
    }
  }

  if (p_ring->p_desc) {
    dma_free_coherent(&p_ring->p_pci_dev->dev, sizeof(struct pcp_dma_desc) * p_ring->desc_count, p_ring->p_desc, p_ring->desc_phys_base);
  }

  /* free skbuffs */
  for (i = 0; i < p_ring->buf_count; i++) {
    struct pcp_dma_buf* p_buf = &p_ring->p_buf[i];
    if (p_buf->skb) {
      kfree_skb(p_buf->skb);
    }
  }

  /* free buffers */
  kfree(p_ring->p_buf);
}

/* send packet to the given pipe on the given line. 
 * packet should already contain ITMH
 */
static int pcp_dma_tx(struct sk_buff* skb, struct pcp_dma_tx_ring* p_ring)
{ 
  int ret = 0;
  int i = 0;
  struct pcp_dma_buf* p_buf;
  struct pcp_dma_desc* p_desc;
  
  if(p_ring->used_desc_count == p_ring->desc_count) {
    /* All descriptores are in use by PCP_DMA. Try to recycle. */
    for (i = 0; i < tx_recycle_retry; i++) {
      if (p_ring->used_desc_count == p_ring->desc_count) {
        BCM_KM_U32 op_count = 1; /* p_ring->desc_count; */
        BCM_PCP_DMA_KM_TX_TRACE(7, "PCP_DMA_KM: DP: tx ring full, trying to recycle tx line %"KM_PRINTF_FORMAT_U32" pipe %"KM_PRINTF_FORMAT_U32"", p_ring->line, p_ring->pipe);
        pcp_dma_recycle_tx_ring(p_ring, &op_count);
      } else {
        break;
      }
    }
    
#if BCM_PCP_DMA_KM_DEBUG
    if (dbg3 < i) {
      dbg3 = i;
    }
#endif    

    if (p_ring->used_desc_count == p_ring->desc_count) {
      BCM_PCP_DMA_KM_TX_TRACE(5, "PCP_DMA_KM: DP: tx ring full, dropped packet line %"KM_PRINTF_FORMAT_U32" pipe %"KM_PRINTF_FORMAT_U32"", p_ring->line, p_ring->pipe);
    
      /* drop */
      kfree_skb(skb);
      ret = -1;
      pkts_out_discarded++;
      goto exit;
    } 
  }
  
  /* there is free descriptor */
  p_desc = pcp_dma_tx_ring_desc_get(p_ring, p_ring->next_free_desc);
  p_buf = &p_ring->p_buf[p_ring->next_free_desc];
  
  /* associate descriptor with the buffer */
  p_buf->skb = skb;
      
  BCM_PCP_DMA_KM_TX_TRACE(6, "PCP_DMA_KM: DP: tx line %"KM_PRINTF_FORMAT_U32" pipe %"KM_PRINTF_FORMAT_U32" (desc %"KM_PRINTF_FORMAT_U32" desc addr %p) pkt addr %p pkt len %u", 
    p_ring->line, p_ring->pipe, p_ring->next_free_desc, p_desc, p_buf->skb->data, p_buf->skb->len);

  BCM_PCP_DMA_KM_TX_PKT_TRC(6, "PCP_DMA_KM: DP: tx buffer before setting owner", p_buf->skb->data, p_buf->skb->len);

  p_buf->dma_buf_phys = pci_map_single(p_ring->p_pci_dev, p_buf->skb->data, p_buf->skb->len, PCI_DMA_TODEVICE);

  pcp_dma_addr_to_upper_and_lower(p_buf->dma_buf_phys, &p_desc->buf_addr_upper, &p_desc->buf_addr_lower);

  /* advance next-free-descriptor index */
  p_ring->next_free_desc = p_ring->next_free_desc + 1;
  if (unlikely(p_ring->next_free_desc == p_ring->desc_count)) {
    p_ring->next_free_desc = 0;
  }

  /* set PCP_DMA descriptor header: PCP_DMA as the owner, SOP, EOP, skb len, petra don't care, no error */
  pcp_dma_desc_hdr_set(p_desc, PCP_DMA_DESC_OWNER_CPU, 1, 1, p_buf->skb->len, p_ring->pipe, 0);
  
  wmb();
  
  BCM_PCP_DMA_KM_TX_PKT_TRC(6, "PCP_DMA_KM: DP: tx desc before setting owner", p_desc, 20);
  
  pcp_dma_desc_owner_set(p_desc, PCP_DMA_DESC_OWNER_PCP_DMA);

  pcp_dma_tx_trigger(p_ring->line, p_ring->pipe);
  
  p_ring->used_desc_count++;
  pkts_out++;
  
  exit:
    return ret;
}

/* Go over tx ring and recycle descriptors which were filled and given to PCP_DMA, and then consumed
 * by PCP_DMA. For each such descriptor, change its ownership to CPU.
 */
static void pcp_dma_recycle_tx_ring(struct pcp_dma_tx_ring* p_ring, BCM_KM_U32* p_op_count)
{
  while((*p_op_count) > 0) {
    struct pcp_dma_desc* p_desc = pcp_dma_tx_ring_desc_get(p_ring, p_ring->next_used_desc);
    struct pcp_dma_buf* p_buf = &p_ring->p_buf[p_ring->next_used_desc];
    
    if(p_ring->used_desc_count == 0) {
      /* ring is empty - do nothing */
      return;
    }
    
    if(pcp_dma_desc_owner_get(p_desc) == PCP_DMA_DESC_OWNER_PCP_DMA) {
      BCM_PCP_DMA_KM_TX_TRACE(6, "PCP_DMA_KM: DP: tx recycle line %"KM_PRINTF_FORMAT_U32" pipe %"KM_PRINTF_FORMAT_U32" (desc %"KM_PRINTF_FORMAT_U32" %p) - descriptor is still owned by PCP_DMA", p_ring->line, p_ring->pipe, p_ring->next_used_desc, p_desc);
      BCM_PCP_DMA_KM_TX_PKT_TRC(6, "Desc: ", p_desc, 20);
            
      /* descriptor is still in use by PCP_DMA - do nothing */
      return;
    }
    
    BCM_PCP_DMA_KM_TX_TRACE(6, "PCP_DMA_KM: DP: tx recycle line %"KM_PRINTF_FORMAT_U32" pipe %"KM_PRINTF_FORMAT_U32" (desc %"KM_PRINTF_FORMAT_U32")", p_ring->line, p_ring->pipe, p_ring->next_used_desc);

    /* recycle descriptor: unmap memory, deallocate skb */
    pci_unmap_single(p_ring->p_pci_dev, p_buf->dma_buf_phys, p_buf->skb->len, PCI_DMA_TODEVICE);
    p_buf->dma_buf_phys = 0;
    kfree_skb(p_buf->skb);
    
    p_buf->skb = NULL;
    
    /* advance next-used-descriptor */
    p_ring->next_used_desc = p_ring->next_used_desc + 1;
    if (unlikely(p_ring->next_used_desc == p_ring->desc_count)) {
      p_ring->next_used_desc = 0;
    }
    p_ring->used_desc_count--;
    *p_op_count = *p_op_count - 1;
  }
}

/* go over rx ring, consume recieved packets and recycle descriptors
 */
static int pcp_dma_rx_ring(struct pcp_dma_rx_ring* p_ring, BCM_KM_U32* p_op_count)
{
  int ret = 0;
  int error = 0;
  struct sk_buff
    *skb = NULL;
  BCM_KM_U32 size = 0, is_sop = 0, is_eop = 0;
#if BCM_PCP_DMA_KM_DEBUG
  int i;
#endif    
  
  while(*p_op_count > 0) {
    struct pcp_dma_desc* p_desc = &p_ring->p_desc[p_ring->next_used_desc];
    struct pcp_dma_buf* p_buf = &p_ring->p_buf[p_ring->next_used_desc];

    if(pcp_dma_desc_owner_get(p_desc) == PCP_DMA_DESC_OWNER_PCP_DMA) {
      /* descriptor is still in use by PCP_DMA - do nothing */
      BCM_PCP_DMA_KM_RX_TRACE(7, "PCP_DMA_KM: DP: RX: desc owned by pcp_dma: rx line %"KM_PRINTF_FORMAT_U32" class %"KM_PRINTF_FORMAT_U32" desc ndx %"KM_PRINTF_FORMAT_U32" desc addr %p", p_ring->line, p_ring->class, p_ring->next_used_desc, p_desc);
      goto exit;
    }

    if (unlikely(pcp_dma_desc_error_get(p_desc) != 0)) {
      pkts_in_error++;
      error = 1;
      BCM_PCP_DMA_KM_RX_TRACE(6, "PCP_DMA_KM: DP: RX: rx error line %"KM_PRINTF_FORMAT_U32" class %"KM_PRINTF_FORMAT_U32" (error %"KM_PRINTF_FORMAT_U32")", p_ring->line, p_ring->class, pcp_dma_desc_error_get(p_desc));
      pcp_dma_desc_error_set(p_desc, 0);
    } else {
      skb = p_buf->skb;

      /* consume recvd buffer*/
      size = pcp_dma_desc_byte_count_get(p_desc);
      BCM_PCP_DMA_KM_RX_TRACE(6, "PCP_DMA_KM: DP: RX: ********** size=%"KM_PRINTF_FORMAT_U32", p_ring->buf_size=%"KM_PRINTF_FORMAT_U32, size, p_ring->buf_size);

      pci_unmap_single(p_ring->p_pci_dev, p_buf->dma_buf_phys, p_ring->buf_size, PCI_DMA_FROMDEVICE);
      p_buf->dma_buf_phys = 0;
      p_buf->skb = NULL;
      skb_put(skb, size);

      BCM_PCP_DMA_KM_RX_TRACE(6, "PCP_DMA_KM: DP: RX: rx line %"KM_PRINTF_FORMAT_U32" class %"KM_PRINTF_FORMAT_U32" pipe %"KM_PRINTF_FORMAT_U32" size %"KM_PRINTF_FORMAT_U32"", p_ring->line, p_ring->class, pcp_dma_desc_petra_get(p_desc), pcp_dma_desc_byte_count_get(p_desc));
      BCM_PCP_DMA_KM_RX_PKT_TRC(6, "PCP_DMA_KM: DP: RX: rx desc ", p_desc, 20);
      BCM_PCP_DMA_KM_RX_PKT_TRC(6, "PCP_DMA_KM: DP: RX: rx buffer", skb->data, skb->len);
      
      is_sop = pcp_dma_desc_sop_get(p_desc);
      is_eop = pcp_dma_desc_eop_get(p_desc);
      
      BCM_PCP_DMA_KM_RX_TRACE(6, "PCP_DMA_KM: DP: RX: is_sop=%"KM_PRINTF_FORMAT_U32", is_eop=%"KM_PRINTF_FORMAT_U32, is_sop, is_eop);
      
      /* for debug: if pkt corrupted, stop PCP_DMA machines */
#if BCM_PCP_DMA_KM_DEBUG
      if(stop_pcp_dma_on_corrupted_rx_buf) {
        if (is_sop) {
          BCM_KM_U32 ftmh0 = *((BCM_KM_U32*)skb->data);
          BCM_KM_U32 sys_port =  ftmh0 & 0x1fff;
          if(sys_port > 1300) {
            BCM_PCP_DMA_KM_RX_TRACE(0, "PCP_DMA_KM: DP: corrupted rx buf");
            BCM_PCP_DMA_KM_RX_PKT_TRC(0, "PCP_DMA_KM: DP: rx pkt", skb->data, 40);
            BCM_PCP_DMA_KM_RX_PKT_TRC(0, "PCP_DMA_KM: DP: rx desc ", p_desc, 20);
  
            pcp_dma_write_reg(dev.regs[p_ring->line].edm_rst, EDM_RST_BIT_RST);
            pcp_dma_write_reg(dev.regs[p_ring->line].ess_rst, ESS_RST_BIT_RST);
            pcp_dma_write_reg(dev.regs[p_ring->line].idm_rst, IDM_RST_BIT_RST);
            pcp_dma_write_reg(dev.regs[p_ring->line].iss_rst, ISS_RST_BIT_RST);
            dev.pcp_dma_line_ready[p_ring->line] = 0;
            goto exit;
          }
        }
      }
#endif /* BCM_PCP_DMA_KM_DEBUG */

      if (is_sop && is_eop) {
        /* the buffer contains a full packet */
        p_ring->pend_skb = skb;
        BCM_PCP_DMA_KM_TRACE(6, "PCP_DMA_KM: DP: RX: SOP && EOP. Full packet on pipe %"KM_PRINTF_FORMAT_U32, pcp_dma_desc_petra_get(p_desc));
        skb = NULL;
      } else {
        if (is_sop) {
          /* Start of packet. Alloc pending skb and append */
          p_ring->pend_skb = alloc_skb(PCP_DMA_MAX_PKT_SIZE, GFP_ATOMIC);
          BCM_PCP_DMA_KM_TRACE(6, "PCP_DMA_KM: DP: RX: SOP. Allocate pending packet on pipe %"KM_PRINTF_FORMAT_U32, pcp_dma_desc_petra_get(p_desc));

          if (unlikely(p_ring->pend_skb == NULL)) {
            ret = -1;
            pkts_in_discarded++;
            goto exit;
          }
        }
        
        /* Append data to pending skb */
        if ((p_ring->pend_skb->len + skb->len) > PCP_DMA_MAX_PKT_SIZE)
        {
          BCM_PCP_DMA_KM_TRACE(
              1,
              "PCP_DMA_KM: DP: RX: multi-buffer packet is larger then expected (max=%d)", 
              PCP_DMA_MAX_PKT_SIZE);
          ret = -1;
          pkts_in_discarded++;
          goto exit;
        }

        ++bufs_in;

        BCM_PCP_DMA_KM_TRACE(6, "PCP_DMA_KM: DP: RX: SOP. Appending buffer (len=%d) to pending pkt (current_len=%d) on pipe %"KM_PRINTF_FORMAT_U32, skb->len, p_ring->pend_skb->len, pcp_dma_desc_petra_get(p_desc));
        memcpy(p_ring->pend_skb->tail, skb->data, skb->len);
        skb_put(p_ring->pend_skb, skb->len);
        
        /* Do not put NULL in skb, so it'll be recycled later */
      }
      
      /* Handle packet only if full packet has arrived */
      if (is_eop)
      {
        BCM_PCP_DMA_KM_TRACE(6, "PCP_DMA_KM: DP: RX: EOP. Processing packet on pipe %"KM_PRINTF_FORMAT_U32, pcp_dma_desc_petra_get(p_desc));

        pkts_in++;

        BCM_PCP_DMA_KM_RX_PKT_TRC(6, "PCP_DMA_KM: DP: RX: rx pkt", p_ring->pend_skb->data, p_ring->pend_skb->len);
        
        /* for debug: send pkt back to pcp_dma instead of forwridng it to upper layers */
#if BCM_PCP_DMA_KM_DEBUG
        /* if echo_rx flag is on, send pkt back to pcp_dma instead of forwridng it to upper layers
         *
         * send either to the port which a pkt arrived from, or, if echo_rx_snake is on,
         *  send to a port on another petra device for snake-like testing
         */
        if (echo_rx) {
          BCM_KM_U32 ftmh0 = *((BCM_KM_U32*)p_ring->pend_skb->data);
          BCM_KM_U32 sys_port =  ftmh0 & 0x1fff;
          BCM_KM_U32 pipe;

          if (echo_rx_snake) {
            /* same port on the next pipe (cyclic) */
            pipe = (((sys_port % 400) / 100) + 1 ) % 4;
            sys_port = 400 * p_ring->line + (sys_port + 100) % 400;
          } else {
            /* pipe and sys port of the incoming pkt */
            pipe = ((sys_port % 400) / 100);
            sys_port = sys_port;
          }

          if (pcp_dma_copy_buf_on_echo_rx)
          {
            /* Copy packet data to keep alignemt as in rx */
            memmove(&p_ring->pend_skb->data[4], &p_ring->pend_skb->data[16], p_ring->pend_skb->len-16);
            skb_trim(p_ring->pend_skb, p_ring->pend_skb->len-12);            
          }
          else
          {
            /* Don't copy. Pull head of packet pointer */
            /* PCP_DMA: ignore ftmh + pph + (total = 16), and leave room for sys port (4) */
            /* when using fabric_ftmh_outlif_extension=ALWAYS: PCP:  ignore ftmh base & dest ext (6+2) + PPH base (8) + EEP (2) +learn (5) (total = 23), and leave room for sys port (4) */
            /* skb_pull(p_ring->pend_skb, 23-4); */
            /* when using fabric_ftmh_outlif_extension=IF_MC: PCP:  ignore ftmh base (6) + PPH base (8) + EEP (2) +learn (5) (total = 21), and leave room for sys port (4) */
            skb_pull(p_ring->pend_skb, 21-4);
          }

          /* set sys port */
          *((BCM_KM_U32*)p_ring->pend_skb->data) = sys_port;          
          
          /* send */
          bcm_pcp_dma_km_dp_via_dma_tx_raw(p_ring->line, pipe, p_ring->pend_skb);
          p_ring->pend_skb = NULL;
        } else if (discard_rx) {
          /* skb from rx ring will be recycled, full_pkt_skb should be freed */
          kfree_skb(p_ring->pend_skb);
          p_ring->pend_skb = NULL;
        } else {
#if BCM_PCP_DMA_KM_FRWRD_PKT_UP_LEVEL
          /* forward pkt to upper layers */
          pcp_dma_on_rx(p_ring->line, p_ring->class, p_ring->pend_skb);
#endif
          p_ring->pend_skb = NULL;
        }
#else /* BCM_PCP_DMA_KM_DEBUG */
#if BCM_PCP_DMA_KM_FRWRD_PKT_UP_LEVEL
          /* forward pkt to upper layers */
          pcp_dma_on_rx(p_ring->line, p_ring->class, p_ring->pend_skb);
#endif
          p_ring->pend_skb = NULL;
#endif /* BCM_PCP_DMA_KM_DEBUG */
      }

      /* recycle descriptor:  */
      if (skb == NULL) {
        skb = alloc_skb(p_ring->buf_size + (buff_alloc_alignment >= 0 ? PCP_DMA_MAX_BURST : 0), GFP_ATOMIC);

        if (unlikely(skb == NULL)) {
          ret = -1;
          pkts_in_discarded++;
          goto exit;
        }
        align_skb(skb);
      } else {
        skb_trim(skb, 0);
      }
      p_buf->skb = skb;
      p_buf->dma_buf_phys = pci_map_single(p_ring->p_pci_dev, p_buf->skb->data, p_ring->buf_size, PCI_DMA_FROMDEVICE);
    }
    
    pcp_dma_addr_to_upper_and_lower(p_buf->dma_buf_phys, &p_desc->buf_addr_upper, &p_desc->buf_addr_lower);
         
    pcp_dma_desc_byte_count_set(p_desc, p_ring->buf_size);
    
    /* barrier */
    mb();
    
#if BCM_PCP_DMA_KM_DEBUG
    for (i = 0; i < rx_delay_iter; ++i);    
    BCM_PCP_DMA_KM_TRACE(5, "i=%d", i);
#endif    
    
    /* mark descriptor as ready to be used by PCP_DMA */ 
    pcp_dma_desc_owner_set(p_desc, PCP_DMA_DESC_OWNER_PCP_DMA);
    
    /* advance next-used-descriptor */
    p_ring->next_used_desc = p_ring->next_used_desc + 1;
    if (unlikely(p_ring->next_used_desc == p_ring->desc_count)) {
      p_ring->next_used_desc = 0;
    }

    *p_op_count = *p_op_count - 1;
  }
 exit:
    return ret;
}


void  pcp_dma_rx_line_class_working_fn(unsigned long);
DECLARE_TASKLET(pcp_dma_rx_tasklet, pcp_dma_rx_line_class_working_fn, 0);

static void pcp_dma_rx_bh_fn(void* param)
{
  tasklet_schedule(&pcp_dma_rx_tasklet);
}


void  pcp_dma_rx_line_class_working_fn(unsigned long param)
{
  struct pcp_dma_dev* p_dev = &dev;
  struct pcp_dma_rx_ring* p_ring;
  /* TMP - hardcoded line */
  BCM_KM_U32 op_count, line = 1, edm_interrupt = 0x0;
  int class_i = -1;
  
  op_count = pcp_dma_rx_task_budget;
  
  interrupts_rx++;

  BCM_PCP_DMA_KM_TRACE(5, "PCP_DMA_KM: DP: EDM interrupt line %"KM_PRINTF_FORMAT_U32, line);
  
  if(p_dev->pcp_dma_line_ready[line] == 0) {
    goto exit_enable;
  }
#if BCM_PCP_DMA_KM_ENABLE_INTERRUPT
  bcm_pcp_dma_km_isr_edm_cause_classes_get(line, &edm_interrupt);
#endif
  BCM_PCP_DMA_KM_TRACE(5, "PCP_DMA_KM: pcp_dma_rx_line_class_working_fn: edm_interrupt=%"KM_PRINTF_FORMAT_U32, edm_interrupt);

  /* Should also handle previous edm_interrupt value, if function did not handle all (op_count reached 0) */
  edm_interrupt |= remain_edm_interrupt[line];

  while (edm_interrupt)
  {
    if (strict_prio)
    {
      ++class_i;
    }
    else 
    {
      class_i = next_class[line] = (next_class[line] + 1) % PCP_DMA_RX_NUM_CLASSES;
    }  
    
    if (edm_interrupt & pcp_dma_edm_class_mask[class_i])
    {
      BCM_PCP_DMA_KM_TRACE(5, "PCP_DMA_KM: DP: EDM interrupt line %"KM_PRINTF_FORMAT_U32" class %d", line, class_i);
      p_ring = pcp_dma_rx_ring_get(p_dev, line, class_i);
      if (unlikely(!p_ring))
      {
        goto exit_enable;
      }

      /*  process received packets until ring is empty or budget is over */
      pcp_dma_rx_ring(p_ring, &op_count);

      /* if there are still unprocessed packets in the ring (budget is over), reschedule this work */
      if (op_count == 0)
      {
#if BCM_PCP_DMA_KM_DEBUG
        dbg2++;
#endif
        /* Remember the unhandled classes for next time we enter this function */
        remain_edm_interrupt[line] = edm_interrupt;
        tasklet_schedule(&pcp_dma_rx_tasklet);
        return;
      } else {
        /* Remove this class from the edm_interrupt */
        edm_interrupt &= (~pcp_dma_edm_class_mask[class_i]);
      }
    }
  }

exit_enable:

  remain_edm_interrupt[line] = edm_interrupt;
#if BCM_PCP_DMA_KM_ENABLE_INTERRUPT
  /* enable EDM interrupts for this line (all classes) */
  bcm_pcp_dma_km_isr_edm_interrupt_enable(line, 1);
#endif
}

static int line_rr = 0;
static void pcp_dma_rx(struct pcp_dma_dev* p_dev, BCM_KM_U32* p_op_count)
{
  BCM_KM_U32 class = 0, i;
  struct pcp_dma_rx_ring* p_ring;
  
  while(*p_op_count > 0)
  {
    BCM_KM_U32 init_count = *p_op_count;
    
    /* for all classes in the strict priority order
     * within each class for all line cards (maintaining RR)
     * ...
     */
    for(class = 0; class < p_dev->num_classes; class++) {
      for(i = 0; i < p_dev->num_lines; i++) {
        line_rr++;
                
        if(unlikely(line_rr == p_dev->num_lines)) {
          line_rr = 0;
        }

        if(p_dev->pcp_dma_line_ready[line_rr] == 0) {
            continue;
        }
        
        p_ring = pcp_dma_rx_ring_get(p_dev, line_rr, class);
        if(unlikely(!p_ring)) {
          break;
        }
        
        pcp_dma_rx_ring(p_ring, p_op_count);
       
        if(*p_op_count == 0) {
          return;
        }
      }
    }
    if(*p_op_count == init_count) {
      /* no available packets, exit */
      return;
    }
  }
}

static void  pcp_dma_rx_working_fn(KM_WORKQUEUE_FN_PARAM* arg)
{
  struct pcp_dma_dev* p_dev = &dev;
  
  BCM_KM_U32 op_count = 10;
  
  if(!p_dev->do_continue) {
    up(&p_dev->sem);
    return;
  }
  
  BCM_PCP_DMA_KM_RX_TRACE(7, "PCP_DMA_KM: DP: pcp_dma_rx_working_fn: calling pcp_dma_rx");

  if(p_dev->do_continue) {
    BCM_KM_U32 op_count = 10;
    pcp_dma_rx(p_dev, &op_count);
  }

  if( op_count != 0 ) {
    BCM_PCP_DMA_KM_RX_TRACE(7, "PCP_DMA_KM: DP: scheduling schedule_delayed_work (op_count != 0)");
    if(schedule_delayed_work(&rx_work, pcp_dma_rx_working_fn_sleep_jiffies ) <= 0) {
      BCM_PCP_DMA_KM_RX_TRACE(0, "PCP_DMA_KM: DP: schedule_delayed_work failed");
    }
  } else {
    BCM_PCP_DMA_KM_RX_TRACE(7, "PCP_DMA_KM: DP: scheduling schedule_delayed_work (op_count == 0)");
    /* otherwise, schedule next recv work immediately */
    schedule_delayed_work(&rx_work, pcp_dma_rx_working_fn_sleep_jiffies);
  } 
}


/* initialize static struct with addresses of relevant registers on all line cards */
static void pcp_dma_init_regs_addrs(void)
{
  BCM_KM_U32 line;
  
  for(line = 0; line < PCP_DMA_NUM_LINE_CARDS; line++)
  {
    dev.regs[line].iss_rst =                    (char*)(pcp_dma_offset[line]) + ISS_RST;
                                                                        
    dev.regs[line].ess_rst =                    (char*)(pcp_dma_offset[line]) + ESS_RST;
                                                                       
    dev.regs[line].idm_rst =                    (char*)(pcp_dma_offset[line]) + IDM_RST;
    dev.regs[line].idm_petra_trig =             (char*)(pcp_dma_offset[line]) + IDM_TRIG;
#if PCP_DMA_NUM_PETRAS_ON_LINE_CARD >= 1
    dev.regs[line].idm_petra_head_desc_lsb[0] = (char*)(pcp_dma_offset[line]) + IDM_DESC_HDR_LSB_PETRA0;
#endif
#if PCP_DMA_NUM_PETRAS_ON_LINE_CARD >= 2
    dev.regs[line].idm_petra_head_desc_lsb[1] = (char*)(pcp_dma_offset[line]) + IDM_DESC_HDR_LSB_PETRA1;
#endif
#if PCP_DMA_NUM_PETRAS_ON_LINE_CARD >= 3
    dev.regs[line].idm_petra_head_desc_lsb[2] = (char*)(pcp_dma_offset[line]) + IDM_DESC_HDR_LSB_PETRA2;
#endif
#if PCP_DMA_NUM_PETRAS_ON_LINE_CARD >= 4
    dev.regs[line].idm_petra_head_desc_lsb[3] = (char*)(pcp_dma_offset[line]) + IDM_DESC_HDR_LSB_PETRA3;
#endif
  
                                                                        
    dev.regs[line].edm_rst =                    (char*)(pcp_dma_offset[line]) + EDM_RST;
#if PCP_DMA_RX_NUM_CLASSES >= 1
    dev.regs[line].edm_petra_head_desc_lsb[0] = (char*)(pcp_dma_offset[line]) + EDM_DESC_HEAD_LSB_CLASS1;
#endif
#if PCP_DMA_RX_NUM_CLASSES >= 2
    dev.regs[line].edm_petra_head_desc_lsb[1] = (char*)(pcp_dma_offset[line]) + EDM_DESC_HEAD_LSB_CLASS2;
#endif
#if PCP_DMA_RX_NUM_CLASSES >= 3
    dev.regs[line].edm_petra_head_desc_lsb[2] = (char*)(pcp_dma_offset[line]) + EDM_DESC_HEAD_LSB_CLASS3;
#endif
#if PCP_DMA_RX_NUM_CLASSES > 4
    dev.regs[line].edm_petra_head_desc_lsb[3] = (char*)(pcp_dma_offset[line]) + EDM_DESC_HEAD_LSB_CLASS4;
#endif
#if PCP_DMA_RX_NUM_CLASSES >= 5
    dev.regs[line].edm_petra_head_desc_lsb[4] = (char*)(pcp_dma_offset[line]) + EDM_DESC_HEAD_LSB_CLASS5;
#endif
#if PCP_DMA_RX_NUM_CLASSES >= 6
    dev.regs[line].edm_petra_head_desc_lsb[5] = (char*)(pcp_dma_offset[line]) + EDM_DESC_HEAD_LSB_CLASS6;
#endif
#if PCP_DMA_RX_NUM_CLASSES >= 7
    dev.regs[line].edm_petra_head_desc_lsb[6] = (char*)(pcp_dma_offset[line]) + EDM_DESC_HEAD_LSB_CLASS7;
#endif
 }
 
}


static void pcp_dma_line_stop(BCM_KM_U32 line)
{
  BCM_KM_U32 class, petra;
  
  if(dev.pcp_dma_line_ready[line] == 0) {
    BCM_PCP_DMA_KM_TRACE(6, "PCP_DMA_KM: DP: stopping line %"KM_PRINTF_FORMAT_U32" - line is not active", line);
    goto exit;
  }
  
#if BCM_PCP_DMA_KM_ENABLE_INTERRUPT
    bcm_km_isr_enable_line(line, 0);
#endif

  BCM_PCP_DMA_KM_TRACE(6, "PCP_DMA_KM: DP: stopping line %"KM_PRINTF_FORMAT_U32"", line);
  
  dev.pcp_dma_line_ready[line] = 0;
  
  /* stop PCP_DMA machines */
  pcp_dma_write_reg(dev.regs[line].edm_rst, EDM_RST_BIT_RST);
  pcp_dma_write_reg(dev.regs[line].ess_rst, ESS_RST_BIT_RST);

  pcp_dma_write_reg(dev.regs[line].idm_rst, IDM_RST_BIT_RST);
  pcp_dma_write_reg(dev.regs[line].iss_rst, ISS_RST_BIT_RST);
  
  /* deallocate tx rings */
  for(petra = 0; petra < PCP_DMA_NUM_PETRAS_ON_LINE_CARD; petra++) {
    struct pcp_dma_tx_ring* p_ring = pcp_dma_tx_ring_get(&dev, line, petra);
    deinit_tx_ring(p_ring);
  }
  
  /* deallocate rx rings */
  for(class = 0; class < PCP_DMA_RX_NUM_CLASSES; class++) {
    struct pcp_dma_rx_ring* p_ring = pcp_dma_rx_ring_get(&dev, line, class);
    deinit_rx_ring(p_ring);
  }

  exit:
    return;
}

static int pcp_dma_line_start(BCM_KM_U32 line)
{
  BCM_KM_U32 petra, class;
  int ret;
  
  
  
#if BCM_PCP_DMA_KM_DEBUG
  rx_ring_0_curr_desc_ndx_ptr = (int)&dev.p_rx_rings[0].next_used_desc;
#endif

  if(dev.pcp_dma_line_ready[line]) {
    BCM_PCP_DMA_KM_TRACE(6, "PCP_DMA_KM: DP: restarting line %"KM_PRINTF_FORMAT_U32" - line already active", line);
        
     pcp_dma_line_stop(line);

  }
  
  BCM_PCP_DMA_KM_TRACE(6, "PCP_DMA_KM: DP: starting line %"KM_PRINTF_FORMAT_U32"", line);
  
  /* reset, then out of reset iss,ess,idm,edm machines */
  ret = pcp_dma_write_reg(dev.regs[line].ess_rst, ESS_RST_BIT_RST);
  if(ret) goto exit;
  ret = pcp_dma_write_reg(dev.regs[line].edm_rst, EDM_RST_BIT_RST);
  if(ret) goto exit;
  
  ret = pcp_dma_write_reg(dev.regs[line].iss_rst, ISS_RST_BIT_RST);
  if(ret) goto exit;
  ret = pcp_dma_write_reg(dev.regs[line].idm_rst, IDM_RST_BIT_RST);
  if(ret) goto exit;
      
  ret = pcp_dma_write_reg(dev.regs[line].idm_rst, 0);
  if(ret) goto exit;
  
  ret = pcp_dma_write_reg(dev.regs[line].iss_rst, 0);
  if(ret) goto exit;
 
  ret = pcp_dma_write_reg(dev.regs[line].ess_rst, 0);
  if(ret) goto exit;
  ret = pcp_dma_write_reg(dev.regs[line].edm_rst, 0);
  if(ret) goto exit;
  

  /* idm and edm*/
  for(petra = 0; petra < PCP_DMA_NUM_PETRAS_ON_LINE_CARD; petra++) {
    struct pcp_dma_tx_ring* p_ring = pcp_dma_tx_ring_get(&dev, line, petra);
    
    init_tx_ring(p_ring, line, petra);
    
    /* configure ring head descriptors registers */
    ret = pcp_dma_write_reg(dev.regs[line].idm_petra_head_desc_lsb[petra], p_ring->desc_phys_base);
    if(ret) goto exit;
  }
  
  /* configure ring head descriptors registers */ 
  for(class = 0; class < PCP_DMA_RX_NUM_CLASSES; class++) {
    struct pcp_dma_rx_ring* p_ring = pcp_dma_rx_ring_get(&dev, line, class);
    
    init_rx_ring(p_ring, line, class);
  
    /* configure ring head descriptors registers */
    ret = pcp_dma_write_reg(dev.regs[line].edm_petra_head_desc_lsb[class], p_ring->desc_phys_base);
    if(ret) goto exit;
  }

  /* enable all machines */
  ret = pcp_dma_write_reg(dev.regs[line].iss_rst, ISS_RST_BIT_ENABLE);
  if(ret) goto exit;
  ret = pcp_dma_write_reg(dev.regs[line].idm_rst, IDM_RST_BIT_ENABLE);
  if(ret) goto exit;

  ret = pcp_dma_write_reg(dev.regs[line].ess_rst, ESS_RST_BIT_ENABLE);
  if(ret) goto exit;
  ret = pcp_dma_write_reg(dev.regs[line].edm_rst, EDM_RST_BIT_ENABLE);
  if(ret) goto exit;

  
  for(petra = 0; petra < PCP_DMA_NUM_PETRAS_ON_LINE_CARD; petra++) {
    pcp_dma_tx_trigger(line, petra);
  }
    
  dev.pcp_dma_line_ready[line] = 1;
  
#if BCM_PCP_DMA_KM_ENABLE_INTERRUPT
    bcm_km_isr_enable_line(line, 1);
#endif

exit:
  return 0;
}



int bcm_pcp_dma_km_dp_via_dma_line_init(int line, int deinit)
{
  int ret = 0;
  
  if(deinit) {
    pcp_dma_line_stop(line);
  }
  else {
    ret = pcp_dma_line_start(line);
  }
  
  return ret;
}

int bcm_pcp_dma_km_dp_via_dma_module_init(void)
{
  int ret = 0;
  int i;
  static struct pcp_dma_dev* p_dev = &dev;

  if(bcm_pcp_dma_km_pkt_interrupt_support) {
#if BCM_PCP_DMA_KM_ENABLE_INTERRUPT
    bcm_km_isr_set_pcp_dma_rx_bh_fn(pcp_dma_rx_bh_fn);
#else
    /* This "else" is to avoid compilation warning when BCM_PCP_DMA_KM_ENABLE_INTERRUPT=0 */
    pcp_dma_rx_bh_fn(NULL);
#endif
  }
  
  pcp_dma_init_regs_addrs();
  
  for(i = 0; i < PCP_DMA_NUM_LINE_CARDS; ++i) {
    p_dev->pcp_dma_line_ready[i] = 0; 
  }
   
  p_dev->num_classes = PCP_DMA_RX_NUM_CLASSES;
  p_dev->num_lines = PCP_DMA_NUM_LINE_CARDS;
  p_dev->num_pipes_per_line = PCP_DMA_NUM_PETRAS_ON_LINE_CARD;
  p_dev->p_rx_rings = NULL;
  p_dev->p_tx_rings = NULL;

  p_dev->p_rx_rings = kmalloc(sizeof(struct pcp_dma_rx_ring) * p_dev->num_classes * p_dev->num_lines, GFP_KERNEL);
  
  if(!p_dev->p_rx_rings) {
    ret = -1;
    BCM_PCP_DMA_KM_TRACE(0, "Failed to allocate rx ring");
    goto exit;
  }
   
  p_dev->p_tx_rings = kmalloc(sizeof(struct pcp_dma_tx_ring) * p_dev->num_pipes_per_line * p_dev->num_lines, GFP_KERNEL);
  
  if(!p_dev->p_tx_rings) {
    ret = -1;
    BCM_PCP_DMA_KM_TRACE(0, "Failed to allocate tx ring");
    kfree(p_dev->p_rx_rings);
    goto exit;
  }
  

  if(!bcm_pcp_dma_km_pkt_interrupt_support){
    p_dev->do_continue = 1;
    sema_init (&p_dev->sem, 0);
    
    INIT_DELAYED_WORK(&rx_work, pcp_dma_rx_working_fn);

    if( schedule_delayed_work(&rx_work, 0) <= 0) {
      kfree(p_dev->p_tx_rings);
      kfree(p_dev->p_rx_rings);
      BCM_PCP_DMA_KM_TRACE(0, "Schedule_work failed");
      ret = -1;
    }
  }
  exit:
    return ret;
}

int bcm_pcp_dma_km_dp_via_dma_module_deinit(void)
{
  static struct pcp_dma_dev* p_dev = &dev;
  BCM_KM_U32 line;

  if(!bcm_pcp_dma_km_pkt_interrupt_support) {
    p_dev->do_continue = 0;
    down(&p_dev->sem);
  }

  flush_scheduled_work();
      
  for(line = 0; line < PCP_DMA_NUM_LINE_CARDS; line++) {
    pcp_dma_line_stop(line);
  }
  
  kfree(p_dev->p_tx_rings);
  kfree(p_dev->p_rx_rings);
  return 0;
}

int bcm_pcp_dma_km_dp_via_dma_tx_raw(BCM_KM_U32 line, BCM_KM_U32 pipe, struct sk_buff* skb)
{
  int ret = 0;
  static struct pcp_dma_dev* p_dev = &dev;
  
  if(pipe >= PCP_DMA_NUM_PETRAS_ON_LINE_CARD) {
    BCM_PCP_DMA_KM_TX_TRACE(5, "PCP_DMA_KM: DP: TX: pipe %"KM_PRINTF_FORMAT_U32" does not exist. Drop packet.", pipe);
    kfree_skb(skb);
    ret = -1;
    goto exit;
  } 

  /* if line card was not initalized - error */
  if(dev.pcp_dma_line_ready[line] == 0) {
     kfree_skb(skb);
     ret = -1;
     goto exit;
  }

  
  /* send */
  ret = pcp_dma_tx(skb, pcp_dma_tx_ring_get(p_dev, line, pipe));
 
exit:
  return ret;
}

/* Module Declarations */

/*
 * Initialize the module 
 */
int bcm_pcp_dma_km_init(void)
{
  
  bcm_pcp_dma_km_dp_via_dma_module_init();
  bcm_pcp_dma_km_dp_via_dma_line_init(0, 0);

  return 0;
}

#ifdef BCM_PCP_DMA_KM_DEBUG
/* handler for BCM_PCP_DMA_KM_IOCTL_PKT_TX_RAW ioctl */
int bcm_pcp_dma_km_usr_pkt_tx_raw_cmd(unsigned long param)
{
  /* struct bcm_pcp_dma_km_ioctl_pkt_tx_raw_info params; */
  struct sk_buff* p_skb;
  char buf[50];

  memset(buf, 0x0, sizeof(char) * 50);

  /* copy params from user */
  /*copy_from_user(&params, (struct bcm_pcp_dma_km_ioctl_rx_raw_info *) param, 
    sizeof (struct bcm_pcp_dma_km_ioctl_pkt_tx_raw_info));

  if((params.buf_len <= 0) || (params.buf_len > KM_MAX_PKT_LEN)) {
    return -EINVAL;
  }*/

  /* allocate skb */
  p_skb = alloc_skb (50 /* params.buf_len*/, GFP_KERNEL);

  if (p_skb == NULL)  {
    BCM_PCP_DMA_KM_TRACE(7, "bcm_pcp_dma_km_usr_pkt_tx_raw_cmd fail to alloc skb");
    return -ENOMEM;
  }

  /* copy packet from user */
/*  if (copy_from_user (skb_put(p_skb, params.buf_len), params.buf, params.buf_len)) {
    kfree_skb (p_skb);
    BCM_PCP_DMA_KM_TRACE(7, "bcm_pcp_dma_km_usr_pkt_tx_raw_cmd fail to copy user buf into skb");
    return -ENOMEM;
  }*/

  skb_put(p_skb, 50);
  memcpy(p_skb->data, buf, 50);  

  /* send */
  bcm_pcp_dma_km_dp_via_dma_tx_raw(0/*params.line*/, 0/*params.pipe*/, p_skb);

  return 0;
}

#define BCM_PCP_DMA_KM_IOCTL_PKT_TX_RAW       _IOR(BCM_PCP_DMA_KM_CHDEV_NUM, 1, char*)
#define BCM_PCP_DMA_KM_IOCTL_MEM_READ       _IOR(BCM_PCP_DMA_KM_CHDEV_NUM, 2, char*)
#endif

static int bcm_pcp_dma_km_chdev_open(struct inode *p_inode, struct file *p_file);
static int bcm_pcp_dma_km_chdev_close (struct inode *p_inode, struct file *p_file);
static int bcm_pcp_dma_km_chdev_ioctl(struct inode *p_inode, struct file *p_file, 
    unsigned int ioctl_num, unsigned long ioctl_param);

/* character device operations structure */
static struct file_operations fops = {
  owner:THIS_MODULE,
  ioctl:bcm_pcp_dma_km_chdev_ioctl,
  open:bcm_pcp_dma_km_chdev_open,
  release:bcm_pcp_dma_km_chdev_close,
};

/* character device info */
struct {
} bcm_pcp_dma_km_chdev_info;

/* the global instance of character device info */
struct bcm_pcp_dma_km_chdev_info* G_p_chdev_info = NULL;  

/* open character device */
static int 
  bcm_pcp_dma_km_chdev_open (
    struct inode *p_inode, 
    struct file *p_file)
{
  /* KM_UNUSED_ARG(p_inode); */

  p_file->private_data = (void *) G_p_chdev_info;
      
  BCM_PCP_DMA_KM_TRACE(0,"BCM_PCP_DMA character device opened");

  return 0;
}

/* release character device */
static int 
  bcm_pcp_dma_km_chdev_close (
    struct inode *p_inode, 
    struct file *p_file)
{
  /* KM_UNUSED_ARG(p_inode);
  KM_UNUSED_ARG(p_file); */

  BCM_PCP_DMA_KM_TRACE(0,"BCM_PCP_DMA character device closed");

  return 0;
}

/* character device IOCTL */
static int 
  bcm_pcp_dma_km_chdev_ioctl(
    struct inode *p_inode, 
    struct file *p_file, 
    unsigned int ioctl_num,
    unsigned long ioctl_param)
{
  int ret = 0;
  BCM_KM_U32 addr, data;

  /* KM_UNUSED_ARG(p_inode);
  KM_UNUSED_ARG(p_file); */

  switch (ioctl_num)
  {
#if BCM_PCP_DMA_KM_DEBUG
    case BCM_PCP_DMA_KM_IOCTL_PKT_TX_RAW:
      BCM_PCP_DMA_KM_TRACE(5,"BCM_PCP_DMA_KM_IOCTL_PKT_TX_RAW:");
      ret = bcm_pcp_dma_km_usr_pkt_tx_raw_cmd(ioctl_param);
      break;
    case BCM_PCP_DMA_KM_IOCTL_MEM_READ:
      BCM_PCP_DMA_KM_TRACE(0,"BCM_PCP_DMA_KM_IOCTL_MEM_READ:");
      addr = 0x1574;
      ret = lkbde_mem_read(PCP_BDE_UNIT_NUM, addr, &data);
      BCM_PCP_DMA_KM_TRACE(0,"data=0x%x",data);
      break;
#endif
    default:
        BCM_PCP_DMA_KM_TRACE(0,"Unknown ioctl in PCP_DMA_KM character device, ioctl = %d", ioctl_num);
      break;
  }

  return ret;
}

int bcm_pcp_dma_km_chdev_module_init(void)
{
  int ret = 0;

 
  /* Register character device */
  ret = register_chrdev (BCM_PCP_DMA_KM_CHDEV_NUM, BCM_PCP_DMA_KM_CHDEV_NAME, &fops);

  if (ret < 0)
  {
    BCM_PCP_DMA_KM_TRACE(0, "Registration of character device %d %s failed in bcm_pcp_dma_km_chdev_init()", BCM_PCP_DMA_KM_CHDEV_NUM, BCM_PCP_DMA_KM_CHDEV_NAME);
    return ret;
  }

  G_p_chdev_info = (struct bcm_pcp_dma_km_chdev_info*) kmalloc (sizeof (bcm_pcp_dma_km_chdev_info), GFP_KERNEL);

  if (G_p_chdev_info == NULL)
  {
      unregister_chrdev (BCM_PCP_DMA_KM_CHDEV_NUM, BCM_PCP_DMA_KM_CHDEV_NAME);

      BCM_PCP_DMA_KM_TRACE(0, "Memory allocation failed in bcm_pcp_dma_km_chdev_init()");
      return -ENOMEM;
  }

  BCM_PCP_DMA_KM_TRACE(0, "Registered PCP_DMA_KM character device %d %s", BCM_PCP_DMA_KM_CHDEV_NUM, BCM_PCP_DMA_KM_CHDEV_NAME);
  
  return 0;
}

int bcm_pcp_dma_km_chdev_module_deinit(void)
{
  unregister_chrdev (BCM_PCP_DMA_KM_CHDEV_NUM, BCM_PCP_DMA_KM_CHDEV_NAME);
  
  if (G_p_chdev_info != NULL)
  {
    kfree(G_p_chdev_info);
  }

  BCM_PCP_DMA_KM_TRACE(0, "Unregistered PCP_DMA_KM character device %d %s", BCM_PCP_DMA_KM_CHDEV_NUM, BCM_PCP_DMA_KM_CHDEV_NAME);
  
  return 0;
}


#ifdef BCM_KNET_SUPPORT
/* Compatibility Macros */

#ifdef LKM_2_4

#include <linux/compatmac.h>
#include <linux/wrapper.h>
#define LKM_MOD_PARAM(n,ot,nt,d) MODULE_PARM(n,ot)
#define LKM_EXPORT_SYM(s)
#define _free_netdev kfree

#else /* LKM_2_6 */

#define LKM_MOD_PARAM(n,ot,nt,d) module_param(n,nt,d)
#define LKM_EXPORT_SYM(s) EXPORT_SYMBOL(s)
#define _free_netdev free_netdev

#endif /* LKM_2_x */

static char *mac_addr = NULL;
LKM_MOD_PARAM(mac_addr, "s", charp, 0);
MODULE_PARM_DESC(mac_addr,
"Ethernet MAC address (default 02:10:18:xx:xx:xx)");

typedef char mac_addr_t[6];

#define BCM_OK      (0)
#define BCM_ERR     (-1)

/* Default random MAC address has Broadcom OUI with local admin bit set */
static mac_addr_t bnet_dev_mac = { 0x02, 0x10, 0x18, 0x00, 0x00, 0x00 };
static int mac_seed = 1;

typedef struct {
    struct net_device *ndev;
    struct net_device_stats stats;
} bnet_dev_t;

static bnet_dev_t bnet_dev;
#define bnet_dev_name "bcmPCP"

struct bnet_priv_t {
    bnet_dev_t *bdev;
};

/* Compatibility */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,27)
static inline void *netdev_priv(struct net_device *dev)
{
        return dev->priv;
}
#endif /* KERNEL_VERSION(2,4,27) */

int 
bcm_knet_pkt_rx(BCM_KM_U32 line, BCM_KM_U32 class, struct sk_buff* skb)
{
    bnet_dev_t *bdev = ((struct bnet_priv_t *)netdev_priv(bnet_dev.ndev))->bdev;

    BCM_PCP_DMA_KM_TRACE(1, "bcm_knet_pkt_rx: rx'ed a packet from PCP\n");

    if (bdev == NULL) {
        kfree_skb(skb);
        bdev->stats.rx_dropped++;
        BCM_PCP_DMA_KM_TRACE(0, "bcm_knet_pkt_rx: Received invalid skb from PCP\n");
        return BCM_ERR;
    }

    /* send packet up the stack */
    bdev->stats.rx_packets++;
    bdev->stats.rx_bytes += skb->len;
    skb->dev = bdev->ndev;
    netif_rx (skb);

    BCM_PCP_DMA_KM_TRACE(1, "bcm_knet_pkt_rx: sent packet up netif stack\n");
    return BCM_OK;
}

static int 
bnet_open(struct net_device *dev)
{
    netif_start_queue(dev);
    return BCM_OK;
}

static int
bnet_stop(struct net_device *dev)
{
    netif_stop_queue(dev);
    return BCM_OK;
}	

/* Network Device Statistics. Cleared at init time. */
static struct net_device_stats *
bnet_get_stats(struct net_device *dev)
{
    bnet_dev_t *bdev = ((struct bnet_priv_t *)netdev_priv(dev))->bdev;

    if (bdev == NULL) {
        /* Unknown device */
        return NULL;
    }

    return &bdev->stats;
}

/* fake multicast ability */
static void
bnet_set_multicast_list(struct net_device *dev)
{
}

static int 
bnet_tx(struct sk_buff *skb, struct net_device *edev)
{
    bnet_dev_t*         bdev = ((struct bnet_priv_t *)netdev_priv(edev))->bdev;
    struct pcp_dma_dev* pdev = &dev;
    int                 line = 0; 
    int                 pipe = 0; 
    int                 rv;

    BCM_PCP_DMA_KM_TRACE(1, "bnet_tx called.\n");

    if (bdev == NULL) {
        /* Unknown device */
        BCM_PCP_DMA_KM_TRACE(0, "bnet_tx called with invalid net_device ptr.\n");
        return BCM_ERR;
    }

    bdev->stats.tx_packets++;
    bdev->stats.tx_bytes += skb->len;

    /* if line card was not initalized - error */
    if(dev.pcp_dma_line_ready[line] == 0) {
        bdev->stats.tx_dropped++;
        kfree_skb(skb);
        BCM_PCP_DMA_KM_TRACE(0, "bnet_tx line card was not initalized error \n");
        rv = BCM_ERR;
    }

    if (rv == BCM_OK) {
        /* send */
        rv = pcp_dma_tx(skb, pcp_dma_tx_ring_get(pdev, line, pipe)); 
        if (rv != 0) {
            BCM_PCP_DMA_KM_TRACE(0, "bnet_tx: failed to tx to PCP\n");
            rv = BCM_ERR;
            bdev->stats.tx_errors++;
        }
    }

    return rv;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29))
static const struct net_device_ops bkn_netdev_ops = {
	.ndo_open		= bnet_open,
	.ndo_stop		= bnet_stop,
	.ndo_start_xmit		= bnet_tx,
	.ndo_get_stats		= bnet_get_stats,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_multicast_list	= bnet_set_multicast_list,
	.ndo_set_mac_address	= NULL,
	.ndo_do_ioctl		= NULL,
	.ndo_tx_timeout		= NULL,
	.ndo_change_mtu		= NULL,
};
#endif

static struct net_device *
bnet_dev_init(mac_addr_t mac, bnet_dev_t *bnet_dev)
{
    struct net_device *ndev;

    /* Create the kernel ethernet device */
    ndev = alloc_etherdev(sizeof(struct bnet_priv_t));
#ifdef SET_MODULE_OWNER
    SET_MODULE_OWNER(ndev);
#endif

    if (ndev == NULL) {
        BCM_PCP_DMA_KM_TRACE(0, "Error initializing ethernet device.\n");
        return NULL;
    }

    ((struct bnet_priv_t *)netdev_priv(ndev))->bdev = bnet_dev;

    /* Set the device MAC address */
    memcpy(ndev->dev_addr, mac, 6);

    /* Device information -- not available right now */
    ndev->irq = 0;
    ndev->base_addr = 0;
    
    /* Device vectors */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29))
    ndev->netdev_ops = &bkn_netdev_ops;
#else
    ndev->open = bnet_open;
    ndev->hard_start_xmit = bnet_tx;
    ndev->stop = bnet_stop;
    ndev->set_multicast_list = bnet_set_multicast_list;
    ndev->do_ioctl = NULL;
    ndev->get_stats = bnet_get_stats;
#endif

    /* Register the kernel ethernet device */
    if (register_netdev(ndev)) {
        BCM_PCP_DMA_KM_TRACE(0, "Error registering ethernet device.\n");
        _free_netdev(ndev);
        return NULL;
    }

    return ndev;
}

/* initialize the BCM knet module */
int bnet_init(void)
{
    int     i;
    
    /* add unique last 3 bytes */
    for (i = 5; i >= 3; i--) {
        bnet_dev_mac[i] = (mac_seed << ((5 - i) * 8)) & 0xff;
    }

    /* Check for user-supplied MAC address (recommended) */
    if (mac_addr != NULL && strlen(mac_addr) == 17) {
        for (i = 0; i < 6; i++) {
            bnet_dev_mac[i] = simple_strtoul(&mac_addr[i*3], NULL, 16) & 0xFF;
        }
        /* Do not allow multicast address */
        bnet_dev_mac[0] &= ~0x01;
    }

    if ((bnet_dev.ndev = bnet_dev_init(bnet_dev_mac, &bnet_dev)) == NULL) {
        BCM_PCP_DMA_KM_TRACE(0, "bnet_dev_init failed.");
        return BCM_ERR;
    }

    BCM_PCP_DMA_KM_TRACE(0, "bnet_init successful.");
    return BCM_OK;
}

static int
bnet_cleanup(void)
{
    unregister_netdev(bnet_dev.ndev);

    _free_netdev(bnet_dev.ndev);

    BCM_PCP_DMA_KM_TRACE(0, "bnet_cleanup successful.");
    return BCM_OK;
}

#endif /* BCM_KNET_SUPPORT */



/* initialize module */
int bcm_pcp_dma_km_module_init (void)
{
  int ret = 0;

  BCM_PCP_DMA_KM_TRACE(0, "Init BCM_PCP_DMA kernel module.");
  bcm_pcp_dma_km_chdev_module_init();
  bcm_pcp_dma_km_init();

  bnet_init();

  return ret;
}

/* de-initialize module */
void bcm_pcp_dma_km_module_exit(void)
{
  bcm_pcp_dma_km_dp_via_dma_module_deinit();
  bcm_pcp_dma_km_chdev_module_deinit();  

  BCM_PCP_DMA_KM_TRACE(0, "BCM_PCP_DMA kernel module De-Init");

  bnet_cleanup();

  return;
}

module_init(bcm_pcp_dma_km_module_init);
module_exit(bcm_pcp_dma_km_module_exit);
