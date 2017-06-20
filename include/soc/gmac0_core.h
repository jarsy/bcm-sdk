/*
 * $Id: gmac0_core.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * gmacdefs - Broadcom gmac (Unimac) specific definitions
 */

#ifndef _gmac0_core_h_
#define _gmac0_core_h_

#ifndef PAD
#define     _PADLINE(line)    pad ## line
#define     _XSTR(line)     _PADLINE(line)
#define     PAD     _XSTR(__LINE__)
#endif

typedef volatile struct _gmac0regs {
    uint32 	devcontrol;
    uint32 	devstatus;
    uint32 	PAD[1];
    uint32 	biststatus;
    uint32 	PAD[4];
    uint32 	intstatus;
    uint32 	intmask;
    uint32 	gptimer;
    uint32 	PAD[1];
    uint32 	rx_ch0_flow_ctrl;
    uint32 	rx_ch1_flow_ctrl;
    uint32 	rx_ch2_flow_ctrl;
    uint32 	rx_ch3_flow_ctrl;
    uint32 	desc_flow_ctrl_ps_stat;
    uint32 	PAD[47];
    uint32 	intrecvlazy;
    uint32 	flowctlthresh;
    uint32 	txqos;
    uint32 	gmac_idle_cnt_thresh;
    uint32 	PAD[4];
    uint32 	fifoaccessaddr;
    uint32 	fifoaccessbyte;
    uint32 	fifoaccessdata;
    uint32 	PAD[1];
    uint32 	irc_cfg;
    uint32 	PAD[1];
    uint32 	erc0_cfg;
    uint32 	erc1_cfg;
    uint32 	erc2_cfg;
    uint32 	erc3_cfg;
    uint32 	PAD[2];
    uint32 	xmt0launch;
    uint32 	xmt1launch;
    uint32 	xmt2launch;
    uint32 	xmt3launch;
    uint32 	PAD[11];
    uint32 	txqctl;
    uint32 	rxqctl;
    uint32 	gpioselect;
    uint32 	gpio_output_en;
    uint32 	PAD[17];
    uint32 	clk_ctl_st;
    uint32 	PAD[1];
    uint32 	pwrctl;
    uint32 	PAD[5];
    uint32 	d64xmt0control;
    uint32 	d64xmt0ptr;
    uint32 	d64xmt0addrlow;
    uint32 	d64xmt0addrhigh;
    uint32 	d64xmt0status0;
    uint32 	d64xmt0status1;
    uint32 	PAD[2];
    uint32 	d64rcv0control;
    uint32 	d64rcv0ptr;
    uint32 	d64rcv0addrlow;
    uint32 	d64rcv0addrhigh;
    uint32 	d64rcv0status0;
    uint32 	d64rcv0status1;
    uint32 	PAD[2];
    uint32 	d64xmt1control;
    uint32 	d64xmt1ptr;
    uint32 	d64xmt1addrlow;
    uint32 	d64xmt1addrhigh;
    uint32 	d64xmt1status0;
    uint32 	d64xmt1status1;
    uint32 	PAD[2];
    uint32 	d64rcv1control;
    uint32 	d64rcv1ptr;
    uint32 	d64rcv1addrlow;
    uint32 	d64rcv1addrhigh;
    uint32 	d64rcv1status0;
    uint32 	d64rcv1status1;
    uint32 	PAD[2];
    uint32 	d64xmt2control;
    uint32 	d64xmt2ptr;
    uint32 	d64xmt2addrlow;
    uint32 	d64xmt2addrhigh;
    uint32 	d64xmt2status0;
    uint32 	d64xmt2status1;
    uint32 	PAD[2];
    uint32 	d64rcv2control;
    uint32 	d64rcv2ptr;
    uint32 	d64rcv2addrlow;
    uint32 	d64rcv2addrhigh;
    uint32 	d64rcv2status0;
    uint32 	d64rcv2status1;
    uint32 	PAD[2];
    uint32 	d64xmt3control;
    uint32 	d64xmt3ptr;
    uint32 	d64xmt3addrlow;
    uint32 	d64xmt3addrhigh;
    uint32 	d64xmt3status0;
    uint32 	d64xmt3status1;
    uint32 	PAD[2];
    uint32 	d64rcv3control;
    uint32 	d64rcv3ptr;
    uint32 	d64rcv3addrlow;
    uint32 	d64rcv3addrhigh;
    uint32 	d64rcv3status0;
    uint32 	d64rcv3status1;
    uint32 	PAD[2];
    uint32 	tx_good_octets;
    uint32 	tx_good_octets_high;
    uint32 	tx_good_pkts;
    uint32 	tx_octets;
    uint32 	tx_octets_high;
    uint32 	tx_pkts;
    uint32 	tx_broadcast_pkts;
    uint32 	tx_multicast_pkts;
    uint32 	tx_uni_pkts;
    uint32 	tx_len_64;
    uint32 	tx_len_65_to_127;
    uint32 	tx_len_128_to_255;
    uint32 	tx_len_256_to_511;
    uint32 	tx_len_512_to_1023;
    uint32 	tx_len_1024_to_max;
    uint32 	tx_len_max_to_jumbo;
    uint32 	tx_jabber_pkts;
    uint32 	tx_oversize_pkts;
    uint32 	tx_fragment_pkts;
    uint32 	tx_underruns;
    uint32 	tx_total_cols;
    uint32 	tx_single_cols;
    uint32 	tx_mulitple_cols;
    uint32 	tx_excessive_cols;
    uint32 	tx_late_cols;
    uint32 	tx_defered;
    uint32 	tx_pause_pkts;
    uint32 	PAD[5];
    uint32 	rx_good_octets;
    uint32 	rx_good_octets_high;
    uint32 	rx_good_pkts;
    uint32 	rx_octets;
    uint32 	rx_octets_high;
    uint32 	rx_pkts;
    uint32 	rx_broadcast_pkts;
    uint32 	rx_multicast_pkts;
    uint32 	rx_uni_pkts;
    uint32 	rx_len_64;
    uint32 	rx_len_65_to_127;
    uint32 	rx_len_128_to_255;
    uint32 	rx_len_256_to_511;
    uint32 	rx_len_512_to_1023;
    uint32 	rx_len_1024_to_max;
    uint32 	rx_len_max_to_jumbo;
    uint32 	rx_jabber_pkts;
    uint32 	rx_oversize_pkts;
    uint32 	rx_fragment_pkts;
    uint32 	rx_missed_pkts;
    uint32 	rx_undersize;
    uint32 	rx_crc_errs;
    uint32 	rx_align_errs;
    uint32 	rx_symbol_errs;
    uint32 	rx_pause_pkts;
    uint32 	rx_nonpause_pkts;
    uint32 	rxq0_irc_drop;
    uint32 	rxq1_irc_drop;
    uint32 	rxq2_irc_drop;
    uint32 	rxq3_irc_drop;
    uint32 	rx_cfp_drop;
    uint32 	PAD[257];
    uint32 	core_version;
    uint32 	ipg_hd_bkp_cntl;
    uint32 	command_config;
    uint32 	mac_0;
    uint32 	mac_1;
    uint32 	frm_length;
    uint32 	pause_quant;
    uint32 	PAD[10];
    uint32 	mac_mode;
    uint32 	PAD[5];
    uint32 	tx_ipg_length;
    uint32 	PAD[174];
    uint32 	ts_status_cntrl;
    uint32 	tx_ts_data;
    uint32 	PAD[4];
    uint32 	pause_control;
    uint32 	flush_control;
    uint32 	rxfifo_stat;
    uint32 	txfifo_stat;
} gmac0regs_t;

/*  devcontrol offset0x0  */
#define 	DEVCONTROL_PV_TAG_SHIFT	20
#define 	DEVCONTROL_PV_TAG_MASK	0x700000  
#define 	DEVCONTROL_RESERVED3_SHIFT	18
#define 	DEVCONTROL_RESERVED3_MASK	0xc0000   
#define 	DEVCONTROL_RESERVED2_SHIFT	16
#define 	DEVCONTROL_RESERVED2_MASK	0x30000   
#define 	DEVCONTROL_TXQ_FLUSH_SHIFT	8
#define 	DEVCONTROL_TXQ_FLUSH_MASK	0x100     
#define 	DEVCONTROL_FLOW_CTRL_MODE_SHIFT	5
#define 	DEVCONTROL_FLOW_CTRL_MODE_MASK	0x60      
#define 	DEVCONTROL_MIB_RESET_ON_READ_SHIFT	4
#define 	DEVCONTROL_MIB_RESET_ON_READ_MASK	0x10      
#define 	DEVCONTROL_CPU_FLOW_CTRL_ON_SHIFT	2
#define 	DEVCONTROL_CPU_FLOW_CTRL_ON_MASK	0x4       
#define 	DEVCONTROL_SW_LINK_SHIFT	1
#define 	DEVCONTROL_SW_LINK_MASK	0x2       
#define 	DEVCONTROL_SW_OVR_SHIFT	0
#define 	DEVCONTROL_SW_OVR_MASK	0x1       

/*  devstatus offset0x4  */
#define 	DEVSTATUS_MII_MODE_SHIFT	8
#define 	DEVSTATUS_MII_MODE_MASK	0x300     
#define 	DEVSTATUS_PAUSE_ON_SHIFT	6
#define 	DEVSTATUS_PAUSE_ON_MASK	0x40      
#define 	DEVSTATUS_TXQ_INFO_FULL_SHIFT	5
#define 	DEVSTATUS_TXQ_INFO_FULL_MASK	0x20      
#define 	DEVSTATUS_TXQ_DATA_FULL_SHIFT	4
#define 	DEVSTATUS_TXQ_DATA_FULL_MASK	0x10      
#define 	DEVSTATUS_TXQ_BURST_FULL_SHIFT	3
#define 	DEVSTATUS_TXQ_BURST_FULL_MASK	0x8       
#define 	DEVSTATUS_RXQ_INFO_FULL_SHIFT	2
#define 	DEVSTATUS_RXQ_INFO_FULL_MASK	0x4       
#define 	DEVSTATUS_RXQ_DATA_FULL_SHIFT	1
#define 	DEVSTATUS_RXQ_DATA_FULL_MASK	0x2       
#define 	DEVSTATUS_RXQ_BURST_FULL_SHIFT	0
#define 	DEVSTATUS_RXQ_BURST_FULL_MASK	0x1       

/*  biststatus offset0xc  */
#define 	BISTSTATUS_UNIMAC_RX_FIFO_SHIFT	10
#define 	BISTSTATUS_UNIMAC_RX_FIFO_MASK	0x400     
#define 	BISTSTATUS_UNIMAC_TX_FIFO_SHIFT	9
#define 	BISTSTATUS_UNIMAC_TX_FIFO_MASK	0x200     
#define 	BISTSTATUS_UNIMAC_RE_TX_FIFO_SHIFT	8
#define 	BISTSTATUS_UNIMAC_RE_TX_FIFO_MASK	0x100     
#define 	BISTSTATUS_RXQ_BURST_FIFO_SHIFT	7
#define 	BISTSTATUS_RXQ_BURST_FIFO_MASK	0x80      
#define 	BISTSTATUS_RXQ_INFO_BUFFER_SHIFT	6
#define 	BISTSTATUS_RXQ_INFO_BUFFER_MASK	0x40      
#define 	BISTSTATUS_RXQ_DATA_BUFFER_SHIFT	5
#define 	BISTSTATUS_RXQ_DATA_BUFFER_MASK	0x20      
#define 	BISTSTATUS_TXQ_BURST_FIFO_SHIFT	4
#define 	BISTSTATUS_TXQ_BURST_FIFO_MASK	0x10      
#define 	BISTSTATUS_TXQ_INFO_BUFFER_SHIFT	3
#define 	BISTSTATUS_TXQ_INFO_BUFFER_MASK	0x8       
#define 	BISTSTATUS_TXQ_DATA_BUFFER_SHIFT	2
#define 	BISTSTATUS_TXQ_DATA_BUFFER_MASK	0x4       
#define 	BISTSTATUS_MIB_RX_FIFO_SHIFT	1
#define 	BISTSTATUS_MIB_RX_FIFO_MASK	0x2       
#define 	BISTSTATUS_MIB_TX_FIFO_SHIFT	0
#define 	BISTSTATUS_MIB_TX_FIFO_MASK	0x1       

/*  intstatus offset0x20  */
#define 	INTSTATUS_XMT3INTERRUPT_SHIFT	27
#define 	INTSTATUS_XMT3INTERRUPT_MASK	0x8000000 
#define 	INTSTATUS_XMT2INTERRUPT_SHIFT	26
#define 	INTSTATUS_XMT2INTERRUPT_MASK	0x4000000 
#define 	INTSTATUS_XMT1INTERRUPT_SHIFT	25
#define 	INTSTATUS_XMT1INTERRUPT_MASK	0x2000000 
#define 	INTSTATUS_XMT0INTERRUPT_SHIFT	24
#define 	INTSTATUS_XMT0INTERRUPT_MASK	0x1000000 
#define 	INTSTATUS_RCVDESCUF_3_SHIFT	23
#define 	INTSTATUS_RCVDESCUF_3_MASK	0x800000  
#define 	INTSTATUS_RCVDESCUF_2_SHIFT	22
#define 	INTSTATUS_RCVDESCUF_2_MASK	0x400000  
#define 	INTSTATUS_RCVDESCUF_1_SHIFT	21
#define 	INTSTATUS_RCVDESCUF_1_MASK	0x200000  
#define 	INTSTATUS_RCVDESCUF_0_SHIFT	20
#define 	INTSTATUS_RCVDESCUF_0_MASK	0x100000  
#define 	INTSTATUS_RCVINTERRUPT_3_SHIFT	19
#define 	INTSTATUS_RCVINTERRUPT_3_MASK	0x80000   
#define 	INTSTATUS_RCVINTERRUPT_2_SHIFT	18
#define 	INTSTATUS_RCVINTERRUPT_2_MASK	0x40000   
#define 	INTSTATUS_RCVINTERRUPT_1_SHIFT	17
#define 	INTSTATUS_RCVINTERRUPT_1_MASK	0x20000   
#define 	INTSTATUS_RCVINTERRUPT_0_SHIFT	16
#define 	INTSTATUS_RCVINTERRUPT_0_MASK	0x10000   
#define 	INTSTATUS_XMTFIFOUNDERFLOW_SHIFT	15
#define 	INTSTATUS_XMTFIFOUNDERFLOW_MASK	0x8000    
#define 	INTSTATUS_RCVFIFOOVERFLOW_SHIFT	14
#define 	INTSTATUS_RCVFIFOOVERFLOW_MASK	0x4000    
#define 	INTSTATUS_TSINT_SHIFT	13
#define 	INTSTATUS_TSINT_MASK	0x2000    
#define 	INTSTATUS_DESCERROR_SHIFT	12
#define 	INTSTATUS_DESCERROR_MASK	0x1000    
#define 	INTSTATUS_PCIDATAERROR_SHIFT	11
#define 	INTSTATUS_PCIDATAERROR_MASK	0x800     
#define 	INTSTATUS_PCIDESCERROR_SHIFT	10
#define 	INTSTATUS_PCIDESCERROR_MASK	0x400     
#define 	INTSTATUS_TIMEOUT_SHIFT	7
#define 	INTSTATUS_TIMEOUT_MASK	0x80      
#define 	INTSTATUS_MIB_TX_SHIFT	6
#define 	INTSTATUS_MIB_TX_MASK	0x40      
#define 	INTSTATUS_MIB_RX_SHIFT	5
#define 	INTSTATUS_MIB_RX_MASK	0x20      
#define 	INTSTATUS_MDIO_SHIFT	4
#define 	INTSTATUS_MDIO_MASK	0x10      
#define 	INTSTATUS_LINK_STAT_SHIFT	3
#define 	INTSTATUS_LINK_STAT_MASK	0x8       
#define 	INTSTATUS_TXQ_FLUSH_DONE_SHIFT	2
#define 	INTSTATUS_TXQ_FLUSH_DONE_MASK	0x4       
#define 	INTSTATUS_MIB_TX_OV_SHIFT	1
#define 	INTSTATUS_MIB_TX_OV_MASK	0x2       
#define 	INTSTATUS_MIB_RX_OV_SHIFT	0
#define 	INTSTATUS_MIB_RX_OV_MASK	0x1       

/*  intmask offset0x24  */
#define 	INTMASK_XMT3INTERRUPT_SHIFT	27
#define 	INTMASK_XMT3INTERRUPT_MASK	0x8000000 
#define 	INTMASK_XMT2INTERRUPT_SHIFT	26
#define 	INTMASK_XMT2INTERRUPT_MASK	0x4000000 
#define 	INTMASK_XMT1INTERRUPT_SHIFT	25
#define 	INTMASK_XMT1INTERRUPT_MASK	0x2000000 
#define 	INTMASK_XMT0INTERRUPT_SHIFT	24
#define 	INTMASK_XMT0INTERRUPT_MASK	0x1000000 
#define 	INTMASK_RCVDESCUF_3_SHIFT	23
#define 	INTMASK_RCVDESCUF_3_MASK	0x800000  
#define 	INTMASK_RCVDESCUF_2_SHIFT	22
#define 	INTMASK_RCVDESCUF_2_MASK	0x400000  
#define 	INTMASK_RCVDESCUF_1_SHIFT	21
#define 	INTMASK_RCVDESCUF_1_MASK	0x200000  
#define 	INTMASK_RCVDESCUF_0_SHIFT	20
#define 	INTMASK_RCVDESCUF_0_MASK	0x100000  
#define 	INTMASK_RCVINTERRUPT_3_SHIFT	19
#define 	INTMASK_RCVINTERRUPT_3_MASK	0x80000   
#define 	INTMASK_RCVINTERRUPT_2_SHIFT	18
#define 	INTMASK_RCVINTERRUPT_2_MASK	0x40000   
#define 	INTMASK_RCVINTERRUPT_1_SHIFT	17
#define 	INTMASK_RCVINTERRUPT_1_MASK	0x20000   
#define 	INTMASK_RCVINTERRUPT_0_SHIFT	16
#define 	INTMASK_RCVINTERRUPT_0_MASK	0x10000   
#define 	INTMASK_XMTFIFOUNDERFLOW_SHIFT	15
#define 	INTMASK_XMTFIFOUNDERFLOW_MASK	0x8000    
#define 	INTMASK_RCVFIFOOVERFLOW_SHIFT	14
#define 	INTMASK_RCVFIFOOVERFLOW_MASK	0x4000    
#define 	INTMASK_TSINTEN_SHIFT	13
#define 	INTMASK_TSINTEN_MASK	0x2000    
#define 	INTMASK_DESCERROR_SHIFT	12
#define 	INTMASK_DESCERROR_MASK	0x1000    
#define 	INTMASK_PCIDATAERROR_SHIFT	11
#define 	INTMASK_PCIDATAERROR_MASK	0x800     
#define 	INTMASK_PCIDESCERROR_SHIFT	10
#define 	INTMASK_PCIDESCERROR_MASK	0x400     
#define 	INTMASK_TIMEOUT_SHIFT	7
#define 	INTMASK_TIMEOUT_MASK	0x80      
#define 	INTMASK_MIB_TX_SHIFT	6
#define 	INTMASK_MIB_TX_MASK	0x40      
#define 	INTMASK_MIB_RX_SHIFT	5
#define 	INTMASK_MIB_RX_MASK	0x20      
#define 	INTMASK_MDIO_SHIFT	4
#define 	INTMASK_MDIO_MASK	0x10      
#define 	INTMASK_LINK_STAT_SHIFT	3
#define 	INTMASK_LINK_STAT_MASK	0x8       
#define 	INTMASK_TXQ_FLUSH_DONE_SHIFT	2
#define 	INTMASK_TXQ_FLUSH_DONE_MASK	0x4       
#define 	INTMASK_MIB_TX_OV_SHIFT	1
#define 	INTMASK_MIB_TX_OV_MASK	0x2       
#define 	INTMASK_MIB_RX_OV_SHIFT	0
#define 	INTMASK_MIB_RX_OV_MASK	0x1       

/*  gptimer offset0x28  */
#define 	GPTIMER_GPTIMER_SHIFT	0
#define 	GPTIMER_GPTIMER_MASK	0xffffffff

/*  rx_ch0_flow_ctrl offset0x30  */
#define 	RX_CH0_FLOW_CTRL_FLOWCNTLONTH_SHIFT	0
#define 	RX_CH0_FLOW_CTRL_FLOWCNTLONTH_MASK	0x1ff     
#define 	RX_CH0_FLOW_CTRL_FLOWCNTLOFFTH_SHIFT	16
#define 	RX_CH0_FLOW_CTRL_FLOWCNTLOFFTH_MASK	0x1ff0000 

/*  rx_ch1_flow_ctrl offset0x34  */
#define 	RX_CH1_FLOW_CTRL_FLOWCNTLONTH_SHIFT	0
#define 	RX_CH1_FLOW_CTRL_FLOWCNTLONTH_MASK	0x1ff     
#define 	RX_CH1_FLOW_CTRL_FLOWCNTLOFFTH_SHIFT	16
#define 	RX_CH1_FLOW_CTRL_FLOWCNTLOFFTH_MASK	0x1ff0000 

/*  rx_ch2_flow_ctrl offset0x38  */
#define 	RX_CH2_FLOW_CTRL_FLOWCNTLONTH_SHIFT	0
#define 	RX_CH2_FLOW_CTRL_FLOWCNTLONTH_MASK	0x1ff     
#define 	RX_CH2_FLOW_CTRL_FLOWCNTLOFFTH_SHIFT	16
#define 	RX_CH2_FLOW_CTRL_FLOWCNTLOFFTH_MASK	0x1ff0000 

/*  rx_ch3_flow_ctrl offset0x3c  */
#define 	RX_CH3_FLOW_CTRL_FLOWCNTLONTH_SHIFT	0
#define 	RX_CH3_FLOW_CTRL_FLOWCNTLONTH_MASK	0x1ff     
#define 	RX_CH3_FLOW_CTRL_FLOWCNTLOFFTH_SHIFT	16
#define 	RX_CH3_FLOW_CTRL_FLOWCNTLOFFTH_MASK	0x1ff0000 

/*  desc_flow_ctrl_ps_stat offset0x40  */
#define 	DESC_FLOW_CTRL_PS_STAT_PS_STAT_SHIFT	0
#define 	DESC_FLOW_CTRL_PS_STAT_PS_STAT_MASK	0xf       

/*  intrecvlazy offset0x100  */
#define 	INTRECVLAZY_FRAME_COUNT_SHIFT	24
#define 	INTRECVLAZY_FRAME_COUNT_MASK	0xff000000
#define 	INTRECVLAZY_TIME_OUT_SHIFT	0
#define 	INTRECVLAZY_TIME_OUT_MASK	0xffffff  

/*  flowctlthresh offset0x104  */
#define 	FLOWCTLTHRESH_OFF_THRESH_SHIFT	16
#define 	FLOWCTLTHRESH_OFF_THRESH_MASK	0xfff0000 
#define 	FLOWCTLTHRESH_ON_THRESH_SHIFT	0
#define 	FLOWCTLTHRESH_ON_THRESH_MASK	0xfff     

/*  txqos offset0x108  */
#define 	TXQOS_TXQOS_GRNL_SHIFT	26
#define 	TXQOS_TXQOS_GRNL_MASK	0x4000000 
#define 	TXQOS_TXQOS_POLICY_SHIFT	24
#define 	TXQOS_TXQOS_POLICY_MASK	0x3000000 
#define 	TXQOS_TXQOS_WEIGHT2_SHIFT	16
#define 	TXQOS_TXQOS_WEIGHT2_MASK	0xff0000  
#define 	TXQOS_TXQOS_WEIGHT1_SHIFT	8
#define 	TXQOS_TXQOS_WEIGHT1_MASK	0xff00    
#define 	TXQOS_TXQOS_WEIGHT0_SHIFT	0
#define 	TXQOS_TXQOS_WEIGHT0_MASK	0xff      

/*  gmac_idle_cnt_thresh offset0x10c  */
#define 	GMAC_IDLE_CNT_THRESH_TX_IDLE_TH_SHIFT	8
#define 	GMAC_IDLE_CNT_THRESH_TX_IDLE_TH_MASK	0xff00    
#define 	GMAC_IDLE_CNT_THRESH_RX_IDLE_TH_SHIFT	0
#define 	GMAC_IDLE_CNT_THRESH_RX_IDLE_TH_MASK	0xff      

/*  fifoaccessaddr offset0x120  */
#define 	FIFOACCESSADDR_SELECT_SHIFT	16
#define 	FIFOACCESSADDR_SELECT_MASK	0xf0000   
#define 	FIFOACCESSADDR_OFFSETX_SHIFT	0
#define 	FIFOACCESSADDR_OFFSETX_MASK	0xffff    

/*  fifoaccessbyte offset0x124  */
#define 	FIFOACCESSBYTE_BYTEENABLE_SHIFT	0
#define 	FIFOACCESSBYTE_BYTEENABLE_MASK	0xf       

/*  fifoaccessdata offset0x128  */
#define 	FIFOACCESSDATA_FIFODATA_SHIFT	0
#define 	FIFOACCESSDATA_FIFODATA_MASK	0xffffffff

/*  irc_cfg offset0x130  */
#define 	IRC_CFG_IRC3_IDX_SHIFT	24
#define 	IRC_CFG_IRC3_IDX_MASK	0x3f000000
#define 	IRC_CFG_IRC2_IDX_SHIFT	16
#define 	IRC_CFG_IRC2_IDX_MASK	0x3f0000  
#define 	IRC_CFG_IRC1_IDX_SHIFT	8
#define 	IRC_CFG_IRC1_IDX_MASK	0x3f00    
#define 	IRC_CFG_IRC0_IDX_SHIFT	0
#define 	IRC_CFG_IRC0_IDX_MASK	0x3f      

/*  erc0_cfg offset0x138  */
#define 	ERC0_CFG_ENABLE_SHIFT	24
#define 	ERC0_CFG_ENABLE_MASK	0x1000000 
#define 	ERC0_CFG_BURSTACC_SHIFT	23
#define 	ERC0_CFG_BURSTACC_MASK	0x800000  
#define 	ERC0_CFG_BKTSIZE_SHIFT	15
#define 	ERC0_CFG_BKTSIZE_MASK	0x7f8000  
#define 	ERC0_CFG_RESERVED_SHIFT	14
#define 	ERC0_CFG_RESERVED_MASK	0x4000    
#define 	ERC0_CFG_RFSHCNT_SHIFT	0
#define 	ERC0_CFG_RFSHCNT_MASK	0x3fff    

/*  erc1_cfg offset0x13c  */
#define 	ERC1_CFG_ENABLE_SHIFT	24
#define 	ERC1_CFG_ENABLE_MASK	0x1000000 
#define 	ERC1_CFG_BURSTACC_SHIFT	23
#define 	ERC1_CFG_BURSTACC_MASK	0x800000  
#define 	ERC1_CFG_BKTSIZE_SHIFT	15
#define 	ERC1_CFG_BKTSIZE_MASK	0x7f8000  
#define 	ERC1_CFG_RFSHCNT_SHIFT	0
#define 	ERC1_CFG_RFSHCNT_MASK	0x7fff    

/*  erc2_cfg offset0x140  */
#define 	ERC2_CFG_ENABLE_SHIFT	24
#define 	ERC2_CFG_ENABLE_MASK	0x1000000 
#define 	ERC2_CFG_BURSTACC_SHIFT	23
#define 	ERC2_CFG_BURSTACC_MASK	0x800000  
#define 	ERC2_CFG_BKTSIZE_SHIFT	15
#define 	ERC2_CFG_BKTSIZE_MASK	0x7f8000  
#define 	ERC2_CFG_RFSHCNT_SHIFT	0
#define 	ERC2_CFG_RFSHCNT_MASK	0x7fff    

/*  erc3_cfg offset0x144  */
#define 	ERC3_CFG_ENABLE_SHIFT	24
#define 	ERC3_CFG_ENABLE_MASK	0x1000000 
#define 	ERC3_CFG_BURSTACC_SHIFT	23
#define 	ERC3_CFG_BURSTACC_MASK	0x800000  
#define 	ERC3_CFG_BKTSIZE_SHIFT	15
#define 	ERC3_CFG_BKTSIZE_MASK	0x7f8000  
#define 	ERC3_CFG_RFSHCNT_SHIFT	0
#define 	ERC3_CFG_RFSHCNT_MASK	0x7fff    

/*  xmt0launch offset0x150  */
#define 	XMT0LAUNCH_LAUNCH_TIME_SHIFT	0
#define 	XMT0LAUNCH_LAUNCH_TIME_MASK	0xffffffff

/*  xmt1launch offset0x154  */
#define 	XMT1LAUNCH_LAUNCH_TIME_SHIFT	0
#define 	XMT1LAUNCH_LAUNCH_TIME_MASK	0xffffffff

/*  xmt2launch offset0x158  */
#define 	XMT2LAUNCH_LAUNCH_TIME_SHIFT	0
#define 	XMT2LAUNCH_LAUNCH_TIME_MASK	0xffffffff

/*  xmt3launch offset0x15c  */
#define 	XMT3LAUNCH_LAUNCH_TIME_SHIFT	0
#define 	XMT3LAUNCH_LAUNCH_TIME_MASK	0xffffffff

/*  txqctl offset0x18c  */
#define 	TXQCTL_DATA_BUFFER_THRESH_SHIFT	0
#define 	TXQCTL_DATA_BUFFER_THRESH_MASK	0xfff     

/*  rxqctl offset0x190  */
#define 	RXQCTL_RESERVED0_SHIFT	24
#define 	RXQCTL_RESERVED0_MASK	0x3f000000
#define 	RXQCTL_RXQ_PERF_TEST_EN_SHIFT	12
#define 	RXQCTL_RXQ_PERF_TEST_EN_MASK	0x1000    
#define 	RXQCTL_RXQ_DATA_BUF_TH_SHIFT	0
#define 	RXQCTL_RXQ_DATA_BUF_TH_MASK	0xfff     

/*  gpioselect offset0x194  */
#define 	GPIOSELECT_GPIO_SELECT_SHIFT	0
#define 	GPIOSELECT_GPIO_SELECT_MASK	0xf       

/*  gpio_output_en offset0x198  */
#define 	GPIO_OUTPUT_EN_GPIO_OE_SHIFT	0
#define 	GPIO_OUTPUT_EN_GPIO_OE_MASK	0xffff    

/*  clk_ctl_st offset0x1e0  */
#define 	CLK_CTL_ST_DMP_OOB_CIN_5_4_3_SHIFT	24
#define 	CLK_CTL_ST_DMP_OOB_CIN_5_4_3_MASK	0x7000000 
#define 	CLK_CTL_ST_DMP_OOB_CIN_7_6_SHIFT	18
#define 	CLK_CTL_ST_DMP_OOB_CIN_7_6_MASK	0xc0000   
#define 	CLK_CTL_ST_HTCLOCKAVAILABLE_SHIFT	17
#define 	CLK_CTL_ST_HTCLOCKAVAILABLE_MASK	0x20000   
#define 	CLK_CTL_ST_RESERVED4_SHIFT	16
#define 	CLK_CTL_ST_RESERVED4_MASK	0x10000   
#define 	CLK_CTL_ST_EXTRSRCREQ_SHIFT	8
#define 	CLK_CTL_ST_EXTRSRCREQ_MASK	0x100     
#define 	CLK_CTL_ST_RESERVED3_SHIFT	5
#define 	CLK_CTL_ST_RESERVED3_MASK	0x20      
#define 	CLK_CTL_ST_HTAVAILREQ_SHIFT	4
#define 	CLK_CTL_ST_HTAVAILREQ_MASK	0x10      
#define 	CLK_CTL_ST_RESERVED2_SHIFT	3
#define 	CLK_CTL_ST_RESERVED2_MASK	0x8       
#define 	CLK_CTL_ST_RESERVED1_SHIFT	2
#define 	CLK_CTL_ST_RESERVED1_MASK	0x4       
#define 	CLK_CTL_ST_FORCEHT_SHIFT	1
#define 	CLK_CTL_ST_FORCEHT_MASK	0x2       
#define 	CLK_CTL_ST_RESERVED0_SHIFT	0
#define 	CLK_CTL_ST_RESERVED0_MASK	0x1       

/*  pwrctl offset0x1e8  */
#define 	PWRCTL_PWRCTL_SHIFT	0
#define 	PWRCTL_PWRCTL_MASK	0xf       

/*  d64xmt0control offset0x200  */
#define 	D64XMT0CONTROL_ADDREXT_SHIFT	16
#define 	D64XMT0CONTROL_ADDREXT_MASK	0x30000   
#define 	D64XMT0CONTROL_TX_PT_CHK_DISABLE_SHIFT	11
#define 	D64XMT0CONTROL_TX_PT_CHK_DISABLE_MASK	0x800     
#define 	D64XMT0CONTROL_TX_LAUNCH_TIME_EN_SHIFT	3
#define 	D64XMT0CONTROL_TX_LAUNCH_TIME_EN_MASK	0x8       
#define 	D64XMT0CONTROL_DMA_LB_SHIFT	2
#define 	D64XMT0CONTROL_DMA_LB_MASK	0x4       
#define 	D64XMT0CONTROL_TXSUSPEND_SHIFT	1
#define 	D64XMT0CONTROL_TXSUSPEND_MASK	0x2       
#define 	D64XMT0CONTROL_XMTEN_SHIFT	0
#define 	D64XMT0CONTROL_XMTEN_MASK	0x1       

/*  d64xmt0ptr offset0x204  */
#define 	D64XMT0PTR_LASTDSCR_SHIFT	0
#define 	D64XMT0PTR_LASTDSCR_MASK	0x1fff    

/*  d64xmt0addrlow offset0x208  */
#define 	D64XMT0ADDRLOW_XMTADDR_LOW_SHIFT	0
#define 	D64XMT0ADDRLOW_XMTADDR_LOW_MASK	0xffffffff

/*  d64xmt0addrhigh offset0x20c  */
#define 	D64XMT0ADDRHIGH_XMTADDR_HIGH_SHIFT	0
#define 	D64XMT0ADDRHIGH_XMTADDR_HIGH_MASK	0xffffffff

/*  d64xmt0status0 offset0x210  */
#define 	D64XMT0STATUS0_XMTSTATE_SHIFT	28
#define 	D64XMT0STATUS0_XMTSTATE_MASK	0xf0000000
#define 	D64XMT0STATUS0_CURRDSCR_SHIFT	0
#define 	D64XMT0STATUS0_CURRDSCR_MASK	0x1fff    

/*  d64xmt0status1 offset0x214  */
#define 	D64XMT0STATUS1_XMTERR_SHIFT	28
#define 	D64XMT0STATUS1_XMTERR_MASK	0xf0000000
#define 	D64XMT0STATUS1_ACTIVEDESCR_SHIFT	0
#define 	D64XMT0STATUS1_ACTIVEDESCR_MASK	0x1fff    

/*  d64rcv0control offset0x220  */
#define 	D64RCV0CONTROL_ADDREXT_SHIFT	16
#define 	D64RCV0CONTROL_ADDREXT_MASK	0x30000   
#define 	D64RCV0CONTROL_RX_PT_CHK_DISABLE_SHIFT	11
#define 	D64RCV0CONTROL_RX_PT_CHK_DISABLE_MASK	0x800     
#define 	D64RCV0CONTROL_OFLOWEN_SHIFT	10
#define 	D64RCV0CONTROL_OFLOWEN_MASK	0x400     
#define 	D64RCV0CONTROL_SEPDESCRHDREN_SHIFT	9
#define 	D64RCV0CONTROL_SEPDESCRHDREN_MASK	0x200     
#define 	D64RCV0CONTROL_RCVOFFSET_SHIFT	1
#define 	D64RCV0CONTROL_RCVOFFSET_MASK	0xfe      
#define 	D64RCV0CONTROL_REVEN_SHIFT	0
#define 	D64RCV0CONTROL_REVEN_MASK	0x1       

/*  d64rcv0ptr offset0x224  */
#define 	D64RCV0PTR_RCVPTR_SHIFT	0
#define 	D64RCV0PTR_RCVPTR_MASK	0x1fff    

/*  d64rcv0addrlow offset0x228  */
#define 	D64RCV0ADDRLOW_RCVADDR_LOW_SHIFT	0
#define 	D64RCV0ADDRLOW_RCVADDR_LOW_MASK	0xffffffff

/*  d64rcv0addrhigh offset0x22c  */
#define 	D64RCV0ADDRHIGH_RCVADDR_HIGH_SHIFT	0
#define 	D64RCV0ADDRHIGH_RCVADDR_HIGH_MASK	0xffffffff

/*  d64rcv0status0 offset0x230  */
#define 	D64RCV0STATUS0_RCVSTATE_SHIFT	28
#define 	D64RCV0STATUS0_RCVSTATE_MASK	0xf0000000
#define 	D64RCV0STATUS0_CURRDSCR_SHIFT	0
#define 	D64RCV0STATUS0_CURRDSCR_MASK	0x1fff    

/*  d64rcv0status1 offset0x234  */
#define 	D64RCV0STATUS1_RCVERR_SHIFT	28
#define 	D64RCV0STATUS1_RCVERR_MASK	0xf0000000
#define 	D64RCV0STATUS1_ACTIVEDESCR_SHIFT	0
#define 	D64RCV0STATUS1_ACTIVEDESCR_MASK	0x1fff    

/*  d64xmt1control offset0x240  */
#define 	D64XMT1CONTROL_ADDREXT_SHIFT	16
#define 	D64XMT1CONTROL_ADDREXT_MASK	0x30000   
#define 	D64XMT1CONTROL_TX_PT_CHK_DISABLE_SHIFT	11
#define 	D64XMT1CONTROL_TX_PT_CHK_DISABLE_MASK	0x800     
#define 	D64XMT1CONTROL_TX_LAUNCH_TIME_EN_SHIFT	3
#define 	D64XMT1CONTROL_TX_LAUNCH_TIME_EN_MASK	0x8       
#define 	D64XMT1CONTROL_TXSUSPEND_SHIFT	1
#define 	D64XMT1CONTROL_TXSUSPEND_MASK	0x2       
#define 	D64XMT1CONTROL_XMTEN_SHIFT	0
#define 	D64XMT1CONTROL_XMTEN_MASK	0x1       

/*  d64xmt1ptr offset0x244  */
#define 	D64XMT1PTR_LASTDSCR_SHIFT	0
#define 	D64XMT1PTR_LASTDSCR_MASK	0x1fff    

/*  d64xmt1addrlow offset0x248  */
#define 	D64XMT1ADDRLOW_XMTADDR_LOW_SHIFT	0
#define 	D64XMT1ADDRLOW_XMTADDR_LOW_MASK	0xffffffff

/*  d64xmt1addrhigh offset0x24c  */
#define 	D64XMT1ADDRHIGH_XMTADDR_HIGH_SHIFT	0
#define 	D64XMT1ADDRHIGH_XMTADDR_HIGH_MASK	0xffffffff

/*  d64xmt1status0 offset0x250  */
#define 	D64XMT1STATUS0_XMTSTATE_SHIFT	28
#define 	D64XMT1STATUS0_XMTSTATE_MASK	0xf0000000
#define 	D64XMT1STATUS0_CURRDSCR_SHIFT	0
#define 	D64XMT1STATUS0_CURRDSCR_MASK	0x1fff    

/*  d64xmt1status1 offset0x254  */
#define 	D64XMT1STATUS1_XMTERR_SHIFT	28
#define 	D64XMT1STATUS1_XMTERR_MASK	0xf0000000
#define 	D64XMT1STATUS1_ACTIVEDESCR_SHIFT	0
#define 	D64XMT1STATUS1_ACTIVEDESCR_MASK	0x1fff    

/*  d64rcv1control offset0x260  */
#define 	D64RCV1CONTROL_ADDREXT_SHIFT	16
#define 	D64RCV1CONTROL_ADDREXT_MASK	0x30000   
#define 	D64RCV1CONTROL_RX_PT_CHK_DISABLE_SHIFT	11
#define 	D64RCV1CONTROL_RX_PT_CHK_DISABLE_MASK	0x800     
#define 	D64RCV1CONTROL_OFLOWEN_SHIFT	10
#define 	D64RCV1CONTROL_OFLOWEN_MASK	0x400     
#define 	D64RCV1CONTROL_RCVOFFSET_SHIFT	1
#define 	D64RCV1CONTROL_RCVOFFSET_MASK	0xfe      
#define 	D64RCV1CONTROL_REVEN_SHIFT	0
#define 	D64RCV1CONTROL_REVEN_MASK	0x1       

/*  d64rcv1ptr offset0x264  */
#define 	D64RCV1PTR_RCVPTR_SHIFT	0
#define 	D64RCV1PTR_RCVPTR_MASK	0x1fff    

/*  d64rcv1addrlow offset0x268  */
#define 	D64RCV1ADDRLOW_RCVADDR_LOW_SHIFT	0
#define 	D64RCV1ADDRLOW_RCVADDR_LOW_MASK	0xffffffff

/*  d64rcv1addrhigh offset0x26c  */
#define 	D64RCV1ADDRHIGH_RCVADDR_HIGH_SHIFT	0
#define 	D64RCV1ADDRHIGH_RCVADDR_HIGH_MASK	0xffffffff

/*  d64rcv1status0 offset0x270  */
#define 	D64RCV1STATUS0_RCVSTATE_SHIFT	28
#define 	D64RCV1STATUS0_RCVSTATE_MASK	0xf0000000
#define 	D64RCV1STATUS0_CURRDSCR_SHIFT	0
#define 	D64RCV1STATUS0_CURRDSCR_MASK	0x1fff    

/*  d64rcv1status1 offset0x274  */
#define 	D64RCV1STATUS1_RCVERR_SHIFT	28
#define 	D64RCV1STATUS1_RCVERR_MASK	0xf0000000
#define 	D64RCV1STATUS1_ACTIVEDESCR_SHIFT	0
#define 	D64RCV1STATUS1_ACTIVEDESCR_MASK	0x1fff    

/*  d64xmt2control offset0x280  */
#define 	D64XMT2CONTROL_ADDREXT_SHIFT	16
#define 	D64XMT2CONTROL_ADDREXT_MASK	0x30000   
#define 	D64XMT2CONTROL_TX_PT_CHK_DISABLE_SHIFT	11
#define 	D64XMT2CONTROL_TX_PT_CHK_DISABLE_MASK	0x800     
#define 	D64XMT2CONTROL_TX_LAUNCH_TIME_EN_SHIFT	3
#define 	D64XMT2CONTROL_TX_LAUNCH_TIME_EN_MASK	0x8       
#define 	D64XMT2CONTROL_TXSUSPEND_SHIFT	1
#define 	D64XMT2CONTROL_TXSUSPEND_MASK	0x2       
#define 	D64XMT2CONTROL_XMTEN_SHIFT	0
#define 	D64XMT2CONTROL_XMTEN_MASK	0x1       

/*  d64xmt2ptr offset0x284  */
#define 	D64XMT2PTR_LASTDSCR_SHIFT	0
#define 	D64XMT2PTR_LASTDSCR_MASK	0x1fff    

/*  d64xmt2addrlow offset0x288  */
#define 	D64XMT2ADDRLOW_XMTADDR_LOW_SHIFT	0
#define 	D64XMT2ADDRLOW_XMTADDR_LOW_MASK	0xffffffff

/*  d64xmt2addrhigh offset0x28c  */
#define 	D64XMT2ADDRHIGH_XMTADDR_HIGH_SHIFT	0
#define 	D64XMT2ADDRHIGH_XMTADDR_HIGH_MASK	0xffffffff

/*  d64xmt2status0 offset0x290  */
#define 	D64XMT2STATUS0_XMTSTATE_SHIFT	28
#define 	D64XMT2STATUS0_XMTSTATE_MASK	0xf0000000
#define 	D64XMT2STATUS0_CURRDSCR_SHIFT	0
#define 	D64XMT2STATUS0_CURRDSCR_MASK	0x1fff    

/*  d64xmt2status1 offset0x294  */
#define 	D64XMT2STATUS1_XMTERR_SHIFT	28
#define 	D64XMT2STATUS1_XMTERR_MASK	0xf0000000
#define 	D64XMT2STATUS1_ACTIVEDESCR_SHIFT	0
#define 	D64XMT2STATUS1_ACTIVEDESCR_MASK	0x1fff    

/*  d64rcv2control offset0x2a0  */
#define 	D64RCV2CONTROL_ADDREXT_SHIFT	16
#define 	D64RCV2CONTROL_ADDREXT_MASK	0x30000   
#define 	D64RCV2CONTROL_RX_PT_CHK_DISABLE_SHIFT	11
#define 	D64RCV2CONTROL_RX_PT_CHK_DISABLE_MASK	0x800     
#define 	D64RCV2CONTROL_OFLOWEN_SHIFT	10
#define 	D64RCV2CONTROL_OFLOWEN_MASK	0x400     
#define 	D64RCV2CONTROL_RCVOFFSET_SHIFT	1
#define 	D64RCV2CONTROL_RCVOFFSET_MASK	0xfe      
#define 	D64RCV2CONTROL_REVEN_SHIFT	0
#define 	D64RCV2CONTROL_REVEN_MASK	0x1       

/*  d64rcv2ptr offset0x2a4  */
#define 	D64RCV2PTR_RCVPTR_SHIFT	0
#define 	D64RCV2PTR_RCVPTR_MASK	0x1fff    

/*  d64rcv2addrlow offset0x2a8  */
#define 	D64RCV2ADDRLOW_RCVADDR_LOW_SHIFT	0
#define 	D64RCV2ADDRLOW_RCVADDR_LOW_MASK	0xffffffff

/*  d64rcv2addrhigh offset0x2ac  */
#define 	D64RCV2ADDRHIGH_RCVADDR_HIGH_SHIFT	0
#define 	D64RCV2ADDRHIGH_RCVADDR_HIGH_MASK	0xffffffff

/*  d64rcv2status0 offset0x2b0  */
#define 	D64RCV2STATUS0_RCVSTATE_SHIFT	28
#define 	D64RCV2STATUS0_RCVSTATE_MASK	0xf0000000
#define 	D64RCV2STATUS0_CURRDSCR_SHIFT	0
#define 	D64RCV2STATUS0_CURRDSCR_MASK	0x1fff    

/*  d64rcv2status1 offset0x2b4  */
#define 	D64RCV2STATUS1_RCVERR_SHIFT	28
#define 	D64RCV2STATUS1_RCVERR_MASK	0xf0000000
#define 	D64RCV2STATUS1_ACTIVEDESCR_SHIFT	0
#define 	D64RCV2STATUS1_ACTIVEDESCR_MASK	0x1fff    

/*  d64xmt3control offset0x2c0  */
#define 	D64XMT3CONTROL_ADDREXT_SHIFT	16
#define 	D64XMT3CONTROL_ADDREXT_MASK	0x30000   
#define 	D64XMT3CONTROL_TX_PT_CHK_DISABLE_SHIFT	11
#define 	D64XMT3CONTROL_TX_PT_CHK_DISABLE_MASK	0x800     
#define 	D64XMT3CONTROL_TX_LAUNCH_TIME_EN_SHIFT	3
#define 	D64XMT3CONTROL_TX_LAUNCH_TIME_EN_MASK	0x8       
#define 	D64XMT3CONTROL_TXSUSPEND_SHIFT	1
#define 	D64XMT3CONTROL_TXSUSPEND_MASK	0x2       
#define 	D64XMT3CONTROL_XMTEN_SHIFT	0
#define 	D64XMT3CONTROL_XMTEN_MASK	0x1       

/*  d64xmt3ptr offset0x2c4  */
#define 	D64XMT3PTR_LASTDSCR_SHIFT	0
#define 	D64XMT3PTR_LASTDSCR_MASK	0x1fff    

/*  d64xmt3addrlow offset0x2c8  */
#define 	D64XMT3ADDRLOW_XMTADDR_LOW_SHIFT	0
#define 	D64XMT3ADDRLOW_XMTADDR_LOW_MASK	0xffffffff

/*  d64xmt3addrhigh offset0x2cc  */
#define 	D64XMT3ADDRHIGH_XMTADDR_HIGH_SHIFT	0
#define 	D64XMT3ADDRHIGH_XMTADDR_HIGH_MASK	0xffffffff

/*  d64xmt3status0 offset0x2d0  */
#define 	D64XMT3STATUS0_XMTSTATE_SHIFT	28
#define 	D64XMT3STATUS0_XMTSTATE_MASK	0xf0000000
#define 	D64XMT3STATUS0_CURRDSCR_SHIFT	0
#define 	D64XMT3STATUS0_CURRDSCR_MASK	0x1fff    

/*  d64xmt3status1 offset0x2d4  */
#define 	D64XMT3STATUS1_XMTERR_SHIFT	28
#define 	D64XMT3STATUS1_XMTERR_MASK	0xf0000000
#define 	D64XMT3STATUS1_ACTIVEDESCR_SHIFT	0
#define 	D64XMT3STATUS1_ACTIVEDESCR_MASK	0x1fff    

/*  d64rcv3control offset0x2e0  */
#define 	D64RCV3CONTROL_ADDREXT_SHIFT	16
#define 	D64RCV3CONTROL_ADDREXT_MASK	0x30000   
#define 	D64RCV3CONTROL_RX_PT_CHK_DISABLE_SHIFT	11
#define 	D64RCV3CONTROL_RX_PT_CHK_DISABLE_MASK	0x800     
#define 	D64RCV3CONTROL_OFLOWEN_SHIFT	10
#define 	D64RCV3CONTROL_OFLOWEN_MASK	0x400     
#define 	D64RCV3CONTROL_RCVOFFSET_SHIFT	1
#define 	D64RCV3CONTROL_RCVOFFSET_MASK	0xfe      
#define 	D64RCV3CONTROL_REVEN_SHIFT	0
#define 	D64RCV3CONTROL_REVEN_MASK	0x1       

/*  d64rcv3ptr offset0x2e4  */
#define 	D64RCV3PTR_RCVPTR_SHIFT	0
#define 	D64RCV3PTR_RCVPTR_MASK	0x1fff    

/*  d64rcv3addrlow offset0x2e8  */
#define 	D64RCV3ADDRLOW_RCVADDR_LOW_SHIFT	0
#define 	D64RCV3ADDRLOW_RCVADDR_LOW_MASK	0xffffffff

/*  d64rcv3addrhigh offset0x2ec  */
#define 	D64RCV3ADDRHIGH_RCVADDR_HIGH_SHIFT	0
#define 	D64RCV3ADDRHIGH_RCVADDR_HIGH_MASK	0xffffffff

/*  d64rcv3status0 offset0x2f0  */
#define 	D64RCV3STATUS0_RCVSTATE_SHIFT	28
#define 	D64RCV3STATUS0_RCVSTATE_MASK	0xf0000000
#define 	D64RCV3STATUS0_CURRDSCR_SHIFT	0
#define 	D64RCV3STATUS0_CURRDSCR_MASK	0x1fff    

/*  d64rcv3status1 offset0x2f4  */
#define 	D64RCV3STATUS1_RCVERR_SHIFT	28
#define 	D64RCV3STATUS1_RCVERR_MASK	0xf0000000
#define 	D64RCV3STATUS1_ACTIVEDESCR_SHIFT	0
#define 	D64RCV3STATUS1_ACTIVEDESCR_MASK	0x1fff    

/*  tx_good_octets offset0x300  */
#define 	TX_GOOD_OCTETS_COUNT_SHIFT	0
#define 	TX_GOOD_OCTETS_COUNT_MASK	0xffffffff

/*  tx_good_octets_high offset0x304  */
#define 	TX_GOOD_OCTETS_HIGH_COUNT_SHIFT	0
#define 	TX_GOOD_OCTETS_HIGH_COUNT_MASK	0xffffffff

/*  tx_good_pkts offset0x308  */
#define 	TX_GOOD_PKTS_COUNT_SHIFT	0
#define 	TX_GOOD_PKTS_COUNT_MASK	0xffffffff

/*  tx_octets offset0x30c  */
#define 	TX_OCTETS_COUNT_SHIFT	0
#define 	TX_OCTETS_COUNT_MASK	0xffffffff

/*  tx_octets_high offset0x310  */
#define 	TX_OCTETS_HIGH_COUNT_SHIFT	0
#define 	TX_OCTETS_HIGH_COUNT_MASK	0xffffffff

/*  tx_pkts offset0x314  */
#define 	TX_PKTS_COUNT_SHIFT	0
#define 	TX_PKTS_COUNT_MASK	0xffffffff

/*  tx_broadcast_pkts offset0x318  */
#define 	TX_BROADCAST_PKTS_COUNT_SHIFT	0
#define 	TX_BROADCAST_PKTS_COUNT_MASK	0xffffffff

/*  tx_multicast_pkts offset0x31c  */
#define 	TX_MULTICAST_PKTS_COUNT_SHIFT	0
#define 	TX_MULTICAST_PKTS_COUNT_MASK	0xffffffff

/*  tx_uni_pkts offset0x320  */
#define 	TX_UNI_PKTS_COUNT_SHIFT	0
#define 	TX_UNI_PKTS_COUNT_MASK	0xffffffff

/*  tx_len_64 offset0x324  */
#define 	TX_LEN_64_COUNT_SHIFT	0
#define 	TX_LEN_64_COUNT_MASK	0xffffffff

/*  tx_len_65_to_127 offset0x328  */
#define 	TX_LEN_65_TO_127_COUNT_SHIFT	0
#define 	TX_LEN_65_TO_127_COUNT_MASK	0xffffffff

/*  tx_len_128_to_255 offset0x32c  */
#define 	TX_LEN_128_TO_255_COUNT_SHIFT	0
#define 	TX_LEN_128_TO_255_COUNT_MASK	0xffffffff

/*  tx_len_256_to_511 offset0x330  */
#define 	TX_LEN_256_TO_511_COUNT_SHIFT	0
#define 	TX_LEN_256_TO_511_COUNT_MASK	0xffffffff

/*  tx_len_512_to_1023 offset0x334  */
#define 	TX_LEN_512_TO_1023_COUNT_SHIFT	0
#define 	TX_LEN_512_TO_1023_COUNT_MASK	0xffffffff

/*  tx_len_1024_to_max offset0x338  */
#define 	TX_LEN_1024_TO_MAX_COUNT_SHIFT	0
#define 	TX_LEN_1024_TO_MAX_COUNT_MASK	0xffffffff

/*  tx_len_max_to_jumbo offset0x33c  */
#define 	TX_LEN_MAX_TO_JUMBO_COUNT_SHIFT	0
#define 	TX_LEN_MAX_TO_JUMBO_COUNT_MASK	0xffffffff

/*  tx_jabber_pkts offset0x340  */
#define 	TX_JABBER_PKTS_COUNT_SHIFT	0
#define 	TX_JABBER_PKTS_COUNT_MASK	0xffffffff

/*  tx_oversize_pkts offset0x344  */
#define 	TX_OVERSIZE_PKTS_COUNT_SHIFT	0
#define 	TX_OVERSIZE_PKTS_COUNT_MASK	0xffffffff

/*  tx_fragment_pkts offset0x348  */
#define 	TX_FRAGMENT_PKTS_COUNT_SHIFT	0
#define 	TX_FRAGMENT_PKTS_COUNT_MASK	0xffffffff

/*  tx_underruns offset0x34c  */
#define 	TX_UNDERRUNS_COUNT_SHIFT	0
#define 	TX_UNDERRUNS_COUNT_MASK	0xffffffff

/*  tx_total_cols offset0x350  */
#define 	TX_TOTAL_COLS_COUNT_SHIFT	0
#define 	TX_TOTAL_COLS_COUNT_MASK	0xffffffff

/*  tx_single_cols offset0x354  */
#define 	TX_SINGLE_COLS_COUNT_SHIFT	0
#define 	TX_SINGLE_COLS_COUNT_MASK	0xffffffff

/*  tx_mulitple_cols offset0x358  */
#define 	TX_MULITPLE_COLS_COUNT_SHIFT	0
#define 	TX_MULITPLE_COLS_COUNT_MASK	0xffffffff

/*  tx_excessive_cols offset0x35c  */
#define 	TX_EXCESSIVE_COLS_COUNT_SHIFT	0
#define 	TX_EXCESSIVE_COLS_COUNT_MASK	0xffffffff

/*  tx_late_cols offset0x360  */
#define 	TX_LATE_COLS_COUNT_SHIFT	0
#define 	TX_LATE_COLS_COUNT_MASK	0xffffffff

/*  tx_defered offset0x364  */
#define 	TX_DEFERED_COUNT_SHIFT	0
#define 	TX_DEFERED_COUNT_MASK	0xffffffff

/*  tx_pause_pkts offset0x368  */
#define 	TX_PAUSE_PKTS_COUNT_SHIFT	0
#define 	TX_PAUSE_PKTS_COUNT_MASK	0xffffffff

/*  rx_good_octets offset0x380  */
#define 	RX_GOOD_OCTETS_COUNT_SHIFT	0
#define 	RX_GOOD_OCTETS_COUNT_MASK	0xffffffff

/*  rx_good_octets_high offset0x384  */
#define 	RX_GOOD_OCTETS_HIGH_COUNT_SHIFT	0
#define 	RX_GOOD_OCTETS_HIGH_COUNT_MASK	0xffffffff

/*  rx_good_pkts offset0x388  */
#define 	RX_GOOD_PKTS_COUNT_SHIFT	0
#define 	RX_GOOD_PKTS_COUNT_MASK	0xffffffff

/*  rx_octets offset0x38c  */
#define 	RX_OCTETS_COUNT_SHIFT	0
#define 	RX_OCTETS_COUNT_MASK	0xffffffff

/*  rx_octets_high offset0x390  */
#define 	RX_OCTETS_HIGH_COUNT_SHIFT	0
#define 	RX_OCTETS_HIGH_COUNT_MASK	0xffffffff

/*  rx_pkts offset0x394  */
#define 	RX_PKTS_COUNT_SHIFT	0
#define 	RX_PKTS_COUNT_MASK	0xffffffff

/*  rx_broadcast_pkts offset0x398  */
#define 	RX_BROADCAST_PKTS_COUNT_SHIFT	0
#define 	RX_BROADCAST_PKTS_COUNT_MASK	0xffffffff

/*  rx_multicast_pkts offset0x39c  */
#define 	RX_MULTICAST_PKTS_COUNT_SHIFT	0
#define 	RX_MULTICAST_PKTS_COUNT_MASK	0xffffffff

/*  rx_uni_pkts offset0x3a0  */
#define 	RX_UNI_PKTS_COUNT_SHIFT	0
#define 	RX_UNI_PKTS_COUNT_MASK	0xffffffff

/*  rx_len_64 offset0x3a4  */
#define 	RX_LEN_64_COUNT_SHIFT	0
#define 	RX_LEN_64_COUNT_MASK	0xffffffff

/*  rx_len_65_to_127 offset0x3a8  */
#define 	RX_LEN_65_TO_127_COUNT_SHIFT	0
#define 	RX_LEN_65_TO_127_COUNT_MASK	0xffffffff

/*  rx_len_128_to_255 offset0x3ac  */
#define 	RX_LEN_128_TO_255_COUNT_SHIFT	0
#define 	RX_LEN_128_TO_255_COUNT_MASK	0xffffffff

/*  rx_len_256_to_511 offset0x3b0  */
#define 	RX_LEN_256_TO_511_COUNT_SHIFT	0
#define 	RX_LEN_256_TO_511_COUNT_MASK	0xffffffff

/*  rx_len_512_to_1023 offset0x3b4  */
#define 	RX_LEN_512_TO_1023_COUNT_SHIFT	0
#define 	RX_LEN_512_TO_1023_COUNT_MASK	0xffffffff

/*  rx_len_1024_to_max offset0x3b8  */
#define 	RX_LEN_1024_TO_MAX_COUNT_SHIFT	0
#define 	RX_LEN_1024_TO_MAX_COUNT_MASK	0xffffffff

/*  rx_len_max_to_jumbo offset0x3bc  */
#define 	RX_LEN_MAX_TO_JUMBO_COUNT_SHIFT	0
#define 	RX_LEN_MAX_TO_JUMBO_COUNT_MASK	0xffffffff

/*  rx_jabber_pkts offset0x3c0  */
#define 	RX_JABBER_PKTS_COUNT_SHIFT	0
#define 	RX_JABBER_PKTS_COUNT_MASK	0xffffffff

/*  rx_oversize_pkts offset0x3c4  */
#define 	RX_OVERSIZE_PKTS_COUNT_SHIFT	0
#define 	RX_OVERSIZE_PKTS_COUNT_MASK	0xffffffff

/*  rx_fragment_pkts offset0x3c8  */
#define 	RX_FRAGMENT_PKTS_COUNT_SHIFT	0
#define 	RX_FRAGMENT_PKTS_COUNT_MASK	0xffffffff

/*  rx_missed_pkts offset0x3cc  */
#define 	RX_MISSED_PKTS_COUNT_SHIFT	0
#define 	RX_MISSED_PKTS_COUNT_MASK	0xffffffff

/*  rx_undersize offset0x3d0  */
#define 	RX_UNDERSIZE_COUNT_SHIFT	0
#define 	RX_UNDERSIZE_COUNT_MASK	0xffffffff

/*  rx_crc_errs offset0x3d4  */
#define 	RX_CRC_ERRS_COUNT_SHIFT	0
#define 	RX_CRC_ERRS_COUNT_MASK	0xffffffff

/*  rx_align_errs offset0x3d8  */
#define 	RX_ALIGN_ERRS_COUNT_SHIFT	0
#define 	RX_ALIGN_ERRS_COUNT_MASK	0xffffffff

/*  rx_symbol_errs offset0x3dc  */
#define 	RX_SYMBOL_ERRS_COUNT_SHIFT	0
#define 	RX_SYMBOL_ERRS_COUNT_MASK	0xffffffff

/*  rx_pause_pkts offset0x3e0  */
#define 	RX_PAUSE_PKTS_COUNT_SHIFT	0
#define 	RX_PAUSE_PKTS_COUNT_MASK	0xffffffff

/*  rx_nonpause_pkts offset0x3e4  */
#define 	RX_NONPAUSE_PKTS_COUNT_SHIFT	0
#define 	RX_NONPAUSE_PKTS_COUNT_MASK	0xffffffff

/*  rxq0_irc_drop offset0x3e8  */
#define 	RXQ0_IRC_DROP_COUNT_SHIFT	0
#define 	RXQ0_IRC_DROP_COUNT_MASK	0xffffffff

/*  rxq1_irc_drop offset0x3ec  */
#define 	RXQ1_IRC_DROP_COUNT_SHIFT	0
#define 	RXQ1_IRC_DROP_COUNT_MASK	0xffffffff

/*  rxq2_irc_drop offset0x3f0  */
#define 	RXQ2_IRC_DROP_COUNT_SHIFT	0
#define 	RXQ2_IRC_DROP_COUNT_MASK	0xffffffff

/*  rxq3_irc_drop offset0x3f4  */
#define 	RXQ3_IRC_DROP_COUNT_SHIFT	0
#define 	RXQ3_IRC_DROP_COUNT_MASK	0xffffffff

/*  rx_cfp_drop offset0x3f8  */
#define 	RX_CFP_DROP_COUNT_SHIFT	0
#define 	RX_CFP_DROP_COUNT_MASK	0xffffffff

/*  core_version offset0x800  */
#define 	CORE_VERSION_CORE_VERSION_SHIFT	0
#define 	CORE_VERSION_CORE_VERSION_MASK	0xffff    
#define 	CORE_VERSION_CUST_VERSION_SHIFT	16
#define 	CORE_VERSION_CUST_VERSION_MASK	0xffff0000

/*  ipg_hd_bkp_cntl offset0x804  */
#define 	IPG_HD_BKP_CNTL_HD_FC_ENA_SHIFT	0
#define 	IPG_HD_BKP_CNTL_HD_FC_ENA_MASK	0x1       
#define 	IPG_HD_BKP_CNTL_HD_FC_BKOFF_OK_SHIFT	1
#define 	IPG_HD_BKP_CNTL_HD_FC_BKOFF_OK_MASK	0x2       
#define 	IPG_HD_BKP_CNTL_IPG_CONFIG_RX_SHIFT	2
#define 	IPG_HD_BKP_CNTL_IPG_CONFIG_RX_MASK	0x7c      

/*  command_config offset0x808  */
#define 	COMMAND_CONFIG_TX_ENA_SHIFT	0
#define 	COMMAND_CONFIG_TX_ENA_MASK	0x1       
#define 	COMMAND_CONFIG_RX_ENA_SHIFT	1
#define 	COMMAND_CONFIG_RX_ENA_MASK	0x2       
#define 	COMMAND_CONFIG_ETH_SPEED_SHIFT	2
#define 	COMMAND_CONFIG_ETH_SPEED_MASK	0xc       
#define 	COMMAND_CONFIG_PROMIS_EN_SHIFT	4
#define 	COMMAND_CONFIG_PROMIS_EN_MASK	0x10      
#define 	COMMAND_CONFIG_PAD_EN_SHIFT	5
#define 	COMMAND_CONFIG_PAD_EN_MASK	0x20      
#define 	COMMAND_CONFIG_CRC_FWD_SHIFT	6
#define 	COMMAND_CONFIG_CRC_FWD_MASK	0x40      
#define 	COMMAND_CONFIG_PAUSE_FWD_SHIFT	7
#define 	COMMAND_CONFIG_PAUSE_FWD_MASK	0x80      
#define 	COMMAND_CONFIG_PAUSE_IGNORE_SHIFT	8
#define 	COMMAND_CONFIG_PAUSE_IGNORE_MASK	0x100     
#define 	COMMAND_CONFIG_TX_ADDR_INS_SHIFT	9
#define 	COMMAND_CONFIG_TX_ADDR_INS_MASK	0x200     
#define 	COMMAND_CONFIG_HD_ENA_SHIFT	10
#define 	COMMAND_CONFIG_HD_ENA_MASK	0x400     
#define 	COMMAND_CONFIG_OVERFLOW_EN_SHIFT	12
#define 	COMMAND_CONFIG_OVERFLOW_EN_MASK	0x1000    
#define 	COMMAND_CONFIG_SW_RESET_SHIFT	13
#define 	COMMAND_CONFIG_SW_RESET_MASK	0x2000    
#define 	COMMAND_CONFIG_LOOP_ENA_SHIFT	15
#define 	COMMAND_CONFIG_LOOP_ENA_MASK	0x8000    
#define 	COMMAND_CONFIG_MAC_LOOP_CON_SHIFT	16
#define 	COMMAND_CONFIG_MAC_LOOP_CON_MASK	0x10000   
#define 	COMMAND_CONFIG_ENA_EXT_CONFIG_SHIFT	22
#define 	COMMAND_CONFIG_ENA_EXT_CONFIG_MASK	0x400000  
#define 	COMMAND_CONFIG_CNTL_FRM_ENA_SHIFT	23
#define 	COMMAND_CONFIG_CNTL_FRM_ENA_MASK	0x800000  
#define 	COMMAND_CONFIG_NO_LGTH_CHECK_SHIFT	24
#define 	COMMAND_CONFIG_NO_LGTH_CHECK_MASK	0x1000000 
#define 	COMMAND_CONFIG_LINE_LOOPBACK_SHIFT	25
#define 	COMMAND_CONFIG_LINE_LOOPBACK_MASK	0x2000000 
#define 	COMMAND_CONFIG_RESVERED_1_SHIFT	26
#define 	COMMAND_CONFIG_RESVERED_1_MASK	0x4000000 
#define 	COMMAND_CONFIG_RESERVED_2_SHIFT	27
#define 	COMMAND_CONFIG_RESERVED_2_MASK	0x8000000 
#define 	COMMAND_CONFIG_IGNORE_TX_PAUSE_SHIFT	28
#define 	COMMAND_CONFIG_IGNORE_TX_PAUSE_MASK	0x10000000
#define 	COMMAND_CONFIG_SW_CTRL_RXTX_AFTER_LKUP_SHIFT	29
#define 	COMMAND_CONFIG_SW_CTRL_RXTX_AFTER_LKUP_MASK	0x20000000
#define 	COMMAND_CONFIG_RESVERED_3_SHIFT	30
#define 	COMMAND_CONFIG_RESVERED_3_MASK	0x40000000

/*  mac_0 offset0x80c  */
#define 	MAC_0_MAC_ADDR0_SHIFT	0
#define 	MAC_0_MAC_ADDR0_MASK	0xffffffff

/*  mac_1 offset0x810  */
#define 	MAC_1_MAC_ADDR1_SHIFT	0
#define 	MAC_1_MAC_ADDR1_MASK	0xffff    

/*  frm_length offset0x814  */
#define 	FRM_LENGTH_MAXFR_SHIFT	0
#define 	FRM_LENGTH_MAXFR_MASK	0x3fff    

/*  pause_quant offset0x818  */
#define 	PAUSE_QUANT_STAD2_SHIFT	0
#define 	PAUSE_QUANT_STAD2_MASK	0xffff    

/*  mac_mode offset0x844  */
#define 	MAC_MODE_MAC_SPEED_SHIFT	0
#define 	MAC_MODE_MAC_SPEED_MASK	0x3       
#define 	MAC_MODE_MAC_DUPLEX_SHIFT	2
#define 	MAC_MODE_MAC_DUPLEX_MASK	0x4       
#define 	MAC_MODE_MAC_RX_PAUSE_SHIFT	3
#define 	MAC_MODE_MAC_RX_PAUSE_MASK	0x8       
#define 	MAC_MODE_MAC_TX_PAUSE_SHIFT	4
#define 	MAC_MODE_MAC_TX_PAUSE_MASK	0x10      
#define 	MAC_MODE_LINK_STATUS_SHIFT	5
#define 	MAC_MODE_LINK_STATUS_MASK	0x20      

/*  tx_ipg_length offset0x85c  */
#define 	TX_IPG_LENGTH_TX_IPG_LENGTH_SHIFT	0
#define 	TX_IPG_LENGTH_TX_IPG_LENGTH_MASK	0x7f      

/*  ts_status_cntrl offset0xb18  */
#define 	TS_STATUS_CNTRL_WORD_AVAIL_SHIFT	2
#define 	TS_STATUS_CNTRL_WORD_AVAIL_MASK	0x1c      
#define 	TS_STATUS_CNTRL_TX_TS_FIFO_EMPTY_SHIFT	1
#define 	TS_STATUS_CNTRL_TX_TS_FIFO_EMPTY_MASK	0x2       
#define 	TS_STATUS_CNTRL_TX_TS_FIFO_FULL_SHIFT	0
#define 	TS_STATUS_CNTRL_TX_TS_FIFO_FULL_MASK	0x1       

/*  tx_ts_data offset0xb1c  */
#define 	TX_TS_DATA_TX_TS_DATA_SHIFT	0
#define 	TX_TS_DATA_TX_TS_DATA_MASK	0xffffffff

/*  pause_control offset0xb30  */
#define 	PAUSE_CONTROL_VALUE_SHIFT	0
#define 	PAUSE_CONTROL_VALUE_MASK	0x1ffff   
#define 	PAUSE_CONTROL_ENABLE_SHIFT	17
#define 	PAUSE_CONTROL_ENABLE_MASK	0x20000   

/*  flush_control offset0xb34  */
#define 	FLUSH_CONTROL_FLUSH_SHIFT	0
#define 	FLUSH_CONTROL_FLUSH_MASK	0x1       

/*  rxfifo_stat offset0xb38  */
#define 	RXFIFO_STAT_RXFIFO_UNDERRUN_SHIFT	0
#define 	RXFIFO_STAT_RXFIFO_UNDERRUN_MASK	0x1       
#define 	RXFIFO_STAT_RXFIFO_OVERRUN_SHIFT	1
#define 	RXFIFO_STAT_RXFIFO_OVERRUN_MASK	0x2       

/*  txfifo_stat offset0xb3c  */
#define 	TXFIFO_STAT_TXFIFO_UNDERRUN_SHIFT	0
#define 	TXFIFO_STAT_TXFIFO_UNDERRUN_MASK	0x1       
#define 	TXFIFO_STAT_TXFIFO_OVERRUN_SHIFT	1
#define 	TXFIFO_STAT_TXFIFO_OVERRUN_MASK	0x2       

#endif /* _gmac0_core_h_ */
