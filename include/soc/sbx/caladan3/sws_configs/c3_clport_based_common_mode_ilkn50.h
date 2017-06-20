
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $Id: c3_clport_based_common_mode_ilkn50.h.h, Broadcom SDK $
 */
#ifdef BCM_CALADAN3_SUPPORT

#define BCM_C3_TDM_OVERSUBSCRIBED 1

sws_config_t c3_clport_based_common_mode_cfg = {

    /*
     * QM Config
     */
     {
         /* Buffer Thresholds

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
           3548, 2661, 1774,
           0x3fff, 355, 11527, 
           8646, 5764, 10300,
           1153, 38
        },

        /* Queue Config */
        52, 
        52, 
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

            /* Qs for XE and SGMII GE ports. Supports *UP TO* 10GE rate. 
             * Single Q per port model.
             */

            {350, 233, 155, 0x3FFF, 68, 68}, /* 0 */
            {350, 233, 155, 0x3FFF, 68, 68}, /* 1 */
            {350, 233, 155, 0x3FFF, 68, 68}, /* 2 */ 
            {350, 233, 155, 0x3FFF, 68, 68}, /* 3 */ 
            {350, 233, 155, 0x3FFF, 68, 68}, /* 4 */
            {350, 233, 155, 0x3FFF, 68, 68}, /* 5 */
            {350, 233, 155, 0x3FFF, 68, 68}, /* 6 */
            {350, 233, 155, 0x3FFF, 68, 68}, /* 7 */
            {350, 233, 155, 0x3FFF, 68, 68}, /* 8 */
            {350, 233, 155, 0x3FFF, 68, 68}, /* 9 */
            {350, 233, 155, 0x3FFF, 68, 68}, /* 10 */
            {350, 233, 155, 0x3FFF, 68, 68}, /* 11 */

            /*HG10 - up to 10, 1GE channels/queues*/               
            {64, 48, 32, 0x3FFF, 17, 17}, /* 12 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 13 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 14 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 15 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 16 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 17 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 18 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 19 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 20 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 21 */

            /*HG10 - up to 10, 1GE channels*/
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

            /*HG10 - up to 10, 1GE  channels*/
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

            /*HG10 - up to 10, 1GE channels*/
            {64, 48, 32, 0x3FFF, 17, 17}, /* 42 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 43 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 44 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 45 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 46 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 47 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 48 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 49 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 50 */
            {64, 48, 32, 0x3FFF, 17, 17}, /* 51 */
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
            /* 
             * We can not know a-priori what physical ports the HG10
             * logical ports will be associated with.
             * Fabric Qs get allocated on a first-come first-serve basis, 
             * (sequentially) unlike line-side Qs. Therefore, the MAX size must be allocated 
             * for all Fab Qs!
             */

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

            {1007, 756, 504, 227, 68, 68}, /* 12 */
            {1007, 756, 504, 227, 68, 68}, /* 13 */
            {1007, 756, 504, 227, 68, 68}, /* 14 */
            {1007, 756, 504, 227, 68, 68}, /* 15 */
            {1007, 756, 504, 227, 68, 68}, /* 16 */
            {1007, 756, 504, 227, 68, 68}, /* 17 */
            {1007, 756, 504, 227, 68, 68}, /* 18 */
            {1007, 756, 504, 227, 68, 68}, /* 19 */
            {1007, 756, 504, 227, 68, 68}, /* 20 */
            {1007, 756, 504, 227, 68, 68}, /* 21 */
            {1007, 756, 504, 227, 68, 68}, /* 22 */
            {1007, 756, 504, 227, 68, 68}, /* 23 */
            {1007, 756, 504, 227, 68, 68}, /* 24 */
            {1007, 756, 504, 227, 68, 68}, /* 25 */
            {1007, 756, 504, 227, 68, 68}, /* 26 */
            {1007, 756, 504, 227, 68, 68}, /* 27 */
            {1007, 756, 504, 227, 68, 68}, /* 28 */
            {1007, 756, 504, 227, 68, 68}, /* 29 */
            {1007, 756, 504, 227, 68, 68}, /* 30 */
            {1007, 756, 504, 227, 68, 68}, /* 31 */
            {1007, 756, 504, 227, 68, 68}, /* 32 */
            {1007, 756, 504, 227, 68, 68}, /* 33 */
            {1007, 756, 504, 227, 68, 68}, /* 34 */
            {1007, 756, 504, 227, 68, 68}, /* 35 */
            {1007, 756, 504, 227, 68, 68}, /* 36 */
            {1007, 756, 504, 227, 68, 68}, /* 37 */
            {1007, 756, 504, 227, 68, 68}, /* 38 */
            {1007, 756, 504, 227, 68, 68}, /* 39 */
            {1007, 756, 504, 227, 68, 68}, /* 40 */
            {1007, 756, 504, 227, 68, 68}, /* 41 */
            {1007, 756, 504, 227, 68, 68}, /* 42 */
            {1007, 756, 504, 227, 68, 68}, /* 43 */
            {1007, 756, 504, 227, 68, 68}, /* 44 */
            {1007, 756, 504, 227, 68, 68}, /* 45 */
            {1007, 756, 504, 227, 68, 68}, /* 46 */
            {1007, 756, 504, 227, 68, 68}, /* 47 */
            {1007, 756, 504, 227, 68, 68}, /* 48 */
            {1007, 756, 504, 227, 68, 68}, /* 49 */
            {1007, 756, 504, 227, 68, 68}, /* 50 */
            {1007, 756, 504, 227, 68, 68}, /* 51 */
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
                {1,64,8,2},   /* Port 1 */
                {2,128,8,2},  /* Port 2 */
                {3,192,8,2},  /* Port 3 */
                {4,256,8,2},  /* Port 4 */
                {5,320,8,2},  /* Port 5 */
                {6,384,8,2},  /* Port 6 */
                {7,448,8,2},  /* Port 7 */
                {8,512,8,2},  /* Port 8 */
                {9,576,8,2},  /* Port 9 */
                {10,640,8,2}, /* Port 10 */
                {11,704,8,2}, /* Port 11 */

                {12,0,0,0},   /* Port 12 */
                {13,0,0,0},   /* Port 13 */
                {14,0,0,0},   /* Port 14 */
                {15,0,0,0},   /* Port 15 */
                {16,0,0,0},   /* Port 16 */
                {17,0,0,0},   /* Port 17 */
                {18,0,0,0},   /* Port 18 */
                {19,0,0,0},   /* Port 19 */
                {20,0,0,0},   /* Port 20 */
                {21,0,0,0},   /* Port 21 */
                {22,0,0,0},   /* Port 22 */
                {23,0,0,0},   /* Port 23 */
                {24,0,0,0},   /* Port 24 */
                {25,0,0,0},   /* Port 25 */
                {26,0,0,0},   /* Port 26 */
                {27,0,0,0},   /* Port 27 */
                {28,0,0,0},   /* Port 28 */
                {29,0,0,0},   /* Port 29 */
                {30,0,0,0},   /* Port 30 */
                {31,0,0,0},   /* Port 31 */
                {32,0,0,0},   /* Port 32 */
                {33,0,0,0},   /* Port 33 */
                {34,0,0,0},   /* Port 34 */
                {35,0,0,0},   /* Port 35 */
                {36,0,0,0},   /* Port 36 */
                {37,0,0,0},   /* Port 37 */
                {38,0,0,0},   /* Port 38 */
                {39,0,0,0},   /* Port 39 */
                {40,0,0,0},   /* Port 40 */
                {41,0,0,0},   /* Port 41 */
                {42,0,0,0},   /* Port 42 */
                {43,0,0,0},   /* Port 43 */
                {44,0,0,0},   /* Port 44 */
                {45,0,0,0},   /* Port 45 */
                {46,0,0,0},   /* Port 46 */
                {47,0,0,0},   /* Port 47 */
                {48,768,2,1},    /* Port 48 */
                {49,784,2,1},    /* Port 49 */
                {50,800,2,1},    /* Port 50 */
            }
        },

        /* Client Calendar */
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

        /* Port Calendar */
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

     {
         {
             {
                 {0,0,0,0,0,0}, {0,0,0,0,0,0}, {0,0,0,0}
             }
         }
     },

     { 
         BCM_C3_TDM_OVERSUBSCRIBED
     },


};

#endif
