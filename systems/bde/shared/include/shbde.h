/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef __SHBDE_H__
#define __SHBDE_H__

typedef void (*shbde_log_func_t)(int level, const char *str, int param);

#define SHBDE_ERR       0
#define SHBDE_WARN      1
#define SHBDE_DBG       2

/* Hardware abstraction functions */
typedef struct shbde_hal_s {

    /* Optional log output interface */
    shbde_log_func_t log_func;

    /* PCI configuration access */
    unsigned char (*pcic8_read)(void *pci_dev, unsigned int reg);
    void (*pcic8_write)(void *pci_dev, unsigned int reg, unsigned char data);
    unsigned short (*pcic16_read)(void *pci_dev, unsigned int reg);
    void (*pcic16_write)(void *pci_dev, unsigned int reg, unsigned short data);
    unsigned int (*pcic32_read)(void *pci_dev, unsigned int reg);
    void (*pcic32_write)(void *pci_dev, unsigned int reg, unsigned int data);

    /* iProc register access */
    unsigned int (*io32_read)(void *addr);
    void (*io32_write)(void *addr, unsigned int);

    /* usleep function (optional) */
    void (*usleep)(int usec);

    /* PCI parent device access */
    void *(*pci_parent_device_get)(void *pci_dev);

} shbde_hal_t;

/* iProc configuration (primarily used for PCI-AXI bridge) */
typedef struct shbde_iproc_config_s {
    unsigned int dev_id;
    unsigned int dev_rev;
    unsigned int use_msi;
    unsigned int iproc_ver;
    unsigned int dma_hi_bits;
    unsigned int mdio_base_addr;
    unsigned int pcie_phy_addr;
    unsigned int adjust_pcie_preemphasis;
} shbde_iproc_config_t;

#endif /* __SHBDE_H__ */
