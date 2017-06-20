/* 
 * $Id: soc.h,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        soc.h
 * Purpose:     Device diagnostics commands.
 *
 */


#ifndef _APPL_DCMN_SOC_H_INCLUDED_
#define _APPL_DCMN_SOC_H_INCLUDED_

#include <soc/chip.h>

typedef struct soc_interrupt_data_s {
  char*             reg_name;
  soc_reg_t	 	    	reg;
  soc_field_t  	    field;
  soc_reg_t					mask_reg;
  soc_field_t	    	mask_field;
  uint32 	        flags;
} soc_interrupt_data_t;



#endif /*_APPL_DCMN_SOC_H_INCLUDED_*/
