/* $Id: sand_integer_arithmetic.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_SAND_INTEGER_ARITHMETIC_H__
/* { */
#define __SOC_SAND_INTEGER_ARITHMETIC_H__
#ifdef  __cplusplus
extern "C" {
#endif

#include <soc/dpp/SAND/Utils/sand_framework.h>
#include <sal/limits.h>

/* $Id: sand_integer_arithmetic.h,v 1.8 Broadcom SDK $
 * Divide Section
 * {
 */
/*
 * Integer division x/y with integer round up.
 * NOTE!!!!!! if y=0 .... ERROR
 */
#define SOC_SAND_DIV_ROUND_UP(x,y) ( ((x)+((y)-1)) / (y) )
/*
 * Integer division x/y with integer round down.
 * NOTE!!!!!! if y=0  .... ERROR
 */
#define SOC_SAND_DIV_ROUND_DOWN(x,y) ( (x) / (y) )
/*
 *  Align x to the upper boundary of y-sized groups - 
 * NOTE!!!!!! if y=0 .... ERROR
 */
#define SOC_SAND_ALIGN_UP(x, y) \
  (SOC_SAND_DIV_ROUND_UP(x,y) * (y))
/*
 *  Align x to the upper boundary of y-sized groups - 
 * NOTE!!!!!! if y=0 .... ERROR
 */
/*
 * Integer division x/y with integer round to nearest.
 * NOTE!!!!!! if y=0  .... ERROR
 */
#define SOC_SAND_DIV_ROUND(x,y) ( ((x) + ((y)/2) ) / (y) )
/*
 * Integer division x/8 with integer round down.
 */

/*
 *  The absolute (positive) value of the difference between two integers
 */
#define SOC_SAND_DELTA(x, y) (((x)>(y))?((x)-(y)):((y)-(x)))

/*
 *
 */

/*
 * }
 */


/*
 * General Constants
 * {
 */

/*
 * Number of bits in char (unsigned char) - 8.
 */
#define SOC_SAND_NOF_BITS_IN_CHAR   (SAL_CHAR_BIT)

/*
 * Number of bits in uint32 - 32.
 */
#define SOC_SAND_NOF_BITS_IN_UINT32 (SOC_SAND_NOF_BITS_IN_CHAR * sizeof(uint32))


#define SOC_SAND_LOHALF(x) ((uint32)((x) & 0xffff))
#define SOC_SAND_HIHALF(x) ((uint32)((x) >> 16 & 0xffff))
#define SOC_SAND_TOHIGH(x) ((uint32)((x) << 16))
#define SOC_SAND_HIBITMASK (0x80000000)

/*
 * }
 */



/*
 * Integer Logarithmic
 * {
 */
/*
 * extract the log2(x) - Round up.
 */
uint32 soc_sand_log2_round_up(SOC_SAND_IN uint32 x);
/*
 *  Calculate 2^power
 */
uint32
  soc_sand_power_of_2(
    SOC_SAND_IN uint32 power
  );

/*
 * extract the log2(x) - Round down.
 */
uint32 soc_sand_log2_round_down(SOC_SAND_IN uint32 x);
/*
 * Return TRUE: iff the number x is power of 2.
 * Otherwise return FALSE
 */
uint32 soc_sand_is_power_of_2(SOC_SAND_IN uint32 x);

/*
 * }
 */

/*
 * Bit Manipulation
 * {
 */
/*
 * get the number msb bit that is on.
 */
uint32
  soc_sand_msb_bit_on(
    SOC_SAND_IN uint32 x
  );

/*
 * Get the number of bits are on in uint32
 */
uint32
  soc_sand_nof_on_bits_in_long(
    SOC_SAND_IN uint32 x
  );

/*
 * Get the number of bits are on in SHORT (16 bits)
 */
uint32
  soc_sand_nof_on_bits_in_short(
    SOC_SAND_IN unsigned short x
  );

/*
 * Get the number of bits are on in CHAR (8 bits)
 */
uint32
  soc_sand_nof_on_bits_in_char(
    SOC_SAND_IN unsigned char x
  );

/*
 * Given number - x,
 * reverse (bit wise) the number for uint32
 */
uint32
  soc_sand_bits_reverse_long(
    SOC_SAND_IN uint32 x
  );

/*
 * Given number - x,
 * reverse (bit wise) the number for SHORT (16 bits)
 */
unsigned short
  soc_sand_bits_reverse_short(
    SOC_SAND_IN unsigned short x
  );

/*
 * Given number - x,
 * reverse (bit wise) the number for CHAR (8 bits)
 */
unsigned char
  soc_sand_bits_reverse_char(
    SOC_SAND_IN unsigned char x
  );

/*
 * }
 */

/*
 * Mantissa, Exponent
 * {
 */
/*
 * Translate an absolute value and reference value to a fraction (given as a binary fraction
 * mantissa and an exponent)
 */
SOC_SAND_RET
  soc_sand_abs_val_to_mnt_binary_fraction_exp(
    SOC_SAND_IN  uint32  abs_val_numerator,
    SOC_SAND_IN  uint32  abs_val_denominator,
    SOC_SAND_IN  uint32   mnt_nof_bits,
    SOC_SAND_IN  uint32   exp_nof_bits,
    SOC_SAND_IN  uint32  max_val,
    SOC_SAND_OUT uint32* mnt_bin_fraction,
    SOC_SAND_OUT uint32* exp
  );
/*
 * Translate a fraction (given as a binary fraction mantissa and an exponent)
 * and reference value to an absolute value
 */
SOC_SAND_RET
  soc_sand_mnt_binary_fraction_exp_to_abs_val(
    SOC_SAND_IN  uint32   mnt_nof_bits,
    SOC_SAND_IN  uint32   exp_nof_bits,
    SOC_SAND_IN  uint32  max_val,
    SOC_SAND_IN  uint32  mnt_bin_fraction,
    SOC_SAND_IN  uint32  exp,
    SOC_SAND_OUT uint32  *abs_val_numerator,
    SOC_SAND_OUT uint32  *abs_val_denominator
  );
/* break a number to mantissa and exponent - Round Up */
SOC_SAND_RET soc_sand_break_to_mnt_exp_round_up(SOC_SAND_IN  uint32 x,
                                        SOC_SAND_IN  uint32  man_nof_bits,
                                        SOC_SAND_IN  uint32  exp_nof_bits,
                                        SOC_SAND_IN  uint32  mnt_inc,
                                        SOC_SAND_OUT uint32* man,
                                        SOC_SAND_OUT uint32* exp);
/* break a number to mantissa and exponent - Round Down */
SOC_SAND_RET soc_sand_break_to_mnt_exp_round_down(SOC_SAND_IN  uint32 x,
                                          SOC_SAND_IN  uint32  man_nof_bits,
                                          SOC_SAND_IN  uint32  exp_nof_bits,
                                          SOC_SAND_IN  uint32  mnt_inc,
                                          SOC_SAND_OUT uint32* man,
                                          SOC_SAND_OUT uint32* exp);

/* break a number to mantissa and exponent "x = eq_const_multi * (eq_const_mnt_inc + mnt) * 2^exp" */
SOC_SAND_RET
  soc_sand_break_complex_to_mnt_exp_round_down(SOC_SAND_IN  uint32 x,
                                           SOC_SAND_IN  uint32  max_mnt,
                                           SOC_SAND_IN  uint32  max_exp,
                                           SOC_SAND_IN  uint32  eq_const_multi,
                                           SOC_SAND_IN  uint32  eq_const_man_inc,
                                           SOC_SAND_OUT uint32* mnt,
                                           SOC_SAND_OUT uint32* exp);
											  
/* compute a number from mantissa and exponent "x = eq_const_multi * (eq_const_mnt_inc + mnt) * 2^exp" */
SOC_SAND_RET
  soc_sand_compute_complex_to_mnt_exp(SOC_SAND_IN  uint32  mnt,
                                  SOC_SAND_IN  uint32  exp,
                                  SOC_SAND_IN  uint32 eq_const_multi,
                                  SOC_SAND_IN  uint32 eq_const_mnt_inc,
                                  SOC_SAND_OUT uint32* x);

/* break a number to mantissa and reverse exponent "x = eq_const_multi * (eq_const_mnt_inc + mnt) / 2^exp" */
SOC_SAND_RET
  soc_sand_break_complex_to_mnt_reverse_exp_round_down(SOC_SAND_IN  uint32 x,
                                                   SOC_SAND_IN  uint32  max_mnt,
                                                   SOC_SAND_IN  uint32  max_exp,
                                                   SOC_SAND_IN  uint32  eq_const_multi,
                                                   SOC_SAND_IN  uint32  eq_const_man_inc,
                                                   SOC_SAND_OUT uint32* mnt,
                                                   SOC_SAND_OUT uint32* exp);

/* compute a number from mantissa and reverse exponent "x = eq_const_multi * (eq_const_mnt_inc + mnt) / 2^exp" */
SOC_SAND_RET
  soc_sand_compute_complex_to_mnt_reverse_exp(SOC_SAND_IN  uint32  mnt,
                                          SOC_SAND_IN  uint32  exp,
                                          SOC_SAND_IN  uint32 eq_const_multi,
                                          SOC_SAND_IN  uint32 eq_const_mnt_inc,
                                          SOC_SAND_OUT uint32* x);

/* compute a number from mantissa, exponent and reverse exponent "x = eq_const_multi * (eq_const_mnt_inc + mnt) * 2^exp / (eq_const_div * 2^rev_exp)" */
SOC_SAND_RET
  soc_sand_compute_complex_to_mnt_exp_reverse_exp(
    SOC_SAND_IN  uint32  mnt,
    SOC_SAND_IN  uint32  exp,
    SOC_SAND_IN  uint32  rev_exp,
    SOC_SAND_IN  uint32  eq_const_multi,
    SOC_SAND_IN  uint32  eq_const_div,
    SOC_SAND_IN  uint32  eq_const_mnt_inc,
    SOC_SAND_OUT uint32* x);

/* break a number to mantissa and exponent "x = eq_const_multi * (eq_const_mnt_inc + mnt) * 2^exp / eq_const_div" */
SOC_SAND_RET
  soc_sand_break_complex_to_mnt_exp_round_down_2(
    SOC_SAND_IN  uint32  x,
    SOC_SAND_IN  uint32  max_mnt,
    SOC_SAND_IN  uint32  min_mnt,
    SOC_SAND_IN  uint32  max_exp,
    SOC_SAND_IN  uint32  eq_const_multi,
    SOC_SAND_IN  uint32  eq_const_div,
    SOC_SAND_IN  uint32  eq_const_mnt_inc,
    SOC_SAND_OUT uint32* mnt,
    SOC_SAND_OUT uint32* exp
  );

/* return abs number*/
uint32 soc_sand_abs(SOC_SAND_IN int);

/* return sign of a number*/
int soc_sand_sign(SOC_SAND_IN int);

/* return TRUE iff x is even */
int soc_sand_is_even(SOC_SAND_IN uint32 x);

/*
 * }
 */

/*
 * Min, Max
 * {
 */
#define SOC_SAND_MIN(x,y) ( (x)<(y)? (x) : (y) )
#define SOC_SAND_MAX(x,y) ( (x)>(y)? (x) : (y) )
#define SOC_SAND_MAX3(x,y,z) ( SOC_SAND_MAX(x, SOC_SAND_MAX(y, z)) )
/*
 * }
 */

#if defined (SOC_SAND_DEBUG)
/* { */
/*
 */
uint32
  soc_sand_integer_arithmetic_test(
    SOC_SAND_IN uint32 silent
  );
uint32
  soc_sand_mnt_binary_fraction_test(
    SOC_SAND_IN uint32 silent
  );

void
  soc_sand_print_u_long_binary_format(
    SOC_SAND_IN uint32 x,
    SOC_SAND_IN uint32 max_digits_in_number
  );
/*
 * }
 */
#endif

#ifdef  __cplusplus
}
#endif

/* } __SOC_SAND_INTEGER_ARITHMETIC_H__*/
#endif
