/*
 * HND SiliconBackplane PCI core software interface.
 *
 * $Id: hndpci.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _hndpci_h_
#define _hndpci_h_

/* Starting bus for secondary PCIE port */
#define PCIE_PORT1_BUS_START    (17)

/* Host bridge */
#define PCIE_PORT0_HB_BUS       (1)
#define PCIE_PORT1_HB_BUS       (17)

/* PCIE mapped addresses per port */
#define SI_PCI_MEM(p)           ((p)? SI_PCI1_MEM : SI_PCI0_MEM)
#define SI_PCI_CFG(p)           ((p)? SI_PCI1_CFG : SI_PCI0_CFG)
#define SI_PCIE_DMA_HIGH(p)     ((p)? SI_PCIE1_DMA_H32 : SI_PCIE_DMA_H32)

/* Determine actual port by bus number */
#define PCIE_GET_PORT_BY_BUS(bus)       \
    ((bus) >= PCIE_PORT1_BUS_START ? 1 : 0)

/* Check if the given bus has a host bridge */
#define PCIE_IS_BUS_HOST_BRIDGE(bus)    \
    ((bus == PCIE_PORT0_HB_BUS) || (bus == PCIE_PORT1_HB_BUS))

/* Get bus number that has a host bridge by given port */    
#define PCIE_GET_HOST_BRIDGE_BUS(port)   \
    ((port)? PCIE_PORT1_HB_BUS : PCIE_PORT0_HB_BUS)


extern int hndpci_read_config(si_t *sih, uint bus, uint dev, uint func, uint off, void *buf,
                              int len);
extern int extpci_read_config(si_t *sih, uint bus, uint dev, uint func, uint off, void *buf,
                              int len);
extern int hndpci_write_config(si_t *sih, uint bus, uint dev, uint func, uint off, void *buf,
                               int len);
extern int extpci_write_config(si_t *sih, uint bus, uint dev, uint func, uint off, void *buf,
                               int len);
extern void hndpci_ban(uint16 core, uint8 unit);
extern int hndpci_init(si_t *sih);
extern int hndpci_init_pci(si_t *sih, uint8 port);
extern void hndpci_init_cores(si_t *sih);
extern void hndpci_arb_park(si_t *sih, uint parkid);

#define PCI_PARK_NVRAM    0xff
#define PCIE_ROM_ADDRESS_ENABLE     0x1

#endif /* _hndpci_h_ */
