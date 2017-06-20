/*
 * $Id: systemInit.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef SYSTEMINIT_H
#define SYSTEMINIT_H

typedef int bcm_port_t;
typedef int soc_port_t;
typedef int soc_port_if_t;

/************************************************************/
/*************************** constants **********************/
/************************************************************/
#define MAX_DEVICES 4
#define BCM_MAX_NUM_UNITS MAX_DEVICES

#define SOC_E_NONE       0
#define SOC_E_INTERNAL  -1
#define SOC_E_PARAM     -4
#define SOC_E_FULL      -6
#define SOC_E_TIMEOUT   -9
#define SOC_E_BUSY      -10
#define SOC_E_FAIL      -11
#define SOC_E_CONFIG    -15
#define SOC_E_RESOURCE  -14 
#define BCM_E_NONE       0
#define BCM_E_BUSY      -10
#define BCM_E_MEMORY    -2
#define BCM_E_PARAM     -4

#define HE_SOC_BLOCK2SCH_CMIC_BLOCK        0

/* Some basic definitions */
#define SOC_BLOCK_BP                    20
#define SOC_MEMOFS_BP                   16
#define SOC_REGIDX_BP                   12

#define FB_NUM_COS  8
#  define u64_MSW	0
#  define u64_LSW	1

#define PHY_RESET_TIMEOUT_USEC  100000

/**********************************************************************/
/*********************************** macros ***************************/
/**********************************************************************/

#define PRINTF_PCI(fmt)        
#define PRINTF_PCI2(fmt)       

#define PRINTF_DEBUG0(fmt)
#define PRINTF_DEBUG(fmt) 
#define PRINTF_DEBUG1(fmt)
#define PRINTF_ERROR(fmt)    printf fmt
#define PRINTF_DEBUG2(fmt)


#define SOC_IF_ERROR_RETURN(op) \
    do { int __rv__; if ((__rv__ = (op)) < 0) \
	{ PRINTF_ERROR((__FILE__ " : " #op " %d\n", __LINE__)); \
	return(__rv__);} } \
    while(0)
#define BCM_IF_ERROR_RETURN(op)  SOC_IF_ERROR_RETURN(op)

#ifdef ICS
#define	CMREAD(_d,_a)	 \
        (((volatile uint32 *)(ICS_CMIC_BASE_ADDR))[(_a)>>2])
#define	CMWRITE(_d,_a,_data)	\
	(CMREAD(_d,_a) = _data)
#else
#define	CMREAD(_d,_a)	 \
        (((volatile uint32 *)(PCI_SOC_MBAR0 + (PCI_SOC_MEM_WINSZ * (_d))))[(_a)>>2])
#define	CMWRITE(_d,_a,_data)	\
	(CMREAD(_d,_a) = _data)
#endif /* ICS */

#define u64_H(v) (((uint32 *) &(v))[u64_MSW])
#define u64_L(v) (((uint32 *) &(v))[u64_LSW])

#define HE__PORT_ITER(u,_p,m, m1)  \
        for ((_p) = 0; (_p) <= 53; (_p)++) \
                if (((_p) < 32) ? ((1 << (_p)) & (m)) : ((1 << ((_p) - 32)) & (m1)))

/**********************************************************************/
/******************************************************& basic types **/
/**********************************************************************/
typedef signed char        int8;
typedef signed short       int16;
typedef signed int         int32;

typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;

/**********************************************************************/
/************************************************************* types **/
/**********************************************************************/
typedef uint32  sal_paddr_t;		/* Physical address (PCI address) */
typedef uint32  sal_vaddr_t;		/* Virtual address (Host address) */
typedef uint32 pbmp_t;
typedef pbmp_t bcm_pbmp_t;

typedef struct vxbde_bus_s {
    uint32 base_addr_start;
    int int_line;
    int be_pio;
    int be_packet;
    int be_other;
} vxbde_bus_t;

typedef struct ibde_dev_s
{
    uint16 device;
    uint8 rev;
    uint32 base_address;
} ibde_dev_t;

typedef struct pci_dev_s {
    int		busNo;
    int		devNo;
    int		funcNo;
} pci_dev_t;

typedef struct vxbde_dev_s {
  ibde_dev_t bde_dev;
  pci_dev_t  pci_dev;
  int iLine;
  int iPin;
} vxbde_dev_t;

typedef unsigned long sal_time_t;
typedef uint32 sal_usecs_t;

typedef struct soc_timeout_s {
    sal_usecs_t		expire;
    sal_usecs_t		usec;
    int			min_polls;
    int			polls;
    int			exp_delay;
} soc_timeout_t;

typedef int (*soc_mem_cmp_t)(int, void *, void *);

typedef uint8   sal_mac_addr_t[6];      /* MAC address */
typedef uint8   bcm_mac_t[6];
typedef uint32  sal_ip_addr_t;          /* IP Address */

#define SAL_USECS_SUB(_t2, _t1)         (((int) ((_t2) - (_t1))))
#define SAL_USECS_ADD(_t, _us)          ((_t) + (_us))

/************************************************************/
/*************************** functions **********************/
/************************************************************/
void sal_usleep(uint32 usec);

void soc_timeout_init(soc_timeout_t *to, sal_usecs_t usec, int min_polls);
int soc_timeout_check(soc_timeout_t *to);
int soc_reg32_write(int unit, uint32 addr, uint32 data);
int soc_reg32_read(int unit, uint32 addr, uint32 *data);

int pci_config_putw(pci_dev_t *dev, uint32 addr, uint32 data);
uint32 pci_config_getw(pci_dev_t *dev, uint32 addr);
uint32 soc_intr_enable(int unit, uint32 mask);
uint32 soc_intr_disable(int unit, uint32 mask);


#ifdef  SOC_PCI_DEBUG
extern int soc_pci_getreg(int unit, uint32 addr, uint32 *data_ptr);
extern uint32 soc_pci_read(int unit, uint32 addr);
extern int soc_pci_write(int unit, uint32 addr, uint32 data);
#else
#define soc_pci_getreg(unit, addr, datap) \
        (*datap = CMREAD(unit, addr), SOC_E_NONE)
#define soc_pci_read(unit, addr)        CMREAD(unit, addr)
#define soc_pci_write(unit, addr, data) CMWRITE(unit, addr, data)
#endif  /* SOC_PCI_DEBUG */

#define MII_STAT_LA             (1 << 2) /* Link Active */
#define MII_STAT_AN_DONE        (1 << 5) /* Autoneg complete */

#endif
