/*
 * $Id: b57proc.c,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

/******************************************************************************/
/*                                                                            */
/* Broadcom BCM5700 Linux Network Driver, Copyright (c) 2000 Broadcom         */
/* Corporation.                                                               */
/* All rights reserved.                                                       */
/*                                                                            */
/* /proc file system handling code.                                           */
/*                                                                            */
/******************************************************************************/

#include "mm.h"
#ifdef CONFIG_PROC_FS

#define NICINFO_PROC_DIR "nicinfo"

static struct proc_dir_entry *bcm5700_procfs_dir;

extern char bcm5700_driver[], bcm5700_version[];

extern LM_UINT32 bcm5700_crc_count(PUM_DEVICE_BLOCK pUmDevice);

static struct proc_dir_entry *
proc_getdir(char *name, struct proc_dir_entry *proc_dir)
{
	struct proc_dir_entry *pde = proc_dir;

	lock_kernel();
	for (pde=pde->subdir; pde; pde = pde->next) {
		if (pde->namelen && (strcmp(name, pde->name) == 0)) {
			/* directory exists */
			break;
		}
	}
	if (pde == (struct proc_dir_entry *) 0)
	{
		/* create the directory */
#if (LINUX_VERSION_CODE > 0x20300)
		pde = proc_mkdir(name, proc_dir);
#else
		pde = create_proc_entry(name, S_IFDIR, proc_dir);
#endif
		if (pde == (struct proc_dir_entry *) 0) {
			unlock_kernel();
			return (pde);
		}
	}
	unlock_kernel();
	return (pde);
}

int
bcm5700_proc_create(void)
{
	bcm5700_procfs_dir = proc_getdir(NICINFO_PROC_DIR, proc_net);

	if (bcm5700_procfs_dir == (struct proc_dir_entry *) 0) {
		printk(KERN_DEBUG "Could not create procfs nicinfo directory %s\n", NICINFO_PROC_DIR);
		return -1;
	}
	return 0;
}

int
bcm5700_read_pfs(char *page, char **start, off_t off, int count,
	int *eof, void *data)
{
	struct net_device *dev = (struct net_device *) data;
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) dev->priv;
	PLM_DEVICE_BLOCK pDevice = &pUmDevice->lm_dev;
	PT3_STATS_BLOCK pStats = (PT3_STATS_BLOCK) pDevice->pStatsBlkVirt;
	int len = 0;
	LM_UINT32 rx_mac_errors, rx_crc_errors, rx_align_errors;
	LM_UINT32 rx_runt_errors, rx_frag_errors, rx_long_errors;
	LM_UINT32 rx_overrun_errors, rx_jabber_errors;

	if (pUmDevice->opened == 0)
		pStats = 0;

	len += sprintf(page+len, "Description\t\t\t%s\n", pUmDevice->name);
	len += sprintf(page+len, "Driver_Name\t\t\t%s\n", bcm5700_driver);
	len += sprintf(page+len, "Driver_Version\t\t\t%s\n", bcm5700_version);
	len += sprintf(page+len, "Bootcode_Version\t\t%s\n", pDevice->BootCodeVer);
	len += sprintf(page+len, "PCI_Vendor\t\t\t0x%04x\n", pDevice->PciVendorId);
	len += sprintf(page+len, "PCI_Device_ID\t\t\t0x%04x\n",
		pDevice->PciDeviceId);
	len += sprintf(page+len, "PCI_Subsystem_Vendor\t\t0x%04x\n",
		pDevice->SubsystemVendorId);
	len += sprintf(page+len, "PCI_Subsystem_ID\t\t0x%04x\n",
		pDevice->SubsystemId);
	len += sprintf(page+len, "PCI_Revision_ID\t\t\t0x%02x\n",
		pDevice->PciRevId);
	len += sprintf(page+len, "PCI_Slot\t\t\t%d\n",
		PCI_SLOT(pUmDevice->pdev->devfn));
	if (T3_ASIC_REV(pDevice->ChipRevId) == T3_ASIC_REV_5704) {
		len += sprintf(page+len, "PCI_Function\t\t\t%d\n",
			PCI_FUNC(pUmDevice->pdev->devfn));
	}
	len += sprintf(page+len, "PCI_Bus\t\t\t\t%d\n",
		pUmDevice->pdev->bus->number);

	len += sprintf(page+len, "PCI_Bus_Speed\t\t\t%s\n",
		pDevice->BusSpeedStr);

	len += sprintf(page+len, "Memory\t\t\t\t0x%lx\n", pUmDevice->dev->base_addr);
	len += sprintf(page+len, "IRQ\t\t\t\t%d\n", dev->irq);
	len += sprintf(page+len, "System_Device_Name\t\t%s\n", dev->name);
	len += sprintf(page+len, "Current_HWaddr\t\t\t%02x:%02x:%02x:%02x:%02x:%02x\n",
		dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
		dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);
	len += sprintf(page+len,
		"Permanent_HWaddr\t\t%02x:%02x:%02x:%02x:%02x:%02x\n",
		pDevice->NodeAddress[0], pDevice->NodeAddress[1],
		pDevice->NodeAddress[2], pDevice->NodeAddress[3],
		pDevice->NodeAddress[4], pDevice->NodeAddress[5]);
	len += sprintf(page+len, "Part_Number\t\t\t%s\n\n", pDevice->PartNo);

	len += sprintf(page+len, "Link\t\t\t\t%s\n", 
		(pUmDevice->opened == 0) ? "unknown" :
    		((pDevice->LinkStatus == LM_STATUS_LINK_ACTIVE) ? "up" :
		"down"));
	len += sprintf(page+len, "Speed\t\t\t\t%s\n", 
    		(pDevice->LinkStatus == LM_STATUS_LINK_DOWN) ? "N/A" :
    		((pDevice->LineSpeed == LM_LINE_SPEED_1000MBPS) ? "1000" :
    		((pDevice->LineSpeed == LM_LINE_SPEED_100MBPS) ? "100" :
    		(pDevice->LineSpeed == LM_LINE_SPEED_10MBPS) ? "10" : "N/A")));
	len += sprintf(page+len, "Duplex\t\t\t\t%s\n", 
    		(pDevice->LinkStatus == LM_STATUS_LINK_DOWN) ? "N/A" :
		((pDevice->DuplexMode == LM_DUPLEX_MODE_FULL) ? "full" :
			"half"));
	len += sprintf(page+len, "Flow_Control\t\t\t%s\n", 
    		(pDevice->LinkStatus == LM_STATUS_LINK_DOWN) ? "N/A" :
		((pDevice->FlowControl == LM_FLOW_CONTROL_NONE) ? "off" :
		(((pDevice->FlowControl & LM_FLOW_CONTROL_RX_TX_PAUSE) ==
			LM_FLOW_CONTROL_RX_TX_PAUSE) ? "receive/transmit" :
		(pDevice->FlowControl & LM_FLOW_CONTROL_RECEIVE_PAUSE) ?
			"receive" : "transmit")));
	len += sprintf(page+len, "State\t\t\t\t%s\n", 
    		(dev->flags & IFF_UP) ? "up" : "down");
	len += sprintf(page+len, "MTU_Size\t\t\t%d\n\n", dev->mtu);
	len += sprintf(page+len, "Rx_Packets\t\t\t%u\n", 
			((pStats == 0) ? 0 :
			pStats->ifHCInUcastPkts.Low +
			pStats->ifHCInMulticastPkts.Low +
			pStats->ifHCInBroadcastPkts.Low));
	if (dev->mtu > 1500) {
		len += sprintf(page+len, "Rx_Jumbo_Packets\t\t%u\n", 
			((pStats == 0) ? 0 :
			pStats->etherStatsPkts1523Octetsto2047Octets.Low +
			pStats->etherStatsPkts2048Octetsto4095Octets.Low +
			pStats->etherStatsPkts4096Octetsto8191Octets.Low +
			pStats->etherStatsPkts8192Octetsto9022Octets.Low));
	}
	len += sprintf(page+len, "Tx_Packets\t\t\t%u\n",
		((pStats == 0) ? 0 :
		pStats->COSIfHCOutPkts[0].Low));
	len += sprintf(page+len, "Rx_Bytes\t\t\t%u\n",
		((pStats == 0) ? 0 :
		pStats->ifHCInOctets.Low));
	len += sprintf(page+len, "Tx_Bytes\t\t\t%u\n",
		((pStats == 0) ? 0 :
		pStats->ifHCOutOctets.Low));
	if (pStats == 0) {
		rx_crc_errors = 0;
		rx_align_errors = 0;
		rx_runt_errors = 0;
		rx_frag_errors = 0;
		rx_long_errors = 0;
		rx_overrun_errors = 0;
		rx_jabber_errors = 0;
	}
	else {
		rx_crc_errors = bcm5700_crc_count(pUmDevice);
		rx_align_errors = pStats->dot3StatsAlignmentErrors.Low;
		rx_runt_errors = pStats->etherStatsUndersizePkts.Low;
		rx_frag_errors = pStats->etherStatsFragments.Low;
		rx_long_errors = pStats->dot3StatsFramesTooLong.Low;
		rx_overrun_errors = pStats->nicNoMoreRxBDs.Low;
		rx_jabber_errors = pStats->etherStatsJabbers.Low;
	}
	rx_mac_errors = rx_crc_errors + rx_align_errors + rx_runt_errors +
		rx_frag_errors + rx_long_errors + rx_jabber_errors;
	len += sprintf(page+len, "Rx_Errors\t\t\t%u\n",
		((pStats == 0) ? 0 :
		rx_mac_errors + rx_overrun_errors + pUmDevice->rx_misc_errors));
	len += sprintf(page+len, "Tx_Errors\t\t\t%u\n",
		((pStats == 0) ? 0 :
		pStats->ifOutErrors.Low));
	len += sprintf(page+len, "\nTx_Carrier_Errors\t\t%u\n",
		((pStats == 0) ? 0 :
		pStats->dot3StatsCarrierSenseErrors.Low));
	len += sprintf(page+len, "Tx_Abort_Excess_Coll\t\t%u\n",
		((pStats == 0) ? 0 :
    		pStats->dot3StatsExcessiveCollisions.Low));
	len += sprintf(page+len, "Tx_Abort_Late_Coll\t\t%u\n",
		((pStats == 0) ? 0 :
    		pStats->dot3StatsLateCollisions.Low));
	len += sprintf(page+len, "Tx_Deferred_Ok\t\t\t%u\n",
		((pStats == 0) ? 0 :
    		pStats->dot3StatsDeferredTransmissions.Low));
	len += sprintf(page+len, "Tx_Single_Coll_Ok\t\t%u\n",
		((pStats == 0) ? 0 :
    		pStats->dot3StatsSingleCollisionFrames.Low));
	len += sprintf(page+len, "Tx_Multi_Coll_Ok\t\t%u\n",
		((pStats == 0) ? 0 :
    		pStats->dot3StatsMultipleCollisionFrames.Low));
	len += sprintf(page+len, "Tx_Total_Coll_Ok\t\t%u\n",
		((pStats == 0) ? 0 :
		pStats->etherStatsCollisions.Low));
	len += sprintf(page+len, "\nRx_CRC_Errors\t\t\t%u\n", rx_crc_errors);
	len += sprintf(page+len, "Rx_Short_Fragment_Errors\t%u\n",
		rx_frag_errors);
	len += sprintf(page+len, "Rx_Short_Length_Errors\t\t%u\n",
		rx_runt_errors);
	len += sprintf(page+len, "Rx_Long_Length_Errors\t\t%u\n",
		rx_long_errors);
	len += sprintf(page+len, "Rx_Align_Errors\t\t\t%u\n",
		rx_align_errors);
	len += sprintf(page+len, "Rx_Overrun_Errors\t\t%u\n",
		rx_overrun_errors);
	len += sprintf(page+len, "\nTx_MAC_Errors\t\t\t%u\n",
		((pStats == 0) ? 0 :
		pStats->dot3StatsInternalMacTransmitErrors.Low));
	len += sprintf(page+len, "Rx_MAC_Errors\t\t\t%u\n\n",
		rx_mac_errors);

	len += sprintf(page+len, "Tx_Checksum\t\t\t%s\n",
		((pDevice->TaskToOffload & LM_TASK_OFFLOAD_TX_TCP_CHECKSUM) ?
		"ON" : "OFF"));
	len += sprintf(page+len, "Rx_Checksum\t\t\t%s\n",
		((pDevice->TaskToOffload & LM_TASK_OFFLOAD_RX_TCP_CHECKSUM) ?
		"ON" : "OFF"));
	len += sprintf(page+len, "Scatter_Gather\t\t\t%s\n",
#if (LINUX_VERSION_CODE >= 0x20400)
		((dev->features & NETIF_F_SG) ? "ON" : "OFF"));
#else
		"OFF");
#endif
	len += sprintf(page+len, "Tx_Desc_Count\t\t\t%u\n",
		pDevice->TxPacketDescCnt);
	len += sprintf(page+len, "Rx_Desc_Count\t\t\t%u\n",
		pDevice->RxStdDescCnt);
	len += sprintf(page+len, "Rx_Jumbo_Desc_Count\t\t%u\n",
		pDevice->RxJumboDescCnt);
	len += sprintf(page+len, "Adaptive_Coalescing\t\t%s\n",
		(pUmDevice->adaptive_coalesce ? "ON" : "OFF"));
	len += sprintf(page+len, "Rx_Coalescing_Ticks\t\t%u\n",
		pUmDevice->rx_curr_coalesce_ticks);
	len += sprintf(page+len, "Rx_Coalesced_Frames\t\t%u\n",
		pUmDevice->rx_curr_coalesce_frames);
	len += sprintf(page+len, "Tx_Coalescing_Ticks\t\t%u\n",
		pDevice->TxCoalescingTicks);
	len += sprintf(page+len, "Tx_Coalesced_Frames\t\t%u\n",
		pUmDevice->tx_curr_coalesce_frames);
	len += sprintf(page+len, "Stats_Coalescing_Ticks\t\t%u\n",
		pDevice->StatsCoalescingTicks);
	len += sprintf(page+len, "Wake_On_LAN\t\t\t%s\n",
        	((pDevice->WakeUpMode & LM_WAKE_UP_MODE_MAGIC_PACKET) ?
		"ON" : "OFF"));
#if TIGON3_DEBUG
	len += sprintf(page+len, "\nDmaReadWriteCtrl\t\t%x\n",
		pDevice->DmaReadWriteCtrl);
	len += sprintf(page+len, "\nTx_Zero_Copy_Packets\t\t%u\n",
		pUmDevice->tx_zc_count);
	len += sprintf(page+len, "Tx_Chksum_Packets\t\t%u\n",
		pUmDevice->tx_chksum_count);
	len += sprintf(page+len, "Tx_Highmem_Fragments\t\t%u\n",
		pUmDevice->tx_himem_count);
	len += sprintf(page+len, "Rx_Good_Chksum_Packets\t\t%u\n",
		pUmDevice->rx_good_chksum_count);
	len += sprintf(page+len, "Rx_Bad_Chksum_Packets\t\t%u\n",
		pUmDevice->rx_bad_chksum_count);
	if (!pDevice->EnableTbi) {
		LM_UINT32 value32;

        	LM_ReadPhy(pDevice, 0, &value32);
		len += sprintf(page+len, "\nPhy_Register_0x00\t\t0x%x\n", value32);
        	LM_ReadPhy(pDevice, 1, &value32);
		len += sprintf(page+len, "Phy_Register_0x01\t\t0x%x\n", value32);
        	LM_ReadPhy(pDevice, 2, &value32);
		len += sprintf(page+len, "Phy_Register_0x02\t\t0x%x\n", value32);
        	LM_ReadPhy(pDevice, 3, &value32);
		len += sprintf(page+len, "Phy_Register_0x03\t\t0x%x\n", value32);
        	LM_ReadPhy(pDevice, 4, &value32);
		len += sprintf(page+len, "Phy_Register_0x04\t\t0x%x\n", value32);
        	LM_ReadPhy(pDevice, 5, &value32);
		len += sprintf(page+len, "Phy_Register_0x05\t\t0x%x\n", value32);
        	LM_ReadPhy(pDevice, 9, &value32);
		len += sprintf(page+len, "Phy_Register_0x09\t\t0x%x\n", value32);
        	LM_ReadPhy(pDevice, 0xa, &value32);
		len += sprintf(page+len, "Phy_Register_0x0A\t\t0x%x\n", value32);
        	LM_ReadPhy(pDevice, 0xf, &value32);
		len += sprintf(page+len, "Phy_Register_0x0F\t\t0x%x\n", value32);
        	LM_ReadPhy(pDevice, 0x10, &value32);
		len += sprintf(page+len, "Phy_Register_0x10\t\t0x%x\n", value32);
        	LM_ReadPhy(pDevice, 0x19, &value32);
		len += sprintf(page+len, "Phy_Register_0x19\t\t0x%x\n", value32);
	}
#endif

	*eof = 1;
	return len;
}

int
bcm5700_proc_create_dev(struct net_device *dev)
{
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) dev->priv;

	if (!bcm5700_procfs_dir)
		return -1;

	sprintf(pUmDevice->pfs_name, "%s.info", dev->name);
	pUmDevice->pfs_entry = create_proc_entry(pUmDevice->pfs_name,
		S_IFREG, bcm5700_procfs_dir);
	if (pUmDevice->pfs_entry == 0)
		return -1;
	pUmDevice->pfs_entry->read_proc = bcm5700_read_pfs;
	pUmDevice->pfs_entry->data = dev;
	return 0;
}
int
bcm5700_proc_remove_dev(struct net_device *dev)
{
	PUM_DEVICE_BLOCK pUmDevice = (PUM_DEVICE_BLOCK) dev->priv;

	remove_proc_entry(pUmDevice->pfs_name, bcm5700_procfs_dir);
	return 0;
}

#endif
