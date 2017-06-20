/*
 * $Id: mde_agent.c,v 1.5.2.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:      mde_agent.c
 *
 * Purpose:   MDE Server
 *
 *            This module provides a network interface for controling
 *            the C3 debugger.
 *
 *            Currently, this only supported for Linux User mode and VxWorks
 *            chassis based systems.
 */


/*
 * This implementation is built on top of a socket API interface.
 * This means that there must be OS dependent include files to use the
 * socket interface.  To make this truly portable, this interface should
 * be abstracted in some way.  Until this is done, the MDE Agent
 * implementation works for Linux user mode and VxWorks, but not Linux
 * kernel mode.
 *
 * The socket APIs used are:
 *    socket, connect, accept, bind, listen, close, shutdown,
 *    read, write, setsockopt
 */

#undef BCM_MDE_AGENT_SOCKET_SUPPORTED

#include <shared/bsl.h>

/*
 * Linux User mode or unix
 */
#if (defined(LINUX) || defined(unix)) && !defined(__KERNEL__)
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#define BCM_MDE_AGENT_SOCKET_SUPPORTED 1
typedef socklen_t mde_agenttrans_socklen_t;
#endif /* LINUX && !__KERNEL__ */

/*
 * Linux Kernel mode (unsupported)
 */
/*#if defined(LINUX) && defined(__KERNEL__)*/
#if defined(LINUX) && defined(__KERNEL__)
#undef BCM_MDE_AGENT_SOCKET_SUPPORTED
#endif /* (LINUX || unix) && __KERNEL__ */

/*
 * VxWorks
 */
#if defined(VXWORKS)
#include <ioLib.h>
#include <sockLib.h>
#include <inetLib.h>
#include <selectLib.h>
#include <netinet/tcp.h>
#define BCM_MDE_AGENT_SOCKET_SUPPORTED 1
typedef int mde_agenttrans_socklen_t;
#endif /* VXWORKS */

#include <appl/diag/system.h>
#include <sal/types.h>
#include <shared/alloc.h>
#include <sal/core/libc.h>
#include <sal/core/thread.h>
#include <soc/util.h>
#include <soc/cm.h>
#include <soc/feature.h>
#include <soc/property.h>
#include <soc/sbx/sbDq.h>
#include <soc/debug.h>
#include <soc/types.h>
#include <appl/diag/shell.h>
#include <appl/diag/parse.h>
#ifdef BCM_CALADAN3_SUPPORT
#include <appl/diag/sbx/caladan3_cmds.h>
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/caladan3/port.h>
#include <soc/sbx/caladan3/ppe.h>
#include <soc/sbx/caladan3/ped.h>
#include <soc/sbx/caladan3/sws.h>
#include <soc/sbx/caladan3/tmu/tmu.h>
#include <soc/sbx/caladan3/lrp.h>
#include <soc/sbx/caladan3/ucodemgr.h>
#include <soc/sbx/caladan3/asm3/debug.h>
#endif
#include <soc/shared/mde_agent_msg.h>
#include <soc/sbx/sbx_txrx.h>

#include <shared/util.h>

#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/init.h>
#include <bcm/rx.h>
#include <bcm/tx.h>
#include <bcm/pkt.h>


#ifdef BCM_CALADAN3_SUPPORT
#ifdef BCM_CALADAN3_G3P1_SUPPORT
extern int mde_agent_g3p1_ocm_table_info_get(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length);
#endif
#ifdef BCM_CALADAN3_T3P1_SUPPORT
extern int mde_agent_t3p1_ocm_table_info_get(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length);
#endif


#if defined(BCM_MDE_AGENT_SOCKET_SUPPORTED)

/* Note: Couldn't find equivalent for VxWorks */
#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif

typedef enum {
    MDE_AGENT_DEBUG_STATUS_IDLE = 0,
    MDE_AGENT_DEBUG_STATUS_RUNNING,
    MDE_AGENT_DEBUG_STATUS_DEBUG
} asm_debug_status_t;

asm_debug_status_t mde_agent_debug_state;

/* -------------------------------------------------------------------
 * Semaphores - Locks
 *
 * Used for coordinating access to global resources
 */
static sal_mutex_t mde_agent_sock_mlock = NULL;

#define MDE_AGENT_SOCK_LOCK         sal_mutex_take(mde_agent_sock_mlock, sal_mutex_FOREVER)
#define MDE_AGENT_SOCK_UNLOCK       sal_mutex_give(mde_agent_sock_mlock)
#define MDE_AGENT_SOCK_INIT_DONE    (mde_agent_sock_mlock != NULL)

/* -------------------------------------------------------------------
 * Threads
 *
 * The MDE_AGENT over Sockets module may spawn the following threads,
 *   - A Server thread to service connection requests
 *   - A Connection-RX thread for each connection to service incoming packets
 */
#define MDE_AGENT_SOCK_THREAD_NAME_LEN         15
#define MDE_AGENT_SOCK_THREAD_PRI_DEFAULT     100
#define MDE_AGENT_SOCK_STOP_RETRIES_MAX        50
#define MDE_AGENT_SOCK_RETRY_WAIT_US       100000  /* 100 ms */


/* -------------------------------------------------------------------
 * Server
 *
 * The Server will have a thread that will listen for connection
 * requests.
 *
 * Note:  Current implementation supports only ONE server thread
 *        running on a system at a given time.
 */
#define MDE_AGENT_SOCK_BACKLOG                 5
#define MDE_AGENT_SOCK_SERVER_LPORT_DEFAULT 20567 /* Port to listen for connections */
#define MDE_AGENT_SOCK_SERVER_NAME_DEFAULT  "bcmMDESockS"

static int  mde_agent_sock_server_lport     = -1;
static int  mde_agent_sock_server_lsock     = -1;
static char mde_agent_sock_server_thread_name[MDE_AGENT_SOCK_THREAD_NAME_LEN] =
                                             MDE_AGENT_SOCK_SERVER_NAME_DEFAULT;
static volatile sal_thread_t mde_agent_sock_server_thread_id = SAL_THREAD_ERROR;
static volatile int          mde_agent_sock_server_exit = FALSE;

#define MDE_AGENT_SOCK_SERVER_RUNNING  (mde_agent_sock_server_thread_id != SAL_THREAD_ERROR)


/* Configurable parameters */
static int mde_agent_sock_cfg_thread_pri = MDE_AGENT_SOCK_THREAD_PRI_DEFAULT;
static int mde_agent_sock_cfg_lport      = MDE_AGENT_SOCK_SERVER_LPORT_DEFAULT;

extern debugmgr_t _dbgmgr[SOC_MAX_NUM_DEVICES];

/* -------------------------------------------------------------------
 * Connection
 *
 * Contains information for a given connection.  It manages the transmitting
 * and receiving of packets.  An RX-Thread is associated for each connection.
 *
 *   sock_fd         - Socket for sending/receiving data
 *   rx_thread_name  - Name of the connection receive thread
 *   rx_thread_id    - ID of the connection receive thread
 *   flags           - Indicates more information for connection
 */

#define MDE_AGENT_SOCK_CONN_NAME  "bcmMDESock" /* bcmMDESockXXX, where XXX = socket */

/* Flags for Connection entry */
#define MDE_AGENT_SOCK_CONN_CLEAR    0x0
#define MDE_AGENT_SOCK_CONN_VALID    0x1 /* If set, a connection exists */
#define MDE_AGENT_SOCK_CONN_ABORT    0x2 /* If set, connection is to be destroyed */

typedef struct mde_agent_sock_conn_entry_s {
    int                 unit;
    int                 sock_fd;
    char                rx_thread_name[MDE_AGENT_SOCK_THREAD_NAME_LEN];
    sal_thread_t        rx_thread_id;
    volatile uint32     flags;
} mde_agent_sock_conn_entry_t;

static mde_agent_sock_conn_entry_t mde_agent_mde;

/*
 * Packet Tx and Rx bufs
 */
#define MDE_AGENT_TXRX_BUF_SIZE 16384
static char **mde_agent_txrx_bufs = NULL;

cmd_result_t cmd_sbx_caladan_hc(int unit, args_t *a);

/* -------------------------------------------------------------------
 * Function - Implementation
 */

/*
 * Function:
 *     mde_agent_txrx_init_buf
 * Purpose:
 *     Allocate tx and rx buf
 * Parameters:
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
static int
mde_agent_txrx_init_buf (int unit)
{
    if (!mde_agent_txrx_bufs) {
        mde_agent_txrx_bufs
            = sal_alloc(sizeof(char *) * BCM_MAX_NUM_UNITS,
                        "MDE Agent txrx bufs");
        if (!mde_agent_txrx_bufs) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "mde_agent_txrx_init_buf: buffer pointer alloc"
                                  " failed.\n")));
            return BCM_E_MEMORY;
        }
        sal_memset(mde_agent_txrx_bufs, 0, sizeof(char *) * BCM_MAX_NUM_UNITS);
    }

    if (!mde_agent_txrx_bufs[unit]) {
        mde_agent_txrx_bufs[unit]
            = soc_cm_salloc(unit, MDE_AGENT_TXRX_BUF_SIZE, "SBX diag packet buffer");
        if (!mde_agent_txrx_bufs[unit]) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "mde_agent_txrx_init_buf: DMAable buffer alloc"
                                  " failed. \n")));
            return BCM_E_MEMORY;
        }
        sal_memset(mde_agent_txrx_bufs[unit], 0, sizeof(char) * MDE_AGENT_TXRX_BUF_SIZE);
    }

   return BCM_E_NONE;
}


/*
 * Function:
 *     mde_agent_create_system_status
 * Purpose:
 *     Create system status mesage
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_create_system_status(int unit, mde_agent_msg_pkt_hdr_t *tx_hdr, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;
    char str[128];
    char msg[512];
    uint16 tx_flags;

    tx_flags = MDE_AGENT_MSG_FLAG_LAST_MSG;

    tx_hdr->version = MDE_AGENT_VERSION;
    tx_hdr->flags   = htons(tx_flags);
    tx_hdr->module  = MDE_AGENT_MODULE_SYSTEM;
    tx_hdr->type    = MDE_AGENT_MSG_T_SYSTEM_STATUS;
    tx_hdr->reserved[0] = 0;
    tx_hdr->reserved[1] = 0;
    tx_hdr->reserved[2] = 0;
    tx_hdr->device = htonl(MDE_AGENT_C3_DEVICE);
    tx_hdr->sequence_number = 0;

    msg[0] = '\0';
    sal_sprintf(str, "Broadcom Command Monitor: Copyright (c) 1998-2010 Broadcom Corporation\n");
    strcat(msg, str);
    sal_sprintf(str, "Release: %s built %s (%s)\n", _build_release, _build_datestamp, _build_date);
    strcat(msg, str);
    sal_sprintf(str, "From %s@%s:%s\n", _build_user, _build_host, _build_tree);
    strcat(msg, str);
#if defined(BCM_PLATFORM_STRING)
    sal_sprintf(str, "Platform: %s\n", BCM_PLATFORM_STRING);
    strcat(msg, str);
#endif
    sal_sprintf(str, "OS: %s\n", sal_os_name() ? sal_os_name() : "unknown");
    strcat(msg, str);

    *tx_length = sal_strlen(msg) + 1;
    tx_hdr->length = htonl(*tx_length);
    *tx_data = sal_alloc(*tx_length, "tx_data");
    sal_strncpy((char*)*tx_data, msg, *tx_length);

    return rv;
}

/*
 * Function:
 *     mde_agent_process_bp_set
 * Purpose:
 *     Process a breakpoint set message
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_bp_set(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;
    uint32 snum;
    uint32 inum;
    int    bpnum;
    uint32 *ptr;

    *tx_data = NULL;
    *tx_length = 0;
    *tx_flags = 0;

    /*
     * Data should consist of:
     * uint32 stream
     * uint32 instruction_number
     */
    if (rx_length != sizeof(uint32) * 2) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));
        rv = BCM_E_PARAM;
    } else {

        snum = ntohl(*(uint32*)rx_data);
        rx_data += sizeof(uint32);
        inum = ntohl(*(uint32*)rx_data);

        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "MDE_AGENT-Socket: Breakpoint stream:%d instruction:%d\n"), snum, inum));

        rv = soc_sbx_caladan3_ucode_debug_bp_set(unit, snum, inum, &bpnum);

        if (rv == SOC_E_NONE) {
            *tx_flags = MDE_AGENT_MSG_FLAG_ACK;
            *tx_data = (uint8*)sal_alloc(sizeof(uint32) * 3, "tx data");
            *tx_length = 0;
            ptr = (uint32*)(*tx_data);
            *ptr = htonl(bpnum);
            ptr++;
            *tx_length += sizeof(uint32);
            *ptr = htonl(snum);
            ptr++;
            *tx_length += sizeof(uint32);
            *ptr = htonl(inum);
            *tx_length += sizeof(uint32);
        }

        *tx_flags |= MDE_AGENT_MSG_FLAG_LAST_MSG;
    }

    return rv;
}


/*
 * Function:
 *     mde_agent_process_state
 * Purpose:
 *     Process a state message
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_state(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;
    uint32 *ptr;
    debugmgr_t *mgr;

    *tx_data = NULL;
    *tx_length = 0;
    *tx_flags = 0;

    /*
     * Data should consist of:
     * uint32 stream
     * uint32 instruction_number
     */
    if (rx_length != 0) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));
        rv = BCM_E_PARAM;
    } else {
        mgr = &_dbgmgr[unit];

        *tx_flags = MDE_AGENT_MSG_FLAG_ACK;
        *tx_data = (uint8*)sal_alloc(sizeof(uint32) * 3, "tx data");
        ptr = (uint32*)*tx_data;
        *ptr = htonl(mgr->curr_frame.snum);
        ptr++;
        *tx_length += sizeof(uint32);
        *ptr = htonl(mgr->curr_frame.inum);
        *tx_length += sizeof(uint32);

        *tx_flags |= MDE_AGENT_MSG_FLAG_LAST_MSG;
    }

    return rv;
}


/*
 * Function:
 *     mde_agent_process_bp_clear
 * Purpose:
 *     Process a breakpoint clear message
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_bp_clear(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;
    uint32 bpnum;

    *tx_data = NULL;
    *tx_length = 0;

    /*
     * Data should consist of:
     * uint32 bp_num
     */
    if (rx_length != sizeof(uint32)) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));
        rv = BCM_E_PARAM;
    } else {

        bpnum = ntohl(*(uint32*)rx_data);

        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Breakpoint bp_num:%d\n"), bpnum));

        rv = soc_sbx_caladan3_ucode_debug_bp_clear(unit, bpnum );

        if (rv == SOC_E_NONE)
            *tx_flags = MDE_AGENT_MSG_FLAG_ACK;
        else
            *tx_flags = 0;

        *tx_flags = MDE_AGENT_MSG_FLAG_LAST_MSG;
    }
    return rv;
}


/*
 * Function:
 *     mde_agent_process_bp_clear_all
 * Purpose:
 *     Process a breakpoint clear message
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_bp_clear_all(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;
    uint32 bpnum = 0;

    *tx_data = NULL;
    *tx_length = 0;

    /*
     * Data should consist of:
     *
     */
    if (rx_length != 0) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));
        rv = BCM_E_PARAM;
    } else {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Breakpoint bp_num:%d\n"), bpnum));

        rv = soc_sbx_caladan3_ucode_debug_bp_clear_all(unit);

        if (rv == SOC_E_NONE)
            *tx_flags = MDE_AGENT_MSG_FLAG_ACK;
        else
            *tx_flags = 0;

        *tx_flags = MDE_AGENT_MSG_FLAG_LAST_MSG;
    }
    return rv;
}


/*
 * Function:
 *     mde_agent_process_bp_list
 * Purpose:
 *     Process a breakpoint list message
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_bp_list(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;
    uint32 *bpnum = NULL;
    uint32 *snum = NULL;
    uint32 *inum = NULL;
    uint32 count = 0;
    int i;

    *tx_data = NULL;
    *tx_length = 0;

    /*
     * Data should consist of:
     *
     */
    if (rx_length != 0) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));
        rv = BCM_E_PARAM;
    } else {
        count = soc_sbx_caladan3_ucode_debug_bp_count(unit);

        *tx_length = (count * 3 * sizeof(uint32)) + sizeof(uint32);
        *tx_data = (uint8*)sal_alloc(*tx_length, "bp_list data");
        *((uint32*)*tx_data) = htonl(count);
        if (count) {
            bpnum = (uint32*)(*tx_data + sizeof(uint32));
            snum = bpnum + count;
            inum = snum + count;

            rv = soc_sbx_caladan3_ucode_debug_bp_list_get(unit, count, bpnum, snum, inum);
        }

        for (i = 0;i < count;i++) {
            *bpnum = htonl(*bpnum);
            bpnum++;
            *snum = htonl(*snum);
            snum++;
            *inum = htonl(*inum);
            inum++;
        }

        if (rv == SOC_E_NONE)
            *tx_flags = MDE_AGENT_MSG_FLAG_ACK;
        else
            *tx_flags = 0;

        *tx_flags = MDE_AGENT_MSG_FLAG_LAST_MSG;
    }
    return rv;
}


/*
 * Function:
 *     mde_agent_process_reg_read
 * Purpose:
 *     Process a register read message
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_reg_read(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;
    uint32 count;
    char *reg;
    uint32 value;
    int i;
    uint32 *regv;
    char *regn;
    char *ptr;

    /*
     * Data should consist of:
     * uint32 count
     * char register[count]
     */
    if (rx_length < sizeof(uint32) * 2) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));
        rv = BCM_E_PARAM;
        return rv;
    }

    count = ntohl(*(uint32*)rx_data);
    rx_data += sizeof(uint32);
    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                (BSL_META_U(unit,
                            "MDE_AGENT-Socket: Register read count:%d\n"), count));

    if (tx_data == NULL) {
        *tx_length = 0;
        *tx_flags = 0;
        rv = BCM_E_MEMORY;
        return rv;
    } else {

        *tx_length = sizeof(uint32) + (count * MDE_AGENT_MAX_REG_NAME) + (count * sizeof(uint32));
        *tx_data = sal_alloc(*tx_length, "tx_data");
        ptr = (char*)*tx_data;
        *(uint32*)ptr = htonl(count);
        ptr += sizeof(uint32);
        regn = (char*)ptr;
        regv = (uint32*)(ptr + (count * MDE_AGENT_MAX_REG_NAME));

        *tx_flags = MDE_AGENT_MSG_FLAG_LAST_MSG | MDE_AGENT_MSG_FLAG_ACK;

        for (i = 0;i < count && rv == BCM_E_NONE;i++) {
            reg = (char*)rx_data;
            rx_data += MDE_AGENT_MAX_REG_NAME;

            rv = soc_sbx_caladan3_ucode_debug_get_reg(unit, reg, &value);
            if (SOC_SUCCESS(rv)) {
                LOG_VERBOSE(BSL_LS_APPL_COMMON,
                            (BSL_META_U(unit,
                                        "%s: get reg:%s value:0x%08x\n"), FUNCTION_NAME(), reg, value));
                sal_strcpy(regn, reg);
                regn += MDE_AGENT_MAX_REG_NAME;
                *regv = htonl(value);
                regv++;
            } else {
                *tx_flags &= ~MDE_AGENT_MSG_FLAG_ACK;
                break;
            }
        }

        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "%s: get reg count:%d length:%d\n"), FUNCTION_NAME(), count, *tx_length));
        *tx_flags |= MDE_AGENT_MSG_FLAG_LAST_MSG;

    }

    return rv;
}

/*
 * Function:
 *     mde_agent_process_reg_write
 * Purpose:
 *     Process a register write message
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_reg_write(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;
    uint32 count;
    char *regn;
    uint32 *regv;
    int i;
    int update_context = 0;

    *tx_length = 0;
    *tx_data = NULL;
    *tx_flags = 0;

    /*
     * Data should consist of:
     * uint32 count
     * uint32 register[count]
     * uint32 value[count]
     */
    if (rx_length < sizeof(uint32) * 2) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));
        rv = BCM_E_PARAM;
    } else {

        count = ntohl(*(uint32*)rx_data);
        rx_data += sizeof(uint32);
        regn = (char*)rx_data;
        regv = (uint32*)(rx_data + (count * MDE_AGENT_MAX_REG_NAME));

        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Register read count:%d\n"), count));


        *tx_flags = MDE_AGENT_MSG_FLAG_LAST_MSG | MDE_AGENT_MSG_FLAG_ACK;

        for (i = 0;i < count;i++) {
            /*
             * If this is the last reg then update the context
             */
            if (i == (count - 1))
                update_context = 1;

            rv = soc_sbx_caladan3_ucode_debug_set_reg(unit, regn, ntohl(*regv), update_context);
            if (SOC_SUCCESS(rv)) {
                regn += MDE_AGENT_MAX_REG_NAME;
                regv++;
            } else {
                *tx_flags &= ~MDE_AGENT_MSG_FLAG_ACK;
                break;
            }
        }
    }

    return rv;
}


/*
 * Function:
 *     mde_agent_process_symbol_get
 * Purpose:
 *     Process a symbol get (read) request
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_ucode_symbol_get(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;
    uint32 count;
    char *sname;
    uint32 value;
    int i;
    char *ptr;
    int slen;

    /*
     * Data should consist of:
     * uint32 count
     * char symbols[] - Stream of count Null terminated strings
     */
    if (rx_length < sizeof(uint32)) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));
        rv = BCM_E_PARAM;
        return rv;
    }

    count = ntohl(*(uint32*)rx_data);
    rx_data += sizeof(uint32);
    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                (BSL_META_U(unit,
                            "MDE_AGENT-Socket: Symbol get count:%d\n"), count));

    if (tx_data == NULL) {
        *tx_length = 0;
        *tx_flags = 0;
        rv = BCM_E_MEMORY;
        return rv;
    } else {

        *tx_length = rx_length + (count * sizeof(uint32));
        *tx_data = sal_alloc(*tx_length, "tx_data");
        ptr = (char*)*tx_data;
        *(uint32*)ptr = htonl(count);
        ptr += sizeof(uint32);


        *tx_flags = MDE_AGENT_MSG_FLAG_LAST_MSG | MDE_AGENT_MSG_FLAG_ACK;

        for (i = 0;i < count && rv == BCM_E_NONE;i++) {
            sname = (char*)rx_data;
            slen = sal_strlen(sname) + 1;
            rx_data += slen;

            rv = soc_sbx_caladan3_ucode_debug_get_symbol(unit, sname, &value);
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "%s: get symbol:%s value:0x%08x\n"), FUNCTION_NAME(), sname, value));
            if (SOC_SUCCESS(rv)) {
                sal_strncpy(ptr, sname, slen);
                ptr += slen;
                *((uint32*)ptr) = htonl(value);
                ptr += sizeof(uint32);
            } else {
                *tx_length = 0;
                sal_free(*tx_data);
                *tx_data = NULL;
                *tx_flags &= ~MDE_AGENT_MSG_FLAG_ACK;
                break;
            }
        }
    }
    return rv;
}


/*
 * Function:
 *     mde_agent_process_symbol_set
 * Purpose:
 *     Process a symbol set message
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_ucode_symbol_set(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;
    uint32 count;
    char *sname;
    uint32 value;
    int i;

    *tx_length = 0;
    *tx_data = NULL;
    *tx_flags = 0;

    /*
     * Data should consist of:
     * uint32 count
     * uint32 symbol[]
     * uint32 value
     */
    if (rx_length < sizeof(uint32) * 2) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));
        rv = BCM_E_PARAM;
    } else {

        count = ntohl(*(uint32*)rx_data);
        rx_data += sizeof(uint32);

        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Symbol set count:%d\n"), count));


        *tx_flags = MDE_AGENT_MSG_FLAG_LAST_MSG | MDE_AGENT_MSG_FLAG_ACK;

        for (i = 0;i < count;i++) {
            sname = (char*)rx_data;
            rx_data += sal_strlen(sname) + 1;
            value = ntohl(*((uint32*)rx_data));
            rx_data += sizeof(uint32);
            rv = soc_sbx_caladan3_ucode_debug_set_symbol(unit, sname, value);

            if (!SOC_SUCCESS(rv)) {
                *tx_flags &= ~MDE_AGENT_MSG_FLAG_ACK;
                break;
            }
        }
    }

    return rv;
}



/*
 * Function:
 *     mde_agent_process_tx_packet
 * Purpose:
 *     Process a tx packet message
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_tx_packet(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;
    uint32 count;
    uint32 length;
    int i;
    uint8 *ptr;
    bcm_pkt_t  pkt;
#ifndef BCM_SIRIUS_SUPPORT
    int node = unit;
#endif
    int port = 49, mod_id=0;

    *tx_length = 0;
    *tx_data = NULL;
    *tx_flags = 0;

    /*
     * Data should consist of:
     * uint32 count
     * uint32 length
     * uint8  data[length]
     */
    if (rx_length < sizeof(uint32) * 2) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));
        rv = BCM_E_PARAM;
    } else {

        count = ntohl(*(uint32*)rx_data);
        rx_data += sizeof(uint32);
        length = ntohl(*(uint32*)rx_data);
        rx_data += sizeof(uint32);

        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "%s: count:%d  length:%d\n"),
                     FUNCTION_NAME(),
                     count,
                     length));

        if (length > MDE_AGENT_TXRX_BUF_SIZE) {
            rv = BCM_E_PARAM;
        } else {

            rv = mde_agent_txrx_init_buf (unit);

            if (SOC_SUCCESS(rv)) {
                ptr = (uint8*)mde_agent_txrx_bufs[unit];

                for (i = 0;i < length;i++) {
                    *ptr = *rx_data;
                    ptr++;
                    rx_data++;
                }

                sal_memset(&pkt, 0, sizeof(bcm_pkt_t));
                pkt.pkt_data = &pkt._pkt_data;

                pkt.blk_count = 1;
                pkt.pkt_data->data = (uint8 *) mde_agent_txrx_bufs[unit];
                pkt.pkt_data->len = length;

#ifndef BCM_SIRIUS_SUPPORT
                SOC_SBX_MODID_FROM_NODE(node, mod_id);
#endif

                pkt.dest_mod = mod_id;
                pkt.dest_port = port;
                pkt.cos = 0;
                pkt.flags |= BCM_PKT_F_TEST | BCM_TX_CRC_ALLOC;
                pkt.opcode = BCM_PKT_OPCODE_CPU;

                for (i=0; i<count; i++) {
                    rv = bcm_tx_pkt_setup(unit, &pkt);
                    if (BCM_FAILURE(rv)) {
                        cli_out("bcm_tx_pkt_setup error:%d %s\n", rv, bcm_errmsg(rv));
                        return CMD_FAIL;
                    }
#ifdef BCM_SIRIUS_SUPPORT
                    if (SOC_IS_SBX_SIRIUS(unit)) {
                        pkt.flags |= (BCM_TX_HG_READY | BCM_TX_NO_PAD);
                    }
#endif

                    rv = bcm_tx(unit, &pkt, NULL);
                    if (BCM_FAILURE(rv)) {
                        cli_out("bcm_tx error:%d %s\n", rv, bcm_errmsg(rv));
                        return CMD_FAIL;
                    }
                }

                *tx_flags = MDE_AGENT_MSG_FLAG_LAST_MSG | MDE_AGENT_MSG_FLAG_ACK;
            }
        }
    }

    return rv;
}



/*
 * Function:
 *     mde_agent_process_header_capture
 * Purpose:
 *     Process a header capture message
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_header_capture(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;
    int *ptr;
    int on;
    int drop;
    int clear;
    int queue;
    int stream;
    int var;
    int varmask;

    *tx_length = 0;
    *tx_data = NULL;
    *tx_flags = 0;

    /*
     * Data should consist of:
     */
    if (rx_length < sizeof(uint32) * 7) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));
        rv = BCM_E_PARAM;
    } else {
        ptr = (int*)rx_data;
        on = ntohl(*ptr);
        ptr++;
        drop = ntohl(*ptr);
        ptr++;
        clear = ntohl(*ptr);
        ptr++;
        queue = ntohl(*ptr);
        ptr++;
        stream = ntohl(*ptr);
        ptr++;
        var = ntohl(*ptr);
        ptr++;
        varmask = ntohl(*ptr);
        ptr++;

#if 0
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "%s: count:%d  length:%d\n"),
                     FUNCTION_NAME(),
                     count,
                     length));
#endif
        rv = soc_sbx_caladan3_ppe_hc_control(unit,
                                             on,
                                             clear,
                                             queue,
                                             stream,
                                             var,
                                             varmask);
        /* coverity [mixed_enums] */
        if(rv == BCM_E_NONE) {
            rv = cmd_sbx_caladan3_pd_hc(unit, queue, on, drop);
        }

        *tx_flags = MDE_AGENT_MSG_FLAG_LAST_MSG | MDE_AGENT_MSG_FLAG_ACK;
    }

    return rv;
}

/*
 * Function:
 *     mde_agent_process_header_dump
 * Purpose:
 *     Process a header dump message
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_header_dump(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;
    uint32 *count;
    uint32 pedin;
    uint32 pedout;
    uint32 parsed;
    uint32 *ptr;
    uint32 *ppe_headers;
    uint32 *ped_headers_in;
    uint32 *ped_headers_out;
    uint32 regval = 0;
    int npedin, npedout;
    int numpkts;

    *tx_length = 0;
    *tx_data = NULL;
    *tx_flags = 0;

    /*
     * Data should consist of:
     * uint32 count
     * uint32 length
     * uint8  data[length]
     */
    if (rx_length < sizeof(uint32) * 3) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));
        rv = BCM_E_PARAM;
    } else {

        ptr = (uint32*)rx_data;
        pedin = ntohl(*ptr);
        ptr++;
        pedout = ntohl(*ptr);
        ptr++;
        parsed = ntohl(*ptr);
        ptr++;

        regval = 0;
        READ_PP_HDR_COPY_CONTROL0r(unit, &regval);
        numpkts = soc_reg_field_get(unit, PP_HDR_COPY_CONTROL0r,
                                    regval, COPY_COUNTf);
        regval = 0;
        READ_PD_COPY_BUF_LEVELr(unit, &regval);
        npedin = soc_reg_field_get(unit, PD_COPY_BUF_LEVELr,
                            regval, CI_LVLf);
        npedin /= 2;
        npedout = soc_reg_field_get(unit, PD_COPY_BUF_LEVELr,
                            regval, CO_LVLf);
        npedout /= 2;

        *tx_length = (sizeof(uint32) * 3) + (numpkts * MDE_AGENT_HEADER_SIZE) + (npedin * MDE_AGENT_HEADER_SIZE) + (npedout * MDE_AGENT_HEADER_SIZE);
        *tx_data = sal_alloc(*tx_length, "MDE Agent dump header buffers");
        count = (uint32*)*tx_data;
        *count = htonl(numpkts);
        ppe_headers = count + 1;
        count = ppe_headers + ((numpkts * MDE_AGENT_HEADER_SIZE)/sizeof(uint32));
        *count = htonl(npedin);
        ped_headers_in = count + 1;
        count = ped_headers_in + ((npedin * MDE_AGENT_HEADER_SIZE)/sizeof(uint32));
        *count = htonl(npedout);
        ped_headers_out = count + 1;

        if (numpkts)
            rv = soc_sbx_caladan3_ppe_hd(unit, parsed, ppe_headers);
        if (SOC_SUCCESS(rv) && (npedin || npedout))
            rv = soc_sbx_caladan3_ped_hd(unit, pedin, pedout, parsed, ped_headers_in, ped_headers_out);

        *tx_flags = MDE_AGENT_MSG_FLAG_LAST_MSG | MDE_AGENT_MSG_FLAG_ACK;
    }

    return rv;
}



/*
 * Function:
 *     mde_agent_process_header_read
 * Purpose:
 *     Process a header read message
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_header_read(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;

    *tx_length = 0;
    *tx_data = NULL;
    *tx_flags = 0;

    /*
     * Data should consist of:
     */
    if (rx_length != 0) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));
        rv = BCM_E_PARAM;
    } else {

        /*
         * Nothing to unpack. tx_data is allocated by the *get_header() call
         */
        rv = soc_sbx_caladan3_ucode_debug_get_header(unit, tx_data);
        if (SOC_SUCCESS(rv)) {
            *tx_flags = MDE_AGENT_MSG_FLAG_LAST_MSG | MDE_AGENT_MSG_FLAG_ACK;
            *tx_length = MDE_AGENT_HEADER_SIZE;
        }
    }

    return rv;
}


/*
 * Function:
 *     mde_agent_process_header_write
 * Purpose:
 *     Process a header write message
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_header_write(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;
    uint32 header[MDE_AGENT_HEADER_SIZE/4];
    int i;

    *tx_length = 0;
    *tx_data = NULL;
    *tx_flags = 0;

    /*
     * Data should consist of:
     */
    if (rx_length != MDE_AGENT_HEADER_SIZE) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));
        rv = BCM_E_PARAM;
    } else {
        /*
         * Convert to local endian
         */
        for (i = 0;i < 64;i++) {
            header[i] = ntohl(*((uint32*)rx_data));
            rx_data += sizeof(uint32);
        }

        rv = soc_sbx_caladan3_ucode_debug_set_header(unit, header);
        if (SOC_SUCCESS(rv)) {
            *tx_flags = MDE_AGENT_MSG_FLAG_LAST_MSG | MDE_AGENT_MSG_FLAG_ACK;
            *tx_length = 0;
        }
    }

    return rv;
}


/*
 * Function:
 *     mde_agent_process_step
 * Purpose:
 *     Process a debug step message
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_step(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;
    int s, i;
    int count = 0;
    uint8 inst[12];
    uint32 *ptr;

    *tx_length = 0;
    *tx_data = NULL;
    *tx_flags = 0;

    /*
     * Data should consist of:
     */
    if (rx_length != 0) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));

        rv = BCM_E_PARAM;
    } else {
        rv = soc_sbx_caladan3_ucode_debug_next(unit);

        if (rv == BCM_E_NONE) {
            do {
                rv = soc_sbx_caladan3_ucode_debug_get_current_frame(unit, 10000,
                                                                    &s, &i, inst);
                count++;
                sal_udelay(1000);
            } while (SOC_FAILURE(rv) && (count < 10));

            if (SOC_SUCCESS(rv)) {
                *tx_length = sizeof(uint32) * 2;
                *tx_data = (uint8*)sal_alloc(*tx_length, "tx_data step");
                ptr = (uint32*)*tx_data;
                *ptr = htonl(s);
                ptr++;
                *ptr = htonl(i);
                *tx_flags = MDE_AGENT_MSG_FLAG_ACK;
            } else {
                cli_out("Timeout waiting for breakpoint to be hit\n");
            }
        }

        *tx_flags |= MDE_AGENT_MSG_FLAG_LAST_MSG;
    }

    return rv;
}


/*
 * Function:
 *     mde_agent_process_continue
 * Purpose:
 *     Process a debug continue message
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_continue(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;

    *tx_length = 0;
    *tx_data = NULL;

    /*
     * Data should consist of:
     */
    if (rx_length != 0) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));

        rv = BCM_E_PARAM;
    } else {

        rv = soc_sbx_caladan3_ucode_debug_continue(unit);

        if (rv == BCM_E_NONE)
            *tx_flags = MDE_AGENT_MSG_FLAG_ACK;
        else
            *tx_flags = 0;

        *tx_flags |= MDE_AGENT_MSG_FLAG_LAST_MSG;
        *tx_data = NULL;
        *tx_length = 0;
    }


    return rv;
}


/*
 * Function:
 *     mde_agent_process_wp_set
 * Purpose:
 *     Process a watchpoint set message
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_wp_set(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;

    *tx_length = 0;
    *tx_data = NULL;

    /*
     * Data should consist of:
     *
     * Not yet supported
     */
    if (rx_length != 0) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));

        rv = BCM_E_PARAM;
    } else {
    }

    return rv;
}

/*
 * Function:
 *     mde_agent_process_wp_clear
 * Purpose:
 *     Process a watchpoint clear message
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_wp_clear(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;

    *tx_length = 0;
    *tx_data = NULL;

    /*
     * Data should consist of:
     *
     * Not yet supported
     */
    if (rx_length != 0) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));
        rv = BCM_E_PARAM;
    } else {

    }

    return rv;
}


/*
 * Function:
 *     mde_agent_process_ocm_read
 * Purpose:
 *     Process a ocm read message
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_ocm_read(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;
    uint32 port;
    uint32 index_min;
    uint32 index_max;
    uint32 segment;
    uint32 *data;
    uint32 *ptr;
    uint32 *dp;
    int i;
    uint32 size;

    *tx_length = 0;
    *tx_data = NULL;

    /*
     * Data should consist of:
     * uint32 port
     * uint32 address
     * uint32 count
     */
    if (rx_length < sizeof(uint32) * 3) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));
        rv = BCM_E_PARAM;
    } else {

        port = ntohl(*(uint32*)rx_data);
        rx_data += sizeof(uint32);
        segment = ntohl(*(uint32*)rx_data);
        rx_data += sizeof(uint32);
        index_min = ntohl(*(uint32*)rx_data);
        rx_data += sizeof(uint32);
        index_max = ntohl(*(uint32*)rx_data);
        rx_data += sizeof(uint32);
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "MDE_AGENT-Socket: OCM read port:%d  segment:%d  index_min:%d  index_max:%d\n"),
                   port,
                   segment,
                   index_min,
                   index_max));

        if (index_min > index_max) {
            rv = BCM_E_PARAM;
        } else {
            size = (index_max - index_min) + 1;
            data = (uint32*)soc_cm_salloc(unit, sizeof(uint32) * size * 3, "ocm data");
            dp = data;


            rv = soc_sbx_caladan3_ocm_port_mem_read(unit,
                                                    port,
                                                    segment,
                                                    index_min,
                                                    index_max,
                                                    data);
            if (SOC_SUCCESS(rv)) {
                soc_sbx_caladan3_cmic_endian((uint8*)data, 8);
                rv = BCM_E_NONE;
                *tx_length = sizeof(uint32) + (sizeof(uint32) * 2 * size);
                *tx_data = sal_alloc(*tx_length, "ocm data");
                ptr = (uint32*)*tx_data;
                *ptr = htonl(size);
                ptr++;
                for (i = 0;i < size;i++) {
                    LOG_ERROR(BSL_LS_APPL_COMMON,
                              (BSL_META_U(unit,
                                          "MDE_AGENT-Socket: read data: 0x%08x %08x\n"),
                               *data, *(data + 1)));
                    *ptr = htonl(*data);
                    ptr++;
                    data++;
                    *ptr = htonl(*data);
                    ptr++;
                    data++;
                }

            } else {
                rv = BCM_E_PARAM;
                LOG_ERROR(BSL_LS_APPL_COMMON,
                          (BSL_META_U(unit,
                                      "MDE_AGENT-Socket: OCM read failed\n")));
            }

            soc_cm_sfree(unit, dp);
        }
    }

    return rv;
}


/*
 * Function:
 *     mde_agent_process_ocm_write
 * Purpose:
 *     Process a ocm write message
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_ocm_write(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;
    uint32 port;
    uint32 segment;
    uint32 index_min;
    uint32 index_max;
    uint32 *data;
    uint32 *dp;
    int i;
    uint32 size;

    *tx_length = 0;
    *tx_data = NULL;

    /*
     * Data should consist of:
     * uint32 port
     * uint32 address
     * uint32 count
     * uint8  data[count]
     */
    if (rx_length < sizeof(uint32) * 3) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));
        rv = BCM_E_PARAM;
    } else {
        port = ntohl(*(uint32*)rx_data);
        rx_data += sizeof(uint32);
        segment = ntohl(*(uint32*)rx_data);
        rx_data += sizeof(uint32);
        index_min = ntohl(*(uint32*)rx_data);
        rx_data += sizeof(uint32);
        index_max = ntohl(*(uint32*)rx_data);
        rx_data += sizeof(uint32);
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "MDE_AGENT-Socket: OCM write port:%d  segment:%d  index_min:0x%d  index_max:%d\n"),
                   port,
                   segment,
                   index_min,
                   index_max));

        if (index_min > index_max) {
            rv = BCM_E_PARAM;
        } else {
            size = (index_max - index_min) + 1;
            data = soc_cm_salloc(unit, sizeof(uint32) * size * 2, "ocm data");
            dp = data;

            for (i = 0;i < size;i++) {
                *data = ntohl(*(uint32*)rx_data);
                rx_data += sizeof(uint32);
                data++;
                *data = ntohl(*(uint32*)rx_data);
                LOG_ERROR(BSL_LS_APPL_COMMON,
                          (BSL_META_U(unit,
                                      "MDE_AGENT-Socket: write data: 0x%08x %08x\n"),
                           *(data - 1), *data));
                rx_data += sizeof(uint32);
                data++;
            }

            soc_sbx_caladan3_cmic_endian((uint8*)dp, 8);
            rv = soc_sbx_caladan3_ocm_port_mem_write(unit,
                                                port,
                                                segment,
                                                index_min,
                                                index_max,
                                                dp);


            if (SOC_SUCCESS(rv)) {
                rv = BCM_E_NONE;
            } else {
                rv = BCM_E_PARAM;
                LOG_ERROR(BSL_LS_APPL_COMMON,
                          (BSL_META_U(unit,
                                      "MDE_AGENT-Socket: OCM write failed\n")));
            }

            soc_cm_sfree(unit, dp);
        }
    }

    return rv;
}


/*
 * Function:
 *     mde_agent_process_ucode_update
 * Purpose:
 *     Process a ucode update message
 * Parameters:
 *     data      - Pointer to message data
 *     length    - Length of message adta in bytes
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */
STATIC int mde_agent_process_ucode_update(int unit, uint8 *rx_data, uint32 rx_length, uint16 *tx_flags, uint8 **tx_data, uint32 *tx_length) {
    int rv = BCM_E_NONE;
    uint32 count;
    int blen;
    unsigned char *bbuf=NULL;
    soc_sbx_caladan3_ucode_pkg_t *pi = NULL;


    *tx_length = 0;
    *tx_data = NULL;
    *tx_flags = 0;

    /*
     * Data should consist of:
     * uint32 count
     * uint32 register[count]
     * uint32 value[count]
     */
    if (rx_length < sizeof(uint32)) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Unexpected data length:%d\n"), rx_length));
        rv = BCM_E_PARAM;
    } else {

        /*
         * Remove existing breakpoints
         */
        soc_sbx_caladan3_ucode_debug_bp_clear_all(unit);
        soc_sbx_caladan3_ucode_debug_clr_session(unit);

        count = ntohl(*(uint32*)rx_data);
        rx_data += sizeof(uint32);
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "MDE_AGENT-Socket: Ucode update count:%d\n"), count));

        /*
         * Binary length
         */
        blen = count/2;

        /*
         * allocate binary buffer
         */
        bbuf = sal_alloc(blen, "ucode_buffer");
        if (!bbuf) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "system error: out of memory\n")));
            return SOC_E_RESOURCE;
        }

        /*
         * convert ascii buffer into the binary buffer
         */
        C3Asm3__s2b(bbuf, blen, rx_data, count);

        /*
         * allocate package interface structure
         */
        pi = sal_alloc(sizeof(soc_sbx_caladan3_ucode_pkg_t), "ucode_pkg");
        if(!pi) {
            sal_free(bbuf);
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "system error: out of memory\n")));
            return SOC_E_RESOURCE;
        }

        /*
         * initialize package interface structure
         */
        C3Asm3__PkgInt__init(pi);

        /*
         * read binary buffer into the interface structture
         */
        C3Asm3__PkgInt__readBuf(pi, bbuf, blen);

        /*
         * free binary buffer
         */
        sal_free(bbuf);

        /*
         * download ucode here
         */
        rv = soc_sbx_caladan3_ucodemgr_loadimg_from_pkg(unit, pi, 0, 0);

        if (SOC_SUCCESS(rv)) {
            *tx_flags = MDE_AGENT_MSG_FLAG_ACK;
            LOG_VERBOSE(BSL_LS_APPL_COMMON,
                        (BSL_META_U(unit,
                                    "G3Util Reload Microcode - complete\n")));
        } else {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META_U(unit,
                                  "G3Util Reload Microcode - error rv:%x\n"), rv));
        }

        /*
         * free package
         */
        C3Asm3__PkgInt__destroy(pi);

    }

    *tx_flags |= MDE_AGENT_MSG_FLAG_LAST_MSG;
    return rv;
}



/*
 * Function:
 *     mde_agent_ip_to_str
 * Purpose:
 *     Converts an IPv4 address from bcm_ip_t to string format.
 * Parameters:
 *     str       - (OUT) IP address in string format
 *     ip        - IP address in bcm_ip_t format
 * Returns:
 *     BCM_E_NONE    Success
 *     BCM_E_XXX     Failure
 */

STATIC int
mde_agent_ip_to_str(char *str, bcm_ip_t ip)
{
    sal_sprintf(str, "%d.%d.%d.%d",
                (ip >> 24) & 0xff, (ip >> 16) & 0xff,
                (ip >> 8) & 0xff, ip & 0xff);

    return BCM_E_NONE;
}


/*
 * Function:
 *     mde_agent_read_n
 * Purpose:
 *     Read 'n' bytes from the given file descriptor.
 * Parameters:
 *     fd        - File descriptor to read from
 *     buffer    - (OUT) Buffer where data read is returned
 *     len       - Number of bytes to read
 * Returns:
 *     > 0   Number of actual bytes read
 *       0   EOF, received FIN
 *     (-1)  On read error
 * Notes:
 *     1. The routine does not return until the specified number of bytes
 *        has been read, or, an EOF or read error has ocurred.
 *     2. If actual bytes read is less than expected, an EOF
 *        was also received.
 */

STATIC int
mde_agent_read_n(int fd, void *buffer, int len)
{
    int        n_left;    /* Number of bytes left to read */
    int        n_read;    /* Number of bytes read */
    char       *ptr;

    ptr = buffer;
    n_left = len;

    while (n_left > 0) {
        if ((n_read = read(fd, ptr, n_left)) < 0) {
            return (-1);
        } else if (n_read == 0) {    /* EOF */
            break;
        }

        n_left -= n_read;
        ptr    += n_read;
    }

    return (len - n_left);
}


/*
 * Function:
 *     mde_agent_write_n
 * Purpose:
 *     Write 'n' bytes to the given file descriptor.
 * Parameters:
 *     fd        - File descriptor to write to
 *     buffer    - Buffer containing data
 *     len       - Number of bytes in buffer to be written
 * Returns:
 *     >= 0  Number of actual bytes written
 *     (-1)  On write error
 * Notes:
 *     The routine does not return until the specified number of bytes
 *     has been written or a write error has ocurred.
 */

STATIC int
mde_agent_write_n(int fd, void *buffer, int len)
{
    int         n_left;       /* Number of bytes left to write */
    int         n_written;    /* Number of bytes written */
    char        *ptr;

    ptr = (char *)buffer;
    n_left = len;

#if 0
    if (len > 2140 || n_left > 2140)
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("%s: Bad length:%d\n"), FUNCTION_NAME(), n_left));
#endif

    while (n_left > 0) {
        if ((n_written = write(fd, ptr, n_left)) <= 0) {
            return (-1);
        }

        n_left -= n_written;
        ptr    += n_written;
    }

    return len;
}


/*
 * Function:
 *     mde_agent_recv_pkt_hdr
 * Purpose:
 *     Read a MDE_AGENT-Socket packet header from the given file descriptor.
 * Parameters:
 *     fd            - File descriptor to read packet header from
 *     hdr           - (OUT) Packet header information returned
 * Returns:
 *     BCM_E_NONE  Success, received packet header
 *     BCM_E_XXX   Failure, on read error
 */

STATIC int
mde_agent_recv_pkt_hdr(int fd, mde_agent_msg_pkt_hdr_t *hdr)
{
    int          rv = BCM_E_FAIL;
    mde_agent_msg_pkt_hdr_t hdr_buffer;
    int          n_recv;


    /* Read packet header buffer */
    n_recv = mde_agent_read_n(fd, (void *)&hdr_buffer, sizeof(hdr_buffer));

    if (n_recv == sizeof(hdr_buffer)) {
        /* Receive complete header */
        /* Unpack Packet Header */
        hdr->version  = hdr_buffer.version;
        hdr->flags    = ntohs(hdr_buffer.flags);
        hdr->module   = hdr_buffer.module;
        hdr->type     = hdr_buffer.type;
        sal_memcpy(hdr->reserved, hdr_buffer.reserved, 3);
        hdr->device = ntohl(hdr_buffer.device);
        hdr->sequence_number = ntohl(hdr_buffer.sequence_number);
        hdr->length = ntohl(hdr_buffer.length);

        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META("MDE_AGENT-Socket: Receive packet header on socket %d:\n\t" \
                              "Version:%d\n\tFlags:0x%02x\n\tModule:%d\n\tType:%d\n\t" \
                              "Device:%d\n\tSeqNum:%d\n\tLength:%d\n"),
                     fd,
                     hdr->version,
                     hdr->flags,
                     hdr->module,
                     hdr->type,
                     hdr->device,
                     hdr->sequence_number,
                     hdr->length));


        rv = BCM_E_NONE;
    } else if (n_recv == 0) {    /* EOF */
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META("MDE_AGENT-Socket: Connection socket %d received FIN\n"), fd));
    } else if (n_recv < 0) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("MDE_AGENT-Socket ERROR: System error\n")));
    } else {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("MDE_AGENT-Socket ERROR: Cannot read packet header\n")));
    }

    return rv;
}


/*
 * Function:
 *     mde_agent_recv_pkt_data
 * Purpose:
 *     Read packet data from the given file descriptor.
 * Parameters:
 *     fd            - File descriptor to read data from
 *     buffer        - (OUT) Buffer where data is returned;
 *                     buffer size must be at least 'n_expected' bytes
 *     n_expected    - Expected packet data length
 * Returns:
 *     BCM_E_NONE  Success, received expected size packet data
 *     BCM_E_XXX   Failure,
 *                 . Buffer is NULL
 *                 . Packet data returned is smaller than expected
 *                 . Read error
 */

STATIC int
mde_agent_recv_pkt_data(int fd, uint8 *buffer, int n_expected)
{
    int          rv = BCM_E_FAIL;
    int          n_recv;

    /* Read packet data */
    n_recv = mde_agent_read_n(fd, buffer, n_expected);

    if (n_expected == n_recv) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META("MDE_AGENT-Socket: Receive packet data on socket %d, "
                              "length %d\n"), fd, n_expected));
        rv = BCM_E_NONE;
    } else if (n_recv == 0) {    /* EOF */
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META("MDE_AGENT-Socket: Connection socket %d received FIN\n"), fd));
    } else if (n_recv < 0) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("MDE_AGENT-Socket ERROR: System error\n")));
    } else {
        /* Length of data read is not what was expected */
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("MDE_AGENT-Socket ERROR: Expected packet data length %d, received %d bytes\n"), n_expected, n_recv));
    }

    return rv;
}


/*
 * Function:
 *     mde_agent_send_pkt_data
 * Purpose:
 *     Write data to given file descriptor.
 * Parameters:
 *     client_id    - Client id where to send data to
 *     fd           - File descriptor to write data to
 *     buffer       - Buffer containing packet data to be sent
 *     data_len     - Number of data bytes in 'buffer' to send (payload)
 * Returns:
 *     BCM_E_NONE  Success, specified number of bytes were written
 *     BCM_E_XXX   Failure,
 *                 . Buffer is NULL
 *                 . Write error
 */
STATIC int
mde_agent_send_pkt_data(int fd, mde_agent_msg_pkt_hdr_t *hdr, uint8 *buffer, uint32 tx_length)
{
    int            n_wrote;
    int            hdr_size;

/*
    LOG_ERROR(BSL_LS_APPL_COMMON,
              (BSL_META("%s:  tx_length:%d  hdr_len:%d\n"),
               FUNCTION_NAME(),
               tx_length,
               ntohl(hdr->length)));
*/
    /*
     * Send packet header
     */
    hdr_size = sizeof(mde_agent_msg_pkt_hdr_t);
    n_wrote = mde_agent_write_n(fd, (void *)hdr, hdr_size);
    if (n_wrote < hdr_size) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("MDE_AGENT-Socket ERROR: Cannot send packet header\n")));
        return BCM_E_FAIL;
    }

    /*
     * Send packet data
     */
    if (tx_length > 0 && buffer != NULL) {
        n_wrote = mde_agent_write_n(fd, buffer, tx_length);
        if (n_wrote < tx_length) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META("MDE_AGENT-Socket ERROR: Cannot send packet data, payload length %d, sent %d\n"), tx_length, n_wrote));
            return BCM_E_FAIL;
        }
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *     mde_agent_rx_conn_thread
 * Purpose:
 *     Thread that processes incoming packets for a given connection.
 *
 *     It expects an MDE_AGENT-Socket packet header and the correspoding packet data.
 *     Once the payload is received, it gives the packet data to the upper
 *     layer transport protocol MDE_AGENT to handle it.  It then proceeds to wait
 *     for another MDE_AGENT-Socket packet to arrive.
 *
 *     Thread terminates on,
 *     . Read error
 *     . Closed connection socket
 *     . ABORT flag set
 *
 * Parameters:
 *     cookie      - Pointer to DB entry for RX thread
 * Returns:
 *     None
 */

STATIC void
mde_agent_rx_conn_thread(void *cookie)
{
    int                    rv = BCM_E_FAIL;
    mde_agent_msg_pkt_hdr_t hdr;
    mde_agent_msg_pkt_hdr_t tx_hdr;
    uint8                  *buffer = NULL;
    int                    data_len = 0;
    int                    rx_sock;
    mde_agent_sock_conn_entry_t  *conn_ptr;
    int                    lock_set = FALSE;
    uint16 tx_flags = 0;
    uint8  *tx_data;
    uint32 tx_length;
    uint32 tx_response_code = BCM_E_NONE;
    fd_set socks;
    struct timeval timeout;
    int readsocks;
#if 0
    asm_debug_status_t temp_state;
#endif
    debugmgr_t *mgr;

    conn_ptr = (mde_agent_sock_conn_entry_t*)cookie;

    rx_sock = conn_ptr->sock_fd;
    mgr = &_dbgmgr[conn_ptr->unit];

    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                (BSL_META("MDE_AGENT-Socket: Start RX Connection Thread, socket %d\n"),
                 rx_sock));

    /*
     * Send status on connect
     */
    mde_agent_create_system_status(conn_ptr->unit, &tx_hdr, &tx_data, &tx_length);

    /*
     * Send status
     */
    rv = mde_agent_send_pkt_data(rx_sock, &tx_hdr, (uint8*)tx_data, tx_length);

    if (tx_hdr.length > 0 && tx_data != NULL) {
        sal_free(tx_data);
    }

    if (rv != BCM_E_NONE)
        sal_thread_exit(rv);


    /* Main loop to service incoming data for the connection */
    while (!(conn_ptr->flags & MDE_AGENT_SOCK_CONN_ABORT)) {

        /*
         * Add rx socket to select sock rx list
         */
        FD_ZERO(&socks);
        FD_SET(rx_sock,&socks);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        readsocks = select(rx_sock + 1,
                           &socks,
                           (fd_set *) 0,
                           (fd_set *) 0,
                           &timeout);

        if (readsocks < 0) {
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META("MDE_AGENT-Socket: select")));
            rv = BCM_E_MEMORY;
            break;
        } else if (readsocks == 0) {
            /*
             * Check debug state
             */
#if 0
            if (soc_sbx_caladan3_ucode_debug_pull_session(conn_ptr->unit) == SOC_E_NONE) {
                if (mgr->session_active) {
                    temp_state = MDE_AGENT_DEBUG_STATUS_DEBUG;
                } else {
                    temp_state = MDE_AGENT_DEBUG_STATUS_RUNNING;
                }

                if (temp_state != mde_agent_debug_state) {
                    mde_agent_debug_state = temp_state;

                    /*
                     * Send state update to client
                     */
                    mde_agent_send_debug_state(conn_ptr->unit, rx_sock);
                }
            }
#endif
        } else {

            /* Get packet header */
            rv = mde_agent_recv_pkt_hdr(rx_sock, &hdr);
            if (BCM_FAILURE(rv) || (conn_ptr->flags & MDE_AGENT_SOCK_CONN_ABORT)) {
                break;
            }

            LOG_VERBOSE(BSL_LS_APPL_COMMON,
                        (BSL_META("MDE_AGENT-Socket: Got header\n")));

            /* Get packet data */
            if (hdr.length > 0) {
                data_len = hdr.length;

                buffer = sal_alloc(data_len, "MDE_AGENT Debug Rx Buffer");
                if (buffer == NULL) {
                    LOG_ERROR(BSL_LS_APPL_COMMON,
                              (BSL_META("MDE_AGENT-Socket ERROR: Cannot allocate receive "
                                        "buffer (%d bytes)\n"), hdr.length));
                    rv = BCM_E_MEMORY;
                    break;
                }

                rv = mde_agent_recv_pkt_data(rx_sock, buffer, data_len);
                if (BCM_FAILURE(rv) || (conn_ptr->flags & MDE_AGENT_SOCK_CONN_ABORT)) {
                    sal_free(buffer);
                    break;
                }
            } else {
                data_len = 0;
            }

            switch (hdr.module) {
                case MDE_AGENT_MODULE_LRP:
                    /*
                     * Process message by type
                     */
                    switch (hdr.type) {
                        case MDE_AGENT_MSG_T_NULL:
                            break;

                        case MDE_AGENT_MSG_T_DEBUG_BREAKPOINT_SET:
                            tx_response_code = mde_agent_process_bp_set(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        case MDE_AGENT_MSG_T_DEBUG_BREAKPOINT_CLEAR:
                            tx_response_code = mde_agent_process_bp_clear(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        case MDE_AGENT_MSG_T_DEBUG_BREAKPOINT_CLEAR_ALL:
                            tx_response_code = mde_agent_process_bp_clear_all(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        case MDE_AGENT_MSG_T_DEBUG_BREAKPOINT_LIST:
                            tx_response_code = mde_agent_process_bp_list(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        case MDE_AGENT_MSG_T_DEBUG_REG_READ:
                            tx_response_code = mde_agent_process_reg_read(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        case MDE_AGENT_MSG_T_DEBUG_REG_WRITE:
                            tx_response_code = mde_agent_process_reg_write(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        case MDE_AGENT_MSG_T_DEBUG_HEADER_READ:
                            tx_response_code = mde_agent_process_header_read(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        case MDE_AGENT_MSG_T_DEBUG_HEADER_WRITE:
                            tx_response_code = mde_agent_process_header_write(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        case MDE_AGENT_MSG_T_DEBUG_STEP:
                            tx_response_code = mde_agent_process_step(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        case MDE_AGENT_MSG_T_DEBUG_CONTINUE:
                            tx_response_code = mde_agent_process_continue(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        case MDE_AGENT_MSG_T_DEBUG_WP_SET:
                            tx_response_code = mde_agent_process_wp_set(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        case MDE_AGENT_MSG_T_DEBUG_WP_CLEAR:
                            tx_response_code = mde_agent_process_wp_clear(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        case MDE_AGENT_MSG_T_DEBUG_STATE:
                            tx_response_code = mde_agent_process_state(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        default:
                            LOG_ERROR(BSL_LS_APPL_COMMON,
                                      (BSL_META("MDE_AGENT-Socket ERROR: Unknown message type:%d\n"),hdr.type));
                            break;
                    }
                    break;

                case MDE_AGENT_MODULE_UCODE:
                    switch (hdr.type) {
                        case MDE_AGENT_MSG_T_UCODE_UPDATE:
                            tx_response_code = mde_agent_process_ucode_update(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        case MDE_AGENT_MSG_T_UCODE_SYMBOL_GET:
                            tx_response_code = mde_agent_process_ucode_symbol_get(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        case MDE_AGENT_MSG_T_UCODE_SYMBOL_SET:
                            tx_response_code = mde_agent_process_ucode_symbol_set(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        default:
                            LOG_ERROR(BSL_LS_APPL_COMMON,
                                      (BSL_META("MDE_AGENT-Socket ERROR: Unknown message type:%d\n"),hdr.type));
                            break;
                    }
                    break;

                case MDE_AGENT_MODULE_SYSTEM:
                    switch (hdr.type) {
                        case MDE_AGENT_MSG_T_SYSTEM_START:
                            break;

                        case MDE_AGENT_MSG_T_SYSTEM_STOP:
                            break;

                        case MDE_AGENT_MSG_T_SYSTEM_TX_PACKET:
                            tx_response_code = mde_agent_process_tx_packet(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        case MDE_AGENT_MSG_T_SYSTEM_HEADER_CAPTURE:
                            tx_response_code = mde_agent_process_header_capture(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        case MDE_AGENT_MSG_T_SYSTEM_HEADER_DUMP:
                            tx_response_code = mde_agent_process_header_dump(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        default:
                            break;
                    }
                    break;

                case MDE_AGENT_MODULE_OCM:
                    switch (hdr.type) {
                        case MDE_AGENT_MSG_T_OCM_READ:
                            tx_response_code = mde_agent_process_ocm_read(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        case MDE_AGENT_MSG_T_OCM_WRITE:
                            tx_response_code = mde_agent_process_ocm_write(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            break;

                        case MDE_AGENT_MSG_T_OCM_TABLE_INFO_GET:
#ifdef BCM_CALADAN3_G3P1_SUPPORT
                            if (SOC_IS_SBX_G3P1(conn_ptr->unit)) {
                                tx_response_code = mde_agent_g3p1_ocm_table_info_get(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            }
#endif
#ifdef BCM_CALADAN3_T3P1_SUPPORT
                            if (SOC_IS_SBX_T3P1(conn_ptr->unit)) {
                                tx_response_code = mde_agent_t3p1_ocm_table_info_get(conn_ptr->unit, buffer, data_len, &tx_flags, &tx_data, &tx_length);
                            }
#endif
                        break;

                        default:
                            break;
                    }
                    break;

                default:
                    LOG_ERROR(BSL_LS_APPL_COMMON,
                              (BSL_META("MDE_AGENT-Socket ERROR: Unknown module type:%d\n"),hdr.module));
                    break;
            }

            /*
             * Set ACK flag
             */
            if (tx_response_code == BCM_E_NONE)
                tx_flags |= MDE_AGENT_MSG_FLAG_ACK;
            else
                tx_flags &= ~MDE_AGENT_MSG_FLAG_ACK;

            tx_flags |= MDE_AGENT_MSG_FLAG_RESPONSE;

#if 0
            if (tx_flags & MDE_AGENT_MSG_FLAG_ACK)
                LOG_ERROR(BSL_LS_APPL_COMMON,
                          (BSL_META("MDE_AGENT Positive response\n")));
            else
                LOG_ERROR(BSL_LS_APPL_COMMON,
                          (BSL_META("MDE_AGENT Negative response\n")));
#endif
            /*
             * Set debug state flags
             */
            if (mgr->session_active)
                tx_flags |= MDE_AGENT_MSG_FLAG_STATE_DEBUG | MDE_AGENT_MSG_FLAG_ACTIVE;
            else
                tx_flags |= MDE_AGENT_MSG_FLAG_STATE_RUNNING;

            if (mgr->context_valid)
                tx_flags |= MDE_AGENT_MSG_FLAG_CONTEXT_VALID;

            if (mgr->direction)
                tx_flags |= MDE_AGENT_MSG_FLAG_INGRESS;

#if 0
            LOG_ERROR(BSL_LS_APPL_COMMON,
                      (BSL_META("Flags:0x%08x\n"), tx_flags));
#endif

            /*
             * Create message response
             */
            tx_hdr.version = MDE_AGENT_VERSION;
            tx_hdr.flags   = htons(tx_flags);
            tx_hdr.module  = hdr.module;
            tx_hdr.type    = hdr.type;
            tx_hdr.reserved[0] = 0;
            tx_hdr.reserved[1] = 0;
            tx_hdr.reserved[2] = 0;
            tx_hdr.device = htonl(MDE_AGENT_C3_DEVICE);
            tx_hdr.response_code = htonl(tx_response_code);
            tx_hdr.sequence_number = htonl(hdr.sequence_number);
            tx_hdr.length = htonl(tx_length);

            /*
             * Send response
             */
            rv = mde_agent_send_pkt_data(conn_ptr->sock_fd, &tx_hdr, (uint8*)tx_data, tx_length);

            if (tx_length > 0 && tx_data != NULL) {
                sal_free(tx_data);
            }

            if(buffer != NULL) {
                sal_free(buffer);
                buffer = NULL;
            }
            if (rv != BCM_E_NONE)
                break;
        }
    }

    LOG_ERROR(BSL_LS_APPL_COMMON,
              (BSL_META("MDE_AGENT-Rx thread: Terminating...\n")));
    close(rx_sock);

    /* Cleanup connection entry */

    /*
     * Get lock only if ABORT flag is not set
     *
     * Note:
     *   The routine that set the ABORT flag holds the lock
     *   and waits for this thread to exit.  If this
     *   thread tries to get the lock, the routine waiting
     *   for this thread to exit will timeout with an error.
     */
    if (!(conn_ptr->flags & MDE_AGENT_SOCK_CONN_ABORT)) {
        MDE_AGENT_SOCK_LOCK;
        lock_set = TRUE;
    }

    conn_ptr->sock_fd = -1;
    conn_ptr->rx_thread_name[0] = '\0';
    conn_ptr->rx_thread_id = SAL_THREAD_ERROR;
    conn_ptr->flags = MDE_AGENT_SOCK_CONN_CLEAR;

    if (lock_set) {
        MDE_AGENT_SOCK_UNLOCK;
    }

    LOG_ERROR(BSL_LS_APPL_COMMON,
              (BSL_META("MDE_AGENT-Rx thread: Terminated\n")));

    sal_thread_exit(rv);

    return;
}



/*
 * Function:
 *     mde_agent_conn_create
 * Purpose:
 *     It creates a connection as follows:
 *     - Checks that DB entry does not have a valid connection,
 *       if so, return failure
 *     - Creates an RX connection thread
 *     - Sets MDE_AGENT override to use sockets
 *     - Updates the connection information on corresponding DB entry
 * Parameters:
 *     db_ptr      - Pointer to DB entry to create connection for
 *     sock_fd     - Connection socket
 * Returns:
 *     BCM_E_NONE  Success
 *     BCM_E_XXX   Failure,
 *                 . DB entry contains a valid connection
 *                 . Cannot create thread
 * Note:
 *     Assumes LOCK is held by caller
 */

STATIC int
mde_agent_conn_create(int sock_fd)
{
    char                   thread_name[MDE_AGENT_SOCK_THREAD_NAME_LEN];
    sal_thread_t           thread_id = SAL_THREAD_ERROR;
    const int              sock_opt_on = 1;


    /*
     * Set socket option to disable Nagle algorithm.
     * The purpose is to increase performance.
     *
     * Note: The Nagle algorithm prevents a connection from
     * having multiple outstanding packets at any time.
     * It waits for the 'ack' to come back before it sends
     * any packet.
     */
   if (setsockopt(sock_fd, IPPROTO_TCP, TCP_NODELAY,
                  (char *)&sock_opt_on, sizeof(sock_opt_on))) {
       LOG_ERROR(BSL_LS_APPL_COMMON,
                 (BSL_META("mde_agent_conn_create: setsockopt failed\n")));
    }

    /* Create RX connection thread */
    sal_snprintf(thread_name, MDE_AGENT_SOCK_THREAD_NAME_LEN,
                 "%s%d", MDE_AGENT_SOCK_CONN_NAME, sock_fd);

    if (mde_agent_mde.flags & MDE_AGENT_SOCK_CONN_VALID) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("MDE_AGENT-Socket ERROR:  Connection already exists\n")));
        return BCM_E_EXISTS;
    }

    /* Set information */
    mde_agent_mde.flags   = MDE_AGENT_SOCK_CONN_VALID;
    mde_agent_mde.sock_fd = sock_fd;

     /* Create thread to handle specific connection request */
    sal_strcpy(mde_agent_mde.rx_thread_name, thread_name);
    thread_id = sal_thread_create(mde_agent_mde.rx_thread_name,
                                  SAL_THREAD_STKSZ,
                                  mde_agent_sock_cfg_thread_pri,
                                  mde_agent_rx_conn_thread,
                                  (void *) &mde_agent_mde);

    if (thread_id == SAL_THREAD_ERROR) {
        mde_agent_mde.flags = MDE_AGENT_SOCK_CONN_CLEAR;
        mde_agent_mde.sock_fd = -1;
        mde_agent_mde.rx_thread_name[0]='\0';
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("MDE_AGENT-Socket ERROR: Cannot create connection thread\n")));
        return BCM_E_FAIL;
    }

    mde_agent_mde.rx_thread_id = thread_id;
    mde_agent_debug_state = MDE_AGENT_DEBUG_STATUS_RUNNING;

    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                (BSL_META("MDE_AGENT-Socket: Created connection to socket %d thread '%s'\n"),
                 mde_agent_mde.sock_fd,
                 mde_agent_mde.rx_thread_name));

    return BCM_E_NONE;
}



/*
 * Function:
 *     mde_agent_accept_connection
 * Purpose:
 *     It waits for a connection request and updates the MDE_AGENT-Socket database
 *     accordingly.
 *
 *     On a incoming connection request, it searches the database
 *     list for the client IP address.  If client is not found, a
 *     new DB entry is created and added to the database list.  A
 *     connection is created for the corresponding DB entry.
 *
 *     If client already has a valid connection, this routine
 *     will return a failure.
 *
 * Parameters:
 *     listen_sock      - Port to listen to for connection requests
 * Returns:
 *     BCM_E_NONE  Success
 *     BCM_E_XXX   Failure,
 *                 . Error on listening socket
 *                 . Could not create DB entry
 *                 . Could not create connection
 *                 . A valid connection already exists
 */

STATIC int
mde_agent_accept_connection(int listen_sock)
{
    int                  rv;
    mde_agenttrans_socklen_t   addr_len;
    struct sockaddr_in   client_addr;
    char                 dest_ip_str[SAL_IPADDR_STR_LEN];
    bcm_ip_t             dest_ip;
    int                  conn_sock;     /* Connection socket */


    addr_len = (mde_agenttrans_socklen_t) sizeof(client_addr);

    rv = accept(listen_sock,
                (struct sockaddr *) &client_addr, &addr_len);

    if (rv < 0) {
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META("MDE_AGENT-Socket:  Accept returned error on socket %d "
                              "(rv=%d)\n"), listen_sock, rv));
        return BCM_E_FAIL;
    }

    /* Got a connection request, process it */
    conn_sock = rv;

    /* in_addr.s_addr always in network order; bcm_ip_t in host order */
    dest_ip = ntohl(client_addr.sin_addr.s_addr);
    mde_agent_ip_to_str(dest_ip_str, dest_ip);

    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                (BSL_META("MDE_AGENT-Socket: Connection request from client '%s' on "
                          "socket %d\n"), dest_ip_str, conn_sock));

    MDE_AGENT_SOCK_LOCK;

    /* Create connection */
    rv = mde_agent_conn_create(conn_sock);
    if (BCM_FAILURE(rv)) {
        MDE_AGENT_SOCK_UNLOCK;
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META("MDE_AGENT-Socket WARNING: Cannot create connection to '%s', "
                              "closing socket %d\n"), dest_ip_str, conn_sock));
        close(conn_sock);
        return BCM_E_NONE;
    }

    MDE_AGENT_SOCK_UNLOCK;
    close(conn_sock);

    return BCM_E_NONE;
}


/*
 * Function:
 *     mde_agent_server_conn_thread
 * Purpose:
 *     Out-Of-Band Server thread that listens for connection requests.
 *     For each connection request, it spawns a new thread that will
 *     handle incoming packets from the connection request originator.
 * Parameters:
 *     cookie      - Pointer to Server listening port
 * Returns:
 *     None
 */

STATIC void
mde_agent_server_conn_thread(void *cookie)
{
    int                 rv = 0;
    int                 listen_port = 0;
    int                 listen_sock = 0;   /* Listening socket */
    const int           sock_opt_on = 1;
    mde_agenttrans_socklen_t  addr_len;
    struct sockaddr_in  server_addr;


    listen_port = *((int *)cookie);

    /* Open socket */
    if ((rv = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("MDE_AGENT-Socket ERROR: Cannot open Server listening socket "
                            "(err=%d)\n"), rv));
        mde_agent_sock_server_thread_id = SAL_THREAD_ERROR;
        sal_thread_exit(rv);
        return;
    }

    listen_sock = rv;

    /*
     * Set socket option to allow server to restart, in case
     * there are existing connections using the listening port
     */
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR,
                   (char *)&sock_opt_on, sizeof(sock_opt_on))) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("mde_agent_server_conn_thread: setsockopt failed\n")));
    }

    /* Bind socket to listen to any IP address and to a known port */
    addr_len = (mde_agenttrans_socklen_t) sizeof(server_addr);
    sal_memset((void *)&server_addr, 0, (int)addr_len);
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(listen_port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if ((rv = bind (listen_sock,
                    (struct sockaddr *) &server_addr, addr_len)) < 0) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("MDE_AGENT-Socket ERROR: Cannot bind Server listening "
                            "socket %d (err=%d)\n"), listen_sock, rv));
        close(listen_sock);
        mde_agent_sock_server_thread_id = SAL_THREAD_ERROR;
        sal_thread_exit(rv);
        return;
    }

    /* Set the socket to listen for connection requests */
    if ((rv = listen(listen_sock, MDE_AGENT_SOCK_BACKLOG)) < 0) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("MDE_AGENT-Socket ERROR: Cannot set Server listening mode "
                            "on socket %d (err=%d)\n"), listen_sock, rv));
        close(listen_sock);
        mde_agent_sock_server_thread_id = SAL_THREAD_ERROR;
        sal_thread_exit(rv);
        return;
    }

    mde_agent_sock_server_lport = listen_port;
    mde_agent_sock_server_lsock = listen_sock;

    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                (BSL_META("MDE_AGENT-Socket: Server ready, listening on socket %d (port %d)\n"),
                 mde_agent_sock_server_lsock,
                 mde_agent_sock_server_lport));

    /* Main loop to service connection requests */
    mde_agent_sock_server_exit = FALSE;
    while (!mde_agent_sock_server_exit) {

        rv = mde_agent_accept_connection(mde_agent_sock_server_lsock);

        if (BCM_FAILURE(rv)) {
            break;
        }
    }
    close(mde_agent_sock_server_lsock);

    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                (BSL_META("MDE_AGENT-Socket: Exit Server Thread '%s', "
                          "close socket %d\n"), mde_agent_sock_server_thread_name,
                 mde_agent_sock_server_lsock));

    MDE_AGENT_SOCK_LOCK;

    mde_agent_sock_server_lsock = -1;
    mde_agent_sock_server_lport = -1;
    mde_agent_sock_server_thread_id = SAL_THREAD_ERROR;

    MDE_AGENT_SOCK_UNLOCK;

    sal_thread_exit(rv);

    return;
}


/*
 * Function:
 *     mde_agent_init
 * Purpose:
 *     It initializes the MDE_AGENT over Sockets subsystem:
 *     - If needed, destroys current connections, uninstalls, and
 *       clears the database list
 *     - Destroys current mutex lock and creates a new one
 *     - Initializes database list
 *     - Reset configuration parameters to default values
 * Parameters:
 *     None
 * Returns:
 *     BCM_E_NONE  Success
 *     BCM_E_XXX   Failure,
 *                 . Cannot create mutex lock
 */

STATIC int
mde_agent_init(void)
{
    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                (BSL_META("MDE_AGENT-Socket: Init\n")));

    /* System resources cleanup, if needed */
    if (MDE_AGENT_SOCK_INIT_DONE) {
/*        cleanup();*/
    }

    /* Mutex locks */
    if (mde_agent_sock_mlock != NULL) {
        sal_mutex_destroy(mde_agent_sock_mlock);
        mde_agent_sock_mlock = NULL;
    }
    if ((mde_agent_sock_mlock = sal_mutex_create("MDE_AGENT_Socket_mlock")) == NULL) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("MDE_AGENT-Socket ERROR: Cannot create mutex\n")));
        return BCM_E_MEMORY;
    }

    /* Reset configuration parameters to default values */
    mde_agent_sock_cfg_thread_pri = MDE_AGENT_SOCK_THREAD_PRI_DEFAULT;
    mde_agent_sock_cfg_lport = MDE_AGENT_SOCK_SERVER_LPORT_DEFAULT;


    return BCM_E_NONE;
}


/*
 * Function:
 *     mde_agent_start
 * Purpose:
 *     Initializes the MDE_AGENT over Sockets subsystem (if it has not been yet), and
 *     it creates the Server thread.
 * Parameters:
 *     None
 * Returns:
 *     BCM_E_NONE  Success, MDE_AGENT-Socket Server created and ready for connection
 *                 requests.
 *     BCM_E_XXX   Failure,
 *                 . Server thread is running
 *                 . Cannot create Server thread
 * Note:
 *     1. Only one MDE_AGENT-Socket Server can be running at a time.
 *     2. Supported only in chassis based systems.
 */

int
mde_agent_start(int unit)
{
    int rv;

    mde_agent_debug_state = MDE_AGENT_DEBUG_STATUS_IDLE;

    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                (BSL_META_U(unit,
                            "MDE_AGENT-Socket: Start Server\n")));

    if (!MDE_AGENT_SOCK_INIT_DONE) {
        rv = mde_agent_init();
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }

    MDE_AGENT_SOCK_LOCK;

    mde_agent_mde.unit = unit;

    if (MDE_AGENT_SOCK_SERVER_RUNNING) {
        MDE_AGENT_SOCK_UNLOCK;
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "MDE_AGENT-Socket: Server thread is already running\n")));
        return BCM_E_EXISTS;
    }

    /* Create Server thread */
    mde_agent_sock_server_thread_id = sal_thread_create(mde_agent_sock_server_thread_name,
                                                  SAL_THREAD_STKSZ,
                                                  mde_agent_sock_cfg_thread_pri,
                                                  mde_agent_server_conn_thread,
                                                  (void *)&mde_agent_sock_cfg_lport);

    if (mde_agent_sock_server_thread_id == SAL_THREAD_ERROR) {
        MDE_AGENT_SOCK_UNLOCK;
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "MDE_AGENT-Socket ERROR: Cannot create Server thread\n")));
        return BCM_E_FAIL;
    }

    MDE_AGENT_SOCK_UNLOCK;

    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                (BSL_META_U(unit,
                            "MDE_AGENT-Socket: Created Server Server Thread '%s' \n"),
                 mde_agent_sock_server_thread_name));

    return BCM_E_NONE;
}


/*
 * Function:
 *     mde_agent_stop
 * Purpose:
 *     It stops the MDE_AGENT-Socket Server.  It closes Server listening socket
 *     and destroys the corresponding Server thread.  The routine
 *     only returns after the Server thread has exited or has
 *     exceeded the waiting time.
 * Parameters:
 *     None
 * Returns:
 *     BCM_E_NONE  Success, Server thread exited
 *     BCM_E_XXX   Failure, Server thread did not exit in expected wait time
 * Note:
 *     Supported only in chassis based systems
 */

int
mde_agent_stop(void)
{
    int          i;

    if (!MDE_AGENT_SOCK_INIT_DONE) {
        return BCM_E_NONE;
    }

    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                (BSL_META("MDE_AGENT-Socket: Stop Server\n")));

    MDE_AGENT_SOCK_LOCK;

    if (!MDE_AGENT_SOCK_SERVER_RUNNING) {
        MDE_AGENT_SOCK_UNLOCK;
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META("MDE_AGENT-Socket: Server not running\n")));
        return BCM_E_NONE;
    }

    /* Force Server thread to exit */
    mde_agent_sock_server_exit = TRUE;
    mde_agent_mde.flags |= MDE_AGENT_SOCK_CONN_ABORT;
    shutdown(mde_agent_sock_server_lsock, SHUT_RDWR);

    MDE_AGENT_SOCK_UNLOCK;

    /* Wait for server thread to exit   */
    for (i = 0; i < MDE_AGENT_SOCK_STOP_RETRIES_MAX; i++) {
        if (!MDE_AGENT_SOCK_SERVER_RUNNING) {
            break;
        }
        sal_usleep(MDE_AGENT_SOCK_RETRY_WAIT_US);
    }

    if (i >= MDE_AGENT_SOCK_STOP_RETRIES_MAX) {
        LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("MDE_AGENT-Socket ERROR: Cannot stop Server Thread in %d ms\n"),
                   (MDE_AGENT_SOCK_STOP_RETRIES_MAX * MDE_AGENT_SOCK_RETRY_WAIT_US)/1000));
        return BCM_E_FAIL;
    }

/*    _mde_agenttrans_socket_host_stop(); */

    return BCM_E_NONE;
}



#else /* BCM_MDE_AGENT_SOCKET_SUPPORTED */

#endif /* BCM_MDE_AGENT_SOCKET_SUPPORTED */

#endif /*  BCM_CALADAN3_SUPPORT */
