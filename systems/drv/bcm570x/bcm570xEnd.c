/*
 * $Id: bcm570xEnd.c,v 1.12 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * 
 * NPT/SENS Driver for Broadcom BCM570x Gigabit Ethernet NIC ASIC family.
 *
 * This module implements an END driver for BCM570x family of Gigabit
 * Ethernet NIC ASIC's for use with the VxWorks 5.4.x SENS protocol
 * stack (Tornado 2.0). 
 *
 * This driver should operate with little or no modifications on a
 * wide variety of target platforms. The driver can be built to 
 * support big-endian or little-endian architectures, however it has only
 * been tested on big endian machines.
 *
 * The driver is configured via an END load string.  The load string
 * contains the following fields:
 * 
 *   unit : membase: iline : align
 * 
 *   Where:
 *      1) unit defines the device instance (0,1,2 ... N)
 *         This value is prepended to your load string by the VxWorks.
 *         Do not specify it!  
 *         NOTE: Be sure value < IP_MAX_UNITS in config*.h
 *      2) membase defines the PCI memory region on your target CPU.
 *      3) iline defines the interrupt number or IRQ for the device.
 *      4) align: indicates how many bytes to shift data in ethernet
 *                 frame for protocol stack to align IP header in packet.
 *                 For PowerPC and most other architectures, use 0,
 *                 for MIPS, use the value 2.
 *
 * Required BSP functions
 * ----------------------
 *
 * _func_bcm570xEnetAddrGet() -- pointer to a BSP function to get the
 *     MAC address. If that function doesn't return OK,
 *     the driver will supply a hardwired MAC address.
 *
 * sysUsecDelay() -- a hardware-level delay with microsecond granularity
 *  
 * sysSerialPrintString() -- a polled serial printing function,
 *                           similar to Linux printk()
 *
 * sysBcm570xCacheFuncsGet() -- fills in the structure that contains
 *                              the flush and invalidate function pointers.
 * 
 *
 * Defines
 * -------
 *
 * USE_ZERO_COPY_TX -- if this is true, then hardware gather/scatter
 *     is used for transmit.  If not, then all outgoing packets that
 *     consist of multiple mbufs, will be copied to a single contiguous
 *     buffer.  
 *
 * USE_ZERO_COPY_RX -- if this is true, receive buffers will not be
 *     copied when the "align" value is non-zero.  This can be used
 *     with CPUs that support DMA to unaligned addresses.
 *
 * INCLUDE_MIBII_RFC2233 -- if this is supported by VxWorks, it will
 *     be defined in a VxWorks header file.  Do not define this yourself.
 * 
 * PKT_DEBUG -- used for debugging.  When true, packets will be dumped
 *     to the console if global variable bcm570x_pkt_debug is non-zero.
 *
 * JUMBO_FRAMES -- ignored
 *
 * BCM570X_ADAPTIVE_RX -- enables support code for adaptive receive
 *     coalesence; however, it has no effect unless you supply a periodic
 * task that calls bcm570xAdaptRxCoalesce().
 *
 * BCM570X_DEBUG_TASK -- used for debugging the driver if the bcm570x
 *     isn't being used as the boot device.
 *
 * Others -- a few other debug macros are lying about...
 *
 * Defines used in tigon3
 * ----------------------
 * 
 * BIG_ENDIAN_HOST -- this macro will be set for you automatically in mm.h
 *
 * BIG_ENDIAN_PCI -- define this macro if you're running on a big endian 
 *     machine that swaps PCI memory accesses for you.  
 * 
 * T3_JUMBO_RCV_ENTRY_COUNT -- this is the size of the ring for receive frames
 *    greater than 1500 bytes.  VxWorks doesn't support jumbo frames, so you
 *    can turn this on if you like but it will just compile in a bunch of code
 *    that doesn't do anything.
 *
 * INCLUDE_5701_AX_FIX -- define this if your chip is a 5701 A0 or B0.
 *
 * PCIX_TARGET_WORKAROUND -- define this if you're using PCIX, not
 *     PCI.  I think it's really only needed for Intel hosts but I'm not
 *     sure.
 *
 * INCLUDE_TBI_SUPPORT -- for Gig Ethernet.  If you're running 10/100,
 *     you don't need this.
 * 
 * ----------------------------------------------------------------------------
 * Known Issues
 * 
 *    Polled mode send/receive implemented, but not reliable.
 *
 *    Driver only supports "standard" rings.  Mini and jumbo not supported.
 *
 *    Chip monitor task not spawned by this driver.  See the Linux driver,
 *    b57um.c, if you're interested in adding adaptive features.
 *
 *    VLAN tags are not supported.
 * 
 * Related Documentation
 *
 *    Broadcom BCM570x Register Reference
 *    Broadcom BCM570x Host Programming I/F Specification
 *    Wind River Systems Network Protocol Toolkit
 *    Wind River Systems VxWorks Network Programmers Guide
 * 
 * Based on: Linux TIGON3 Driver version 3.0.2
 * 
 * Author(s):  James Dougherty (jfd@broadcom.com)    (VxWorks port)
 *             Jimmy Blair     (jblair@broadcom.com) (VxWorks port)
 *             Michael Chan (mchan@broadcom.com)     (Original Linux version)
 *
 *
 */
#include <time.h>
#include <vxWorks.h>
#include <taskLib.h>
#include <intLib.h>
#include <lstLib.h>
#include <semLib.h>
#include "bcm570xEnd.h"
#include "mm.h"
#include "autoneg.h"

#if (CPU == RC32364 || CPU == MIPS32)
#include "idts334.h"

/* Enable/Disable bus errors before performing PCI config I/O */
IMPORT void     sysPciBusErrEnable();
IMPORT void     sysPciBusErrDisable();
IMPORT void     sysLedDsply();
#endif
 
IMPORT void sysBcm570xCacheFuncsGet (CACHE_FUNCS *pCacheFuncs);
IMPORT void   sysSerialPrintString(char *s);
STATUS (* _func_bcm570xEnetAddrGet) 
    (char *dev,int unit,unsigned char *pMac) = NULL;

/*
 *  Max number of chips in a system
 */
#define MAX_UNITS 4

/* Forward declations: SENS driver entrypoints */
END_OBJ *bcm570xEndLoad(char *is, void* ap);
STATIC STATUS	bcm570xEndUnload(END_OBJ *);
STATIC STATUS	bcm570xEndIoctl(END_OBJ *, int cmd, caddr_t data);
STATIC STATUS	bcm570xEndSend(END_OBJ *, M_BLK_ID mb);
STATIC STATUS	bcm570xEndStart(END_OBJ *p);
STATIC STATUS	bcm570xEndStop(END_OBJ *p);
STATIC STATUS	bcm570xEndPollSend(END_OBJ *p, M_BLK_ID mb);
STATIC STATUS	bcm570xEndPollReceive(END_OBJ *p, M_BLK_ID mb);
STATIC STATUS	bcm570xEndMcastAdd(END_OBJ *p, char *a);
STATIC STATUS	bcm570xEndMcastDel(END_OBJ *p, char *a);
STATIC STATUS	bcm570xEndMcastGet(END_OBJ *p, MULTI_TABLE *mt);

/* SENS Dispatch table */
STATIC NET_FUNCS bcm570xEndNetFuncs = {
    bcm570xEndStart,		/* Start Function */
    bcm570xEndStop,		/* Stop Function */
    bcm570xEndUnload,		/* Unload function */
    bcm570xEndIoctl,		/* IOCTL function */
    bcm570xEndSend,		/* Send Function */
    bcm570xEndMcastAdd,		/* Add mcast address function */
    bcm570xEndMcastDel,		/* Delete mcast address function */
    bcm570xEndMcastGet,		/* Get mcast address function */
    bcm570xEndPollSend,		/* Polling send function */
    bcm570xEndPollReceive,	/* Poling receive function */
    endEtherAddressForm,	/* Address formation function */
    endEtherPacketDataGet,	/* Packet data get function */
    endEtherPacketAddrGet	/* Packet address get function */
};

STATIC STATUS bcm570xEndPollStart(END_OBJ *pDrvCtrl);
STATIC STATUS bcm570xEndPollStop(END_OBJ *pDrvCtrl);


/* VLAN tag format (32 bits) */
typedef struct {
	ushort  tag;
	ushort  signature;
} vlan_tag_t;

/* Gigabit Ethernet buffer pool object */
typedef struct bcm570x_buff_t {
    unsigned int 	geu_init;	/* Initialized */
    bcm570x_end_t*      geu_devices;    /* Linked list of devices */
    M_CL_CONFIG		geu_blks;	/* mBlks/clBlks */
    CL_DESC		geu_cluster;	/* cluster memory */
    CL_POOL_ID		geu_clpool_id;	/* Cluster Pool ID */
    NET_POOL_ID		geu_netpool_id;	/* Net Pool ID  */
} bcm570x_buff_t;


static	unsigned char mac[6] = {0, 0, 0, 0, 0xf, 0xd};
static  unsigned char mac_zero[6] = {0,0,0,0,0,0};

static CACHE_FUNCS bcm570xCacheFuncs;
static CACHE_FUNCS *pCacheFuncs = &bcm570xCacheFuncs;

/* Default MAC address if an EEPROM or NVRAM not configured */
static unsigned char enetAdrs[6] = {0x0,0x10,0x18,0x04,0xf,0xd};

/* Used to pass the full-duplex flag, etc. */
int line_speed[MAX_UNITS] = {0,0,0,0};
static int full_duplex[MAX_UNITS] = {1,1,1,1};
static int rx_flow_control[MAX_UNITS] = {0,0,0,0};
static int tx_flow_control[MAX_UNITS] = {0,0,0,0};
static int auto_flow_control[MAX_UNITS] = {0,0,0,0};
static int tx_checksum[MAX_UNITS] = {1,1,1,1};
static int rx_checksum[MAX_UNITS] = {1,1,1,1};
static int auto_speed[MAX_UNITS] = {1,1,1,1};

#if JUMBO_FRAMES
/* Jumbo MTU for interfaces. */
static int mtu[MAX_UNITS] = {0,0,0,0};	
#endif

/* Turn on Wake-on lan for a device unit */
static int enable_wol[MAX_UNITS] = {0,0,0,0};

#define TX_DESC_CNT DEFAULT_TX_PACKET_DESC_COUNT
static unsigned int tx_pkt_desc_cnt[MAX_UNITS] =
	{TX_DESC_CNT,TX_DESC_CNT,TX_DESC_CNT, TX_DESC_CNT};

#define RX_DESC_CNT DEFAULT_STD_RCV_DESC_COUNT
static unsigned int rx_std_desc_cnt[MAX_UNITS] =
	{RX_DESC_CNT,RX_DESC_CNT,RX_DESC_CNT,RX_DESC_CNT};

static unsigned int rx_adaptive_coalesce[MAX_UNITS] = {1,1,1,1};

#if T3_JUMBO_RCV_RCB_ENTRY_COUNT
#define JBO_DESC_CNT DEFAULT_JUMBO_RCV_DESC_COUNT
static unsigned int rx_jumbo_desc_cnt[MAX_UNITS] =
	{JBO_DESC_CNT, JBO_DESC_CNT, JBO_DESC_CNT, JBO_DESC_CNT};
#endif
#define RX_COAL_TK DEFAULT_RX_COALESCING_TICKS
static unsigned int rx_coalesce_ticks[MAX_UNITS] =
	{RX_COAL_TK, RX_COAL_TK, RX_COAL_TK, RX_COAL_TK};

#define RX_COAL_FM DEFAULT_RX_MAX_COALESCED_FRAMES
static unsigned int rx_max_coalesce_frames[MAX_UNITS] =
	{RX_COAL_FM, RX_COAL_FM, RX_COAL_FM, RX_COAL_FM};

#define TX_COAL_TK DEFAULT_TX_COALESCING_TICKS
static unsigned int tx_coalesce_ticks[MAX_UNITS] =
	{TX_COAL_TK, TX_COAL_TK, TX_COAL_TK, TX_COAL_TK};

#define TX_COAL_FM DEFAULT_TX_MAX_COALESCED_FRAMES
static unsigned int tx_max_coalesce_frames[MAX_UNITS] =
	{TX_COAL_FM, TX_COAL_FM, TX_COAL_FM, TX_COAL_FM};

#define ST_COAL_TK DEFAULT_STATS_COALESCING_TICKS
static unsigned int stats_coalesce_ticks[MAX_UNITS] =
	{ST_COAL_TK, ST_COAL_TK, ST_COAL_TK, ST_COAL_TK};


/* Debugging options */
#define	DRV_DEBUG
#define	DRV_DEBUG_LOG
#define PKT_DEBUG 0
#define POLL_DEBUG 0

#ifdef PKT_DEBUG
int bcm570x_pkt_debug = 1;
#endif
int bcm570x_njafail_rxb=0;
int bcm570x_njafail_lnk=0;
int bcm570x_njafail_ipkt=0;
int bcm570x_npen=0;

/* defines */
#define LOG_MSG(X0, X1, X2, X3, X4, X5, X6)				\
	if (_func_logMsg != NULL)					\
	    _func_logMsg(X0, X1, X2, X3, X4, X5, X6);

#ifdef	DRV_DEBUG_LOG
#define DRV_DEBUG_OFF		0x0000
#define DRV_DEBUG_RX		0x0001
#define	DRV_DEBUG_TX		0x0002
#define DRV_DEBUG_INT		0x0004
#define	DRV_DEBUG_POLL		(DRV_DEBUG_POLL_RX | DRV_DEBUG_POLL_TX)
#define	DRV_DEBUG_POLL_RX	0x0008
#define	DRV_DEBUG_POLL_TX	0x0010
#define	DRV_DEBUG_LOAD		0x0020
#define	DRV_DEBUG_IOCTL		0x0040
#define	DRV_DEBUG_MEM		0x0080
#define DRV_DEBUG_POLL_REDIR	0x0100
#define	DRV_DEBUG_LOG_NVRAM	0x0200
#define DRV_DEBUG_IO            (DRV_DEBUG_RX|DRV_DEBUG_TX|DRV_DEBUG_INT)
#define DRV_DEBUG_STATS         0x0400
#define DRV_DEBUG_PERF          0x0800
#define DRV_DEBUG_PKT           0x1000
#define DRV_DEBUG_ALL           (DRV_DEBUG_PERF|DRV_DEBUG_STATS|\
				 DRV_DEBUG_RX|DRV_DEBUG_TX)

/* Debug flags */
int bcm570xChipDebug = DRV_DEBUG_ALL;

/* END object : global for debugging */
END_OBJ* tigonEnd = NULL;

#define DRV_LOG(FLG, X0, X1, X2, X3, X4, X5, X6)                        \
	if (bcm570xChipDebug & FLG)                                     \
            LOG_MSG(X0, X1, X2, X3, X4, X5, X6);

#else /* DRV_DEBUG_LOG */

#define DRV_LOG(DBG_SW, X0, X1, X2, X3, X4, X5, X6)

#endif /* DRV_DEBUG_LOG */

/* 
 * Legitimate values for BCM570x device types
 */
typedef enum {
	BCM5700VIGIL = 0,
	BCM5700A6,
	BCM5700T6,
	BCM5700A9,
	BCM5700T9,
	BCM5700,
	BCM5701A5,
	BCM5701T1,
	BCM5701T8,
	BCM5701A7,
	BCM5701A10,
	BCM5701A12,
	BCM5701,
	BCM5702,
	BCM5703,
	BCM5703A31,
	TC996T,
	TC996ST,
	TC996SSX,
	TC996SX,
	TC996BT,
	TC997T,
	TC997SX,
	TC1000T,
	TC940BR01,
	TC942BR01,
	NC6770,
	NC7760,
	NC7770,
	NC7780
} board_t;

/* Chip-Rev names for each device-type */
static struct {
    char* name;
} chip_rev[] = {
       {"BCM5700VIGIL"},
       {"BCM5700A6"},
       {"BCM5700T6"},
       {"BCM5700A9"},
       {"BCM5700T9"},
       {"BCM5700"},
       {"BCM5701A5"},
       {"BCM5701T1"},
       {"BCM5701T8"},
       {"BCM5701A7"},
       {"BCM5701A10"},
       {"BCM5701A12"},
       {"BCM5701"},
       {"BCM5702"},
       {"BCM5703"},
       {"BCM5703A31"},
       {"TC996T"},
       {"TC996ST"},
       {"TC996SSX"},
       {"TC996SX"},
       {"TC996BT"},
       {"TC997T"},
       {"TC997SX"},
       {"TC1000T"},
       {"TC940BR01"},
       {"TC942BR01"},
       {"NC6770"},
       {"NC7760"},
       {"NC7770"},
       {"NC7780"},
       {0}
};


/* indexed by board_t, above */
/* indexed by board_t, above */
static struct {
    char *name;
} board_info[] = {
	{ "Broadcom Vigil B5700 1000Base-T" },
	{ "Broadcom BCM5700 1000Base-T" },
	{ "Broadcom BCM5700 1000Base-SX" },
	{ "Broadcom BCM5700 1000Base-SX" },
	{ "Broadcom BCM5700 1000Base-T" },
	{ "Broadcom BCM5700" },
	{ "Broadcom BCM5701 1000Base-T" },
	{ "Broadcom BCM5701 1000Base-T" },
	{ "Broadcom BCM5701 1000Base-T" },
	{ "Broadcom BCM5701 1000Base-SX" },
	{ "Broadcom BCM5701 1000Base-T" },
	{ "Broadcom BCM5701 1000Base-T" },
	{ "Broadcom BCM5701" },
	{ "Broadcom BCM5702 1000Base-T" },
	{ "Broadcom BCM5703 1000Base-T" },
	{ "Broadcom BCM5703 1000Base-SX" },
	{ "3Com 3C996 10/100/1000 Server NIC" },
	{ "3Com 3C996 10/100/1000 Server NIC" },
	{ "3Com 3C996 Gigabit Fiber-SX Server NIC" },
	{ "3Com 3C996 Gigabit Fiber-SX Server NIC" },
	{ "3Com 3C996B Gigabit Server NIC" },
	{ "3Com 3C997 Gigabit Server NIC" },
	{ "3Com 3C997 Gigabit Fiber-SX Server NIC" },
	{ "3Com 3C1000 Gigabit NIC" },
	{ "3Com 3C940 Gigabit LOM (21X21)" },
	{ "3Com 3C942 Gigabit LOM (31X31)" },
	{ "Compaq NC6770 Gigabit Server Adapter" },
	{ "Compaq NC7760 Gigabit Server Adapter" },
	{ "Compaq NC7770 Gigabit Server Adapter" },
	{ "Compaq NC7780 Gigabit Server Adapter" },
	{ 0 },
};

/* PCI Devices which use the 570x chipset */
struct pci_device_id {
        /* Vendor and device ID or PCI_ANY_ID */
        unsigned short vendor_id, device_id;	
	/* Subsystem ID's or PCI_ANY_ID */
        unsigned short subvendor, subdevice;
        /* (class,subclass,prog-if) triplet */
        unsigned int class, class_mask;		
        /* Data private to the driver */
        unsigned long board_id;		
        int io_size, min_latency;
} bcm570xDevices[] = {
	{0x14e4, 0x1644, 0x1014, 0x0277, 0, 0, BCM5700VIGIL ,128,32},
	{0x14e4, 0x1644, 0x14e4, 0x1644, 0, 0, BCM5700A6 ,128,32},
	{0x14e4, 0x1644, 0x14e4, 0x2, 0, 0, BCM5700T6 ,128,32},
	{0x14e4, 0x1644, 0x14e4, 0x3, 0, 0, BCM5700A9 ,128,32},
	{0x14e4, 0x1644, 0x14e4, 0x4, 0, 0, BCM5700T9 ,128,32},
	{0x14e4, 0x1644, 0x1028, 0xd1, 0, 0, BCM5700 ,128,32},
	{0x14e4, 0x1644, 0x1028, 0x0106, 0, 0, BCM5700 ,128,32},
	{0x14e4, 0x1644, 0x1028, 0x0109, 0, 0, BCM5700 ,128,32},
	{0x14e4, 0x1644, 0x1028, 0x010a, 0, 0, BCM5700 ,128,32},
	{0x14e4, 0x1644, 0x10b7, 0x1000, 0, 0, TC996T ,128,32},
	{0x14e4, 0x1644, 0x10b7, 0x1001, 0, 0, TC996ST ,128,32},
	{0x14e4, 0x1644, 0x10b7, 0x1002, 0, 0, TC996SSX ,128,32},
	{0x14e4, 0x1644, 0x10b7, 0x1003, 0, 0, TC997T ,128,32},
	{0x14e4, 0x1644, 0x10b7, 0x1005, 0, 0, TC997SX ,128,32},
	{0x14e4, 0x1644, 0x10b7, 0x1008, 0, 0, TC942BR01 ,128,32},
	{0x14e4, 0x1644, PCI_ANY_ID, PCI_ANY_ID, 0, 0, BCM5700 ,128,32},
	{0x14e4, 0x1645, 0x14e4, 1, 0, 0, BCM5701A5 ,128,32},
	{0x14e4, 0x1645, 0x14e4, 5, 0, 0, BCM5701T1 ,128,32},
	{0x14e4, 0x1645, 0x14e4, 6, 0, 0, BCM5701T8 ,128,32},
	{0x14e4, 0x1645, 0x14e4, 7, 0, 0, BCM5701A7 ,128,32},
	{0x14e4, 0x1645, 0x14e4, 8, 0, 0, BCM5701A10 ,128,32},
	{0x14e4, 0x1645, 0x14e4, 0x8008, 0, 0, BCM5701A12 ,128,32},
	{0x14e4, 0x1645, 0x0e11, 0xc1, 0, 0, NC6770 ,128,32},
	{0x14e4, 0x1645, 0x0e11, 0x7c, 0, 0, NC7770 ,128,32},
	{0x14e4, 0x1645, 0x0e11, 0x85, 0, 0, NC7780 ,128,32},
	{0x14e4, 0x1645, 0x1028, 0x0121, 0, 0, BCM5701 ,128,32},
	{0x14e4, 0x1645, 0x10b7, 0x1004, 0, 0, TC996SX ,128,32},
	{0x14e4, 0x1645, 0x10b7, 0x1006, 0, 0, TC996BT ,128,32},
	{0x14e4, 0x1645, 0x10b7, 0x1007, 0, 0, TC1000T ,128,32},
	{0x14e4, 0x1645, 0x10b7, 0x1008, 0, 0, TC940BR01 ,128,32},
	{0x14e4, 0x1645, PCI_ANY_ID, PCI_ANY_ID, 0, 0, BCM5701 ,128,32},
	{0x14e4, 0x1646, 0x14e4, 0x8009, 0, 0, BCM5702 ,128,32},
	{0x14e4, 0x1646, 0x0e11, 0xbb, 0, 0, NC7760 ,128,32},
	{0x14e4, 0x1646, PCI_ANY_ID, PCI_ANY_ID, 0, 0, BCM5702 ,128,32},
	{0x14e4, 0x16a6, 0x14e4, 0x8009, 0, 0, BCM5702 ,128,32},
	{0x14e4, 0x16a6, 0x0e11, 0xbb, 0, 0, NC7760 ,128,32},
	{0x14e4, 0x16a6, PCI_ANY_ID, PCI_ANY_ID, 0, 0, BCM5702 ,128,32},
	{0x14e4, 0x1647, 0x14e4, 0x0009, 0, 0, BCM5703 ,128,32},
	{0x14e4, 0x1647, 0x14e4, 0x000a, 0, 0, BCM5703A31 ,128,32},
	{0x14e4, 0x1647, 0x14e4, 0x000b, 0, 0, BCM5703 ,128,32},
	{0x14e4, 0x1647, 0x14e4, 0x800a, 0, 0, BCM5703 ,128,32},
	{0x14e4, 0x1647, 0x0e11, 0x9a, 0, 0, NC7770 ,128,32},
	{0x14e4, 0x1647, 0x0e11, 0x99, 0, 0, NC7780 ,128,32},
	{0x14e4, 0x1647, PCI_ANY_ID, PCI_ANY_ID, 0, 0, BCM5703 ,128,32},
	{0x14e4, 0x16a7, 0x14e4, 0x0009, 0, 0, BCM5703 ,128,32},
	{0x14e4, 0x16a7, 0x14e4, 0x000a, 0, 0, BCM5703A31 ,128,32},
	{0x14e4, 0x16a7, 0x14e4, 0x000b, 0, 0, BCM5703 ,128,32},
	{0x14e4, 0x16a7, 0x14e4, 0x800a, 0, 0, BCM5703 ,128,32},
	{0x14e4, 0x16a7, 0x0e11, 0x9a, 0, 0, NC7770 ,128,32},
	{0x14e4, 0x16a7, 0x0e11, 0x99, 0, 0, NC7780 ,128,32},
	{0x14e4, 0x16a7, PCI_ANY_ID, PCI_ANY_ID, 0, 0, BCM5703 ,128,32}
};

#define n570xDevices   (sizeof(bcm570xDevices)/sizeof(bcm570xDevices[0]))

#ifdef INCLUDE_MIBII_RFC2233
#define END_HADDR(pEnd)                                                 \
        ((pEnd)->pMib2Tbl->m2Data.mibIfTbl.ifPhysAddress.phyAddress)

#define END_HADDR_LEN(pEnd)                                             \
        ((pEnd)->pMib2Tbl->m2Data.mibIfTbl.ifPhysAddress.addrLength)
#else
#define END_HADDR(pEnd)                                                 \
              ((pEnd)->mib2Tbl.ifPhysAddress.phyAddress)

#define END_HADDR_LEN(pEnd)                                             \
              ((pEnd)->mib2Tbl.ifPhysAddress.addrLength)
#endif /* INCLUDE_MIBII_RFC2233 */

#define D5703(a)
/*
 * Purpose:	One entry for each 570x unit to not allow one unit to 
 *		use up all memory, and keep reference counts. 
 * Notes: 	Since the driver only uses one size of cluster, each 
 *		element in the array for for a different 570x unit. 
 */
static bcm570x_buff_t  tigonBuffPool[MAX_UNITS];

/*
 * Debug Utility: Dump a packet descriptor received.
 */
void
bcm570xPktDump(char* pfx, void* data, int size)
{
    int i;
    unsigned char* ptr8 = (unsigned char*)data;
    unsigned short* ptr16 = (unsigned short*)data;    
    /* if(ptr8[0] != 0xff){ */
    printf("DMA Addr: 0x%x\n", (unsigned int)data);

    /* show  packet type and len */
    if(ptr8[0] == 0xff)
	printf("%s: broadcast %d bytes ----\n", pfx, size);	
    else if((ptr8[0] & 0x1) && (ptr8[0] != 0xff))
	printf("%s: multicast %d bytes ----\n", pfx, size);	
    else
	printf("%s: unicast %d bytes ----\n", pfx, size);

    /* Solaris snoop output */

    printf("\t 0: ");
    ptr16 = (unsigned short*)data;
    for ( i = 0; i < size/2; i++){
	printf("%4.4x ", ptr16[i]);
	if( (((i+1) % 8) == 0) && (i > 0))
	    printf("\n\t%2.2d: ",(i+1)*2);
    }
    printf("\n");
    /* } */
}

/*
 * Dump packet buffer pools.
 */
void
bcm570xMemShow (int u)
{
    IMPORT void 	netPoolShow(NET_POOL_ID);
    bcm570x_buff_t*	geu = &tigonBuffPool[u];

    if ((u < MAX_UNITS) && geu->geu_init) {
	printf("BCM570x NET POOL: unit %d\n", u);
	netPoolShow(tigonBuffPool[u].geu_netpool_id);
    } else {
	printf("Invalid unit: %d\n", u);
    }
}

/*
 * Debug utility: dump PCI configuration space.
 */
void
showBCM570xConfig(int bus, int dev, int func)
{
    unsigned int tmp ;
#if (CPU == RC32364 || CPU == MIPS32)
    sysPciBusErrDisable();
#endif
    pciConfigInLong(bus, dev,func, PCI_CFG_BASE_ADDRESS_0, &tmp);
    printf("MBAR0(0x10): 0x%x\n", tmp);
    pciConfigInLong(bus, dev,func, PCI_CFG_BASE_ADDRESS_1, &tmp);
    printf("MBAR1(0x14): 0x%x\n", tmp);
    pciConfigInLong(bus, dev,func, PCI_CFG_BASE_ADDRESS_2, &tmp);
    printf("MBAR2(0x18): 0x%x\n", tmp);
    pciConfigInLong(bus, dev,func, PCI_CFG_BASE_ADDRESS_3, &tmp);
    printf("MBAR3(0x1c): 0x%x\n", tmp);
    pciConfigInLong(bus, dev,func, PCI_CFG_BASE_ADDRESS_4, &tmp);
    printf("MBAR4(0x20): 0x%x\n", tmp);
    pciConfigInLong(bus, dev,func, PCI_CFG_BASE_ADDRESS_5, &tmp);
    printf("MBAR5(0x24): 0x%x\n", tmp);
    pciConfigInLong(bus, dev,func, 0x68, &tmp);
    printf("HOST_CONTROL(0x68): 0x%x\n", tmp);
    pciConfigInLong(bus, dev,func, 0x70, &tmp);
    printf("PCI_STATE(0x70): 0x%x\n", tmp);
    pciConfigInLong(bus, dev,func, 0x78, &tmp);
    printf("REG_BASE(0x78): 0x%x\n", tmp);    
    pciConfigInLong(bus, dev,func, 0x7c, &tmp);
    printf("MEM_WIN_BAR(0x7c): 0x%x\n", tmp);
    pciConfigInLong(bus, dev,func, 0x80, &tmp);
    printf("REG_DATA_REG(0x80): 0x%x\n", tmp);
    pciConfigInLong(bus, dev,func, 0x84, &tmp);
    printf("MEM_WIN_DATA_REG(0x84): 0x%x\n", tmp);
#if (CPU == RC32364 || CPU == MIPS32)
    sysPciBusErrEnable();
#endif
}

/*
 * Allocate a packet buffer from the bcm570x packet pool.
 */
STATIC void *
bcm570xPktAlloc(int u, int pksize)
{
    void 	*p;
    bcm570x_buff_t  *bpu = &tigonBuffPool[u];
    p = netClusterGet(bpu->geu_netpool_id, bpu->geu_clpool_id);
    DRV_LOG(DRV_DEBUG_PKT, "pkt_alloc: t=%d, addr=0x%x, size=%d, unit=%d\n",
	    time(0), (unsigned int)p, pksize, u, 0,0);
    bpu->geu_devices->bcm570x_allocs++;
    return(p);
}

/*
 * Free a packet previously allocated from the bcm570x packet
 * buffer pool.
 */
STATIC void
bcm570xPktFree(int u, void *p)
{
    bcm570x_buff_t	*bpu = &tigonBuffPool[u];
    DRV_LOG(DRV_DEBUG_PKT, "pkt_free: t=%d addr=0x%x, unit=%d\n",
	    time(0), (unsigned int)p, u, 0, 0,0);
    netClFree(bpu->geu_netpool_id, p);
    bpu->geu_devices->bcm570x_frees++;
}

/*
 * When a receive packet interrupt is received by the 570x device,
 * a netJob task is started using the routine below. The job of
 * this routine is to construct an MBLK for the SENS stack, and
 * perform any auxiliary packet processing before gived to the
 * END Mux module.
 */
STATIC void
bcm570xReceiveNetJob(int devPtr, int pckti, int pckt_size, int pPkt)
{
    PLM_DEVICE_BLOCK pDevice = (PLM_DEVICE_BLOCK)devPtr;
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    bcm570x_end_t	*ge = (bcm570x_end_t *)pUmDevice->end;
    struct end_object 	*eo = &ge->ge_eo;
    void		*pckt = (void *)pckti;
    PLM_PACKET          pPacket = (PLM_PACKET)pPkt;
    PUM_PACKET          pUmPacket = (PUM_PACKET)pPacket;
#if PKT_DEBUG
    int                 flags = pPacket->Flags;
#endif
    CL_BLK_ID		cb = NULL;
    M_BLK_ID		mb = NULL;

#if PKT_DEBUG
    if(bcm570x_pkt_debug){
	/* All frames are 16byte aligned */
	printf("RX:");	
	if(flags & RCV_BD_FLAG_TCP_PACKET)
	    printf("TCP ");
	if(flags & RCV_BD_FLAG_END)
	    printf("frame ");
	if(flags & RCV_BD_FLAG_JUMBO_RING)
	    printf("jumbo ");
	if(flags & RCV_BD_FLAG_VLAN_TAG)
	    printf("vlan ");
	if(flags & RCV_BD_FLAG_FRAME_HAS_ERROR)
	    printf("ERROR ");
	if(flags & RCV_BD_FLAG_IP_CHKSUM_FIELD)
	    printf("+ip chksum ok ");
	if(flags & RCV_BD_FLAG_TCP_UDP_CHKSUM_FIELD)
	    printf("+tcp/udp chksum ok" );
	printf("\n");
    
	bcm570xPktDump("RX", (void*)pckt, pckt_size);
    }
#endif
    /* 
     * Build Cluster - then attach to mblks, 1 for each interface that 
     * should get the packet. Broadcast/Multicast packets go up all the stacks.
     *
     * For IPMC, it should only go up stacks that have registered for that 
     * address, but for now, this will allow multiple interfaces to work.
     */
    
    if ((NULL == (cb = netClBlkGet(eo->pNetPool, M_DONTWAIT)))) {
	netClFree(eo->pNetPool, pckt);	/* Free packet */
#ifdef INCLUDE_MIBII_RFC2233
    if (eo->pMib2Tbl != NULL)
        {
        eo->pMib2Tbl->m2CtrUpdateRtn(eo->pMib2Tbl,
                                                    M2_ctrId_ifOutErrors,1);
        }
#else
        END_ERR_ADD(eo, MIB2_IN_ERRS, +1);
#endif /* INCLUDE_MIBII_RFC2233 */
	printf("bcm570xReceiveNetJob: out of memory!\n");
	ge->bcm570x_rx_errs++;
	return;
    }

    /* 1 MBLK for each interface */
    if ((mb = mBlkGet(eo->pNetPool, M_DONTWAIT, MT_DATA))) {
	netClBlkJoin(cb, pckt, BCM570X_END_PK_SZ, NULL, 0, 0, 0);
	netMblkClJoin(mb, cb);
	mb->mBlkPktHdr.len = mb->mBlkHdr.mLen = pckt_size;
	mb->mBlkHdr.mFlags |= M_PKTHDR;

#if PKT_DEBUG
	if(flags & RCV_BD_FLAG_VLAN_TAG) { 
	    printf("bcm570xReceiveNetJob: tagged frame received.\n");
	}
#endif
    }
    else {
	netClBlkFree(eo->pNetPool, cb);
    bcm570xPktFree (pUmDevice->index, pckt);
	printf("mBlkGet: could not get packet!\n");
	ge->bcm570x_rx_errs++;
	return;
    }


    /*
     * Fix IP header access bug in VxWorks on certain architectures
     *
     * The code below solves an alignment problem when the CPU and the
     * ethernet chip don't accept longword unaligned addresses.
     *
     * When the ethernet chip receives a packet from the network,
     * it needs a longword aligned buffer to copy the data. To process the
     * IP packet, MUX layer adds a ENET_HDR_REAL_SIZ (0x14) offset to the
     * data buffer. So the CPU obtains a longword unaligned buffer and
     * a fault execption occurs when it reads "ip_src" and "ip_dst" fields
     * in the IP structure (ip_src and ip_dst fields are longwords).
     *
     * The problem is solved by copying the data to the correct offset
     * into the MBLK buffer where the IP structure is aligned.
     */
    if ( ge->align != 0) { 
#ifndef USE_ZERO_COPY_RX
      memcpy((void *)((unsigned int)pckt + ge->align), pckt, pckt_size);
#endif
      mb->mBlkHdr.mData += ge->align;
    }

    /* Give packet to VxWorks SENS Stack */
    END_RCV_RTN_CALL(eo, mb);
    ge->bcm570x_n_rx++;

    /* Replenish RX buffer for device while VxWorks has this one */
    pckt = bcm570xPktAlloc(pUmDevice->index,
			   pPacket->u.Rx.RxBufferSize + 2);
    if (pckt == 0) {
	pUmPacket->skbuff = 0;
	QQ_PushTail(&pUmDevice->rx_out_of_buf_q.Container, pPacket);
    }
    else {
	pUmPacket->skbuff = pckt;
	pUmPacket->mb = (M_BLK_ID)0;
	QQ_PushTail(&pDevice->RxPacketFreeQ.Container, pPacket);
    }
}


/*
 * Initialize END memory pools for the specified BCM570x
 * Parameters:	u - unit number to initialize.
 *		pksize - Packet size to use, MUST BE MTU size or greater.
 *		clblks - number of cluster block headers this unit
 *		cl - number of actual cluster buffers this unit.
 *		mblks - number of mblks this unit.		
 * Returns:	OK or error if failed.
 * Notes:	If already initialized, nothing is done. 
 */
STATIC STATUS
bcm570xMemPoolAlloc(bcm570x_end_t *ge,
		    int pksize, int clblks, int cl, int mblks)
{
    bcm570x_buff_t  *geu = &tigonBuffPool[ge->ge_unit];
    int sz = 0;
    
    DRV_LOG(DRV_DEBUG_MEM, "bcm570xMemPoolAlloc: Unit %d %sinitialized\n", 
	    ge->ge_unit, geu->geu_init ? "" : "not ", 0, 0,0,0);
 
    /*
     * If already done, skip the allocation and just assign the required 
     * values.
     */
    if (!geu->geu_init) {
	geu->geu_init = TRUE;
	geu->geu_netpool_id = (NET_POOL_ID)calloc(1, sizeof(NET_POOL));
	sz += sizeof(NET_POOL);
	/* First Initialize the CL configuration */

	geu->geu_blks.mBlkNum = mblks;
	geu->geu_blks.clBlkNum= clblks;
	geu->geu_blks.memSize = (mblks * (M_BLK_SZ + sizeof(long))) 
	    + (clblks * (CL_BLK_SZ + sizeof(long)));
        geu->geu_blks.memArea =
	    (char *)memalign (sizeof (long), geu->geu_blks.memSize);
	sz += geu->geu_blks.memSize;
	/* Now initialize the actual cluster buffers that contain packets */

	geu->geu_cluster.clNum = cl;
	geu->geu_cluster.clSize = pksize;
	geu->geu_cluster.memSize = cl*(pksize + sizeof(long)) + sizeof(long);

        /*
        * If flushRtn is NULL, then memory can be allocated that
        * doesn't need to be flushed or invalidated for DMA.  Or, in the case
        * of MIPS, it means that we'd rather use uncached buffers.
        */
        if (pCacheFuncs->flushRtn == NULL)
            geu->geu_cluster.memArea = cacheDmaMalloc(geu->geu_cluster.memSize);
        else
            geu->geu_cluster.memArea = malloc(geu->geu_cluster.memSize);

	sz += geu->geu_cluster.memSize;

	/* Now set up the pool and GO! */

	if (!geu->geu_blks.memArea ||
	    !geu->geu_cluster.memArea || !geu->geu_netpool_id ||
	    (OK != netPoolInit(geu->geu_netpool_id, &geu->geu_blks,
			       &geu->geu_cluster, 1, NULL)))
            {
	    printf("bcm570xMemPoolAlloc: Failed for unit %d\n", ge->ge_unit);
	    if (geu->geu_blks.memArea) {
		free(geu->geu_blks.memArea);
	    }
	    if (geu->geu_cluster.memArea) {
                if (pCacheFuncs->flushRtn == NULL)
                    cacheDmaFree(geu->geu_cluster.memArea);
                else
                    free(geu->geu_cluster.memArea);
            }
	    if (geu->geu_netpool_id) {
		free(geu->geu_netpool_id);
	    }
	    bzero((void *)geu, sizeof(*geu));
	    return(ERROR);
	}
	geu->geu_clpool_id = netClPoolIdGet(geu->geu_netpool_id, pksize, 
					    FALSE);

	DRV_LOG(DRV_DEBUG_MEM,
		"bcm570xMemPoolAlloc: unit %d initialized: %d bytes\n",
		ge->ge_unit,sz,0, 0,0,0);
    }
    ge->ge_eo.pNetPool	= geu->geu_netpool_id;

    return(OK);
}

/*
 * Replenish packet buffers available to the chip.
 * This is usually called by a task which checks to see
 * if we are low on packet memory, and if so, then we either re-use
 * or allocate more packets.
 * Returns 1 if not all buffers are allocated
 */
STATIC int
bcm570xReplenishRxBuffers(PUM_DEVICE_BLOCK pUmDevice)
{
    PLM_PACKET pPacket;
    PUM_PACKET pUmPacket;
    PLM_DEVICE_BLOCK pDevice = (PLM_DEVICE_BLOCK) pUmDevice;
    void *skb;
    int queue_rx = 0;
    int ret = 0;

    DRV_LOG(DRV_DEBUG_PKT, "bcm570xReplenishRxBuffers: 0x%x\n",
	    pUmDevice, 0,0, 0,0,0);
    
    while ((pUmPacket = (PUM_PACKET)
	    QQ_PopHead(&pUmDevice->rx_out_of_buf_q.Container)) != 0) {

	pPacket = (PLM_PACKET) pUmPacket;

	/* reuse an old skb */
	if (pUmPacket->skbuff) {
	    QQ_PushTail(&pDevice->RxPacketFreeQ.Container, pPacket);
	    queue_rx = 1;
	    continue;
	}
	if ( ( skb = bcm570xPktAlloc(pUmDevice->index,
				     pPacket->u.Rx.RxBufferSize + 2)) == 0) {
	    QQ_PushHead(&pUmDevice->rx_out_of_buf_q.Container,pPacket);
	    printf("NOTICE: Out of RX memory.\n");
	    ret = 1;
	    break;
	}

	pUmPacket->skbuff = skb;
	QQ_PushTail(&pDevice->RxPacketFreeQ.Container, pPacket);
	queue_rx = 1;
    }

    if (queue_rx) {
	LM_QueueRxPackets(pDevice);
    }

    return ret;
}

/*
 * BCM570x interrupt service routine. This routine is bound to the
 * H/W interrupt via intConnect().
 */
STATIC VOID
bcm570xIntr(VOID* arg)
{
    bcm570x_end_t* ge = (bcm570x_end_t*)arg;
    struct end_object 	*eo = &ge->ge_eo;
    PLM_DEVICE_BLOCK pDevice = (PLM_DEVICE_BLOCK)ge->pDevice;
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK)ge->pUmDevice;    
    LM_UINT32 oldtag, newtag;
    int i;
    int nja_rv=0;
    
    DRV_LOG(DRV_DEBUG_INT, "bc%d: interrupt!\n", ge->ge_unit, 0,0, 0,0,0);

    pUmDevice->interrupt = 1;

    if (pDevice->UseTaggedStatus) {
	if ((pDevice->pStatusBlkVirt->Status & STATUS_BLOCK_UPDATED) ||
	    pUmDevice->adapter_just_inited) {
            MB_REG_WR(pDevice, Mailbox.Interrupt[0].Low, 1);
            oldtag = pDevice->pStatusBlkVirt->StatusTag;

            for (i = 0; ; i++) {
                pDevice->pStatusBlkVirt->Status &= ~STATUS_BLOCK_UPDATED;

                LM_ServiceInterrupts(pDevice);
                newtag = pDevice->pStatusBlkVirt->StatusTag;
                if ((newtag == oldtag) || (i > 50)) {
                    MB_REG_WR(pDevice, Mailbox.Interrupt[0].Low, newtag << 24);
                    if (pDevice->UndiFix) {
                        REG_WR(pDevice, Grc.LocalCtrl,
                        pDevice->GrcLocalCtrl | 0x2);
                    }
                    break;
                 }
                 oldtag = newtag;
             }
	}	
    }
    else {	
	while (pDevice->pStatusBlkVirt->Status & STATUS_BLOCK_UPDATED) {
	    unsigned int dummy;
	    
	    pDevice->pMemView->Mailbox.Interrupt[0].Low = 1;
	    pDevice->pStatusBlkVirt->Status &= ~STATUS_BLOCK_UPDATED;
	    LM_ServiceInterrupts(pDevice);
	    pDevice->pMemView->Mailbox.Interrupt[0].Low = 0;
	    dummy = pDevice->pMemView->Mailbox.Interrupt[0].Low;
	}
    }
    /* Allocate new RX buffers */
    if (QQ_GetEntryCnt(&pUmDevice->rx_out_of_buf_q.Container)) {
	if(ge->polled_mode) {
	    bcm570xReplenishRxBuffers(pUmDevice);
	} else {
	    nja_rv = netJobAdd((FUNCPTR)bcm570xReplenishRxBuffers,
		      (int)pUmDevice,
		      0,
		      0,
		      0,
		      0xBBBBBBBB);
        if (nja_rv == ERROR) {
            bcm570x_njafail_rxb++;
            if (bcm570x_npen) { sysSerialPrintString("570xIntr:RXB\n"); }
        }
	}
    }
    /* Queue packets */
    if (QQ_GetEntryCnt(&pDevice->RxPacketFreeQ.Container)) {
	LM_QueueRxPackets(pDevice);
    }
    
    if (pUmDevice->tx_queued) {
	pUmDevice->tx_queued = 0;
	/* netif_wake_queue(dev); */
    }

    if(pUmDevice->tx_full){
	if(pDevice->LinkStatus != LM_STATUS_LINK_DOWN){
	    printf("NOTICE: tx was previously blocked, restarting MUX\n");
	    if(ge->polled_mode){
		muxTxRestart(eo);
	    } else {
		nja_rv = netJobAdd ((FUNCPTR)muxTxRestart, (int)eo, 0, 0,
			   0, 0xCCCCCCCC);
            if (nja_rv == ERROR) {
                bcm570x_njafail_lnk++;
                if (bcm570x_npen) { sysSerialPrintString("570xIntr:LNK\n"); }
            }
	    }
	    pUmDevice->tx_full = 0;
	}
    }
    
    pUmDevice->interrupt = 0;

    return;
}

STATIC VOID
bcm570xIntrIntContext(VOID* arg)
{
    bcm570x_end_t* ge = (bcm570x_end_t*)arg;
	if (ge->polled_mode == 0) {
        bcm570xIntr(arg);
    }
}


#ifdef BCM570X_ADAPTIVE_RX
/*
 * Adaptive Coalescing of packets.
 * Measures stats to adapt to ingress traffic.
 */
STATIC int
bcm570xAdaptRxCoalesce(PUM_DEVICE_BLOCK pUmDevice)
{
    PLM_DEVICE_BLOCK pDevice = &pUmDevice->lm_dev;
    unsigned int rx_curr_cnt, tx_curr_cnt, rx_delta, tx_delta, total_delta;


    rx_curr_cnt = pDevice->pStatsBlkVirt->ifHCInUcastPkts.Low;
    tx_curr_cnt = pDevice->pStatsBlkVirt->COSIfHCOutPkts[0].Low;

    if ((rx_curr_cnt <= pUmDevice->rx_last_cnt) ||
	(tx_curr_cnt <= pUmDevice->tx_last_cnt)) {
	
	/* skip if there is counter rollover */
	pUmDevice->rx_last_cnt = rx_curr_cnt;
	pUmDevice->tx_last_cnt = tx_curr_cnt;
	return 0;
    }

    rx_delta = rx_curr_cnt - pUmDevice->rx_last_cnt;
    tx_delta = tx_curr_cnt - pUmDevice->tx_last_cnt;
    total_delta = rx_delta + tx_delta;
    pUmDevice->rx_last_cnt = rx_curr_cnt;
    pUmDevice->tx_last_cnt = tx_curr_cnt;
    
    if (total_delta < rx_delta){
        printf("total_delta (%d) < rx_delta(%d)\n",
               total_delta, rx_delta);
	return 0;
    }

#if 0
    printf("rx_adapt: t_del: %d pkts,rx_del: %d pkts, tx_del: %d pkts\n",
	   total_delta, rx_delta, tx_delta);
#endif

    if (total_delta < ADAPTIVE_LO_PKT_THRESH) {
	if (pUmDevice->rx_curr_coalesce_frames !=
	    ADAPTIVE_LO_RX_MAX_COALESCED_FRAMES) {
	    
	    pUmDevice->rx_curr_coalesce_frames =
		ADAPTIVE_LO_RX_MAX_COALESCED_FRAMES;
	    pUmDevice->rx_curr_coalesce_ticks =
		ADAPTIVE_LO_RX_COALESCING_TICKS;
	    
	    REG_WR(pDevice, HostCoalesce.RxMaxCoalescedFrames,
		   ADAPTIVE_LO_RX_MAX_COALESCED_FRAMES);
	    
	    REG_WR(pDevice, HostCoalesce.RxCoalescingTicks,
		   ADAPTIVE_LO_RX_COALESCING_TICKS);
	}
    }
    else if (total_delta < ADAPTIVE_HI_PKT_THRESH) {
	if (pUmDevice->rx_curr_coalesce_frames !=
	    DEFAULT_RX_MAX_COALESCED_FRAMES) {
	    
	    pUmDevice->rx_curr_coalesce_frames = 
		DEFAULT_RX_MAX_COALESCED_FRAMES;
	    pUmDevice->rx_curr_coalesce_ticks =
		DEFAULT_RX_COALESCING_TICKS;
	    
	    REG_WR(pDevice, HostCoalesce.RxMaxCoalescedFrames,
		   DEFAULT_RX_MAX_COALESCED_FRAMES);
	    
	    REG_WR(pDevice, HostCoalesce.RxCoalescingTicks,
		   DEFAULT_RX_COALESCING_TICKS);
	}
    }
    else {
	if (pUmDevice->rx_curr_coalesce_frames !=
	    ADAPTIVE_HI_RX_MAX_COALESCED_FRAMES) {
	    
	    pUmDevice->rx_curr_coalesce_frames = 
		ADAPTIVE_HI_RX_MAX_COALESCED_FRAMES;
	    pUmDevice->rx_curr_coalesce_ticks =
		ADAPTIVE_HI_RX_COALESCING_TICKS;
	    
	    REG_WR(pDevice, HostCoalesce.RxMaxCoalescedFrames,
		   ADAPTIVE_HI_RX_MAX_COALESCED_FRAMES);
	    
	    REG_WR(pDevice, HostCoalesce.RxCoalescingTicks,
		   ADAPTIVE_HI_RX_COALESCING_TICKS);

	}
    }
    return 0;
}	
#endif /* BCM570X_ADAPTIVE_RX */


/******************************************************************************
*
* endTok_r - get a token string (modified version)
*
* This modified version can be used with optional parameters.  If the
* parameter is not specified, this version returns NULL.  It does not
* signify the end of the original string, but that the parameter is null.
*
* .CS
*
*    /@ required parameters @/
*
*    string = endTok_r (initString, ":", &pLast);
*    if (string == NULL)
*        return ERROR;
*    reqParam1 = strtoul (string);
*
*    string = endTok_r (NULL, ":", &pLast);
*    if (string == NULL)
*        return ERROR;
*    reqParam2 = strtoul (string);
*
*    /@ optional parameters @/
*
*    string = endTok_r (NULL, ":", &pLast);
*    if (string != NULL)
*        optParam1 = strtoul (string);
*
*    string = endTok_r (NULL, ":", &pLast);
*    if (string != NULL)
*        optParam2 = strtoul (string);
* .CE
*/
 
static char * endTok_r
    (
    char *       string,      /* string to break into tokens */
    const char * separators,  /* the separators */
    char **      ppLast               /* pointer to serve as string index */
    )
    {
    if ((string == NULL) && ((string = *ppLast) == NULL))
      return (NULL);

    if ((*ppLast = strpbrk (string, separators)) != NULL)
      *(*ppLast)++ = EOS;

    /* Return NULL, if string is empty */
    if (*string == EOS)
      return NULL;

    return (string);
    }

/*
 * Load BCM570x SENS driver for VxWorks; main entry point to start driver.
 */
END_OBJ* bcm570xEndLoad(
			char *initString, 	
			void* ap
			)
{
    int BusNo;
    int DevNo;
    int FuncNo;
    int j, i, rv, iline, devFound = FALSE;
    unsigned int net_unit;
    pci_dev* pci_device;
    bcm570x_end_t  *ge;
    bcm570x_buff_t *geu;
    END_OBJ *eo = NULL;
    char dev_name[20];
    char * tok;
    char * pLast;
    int devConfigCommand = (PCI_CMD_IO_ENABLE  |
			    PCI_CMD_MEM_ENABLE |
			    PCI_CMD_MASTER_ENABLE);
    
    if (initString == NULL) {
	printf("bcm570xEndLoad: unexpected NULL pointer for device name\n");
	return NULL;
    }
     else if (*initString == '\0') {
	 /* Copy in our device name */
	strcpy(initString, "bc");
    D5703("device string bc\n");
	return NULL;
     }

    DRV_LOG (DRV_DEBUG_LOAD, "Loading bc: init_str:%s ...\n",
	     initString, 0, 0, 0, 0, 0);

    /* Setup END structure */

    if ((ge = malloc (sizeof (bcm570x_end_t))) == NULL) {
        printf("bcm570xEndLoad: failed to allocate memory for %s\n",
	       initString);
        return NULL;
    }

    D5703("5703 CHK-1\n");

    bzero ((void *) ge, sizeof (*ge));
    eo = &ge->ge_eo;

    /* Parse init string, format is "net_unit:mbar:irq:align" */

    tok = endTok_r (initString, ":", &pLast);

    if (tok == NULL) {
        free (ge);
	printf ("bcm570xEndLoad parse error:  unit_num\n");
        return NULL;
    }
    D5703("5703 CHK-2\n");

    net_unit = atoi (tok);

    if (net_unit >= MAX_UNITS) {
	printf ("bcm570xEndLoad: Invalid ge device: %d\n", net_unit);
        free (ge);
        return NULL;
    }

    D5703("5703 CHK-3\n");
    ge->ge_unit = net_unit;
    geu = &tigonBuffPool[ge->ge_unit];

    tok = endTok_r (NULL, ":", &pLast);
    if (tok == NULL){
        free (ge);
	printf ("bcm570xEndLoad parse error:  mbar, "
		"enter hex PCI memory base\n");
        return NULL;
    }
    D5703("5703 CHK-4\n");

    /* Get provided memory base address */
    ge->mbar = strtoul (tok, NULL, 16);
     
    tok = endTok_r (NULL, ":", &pLast);
    if (tok == NULL){
        free (ge);
	printf ("bcm570xEndLoad parse error:  irq\n");
        return NULL;
    }

    D5703("5703 CHK-5\n");
    iline = atoi(tok);
    ge->irq = iline;

    tok = endTok_r (NULL, ":", &pLast);
    if (tok != NULL) {
        ge->align = atoi (tok);
    }
    else{
        ge->align = 0;
    }
#if (CPU == RC32364 || CPU == MIPS32)
    /* On RC3233x, disable bus errors before PCI probe */
    sysPciBusErrDisable();
#endif

    D5703("5703 CHK-6\n");
    /* Find PCI device, if it exists, configure ...  */
    for( i = 0; i < n570xDevices; i++){
	if (pciFindDevice(bcm570xDevices[i].vendor_id,
			  bcm570xDevices[i].device_id,
			  0,
			  &BusNo, &DevNo, &FuncNo) != ERROR) {

            /* Show status on LCD on IDT MIPS CPU */
#if (CPU == RC32364 || CPU == MIPS32)
           sysLedDsply('G', 'I', 'G', 'E');
#endif
	    /* Set ILINE */
	    pciConfigOutByte(BusNo,DevNo,FuncNo,
			     PCI_CFG_DEV_INT_LINE,iline);

	    /*
	     * 0x10 - 0x14 define one 64-bit MBAR.
	     * 0x14 is the higher-order address bits of the BAR.
	     */
	    pciConfigOutLong(BusNo, DevNo, FuncNo,
			     PCI_CFG_BASE_ADDRESS_1, 0);
	    pciConfigOutLong(BusNo, DevNo, FuncNo,
			     PCI_CFG_BASE_ADDRESS_0, ge->mbar);

	    /*
	     * Enable PCI memory, IO, and Master -- don't
	     * reset any status bits in doing so.
	     */
	    pciConfigModifyLong (BusNo, DevNo, FuncNo, PCI_CFG_COMMAND,
  		      (PCI_CMD_MASK | devConfigCommand), devConfigCommand);

	    printf("\n%s: bus %d, device %d, function %d: MBAR=0x%x\n",
		   board_info[bcm570xDevices[i].board_id].name,
		   BusNo, DevNo, FuncNo, ge->mbar);

#if DEBUG_MMAP
            showBCM570xConfig(BusNo, DevNo, FuncNo);
#endif

#if (CPU == RC32364 || CPU == MIPS32)
            pciConfigOutWord(BusNo, DevNo, FuncNo,
                           PCI_CFG_CACHE_LINE_SIZE,
                           PCI_DEVICE_MAX_LATENCY);
            pciConfigOutWord (BusNo, DevNo, FuncNo,
                            PCI_CFG_COMMAND,
                            0x117);
#endif
	    ge->polled_mode = 0;
	    ge->pkt_out = 0;
	    ge->pkt_in = 0;	    

	    ge->pDevice = malloc(sizeof(UM_DEVICE_BLOCK));
	    memset(ge->pDevice, 0x0, sizeof(UM_DEVICE_BLOCK));
	    ge->pUmDevice = (PUM_DEVICE_BLOCK)ge->pDevice;

	    ge->pUmDevice->global_lock = semMCreate(SEM_Q_PRIORITY|
						    SEM_DELETE_SAFE|
						    SEM_INVERSION_SAFE);
	    ge->pUmDevice->undi_lock = semMCreate(SEM_Q_PRIORITY|
						  SEM_DELETE_SAFE|
						  SEM_INVERSION_SAFE);
	    /* Configure pci dev structure */
	    pci_device = (pci_dev*)malloc(sizeof(pci_dev));
	    ge->pUmDevice->pdev = pci_device;
	    ge->pUmDevice->pdev->bus = BusNo;
	    ge->pUmDevice->pdev->device = DevNo;
	    ge->pUmDevice->pdev->func = FuncNo;
	    ge->pUmDevice->index = ge->ge_unit;
	    ge->mtu = ETHERMTU;
	    ge->pUmDevice->end = (struct bcm570x_end_t *) ge;

	    devFound = TRUE;
	    break;
	}
    }
    D5703("5703 CHK-7\n");

    if(!devFound){
#if (CPU == RC32364 || CPU == MIPS32)
        sysPciBusErrEnable();
#endif
	printf("bcm570xEndLoad: no BCM570x devices found!\n"); 
	return NULL;
    }
    D5703("5703 CHK-8\n");
    
    if (ERROR == END_OBJ_INIT(eo, (DEV_OBJ*)ge, "bc", net_unit, 
			      &bcm570xEndNetFuncs, BCM570X_END_STRING)) {
	printf("bcm570xEndLoad: END_OBJ_INIT failed for %s\n", dev_name);
    }

    /* Call BSP to get ethernet address. if call fails, use default */
    if (_func_bcm570xEnetAddrGet == NULL ||
        _func_bcm570xEnetAddrGet ("bc", net_unit, mac) != OK)
        memcpy (mac, enetAdrs, 6);

    memcpy(enetAdrs, mac, 6);

    D5703("5703 CHK-9\n");
#ifdef INCLUDE_MIBII_RFC2233
    /* Initialize MIB-II entries (for RFC 2233 ifXTable) */
    eo->pMib2Tbl = m2IfAlloc(M2_ifType_ethernet_csmacd,
                                          &mac[0], sizeof(mac),
                                          ETHERMTU, BCM570X_END_SPEED,
                                          "bc", net_unit);
    if (eo->pMib2Tbl == NULL){
        printf("bcm570xEndLoad: END_MIB_INIT failed for %s\n", dev_name);
        return (NULL);
    }

    D5703("5703 CHK-10\n");
    /*
     * Because this driver is not a NPT driver we must copy the MAC
     * address into the old mib2Tbl.ifPhysAddress location.
     */
    eo->mib2Tbl.ifPhysAddress.addrLength = sizeof(mac);
    bcopy( (char *)(&mac[0]), (char *)(eo->mib2Tbl.ifPhysAddress.phyAddress),
                      (int)(eo->mib2Tbl.ifPhysAddress.addrLength));

    m2IfPktCountRtnInstall(eo->pMib2Tbl, m2If8023PacketCount);

    if ( (eo->pMib2Tbl != NULL) &&
         (eo->pMib2Tbl->m2VarUpdateRtn != NULL))
    {
        M2_OBJECTID ifSpecific;

        /* Force the ifSpecific Object to empty, needs to be set correctly. */
        bzero( (char *)&ifSpecific, sizeof(ifSpecific) );
        ifSpecific.idLength = 2;
        eo->pMib2Tbl->m2VarUpdateRtn (
                                        eo->pMib2Tbl,
                                        M2_varId_ifSpecific,
                                        (caddr_t)&ifSpecific);
        eo->pMib2Tbl->m2VarUpdateRtn (
                                        eo->pMib2Tbl,
                                        M2_varId_ifConnectorPresent,
                                        (caddr_t)M2_CONNECTOR_PRESENT);
        eo->pMib2Tbl->m2VarUpdateRtn (
                                        eo->pMib2Tbl,
                                        M2_varId_ifLinkUpDownTrapEnable,
                                       (caddr_t)M2_LINK_UP_DOWN_TRAP_DISABLED);

         eo->pMib2Tbl->m2VarUpdateRtn (
                                        eo->pMib2Tbl,
                                        M2_varId_ifPromiscuousMode,
                                        (caddr_t)M2_PROMISCUOUS_MODE_OFF);
    }

#else

    if (ERROR == END_MIB_INIT(eo, M2_ifType_ethernet_csmacd, (UCHAR *)mac,
                              sizeof(mac), ETHERMTU, BCM570X_END_SPEED)) {
        printf("bcm570xEndLoad: END_MIB_INIT failed for %s\n", dev_name);
    }
#endif /* INCLUDE_MIBII_RFC2233 */

    D5703("5703 CHK-11\n");
    sysBcm570xCacheFuncsGet (pCacheFuncs);

    /* Setup packet buffer pools */
    bcm570xMemPoolAlloc(ge,
			BCM570X_END_PK_SZ,
			BCM570X_END_CLBLKS,
			BCM570X_END_CLBUFS, 
			BCM570X_END_MBLKS);
    ge->pDevice->TaskToOffload = LM_TASK_OFFLOAD_NONE;     

    /* Setup defaults for chip */
    if (ge->pDevice->ChipRevId == T3_CHIP_ID_5700_B0) {
	ge->pDevice->TaskToOffload = LM_TASK_OFFLOAD_NONE; 
    }
    else {
	if (rx_checksum[i]) {
	    ge->pDevice->TaskToOffload |=
		LM_TASK_OFFLOAD_RX_TCP_CHECKSUM |
		LM_TASK_OFFLOAD_RX_UDP_CHECKSUM;
	}
	if (tx_checksum[i]) {
	    ge->pDevice->TaskToOffload |=
		LM_TASK_OFFLOAD_TX_TCP_CHECKSUM |
		LM_TASK_OFFLOAD_TX_UDP_CHECKSUM;
	    ge->pDevice->NoTxPseudoHdrChksum = TRUE;
	}
    }

    /* Set Device PCI Memory base address */
    ge->pDevice->pMappedMemBase = (PLM_UINT8) ge->mbar;

    /* I believe UndiFix is just for Intel PCI, so turn it off */
    ge->pDevice->UndiFix = 0;

    /* Pull down adapter info */
    if ((rv = LM_GetAdapterInfo(ge->pDevice)) != LM_STATUS_SUCCESS) {
	printf("bcm570xEnd: LM_GetAdapterInfo failed: rv=%d!\n", rv );
	return NULL;
    }
    D5703("5703 CHK-12\n");
    /* Lock not needed */
    ge->pUmDevice->do_global_lock = 0;

    if (T3_ASIC_REV(ge->pUmDevice->lm_dev.ChipRevId) == T3_ASIC_REV_5700) {
	/* The 5700 chip works best without interleaved register */
	/* accesses on certain machines. */
	ge->pUmDevice->do_global_lock = 1;
    }

    /* Setup timer delays */
    if (T3_ASIC_REV(ge->pDevice->ChipRevId) == T3_ASIC_REV_5701) {
	ge->pDevice->UseTaggedStatus = TRUE;
	ge->pUmDevice->timer_interval = SECOND_USEC;
    }
    else {
	ge->pUmDevice->timer_interval = SECOND_USEC / 50;
    }
    
    /* Grab name .... */
    ge->pUmDevice->name =
	(char*)malloc(strlen(board_info[bcm570xDevices[i].board_id].name)+1);
    strcpy(ge->pUmDevice->name,board_info[bcm570xDevices[i].board_id].name);

    memcpy(ge->pDevice->NodeAddress, enetAdrs, 6);
    memcpy(ge->ge_mac, enetAdrs, 6);
    LM_SetMacAddress(ge->pDevice, enetAdrs);

    printf("bc%d: %s @0x%lx, IRQ %d, align %d,",
	   net_unit, ge->pUmDevice->name, (ULONG)ge->mbar,
	   ge->irq, ge->align);
    printf(	"node addr ");
    for (j = 0; j < 6; j++) {
	printf("%2.2x", ge->pDevice->NodeAddress[j]);
    }
    printf("\n");

    printf("bc%d: ", ge->ge_unit);    
    printf("%s with ",
	   chip_rev[bcm570xDevices[i].board_id].name);

    if ((ge->pDevice->PhyId & PHY_ID_MASK) == PHY_BCM5400_PHY_ID)
	printf("Broadcom BCM5400 Copper ");
    else if ((ge->pDevice->PhyId & PHY_ID_MASK) == PHY_BCM5401_PHY_ID)
	printf("Broadcom BCM5401 Copper ");
    else if ((ge->pDevice->PhyId & PHY_ID_MASK) == PHY_BCM5411_PHY_ID)
	printf("Broadcom BCM5411 Copper ");
    else if ((ge->pDevice->PhyId & PHY_ID_MASK) == PHY_BCM5701_PHY_ID)
	printf("Broadcom BCM5701 Integrated Copper ");
    else if ((ge->pDevice->PhyId & PHY_ID_MASK) == PHY_BCM5703_PHY_ID)
        printf("Broadcom BCM5703 Integrated Copper ");
    else if ((ge->pDevice->PhyId & PHY_ID_MASK) == PHY_BCM8002_PHY_ID)
	printf("Broadcom BCM8002 SerDes ");
    else if (ge->pDevice->EnableTbi)
	printf("Agilent HDMP-1636 SerDes ");
    else
	printf("Unknown ");
    printf("transceiver found\n");
    
    printf("bc%d: MTU: %d,", net_unit, ge->mtu);

    if ((ge->pDevice->ChipRevId != T3_CHIP_ID_5700_B0) &&
	rx_checksum[i])
	printf("Rx Checksum ON\n");
    else
	printf("Rx Checksum OFF\n");
    
    ge->pUmDevice->rx_last_cnt = ge->pUmDevice->tx_last_cnt = 0;

    /* delay for 4 seconds */
    ge->pUmDevice->delayed_link_ind =
	(4 * SECOND_USEC) / ge->pUmDevice->timer_interval;
    ge->pUmDevice->adaptive_expiry =
	SECOND_USEC / ge->pUmDevice->timer_interval;
    
    /* Sometimes we get spurious ints. after reset when link is down. */
    /* This field tells the isr to service the int. even if there is */
    /* no status block update. */
    ge->pUmDevice->adapter_just_inited =
	(3 * SECOND_USEC) / ge->pUmDevice->timer_interval;
    
    /* Init queues  .. */
    QQ_InitQueue(&ge->pUmDevice->rx_out_of_buf_q.Container,
		 MAX_RX_PACKET_DESC_COUNT);

    rv = 0;

    /* Set Ready flags in END object ... */
    END_OBJ_READY (eo, IFF_NOTRAILERS | IFF_MULTICAST | IFF_BROADCAST);

    /* Queue device info structure */
    ge->ge_next = geu->geu_devices;
    geu->geu_devices = ge;
    D5703("5703 CHK-13\n");
    return eo;
}


/*
 * Start the driver, hook up interrupt and initialize chip.
 */
STATIC STATUS
bcm570xEndStart(END_OBJ *pDrvCtrl)
{
    struct end_object* eo = (struct end_object*)pDrvCtrl;
    bcm570x_end_t* ge = (bcm570x_end_t*)eo->devObject.pDevice;

    D5703("5703 CHK-14\n");
    /* Get MAC address */
    bcopy((char *)ge->ge_mac, (char *)eo->mib2Tbl.ifPhysAddress.phyAddress, 
	  eo->mib2Tbl.ifPhysAddress.addrLength);

#if CPU == PPC603
    /* Hookup interrupt routine and enable it */
#ifndef BCM5703_PCI_INTCONNECT
    if (intConnect ((VOIDFUNCPTR *)INUM_TO_IVEC (ge->irq),
#else
    if (pciIntConnect ((VOIDFUNCPTR *)INUM_TO_IVEC (ge->irq),
#endif
             (VOIDFUNCPTR)bcm570xIntrIntContext, (int)ge) != OK)
        {
        printf("bcm570xEndStart: intConnect failed:inum=0x%x, irq=%d\n",
               (unsigned int)INUM_TO_IVEC(ge->irq), ge->irq);
        return ERROR;
        }
     DRV_LOG(DRV_DEBUG_INT, "bc%d: intConnect(ivec=%d,irq=%d)\n",
            ge->ge_unit, INUM_TO_IVEC(ge->irq),ge->irq, 0,0,0);

#elif (CPU == RC32364 || CPU == MIPS32)
    /* IDT R3K */
    /* Hookup interrupt routine and enable it */
    if (intConnect ((VOIDFUNCPTR *)INUM_TO_IVEC(IV_IORQ0_VEC + ge->irq),
                         (VOIDFUNCPTR)bcm570xIntrIntContext, (int)ge) != OK)
        {
        printf("bcm570xEndStart: intConnect failed:inum=%d, irq=%d\n",
               (int)IV_IORQ0_VEC, ge->irq);
        return ERROR;
        }

    DRV_LOG(DRV_DEBUG_INT, "bc%d: intConnect(in2iv=0x%x ivec=0x%x,irq=%d)\n",
            ge->ge_unit,
            (unsigned)((VOIDFUNCPTR*)INUM_TO_IVEC(IV_IORQ0_VEC + ge->irq)),
            IV_IORQ0_VEC + ge->irq, ge->irq, 0,0);
#endif

    D5703("5703 CHK-15\n");
    /* Initialize 570x */
    if (LM_InitializeAdapter(ge->pDevice) != LM_STATUS_SUCCESS) {
	printf("ERROR: Adapter initialization failed.\n");
	return ERROR;
    }

    D5703("5703 CHK-16\n");
    /* Enable chip ISR */
    LM_EnableInterrupt(ge->pDevice);

    /* Clear MC table */
    LM_MulticastClear(ge->pDevice);

    /* Enable Multicast */
    LM_SetReceiveMask(ge->pDevice,
		      ge->pDevice->ReceiveMask | LM_ACCEPT_ALL_MULTICAST);

    ge->pUmDevice->tx_full = 0;
    
    /* Set driver flags */
    END_FLAGS_SET(eo, IFF_UP | IFF_RUNNING | IFF_BROADCAST);    
    ge->pUmDevice->opened = 1;

    intEnable(ge->irq);
    D5703("5703 CHK-17\n");
    return OK;
}


/*
 * Halt driver.
 */
STATIC STATUS
bcm570xEndStop(
	       END_OBJ *pDrvCtrl
	       )
{
    struct end_object* eo = (struct end_object*) pDrvCtrl;
    bcm570x_end_t* ge = (bcm570x_end_t*) eo->devObject.pDevice;
    PUM_DEVICE_BLOCK pUmDevice;
    int i;

    if (ge && (ge->pDevice != NULL) && (ge->pUmDevice != NULL) &&
        (ge->pUmDevice->opened != 0)) { 

        pUmDevice = (PUM_DEVICE_BLOCK) ge->pDevice;

        /* Disable interrupt signal */
        intDisable(ge->irq);
	
        /* stop device */
        LM_Halt(ge->pDevice);
        LM_SetPowerState(ge->pDevice, LM_POWER_STATE_D3);

        /* Free the memory allocated by the device in tigon3 */
        for (i = 0; i < pUmDevice->mem_list_num; i++)  {
            if (pUmDevice->mem_list[i])  {
		/* sanity check */
                if (pUmDevice->dma_list[i]) {  /* cache-safe memory */
                    cacheDmaFree (pUmDevice->mem_list[i]);
		} else {
                    free (pUmDevice->mem_list[i]);  /* normal memory   */
		}	
	    }	
	}

        /* mark the interface down */
        END_FLAGS_CLR (eo, IFF_UP | IFF_RUNNING);
        ge->pUmDevice->opened = 0;
    }

    return OK;
}

/*
 * Unload END driver for SENS/VxWorks.
 */
STATIC STATUS
bcm570xEndUnload(END_OBJ * pDrvCtrl)
{
    struct end_object 	*eo = (struct end_object *)pDrvCtrl;
    bcm570x_end_t 	*ge = (bcm570x_end_t *)eo->devObject.pDevice;
    bcm570x_end_t	*ge_cur, *ge_prev;
    bcm570x_buff_t	*geu= &tigonBuffPool[ge->ge_unit];

#ifdef INCLUDE_MIBII_RFC2233
    /* Free MIB-II entries */
    m2IfFree(eo->pMib2Tbl);
    eo->pMib2Tbl = NULL;
#endif /* INCLUDE_MIBII_RFC2233 */

    END_OBJECT_UNLOAD(eo);

    /* Remove from list of active devices */
    ge_prev = NULL;
    for (ge_cur = geu->geu_devices; ge_cur != ge; ge_cur = ge_cur->ge_next) {
	if (ge_cur == NULL) {
	    return(ERROR);
	}
	ge_prev = ge_cur;
    }

    if (ge_prev) {			/* Not head */
	ge_prev->ge_next = ge->ge_next;
    } else {				/* Head */
	geu->geu_devices = ge->ge_next;
    }

    return OK;
}


/*
 * Pull down stats from the 570x chip and populate the
 * MIB2 interface table.
 */
STATIC STATUS
bcm570xM2Update(END_OBJ* eo)
{
    bcm570x_end_t 	*ge = (bcm570x_end_t *)eo->devObject.pDevice;    
    M2_INTERFACETBL* m2 = &eo->mib2Tbl;
    PT3_STATS_BLOCK ps = ge->pDevice->pStatsBlkVirt;
    static int ranOnce = 0;
    
    if( !m2 )
	return ERROR;
    
    /* Populate MIB2 object descriptor */


    /* Non-changing elements, no need to update every time .. */
    if ( !ranOnce++ ) { 
	strcpy(m2->ifDescr, ge->pUmDevice->name);
	m2->ifType = M2_ifType_ethernet_csmacd;
	m2->ifMtu = ETHERMTU;
    }
    
    /* RX Stats */
    m2->ifInOctets = ps->ifHCInOctets.Low;
    m2->ifInUcastPkts = ps->ifHCInUcastPkts.Low;
    m2->ifInNUcastPkts = ps->ifHCInMulticastPkts.Low +
	                 ps->ifHCInBroadcastPkts.Low;
    m2->ifInDiscards = ps->ifInDiscards.Low;
    m2->ifInErrors  = ps->ifInErrors.Low;

    /* TX Stats */
    m2->ifOutOctets = ps->ifHCOutOctets.Low;
    m2->ifOutUcastPkts = ps->ifHCOutUcastPkts.Low;
    m2->ifOutNUcastPkts = ps->ifHCOutMulticastPkts.Low +
	                  ps->ifHCOutBroadcastPkts.Low;
    m2->ifOutDiscards = ps->ifOutDiscards.Low;
    m2->ifOutErrors = ps->ifOutErrors.Low;

    /* See tigon3.h for more stats */
    return OK;
}


/*
 * END IO-Control interface.
 */
STATIC STATUS
bcm570xEndIoctl(END_OBJ *pDrvCtrl, int cmd, caddr_t data)
{
    STATUS error = OK;
    END_OBJ * eo = (END_OBJ*)pDrvCtrl;
    bcm570x_end_t* ge = (bcm570x_end_t*)eo->devObject.pDevice;
    int unit = ge->ge_unit;
    
    DRV_LOG(DRV_DEBUG_IOCTL, "bcm570xEndIoctl: unit=%d\n", unit, 0,0, 0,0,0);

    switch((unsigned int)cmd) {

#if VX_VERSION == 62
        case EIOCGRCVJOBQ:
	    error = EINVAL;
            break;
#endif /* VX_VERSION == 62 */

    case EIOCSFLAGS:
	DRV_LOG(DRV_DEBUG_IOCTL,"bcm570xEndIoctl: %s%d: EIOCSFLAGS(0x%x)\n", 
		eo->devObject.name, eo->devObject.unit,
		(unsigned int)data,0,0,0);
	if ((long)data < 0) {	/* Clearing flags? */
	    long int value = (long)data;
	    value = -(--value);
	    END_FLAGS_CLR(eo, (long)value);
	} else {
	    END_FLAGS_SET(eo, (long)data);
	}
	break;

    case EIOCGFLAGS:
	DRV_LOG(DRV_DEBUG_IOCTL,"bcm570xEndIoctl: %s%d: EIOCGFLAGS\n", 
		eo->devObject.name, eo->devObject.unit,0, 0,0,0);
	if (NULL == data) {
	    error = EINVAL;
	} else {
	    *(int *)data = eo->flags;
	}
	break;

    case EIOCSADDR:		/* Set interface address */
	DRV_LOG(DRV_DEBUG_IOCTL,"bcm570xEndIoctl: %s%d: EIOCSADDR\n", 
		eo->devObject.name, eo->devObject.unit,0, 0,0,0);
	if (NULL == data) {
	    error = EINVAL;
	} else {
	    if (!memcmp(ge->ge_mac, mac_zero,6)) {
		printf( "bcm570xEndIoctl: Deleting old mac address\n");
		/* Remove old MAC from Enet chip */
	    }
	    memcpy(ge->ge_mac, data, 6);
	    printf( "bcm570xEndIoctl: set new mac address="
		    "<%2x:%2x:%2x:%2x:%2x:%2x\n",
		    ge->ge_mac[0],
		    ge->ge_mac[1],	
		    ge->ge_mac[2],
		    ge->ge_mac[3],
		    ge->ge_mac[4],
		    ge->ge_mac[5]);
	    /* Write back to H/W */
	    if(  LM_STATUS_SUCCESS !=
		 LM_SetMacAddress(ge->pDevice, ge->ge_mac)){
		error = EIO;
		break;
	    }
	    
#ifdef INCLUDE_MIBII_RFC2233
                memcpy ((char *)END_HADDR(eo), (char *)data,
                           END_HADDR_LEN(eo));
#else
            END_MIB_INIT(eo, M2_ifType_ethernet_csmacd, (void*)data, 6,
                         ETHERMTU, BCM570X_END_SPEED);
#endif /* INCLUDE_MIBII_RFC2233 */
	}
	break;

    case EIOCGADDR:		/* Get Interface address */
	DRV_LOG(DRV_DEBUG_IOCTL,
		"bcm570xEndIoctl: %s%d: EIOCGADDR\n", 
		eo->devObject.name, eo->devObject.unit,0, 0,0,0);
	if (NULL == data) {
	    error = EINVAL;
	} else {
#ifdef INCLUDE_MIBII_RFC2233
                memcpy ((char *)data,(char *)END_HADDR(eo),
                           END_HADDR_LEN(eo));
#else
            bcopy((char *)eo->mib2Tbl.ifPhysAddress.phyAddress,
                  (char *)data, eo->mib2Tbl.ifPhysAddress.addrLength);
#endif /* INCLUDE_MIBII_RFC2233 */

	}
	break;

    case EIOCGFBUF:		/* Get min 1st buf for chain */
	DRV_LOG(DRV_DEBUG_IOCTL,"bcm570xEndIoctl: %s%d: EIOCGFBUF\n", 
		eo->devObject.name, eo->devObject.unit,0, 0,0,0);
	if (data == NULL) {
	    error = EINVAL;
	} else {
	    *(int *)data = 32;
	}
	break;

    case EIOCGMWIDTH:		/* Get device memory witdh */
	DRV_LOG(DRV_DEBUG_IOCTL, "bcm570xEndIoctl: %s%d: EIOCGMWIDTH\n", 
		eo->devObject.name, eo->devObject.unit,0, 0,0,0);
	if (NULL == data) {
	    error = EINVAL;
	} else {
	    *(int *)data = 32;
	}
	break;

    case EIOCMULTIADD:
    case EIOCMULTIDEL:
    case EIOCMULTIGET:
	DRV_LOG(DRV_DEBUG_IOCTL, "bcm570xEndIoctl: %s%d: MULTI/POLL\n", 
		eo->devObject.name, eo->devObject.unit,0, 0,0,0);
	error = ENOSYS;
	break;

    case EIOCGMIB2:		/* Get MIB2 Table */
	DRV_LOG(DRV_DEBUG_IOCTL,"bcm570xEndIoctl: %s%d: EIOCGMIB2\n", 
		eo->devObject.name, eo->devObject.unit,0, 0,0,0);
	if (data == NULL) {
	    error = EINVAL;
	} else {
	    /* Update MIB2 stats: lazy evaluate */
	    bcm570xM2Update(eo);
	    bcopy ((char *)&eo->mib2Tbl, (char *)data, sizeof(eo->mib2Tbl));
	}
	break;

    case EIOCGNAME:		/* Get device Name */
	DRV_LOG(DRV_DEBUG_IOCTL,"bcm570xEndIoctl: %s%d: EIOCGNAME\n", 
		eo->devObject.name, eo->devObject.unit,0, 0,0,0);
	if (NULL == data) {
	    error = EINVAL;
	} else {
	    bcopy(eo->devObject.name, (char *)data, 
		  sizeof(eo->devObject.name));
	}
	break;
	
    case EIOCGHDRLEN:
	if (NULL == data) {
	    error = EINVAL;
	} else {
	    if(0){
		/* DA+SA+VLANTAG+LEN = 6+6+4+2 = 18 */		
		*(int *)data = BCM570X_ENET_TAGGED_HDR_LEN;
	    }
	    else{
		/* DA+SA+LEN = 6+6+2 = 14 */
		*(int *)data = BCM570X_ENET_UNTAGGED_HDR_LEN;
	    }
	}
	break;
#ifdef INCLUDE_MIBII_RFC2233
    case EIOCGMIB2233:
        if ((data == NULL) || (eo->pMib2Tbl == NULL))
            error = EINVAL;
        else
            *((M2_ID **)data) = eo->pMib2Tbl;
        break;
#endif /* INCLUDE_MIBII_RFC2233 */

	/* Support polled-mode I/O */
    case EIOCPOLLSTART:              
	bcm570xEndPollStart (pDrvCtrl);
	break;

    case EIOCPOLLSTOP:               
	bcm570xEndPollStop (pDrvCtrl);
	break;

    default:
	DRV_LOG(DRV_DEBUG_IOCTL,"bcm570xEndIoctl: %s%d: Unknown IOCTL\n", 
		eo->devObject.name, eo->devObject.unit,0, 0,0,0);
	error = ENOSYS;
	break;
    }
    DRV_LOG(DRV_DEBUG_IOCTL, "bcm570xEndIoctl: %s%d: cmd=0x%x Return(%d)\n", 
	    eo->devObject.name, eo->devObject.unit, cmd, error,0,0);

    return error;
}

/*
 * Send the provided frame out on the wire.
 */
STATIC STATUS
bcm570xEndSend(END_OBJ *pDrvCtrl, M_BLK_ID mb)
{
    struct end_object* eo = (struct end_object*)pDrvCtrl;
    bcm570x_end_t* ge = (bcm570x_end_t*)eo->devObject.pDevice;
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK)ge->pUmDevice;
    PLM_DEVICE_BLOCK pDevice = (PLM_DEVICE_BLOCK)ge->pDevice;
    PLM_PACKET pPacket;
    PUM_PACKET pUmPacket;
    STATUS rval = OK;
    void* skb = NULL;
    unsigned int len = 0;
#ifdef USE_ZERO_COPY_TX
    M_BLK_ID pMblk = NULL;
    int frag_cnt = 0;
#endif
    if(ge->polled_mode == 0) {
        END_TX_SEM_TAKE(eo, WAIT_FOREVER);
    }

    /* Link down, return */
    if (pDevice->LinkStatus == LM_STATUS_LINK_DOWN) {
        printf("bc%d: link down - check cable or link partner.\n",
               pUmDevice->index);
        if(ge->polled_mode == 0) {
            END_TX_SEM_GIVE(eo);
        }
        return END_ERR_BLOCK;
    }

    pPacket = (PLM_PACKET)
	QQ_PopHead(&pDevice->TxPacketFreeQ.Container);


    /* Previously blocked, continue in blocked state only
     * if you still can't get a packet descriptor after
     * a garbage collection of Tx'd packets.
     */
    if(pUmDevice->tx_full && (pPacket == 0)){
	printf("bc%d: tx blocked, garbage collecting.\n",
	       pUmDevice->index);
	/*
	 * Reclaim buffers from TX queue, move to free queue.
	 */
	MM_IndicateTxPackets(pDevice);
	ge->bcm570x_tx_errs++;
    if(ge->polled_mode == 0) {
        END_TX_SEM_GIVE(eo);
    }
	return END_ERR_BLOCK;
    }

    if (pPacket == 0) {
	pUmDevice->tx_full = 1;
	printf("bcm570xEndSend: TX full!\n");
	ge->bcm570x_tx_errs++;
    if(ge->polled_mode == 0) {
	END_TX_SEM_GIVE(eo);
    }
	return END_ERR_BLOCK;
    }

    if (pDevice->SendBdLeft.counter == 0) {
	pUmDevice->tx_full = 1;
	printf("bcm570xEndSend: no more TX descriptors!\n");
	QQ_PushHead(&pDevice->TxPacketFreeQ.Container, pPacket);
	ge->bcm570x_tx_errs++;
    if(ge->polled_mode == 0) {
	END_TX_SEM_GIVE(eo);
    }
	return END_ERR_BLOCK;
    }
    
    /* Start data at first MBuf pointer */
    skb = (void*)mb->m_data;
    len = mb->m_len;

    /* Get packet buffers and fragment list */
    pUmPacket = (PUM_PACKET) pPacket;
    pUmPacket->skbuff = skb;
    pUmPacket->mb = mb;
    pPacket->u.Tx.FragCount = 0;
 
#ifdef USE_ZERO_COPY_TX

    /* Multiple DMA descriptor transmit.
     * VxWorks may give us a scattered range of addresses that make
     * up the packet. If so, we setup multiple DMA descriptors for
     * the fragment addresses up to MAX_SKB_FRAGS.
     */
    /* Init */
    len = 0;
    pMblk = mb;
    frag_cnt = 0;
    
    /* Get the fragment count */
    while (pMblk != NULL) {
	if(pMblk->m_len > 0) {
	    frag_cnt++;
	    len += pMblk->m_len;
	}
	pMblk = pMblk->m_next;
    }
    pPacket->u.Tx.FragCount = frag_cnt;

    /* No more TX descriptors, push packet back into workq and return*/
    if (pDevice->SendBdLeft.counter < (frag_cnt + 1)) {
	pUmDevice->tx_full = 1;
	QQ_PushHead(&pDevice->TxPacketFreeQ.Container, pPacket);
    if(ge->polled_mode == 0) {
	END_TX_SEM_GIVE(eo);
    }
	printf("bcm570xEndSend: no more descriptors (%d left, need %d)\n",
	       atomic_read(&pDevice->SendBdLeft), frag_cnt+1);
	return END_ERR_BLOCK;
    }

    pUmDevice->tx_full = 0;

    /* pPacket->Flags |= SND_BD_FLAG_COAL_NOW; */
    /* Done SG DMA */
#else
    /* Single DMA Descriptor transmit.
     * Fragments may be provided, but one DMA descriptor max is
     * used to send the packet.
     */
    if (MM_CoalesceTxBuffer (pDevice, pPacket) != LM_STATUS_SUCCESS)
        {
        printf("MM_CoalesceTxBuffer Failed\n");
        if (pUmPacket->skbuff == NULL)
           {
           /* Packet was discarded */
           rval = ERROR;
           }
        else
           {
           rval = END_ERR_BLOCK;
           }
        QQ_PushHead (&pDevice->TxPacketFreeQ.Container, pPacket);
        if(ge->polled_mode == 0) {
        END_TX_SEM_GIVE (eo);
        }
        return rval;
        }

    pPacket->u.Tx.FragCount = 1;

    pPacket->Flags |= SND_BD_FLAG_END|SND_BD_FLAG_COAL_NOW;

#endif /* MAX_SKB_FRAGS */
    
    /* VxWorks has already provided a frame ready for transmission */
    pPacket->Flags &= ~SND_BD_FLAG_TCP_UDP_CKSUM;

    /*
     *
     * NOTE: If you could turn off IP, UDP, and TCP checksums in
     * the VxWorks network stack, you could let 570x HW do the
     * checksums for you in H/W! AFAIK, Linux is the only OS that
     * supports the below DMA flags.
     *
     pPacket->Flags |=
                   SND_BD_FLAG_IP_CKSUM|SND_BD_FLAG_TCP_UDP_CKSUM;
    */

#if PKT_DEBUG
    if(bcm570x_pkt_debug){
	printf("TX: cksum:");	
	if(pPacket->Flags & SND_BD_FLAG_IP_CKSUM)
	    printf("+ip ");
	if(pPacket->Flags & SND_BD_FLAG_TCP_UDP_CKSUM)
	    printf("+tcp/udp ");
	if(pPacket->Flags & SND_BD_FLAG_END)
	    printf("eop ");
	if(pPacket->Flags & SND_BD_FLAG_COAL_NOW)
	    printf("cnow ");
	if(pPacket->Flags & SND_BD_FLAG_CPU_PRE_DMA)
	    printf("before dma ");
	if(pPacket->Flags & SND_BD_FLAG_CPU_POST_DMA)
	    printf("after dma ");
	if(pPacket->Flags & SND_BD_FLAG_INSERT_SRC_ADDR)
	    printf("isa ");
	if(pPacket->Flags & SND_BD_FLAG_CHOOSE_SRC_ADDR)
	    printf("csa ");
	if(pPacket->Flags & SND_BD_FLAG_DONT_GEN_CRC)
	    printf("nocrc ");
	if(pPacket->Flags & SND_BD_FLAG_IP_FRAG)
	    printf("ipf ");
	if(pPacket->Flags & SND_BD_FLAG_IP_FRAG_END)
	    printf("ipfe ");
	if(pPacket->Flags & SND_BD_FLAG_VLAN_TAG)
	    printf("+vlan ");
	printf("\n");
	bcm570xPktDump("TX", mb->m_data, len);
    }
#endif

    CACHE_PIPE_FLUSH();
    
    if (LM_SendPacket(pDevice, pPacket) == LM_STATUS_FAILURE)
        {
        ge->bcm570x_n_tx--;
        /* 
         *  A lower level send failure will push the packet descriptor back
         *  in the free queue, so just deal with the VxWorks clusters.
         */
        if (pUmPacket->skbuff == NULL)
           {
           /* Packet was discarded */
           rval = ERROR;
           }
        else
           {
           /* A resource problem, let MUX decide whether to retransmit */
           rval = END_ERR_BLOCK;
           }
        }

    if (QQ_GetEntryCnt(&pDevice->TxPacketFreeQ.Container) == 0) {
	printf("TX: emptyQ!\n");
	pUmDevice->tx_full = 1;
    }

    ge->bcm570x_n_tx++;

    if(ge->polled_mode == 0) {
    END_TX_SEM_GIVE(eo); 
    }
    
    return (rval);
}

/*
 * This routine is called by a user to try and send a packet on the
 * device. It sends a packet directly on the network from the caller without
 * going through the normal processes of queuing a pacet on an output queue
 * and the waiting for the device to decide to transmit it.
 *
 * If it detects a transmission error, the restart command is issued.
 *
 * These routine should not call any kernel functions.
 *
 * RETURNS: OK on success, EAGAIN on failure
 */
STATIC STATUS
bcm570xEndPollSend(END_OBJ *pDrvCtrl, M_BLK_ID mb)
{
    struct end_object* eo = (struct end_object*)pDrvCtrl;
    bcm570x_end_t* ge = (bcm570x_end_t*)eo->devObject.pDevice;
    int status;
    int count;
#if POLL_DEBUG
    sysSerialPrintString("PollSend\n");
#endif	
    status = bcm570xEndSend(pDrvCtrl, mb);

    for(count = 256; count > 0; count--) {
#if POLL_DEBUG
    sysSerialPrintString("PollSend: Polling for TX complete\n");
#endif	
        bcm570xIntr(ge);
        if (ge->pkt_out) { break; }
    }

    if ( ge->pkt_out){
	ge->pkt_out = 0;
	return status == OK ? OK : ERROR;
    }
    else {
	return EAGAIN;
    }
}

/*
 * This routine is called by a user to try and get a packet from the
 * device. It returns EAGAIN if no packet is available. The caller must
 * supply a M_BLK_ID with enough space to contain the receiving packet. If
 * enough buffer is not available then EAGAIN is returned.
 *
 * These routine should not call any kernel functions.
 *
 * RETURNS: OK on success, EAGAIN on failure.
 */
STATIC STATUS
bcm570xEndPollReceive(END_OBJ *pDrvCtrl, M_BLK_ID pMblk)
{
    struct end_object* eo = (struct end_object*)pDrvCtrl;
    bcm570x_end_t* ge = (bcm570x_end_t*)eo->devObject.pDevice;
    PLM_DEVICE_BLOCK pDevice = (PLM_DEVICE_BLOCK)ge->pDevice;
    PLM_PACKET          pPacket = NULL;
    PUM_PACKET          pUmPacket = NULL;
    void *skb;
    int size=0;

    /* Check if the M_BLK has an associated cluster */
    if ((pMblk->mBlkHdr.mFlags & M_EXT) != M_EXT)
        return EAGAIN;
    
    /* Pull down packet if it is there */
    bcm570xIntr(ge);

    /* Indicate RX packets called */
    if(!(ge->pkt_in) && (QQ_Empty(&pDevice->RxPacketReceivedQ.Container))) {
        return EAGAIN;
    }
#if POLL_DEBUG
    sysSerialPrintString("PollReceive (Packet waiting)\n");
#endif    
    pPacket = (PLM_PACKET)
	QQ_PopHead(&pDevice->RxPacketReceivedQ.Container);
    if(QQ_Empty(&pDevice->RxPacketReceivedQ.Container)) {
        ge->pkt_in = 0;
#if POLL_DEBUG
        sysSerialPrintString("Rx Last Pkt\n");
#endif    
    }
    else {
#if POLL_DEBUG
        sysSerialPrintString("Rx More Pkt\n");
#endif    
    }

    if (pPacket == 0){
        sysSerialPrintString("PollReceive QQ_PopHead failed!\n");
        return EAGAIN;
    }
    pUmPacket = (PUM_PACKET) pPacket;

    /* If the packet generated an error, reuse buffer */
    if ((pPacket->PacketStatus != LM_STATUS_SUCCESS) ||
	((size = pPacket->PacketSize) > pDevice->RxMtu)) {
        /* reuse skb */
        QQ_PushTail(&pDevice->RxPacketFreeQ.Container, pPacket);
        sysSerialPrintString("PollReceive Error in Packet DMA!\n");
        return EAGAIN;
    }
    
    /* Set size and address */
    skb = pUmPacket->skbuff;
    size = pPacket->PacketSize;
    memcpy ((char *)pMblk->mBlkHdr.mData , skb, size);
    pMblk->mBlkHdr.mFlags |= M_PKTHDR; /* set the packet header */
    pMblk->mBlkPktHdr.len = pMblk->mBlkHdr.mLen = size;
	pUmPacket->skbuff = skb;
	pUmPacket->mb = (M_BLK_ID)0;
	QQ_PushTail(&pDevice->RxPacketFreeQ.Container, pPacket);

    ge->bcm570x_n_rx++;
    return OK;
}

/*
 *
 * This routine starts polling mode by disabling ethernet interrupts and
 * setting the polling flag in the END_CTRL stucture.
 *
 * RETURNS: OK, always.
 */

STATIC STATUS
bcm570xEndPollStart(END_OBJ *pDrvCtrl)
{
    int il;
    struct end_object* eo = (struct end_object*)pDrvCtrl;
    bcm570x_end_t* ge = (bcm570x_end_t*)eo->devObject.pDevice;    
    DRV_LOG (DRV_DEBUG_POLL, "bcm570xEndPollStart\n", 0, 0, 0, 0, 0, 0);
#if POLL_DEBUG
    sysSerialPrintString("PollStart\n");
#endif
    
    /* Mode sanity check */
    if(ge->polled_mode == 1)
	return OK;

    /* mask normal and abnormal interrupts */
    LM_DisableInterrupt(ge->pDevice);
    intDisable(ge->irq);

    /* set polling flag */    
    il = intLock();
    ge->polled_mode = 1;
    intUnlock(il);

    return OK;
}

STATIC STATUS
bcm570xEndPollStop(END_OBJ *pDrvCtrl)
{
    int il;
    struct end_object* eo = (struct end_object*)pDrvCtrl;
    bcm570x_end_t* ge = (bcm570x_end_t*)eo->devObject.pDevice;
    DRV_LOG (DRV_DEBUG_POLL, "bcm570xEndPollStop\n", 0, 0, 0, 0, 0, 0);
#if POLL_DEBUG
    sysSerialPrintString("PollStop\n");
#endif
    
    /* Check for not already disabled */
    if(ge->polled_mode == 0)
	return OK;

    /* Clear polling flag */    
    il = intLock();
    ge->polled_mode = 0;    
    intUnlock(il);
    
    intEnable(ge->irq);
    /* Re-enable interrupts */
    LM_EnableInterrupt(ge->pDevice);
    return OK;
}

STATIC STATUS
bcm570xEndMcastAdd(END_OBJ *pDrvCtrl, char *a)
{
    struct end_object* eo = (struct end_object*)pDrvCtrl;
    bcm570x_end_t* ge = (bcm570x_end_t*)eo->devObject.pDevice;

    if ( LM_STATUS_SUCCESS != LM_MulticastAdd(ge->pDevice,(PLM_UINT8)a)){
	printf("bcm570xEndMcastAdd: could not add multicast address.\n");
	return ERROR;
    }
    return OK;
}

STATIC STATUS
bcm570xEndMcastDel(END_OBJ *pDrvCtrl, char *a)
{
    struct end_object* eo = (struct end_object*)pDrvCtrl;
    bcm570x_end_t* ge = (bcm570x_end_t*)eo->devObject.pDevice;

    if ( LM_STATUS_SUCCESS != LM_MulticastDel(ge->pDevice, (PLM_UINT8)a)){
	printf("bcm570xEndMcastAdd: could not delete multicast address.\n");
	return ERROR;
    }
    return OK;
}

STATIC STATUS
bcm570xEndMcastGet(END_OBJ *eo, MULTI_TABLE *mt)
{
    return etherMultiGet(&eo->multiList, mt) == OK ? OK : ERROR;
}


#ifdef BCM570X_DEBUG_TASK
/*
 * Load a 570x driver and bind TCP/IP protocols to the interface with the
 * specified parameters.
 */
END_OBJ*
bcm570xIfConfig(char* devname,
		int unit,
		char* ipaddr,
		unsigned int if_netmask,
		char* hostname)
{
    char	if_name[END_NAME_MAX];
    char	if_name_str[END_NAME_MAX];    
    char 	if_ip_str[64];
    char 	if_ip[64];
    char 	if_host[128];        
    int         if_unit = 0;
    END_OBJ	*eo;			/* END object  */
    M2_INTERFACETBL	m2;

    /* Build vxWorks device name */
    sprintf(if_name, "%s", devname);
    sprintf(if_name_str, "%s%d", if_name, unit);
    sprintf(if_ip,"%s", ipaddr);
    sprintf(if_ip_str,"%s", ipaddr);
    sprintf(if_host,"%s", hostname);

    /* Check to see if end device already loaded */

    if (NULL == (eo = endFindByName(if_name, if_unit))) {
	if (NULL == (eo = muxDevLoad(if_unit, bcm570xEndLoad, 
				     BCM570X_INIT_STR,
				     0 /*TRUE%%%*/, NULL))) {
	    /* printf("%s: muxDevLoad failed\n", if_name_str); */
	    return(NULL);
	}

	DRV_LOG(DRV_DEBUG_LOAD, "%s: muxDevLoad successful: 0x%x\n",
		if_name_str, (int)eo,0, 0,0,0);

	if (ERROR == muxDevStart(eo)) {
	    printf("%s: muxDevStart failed: Unit %d\n", if_name_str, if_unit);
	    (void)muxDevUnload(if_name, if_unit);
	    return(NULL);
	}
	DRV_LOG(DRV_DEBUG_LOAD, "%s: muxDevStart successful: 0x%x\n",
		if_name_str, (int)eo,0, 0,0,0);
    }
    /*
     * Configure device....
     */
    if (OK != muxIoctl(eo, EIOCGMIB2, (caddr_t)&m2)) {
	printf("%s: muxIoctl failed: Unit %d\n", if_name_str, if_unit);
	return(NULL);
    }
    DRV_LOG(DRV_DEBUG_LOAD, "%s: muxIOCTL successful: Unit %d\n",
	    if_name_str, if_unit,0, 0,0,0);
    /*
     * Setup interface in following order:
     *		[1] Attach TCP/IP to END device
     *		[2] Set Netmask (if non-0)
     *		[3] Set IP address
     *		[4] Set host name associated with interface (if given).
     */
    if (OK != ipAttach(if_unit, if_name)) { /* [1] */
	printf("%s: ipAttach failed: Unit %d (interface %s)\n", 
	       if_name_str, if_unit, if_name_str);
	return(NULL);
    }
    DRV_LOG(DRV_DEBUG_LOAD,
	    "%s: ipAttach successful: if_name %s if_unit %d\n", 
	   if_name_str, if_name, if_unit, 0,0,0);

    if (0 != if_netmask) {		/* [2] */
	if (ERROR == ifMaskSet(if_name_str, if_netmask)) {
	    printf("%s: ifMaskSet failed: %s 0x%x\n", 
		   if_name_str, if_name_str, if_netmask);
	    return(NULL);
	}
    }

    if (OK != ifAddrSet(if_name_str, (char *)if_ip_str)) {
	printf("%s: ifAddrSet failed: %s <-- %s\n",
	       if_name_str, if_name, if_ip_str);
	return(NULL);
    }

    DRV_LOG(DRV_DEBUG_LOAD,
	    "%s: ifAddrSet successful: if %s\n", if_name_str,
	    if_name_str,0, 0,0,0);

    if (if_host && *if_host) {		/* [4] */
	DRV_LOG(DRV_DEBUG_LOAD,
		"%s: Setting hostname: %s\n",
		if_name_str, if_host,0, 0,0,0);
	if (OK != hostAdd (if_host, if_ip_str)) {
	    printf("%s: Warning: Failed to set hostname %s for device %s\n", 
		   if_name_str, if_host, if_name_str);
	}
    }

    return((void *)eo);			/* This is our opaque value */
}


/*
 * Diags: test program
 */
void BCM570x()
{
    DEV_OBJ* devObject = NULL;
    bcm570x_end_t	*ge = NULL;
    struct end_object 	*eo = NULL;
    PLM_DEVICE_BLOCK pDevice = NULL;
    PUM_DEVICE_BLOCK pUmDevice = NULL;

    char buf[128];
    char tmp[128];    
    char dev[24];
    char ip[24];
    char hostname[100];
    int instance;
    unsigned int netmask;

    /* Interface configured with this program */
    if(tigonEnd !=NULL) { 
	devObject = (DEV_OBJ*)&tigonEnd->devObject;
	ge = (bcm570x_end_t *)devObject->pDevice;
	eo = &ge->ge_eo;
	pDevice = (PLM_DEVICE_BLOCK)ge->pDevice;
	pUmDevice = (PUM_DEVICE_BLOCK)ge->pUmDevice;    
    } else {
	printf("Device name (e.g.\"bc\"):");
	scanf("%s",dev);
	printf("Unit number (e.g. 0):");
	scanf("%d", &instance);
	/* VxWorks configured */
	if (NULL == (eo = endFindByName(dev, instance))){
	    printf("BCM570x: could not find driver[%s], instance %d\n",
		   dev, instance);
	    return;
	}
	tigonEnd = eo;
	devObject = (DEV_OBJ*)&tigonEnd->devObject;
	ge = (bcm570x_end_t *)devObject->pDevice;
	pDevice = (PLM_DEVICE_BLOCK)ge->pDevice;
	pUmDevice = (PUM_DEVICE_BLOCK)ge->pUmDevice;    
    }
        
    while(TRUE){
	printf("Broadcom BCM570x configuration Menu\n");
	printf("Options\n");
	printf("1) Show stats\n");
	printf("2) Configure Interface\n");
	printf("Type \"q\" to exit this menu.\n");
	printf("Enter selection:");
	scanf("%s",buf);

	if(buf[0] == 'q' || buf[0] == 'Q')
	    break;

	switch(buf[0]){

	case '1':
	    /* Show stats */
	    if(tigonEnd != NULL){
		M2_INTERFACETBL* m2 = &eo->mib2Tbl;		
		printf("Driver dma sends: %d\n", ge->bcm570x_n_tx);
		printf("Driver dma send errors: %d\n", ge->bcm570x_tx_errs);
		printf("Driver dma receives: %d\n", ge->bcm570x_n_rx);
		printf("Driver dma received errors: %d\n", ge->bcm570x_rx_errs);
		printf("Packet mem alloc requests: %d\n", ge->bcm570x_allocs);
		printf("Packet mem free requests: %d\n", ge->bcm570x_frees);
		/* Update MIB2 from Stats on 570x */
		bcm570xM2Update(eo);
		/* RX Stats from 570x chip via MIB2*/
		printf("570x ifInOctets:     %ld\n", m2->ifInOctets);
		printf("570x ifInUcastPkts:  %ld\n", m2->ifInUcastPkts);
		printf("570x ifInNUcastPkts: %ld\n", m2->ifInNUcastPkts);
		printf("570x ifInDiscards:   %ld\n", m2->ifInDiscards);
		printf("570x ifInErrors:     %ld\n", m2->ifInErrors);
		/* TX Stats from 570x chip via MIB2 */
		printf("570x ifOutOctets:    %ld\n", m2->ifOutOctets);
		printf("570x ifOutUcastPkts: %ld\n", m2->ifOutUcastPkts);
		printf("570x ifOutNUcastPkts:%ld\n", m2->ifOutNUcastPkts);
		printf("570x ifOutDiscards:  %ld\n", m2->ifOutDiscards);
		printf("570x ifOutErrors:    %ld\n", m2->ifOutErrors);
		icmpstatShow();
		ipstatShow(0);
		tcpstatShow();
	    } else {
		printf("Choose option 2) to configure first.\n");
	    }
	    break;

	case '2':
	    /* ifconfig */
	    printf("Device name (e.g.\"bc\"):");
	    scanf("%s",dev);
	    printf("Unit number (e.g. 0):");
	    scanf("%d", &instance);
	    printf("IP Address[a.b.c.d]:");
	    scanf("%s", ip);
	    printf("Netmask[0xaabbccdd]:");
	    scanf("%s", tmp);
	    printf("Hostname:");
	    scanf("%s",hostname);
	    netmask = strtoul(tmp, NULL, 16);	    
	    tigonEnd =
		bcm570xIfConfig(dev,instance,ip,netmask,hostname);
	    if(tigonEnd != NULL){
		muxShow(NULL, 0);
		ifShow(NULL);
	    }
	    break;

	default:
	    printf("Invalid action.\n");
	    break;
	}

    }



}
#endif


/*
 * 
 * Middle Module: Interface between the HW driver (tigon3 modules) and
 * the native (SENS) driver.  These routines implement the system
 * interface for tigon3 on VxWorks.
 */

/* Middle module dependency - size of a packet descriptor */
int MM_Packet_Desc_Size = sizeof(UM_PACKET);

LM_STATUS
MM_ReadConfig32(PLM_DEVICE_BLOCK pDevice,
		LM_UINT32 Offset,
		LM_UINT32 *pValue32)
{
    UM_DEVICE_BLOCK *pUmDevice;
    pUmDevice = (UM_DEVICE_BLOCK *) pDevice;
#if (CPU == RC32364 || CPU == MIPS32)
    sysPciBusErrDisable();
#endif
    pciConfigInLong(pUmDevice->pdev->bus,
		    pUmDevice->pdev->device,
		    pUmDevice->pdev->func,		    
		    Offset, (UINT32 *) pValue32);
#if (CPU == RC32364 || CPU == MIPS32)
    sysPciBusErrEnable();
#endif
    return LM_STATUS_SUCCESS;
}


LM_STATUS
MM_WriteConfig32(PLM_DEVICE_BLOCK pDevice,
		 LM_UINT32 Offset,
		 LM_UINT32 Value32)
{
    UM_DEVICE_BLOCK *pUmDevice;
    pUmDevice = (UM_DEVICE_BLOCK *) pDevice;
#if (CPU == RC32364 || CPU == MIPS32)
    sysPciBusErrDisable();
#endif
    pciConfigOutLong(pUmDevice->pdev->bus,
		     pUmDevice->pdev->device,
		     pUmDevice->pdev->func,		    
		     Offset, Value32);
#if (CPU == RC32364 || CPU == MIPS32)
    sysPciBusErrEnable();
#endif
    return LM_STATUS_SUCCESS;
}	


LM_STATUS
MM_ReadConfig16(PLM_DEVICE_BLOCK pDevice,
		LM_UINT32 Offset,
		LM_UINT16 *pValue16)
{
    UM_DEVICE_BLOCK *pUmDevice;
    pUmDevice = (UM_DEVICE_BLOCK *) pDevice;
#if (CPU == RC32364 || CPU == MIPS32)
    sysPciBusErrDisable();
#endif
    pciConfigInWord(pUmDevice->pdev->bus,
		    pUmDevice->pdev->device,
		    pUmDevice->pdev->func,		    
		    Offset, (UINT16*) pValue16);
#if (CPU == RC32364 || CPU == MIPS32)
    sysPciBusErrEnable();
#endif
    return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_WriteConfig16(PLM_DEVICE_BLOCK pDevice,
		 LM_UINT32 Offset,
		 LM_UINT16 Value16)
{
    UM_DEVICE_BLOCK *pUmDevice;
    pUmDevice = (UM_DEVICE_BLOCK *) pDevice;
#if (CPU == RC32364 || CPU == MIPS32)
    sysPciBusErrDisable();
#endif
    pciConfigOutWord(pUmDevice->pdev->bus,
		     pUmDevice->pdev->device,
		     pUmDevice->pdev->func,		    
		     Offset, Value16);
#if (CPU == RC32364 || CPU == MIPS32)
    sysPciBusErrEnable();
#endif
    return LM_STATUS_SUCCESS;
}	


LM_STATUS
MM_AllocateSharedMemory(PLM_DEVICE_BLOCK pDevice, LM_UINT32 BlockSize,
			PLM_VOID *pMemoryBlockVirt,
			PLM_PHYSICAL_ADDRESS pMemoryBlockPhy,
			LM_BOOL Cached)
{
    PLM_VOID pvirt;
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    dma_addr_t mapping;

    /* 
    * cacheDmaMalloc() returns a cache-safe virtual address..
    * On a machine with bus snooping, like the PPC 8245, it may look
    * like an address returned by a normal malloc().
    * On a Mips, it could be
    * an address that is altered by ORing in the "uncached" address
    * space identifier, unless a normally cached address has been rendered
    * uncached by the MMU.  The DMA engine in the BCM5701
    * requires a physical address. Use CACHE_DMA_VIRT_TO_PHYS() for that.
    */

    pvirt = cacheDmaMalloc(BlockSize);
#if (CPU == RC32364 || CPU == MIPS32)
    /* 
     * CACHE_DMA_VIRT_TO_PHYS on MIPS will give a cached address.
     * cacheDmaMalloc returns the uncached address, so don't modify.
     */
    mapping = (dma_addr_t)(pvirt);
#else
    mapping = (dma_addr_t)CACHE_DMA_VIRT_TO_PHYS(pvirt);
#endif
    if (!pvirt)
	return LM_STATUS_FAILURE;

    pUmDevice->mem_list[pUmDevice->mem_list_num] = pvirt;
    pUmDevice->dma_list[pUmDevice->mem_list_num] = mapping;
    pUmDevice->mem_size_list[pUmDevice->mem_list_num++] = BlockSize;
    memset(pvirt, 0, BlockSize);

    *pMemoryBlockVirt = (PLM_VOID) pvirt;
    MM_SetAddr (pMemoryBlockPhy, (dma_addr_t) mapping);

    return LM_STATUS_SUCCESS;
}



LM_STATUS
MM_AllocateMemory(PLM_DEVICE_BLOCK pDevice, LM_UINT32 BlockSize,
	PLM_VOID *pMemoryBlockVirt)
{
    PLM_VOID pvirt;
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    
    pvirt = malloc(BlockSize);

    if (!pvirt)
	return LM_STATUS_FAILURE;
    
    pUmDevice->mem_list[pUmDevice->mem_list_num] = pvirt;
    pUmDevice->dma_list[pUmDevice->mem_list_num] = 0;
    pUmDevice->mem_size_list[pUmDevice->mem_list_num++] = BlockSize;
    memset(pvirt, 0, BlockSize);
    *pMemoryBlockVirt = pvirt;

    return LM_STATUS_SUCCESS;
}	

LM_STATUS
MM_MapMemBase(PLM_DEVICE_BLOCK pDevice)
{
    DRV_LOG(DRV_DEBUG_LOAD, "BCM570x PCI Memory base address @0x%x\n",
	    (unsigned int)pDevice->pMappedMemBase,0,0, 0,0,0);
    /* See bcm570xEndLoad */
    return LM_STATUS_SUCCESS;	
}

LM_STATUS
MM_InitializeUmPackets(PLM_DEVICE_BLOCK pDevice)
{
    int i;
    void* skb;
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    PUM_PACKET pUmPacket = NULL;
    PLM_PACKET pPacket = NULL;

    for (i = 0; i < pDevice->RxPacketDescCnt; i++) {
	pPacket = QQ_PopHead(&pDevice->RxPacketFreeQ.Container);
	pUmPacket = (PUM_PACKET) pPacket;

	if (pPacket == 0) {
	    printf("MM_InitializeUmPackets: Bad RxPacketFreeQ\n");
	}

	pUmPacket->mb = (M_BLK_ID)0;
	skb = bcm570xPktAlloc(pUmDevice->index,
			      pPacket->u.Rx.RxBufferSize + 2);

	if (skb == 0) {
	    pUmPacket->skbuff = 0;
	    QQ_PushTail(&pUmDevice->rx_out_of_buf_q.Container, pPacket);
	    continue;
	}

	pUmPacket->skbuff = skb;
	QQ_PushTail(&pDevice->RxPacketFreeQ.Container, pPacket);
    }

#if 0  
    /* 
    * This is what the Linux driver does.
    * For now I'll keep it simple and just use a single threshold.
    * This may need tuning later.
    */
    if (T3_ASIC_REV(pUmDevice->lm_dev.ChipRevId) == T3_ASIC_REV_5700) {
        /* reallocate buffers in the ISR */
        pUmDevice->rx_buf_repl_thresh = 0;
        pUmDevice->rx_buf_repl_panic_thresh = 0;
        }
    else if (T3_ASIC_REV(pUmDevice->lm_dev.ChipRevId) == T3_ASIC_REV_5703) {
        pUmDevice->rx_buf_repl_thresh = pDevice->RxPacketDescCnt / 10;
        pUmDevice->rx_buf_repl_panic_thresh =
            pDevice->RxPacketDescCnt / 3;
        }
    else {
        pUmDevice->rx_buf_repl_thresh = pDevice->RxPacketDescCnt / 8;
        pUmDevice->rx_buf_repl_panic_thresh =
            pDevice->RxPacketDescCnt / 2;
        }
#else
    /* 
    * The threshold is ignored.  The "Replenish" stuff was
    * borrowed from the Linux driver and I don't think it makes any
    * sense for VxWorks, but I'm leaving the code in for now.
    */ 
    pUmDevice->rx_low_buf_thresh = pDevice->RxPacketDescCnt / 8;
#endif
    return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_GetConfig(PLM_DEVICE_BLOCK pDevice)
{
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    bcm570x_end_t* ge = (bcm570x_end_t*)pUmDevice->end;
    int index = ge->ge_unit;
    
    if (auto_speed[index] == 0)
	pDevice->DisableAutoNeg = TRUE;
    else
	pDevice->DisableAutoNeg = FALSE;
    
    if (line_speed[index] == 0) {
	pDevice->RequestedMediaType =
	    LM_REQUESTED_MEDIA_TYPE_AUTO;
	pDevice->DisableAutoNeg = FALSE;
    }
    else {
	if (line_speed[index] == 1000) {
	    if (pDevice->EnableTbi) {
		pDevice->RequestedMediaType =
		    LM_REQUESTED_MEDIA_TYPE_FIBER_1000MBPS_FULL_DUPLEX;
	    }
	    else if (full_duplex[index]) {
		pDevice->RequestedMediaType =
		    LM_REQUESTED_MEDIA_TYPE_UTP_1000MBPS_FULL_DUPLEX;
	    }
	    else {
		pDevice->RequestedMediaType =
		    LM_REQUESTED_MEDIA_TYPE_UTP_1000MBPS;
	    }
	    if (!pDevice->EnableTbi)
		pDevice->DisableAutoNeg = FALSE;
	}
	else if (line_speed[index] == 100) {
	    if (full_duplex[index]) {
		pDevice->RequestedMediaType =
		    LM_REQUESTED_MEDIA_TYPE_UTP_100MBPS_FULL_DUPLEX;
	    }
	    else {
		pDevice->RequestedMediaType =
		    LM_REQUESTED_MEDIA_TYPE_UTP_100MBPS;
	    }
	}
	else if (line_speed[index] == 10) {
	    if (full_duplex[index]) {
		pDevice->RequestedMediaType =
		    LM_REQUESTED_MEDIA_TYPE_UTP_10MBPS_FULL_DUPLEX;
	    }
	    else {
		pDevice->RequestedMediaType =
		    LM_REQUESTED_MEDIA_TYPE_UTP_10MBPS;
	    }
	}
	else {
	    pDevice->RequestedMediaType =
		LM_REQUESTED_MEDIA_TYPE_AUTO;
	    pDevice->DisableAutoNeg = FALSE;
	}
	
    }
    pDevice->FlowControlCap = 0;
    if (rx_flow_control[index] != 0) {
	pDevice->FlowControlCap |= LM_FLOW_CONTROL_RECEIVE_PAUSE;
    }
    if (tx_flow_control[index] != 0) {
	pDevice->FlowControlCap |= LM_FLOW_CONTROL_TRANSMIT_PAUSE;
    }
    if ((auto_flow_control[index] != 0) &&
	(pDevice->DisableAutoNeg == FALSE)) {
	
	pDevice->FlowControlCap |= LM_FLOW_CONTROL_AUTO_PAUSE;
	if ((tx_flow_control[index] == 0) &&
	    (rx_flow_control[index] == 0)) {
	    pDevice->FlowControlCap |=
		LM_FLOW_CONTROL_TRANSMIT_PAUSE |
		LM_FLOW_CONTROL_RECEIVE_PAUSE;
	}
    }
    
#if T3_JUMBO_RCV_RCB_ENTRY_COUNT
    if (((bcm570x_end_t*)pUmDevice->end)->mtu > 1500) {
	pDevice->RxMtu = ((bcm570x_end_t*)pUmDevice->end)->mtu;
	pDevice->RxJumboDescCnt = DEFAULT_JUMBO_RCV_DESC_COUNT;
    }
    else {
	pDevice->RxJumboDescCnt = 0;
    }
    pDevice->RxJumboDescCnt = rx_jumbo_desc_cnt[index];
#else
    pDevice->RxMtu = ((bcm570x_end_t*)pUmDevice->end)->mtu;
#endif
    
    if (T3_ASIC_REV(pDevice->ChipRevId) == T3_ASIC_REV_5701) {
	pDevice->UseTaggedStatus = TRUE;
	pUmDevice->timer_interval = SECOND_USEC;
    }	
    else {	
	pUmDevice->timer_interval = SECOND_USEC/50;
    }	
    
    pDevice->TxPacketDescCnt = tx_pkt_desc_cnt[index];
    pDevice->RxStdDescCnt = rx_std_desc_cnt[index];
    /* Note:  adaptive coalescence really isn't adaptive in this driver */
    pUmDevice->rx_adaptive_coalesce = rx_adaptive_coalesce[index];
    if (!pUmDevice->rx_adaptive_coalesce) {
	pDevice->RxCoalescingTicks = rx_coalesce_ticks[index];
	if (pDevice->RxCoalescingTicks > MAX_RX_COALESCING_TICKS)
	    pDevice->RxCoalescingTicks = MAX_RX_COALESCING_TICKS;
	pUmDevice->rx_curr_coalesce_ticks =pDevice->RxCoalescingTicks;
	
	pDevice->RxMaxCoalescedFrames = rx_max_coalesce_frames[index];
	if (pDevice->RxMaxCoalescedFrames>MAX_RX_MAX_COALESCED_FRAMES)
	    pDevice->RxMaxCoalescedFrames =
				MAX_RX_MAX_COALESCED_FRAMES;
	pUmDevice->rx_curr_coalesce_frames =
	    pDevice->RxMaxCoalescedFrames;
	pDevice->StatsCoalescingTicks = stats_coalesce_ticks[index];
	if (pDevice->StatsCoalescingTicks>MAX_STATS_COALESCING_TICKS)
	    pDevice->StatsCoalescingTicks=
		MAX_STATS_COALESCING_TICKS;
	}	
	else {	
	    pUmDevice->rx_curr_coalesce_frames =
		DEFAULT_RX_MAX_COALESCED_FRAMES;
	    pUmDevice->rx_curr_coalesce_ticks =
		DEFAULT_RX_COALESCING_TICKS;
	}
    pDevice->TxCoalescingTicks = tx_coalesce_ticks[index];
    if (pDevice->TxCoalescingTicks > MAX_TX_COALESCING_TICKS)
	pDevice->TxCoalescingTicks = MAX_TX_COALESCING_TICKS;
    pDevice->TxMaxCoalescedFrames = tx_max_coalesce_frames[index];
    if (pDevice->TxMaxCoalescedFrames > MAX_TX_MAX_COALESCED_FRAMES)
	pDevice->TxMaxCoalescedFrames = MAX_TX_MAX_COALESCED_FRAMES;

    if (enable_wol[index]) {
	pDevice->WakeUpModeCap = LM_WAKE_UP_MODE_MAGIC_PACKET;
	pDevice->WakeUpMode = LM_WAKE_UP_MODE_MAGIC_PACKET;
    }
    pDevice->NicSendBd = TRUE;

    /* Don't update status blocks during interrupt */
    pDevice->RxCoalescingTicksDuringInt = 0;
    pDevice->TxCoalescingTicksDuringInt = 0;

    return LM_STATUS_SUCCESS;

}


LM_STATUS
MM_StartTxDma(PLM_DEVICE_BLOCK pDevice, PLM_PACKET pPacket)
{
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    printf("Start TX DMA: dev=%d packet @0x%x\n",
	   (int)pUmDevice->index, (unsigned int)pPacket);

    return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_CompleteTxDma(PLM_DEVICE_BLOCK pDevice, PLM_PACKET pPacket)
{
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    printf("Complete TX DMA: dev=%d packet @0x%x\n",
	   (int)pUmDevice->index, (unsigned int)pPacket);
    return LM_STATUS_SUCCESS;
}


LM_STATUS
MM_IndicateStatus(PLM_DEVICE_BLOCK pDevice, LM_STATUS Status)
{
    char buf[128];
#if (CPU == RC32364 || CPU == MIPS32)
    char lcd[4];
#endif
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    LM_FLOW_CONTROL flow_control;
    bcm570x_end_t* ge = (bcm570x_end_t*)pUmDevice->end;
    END_OBJ* endObject = (END_OBJ*)&ge->ge_eo;
    DEV_OBJ* devObject = &endObject->devObject;

    pUmDevice->delayed_link_ind = 0;
#if (CPU == RC32364 || CPU == MIPS32)
    memset(lcd, 0x0, 4);
#endif

    if (Status == LM_STATUS_LINK_DOWN) {
	sprintf(buf,"%s%d: NIC Link is down\n", devObject->name, ge->ge_unit);
	endObject->mib2Tbl.ifOperStatus = M2_ifOperStatus_down;
	endObject->mib2Tbl.ifAdminStatus = M2_ifAdminStatus_down;
#if (CPU == RC32364 || CPU == MIPS32)
        lcd[0] = 'L';lcd[1]='N';lcd[2]='K';lcd[3] = '?';
#endif
    } else if (Status == LM_STATUS_LINK_ACTIVE) {

	endObject->mib2Tbl.ifOperStatus = M2_ifOperStatus_up;
	endObject->mib2Tbl.ifAdminStatus = M2_ifAdminStatus_up;

	sprintf(buf,"%s%d: ", devObject->name, ge->ge_unit);

	if (pDevice->LineSpeed == LM_LINE_SPEED_1000MBPS){
	    strcat(buf,"1000 Mbps ");
	    endObject->mib2Tbl.ifSpeed = 1000000000;
#if (CPU == RC32364 || CPU == MIPS32)
            lcd[0] = '1';lcd[1]='G';lcd[2]='B';
#endif
	} else if (pDevice->LineSpeed == LM_LINE_SPEED_100MBPS){
	    strcat(buf,"100 Mbps ");
	    endObject->mib2Tbl.ifSpeed =  100000000;	    
#if (CPU == RC32364 || CPU == MIPS32)
            lcd[0] = '1';lcd[1]='0';lcd[2]='0';
#endif
	} else if (pDevice->LineSpeed == LM_LINE_SPEED_10MBPS){
	    strcat(buf,"10 Mbps ");
	    endObject->mib2Tbl.ifSpeed =   10000000;	    
#if (CPU == RC32364 || CPU == MIPS32)
            lcd[0] = '1';lcd[1]='0';lcd[2]=' ';
#endif
	}
	if (pDevice->DuplexMode == LM_DUPLEX_MODE_FULL){
	    strcat(buf, "full duplex");
#if (CPU == RC32364 || CPU == MIPS32)
            lcd[3] = 'F';
#endif
	} else {
	    strcat(buf, "half duplex");
#if (CPU == RC32364 || CPU == MIPS32)
            lcd[3] = 'H';
#endif
	}
	strcat(buf, " link up");
	
	flow_control = pDevice->FlowControl &
	    (LM_FLOW_CONTROL_RECEIVE_PAUSE |
	     LM_FLOW_CONTROL_TRANSMIT_PAUSE);

	if (flow_control) {
	    if (flow_control & LM_FLOW_CONTROL_RECEIVE_PAUSE) {
		strcat(buf,", receive ");
		if (flow_control & LM_FLOW_CONTROL_TRANSMIT_PAUSE)
		    strcat(buf," & transmit ");
	    }
	    else {
		strcat(buf,", transmit ");
	    }
	    strcat(buf,"flow control ON");
	} else {
	    strcat(buf, ", flow control OFF.");
	}
        strcat(buf,"\n");
        /* 
        * Don't use printf because this routine can be called from
        * interrupt context
        */
        if (INT_CONTEXT())
            logMsg("%s",(int)buf,0,0,0,0,0);
        else
            printf("%s",buf);
    }
#if (CPU == RC32364 || CPU == MIPS32)
    sysLedDsply(lcd[0],lcd[1],lcd[2],lcd[3]);
#endif
    return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_FreeRxBuffer(PLM_DEVICE_BLOCK pDevice, PLM_PACKET pPacket)
{

    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    PUM_PACKET pUmPacket;
    void *skb;
    
    pUmPacket = (PUM_PACKET) pPacket;

    if ((skb = pUmPacket->skbuff))
	bcm570xPktFree(pUmDevice->index, skb);

    pUmPacket->skbuff = 0;

    return LM_STATUS_SUCCESS;
}	

unsigned long
MM_AnGetCurrentTime_us(PAN_STATE_INFO pAnInfo)
{
    struct timespec	tv;
    clock_gettime(CLOCK_REALTIME, &tv);
    return (tv.tv_sec + tv.tv_nsec * 0.000000001);
}

/*
 *   Transform an MBUF chain into a single MBUF.
 *   This routine will fail if the amount of data in the
 *   chain overflows a transmit buffer.  In that case,
 *   the incoming MBUF chain will be freed.  This routine can
 *   also fail by not being able to allocate a new MBUF (including
 *   cluster and mbuf headers).  In that case the failure is
 *   non-fatal.  The incoming cluster chain is not freed, giving
 *   the caller the choice of whether to try a retransmit later.
 */
LM_STATUS
MM_CoalesceTxBuffer(PLM_DEVICE_BLOCK pDevice, PLM_PACKET pPacket)
{
    PUM_PACKET pUmPacket = (PUM_PACKET) pPacket;
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    bcm570x_end_t       *ge = (bcm570x_end_t *)pUmDevice->end;
    struct end_object   *eo = &ge->ge_eo;
    void *skbnew;
    CL_BLK_ID cb = NULL;
    M_BLK_ID pMblk = pUmPacket->mb;
    M_BLK_ID pMblkNew;
    int len = 0;

    /* 
    * Traverse the mbufs and find total length, even though
    * we should be able to just read it in the block header
    */
    while (pMblk != NULL)
        {
        if (pMblk->m_len > 0)
            len  += pMblk->m_len;

        pMblk   = pMblk->m_next;
        }

    pMblk = pUmPacket->mb;

    if (len == 0)
        return (LM_STATUS_SUCCESS);

    if (len > BCM570X_END_PK_SZ)
        {
        printf ("bc%d: xmit frame discarded, too big!, size = %d\n",
            pUmDevice->index, len);

        netMblkClChainFree (pMblk);
        pUmPacket->skbuff = 0;

#ifdef INCLUDE_MIBII_RFC2233
        if (eo->pMib2Tbl != NULL)
            {
            eo->pMib2Tbl->m2CtrUpdateRtn(eo->pMib2Tbl,
                               M2_ctrId_ifOutErrors, 1);
            }
#else
        END_ERR_ADD(eo, MIB2_OUT_ERRS, +1);
#endif /* INCLUDE_MIBII_RFC2233 */
        ge->bcm570x_tx_errs++;
        return (LM_STATUS_FAILURE);
        }

    skbnew = bcm570xPktAlloc(pUmDevice->index, BCM570X_END_PK_SZ);

    if (skbnew == NULL)
        {
        pUmDevice->tx_full = 1;
        printf ("bc%d: out of transmit buffers", pUmDevice->index);
#ifdef INCLUDE_MIBII_RFC2233
        if (eo->pMib2Tbl != NULL)
            {
            eo->pMib2Tbl->m2CtrUpdateRtn(eo->pMib2Tbl,
                 M2_ctrId_ifOutErrors, 1);
            }
#else
        END_ERR_ADD(eo, MIB2_OUT_ERRS, +1);
#endif /* INCLUDE_MIBII_RFC2233 */
        ge->bcm570x_tx_errs++;
        return (LM_STATUS_FAILURE);
        }

    /* Create new cluster */
    if ((cb = netClBlkGet(eo->pNetPool, M_DONTWAIT)) == NULL) {
        bcm570xPktFree (pUmDevice->index, skbnew);
#ifdef INCLUDE_MIBII_RFC2233
        if (eo->pMib2Tbl != NULL)
            {
            eo->pMib2Tbl->m2CtrUpdateRtn(eo->pMib2Tbl,
                 M2_ctrId_ifOutErrors, 1);
            }
#else
        END_ERR_ADD(eo, MIB2_OUT_ERRS, +1);
#endif /* INCLUDE_MIBII_RFC2233 */
        return ERROR;
    }

    /* Join cluster. MBLK, and data ... */
 
    if ((pMblkNew = mBlkGet(eo->pNetPool, M_DONTWAIT, MT_DATA)) == NULL) {
        clBlkFree(eo->pNetPool,cb);
        bcm570xPktFree (pUmDevice->index, skbnew);
	printf("TX: mBlkGet: could not get packet!\n");
#ifdef INCLUDE_MIBII_RFC2233
        if (eo->pMib2Tbl != NULL)
            {
            eo->pMib2Tbl->m2CtrUpdateRtn(eo->pMib2Tbl,
                 M2_ctrId_ifOutErrors, 1);
            }
#else
        END_ERR_ADD(eo, MIB2_OUT_ERRS, +1);
#endif /* INCLUDE_MIBII_RFC2233 */
        return ERROR;
    }

    /* Create MBLK-CLBLK */

    netClBlkJoin(cb, skbnew, BCM570X_END_PK_SZ, NULL, 0, 0, 0);
    netMblkClJoin(pMblkNew, cb);

    /* Copy old Mbuf's data and free it */

    len = netMblkToBufCopy(pUmPacket->mb, skbnew, NULL);
    if(ge->polled_mode == 0) {
        netMblkClChainFree (pUmPacket->mb);
    }

    pMblkNew->mBlkPktHdr.len = pMblkNew->mBlkHdr.mLen = len;
    pMblkNew->mBlkHdr.mFlags |= M_PKTHDR;


    /* New packet values */

    pUmPacket->skbuff = skbnew;
    pUmPacket->mb = pMblkNew;
    pUmPacket->lm_packet.u.Tx.FragCount = 1;

    return (LM_STATUS_SUCCESS);
}

LM_STATUS
MM_IndicateRxPacketsDPC(PLM_DEVICE_BLOCK pDevice)
{
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    bcm570x_end_t       *ge = (bcm570x_end_t *)pUmDevice->end;

    PLM_PACKET pPacket;
    PUM_PACKET pUmPacket;
    void *skb;
    int size=0;

    /* Polled mode, indicate RX completion, return OK*/
    if (ge->polled_mode){
	ge->pkt_in = 1;
	return LM_STATUS_SUCCESS;
    }
    /* Interrupt mode */
    ge->rx_dpc = 0;
    while ( TRUE ) {

	pPacket = (PLM_PACKET)
	    QQ_PopHead(&pDevice->RxPacketReceivedQ.Container);

	if (pPacket == 0)
	    break;

	pUmPacket = (PUM_PACKET) pPacket;

	/* If the packet generated an error, reuse buffer */
	if ((pPacket->PacketStatus != LM_STATUS_SUCCESS) ||
	    ((size = pPacket->PacketSize) > pDevice->RxMtu)) {

	    /* reuse skb */
	    QQ_PushTail(&pDevice->RxPacketFreeQ.Container, pPacket);
	    printf("size=%d MTU=%d, rejecting packet!\n",
		   size,pDevice->RxMtu);
	    continue;
	}
	/* Set size and address */
	skb = pUmPacket->skbuff;
	size = pPacket->PacketSize;

	if ((pPacket->Flags & RCV_BD_FLAG_TCP_UDP_CHKSUM_FIELD) &&
	    (pDevice->TaskToOffload & LM_TASK_OFFLOAD_RX_TCP_CHECKSUM) &&
	    (pPacket->u.Rx.TcpUdpChecksum == 0xffff)) {
	    /* skb->ip_summed = CHECKSUM_UNNECESSARY; */
	    printf("notice: ipchecksum\n");
	}
	/* Process the rest in netTask */
	bcm570xReceiveNetJob((int) pDevice, (int) skb, size, (int) pPacket);
    }
    return LM_STATUS_SUCCESS;
}

LM_STATUS
MM_IndicateRxPackets(PLM_DEVICE_BLOCK pDevice)
{
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    bcm570x_end_t       *ge = (bcm570x_end_t *)pUmDevice->end;
    struct end_object   *eo = &ge->ge_eo;
    PLM_PACKET pPacket;

    /* Polled mode, indicate RX completion, return OK*/
    if (ge->polled_mode){
	ge->pkt_in = 1;
	return LM_STATUS_SUCCESS;
    }

    /* Only queue Rx DPC if necessary */
    if (ge->rx_dpc){
	return LM_STATUS_SUCCESS;
    }

	/* Process the rest in netTask */
	if (netJobAdd ((FUNCPTR) MM_IndicateRxPacketsDPC,
            (int) pDevice, (int) 2, 3, (int) 4, 0xDDDDDDDD) == ERROR)
            {
            bcm570x_njafail_ipkt++;
            if (bcm570x_npen) { sysSerialPrintString("IPKT\n"); }
            /* 
            * move all packets from receive queue to free queue
            * increment error and discard counters for each packet
            */
                pPacket = (PLM_PACKET) 
                    QQ_PopHead (&pDevice->RxPacketReceivedQ.Container);
                while (pPacket != 0) {
                ge->bcm570x_rx_errs++;
#ifdef INCLUDE_MIBII_RFC2233
                if (eo->pMib2Tbl != NULL)
                    {
                    eo->pMib2Tbl->m2CtrUpdateRtn(eo->pMib2Tbl,
                                                    M2_ctrId_ifInDiscards,1);
                    }
#else
                END_ERR_ADD(eo, MIB2_IN_ERRS, +1);
#endif /* INCLUDE_MIBII_RFC2233 */
                QQ_PushTail(&pDevice->RxPacketFreeQ.Container, pPacket);
                pPacket = (PLM_PACKET) 
                    QQ_PopHead (&pDevice->RxPacketReceivedQ.Container);
                }
            }
        else {
            ge->rx_dpc = 1;
            }

    return LM_STATUS_SUCCESS;
}


LM_STATUS
MM_IndicateTxPackets(PLM_DEVICE_BLOCK pDevice)
{
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    bcm570x_end_t *ge = (bcm570x_end_t *)pUmDevice->end;

    PLM_PACKET pPacket;
    PUM_PACKET pUmPacket;
    void *skb;

    while ( TRUE ) {

	pPacket = (PLM_PACKET)
	    QQ_PopHead(&pDevice->TxPacketXmittedQ.Container);

	if (pPacket == 0)
	    break;

	pUmPacket = (PUM_PACKET) pPacket;
	skb = (void*)pUmPacket->skbuff;

        /*
        * Free MBLK if we transmitted a fragmented packet or a
        * non-fragmented packet straight from the VxWorks
        * buffer pool. If packet was copied to a local transmit
        * buffer, then there's no MBUF to free, just free
        * the transmit buffer back to the cluster pool.
        */

        if ( pUmPacket->mb ){

            if(pUmPacket->mb->m_data != skb)
            printf("free tx buf mismatch. pUmP=0x%08x Exp=0x%08x Act=%08x\n",
                (int)pUmPacket, (int)pUmPacket->mb->m_data, (int)skb);
            netMblkClChainFree(pUmPacket->mb);
        }
        else {
            if (skb)
                bcm570xPktFree (pUmDevice->index, skb);
            else
                printf("free tx: error: buffer NULL buffer.\n");
        }

	pUmPacket->skbuff = 0;
	pUmPacket->mb = 0;
	
	QQ_PushTail(&pDevice->TxPacketFreeQ.Container, pPacket);
	ge->pkt_out = 1;
    }
    if (pUmDevice->tx_full) {
	if (QQ_GetEntryCnt(&pDevice->TxPacketFreeQ.Container) >=
	    (QQ_GetSize(&pDevice->TxPacketFreeQ.Container) >> 1))

	    pUmDevice->tx_full = 0;
    }
    return LM_STATUS_SUCCESS;
}

/*
 *  Scan an MBUF chain until we reach fragment number "frag"
 *  Return its length and physical address.
 */
void MM_MapTxDma
    (
    PLM_DEVICE_BLOCK pDevice,
    struct _LM_PACKET *pPacket,
    T3_64BIT_HOST_ADDR *paddr,
    LM_UINT32 *len,
    int frag) 
{
    PUM_PACKET pUmPacket = (PUM_PACKET) pPacket;
    M_BLK_ID pMblk;
    dma_addr_t map;
    void *skb;

    pMblk = pUmPacket->mb;
    while (frag) {
        pMblk = pMblk->m_next;
        frag--;
    }
 
    *len = pMblk->m_len;
    skb = pMblk->m_data;

#if (CPU==MIPS32 || CPU==RC32364)
    /*
    *  In the MIPS world, the physical address is cached! 
    *  PHYS_TO_VIRT will give the KSEG1 address
    */
    map = MEM_TO_PCI_PHYS (CACHE_DMA_PHYS_TO_VIRT (skb));
#else
    map = MEM_TO_PCI_PHYS (CACHE_DMA_VIRT_TO_PHYS (skb));
#endif

    CACHE_DRV_FLUSH (pCacheFuncs, skb, *len);

    MM_SetT3Addr(paddr, (dma_addr_t) map);
}

/*
 *  Convert an mbuf address, a CPU local virtual address,
 *  to a physical address as seen from a PCI device.  Store the
 *  result at paddr.
 */
void MM_MapRxDma
    (
    PLM_DEVICE_BLOCK pDevice,
    struct _LM_PACKET *pPacket,
    T3_64BIT_HOST_ADDR *paddr) {
    PUM_PACKET pUmPacket = (PUM_PACKET) pPacket;
    void *skb =  pUmPacket->skbuff;
    void * map;
#ifdef USE_ZERO_COPY_RX
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) pDevice;
    bcm570x_end_t       *ge = (bcm570x_end_t *)pUmDevice->end;
#endif


#if (CPU==MIPS32 || CPU==RC32364)
    /*
    *  In the MIPS world, the physical address is cached! 
    *  PHYS_TO_VIRT will give the KSEG1 address
    */
    map = MEM_TO_PCI_PHYS (CACHE_DMA_PHYS_TO_VIRT (skb));
#else
    map = MEM_TO_PCI_PHYS (CACHE_DMA_VIRT_TO_PHYS (skb));
#endif

    CACHE_DRV_INVALIDATE (pCacheFuncs, skb,
        pPacket->u.Rx.RxBufferSize - _CACHE_ALIGN_SIZE);

#ifdef USE_ZERO_COPY_RX
    if (ge->align)
        map = (char *) map + ge->align;
#endif
    MM_SetT3Addr(paddr, (dma_addr_t) map);
}

void 
MM_SetAddr (LM_PHYSICAL_ADDRESS *paddr, dma_addr_t addr)
{
#if (BITS_PER_LONG == 64)
        paddr->High = ((unsigned long) addr) >> 32;
        paddr->Low = ((unsigned long) addr) & 0xffffffff;
#else
        paddr->High = 0;
        paddr->Low = (unsigned long) addr;
#endif
}

void
MM_SetT3Addr(T3_64BIT_HOST_ADDR *paddr, dma_addr_t addr)
{
        unsigned long baddr = (unsigned long) addr;
#if (BITS_PER_LONG == 64)
        set_64bit_addr(paddr, baddr & 0xffffffff, baddr >> 32);
#else
        set_64bit_addr(paddr, baddr, 0);
#endif
}

/*
 * This combination of `inline' and `extern' has almost the effect of a
 * macro.  The way to use it is to put a function definition in a header
 * file with these keywords, and put another copy of the definition
 * (lacking `inline' and `extern') in a library file.  The definition in
 * the header file will cause most calls to the function to be inlined.
 * If any uses of the function remain, they will refer to the single copy
 * in the library.
 */
void
atomic_set(atomic_t* entry, int val)
{
    int il;
    il = intLock();
    entry->counter = val;
    intUnlock(il);    
}
int
atomic_read(atomic_t* entry)
{
    int il;
    int val;
    il = intLock();
    val =  entry->counter;
    intUnlock(il);
    return val;
}
void
atomic_inc(atomic_t* entry)
{
    int il;
    il = intLock();
    if(entry)
	entry->counter++;
    intUnlock(il);
}

void
atomic_dec(atomic_t* entry)
{
    int il;
    il = intLock();
    if(entry)
	entry->counter--;
    intUnlock(il);
}

void
atomic_sub(int a, atomic_t* entry)
{
    int il;
    il = intLock();
    if(entry)
	entry->counter -= a;
    intUnlock(il);
}

void
atomic_add(int a, atomic_t* entry)
{
    int il;
    il = intLock();
    if(entry)
	entry->counter += a;
    intUnlock(il);
}

/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
void 
QQ_InitQueue(
PQQ_CONTAINER pQueue,
unsigned int QueueSize) {
    pQueue->Head = 0;
    pQueue->Tail = 0;
    pQueue->Size = QueueSize+1;
    atomic_set(&pQueue->EntryCnt, 0);
} /* QQ_InitQueue */



/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
char 
QQ_Full(
PQQ_CONTAINER pQueue) {
    unsigned int NewHead;

    NewHead = (pQueue->Head + 1) % pQueue->Size;

    return(NewHead == pQueue->Tail);
} /* QQ_Full */



/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
char 
QQ_Empty(
PQQ_CONTAINER pQueue) {
    return(pQueue->Head == pQueue->Tail);
} /* QQ_Empty */



/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
unsigned int 
QQ_GetSize(
PQQ_CONTAINER pQueue) {
    return pQueue->Size;
} /* QQ_GetSize */



/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
unsigned int 
QQ_GetEntryCnt(
PQQ_CONTAINER pQueue) {
    return atomic_read(&pQueue->EntryCnt);
} /* QQ_GetEntryCnt */



/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/*    TRUE entry was added successfully.                                      */
/*    FALSE queue is full.                                                    */
/******************************************************************************/
char 
QQ_PushHead(
PQQ_CONTAINER pQueue, 
PQQ_ENTRY pEntry) {
    unsigned int Head;

    Head = (pQueue->Head + 1) % pQueue->Size;

#if !defined(QQ_NO_OVERFLOW_CHECK)
    if(Head == pQueue->Tail) {
        return 0;
    } /* if */
#endif /* QQ_NO_OVERFLOW_CHECK */

    pQueue->Array[pQueue->Head] = pEntry;
    wmb();
    pQueue->Head = Head;
    atomic_inc(&pQueue->EntryCnt);

    return -1;
} /* QQ_PushHead */



/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/*    TRUE entry was added successfully.                                      */
/*    FALSE queue is full.                                                    */
/******************************************************************************/
char 
QQ_PushTail(
PQQ_CONTAINER pQueue,
PQQ_ENTRY pEntry) {
    unsigned int Tail;

    Tail = pQueue->Tail;
    if(Tail == 0) {
        Tail = pQueue->Size;
    } /* if */
    Tail--;

#if !defined(QQ_NO_OVERFLOW_CHECK)
    if(Tail == pQueue->Head) {
        return 0;
    } /* if */
#endif /* QQ_NO_OVERFLOW_CHECK */

    pQueue->Array[Tail] = pEntry;
    wmb();
    pQueue->Tail = Tail;
    atomic_inc(&pQueue->EntryCnt);

    return -1;
} /* QQ_PushTail */



/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
PQQ_ENTRY
QQ_PopHead(
PQQ_CONTAINER pQueue) {
    unsigned int Head;
    PQQ_ENTRY Entry;

    Head = pQueue->Head;

#if !defined(QQ_NO_UNDERFLOW_CHECK)
    if(Head == pQueue->Tail) {
        return (PQQ_ENTRY) 0;
    } /* if */
#endif /* QQ_NO_UNDERFLOW_CHECK */

    if(Head == 0) {
        Head = pQueue->Size;
    } /* if */
    Head--;

    Entry = pQueue->Array[Head];
#ifdef VXWORKS
    membar();
#else
    mb();
#endif
    pQueue->Head = Head;
    atomic_dec(&pQueue->EntryCnt);

    return Entry;
} /* QQ_PopHead */



/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
PQQ_ENTRY
QQ_PopTail(
PQQ_CONTAINER pQueue) {
    unsigned int Tail;
    PQQ_ENTRY Entry;

    Tail = pQueue->Tail;

#if !defined(QQ_NO_UNDERFLOW_CHECK)
    if(Tail == pQueue->Head) {
        return (PQQ_ENTRY) 0;
    } /* if */
#endif /* QQ_NO_UNDERFLOW_CHECK */

    Entry = pQueue->Array[Tail];
#ifdef VXWORKS
    membar();
#else
    mb();
#endif
    pQueue->Tail = (Tail + 1) % pQueue->Size;
    atomic_dec(&pQueue->EntryCnt);

    return Entry;
} /* QQ_PopTail */



/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
PQQ_ENTRY
QQ_GetHead(
    PQQ_CONTAINER pQueue,
    unsigned int Idx)
{
    if(Idx >= atomic_read(&pQueue->EntryCnt))
    {
        return (PQQ_ENTRY) 0;
    }

    if(pQueue->Head > Idx)
    {
        Idx = pQueue->Head - Idx;
    }
    else
    {
        Idx = pQueue->Size - (Idx - pQueue->Head);
    }
    Idx--;

    return pQueue->Array[Idx];
}



/******************************************************************************/
/* Description:                                                               */
/*                                                                            */
/* Return:                                                                    */
/******************************************************************************/
PQQ_ENTRY
QQ_GetTail(
    PQQ_CONTAINER pQueue,
    unsigned int Idx)
{
    if(Idx >= atomic_read(&pQueue->EntryCnt))
    {
        return (PQQ_ENTRY) 0;
    }

    Idx += pQueue->Tail;
    if(Idx >= pQueue->Size)
    {
        Idx = Idx - pQueue->Size;
    }

    return pQueue->Array[Idx];
}
#ifdef BCM570X_DEBUG_TASK
void
vxDebug570x(void)
{
    printf("Waiting for attach...\n");
    taskDelay( sysClkRateGet() * 20 );
    BCM570x();
}

STATUS
dbgTigon()
{
    taskSpawn("tigon",
              10, 0, 8000, (FUNCPTR)vxDebug570x, 0,0,0,0,0,0,0,0,0,0);
    return 0;
}
#endif

/* DEBUG */
void
bcm570xQstatus()
{
    atomic_t bcm570x_in_rxq={0};
    int rx_out_of_buf_q,
        RxPacketFreeQ,
        RxPacketReceivedQ,
        TxPacketFreeQ,
        TxPacketActiveQ,
        TxPacketXmittedQ;
    int il;
    bcm570x_end_t* ge = tigonBuffPool[0].geu_devices;
    PLM_DEVICE_BLOCK pDevice = (PLM_DEVICE_BLOCK)ge->pDevice;
    PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK)ge->pUmDevice;    

    il = intLock();
    
    bcm570x_in_rxq = pDevice->RxPacketQueuedCnt;
    rx_out_of_buf_q = QQ_GetEntryCnt(&pUmDevice->rx_out_of_buf_q.Container);
	RxPacketFreeQ = QQ_GetEntryCnt(&pDevice->RxPacketFreeQ.Container);
	RxPacketReceivedQ = QQ_GetEntryCnt(&pDevice->RxPacketReceivedQ.Container);
	TxPacketFreeQ = QQ_GetEntryCnt(&pDevice->TxPacketFreeQ.Container);
	TxPacketActiveQ = QQ_GetEntryCnt(&pDevice->TxPacketActiveQ.Container);
	TxPacketXmittedQ = QQ_GetEntryCnt(&pDevice->TxPacketXmittedQ.Container);

    intUnlock(il);

    printf("bcm570x_in_rxq = %d\n",bcm570x_in_rxq.counter);
    printf("rx_out_of_buf_q = %d\nRxPacketFreeQ = %d\nRxPacketReceivedQ= %d\n",
            rx_out_of_buf_q,    RxPacketFreeQ,          RxPacketReceivedQ);
    printf("TxPacketFreeQ = %d\nTxPacketActiveQ = %d\nTxPacketXmittedQ= %d\n",
            TxPacketFreeQ,    TxPacketActiveQ,          TxPacketXmittedQ);
    printf("Total RX=%d  TX=%d\n",
        rx_out_of_buf_q + RxPacketFreeQ + RxPacketReceivedQ + bcm570x_in_rxq.counter,
        TxPacketFreeQ   + TxPacketActiveQ + TxPacketXmittedQ
        );
}

#ifdef RNG_DBG
#include <rngLib.h>

int rj_dump_verbose = 0;

void
drb()
{
    int ilevel;
    RING_ID r;
    RING rd;
    SEM_ID s;
    UINT32 *nTask;
    static UINT8 *rb=0;

    nTask = (UINT32 *)netTask;

    r = (RING_ID)*(UINT32 *)(((nTask[8] & 0xFFFF) << 16)
                            + (nTask[0xf] & 0xFFFF));
    s = (SEM_ID)*(UINT32 *)(((nTask[9] & 0xFFFF) << 16)
                            + (nTask[0xc] & 0xFFFF));

    if (rb == 0) {
        rb = malloc(r->bufSize);
    }

    ilevel = intLock();
    rd = *r;
    if ((rb) && (rj_dump_verbose)) {
        memcpy(rb, rd.buf, rd.bufSize);
    }
    intUnlock(ilevel);

    if ((rb) && (rj_dump_verbose)) {
        d(rd.buf, (rd.bufSize + 3)/4, 4);
    }
    show(s);
    printf("pToBuf = 0x%08x pFromBuf = 0x%08x bufSize = 0x%08x buf = 0x%08x\n",
            rd.pToBuf, rd.pFromBuf, rd.bufSize, (UINT32)rd.buf);
    printf("RB Tail -> 0x%08x RB Head -> 0x%08x Last Job = %02x-%02x\n",
            ((int)rd.buf + rd.pToBuf),
            ((int)rd.buf + rd.pFromBuf),
            *(rd.buf + (rd.pFromBuf + rd.bufSize - 1) % rd.bufSize),
            *(rd.buf + rd.pToBuf));
    printf("SEM_ID=0x%08x  RING_ID = 0x%08x\n", (UINT32)s, (UINT32)r);
}
#endif

/* End of module */
