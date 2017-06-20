/*******************************************************************************
 *
 * Copyright 2015-2017 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in an
 * Authorized License, Broadcom grants no license (express or implied), right to
 * use, or waiver of any kind with respect to the Software, and Broadcom expressly
 * reserves all rights in and to the Software and all intellectual property rights
 * therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
 * SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 * TO THE SOFTWARE. BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 * OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 *******************************************************************************/

#ifndef __KBP_DRIVER_H
#define __KBP_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#define KBP_IOCTL_INVALID       0x0
#define KBP_IOCTL_INTERRUPT     0x1
#define KBP_IOCTL_GET_REG_MEM   0x2
#define KBP_IOCTL_DMA_SETUP     0x3
#define KBP_IOCTL_DMA_CTRL      0x4
#define KBP_IOCTL_LOOPBACK_MODE 0x5
#define KBP_IOCTL_VERSION       0x6

struct kbp_sys_cfg
{
    unsigned long long reg_physical_addr;
    unsigned long long sys_physical_addr;
    unsigned int reg_mem_size;
    unsigned int sys_mem_size;
    unsigned int req_base_offset;
    unsigned int resp_base_offset;
    unsigned int req_size;
    unsigned int resp_size;
    unsigned int req_head_offset;
    unsigned int resp_head_offset;
    unsigned int req_tail_offset;
    unsigned int resp_tail_offset;
    unsigned int major;
    unsigned int minor;
};

#define KBP_FPGA_PATH "/proc/kbp/fpga"
#define KBP_PCIE_PATH "/proc/kbp/pcie"

#define KBP_PCIE_SIGNAL 10
#define KBP_FPGA_SIGNAL 12

#define KBP_DRIVER_VERSION_MAJOR 1
#define KBP_DRIVER_VERSION_MINOR 7

#define KBP_DRIVER_VERSION_MAJOR_STR "1"
#define KBP_DRIVER_VERSION_MINOR_STR "7"
/**
 * m = memory, c = core, r = register, f = field, d = data.
 */
#if !defined(GET_FIELD) && !defined(SET_FIELD)
#define BRCM_ALIGN(c,r,f)   c##_##r##_##f##_ALIGN
#define BRCM_BITS(c,r,f)    c##_##r##_##f##_BITS
#define BRCM_MASK(c,r,f)    c##_##r##_##f##_MASK
#define BRCM_SHIFT(c,r,f)   c##_##r##_##f##_SHIFT

#define GET_FIELD(m,c,r,f) \
    ((((m) & BRCM_MASK(c,r,f)) >> BRCM_SHIFT(c,r,f)) << BRCM_ALIGN(c,r,f))

#define SET_FIELD(m,c,r,f,d) \
    ((m) = (((m) & ~BRCM_MASK(c,r,f)) | ((((d) >> BRCM_ALIGN(c,r,f)) << \
    BRCM_SHIFT(c,r,f)) & BRCM_MASK(c,r,f))) \
    )

#define SET_TYPE_FIELD(m,c,r,f,d) SET_FIELD(m,c,r,f,c##_##d)
#define SET_NAME_FIELD(m,c,r,f,d) SET_FIELD(m,c,r,f,c##_##r##_##f##_##d)
#define SET_VALUE_FIELD(m,c,r,f,d) SET_FIELD(m,c,r,f,d)

#endif /* GET & SET */

/***************************************************************************
 *icf_pdc_registers
 ***************************************************************************/
#define icf_pdc_registers_DMA_CONTROL            0x00000000 /* [RW] DMA_CONTROL */
#define icf_pdc_registers_TEST_CAPABILITIES      0x00000004 /* [RW] TEST_CAPABILITIES */
#define icf_pdc_registers_ReqBuf_Head            0x00000008 /* [RO] ReqBuf_Head */
#define icf_pdc_registers_RspBuf_Head            0x0000000c /* [RO] RspBuf_Head */
#define icf_pdc_registers_NONDMA_STATUS          0x00000010 /* [RO] NONDMA_STATUS */
#define icf_pdc_registers_EFUSE_1B_CORR_STS      0x00000014 /* [RO] EFUSE_1B_CORR_STS */
#define icf_pdc_registers_EFUSE_2B_ERR_STS       0x00000018 /* [RO] EFUSE_2B_ERR_STS */
#define icf_pdc_registers_PDC_DEBUG_STS          0x0000001c /* [RO] PDC_DEBUG_STS */
#define icf_pdc_registers_PCIE_IP_CTRL_LO        0x00000020 /* [RO] PCIE_IP_CTRL_LO */
#define icf_pdc_registers_PCIE_IP_CTRL_HI        0x00000024 /* [RO] PCIE_IP_CTRL_HI */
#define icf_pdc_registers_REQ_Q_LBASE            0x00000100 /* [RW] REQ_Q_LBASE */
#define icf_pdc_registers_REQ_Q_HBASE            0x00000104 /* [RW] REQ_Q_HBASE */
#define icf_pdc_registers_REQ_H_LBASE            0x00000108 /* [RW] REQ_H_LBASE */
#define icf_pdc_registers_REQ_H_HBASE            0x0000010c /* [RW] REQ_H_HBASE */
#define icf_pdc_registers_REQ_T_LBASE            0x00000110 /* [RW] REQ_T_LBASE */
#define icf_pdc_registers_REQ_T_HBASE            0x00000114 /* [RW] REQ_T_HBASE */
#define icf_pdc_registers_REQ_Q_TIMER            0x00000118 /* [RW] REQ_Q_TIMER */
#define icf_pdc_registers_REQ_Q_CTRL             0x0000011c /* [RW] REQ_Q_CTRL */
#define icf_pdc_registers_RSP_Q_LBASE            0x00000200 /* [RW] RSP_Q_LBASE */
#define icf_pdc_registers_RSP_Q_HBASE            0x00000204 /* [RW] RSP_Q_HBASE */
#define icf_pdc_registers_RSP_H_LBASE            0x00000208 /* [RW] RSP_H_LBASE */
#define icf_pdc_registers_RSP_H_HBASE            0x0000020c /* [RW] RSP_H_HBASE */
#define icf_pdc_registers_RSP_T_LBASE            0x00000210 /* [RW] RSP_T_LBASE */
#define icf_pdc_registers_RSP_T_HBASE            0x00000214 /* [RW] RSP_T_HBASE */
#define icf_pdc_registers_RSP_Q_TIMER            0x00000218 /* [RW] RSP_Q_TIMER */
#define icf_pdc_registers_RSP_Q_CTRL             0x0000021c /* [RW] RSP_Q_CTRL */
#define icf_pdc_registers_SAT_M_TIMER_LO         0x00000220 /* [RW] SAT_M_TIMER_LO */
#define icf_pdc_registers_SAT_M_TIMER_HI         0x00000224 /* [RW] SAT_M_TIMER_HI */
#define icf_pdc_registers_INTR_ENABLE            0x00000228 /* [RW] INTR_ENABLE */
#define icf_pdc_registers_INTR_CLEAR             0x0000022c /* [RW] INTR_CLEAR */
#define icf_pdc_registers_PDC_INTR               0x00000230 /* [RO] PDC_INTR */
#define icf_pdc_registers_PDC_REQBUF_TAIL_OVRD   0x00000234 /* [RW] PDC_REQBUF_TAIL_OVRD */
#define icf_pdc_registers_PDC_ERROR_STS_0        0x00000300 /* [RO] PDC_ERROR_STS_0 */
#define icf_pdc_registers_PDC_ERROR_STS_1        0x00000304 /* [RO] PDC_ERROR_STS_1 */
#define icf_pdc_registers_PDC_ERROR_STS_2        0x00000308 /* [RO] PDC_ERROR_STS_2 */
#define icf_pdc_registers_PDC_ERROR_STS_3        0x0000030c /* [RO] PDC_ERROR_STS_3 */
#define icf_pdc_registers_PDC_ERROR_STS_4        0x00000310 /* [RO] PDC_ERROR_STS_4 */
#define icf_pdc_registers_PDC_ERROR_STS_5        0x00000314 /* [RO] PDC_ERROR_STS_5 */
#define icf_pdc_registers_PDC_ERROR_STS_6        0x00000318 /* [RO] PDC_ERROR_STS_6 */
#define icf_pdc_registers_PDC_ALERT_STS_0        0x00000340 /* [RO] PDC_ALERT_STS_0 */
#define icf_pdc_registers_PDC_ALERT_STS_1        0x00000344 /* [RO] PDC_ALERT_STS_1 */
#define icf_pdc_registers_PDC_ALERT_STS_2        0x00000348 /* [RO] PDC_ALERT_STS_2 */
#define icf_pdc_registers_PCIE_TEST_BUS_0        0x00000380 /* [RO] PCIE_TEST_BUS_0 */
#define icf_pdc_registers_PCIE_TEST_BUS_1        0x00000384 /* [RO] PCIE_TEST_BUS_1 */

/***************************************************************************
 *DMA_CONTROL - DMA_CONTROL
 ***************************************************************************/
/* icf_pdc_registers :: DMA_CONTROL :: reserved0 [31:25] */
#define icf_pdc_registers_DMA_CONTROL_reserved0_MASK               0xfe000000
#define icf_pdc_registers_DMA_CONTROL_reserved0_ALIGN              0
#define icf_pdc_registers_DMA_CONTROL_reserved0_BITS               7
#define icf_pdc_registers_DMA_CONTROL_reserved0_SHIFT              25

/* icf_pdc_registers :: DMA_CONTROL :: pram_quad_sel [24:16] */
#define icf_pdc_registers_DMA_CONTROL_pram_quad_sel_MASK           0x01ff0000
#define icf_pdc_registers_DMA_CONTROL_pram_quad_sel_ALIGN          0
#define icf_pdc_registers_DMA_CONTROL_pram_quad_sel_BITS           9
#define icf_pdc_registers_DMA_CONTROL_pram_quad_sel_SHIFT          16
#define icf_pdc_registers_DMA_CONTROL_pram_quad_sel_DEFAULT        0x000001ff

/* icf_pdc_registers :: DMA_CONTROL :: txdma_buffer_size [15:12] */
#define icf_pdc_registers_DMA_CONTROL_txdma_buffer_size_MASK       0x0000f000
#define icf_pdc_registers_DMA_CONTROL_txdma_buffer_size_ALIGN      0
#define icf_pdc_registers_DMA_CONTROL_txdma_buffer_size_BITS       4
#define icf_pdc_registers_DMA_CONTROL_txdma_buffer_size_SHIFT      12
#define icf_pdc_registers_DMA_CONTROL_txdma_buffer_size_DEFAULT    0x00000000

/* icf_pdc_registers :: DMA_CONTROL :: rxdma_buffer_size [11:08] */
#define icf_pdc_registers_DMA_CONTROL_rxdma_buffer_size_MASK       0x00000f00
#define icf_pdc_registers_DMA_CONTROL_rxdma_buffer_size_ALIGN      0
#define icf_pdc_registers_DMA_CONTROL_rxdma_buffer_size_BITS       4
#define icf_pdc_registers_DMA_CONTROL_rxdma_buffer_size_SHIFT      8
#define icf_pdc_registers_DMA_CONTROL_rxdma_buffer_size_DEFAULT    0x00000000

/* icf_pdc_registers :: DMA_CONTROL :: reserved1 [07:06] */
#define icf_pdc_registers_DMA_CONTROL_reserved1_MASK               0x000000c0
#define icf_pdc_registers_DMA_CONTROL_reserved1_ALIGN              0
#define icf_pdc_registers_DMA_CONTROL_reserved1_BITS               2
#define icf_pdc_registers_DMA_CONTROL_reserved1_SHIFT              6

/* icf_pdc_registers :: DMA_CONTROL :: pram_dma_mode [05:05] */
#define icf_pdc_registers_DMA_CONTROL_pram_dma_mode_MASK           0x00000020
#define icf_pdc_registers_DMA_CONTROL_pram_dma_mode_ALIGN          0
#define icf_pdc_registers_DMA_CONTROL_pram_dma_mode_BITS           1
#define icf_pdc_registers_DMA_CONTROL_pram_dma_mode_SHIFT          5
#define icf_pdc_registers_DMA_CONTROL_pram_dma_mode_DEFAULT        0x00000000

/* icf_pdc_registers :: DMA_CONTROL :: tx_fifo_clear_status [04:04] */
#define icf_pdc_registers_DMA_CONTROL_tx_fifo_clear_status_MASK    0x00000010
#define icf_pdc_registers_DMA_CONTROL_tx_fifo_clear_status_ALIGN   0
#define icf_pdc_registers_DMA_CONTROL_tx_fifo_clear_status_BITS    1
#define icf_pdc_registers_DMA_CONTROL_tx_fifo_clear_status_SHIFT   4
#define icf_pdc_registers_DMA_CONTROL_tx_fifo_clear_status_DEFAULT 0x00000000

/* icf_pdc_registers :: DMA_CONTROL :: rx_fifo_clear_status [03:03] */
#define icf_pdc_registers_DMA_CONTROL_rx_fifo_clear_status_MASK    0x00000008
#define icf_pdc_registers_DMA_CONTROL_rx_fifo_clear_status_ALIGN   0
#define icf_pdc_registers_DMA_CONTROL_rx_fifo_clear_status_BITS    1
#define icf_pdc_registers_DMA_CONTROL_rx_fifo_clear_status_SHIFT   3
#define icf_pdc_registers_DMA_CONTROL_rx_fifo_clear_status_DEFAULT 0x00000000

/* icf_pdc_registers :: DMA_CONTROL :: tx_dma_enable [02:02] */
#define icf_pdc_registers_DMA_CONTROL_tx_dma_enable_MASK           0x00000004
#define icf_pdc_registers_DMA_CONTROL_tx_dma_enable_ALIGN          0
#define icf_pdc_registers_DMA_CONTROL_tx_dma_enable_BITS           1
#define icf_pdc_registers_DMA_CONTROL_tx_dma_enable_SHIFT          2
#define icf_pdc_registers_DMA_CONTROL_tx_dma_enable_DEFAULT        0x00000000

/* icf_pdc_registers :: DMA_CONTROL :: rx_dma_enable [01:01] */
#define icf_pdc_registers_DMA_CONTROL_rx_dma_enable_MASK           0x00000002
#define icf_pdc_registers_DMA_CONTROL_rx_dma_enable_ALIGN          0
#define icf_pdc_registers_DMA_CONTROL_rx_dma_enable_BITS           1
#define icf_pdc_registers_DMA_CONTROL_rx_dma_enable_SHIFT          1
#define icf_pdc_registers_DMA_CONTROL_rx_dma_enable_DEFAULT        0x00000000

/* icf_pdc_registers :: DMA_CONTROL :: instruction_sync [00:00] */
#define icf_pdc_registers_DMA_CONTROL_instruction_sync_MASK        0x00000001
#define icf_pdc_registers_DMA_CONTROL_instruction_sync_ALIGN       0
#define icf_pdc_registers_DMA_CONTROL_instruction_sync_BITS        1
#define icf_pdc_registers_DMA_CONTROL_instruction_sync_SHIFT       0
#define icf_pdc_registers_DMA_CONTROL_instruction_sync_DEFAULT     0x00000000

/***************************************************************************
 *TEST_CAPABILITIES - TEST_CAPABILITIES
 ***************************************************************************/
/* icf_pdc_registers :: TEST_CAPABILITIES :: reserved0 [31:24] */
#define icf_pdc_registers_TEST_CAPABILITIES_reserved0_MASK         0xff000000
#define icf_pdc_registers_TEST_CAPABILITIES_reserved0_ALIGN        0
#define icf_pdc_registers_TEST_CAPABILITIES_reserved0_BITS         8
#define icf_pdc_registers_TEST_CAPABILITIES_reserved0_SHIFT        24

/* icf_pdc_registers :: TEST_CAPABILITIES :: pcie_debug_mux_sel1 [23:20] */
#define icf_pdc_registers_TEST_CAPABILITIES_pcie_debug_mux_sel1_MASK 0x00f00000
#define icf_pdc_registers_TEST_CAPABILITIES_pcie_debug_mux_sel1_ALIGN 0
#define icf_pdc_registers_TEST_CAPABILITIES_pcie_debug_mux_sel1_BITS 4
#define icf_pdc_registers_TEST_CAPABILITIES_pcie_debug_mux_sel1_SHIFT 20
#define icf_pdc_registers_TEST_CAPABILITIES_pcie_debug_mux_sel1_DEFAULT 0x00000000

/* icf_pdc_registers :: TEST_CAPABILITIES :: pcie_debug_mux_sel2 [19:16] */
#define icf_pdc_registers_TEST_CAPABILITIES_pcie_debug_mux_sel2_MASK 0x000f0000
#define icf_pdc_registers_TEST_CAPABILITIES_pcie_debug_mux_sel2_ALIGN 0
#define icf_pdc_registers_TEST_CAPABILITIES_pcie_debug_mux_sel2_BITS 4
#define icf_pdc_registers_TEST_CAPABILITIES_pcie_debug_mux_sel2_SHIFT 16
#define icf_pdc_registers_TEST_CAPABILITIES_pcie_debug_mux_sel2_DEFAULT 0x00000000

/* icf_pdc_registers :: TEST_CAPABILITIES :: reserved1 [15:12] */
#define icf_pdc_registers_TEST_CAPABILITIES_reserved1_MASK         0x0000f000
#define icf_pdc_registers_TEST_CAPABILITIES_reserved1_ALIGN        0
#define icf_pdc_registers_TEST_CAPABILITIES_reserved1_BITS         4
#define icf_pdc_registers_TEST_CAPABILITIES_reserved1_SHIFT        12

/* icf_pdc_registers :: TEST_CAPABILITIES :: rsp_flip_parity [11:08] */
#define icf_pdc_registers_TEST_CAPABILITIES_rsp_flip_parity_MASK   0x00000f00
#define icf_pdc_registers_TEST_CAPABILITIES_rsp_flip_parity_ALIGN  0
#define icf_pdc_registers_TEST_CAPABILITIES_rsp_flip_parity_BITS   4
#define icf_pdc_registers_TEST_CAPABILITIES_rsp_flip_parity_SHIFT  8
#define icf_pdc_registers_TEST_CAPABILITIES_rsp_flip_parity_DEFAULT 0x00000000

/* icf_pdc_registers :: TEST_CAPABILITIES :: req_flip_parity [07:04] */
#define icf_pdc_registers_TEST_CAPABILITIES_req_flip_parity_MASK   0x000000f0
#define icf_pdc_registers_TEST_CAPABILITIES_req_flip_parity_ALIGN  0
#define icf_pdc_registers_TEST_CAPABILITIES_req_flip_parity_BITS   4
#define icf_pdc_registers_TEST_CAPABILITIES_req_flip_parity_SHIFT  4
#define icf_pdc_registers_TEST_CAPABILITIES_req_flip_parity_DEFAULT 0x00000000

/* icf_pdc_registers :: TEST_CAPABILITIES :: reserved2 [03:03] */
#define icf_pdc_registers_TEST_CAPABILITIES_reserved2_MASK         0x00000008
#define icf_pdc_registers_TEST_CAPABILITIES_reserved2_ALIGN        0
#define icf_pdc_registers_TEST_CAPABILITIES_reserved2_BITS         1
#define icf_pdc_registers_TEST_CAPABILITIES_reserved2_SHIFT        3

/* icf_pdc_registers :: TEST_CAPABILITIES :: multi_dma_req_en [02:02] */
#define icf_pdc_registers_TEST_CAPABILITIES_multi_dma_req_en_MASK  0x00000004
#define icf_pdc_registers_TEST_CAPABILITIES_multi_dma_req_en_ALIGN 0
#define icf_pdc_registers_TEST_CAPABILITIES_multi_dma_req_en_BITS  1
#define icf_pdc_registers_TEST_CAPABILITIES_multi_dma_req_en_SHIFT 2
#define icf_pdc_registers_TEST_CAPABILITIES_multi_dma_req_en_DEFAULT 0x00000000

/* icf_pdc_registers :: TEST_CAPABILITIES :: rspbuf_tail_ptr_chk [01:01] */
#define icf_pdc_registers_TEST_CAPABILITIES_rspbuf_tail_ptr_chk_MASK 0x00000002
#define icf_pdc_registers_TEST_CAPABILITIES_rspbuf_tail_ptr_chk_ALIGN 0
#define icf_pdc_registers_TEST_CAPABILITIES_rspbuf_tail_ptr_chk_BITS 1
#define icf_pdc_registers_TEST_CAPABILITIES_rspbuf_tail_ptr_chk_SHIFT 1
#define icf_pdc_registers_TEST_CAPABILITIES_rspbuf_tail_ptr_chk_DEFAULT 0x00000000

/* icf_pdc_registers :: TEST_CAPABILITIES :: reqf_rspf_loopback [00:00] */
#define icf_pdc_registers_TEST_CAPABILITIES_reqf_rspf_loopback_MASK 0x00000001
#define icf_pdc_registers_TEST_CAPABILITIES_reqf_rspf_loopback_ALIGN 0
#define icf_pdc_registers_TEST_CAPABILITIES_reqf_rspf_loopback_BITS 1
#define icf_pdc_registers_TEST_CAPABILITIES_reqf_rspf_loopback_SHIFT 0
#define icf_pdc_registers_TEST_CAPABILITIES_reqf_rspf_loopback_DEFAULT 0x00000000

/***************************************************************************
 *ReqBuf_Head - ReqBuf_Head
 ***************************************************************************/
/* icf_pdc_registers :: ReqBuf_Head :: reserved0 [31:24] */
#define icf_pdc_registers_ReqBuf_Head_reserved0_MASK               0xff000000
#define icf_pdc_registers_ReqBuf_Head_reserved0_ALIGN              0
#define icf_pdc_registers_ReqBuf_Head_reserved0_BITS               8
#define icf_pdc_registers_ReqBuf_Head_reserved0_SHIFT              24

/* icf_pdc_registers :: ReqBuf_Head :: cur_hp_txdma [23:00] */
#define icf_pdc_registers_ReqBuf_Head_cur_hp_txdma_MASK            0x00ffffff
#define icf_pdc_registers_ReqBuf_Head_cur_hp_txdma_ALIGN           0
#define icf_pdc_registers_ReqBuf_Head_cur_hp_txdma_BITS            24
#define icf_pdc_registers_ReqBuf_Head_cur_hp_txdma_SHIFT           0
#define icf_pdc_registers_ReqBuf_Head_cur_hp_txdma_DEFAULT         0x00000000

/***************************************************************************
 *RspBuf_Head - RspBuf_Head
 ***************************************************************************/
/* icf_pdc_registers :: RspBuf_Head :: reserved0 [31:24] */
#define icf_pdc_registers_RspBuf_Head_reserved0_MASK               0xff000000
#define icf_pdc_registers_RspBuf_Head_reserved0_ALIGN              0
#define icf_pdc_registers_RspBuf_Head_reserved0_BITS               8
#define icf_pdc_registers_RspBuf_Head_reserved0_SHIFT              24

/* icf_pdc_registers :: RspBuf_Head :: cur_hp_txdma [23:00] */
#define icf_pdc_registers_RspBuf_Head_cur_hp_txdma_MASK            0x00ffffff
#define icf_pdc_registers_RspBuf_Head_cur_hp_txdma_ALIGN           0
#define icf_pdc_registers_RspBuf_Head_cur_hp_txdma_BITS            24
#define icf_pdc_registers_RspBuf_Head_cur_hp_txdma_SHIFT           0
#define icf_pdc_registers_RspBuf_Head_cur_hp_txdma_DEFAULT         0x00000000

/***************************************************************************
 *NONDMA_STATUS - NONDMA_STATUS
 ***************************************************************************/
/* icf_pdc_registers :: NONDMA_STATUS :: rspf_rd_fill_level [31:24] */
#define icf_pdc_registers_NONDMA_STATUS_rspf_rd_fill_level_MASK    0xff000000
#define icf_pdc_registers_NONDMA_STATUS_rspf_rd_fill_level_ALIGN   0
#define icf_pdc_registers_NONDMA_STATUS_rspf_rd_fill_level_BITS    8
#define icf_pdc_registers_NONDMA_STATUS_rspf_rd_fill_level_SHIFT   24
#define icf_pdc_registers_NONDMA_STATUS_rspf_rd_fill_level_DEFAULT 0x00000000

/* icf_pdc_registers :: NONDMA_STATUS :: rspf_wr_fill_level [23:16] */
#define icf_pdc_registers_NONDMA_STATUS_rspf_wr_fill_level_MASK    0x00ff0000
#define icf_pdc_registers_NONDMA_STATUS_rspf_wr_fill_level_ALIGN   0
#define icf_pdc_registers_NONDMA_STATUS_rspf_wr_fill_level_BITS    8
#define icf_pdc_registers_NONDMA_STATUS_rspf_wr_fill_level_SHIFT   16
#define icf_pdc_registers_NONDMA_STATUS_rspf_wr_fill_level_DEFAULT 0x00000080

/* icf_pdc_registers :: NONDMA_STATUS :: reqf_rd_fill_level [15:08] */
#define icf_pdc_registers_NONDMA_STATUS_reqf_rd_fill_level_MASK    0x0000ff00
#define icf_pdc_registers_NONDMA_STATUS_reqf_rd_fill_level_ALIGN   0
#define icf_pdc_registers_NONDMA_STATUS_reqf_rd_fill_level_BITS    8
#define icf_pdc_registers_NONDMA_STATUS_reqf_rd_fill_level_SHIFT   8
#define icf_pdc_registers_NONDMA_STATUS_reqf_rd_fill_level_DEFAULT 0x00000000

/* icf_pdc_registers :: NONDMA_STATUS :: reqf_wr_fill_level [07:00] */
#define icf_pdc_registers_NONDMA_STATUS_reqf_wr_fill_level_MASK    0x000000ff
#define icf_pdc_registers_NONDMA_STATUS_reqf_wr_fill_level_ALIGN   0
#define icf_pdc_registers_NONDMA_STATUS_reqf_wr_fill_level_BITS    8
#define icf_pdc_registers_NONDMA_STATUS_reqf_wr_fill_level_SHIFT   0
#define icf_pdc_registers_NONDMA_STATUS_reqf_wr_fill_level_DEFAULT 0x00000080

/***************************************************************************
 *EFUSE_1B_CORR_STS - EFUSE_1B_CORR_STS
 ***************************************************************************/
/* icf_pdc_registers :: EFUSE_1B_CORR_STS :: ecc_1b_corr_sts [31:00] */
#define icf_pdc_registers_EFUSE_1B_CORR_STS_ecc_1b_corr_sts_MASK   0xffffffff
#define icf_pdc_registers_EFUSE_1B_CORR_STS_ecc_1b_corr_sts_ALIGN  0
#define icf_pdc_registers_EFUSE_1B_CORR_STS_ecc_1b_corr_sts_BITS   32
#define icf_pdc_registers_EFUSE_1B_CORR_STS_ecc_1b_corr_sts_SHIFT  0
#define icf_pdc_registers_EFUSE_1B_CORR_STS_ecc_1b_corr_sts_DEFAULT 0x00000000

/***************************************************************************
 *EFUSE_2B_ERR_STS - EFUSE_2B_ERR_STS
 ***************************************************************************/
/* icf_pdc_registers :: EFUSE_2B_ERR_STS :: ecc_2b_err_sts [31:00] */
#define icf_pdc_registers_EFUSE_2B_ERR_STS_ecc_2b_err_sts_MASK     0xffffffff
#define icf_pdc_registers_EFUSE_2B_ERR_STS_ecc_2b_err_sts_ALIGN    0
#define icf_pdc_registers_EFUSE_2B_ERR_STS_ecc_2b_err_sts_BITS     32
#define icf_pdc_registers_EFUSE_2B_ERR_STS_ecc_2b_err_sts_SHIFT    0
#define icf_pdc_registers_EFUSE_2B_ERR_STS_ecc_2b_err_sts_DEFAULT  0x00000000

/***************************************************************************
 *PDC_DEBUG_STS - PDC_DEBUG_STS
 ***************************************************************************/
/* icf_pdc_registers :: PDC_DEBUG_STS :: reserved0 [31:04] */
#define icf_pdc_registers_PDC_DEBUG_STS_reserved0_MASK             0xfffffff0
#define icf_pdc_registers_PDC_DEBUG_STS_reserved0_ALIGN            0
#define icf_pdc_registers_PDC_DEBUG_STS_reserved0_BITS             28
#define icf_pdc_registers_PDC_DEBUG_STS_reserved0_SHIFT            4

/* icf_pdc_registers :: PDC_DEBUG_STS :: efuse_programming_done [03:03] */
#define icf_pdc_registers_PDC_DEBUG_STS_efuse_programming_done_MASK 0x00000008
#define icf_pdc_registers_PDC_DEBUG_STS_efuse_programming_done_ALIGN 0
#define icf_pdc_registers_PDC_DEBUG_STS_efuse_programming_done_BITS 1
#define icf_pdc_registers_PDC_DEBUG_STS_efuse_programming_done_SHIFT 3
#define icf_pdc_registers_PDC_DEBUG_STS_efuse_programming_done_DEFAULT 0x00000000

/* icf_pdc_registers :: PDC_DEBUG_STS :: pcie_perst_assert [02:02] */
#define icf_pdc_registers_PDC_DEBUG_STS_pcie_perst_assert_MASK     0x00000004
#define icf_pdc_registers_PDC_DEBUG_STS_pcie_perst_assert_ALIGN    0
#define icf_pdc_registers_PDC_DEBUG_STS_pcie_perst_assert_BITS     1
#define icf_pdc_registers_PDC_DEBUG_STS_pcie_perst_assert_SHIFT    2
#define icf_pdc_registers_PDC_DEBUG_STS_pcie_perst_assert_DEFAULT  0x00000000

/* icf_pdc_registers :: PDC_DEBUG_STS :: core_init_done [01:01] */
#define icf_pdc_registers_PDC_DEBUG_STS_core_init_done_MASK        0x00000002
#define icf_pdc_registers_PDC_DEBUG_STS_core_init_done_ALIGN       0
#define icf_pdc_registers_PDC_DEBUG_STS_core_init_done_BITS        1
#define icf_pdc_registers_PDC_DEBUG_STS_core_init_done_SHIFT       1
#define icf_pdc_registers_PDC_DEBUG_STS_core_init_done_DEFAULT     0x00000000

/* icf_pdc_registers :: PDC_DEBUG_STS :: satbus_pcie_ctrl_cscrs [00:00] */
#define icf_pdc_registers_PDC_DEBUG_STS_satbus_pcie_ctrl_cscrs_MASK 0x00000001
#define icf_pdc_registers_PDC_DEBUG_STS_satbus_pcie_ctrl_cscrs_ALIGN 0
#define icf_pdc_registers_PDC_DEBUG_STS_satbus_pcie_ctrl_cscrs_BITS 1
#define icf_pdc_registers_PDC_DEBUG_STS_satbus_pcie_ctrl_cscrs_SHIFT 0
#define icf_pdc_registers_PDC_DEBUG_STS_satbus_pcie_ctrl_cscrs_DEFAULT 0x00000000

/***************************************************************************
 *PCIE_IP_CTRL_LO - PCIE_IP_CTRL_LO
 ***************************************************************************/
/* icf_pdc_registers :: PCIE_IP_CTRL_LO :: user_subsystem_id [31:16] */
#define icf_pdc_registers_PCIE_IP_CTRL_LO_user_subsystem_id_MASK   0xffff0000
#define icf_pdc_registers_PCIE_IP_CTRL_LO_user_subsystem_id_ALIGN  0
#define icf_pdc_registers_PCIE_IP_CTRL_LO_user_subsystem_id_BITS   16
#define icf_pdc_registers_PCIE_IP_CTRL_LO_user_subsystem_id_SHIFT  16
#define icf_pdc_registers_PCIE_IP_CTRL_LO_user_subsystem_id_DEFAULT 0x00000000

/* icf_pdc_registers :: PCIE_IP_CTRL_LO :: user_device_id [15:00] */
#define icf_pdc_registers_PCIE_IP_CTRL_LO_user_device_id_MASK      0x0000ffff
#define icf_pdc_registers_PCIE_IP_CTRL_LO_user_device_id_ALIGN     0
#define icf_pdc_registers_PCIE_IP_CTRL_LO_user_device_id_BITS      16
#define icf_pdc_registers_PCIE_IP_CTRL_LO_user_device_id_SHIFT     0
#define icf_pdc_registers_PCIE_IP_CTRL_LO_user_device_id_DEFAULT   0x00009800

/***************************************************************************
 *PCIE_IP_CTRL_HI - PCIE_IP_CTRL_HI
 ***************************************************************************/
/* icf_pdc_registers :: PCIE_IP_CTRL_HI :: misc_control [31:08] */
#define icf_pdc_registers_PCIE_IP_CTRL_HI_misc_control_MASK        0xffffff00
#define icf_pdc_registers_PCIE_IP_CTRL_HI_misc_control_ALIGN       0
#define icf_pdc_registers_PCIE_IP_CTRL_HI_misc_control_BITS        24
#define icf_pdc_registers_PCIE_IP_CTRL_HI_misc_control_SHIFT       8
#define icf_pdc_registers_PCIE_IP_CTRL_HI_misc_control_DEFAULT     0x00000000

/* icf_pdc_registers :: PCIE_IP_CTRL_HI :: user_revision_id [07:00] */
#define icf_pdc_registers_PCIE_IP_CTRL_HI_user_revision_id_MASK    0x000000ff
#define icf_pdc_registers_PCIE_IP_CTRL_HI_user_revision_id_ALIGN   0
#define icf_pdc_registers_PCIE_IP_CTRL_HI_user_revision_id_BITS    8
#define icf_pdc_registers_PCIE_IP_CTRL_HI_user_revision_id_SHIFT   0
#define icf_pdc_registers_PCIE_IP_CTRL_HI_user_revision_id_DEFAULT 0x00000000

/***************************************************************************
 *REQ_Q_LBASE - REQ_Q_LBASE
 ***************************************************************************/
/* icf_pdc_registers :: REQ_Q_LBASE :: address [31:00] */
#define icf_pdc_registers_REQ_Q_LBASE_address_MASK                 0xffffffff
#define icf_pdc_registers_REQ_Q_LBASE_address_ALIGN                0
#define icf_pdc_registers_REQ_Q_LBASE_address_BITS                 32
#define icf_pdc_registers_REQ_Q_LBASE_address_SHIFT                0
#define icf_pdc_registers_REQ_Q_LBASE_address_DEFAULT              0x00000000

/***************************************************************************
 *REQ_Q_HBASE - REQ_Q_HBASE
 ***************************************************************************/
/* icf_pdc_registers :: REQ_Q_HBASE :: address [31:00] */
#define icf_pdc_registers_REQ_Q_HBASE_address_MASK                 0xffffffff
#define icf_pdc_registers_REQ_Q_HBASE_address_ALIGN                0
#define icf_pdc_registers_REQ_Q_HBASE_address_BITS                 32
#define icf_pdc_registers_REQ_Q_HBASE_address_SHIFT                0
#define icf_pdc_registers_REQ_Q_HBASE_address_DEFAULT              0x00000000

/***************************************************************************
 *REQ_H_LBASE - REQ_H_LBASE
 ***************************************************************************/
/* icf_pdc_registers :: REQ_H_LBASE :: address [31:00] */
#define icf_pdc_registers_REQ_H_LBASE_address_MASK                 0xffffffff
#define icf_pdc_registers_REQ_H_LBASE_address_ALIGN                0
#define icf_pdc_registers_REQ_H_LBASE_address_BITS                 32
#define icf_pdc_registers_REQ_H_LBASE_address_SHIFT                0
#define icf_pdc_registers_REQ_H_LBASE_address_DEFAULT              0x00000000

/***************************************************************************
 *REQ_H_HBASE - REQ_H_HBASE
 ***************************************************************************/
/* icf_pdc_registers :: REQ_H_HBASE :: address [31:00] */
#define icf_pdc_registers_REQ_H_HBASE_address_MASK                 0xffffffff
#define icf_pdc_registers_REQ_H_HBASE_address_ALIGN                0
#define icf_pdc_registers_REQ_H_HBASE_address_BITS                 32
#define icf_pdc_registers_REQ_H_HBASE_address_SHIFT                0
#define icf_pdc_registers_REQ_H_HBASE_address_DEFAULT              0x00000000

/***************************************************************************
 *REQ_T_LBASE - REQ_T_LBASE
 ***************************************************************************/
/* icf_pdc_registers :: REQ_T_LBASE :: address [31:00] */
#define icf_pdc_registers_REQ_T_LBASE_address_MASK                 0xffffffff
#define icf_pdc_registers_REQ_T_LBASE_address_ALIGN                0
#define icf_pdc_registers_REQ_T_LBASE_address_BITS                 32
#define icf_pdc_registers_REQ_T_LBASE_address_SHIFT                0
#define icf_pdc_registers_REQ_T_LBASE_address_DEFAULT              0x00000000

/***************************************************************************
 *REQ_T_HBASE - REQ_T_HBASE
 ***************************************************************************/
/* icf_pdc_registers :: REQ_T_HBASE :: address [31:00] */
#define icf_pdc_registers_REQ_T_HBASE_address_MASK                 0xffffffff
#define icf_pdc_registers_REQ_T_HBASE_address_ALIGN                0
#define icf_pdc_registers_REQ_T_HBASE_address_BITS                 32
#define icf_pdc_registers_REQ_T_HBASE_address_SHIFT                0
#define icf_pdc_registers_REQ_T_HBASE_address_DEFAULT              0x00000000

/***************************************************************************
 *REQ_Q_TIMER - REQ_Q_TIMER
 ***************************************************************************/
/* icf_pdc_registers :: REQ_Q_TIMER :: timer_val [31:00] */
#define icf_pdc_registers_REQ_Q_TIMER_timer_val_MASK               0xffffffff
#define icf_pdc_registers_REQ_Q_TIMER_timer_val_ALIGN              0
#define icf_pdc_registers_REQ_Q_TIMER_timer_val_BITS               32
#define icf_pdc_registers_REQ_Q_TIMER_timer_val_SHIFT              0
#define icf_pdc_registers_REQ_Q_TIMER_timer_val_DEFAULT            0x000003ff

/***************************************************************************
 *REQ_Q_CTRL - REQ_Q_CTRL
 ***************************************************************************/
/* icf_pdc_registers :: REQ_Q_CTRL :: reserved0 [31:23] */
#define icf_pdc_registers_REQ_Q_CTRL_reserved0_MASK                0xff800000
#define icf_pdc_registers_REQ_Q_CTRL_reserved0_ALIGN               0
#define icf_pdc_registers_REQ_Q_CTRL_reserved0_BITS                9
#define icf_pdc_registers_REQ_Q_CTRL_reserved0_SHIFT               23

/* icf_pdc_registers :: REQ_Q_CTRL :: thresold [22:16] */
#define icf_pdc_registers_REQ_Q_CTRL_thresold_MASK                 0x007f0000
#define icf_pdc_registers_REQ_Q_CTRL_thresold_ALIGN                0
#define icf_pdc_registers_REQ_Q_CTRL_thresold_BITS                 7
#define icf_pdc_registers_REQ_Q_CTRL_thresold_SHIFT                16
#define icf_pdc_registers_REQ_Q_CTRL_thresold_DEFAULT              0x0000001f

/* icf_pdc_registers :: REQ_Q_CTRL :: reserved1 [15:14] */
#define icf_pdc_registers_REQ_Q_CTRL_reserved1_MASK                0x0000c000
#define icf_pdc_registers_REQ_Q_CTRL_reserved1_ALIGN               0
#define icf_pdc_registers_REQ_Q_CTRL_reserved1_BITS                2
#define icf_pdc_registers_REQ_Q_CTRL_reserved1_SHIFT               14

/* icf_pdc_registers :: REQ_Q_CTRL :: min_fill_level [13:08] */
#define icf_pdc_registers_REQ_Q_CTRL_min_fill_level_MASK           0x00003f00
#define icf_pdc_registers_REQ_Q_CTRL_min_fill_level_ALIGN          0
#define icf_pdc_registers_REQ_Q_CTRL_min_fill_level_BITS           6
#define icf_pdc_registers_REQ_Q_CTRL_min_fill_level_SHIFT          8
#define icf_pdc_registers_REQ_Q_CTRL_min_fill_level_DEFAULT        0x00000007

/* icf_pdc_registers :: REQ_Q_CTRL :: reserved2 [07:06] */
#define icf_pdc_registers_REQ_Q_CTRL_reserved2_MASK                0x000000c0
#define icf_pdc_registers_REQ_Q_CTRL_reserved2_ALIGN               0
#define icf_pdc_registers_REQ_Q_CTRL_reserved2_BITS                2
#define icf_pdc_registers_REQ_Q_CTRL_reserved2_SHIFT               6

/* icf_pdc_registers :: REQ_Q_CTRL :: max_burst_len [05:00] */
#define icf_pdc_registers_REQ_Q_CTRL_max_burst_len_MASK            0x0000003f
#define icf_pdc_registers_REQ_Q_CTRL_max_burst_len_ALIGN           0
#define icf_pdc_registers_REQ_Q_CTRL_max_burst_len_BITS            6
#define icf_pdc_registers_REQ_Q_CTRL_max_burst_len_SHIFT           0
#define icf_pdc_registers_REQ_Q_CTRL_max_burst_len_DEFAULT         0x0000001f

/***************************************************************************
 *RSP_Q_LBASE - RSP_Q_LBASE
 ***************************************************************************/
/* icf_pdc_registers :: RSP_Q_LBASE :: address [31:00] */
#define icf_pdc_registers_RSP_Q_LBASE_address_MASK                 0xffffffff
#define icf_pdc_registers_RSP_Q_LBASE_address_ALIGN                0
#define icf_pdc_registers_RSP_Q_LBASE_address_BITS                 32
#define icf_pdc_registers_RSP_Q_LBASE_address_SHIFT                0
#define icf_pdc_registers_RSP_Q_LBASE_address_DEFAULT              0x00000000

/***************************************************************************
 *RSP_Q_HBASE - RSP_Q_HBASE
 ***************************************************************************/
/* icf_pdc_registers :: RSP_Q_HBASE :: address [31:00] */
#define icf_pdc_registers_RSP_Q_HBASE_address_MASK                 0xffffffff
#define icf_pdc_registers_RSP_Q_HBASE_address_ALIGN                0
#define icf_pdc_registers_RSP_Q_HBASE_address_BITS                 32
#define icf_pdc_registers_RSP_Q_HBASE_address_SHIFT                0
#define icf_pdc_registers_RSP_Q_HBASE_address_DEFAULT              0x00000000

/***************************************************************************
 *RSP_H_LBASE - RSP_H_LBASE
 ***************************************************************************/
/* icf_pdc_registers :: RSP_H_LBASE :: address [31:00] */
#define icf_pdc_registers_RSP_H_LBASE_address_MASK                 0xffffffff
#define icf_pdc_registers_RSP_H_LBASE_address_ALIGN                0
#define icf_pdc_registers_RSP_H_LBASE_address_BITS                 32
#define icf_pdc_registers_RSP_H_LBASE_address_SHIFT                0
#define icf_pdc_registers_RSP_H_LBASE_address_DEFAULT              0x00000000

/***************************************************************************
 *RSP_H_HBASE - RSP_H_HBASE
 ***************************************************************************/
/* icf_pdc_registers :: RSP_H_HBASE :: address [31:00] */
#define icf_pdc_registers_RSP_H_HBASE_address_MASK                 0xffffffff
#define icf_pdc_registers_RSP_H_HBASE_address_ALIGN                0
#define icf_pdc_registers_RSP_H_HBASE_address_BITS                 32
#define icf_pdc_registers_RSP_H_HBASE_address_SHIFT                0
#define icf_pdc_registers_RSP_H_HBASE_address_DEFAULT              0x00000000

/***************************************************************************
 *RSP_T_LBASE - RSP_T_LBASE
 ***************************************************************************/
/* icf_pdc_registers :: RSP_T_LBASE :: address [31:00] */
#define icf_pdc_registers_RSP_T_LBASE_address_MASK                 0xffffffff
#define icf_pdc_registers_RSP_T_LBASE_address_ALIGN                0
#define icf_pdc_registers_RSP_T_LBASE_address_BITS                 32
#define icf_pdc_registers_RSP_T_LBASE_address_SHIFT                0
#define icf_pdc_registers_RSP_T_LBASE_address_DEFAULT              0x00000000

/***************************************************************************
 *RSP_T_HBASE - RSP_T_HBASE
 ***************************************************************************/
/* icf_pdc_registers :: RSP_T_HBASE :: address [31:00] */
#define icf_pdc_registers_RSP_T_HBASE_address_MASK                 0xffffffff
#define icf_pdc_registers_RSP_T_HBASE_address_ALIGN                0
#define icf_pdc_registers_RSP_T_HBASE_address_BITS                 32
#define icf_pdc_registers_RSP_T_HBASE_address_SHIFT                0
#define icf_pdc_registers_RSP_T_HBASE_address_DEFAULT              0x00000000

/***************************************************************************
 *RSP_Q_TIMER - RSP_Q_TIMER
 ***************************************************************************/
/* icf_pdc_registers :: RSP_Q_TIMER :: timer_val [31:00] */
#define icf_pdc_registers_RSP_Q_TIMER_timer_val_MASK               0xffffffff
#define icf_pdc_registers_RSP_Q_TIMER_timer_val_ALIGN              0
#define icf_pdc_registers_RSP_Q_TIMER_timer_val_BITS               32
#define icf_pdc_registers_RSP_Q_TIMER_timer_val_SHIFT              0
#define icf_pdc_registers_RSP_Q_TIMER_timer_val_DEFAULT            0x000003ff

/***************************************************************************
 *RSP_Q_CTRL - RSP_Q_CTRL
 ***************************************************************************/
/* icf_pdc_registers :: RSP_Q_CTRL :: reserved0 [31:23] */
#define icf_pdc_registers_RSP_Q_CTRL_reserved0_MASK                0xff800000
#define icf_pdc_registers_RSP_Q_CTRL_reserved0_ALIGN               0
#define icf_pdc_registers_RSP_Q_CTRL_reserved0_BITS                9
#define icf_pdc_registers_RSP_Q_CTRL_reserved0_SHIFT               23

/* icf_pdc_registers :: RSP_Q_CTRL :: thresold [22:16] */
#define icf_pdc_registers_RSP_Q_CTRL_thresold_MASK                 0x007f0000
#define icf_pdc_registers_RSP_Q_CTRL_thresold_ALIGN                0
#define icf_pdc_registers_RSP_Q_CTRL_thresold_BITS                 7
#define icf_pdc_registers_RSP_Q_CTRL_thresold_SHIFT                16
#define icf_pdc_registers_RSP_Q_CTRL_thresold_DEFAULT              0x0000001f

/* icf_pdc_registers :: RSP_Q_CTRL :: reserved1 [15:06] */
#define icf_pdc_registers_RSP_Q_CTRL_reserved1_MASK                0x0000ffc0
#define icf_pdc_registers_RSP_Q_CTRL_reserved1_ALIGN               0
#define icf_pdc_registers_RSP_Q_CTRL_reserved1_BITS                10
#define icf_pdc_registers_RSP_Q_CTRL_reserved1_SHIFT               6

/* icf_pdc_registers :: RSP_Q_CTRL :: max_burst_len [05:00] */
#define icf_pdc_registers_RSP_Q_CTRL_max_burst_len_MASK            0x0000003f
#define icf_pdc_registers_RSP_Q_CTRL_max_burst_len_ALIGN           0
#define icf_pdc_registers_RSP_Q_CTRL_max_burst_len_BITS            6
#define icf_pdc_registers_RSP_Q_CTRL_max_burst_len_SHIFT           0
#define icf_pdc_registers_RSP_Q_CTRL_max_burst_len_DEFAULT         0x0000001f

/***************************************************************************
 *SAT_M_TIMER_LO - SAT_M_TIMER_LO
 ***************************************************************************/
/* icf_pdc_registers :: SAT_M_TIMER_LO :: satm_timer_lo [31:00] */
#define icf_pdc_registers_SAT_M_TIMER_LO_satm_timer_lo_MASK        0xffffffff
#define icf_pdc_registers_SAT_M_TIMER_LO_satm_timer_lo_ALIGN       0
#define icf_pdc_registers_SAT_M_TIMER_LO_satm_timer_lo_BITS        32
#define icf_pdc_registers_SAT_M_TIMER_LO_satm_timer_lo_SHIFT       0
#define icf_pdc_registers_SAT_M_TIMER_LO_satm_timer_lo_DEFAULT     0x0000001f

/***************************************************************************
 *SAT_M_TIMER_HI - SAT_M_TIMER_HI
 ***************************************************************************/
/* icf_pdc_registers :: SAT_M_TIMER_HI :: satm_timer_hi [31:00] */
#define icf_pdc_registers_SAT_M_TIMER_HI_satm_timer_hi_MASK        0xffffffff
#define icf_pdc_registers_SAT_M_TIMER_HI_satm_timer_hi_ALIGN       0
#define icf_pdc_registers_SAT_M_TIMER_HI_satm_timer_hi_BITS        32
#define icf_pdc_registers_SAT_M_TIMER_HI_satm_timer_hi_SHIFT       0
#define icf_pdc_registers_SAT_M_TIMER_HI_satm_timer_hi_DEFAULT     0x00000000

/***************************************************************************
 *INTR_ENABLE - INTR_ENABLE
 ***************************************************************************/
/* icf_pdc_registers :: INTR_ENABLE :: intr_enable [31:00] */
#define icf_pdc_registers_INTR_ENABLE_intr_enable_MASK             0xffffffff
#define icf_pdc_registers_INTR_ENABLE_intr_enable_ALIGN            0
#define icf_pdc_registers_INTR_ENABLE_intr_enable_BITS             32
#define icf_pdc_registers_INTR_ENABLE_intr_enable_SHIFT            0
#define icf_pdc_registers_INTR_ENABLE_intr_enable_DEFAULT          0x00000000

/***************************************************************************
 *INTR_CLEAR - INTR_CLEAR
 ***************************************************************************/
/* icf_pdc_registers :: INTR_CLEAR :: intr_clear [31:00] */
#define icf_pdc_registers_INTR_CLEAR_intr_clear_MASK               0xffffffff
#define icf_pdc_registers_INTR_CLEAR_intr_clear_ALIGN              0
#define icf_pdc_registers_INTR_CLEAR_intr_clear_BITS               32
#define icf_pdc_registers_INTR_CLEAR_intr_clear_SHIFT              0
#define icf_pdc_registers_INTR_CLEAR_intr_clear_DEFAULT            0x00000000

/***************************************************************************
 *PDC_INTR - PDC_INTR
 ***************************************************************************/
/* icf_pdc_registers :: PDC_INTR :: pdc_intr [31:00] */
#define icf_pdc_registers_PDC_INTR_pdc_intr_MASK                   0xffffffff
#define icf_pdc_registers_PDC_INTR_pdc_intr_ALIGN                  0
#define icf_pdc_registers_PDC_INTR_pdc_intr_BITS                   32
#define icf_pdc_registers_PDC_INTR_pdc_intr_SHIFT                  0
#define icf_pdc_registers_PDC_INTR_pdc_intr_DEFAULT                0x00000000

/***************************************************************************
 *PDC_REQBUF_TAIL_OVRD - PDC_REQBUF_TAIL_OVRD
 ***************************************************************************/
/* icf_pdc_registers :: PDC_REQBUF_TAIL_OVRD :: reserved0 [31:24] */
#define icf_pdc_registers_PDC_REQBUF_TAIL_OVRD_reserved0_MASK      0xff000000
#define icf_pdc_registers_PDC_REQBUF_TAIL_OVRD_reserved0_ALIGN     0
#define icf_pdc_registers_PDC_REQBUF_TAIL_OVRD_reserved0_BITS      8
#define icf_pdc_registers_PDC_REQBUF_TAIL_OVRD_reserved0_SHIFT     24

/* icf_pdc_registers :: PDC_REQBUF_TAIL_OVRD :: req_buf_tail_ovrd [23:00] */
#define icf_pdc_registers_PDC_REQBUF_TAIL_OVRD_req_buf_tail_ovrd_MASK 0x00ffffff
#define icf_pdc_registers_PDC_REQBUF_TAIL_OVRD_req_buf_tail_ovrd_ALIGN 0
#define icf_pdc_registers_PDC_REQBUF_TAIL_OVRD_req_buf_tail_ovrd_BITS 24
#define icf_pdc_registers_PDC_REQBUF_TAIL_OVRD_req_buf_tail_ovrd_SHIFT 0
#define icf_pdc_registers_PDC_REQBUF_TAIL_OVRD_req_buf_tail_ovrd_DEFAULT 0x00000000

/***************************************************************************
 *PDC_ERROR_STS_0 - PDC_ERROR_STS_0
 ***************************************************************************/
/* icf_pdc_registers :: PDC_ERROR_STS_0 :: reserved0 [31:11] */
#define icf_pdc_registers_PDC_ERROR_STS_0_reserved0_MASK           0xfffff800
#define icf_pdc_registers_PDC_ERROR_STS_0_reserved0_ALIGN          0
#define icf_pdc_registers_PDC_ERROR_STS_0_reserved0_BITS           21
#define icf_pdc_registers_PDC_ERROR_STS_0_reserved0_SHIFT          11

/* icf_pdc_registers :: PDC_ERROR_STS_0 :: req_fifo_perr_addr [10:00] */
#define icf_pdc_registers_PDC_ERROR_STS_0_req_fifo_perr_addr_MASK  0x000007ff
#define icf_pdc_registers_PDC_ERROR_STS_0_req_fifo_perr_addr_ALIGN 0
#define icf_pdc_registers_PDC_ERROR_STS_0_req_fifo_perr_addr_BITS  11
#define icf_pdc_registers_PDC_ERROR_STS_0_req_fifo_perr_addr_SHIFT 0
#define icf_pdc_registers_PDC_ERROR_STS_0_req_fifo_perr_addr_DEFAULT 0x00000000

/***************************************************************************
 *PDC_ERROR_STS_1 - PDC_ERROR_STS_1
 ***************************************************************************/
/* icf_pdc_registers :: PDC_ERROR_STS_1 :: reserved0 [31:16] */
#define icf_pdc_registers_PDC_ERROR_STS_1_reserved0_MASK           0xffff0000
#define icf_pdc_registers_PDC_ERROR_STS_1_reserved0_ALIGN          0
#define icf_pdc_registers_PDC_ERROR_STS_1_reserved0_BITS           16
#define icf_pdc_registers_PDC_ERROR_STS_1_reserved0_SHIFT          16

/* icf_pdc_registers :: PDC_ERROR_STS_1 :: req_fifo_wr_addr [15:08] */
#define icf_pdc_registers_PDC_ERROR_STS_1_req_fifo_wr_addr_MASK    0x0000ff00
#define icf_pdc_registers_PDC_ERROR_STS_1_req_fifo_wr_addr_ALIGN   0
#define icf_pdc_registers_PDC_ERROR_STS_1_req_fifo_wr_addr_BITS    8
#define icf_pdc_registers_PDC_ERROR_STS_1_req_fifo_wr_addr_SHIFT   8

/* icf_pdc_registers :: PDC_ERROR_STS_1 :: req_fifo_rd_addr [07:00] */
#define icf_pdc_registers_PDC_ERROR_STS_1_req_fifo_rd_addr_MASK    0x000000ff
#define icf_pdc_registers_PDC_ERROR_STS_1_req_fifo_rd_addr_ALIGN   0
#define icf_pdc_registers_PDC_ERROR_STS_1_req_fifo_rd_addr_BITS    8
#define icf_pdc_registers_PDC_ERROR_STS_1_req_fifo_rd_addr_SHIFT   0
#define icf_pdc_registers_PDC_ERROR_STS_1_req_fifo_rd_addr_DEFAULT 0x00000000

/***************************************************************************
 *PDC_ERROR_STS_2 - PDC_ERROR_STS_2
 ***************************************************************************/
/* icf_pdc_registers :: PDC_ERROR_STS_2 :: reserved0 [31:28] */
#define icf_pdc_registers_PDC_ERROR_STS_2_reserved0_MASK           0xf0000000
#define icf_pdc_registers_PDC_ERROR_STS_2_reserved0_ALIGN          0
#define icf_pdc_registers_PDC_ERROR_STS_2_reserved0_BITS           4
#define icf_pdc_registers_PDC_ERROR_STS_2_reserved0_SHIFT          28

/* icf_pdc_registers :: PDC_ERROR_STS_2 :: rsp_fifo_perr [27:24] */
#define icf_pdc_registers_PDC_ERROR_STS_2_rsp_fifo_perr_MASK       0x0f000000
#define icf_pdc_registers_PDC_ERROR_STS_2_rsp_fifo_perr_ALIGN      0
#define icf_pdc_registers_PDC_ERROR_STS_2_rsp_fifo_perr_BITS       4
#define icf_pdc_registers_PDC_ERROR_STS_2_rsp_fifo_perr_SHIFT      24
#define icf_pdc_registers_PDC_ERROR_STS_2_rsp_fifo_perr_DEFAULT    0x00000000

/* icf_pdc_registers :: PDC_ERROR_STS_2 :: rspbuf_perr_ptr [23:00] */
#define icf_pdc_registers_PDC_ERROR_STS_2_rspbuf_perr_ptr_MASK     0x00ffffff
#define icf_pdc_registers_PDC_ERROR_STS_2_rspbuf_perr_ptr_ALIGN    0
#define icf_pdc_registers_PDC_ERROR_STS_2_rspbuf_perr_ptr_BITS     24
#define icf_pdc_registers_PDC_ERROR_STS_2_rspbuf_perr_ptr_SHIFT    0
#define icf_pdc_registers_PDC_ERROR_STS_2_rspbuf_perr_ptr_DEFAULT  0x00000000

/***************************************************************************
 *PDC_ERROR_STS_3 - PDC_ERROR_STS_3
 ***************************************************************************/
/* icf_pdc_registers :: PDC_ERROR_STS_3 :: reserved0 [31:16] */
#define icf_pdc_registers_PDC_ERROR_STS_3_reserved0_MASK           0xffff0000
#define icf_pdc_registers_PDC_ERROR_STS_3_reserved0_ALIGN          0
#define icf_pdc_registers_PDC_ERROR_STS_3_reserved0_BITS           16
#define icf_pdc_registers_PDC_ERROR_STS_3_reserved0_SHIFT          16

/* icf_pdc_registers :: PDC_ERROR_STS_3 :: rsp_fifo_wr_addr [15:08] */
#define icf_pdc_registers_PDC_ERROR_STS_3_rsp_fifo_wr_addr_MASK    0x0000ff00
#define icf_pdc_registers_PDC_ERROR_STS_3_rsp_fifo_wr_addr_ALIGN   0
#define icf_pdc_registers_PDC_ERROR_STS_3_rsp_fifo_wr_addr_BITS    8
#define icf_pdc_registers_PDC_ERROR_STS_3_rsp_fifo_wr_addr_SHIFT   8

/* icf_pdc_registers :: PDC_ERROR_STS_3 :: rsp_fifo_rd_addr [07:00] */
#define icf_pdc_registers_PDC_ERROR_STS_3_rsp_fifo_rd_addr_MASK    0x000000ff
#define icf_pdc_registers_PDC_ERROR_STS_3_rsp_fifo_rd_addr_ALIGN   0
#define icf_pdc_registers_PDC_ERROR_STS_3_rsp_fifo_rd_addr_BITS    8
#define icf_pdc_registers_PDC_ERROR_STS_3_rsp_fifo_rd_addr_SHIFT   0
#define icf_pdc_registers_PDC_ERROR_STS_3_rsp_fifo_rd_addr_DEFAULT 0x00000000

/***************************************************************************
 *PDC_ERROR_STS_4 - PDC_ERROR_STS_4
 ***************************************************************************/
/* icf_pdc_registers :: PDC_ERROR_STS_4 :: reserved0 [31:24] */
#define icf_pdc_registers_PDC_ERROR_STS_4_reserved0_MASK           0xff000000
#define icf_pdc_registers_PDC_ERROR_STS_4_reserved0_ALIGN          0
#define icf_pdc_registers_PDC_ERROR_STS_4_reserved0_BITS           8
#define icf_pdc_registers_PDC_ERROR_STS_4_reserved0_SHIFT          24

/* icf_pdc_registers :: PDC_ERROR_STS_4 :: rspbuf_perr_ptr [23:00] */
#define icf_pdc_registers_PDC_ERROR_STS_4_rspbuf_perr_ptr_MASK     0x00ffffff
#define icf_pdc_registers_PDC_ERROR_STS_4_rspbuf_perr_ptr_ALIGN    0
#define icf_pdc_registers_PDC_ERROR_STS_4_rspbuf_perr_ptr_BITS     24
#define icf_pdc_registers_PDC_ERROR_STS_4_rspbuf_perr_ptr_SHIFT    0
#define icf_pdc_registers_PDC_ERROR_STS_4_rspbuf_perr_ptr_DEFAULT  0x00000000

/***************************************************************************
 *PDC_ERROR_STS_5 - PDC_ERROR_STS_5
 ***************************************************************************/
/* icf_pdc_registers :: PDC_ERROR_STS_5 :: reserved0 [31:26] */
#define icf_pdc_registers_PDC_ERROR_STS_5_reserved0_MASK           0xfc000000
#define icf_pdc_registers_PDC_ERROR_STS_5_reserved0_ALIGN          0
#define icf_pdc_registers_PDC_ERROR_STS_5_reserved0_BITS           6
#define icf_pdc_registers_PDC_ERROR_STS_5_reserved0_SHIFT          26

/* icf_pdc_registers :: PDC_ERROR_STS_5 :: satm_read_cyc [25:25] */
#define icf_pdc_registers_PDC_ERROR_STS_5_satm_read_cyc_MASK       0x02000000
#define icf_pdc_registers_PDC_ERROR_STS_5_satm_read_cyc_ALIGN      0
#define icf_pdc_registers_PDC_ERROR_STS_5_satm_read_cyc_BITS       1
#define icf_pdc_registers_PDC_ERROR_STS_5_satm_read_cyc_SHIFT      25
#define icf_pdc_registers_PDC_ERROR_STS_5_satm_read_cyc_DEFAULT    0x00000000

/* icf_pdc_registers :: PDC_ERROR_STS_5 :: satm_write_cyc [24:24] */
#define icf_pdc_registers_PDC_ERROR_STS_5_satm_write_cyc_MASK      0x01000000
#define icf_pdc_registers_PDC_ERROR_STS_5_satm_write_cyc_ALIGN     0
#define icf_pdc_registers_PDC_ERROR_STS_5_satm_write_cyc_BITS      1
#define icf_pdc_registers_PDC_ERROR_STS_5_satm_write_cyc_SHIFT     24
#define icf_pdc_registers_PDC_ERROR_STS_5_satm_write_cyc_DEFAULT   0x00000000

/* icf_pdc_registers :: PDC_ERROR_STS_5 :: reserved1 [23:21] */
#define icf_pdc_registers_PDC_ERROR_STS_5_reserved1_MASK           0x00e00000
#define icf_pdc_registers_PDC_ERROR_STS_5_reserved1_ALIGN          0
#define icf_pdc_registers_PDC_ERROR_STS_5_reserved1_BITS           3
#define icf_pdc_registers_PDC_ERROR_STS_5_reserved1_SHIFT          21

/* icf_pdc_registers :: PDC_ERROR_STS_5 :: satm_addr [20:00] */
#define icf_pdc_registers_PDC_ERROR_STS_5_satm_addr_MASK           0x001fffff
#define icf_pdc_registers_PDC_ERROR_STS_5_satm_addr_ALIGN          0
#define icf_pdc_registers_PDC_ERROR_STS_5_satm_addr_BITS           21
#define icf_pdc_registers_PDC_ERROR_STS_5_satm_addr_SHIFT          0
#define icf_pdc_registers_PDC_ERROR_STS_5_satm_addr_DEFAULT        0x00000000

/***************************************************************************
 *PDC_ERROR_STS_6 - PDC_ERROR_STS_6
 ***************************************************************************/
/* icf_pdc_registers :: PDC_ERROR_STS_6 :: reserved0 [31:31] */
#define icf_pdc_registers_PDC_ERROR_STS_6_reserved0_MASK           0x80000000
#define icf_pdc_registers_PDC_ERROR_STS_6_reserved0_ALIGN          0
#define icf_pdc_registers_PDC_ERROR_STS_6_reserved0_BITS           1
#define icf_pdc_registers_PDC_ERROR_STS_6_reserved0_SHIFT          31

/* icf_pdc_registers :: PDC_ERROR_STS_6 :: PCIe_trans_type [30:24] */
#define icf_pdc_registers_PDC_ERROR_STS_6_PCIe_trans_type_MASK     0x7f000000
#define icf_pdc_registers_PDC_ERROR_STS_6_PCIe_trans_type_ALIGN    0
#define icf_pdc_registers_PDC_ERROR_STS_6_PCIe_trans_type_BITS     7
#define icf_pdc_registers_PDC_ERROR_STS_6_PCIe_trans_type_SHIFT    24
#define icf_pdc_registers_PDC_ERROR_STS_6_PCIe_trans_type_DEFAULT  0x00000000

/* icf_pdc_registers :: PDC_ERROR_STS_6 :: PDC_access_type [23:20] */
#define icf_pdc_registers_PDC_ERROR_STS_6_PDC_access_type_MASK     0x00f00000
#define icf_pdc_registers_PDC_ERROR_STS_6_PDC_access_type_ALIGN    0
#define icf_pdc_registers_PDC_ERROR_STS_6_PDC_access_type_BITS     4
#define icf_pdc_registers_PDC_ERROR_STS_6_PDC_access_type_SHIFT    20
#define icf_pdc_registers_PDC_ERROR_STS_6_PDC_access_type_DEFAULT  0x00000000

/* icf_pdc_registers :: PDC_ERROR_STS_6 :: reserved1 [19:18] */
#define icf_pdc_registers_PDC_ERROR_STS_6_reserved1_MASK           0x000c0000
#define icf_pdc_registers_PDC_ERROR_STS_6_reserved1_ALIGN          0
#define icf_pdc_registers_PDC_ERROR_STS_6_reserved1_BITS           2
#define icf_pdc_registers_PDC_ERROR_STS_6_reserved1_SHIFT          18

/* icf_pdc_registers :: PDC_ERROR_STS_6 :: PDC_trans_len [17:08] */
#define icf_pdc_registers_PDC_ERROR_STS_6_PDC_trans_len_MASK       0x0003ff00
#define icf_pdc_registers_PDC_ERROR_STS_6_PDC_trans_len_ALIGN      0
#define icf_pdc_registers_PDC_ERROR_STS_6_PDC_trans_len_BITS       10
#define icf_pdc_registers_PDC_ERROR_STS_6_PDC_trans_len_SHIFT      8
#define icf_pdc_registers_PDC_ERROR_STS_6_PDC_trans_len_DEFAULT    0x00000000

/* icf_pdc_registers :: PDC_ERROR_STS_6 :: PDC_trans_type [07:04] */
#define icf_pdc_registers_PDC_ERROR_STS_6_PDC_trans_type_MASK      0x000000f0
#define icf_pdc_registers_PDC_ERROR_STS_6_PDC_trans_type_ALIGN     0
#define icf_pdc_registers_PDC_ERROR_STS_6_PDC_trans_type_BITS      4
#define icf_pdc_registers_PDC_ERROR_STS_6_PDC_trans_type_SHIFT     4
#define icf_pdc_registers_PDC_ERROR_STS_6_PDC_trans_type_DEFAULT   0x00000000

/* icf_pdc_registers :: PDC_ERROR_STS_6 :: PDC_error_value [03:00] */
#define icf_pdc_registers_PDC_ERROR_STS_6_PDC_error_value_MASK     0x0000000f
#define icf_pdc_registers_PDC_ERROR_STS_6_PDC_error_value_ALIGN    0
#define icf_pdc_registers_PDC_ERROR_STS_6_PDC_error_value_BITS     4
#define icf_pdc_registers_PDC_ERROR_STS_6_PDC_error_value_SHIFT    0
#define icf_pdc_registers_PDC_ERROR_STS_6_PDC_error_value_DEFAULT  0x00000000

/***************************************************************************
 *PDC_ALERT_STS_0 - PDC_ALERT_STS_0
 ***************************************************************************/
/* icf_pdc_registers :: PDC_ALERT_STS_0 :: reserved0 [31:08] */
#define icf_pdc_registers_PDC_ALERT_STS_0_reserved0_MASK           0xffffff00
#define icf_pdc_registers_PDC_ALERT_STS_0_reserved0_ALIGN          0
#define icf_pdc_registers_PDC_ALERT_STS_0_reserved0_BITS           24
#define icf_pdc_registers_PDC_ALERT_STS_0_reserved0_SHIFT          8

/* icf_pdc_registers :: PDC_ALERT_STS_0 :: REQ_fifo_Threshold [07:00] */
#define icf_pdc_registers_PDC_ALERT_STS_0_REQ_fifo_Threshold_MASK  0x000000ff
#define icf_pdc_registers_PDC_ALERT_STS_0_REQ_fifo_Threshold_ALIGN 0
#define icf_pdc_registers_PDC_ALERT_STS_0_REQ_fifo_Threshold_BITS  8
#define icf_pdc_registers_PDC_ALERT_STS_0_REQ_fifo_Threshold_SHIFT 0
#define icf_pdc_registers_PDC_ALERT_STS_0_REQ_fifo_Threshold_DEFAULT 0x00000000

/***************************************************************************
 *PDC_ALERT_STS_1 - PDC_ALERT_STS_1
 ***************************************************************************/
/* icf_pdc_registers :: PDC_ALERT_STS_1 :: reserved0 [31:08] */
#define icf_pdc_registers_PDC_ALERT_STS_1_reserved0_MASK           0xffffff00
#define icf_pdc_registers_PDC_ALERT_STS_1_reserved0_ALIGN          0
#define icf_pdc_registers_PDC_ALERT_STS_1_reserved0_BITS           24
#define icf_pdc_registers_PDC_ALERT_STS_1_reserved0_SHIFT          8

/* icf_pdc_registers :: PDC_ALERT_STS_1 :: RSP_fifo_Threshold [07:00] */
#define icf_pdc_registers_PDC_ALERT_STS_1_RSP_fifo_Threshold_MASK  0x000000ff
#define icf_pdc_registers_PDC_ALERT_STS_1_RSP_fifo_Threshold_ALIGN 0
#define icf_pdc_registers_PDC_ALERT_STS_1_RSP_fifo_Threshold_BITS  8
#define icf_pdc_registers_PDC_ALERT_STS_1_RSP_fifo_Threshold_SHIFT 0
#define icf_pdc_registers_PDC_ALERT_STS_1_RSP_fifo_Threshold_DEFAULT 0x00000000

/***************************************************************************
 *PDC_ALERT_STS_2 - PDC_ALERT_STS_2
 ***************************************************************************/
/* icf_pdc_registers :: PDC_ALERT_STS_2 :: RSP_buffer_or_fifo_Threshold [31:00] */
#define icf_pdc_registers_PDC_ALERT_STS_2_RSP_buffer_or_fifo_Threshold_MASK 0xffffffff
#define icf_pdc_registers_PDC_ALERT_STS_2_RSP_buffer_or_fifo_Threshold_ALIGN 0
#define icf_pdc_registers_PDC_ALERT_STS_2_RSP_buffer_or_fifo_Threshold_BITS 32
#define icf_pdc_registers_PDC_ALERT_STS_2_RSP_buffer_or_fifo_Threshold_SHIFT 0
#define icf_pdc_registers_PDC_ALERT_STS_2_RSP_buffer_or_fifo_Threshold_DEFAULT 0x00000000

/***************************************************************************
 *PCIE_TEST_BUS_0 - PCIE_TEST_BUS_0
 ***************************************************************************/
/* icf_pdc_registers :: PCIE_TEST_BUS_0 :: MAC_err_link_sts_debug_vector_pm_state [31:00] */
#define icf_pdc_registers_PCIE_TEST_BUS_0_MAC_err_link_sts_debug_vector_pm_state_MASK 0xffffffff
#define icf_pdc_registers_PCIE_TEST_BUS_0_MAC_err_link_sts_debug_vector_pm_state_ALIGN 0
#define icf_pdc_registers_PCIE_TEST_BUS_0_MAC_err_link_sts_debug_vector_pm_state_BITS 32
#define icf_pdc_registers_PCIE_TEST_BUS_0_MAC_err_link_sts_debug_vector_pm_state_SHIFT 0
#define icf_pdc_registers_PCIE_TEST_BUS_0_MAC_err_link_sts_debug_vector_pm_state_DEFAULT 0x00000000

/***************************************************************************
 *PCIE_TEST_BUS_1 - PCIE_TEST_BUS_1
 ***************************************************************************/
/* icf_pdc_registers :: PCIE_TEST_BUS_1 :: SERDES_testbus_pll_sts_phy_rst [31:00] */
#define icf_pdc_registers_PCIE_TEST_BUS_1_SERDES_testbus_pll_sts_phy_rst_MASK 0xffffffff
#define icf_pdc_registers_PCIE_TEST_BUS_1_SERDES_testbus_pll_sts_phy_rst_ALIGN 0
#define icf_pdc_registers_PCIE_TEST_BUS_1_SERDES_testbus_pll_sts_phy_rst_BITS 32
#define icf_pdc_registers_PCIE_TEST_BUS_1_SERDES_testbus_pll_sts_phy_rst_SHIFT 0
#define icf_pdc_registers_PCIE_TEST_BUS_1_SERDES_testbus_pll_sts_phy_rst_DEFAULT 0x00000000

#ifdef __cplusplus
}
#endif

#endif /* __KBP_DRIVER_H */


