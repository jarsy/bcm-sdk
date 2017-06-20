/****************************************************************************
 *
 * Copyright 2014-2017 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in an
 * Authorized License, Broadcom grants no license (express or implied), right to
 * use, or waiver of any kind with respect to the Software, and Broadcom expressly
 * reserves all rights in and to the Software and all intellectual property rights
 * therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
 * SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 * TO THE SOFTWARE. BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 * OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 *******************************************************************************/

#include "default_allocator.h"
#include "device.h"
#include "errors.h"
#include "kbp_portable.h"
#include "kbp_legacy.h"
#include "kbp_mem_test.h"
#include "db.h"
#include "key.h"
#include "instruction.h"


#include <getopt.h>

#ifdef USE_OP_PCIE_XPT
#include "kbp_pcie.h"
#endif


#define DEMO_REG_DATA_ARRAY_SIZE           (16)
#define DEMO_REG_80B_DATA_ARRAY_LEN        (80/DEMO_REG_DATA_ARRAY_SIZE)
#define DEMO_GET_80B_ARR_INDEX(bit_index)  ((DEMO_REG_80B_DATA_ARRAY_LEN) - 1 - ((bit_index) / (DEMO_REG_DATA_ARRAY_SIZE)))
#define DEMO_GET_ARR_BIT_OFFSET(bit_index)  ((bit_index) % (DEMO_REG_DATA_ARRAY_SIZE))
#define DEMO_PARITY_SCAN_DELAY              (60*1000000)  /* delay for parity scan: say 60usec */

#define DEMO_MODIFIED_ADDR_LENGTH           (32U)
#define DEMO_INJECT_ERROR_ADDR_LENGTH       (30U)


/**
 * Demo refapp data structures
 */

struct kbp_mem_test_demo {
    struct kbp_device *device; /**< device handles usleep_or_addr */
    struct kbp_mem_test_info info; /**< mem test info, filled by appl */
};

/* private MDIO functions */
int32_t mem_test_demo_mdio_read( void *handle, int32_t chip_no, uint8_t dev, uint16_t reg, uint16_t *value ) {
    struct kbp_mem_test_demo *demo_info = (struct kbp_mem_test_demo*) handle;

    if ( demo_info->info.debug_prints )
        kbp_printf( "\n\t ----  mem_test_demo mdio_read: chip: %x, dev :%x, reg: %x, value :%x ", chip_no, dev, reg, *value );

    return KBP_OK;
}

int32_t mem_test_demo_mdio_write( void *handle, int32_t chip_no, uint8_t dev, uint16_t reg, uint16_t value ) {

    struct kbp_mem_test_demo *demo_info = (struct kbp_mem_test_demo*) handle;

    if ( demo_info->info.debug_prints )
        kbp_printf( "\n\t ----  mem_test_demo mdio_write:  chip: %x, dev :%x, reg: %x, value :%x ", chip_no, dev, reg, value );

    return KBP_OK;
}

#include <unistd.h>

/* callback functions */
/* usleep */
kbp_status mem_test_demo_usleep( void *handle, uint64_t usec ) {

    usleep(usec);
    (void)handle;
    return KBP_OK;
}


kbp_status mem_test_demo_mdio_reg_write_80(void *handle, uint8_t kbp_select, uint8_t device_id, uint16_t addr, uint16_t *write_data) {

    /* write Device Configuration Register (DCR) */
    (void)addr; /* will map DCR 0x0001 address to MDIO : [ ((addr << 3) | A) where A = 0x3,0x4,0x5,0x6,0x7 ]*/
    kbp_printf( "\n\t ----  mem_test_demo_mdio_reg_write_80: %.6x_%.6x_%.6x_%.6x_%.6x ",
        write_data[0], write_data[1], write_data[2], write_data[3], write_data[4]);

    KBP_TRY(mem_test_demo_mdio_write(handle, kbp_select, device_id, 0x000B, write_data[0]));
    KBP_TRY(mem_test_demo_mdio_write(handle, kbp_select, device_id, 0x000C, write_data[1]));
    KBP_TRY(mem_test_demo_mdio_write(handle, kbp_select, device_id, 0x000D, write_data[2]));
    KBP_TRY(mem_test_demo_mdio_write(handle, kbp_select, device_id, 0x000E, write_data[3]));
    KBP_TRY(mem_test_demo_mdio_write(handle, kbp_select, device_id, 0x000F, write_data[4]));

    return KBP_OK;
}

kbp_status mem_test_demo_mdio_reg_write_64(void *handle, uint8_t kbp_select, uint8_t device_id, uint16_t addr, uint16_t *write_data) {

    /* write registers via MDIO */
    (void)addr;
    (void)handle;
    (void)kbp_select;
    (void)device_id;

    kbp_printf( "\n\t ----  mem_test_demo_mdio_reg_write_64: %.6x_%.6x_%.6x_%.6x ",
        write_data[0], write_data[1], write_data[2], write_data[3]);

    return KBP_OK;
}

kbp_status mem_test_demo_mdio_reg_read_80(void *handle, uint8_t kbp_select, uint8_t device_id, uint16_t addr, uint16_t *read_data) {

    /* read Device Configuration Register (DCR) */
    (void)addr; /* will map DCR 0x0001 address to MDIO : [ ((addr << 3) | A) where A = 0x3,0x4,0x5,0x6,0x7 ]*/
    KBP_TRY(mem_test_demo_mdio_read(handle, kbp_select, device_id, 0x000B, &read_data[0]));
    KBP_TRY(mem_test_demo_mdio_read(handle, kbp_select, device_id, 0x000C, &read_data[1]));
    KBP_TRY(mem_test_demo_mdio_read(handle, kbp_select, device_id, 0x000D, &read_data[2]));
    KBP_TRY(mem_test_demo_mdio_read(handle, kbp_select, device_id, 0x000E, &read_data[3]));
    KBP_TRY(mem_test_demo_mdio_read(handle, kbp_select, device_id, 0x000F, &read_data[4]));

    kbp_printf( "\n\t ----  mem_test_demo_mdio_reg_read_80: %.6x_%.6x_%.6x_%.6x_%.6x ",
        read_data[0], read_data[1], read_data[2], read_data[3], read_data[4]);

    return KBP_OK;
}

kbp_status mem_test_demo_mdio_reg_read_64(void *handle, uint8_t kbp_select, uint8_t device_id, uint16_t addr, uint16_t *read_data) {

    /* read register using MDIO */
    (void)addr;
    (void)handle;
    (void)kbp_select;
    (void)device_id;

    kbp_printf( "\n\t ----  mem_test_demo_mdio_reg_read_64: %.6x_%.6x_%.6x_%.6x ",
        read_data[0], read_data[1], read_data[2], read_data[3]);

    return KBP_OK;
}

static void device_type_print(enum kbp_device_type dev_type)
{
    kbp_printf("\n  - Device type: ");

    switch ( dev_type ) {
        case KBP_DEVICE_12K:
            kbp_printf("O3S \n");
            break;
        case KBP_DEVICE_OP:
            kbp_printf("Optimus Prime \n");
            break;
        default:
            kbp_printf("Unsupported \n");
    }
    return;
}

/**
 * @addtogroup MEM_TEST
 * @{
 *
 *
 * <I> Usage:
 *
 * @li -h        : Print Help
 * @li -d [cnt]  : KBP cascade device count: 1/2/3/4: default 1
 * @li -D [type] : KBP device type nla12k or op [default nla12k]
 * </I>
 *
 * @}
 */



static void print_usage(char *progname)
{
    kbp_printf("Release: %s\n", KBP_SDK_VERSION);
    kbp_printf("Usage: %s [options]\n"
           "  -h         Print this help, then exit\n"
           "  -d <cnt>   KBP cascade device count: 1/2/3/4: default 1 \n"
           "  -D <type>  KBP device type: nla12k for nla12k or op for Optimus Prime, Default is O3S \n",
               progname);
    exit(1);
}


struct demo_info {
    struct kbp_db *db;
    struct kbp_key *key;
    struct kbp_instruction *instruction;
    struct kbp_key *mkey;
};


kbp_status create_stack(struct demo_info *in, struct kbp_device *device) {
    KBP_TRY(kbp_db_init(device, KBP_DB_ACL, 1, 1000, &in->db));     /* Initialize database */
    KBP_TRY(kbp_key_init(device, &in->key));                            /* Initialize key memory */
    KBP_TRY(kbp_key_add_field(in->key, "DIP", 128, KBP_KEY_FIELD_PREFIX));
    KBP_TRY(kbp_db_set_key(in->db, in->key));
    KBP_TRY(kbp_instruction_init(device, 1, 0, &in->instruction));      /* init instruction */
    KBP_TRY(kbp_key_init(device, &in->mkey));                           /* create the master key */
    KBP_TRY(kbp_key_add_field(in->mkey, "DIP", 128, KBP_KEY_FIELD_PREFIX));
    KBP_TRY(kbp_instruction_set_key(in->instruction, in->mkey));
    KBP_TRY(kbp_instruction_add_db(in->instruction, in->db, 0));
    KBP_TRY(kbp_instruction_install(in->instruction));
    KBP_TRY(kbp_device_lock(device));
    return KBP_OK;
}
/**
 * @addtogroup MEM_TEST
 * @{
 *
 * <B> Description:
 *
 * The following demo application demonstrates the memory testing for DBA and UDA.
 *
 * Demo will have DBA/UDA location which are to be injected with parity, do the parity scanning
 * read the entires and check the parity error occured, read FIFO's and get send them back for cross check.
 * This API wont do the error fixing or correction.
 *
 * The following steps are performed by using appropriate APIs </B>
 *
 * Note:
 *     Device Config Register[DCR]: The parity scan bit set, MDIO support for internal register write/read operations
 *
 * @}
 */


int main( int argc, char **argv ) {
    struct kbp_allocator *dalloc_p = NULL;
    void *xpt_p = NULL;
    struct kbp_device *device_p = NULL;
    kbp_status status = KBP_OK;
    uint32_t test_flag = 0, iter = 0;
    struct kbp_mem_test_demo handle;
    uint32_t dba_modified_addr[DEMO_MODIFIED_ADDR_LENGTH];
    uint32_t uda_modified_addr[DEMO_MODIFIED_ADDR_LENGTH];
    int32_t opt;
    enum kbp_device_type device_type = KBP_DEVICE_12K; /* Default is O3S */
    uint32_t inject_dba_error_addr[DEMO_INJECT_ERROR_ADDR_LENGTH] = { 0x01234, 0x12345, 0x23456, 0x34567, 0x45678, /* pass-1*/
                                                             0x56789, 0x6789A, 0x789AB, 0x89ABC, 0x9ABCD,
                                                             0xABCDE, 0xBCDEF, 0xCDEFE, 0xDEFED, 0xEFEDC,
                                                             0xFEDCB, 0xEDCBA, 0xDCBA9, 0xCBA98, 0xBA987, /* pass-2*/
                                                             0xA9876, 0x98765, 0x87654, 0x76543, 0x65432,
                                                             0x54321, 0x43210, 0x32101, 0x21012, 0x10123 };

    uint32_t inject_uda_error_addr[DEMO_INJECT_ERROR_ADDR_LENGTH] = { 0x34567, 0x01234, 0x23456, 0x12345};
    uint32_t num_devices = 1, flag = 0;
    struct demo_info in = {0, };

    kbp_printf("\n ---------------------------------------------------------");
    kbp_printf("\n     Referance Application to demonstrate the MEM_TEST    ");
    kbp_printf("\n     Release: %s            ", kbp_device_get_sdk_version());
    kbp_printf("\n ---------------------------------------------------------");
    kbp_printf("\n  Note: this demo can run only with mdio support and real kbp device\n\n");
    kbp_printf("  - Command Line: mem_test_demo\n");

#ifdef USE_OP_PCIE_XPT
    device_type = KBP_DEVICE_OP; /* Default is OP */
#endif


/**
* @addtogroup MEM_TEST
* @{
* @li Parse command line arguments
* @}
*/

    /* parse command-line arguments */
        while ((opt = getopt(argc, argv, "hd:D:")) != -1) {
            switch (opt) {
             case 'd':
                    num_devices = atoi(optarg);

                    if(num_devices < 1 || num_devices > 4) {
                        kbp_printf("MEM Test not supported more than 4 devices (1 to 4)\n", optarg);
                        exit(1);
                    }
                    break;
             case 'D':
                if (strcmp(optarg, "nla12k") == 0) {
                    device_type = KBP_DEVICE_12K;
                } else if (strcmp(optarg, "op") == 0) {
                    device_type = KBP_DEVICE_OP;
                }  else {
                    kbp_printf("Unsupported device type: %s [nla12k/op]\n", optarg);
                    print_usage(argv[0]);
                    exit(1);
                }
                break;
            case 'h':
            default:
                print_usage(argv[0]);
                exit(1);
            }
        }

    kbp_memset( &handle, 0, sizeof(struct kbp_mem_test_demo) );

   /**
     * @addtogroup MEM_TEST
     * @{
     * @li Create default allocator
     * @}
     */

    kbp_printf("\n-> Create default allocator");
    KBP_TRY(default_allocator_create(&dalloc_p));

    /* Print witch device type in use */
    device_type_print(device_type);

    flag = KBP_DEVICE_DEFAULT;

    if(num_devices > 1)
        flag |= KBP_DEVICE_CASCADE;

    /**
     * @addtogroup MEM_TEST
     * @{
     * @li Device initialization [will work with cascade of devices too KBP_DEVICE_CASCADE]
     * @}
     */

#ifdef USE_OP_PCIE_XPT
    {
        if (device_type != KBP_DEVICE_OP) {
            kbp_printf("  Unsupported device type\n");
            exit(1);
        }

        status = kbp_pcie_init(KBP_DEVICE_OP, KBP_DEVICE_DEFAULT, 0,
                               dalloc_p, NULL, NULL, &xpt_p);
        if (status != KBP_OK) {
            kbp_printf("\nFailed to initialize XPT: %s\n", kbp_get_status_string(status));
            exit(1);
        }
        KBP_TRY(kbp_pcie_soft_reset(xpt_p));
    }
#endif

    kbp_printf("\n-> Create device instance/s");
    KBP_TRY(kbp_device_init(dalloc_p, device_type, flag, xpt_p, NULL, &device_p));

    /**
     * @addtogroup MEM_TEST
     * @{
     * @li Check the DCR (parity scan is set), enable interrupt by prop [KBP_DEVICE_PROP_INTERRUPT], latency KBP_DEVICE_PROP_INST_LATENCY [8 for block opers, else 1] for O3S only
     * @}
     */

    /* Enable the supported device interrupts; This prop enables the DBA intruptes
           Interrupt error mask register:   b'1 dbSoftErr = 1 ,  b'0  gioL1Enable = 1
       */
    if (device_type == KBP_DEVICE_12K) {
        KBP_TRY(kbp_device_set_property(device_p, KBP_DEVICE_PROP_INTERRUPT, 1));
        KBP_TRY(kbp_device_set_property(device_p, KBP_DEVICE_PROP_INST_LATENCY, 1)); /* set to 8 if block operations supported */
    } else {
        KBP_TRY(kbp_device_set_property(device_p, 682, 1)); /* FW load*/
    }
    /**
     * @addtogroup MEM_TEST
     * @{
     * @li Fill out proper MDIO read/write, usleep, scan duration and handles
     * @}
     */

    KBP_STRY(create_stack(&in, device_p));

    handle.device = device_p;
    handle.info.handle = &handle;
    handle.info.debug_prints = 1; /* enable to see the debug prints */
    handle.info.test_duration_in_usec =  (150* 1000);
    handle.info.kbp_mem_test_usleep = mem_test_demo_usleep;
    handle.info.kbp_mdio_register_write_80 = mem_test_demo_mdio_reg_write_80;
    handle.info.kbp_mdio_register_read_80  = mem_test_demo_mdio_reg_read_80;
    handle.info.modified_addr_length       = DEMO_MODIFIED_ADDR_LENGTH;
    handle.info.kbp_mdio_register_write_64 = mem_test_demo_mdio_reg_write_64;
    handle.info.kbp_mdio_register_read_64  = mem_test_demo_mdio_reg_read_64;


    /**
     * @addtogroup MEM_TEST
     * @{
     * @li UDA memory test fill out details: error inject locations, num locations, handles e.t.c
     * @}
     */

    /* error injection, for development purpose only */
    handle.info.modified_addr_count = 0;
    handle.info.modified_addr = uda_modified_addr;
    handle.info.inject_error_addr_length = 4;
    handle.info.inject_error_addr = inject_uda_error_addr;
    /* distribute among devices */
    {
        for(iter =0; iter < 4; iter++)
            inject_uda_error_addr[iter] = (inject_uda_error_addr[iter] | ( (iter%num_devices) << 23) );
    }

    /**
     * @addtogroup MEM_TEST
     * @{
     * @li UDA memory test [this will inject errors on specified locations and scan, and report by reading FIFO], cross check with expected
     * @}
     */

    /**
     * @addtogroup MEM_TEST
     * @{
     * @li UDA memory test done by [kbp_mem_test_uda()]
     * @}
     */

    /* API to get the parity error locations() */
    kbp_printf("\n\n-> UDA memory test");
    status = kbp_mem_test_uda( device_p, &handle.info);
    if(status != KBP_OK) {
        if(status != KBP_UNSUPPORTED)
            test_flag = status;
    }

    kbp_printf( "\n\n-> Memory test result: %s", kbp_get_status_string(status));

    kbp_printf("\nSummary:\n");
    kbp_printf("Number of error injected: %u\n", handle.info.inject_error_addr_length);
    kbp_printf("Number of error reported: %u\n", handle.info.modified_addr_count);
    kbp_printf("Parity locations:\n");
    for (iter = 0; iter < handle.info.modified_addr_count; iter++) {
        if (iter && (iter%5 == 0))
            kbp_printf("\n");
        kbp_printf( "  %.6X,", uda_modified_addr[iter]);
    }

    /**
     * @addtogroup MEM_TEST
     * @{
     * @li DBA memory test fill out details: error inject locations, num locations, handles e.t.c
     * @}
     */

    handle.info.inject_error_addr = inject_dba_error_addr;
    handle.info.modified_addr_count = 0;
    handle.info.modified_addr = dba_modified_addr;
    handle.info.inject_error_addr_length = DEMO_INJECT_ERROR_ADDR_LENGTH;
    /* distribute among devices */
    {
        for(iter =0; iter < DEMO_INJECT_ERROR_ADDR_LENGTH; iter++)
            inject_dba_error_addr[iter] = (inject_dba_error_addr[iter] | ( (iter%num_devices) << 23) );
    }

    /**
     * @addtogroup MEM_TEST
     * @{
     * @li DBA memory test [this will inject errors on specified locations and scan, and report by reading FIFO], cross check with expected
     * @}
     */

    /**
     * @addtogroup MEM_TEST
     * @{
     * @li DBA memory test done by [kbp_mem_test_dba()]
     * @}
     */

    /* API to get the parity error locations() */
    kbp_printf("\n\n\n-> DBA memory test");
    status = kbp_mem_test_dba( device_p, &handle.info);
    if(status != KBP_OK)
        test_flag = status;

    kbp_printf( "\n\n-> Memory test result: %s", kbp_get_status_string(status));

    kbp_printf("\nSummary:\n");
    kbp_printf("Number of error injected: %u\n", handle.info.inject_error_addr_length);
    kbp_printf("Number of error reported: %u\n", handle.info.modified_addr_count);
    kbp_printf("Parity locations:\n");
    for (iter = 0; iter < handle.info.modified_addr_count; iter++) {
        if (iter && (iter%5 == 0))
            kbp_printf("\n");
        kbp_printf( "  %.6X,", dba_modified_addr[iter]);
    }

    /**
     * @addtogroup MEM_TEST
     * @{
     * @li Destroy the device handle
     * @}
     */

    kbp_printf("\n-> Destroy the device instance");
    KBP_TRY(kbp_device_destroy(device_p));


#ifdef USE_OP_PCIE_XPT
    KBP_TRY(kbp_pcie_destroy(dalloc_p, xpt_p));
#endif

    /**
     * @addtogroup MEM_TEST
     * @{
     * @li Destroy the default allocator
     * @}
     */

    /* destroy the default allocator */
    kbp_printf("\n-> Destroy the default allocator");
    KBP_TRY(default_allocator_destroy(dalloc_p));

    kbp_printf("\n ----------------------------------------------------------------------------------");
    kbp_printf("\n     Referance Application to demonstrate the MEM_BIST done");
    kbp_printf("\n ----------------------------------------------------------------------------------");

    if (test_flag) {
        kbp_printf("\n --> Overall Result: FAILED <--\n");
    } else {
        kbp_printf("\n --> Overall Result: PASSED <--\n");
    }

    return KBP_OK;
}

