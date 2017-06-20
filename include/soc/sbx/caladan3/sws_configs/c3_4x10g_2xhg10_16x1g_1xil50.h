
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $Id: c3_4x10g_2xhg10_16x1g_1xil50.h,v 1.1.2.4 Broadcom SDK $
 */
#ifdef BCM_CALADAN3_SUPPORT

sws_config_t c3_4x10g_2xhg10_16x1g_1xil50_cfg = {


    /*
     * QM Config
     */
     {
        /* Buffer Thresholds */
        {
           15384, 11538, 7692, 11291, 3728, 1539, 3216, 2412, 
           1608, 0, 322, 11861, 8896, 5931, 10616, 1187, 38
        },

        /* Queue Config */
        40, 
        40, 
        {
	    {256, 153, 102, 0x3fff, 68, 68}, /* 0 Populate all even though only 4 will be used, this allow flexible mapping to wc lane */
	    {256, 153, 102, 0x3fff, 68, 68}, /* 1 */
	    {256, 153, 102, 0x3fff, 68, 68}, /* 2 */
	    {256, 153, 102, 0x3fff, 68, 68}, /* 3 */
	    {256, 153, 102, 0x3fff, 68, 68}, /* 4 */
	    {256, 153, 102, 0x3fff, 68, 68}, /* 5 */
	    {256, 153, 102, 0x3fff, 68, 68}, /* 6 */
	    {256, 153, 102, 0x3fff, 68, 68}, /* 7 */
	    {256, 153, 102, 0x3fff, 68, 68}, /* 8 */ 
	    {256, 153, 102, 0x3fff, 68, 68}, /* 9 */ 
	    {256, 153, 102, 0x3fff, 68, 68}, /* 10 */
	    {256, 153, 102, 0x3fff, 68, 68}, /* 11 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 12 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 13 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 14 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 15 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 16 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 17 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 18 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 19 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 20 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 21 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 22 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 23 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 24 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 25 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 26 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 27 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 28 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 29 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 30 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 31 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 32 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 33 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 34 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 35 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 36 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 37 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 38 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 39 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 40 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 41 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 42 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 43 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 44 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 45 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 46 */
            {109, 82, 55, 0x3fff, 26, 26}, /* 47 */
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
            {459, 345, 230, 122, 24, 24}, /* 0 */
            {459, 345, 230, 122, 24, 24}, /* 1 */
            {459, 345, 230, 122, 24, 24}, /* 2 */
            {459, 345, 230, 122, 24, 24}, /* 3 */
            {459, 345, 230, 122, 24, 24}, /* 4 */
            {459, 345, 230, 122, 24, 24}, /* 5 */
            {459, 345, 230, 122, 24, 24}, /* 6 */
            {459, 345, 230, 122, 24, 24}, /* 7 */
            {459, 345, 230, 122, 24, 24}, /* 8 */
            {459, 345, 230, 122, 24, 24}, /* 9 */
            {459, 345, 230, 122, 24, 24}, /* 10 */
            {459, 345, 230, 122, 24, 24}, /* 11 */
            {459, 345, 230, 122, 24, 24}, /* 12 */
            {459, 345, 230, 122, 24, 24}, /* 13 */
            {459, 345, 230, 122, 24, 24}, /* 14 */
            {459, 345, 230, 122, 24, 24}, /* 15 */
            {459, 345, 230, 122, 24, 24}, /* 16 */
            {459, 345, 230, 122, 24, 24}, /* 17 */
            {459, 345, 230, 122, 24, 24}, /* 18 */
            {459, 345, 230, 122, 24, 24}, /* 19 */
            {638, 479, 319, 229, 49, 49}, /* 20 */
            {638, 479, 319, 229, 49, 49}, /* 21 */
            {638, 479, 319, 229, 49, 49}, /* 22 */
            {638, 479, 319, 229, 49, 49}, /* 23 */
            {459, 345, 230, 122, 24, 24}, /* 24 */
            {459, 345, 230, 122, 24, 24}, /* 25 */
            {459, 345, 230, 122, 24, 24}, /* 26 */
            {459, 345, 230, 122, 24, 24}, /* 27 */
            {459, 345, 230, 122, 24, 24}, /* 28 */
            {459, 345, 230, 122, 24, 24}, /* 29 */
            {459, 345, 230, 122, 24, 24}, /* 30 */
            {459, 345, 230, 122, 24, 24}, /* 31 */
            {459, 345, 230, 122, 24, 24}, /* 32 */
            {459, 345, 230, 122, 24, 24}, /* 33 */
            {459, 345, 230, 122, 24, 24}, /* 34 */
            {459, 345, 230, 122, 24, 24}, /* 35 */
            {459, 345, 230, 122, 24, 24}, /* 36 */
            {459, 345, 230, 122, 24, 24}, /* 37 */
            {459, 345, 230, 122, 24, 24}, /* 38 */
            {459, 345, 230, 122, 24, 24}, /* 39 */
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
                {6,384,8,2},    /* Port 6. Reserved but not used */
                {7,448,8,2},    /* Port 7. Reserved but not used */
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
                {32,512,2,1},    /* Port 32 */
                {33,528,2,1},    /* Port 33 */
                {34,544,2,1},    /* Port 34 */
                {35,560,2,1},    /* Port 35 */
                {36,576,2,1},    /* Port 36 */
                {37,592,2,1},    /* Port 37 */
                {38,608,2,1},    /* Port 38 */
                {39,624,2,1},    /* Port 39 */
                {40,640,2,1},    /* Port 40 */
                {41,656,2,1},    /* Port 41 */
                {42,672,2,1},    /* Port 42 */
                {43,688,2,1},    /* Port 43 */
                {44,704,2,1},    /* Port 44 */
                {45,720,2,1},    /* Port 45 */
                {46,736,2,1},    /* Port 46 */
                {47,752,2,1},    /* Port 47 */
                {48,768,2,1},    /* Port 48 */
                {49,784,2,1},    /* Port 49 */
                {50,800,2,1},    /* Port 50 */
            }
        },

        /* Client Cal */
        {
            60,
            {
                 CLIENT0, CLIENT3, CLIENT0, CLIENT2, CLIENT0, 
                 CLIENT0, CLIENT3, CLIENT0, CLIENT0, CLIENT4, 
                 CLIENT0, CLIENT0, CLIENT3, CLIENT0, CLIENT0, 
                 CLIENT0, CLIENT0, CLIENT0, CLIENT3, CLIENT0, 
                 CLIENT0, CLIENT2, CLIENT0, CLIENT0, CLIENT3, 
                 CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
                 CLIENT3, CLIENT0, CLIENT0, CLIENT2, CLIENT0, 
                 CLIENT0, CLIENT3, CLIENT0, CLIENT0, CLIENT4, 
                 CLIENT0, CLIENT0, CLIENT3, CLIENT0, CLIENT0, 
                 CLIENT2, CLIENT0, CLIENT3, CLIENT0, CLIENT0, 
                 CLIENT5, CLIENT0, CLIENT3, CLIENT0, CLIENT0, 
                 CLIENT2, CLIENT0, CLIENT3, CLIENT0, CLIENT0
            },
        },

        /* Port Cal */
        {
            99,
            {
                0, 1, 2, 3, 4, 5, PORTX, PORTX, 32, 33, 
                0, 1, 2, 3, 4, 5, PORTX, PORTX, 34, 35, 
                0, 1, 2, 3, 4, 5, PORTX, PORTX, 36, 37, 
                0, 1, 2, 3, 4, 5, PORTX, PORTX, 38, 39, 
                0, 1, 2, 3, 4, 5, PORTX, PORTX, 40, 41, 
                0, 1, 2, 3, 4, 5, PORTX, PORTX, 42, 43, 
                0, 1, 2, 3, 4, 5, PORTX, PORTX, 48, 49, 
                0, 1, 2, 3, 4, 5, PORTX, PORTX, 50, 44, 
                0, 1, 2, 3, 4, 5, PORTX, PORTX, 45, 46, 
                0, 1, 2, 3, 4, 5, PORTX, PORTX, 47
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
            76,
            {
                 CLIENT0, CLIENTX, CLIENT0, CLIENTX, CLIENT0, CLIENTX, CLIENT0, CLIENTX,
                 CLIENT0, CLIENTX, CLIENT0, CLIENTX, CLIENT0, CLIENTX, CLIENT0, CLIENTX,
                 CLIENT0, CLIENTX, CLIENT0, CLIENTX, CLIENT0, CLIENTX, CLIENT0, CLIENTX,
                 CLIENT0, CLIENTX, CLIENT0, CLIENTX, CLIENT0, CLIENTX, CLIENT0, CLIENTX,
                 CLIENT0, CLIENTX, CLIENT0, CLIENTX, CLIENT0, CLIENTX, CLIENT0, CLIENTX,
                 CLIENT0, CLIENTX, CLIENT0, CLIENTX, CLIENT0, CLIENTX, CLIENT0, CLIENTX,
                 CLIENT0, CLIENTX, CLIENT0, CLIENTX, CLIENT0, CLIENTX, CLIENT0, CLIENTX,
                 CLIENT0, CLIENTX, CLIENT0, CLIENTX, CLIENT0, CLIENTX, CLIENT0, CLIENTX,
                 CLIENT0, CLIENTX, CLIENT0, CLIENTX, CLIENT0, CLIENTX, CLIENT0, CLIENTX,
                 CLIENT0, CLIENTX, CLIENT0, CLIENTX,
            },
        },

        /* Port Cal */
        {
            108,
            {
              0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX,
              0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX,
              0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX,
              0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX,
              0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX,
              0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX,
              0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX, 0, PORTX
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


};

#endif
