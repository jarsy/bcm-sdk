/*
 * $Id: robo_drv.h,v 1.13 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * 
 */
   

#ifndef _SOC_ROBO_DRV_H
#define _SOC_ROBO_DRV_H

typedef enum soc_robo_rev_id_e {
	SocRoboRevA,
	SocRoboRevB,
	SocRoboRevC
}soc_robo_rev_id_t;


typedef enum soc_robo_arch_type_e {
	SOC_ROBO_ARCH_FEX,
	SOC_ROBO_ARCH_TBX,
	SOC_ROBO_ARCH_VULCAN,
	SOC_ROBO_ARCH_DINO
}soc_robo_arch_type_t;

typedef enum soc_robo_chip_type_e {
    SOC_ROBO_CHIP_53242,
    SOC_ROBO_CHIP_53262,
    SOC_ROBO_CHIP_53115,
    SOC_ROBO_CHIP_53118,
    SOC_ROBO_CHIP_53280,
    SOC_ROBO_CHIP_53101,
    SOC_ROBO_CHIP_53125,
    SOC_ROBO_CHIP_53128,
    SOC_ROBO_CHIP_53600,
    SOC_ROBO_CHIP_89500,
    SOC_ROBO_CHIP_53010,
    SOC_ROBO_CHIP_5389,
    SOC_ROBO_CHIP_53020,
    SOC_ROBO_CHIP_5396,
    SOC_ROBO_CHIP_53134,
}soc_robo_chip_type_t;


typedef struct soc_robo_control_s{
soc_robo_chip_type_t	chip_type;
soc_robo_rev_id_t	rev_id;
int			rev_num;
uint32			chip_bonding;
soc_robo_arch_type_t	arch_type;
}soc_robo_control_t;


/* Bonding option */
#define SOC_ROBO_BOND_M                     0x0001
#define SOC_ROBO_BOND_S                     0x0002
#define SOC_ROBO_BOND_V                     0x0004
#define SOC_ROBO_BOND_PHY_S3MII             0x0008
#define SOC_ROBO_BOND_PHY_COMBOSERDES       0x0010


/*
 * SOC_ROBO_* control driver macros
 */
#define SOC_ROBO_CONTROL(unit) \
        ((soc_robo_control_t *)(SOC_CONTROL(unit)->drv))

#define SOC_IS_ROBO_ARCH_FEX(unit) \
    (SOC_ROBO_CONTROL(unit) == NULL ? 0: \
    (SOC_ROBO_CONTROL(unit)->arch_type == SOC_ROBO_ARCH_FEX))

#define SOC_IS_ROBO_ARCH_GEX(unit) \
    (SOC_ROBO_CONTROL(unit) == NULL ? 0: \
    (SOC_ROBO_CONTROL(unit)->arch_type == SOC_ROBO_ARCH_GEX))

#ifdef BCM_TB_SUPPORT
#define SOC_IS_ROBO_ARCH_TBX(unit) \
    (SOC_ROBO_CONTROL(unit) == NULL ? 0: \
    (SOC_ROBO_CONTROL(unit)->arch_type == SOC_ROBO_ARCH_TBX))
#else
#define SOC_IS_ROBO_ARCH_TBX(unit) (0)
#endif

#define SOC_IS_ROBO_ARCH_VULCAN(unit) \
    (SOC_ROBO_CONTROL(unit) == NULL ? 0: \
    (SOC_ROBO_CONTROL(unit)->arch_type == SOC_ROBO_ARCH_VULCAN))

#define SOC_IS_ROBO_ARCH_DINO(unit) \
    (SOC_ROBO_CONTROL(unit) == NULL ? 0: \
    (SOC_ROBO_CONTROL(unit)->arch_type == SOC_ROBO_ARCH_DINO))

#ifdef BCM_TB_SUPPORT
#define SOC_IS_TB(unit)   \
    ((SOC_ROBO_CONTROL(unit) == NULL) ? 0: \
     (SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_53280))

#define SOC_IS_TB_BX(unit)   \
        ((SOC_ROBO_CONTROL(unit) == NULL) ? 0: \
        ((SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_53280) && \
        (SOC_ROBO_CONTROL(unit)->rev_id == SocRoboRevB) ))


#define SOC_IS_TB_B0(unit) SOC_IS_TB_BX(unit)

#define SOC_IS_TB_AX(unit) \
    (SOC_ROBO_CONTROL(unit) == NULL ? 0: \
        ((SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_53280) && \
        (SOC_ROBO_CONTROL(unit)->rev_id == SocRoboRevA) ))

#define SOC_IS_TBX(unit)   SOC_IS_ROBO_ARCH_TBX(unit)

#define SOC_IS_VO(unit) \
    (SOC_ROBO_CONTROL(unit) == NULL ? 0: \
    (SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_53600))

#else
#define SOC_IS_TB(unit)    (0)
#define SOC_IS_TB_BX(unit)   (0)
#define SOC_IS_TB_B0(unit)  (0)
#define SOC_IS_TB_AX(unit)  (0)
#define SOC_IS_TBX(unit)    (0)
#define SOC_IS_VO(unit) (0)

#endif

#define SOC_IS_ROBO53242(unit)   \
        ((SOC_ROBO_CONTROL(unit) == NULL) ? 0: \
         (SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_53242))
         
#define SOC_IS_ROBO53262(unit)   \
        ((SOC_ROBO_CONTROL(unit) == NULL) ? 0: \
         (SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_53262))

#define SOC_IS_HARRIER(unit)    \
        (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit))

#define SOC_IS_VULCAN(unit)   \
        ((SOC_ROBO_CONTROL(unit) == NULL) ? 0: \
         (SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_53115))

#define SOC_IS_BLACKBIRD(unit)   \
        ((SOC_ROBO_CONTROL(unit) == NULL) ? 0: \
         (SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_53118))
         
#define SOC_IS_LOTUS(unit)   \
        ((SOC_ROBO_CONTROL(unit) == NULL) ? 0: \
         (SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_53101))

#define SOC_IS_STARFIGHTER(unit)   \
        ((SOC_ROBO_CONTROL(unit) == NULL) ? 0: \
         (SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_53125))

#define SOC_IS_STARFIGHTER3(unit)   \
        ((SOC_ROBO_CONTROL(unit) == NULL) ? 0: \
         (SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_53134))

#define SOC_IS_STARFIGHTER3_A0(unit)   \
        ((SOC_ROBO_CONTROL(unit) == NULL) ? 0: \
         (SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_53134) && \
         (CMDEV(unit).dev.rev_id == SocRoboRevA))

#define SOC_IS_STARFIGHTER3_B0(unit)   \
        ((SOC_ROBO_CONTROL(unit) == NULL) ? 0: \
         (SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_53134) && \
         ((CMDEV(unit).dev.rev_id == SocRoboRevB) || \
         (CMDEV(unit).dev.rev_id == SocRoboRevC)))

#define SOC_IS_BLACKBIRD2(unit)   \
        ((SOC_ROBO_CONTROL(unit) == NULL) ? 0: \
         (SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_53128))

#define SOC_IS_BLACKBIRD2_BOND_V(unit)   \
        ((SOC_ROBO_CONTROL(unit) == NULL) ? 0: \
        ((SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_53128) && \
        (SOC_ROBO_CONTROL(unit)->chip_bonding == SOC_ROBO_BOND_V) ))


#define SOC_IS_POLAR(unit)   \
        ((SOC_ROBO_CONTROL(unit) == NULL) ? 0: \
         (SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_89500))

#define SOC_IS_NORTHSTAR(unit)   \
        ((SOC_ROBO_CONTROL(unit) == NULL) ? 0: \
         (SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_53010))

#define SOC_IS_DINO8(unit)   \
        ((SOC_ROBO_CONTROL(unit) == NULL) ? 0: \
         (SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_5389))

#define SOC_IS_NORTHSTARPLUS(unit)   \
        ((SOC_ROBO_CONTROL(unit) == NULL) ? 0: \
         (SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_53020))

#define SOC_IS_DINO16(unit)   \
        ((SOC_ROBO_CONTROL(unit) == NULL) ? 0: \
         (SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_5396))

#define SOC_IS_DINO(unit)   \
        ((SOC_ROBO_CONTROL(unit) == NULL) ? 0: \
        ((SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_5389) || \
        (SOC_ROBO_CONTROL(unit)->chip_type == SOC_ROBO_CHIP_5396)))

#define SOC_IS_GEX(unit)    \
    (SOC_IS_LOTUS(unit) || SOC_IS_VULCAN(unit) || \
    SOC_IS_BLACKBIRD(unit) || SOC_IS_STARFIGHTER(unit) || \
    SOC_IS_BLACKBIRD2(unit) || SOC_IS_POLAR(unit) || \
    SOC_IS_NORTHSTAR(unit) || SOC_IS_DINO8(unit) || \
    SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_DINO16(unit) || \
    SOC_IS_STARFIGHTER3(unit))

#ifdef  BCM_ROBO_SUPPORT
#define SOC_IS_ROBO_ARCH(unit) \
    (SOC_IS_ROBO_ARCH_TBX(unit) ||\
    SOC_IS_ROBO_ARCH_FEX(unit) ||\
    SOC_IS_ROBO_ARCH_VULCAN(unit) ||\
    SOC_IS_ROBO_ARCH_DINO(unit))
#else
#define SOC_IS_ROBO_ARCH(unit)  (0)
#endif

#define SOC_IS_ROBO_GE_SWITCH(unit) (SOC_IS_VULCAN(unit) || \
    SOC_IS_BLACKBIRD(unit) || SOC_IS_BLACKBIRD2(unit) || \
    SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) || \
    SOC_IS_NORTHSTAR(unit) || SOC_IS_DINO8(unit) || \
    SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_DINO16(unit) || \
    SOC_IS_STARFIGHTER3(unit))


#endif
