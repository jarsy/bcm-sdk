/*******************************************************************************
 * $Id: tut.h,v 1.1 2004/02/24 07:47:01 csm Exp $
 * tut.h - top level definitions for TUT protocol
 ******************************************************************************/

#ifndef _TUT_H_
#define _TUT_H_

#include "proto/ethernet.h"
#define TUT_HDR_NBYTES		(6)
#define TUT_DATA_CRC_FIELD_SIZE	(ETHER_CRC_LEN)
#define TUT_DATA_MAX_NBYTES	(ETHER_MAX_LEN)

/* XXX - header struct defined locally somewhere else... */

#define TUT_NBYTES_UNPROTECTED	(TUT_HDR_NBYTES)

/* XXX - For CNSR compatibility... */
#define	TUT_MAX_PACKET_NBYTES			(0x10000)

/* XXX - workaround for precise length computation */
#define TUT_LENGTH_WORKAROUND

#endif /* _TUT_H_ */
