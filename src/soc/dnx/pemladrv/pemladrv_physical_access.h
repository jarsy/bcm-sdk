/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _PEMLADRV_PHYSICAL_ACCESS_H_
#define _PEMLADRV_PHYSICAL_ACCESS_H_

#include <stdlib.h>

#define PEM_BLOCK_ID  59

#ifdef BCM_DNX_SUPPORT
  #include <soc/schanmsg.h>

#else /* BCM_DNX_SUPPORT */

typedef struct {
  unsigned int   reserve;        /* maybe used as memory index*/
  unsigned int   block_identifier;      /* PEM is 59 */
  unsigned int   mem_address;  
  unsigned int   row_index;      /* 0 for register access*/
  unsigned int   mem_width_in_bits;
 /* int            is_mem; */
} phy_mem_t;


int phy_mem_read(int          unit,
                 phy_mem_t*    mem,
                 void *entry_data);

int phy_mem_write(int          unit,
                  phy_mem_t*    mem,
                  void *entry_data);

#endif /* BCM_DNX_SUPPORT */

#endif /* _PEMLADRV_PHYSICAL_ACCESS_H_ */

