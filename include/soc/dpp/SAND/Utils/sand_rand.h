/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __SOC_SAND_RAND_H_INCLUDED__
/* { */
#define __SOC_SAND_RAND_H_INCLUDED__

#ifdef  __cplusplus
extern "C" {
#endif


#include <soc/dpp/SAND/Utils/sand_framework.h>
/* $Id$
 * Random constants - Do not edit.
 * {
 */
  /*
   * Random array of 351 long
   */
  #define SOC_SAND_RAND_N   351
  #define SOC_SAND_RAND_M   175
  #define SOC_SAND_RAND_MATRIX_A  0xEABD75F5
  #define SOC_SAND_RAND_TEMU  11
  #define SOC_SAND_RAND_TEMS  7
  #define SOC_SAND_RAND_TEMT  15
  #define SOC_SAND_RAND_TEML  17
  #define SOC_SAND_RAND_TEMB  0x655E5280UL
  #define SOC_SAND_RAND_TEMC  0xFFD58000
/*
 * }
 */

/*
 * Random status structure
 */
typedef struct
{
  uint32   state[SOC_SAND_RAND_N]; /* the array for the state vector  */
  uint32   left;  /*how many lest in state*/
  uint32   initf; /*is initialized*/
  uint32*  next;

  /*
   */
  uint32   seed;
} SOC_SAND_RAND;


/*
 * {
 * SOC_SAND_RAND APIs.
 * These function are basic random functions.
 */


/*
 * Various soc_sand_rand constants
 */
enum
{
  SOC_SAND_RAND_PERCENT_ONE = 10000,
  SOC_SAND_RAND_PERCENT_100 = 100*SOC_SAND_RAND_PERCENT_ONE
};

/*****************************************************
*NAME:
* soc_sand_rand_seed_set
* soc_sand_rand_seed_get
*FUNCTION:
*   Set/Get seed for random.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_OUT SOC_SAND_RAND     *r         -
*      Random structure
*    SOC_SAND_IN  uint32 seed -
*      seed value
*OUTPUT:
*  SOC_SAND_DIRECT:
*    None
*  SOC_SAND_INDIRECT:
*    Random structure
*****************************************************/
void
  soc_sand_rand_seed_set(
    SOC_SAND_OUT SOC_SAND_RAND     *r,
    SOC_SAND_IN  uint32 seed
  );
uint32
  soc_sand_rand_seed_get(
    SOC_SAND_IN SOC_SAND_RAND    *r
  );


/*****************************************************
*NAME:
* soc_sand_rand_coin_1
*FUNCTION:
*   Get 0/1 based on random coin fliping with positive bias
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT SOC_SAND_RAND     *r         -
*      Random structure
*    SOC_SAND_IN  uint32 positive_bias -
*      Positive bias in precents.
*      Assuming 0<=positive_bias<=100*SOC_SAND_RAND_ONE_PERCENT
*OUTPUT:
*  SOC_SAND_DIRECT:
*    None
*  SOC_SAND_INDIRECT:
*    Random structure
*****************************************************/
uint32
  soc_sand_rand_coin_1(
    SOC_SAND_INOUT SOC_SAND_RAND     *r,
    SOC_SAND_IN    uint32 positive_bias
  );

/*****************************************************
*NAME:
* soc_sand_rand_coin_2
*FUNCTION:
*   Get 0/1 based on random coin flipping with
*  positive/negative bias
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_OUT SOC_SAND_RAND     *r         -
*      Random structure
*    SOC_SAND_IN  uint32 positive_bias -
*      Positive bias in precents.
*      Assuming 0<=positive_bias<=100*SOC_SAND_RAND_ONE_PERCENT
*    SOC_SAND_IN  uint32 negative_bias -
*      negative bias in precents.
*      Assuming 0<=negative_bias<=100*SOC_SAND_RAND_ONE_PERCENT
*OUTPUT:
*  SOC_SAND_DIRECT:
*    None
*  SOC_SAND_INDIRECT:
*    Random structure
*****************************************************/
uint32
  soc_sand_rand_coin_2(
    SOC_SAND_INOUT SOC_SAND_RAND     *r,
    SOC_SAND_IN    uint32 positive_bias,
    SOC_SAND_IN    uint32 negative_bias
  );

/*****************************************************
*NAME:
* soc_sand_rand_range
*FUNCTION:
*   Get a number in the range min<=x<=max
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT SOC_SAND_RAND     *r         -
*      Random structure
*    SOC_SAND_IN    uint32 min -
*      Min value
*    SOC_SAND_IN    uint32 max -
*      Max value
*OUTPUT:
*  SOC_SAND_DIRECT:
*    uint32
*  SOC_SAND_INDIRECT:
*    Random structure
*****************************************************/
uint32
  soc_sand_rand_range(
    SOC_SAND_INOUT SOC_SAND_RAND     *r,
    SOC_SAND_IN    uint32 min,
    SOC_SAND_IN    uint32 max
  );

/*****************************************************
*NAME:
* soc_sand_rand_range
*FUNCTION:
*   Get a random number in the range
*  0<=x<=Max-integer-positive-value
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT SOC_SAND_RAND     *r         -
*      Random structure
*OUTPUT:
*  SOC_SAND_DIRECT:
*    int
*  SOC_SAND_INDIRECT:
*    Random structure
*****************************************************/
int
  soc_sand_rand_int(
    SOC_SAND_INOUT SOC_SAND_RAND     *r
  );

/*****************************************************
*NAME:
* soc_sand_rand_u_long
*FUNCTION:
*   Get a number of 32 bits
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT SOC_SAND_RAND     *r         -
*      Random structure
*OUTPUT:
*  SOC_SAND_DIRECT:
*    int
*  SOC_SAND_INDIRECT:
*    Random structure
*****************************************************/
uint32
  soc_sand_rand_u_long(
    SOC_SAND_INOUT SOC_SAND_RAND     *r
  );

/*****************************************************
*NAME:
* soc_sand_rand_u_long
*FUNCTION:
*   Get a random number in the range o and m-1
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT SOC_SAND_RAND     *r         -
*      Random structure
*    SOC_SAND_IN    uint32 m -
*      Module number
*OUTPUT:
*  SOC_SAND_DIRECT:
*    int
*  SOC_SAND_INDIRECT:
*    Random structure
*****************************************************/
uint32
  soc_sand_rand_modulo(
    SOC_SAND_INOUT SOC_SAND_RAND     *r,
    SOC_SAND_IN    uint32 m
  );

/*
 * }
 */



/*
 * {
 * SOC_SAND_RAND more APIs.
 * These function are more complicated random functions.
 */


/*****************************************************
*NAME:
* soc_sand_rand_array_permute_u_long
*FUNCTION:
*   Given array of u-longs 'array', the function will
*  generate one permutation of this array.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_OUT SOC_SAND_RAND     *r         -
*      Random structure
*    SOC_SAND_INOUT uint32 array[],
*      Array of 'unsigned-longs'.
*    SOC_SAND_IN    uint32 array_size
*      Number of 'u-long' in 'array'
*OUTPUT:
*  SOC_SAND_INDIRECT:
*    + 'array' will be changed.
*    + Random structure
*****************************************************/
void
  soc_sand_rand_array_permute_u_long(
    SOC_SAND_INOUT SOC_SAND_RAND     *r,
    SOC_SAND_INOUT uint32 array[],
    SOC_SAND_IN    uint32 array_size
  );


/*****************************************************
*NAME:
* soc_sand_rand_array_permute_u_long
*FUNCTION:
*   Given array of u-longs 'array', the function will
*  generate one permutation of this array.
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_INOUT SOC_SAND_RAND               *r,
*      Random structure
*    SOC_SAND_INOUT unsigned char           array[],
*      Array of 'STRUCTUTE'.
*    SOC_SAND_IN    uint32           nof_items,
*      Number of 'STRUCTUTE' in 'array'
*    SOC_SAND_IN    SOC_SAND_RAND_SWAP_ARR_FUNC swap
*      Swap function of 'STRUCTUTE' in 'array'.
*OUTPUT:
*  SOC_SAND_INDIRECT:
*    + 'array' will be changed.
*    + Random structure
*    + 'swap' will be called approx. nof_items
*SEE ALSO:
*   Implementation of 'soc_sand_rand_array_permute_u_long()'.
*****************************************************/
/*
 */
typedef void (*SOC_SAND_RAND_SWAP_ARR_FUNC)(unsigned char array[], uint32 indx_1, uint32 indx_2);
/*
 */
void
  soc_sand_rand_array_permute(
    SOC_SAND_INOUT SOC_SAND_RAND               *r,
    SOC_SAND_INOUT unsigned char           array[],
    SOC_SAND_IN    uint32           nof_items,
    SOC_SAND_IN    SOC_SAND_RAND_SWAP_ARR_FUNC swap
  );

/*
 * }
 */


#if SOC_SAND_DEBUG
/* { */

/*prints the seed*/
void
  soc_sand_rand_print(
    SOC_SAND_IN SOC_SAND_RAND    *r
  );

int soc_sand_rand_test(SOC_SAND_IN uint32);

/*
 * }
 */
#endif


#ifdef  __cplusplus
}
#endif


/* } __SOC_SAND_RAND_H_INCLUDED__*/
#endif

