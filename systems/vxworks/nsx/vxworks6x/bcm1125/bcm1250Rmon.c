/* $Id: bcm1250Rmon.c,v 1.3 2011/07/21 16:14:49 yshtil Exp $ */
#include "vxWorks.h"
#include "stdio.h"
#include "bcm1250Lib.h"
#include "etherLib.h"
#include "config.h"
#include "end.h"
#include "endLib.h"
#include "bcm1250MacEnd.h"                                                      

#ifndef SB_MAC_REG_READ
#define SB_MAC_REG_READ(reg)  \
    MIPS3_LD(pDrvCtrl->sbm_macbase + reg)
#endif /* BCM1250_MAC_REG_READ */
 
#ifndef SB_MAC_REG_WRITE
#define SB_MAC_REG_WRITE(reg,val)  \
    MIPS3_SD((pDrvCtrl->sbm_macbase + reg), (val))
#endif /* SB1_MAC_REG_WRITE */                                                  

void sbrmonclr(int inst)
{
    DRV_CTRL * pDrvCtrl = (DRV_CTRL *) endFindByName ("sbe", inst);
 
    pDrvCtrl = (DRV_CTRL *) endFindByName ("sbe", inst);
    if ( pDrvCtrl == NULL )
        printf("can't find sbe%d\n", inst);
 
 
    SB_MAC_REG_WRITE(R_MAC_RMON_TX_BYTES, 0);
    SB_MAC_REG_WRITE(R_MAC_RMON_TX_BAD, 0);
    SB_MAC_REG_WRITE(R_MAC_RMON_TX_GOOD, 0);
 
}
 
void sbrmonshow(int inst)
{
    DRV_CTRL * pDrvCtrl = (DRV_CTRL *) endFindByName ("sbe", inst);
    UINT64   reg;
    UINT32   reg_hi, reg_lo;
 
    pDrvCtrl = (DRV_CTRL *) endFindByName ("sbe", inst);
    if ( pDrvCtrl == NULL )
        printf("can't find sbe%d\n", inst);
 
 
    reg = SB_MAC_REG_READ(R_MAC_RMON_TX_BYTES);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("tx_byte = 0x%08x%08x \n", reg_hi, reg_lo);
 
    reg = SB_MAC_REG_READ(R_MAC_RMON_TX_BAD);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("tx_bad = 0x%08x%08x \n", reg_hi, reg_lo);
 
    reg = SB_MAC_REG_READ(R_MAC_RMON_TX_GOOD);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("tx_good = 0x%08x%08x \n", reg_hi, reg_lo);
 
}                                                                               
