/* pciAutoConfigLib.c - PCI bus scan and resource allocation facility */

/* Copyright 1997,1998,1999 Motorola, Inc. All Rights Reserved */
/* Copyright 1997-2004 Wind River Systems, Inc. All Rights Reserved */

/* $Id: pciAutoConfigLib.c,v 1.3 2011/07/21 16:14:49 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01s,08nov04,mdo  Documentation fixes for apigen
01r,11nov02,agf  update PCI device mode logic
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
PCI-to-PCI Bridges. Bridges are identified and properly numbered
before a recursive scan identifies all resources on the bus implemented
by the bridge. Post-scan configuration of the subordinate bus number
is performed. 

Resource requirements of each device are identified and allocated according
to system resource pools that are specified by the BSP Developer. Devices may
be conditionally excluded, and interrupt routing information obtained via
optional routines provided by the BSP Developer.

GENERAL ALGORITHM

Autoconfiguration takes place in two phases. In the first phase, all
devices and subordinate busses in a given system are scanned and each device 
that is found causes an entry to be created in the "Probelist" or list of 
devices found during the probe/configuration process. 

In the second phase each device that is on the Probelist is checked to see
if it has been excluded from automatic configuration by the BSP developer.
If a particular function has not been excluded, then it is first disabled. 
The Base Address Registers of the particular function are read to ascertain
the resource requirements of the function. Each resource requirement is
checked against available resources in the applicable pool based on size
and alignment constraints.

After all functions on the Probelist have been processed, each function
and it's appropriate Memory or I/O decoder(s) is enabled for operation.

HOST BRIDGE DETECTION/CONFIGURATION

Note that the PCI Host Bridge is automatically excluded from configuration
by the autoconfig routines as it is often already configured as part of
the system bootstrap device configuration.

PCI-PCI BRIDGE DETECTION/CONFIGURATION

Busses are scanned by first writing the primary, secondary, and subordinate
bus information into the bridge that implements the bus.  Specifically,
the primary and secondary bus numbers are set to their corresponding value, 
and the subordinate bus number is set to 0xFF, because the final number of 
sub-busses is not known.  The subordinate bus number is also updated to
indicate the highest numbered sub-bus that was scanned once the scan is
complete. 

GENERIC DEVICE DETECTION/CONFIGURATION

The autoconfiguration library creates a list of devices during the process
of scanning all of the busses in a system.  Devices with vendor
IDs of 0xFFFF and 0x0000 are skipped.  Once all busses have been 
scanned, all non-excluded devices are then disabled prior to configuration.

Devices that are not excluded will have Resources allocated according
to Base Address Registers that are implemented by the device and 
available space in the applicable resource pool. PCI 'Natural' alignment
constraints are adhered to when allocating resources from pools.

Also initialized are the cache line size register and the latency 
timer.  Bus mastering is unconditionally enabled. 

If an interrupt assignment routine is registered, then the interrupt
pin register of the PCI Configuration space is passed to this routine
along with the bus, device, and function number of the device under
consideration.

There are two different schemes to determine when the BSP interrupt
assignment routine is called by autoconfig. The call is done either
only for bus-0 devices or for all devices depending upon how the
autoIntRouting is set by the BSP developer (see the section
"INTERRUPT ROUTING ACROSS PCI-TO-PCI BRIDGES" below for more details).

The interrupt level number returned by this routine is then written
into the interrupt line register of the PCI Configuration Space for
subsequent use by device drivers. If no interrupt assignment routine
is registered, 0xFF is written into the interrupt line register,
specifying an unknown interrupt binding.

Lastly, the functions are enabled with what resources were able to be
provided from the applicable resource pools.

RESOURCE ALLOCATION

Resource pools include the 32-bit Prefetchable Memory pool,
the 32-bit Non-prefetchable Memory ("MemIO") pool, the 32-bit I/O pool,
and the 16-bit I/O allocation pool. The allocation in each pool begins at
the specified base address and progresses to higher numbered addresses. 
Each allocated address adheres to the PCI 'natural' alignment constraints
of the given resource requirement specified in the Base Address Register.

DATA STRUCTURES

In order to utilize the PCI Autoconfig facility, the user instantiates a 
data structure that describes resource pools that the autoconfig facility 
uses.  This structure is the PCI_SYSTEM structure, and has two groups of
information that are specifiable by the BSP Developer. 

PCI RESOURCE POOLS

The first group of information is the Memory and I/O resources, that are
available in the system and that autoconfig can use to allocate to functions. 
These resource pools consist of a base address and size. The base address 
specified here should be the address relative to the PCI bus. Each of these
values in the PCI_SYSTEM data structure is described below:

\is
\i pciMem32
Specifies the 32-bit prefetchable memory pool base address. 
Normally, this is given by the BSP constant PCI_MEM_ADRS

\i pciMem32Size
Specifies the 32-bit prefetchable memory pool size.
Normally, this is given by the BSP constant PCI_MEM_SIZE

\i pciMemIo32
Specifies the 32-bit non-prefetchable memory pool base address.
Normally, this is given by the BSP constant PCI_MEMIO_ADRS

\i pciMemIo32Size
Specifies the 32-bit non-prefetchable memory pool size
Normally, this is given by the BSP constant PCI_MEMIO_SIZE

\i pciIo32
Specifies the 32-bit I/O pool base address. 
Normally, this is given by the BSP constant PCI_IO_ADRS

\i pciIo32Size
Specifies the 32-bit I/O pool size. 
Normally, this is given by the BSP constant PCI_IO_SIZE

\i pciIo16
Specifies the 16-bit I/O pool base address. 
Normally, this is given by the BSP constant PCI_ISA_IO_ADDR

\i pciIo16Size
Specifies the 16-bit I/O pool size. 
Normally, this is given by the BSP constant PCI_ISA_IO_SIZE

\ie

PREFETCH MEMORY ALLOCATION

The pciMem32 pointer is assumed to point to a pool of prefetchable PCI
memory. If the size of this pool is non-zero, then prefetch memory will
be allocated to devices that request it given that there is enough 
memory in the pool to satisfy the request, and the host bridge or 
PCI-to-PCI bridge that implements the bus that the device resides on is
capable of handling prefetchable memory. If a device requests it, and 
no prefetchable memory is available or the bridge implementing the bus
does not handle prefetchable memory then the request will be attempted
from the non-prefetchable memory pool. 

PCI-to-PCI bridges are queried as to whether they support prefetchable
memory by writing a non-zero value to the prefetchable memory base
address register and reading back a non-zero value. A zero value would
indicate the bridge does not support prefetchable memory.

BSP-SPECIFIC ROUTINES

Four routines can be provided by the BSP Developer to customize the degree
to which the system can be automatically configured.  These routines are
normally put into a file called sysBusPci.c in the BSP directory. The trivial
cases of each of these routines are shown in the USAGE section below to
illustrate the API to the BSP Developer. 

DEVICE INCLUSION

The device inclusion routine is specified by assigning a function pointer
to the includeRtn member of the PCI_SYSTEM structure: 

\cs
    sysParams.includeRtn = sysPciAutoconfigInclude;
\ce

This optional user-supplied routine takes as input both the
bus-device-function tuple, and a 32-bit quantity containing both the PCI
vendorID and deviceID of the function. The function prototype for this
function is shown below:

\cs
    STATUS sysPciAutoconfigInclude
    (
    PCI_SYSTEM *pSys,
    PCI_LOC *pLoc, 
    UINT devVend
    );
\ce

This optional user-specified routine is called by PCI AutoConfig for 
each and every function encountered in the scan phase. The BSP developer
may use any combination of the input data to ascertain whether a device
is to be excluded from the autoconfig process.  The exclusion routine 
then returns ERROR if a device is to be excluded, and OK if a device is 
to be included in the autoconfiguration process.

Note that PCI-to-PCI Bridges may not be excluded, regardless of the value
returned by the BSP device inclusion routine. The return value is ignored for
PCI-to-PCI bridges. 

The Bridge device will be always be configured with proper primary,
secondary, and subordinate bus numbers in the device scanning phase
and proper I/O and Memory aperture settings in the configuration phase
of autoconfig regardless of the value returned by the BSP device inclusion 
routine.

INTERRUPT ASSIGNMENT

The interrupt assignment routine is specified by assigning a function pointer
to the intAssignRtn member of the PCI_SYSTEM structure: 

\cs
    sysParams.intAssignRtn = sysPciAutoconfigIntrAssign;
\ce

This optional user-specified routine takes as input both the bus-device-function
tuple, and an 8-bit quantity containing the contents of the interrupt Pin 
register from the PCI configuration header of the device under consideration.
The interrupt pin register specifies which of the four PCI Interrupt request
lines available are connected.  The function prototype for this function is 
shown below:

\cs
    STATUS sysPciAutoconfigIntrAssign
    (
    PCI_SYSTEM *pSys,
    PCI_LOC *pLoc, 
    UCHAR pin
    );
\ce

This routine may use any combination of these data to ascertain the 
interrupt level. This value is returned from the function, and should 
be programmed into the interrupt line register of the function's PCI 
configuration header. In this manner, device drivers may subsequently 
read this register in order to calculate the appropriate 
interrupt vector which to attach an interrupt service routine.

INTERRUPT ROUTING ACROSS PCI-TO-PCI BRIDGES

PCI Autoconfig allows use of two interrupt routing strategies for handling
devices that reside across a PCI-to-PCI Bridge. The BSP-specific interrupt
assignment routine described in the above section is called for all devices
that reside on bus 0. For devices residing across a PCI-to-PCI bridge, one
of two supported interrupt routing strategies may be selected by setting
the autoIntRouting variable in the PCI_SYSTEM structure to the boolean
value TRUE or FALSE:

\is
\i TRUE
If autoIntRouting is set to TRUE, then autoconfig only calls the BSP 
interrupt routing routine for devices on bus number 0. If a device resides
on a higher numbered bus, then a cyclic algorithm is applied to the IRQs
that are routed through the bridge. The algorithm is based on computing a
'route offset' that is the device number modulo 4 for every bridge device 
that is traversed. This offset is used with the device number and interrupt
pin register of the device of interest to compute the contents of the
interrupt line register.

\i FALSE
If autoIntRouting is set to FALSE, then autoconfig calls the BSP
interrupt assignment routine to do all interrupt routing regardless
of the bus on which the device resides. The return value represents the
contents of the interrupt line register in all cases.
\ie

BRIDGE CONFIGURATION

The bridge pre-configuration pass initialization routine is provided
so that the BSP Developer can initialize a bridge device prior to the
configuration pass on the bus that the bridge implements. This routine
is specified by assigning a function pointer to the bridgePreInit
member of the PCI_SYSTEM structure: 

\cs
    sysParams.bridgePreInit
    = sysPciAutoconfigPreEnumBridgeInit;
\ce

This optional user-specified routine takes as input both the bus-device-function
tuple, and a 32-bit quantity containing both the PCI deviceID and vendorID of 
the device. The function prototype for this function is shown below:

\cs
    STATUS sysPciAutoconfigPreEnumBridgeInit 
    (
    PCI_SYSTEM *pSys,
    PCI_LOC *pLoc, 
    UINT devVend
    );
\ce

This routine may use any combination of these input data 
to ascertain any special initialization requirements of a particular 
type of bridge at a specified geographic location.

The bridge post-configuration pass initialization routine is provided
so that the BSP Developer can initialize the bridge device after the bus
that the bridge implements has been enumerated. This routine
is specified by assigning a function pointer to the bridgePostInit
member of the PCI_SYSTEM structure: 

\cs
    sysParams.bridgePostInit
    = sysPciAutoconfigPostEnumBridgeInit;
\ce

This optional user-specified routine takes as input both the bus-device-function
tuple, and a 32-bit quantity containing both the PCI deviceID and vendorID of 
the device. The function prototype for this function is shown below:

\cs
     STATUS sysPciAutoconfigPostEnumBridgeInit 
    (
    PCI_SYSTEM *pSys,
    PCI_LOC *pLoc, 
    UINT devVend
    );
\ce

This routine may use any combination of these input data 
to ascertain any special initialization requirements of a particular 
type of bridge at a specified geographic location.

HOST BRIDGE CONFIGURATION

The PCI Local Bus Specification, rev 2.1 does not specify the content or 
initialization requirements of the configuration space of PCI Host Bridges. 
Due to this fact, no host bridge specific assumptions are made by auto-
config and any PCI Host Bridge initialization that must be done before either
scan or configuration of the bus must be done in the BSP. Comments
illustrating where this initialization could be called in relation to 
invoking the pciAutoConfig() routine are in the USAGE section below.

LIBRARY CONFIGURATION MACROS

The following four macros can be defined by the BSP Developer in config.h
to govern the operation of the autoconfig library.

\is
\i PCI_AUTO_MAX_FUNCTIONS
Defines the maximum number of functions that can be stored in the probe list
during the autoconfiguration pass. The default value for this define is 32,
but this may be overridden by defining PCI_AUTO_MAX_FUNCTIONS in config.h.

\i PCI_AUTO_STATIC_LIST
If defined, then a statically allocated array of size PCI_AUTO_MAX_FUNCTION
instances of the PCI_LOC structure will be instantiated. 

\i PCI_AUTO_RECLAIM_LIST
This define may only be used if PCI_AUTO_STATIC_LIST is not defined. If
defined, this allows the autoconfig routine to perform a free() operation 
on a dynamically allocated probe list. Note that if PCI_AUTO_RECLAIM_LIST
is defined and PCI_AUTO_STATIC_LIST is also, a compiler error will be 
generated.

\ie

USAGE

The following code sample illustrates the usage of the PCI_SYSTEM structure
and invocation of the autoconfig library.  Note that the example BSP-specific
routines are merely stubs. The code in each routine varies by BSP and 
application.

\cs

#include "pciAutoConfigLib.h"

LOCAL PCI_SYSTEM sysParams;

void sysPciAutoConfig (void)
    {

    /@ 32-bit Prefetchable Memory Space @/

    sysParams.pciMem32 = PCI_MEM_ADRS;
    sysParams.pciMem32Size = PCI_MEM_SIZE;

    /@ 32-bit Non-prefetchable Memory Space @/

    sysParams.pciMemIo32 = PCI_MEM_ADRS;
    sysParams.pciMemIo32Size = PCI_MEM_SIZE;

    /@ 16-bit ISA I/O Space @/

    sysParams.pciIo16 = PCI_ISA_IO_ADRS;
    sysParams.pciIo16Size = PCI_ISA_IO_SIZE;

    /@ 32-bit PCI I/O Space @/

    sysParams.pciIo32 = PCI_IO_ADRS;
    sysParams.pciIo32Size = PCI_IO_SIZE;

    /@ Configuration space parameters @/

    sysParams.maxBus = 0;
    sysParams.cacheSize = ( _CACHE_ALIGN_SIZE / 4 );
    sysParams.maxLatency = PCI_LAT_TIMER;

    /@
     * Interrupt routing strategy
     * across PCI-to-PCI Bridges
     @/

    sysParams.autoIntRouting = TRUE;

    /@ Device inclusion and interrupt routing routines @/

    sysParams.includeRtn = sysPciAutoconfigInclude;
    sysParams.intAssignRtn = sysPciAutoconfigIntrAssign;

    /@
     * PCI-to-PCI Bridge Pre-
     * and Post-enumeration init
     * routines
     @/

    sysParams.bridgePreConfigInit =
    sysPciAutoconfigPreEnumBridgeInit;
    sysParams.bridgePostConfigInit =
    sysPciAutoconfigPostEnumBridgeInit;

    /@
     * Perform any needed PCI Host Bridge
     * Initialization that needs to be done
     * before pciAutoConfig is invoked here 
     * utilizing the information in the 
     * newly-populated sysParams structure. 
     @/

    pciAutoConfig (&sysParams);

    /@
     * Perform any needed post-enumeration
     * PCI Host Bridge Initialization here
     * utilizing the information in the 
     * sysParams structure that has been 
     * updated as a result of the scan 
     * and configuration passes. 
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

Note that in the above example, the macro _CACHE_ALIGN_SIZE is utilized. This
macro is implemented for all supported architectures and is located in the
<architecture>.h file in .../target/h/arch/<architecture>. The value of the
macro indicates the cache line size in bytes for the particular architecture.
For example, the PowerPC architecture defines this macro to be 32, while
the ARM 810 defines it to be 16. The PCI cache line size field and
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

\tb PCI Local Bus Specification, Revision 2.1, June 1, 1996 
\tb PCI Local Bus PCI to PCI Bridge Architecture Specification, Revision 1.0, 
April 5, 1994"

INTERNAL: SIMPLE TEXT-BASED DEBUG SUPPORT

Note that the macro PCI_AUTO_DEBUG may be defined, and the macro
PCI_AUTO_DEBUG_MSG utilized. PCI_AUTO_DEBUG_MSG is identical to a function 
call to logMsg() in function in that it calls _func_logMsg() with the 
string and six parameters passed to it. The macro also invokes taskDelay 
to allow the debug string to be sent with minimal interruption.

Also note that the macro PCI_AUTO_DEBUG initializes a global variable 
pciAutoDebug to a non-zero value. Display of debug messages may be turned
on and off during runtime by manipulating this variable. If the variable is
set to zero, messages will not be displayed.

INTERNAL: ATTRIBUTES

Attributes are reserved for use by the autoconfiguration routines. There
is presently no user-level API at this time to access attributes for a
particular device.  The BSP-specific device exclusion routine affects 
the attributes indirectly by specifying which devices are to be excluded 
from the scan and configuration process.

Attributes are divided into device attributes and bridge attributes.  The first 
group below describes device attributes.

\is
\i PCI_AUTO_ATTR_DEV_EXCLUDE
Specifies that a device is to be excluded from the automatic scan
and configuration process
\i PCI_AUTO_ATTR_DEV_DISPLAY
Specifies that a device is a display device
\i PCI_AUTO_ATTR_DEV_PREFETCH
Specifies that a device has requested Prefetchable PCI memory
\ie

The second group below describes Bridge attributes. 

\is
\i PCI_AUTO_ATTR_BUS_PREFETCH
Specifies that the bridge device supports Prefetchable Memory behind the bridge
\i PCI_AUTO_ATTR_BUS_PCI
Specifies that the bridge device is a PCI-to-PCI bridge and implements a PCI bus
\i PCI_AUTO_ATTR_BUS_HOST
Specifies that the bridge device is a PCI Host bridge and implements a PCI bus
\i PCI_AUTO_ATTR_BUS_ISA
Specifies that the bridge device is an ISA bridge implements an ISA bus
\i PCI_AUTO_ATTR_BUS_4GB_IO
Specifies that the bridge device supports 32-bit I/O Addressing
behind the bridge
\ie


INTERNAL: ATTRIBUTE INHERITANCE

Devices that reside on a particular bus automatically inherit the attributes 
of the bridge (Host or PCI-to-PCI) that implements that bus. This allows
devices to take advantage of the fact that, for example, a PCI-to-PCI bridge
implements full 32-bit PCI I/O. Note that device attributes, such as the
inclusion attribute, are not inherited.

*/


/* includes */

#include "vxWorks.h"
#include "logLib.h"
#include "taskLib.h"
#include "string.h"
#include "dllLib.h"
#include "config.h"

#include "drv/pci/pciConfigLib.h"
#include "pciAutoConfigLib.h"

#undef  PASS1        /* SB-1250 pass 1 silicon */

#ifdef PASS1
/* The SB-1250 first pass silicon has bugs requiring special treatment
   of the base and limit registers of the LDT host bridge.  No device
   address space should be assigned to the cache line (32 bytes)
   following each.  Also, the values of the limit registers specify
   the next unused address, not the last used address. */

/* Since this is temporary code, we assume that we can identify the
   LDT host bridge by its secondary bus number of 1 (guaranteed by the
   current numbering algorithm).  Note that identification based on
   vendor/device id must deal with the possibility of double-hosted
   chains. */
#define SB1250_LDT_BUS  1
#endif

/* local defines */

#define NO_ALLOCATION  0xffffffff

#define PCI_CONFIG_ABSENT_F 0xffff
#define PCI_CONFIG_ABSENT_0 0x0000

/* local configuration defines */

#define PCI_AUTO_STATIC_LIST

#undef PCI_AUTO_RECLAIM_LIST

#ifndef PCI_AUTO_MAX_FUNCTIONS
# define PCI_AUTO_MAX_FUNCTIONS 32
#endif /* PCI_AUTO_MAX_FUNCTIONS */

IMPORT FUNCPTR _func_logMsg;

#define PCI_AUTO_DEBUG_MSG(s, a, b, c, d, e, f) \
    { \
    if ((pciAutoDebug == TRUE) && (_func_logMsg != NULL)) \
        { \
    (*_func_logMsg)(s, a, b, c, d, e, f); \
        taskDelay(10); \
        } \
    }

/* typedefs */

/* globals */

#ifdef PCI_AUTO_DEBUG
BOOL pciAutoDebug = TRUE;
#else
BOOL pciAutoDebug = FALSE;
#endif

IMPORT int pciMaxBus;
IMPORT int pciDeviceMode;

/* locals */

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

LOCAL int loop;

/* forward declarations */

LOCAL PCI_LOC * pciAutoListCreate ( PCI_SYSTEM * pSystem, int *pListSize);
LOCAL UINT pciAutoBusProbe ( PCI_SYSTEM * pSystem, UINT priBus,
    UINT secBus, PCI_LOC*  pPciLoc, PCI_LOC** ppPciList,
    int * pListSize);
LOCAL UINT pciAutoDevProbe ( PCI_SYSTEM * pSystem, UINT bus,
    UCHAR offset, UCHAR inheritAttrib, PCI_LOC **ppPciList, int * pListSize);
LOCAL void pciAutoFuncConfigAll ( PCI_SYSTEM * pSystem,
    PCI_LOC *pPciList, UINT nSize);
LOCAL UCHAR pciAutoIntAssign ( PCI_SYSTEM * pSystem, PCI_LOC * pFunc);
LOCAL void pciAutoDevConfig ( PCI_SYSTEM * pSystem, UINT bus,
    PCI_LOC **ppPciList, UINT *nSize);
LOCAL void pciAutoFuncConfig ( PCI_SYSTEM * pSystem, PCI_LOC * pPciFunc);
LOCAL UINT pciAutoIoAlloc ( PCI_SYSTEM * pPciSys, PCI_LOC *pPciFunc,
    UINT *pAlloc, UINT nSize);
LOCAL UINT pciAutoMemAlloc ( PCI_SYSTEM * pPciSys, PCI_LOC * pPciFunc,
    UINT * pAlloc, UINT size, UINT addrInfo);
LOCAL void pciAutoBusConfig ( PCI_SYSTEM * pSystem, PCI_LOC * pPciLoc,
    PCI_LOC **ppPciList, UINT *nSize);

/* subroutines */

#define BIG_LOOP for(loop=50000;loop>0;loop--);

#ifdef DEBUG_PCILDT_CONFIG
#define SIO_CHAR(x)   \
       * (volatile unsigned long long  *)  (0xb0060170) = (x);BIG_LOOP
#else
#define SIO_CHAR(x)
#endif

/******************************************************************************
*
* pciAutoConfig - Automatically configure all nonexcluded PCI headers.
*
* Top level function in the PCI configuration process:
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
*
* ALGORITHM:
*
* Probe PCI config space and create a list of available PCI functions.
* Call device exclusion function, if registered, to exclude/include device.
* Disable all devices before we initialize any.
* Allocate and assign PCI space to each device.
* Calculate and set interrupt line value.
* Initialize and enable each device.
*
* RETURNS: N/A.
*
* ERRNO
*/

void pciAutoConfig
    (
    PCI_SYSTEM * pSystem    /* PCI system to configure */
    )
    {
    PCI_LOC* pPciList;      /* Pointer to PCI include list  */
    int listSize;           /* Size of PCI include list */

    /* Input parameter sanity checking */

    if (pSystem == NULL)
        {
        return;
        }

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
        while (TRUE)
            {

            /*
             * Probe all PCI busses dynamically creating a function list
             * of all functions found. Excluded devices are skipped over.
             */

            pPciList = pciAutoListCreate (pSystem, &listSize);

            /* Perform roll call function, if we pass, exit the loop */

            if (pSystem->pciRollcallRtn () == TRUE)
                break;
            }
        }

    /*
     * Probe all PCI busses dynamically creating a function list
     * of all functions found. Excluded devices are skipped over.
     */

    SIO_CHAR('v')
        BPRINT("PA02");
    pPciList = pciAutoListCreate (pSystem, &listSize);
    SIO_CHAR('V')

        BPRINT("PA01");
    pciAutoFuncConfigAll (pSystem, pPciList, listSize);
    SIO_CHAR('v')

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

    }

/******************************************************************************
*
* pciAutoListCreate - probe for all functions and make a PCI probe list
*
* This routine creates a dynamic probelist containing all PCI functions
* located in the PCI configuration hierarchy.  In addition, it assigns
* base addresses for the 32-bit prefetchable memory allocation pool,
* the 32-bit non-prefetchable memory allocation pool, the 32-bit I/O
* allocation pool, and the 16-bit I/O allocation pool.   When I/O space
* or memory space is actually assigned (not in this routine), the space
* allocation will begin with the base address in each of these categories
* and proceed to higher numbered addresses.
*
* Note that 20-bit memory space is not currently handled as a special case.
*
* RETURNS: pointer to newly populated PCI device list
*
* ERRNO
*/

LOCAL PCI_LOC * pciAutoListCreate
    (
    PCI_SYSTEM * pSystem,
    int *pListSize
    )
    {
    PCI_LOC  pciLoc;        /* PCI bus/device/function structure */
    PCI_LOC *pPciList;
    PCI_LOC *pRetPciList;

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

    pciLoc.bus = (UINT8)0;
    pciLoc.device = (UINT8)0;
    pciLoc.function = (UINT8)0;

    /*
     * Note that the host bridge is assumed to support 32-bit I/O addressing
     * (PCI_AUTO_ATTR_BUS_4GB_IO) and prefetchable memory
     * (PCI_AUTO_ATTR_BUS_PREFETCH)
     */

    SIO_CHAR('w')
    pciMaxBus = pciAutoDevProbe (pSystem, pciLoc.bus, (UCHAR)0,
                                 (UCHAR)( PCI_AUTO_ATTR_BUS_4GB_IO |
                                          PCI_AUTO_ATTR_BUS_PREFETCH),
                                 &pPciList, pListSize);
    SIO_CHAR('W')

    pSystem->maxBus = pciMaxBus;

    return(pRetPciList);
    }

/******************************************************************************
*
* pciAutoDevReset - Quiesce a PCI device and reset all writeable status bits
*
* This routine turns 'off' a PCI device by disabling the Memory decoders, I/O
* decoders, and Bus Master capability. The routine also resets all writeable
* status bits in the status word that follows the command word sequentially
* in PCI config space by performing a longword access.
*
* RETURNS: OK, always
*
* ERRNO
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
* pciAutoBusNumberSet - Set the primary, secondary, and subordinate bus number
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
*/

STATUS pciAutoBusNumberSet
    (
    PCI_LOC * pPciLoc,
    UINT primary,
    UINT secondary,
    UINT subordinate
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
* pciAutoBusProbe - Configure a bridge and probe all devices behind it
*
* This routine assigns an initial range of subordinate busses to a
* PCI bridge, searches for functions under this bridge, and then
* updates the range assignment to the correct value.  It calls
* pciAutoDevProbe() which in turn calls this function in a recursive
* manner.  In addition to actually programming the PCI-PCI bridge
* headers with correct bus numbers, the 'pciLoc' list of functions
* is extended as each new PCI function is found.
*
* RETURNS: subordinate bus number.
*
* ERRNO
*/

LOCAL UINT pciAutoBusProbe
    (
    PCI_SYSTEM * pSystem,   /* PCI system information   */
    UINT priBus,            /* Primary PCI bus      */
    UINT secBus,            /* Secondary PCI bus        */
    PCI_LOC*  pPciLoc,      /* PCI address of this bridge   */
    PCI_LOC** ppPciList,    /* Pointer to next PCI location */
                            /* entry pointer        */
    int * pListSize         /* number of PCI_LOC entries    */
    )
    {
    UINT subBus = 0xff; /* Highest subordinate PCI bus  */
    UCHAR offset = 0;   /* Interrupt routing offset for this bus*/
    UINT dev_vend;      /* Device/Vendor identifier     */

SIO_CHAR('q')
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

    /* Do pre-probe initialization of the bridge or secondary bus */

    if ((pSystem->bridgePreProbeInit) != NULL )
        {
        pciConfigInLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                         PCI_CFG_VENDOR_ID, &dev_vend);

        (pSystem->bridgePreProbeInit) (pSystem, pPciLoc, dev_vend);
        }

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

    /* Do post-probe initialization of the bridge or secondary bus */

    if ((pSystem->bridgePostProbeInit) != NULL )
        {
        pciConfigInLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                         PCI_CFG_VENDOR_ID, &dev_vend);

        (pSystem->bridgePostProbeInit) (pSystem, pPciLoc, dev_vend);
        }

    /* Return the highest subordinate bus */

SIO_CHAR('Q')
    return subBus;

    }

/* These macros are similar to ones in pciLocalBus.h */
#define PCI_CLASS(x)     (((x)>>8) & 0xff)
#define PCI_SUBCLASS(x)  ((x) & 0xff)
/******************************************************************************
*
* pciAutoDevProbe - Probe all devices on a single PCI bus.
*
* This routine probes a single PCI bus and adds each detected PCI function
* to the function list.  In case a PCI-PCI bridge is found, pciAutoBusProbe()
* is called to probe that bus. pciAutoBusProbe() in turn calls this function
* in a recursive manner until all busses have been probed.
*
* RETURNS: Highest subordinate bus number found during probing process.
*
* ERRNO
*/

LOCAL UINT pciAutoDevProbe
    (
    PCI_SYSTEM * pSystem,   /* PCI system info          */
    UINT bus,               /* current bus number to probe      */
    UCHAR offset,           /* bridge contrib to int vector xlation */
    UCHAR inheritAttrib,    /* attributes inherited from bridge     */
    PCI_LOC **ppPciList,    /* Pointer to next PCI location entry   */
    int * pListSize         /* Number of PCI_LOC's currently in list*/
    )
    {
    PCI_LOC pciLoc;     /* PCI bus/device/function structure    */
    UINT16 pciClass;        /* PCI class/subclass contents      */
    UINT dev_vend;      /* Device/Vendor identifier     */
    int device;         /* Device location          */
    int function;       /* Function location            */
    int subBus;         /* Highest subordinate PCI bus      */
    UCHAR btemp;        /* Temporary holding area       */
    UINT temp;

    /* Initialize variables */

    bzero ((char *)&pciLoc, sizeof (PCI_LOC));
    pciLoc.bus = bus;

SIO_CHAR(13)    /* carriage return */
SIO_CHAR(10)    /* line feed */
SIO_CHAR('!')
SIO_CHAR('0'+bus)

    subBus = bus;

    /* if attributes indicate a host bus, then set equal to pciLoc.attrib */

    /* Locate each active function on the current bus */

    for (device = 0; device < PCI_MAX_DEV; device++)
        {
SIO_CHAR(13)
SIO_CHAR(10)
SIO_CHAR(' ')
SIO_CHAR('u')
SIO_CHAR('0'+device)
        if (pciDeviceMode && bus == 0 && device >= 2)
            break;

        pciLoc.device = device;

        /* Check each function until an unused one is detected */

        for (function = 0; function < PCI_MAX_FUNC; function++)
            {
SIO_CHAR('r')
SIO_CHAR('0'+function)
            pciLoc.function = function;

            /* Check for a valid device/vendor number */

SIO_CHAR('(')
            pciConfigInLong (pciLoc.bus, pciLoc.device, pciLoc.function,
                             PCI_CFG_VENDOR_ID, &dev_vend);
SIO_CHAR(')')

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

SIO_CHAR('(')
            pciConfigInWord (pciLoc.bus, pciLoc.device, pciLoc.function,
                             PCI_CFG_SUBCLASS, &pciClass);
SIO_CHAR(')')

            /* Set Bridge device attributes for this device */

            if (PCI_CLASS(pciClass) == PCI_CLASS_BRIDGE_CTLR)
                {
                UCHAR pcitype;      /* PCI header type      */
                UINT  includeSecondary;

SIO_CHAR('s')
                pciConfigInByte (pciLoc.bus, pciLoc.device, pciLoc.function,
                                 PCI_CFG_HEADER_TYPE, &pcitype);
                /* If this function has a Type 1 header with primary and
                   secondary bus fields,  it makes sense to include the
                   function as a bridge between PCI-like buses.  We can
                   trust the decision of the callback, if provided.
                   Otherwise, decide based on the subclass.
                 */
                if (pcitype == 1 && pSystem->includeSecondaryRtn != NULL)
                    {
                    if ((pSystem->includeSecondaryRtn) (pSystem, &pciLoc,
                                                        dev_vend) == OK)
                        includeSecondary = 1;
                    else
                        includeSecondary = 0;
                    }
                else if (PCI_SUBCLASS(pciClass) == PCI_SUBCLASS_P2P_BRIDGE)
                    includeSecondary = 1;
                else
                    includeSecondary = 0;

                if (includeSecondary)
                    pciLoc.attribute |= PCI_AUTO_ATTR_BUS_PCI;
                else
                    {
                    switch (PCI_SUBCLASS(pciClass))
                        {
                        case PCI_SUBCLASS_HOST_PCI_BRIDGE:
                            pciLoc.attribute |= ( PCI_AUTO_ATTR_DEV_EXCLUDE |
                                                  PCI_AUTO_ATTR_BUS_HOST );
                            break;

                        case PCI_SUBCLASS_ISA_BRIDGE:
                            pciLoc.attribute |= PCI_AUTO_ATTR_BUS_ISA;
                            break;

                        default:
                            break;
                        }
                    }
                }

            else if (pciClass == (PCI_CLASS_DISPLAY_CTLR << 8))
                pciLoc.attribute |= PCI_AUTO_ATTR_DEV_DISPLAY;

            else
                {
SIO_CHAR('p')
                /* Mask off all but bus attribute bits to inherit */

                inheritAttrib &=   ( PCI_AUTO_ATTR_BUS_4GB_IO |
                                     PCI_AUTO_ATTR_BUS_PREFETCH );

                /* devices inherit bus attributes from their bridge */

                pciLoc.attribute |= inheritAttrib;
                }

            /* If the device is a PCI-to-PCI bridge, get new attributes */

            if (pciLoc.attribute & PCI_AUTO_ATTR_BUS_PCI)
                {
                /*
                 * Check for 32 bit I/O addressability,
                 * but only if the parent bridge supports it
                 */
SIO_CHAR('{')

                if (inheritAttrib & PCI_AUTO_ATTR_BUS_4GB_IO)
                    {
                    pciConfigInByte (pciLoc.bus,
                                     pciLoc.device, pciLoc.function,
                                     PCI_CFG_IO_BASE, &btemp);

                    if ((btemp & 0x0F) == 0x01)
                        {
                        pciConfigInByte (pciLoc.bus, 
                                         pciLoc.device, pciLoc.function,
                                         PCI_CFG_IO_LIMIT, &btemp);
                        if ((btemp & 0x0F) == 0x01)
                            {
                            pciLoc.attribute |= PCI_AUTO_ATTR_BUS_4GB_IO;
                            PCI_AUTO_DEBUG_MSG("pciAutoDevProbe: 4GB I/O \n",
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
SIO_CHAR('}')
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
SIO_CHAR('t')
                pciConfigInByte (pciLoc.bus, pciLoc.device, pciLoc.function,
                                 PCI_CFG_HEADER_TYPE, &btemp);
                if ((btemp & PCI_HEADER_MULTI_FUNC) == 0)
                    {
                    break; /* No more functions - proceed to next PCI device */
                    }
                }
            }
        }

SIO_CHAR(13)
SIO_CHAR(10)
SIO_CHAR('@')
SIO_CHAR('0'+bus)
    return(subBus);
    }

/******************************************************************************
*
* pciAutoFuncConfigAll - Configure all PCI functions contained in list
*
* This routine initializes all PCI functions within the specified
* list.  This may be anything from a full list to a single entry.
*
* RETURNS: N/A.
*
* ERRNO
*/

LOCAL void pciAutoFuncConfigAll
    (
    PCI_SYSTEM * pSystem,   /* PCI system info */
    PCI_LOC *pPciList,      /* input: Pointer to first function   */
    UINT nSize              /* input: Number of functions to init */
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
        pciAutoFuncEnable (pSystem, pPciFunc);
        pPciFunc++;
        }

    }

/******************************************************************************
*
* pciAutoFuncDisable - Disable a specific PCI function.
*
* This routine clears the I/O, mem, master, & ROM space enable bits
* for a single PCI function.
*
* The PCI spec says that devices should normally clear these by default after
* reset but in actual practice, some PCI devices do not fully comply. This
* routine ensures that the devices have all been disabled before configuration
* is started.
*
* RETURNS: N/A.
*
* ERRNO
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
* pciAutoFuncEnable - Perform final configuration and enable a function
*
* Depending upon whether the device is included, this routine initializes
* a single PCI function as follows:
*
* Initialize the cache line size register
* Initialize the PCI-PCI bridge latency timers
* Enable the master PCI bit for non-display devices
* Set the interrupt line value with the value from the BSP.
*
* RETURNS: N/A.
*
* ERRNO
*/

void pciAutoFuncEnable
    (
    PCI_SYSTEM * pSystem, /* PCI system info */
    PCI_LOC * pFunc       /* input: Pointer to PCI function structure */
    )
    {
    UINT16 pciClass;      /* PCI class/subclass contents */
    UCHAR intLine = 0xff; /* Interrupt "Line" value           */

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

    pciConfigOutByte (pFunc->bus, pFunc->device, pFunc->function,
                      PCI_CFG_LATENCY_TIMER, pSystem->maxLatency);

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

    return;
    }

/******************************************************************************
*
* pciAutoIntAssign - Compute the interrupt number for a given PCI device.
*
* This routine computes the interrupt number for a PCI device identified
* by the parameter <pFunc>.
*
* The algorithm used is the following: if the device is on the PCI bus 0,
* then the bsp interrupt assign routine (if any) is called to get the 
* actual IRQ number used. In addition, if the device is a Pci-To-Pci 
* Bridge, then this routine populates a routing table that will be used 
* later for all of the PCI devices on its every subordinate buses.
* Conversely, if the device sits on any PCI bus other than bus 0, this
* routines only looks at that table. The index used depends not
* only on the device's location on the bus, but also on the routing offset
* of the most immediate Pci-To-Pci Bridge. This offset, in turn, is based
* on its location in the PCI hierarchy and was computed earlier by the 
* PCI configuration process. However, the user may skip this automatic 
* interrupt assignment process by simply setting the variable autoIntRouting
* in the relevant PCI_SYSTEM structure to FALSE. In this case the BSP 
* interrupt assign routine will be called to get the IRQ number for the device.
*
* RETURNS: the interrupt number associated with the device.
*
* ERRNO
*/
 
LOCAL UCHAR pciAutoIntAssign
    (
    PCI_SYSTEM * pSystem, /* PCI system info */
    PCI_LOC * pFunc       /* input: Pointer to PCI function structure */
    )
    {
    UCHAR retVal = 0xFF;
    UCHAR        intPin;        /* Interrupt "Pin" value            */
 
    pciConfigInByte (pFunc->bus, pFunc->device, pFunc->function,
                     PCI_CFG_DEV_INT_PIN, &intPin);

    /* 
     * if the BSP provides interrupt routing for all PCI devices, 
     * then there's no need of any computation whatsoever 
     */

    if ((!(pSystem->autoIntRouting)) && (intPin != 0))
        {
        if ((pSystem->intAssignRtn) != NULL )
            {
            retVal = (pSystem->intAssignRtn) (pSystem, pFunc, intPin);

            return(retVal);
            }

        }

    /* default interrupt routing: let's find out the IRQ for this device */

    switch (pFunc->bus)
        {
        case 0:

            if (((pSystem->intAssignRtn) != NULL) && (intPin != 0))
                {
                retVal = (pSystem->intAssignRtn) (pSystem, pFunc, intPin);
                }

            /* 
             * if this is a P2P Bridge, then populate its interrupt
             * routing table. This will be used later for all the devices
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
                                                      (pSystem, pFunc, (i+1));
                        }
                    }

                }

            break;

        default:

            retVal = pciAutoIntRoutingTable [(((pFunc->device) + (intPin - 1) 
                                               + (pFunc->offset)) % 4)];
            break;
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
* RETURNS: TRUE if a device was found, else FALSE
*
* ERRNO
*/

STATUS pciAutoGetNextClass
    (
    PCI_SYSTEM *pPciSystem,
    PCI_LOC *pPciFunc,  /* output: Contains the BDF of the device found */
    UINT *index,        /* Zero-based device instance number */
    UINT pciClass,      /* class code field from the PCI header */
    UINT mask           /* mask is ANDed with the class field */
    )
    {
    UINT i;
    UINT idx = *index;
    UINT classCode;
    UINT nSize;
    PCI_LOC *pciList;
  
    nSize = (UINT)lastPciListSize;
    pciList = pLastPciList;

    PCI_AUTO_DEBUG_MSG("\npciAutoGetNextClass: index[%d] listSiz[%d]\n",
    *index, nSize, 0, 0, 0, 0);
    PCI_AUTO_DEBUG_MSG("                     pciClass[0x%08x], mask[0x%08x]\n",
    pciClass, mask, 0, 0, 0, 0);

    if ((nSize <= idx) || (pciList == NULL))
        return (FALSE);             /* No more devices */
 
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
            return (TRUE);
            }

        }
    return (FALSE);
    }

/******************************************************************************
*
* pciAutoDevConfig - Allocate memory and I/O space to PCI function.
*
* This routine allocates memory and I/O space to functions on an
* individual PCI bus.
*
* LIMITATIONS: 
*
* Do not sort the include function list before this routine is
* called.  This routine requires each function in the list to be in the
* same order as the probe occurred.
*
* RETURNS: N/A
*
* ERRNO
*/

LOCAL void pciAutoDevConfig
    (
    PCI_SYSTEM * pSystem,  /* PCI system info */
    UINT bus,              /* Current bus number      */
    PCI_LOC **ppPciList,   /* Pointer to function list */
    UINT *nSize            /* Number of remaining funcs */
    )
    {
    PCI_LOC *pPciFunc;  /* Pointer to PCI function     */
    UINT nextBus;       /* Bus where function is located   */

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

        if (pPciFunc->attribute & PCI_AUTO_ATTR_BUS_PCI)
            {
            /* PCI-PCI bridge functions increase recursion depth */

            /* Configure the secondary bus */
            pciAutoBusConfig (pSystem, pPciFunc, ppPciList, nSize);

            }
        }
    }

/******************************************************************************
*
* pciAutoFuncConfig - Assign memory and/or I/O space to single function.
*
* This routine allocates and assigns memory and/or I/O space to a
* single PCI function.   Allocations are made for each implemented
* base address register (BAR) in the PCI configuration header.
*
* RETURNS: N/A
*
* ERRNO
*/

LOCAL void pciAutoFuncConfig
    (
    PCI_SYSTEM * pSystem,
    PCI_LOC * pPciFunc  /* input: "Include list" pointer to function */
    )
    {
    UINT baMax;         /* Total number of base addresses    */
    UINT baI;           /* Base address register index       */
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
        if ( ((pSystem->includeRtn) (pSystem, pPciFunc, dev_vend)) == ERROR )
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
                baI += pciAutoRegConfig (pSystem, pPciFunc, baseAddr, sizeMask,
                                         addrInfo);
                break;
                }
            }
        }

    }

/******************************************************************************
*
* pciAutoRegConfig - Assign PCI space to a single PCI base address register.
*
* This routine allocates and assigns PCI space (either memory or I/O)
* to a single PCI base address register.
*
* RETURNS: 1 if BAR supports mapping anywhere in 64-bit address space, 
*         0 otherwise.
*
* ERRNO
*/

UINT pciAutoRegConfig
    (
    PCI_SYSTEM * pSystem,   /* Pointer to PCI System structure */
    PCI_LOC *pPciFunc,      /* Pointer to function in device list */
    UINT baseAddr,          /* Offset of base PCI address */
    UINT nSize,             /* Size and alignment requirements */
    UINT addrInfo           /* PCI address type information */
    )
    {
    UINT addr;              /* Working address variable */
    UINT spaceEnable = 0;   /* PCI space enable bit */
    UINT baseaddr_mask;     /* Mask for base address register */
    UINT register64Bit;     /* 64 bit register flag */

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
    return(register64Bit);
    }

/******************************************************************************
*
* pciAutoIoAlloc - Select appropriate I/O space for device.
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
*/

LOCAL UINT pciAutoIoAlloc
    (
    PCI_SYSTEM * pPciSys,   /* PCI system structure   */
    PCI_LOC *pPciFunc,      /* input: Pointer to PCI function element     */
    UINT *pAlloc,           /* output: Pointer to PCI space alloc pointer */
    UINT nSize              /* requested size (power of 2) */
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
        logMsg("Warning: PCI I/O allocation failed\n", 0, 0, 0, 0, 0, 0);
        *pAlloc = NO_ALLOCATION;
        return 0;
        }

    PCI_AUTO_DEBUG_MSG("pciAutoIoAlloc: Pre/Post alloc: \n", 0, 0, 0, 0, 0, 0);
    PCI_AUTO_DEBUG_MSG("  Pre: pBase[0x%08x], pAvail[0x%08x]\n",
                       (int)(*pBase), (int)(*pAvail), 0, 0, 0, 0);

    *pAlloc  = alignedBase;
    sizeAdj = (alignedBase - *pBase) + nSize;
    *pBase  += sizeAdj;
    *pAvail -= sizeAdj;

    PCI_AUTO_DEBUG_MSG("  Post: pBase[0x%08x], pAvail[0x%08x]\n",
                       (int)(*pBase), (int)(*pAvail), 0, 0, 0, 0);

    return 0; /* can't have 64 bit i/o addresses */
    }

/******************************************************************************
*
* pciAutoMemAlloc - Select appropriate memory space for device.
*
* This routine determines which PCI memory space pool to use for assignment
* to a particular function.  Note that functions located on subordinate
* busses must be  assigned to the 32 bit PCI memory space due to 32 bit
* requirements of functions using more than 1MB memory space.
*
* LIMITATIONS:
*
* Does not support 64-bit Memory space
*
* RETURNS: 1 if 64-bit addressable memory space, zero otherwise.
*
* ERRNO
*/

LOCAL UINT pciAutoMemAlloc
    (
    PCI_SYSTEM * pPciSys,   /* PCI system structure */
    PCI_LOC * pPciFunc,     /* Pointer to PCI function element     */
    UINT * pAlloc,          /* Pointer to PCI space alloc pointer */
    UINT size,              /* space requested (power of 2)  */
    UINT addrInfo           /* PCI address type information       */
    )
    {
    UINT register64Bit = 0; /* 64 bit register flag */
    UINT * pBase;
    UINT * pAvail;
    UINT32 alignedBase;
    UINT32 sizeAdj;
    STATUS retStat = ERROR;

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

            if (*pAvail > 0)
                {
                retStat = pciAutoAddrAlign (*pBase,
                                            (*pBase + *pAvail),
                                            size,
                                            &alignedBase);
                }

            if (retStat == ERROR)
                {
                logMsg("Warning: PCI PF Mem alloc failed\n", 0, 0, 0, 0, 0, 0);
                *pAlloc = NO_ALLOCATION;
                return 0;
                }
            }
        }
    else
        {

        /* Use 32-bit Non-Prefetch Memory */

        pBase = &(pPciSys->pciMemIo32);
        pAvail = &(pPciSys->pciMemIo32Size);

        if (*pAvail > 0)
            {
            retStat = pciAutoAddrAlign (*pBase,
                                        (*pBase + *pAvail),
                                        size,
                                        &alignedBase);
            }

        if (retStat == ERROR)
            {
            logMsg("Warning: PCI Memory allocation failed\n", 0, 0, 0, 0, 0, 0);
            *pAlloc = NO_ALLOCATION;
            return 0;
            }
        }

    PCI_AUTO_DEBUG_MSG("pciAutoMemAlloc: \n", 0, 0, 0, 0, 0, 0);
    PCI_AUTO_DEBUG_MSG("  Pre: pBase[0x%08x], pAvail[0x%08x]\n",
                       (int)(*pBase), (int)(*pAvail), 0, 0, 0, 0);

    *pAlloc  = alignedBase;
    sizeAdj = (alignedBase - *pBase) + size;
    *pBase  += sizeAdj;
    *pAvail -= sizeAdj;

    PCI_AUTO_DEBUG_MSG("  Post: pBase[0x%08x], pAvail[0x%08x]\n",
                       (int)(*pBase), (int)(*pAvail), 0, 0, 0, 0);

    return register64Bit;
    }

/******************************************************************************
*
* pciAutoBusConfig - Set mem and I/O registers for a single PCI-PCI bridge.
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
*/

LOCAL void pciAutoBusConfig
    (
    PCI_SYSTEM * pSystem,   /* PCI system info */
    PCI_LOC * pPciLoc,      /* PCI address of this bridge */
    PCI_LOC **ppPciList,    /* Pointer to function list pointer */
    UINT *nSize             /* Number of remaining functions */
    )
    {
    UCHAR bus;          /* Bus number for current bus */
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

        (pSystem->bridgePreConfigInit) (pSystem, pPciLoc, dev_vend);
        }

#ifdef PASS1
    pciConfigInByte (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                     PCI_CFG_SECONDARY_BUS, &bus);
#endif

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

#ifdef PASS1
        if (bus == SB1250_LDT_BUS)
            {
            /* Pad away from the mismapped cache line. */
            (pSystem->pciIo16) += 0x1000;
            (pSystem->pciIo16Size) -= 0x1000;
            }
#endif
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

#ifdef PASS1
        if (bus == SB1250_LDT_BUS)
            {
            /* Pad away from the mismapped cache line. */
            (pSystem->pciMemIo32) += 0x100000;
            (pSystem->pciMemIo32Size) -= 0x100000;
            }
#endif
        }

    /* Configure devices on the bus implemented by this bridge */

#ifdef PASS1
    pciConfigInByte (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                     PCI_CFG_SECONDARY_BUS, &bus);
#endif

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

        (pSystem->pciIo16Size) -= (alignedBase - (pSystem->pciIo16));
        (pSystem->pciIo16) = alignedBase;

        pciConfigModifyLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                             PCI_CFG_IO_BASE, 0x0000f000, 
#ifdef PASS1
                             (pSystem->pciIo16 - (bus==SB1250_LDT_BUS ? 0 : 1))
#else
                             (pSystem->pciIo16 - 1)
#endif
                            );

        pciConfigModifyLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                             PCI_CFG_IO_BASE_U, 0xffff0000, 
#ifdef PASS1
                             (pSystem->pciIo16 - (bus==SB1250_LDT_BUS ? 0 : 1))
#else
                             (pSystem->pciIo16 - 1)
#endif
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

#ifdef PASS1
        if (bus == SB1250_LDT_BUS)
            {
            /* Pad away from the mismapped cache line. */
            (pSystem->pciIo16) += 0x1000;
            (pSystem->pciIo16Size) -= 0x1000;
            }
#endif
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

        (pSystem->pciMemIo32Size) -= (alignedBase - (pSystem->pciMemIo32));
        (pSystem->pciMemIo32) = alignedBase;

        pciConfigModifyLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                             PCI_CFG_MEM_BASE, 0xfff00000,
#ifdef PASS1
                             (pSystem->pciMemIo32
                              - (bus == SB1250_LDT_BUS ? 0 : 1))
#else
                             (pSystem->pciMemIo32 - 1)
#endif
                            );

        pciConfigInLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                         PCI_CFG_MEM_BASE,
                         &debugTmp2);

        debugTmp = (debugTmp2 & 0xfff00000);
        debugTmp |= 0x000FFFFF;

        PCI_AUTO_DEBUG_MSG("pciAutoBusConfig: MemIo Limit [0x%08x]\n",

                           debugTmp, 0, 0, 0, 0, 0);

#ifdef PASS1
        if (bus == SB1250_LDT_BUS)
            {
            /* Pad away from the mismapped cache line. */
            (pSystem->pciMemIo32) += 0x100000;
            (pSystem->pciMemIo32Size) -= 0x100000;
            }
#endif
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

        (pSystem->bridgePostConfigInit) (pSystem, pPciLoc, dev_vend);
        }

    /* Initialize primary and secondary PCI-PCI bridge latency timers */

    pciConfigOutByte (pPciLoc->bus, pPciLoc->device, pPciLoc->function, 
                      PCI_CFG_SEC_LATENCY, pSystem->maxLatency);

    /* Clear status bits turn on downstream and upstream (master) mem,IO */

    pciConfigOutLong (pPciLoc->bus, pPciLoc->device, pPciLoc->function,
                      PCI_CFG_COMMAND,
                      (UINT32)(0xffff0000 | PCI_CMD_IO_ENABLE |
                               PCI_CMD_MEM_ENABLE | PCI_CMD_MASTER_ENABLE)
                     );
    }

/*******************************************************************************
*
* pciAutoAddrAlign - Align a PCI address and check boundary conditions
*
* This routine aligns and checks boundary condition is of the given PCI address.
* 
* RETURNS: OK, or ERROR
*
* ERRNO
*/

STATUS pciAutoAddrAlign
    (
    UINT32 base,
    UINT32 limit,
    UINT32 reqSize,
    UINT32 *pAlignedBase
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
        return ERROR;
        }

    *pAlignedBase = base + alignAdjust;

    /* see if the aligned base+size exceeds the resource boundary */

    if ( ((base + alignAdjust + reqSize) < base) || 
         ((base + alignAdjust + reqSize) > limit) )
        {
        PCI_AUTO_DEBUG_MSG ("pciAutoAddrAlign: base + adjustment + req [%08x]"
                " exceeds limit [%08x]\n",
                (base + alignAdjust + reqSize),limit,0,0,0,0);
        return ERROR;
        }

    PCI_AUTO_DEBUG_MSG ("pciAutoAddrAlign: new aligned base [%08x]\n",
            (base + alignAdjust),0,0,0,0,0);

    return OK;
    }

