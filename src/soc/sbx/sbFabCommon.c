/*
 * $Id: sbFabCommon.c,v 1.62 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Link configuration for QE, BME
 *
 */

#include <shared/bsl.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/sirius.h>
#include "bm9600_properties.h"
#include <bcm/fabric.h>

#define printf bsl_printf

/**********************************************************************
 Rambus RS2314 driver strength is based upon the following table:


HiDrv, LoDrv, I(Nom), Dtx (dec), Dtx (bin), ratio, I(Tx) mA, distributed value
  0,     1,    10,       8,       1000,     0.60,   6,        16
  0,     1,    10,       9,       1001,     0.65,   6.5,      17
  0,     1,    10,       10,      1010,     0.70,   7,        19
  0,     1,    10,       11,      1011,     0.75,   7.5,      20
  0,     1,    10,       12,      1100,     0.80,   8,        21
  0,     1,    10,       13,      1101,     0.85,   8.5,      22
  0,     1,    10,       14,      1110,     0.90,   9,        24
  0,     1,    10,       15,      1111,     0.95,   9.5,      25
  0,     1,    10,       0,       0000,     1.00,   10,       26
  0,     1,    10,       1,       0001,     1.05,   10.5,     28
  0,     1,    10,       2,       0010,     1.10,   11,       29
  0,     1,    10,       3,       0011,     1.15,   11.5,     30
  0,     0,    20,       8,       1000,     0.60,   12,       32 (not used)
  0,     1,    10,       4,       0100,     1.20,   12,       32
  0,     1,    10,       5,       0101,     1.25,   12.5,     33
  0,     0,    20,       9,       1001,     0.65,   13,       34
  0,     1,    10,       6,       0110,     1.30,   13,       34 (not used)
  0,     1,    10,       7,       0111,     1.35,   13.5,     36
  0,     0,    20,       10,      1010,     0.70,   14,       37
  0,     0,    20,       11,      1011,     0.75,   15,       40
  0,     0,    20,       12,      1100,     0.80,   16,       42
  1,     0,    28,       8,       1000,     0.60,   16.8,     44
  0,     0,    20,       13,      1101,     0.85,   17,       45
  0,     0,    20,       14,      1110,     0.90,   18,       48
  1,     0,    28,       9,       1001,     0.65,   18.2,     48 (not used)
  0,     0,    20,       15,      1111,     0.95,   19,       50
  1,     0,    28,       10,      1010,     0.70,   19.6,     52
  0,     0,    20,       0,       0000,     1.00,   20,       53
  0,     0,    20,       1,       0001,     1.05,   21,       56
  1,     0,    28,       11,      1011,     0.75,   21,       56 (not used)
  0,     0,    20,       2,       0010,     1.10,   22,       58
  1,     0,    28,       12,      1100,     0.80,   22.4,     59
  0,     0,    20,       3,       0011,     1.15,   23,       61
  1,     0,    28,       13,      1101,     0.85,   23.8,     63
  0,     0,    20,       4,       0100,     1.20,   24,       63 (not used)
  0,     0,    20,       5,       0101,     1.25,   25,       66
  1,     0,    28,       14,      1110,     0.90,   25.2,     67
  0,     0,    20,       6,       0110,     1.30,   26,       69
  1,     0,    28,       15,      1111,     0.95,   26.6,     70
  0,     0,    20,       7,       0111,     1.35,   27,       71
  1,     0,    28,       0,       0000,     1.00,   28,       74
  1,     0,    28,       1,       0001,     1.05,   29.4,     78
  1,     0,    28,       2,       0010,     1.10,   30.8,     81
  1,     0,    28,       3,       0011,     1.15,   32.2,     85
  1,     0,    28,       4,       0100,     1.20,   33.6,     89
  1,     0,    28,       5,       0101,     1.25,   35,       93
  1,     0,    28,       6,       0110,     1.30,   36.4,     96
  1,     0,    28,       7,       0111,     1.35,   37.8,     100
**************************************************************************************/


static sbLinkSpecificDriverConfigRambusRs3214_t s_RambusRs2314Config[101] = {
    /* |- nHiDrv                             */
    /* |                                     */
    /* |  |- nLowDrv                         */
    /* |  |                                  */
    /* |  |   |- nDtx                        */
    /* |  |   |                              */
    /* |  |   |  |- nDeq (overridden later)  */
    /* |  |   |  |                           */
    /* |  |   |  |         |- strength index */
    /* |  |   |  |         |                 */
    {  0, 1,  8, 0},  /*   0 */
    {  0, 1,  8, 0},  /*   1 */
    {  0, 1,  8, 0},  /*   2 */
    {  0, 1,  8, 0},  /*   3 */
    {  0, 1,  8, 0},  /*   4 */
    {  0, 1,  8, 0},  /*   5 */
    {  0, 1,  8, 0},  /*   6 */
    {  0, 1,  8, 0},  /*   7 */
    {  0, 1,  8, 0},  /*   8 */
    {  0, 1,  8, 0},  /*   9 */

    {  0, 1,  8, 0},  /*  10 */
    {  0, 1,  8, 0},  /*  11 */
    {  0, 1,  8, 0},  /*  12 */
    {  0, 1,  8, 0},  /*  13 */
    {  0, 1,  8, 0},  /*  14 */
    {  0, 1,  8, 0},  /*  15 */
    {  0, 1,  8, 0},  /*  16 */
    {  0, 1,  9, 0},  /*  17 */
    {  0, 1, 10, 0},  /*  18 */
    {  0, 1, 10, 0},  /*  19 */
    {  0, 1, 11, 0},  /*  20 */

    {  0, 1, 12, 0},  /*  21 */
    {  0, 1, 13, 0},  /*  22 */
    {  0, 1, 14, 0},  /*  23 */
    {  0, 1, 14, 0},  /*  24 */
    {  0, 1, 15, 0},  /*  25 */
    {  0, 1,  0, 0},  /*  26 */
    {  0, 1,  1, 0},  /*  27 */
    {  0, 1,  1, 0},  /*  28 */
    {  0, 1,  2, 0},  /*  29 */
    {  0, 1,  3, 0},  /*  30 */

    {  0, 1,  4, 0},  /*  31 */
    {  0, 1,  4, 0},  /*  32 */
    {  0, 1,  5, 0},  /*  33 */
    {  0, 0,  9, 0},  /*  34 */
    {  0, 1,  7, 0},  /*  35 */
    {  0, 1,  7, 0},  /*  36 */
    {  0, 0, 10, 0},  /*  37 */
    {  0, 0, 10, 0},  /*  38 */
    {  0, 0, 11, 0},  /*  39 */
    {  0, 0, 11, 0},  /*  40 */

    {  0, 0, 12, 0},  /*  41 */
    {  0, 0, 12, 0},  /*  42 */
    {  1, 0,  8, 0},  /*  43 */
    {  1, 0,  8, 0},  /*  44 */
    {  0, 0, 13, 0},  /*  45 */
    {  0, 0, 13, 0},  /*  46 */
    {  0, 0, 14, 0},  /*  47 */
    {  0, 0, 14, 0},  /*  48 */
    {  0, 0, 15, 0},  /*  49 */
    {  0, 0, 15, 0},  /*  50 */

    {  1, 0, 10, 0},  /*  51 */
    {  1, 0, 10, 0},  /*  52 */
    {  0, 0,  0, 0},  /*  53 */
    {  0, 0,  0, 0},  /*  54 */
    {  0, 0,  1, 0},  /*  55 */
    {  0, 0,  1, 0},  /*  56 */
    {  0, 0,  2, 0},  /*  57 */
    {  0, 0,  2, 0},  /*  58 */
    {  1, 0, 12, 0},  /*  59 */
    {  0, 0,  3, 0},  /*  60 */

    {  0, 0,  3, 0},  /*  61 */
    {  1, 0, 13, 0},  /*  62 */
    {  1, 0, 13, 0},  /*  63 */
    {  1, 0, 13, 0},  /*  64 */
    {  0, 0,  5, 0},  /*  65 */
    {  0, 0,  5, 0},  /*  66 */
    {  1, 0, 14, 0},  /*  67 */
    {  0, 0,  6, 0},  /*  68 */
    {  0, 0,  6, 0},  /*  69 */
    {  1, 0, 15, 0},  /*  70 */

    {  0, 0,  7, 0},  /*  71 */
    {  0, 0,  7, 0},  /*  72 */
    {  1, 0,  0, 0},  /*  73 */
    {  1, 0,  0, 0},  /*  74 */
    {  1, 0,  0, 0},  /*  75 */
    {  1, 0,  1, 0},  /*  76 */
    {  1, 0,  1, 0},  /*  77 */
    {  1, 0,  1, 0},  /*  78 */
    {  1, 0,  1, 0},  /*  79 */
    {  1, 0,  2, 0},  /*  80 */

    {  1, 0,  2, 0},  /*  81 */
    {  1, 0,  2, 0},  /*  82 */
    {  1, 0,  2, 0},  /*  83 */
    {  1, 0,  3, 0},  /*  84 */
    {  1, 0,  3, 0},  /*  85 */
    {  1, 0,  3, 0},  /*  86 */
    {  1, 0,  3, 0},  /*  87 */
    {  1, 0,  4, 0},  /*  88 */
    {  1, 0,  4, 0},  /*  89 */
    {  1, 0,  4, 0},  /*  90 */

    {  1, 0,  5, 0},  /*  91 */
    {  1, 0,  5, 0},  /*  92 */
    {  1, 0,  5, 0},  /*  93 */
    {  1, 0,  5, 0},  /*  94 */
    {  1, 0,  6, 0},  /*  95 */
    {  1, 0,  6, 0},  /*  96 */
    {  1, 0,  6, 0},  /*  97 */
    {  1, 0,  6, 0},  /*  98 */
    {  1, 0,  7, 0},  /*  99 */
    {  1, 0,  7, 0}   /* 100 */
};

static void
GetLinkSpecificConfigForRs2314(sbLinkDriverConfig_t *pLinkDriverConfig,
                               sbLinkSpecificDriverConfig_t *pSpecificConfig)
{
    /* Set nHiDrv, nLoDrv, nDtx from table */
    pSpecificConfig->u.rambusRs2314 = \
            s_RambusRs2314Config[pLinkDriverConfig->uDriverStrength];

    /* Set nDeq based on pLinkDriverConfig->uDriverEqualization */
    /* For this link type, we can run in the range of [0,15] for nDeq */
    pSpecificConfig->u.rambusRs2314.nDeq = \
            (pLinkDriverConfig->uDriverEqualization * 100 ) / 666;

    { /* ensure that nDeq is in its valid range */
        if ( pSpecificConfig->u.rambusRs2314.nDeq < 0 ) {
            pSpecificConfig->u.rambusRs2314.nDeq = 0;
        }

        if ( pSpecificConfig->u.rambusRs2314.nDeq > 15 ) {
            pSpecificConfig->u.rambusRs2314.nDeq = 15;
        }
    }
}


static sbLinkSpecificDriverConfigHypercoreEqualization_t s_HypercoreConfigEqualization[LINK_SPECIFIC_CONFIG_EQUALIZATION_HYPERCORE_MAX] = {
    /*  |-  nPreemphasisPost                      */
    /*  |                                         */
    /*  |                                         */
    /*  |                                         */
    /*  |                                         */
    /*  |                                         */
    /*  |         |- nPreemphasisPre              */
    /*  |         |  (for 3.125G, this will be    */
    /*  |         |   overwritten to 0)           */
    /*  |         |            |- strength index  */
    /*  |         |            |                  */
    {  0x00,     0x00    }, /*   0  */
    {  0x00,     0x01    }, /*   1  */
    {  0x00,     0x02    }, /*   2  */
    {  0x00,     0x03    }, /*   3  */
    {  0x00,     0x04    }, /*   4  */
    {  0x00,     0x05    }, /*   5  */
    {  0x00,     0x06    }, /*   6  */
    {  0x00,     0x07    }, /*   7  */
    {  0x00,     0x08    }, /*   8  */
    {  0x00,     0x09    }, /*   9  */
    {  0x00,     0x0a    }, /*   10 */
    {  0x00,     0x0b    }, /*   11 */
    {  0x00,     0x0c    }, /*   12 */
    {  0x00,     0x0d    }, /*   13 */
    {  0x00,     0x0e    }, /*   14 */
    {  0x00,     0x0f    }, /*   15 */
    {  0x01,     0x00    }, /*   16 */
    {  0x01,     0x01    }, /*   17 */
    {  0x01,     0x02    }, /*   18 */
    {  0x01,     0x03    }, /*   19 */
    {  0x01,     0x04    }, /*   20 */
    {  0x01,     0x05    }, /*   21 */
    {  0x01,     0x06    }, /*   22 */
    {  0x01,     0x07    }, /*   23 */
    {  0x01,     0x08    }, /*   24 */
    {  0x01,     0x09    }, /*   25 */
    {  0x01,     0x0a    }, /*   26 */
    {  0x01,     0x0b    }, /*   27 */
    {  0x01,     0x0c    }, /*   28 */
    {  0x01,     0x0d    }, /*   29 */
    {  0x01,     0x0e    }, /*   30 */
    {  0x01,     0x0f    }, /*   31 */
    {  0x02,     0x00    }, /*   32 */
    {  0x02,     0x01    }, /*   33 */
    {  0x02,     0x02    }, /*   34 */
    {  0x02,     0x03    }, /*   35 */
    {  0x02,     0x04    }, /*   36 */
    {  0x02,     0x05    }, /*   37 */
    {  0x02,     0x06    }, /*   38 */
    {  0x02,     0x07    }, /*   39 */
    {  0x02,     0x08    }, /*   40 */
    {  0x02,     0x09    }, /*   41 */
    {  0x02,     0x0a    }, /*   42 */
    {  0x02,     0x0b    }, /*   43 */
    {  0x02,     0x0c    }, /*   44 */
    {  0x02,     0x0d    }, /*   45 */
    {  0x02,     0x0e    }, /*   46 */
    {  0x02,     0x0f    }, /*   47 */
    {  0x03,     0x00    }, /*   48 */
    {  0x03,     0x01    }, /*   49 */
    {  0x03,     0x02    }, /*   50 */
    {  0x03,     0x03    }, /*   51 */
    {  0x03,     0x04    }, /*   52 */
    {  0x03,     0x05    }, /*   53 */
    {  0x03,     0x06    }, /*   54 */
    {  0x03,     0x07    }, /*   55 */
    {  0x03,     0x08    }, /*   56 */
    {  0x03,     0x09    }, /*   57 */
    {  0x03,     0x0a    }, /*   58 */
    {  0x03,     0x0b    }, /*   59 */
    {  0x03,     0x0c    }, /*   60 */
    {  0x03,     0x0d    }, /*   61 */
    {  0x03,     0x0e    }, /*   62 */
    {  0x03,     0x0f    }, /*   63 */
    {  0x04,     0x00    }, /*   64 */
    {  0x04,     0x01    }, /*   65 */
    {  0x04,     0x02    }, /*   66 */
    {  0x04,     0x03    }, /*   67 */
    {  0x04,     0x04    }, /*   68 */
    {  0x04,     0x05    }, /*   69 */
    {  0x04,     0x06    }, /*   70 */
    {  0x04,     0x07    }, /*   71 */
    {  0x04,     0x08    }, /*   72 */
    {  0x04,     0x09    }, /*   73 */
    {  0x04,     0x0a    }, /*   74 */
    {  0x04,     0x0b    }, /*   75 */
    {  0x04,     0x0c    }, /*   76 */
    {  0x04,     0x0d    }, /*   77 */
    {  0x04,     0x0e    }, /*   78 */
    {  0x04,     0x0f    }, /*   79 */
    {  0x05,     0x00    }, /*   80 */
    {  0x05,     0x01    }, /*   81 */
    {  0x05,     0x02    }, /*   82 */
    {  0x05,     0x03    }, /*   83 */
    {  0x05,     0x04    }, /*   84 */
    {  0x05,     0x05    }, /*   85 */
    {  0x05,     0x06    }, /*   86 */
    {  0x05,     0x07    }, /*   87 */
    {  0x05,     0x08    }, /*   88 */
    {  0x05,     0x09    }, /*   89 */
    {  0x05,     0x0a    }, /*   90 */
    {  0x05,     0x0b    }, /*   91 */
    {  0x05,     0x0c    }, /*   92 */
    {  0x05,     0x0d    }, /*   93 */
    {  0x05,     0x0e    }, /*   94 */
    {  0x05,     0x0f    }, /*   95 */
    {  0x06,     0x00    }, /*   96 */
    {  0x06,     0x01    }, /*   97 */
    {  0x06,     0x02    }, /*   98 */
    {  0x06,     0x03    }, /*   99 */
    {  0x06,     0x04    }, /*   100 */
    {  0x06,     0x05    }, /*   101 */
    {  0x06,     0x06    }, /*   102 */
    {  0x06,     0x07    }, /*   103 */
    {  0x06,     0x08    }, /*   104 */
    {  0x06,     0x09    }, /*   105 */
    {  0x06,     0x0a    }, /*   106 */
    {  0x06,     0x0b    }, /*   107 */
    {  0x06,     0x0c    }, /*   108 */
    {  0x06,     0x0d    }, /*   109 */
    {  0x06,     0x0e    }, /*   110 */
    {  0x06,     0x0f    }, /*   111 */
    {  0x07,     0x00    }, /*   112 */
    {  0x07,     0x01    }, /*   113 */
    {  0x07,     0x02    }, /*   114 */
    {  0x07,     0x03    }, /*   115 */
    {  0x07,     0x04    }, /*   116 */
    {  0x07,     0x05    }, /*   117 */
    {  0x07,     0x06    }, /*   118 */
    {  0x07,     0x07    }, /*   119 */
    {  0x07,     0x08    }, /*   120 */
    {  0x07,     0x09    }, /*   121 */
    {  0x07,     0x0a    }, /*   122 */
    {  0x07,     0x0b    }, /*   123 */
    {  0x07,     0x0c    }, /*   124 */
    {  0x07,     0x0d    }, /*   125 */
    {  0x07,     0x0e    }, /*   126 */
    {  0x07,     0x0f    }, /*   127 */
    {  0x08,     0x00    }, /*   128 */
    {  0x08,     0x01    }, /*   129 */
    {  0x08,     0x02    }, /*   130 */
    {  0x08,     0x03    }, /*   131 */
    {  0x08,     0x04    }, /*   132 */
    {  0x08,     0x05    }, /*   133 */
    {  0x08,     0x06    }, /*   134 */
    {  0x08,     0x07    }, /*   135 */
    {  0x08,     0x08    }, /*   136 */
    {  0x08,     0x09    }, /*   137 */
    {  0x08,     0x0a    }, /*   138 */
    {  0x08,     0x0b    }, /*   139 */
    {  0x08,     0x0c    }, /*   140 */
    {  0x08,     0x0d    }, /*   141 */
    {  0x08,     0x0e    }, /*   142 */
    {  0x08,     0x0f    }, /*   143 */
    {  0x09,     0x00    }, /*   144 */
    {  0x09,     0x01    }, /*   145 */
    {  0x09,     0x02    }, /*   146 */
    {  0x09,     0x03    }, /*   147 */
    {  0x09,     0x04    }, /*   148 */
    {  0x09,     0x05    }, /*   149 */
    {  0x09,     0x06    }, /*   150 */
    {  0x09,     0x07    }, /*   151 */
    {  0x09,     0x08    }, /*   152 */
    {  0x09,     0x09    }, /*   153 */
    {  0x09,     0x0a    }, /*   154 */
    {  0x09,     0x0b    }, /*   155 */
    {  0x09,     0x0c    }, /*   156 */
    {  0x09,     0x0d    }, /*   157 */
    {  0x09,     0x0e    }, /*   158 */
    {  0x09,     0x0f    }, /*   159 */
    {  0x0a,     0x00    }, /*   160 */
    {  0x0a,     0x01    }, /*   161 */
    {  0x0a,     0x02    }, /*   162 */
    {  0x0a,     0x03    }, /*   163 */
    {  0x0a,     0x04    }, /*   164 */
    {  0x0a,     0x05    }, /*   165 */
    {  0x0a,     0x06    }, /*   166 */
    {  0x0a,     0x07    }, /*   167 */
    {  0x0a,     0x08    }, /*   168 */
    {  0x0a,     0x09    }, /*   169 */
    {  0x0a,     0x0a    }, /*   170 */
    {  0x0a,     0x0b    }, /*   171 */
    {  0x0a,     0x0c    }, /*   172 */
    {  0x0a,     0x0d    }, /*   173 */
    {  0x0a,     0x0e    }, /*   174 */
    {  0x0a,     0x0f    }, /*   175 */
    {  0x0b,     0x00    }, /*   176 */
    {  0x0b,     0x01    }, /*   177 */
    {  0x0b,     0x02    }, /*   178 */
    {  0x0b,     0x03    }, /*   179 */
    {  0x0b,     0x04    }, /*   180 */
    {  0x0b,     0x05    }, /*   181 */
    {  0x0b,     0x06    }, /*   182 */
    {  0x0b,     0x07    }, /*   183 */
    {  0x0b,     0x08    }, /*   184 */
    {  0x0b,     0x09    }, /*   185 */
    {  0x0b,     0x0a    }, /*   186 */
    {  0x0b,     0x0b    }, /*   187 */
    {  0x0b,     0x0c    }, /*   188 */
    {  0x0b,     0x0d    }, /*   189 */
    {  0x0b,     0x0e    }, /*   190 */
    {  0x0b,     0x0f    }, /*   191 */
    {  0x0c,     0x00    }, /*   192 */
    {  0x0c,     0x01    }, /*   193 */
    {  0x0c,     0x02    }, /*   194 */
    {  0x0c,     0x03    }, /*   195 */
    {  0x0c,     0x04    }, /*   196 */
    {  0x0c,     0x05    }, /*   197 */
    {  0x0c,     0x06    }, /*   198 */
    {  0x0c,     0x07    }, /*   199 */
    {  0x0c,     0x08    }, /*   200 */
    {  0x0c,     0x09    }, /*   201 */
    {  0x0c,     0x0a    }, /*   202 */
    {  0x0c,     0x0b    }, /*   203 */
    {  0x0c,     0x0c    }, /*   204 */
    {  0x0c,     0x0d    }, /*   205 */
    {  0x0c,     0x0e    }, /*   206 */
    {  0x0c,     0x0f    }, /*   207 */
    {  0x0d,     0x00    }, /*   208 */
    {  0x0d,     0x01    }, /*   209 */
    {  0x0d,     0x02    }, /*   210 */
    {  0x0d,     0x03    }, /*   211 */
    {  0x0d,     0x04    }, /*   212 */
    {  0x0d,     0x05    }, /*   213 */
    {  0x0d,     0x06    }, /*   214 */
    {  0x0d,     0x07    }, /*   215 */
    {  0x0d,     0x08    }, /*   216 */
    {  0x0d,     0x09    }, /*   217 */
    {  0x0d,     0x0a    }, /*   218 */
    {  0x0d,     0x0b    }, /*   219 */
    {  0x0d,     0x0c    }, /*   220 */
    {  0x0d,     0x0d    }, /*   221 */
    {  0x0d,     0x0e    }, /*   222 */
    {  0x0d,     0x0f    }, /*   223 */
    {  0x0e,     0x00    }, /*   224 */
    {  0x0e,     0x01    }, /*   225 */
    {  0x0e,     0x02    }, /*   226 */
    {  0x0e,     0x03    }, /*   227 */
    {  0x0e,     0x04    }, /*   228 */
    {  0x0e,     0x05    }, /*   229 */
    {  0x0e,     0x06    }, /*   230 */
    {  0x0e,     0x07    }, /*   231 */
    {  0x0e,     0x08    }, /*   232 */
    {  0x0e,     0x09    }, /*   233 */
    {  0x0e,     0x0a    }, /*   234 */
    {  0x0e,     0x0b    }, /*   235 */
    {  0x0e,     0x0c    }, /*   236 */
    {  0x0e,     0x0d    }, /*   237 */
    {  0x0e,     0x0e    }, /*   238 */
    {  0x0e,     0x0f    }, /*   239 */
    {  0x0e,     0x00    }, /*   240 */
    {  0x0e,     0x01    }, /*   241 */
    {  0x0e,     0x02    }, /*   242 */
    {  0x0e,     0x03    }, /*   243 */
    {  0x0e,     0x04    }, /*   244 */
    {  0x0e,     0x05    }, /*   245 */
    {  0x0e,     0x06    }, /*   246 */
    {  0x0e,     0x07    }, /*   247 */
    {  0x0e,     0x08    }, /*   248 */
    {  0x0e,     0x09    }, /*   249 */
    {  0x0e,     0x0a    }, /*   250 */
    {  0x0e,     0x0b    }, /*   251 */
    {  0x0e,     0x0c    }, /*   252 */
    {  0x0e,     0x0d    }, /*   253 */
    {  0x0e,     0x0e    }, /*   254 */
    {  0x0e,     0x0f    }, /*   255 */
};


static sbLinkSpecificDriverConfigHypercoreStrength_t s_HypercoreConfigStrength[LINK_SPECIFIC_CONFIG_DRIVE_STRENGTH_HYPERCORE_MAX] = {
    /*                                    */
    /*    |- nIDriver                     */
    /*    |                               */
    /*    |   |- nIPreDriver              */
    /*    |   |                           */
    /*    |   |                           */
    /*    |   |                           */
    /*    |   |                           */
    /*    |   |         |- strength index */
    /*    |   |         |                 */
    {  0x00,  0x05}, /*   0 */
    {  0x01,  0x05}, /*   1 */
    {  0x02,  0x06}, /*   2 */
    {  0x03,  0x06}, /*   3 */
    {  0x04,  0x07}, /*   4 */
    {  0x05,  0x07}, /*   5 */
    {  0x06,  0x08}, /*   6 */
    {  0x07,  0x08}, /*   7 */
    {  0x08,  0x09}, /*   8 */
    {  0x09,  0x09}, /*   9 */
    {  0x0a,  0x0a}, /*   10 */
    {  0x0b,  0x0a}, /*   11 */
    {  0x0c,  0x0b}, /*   12 */
    {  0x0d,  0x0b}, /*   13 */
    {  0x0e,  0x0c}, /*   14 */
    {  0x0f,  0x0c}, /*   15 */
};

static void
GetLinkSpecificConfigForHypercore(sbLinkDriverConfig_t *pLinkDriverConfig,
				  sbLinkSpecificDriverConfig_t *pSpecificConfig)
{
    if (pLinkDriverConfig->uDriverStrength >= LINK_SPECIFIC_CONFIG_DRIVE_STRENGTH_HYPERCORE_MAX) {
	LOG_CLI((BSL_META("WARNING: Maximum drive strength setting is %d for hypercore, using this value instead of requested(%d)\n"),
                 LINK_SPECIFIC_CONFIG_DRIVE_STRENGTH_HYPERCORE_MAX - 1, pLinkDriverConfig->uDriverStrength));
	pLinkDriverConfig->uDriverStrength = LINK_SPECIFIC_CONFIG_DRIVE_STRENGTH_HYPERCORE_MAX - 1;
    }

    pSpecificConfig->u.hypercore.strength = \
	s_HypercoreConfigStrength[pLinkDriverConfig->uDriverStrength];


    if (pLinkDriverConfig->uDriverEqualization >= LINK_SPECIFIC_CONFIG_EQUALIZATION_HYPERCORE_MAX) {
	LOG_CLI((BSL_META("WARNING: Maximum drive equalization setting is %d for hypercore, using this value instead of requested(%d)\n"),
                 LINK_SPECIFIC_CONFIG_EQUALIZATION_HYPERCORE_MAX - 1, pLinkDriverConfig->uDriverEqualization));
	pLinkDriverConfig->uDriverEqualization = LINK_SPECIFIC_CONFIG_EQUALIZATION_HYPERCORE_MAX - 1;
    }

    pSpecificConfig->u.hypercore.equalization = \
	s_HypercoreConfigEqualization[pLinkDriverConfig->uDriverEqualization];

    /* If we are running at 6.25 half-speed, we will override preemphasis_pre for this link when we initialize the hw */
}

/*
 * at 3.125G, the period is 3.2x10^-10.  The window size is in clocks
 * with a 4ns clock, there are 12.5 serializer clocks per internal clock.
 * The uLsThreshold is the number of bit errors
 * The uLsWindow is the number of 256 internal clock cycle
 * to get the error rate:
 * uLsThreshold / (uLsWindow*256*12.5 bits)
 * Note that the lower the linkThreshold index, the higher the error tolerance.
 * In general, a low error tolerance is preferred.
 */
static
sbLinkThresholdConfig_t s_linkThresholdConfig[101] = {
    /* |- uLsThreshold              */
    /* |                            */
    /* |  |- uLsWindowIn256Clocks   */
    /* |  |            tolerable    */
    /* |  |            error        */
    /* |  |         |- rate index   */
    /* |  |         |               */
    {255, 255}, /*  0  7.9 x 10^-2  */
    {255, 255}, /*  1  7.9 x 10^-2  */
    {127, 255}, /*  2  3.9 x 10^-2  */
    {63,  255}, /*  3  1.9 x 10^-2  */
    {255,   7}, /*  4  1.1 x 10^-2  */
    {31,    1}, /*  5  9.6 x 10^-3  */
    {127,   7}, /*  6  5.7 x 10^-3  */
    {255,  15}, /*  7  5.3 x 10^-3  */
    {15,    1}, /*  8  4.6 x 10^-3  */
    {63,    7}, /*  9  2.8 x 10^-3  */
    {127,  15}, /* 10  2.6 x 10^-3  */
    {255,  31}, /* 11  2.6 x 10^-3  */
    {7,     1}, /* 12  2.2 x 10^-3  */
    {31,    7}, /* 13  1.4 x 10^-3  */
    {63,   15}, /* 14  1.3 x 10^-3  */
    {127,  31}, /* 15  1.3 x 10^-3  */
    {255,  63}, /* 16  1.3 x 10^-3  */
    {3,     1}, /* 17  9.4 x 10^-4  */
    {255,  95}, /* 18  8.4 x 10^-4  */
    {255, 111}, /* 19  7.2 x 10^-4  */
    {15,    7}, /* 20  6.7 x 10^-4  */
    {31,   15}, /* 21  6.5 x 10^-4  */
    {63,   31}, /* 22  6.4 x 10^-4  */
    {255, 127}, /* 24  6.3 x 10^-4  */
    {127,  63}, /* 23  6.3 x 10^-4  */
    {255, 159}, /* 25  5.0 x 10^-4  */
    {127,  95}, /* 26  4.2 x 10^-4  */
    {255, 191}, /* 27  4.2 x 10^-4  */
    {255, 223}, /* 28  3.6 x 10^-4  */
    {127, 111}, /* 29  3.6 x 10^-4  */
    {1,     1}, /* 30  3.1 x 10^-4  */
    {7,     7}, /* 31  3.1 x 10^-4  */
    {15,   15}, /* 32  3.1 x 10^-4  */
    {31,   31}, /* 33  3.1 x 10^-4  */
    {63,   63}, /* 34  3.1 x 10^-4  */
    {127, 127}, /* 35  3.1 x 10^-4  */
    {255, 255}, /* 36  3.1 x 10^-4  */
    {127, 159}, /* 37  2.5 x 10^-4  */
    {63,   95}, /* 38  2.1 x 10^-4  */
    {127, 191}, /* 39  2.1 x 10^-4  */
    {63,  111}, /* 40  1.8 x 10^-4  */
    {127, 223}, /* 41  1.8 x 10^-4  */
    {63,  127}, /* 42  1.6 x 10^-4  */
    {127, 255}, /* 43  1.6 x 10^-4  */
    {7,    15}, /* 44  1.5 x 10^-4  */
    {15,   31}, /* 45  1.5 x 10^-4  */
    {31,   63}, /* 46  1.5 x 10^-4  */
    {3,     7}, /* 47  1.3 x 10^-4  */
    {63,  159}, /* 48  1.2 x 10^-4  */
    {31,   95}, /* 49  1.0 x 10^-4  */
    {63,  191}, /* 50  1.0 x 10^-4  */
    {63,  223}, /* 51  8.8 x 10^-5  */
    {31,  111}, /* 52  8.7 x 10^-5  */
    {63,  255}, /* 53  7.7 x 10^-5  */
    {31,  127}, /* 54  7.6 x 10^-5  */
    {15,   63}, /* 55  7.4 x 10^-5  */
    {7,    31}, /* 56  7.1 x 10^-5  */
    {3,    15}, /* 57  6.3 x 10^-5  */
    {31,  159}, /* 58  6.1 x 10^-5  */
    {31,  191}, /* 59  5.1 x 10^-5  */
    {15,   95}, /* 60  4.9 x 10^-5  */
    {1,     7}, /* 61  4.5 x 10^-5  */
    {31,  223}, /* 62  4.3 x 10^-5  */
    {15,  111}, /* 63  4.2 x 10^-5  */
    {31,  255}, /* 64  3.8 x 10^-5  */
    {15,  127}, /* 65  3.7 x 10^-5  */
    {7,    63}, /* 66  3.5 x 10^-5  */
    {3,    31}, /* 67  3.0 x 10^-5  */
    {15,  159}, /* 68  2.9 x 10^-5  */
    {15,  191}, /* 69  2.5 x 10^-5  */
    {7,    95}, /* 70  2.3 x 10^-5  */
    {1,    15}, /* 71  2.1 x 10^-5  */
    {15,  223}, /* 72  2.1 x 10^-5  */
    {7,   111}, /* 73  2.0 x 10^-5  */
    {15,  255}, /* 74  1.8 x 10^-5  */
    {7,   127}, /* 75  1.7 x 10^-5  */
    {3,    63}, /* 76  1.5 x 10^-5  */
    {7,   159}, /* 77  1.4 x 10^-5  */
    {7,   191}, /* 78  1.1 x 10^-5  */
    {1,    31}, /* 79  1.0 x 10^-5  */
    {3,    95}, /* 80  9.9 x 10^-6  */
    {7,   223}, /* 81  9.8 x 10^-6  */
    {7,   255}, /* 82  8.6 x 10^-6  */
    {3,   111}, /* 83  8.4 x 10^-6  */
    {3,   127}, /* 84  7.4 x 10^-6  */
    {3,   159}, /* 85  5.9 x 10^-6  */
    {1,    63}, /* 86  4.9 x 10^-6  */
    {3,   191}, /* 87  4.9 x 10^-6  */
    {3,   223}, /* 88  4.2 x 10^-6  */
    {3,   255}, /* 89  3.7 x 10^-6  */
    {1,    95}, /* 90  3.3 x 10^-6  */
    {1,   111}, /* 91  2.8 x 10^-6  */
    {1,   127}, /* 92  2.5 x 10^-6  */
    {1,   159}, /* 93  2.0 x 10^-6  */
    {1,   191}, /* 94  1.6 x 10^-6  */
    {1,   223}, /* 95  1.4 x 10^-6  */
    {1,   255}, /* 96  1.2 x 10^-6  */
    {1,   255}, /* 97  1.2 x 10^-6  */
    {1,   255}, /* 98  1.2 x 10^-6  */
    {1,   255}, /* 99  1.2 x 10^-6  */
    {1,   255}  /* 100 1.2 x 10^-6  */
};

sbFabStatus_t
GetLinkThresholdConfig(uint32 uLinkThresholdIndex,
		       sbLinkThresholdConfig_t *pLinkThresholdConfig)
{

    SB_FAB_ENTER();
    SB_FAB_VERIFY(pLinkThresholdConfig);

    if (uLinkThresholdIndex > 100) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
	          (BSL_META("Link threshold index should be between 0"
	           " and 100, value (0%d)\n"), uLinkThresholdIndex));
	return SB_FAB_STATUS_BAD_INIT_PARAMS;
    }

    pLinkThresholdConfig->uLsThreshold = s_linkThresholdConfig[uLinkThresholdIndex].uLsThreshold;
    pLinkThresholdConfig->uLsWindowIn256Clocks    = s_linkThresholdConfig[uLinkThresholdIndex].uLsWindowIn256Clocks;

    return SB_FAB_STATUS_OK;
}


static int32
GetLinkImplementationType(sbFabUserDeviceHandle_t devHandle, int32 nLink) {
    SB_FAB_ENTER();
    /* SB_FAB_VERIFY(pTargetDevice); */

    switch (SOC_SBX_CONTROL((int)devHandle)->fabtype) {
	case SB_FAB_DEVICE_TME2000:
	case SB_FAB_DEVICE_QE2000:
        case SB_FAB_DEVICE_BM3200:
        case SB_FAB_DEVICE_SE4000:
        case SB_FAB_DEVICE_LCM4000:
	  /* for the QE2000 and BM3200 all links are rambus */
	    return SB_FAB_TARGET_DEVICE_LINK_IMPLEMENTATION_TYPE_RAMBUS_RS2314;
        case SB_FAB_DEVICE_BM9600:
        case SB_FAB_DEVICE_SIRIUS:
	    return SB_FAB_TARGET_DEVICE_LINK_IMPLEMENTATION_TYPE_HYPERCORE;
	default:
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("Device type error, not an SBX device\n")));
            return -1;
    }
}

sbFabStatus_t
GetLinkSpecificConfig(sbFabUserDeviceHandle_t devHandle, int32 nLink,
                      sbLinkDriverConfig_t *pLinkDriverConfig,
                      sbLinkSpecificDriverConfig_t *pSpecificConfig)
{
    int32 nLinkType;

    SB_FAB_VERIFY(pLinkDriverConfig);
    SB_FAB_VERIFY(pSpecificConfig);

    sal_memset(pSpecificConfig, 0x0, sizeof(sbLinkSpecificDriverConfig_t));
    pSpecificConfig->nLinkType = SB_FAB_TARGET_DEVICE_LINK_IMPLEMENTATION_TYPE_UNKNOWN;

    if ( pLinkDriverConfig->uDriverStrength > LINK_SPECIFIC_CONFIG_DRIVE_STRENGTH_MAX ) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META("Found requested driver strength(%d) out of"
                  " range, setting to maximum value of %d.\n"),
                  pLinkDriverConfig->uDriverStrength,
                  LINK_SPECIFIC_CONFIG_DRIVE_STRENGTH_MAX));
        pLinkDriverConfig->uDriverStrength = LINK_SPECIFIC_CONFIG_DRIVE_STRENGTH_MAX;
    }

    if ( pLinkDriverConfig->uDriverEqualization > LINK_SPECIFIC_CONFIG_EQUALIZATION_MAX ) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META("Found requested driver equalization(%d)"
                  " out of range, setting to maximum=%d.\n"),
                  pLinkDriverConfig->uDriverEqualization,
                  LINK_SPECIFIC_CONFIG_EQUALIZATION_MAX));
        pLinkDriverConfig->uDriverEqualization = LINK_SPECIFIC_CONFIG_EQUALIZATION_MAX;
    }

    nLinkType = GetLinkImplementationType(devHandle, nLink);
    switch (nLinkType) {
        case SB_FAB_TARGET_DEVICE_LINK_IMPLEMENTATION_TYPE_RAMBUS_RS2314:
        {
            pSpecificConfig->nLinkType = SB_FAB_TARGET_DEVICE_LINK_IMPLEMENTATION_TYPE_RAMBUS_RS2314;
            GetLinkSpecificConfigForRs2314(pLinkDriverConfig, pSpecificConfig);
            break;
        }
        case SB_FAB_TARGET_DEVICE_LINK_IMPLEMENTATION_TYPE_HYPERCORE:
	{
            pSpecificConfig->nLinkType = SB_FAB_TARGET_DEVICE_LINK_IMPLEMENTATION_TYPE_HYPERCORE;
            GetLinkSpecificConfigForHypercore(pLinkDriverConfig, pSpecificConfig);
            break;
	}

        default:
        {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("Unknown link type(0x%08x) found for"
                       " TargetDevice(0x%08x) nLink(%d)\n"),
                       nLinkType, (uint32)devHandle, nLink));
            return SB_FAB_STATUS_UNKNOWN_LINK_TYPE;
        }
    }

    return SB_FAB_STATUS_OK;
}

void
GetDefaultLinkDriverConfig(sbFabUserDeviceHandle_t sbHandle, int32 nLink,
                           sbLinkDriverConfig_t *pLinkDriverConfig)
{
    SB_FAB_ENTER();
    SB_FAB_VERIFY(pLinkDriverConfig);

    /* by default, lowest strength with maximum equalization */
    switch (SOC_SBX_CONTROL((int)sbHandle)->fabtype) {
        case SB_FAB_DEVICE_TME2000:
        case SB_FAB_DEVICE_QE2000:
            pLinkDriverConfig->uDriverStrength     = 53;
            pLinkDriverConfig->uDriverEqualization = 66;
            break;
        case SB_FAB_DEVICE_BM3200:
        case SB_FAB_DEVICE_SE4000:
        case SB_FAB_DEVICE_LCM4000:
            pLinkDriverConfig->uDriverStrength     = 53;
            pLinkDriverConfig->uDriverEqualization = 66;
             break;
        case SB_FAB_DEVICE_BM9600:
        case SB_FAB_DEVICE_SIRIUS:
            pLinkDriverConfig->uDriverStrength = 9; /* for iDriver=9 iPreDriver=9 - the default */
	    pLinkDriverConfig->uDriverEqualization = 6; /* for preemphasis_pre of 6 */
	    break;
        default:
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META("Unknown device type(0x%08x) for"
                      " sbFabDevice_GetDefaultLinkDriverConfig."
                      " Using QE2000 defaults.\n"),
                      (SOC_SBX_CONTROL(sbHandle)->fabtype)));
            pLinkDriverConfig->uDriverStrength     = 10;
            pLinkDriverConfig->uDriverEqualization = 100;
            break;
    }
}

/* Timeslot sizes' tables in nano-seconds as a factor of number of links */
/* QE2000--BM3200 system*/
static int32 TS_QE2000_BM3200[SB_FAB_DEVICE_QE2000_SFI_LINKS + 1] =
{  760,  8192, 5684, 3836, 2856, 2312, 1936, 1660, 1444, 1332,
   1172, 1064, 1008,  900,  848,  788,  760,  760,  760};
/* QE2000--BM3200 system with half-bus */
static int32 TS_QE2000_BM3200_HB[SB_FAB_DEVICE_QE2000_SFI_LINKS + 1] =
{  760,  5684, 2856, 1936, 1444, 1172, 1008, 848,  760,  760,
   760,  760,  760,  760,  760,  760,  760,  760,  760};

/* QE2000--BM9600 system */
static int32 TS_QE2000_BM9600[SB_FAB_DEVICE_QE2000_SFI_LINKS + 1] = 
{760,  11365,5705, 3855, 2875, 2330, 1950, 1680, 1460, 1355,
 1190, 1080, 1025, 920,  865,  810,  760,  760,  760}; 

/* QE2000--SIRIUS--CUSTOM1--BM9600 system */
static int32 SBX_TS_QE2000_SIRIUS_CUSTOM1_BM9600[BM9600_NUM_TIMESLOTSIZE + 1] = 
{
  1070,
  1070, 1070, 1070, 1070, 1070, 1070, 1070, 1070, 1070,  /* links: 01-09 */
  1070, 1070, 1070, 1070, 1070, 1070, 1070, 1070, 1070,  /* links: 10-18 */
  1070, 1070, 1070, 1070, 1070, 1070, 1070, 1070, 1070,  /* unused default */
  1070, 1070, 1070, 1070, 1070                           /* unused default */
}; 


int
soc_sbx_fabric_get_timeslot_optimized(int unit, uint32 *ts_opt, 
                                      uint32 *opt_packlets)
{
    int32   ts_min;
    uint32  link_speed, numerator, denominator;

    if ((ts_opt == NULL) ||(opt_packlets == NULL)) {
        return SOC_E_PARAM;
    }
    switch (SOC_SBX_CFG(unit)->uFabricConfig) {
    case SOC_SBX_SYSTEM_CFG_VPORT:
        ts_min = SB_FAB_DEVICE_BM9600_MIN_TIMESLOT_IN_NS_VPORT;
        break;
    case SOC_SBX_SYSTEM_CFG_VPORT_MIX:
        ts_min = SB_FAB_DEVICE_BM9600_MIN_TIMESLOT_IN_NS_VPORT_MIX;
        break;
    default:
        /* this function should be called only in VPORT & VPORT_MIX modes */
        return SOC_E_INTERNAL;
    }

    link_speed = SOC_SBX_CFG(unit)->uSerdesSpeed;
    if (link_speed == 6500) {
        /* 6500 is really 6.56Gbps */
        link_speed = 6560;
    }
    if (link_speed > 3125) {
        /* if > 3125, there are two channels on each link */
        link_speed /= 2;
    }

    /* first calculate optimized packlets per channel
     * Roundup (linkrate * encoding * base_TS - TS-HDR * 8 - 16 * 2)/17 * 8
     * 1000 --> is because speed is in mbps
     */
    numerator = link_speed * ts_min;
    numerator *= (SOC_SBX_CFG(unit)->bSerdesEncoding? 8 : 64);
    numerator /= (SOC_SBX_CFG(unit)->bSerdesEncoding? 10 : 66);
    numerator -= ((SB_FAB_DEVICE_BM9600_FABRIC_HEADER_LENGTH * 8 + 
                   16 * 2) * 1000);
    denominator = (17 * 8 * 1000);
    *opt_packlets = ((numerator/denominator) + 
                    ((numerator % denominator) ? 1 : 0)); /* roundup */

    /* now calculated optimized TS...roundup to multiple of 5 */
    numerator = ((*opt_packlets) * 17 * 8) + 
                (SB_FAB_DEVICE_BM9600_FABRIC_HEADER_LENGTH * 8 + 16 * 2);
    denominator = link_speed * 5;
    denominator *= (SOC_SBX_CFG(unit)->bSerdesEncoding? 8 : 64);
    numerator *= (SOC_SBX_CFG(unit)->bSerdesEncoding? 10 : 66);
    *ts_opt = (((numerator * 1000)/denominator + 
               (((numerator * 1000) % denominator) ? 1 : 0)) * 5);

    return SOC_E_NONE;
}

#ifdef BCM_SIRIUS_SUPPORT
static int  ts_override[31] = {
    0x0,   /* 0 xbar */
    0x0,   /* 1 xbar */
    0x0,   /* 2 xbar */
    0x0,   /* 3 xbar */
    0x0,   /* 4 xbar */
    0x1CE, /* 5 xbar */
    0x183, /* 6 xbar */
    0x14C, /* 7 xbar */
    0x120, /* 8 xbar */
    0x10A, /* 9 xbar */
    0xEA,  /* 10 xbar */
    0xD4,  /* 11 xbar */
    0xD1,  /* 12 xbar */
    0xD1,  /* 13 xbar */
    0xD1,  /* 14 xbar */
    0xD1,  /* 15 xbar */
    0xD1,  /* 16 xbar */
    0xD1,  /* 17 xbar */
    0xD1,  /* 18 xbar */
    0xD1,  /* 19 xbar */
    0xD1,  /* 20 xbar */
    0xD1,  /* 21 xbar */
    0xD1,  /* 22 xbar */
    0xD1,  /* 23 xbar */
    0xD1,  /* 24 xbar */
    0xD1,  /* 25 xbar */
    0xD1,  /* 26 xbar */
    0xD1,  /* 27 xbar */
    0xD1,  /* 28 xbar */
    0xD1,  /* 29 xbar */
    0xD1,  /* 30 xbar */
};

#endif

int
soc_sbx_fabric_get_timeslot_size_sirius(int unit, int32 num_logical_xbars, 
                                        uint32 *ts_rv)
{
#ifdef BCM_SIRIUS_SUPPORT
    uint32  fabric_config = SOC_SBX_CFG(unit)->uFabricConfig;
    uint32  ss_ss_bs, ss_qe_bs, ss_qe_bs_ddr_limited; /* burst sizes */
    uint32  fab_bw, ddr_bw, ddr_cong_bw; /* fabric serdes & memory bandwidths*/
    uint32  link_speed, numerator, denominator;
    uint32  ts_ss_ss, ts_ss_qe, ts_opt, opt_packlets;
    int     ss_links, qe2k_links, used_xbars;
    int     real_packlets_ss_ss, real_packlets_ss_qe;
    int     num_plane;

    if (ts_rv == NULL) {
        return SOC_E_PARAM;
    }

    if (num_logical_xbars > SB_FAB_DEVICE_SIRIUS_SFI_LINKS){
        num_logical_xbars = SB_FAB_DEVICE_SIRIUS_SFI_LINKS;
    }
    /* This is the maximum number of links for any QE2K type node */
    qe2k_links = soc_property_get(unit, spn_QE2K_LINKS, 
                                  SB_FAB_DEVICE_QE2000_SFI_LINKS);
    /* This is the maximum number of links for any Sirius type node */
    ss_links = soc_property_get(unit, spn_SIRIUS_LINKS, 
                                SB_FAB_DEVICE_SIRIUS_SFI_LINKS);

    link_speed = SOC_SBX_CFG(unit)->uSerdesSpeed;
    if (link_speed == 6500) {
        /* 6500 is really 6.56Gbps */
        link_speed = 6560;
    }
    if (link_speed > 3125) {
        /* if > 3125, there are two channels on each link */
        link_speed /= 2;
	num_plane = 2;
    } else {
        num_plane = 1;
    }

    ts_opt = 0;
    opt_packlets = 0;
    SOC_IF_ERROR_RETURN(soc_sbx_fabric_get_timeslot_optimized(unit, &ts_opt, 
                                                             &opt_packlets));
    if ((fabric_config == SOC_SBX_SYSTEM_CFG_VPORT) && 
        ((SOC_SBX_CFG(unit)->uRedMode == bcmFabricRed1Plus1ELS) ||
         (SOC_SBX_CFG(unit)->uRedMode == bcmFabricRedELS))) {
        /* In ELS mode, timeslot is fixed and burst size varies with
           number of links. Use optimized value for TS */
        *ts_rv = ts_opt;
        return SOC_E_NONE;
    }

    /* following calculations are for both - VPORT (but !ELS) and VPORT_MIX */

    /* First calculate fabric bandwidth limited by the serdes connectivity
     * 1000 --> convert from gbps to mbps 
     */
    fab_bw = ((opt_packlets * 16 * 8) * (ss_links * num_plane) * 1000)/ts_opt;

    /* pick minimum of DDR BW and fabric connectivity BW...in mbps */
    SOC_IF_ERROR_RETURN(soc_sirius_ddr_bandwidth_get(unit, &ddr_bw, 
                                                     &ddr_cong_bw));
    if (fab_bw > (ddr_bw * 1000)) {
        fab_bw = ddr_bw * 1000;
    }

    /* now calculate the ss-to-ss burst size in Bytes */
    ss_ss_bs = (fab_bw * ts_opt)/(16 * 8 * num_plane * 1000);

    /* Now calculate the ss-to-qe burst size in bytes.
     * Assume 3.125G and 8B10B encoding
     */
    numerator = ((3125 * 8 * ts_opt)
                 - ((SB_FAB_DEVICE_BM9600_FABRIC_HEADER_LENGTH * 8 + 16*2)
                    * 10000));
    denominator = (17 * 8 * 10000);
    ss_qe_bs = (numerator/denominator) * qe2k_links; /* rounddown */
    /* check to see if ss_qe_bs is limited by BW on Sirius */
    ss_qe_bs_ddr_limited = (ddr_bw * ts_opt)/(num_plane * 16 * 8); /* rounddown */
    /* Pick smaller of the two. If ss_qe_bs_ddr_limited is smaller
     * it means, burst size from ss to qe is limited by Sirius DDR BW
     */
    if (ss_qe_bs > ss_qe_bs_ddr_limited) {
        ss_qe_bs = ss_qe_bs_ddr_limited;
    }

    if (num_logical_xbars == 0) {
        real_packlets_ss_ss = real_packlets_ss_qe = 0;
    } else {
        used_xbars = (ss_links < num_logical_xbars) ? ss_links :
                      num_logical_xbars;
        real_packlets_ss_ss = (ss_ss_bs/used_xbars) + /* roundup */
                              ((ss_ss_bs % used_xbars) ? 1 : 0);

        used_xbars = qe2k_links - ((ss_links < num_logical_xbars)? 0 : 
                                 (ss_links - num_logical_xbars));
        real_packlets_ss_qe = (ss_qe_bs/used_xbars) + /* roundup */
                              ((ss_qe_bs % used_xbars) ? 1 : 0);
    }

    /* now calculate the TS values for both ss-to-ss and ss-to-qe
     * TS = (pklets *17*8 + TS hdr*8 + 2*16)/link_speed * encoding 
     * TS is rounded up to multiple of 5
     */
    numerator = (real_packlets_ss_ss * 17 * 8) + 
                (SB_FAB_DEVICE_BM9600_FABRIC_HEADER_LENGTH * 8 + 16 * 2);
    denominator = link_speed * 5;
    denominator *= (SOC_SBX_CFG(unit)->bSerdesEncoding? 8 : 64);
    numerator *= (SOC_SBX_CFG(unit)->bSerdesEncoding? 10 : 66);
    ts_ss_ss = ((numerator * 1000)/denominator + 
                (((numerator * 1000) % denominator) ? 1 : 0)) * 5;

    /* assume 3.125G and 8B10B encoding */
    numerator = (real_packlets_ss_qe * 17 * 8) + 
                (SB_FAB_DEVICE_BM9600_FABRIC_HEADER_LENGTH * 8 + 16 * 2);
    denominator = 3125 * 8 * 5;
    ts_ss_qe = ((numerator * 10000)/denominator + 
                (((numerator * 10000) % denominator) ? 1 : 0)) * 5;

    if (fabric_config == SOC_SBX_SYSTEM_CFG_VPORT) {
        *ts_rv = ts_ss_ss;

	/* override the timeslot size for certain number of crossbars
	 * based on regression result.
	 */
	if ((num_logical_xbars >= 5) &&
	    (num_logical_xbars <= 30) &&
	    (num_plane == 2)) {
	    *ts_rv = ts_override[num_logical_xbars] * 5; /* 5us/cycle */
	}

    } else {
        /* for VPORT_MIX, pick the larger of the two */
        *ts_rv = (ts_ss_ss > ts_ss_qe) ? ts_ss_ss : ts_ss_qe;
    }
    if (*ts_rv < ts_opt) {
        *ts_rv = ts_opt;
    }

    return SOC_E_NONE;
#else 
    return SOC_E_UNAVAIL;
#endif
}

int
soc_sbx_fabric_get_timeslot_size_qe2k(int unit, int32 num_logical_xbars, 
                                      uint32 *ts_rv)
{
    uint32  fabric_config = SOC_SBX_CFG(unit)->uFabricConfig;
    uint32  qe2k_fab_bs; /* qe2k to fabric burst size */
    uint32  fab_bw; /* bandwidth of fabric serdes */
    uint32  numerator, ts_opt, opt_packlets;
    int     ss_links, qe2k_links, real_packlets;

    if (ts_rv == NULL) {
        return SOC_E_PARAM;
    }
    if (fabric_config == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) {
        if (num_logical_xbars > SB_FAB_DEVICE_QE2000_SFI_LINKS){
            num_logical_xbars = SB_FAB_DEVICE_QE2000_SFI_LINKS;
        }
        *ts_rv = TS_QE2000_BM9600[num_logical_xbars];         
        if (*ts_rv < SB_FAB_DEVICE_BM9600_MIN_TIMESLOT_IN_NS_VPORT_LEGACY) {
            *ts_rv = SB_FAB_DEVICE_BM9600_MIN_TIMESLOT_IN_NS_VPORT_LEGACY;
        }
    } else if (fabric_config == SOC_SBX_SYSTEM_CFG_VPORT_MIX) {
        /* This is the maximum number of links for any QE2K type node */
        qe2k_links = soc_property_get(unit, spn_QE2K_LINKS, 
                                      SB_FAB_DEVICE_QE2000_SFI_LINKS);
        /* This is the maximum number of links for any Sirius type node */
        ss_links = soc_property_get(unit, spn_SIRIUS_LINKS, 
                                      SB_FAB_DEVICE_SIRIUS_SFI_LINKS);
        ts_opt = 0;
        opt_packlets = 0;
        SOC_IF_ERROR_RETURN(soc_sbx_fabric_get_timeslot_optimized(unit, 
                                                      &ts_opt, &opt_packlets));

        /* If Sirius is operating with more links, remapping of sfi links
         * should be in place.
         */
        num_logical_xbars -= ((ss_links > qe2k_links) ? 
                              (ss_links - qe2k_links): 0);
        if (num_logical_xbars <= 0) {
            /* Too many link failures. default to min TS value */
            *ts_rv = ts_opt;
            return SOC_E_NONE;
        }

        /* now calculate fabric bandwidth limited by the serdes connectivity
         * 1000 --> convert from gbps to mbps
         */
        fab_bw =  ((opt_packlets * 16 * 8) * qe2k_links * 1000)/ts_opt;

        /* pick the minimum of DDR BW of fabric connectivity BW */
        fab_bw = (fab_bw < (SB_FAB_DEVICE_QE2000_MAX_DDR_THROUGHPUT * 1000)) ?
                  fab_bw : (SB_FAB_DEVICE_QE2000_MAX_DDR_THROUGHPUT * 1000);

        /* now calculate the qe2k to fabric burst size in Bytes */
        qe2k_fab_bs = (fab_bw * ts_opt)/(16 * 8 * 1000);
        /* 255 is max burst size */
        if (qe2k_fab_bs > 255) {
            qe2k_fab_bs = 255;
        }

        if (num_logical_xbars > SB_FAB_DEVICE_QE2000_SFI_LINKS){
            num_logical_xbars = SB_FAB_DEVICE_QE2000_SFI_LINKS;
        }
        if (num_logical_xbars > qe2k_links) {
            num_logical_xbars = qe2k_links;
        }
        if (num_logical_xbars == 0) {
            real_packlets = 0;
        } else { 
            /* roundup */
            real_packlets = (qe2k_fab_bs/num_logical_xbars) + 
                            ((qe2k_fab_bs % num_logical_xbars) ? 1 : 0);
        }

        /* now calculate the TS value
         * TS = (pklets * 17 * 8 + TS hdr * 8 + 2 * 16)/link_speed * encoding 
         * Roundup to multiple of 5 
         */
        numerator = (real_packlets * 17 * 8) + 
                    (SB_FAB_DEVICE_BM9600_FABRIC_HEADER_LENGTH * 8 + 16 * 2);
        *ts_rv = ((numerator * 10000)/(3125 * 8 * 5)) * 5;;
        if (*ts_rv < ts_opt) {
            *ts_rv = ts_opt;
        }
    } else {
        return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

int
soc_sbx_fabric_custom1_get_timeslot_size(int      unit,
                                         uint64   xbars,
                                         int32  num_logical_xbars,
                                         uint32  *ts_size)
{
    int     rv = SOC_E_NONE;
    uint32  fabric_config = SOC_SBX_CFG(unit)->uFabricConfig;

    if (num_logical_xbars > BM9600_NUM_TIMESLOTSIZE) {
        return SOC_E_PARAM;
    }

    /* set a default value */
    (*ts_size) = SBX_TS_QE2000_SIRIUS_CUSTOM1_BM9600[0];         

    switch (fabric_config) {
        case SOC_SBX_SYSTEM_CFG_DMODE:
            rv = SOC_E_CONFIG;
            break;

        case SOC_SBX_SYSTEM_CFG_VPORT_LEGACY:
        case SOC_SBX_SYSTEM_CFG_VPORT_MIX:
            (*ts_size) = SBX_TS_QE2000_SIRIUS_CUSTOM1_BM9600[0];         
            break;

        case SOC_SBX_SYSTEM_CFG_VPORT:
            rv = SOC_E_CONFIG;
            break;

        default:
            break;
    }

    if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Retreiving Timeslot, mode(%d) nbr_xbars(%d)\n"),
                   fabric_config, num_logical_xbars));
    }

    return(rv);
}

int32
soc_sbx_fabric_get_timeslot_size(int unit, int32 num_logical_xbars, 
                                 int32 b_half_bus, int32 b_hybrid)
{
    int     rv = SOC_E_NONE;
    uint32  fabric_config = SOC_SBX_CFG(unit)->uFabricConfig;
    uint32  ts_rv, ts_qe2k, ts_ss;
    uint64  xbars = COMPILER_64_INIT(0,0);
              /* For flexibility this needs to be passed as a parameter to this function  */
              /* It will then be a central place to base decisions on logical xbars       */
              /* belonging to different node's/device's. Currently it is just declared    */
              /* to document change when if flexibility is required                       */

    if (SOC_SBX_CFG(unit)->module_custom1_in_system == TRUE) {
        rv = soc_sbx_fabric_custom1_get_timeslot_size(unit, xbars, num_logical_xbars, &ts_rv);
        return(ts_rv);
    }

    switch (fabric_config) {
    case SOC_SBX_SYSTEM_CFG_DMODE:
        if (num_logical_xbars > SB_FAB_DEVICE_QE2000_SFI_LINKS){
            num_logical_xbars = SB_FAB_DEVICE_QE2000_SFI_LINKS;
        }
        if (b_half_bus) {
            ts_rv = TS_QE2000_BM3200_HB[num_logical_xbars];
        } else { 
            ts_rv = TS_QE2000_BM3200[num_logical_xbars];
        }
        break;

    case SOC_SBX_SYSTEM_CFG_VPORT_LEGACY:
        rv = soc_sbx_fabric_get_timeslot_size_qe2k(unit, num_logical_xbars, 
                                                   &ts_rv);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Internal error calculating Timeslot size "
                                   "for VPORT_LEGACY mode \n")));
            /* set to min TS value in VPORT legacy */
            ts_rv = SB_FAB_DEVICE_BM9600_MIN_TIMESLOT_IN_NS_VPORT_LEGACY;
        }
        if (ts_rv > SB_FAB_DEVICE_BM9600_MAX_TIMESLOT_IN_NS) {
            ts_rv = SB_FAB_DEVICE_BM9600_MAX_TIMESLOT_IN_NS;
        }
        break;

    case SOC_SBX_SYSTEM_CFG_VPORT_MIX:
        rv = soc_sbx_fabric_get_timeslot_size_qe2k(unit, num_logical_xbars, 
                                                   &ts_qe2k);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Internal error calculating Timeslot size "
                                   "for QE2K nodes in VPORT_MIX mode \n")));
            /* set to optimized TS value in VPORT MIX */
            ts_qe2k = SB_FAB_DEVICE_BM9600_MIN_TIMESLOT_IN_NS_VPORT_MIX;
        }
        rv = soc_sbx_fabric_get_timeslot_size_sirius(unit, 
                                                    num_logical_xbars, &ts_ss);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Internal error calculating Timeslot size "
                                   "for Sirius nodes in VPORT_MIX mode \n")));
            /* set to optimized TS value in VPORT MIX */
            ts_ss = SB_FAB_DEVICE_BM9600_MIN_TIMESLOT_IN_NS_VPORT_MIX;
        }
        ts_rv = (ts_qe2k > ts_ss) ? ts_qe2k : ts_ss; /* pick the larger one */
        if (ts_rv > SB_FAB_DEVICE_BM9600_MAX_TIMESLOT_IN_NS) {
            ts_rv = SB_FAB_DEVICE_BM9600_MAX_TIMESLOT_IN_NS;
        }
        break;

    case SOC_SBX_SYSTEM_CFG_VPORT:
        rv = soc_sbx_fabric_get_timeslot_size_sirius(unit, num_logical_xbars, 
                                                     &ts_rv);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Internal error calculating Timeslot size "
                                   "for VPORT mode \n")));
            /* set to min TS value in VPORT */
            ts_rv = SB_FAB_DEVICE_BM9600_MIN_TIMESLOT_IN_NS_VPORT;
        }
        if (ts_rv > SB_FAB_DEVICE_BM9600_MAX_TIMESLOT_IN_NS) {
            ts_rv = SB_FAB_DEVICE_BM9600_MAX_TIMESLOT_IN_NS;
        }
        break;

    default:
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Invalid fabric mode (%d) configured\n"), 
                   fabric_config));
        ts_rv = SB_FAB_DEVICE_BM9600_MIN_TIMESLOT_IN_NS_VPORT;
        break;
    }

    return ts_rv;
}
void 
get_ts_exp_mant(uint32 kbits, uint32 *exp, uint32 *mant)
{
    int32 num;
    int flag;

    uint32 tmp;
    if (kbits < 101250) {
        num = kbits * 1000;
	flag=0;
	if (num <  6328125){
	    if        (num<1582031) {
	        *exp=1;
                if (num>791015)
                    tmp = 791015;
                else
                    tmp = 787949; 
	    } else if (num<3164062) {
	        *exp=2;
	        tmp = 1582031;
	    } else {
	        *exp=3;
	        tmp = 3164062;
	    }
	} else {
	    if (num < 12656250 ) {
	      *exp = 4;
	      tmp = 6328125;
	    } else if (num < 25312500) {
	        *exp = 5;
		tmp = 12656250;
	    } else if (num < 50625000) {
	        *exp = 6;
                flag = 4;
		tmp = 253125;
	    } else {
	        *exp = 7;
                flag = 1;
		tmp = 50625;
	    }
	}
    } else {
        num = kbits;
	flag=1;
	if (num < 1620000){
	    if (num < 202500) {
	        *exp=8; 
		tmp = 101250;
	    } else if (num < 405000){
	        *exp=9;
		tmp = 202500;
	    } else if (num < 810000){
	        *exp = 10;
		tmp = 405000;
	    } else {
	        *exp =11;
		tmp = 810000;
	    }
	} else {
	    if (num < 3240000) {
	        *exp = 12;
		tmp = 1620000;
	    } else if (num < 6480000) {
	        *exp = 13;
		tmp = 3240000;
	    } else if( num < 12960000) {
	        *exp=14;
		flag = 2;
		tmp = 6480000/4;
	    } else {
	        flag = 3;
	        *exp=15;
		tmp = 12960000/8;
	    }
	}
    }
    if (flag == 0) {
        *mant = (kbits*1000-tmp)*256/tmp;
    } else if (flag == 1){
        *mant = (kbits-tmp)*256/tmp;
    } else if (flag == 2) {
      *mant = (kbits - tmp*4)*64/tmp;
    } else if (flag == 3) {
      *mant = (kbits - tmp*8)*32/tmp;
    } else {
      *mant = (kbits*10 - tmp)*256/tmp;
    }
}

int32
soc_sbx_fabric_util_num_to_mant_exp(int unit, soc_mem_t mem, soc_field_t field, uint32 num, uint32 *mant, uint32 *exp)
{
    uint64 uuTmp;
    uint32 uClock;

    *mant = 0;
    *exp = 0;
    switch (mem) {
	case L1_BPm:
	case L2_BPm:
	case L3_BPm:
	case L4_BPm:
	case L5_BPm:
	case L6_BPm:
	case L7_BPm:
	    switch (field) {
		case SHAPER_RATE_MANTf:
		case SHAPER_RATE_EXPf:
		case CREDITOR_RATE_MANTf:
		case CREDITOR_RATE_EXPf:
		    /* based on following table from SPEC
		     * r_e r_m=0            r_m=0xFF            step size
		     *  0  0                778,198             3,052
		     *  1  781,250          1,559,448           3,052
		     *  2  1,562,500        3,118,896           6,104
		     *  3  3,125,000        6,237,793           12,207
		     *  4  6,250,000        12,475,586          24,414
		     *  5  12,500,000       24,951,172          48,828
		     *  6  25,000,000       49,902,344          97,656
		     *  7  50,000,000       99,804,688          195,313
		     *  8  100,000,000      199,609,375         390,625
		     *  9  200,000,000      399,218,750         781,250
		     *  10 400,000,000      798,437,500         1,562,500
		     *  11 800,000,000      1,596,875,000       3,125,000
		     *  12 1,600,000,000    3,193,750,000       6,250,000
		     *  13 3,200,000,000    6,387,500,000       12,500,000
		     *  14 6,400,000,000    12,775,000,000      25,000,000
		     *  15 12,800,000,000   25,550,000,000      50,000,000
		     *
		     * Table assumes 400M clock, adjust based on real TS clock
		     */		    

		    if (num < ((3090 * 0xFF)/1000)) {
			*exp = 0;
			*mant = (num * 1000) / 3090;
		    } else if (num > 25550000) {
			/* overflowed */
			*exp = 15;
			*mant = 0xFF;
			LOG_ERROR(BSL_LS_SOC_COMMON,
			          (BSL_META_U(unit,
			                      "Rate out of valid range\n")));
			return SOC_E_PARAM;
		    } else {
		        get_ts_exp_mant(num, exp, mant);
			if ((*exp > 15) || (*mant > 0xFF)) {
			    LOG_ERROR(BSL_LS_SOC_COMMON,
			              (BSL_META_U(unit,
			                          "Rate coversion result in"
			                           " exp/mant out of range\n")));
			    return SOC_E_INTERNAL;
			}
		    }
		    break;
		case SHAPER_MAXBURST_MANTf:
		case SHAPER_MAXBURST_EXPf:
		    /* max size of bucket = mant << exp */
		    /* assuming num is bits, hardware uses 1/8 bit */
		    num <<= 3;
		    if (num > (196 << 15)) {
			/* max allowed is around 802Kbits, overflowed */
			*mant = 196;
			*exp = 15;
			LOG_ERROR(BSL_LS_SOC_COMMON,
			          (BSL_META_U(unit,
			                      "Number out of valid range\n")));
			return SOC_E_PARAM;
		    } else if (num == 0) {
			*mant = 0;
			*exp = 0;
			return SOC_E_NONE;
		    } else if (num < (195 << 12)) {
			/* min allowed is around 100Kbits, underflowed */
			*mant = 195;
			*exp = 12;
			LOG_ERROR(BSL_LS_SOC_COMMON,
			          (BSL_META_U(unit,
			                      "Number out of valid range\n")));
			return SOC_E_PARAM;
		    }
		    for (*exp = 0; num > 0xFF; num >>= 1, (*exp)++) {
		    }		    
		    *mant = num;
		    break;
		default:
		    return SOC_E_PARAM;
	    }
	    break;
	case INTERFACE_MAX_SHAPER_TABLEm:
	case CHANNEL_SHAPER_TABLEm:
	case SUBPORT_SHAPER_TABLEm:
	case FIFO_SHAPER_TABLE_0m:
	case FIFO_SHAPER_TABLE_1m:
	case FIFO_SHAPER_TABLE_2m:
	case FIFO_SHAPER_TABLE_3m:
	    switch (field) {
		case MIN_REF_RATEf:
		case MAX_REF_RATEf:
		    /* assuming num is kbits/second */
		    if (num == 0) {
			*mant = 0;
			*exp = 0;
			return SOC_E_NONE;
		    } else {
			/* convert rate from kbits/second to bits/refresh cycle 
			 * each refresh cycle is 1.95us based on 125Mhz frequency
			 * shaper rate is round down.
			 * The algorithm down is coded based on the table on sirius spec
			 * r_e | rate with r_m=0 (bits/s) | rate with r_m=1023 (bits/s) | Step Size (bits/s)
			 * 0    0                           8,184,000                     8,000
			 * 1    8,192,000                   16,376,000                    8,000
			 * 2    16,384,000                  32,752,000                    16,000
			 * 3    32,768,000                  65,504,000                    32,000
			 * 4    65,536,000                  131,008,000                   64,000
			 * 5    131,072,000                 262,016,000                   128,000
			 * 6    262,144,000                 524,032,000                   256,000
			 * 7    524,288,000                 1,048,064,000                 512,000
			 * 8    1,048,576,000               2,096,128,000                 1,024,000
			 * 9    2,097,152,000               4,192,256,000                 2,048,000
			 * 10   4,194,304,000               8,384,512,000                 4,096,000
			 * 11   8,388,608,000               16,769,024,000                8,192,000
			 * 12   16,777,216,000              33,538,048,000                16,384,000
			 * 13   reserved                    reserved                      reserved
			 * 14   reserved                    reserved                      reserved
			 * 15   reserved                    reserved                      reserved
			 */
			if (num < 8 * 1024) {
			    /* when rate is lower than 8Mbps, exp=0, bits/refresh cycle = 
			     * (mant/1024) * 2^(exp+4)
			     */
			    *exp = 0;
			    *mant = num / 8;
			} else if (num > 32 * 1024 * 1024) {
			    /* overflowed */
			    *exp = 12;
			    *mant = 0x3FF;
			} else {
			    /* when rate is higher than 8Mbps, bits/refresh cycle 
			     * = (1+mant/1024) * 2^(exp+3)
			     */
			    *mant = num;
			    for (*exp = 0; num > 0x1FFF; num >>=1, (*exp)++) {
			    }
			    *mant = (*mant - (8192 << (*exp - 1))) / (8 << (*exp -1));
			}

			if ( (*exp > 12) || (*mant > 1023) ) {
			    /* exp valid range 0-12 */
			    /* mant valid range 0-1023 */
			    return SOC_E_INTERNAL;
			} else {
			    return SOC_E_NONE;
			}
		    }
		    break;
		case MIN_THLDf:
		case MAX_THLDf:
		    /* max size of bucket = (1 + mant/128) * 2(exp+9) */
		    /* assuming num is bits */
		    if (num == 0) {
			*mant = 0;
			*exp = 0;
			return SOC_E_NONE;
		    } else {
			/* 
			 * The algorithm down is coded based on the table on sirius spec
			 * t_e | thresh with t_m=0 (bits) | thresh with t_m=127 (bits) | Step Size (bits)
			 * 0     512                          1,020                         4	   
			 * 1     1,024                        2,040                         8	   
			 * 2     2,048                        4,080                         16	   
			 * 3     4,096                        8,160                         32	   
			 * 4     8,192                        16,320                        64	   
			 * 5     16,384                       32,640                        128	   
			 * 6     32,768                       65,280                        256	   
			 * 7     65,536                       130,560                       512	   
			 * 8     131,072                      261,120                       1024	   
			 * 9     262,144                      522,240                       2048	   
			 * 10    524,288                      1,044,480                     4096	   
			 * 11    1,048,576                    2,088,960                     8192	   
			 * 12    2,097,152                    4,177,920                     16384   
			 * 13    4,194,304                    8,355,840                     32768   
			 * 14    8,388,608                    16,711,680                    65536   
			 * 15    reserved                     reserved                      reserved
			 */
			if (num < 512) {
			    /* underflowed, set to 512 bits */
			    *exp = 0;
			    *mant = 0;
			} else if (num > 16 * 1024 * 1024) {
			    /* overflowed */
			    *exp = 14;
			    *mant = 0x7F;
			} else {
			    *mant = num;
			    for (*exp = 0; num > 0x1FF; num >>=1, (*exp)++) {
			    }
			    (*exp)--;
			    *mant = (*mant - (512 << (*exp))) / (4 << (*exp));
			}

			if ( (*exp > 14) || (*mant > 127) ) {
			    /* exp valid range 0-14 */
			    /* mant valid range 0-127 */
			    return SOC_E_INTERNAL;
			} else {
			    return SOC_E_NONE;
			}
		    }
		    break;
		default:
		    return SOC_E_PARAM;
	    }
	    break;
	case SHAPER_LEAK_0m:
	case SHAPER_LEAK_1m:
	case SHAPER_LEAK_2m:
	case SHAPER_LEAK_3m:
	case BAA_LEAK_A0m:
	case BAA_LEAK_A1m:
	case BAA_LEAK_A2m:
	case BAA_LEAK_A3m:
	case BAA_LEAK_B0m:
	case BAA_LEAK_B1m:
	case BAA_LEAK_B2m:
	case BAA_LEAK_B3m:
	    switch (field) {
		case SHAPE_RATE_EXPf:
		case SHAPE_RATE_MANTf:
		case BAA_RATE_EXPf:
		case BAA_RATE_MANTf:
		    /* pass in kbits/second, convert to bits/(2^15 cycles) */
		    COMPILER_64_SET(uuTmp, 0, num);
		    COMPILER_64_SHL(uuTmp, 15);
		    uClock = SOC_SBX_CFG(unit)->uClockSpeedInMHz * 1000;
		    if (soc_sbx_div64(uuTmp, uClock, &num) != 0) {
			/* overflowed */
			*mant = 0xFF;
			*exp = 0;
			LOG_ERROR(BSL_LS_SOC_COMMON,
			          (BSL_META_U(unit,
			                      "Rate out of valid range\n")));
			return SOC_E_PARAM;
		    }

		    /* bits/cycle leak = mant * (2^-exp) */
		    if (num > (0xFF << 15)) {
			/* overflowed */
			*mant = 0xFF;
			*exp = 15;
			LOG_ERROR(BSL_LS_SOC_COMMON,
			          (BSL_META_U(unit,
			                      "Number out of valid range\n")));
			return SOC_E_PARAM;
		    } else if (num == 0) {
			*mant = 0;
			*exp = 0;
			return SOC_E_NONE;
		    }
		    for (*exp = 15; num > 0xFF; num >>= 1, (*exp)--) {
		    }
		    *mant = num;
		    break;
		case SHAPE_THRESH_EXPf:
		case SHAPE_THRESH_MANTf:
		    /* bits mant * (2^exp) */
		    if (num > (0xFF << 15)) {
			/* overflowed */
			*mant = 0xFF;
			*exp = 15;
			LOG_ERROR(BSL_LS_SOC_COMMON,
			          (BSL_META_U(unit,
			                      "Number out of valid range\n")));
			return SOC_E_PARAM;
		    } else if (num == 0) {
			*mant = 0;
			*exp = 0;
			return SOC_E_NONE;
		    }
		    for (*exp = 0; num > 0xFF; num >>= 1, (*exp)++) {
		    }
		    *mant = num;
		    break;
		default:
		    break;
	    }
	    break;
	default:
	    return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}
