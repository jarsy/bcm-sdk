/*
 * OS Abstraction Layer for Switch Management API subsystem.
 * $Id: osl_vxworks.c,v 1.3 2007/10/08 22:31:08 iakramov Exp $
 * 
 *  Copyright 2002, Broadcom Corporation
 *  All Rights Reserved.
 *  
 *  This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 *  the contents of this file may not be disclosed to third parties, copied or
 *  duplicated in any form, in whole or in part, without the prior written
 *  permission of Broadcom Corporation.
 * 
 */
#include "vxWorks.h"			/* always first */
#include "config.h"			/* board support configuration header */
#include <if_robo.h>
#include <swapi.h>

/* Linux OS Specific interface */
int robo_fd = -1;
int robo_fd_instance = 0;
/* In swapi.c */
extern void *robo;
extern void *hSb;
extern robo_port_map_t robo_port_map[];
extern ROBO_MIB_AC_STRUCT port_stats[BCM_NUM_PORTS];
extern ROBO_MIB_AC_STRUCT mii_stats;
extern int mibac_count_port[BCM_NUM_PORTS];
extern int mibac_count_MII;
extern int bMIBAC;
IMPORT int 	ipAttach ();

#define ROBO_DEVICE "/dev/robo"

/*
 * Linux Semaphore API
 * this routine handles getting the semaphore for swapi functions
 * the kernel level routine actually gets a semaphore, but the user
 * level routine calls the robo driver (swmod.c), which gets the semaphore
 *
 * this attempts to get a semaphore, if can't there are 2 cases:
 * 1. if not in interrupt mode, just sleep 
 * 2. if in interrupt mode, do retry algorithm with exponential backoff
 *    using busywait.  note that user still may have to handle the failure
 *    case.
 */
int
bcm_get_sema(void)
{
    int retval = BCM_RET_SUCCESS;
    return retval;
}

/*
 * this routine handles releasing the semaphore for swapi functions
 * the kernel level routine actually releases a semaphore, but the user
 * level routine calls the robo driver (swmod.c), which releases the
 *  semaphore
 */
int
bcm_rel_sema(void)
{
    int retval = BCM_RET_SUCCESS;
    return retval;
}

/* 
 * Linux: Init API
 */
void *
bcm_api_init()
{
#ifdef __KERNEL__
#ifdef BCM5380
    if (!(hSb = sb_kattach()))
        return 0;
    if (!(robo = robosw_attach(hSb, (1 << 2), (1 << 3), (1 << 4), (1 << 5))))
        return 0;
#else
    if (!(robo = robosw_attach_pmii()))
        return 0;
#endif
    return robo;
#else
    /* for user mode, just open file handle */
    if (0 == robo_fd_instance)
    {
      robo_fd = open(ROBO_DEVICE, O_RDWR, 0);
    }
    robo_fd_instance++;
    return (void *)robo_fd;
#endif
}

/* 
 * Linux: Deinit API
 */
void
bcm_api_deinit()
{
#ifdef __KERNEL__
#ifdef BCM5380
  if (robo)
      robosw_detach(robo);
  if (hSb)
      sb_detach(hSb);
#else
  if (robo)
      robosw_detach_pmii(robo);
#endif
#else
  robo_fd_instance--;
  if (0 == robo_fd_instance)
  {
    if (-1 != robo_fd)
      close(robo_fd);
    robo_fd = -1;
  }
#endif
}
/*
 * Linux Write Reg (User/Kernel)
 */
int
bcms5_write_reg(int unit, uint8 page, uint8 addr, uint8 *buf, uint len)
{
    int retval = BCM_RET_SUCCESS;

    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    ROBO_WREG(unit,page,addr,buf,len);
    bcm_rel_sema();
    return retval;
}

/*
 * Linux Write Reg (no semaphore) (user/kernel)
 */
int
bcm_write_reg_no_sema(int unit, uint8 page, uint8 addr, uint8 *buf, uint len)
{
    int retval = BCM_RET_SUCCESS;

#ifdef __KERNEL__
    robo_driver->robo_write(robo,unit,page,addr,buf,len);
#else
    robo_io_t rd;

    memset(&rd,0,sizeof(rd));
    rd.page = page;
    rd.addr = addr;
    rd.write = 1;
    rd.unit = unit;
    rd.len = len;
    if (len > sizeof(rd.data))
        rd.data = (int)buf;
    else
        memcpy(&rd.data,buf,rd.len);
    if ((retval = ioctl(robo_fd, UBSEC_ROBOIF, &rd))) {
        printf("ioctl failed %x\n",retval);
    }
#endif
    return retval;
}

/*
 * Linux Read Reg (user/kernel)
 */
int
bcms5_read_reg(int unit, uint8 page, uint8 addr, uint8 *buf, uint len)
{
    int retval = BCM_RET_SUCCESS;

    if (BCM_RET_SUCCESS != (retval = bcm_get_sema()))
        return retval;
    ROBO_RREG(unit,page,addr,buf,len);
    bcm_rel_sema();
    return retval;
}

/*
 * Linux Read Reg (no semaphore) (user/kernel)
 */
int
bcm_read_reg_no_sema(int unit, uint8 page, uint8 addr, uint8 *buf, uint len)
{
    int retval = BCM_RET_SUCCESS;

#ifdef __KERNEL__
    robo_driver->robo_read(robo,unit,page,addr,buf,len);
#else
    robo_io_t rd;

    memset(&rd,0,sizeof(rd));
    rd.page = page;
    rd.addr = addr;
    rd.unit = unit;
    rd.len = len;
    if (len > sizeof(rd.data))
       rd.data = (int)buf;
    if ((retval = ioctl(robo_fd, UBSEC_ROBOIF, &rd))) {
       printf("ioctl failed %x\n",retval);
    } else {
      if (len <=  sizeof(rd.data))
        memcpy(buf,&rd.data,rd.len);
    }
#endif
    return retval;
}

void
bcms5_vlan_init(void)
{
    int retVal;
    char buf[80];
    bcm_mac_t mcast_addr = BRIDGE_GROUP_MAC_ADDR;
    bcm_mac_t def_mac_addr = DEFAULT_MAC_ADDR;
    bcm_l2_addr_t l2addr;

    printf("configure vlans\n");
    /* lan; ports 2-5; vid 1; don't untag MII */
    if (BCM_RET_SUCCESS != (retVal = bcms5_vlan_create(0,DEFAULT_LAN_VLAN)))
    {
        printf("ERROR: trying to create VLAN 0 for switch\n");
    }
    if (BCM_RET_SUCCESS != (retVal = bcms5_vlan_port_add(0,DEFAULT_LAN_VLAN,0x3e,0x1e)))
    {
        printf("ERROR: trying to add port\n");
    }
    /* wan; port 1; vid 2; untag MII */
    if (BCM_RET_SUCCESS != (retVal = bcms5_vlan_create(0,DEFAULT_WAN_VLAN)))
    {
        printf("ERROR: trying to create VLAN 0 for switch\n");
    }
    if (BCM_RET_SUCCESS != (retVal = bcms5_vlan_port_add(0,DEFAULT_WAN_VLAN,0x21,0x21)))
    {
        printf("ERROR: trying to add port\n");
    }

    if (BCM_RET_SUCCESS != (retVal =bcms5_vlan_enable(0,1)))
    {
        printf("ERROR: trying to enable ports\n");
    }
    /* set multicast related registers */
    buf[0] = 0x0e;      /* Rsvd MC tag/untag */
    bcms5_write_reg(0,ROBO_VLAN_PAGE,ROBO_VLAN_CTRL1,buf,1);
    buf[0] = 0x01;      /* enable MC ARL entries to use port map  */
    bcms5_write_reg(0,ROBO_CTRL_PAGE,ROBO_IP_MULTICAST_CTRL,buf,1);
    buf[0] = 0x1c;      /* MII receive unicast, multicast, broadcast */
    bcms5_write_reg(0,ROBO_CTRL_PAGE,ROBO_IM_PORT_CTRL,buf,1);
    /* set up static addr for WAN & LAN VLAN BPDUs */
    bcms5_l2_addr_init(&l2addr,mcast_addr,DEFAULT_WAN_VLAN);
    l2addr.pbmp = 0x21;
    if (BCM_RET_SUCCESS != (retVal = bcms5_l2_addr_add(0, &l2addr)))
      printf("failure writing wan l2 addr: %d\n",retVal);
    bcms5_l2_addr_init(&l2addr,mcast_addr,DEFAULT_LAN_VLAN);
    l2addr.pbmp = 0x3e;
    if (BCM_RET_SUCCESS != (retVal = bcms5_l2_addr_add(0, &l2addr)))
      printf("failure writing lan l2 addr: %d\n",retVal);
    bcms5_l2_addr_init(&l2addr,def_mac_addr,DEFAULT_WAN_VLAN);
    l2addr.port = BCM_MII_ARL_UC_PORT-1;
    if (BCM_RET_SUCCESS != (retVal = bcms5_l2_addr_add(0, &l2addr)))
      printf("failure writing lan l2 addr: %d\n",retVal);
    bcms5_l2_addr_init(&l2addr,def_mac_addr,DEFAULT_LAN_VLAN);
    l2addr.port = BCM_MII_ARL_UC_PORT-1;
    if (BCM_RET_SUCCESS != (retVal = bcms5_l2_addr_add(0, &l2addr)))
      printf("failure writing lan l2 addr: %d\n",retVal);
    memset(&mcast_addr,0,sizeof(mcast_addr));
    bcms5_write_reg(0,ROBO_ARLCTRL_PAGE,ROBO_BPDU_MC_ADDR_REG,mcast_addr,sizeof(mcast_addr));
    printf("configure vlans...done\n");
}

void
bcms5_gvlan(void)
{
    int         numVLANEntries, group, untag;
    bcm_vlan_table_entry_t VLANTable[NUM_VLAN_TABLE_ENTRIES*2];
    uint        retval = BCM_RET_SUCCESS;
    uint i;
    
    numVLANEntries = NUM_VLAN_TABLE_ENTRIES*2;  /* 2 vlans per table entry */
    retval = bcm_vlan_dumptable(0, &VLANTable[0], &numVLANEntries);
    if (retval != BCM_RET_SUCCESS)
    {
        printf("bcm_vlan_dumptable failed: %x\n",retval);
        return;
    }
    printf("%d valid VLAN entries found\n",numVLANEntries);
    for (i=0;i<numVLANEntries;i++)
    {
        group = VLANTable[i].vlan_entry.VLANgroup;
        untag = VLANTable[i].vlan_entry.VLANuntag;
        printf("VLANgroup: %3x VLANuntag: %3x vlan: %4d  addr: %x vlan_index: %x\n",
            group,untag,
            VLANTable[i].vlan_id, VLANTable[i].vlan_addr, VLANTable[i].vlan_index);
    }
}

void
bcms5_garl(void)
{
    bcm_arl_table_entry_t ARLTable[NUM_ARL_TABLE_ENTRIES];
    int         numARLEntries;
    uint        retval = BCM_RET_SUCCESS;
    uint i;
    bcm_mac_t   mcastAddr = BRIDGE_GROUP_MAC_ADDR;

    numARLEntries = NUM_ARL_TABLE_ENTRIES;
    retval = bcm_l2_addr_dumptable(0, &ARLTable[0], &numARLEntries);
    if (retval != BCM_RET_SUCCESS)
    {
        printf("bcm_l2_addr_dumptable failed: %x\n",retval);
        return;
    }
    printf("%d valid ARL entries found\n",numARLEntries);
    for (i=0;i<numARLEntries;i++)
    {
        /* check to see if this is a multicast address */
        if (memcmp(&ARLTable[i].MACaddr[5],&mcastAddr[5],1))
        {
          /* not multicast */
          printf("mac: %02x:%02x:%02x:%02x:%02x:%02x port: %d prio: %d age: %d static: %d vlan: %4d  addr: %x\n",
              ARLTable[i].MACaddr[5],ARLTable[i].MACaddr[4],
              ARLTable[i].MACaddr[3],ARLTable[i].MACaddr[2],
              ARLTable[i].MACaddr[1],ARLTable[i].MACaddr[0],
              ARLTable[i].port,ARLTable[i].highPrio,
              ARLTable[i].age,ARLTable[i].staticAddr,
              ARLTable[i].vid,ARLTable[i].arl_addr);
        } else {
          /* multicast */
          printf("mac: %02x:%02x:%02x:%02x:%02x:%02x port mask: %x prio: %d static: %d vlan: %4d  addr: %x\n",
              ARLTable[i].MACaddr[5],ARLTable[i].MACaddr[4],
              ARLTable[i].MACaddr[3],ARLTable[i].MACaddr[2],
              ARLTable[i].MACaddr[1],ARLTable[i].MACaddr[0],
              ARLTable[i].port,ARLTable[i].highPrio,ARLTable[i].staticAddr,
              ARLTable[i].vid,ARLTable[i].arl_addr);
        }
    }
}

void
bcms5_greg(uint page, uint addr, uint len)
{
    unsigned char RoboData[10];
    uint        retval = BCM_RET_SUCCESS;
    uint leadingZeros = 1;
    int i;
    
    if (len > sizeof(RoboData))
    {
        printf("length greater than %d\n",sizeof(RoboData));
        return;
    }
    retval = bcms5_read_reg(0, page, addr, (uint8 *)&RoboData[0],len);
    for (i=0;i<=(len-1);i++)
    {
      if (leadingZeros)
      {
          if (RoboData[i])
          {
              leadingZeros = 0;
              if (RoboData[i] <= 0xf)
                  printf("%1x",RoboData[i]);
              else
                  printf("%2x",RoboData[i]);
          }
          else
              continue;
      } else
          printf("%02x",RoboData[i]);
    }
    if (leadingZeros)
      printf("0");
    printf("\n");
}

void
bcms5_configure_LAN(void)
{
    /* bring up etv0 as LAN VLAN device */
    bcm_api_init();
    bcms5_vlan_init();
    ipAttach(0,"etv");
    ifAddrSet("etv0", ETV0_IP_ADDRESS);
}

void
bcms5_gport(uint port)
{
    uint        retval = BCM_RET_SUCCESS;
    bcm_port_info_t portinfo;

    if (port < BCM_MIN_PORT || port > BCM_NUM_PORTS)
    {
        printf("port number must be between %d and %d, inclusive\n",
                BCM_MIN_PORT,BCM_NUM_PORTS);
        return;
    }
    retval = bcms5_port_info_get(port, &portinfo);
    if (retval != BCM_RET_SUCCESS)
    {
        printf("bcm_port_info_get returned error %x\n",retval);
        return;
    }
    if (portinfo.link)
        printf("link is                up\n");
    else
        printf("link is                down\n");
    printf("port enable tx is      %d\n",portinfo.porten_tx);
    printf("port enable rx is      %d\n",portinfo.porten_rx);
    printf("auto negotiate is      %d\n",portinfo.anmode);
    printf("link speed is          %d\n",portinfo.speed);
    if (portinfo.dpx == ROBO_HALF_DUPLEX)
        printf("duplex is              half\n");
    else
        printf("duplex is              full\n");
    printf("flow control is        %d\n",portinfo.flow);
    printf("autocast is            %d\n",portinfo.anmode);
    printf("port mirror is         %d\n",portinfo.portmirror);
    if (portinfo.stp_state == BCM_PORT_STP_NONE)
        printf("spanning tree state is none\n");
    else if (portinfo.stp_state == BCM_PORT_STP_DISABLE)
        printf("spanning tree state is disabled\n");
    else if (portinfo.stp_state == BCM_PORT_STP_BLOCK)
        printf("spanning tree state is blocking\n");
    else if (portinfo.stp_state == BCM_PORT_STP_LISTEN)
        printf("spanning tree state is listening\n");
    else if (portinfo.stp_state == BCM_PORT_STP_LEARN)
        printf("spanning tree state is learning\n");
    else if (portinfo.stp_state == BCM_PORT_STP_FORWARD)
        printf("spanning tree state is forwarding\n");
    else
        printf("spanning tree state is unknown state %d\n",portinfo.stp_state);
}


IMPORT STATUS tffsBCM47xxBlkRd(int sector, int numsecs, char *buf);
IMPORT STATUS tffsBCM47xxBlkWrt(int sector, int numsecs, char *buf);

void
bcms5_readsec(int sector, int numsecs)
{
    /* reads a sector and prints contents */
    unsigned char *buf;
    int i,j,k;

    buf = malloc(numsecs*512);
    if (buf == NULL)
        return;
    tffsBCM47xxBlkRd(sector,numsecs,(char *)buf);
    for (i=0;i<numsecs;i++)
    {
        for (j=0;j<32;j++)
        {
            for (k=0;k<16;k++)
            {
                printf("%02x ",buf[i*512 + 16*j + k]);
            }
            printf("\n");
        }
    }
    free(buf);
}

void
bcms5_writesec(int sector, int numsecs)
{
    /* reads a sector and prints contents */
    unsigned char *buf;
    int j,count = 0;

    buf = malloc(numsecs*512);
    if (buf == NULL)
        return;
    for (j=0;j<(numsecs*512);j++)
        buf[j] = count++;
    tffsBCM47xxBlkWrt(sector,numsecs,(char *)buf);
    free(buf);
}

void
bcms5_clearsec(int sector, int numsecs)
{
    /* reads a sector and prints contents */
    unsigned char *buf;

    buf = malloc(numsecs*512);
    if (buf == NULL)
        return;
    memset(buf,0,numsecs*512);
    tffsBCM47xxBlkWrt(sector,numsecs,(char *)buf);
    free(buf);
}

