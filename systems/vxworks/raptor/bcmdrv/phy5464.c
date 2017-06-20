/*
 * $Id: phy5464.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#include "vxWorks.h"
#include "stdio.h"
#include "systemInit.h"
#include "string.h"
#include "config.h"
#include "memreg.h"

#define _SHR_PM_10MB_HD     (1 << 0)
#define _SHR_PM_10MB_FD     (1 << 1)
#define _SHR_PM_100MB_HD    (1 << 2)
#define _SHR_PM_100MB_FD    (1 << 3)
#define _SHR_PM_1000MB_HD   (1 << 4)
#define _SHR_PM_1000MB_FD   (1 << 5)
#define _SHR_PM_2500MB_HD   (1 << 6)
#define _SHR_PM_2500MB_FD   (1 << 7)
#define _SHR_PM_10GB_HD     (1 << 8)
#define _SHR_PM_10GB_FD     (1 << 9)
#define _SHR_PM_PAUSE_TX    (1 << 10) /* TX pause capable */
#define _SHR_PM_PAUSE_RX    (1 << 11) /* RX pause capable */
#define _SHR_PM_PAUSE_ASYMM (1 << 12) /* Asymm pause capable (R/O) */
#define _SHR_PM_TBI         (1 << 13) /* TBI mode supported */
#define _SHR_PM_MII         (1 << 14) /* MII mode supported */
#define _SHR_PM_GMII        (1 << 15) /* GMII mode supported */
#define _SHR_PM_SGMII       (1 << 16) /* SGMII mode supported */
#define _SHR_PM_XGMII       (1 << 17) /* XGMII mode supported */
#define _SHR_PM_LB_MAC      (1 << 18) /* MAC loopback supported */
#define _SHR_PM_LB_NONE     (1 << 19) /* Useful for automated test */
#define _SHR_PM_LB_PHY      (1 << 20) /* PHY loopback supported */
#define _SHR_PM_AN          (1 << 21) /* Auto-negotiation */
#define _SHR_PM_ENCAP_IEEE  (1 << 22) /* IEEE 802.3 enet encap */
#define _SHR_PM_ENCAP_VLAN  (1 << 23) /* VLAN Tagged 802.3 encap*/
#define _SHR_PM_ENCAP_HIGIG (1 << 24) /* Broadcom HiGig encap */
#define _SHR_PM_ENCAP_ALLYR (1 << 25) /* BCM5632 encap */
#define _SHR_PM_CRC_LEAVE   (1 << 26) /* MAC leaves CRC alone */
#define _SHR_PM_CRC_APPEND  (1 << 27) /* MAC appends CRC */
#define _SHR_PM_CRC_REGEN   (1 << 28) /* MAC regenerates CRC */

#define _SHR_PM_PAUSE       (_SHR_PM_PAUSE_TX  | _SHR_PM_PAUSE_RX)
#define _SHR_PM_10MB        (_SHR_PM_10MB_HD   | _SHR_PM_10MB_FD)
#define _SHR_PM_100MB       (_SHR_PM_100MB_HD  | _SHR_PM_100MB_FD)
#define _SHR_PM_1000MB      (_SHR_PM_1000MB_HD | _SHR_PM_1000MB_FD)
#define _SHR_PM_2500MB      (_SHR_PM_2500MB_HD | _SHR_PM_2500MB_FD)
#define _SHR_PM_10GB        (_SHR_PM_10GB_HD   | _SHR_PM_10GB_FD)

#define _SHR_PM_SPEED_ALL   (_SHR_PM_10GB | \
                            _SHR_PM_2500MB | \
                            _SHR_PM_1000MB | \
                            _SHR_PM_100MB | \
                            _SHR_PM_10MB)

#define SOC_PM_10MB_HD      _SHR_PM_10MB_HD
#define SOC_PM_10MB_FD      _SHR_PM_10MB_FD
#define SOC_PM_100MB_HD     _SHR_PM_100MB_HD
#define SOC_PM_100MB_FD     _SHR_PM_100MB_FD
#define SOC_PM_1000MB_HD    _SHR_PM_1000MB_HD
#define SOC_PM_1000MB_FD    _SHR_PM_1000MB_FD
#define SOC_PM_2500MB_HD    _SHR_PM_2500MB_HD
#define SOC_PM_2500MB_FD    _SHR_PM_2500MB_FD
#define SOC_PM_10GB_HD      _SHR_PM_10GB_HD
#define SOC_PM_10GB_FD      _SHR_PM_10GB_FD
#define SOC_PM_PAUSE_TX     _SHR_PM_PAUSE_TX
#define SOC_PM_PAUSE_RX     _SHR_PM_PAUSE_RX
#define SOC_PM_PAUSE_ASYMM  _SHR_PM_PAUSE_ASYMM
#define SOC_PM_TBI          _SHR_PM_TBI
#define SOC_PM_MII          _SHR_PM_MII
#define SOC_PM_GMII         _SHR_PM_GMII
#define SOC_PM_SGMII        _SHR_PM_SGMII
#define SOC_PM_XGMII        _SHR_PM_XGMII
#define SOC_PM_LB_MAC       _SHR_PM_LB_MAC
#define SOC_PM_LB_NONE      _SHR_PM_LB_NONE
#define SOC_PM_LB_PHY       _SHR_PM_LB_PHY
#define SOC_PM_AN           _SHR_PM_AN
#define SOC_PM_ENCAP_IEEE   _SHR_PM_ENCAP_IEEE
#define SOC_PM_ENCAP_VLAN   _SHR_PM_ENCAP_VLAN
#define SOC_PM_ENCAP_HIGIG  _SHR_PM_ENCAP_HIGIG
#define SOC_PM_ENCAP_ALLYR  _SHR_PM_ENCAP_ALLYR
#define SOC_PM_CRC_LEAVE    _SHR_PM_CRC_LEAVE
#define SOC_PM_CRC_APPEND   _SHR_PM_CRC_APPEND
#define SOC_PM_CRC_REGEN    _SHR_PM_CRC_REGEN
#define SOC_PM_PAUSE        _SHR_PM_PAUSE
#define SOC_PM_10MB         _SHR_PM_10MB
#define SOC_PM_100MB        _SHR_PM_100MB
#define SOC_PM_1000MB       _SHR_PM_1000MB
#define SOC_PM_2500MB       _SHR_PM_2500MB
#define SOC_PM_10GB         _SHR_PM_10GB
#define SOC_PM_FD           _SHR_PM_FD
#define SOC_PM_HD           _SHR_PM_HD

#define ADVERT_ALL_COPPER \
    (SOC_PM_PAUSE | SOC_PM_10MB | SOC_PM_100MB | SOC_PM_1000MB)

#define ADVERT_ALL_FIBER \
    (SOC_PM_PAUSE | SOC_PM_1000MB_FD)

/*
 * 1000Base-T Control Register
 */
#define MII_GB_CTRL_MS_MAN      (1 << 12) /* Manual Master/Slave mode */
#define MII_GB_CTRL_MS          (1 << 11) /* Master/Slave negotiation mode */
#define MII_GB_CTRL_PT          (1 << 10) /* Port type */
#define MII_GB_CTRL_ADV_1000FD  (1 << 9)  /* Advertise 1000Base-T FD */
#define MII_GB_CTRL_ADV_1000HD  (1 << 8)  /* Advertise 1000Base-T HD */

/*
 * MII Link Advertisment
 */
#define MII_ANA_ASF        (1 << 0)  /* Advertise Selector Field */
#define MII_ANA_HD_10      (1 << 5)  /* Half duplex 10Mb/s supported */
#define MII_ANA_FD_10      (1 << 6)  /* Full duplex 10Mb/s supported */
#define MII_ANA_HD_100     (1 << 7)  /* Half duplex 100Mb/s supported */
#define MII_ANA_FD_100     (1 << 8)  /* Full duplex 100Mb/s supported */
#define MII_ANA_T4         (1 << 9)  /* T4 */
#define MII_ANA_PAUSE      (1 << 10) /* Pause supported */
#define MII_ANA_ASYM_PAUSE (1 << 11) /* Asymmetric pause supported */
#define MII_ANA_RF         (1 << 13) /* Remote fault */
#define MII_ANA_NP         (1 << 15) /* Next Page */

#define MII_ANA_ASF_802_3  (1)       /* 802.3 PHY */

/* MII Control register */
#define DDS_MII_CTRL_REG   MII_CTRL_REG /* 0x00 */

/* DDS: MII Control bit definitions, same as Standard 10/100 PHY */
#define DDS_MII_CTRL_RESET      MII_CTRL_RESET  /* Self clearing SW reset */
#define DDS_MII_CTRL_SS_LSB     MII_CTRL_SS_LSB /* Speed Select, LSB */
#define DDS_MII_CTRL_SS_MSB     MII_CTRL_SS_MSB /* Speed Select, MSB */
#define DDS_MII_CTRL_SS(_x)     MII_CTRL_SS(_x)
#define DDS_MII_CTRL_SS_10      MII_CTRL_SS_10
#define DDS_MII_CTRL_SS_100     MII_CTRL_SS_100
#define DDS_MII_CTRL_SS_1000    MII_CTRL_SS_1000
#define DDS_MII_CTRL_SS_INVALID MII_CTRL_SS_INVALID
#define DDS_MII_CTRL_SS_MASK    MII_CTRL_SS_MASK
#define DDS_MII_CTRL_FD         MII_CTRL_FD /* Full Duplex */
#define DDS_MII_CTRL_AN_RESTART (1 << 9)    /* Auto Negotiation Enable */
#define DDS_MII_CTRL_AN_ENABLE  (1 << 12)   /* Auto Negotiation Enable */
#define DDS_MII_CTRL_LOOPBACK   (1 << 14)   /* Loopback Enable */

/* 1000X Control #1 Register: Controls 10B/SGMII mode */
#define DDS_1000X_CTRL1_REG     0x10
#define DDS_1000X_FIBER_MODE   (1 << 0)     /* Enable SGMII fiber mode */

/* 1000X Control #2 Register: */
#define DDS_1000X_CTRL2_REG      0x11

/* 1000X Control #3 Register: */
#define DDS_1000X_CTRL3_REG             0x12

/* 1000X Control #4 Register: */
#define DDS_1000X_CTRL4_REG             0x13

typedef enum soc_port_ms_e
{
    SOC_PORT_MS_SLAVE,
    SOC_PORT_MS_MASTER,
    SOC_PORT_MS_AUTO,
    SOC_PORT_MS_NONE,
    SOC_PORT_MS_COUNT /* last, please */
} soc_port_ms_t;

typedef enum _shr_port_mdix_e
{
    _SHR_PORT_MDIX_AUTO,
    _SHR_PORT_MDIX_FORCE_AUTO,
    _SHR_PORT_MDIX_NORMAL,
    _SHR_PORT_MDIX_XOVER,
    _SHR_PORT_MDIX_COUNT /* last, please */
} _shr_port_mdix_t;

typedef enum soc_port_mdix_e
{
    SOC_PORT_MDIX_AUTO        = _SHR_PORT_MDIX_AUTO,
    SOC_PORT_MDIX_FORCE_AUTO  = _SHR_PORT_MDIX_FORCE_AUTO,
    SOC_PORT_MDIX_NORMAL      = _SHR_PORT_MDIX_NORMAL,
    SOC_PORT_MDIX_XOVER       = _SHR_PORT_MDIX_XOVER,
    SOC_PORT_MDIX_COUNT       = _SHR_PORT_MDIX_COUNT
} soc_port_mdix_t;

#define GE_PORT_CONFIG_SPEED(v)  (((v)>>1)&0x3)

extern int PCI_DEBUG_ON;

int
soc_miim_write(int unit, uint8 phy_id, uint8 phy_reg_addr, uint16 phy_wr_data)
{
    int     rv = SOC_E_NONE;
    uint32  phy_param;

    phy_param = ((uint32)phy_id << MIIM_PARAM_ID_OFFSET | (uint32)phy_wr_data);

    WRITE_CMIC_MIIM_ADDRESSr(unit, phy_reg_addr);

    soc_pci_write(unit, CMIC_MIIM_PARAM, phy_param);

    soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MIIM_WR_START_SET);

    {
        soc_timeout_t to;

        soc_timeout_init(&to, MIIM_TIMEOUT, 250);

        while ((soc_pci_read(unit, CMIC_SCHAN_CTRL) &
        SC_MIIM_OP_DONE_TST) == 0) {
            if (soc_timeout_check(&to)) {
                rv = SOC_E_TIMEOUT;
                break;
            }
        }

        soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MIIM_OP_DONE_CLR);
    }

    if (rv == SOC_E_TIMEOUT) {
        PRINTF_ERROR(("soc_miim_write: "
            "timeout (id=0x%02x addr=0x%02x data=0x%04x)\n",
            phy_id, phy_reg_addr, phy_wr_data));
    }

    return rv;
}


int
soc_miim_read(int unit, uint8 phy_id, uint8 phy_reg_addr, uint16 *phy_rd_data)
{
    int                 rv = SOC_E_NONE;
    uint32              phy_param;

    phy_param = (uint32)phy_id << MIIM_PARAM_ID_OFFSET;

    WRITE_CMIC_MIIM_ADDRESSr(unit, phy_reg_addr);

    soc_pci_write(unit, CMIC_MIIM_PARAM, phy_param);

    soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MIIM_RD_START_SET);

    {
        soc_timeout_t to;

        soc_timeout_init(&to, MIIM_TIMEOUT, 250);

        while ((soc_pci_read(unit, CMIC_SCHAN_CTRL) & SC_MIIM_OP_DONE_TST) == 0) {
            if (soc_timeout_check(&to)) {
                rv = SOC_E_TIMEOUT;
                break;
            }
        }

        soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_MIIM_OP_DONE_CLR);
    }

    if (rv == SOC_E_TIMEOUT) {
        PRINTF_ERROR(("soc_miim_read: "
            "timeout (id=0x%02x addr=0x%02x)\n",
            phy_id, phy_reg_addr));
    }

    *phy_rd_data = (uint16)soc_pci_read(unit, CMIC_MIIM_READ_DATA);

    return rv;
}


#define PORT_TO_PHY_ADDR(u,p)  (p + ((p > 29) ? (0x40 - 29) : (1)))

int
phy_fe_ge_reset(int unit, soc_port_t port, void *user_arg)
{
    uint8     phy_addr = PORT_TO_PHY_ADDR(unit, port);
    uint16        ctrl, tmp;
    soc_timeout_t     to;
    int           timeout = 0;

    COMPILER_REFERENCE(user_arg);

    /*  SOC_IF_ERROR_RETURN(soc_miim_read(unit, phy_addr, MII_CTRL_REG, &ctrl));*/
    ctrl = 0x91C0 & (~MII_CTRL_RESET);

    SOC_IF_ERROR_RETURN
        (soc_miim_write(unit, phy_addr, MII_CTRL_REG, ctrl | MII_CTRL_RESET));
    PRINTF_DEBUG2(("____WRITTING 0x%08x 0x%08x 0x%08x\n", phy_addr, MII_CTRL_REG, ctrl | MII_CTRL_RESET));
    soc_timeout_init(&to, PHY_RESET_TIMEOUT_USEC, 1);

    do {
        SOC_IF_ERROR_RETURN(soc_miim_read(unit, phy_addr, MII_CTRL_REG, &tmp));
        if (soc_timeout_check(&to)) {
            timeout = 1;
            break;
        }
    } while ((tmp & MII_CTRL_RESET) != 0);

    if (timeout) {
        PRINTF_ERROR(("phy_fe_ge_reset: on u=%d p=%d\n", unit, port));
        SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, MII_CTRL_REG, ctrl));
    }

    return (SOC_E_NONE);
}


int
phy_ge_init(int unit, soc_port_t port)
{
    uint8     phy_addr = PORT_TO_PHY_ADDR(unit, port);

    /* Reset PHY */
    SOC_IF_ERROR_RETURN( phy_fe_ge_reset( unit, port, NULL ) );

#define phy_ge_interface_set(u,p,v) \
        (soc_miim_write((u), PORT_TO_PHY_ADDR((u),(p)), 0x10, (v)))

    phy_ge_interface_set(unit, port, 0x0);

    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, MII_GB_CTRL_REG, 0x600));
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, MII_CTRL_REG,  0x1340));

    return(SOC_E_NONE);
}


typedef uint32 _shr_port_mode_t;

typedef struct _shr_phy_config_s {
    int         enable;
    int         preferred;
    int         autoneg_enable;
    _shr_port_mode_t    autoneg_advert;
    int         force_speed;
    int         force_duplex;
    int         master;
    _shr_port_mdix_t    mdix;
} _shr_phy_config_t;

typedef _shr_port_mode_t soc_port_mode_t;
typedef _shr_phy_config_t soc_phy_config_t;

typedef struct phy_5464_state_s {
    uint16            phy_model;    /* PHY model from ID reg */
    uint16            phy_rev;      /* PHY revision from ID reg */
    uint8             ledmode[4];   /* LED1..4 selectors */
    uint16            ledctrl;      /* LED control */
    uint16            ledselect;    /* LED select */
    int               automedium;   /* Medium auto-select active */
    int               fiber_detect; /* Fiber Signal Detection */
    soc_phy_config_t  copper;       /* Current copper config */
    soc_phy_config_t  fiber;        /* Current fiber config */
} phy_5464_state_t;

/** FIXED allocation of phy states */
static phy_5464_state_t phy_5464_state[2][54];
#define PHY_MODEL(id0,id1) ((id1)>>4&0x3f)
#define PHY_REV(id0, id1)  ((id1)&0xf)
/** always copper mode */
#define COPPER_MODE(u,p)   (1)

#define PORT_TO_PHY_ADDR_INT(u,p) ((p) + 128)


int
bcm_port_jam_set(int unit, bcm_port_t port, int enable)
{
    uint32 gpc;
    int    oenable;

    SOC_IF_ERROR_RETURN(READ_GE_PORT_CONFIGr(unit, port, &gpc));
    oenable = soc_reg_field_get(unit, GE_PORT_CONFIGr, gpc, JAM_ENf);
    enable = enable ? 1 : 0;
    if (oenable != enable) {
        soc_reg_field_set(unit, GE_PORT_CONFIGr, &gpc, JAM_ENf, enable);
        SOC_IF_ERROR_RETURN(WRITE_GE_PORT_CONFIGr(unit, port, gpc));
    }

    return BCM_E_NONE;
}


int
phy_56xxx_init(int unit, soc_port_t port)
{
    uint8               phy_addr = PORT_TO_PHY_ADDR_INT(unit, port);

    PRINTF_DEBUG2(("phy_56xxx_init: u=%d p=%d %d\n", unit, port, phy_addr));

    /*    SOC_IF_ERROR_RETURN(phy_56xxx_attach(unit, port));*/

    /*
     * The internal SERDES PHY's reset bit is self-clearing.
     */
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, DDS_MII_CTRL_REG, 0x9140));

    sal_usleep(10000);

    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, DDS_1000X_CTRL2_REG, 0x6));

    /** COPPER MODE */
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, DDS_1000X_CTRL1_REG, 0x180));

    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, DDS_1000X_CTRL2_REG, 0x6));
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, DDS_MII_CTRL_REG, 0x140));

    return SOC_E_NONE;
}


static int _phy_5464_medium_config_update(int unit, soc_port_t port, soc_phy_config_t *cfg);

static int
_phy_5464_reset_setup( int unit, soc_port_t port)
{
    phy_5464_state_t    *st = &phy_5464_state[unit][port];
    uint8               phy_addr = PORT_TO_PHY_ADDR(unit, port);
    uint16              tmp, otmp;

    PRINTF_DEBUG2(("_phy_5464_reset_setup: u=%d p=%d medium=%s\n",
        unit, port, COPPER_MODE(unit, port) ? "COPPER" : "FIBER"));

    SOC_IF_ERROR_RETURN(phy_ge_init(unit, port));

    /* phy_ge_init has reset the phy, powering down the unstrapped interface */
    /* make sure enabled interfaces are powered up */
    /* fiber regs */
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, 0x1c, 0x7c00));
    SOC_IF_ERROR_RETURN(soc_miim_read(unit, phy_addr, 0x1c, &tmp));
    tmp |= 0x0001;
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, 0x1c, tmp | 0x8000));

    /* remove power down */
    SOC_IF_ERROR_RETURN(soc_miim_read(unit, phy_addr, MII_CTRL_REG, &tmp));
    otmp = tmp;

    /** dodeca_serdes */
    tmp &= ~MII_CTRL_PD;

    tmp &= ~MII_CTRL_PD;

    /*
     * Disable SGMII autonegotiation on the switch side.  SGMII autoneg
     * does not work in the 5690 (but SERDES autoneg for fiber
     * pass-through does work).
     */
    tmp &= ~MII_CTRL_AE;

    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, MII_CTRL_REG, tmp));

    /* copper regs */
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, 0x1c, 0x7c00));
    SOC_IF_ERROR_RETURN(soc_miim_read(unit, phy_addr, 0x1c, &tmp));
    tmp &= ~0x0001;
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, 0x1c, tmp | 0x8000));

    /* remove power down */
    SOC_IF_ERROR_RETURN(soc_miim_read(unit, phy_addr, MII_CTRL_REG, &tmp));
    otmp = tmp;

    tmp &= ~MII_CTRL_PD;
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, MII_CTRL_REG, tmp));

    {
        uint8   phy_addr_int = PORT_TO_PHY_ADDR_INT(unit, port);
        uint16      ctrl;

        SOC_IF_ERROR_RETURN(phy_56xxx_init(unit, port));

        SOC_IF_ERROR_RETURN
            (soc_miim_read(unit, phy_addr_int, DDS_1000X_CTRL1_REG, &ctrl));

        /*
         * Put the Serdes in SGMII mode
         */
        ctrl &= ~DDS_1000X_FIBER_MODE;
        SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr_int, DDS_1000X_CTRL1_REG, 0x180));
        /** auto negotiation is off for internal phy */
    }

    /* Configure Auto-detect Medium (0x1c shadow 11110) */
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, 0x1c, 0x7800));
    SOC_IF_ERROR_RETURN(soc_miim_read(unit, phy_addr, 0x1c, &tmp));
    otmp = tmp;
    tmp &= ~0x1f; /* Clear auto-select requirements */

    /*
     * Enable internal inversion of LED4/SD input pin.  Used only if
     * the fiber module provides RX_LOS instead of Signal Detect.
     */
    PRINTF_DEBUG2(("_phy_5464_reset_setup: u=%d p=%d 0x1c val=%04x\n",
                   unit, port, tmp));

    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, 0x1c, tmp | 0x8000));

    /*
     * Configure Auxiliary 1000BASE-X control register to turn off
     * carrier extension.  The Intel 7131 NIC does not accept carrier
     * extension and gets CRC errors.
     */

    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, 0x1c, 0x6c00));
    SOC_IF_ERROR_RETURN(soc_miim_read(unit, phy_addr, 0x1c, &tmp));
    tmp |= 0x40; /* Disable carrier extension */
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, 0x1c, 0x8000 | tmp));

    /* Configure Extended Control Register */
    SOC_IF_ERROR_RETURN(soc_miim_read(unit, phy_addr, MII_ECR_REG, &tmp));
    tmp |= 0x0020; /* Enable LEDs to indicate traffic status */
    tmp |= 0x0001; /* Set high FIFO elasticity to support jumbo frames */
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, MII_ECR_REG, tmp));

    /* Enable extended packet length (4.5k through 25k) */
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, 0x18, 0x0007));
    SOC_IF_ERROR_RETURN(soc_miim_read(unit, phy_addr, 0x18, &tmp));
    tmp |= 0x4000;
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, 0x18, tmp));

    /* Configure LED selectors */
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, 0x1c,
        0xb400 |
        ((st->ledmode[1] & 0xf) << 4) |
        (st->ledmode[0] & 0xf)));
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, 0x1c,
        0xb800 |
        ((st->ledmode[3] & 0xf) << 4) |
        (st->ledmode[2] & 0xf)));
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, 0x1c,
        0xa400 | (st->ledctrl & 0x3ff)));
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, 0x17, 0x0f04));
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, 0x15, st->ledselect));
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, 0x17, 0));

    /* if using led link/activity mode, disable led traffic mode */
    if (st->ledctrl & 0x10 || st->ledselect == 0x01) {
        SOC_IF_ERROR_RETURN(soc_miim_read(unit, phy_addr, MII_ECR_REG, &tmp));
        tmp &= ~0x0020;
        SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, MII_ECR_REG, tmp));
    }
    return SOC_E_NONE;
}


int
phy_5464_init(int unit, soc_port_t port)
{
    phy_5464_state_t    *st;
    uint8               phy_addr = PORT_TO_PHY_ADDR(unit, port);
    int                 fiber_preferred;
    uint16              id0, id1;

    PRINTF_DEBUG2(("phy_5464_init: u=%d p=%d phy_addr=%d\n",
        unit, port, (int)phy_addr));

    st = &phy_5464_state[unit][port];

    memset(st, 0, sizeof (*st));

    SOC_IF_ERROR_RETURN(soc_miim_read(unit, phy_addr, MII_PHY_ID0_REG, &id0));
    SOC_IF_ERROR_RETURN(soc_miim_read(unit, phy_addr, MII_PHY_ID1_REG, &id1));

    st->phy_model = PHY_MODEL(id0, id1);
    st->phy_rev = PHY_REV(id0, id1);

    /*
     * In automedium mode, the property phy_fiber_pref_port<X> is used
     * to set a preference for fiber mode in the event both copper and
     * fiber links are up.
     *
     * If NOT in automedium mode, phy_fiber_pref_port<X> is used to
     * select which of fiber or copper is forced.
     */
    fiber_preferred  = 0;
    st->automedium   = 0;
    st->fiber_detect = 0;

    PRINTF_DEBUG2(("phy_5464_init: "
        "u=%d p=%d type=546%d%s automedium=%d fiber_pref=%d detect=%d\n",
        unit, port, 4,
        "",
        st->automedium, fiber_preferred, st->fiber_detect));

    st->copper.enable = TRUE;
    st->copper.preferred = !fiber_preferred;
    st->copper.autoneg_enable = TRUE;
    st->copper.autoneg_advert = ADVERT_ALL_COPPER;
    st->copper.force_speed = 1000;
    st->copper.force_duplex = TRUE;
    st->copper.master = SOC_PORT_MS_AUTO;
    st->copper.mdix = SOC_PORT_MDIX_AUTO;

    st->fiber.enable = FALSE;
    st->fiber.preferred = fiber_preferred;
    st->fiber.autoneg_enable = 0;
    st->fiber.autoneg_advert = ADVERT_ALL_FIBER;
    st->fiber.force_speed = 1000;
    st->fiber.force_duplex = TRUE;
    st->fiber.master = SOC_PORT_MS_NONE;
    st->fiber.mdix = SOC_PORT_MDIX_NORMAL;

    /* Get Requested LED selectors (defaults are hardware defaults) */
    st->ledmode[0] = 0;
    st->ledmode[1] = 1;
    st->ledmode[2] = 3;
    st->ledmode[3] = 6;
    st->ledctrl = 8;
    st->ledselect = 0;

    SOC_IF_ERROR_RETURN(_phy_5464_reset_setup(unit, port));

    SOC_IF_ERROR_RETURN(_phy_5464_medium_config_update(unit, port, &st->copper));

    return SOC_E_NONE;
}


static int
phy_fe_ge_speed_set(int unit, soc_port_t port, int speed)
{
    uint8   phy_addr = PORT_TO_PHY_ADDR(unit, port);
    uint16  mii_ctrl;

    if (speed == 0)
        return SOC_E_NONE;

    SOC_IF_ERROR_RETURN(soc_miim_read(unit, phy_addr, MII_CTRL_REG, &mii_ctrl));

    mii_ctrl &= ~(MII_CTRL_SS_LSB | MII_CTRL_SS_MSB);

    switch(speed) {
        case 10:
            mii_ctrl |= MII_CTRL_SS_10;
            break;
        case 100:
            mii_ctrl |= MII_CTRL_SS_100;
            break;
        case 1000:
            mii_ctrl |= MII_CTRL_SS_1000;
            break;
        default:
            return(SOC_E_CONFIG);
    }

    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, MII_CTRL_REG, mii_ctrl));

    return(SOC_E_NONE);
}


static int
phy_5464_speed_set(int unit, soc_port_t port, int speed)
{
    phy_5464_state_t    *st = &phy_5464_state[unit][port];

    PRINTF_DEBUG2(("phy_5464_speed_set: u=%d p=%d s=%d\n",unit, port, speed));

    /** only deal with copper mode */
    SOC_IF_ERROR_RETURN(phy_fe_ge_speed_set(unit, port, speed));
    st->copper.force_speed = speed;

    return SOC_E_NONE;
}


static int
phy_fe_ge_duplex_set(int unit, soc_port_t port, int duplex)
{
    uint8   phy_addr = PORT_TO_PHY_ADDR(unit, port);
    uint16  mii_ctrl;

    SOC_IF_ERROR_RETURN(soc_miim_read(unit, phy_addr, MII_CTRL_REG, &mii_ctrl));

    if (duplex) {
        mii_ctrl |= MII_CTRL_FD;
    }
    else {
        mii_ctrl &= ~MII_CTRL_FD;
    }

    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, MII_CTRL_REG, mii_ctrl));

    return(SOC_E_NONE);
}


static int
phy_5464_duplex_set(int unit, soc_port_t port, int duplex)
{
    phy_5464_state_t    *st = &phy_5464_state[unit][port];

    PRINTF_DEBUG2(("phy_5464_duplex_set: u=%d p=%d d=%d\n",unit, port, duplex));

    /* Note: mac.c uses phy_5690_notify_duplex to update 5690 */
    SOC_IF_ERROR_RETURN(phy_fe_ge_duplex_set(unit, port, duplex));
    st->copper.force_duplex = duplex;

    return SOC_E_NONE;
}


static int
phy_fe_ge_master_set(int unit, soc_port_t port, int master)
{
    uint8 phy_addr = PORT_TO_PHY_ADDR(unit, port);
    uint16    mii_gb_ctrl;

    /* 
        if (IS_FE_PORT(unit, port)) {
            return(SOC_E_NONE);
        }
    */

    SOC_IF_ERROR_RETURN
        (soc_miim_read(unit, phy_addr, MII_GB_CTRL_REG, &mii_gb_ctrl));

    switch (master) {
        case SOC_PORT_MS_SLAVE:
            mii_gb_ctrl |= MII_GB_CTRL_MS_MAN;
            mii_gb_ctrl &= ~MII_GB_CTRL_MS;
            break;
        case SOC_PORT_MS_MASTER:
            mii_gb_ctrl |= MII_GB_CTRL_MS_MAN;
            mii_gb_ctrl |= MII_GB_CTRL_MS;
            break;
        case SOC_PORT_MS_AUTO:
            mii_gb_ctrl &= ~MII_GB_CTRL_MS_MAN;
            break;
    }

    SOC_IF_ERROR_RETURN
        (soc_miim_write(unit, phy_addr, MII_GB_CTRL_REG, mii_gb_ctrl));

    return(SOC_E_NONE);
}


static int
phy_5464_master_set(int unit, soc_port_t port, int master)
{
    phy_5464_state_t    *st = &phy_5464_state[unit][port];

    PRINTF_DEBUG2(("phy_5464_master_set: u=%d p=%d master=%d\n",
                   unit, port, master));

    SOC_IF_ERROR_RETURN(phy_fe_ge_master_set(unit, port, master));
    st->copper.master = master;

    return SOC_E_NONE;
}


static int
phy_ge_adv_local_set(int unit, soc_port_t port, soc_port_mode_t mode)
{
    uint8   phy_addr = PORT_TO_PHY_ADDR(unit, port);
    uint16  mii_ctrl, mii_adv, mii_gb_ctrl;

    mii_adv = MII_ANA_ASF_802_3;

    SOC_IF_ERROR_RETURN(soc_miim_read(unit, phy_addr, MII_GB_CTRL_REG, &mii_gb_ctrl));

    mii_gb_ctrl &= ~(MII_GB_CTRL_ADV_1000HD | MII_GB_CTRL_ADV_1000FD);

    if (mode & SOC_PM_10MB_HD)  mii_adv |= MII_ANA_HD_10;
    if (mode & SOC_PM_10MB_FD)  mii_adv |= MII_ANA_FD_10;
    if (mode & SOC_PM_100MB_HD) mii_adv |= MII_ANA_HD_100;
    if (mode & SOC_PM_100MB_FD) mii_adv |= MII_ANA_FD_100;
    if (mode & SOC_PM_1000MB_HD) mii_gb_ctrl |= MII_GB_CTRL_ADV_1000HD;
    if (mode & SOC_PM_1000MB_FD) mii_gb_ctrl |= MII_GB_CTRL_ADV_1000FD;

    if ((mode & SOC_PM_PAUSE) == SOC_PM_PAUSE) {
        /* Advertise symmetric pause */
        mii_adv |= MII_ANA_PAUSE;
    }
    else {
        /*
         * For Asymmetric pause,
         *   if (Bit 10)
         *       then pause frames flow toward the Transceiver
         *       else pause frames flow toward link partner.
         */
        if (mode & SOC_PM_PAUSE_TX) {
            mii_adv |= MII_ANA_ASYM_PAUSE;
        }
        else if (mode & SOC_PM_PAUSE_RX) {
            mii_adv |= MII_ANA_ASYM_PAUSE;
            mii_adv |= MII_ANA_PAUSE;
        }
    }

    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, MII_ANA_REG, mii_adv));
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, MII_GB_CTRL_REG, mii_gb_ctrl));

    /* Restart auto-neg if enabled */

    SOC_IF_ERROR_RETURN(soc_miim_read(unit, phy_addr, MII_CTRL_REG, &mii_ctrl));

    if (mii_ctrl & MII_CTRL_AE) {
        SOC_IF_ERROR_RETURN
            (soc_miim_write(unit, phy_addr,
            MII_CTRL_REG, mii_ctrl | MII_CTRL_RAN));
    }

    return(SOC_E_NONE);
}


static int
phy_5464_adv_local_set(int unit, soc_port_t port, soc_port_mode_t mode)
{
    phy_5464_state_t    *st = &phy_5464_state[unit][port];

    PRINTF_DEBUG2(("phy_5464_adv_local_set: u=%d p=%d mode=0x%x\n",
                   unit, port, mode));
    SOC_IF_ERROR_RETURN(phy_ge_adv_local_set(unit, port, mode));
    st->copper.autoneg_advert = mode;

    return SOC_E_NONE;
}


static int
phy_fe_ge_an_set(int unit, soc_port_t port, int an)
{
    uint8   phy_addr = PORT_TO_PHY_ADDR(unit, port);
    uint16  mii_ctrl;

    SOC_IF_ERROR_RETURN(soc_miim_read(unit, phy_addr, MII_CTRL_REG, &mii_ctrl));

    if (an) {
        mii_ctrl |= MII_CTRL_AE | MII_CTRL_RAN;
    }
    else {
        mii_ctrl &= ~(MII_CTRL_AE | MII_CTRL_RAN);
    }

    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, MII_CTRL_REG, mii_ctrl));

    return(SOC_E_NONE);
}


static int
phy_5464_autoneg_set(int unit, soc_port_t port, int autoneg)
{
    phy_5464_state_t    *st = &phy_5464_state[unit][port];

    PRINTF_DEBUG2(("phy_5464_autoneg_set: u=%d p=%d autoneg=%d auto=%d\n",
        unit, port, autoneg, st->automedium));

    SOC_IF_ERROR_RETURN(phy_fe_ge_an_set(unit, port, autoneg));
    st->copper.autoneg_enable = autoneg ? 1 : 0;
    return SOC_E_NONE;
}


static int
phy_5464_mdix_set(int unit, soc_port_t port, soc_port_mdix_t mode)
{
    phy_5464_state_t    *st = &phy_5464_state[unit][port];
    uint8                phy_addr = PORT_TO_PHY_ADDR(unit, port);
    uint16               tmp;
    /*    int                  speed;*/

    if (st == NULL) {
        return SOC_E_PARAM;
    }

    /** always AUTO_MDIX regardless */

    SOC_IF_ERROR_RETURN(soc_miim_read(unit, phy_addr, 0x10, &tmp));
    tmp &= ~0x4000; /* Clear bit 14 for automatic MDI crossover */
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, 0x10, tmp));

    /*
     * Read PHY register 0x18 shadow copy 7
     */
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, 0x18, 0x7007));
    SOC_IF_ERROR_RETURN(soc_miim_read(unit, phy_addr, 0x18, &tmp));
    tmp &= ~0x0200; /* Clear bit 9 to disable forced auto MDI xover */

    /*
     * Write the result in the register 0x18, shadow copy 7
     */
    SOC_IF_ERROR_RETURN(soc_miim_write(unit, phy_addr, 0x18, tmp | 0x8007));

    return SOC_E_NONE;
}


int phy_fe_ge_link_get(int unit, soc_port_t port, int *link);

static int
_phy_5464_medium_config_update(int unit, soc_port_t port, soc_phy_config_t *cfg)
{

    SOC_IF_ERROR_RETURN(phy_5464_speed_set(unit, port, cfg->force_speed));
    SOC_IF_ERROR_RETURN(phy_5464_duplex_set(unit, port, cfg->force_duplex));
    SOC_IF_ERROR_RETURN(phy_5464_master_set(unit, port, cfg->master));
    SOC_IF_ERROR_RETURN(phy_5464_adv_local_set(unit, port, cfg->autoneg_advert));
    SOC_IF_ERROR_RETURN(phy_5464_autoneg_set(unit, port, cfg->autoneg_enable));
    SOC_IF_ERROR_RETURN(phy_5464_mdix_set(unit, port, cfg->mdix));

    return SOC_E_NONE;
}


typedef enum soc_mac_mode_e {
    SOC_MAC_MODE_10_100, /* 10/100 Mb/s MAC selected */
    SOC_MAC_MODE_10,     /* 10/100 Mb/s MAC in 10Mbps mode */
    SOC_MAC_MODE_1000_T, /* 1000/TURBO MAC selected */
    SOC_MAC_MODE_10000   /* 10G MAC selected */
} soc_mac_mode_t;

int
_port_sp_sel_get(int unit, int port)
{
    uint32 val;

    SOC_IF_ERROR_RETURN(READ_GE_PORT_CONFIGr(unit, port, &val));

    /* PRINTF_DEBUG(("_port_sp_sel_get :: %2d 0x%08x\n", port, val));*/

    return GE_PORT_CONFIG_SPEED(val);
}


static int
_port_sp_sel_set(int unit, int port, uint32 sp_sel)
{
    uint32 val;

    SOC_IF_ERROR_RETURN(READ_GE_PORT_CONFIGr(unit, port, &val));
    val &= ~0x6;
    val |= (sp_sel & 0x3) << 1;
    SOC_IF_ERROR_RETURN(WRITE_GE_PORT_CONFIGr(unit, port, val));

    return sp_sel;
}


int
soc_mac_mode_get(int unit, soc_port_t port, soc_mac_mode_t *mode)
{

    uint16 data1=0;
    uint16 data2=0;
    uint8 phyid=(uint8)(port+1); 
    int   rv = SOC_E_NONE;

    /* testMaxFrame(unit,port); */
    *mode = SOC_MAC_MODE_1000_T;
    rv = soc_miim_read(unit, phyid, 0x9, &data1);
    rv += soc_miim_read(unit, phyid, 0xa, &data2);

    if ( data1 && data2 ) {
        *mode = SOC_MAC_MODE_1000_T;
        _port_sp_sel_set(unit, port, 0);
        soc_miim_write(unit,port+128,0,0x140);
        return SOC_E_NONE;
    }
    rv = 0;
    rv = soc_miim_read(unit, phyid, 0x4, &data1);
    rv += soc_miim_read(unit, phyid, 0x5, &data2);

    if ( (data1 & data2) & 0x180 ) {
        *mode = SOC_MAC_MODE_10_100;
        _port_sp_sel_set(unit, port, 1);
        bcm_port_jam_set(unit, port, 1);
        soc_miim_write(unit, port+128, 0, 0x2000);
        return SOC_E_NONE;
    }
    else if ( (data1 & data2) & 0x60 ) {
        *mode = SOC_MAC_MODE_10;
        _port_sp_sel_set(unit, port, 2);
        bcm_port_jam_set(unit, port, 1);
        soc_miim_write(unit, port+128, 0, 0x0000);
        return SOC_E_NONE;
    }
    else
        return SOC_E_NONE;

}


static int
mac_fe_duplex_get(int unit, soc_port_t port, int *duplex)
{
    uint32      fe_mac2;

    SOC_IF_ERROR_RETURN(READ_FE_MAC2r(unit, port, &fe_mac2));

    *duplex = (fe_mac2 & 1);

    return (SOC_E_NONE);
}


static int
mac_fe_speed_get(int unit, soc_port_t port, int *speed)
{
    uint32      fe_supp;

    SOC_IF_ERROR_RETURN(READ_FE_SUPPr(unit, port, &fe_supp));

    *speed = ( (fe_supp & 0x100) ? 100 : 10);

    return (SOC_E_NONE);
}


static int
mac_fe_ifg_to_ipg(int unit, soc_port_t port, int speed,
                  int duplex, int ifg, int *ipg)
{
    /*
     * The inter-frame gap should be a multiple of 8 bit-times.
     */
    ifg = (ifg + 7) & ~7;

    /*
     * The smallest supported ifg is 64 bit-times
     */
    ifg = (ifg < 64) ? 64 : ifg;

    /*
     * Now we need to convert the value according to various chips'
     * peculiarities
     */
    if (duplex) {
        *ipg = ifg / 4 - 3;
    } else {
        switch (speed) {
            case 10:
                *ipg = ifg / 4 - 12;
                break;
            case 100:
                *ipg = ifg / 4 - 13;
                break;
            default:
                return SOC_E_INTERNAL;
        }
    }

    /*
     * Notice, that since we force the minimal ifg to be 64, then there
     * no way ipg will be less than 0 or something like that.
     */
    return SOC_E_NONE;
}


#define __IFG_DEFAULT     96

static int
mac_fe_ipg_update(int unit, int port)
{
    int                 fd, speed, ifg, ipg;
    /*    soc_ipg_t           *si = &SOC_PERSIST(unit)->ipg[port];*/

    SOC_IF_ERROR_RETURN(mac_fe_duplex_get(unit, port, &fd));
    SOC_IF_ERROR_RETURN(mac_fe_speed_get(unit, port, &speed));

    ifg = __IFG_DEFAULT;

    /*
     * Convert the ifg value from bit-times into IPG register-specific value
     */
    
    ifg = 0x15;

    /*
     * Program the appropriate register
     */
    SOC_IF_ERROR_RETURN(WRITE_FE_IPGTr(unit, port, ipg));

    return SOC_E_NONE;
}


static int mac_fe_enable_set(int unit, soc_port_t port, int enable);

static int
mac_fe_enable_get(int unit, soc_port_t port, int *enable)
{
    uint32      regval;
    SOC_IF_ERROR_RETURN(READ_FE_MAC1r(unit, port, &regval));
    *enable = regval & 1;
    return SOC_E_NONE;
}


static  int
mac_fe_duplex_set(int unit, soc_port_t port, int duplex)
{
    uint32      fe_mac2;
    int         old_mac_en;

    SOC_IF_ERROR_RETURN(mac_fe_enable_get(unit, port, &old_mac_en));
    if (old_mac_en) {
        SOC_IF_ERROR_RETURN(mac_fe_enable_set(unit, port, FALSE));
    }

    SOC_IF_ERROR_RETURN(READ_FE_MAC2r(unit, port, &fe_mac2));
    fe_mac2 |= (duplex ? 1 : 0);
    SOC_IF_ERROR_RETURN(WRITE_FE_MAC2r(unit, port, fe_mac2));

    /* Set IPG to match new duplex */
    SOC_IF_ERROR_RETURN(mac_fe_ipg_update(unit, port));

    if (old_mac_en) {
        SOC_IF_ERROR_RETURN(mac_fe_enable_set(unit, port, TRUE));
    }

    return (SOC_E_NONE);
}


static int
mac_ge_duplex_get(int unit, soc_port_t port, int *duplex)
{
    soc_mac_mode_t      mode;

    SOC_IF_ERROR_RETURN(soc_mac_mode_get(unit, port, &mode));

    if ((mode == SOC_MAC_MODE_10_100) || (mode == SOC_MAC_MODE_10)) {

        return mac_fe_duplex_get(unit, port, duplex);

    }
    else {
        uint32          gmacc1;
        SOC_IF_ERROR_RETURN(READ_GMACC1r(unit, port, &gmacc1));

        #define GMACC1_GET_FULLDf(v) ((v)&1)
        *duplex = GMACC1_GET_FULLDf( gmacc1 );

        return SOC_E_NONE;
    }
}


static int
mac_ge_speed_get(int unit, soc_port_t port, int *speed)
{
    soc_mac_mode_t      mode;
    int                 rv = SOC_E_NONE;

    SOC_IF_ERROR_RETURN(soc_mac_mode_get(unit, port, &mode));

    switch(mode) {
        case SOC_MAC_MODE_10:
        case SOC_MAC_MODE_10_100:
            rv = mac_fe_speed_get(unit, port, speed);
            break;
        case SOC_MAC_MODE_1000_T:
            /*SOC_IF_ERROR_RETURN(_mac_turbo_get(unit, port, &turbo));*/
            *speed = 1000; /*turbo ? 2500 : 1000;*/
            break;
        default:
            rv = SOC_E_INTERNAL;
    }
    return(rv);
}


static int
mac_ge_ifg_to_ipg(int unit, soc_port_t port,
                  int speed, int duplex, int ifg, int *ipg)
{
    /*
     * The inter-frame gap should be a multiple of 8 bit-times.
     */
    ifg = (ifg + 7) & ~7;

    /*
     * The smallest supported ifg is 64 bit-times
     */
    ifg = (ifg < 64) ? 64 : ifg;

    /*
     * Now we need to convert the value according to various chips'
     * peculiarities (there are none as of now)
     */
    *ipg = ifg / 8;

    return SOC_E_NONE;
}


static int
mac_ge_ipg_update(int unit, int port)
{
    uint32              gmacc2;
    int                 fd, speed, ipg, ifg;
    /*    soc_ipg_t           *si = &SOC_PERSIST(unit)->ipg[port];*/

    SOC_IF_ERROR_RETURN(mac_ge_duplex_get(unit, port, &fd));
    SOC_IF_ERROR_RETURN(mac_ge_speed_get(unit, port, &speed));

    if ( speed == 100 || speed == 10 )
        return mac_fe_ipg_update(unit, port);

    ifg = __IFG_DEFAULT;

    /*
     * Convert the ifg value from bit-times into IPG register-specific value
     */
    SOC_IF_ERROR_RETURN(mac_ge_ifg_to_ipg(unit, port, speed, fd, ifg, &ipg));

    /*
     * Program the appropriate register (if necessary)
     */
    SOC_IF_ERROR_RETURN(READ_GMACC2r(unit, port, &gmacc2));
    if ( gmacc2 != ipg ) {
        SOC_IF_ERROR_RETURN(WRITE_GMACC2r(unit, port, ipg));
    }

    return SOC_E_NONE;
}


static int
mac_ge_duplex_set(int unit, soc_port_t port, int duplex)
{
    soc_mac_mode_t      mode;

    SOC_IF_ERROR_RETURN(soc_mac_mode_get(unit, port, &mode));

    if ((mode == SOC_MAC_MODE_10_100) || (mode == SOC_MAC_MODE_10)) {
        return (mac_fe_duplex_set(unit, port, duplex));
    }
    else {
        uint32 gmacc1, ogmacc1;

        SOC_IF_ERROR_RETURN(READ_GMACC1r(unit, port, &gmacc1));
        ogmacc1 = gmacc1;
        gmacc1 |= (duplex ? 1 : 0);
        /*        soc_reg64_field32_set(unit, GMACC1r, &gmacc1, FULLDf, duplex ? 1 : 0);*/
        if ( gmacc1 != ogmacc1 ) {
            SOC_IF_ERROR_RETURN(WRITE_GMACC1r(unit, port, gmacc1));
        }

        /* Set IPG to match new duplex */
        SOC_IF_ERROR_RETURN(mac_ge_ipg_update(unit, port));
    }

    return SOC_E_NONE;
}


static int
mac_fe_pause_get(int unit, soc_port_t port, int *pause_tx, int *pause_rx)
{
    uint32      fe_mac1;

    SOC_IF_ERROR_RETURN(READ_FE_MAC1r(unit, port, &fe_mac1));

    *pause_rx = (fe_mac1 >> 2) & 1;
    *pause_tx = (fe_mac1 >> 3) & 1;

    return (SOC_E_NONE);
}


static int
mac_fe_pause_set(int unit, soc_port_t port, int pause_tx, int pause_rx)
{
    uint32      fe_mac1;

    if (pause_tx < 0 && pause_rx < 0)
        return SOC_E_NONE;

    SOC_IF_ERROR_RETURN(READ_FE_MAC1r(unit, port, &fe_mac1));

    fe_mac1 |= (pause_rx ? (1 << 2) : 0);

    fe_mac1 |= (pause_tx ? (1 << 3) : 0);

    SOC_IF_ERROR_RETURN(WRITE_FE_MAC1r(unit, port, fe_mac1));

    return SOC_E_NONE;
}


static int
mac_ge_pause_get(int unit, soc_port_t port, int *pause_tx, int *pause_rx)
{
    soc_mac_mode_t      mode;

    SOC_IF_ERROR_RETURN(soc_mac_mode_get(unit, port, &mode));

    if ((mode == SOC_MAC_MODE_10_100) || (mode == SOC_MAC_MODE_10)) {
        return (mac_fe_pause_get(unit, port, pause_tx, pause_rx));
    } else { /* mac in gig mode */
        uint32          gmacc1;

        SOC_IF_ERROR_RETURN(READ_GMACC1r(unit, port, &gmacc1));

        *pause_tx = (gmacc1 >> 30) & 1;
        *pause_rx = (gmacc1 >> 28) & 1;

        return (SOC_E_NONE);
    }
}


static int
mac_ge_pause_set(int unit, soc_port_t port, int pause_tx, int pause_rx)
{
    soc_mac_mode_t      mode;

    if (pause_tx < 0 && pause_rx < 0) {
        return SOC_E_NONE;
    }

    SOC_IF_ERROR_RETURN(soc_mac_mode_get(unit, port, &mode));

    if ((mode == SOC_MAC_MODE_10_100) || (mode == SOC_MAC_MODE_10)) {
        return (mac_fe_pause_set(unit, port, pause_tx, pause_rx));
    }
    else {
        uint32          gmacc1, ogmacc1;

        SOC_IF_ERROR_RETURN(READ_GMACC1r(unit, port, &gmacc1));
        ogmacc1 = gmacc1;

        gmacc1 |= (pause_tx ? (1<<30) : 0);

        gmacc1 |= (pause_rx ? (1<<28) : 0);

        if ( gmacc1 != ogmacc1 )
            SOC_IF_ERROR_RETURN(WRITE_GMACC1r(unit, port, gmacc1));

        return SOC_E_NONE;
    }
}


static int
mac_fe_pause_addr_get(int unit, soc_port_t port, sal_mac_addr_t pause_mac)
{
    uint32      esa0, esa1, esa2;

    SOC_IF_ERROR_RETURN(READ_ESA0r(unit, port, &esa0));
    SOC_IF_ERROR_RETURN(READ_ESA1r(unit, port, &esa1));
    SOC_IF_ERROR_RETURN(READ_ESA2r(unit, port, &esa2));

    pause_mac[0] = (uint8)(esa2 >> 8);
    pause_mac[1] = (uint8)(esa2 >> 0);
    pause_mac[2] = (uint8)(esa1 >> 8);
    pause_mac[3] = (uint8)(esa1 >> 0);
    pause_mac[4] = (uint8)(esa0  >> 8);
    pause_mac[5] = (uint8)(esa0  >> 0);

    return (SOC_E_NONE);
}


static int
mac_ge_pause_addr_get(int unit, soc_port_t port, sal_mac_addr_t pause_mac)
{
    soc_mac_mode_t      mode;

    SOC_IF_ERROR_RETURN(soc_mac_mode_get(unit, port, &mode));

    if ((mode == SOC_MAC_MODE_10_100) || (mode == SOC_MAC_MODE_10)) {
        return (mac_fe_pause_addr_get(unit, port, pause_mac));
    } else { /* mac in gig mode */
        uint32 gsa0, gsa1;

        SOC_IF_ERROR_RETURN(READ_GSA0r(unit, port, &gsa0));
        SOC_IF_ERROR_RETURN(READ_GSA1r(unit, port, &gsa1));

        pause_mac[0] = (uint8)(gsa0 >> 24);
        pause_mac[1] = (uint8)(gsa0 >> 16);
        pause_mac[2] = (uint8)(gsa0 >> 8);
        pause_mac[3] = (uint8)(gsa0 >> 0);
        pause_mac[4] = (uint8)(gsa1 >> 24);
        pause_mac[5] = (uint8)(gsa1 >> 16);

        return (SOC_E_NONE);
    }
}


static int
mac_fe_pause_addr_set(int unit, soc_port_t port, sal_mac_addr_t pause_mac)
{
    uint32      esa0, esa1, esa2;

    esa0  = (pause_mac[4] << 8) | pause_mac[5];
    esa1  = (pause_mac[2] << 8) | pause_mac[3];
    esa2  = (pause_mac[0] << 8) | pause_mac[1];

    SOC_IF_ERROR_RETURN(WRITE_ESA0r(unit, port, esa0));
    SOC_IF_ERROR_RETURN(WRITE_ESA1r(unit, port, esa1));
    SOC_IF_ERROR_RETURN(WRITE_ESA2r(unit, port, esa2));

    return (SOC_E_NONE);
}


static int
mac_ge_pause_addr_set(int unit, soc_port_t port, sal_mac_addr_t pause_mac)
{
    soc_mac_mode_t      mode;

    SOC_IF_ERROR_RETURN(soc_mac_mode_get(unit, port, &mode));

    if ((mode == SOC_MAC_MODE_10_100) || (mode == SOC_MAC_MODE_10)) {
        return (mac_fe_pause_addr_set(unit, port, pause_mac));
    } else { /* mac in gig mode */
        uint32  gsa0, gsa1;

        gsa0 = pause_mac[0] << 24 |
            pause_mac[1] << 16 |
            pause_mac[2] << 8 |
            pause_mac[3] << 0;

        gsa1 = ((pause_mac[4] << 8) | pause_mac[5]) << 16;

        SOC_IF_ERROR_RETURN(WRITE_GSA0r(unit, port, gsa0));
        SOC_IF_ERROR_RETURN(WRITE_GSA1r(unit, port, gsa1));
    }

    return (SOC_E_NONE);
}


static int
soc_autoz_get(int unit, soc_port_t port, int *enable)
{
    /* XGS (Hercules): AutoZ not supported */
    *enable = 0;
    return SOC_E_NONE;
}


static int
soc_autoz_set(int unit, soc_port_t port, int enable)
{
    return SOC_E_NONE;
}


static int
mac_fe_loopback_get(int unit, soc_port_t port, int *loopback)
{
    int         rv;
    uint32      fe_mac1;

    if ((rv = READ_FE_MAC1r(unit, port, &fe_mac1)) == SOC_E_NONE) {
        *loopback = (fe_mac1 >> 4) & 1;
    }

    return(rv);
}


static int
mac_fe_loopback_set(int unit, soc_port_t port, int loopback)
{
    uint32      fe_mac1;

    SOC_IF_ERROR_RETURN(READ_FE_MAC1r(unit, port, &fe_mac1));
    fe_mac1 &= ~(1<<4);
    fe_mac1 |= ((loopback&1)<<4);
    SOC_IF_ERROR_RETURN(WRITE_FE_MAC1r(unit, port, fe_mac1));

    return (SOC_E_NONE);
}


static int
mac_ge_loopback_get(int unit, soc_port_t port, int *loopback)
{
    soc_mac_mode_t      mode;

    SOC_IF_ERROR_RETURN(soc_mac_mode_get(unit, port, &mode));

    if ((mode == SOC_MAC_MODE_10_100) || (mode == SOC_MAC_MODE_10)) {
        return mac_fe_loopback_get(unit, port, loopback);
    } else { /* mac in gig mode */
        uint32 gmacc0;
        SOC_IF_ERROR_RETURN(READ_GMACC0r(unit, port, &gmacc0));
        *loopback = (gmacc0 >> 8) & 1;

        return (SOC_E_NONE);
    }
}


static int
mac_ge_loopback_set(int unit, soc_port_t port, int loopback)
{
    soc_mac_mode_t      mode;

    SOC_IF_ERROR_RETURN(soc_mac_mode_get(unit, port, &mode));

    if ((mode == SOC_MAC_MODE_10_100) || (mode == SOC_MAC_MODE_10)) {
        return mac_fe_loopback_set(unit, port, loopback);
    }
    else {
        uint32 gmacc0, ogmacc0;

        /* L10B also works, but not on Draco so we'll use L32B */

        SOC_IF_ERROR_RETURN(READ_GMACC0r(unit, port, &gmacc0));
        ogmacc0 = gmacc0;
        gmacc0 |= (loopback ? (1<<8) : 0);
        if ( ogmacc0 != gmacc0 ) {
            SOC_IF_ERROR_RETURN(WRITE_GMACC0r(unit, port, gmacc0));
        }

        return (SOC_E_NONE);
    }
}


static int
_soc_cell_count(int unit, soc_port_t port, uint32 *count)
{
    uint32              val;
    int                 cos;

    *count = 0;

    for (cos = 0; cos < 8; cos++) {
        SOC_IF_ERROR_RETURN(READ_COSLCCOUNTr(unit, port, cos, &val));
        *count += val;
    }

    return SOC_E_NONE;
}


#define soc_link_mask2_get(u,m) (*(m)=myLinkMask2[u])
#define soc_link_mask2_set(u,m) (myLinkMask2[u]=(m),0)

static int myLinkMask2[MAX_DEVICES];
/*int myLinkMask[MAX_DEVICES];*/

int soc_drain_cells(int unit, soc_port_t port)
{
    soc_timeout_t       to;
    int                 rv, timeout_usec, loopback = -1;
    uint32              cur_cells, new_cells;
    soc_mac_mode_t  mode;
    uint32  pkt_drop_ctrl;
    int pause_tx, pause_rx;

    timeout_usec = 250000;

    SOC_IF_ERROR_RETURN
        (mac_ge_pause_get(unit, port, &pause_tx, &pause_rx));
    SOC_IF_ERROR_RETURN
        (mac_ge_pause_set(unit, port, 0, 0));
    SOC_IF_ERROR_RETURN
        (READ_GE_EGR_PKT_DROP_CTLr(unit, port, &pkt_drop_ctrl));
    pkt_drop_ctrl &= ~1;
    SOC_IF_ERROR_RETURN
        (WRITE_GE_EGR_PKT_DROP_CTLr(unit, port, pkt_drop_ctrl));

    if (soc_mac_mode_get(unit, port, &mode) < 0) {
        mode = SOC_MAC_MODE_10_100;
    }

    cur_cells = 0xffffffff;

    for (;;) {
        if ((rv = _soc_cell_count(unit, port, &new_cells)) < 0) {
            break;
        }

        if (new_cells == 0) {
            rv = SOC_E_NONE;
            break;
        }

        /* gig mac in pause state needs loopback mode to clear xoff */
        if (new_cells == cur_cells && loopback < 0 &&
            mode == SOC_MAC_MODE_1000_T) {
            if (mac_ge_loopback_get(unit, port, &loopback) < 0) {
                loopback = -1;
            }
            else {
                mac_ge_loopback_set(unit, port, 1);
            }
        }

        if (new_cells < cur_cells) { /* Progress made */
            soc_timeout_init(&to, timeout_usec, 0); /* Restart timeout */
            cur_cells = new_cells;
        }

        if (soc_timeout_check(&to)) {
            if ((rv = _soc_cell_count(unit, port, &new_cells)) < 0) {
                break;
            }

            PRINTF_ERROR(("ERROR: port %d:%d: "
                "timeout draining packets (%d cells remain)\n",
                unit, port, new_cells));
            rv = SOC_E_INTERNAL;
            break;
        }
    }

    if (loopback >= 0) {
        SOC_IF_ERROR_RETURN
            (mac_ge_loopback_set(unit, port, loopback));
    }

    SOC_IF_ERROR_RETURN
        (WRITE_GE_EGR_PKT_DROP_CTLr(unit, port, pkt_drop_ctrl));
    SOC_IF_ERROR_RETURN
        (mac_ge_pause_set(unit, port, pause_tx, pause_rx));

    return rv;
}


static int
mac_fe_enable_set(int unit, soc_port_t port, int enable)
{
    uint32              regval;
    pbmp_t              mask;

    if (enable) {
        WRITE_FE_MAC2r(unit, port, 0x4001);
        WRITE_FE_MAC1r(unit, port, 0x800c);
    } else {
        WRITE_FE_MAC1r(unit, port, 0x8000);
    }
    return SOC_E_NONE;
}


static int
mac_ge_enable_get(int unit, soc_port_t port, int *enable)
{
    soc_mac_mode_t      mode;

    SOC_IF_ERROR_RETURN(soc_mac_mode_get(unit, port, &mode));

    if ((mode == SOC_MAC_MODE_10_100) || (mode == SOC_MAC_MODE_10)) {
        return (mac_fe_enable_get(unit, port, enable));
    }
    else {
        uint32          gmacc1;
        SOC_IF_ERROR_RETURN(READ_GMACC1r(unit, port, &gmacc1));
        *enable = (gmacc1 >> 28) & 1;
    }

    return SOC_E_NONE;
}


int
mac_ge_enable_set(int unit, soc_port_t port, int enable)
{
    uint32              gmacc1, ogmacc1;
    soc_mac_mode_t      mode;
    pbmp_t              mask;

    SOC_IF_ERROR_RETURN(soc_mac_mode_get(unit, port, &mode));

    if ((mode == SOC_MAC_MODE_10_100) || (mode == SOC_MAC_MODE_10)) {
        return (mac_fe_enable_set(unit, port, enable));
    }

    /*
     * Turn on/off receive enable.
     *
     * We do not bother then TXEN because on some chips this signal does
     * not take effect (get latched) until traffic stops flowing.
     */

    SOC_IF_ERROR_RETURN(READ_GMACC1r(unit, port, &gmacc1));
    ogmacc1 = gmacc1;
    if (enable) {
        gmacc1 |= (1<<28) | (1<<30);
    } else { /* Turn off Receive Enable */
        gmacc1 &= ~(1<<28);
    }
    if ( ogmacc1 != gmacc1 ) {
        SOC_IF_ERROR_RETURN(WRITE_GMACC1r(unit, port, gmacc1));
    }

    /*
     * Use EPC_LINK to control other ports sending to this port.  This
     * blocking is independent and in addition to what Linkscan does.
     *
     * Single-step mode is used to stop traffic from going from the
     * egress to the MAC, except on Strata GE where drain is used.
     */

    if (!enable) {
        soc_link_mask2_get(unit, &mask);
        SOC_PBMP_PORT_REMOVE(mask, port);
        SOC_IF_ERROR_RETURN(soc_link_mask2_set(unit, mask));

        SOC_IF_ERROR_RETURN(soc_drain_cells(unit, port));

    }
    else {
        soc_link_mask2_get(unit, &mask);
        SOC_PBMP_PORT_ADD(mask, port);
        SOC_IF_ERROR_RETURN(soc_link_mask2_set(unit, mask));
    }

    return SOC_E_NONE;
}


int
soc_mac_mode_set(int unit, soc_port_t port, soc_mac_mode_t new_mode)
{
    soc_mac_mode_t      old_mode;
    int                 old_mac_enable;
    int                 save_duplex;
    int                 save_autoz;      /* AutoZ mode */
    sal_mac_addr_t      save_pause_mac;  /* Pause MAC address */
    int                 save_pause_tx;   /* Pause Tx enable status */
    int                 save_pause_rx;   /* Pause Rx enable status */
    int                 save_lb;         /* Mac LB enable status */

    PRINTF_DEBUG2(("soc_mac_mode_set: reconfiguring port=%d mode=%d\n",
        port, new_mode));

    /*
     * At this point, we know the request is valid, and that we *MAY* need
     * to take action. Cases such as setting mac on 10/100 port only OR
     * 10G on HG port are already handled.
     */

    SOC_IF_ERROR_RETURN(soc_mac_mode_get(unit, port, &old_mode));

    if (new_mode == old_mode) { /* Already in requested mode */
        return SOC_E_NONE;
    }

    /*
     * Save some state from the current MAC that must be restored in the
     * new MAC after the mode is switched.
     */

    SOC_IF_ERROR_RETURN
        (mac_ge_duplex_get(unit, port, &save_duplex));

    SOC_IF_ERROR_RETURN
        (mac_ge_pause_get(unit, port, &save_pause_tx, &save_pause_rx));

    SOC_IF_ERROR_RETURN
        (mac_ge_pause_addr_get(unit, port, save_pause_mac));

    SOC_IF_ERROR_RETURN
        (soc_autoz_get(unit, port, &save_autoz));

    SOC_IF_ERROR_RETURN
        (mac_ge_loopback_get(unit, port, &save_lb));

    /*
     * The currently selected MAC is quiesced before switching it out.
     * It must not be talking to the MMU when the switch occurs.  For
     * receive, RX is disabled.  For transmit, Egress is disabled.
     */

    SOC_IF_ERROR_RETURN
        (mac_ge_enable_get(unit, port, &old_mac_enable));
    SOC_IF_ERROR_RETURN
        (mac_ge_enable_set(unit, port, FALSE));

    switch (new_mode) {
        case SOC_MAC_MODE_10:
        case SOC_MAC_MODE_10_100:
            PRINTF_DEBUG2(("soc_mac_mode_set: port=%d, 10/100 mode\n", port));
            if (new_mode == SOC_MAC_MODE_10) {
                _port_sp_sel_set(unit, port, 2);
                bcm_port_jam_set(unit, port, 1);
                soc_miim_write(unit, port+128, 0, 0x0000);
            }
            else {
                _port_sp_sel_set(unit, port, 1);
                bcm_port_jam_set(unit, port, 1);
                soc_miim_write(unit, port+128, 0, 0x2000);
            }
            break;
        case SOC_MAC_MODE_1000_T:
            PRINTF_DEBUG2(("soc_mac_mode_set: port=%d, Gig mode\n", port));
            _port_sp_sel_set(unit, port, 0);
            soc_miim_write(unit,port+128,0,0x140);
            break;
        default:
            return SOC_E_INTERNAL;
    }

    /*
     * Restore the saved state in the new MAC.
     */

    SOC_IF_ERROR_RETURN
        (mac_ge_duplex_set(unit, port, save_duplex));

    SOC_IF_ERROR_RETURN
        (mac_ge_pause_set(unit, port, save_pause_tx, save_pause_rx));

    SOC_IF_ERROR_RETURN
        (mac_ge_pause_addr_set(unit, port, save_pause_mac));

    SOC_IF_ERROR_RETURN
        (soc_autoz_set(unit, port, save_autoz));

    SOC_IF_ERROR_RETURN
        (mac_ge_loopback_set(unit, port, save_lb));

    /*
     * Enable the egress and MAC, if previously enabled.
     */
    if (old_mac_enable) {
        if (new_mode == SOC_MAC_MODE_1000_T) {
            SOC_IF_ERROR_RETURN(mac_ge_enable_set(unit, port, TRUE));
        }
        else {
            SOC_IF_ERROR_RETURN(mac_fe_enable_set(unit, port, TRUE));
        }
    }

    return SOC_E_NONE;
}


int
mac_fe_init(int unit, soc_port_t port)
{
    uint32 val32;

    /* FE_MAC2r */
    SOC_IF_ERROR_RETURN(READ_FE_MAC2r(unit, port, &val32));
    val32 = 0x4000;
    SOC_IF_ERROR_RETURN(WRITE_FE_MAC2r(unit, port, val32));

    /* FE_CLRTr
    SOC_IF_ERROR_RETURN(READ_FE_CLRTr(unit, port, &val32));
    val32 = 0x370f ;
    SOC_IF_ERROR_RETURN(WRITE_FE_CLRTr(unit, port, val32));
    */
    /* FE_IPGRr */
    SOC_IF_ERROR_RETURN(READ_FE_IPGRr(unit, port, &val32));
    val32 = 0xc12;
    SOC_IF_ERROR_RETURN(WRITE_FE_IPGRr(unit, port, val32));

    /* FE_SUPPr */
    SOC_IF_ERROR_RETURN(READ_FE_SUPPr(unit, port, &val32));
    val32 = 0x1100;
    SOC_IF_ERROR_RETURN(WRITE_FE_SUPPr(unit, port, val32));

    SOC_IF_ERROR_RETURN(READ_FE_MAXFr(unit, port, &val32));
    /* BSP_Printf("FE MAX frame:0x%x\r\n",val32); */
    val32 = 0x05ee + 4;
    SOC_IF_ERROR_RETURN(WRITE_FE_MAXFr(unit, port, val32));

    /* FE_MAC1r
    SOC_IF_ERROR_RETURN(READ_FE_MAC1r(unit, port, &val32));
    val32 = 0;
    SOC_IF_ERROR_RETURN(WRITE_FE_MAC1r(unit, port, val32));
    */
    val32 = 0;
    /* FE_ESA0r */
    SOC_IF_ERROR_RETURN(WRITE_ESA0r(unit, port, val32));

    /* FE_ESA1r */
    SOC_IF_ERROR_RETURN(WRITE_ESA1r(unit, port, val32));

    /* FE_ESA2r */
    SOC_IF_ERROR_RETURN(WRITE_ESA2r(unit, port, val32));

    /* Set IPG to match initial operating mode */
    SOC_IF_ERROR_RETURN(mac_fe_ipg_update(unit, port));

    /* Set egress enable */
    SOC_IF_ERROR_RETURN(READ_EGR_ENABLEr(unit, port, &val32));
    val32 |= 1;
    SOC_IF_ERROR_RETURN(WRITE_EGR_ENABLEr(unit, port, val32));

    return SOC_E_NONE;
}


int
mac_56304_ge_init(int unit, soc_port_t port)
{
    uint32              gmacc0, gmacc1, gmacc2;
    uint32              ogmacc0, ogmacc1, ogmacc2;
    uint32              val32;

    /*
     * Initialize 10/100MB/s MAC if present
     */

    SOC_IF_ERROR_RETURN(soc_mac_mode_set(unit, port, SOC_MAC_MODE_10_100));
    SOC_IF_ERROR_RETURN(mac_fe_init(unit, port));

    /*
     * Initialize 1000Mb/s MAC
     */

    SOC_IF_ERROR_RETURN(soc_mac_mode_set(unit, port, SOC_MAC_MODE_1000_T));

    SOC_IF_ERROR_RETURN(READ_GMACC0r(unit, port, &gmacc0));
    SOC_IF_ERROR_RETURN(READ_GMACC1r(unit, port, &gmacc1));
    SOC_IF_ERROR_RETURN(READ_GMACC2r(unit, port, &gmacc2));
    ogmacc0 = gmacc0;
    ogmacc1 = gmacc1;
    ogmacc2 = gmacc2;

    gmacc0 = 0;
    gmacc1 = 0xA1681;
    gmacc2 = 0xC;

    {
        uint32 pctrl;
        pctrl = 0x3FFFF;
        SOC_IF_ERROR_RETURN(WRITE_PAUSE_CONTROLr(unit, port, pctrl));
    }

    SOC_IF_ERROR_RETURN(WRITE_TEST2r(unit, port, 0));

    if ( gmacc2 != ogmacc2 ) {
        SOC_IF_ERROR_RETURN(WRITE_GMACC2r(unit, port, gmacc2));
    }

    if ( gmacc1 != ogmacc1 ) {
        SOC_IF_ERROR_RETURN(WRITE_GMACC1r(unit, port, gmacc1));
    }

    if ( gmacc0 != ogmacc0 ) {
        SOC_IF_ERROR_RETURN(WRITE_GMACC0r(unit, port, gmacc0));
    }

    /* Clear station address */
    SOC_IF_ERROR_RETURN(WRITE_GSA0r(unit, port, 0));
    SOC_IF_ERROR_RETURN(WRITE_GSA1r(unit, port, 0));

    SOC_IF_ERROR_RETURN(WRITE_MAXFRr(unit, port, 0x5f2));
    /* Set IPG to match initial operating mode */
    SOC_IF_ERROR_RETURN(mac_ge_ipg_update(unit, port));

    SOC_IF_ERROR_RETURN(WRITE_FE_IPGRr(unit,port, ((0x6 << 8) | 0xf)));

    /* Set egress enable */
    SOC_IF_ERROR_RETURN(READ_EGR_ENABLEr(unit, port, &val32));
    val32 |= 1;
    SOC_IF_ERROR_RETURN(WRITE_EGR_ENABLEr(unit, port, val32));

    /*
     * Firebolt ports are initialized in bcm_port_init() so as to
     * block control frames (PORT_TABm.PASS_CONTROL_FRAMESf = 0).
     */

    return SOC_E_NONE;
}


int
phy_5464_interface_set(int unit, soc_port_t port, soc_port_if_t pif)
{
    return SOC_E_NONE;
}


int
mac_ge_interface_set(int unit, soc_port_t port, soc_port_if_t pif)
{
    uint32              gmacc0, gpcsc;
    uint32              ogmacc0, ogpcsc;

    SOC_IF_ERROR_RETURN(READ_GMACC0r(unit, port, &gmacc0));
    SOC_IF_ERROR_RETURN(READ_GPCSCr(unit, port, &gpcsc));
    ogmacc0 = gmacc0;
    ogpcsc = gpcsc;

    gmacc0 &= ~(0xf);
    gmacc0 |= 1;
    gpcsc  |= 0x4;

    if ( gpcsc != ogpcsc ) {
        SOC_IF_ERROR_RETURN(WRITE_GPCSCr(unit, port, gpcsc));
    }
    if ( gmacc0 != ogmacc0 ) {
        SOC_IF_ERROR_RETURN(WRITE_GMACC0r(unit, port, gmacc0));
    }
    return SOC_E_NONE;
}


int
phy_5464_enable_set(int unit, soc_port_t port, int enable)
{

    return SOC_E_NONE;
}


int
phy_fe_ge_link_get(int unit, soc_port_t port, int *link)
{
    uint8       phy_addr = PORT_TO_PHY_ADDR(unit, port);
    uint16      mii_ctrl, mii_stat;
    soc_timeout_t   to;

    *link = FALSE;   /* Default */

    SOC_IF_ERROR_RETURN
        (soc_miim_read(unit, phy_addr, MII_STAT_REG, &mii_stat));

    if (!(mii_stat & MII_STAT_LA)) {
        return SOC_E_NONE;
    }

    /* Link appears to be up; we are done if autoneg is off. */
    SOC_IF_ERROR_RETURN
        (soc_miim_read(unit, phy_addr, MII_CTRL_REG, &mii_ctrl));

    if (!(mii_ctrl & MII_CTRL_AE)) {
        *link = TRUE;
        return SOC_E_NONE;
    }

    /*
     * If link appears to be up but autonegotiation is still in
     * progress, wait for it to complete.  For BCM5228, autoneg can
     * still be busy up to about 200 usec after link is indicated.  Also
     * continue to check link state in case it goes back down.
     */

    soc_timeout_init(&to, 250000, 0);

    for (;;) {
        SOC_IF_ERROR_RETURN
            (soc_miim_read(unit, phy_addr, MII_STAT_REG, &mii_stat));

        if (!(mii_stat & MII_STAT_LA)) {
            return SOC_E_NONE;
        }

        if (mii_stat & MII_STAT_AN_DONE) {
            break;
        }

        if (soc_timeout_check(&to)) {
            return SOC_E_BUSY;
        }
    }

    /* Return link state at end of polling */

    *link = ((mii_stat & MII_STAT_LA) != 0);

    return SOC_E_NONE;
}

