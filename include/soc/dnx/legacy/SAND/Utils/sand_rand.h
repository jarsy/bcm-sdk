/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __DNX_SAND_RAND_H_INCLUDED__
/* { */
#define __DNX_SAND_RAND_H_INCLUDED__

#ifdef  __cplusplus
extern "C" {
#endif


#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
/* $Id$
 * Random constants - Do not edit.
 * {
 */
  /*
   * Random array of 351 long
   */
  #define DNX_SAND_RAND_N   351
  #define DNX_SAND_RAND_M   175
  #define DNX_SAND_RAND_MATRIX_A  0xEABD75F5
  #define DNX_SAND_RAND_TEMU  11
  #define DNX_SAND_RAND_TEMS  7
  #define DNX_SAND_RAND_TEMT  15
  #define DNX_SAND_RAND_TEML  17
  #define DNX_SAND_RAND_TEMB  0x655E5280UL
  #define DNX_SAND_RAND_TEMC  0xFFD58000
/*
 * }
 */

/*
 * Random status structure
 */
typedef struct
{
  uint32   state[DNX_SAND_RAND_N]; /* the array for the state vector  */
  uint32   left;  /*how many lest in state*/
  uint32   initf; /*is initialized*/
  uint32*  next;

  /*
   */
  uint32   seed;
} DNX_SAND_RAND;


/*
 * {
 * DNX_SAND_RAND APIs.
 * These function are basic random functions.
 */


/*
 * Various dnx_sand_rand constants
 */
enum
{
  DNX_SAND_RAND_PERCENT_ONE = 10000,
  DNX_SAND_RAND_PERCENT_100 = 100*DNX_SAND_RAND_PERCENT_ONE
};

/*****************************************************
*NAME:
* dnx_sand_rand_seed_set
* dnx_sand_rand_seed_get
*FUNCTION:
*   Set/Get seed for random.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_OUT DNX_SAND_RAND     *r         -
*      Random structure
*    DNX_SAND_IN  uint32 seed -
*      seed value
*OUTPUT:
*  DNX_SAND_DIRECT:
*    None
*  DNX_SAND_INDIRECT:
*    Random structure
*****************************************************/
void
  dnx_sand_rand_seed_set(
    DNX_SAND_OUT DNX_SAND_RAND     *r,
    DNX_SAND_IN  uint32 seed
  );
uint32
  dnx_sand_rand_seed_get(
    DNX_SAND_IN DNX_SAND_RAND    *r
  );


/*****************************************************
*NAME:
* dnx_sand_rand_coin_1
*FUNCTION:
*   Get 0/1 based on random coin fliping with positive bias
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT DNX_SAND_RAND     *r         -
*      Random structure
*    DNX_SAND_IN  uint32 positive_bias -
*      Positive bias in precents.
*      Assuming 0<=positive_bias<=100*DNX_SAND_RAND_ONE_PERCENT
*OUTPUT:
*  DNX_SAND_DIRECT:
*    None
*  DNX_SAND_INDIRECT:
*    Random structure
*****************************************************/
uint32
  dnx_sand_rand_coin_1(
    DNX_SAND_INOUT DNX_SAND_RAND     *r,
    DNX_SAND_IN    uint32 positive_bias
  );

/*****************************************************
*NAME:
* dnx_sand_rand_coin_2
*FUNCTION:
*   Get 0/1 based on random coin flipping with
*  positive/negative bias
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_OUT DNX_SAND_RAND     *r         -
*      Random structure
*    DNX_SAND_IN  uint32 positive_bias -
*      Positive bias in precents.
*      Assuming 0<=positive_bias<=100*DNX_SAND_RAND_ONE_PERCENT
*    DNX_SAND_IN  uint32 negative_bias -
*      negative bias in precents.
*      Assuming 0<=negative_bias<=100*DNX_SAND_RAND_ONE_PERCENT
*OUTPUT:
*  DNX_SAND_DIRECT:
*    None
*  DNX_SAND_INDIRECT:
*    Random structure
*****************************************************/
uint32
  dnx_sand_rand_coin_2(
    DNX_SAND_INOUT DNX_SAND_RAND     *r,
    DNX_SAND_IN    uint32 positive_bias,
    DNX_SAND_IN    uint32 negative_bias
  );

/*****************************************************
*NAME:
* dnx_sand_rand_range
*FUNCTION:
*   Get a number in the range min<=x<=max
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT DNX_SAND_RAND     *r         -
*      Random structure
*    DNX_SAND_IN    uint32 min -
*      Min value
*    DNX_SAND_IN    uint32 max -
*      Max value
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32
*  DNX_SAND_INDIRECT:
*    Random structure
*****************************************************/
uint32
  dnx_sand_rand_range(
    DNX_SAND_INOUT DNX_SAND_RAND     *r,
    DNX_SAND_IN    uint32 min,
    DNX_SAND_IN    uint32 max
  );

/*****************************************************
*NAME:
* dnx_sand_rand_range
*FUNCTION:
*   Get a random number in the range
*  0<=x<=Max-integer-positive-value
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT DNX_SAND_RAND     *r         -
*      Random structure
*OUTPUT:
*  DNX_SAND_DIRECT:
*    int
*  DNX_SAND_INDIRECT:
*    Random structure
*****************************************************/
int
  dnx_sand_rand_int(
    DNX_SAND_INOUT DNX_SAND_RAND     *r
  );

/*****************************************************
*NAME:
* dnx_sand_rand_u_long
*FUNCTION:
*   Get a number of 32 bits
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT DNX_SAND_RAND     *r         -
*      Random structure
*OUTPUT:
*  DNX_SAND_DIRECT:
*    int
*  DNX_SAND_INDIRECT:
*    Random structure
*****************************************************/
uint32
  dnx_sand_rand_u_long(
    DNX_SAND_INOUT DNX_SAND_RAND     *r
  );

/*****************************************************
*NAME:
* dnx_sand_rand_u_long
*FUNCTION:
*   Get a random number in the range o and m-1
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT DNX_SAND_RAND     *r         -
*      Random structure
*    DNX_SAND_IN    uint32 m -
*      Module number
*OUTPUT:
*  DNX_SAND_DIRECT:
*    int
*  DNX_SAND_INDIRECT:
*    Random structure
*****************************************************/
uint32
  dnx_sand_rand_modulo(
    DNX_SAND_INOUT DNX_SAND_RAND     *r,
    DNX_SAND_IN    uint32 m
  );

/*
 * }
 */



/*
 * {
 * DNX_SAND_RAND more APIs.
 * These function are more complicated random functions.
 */


/*****************************************************
*NAME:
* dnx_sand_rand_array_permute_u_long
*FUNCTION:
*   Given array of u-longs 'array', the function will
*  generate one permutation of this array.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_OUT DNX_SAND_RAND     *r         -
*      Random structure
*    DNX_SAND_INOUT uint32 array[],
*      Array of 'unsigned-longs'.
*    DNX_SAND_IN    uint32 array_size
*      Number of 'u-long' in 'array'
*OUTPUT:
*  DNX_SAND_INDIRECT:
*    + 'array' will be changed.
*    + Random structure
*****************************************************/
void
  dnx_sand_rand_array_permute_u_long(
    DNX_SAND_INOUT DNX_SAND_RAND     *r,
    DNX_SAND_INOUT uint32 array[],
    DNX_SAND_IN    uint32 array_size
  );


/*****************************************************
*NAME:
* dnx_sand_rand_array_permute_u_long
*FUNCTION:
*   Given array of u-longs 'array', the function will
*  generate one permutation of this array.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_INOUT DNX_SAND_RAND               *r,
*      Random structure
*    DNX_SAND_INOUT unsigned char           array[],
*      Array of 'STRUCTUTE'.
*    DNX_SAND_IN    uint32           nof_items,
*      Number of 'STRUCTUTE' in 'array'
*    DNX_SAND_IN    DNX_SAND_RAND_SWAP_ARR_FUNC swap
*      Swap function of 'STRUCTUTE' in 'array'.
*OUTPUT:
*  DNX_SAND_INDIRECT:
*    + 'array' will be changed.
*    + Random structure
*    + 'swap' will be called approx. nof_items
*SEE ALSO:
*   Implementation of 'dnx_sand_rand_array_permute_u_long()'.
*****************************************************/
/*
 */
typedef void (*DNX_SAND_RAND_SWAP_ARR_FUNC)(unsigned char array[], uint32 indx_1, uint32 indx_2);
/*
 */
void
  dnx_sand_rand_array_permute(
    DNX_SAND_INOUT DNX_SAND_RAND               *r,
    DNX_SAND_INOUT unsigned char           array[],
    DNX_SAND_IN    uint32           nof_items,
    DNX_SAND_IN    DNX_SAND_RAND_SWAP_ARR_FUNC swap
  );

/*
 * }
 */


#if DNX_SAND_DEBUG
/* { */

/*prints the seed*/
void
  dnx_sand_rand_print(
    DNX_SAND_IN DNX_SAND_RAND    *r
  );

int dnx_sand_rand_test(DNX_SAND_IN uint32);

/*
 * }
 */
#endif


#ifdef  __cplusplus
}
#endif


/* } __DNX_SAND_RAND_H_INCLUDED__*/
#endif

