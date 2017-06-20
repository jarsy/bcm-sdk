/*
 * $Id: reg.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Register address and value manipulations.
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <soc/debug.h>

#include <soc/robo/mcm/driver.h>
#include <soc/error.h>
#include <soc/drv.h>
#include <soc/spi.h>



#define VALUE_TOO_BIG_FOR_FIELD     ((fldbuf[i] & ~mask) != 0)


/************************************************************************/
/* Routines for reading/writing SOC internal registers                  */
/************************************************************************/

struct drv_reg_page_info{
    uint32 page_min;
    uint32 page_max;    
};

typedef struct  drv_reg_page_info drv_reg_page_info_t;

#ifdef BCM_HARRIER_SUPPORT
struct drv_reg_page_info bcm53242_reg_page_table[]={
{0xa0, 0xb7},   /* MII_PORT */
{0xd8, 0xdc},   /* EXT_GMII_PORT */
{0x68, 0x84},   /* MIB_PORT */
{0xf0, 0xf7},   /* SPI */
{-1, -1}
};

struct drv_reg_page_info bcm53262_reg_page_table[]={
{0xa0, 0xb7},   /* MII_PORT */
{0xb8, 0xbc},   /* SERDES_PORT */
{0xd8, 0xdc},   /* EXT_GMII_PORT */
{0x68, 0x84},   /* MIB_PORT */
{0xf0, 0xf7},   /* SPI */
{-1, -1}
};
#endif

#if defined(BCM_VULCAN_SUPPORT)|| defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
struct drv_reg_page_info bcm53115_reg_page_table[]={
{0x10, 0x15},   /* MII_PORT */
{0x20, 0x28},   /* MIB_PORT */
{0x88, 0x88},   /* MII_PORT_EXT */
{0xf0, 0xf7},   /* SPI */
{-1, -1}
};
#endif

#if defined(BCM_BLACKBIRD_SUPPORT)|| defined(BCM_BLACKBIRD2_SUPPORT)
struct drv_reg_page_info bcm53118_reg_page_table[]={
{0x10, 0x17},   /* MII_PORT */
{0x20, 0x28},   /* MIB_PORT */
{0x88, 0x88},   /* MII_PORT_EXT */
{0xf0, 0xf7},   /* SPI */
{-1, -1}
};
#endif

#ifdef BCM_TB_SUPPORT
struct drv_reg_page_info bcm53280_reg_page_table[]={
{0xa0, 0xb7},   /* MII_PORT */
{0xb9, 0xbc},   /* SERDES_PORT */
{0xd8, 0xdc},   /* EXT_GMII_PORT */
{0xf0, 0xf7},   /* SPI */
{-1, -1}
};
#endif /* BCM_TB_SUPPORT */

#ifdef BCM_LOTUS_SUPPORT
struct drv_reg_page_info bcm53101_reg_page_table[]={
{0x10, 0x15},   /* MII_PORT */
{0x20, 0x28},   /* MIB_PORT */
{0x88, 0x88},   /* MII_PORT_EXT */
{0xf0, 0xf7},   /* SPI */
{-1, -1}
};
#endif

#ifdef BCM_VO_SUPPORT
struct drv_reg_page_info bcm53600_reg_page_table[]={
{0xa0, 0xb7},   /* MII_PORT */
{0xc0, 0xd7},   /* EXT_MII_PORT */    
{0xbb, 0xbc},   /* SERDES_PORT */
{0xd8, 0xdc},   /* EXT_GMII_PORT */
{0xf0, 0xf7},   /* SPI */
{-1, -1}
};
#endif /* BCM_VO_SUPPORT */

#ifdef BCM_POLAR_SUPPORT
struct drv_reg_page_info bcm89500_reg_page_table[]={
{0x10, 0x14},   /* MII_PORT */
{0x15, 0x15},   /* SERDES_PORT */
{0x20, 0x28},   /* MIB_PORT */
{0x80, 0x88},   /* MII_PORT_EXT */
{0xf0, 0xf7},   /* SPI */
{-1, -1}
};
#endif

#ifdef BCM_NORTHSTAR_SUPPORT
struct drv_reg_page_info bcm53010_reg_page_table[]={
{0x10, 0x14},   /* MII_PORT */
{0x20, 0x28},   /* MIB_PORT */
{0xf0, 0xf7},   /* SPI */
{-1, -1}
};
#endif

#ifdef BCM_DINO8_SUPPORT
struct drv_reg_page_info bcm5389_reg_page_table[]={
{0x10, 0x18},   /* MII_PORT */
{0x20, 0x28},   /* MIB_PORT */
{0x80, 0x88},   /* MII_PORT */
{0xf0, 0xf7},   /* SPI */
{-1, -1}
};
#endif  /* BCM_DINO8_SUPPORT */

#ifdef BCM_NORTHSTARPLUS_SUPPORT
struct drv_reg_page_info bcm53020_reg_page_table[]={
{0x10, 0x17},   /* MII_PORT */
{0x20, 0x28},   /* MIB_PORT */
{0x80, 0x88},   /* MII_PORT_EXT */
{0xf0, 0xf7},   /* SPI */
{-1, -1}
};
#endif

#ifdef BCM_DINO16_SUPPORT
struct drv_reg_page_info bcm5396_reg_page_table[]={
{0x10, 0x1F},   /* MII_PORT */
{0x50, 0x60},   /* MIB_PORT */
{0x80, 0x8f},   /* MII_PORT */
{0xf0, 0xf7},   /* SPI */
{-1, -1}
};
#endif  /* BCM_DINO16_SUPPORT */

int
_drv_reg_page_check(int unit, uint32 addr,uint32 *offset) {

    int i;
    uint32 page;
    uint8 flag;

     /* address checking */
    page = (addr >> SOC_ROBO_PAGE_BP) & 0xFF;
    flag = 0;

    if(SOC_IS_ROBO53242(unit)) {
#ifdef BCM_HARRIER_SUPPORT
        for (i = 0; i < COUNTOF(bcm53242_reg_page_table); i++) {
            if ((page >= bcm53242_reg_page_table[i].page_min) &&
                    (page <= bcm53242_reg_page_table[i].page_max)) {
                flag = 1;                    
                break;
            }                
        }
        if (flag) {
            page = bcm53242_reg_page_table[i].page_min;
        }
#endif /* BCM_HARRIER_SUPPORT */
    }else if(SOC_IS_ROBO53262(unit)) {
#ifdef BCM_HARRIER_SUPPORT

        for (i = 0; i < COUNTOF(bcm53262_reg_page_table); i++) {
            if ((page >= bcm53262_reg_page_table[i].page_min) &&
                    (page <= bcm53262_reg_page_table[i].page_max)) {
                flag = 1;                    
                break;
            }                
        }
        if (flag) {
            page = bcm53262_reg_page_table[i].page_min;
        }        
#endif /* BCM_HARRIER_SUPPORT */
    } else if(SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_VULCAN_SUPPORT)|| defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        for (i = 0; i < COUNTOF(bcm53115_reg_page_table); i++) {
            if ((page >= bcm53115_reg_page_table[i].page_min) &&
                    (page <= bcm53115_reg_page_table[i].page_max)) {
                flag = 1;                    
                break;
            }                
        }
        if (flag) {
            page = bcm53115_reg_page_table[i].page_min;
        }
#endif
    } else if(SOC_IS_BLACKBIRD(unit) || SOC_IS_BLACKBIRD2(unit)) {
#if defined(BCM_BLACKBIRD_SUPPORT)|| defined(BCM_BLACKBIRD2_SUPPORT)
        for (i = 0; i < COUNTOF(bcm53118_reg_page_table); i++) {
            if ((page >= bcm53118_reg_page_table[i].page_min) &&
                    (page <= bcm53118_reg_page_table[i].page_max)) {
                flag = 1;                    
                break;
            }                
        }
        if (flag) {
            page = bcm53118_reg_page_table[i].page_min;
        }
#endif 
    } else if(SOC_IS_TB(unit)) {
#ifdef BCM_TB_SUPPORT
        for (i = 0; i < COUNTOF(bcm53280_reg_page_table); i++) {
            if ((page >= bcm53280_reg_page_table[i].page_min) &&
                    (page <= bcm53280_reg_page_table[i].page_max)) {
                flag = 1;                    
                break;
            }                
        }
        if (flag) {
            page = bcm53280_reg_page_table[i].page_min;
        }
#endif /* BCM_TB_SUPPORT */
	} else if(SOC_IS_LOTUS(unit)) {
#ifdef BCM_LOTUS_SUPPORT	
        for (i = 0; i < COUNTOF(bcm53101_reg_page_table); i++) {
            if ((page >= bcm53101_reg_page_table[i].page_min) &&
                    (page <= bcm53101_reg_page_table[i].page_max)) {
                flag = 1;                    
                break;
            }                
        }
        if (flag) {
            page = bcm53101_reg_page_table[i].page_min;
        }
#endif /* BCM_LOTUS_SUPPORT */        
    } else if(SOC_IS_VO(unit)) {
#ifdef BCM_VO_SUPPORT
        for (i = 0; i < COUNTOF(bcm53600_reg_page_table); i++) {
            if ((page >= bcm53600_reg_page_table[i].page_min) &&
                    (page <= bcm53600_reg_page_table[i].page_max)) {
                flag = 1;                    
                break;
            }                
        }
        if (flag) {
            page = bcm53600_reg_page_table[i].page_min;
        }
#endif /* BCM_VO_SUPPORT */    
    } else if(SOC_IS_POLAR(unit)) {
#ifdef BCM_POLAR_SUPPORT
        for (i = 0; i < COUNTOF(bcm89500_reg_page_table); i++) {
            if ((page >= bcm89500_reg_page_table[i].page_min) &&
                    (page <= bcm89500_reg_page_table[i].page_max)) {
                flag = 1;                    
                break;
            }                
        }
        if (flag) {
            page = bcm89500_reg_page_table[i].page_min;
        }
#endif
    } else if (SOC_IS_NORTHSTAR(unit)) {
#ifdef BCM_NORTHSTAR_SUPPORT
        for (i = 0; i < COUNTOF(bcm53010_reg_page_table); i++) {
            if ((page >= bcm53010_reg_page_table[i].page_min) &&
                    (page <= bcm53010_reg_page_table[i].page_max)) {
                flag = 1;                    
                break;
            }                
        }
        if (flag) {
            page = bcm53010_reg_page_table[i].page_min;
        }
#endif
    } else if (SOC_IS_DINO8(unit)) {
#ifdef BCM_DINO8_SUPPORT
        for (i = 0; i < COUNTOF(bcm5389_reg_page_table); i++) {
            if ((page >= bcm5389_reg_page_table[i].page_min) &&
                    (page <= bcm5389_reg_page_table[i].page_max)) {
                flag = 1;                    
                break;
            }                
        }
        if (flag) {
            page = bcm5389_reg_page_table[i].page_min;
        }
#endif		
    } else if (SOC_IS_NORTHSTARPLUS(unit)) {
#ifdef BCM_NORTHSTARPLUS_SUPPORT
        for (i = 0; i < COUNTOF(bcm53020_reg_page_table); i++) {
            if ((page >= bcm53020_reg_page_table[i].page_min) &&
                    (page <= bcm53020_reg_page_table[i].page_max)) {
                flag = 1;                    
                break;
            }                
        }
        if (flag) {
            page = bcm53020_reg_page_table[i].page_min;
        }
#endif
    } else if (SOC_IS_DINO16(unit)) {
#ifdef BCM_DINO16_SUPPORT
        for (i = 0; i < COUNTOF(bcm5396_reg_page_table); i++) {
            if ((page >= bcm5396_reg_page_table[i].page_min) &&
                    (page <= bcm5396_reg_page_table[i].page_max)) {
                flag = 1;                    
                break;
            }                
        }
        if (flag) {
            page = bcm5396_reg_page_table[i].page_min;
        }
#endif	
    }


    if (flag == 1) {
        *offset = (addr & 0xFF) | (page << SOC_ROBO_PAGE_BP) ;
    } else {
        *offset = addr & 0xFFFF;
    }

    return SOC_E_NONE;
}

/************************************************************************/
/* Routines for reading/writing SOC internal registers                  */
/************************************************************************/
 /*
 *  Function : drv_reg_read
 *
 *  Purpose :
 *      Read an internal SOC register (8-, 16-, 32-, 48- and 64-bit register).
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      addr   :   Address that combine with Page number and register address.
 *      data     :   (OUT)Result stored here.
 *      len     :   number of bytes to be read.
 *
 *  Return :
 *      SOC_E_NONE      :   success
 *      SOC_E_PARAM    :   parameter error
 *
 *  Note :
 *      
 *
 */
int
drv_reg_read(int unit, uint32 addr, void *data, int len)
{
    int         rv = SOC_E_NONE;
#ifdef BROADCOM_DEBUG
    uint32      i;
    int         reg_len = 0, reg_numels=0;
    uint32      reg_offset=0;
    uint32      mask_lo;
    uint32      mask_hi;
    uint32          offset;
    uint32      *data_ptr;
#else
#ifdef BE_HOST
    uint32  i;
#endif
#endif
    uint64      data_rw;
    uint8           *data8_ptr;
#ifdef BE_HOST
    uint8       tmp;
#endif

    COMPILER_64_ZERO(data_rw);

    LOG_VERBOSE(BSL_LS_SOC_REG,
                (BSL_META_U(unit,
                            "drv_reg_read: unit = %d, addr = %x, length = %d\n"),
                 unit, addr, len));

#ifdef BROADCOM_DEBUG

    _drv_reg_page_check(unit, addr, &offset);

    /* address checking */
    for (i = 0; i < NUM_SOC_ROBO_REG; i++) {

        if (&SOC_REG_INFO(unit,i) == NULL) {            
            continue;
        }
        reg_numels = SOC_REG_INFO(unit, i).numels;

        reg_offset = SOC_REG_INFO(unit, i).offset;
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, i);

        if ((offset == reg_offset) ||((offset > reg_offset) &&
                 (offset < (reg_offset + reg_numels*reg_len)))) {
                break;
        }
        
    }    
    
    if ((i >= NUM_SOC_ROBO_REG) || (len != reg_len)) {
        if (!LOG_CHECK(BSL_LS_SOC_REG | BSL_INFO)){
            return SOC_E_PARAM;
        }
    }
#endif

    SPI_LOCK;

    rv = soc_spi_read(unit, addr, (uint8 *)&data_rw, len);

    if (SOC_FAILURE(rv)) {
        LOG_INFO(BSL_LS_SOC_REG,
                 (BSL_META_U(unit,
                             "drv_reg_read: error reading unit=0x%x addr=0x%x\n"), 
                  unit, addr));
        soc_event_generate(unit, SOC_SWITCH_EVENT_ACCESS_ERROR, 
                addr, (uint32)len, rv);
    }
    
    SPI_UNLOCK;

    /* endian translation */
    data8_ptr = (uint8 *)&data_rw;
#ifdef BROADCOM_DEBUG
    if (SOC_REG_IS_COUNTER(unit, i)) {
        mask_lo = 0xffffffff;
        mask_hi = (SOC_REG_IS_64(unit, i))? 0xffffffff : 0x0;
    } else {
        /* mask with reset mask value */
        mask_lo = SOC_REG_INFO(unit, i).rst_mask_lo;
        mask_hi = SOC_REG_INFO(unit, i).rst_mask_hi;
    }

    LOG_INFO(BSL_LS_SOC_REG,
             (BSL_META_U(unit,
                         "drv_reg_read: addr=0x%04x data = 0x"), addr));
    for (i = 0; i < len; i++) {
        LOG_INFO(BSL_LS_SOC_REG,
                 (BSL_META_U(unit,
                             "%02x"), data8_ptr[len-i-1]));
    }
    LOG_INFO(BSL_LS_SOC_REG,
             (BSL_META_U(unit,
                         "\n")));
#endif
#ifdef BE_HOST
    if (len > 4) {
        for (i=0; i < 4; i++) {
            tmp = data8_ptr[i];
            data8_ptr[i] = data8_ptr[7-i];
            data8_ptr[7-i] = tmp;
        }
    } else {
        for (i = 0; i < 2; i++) {
            tmp = data8_ptr[i];
            data8_ptr[i] = data8_ptr[3-i];
            data8_ptr[3-i] = tmp;
        }
    }
#endif
    
#ifdef BROADCOM_DEBUG
    data_ptr = (uint32 *)data;
    *data_ptr = *data_ptr & mask_lo;
    if (len > 4) {
        *(data_ptr + 1) = *(data_ptr  +1) & mask_hi;
    }
#endif
 
    if (len > 4) {
        sal_memcpy(data, data8_ptr, 8);
        LOG_VERBOSE(BSL_LS_SOC_REG,
                    (BSL_META_U(unit,
                                "len > 4 ")));
    } else {
        sal_memcpy(data, data8_ptr, 4);
        LOG_VERBOSE(BSL_LS_SOC_REG,
                    (BSL_META_U(unit,
                                "len <= 4 ")));
    }
    return rv;
}


 /*
 *  Function : drv_reg_write
 *
 *  Purpose :
 *      Write an internal SOC register (8-, 16-, 32-, 48- and 64-bit register).
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      addr   :   Address that combine with Page number and register address.
 *      data     :  (IN)Value here to store.
 *      len     :   number of bytes to be read.
 *
 *  Return :
 *      SOC_E_NONE      :   success
 *      SOC_E_PARAM    :   parameter error
 *
 *  Note :
 *      
 *
 */
int
drv_reg_write(int unit, uint32 addr, void *data, int len)
{
    int         rv = SOC_E_NONE;
#ifdef BROADCOM_DEBUG
    uint32      i;
    int         reg_len = 0, reg_numels;
    uint32      reg_offset;
    uint32      offset;
#else
#ifdef BE_HOST
    uint32  i;
#endif
#endif
    uint64      data_rw;
    uint8           *data8_ptr;
#ifdef BE_HOST
    uint8       tmp;
#endif

    COMPILER_64_ZERO(data_rw);

    LOG_VERBOSE(BSL_LS_SOC_REG,
                (BSL_META_U(unit,
                            "drv_reg_write: unit = %d, addr = %x, length = %d\n"),
                 unit, addr, len));
#ifdef BROADCOM_DEBUG        
    _drv_reg_page_check(unit, addr, &offset);
    /* address checking */
    for (i = 0; i < NUM_SOC_ROBO_REG; i++) {
         if (&SOC_REG_INFO(unit,i) == NULL) {            
            continue;
        }
        reg_numels = SOC_REG_INFO(unit, i).numels;
        reg_offset = SOC_REG_INFO(unit, i).offset;
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, i);
        if ((offset == reg_offset) ||((offset > reg_offset) &&
                 (offset < (reg_offset + reg_numels*reg_len)))) {
                break;
        }
    }    
    
    if ((i >= NUM_SOC_ROBO_REG) || (len != reg_len)) {
        if (!LOG_CHECK(BSL_LS_SOC_REG | BSL_INFO)){
            return SOC_E_PARAM;
        }
    }
#endif

    /* endian translation */
    data8_ptr = (uint8 *)&data_rw;
    if (len > 4) {
        sal_memcpy(data8_ptr, data, 8);
        LOG_VERBOSE(BSL_LS_SOC_REG,
                    (BSL_META_U(unit,
                                "len > 4 ")));
    } else {
        sal_memcpy(data8_ptr, data, 4);
        LOG_VERBOSE(BSL_LS_SOC_REG,
                    (BSL_META_U(unit,
                                "len <= 4 ")));
    }
#ifdef BE_HOST
    if (len > 4) {
        for (i = 0; i < 4; i++) {
            tmp = data8_ptr[i];
            data8_ptr[i] = data8_ptr[7-i];
            data8_ptr[7-i] = tmp;
        }
    } else {
        for (i = 0; i < 2; i++) {
            tmp = data8_ptr[i];
            data8_ptr[i] = data8_ptr[3-i];
            data8_ptr[3-i] = tmp;
        }
    }
#endif

    SPI_LOCK;

    rv = soc_spi_write(unit, addr, (uint8 *)&data_rw, len);
    if (SOC_FAILURE(rv)) {
        LOG_INFO(BSL_LS_SOC_REG,
                 (BSL_META_U(unit,
                             "drv_reg_write: error reading unit=0x%x addr=0x%x\n"), 
                  unit, addr));
        soc_event_generate(unit, SOC_SWITCH_EVENT_ACCESS_ERROR, 
                addr, (uint32)len, rv);
    }
#ifdef BROADCOM_DEBUG 
    LOG_INFO(BSL_LS_SOC_REG,
             (BSL_META_U(unit,
                         "drv_reg_write: offset = 0x%04x data = 0x"), offset));
    for (i = 0; i < len; i++) {
        LOG_INFO(BSL_LS_SOC_REG,
                 (BSL_META_U(unit,
                             "%02x"), data8_ptr[len-i-1]));
    } 
    LOG_INFO(BSL_LS_SOC_REG,
             (BSL_META_U(unit,
                         "\n")));
#endif

    SPI_UNLOCK;

    return rv;
}

/*
 *  Function : drv_reg_length_get
 *
 *  Purpose :
 *      Get the length of the specific register.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      reg   :   register index.
 *
 *  Return :
 *      register length (bytes)
 *
 *  Note :
 *      
 *
 */
int 
drv_reg_length_get(int unit, uint32 reg)
{
    uint32      mask;

    LOG_VERBOSE(BSL_LS_SOC_REG,
                (BSL_META_U(unit,
                            "drv_reg_length_get: unit = %d, reg index= 0x%x\n"),
                 unit, reg));
    if (SOC_REG_IS_COUNTER(unit, reg)) {
        if (SOC_REG_IS_64(unit, reg)) {
            return 8;
        } else {
            return 4;
        }
    } else {
        if (SOC_REG_IS_64(unit, reg)) {
            mask = SOC_REG_INFO(unit, reg).rst_mask_hi;
            if (mask & 0xFF000000) {
                return 8;
            } else if (mask & 0x00FF0000) {
                return 7;
            } else if (mask & 0x0000FF00) {
                return 6;
            } else {
                return 5;
            }
        } else if (SOC_REG_IS_32(unit, reg)) {
            mask = SOC_REG_INFO(unit, reg).rst_mask_lo;
            if (mask & 0xFF000000) {
                return 4;
            } else {
                return 3;
            }
        } else if (SOC_REG_IS_16(unit, reg)) {
            return 2;
        } else if (SOC_REG_IS_8(unit, reg)){
            return 1;
        } else {
            return 0;
        }
    }
}

/*
 *  Function : drv_reg_field_get
 *
 *  Purpose :
 *      Extract the value of a field from a 8-, 16-, and 32-bit register value.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      reg   :   register index.
 *      regbuf  :   data pointer of register's current value.
 *      field   :   field index.
 *      fldbuf  :   data pointer of field value
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int
drv_reg_field_get(int unit, uint32 reg, uint32 *regbuf, uint32 field, 
    uint32 *fldbuf)
{
    soc_field_info_t    *finfop;
    int         i, wp, bp, len;
#ifdef BE_HOST
    uint32      val32;
#endif

    LOG_VERBOSE(BSL_LS_SOC_REG,
                (BSL_META_U(unit,
                            "drv_reg_field_get: unit = %d, reg index= 0x%x, field index = %x\n"),
                 unit, reg, field));
    assert(SOC_REG_IS_VALID(unit, (int)reg));

    SOC_FIND_FIELD(field,
        SOC_REG_INFO(unit, reg).fields,
        SOC_REG_INFO(unit, reg).nFields,
        finfop);
    assert(finfop);
#ifdef BE_HOST
    if (SOC_REG_IS_64(unit, reg)) {
         val32 = regbuf[0];
         regbuf[0] = regbuf[1];
         regbuf[1] = val32;
    }
    if (finfop->len > 32) {
        val32 = fldbuf[0];
        fldbuf[0] = fldbuf[1];
        fldbuf[1] = val32;
    }
#endif
    bp = finfop->bp;

    wp = bp / 32;
    bp = bp & (32 - 1);
    i = 0;

    for (len = finfop->len; len > 0; len -= 32) {
        if (bp) {
            fldbuf[i] = regbuf[wp++] >> bp & ((1 << (32 - bp)) - 1);
            fldbuf[i] |= regbuf[wp] << (32 - bp);
        } else {
            fldbuf[i] = regbuf[wp++];
        }
        if (len < 32) {
            fldbuf[i] &= ((1 << len) - 1);
        }
        i++;
    }
#ifdef BE_HOST
    if (SOC_REG_IS_64(unit, reg)) {
        val32 = regbuf[0];
        regbuf[0] = regbuf[1];
        regbuf[1] = val32;
    }
    if (finfop->len > 32) {
        val32 = fldbuf[0];
        fldbuf[0] = fldbuf[1];
        fldbuf[1] = val32;
    }
#endif
    /* Port number should subtract 24 for BCM53242 */
    if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        if((field == INDEX(PORT_IDf)) ||
        (field == INDEX(SNAPSHOT_PORTf)) || (field == INDEX(EGRESS_Pf)) ||
        (field == INDEX(ARL_PIDf)) || (field == INDEX(ARLA_SRCH_RSLT_PRIDf)) ||
        (field == INDEX(DIG_FLOWCON_PROTf)) || (field == INDEX(NEW_FWD_OBf)) ||
        (field == INDEX(NEW_FWD_IBf)) || (field == INDEX(PORT_NUM_Rf)) ||
        (field == INDEX(RCM_PORTf))) {
            *fldbuf -= 24;
        }
    }
    return SOC_E_NONE;
}


/*
 *  Function : drv_reg_field_get
 *
 *  Purpose :
 *      Extract the value of a field from a 8-, 16-, and 32-bit register value.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      reg   :   register index.
 *      regbuf  :   data pointer of register's current value.
 *      field   :   field index.
 *      fldbuf  :   data pointer of field value
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int
drv_reg_field_set(int unit, uint32 reg, uint32 *regbuf, uint32 field, 
    uint32 *fldbuf)
{
    soc_field_info_t    *finfop;
    uint32              mask;
    int         i, wp, bp, len;
#ifdef BE_HOST
    uint32      val32;
#endif

    LOG_VERBOSE(BSL_LS_SOC_REG,
                (BSL_META_U(unit,
                            "drv_reg_field_set: unit = %d, reg index= 0x%x, field index = %x\n"),
                 unit, reg, field));
    assert(SOC_REG_IS_VALID(unit, (int)reg));

    SOC_FIND_FIELD(field,
    SOC_REG_INFO(unit, reg).fields,
    SOC_REG_INFO(unit, reg).nFields,
    finfop);
    assert(finfop);
#ifdef BE_HOST
    if (SOC_REG_IS_64(unit, reg)) {
        val32 = regbuf[0];
        regbuf[0] = regbuf[1];
        regbuf[1] = val32;
    }
    if (finfop->len > 32) {
        val32 = fldbuf[0];
        fldbuf[0] = fldbuf[1];
        fldbuf[1] = val32;
    }
#endif
    /* Port number should subtract 24 for BCM53242 */
    if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        if ((field == INDEX(PORT_IDf)) ||
        (field == INDEX(SNAPSHOT_PORTf)) || (field == INDEX(EGRESS_Pf)) ||
        (field == INDEX(ARL_PIDf)) || (field == INDEX(ARLA_SRCH_RSLT_PRIDf)) ||
        (field == INDEX(DIG_FLOWCON_PROTf)) || (field == INDEX(NEW_FWD_OBf)) ||
        (field == INDEX(NEW_FWD_IBf)) || (field == INDEX(PORT_NUM_Rf)) ||
        (field == INDEX(RCM_PORTf))) {
            *fldbuf += 24;
        }
    }
    bp = finfop->bp;

    wp = bp / 32;
    bp = bp & (32 - 1);
    i = 0;

    for (len = finfop->len; len > 0; len -= 32) {
        if (bp) {
            if (len < 32) {
                mask = (1 << len) - 1;
                assert(!VALUE_TOO_BIG_FOR_FIELD);
            } else {
                mask = -1;
            }
            regbuf[wp] &= ~(mask << bp);
            regbuf[wp++] |= fldbuf[i] << bp;
            regbuf[wp] &= ~(mask >> (32 - bp));
            regbuf[wp] |=
            fldbuf[i] >> (32 - bp) & ((1 << bp) - 1);
        } else {
            if (len < 32) {
                mask = (1 << len) - 1;
                assert(!VALUE_TOO_BIG_FOR_FIELD);
                regbuf[wp] &= ~mask;
                regbuf[wp++] |= fldbuf[i] << bp;
            } else {
                regbuf[wp++] = fldbuf[i];
            }
        }
        i++;
    } 
#ifdef BE_HOST
    if (SOC_REG_IS_64(unit, reg)) {
        val32 = regbuf[0];
        regbuf[0] = regbuf[1];
        regbuf[1] = val32;
    }
    if (finfop->len > 32) {
        val32 = fldbuf[0];
        fldbuf[0] = fldbuf[1];
        fldbuf[1] = val32;
    }
#endif
    /* Port number should subtract 24 for BCM53242 */
    if (SOC_IS_ROBO53242(unit)||SOC_IS_ROBO53262(unit)) {
        if ((field == INDEX(PORT_IDf)) ||
        (field == INDEX(SNAPSHOT_PORTf)) || (field == INDEX(EGRESS_Pf)) ||
        (field == INDEX(ARL_PIDf)) || (field == INDEX(ARLA_SRCH_RSLT_PRIDf)) ||
        (field == INDEX(DIG_FLOWCON_PROTf)) || (field == INDEX(NEW_FWD_OBf)) ||
        (field == INDEX(NEW_FWD_IBf)) || (field == INDEX(PORT_NUM_Rf)) ||
        (field == INDEX(RCM_PORTf))) {
            *fldbuf -= 24;
        }
    }
    return SOC_E_NONE;
}

/*
 *  Function : drv_reg_addr
 *
 *  Purpose :
 *      calculate the address of a register.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      reg   :   register index.
 *      port  :   port index.
 *      index   :   index value.
 *
 *  Return :
 *      register address
 *
 *  Note :
 *      
 *
 */
uint32
drv_reg_addr(int unit, uint32 reg, int port, int index)
{
    uint32              base = 0;       /* base address from reg_info */
    soc_regtype_t regtype;
    int nByte;
    uint32 offset;
    soc_block_types_t     regblktype;
    int numfe, i;


    assert(SOC_REG_IS_VALID(unit, (int)reg));

    regblktype = SOC_REG_INFO(unit, reg).block;

    regtype = SOC_REG_INFO(unit, reg).regtype;
    nByte = (DRV_SERVICES(unit)->reg_length_get)(unit, reg);
    offset = SOC_REG_INFO(unit, reg).offset;

    if (port < 0) {
        port = 0;
    }

    if (index < 0) {
        index = 0;
    }

    numfe = 0;
    for(i = 0; SOC_BLOCK_INFO(unit, i).type >= 0; i++) {
      if (SOC_BLOCK_INFO(unit,i).type == SOC_BLK_EPIC) {
        numfe = 1;
        break;
      }
    }

    switch (regtype) {
        case soc_genreg:
            /* general case */
            if (SOC_BLOCK_IS(regblktype, SOC_BLK_SYS)) {
                /* [SYS] */
                base = (0x0000FFFF & (offset + index * nByte));        
                break;
            } else if (SOC_BLOCK_IN_LIST(regblktype, SOC_BLK_PORT)) {                

                if (SOC_BLOCK_IS(regblktype, SOC_BLK_EPIC)) {
                    /* [EPIC0] */
                            base = (0x0000FFFF & (offset + 
                                port * nByte));
                    break;
                }

                if (SOC_BLOCK_IS(regblktype, SOC_BLK_GPIC)) {
                    /* [GPIC0]*/
                    if (numfe){
                        base = (0x0000FFFF & (offset + 
                            (port - CMIC_PORT(unit) -1) * nByte));
                    } else {
                        base = (0x0000FFFF & (offset + 
                                port * nByte));
                    }
                    
                    break;
                }
                
                if(SOC_BLOCK_IN_LIST(regblktype, SOC_BLK_CPIC)){
                    if (regblktype[1] == SOC_BLOCK_TYPE_INVALID){
                        /* [CPIC] */
                        assert (IS_CPU_PORT(unit,port)) ;
                        base = (0x0000FFFF & (offset));
                        break;
                    }
                    if (regblktype[1] == SOC_BLK_EPIC){
                        /* [EPIC0, CPIC, GPIC0] or [EPIC0,CPIC]*/                    
                        base = (0x0000FFFF & (offset + 
                                port * nByte));                     
                        break;
                    }
                    if (regblktype[1] == SOC_BLK_GPIC){
                        /* [GPIC0, CPIC] */
                        if (numfe){
                            base = (0x0000FFFF & (offset + 
                                (port - CMIC_PORT(unit) -1) * nByte));
                        } else {
                            base = (0x0000FFFF & (offset + 
                                    port * nByte));
                        }
                        break;
                    }
                }                
            }
            base = (0x0000FFFF & (offset));
            break;
        case soc_spi_reg:
            if (port > 0) {
                assert(0);
            }
            break;
        case soc_phy_reg:
        case soc_portreg:
            if (index > 0) {
                assert(0);
            }            
            if (numfe){
                if (SOC_BLOCK_IS(regblktype, SOC_BLK_CPIC) && 
                (regblktype[1] == SOC_BLK_GPIC)){
                    /* CPIC, GPIC0*/
                    port = port - CMIC_PORT(unit);
                } else if (SOC_BLOCK_IS(regblktype, SOC_BLK_GPIC)){
                    /* GPIC0 */
                    port = port - CMIC_PORT(unit) - 1 ;
                }
            }

            base = ((0x0000FF00 & (port << 8)) + (0x0000FFFF & offset));
            break;
        default:
            assert(0);
        break;
    }
    LOG_VERBOSE(BSL_LS_SOC_REG,
                (BSL_META_U(unit,
                            "drv_reg_addr: unit = %d, reg index= 0x%x, \
                            port = %d, index = %d, addr = 0x%x\n"),
                 unit, reg, port, index, base));
    return base;
}

