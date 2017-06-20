/* $Id: pciConfigLib.c,v 1.4 2011/07/21 16:14:28 yshtil Exp $
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/

/* pciConfigLib.c - PCI Configuration space access support for the bcm4710 */

/* This module allows the SI Bus devices to mimic PCI devices on bus 0 */
#include "vxWorks.h"
#include "config.h"

#include "vxLib.h"
#include "taskLib.h"

#include "dllLib.h"
#include "sysLib.h"
#include "intLib.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "config.h"
#include "drv/pci/pciConfigLib.h"

#include <typedefs.h>
#include <osl.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <vxbsp.h>
#include <bcmdevs.h>
#ifdef PCI_CONFIG_ADDR  /* make sure to use our definition */
#undef PCI_CONFIG_ADDR
#endif
#ifdef PCI_CFG_BIST /* make sure to use our definition */
#undef PCI_CFG_BIST
#endif
#include <pcicfg.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <siutils.h>
#include <hndmips.h>
#include <pci_core.h>
#include <hndpci.h>
#include <bcmnvram.h>

/* globals */
int     pciMaxBus = 1;
int     pci0MaxBus = 1;             /* Max number of sub-busses */
int     pci1MaxBus = PCIE1_BUS_MIN; /* Max number of sub-busses */
int     pciConfigMech = NONE;       /* 1=mechanism-1, 2=mechanism-2 */
STATUS  pciLibInitStatus = NONE;    /* initialization done */
STATUS  pciLibFixupStatus = NONE;   /* fixup done */

static uint32 pci0_membase = SI_PCI0_MEM;
static uint32 pci1_membase = SI_PCI1_MEM;

static si_t *sih = NULL;

extern void sysPciInt(int);

LOCAL int _pciFindExtDeviceNextBus
    (
    int nextBus,        /* Next PCI bus        */
    int    vendorId,    /* vendor ID */
    int    deviceId,    /* device ID */
    int    index,   /* desired instance of device */
    int *  pBusNo,  /* bus number */
    int *  pDeviceNo,   /* device number */
    int *  pFuncNo,  /* function number */
    int * found
    );
LOCAL int _pciFindExtDevice
    (
    int bus,           /* current bus number to probe      */
    int    vendorId,    /* vendor ID */
    int    deviceId,    /* device ID */
    int    index,   /* desired instance of device */
    int *  pBusNo,  /* bus number */
    int *  pDeviceNo,   /* device number */
    int *  pFuncNo,  /* function number */
    int * found
    );
LOCAL int _pciFindExtClassNextBus
    (
    int nextBus,        /* Next PCI bus        */
    int    classCode,    /* class code */
    int    index,   /* desired instance of device */
    int *  pBusNo,  /* bus number */
    int *  pDeviceNo,   /* device number */
    int *  pFuncNo,  /* function number */
    int *found
    );
LOCAL int _pciFindExtClass
    (
    int bus,           /* current bus number to probe      */
    int    classCode,    /* class code */
    int    index,   /* desired instance of device */
    int *  pBusNo,  /* bus number */
    int *  pDeviceNo,   /* device number */
    int *  pFuncNo,  /* function number */
    int *found
    );

/*******************************************************************************
*
* pciConfigLibInit - initialize the configuration access-method and addresses
*
* This routine initializes the configuration access-method and addresses.
*
* RETURNS:
* OK, or ERROR if a mechanism is not 0, 1, or 2.
*/
STATUS pciConfigLibInit
    (
    int mechanism, /* configuration mechanism: 0, 1, 2 */
    ULONG addr0,   /* config-addr-reg / CSE-reg */
    ULONG addr1,   /* config-data-reg / Forward-reg */
    ULONG addr2    /* none            / Base-address */
    )
{

    if (pciLibInitStatus != NONE)
        return (pciLibInitStatus);

    sih = si_kattach(SI_OSH);
    ASSERT(sih);

    pciLibInitStatus = OK;

    /* Initialize PCI core */
    if (hndpci_init(sih)) {
        return OK;
    }

    return (OK);
}

STATUS
pciConfigFixup (void)
{
    int    busNo;
    int    deviceNo;
    int    funcNo;
    UINT32 device;
    UINT32 vendor;
    char   header;
    UINT8 irq, flag;
    int pcie_core_id = 0;

    if (pciLibFixupStatus != NONE)
        return (pciLibFixupStatus);

        /* Fix up system backplane cores' interrupts */
        busNo = 0;
        for (deviceNo = 0; deviceNo < PCI_MAX_DEV; deviceNo++)
            for (funcNo = 0; funcNo < PCI_MAX_FUNC; funcNo++) {
                /* avoid a special bus cycle */

                if ((deviceNo == 0x1f) && (funcNo == 7))
                    continue;

                pciConfigInLong (busNo, deviceNo, funcNo, PCI_CFG_VENDOR_ID,
                         &vendor);

                /*
                 * If nonexistent device, skip to next, only look at
                 * vendor ID field for existence check
                 */

                if (((vendor & 0x0000ffff) == 0x0000ffff) && (funcNo == 0))
                    break;

                device  = vendor >> 16;
                device &= 0x0000FFFF;
                vendor &= 0x0000FFFF;
                /* Fix up interrupt lines */
                pciConfigInByte(busNo, deviceNo, funcNo, PCI_CFG_DEV_INT_LINE, &irq);
                if (irq == 0) {
                    /* Assign to shared interrupt vector */
                    pciConfigInByte(busNo, deviceNo, funcNo, PCI_CFG_DEV_INT_PIN, &flag);
                    irq = IV_IORQ0_BIT0_VEC + flag;
                } else {
                    /* Assign to regular interrupt vector */
                    irq += IV_IORQ0_VEC;
                }
                if (device == PCIE_CORE_ID) {
                    /* Hook up PCI interrupt */
                    (void) intConnect (INUM_TO_IVEC(irq), sysPciInt, pcie_core_id);
                    pcie_core_id ++;
                }                       
                pciConfigOutByte(busNo, deviceNo, funcNo, PCI_CFG_DEV_INT_LINE, irq);

                /* goto next if current device is single function */

                pciConfigInByte (busNo, deviceNo, funcNo, PCI_CFG_HEADER_TYPE, 
                         &header);
                if ((header & PCI_HEADER_MULTI_FUNC) != PCI_HEADER_MULTI_FUNC &&
                    funcNo == 0)
                    break;
            }


    return (pciLibFixupStatus = OK);
}

LOCAL int _pciFindExtDeviceNextBus
    (
    int nextBus,        /* Next PCI bus        */
    int    vendorId,    /* vendor ID */
    int    deviceId,    /* device ID */
    int    index,   /* desired instance of device */
    int *  pBusNo,  /* bus number */
    int *  pDeviceNo,   /* device number */
    int *  pFuncNo,  /* function number */
    int * found
    )
    {
    int subBus = -1; 
    subBus = _pciFindExtDevice (nextBus, vendorId, deviceId, index, pBusNo,
                              pDeviceNo, pFuncNo, found);

    /* Return the highest subordinate bus */

    return subBus;

    }

LOCAL int _pciFindExtDevice
    (
    int bus,           /* current bus number to probe      */
    int    vendorId,    /* vendor ID */
    int    deviceId,    /* device ID */
    int    index,   /* desired instance of device */
    int *  pBusNo,  /* bus number */
    int *  pDeviceNo,   /* device number */
    int *  pFuncNo,  /* function number */
    int * found
    )
    {
    UINT16 pciclass;        /* PCI class/subclass contents      */
    int ven_id, dev_id;      /* Device/Vendor identifier     */
    int device;         /* Device location          */
    int function;       /* Function location            */
    int subBus;         /* Highest subordinate PCI bus      */
    unsigned char btemp;        /* Temporary holding area       */
    int temp;
    int found_count = 0;
    BOOL   cont   = TRUE;

    /* Initialize variables */

    subBus = bus;

    /* if attributes indicate a host bus, then set equal to pciLoc.attrib */

    /* Locate each active function on the current bus */

    for (device = 0; ((cont == TRUE) && (device < PCI_MAX_DEV)); device++)
        {
        /* Check each function until an unused one is detected */

        for (function = 0; cont == TRUE && function < PCI_MAX_FUNC; function++)
            {

            /* Check for a valid device/vendor number */

            pciConfigInLong (bus, device, function,
                             PCI_CFG_VENDOR_ID, &temp);

            if (((temp & 0x0000ffff) == 0x0000FFFF) && (function == 0))
                break;

            dev_id  = temp >> 16;
            dev_id &= 0x0000FFFF;
            ven_id = temp & 0x0000FFFF;
            if ((ven_id == (UINT32)vendorId) &&
                (dev_id == (UINT32)deviceId) ) {
                found_count ++;
            }

            if ((found_count != 0) && (index-- == 0))
                {
                *pBusNo = bus;
                *pDeviceNo  = device;
                *pFuncNo    = function;
                cont    = FALSE;    /* terminate all loops */
                }


           if (bus != 0) { /* Don't process the following for backplane bus */
                /* Check to see if this function belongs to a PCI-PCI bridge */
    
                pciConfigInWord (bus, device, function,
                                 PCI_CFG_SUBCLASS, &pciclass);
    
                if (pciclass == ((PCI_CLASS_BRIDGE_CTLR << 8) + PCI_SUBCLASS_HOST_PCI_BRIDGE)) {
                    /* This is a host bridge, skip and find next device */
                    continue;
                }
                
                if ((pciclass == ((PCI_CLASS_BRIDGE_CTLR << 8) + PCI_SUBCLASS_CARDBUS_BRIDGE)) ||
                    (pciclass == ((PCI_CLASS_BRIDGE_CTLR << 8) + PCI_SUBCLASS_P2P_BRIDGE))) {
                    /* Setup and probe this bridge device */
                    subBus = _pciFindExtDeviceNextBus (subBus+1, vendorId, deviceId, index, pBusNo, pDeviceNo, pFuncNo, found);
                    return (subBus);
                }
            }

            /* Proceed to next device if this is a single function device */

            if (function == 0)
                {
                pciConfigInByte (bus, device, function,
                                 PCI_CFG_HEADER_TYPE, &btemp);
                if ((btemp & PCI_HEADER_MULTI_FUNC) == 0)
                    {
                    break; /* No more functions - proceed to next PCI device */
                    }
                }
            }
        }

    *found = found_count;
    if (!found) {
        return -1;
    } else {
        return subBus;
    }
    }

LOCAL int _pciFindExtClassNextBus
    (
    int nextBus,        /* Next PCI bus        */
    int    classCode,    /* class code */
    int    index,   /* desired instance of device */
    int *  pBusNo,  /* bus number */
    int *  pDeviceNo,   /* device number */
    int *  pFuncNo,  /* function number */
    int *found
    )
    {
    int subBus = -1; 
    subBus = _pciFindExtClass (nextBus, classCode, index, pBusNo,
                              pDeviceNo, pFuncNo, found);

    /* Return the highest subordinate bus */

    return subBus;

    }

LOCAL int _pciFindExtClass
    (
    int bus,           /* current bus number to probe      */
    int    classCode,    /* class code */
    int    index,   /* desired instance of device */
    int *  pBusNo,  /* bus number */
    int *  pDeviceNo,   /* device number */
    int *  pFuncNo,  /* function number */
    int *found
    )
    {
    UINT16 pciclass;        /* PCI class/subclass contents      */
    int    classCodeReg;
    int device;         /* Device location          */
    int function;       /* Function location            */
    int subBus;         /* Highest subordinate PCI bus      */
    unsigned char btemp;        /* Temporary holding area       */
    int temp;
    int found_count = 0;
    BOOL   cont   = TRUE;

    /* Initialize variables */

    subBus = bus;

    /* if attributes indicate a host bus, then set equal to pciLoc.attrib */

    /* Locate each active function on the current bus */

    for (device = 0; ((cont == TRUE) && (device < PCI_MAX_DEV)); device++)
        {
        /* Check each function until an unused one is detected */

        for (function = 0; cont == TRUE && function < PCI_MAX_FUNC; function++)
            {

            /* Check for a valid device/vendor number */

            pciConfigInLong (bus, device, function,
                             PCI_CFG_VENDOR_ID, &temp);

            if (((temp & 0x0000ffff) == 0x0000FFFF) && (function == 0))
                break;

            pciConfigInLong (bus, device, function, PCI_CFG_REVISION,
                 &classCodeReg);

            if (((classCodeReg >> 8) & 0x00ffffff) == classCode) {
                found_count ++;
            }

            if ((found_count != 0) &&  (index-- == 0))
                {
                *pBusNo = bus;
                *pDeviceNo  = device;
                *pFuncNo    = function;
                cont    = FALSE;    /* terminate all loops */
                }

           if (bus != 0) { /* Don't process the following for backplane bus */
                /* Check to see if this function belongs to a PCI-PCI bridge */
    
                pciConfigInWord (bus, device, function,
                                 PCI_CFG_SUBCLASS, &pciclass);
    
                if (pciclass == ((PCI_CLASS_BRIDGE_CTLR << 8) + PCI_SUBCLASS_HOST_PCI_BRIDGE)) {
                    /* This is a host bridge, skip and find next device */
                    continue;
                }
                
                if ((pciclass == ((PCI_CLASS_BRIDGE_CTLR << 8) + PCI_SUBCLASS_CARDBUS_BRIDGE)) ||
                    (pciclass == ((PCI_CLASS_BRIDGE_CTLR << 8) + PCI_SUBCLASS_P2P_BRIDGE))) {
                    /* Setup and probe this bridge device */
                    subBus = _pciFindExtClassNextBus (subBus+1, classCode, index, pBusNo, pDeviceNo, pFuncNo, found);
                    return (subBus);
                }
            }

            /* Proceed to next device if this is a single function device */

            if (function == 0)
                {
                pciConfigInByte (bus, device, function,
                                 PCI_CFG_HEADER_TYPE, &btemp);
                if ((btemp & PCI_HEADER_MULTI_FUNC) == 0)
                    {
                    break; /* No more functions - proceed to next PCI device */
                    }
                }
            }
        }

    *found = found_count;
    if (!found) {
        return -1;
    } else {
        return subBus;
    }
    }

/*******************************************************************************
*
* pciFindDevice - find the nth device with the given device & vendor ID
*
* This routine finds the nth device with the given device & vendor ID.
*
* RETURNS:
* OK, or ERROR if the deviceId and vendorId didn't match.
*/

STATUS pciFindDevice
    (
    int    vendorId,    /* vendor ID */
    int    deviceId,    /* device ID */
    int    index,   /* desired instance of device */
    int *  pBusNo,  /* bus number */
    int *  pDeviceNo,   /* device number */
    int *  pFuncNo  /* function number */
    )
    {
    STATUS status = ERROR;
    int    busNo;
    int rv_bus = -1;
    int found = 0;

    if (pciLibInitStatus != OK)         /* sanity check */
        return status;

    /* Search backplane bus */
    busNo = 0;
    rv_bus = _pciFindExtDevice(busNo, vendorId, deviceId, index, pBusNo, pDeviceNo, pFuncNo, &found);
    if ((index+1-found) > 0) {
        /* keep search in the next buses */
    } else {
        if (rv_bus != -1)  {
        	status = OK;
        } else {
        	status = ERROR;
        }
        return status;
    }

    /* Search PCIE control #0 */
    busNo = 1;
    index -= found;
    rv_bus = _pciFindExtDevice(busNo, vendorId, deviceId, index, pBusNo, pDeviceNo, pFuncNo, &found);
    if ((index+1-found) > 0) {
        /* keep search in the next buses */
    } else {
        if (rv_bus != -1)  {
        	status = OK;
        } else {
        	status = ERROR;
        }
        return status;
    }

    /* Search PCIE control #1 */
    busNo = PCIE1_BUS_MIN;
    index -= found;
    rv_bus = _pciFindExtDevice(busNo, vendorId, deviceId, index, pBusNo, pDeviceNo, pFuncNo, &found);
    if (rv_bus != -1) {
    	status = OK;
    } else {
    	status = ERROR;
    }

    return (status);
    }

/*******************************************************************************
*
* pciFindClass - find the nth occurence of a device by PCI class code.
*
* This routine finds the nth device with the given 24-bit PCI class code
* (class subclass prog_if).
* 
* The classcode arg of must be carfully constructed from class and sub-class
* macros.  
* 
* Example : To find an ethernet class device, construct the classcode arg 
*           as follows:
*
*           ((PCI_CLASS_NETWORK_CTLR << 16 | PCI_SUBCLASS_NET_ETHERNET << 8))
*
* RETURNS:
* OK, or ERROR if the class didn't match.
*/

STATUS pciFindClass
    (
    int    classCode,   /* 24-bit class code */
    int    index,   /* desired instance of device */
    int *  pBusNo,  /* bus number */
    int *  pDeviceNo,   /* device number */
    int *  pFuncNo  /* function number */
    )
    {
    STATUS status = ERROR;
    int    busNo;
    int rv_bus= -1;
    BOOL   cont   = TRUE;
    int    deviceNo;
    int    funcNo;
    int    classCodeReg;
    int    vendor;
    char   header;
    int found;

    if (pciLibInitStatus != OK)         /* sanity check */
        return status;

    /* Search backplane bus */
    busNo = 0;
    rv_bus = _pciFindExtClass(busNo, classCode, index, pBusNo, pDeviceNo, pFuncNo, &found);
    if ((index+1-found) > 0) {
        /* keep search in the next buses */
    } else {
        if (rv_bus != -1)  {
        	status = OK;
        } else {
        	status = ERROR;
        }
        return status;
    }

    /* Search PCIE control #0 */
    busNo = 1;
    index -= found;
    rv_bus = _pciFindExtClass(busNo, classCode, index, pBusNo, pDeviceNo, pFuncNo, &found);
    if ((index+1-found) > 0) {
        /* keep search in the next buses */
    } else {
        if (rv_bus != -1)  {
        	status = OK;
        } else {
        	status = ERROR;
        }
        return status;
    }

    /* Search PCIE control #1 */
    busNo = PCIE1_BUS_MIN;
    index -= found;
    rv_bus = _pciFindExtClass(busNo, classCode, index, pBusNo, pDeviceNo, pFuncNo, &found);
    if (rv_bus != -1) {
    	status = OK;
    } else {
    	status = ERROR;
    }


    return (status);
    }

/*******************************************************************************
*
* pciConfigInByte - read one byte from the PCI configuration space
*
* This routine reads one byte from the PCI configuration space
*
* RETURNS: OK, or ERROR if this library is not initialized
*/

STATUS pciConfigInByte
    (
    int busNo,    /* bus number */
    int deviceNo, /* device number */
    int funcNo,   /* function number */
    int offset,   /* offset into the configuration space */
    UINT8 * pData /* data read from the offset */
    )
{
    int key;
    STATUS ret;

    if (pciLibInitStatus != OK)
        return (ERROR);

    key = intLock();
    ret = hndpci_read_config(sih, busNo, deviceNo, funcNo, offset, pData, sizeof(*pData));
    intUnlock(key);

    return ret ? ERROR : OK;
}
    
/*******************************************************************************
*
* pciConfigInWord - read one word from the PCI configuration space
*
* This routine reads one word from the PCI configuration space
*
* RETURNS: OK, or ERROR if this library is not initialized
*/

STATUS pciConfigInWord
    (
    int busNo,      /* bus number */
    int deviceNo,   /* device number */
    int funcNo,     /* function number */
    int offset,     /* offset into the configuration space */
    UINT16 * pData  /* data read from the offset */
    )
{
    int key;
    STATUS ret;

    if (pciLibInitStatus != OK)
        return (ERROR);

    key = intLock();
    ret = hndpci_read_config(sih, busNo, deviceNo, funcNo, offset, pData, sizeof(*pData));
    intUnlock(key);

    return ret ? ERROR : OK;
}

/*******************************************************************************
*
* pciConfigInLong - read one longword from the PCI configuration space
*
* This routine reads one longword from the PCI configuration space
*
* RETURNS: OK, or ERROR if this library is not initialized
*/

STATUS pciConfigInLong
    (
    int busNo,     /* bus number */
    int deviceNo,  /* device number */
    int funcNo,    /* function number */
    int offset,    /* offset into the configuration space */
    UINT32 * pData /* data read from the offset */
    )
{
    int key;
    STATUS ret;

    if (pciLibInitStatus != OK)
        return (ERROR);

    key = intLock();
    ret = hndpci_read_config(sih, busNo, deviceNo, funcNo, offset, pData, sizeof(*pData));
    intUnlock(key);

    return ret ? ERROR : OK;
}

/*******************************************************************************
*
* pciConfigOutByte - write one byte to the PCI configuration space
*
* This routine writes one byte to the PCI configuration space.
*
* RETURNS: OK, or ERROR if this library is not initialized
*/

STATUS pciConfigOutByte
    (
    int busNo,    /* bus number */
    int deviceNo, /* device number */
    int funcNo,   /* function number */
    int offset,   /* offset into the configuration space */
    UINT8 data    /* data written to the offset */
    )
{
    int key;
    STATUS ret;

    if (pciLibInitStatus != OK)
        return (ERROR);

    key = intLock();
    ret = hndpci_write_config(sih, busNo, deviceNo, funcNo, offset, &data, sizeof(data));
    intUnlock(key);

    return ret ? ERROR : OK;
}

/*******************************************************************************
*
* pciConfigOutWord - write one 16-bit word to the PCI configuration space
*
* This routine writes one 16-bit word to the PCI configuration space.
*
* RETURNS: OK, or ERROR if this library is not initialized
*/

STATUS pciConfigOutWord
    (
    int busNo,    /* bus number */
    int deviceNo, /* device number */
    int funcNo,   /* function number */
    int offset,   /* offset into the configuration space */
    UINT16 data   /* data written to the offset */
    )
{
    int key;
    STATUS ret;

    if (pciLibInitStatus != OK)
        return (ERROR);

    key = intLock();
    ret = hndpci_write_config(sih, busNo, deviceNo, funcNo, offset, &data, sizeof(data));
    intUnlock(key);

    return ret ? ERROR : OK;
}

/*******************************************************************************
*
* pciConfigOutLong - write one longword to the PCI configuration space
*
* This routine writes one longword to the PCI configuration space.
*
* RETURNS: OK, or ERROR if this library is not initialized
*/

STATUS pciConfigOutLong
    (
    int busNo,    /* bus number */
    int deviceNo, /* device number */
    int funcNo,   /* function number */
    int offset,   /* offset into the configuration space */
    UINT32 data   /* data written to the offset */
    )
{
    int key;
    STATUS ret;

    if (pciLibInitStatus != OK)
        return (ERROR);

    key = intLock();
    ret = hndpci_write_config(sih, busNo, deviceNo, funcNo, offset, &data, sizeof(data));
    intUnlock(key);

    return ret ? ERROR : OK;
}

/*****************************************************************************
*
* pciConfigModifyLong - Perform a masked longword register update
*
* This function writes a field into a PCI configuration header without
* altering any bits not present in the field.  It does this by first
* doing a PCI configuration read (into a temporary location) of the PCI
* configuration header word which contains the field to be altered.
* It then alters the bits in the temporary location to match the desired
* value of the field.  It then writes back the temporary location with
* a configuration write.  All configuration accesses are long and the
* field to alter is specified by the "1" bits in the 'bitMask' parameter.
*
* Be careful to using pciConfigModifyLong for updating the Command and
* status register.  The status bits must be written back as zeroes, else
* they will be cleared.  Proper use involves including the status bits in
* the mask value, but setting their value to zero in the data value.
*
* The following example will set the PCI_CMD_IO_ENABLE bit without clearing any
* status bits.  The macro PCI_CMD_MASK includes all the status bits as
* part of the mask.  The fact that PCI_CMD_MASTER doesn't include these bits,
* causes them to be written back as zeroes, therefore they aren't cleared.
*
* .CS
*     pciConfigModifyLong (b,d,f,PCI_CFG_COMMAND,
*                  (PCI_CMD_MASK | PCI_CMD_IO_ENABLE), PCI_CMD_IO_ENABLE);
* .CE
*
* Use of explicit longword read and write operations for dealing with any
* register containing "write 1 to clear" bits is sound policy.
*
* RETURNS: OK if operation succeeds, ERROR if operation fails.
*/

STATUS pciConfigModifyLong
    (
    int busNo,          /* bus number */
    int deviceNo,       /* device number */
    int funcNo,         /* function number */
    int offset,         /* offset into the configuration space */
    UINT32 bitMask,     /* Mask which defines field to alter */
    UINT32 data         /* data written to the offset */
    )
    {
    UINT32 temp;
    STATUS stat;
    int key;

    /* check for library initialization or unaligned access */

#ifdef PCI_CONFIG_OFFSET_NOCHECK
    if (pciLibInitStatus != OK)
    {
        return (ERROR);
    }
#else
    if ((pciLibInitStatus != OK) || ((offset & (int)0x3) > 0) )
    {
        return (ERROR);
    }
#endif
 
    key = intLock ();

    stat = pciConfigInLong (busNo, deviceNo, funcNo, offset, &temp);

    if (stat == OK)
    {
    temp = (temp & ~bitMask) | (data & bitMask);
    stat = pciConfigOutLong (busNo, deviceNo, funcNo, offset, temp);
    }

    intUnlock (key);

    return (stat);
    }


/*****************************************************************************
*
* pciConfigModifyWord - Perform a masked longword register update
*
* This function writes a field into a PCI configuration header without
* altering any bits not present in the field.  It does this by first
* doing a PCI configuration read (into a temporary location) of the PCI
* configuration header word which contains the field to be altered.
* It then alters the bits in the temporary location to match the desired
* value of the field.  It then writes back the temporary location with
* a configuration write.  All configuration accesses are long and the
* field to alter is specified by the "1" bits in the 'bitMask' parameter.
*
* Do not use this routine to modify any register that contains 'write 1
* to clear' type of status bits in the same longword.  This specifically
* applies to the command register.  Modify byte operations could potentially
* be implemented as longword operations with bit shifting and masking.  This
* could have the effect of clearing status bits in registers that aren't being
* updated.  Use pciConfigInLong and pciConfigOutLong, or pciModifyLong,
* to read and update the entire longword.
*
* RETURNS: OK if operation succeeds.  ERROR if operation fails.
*/

STATUS pciConfigModifyWord
    (
    int busNo,          /* bus number */
    int deviceNo,       /* device number */
    int funcNo,         /* function number */
    int offset,         /* offset into the configuration space */
    UINT16 bitMask,     /* Mask which defines field to alter */
    UINT16 data         /* data written to the offset */
    )
    {
    UINT16 temp;
    STATUS stat;
    int key;

    /* check for library initialization or unaligned access */

#ifdef PCI_CONFIG_OFFSET_NOCHECK
    if (pciLibInitStatus != OK)
    {
        return (ERROR);
    }
#else
    if ((pciLibInitStatus != OK) || ((offset & (int)0x1) > 0) )
    {
        return (ERROR);
    }
#endif
 
    key = intLock ();

    stat = pciConfigInWord (busNo, deviceNo, funcNo, offset, &temp);

    if (stat == OK)
    {
    temp = (temp & ~bitMask) | (data & bitMask);
    stat = pciConfigOutWord (busNo, deviceNo, funcNo, offset, temp);
    }

    intUnlock (key);

    return (stat);
    }



/*****************************************************************************
*
* pciConfigModifyByte - Perform a masked longword register update
*
* This function writes a field into a PCI configuration header without
* altering any bits not present in the field.  It does this by first
* doing a PCI configuration read (into a temporary location) of the PCI
* configuration header word which contains the field to be altered.
* It then alters the bits in the temporary location to match the desired
* value of the field.  It then writes back the temporary location with
* a configuration write.  All configuration accesses are long and the
* field to alter is specified by the "1" bits in the 'bitMask' parameter.
*
* Do not use this routine to modify any register that contains 'write 1
* to clear' type of status bits in the same longword.  This specifically
* applies to the command register.  Modify byte operations could potentially
* be implemented as longword operations with bit shifting and masking.  This
* could have the effect of clearing status bits in registers that aren't being
* updated.  Use pciConfigInLong and pciConfigOutLong, or pciModifyLong,
* to read and update the entire longword.
*
* RETURNS: OK if operation succeeds, ERROR if operation fails.
*/

STATUS pciConfigModifyByte
    (
    int busNo,          /* bus number */
    int deviceNo,       /* device number */
    int funcNo,         /* function number */
    int offset,         /* offset into the configuration space */
    UINT8 bitMask,      /* Mask which defines field to alter */
    UINT8 data          /* data written to the offset */
    )
    {
    UINT8 temp;
    STATUS stat;
    int key;

    /* check for library initialization or unaligned access */

    if (pciLibInitStatus != OK)
    {
        return (ERROR);
    }

 
    key = intLock ();

    stat = pciConfigInByte (busNo, deviceNo, funcNo, offset, &temp);

    if (stat == OK)
    {
    temp = (temp & ~bitMask) | (data & bitMask);
    stat = pciConfigOutByte (busNo, deviceNo, funcNo, offset, temp);
    }

    intUnlock (key);

    return (stat);

    }

/**********************************************************************
*
* pciConfigForeachFunc - check condition on specified bus
*
* pciConfigForeachFunc() discovers the PCI functions present on the
* bus and calls a specified C-function for each one.  If the
* function returns ERROR, further processing stops.
*
* pciConfigForeachFunc() does not affect any HOST<->PCI
* bridge on the system.
*
* ERRNO: not set
*
* RETURNS: OK normally, or ERROR if funcCheckRtn() doesn't return OK.
*
*/

#define PCI_CONFIG_ABSENT_WORD_F 0xffff
#define PCI_CONFIG_ABSENT_WORD_0 0x0000

STATUS pciConfigForeachFunc
 (
  UINT8 bus,/* bus to start on */
  BOOL recurse,/* if TRUE, do subordinate busses */
  PCI_FOREACH_FUNC funcCheckRtn,  /* routine to call for each PCI func */
  void *pArg/* argument to funcCheckRtn */
 )
 {
  int     pciLocBus;  /* PCI bus/device/function structure */
  int     pciLocDevice;   /* PCI bus/device/function structure */
  int     pciLocFunction; /* PCI bus/device/function structure */
    int     device;     /* loop over devices */
    int     function;   /* loop over functions */
    UINT    devVend;
    UINT16  pciClass;   /* PCI class/subclass contents */
    int     status;
    UINT8   btemp;
    UINT8   secBus;     /* secondary bus, for recursion */
    UINT16  hostBridge = (PCI_CLASS_BRIDGE_CTLR<<8) |
                         PCI_SUBCLASS_HOST_PCI_BRIDGE;

    /* Begin the scanning process at [bus,0,0] */

    pciLocBus = (UINT8)bus;
    pciLocDevice = (UINT8)0;
    pciLocFunction = (UINT8)0;

    /* discover devices and perform check */

    /* Locate each active function on the current bus */

    for (device = 0; device < PCI_MAX_DEV; device++)
        {
        pciLocDevice = device;

        /* Check each function until an unused one is detected */

        for (function = 0; function < PCI_MAX_FUNC; function++)
            {
            pciLocFunction = function;

            /* Check for a valid device/vendor number */
            pciConfigInLong (pciLocBus, pciLocDevice, pciLocFunction,
                             PCI_CFG_VENDOR_ID, &devVend);

            /* If function 0 then check next dev else check next function */
            if ( ((devVend & 0x0ffff) == PCI_CONFIG_ABSENT_WORD_F) ||
                 ((devVend & 0x0ffff) == PCI_CONFIG_ABSENT_WORD_0) )
                {
                if (function == 0)
                    {
                    break;  /* non-existent device, go to next device */
                    }
                else
                    {
                    continue;  /* function empty, try the next function */
                    }
                }
            /* Check to see if this function belongs to a PCI-PCI bridge */
            pciConfigInWord (pciLocBus, pciLocDevice, pciLocFunction,
                             PCI_CFG_SUBCLASS, &pciClass);

            if ( pciClass != hostBridge )
                {
                /* call specified function */
                status = (*funcCheckRtn)(pciLocBus, pciLocDevice,
                                         pciLocFunction, pArg);
                if ( status != OK )
                    return(ERROR);
                }

            if ( recurse )
                {
                /* if P2P bridge, check that bus recursively */
                if ( (pciClass == ((PCI_CLASS_BRIDGE_CTLR << 8) + PCI_SUBCLASS_P2P_BRIDGE)) ||
                     (pciClass == ((PCI_CLASS_BRIDGE_CTLR << 8) + PCI_SUBCLASS_CARDBUS_BRIDGE)))
                    {
                    pciConfigInByte (pciLocBus, pciLocDevice, pciLocFunction,
                                     PCI_CFG_SECONDARY_BUS, &secBus);
                    if ( secBus > 0 )
                        status = pciConfigForeachFunc(secBus, recurse,
                                                      funcCheckRtn, pArg);
                    else
                        status = OK;

                    if ( status != OK )
                        return(ERROR);
                    }
                }
            /* Proceed to next device if this is a single function device */
            if (function == 0)
                {
                pciConfigInByte (pciLocBus, pciLocDevice, pciLocFunction,
                                 PCI_CFG_HEADER_TYPE, &btemp);
                if ((btemp & PCI_HEADER_MULTI_FUNC) == 0)
                    {
                    break; /* No more functions - proceed to next PCI device */
                    }
                }
            }
        }

    return(OK);
    }

