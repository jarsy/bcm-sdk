/*
 * BCM53xx RoboSwitch utility functions
 *
 * Copyright (C) 2002 Broadcom Corporation
 *
 * $Id: if_robo.h,v 1.1 2004/02/24 07:47:00 csm Exp $
 */

#ifndef _robo_h_
#define _robo_h_

#if 1 /* add for vxworks */
#include "vx_osl.h"
#endif /* add for vxworks */

/* PMII definitions */
#define PMII_PHYADDR 0x1e
#define PMII_PAGE_REG     16
#define PMII_ADDR_REG     17
#define PMII_ACCESS_STAT  18
#define PMII_REG_WORD1    24
#define PMII_REG_WORD2    25
#define PMII_REG_WORD3    26
#define PMII_REG_WORD4    27
#define PMII_MDC_ACCESS_ENB 0x1
#define PMII_WRITE        0x1
#define PMII_READ         0x2
#define PMII_OPCODE_MASK  0x3
#define PMII_FORMAT_PAGE(page) ((page<<8)  | PMII_MDC_ACCESS_ENB)
#define PMII_FORMAT_ADDR_WR(addr) ((addr<<8)  | PMII_WRITE)
#define PMII_FORMAT_ADDR_RD(addr) ((addr<<8)  | PMII_READ)

/* Private state per RoboSwitch */
typedef struct {
	void *sbh;			/* SiliconBackplane handle */
	uint coreidx;			/* Current core index */
	uint32 ssl, clk, mosi, miso;	/* GPIO mapping */
	int cid, page;			/* Current chip ID and page */
} robo_info_t;

typedef struct {
	void *ch;			
	uint16 (*phyrd)(void *ch, uint phyaddr, uint reg);	          /* read phy register */
	void (*phywr)(void *ch, uint phyaddr, uint reg, uint16 val);	/* write phy register */
} robo_info_pmii_t;

typedef void (*ROBO_READ)(robo_info_t *robo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len);
typedef void (*ROBO_WRITE)(robo_info_t *robo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len);

robo_info_t *robosw_attach(void *sbh, uint32 ssl, uint32 clk, uint32 mosi, uint32 miso);
void robosw_detach(robo_info_t *robo);
robo_info_pmii_t *robosw_attach_pmii(void);
void robosw_detach_pmii(robo_info_pmii_t *robo);
void robosw_rreg(robo_info_t *robo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len);
void robosw_wreg(robo_info_t *robo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len);
void robosw_rreg_pmii(robo_info_pmii_t *robo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len);
void robosw_wreg_pmii(robo_info_pmii_t *robo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len);

typedef struct robo_driver_s {
  char  *drv_name;
  ROBO_READ robo_read;
  ROBO_WRITE robo_write;
} robo_driver_t;

#define PMII_WREG(robo,reg,val)  (*(robo->phywr))(robo->ch,PMII_PHYADDR,reg,val)
#define PMII_RREG(robo,reg)  (*(robo->phyrd))(robo->ch,PMII_PHYADDR,reg)

int robosw_pmii_poll(robo_info_pmii_t *robo);
void robosw_wreg_pmii(robo_info_pmii_t *robo,
		      uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len);
void
robosw_rreg_pmii(robo_info_pmii_t *robo,
		 uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len);


#endif /* _robo_h_ */
