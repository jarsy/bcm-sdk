/*
 * $Id: miim.c,v 1.56 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * MII Management (MDIO) bus access routines for reading and writing PHY
 * registers.
 *
 * BCM5690 has an internal MDIO bus in addition to the external one.
 * The internal bus is accessed by setting bit 7 in the PHY ID.
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <sal/core/boot.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cm.h>

#include <soc/robo/mcm/driver.h>
#include <soc/error.h>
#include <soc/cmic.h>
#include <soc/drv.h>
#include <soc/spi.h>
#include <soc/phyctrl.h>

#define PHY_MII_BASEPAGE_UNKNOW         0xFF    /* unknow chip */
#define PHY_MII_BASEPAGE_INT            0x10    /* internal phy: bcm5324 bcm5498 */
#define PHY_MII_BASEPAGE_EXT            0x80    /* external phy: bcm5396 bcm5489 */

#define PHY_HARRIER_MII_BASEPAGE_FE_0_TO_23  0xa0 /* BCM53242 port 0~23. Internal PHY */
#define PHY_HARRIER_MII_BASEPAGE_IMP  0xd8 /* BCM53242 port IMP. External PHY */
#define PHY_HARRIER_MII_BASEPAGE_GE  0xd9 /* BCM53242 port 24~28. External PHY */
#define PHY_HARRIER_MII_BASEPAGE_INTER_SERDES 0xb8 /* BCM53242 port 49 ~52 Sedes MII*/

#define PHY_VULCAN_MII_BASEPAGE_EXT_WAN  0x85 /* BCM53115 port5 External PHY */

#define PHY_TB_MII_BASEPAGE_FE  0xA0 /* Thunderbolt Internal PHY(FE) */
#define PHY_TB_MII_BASEPAGE_INT_SERDES  0xB9 /* Thunderbolt Internal SerDes */
#define PHY_TB_MII_BASEPAGE_EXT_GE  0xD8 /* Thunderbolt External PHY(GE) */

#define PHY_VO_MII_BASEPAGE_EXT_FE  0xC0 /* Voyager External S3MII PHY(FE) */
#define PHY_VO_MII_BASEPAGE_INT_SERDES  0xBB /* Voyager Internal SerDes (ge2(port27)/ge3(port28))*/


#define PHY_MIIREG_LENGTH_ROBO      2           /* 2 bytes */
                                   
#define UNKNOW_ROBO_PHY_MIIADDR(addr)      \
        (((addr) >> 8) == PHY_MII_BASEPAGE_UNKNOW)
                            
uint32
_soc_robo_serdes_phy_miiaddr_get(int unit, soc_port_t port, 
                                  uint16 phy_reg_addr)
{
    uint32 page = 0, offset= 0;
#ifdef BCM_NORTHSTARPLUS_SUPPORT
	uint16 phy_addr;
#endif /* BCM_NORTHSTARPLUS_SUPPORT */

    offset = (uint32)(phy_reg_addr * PHY_MIIREG_LENGTH_ROBO);

    if (IS_FE_PORT(unit, port)) {
        /* BCM53101 build-in serdes on port5 only */
        if (SOC_IS_LOTUS(unit) && (port == 5)) {
            page = (port + PHY_MII_BASEPAGE_INT) << 8;
            return (page | offset);
        }
    }

    if (!IS_GE_PORT(unit, port)) {
        return -1;
    }
    if (SOC_IS_HARRIER(unit)){
        page = (port - NUM_FE_PORT(unit) ) + 
            PHY_HARRIER_MII_BASEPAGE_INTER_SERDES;
        page <<= 8;
    } else if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
               SOC_IS_POLAR(unit)) {
        /* bcm53115 build-in serdes on port5 only. 
         *  - reture unknow page address for the internal SerDes miim reg 
         *      read/write access. (the external phy miim read/write will be 
         *      executed as a proper PHY miim read/write those known SerDes 
         *      ports)
         */
        if (port == 5){
    	    page = (port + PHY_MII_BASEPAGE_INT) << 8;
        } else {
    	    page = PHY_MII_BASEPAGE_UNKNOW << 8;
        }
    } else if (SOC_IS_TBX(unit)){
        if (IS_CPU_PORT(unit, port) || IS_GMII_PORT(unit, port)){
    	    page = PHY_MII_BASEPAGE_UNKNOW << 8;
        } else {
            if (SOC_IS_TB(unit)){            
                page = ((port - 24 - 1) + 
    	            PHY_TB_MII_BASEPAGE_INT_SERDES) << 8;
            } else if (SOC_IS_VO(unit)){            
                /* Voyager port 27/28 with SGMII interface*/
                page = ((port - 27) + 
    	            PHY_VO_MII_BASEPAGE_INT_SERDES) << 8;
            } else {
                page = PHY_MII_BASEPAGE_UNKNOW << 8;            
            }
        }
    } else if (SOC_IS_DINO(unit)) {
        page = (port + PHY_MII_BASEPAGE_INT) << 8;
    } else if (SOC_IS_NORTHSTARPLUS(unit)) {
#ifdef BCM_NORTHSTARPLUS_SUPPORT
    	/* Inetrnal SGMII */
		phy_addr = PHY_ADDR(unit, port);
		phy_addr &= ~(PHY_ADDR_ROBO_INT_SERDES);
        page = (0x10 + (phy_addr - 6)) << 8;
#endif /* BCM_NORTHSTARPLUS_SUPPORT */		
#ifdef BCM_STARFIGHTER3_SUPPORT
    }  else if (SOC_IS_STARFIGHTER3(unit)) {
        if (port == 5) {
            /* Starfighter3 serdes uses page 0x14. MDIO address: 0x4 */
            page = (0x4 + PHY_MII_BASEPAGE_INT) << 8;
        }
#endif /* BCM_STARFIGHTER3_SUPPORT */
    }
    return (page | offset);
}    
uint32
_soc_robo_phy_miiaddr_get(int unit, soc_port_t port, 
                                  uint16 phy_reg_addr)
{
    uint32 page = 0, offset= 0;
#if defined(BCM_TB_SUPPORT)
    char *s;
#endif /* BCM_TB_SUPPORT */

#ifdef BCM_POLAR_SUPPORT
    uint16  dev_id = 0;
    uint8   rev_id = 0;    
#endif

#ifdef BCM_NORTHSTAR_SUPPORT
    uint16 ext_phy_id;
#endif

#if defined(BCM_NORTHSTARPLUS_SUPPORT)
    uint16 phy_addr;
#endif /* BCM_NORTHSTARPLUS_SUPPORT */


    offset = (uint32)(phy_reg_addr * PHY_MIIREG_LENGTH_ROBO);

    if (SOC_IS_HARRIER(unit)) {
        if (port < 24) { /* Port 0~23, internal phy */
            page = (port + PHY_HARRIER_MII_BASEPAGE_FE_0_TO_23) << 8;
        } else if (port == 24) { /* IMP Port, external phy */
            page = ((port - 24) + PHY_HARRIER_MII_BASEPAGE_IMP) << 8;
        } else if (port < 29) { /* Port 25~28, external ge phy */
            page = ((port - 25) + PHY_HARRIER_MII_BASEPAGE_GE) << 8;
        } else { /* Invalid port */
            page = PHY_MII_BASEPAGE_UNKNOW << 8;
        }    
#ifdef BCM_POLAR_SUPPORT
    }else if (SOC_IS_POLAR(unit)){
            soc_cm_get_id(unit, &dev_id, &rev_id);            
            if (port == CMIC_PORT(unit) || (port == 5)) {
                page = (port + PHY_MII_BASEPAGE_EXT) << 8;
            } else if ((port == 4) && (dev_id == BCM89500_DEVICE_ID)) {
                page = (port + PHY_MII_BASEPAGE_EXT) << 8;
            } else {
                page = (port + PHY_MII_BASEPAGE_INT) << 8;
            }
#endif /* BCM_POLAR_SUPPORT */
#ifdef BCM_NORTHSTAR_SUPPORT
    } else if (SOC_IS_NORTHSTAR(unit)) {
        if (port < 5) { /* Port 0~4, internal phy */
            page = (port + PHY_MII_BASEPAGE_INT) << 8;
        } else if (port == 5) {
            /* For Northstar, access of external PHY of port5 (if exist) is not through
             * switch register table. Instead it is through CPU's ChipcommonB MDIO interface.
             * Change the return address of port5 here, so it can be identified in CPU CCB MDIO
             * driver of linux-user-bde.
             */
            soc_phy_cfg_addr_get(unit, port, 0, &ext_phy_id);
            PHY_ADDR_ROBO_CPU_MDIOBUS_CLR(ext_phy_id);
            page = SOC_EXTERNAL_PHY_BUS_CPUMDIO | ext_phy_id;
            offset = phy_reg_addr;
        } else { /* No external phy for port 7, 8 */
            page = PHY_MII_BASEPAGE_UNKNOW << 8;
        }
#endif /* BCM_NORTHSTAR_SUPPORT */
#if defined(BCM_NORTHSTARPLUS_SUPPORT)
        } else if (SOC_IS_NORTHSTARPLUS(unit)) {
            /* tranfer to phy address */
            phy_addr = PHY_ADDR(unit, port);
            
            if ((phy_addr > 5) && (phy_addr < 11)) {
                /* Internal GPHY */
                /* phy address 6 ~ 10 */
                page = (0x10 + (phy_addr - 6)) << 8;
            } else if ((phy_addr == 12) || (phy_addr== 13)) {
                /* Inetrnal SGMII */
                page = (0x10 + (phy_addr - 6)) << 8;
            } else {
                /* External PHY address */
                page = (port + PHY_MII_BASEPAGE_EXT) << 8;
            }
#endif /* BCM_NORTHSTARPLUS_SUPPORT */
    } else if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
        if (port == CMIC_PORT(unit)) {
            page = (port + PHY_MII_BASEPAGE_EXT) << 8;
        } else {
            /* bcm53115 port5 within built-in SerDes and allowed the external 
             *  phy to connect through SGMII with this SerDes.
             */
            if ((SOC_IS_VULCAN(unit) || SOC_IS_LOTUS(unit) ||
                SOC_IS_STARFIGHTER(unit)) &&
                (port == 5)){
                page = (PHY_VULCAN_MII_BASEPAGE_EXT_WAN) << 8;
            } else if (SOC_IS_BLACKBIRD2_BOND_V(unit) && (port == 7)) {
                page = (port + PHY_MII_BASEPAGE_EXT) << 8;
#ifdef BCM_STARFIGHTER3_SUPPORT
            } else if (SOC_IS_STARFIGHTER3(unit) && (port == 5)) {
                if (soc_property_port_get(unit, port,
                        spn_PHY_SYS_INTERFACE, 0) == SOC_PORT_IF_SGMII) {
                   page = (4 + PHY_MII_BASEPAGE_INT) << 8;
               } else {
                   page = (PHY_VULCAN_MII_BASEPAGE_EXT_WAN) << 8;
               }
#endif /* BCM_STARFIGHTER3_SUPPORT */
            } else {
                page = (port + PHY_MII_BASEPAGE_INT) << 8;
            }
        }
#ifdef BCM_TB_SUPPORT
    } else if (SOC_IS_TBX(unit)){
        s = soc_property_get_str(unit, "board_name");
        if( (s != NULL) && (sal_strcmp(s, "bcm53280_fpga") == 0)) {
            if (port < 4) {
                page = (PHY_TB_MII_BASEPAGE_FE + port) << 8;
            } else if (port < 29){
                page = (PHY_TB_MII_BASEPAGE_EXT_GE + 
                        (port - 24 )) << 8;
            }
        } else {
            if (port < 24) {
                page = (PHY_TB_MII_BASEPAGE_FE + port) << 8;
                if (SOC_IS_VO(unit) && 
                    (SOC_ROBO_CONTROL(unit)->chip_bonding == 
                    SOC_ROBO_BOND_PHY_S3MII)) {
                    /* bcm53606 Voyager with S3MII bonding */
                    page = (PHY_VO_MII_BASEPAGE_EXT_FE + port) << 8;
                }
            } else if (port < 29){
                if (IS_CPU_PORT(unit, port)){
                    page = PHY_TB_MII_BASEPAGE_EXT_GE << 8;
                } else if (IS_GE_PORT(unit, port)){
                    page = (PHY_TB_MII_BASEPAGE_EXT_GE + 
                            (port - 24 )) << 8;
                } else {
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s,%d,Unexpected port property!"),
                              FUNCTION_NAME(), __LINE__));
                    page = (PHY_TB_MII_BASEPAGE_INT_SERDES + 
                            (port - 24 )) << 8;
                }
            }
        }
#endif
    } else if (SOC_IS_DINO(unit)) {
        page = (port + PHY_MII_BASEPAGE_EXT) << 8;
    } else {
        page = PHY_MII_BASEPAGE_UNKNOW << 8;
    }
                                   
    return (page | offset);
}

/*
 * Function:
 *	soc_miim_write
 * Purpose:
 *	Write a value to a MIIM register.
 * Parameters:
 *	unit - StrataSwitch Unit #.
 *	phy_id - Phy ID to write (MIIM address)
 *	phy_reg_addr - PHY register to write
 *	phy_wr_data - Data to write.
 * Returns:
 *	SOC_E_XXX
 * Notes:
 * 	Temporarily disables auto link scan if it was enabled.  The MIIM
 * 	registers are locked during the operation to prevent multiple
 * 	tasks from trying to access PHY registers simultaneously.
 */

int
soc_robo_miim_write(int unit, uint32 phy_id,
                   uint32 phy_reg_addr, uint16 phy_wr_data)

{
    int	rv = SOC_E_NONE;
    int	len = 2; /* 16-bit register */
    uint32 addr;
    soc_port_t port;

    assert(!sal_int_context());

    LOG_INFO(BSL_LS_SOC_MIIM,
             (BSL_META_U(unit,
                         "soc_robo_miim_write: id=0x%02x addr=0x%02x data=0x%04x\n"),
              phy_id, phy_reg_addr, phy_wr_data));

#if defined(BCM_53101)
    if (SOC_IS_LOTUS(unit) && 
        SOC_MAC_LOW_POWER_ENABLED(unit)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Don't allow to write the PHY addr 0x%02x.\n"),
                   phy_reg_addr));
        return SOC_E_UNAVAIL;
    }
#endif /* BCM_53101 */

    /* PHY_ADDR_ROBO_INT_SERDES is a special flag for indicating the GE port 
     *  is a ROBO internal SerDes (check phyident.c)
     */
    if (PHY_ADDR_ROBO_INT_FLAG_CHK(phy_id)) {
        PHY_ADDR_ROBO_INT_FLAG_CLR(phy_id);
        
        rv = soc_robo_miim_int_write(unit, phy_id, 
                phy_reg_addr, phy_wr_data);
        return rv;
    }

    if (PHY_ADDR_ROBO_CPU_MDIOBUS_CHK(phy_id)) {
        PHY_ADDR_ROBO_CPU_MDIOBUS_CLR(phy_id);

        addr = SOC_EXTERNAL_PHY_BUS_CPUMDIO | (phy_id << 8);
        addr |= phy_reg_addr;
        SPI_LOCK;
        rv = soc_spi_write(unit, addr, (uint8*)&phy_wr_data, len);
        SPI_UNLOCK;
        return rv;
    } 

    port = soc_phy_addr_to_port(unit, phy_id);

#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_POLAR(unit) && !IS_GMII_PORT(unit, port)){
        return rv;
    }
#endif /* BCM_POLAR_SUPPORT */
#if defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
    if ((SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) 
		&& (port == 7)) {
        return rv;
    }
#endif /* BCM_NORTHSTAR_SUPPORT || NS+ */

    addr = _soc_robo_phy_miiaddr_get(unit, port, phy_reg_addr);

    if (UNKNOW_ROBO_PHY_MIIADDR(addr)){
        /* PHY_MII_BASEPAGE not defined for current robo_chip */
        return SOC_E_RESOURCE;
    }

#ifdef BE_HOST
    phy_wr_data = (phy_wr_data>>8) | (phy_wr_data<<8);
#endif

    SPI_LOCK;

    rv = soc_spi_write(unit, addr, (uint8*)&phy_wr_data, len);

    SPI_UNLOCK;

    return rv;
}



/*
 * Function:
 *	soc_miim_read
 * Purpose:
 *	Read a value from an MII register.
 * Parameters:
 *	unit - StrataSwitch Unit #.
 *	phy_id - Phy ID to write (MIIM address)
 *	phy_reg_addr - PHY register to write
 *	phy_rd_data - 16bit data to write into
 * Returns:
 *	SOC_E_XXX
 * Notes:
 * 	Temporarily disables auto link scan if it was enabled.  The MIIM
 * 	registers are locked during the operation to prevent multiple
 * 	tasks from trying to access PHY registers simultaneously.
 */

int
soc_robo_miim_read(int unit, uint32 phy_id,
                  uint32 phy_reg_addr, uint16 *phy_rd_data)
{
    int	rv = SOC_E_NONE;
    int	len = 2; /* 16-bit register */
    uint32 addr;
    soc_port_t port;

    assert(!sal_int_context());
    assert(phy_rd_data);

    LOG_INFO(BSL_LS_SOC_MIIM,
             (BSL_META_U(unit,
                         "drv_miim_read: id=0x%02x addr=0x%02x\n"),
              phy_id, phy_reg_addr));

#if defined(BCM_53101)
    if (SOC_IS_LOTUS(unit) && 
        SOC_MAC_LOW_POWER_ENABLED(unit)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Don't allow to read the PHY addr 0x%02x.\n"),
                   phy_reg_addr));
        return SOC_E_UNAVAIL;
    }
#endif /* BCM_53101 */    

    if (PHY_ADDR_ROBO_INT_FLAG_CHK(phy_id)) {
        PHY_ADDR_ROBO_INT_FLAG_CLR(phy_id);
        
        rv = soc_robo_miim_int_read(unit, phy_id, 
                phy_reg_addr, phy_rd_data);
        return rv;
    }

    if (PHY_ADDR_ROBO_CPU_MDIOBUS_CHK(phy_id)) {
        PHY_ADDR_ROBO_CPU_MDIOBUS_CLR(phy_id);

        addr = SOC_EXTERNAL_PHY_BUS_CPUMDIO | (phy_id << 8);
        addr |= phy_reg_addr;
        SPI_LOCK;
        rv = soc_spi_read(unit, addr, (uint8*)phy_rd_data, len);
        SPI_UNLOCK;
        return rv;
    } 

    port = soc_phy_addr_to_port(unit, phy_id);

#ifdef BCM_POLAR_SUPPORT
        if (SOC_IS_POLAR(unit) && !IS_GMII_PORT(unit, port)){
            *phy_rd_data = 0;
            return rv;
        }
#endif /* BCM_POLAR_SUPPORT */
#if defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
    if ((SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) 
		&& (port == 7)) {
        *phy_rd_data = 0;
        return rv;
    }
#endif /* BCM_NORTHSTAR_SUPPORT || NS+ */

    addr = _soc_robo_phy_miiaddr_get(unit, port, phy_reg_addr);
    
    if (UNKNOW_ROBO_PHY_MIIADDR(addr)){
        /* PHY_MII_BASEPAGE not defined for current robo_chip */
        return SOC_E_RESOURCE;
    }

    SPI_LOCK;

    rv = soc_spi_read(unit, addr, (uint8*)phy_rd_data, len);

    SPI_UNLOCK;
#ifdef BE_HOST
    *phy_rd_data = (*phy_rd_data>>8) | (*phy_rd_data<<8);
#endif

    LOG_INFO(BSL_LS_SOC_MIIM,
             (BSL_META_U(unit,
                         "drv_miim_read: spi_addr=0x%04x,read data=0x%04x\n"), 
              addr, *phy_rd_data));

    return rv;
}

/*
 * Function:
 *      soc_robo_miim_int_write
 * Purpose:
 *      New interface to write a value to a MIIM register.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      phy_id - Phy ID to write (MIIM address)
 *      phy_reg_addr - PHY register to write
 *      phy_wr_data - Data to write.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 * Added to have the same interface for PHY register access among
 * ESW, ROBO, SBX
 */
int
soc_robo_miim_int_write(int unit, uint32 phy_id,
                   uint32 phy_reg_addr, uint16 phy_wr_data)
{
    int	rv = SOC_E_NONE;
    int	len = 2; /* 16-bit register */
    uint32 addr;
    soc_port_t port;

    assert(!sal_int_context());

    LOG_INFO(BSL_LS_SOC_MIIM,
             (BSL_META_U(unit,
                         "soc_robo_miim_int_write: id=0x%02x addr=0x%02x data=0x%04x\n"),
              phy_id, phy_reg_addr, phy_wr_data));

    port = soc_phy_addr_to_port(unit, phy_id);
    
    /* soc_feature_dodeca_serdes is a system-wide SerDes feature 
     *  definition on all GE port in this chip, but is not suitable for 
     *  some ROBO device. Like bcm53115, 6 GE ports included, but the last 
     *  GE port is a built-in SerDes. Starfighter3 uses viper core for SGMII serdes.
     */
    if (!SOC_IS_STARFIGHTER3(unit)) {
        if (!IS_ROBO_SPECIFIC_INT_SERDES(unit, port)){
            if (!soc_feature(unit, soc_feature_dodeca_serdes)) {
                return rv;
            }
        }
    }

    if (!IS_GE_PORT(unit, port)){
        return rv;
    }

#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_POLAR(unit) && IS_GMII_PORT(unit, port)){
        return rv;
    }
#endif /* BCM_POLAR_SUPPORT */
#ifdef BCM_NORTHSTAR_SUPPORT
    if (SOC_IS_NORTHSTAR(unit) && (port == 7)) {
        return rv;
    }
#endif /* BCM_NORTHSTAR_SUPPORT */

    addr = _soc_robo_serdes_phy_miiaddr_get(unit, port, phy_reg_addr);
    if (UNKNOW_ROBO_PHY_MIIADDR(addr)){
        /* PHY_MII_BASEPAGE not defined for current robo_chip */
        return SOC_E_RESOURCE;
    }

#ifdef BE_HOST
    phy_wr_data = (phy_wr_data>>8) | (phy_wr_data<<8);
#endif

    SPI_LOCK;

    rv = soc_spi_write(unit, addr, (uint8*)&phy_wr_data, len);

    SPI_UNLOCK;

    return rv;

}
/*
 * Function:
 *      soc_robo_miim_int_read
 * Purpose:
 *      New interface to read a value from a MIIM register.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      phy_id - Phy ID to write (MIIM address)
 *      phy_reg_addr - PHY register to write
 *      phy_rd_data - Data read.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 * Added to have the same interface for PHY register access among
 * ESW, ROBO, SBX.
 */
int
soc_robo_miim_int_read(int unit, uint32 phy_id,
                  uint32 phy_reg_addr, uint16 *phy_rd_data)
{
    int	rv = SOC_E_NONE;
    int	len = 2; /* 16-bit register */
    uint32 addr;
    soc_port_t port;

    assert(!sal_int_context());
    assert(phy_rd_data);

    LOG_INFO(BSL_LS_SOC_MIIM,
             (BSL_META_U(unit,
                         "soc_robo_miim_int_read: id=0x%02x addr=0x%02x\n"),
              phy_id, phy_reg_addr));

    port = soc_phy_addr_to_port(unit, phy_id);

    /* soc_feature_dodeca_serdes is a system-wide SerDes feature 
     *  definition on all GE port in this chip, but is not suitable for 
     *  some ROBO device. Like bcm53115, 6 GE ports included, but the last 
     *  GE port is a built-in SerDes. Starfighter3 uses viper serdes.
     */
    if (!SOC_IS_STARFIGHTER3(unit)) {
        if (!IS_ROBO_SPECIFIC_INT_SERDES(unit, port)){
            if (!soc_feature(unit, soc_feature_dodeca_serdes)) {
                *phy_rd_data = 0;
                return rv;
            }
        }
    }

    if (!IS_GE_PORT(unit, port)){
        *phy_rd_data = 0;
        return rv;
    }

#ifdef BCM_POLAR_SUPPORT
    if (SOC_IS_POLAR(unit) && IS_GMII_PORT(unit, port)){
        *phy_rd_data = 0;
        return rv;
    }
#endif /* BCM_POLAR_SUPPORT */
#ifdef BCM_NORTHSTAR_SUPPORT
    if (SOC_IS_NORTHSTAR(unit) && (port == 7)) {
        *phy_rd_data = 0;
        return rv;
    }
#endif /* BCM_NORTHSTAR_SUPPORT */

    addr = _soc_robo_serdes_phy_miiaddr_get(unit, port, phy_reg_addr);

    if (UNKNOW_ROBO_PHY_MIIADDR(addr)){
        /* PHY_MII_BASEPAGE not defined for current robo_chip */
        *phy_rd_data = 0;
        return SOC_E_RESOURCE;
    }

    SPI_LOCK;

    rv = soc_spi_read(unit, addr, (uint8*)phy_rd_data, len);

    SPI_UNLOCK;
#ifdef BE_HOST
    *phy_rd_data = (*phy_rd_data>>8) | (*phy_rd_data<<8);
#endif

    LOG_INFO(BSL_LS_SOC_MIIM,
             (BSL_META_U(unit,
                         "soc_robo_miim_int_read: read data=0x%04x\n"), *phy_rd_data));

    return rv;
}

#ifdef BCM_SWITCHMACSEC_SUPPORT

#define _SOC_ROBO_MACSEC_MIIADDR_GET(_page, _offset)  \
            ((((_page) & 0xFF) << 8) | \
            (((_offset) & 0xFF) * PHY_MIIREG_LENGTH_ROBO))
/*
 * Function:
 *      soc_robo_macsec_miim_read
 * Purpose:
 *      Interface to read a value from a MIIM register page for MACSEC access.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      phy_id - Phy ID to write (MIIM address)
 *      phy_reg_addr - PHY register to write
 *      phy_rd_data - Data read.
 * Returns:
 *      SOC_E_XXX
 *  Note :
 *  1. phy_id in this routine will be directly been referenced as ROBO's 
 *      register page index since this routine is a specific routine for 
 *      MACSEC MDIO access only. SDK user must assign the correct phy_id 
 *      through config.bcm. (it is flexible designn to allow user on 
 *      assigning SPI page for MACSEC MDIO access through config.bcm 
 *      directly.)
 *      
 */
int
soc_robo_macsec_miim_read(int unit, uint32 phy_id, 
                uint32 phy_reg_addr, uint16 *phy_rd_data)
{
    int	rv = SOC_E_NONE;
    int	len = PHY_MIIREG_LENGTH_ROBO; /* 16-bit register */
    uint32 addr;

    assert(!sal_int_context());
    assert(phy_rd_data);

    LOG_INFO(BSL_LS_SOC_MIIM,
             (BSL_META_U(unit,
                         "%s:id=0x%02x addr=0x%02x\n"),
              FUNCTION_NAME(), phy_id, phy_reg_addr));

    /* addr for ROBO is the register page+offset which is specified for 
     * MACSEC MDIO access of a specific port already.
     *  
     * Note :
     *  1. phy_id : this ID is assigned by SDK user thorugh config.bcm
     *      this ID will be used to indicate ROBO's register page for MDIO 
     *      access on MACSEC's LMI/MMI interface.
     */
    addr = _SOC_ROBO_MACSEC_MIIADDR_GET(phy_id, phy_reg_addr);    

    if (UNKNOW_ROBO_PHY_MIIADDR(addr)){
        /* PHY_MII_BASEPAGE not defined for current robo_chip */
        *phy_rd_data = 0;
        return SOC_E_RESOURCE;
    }

    SPI_LOCK;

    rv = soc_spi_read(unit, addr, (uint8*)phy_rd_data, len);

    SPI_UNLOCK;
#ifdef BE_HOST
    *phy_rd_data = (*phy_rd_data>>8) | (*phy_rd_data<<8);
#endif

    LOG_INFO(BSL_LS_SOC_MIIM,
             (BSL_META_U(unit,
                         "%s:read data=0x%04x\n"), 
              FUNCTION_NAME(), *phy_rd_data));

    return rv;
}

/*
 * Function:
 *      soc_robo_macsec_miim_write
 * Purpose:
 *      Interface to write a value from a MIIM register page for MACSEC access.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      phy_id - Phy ID to write (MIIM address)
 *      phy_reg_addr - PHY register to write
 *      phy_rd_data - Data read.
 * Returns:
 *      SOC_E_XXX
 *  Note :
 *  1. phy_id in this routine will be directly been referenced as ROBO's 
 *      register page index since this routine is a specific routine for 
 *      MACSEC MDIO access only. SDK user must assign the correct phy_id 
 *      through config.bcm. (it is flexible designn to allow user on 
 *      assigning SPI page for MACSEC MDIO access through config.bcm 
 *      directly.)*/
int
soc_robo_macsec_miim_write(int unit, uint32 phy_id, 
                uint32 phy_reg_addr, uint16 phy_wr_data)
{
    int	rv = SOC_E_NONE;
    int	len = PHY_MIIREG_LENGTH_ROBO; /* 16-bit register */
    uint32 addr;

    assert(!sal_int_context());

    LOG_INFO(BSL_LS_SOC_MIIM,
             (BSL_META_U(unit,
                         "%s: id=0x%02x addr=0x%02x data=0x%04x\n"),
              FUNCTION_NAME(), phy_id, phy_reg_addr, phy_wr_data));

    /* addr for ROBO is the register page+offset which is specified for 
     * MACSEC MDIO access of a specific port already.
     *  
     * Note :
     *  1. phy_id : this ID is assigned by SDK user thorugh config.bcm
     *      this ID will be used to indicate ROBO's register page for MDIO 
     *      access on MACSEC's LMI/MMI interface.
     */
    addr = _SOC_ROBO_MACSEC_MIIADDR_GET(phy_id, phy_reg_addr);    

    if (UNKNOW_ROBO_PHY_MIIADDR(addr)){
        /* PHY_MII_BASEPAGE not defined for current robo_chip */
        return SOC_E_RESOURCE;
    }

#ifdef BE_HOST
    phy_wr_data = (phy_wr_data>>8) | (phy_wr_data<<8);
#endif

    SPI_LOCK;

    rv = soc_spi_write(unit, addr, (uint8*)&phy_wr_data, len);

    SPI_UNLOCK;

    return rv;
}
#endif /* BCM_SWITCHMACSEC_SUPPORT */

