/* hwconf.c - Hardware configuration support module */

/*
 * Copyright (c) 2010,2013 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */
 
/*
modification history
--------------------
01a,05nov13,dnb  updated for katana2
*/

#include <vxWorks.h>
#include <vxBusLib.h>
#include <hwif/vxbus/vxBus.h>
#include <hwif/vxbus/vxbIntrCtlr.h>
#include <hwif/util/vxbParamSys.h>
#include <hwif/vxbus/hwConf.h>
#include "config.h"

struct intrCtlrInputs gicInputs[] = {
    /* pin,                 driver,             unit,   index */
    { INT_PIN_UART0,        "ns16550",           0,       0 },
    { INT_PIN_PVT_TIM,      "armMpCoreTimer",    0,       0 },
    { IPROC_GMAC0_INT,      "et",                0,       0 },
    { IPROC_GMAC1_INT,      "et",                1,       0 },
    { IPROC_GMAC2_INT,      "et",                2,       0 },
    { IPROC_GMAC3_INT,      "et",                3,       0 },
    { IPROC_PCIE_INT5,      "iProcPCIe",         0,       0 },
    { IPROC_CCB_TIM0_0_INT, "iProcCcbTimer",     0,       0 },
    { IPROC_CCB_TIM0_1_INT, "iProcCcbTimer",     1,       0 },
    { IPROC_CCB_TIM1_0_INT, "iProcCcbTimer",     2,       0 },
    { IPROC_CCB_TIM1_1_INT, "iProcCcbTimer",     3,       0 },
    };

LOCAL const struct hcfResource armGIC0Resources[] = {
    { VXB_REG_BASE,        HCF_RES_INT,  {(void *)IPROC_PERIPH_BASE } },
    { "input",             HCF_RES_ADDR, {(void *)&gicInputs[0] } },
    { "intMode",           HCF_RES_INT,  {(void *)INT_MODE } },
    { "maxIntLvl",         HCF_RES_INT,  {(void *)SYS_INT_LEVELS_MAX } },
    { "inputTableSize",    HCF_RES_INT,  {(void *)NELEMENTS(gicInputs) } },
};
#define armGIC0Num NELEMENTS(armGIC0Resources)

extern UINT32 sysUartFreqGet(void);

LOCAL struct hcfResource ns1655x0Resources[] = {
    { VXB_REG_BASE,  HCF_RES_INT,  {(void *)IPROC_CCA_UART0_REG_BASE} },
    { "irq",         HCF_RES_INT,  {(void *)INT_PIN_UART0} },
    { "clkFreq",     HCF_RES_ADDR, {(FUNCPTR)sysUartFreqGet} },

};

#define ns1655x0Num NELEMENTS(ns1655x0Resources)

LOCAL struct hcfResource et0Resources[] = {
    { VXB_REG_BASE,  HCF_RES_INT,  {(void *)IPROC_GMAC0_REGBASE} },
    { "irq",         HCF_RES_INT,  {(void *)IPROC_GMAC0_INT} },
    { "phyAddr",     HCF_RES_INT,  {(void *)IPROC_GMAC0_PHY } },
};

#define et0Num NELEMENTS(et0Resources)

struct hcfResource armMpCoreTimerDevResources[] = {
    { "regBase",     HCF_RES_INT,  { (void *)IPROC_PERIPH_PVT_TIM_REG_BASE} },
    { "clkFreq",     HCF_RES_INT,  { (void *)500000000 /* 500 MHz */} },
    { "minClkRate",  HCF_RES_INT,  { (void *)SYS_CLK_RATE_MIN } },
    { "maxClkRate",  HCF_RES_INT,  { (void *)SYS_CLK_RATE_MAX } },
};
#define armMpCoreTimerDevNum NELEMENTS(armMpCoreTimerDevResources)

#ifdef DRV_TIMER_IPROC_CCB
struct hcfResource iProcCcbTimer0Resources[] = {
    { "regBase",     HCF_RES_INT,  { (void *)IPROC_CCB_TIM0_REG_BASE} },
    { "clkFreq",     HCF_RES_INT,  { (void *)123750000 /* 123.75 MHz */} },
    { "clkRateMin",  HCF_RES_INT,  { (void *)1 } },
    { "clkRateMax",  HCF_RES_INT,  { (void *)1000 } },
};
#define iProcCcbTimer0Num NELEMENTS(iProcCcbTimer0Resources)

struct hcfResource iProcCcbTimer1Resources[] = {
    { "regBase",     HCF_RES_INT,  { (void *)IPROC_CCB_TIM0_REG_BASE} },
    { "clkFreq",     HCF_RES_INT,  { (void *)123750000 /* 123.75 MHz */} },
    { "clkRateMin",  HCF_RES_INT,  { (void *)1 } },
    { "clkRateMax",  HCF_RES_INT,  { (void *)1000 } },
};
#define iProcCcbTimer1Num NELEMENTS(iProcCcbTimer1Resources)

struct hcfResource iProcCcbTimer2Resources[] = {
    { "regBase",     HCF_RES_INT,  { (void *)IPROC_CCB_TIM1_REG_BASE} },
    { "clkFreq",     HCF_RES_INT,  { (void *)123750000 /* 123.75 MHz */} },
    { "clkRateMin",  HCF_RES_INT,  { (void *)1 } },
    { "clkRateMax",  HCF_RES_INT,  { (void *)1000 } },
};
#define iProcCcbTimer2Num NELEMENTS(iProcCcbTimer2Resources)

struct hcfResource iProcCcbTimer3Resources[] = {
    { "regBase",     HCF_RES_INT,  { (void *)IPROC_CCB_TIM1_REG_BASE} },
    { "clkFreq",     HCF_RES_INT,  { (void *)123750000 /* 123.75MHz */} },
    { "clkRateMin",  HCF_RES_INT,  { (void *)1 } },
    { "clkRateMax",  HCF_RES_INT,  { (void *)1000 } },
};
#define iProcCcbTimer3Num NELEMENTS(iProcCcbTimer3Resources)
#endif /* DRV_TIMER_IPROC_CCB */

#ifdef DRV_IPROC_PCIE
struct hcfResource iProcPCIeResources[] = {
    { "regBase",     	    HCF_RES_INT,    { (void *)IPROC_PCIE_AXIB0_REG_BASE} },
    { "memIo32Addr",        HCF_RES_ADDR,   { (void *)PCIEX0_MEMIO_ADRS } },
    { "memIo32Size",        HCF_RES_INT,    { (void *)PCIEX0_MEMIO_SIZE } },
};
#define iProcPCIeResourceNum NELEMENTS(iProcPCIeResources)
#endif /* DRV_IPROC_PCIE */

const struct hcfDevice hcfDeviceList[] = {
    { "armGicDev",       0, VXB_BUSID_PLB, 0, armGIC0Num, armGIC0Resources},
    { "ns16550"  ,       0, VXB_BUSID_PLB, 0, ns1655x0Num, ns1655x0Resources },
    { "armMpCoreTimer",  0, VXB_BUSID_PLB, 0, armMpCoreTimerDevNum, armMpCoreTimerDevResources },
#ifdef DRV_TIMER_IPROC_CCB
    { "iProcCcbTimer",   0, VXB_BUSID_PLB, 0, iProcCcbTimer0Num, iProcCcbTimer0Resources },
    { "iProcCcbTimer",   1, VXB_BUSID_PLB, 0, iProcCcbTimer1Num, iProcCcbTimer1Resources },
    { "iProcCcbTimer",   2, VXB_BUSID_PLB, 0, iProcCcbTimer2Num, iProcCcbTimer2Resources },
    { "iProcCcbTimer",   3, VXB_BUSID_PLB, 0, iProcCcbTimer3Num, iProcCcbTimer3Resources },
#endif
#ifdef DRV_VXBEND_IPROC
    { "et",              0, VXB_BUSID_PLB, 0, et0Num,et0Resources },
#endif
#ifdef DRV_IPROC_PCIE
    { "iProcPCIe",       0, VXB_BUSID_PLB, 0, iProcPCIeResourceNum, iProcPCIeResources },
#endif
};

const int hcfDeviceNum = NELEMENTS(hcfDeviceList);

VXB_INST_PARAM_OVERRIDE sysInstParamTable[] =
    {
    { NULL, 0, NULL, VXB_PARAM_END_OF_LIST, {(void *)0} }
    };
