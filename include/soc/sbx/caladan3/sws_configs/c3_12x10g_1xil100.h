
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $Id: c3_12x10g_1xil100.h,v 1.2 Broadcom SDK $
 */
#ifdef BCM_CALADAN3_SUPPORT

sws_config_t c3_12x10g_1xil100_cfg = {


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
        12, 
        12, 
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
            {310, 233, 155, 0x3fff, 68, 68}, /* 8 */
            {310, 233, 155, 0x3fff, 68, 68}, /* 9 */
            {310, 233, 155, 0x3fff, 68, 68}, /* 10 */
            {310, 233, 155, 0x3fff, 68, 68}, /* 11 */
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
            {1007, 756, 504, 227, 68, 68}, /* 0 */
            {1007, 756, 504, 227, 68, 68}, /* 1 */
            {1007, 756, 504, 227, 68, 68}, /* 2 */
            {1007, 756, 504, 227, 68, 68}, /* 3 */
            {1007, 756, 504, 227, 68, 68}, /* 4 */
            {1007, 756, 504, 227, 68, 68}, /* 5 */
            {1007, 756, 504, 227, 68, 68}, /* 6 */
            {1007, 756, 504, 227, 68, 68}, /* 7 */
            {1007, 756, 504, 227, 68, 68}, /* 8 */
            {1007, 756, 504, 227, 68, 68}, /* 9 */
            {1007, 756, 504, 227, 68, 68}, /* 10 */
            {1007, 756, 504, 227, 68, 68}, /* 11 */
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
        {258, 194, 129, 0, 64, 64},

        /* XL queues */
        { {256, 192, 128, 0, 64, 64}, {256, 192, 128, 0, 64, 64} },

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

};

#endif
