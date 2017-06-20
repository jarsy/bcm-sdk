/* $Id: sand_rand.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/





#include <shared/bsl.h>
#include <soc/dnx/legacy/drv.h>



#include <soc/dnx/legacy/SAND/Utils/sand_rand.h>

#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>

/* $Id: sand_rand.c,v 1.5 Broadcom SDK $
 * Internal implementation of random
 * {
 */

/* Period parameters */
#define DNX_SAND_RAND_UMASK 0x80000000UL /* most significant w-r bits */
#define DNX_SAND_RAND_LMASK 0x7fffffffUL /* least significant r bits */
#define DNX_SAND_RAND_MIXBITS(u,v) ( ((u) & DNX_SAND_RAND_UMASK) | ((v) & DNX_SAND_RAND_LMASK) )
#define DNX_SAND_RAND_TWIST(u,v) ((DNX_SAND_RAND_MIXBITS(u,v) >> 1) ^ ((v)&1UL ? DNX_SAND_RAND_MATRIX_A : 0UL))

#define DNX_SAND_RAND_INIT_INDICATOR 0x12FF87A5



/* initializes state[DNX_SAND_RAND_N] with a seed */
static void
  dnx_sand_rand_init(
    DNX_SAND_INOUT DNX_SAND_RAND*    r,
    DNX_SAND_IN    uint32 s
  )
{
  int
    j;

  if(NULL == r)
  {
    goto exit;
  }

  r->state[0]= s & 0xffffffffUL;
  for (j=1; j<DNX_SAND_RAND_N; j++)
  {
    r->state[j] = (1812433253UL * (r->state[j-1] ^ (r->state[j-1] >> 30)) + j);
    r->state[j] &= 0xffffffffUL;
  }
  r->left = 1;
  r->initf = DNX_SAND_RAND_INIT_INDICATOR;

exit:
  return;
}

/* if dnx_sand_rand_init() has not been called, */
/* a default initial seed is used           */
static void
  dnx_sand_rand_check_init(
    DNX_SAND_INOUT DNX_SAND_RAND*    r
  )
{
  if (r->initf!=DNX_SAND_RAND_INIT_INDICATOR)
  {
    dnx_sand_rand_init(r, 5489UL);
  }
}

static void
  dnx_sand_rand_next_state(
    DNX_SAND_INOUT DNX_SAND_RAND *r
  )
{
  uint32
    *p=NULL;
  int
    j;

  if(NULL == r)
  {
    goto exit;
  }

  dnx_sand_rand_check_init(r);

  p=r->state;


  r->left = DNX_SAND_RAND_N;
  r->next = r->state;

  for (j=DNX_SAND_RAND_N-DNX_SAND_RAND_M+1; --j; p++)
  {
    *p = p[DNX_SAND_RAND_M] ^ DNX_SAND_RAND_TWIST(p[0], p[1]);
  }

  for (j=DNX_SAND_RAND_M; --j; p++)
  {
    *p = p[DNX_SAND_RAND_M-DNX_SAND_RAND_N] ^ DNX_SAND_RAND_TWIST(p[0], p[1]);
  }

  *p = p[DNX_SAND_RAND_M-DNX_SAND_RAND_N] ^ DNX_SAND_RAND_TWIST(p[0], r->state[0]);
exit:
  return;
}

/* generates a random number on [0,0xffffffff]-interval */
static uint32
  dnx_sand_rand_genrand_int32(
    DNX_SAND_INOUT DNX_SAND_RAND *r
  )
{
  uint32
    y = 0;

  if(NULL == r)
  {
    goto exit;
  }

  dnx_sand_rand_check_init(r);

  if (--(r->left) == 0)
  {
    dnx_sand_rand_next_state(r);
  }
  y = *(r->next)++;

  /* Tempering */
  y ^= (y >> DNX_SAND_RAND_TEMU);
  y ^= (y << DNX_SAND_RAND_TEMS) & DNX_SAND_RAND_TEMB;
  y ^= (y << DNX_SAND_RAND_TEMT) & DNX_SAND_RAND_TEMC;
  y ^= (y >> DNX_SAND_RAND_TEML);

exit:
  return y;
}

/* generates a random number on [0,0x7fffffff]-interval */
static uint32
  dnx_sand_rand_genrand_int31(
    DNX_SAND_INOUT DNX_SAND_RAND *r
  )
{
  uint32
    y = 0;

  if(NULL == r)
  {
    goto exit;
  }

  dnx_sand_rand_check_init(r);

  if (--(r->left) == 0)
  {
    dnx_sand_rand_next_state(r);
  }
  y = *(r->next)++;

  /* Tempering */
  y ^= (y >> DNX_SAND_RAND_TEMU);
  y ^= (y << DNX_SAND_RAND_TEMS) & DNX_SAND_RAND_TEMB;
  y ^= (y << DNX_SAND_RAND_TEMT) & DNX_SAND_RAND_TEMC;
  y ^= (y >> DNX_SAND_RAND_TEML);

exit:
  return y>>1;
}

/*
 * }
 */


/*
 * API of RAND
 * {
 */

/*****************************************************
 * See details in dnx_sand_rand.h
 *****************************************************/
void
  dnx_sand_rand_seed_set(
    DNX_SAND_OUT DNX_SAND_RAND     *r,
    DNX_SAND_IN  uint32 seed
  )
{
  if(NULL == r)
  {
    goto exit;
  }

  r->seed = seed;
  dnx_sand_rand_init(r, seed);

exit:
  return;
}

/*****************************************************
 * See details in dnx_sand_rand.h
 *****************************************************/
uint32
  dnx_sand_rand_seed_get(
    DNX_SAND_IN DNX_SAND_RAND    *r
  )
{
  uint32
    seed = 0;
  if(NULL == r)
  {
    goto exit;
  }
  seed = r->seed;

exit:
  return seed;
}


/*****************************************************
 * See details in dnx_sand_rand.h
 *****************************************************/
uint32
  dnx_sand_rand_coin_1(
    DNX_SAND_INOUT DNX_SAND_RAND     *r,
    DNX_SAND_IN    uint32 positive_bias
  )
{
  uint32
    x = 0;
  if(NULL == r)
  {
    goto exit;
  }

  if(positive_bias >= DNX_SAND_RAND_PERCENT_100)
  {
    x=1;
  }
  else
  {
    if(dnx_sand_rand_modulo(r, DNX_SAND_RAND_PERCENT_100) <= positive_bias)
    {
      x=1;
    }
    else
    {
      x = 0;
    }
  }

exit:
  return x;
}


/*****************************************************
 * See details in dnx_sand_rand.h
 *****************************************************/
uint32
  dnx_sand_rand_coin_2(
    DNX_SAND_INOUT DNX_SAND_RAND     *r,
    DNX_SAND_IN    uint32 positive_bias,
    DNX_SAND_IN    uint32 negative_bias
  )
{
  uint32
    x = 0;
  if(NULL == r)
  {
    goto exit;
  }

  if(dnx_sand_rand_modulo(r, positive_bias+negative_bias) <= positive_bias)
  {
    x=1;
  }
  else
  {
    x = 0;
  }

exit:
  return x;
}


/*****************************************************
 * See details in dnx_sand_rand.h
 *****************************************************/
uint32
  dnx_sand_rand_range(
    DNX_SAND_INOUT DNX_SAND_RAND     *r,
    DNX_SAND_IN    uint32 min,
    DNX_SAND_IN    uint32 max
  )
{
  uint32
    x = 0;
  if(NULL == r)
  {
    goto exit;
  }

  if(min >= max)
  {
    x = min;
  }
  else
  {
    x = min + dnx_sand_rand_modulo(r, max-min);
  }

exit:
  return x;
}


/*****************************************************
 * See details in dnx_sand_rand.h
 *****************************************************/
int
  dnx_sand_rand_int(
    DNX_SAND_INOUT DNX_SAND_RAND     *r
  )
{
  uint32
    x = 0;
  if(NULL == r)
  {
    goto exit;
  }

  x = dnx_sand_rand_genrand_int31(r);


exit:
  return x;
}


/*****************************************************
 * See details in dnx_sand_rand.h
 *****************************************************/
uint32
  dnx_sand_rand_u_long(
    DNX_SAND_INOUT DNX_SAND_RAND     *r
  )
{
  uint32
    x = 0;
  if(NULL == r)
  {
    goto exit;
  }

  x = dnx_sand_rand_genrand_int32(r);


exit:
  return x;
}


/*****************************************************
 * See details in dnx_sand_rand.h
 *****************************************************/
uint32
  dnx_sand_rand_modulo(
    DNX_SAND_INOUT DNX_SAND_RAND     *r,
    DNX_SAND_IN    uint32 m
  )
{
  uint32
    x = 0;
  if(NULL == r)
  {
    goto exit;
  }

  x = dnx_sand_rand_genrand_int32(r) % m;

exit:
  return x;
}



/*
 * }
 */


/*
 * {
 * DNX_SAND_RAND more APIs.
 * These function are more complicated random functions.
 */


static void
  dnx_sand_rand_array_permute_u_long_swap(
    unsigned char arr_char[],
    uint32 indx_1,
    uint32 indx_2
  )
{
  uint32
    temp_val;
  uint32
    *array = (uint32*)arr_char;

  temp_val      = array[indx_1];
  array[indx_1] = array[indx_2];
  array[indx_2] = temp_val;
}

/*****************************************************
 * See details in dnx_sand_rand.h
 *****************************************************/
void
  dnx_sand_rand_array_permute_u_long(
    DNX_SAND_INOUT DNX_SAND_RAND     *r,
    DNX_SAND_INOUT uint32 array[],
    DNX_SAND_IN    uint32 nof_items
  )
{
  if( (NULL == r) ||
      (NULL == array)
    )
  {
    goto exit;
  }

  dnx_sand_rand_array_permute(
    r,
    (unsigned char*)array,
    nof_items,
    dnx_sand_rand_array_permute_u_long_swap
  );

exit:
  return;
}


/*****************************************************
 * See details in dnx_sand_rand.h
 *****************************************************/
void
  dnx_sand_rand_array_permute(
    DNX_SAND_INOUT DNX_SAND_RAND               *r,
    DNX_SAND_INOUT unsigned char           array[],
    DNX_SAND_IN    uint32           nof_items,
    DNX_SAND_IN    DNX_SAND_RAND_SWAP_ARR_FUNC swap
  )
{
  uint32
    arr_end;
  uint32
    indx_to_switch;

  if( (NULL == r) ||
      (NULL == array) ||
      (NULL == swap)
    )
  {
    goto exit;
  }

  for(arr_end=nof_items; arr_end>1;)
  {
    indx_to_switch = dnx_sand_rand_modulo(r, arr_end--);
    (*swap)(array, arr_end, indx_to_switch);
  }

exit:
  return;
}

/*
 * }
 */



#if DNX_SAND_DEBUG
/* { */


/*****************************************************
 * See details in dnx_sand_rand.h
 *****************************************************/
void
  dnx_sand_rand_print(
    DNX_SAND_IN DNX_SAND_RAND    *r
  )
{
  if(NULL == r)
  {
    LOG_CLI((BSL_META("ERROR: dnx_sand_rand_print - got NULL -\n\r")));
    goto exit;
  }
  LOG_CLI((BSL_META("%u"),
           dnx_sand_rand_seed_get(r)
           ));

exit:
  return;
}


/*****************************************************
 * This is print test. That is, no automatic test for
 * this module.
 *****************************************************/
int dnx_sand_rand_test(DNX_SAND_IN uint32 silent)
{
  int
    i,
    pass;
  uint32
    seed=0x65464;
  DNX_SAND_RAND
    dnx_sand_rand;
#define DNX_SAND_RAND_TEST_ARR_SIZE 200
  uint32
    array[DNX_SAND_RAND_TEST_ARR_SIZE];

  pass = TRUE;

  dnx_sand_rand_seed_set(&dnx_sand_rand, seed);

  LOG_INFO(BSL_LS_SOC_COMMON,
           (BSL_META("1000 outputs of dnx_sand_rand_u_long()\n")));
  for (i=0; i<1000; i++)
  {
    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META("%10u "), dnx_sand_rand_u_long(&dnx_sand_rand)));
    if (i%5==4)
    {
      LOG_INFO(BSL_LS_SOC_COMMON,
               (BSL_META("\n")));
    }
  }

  LOG_INFO(BSL_LS_SOC_COMMON,
           (BSL_META("1000 outputs of dnx_sand_rand_coin_1(, 50%%)\n")));
  for (i=0; i<1000; i++)
  {
    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META("%1u"), dnx_sand_rand_coin_1(&dnx_sand_rand, 50*DNX_SAND_RAND_PERCENT_ONE)));
    if (i%40==39)
    {
      LOG_INFO(BSL_LS_SOC_COMMON,
               (BSL_META("\n")));
    }
    else
    {
      LOG_INFO(BSL_LS_SOC_COMMON,
               (BSL_META(" ")));
    }
  }

  LOG_INFO(BSL_LS_SOC_COMMON,
           (BSL_META("1000 outputs of dnx_sand_rand_coin_2(, 1%%, 10%%)\n")));
  for (i=0; i<1000; i++)
  {
    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META("%1u"), dnx_sand_rand_coin_2(&dnx_sand_rand, 1, 10)));
    if (i%40==39)
    {
      LOG_INFO(BSL_LS_SOC_COMMON,
               (BSL_META("\n")));
    }
    else
    {
      LOG_INFO(BSL_LS_SOC_COMMON,
               (BSL_META(" ")));
    }
  }

  LOG_INFO(BSL_LS_SOC_COMMON,
           (BSL_META("2 outputs of dnx_sand_rand_array_permute_u_long()\n")));
  for (i=0; i<DNX_SAND_RAND_TEST_ARR_SIZE; i++)
  {
    array[i] = i;
  }

  dnx_sand_rand_array_permute_u_long(&dnx_sand_rand, array, DNX_SAND_RAND_TEST_ARR_SIZE);
  for (i=0; i<DNX_SAND_RAND_TEST_ARR_SIZE; i++)
  {
    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META("%3u"), array[i]));
    if (i%20==19)
    {
      LOG_INFO(BSL_LS_SOC_COMMON,
               (BSL_META("\n")));
    }
    else
    {
      LOG_INFO(BSL_LS_SOC_COMMON,
               (BSL_META(" ")));
    }
  }

  LOG_INFO(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));
  dnx_sand_rand_array_permute_u_long(&dnx_sand_rand, array, DNX_SAND_RAND_TEST_ARR_SIZE);
  for (i=0; i<DNX_SAND_RAND_TEST_ARR_SIZE; i++)
  {
    LOG_INFO(BSL_LS_SOC_COMMON,
             (BSL_META("%3u"), array[i]));
    if (i%20==19)
    {
      LOG_INFO(BSL_LS_SOC_COMMON,
               (BSL_META("\n")));
    }
    else
    {
      LOG_INFO(BSL_LS_SOC_COMMON,
               (BSL_META(" ")));
    }
  }



  return pass;
}

/*
 * }
 */
#endif


