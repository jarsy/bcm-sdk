/* hwconf.c - Hardware configuration support module */

/* $Id: hwconf.c,v 1.6 2011/07/21 16:14:13 yshtil Exp $
 * Copyright (c) 2006-2007 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01d,09apr07,b_m  modify to use m85xxCCSR driver.
01c,09mar06,wap  Allow either mottsec or motetsec driver to be used
                 (SPR #118829)
01b,07feb06,wap  Add VxBus parameter table
01a,16jan06,dtr  written.
*/

#include <vxWorks.h>
#include <vxBusLib.h>
#include <hwif/vxbus/vxBus.h>
#include <hwif/vxbus/vxbPlbLib.h>
#include <hwif/vxbus/hwConf.h>
#include <../src/hwif/h/vxbus/vxbRapidIO.h>
#include <hwif/util/vxbParamSys.h>
#include <config.h>

const struct hcfResource motEtsecHEnd0Resources[] = {
    { "regBase", HCF_RES_INT, { (void *)(CCSBAR + 0x24000) } },
    { "txInt", HCF_RES_INT, { (void *)EPIC_TSEC1TX_INT_VEC } },
    { "txIntLevel", HCF_RES_INT, { (void *)EPIC_TSEC1TX_INT_VEC } },
    { "rxInt", HCF_RES_INT, { (void *)EPIC_TSEC1RX_INT_VEC } },
    { "rxIntLevel", HCF_RES_INT, { (void *)EPIC_TSEC1RX_INT_VEC } },
    { "errInt", HCF_RES_INT, { (void *)EPIC_TSEC1ERR_INT_VEC } },
    { "errIntLevel", HCF_RES_INT, { (void *)EPIC_TSEC1ERR_INT_VEC } },
};
#define etsecHEnd0Num 7

const struct hcfResource motEtsecHEnd1Resources[] = {
    { "regBase", HCF_RES_INT, { (void *)(CCSBAR + 0x25000) } },
    { "txInt", HCF_RES_INT, { (void *)EPIC_TSEC2TX_INT_VEC } },
    { "txIntLevel", HCF_RES_INT, { (void *)EPIC_TSEC2TX_INT_VEC } },
    { "rxInt", HCF_RES_INT, { (void *)EPIC_TSEC2RX_INT_VEC } },
    { "rxIntLevel", HCF_RES_INT, { (void *)EPIC_TSEC2RX_INT_VEC } },
    { "errInt", HCF_RES_INT, { (void *)EPIC_TSEC2ERR_INT_VEC } },
    { "errIntLevel", HCF_RES_INT, { (void *)EPIC_TSEC2ERR_INT_VEC } },
};
#define etsecHEnd1Num 7

const struct hcfResource motEtsecHEnd2Resources[] = {
    { "regBase", HCF_RES_INT, { (void *)(CCSBAR + 0x26000) } },
    { "txInt", HCF_RES_INT, { (void *)EPIC_TSEC3TX_INT_VEC } },
    { "txIntLevel", HCF_RES_INT, { (void *)EPIC_TSEC3TX_INT_VEC } },
    { "rxInt", HCF_RES_INT, { (void *)EPIC_TSEC3RX_INT_VEC } },
    { "rxIntLevel", HCF_RES_INT, { (void *)EPIC_TSEC3RX_INT_VEC } },
    { "errInt", HCF_RES_INT, { (void *)EPIC_TSEC3ERR_INT_VEC } },
    { "errIntLevel", HCF_RES_INT, { (void *)EPIC_TSEC3ERR_INT_VEC } },
};
#define etsecHEnd2Num 7

const struct hcfResource motEtsecHEnd3Resources[] = {
    { "regBase", HCF_RES_INT, { (void *)(CCSBAR + 0x27000) } },
    { "txInt", HCF_RES_INT, { (void *)EPIC_TSEC4TX_INT_VEC } },
    { "txIntLevel", HCF_RES_INT, { (void *)EPIC_TSEC4TX_INT_VEC } },
    { "rxInt", HCF_RES_INT, { (void *)EPIC_TSEC4RX_INT_VEC } },
    { "rxIntLevel", HCF_RES_INT, { (void *)EPIC_TSEC4RX_INT_VEC } },
    { "errInt", HCF_RES_INT, { (void *)EPIC_TSEC4ERR_INT_VEC } },
    { "errIntLevel", HCF_RES_INT, { (void *)EPIC_TSEC4ERR_INT_VEC } },
};
#define etsecHEnd3Num 7

/*
 * On the SBC8548 board, PHYs are physically wired according
 * to the following table:
 *
 * 		Data port pins	Management port pins	MII address
 *		--------------	--------------------	-----------
 * PHY0:        TSEC0           TSEC0                   1 
 * PHY1:        TSEC1           TSEC0                   2
 * PHY2: (note) TSEC2           TSEC0                   3 (note)
 * PHY3: (note) TSEC3           TSEC0                   4 (note)
 *
 * The tricky part is that the PHYs have their management pins
 * connected to TSEC0. We have to make it look like PHY1 is connected
 * to TSEC1, so we provide a remapping resource that will cause
 * PHY1 to be attached to miibus1 instead of miibus0.
 *
 * Note: PHY2 and PHY3 do not exist on the SBC8548, but the corresponding
 * TSEC signals are routed to a high-speed expansion connector. So, as a
 * placeholder for the following structures, addresses 27 and 28 were
 * entered for the phyAddr fields.
 *
 */

const struct hcfResource phy0Resources[] = {
    { "realBus", HCF_RES_INT, { (void *)0 } },
    { "phyAddr", HCF_RES_INT, { (void *)1 } },
    { "virtBus", HCF_RES_INT, { (void *)0 } }
};

#define phy0Num NELEMENTS(phy0Resources)

const struct hcfResource phy1Resources[] = {
    { "realBus", HCF_RES_INT, { (void *)0 } },
    { "phyAddr", HCF_RES_INT, { (void *)2 } },
    { "virtBus", HCF_RES_INT, { (void *)1 } }
};

#define phy1Num NELEMENTS(phy1Resources)

const struct hcfResource phy2Resources[] = {
    { "realBus", HCF_RES_INT, { (void *)0 } },
    { "phyAddr", HCF_RES_INT, { (void *)3 } },
    { "virtBus", HCF_RES_INT, { (void *)2 } }
};

#define phy2Num NELEMENTS(phy2Resources)

const struct hcfResource phy3Resources[] = {
    { "realBus", HCF_RES_INT, { (void *)0 } },
    { "phyAddr", HCF_RES_INT, { (void *)4 } },
    { "virtBus", HCF_RES_INT, { (void *)3 } }
};

#define phy3Num NELEMENTS(phy3Resources)


#ifdef INCLUDE_RAPIDIO_BUS
const struct hcfResource m85xxRio0Resources[] = {
{ "regBase",	HCF_RES_INT, {(void *)RAPIDIO_BASE} },
{ "deviceBase", HCF_RES_INT, {(void *)(RAPIDIO_ADRS + 0x0000000)}},
{ "deviceSize", HCF_RES_INT, {(void *)(RAPIDIO_SIZE - 0x0000000)}},
{ "rioBusAdrs", HCF_RES_INT, {(void *)RAPIDIO_BUS_ADRS }},
{ "rioBusSize", HCF_RES_INT, {(void *)RAPIDIO_BUS_SIZE }}
};
#define m85xxRio0Num    NELEMENTS(m85xxRio0Resources)

const struct hcfResource m85xxCPU0Resources[] = {
	{ "regBase",	HCF_RES_INT, {(void *)RAPIDIO_BASE }},
	{ "targetID",	HCF_RES_INT, {(void *)0 }},
	{ "outboundWindow0",     HCF_RES_INT, {(void *)RIO_CHANNEL_RESERVED }},
	{ "outboundWindow1",     HCF_RES_INT, {(void *)RIO_CHANNEL_MAINT }},
	{ "outboundWindow2",     HCF_RES_INT, {(void *)RIO_CHANNEL_CFG }},
	{ "outboundWindow3",     HCF_RES_INT, {(void *)RIO_CHANNEL_UNRESERVED }},
	{ "outboundWindow4",     HCF_RES_INT, {(void *)RIO_CHANNEL_UNRESERVED }},
	{ "outboundWindow5",     HCF_RES_INT, {(void *)RIO_CHANNEL_UNRESERVED }},
	{ "outboundWindow6",     HCF_RES_INT, {(void *)RIO_CHANNEL_UNRESERVED }},
	{ "outboundWindow7",     HCF_RES_INT, {(void *)RIO_CHANNEL_UNRESERVED }},
	{ "outboundWindow8",     HCF_RES_INT, {(void *)RIO_CHANNEL_UNRESERVED }},
	{ "inboundWindow0",     HCF_RES_INT, {(void *)RIO_CHANNEL_RESERVED }},
	{ "inboundWindow1",     HCF_RES_INT, {(void *)RIO_CHANNEL_SM }},
	{ "inboundWindow2",     HCF_RES_INT, {(void *)RIO_CHANNEL_UNRESERVED }},
	{ "inboundWindow3",     HCF_RES_INT, {(void *)RIO_CHANNEL_UNRESERVED }},
	{ "inboundWindow4",     HCF_RES_INT, {(void *)RIO_CHANNEL_UNRESERVED }}

};
#define m85xxCPU0Num    NELEMENTS(m85xxCPU0Resources)

const struct hcfResource m85xxCPU1Resources[] = {
	{ "regBase",	HCF_RES_INT, {(void *)RAPIDIO_BASE }},
	{ "targetID",	HCF_RES_INT, {(void *) 0x9 }},
	{ "hopCount",	HCF_RES_INT, {(void *) 0x0 }},
	{ "outboundWindow0",     HCF_RES_INT, {(void *)RIO_CHANNEL_RESERVED }},
	{ "outboundWindow1",     HCF_RES_INT, {(void *)RIO_CHANNEL_SM }},
	{ "outboundWindow2",     HCF_RES_INT, {(void *)RIO_CHANNEL_TAS_SET }},
	{ "outboundWindow3",     HCF_RES_INT, {(void *)RIO_CHANNEL_TAS_CLEAR }},
	{ "outboundWindow4",     HCF_RES_INT, {(void *)RIO_CHANNEL_DOORBELL }},
	{ "outboundWindow5",     HCF_RES_INT, {(void *)RIO_CHANNEL_UNRESERVED }},
	{ "outboundWindow6",     HCF_RES_INT, {(void *)RIO_CHANNEL_UNRESERVED }},
	{ "outboundWindow7",     HCF_RES_INT, {(void *)RIO_CHANNEL_UNRESERVED }},
	{ "outboundWindow8",     HCF_RES_INT, {(void *)RIO_CHANNEL_UNRESERVED }},
	{ "inboundWindow0",     HCF_RES_INT, {(void *) RIO_CHANNEL_RESERVED}},
	{ "inboundWindow1",     HCF_RES_INT, {(void *)RIO_CHANNEL_UNRESERVED }},
	{ "inboundWindow2",     HCF_RES_INT, {(void *)RIO_CHANNEL_UNRESERVED }},
	{ "inboundWindow3",     HCF_RES_INT, {(void *)RIO_CHANNEL_UNRESERVED }},
	{ "inboundWindow4",     HCF_RES_INT, {(void *)RIO_CHANNEL_UNRESERVED }}

};
#define m85xxCPU1Num    NELEMENTS(m85xxCPU1Resources)

const struct hcfResource m85xxLawBarResources[] = {
{ "regBase",	HCF_RES_INT, {(void *)CCSBAR} },
{ "LAWBAR0",	HCF_RES_STRING, {"reserved"} },
{ "LAWBAR1",	HCF_RES_STRING, {"reserved"} },
{ "LAWBAR2",	HCF_RES_STRING, {"reserved"} },
{ "LAWBAR3",	HCF_RES_STRING, {"reserved"} },
{ "LAWBAR4",	HCF_RES_STRING, {"reserved"} },
{ "LAWBAR5",	HCF_RES_STRING, {"reserved"} },
{ "LAWBAR6",	HCF_RES_STRING, {"reserved"} },
{ "LAWBAR7",	HCF_RES_STRING, {"m85xxRio"} }

};

#define m85xxLawBarNum    NELEMENTS(m85xxLawBarResources)

const struct hcfResource smEnd0Resources[] = {
{ "regBase",	HCF_RES_INT, {(void *)CCSBAR} },
{ "SM_ANCHOR_OFFSET", HCF_RES_INT, {(void *)SM_ANCHOR_OFFSET}},
{ "SM_MEM_ADRS", HCF_RES_INT,{(void *)SM_MEM_ADRS }},
{ "SM_MEM_SIZE", HCF_RES_INT,{(void *)SM_MEM_SIZE}},
{ "SM_TAS_TYPE", HCF_RES_INT,{(void *)SM_TAS_TYPE} },
{ "SM_INT_TYPE", HCF_RES_INT,{(void *)SM_INT_TYPE} },
{ "SM_INT_ARG1", HCF_RES_INT,{(void *)SM_INT_ARG1}},
{ "SM_INT_ARG2", HCF_RES_INT, {(void *)SM_INT_ARG2}},
{ "SM_INT_ARG3", HCF_RES_INT, {(void *)SM_INT_ARG3}},
{ "SM_MBLK_NUM", HCF_RES_INT, {(void *)600}},
{ "SM_CBLK_NUM", HCF_RES_INT, {(void *)200}}
};

#define smEnd0Num    NELEMENTS(smEnd0Resources)
#endif /* INCLUDE_RAPIDIO_BUS */

const struct hcfDevice hcfDeviceList[] = {

#ifdef INCLUDE_MOT_TSEC_HEND
    { "mottsecHEnd", 0, VXB_BUSID_PLB, 0, etsecHEnd0Num, motEtsecHEnd0Resources },
    { "mottsecHEnd", 1, VXB_BUSID_PLB, 0, etsecHEnd1Num, motEtsecHEnd1Resources },
    { "mottsecHEnd", 2, VXB_BUSID_PLB, 0, etsecHEnd2Num, motEtsecHEnd2Resources },
    { "mottsecHEnd", 3, VXB_BUSID_PLB, 0, etsecHEnd3Num, motEtsecHEnd3Resources },
#endif
#ifdef INCLUDE_MOT_ETSEC_HEND
    { "motetsecHEnd", 0, VXB_BUSID_PLB, 0, etsecHEnd0Num, motEtsecHEnd0Resources },
    { "motetsecHEnd", 1, VXB_BUSID_PLB, 0, etsecHEnd1Num, motEtsecHEnd1Resources },
    { "motetsecHEnd", 2, VXB_BUSID_PLB, 0, etsecHEnd2Num, motEtsecHEnd2Resources },
    { "motetsecHEnd", 3, VXB_BUSID_PLB, 0, etsecHEnd3Num, motEtsecHEnd3Resources },
#endif
    { "phy", 0, VXB_BUSID_MII, 0, phy0Num, phy0Resources },
    { "phy", 1, VXB_BUSID_MII, 0, phy1Num, phy1Resources },
    { "phy", 2, VXB_BUSID_MII, 0, phy2Num, phy2Resources },
    { "phy", 3, VXB_BUSID_MII, 0, phy3Num, phy3Resources },

#ifdef INCLUDE_RAPIDIO_BUS

    { "m85xxRio", 0, VXB_BUSID_PLB, 0, m85xxRio0Num, m85xxRio0Resources },
    { "m85xxCPU", 0, VXB_BUSID_RAPIDIO, 0, m85xxCPU0Num, m85xxCPU0Resources },
    { "m85xxCPU", 1, VXB_BUSID_RAPIDIO, 0, m85xxCPU1Num, m85xxCPU1Resources },
    { "m85xxCCSR", 0, VXB_BUSID_PLB, 0, m85xxLawBarNum, m85xxLawBarResources },
    { "smEnd", 0, VXB_BUSID_PLB,0, smEnd0Num, smEnd0Resources}
#endif
};
const int hcfDeviceNum = NELEMENTS(hcfDeviceList);

VXB_INST_PARAM_OVERRIDE sysInstParamTable[] =
    {
    { NULL, 0, NULL, VXB_PARAM_END_OF_LIST, {(void *)0} }
    };
