/*
 * $Id: nlmgenerictblmgr.c,v 1.1.6.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include "nlmsdksmoketest.h"

/* this bank mode will decide to run
    1. single bank mode  (2x parallel srchs, only bank 0, key:0-1 and rs_port:0-1)
*/




/* Number of devices */
#define NLM_REFAPP_NUM_OF_DEVICES        1


static void CreateRecordsInTables_nosmt( genericTblMgrRefAppData *refAppData_p);
static void FillRecordPatternFor_80B0_Table( nlm_u8 *data_p, nlm_u8 *mask_p, nlm_u32 iter );

/* define NETL_DEBUG to get the index change log messages */
#ifdef NETL_DEBUG
#undef NETL_DEBUG
#endif

static void printReasonCode(NlmReasonCode reason)
{
    NlmCmFile__printf("\tReason Code = %d\n\n", reason);
}

#ifdef NETL_DEBUG
    static void print160Bits( nlm_u8 *data_p )
    {
        nlm_u8 iter;

        for( iter = 0; iter < NLMDEV_CB_WIDTH_IN_BYTES; iter++ ){
            if( iter && ( (iter % 5) == 0 ) )
                NlmCmFile__printf("_");

            NlmCmFile__printf("%02x", *( data_p + iter ) );
        }

        NlmCmFile__printf("\n");
    }
#endif



/* This function takes unsigned integer, puts each byte into array pointed by 'data' */
static void WriteValueToBitVector4( nlm_u8 *data, nlm_u32 value )
{
      data[ 0 ] = ( nlm_u8 )( 0xFF & ( value >> 24 ) );
      data[ 1 ] = ( nlm_u8 )( 0xFF & ( value >> 16 ) );
      data[ 2 ] = ( nlm_u8 )( 0xFF & ( value >> 8 ) );
      data[ 3 ] = ( nlm_u8 )( 0xFF & ( value >> 0 ) );
}


static void IndexChangeCallBack( void* client_p,
                                NlmGenericTbl* genericTbl_p,
                                NlmRecordIndex oldIndex,
                                NlmRecordIndex newIndex
                                )
{
    genericTblMgrRefAppData *refAppData_p = (genericTblMgrRefAppData *)client_p;
    tableInfo *tblInfo_p;
    tableRecordInfo *tblRecordInfo_p;
    nlm_u32 num_recs, iter_rec;
    nlm_u8 instance = 0;

    instance = genericTbl_p->m_tblId;

#ifdef NETL_DEBUG
    NlmCmFile__printf("\tOld Index = %u, New Index = %u [Table = %d]\n",
                        oldIndex, newIndex, genericTbl_p->m_tblId);
#endif

    /* Just return if oldIndex is NLM_GTM_INVALID_INDEX. tblRecordInfo_p->index
     * is already updated when passed as outParam in __AddRecord() function call.
     */
    if( oldIndex == NLM_GTM_INVALID_INDEX )
        return;

    tblInfo_p = &refAppData_p->g_gtmInfo[instance].tblInfo;
    tblRecordInfo_p = tblInfo_p->tblRecordInfo_p;
    num_recs = tblInfo_p->max_recCount;

    for( iter_rec = 0; iter_rec < num_recs; iter_rec++ )
    {
        if( (tblRecordInfo_p + iter_rec)->index == oldIndex)
        {
            (tblRecordInfo_p + iter_rec)->index = newIndex;
            break;
        }
    }

    if( iter_rec == num_recs )
    {
        NlmCmFile__printf("\tcallBackCB -- Invalid indexes; oldIndex = %u, newIndex =  %u\n",
                                oldIndex, newIndex );
    }

    return;
}

static int InitEnvironment( genericTblMgrRefAppData *refAppData_p )
{

    NlmCm__memset( refAppData_p, 0, sizeof( genericTblMgrRefAppData ) );

    /* create default memory allocator */
    refAppData_p->alloc_p = NlmCmAllocator__ctor( &refAppData_p->alloc_bdy );
    NlmCmAssert( ( refAppData_p->alloc_p != NULL ), "Memory Allocator Init Failed.\n");
    if(refAppData_p->alloc_p == NULL)
        return RETURN_STATUS_FAIL;

    /* Currently Device Manager is flushing out each request immediately. */
    refAppData_p->request_queue_len = 1;
    refAppData_p->result_queue_len  = 1;

    /* Search system has only one channel (cascade of devices) */
    refAppData_p->channel_id = 0;
    refAppData_p->if_type    = IFTYPE_CMODEL;

    /* Create transport interface. Only interface supported with this release is SimXpt */
    switch( refAppData_p->if_type )
    {
        case IFTYPE_CMODEL:
        {
            refAppData_p->xpt_p = kbp_simxpt_init( refAppData_p->alloc_p,
                                                     NLM_DEVTYPE_3,
                                                     refAppData_p->request_queue_len,
                                                        refAppData_p->result_queue_len,
                                                     refAppData_p->channel_id
                                                    );

            NlmCmAssert( ( refAppData_p->xpt_p != NULL ), "Could not create" \
                                " Simulation Transport Interface. Exiting..\n");
            NlmCmFile__printf("\n\tSimulation Transport Interface Created Successfully\n");

            if(refAppData_p->xpt_p == NULL)
                return RETURN_STATUS_FAIL;

            break;
        }
        case IFTYPE_FPGA:
        {
            NlmCmFile__printf("\n\tThis interface type is not yet supported...\n");

            return RETURN_STATUS_FAIL;
        }
        default:
        {
            NlmCmFile__printf("Invalid interface type...\n");

            return RETURN_STATUS_FAIL;
        }

    }

    return RETURN_STATUS_OK;
}



/* Destroy SimXpt transport interface */
static int DestroyEnvironment( genericTblMgrRefAppData *refAppData_p )
{
    /* Check the XPT interface type and call appropriate functions */
    switch( refAppData_p->if_type )
    {
        case IFTYPE_CMODEL:
        {
            NlmCmFile__printf("\n\tDestroying Simulation Transport Interface\n");
            kbp_simxpt_destroy( refAppData_p->xpt_p );

            break;
        }
        case IFTYPE_FPGA:
        {
            NlmCmFile__printf("\n\tThis interface type is not yet supported...\n");

            return RETURN_STATUS_FAIL;
        }
        default:
        {
            NlmCmFile__printf("\n\tInvalid interface type...\n");

            return RETURN_STATUS_FAIL;
        }

    }

    return RETURN_STATUS_OK;
}


/* This function performs Device Manager related Inits */
static int InitDeviceManager( genericTblMgrRefAppData *refAppData_p
                              )
{
    NlmDevId    dev_id;
    NlmReasonCode    reason = NLMRSC_REASON_OK;

    if(refAppData_p->smtMode != NLMDEV_NO_SMT_MODE &&
        refAppData_p->smtMode != NLMDEV_DUAL_SMT_MODE)
    {
        NlmCmFile__printf("\tInvalid bank mode...\n" );
        return RETURN_STATUS_ABORT;
    }


    if( NULL == ( refAppData_p->devMgr_p = kbp_dm_init( refAppData_p->alloc_p,
                                                                refAppData_p->xpt_p,
                                                                NLM_DEVTYPE_3,
                                                                refAppData_p->portMode,
                                                                refAppData_p->smtMode,
                                                                0, /* ignored in 12K mode */
                                                               &reason
                                                              ) ) )
    {
        NlmCmFile__printf("\tCould not initialize Device Manager...\n" );
        printReasonCode( reason );

        return RETURN_STATUS_ABORT;
    }

    NlmCmFile__printf("\n\tDevice Manager Initialized Successfully\n");



    /* Now add a device to the search system */
    if( NULL == (refAppData_p->dev_p = kbp_dm_add_device(refAppData_p->devMgr_p,
                                                                     &dev_id,
                                                                     &reason
                                                                    ) ) )
    {
        NlmCmFile__printf("Could not add device to the search system...\n");
        printReasonCode( reason );

        return RETURN_STATUS_ABORT;
    }

    NlmCmFile__printf("\tDevice#0 Added to the search system\n");

    /* We are done with configurations. Now Lock Device Manager */
    if( NLMERR_OK != kbp_dm_lock_config( refAppData_p->devMgr_p, &reason ) )
    {
        NlmCmFile__printf("Could not lock Device Manager...\n");
        printReasonCode( reason );

        return RETURN_STATUS_ABORT;
    }

    NlmCmFile__printf("\tDevice Manager Configuration Locked\n");

    return RETURN_STATUS_OK;
}


/* Destroys Device Manager instance */
static int DestroyDeviceManager( genericTblMgrRefAppData *refAppData_p )
{
    NlmCmFile__printf("\n\tDestroying Device Manager\n");
    kbp_dm_destroy( refAppData_p->devMgr_p );

    return RETURN_STATUS_OK;
}


/* This function performs Generic Table Manager related Inits */
static int InitGenericTableManager_nosmt(genericTblMgrRefAppData *refAppData_p)

{
    NlmReasonCode    reason;
    nlm_u32 i = 0;

    /* Set the Generic Table Manager block range */
    refAppData_p->g_gtmInfo[0].dbaSBRange.m_stSBNum  = NLM_GTM_REFAPP_NOSMT_80B_DBA_START_SB;
    refAppData_p->g_gtmInfo[0].dbaSBRange.m_endSBNum = NLM_GTM_REFAPP_NOSMT_80B_DBA_END_SB;

    refAppData_p->g_gtmInfo[0].udaSBRange.m_stSBNum = NLM_GTM_REFAPP_NOSMT_80B_UDA_START_SB;
    refAppData_p->g_gtmInfo[0].udaSBRange.m_endSBNum = NLM_GTM_REFAPP_NOSMT_80B_UDA_END_SB;


    NlmCmFile__printf("\n\tDBA Super Blocks %d %d are mapped to Bank-0 \n",
            refAppData_p->g_gtmInfo[0].dbaSBRange.m_stSBNum, refAppData_p->g_gtmInfo[0].dbaSBRange.m_endSBNum);
        NlmCmFile__printf("\n\tUDA Super Blocks %d %d are mapped to Bank-0 \n",
            refAppData_p->g_gtmInfo[0].udaSBRange.m_stSBNum, refAppData_p->g_gtmInfo[0].udaSBRange.m_endSBNum);


    /* Call back function called by GTM when index for a record is changed */
    refAppData_p->indexChangeCB = IndexChangeCallBack;

    if( NULL == ( refAppData_p->genericTblMgr_p = kbp_gtm_init(
                                                        refAppData_p->alloc_p,
                                                        refAppData_p->devMgr_p,
                                                        NLM_DEVTYPE_3,
                                                        refAppData_p->g_gtmInfo[NLMDEV_SMT_0].bankNum,
                                                        NLM_REFAPP_NUM_OF_DEVICES,
                                                        &refAppData_p->g_gtmInfo[NLMDEV_SMT_0].dbaSBRange,
                                                        &refAppData_p->g_gtmInfo[NLMDEV_SMT_0].udaSBRange,
                                                        refAppData_p->indexChangeCB,
                                                        refAppData_p,
                                                        &reason ) ) )
    {
        NlmCmFile__printf("\tGTM Init failed...\n");
        printReasonCode( reason );

        return RETURN_STATUS_ABORT;
    }

    NlmCmFile__printf("\n\tGeneric Table Manager Initialized Successfully\n\n");

    /* Create tables now, 2 tables are created */

    /* Table-0 : 80b, <4'b 0s> <12b VRF all 0s> < 32b Source IP> <32b don't cares */
    refAppData_p->g_gtmInfo[0].tblInfo.tbl_id         = TABLE_ID_80B_0;
    refAppData_p->g_gtmInfo[0].tblInfo.tbl_width     = TABLE_WIDTH_80B_0;
    refAppData_p->g_gtmInfo[0].tblInfo.tbl_size      = TABLE_SIZE_80B_0;
    refAppData_p->g_gtmInfo[0].tblInfo.tbl_assoWidth = TABLE_AD_WIDTH_80B_0;
    refAppData_p->g_gtmInfo[0].tblInfo.groupId_start = START_GROUPID_80B_0;
    refAppData_p->g_gtmInfo[0].tblInfo.groupId_end   = END_GROUPID_80B_0;

    /* Table-0 : 80b, <4'b 0s> <12b VRF all 0s> < 32b Destination IP> <32b don't cares */
    refAppData_p->g_gtmInfo[1].tblInfo.tbl_id         = TABLE_ID_80B_1;
    refAppData_p->g_gtmInfo[1].tblInfo.tbl_width     = TABLE_WIDTH_80B_1;
    refAppData_p->g_gtmInfo[1].tblInfo.tbl_size      = TABLE_SIZE_80B_1;
    refAppData_p->g_gtmInfo[1].tblInfo.tbl_assoWidth = TABLE_AD_WIDTH_80B_1;
    refAppData_p->g_gtmInfo[1].tblInfo.groupId_start = START_GROUPID_80B_1;
    refAppData_p->g_gtmInfo[1].tblInfo.groupId_end   = END_GROUPID_80B_1;

    for( i = 0 ;i < NUM_OF_TABLES; i++)
    {
        /*Call the CreateTable API to create each of the tables */
        if( NULL == ( refAppData_p->g_gtmInfo[i].tbl_p = kbp_gtm_create_table(
                                                    refAppData_p->genericTblMgr_p,
                                                    NLMDEV_PORT_0,
                                                    refAppData_p->g_gtmInfo[i].tblInfo.tbl_id,
                                                    refAppData_p->g_gtmInfo[i].tblInfo.tbl_width,
                                                    refAppData_p->g_gtmInfo[i].tblInfo.tbl_assoWidth,
                                                    refAppData_p->g_gtmInfo[i].tblInfo.tbl_size,
                                                    &reason ) ) )
        {
            NlmCmFile__printf("\tTable [%d] created on Bank - %d creation failed...\n",
                                refAppData_p->g_gtmInfo[i].tblInfo.tbl_id,i);
            printReasonCode( reason );

            return RETURN_STATUS_FAIL;
        }
        else
        {
            NlmCmFile__printf("\tTable [%d] creation succeded with %d width, %d asso-data for BANK: 0...\n",
                refAppData_p->g_gtmInfo[i].tblInfo.tbl_id, (refAppData_p->g_gtmInfo[i].tblInfo.tbl_width),
                (refAppData_p->g_gtmInfo[i].tblInfo.tbl_assoWidth * 32));
        }

    }

    NlmCmFile__printf("\n\n");

    /* Configure searches now. Table-0 (SrcIP table) is searched with Key#0,
     * Table-1 (DstIP table) is searched with Key#1)
     */
    {
        NlmGenericTblParallelSearchInfo  *psInfo_p = NULL;

        refAppData_p->g_gtmInfo[0].ltr_num = NLM_GTM_REFAPP_SMT0_LTR_NUM;
        refAppData_p->g_gtmInfo[0].search_attrs.m_numOfParallelSrches  = 2;
        refAppData_p->g_gtmInfo[0].search_attrs.m_isCmp3Search = NlmFalse;

        /* Master key: 80b:  <4b 0's><12b VRF all 0's><32b source IP><32b dest IP>
                                00_00___55.55.12.34___ee.ee.12.34 */

        psInfo_p = &refAppData_p->g_gtmInfo[0].search_attrs.m_psInfo[0];

        psInfo_p->m_tblId        = TABLE_ID_80B_0;
        psInfo_p->m_rsltPortNum = 0 ;
        psInfo_p->m_keyNum        = 0 ;

        /* KCR construction key#0 */
        /* Key0: 80b:  <4b 0's><12b VRF all 0's><32b source IP><32b value that will anyway match>
                          00_00___55.55.12.34___XX.XX.XX.XX

        copy the 10B as is, only <4b 0's><12b VRF all 0's><32b source IP> are valid,
        remaining 32b are masked off and will always match */

        psInfo_p->m_kcm.m_segmentStartByte[0] = 0;
        psInfo_p->m_kcm.m_segmentNumOfBytes[0] = 10;
        /* end of key segment*/
        psInfo_p->m_kcm.m_segmentStartByte[1] = 0x7f;
        psInfo_p->m_kcm.m_segmentNumOfBytes[1] = 0;

        psInfo_p = &refAppData_p->g_gtmInfo[0].search_attrs.m_psInfo[1];

        psInfo_p->m_tblId        = TABLE_ID_80B_1;
        psInfo_p->m_rsltPortNum = 1 ;
        psInfo_p->m_keyNum        = 1 ;

        /* KCR construction key#1 */
        /* Key1: 80b:  <4b 0's><12b VRF all 0's><32b dest IP><32b value that will always match>
                       00_00___ee.ee.12.34___XX.XX.XX.XX

        copy the LS 4B of master key, then copy the 32b dest IP, and 4b table-id and 12b VRF (16b zeros)
        Note: LS 32b are masked off    and will always match */

        /* LS 32b masked */
        psInfo_p->m_kcm.m_segmentStartByte[0] = 0;
        psInfo_p->m_kcm.m_segmentNumOfBytes[0] = 4;
        /* dest Ip */
        psInfo_p->m_kcm.m_segmentStartByte[1] = 0;
        psInfo_p->m_kcm.m_segmentNumOfBytes[1] = 4;
        /* msb 16b zeros */
        psInfo_p->m_kcm.m_segmentStartByte[2] = 8;
        psInfo_p->m_kcm.m_segmentNumOfBytes[2] = 2;
        /* end of key segment */
        psInfo_p->m_kcm.m_segmentStartByte[3] = 0x7f;
        psInfo_p->m_kcm.m_segmentNumOfBytes[3] = 0;

        /*Call the ConfigSearch API to configure each of the LTRs */
        if( NLMERR_OK != kbp_gtm_config_search(
                                     refAppData_p->genericTblMgr_p,
                                     NLMDEV_PORT_0,
                                    refAppData_p->g_gtmInfo[0].ltr_num,
                                    &refAppData_p->g_gtmInfo[0].search_attrs,
                                    &reason ) )
        {
            NlmCmFile__printf("\tLTR#%d ConfigSearch  failed...\n",
                                    refAppData_p->g_gtmInfo[0].ltr_num);
            printReasonCode( reason );

            return RETURN_STATUS_FAIL;
        }
        else
        {
            NlmCmFile__printf("\tLTR#%d ConfigSearch (%dX parallel searches)  done...\n",
                refAppData_p->g_gtmInfo[0].ltr_num,
                refAppData_p->g_gtmInfo[0].search_attrs.m_numOfParallelSrches);

        }

    }         /* end of configurations */

    NlmCmFile__printf("\n\n");

    /* Configuration is finished, Lock it now */
    if( NLMERR_OK != kbp_gtm_lock_config(refAppData_p->genericTblMgr_p, NLMDEV_PORT_0, &reason ) )
    {
        NlmCmFile__printf("\tGTM LockConfig failed...\n");
        printReasonCode( reason );

        return RETURN_STATUS_FAIL;
    }

    NlmCmFile__printf("\tGTM Configuration Locked\n\n");

    /* Create records to be inserted into various tables. */
    CreateRecordsInTables_nosmt( refAppData_p);

    return RETURN_STATUS_OK;

}

/* Destroy generic tables and manager instance */
static int DestroyGenericTableManager(genericTblMgrRefAppData *refAppData_p
                                      )
{
    tableInfo *tblInfo_p;
    tableRecordInfo    *tblRecordInfo_p;
    nlm_u32 iter_tbl, iter_rec, num_recs;
    NlmReasonCode   reason = NLMRSC_REASON_OK;

    /* First free memory allocated for keeping records within Ref App. Next step is to
     * destroy tables and finally calling destroy table manager.
     */
    for( iter_tbl = 0; iter_tbl < NUM_OF_TABLES; iter_tbl++ )
    {
        tblInfo_p = &refAppData_p->g_gtmInfo[iter_tbl].tblInfo;
        tblRecordInfo_p = tblInfo_p->tblRecordInfo_p;

        num_recs = tblInfo_p->max_recCount;
        for( iter_rec = 0; iter_rec < num_recs; tblRecordInfo_p++, iter_rec++ )
        {
            NlmCmAllocator__free( refAppData_p->alloc_p, tblRecordInfo_p->record.m_data );
            NlmCmAllocator__free( refAppData_p->alloc_p, tblRecordInfo_p->record.m_mask );
        }

        /* tblRecordInfo is calloced, free that space */
        NlmCmAllocator__free( refAppData_p->alloc_p, tblInfo_p->tblRecordInfo_p );

        /* And destroy the table */
        NlmCmFile__printf("\n\tDestroying table#%u", iter_tbl);
        if( NLMERR_OK != kbp_gtm_destroy_table( refAppData_p->genericTblMgr_p,
                                         NLMDEV_PORT_0,
                                         refAppData_p->g_gtmInfo[iter_tbl].tbl_p,
                                         &reason ) )
        {
            NlmCmFile__printf("\tDestroyTable#%u failed...\n", iter_tbl);
            printReasonCode( reason );
        }
    }
    NlmCmFile__printf("\n");

    /* Time to destroy the manager now. */
    NlmCmFile__printf("\n\tDestroying Generic Table Manager\n");
    if( NLMERR_OK != kbp_gtm_destroy( refAppData_p->genericTblMgr_p,
                                            NLMDEV_PORT_0, &reason ) )
    {
        NlmCmFile__printf("\tGeneric Table Manager destroy failed...\n");
        printReasonCode( reason );

        return NLMERR_FAIL;
    }

    return RETURN_STATUS_OK;
}






/* This function fills the internal data strcture for tables with records */
static void CreateRecordsInTables_nosmt(genericTblMgrRefAppData *refAppData_p
                                  )
{
    tableInfo *tblInfo_p;
    tableRecordInfo *tblRecordInfo_p;
    nlm_u32 alloc_size;
    nlm_u8  iter_tbl, tblWidth_inBytes;
    nlm_u8  *rec_data, *rec_mask;
    nlm_u16  start, end, iter_group, iter_priority;
    nlm_u32 loop = 0, start_value = 0, advlaue = 0;

    /* First allocate memory for storing records within the data structures.
     * Each table structure has start groupId and end groupId. Records of
     * all groupIds including the start-end groupId are added. With each
     * groupId, priorities of the records would range from [0 TO groupId-1].
     */
     for( iter_tbl = 0; iter_tbl < NUM_OF_TABLES; iter_tbl++ )
     {
        tblInfo_p = &refAppData_p->g_gtmInfo[iter_tbl].tblInfo;
         start     = tblInfo_p->groupId_start;
         end       = tblInfo_p->groupId_end;

        alloc_size = tblInfo_p->tbl_size;
        tblInfo_p->tblRecordInfo_p = NlmCmAllocator__calloc(
            refAppData_p->alloc_p, alloc_size, sizeof( tableRecordInfo ) );

        tblInfo_p->max_recCount    = alloc_size;
         tblWidth_inBytes = (nlm_u8)(tblInfo_p->tbl_width / 8);

        rec_data = NlmCmAllocator__calloc( refAppData_p->alloc_p, 1, tblWidth_inBytes );
        rec_mask = NlmCmAllocator__calloc( refAppData_p->alloc_p, 1, tblWidth_inBytes );

        /* get the starting record pattern based on the table id */
        if( tblInfo_p->tbl_id  == 0)
        {
            start_value = 0x55551234; /* src  Ip: table-0 */
            advlaue = 0xdead0000;      /* 64b  ad:         */
        }
        else
        {
            start_value = 0xeeee1234; /* dest IP: table-1 */
            advlaue = 0xbeaf8000;      /* 32b  ad:         */
        }

        /* store the records */
        iter_group = start;
        iter_priority = 0;
        tblRecordInfo_p = tblInfo_p->tblRecordInfo_p;

        for(loop = 0; loop < tblInfo_p->tbl_size; loop++)
        {
            tblRecordInfo_p->groupId  = iter_group;
            tblRecordInfo_p->priority = iter_priority;

            tblRecordInfo_p->record.m_data = NlmCmAllocator__calloc( refAppData_p->alloc_p, 1, tblWidth_inBytes );
            tblRecordInfo_p->record.m_mask = NlmCmAllocator__calloc( refAppData_p->alloc_p, 1, tblWidth_inBytes );
            tblRecordInfo_p->record.m_len  = tblInfo_p->tbl_width;

            /* Generate the ACL record here */
            FillRecordPatternFor_80B0_Table(rec_data, rec_mask, (start_value + loop));

            /* associated data : msb32b */
            WriteValueToBitVector4(&tblRecordInfo_p->assoData[0], (advlaue + loop));

            if( tblInfo_p->tbl_id  == 0) /* lsb 32b associated data */
                WriteValueToBitVector4(&tblRecordInfo_p->assoData[4], (advlaue + loop));

            NlmCm__memcpy( tblRecordInfo_p->record.m_data, rec_data, tblWidth_inBytes );
            NlmCm__memcpy( tblRecordInfo_p->record.m_mask, rec_mask, tblWidth_inBytes );

            tblRecordInfo_p++;

#ifdef WITH_INDEX_SHUFFLE
            iter_priority++;

            /* loop back priority */
            if( (iter_priority % end) == 0)
            {
                iter_priority = start;
                iter_group++;
            }

            /* loop back group */
            if( iter_group && ((iter_group % end) == 0))
            {
                iter_group = start;
            }
#else
            iter_group++;
            iter_priority++;

            /* loop back priority */
            if( (iter_priority % end) == 0)
            {
                iter_priority = start;
            }

            /* loop back group */
            if( (iter_group % end) == 0)
            {
                iter_group = start;
            }
#endif
        }

        /* Free the memory allocated for temp data and mask. A fresh memory will be
         * based on the table width
         */
        NlmCmAllocator__free( refAppData_p->alloc_p, rec_data );
        NlmCmAllocator__free( refAppData_p->alloc_p, rec_mask );

     } /* end of table's loop */
}


/* Adds records into various tables */
static int AddRecordsIntoTables(
                genericTblMgrRefAppData *refAppData_p,
                NlmGenericTbl *tbl_p,
                nlm_u8 instance
                )
{
    tableInfo *tblInfo_p;
    tableRecordInfo    *tblRecordInfo_p;
    NlmReasonCode   reason = NLMRSC_REASON_OK;
    nlm_u32 iter_rec, num_recs;
    nlm_u16 portNum;

    tblInfo_p = &refAppData_p->g_gtmInfo[instance].tblInfo;
    tblRecordInfo_p = tblInfo_p->tblRecordInfo_p;
    num_recs  = tblInfo_p->max_recCount;

    portNum = NLMDEV_PORT_0;

    NlmCmFile__printf("\n\tAdding records into table [%d]\n", tbl_p->m_tblId);
    for(iter_rec = 0; iter_rec < num_recs; iter_rec++ )
    {
        /* Add record now */
        if( NLMERR_OK != kbp_gtm_add_record(tbl_p,
                                                portNum,
                                                &tblRecordInfo_p->record,
                                                &tblRecordInfo_p->assoData[0],
                                                tblRecordInfo_p->groupId,
                                                tblRecordInfo_p->priority,
                                                &tblRecordInfo_p->index,
                                                &reason ) )
        {
            NlmCmFile__printf("\ttable [%d] insertions: could not add record#%u\n",
                                tbl_p->m_tblId, tblInfo_p->rec_count);
            printReasonCode( reason );

            return NLMERR_FAIL;
        }

        /* Advance the record pointer */
        tblRecordInfo_p++;
        tblInfo_p->rec_count++;

        if( (tblInfo_p->rec_count % 500) == 0 )
            NlmCmFile__printf("\t    - %u records added to table %d\n", tblInfo_p->rec_count, tbl_p->m_tblId);

    } /* end of for loop */

    NlmCmFile__printf("\t   Total number of records added [%u] to table %d\n", tblInfo_p->rec_count, tbl_p->m_tblId);

    return RETURN_STATUS_OK;
}


/* 2x parallel search of SrcIP(table-0) and DstIP(table-1) tables using LTR 0 */
static int Perform_LTR0_Searches(genericTblMgrRefAppData *refAppData_p
                                 )
{
    tableInfo   *tblInfo0_p, *tblInfo1_p;
    tableRecordInfo   *tblRecordInfo0_p,*tblRecordInfo1_p;
    NlmGenericTblRecord *tblRecord0_p, *tblRecord1_p;
    NlmReasonCode   reason = NLMRSC_REASON_OK;
    nlm_u32 iter, numRecs;
    nlm_u8  ltr_num, i=0;
    tableRecordInfo   *tblRecHoldInfo0_p,*tblRecHoldInfo1_p; /* reference for random prefixes */

    /* Device Manager declarations */
    NlmDevCtxBufferInfo cb_info;
    NlmDevCmpResult     search_result;
    NlmDevCmpResult     exp_result;

    tblInfo0_p = &refAppData_p->g_gtmInfo[0].tblInfo;
    tblInfo1_p = &refAppData_p->g_gtmInfo[1].tblInfo;

    numRecs = tblInfo0_p->rec_count; /* both records count is same */

    /* expected search results */
    tblRecHoldInfo0_p = tblRecordInfo0_p = tblInfo0_p->tblRecordInfo_p;
    tblRecHoldInfo1_p = tblRecordInfo1_p = tblInfo1_p->tblRecordInfo_p;

    ltr_num = NLM_GTM_REFAPP_SMT0_LTR_NUM;

    NlmCmFile__printf("\n\tPerforming LTR#%d searches\n", ltr_num);

    iter = 0;

    /* MasterKey[79:0] is constructed from table0/1 records */
    for( iter = 0; iter < numRecs; iter++ )
    {
        NlmCm__memset(&search_result,0,sizeof(NlmDevCmpResult));
        NlmCm__memset(&exp_result,0,sizeof(NlmDevCmpResult));

        /* Expected results */
        for(i = 0; i < 2; i++)
        {
            exp_result.m_resultValid[i] = NLMDEV_RESULT_VALID;
            exp_result.m_hitOrMiss[i]   = NLMDEV_HIT;
            exp_result.m_hitDevId[i]    = 0;

            if(i == 0)
            {
                exp_result.m_hitIndex[i]    = tblRecordInfo0_p->index;
                exp_result.m_respType[i]    = NLMDEV_INDEX_AND_64B_AD;
                NlmCm__memcpy(&exp_result.m_AssocData[i], &tblRecordInfo0_p->assoData[0], 8);
            }
            else
            {
                exp_result.m_hitIndex[i]    = tblRecordInfo1_p->index;
                exp_result.m_respType[i]    = NLMDEV_INDEX_AND_32B_AD;
                NlmCm__memcpy(&exp_result.m_AssocData[i], &tblRecordInfo1_p->assoData[0], 4);
            }
        }

        /* construct the master key 79:0  <16B_0's><32b_sourceIP><32b_destinationIP> */
        tblRecord0_p = &(tblRecordInfo0_p->record);
        NlmCm__memcpy( &cb_info.m_data[ 0 ], tblRecord0_p->m_data, 10 );

        tblRecord1_p = &(tblRecordInfo1_p->record);
        NlmCm__memcpy( &cb_info.m_data[ 6 ], &tblRecord1_p->m_data[2], 4 );

        cb_info.m_datalen = 10;
        cb_info.m_cbStartAddr = 0;
        /* Fire Compare1 instruction now */
        if( NLMERR_OK != kbp_dm_cbwcmp1( refAppData_p->devMgr_p, NLMDEV_PORT_0, ltr_num,
                                                &cb_info, &search_result, &reason ) )
        {
            NlmCmFile__printf("Compare1 Instruction failed. Exiting...\n");
            printReasonCode( reason );

            return NLMERR_FAIL;
        }
        if(NlmCm__memcmp(&search_result, &exp_result, sizeof(NlmDevCmpResult)) != 0)
        {
            printf("\r\t compare fails %d", iter);
            return NLMERR_FAIL;
        }

#ifdef RAND_SEARCH_KEYS
        tblRecordInfo0_p = (tblRecHoldInfo0_p + (rand() % numRecs));
        tblRecordInfo1_p = (tblRecHoldInfo1_p + (rand() % numRecs));
#else
        /* next records */
        tblRecordInfo0_p++;
        tblRecordInfo1_p++;
#endif

        if( iter && ((iter % 500) == 0 ) )
            NlmCmFile__printf("\n\t     - %u keys are searched ", iter);

    }

    NlmCmFile__printf("\n\t   Total number of keys searched [%u]\n", iter);
    return RETURN_STATUS_OK;
}




void FillRecordPatternFor_80B0_Table(nlm_u8 *data_p, nlm_u8 *mask_p, nlm_u32 iter)
{
    /* 0.0.32b <source/dest_IP>.X.X.X.X */

    NlmCm__memset( data_p, 0, 10 );
    NlmCm__memset( mask_p, 0, 10 );

    /* first 2B are 0, second 4B are source/dest IP, remaining 4B don't cares. MS 48b valid,
     * so mask the LS 32b
     */
    WriteValueToBitVector4(&data_p[2], iter);

    /* Mask off last 4 bytes */
    NlmCm__memset(&mask_p[6], 0xFF, 4 );

    return;
}


/* will work on 1-port, no-smt mode contains only  2#80b table with
   64b and 32b AD respectively, 2x parallel search using LTR 0 */

nlm_u32 NlmGenericTableManager_nosmt(genericTblMgrRefAppData *refAppData_p)
{
    nlm_u8  iter_tbl;

    /* Initialize Generic Table Manager now */
    if( RETURN_STATUS_OK != InitGenericTableManager_nosmt( refAppData_p) )
    {
        NlmCmFile__printf("\tGeneric Table Manager Initialization failed. Exiting...\n");

        return RETURN_STATUS_ABORT;
    }
    /* Add records to tables now */
    for( iter_tbl = 0; iter_tbl < NUM_OF_TABLES; iter_tbl++ )
    {
        AddRecordsIntoTables( refAppData_p, refAppData_p->g_gtmInfo[iter_tbl].tbl_p, iter_tbl);
    }

    /* perform searches */
    Perform_LTR0_Searches( refAppData_p);

    /* destroying GTM */
    if( RETURN_STATUS_OK != DestroyGenericTableManager( refAppData_p) )
    {
        NlmCmFile__printf("\tGeneric Table Manager Destroy failed. Exiting...\n");

        return RETURN_STATUS_ABORT;
    }

    return RETURN_STATUS_OK;
}




int main(int argc, char    *argv[])
{
    genericTblMgrRefAppData  refAppData;
    nlm_32    break_alloc_id = -1;
    nlm_u32 errNum = RETURN_STATUS_OK;

    NlmCmDebug__Setup( break_alloc_id, NLMCM_DBG_EBM_ENABLE );

    NlmCmFile__printf ("\n\tGeneric Table Manager Application Reference Code Using\n");
    NlmCmFile__printf ("\tGeneric Table Manager Module (for Arad). \n\n\n");

    (void)argv;
    (void)argc;

    /* Initialize customer specific entities: Memory Allocator, XPT Interface */
    if( RETURN_STATUS_OK != InitEnvironment( &refAppData ) )
    {
        NlmCmFile__printf("\tEnvironment Initialization failed. Exiting...\n");

        return RETURN_STATUS_ABORT;
    }

    /* bank mode and bank type */
    {
        refAppData.smtMode = NLMDEV_NO_SMT_MODE;
        refAppData.portMode = NLMDEV_SINGLE_PORT;
        refAppData.g_gtmInfo[0].bankNum = NLMDEV_BANK_0;
        NlmCmFile__printf("\tDevice working in 1-PORT/ no-SMT MODE...\n");
    }

    /* Initialize Device Manager now */
    if( RETURN_STATUS_OK != InitDeviceManager( &refAppData) )
    {
        NlmCmFile__printf("\tDevice Manager Initialization failed for %d-PORT/ %d-SMT mode. Exiting...\n",
                                                        (1+refAppData.portMode),(1+refAppData.smtMode));

        return RETURN_STATUS_ABORT;
    }

    errNum = NlmGenericTableManager_nosmt( &refAppData);

    if( RETURN_STATUS_OK != errNum)
    {
        NlmCmFile__printf("\tError in refapp ...\n");
        return RETURN_STATUS_ABORT;
    }

    /* destroy device manager */
    if( RETURN_STATUS_OK != DestroyDeviceManager( &refAppData ) )
    {
        NlmCmFile__printf("\tDevice Manager Destroy failed. Exiting...\n");

        return RETURN_STATUS_ABORT;
    }

    if( RETURN_STATUS_OK != DestroyEnvironment( &refAppData ) )
    {
        NlmCmFile__printf("\tDestroy Environment failed. Exiting...\n");

        return RETURN_STATUS_ABORT;
    }

    if(NlmCmDebug__IsMemLeak())
    {
        NlmCmFile__printf("\tMemory Leak\n");
    }

    NlmCmFile__printf("\n\tProgram Completed Successfully\n");

    return 0;
}

