/*
 * $Id: mgmt.h,v 1.21 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Arm Processor Subsystem remote management library
 */

/*
 * Warning:
 * This file is included by both uKernel and host code. Be very careful
 * about what goes into this file, esp. with regards to including other
 * files.
 */

#ifndef APPL_DIAG_APS_MGMT_H
#define APPL_DIAG_APS_MGMT_H

#define MGMT_COMMAND_MAGIC      0xa5a5a5a5
#define MGMT_REPLY_MAGIC        0x5a5a5a5a

#define MGMT_SESSION_KEY_LEN            (128/8)     /* AES 128 bits */
#define MGMT_SESSION_KEY_MSG_LEN        (2048/8)    /* pkcs encrypted session key */

/* SPI packet encapsulation */
#define MGMT_SPI_PREAMBLE               0xee
#define MGMT_SPI_PREAMBLE_LEN           16
#define MGMT_SPI_ESCAPE                 0xe0
#define MGMT_SPI_ESCAPE_ESCAPE          0xe1
#define MGMT_SPI_ESCAPE_PREAMBLE        0xe2
#define MGMT_SPI_ESCAPE_DUPLICATE       0xe3
#define MGMT_SPI_DUPLICATE_SEPARATOR    0xe4

/* SPI command codes */
#define MGMT_SPI_CMD_READY              0xa1
#define MGMT_SPI_CMD                    0xa2
#define MGMT_SPI_REPLY                  0xa3
typedef enum
{
    INVALID = 0,
    PAIR_OK,
    PAIR_OPEN,
    INTRA_PAIR_SHORT,
    PAIR_BUSY = 9
}CABLE_HEALTH_STATUS_t;

typedef enum {
    MGMT_INTF_MIRROR_INGRESS = 0,
    MGMT_INTF_MIRROR_EGRESS,
    MGMT_INTF_MIRROR_BOTH
}MGMT_INTF_MIRROR_MODE_t;

typedef enum {
    MGMT_VERSION_INFO,                  /* Retrieve version info */
    MGMT_REBOOT,                        /* Reboot request */
    MGMT_SESSION_KEY,                   /* Request new session key */
    MGMT_SET_MACADDR,
    MGMT_SET_MACSEC_KEY,
    MGMT_SWITCH_READ_REG,
    MGMT_SWITCH_WRITE_REG,
    MGMT_SWITCH_SPI_CONTROL,            /* Change SPI-MUX to point at the switch */
    MGMT_FLASH_INFO,                    /* Retrieve flash info */
    MGMT_FLASH_ERASE_SECTOR,            /* Erase the specified sector */
    MGMT_FLASH_ERASE_PAGE,              /* Erase the specified page */
    MGMT_FLASH_PAGE_READ,               /* Read a page (256 bytes) */
    MGMT_FLASH_PAGE_WRITE,              /* Write a page (256 bytes) */
    MGMT_FLASH_DUMP_CORE,               /* Write a core file to flash */
    MGMT_READ_MEMORY,                   /* Read target memory */
    MGMT_WRITE_MEMORY,                  /* Write target memory */
    MGMT_EXECUTE,                       /* Execute downloaded image */
    MGMT_RNG_DATA,                      /* Fetch RNG data */
    MGMT_CABLE_DIAG,                    /* Cable diagnostics (Polar) */
    MGMT_PERF_COUNTER,                  /* Read performance counters */
    MGMT_PING,                          /* ping the device */
    MGMT_GET_POWER_STATE,               /* set the power state */
    MGMT_SET_POWER_STATE,               /* get the power state */
    MGMT_PLL_SPREAD,                    /* set PLL spread spectrum mode */
    MGMT_SHELL_CMD,                     /* execute shell command */
    MGMT_BUFFER_INFO,                   /* request buffer info */
    MGMT_READ_BUFFER,                   /* buffer read */
    MGMT_READ_BUFFER_PROT,              /* buffer read (protected) */
    MGMT_WRITE_BUFFER,                  /* buffer write */
    MGMT_WRITE_BUFFER_PROT,             /* buffer write (protected) */
    MGMT_CERT_SESSION_KEY,              /* certificate based management key request */
    MGMT_TRANSMIT_BUFFER,               /* buffer transmit */
    MGMT_ACD,                           /* Automotive cable diagnostics - Polar only */
    MGMT_DMU,                           /* Change the DMU settings - Polar B0 only */
    /* No changes above this line - ROM code depends on these values */
    MGMT_SET_IMP_SPEED,                 /* Set IMP port speed */
    MGMT_PLAINTEXT_SESSION_KEY,         /* get session key */
    MGMT_AVB                            /* Audio Video Bridging command - Polar only */
} mgmt_cmd_t;

typedef enum {
    MGMT_AVB_PORT_INFO_GET = 0x1,
    MGMT_AVB_PORT_ADMIN_MODE_SET = 0x2,
    MGMT_AVB_PORT_SPEED_SET = 0x3,
    MGMT_AVB_PORT_MASTER_SLAVE_SET = 0x4,
    MGMT_AVB_PORT_JUMBO_MODE_SET = 0x5,
    MGMT_AVB_PORT_PHY_LB_SET = 0x6,
    MGMT_AVB_PORT_DUMBFWD_MODE_SET = 0X7,
    MGMT_AVB_PORT_DUMBFWD_MODE_GET = 0X8,
    MGMT_AVB_PORT_JUMBO_MODE_GET = 0x9,
    MGMT_AVB_PORT_ACD = 0xa,
    MGMT_AVB_PORT_INIT = 0xb,
    MGMT_AVB_PORT_TYPE_GET = 0xc,

    MGMT_AVB_DOT1AS_INFO_GET = 0x20,
    MGMT_AVB_DOT1AS_MODE_SET = 0x21,
    MGMT_AVB_DOT1AS_PRI_SET = 0x22,
    MGMT_AVB_DOT1AS_PORT_INFO_GET = 0x23,
    MGMT_AVB_DOT1AS_PORT_MODE_SET = 0x24,
    MGMT_AVB_DOT1AS_STATS_GET = 0x25,
    MGMT_AVB_DOT1AS_STATS_CLEAR = 0x26,
    MGMT_AVB_DOT1AS_INTERVAL_SET = 0x27,
    MGMT_AVB_DOT1AS_PDELAY_THRESH_SET = 0x28,
    MGMT_AVB_DOT1AS_NUM_LSTRESP_SET = 0x29,

    MGMT_AVB_VLAN_CREATE = 0x40,
    MGMT_AVB_VLAN_DELETE = 0x41,
    MGMT_AVB_VLAN_PORT_ADD = 0x42,
    MGMT_AVB_VLAN_PORT_REMOVE = 0x43,
    MGMT_AVB_VLAN_PVID_SET = 0x44,
    MGMT_AVB_VLAN_IFILTER_SET = 0x45,
    MGMT_AVB_VLAN_PRIO_SET = 0x46,
    MGMT_AVB_VLAN_GET = 0x47,
    MGMT_AVB_VLAN_PORT_GET = 0x48,

    MGMT_AVB_STREAM_ENTRY_ADD = 0x60,
    MGMT_AVB_STREAM_ENTRY_DELETE = 0x61,
    MGMT_AVB_STREAM_ENTRY_GET = 0x62,
    MGMT_AVB_STREAM_BANDWIDTH_GET = 0x63,
    MGMT_AVB_STREAM_BANDWIDTH_SET = 0x64,
    MGMT_AVB_STREAM_PCP_MAPPING_GET = 0x65,
    MGMT_AVB_STREAM_PCP_MAPPING_SET = 0x66,

    MGMT_AVB_SWITCH_MIRROR_ENABLE = 0x80,
    MGMT_AVB_SWITCH_MIRROR_DISABLE = 0x81,
    MGMT_AVB_SWITCH_AGE_TIME_SET = 0x82,
    MGMT_AVB_SWITCH_AGE_TIME_GET = 0x83,
    MGMT_AVB_SWITCH_CONFIG_SAVE = 0x84,

    MGMT_AVB_LAG_STATUS_GET = 0xa0,
    MGMT_AVB_LAG_MEMBERSHIP_SET = 0xa1,
    MGMT_AVB_LAG_HASH_SET = 0xa2,
    MGMT_AVB_LAG_HASH_GET = 0xa3,
} mgmt_avb_cmd_t;

typedef enum {
    MGMT_SUCCESS,                       /* successful operation */
    MGMT_ERROR                          /* generic error */
} mgmt_status_t;

typedef enum {
    MGMT_FLASH_INIT_MODE,               /* ROM flash-init mode, commands/replys not encrypted */
    MGMT_ENGINEERING_MODE,              /* Engineering/test mode */
    MGMT_PRODUCTION_MODE                /* Production mode */
} mgmt_mode_t;


/*
 * Power states
 */

typedef enum {
    POWER_INIT =        0,      /* Default value out of reset */
    POWER_ULTRA_LOW =   1,      /* About to power down */
    POWER_DEEP_SLEEP =  2,      /* ARM at REFCLK, interfaces off */
    POWER_SLEEP =       3,      /* REFCLK no SERDES */
    POWER_REF =         4,      /* ARM at REFCLK */
    POWER_HALF_SPEED =  5,      /* ARM at half full speed */
    POWER_FULL_SPEED =  6       /* Full speed ARM, everything on */
} power_state_t;

#define _POWER_STATE_LIMIT 7


#define _POWER_STATE_STR_INIT        { \
        "default",                     \
        "ultra",                       \
        "deep",                        \
        "sleep",                       \
        "ref",                         \
        "half",                        \
        "full",                        \
        "unknown"                      \
        }

extern char *_power_state_str[];

#define _POWER_STATE_STR(s)          \
        _power_state_str[(((int)s) <= 0 && ((int)s) >= _POWER_STATE_LIMIT) ? _POWER_STATE_LIMIT : s]

#define MGMT_REBOOT_SLOT        0x4a696d42  /* boot specific slot image */
#define MGMT_REBOOT_FLASH_INIT  0x4d61726b  /* reenter flash init mode */

#define MGMT_COMMAND_LEN        12      /* mgmt command w/ no data */
#define MGMT_STATUS_LEN         12      /* mgmt reply w/ no data */

#define MGMT_AVB_COMMAND_CODE_LEN    4       /* 4 bytes enum is 4 bytes even on 64 bit machine*/ 
#define MGMT_AVB_COMMAND_LEN    16      /* MGMT_COMMAND_LEN + MGMT_AVB_COMMAND_CODE_LEN*/

#define MGMT_NUM_ARGS           4
#define MGMT_RNG_DATA_SIZE      32      /* bytes of RNG data to return */

#define MGMT_FLASH_PAGE_SIZE    256     /* Flash page size */
#define MGMT_MEMORY_PAGE_SIZE   256     /* Memory page size */

#define MGMT_NUM_BUFFERS        4       /* Max number of buffers */
#define MGMT_BUFFER_PAGE_SIZE   256     /* Buffer page size */
#define MGMT_BUFFER_FLAG_PROT   1       /* Buffer is protected */

#define MGMT_SHELL_CMD_LEN      256     /* Max length of shell command */

typedef struct {
    uint32 avb_cmd;  /* AVB command type */
    union {
        struct {
            uint32 value;   /* Switch age time */
        } switch_age_time;
        struct {
            uint32 value;   /* Switch age time */
        } switch_mtu;
        struct {
            uint32 num;   /* Port number */
        } port_info;
        struct {
            uint32 num;   /* Port number */
            uint32 value; /* 0=Disabled, 1=Enabled */
        } port_admin_mode;
        struct {
            uint32 num;   /* Port number */
            uint32 value; /* 0=Autoneg, 10, 100, 1000 */
        } port_speed; 
        struct {
            uint32 num;   /* Port number */
            uint32 value; /* 0=slave, 1=master, 2=auto 3=none */
        } br_mode;
        struct {
            uint32 num;   /* Port number */
        } port_stat;
        struct {
            uint32 num;   /* Port number */
            uint32 value; /* 0=Disabled, 1=Enabled */
        } port_phy_lb_mode;
        struct {
            uint32 num;   /* Port number */
            uint32 value; /* 0=Disabled, 1=Enabled */
        } port_jumbo_frame_mode;
        struct { 
            uint32  value; /* 0=Disabled, 1=Enabled */
        } as_mode;
        struct {
            uint32 type;   /* 0=Priority1, 1=Priority2 */
            uint32 value;  /* 0 through 255 */
        } as_prio;
        struct {
            uint32 num;   /* Port number */
        } as_port_info;
        struct {
            uint32 num;   /* Port number */
            uint32 value; /* 0=Disabled, 1=Enabled */
        } as_port_mode;
        struct {
            uint32 num;   /* Port number */
        } as_port_stats;
        struct {
            uint32 num;   /* Port number */
            uint32 type;  /* Interval type */
            char value[20]; /* Interval value */
        } as_port_interval;
        struct {
            uint32 num;   /* Port number */
            uint32 value; 
        } as_pdelay_thresh;
        struct {
            uint32 num;   /* Port number */
            uint32 value; 
        } as_allowed_lst_resp;
        struct {
            uint32 id;   /* VLAN ID */
        } vlan_create;
        struct {
            uint32 id;   /* VLAN ID */
        } vlan_delete;
        struct {
            uint32 id;    /* VLAN ID */
            uint32 num;   /* Port number */
            uint32 is_tagged;   /* tagged or untagged */
        } vlan_port_add;
        struct {
            uint32 id;   /* VLAN ID */
            uint32 num;   /* Port number */
        } vlan_port_del;
        struct {
            uint32 num;   /* Port number */
            uint32 value; /* Value */
        } vlan_pvid;
        struct {
            uint32 num;   /* Port number */
            uint32 value; /* Value */
        } vlan_ifilter;
        struct {
            uint32 num;   /* Port number */
            uint32 value; /* Value */
        } vlan_untag_prio;
        struct {
            uint32 id;   /* VLAN ID */
        } vlan_get;
        struct {
            uint32 id;        /* VLAN ID */
            uint8  addr[6];   /* MAC address */
            uint8  pad[2];
            uint32 port_bmp;  /* Port or bitmap */
        } fdb_add;
        struct {
            uint32 id;        /* VLAN ID */
            uint8  addr[6];   /* MAC address */
        } fdb_del;
        struct {
            uint32 id;        /* VLAN ID */
            uint32 value;     /* port */
        } fdb_get;
        struct {
            uint32 num;   /* Port number */
            uint32 sr_class;  /* Stream class */
        } bandwidth_get;
        struct {
            uint32 num;   /* Port number */
            uint32 sr_class;  /* Stream Class */
            uint32 rate_kbps;    /* Rate in kpbs */
            uint32 burst_kbits;  /* Burst in kbits */
        } bandwidth_set;
        struct {
            uint32 sr_class;  /* AVB class */
        } pcp_mapping_get;
        struct {
            uint32 sr_class;   /* AVB class */
            uint32 prio;       /* Priority of class */
            uint32 remap_prio; /* Remap priority of class */
        } pcp_mapping_set;
        struct {
            uint32 bmp;        /* Mirror bitmap */
            uint32 probe_port; /* MTP port */
            uint32 mode;       /* Mirror mode - ingres/egress/both */
        } mirror_enable; 
        struct {
            uint32 lag_id; /* lag member number*/
        } lag_status_request;
        struct {
            uint32 hash_index; /* lag hash index value*/
        } hash_info_set;
        struct {
            uint32 lag_id; /* lag id*/
            uint32 port_bitmap; /* port bit map*/
        } lag_membership_set;
        struct {
            uint32 dumbfwd_mode;  /* configure dumb forwarding mode */
        } dumbfwd_config;
        struct {
            uint32      port;
            uint32      command;
        } mgmt_acd;
        struct {
            uint32      port;
        } mgmt_phy_init;
        struct {
            uint32      port;
        } mgmt_port_type;
    } ;  
} mgmt_avb_request_t;

typedef struct mgmt_command_s {
    uint32            cmd;                    /* command */
    uint32            len;                    /* Length of entire packet */
    uint32            magic;                  /* MGMT_COMMAND_MAGIC */
    union {
        struct {
            uint32    number;             /* ping number */
        } ping;
        struct {
            uint32    state;              /* power state */
        } power;
        struct {
            uint32    spread;             /* PLL spread-spectrum param */
        } spread;
        struct {
            uint32    args[MGMT_NUM_ARGS];    /* args to reboot with */
        } reboot;
        struct {
            char        cmd[MGMT_SHELL_CMD_LEN];   /* shell command to execute */
        } shell_cmd;
        struct {
            uint32    size;               /* 1, 2, 4 or 8 bytes */
            uint32    address;            /* switch reg addr */
        } switch_read;
        struct {
            uint32    size;               /* 1, 2, 4 or 8 bytes */
            uint32    address;            /* switch reg addr */
            uint32    upper_regval;       /* switch reg value, upper bits */
            uint32    lower_regval;       /* switch reg value, lower bits */
        } switch_write;
        struct {
            uint8     addr[6];            /* MAC address */
        } macaddr;
        struct {
            uint8     data[256];          /* MACSEC key */
        } macsec_key;
        struct {
            uint32    spi;                /* SPI flash # */
        } flash_info;
        struct {
            uint32    spi;                /* SPI flash # */
            uint32    address;            /* address */
        } flash_erase;
        struct {
            uint32    spi;                /* SPI flash # */
            uint32    address;            /* address */
        } flash_read;
        struct {
            uint32    spi;                        /* SPI flash # */
            uint32    address;                    /* address */
            uint8     data[MGMT_FLASH_PAGE_SIZE]; /* Flash read/write */
        } flash_write;
        struct {
            uint32    spi;                        /* SPI flash # */
        } flash_coredump;
        struct {
            uint32    address;            /* address */
        } read_memory;
        struct {
            uint32    address;            /* address */
            uint8     data[MGMT_MEMORY_PAGE_SIZE];
        } write_memory;
        struct {
            uint32    length;
            uint32    cksum;
            uint32    args[MGMT_NUM_ARGS];
            uint8     signature[MGMT_MEMORY_PAGE_SIZE];
        } execute;
        struct {
            uint32    counter;
        } perf_counter;
        struct {
            uint32      buffer;
            uint32      offset;
            uint32      length;
        } read_buffer;
        struct {
            uint32      buffer;
            uint32      offset;
            uint32      length;
            uint8       data[MGMT_BUFFER_PAGE_SIZE];
        } write_buffer;
        struct {
            uint32      buffer;
            uint32      length;
        } transmit_buffer;
        struct {
            uint32      buffer;
            uint32      length;
        } cert_session_key;
        struct {
            uint32      port;
            uint32      command;
        } acd;
        struct {
            uint32      hz;
            uint32      hclk_freq;
            uint32      hclk_sel;
            uint32      pclk_freq;
            uint32      pclk_sel;
            uint32      p1div;
            uint32      p2div;
            uint32      ndiv;
            uint32      m1div;
            uint32      m2div;
            uint32      m3div;
            uint32      m4div;
            uint32      pll_num;
            uint32      frac;
            uint32      bclk_sel;
        } dmu;
        struct {
            uint32      speed;
        } set_imp_speed;
        mgmt_avb_request_t avb;
    } u;
    char        pad[16];                                /* encryption padding */
} mgmt_command_t;


typedef struct mgmt_port_info_s {
    uint32   admin_mode;   /* Admin mode of the port - enabled/disabled */
    uint32   link_status;  /* Link status of the port - up/down */
    uint32   speed;        /* Current speed of the port - 10/100/Autoneg */
    uint32   br_mode;      /* BroadR-Reach mode - master/slave */
    uint32   jumbo_frame_mode;      /* Jumbo frmae mode - enabled/disabled*/
    uint32   loop_back_mode;        /* Loop back mode - enabled/disabled*/
    uint32   autoneg;/* Auto Neg - enabled/disabled*/
    uint32   autoneg_complete;/* Auto Neg - complete/pending*/
    uint32   duplex;/* duplex - full/half*/
} mgmt_port_info_t;

typedef  struct mgmt_dot1as_info_s {
    uint32   admin_mode;   /* Global admin mode for 802.1AS */
    uint32   local_pri1;   /* Priority1 value of this bridge */
    uint32   local_pri2;   /* Priority2 value of this bridge */
    uint32   local_clk_id_upper; /* Local clock identity */
    uint32   local_clk_id_lower; 
    uint32   gm_present;   /* Grandmaster present? */
    uint32   gm_clk_id_upper; /* Clock identify of grandmaster */
    uint32   gm_clk_id_lower;
    uint32   gm_pri1;      /* Priority1 value of grandmaster */
    uint32   gm_pri2;      /* Priority2 value of grandmaster */
    uint32   steps_to_gm;  /* Steps value */
    uint32   gm_change_count; /* Count for grandmaster changes */
} mgmt_dot1as_info_t;

typedef struct mgmt_dot1as_port_info_s {
    uint32   admin_mode;      /* Port admin mode for 802.1AS */
    uint32   dot1as_capable;  /* Is port 802.1AS capable? */
    uint32   peer_delay;      /* Current peer delay */
    uint32   nbr_rate_ratio;  /* Current neighbor rate ratio */
    uint32   is_measuring_pdelay; /* Is port measuring PDELAY? */
    uint32   allowed_lost_responses; /* Number PDELAY responses allowed */
    uint32   peer_delay_threshold;   /* PDELAY threshold value */
    uint32   port_role;       /* Port role determined by BMCA */
    uint32   sync_receipt_timeout;  /* Current SYNC RX timeout */
    uint32   announce_receipt_timeout; /* Current BMCA RX timeout */
    char     sync_interval[20];   /* Current SYNC TX interval */
    char     announce_interval[20]; /* Current ANNOUNCE TX interval */
    char     pdelay_interval[20];   /* Current PDELAY TX interval */
} mgmt_dot1as_port_info_t;

typedef struct mgmt_dot1as_port_stats_s {
    uint32   sync_tx;
    uint32   sync_rx;
    uint32   followup_tx;
    uint32   followup_rx;
    uint32   announce_tx;
    uint32   announce_rx;
    uint32   pdelay_req_tx;
    uint32   pdelay_req_rx;
    uint32   pdelay_resp_tx;
    uint32   pdelay_resp_rx;
    uint32   pdelay_followup_tx;
    uint32   pdelay_followup_rx;
    uint32   sync_rx_timeouts;
    uint32   sync_discards;
    uint32   announce_rx_timeouts;
    uint32   announce_discards;
    uint32   pdelay_rx_timeouts;
    uint32   pdelay_discards;
    uint32   bad_headers;
} mgmt_dot1as_port_stats_t;

typedef struct mgmt_lag_status_reply_s {
    uint32   lag_id_status;
    uint8    port_bitmap;
    uint32   hash_index;
} mgmt_lag_status_reply_t;

typedef struct mgmt_acd_results_s {
    uint32      fault;                  /* ECD cable fault register */
    uint32      length;
    uint32      status;
} mgmt_acd_results_t;

typedef union  {
    mgmt_port_info_t port_info;
    mgmt_dot1as_info_t dot1as_info;
    mgmt_dot1as_port_info_t dot1as_port_info;
    mgmt_dot1as_port_stats_t dot1as_port_stats;
    mgmt_lag_status_reply_t lag_status_reply;
    mgmt_acd_results_t mgmt_acd;           /* ACD results */
    struct {
      uint32 value;   /* Switch age time */
    } switch_age_time;
 
    struct {
      uint32 value;   /* Switch age time */
    } switch_mtu;

    struct {
       uint32 num;   /* Port number */
       uint32 value; /* 0=Disabled, 1=Enabled */
    } port_jumbo_frame_mode;
    
    struct {
        uint32   port_bitmap;  /* VLAN membership */
        uint32   tag_bitmap;   /* Tagged bitmap */
    } vlan_info;

    struct {
        uint32   rate_kbps;    /* Rate in kpbs */
        uint32   burst_kbits;  /* Burst in kbits */
    } bw_info;

    struct {
        uint32 prio;       /* Priority of class */
        uint32 remap_prio; /* Remap priority of class */
    } pcp_mapping;
    struct {
        uint32 hash_index;  /* hash index value */
    } hash_info_get;
    struct {
        uint32 dumbfwd_mode; /*  get dumbfwd mode */
    } dumbfwd_status;
    struct {
        uint32 port_type; /* get port type */
    } mgmt_port_type;

} mgmt_reply_avb_t;

typedef struct mgmt_reply_version_s {
    uint32    mode;               /* Navigator mode */
    uint32    mos_version;        /* Software version */
    uint32    args[4];            /* args to booted with */
    uint32    model_id;           /* model id */
    uint32    chip_id;            /* chip id */
    uint32    otp_bits;           /* OTP bit values */
} mgmt_reply_version_t;

typedef struct mgmt_reply_buffer_info_s {
    uint32      count;
    struct {
        uint32  flags;
        uint32  length;
    } info[MGMT_NUM_BUFFERS];
} mgmt_reply_buffer_info_t;

typedef struct mgmt_reply_acd_results_s {
    uint32      fault;                  /* ECD cable fault register */
    uint32      index;
    uint32      peak_amplitude;
    uint32      length;
    uint32      status;
} mgmt_reply_acd_results_t;


typedef struct mgmt_reply_s {
    uint32            status;                 /* Used in replies */
    uint32            len;                    /* Length of entire packet */
    uint32            magic;                  /* MGMT_REPLY_MAGIC */
    union {
        mgmt_reply_version_t    version;    /* version/status information */
        struct {
            uint32      number;             /* ping number */
        } ping;
        struct {
            uint32      state;              /* power state */
        } power;
        struct {
            /* nonce to use as the IV with the encrypted session key */
            uint8     nonce[MGMT_SESSION_KEY_LEN];
            /*
             * The new AES session key, which is itself encrypted
             * with the RSA public key embedded in the Navigator device.
             * The host code will have to have a copy of the corresponding
             * private key to use this.
             */
            uint8     message[MGMT_SESSION_KEY_MSG_LEN];
        } session;
        struct {
            uint32    upper_regval;               /* switch reg value, upper bits */
            uint32    lower_regval;               /* switch reg value, lower bits */
        } switch_read;
        struct {
            uint32    rdid;                       /* response to RDID command */
        } flash_info;
        struct {
            /* Flash read data */
            uint8     data[MGMT_FLASH_PAGE_SIZE];
        } flash_read;
        struct {
            /* memory read data */
            uint8     data[MGMT_MEMORY_PAGE_SIZE];
        } read_memory;
        struct {
            uint8     data[MGMT_RNG_DATA_SIZE];
        } rng;
        struct {
            uint32    value;
        } perf_counter;
        mgmt_reply_buffer_info_t buffer_info;
        struct {
            uint8       data[MGMT_BUFFER_PAGE_SIZE];
        } read_buffer;
        mgmt_reply_acd_results_t acd;           /* ACD results */
        mgmt_reply_avb_t avb;  /* Command response for AVB */
    } u;
    char        pad[16];                        /* AES encryption padding */
} mgmt_reply_t;

#endif
