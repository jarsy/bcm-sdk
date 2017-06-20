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

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/capability.h>
#include <linux/pid.h>
#include <linux/sort.h>
#include <linux/compat.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <asm/compat.h>
#include <asm/uaccess.h>
#include <linux/string.h>

#include "kbp_driver.h"

#define DRV_NAME "KBP_PCIE_DRIVER_"KBP_DRIVER_VERSION_MAJOR_STR"."KBP_DRIVER_VERSION_MINOR_STR

static int verbose = 1;
static int req_q_size = 1;
static int resp_q_size = 1;
static char *pcie_bus_mapping = NULL;

module_param(verbose, int, S_IRUGO);
module_param(req_q_size, int, S_IRUGO);
module_param(resp_q_size, int, S_IRUGO);
module_param(pcie_bus_mapping, charp, S_IRUGO | S_IWUSR);

#define KBP_INFO(f, args...) printk (KERN_INFO DRV_NAME " "f, ##args)
#define KBP_VERB(f, args...) do { if (verbose) printk (KERN_INFO DRV_NAME " "f, ##args); } while (0)

#define KBP_PCI_VENDOR_ID (0x14e4)
#define KBP_PCI_DEV_ID1 (0x9800)
#define KBP_PCI_DEV_ID0 (0x0F10)

#if defined(__powerpc__) || defined(__ppc__)
#define KBP_READ_PCIE_REG(device, reg, value)                           \
    do {                                                                \
        uint32_t __tmp = *((uint32_t *) (device->regmap_virt + (reg))); \
        value = __KBP_BYTESWAP_32(__tmp);                               \
    } while(0);


#define KBP_WRITE_PCIE_REG(device, reg, value)                          \
    do {                                                                \
       uint32_t __tmp = __KBP_BYTESWAP_32(value);                       \
       *((uint32_t *) (device->regmap_virt + (reg))) = (__tmp);         \
   } while(0);

#else

#define KBP_READ_PCIE_REG(device, reg, value) \
    ((value) = *((uint32_t *) (device->regmap_virt + (reg))))

#define KBP_WRITE_PCIE_REG(device, reg, value) \
    (*((uint32_t *) (device->regmap_virt + (reg))) = (value))
#endif

#define KBP_UPDATE_POINTERS(offset, paddr, rmem, asize)    \
    do {                                                   \
        if (rmem < asize)                                  \
            return -1;                                     \
        offset += asize;                                   \
        paddr += asize;                                    \
        rmem -= asize;                                     \
    } while (0);

enum kbp_device_type {
    KBP_DEVICE_FPGA = 0,
    KBP_DEVICE_PCIE
};

static const char *kbp_device_type_name[] = {
    "FPGA For ILA",
    "Direct PCIE connect to DUT"
};

struct kbp_device {
    enum kbp_device_type type;
    struct kbp_device *next;
    struct kbp_device *prev;
    pid_t owner_pid;
    struct task_struct *owner_task;
    struct proc_dir_entry *proc_entry;
    unsigned long long sysmem_virt;
    unsigned long long sysmem_base;
/* chunk of system memory for this device */
    uint8_t *regmap_virt;
    unsigned long long regmap_base;
    unsigned int req_base_offset;
    unsigned int resp_base_offset;
    unsigned int req_q_size;
    unsigned int resp_q_size;
    unsigned int req_q_head_offset;
    unsigned int resp_q_head_offset;
    unsigned int req_q_tail_offset;
    unsigned int resp_q_tail_offset;
    u32 regmap_size;
    u32 sysmem_size;
    u32 interrupt_enabled;
    u32 loopback_mode;
    u32 dma_enable;
    int32_t id;
    u32 signal_num;
    struct pci_dev *kbp_dev;
    spinlock_t lock;
    char name[256];
    char bus_name[256];
};

/*
 * A list of all the devices discovered on the system
 */

struct kbp_device *device_list_root;

/*
 * Free list of memory for rescan or hot plug
 */

struct kbp_device *device_free_list;

/*
 * The root directory /proc/kbp in the file system
 */

static struct proc_dir_entry *kbp_proc_root;

/*
 * To create unique entries
 */

static int32_t next_pcie_id = -1;
static int32_t next_fpga_id = 0;

/*
 * Keeps a mapping of the bus number to an optionally user assigned PCIE id for device
 */

struct kbp_enumeration_id
{
    struct kbp_enumeration_id *next;
    char bus_name[128]; /* The PCIE bus number in format 0000:01:00.0 */
    int32_t id;        /* Any integer ID number will be enumerated as /proc/kbp/pcie<id> */
};

/*
 * Sample table, customer to modify
 */

static struct kbp_enumeration_id *enumeration_ids;

/*
 * Static functions
 */

static int kbp_create_proc_entry(struct kbp_device *device);
static int kbp_memory_init(struct kbp_device *device);
static void kbp_free_device(struct kbp_device *device);
static int kbp_initialize_dma(struct kbp_device *dev);
static int dma_enable_disable(struct kbp_device *dev, unsigned int enable);
static int loopback_enable_disable(struct kbp_device *dev, unsigned int enable);
static int dma_clear_fifo(struct kbp_device *device);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
static irqreturn_t pdc_msi_interrupt(int irq, void *kbp_dev, struct pt_regs *regs)
#else
static irqreturn_t pdc_msi_interrupt(int irq, void *kbp_dev)
#endif
{
    int ret;
    struct kbp_device *device = NULL;
    for (device = device_list_root; device; device = device->next) {
        if ((void *) device->kbp_dev == kbp_dev) {
            break;
        }
    }

    if (device == NULL) {
        KBP_INFO(": Received MSI interrupt on device with no owner\n");
        return IRQ_HANDLED;
    } else {
        KBP_INFO(": Received MSI interrupt on device owned by %d\n",
                 device->owner_pid);
    }

    if (device->type == KBP_DEVICE_PCIE)
        ret = send_sig(KBP_PCIE_SIGNAL, device->owner_task, 0);
    else
        ret = send_sig(KBP_FPGA_SIGNAL, device->owner_task, 0);

    if (ret == 0) {
        KBP_INFO(": Delivered signal to %d\n", device->owner_pid);
    } else {
        KBP_INFO(": Failed to deliver signal to %d, err = %d\n",
                 device->owner_pid, ret);
    }
    return IRQ_HANDLED;
}

/*
 * mapping provided as
 *
 * bus,pcieid|bus,pcieid
 * <bus>,<id>|<bus>,<id>
 */

static int parse_pcie_mapping(char *pci_bus_mapping)
{
    char *next_tok;
    char *tmp, *mapping = kstrdup(pci_bus_mapping, GFP_KERNEL);
    struct kbp_enumeration_id *pci_map;

    tmp = mapping;
    if (!mapping)
        return -ENOMEM;

    while ((next_tok = strsep(&mapping, "|")) != NULL) {
        int i, found, len;

        if (!*next_tok)
            continue;

        len = strlen(next_tok);
        found = 0;
        for (i = 0; i < len; i++) {
            if (next_tok[i] == ',') {
                unsigned long id;
                int err;
                pci_map = kmalloc(sizeof(*pci_map), GFP_KERNEL);
                if (!pci_map)
                    return -ENOMEM;
                strncpy(pci_map->bus_name, next_tok, i);
                pci_map->bus_name[i] = '\0';
                err = kstrtol(&next_tok[i + 1], 10, &id);
                if (err < 0) {
                    KBP_INFO(": kstrtol of %s failed with %d\n",
                             &next_tok[i + 1], err);
                    return err;
                }
                pci_map->id = id;
                pci_map->next = enumeration_ids;
                enumeration_ids = pci_map;
                KBP_INFO(": Parsed mapping %s --> /proc/kbp/pcie%d\n",
                         pci_map->bus_name, pci_map->id);
                found = 1;
                break;
            }
        }
        if (!found) {
            KBP_INFO(": Error parsing string %s\n", next_tok);
            return -EFAULT;
        }
    }

    kfree(tmp);
    return 0;
}

static int32_t get_pcie_id(struct pci_dev *kbp_dev)
{
    int32_t id;
    struct kbp_enumeration_id *t;

    if (next_pcie_id == -1) {
        /* First time setup */
        int32_t largest_id = 0;
        for (t = enumeration_ids; t; t = t->next) {
            if (t->id >= largest_id)
                largest_id = t->id + 1;
        }

        next_pcie_id = largest_id;
    }

    for (t = enumeration_ids; t; t = t->next) {
        if (strcmp(t->bus_name, pci_name(kbp_dev)) == 0)
            return t->id;
    }

    id = next_pcie_id;
    next_pcie_id++;
    return id;
}


static struct kbp_device *kbp_alloc_device_handle(struct pci_dev *kbp_dev,
                                                  enum kbp_device_type type)
{
    struct kbp_device *tmp;
    unsigned long long regmap_base;
    u32 regmap_size;

    regmap_size = pci_resource_len(kbp_dev, 0);
    regmap_base = pci_resource_start(kbp_dev, 0);

    for (tmp = device_free_list; tmp; tmp = tmp->next) {
        if ((tmp->type == type)
            && (strcmp(tmp->bus_name, pci_name(kbp_dev)) == 0)) {
            /* Re-use device handle */
            uint32_t saved_id = tmp->id;
            if (tmp->next)
                tmp->next->prev = tmp->prev;
            if (tmp->prev)
                tmp->prev->next = tmp->next;
            else
                device_free_list = tmp->next;
            memset(tmp, 0, sizeof(*tmp));
            tmp->id = saved_id;
            break;
        }
    }

    if (!tmp) {
        uint32_t id;
        tmp = kmalloc(sizeof(*tmp), GFP_KERNEL);
        if (!tmp)
            return NULL;
        if (type == KBP_DEVICE_FPGA) {
            id = next_fpga_id;
            next_fpga_id++;
        } else {
            id = get_pcie_id(kbp_dev);
        }
        memset(tmp, 0, sizeof(*tmp));
        tmp->id = id;
    }

    tmp->kbp_dev = kbp_dev;
    strcpy(tmp->bus_name, pci_name(kbp_dev));
    tmp->type = type;
    tmp->regmap_size = regmap_size;
    tmp->regmap_base = regmap_base;
    spin_lock_init(&tmp->lock);
    tmp->next = device_list_root;
    if (tmp->next)
        tmp->next->prev = tmp;
    device_list_root = tmp;

    tmp->regmap_virt = (uint8_t*)ioremap_nocache(regmap_base, regmap_size);
    return tmp;
}

/* PCI SECTION */
static int kbp_pci_probe(struct pci_dev *kbp_dev, const struct pci_device_id *kbp_id)
{
    enum kbp_device_type type;
    struct kbp_device *device;
    int retval;

    if (kbp_dev == NULL) {
        KBP_INFO(": Probe called with null dev ptr\n");
        return -EINVAL;
    }

    /* Enable the device before any operations */
    if (pci_enable_device(kbp_dev) != 0) {
        KBP_INFO(": Cannot enable device (%p)\n", kbp_dev);
        return -EINVAL;
    }
    KBP_INFO(": Enabled device %s\n", pci_name(kbp_dev));
    switch (kbp_id->device) {
    case KBP_PCI_DEV_ID1:
        type = KBP_DEVICE_PCIE;
        break;
    case KBP_PCI_DEV_ID0:
        type = KBP_DEVICE_FPGA;
        break;
    default:
        KBP_INFO(": Unexpected device ID %d\n", kbp_id->device);
        return -EINVAL;
    }

    if (!pci_set_dma_mask(kbp_dev, DMA_BIT_MASK(64))) {
        pci_set_consistent_dma_mask(kbp_dev, DMA_BIT_MASK(64));
        KBP_INFO(": 64b DMA capable\n");
    } else if (!pci_set_dma_mask(kbp_dev, DMA_BIT_MASK(32))) {
        pci_set_consistent_dma_mask(kbp_dev, DMA_BIT_MASK(32));
        KBP_INFO(": 32b DMA capable\n");
    } else if (!pci_set_dma_mask(kbp_dev, DMA_BIT_MASK(24))) {
        pci_set_consistent_dma_mask(kbp_dev, DMA_BIT_MASK(24));
        KBP_INFO(": 24b DMA capable\n");
    } else {
        KBP_INFO(": Not suitable for DMA\n");
        return -EINVAL;
    }

    /* enable bus mastering */
    pci_set_master(kbp_dev);
    device = kbp_alloc_device_handle(kbp_dev, type);
    if (!device)
        return -ENOMEM;

    /* Create proc entry for the device */
    retval = kbp_create_proc_entry(device);
    if (retval) {
        kbp_free_device(device);
        return retval;
    }

    retval = kbp_memory_init(device);
    if (retval) {
        kbp_free_device(device);
        return retval;
    }

    retval = kbp_initialize_dma(device);
    if (retval) {
        kbp_free_device(device);
        return retval;
    }

    pci_set_drvdata(kbp_dev, device);
    KBP_INFO(": Created      : %s\n", device->name);
    KBP_INFO(":     config.register_map_base_phys = 0x%llx\n", device->regmap_base);
    KBP_INFO(":     config.register_map_size = %d\n", device->regmap_size);
    KBP_INFO(":     DMA Memory: 0x%016llx  |  size: 0x%08x\n",
             device->sysmem_base, device->sysmem_size);
    return 0;
}

static void kbp_pci_remove(struct pci_dev *kbp_dev)
{
    struct kbp_device *device;

    device = pci_get_drvdata(kbp_dev);
    if (device) {
        KBP_INFO(": Remove called on device %s\n", device->name);
    } else {
        KBP_INFO(": Remove called on unidentified device %s\n",
                 pci_name(kbp_dev));
        return;
    }

    kbp_free_device(device);
}

static const struct pci_device_id kbp_pci_ids[] = {
    {PCI_DEVICE(KBP_PCI_VENDOR_ID, KBP_PCI_DEV_ID0)},
    {PCI_DEVICE(KBP_PCI_VENDOR_ID, KBP_PCI_DEV_ID1)},
    {0,},
};

MODULE_DEVICE_TABLE(pci, kbp_pci_ids);

static struct pci_driver pci_driver_funcs = {
    .name = DRV_NAME "_pci",
    .id_table = kbp_pci_ids,
    .probe = kbp_pci_probe,
    .remove = kbp_pci_remove,
    .driver = {
        .name = DRV_NAME "_device",
    }
};

static int __init kbp_drv_module_init(void)
{
    int status;

    /* Print driver arguments */
    KBP_INFO(": Driver args: verbose=%d req_q_size=%d resp_q_size=%d pcie_bus_mapping=%s",
             verbose, req_q_size, resp_q_size, pcie_bus_mapping);

    /* Lets create the top level /proc/kbp directory */
    kbp_proc_root = proc_mkdir("kbp", NULL);
    if (kbp_proc_root == NULL) {
        KBP_INFO(": Creating top level /proc/kbp directory failed\n");
        return -EINVAL;
    }
    KBP_INFO(": =========================================\n");
    KBP_INFO(": Created /proc/kbp\n");
    if (pcie_bus_mapping) {
        KBP_INFO(": Parsing user provided PCIE bus mapping\n");
        status = parse_pcie_mapping(pcie_bus_mapping);
        if (status < 0)
            return status;
    }
    KBP_INFO(": driver load process started.\n");
    status = pci_register_driver(&pci_driver_funcs);
    if (status < 0) {
        KBP_INFO(": Unable to register PCI driver! (%d)\n", status);
        return -EINVAL;
    }
    KBP_INFO(": =========================================\n");
    return 0;
}

static void kbp_free_device(struct kbp_device *device)
{
    KBP_INFO(": Removing device %s\n", device->name);
    if (device->next)
        device->next->prev = device->prev;
    if (device->prev)
        device->prev->next = device->next;
    else
        device_list_root = device->next;

    device->next = NULL;
    device->prev = NULL;
    if (device->proc_entry) {
        remove_proc_entry(device->name, kbp_proc_root);
        device->proc_entry = NULL;
    }

    dma_enable_disable(device, 0);
    loopback_enable_disable(device, 0);
    dma_clear_fifo(device);

    if (device->sysmem_virt) {
        free_pages(device->sysmem_virt, get_order(device->sysmem_size));
        device->sysmem_virt = 0ULL;
    }

    device->next = device_free_list;
    if (device_free_list)
        device_free_list->prev = device;
    device_free_list = device;
}

static void __exit kbp_drv_module_exit(void)
{
    struct kbp_device *tmp;
    struct kbp_enumeration_id *t;

    pci_unregister_driver(&pci_driver_funcs);

    tmp = device_list_root;
    while (tmp) {
        kbp_free_device(tmp);
        tmp = device_list_root;
    }

    tmp = device_free_list;
    while (tmp) {
        struct kbp_device *next = tmp->next;
        kfree(tmp);
        tmp = next;
    }
    remove_proc_entry("kbp", NULL);

    t = enumeration_ids;
    while (t) {
        struct kbp_enumeration_id *next = t->next;
        kfree(t);
        t = next;
    }

    KBP_INFO(": Unloaded.\n");
    KBP_INFO(": =========================================\n");
}

static void kbp_driver_log_reg(struct seq_file *m, struct kbp_device *device,
                                   uint32_t address, char *name)
{
    uint32_t regval = 0;

    KBP_READ_PCIE_REG(device, address, regval);
    seq_printf(m, " %s (%08x) = %08x\n",
               name, address, regval);
}

/* /proc handling section */
static int kbp_device_info_show(struct seq_file *m, void *v)
{
    struct kbp_device *dev;
    pid_t owner;
    dev = (struct kbp_device *)m->private;

    if (dev == NULL || m == NULL)
        return -EINVAL;

    /* We are looking at the owner field, without any form of
       lock. There is a possibility that the owner may be inlocking
       the device as we are doing this. However, this routine is just
       an instant snapshot, and we do not de-reference any memory that
       is changing, so the worst case is data is stale */
    seq_printf(m, "Device Type    : %s\n", kbp_device_type_name[dev->type]);
    seq_printf(m, "Driver Version : %s\n", DRV_NAME);
    owner = dev->owner_pid;
    if (owner == 0)
        seq_printf(m, "Status         : Available\n");
    else
        seq_printf(m, "Status         : Busy, locked by PID %d\n", owner);
    seq_printf(m, "PCI Device     : %s\n", dev->bus_name);
    seq_printf(m, "       config.register_map_base_phys = 0x%llx\n", dev->regmap_base);
    seq_printf(m, "       config.register_map_size = %d\n", dev->regmap_size);
    seq_printf(m, "ioctl the /proc file from your process to obtain the virtual\n");
    seq_printf(m, "address of the register map\n");
    seq_printf(m, "=======================================================\n");
    seq_printf(m, "Driver Assigned DMA memory\n");
    seq_printf(m, "=======================================================\n");
    seq_printf(m, "DMA Memory: 0x%016llx  |  size: 0x%08x\n", dev->sysmem_base, dev->sysmem_size);
    seq_printf(m, "DMA REQUEST Q OFFSETS: base: 0x%x, size: %d, head: 0x%x, tail: 0x%x\n",
               dev->req_base_offset, dev->req_q_size, dev->req_q_head_offset, dev->req_q_tail_offset);
    seq_printf(m, "DMA RESPONSE Q OFFSETS: base: 0x%x, size: %d, head: 0x%x, tail: 0x%x\n",
               dev->resp_base_offset, dev->resp_q_size, dev->resp_q_head_offset, dev->resp_q_tail_offset);
    if (dev->type == KBP_DEVICE_PCIE) {
        seq_printf(m, "========== DMA Register dump start ====================\n");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_DMA_CONTROL, "icf_pdc_registers_DMA_CONTROL");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_TEST_CAPABILITIES, "icf_pdc_registers_TEST_CAPABILITIES");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_ReqBuf_Head, "icf_pdc_registers_REQBUF_HEAD");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_RspBuf_Head, "icf_pdc_registers_RSPBUF_HEAD");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_NONDMA_STATUS, "icf_pdc_registers_NONDMA_STATUS");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_EFUSE_1B_CORR_STS, "icf_pdc_registers_EFUSE_1B_CORR_STS");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_EFUSE_2B_ERR_STS, "icf_pdc_registers_EFUSE_2B_ERR_STS");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_PDC_DEBUG_STS, "icf_pdc_registers_PDC_DEBUG_STS");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_PCIE_IP_CTRL_LO, "icf_pdc_registers_PCIE_IP_CTRL_LO");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_PCIE_IP_CTRL_HI, "icf_pdc_registers_PCIE_IP_CTRL_HI");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_REQ_Q_LBASE, "icf_pdc_registers_REQ_Q_LBASE");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_REQ_Q_HBASE, "icf_pdc_registers_REQ_Q_HBASE");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_REQ_H_LBASE, "icf_pdc_registers_REQ_H_LBASE");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_REQ_H_HBASE, "icf_pdc_registers_REQ_H_HBASE");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_REQ_T_LBASE, "icf_pdc_registers_REQ_T_LBASE");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_REQ_T_HBASE, "icf_pdc_registers_REQ_T_HBASE");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_REQ_Q_TIMER, "icf_pdc_registers_REQ_Q_TIMER");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_REQ_Q_CTRL, "icf_pdc_registers_REQ_Q_CTRL");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_RSP_Q_LBASE, "icf_pdc_registers_RSP_Q_LBASE");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_RSP_Q_HBASE, "icf_pdc_registers_RSP_Q_HBASE");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_RSP_H_LBASE, "icf_pdc_registers_RSP_H_LBASE");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_RSP_H_HBASE, "icf_pdc_registers_RSP_H_HBASE");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_RSP_T_LBASE, "icf_pdc_registers_RSP_T_LBASE");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_RSP_T_HBASE, "icf_pdc_registers_RSP_T_HBASE");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_RSP_Q_TIMER, "icf_pdc_registers_RSP_Q_TIMER");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_RSP_Q_CTRL, "icf_pdc_registers_RSP_Q_CTRL");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_SAT_M_TIMER_LO, "icf_pdc_registers_SAT_M_TIMER_LO");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_SAT_M_TIMER_HI, "icf_pdc_registers_SAT_M_TIMER_HI");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_INTR_ENABLE, "icf_pdc_registers_INTR_ENABLE");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_INTR_CLEAR, "icf_pdc_registers_INTR_CLEAR");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_PDC_INTR, "icf_pdc_registers_PDC_INTR");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_PDC_REQBUF_TAIL_OVRD, "icf_pdc_registers_PDC_REQBUF_TAIL_OVRD");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_PDC_ERROR_STS_0, "icf_pdc_registers_PDC_ERROR_STS_0");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_PDC_ERROR_STS_1, "icf_pdc_registers_PDC_ERROR_STS_1");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_PDC_ERROR_STS_2, "icf_pdc_registers_PDC_ERROR_STS_2");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_PDC_ERROR_STS_3, "icf_pdc_registers_PDC_ERROR_STS_3");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_PDC_ERROR_STS_4, "icf_pdc_registers_PDC_ERROR_STS_4");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_PDC_ERROR_STS_5, "icf_pdc_registers_PDC_ERROR_STS_5");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_PDC_ERROR_STS_6, "icf_pdc_registers_PDC_ERROR_STS_6");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_PDC_ALERT_STS_0, "icf_pdc_registers_PDC_ALERT_STS_0");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_PDC_ALERT_STS_1, "icf_pdc_registers_PDC_ALERT_STS_1");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_PDC_ALERT_STS_2, "icf_pdc_registers_PDC_ALERT_STS_2");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_PCIE_TEST_BUS_0, "icf_pdc_registers_PCIE_TEST_BUS_0");
        kbp_driver_log_reg(m, dev, icf_pdc_registers_PCIE_TEST_BUS_1, "icf_pdc_registers_PCIE_TEST_BUS_1");
    }
    seq_printf(m, "\n");

    return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
#define PDE_DATA(x) (PDE((x))->data)
#endif

static int kbp_device_open(struct inode *inode, struct file *file)
{
    struct kbp_device *dev = PDE_DATA(inode);

    if (dev == NULL)
        return -EINVAL;

    if (file->f_mode & FMODE_WRITE) {
        /* Only when writing to the device (ioctl) set the
           current process as the owner. This will create
           exclusive access to the device for the user process.

           For cat on the /proc device, just let them happen */
        unsigned long flags;
        /*KBP_INFO (":open The file reference count is %ld\n", file->f_count.counter); */
        spin_lock_irqsave(&dev->lock, flags);
        if ((dev->owner_pid == 0) && (!(file->f_flags & O_NONBLOCK))) {
            dev->owner_pid = current->pid;
            dev->owner_task = current;
        } else if (dev->owner_pid == current->pid) {
            /* recursive */
        } else if (!(file->f_flags & O_NONBLOCK)){
            spin_unlock_irqrestore(&dev->lock, flags);
            return -EBUSY;
        }
        file->private_data = dev;
        spin_unlock_irqrestore(&dev->lock, flags);

        /* Note! we cannot use single open, when the
           file has been opened with write, as we
           use the private_data field in the file
           descriptor field for ioctl reference.

           If you single_open or seq_open you will
           die! */
        return 0;
    }

    return single_open(file, kbp_device_info_show, dev);
}

static long setup_interrupts(struct kbp_device *dev, unsigned int signal_num)
{
    int ret;

    if (dev->interrupt_enabled)
        return 0;

    ret = pci_enable_msi(dev->kbp_dev);
    if (ret != 0) {
        KBP_INFO(": enable MSI failed with error %d\n", ret);
        return ret;
    }

    KBP_INFO(": enable MSI and IRQ received %d\n", dev->kbp_dev->irq);
    ret = request_irq(dev->kbp_dev->irq, pdc_msi_interrupt,
                      IRQF_SHARED, "kbp_device", dev->kbp_dev);
    if (ret != 0) {
        pci_disable_msi(dev->kbp_dev);
        KBP_INFO(": cannot register IRQ %d\n", dev->kbp_dev->irq);
        return ret;
    }

    dev->interrupt_enabled = 1;
    dev->signal_num = signal_num;
    KBP_INFO(": Registered MSI interrupt handler with signal %d\n", signal_num);
    return 0;
}

static long disable_interrupts(struct kbp_device *dev)
{
    if (dev->interrupt_enabled == 0)
        return 0;

    free_irq(dev->kbp_dev->irq, dev->kbp_dev);
    pci_disable_msi(dev->kbp_dev);
    dev->interrupt_enabled = 0;
    dev->signal_num = 0;
    KBP_INFO(": Disabled interrupts\n");
    return 0;
}

/* In the ioctl routine we will already have exclusive access
   to the proc file. Multiple writers are blocked out in
   the file open call */
static long kbp_device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct kbp_device *dev = file->private_data;
    struct kbp_sys_cfg *buf = (struct kbp_sys_cfg *) arg;
    unsigned int signal, *signal_arg = (unsigned int *) arg;
    unsigned int enable, *enable_arg = (unsigned int *) arg;

    if (dev == NULL) {
        KBP_VERB(": ioctl called by pid %d failed, device is null\n", current->pid);
        return -EINVAL;
    }

    /*KBP_INFO (":ioctl The file reference count is %ld\n", file->f_count.counter); */

    switch (_IOC_TYPE(cmd)) {
    case KBP_IOCTL_GET_REG_MEM:
        if (put_user(dev->regmap_size, &buf->reg_mem_size))
            return -EFAULT;
        if (put_user(dev->regmap_base, &buf->reg_physical_addr))
            return -EFAULT;
        KBP_VERB(": default ioctl regmem for pid %d, addr 0x%llx, size 0x%x\n",
                 current->pid, dev->regmap_base, dev->regmap_size);
        break;
    case KBP_IOCTL_DMA_SETUP:
        if (put_user(dev->sysmem_base, &buf->sys_physical_addr))
            return -EFAULT;
        if (put_user(dev->sysmem_size, &buf->sys_mem_size))
            return -EFAULT;
        if (put_user(dev->req_base_offset, &buf->req_base_offset))
            return -EFAULT;
        if (put_user(dev->resp_base_offset, &buf->resp_base_offset))
            return -EFAULT;
        if (put_user(dev->req_q_size, &buf->req_size))
            return -EFAULT;
        if (put_user(dev->resp_q_size, &buf->resp_size))
            return -EFAULT;
        if (put_user(dev->req_q_head_offset, &buf->req_head_offset))
            return -EFAULT;
        if (put_user(dev->resp_q_head_offset, &buf->resp_head_offset))
            return -EFAULT;
        if (put_user(dev->req_q_tail_offset, &buf->req_tail_offset))
            return -EFAULT;
        if (put_user(dev->resp_q_tail_offset,&buf->resp_tail_offset))
            return -EFAULT;
        KBP_VERB(": default ioctl DMA memory for pid %d, addr 0x%llx, size 0x%x\n",
                 current->pid, dev->sysmem_base, dev->sysmem_size);
        KBP_VERB(": default ioctl req_base_offset req_q_size for pid %d, addr 0x%x, size 0x%x\n",
                 current->pid, dev->req_base_offset, dev->req_q_size);
        KBP_VERB(": default ioctl resp_base_offset resp_q_size for pid %d, addr 0x%x, size 0x%x\n",
                 current->pid, dev->resp_base_offset, dev->resp_q_size);
        KBP_VERB(": default ioctl req_q_head_offset req_q_tail_offset for pid %d, addr 0x%x, size 0x%x\n",
                 current->pid, dev->req_q_head_offset, dev->req_q_tail_offset);
        KBP_VERB(": default ioctl req_q_head_offset req_q_tail_offset for pid %d, addr 0x%x, size 0x%x\n",
                 current->pid, dev->resp_q_head_offset, dev->resp_q_tail_offset);
        break;
    case KBP_IOCTL_INTERRUPT:
        if (get_user(signal, signal_arg))
            return -EFAULT;
        return setup_interrupts(dev, signal);
    case KBP_IOCTL_DMA_CTRL:
        if (get_user(enable, enable_arg))
            return -EFAULT;
        if (dma_enable_disable(dev, enable))
            return -EFAULT;
        break;
    case KBP_IOCTL_LOOPBACK_MODE:
        if (get_user(enable,enable_arg))
            return -EFAULT;
        if(loopback_enable_disable(dev, enable))
            return -EFAULT;
        break;
    case KBP_IOCTL_VERSION:
        if (put_user(KBP_DRIVER_VERSION_MAJOR, &buf->major))
            return -EFAULT;
        if (put_user(KBP_DRIVER_VERSION_MINOR,&buf->minor))
            return -EFAULT;
        break;
    default:
        KBP_VERB(": default ioctl code incorrect on %s by pid %d, ioctl=%d\n",
                 dev->name, current->pid, _IOC_TYPE(cmd));
        return -EFAULT;
    }

    return 0;
}

#ifdef CONFIG_COMPAT
static long kbp_device_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
#ifdef __x86_64
    /* Padding & structure alignment difference */
    struct kbp_sys_cfg_compat
    {
        unsigned int reg_mem_size;
        unsigned int sys_mem_size;
        unsigned long long __attribute__((aligned(4))) reg_physical_addr;
        unsigned long long __attribute__((aligned(4))) sys_physical_addr;
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

    struct kbp_device *dev = file->private_data;
    struct kbp_sys_cfg_compat *buf = (struct kbp_sys_cfg_compat *) arg;
    unsigned int signal, *signal_arg = (unsigned int *) arg;
    unsigned int enable, *enable_arg = (unsigned int *) arg;

    if (dev == NULL) {
        KBP_VERB(": ioctl called by pid %d failed, device is null\n", current->pid);
        return -EINVAL;
    }

    /*KBP_INFO (":ioctl The file reference count is %ld\n", file->f_count.counter); */

    switch (_IOC_TYPE(cmd)) {
    case KBP_IOCTL_GET_REG_MEM:
        if (put_user(dev->regmap_size, &buf->reg_mem_size))
            return -EFAULT;
        if (put_user(dev->regmap_base, &buf->reg_physical_addr))
            return -EFAULT;
        KBP_VERB(": compat ioctl regmem for pid %d, addr 0x%llx, size 0x%x\n",
                 current->pid, dev->regmap_base, dev->regmap_size);
        break;
    case KBP_IOCTL_DMA_SETUP:
        if (put_user(dev->sysmem_base, &buf->sys_physical_addr))
            return -EFAULT;
        if (put_user(dev->sysmem_size, &buf->sys_mem_size))
            return -EFAULT;
        if (put_user(dev->req_base_offset, &buf->req_base_offset))
            return -EFAULT;
        if (put_user(dev->resp_base_offset, &buf->resp_base_offset))
            return -EFAULT;
        if (put_user(dev->req_q_size, &buf->req_size))
            return -EFAULT;
        if (put_user(dev->resp_q_size, &buf->resp_size))
            return -EFAULT;
        if (put_user(dev->req_q_head_offset, &buf->req_head_offset))
            return -EFAULT;
        if (put_user(dev->resp_q_head_offset, &buf->resp_head_offset))
            return -EFAULT;
        if (put_user(dev->req_q_tail_offset, &buf->req_tail_offset))
            return -EFAULT;
        if (put_user(dev->resp_q_tail_offset,&buf->resp_tail_offset))
            return -EFAULT;
        KBP_VERB(": compat ioctl DMA memory for pid %d, addr 0x%llx, size 0x%x\n",
                 current->pid, dev->sysmem_base, dev->sysmem_size);
        KBP_VERB(": compat ioctl req_base_offset req_q_size for pid %d, addr 0x%x, size 0x%x\n",
                 current->pid, dev->req_base_offset, dev->req_q_size);
        KBP_VERB(": compat ioctl resp_base_offset resp_q_size for pid %d, addr 0x%x, size 0x%x\n",
                 current->pid, dev->resp_base_offset, dev->resp_q_size);
        KBP_VERB(": compat ioctl req_q_head_offset req_q_tail_offset for pid %d, addr 0x%x, size 0x%x\n",
                 current->pid, dev->req_q_head_offset, dev->req_q_tail_offset);
        KBP_VERB(": compat ioctl req_q_head_offset req_q_tail_offset for pid %d, addr 0x%x, size 0x%x\n",
                 current->pid, dev->resp_q_head_offset, dev->resp_q_tail_offset);
        break;
    case KBP_IOCTL_INTERRUPT:
        if (get_user(signal, signal_arg))
            return -EFAULT;
        return setup_interrupts(dev, signal);
    case KBP_IOCTL_DMA_CTRL:
        if (get_user(enable, enable_arg))
            return -EFAULT;
        if (dma_enable_disable(dev, enable))
            return -EFAULT;
        break;
    case KBP_IOCTL_LOOPBACK_MODE:
        if(get_user(enable,enable_arg))
            return -EFAULT;
        if (loopback_enable_disable(dev, enable))
           return -EFAULT;
        break;
    case KBP_IOCTL_VERSION:
        if (put_user(KBP_DRIVER_VERSION_MAJOR, &buf->major))
            return -EFAULT;
        if (put_user(KBP_DRIVER_VERSION_MINOR,&buf->minor))
            return -EFAULT;
        break;
    default:
        KBP_VERB(": compat ioctl code incorrect on %s by pid %d, ioctl=%d\n",
                 dev->name, current->pid, _IOC_TYPE(cmd));
        return -EFAULT;
    }

    return 0;
#else
    return kbp_device_ioctl(file, cmd, arg);
#endif
}
#endif

#ifndef VM_RESERVED
#define  VM_RESERVED   (VM_DONTEXPAND | VM_DONTDUMP)
#endif

/* Remap the physical address returned to the user through
   ioctl call above to a user visible virtual address */
static int kbp_device_mmap(struct file *file_p, struct vm_area_struct *vm_p)
{
    int cacheable;
    ulong phy_addr;

    if (vm_p->vm_flags & VM_LOCKED)
        return -EPERM;

    /*KBP_INFO (":mmap The file reference count is %ld\n", file_p->f_count.counter); */

    vm_p->vm_flags |= VM_RESERVED;
    vm_p->vm_flags |= VM_IO;
    cacheable = 1;

    phy_addr = vm_p->vm_pgoff << PAGE_SHIFT;

#if defined (__arm__)
    /* On ARM all system memory is non cacheable */
    vm_p->vm_page_prot = pgprot_noncached(vm_p->vm_page_prot);
    cacheable = 0;
#else
    {
        struct kbp_device *device;
        for (device = device_list_root; device; device = device->next) {
            if (device->regmap_base == phy_addr) {
                vm_p->vm_page_prot = pgprot_noncached(vm_p->vm_page_prot);
                cacheable = 0;
                break;
            }
        }
    }
#endif

    KBP_VERB(": mmap called start 0x%lx, end 0x%lx, vm_pg_off 0x%lx, phy_addr 0x%lx, cacheable %d\n",
             vm_p->vm_start, vm_p->vm_end, vm_p->vm_pgoff, phy_addr, cacheable);

    if (remap_pfn_range(vm_p, vm_p->vm_start, vm_p->vm_pgoff,
                        vm_p->vm_end - vm_p->vm_start, vm_p->vm_page_prot)) {
        KBP_INFO(": remap_pfn_range failed.\n");
        return -EFAULT;
    }

    return 0;
}

static int kbp_device_close(struct inode *inode, struct file *file)
{
    struct kbp_device *dev = PDE_DATA(inode);

    if (dev == NULL)
        return -EINVAL;

    /*KBP_INFO (":close The file reference count is %ld\n", file->f_count.counter); */

    if (file->f_mode & FMODE_WRITE) {
        unsigned long flags;
        spin_lock_irqsave(&dev->lock, flags);
        dev->owner_pid = 0;
        dev->owner_task = NULL;
        spin_unlock_irqrestore(&dev->lock, flags);
        if(!dev->dma_enable) {
            if (dma_enable_disable(dev, 1))
                return -EFAULT;
        }
        if(dev->loopback_mode) {
            if (loopback_enable_disable(dev, 0))
                return -EFAULT;
        }
        return disable_interrupts(dev);
    }

    return single_release(inode, file);
}


/* /proc/kbp/ * file operations */
static const struct file_operations device_proc_fops = {
    .owner = THIS_MODULE,
    .open = kbp_device_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = kbp_device_close,
    .unlocked_ioctl = kbp_device_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = kbp_device_compat_ioctl,
#endif
    .mmap = kbp_device_mmap
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
struct proc_dir_entry *proc_create_data(const char *name, umode_t mode, struct proc_dir_entry *parent,
                                        const struct file_operations *proc_fops, void *data)
{
    struct proc_dir_entry *entry;
    entry = proc_create(name, S_IFREG | S_IRUGO | S_IWUGO, kbp_proc_root,
                        &device_proc_fops);
    if (entry == NULL) {
         return entry;
    }
    entry->data = data;
    return entry;
}
#endif

static int kbp_create_proc_entry(struct kbp_device *device)
{
    struct proc_dir_entry *entry;

    switch (device->type) {
    case KBP_DEVICE_FPGA:
        sprintf(device->name, "fpga%d", device->id);
        break;
    case KBP_DEVICE_PCIE:
        sprintf(device->name, "pcie%d", device->id);
        break;
    default:
        KBP_INFO(": Unexpected device ID during /proc creation\n");
        return -EINVAL;
    }

    entry = proc_create_data(device->name, S_IFREG | S_IRUGO | S_IWUGO,
                             kbp_proc_root, &device_proc_fops,
                             device);
    if (entry == NULL) {
        return -ENOMEM;
    }

    device->proc_entry = entry;
    return 0;
}

/* Padding used between pointers for DMA */
#define KBP_DEFAULT_PAD_SIZE 64

static int kbp_memory_init(struct kbp_device *device)
{
    ulong phys_addr;
    size_t alloc_size;
    unsigned long long alloc_memory;
    uint32_t tx_size_x = (1 << (resp_q_size - 1));
    uint32_t rx_size_x = (1 << (req_q_size - 1));
    alloc_size = ((tx_size_x + rx_size_x) * 1024 * sizeof(uint64_t))
        + 6 * KBP_DEFAULT_PAD_SIZE;

    alloc_memory = __get_dma_pages(__GFP_NOWARN | GFP_USER,
                                   get_order(alloc_size));
    if (!alloc_memory) {
        KBP_INFO(": Failed allocation for device %s\n", device->name);
        return -ENOMEM;
    }
    phys_addr = virt_to_phys((void *) alloc_memory);
    device->sysmem_virt = alloc_memory;
    device->sysmem_base = phys_addr;
    device->sysmem_size = alloc_size;
    return 0;
}

static int dma_enable_disable(struct kbp_device *device, unsigned int enable)
{
    unsigned int regval;

    if (device->dma_enable == enable)
        return 0;

    KBP_INFO(": DMA set to %d for %s\n", enable, device->name);

    KBP_READ_PCIE_REG(device, icf_pdc_registers_DMA_CONTROL, regval);
    SET_FIELD(regval, icf_pdc_registers, DMA_CONTROL, tx_dma_enable, enable);
    SET_FIELD(regval, icf_pdc_registers, DMA_CONTROL, rx_dma_enable, enable);
    KBP_WRITE_PCIE_REG(device, icf_pdc_registers_DMA_CONTROL, regval);

    KBP_READ_PCIE_REG(device, icf_pdc_registers_DMA_CONTROL, regval);
    KBP_INFO(": DMA_CONTROL = %x for %s\n", regval, device->name);
    device->dma_enable = enable;

    return 0;
}

static int loopback_enable_disable(struct kbp_device *device, unsigned int enable)
{
    unsigned int regval;

    if (device->loopback_mode == enable)
        return 0;

    KBP_INFO(": Loopback set to %d for %s\n", enable, device->name);

    KBP_READ_PCIE_REG(device, icf_pdc_registers_TEST_CAPABILITIES, regval);
    SET_FIELD(regval, icf_pdc_registers, TEST_CAPABILITIES, reqf_rspf_loopback, enable);
    KBP_WRITE_PCIE_REG(device, icf_pdc_registers_TEST_CAPABILITIES, regval);

    KBP_READ_PCIE_REG(device, icf_pdc_registers_TEST_CAPABILITIES, regval);
    KBP_INFO(": TEST_CAPABILITIES = %x for %s\n", regval, device->name);
    device->loopback_mode = enable;

    return 0;
}

static int dma_clear_fifo(struct kbp_device *device)
{
    unsigned int regval;
    KBP_INFO(": DMA FIFO's cleared for %s\n", device->name);
    KBP_READ_PCIE_REG(device, icf_pdc_registers_DMA_CONTROL, regval);
    SET_FIELD(regval, icf_pdc_registers, DMA_CONTROL, tx_fifo_clear_status, 1);
    SET_FIELD(regval, icf_pdc_registers, DMA_CONTROL, rx_fifo_clear_status, 1);
    KBP_WRITE_PCIE_REG(device, icf_pdc_registers_DMA_CONTROL, regval);

    KBP_READ_PCIE_REG(device, icf_pdc_registers_DMA_CONTROL, regval);
    KBP_INFO(": DMA_CONTROL = %x for %s\n", regval, device->name);
    return 0;
}

static int kbp_initialize_dma(struct kbp_device *device)
{
    uint32_t rem_mem;
    uint64_t phy_addr;
    uint32_t regval, alloc_size;
    int32_t offset = 0;
    uint32_t tx_size_x = 1, rx_size_x = 1;

    KBP_INFO(": Initializing the DMA sequence for %s\n", device->name);
    tx_size_x = (1 << (resp_q_size - 1));
    rx_size_x = (1 << (req_q_size - 1));

    memset((void *)device->sysmem_virt, 0, device->sysmem_size);

    KBP_READ_PCIE_REG(device, icf_pdc_registers_DMA_CONTROL, regval);
    SET_FIELD(regval, icf_pdc_registers, DMA_CONTROL, pram_dma_mode, 0);
    KBP_WRITE_PCIE_REG(device, icf_pdc_registers_DMA_CONTROL, regval);

    /*
     * Disable the DMA and clear the FIFO's
     */

    dma_enable_disable(device, 0);
    dma_clear_fifo(device);

    /* Clear interrupts */
    KBP_WRITE_PCIE_REG(device, icf_pdc_registers_INTR_CLEAR, 0xFFFFFFFF);

    /*
     * Lets set up the circular buffers
     */
    rem_mem = device->sysmem_size;
    phy_addr = device->sysmem_base;

    device->req_base_offset = offset;
    device->req_q_size = rx_size_x * 1024;

    regval = 0;
    SET_FIELD(regval, icf_pdc_registers, REQ_Q_LBASE,
              address, phy_addr & 0xFFFFFFFF);
    KBP_WRITE_PCIE_REG(device, icf_pdc_registers_REQ_Q_LBASE, regval);

    regval = 0;
    SET_FIELD(regval, icf_pdc_registers, REQ_Q_HBASE,
              address, (phy_addr >> 32) & 0xFFFFFFFF);
    KBP_WRITE_PCIE_REG(device, icf_pdc_registers_REQ_Q_HBASE, regval);

    alloc_size = rx_size_x * 1024 * sizeof(uint64_t);
    KBP_UPDATE_POINTERS(offset, phy_addr, rem_mem, alloc_size);

    device->resp_base_offset = offset;
    device->resp_q_size = tx_size_x * 1024;

    regval = 0;
    SET_FIELD(regval, icf_pdc_registers, RSP_Q_LBASE,
              address, phy_addr & 0xFFFFFFFF);
    KBP_WRITE_PCIE_REG(device, icf_pdc_registers_RSP_Q_LBASE, regval);

    regval = 0;
    SET_FIELD(regval, icf_pdc_registers, RSP_Q_HBASE,
              address, (phy_addr >> 32) & 0xFFFFFFFF);
    KBP_WRITE_PCIE_REG(device, icf_pdc_registers_RSP_Q_HBASE, regval);

    alloc_size = tx_size_x * 1024 * sizeof(uint64_t);
    KBP_UPDATE_POINTERS(offset, phy_addr, rem_mem, alloc_size);

    /*
     * Leave a hole before we allocate head/tail pointer.
     * Not really necessary
     */

    KBP_UPDATE_POINTERS(offset, phy_addr, rem_mem, KBP_DEFAULT_PAD_SIZE);

    /*
     * For the request queue
     */

    /* Head pointer */
    regval = 0;
    SET_FIELD(regval, icf_pdc_registers, REQ_H_LBASE,
              address, phy_addr & 0xFFFFFFFF);
    KBP_WRITE_PCIE_REG(device, icf_pdc_registers_REQ_H_LBASE, regval);

    regval = 0;
    SET_FIELD(regval, icf_pdc_registers, REQ_H_HBASE,
              address, (phy_addr >> 32) & 0xFFFFFFFF);
    KBP_WRITE_PCIE_REG(device, icf_pdc_registers_REQ_H_HBASE, regval);

    device->req_q_head_offset =  offset;

    /*
     * Leave a hole, includes the size of pointer above
     */

    KBP_UPDATE_POINTERS(offset, phy_addr, rem_mem, KBP_DEFAULT_PAD_SIZE);

    /* Tail pointer */
    regval = 0;
    SET_FIELD(regval, icf_pdc_registers, REQ_T_LBASE,
              address, phy_addr & 0xFFFFFFFF);
    KBP_WRITE_PCIE_REG(device, icf_pdc_registers_REQ_T_LBASE, regval);

    regval = 0;
    SET_FIELD(regval, icf_pdc_registers, REQ_T_HBASE,
              address, (phy_addr >> 32) & 0xFFFFFFFF);
    KBP_WRITE_PCIE_REG(device, icf_pdc_registers_REQ_T_HBASE, regval);

    device->req_q_tail_offset = offset;

    /*
     * Leave a hole, includes the size of pointer above
     */

    KBP_UPDATE_POINTERS(offset, phy_addr, rem_mem, KBP_DEFAULT_PAD_SIZE);

    /*
     * For the response queue
     */

    /* Head pointer */
    regval = 0;
    SET_FIELD(regval, icf_pdc_registers, RSP_H_LBASE,
              address, phy_addr & 0xFFFFFFFF);
    KBP_WRITE_PCIE_REG(device, icf_pdc_registers_RSP_H_LBASE, regval);

    regval = 0;
    SET_FIELD(regval, icf_pdc_registers, RSP_H_HBASE,
              address, (phy_addr >> 32) & 0xFFFFFFFF);
    KBP_WRITE_PCIE_REG(device, icf_pdc_registers_RSP_H_HBASE, regval);

    device->resp_q_head_offset = offset;

    /*
     * Leave a hole
     */

    KBP_UPDATE_POINTERS(offset, phy_addr, rem_mem, KBP_DEFAULT_PAD_SIZE);

    /* Tail pointer */
    regval = 0;
    SET_FIELD(regval, icf_pdc_registers, RSP_T_LBASE,
              address, phy_addr & 0xFFFFFFFF);
    KBP_WRITE_PCIE_REG(device, icf_pdc_registers_RSP_T_LBASE, regval);

    regval = 0;
    SET_FIELD(regval, icf_pdc_registers, RSP_T_HBASE,
              address, (phy_addr >> 32) & 0xFFFFFFFF);
    KBP_WRITE_PCIE_REG(device, icf_pdc_registers_RSP_T_HBASE, regval);

    device->resp_q_tail_offset = offset;

    /*
     * Leave a hole
     */
    KBP_UPDATE_POINTERS(offset, phy_addr, rem_mem, KBP_DEFAULT_PAD_SIZE);

    /*
     * Independent clear the tx_fifo)clear and rx_fifo clear as this
     * re-sets the DBA control fifo sizes also
     */
    dma_clear_fifo(device);

    KBP_READ_PCIE_REG(device, icf_pdc_registers_TEST_CAPABILITIES, regval);
    SET_FIELD(regval, icf_pdc_registers, TEST_CAPABILITIES, multi_dma_req_en, 1);
    KBP_WRITE_PCIE_REG(device, icf_pdc_registers_TEST_CAPABILITIES, regval);

    KBP_READ_PCIE_REG(device, icf_pdc_registers_TEST_CAPABILITIES, regval);
    SET_FIELD(regval, icf_pdc_registers, TEST_CAPABILITIES, rspbuf_tail_ptr_chk, 1);
    KBP_WRITE_PCIE_REG(device, icf_pdc_registers_TEST_CAPABILITIES, regval);


    /* set the resp and req buffer size and enable DMA */
    KBP_READ_PCIE_REG(device, icf_pdc_registers_DMA_CONTROL, regval);
    /* 15:12 2^x K  x=0, 1, ... 0xC */
    SET_FIELD(regval, icf_pdc_registers, DMA_CONTROL, txdma_buffer_size, (resp_q_size - 1));
    /* 11:18 2^x K  x=0, 1, ... 0xC */
    SET_FIELD(regval, icf_pdc_registers, DMA_CONTROL, rxdma_buffer_size, (req_q_size - 1));
    KBP_WRITE_PCIE_REG(device, icf_pdc_registers_DMA_CONTROL, regval);


    /*enable dma*/
    dma_enable_disable(device, 1);
    return 0;

}

module_init(kbp_drv_module_init);
module_exit(kbp_drv_module_exit);

MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("Broadcom Limited");
MODULE_DESCRIPTION("Driver for KBP device");
