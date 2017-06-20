/*
 * ! \file cmodel_reg_access.c This file presents an example of replacing the registers\memories access functions. It
 * assumes using the portmod register DB bcm2801pb_a0
 */
/*
 * $Id:$
 $Copyright: (c) 2016 Broadcom.
 Broadcom Proprietary and Confidential. All rights reserved.$ $
 */

int
tmp_workaround_func(
    void)
{
    return 0;
}
#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SOCDNX_CMODELDNX

#ifdef CMODEL_SERVER_MODE
/*
 * {
 */

#ifndef VXWORKS
/*
 * {
 */
#include <netdb.h>
/*
 * }
 */
#endif
/*
 * Include files.
 * {
 */
#include <shared/bsl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <bde/pli/verinet.h>
#include <errno.h>
#include <sys/socket.h>
#include <soc/error.h>
#include <soc/drv.h>
#include <shared/shrextend/shrextend_debug.h>


#include <soc/dcmn/error.h>

#include <netinet/tcp.h>
#include <soc/dnx/cmodel/cmodel_reg_access.h>
/*
 * }
 */

/*
 * DEFINEs
 * {
 */
/** Default cmodel server port */
#define DEFAULT_CMODEL_SERVER_PORT 6816
/** Default cmodel server address */
#define DEFAULT_CMODEL_SERVER_ADDRESS "localhost"
/** Maximum host address length */
#define HOST_ADDRESS_MAX_LEN 40
/** Maximum packet header size in the cmodel */
#define MAX_PACKET_HEADER_SIZE_CMODEL 12
/** Memory/Registers response header size in cmodel */
#define MEM_RESPONSE_HEADER_SIZE_CMODEL 10
/** Signal response header size in cmodel */
#define SIGNALS_RESPONSE_LENGTH_SIZE 4
/** Constant header size for cmodel signals */
#define CMODEL_SIGNALS_CONSTANT_HEADER_SIZE 9
/** Constant header size for cmodel block names */
#define CMODEL_BLOCK_NAMES_CONSTANT_HEADER_SIZE 5
/** Minimum signals data size */
#define MIN_SIGNALS_DATA_SIZE 9
/** Maximum signals data size */
#define MAX_SIGNALS_DATA_SIZE 8192
/** Signals request header size */
#define SIGNALS_REQUEST_HEADER_SIZE 5
/** Signals all MS IDs */
#define SIGNALS_ALL_MS_ID -1
/** Signal ID for packet TX */
#define TX_PACKET_SIGNAL 3
/** RX thread is about to be closed */
#define RX_THREAD_NOTIFY_CLOSED (-2)

/** Opcode values taken from the C model's server */
/** Opcode value for write register */
#define UPDATE_FRM_SOC_WRITE_REG 128
/** Opcode value for write table */
#define UPDATE_FRM_SOC_WRITE_TBL 129

/** Opcode value for read register request */
#define UPDATE_FRM_SOC_READ_REG_REQ 133
/** Opcode value for read table request */
#define UPDATE_FRM_SOC_READ_TBL_REQ 134
/** Opcode value for read register reply */
#define UPDATE_TO_SOC_READ_REG_REP 8
/** Opcode value for read table reply */
#define UPDATE_TO_SOC_READ_TBL_REP 9

/*
 * }
 */
const char *conversion_tbl[] = {
    "00000000",
    "00000001",
    "00000010",
    "00000011",
    "00000100",
    "00000101",
    "00000110",
    "00000111",
    "00001000",
    "00001001",
    "00001010",
    "00001011",
    "00001100",
    "00001101",
    "00001110",
    "00001111",
    "00010000",
    "00010001",
    "00010010",
    "00010011",
    "00010100",
    "00010101",
    "00010110",
    "00010111",
    "00011000",
    "00011001",
    "00011010",
    "00011011",
    "00011100",
    "00011101",
    "00011110",
    "00011111",
    "00100000",
    "00100001",
    "00100010",
    "00100011",
    "00100100",
    "00100101",
    "00100110",
    "00100111",
    "00101000",
    "00101001",
    "00101010",
    "00101011",
    "00101100",
    "00101101",
    "00101110",
    "00101111",
    "00110000",
    "00110001",
    "00110010",
    "00110011",
    "00110100",
    "00110101",
    "00110110",
    "00110111",
    "00111000",
    "00111001",
    "00111010",
    "00111011",
    "00111100",
    "00111101",
    "00111110",
    "00111111",
    "01000000",
    "01000001",
    "01000010",
    "01000011",
    "01000100",
    "01000101",
    "01000110",
    "01000111",
    "01001000",
    "01001001",
    "01001010",
    "01001011",
    "01001100",
    "01001101",
    "01001110",
    "01001111",
    "01010000",
    "01010001",
    "01010010",
    "01010011",
    "01010100",
    "01010101",
    "01010110",
    "01010111",
    "01011000",
    "01011001",
    "01011010",
    "01011011",
    "01011100",
    "01011101",
    "01011110",
    "01011111",
    "01100000",
    "01100001",
    "01100010",
    "01100011",
    "01100100",
    "01100101",
    "01100110",
    "01100111",
    "01101000",
    "01101001",
    "01101010",
    "01101011",
    "01101100",
    "01101101",
    "01101110",
    "01101111",
    "01110000",
    "01110001",
    "01110010",
    "01110011",
    "01110100",
    "01110101",
    "01110110",
    "01110111",
    "01111000",
    "01111001",
    "01111010",
    "01111011",
    "01111100",
    "01111101",
    "01111110",
    "01111111",
    "10000000",
    "10000001",
    "10000010",
    "10000011",
    "10000100",
    "10000101",
    "10000110",
    "10000111",
    "10001000",
    "10001001",
    "10001010",
    "10001011",
    "10001100",
    "10001101",
    "10001110",
    "10001111",
    "10010000",
    "10010001",
    "10010010",
    "10010011",
    "10010100",
    "10010101",
    "10010110",
    "10010111",
    "10011000",
    "10011001",
    "10011010",
    "10011011",
    "10011100",
    "10011101",
    "10011110",
    "10011111",
    "10100000",
    "10100001",
    "10100010",
    "10100011",
    "10100100",
    "10100101",
    "10100110",
    "10100111",
    "10101000",
    "10101001",
    "10101010",
    "10101011",
    "10101100",
    "10101101",
    "10101110",
    "10101111",
    "10110000",
    "10110001",
    "10110010",
    "10110011",
    "10110100",
    "10110101",
    "10110110",
    "10110111",
    "10111000",
    "10111001",
    "10111010",
    "10111011",
    "10111100",
    "10111101",
    "10111110",
    "10111111",
    "11000000",
    "11000001",
    "11000010",
    "11000011",
    "11000100",
    "11000101",
    "11000110",
    "11000111",
    "11001000",
    "11001001",
    "11001010",
    "11001011",
    "11001100",
    "11001101",
    "11001110",
    "11001111",
    "11010000",
    "11010001",
    "11010010",
    "11010011",
    "11010100",
    "11010101",
    "11010110",
    "11010111",
    "11011000",
    "11011001",
    "11011010",
    "11011011",
    "11011100",
    "11011101",
    "11011110",
    "11011111",
    "11100000",
    "11100001",
    "11100010",
    "11100011",
    "11100100",
    "11100101",
    "11100110",
    "11100111",
    "11101000",
    "11101001",
    "11101010",
    "11101011",
    "11101100",
    "11101101",
    "11101110",
    "11101111",
    "11110000",
    "11110001",
    "11110010",
    "11110011",
    "11110100",
    "11110101",
    "11110110",
    "11110111",
    "11111000",
    "11111001",
    "11111010",
    "11111011",
    "11111100",
    "11111101",
    "11111110",
    "11111111"
};

typedef enum
{
    CMODEL_RX_TX,
    CMODEL_REGS,
    CMODEL_SIGNALS
} socket_target_e;

typedef enum
{
    CMODEL_SIGNALS_REQUEST_OPCODE,
    CMODEL_SIGNALS_RESPONSE_OPCODE,
    CMODEL_BLOCK_NAMES_REQUEST_OPCODE,
    CMODEL_BLOCK_NAMES_RESPONSE_OPCODE
} cmodel_opcode_e;

/**
 * \brief
 *  Context data of the C model
 */
typedef struct
{

    /** Params for sending and receiving packets and signals */
    int cmodel_rx_tx_fd;
    sal_mutex_t cmodel_rx_tx_mutex;

    /** Params for accessing the registers and memories */
    int cmodel_mem_reg_fd;
    sal_mutex_t cmodel_mem_reg_mutex;

    /** Params for accessing the signals */
    int cmodel_sdk_interface_fd;
    sal_mutex_t cmodel_sdk_interface_mutex;

    /** Rx thread handle */
    sal_thread_t rx_tid;

} cmodel_access_info_t;

/**
 * \brief - context data of the C model
 *
 */
cmodel_access_info_t *cmodel_info = { 0 };

/**
 * \brief - Register get 32 bits
 *
 * DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] reg - register ID
 *   \param [in] port - in port
 *   \param [in] index - index of the reg/memory in the arrayindex of the reg/memory in the arrayindex of the reg/memory in the array
 *   \param [in] data - value of the register/memory value of the register/memory
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int cmodel_reg32_get(
    int unit,
    soc_reg_t reg,
    int port,
    int index,
    uint32 * data);

/**
 * \brief - Register get 64 bits
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] reg - register ID
 *   \param [in] port - in port
 *   \param [in] index - index of the reg/memory in the arrayindex of the reg/memory in the arrayindex of the reg/memory in the array
 *   \param [in] data - value of the register/memory value of the register/memory
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int cmodel_reg64_get(
    int unit,
    soc_reg_t reg,
    int port,
    int index,
    uint64 * data);

/**
 * \brief - Register get above 64 bits
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] reg - register ID
 *   \param [in] port - in port
 *   \param [in] index - index of the reg/memory in the array
 *   \param [in] data - value of the register/memory
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int cmodel_reg_above64_get(
    int unit,
    soc_reg_t reg,
    int port,
    int index,
    soc_reg_above_64_val_t data);

/**
 * \brief - Register set 32 bits
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] reg - register ID
 *   \param [in] port - in port
 *   \param [in] index - index of the reg/memory in the array
 *   \param [in] data - value of the register/memory
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int cmodel_reg32_set(
    int unit,
    soc_reg_t reg,
    int port,
    int index,
    uint32 data);

/**
 * \brief - Register set 64 bits
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] reg - register ID
 *   \param [in] port - in port
 *   \param [in] index - index of the reg/memory in the array
 *   \param [in] data - value of the register/memory
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int cmodel_reg64_set(
    int unit,
    soc_reg_t reg,
    int port,
    int index,
    uint64 data);

/**
 * \brief - Register set above 64 bits
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] reg - register ID
 *   \param [in] port - in port
 *   \param [in] index - index of the reg/memory in the array
 *   \param [in] data - value of the register/memory
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int cmodel_reg_above64_set(
    int unit,
    soc_reg_t reg,
    int port,
    int index,
    soc_reg_above_64_val_t data);

/**
 * \brief - Read the data of a memory array
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] mem - memory ID
 *   \param [in] array_index - index of the memory in the array
 *   \param [in] copyno - core ID
 *   \param [in] index - index of the reg/memory in the array
 *   \param [in] entry_data - data of the memory's entry
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int cmodel_mem_array_read(
    int unit,
    soc_mem_t mem,
    unsigned int array_index,
    int copyno,
    int index,
    void *entry_data);

/**
 * \brief - Write data to a memory array
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] mem - memory ID
 *   \param [in] array_index - index of the memory in the array
 *   \param [in] copyno - core ID
 *   \param [in] index - index of the reg/memory in the array
 *   \param [in] entry_data - data of the memory's entry
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int cmodel_mem_array_write(
    int unit,
    soc_mem_t mem,
    unsigned int array_index,
    int copyno,
    int index,
    void *entry_data);

/**
 * \brief - Callbacks that support C model. Registers using
 * soc_reg_access_func_register
 */
soc_reg_access_t cmodel_access = {
    cmodel_reg32_get,
    cmodel_reg64_get,
    cmodel_reg_above64_get,

    cmodel_reg32_set,
    cmodel_reg64_set,
    cmodel_reg_above64_set,

    cmodel_mem_array_read,
    cmodel_mem_array_write
};

extern char *getenv(
    const char *);
extern int _soc_mem_write_copyno_update(
    int unit,
    soc_mem_t mem,
    int *copyno,
    int *copyno_override);

/**
 * \brief - Wait for the response from the C model's server and read it from the socket
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] is_mem - 0 for registers, 1 for memories
 *   \param [in] data_len - length (in bytes) of the data parameter
 *   \param [in] data - value of the register/memory
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   soc_error_t
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
static soc_error_t
cmodel_memreg_read_response(
    int unit,
    int is_mem,
    uint32 data_len,
    uint32 * data)
{
    fd_set read_vect;
    char swapped_header[MEM_RESPONSE_HEADER_SIZE_CMODEL];
    uint32 nw_order_int;
    uint32 rtl_address;
    int8 opcode, block_id;
    int read_length = 0;
    uint32 actual_data_len;
    uint32 half_actual_data_len;
    int ii;
    char *data_char_ptr = (char *) data;

    /**Used to swap the data bytes */
    int tmp;

    SHR_FUNC_INIT_VARS(unit);

    FD_ZERO(&read_vect);
    FD_SET(cmodel_info->cmodel_mem_reg_fd, &read_vect);

    while (1)
    {
        if (select(cmodel_info->cmodel_mem_reg_fd + 1, &read_vect, (fd_set *) 0x0, (fd_set *) 0x0, NULL) < 0)
        {
            if (errno == EINTR)
            {
                /*
                 * Interrupted by a signal such as a GPROF alarm so
                 * restart the call
                 */
                continue;
            }
            perror("get_command: select error");
            SHR_SET_CURRENT_ERR(_SHR_E_FAIL);
            SHR_EXIT();
        }
        else
        {
            break;
        }
    }

    /*
     * Data ready to be read.
     */
    if (FD_ISSET(cmodel_info->cmodel_mem_reg_fd, &read_vect))
    {
        read_length = readn(cmodel_info->cmodel_mem_reg_fd, &(swapped_header[0]), MEM_RESPONSE_HEADER_SIZE_CMODEL);

        /*
         * Read the length of the packet
         */
        if (read_length < MEM_RESPONSE_HEADER_SIZE_CMODEL)
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "cmodel_read_buffer: could not read packet length\n")));
            SHR_SET_CURRENT_ERR(_SHR_E_FAIL);
            SHR_EXIT();
        }

        /*
         * Read opcode
         */
        opcode = *((int8 *) & (swapped_header[0]));

        /*
         * Read length
         */
        sal_memcpy(&nw_order_int, &swapped_header[1], sizeof(int));
        /*
         * actual_data_len is only the data length without the block id field and the RTL address field
         */
        actual_data_len = ntohl(nw_order_int) - 5;

        /*
         * Read block_id
         */
        block_id = *((int8 *) & (swapped_header[5]));

        /*
         * Read RTL address
         */
        sal_memcpy(&nw_order_int, &swapped_header[6], sizeof(int));
        rtl_address = ntohl(nw_order_int);

        LOG_INFO(BSL_LS_SYS_VERINET,
                 (BSL_META("cmodel_memreg_read_response: opcode=%d actual_data_len=%d block_id=%d rtl_address=%d\n"),
                  opcode, actual_data_len, block_id, rtl_address));

        read_length = readn(cmodel_info->cmodel_mem_reg_fd, data, actual_data_len);

        /*
         * Read the data field according to the length
         */
        
        if (read_length < actual_data_len)
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "cmodel_memreg_read_response: could not read data\n")));
            SHR_SET_CURRENT_ERR(_SHR_E_FAIL);
            SHR_EXIT();
        }

        /*
         * The bytes should be swapped. The bytes are writen swapped and should be swapped again on the way from the c model's server
         */
        half_actual_data_len = actual_data_len >> 1;
        for (ii = 0; ii < half_actual_data_len; ii++)
        {
            tmp = data_char_ptr[ii];
            data_char_ptr[ii] = data_char_ptr[actual_data_len - 1 - ii];
            data_char_ptr[actual_data_len - 1 - ii] = tmp;
        }

    }
    else
    {
        SHR_SET_CURRENT_ERR(_SHR_E_TIMEOUT);
        /*
         * Time expire with no command
         */
        SHR_EXIT();
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Reads the signal response from the cmodel server
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit id
 *   \param [in] signal_data - Allocated buffer for cmodel signal data
 *
 * Packet response format:
 * length in bytes from opcode (32 bits) | opcode (1 byte) | MS_ID (32 bits) | nof_signals (32 bits) |
 * (signal_name_length (32 bits) | signal_name | signal_data_length in bytes (32 bits) | signal_data)*
 *
 * signal_data buffer includes:
 * nof_signals (32 bits) | (signal_name_length (32 bits) | signal_name | signal_data_length in bytes (32 bits) | signal_data)*
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   \param [out] signal_data_length - Number of bytes in signal_buffer
 *   \param [out] signal_data - Allocated buffer for cmodel signal data that should be freed by the user.
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
cmodel_signals_read_response(
    int unit,
    uint32 *signal_data_length,
    char **signal_data)
{
    fd_set read_vect;
    int packet_size;
    int8 opcode;
    uint32 ms_id;
    int read_constant_header = 0;
    char buffer[CMODEL_SIGNALS_CONSTANT_HEADER_SIZE];
    char *signal_data_dyn;
    int buffer_offset = 0;
    uint32 long_val;

    SHR_FUNC_INIT_VARS(unit);

    FD_ZERO(&read_vect);
    FD_SET(cmodel_info->cmodel_sdk_interface_fd, &read_vect);

    while (1)
    {
        if (select(cmodel_info->cmodel_sdk_interface_fd + 1, &read_vect, (fd_set *) 0x0, (fd_set *) 0x0, NULL) < 0)
        {
            if (errno == EINTR)
            {
                /*
                 * Interrupted by a signal such as a GPROF alarm so restart the call
                 */
                continue;
            }
            perror("get_command: select error");
            SHR_SET_CURRENT_ERR(_SHR_E_FAIL);
            SHR_EXIT();
        }
        else
        {
            break;
        }
    }

    /*
     * Data ready to be read.
     */
    if (FD_ISSET(cmodel_info->cmodel_sdk_interface_fd, &read_vect))
    {
        /*
         * Read the constant header of the packet
         */
        read_constant_header = readn(cmodel_info->cmodel_sdk_interface_fd, buffer, CMODEL_SIGNALS_CONSTANT_HEADER_SIZE);

        if (read_constant_header < CMODEL_SIGNALS_CONSTANT_HEADER_SIZE)
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "cmodel_read_buffer: could not read the constant header \n")));
            SHR_SET_CURRENT_ERR(_SHR_E_FAIL);
            SHR_EXIT();
        }

        /** Read packet size */
        long_val = ntohl(*(uint32 *) & (buffer[buffer_offset]));
        packet_size = long_val;
        buffer_offset += sizeof(packet_size);

        if ((packet_size < MIN_SIGNALS_DATA_SIZE) || (packet_size > MAX_SIGNALS_DATA_SIZE))
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Cmodel signal invalid packet size\n")));
            SHR_SET_CURRENT_ERR(_SHR_E_FAIL);
            SHR_EXIT();
        }

        /** Read opcode */
        opcode = buffer[buffer_offset];
        buffer_offset += sizeof(opcode);

        /** Check opcode */
        if (opcode != CMODEL_SIGNALS_RESPONSE_OPCODE)
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Wrong opcode in signals response message\n")));
            SHR_SET_CURRENT_ERR(_SHR_E_FAIL);
            SHR_EXIT();
        }

        /** Read ms_id */
        long_val = ntohl(*(uint32 *) & (buffer[buffer_offset]));
        ms_id = long_val;
        buffer_offset += sizeof(ms_id);

        /*
         * The signal_data_length includes nof_signals (32 bits) | (signal_name_length (32 bits) | signal_name |
         * signal_data_length in bytes (32 bits) | signal_data)*
         */
        *signal_data_length = packet_size - buffer_offset + 4;
        signal_data_dyn = sal_alloc(*signal_data_length, "cmodel signals buffer");
        readn(cmodel_info->cmodel_sdk_interface_fd, signal_data_dyn, *signal_data_length);
        *signal_data = signal_data_dyn;

    }
    else
    {
        SHR_SET_CURRENT_ERR(_SHR_E_TIMEOUT);
        /*
         * Time expire with no command
         */
        SHR_EXIT();
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Get signals from the cmodel server
 *
 * Packet request format: | opcode (1 byte) | MS_ID (32 bits)
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit id
 *   \param [in] ms_id - Block id
 *   \param [in] signal_data - Dynamically allocated buffer inside the cmodel_get_signal for cmodel signal data. It should be
 *          freed using sal_free().
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   \param [out] signal_data_length - Number of bytes in signal_buffer
 *   \param [out] signal_data - Dynamically allocated buffer for cmodel signal data
 *   signal_data format: nof_signals (32 bits) | (signal_name_length (32 bits) | signal_name | signal_data_length in
 *   bytes (32 bits) | signal_data)*
 *   All the numbers in signal_data arrive in NW order and should be converted using ntohl.
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
cmodel_get_signal(
    int unit,
    uint32 ms_id,
    uint32 * signal_data_length,
    char **signal_data)
{
    uint32 nw_order_int;
    int write_index = 0;
    uint32 host_order_int;
    char buffer[SIGNALS_REQUEST_HEADER_SIZE];

    SHR_FUNC_INIT_VARS(unit);

    LOG_INFO(BSL_LS_SYS_VERINET, (BSL_META("cmodel_signals_write_request: unit=%d ms_id=%d \n"), unit, ms_id));

    sal_mutex_take(cmodel_info->cmodel_sdk_interface_mutex, sal_mutex_FOREVER);

    /** Add opcode (1B) to the buffer */
    buffer[write_index] = CMODEL_SIGNALS_REQUEST_OPCODE;
    write_index += sizeof(char);

    /** Add MS ID (4 B) */
    host_order_int = ms_id;
    nw_order_int = ntohl(host_order_int);
    sal_memcpy(&(buffer[write_index]), &nw_order_int, sizeof(ms_id));
    write_index += sizeof(ms_id);

    /*
     * write the packet to the socket
     */
    if (writen(cmodel_info->cmodel_sdk_interface_fd, buffer, write_index) != write_index)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Error: cmodel_signals_write_request data failed\n")));
        SHR_SET_CURRENT_ERR(_SHR_E_FAIL);
        SHR_EXIT();
    }
    /** Fetch the signals from the C model */
    cmodel_signals_read_response(unit, signal_data_length, signal_data);

exit:
    sal_mutex_give(cmodel_info->cmodel_sdk_interface_mutex);
    SHR_FUNC_EXIT;
}

/**
 * \brief - Read the block response from the cmodel server
 *
 * Packet response format:
 * length in bytes from opcode (32 bits) | opcode (1 byte) | nof_blocks (32 bits)| (MS_ID (32 bits) |
 * ms_name_length | ms_name)*
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit id
 *   \param [in] block_names - Dynamically allocated buffer for cmodel block names and freed by the caller.
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   \param [out] block_names_length - Returned cmodel block names length
 *   \param [out] block_names - Returned cmodel block names
 * block_names format: nof_blocks (32 bits)| (MS_ID (32 bits) | ms_name_length (32 bits) | ms_name)*
 * All the numbers in block_names arrive in NW order and should be converted using ntohl.
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
cmodel_blocks_read_response(
    int unit,
    int *block_names_length,
    char **block_names)
{
    fd_set read_vect;
    int packet_size;
    int8 opcode;
    uint32 long_val;
    int buffer_offset = 0;
    char buffer[CMODEL_BLOCK_NAMES_CONSTANT_HEADER_SIZE];
    char *block_names_dyn;
    int read_constant_header = 0;

    SHR_FUNC_INIT_VARS(unit);

    FD_ZERO(&read_vect);
    FD_SET(cmodel_info->cmodel_sdk_interface_fd, &read_vect);

    while (1)
    {
        if (select(cmodel_info->cmodel_sdk_interface_fd + 1, &read_vect, (fd_set *) 0x0, (fd_set *) 0x0, NULL) < 0)
        {
            if (errno == EINTR)
            {
                /*
                 * Interrupted by a signal such as a GPROF alarm so restart the call
                 */
                continue;
            }
            perror("get_command: select error");
            SHR_SET_CURRENT_ERR(_SHR_E_FAIL);
            SHR_EXIT();
        }
        else
        {
            break;
        }
    }

    /*
     * Data ready to be read.
     */
    if (FD_ISSET(cmodel_info->cmodel_sdk_interface_fd, &read_vect))
    {
        read_constant_header =
            readn(cmodel_info->cmodel_sdk_interface_fd, buffer, CMODEL_BLOCK_NAMES_CONSTANT_HEADER_SIZE);
        /** Read the length of the packet */
        if (read_constant_header < CMODEL_BLOCK_NAMES_CONSTANT_HEADER_SIZE)
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "cmodel_read_buffer: could not read the constant header \n")));
            SHR_SET_CURRENT_ERR(_SHR_E_FAIL);
            SHR_EXIT();
        }

        /** Read packet_size */
        long_val = ntohl(*(uint32 *) & (buffer[buffer_offset]));
        packet_size = long_val;
        buffer_offset += sizeof(packet_size);

        if ((packet_size <= MIN_SIGNALS_DATA_SIZE) || (packet_size > MAX_SIGNALS_DATA_SIZE))
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Cmodel signal invalid packet size\n")));
            SHR_SET_CURRENT_ERR(_SHR_E_FAIL);
            SHR_EXIT();
        }

        /** Read opcode */
        opcode = buffer[buffer_offset];
        buffer_offset += sizeof(opcode);

        if (opcode != CMODEL_BLOCK_NAMES_RESPONSE_OPCODE)
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Cmodel block_names wrong opcode \n")));
            SHR_SET_CURRENT_ERR(_SHR_E_FAIL);
            SHR_EXIT();
        }

        /** block_names_length includes nof_blocks (32 bits)| (MS_ID (32 bits) | ms_name_length (32 bits) | ms_name)* */
        *block_names_length = packet_size - buffer_offset + 4;
        block_names_dyn = sal_alloc(*block_names_length, "cmodel block names buffer");
        readn(cmodel_info->cmodel_sdk_interface_fd, block_names_dyn, *block_names_length);
        *block_names = block_names_dyn;
    }
    else
    {
        SHR_SET_CURRENT_ERR(_SHR_E_TIMEOUT);
        /*
         * Time expire with no command
         */
        SHR_EXIT();
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Cmodel get block names
 *
 * Packet request format: opcode (1B)
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit id
 *   \param [in] block_names_length -  Length of the cmodel block names
 *   \param [in] block_names - Dynamicallly allocated buffer inside cmodel_get_block_names for cmodel block names. It should be
 *          freed using sal_free().
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
cmodel_get_block_names(
    int unit,
    int *block_names_length,
    char **block_names)
{
    int write_index = 0;
    char buffer[SIGNALS_REQUEST_HEADER_SIZE];

    SHR_FUNC_INIT_VARS(unit);

    sal_mutex_take(cmodel_info->cmodel_sdk_interface_mutex, sal_mutex_FOREVER);

    /** Add opcode (1B) to the buffer */
    buffer[write_index] = CMODEL_BLOCK_NAMES_REQUEST_OPCODE;
    write_index += sizeof(char);

    /** write the packet to the socket */
    if (writen(cmodel_info->cmodel_sdk_interface_fd, buffer, write_index) != write_index)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Error: cmodel_get_block_names writing the opcode failed\n")));
        SHR_SET_CURRENT_ERR(_SHR_E_FAIL);
        SHR_EXIT();
    }
    /** Read the block names from the C model */
    cmodel_blocks_read_response(unit, block_names_length, block_names);

exit:
    sal_mutex_give(cmodel_info->cmodel_sdk_interface_mutex);
    SHR_FUNC_EXIT;
}

/**
 * \brief - Send a command to the C model. This function serves
 * read/write and memories/registers
 *
 * data_length - data length in bytes
 *
 * The message's format to receive data from the C model's server is:
 * GET:
 * Opcode (1 byte) | Data Length (4 bytes) | Block ID (1 byte) | RTL Address (4 bytes)
 * GET reply
 * Opcode (1 byte) | Data Length (4 bytes) | Block ID (1 byte) |
 * RTL Address (4 bytes) | Data (variable length)
 *
 * SET:
 * Opcode (1 byte) | Data Length (4 bytes) | Block ID (1 byte) |
 * RTL Address (4 bytes) | Data (variable length)
 *
 * Data Length includes only the field after it.
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] cmic_block - block ID of the reg/memory
 *   \param [in] addr - addr of the reg/memory
 *   \param [in] data_length - length (in bytes) of the data param
 *   \param [in] is_mem - 0 for registers, 1 for memories
 *   \param [in] is_get - 0 for set, 1 for get
 *   \param [in] data - value of the register/memory
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
cmodel_memreg_access(
    int unit,
    int cmic_block,
    uint32 addr,
    uint32 data_length,
    int is_mem,
    int is_get,
    uint32 * data)
{
    uint32 nw_order_int;
    uint8 opcode;
    uint32 host_order_int;
    char packet_data[MAX_PACKET_SIZE_CMODEL];
    char *data_char_ptr;
    int write_index = 0;
    int data_len = 0;
    uint8 rounded_data_length = 0;
    uint8 padding_length = 0;
    /*
     * (padding_length + last_word_length) % 4 == 0. The 0 padding is added before the data of the last DWORD
     */
    uint8 last_dword_length = 0;
    int ii = 0;

    SHR_FUNC_INIT_VARS(unit);

    LOG_INFO(BSL_LS_SYS_VERINET,
             (BSL_META("cmodel_memreg_access: unit=%d cmic_block=%d addr=0x%x data_length=%d is_mem=%d is_get=%d\n"),
              unit, cmic_block, addr, data_length, is_mem, is_get));

    sal_mutex_take(cmodel_info->cmodel_mem_reg_mutex, sal_mutex_FOREVER);

    /*
     * Round up to the closest multiple of 4
     */
    rounded_data_length = ((uint8) data_length + 3) & ~0x3;

    /*
     * Add Opcode according to Set/Get and Table/Register
     */
    if (is_get)
    {
        if (is_mem)
        {
            opcode = UPDATE_FRM_SOC_READ_TBL_REQ;
        }
        else
        {
            opcode = UPDATE_FRM_SOC_READ_REG_REQ;
        }
    }
    else
    {
        /*
         * In case of writes to table or register the len should include the data length
         */
        data_len = data_length;
        padding_length = rounded_data_length - data_length;
        last_dword_length = (4 - padding_length) % 4;

        if (is_mem)
        {
            opcode = UPDATE_FRM_SOC_WRITE_TBL;
        }
        else
        {
            opcode = UPDATE_FRM_SOC_WRITE_REG;
        }
    }

    packet_data[write_index] = opcode;
    write_index += 1;

    /*
     * Add data length ( sizeof(Block ID) + sizeof(RTL Address) + sizeof(Data) )
     */
    host_order_int = 1 + 4 + data_len + padding_length;

    /*
     * Expected result length should be added to registers and not tables
     */
    if (is_get && !is_mem)
    {
        /*
         * Add sizeof(expected data field)
         */
        host_order_int++;
    }
    nw_order_int = ntohl(host_order_int);
    sal_memcpy(&(packet_data[write_index]), &nw_order_int, sizeof(int));
    write_index += sizeof(int);

    /*
     * Add Block ID (1 byte)
     */
    packet_data[write_index] = (uint8) cmic_block;
    write_index += 1;

    /*
     * Add RTL Address (4 bytes)
     */
    host_order_int = addr;
    nw_order_int = ntohl(host_order_int);
    sal_memcpy(&(packet_data[write_index]), &nw_order_int, sizeof(int));
    write_index += sizeof(int);

    /*
     * Expected result length should be added to registers and not tables
     */
    /*
     * When doing a get the packet should describe how many bytes are expected
     */
    if (is_get && !is_mem)
    {
        packet_data[write_index] = (uint8) rounded_data_length;
        write_index += 1;
    }

    /*
     * Add data for set command
     */
    if (!is_get)
    {
        data_char_ptr = (char *) data;

        /*
         * Make sure that the packet is not too long
         */
        if (MAX_PACKET_SIZE_CMODEL < rounded_data_length + write_index)
        {
            LOG_ERROR(BSL_LOG_MODULE,
                      (BSL_META_U(unit, "Error: cmodel_memreg_access packet too long rounded_data_length=%d\n"),
                       rounded_data_length));
            SHR_SET_CURRENT_ERR(_SHR_E_FAIL);
            SHR_EXIT();
        }

        /*
         * Memories/Registers are writen in a reverse order. The LSB is writen first since the padding is done on the MSB
         */
        for (ii = 0; ii < rounded_data_length; ii++)
        {
            packet_data[write_index + ii] = data_char_ptr[(rounded_data_length - 1) - ii];
        }
        write_index += rounded_data_length;
    }

    /*
     * write the packet to the socket
     */
    if (writen(cmodel_info->cmodel_mem_reg_fd, packet_data, write_index) != write_index)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Error: cmodel_memreg_access data failed\n")));
        SHR_SET_CURRENT_ERR(_SHR_E_FAIL);
        SHR_EXIT();
    }

    /*
     * In case of get memory or register wait for the value to return
     */
    if (is_get)
    {
        SHR_IF_ERR_EXIT(cmodel_memreg_read_response(unit, is_mem, rounded_data_length, data));
    }

exit:
    sal_mutex_give(cmodel_info->cmodel_mem_reg_mutex);
    SHR_FUNC_EXIT;
}

/**
 * \brief - Get register's size, address and block and then call
 * the access function that serves the memories and registers
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] reg - register ID
 *   \param [in] port - in port
 *   \param [in] index - index of the reg/memory in the array
 *   \param [in] is_get - 0 for set, 1 for get
 *   \param [in] data - value of the register/memory
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
static int
cmodel_reg_access_handle(
    int unit,
    soc_reg_t reg,
    int port,
    int index,
    int is_get,
    soc_reg_above_64_val_t data)
{
    uint32 addr;
    int reg_size;
    int blk_id;
    uint8 acc_type;
    int rv;

    /*
     * reg validity check
     */
    if (reg >= NUM_SOC_REG)
    {
        LOG_ERROR(BSL_LS_SOC_PHYMOD, (BSL_META_U(unit, "invalid register")));
        return SOC_E_INTERNAL;
    }

    addr = soc_reg_addr_get(unit, reg, port, index, SOC_REG_ADDR_OPTION_NONE, &blk_id, &acc_type);
    if (addr <= 0)
    {
        return _SHR_E_MEMORY;
    }

    reg_size = soc_reg_bytes(unit, reg);

    rv = cmodel_memreg_access(unit, blk_id, addr, reg_size, 0, is_get, (uint32 *) data);

    return rv;
}

/**
 * \brief - handler function for memory/register read/write
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] mem - memory ID
 *   \param [in] array_index - index of the memory in the array
 *   \param [in] copyno - core ID
 *   \param [in] index - index of the reg/memory in the array
 *   \param [in] is_read - 0 for write. 1 for read
 *   \param [in] entry_data - data of the memory's entry
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
static int
cmodel_mem_access_handle(
    int unit,
    soc_mem_t mem,
    unsigned int array_index,
    int copyno,
    int index,
    int is_read,
    void *entry_data)
{
    uint32 data_byte_len, addr;
    int cmic_blk, blk;
    uint8 acc_type;
    int entry_dw;
    int rv = _SHR_E_NONE;

    SHR_FUNC_INIT_VARS(unit);

    if (mem >= NUM_SOC_REG)
    {
        LOG_ERROR(BSL_LS_SOC_PHYMOD, (BSL_META_U(unit, "invalid memory")));
        return _SHR_E_INTERNAL;
    }

    entry_dw = soc_mem_entry_words(unit, mem);
    data_byte_len = entry_dw * sizeof(uint32);

    SOC_MEM_BLOCK_ITER(unit, mem, blk)
    {
        if (copyno != COPYNO_ALL && copyno != blk)
        {
            continue;
        }

        /*
         * SW block representation to cmic HW block representation
         */
        cmic_blk = SOC_BLOCK2SCH(unit, blk);

        addr = soc_mem_addr_get(unit, mem, array_index, blk, index, &acc_type);

        rv = cmodel_memreg_access(unit, cmic_blk, addr, data_byte_len, 1, is_read, (uint32 *) entry_data);
        SHR_IF_ERR_EXIT(rv);
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Register get 32 bits
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] reg - register ID
 *   \param [in] port - in port
 *   \param [in] index - index of the reg/memory in the array
 *   \param [in] data - value of the register/memory
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
cmodel_reg32_get(
    int unit,
    soc_reg_t reg,
    int port,
    int index,
    uint32 * data)
{
    soc_reg_above_64_val_t data_a64;
    int rv;

    rv = cmodel_reg_access_handle(unit, reg, port, index, 1, data_a64);
    if (rv < 0)
    {
        return rv;
    }

    data[0] = data_a64[0];

    return _SHR_E_NONE;
}

/**
 * \brief - Register get 64 bits
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] reg - register ID
 *   \param [in] port - in port
 *   \param [in] index - index of the reg/memory in the array
 *   \param [in] data - value of the register/memory
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
cmodel_reg64_get(
    int unit,
    soc_reg_t reg,
    int port,
    int index,
    uint64 * data)
{
    soc_reg_above_64_val_t data_a64;
    int rv;

    rv = cmodel_reg_access_handle(unit, reg, port, index, 1, data_a64);
    if (rv < 0)
    {
        return rv;
    }

    COMPILER_64_SET(*data, data_a64[1], data_a64[0]);

    return _SHR_E_NONE;
}

/**
 * \brief - Register get above 64 bits
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] reg - register ID
 *   \param [in] port - in port
 *   \param [in] index - index of the reg/memory in the array
 *   \param [in] data - value of the register/memory
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
cmodel_reg_above64_get(
    int unit,
    soc_reg_t reg,
    int port,
    int index,
    soc_reg_above_64_val_t data)
{
    int rv;
    rv = cmodel_reg_access_handle(unit, reg, port, index, 1, data);
    return rv;
}

/**
 * \brief - Register set 32 bits
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] reg - register ID
 *   \param [in] port - in port
 *   \param [in] index - index of the reg/memory in the array
 *   \param [in] data - value of the register/memory
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
cmodel_reg32_set(
    int unit,
    soc_reg_t reg,
    int port,
    int index,
    uint32 data)
{
    int rv;
    soc_reg_above_64_val_t data_a64;

    SOC_REG_ABOVE_64_CLEAR(data_a64);
    data_a64[0] = data;

    rv = cmodel_reg_access_handle(unit, reg, port, index, 0, data_a64);
    return rv;
}

/**
 * \brief - Register set 64 bits
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] reg - register ID
 *   \param [in] port - in port
 *   \param [in] index - index of the reg/memory in the array
 *   \param [in] data - value of the register/memory
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
cmodel_reg64_set(
    int unit,
    soc_reg_t reg,
    int port,
    int index,
    uint64 data)
{
    int rv;
    soc_reg_above_64_val_t data_a64;

    SOC_REG_ABOVE_64_CLEAR(data_a64);
    data_a64[1] = COMPILER_64_HI(data);
    data_a64[0] = COMPILER_64_LO(data);

    rv = cmodel_reg_access_handle(unit, reg, port, index, 0, data_a64);
    return rv;
}

/**
 * \brief - Register set above 64 bits
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] reg - register ID
 *   \param [in] port - in port
 *   \param [in] index - index of the reg/memory in the array
 *   \param [in] data - value of the register/memory
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
cmodel_reg_above64_set(
    int unit,
    soc_reg_t reg,
    int port,
    int index,
    soc_reg_above_64_val_t data)
{
    int rv;
    rv = cmodel_reg_access_handle(unit, reg, port, index, 0, data);
    return rv;
}

/**
 * \brief - handler for memory array read
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] mem - memory ID
 *   \param [in] array_index - index of the memory in the array
 *   \param [in] copyno - core ID
 *   \param [in] index - index of the reg/memory in the array
 *   \param [in] entry_data - data of the memory's entry
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
cmodel_mem_array_read(
    int unit,
    soc_mem_t mem,
    unsigned int array_index,
    int copyno,
    int index,
    void *entry_data)
{
    int rv;
    rv = cmodel_mem_access_handle(unit, mem, array_index, copyno, index, 1, entry_data);
    return rv;
}

/**
 * \brief - handler for memory array write
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] mem - memory ID
 *   \param [in] array_index - index of the memory in the array
 *   \param [in] copyno - core ID
 *   \param [in] index - index of the reg/memory in the array
 *   \param [in] entry_data - data of the memory's entry
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
cmodel_mem_array_write(
    int unit,
    soc_mem_t mem,
    unsigned int array_index,
    int copyno,
    int index,
    void *entry_data)
{
    int rv;
    rv = cmodel_mem_access_handle(unit, mem, array_index, copyno, index, 0, entry_data);
    return rv;
}

/**
 * \brief - Allocate the sockets for the cmodel.
 * is_regs = 1 allocates the socket for the registers/memories.
 * is_regs = 0 is used for packets.
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] is_regs - 0 for tx socket. 1 for memories and
 *          registers.
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
cmodel_sockets_init(
    int unit,
    socket_target_e socket_target)
{
    /*
     * struct sockaddr_in cli_addr;
     */
    /*
     * socklen_t cli_addr_size;
     */
    struct sockaddr_in srv_addr;
#ifndef VXWORKS
    struct hostent *hostent_ptr = NULL;
#endif
    char *s = NULL;
    char *cmodel_host = NULL;
    char tmp[80];
    int cmodel_host_port;
    int optval = 1;
    int soc_fd;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Allocate mutex to control the socket's handling
     */
    switch (socket_target)
    {
        case CMODEL_RX_TX:
            cmodel_info->cmodel_rx_tx_mutex = sal_mutex_create("C model RxTx socket mutex");
            s = getenv("CMODEL_PACKET_PORT");
            break;
        case CMODEL_REGS:
            cmodel_info->cmodel_mem_reg_mutex = sal_mutex_create("C model registers socket mutex");
            s = getenv("CMODEL_MEMORY_PORT");
            break;
        case CMODEL_SIGNALS:
            cmodel_info->cmodel_sdk_interface_mutex = sal_mutex_create("C model sdk interface socket mutex");
            s = getenv("CMODEL_SDK_INTERFACE_PORT");
            break;
        default:
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Error: cmodel_sockets_init failed\n")));
            SHR_SET_CURRENT_ERR(_SHR_E_FAIL);
            SHR_EXIT();
    }

    if (!s)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_INIT);
        SHR_EXIT();
    }

    /*
     * Setup C model's server port
     */

    cmodel_host = getenv("SOC_TARGET_SERVER");

    /*
     * Get C model's server name
     */
    /** Setup target host */
    if (!cmodel_host)
    {
        snprintf(tmp, sizeof(tmp), "SOC_TARGET_SERVER%d", 0 /* devNo */ );
        cmodel_host = getenv(tmp);
    }

    /*
     * Allocate a socket to the C model's server. This socket will be used for the RxTx packet transmission
     */
    /** Wait until the C model's server is up */
    cmodel_host_port = atoi(s);

    memset((void *) &srv_addr, 0, sizeof(srv_addr));

#ifdef VXWORKS
    if (!isdigit((unsigned) *cmodel_host))
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "cmodel_reg_access_init: can't resolve host names in vxworks\n")));
    }

    srv_addr.sin_addr.s_addr = inet_addr(cmodel_host);
#else
    hostent_ptr = gethostbyname(cmodel_host);
    if (hostent_ptr == NULL)
    {
        /*
         * cli_out("cmodel_reg_access_init: hostname lookup failed for host [%s].\n", cmodel_host);
         * perror("gethostbyname");
         */
        SHR_SET_CURRENT_ERR(_SHR_E_INIT);
        SHR_EXIT();
    }
    memcpy(&srv_addr.sin_addr, hostent_ptr->h_addr, 4);
#endif

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(cmodel_host_port);

    soc_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (soc_fd < 0)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_INIT);
        SHR_EXIT();
    }

    switch (socket_target)
    {
        case CMODEL_RX_TX:
            cmodel_info->cmodel_rx_tx_fd = soc_fd;
            break;
        case CMODEL_REGS:
            cmodel_info->cmodel_mem_reg_fd = soc_fd;
            break;
        case CMODEL_SIGNALS:
            cmodel_info->cmodel_sdk_interface_fd = soc_fd;
            break;
        default:
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Error: cmodel_sockets_init failed\n")));
            SHR_SET_CURRENT_ERR(_SHR_E_FAIL);
            SHR_EXIT();
    }

    /*
     * configure the socket to send the data immidiately
     */
    if (setsockopt(soc_fd, IPPROTO_TCP, TCP_NODELAY, (char *) &optval, sizeof(optval)) < 0)
    {
        SHR_SET_CURRENT_ERR(_SHR_E_INIT);
        SHR_EXIT();
    }

    if (connect(soc_fd, (struct sockaddr *) &srv_addr, sizeof(srv_addr)) < 0)
    {
        SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_INIT,
                                 "Failed connecting the Memory (1), Tx (0) or Signals (2) (%d) socket. Port (%d) cmodel IP %s\r\n",
                                 socket_target, cmodel_host_port, cmodel_host);
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Init sockets to the c model.
 * One socket for registers and memories and the other for RxTx
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   soc_error_t
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
soc_error_t
cmodel_reg_access_init(
    int unit)
{
    SHR_FUNC_INIT_VARS(unit);

    if (!cmodel_info)
    {

        /*
         * Allocate the static context parameter
         */
        cmodel_info = sal_alloc(sizeof(cmodel_access_info_t), "cmodel access_info");

        if (!cmodel_info)
        {
            return _SHR_E_MEMORY;
        }

        sal_memset(cmodel_info, 0, sizeof(cmodel_access_info_t));

        SHR_IF_ERR_EXIT(cmodel_sockets_init(unit, CMODEL_RX_TX));
        SHR_IF_ERR_EXIT(cmodel_sockets_init(unit, CMODEL_REGS));
        cmodel_sockets_init(unit, CMODEL_SIGNALS);
    }

    /*
     * Register callbacks for memory access
     */
    SHR_IF_ERR_EXIT(soc_reg_access_func_register(unit, &cmodel_access));

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - Close the sockets and mutex to the c model's server
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   int
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
int
cmodel_reg_access_deinit(
    int unit)
{
    if (!cmodel_info)
    {
        return _SHR_E_INIT;
    }

    /*
     * Close the socket to the C model's server
     */
    if (cmodel_info->cmodel_rx_tx_fd >= 0)
    {
        close(cmodel_info->cmodel_rx_tx_fd);
        cmodel_info->cmodel_rx_tx_fd = -1;
    }
    /*
     * Free the mutex that control's the socket to the C model's server
     */
    if (cmodel_info->cmodel_rx_tx_mutex)
    {
        sal_mutex_destroy(cmodel_info->cmodel_rx_tx_mutex);
    }

    if (cmodel_info->cmodel_mem_reg_fd >= 0)
    {
        close(cmodel_info->cmodel_mem_reg_fd);
        cmodel_info->cmodel_mem_reg_fd = -1;
    }
    /*
     * Free the mutex that control's the socket to the C model's server
     */
    if (cmodel_info->cmodel_mem_reg_mutex)
    {
        sal_mutex_destroy(cmodel_info->cmodel_mem_reg_mutex);
    }

    /*
     * Close the signals socket to the C model's server
     */
    if (cmodel_info->cmodel_sdk_interface_fd >= 0)
    {
        close(cmodel_info->cmodel_sdk_interface_fd);
        cmodel_info->cmodel_sdk_interface_fd = -1;
    }
    /*
     * Free the mutex that control's the signal socket to the C model's server
     */
    if (cmodel_info->cmodel_sdk_interface_mutex)
    {
        sal_mutex_destroy(cmodel_info->cmodel_sdk_interface_mutex);
    }

    sal_free(cmodel_info);
    cmodel_info = NULL;

    return _SHR_E_NONE;
}

/**
 * \brief - Reads a Tx buffer from the C model's server.
 * Assumes that the buffer's length is a multiple of 4
 * Parameters: sockfd -socket file descriptor. Packet format:
 * length (4 bytes), block ID (4 bytes), NOF signals (4 bytes),
 * [signal ID (4 bytes), signal length (4 bytes), data]
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] ms_id - module ID in the cmodel
 *   \param [in] nof_signals - number of signals
 *   \param [in] src_port - source port
 *   \param [in] len - length of the buffer parameter
 *   \param [in] buf - buffer of the rx packet
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   uint32
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
uint32
cmodel_read_buffer(
    int unit,
    cmodel_ms_id_e * ms_id,
    uint32 * nof_signals,
    uint32 * src_port,
    int *len,
    unsigned char *buf)
{
    fd_set read_vect;
    char swapped_header[MAX_PACKET_HEADER_SIZE_CMODEL];
    uint32 packet_length;
    long long_val;
    int offset = 0;
    int nfds = 0;
    int rv = _SHR_E_NONE;

    SHR_FUNC_INIT_VARS(unit);

    assert(cmodel_info->cmodel_rx_tx_fd);

    LOG_VERBOSE(BSL_LS_SYS_VERINET, (BSL_META("cmodel_read_buffer: sockfd=%d\n"), cmodel_info->cmodel_rx_tx_fd));

    /** Setup bitmap for read notification*/
    FD_ZERO(&read_vect);

    /** Add Cmodel Rx-Tx socket to selected fds*/
    FD_SET(cmodel_info->cmodel_rx_tx_fd, &read_vect);

    /** Add read end of pipe to selected fds*/
    FD_SET(pipe_fds[0], &read_vect);

    /** Set maximum fd */
    nfds = (cmodel_info->cmodel_rx_tx_fd > pipe_fds[0]) ? cmodel_info->cmodel_rx_tx_fd + 1 : pipe_fds[0] + 1;

    /*
     * Listen to two files:
     * - Cmodel Rx-Tx socket (for incoming packets)
     * - Read end of pipe     (for thread exit notification)
     * Once a file contains information to be read, we read it and process
     * accordingly - handle packet or exit thread
     */
    while ((rv = select(nfds, &read_vect, (fd_set *) 0x0, (fd_set *) 0x0, NULL)) == -1 && errno == EINTR)
    {
        continue;
    }

    if (rv < 0)
    {
        perror("get_command: select error");
        SHR_IF_ERR_EXIT(rv);
    }

    /** Thread is about to be closed */
    if (FD_ISSET(pipe_fds[0], &read_vect))
    {
        return RX_THREAD_NOTIFY_CLOSED;
    }

    /** Data ready to be read.*/
    if (FD_ISSET(cmodel_info->cmodel_rx_tx_fd, &read_vect))
    {
        /** Read the length of the packet*/
        if (readn(cmodel_info->cmodel_rx_tx_fd, &(swapped_header[0]), MAX_PACKET_HEADER_SIZE_CMODEL) <
            MAX_PACKET_HEADER_SIZE_CMODEL)
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Cmodel server disconnected\n")));
            SHR_SET_CURRENT_ERR(_SHR_E_DISABLED);
            SHR_EXIT();
        }

        long_val = ntohl(*(uint32 *) & (swapped_header[offset]));
        packet_length = long_val;
        offset += sizeof(uint32);

        long_val = ntohl(*(uint32 *) & (swapped_header[offset]));
        *ms_id = long_val;
        offset += sizeof(uint32);

        long_val = ntohl(*(uint32 *) & (swapped_header[offset]));
        *nof_signals = long_val;
        offset += sizeof(uint32);

        LOG_INFO(BSL_LS_SYS_VERINET,
                 (BSL_META("cmodel_read_buffer: packet_length=%d ms_id=%d nof_signals=%d\n"), packet_length, *ms_id,
                  *nof_signals));

        /*
         * len includes the signal ID, signal length and the data length
         */
        *len = packet_length - 2 * sizeof(uint32);

        /*
         * Read the Rx packet
         */
        if (readn(cmodel_info->cmodel_rx_tx_fd, buf, *len) < *len)
        {
            LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "cmodel_read_buffer: could not read packet\n")));
            SHR_SET_CURRENT_ERR(_SHR_E_FAIL);
            SHR_EXIT();
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief -  Client call: send packet to the cmodel's server
 * unit - not in use
 * ms_id - ID of the block in the C model
 * src_port - not in use
 * len - buf length in bytes
 * not_signals - number of signals in buf
 * Packet format: | length in bytes from ms_id(32 bits) | ms_id (32 bits) | nof_signals (32 bits) | SIGNALS (Signal id 32 bits | data length in bytes 32 bits | data )*
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit-ID
 *   \param [in] ms_id - module ID in the cmodel
 *   \param [in] src_port - source port
 *   \param [in] len - length of the buffer parameter
 *   \param [in] buf - buffer with the tx packet's data
 *   \param [in] nof_signals - number of signals
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   uint32
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
uint32
cmodel_send_buffer(
    int unit,
    cmodel_ms_id_e ms_id,
    uint32 src_port,
    int len,
    unsigned char *buf,
    int nof_signals)
{
    uint32 nw_order_int;
    uint32 index_position = 0;
    uint32 buf_len_in_bits = len * 8;
    int index;
    char packet_data[MAX_PACKET_SIZE_CMODEL];

    LOG_INFO(BSL_LS_SYS_VERINET, (BSL_META("cmodel_send_buffer: src_port=%d len=0x%x\n"), src_port, len));

    sal_mutex_take(cmodel_info->cmodel_rx_tx_mutex, sal_mutex_FOREVER);

    /** Add packet's size. Not including the size field in the size */
    /*
     * Add the ms_id size and nof_signals size to the buffer's length.
     * Add the signal ID and signal's length as well.
     */
    nw_order_int = ntohl(buf_len_in_bits + 4 * sizeof(uint32));
    sal_memcpy(&(packet_data[index_position]), &nw_order_int, sizeof(int));
    index_position += sizeof(int);

    /** Add MS ID */
    nw_order_int = ntohl(ms_id);
    sal_memcpy(&(packet_data[index_position]), &nw_order_int, sizeof(int));
    index_position += sizeof(int);

    /** Add number of signals */
    nw_order_int = ntohl(nof_signals);
    sal_memcpy(&(packet_data[index_position]), &nw_order_int, sizeof(int));
    index_position += sizeof(int);

    /** Add signal ID */
    nw_order_int = ntohl(TX_PACKET_SIGNAL);
    sal_memcpy(&(packet_data[index_position]), &nw_order_int, sizeof(int));
    index_position += sizeof(int);

    /** Add signal length in bytes. Each bit consumes a byte in the packet */
    nw_order_int = ntohl(buf_len_in_bits);
    sal_memcpy(&(packet_data[index_position]), &nw_order_int, sizeof(int));
    index_position += sizeof(int);

    /** Add bufferf */
    /*
     * Convert the buf to characters of '0' and '1'
     */
    for (index = 0; index < len; index++)
    {
        sal_memcpy(&(packet_data[index_position + index * 8]), conversion_tbl[(int) buf[index]], 8);
    }
    index_position += buf_len_in_bits;

    /*
     * write the packet to the socket
     */
    if (writen(cmodel_info->cmodel_rx_tx_fd, packet_data, index_position) != index_position)
    {
        LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "Error: cmodel_send_packet data failed\n")));
        sal_mutex_give(cmodel_info->cmodel_rx_tx_mutex);
        return -1;
    }

    sal_mutex_give(cmodel_info->cmodel_rx_tx_mutex);

    return 0;
}

/*
 * }
 */
/** CMODEL_SERVER_MODE */
#endif
