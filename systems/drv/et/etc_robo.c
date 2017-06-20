/*
 * $Id: etc_robo.c,v 1.27 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BCM53xx RoboSwitch utility functions
 */

/* #define EXPORT_SYMTAB */
#if defined(linux)
#include <linux-bde.h>
#include <typedefs.h>
#include <shared/et/linux_osl.h>
#include <sbutils.h>
#include <nvutils.h>
#include <proto/ethernet.h>
#include <et_dbg.h>
#include <etc53xx.h>
#include <sal/core/thread.h>
#elif defined(VXWORKS)
#include <hnbutypedefs.h>
#include <osl.h>
#include <sbconfig.h>
#include <sbutils.h>
#include <ethernet.h>
#include <et_dbg.h>
#include <etc53xx.h>
#endif

#define OSL_DELAY_NULL(a)

#define ROBO_DEFAULT_SPI_TIMEOUT    50
#define ROBO_EBBUS_TIMEOUT  200

#define SPI_PAGE            0xFF /* SPI global PAGE */
#define SPI_ROBO_RESET      0xFF /* SPI PAGE register */
#define SPI_ROBO_STRAP_CID  6    /* The chip ID (CID) of slave 8051 on SPI bus */
#define SPI_ROBO_STRAP_PAGE 0    /* The page # of slave 8051 on SPI bus */
#define SPI_ROBO_STRAP_MASK 0xF  /* Bit 0-3 reserved for the offset of strap pin setting */
#define ROBO_HW_RESET_GPIO  0x1  /* GPIO pin 0 (bitmask 0x1) used for HW reset in BCM5836/4704 */

/* Private state per RoboSwitch */
typedef struct {
    void *sbh;          /* SiliconBackplane handle */
    uint coreidx;           /* Current core index */
    uint32 ssl, clk, mosi, miso;    /* GPIO mapping */
    int cid, page;          /* Current chip ID and page */
       void *ebbase;
       uint8  bustype;
#ifdef _KERNEL_
    spinlock_t lock;        /* per-device perimeter lock */    
#endif
} robo_info_t;

/* Forward declarations */
void *
robo_attach(void *sbh, uint32 ssl, uint32 clk, uint32 mosi, uint32 miso);
void robo_detach(void *robo);
void
robo_wreg(void *rinfo, uint8 cid, uint8 page,
      uint8 addr, uint8 *buf, uint len);
void
robo_rreg(void *rinfo, uint8 cid, uint8 page,
      uint8 addr, uint8 *buf, uint len);
void
robo_rvmii(void *rinfo, uint8 cid);

static void *robo_sbh = NULL;

#if 0 /* ifndef _CFE_ */
EXPORT_SYMBOL(robo_attach);
EXPORT_SYMBOL(robo_detach);
EXPORT_SYMBOL(robo_wreg);
EXPORT_SYMBOL(robo_rreg);
#endif /* !_CFE */

#define ROBO_POLL_DELAY_US 10
#define EB_BUS  0
#define SPI_BUS  1
#define MDCMDIO_BUS  2
/* Get access to the RoboSwitch */
void *
robo_attach(void *sbh, uint32 ssl, uint32 clk, uint32 mosi, uint32 miso)
{
    robo_info_t *robo;
        uint32 temp;

    /* Allocate private state */
    if (!(robo = MALLOC(sizeof(robo_info_t)))) {
        ET_ERROR(("robo_attach: out of memory"));
        return NULL;
    } else {
        robo_sbh = (void *)robo;
    }
    bzero((char *) robo, sizeof(robo_info_t));
    robo->sbh = sbh;
    robo->ssl = ssl;
    robo->clk = clk;
    robo->mosi = mosi;
    robo->miso = miso;
    robo->cid = robo->page = -1;
    robo->bustype = EB_BUS;
    /* Initialize GPIO outputs */
    sb_gpioout(robo->sbh, robo->ssl | robo->clk | robo->mosi, robo->ssl);
    sb_gpioouten(robo->sbh, robo->ssl | robo->clk | robo->mosi | robo->miso,
                            robo->ssl | robo->clk | robo->mosi);
#ifdef EBBUS_BIT16
       temp=sb_EBbus_enable(robo->sbh, DATA_BIT_16);
#elif defined(EBBUS_BIT8)
       temp=sb_EBbus_enable(robo->sbh, DATA_BIT_8);
#endif
       robo->ebbase = (void*)REG_MAP(0x1A000000, 0x1000000);

#ifdef _KERNEL_
    /* Initialize lock */
    spin_lock_init(&robo->lock);
#endif
    return robo;
}




/* Release access to the RoboSwitch */
void
robo_detach(void *rinfo)
{
    robo_info_t *robo = (robo_info_t *)rinfo;

    /* Disable GPIO outputs */
    sb_gpioouten(robo->sbh, robo->ssl | robo->clk | robo->mosi, 0);

        REG_UNMAP(robo->ebbase);
    /* Free private state */
    MFREE(robo, sizeof(robo_info_t));
}

void 
robo_switch_bus(void *rinfo,uint8 bustype)
{
    robo_info_t *robo = (robo_info_t *)rinfo;

    if (bustype == SPI_BUS) {
        EB_TRACE(("Switch to SPI mode\n"));  
    } else if (bustype == MDCMDIO_BUS) {
        EB_TRACE(("Switch to Pseudo Phy mode\n"));  
    } else {
        EB_TRACE(("Switch to EB Bus mode\n"));  
    }
    robo->bustype = bustype;
}
/* Enable serial access to the chip */
static void
robo_enable(robo_info_t *robo)
{
    void *regs;

    /* Save current core index */
    robo->coreidx = sb_coreidx(robo->sbh);

    /* Switch to GPIO core for faster access */
    regs = sb_gpiosetcore(robo->sbh);
    ASSERT(regs);
}

/* Disable serial access to the chip */
static void
robo_disable(robo_info_t *robo)
{
    /* Switch back to original core */
    sb_setcoreidx(robo->sbh, robo->coreidx);
}
    
/* Write a byte stream to the chip */
static void
robo_write(robo_info_t *robo, uint8 *buf, uint len)
{
    uint i;
    uint8 mask;

    for (i = 0; i < len; i++) {
        /* Bit bang from MSB to LSB */
        for (mask = 0x80; mask; mask >>= 1) {
            /* Clock low */
            sb_gpioout(robo->sbh, robo->clk, 0);
            OSL_DELAY_NULL(10);
            if (robo->cid == SPI_ROBO_STRAP_CID)
                OSL_DELAY(20);

            /* Output on rising edge */
            if (mask & buf[i])
                sb_gpioout(robo->sbh, robo->mosi, robo->mosi);
            else
                sb_gpioout(robo->sbh, robo->mosi, 0);
        
            /* Clock high */
            sb_gpioout(robo->sbh, robo->clk, robo->clk);
            OSL_DELAY_NULL(10);
            if (robo->cid == SPI_ROBO_STRAP_CID)
                OSL_DELAY(20);
        }
    }
}

/* Handy macros for writing fixed length values */
#define robo_write8(robo, b) { uint8 val = (uint8) (b); robo_write((robo), &val, sizeof(val)); }
#define robo_write16(robo, w) { uint16 val = (uint16) (w); robo_write((robo), &val, sizeof(val)); }
#define robo_write32(robo, l) { uint32 val = (uint32) (l); robo_write((robo), &val, sizeof(val)); }

/* Read a byte stream from the chip */
static void
robo_read(robo_info_t *robo, uint8 *buf, uint len)
{
    uint i;
    uint8 rack, mask, byte;

    for (i = 0, rack = 0; i < len;) {
        /* Bit bang from MSB to LSB */
        for (mask = 0x80, byte = 0; mask; mask >>= 1) {
            /* Clock low */
            sb_gpioout(robo->sbh, robo->clk, 0);
            OSL_DELAY_NULL(10);
            if (robo->cid == SPI_ROBO_STRAP_CID)
                OSL_DELAY(20);

            sb_gpioout(robo->sbh, robo->mosi, 0);

            /* Sample */
            if (sb_gpioin(robo->sbh) & robo->miso)
                byte |= mask;

            /* Clock high */
            sb_gpioout(robo->sbh, robo->clk, robo->clk);
            OSL_DELAY_NULL(10);
            if (robo->cid == SPI_ROBO_STRAP_CID)
                OSL_DELAY(20);

        }

        buf[i] = byte;
        i++;
    }
}
/* poll for RACK */
static int
robo_poll_for_RACK(robo_info_t *robo)
{
    uint i, timeout;
    uint8 byte;

    /* Timeout after 100 tries without RACK */
    for (i = 0, timeout = ROBO_DEFAULT_SPI_TIMEOUT; timeout;) {
        robo_read(robo, &byte, sizeof(byte));
         /*  In fast read mode 
               *  check bit 0 or bit 1 are high 
               */
        if (byte & 0x03)
            break;
        else
        {
            timeout--;
            /* sleep, unless in interrupt mode */
            if (OSL_IN_INTERRUPT())
            {
#ifndef _CFE_
                OSL_DELAY(ROBO_POLL_DELAY_US);
#else
                et_delay(ROBO_POLL_DELAY_US);
#endif          
                OSL_DELAY_NULL(ROBO_POLL_DELAY_US); 
            }
            else
            {
                /* Remove delay for faster access */
                /* OSL_SLEEP(ROBO_POLL_DELAY_US); */
            }
        }
    }

    if (timeout == 0) {
        ET_ERROR(("robo_read: timeout"));
        return -1;
    }
    return 0;

}

/* poll for SPIF low */
static int
robo_poll_for_SPIF(robo_info_t *robo, uint8 cid)
{
    uint i, timeout;
    uint8 byte;
    uint data;

    /* Timeout after 100 tries without SPIF low */
    for (i = 0, timeout = ROBO_DEFAULT_SPI_TIMEOUT; timeout;) {
        sb_gpioout(robo->sbh, robo->ssl, 0);
        robo_write8(robo, 0x60 | ((cid & 0x7) << 1));
        robo_write8(robo, 0xfe);
        robo_read((robo), &byte, sizeof(char));
        sb_gpioout(robo->sbh, robo->ssl, robo->ssl);

        /* SPIF is bit 7 of SPI_STS */
            /* check SPIF = 0 ? */
        if (!(byte & 0x80))
            break;
        else
        {
            timeout--;
            data = (uint)byte;
            /* sleep, unless in interrupt mode */
            if (OSL_IN_INTERRUPT())
            {
#ifndef _CFE_
                OSL_DELAY(ROBO_POLL_DELAY_US);
#else
                et_delay(ROBO_POLL_DELAY_US);
#endif          

                OSL_DELAY_NULL(ROBO_POLL_DELAY_US);
            }
            else
            {
                OSL_SLEEP(ROBO_POLL_DELAY_US);
            }
        }
    }

    if (timeout == 0) {
        ET_ERROR(("robo_read: timeout"));
        return -1;
    }
    return 0;
}

/* Handy macros for reading fixed length values */
#define robo_read8(robo) { uint8 val; robo_read((robo), &val, sizeof(val)); val; }
#define robo_read16(robo) { uint16 val; robo_read((robo), &val, sizeof(val)); val; }
#define robo_read32(robo) { uint32 val; robo_read((robo), &val, sizeof(val)); val; }

/* Select new chip and page */
static void
robo_select(robo_info_t *robo, uint8 cid, uint8 page)
{
    /* Chip and page already selected */
    if (robo->cid == (int) cid && robo->page == (int) page)
        return;
    robo->cid = (int) cid;
    robo->page = (int) page;

    /* Enable CS */
    sb_gpioout(robo->sbh, robo->ssl, 0);
    OSL_DELAY_NULL(10);

    /* Select new chip */
    robo_write8(robo, 0x61 | ((cid & 0x7) << 1));

    /* Select new page */
    robo_write8(robo, 0xff);
    robo_write8(robo, page);

    /* Disable CS */
    sb_gpioout(robo->sbh, robo->ssl, robo->ssl);
    OSL_DELAY_NULL(10);
}


void
robo_read_EB(void *base, uint8 addr, uint8 * buf, uint8 len)
{
    volatile uint8 *ptr;
    uint8   i;

#if defined(BE_HOST) ||defined(VXWORKS)
    ptr = (uint8 *)((uint32)base + (addr^0x3));
#else
    ptr = (uint8*)((uint32)base +addr);    
#endif

    for(i=0;i<len;i++){
        buf[i] =  *ptr;
        EB_TRACE((
                "read addr %x buf %x \n",addr, buf[i]));
        ptr = (volatile uint8 *)((uint32)ptr+1);
    }

}
void
robo_write_EB(void *base, uint8 addr, uint8 *buf, uint8 len)
{
    volatile uint8 *ptr;
    uint8 i;
#if defined(BE_HOST) ||defined(VXWORKS)
    ptr = (volatile uint8 *)((uint32)base + (addr^0x3));
#else
    ptr = (volatile uint8*)((uint32)base +addr);    
#endif

    for(i=0;i<len;i++){
       *ptr = buf[i];
        EB_TRACE(( 
            "write addr %x buf %x \n", addr, buf[i]));
        ptr = (volatile uint8 *)((uint32)ptr+1);
    }
}

void
robo_read_EB16(void *base, uint8 addr, uint8 * buf, uint8 len)
{
    volatile uint16 *ptr;
    uint8   i;
    uint16  temp=0;
#if defined(BE_HOST) ||defined(VXWORKS)
    ptr = (uint16 *)((uint32)base + (addr^0x2));
#else
    ptr = (uint16 *)((uint32)base +addr);    
#endif

    for(i=0;i<len;i++){
        if(i%2==0){
#if defined(BE_HOST) ||defined(VXWORKS)
            ptr = (uint16 *)((uint32)base + ((addr+i)^0x2));
#else
            ptr = (uint16 *)((uint32)base + (addr+i));
#endif
            temp = *ptr;
            buf[i]= (uint8)(temp&0x00ff);
        }else{
            buf[i]= (uint8)((temp&0xff00)>>8);
         }
        EB_TRACE(( 
            "robo_read_EB16 %x ptr %p buf[%x] %x \n",addr,ptr,i,buf[i]));
    }


}
void
robo_write_EB16(void *base, uint8 addr, uint8 *buf, uint8 len)
{
    volatile uint16 *ptr;
    uint8 i;
    uint16 temp=0;
#if defined(BE_HOST) ||defined(VXWORKS)
    ptr = (volatile uint16 *)((uint32)base + (addr^0x2));
#else
    ptr = (volatile uint16 *)((uint32)base +addr);    
#endif

    for(i=0;i<len;i++){        

        if(i%2==0){
#if defined(BE_HOST) ||defined(VXWORKS)
            ptr = (uint16 *)((uint32)base + ((addr+i)^0x2));
#else
            ptr = (uint16 *)((uint32)base + (addr+i));
#endif
            temp = (uint16)buf[i];
        }else{
            temp |= (uint16)(buf[i] << 8);    
         }

        if(( i%2) || (i == len-1)){
            EB_TRACE((
                "Write addr %x ptr %p %04x  \n",addr,ptr,temp));
            *ptr = temp;
        }
    }
}


/* Write chip register */
void
robo_wreg(void *rinfo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len)
{
    robo_info_t *robo = (robo_info_t *)(rinfo ? rinfo : robo_sbh);
    uint8 temp, cmd;
    uint32  time_out;
#ifdef _KERNEL_
    unsigned long flags;
    spin_lock_irqsave(&robo->lock, flags);
#endif

    if (robo->bustype == SPI_BUS){    
        uint8 lcid=cid, lpage=page, laddr=addr, llen=len;
        /* (page, addr) (0xFF, 0xFx) is reserved for the slave 8051 */
        if (page == SPI_PAGE) {
            lcid = SPI_ROBO_STRAP_CID;
            lpage = SPI_ROBO_STRAP_PAGE;
            laddr = addr & SPI_ROBO_STRAP_MASK;
            llen = 1;
        }
        robo_enable(robo);
        if (robo_poll_for_SPIF(robo, lcid)){
          /* timeout */
            robo_select(robo, lcid, lpage);
#ifdef _KERNEL_
            spin_unlock_irqrestore(&robo->lock, flags);
#endif
            return;
        }

        /* Select chip and page */
        robo_select(robo, lcid, lpage);

        /* Enable CS */
        sb_gpioout(robo->sbh, robo->ssl, 0);
        OSL_DELAY_NULL(10);

        /* Write */
        robo_write8(robo, 0x61 | ((lcid & 0x7) << 1));
        robo_write8(robo, laddr);
        robo_write(robo, buf, llen);

        /* Disable CS */
        sb_gpioout(robo->sbh, robo->ssl, robo->ssl);
        OSL_DELAY_NULL(10);

        robo_disable(robo);
    } else if (robo->bustype == EB_BUS) {
        if (cid > 0) {
            /* timeout */
#ifdef _KERNEL_
            spin_unlock_irqrestore(&robo->lock, flags);
#endif
            return;
        }        
        
#ifdef EBBUS_BIT8
        {        
            uint8 i;
            /* Page /Offse*/
            robo_write_EB(robo->ebbase,0x8,&page,1);

            robo_write_EB(robo->ebbase,0x9,&addr,1);

            for (i=0;i<len;i++) {
                robo_write_EB(robo->ebbase, i, buf,1);      
                buf ++;
            }
            /* WRITE */
            cmd = 0x2;
            robo_write_EB(robo->ebbase, 0xA, &cmd,1);
            temp = 1;
            time_out = 0;

            while (temp) {
                time_out ++;
                robo_read_EB(robo->ebbase, 0xB, &temp, 1);

                if (time_out  > ROBO_EBBUS_TIMEOUT) {
                    EB_TRACE((
                    "write op time out reg %x %x\n",page,addr)); 
                    cmd = 0x0;
                    robo_write_EB(robo->ebbase,0xA,&cmd,1);
                    break;
                }
            }
        }
#elif defined(EBBUS_BIT16)
        {
            /*  PAGE & OFFSET */
            uint8  temp16[2];
            temp16[0] = page;
            temp16[1] = addr;

            robo_write_EB16(robo->ebbase,0x8,(uint8 *)&temp16,2);
            robo_write_EB16(robo->ebbase,0x8,(uint8 *)&temp16,2);

            /* DATA */
            robo_write_EB16(robo->ebbase, 0, buf,len);      

            /* Write */
            cmd = 0x2;
            robo_write_EB(robo->ebbase,0xA,&cmd,1);
            time_out = 0;
            temp = 1;
    
            while (temp) {
                time_out ++;
                robo_read_EB(robo->ebbase,0xB,&temp,1);
                if (time_out > ROBO_EBBUS_TIMEOUT) {
                    EB_TRACE((
                    "write op time out reg %x %x\n",page,addr));            
                    cmd = 0x0;
                    robo_write_EB(robo->ebbase,0xA,&cmd,1);
                    break;
                }
            }
        }
#endif

    } else {
        /* MDCMDIO bus not support here now */
    }
#ifdef _KERNEL_
    spin_unlock_irqrestore(&robo->lock, flags);
#endif

}

/* Read chip register */
void
robo_rreg(void *rinfo, uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len)
{
    robo_info_t *robo = (robo_info_t *)(rinfo ? rinfo : robo_sbh);
    uint8 temp,length,i, cmd;
    uint16  time_out;
#ifdef _KERNEL_
    unsigned long flags;
    spin_lock_irqsave(&robo->lock, flags);
#endif

    if(robo->bustype == SPI_BUS){
        uint8 lcid=cid, lpage=page, laddr=addr, llen=len;
        /* (page, addr) (0xFF, 0xFx) is reserved for the slave 8051 */
        if (page == SPI_PAGE) {
            /* (page,addr) (0xFF,0xFF) is reserved for ROBO HW reset */
            if (addr == SPI_ROBO_RESET) {
                /* Trigger the ROBO HW reset via GPIO0 on BCM5836/4704 */
                robo_enable(robo);
                /* Initialize gpio 0 as high */
                sb_gpioout(robo->sbh, ROBO_HW_RESET_GPIO, ROBO_HW_RESET_GPIO);
                /* enable gpio 0 */
                sb_gpioouten(robo->sbh, ROBO_HW_RESET_GPIO, ROBO_HW_RESET_GPIO);
                OSL_DELAY(20);
                /* Low active for the reset */
                sb_gpioout(robo->sbh, ROBO_HW_RESET_GPIO, 0);
                OSL_DELAY(20);
                /* Release the reset */
                sb_gpioout(robo->sbh, ROBO_HW_RESET_GPIO, ROBO_HW_RESET_GPIO);
                /* disable gpio 0 */
                sb_gpioouten(robo->sbh, ROBO_HW_RESET_GPIO, 0);
                robo_disable(robo);
#ifdef _KERNEL_
                spin_unlock_irqrestore(&robo->lock, flags);
#endif
                return;
            }
            lcid = SPI_ROBO_STRAP_CID;
            lpage = SPI_ROBO_STRAP_PAGE;
            laddr = addr & SPI_ROBO_STRAP_MASK;
            llen = 1;
        }
        robo_enable(robo);  
        if (robo_poll_for_SPIF(robo,lcid)) {
            /* timeout */
            robo_select(robo, lcid, lpage);
#ifdef _KERNEL_
            spin_unlock_irqrestore(&robo->lock, flags);
#endif
            return;
        }
        /* Select chip and page */
        robo_select(robo, lcid, lpage);

        /* Enable CS */
        sb_gpioout(robo->sbh, robo->ssl, 0);
        OSL_DELAY_NULL(10);

        /* Fast read */
        robo_write8(robo, 0x10 | ((lcid & 0x7) << 1));
        robo_write8(robo, laddr);
        if (robo_poll_for_RACK(robo)) {
          /* timeout */
#ifdef _KERNEL_
            spin_unlock_irqrestore(&robo->lock, flags);
#endif
            return;
        }

        robo_read(robo, buf, llen);
        OSL_DELAY_NULL(10);

        /* Disable CS */
        sb_gpioout(robo->sbh, robo->ssl, robo->ssl);
        OSL_DELAY_NULL(10);

        robo_disable(robo);

    } else if (robo->bustype == EB_BUS) {
        if(cid > 0) {
          /* timeout */
#ifdef _KERNEL_
            spin_unlock_irqrestore(&robo->lock, flags);
#endif
            return;
        }          
#ifdef EBBUS_BIT8
        {
            /* Page /Offse*/
            robo_write_EB(robo->ebbase,0x8,&page,1);
            robo_write_EB(robo->ebbase,0x9,&addr,1);

            /* READ */
            cmd = 1;
            robo_write_EB(robo->ebbase,0xA,&cmd,1);

            time_out = 0;
            temp = 1;
            while (temp) {
                time_out ++;
                robo_read_EB(robo->ebbase,0xB,&temp,1);
                if (time_out  > ROBO_EBBUS_TIMEOUT) {
                    EB_TRACE((
                    "read op time out %x %x\n",page,addr));
                    cmd = 0;
                    robo_write_EB(robo->ebbase,0xA,&cmd,1);
#ifdef _KERNEL_
                    spin_unlock_irqrestore(&robo->lock, flags);
#endif
                    return;
                }
            }

            robo_read_EB(robo->ebbase,0xc,&length,1);

            if (length != (0x1 << (len-1))) {
                EB_TRACE((
                " reg read length not match page %x offset %x length %x len %x\n",
                    page,addr, length,len));
                for(i=0;i<len;i++) {
                    *buf = 0;
                    buf ++;
                }
#ifdef _KERNEL_
                spin_unlock_irqrestore(&robo->lock, flags);
#endif
                return;
            }
            for (i=0; i<len; i++) {
                robo_read_EB(robo->ebbase,i,buf,1);
                buf ++;
            }
        }
#elif defined(EBBUS_BIT16)
        {
        /* READ EB bus 16-bit*/
        /*  PAGE & OFFSET */
            uint8  temp16[2];

            temp16[0] = page;
            temp16[1] = addr;
            robo_write_EB16(robo->ebbase,0x8,(uint8 *)&temp16,2);

            /* READ */
            cmd = 0x1;
            robo_write_EB(robo->ebbase,0xA,&cmd,1);
            time_out = 0;
            temp = 1;
            while (temp) {
                time_out ++;
                robo_read_EB(robo->ebbase, 0xB, &temp, 1);
                if (time_out > ROBO_EBBUS_TIMEOUT) {
                    EB_TRACE((
                    "read op time out %x %x\n",page,addr));
                    cmd = 0x0;
                    robo_write_EB(robo->ebbase,0xA,&cmd,1);
#ifdef _KERNEL_
                    spin_unlock_irqrestore(&robo->lock, flags);
#endif
                    return;
                }
            }

            robo_read_EB16(robo->ebbase,0xc,&length,1);

            if (length != (0x1 << (len-1))) {
                EB_TRACE((
                "length not match page %x offset %x length %x len %x\n",
                    page,addr, length,len));
                for(i=0;i<len;i++) {
                    *buf = 0;
                    buf ++;
                }
#ifdef _KERNEL_
                spin_unlock_irqrestore(&robo->lock, flags);
#endif
                return;
            }
            robo_read_EB16(robo->ebbase,0,buf,len);
        }
#endif

    } else {
        /* MDCMDIO bus not support here now */
    }

#ifdef _KERNEL_
    spin_unlock_irqrestore(&robo->lock, flags);
#endif

}

#define SCL(robo) (robo->clk)
#define SDAIN(robo) (robo->mosi)
#define SDAOUT(robo) (robo->miso)
#define I2C_ADDRESS_BASE 0x20 /* see LTC4259 spec. page 23. */
#define ARA_BUS_ADDRESS 0x0c /* see LTC4259 spec. page 25. */
#define READ_BIT 1
#define WRITE_BIT 0

/* Respond ACK to slave device */
void 
no_ack_by_master(robo_info_t *robo)
{   

    sb_gpioout(robo->sbh, SDAIN(robo), SDAIN(robo)); /* SDAIN HIGH */
    sb_gpioout(robo->sbh, SCL(robo), SCL(robo)); /* SCL HIGH */
    OSL_DELAY_NULL(10);
    sb_gpioout(robo->sbh, SCL(robo), 0); /* SCL LOW */

}

/* Check slave device ack signal */
int
ack_by_slave(robo_info_t *robo)
{
    uint8  Errbit;      
    sb_gpioout(robo->sbh, SCL(robo), SCL(robo)); /* SCL HIGH */
    OSL_DELAY_NULL(20);
    Errbit = (uint8)(sb_gpioin(robo->sbh) & SDAOUT(robo)); 
    sb_gpioout(robo->sbh, SCL(robo), 0); /* SCL LOW */

    return 1;
}

/* Read a byte stream from the chip */
static void
robo_i2c_read(robo_info_t *robo, uint8 *buf, uint len)
{
    uint i;
    uint8 mask, byte;

    for (i = 0; i < len;) {
        /* Bit bang from MSB to LSB */
        for (mask = 0x80, byte = 0; mask; mask >>= 1) {
            sb_gpioout(robo->sbh, SCL(robo), SCL(robo)); /* SCL HIGH */ 
            if (sb_gpioin(robo->sbh) & SDAOUT(robo))
                byte |= mask;
            OSL_DELAY_NULL(10);
            sb_gpioout(robo->sbh, SCL(robo), 0); /* SCL HIGH */ 
            OSL_DELAY_NULL(10);

        }

        buf[i] = byte;
        i++;
    }
}

/* Write a byte stream to the chip */
static void
robo_i2c_write(robo_info_t *robo, uint8 *buf, uint len)
{
    uint i;
    uint8 mask;

    for (i = 0; i < len; i++) {
        /* Bit bang from MSB to LSB */
        for (mask = 0x80; mask; mask >>= 1) {
            if (mask & buf[i])
                sb_gpioout(robo->sbh, SDAIN(robo), SDAIN(robo));
            else
                sb_gpioout(robo->sbh, SDAIN(robo), 0);

            sb_gpioout(robo->sbh, SCL(robo), SCL(robo)); /* SCL HIGH */
            OSL_DELAY_NULL(10);
            sb_gpioout(robo->sbh, SCL(robo), 0); /* SCL LOW */

        }
    }
}

#define robo_i2c_write8(robo, b) { uint8 val = (uint8) (b); robo_i2c_write((robo), &val, sizeof(val)); }

/* I2C START condition */
void
robo_i2c_start(robo_info_t *robo) 
{
    sb_gpioout(robo->sbh, SDAIN(robo), SDAIN(robo)); /* SDAIN = HIGH */
    OSL_DELAY_NULL(10);
    sb_gpioout(robo->sbh, SCL(robo), SCL(robo)); /* SCL = HIGH */
    OSL_DELAY_NULL(10);
    sb_gpioout(robo->sbh, SDAIN(robo), 0); /* SDAIN = LOW */
    OSL_DELAY_NULL(10);
    sb_gpioout(robo->sbh, SCL(robo), 0); /* SCL = LOW */
}

/* I2C STOP condition */
void
robo_i2c_stop(robo_info_t *robo) 
{
    sb_gpioout(robo->sbh, SCL(robo), SCL(robo)); /* SCL = HIGH */
    sb_gpioout(robo->sbh, SDAIN(robo), 0); /* SDAIN = LOW */
    OSL_DELAY_NULL(20);
    sb_gpioout(robo->sbh, SDAIN(robo), SDAIN(robo)); /* SDAIN = HIGH */
    sb_gpioout(robo->sbh, SCL(robo), SCL(robo)); /* SCL = HIGH */
    sb_gpioout(robo->sbh, SDAIN(robo), SDAIN(robo)); /* SDAIN = HIGH */
}

/* I2C REPEATED START condition */
void
robo_i2c_rep_start(robo_info_t *robo, uint8 buf) 
{
    robo_i2c_start(robo);
    OSL_DELAY_NULL(20);
    robo_i2c_write8(robo, buf);
    OSL_DELAY_NULL(20);
    ack_by_slave(robo); 
}

/* Write chip register */
void
robo_i2c_wreg(void *rinfo, uint8 chipid, uint8 addr, uint8 *buf, uint len)
{
    robo_info_t *robo = (robo_info_t *)rinfo;
#ifdef _KERNEL_
    unsigned long flags;
    spin_lock_irqsave(&robo->lock, flags);
#endif
    
    robo_enable(robo);

    /* Set SS High for i2c mode */
    sb_gpioout(robo->sbh, robo->ssl, robo->ssl);
    OSL_DELAY_NULL(10);

    robo_i2c_start(robo);

    robo_i2c_write8(robo, ((I2C_ADDRESS_BASE | chipid) << 1) | WRITE_BIT);
    ack_by_slave(robo);
    robo_i2c_write8(robo, addr);
    ack_by_slave(robo);
    robo_i2c_write(robo, buf, len);
    ack_by_slave(robo);

    robo_i2c_stop(robo);

    /* Remain SS High */
    sb_gpioout(robo->sbh, robo->ssl, robo->ssl);
    OSL_DELAY_NULL(10);

    robo_disable(robo);

#ifdef _KERNEL_
    spin_unlock_irqrestore(&robo->lock, flags);
#endif

}

/* Read chip register */
void
robo_i2c_rreg(void *rinfo, uint8 chipid, uint8 addr, uint8 *buf, uint len)
{
    robo_info_t *robo = (robo_info_t *)rinfo;
        
#ifdef _KERNEL_
    unsigned long flags;
    spin_lock_irqsave(&robo->lock, flags);
#endif
    robo_enable(robo);

    /* Set SS High for i2c mode */
    sb_gpioout(robo->sbh, robo->ssl, robo->ssl);
    OSL_DELAY_NULL(10);

    robo_i2c_start(robo);                           

    robo_i2c_write8(robo, ((I2C_ADDRESS_BASE | chipid) << 1) | WRITE_BIT);
    ack_by_slave(robo);
    robo_i2c_write8(robo, addr);
    ack_by_slave(robo);

    robo_i2c_rep_start(robo, ((I2C_ADDRESS_BASE | chipid) << 1) | READ_BIT);

    /* Receive data byte from salve device */
    robo_i2c_read(robo, buf, len);
    no_ack_by_master(robo);
    
    robo_i2c_stop(robo);

    /* Remain SS High */
    sb_gpioout(robo->sbh, robo->ssl, robo->ssl);
    OSL_DELAY_NULL(10);

    robo_disable(robo);

#ifdef _KERNEL_
    spin_unlock_irqrestore(&robo->lock, flags);
#endif
}

/* Read chip interrupt  register */
void
robo_i2c_rreg_intr(void *rinfo, uint8 chipid, uint8 *buf)
{
    robo_info_t *robo = (robo_info_t *)rinfo;
        
#ifdef _KERNEL_
    unsigned long flags;
    spin_lock_irqsave(&robo->lock, flags);
#endif
    robo_enable(robo);
    /* Set SS High for i2c mode */
    sb_gpioout(robo->sbh, robo->ssl, robo->ssl);
    OSL_DELAY_NULL(10);

    robo_i2c_start(robo);                           
    robo_i2c_write8(robo, ((I2C_ADDRESS_BASE | chipid) << 1) | READ_BIT);
    ack_by_slave(robo);

    /* Get content of interrupr register */
    robo_i2c_read(robo, buf, sizeof(uint8));
    no_ack_by_master(robo);
    
    /* Stop the transaction */
    robo_i2c_stop(robo);

    /* Remain SS High */
    sb_gpioout(robo->sbh, robo->ssl, robo->ssl);
    OSL_DELAY_NULL(10);

    robo_disable(robo);

#ifdef _KERNEL_
    spin_unlock_irqrestore(&robo->lock, flags);
#endif

}

/* Get ARA */
void
robo_i2c_read_ARA(void *rinfo, uint8 *chipid)
{
    robo_info_t *robo = (robo_info_t *)rinfo;
        
#ifdef _KERNEL_
    unsigned long flags;
    spin_lock_irqsave(&robo->lock, flags);
#endif
    robo_enable(robo);

    /* Set SS High for i2c mode */
    sb_gpioout(robo->sbh, robo->ssl, robo->ssl);
    OSL_DELAY_NULL(10);

    robo_i2c_start(robo);                          
    robo_i2c_write8(robo, (ARA_BUS_ADDRESS << 1) | READ_BIT);
    ack_by_slave(robo);

    /* Get value from slave */
    robo_i2c_read(robo, chipid, sizeof(uint8));
    no_ack_by_master(robo);
    
    /* Stop the transaction */
    robo_i2c_stop(robo);

    /* Remain SS High */
    sb_gpioout(robo->sbh, robo->ssl, robo->ssl);
    OSL_DELAY_NULL(10);

    robo_disable(robo);

    if (*chipid & 0x41) {
        *chipid >>= 1;
        *chipid &=0x0f;
    }

#ifdef _KERNEL_
    spin_unlock_irqrestore(&robo->lock, flags);
#endif

}


/* Enable reverse MII mode */
void
robo_rvmii(void *rinfo, uint8 cid)
{
        robo_info_t *robo = (robo_info_t *)rinfo;
    uint8 mii;

    /* MII port state override (page 0 register 14) */
    robo_rreg(robo, cid, ROBO_CTRL_PAGE,
          ROBO_PORT_OVERRIDE_CTRL, &mii, sizeof(mii));

    /* Bit 4 enables reverse MII mode */
    if(mii & ROBO_PORT_OVERRIDE_RVMII)
        return;

    mii |= ROBO_PORT_OVERRIDE_RVMII;

    robo_wreg(robo, cid, ROBO_CTRL_PAGE,
          ROBO_PORT_OVERRIDE_CTRL, &mii, sizeof(mii));
    /* Read back */
    robo_rreg(robo, cid, ROBO_CTRL_PAGE,
          ROBO_PORT_OVERRIDE_CTRL, &mii, sizeof(mii));

    if (!(mii & ROBO_PORT_OVERRIDE_RVMII)) {
        ET_ERROR(("robo_rvmii: error enabling mode"));
    }

    
#ifdef _CFE_
    {
        #include <bcmnvram.h>
        uint8 tmp;
        char *var;

        /* Board specific options */
        if((var = nvram_get("boardtype"))){
        if (!strcmp(var, "bcm95380_rr")){

            /* Show chiprev on 53{3,8}z for sanity check */
            /* Read PartID */
            robo_rreg(robo, cid, 0x02, 0x30, &mii, sizeof(mii));

            /* Read RevID */
            robo_rreg(robo, cid, 0x02, 0x40, &tmp, sizeof(tmp));
            printf("robo0: BCM5380 Part#%x, Rev %x\n", mii, tmp);
        }
        }
       }
#endif      

}

void 
robo_select_device(void *rinfo, uint16 phyidh, uint16 phyidl)
{
    /* Select pseudo phy access type by chips */
    /* Not supported here now */
}

