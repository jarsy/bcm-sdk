/*
 * $Id$
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __NEMO_UTIL_H__
#define __NEMO_UTIL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "nd_platform.h"


/*///////////////////////////////////////////////////////////////////////////// */
/* bitwise operations */
/* */
typedef struct
{
    AG_U8   *p_buf;
    AG_U8   *p_addr;
    AG_U32  n_offset;
    AG_U32  n_buf_size;

} AgNdBitwise;

void    ag_nd_bitwise_write(AgNdBitwise *p_bw, AG_U32 n_data, AG_U32 n_len);

/*///////////////////////////////////////////////////////////////////////////// */
/* some handy power of 2 arithmetics */
/*  */
AG_BOOL ag_nd_is_power_of_2(AG_U32 n);

/*///////////////////////////////////////////////////////////////////////////// */
/* log2 (assumed that the arg is power of 2) */
/*  */
AG_U32 ag_nd_log2(AG_U32 x);

/*///////////////////////////////////////////////////////////////////////////// */
/* Rounding down to the next power of 2 */
/* */
AG_U32  ag_nd_flp2(AG_U32 n);


/*///////////////////////////////////////////////////////////////////////////// */
/* Rounding up to the next power of 2 */
/* */
AG_U32  ag_nd_clp2(AG_U32 n);


/*///////////////////////////////////////////////////////////////////////////// */
/* Rounding down to a multiple of a known power of 2 */
/* */
AG_U32 ag_nd_round_down2(AG_U32 n, AG_U32 p);


/*///////////////////////////////////////////////////////////////////////////// */
/* Rounding up to a multiple of a known power of 2 */
/* */
AG_U32 ag_nd_round_up2(AG_U32 n, AG_U32 p);



#ifdef __cplusplus
}
#endif

#endif /* __NEMO_UTIL_H__ */

