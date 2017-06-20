/*
 * $Id: qe2000_intr.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef _QE2000_INTR_H
#define _QE2000_INTR_H

#include <sal/types.h>

void soc_qe2000_pci_error0(int unit, uint32 ignored);
void soc_qe2000_pci_error1(int unit, uint32 ignored);
void soc_qe2000_dma_done_intr(int unit, uint32 ignored);
void soc_qe2000_qmgr_intr(int unit, uint32 ignored);
void soc_qe2000_qmgr_intr_enable(int unit);

#define  SBQE_PCI_ERR1_GET_LARGE_DROP(r)	    (((r) >> 24) & 0xff)
#define  SBQE_PCI_ERR1_GET_EOP_MISSING(r)	    (((r) >> 20) & 0x0f)
#define  SBQE_PCI_ERR1_GET_EXTRA_SOP(r)		    (((r) >> 16) & 0x0f)
#define  SBQE_PCI_ERR1_GET_RXPKT_ABRT(r)	    (((r) >> 12) & 0x0f)
#define  SBQE_PCI_ERR1_GET_DROP_SMALL_PKT(r)    (((r) >> 0) & 0x0f)

/* Keep these interfaces for now so that merge won't break customer exisiting code */
#ifndef TO_BE_DEPRECIATED_CODE
#define TO_BE_DEPRECIATED_CODE
#endif 
#ifdef TO_BE_DEPRECIATED_CODE
void soc_sbx_qe2000_pci_error0(int unit, uint32 ignored);
void soc_sbx_qe2000_pci_error1(int unit, uint32 ignored);
#endif

#endif /* _QE2000_INTR_H */
