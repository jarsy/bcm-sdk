
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $Id: c3_4x10g+6xhg10_1xil100.h,v 1.1 Broadcom SDK $
 */
#ifdef BCM_CALADAN3_SUPPORT

sws_config_t c3_8x10g_4xhg10_cfg = {


    /*
     * QM Config
     */
     {
        /* Buffer Thresholds */
        {
           15384, 11538, 7692, 11226, 3776, 1539, 3548, 2661, 
           1774,
           0x3fff, /* This sets QM_FC_CONFIG2.FC_INGRESS_XOFF_THRESH. Proper value is highly
                      dependent on customer systems and microcode design. Set it to the maximum
                      as a place holder. A document guiding customers to obtain this value is available. */
           355, 11527, 8646, 5764, 10300, 1153, 38
        },

        /* Queue Config */
        52, 
        52, 
        {
            /* The 4th parameter is per-queue flow control threshold. Similarly to the global setting,
               it needs to be obtained on customer systems. 0x3fff is a placeholder. */
            {310, 233, 155, 0x3fff, 68, 68}, /* 0 */
            {310, 233, 155, 0x3fff, 68, 68}, /* 1 */
            {310, 233, 155, 0x3fff, 68, 68}, /* 2 */
            {310, 233, 155, 0x3fff, 68, 68}, /* 3 */
            {310, 233, 155, 0x3fff, 68, 68}, /* 4 */
            {310, 233, 155, 0x3fff, 68, 68}, /* 5 */
            {310, 233, 155, 0x3fff, 68, 68}, /* 6 */
            {310, 233, 155, 0x3fff, 68, 68}, /* 7 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 8 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 9 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 10 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 11 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 12 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 13 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 14 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 15 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 16 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 17 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 18 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 19 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 20 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 21 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 22 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 23 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 24 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 25 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 26 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 27 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 28 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 29 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 30 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 31 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 32 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 33 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 34 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 35 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 36 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 37 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 38 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 39 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 40 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 41 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 42 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 43 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 44 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 45 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 46 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 47 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 48 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 49 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 50 */
            {64, 48, 32, 0x3fff, 17, 17}, /* 51 */
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
            {1007, 756, 504, 0x3fff, 68, 68}, /* 0 */
            {1007, 756, 504, 0x3fff, 68, 68}, /* 1 */
            {1007, 756, 504, 0x3fff, 68, 68}, /* 2 */
            {1007, 756, 504, 0x3fff, 68, 68}, /* 3 */
            {1007, 756, 504, 0x3fff, 68, 68}, /* 4 */
            {1007, 756, 504, 0x3fff, 68, 68}, /* 5 */
            {1007, 756, 504, 0x3fff, 68, 68}, /* 6 */
            {1007, 756, 504, 0x3fff, 68, 68}, /* 7 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 8 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 9 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 10 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 11 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 12 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 13 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 14 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 15 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 16 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 17 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 18 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 19 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 20 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 21 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 22 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 23 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 24 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 25 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 26 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 27 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 28 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 29 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 30 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 31 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 32 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 33 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 34 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 35 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 36 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 37 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 38 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 39 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 40 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 41 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 42 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 43 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 44 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 45 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 46 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 47 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 48 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 49 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 50 */
            {271, 204, 136, 0x3fff, 17, 17}, /* 51 */
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
        {258, 194, 129, 0x3fff, 64, 64},

        /* XL queues */
        { {256, 192, 128, 0x3fff, 64, 64}, {256, 192, 128, 0x3fff, 64, 64} },

        /* Ingress Redirection Queues */
        { {128, 96, 64, 0x3fff, 0, 64}, {128, 96, 64, 0x3fff, 0, 64} },

        /* Egress Redirection Queues */
        { {128, 96, 64, 0x3fff, 0, 64}, {128, 96, 64, 0x3fff, 0, 64} },

        /* Ingress Bubble Queues */
        {128, 96, 64, 0x3fff, 0, 64},

        /* Egress Bubble Queues */
        {128, 96, 64, 0x3fff, 0, 64},

    },

    /*
     * Line PT Config
     */
    {
        /* Fifo info */
        {
            51,
            {
                {0,0,8,2},    /* Port 0 */
                {1,64,8,2},    /* Port 1 */
                {2,128,8,2},    /* Port 2 */
                {3,192,8,2},    /* Port 3 */
                {4,256,8,2},    /* Port 4 */
                {5,320,8,2},    /* Port 5 */
                {6,384,8,2},    /* Port 6 */
                {7,448,8,2},    /* Port 7 */
                {8,512,8,2},    /* Port 8 */
                {9,576,8,2},    /* Port 9 */
                {10,640,8,2},    /* Port 10 */
                {11,704,8,2},    /* Port 11 */
                {0},    /* Port 12 */
                {0},    /* Port 13 */
                {0},    /* Port 14 */
                {0},    /* Port 15 */
                {0},    /* Port 16 */
                {0},    /* Port 17 */
                {0},    /* Port 18 */
                {0},    /* Port 19 */
                {0},    /* Port 20 */
                {0},    /* Port 21 */
                {0},    /* Port 22 */
                {0},    /* Port 23 */
                {0},    /* Port 24 */
                {0},    /* Port 25 */
                {0},    /* Port 26 */
                {0},    /* Port 27 */
                {0},    /* Port 28 */
                {0},    /* Port 29 */
                {0},    /* Port 30 */
                {0},    /* Port 31 */
                {0},    /* Port 32 */
                {0},    /* Port 33 */
                {0},    /* Port 34 */
                {0},    /* Port 35 */
                {0},    /* Port 36 */
                {0},    /* Port 37 */
                {0},    /* Port 38 */
                {0},    /* Port 39 */
                {0},    /* Port 40 */
                {0},    /* Port 41 */
                {0},    /* Port 42 */
                {0},    /* Port 43 */
                {0},    /* Port 44 */
                {0},    /* Port 45 */
                {0},    /* Port 46 */
                {0},    /* Port 47 */
                {48,768,2,1},    /* Port 48 */
                {49,784,2,1},    /* Port 49 */
                {50,800,2,1},    /* Port 50 */
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
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENTX, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENTX, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENTX, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENTX, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0,
              CLIENT0, CLIENT0, CLIENTX, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENTX,
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0,
              CLIENT0, CLIENT0, CLIENTX, CLIENT0, CLIENT0, CLIENT0,
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENTX 
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
        { 255, 192, 128, 0, 228 },
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
           {116, 87, 58, 0, 34, 34},      /* Line Queue cfg */
           {543, 408, 272, 155, 33, 33},  /* Fab Queue cfg */
           {0},  /* PT fifo   */
        },
        /*  CLPORT_10GE  */
        { 
           {310, 233, 155, 0, 68, 68},      /* Line Queue cfg */
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
           {108, 81, 54, 0, 19, 19},      /* Line Queue cfg */
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
           {258, 194, 129, 0, 64, 64},  /* Line Queue cfg */
           {258, 194, 129, 0, 64, 64},  /* Fab Queue cfg */
           {0},  /* PT fifo   */
        },
        /* XLPORT        */
        { 
           {256, 192, 128, 0, 64, 64},  /* Line Queue cfg */
           {256, 192, 128, 0, 64, 64},  /* Fab Queue cfg */
           {0},  /* PT fifo   */
        },
    }}

};

#endif
