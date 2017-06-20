
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $Id: c3_1x100g_1xil100.h,v 1.2.24.1 Broadcom SDK $
 */
#ifdef BCM_CALADAN3_SUPPORT

sws_config_t c3_4cos_1x100g_1xil100_cfg = {


    /*
     * QM Config
     */
     {
        /* Buffer Thresholds */
        {
           15384, 11538, 7692, 11226, 3776, 1539, 6818, 5114, 
           3409, 0x3fff, 682, 8257, 6193, 4129, 7357, 826, 38
        },

        /* Queue Config */
        1, 
        1, 
        {
            {1152, 864, 576, 0x3fff, 102, 102}, /* 0 */
            {1152, 864, 576, 0x3fff, 102, 102}, /* 1 */
            {1152, 864, 576, 0x3fff, 102, 102}, /* 2 */
            {1152, 864, 576, 0x3fff, 102, 102}, /* 3 */
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
            {1395, 1047, 698, 1257, 102, 102}, /* 0 */
            {1395, 1047, 698, 1257, 102, 102}, /* 1 */
            {1395, 1047, 698, 1257, 102, 102}, /* 2 */
            {1395, 1047, 698, 1257, 102, 102}, /* 3 */
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
                {0,0,76,20},    /* Port 0 */
                {0},    /* Port 1 */
                {0},    /* Port 2 */
                {0},    /* Port 3 */
                {0},    /* Port 4 */
                {0},    /* Port 5 */
                {0},    /* Port 6 */
                {0},    /* Port 7 */
                {0},    /* Port 8 */
                {0},    /* Port 9 */
                {0},    /* Port 10 */
                {0},    /* Port 11 */
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
                {48,608,4,1},    /* Port 48 */
                {49,640,4,1},    /* Port 49 */
                {50,672,4,1},    /* Port 50 */
            }
        },

        /* Client Cal */
        {
            72,
            {
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT4, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT4, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT5, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0,
              CLIENT0, CLIENT0, CLIENT0, CLIENT4, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, 
            },
        },

        /* Port Cal */
        {
            108,
            {
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
              0, 0, 0, 0, 0, 48, 0, 0, 0, 0, 
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
              0, 49, 0, 0, 0, 0, 0, 0, 0, 0, 
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
              0, 0, 0, 0, 0, 0, 0, 50,
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
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0,
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, 
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0,
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0,
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0,
              CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0 
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
              0, 0, 0
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
