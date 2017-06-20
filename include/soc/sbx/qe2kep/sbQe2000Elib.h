#ifndef __SBQE2000ELIB_H__
#define __SBQE2000ELIB_H__
/**
 * @file sbQe2000Elib.h elib public API
 *
 * <pre>
 * ========================================
 * ==  sbQe2000Elib.h - elib public API  ==
 * ========================================
 *
 * WORKING REVISION: $Id: sbQe2000Elib.h,v 1.6 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * MODULE NAME:
 *
 *     sbQe2000Elib.h
 *
 * ABSTRACT:
 *
 *     elib public API
 *
 * LANGUAGE:
 *
 *     C
 *
 * AUTHORS:
 *
 *     David R. Pickett
 *     Travis B. Sawyer
 *
 * CREATION DATE:
 *
 *     20-August-2004
 * </pre>
 */
#include <sbTypes.h>
#include <sbElibStatus.h>
#include <glue.h>
#include <hal_ka_auto.h>
#include <hal_ka_inline.h>
#include "sbQe2000ElibZf.h"

#ifndef bool_t
typedef unsigned char bool_t;
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#define SB_QE2000_ELIB_NUM_CLASSES_K        (16)  /**< Number of packet classes */
#define SB_QE2000_ELIB_NUM_PORTS_K          (50)  /**< Number of ports */
#define SB_QE2000_ELIB_NUM_VLANS_K        (4096)  /**< Number of VLANs */
#define SB_QE2000_ELIB_NUM_CRT_ENTRIES_K  (1024)  /**< Number of entries in the Class Resolution Table (CRT) */
#define SB_QE2000_ELIB_NUM_CRT_BITS_K       (10)  /**< Number of bits in the Class Resolution Table (CRT) index */
#define SB_QE2000_ELIB_NUM_CMAPS_K           (2)  /**< Number of counter maps */
#define SB_QE2000_ELIB_NUM_SEGMENTS_K       (16)  /**< Number of memory segments */

#define SB_QE2000_ELIB_NUM_CLASS_INSTR_K     (8)

#define SB_QE2000_ELIB_SEGMENT_BASE_V        (3)
#define SB_QE2000_ELIB_SEGMENT_SIZ_DIV_K     (8)

#define SB_QE2000_ELIB_SEM_TO_K              (2)  /**< Two second timeout for semaphore wait */
#define SB_QE2000_ELIB_MAX_COUNTERS_K     (SB_QE2000_ELIB_NUM_CLASSES_K + SB_QE2000_ELIB_NUM_PORTS_K)


/**
 * elib handle
 *
 * An elib handle.  The handle is created by calling sbQe2000ElibInit.  The handle is passed
 * to all subsequent elib calls.
 */
typedef void * SB_QE2000_ELIB_HANDLE;

/**
 * EP Memory Configuration Encoding
 */
typedef enum sb_qe2000_elib_mm_config_t {
    SB_QE2000_ELIB_MM_CONFIG_DISABLED = 0,   /**< BF/IP memory is disabled */
    SB_QE2000_ELIB_MM_CONFIG_BF0,            /**< BF has 0K, IP has 32k x 64b */
    SB_QE2000_ELIB_MM_CONFIG_BF32,           /**< BF has 32k x 64b, IP has 0 */
    SB_QE2000_ELIB_MM_CONFIG_BF16,           /**< BF has 16k x 64b, IP has 16k x 64b */
    SB_QE2000_ELIB_MM_CONFIG_MAX
} SB_QE2000_ELIB_MM_CONFIG_T, *SB_QE2000_ELIB_MM_CONFIG_PT;

/**
 * IP Memory Segment Configuration
 *
 * This struct defines the parameters for configuring one of the 16 IP Memory
 * Segments.  Each segment is defined by an offset, a length, and a word size.
 * The offset and length paramters are in units of 8 64-bit words.  The word
 * size is Log2(n) where n is the number of bits in the word.
 *
 * <pre>
 * size   word width
 * ----   ----------
 *  0         1
 *  1         2
 *  2         4
 *  3         8
 *  4        16
 *  5        32
 *  6        64
 *  7       128
 * </pre>
 */
typedef struct sb_qe2000_elib_ip_mem_seg_cfg_s {
    uint32 ulOffset;                  /**< Offset of segment in 8x64-bit words */
    uint32 ulLength;                  /**< Length of segment in 8x64-bit words */
    uint32 ulWordSize;                /**< Log2(n) size of data word in bits */
} SB_QE2000_ELIB_IP_MEM_SEG_ST, *SB_QE2000_ELIB_IP_MEM_SEG_PST;


typedef enum sb_qe2000_elib_dma_stats_mem_t {
    SB_QE2000_ELIB_STATS_PCT = 8,
    SB_QE2000_ELIB_IP_MEM    = 10,
    SB_QE2000_ELIB_STATS_VRT = 11,
    SB_QE2000_ELIB_STATS_MAX
} SB_QE2000_ELIB_DMA_STATS_MEM_T, *SB_QE2000_ELIB_DMA_STATS_MEM_PT;

typedef struct sb_qe2000_elib_counter_s {
    uint64 ullPktCount;
    uint64 ullByteCount;
} SB_QE2000_ELIB_COUNTER_ST, *SB_QE2000_ELIB_COUNTER_PST;



/**
 * elib DMA function argument structure
 *
 * This struct is used to pass pertinent information from the ELIB to the user
 * supplied DMA function.
 *
 */
typedef struct sb_qe2000_elib_dma_func_params_s {
    void*    pUserData;                      /**< User supplied data */
    SB_QE2000_ELIB_DMA_STATS_MEM_T StatsMem; /**< Which statistic memory to transfer */
    uint32 ulStartAddr;                    /**< Start address within specified memory */
    uint32 ulNumLines;                     /**< Number of lines to transfer */
    uint32 ulPreserve;                     /**< Preserve the memory contents */
    void*    pData;                          /**< Pointer to data to receive the transfer */
} SB_QE2000_ELIB_DMA_FUNC_PARAMS_ST, *SB_QE2000_ELIB_DMA_FUNC_PARAMS_PST;

/**
 * elib DMA function pointer
 *
 * Connection between the user supplied OSAL used for DMAing statistics from the
 * QE2000 to the elib.
 */
typedef int (*SB_QE2000_ELIB_USER_DMA_PF)(sbhandle hSbQe, SB_QE2000_ELIB_DMA_FUNC_PARAMS_PST pDmaParam);

/**
 * elib semaphore creation function pointer
 *
 * Function pointer to user supplied semaphore creation function.
 * Takes user supplied void pointer. Returns semaphore ID, or -1 on failure.
 */
typedef int (*SB_QE2000_ELIB_SEM_CREATE_PF)(void* pUserSemData);

/**
 * elib semaphore give function pointer
 *
 * Function pointer to user supplied semaphore give function.
 * Takes semaphore ID (int).
 * Returns 0 on OK, some other valure for failure.
 */
typedef int (*SB_QE2000_ELIB_SEM_GIVE_PF)(int nSemId, void* pUserSemData);

/**
 * elib semaphore try & wait function pointer
 *
 * Function pointer to user supplied semaphore try & wait function.
 * Takes semaphore ID (int) and timeout in seconds.
 * Returns 0 on OK, some other value for failure.
 */
typedef int (*SB_QE2000_ELIB_SEM_TRYWAIT_PF)(int nSemId, void* pUserSemData, int nTimeOut);

/**
 * elib semaphore destroy function pointer
 *
 * Function pointer to user supplied semaphore destory function.
 * Takes semaphore ID (int) and user supplied semaphore data.
 * Returns 0 on OK, some other value for failure.
 */
typedef int (*SB_QE2000_ELIB_SEM_DESTROY_PF)(int nSemId, void* pUserSemData);

/**
 * elib initialization parameters
 *
 * This struct is passed to the sbQe2000ElibInit function to set the parameters in the elib that
 * are global, and generally do not change in runtime for a given application.  There is
 * a function sbQe2000ElibDefParamsGet that populates an instance of this struct with the default
 * values that are documented here.
 *
 * Usage notes:
 *
 * - The bSoftClassEna flag sets all eight hard class flags in the ep_cl_enable register at
 *   once.  The rationale is that you either use soft classification for an application or
 *   you do not.  There is no mixing/matching.  If this turns into an issue, this is
 *   easily modified, but woe be to the system integrator who is charged with managing
 *   such a system.
 */
typedef struct sb_qe2000_elib_init_params_s {
    bool_t bVitEna;             /**< Flag to enable the VLAN Indirection Table (VIT) - Default TRUE  (ep_bf_config) */
    bool_t bOnePcwEna;          /**< Flag to enable single PCW in the VLAN Record Table (VRT) - Default FALSE (ep_bf_config) */
    bool_t bBypassBF;           /**< Flag to bypass the Bridging Function - Default FALSE (ep_config) */
    bool_t bBypassIP;           /**< Flag to bypass the Instruction Processor - Default FALSE (ep_config) */
    bool_t bBypassCL;           /**< Flag to bypass the Classifier - Default FALSE (ep_config) */
    bool_t bEpEna;              /**< Flag to enable the EP - This is the make it go bit - Default TRUE (ep_config) */
    SB_QE2000_ELIB_MM_CONFIG_T MmConfig; /**< BF/IP memory configuration - Default SB_QE2000_ELIB_MM_CONFIG_BF16 (ep_mm_config) */
    bool_t bSoftClassEna;       /**< Flag to enable EP Soft Classification - Default TRUE (ep_cl_enable) */
    uint32 ulVlanA;           /**< VLAN Tag A to use for VLAN Encapsulation - Default 0x8100 (ep_ip_vencap_tag) */
    uint32 ulVlanB;           /**< VLAN Tag B to use for VLAN Encapsulation - Default 0x9100 (ep_ip_vencap_tag) */
    uint32 ulEthertype0;      /**< Ethertype for ucast Ethernet payload type 0 - Default 0x0000 (ep_ip_ueencap_type0) */
    uint32 ulEthertype1;      /**< Ethertype for ucast Ethernet payload type 1 - Default 0x0000 (ep_ip_ueencap_type1) */
    uint64 ullERHForward;     /**< Enable transmission of Egress Route Header (ERH) Per Port Bit Mask (ep_bf_erhX) - Default 0 (drop) */
    uint64 ullSmacMsbEnable;  /**< Enable SMAC MSB, Per Port BitMask, default 0 (do not enable SMAC MC bit rewrite) */
    uint64 ullMirrorEnable;     /**< Enable mirroring, Per Port BitMask */
    int nPorts;                 /**< Number of ports within the system */
    SB_QE2000_ELIB_IP_MEM_SEG_ST IpMemSegCfg[ SB_QE2000_ELIB_NUM_SEGMENTS_K ]; /**< IP memory segment configuration (ep_ip_segmentN) */
    SB_QE2000_ELIB_USER_DMA_PF pfUserDmaFunc;        /**< User supplied DMA function.  Used to dma statistics from the QE2000 */
    void*    pUserData;                              /**< User supplied data to assist in DMA functions */
    SB_QE2000_ELIB_SEM_CREATE_PF pfUserSemCreate;    /**< User supplied sem create function */
    SB_QE2000_ELIB_SEM_TRYWAIT_PF pfUserSemTryWait;  /**< User supplied sem try & wait function */
    SB_QE2000_ELIB_SEM_GIVE_PF pfUserSemGive;        /**< User supplied sem give function */
    SB_QE2000_ELIB_SEM_DESTROY_PF pfUserSemDestroy;  /**< User supplied sem destroy function */
    void*    pUserSemData;                           /**< User supplied semaphore void pointer for use with the MVT */
    void*    pMVTUserSemData;                        /**< User supplied semaphore void pointer for use with the MVT */
    int      nMVTSemId;                              /**< Semaphore ID supplied by the user to negogiate access to the MVT */
    int      nMVTTimeOut;                            /**< Time out value for the semaphore try & wait for use with the MVT semaphore */
} SB_QE2000_ELIB_INIT_PARAMS_ST, *SB_QE2000_ELIB_INIT_PARAMS_PST;

/**
 * Initialize an SB_QE2000_ELIB_INIT_PARAMS_ST with default values.
 *
 * @param pParams    The elib initialization parameters.  This is a pointer to an
 *                   SB_QE2000_ELIB_INIT_PARAMS_ST struct.  Upon return indication of success,
 *                   the struct is initialized with the default values for each member
 *                   element in the struct.
 *
 * @return           Status.  0 indicates success - Non-Zero indicates failure
 */
sbElibStatus_et sbQe2000ElibDefParamsGet( SB_QE2000_ELIB_INIT_PARAMS_PST pParams );

/**
 * Initialize the elib, and get a context for future calls to the library.
 * This must be the first call to the elib.  If the function returns
 * success, the context is used for all subsequent calls to the elib.
 *
 * @param pHandle    A pointer to an SB_QE2000_ELIB_HANDLE
 * @param pHalCtx    The HAL context pointer, glue dependant pointer for accessing the QE2000.
 * @param Params     The elib initialization parameters.  This is an SB_QE2000_ELIB_INIT_PARAMS_ST struct.
 * @return           Status.  0 indicates success - Non-Zero indicates failure
 */
sbElibStatus_et sbQe2000ElibInit( SB_QE2000_ELIB_HANDLE *pHandle, void *pHalCtx, SB_QE2000_ELIB_INIT_PARAMS_ST Params );

/**
 * Uninitialize the elib
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibUnInit( SB_QE2000_ELIB_HANDLE Handle );

/**
 * Set VLAN Encapsulation Tags
 *
 * @param pHandle    A pointer to an SB_QE2000_ELIB_HANDLE
 * @param ulVlanTagA VLAN Tag A.
 * @param ulVlanTagB Vlan Tag B.
 * @return           Status.  0 indicates success - Non-Zero indicates failure
 */
sbElibStatus_et sbQe2000ElibVlanEncapSet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulVlanTagA, uint32 ulVlanTagB );

/**
 * Get VLAN Encapsulation Tags
 *
 * @param pHandle     A pointer to an SB_QE2000_ELIB_HANDLE
 * @param pulVlanTagA Upon success VLAN Tag A will be placed in this location.
 * @param pulVlanTagB Upon success Vlan Tag B will be placed in this location.
 * @return            Status.  0 indicates success - Non-Zero indicates failure
 */
sbElibStatus_et sbQe2000ElibVlanEncapGet( SB_QE2000_ELIB_HANDLE Handle, uint32 *pulVlanTagA, uint32 *pulVlanTagB );

/**
 * Set VLAN Encapsulation Tags
 *
 * @param pHandle    A pointer to an SB_QE2000_ELIB_HANDLE
 * @param ulMpls     Ethernet type for MPLS
 * @return           Status.  0 indicates success - Non-Zero indicates failure
 */
sbElibStatus_et sbQe2000ElibMplsEncapSet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulMpls );

/**
 * Get VLAN Encapsulation Tags
 *
 * @param pHandle     A pointer to an SB_QE2000_ELIB_HANDLE
 * @param pulMpls     Upon success Ethernet type for MPLS will be placed in this location.
 * @return            Status.  0 indicates success - Non-Zero indicates failure
 */
sbElibStatus_et sbQe2000ElibMplsEncapGet( SB_QE2000_ELIB_HANDLE Handle, uint32 *pulMpls );




/**
 * Encoding for tag type to select when performing the eencap operation
 */
typedef enum sb_qe2000_elib_eetag_t {
    SB_QE2000_ELIB_EETAG_A = 0,  /**< Use Tag A from ep_ip_vencap_tag register */
    SB_QE2000_ELIB_EETAG_B,      /**< Use Tag B from ep_ip_vencap_tag register */
    SB_QE2000_ELIB_EETAG_MAX
} SB_QE2000_ELIB_EETAG_T, *SB_QE2000_ELIB_EETAG_PT;

/**
 * Port Configuration Parameters
 */
typedef struct sb_qe2000_elib_port_cfg_s {
    bool_t bClassEna[SB_QE2000_ELIB_NUM_CLASSES_K];  /**< Array of class enable flags - 1 = Class Enabled - 0 = Class Disabled (PT) */
    bool_t bAppend;                         /**< Append/Prepend Instruction - 1 = Append - 0 = Prepend (PT) */
    bool_t bInstValid;                      /**< Instruction valid bit - 1 = Valid - 0 = Not valid (PT) */
    uint32 ulInst;                        /**< EP Instruction to associate with this port (PT) */
    bool_t bPriRewriteEna;                  /**< Enable PRI Re-write (ep_bf_pri_rewriteX) */
    bool_t bBfEna;                          /**< Enable Bridging Function (BF) (ep_bf_portX) */
    bool_t bUntaggedEna;                    /**< Enable tramsission of untagged frames (ep_bf_tagX) */
    SB_QE2000_ELIB_EETAG_T TagSel;          /**< VLAN tag to use for Ethernet Encapsulation (ep_ip_eencap_tagX) */
} SB_QE2000_ELIB_PORT_CFG_ST, *SB_QE2000_ELIB_PORT_CFG_PST;

/**
 * Set the configuration of an EP port
 *
 * The configuration information is specified in an SB_QE2000_ELIB_PORT_CFG_ST struct.  See the
 * declaration of that struct for details on the meaning of individual members.
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 * @param ulPort     Port identifier.  Valid values are 0..SB_QE2000_ELIB_NUM_PORTS_K - 1.
 * @param Cfg        The port configuration.  This is an SB_QE2000_ELIB_PORT_CFG_ST struct.
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibPortCfgSet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulPort, SB_QE2000_ELIB_PORT_CFG_ST Cfg );

/**
 * Get the configuration of an EP port
 *
 * The configuration information is returned in an SB_QE2000_ELIB_PORT_CFG_ST struct.  See the
 * declaration of that struct for details on the meaning of individual members.
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 * @param ulPort     Port identifier.  Valid values are 0..SB_QE2000_ELIB_NUM_PORTS_K - 1.
 * @param pCfg       The port configuration.  This is a pointer to an SB_QE2000_ELIB_PORT_CFG_ST struct.
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibPortCfgGet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulPort, SB_QE2000_ELIB_PORT_CFG_PST pCfg );

/**
 * Class Resolution Table (CRT) index bit source field encoding.
 */
typedef enum sb_qe2000_elib_crt_bs_src_t {
    SB_QE2000_ELIB_CRT_BS_SRC_ERH = 0,       /**< Egress Route Header */
    SB_QE2000_ELIB_CRT_BS_SRC_IS,            /**< Instruction Stack */
    SB_QE2000_ELIB_CRT_BS_SRC_PAYLOAD,       /**< Payload */
    SB_QE2000_ELIB_CRT_BS_SRC_MAX
} SB_QE2000_ELIB_CRT_BS_SRC_T, *SB_QE2000_ELIB_CRT_BS_SRC_PT;

/**
 * Class Resolution Table (CRT) bit selection specifier
 *
 * This struct defines the selection criteria for a single bit in the index to
 * the CRT.
 *
 */
typedef struct sb_qe2000_elib_crt_bit_sel_s {
    SB_QE2000_ELIB_CRT_BS_SRC_T SourceField;     /**< Source Field for the bit select    */
    uint32 ulOffset;                  /**< Bit offset within the Source Field */
} SB_QE2000_ELIB_CRT_BIT_SEL_ST, *SB_QE2000_ELIB_CRT_BIT_SEL_PST;

/**
 * Set the selection criteria for a Class Resolution Table (CRT) index bit
 *
 * The Class Resolution Table (CRT) is indexed by a 10-bit (0..SB_QE2000_ELIB_NUM_CRT_ENTRIES_K -1 ) value
 * that is user-defined.
 *
 * This function is used to set the selection criteria for one of the ten bits in the index.
 * The user specifies which bit (0..9) to configure, the source of the bit (SB_QE2000_ELIB_CRT_BS_SRC_T),
 * and the bit offset within the particular source field.  The range of the bit offset varies
 * according to the bit source:
 *
 * - SB_QE2000_ELIB_CRT_BS_SRC_ERH     - Egress Route Header - 0..127
 * - SB_QE2000_ELIB_CRT_BS_SRC_IS      - Instruction Stack - 0..127
 * - SB_QE2000_ELIB_CRT_BS_SRC_PAYLOAD - Payload - 0..511
 *
 * NOTE: BIT ZERO IS MOST SIGNIFICANT WHEN CALCULATING THE CRT INDEX.
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param pBitSelect A pointer to an array of SB_QE2000_ELIB_CRT_BIT_SEL_ST structs.  The array
 *                   has an entry for each bit in the select index, which is
 *                   SB_QE2000_ELIB_NUM_CRT_BITS_K.
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibCRTBitSelectSet( SB_QE2000_ELIB_HANDLE Handle,
				 SB_QE2000_ELIB_CRT_BIT_SEL_PST pBitSelect );

/**
 * Get the selection criteria for a Class Resolution Table (CRT) index bit
 *
 * See the documentation for sbQe2000ElibCRTBitSelectSet for details on the construction of the CRT index.
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param pBitSelect A pointer to an array of SB_QE2000_ELIB_CRT_BIT_SEL_ST structs.  The array
 *                   has an entry for each bit in the select index, which is
 *                   SB_QE2000_ELIB_NUM_CRT_BITS_K.  Upon a return indication of success, this
 *                   array is populated with the bit selection criteria.
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibCRTBitSelectGet( SB_QE2000_ELIB_HANDLE Handle,
				 SB_QE2000_ELIB_CRT_BIT_SEL_PST pBitSelect );

/**
 * Set an entry in the Class Resolution Table (CRT)
 *
 * The Class Resolution Table (CRT) is indexed by a 10-bit (0..SB_QE2000_ELIB_NUM_CRT_ENTRIES_K - 1) value
 * that is user-defined.
 * See sbQe2000ElibCRTBitSelectSet for details on the configuration of this index.  The index points to
 * a 4-bit (0..SB_QE2000_ELIB_NUM_CLASSES_K - 1) value that defines the packet class for the given index.
 *
 * NOTE:  CRT entries are not forwarded to the device until a call to sbQe2000ElibCRTPush is made.  This
 * NOTE:  optimization reduces overhead with respect to memory packing.
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param ulIndex    The index.  This is an integer in the range of 0..SB_QE2000_ELIB_NUM_CRT_ENTRIES_K - 1.
 *
 * @param ulClass    The packet class.  This in an integer in the range of 0..SB_QE2000_ELIB_NUM_CLASSES_K - 1
 *                   that is the packet class associated with the given ulIndex.
 *
 * @return           Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibCRTSet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulIndex, uint32 ulClass );

/**
 * Push previously set CRT entries to the device.
 *
 * The Class Resolution Table (CRT) is indexed by a 10-bit (0..1023) value that is user-defined.
 * This function synchronizes the local copy of the CRT with the device.  All previously set
 * CRT entries are pushed.
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param bAll       Flush all CRT entries, not just the lines marked as dirty.
 *
 * @return           Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibCRTPush( SB_QE2000_ELIB_HANDLE Handle, bool_t bAll );

/**
 * Get an entry from the Class Resolution Table (CRT)
 *
 * The Class Resolution Table (CRT) is indexed by a 10-bit (0..1023) value that is user-defined.
 * See sbQe2000ElibCRTBitSelectSet for details on the configuration of this index.  The index points to
 * a 4-bit (0..SB_QE2000_ELIB_NUM_CLASSES_K - 1) value that defines the packet class for the given index.
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param ulIndex    The index.  This is an integer in the range of 0..SB_QE2000_ELIB_NUM_CRT_ENTRIES_K - 1.
 *
 * @param pulClass   The packet class.  Upon a return indication of success, this points to an
 *                   integer in the range of 0..SB_QE2000_ELIB_NUM_CLASSES_K - 1 that is the packet class
 *                   associated with the given ulIndex.
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibCRTGet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulIndex, uint32* pulClass );

/**
 * Class Instruction Table (CIT) Entry
 */
typedef struct sb_qe2000_elib_cit_entry_s
{
    uint32 ulInst[SB_QE2000_ELIB_NUM_CLASS_INSTR_K];         /**< EP Instruction  */
    bool_t bInstValid[SB_QE2000_ELIB_NUM_CLASS_INSTR_K];       /**< Instruction Valid flag - 1 = Valid - 0 = Not Valid */
    bool_t bAppend[SB_QE2000_ELIB_NUM_CLASS_INSTR_K];          /**< Instruction Append flag - 1 = Append - 0 = Prepend */
} SB_QE2000_ELIB_CIT_ENTRY_ST, *SB_QE2000_ELIB_CIT_ENTRY_PST;

/**
 * Set an entry in the Class Instruction Table (CIT)
 *
 * The Class Resolution Table (CRT) is indexed by a 10-bit (0..1023) value that is user-defined.
 * See sbQe2000ElibCRTBitSelectSet for details on the configuration of this index.  The index points to
 * a 4-bit (0..SB_QE2000_ELIB_NUM_CLASSES_K - 1) value that defines the packet class for the given index.
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param ulClass    The packet class.  This in an integer in the range of 0..SB_QE2000_ELIB_NUM_CLASSES_K - 1
 *                   that is the packet class with which the instructions are associated.
 *
 * @param Inst       The set of instructions to associate with the given class.  This is an
 *                   SP_EP_CIT_ENTRY_ST struct.
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibCITSet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulClass, SB_QE2000_ELIB_CIT_ENTRY_ST Inst );

/**
 * Get an entry from the Class Instruction Table (CIT)
 *
 * The Class Resolution Table (CRT) is indexed by a 10-bit (0..1023) value that is user-defined.
 * See sbQe2000ElibCRTBitSelectSet for details on the configuration of this index.  The index points to
 * a 4-bit (0..SB_QE2000_ELIB_NUM_CLASSES_K - 1) value that defines the packet class for the given index.
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param ulClass    The packet class.  This in an integer in the range of 0..SB_QE2000_ELIB_NUM_CLASSES_K - 1
 *                   that is the packet class with which the instructions are associated.
 *
 * @param pInst      A pointer to an SB_QE2000_ELIB_CIT_ENTRY_ST.  Upon a return indication of success,
 *                   this struct contains the CIT entry for the given packet class.
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibCITGet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulClass, SB_QE2000_ELIB_CIT_ENTRY_PST pInst );

/**
 * Get a counter pair from the Port Counter Table (PCT)
 *
 * The Port Counter Table (PCT) is indexed by (port,class).  Each entry in this table
 * contains a 32-bit byte count, and a 32-bit packet count.  The counters in this
 * table are cleared on read.
 *
 * @param Handle         A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param ulPort         The port.  It has a range of 0..SB_QE2000_ELIB_NUM_PORTS_K - 1.
 *
 * @param ulClass        The class.  It has a range of 0..SB_QE2000_ELIB_NUM_CLASSES_K - 1.
 *
 * @param pullPacketCnt  A pointer to a uint64.  Upon a return indication of success,
 *                       this contains the packet count for the (port,class) pair.
 *
 * @param pullByteCnt    A pointer to a uint64.  Upon a return indication of success,
 *                       this contains the byte count for the (port,class) pair.
 *
 * @return               Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibPCTGet( SB_QE2000_ELIB_HANDLE Handle,
			uint32 ulPort,
			uint32 ulClass,
			uint64 *pullPacketCnt,
			uint64 *pullByteCnt );



/**
 * VLAN Indirection Table (VIT) CMAP field encoding
 */
typedef enum sb_qe2000_elib_vit_cmap_t {
    SB_QE2000_ELIB_VIT_CMAP_0_NO_PORT = 0,   /**< VLAN Pass/Drop Counters map 0 with no port counters. */
    SB_QE2000_ELIB_VIT_CMAP_1_NO_PORT,       /**< VLAN Pass/Drop Counters map 1 with no port counters. */
    SB_QE2000_ELIB_VIT_CMAP_MAP0,            /**< Global VLAN class to counter map 0.            */
    SB_QE2000_ELIB_VIT_CMAP_MAP1,            /**< Global VLAN class to counter map 1.            */
    SB_QE2000_ELIB_VIT_CMAP_MAX
} SB_QE2000_ELIB_VIT_CMAP_T, *SB_QE2000_ELIB_VIT_CMAP_PT;


/**
 * VLAN Record Table (VRT) Port state encoding
 */
typedef enum sb_qe2000_elib_vrt_ps_t {
    SB_QE2000_ELIB_VRT_PS_UNMANAGED = 0, /**< Port is not managed.                                 */
    SB_QE2000_ELIB_VRT_PS_DISABLED,      /**< Port is managed but disabled.                        */
    SB_QE2000_ELIB_VRT_PS_ENA_NOTAG,     /**< Port is managed and enabled for untagged forwarding. */
    SB_QE2000_ELIB_VRT_PS_ENA_TAGGED,    /**< Port is managed and enabled for tagged forwarding.   */
    SB_QE2000_ELIB_VRT_PS_MAX
} SB_QE2000_ELIB_VRT_PS_T, *SB_QE2000_ELIB_VRT_PS_PT;

/**
 * Create a VLAN Record
 *
 * This function creates a VLAN record associated with a ScIdx.  It populates the VLAN
 * record Table with the VLAN ID, and Port States, sets up the counters associated with
 * the port states, and creates a VLAN Indirection Table entry.
 *
 * @param Handle      A valid SB_QE2000_ELIB_HANDLE.
 * @param ulScIdx     The ScIdx to be associated with this VLAN.
 * @param ulVid       The VLAN ID.
 * @param tPortState  Default port state for all ports.
 * @param tCmap       The Class Counter Map to be used with this VLAN entry.
 *
 * @return            Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibVlanRecordCreate( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx, uint32 ulVid,
                                  SB_QE2000_ELIB_VRT_PS_T tPortState,
                                  SB_QE2000_ELIB_VIT_CMAP_T tCmap);

/**
 * Retrieve a VLAN Record
 *
 * This function gets a VLAN record associated with a ScIdx.
 *
 * @param Handle      A valid SB_QE2000_ELIB_HANDLE.
 * @param ulScIdx     The ScIdx to be associated with this VLAN.
 * @param ptCmap      Pointer to a cmap type.
 * @param pZf         A pointer to a VRT Zframe.
 *
 * @return            Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibVlanRecordGet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx,
                               SB_QE2000_ELIB_VIT_CMAP_PT ptCmap,
                               sbZfSbQe2000ElibVRT_t *pZf);

/**
 * Update the port state for a VLAN Record
 *
 * This function updates a VLAN record associated with a ScIdx.  It changes the state
 * for the given port and possibly realigns the associated counters.  Updating a VLAN
 * record may require reallocation of the VLAN record if the new port state dictates
 * a counter change.  Any call to this function forces a counter accumulate for the
 * VLAN associated with the ScIdx.
 *
 * @param Handle      A valid SB_QE2000_ELIB_HANDLE.
 * @param ulScIdx     The ScIdx to be associated with this VLAN.
 * @param ulPort      The port to apply the state change to.
 * @param tPortState  The new port state.
 *
 * @return            Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibVlanPortStateSet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx, uint32 ulPort,
                                  SB_QE2000_ELIB_VRT_PS_T tPortState );


/**
 * Retrive the port state for a VLAN Record
 *
 * This function gets a VLAN port state associated with a ScIdx.
 *
 * @param Handle      A valid SB_QE2000_ELIB_HANDLE.
 * @param ulScIdx     The ScIdx to be associated with this VLAN.
 * @param ulPort      The port to retrieve the state of.
 * @param ptPortState The returned port state.
 *
 * @return            Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibVlanPortStateGet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx,
                                  uint32 ulPort, SB_QE2000_ELIB_VRT_PS_T *ptPortState);


/**
 * Update a VLAN Record VLAN Id
 *
 * This function updates a VLAN record VLAN ID associated with a ScIdx.
 *
 * @param Handle      A valid SB_QE2000_ELIB_HANDLE.
 * @param ulScIdx     The ScIdx to be associated with this VLAN.
 * @param ulVid       The new VLAN ID to use in the record
 *
 * @return            Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibVlanVidUpdate( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx, uint32 ulVid);

/**
 * Attach a new ScIdx to a VLAN Record
 *
 * This function attaches a new ScIdx to a previously created VLAN.
 *
 * @param Handle        A valid SB_QE2000_ELIB_HANDLE.
 * @param ulScIdx       The ScIdx associated with this VLAN.
 * @param ulScIdxAttach The new ScIdx to attach to the VLAN associated with the original ScIdx.
 *
 * @return              Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibVlanScIdxAttach( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx,
                                       uint32 ulScIdxAttach );

/**
 * Detach a ScIdx from a VLAN Record
 *
 * This function removes a ScIdx association from a VLAN Record.
 *
 * @param Handle  A valid SB_QE2000_ELIB_HANDLE.
 * @param ulScIdx The ScIdx to be removed from the VLAN.
 *
 * @return        Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibVlanScIdxDetach( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx );

/**
 * Delete a VLAN Record and all associated ScIdx.
 *
 * This function removes all ScIdx associated with a VLAN Record.
 * It then removes the VLAN Record Table Entry, and removes all
 * associated counters.
 *
 * @param Handle  A valid SB_QE2000_ELIB_HANDLE.
 * @param ulScIdx The ScIdx of the VLAN to be removed.
 *
 * @return        Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibVlanRecordDelete( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx );

/**
 * Flush all information retained about VLANs
 *
 * This function removes all vlans, clears the Bridging Function memory,
 * and reinitializes the VLAN memory manager.  It also reinstalls the
 * default to/from proc VIT direct entry.
 *
 * @param Handle  A valid SB_QE2000_ELIB_HANDLE.
 *
 * @return        Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibVlanFlush( SB_QE2000_ELIB_HANDLE Handle );


typedef enum sb_qe2000_elib_vlan_count_index_t
{
    SB_QE2000_ELIB_CNT_CLASS0 = 0,
    SB_QE2000_ELIB_CNT_CLASS1,
    SB_QE2000_ELIB_CNT_CLASS2,
    SB_QE2000_ELIB_CNT_CLASS3,
    SB_QE2000_ELIB_CNT_CLASS4,
    SB_QE2000_ELIB_CNT_CLASS5,
    SB_QE2000_ELIB_CNT_CLASS6,
    SB_QE2000_ELIB_CNT_CLASS7,
    SB_QE2000_ELIB_CNT_CLASS8,
    SB_QE2000_ELIB_CNT_CLASS9,
    SB_QE2000_ELIB_CNT_CLASS10,
    SB_QE2000_ELIB_CNT_CLASS11,
    SB_QE2000_ELIB_CNT_CLASS12,
    SB_QE2000_ELIB_CNT_CLASS13,
    SB_QE2000_ELIB_CNT_CLASS14,
    SB_QE2000_ELIB_CNT_CLASS15,
    SB_QE2000_ELIB_CNT_PORT0,
    SB_QE2000_ELIB_CNT_PORT1,
    SB_QE2000_ELIB_CNT_PORT2,
    SB_QE2000_ELIB_CNT_PORT3,
    SB_QE2000_ELIB_CNT_PORT4,
    SB_QE2000_ELIB_CNT_PORT5,
    SB_QE2000_ELIB_CNT_PORT6,
    SB_QE2000_ELIB_CNT_PORT7,
    SB_QE2000_ELIB_CNT_PORT8,
    SB_QE2000_ELIB_CNT_PORT9,
    SB_QE2000_ELIB_CNT_PORT10,
    SB_QE2000_ELIB_CNT_PORT11,
    SB_QE2000_ELIB_CNT_PORT12,
    SB_QE2000_ELIB_CNT_PORT13,
    SB_QE2000_ELIB_CNT_PORT14,
    SB_QE2000_ELIB_CNT_PORT15,
    SB_QE2000_ELIB_CNT_PORT16,
    SB_QE2000_ELIB_CNT_PORT17,
    SB_QE2000_ELIB_CNT_PORT18,
    SB_QE2000_ELIB_CNT_PORT19,
    SB_QE2000_ELIB_CNT_PORT20,
    SB_QE2000_ELIB_CNT_PORT21,
    SB_QE2000_ELIB_CNT_PORT22,
    SB_QE2000_ELIB_CNT_PORT23,
    SB_QE2000_ELIB_CNT_PORT24,
    SB_QE2000_ELIB_CNT_PORT25,
    SB_QE2000_ELIB_CNT_PORT26,
    SB_QE2000_ELIB_CNT_PORT27,
    SB_QE2000_ELIB_CNT_PORT28,
    SB_QE2000_ELIB_CNT_PORT29,
    SB_QE2000_ELIB_CNT_PORT30,
    SB_QE2000_ELIB_CNT_PORT31,
    SB_QE2000_ELIB_CNT_PORT32,
    SB_QE2000_ELIB_CNT_PORT33,
    SB_QE2000_ELIB_CNT_PORT34,
    SB_QE2000_ELIB_CNT_PORT35,
    SB_QE2000_ELIB_CNT_PORT36,
    SB_QE2000_ELIB_CNT_PORT37,
    SB_QE2000_ELIB_CNT_PORT38,
    SB_QE2000_ELIB_CNT_PORT39,
    SB_QE2000_ELIB_CNT_PORT40,
    SB_QE2000_ELIB_CNT_PORT41,
    SB_QE2000_ELIB_CNT_PORT42,
    SB_QE2000_ELIB_CNT_PORT43,
    SB_QE2000_ELIB_CNT_PORT44,
    SB_QE2000_ELIB_CNT_PORT45,
    SB_QE2000_ELIB_CNT_PORT46,
    SB_QE2000_ELIB_CNT_PORT47,
    SB_QE2000_ELIB_CNT_PORT48,
    SB_QE2000_ELIB_CNT_PORT49
} SB_QE2000_ELIB_VLAN_COUNT_INDEX_T;


/**
 * Retrieve a count associated with a VLAN.
 *
 * This function retrieve the specified count (Class and port) associated with the
 * VLAN entry that is associated with a particular Switching Context.
 *
 * @param Handle    A valid SB_QE2000_ELIB_HANDLE.
 * @param ulScIdx   The ScIdx of the VLAN.
 * @param bClrOnRd  Clear the counts after reading them.
 * @param pCount    Pointer to a counter struct.
 *
 * @return          Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibVlanCountGet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx,
                              SB_QE2000_ELIB_VLAN_COUNT_INDEX_T tCntIdx,
                              bool_t bClrOnRd,
                              SB_QE2000_ELIB_COUNTER_PST pCount);


/**
 * Accumulate & Retrieve a count associated with a VLAN.
 *
 * This function retrieve the specified count (Class and port) associated with the
 * VLAN entry that is associated with a particular Switching Context.
 *
 * @param Handle    A valid SB_QE2000_ELIB_HANDLE.
 * @param ulScIdx   The ScIdx of the VLAN.
 * @param bClrOnRd  Clear the counts after reading them.
 * @param aCounts   An array large enough to contain all of the associated counters.
 *
 * @return          Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibVlanCountAccumulateGet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx,
                                        bool_t bClrOnRd,
                                        SB_QE2000_ELIB_COUNTER_ST aCounts[SB_QE2000_ELIB_MAX_COUNTERS_K]);



/**
 * Retrieve the Counts associated with a VLAN.
 *
 * This function retrieves the counts (Class and port) associated with the
 * VLAN entry that is associated with a particular Switching Context.
 *
 * @param Handle    A valid SB_QE2000_ELIB_HANDLE.
 * @param ulScIdx   The ScIdx of the VLAN.
 * @param bClrOnRd  Clear the counts after reading them.
 * @param aCounts   An array large enough to contain all of the associated counters.
 *
 * @return          Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibVlanCountGetAll( SB_QE2000_ELIB_HANDLE Handle, uint32 ulScIdx,
                                 bool_t bClrOnRd,
                                 SB_QE2000_ELIB_COUNTER_ST aCounts[SB_QE2000_ELIB_MAX_COUNTERS_K]);

/**
 * Accumulate the counters for a number of VLANs.
 *
 * This function retrieves the counts (Class and port) associated with the
 * VLAN entry that is associated with a particular Switching Context.
 *
 * This needs to be run periodically so as not to saturate the counters.
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 * @param ulNumScIdx The number of ScIdx's to accumulate counters for.
 * @param pulCounts  The number of counts accumulated.
 *
 * @return        Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibVlanCountAccumulate( SB_QE2000_ELIB_HANDLE Handle, uint32 ulNumScIdx, uint32 *pulCounts );


/**
 * Accumulate the counters for a number of VLANs, stopping when ulNumCounters
 * has been reached.
 *
 * This function retrieves the counts (Class and port) associated with the
 * VLAN entry that is associated with a particular Switching Context.
 *
 * This needs to be run periodically so as not to saturate the counters.
 *
 * @param Handle         A valid SB_QE2000_ELIB_HANDLE.
 * @param pulNumCounters The number of counter sets to accumulate.  Number of sets
 *                       actually accumulated returned in this parameter.
 *
 * @return        Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibVlanCounterSetsAccumulate( SB_QE2000_ELIB_HANDLE Handle, uint32 *pulNumCounters );

/**
 * Reset all counters to zero.
 *
 *
 * @param Handle  A valid SB_QE2000_ELIB_HANDLE.
 *
 * @return        Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibCountsReset( SB_QE2000_ELIB_HANDLE Handle );


/**
 * Counter Map Configuration
 *
 * This struct defines a counter map. The FwdMap and DrpMap fields are indexed
 * by packet class.  A FwdMap entry selectes the counter that is used for the
 * forward event for the selected class.  A DrpMap entry selects the the counter
 * for the drop event for the selected class.
 *
 * Example:
 *
 * ulFwdMap[ 5 ] = 1
 *
 * Maps forward counts for packet class 5 to counter 1.
 *
 */
typedef struct sb_qe2000_elib_cmap_s {
    uint32 ulFwdMap[ SB_QE2000_ELIB_NUM_CLASSES_K ];      /**< Forward Counter Map. */
    uint32 ulDrpMap[ SB_QE2000_ELIB_NUM_CLASSES_K ];      /**< Drop Counter Map. */
} SB_QE2000_ELIB_CMAP_ST, *SB_QE2000_ELIB_CMAP_PST;

/**
 * Set a counter map configuration
 *
 * See the documentation for SB_QE2000_ELIB_CMAP_ST for details on the structure of the
 * counter map.
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param ulSelect   The counter map to set.  0 selects counter map 0.  1 selects
 *                   counter map 1.
 *
 * @param Map        The counter map.  This is an SB_QE2000_ELIB_CMAP_ST struct.
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibCmapSet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulSelect, SB_QE2000_ELIB_CMAP_ST Map );

/**
 * Get a counter map configuration
 *
 * See the documentation for SB_QE2000_ELIB_CMAP_ST for details on the structure of the
 * counter map.
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param ulSelect   The counter map to set.  0 selects counter map 0.  1 selects
 *                   counter map 1.
 *
 * @param pMap       The counter map.  Upon a return indication of success, this
 *                   points to the selected counter map.
 *
 * @return           Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibCmapGet( SB_QE2000_ELIB_HANDLE Handle, uint32 ulSelect, SB_QE2000_ELIB_CMAP_PST pMap );


/**
 * Set Pri re-write enable
 *
 * Enable or disable Pri re-write for a port.
 *
 * @param Handle    A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param ulPort    The port id.  Range 0..SB_QE2000_ELIB_NUM_PORTS_K - 1.
 *
 * @param bEnable   TRUE is enabled, FALSE is disabled.
 *
 * @return          Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibPriEnableSet( SB_QE2000_ELIB_HANDLE Handle,
			      uint32 ulPort,
			      bool_t   bEnable );

/**
 * Get Pri re-write enable status
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param ulPort     The port id.  Range 0..SB_QE2000_ELIB_NUM_PORTS_K - 1.
 *
 * @param pbEnable   Then enable status.  Upone a return indication of success,
 *                   this points to the enable status.  TRUE is enabled,
 *                   FALSE is disabled.
 *
 * @return          Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibPriEnableGet( SB_QE2000_ELIB_HANDLE Handle,
			      uint32 ulPort,
			      bool_t   *pbEnable);

/**
 * Set Pri Rewrite configuration for all cos/dp/e values of a given port
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param ulPort     The port id.  Range 0..SB_QE2000_ELIB_NUM_PORTS_K - 1.
 *
 * @param aulPriTable 64 Pri values (index by cos/dp/e)
 *
 * @return           Status, SB_ELIB_OK or failure code
 */
sbElibStatus_et sbQe2000ElibPortPriRewriteSet( SB_QE2000_ELIB_HANDLE Handle,
                                               uint32 ulPort,
                                               uint32 pPortPriTable[64]);


/**
 * Get Pri Rewrite configuration for all cos/dp/e values of a given port
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param ulPort     The port id.  Range 0..SB_QE2000_ELIB_NUM_PORTS_K - 1.
 *
 * @param aulPriTable 64 Pri values (index by cos/dp/e)
 *
 * @return           Status, SB_ELIB_OK or failure code
 */
sbElibStatus_et sbQe2000ElibPortPriRewriteGet( SB_QE2000_ELIB_HANDLE Handle,
                                               uint32 ulPort,
                                               uint32 pPortPriTable[64]);



/**
 * Set Pri re-write configuration
 *
 * There is a Pri memory (3200x3b) that stores rewrite information for the
 * TCI.Pri field.  The rewrite function is enabled on a per port basis.
 *
 * The index for this table is a catenation of (port, cos, dp, ecn).  This
 * creates a 12-bit index of the form:
 *
 * index[11:6] = port[5:0]
 * index[5:3]  = cos[2:0]
 * index[2:1]  = dp[1:0]
 * index[0]    = ecn
 *
 * Since the range of the port[5:0] is limited to 0..49, the range of the
 * index is 0..3199, thus the 3200x3b organization.
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param ulPort     The port id.  Range 0..SB_QE2000_ELIB_NUM_PORTS_K - 1.
 *
 * @param ulCos      The class of service.  Range 0..7.
 *
 * @param ulDp       The drop precedence.  Range 0..3.
 *
 * @param ulEcn      The explicit congenstion notification bit.  Range 0..1.
 *
 * @param ulPri      The Pri to store at the indexed location.  Range 0..7.
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibPriRewriteSet( SB_QE2000_ELIB_HANDLE Handle,
			       uint32 ulPort,
			       uint32 ulCos,
			       uint32 ulDp,
			       uint32 ulEcn,
			       uint32 ulPri );

/**
 * Get Pri re-write configuration
 *
 * See the documentation for sbQe2000ElibPriRewriteSet for details on the organization
 * and indexing of this table.
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param ulPort     The port id.  Range 0..SB_QE2000_ELIB_NUM_PORTS_K - 1.
 *
 * @param ulCos      The class of service.  Range 0..7.
 *
 * @param ulDp       The drop precedence.  Range 0..3.
 *
 * @param ulEcn      The explicit congenstion notification bit.  Range 0..1.
 *
 * @param pulPri     The Pri stored at the indexed location.  Upon a return
 *                   indication of success, this points to a Pri in the range
 *                   of 0..7.
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibPriRewriteGet( SB_QE2000_ELIB_HANDLE Handle,
			       uint32 ulPort,
			       uint32 ulCos,
			       uint32 ulDp,
			       uint32 ulEcn,
			       uint32* pulPri );

/**
 * Multicast Vector Table (MVT) Entry
 */
typedef struct sb_qe2000_elib_mvt_entry_s {
    bool_t bPortEnable[ SB_QE2000_ELIB_NUM_PORTS_K ]; /**< Port Enable array - TRUE means multicast to this port. */
    uint32 ulMvtdA;       /**< MVTD_A value - This is a 14-bit cookie that is available to the EP. */
    uint32 ulMvtdB;       /**< MVTD_B value - this is a 4-bit cookie that is available to the EP. */
    bool_t bSourceKnockout; /**< Enable Source Knockout for this entry */
    uint32 ulNext;        /**< 16-bit Offset to the next MVT 0xFFFF terminates list */
} SB_QE2000_ELIB_MVT_ENTRY_ST, *SB_QE2000_ELIB_MVT_ENTRY_PST;


/**
 * Set an Multicast Vector Table Entry
 *
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param Entry      The MVT entry to be placed in the table.
 *
 * @param ulIndex    The MVT entry index.
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibMVTSet( SB_QE2000_ELIB_HANDLE Handle,
			SB_QE2000_ELIB_MVT_ENTRY_ST Entry,
			uint32 ulIndex );

/**
 * Set an Multicast Vector Table Entry
 *
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param Entry      The MVT entry to be placed in the table.
 * @param ulIndex    The MVT entry index.
 * @param pfSemTryWait Pointer to the Semaphore Get function
 * @param pfSemGive    Pointer to the Semaphore Give function
 * @param nSemId       The semaphore ID
 * @param ulTimeOut    Time out value passed to the sem get
 * @param pUserData    User specified data to be passed to the sem get/give functions
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
int _sbQe2000ElibMVTSet( sbhandle pHalCtx,
                         SB_QE2000_ELIB_MVT_ENTRY_ST Entry,
                         uint32 ulIndex,
                         SB_QE2000_ELIB_SEM_TRYWAIT_PF pfSemTryWait,
                         SB_QE2000_ELIB_SEM_GIVE_PF pfSemGive,
                         int nSemId,
                         uint32 ulTimeOut,
                         void *pUserData);

/**
 * Get an Multicast Vector Table Entry
 *
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param pEntry     The MVT entry stored at the given index.  Upon success, the MVT
 *                   entry will be placed at this location.
 *
 * @param ulIndex    The MVT entry index.
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibMVTGet( SB_QE2000_ELIB_HANDLE Handle,
			SB_QE2000_ELIB_MVT_ENTRY_PST pEntry,
			uint32 ulIndex );

/**
 * Get an Multicast Vector Table Entry
 *
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 * @param pEntry     The MVT entry stored at the given index.  Upon success, the MVT
 *                   entry will be placed at this location.
 * @param ulIndex    The MVT entry index.
 * @param pfSemTryWait Pointer to the Semaphore Get function
 * @param pfSemGive    Pointer to the Semaphore Give function
 * @param nSemId       The semaphore ID
 * @param ulTimeOut    Time out value passed to the sem get
 * @param pUserData    User specified data to be passed to the sem get/give functions
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
int _sbQe2000ElibMVTGet( sbhandle pHalCtx,
                         SB_QE2000_ELIB_MVT_ENTRY_PST pEntry,
                         uint32 ulIndex,
                         SB_QE2000_ELIB_SEM_TRYWAIT_PF pfSemTryWait,
                         SB_QE2000_ELIB_SEM_GIVE_PF pfSemGive,
                         int nSemId,
                         uint32 ulTimeOut,
                         void *pUserData);
/**
 * Read/Modify/Write the Port Enable fields of an MVT Entry
 *
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param Entry      The MVT entry to be placed in the table.
 * @param ulIndex    The MVT entry index.
 * @param pfSemTryWait Pointer to the Semaphore Get function
 * @param pfSemGive    Pointer to the Semaphore Give function
 * @param nSemId       The semaphore ID
 * @param ulTimeOut    Time out value passed to the sem get
 * @param pUserData    User specified data to be passed to the sem get/give functions
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibMVTPortEnableRMW(sbhandle pHalCtx,
                                 SB_QE2000_ELIB_MVT_ENTRY_ST Entry,
                                 uint32 ulIndex,
                                 SB_QE2000_ELIB_SEM_TRYWAIT_PF pfSemTryWait,
                                 SB_QE2000_ELIB_SEM_GIVE_PF pfSemGive,
                                 int nSemId,
                                 uint32 ulTimeOut,
                                 void *pUserData);

/**
 * Read/Modify/Write the MVTD A field of an MVT Entry
 *
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param Entry      The MVT entry to be placed in the table.
 * @param ulIndex    The MVT entry index.
 * @param pfSemTryWait Pointer to the Semaphore Get function
 * @param pfSemGive    Pointer to the Semaphore Give function
 * @param nSemId       The semaphore ID
 * @param ulTimeOut    Time out value passed to the sem get
 * @param pUserData    User specified data to be passed to the sem get/give functions
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibMVTMvtdARMW(sbhandle pHalCtx,
                            SB_QE2000_ELIB_MVT_ENTRY_ST Entry,
                            uint32 ulIndex,
                            SB_QE2000_ELIB_SEM_TRYWAIT_PF pfSemTryWait,
                            SB_QE2000_ELIB_SEM_GIVE_PF pfSemGive,
                            int nSemId,
                            uint32 ulTimeOut,
                            void *pUserData);

/**
 * Read/Modify/Write the MVTD B field of an MVT Entry
 *
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param Entry      The MVT entry to be placed in the table.
 * @param ulIndex    The MVT entry index.
 * @param pfSemTryWait Pointer to the Semaphore Get function
 * @param pfSemGive    Pointer to the Semaphore Give function
 * @param nSemId       The semaphore ID
 * @param ulTimeOut    Time out value passed to the sem get
 * @param pUserData    User specified data to be passed to the sem get/give functions
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibMVTMvtdBRMW(sbhandle pHalCtx,
                            SB_QE2000_ELIB_MVT_ENTRY_ST Entry,
                            uint32 ulIndex,
                            SB_QE2000_ELIB_SEM_TRYWAIT_PF pfSemTryWait,
                            SB_QE2000_ELIB_SEM_GIVE_PF pfSemGive,
                            int nSemId,
                            uint32 ulTimeOut,
                            void *pUserData);

/**
 * Read/Modify/Write the Link field of an MVT Entry
 *
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param Entry      The MVT entry to be placed in the table.
 * @param ulIndex    The MVT entry index.
 * @param pfSemTryWait Pointer to the Semaphore Get function
 * @param pfSemGive    Pointer to the Semaphore Give function
 * @param nSemId       The semaphore ID
 * @param ulTimeOut    Time out value passed to the sem get
 * @param pUserData    User specified data to be passed to the sem get/give functions
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibMVTLinkRMW(sbhandle pHalCtx,
                           SB_QE2000_ELIB_MVT_ENTRY_ST Entry,
                           uint32 ulIndex,
                           SB_QE2000_ELIB_SEM_TRYWAIT_PF pfSemTryWait,
                           SB_QE2000_ELIB_SEM_GIVE_PF pfSemGive,
                           int nSemId,
                           uint32 ulTimeOut,
                           void *pUserData);

/**
 * Read/Modify/Write the Port Enable fields of an MVT Entry
 *
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param Entry      The MVT entry to be placed in the table.
 * @param ulIndex    The MVT entry index.
 * @param pfSemTryWait Pointer to the Semaphore Get function
 * @param pfSemGive    Pointer to the Semaphore Give function
 * @param nSemId       The semaphore ID
 * @param ulTimeOut    Time out value passed to the sem get
 * @param pUserData    User specified data to be passed to the sem get/give functions
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibMVTSrcKnockoutRMW(sbhandle pHalCtx,
                                  SB_QE2000_ELIB_MVT_ENTRY_ST Entry,
                                  uint32 ulIndex,
                                  SB_QE2000_ELIB_SEM_TRYWAIT_PF pfSemTryWait,
                                  SB_QE2000_ELIB_SEM_GIVE_PF pfSemGive,
                                  int nSemId,
                                  uint32 ulTimeOut,
                                  void *pUserData);


/**
 * Clear out (zero) all indirect memories
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE
 *
 * @return           Status.  0 indicates success - Non-Zero indicates failure
 */
sbElibStatus_et sbQe2000ElibMemClear( SB_QE2000_ELIB_HANDLE Handle );


/**
 * Errors - Structure to hold error indications
 *
 */
typedef struct sb_qe2000_elib_error_s {
    bool_t   bBfPortUnmanaged;  /**< Frame arrived for unmanaged port, VLAN */
    uint32 ulUnmanagedVlan;   /**< VLAN associated w/ BfPortUnmanaged Error */
    uint32 ulUnmanagedPort;   /**< Port associated w/ BfPortUnmanaged Error */
    bool_t   bIpSegErr;         /**< Segment Addressing Error */
    bool_t   bBfVlanDisabled;   /**< Frame arrived for Disabled or uninit'ed VLAN */
    uint32 ulDisabledVlan;    /**< VLAN associated w/ Disabled error */
    bool_t   bAmParityErr;      /**< Parity error in CIT, CRT, PT, or PCT */
    bool_t   bClIstackLimit;    /**< Instruction Overflow (config error) */
    bool_t   bPfOverflow;       /**< FIFO Overflow, fatal error */
} SB_QE2000_ELIB_ERROR_ST, *SB_QE2000_ELIB_ERROR_PST;

/**
 * Get the error summary for the QE2000 EP
 *
 * See the documentation for SB_QE2000_ELIB_ERRORS_ST for details on the structure of the errors.
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param pError     Upon success, this points to the errors.
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibErrorsGet( SB_QE2000_ELIB_HANDLE Handle, SB_QE2000_ELIB_ERROR_PST pError );


/**
 * Clear errors within the QE2000 EP
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @param bBfPortUnmanaged  Clear BfPortUnmanaged Error.
 * @param bIpSegErr         Clear Ip Segment Error.
 * @param bBfVlanDisabled   Clear BfVlanDisabled Error.
 * @param bAmParityErr      Clear Parity error in CIT, CRT, PT, or PCT.
 * @param bClIstackLimit    Clear Instruction Overflow Error.
 * @param bPfOverflow       Clear FIFO Overflow Error.
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibErrorsClear( SB_QE2000_ELIB_HANDLE Handle,
			     bool_t bBfPortUnmanaged,
			     bool_t bIpSegErr,
			     bool_t bBfVlanDisabled,
			     bool_t bAmParityErr,
			     bool_t bClIstackLimit,
			     bool_t bPfOverflow);


/**
 * Retrieve per class packet count
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 * @param pulPktCnt  upon success the packet count for each class
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibClassPktCountGet( SB_QE2000_ELIB_HANDLE Handle,
				  uint64 pullPktCnt[SB_QE2000_ELIB_NUM_CLASSES_K]);

/**
 * Poll the Elib Counters.
 *
 * This function polls the appropriate counters within the EP and accumulates
 * them for user access.  To ensure no counters wrap, this function should be
 * called every 10 seconds.
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 *
 * @return           Status.  0 indicates success.  Non-zero indicates failure.
 */
sbElibStatus_et sbQe2000ElibCountsPoll( SB_QE2000_ELIB_HANDLE Handle );


/**
 * Dump the Vlan Memory Management Node List.
 *
 * This function dumps the VLAN MM node list (free and used nodes) via
 * SOC_DEBUG_PRINT.  Useful for low level debugging only.
 *
 * @param Handle    A valid SB_QE2000_ELIB_HANDLE.
 *
 * @return          Status, SB_ELIB_OK, always.
 */
sbElibStatus_et sbQe2000ElibVlanMemListOutput( SB_QE2000_ELIB_HANDLE Handle );

/**
 * Dump the Vlan Memory Management Free Tree (avl balanced tree)
 *
 * This function dumps the VLAN MM Free Tree via
 * SOC_DEBUG_PRINT.  Useful for low level debugging only.
 *
 * @param Handle    A valid SB_QE2000_ELIB_HANDLE.
 *
 * @return          Status, SB_ELIB_OK, always.
 */
sbElibStatus_et sbQe2000ElibVlanMemFreeTreeList( SB_QE2000_ELIB_HANDLE Handle );


/**
 * Retrieve the Vlan Memory Manager Statistics
 *
 * @param Handle    A valid SB_QE2000_ELIB_HANDLE.
 * @param pZf       Pointer to a sbZfSbQe2000ElibVlanMem_t
 *
 * @return          Status, SB_ELIB_OK, alwyas.
 */
sbElibStatus_et sbQe2000ElibVlanMemCheck( SB_QE2000_ELIB_HANDLE Handle,
                                          sbZfSbQe2000ElibVlanMem_t *pZf);

#endif
