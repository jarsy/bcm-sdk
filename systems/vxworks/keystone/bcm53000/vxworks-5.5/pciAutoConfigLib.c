/* pciAutoConfigLib.c - PCI bus scan and resource allocation facility */

/* $Id: pciAutoConfigLib.c,v 1.3 2011/07/21 16:14:25 yshtil Exp $
 * Copyright (c) 1998-2006 Wind River Systems, Inc.
 * 
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/* Copyright 1997,1998,1999 Motorola, Inc. All Rights Reserved */

/*
modification history
--------------------
02b,28mar06,ajc  SPR 119277: update sysExcMsg pointer (comments)
02a,03nov05,mdo  SPR#107002 - inherit bridge capabilities for graphics
                 controller
01z,23jun05,mdo  Changing several PCI defines to common PCI defined names.
01y,08sep04,mdo  Documentation fixes for apigen
01x,05jun04,dgp  clean up formatting to correct doc build errors, convert to
         apigen markup
01w,06apr04,pes  Clean up compilation warnings.
01v,23jun03,m_h  Cardbus support
01u,15may02,pmr  SPR 76759: do not assume 32-bit I/O space is supported.
01t,02nov01,tor  Remove (void *) on lvalue.
01s,09oct01,tor  Add pciAutoCfgCtl() & other config functions,
          SPRs #24505, #24506, #31830, #27280, #67568
01r,10aug00,wef  Removed redundant call to pciAutoListCreate in
              pciAutoConfig: SPR 33636, fix logMsg 31830 (merge from AE)
01q,04oct99,scb  Fix broken roll call, interrupt routing. SPRs 29158, 31254
01p,20jul99,tm   AddrAlign ret assign before base+align+alloc check (SPR 28243)
01o,10jun99,scb  Update to latest WRS version.
01n,08jun99,scb  Moved from T1 base to T2 base.
01m,13apr99,djs  Made FuncEnable, FuncDisable, RegConfig global (SPR 26484)
01l,12apr99,scb  added roll call capability
01k,05may99,tm   Moved bridge Prefetch detection/attribute set to probe phase
                 Prefetch enable gated by parent bridge prefetch support
                 Added pciAutoAddrAlign to handle address alignment/checking
         BAR scanning now not stopped at first unimplemented BAR
01j,30mar99,dat  changed class to pciClass (SPR 25312)
01i,02mar99,tm   Fixed Prefetch bridge attrib (SPR 25396) / doc cleanup
01h,10feb99,tm   Prefetch bit is now gated correctly from BARs (SPR 25033)
01g,21jan99,tm   Fixed attribute inheritance mask (SPR 24605)
01f,18nov98,tm   Limit non-bus-0 I/O addresses to 16-bit decode (SPR 23428)
01e,23oct98,dat  fixed CSR accesses to not reset status.
01d,30jul98,tm   more code review revisions / Prefetchable Memory support added
01c,01jul98,tm   code review revisions / Raven-like host bridge init removed
                   Cyclic interrupt routing for non-bus0 devices added
           Corrected non-spec existence check (SPR 21934)
01b,20mar98,tm   adapted from pciAutoLib
01a,05Jan98,scb  written based on Motorola bug code
*/

/*
DESCRIPTION

This library provides a facility for automated PCI device scanning and
configuration on PCI-based systems.

Modern PCI based systems incorporate many peripherals and may span multiple
physical bus segments, and these bus segments may be connected via
PCI-to-PCI Bridges.  Bridges are identified and properly numbered
before a recursive scan identifies all resources on the bus implemented
by the bridge.  Post-scan configuration of the subordinate bus number
is performed.

Resource requirements of each device are identified and allocated according
to system resource pools that are specified by the BSP Developer.  Devices may
be conditionally excluded, and interrupt routing information obtained via
optional routines provided by the BSP Developer.

GENERAL ALGORITHM

The library must first be initialized by a call to pciAutoConfigLibInit().
The return value, pCookie, must be passed to each subsequent call
from the library.  Options can be set using the function pciAutoCfgCtl().
The available options are described in the documentation for pciAutoCfgCtl().

After initialization of the library and configuration of any
options, autoconfiguration takes place in two phases.  In the first
phase, all devices and subordinate busses in a given system are
scanned and each device that is found causes an entry to be created
in the 'Probelist' or list of devices found during the
probe/configuration process.

In the second phase each device that is on the Probelist is checked to see
if it has been excluded from automatic configuration by the BSP developer.
If a particular function has not been excluded, then it is first disabled.
The Base Address Registers of the particular function are read to ascertain
the resource requirements of the function.  Each resource requirement is
checked against available resources in the applicable pool based on size
and alignment constraints.

After all functions on the Probelist have been processed, each function
and it's appropriate Memory or I/O decoder(s) are enabled for operation.

HOST BRIDGE DETECTION/CONFIGURATION

Note that the PCI Host Bridge is automatically excluded from configuration
by the autoconfig routines, as it is often already configured as part of
the system bootstrap device configuration.

PCI-PCI BRIDGE DETECTION/CONFIGURATION

Busses are scanned by first writing the primary, secondary, and subordinate
bus information into the bridge that implements the bus.  Specifically,
the primary and secondary bus numbers are set to their corresponding value,
and the subordinate bus number is set to 0xFF, because the final number of
sub-busses is not known.  The subordinate bus number is later updated to
indicate the highest numbered sub-bus that was scanned once the scan is
complete.

GENERIC DEVICE DETECTION/CONFIGURATION

The autoconfiguration library creates a list of devices during the process
of scanning all of the busses in a system.  Devices with vendor
IDs of 0xFFFF and 0x0000 are skipped.  Once all busses have been
scanned, all non-excluded devices are then disabled prior to configuration.

Devices that are not excluded will have Resources allocated according
to Base Address Registers that are implemented by the device and
available space in the applicable resource pool.  PCI 'Natural' alignment
constraints are adhered to when allocating resources from pools.

Also initialized are the cache line size register and the latency
timer.  Bus mastering is unconditionally enabled.

If an interrupt assignment routine is registered, then the interrupt
pin register of the PCI Configuration space is passed to this routine
along with the bus, device, and function number of the device under
consideration.

There are two different schemes to determine when the BSP interrupt
assignment routine is called by autoconfig.  The call is done either
only for bus-0 devices or for all devices depending upon how the
autoIntRouting is set by the BSP developer (see the section
"INTERRUPT ROUTING ACROSS PCI-TO-PCI BRIDGES" below for more details).

The interrupt level number returned by this routine is then written
into the interrupt line register of the PCI Configuration Space for
subsequent use by device drivers.  If no interrupt assignment routine
is registered, 0xFF is written into the interrupt line register,
specifying an unknown interrupt binding.

Lastly, the functions are enabled with what resources were able to be
provided from the applicable resource pools.

RESOURCE ALLOCATION

Resource pools include the 32-bit Prefetchable Memory pool,
the 32-bit Non-prefetchable Memory ("MemIO") pool, the 32-bit I/O pool,
and the 16-bit I/O allocation pool.  The allocation in each pool begins at
the specified base address and progresses to higher numbered addresses.
Each allocated address adheres to the PCI 'natural' alignment constraints
of the given resource requirement specified in the Base Address Register.

DATA STRUCTURES

Data structures are either allocated statically or allocated
dynamically, depending on the value of the build macro
PCI_AUTO_STATIC_LIST, discussed below.  In either case, the
structures are initialized by the call to pciAutoConfigLibInit().

For ease of upgrading from the older method which used the PCI_SYSTEM
structure, the option PCI_SYSTEM_STRUCT_COPY has been implemented.
See the in the documentation for pciAutoCfgCtl() for more information.

PCI RESOURCE POOLS

Resources used by pciAutoConfigLib can be divided into two groups.

The first group of information is the Memory and I/O resources, that are
available in the system and that autoconfig can use to allocate to functions.
These resource pools consist of a base address and size.  The base address
specified here should be the address relative to the PCI bus.  Each of these
values in the PCI_SYSTEM data structure is described below:

\is
\i 'pciMem32'
Specifies the 32-bit prefetchable memory pool base address.
Normally, this is given by the BSP constant PCI_MEM_ADRS.
It can be set with the pciAutoCfgCtl() command PCI_MEM32_LOC_SET.

\i 'pciMem32Size'
Specifies the 32-bit prefetchable memory pool size.
Normally, this is given by the BSP constant PCI_MEM_SIZE.
It can be set with the pciAutoCfgCtl() command PCI_MEM32_SIZE_SET.

\i 'pciMemIo32'
Specifies the 32-bit non-prefetchable memory pool base address.
Normally, this is given by the BSP constant PCI_MEMIO_ADRS.
It can be set with the pciAutoCfgCtl() command PCI_MEMIO32_LOC_SET.

\i 'pciMemIo32Size'
Specifies the 32-bit non-prefetchable memory pool size
Normally, this is given by the BSP constant PCI_MEMIO_SIZE.
It can be set with the pciAutoCfgCtl() command PCI_MEMIO32_SIZE_SET.

\i 'pciIo32'
Specifies the 32-bit I/O pool base address.
Normally, this is given by the BSP constant PCI_IO_ADRS.
It can be set with the pciAutoCfgCtl() command PCI_IO32_LOC_SET.

\i 'pciIo32Size'
Specifies the 32-bit I/O pool size.
Normally, this is given by the BSP constant PCI_IO_SIZE.
It can be set with the pciAutoCfgCtl() command PCI_IO32_SIZE_SET.

\i 'pciIo16'
Specifies the 16-bit I/O pool base address.
Normally, this is given by the BSP constant PCI_ISA_IO_ADDR.
It can be set with the pciAutoCfgCtl() command PCI_IO16_LOC_SET.

\i 'pciIo16Size'
Specifies the 16-bit I/O pool size.
Normally, this is given by the BSP constant PCI_ISA_IO_SIZE.
It can be set with the pciAutoCfgCtl() command PCI_IO16_SIZE_SET.
\ie

PREFETCH MEMORY ALLOCATION
The pciMem32 pointer is assumed to point to a pool of prefetchable PCI
memory.  If the size of this pool is non-zero, then prefetch memory will
be allocated to devices that request it given that there is enough
memory in the pool to satisfy the request, and the host bridge or
PCI-to-PCI bridge that implements the bus that the device resides on is
capable of handling prefetchable memory.  If a device requests it, and
no prefetchable memory is available or the bridge implementing the bus
does not handle prefetchable memory then the request will be attempted
from the non-prefetchable memory pool.

PCI-to-PCI bridges are queried as to whether they support prefetchable
memory by writing a non-zero value to the prefetchable memory base
address register and reading back a non-zero value.  A zero value would
indicate the bridge does not support prefetchable memory.

BSP-SPECIFIC ROUTINES

Several routines can be provided by the BSP Developer to customize
the degree to which the system can be automatically configured.
These routines are normally put into a file called sysBusPci.c in
the BSP directory.  The trivial cases of each of these routines are
shown in the USAGE section below to illustrate the API to the BSP
Developer.

DEVICE INCLUSION
Specific devices other than bridges can be excluded from auto
configuration and either not used or manually configured later.
For more information, see the PCI_INCLUDE_FUNC_SET section in the
documentation for pciAutoCfgCtl().

INTERRUPT ASSIGNMENT
Interrupt assignment can be specified by the BSP developer by
specifying a routine for pciAutoConfigLib to call at the time each
device or bridge is configured.  See the PCI_INT_ASSIGN_FUNC_SET
section in the documentation for pciAutoCfgCtl() for more information.

INTERRUPT ROUTING ACROSS PCI-TO-PCI BRIDGES
PCI autoconfig allows use of two interrupt routing strategies for handling
devices that reside across a PCI-to-PCI Bridge.  The BSP-specific interrupt
assignment routine described in the above section is called for all devices
that reside on bus 0.  For devices residing across a PCI-to-PCI bridge, one
of two supported interrupt routing strategies may be selected by setting
the PCI_AUTO_INT_ROUTE_SET command using pciAutoCfgCtl() to the boolean
value TRUE or FALSE:

\is
\i 'TRUE'
If automatic interrupt routing is set to TRUE, then autoconfig only
calls the BSP interrupt routing routine for devices on bus number
0.  If a device resides on a higher numbered bus, then a cyclic
algorithm is applied to the IRQs that are routed through the
bridge.  The algorithm is based on computing a 'route offset' that
is the device number modulo 4 for every bridge device that is
traversed.  This offset is used with the device number and interrupt
pin register of the device of interest to compute the contents of
the interrupt line register.

\i 'FALSE'
If automatic interrupt routing is set to FALSE, then autoconfig
calls the BSP interrupt assignment routine to do all interrupt
routing regardless of the bus on which the device resides.  The
return value represents the contents of the interrupt line register
in all cases.
\ie

BRIDGE CONFIGURATION
The BSP developer may wish to perform configuration of bridges
before and/or after the normal configuration of the bus they reside
on.  Two routines can be specified for this purpose.

The bridge pre-configuration pass initialization routine is provided
so that the BSP Developer can initialize a bridge device prior to the
configuration pass on the bus that the bridge implements.

The bridge post-configuration pass initialization routine is provided
so that the BSP Developer can initialize the bridge device after the bus
that the bridge implements has been enumerated.

These routines are configured by calling pciAutoCfgCtl() with the
command PCI_BRIDGE_PRE_CONFIG_FUNC_SET and the command
PCI_BRIDGE_POST_CONFIG_FUNC_SET, respectively.

HOST BRIDGE CONFIGURATION
The PCI Local Bus Specification, rev 2.1 does not specify the content or
initialization requirements of the configuration space of PCI Host Bridges.
Due to this fact, no host bridge specific assumptions are made by
autoconfig and any PCI Host Bridge initialization that must be done before either
scan or configuration of the bus must be done in the BSP.  Comments
illustrating where this initialization could be called in relation to
invoking the pciAutoConfig() routine are in the USAGE section below.

LIBRARY CONFIGURATION MACROS
The following four macros can be defined by the BSP Developer in config.h
to govern the operation of the autoconfig library.

\is
\i 'PCI_AUTO_MAX_FUNCTIONS'
Defines the maximum number of functions that can be stored in the probe list
during the autoconfiguration pass.  The default value for this define is 32,
but this may be overridden by defining PCI_AUTO_MAX_FUNCTIONS in config.h.

\i 'PCI_AUTO_STATIC_LIST'
If defined, then a statically allocated array of size PCI_AUTO_MAX_FUNCTION
instances of the PCI_LOC structure will be instantiated.

\i 'PCI_AUTO_RECLAIM_LIST'
This define may only be used if PCI_AUTO_STATIC_LIST is not defined.  If
defined, this allows the autoconfig routine to perform a free() operation
on a dynamically allocated probe list.  Note that if PCI_AUTO_RECLAIM_LIST
is defined and PCI_AUTO_STATIC_LIST is also, a compiler error will be
generated.
\ie

USAGE

The following code sample illustrates the usage of the PCI_SYSTEM structure
and invocation of the autoconfig library.  Note that the example BSP-specific
routines are merely stubs.  The code in each routine varies by BSP and
application.

\cs
#include "pciAutoConfigLib.h"

LOCAL PCI_SYSTEM sysParams;

void sysPciAutoConfig (void)
    {
    void * pCookie;

    /@ initialize the library @/
    pCookie = pciAutoConfigLibInit(NULL);

    /@ 32-bit Prefetchable Memory Space @/

    pciAutoCfgCtl(pCookie, PCI_MEM32_LOC_SET, PCI_MEM_ADRS);
    pciAutoCfgCtl(pCookie, PCI_MEM32_SIZE_SET, PCI_MEM_SIZE);

    /@ 32-bit Non-prefetchable Memory Space @/

    pciAutoCfgCtl(pCookie, PCI_MEMIO32_LOC_SET, PCI_MEMIO_ADRS);
    pciAutoCfgCtl(pCookie, PCI_MEMIO32_SIZE_SET, PCI_MEMIO_SIZE);

    /@ 16-bit ISA I/O Space @/

    pciAutoCfgCtl(pCookie, PCI_IO16_LOC_SET, PCI_ISA_IO_ADRS);
    pciAutoCfgCtl(pCookie, PCI_IO16_SIZE_SET, PCI_ISA_IO_SIZE);

    /@ 32-bit PCI I/O Space @/

    pciAutoCfgCtl(pCookie, PCI_IO32_LOC_SET, PCI_IO_ADRS);
    pciAutoCfgCtl(pCookie, PCI_IO32_SIZE_SET, PCI_IO_SIZE);

    /@ Configuration space parameters @/

    pciAutoCfgCtl(pCookie, PCI_MAX_BUS_SET, 0);
    pciAutoCfgCtl(pCookie, PCI_MAX_LAT_ALL_SET, PCI_LAT_TIMER);
    pciAutoCfgCtl(pCookie, PCI_CACHE_SIZE_SET,
            ( _CACHE_ALIGN_SIZE / 4 ));

    /@
     * Interrupt routing strategy
     * across PCI-to-PCI Bridges
     @/

    pciAutoCfgCtl(pCookie, PCI_AUTO_INT_ROUTE_SET, TRUE);

    /@ Device inclusion and interrupt routing routines @/

    pciAutoCfgCtl(pCookie, PCI_INCLUDE_FUNC_SET,
            sysPciAutoconfigInclude);
    pciAutoCfgCtl(pCookie, PCI_INT_ASSIGN_FUNC_SET,
            sysPciAutoconfigIntrAssign);

    /@
     * PCI-to-PCI Bridge Pre-
     * and Post-enumeration init
     * routines
     @/

    pciAutoCfgCtl(pCookie, PCI_BRIDGE_PRE_CONFIG_FUNC_SET,
    sysPciAutoconfigPreEnumBridgeInit);
    pciAutoCfgCtl(pCookie, PCI_BRIDGE_POST_CONFIG_FUNC_SET,
    sysPciAutoconfigPostEnumBridgeInit);

    /@
     * Perform any needed PCI Host Bridge
     * Initialization that needs to be done
     * before pciAutoConfig is invoked here
     * utilizing the information in the
     * newly-populated sysParams structure.
     @/

    pciAutoCfg (&sysParams);

    /@
     * Perform any needed post-enumeration
     * PCI Host Bridge Initialization here.
     * Information about the actual configuration
     * from the scan and configuration passes
     * can be obtained using the assorted
     * PCI_*_GET commands to pciAutoCfgCtl().
     @/

    }

    /@
     * Local BSP-Specific routines
     * supplied by BSP Developer
     @/

STATUS sysPciAutoconfigInclude
    (
    PCI_SYSTEM * pSys,          /@ PCI_SYSTEM structure pointer @/
    PCI_LOC * pLoc,         /@ pointer to function in question @/
    UINT devVend            /@ deviceID/vendorID of device @/
    )
    {
    return OK; /@ Autoconfigure all devices @/
    }

UCHAR sysPciAutoconfigIntrAssign
    (
    PCI_SYSTEM * pSys,          /@ PCI_SYSTEM structure pointer @/
    PCI_LOC * pLoc,         /@ pointer to function in question @/
    UCHAR pin               /@ contents of PCI int pin register @/
    )
    {
    return (UCHAR)0xff;
    }

void sysPciAutoconfigPreEnumBridgeInit
    (
    PCI_SYSTEM * pSys,          /@ PCI_SYSTEM structure pointer @/
    PCI_LOC * pLoc,         /@ pointer to function in question @/
    UINT devVend            /@ deviceID/vendorID of device @/
    )
    {
    return;
    }

void sysPciAutoconfigPostEnumBridgeInit
    (
    PCI_SYSTEM * pSys,          /@ PCI_SYSTEM structure pointer @/
    PCI_LOC * pLoc,         /@ pointer to function in question @/
    UINT devVend            /@ deviceID/vendorID of device @/
    )
    {
    return;
    }
\ce

CONFIGURATION SPACE PARAMETERS
The cache line size register specifies the cacheline size in longwords.
This register is required when a device can generate a memory write and
Invalidate bus cycle, or when a device provides cacheable memory to the system.

Note that in the above example, the macro _CACHE_ALIGN_SIZE is utilized.  This
macro is implemented for all supported architectures and is located in the
<architecture>.h file in .../target/h/arch/<architecture>.  The value of the
macro indicates the cache line size in bytes for the particular architecture.
For example, the PowerPC architecture defines this macro to be 32, while
the ARM 810 defines it to be 16.  The PCI cache line size field and
the cacheSize element of the PCI_SYSTEM structure expect to see this quantity
in longwords, so the byte value must be divided by 4.

LIMITATIONS
The current version of the autoconfig facility does not support 64-bit
prefetchable memory behind PCI-to-PCI bridges, but it does support
32-bit prefetchable memory.

The autoconfig code also depends upon the BSP Developer specifying resource
pools that do not conflict with any resources that are being used by
statically configured devices.

INCLUDE FILES:
pciAutoConfigLib.h

SEE ALSO:
\tb "PCI Local Bus Specification, Revision 2.1, June 1, 1996"
\tb "PCI Local Bus PCI to PCI Bridge Architecture Specification, Revision 1.0, April 5, 1994"

\INTERNAL
Simple Text-Based Debug Support
Note that the macro PCI_AUTO_DEBUG may be defined, and the macro
PCI_AUTO_DEBUG_MSG utilized.  PCI_AUTO_DEBUG_MSG is identical to a function
call to logMsg() in function in that it calls _func_logMsg() with the
string and six parameters passed to it.  The macro also invokes taskDelay
to allow the debug string to be sent with minimal interruption.

Also note that the macro PCI_AUTO_DEBUG initializes a global variable
pciAutoDebug to a non-zero value.  Display of debug messages may be turned
on and off during runtime by manipulating this variable.  If the variable is
set to zero, messages will not be displayed.

Attributes
Attributes are reserved for use by the autoconfiguration routines.  There
is presently no user-level API at this time to access attributes for a
particular device.  The BSP-specific device exclusion routine affects
the attributes indirectly by specifying which devices are to be excluded
from the scan and configuration process.

Attributes are divided into device attributes and bridge attributes.  The first
group below describes device attributes.

\is
\i 'PCI_AUTO_ATTR_DEV_EXCLUDE'
Specifies that a device is to be excluded from the automatic scan
and configuration process
\i 'PCI_AUTO_ATTR_DEV_DISPLAY'
Specifies that a device is a display device
\i 'PCI_AUTO_ATTR_DEV_PREFETCH'
Specifies that a device has requested Prefetchable PCI memory
\ie

The second group below describes Bridge attributes.

\is
\i 'PCI_AUTO_ATTR_BUS_PREFETCH'
Specifies that the bridge device supports Prefetchable Memory behind the bridge
\i 'PCI_AUTO_ATTR_BUS_PCI'
Specifies that the bridge device is a PCI-to-PCI bridge and implements a PCI bus
\i 'PCI_AUTO_ATTR_BUS_HOST'
Specifies that the bridge device is a PCI Host bridge and implements a PCI bus
\i 'PCI_AUTO_ATTR_BUS_ISA'
Specifies that the bridge device is an ISA bridge implements an ISA bus
\i 'PCI_AUTO_ATTR_BUS_4GB_IO'
Specifies that the bridge device supports 32-bit I/O Addressing
behind the bridge
\ie

Attribute Inheritance
Devices that reside on a particular bus automatically inherit the attributes
of the bridge (Host or PCI-to-PCI) that implements that bus.  This allows
devices to take advantage of the fact that, for example, a PCI-to-PCI bridge
implements full 32-bit PCI I/O.  Note that device attributes, such as the
inclusion attribute, are not inherited.

*/


/* includes */

#include <vxWorks.h>
#include <logLib.h>
#include <taskLib.h>
#include <string.h>
#include <dllLib.h>
#include <config.h>
#include <errnoLib.h>
#ifdef USE_PCI_SIMULATOR
#include <stdio.h>
#include <stdlib.h>
#endif /* USE_PCI_SIMULATOR */

#include "drv/pci/pciConfigLib.h"
#include "drv/pci/pciAutoConfigLib.h"

#include <hndsoc.h>

/* local defines */

#define PCI_CMD_MASK    0xffff0000  /* mask to save status bits */

#define NO_ALLOCATION  0xffffffff

#define PCI_CONFIG_ABSENT_F 0xffff
#define PCI_CONFIG_ABSENT_0 0x0000

#define ERROR_EXCESS_SIZE -2
#define ALLOCATION_64BIT_MEM_SPACE 0xfffffffe

LOCAL int pcie0_64bit_region_alloc_index = 0;
LOCAL int pcie1_64bit_region_alloc_index = 0;

/* local configuration defines */

#define PCI_AUTO_STATIC_LIST

#undef PCI_AUTO_RECLAIM_LIST

#ifndef PCI_AUTO_MAX_FUNCTIONS
# define PCI_AUTO_MAX_FUNCTIONS 32
#endif /* PCI_AUTO_MAX_FUNCTIONS */

IMPORT FUNCPTR _func_logMsg;

#define PCI_LOG_MSG(s, a, b, c, d, e, f) \
    do { \
    if (_func_logMsg != NULL) \
        { \
        (*_func_logMsg)(s, a, b, c, d, e, f); \
        } \
    else if ( ( pPciCfgOpts->pciConfigInit == TRUE ) && ( pPciCfgOpts->pciLogMsgFunc != NULL ) ) \
        { \
        (*(pPciCfgOpts->pciLogMsgFunc))(s,a,b,c,d,e,f); \
        } \
    } while ((0))

#define PCI_AUTO_DEBUG_MSG(s, a, b, c, d, e, f) \
    do { \
    if ( pciAutoDebug == TRUE ) \
        { \
        PCI_LOG_MSG (s, a, b, c, d, e, f); \
    if ( taskIdCurrent != NULL ) \
            taskDelay(10); \
        } \
    } while ((0))

/* typedefs */

typedef struct pciAutoConfigOpts
    {
    /* copy of pSystem from pciAutoConfig() interface */
    UINT pciMem32;      /* 32 bit prefetchable memory location */
    UINT pciMem32Size;      /* 32 bit prefetchable memory size */
    UINT pciMemIo32;        /* 32 bit non-prefetchable memory location */
    UINT pciMemIo32Size;    /* 32 bit non-prefetchable memory size */
    UINT pciIo32;       /* 32 bit io location */
    UINT pciIo32Size;       /* 32 bit io size */
    UINT pciIo16;       /* 16 bit io location */
    UINT pciIo16Size;       /* 16 bit io size */
    int maxBus;         /* Highest subbus number */
    int cacheSize;      /* cache line size */
    UINT maxLatency;        /* max latency */
    BOOL autoIntRouting;        /* automatic routing strategy */
    PCI_INCLUDE_FUNC        includeRtn;
    PCI_INT_ASSIGN_FUNC     intAssignRtn;
    PCI_BRIDGE_PRE_CONFIG_FUNC  bridgePreConfigInit;
    PCI_BRIDGE_POST_CONFIG_FUNC bridgePostConfigInit;
    PCI_ROLL_CALL_FUNC      pciRollcallRtn;

    /* new stuff not available in the obsolete PCI_SYSTEM structure */

    BOOL        pciConfigInit;      /* internal use only */
    /* Fast Back TO Back Enable */
    BOOL        pciFBBEnable;       /* Enabled for system */
    BOOL        pciFBBActive;       /* implemented all cards & turned on */
    /* memory allocation */
    UINT32      pciMemBusMinRes;    /* minimum to reserve per bus */
    UINT32      pciMemBusExtraRes;  /* extra to reserve per bus */
    UINT32      pciMemMax;      /* maximum total to reserve */
    UINT32      pciMem32Used;       /* total 32-bit mem actually used */
    UINT32      pciMemIo32Used;     /* total 32-bit IOmem used */
    UINT32      pciIo32Used;        /* total 32-bit IO space used */
    UINT32      pciIo16Used;        /* total 16-bit IO space used */
    PCI_MEM_BUS_EXTRA_FUNC pciMemBusExtraFunc;  /* per bus, function to calculate */
    /* misc functions */
    PCI_LOGMSG_FUNC pciLogMsgFunc;      /* safe logMsg() func */
    PCI_MAX_LAT_FUNC    pciMaxLatFunc;      /* MAX_LAT calc each device */
    void *      pciMaxLatPArg;      /* user-supplied arg */
    /* PCI_AUTO_MAX_FUNCTIONS */
    PCI_LOC *       pFuncList;      /* user-supplied space */
    int         numFuncListEntries; /* number of entries available */
    BOOL        minimizeResources;  /* sort resource requirements */
    } PCI_AUTO_CONFIG_OPTS;

/* globals */

#ifdef PCI_AUTO_DEBUG
BOOL pciAutoDebug = TRUE;
#else
BOOL pciAutoDebug = FALSE;
#endif

IMPORT int pci0MaxBus;
IMPORT int pci1MaxBus;

/* locals */

LOCAL PCI_AUTO_CONFIG_OPTS pciAutoConfigOpts =
    {
    /* pSystem */
    0,      /* pciMem32 */
    0,      /* pciMem32Size */
    0,      /* pciMemIo32 */
    0,      /* pciMemIo32Size */
    0,      /* pciIo32 */
    0,      /* pciIo32Size */
    0,      /* pciIo16 */
    0,      /* pciIo16Size */
    0,      /* maxBus */
    0,      /* cacheSize */
    0,      /* maxLatency */
    FALSE,  /* autoIntRouting */
    NULL,   /* includeRtn */
    NULL,   /* intAssignRtn */
    NULL,   /* bridgePreConfigInit */
    NULL,   /* bridgePostConfigInit */
    NULL,   /* pciRollCallRtn */
    FALSE,  /* is this structure valid? */
    FALSE,  /* Fast Back To Back enabled */
    FALSE,  /* Fast Back To Back active */
    0,      /* mem 32-bit min to reserve */
    0,      /* mem 32-bit extra to reserve */
    0,      /* max total 32-bit memory */
    0,      /* actual 32-bit mem used */
    0,      /* actual 32-bit IOmem used */
    0,      /* actual 32-bit IO space used */
    0,      /* actual 16-bit IO space used */
    NULL,   /* per bus func to calculate extra mem */
    NULL,   /* logMsg() function */
    NULL,   /* function to calc MAX_LAT */
    NULL,   /* pArg for pciMaxLatFunc() */
    NULL,   /* pFuncList */
    0,      /* number of entries available in pFuncList */
    FALSE   /* minimize resources */
    };

LOCAL PCI_AUTO_CONFIG_OPTS * pPciCfgOpts;

#ifdef PCI_AUTO_STATIC_LIST
LOCAL PCI_LOC pciAutoLocalFuncList[PCI_AUTO_MAX_FUNCTIONS];
#endif

LOCAL int lastPciListSize;
LOCAL PCI_LOC *pLastPciList;

LOCAL UCHAR pciAutoIntRoutingTable[4] = { (UCHAR) 0xff,
                                      (UCHAR) 0xff,
                                      (UCHAR) 0xff,
                                      (UCHAR) 0xff
                                    };
/* forward declarations */

LOCAL PCI_LOC * pciAutoListCreate ( PCI_AUTO_CONFIG_OPTS * pSystem, int *pListSize);
LOCAL UINT pciAutoBusProbe ( PCI_AUTO_CONFIG_OPTS * pSystem, UINT priBus,
    UINT secBus, PCI_LOC*  pPciLoc, PCI_LOC** ppPciList,
    int * pListSize);
LOCAL UINT pciAutoDevProbe ( PCI_AUTO_CONFIG_OPTS * pSystem, UINT bus,
    UCHAR offset, UCHAR inheritAttrib, PCI_LOC **ppPciList, int * pListSize);
LOCAL void pciAutoFuncConfigAll ( PCI_AUTO_CONFIG_OPTS * pSystem,
    PCI_LOC *pPciList, UINT nSize);
LOCAL UCHAR pciAutoIntAssign ( PCI_AUTO_CONFIG_OPTS * pSystem, PCI_LOC * pFunc);
LOCAL void pciAutoDevConfig ( PCI_AUTO_CONFIG_OPTS * pSystem, UINT bus,
    PCI_LOC **ppPciList, UINT *nSize);
LOCAL void pciAutoFuncConfig ( PCI_AUTO_CONFIG_OPTS * pSystem, PCI_LOC * pPciFunc);
LOCAL UINT pciAutoIoAlloc ( PCI_AUTO_CONFIG_OPTS * pPciSys, PCI_LOC *pPciFunc,
    UINT *pAlloc, UINT nSize);
LOCAL UINT pciAutoMemAlloc ( PCI_AUTO_CONFIG_OPTS * pPciSys, PCI_LOC * pPciFunc,
    UINT * pAlloc, UINT size, UINT addrInfo);
LOCAL void pciAutoBusConfig ( PCI_AUTO_CONFIG_OPTS * pSystem, PCI_LOC * pPciLoc,
    PCI_LOC **ppPciList, UINT *nSize);
#if 0 /* TBD MDG */
LOCAL void pciAutoCardBusConfig ( PCI_AUTO_CONFIG_OPTS * pSystem,
    PCI_LOC * pPciLoc, PCI_LOC **ppPciList, UINT *nSize);
#endif
LOCAL STATUS pciAutoCfgFunc ( void *pCookie );
LOCAL STATUS pciAutoConfigFBBEnable ( PCI_AUTO_CONFIG_OPTS *    pOpts );
LOCAL STATUS pciAutoConfigFBBDisable ( PCI_AUTO_CONFIG_OPTS *   pOpts );
LOCAL STATUS pciFBBFuncClear ( UINT bus, UINT device, UINT function, void *pArg );
LOCAL STATUS pciFBBFuncSet ( UINT bus, UINT device, UINT function, void *pArg );
LOCAL STATUS pciFBBFuncCheck ( UINT bus, UINT device, UINT function, void *pArg );
LOCAL void pciAutoConfigCopyback ( PCI_AUTO_CONFIG_OPTS * pOpts, PCI_SYSTEM * pSystem);

#ifdef USE_PCI_SIMULATOR
void pciAutoConfigListShow(PCI_LOC *pLoc, int num);
#endif /* USE_PCI_SIMULATOR */
/* subroutines */

LOCAL int currentPcieController = PCIE_CONTROLLER_0;

void sysPciConfigEnable
    (
    int pciHost
    )
    {
    if (pciHost == PCIE_CONTROLLER_1)
        currentPcieController = PCIE_CONTROLLER_1;
    else
        currentPcieController = PCIE_CONTROLLER_0;

    }

/**********************************************************************
*
* pciAutoConfigLibInit - initialize PCI autoconfig library
*
* pciAutoConfigLib initialization function.
*
* RETURNS: A cookie for use by subsequent pciAutoConfigLib function
* calls.
*
* ERRNO
*
*/

void * pciAutoConfigLibInit
    (
    void * pArg             /* reserved for future use */
    )
    {
    pPciCfgOpts = &pciAutoConfigOpts;
    pPciCfgOpts->pciConfigInit = TRUE;
    return((void *)pPciCfgOpts);
    }

/******************************************************************************
*
* pciAutoCfg - Automatically configure all nonexcluded PCI headers
*
* Top level function in the PCI configuration process.
*
* CALLING SEQUENCE:
* \cs
* pCookie = pciAutoConfigLibInit(NULL);
* pciAutoCfgCtl(pCookie, COMMAND, VALUE);
*  ...
* pciAutoCfgCtl(pCookie, COMMAND, VALUE);
* pciAutoCfg(pCookie);
* \ce
*
* For ease in converting from the old interface to the new one,
* a pciAutoCfgCtl() command PCI_PSYSTEM_STRUCT_COPY has been
* implemented.  This can be used just like any other pciAutoCfgCtl()
* command, and it initializes all the values in pSystem.  If
* used, it should be the first call to pciAutoCfgCtl().
*
* For a description of the COMMANDs and VALUEs to pciAutoCfgCtl(), see
* the pciAutoCfgCtl() documentation.
*
* For all nonexcluded PCI functions on all PCI bridges, this routine
* automatically configures the PCI configuration headers for PCI
* devices and subbridges.  The fields that are programmed are:
*
* \ml
* \m 1.
* Status register.
* \m 2.
* Command Register.
* \m 3.
* Latency timer.
* \m 4.
* Cache Line size.
* \m 5.
* Memory and/or I/O base address and limit registers.
* \m 6.
* Primary, secondary, subordinate bus number (for PCI-PCI bridges).
* \m 7.
* Expansion ROM disable.
* \m 8.
* Interrupt Line.
* \me
*
* ALGORITHM
* Probe PCI config space and create a list of available PCI functions.
* Call device exclusion function, if registered, to exclude/include device.
* Disable all devices before we initialize any.
* Allocate and assign PCI space to each device.
* Calculate and set interrupt line value.
* Initialize and enable each device.
*
* RETURNS: N/A
*
* ERRNO
*
*/

STATUS pciAutoCfg
    (
    void *pCookie       /* cookie returned by pciAutoConfigLibInit() */
    )
    {
    PCI_AUTO_CONFIG_OPTS *pOpts;

    pOpts = (PCI_AUTO_CONFIG_OPTS *)pCookie;
    if ( pOpts->pciConfigInit != TRUE )
    {
    errnoSet(EINVAL);
    return(ERROR);
    }

    pciAutoCfgFunc(pCookie);

    /* check FBB Enable & activate if appropriate */
    if ( pOpts->pciFBBEnable == TRUE )
    {
    if ( pciAutoConfigFBBEnable(pOpts) == OK )
        {
            pOpts->pciFBBActive = TRUE;
        }
    else
        {
        PCI_AUTO_DEBUG_MSG("pciAutoCfg(): fast back-to-back NOT enabled\n",
            1,2,3,4,5,6);
        }
    }

    return(OK);
    }

/**********************************************************************
*
* pciAutoCfgCtl - set or get pciAutoConfigLib options
*
* pciAutoCfgCtl() can be considered analogous to ioctl()
* calls: the call takes arguments of (1) a pCookie, returned by
* pciAutoConfigLibInit().  (2) A command,
* macros for which are defined in pciAutoConfigLib.h.  And, (3)
* an argument, the type of which depends on the specific command,
* but will always fit in a pointer variable.  Currently, only
* globally effective commands are implemented.
*
* The commands available are:
*
* \is
* \i 'PCI_FBB_ENABLE - BOOL * pArg'
* \i 'PCI_FBB_DISABLE - void'
* \i 'PCI_FBB_UPDATE - BOOL * pArg'
* \i 'PCI_FBB_STATUS_GET - BOOL * pArg'
* Enable and disable the functions which check Fast Back To Back
* functionality.  PCI_FBB_UPDATE is for use with dynamic/HA
* applications.  It first disables FBB on all functions, then enables
* FBB on all functions, if appropriate.  In HA applications, it should
* be called any time a card is added or removed.  The BOOL pointed to 
* by pArg for PCI_FBB_ENABLE and PCI_FBB_UPDATE is set to TRUE if all 
* cards allow FBB functionality and FALSE if either any card does not
* allow FBB functionality or if FBB is disabled.  The BOOL pointed to 
* by pArg for PCI_FBB_STATUS_GET is set to TRUE if PCI_FBB_ENABLE has
* been called and FBB is enabled, even if FBB is not activated on any
* card.  It is set to FALSE otherwise.
*
* NOTE: In the current implementation, FBB is enabled or disabled on
* the entire bus.  If any device anywhere on the bus cannot support
* FBB, then it is not enabled, even if specific sub-busses
* could support it.
*
* \i 'PCI_MAX_LATENCY_FUNC_SET - FUNCPTR * pArg'
* This routine is called for each function present on the bus
* when discovery takes place.  The routine must accept four
* arguments, specifying bus, device, function, and a user-supplied
* argument of type void *.  See PCI_MAX_LATENCY_ARG_SET.  The routine
* should return a UINT8 value, which will be put into the MAX_LAT
* field of the header structure.  The user supplied routine must
* return a valid value each time it is called. There is no mechanism
* for any ERROR condition, but a default value can be returned in
* such a case. Default = NULL.
*
* \i 'PCI_MAX_LATENCY_ARG_SET - void * pArg'
* When the routine specified in PCI_MAX_LATENCY_FUNC_SET is called,
* this will be passed to it as the fourth argument.
*
* \i 'PCI_MAX_LAT_ALL_SET - int pArg'
* Specifies a constant max latency value for all cards, if no
* function has been specified with PCI_MAX_LATENCY_FUNC_SET..
*
* \i 'PCI_MAX_LAT_ALL_GET - UINT * pArg'
* Retrieves the value of max latency for all cards, if no function
* has been specified with PCI_MAX_LATENCY_FUNC_SET. Otherwise, the
* integer pointed to by pArg is set to the value 0xffffffff.
*
* \i 'PCI_MSG_LOG_SET - FUNCPTR * pArg'
* The argument specifies a routine which is called to print warning
* or error messages from pciAutoConfigLib if logMsg() has not been
* initialized at the time pciAutoConfigLib is used. The specified
* routine must accept arguments in the same format as logMsg(), but
* it does not necessarily need to print the actual message. An
* example of this routine is presented below, which saves the message
* into a safe memory space and turns on an LED. This command is useful
* for BSPs which call pciAutoCfg() before message logging is enabled.
* Note that after logMsg() is configured, output goes to logMsg() even
* if this command has been called. Default = NULL.
*
* \cs
* /@ sample PCI_MSG_LOG_SET function @/
* int pciLogMsg(char *fmt,int a1,int a2,int a3,int a4,int a5,int a6)
*     {
*     int charsPrinted;
*
*     sysLedOn(4);
*     charsPrinted = sprintf (sysExcMsg, fmt, a1, a2, a3, a4, a5, a6);
*     sysExcMsg += charsPrinted;
*     return (charsPrinted);
*     }
* \ce
*
* \i 'PCI_MAX_BUS_GET - int * pArg'
* During autoconfiguration, the library maintains a counter with
* the highest numbered bus. This can be retrieved by
* \cs
* pciAutoCfgCtl(pCookie, PCI_MAX_BUS_GET, &maxBus)
* \ce
*
* \i 'PCI_CACHE_SIZE_SET - int pArg'
* Sets the pci cache line size to the specified value.  See
* CONFIGURATION SPACE PARAMETERS in the pciAutoConfigLib
* documentation for more details.
*
* \i 'PCI_CACHE_SIZE_GET - int * pArg'
* Retrieves the value of the pci cache line size.
*
* \i 'PCI_AUTO_INT_ROUTE_SET - BOOL pArg'
* Enables or disables automatic interrupt routing across bridges
* during the autoconfig process.  See "INTERRUPT ROUTING ACROSS
* PCI-TO-PCI BRIDGES" in the pciAutoConfigLib documentation for more
* details.
*
* \i 'PCI_AUTO_INT_ROUTE_GET - BOOL * pArg'
* Retrieves the status of automatic interrupt routing.
*
* \i 'PCI_MEM32_LOC_SET - UINT32 pArg'
* Sets the base address of the PCI 32-bit memory space.  Normally,
* this is given by the BSP constant PCI_MEM_ADRS.
*
* \i 'PCI_MEM32_SIZE_SET - UINT32 pArg'
* Sets the maximum size to use for the PCI 32-bit memory space.
* Normally, this is given by the BSP constant PCI_MEM_SIZE.
*
* \i 'PCI_MEM32_SIZE_GET - UINT32 * pArg'
* After autoconfiguration has been completed, this retrieves
* the actual amount of space which has been used for the
* PCI 32-bit memory space.
*
* \i 'PCI_MEMIO32_LOC_SET - UINT32 pArg'
* Sets the base address of the PCI 32-bit non-prefetch memory space.
* Normally, this is given by the BSP constant PCI_MEMIO_ADRS.
*
* \i 'PCI_MEMIO32_SIZE_SET - UINT32 pArg'
* Sets the maximum size to use for the PCI 32-bit non-prefetch memory
* space.  Normally, this is given by the BSP constant
* PCI_MEMIO_SIZE.
*
* \i 'PCI_MEMIO32_SIZE_GET - UINT32 * pArg'
* After autoconfiguration has been completed, this retrieves
* the actual amount of space which has been used for the PCI
* 32-bit non-prefetch memory space.
*
* \i 'PCI_IO32_LOC_SET - UINT32 pArg'
* Sets the base address of the PCI 32-bit I/O space.
* Normally, this is given by the BSP constant PCI_IO_ADRS.
*
* \i 'PCI_IO32_SIZE_SET - UINT32 pArg'
* Sets the maximum size to use for the PCI 32-bit I/O space.
* Normally, this is given by the BSP constant PCI_IO_SIZE.
*
* \i 'PCI_IO32_SIZE_GET - UINT32 * pArg'
* After autoconfiguration has been completed, this retrieves
* the actual amount of space which has been used for the PCI
* 32-bit I/O space.
*
* \i 'PCI_IO16_LOC_SET - UINT32 pArg'
* Sets the base address of the PCI 16-bit I/O space.
* Normally, this is given by the BSP constant PCI_ISA_IO_ADRS
*
* \i 'PCI_IO16_SIZE_SET - UINT32 pArg'
* Sets the maximum size to use for the PCI 16-bit I/O space.
* Normally, this is given by the BSP constant PCI_ISA_IO_SIZE
*
* \i 'PCI_IO16_SIZE_GET - UINT32 * pArg'
* After autoconfiguration has been completed, this retrieves
* the actual amount of space which has been used for the PCI
* 16-bit I/O space.
*
* \i 'PCI_INCLUDE_FUNC_SET - FUNCPTR * pArg'
* The device inclusion routine is specified by assigning a function
* pointer with the PCI_INCLUDE_FUNC_SET pciAutoCfgCtl() command:
* \cs
* pciAutoCfgCtl(pSystem, PCI_INCLUDE_FUNC_SET,sysPciAutoconfigInclude);
* \ce
* This optional user-supplied routine takes as input both the
* bus-device-function tuple, and a 32-bit quantity containing both
* the PCI vendorID and deviceID of the function.  The function
* prototype for this function is shown below:
* \cs
* STATUS sysPciAutoconfigInclude
*     (
*     PCI_SYSTEM *pSys,
*     PCI_LOC *pLoc,
*     UINT devVend
*     );
* \ce
* This optional user-specified routine is called by PCI AutoConfig
* for each and every function encountered in the scan phase.  The BSP
* developer may use any combination of the input data to ascertain
* whether a device is to be excluded from the autoconfig process.
* The exclusion routine then returns ERROR if a device is to be
* excluded, and OK if a device is to be included in the
* autoconfiguration process.
*
* Note that PCI-to-PCI Bridges may not be excluded, regardless of the
* value returned by the BSP device inclusion routine.  The return
* value is ignored for PCI-to-PCI bridges.
*
* The Bridge device will be always be configured with proper primary,
* secondary, and subordinate bus numbers in the device scanning phase
* and proper I/O and Memory aperture settings in the configuration
* phase of autoconfig regardless of the value returned by the BSP
* device inclusion routine.
*
* \i 'PCI_INT_ASSIGN_FUNC_SET - FUNCPTR * pArg'
* The interrupt assignment routine is specified by assigning a
* function pointer with the PCI_INCLUDE_FUNC_SET pciAutoCfgCtl()
* command:
* \cs
* pciAutoCfgCtl(pCookie, PCI_INT_ASSIGN_FUNC_SET, sysPciAutoconfigIntrAssign);
* \ce
* This optional user-specified routine takes as input both the
* bus-device-function tuple, and an 8-bit quantity containing the
* contents of the interrupt Pin register from the PCI configuration
* header of the device under consideration.  The interrupt pin
* register specifies which of the four PCI Interrupt request lines
* available are connected.  The function prototype for this function
* is shown below:
* \cs
* UCHAR sysPciAutoconfigIntrAssign
*     (
*     PCI_SYSTEM *pSys,
*     PCI_LOC *pLoc,
*     UCHAR pin
*     );
* \ce
*
* This routine may use any combination of these data to ascertain the
* interrupt level.  This value is returned from the function, and
* is programmed into the interrupt line register of the
* function's PCI configuration header. In this manner, device drivers
* may subsequently read this register in order to calculate the
* appropriate interrupt vector which to attach an interrupt service
* routine.
*
* \i 'PCI_BRIDGE_PRE_CONFIG_FUNC_SET - FUNCPTR * pArg'
* The bridge pre-configuration pass initialization routine is
* provided so that the BSP Developer can initialize a bridge device
* prior to the configuration pass on the bus that the bridge
* implements.  This routine is specified by calling pciAutoCfgCtl()
* with the PCI_BRIDGE_PRE_CONFIG_FUNC_SET command:
* \cs
* pciAutoCfgCtl(pCookie, PCI_BRIDGE_PRE_CONFIG_FUNC_SET,
*         sysPciAutoconfigPreEnumBridgeInit);
* \ce
* This optional user-specified routine takes as input both the
* bus-device-function tuple, and a 32-bit quantity containing both
* the PCI deviceID and vendorID of the device.  The function prototype
* for this function is shown below:
* \cs
* STATUS sysPciAutoconfigPreEnumBridgeInit
*     (
*     PCI_SYSTEM *pSys,
*     PCI_LOC *pLoc,
*     UINT devVend
*     );
* \ce
* This routine may use any combination of these input data to
* ascertain any special initialization requirements of a particular
* type of bridge at a specified geographic location.
*
* \i 'PCI_BRIDGE_POST_CONFIG_FUNC_SET - FUNCPTR * pArg'
* The bridge post-configuration pass initialization routine is
* provided so that the BSP Developer can initialize the bridge device
* after the bus that the bridge implements has been enumerated.  This
* routine is specified by calling pciAutoCfgCtl() with the
* PCI_BRIDGE_POST_CONFIG_FUNC_SET command
* \cs
* pciAutoCfgCtl(pCookie, PCI_BRIDGE_POST_CONFIG_FUNC_SET,
*         sysPciAutoconfigPostEnumBridgeInit);
* \ce
* This optional user-specified routine takes as input both the
* bus-device-function tuple, and a 32-bit quantity containing both
* the PCI deviceID and vendorID of the device.  The function prototype
* for this function is shown below:
* \cs
* STATUS sysPciAutoconfigPostEnumBridgeInit
*     (
*     PCI_SYSTEM *pSys,
*     PCI_LOC *pLoc,
*     UINT devVend
*     );
* \ce
* This routine may use any combination of these input data to
* ascertain any special initialization requirements of a particular
* type of bridge at a specified geographic location.
*
* \i 'PCI_ROLLCALL_FUNC_SET - FUNCPTR * pArg'
* The specified routine will be configured as a roll call routine.
*
* If a roll call routine has been configured, before any
* configuration is actually done, the roll call routine is called
* repeatedly until it returns TRUE.  A return value of TRUE indicates
* that either (1) the specified number and type of devices named in
* the roll call list have been found during PCI bus enumeration or
* (2) the timeout has expired without finding all of the specified
* number and type of devices.  In either case, it is assumed that all
* of the PCI devices which are going to appear on the busses have
* appeared and we can proceed with PCI bus configuration.
*
* \i 'PCI_TEMP_SPACE_SET - char * pArg'
* This command is not currently implemented.  It allows the user to
* set aside memory for use during pciAutoConfigLib execution, e.g.
* memory set aside using USER_RESERVED_MEM.  After PCI configuration
* has been completed, the memory can be added to the system memory
* pool using memAddToPool().
*
* \i 'PCI_MINIMIZE_RESOURCES'
* This command is not currently implemented.  It specifies that
* pciAutoConfigLib minimize requirements for memory and I/O space.
*
* \i 'PCI_PSYSTEM_STRUCT_COPY - PCI_SYSTEM * pArg'
* This command has been added for ease of converting from the old
* interface to the new one.  This will set each value as specified in
* the pSystem structure.  If the PCI_SYSTEM structure has already
* been filled, the pciAutoConfig(pSystem) call can be changed
* to:
* \cs
* void *pCookie;
* pCookie = pciAutoConfigLibInit(NULL);
* pciAutoCfgCtl(pCookie, PCI_PSYSTEM_STRUCT_COPY, (void *)pSystem);
* pciAutoCfgFunc(pCookie);
* \ce
* 
* The fields of the PCI_SYSTEM structure are defined below.  For more
* information about each one, see the paragraphs above and the
* documentation for pciAutoConfigLib.
* \is
* \i 'pciMem32'
* Specifies the 32-bit prefetchable memory pool base address. 
* 
* \i 'pciMem32Size'
* Specifies the 32-bit prefetchable memory pool size.
* 
* \i 'pciMemIo32'
* Specifies the 32-bit non-prefetchable memory pool base address.
* 
* \i 'pciMemIo32Size'
* Specifies the 32-bit non-prefetchable memory pool size
* 
* \i 'pciIo32'
* Specifies the 32-bit I/O pool base address. 
* 
* \i 'pciIo32Size'
* Specifies the 32-bit I/O pool size. 
* 
* \i 'pciIo16'
* Specifies the 16-bit I/O pool base address. 
* 
* \i 'pciIo16Size'
* Specifies the 16-bit I/O pool size. 
* 
* \i 'includeRtn'
* Specifies the device inclusion routine.
* 
* \i 'intAssignRtn'
* Specifies the interrupt assignment routine.
* 
* \i 'autoIntRouting'
* Can be set to TRUE to configure pciAutoConfig() only to
* call the BSP interrupt routing routine for devices on bus number 0.
* Setting autoIntRoutine to FALSE will configure pciAutoConfig()
* to call the BSP interrupt routing routine for every device
* regardless of the bus on which the device resides.
* 
* \i 'bridgePreInit'
* Specifies the bridge initialization routine to call
* before initializing devices on the bus that the bridge
* implements.
* 
* \i 'bridgePostInit'
* Specifies the bridge initialization routine to call
* after initializing devices on the bus that the bridge
* implements.
* \ie
* \ie
* 
* RETURNS: OK, or ERROR if the command or argument is invalid.
*
* ERRNO
* \is
* \i EINVAL 
* if pCookie is not NULL or cmd is not recognized
* \ie
*
*/

STATUS pciAutoCfgCtl
    (
    void *      pCookie,          /* system configuration information */
    int         cmd,              /* command word */
    void *      pArg              /* argument for the cmd */
    )
    {
    PCI_SYSTEM *        pSystem;
    PCI_AUTO_CONFIG_OPTS *  pOpts;

    if ( pCookie == NULL )
        {
        errnoSet(EINVAL);
        return(ERROR);
        }
    pOpts = (PCI_AUTO_CONFIG_OPTS *)pCookie;

    switch (cmd)
        {
        case PCI_PSYSTEM_STRUCT_COPY:
            /* copy from pSystem to pOpt */
            if ( pArg == NULL )
                {
                errnoSet(EINVAL);
                return(ERROR);
                }
            pSystem = (PCI_SYSTEM *)pArg;
            pOpts->pciMem32 = pSystem->pciMem32;
            pOpts->pciMem32Size = pSystem->pciMem32Size;
            pOpts->pciMemIo32 = pSystem->pciMemIo32;
            pOpts->pciMemIo32Size = pSystem->pciMemIo32Size;
            pOpts->pciIo32 = pSystem->pciIo32;
            pOpts->pciIo32Size = pSystem->pciIo32Size;
            pOpts->pciIo16 = pSystem->pciIo16;
            pOpts->pciIo16Size = pSystem->pciIo16Size;
            pOpts->maxBus = pSystem->maxBus;
            pOpts->cacheSize = pSystem->cacheSize;
            pOpts->maxLatency = pSystem->maxLatency;
            pOpts->autoIntRouting = pSystem->autoIntRouting;
            pOpts->includeRtn = pSystem->includeRtn;
            pOpts->intAssignRtn = (PCI_INT_ASSIGN_FUNC)pSystem->intAssignRtn;
            pOpts->bridgePreConfigInit = pSystem->bridgePreConfigInit;
            pOpts->bridgePostConfigInit = pSystem->bridgePostConfigInit;
            pOpts->pciRollcallRtn = pSystem->pciRollcallRtn;
            break;

        case PCI_MINIMIZE_RESOURCES:
            pOpts->minimizeResources = (BOOL)pArg;
            break;

        case PCI_FBB_ENABLE:
            pOpts->pciFBBEnable = TRUE;
            pOpts->pciFBBActive = FALSE;
            if ( pciAutoConfigFBBEnable(pOpts) == OK )
                {
                pOpts->pciFBBActive = TRUE;
                }
            if ( pArg != NULL )
                {
                (*(BOOL *)pArg) = pOpts->pciFBBActive;
                }
            break;

        case PCI_FBB_DISABLE:
            if ( ( pOpts->pciFBBEnable == TRUE ) || ( pOpts->pciFBBActive == TRUE ) )
                {
                pciAutoConfigFBBDisable(pOpts);
                pOpts->pciFBBActive = FALSE;
                }
            pOpts->pciFBBEnable = FALSE;
            break;

        case PCI_FBB_UPDATE:
            if ( ( pOpts->pciConfigInit == TRUE ) &&
                 ( pOpts->pciFBBEnable == TRUE ) )
                {
                pOpts->pciFBBActive = pciAutoConfigFBBEnable(pOpts);
                }
            if ( pArg != NULL )
                {
                (*(BOOL *)pArg) = pOpts->pciFBBActive;
                }
            break;

        case PCI_FBB_STATUS_GET:
            (*(BOOL *)pArg) = pOpts->pciFBBEnable;
            break;

        case PCI_MAX_LATENCY_FUNC_SET:
            pOpts->pciMaxLatFunc = (PCI_MAX_LAT_FUNC)pArg;
            break;

        case PCI_MAX_LATENCY_ARG_SET:
            pOpts->pciMaxLatPArg = pArg;
            break;

        case PCI_MSG_LOG_SET:
            pOpts->pciLogMsgFunc = (FUNCPTR)pArg;
            break;

        case PCI_MAX_BUS_SET:
            pOpts->maxBus = (int)pArg;
            break;

        case PCI_MAX_BUS_GET:
            (*(int *)pArg) = pOpts->maxBus;
            break;

        case PCI_CACHE_SIZE_SET:
            pOpts->cacheSize = (int)pArg;
            break;

        case PCI_CACHE_SIZE_GET:
            *(int *)pArg = pOpts->cacheSize;
            break;

        case PCI_MAX_LAT_ALL_SET:
            pOpts->maxLatency = (UINT)pArg;
            break;

        case PCI_MAX_LAT_ALL_GET:
            if ( pOpts->pciMaxLatFunc == NULL )
                *(UINT *)pArg = pOpts->maxLatency;
            else
                *(UINT *)pArg = 0xffffffff;
            break;

        case PCI_AUTO_INT_ROUTE_SET:
            pOpts->autoIntRouting = (BOOL)pArg;
            break;

        case PCI_AUTO_INT_ROUTE_GET:
            *(BOOL *)pArg = pOpts->autoIntRouting;
            break;

        case PCI_MEM32_LOC_SET:
            pOpts->pciMem32 = (UINT)pArg;
            break;

        case PCI_MEM32_SIZE_SET:
            pOpts->pciMem32Size = (UINT)pArg;
            break;

        case PCI_MEM32_SIZE_GET:
            (*(UINT32 *)pArg) = pOpts->pciMem32Used;
            break;

        case PCI_MEMIO32_LOC_SET:
            pOpts->pciMemIo32 = (UINT)pArg;
            break;

        case PCI_MEMIO32_SIZE_SET:
            pOpts->pciMemIo32Size = (UINT)pArg;
            break;

        case PCI_MEMIO32_SIZE_GET:
            *(UINT *)pArg = pOpts->pciMemIo32Used;
            break;

        case PCI_IO32_LOC_SET:
            pOpts->pciIo32 = (UINT)pArg;
            break;

        case PCI_IO32_SIZE_SET:
            pOpts->pciIo32Size = (UINT)pArg;
            break;

        case PCI_IO32_SIZE_GET:
            *(UINT *)pArg = pOpts->pciIo32Used;
            break;

        case PCI_IO16_LOC_SET:
            pOpts->pciIo16 = (UINT)pArg;
            break;

        case PCI_IO16_SIZE_SET:
            pOpts->pciIo16Size = (UINT)pArg;
            break;

        case PCI_IO16_SIZE_GET:
            *(UINT *)pArg = pOpts->pciIo16Used;
            break;

        case PCI_INCLUDE_FUNC_SET:
            pOpts->includeRtn = (PCI_INCLUDE_FUNC)pArg;
            break;

        case PCI_INT_ASSIGN_FUNC_SET:
            pOpts->intAssignRtn = (PCI_INT_ASSIGN_FUNC)pArg;
            break;

        case PCI_BRIDGE_PRE_CONFIG_FUNC_SET:
            pOpts->bridgePreConfigInit = (PCI_BRIDGE_PRE_CONFIG_FUNC)pArg;
            break;

        case PCI_BRIDGE_POST_CONFIG_FUNC_SET:
            pOpts->bridgePostConfigInit = (PCI_BRIDGE_POST_CONFIG_FUNC)pArg;
            break;

        case PCI_ROLLCALL_FUNC_SET:
            pOpts->pciRollcallRtn = (FUNCPTR)pArg;
            break;

        case PCI_TEMP_SPACE_SET:
            pOpts->pFuncList = ((PCI_MEM_PTR *)pArg)->pMem;
            pOpts->numFuncListEntries = ((PCI_MEM_PTR *)pArg)->memSize / sizeof(PCI_LOC);
            break;

        default:
            errnoSet(EINVAL);
            return(ERROR);
        }

    return(OK);
    }

#ifdef USE_PCI_SIMULATOR
void pciAutoConfigListShow(PCI_LOC *pLoc, int num)
    {
    while ( num >= 0 )
        {
        PCI_AUTO_DEBUG_MSG("[%d,%d,%d] bar%d 0x%08x in %s space\n",
                           pLoc->bus, pLoc->device, pLoc->function, 4,5,6);
        pLoc++;
        num--;
        }
    }
#endif /* USE_PCI_SIMULATOR */

/****************************************************************
*
* pciAutoCfgFunc - the actual guts of pciAutoCfg
*
* The functions pciAutoConfig() and pciAutoCfg() are both
* wrapper functions to go around this function.  The actual
* work is done by this function.
*
* ALGORITHM
* Probe PCI config space and create a list of available PCI functions.
* Call device exclusion function, if registered, to exclude/include device.
* Disable all devices before we initialize any.
* Allocate and assign PCI space to each device.
* Calculate and set interrupt line value.
* Initialize and enable each device.
*
* RETURNS: OK, or ERROR if pCookie is not valid.
*
* ERRNO
*
*/

LOCAL STATUS pciAutoCfgFunc
    (
    void *pCookie       /* cookie returned by pciAutoConfigLibInit() */
    )
    {
    PCI_AUTO_CONFIG_OPTS *  pSystem; /* named for backward compatibility */

    PCI_LOC* pPciList;      /* Pointer to PCI include list  */
    int listSize;       /* Size of PCI include list */
    BOOL rollcallSuccess;   /* has pciRollcallRtn() succeeded? */

    /* Input parameter sanity checking */

    if (pCookie == NULL)
        {
        errnoSet(EINVAL);
        return(ERROR);
        }

    pSystem = (PCI_AUTO_CONFIG_OPTS *)pCookie;

    /*
     * If a roll-call routine has been configured, call the roll-call
     * routine repeatedly until it returns TRUE.  A return value
     * of TRUE indicates that either (1) the specified number and
     * type of devices named in the roll call list have been found
     * during PCI bus enumeration or (2) the timeout has expired
     * without finding all of the specified number and type of
     * devices.  In either case, we will assume that all of the PCI
     * devices which are going to appear on the busses have appeared
     * and we can proceed with PCI bus configuration.
     */

    if (pSystem->pciRollcallRtn != NULL)
        {
        rollcallSuccess = FALSE;
        while ( ! rollcallSuccess )
            {

            /*
             * Probe all PCI busses dynamically creating a function list
             * of all functions found.  Excluded devices are skipped over.
             */

            pPciList = pciAutoListCreate (pSystem, &listSize);

            /* Perform roll call function, if we pass, exit the loop */

            if ( (*pSystem->pciRollcallRtn)() == OK )
                rollcallSuccess = TRUE;

#ifdef PCI_AUTO_RECLAIM_LIST
            free(pPciList);
#endif /* PCI_AUTO_RECLAIM_LIST */

            if ( rollcallSuccess == TRUE )
                break;
            }
        }

    /*
     * Probe all PCI busses dynamically creating a function list
     * of all functions found.  Excluded devices are skipped over.
     */

    pPciList = pciAutoListCreate (pSystem, &listSize);

    pciAutoFuncConfigAll (pSystem, pPciList, listSize);

    lastPciListSize = listSize;
    pLastPciList = pPciList;

    /* If the function list is malloc'ed at runtime, then release it */

#if defined(PCI_AUTO_RECLAIM_LIST)
#if defined(PCI_AUTO_STATIC_LIST)
#error "Can't do PCI_AUTO_RECLAIM_LIST with PCI_AUTO_STATIC_LIST"
#endif
    free(pPciList);

    lastPciListSize = 0;
    pLastPciList = NULL;
#endif

    return(OK);
    }

/******************************************************************************
*
* pciAutoListCreate - probe for all functions and make a PCI probe list
*
* This routine creates a dynamic probelist containing all PCI functions
* located in the PCI configuration hierarchy. In addition, it assigns
* base addresses for the 32-bit prefetchable memory allocation pool,
* the 32-bit non-prefetchable memory allocation pool, the 32-bit I/O
* allocation pool, and the 16-bit I/O allocation pool. When I/O space
* or memory space is actually assigned (not in this routine), the space
* allocation will begin with the base address in each of these categories
* and proceed to higher numbered addresses.
*
* Note that 20-bit memory space is not currently handled as a special case.
*
* RETURNS: A pointer to a newly populated PCI device list.
*
* ERRNO
*
*/

LOCAL PCI_LOC * pciAutoListCreate
    (
    PCI_AUTO_CONFIG_OPTS * pSystem,     /* cookie returned by pciAutoConfigLibInit() */
    int *pListSize          /* size of the PCI_LOC list */
    )
    {
    PCI_LOC  pciLoc;        /* PCI bus/device/function structure */
    PCI_LOC *pPciList;
    PCI_LOC *pRetPciList;
    UCHAR    busAttr;

    /* Initialize the list pointer in preparation for probing */

#if defined(PCI_AUTO_STATIC_LIST)
    pPciList = pciAutoLocalFuncList;
    pRetPciList = pPciList;
#else
    pPciList = malloc(sizeof(PCI_LOC) *  PCI_AUTO_MAX_FUNCTIONS);
    if (pPciList == NULL)
        {
        return NULL;
        }
    pRetPciList = pPciList;
#endif

    lastPciListSize = 0;
    *pListSize = 0;

    /* Begin the scanning process at [0,0,0] */
    if (currentPcieController == PCIE_CONTROLLER_0) {
        pciLoc.bus = (UINT8)1;
    } else {
        pciLoc.bus = (UINT8)PCIE1_BUS_MIN;
    }
    pciLoc.device = (UINT8)0;
    pciLoc.function = (UINT8)0;

    /*
     * Note that the host bridge is assumed to support prefetchable memory
     * (PCI_AUTO_ATTR_BUS_PREFETCH) and only assumed to support 32-bit I/O
     * addressing (PCI_AUTO_ATTR_BUS_4GB_IO) if pciIo32Size is non-zero.
     */

    if (pSystem->pciIo32Size == 0)
        busAttr = PCI_AUTO_ATTR_BUS_PREFETCH;
    else
        busAttr = PCI_AUTO_ATTR_BUS_4GB_IO | PCI_AUTO_ATTR_BUS_PREFETCH;


    if (currentPcieController == PCIE_CONTROLLER_0) {
        pci0MaxBus = pciAutoDevProbe (pSystem, pciLoc.bus, (UCHAR)0,
                                     busAttr, &pPciList, pListSize);
        pSystem->maxBus = pci0MaxBus;
    } else {
        pci1MaxBus = pciAutoDevProbe (pSystem, pciLoc.bus, (UCHAR)0,
                                 busAttr, &pPciList, pListSize);
        pSystem->maxBus = pci1MaxBus;
    }

    return(pRetPciList);
    }

/******************************************************************************
*
* pciAutoDevReset - quiesce a PCI device and reset all writeable status bits
*
* This routine turns 'off' a PCI device by disabling the Memory decoders, I/O
* decoders, and Bus Master capability.  The routine also resets all writeable
* status bits in the status word that follows the command word sequentially
* in PCI config space by performing a longword access.
*
* RETURNS: OK, always.
*
* ERRNO
*
*/

STATUS pciAutoDevReset
    (
    PCI_LOC * pPciLoc   /* device to be reset */
    )
    {
    /* Disable the device and reset all writeable status bits */

    pciConfigOutLong (pPciLoc->bus, pPciLoc->device,
                      pPciLoc->function,
                      PCI_CFG_COMMAND, 0xffff0000);
    return OK;
    }

/******************************************************************************
*
* pciAutoBusNumberSet - set the primary, secondary, and subordinate bus number
*
* This routine sets the primary, secondary, and subordinate bus numbers for
* a device that implements the Type 1 PCI Configuration Space Header.
*
* This routine has external visibility to enable it to be used by BSP
* Developers for initialization of PCI Host Bridges that may implement
* registers similar to those found in the Type 1 Header.
*
* RETURNS: OK, always
*
* ERRNO
*
*/

STATUS pciAutoBusNumberSet
    (
    PCI_LOC * pPciLoc,      /* device affected */
    UINT primary,       /* primary bus specification */
    UINT secondary,     /* secondary bus specification */
    UINT subordinate        /* subordinate bus specification */
    )
    {
    UINT workvar = 0;   /* Working variable     */

    workvar = (subordinate << 16) + (secondary << 8) + primary;

    /* longword write */

    pciConfigModifyLong (pPciLoc->bus,
                         pPciLoc->device,
                         pPciLoc->function,
                         PCI_CFG_PRIMARY_BUS,
                         0x00ffffff,
                         workvar);

    return OK;
    }

/******************************************************************************
*
* pciAutoBusProbe - configure a bridge and probe all devices behind it
*
* This routine assigns an initial range of subordinate busses to a
* PCI bridge, searches for functions under this bridge, and then
* updates the range assignment to the correct value.  It calls
* pciAutoDevProbe() which in turn calls this function in a recursive
* manner.  In addition to actually programming the PCI-PCI bridge
* headers with correct bus numbers, the 'pciLoc' list of functions
* is extended as each new PCI function is found.
*
* RETURNS: The subordinate bus number.
*
* ERRNO
*
*/

LOCAL UINT pciAutoBusProbe
    (
    PCI_AUTO_CONFIG_OPTS * pSystem, /* PCI system information   */
    UINT priBus,        /* Primary PCI bus      */
    UINT secBus,        /* Secondary PCI bus        */
    PCI_LOC*  pPciLoc,      /* PCI address of this bridge   */
    PCI_LOC** ppPciList,    /* Pointer to next PCI location */
                /* entry pointer        */
    int * pListSize     /* number of PCI_LOC entries    */
    )
    {
    UINT subBus = 0xff; /* Highest subordinate PCI bus  */
    UCHAR offset = 0;   /* Interrupt routing offset for this bus*/

    /* Disable I/O, Mem, and upstream transactions / reset status bits */

    pciAutoDevReset (pPciLoc);

    /* Set the bus numbers, subordinate bus is 0xff */

    pciAutoBusNumberSet (pPciLoc, priBus, secBus, 0xff);

    PCI_AUTO_DEBUG_MSG("pciAutoBusProbe: using bridge [%d,%d,%d,0x%02x]\n",
                       (pPciLoc->bus),
                       (pPciLoc->device),
                       (pPciLoc->function),
                       (pPciLoc->attribute),
                       0,
                       0
                      );

    /* Probe all devices on this bus */

    PCI_AUTO_DEBUG_MSG("pciAutoBusProbe: calling pciAutoDevProbe on bus [%d]\n",
                       secBus, 0, 0, 0, 0, 0);

    /*
     * Compute the route offset for this bridge: if this bridge is
     * on bus zero (primary bus = zero) then its contribution is null,
     * otherwise it depends on its location on the bus and on the
     * contribution of all of the upper Pci-To-Pci Bridges.
     */

    pPciLoc->offset += (priBus > 0) ? (pPciLoc->device % 4) : 0;
    offset = pPciLoc->offset;

    PCI_AUTO_DEBUG_MSG("pciAutoBusProbe: int route offset for bridge is [%d]\n",
                       offset, 0, 0, 0, 0, 0);

    subBus = pciAutoDevProbe (pSystem, secBus, offset, (pPciLoc->attribute),
                              ppPciList, pListSize);

    /* Set the range assignment to cover the correct range of busses */

    PCI_AUTO_DEBUG_MSG("pciAutoBusProbe: post-config subordinate bus as [%d]\n",
                       subBus, 0, 0, 0, 0, 0);

    pciAutoBusNumberSet (pPciLoc, priBus, secBus, subBus);

    /* Return the highest subordinate bus */

    return subBus;

    }

/******************************************************************************
*
* pciAutoDevProbe - probe all devices on a single PCI bus
*
* This routine probes a single PCI bus and adds each detected PCI function
* to the function list.  In case a PCI-PCI bridge is found, pciAutoBusProbe()
* is called to probe that bus.  pciAutoBusProbe() in turn calls this function
* in a recursive manner until all busses have been probed.
*
* RETURNS: Highest subordinate bus number found during probing process.
*
* ERRNO
*
*/

LOCAL UINT pciAutoDevProbe
    (
    PCI_AUTO_CONFIG_OPTS * pSystem, /* PCI system info          */
    UINT bus,           /* current bus number to probe      */
    UCHAR offset,       /* bridge contrib to int vector xlation */
    UCHAR inheritAttrib,    /* attributes inherited from bridge     */
    PCI_LOC **ppPciList,    /* Pointer to next PCI location entry   */
    int * pListSize     /* Number of PCI_LOC's currently in list*/
    )
    {
    PCI_LOC pciLoc;     /* PCI bus/device/function structure    */
    UINT16 pciclass;        /* PCI class/subclass contents      */
    UINT dev_vend;      /* Device/Vendor identifier     */
    int device;         /* Device location          */
    int function;       /* Function location            */
    int subBus;         /* Highest subordinate PCI bus      */
    UCHAR btemp;        /* Temporary holding area       */
    UINT temp;

    /* Initialize variables */

    bzero ((char *)&pciLoc, sizeof (PCI_LOC));
    pciLoc.bus = bus;

    subBus = bus;

    /* if attributes indicate a host bus, then set equal to pciLoc.attrib */

    /* Locate each active function on the current bus */

    for (device = 0; device < PCI_MAX_DEV; device++)
        {
        pciLoc.device = device;

        /* Check each function until an unused one is detected */

        for (function = 0; function < PCI_MAX_FUNC; function++)
            {
            pciLoc.function = function;

            /* Check for a valid device/vendor number */

            pciConfigInLong (pciLoc.bus, pciLoc.device, pciLoc.function,
                             PCI_CFG_VENDOR_ID, &dev_vend);

            /* If function 0 then check next dev else check next function */

            if ( ((dev_vend & 0x0000ffff) == PCI_CONFIG_ABSENT_F) ||
                 ((dev_vend & 0x0000ffff) == PCI_CONFIG_ABSENT_0) )
                {
                if (function == 0)
                    {
                    break;  /* non-existent device, goto next device */
                    }
                else
                    {
                    continue;  /* function empty, try the next function */
                    }
                }

            /* store the translation offset for the int routing */

            pciLoc.offset = offset;
            pciLoc.attribute = 0;

            /* Check to see if this function belongs to a PCI-PCI bridge */

            pciConfigInWord (pciLoc.bus, pciLoc.device, pciLoc.function,
                             PCI_CFG_SUBCLASS, &pciclass);
            /* Set Bridge device attributes for this device */

            switch (pciclass)
                {

                /* PCI Host Bridge */
                
                case ((PCI_CLASS_BRIDGE_CTLR << 8) + PCI_SUBCLASS_HOST_PCI_BRIDGE):
                    pciLoc.attribute |= ( PCI_AUTO_ATTR_DEV_EXCLUDE |
                                          PCI_AUTO_ATTR_BUS_HOST );
                    break;

                    /* ISA Bridge */

                case ((PCI_CLASS_BRIDGE_CTLR << 8) + PCI_SUBCLASS_ISA_BRIDGE):
                    pciLoc.attribute |= PCI_AUTO_ATTR_BUS_ISA;

                    break;

                    /* Display Device */




                case (PCI_CLASS_DISPLAY_CTLR << 8):
                    pciLoc.attribute |= PCI_AUTO_ATTR_DEV_DISPLAY;

                    /* Mask off all but bus attribute bits to inherit */ 

                    inheritAttrib &=   ( PCI_AUTO_ATTR_BUS_4GB_IO | 
                                         PCI_AUTO_ATTR_BUS_PREFETCH ); 
                    /* devices inherit bus attributes from their bridge */ 

                    pciLoc.attribute |= inheritAttrib; 

                    PCI_AUTO_DEBUG_MSG("pciAutoDevProbe: inheriting attribute 0x%x to\
                                        local attribute 0x%x (bus %d)\n",
                                        inheritAttrib, pciLoc.attribute, pciLoc.bus, 0, 0, 0); 

                    break;

                    /* PCI-to-CardBus bridge */

                case ((PCI_CLASS_BRIDGE_CTLR << 8) + PCI_SUBCLASS_CARDBUS_BRIDGE):
                    /* Fall through */

                    /* PCI-to-PCI Bridge */

                case ((PCI_CLASS_BRIDGE_CTLR << 8) + PCI_SUBCLASS_P2P_BRIDGE):

                    /* Setup and probe this bridge device */

                    pciLoc.attribute |= PCI_AUTO_ATTR_BUS_PCI;

                    /*
                     * Check for 32 bit I/O addressability,
                     * but only if the parent bridge supports it
                     */

                    if (inheritAttrib & PCI_AUTO_ATTR_BUS_4GB_IO)
                        {
                        pciConfigInByte (pciLoc.bus,
                                         pciLoc.device,
                                         pciLoc.function,
                                         PCI_CFG_IO_BASE, &btemp);

                        if ((btemp & 0x0F) == 0x01)
                            {
                            pciConfigInByte (pciLoc.bus,
                                             pciLoc.device, pciLoc.function,
                                             PCI_CFG_IO_LIMIT, &btemp);
                            if ((btemp & 0x0F) == 0x01)
                                {
                                pciLoc.attribute |= PCI_AUTO_ATTR_BUS_4GB_IO;
                                PCI_AUTO_DEBUG_MSG("pciAutoDevProbe: 4G I/O \n",
                                                   0, 0, 0, 0, 0, 0);
                                }
                            }
                        }

                    /* Disable prefetch */

                    pciConfigModifyLong (pciLoc.bus,
                                         pciLoc.device,
                                         pciLoc.function,
                                         PCI_CFG_PRE_MEM_BASE,
                                         0xfff0fff0, 0x0000fff0);

                    pciConfigOutLong (pciLoc.bus,
                                      pciLoc.device,
                                      pciLoc.function,
                                      PCI_CFG_PRE_MEM_LIMIT_U, 0);

                    pciConfigOutLong (pciLoc.bus,
                                      pciLoc.device,
                                      pciLoc.function,
                                      PCI_CFG_PRE_MEM_BASE_U, 0xffffffff);

                    /* Check for Prefetch memory support */

                    if (inheritAttrib & PCI_AUTO_ATTR_BUS_PREFETCH)
                        {
                        pciConfigInLong  (pciLoc.bus,
                                          pciLoc.device,
                                          pciLoc.function,
                                          PCI_CFG_PRE_MEM_BASE, &temp);

                        /* PF Registers return 0 if PF is not implemented */

                        if (temp != 0)
                            {
                            pciLoc.attribute |= PCI_AUTO_ATTR_BUS_PREFETCH;
                            PCI_AUTO_DEBUG_MSG("pciAutoDevProbe: PF present\n",
                                               0, 0, 0, 0, 0, 0);
                            }
                        }
                    break;

                default:

                    /* Mask off all but bus attribute bits to inherit */

                    inheritAttrib &=   ( PCI_AUTO_ATTR_BUS_4GB_IO |
                                         PCI_AUTO_ATTR_BUS_PREFETCH );

                    /* devices inherit bus attributes from their bridge */

                    pciLoc.attribute |= inheritAttrib;

                    break;
                }

            /* Add this function to the PCI function list */

            if (*pListSize < PCI_AUTO_MAX_FUNCTIONS)
                {
                memcpy (*ppPciList, &pciLoc, sizeof (PCI_LOC));
                (*ppPciList)++;
                (*pListSize)++;
                }

            /* If the device is a PCI-to-PCI bridge, then scan behind it */
            if (pciLoc.attribute & PCI_AUTO_ATTR_BUS_PCI)
                {
                PCI_AUTO_DEBUG_MSG("pciAutoDevProbe: scanning bus[%d]\n",
                                   (subBus+1), 0, 0, 0, 0, 0 );
                subBus = pciAutoBusProbe (pSystem, bus, subBus+1, &pciLoc,
                                          ppPciList, pListSize);
                }

            /* Proceed to next device if this is a single function device */

            if (function == 0)
                {
                pciConfigInByte (pciLoc.bus, pciLoc.device, pciLoc.function,
                                 PCI_CFG_HEADER_TYPE, &btemp);
                if ((btemp & PCI_HEADER_MULTI_FUNC) == 0)
                    {
                    break; /* No more functions - proceed to next PCI device */
                    }
                }
            }
        }

    return(subBus);
    }

/******************************************************************************
*
* pciAutoFuncConfigAll - configure all PCI functions contained in list
*
* This routine initializes all PCI functions within the specified
* list.  This may be anything from a full list to a single entry.
*
* RETURNS: N/A
*
* ERRNO
*
*/

LOCAL void pciAutoFuncConfigAll
    (
    PCI_AUTO_CONFIG_OPTS * pSystem, /* PCI system info */
    PCI_LOC *pPciList,              /* input: Pointer to first function   */
    UINT nSize                      /* input: Number of functions to init */
    )
    {
    PCI_LOC *pPciFunc;  /* Pointer to next function */
    UINT nLoop;         /* Loop control variable    */
    UINT nEnd;          /* End of function list     */

    /* Disable all devices before we initialize any */

    /* Allocate and assign PCI space to each device */

    pPciFunc = pPciList;
    nEnd = nSize;

    pciAutoDevConfig (pSystem, pPciList->bus, &pPciFunc, &nEnd);


    /* Enable each device on the device list */

    pPciFunc = pPciList;
    for (nLoop = 0; nLoop < nSize; nLoop++)
        {
        pciAutoFuncEnable ((PCI_SYSTEM *)pSystem, pPciFunc);
        pPciFunc++;
        }

    }

/******************************************************************************
*
* pciAutoFuncDisable - disable a specific PCI function
*
* This routine clears the I/O, mem, master, & ROM space enable bits
* for a single PCI function.
*
* The PCI spec says that devices should normally clear these by default after
* reset but in actual practice, some PCI devices do not fully comply.  This
* routine ensures that the devices have all been disabled before configuration
* is started.
*
* RETURNS: N/A
*
* ERRNO
*
*/

void pciAutoFuncDisable
    (
    PCI_LOC *pPciFunc       /* input: Pointer to PCI function struct */
    )
    {
    UCHAR cTemp;            /* Temporary storage */
    UINT16 wTemp;

    if ((pPciFunc->attribute) & PCI_AUTO_ATTR_DEV_EXCLUDE)
        {
        return;
        }

    PCI_AUTO_DEBUG_MSG("pciAutoFuncDisable: disable device [%d,%d,%d,0x%02x]\n",
                       (pPciFunc->bus),
                       (pPciFunc->device),
                       (pPciFunc->function),
                       (pPciFunc->attribute),
                       0,
                       0
                      );

    /* Disable Memory, I/O, and Bus Mastering, save status bits */

    wTemp = (PCI_CMD_IO_ENABLE | PCI_CMD_MEM_ENABLE | PCI_CMD_MASTER_ENABLE );

    pciConfigModifyLong (pPciFunc->bus, pPciFunc->device, pPciFunc->function,
                         PCI_CFG_COMMAND, (0xffff0000 | wTemp), 0x0);

    /* Disable header dependent fields */

    pciConfigInByte (pPciFunc->bus, pPciFunc->device, pPciFunc->function,
                     PCI_CFG_HEADER_TYPE, &cTemp);
    cTemp &= PCI_HEADER_TYPE_MASK;

    switch (cTemp)
        {
        case PCI_HEADER_TYPE0:  /* non PCI-PCI bridge */

            /*
             * Disable Expansion ROM address decode for the device.
             * Note that no mem space is allocated for the Expansion
             * ROM, so a re-enable later should NOT be done.
             */

            pciConfigModifyLong (pPciFunc->bus, pPciFunc->device,
                                 pPciFunc->function, PCI_CFG_EXPANSION_ROM,
                                 0x1, 0);
            break;

        case PCI_HEADER_PCI_PCI:    /* PCI-PCI bridge */

            pciConfigModifyLong (pPciFunc->bus, pPciFunc->device,
                                 pPciFunc->function, PCI_CFG_ROM_BASE,
                                 0x1, 0);
            break;

        default:
            break;
        }

    return;
    }

/******************************************************************************
*
* pciAutoFuncEnable - perform final configuration and enable a function
*
* Depending upon whether the device is included, this routine initializes
* a single PCI function as follows:
*
* Initialize the cache line size register
* Initialize the PCI-PCI bridge latency timers
* Enable the master PCI bit for non-display devices
* Set the interrupt line value with the value from the BSP.
*
* RETURNS: N/A
*
* ERRNO
*
*/

void pciAutoFuncEnable
    (
    PCI_SYSTEM * pSys,    /* for backwards compatibility */
    PCI_LOC * pFunc       /* input: Pointer to PCI function structure */
    )
    {
    PCI_AUTO_CONFIG_OPTS * pSystem; /* PCI system info */
    UINT16 pciClass;      /* PCI class/subclass contents */
    UCHAR intLine = 0xff; /* Interrupt "Line" value           */
    UINT8 maxLatency;     /* MAX_LAT value for this function */

    /*
    * automatically call pciAutoConfigLibInit() for
    * backwards compatibility if pciAutoConfig() was
    * used.
    */
    if ( pPciCfgOpts->pciConfigInit == TRUE )
        pSystem = pPciCfgOpts;
    else
        {
        pSystem = pciAutoConfigLibInit(NULL);
        pciAutoCfgCtl(pSystem, PCI_PSYSTEM_STRUCT_COPY, pSys);
        }

    if ((pFunc->attribute) & PCI_AUTO_ATTR_DEV_EXCLUDE)
        {
        return;
        }

    PCI_AUTO_DEBUG_MSG("pciAutoFuncConfig: enable device [%d,%d,%d,0x%02x]\n",
                       (pFunc->bus),
                       (pFunc->device),
                       (pFunc->function),
                       (pFunc->attribute),
                       0,
                       0
                      );

    /* Initialize the cache line size register */

    pciConfigOutByte (pFunc->bus, pFunc->device, pFunc->function,
                      PCI_CFG_CACHE_LINE_SIZE, pSystem->cacheSize);

    /* Initialize the latency timer */

    /*
    *  If MAX_LAT function supplied by BSP, call it to calculate
    *  the value of MAX_LAT for each function individually.
    *  Otherwise, use the default value for all functions.
    */
    if ( ( pPciCfgOpts->pciConfigInit == TRUE ) &&
         ( pPciCfgOpts->pciMaxLatFunc != NULL ) )
        {
        /* find value by calling user-supplied function */
        maxLatency = (*pPciCfgOpts->pciMaxLatFunc)
                     (pFunc->bus,pFunc->device,pFunc->function,
                      pPciCfgOpts->pciMaxLatPArg);
        }
    else
        {
        /* use default value */
        maxLatency = pSystem->maxLatency;
        }

    pciConfigOutByte (pFunc->bus, pFunc->device, pFunc->function,
                      PCI_CFG_LATENCY_TIMER, maxLatency);


    /* Get the PCI class code */

    pciConfigInWord (pFunc->bus, pFunc->device, pFunc->function,
                     PCI_CFG_SUBCLASS, &pciClass);

    /* Enable Bus Mastering (preserve status bits) */

    pciConfigModifyLong (pFunc->bus, pFunc->device, pFunc->function,
                         PCI_CFG_COMMAND,
                         (0xffff0000 | PCI_CMD_MASTER_ENABLE),
                         PCI_CMD_MASTER_ENABLE);

    /*
     * If an interrupt assignment routine is registered, assign the
     * interrupt and record interrupt line in config header, else
     * write 0xff (default value) to the interrupt line reg
     */

    intLine = pciAutoIntAssign (pSystem, pFunc);

    pciConfigOutByte (pFunc->bus, pFunc->device, pFunc->function,
                      PCI_CFG_DEV_INT_LINE, intLine);

    /* Reset all writeable status bits */

    pciConfigOutWord (pFunc->bus, pFunc->device, pFunc->function,
                      PCI_CFG_STATUS, (UINT16)0xFFFF);

    /* copy back to PCI_SYSTEM struct for backward compatibility */
    pciAutoConfigCopyback(pSystem, pSys);
    return;
    }

/******************************************************************************
*
* pciAutoIntAssign - compute the interrupt number for a given PCI device
*
* This routine computes the interrupt number for a PCI device identified
* by the parameter <pFunc>.
*
* The algorithm used is the following: if the device is on the PCI bus 0,
* then the bsp interrupt assign routine (if any) is called to get the
* actual IRQ number used.  In addition, if the device is a Pci-To-Pci
* Bridge, then this routine populates a routing table that will be used
* later for all of the PCI devices on its every subordinate buses.
* Conversely, if the device sits on any PCI bus other than bus 0, this
* routines only looks at that table.  The index used depends not
* only on the device's location on the bus, but also on the routing offset
* of the most immediate Pci-To-Pci Bridge.  This offset, in turn, is based
* on its location in the PCI hierarchy and was computed earlier by the
* PCI configuration process.  However, the user may skip this automatic
* interrupt assignment process by simply setting the variable autoIntRouting
* in the relevant PCI_SYSTEM structure to FALSE.  In this case the bsp
* interrupt assign routine will be called to get the IRQ number for the device.
*
* RETURNS: The interrupt number associated with the device.
*
* ERRNO
*
*/

LOCAL UCHAR pciAutoIntAssign
    (
    PCI_AUTO_CONFIG_OPTS * pSystem, /* PCI system info */
    PCI_LOC * pFunc       /* input: Pointer to PCI function structure */
    )
    {
    UCHAR retVal = 0xFF;
    UCHAR        intPin;        /* Interrupt "Pin" value            */

    pciConfigInByte (pFunc->bus, pFunc->device, pFunc->function,
                     PCI_CFG_DEV_INT_PIN, &intPin);

    /*
     * if the bsp provides interrupt routing for all PCI devices,
     * then there's no need of any computation whatsoever
     */

    if ((!(pSystem->autoIntRouting)) && (intPin != 0))
        {
        if ((pSystem->intAssignRtn) != NULL )
            {
            retVal = (pSystem->intAssignRtn) ((PCI_SYSTEM *)pSystem, pFunc, intPin);

            return(retVal);
            }

        }

    /* default interrupt routing: let's find out the IRQ for this device */
    if (currentPcieController == PCIE_CONTROLLER_0) {

    switch (pFunc->bus)
        {
        case 1:
                if (((pSystem->intAssignRtn) != NULL) && (intPin != 0))
                    {
                    retVal = (pSystem->intAssignRtn) ((PCI_SYSTEM *)pSystem, pFunc, intPin);
                    }
    
                /*
                 * if this is a P2P Bridge, then populate its interrupt
                 * routing table.  This will be used later for all the devices
                 * belonging to its every subordinate bus
                 */
    
                if (((pFunc->attribute) & PCI_AUTO_ATTR_BUS_PCI) > 0)
                    {
                    int i = 0;
    
                    for (i = 0; i < 4; i++)
                        {
                        if ((pSystem->intAssignRtn) != NULL )
                            {
                            pciAutoIntRoutingTable [i]  = (pSystem->intAssignRtn)
                                                          ((PCI_SYSTEM *)pSystem, pFunc, (i+1));
                            }
                        }
    
                    }
    
                break;
    
            default:
    
                retVal = pciAutoIntRoutingTable [(((pFunc->device) + (intPin - 1)
                                                   + (pFunc->offset)) % 4)];
                break;
            }
    } else {
        switch (pFunc->bus)
            {
            case PCIE1_BUS_MIN:
                if (((pSystem->intAssignRtn) != NULL) && (intPin != 0))
                    {
                    retVal = (pSystem->intAssignRtn) ((PCI_SYSTEM *)pSystem, pFunc, intPin);
                    }

                /*
                 * if this is a P2P Bridge, then populate its interrupt
                 * routing table.  This will be used later for all the devices
                 * belonging to its every subordinate bus
                 */

                if (((pFunc->attribute) & PCI_AUTO_ATTR_BUS_PCI) > 0)
                    {
                    int i = 0;
    
                    for (i = 0; i < 4; i++)
                        {
                        if ((pSystem->intAssignRtn) != NULL )
                            {
                            pciAutoIntRoutingTable [i]  = (pSystem->intAssignRtn)
                                                      ((PCI_SYSTEM *)pSystem, pFunc, (i+1));
                            }
                        }
                     }
            break;

        default:

            retVal = pciAutoIntRoutingTable [(((pFunc->device) + (intPin - 1)
                                               + (pFunc->offset)) % 4)];
            break;
        }
    }

    PCI_AUTO_DEBUG_MSG("pciAutoIntAssign: int for [%d,%d,%d] pin %d is [%d]\n",
                       pFunc->bus, pFunc->device, pFunc->function, intPin, retVal, 0);

    return retVal;
    }

/******************************************************************************
*
* pciAutoGetNextClass - find the next device of specific type from probe list
*
* The function uses the probe list which was built during the probing
* process.  Using configuration accesses, it searches for the
* occurrence of the device subject to the 'class' and 'mask'
* restrictions outlined below.  Setting 'class' to zero and 'mask' to
* zero allows searching the entire set of devices found regardless of
* class.
*
* RETURNS: TRUE if a device was found, else FALSE.
*
* ERRNO
*
*/

STATUS pciAutoGetNextClass
    (
    PCI_SYSTEM *pSys,   /* for backwards compatibility */
    PCI_LOC *pPciFunc,  /* output: Contains the BDF of the device found */
    UINT *index,        /* Zero-based device instance number */
    UINT pciClass,      /* class code field from the PCI header */
    UINT mask           /* mask is ANDed with the class field */
    )
    {
    PCI_AUTO_CONFIG_OPTS *pPciSystem;
    UINT i;
    UINT idx = *index;
    UINT classCode;
    UINT nSize;
    PCI_LOC *pciList;

    /*
     * automatically call pciAutoConfigLibInit() for
     * backwards compatibility if pciAutoConfig() was
     * used.
     */
    if ( pPciCfgOpts->pciConfigInit == TRUE )
        pPciSystem = pPciCfgOpts;
    else
        {
        pPciSystem = pciAutoConfigLibInit(NULL);
        pciAutoCfgCtl(pPciSystem, PCI_PSYSTEM_STRUCT_COPY, pSys);
        }

    nSize = (UINT)lastPciListSize;
    pciList = pLastPciList;

    PCI_AUTO_DEBUG_MSG("\npciAutoGetNextClass: index[%d] listSiz[%d]\n",
                       *index, nSize, 0, 0, 0, 0);
    PCI_AUTO_DEBUG_MSG("                     pciClass[0x%08x], mask[0x%08x]\n",
                       pciClass, mask, 0, 0, 0, 0);

    if ((nSize <= idx) || (pciList == NULL))
        {
        /* copy back to PCI_SYSTEM struct for backward compatibility */
        pciAutoConfigCopyback(pPciSystem, pSys);
        return(FALSE);             /* No more devices */
        }

    for (i = idx; i < nSize; i++)
        {

        /* Grab the class code 24-bit field */

        pciConfigInLong ((UINT)pciList[i].bus, (UINT)pciList[i].device,
                         (UINT)pciList[i].function, (PCI_CFG_CLASS & 0xfc),
                         &classCode);

        classCode >>= 8;        /* Isolate class code in low order bits */

        if ((classCode & mask) == (pciClass & mask))
            {
            *index = i;
            *pPciFunc = pciList[i];

            /* copy back to PCI_SYSTEM struct for backward compat */
            pciAutoConfigCopyback(pPciSystem, pSys);

            return(TRUE);
            }

        }

    /* copy back to PCI_SYSTEM struct for backward compatibility */
    pciAutoConfigCopyback(pPciSystem, pSys);

    return(FALSE);
    }

/******************************************************************************
*
* pciAutoDevConfig - allocate memory and I/O space to PCI function
*
* This routine allocates memory and I/O space to functions on an
* individual PCI bus.
*
* LIMITATIONS
* Do not sort the include function list before this routine is
* called.  This routine requires each function in the list to be in the
* same order as the probe occurred.
*
* RETURNS: N/A
*
* ERRNO
*
*/

LOCAL void pciAutoDevConfig
    (
    PCI_AUTO_CONFIG_OPTS * pSystem,  /* PCI system info */
    UINT bus,       /* Current bus number      */
    PCI_LOC **ppPciList,    /* Pointer to function list */
    UINT *nSize     /* Number of remaining funcs */
    )
    {
    PCI_LOC *pPciFunc;  /* Pointer to PCI function     */
    UINT nextBus;       /* Bus where function is located   */
    UINT16 pciClass;        /* Class field of function     */

    /* Process each function within the list */

    while (*nSize > 0)
        {

        /* Setup local variables */

        pPciFunc = *ppPciList;
        nextBus = pPciFunc->bus;

        /* Decrease recursion depth if this function is on a parent bus */

        if (nextBus < bus)
            {
            return;
            }

        /* Allocate and assign space to functions on this bus */
        pciAutoFuncConfig (pSystem, pPciFunc);
        (*nSize)--;
        (*ppPciList)++;

        /* Increase recursion depth if this function is a PCI-PCI bridge */

        pciConfigInWord (pPciFunc->bus, pPciFunc->device, pPciFunc->function,
                         PCI_CFG_SUBCLASS, &pciClass);

        switch (pciClass)
            {
#if 0 /* TBD MDG */
            case ((PCI_CLASS_BRIDGE_CTLR << 8) + PCI_SUBCLASS_CARDBUS_BRIDGE):

                pciAutoCardBusConfig (pSystem, pPciFunc, ppPciList, nSize);
                break;
#endif
            case (( PCI_CLASS_BRIDGE_CTLR << 8) + PCI_SUBCLASS_P2P_BRIDGE ):

                /* PCI-PCI bridge functions increase recursion depth */

                pciAutoBusConfig (pSystem, pPciFunc, ppPciList, nSize);
                break;

            default:
                /* Maintain current recursion depth */

                break;
            }
        }
    }

/******************************************************************************
*
* pciAutoFuncConfig - assign memory and/or I/O space to single function
*
* This routine allocates and assigns memory and/or I/O space to a
* single PCI function.   Allocations are made for each implemented
* base address register (BAR) in the PCI configuration header.
*
* RETURNS: N/A
*
* ERRNO
*
*/

LOCAL void pciAutoFuncConfig
    (
    PCI_AUTO_CONFIG_OPTS * pSystem, /* cookie returned by pciAutoConfigLibInit() */
    PCI_LOC * pPciFunc  /* input: "Include list" pointer to function */
    )
    {
    UINT baMax;     /* Total number of base addresses    */
    UINT baI;       /* Base address register index       */
    UINT baseAddr;      /* PCI Offset of base address        */
    UINT readVar;       /* Contents of base address register */
    UINT addrInfo;      /* PCI address type information      */
    UINT sizeMask;      /* LSbit for size calculation        */
    UCHAR headerType;   /* Read from PCI config header       */
    UINT dev_vend;

    /* If there is a function, then consult the exclusion routine */

    if ( (pSystem->includeRtn) != NULL )
        {
        pciConfigInLong (pPciFunc->bus, pPciFunc->device, pPciFunc->function,
                         PCI_CFG_VENDOR_ID, &dev_vend);
        if ( ((pSystem->includeRtn) ((PCI_SYSTEM *)pSystem, pPciFunc, dev_vend)) == ERROR )
            {
            if ((pPciFunc->attribute & PCI_AUTO_ATTR_BUS_PCI) == 0)
                {
                pPciFunc->attribute |= PCI_AUTO_ATTR_DEV_EXCLUDE;
                PCI_AUTO_DEBUG_MSG("pciAutoFuncConfig: exc [%d,%d,%d,0x%02x]\n",
                                   pPciFunc->bus, pPciFunc->device, pPciFunc->function,
                                   pPciFunc->attribute,0,0);
                return;
                }
            }
        }

    /* Disable the function */

    pciAutoFuncDisable (pPciFunc);

    /* Determine the number of base address registers present */

    pciConfigInByte (pPciFunc->bus, pPciFunc->device, pPciFunc->function,
                     PCI_CFG_HEADER_TYPE, &headerType);
    headerType &= 0x7f;


    switch (headerType)
        {
        case PCI_HEADER_TYPE0:
            baMax = 6;
            break;

        case PCI_HEADER_PCI_PCI:
            baMax = 2;
            break;
#if 0 /* TBD MDG */
        case PCI_HEADER_PCI_CARDBUS+:
            baMax = 1;
            break;
#endif
        default:
            baMax = 0;
            break;
        }

    /* Allocate Memory or I/O space for each implemented base addr register */
    for (baI = 0; baI < baMax; baI++)
        {
        /* Get the base address register contents */

        baseAddr = PCI_CFG_BASE_ADDRESS_0 + (baI * 4);

        pciConfigOutLong (pPciFunc->bus, pPciFunc->device, pPciFunc->function,
                          baseAddr, 0xFFFFFFFF);

        pciConfigInLong (pPciFunc->bus, pPciFunc->device, pPciFunc->function,
                         baseAddr, &readVar);

        /* Go to the next BAR when an unimplemented one (BAR==0) is found */

        if (readVar == 0)
            {
            continue;
            }

        /* Mask off all but space, memory type, and prefetchable bits */

        addrInfo = readVar & PCI_BAR_ALL_MASK;

        /* Check for type, setup mask variables (based on type) */

        if ((addrInfo & PCI_BAR_SPACE_MASK) == PCI_BAR_SPACE_IO)
            {
            PCI_AUTO_DEBUG_MSG("pciAutoFuncConfig: IO Space found at BAR[%d]\n",
                               baI, 0, 0, 0, 0, 0);
            sizeMask = (1 << 2);
            }
        else
            {
            PCI_AUTO_DEBUG_MSG("pciAutoFuncConfig: MemSpace found at BAR[%d]\n",
                               baI, 0, 0, 0, 0, 0);
            sizeMask = (1 << 4);
            }

        /* Loop until we find a bit set or until we run out of bits */

        for (; sizeMask; sizeMask <<= 1)
            {
            /* is this bit set? if not, keep looking */

            if (readVar & sizeMask)
                {
                baI += pciAutoRegConfig ((PCI_SYSTEM *)pSystem, pPciFunc, baseAddr, sizeMask,
                                         addrInfo);
                break;
                }
            }
        }
    }

/******************************************************************************
*
* pciAutoRegConfig - assign PCI space to a single PCI base address register
*
* This routine allocates and assigns PCI space (either memory or I/O)
* to a single PCI base address register.
*
* RETURNS: Returns (1) if BAR supports mapping anywhere in 64-bit address space.
* Returns (0) otherwise.
*
* ERRNO
*
*/

UINT pciAutoRegConfig
    (
    PCI_SYSTEM *pSys,       /* backwards compatibility */
    PCI_LOC *pPciFunc,      /* Pointer to function in device list */
    UINT baseAddr,      /* Offset of base PCI address */
    UINT nSize,         /* Size and alignment requirements */
    UINT addrInfo       /* PCI address type information */
    )
    {
    PCI_AUTO_CONFIG_OPTS * pSystem; /* Pointer to PCI System structure */
    UINT addr;          /* Working address variable */
    UINT spaceEnable = 0;   /* PCI space enable bit */
    UINT baseaddr_mask;     /* Mask for base address register */
    UINT register64Bit;     /* 64 bit register flag */

    /*
    * automatically call pciAutoConfigLibInit() for
    * backwards compatibility if pciAutoConfig() was
    * used.
    */
    if ( pPciCfgOpts->pciConfigInit == TRUE )
        pSystem = pPciCfgOpts;
    else
        {
        pSystem = pciAutoConfigLibInit(NULL);
        pciAutoCfgCtl(pSystem, PCI_PSYSTEM_STRUCT_COPY, pSys);
        }

    /* Select the appropriate PCI address space for this register */

    if ((addrInfo & PCI_BAR_SPACE_MASK) == PCI_BAR_SPACE_IO)
        {
        /* Configure this register for PCI I/O space */
        spaceEnable = PCI_CMD_IO_ENABLE;
        baseaddr_mask = 0xFFFFFFFC;
        register64Bit = pciAutoIoAlloc (pSystem, pPciFunc, &addr, nSize);
        }
    else
        {

        /* Configure this register for PCI memory space */
        spaceEnable = PCI_CMD_MEM_ENABLE;
        baseaddr_mask = 0xFFFFFFF0;
        register64Bit = pciAutoMemAlloc (pSystem, pPciFunc, &addr, nSize,
                                         addrInfo);
        }

    /*
     * Do not exceed the upper boundary! If this occurs, all we can
     * do here is return, as this is called early in the initialization
     * process, before I/O is available to print error messages.
     */

    if (addr != NO_ALLOCATION)
        {

        if (addr == ALLOCATION_64BIT_MEM_SPACE) {
            addr = SI_PCIE_DMA_L32;

            pciConfigModifyLong (pPciFunc->bus, pPciFunc->device,
                             pPciFunc->function, baseAddr,
                             baseaddr_mask, addr);
            if (register64Bit)
            {
            	  if (currentPcieController == PCIE_CONTROLLER_0) {
                	  addr = SI_PCIE_DMA_H32 + pcie0_64bit_region_alloc_index;
                	  pcie0_64bit_region_alloc_index++;
            	  } else {
                	  addr = SI_PCIE1_DMA_H32 + pcie1_64bit_region_alloc_index;
                	  pcie1_64bit_region_alloc_index++;
        	      }
                pciConfigOutLong (pPciFunc->bus, pPciFunc->device,
                              pPciFunc->function,
                              baseAddr + 4, addr);
            }

            /* Set the appropriate enable bit, preserve status bits */

            pciConfigModifyLong (pPciFunc->bus, pPciFunc->device,
                             pPciFunc->function, PCI_CFG_COMMAND,
                             (0xffff0000 | spaceEnable), spaceEnable);

        } else {
            /* Program the base address register */

            PCI_AUTO_DEBUG_MSG("pciAutoRegConfig:[0x%08x] written to BAR[0x%08x]\n",
                           addr, baseAddr, 0, 0, 0, 0);
            pciConfigModifyLong (pPciFunc->bus, pPciFunc->device,
                             pPciFunc->function, baseAddr,
                             baseaddr_mask, addr);

            if (register64Bit)
            {

            /*
             * Write the base address for 64-bit addressable memory devices:
             * initialize the next base address register to zero, the PReP
             * address map does support physical addresses above 4GB (i.e.,
             * 32-bit address space)
             */

                pciConfigOutLong (pPciFunc->bus, pPciFunc->device,
                              pPciFunc->function,
                              baseAddr + 4, 0);
            }

            /* Set the appropriate enable bit, preserve status bits */

            pciConfigModifyLong (pPciFunc->bus, pPciFunc->device,
                             pPciFunc->function, PCI_CFG_COMMAND,
                             (0xffff0000 | spaceEnable), spaceEnable);

        }
        }

    /* copy back to PCI_SYSTEM struct for backward compatibility */
    pciAutoConfigCopyback(pSystem, pSys);

    return(register64Bit);
    }

/******************************************************************************
*
* pciAutoIoAlloc - select appropriate I/O space for device
*
* This routine determines which PCI I/O space (16-bit or 32-bit) to assign
* to a particular function.  Note that functions located on subordinate
* busses must be  assigned to the 16 bit PCI I/O space due to 16 bit
* decoder limitations of certain bridges and functions.  The PCI
* specification limits each I/O base address register to 256 bytes, so
* this size should not be a problem.
*
* RETURNS: 0, indicating I/O space not located anywhere in 64-bit space.
*
* ERRNO
*
*/

LOCAL UINT pciAutoIoAlloc
    (
    PCI_AUTO_CONFIG_OPTS * pPciSys, /* PCI system structure   */
    PCI_LOC *pPciFunc,  /* input: Pointer to PCI function element     */
    UINT *pAlloc,       /* output: Pointer to PCI space alloc pointer */
    UINT nSize      /* requested size (power of 2) */
    )
    {
    UINT * pBase;
    UINT32 alignedBase;
    UINT32 sizeAdj;
    UINT * pAvail = NULL;
    STATUS retStat = ERROR;

    /* Assign this register to PCI I/O space */

    if ((pPciFunc->bus == 0) &&
        ((pPciFunc->attribute & PCI_AUTO_ATTR_BUS_4GB_IO) != 0))
        {
        PCI_AUTO_DEBUG_MSG("pciAutoIoAlloc: 32-bit I/O\n", 0, 0, 0, 0, 0, 0);
        pBase = &(pPciSys->pciIo32);
        pAvail = &(pPciSys->pciIo32Size);
        }
    else
        {
        PCI_AUTO_DEBUG_MSG("pciAutoIoAlloc: 16-bit I/O\n", 0, 0, 0, 0, 0, 0);
        pBase = &(pPciSys->pciIo16);
        pAvail = &(pPciSys->pciIo16Size);
        }

    /* Adjust for alignment */

    if (*pAvail > 0)
        {
        retStat = pciAutoAddrAlign (*pBase,
                                    (*pBase + *pAvail),
                                    nSize,
                                    &alignedBase);
        }

    /* If the space is exhausted, then return an invalid pointer */

    if (retStat == ERROR)
        {
        PCI_LOG_MSG("Warning: PCI I/O allocation failed\n",
                    0, 0, 0, 0, 0, 0);
        *pAlloc = NO_ALLOCATION;
        return 0;
        }

    PCI_AUTO_DEBUG_MSG("pciAutoIoAlloc: Pre/Post alloc: \n",
                       0, 0, 0, 0, 0, 0);
    PCI_AUTO_DEBUG_MSG("  Pre: pBase[0x%08x], pAvail[0x%08x]\n",
                       (int)(*pBase), (int)(*pAvail), 0, 0, 0, 0);

    *pAlloc  = alignedBase;
    sizeAdj = (alignedBase - *pBase) + nSize;
    *pBase  += sizeAdj;
    *pAvail -= sizeAdj;

    /* update size used in option struct */
    if ((pPciFunc->bus == 0) &&
        ((pPciFunc->attribute & PCI_AUTO_ATTR_BUS_4GB_IO) != 0))
        {
        pPciSys->pciIo32Used += sizeAdj;
        }
    else
        {
        pPciSys->pciIo16Used += sizeAdj;
        }

    PCI_AUTO_DEBUG_MSG("  Post: pBase[0x%08x], pAvail[0x%08x]\n",
                       (int)(*pBase), (int)(*pAvail), 0, 0, 0, 0);

    return 0; /* can't have 64 bit i/o addresses */
    }

/******************************************************************************
*
* pciAutoMemAlloc - select appropriate memory space for device
*
* This routine determines which PCI memory space pool to use for assignment
* to a particular function.  Note that functions located on subordinate
* busses must be  assigned to the 32 bit PCI memory space due to 32 bit
* requirements of functions using more than 1MB memory space.
*
* LIMITATIONS
* Does not support 64-bit Memory space
*
* RETURNS: Returns 1 if 64-bit addressable memory space.
* Returns zero (0) otherwise.
*
* ERRNO
*
*/

LOCAL UINT pciAutoMemAlloc
    (
    PCI_AUTO_CONFIG_OPTS * pPciSys, /* PCI system structure */
    PCI_LOC * pPciFunc,     /* Pointer to PCI function element     */
    UINT * pAlloc,      /* Pointer to PCI space alloc pointer */
    UINT size,      /* space requested (power of 2)  */
    UINT addrInfo       /* PCI address type information       */
    )
    {
    UINT register64Bit = 0; /* 64 bit register flag */
    UINT * pBase;
    UINT * pAvail;
    UINT32 alignedBase;
    UINT32 sizeAdj;
    STATUS retStat = ERROR;
    UINT32 * pUpdate;

    /* Process address attribute info */

    switch (addrInfo & (UINT)PCI_BAR_MEM_TYPE_MASK )
        {
        case PCI_BAR_MEM_BELOW_1MB:
            break;

        case PCI_BAR_MEM_ADDR64:
            /*
             * Check for a 64-bit addressable memory device, currently
             * the PReP address map does not support physical addresses
             * above 4Gb (i.e., 32-bits), so the configuration process
             * will initialize the upper base register to zero (i.e.,
             * the safe thing to do), so for right now we'll skip the
             * next base address register which belongs to the 64-bit
             * pair of 32-bit base address registers.
             */

            register64Bit = 1;
            break;

        case PCI_BAR_MEM_ADDR32:
            break;

        case PCI_BAR_MEM_RESERVED:
            /* fall through */

        default:
            *pAlloc = NO_ALLOCATION;
            return 0;
        }

    if ( (addrInfo & PCI_BAR_MEM_PREFETCH) &&
         ((pPciFunc->attribute) & PCI_AUTO_ATTR_BUS_PREFETCH) )
        {
        PCI_AUTO_DEBUG_MSG("pciAutoMemAlloc: PF Mem requested"
                           "\n", 0, 0, 0, 0, 0, 0);
        pBase = &(pPciSys->pciMem32);
        pAvail = &(pPciSys->pciMem32Size);
        pUpdate = &(pPciSys->pciMem32Used);

        if (*pAvail > 0)
            {
            retStat = pciAutoAddrAlign (*pBase,
                                        (*pBase + *pAvail),
                                        size,
                                        &alignedBase);
            }

        if (retStat == ERROR)
            {

            /* If no PF memory available, then try conventional */

            PCI_AUTO_DEBUG_MSG("pciAutoMemAlloc: No PF Mem available"
                               "Trying MemIO\n", 0, 0, 0, 0, 0, 0);
            pBase = &(pPciSys->pciMemIo32);
            pAvail = &(pPciSys->pciMemIo32Size);
            pUpdate = &(pPciSys->pciMemIo32Used);

            if (*pAvail > 0)
                {
                retStat = pciAutoAddrAlign (*pBase,
                                            (*pBase + *pAvail),
                                            size,
                                            &alignedBase);
                }

                if (retStat == ERROR) {
                    PCI_LOG_MSG("Warning: PCI PF Mem alloc failed\n",
                            0, 0, 0, 0, 0, 0);
                    *pAlloc = NO_ALLOCATION;
                    return 0;
                } else if (retStat == ERROR_EXCESS_SIZE) {
                    *pAlloc = ALLOCATION_64BIT_MEM_SPACE;
                    return register64Bit;
                }
            }
        }
    else
        {

        /* Use 32-bit Non-Prefetch Memory */

        pBase = &(pPciSys->pciMemIo32);
        pAvail = &(pPciSys->pciMemIo32Size);
        pUpdate = &(pPciSys->pciMemIo32Used);

        if (*pAvail > 0)
            {
            retStat = pciAutoAddrAlign (*pBase,
                                        (*pBase + *pAvail),
                                        size,
                                        &alignedBase);
            }

            if (retStat == ERROR) {
                PCI_LOG_MSG("Warning: PCI PF Mem alloc failed\n",
                        0, 0, 0, 0, 0, 0);
                *pAlloc = NO_ALLOCATION;
                return 0;
            } else if (retStat == ERROR_EXCESS_SIZE) {
                *pAlloc = ALLOCATION_64BIT_MEM_SPACE;
                return register64Bit;
            }
        }

    PCI_AUTO_DEBUG_MSG("pciAutoMemAlloc: \n", 0, 0, 0, 0, 0, 0);
    PCI_AUTO_DEBUG_MSG("  Pre: pBase[0x%08x], pAvail[0x%08x]\n",
                       (int)(*pBase), (int)(*pAvail), 0, 0, 0, 0);
    *pAlloc  = alignedBase;
    sizeAdj = (alignedBase - *pBase) + size;
    *pBase  += sizeAdj;
    *pAvail -= sizeAdj;

    /* update size used in option struct */
    *pUpdate += sizeAdj;

    PCI_AUTO_DEBUG_MSG("  Post: pBase[0x%08x], pAvail[0x%08x]\n",
                       (int)(*pBase), (int)(*pAvail), 0, 0, 0, 0);

    return register64Bit;
    }

/******************************************************************************
*
* pciAutoBusConfig - set mem and I/O registers for a single PCI-PCI bridge
*
* This routine  sets up memory and I/O base/limit registers for
* an individual PCI-PCI bridge.
*
* PCI/PCI bridges require that memory space assigned to functions
* on the secondary bus reside in one common block defined by a memory
* base register and a memory limit register, and I/O space assigned to
* functions on the secondary bus must reside in one common block defined
* by an I/O base register and an I/O limit register.  This effectively
* prevents the use of 32 bit PCI I/O space on subordinate (non-zero)
* busses due to the 16 bit decoder limitations of certain bridges and
* functions.
*
* Note that functions which use 20 bit memory decoders must be placed
* on bus zero, or this code will have to be modified to impose a 20 bit
* restriction on all subordinate busses (since some PCI devices allocate
* huge memory spaces, a 1MB total restriction is unacceptable).
*
* Also note that this code does not provide prefetchable memory support
* at this time.
*
* Warning: do not sort the include function list before this routine is
* called.  This routine requires each function in the list to be in the
* same order as the probe occurred.
*
* RETURNS: N/A
*
* ERRNO
*
*/

LOCAL void pciAutoBusConfig
    (
    PCI_AUTO_CONFIG_OPTS * pSystem, /* PCI system info */
    PCI_LOC * pPciLoc,      /* PCI address of this bridge */
    PCI_LOC **ppPciList,    /* Pointer to function list pointer */
    UINT *nSize         /* Number of remaining functions */
    )
    {
    UCHAR bus;          /* Bus number for current bus */
    UINT8 maxLatency;       /* MAX_LAT value */
    UINT dev_vend;
    UINT32 debugTmp;
    UINT32 debugTmp2;
    UINT32 debugTmp3;
    UINT32 alignedBase;

    /* If it exists, call the user-defined pre-config pass bridge init */

    if ((pSystem->bridgePreConfigInit) != NULL )
        {
        pciConfigInLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                         PCI_CFG_VENDOR_ID, &dev_vend);

        (pSystem->bridgePreConfigInit) ((PCI_SYSTEM *)pSystem, pPciLoc, dev_vend);
        }

    /* Clear the secondary status bits */

    pciConfigModifyLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                         PCI_CFG_IO_BASE, 0xffff0000, 0xffff0000);

    /* If Prefetch supported, then pre-configure 32-bit PF Memory Base Addr */

    if ( (pPciLoc->attribute & PCI_AUTO_ATTR_BUS_PREFETCH)
         && (pSystem->pciMem32Size > 0) )
        {

        PCI_AUTO_DEBUG_MSG("pciAutoBusConfig: Configuring prefetch aperture\n",
                           0, 0, 0, 0, 0, 0);

        pciAutoAddrAlign(pSystem->pciMem32,
                         (pSystem->pciMem32 + pSystem->pciMem32Size),
                         0x100000,
                         &alignedBase);

        PCI_AUTO_DEBUG_MSG("PF Mem Base orig[0x%08x] new[0x%08x] adj[0x%08x]\n",
                           (pSystem->pciMem32),
                           alignedBase,
                           (alignedBase - (pSystem->pciMem32)),
                           0,
                           0,
                           0
                          );

        (pSystem->pciMem32Used) += (alignedBase - (pSystem->pciMem32));
        (pSystem->pciMem32Size) -= (alignedBase - (pSystem->pciMem32));
        (pSystem->pciMem32) = alignedBase;

        /* 64-bit Prefetch memory not supported at this time */

        pciConfigOutLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                          PCI_CFG_PRE_MEM_BASE_U, 0);

        pciConfigModifyLong (pPciLoc->bus,pPciLoc->device,pPciLoc->function,
                             PCI_CFG_PRE_MEM_BASE, 0x0000fff0,
                             (pSystem->pciMem32 >> (20-4))
                            );

        pciConfigInLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                         PCI_CFG_PRE_MEM_BASE,
                         &debugTmp2);

        debugTmp = ((debugTmp2 & 0x0000fff0) << 16);

        PCI_AUTO_DEBUG_MSG("pciAutoBusConfig: PF Mem Base [0x%08x]\n",
                           debugTmp, 0, 0, 0, 0, 0);
        }

    /* Pre-configure 16-bit I/O Base Address */

    if ((pSystem->pciIo16Size) > 0)
        {
        pciAutoAddrAlign(pSystem->pciIo16,
                         (pSystem->pciIo16 + pSystem->pciIo16Size),
                         0x1000,
                         &alignedBase);

        PCI_AUTO_DEBUG_MSG("I/O 16 Base orig[0x%08x] new[0x%08x] adj[0x%08x]\n",
                           (pSystem->pciIo16),
                           alignedBase,
                           (alignedBase - (pSystem->pciIo16)),
                           0,
                           0,
                           0
                          );

        (pSystem->pciIo16Used) += (alignedBase - (pSystem->pciIo16));
        (pSystem->pciIo16Size) -= (alignedBase - (pSystem->pciIo16));
        (pSystem->pciIo16) = alignedBase;

        pciConfigModifyLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                             PCI_CFG_IO_BASE, 0x000000f0,
                             (pSystem->pciIo16 >> (12-4))
                            );

        pciConfigModifyLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                             PCI_CFG_IO_BASE_U, 0x0000ffff,
                             (pSystem->pciIo16 >> 16)
                            );

        pciConfigInLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                         PCI_CFG_IO_BASE,
                         &debugTmp);

        pciConfigInLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                         PCI_CFG_IO_BASE_U,
                         &debugTmp2);

        debugTmp3 = (((debugTmp & (UINT32)0xf0) << (12-4)) & 0x0000ffff);
        debugTmp = debugTmp3 | ((debugTmp2 << 16) & 0xffff0000);

        PCI_AUTO_DEBUG_MSG("pciAutoBusConfig: IO16 Base Address [0x%08x]\n",
                           debugTmp, 0, 0, 0, 0, 0);

        }

    /* Pre-configure 32-bit Non-prefetchable Memory Base Address */

    if ((pSystem->pciMemIo32Size) > 0)
        {

        pciAutoAddrAlign(pSystem->pciMemIo32,
                         (pSystem->pciMemIo32 + pSystem->pciMemIo32Size),
                         0x100000,
                         &alignedBase);

        PCI_AUTO_DEBUG_MSG("Memory Base orig[0x%08x] new[0x%08x] adj[0x%08x]\n",
                           (pSystem->pciMemIo32),
                           alignedBase,
                           (alignedBase - (pSystem->pciMemIo32)),
                           0,
                           0,
                           0
                          );

        (pSystem->pciMemIo32Used) += (alignedBase - (pSystem->pciMemIo32));
        (pSystem->pciMemIo32Size) -= (alignedBase - (pSystem->pciMemIo32));
        (pSystem->pciMemIo32) = alignedBase;

        pciConfigModifyLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                             PCI_CFG_MEM_BASE, 0x0000fff0,
                             (pSystem->pciMemIo32 >> (20-4))
                            );

        pciConfigInLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                         PCI_CFG_MEM_BASE,
                         &debugTmp2);

        debugTmp = ((debugTmp2 & 0x0000fff0) << 16);

        PCI_AUTO_DEBUG_MSG("pciAutoBusConfig: Mem Base Address [0x%08x]\n",
                           debugTmp, 0, 0, 0, 0, 0);
        }

    /* Configure devices on the bus implemented by this bridge */

    pciConfigInByte (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                     PCI_CFG_SECONDARY_BUS, &bus);

    pciAutoDevConfig (pSystem, bus, ppPciList, nSize);

    /* Post-configure 32-bit I/O Limit Address */

    if ((pSystem->pciIo16Size) > 0)
        {

        pciAutoAddrAlign(pSystem->pciIo16,
                         (pSystem->pciIo16 + pSystem->pciIo16Size),
                         0x1000,
                         &alignedBase);

        PCI_AUTO_DEBUG_MSG("I/O 16 Lim orig[0x%08x] new[0x%08x] adj[0x%08x]\n",
                           (pSystem->pciIo16),
                           alignedBase,
                           (alignedBase - (pSystem->pciIo16)),
                           0,
                           0,
                           0
                          );

        (pSystem->pciIo16Used) += (alignedBase - (pSystem->pciIo16));
        (pSystem->pciIo16Size) -= (alignedBase - (pSystem->pciIo16));
        (pSystem->pciIo16) = alignedBase;

        pciConfigModifyLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                             PCI_CFG_IO_BASE, 0x0000f000,
                             (pSystem->pciIo16 - 1)
                            );

        pciConfigModifyLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                             PCI_CFG_IO_BASE_U, 0xffff0000,
                             (pSystem->pciIo16 - 1)
                            );

        pciConfigInLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                         PCI_CFG_IO_BASE,
                         &debugTmp);

        pciConfigInLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                         PCI_CFG_IO_BASE_U,
                         &debugTmp2);

        debugTmp3 = ((debugTmp & (UINT32)0xf000) & 0x0000ffff);
        debugTmp = debugTmp3 | (debugTmp2 & 0xffff0000);
        debugTmp |= 0x00000FFF;

        PCI_AUTO_DEBUG_MSG("pciAutoBusConfig: IO Limit [0x%08x]\n",
                           debugTmp, 0, 0, 0, 0, 0);

        }

    /* Post-configure 32-bit Non-prefetchable Memory Limit Address */

    if ((pSystem->pciMemIo32Size) > 0)
        {

        pciAutoAddrAlign(pSystem->pciMemIo32,
                         (pSystem->pciMemIo32 + pSystem->pciMemIo32Size),
                         0x100000,
                         &alignedBase);

        PCI_AUTO_DEBUG_MSG("MemIo Lim orig[0x%08x] new[0x%08x] adj[0x%08x]\n",
                           (pSystem->pciMemIo32),
                           alignedBase,
                           (alignedBase - (pSystem->pciMemIo32)),
                           0,
                           0,
                           0
                          );

        (pSystem->pciMemIo32Used) += (alignedBase - (pSystem->pciMemIo32));
        (pSystem->pciMemIo32Size) -= (alignedBase - (pSystem->pciMemIo32));
        (pSystem->pciMemIo32) = alignedBase;

        pciConfigModifyLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                             PCI_CFG_MEM_BASE, 0xfff00000,
                             (pSystem->pciMemIo32 - 1)
                            );

        pciConfigInLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                         PCI_CFG_MEM_BASE,
                         &debugTmp2);

        debugTmp = (debugTmp2 & 0xfff00000);
        debugTmp |= 0x000FFFFF;

        PCI_AUTO_DEBUG_MSG("pciAutoBusConfig: MemIo Limit [0x%08x]\n",

                           debugTmp, 0, 0, 0, 0, 0);
        }

    /* Post-configure 32-bit Prefetchable Memory Address */

    if ( (pPciLoc->attribute & PCI_AUTO_ATTR_BUS_PREFETCH) &&
         ((pSystem->pciMem32Size) > 0) )
        {

        pciAutoAddrAlign(pSystem->pciMem32,
                         (pSystem->pciMem32 + pSystem->pciMem32Size),
                         0x100000,
                         &alignedBase);

        PCI_AUTO_DEBUG_MSG("PF Lim orig[0x%08x] new[0x%08x] adj[0x%08x]\n",
                           (pSystem->pciMem32),
                           alignedBase,
                           (alignedBase - (pSystem->pciMem32)),
                           0,
                           0,
                           0
                          );

        (pSystem->pciMem32Used) += (alignedBase - (pSystem->pciMem32));
        (pSystem->pciMem32Size) -= (alignedBase - (pSystem->pciMem32));
        (pSystem->pciMem32) = alignedBase;

        /* 64-bit Prefetchable memory not supported at this time */

        pciConfigOutLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                          PCI_CFG_PRE_MEM_LIMIT_U, 0);

        pciConfigModifyLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                             PCI_CFG_PRE_MEM_BASE, 0xfff00000,
                             (pSystem->pciMem32 - 1)
                            );

        pciConfigInLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                         PCI_CFG_PRE_MEM_BASE,
                         &debugTmp2);

        debugTmp = (debugTmp2 & 0xfff00000);
        debugTmp |= 0x000FFFFF;

        PCI_AUTO_DEBUG_MSG("pciAutoBusConfig: PF Mem Limit [0x%08x]\n",
                           debugTmp, 0, 0, 0, 0, 0);
        }

    if ((pSystem->bridgePostConfigInit) != NULL )
        {
        pciConfigInLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                         PCI_CFG_VENDOR_ID, &dev_vend);

        (pSystem->bridgePostConfigInit) ((PCI_SYSTEM *)pSystem, pPciLoc, dev_vend);
        }

    /* Initialize primary and secondary PCI-PCI bridge latency timers */

    /*
    *  If MAX_LAT function supplied by BSP, call it to calculate
    *  the value of MAX_LAT for each function individually.
    *  Otherwise, use the default value for all functions.
    */
    if ( ( pPciCfgOpts->pciConfigInit == TRUE ) &&
         ( pPciCfgOpts->pciMaxLatFunc != NULL ) )
        {
        /* find value by calling user-supplied function */
        maxLatency = (*pPciCfgOpts->pciMaxLatFunc)
                     (pPciLoc->bus,pPciLoc->device,pPciLoc->function,
                      pPciCfgOpts->pciMaxLatPArg);
        }
    else
        {
        /* use default value */
        maxLatency = pSystem->maxLatency;
        }

    /* actually set MAX_LAT for this function */
    pciConfigOutByte (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                      PCI_CFG_SEC_LATENCY, maxLatency);


    /* Clear status bits turn on downstream and upstream (master) mem,IO */

    pciConfigOutLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                      PCI_CFG_COMMAND,
                      (UINT32)(0xffff0000 | PCI_CMD_IO_ENABLE |
                      PCI_CMD_MEM_ENABLE | PCI_CMD_MASTER_ENABLE)
                     );
    }

#if 0 /* TBD MDG */
    /* These should be controlled by pciAutoCfgCtl */
#define PCI_CARDBUS_IO16_SIZE       0x1000
#define PCI_CARDBUS_MEMIO32_SIZE    (1 * 1024 * 1024)
#define PCI_CARDBUS_MEM32_SIZE      (1 * 1024 * 1024)
/******************************************************************************
*
* pciAutoCardBusConfig - set mem and I/O registers for a single PCI-Cardbus bridge
*
* SYNOPSIS
* \cs
* LOCAL void pciAutoCardBusConfig
*     (
*     PCI_AUTO_CONFIG_OPTS * pSystem,     /@ PCI system info @/
*     PCI_LOC              * pPciLoc,     /@ PCI address of this bridge @/
*     PCI_LOC             ** ppPciList,   /@ Pointer to function list pointer @/
*     UINT                 * nSize        /@ Number of remaining functions @/
*     )
* \ce
*
* DESCRIPTION
* This routine sets up memory and I/O base/limit registers for
* an individual PCI-Cardbus bridge.
*
* Cardbus bridges have four windows - 2 memory windows and 2 IO
* windows. The 2 memory windows can be setup individually for either
* prefetchable or non-prefetchable memory accesses.
*
* Since PC Cards can be inserted at any time, and are not necessarily
* present when this code is run, the code does not probe any further
* after encountering a Cardbus bridge. Instead, the code allocates
* default window sizes for the Cardbus bridge. Three windows are used:
*
* \ss
* Memory #0:            Prefetch memory
* Memory #1:            Non-prefetch memory
* IO #0:            IO
* IO #1:            Unused
* \se
*
* Warning: do not sort the include function list before this routine is
* called.  This routine requires each function in the list to be in the
* same order as the probe occurred.
*
* ERRNO
*
* RETURNS: N/A
*
*/
LOCAL void pciAutoCardBusConfig
    (
    PCI_AUTO_CONFIG_OPTS * pSystem, /* PCI system info */
    PCI_LOC * pPciLoc,      /* PCI address of this bridge */
    PCI_LOC **ppPciList,    /* Pointer to function list pointer */
    UINT *nSize         /* Number of remaining functions */
    )
    {
    UINT8 maxLatency;       /* MAX_LAT value */
    UINT dev_vend;
    UINT32 alignedBase;
    UINT32 pciCardBusIo16Size = PCI_CARDBUS_IO16_SIZE;
    UINT32 pciCardBusMemIo32Size = PCI_CARDBUS_MEMIO32_SIZE;
    UINT32 pciCardBusMem32Size = PCI_CARDBUS_MEM32_SIZE;
    UINT32 sizeAdj;

    /* Make sure devices are inactive by placing them in reset */

    pciConfigModifyWord (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                         PCI_CFG_BRIDGE_CONTROL,
                         PCI_CFG_CB_RESET, PCI_CFG_CB_RESET);


    /* If it exists, call the user-defined pre-config pass bridge init */

    if ((pSystem->bridgePreConfigInit) != NULL )
        {
        pciConfigInLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                         PCI_CFG_VENDOR_ID, &dev_vend);

        (pSystem->bridgePreConfigInit) ((PCI_SYSTEM *)pSystem, pPciLoc, dev_vend);
        }

    /* Clear the secondary status bits */

    pciConfigModifyLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                         PCI_CFG_CB_CAP_PTR, 0xffff0000, 0xffff0000);

    /* If Prefetch supported, then configure 32-bit PF Memory Base Addr */

    if ( (pPciLoc->attribute & PCI_AUTO_ATTR_BUS_PREFETCH)
         && (pSystem->pciMem32Size > 0) )
        {

        PCI_AUTO_DEBUG_MSG("pciAutoBusConfig: Configuring prefetch aperture\n",
                           0, 0, 0, 0, 0, 0);

        pciAutoAddrAlign(pSystem->pciMem32,
                         (pSystem->pciMem32 + pSystem->pciMem32Size),
                         pciCardBusMem32Size,
                         &alignedBase);

        PCI_AUTO_DEBUG_MSG("PF Mem Base orig[0x%08x] new[0x%08x] adj[0x%08x]\n",
                           (pSystem->pciMem32),
                           alignedBase,
                           (alignedBase - (pSystem->pciMem32)),
                           0,
                           0,
                           0
                          );

        sizeAdj = alignedBase - (pSystem->pciMem32) + pciCardBusMem32Size;

        (pSystem->pciMem32Used) += sizeAdj;
        (pSystem->pciMem32Size) -= sizeAdj;
        (pSystem->pciMem32) += sizeAdj;

        /* Use Memory base #0 for prefetch memory */

        pciConfigOutLong (pPciLoc->bus,pPciLoc->device,pPciLoc->function,
                          PCI_CFG_CB_MEM_BASE_0, alignedBase);

        PCI_AUTO_DEBUG_MSG("pciAutoBusConfig: PF Mem Base [0x%08x]\n",
                           pSystem->pciMem32, 0, 0, 0, 0, 0);

        /* Set limit register */

        pciConfigOutLong (pPciLoc->bus,pPciLoc->device,pPciLoc->function,
                          PCI_CFG_CB_MEM_LIMIT_0, (pSystem->pciMem32) - 1);

        /* Enable prefetch memory window 0 */

        pciConfigModifyWord (pPciLoc->bus,pPciLoc->device,pPciLoc->function,
                             PCI_CFG_BRIDGE_CONTROL, PCI_CFG_CB_PREFETCH0,
                             PCI_CFG_CB_PREFETCH0);
        }

    /* Configure 16-bit I/O Base Address */

    if ((pSystem->pciIo16Size) > 0)
        {
        pciAutoAddrAlign(pSystem->pciIo16,
                         (pSystem->pciIo16 + pSystem->pciIo16Size),
                         pciCardBusIo16Size,
                         &alignedBase);

        PCI_AUTO_DEBUG_MSG("I/O 16 Base orig[0x%08x] new[0x%08x] adj[0x%08x]\n",
                           (pSystem->pciIo16),
                           alignedBase,
                           (alignedBase - (pSystem->pciIo16)),
                           0,
                           0,
                           0
                          );

        sizeAdj = alignedBase - (pSystem->pciIo16) + pciCardBusIo16Size;

        (pSystem->pciIo16Used) += sizeAdj;
        (pSystem->pciIo16Size) -= sizeAdj;
        (pSystem->pciIo16) += sizeAdj;

        /* Use IO base #0 for 16-bit I/O */

        pciConfigOutLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                          PCI_CFG_CB_IO_BASE_0, alignedBase);

        PCI_AUTO_DEBUG_MSG("pciAutoBusConfig: IO16 Base Address [0x%08x]\n",
                           pSystem->pciIo16, 0, 0, 0, 0, 0);

        /* Set limit register */

        pciConfigOutLong (pPciLoc->bus,pPciLoc->device,pPciLoc->function,
                          PCI_CFG_CB_IO_LIMIT_0, (pSystem->pciIo16) - 1);

        }

    /* Configure 32-bit Non-prefetchable Memory Base Address */

    if ((pSystem->pciMemIo32Size) > 0)
        {

        pciAutoAddrAlign(pSystem->pciMemIo32,
                         (pSystem->pciMemIo32 + pSystem->pciMemIo32Size),
                         pciCardBusMemIo32Size,
                         &alignedBase);

        PCI_AUTO_DEBUG_MSG("Memory Base orig[0x%08x] new[0x%08x] adj[0x%08x]\n",
                           (pSystem->pciMemIo32),
                           alignedBase,
                           (alignedBase - (pSystem->pciMemIo32)),
                           0,
                           0,
                           0
                          );

        sizeAdj = alignedBase - (pSystem->pciMemIo32) + pciCardBusMemIo32Size;

        (pSystem->pciMemIo32Used) += sizeAdj;
        (pSystem->pciMemIo32Size) -= sizeAdj;
        (pSystem->pciMemIo32) += sizeAdj;

        /* Use Memory base #1 for 32-bit non-prefetch memory */

        pciConfigOutLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                          PCI_CFG_CB_MEM_BASE_1, alignedBase);

        PCI_AUTO_DEBUG_MSG("pciAutoBusConfig: Mem Base Address [0x%08x]\n",
                           pSystem->pciMemIo32, 0, 0, 0, 0, 0);

        /* Set limit register */

        pciConfigOutLong (pPciLoc->bus,pPciLoc->device,pPciLoc->function,
                          PCI_CFG_CB_MEM_LIMIT_1, (pSystem->pciMemIo32) - 1);

        /* Disable prefetch memory window 1 */

        pciConfigModifyWord (pPciLoc->bus,pPciLoc->device,pPciLoc->function,
                             PCI_CFG_BRIDGE_CONTROL, PCI_CFG_CB_PREFETCH1,
                             0);
        }

    /* Initialize primary and secondary PCI-CardBus bridge latency timers */

    /*
    *  If MAX_LAT function supplied by BSP, call it to calculate
    *  the value of MAX_LAT for each function individually.
    *  Otherwise, use the default value for all functions.
    */
    if ( ( pPciCfgOpts->pciConfigInit == TRUE ) &&
         ( pPciCfgOpts->pciMaxLatFunc != NULL ) )
        {
        /* find value by calling user-supplied function */
        maxLatency = (*pPciCfgOpts->pciMaxLatFunc)
                     (pPciLoc->bus,pPciLoc->device,pPciLoc->function,
                      pPciCfgOpts->pciMaxLatPArg);
        }
    else
        {
        /* use default value */
        maxLatency = pSystem->maxLatency;
        }

    /* actually set MAX_LAT for this function */
    pciConfigOutByte (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                      PCI_CFG_SEC_LATENCY, maxLatency);


    /* Clear status bits turn on downstream and upstream (master) mem,IO */

    pciConfigOutLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                      PCI_CFG_COMMAND,
                      (UINT32)(0xffff0000 | PCI_CMD_IO_ENABLE |
                      PCI_CMD_MEM_ENABLE  | PCI_CMD_MASTER_ENABLE)
                     );
    }
#endif
/*******************************************************************************
*
* pciAutoAddrAlign - align a PCI address and check boundary conditions
*
* This routine handles address alignment/checking.
*
* RETURNS: OK, or ERROR if available memory has been exceeded.
*
* ERRNO
*
*/

STATUS pciAutoAddrAlign
    (
    UINT32 base,        /* base of available memory */
    UINT32 limit,       /* last addr of available memory */
    UINT32 reqSize,     /* required size */
    UINT32 *pAlignedBase    /* output: aligned address put here */
    )
    {
    UINT32 sizeMask;
    UINT32 alignAdjust;

    /* make the size mask */

    sizeMask = reqSize - 1;

    PCI_AUTO_DEBUG_MSG("pciAutoAddrAlign: sizemask[%08x]\n",sizeMask,0,0,0,0,0);
    /* see if the address needs to be adjusted */

    if ((base & sizeMask) > 0)
        {
        alignAdjust = reqSize - (base & sizeMask);
        PCI_AUTO_DEBUG_MSG ("pciAutoAddrAlign: adjustment [%08x]\n",alignAdjust,
                            0,0,0,0,0);
        }
    else
        {
        PCI_AUTO_DEBUG_MSG("pciAutoAddrAlign: already aligned\n",0,0,0,0,0,0);
        alignAdjust = 0;
        }

    /* see if the aligned base exceeds the resource boundary */

    if ( ((base + alignAdjust) < base) ||
         ((base + alignAdjust) > limit) )
        {
        PCI_AUTO_DEBUG_MSG ("pciAutoAddrAlign: base + adjustment [%08x]"
                            " exceeds limit [%08x]\n", (base + alignAdjust),
                            limit,
                            0,0,0,0);
        if ((base + alignAdjust) > limit) {
            return ERROR_EXCESS_SIZE;
        } else {
            return ERROR;
        }

        }

    *pAlignedBase = base + alignAdjust;

    /* see if the aligned base+size exceeds the resource boundary */

    if ( ((base + alignAdjust + reqSize) < base) ||
         ((base + alignAdjust + reqSize) > limit) )
        {
        PCI_AUTO_DEBUG_MSG ("pciAutoAddrAlign: base + adjustment + req [%08x]"
                            " exceeds limit [%08x]\n",
                            (base + alignAdjust + reqSize),limit,0,0,0,0);
        if ((base + alignAdjust + reqSize) > limit) {
            return ERROR_EXCESS_SIZE;
        } else {
            return ERROR;
        }
        }

    PCI_AUTO_DEBUG_MSG ("pciAutoAddrAlign: new aligned base [%08x]\n",
                        (base + alignAdjust),0,0,0,0,0);

    return OK;
    }

/**********************************************************************
*
* pciFBBFuncSet - enable Fast Back To Back on specified function
*
* pciFBBFuncSet() turns on FBB on the specified function.  This function
* is intended for use with pciConfigForeachFunc().
*
* If the device fails to return a valid status word, then
* return ERROR without modifying FBB enable.
*
* ERRNO: N/A
*
* RETURNS: OK, or ERROR if fast back to back is not supported.
*/

LOCAL STATUS pciFBBFuncSet
    (
    UINT    bus,        /* bus */
    UINT    device,     /* device */
    UINT    function,   /* function */
    void *  pArg        /* not used */
    )
    {
    UINT16  pciStatus;  /* PCI status register */

    /* check if this device supports Fast Back To Back */
    pciConfigInWord (bus, device, function, PCI_CFG_STATUS, &pciStatus);
    if ( ! ( pciStatus & PCI_STATUS_FAST_BB ) )
        return(ERROR);

    /* turn FBB on */
    pciConfigModifyLong (bus, device, function, PCI_CFG_COMMAND,
                         (PCI_CMD_MASK | PCI_CMD_FBTB_ENABLE),
                         PCI_CMD_FBTB_ENABLE);
    return(OK);
    }

/**********************************************************************
*
* pciFBBFuncCheck - check Fast Back To Back on specified function
*
* pciFBBFuncCheck() checks whether the function supports FBB
*
* If the device fails to return a valid status word, then
* return ERROR without modifying FBB enable.
*
* ERRNO: N/A
*
* RETURNS: OK, or ERROR if fast back to back is not supported.
*
*/

LOCAL STATUS pciFBBFuncCheck
    (
    UINT    bus,        /* bus */
    UINT    device,     /* device */
    UINT    function,   /* function */
    void *  pArg        /* not used */
    )
    {
    UINT16  pciStatus;  /* PCI status register */

    /* check if this device supports Fast Back To Back */
    pciConfigInWord (bus, device, function, PCI_CFG_STATUS, &pciStatus);
    if ( ! ( pciStatus & PCI_STATUS_FAST_BB ) )
        return(ERROR);

    return(OK);
    }

/**********************************************************************
*
* pciFBBFuncClear - disable Fast Back To Back on specified function
*
* pciFBBFuncClear() turns off FBB on the specified function.
*
* ERRNO: N/A
*
* RETURNS: OK, always.
*
*/

LOCAL STATUS pciFBBFuncClear
    (
    UINT    bus,        /* bus */
    UINT    device,     /* device */
    UINT    function,   /* function */
    void *  pArg        /* not used */
    )
    {
    UINT16  pciStatus;  /* PCI status register */

    /* check if this device supports Fast Back To Back */
    pciConfigInWord (bus, device, function, PCI_CFG_STATUS, &pciStatus);
    if ( pciStatus & PCI_STATUS_FAST_BB )
        {
        /* turn FBB off */
        pciConfigModifyLong (bus, device, function, PCI_CFG_COMMAND,
                             (PCI_CMD_MASK | PCI_CMD_FBTB_ENABLE), 0);
        }
    return(OK);
    }

/**********************************************************************
*
* pciAutoConfigFBBEnable - enable Fast Back To Back
*
* pciAutoConfigFBBEnable() discovers the functions present
* and verifies that each of them can support FBB.  At any time,
* if it finds a device which cannot handle FBB, it returns ERROR.
* If all listed devices do support FBB, then it makes a second
* pass through the list, and enables FBB on all listed functions.
*
* If any device fails to return a valid status word, then
* pciAutoConfigFBBEnable() will immediately return ERROR without
* modifying FBB enable on any cards which have already been enabled
* and without finishing the list.
*
* ERRNO:
* EINVAL if FBB is not enabled
*
* RETURNS: OK, or ERROR if fast back to back has not been enabled.
*
*/

LOCAL STATUS pciAutoConfigFBBEnable
    (
    PCI_AUTO_CONFIG_OPTS *  pOpts /* system configuration information */
    )
    {
    STATUS status;

    if ( pOpts->pciFBBEnable != TRUE )
        {
        errnoSet(EINVAL);
        return(ERROR);
        }

    /* discover devices and check for FBB implemented */
    status = pciFBBFuncCheck(0,0,0,NULL);
    if ( status != OK )
        return(ERROR);

    status = pciConfigForeachFunc(0, TRUE, (PCI_FOREACH_FUNC)pciFBBFuncCheck, NULL);
    if ( status != OK )
        return(ERROR);

    /* all are FBB aware, so enable devices */
    status = pciFBBFuncSet(0,0,0,NULL);
    if ( status != OK )
        return(status);

    status = pciConfigForeachFunc(0, TRUE, (PCI_FOREACH_FUNC)pciFBBFuncSet, NULL);
    return(status);
    }

/**********************************************************************
*
* pciAutoConfigFBBDisable - disable Fast Back To Back
*
* pciAutoConfigFBBDisable() goes through the list of PCI functions
* and disables FBB on all listed functions.
*
* If any device fails to return a valid status word, then
* pciAutoConfigFBBDisable() will continue disabling FBB on the
* remaining functions, but will return ERROR after finishing.
*
* ERRNO: N/A
*
* RETURNS: OK, or ERROR if fast back to back was not previously enabled.
*
*/

LOCAL STATUS pciAutoConfigFBBDisable
    (
    PCI_AUTO_CONFIG_OPTS *  pOpts /* system configuration information */
    )
    {
    STATUS status;

    if ( pOpts->pciFBBEnable != TRUE )
        {
        return(ERROR);
        }
    pOpts->pciFBBEnable = FALSE;

    /* discover devices and disable FBB */
    status = pciConfigForeachFunc(0, TRUE, (PCI_FOREACH_FUNC)pciFBBFuncClear, NULL);

    return(status);
    }

/* backward compatibility functions */

/**********************************************************************
*
* pciAutoConfigCopyback - copy data back to the pSystem struct
*
* pciAutoConfigLib backwards compatibility function.
*
* The old interface for pciAutoConfigLib allowed the user to
* look at the values of pSystem after pciAutoConfig() had
* run.  This let them look at various values and determine
* system behavior based on how they changed.  The new interface
* does not have this behavior.
*
* To insure backwards compatibility, every time any of the functions
* visible to the user is called, when the function completes, it
* copies information back to the pSystem structure by calling
* this routine.
*
* ERRNO: N/A
*
* RETURNS: N/A
*
*/

LOCAL void pciAutoConfigCopyback
    (
    PCI_AUTO_CONFIG_OPTS *  pOpts,  /* system configuration info */
    PCI_SYSTEM *        pSystem /* PCI_SYSTEM to copy into */
    )
    {
    if ( pSystem == (PCI_SYSTEM *)pOpts )
        return;

    pSystem->pciMem32 = pOpts->pciMem32;
    pSystem->pciMem32Size = pOpts->pciMem32Size;
    pSystem->pciMemIo32 = pOpts->pciMemIo32;
    pSystem->pciMemIo32Size = pOpts->pciMemIo32Size;
    pSystem->pciIo32 = pOpts->pciIo32;
    pSystem->pciIo32Size = pOpts->pciIo32Size;
    pSystem->pciIo16 = pOpts->pciIo16;
    pSystem->pciIo16Size = pOpts->pciIo16Size;
    pSystem->maxBus = pOpts->maxBus;
    pSystem->cacheSize = pOpts->cacheSize;
    pSystem->maxLatency = pOpts->maxLatency;
    pSystem->autoIntRouting = pOpts->autoIntRouting;
    pSystem->includeRtn = pOpts->includeRtn;
    pSystem->bridgePreConfigInit = pOpts->bridgePreConfigInit;
    pSystem->bridgePostConfigInit = pOpts->bridgePostConfigInit;
    pSystem->pciRollcallRtn = pOpts->pciRollcallRtn;
    pSystem->intAssignRtn = (void *)pOpts->intAssignRtn;
    }

/******************************************************************************
*
* pciAutoConfig - automatically configure all nonexcluded PCI headers (obsolete)
*
* This routine is obsolete.  It is included for backward compatibility only.
* It is recommended that you use the pciAutoCfg() interface instead of
* this one.
*
* Top level function in the PCI configuration process.
*
* For all nonexcluded PCI functions on all PCI bridges, this routine
* will automatically configure the PCI configuration headers for PCI
* devices and subbridges.  The fields that are programmed are:
*
* \ml
* \m 1.
* Status register.
* \m 2.
* Command Register.
* \m 3.
* Latency timer.
* \m 4.
* Cache Line size.
* \m 5.
* Memory and/or I/O base address and limit registers.
* \m 6.
* Primary, secondary, subordinate bus number (for PCI-PCI bridges).
* \m 7.
* Expansion ROM disable.
* \m 8.
* Interrupt Line.
* \me
*
* ALGORITHM
* Probe PCI config space and create a list of available PCI functions.
* Call device exclusion function, if registered, to exclude/include device.
* Disable all devices before we initialize any.
* Allocate and assign PCI space to each device.
* Calculate and set interrupt line value.
* Initialize and enable each device.
*
* RETURNS: N/A
*
* ERRNO
*
*/

void pciAutoConfig
    (
    PCI_SYSTEM * pSystem    /* PCI system to configure */
    )
    {
    PCI_AUTO_CONFIG_OPTS *pCookie;

    /* Initialize the allocation index of 64-bit PCIe region */
    if (currentPcieController == PCIE_CONTROLLER_0) {
        pcie0_64bit_region_alloc_index = 0;
    } else {
        pcie1_64bit_region_alloc_index = 0;
    }

    pCookie = pciAutoConfigLibInit(NULL);
    pciAutoCfgCtl(pCookie, PCI_PSYSTEM_STRUCT_COPY, (void *)pSystem);
    pciAutoCfgFunc(pCookie);
    pciAutoConfigCopyback(pCookie, pSystem);
    }
