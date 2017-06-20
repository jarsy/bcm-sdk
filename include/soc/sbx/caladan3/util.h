/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: util.h,v 1.10 Broadcom SDK $
 *
 * util.h : util defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SBX_CALADN3_UTIL_H_
#define _SBX_CALADN3_UTIL_H_

#include <soc/types.h>
#include <soc/register.h>

#define SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(instance) (SOC_REG_ADDR_INSTANCE_MASK | instance)
#define SOC_SBX_CALADAN3_DEFAULT_WAIT_TIME_USEC (2*SECOND_USEC) /* 1 second */

#define SOC_SBX_POWER_OF_TWO(val) (((val) & (val-1)) == 0)

#define GEN_MASK(bitpos) ((1<<(bitpos)) - 1)

#define SOC_SBX_CALADAN3_POLL_REGISTER(u, r, f, v) \
             soc_sbx_caladan3_reg32_expect_field_timeout((u), (r), -1, 0, -1, (f), (v), -1)

#define SOC_SBX_CALADAN3_POLL_PORT_REGISTER(u, p, r, f, v) \
             soc_sbx_caladan3_reg32_expect_field_timeout((u), (r), -1, 0, (p), (f), (v), -1)

#define SOC_SBX_CALADAN3_POLL_BLOCK_REGISTER(u, b, r, f, v) \
             soc_sbx_caladan3_reg32_expect_field_timeout((u), (r), (b), 0, -1, (f), (v), -1)


/* SOC_PBMP Macros depends on max ports, which in turn depends on which
 * chips are included in the build. The max ports could be just around 128 so 
 * this will limit the bitmask length, following macros make it more generic
 * and allow user to specify max words.
 * For now, we are only supporting a subset of SOC_PBMP macros 
 */
#define	_SOC_SBX_C3_BMP_WORD_WIDTH		(32)

#define _SOC_SBX_C3_BMP_WORD_GET(bm, word)	((bm)[(word)])
#define _SOC_SBX_C3_BMP_WORD_SET(bm, word, val)	((bm)[(word)] = (val))
#define	_SOC_SBX_C3_BMP_WENT(bitpos)		((bitpos)/_SOC_SBX_C3_BMP_WORD_WIDTH)
#define	_SOC_SBX_C3_BMP_WBIT(bitpos)		(1U << ((bitpos) % _SOC_SBX_C3_BMP_WORD_WIDTH))

#define _SOC_SBX_C3_BMP_ENTRY(bm, bitpos)         \
  (_SOC_SBX_C3_BMP_WORD_GET(bm,_SOC_SBX_C3_BMP_WENT(bitpos)))

/* clear the bitmap */
#define SOC_SBX_C3_BMP_CLEAR(bm, words) 	do {    \
    int	_w;                                             \
    for (_w = 0; _w < (words); _w++) {                        \
      _SOC_SBX_C3_BMP_WORD_GET(bm, _w) = 0;                   \
    }                                                         \
  } while (0)

#define SOC_SBX_C3_BMP_ADD(bm, bitpos)          \
  (_SOC_SBX_C3_BMP_ENTRY(bm, bitpos) |= _SOC_SBX_C3_BMP_WBIT(bitpos))

#define SOC_SBX_C3_BMP_REMOVE(bm, bitpos)          \
  (_SOC_SBX_C3_BMP_ENTRY(bm, bitpos) &= ~_SOC_SBX_C3_BMP_WBIT(bitpos))

#define SOC_SBX_C3_BMP_FLIP(bm, bitpos)          \
  (_SOC_SBX_C3_BMP_ENTRY(bm, bitpos) ^= _SOC_SBX_C3_BMP_WBIT(bitpos))

#define SOC_SBX_C3_BMP_SET(bm, bitpos, words)	do { \
    SOC_SBX_C3_BMP_CLEAR(bm, words);                 \
    SOC_SBX_C3_BMP_ADD(bm, bitpos);                  \
  } while(0)

/* check if a 0-based bit is set in the bitmap */
#define SOC_SBX_C3_BMP_MEMBER(bm, bitpos)       \
  ((_SOC_SBX_C3_BMP_ENTRY((bm), (bitpos)) & _SOC_SBX_C3_BMP_WBIT(bitpos)) != 0)

/* check if a 0-based bit is set in the bitmap representing a range */
#define SOC_SBX_C3_BMP_MEMBER_IN_RANGE(bm, base, bitpos)       \
  ((SOC_SBX_C3_BMP_MEMBER((bm), ((bitpos)-(base))))) 

/* iterate all bits set in bitmap */
#define SOC_SBX_C3_BMP_ITER(bm, bitpos, words)  \
  for ((bitpos) = 0; (bitpos) < ((words)*_SOC_SBX_C3_BMP_WORD_WIDTH); (bitpos)++) \
    if (SOC_SBX_C3_BMP_MEMBER((bm), (bitpos)))

/* iterate all bits set in bitmap representing a given range*/
#define SOC_SBX_C3_BMP_ITER_RANGE(bm, base, bitpos, words)  \
  for ((bitpos) = (base); (bitpos) < ((base)+((words)*_SOC_SBX_C3_BMP_WORD_WIDTH)); (bitpos)++) \
    if (SOC_SBX_C3_BMP_MEMBER_IN_RANGE((bm),(base),(bitpos)))

/* check how many bits are set in bitmap */
#define	SOC_SBX_C3_BMP_COUNT(bm, count, words)	do {    \
    int	_w;                                             \
    count = 0;                                                          \
    for (_w = 0; _w < (words); _w++) {                                  \
      count += _shr_popcount(_SOC_SBX_C3_BMP_WORD_GET(bm, _w));         \
    }                                                                   \
  } while(0)

extern 
int soc_sbx_caladan3_reg32_expect_field_timeout(int unit,
                                                soc_reg_t reg,/* register */
                                                int blk_instance, /* blk instance */
                                                int index, /* index */
                                                int port, /* port id is applicable */
                                                int field_id, /* field ID */
                                                int field_value, /* field value to wait on */
                                                int wait_time_usec); /* wait time in usec */

extern int soc_sbx_caladan3_msb_bit_pos(unsigned int val);

extern int soc_sbx_caladan3_round_power_of_two(int unit, 
                                               unsigned int *value, 
                                               uint8 down /*true-round down, false-up*/);

void soc_sbx_caladan3_cmic_endian(uint8 *buffer, uint32 size);

extern int soc_sbx_caladan3_reg32_dump(int unit, int reg, uint32 regval);

extern int soc_sbx_caladan3_reg32_reset_val_get(int unit, int reg, 
                                                uint32 *regval, uint32 flags);


#endif /* _SBX_CALADN3_UTIL_H_ */
