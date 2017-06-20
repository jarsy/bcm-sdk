
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $Id: c3_8x10g_2xhg10_1xil100.h,v 1.1.2.5 Broadcom SDK $
 */
#ifdef BCM_CALADAN3_SUPPORT

sws_config_t c3_8x10g_4xhg10_1xil100_cfg = {


    /*
     * QM Config
     */
     {
        /* Buffer Thresholds */
        {
           15384, 11538, 7692, 11291, 3728, 1539, 3216, 2412, 
           1608, 10616, 322, 11861, 8896, 5931, 10616, 1187, 38
        },

        /* Queue Config */
        0, 
        0, 
        {
            { 0 }, /* 0 */
            { 0 }, /* 1 */
            { 0 }, /* 2 */   
            { 0 }, /* 3 */   
            { 0 }, /* 4 */
            { 0 }, /* 5 */
            { 0 }, /* 6 */
            { 0 }, /* 7 */
            { 0 }, /* 8 */
            { 0 }, /* 9 */
            { 0 }, /* 10 */
            { 0 }, /* 11 */
            { 0 }, /* 12 */
            { 0 }, /* 13 */
            { 0 }, /* 14 */
            { 0 }, /* 15 */
            { 0 }, /* 16 */
            { 0 }, /* 17 */
            { 0 }, /* 18 */
            { 0 }, /* 19 */
            { 0 }, /* 20 */
            { 0 }, /* 21 */
            { 0 }, /* 22 */
            { 0 }, /* 23 */
            { 0 }, /* 24 */
            { 0 }, /* 25 */
            { 0 }, /* 26 */
            { 0 }, /* 27 */
            { 0 }, /* 28 */
            { 0 }, /* 29 */
            { 0 }, /* 30 */
            { 0 }, /* 31 */
            { 0 }, /* 32 */
            { 0 }, /* 33 */
            { 0 }, /* 34 */
            { 0 }, /* 35 */
            { 0 }, /* 36 */
            { 0 }, /* 37 */
            { 0 }, /* 38 */
            { 0 }, /* 39 */
            { 0 }, /* 40 */
            { 0 }, /* 41 */
            { 0 }, /* 42 */
            { 0 }, /* 43 */
            { 0 }, /* 44 */
            { 0 }, /* 45 */
            { 0 }, /* 46 */
            { 0 }, /* 47 */
            { 0 }, /* 48 */
            { 0 }, /* 49 */
            { 0 }, /* 50 */
            { 0 }, /* 51 */
            { 0 }, /* 52 */
            { 0 }, /* 53 */
            { 0 }, /* 54 */
            { 0 }, /* 55 */
            { 0 }, /* 56 */
            { 0 }, /* 57 */
            { 0 }, /* 58 */
            { 0 }, /* 59 */
            { 0 }, /* 60 */
            { 0 }, /* 61 */
            { 0 }, /* 62 */
            { 0 }, /* 63 */
        },
        {
            { 0 }, /* 0 */
            { 0 }, /* 1 */
            { 0 }, /* 2 */
            { 0 }, /* 3 */
            { 0 }, /* 4 */
            { 0 }, /* 5 */
            { 0 }, /* 6 */
            { 0 }, /* 7 */
            { 0 }, /* 8 */
            { 0 }, /* 9 */
            { 0 }, /* 10 */
            { 0 }, /* 11 */
            { 0 }, /* 12 */
            { 0 }, /* 13 */
            { 0 }, /* 14 */
            { 0 }, /* 15 */
            { 0 }, /* 16 */
            { 0 }, /* 17 */
            { 0 }, /* 18 */
            { 0 }, /* 19 */
            { 0 }, /* 20 */
            { 0 }, /* 21 */
            { 0 }, /* 22 */
            { 0 }, /* 23 */
            { 0 }, /* 24 */
            { 0 }, /* 25 */
            { 0 }, /* 26 */
            { 0 }, /* 27 */
            { 0 }, /* 28 */
            { 0 }, /* 29 */
            { 0 }, /* 30 */
            { 0 }, /* 31 */
            { 0 }, /* 32 */
            { 0 }, /* 33 */
            { 0 }, /* 34 */
            { 0 }, /* 35 */
            { 0 }, /* 36 */
            { 0 }, /* 37 */
            { 0 }, /* 38 */
            { 0 }, /* 39 */
            { 0 }, /* 40 */
            { 0 }, /* 41 */
            { 0 }, /* 42 */
            { 0 }, /* 43 */
            { 0 }, /* 44 */
            { 0 }, /* 45 */
            { 0 }, /* 46 */
            { 0 }, /* 47 */
            { 0 }, /* 48 */
            { 0 }, /* 49 */
            { 0 }, /* 50 */
            { 0 }, /* 51 */
            { 0 }, /* 52 */
            { 0 }, /* 53 */
            { 0 }, /* 54 */
            { 0 }, /* 55 */
            { 0 }, /* 56 */
            { 0 }, /* 57 */
            { 0 }, /* 58 */
            { 0 }, /* 59 */
            { 0 }, /* 60 */
            { 0 }, /* 61 */
            { 0 }, /* 62 */
            { 0 }, /* 63 */
        }, 

        /* Cmic queues */
        { 0 },

        /* XL queues */
        { { 0 }, { 0 } },

        /* Ingress Redirection Queues */
        { {128, 96, 64, 0, 0, 64}, {128, 96, 64, 0, 0, 64} },

        /* Egress Redirection Queues */
        { {128, 96, 64, 0, 0, 64}, {128, 96, 64, 0, 0, 64} },

        /* Ingress Bubble Queues */
        {128, 96, 64, 0, 0, 64},

        /* Egress Bubble Queues */
        {128, 96, 64, 0, 0, 64},


    },

    /*
     * Line PT Config
     */
    {
        /* Fifo info */
        {
            0,
            {
                {0,0,0,0},    /* Port 0 */
                {1,0,0,0},    /* Port 1 */
                {2,0,0,0},    /* Port 2 */
                {3,0,0,0},    /* Port 3 */
                {4,0,0,0},    /* Port 4 */
                {5,0,0,0},    /* Port 5 */
                {6,0,0,0},    /* Port 6 */
                {7,0,0,0},    /* Port 7 */
                {8,0,0,0},    /* Port 8 */
                {9,0,0,0},    /* Port 9 */
                {10,0,0,0},    /* Port 10 */
                {11,0,0,0},    /* Port 11 */
                {12,0,0,0},    /* Port 12 */
                {13,0,0,0},    /* Port 13 */
                {14,0,0,0},    /* Port 14 */
                {15,0,0,0},    /* Port 15 */
                {16,0,0,0},    /* Port 16 */
                {17,0,0,0},    /* Port 17 */
                {18,0,0,0},    /* Port 18 */
                {19,0,0,0},    /* Port 19 */
                {20,0,0,0},    /* Port 20 */
                {21,0,0,0},    /* Port 21 */
                {22,0,0,0},    /* Port 22 */
                {23,0,0,0},    /* Port 23 */
                {24,0,0,0},    /* Port 24 */
                {25,0,0,0},    /* Port 25 */
                {26,0,0,0},    /* Port 26 */
                {27,0,0,0},    /* Port 27 */
                {28,0,0,0},    /* Port 28 */
                {29,0,0,0},    /* Port 29 */
                {30,0,0,0},    /* Port 30 */
                {31,0,0,0},    /* Port 31 */
                {32,0,0,0},    /* Port 32 */
                {33,0,0,0},    /* Port 33 */
                {34,0,0,0},    /* Port 34 */
                {35,0,0,0},    /* Port 35 */
                {36,0,0,0},    /* Port 36 */
                {37,0,0,0},    /* Port 37 */
                {38,0,0,0},    /* Port 38 */
                {39,0,0,0},    /* Port 39 */
                {40,0,0,0},    /* Port 40 */
                {41,0,0,0},    /* Port 41 */
                {42,0,0,0},    /* Port 42 */
                {43,0,0,0},    /* Port 43 */
                {44,0,0,0},    /* Port 44 */
                {45,0,0,0},    /* Port 45 */
                {46,0,0,0},    /* Port 46 */
                {47,0,0,0},    /* Port 47 */
                {48,0,0,0},    /* Port 48 */
                {49,0,0,0},    /* Port 49 */
                {50,0,0,0},    /* Port 50 */
            }
        },

        /* Client Cal */
        {
            78,
            {
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT4, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT4, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT5, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0,
              CLIENT0, CLIENT4, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT5 
            },
        },

        /* Port Cal */
        {
            99,
            {
              0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 48,
              0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 49,
              0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 50,
              0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0,
              1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1,
              2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2,
              3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3,
              4, 5, 6, 7, 8, 9, 10, 11
            },
        },
    },
    /*
     * Fabric PT Config
     */
    {
        /* Fifo info */
        {
             1, 
             {
                 { 0,0,76,20 },    /* Port 0*/
              }
        },

        /* Client Cal */
        {
            72,
            {
                 CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
                 CLIENT0, CLIENT0, CLIENT0, CLIENTX,
                 CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
                 CLIENT0, CLIENT0, CLIENT0, CLIENTX,
                 CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
                 CLIENT0, CLIENT0, CLIENT0, CLIENTX,
                 CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
                 CLIENT0, CLIENT0, CLIENT0, CLIENTX,
                 CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
                 CLIENT0, CLIENT0, CLIENT0, CLIENTX,
                 CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
                 CLIENT0, CLIENT0, CLIENT0, CLIENTX,
                 CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
                 CLIENT0, CLIENT0, CLIENT0, CLIENTX,
                 CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
                 CLIENT0, CLIENT0, CLIENT0, CLIENTX,
            },
        },

        /* Port Cal */
        {
            108,
            {
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
              0, 0
            },
        },
    },

    /*
     * Line PR config
     */
    {

        /* Rx buf config */
        {
            0,
            {
                {0, 0, 0} 
            },
        },

        /* IDP Thresholds */
        { 255, 192, 128, 0, 0 },
    },

    /*
     * Fabric PR config
     */
    {

        /* Rx buf config */
        {
            0,
            {
                {0, 0, 0}
            }
        },

        /* IDP Thresholds */
        { 255, 192, 128, 0, 196 },
    },

    /*
     * Interface config
     *   Specify Queue cfg and PT fifo at interface level
     *   All queues assigned to this port get the same info
     *   Queue level data overrides these parameters, so can be used for fine tuning at queue level
     */
    {{
        /*  CLPORT_1G    */
        { 
           {116, 87, 58, 42, 34, 34},      /* Line Queue cfg */
           {543, 408, 272, 155, 33, 33},  /* Fab Queue cfg */
           {0},  /* PT fifo   */
        },
        /*  CLPORT_10GE  */
        { 
           {310, 233, 155, 100, 68, 68},      /* Line Queue cfg */
           {744, 558, 372, 299, 109, 109},  /* Fab Queue cfg */
           {0},  /* PT fifo   */
        },
        /*  CLPORT_40GE  */
        { 
           {0},  /* Line Queue cfg */
           {0},  /* Fab Queue cfg */
           {0},  /* PT fifo   */
        },
        /*  CLPORT_100GE */
        { 
           {0},  /* Line Queue cfg */
           {0},  /* Fab Queue cfg */
           {0},  /* PT fifo   */
        },
        /*  CLPORT_HG10  */
        { 
           {108, 81, 54, 40, 19, 19},      /* Line Queue cfg */
           {466, 350, 233, 130, 21, 21},  /* Fab Queue cfg */
           {0},  /* PT fifo   */
        },
        /*  CLPORT_HG25  */
        { 
           {0},  /* Line Queue cfg */
           {0},  /* Fab Queue cfg */
           {0},  /* PT fifo   */
        },
        /*  CLPORT_HG42  */
        { 
           {0},  /* Line Queue cfg */
           {0},  /* Fab Queue cfg */
           {0},  /* PT fifo   */
        },
        /*  CLPORT_HG126 */
        { 
           {0},  /* Line Queue cfg */
           {0},  /* Fab Queue cfg */
           {0},  /* PT fifo   */
        },
        /*  XTPORT       */
        { 
           {0},  /* Line Queue cfg */
           {0},  /* Fab Queue cfg */
           {0},  /* PT fifo   */
        },
        /*  IL50w        */
        { 
           {0},  /* Line Queue cfg */
           {0},  /* Fab Queue cfg */
           {0},  /* PT fifo   */
        },
        /*  IL50n        */
        { 
           {0},  /* Line Queue cfg */
           {0},  /* Fab Queue cfg */
           {0},  /* PT fifo   */
        },
        /*  IL100        */
        { 
           {0},  /* Line Queue cfg */
           {0},  /* Fab Queue cfg */
           {0},  /* PT fifo   */
        },
        /*  CMIC         */
        { 
           {258, 194, 129, 120, 64, 64},  /* Line Queue cfg */
           {258, 194, 129, 120, 64, 64},  /* Fab Queue cfg */
           {0},  /* PT fifo   */
        },
        /* XLPORT        */
        { 
           {256, 192, 128, 120, 64, 64},  /* Line Queue cfg */
           {256, 192, 128, 120, 64, 64},  /* Fab Queue cfg */
           {0},  /* PT fifo   */
        },
    }}

};

#endif
