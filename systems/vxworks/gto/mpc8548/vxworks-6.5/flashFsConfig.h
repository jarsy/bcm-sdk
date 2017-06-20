/*
 * $Id: flashFsConfig.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _FLASH_DRV_CONFIG_H_
#define _FLASH_DRV_CONFIG_H_

#define SIZE_1K  1024
#define SIZE_1M  (SIZE_1K * SIZE_1K)
#define SIZE_1G  (SIZE_1K * SIZE_1M)

typedef enum dev_resource_type_e {
    devResourceUint32
} dev_resource_type_t;

typedef struct dev_resource_s {
    char *          resource_name;
    unsigned int    type;
    unsigned int    value;
} dev_resource_t; 

typedef struct dev_list_s {
    char *                 dev_name;
    int                    count;
    const dev_resource_t * resource;
} dev_list_t;

const dev_resource_t flash0_resource[] = {
    {"device_id",      devResourceUint32, 0x227e},
    {"sector_sz",      devResourceUint32, 128 * SIZE_1K},
    {"sector_count",   devResourceUint32, SIZE_1K},
    {"base_addr",      devResourceUint32, 0xf8000000},
    {"ptable_offset",  devResourceUint32, 126 * SIZE_1M}, 
    {"ptable_size",    devResourceUint32, SIZE_1M},
    {"bootrom_offset", devResourceUint32, 127 * SIZE_1M},
    {"bootrom_size",   devResourceUint32, SIZE_1M},
};

const dev_resource_t flash1_resource[] = {
    {"device_id",      devResourceUint32, 0x227e},
    {"sector_sz",      devResourceUint32, 128 * SIZE_1K},
    {"sector_count",   devResourceUint32, SIZE_1K},
    {"base_addr",      devResourceUint32, 0xf0000000},
    {"bootrom_offset", devResourceUint32, 127 * SIZE_1M},
    {"bootrom_size",   devResourceUint32, SIZE_1M},
};

const dev_list_t flash_dev_list[] = {
    {"flash", NELEMENTS(flash1_resource), flash1_resource}, 
    {"flash", NELEMENTS(flash0_resource), flash0_resource}
};

#endif /* _FLASH_DRV_CONFIG_H_ */
