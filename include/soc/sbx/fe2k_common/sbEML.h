#ifndef _SB_EML_H_
#define _SB_EML_H_

/******************************************************************************
 *
 * $Id: sbEML.h,v 1.8 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * sbEML.h: Exact Match Legacy compiler, public API.
 *
 *****************************************************************************/

#include <soc/sbx/fe2k_common/sbPaylMgr.h>
#include <soc/sbx/fe2k_common/sbFe2000DmaMgr.h>
#include <soc/sbx/fe2k_common/sbFeISupport.h>
#include "sbTypesGlue.h"

/*
 * EML compiler context, using which client identifies the EML instance
 * that it wants to deal with.
 */
typedef void *sbEMLCtx_p;

/* 
 * This part of EML's internal structure has to be opened to the public
 * (unfortunately) because clients build their table2 entries around
 * this base (by adding key/payload bytes immediately following
 * SB_EML_TABLE2_BASE - see e.g. table2entry definition for L2 compiler). 
 * That fact however is irrelevant to EML compiler since it is only 
 * interested in SB_EML_TABLE2_BASE part.
 */
#define SB_EML_MAX_TABLE2_SIZE_LOG 5
#define SB_EML_MAX_TABLE2_SIZE 32
#define SOC_SBX_EML_MAX_BANKS  5

/*
 * client0 and client1 are available for use by each particular EML
 * client implementation.  Previously, client-specific information was kept
 * after the payload (the EML implementation assumes the payload
 * immediately follows the _BASE), but this does not work for
 * generic EML instantiation (where the payload size is only known
 * at run-time).
 */
#define SB_EML_TABLE2_BASE \
  struct sbEMLTable2_s *next; \
  unsigned int valid    : 1; \
  unsigned int slotNo   : SB_EML_MAX_TABLE2_SIZE_LOG; \
  unsigned int usage    : 3; \
  unsigned int clsDirty : 1; \
  unsigned int client0  : 22; \
  unsigned int client1

typedef struct sbEMLTable2_s {
  SB_EML_TABLE2_BASE;
} sbEMLTable2_t, *sbEMLTable2_p_t;

/* 
 * Keys and payloads are exchanged between EML and its clients as
 * sequences of bytes, identified mostly only by a ptr to them.
 * Sometimes however, a client passes in a buffer (where the packed
 * key will be deposited) as an array, with SB_EML_MAX_KEY as its 
 * upper size limit (in words).
 */
/* max key size in words */
#define SB_EML_MAX_KEY 9
typedef uint32 sbEMLPackedKey_t[SB_EML_MAX_KEY];

typedef void *sbEMLKey_p_t;
typedef void *sbEMLPayload_p_t;
typedef uint32 *sbEMLPackedKeyPayload_p_t;


/* 
 * The EML collisions struct (defined below) is passed as an arg to
 * sbEMLAdd(), where the EML returns collisions (if any) detected
 * during the addition of an entry. Note that EML clients might have
 * their own collisions structures defined, which they use to return
 * information to their callers (e.g. see sbMacCollisions_p_t
 * typedef).
 */
typedef struct sbEMLCollisions_s {
  int entries;
  sbEMLTable2_p_t handles[1];
} sbEMLCollisions_t, *sbEMLCollisions_p_t;

typedef struct sbEMLInit_s {
  sbEMLCtx_p ctx;
  sbCommonConfigParams_p_t cParams;
  uint32 maxDirty;
  uint32 table1_count;
  uint32 slab_size;
  uint32 key_size;
  uint32 seed_bits;
  uint8 new_hash;
  uint8 ipv6;
  uint32 table2_max;
  uint32 table2_size;
  uint32 payload_size;

  void (*getKey)(sbEMLCtx_p ctx,
                 sbEMLPackedKeyPayload_p_t pkp,
                 sbEMLPackedKey_t keyData);

  void (*packKey)(sbEMLCtx_p ctx,
                  sbEMLKey_p_t key,
                  sbEMLPackedKey_t pkey);

  void (*unPackKey)(sbEMLCtx_p ctx,
                    sbEMLKey_p_t key,
                    sbEMLPackedKey_t pkey);

  void (*packKeyPayload)(sbEMLCtx_p ctx,
                         sbEMLKey_p_t key,
                         sbEMLPayload_p_t payload,
                         sbEMLPackedKeyPayload_p_t pkp);

  void (*updatePacked)(sbEMLCtx_p ctx,
                       sbEMLPackedKeyPayload_p_t pkp,
                       sbEMLKey_p_t key,
                       sbEMLPayload_p_t payload);

  uint32 (*table2size)(sbEMLCtx_p ctx, uint32 nentries);

  sbFe2000DmaSlab_t* (*slabTable2entry)(sbEMLCtx_p ctx,
                                           sbPHandle_t phdl,
                                           sbEMLTable2_t *entry,
                                           sbFe2000DmaSlab_t *slab);

  sbFe2000DmaSlab_t* (*slabGetTable2entry)(sbEMLCtx_p ctx,
                                              sbPHandle_t phdl,
                                              sbEMLTable2_t *entry,
                                              sbFe2000DmaSlab_t *slab);

  void (*unslabTable2Entry)(sbEMLCtx_p ctx,
                            sbEMLPackedKeyPayload_p_t pkp,
                            struct sbFe2000DmaSlab_s *slab,
                            unsigned int slotNo);


  sbFe2000DmaSlab_t* (*slabTable2)(sbEMLCtx_p ctx,
                                      sbPHandle_t phdl,
                                      uint32 usemap,
                                      sbEMLTable2_t *entries,
                                      sbFe2000DmaSlab_t *slab);


  sbFe2000DmaSlab_t* (*slabSetTable1ptr)(sbEMLCtx_p ctx,
                                            uint32 ix,
                                            uint32 seed,
                                            sbPHandle_t phdl,
                                            uint32 usemap,
                                            sbFe2000DmaSlab_t * slab);

  void (*table1EntryParse) (sbEMLCtx_p cstate, uint32 *s, 
                            uint32 *p, uint32 *m, uint8 *b);

  uint32 (*table1Addr)(sbEMLCtx_p cstate, int u);

  void (*commitDone)(sbEMLCtx_p ctx, sbStatus_t status);

  void (*updateDone)(sbEMLCtx_p ctx, sbStatus_t status);

  void (*getDone)(sbEMLCtx_p ctx, sbStatus_t status);

  unsigned int (*updClsId)(sbEMLCtx_p ctx, sbEMLPackedKeyPayload_p_t pkp,
                 uint8 b, unsigned int clsId, uint8 src);

  struct sbEML_s *sharedFreeList;

  uint32 nbank;

} sbEMLInit_t, *sbEMLInit_p_t;



/* 
 * Public API
 */

/*--- Initialize ---*/
sbStatus_t
sbEMLCompInit (sbEMLCtx_p               *pEmCtxt, 
                sbEMLInit_t              *initStruct, 
                sbCommonConfigParams_p_t   cParams,
                void                      *payloadMgr, 
                sbFe2000DmaMgr_t         *dmaCtxt);

/*--- Hardware initialize (zero out memory etc) ---*/
sbStatus_t
sbEMLCompInitHW (sbEMLCtx_p             emCtxt, 
                  sbFeInitAsyncCallback_f_t cBack,
                  void                   *initId);
/*--- Recover soft state from hardware ---*/
sbStatus_t
sbEMLCompRecover (sbEMLCtx_p *pEmCtxt);

/*--- Un-initialize ---*/
sbStatus_t
sbEMLCompUninit (sbEMLCtx_p emCtxt);

/*--- Add an entry ---*/
sbStatus_t
sbEMLAdd (sbEMLCtx_p           emCtxt,
           sbEMLKey_p_t         key,
           sbEMLPayload_p_t     payload,
           sbEMLCollisions_p_t  coll,
           sbEMLTable2_p_t     *rt2p);

/*--- Delete an entry ---*/
sbStatus_t
sbEMLDel (sbEMLCtx_p emCtxt, 
           sbEMLKey_p_t key);

/*--- Delete all entries present ---*/
sbStatus_t
sbEMLFlush (sbEMLCtx_p emCtxt);

/*--- Update an existing entry ---*/
sbStatus_t
sbEMLUpdate (sbEMLCtx_p        emCtxt,
              sbEMLKey_p_t      key,
              sbEMLPayload_p_t  payload,
              sbEMLTable2_p_t  *at2p);

/*--- Return payload associated with an existing entry ---*/
sbStatus_t
sbEMLGet (sbEMLCtx_p                 emCtxt, 
           sbEMLKey_p_t               key,
           sbEMLPackedKeyPayload_p_t  pkp,
           sbEMLTable2_p_t           *at2p);

/*--- Commit modified entries to memory ---*/
sbStatus_t
sbEMLCommit (sbEMLCtx_p  emCtxt,
              uint32    *pnRunLength);


/*--- Return table2array ptr to be used by ager --*/
void *
sbEMLTable2Array (sbEMLCtx_p emCtxt);

extern int
sbEMLIteratorFirst(sbEMLCtx_p emCtxt, sbEMLKey_p_t key);

extern int
sbEMLIteratorNext(sbEMLCtx_p emCtxt, sbEMLKey_p_t key, sbEMLKey_p_t next);
#endif
