/*
 * BCM53xx RoboSwitch utility functions
 *
 * Copyright (C) 2002 Broadcom Corporation
 *
 * $Id: if_robo.c,v 1.1 2004/02/24 07:47:00 csm Exp $
 */

#if 0 /* remove for vxworks */
#include <typedefs.h>
#ifdef __linux__
#include <linux/module.h>
#include <linux/netdevice.h>
#include <et_linux_priv.h>
#elif _CFE_
#include <et_cfe.h>
#endif
#include <osl.h>
#include <sbutils.h>
#include <nvutils.h>
#include <proto/ethernet.h>
#endif /* remove for vxworks */
#include <etc53xx.h>

#include "if_robo.h"

#ifdef _CFE_
extern et_info_t* et_softc;
#endif

#if 1
#undef OSL_DELAY
#define OSL_DELAY(a)
#endif

#if 1 /* add for vxworks */
#ifdef MALLOC
#undef MALLOC
#endif
#define	MALLOC(size)		malloc(size)
#define	MFREE(addr, size)	free(addr)
#include <osl.h>
#include <bcmendian.h>
#include <ethernet.h>
#include <hnddma.h>
#include <et_dbg.h>
#include <sbconfig.h>
#include <sbutils.h>
#include <bcmenet47xx.h>
#include <bcmutils.h>
#include <et_export.h>		/* for et_phyxx() routines */
#include "et_priv.h"
extern struct bcm4xxx *mpCh[2];
#endif /* add for vxworks */

robo_driver_t *robo_driver;

robo_driver_t drv_table[] = {
  { NULL, 0, 0 },  
  { NULL, (ROBO_READ)robosw_rreg_pmii, (ROBO_WRITE)robosw_wreg_pmii }
};


#define ROBO_POLL_DELAY_US 200000

robo_info_pmii_t *
robosw_attach_pmii(void)
{
    robo_info_pmii_t *robo;
#if 0 /* remove for vxworks */
    et_info_t *et;
#ifdef __linux__
    struct net_device *eth_dev; /* the ethernet device */
#endif
#endif /* remove for vxworks */
    /* set up type of register access based on board type */
    robo_driver = &drv_table[1];
    
    /* Allocate private state */
    if (!(robo = MALLOC(sizeof(robo_info_pmii_t)))) {
	printf("robosw_attach_pmii: out of memory");
	return NULL;
    }
    bzero((char *) robo, sizeof(robo_info_pmii_t));

#ifdef __linux__
    if (!(eth_dev = dev_get_by_name("eth0"))) {
	printf("robosw_attach_pmii: failed to get eth0 device handle");
	return NULL;
    }
    et = (et_info_t *)eth_dev->priv;
#elif _CFE_
    et = et_softc;
#endif	

    if (mpCh[0] == NULL)
    {
    	printf("robosw_attach_pmii: unable to get eth device structure\n");
    	return NULL;
    }
    robo->ch = mpCh[0];
    robo->phyrd = chipphyrd;
    robo->phywr = chipphywr;

#ifdef __linux__
    dev_put(eth_dev);
#endif
    return robo;
}

/* Release access to the RoboSwitch */
void
robosw_detach_pmii(robo_info_pmii_t *robo)
{
    /* Free private state */
    MFREE(robo, sizeof(robo_info_pmii_t));
}

/* PMII poll for complete */
int
robosw_pmii_poll(robo_info_pmii_t *robo)
{
    uint i, timeout;
  
    /* Timeout after 100 tries without op_code 0 */
    for (i = 0, timeout = 100; timeout;) {
	
	/* check for op_code = 0 */
	if (PMII_RREG(robo, PMII_ADDR_REG) & PMII_OPCODE_MASK){
	    timeout--;
	    /* sleep, unless in interrupt mode */
		OSL_DELAY(ROBO_POLL_DELAY_US);
	} else
	    break;
    }

    if (timeout == 0) {
	printf("robosw_pmii_poll: timeout");
	return -1;
    }

    return 0;
}

/* Write chip register */
void
robosw_wreg_pmii(robo_info_pmii_t *robo,
		 uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len)
{
    uint16 data[4];
    uint phyaddr;
    int index;

    if (len > sizeof(data))
	return;

    memcpy(&data[0],buf,len);
  
    /* write phy port regs directly, all others from pseudo phy */
    if (page >= ROBO_PORT0_MII_PAGE && page <= ROBO_IM_PORT_PAGE)
    {
	    phyaddr = page - ROBO_PORT0_MII_PAGE;
	    (*(robo->phywr))(robo->ch,phyaddr,addr/2,data[0]);
	    return;
    }

    /*set up page */
    PMII_WREG(robo, PMII_PAGE_REG, PMII_FORMAT_PAGE(page));

    /* set up 16-bit registers (1-4), based on length */
    index = 0;
    switch(len)
    {
        case 8:
    	PMII_WREG(robo, PMII_REG_WORD4, data[index]);
        index++;
        case 6:
    	PMII_WREG(robo, PMII_REG_WORD3, data[index]);
        index++;
        case 4:
    	PMII_WREG(robo, PMII_REG_WORD2, data[index]);
        index++;
        case 2:
        case 1:
    	/* if 1 byte, move to lo byte */
    	if (len == 1)
    	    data[0] >>=8;
    	PMII_WREG(robo, PMII_REG_WORD1, data[index]);
  }
  
    /* now set op code to write and wait till done */
    PMII_WREG(robo, PMII_ADDR_REG, PMII_FORMAT_ADDR_WR(addr));
    robosw_pmii_poll(robo);
}

/* Read chip register */
void
robosw_rreg_pmii(robo_info_pmii_t *robo,
		 uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len)
{
    uint16 data[4];
    unsigned char data8[8];
    uint phyaddr;
    int index;

    if (len > sizeof(data))
	    return;

    /* read phy port regs directly, all others from pseudo phy */
    if (page >= ROBO_PORT0_MII_PAGE && page <= ROBO_IM_PORT_PAGE)
    {
	    phyaddr = page - ROBO_PORT0_MII_PAGE;
	    data[0] = (*(robo->phyrd))(robo->ch,phyaddr,addr/2);
	    memcpy(buf,&data[0],len);
	    return;
    }

    /*set up page */
    PMII_WREG(robo, PMII_PAGE_REG, PMII_FORMAT_PAGE(page));

    /* now set op code to write and wait till done */
    PMII_WREG(robo, PMII_ADDR_REG, PMII_FORMAT_ADDR_RD(addr));

    /* timed out, just return */
    if (robosw_pmii_poll(robo))
	    return;
    index = 0;
    switch(len)
    {
        case 8:
        data[index] = PMII_RREG(robo, PMII_REG_WORD4);
        index ++;
        case 6:
    	data[index] = PMII_RREG(robo, PMII_REG_WORD3);
        index ++;
        case 4:
    	data[index] = PMII_RREG(robo, PMII_REG_WORD2);
        index ++;
        case 2:
        case 1:
    	data[index] = PMII_RREG(robo, PMII_REG_WORD1);
        if (len == 1)
            /* copy MSB to LSB for memcpy */
            data[0] <<= 8;
    }
    memcpy(&data8,&data,sizeof(data8));
    memcpy(buf,&data[0],len);    
}

