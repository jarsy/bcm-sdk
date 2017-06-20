/*
 * ! \file cmodel_reg_access.h
 * This file handles the communication with the C model's server
 */
/*
 * $Id:cmodel_reg_access.h,v 1.312 Broadcom SDK $                                                           $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$ $
 */

#ifndef C_MODEL_REG_ACCESS_H
#define C_MODEL_REG_ACCESS_H

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif
/*
 * Include files
 * {
 */

#include <include/soc/register.h>

/*
 * }
 */

/*
 * Defines
 * {
 */

/** Maximum packet size in cmodel */
#define MAX_PACKET_SIZE_CMODEL 3000

/*
 * }
 */

/*
 * Enum of the MS IDs in the C model
 */

typedef enum
{
    /** Loopback module id in cmodel */
    CMODEL_MS_ID_LOOPBACK = -3,
    CMODEL_MS_ID_FIRST_MS = -1
} cmodel_ms_id_e;

extern int pipe_fds[2];

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
 * \brief - Get signals from the cmodel server
 *
 * Packet request format: | opcode (1 byte) | MS_ID (32 bits)
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit id
 *   \param [in] ms_id - Block id
 *   \param [in] signal_data - Allocated buffer for cmodel signal data
 *
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   \param [out] signal_data_length - Number of bytes in signal_buffer
 *   \param [out] signal_data - Allocated buffer for cmodel signal data
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
int cmodel_get_signal(
    int unit,
    uint32 ms_id,
    uint32 * signal_data_length,
    char **signal_data);

/**
 * \brief - Read the block response from the cmodel server
 *
 * Packet response format:
 * length in bytes from opcode (32 bits) | opcode (1 byte) | nof_blocks (32 bits)| (MS_ID (32 bits) |
 * ms_name_length | ms_name)
 *
 * \par DIRECT_INPUT:
 *   \param [in] unit - Unit id
 *   \param [in] block_names - Allocated buffer for cmodel block names
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
int cmodel_get_block_names(
    int unit,
    int *block_names_length,
    char **block_names);

/**
 * \brief - Register access init
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
int cmodel_reg_access_init(
    int unit);
/**
 * \brief - Register access deinit
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
int cmodel_reg_access_deinit(
    int unit);

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
uint32 cmodel_read_buffer(
    int unit,
    cmodel_ms_id_e * ms_id,
    uint32 * nof_signals,
    uint32 * src_port,
    int *len,
    unsigned char *buf);
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
uint32 cmodel_send_buffer(
    int unit,
    cmodel_ms_id_e ms_id,
    uint32 src_port,
    int len,
    unsigned char *buf,
    int nof_signals);
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
int cmodel_memreg_access(
    int unit,
    int cmic_block,
    uint32 addr,
    uint32 data_length,
    int is_mem,
    int is_get,
    uint32 * data);

/**C_MODEL_REG_ACCESS_H*/
#endif
