
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $Id: c3_2x10g_32x1g_1xhg127.h ,v 1.1.2.4 Broadcom SDK $
 */
#ifdef BCM_CALADAN3_SUPPORT

sws_config_t c3_2x10g_32x1g_1xhg127_cfg = {

    /*
     * QM Config
     */
     {
        /* Buffer Thresholds */
         /*
           total_buff_max_pages, total_buff_drop_thres_de1, total_buff_drop_thres_de2,
           fc_total_buffer_xoff_thresh, num_pages_reserved, total_buff_hysteresis_delta,
           ingress_max_pages, ingress_drop_thres_de1, ingress_drop_thres_de2,
           fc_ingress_xoff_thresh, ingress_hysteresis_delta, egress_max_pages,
           egress_drop_thres_de1, egress_drop_thres_de2, fc_egress_xoff_thresh,
           egress_hysteresis_delta, per_queue_drop_hysteresis_delta
         */
        {
            15384, 11538, 7692, 
            11226, 3776, 1539, 
            6818, 5114, 3409,
            10300, 682, 8257,
            6193, 4129, 10300,
            826, 38
        },

        /* Queue Config */
        36, 
        36, 
        {
            /* 
             * Note the Queue ordering here must mirror the Q number space implicitly 
             * defined in the C src!.  
             * 0-11 for XE/GE ports on CLPORT0
             * 12-47 for GE ports on XTPORT0-XTPORT2
             * HG10 appears to start at 12 overlapping number space with XTPORT0
             * and possibly XTPORT1 depending on how many 1G channels on HG10 interface!
             */

            /*
              max_pages, drop_thres_de1, drop_thres_de2,
              flow_ctrl_thresh, min_pages_data, min_pages_header
            */
            {310, 233, 155, 0x3FFF, 68, 68}, /* 0 XE0 */
            {310, 233, 155, 0x3FFF, 68, 68}, /* 1 XE1 */

            {310, 233, 155, 0x3FFF, 68, 68}, /* 2 */ 
            {310, 233, 155, 0x3FFF, 68, 68}, /* 3 */ 
            {0}, /* 4 */
            {0}, /* 5 */
            {0}, /* 6 */
            {0}, /* 7 */
            {0}, /* 8 */
            {0}, /* 9 */
            {0}, /* 10 */
            {0}, /* 11 */
            {0}, /* 12 */
            {0}, /* 13 */
            {0}, /* 14 */
            {0}, /* 15 */

            /*GEs */

            {64, 48, 32, 0x3FFF, 17, 17}, /* 16 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 17 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 18 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 19 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 20 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 21 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 22 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 23 */                                    
            {64, 48, 32, 0x3FFF, 17, 17}, /* 24 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 25 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 26 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 27 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 28 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 29 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 30 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 31 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 32 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 33 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 34 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 35 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 36 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 37 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 38 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 39 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 40 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 41 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 42 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 43 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 44 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 45 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 46 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 47 */

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

            /* GEs */
            {271, 204, 136, 121, 17, 17}, /* 4 */
            {271, 204, 136, 121, 17, 17}, /* 5 */
            {271, 204, 136, 121, 17, 17}, /* 6 */
            {271, 204, 136, 121, 17, 17}, /* 7 */
            {271, 204, 136, 121, 17, 17}, /* 8 */
            {271, 204, 136, 121, 17, 17}, /* 9 */
            {271, 204, 136, 121, 17, 17}, /* 10 */
            {271, 204, 136, 121, 17, 17}, /* 11 */
            {271, 204, 136, 121, 17, 17}, /* 12 */
            {271, 204, 136, 121, 17, 17}, /* 13 */
            {271, 204, 136, 121, 17, 17}, /* 14 */
            {271, 204, 136, 121, 17, 17}, /* 15 */
            {271, 204, 136, 121, 17, 17}, /* 16 */
            {271, 204, 136, 121, 17, 17}, /* 17 */
            {271, 204, 136, 121, 17, 17}, /* 18 */
            {271, 204, 136, 121, 17, 17}, /* 19 */
            {271, 204, 136, 121, 17, 17}, /* 20 */
            {271, 204, 136, 121, 17, 17}, /* 21 */
            {271, 204, 136, 121, 17, 17}, /* 22 */
            {271, 204, 136, 121, 17, 17}, /* 23 */
            {271, 204, 136, 121, 17, 17}, /* 24 */
            {271, 204, 136, 121, 17, 17}, /* 25 */
            {271, 204, 136, 121, 17, 17}, /* 26 */
            {271, 204, 136, 121, 17, 17}, /* 27 */
            {271, 204, 136, 121, 17, 17}, /* 28 */
            {271, 204, 136, 121, 17, 17}, /* 29 */
            {271, 204, 136, 121, 17, 17}, /* 30 */
            {271, 204, 136, 121, 17, 17}, /* 31 */
            {271, 204, 136, 121, 17, 17}, /* 32 */
            {271, 204, 136, 121, 17, 17}, /* 33 */
            {271, 204, 136, 121, 17, 17}, /* 34 */
            {271, 204, 136, 121, 17, 17}, /* 35 */
            {271, 204, 136, 121, 17, 17}, /* 36 */
            {271, 204, 136, 121, 17, 17}, /* 37 */
            {0}, /* 38 */
            {0}, /* 39 */
            {0}, /* 40 */
            {0}, /* 41 */
            {0}, /* 42 */
            {0}, /* 43 */
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
                {0,0,8,2},    /* Port 0 XE0*/
                {1,64,8,2},   /* Port 1 XE1 */
                {2,128,8,2},    /* Port 2 Placeholder port */
                {3,192,8,2},    /* Port 3 Placeholder port */
                {4,0,0,0},    /* Port 4 */
                {5,0,0,0},    /* Port 5 */
                {6,0,0,0},    /* Port 6 */
                {7,0,0,0},    /* Port 7 */
                {8,0,0,0},    /* Port 8 */
                {9,0,0,0},    /* Port 9 */
                {10,0,0,0},   /* Port 10 */
                {11,0,0,0},   /* Port 11 */
                {12,0,0,0},   /* Port 12 */
                {13,0,0,0},   /* Port 13 */
                {14,0,0,0},   /* Port 14 */
                {15,0,0,0},   /* Port 15 */
                /* GEs */
                {16,208,2,1},   /* Port 16 */
                {17,224,2,1},   /* Port 17 */
                {18,240,2,1},   /* Port 18 */
                {19,256,2,1},   /* Port 19 */
                {20,272,2,1},   /* Port 20 */
                {21,288,2,1},   /* Port 21 */
                {22,304,2,1},   /* Port 22 */
                {23,320,2,1},   /* Port 23 */
                {24,336,2,1},   /* Port 24 */
                {25,352,2,1},   /* Port 25 */
                {26,368,2,1},   /* Port 26 */
                {27,384,2,1},   /* Port 27 */
                {28,400,2,1},   /* Port 28 */
                {29,416,2,1},   /* Port 29 */
                {30,432,2,1},   /* Port 30 */
                {31,448,2,1},   /* Port 31 */
                {32,464,2,1},   /* Port 32 */
                {33,480,2,1},   /* Port 33 */
                {34,496,2,1},   /* Port 34 */
                {35,512,2,1},   /* Port 35 */
                {36,528,2,1},   /* Port 36 */
                {37,544,2,1},   /* Port 37 */
                {38,560,2,1},   /* Port 38 */
                {39,576,2,1},   /* Port 39 */
                {40,592,2,1},   /* Port 40 */
                {41,608,2,1},   /* Port 41 */
                {42,624,2,1},   /* Port 42 */
                {43,640,2,1},   /* Port 43 */
                {44,656,2,1},  /* Port 44 */
                {45,672,2,1},  /* Port 45 */
                {46,688,2,1},  /* Port 46 */
                {47,704,2,1},  /* Port 47 */
                {48,0,0,0},   /* Port 48 */
                {49,0,0,0},   /* Port 49 */
                {50,0,0,0}    /* Port 50 */
            }
        },

        /* Client Calendar */
        {
            /*
             * IMPORTANT:
             * Calendar spacing of at least 4 entries is required
             * for XTPORT clients! If they are closer then MACs
             * appear to be overdriven result in corrupt frames
             * to MACs resulting in pseudorandom traffic distribution
             * to undesired GE ports.
             * In addition, spacing greater than 8 can result
             * in FIFO underrun for MACs. 
             */
            51,
            {
              CLIENT0, CLIENT1, CLIENT2, CLIENT3, CLIENT4, 
              CLIENT0, CLIENT1, CLIENT2, CLIENT3, CLIENT0, 
              CLIENT0, CLIENT1, CLIENT2, CLIENT3, CLIENT5, 
              CLIENT0, CLIENT1, CLIENT2, CLIENT3, CLIENT0, 
              CLIENT1, CLIENT2, CLIENT3, CLIENT0, CLIENT1, 
              CLIENT2, CLIENT3, CLIENT0, CLIENT1, CLIENT2, 
              CLIENT3, CLIENT0, CLIENT1, CLIENT2, CLIENT3, 
              CLIENT0, CLIENT1, CLIENT2, CLIENT3, CLIENT0, 
              CLIENT1, CLIENT2, CLIENT3, CLIENT0, CLIENT1, 
              CLIENT2, CLIENT3, CLIENT0, CLIENT1, CLIENT2,
              CLIENT3

            },
        },

        /* Port Calendar */
        {
            75,
            {

                0,1,2,3,16,17,18,
                19,0,1,2,3,20,21,
                22,23,0,1,2,3,24,
                25,26,27,0,1,2,3,
                28,29,30,31,0,
                1,2,3,32,33,34,35,
                0,1,2,3,36,37,38,
                39,0,1,2,3,40,41,
                42,43,0,1,2,3,44,
                45,46,47,0,1,2,3,
                48,0,1,2,3,49,50,


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
                 CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0,
                 CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0,
                 CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0,
                 CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0,
                 CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0,
                 CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0,
                 CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0,
                 CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0,
                 CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0, CLIENT0,
                 CLIENT0, CLIENT0, CLIENT0, CLIENT0,
            },
        },

        /* Port Cal */
        {
            108,
            {
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
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
