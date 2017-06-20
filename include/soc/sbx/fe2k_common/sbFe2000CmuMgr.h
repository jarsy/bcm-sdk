/* -*- mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */
/*
 * $Id: sbFe2000CmuMgr.h,v 1.10 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * sbFe2000CmuMgr.h : FE2000 Counter Management Unit
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SB_FE2K_CMU_MGR_H_
#define _SB_FE2K_CMU_MGR_H_

#include "sbTypes.h"
#include <soc/sbx/fe2k_common/sbFe2000DmaMgr.h> /* xxx temporary */

/*-----------------------------------------------------------------------------
 * Module Related Types
 *-----------------------------------------------------------------------------*/
#define SB_FE2000_SEG_TBL_SEG_BANKS_MAX (0x4)
#define SB_FE2000_SEG_TBL_SEG_MAX       (0x1f)

/* cm_segment_table_cfg.cntr_type */
typedef enum sbFe2000SegTblType_e_s {
  SB_FE2000_SEG_TBL_CNTR_TYPE_LEGACY = 0,
  SB_FE2000_SEG_TBL_CNTR_TYPE_CHAINED_LEGACY,
  SB_FE2000_SEG_TBL_CNTR_TYPE_TURBO,
  SB_FE2000_SEG_TBL_CNTR_TYPE_RANGE,
  /* leave as last */
  SB_FE2000_SEG_TBL_CNTR_TYPE_MAX
} sbFe2000SegTblType_e_t;

/* cm_segment_table_cfg.eject */
typedef enum sbFe2000SegTblEject_e_s {
  SB_FE2000_SEG_TBL_CNTR_EJECT_PCI_RAM = 0,
  SB_FE2000_SEG_TBL_CNTR_EJECT_MMU_RAM,
  /* leave as last */
  SB_FE2000_SEG_TBL_CNTR_EJECT_MAX
} sbFe2000SegTblEject_e_t;

/* mm_client_config.cm0_memory_config */
typedef enum sbFe2000MmuCmuConfig_e_s {
  SB_FE2000_MM_CMU_CFG_NONE = 0,
  SB_FE2000_MM_CMU_CFG_INTERNAL,
  SB_FE2000_MM_CMU_CFG_EXTERNAL,
  SB_FE2000_MM_CMU_CFG_INVALID
} sbFe2000MmuCmuConfig_e_t;

/* mmX_prot_schemeX encoding */
typedef enum sbFe2000CmuProtScheme_e_s {
  SB_FE2000_MM_PROT_SCH_36D_0P = 0,
  SB_FE2000_MM_PROT_SCH_35D_1P,
  SB_FE2000_MM_PROT_SCH_34D_2P,
  SB_FE2000_MM_PROT_SCH_30D_4P,
  SB_FE2000_MM_PROT_SCH_29D_7P,
  SB_FE2000_MM_PROT_SCH_INV_0,
  SB_FE2000_MM_PROT_SCH_INV_1
} sbFe2000CmuProtScheme_e_t;

/* Forward declare the CMU structure here. The actual implementation
 * is intentionally hidden (file-scope) in sbFe2000CmuMgr.c */
typedef struct sbFe2000CmuMgr_s sbFe2000CmuMgr_t;

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuSwInit()
 *
 * @brief Perform the counter manager software initialization. This will prepare
 *        the driver that will handle counter updates that are ejected from the
 *        CMU. This initialization does not touch the state of the FE-2000. All
 *        initializations that affec the hardware are performed later by the
 *        hardware initialization.
 *
 * @param pCtl - Referenced pointer to CMU control structure
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuSwInit(sbFe2000CmuMgr_t **ppCtl,
                  uint32 ulRingSize,
                  uint32 ulRingThresh,
                  void *hChipHandle,
                  sbFe2000DmaGetHostBusAddress_f_t v2p,
                  sbFe2000DmaMgr_t *pDmaState); /* xxx remove me (temporary) */


/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuSwUninit()
 *
 * @brief Perform the counter manager software uninitialization. This will 
 *        clear the host programmed memory pointers for chip to host 
 *        communication, and disable such featurs.  Useful when performing
 *        a warm boot.
 *
 * @param pCtl - Referenced pointer to CMU control structure
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuSwUninit(sbFe2000CmuMgr_t **pCtl);

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuHwUninit()
 *
 * @brief Perform the counter manager software uninitialization. This will 
 *        clear the host programmed memory pointers for chip to host 
 *        communication, and disable such featurs.  Useful when performing
 *        a warm boot.
 *
 * @param pCtl - Referenced pointer to CMU control structure
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuHwUninit(sbFe2000CmuMgr_t *pCtl);

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuWbRecover()
 *
 * @brief This function is used to recover the CMU during a WB operation.  Disable counter ejection
 * from the CMU.  This function assumes no CMU state structure is available.
 *
 * @param unit
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuWbRecover(int unit);

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuHwInit()
 *
 * @brief Perform the counter manager hardware initialization
 *
 * @param pCtl - Pointer to CMU control structure
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuHwInit(sbFe2000CmuMgr_t *pCtl, uint32 warmboot);

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuRingProcess()
 *
 * @brief Process entries from the CMU ring counter structure. This will empty
 *        entries from the ring update structure and synchronize their values
 *        with the shadowed 64-bit host-side counters. This needs to be called
 *        periodically, either by interrupt or polling to guarantee the ring
 *        has room for the CMU to operate.
 *
 * @param pCtl - Pointer to CMU control structure
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuRingProcess(sbFe2000CmuMgr_t *pCtl, int flush);

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuSegmentSet()
 *
 * @brief Write a set of CMU segment control registers to prepare it for use.
 *
 * @param pCtl       - Pointer to CMU control structure
 * @param ulSegment  - Segment Id (0 -> 31)
 * @param ulBank     - Bank Id (0 -> 3)
 * @param ulBankBase - Base address for this segment in the device memory
 * @param ulPciBase  - Base address for this segment in host PCI memory
 * @param ulTblLimit - Max number of table entries in this segment
 * @param eType      - Counter Type (0=Turbo, 1=Range, 2=Legacy, 3=Chained)
 * @param eEject     - Eject Type (0=External PCI Memory, 1=MMU Int Memory)
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuSegmentSet(sbFe2000CmuMgr_t *pCtl,
                      uint32 ulSegment,
                      uint32 ulBank,
                      uint32 ulBankBase,
                      uint32 ulPciBase,
                      uint32 ulTblLimit,
                      sbFe2000SegTblType_e_t eType,
                      sbFe2000SegTblEject_e_t eEject);

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuSegmentEnable()
 *
 * @brief Enable or disable a segment
 *
 * @param pCtl       - Pointer to CMU control structure
 * @param ulSegment  - Segment Id (0 -> 31)
 * @param bEnable    - Enable = 1, Disable = 0
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuSegmentEnable(sbFe2000CmuMgr_t *pCtl,
                         uint32 ulSegment,
                         uint8 bEnable);

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuRanceConfig()
 *
 * @brief Configure the CMU's RANdomized Counter Ejection (RANCE).
 *
 * @param pCtl             - Pointer to CMU control structure
 * @param Legacy Prob En   - Enable static eject prob for legacy config
 * @param Byte Prob En     - Enable static eject prob for byte part of turbo config
 * @param Packet Prob En   - Enable static eject prob for packet part of turbo config
 * @param LegacyDivLog     - If static_legacy_prob_en is set, the packet accumulation
 *                           amount is shifted to the right by byte_divisor_log bit
 *                           positions in order to obtain the eject probability.
 * @param ByteDivisorLog   - If static_byte_prob_en is set, the packet accumulation
 *                           amount is shifted to the right by byte_divisor_log bit
 *                           positions in order to obtain the eject probability.
 * @param PacketDivisorLog - If static_packet_prob_en is set, the packet accumulation
 *                           amount is shifted to the right by packet_divisor_log bit
 *                           positions in order to obtain the eject probability.
 * @param Seed0            - Seed for Linear Feedback Shift Register 0
 * @param Seed1            - Seed for Linear Feedback Shift Register 1
 * @param Seed2            - Seed for Linear Feedback Shift Register 2
 * @param Seed3            - Seed for Linear Feedback Shift Register 3
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuRanceConfig(sbFe2000CmuMgr_t *pCtl,
                       uint8 bStaticLegacyProbEn,
                       uint8 bStaticByteProbEn,
                       uint8 bStaticPacketProbEn,
                       uint32 ulLegacyDivisorLog,
                       uint32 ulByteDivisorLog,
                       uint32 ulPacketDivisorLog,
                       uint32 ulSeed0,
                       uint32 ulSeed1,
                       uint32 ulSeed2,
                       uint32 ulSeed3);

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuSegmentFlush()
 *
 * @brief Enable or disable flushing (Forced Counter Ejection) on a specified
 *        segment. When this is enabled, the counter manager will cycle through
 *        a segment and serially eject counters for handling. When it reaches
 *        the end of a segment, it will wrap and continue.
 *
 * @param pCtl       - Pointer to CMU control structure
 * @param ulSegment  - Segment to perform flushing on
 * @param bEnable    - Enable or disable ejection on this segment
 * @param ulRateData - Configure rate of counter ejection.
 *
 * @return error code, SB_OK on success
 *-----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuSegmentFlush(sbFe2000CmuMgr_t *pCtl,
                        uint32 ulSegment,
                        uint8 bEnable,
                        uint32 ulRateData);

sbStatus_t
sbFe2000CmuSegmentClear(sbFe2000CmuMgr_t *pCtl,
                        uint32 ulSegment);

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuSegmentFlushStatus()
 *
 * @brief Return current index in flush progress for a segment. If no flush is
 *        is in progress, return BUSY error code.
 *
 * @param pCtl       - Pointer to CMU control structure
 * @param ulSegment  - Segment performing flushing
 * @param ulCntrId   - Current location in segment space
 *
 * @return error code, SB_OK on success
 *-----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuSegmentFlushStatus(sbFe2000CmuMgr_t *pCtl,
                              uint32 ulSegment,
                              uint32 *ulCntrId);

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuForceFlush()
 *
 * @brief Request a manual flush of a specified counter or counters to the host
 *        for handling. Currently, the hardware only supports ranges (n -> 0)
 *        but we should be adding support for individual counters.
 *
 * @param pCtl      - Pointer to CMU control structure
 * @param ulSegment - Segment to perform the forced flush on
 * @param ulOffset  - Offset within that segment
 * @param ulSize    - Number of entries to flush
 *
 * @return error code, SB_OK on success
 *-----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuForceFlush(sbFe2000CmuMgr_t *pCtl,
                      uint32 ulSegment,
                      uint32 ulOffset,
                      uint32 ulSize);

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuFifoEnable()
 *
 * @brief Enable/Disable Counter Manager Unit fifo that feeds the processor
 *
 * @param pCtl - Pointer to CMU control structure
 * @param bEnable - Enable or Disable the Fifo
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuFifoEnable(sbFe2000CmuMgr_t *pCtl,
                      uint8 bEnable);

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuCounterGroupRegister()
 *
 * @brief Add a segment and it's associated host-side buffer that will cache
 *        the update messages sent by the the CMU. This function should be
 *        called for each unique counter group, implying that for each segment
 *
 * @param pCtl - Pointer to CMU control structure
 * @param ulSegment  - Segment Id (0 -> 31)
 * @param ulBank     - Bank Id (0 -> 3)
 * @param ulBankBase - Base address for this segment in the device memory
 * @param ulTblLimit - Max number of table entries in this segment
 * @param eType      - Counter Type (0=Turbo, 1=Range, 2=Legacy, 3=Chained)
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuCounterGroupRegister(sbFe2000CmuMgr_t *pCtl,
                                uint32 *ulSegId,
                                uint32 ulNumCounters,
                                uint32 ulBank,
                                uint32 ulBankBase,
                                sbFe2000SegTblType_e_t eType);

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuCounterRead()
 *
 * @brief Re
 *
 * @param pCtl      - Pointer to CMU control structure
 * @param ulSegment - Memory segment to read from
 * @param ulOffset  - Which entry/entries to read
 * @param ulEntries - Number of entries to read
 * @param ulData    - Pointer to array for return results
 * @param bSync     - Force flush synchronization with hardware
 * @param bClear    - Clear cache after returning value
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuCounterRead(sbFe2000CmuMgr_t *pCtl,
                       uint32 ulSegment,
                       uint32 ulOffset,
                       uint32 ulEntries,
                       uint64 *ullData,
                       uint8 bSync,
                       uint8 bClear);

/*-----------------------------------------------------------------------------
 * @fn sbFe2000CmuClearSegments()
 *
 * @brief Clear the counter segment memory, respecting the protection scheme of
 *        the CMU.
 *
 * @param pCtl - Referenced pointer to CMU control structure
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
sbStatus_t
sbFe2000CmuClearSegment(sbFe2000CmuMgr_t *pCtl, uint32 ulSegment, uint32 ulSize);
#endif /* _SB_FE2K_CMU_MGR_H_ */
