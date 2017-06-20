#ifndef __SB_EPLIB_H__
#define __SB_EPLIB_H__
/**
 * @file sbG2Eplib.h Guadalupe2k Egress Library Public API
 *
 * <pre>
 * ====================================================
 * ==  sbG2Eplib.h - Guadalupe2k Egress Library public API
 * ====================================================
 *
 * WORKING REVISION: $Id: sbG2Eplib.h,v 1.6 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * MODULE NAME:
 *
 *     sbG2Eplib.h
 *
 * ABSTRACT:
 *
 *     Guadalupe2k Egress Library public API
 *
 * LANGUAGE:
 *
 *     C
 *
 * AUTHORS:
 *
 *     Ning
 *
 * CREATION DATE:
 *
 *     15-April-2007
 *
 * </pre>
 */


/* sandburst system types */
#include <sbWrappers.h>
#include <glue.h>
#include <sbTypes.h>

/* kamino related includes */
#include <hal_ka_auto.h>
#include <hal_ka_inline.h>

/* egress library includes */
#include <sbQe2000Elib.h>
#include "sbElibStatus.h"

/* G2 elib includes */
#include "sbG2EplibContext.h"

/* auto-generated zframes */
#include "sbG2EplibZf.h"

/*
 * User DMA functions and structures needed by the lower level
 */
typedef enum sb_qe2000_elib_dma_stats_mem_t sbG2EplibDmaStatsMem_t;
typedef struct sb_qe2000_elib_dma_func_params_s sbG2EplibDmaFuncParams_s;
typedef SB_QE2000_ELIB_USER_DMA_PF sbG2EplibUserDmaFunc_pf;

/*
 * User supplied semaphore access needed by the lower level
 */
typedef SB_QE2000_ELIB_SEM_CREATE_PF  sbG2EplibUserSemCreate_pf;
typedef SB_QE2000_ELIB_SEM_GIVE_PF    sbG2EplibUserSemGive_pf;
typedef SB_QE2000_ELIB_SEM_TRYWAIT_PF sbG2EplibUserSemWaitGet_pf;
typedef SB_QE2000_ELIB_SEM_DESTROY_PF sbG2EplibUserSemDestroy_pf;


/**
 * Per vlan counter structure
 **/
typedef struct sb_qe2000_elib_counter_s sbG2EplibVlanCounters_s, sbG2EplibCounter_s;


/*
 * Helpful macro for filling in zframes
 */
#define MAC_HI_FROM_BYTES(puc) ( (puc[0] << 8) | (puc[1]) )
#define MAC_LO_FROM_BYTES(puc) ( (puc[2] << 24) | (puc[3] << 16) | (puc[4] << 8) | (puc[5]) )

/**
 *
 * Enumerated Type of the different segments of IP memory.
 *
 **/
typedef enum sbG2EplibIpSegs_es {
    SEG_HQOS_REMAP = 0,   /**< H-QoS Remap Segment */
    SEG_VLAN_REMAP,       /**< VLAN H-QoS Remap Segment */
    SEG_PORT_REMAP,       /**< Port H-Qos Remap Segment */
    /* leave as last */
    SEG_SEGMENTS
} sbG2EplibIpSegs_et;

/**
 * Initialization Parameters
 */
typedef struct sbG2EplibInitParam_s
{
    uint64                  ullERHForward;                /**< Bitwise per port enable, allows ERH to be forwarded with the frame */
    uint64                  ullSmacMsbEnable;             /**< Bitwise per port enable */
    sbG2EplibUserDmaFunc_pf    pUserDmaFunc;                 /**< User Supplied DMA Function, if NULL use PIO for stats transfer */
    void*                     pUserData;                    /**< User Supplied DMA Function Helper Data */
    sbG2EplibUserSemCreate_pf  pUserSemCreateFunc;           /**< User Supplied Semaphore Creation Function, if NULL no semaphore guarding will be used for statistics access */
    sbG2EplibUserSemGive_pf    pUserSemGiveFunc;             /**< User Supplied Semaphore Give Function */
    sbG2EplibUserSemWaitGet_pf pUserSemWaitGetFunc;          /**< User Supplied Semaphore Get w/TimeOut Function */
    sbG2EplibUserSemDestroy_pf pUserSemDestroyFunc;          /**< User Supplied Semaphore Destroy Function */
    void*                     pUserSemData;                 /**< User Supplied Semaphore Data, parameter to all semaphore functions */
    void*                     pHalCtx;                      /**< Pointer to the device */
    int                       nPorts;                       /**< Number of ports for this device */
    int                       nOnePcwEna;                   /**< Flag to enable single PCW in the VLAN Record Table (VRT) - Can only be used if the system uses ports 0-23, 48, & 49 */
    void*                     pMVTUserSemData;              /**< User supplied semaphore void pointer for use with the MVT */
    int                       nMVTSemId;                    /**< Semaphore ID supplied by the user to negogiate access to the MVT */
    int                       nMVTTimeOut;                  /**< Time out value for the semaphore try & wait for use with the MVT semaphore */
    sbZfG2EplibIpSegment_t tIpSegment[SEG_SEGMENTS]; /**< Default Ip Segment memory configuration */
} sbG2EplibInitParams_st, *sbG2EplibInitParams_pst;



/**
 *
 *  Guadalupe2k Specific Egress Processor Parameter Initialization
 *
 *  @param  pParams - initialization parameters structure to be initialized
 *  @return status  - error code, zero on success
 **/
sbElibStatus_et
sbG2EplibDefParamsGet(sbG2EplibInitParams_pst pParams);

/**
 * sbG2EplibIpSegmentTableSet()
 *
 *  Set Ip Segment Table entry information in initialization parameters.
 *  Must be called AFTER sbG2EplibDefParamsGet as DefParamsGet initializes
 *  the segment information to default values.
 *
 *  @param  pParams - initialization parameters structure to be initialized
 *  @param  nSeg    - segment to operation on.
 *  @param  pZf     - pointer to a zframe that contains new segment memory information.
 *  @return status  - error code, zero on success
 **/
sbElibStatus_et
sbG2EplibIpSegmentTableSet(sbG2EplibInitParams_pst pParams, sbG2EplibIpSegs_et nSeg,
                          sbZfG2EplibIpSegment_t *pZf);

/**
 *
 *  Guadalupe2k Specific Egress Processor Initialization sbG2EplibDefParamsGet
 *  MUST be called before this function as it defines the ip segment memory
 *  table.
 *
 *  @param  pCtxt   - Pointer to the Egress context structure
 *  @param  pParams - Initialization Parameters
 *  @return status  - error code, zero on success
 *
 **/
sbElibStatus_et
sbG2EplibInit(sbG2EplibCtxt_pst pEgCtxt, sbG2EplibInitParams_pst pParams);

/**
 *
 *  Guadalupe2k Specific Egress Processor Uninitialization
 *
 *  @param  pCtxt  - Pointer to the Egress context structure
 *  @return status - error code, zero on success
 *
 **/
sbElibStatus_et
sbG2EplibUnInit(sbG2EplibCtxt_pst pCtxt);

/**
 *
 *  Define an entry in the HQosRemap Table that is used to compose 
 *  a packet. This table is located in
 *  the IP memory, and contains H-Qos Information that will be used later by
 *  the Bridging Function (BF)
 *
 *  @param  pCtxt  - Pointer to the Egress context structure
 *  @param  ulIdx  - Index into the H-Qos Table in the IP memory
 *  @param  pData  - entry contains the H-Qos remap info
 *  @return status - error code, zero on success
 *
 **/
sbElibStatus_et
sbG2EplibHQosRemapSet(sbG2EplibCtxt_pst pCtxt, uint32 ulIdx,
		      uint32 * pData);

/**
 *
 *  Retrieve an entry in the HQosRemap Table that is used to compose 
 *  a packet. This table is located in
 *  the IP memory, and contains Qos Information that will be used later by
 *  the Bridging Function (BF)
 *
 *  @param  pCtxt  - Pointer to the Egress context structure
 *  @param  ulIdx  - Index into the H-Qos Table in the IP memory
 *  @param  pData  - entry contains the H-Qos remap info
 *  @return status - error code, zero on success
 *
 **/
sbElibStatus_et
sbG2EplibHQosRemapGet(sbG2EplibCtxt_pst pCtxt, uint32 ulIdx,
		      uint32* pData);

/**
 *
 *  Define an entry in the VlanRemap Table that is used to compose 
 *  a packet. This table is located in
 *  the IP memory, and contains vlan Information that will be used later by
 *  the Bridging Function (BF)
 *
 *  @param  pCtxt  - Pointer to the Egress context structure
 *  @param  ulIdx  - Index into the Vlan Table in the IP memory
 *  @param  pData  - entry contains the vlan remap info
 *  @return status - error code, zero on success
 *
 **/
sbElibStatus_et
sbG2EplibVlanRemapSet(sbG2EplibCtxt_pst pCtxt, uint32 ulIdx,
		      uint32 * pData);

/**
 *
 *  Retrieve an entry in the VlanRemap Table that is used to compose 
 *  a packet. This table is located in
 *  the IP memory, and contains Vlan Information that will be used later by
 *  the Bridging Function (BF)
 *
 *  @param  pCtxt  - Pointer to the Egress context structure
 *  @param  ulIdx  - Index into the TciDMAC Table in the IP memory
 *  @param  pData  - entry contains the vlan remap info
 *  @return status - error code, zero on success
 *
 **/
sbElibStatus_et
sbG2EplibVlanRemapGet(sbG2EplibCtxt_pst pCtxt, uint32 ulIdx,
		      uint32* pData);

/**
 *
 *  Define an entry in the PortEncap Table that is used to compose 
 *  a packet. This table is located in
 *  the IP memory, and contains port encap Information that will be used by
 *  the Bridging Function (BF)
 *
 *  @param  pCtxt  - Pointer to the Egress context structure
 *  @param  ulIdx  - Index into the PortEncap Table in the IP memory
 *  @param  pData  - entry contains the port encap info
 *  @return status - error code, zero on success
 *
 **/
sbElibStatus_et
sbG2EplibPortEncapSet(sbG2EplibCtxt_pst pCtxt, uint32 ulIdx,
		      uint32 * pData);

/**
 *
 *  Retrieve an entry in the PortEncap Table that is used to compose 
 *  a packet. This table is located in
 *  the IP memory, and contains port encap Information that will be used by
 *  the Bridging Function (BF)
 *
 *  @param  pCtxt  - Pointer to the Egress context structure
 *  @param  ulIdx  - Index into the PortEncap Table in the IP memory
 *  @param  pData  - entry contains the port encap info
 *  @return status - error code, zero on success
 *
 **/
sbElibStatus_et
sbG2EplibPortEncapGet(sbG2EplibCtxt_pst pCtxt, uint32 ulIdx,
		      uint32* pData);

/**
 * MVT Type
 **/
typedef enum sbG2EplibMVTType_e
{
    SBGU_ELIB_MVT_BRIDGING = 0, /**< MVT Entry is Bridging */
    SBGU_ELIB_MVT_IPV4 = 1,     /**< MVT Entry is IPV4 */
    SBGU_ELIB_MVT_IPV6 = 2,     /**< MVT Entry is IPV6 */
    SBGU_ELIB_MVT_MPLS = 3      /**< MVT Entry is MPLS */
} sbG2EplibMVTType_t;


/**
 *
 * Set an Multicast Vector Table Entry
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 * @param ulMcGroup  The MVT entry index.
 * @param pZf        Pointer to an MVT entry zframe.
 * @return           Error code, zero on success.
 *
 **/
sbElibStatus_et
sbG2EplibMVTSet(sbG2EplibCtxt_pst pCtxt, uint32 ulMcGroup,
               sbZfG2EplibMvtEntry_t *pZf);

/**
 *
 * Get an Multicast Vector Table Entry
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 * @param ulMcGroup  The MVT entry index.
 * @param pZf        The MVT entry stored at the given index.  Upon success, the MVT
 *                     entry will be placed at this location.
 * @return           Error code, zero on success.
 *
 **/
sbElibStatus_et
sbG2EplibMVTGet(sbG2EplibCtxt_pst pCtxt, uint32 ulMcGroup,
	       sbZfG2EplibMvtEntry_t *pZf);

/**
 *
 *  Define a port configuration.  This is used to setup a port on the egress
 *
 *  @param  pCtxt  - Pointer to the Egress context structure.
 *  @param  ulPort - Port number associated with this configuration.
 *  @return status - Error code, zero on success.
 *
 **/
sbElibStatus_et
sbG2EplibPortConfigSet(sbG2EplibCtxt_pst pCtxt, uint32 ulPort);

/**
 *
 *  Retrieve a port's configuration.
 *
 *  @param  pCtxt  - Pointer to the Egress context structure.
 *  @param  ulPort - Port number associated with this configuration.
 *  @return status - Error code, zero on success.
 *
 **/
sbElibStatus_et
sbG2EplibPortConfigGet(sbG2EplibCtxt_pst pCtxt, uint32 ulPort);

/**
 * Per class counter structure
 **/
typedef struct sbG2EplibPCTCounters_s
{
    uint64 pullPacketCnt[SB_QE2000_ELIB_NUM_CLASSES_K]; /**< Per class packet counters */
    uint64 pullByteCnt[SB_QE2000_ELIB_NUM_CLASSES_K];   /**< Per class byte counters */
} sbG2EplibPCTCounters_st, *sbG2EplibPCTCounters_pst;

/**
 *
 *  Get the per class counters for a port
 *
 *  @param  pCtxt              - Pointer to the Egress context structure.
 *  @param  ulPort             - Port number for which to get the counters.
 *  @param  pPCTCounts         - Pointer to PCTCounter structure.  Upon success this structure is
 *                               filled with the per class counts.
 *  @return status             - Error code, zero on success.
 *
 **/
sbElibStatus_et
sbG2EplibPortCountGet(sbG2EplibCtxt_pst pCtxt, uint32 ulPort, sbG2EplibPCTCounters_pst pPCTCounts);


/**
 *
 * Accumulate All Counts (vlan, port count table, and per class aggregate counts)
 *
 *  @param  pCtxt        - Pointer to the Egress context structure.
 *  @param  ulNumScIdx   - Number of Switching Contexts to accumulate for.
 *
 *  @return status         - Error code, zero on success.
 *
 **/
sbElibStatus_et
sbG2EplibCountAccumulate(sbG2EplibCtxt_pst pCtxt, uint32 ulNumScIdx);

/**
 *
 * Accumulate ulNumCounterSets counters (port count table, per class
 * aggregate counts, and vlan)
 *
 *  @param  pCtxt              - Pointer to the Egress context structure.
 *  @param  ulNumCounterSets   - Number of Counter Sets (pkt & byte) to accumulate for.
 *
 *  @return  status             - Error code, zero on success.
 **/
sbElibStatus_et
sbG2EplibCounterSetsAccumulate(sbG2EplibCtxt_pst pCtxt, uint32 ulNumCounterSets);

/**
 *  Retrieve to/from proc counter pairs from ip segment memory.
 *
 *  @param pCtxt         - Pointer to the Egress context structure.
 *  @param pProcCounter  - Array of 2 counter structures.
 *
 *  @return status       - Error code, zero on success
 **/
sbElibStatus_et
sbG2EplibProcCountGet(sbG2EplibCtxt_pst pCtxt,
		      sbG2EplibCounter_s pProcCounter[2]);

/**
 *  Retrieve Drop Class counter pair from ip segment memory.
 *
 *  @param pCtxt         - Pointer to the Egress context structure.
 *  @param pProcCounter  - Pointer to a counter structures.
 *
 *  @return status       - Error code, zero on success
 **/
sbElibStatus_et
sbG2EplibDropClassCountGet(sbG2EplibCtxt_pst pCtxt,
			   sbG2EplibCounter_s *pProcCounter);

/**
 *  Retrieve TciDmac counter pairs from IP segment memory.
 *  The ram access logic will clear-on-read the address.
 *
 *  @param pCtxt   - Pointer to the Egress context structure.
 *  @param pCnt    - Counter structures for bytes/packets
 *  @param nIdx    - Index into TciDmac table
 *
 *  @return status - Error code, zero on success
 **/
sbElibStatus_et
sbG2EplibTciDmacCountGet(sbG2EplibCtxt_pst pCtxt,
			 sbG2EplibCounter_s *pCnt,
			 uint32 nIdx);

sbElibStatus_et
sbG2EplibTciDmacCountReset(sbG2EplibCtxt_pst pCtxt, uint32 nIdx);

typedef enum sbG2EplibVlanCountIndex_e
{
    SBGU_ELIB_CMAP0_FWD = 0,
    SBGU_ELIB_CMAP0_DRP,
    SBGU_ELIB_CMAP1_L2_FWD,
    SBGU_ELIB_CMAP1_L3_FWD,
    SBGU_ELIB_CMAP1_OTHER_FWD,
    SBGU_ELIB_CMAP1_DRP
} sbG2EplibVlanCountIndex_t;

/**
 *
 *  Retrieve the CMAP Counters for a Vlan
 *
 *  @param  pCtxt          - Pointer to the Egress context structure.
 *  @param  ulScIdx        - The Switching Context associated with the VLAN.
 *  @param  tCountIdx      - Which count to retrieve.
 *  @param  bClrOnRd       - Clear the counters after reading them.
 *  @param  pVlanCounter   - Pointer to a single counter structure.
 *                           port counts for the associated Switching Context.
 *  @return status         - Error code, zero on success.
 *
 **/
sbElibStatus_et
sbG2EplibVlanCmapCountGet(sbG2EplibCtxt_pst pCtxt, uint32 ulScIdx,
                         sbG2EplibVlanCountIndex_t tCountIdx, bool_t bClrOnRd,
                         sbG2EplibVlanCounters_s *pVlanCounter);

/**
 *
 *  Retrieve a port's Counters for a Vlan
 *
 *  @param  pCtxt          - Pointer to the Egress context structure.
 *  @param  ulScIdx        - The Switching Context associated with the VLAN.
 *  @param  ulPort         - Which port count to retrieve.
 *  @param  bClrOnRd       - Clear the counters after reading them.
 *  @param  pVlanCounter   - Pointer to a single counter structure.
 *                           port counts for the associated Switching Context.
 *  @return status         - Error code, zero on success.
 *
 **/
sbElibStatus_et
sbG2EplibVlanPortCountGet(sbG2EplibCtxt_pst pCtxt, uint32 ulScIdx,
                         uint32 ulPort, bool_t bClrOnRd,
                         sbG2EplibVlanCounters_s *pVlanCounter);


/**
 *
 *  Retrieve the full counter set for a VLAN
 *
 *  @param pCtxt         - Pointer to the Egress context structure.
 *  @param ulScIdx       - The Switching Context associated with the VLAN
 *  @param aVlanCounters - Array of Counter structures, large enough to hold class and
 *                         port counts for the associated Switching Context.
 *
 *  @return status        - Error code, zero on success
 *
 **/
sbElibStatus_et
sbG2EplibVlanCountGetAll(sbG2EplibCtxt_pst pCtxt, uint32 ulScIdx,
                        bool_t bClrOnRd,
                        sbG2EplibVlanCounters_s aVlanCounters[SB_QE2000_ELIB_MAX_COUNTERS_K]);



/**
 *
 *  Accumulate & Retrieve the full counter set for a VLAN
 *
 *  @param pCtxt         - Pointer to the Egress context structure.
 *  @param ulScIdx       - The Switching Context associated with the VLAN
 *  @param aVlanCounters - Array of Counter structures, large enough to hold class and
 *                         port counts for the associated Switching Context.
 *
 *  @return status        - Error code, zero on success
 **/
sbElibStatus_et
sbG2EplibVlanCountAccumulateGet(sbG2EplibCtxt_pst pCtxt, uint32 ulScIdx,
                               bool_t bClrOnRd,
                               sbG2EplibVlanCounters_s aVlanCounters[SB_QE2000_ELIB_MAX_COUNTERS_K]);



/* --------------------------------------------------------------------------
 * Wrappers for exported QE2000 Eplib structures
 * -------------------------------------------------------------------------- */
struct sb_qe2000_elib_error_s;
typedef struct sb_qe2000_elib_error_s sbG2EplibError_t;
struct sb_qe2000_elib_mvt_entry_s;
typedef struct sb_qe2000_elib_mvt_entry_s sbG2EplibMVTEntry_t;

/* --------------------------------------------------------------------------
 * Wrappers for Exported QE2000 Eplib functions
 * -------------------------------------------------------------------------- */

/**
 *
 * Set VLAN Encapsulation Tags
 *
 * @param pHandle    Eplib Handle
 * @param ulVlanTagA VLAN Tag A.
 * @param ulVlanTagB Vlan Tag B.
 * @return           Error code, zero on success
 *
 **/
sbElibStatus_et
sbG2EplibVlanEncapSet(sbG2EplibCtxt_pst pCtxt, uint32 ulVlanTagTypeA,
		     uint32 ulVlanTagTypeB);

/**
 *
 * Get VLAN Encapsulation Tags
 *
 * @param pHandle     Eplib Handle
 * @param pulVlanTagA Upon success VLAN Tag A will be placed in this location.
 * @param pulVlanTagB Upon success Vlan Tag B will be placed in this location.
 * @return            Error code, zero on success
 *
 **/
sbElibStatus_et
sbG2EplibVlanEncapGet(sbG2EplibCtxt_pst pCtxt, uint32 *pulVlanTagTypeA,
		     uint32 *pulVlanTagTypeB);

/**
 *
 * Set VLAN Encapsulation Tags
 *
 * @param pHandle    Eplib Handle
 * @param ulMpls     Ethernet type for MPLS
 * @return           Error code, zero on success
 *
 **/
sbElibStatus_et
sbG2EplibMplsEncapSet(sbG2EplibCtxt_pst pCtxt, uint32 ulMpls);

/**
 *
 * Get VLAN Encapsulation Tags
 *
 * @param pHandle     Eplib Handle
 * @param pulMpls     Upon success Ethernet type for MPLS will be placed in this location.
 * @return            Error code, zero on success
 *
 **/
sbElibStatus_et
sbG2EplibMplsEncapGet(sbG2EplibCtxt_pst pCtxt, uint32 *pulMpls);

/**
 *
 * Push previously set CRT entries to the device.
 *
 * The Class Resolution Table (CRT) is indexed by a 10-bit (0..1023) value that is user-defined.
 * This function synchronizes the local copy of the CRT with the device.  All previously set
 * CRT entries are pushed.
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 * @param bAll       Flush all CRT entries, not just the lines marked as dirty.
 * @return           Error code, zero on success.
 *
 **/
sbElibStatus_et
sbG2EplibCRTCommit(sbG2EplibCtxt_pst pCtxt, bool_t bAll);

/**
 *
 * Retrieve per class packet count
 *
 * @param Handle     A valid SB_QE2000_ELIB_HANDLE.
 * @param pulPktCnt  upon success the packet count for each class
 * @return           Error code, zero on success
 *
 **/
sbElibStatus_et
sbG2EplibClassPktCountGet(sbG2EplibCtxt_pst pCtxt,
			 uint64 pullPktCnt[SB_QE2000_ELIB_NUM_CLASSES_K]);

/**
 * Get Eplib version information
 *
 * @param   pLibVersion Pointer to an sbSwLibVersion_t.
 *
 * @return  SB_OK.
 */
extern sbStatus_t
sbG2EplibVersionGet(sbSwLibVersion_p_t pLibVersion);

/**
 *  Remove all VLAN entries, reinitialize associated memory
 *
 *  @param Handle   handle to the elib context structure.
 *  @return         Error code, zero on success
 */
sbElibStatus_et
sbG2EplibVlanFlush(sbG2EplibCtxt_pst pCtxt);

/**
 *  Clear all counters.
 *
 *  @param Handle   handle to the elib context structure.
 *  @return         Error code, zero on success
 */
sbElibStatus_et
sbG2EplibCountersReset(sbG2EplibCtxt_pst pCtxt);



#endif /* __SB_EPLIB_H__ */
