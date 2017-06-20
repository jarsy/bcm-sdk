/*
 * $Id: nlmgenerictblmgr.c,v 1.1.6.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#include "nlmgenerictblmgr_refapp.h"

/* this bank mode will decide to run 
    1. single bank mode  (4x parallel srchs, only bank 0, key:0-3 and rs_port:0-3)

    2. dual bank mode    (2x parallel srchs, either bank 0/1, key:0-1 and rs_port:0-1)
                         (2x parallel srchs, either bank 0/1, key:2-3 and rs_port:2-3)
*/




/* Number of devices */
#define NLM_REFAPP_NUM_OF_DEVICES       1


static void CreateRecordsInTables_nosmt( genericTblMgrRefAppData *refAppData_p);
static nlm_u8 FillRecordPatternFor_80B0_Table( nlm_u8 *data_p, nlm_u8 *mask_p );
static nlm_u8 FillRecordPatternFor_160B0_Table( nlm_u8 *data_p, nlm_u8 *mask_p );

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

    if(genericTbl_p->m_genericTblMgr_p->m_bankType == NLMDEV_BANK_1)
        instance = 1;

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
#ifndef NLMPLATFORM_BCM
            refAppData_p->xpt_p = bcm_kbp_simxpt_init( refAppData_p->alloc_p,
                                                     NLM_DEVTYPE_3,
                                                     refAppData_p->request_queue_len,
                                                     refAppData_p->result_queue_len,
                                                     refAppData_p->channel_id
                                                    );
#else
            refAppData_p->xpt_p = kbp_simxpt_init( refAppData_p->alloc_p,
                                                     NLM_DEVTYPE_3,
                                                     refAppData_p->request_queue_len,
                                                     refAppData_p->result_queue_len,
                                                     refAppData_p->channel_id
                                                    );
#endif

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
#ifndef NLMPLATFORM_BCM
            bcm_kbp_simxpt_destroy( refAppData_p->xpt_p );
#else
            kbp_simxpt_destroy( refAppData_p->xpt_p );
#endif
            
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
    NlmReasonCode   reason = NLMRSC_REASON_OK;

    if(refAppData_p->smtMode != NLMDEV_NO_SMT_MODE && 
        refAppData_p->smtMode != NLMDEV_DUAL_SMT_MODE)
    {
        NlmCmFile__printf("\tInvalid bank mode...\n" );
        return RETURN_STATUS_ABORT;
    }

#ifndef NLMPLATFORM_BCM
    if( NULL == ( refAppData_p->devMgr_p = bcm_kbp_dm_init( refAppData_p->alloc_p,
                                                                refAppData_p->xpt_p,
                                                                NLM_DEVTYPE_3,
                                                                refAppData_p->portMode,
                                                                refAppData_p->smtMode,
                                                                0, /* ignored in 12K mode */
                                                               &reason
                                                              ) ) )
#else
    if( NULL == ( refAppData_p->devMgr_p = kbp_dm_init( refAppData_p->alloc_p,
                                                                refAppData_p->xpt_p,
                                                                NLM_DEVTYPE_3,
                                                                refAppData_p->portMode,
                                                                refAppData_p->smtMode,
                                                                0, /* ignored in 12K mode */
                                                               &reason
                                                              ) ) )
#endif
    {
        NlmCmFile__printf("\tCould not initialize Device Manager...\n" );
        printReasonCode( reason );

        return RETURN_STATUS_ABORT;
    }

    NlmCmFile__printf("\n\tDevice Manager Initialized Successfully\n");

    
    
    /* Now add a device to the search system */
#ifndef NLMPLATFORM_BCM
    if( NULL == (refAppData_p->dev_p = bcm_kbp_dm_add_device(refAppData_p->devMgr_p,
                                                                     &dev_id,
                                                                     &reason
                                                                    ) ) )
#else
    if( NULL == (refAppData_p->dev_p = kbp_dm_add_device(refAppData_p->devMgr_p,
                                                                     &dev_id,
                                                                     &reason
                                                                    ) ) )
#endif

    {
        NlmCmFile__printf("Could not add device to the search system...\n");
        printReasonCode( reason );

        return RETURN_STATUS_ABORT;
    }

    NlmCmFile__printf("\tDevice#0 Added to the search system\n");

    /* We are done with configurations. Now Lock Device Manager */
#ifndef NLMPLATFORM_BCM
    if( NLMERR_OK != bcm_kbp_dm_lock_config( refAppData_p->devMgr_p, &reason ) )
#else
    if( NLMERR_OK != kbp_dm_lock_config( refAppData_p->devMgr_p, &reason ) )
#endif
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
#ifndef NLMPLATFORM_BCM
    bcm_kbp_dm_destroy( refAppData_p->devMgr_p );
#else
    kbp_dm_destroy( refAppData_p->devMgr_p );
#endif

    return RETURN_STATUS_OK;
}

static void CreateRecordsInTables_smt(genericTblMgrRefAppData *refAppData_p,
                                  nlm_u8 instance
                                  )
{
    tableInfo *tblInfo_p;
    tableRecordInfo *tblRecordInfo_p;
    nlm_u32 alloc_size, iter_rec;
    nlm_u8   tblWidth_inBytes, start_byte = 0;
    nlm_u8  *rec_data, *rec_mask, *var_data_p;
    nlm_u16  start, end, iter_group, iter_priority;

    /* First allocate memory for storing records within the data structures.
     * Each table structure has start groupId and end groupId. Records of
     * all groupIds including the start-end groupId are added. With each
     * groupId, priorities of the records would range from [0 TO groupId-1].
     * For e.g. if the start-end groupId are 5-7, then 5 records with priorities
     * ranging from 0-4 are created, 6 records with priorities from 0-5 are
     * created and so on. Hence the formula to calculate the table size is as
     * as follows:
     * n(n+1)/2 - m(m-1)/2 ; given the start-end groupIds being m-n
     */
        tblInfo_p = &refAppData_p->g_gtmInfo[instance].tblInfo;
        start     = tblInfo_p->groupId_start;
        end       = tblInfo_p->groupId_end;

        alloc_size = ( ( end * (end+1) ) - ( start * (start-1) ) ) / 2;

        tblInfo_p->tblRecordInfo_p = NlmCmAllocator__calloc( refAppData_p->alloc_p,
                                                             alloc_size,
                                                             sizeof( tableRecordInfo ) );
        tblInfo_p->max_recCount    = alloc_size;

        tblWidth_inBytes = (nlm_u8)(tblInfo_p->tbl_width / 8);
        rec_data = NlmCmAllocator__calloc( refAppData_p->alloc_p,
                                            1, tblWidth_inBytes );
        rec_mask = NlmCmAllocator__calloc( refAppData_p->alloc_p,
                                            1, tblWidth_inBytes );


        /* get the starting record pattern based on the table id */
        switch( tblInfo_p->tbl_id )
        {
            case 0:
            {
                start_byte = FillRecordPatternFor_80B0_Table( rec_data, rec_mask );
                break;
            }
            case 1:
            {
                start_byte = FillRecordPatternFor_160B0_Table( rec_data, rec_mask );
                break;
            }
            default:
                break;
        }

        /* 4 bytes starting from this byte location are incremented. These 4 bytes are
         * the only variable data portion across all records.
         */
        var_data_p = rec_data + start_byte;

        /* Start filling the records now */
        tblRecordInfo_p = tblInfo_p->tblRecordInfo_p;
        for( iter_group = start, iter_rec = 0; iter_group <= end; iter_group++ )
        {
            for( iter_priority = 0; iter_priority < iter_group; iter_priority++ )
            {
                WriteValueToBitVector4( var_data_p, iter_rec );

                tblRecordInfo_p->groupId  = iter_group;
                tblRecordInfo_p->priority = iter_priority;

                tblRecordInfo_p->record.m_data = NlmCmAllocator__calloc( refAppData_p->alloc_p,
                                                                         1, tblWidth_inBytes );
                tblRecordInfo_p->record.m_mask = NlmCmAllocator__calloc( refAppData_p->alloc_p,
                                                                         1, tblWidth_inBytes );
                tblRecordInfo_p->record.m_len  = tblInfo_p->tbl_width;

                NlmCm__memcpy( tblRecordInfo_p->record.m_data, rec_data, tblWidth_inBytes );
                NlmCm__memcpy( tblRecordInfo_p->record.m_mask, rec_mask, tblWidth_inBytes );

                /* Dont worry, we wont over index. */
                tblRecordInfo_p++;
                iter_rec++;
            }
        }

        /* Free the memory allocated for temp data and mask. A fresh memory will be
         * based on the table width
         */
        NlmCmAllocator__free( refAppData_p->alloc_p, rec_data );
        NlmCmAllocator__free( refAppData_p->alloc_p, rec_mask );

        return;

}

void InitGenericTableManager_smtGetACMapValue(NlmBankNum whichBank,
                                  NlmGenericTblMgrSBRange *m_blkRng,
                                  NlmDevConfigReg *configData_p
                                  )
{   
    nlm_u32 value = 0;
    nlm_u32 stSbBlk = 0, endSbBlk = 0;
    nlm_u32 shift = 0;

    /* get the superblocks from the block range */
    stSbBlk  = m_blkRng->m_stSBNum;
    endSbBlk = m_blkRng->m_endSBNum;

    /* There is one bit per AC (AC has 2 SBs) */
    shift    = (nlm_u32)(stSbBlk/2);

    while(stSbBlk < endSbBlk)
    {
        value += whichBank << shift;

        stSbBlk +=2; /* increment the SB by 2 */
        shift++;
    }

    configData_p->m_ACtoBankMapping |= (nlm_u16)value;

    return;
}


static int InitGenericTableManager_smt(genericTblMgrRefAppData *refAppData_p,
    NlmBool fromRange)
                                  
{
    NlmReasonCode reason;
    nlm_u16 i;


        /* Call back function called by GTM when index for a record is changed */
        refAppData_p->indexChangeCB = IndexChangeCallBack;
        
        /* Set the Generic Table Manager block range */
        refAppData_p->g_gtmInfo[0].dbaSBRange.m_stSBNum  = NLM_GTM_REFAPP_80B_DBA_START_SB ;
        refAppData_p->g_gtmInfo[0].dbaSBRange.m_endSBNum = NLM_GTM_REFAPP_80B_DBA_END_SB;

        refAppData_p->g_gtmInfo[0].udaSBRange.m_stSBNum = NLM_GTM_REFAPP_80B_UDA_START_SB;
        refAppData_p->g_gtmInfo[0].udaSBRange.m_endSBNum = NLM_GTM_REFAPP_80B_UDA_END_SB;


        refAppData_p->g_gtmInfo[1].dbaSBRange.m_stSBNum  = NLM_GTM_REFAPP_160B_DBA_START_SB ;
        refAppData_p->g_gtmInfo[1].dbaSBRange.m_endSBNum = NLM_GTM_REFAPP_160B_DBA_END_SB;

        refAppData_p->g_gtmInfo[1].udaSBRange.m_stSBNum = NLM_GTM_REFAPP_160B_UDA_START_SB;
        refAppData_p->g_gtmInfo[1].udaSBRange.m_endSBNum = NLM_GTM_REFAPP_160B_UDA_END_SB;

    for(i = 0; i< 2; i++)
    {

        NlmCmFile__printf("\n\tDBA Super Blocks %d %d are mapped to Bank-%d \n",
            refAppData_p->g_gtmInfo[i].dbaSBRange.m_stSBNum,
            refAppData_p->g_gtmInfo[i].dbaSBRange.m_endSBNum,i);
        NlmCmFile__printf("\n\tUDA Super Blocks %d %d are mapped to Bank-%d \n",
            refAppData_p->g_gtmInfo[i].udaSBRange.m_stSBNum,
            refAppData_p->g_gtmInfo[i].udaSBRange.m_endSBNum,i);


        /* Write AC to Bank mapping, here table-2 belongs to BANK-1 */
        if(i)
        {
            nlm_u32 ErrNum = 0;
            NlmDev *dev_p = refAppData_p->devMgr_p->m_devList_pp[0];
            NlmDevShadowDevice *shadowDev_p = ((NlmDevShadowDevice*)refAppData_p->dev_p->m_shadowDevice_p);

            InitGenericTableManager_smtGetACMapValue(refAppData_p->g_gtmInfo[i].bankNum,
                                                     &refAppData_p->g_gtmInfo[i].dbaSBRange,
                                                     &shadowDev_p->m_global->m_devConfig);
#ifndef NLMPLATFORM_BCM
            if((ErrNum = bcm_kbp_dm_global_reg_write(dev_p, 
                    NLMDEV_PORT_1, NLMDEV_DEVICE_CONFIG_REG, 
                    &(shadowDev_p->m_global->m_devConfig), &reason))!= NLMERR_OK )
#else
            if((ErrNum = kbp_dm_global_reg_write(dev_p, 
                    NLMDEV_PORT_1, NLMDEV_DEVICE_CONFIG_REG, 
                    &(shadowDev_p->m_global->m_devConfig), &reason))!= NLMERR_OK )
#endif
            {
                NlmCm__printf("\n\t kbp_dm_global_reg_write() failed for BANK-1 \n");
                return ErrNum;
            }


        }
#ifndef NLMPLATFORM_BCM
        if( NULL == ( refAppData_p->g_gtmInfo[i].genericTblMgr_p = bcm_kbp_gtm_init(
                                                            refAppData_p->alloc_p,
                                                            refAppData_p->devMgr_p,
                                                            NLM_DEVTYPE_3,
                                                            refAppData_p->g_gtmInfo[i].bankNum,
                                                            NLM_REFAPP_NUM_OF_DEVICES,
                                                            &refAppData_p->g_gtmInfo[i].dbaSBRange,
                                                            &refAppData_p->g_gtmInfo[i].udaSBRange,                                                         
                                                            refAppData_p->indexChangeCB,
                                                            refAppData_p,
                                                            &reason ) ) )
#else
        if( NULL == ( refAppData_p->g_gtmInfo[i].genericTblMgr_p = kbp_gtm_init(
                                                            refAppData_p->alloc_p,
                                                            refAppData_p->devMgr_p,
                                                            NLM_DEVTYPE_3,
                                                            refAppData_p->g_gtmInfo[i].bankNum,
                                                            NLM_REFAPP_NUM_OF_DEVICES,
                                                            &refAppData_p->g_gtmInfo[i].dbaSBRange,
                                                            &refAppData_p->g_gtmInfo[i].udaSBRange,                                                         
                                                            refAppData_p->indexChangeCB,
                                                            refAppData_p,
                                                            &reason ) ) )
#endif
        {
            NlmCmFile__printf("\tGTM Init failed...\n");
            printReasonCode( reason );

            return RETURN_STATUS_ABORT;
        }


        NlmCmFile__printf("\n\tGeneric Table Manager Initialized Successfully for BANK:%d \n\n",i);

    }

    /* Store table details for later use */
    refAppData_p->g_gtmInfo[0].tblInfo.tbl_id = TABLE_ID_80B_0;
    refAppData_p->g_gtmInfo[0].tblInfo.tbl_width     = TABLE_WIDTH_80B_0;
    refAppData_p->g_gtmInfo[0].tblInfo.tbl_size      = TABLE_SIZE_80B_0;
    refAppData_p->g_gtmInfo[0].tblInfo.tbl_assoWidth = TABLE_80B_0_AD_WIDTH;
    refAppData_p->g_gtmInfo[0].tblInfo.groupId_start = START_GROUPID_80B_O;
    refAppData_p->g_gtmInfo[0].tblInfo.groupId_end   = END_GROUPID_80B_O;

    refAppData_p->g_gtmInfo[1].tblInfo.tbl_id = TABLE_ID_160B_1;
    refAppData_p->g_gtmInfo[1].tblInfo.tbl_width     = TABLE_WIDTH_160B_1;
    refAppData_p->g_gtmInfo[1].tblInfo.tbl_size      = TABLE_SIZE_160B_1;
    refAppData_p->g_gtmInfo[1].tblInfo.tbl_assoWidth = TABLE_160B_0_AD_WIDTH;
    refAppData_p->g_gtmInfo[1].tblInfo.groupId_start = START_GROUPID_160B_1;
    refAppData_p->g_gtmInfo[1].tblInfo.groupId_end   = END_GROUPID_160B_1;


    for( i =0 ;i < 2; i++)
    {
    /*Call the CreateTable API to create each of the tables */
#ifndef NLMPLATFORM_BCM
        if( NULL == ( refAppData_p->g_gtmInfo[i].tbl_p = bcm_kbp_gtm_create_table(
                                                    refAppData_p->g_gtmInfo[i].genericTblMgr_p,
                                                    i,
                                                    refAppData_p->g_gtmInfo[i].tblInfo.tbl_id,
                                                    refAppData_p->g_gtmInfo[i].tblInfo.tbl_width,
                                                    refAppData_p->g_gtmInfo[i].tblInfo.tbl_assoWidth,
                                                    refAppData_p->g_gtmInfo[i].tblInfo.tbl_size,
                                                    &reason ) ) )
#else
        if( NULL == ( refAppData_p->g_gtmInfo[i].tbl_p = kbp_gtm_create_table(
                                                    refAppData_p->g_gtmInfo[i].genericTblMgr_p,
                                                    i,
                                                    refAppData_p->g_gtmInfo[i].tblInfo.tbl_id,
                                                    refAppData_p->g_gtmInfo[i].tblInfo.tbl_width,
                                                    refAppData_p->g_gtmInfo[i].tblInfo.tbl_assoWidth,
                                                    refAppData_p->g_gtmInfo[i].tblInfo.tbl_size,
                                                    &reason ) ) )
#endif
        {
            NlmCmFile__printf("\tTable [%d] created on Bank - %d creation failed...\n",
                                refAppData_p->g_gtmInfo[i].tblInfo.tbl_id,i);
            printReasonCode( reason );

            return RETURN_STATUS_FAIL;
        }
        else
        {
            NlmCmFile__printf("\tTable [%d] creation succeded for BANK: %d...\n",
                                refAppData_p->g_gtmInfo[i].tblInfo.tbl_id,i);
        }

    }
    NlmCmFile__printf("\n\n");
    
     /* Configure searches now */
     {
        NlmGenericTblParallelSearchInfo  *psInfo_p = NULL;

        refAppData_p->g_gtmInfo[0].ltr_num = 0;
        refAppData_p->g_gtmInfo[0].search_attrs.m_numOfParallelSrches  = 1;
        refAppData_p->g_gtmInfo[0].search_attrs.m_isCmp3Search = NlmFalse; 
            
        psInfo_p = &refAppData_p->g_gtmInfo[0].search_attrs.m_psInfo[0];
        psInfo_p->m_tblId = TABLE_ID_80B_0;
        psInfo_p->m_rsltPortNum = 0 ;
        psInfo_p->m_keyNum = NLMDEV_KEY_0 ;
        psInfo_p->m_kcm.m_segmentStartByte[0] = 0;
        psInfo_p->m_kcm.m_segmentNumOfBytes[0] = 10;
        psInfo_p->m_kcm.m_segmentIsZeroFill[0] = 0;

         /*Call the ConfigSearch API to configure each of the LTRs */
#ifndef NLMPLATFORM_BCM            
            if( NLMERR_OK != bcm_kbp_gtm_config_search(
                                        refAppData_p->g_gtmInfo[0].genericTblMgr_p,
                                        NLMDEV_PORT_0,
                                        refAppData_p->g_gtmInfo[0].ltr_num,
                                        &refAppData_p->g_gtmInfo[0].search_attrs,
                                        &reason ) )
#else
        if( NLMERR_OK != kbp_gtm_config_search(
                                        refAppData_p->g_gtmInfo[0].genericTblMgr_p,
                                        NLMDEV_PORT_0,
                                        refAppData_p->g_gtmInfo[0].ltr_num,
                                        &refAppData_p->g_gtmInfo[0].search_attrs,
                                        &reason ) )
#endif

            {
                NlmCmFile__printf("\tLTR#%d Configuration for 1x parallel search on SMT-0 failed...\n",
                                        refAppData_p->g_gtmInfo[0].ltr_num); 
                printReasonCode( reason );

                return RETURN_STATUS_FAIL;
            }
            else
            {
                NlmCmFile__printf("\tLTR#%d Configured for 1x parallel search on  BANK :0  ...\n",
                    refAppData_p->g_gtmInfo[0].ltr_num); 

            }


        refAppData_p->g_gtmInfo[1].ltr_num = 64;
        refAppData_p->g_gtmInfo[1].search_attrs.m_numOfParallelSrches  = 1;
        refAppData_p->g_gtmInfo[1].search_attrs.m_isCmp3Search = NlmFalse;
            
        psInfo_p = &refAppData_p->g_gtmInfo[1].search_attrs.m_psInfo[0];
        psInfo_p->m_tblId = TABLE_ID_160B_1;
        psInfo_p->m_rsltPortNum = 2;
        psInfo_p->m_keyNum = NLMDEV_KEY_2;
        psInfo_p->m_kcm.m_segmentStartByte[0] = 0;
        psInfo_p->m_kcm.m_segmentNumOfBytes[0] = 16;
        psInfo_p->m_kcm.m_segmentIsZeroFill[0] = 0;
        psInfo_p->m_kcm.m_segmentStartByte[1] = 16;
        psInfo_p->m_kcm.m_segmentNumOfBytes[1] = 4;
        psInfo_p->m_kcm.m_segmentIsZeroFill[1] = 0;

        /*Call the ConfigSearch API to configure each of the LTRs */
#ifndef NLMPLATFORM_BCM
            if( NLMERR_OK != bcm_kbp_gtm_config_search(
                                        refAppData_p->g_gtmInfo[1].genericTblMgr_p,
                                        NLMDEV_PORT_1,
                                        refAppData_p->g_gtmInfo[1].ltr_num,
                                        &refAppData_p->g_gtmInfo[1].search_attrs,
                                        &reason ) )
#else
        if( NLMERR_OK != kbp_gtm_config_search(
                                        refAppData_p->g_gtmInfo[1].genericTblMgr_p,
                                        NLMDEV_PORT_1,
                                        refAppData_p->g_gtmInfo[1].ltr_num,
                                        &refAppData_p->g_gtmInfo[1].search_attrs,
                                        &reason ) )
#endif
            {
                NlmCmFile__printf("\tLTR#%d Configuration for 1x parallel search on SMT-0 failed...\n",
                                        refAppData_p->g_gtmInfo[1].ltr_num); 
                printReasonCode( reason );

                return RETURN_STATUS_FAIL;
            }
            else
            {
                NlmCmFile__printf("\tLTR#%d Configured for 1x parallel search on BANK: 1 ...\n",
                    refAppData_p->g_gtmInfo[1].ltr_num); 

            }
        
     }        /* end of configurations */

    NlmCmFile__printf("\n\n");

    for(i =0 ;i<2 ;i++)
    {
        /* Configuration is finished, Lock it now */
#ifndef NLMPLATFORM_BCM
        if( NLMERR_OK != bcm_kbp_gtm_lock_config(
                                        refAppData_p->g_gtmInfo[i].genericTblMgr_p,
                                        i,
                                        &reason ) )
#else
        if( NLMERR_OK != kbp_gtm_lock_config(
                                        refAppData_p->g_gtmInfo[i].genericTblMgr_p,
                                        i,
                                        &reason ) )
#endif
        {
            NlmCmFile__printf("\tGTM LockConfig failed...\n"); 
            printReasonCode( reason );

            return RETURN_STATUS_FAIL;
        }
    }
    NlmCmFile__printf("\tGTM Configuration Locked\n\n");

    if(fromRange == NlmFalse)
    {
        /* Pre Create records to be inserted into various tables. */
        CreateRecordsInTables_smt( refAppData_p , 0 );
        CreateRecordsInTables_smt( refAppData_p , 1 );
    }

    return RETURN_STATUS_OK;

}




/* This function performs Generic Table Manager related Inits */
static int InitGenericTableManager_nosmt(genericTblMgrRefAppData *refAppData_p)
                                  
{
    NlmReasonCode   reason;

    /* Set the Generic Table Manager block range */
    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].dbaSBRange.m_stSBNum  = NLM_GTM_REFAPP_NOSMT_80B_DBA_START_SB;
    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].dbaSBRange.m_endSBNum = NLM_GTM_REFAPP_NOSMT_80B_DBA_END_SB;

    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].udaSBRange.m_stSBNum = NLM_GTM_REFAPP_NOSMT_80B_UDA_START_SB;
    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].udaSBRange.m_endSBNum = NLM_GTM_REFAPP_NOSMT_80B_UDA_END_SB;


    NlmCmFile__printf("\n\tDBA Super Blocks %d %d are mapped to Bank-0 \n",
            refAppData_p->g_gtmInfo[NLMDEV_SMT_0].dbaSBRange.m_stSBNum,
            refAppData_p->g_gtmInfo[NLMDEV_SMT_0].dbaSBRange.m_endSBNum);
        NlmCmFile__printf("\n\tUDA Super Blocks %d %d are mapped to Bank-0 \n",
            refAppData_p->g_gtmInfo[NLMDEV_SMT_0].udaSBRange.m_stSBNum,
            refAppData_p->g_gtmInfo[NLMDEV_SMT_0].udaSBRange.m_endSBNum);


    /* Call back function called by Cynapse when index for a record is changed */
    refAppData_p->indexChangeCB = IndexChangeCallBack;
#ifndef NLMPLATFORM_BCM
    if( NULL == ( refAppData_p->g_gtmInfo[0].genericTblMgr_p = bcm_kbp_gtm_init(
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
#else
    if( NULL == ( refAppData_p->g_gtmInfo[0].genericTblMgr_p = kbp_gtm_init(
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
#endif
    {
        NlmCmFile__printf("\tGTM Init failed...\n");
        printReasonCode( reason );

        return RETURN_STATUS_ABORT;
    }

    NlmCmFile__printf("\n\tGeneric Table Manager Initialized Successfully\n\n");

    /* Create tables now, 6 tables are created */

    /* Store table details for later use */
    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].tblInfo.tbl_id =  TABLE_ID_80B_0;
    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].tblInfo.tbl_width     = TABLE_WIDTH_80B_0;
    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].tblInfo.tbl_size      = TABLE_SIZE_80B_0;
    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].tblInfo.tbl_assoWidth = TABLE_80B_0_AD_WIDTH;
    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].tblInfo.groupId_start = START_GROUPID_80B_O;
    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].tblInfo.groupId_end   = END_GROUPID_80B_O;

    /*Call the CreateTable API to create each of the tables */
#ifndef NLMPLATFORM_BCM
        if( NULL == ( refAppData_p->g_gtmInfo[0].tbl_p = bcm_kbp_gtm_create_table(
                                                    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].genericTblMgr_p,
                                                    NLMDEV_PORT_0,
                                                    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].tblInfo.tbl_id,
                                                    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].tblInfo.tbl_width,
                                                    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].tblInfo.tbl_assoWidth,
                                                    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].tblInfo.tbl_size,
                                                    &reason ) ) )
#else
    if( NULL == ( refAppData_p->g_gtmInfo[0].tbl_p = kbp_gtm_create_table(
                                                    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].genericTblMgr_p,
                                                    NLMDEV_PORT_0,
                                                    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].tblInfo.tbl_id,
                                                    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].tblInfo.tbl_width,
                                                    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].tblInfo.tbl_assoWidth,
                                                    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].tblInfo.tbl_size,
                                                    &reason ) ) )
#endif
        {
            NlmCmFile__printf("\tTable [%d] creation failed...\n",
                                refAppData_p->g_gtmInfo[NLMDEV_SMT_0].tblInfo.tbl_id);
            printReasonCode( reason );

            return RETURN_STATUS_FAIL;
        }
        else
        {
            NlmCmFile__printf("\tTable [%d] creation succeded...\n",
                                refAppData_p->g_gtmInfo[NLMDEV_SMT_0].tblInfo.tbl_id);
        }

    NlmCmFile__printf("\n\n");
    
     /* Configure searches now */
     {
        NlmGenericTblParallelSearchInfo  *psInfo_p = NULL;

        refAppData_p->g_gtmInfo[NLMDEV_SMT_0].ltr_num = 0;
        refAppData_p->g_gtmInfo[NLMDEV_SMT_0].search_attrs.m_numOfParallelSrches  = 1;
        refAppData_p->g_gtmInfo[NLMDEV_SMT_0].search_attrs.m_isCmp3Search = NlmFalse;
            
        psInfo_p = &refAppData_p->g_gtmInfo[NLMDEV_SMT_0].search_attrs.m_psInfo[0];
        psInfo_p->m_tblId = TABLE_ID_80B_0;
        psInfo_p->m_rsltPortNum = 0 ;
        psInfo_p->m_keyNum = 0 ;
        psInfo_p->m_kcm.m_segmentStartByte[0] = 0;
        psInfo_p->m_kcm.m_segmentNumOfBytes[0] = 10;
        psInfo_p->m_kcm.m_segmentIsZeroFill[0] = 0;

        /*Call the ConfigSearch API to configure each of the LTRs */
#ifndef NLMPLATFORM_BCM            
            if( NLMERR_OK != bcm_kbp_gtm_config_search(
                                        refAppData_p->g_gtmInfo[NLMDEV_SMT_0].genericTblMgr_p,
                                        NLMDEV_PORT_0,
                                        refAppData_p->g_gtmInfo[NLMDEV_SMT_0].ltr_num,
                                        &refAppData_p->g_gtmInfo[NLMDEV_SMT_0].search_attrs,
                                        &reason ) )
#else
        if( NLMERR_OK != kbp_gtm_config_search(
                                        refAppData_p->g_gtmInfo[NLMDEV_SMT_0].genericTblMgr_p,
                                        NLMDEV_PORT_0,
                                        refAppData_p->g_gtmInfo[NLMDEV_SMT_0].ltr_num,
                                        &refAppData_p->g_gtmInfo[NLMDEV_SMT_0].search_attrs,
                                        &reason ) )
#endif
            {
                NlmCmFile__printf("\tLTR#%d ConfigSearch  failed...\n",
                                        refAppData_p->g_gtmInfo[NLMDEV_SMT_0].ltr_num); 
                printReasonCode( reason );

                return RETURN_STATUS_FAIL;
            }
            else
            {
                NlmCmFile__printf("\tLTR#%d ConfigSearch (%dX parallel searches)  done...\n",
                    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].ltr_num,
                    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].search_attrs.m_numOfParallelSrches); 

            }

           
        
    }         /* end of configurations */

    NlmCmFile__printf("\n\n");

    /* Configuration is finished, Lock it now */
#ifndef NLMPLATFORM_BCM
    if( NLMERR_OK != bcm_kbp_gtm_lock_config(
                                    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].genericTblMgr_p,
                                    NLMDEV_PORT_0,
                                    &reason ) )
#else
    if( NLMERR_OK != kbp_gtm_lock_config(
                                    refAppData_p->g_gtmInfo[NLMDEV_SMT_0].genericTblMgr_p,
                                    NLMDEV_PORT_0,
                                    &reason ) )
#endif

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
static int DestroyGenericTableManager_smt(genericTblMgrRefAppData *refAppData_p,
                                      nlm_u8 instance,
                                      NlmBool fromRange
                                      )
{
    tableInfo *tblInfo_p;
    tableRecordInfo *tblRecordInfo_p;
    nlm_u32 iter_tbl = 0, iter_rec, num_recs;
    NlmReasonCode   reason = NLMRSC_REASON_OK;

    if(fromRange == NlmFalse)
    {
    /* First free memory allocated for keeping records within Ref App. Next step is to 
     * destroy tables and finally calling destroy table manager.
     */
        iter_tbl = instance;
        tblInfo_p = &refAppData_p->g_gtmInfo[instance].tblInfo;
        tblRecordInfo_p = tblInfo_p->tblRecordInfo_p;

        num_recs = tblInfo_p->max_recCount;
        for( iter_rec = 0; iter_rec < num_recs; tblRecordInfo_p++, iter_rec++ )
        {
            NlmCmAllocator__free( refAppData_p->alloc_p, tblRecordInfo_p->record.m_data );
            NlmCmAllocator__free( refAppData_p->alloc_p, tblRecordInfo_p->record.m_mask );
        }

        /* tblRecordInfo is calloced, free that space */
        NlmCmAllocator__free( refAppData_p->alloc_p, tblInfo_p->tblRecordInfo_p );
    }
    
        /* And destroy the table */
#ifndef NLMPLATFORM_BCM
        if( NLMERR_OK != bcm_kbp_gtm_destroy_table( 
                                    refAppData_p->g_gtmInfo[instance].genericTblMgr_p,
                                    instance, 
                                    refAppData_p->g_gtmInfo[instance].tbl_p,
                                     &reason ) )
#else
    if( NLMERR_OK != kbp_gtm_destroy_table( 
                                    refAppData_p->g_gtmInfo[instance].genericTblMgr_p,
                                    instance, 
                                    refAppData_p->g_gtmInfo[instance].tbl_p,
                                     &reason ) )
#endif
        {
            NlmCmFile__printf("\tDestroyTable#%u failed...\n", iter_tbl);
            printReasonCode( reason );
        }
    NlmCmFile__printf("\n");

    /* Time to destroy the manager now. */
    NlmCmFile__printf("\n\tDestroying Generic Table Manager for BANK:%d \n",instance);
#ifndef NLMPLATFORM_BCM
    if( NLMERR_OK != bcm_kbp_gtm_destroy( refAppData_p->g_gtmInfo[instance].genericTblMgr_p,
                                instance, &reason ) )
#else
    if( NLMERR_OK != kbp_gtm_destroy( refAppData_p->g_gtmInfo[instance].genericTblMgr_p,
                                instance, &reason ) )
#endif
    {
        NlmCmFile__printf("\tGeneric Table Manager destroy failed...\n");
        printReasonCode( reason );

        return NLMERR_FAIL;
    }

    return RETURN_STATUS_OK;
}


/* Destroy generic tables and manager instance */
static int DestroyGenericTableManager(genericTblMgrRefAppData *refAppData_p,
                                      nlm_u8 instance
                                      )
{
    tableInfo *tblInfo_p;
    tableRecordInfo *tblRecordInfo_p;
    nlm_u32 iter_tbl, iter_rec, num_recs;
    NlmReasonCode   reason = NLMRSC_REASON_OK;

    /* First free memory allocated for keeping records within Ref App. Next step is to 
     * destroy tables and finally calling destroy table manager.
     */
    for( iter_tbl = 0; iter_tbl < NUM_OF_TABLES; iter_tbl++ )
    {
        tblInfo_p = &refAppData_p->g_gtmInfo[instance].tblInfo;
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
#ifndef NLMPLATFORM_BCM
        if( NLMERR_OK != bcm_kbp_gtm_destroy_table( refAppData_p->g_gtmInfo[instance].genericTblMgr_p,
                                         NLMDEV_PORT_0,
                                         refAppData_p->g_gtmInfo[instance].tbl_p,
                                         &reason ) )
#else
        if( NLMERR_OK != kbp_gtm_destroy_table( refAppData_p->g_gtmInfo[instance].genericTblMgr_p,
                                         NLMDEV_PORT_0,
                                         refAppData_p->g_gtmInfo[instance].tbl_p,
                                         &reason ) )
#endif
        {
            NlmCmFile__printf("\tDestroyTable#%u failed...\n", iter_tbl);
            printReasonCode( reason );
        }
    }
    NlmCmFile__printf("\n");

    /* Time to destroy the manager now. */
    NlmCmFile__printf("\n\tDestroying Generic Table Manager\n");
#ifndef NLMPLATFORM_BCM
    if( NLMERR_OK != bcm_kbp_gtm_destroy( refAppData_p->g_gtmInfo[instance].genericTblMgr_p, 
                                            NLMDEV_PORT_0, &reason ) )
#else
    if( NLMERR_OK != kbp_gtm_destroy( refAppData_p->g_gtmInfo[instance].genericTblMgr_p, 
                                            NLMDEV_PORT_0, &reason ) )
#endif

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
    nlm_u32 alloc_size, iter_rec;
    nlm_u8  iter_tbl, tblWidth_inBytes, start_byte = 0;
    nlm_u8  *rec_data, *rec_mask, *var_data_p;
    nlm_u16  start, end, iter_group, iter_priority;

    /* First allocate memory for storing records within the data structures.
     * Each table structure has start groupId and end groupId. Records of
     * all groupIds including the start-end groupId are added. With each
     * groupId, priorities of the records would range from [0 TO groupId-1].
     * For e.g. if the start-end groupId are 5-7, then 5 records with priorities
     * ranging from 0-4 are created, 6 records with priorities from 0-5 are
     * created and so on. Hence the formula to calculate the table size is as
     * as follows:
     * n(n+1)/2 - m(m-1)/2 ; given the start-end groupIds being m-n
     */
     for( iter_tbl = 0; iter_tbl < NUM_OF_TABLES; iter_tbl++ )
     {
        tblInfo_p = &refAppData_p->g_gtmInfo[0].tblInfo;
        start     = tblInfo_p->groupId_start;
        end       = tblInfo_p->groupId_end;

        alloc_size = ( ( end * (end+1) ) - ( start * (start-1) ) ) / 2;
        tblInfo_p->tblRecordInfo_p = NlmCmAllocator__calloc( refAppData_p->alloc_p,
                                                             alloc_size,
                                                             sizeof( tableRecordInfo ) );
        tblInfo_p->max_recCount    = alloc_size;
        tblWidth_inBytes = (nlm_u8)(tblInfo_p->tbl_width / 8);
        rec_data = NlmCmAllocator__calloc( refAppData_p->alloc_p,
                                            1, tblWidth_inBytes );
        rec_mask = NlmCmAllocator__calloc( refAppData_p->alloc_p,
                                            1, tblWidth_inBytes );

        /* get the starting record pattern based on the table id */
        switch( tblInfo_p->tbl_id )
        {
            case 0:
            {
                start_byte = FillRecordPatternFor_80B0_Table( rec_data, rec_mask );
                break;
            }
            case 1:
            {
                start_byte = FillRecordPatternFor_160B0_Table( rec_data, rec_mask );
                break;
            }
            default:
                break;
        }

        /* 4 bytes starting from this byte location are incremented. These 4 bytes are
         * the only variable data portion across all records.
         */
        var_data_p = rec_data + start_byte;

        /* Start filling the records now */
        tblRecordInfo_p = tblInfo_p->tblRecordInfo_p;
        for( iter_group = start, iter_rec = 0; iter_group <= end; iter_group++ )
        {
            for( iter_priority = 0; iter_priority < iter_group; iter_priority++ )
            {
                WriteValueToBitVector4( var_data_p, iter_rec );

                tblRecordInfo_p->groupId  = iter_group;
                tblRecordInfo_p->priority = iter_priority;

                tblRecordInfo_p->record.m_data = NlmCmAllocator__calloc( refAppData_p->alloc_p,
                                                                         1, tblWidth_inBytes );
                tblRecordInfo_p->record.m_mask = NlmCmAllocator__calloc( refAppData_p->alloc_p,
                                                                         1, tblWidth_inBytes );
                tblRecordInfo_p->record.m_len  = tblInfo_p->tbl_width;

                NlmCm__memcpy( tblRecordInfo_p->record.m_data, rec_data, tblWidth_inBytes );
                NlmCm__memcpy( tblRecordInfo_p->record.m_mask, rec_mask, tblWidth_inBytes );

                /* Dont worry, we wont over index. */
                tblRecordInfo_p++;
                iter_rec++;
            }
        }

        /* Free the memory allocated for temp data and mask. A fresh memory will be
         * based on the table width
         */
        NlmCmAllocator__free( refAppData_p->alloc_p, rec_data );
        NlmCmAllocator__free( refAppData_p->alloc_p, rec_mask );
     } /* end of table's loop */
}


/* Adds records into various tables */
static int AddRecordsToTables(
                genericTblMgrRefAppData *refAppData_p,
                NlmGenericTbl *tbl_p,
                nlm_u8 flag,
                nlm_u8 instance,
                nlm_u16 adwidth
                )
{
    tableInfo *tblInfo_p;
    tableRecordInfo *tblRecordInfo_p;
    NlmReasonCode   reason = NLMRSC_REASON_OK;
    nlm_u32 iter_rec, num_recs;
    nlm_u16 portNum;
    nlm_u8 adData[8];
    nlm_u8 i=1;

    tblInfo_p = &refAppData_p->g_gtmInfo[instance].tblInfo;
    tblRecordInfo_p = tblInfo_p->tblRecordInfo_p;
    num_recs  = tblInfo_p->max_recCount;

    /* flag decides whether to add records# 0,2,4 and so on OR 1,3,5 and so on */
    iter_rec = 0;
    if( flag )
    {
        tblRecordInfo_p++;
        iter_rec = 1;
    }
    
    /* assign port number */
    if(instance == 0) /* PORT-0 for BANK-0 & PORT-1 for BANK -1 */
    {
        portNum = NLMDEV_PORT_0; 
    }
    else portNum = NLMDEV_PORT_1;


    NlmCmFile__printf("\n\tAdding records into table [%d]\n", tbl_p->m_tblId);
    for( ; iter_rec < num_recs; iter_rec = iter_rec + 2 )
    {
        if(i > 9)
        {
            i = 0;
        }

        NlmCm__memset(adData,i,adwidth/8);

        /* Add record now */
#ifndef NLMPLATFORM_BCM
        if( NLMERR_OK != bcm_kbp_gtm_add_record( tbl_p,
                                                      portNum,
                                                      &tblRecordInfo_p->record,
                                                       adData,
                                                      tblRecordInfo_p->groupId,
                                                      tblRecordInfo_p->priority,
                                                      &tblRecordInfo_p->index,
                                                      &reason ) )
#else
        if( NLMERR_OK != kbp_gtm_add_record( tbl_p,
                                                      portNum,
                                                      &tblRecordInfo_p->record,
                                                       adData,
                                                      tblRecordInfo_p->groupId,
                                                      tblRecordInfo_p->priority,
                                                      &tblRecordInfo_p->index,
                                                      &reason ) )
#endif
        {
            NlmCmFile__printf("\ttable [%d] insertions: could not add record#%u\n",
                                tbl_p->m_tblId, tblInfo_p->rec_count); 
            printReasonCode( reason );

            return NLMERR_FAIL;
        }

        /* Advance the record pointer by two */
        tblRecordInfo_p += 2;
        tblInfo_p->rec_count++;
        i++;

    } /* end of for loop */

    NlmCmFile__printf("\t   Number of records added [%u]\n", tblInfo_p->rec_count);

    return RETURN_STATUS_OK;
}

static int Perform_LTR64_Searches(genericTblMgrRefAppData *refAppData_p
                                 )
{
    tableInfo   *tblInfo0_p;
    tableRecordInfo   *tblRecordInfo0_p;
    NlmGenericTblRecord *tblRecord0_p;
    NlmReasonCode   reason = NLMRSC_REASON_OK;
    nlm_u32 iter_tbl0, numRecs_tbl0, iter;
    nlm_u32 exp_index0 = 0x0ffffffff;
    nlm_u32 search_index0 = 0x0ffffffff;
    nlm_u8  ltr_num;
    nlm_u8 priority = 0;
    nlm_u8 i = 1;
    nlm_u8 adData[8];

    /* Device Manager declarations */
    NlmDevCtxBufferInfo      cb_info;
    NlmDevCmpResult     search_result;
    NlmDevMissHit search_result0 = NLMDEV_HIT;

    tblInfo0_p = &refAppData_p->g_gtmInfo[1].tblInfo;

    numRecs_tbl0 = tblInfo0_p->rec_count;
    tblRecordInfo0_p = tblInfo0_p->tblRecordInfo_p;

    ltr_num = 64;
    
    NlmCmFile__printf("\n\tPerforming LTR#%d searches\n", ltr_num);

    iter = 0;
                
            /* MasterKey[239:80] are constructed from table0 records */
            for( iter_tbl0 = 0; iter_tbl0< numRecs_tbl0; iter_tbl0++ )
            {
                exp_index0 = (tblRecordInfo0_p + iter_tbl0)->index;
                tblRecord0_p = &( (tblRecordInfo0_p + iter_tbl0)->record );
                NlmCm__memcpy( &cb_info.m_data[ 0 ], tblRecord0_p->m_data,
                                    (tblRecord0_p->m_len / 8) );
                if( i > 9)
                {
                    i = 0;
                }
        
                NlmCm__memset(adData,i,8);
                iter++;
                cb_info.m_datalen = 20;
                cb_info.m_cbStartAddr = 8;
                /* Fire Compare1 instruction now */
#ifndef NLMPLATFORM_BCM
                if( NLMERR_OK != bcm_kbp_dm_cbwcmp1( refAppData_p->devMgr_p, NLMDEV_PORT_1, ltr_num,
                                                        &cb_info, &search_result, &reason ) )
#else
                if( NLMERR_OK != kbp_dm_cbwcmp1( refAppData_p->devMgr_p, NLMDEV_PORT_1, ltr_num,
                                                        &cb_info, &search_result, &reason ) )
#endif
                {
                    NlmCmFile__printf("Compare1 Instruction failed. Exiting...\n");
                    printReasonCode( reason );

                    return NLMERR_FAIL;
                }
                else
                {
                    search_result0 = search_result.m_hitOrMiss[2]; 

                    search_index0  = search_result.m_hitIndex[2];

                    /* Hit is expected on all the parallel searches */
                    if( search_result0 == NLMDEV_MISS)
                    {
                        NlmCmFile__printf("Got Miss with MasterKey#%u\n", iter);
                        NlmCmFile__printf("search_result0 = %s\n", 
                                            search_result0 ? "Hit" : "Miss");
                    }
                    else
                    {
                        /* Check whether the indexes returned by search or correct or not? */
                        if( exp_index0 != search_index0)
                        {
                            NlmCmFile__printf("Search failed with MasterKey#%u\n", iter);
                            NlmCmFile__printf("exp_index0 = %u, search_index0 = %u\n",
                                                exp_index0, search_index0);
                        }
                    }
                    /* AD verification */
                    if(search_result.m_respType[2] ==  NLMDEV_INDEX_AND_64B_AD)
                    {
                        if( NlmCm__memcmp(adData,search_result.m_AssocData[2],8) != 0)
                        {
                            printf("\r\tAD miss matched for compare %d",iter_tbl0);
                            return NLMERR_FAIL;
                        }
                    }
                    else 
                    {
                        printf(" response type NLMDEV_INDEX_AND_NO_AD");
                        return NLMERR_FAIL;
                    }
                    priority++;
                    if(priority == 2)
                    {
                        i++;
                        priority = 0;
                    }
                }
            } /* tbl0 loop */

    NlmCmFile__printf("\n\t   Total number of keys searched [%u]\n", iter);
    return RETURN_STATUS_OK;
}


/* Compare1, 1 tables 80B_0 are searched  */
static int Perform_LTR0_Searches(genericTblMgrRefAppData *refAppData_p
                                 )
{
    tableInfo   *tblInfo0_p;
    tableRecordInfo   *tblRecordInfo0_p;
    NlmGenericTblRecord *tblRecord0_p;
    NlmReasonCode   reason = NLMRSC_REASON_OK;
    nlm_u32 iter_tbl0, numRecs_tbl0, iter;
    nlm_u32 exp_index0 = 0x0ffffffff;
    nlm_u32 search_index0 = 0x0ffffffff;
    nlm_u8  ltr_num;
    nlm_u8 adData[4];
    nlm_u8 i = 1;
    nlm_u8 priority = 0;

    /* Device Manager declarations */
    NlmDevCtxBufferInfo      cb_info;
    NlmDevCmpResult     search_result;
    NlmDevMissHit search_result0 = NLMDEV_HIT;


    /* 1 table 80B_0are searched  */
    tblInfo0_p = &refAppData_p->g_gtmInfo[0].tblInfo;

    numRecs_tbl0 = tblInfo0_p->rec_count;
    tblRecordInfo0_p = tblInfo0_p->tblRecordInfo_p;
    
    ltr_num = 0;
    
    NlmCmFile__printf("\n\tPerforming LTR#%d searches\n", ltr_num);

    iter = 0;
                
    /* MasterKey[79:0] are constructed from table0 records */
    for( iter_tbl0 = 0; iter_tbl0< numRecs_tbl0; iter_tbl0++ )
    {
        exp_index0 = (tblRecordInfo0_p + iter_tbl0)->index;
        tblRecord0_p = &( (tblRecordInfo0_p + iter_tbl0)->record );
        NlmCm__memcpy( &cb_info.m_data[ 0 ], tblRecord0_p->m_data,
                            (tblRecord0_p->m_len / 8) );
        if( i > 9)
        {
            i =0;
        }
        NlmCm__memset(adData,i,4);
        NlmCm__memset(&search_result,0,sizeof(NlmDevCmpResult));

        iter++;
        cb_info.m_datalen = 10;
        cb_info.m_cbStartAddr = 0;
        /* Fire Compare1 instruction now */
#ifndef NLMPLATFORM_BCM
        if( NLMERR_OK != bcm_kbp_dm_cbwcmp1( refAppData_p->devMgr_p, NLMDEV_PORT_0, ltr_num,
                                                &cb_info, &search_result, &reason ) )
#else
        if( NLMERR_OK != kbp_dm_cbwcmp1( refAppData_p->devMgr_p, NLMDEV_PORT_0, ltr_num,
                                                &cb_info, &search_result, &reason ) )
#endif
        {
            NlmCmFile__printf("Compare1 Instruction failed. Exiting...\n");
            printReasonCode( reason );

            return NLMERR_FAIL;
        }
        else
        {
            search_result0 = search_result.m_hitOrMiss[0]; 

            search_index0  = search_result.m_hitIndex[0];
            /* Hit is expected on all the parallel searches */
            if( search_result0 == NLMDEV_MISS)
            {
                NlmCmFile__printf("Got Miss with MasterKey#%u\n", iter);
                NlmCmFile__printf("search_result0 = %s\n", 
                                    search_result0 ? "Hit" : "Miss");
            }
            else
            {
                /* Check whether the indexes returned by search or correct or not? */
                if( exp_index0 != search_index0)
                {
                    NlmCmFile__printf("Search failed with MasterKey#%u\n", iter);
                    NlmCmFile__printf("exp_index0 = %u, search_index0 = %u\n",
                                        exp_index0, search_index0);
                }
            }

            /* checking AD */
            if(search_result.m_respType[0] == NLMDEV_INDEX_AND_32B_AD)
            {
                if(NlmCm__memcmp(adData,search_result.m_AssocData[0],4) != 0)
                {
                    printf("\r\t AD not matched for instruction %d",iter_tbl0);
                    return NLMERR_FAIL;
                }
            }
            else
            {
                printf(" Unexpected AD response type ");
                return NLMERR_FAIL;
            }

            priority++;
            if(priority == 2)
            {
            i++;
            priority = 0;
            }

        }

    } /* tbl0 loop */

        NlmCmFile__printf("\n\t   Total number of keys searched [%u]\n", iter);
        return RETURN_STATUS_OK;
}




nlm_u8 FillRecordPatternFor_80B0_Table( nlm_u8 *data_p, nlm_u8 *mask_p )
{
    /* 0.X.X.X.X__0.0.0.0.0, 0.X.X.X.X__0.0.0.0.1,
     * 0.X.X.X.X__0.0.0.0.2 and so on;
     */
    NlmCm__memset( data_p, 0, 10 );
    NlmCm__memset( mask_p, 0, 10 );

    /* Put table id at the MSB byte */
    data_p[ 0 ] = 0;

    /* Mask off next 4 bytes */
    mask_p++;
    NlmCm__memset( mask_p, 0xFF, 4 );

    /* Byte index within the record from where varying data starts */
    return 6;
}


nlm_u8 FillRecordPatternFor_160B0_Table( nlm_u8 *data_p, nlm_u8 *mask_p )
{
    /* 2.9.9.9.9__10.10.10.10.10__X.X.X.X.X__0.0.0.0.0,
     * 2.9.9.9.9__10.10.10.10.10__X.X.X.X.X__0.0.0.0.1 and so on
     */
    NlmCm__memset( data_p, 0, 20 );
    NlmCm__memset( mask_p, 0, 20 );

    /* Put table id at the MSB byte */
    data_p[ 0 ] = 2;

    NlmCm__memset( (data_p + 1), 9, 4 );
    NlmCm__memset( (data_p + 5), 10, 5 );

    /* Mask off some bytes */
    mask_p += 10;
    NlmCm__memset( mask_p, 0xFF, 5 );

    /* Byte index within the record from where varying data starts */
    return 16;
}


/*
*Will work on 2-port, 2-smt mode 
*contains 1#80b table with 32b AD  and 1#160b table with 64b AD
*80b table configured for SMT-0,PORT-0 on LTR 0
*160b table Configured for SMT-1,PORT1 on LTR 64
*/
nlm_u32 NlmGenericTableManager_smt(genericTblMgrRefAppData *refAppData_p)
{
    nlm_u8 i;
    nlm_u8  iter_tbl;
    nlm_u16 adwidth = 32;

    /* Initialize Generic Table Manager now */
    if( RETURN_STATUS_OK != InitGenericTableManager_smt( refAppData_p, NlmFalse) )
    {
        NlmCmFile__printf("\tGeneric Table Manager Initialization failed. Exiting...\n");

        return RETURN_STATUS_ABORT;
    }

    /* Add records to tables now */
    i=0;
    for( iter_tbl = 0; iter_tbl < 2; iter_tbl++ )
    {
        AddRecordsToTables( refAppData_p, refAppData_p->g_gtmInfo[i].tbl_p, 0, i,adwidth);
        
        /* Add records which will cause shuffles in the device */
        AddRecordsToTables( refAppData_p, refAppData_p->g_gtmInfo[i].tbl_p, 1, i,adwidth );
        i++;
        adwidth += 32;
    }

    /* perform searches */
    Perform_LTR0_Searches( refAppData_p);
    Perform_LTR64_Searches(refAppData_p);

    /* destroying GTM */
    if( RETURN_STATUS_OK != DestroyGenericTableManager_smt( refAppData_p, NLMDEV_BANK_0, NlmFalse ) )
    {
        NlmCmFile__printf("\tGeneric Table Manager Destroy failed. Exiting...\n");

        return RETURN_STATUS_ABORT;
    }

    if( RETURN_STATUS_OK != DestroyGenericTableManager_smt( refAppData_p, NLMDEV_BANK_1, NlmFalse ) )
    {
        NlmCmFile__printf("\tGeneric Table Manager Destroy failed. Exiting...\n");

        return RETURN_STATUS_ABORT;
    }

    return RETURN_STATUS_OK;
}



/*
*will work on 1-port, no-smt mode
*contains only  1#80b table with 32b AD
*80b table is configure for port-0 , no-smt on LTR 0 
*/

nlm_u32 NlmGenericTableManager_nosmt(genericTblMgrRefAppData *refAppData_p)
{
    nlm_u8  iter_tbl;
    nlm_u16 adwidth = 32;

    /* Initialize Generic Table Manager now */
    if( RETURN_STATUS_OK != InitGenericTableManager_nosmt( refAppData_p) )
    {
        NlmCmFile__printf("\tGeneric Table Manager Initialization failed. Exiting...\n");

        return RETURN_STATUS_ABORT;
    }
    /* Add records to tables now */
    for( iter_tbl = 0; iter_tbl < NUM_OF_TABLES; iter_tbl++ )
    {
        AddRecordsToTables( refAppData_p, refAppData_p->g_gtmInfo[NLMDEV_BANK_0].tbl_p, 0, NLMDEV_BANK_0,adwidth);
        
        /* Add records which will cause shuffles in the device */
        AddRecordsToTables( refAppData_p, refAppData_p->g_gtmInfo[NLMDEV_BANK_0].tbl_p, 1, NLMDEV_BANK_0 ,adwidth);
    }

    /* perform searches */
    Perform_LTR0_Searches( refAppData_p);

    /* destroying GTM */
    if( RETURN_STATUS_OK != DestroyGenericTableManager( refAppData_p, NLMDEV_BANK_0 ) )
    {
        NlmCmFile__printf("\tGeneric Table Manager Destroy failed. Exiting...\n");

        return RETURN_STATUS_ABORT;
    }
    
    return RETURN_STATUS_OK;
}


static int Perform_LTR0_RangeSearches(
    genericTblMgrRefAppData *refAppData_p
    )
{
    NlmRange    *range;
    NlmReasonCode   reason = NLMRSC_REASON_OK;
    nlm_u16 iter, num_searches;
    nlm_u32 search_index0 = 0x0ffffffff;
    nlm_u8  ltr_num;

    /* Device Manager declarations */
    NlmDevCtxBufferInfo      cb_info;
    NlmDevCmpResult     search_result;
    NlmDevMissHit search_result0 = NLMDEV_HIT;
    
    ltr_num = NLM_GTM_REFAPP_SMT0_LTR_NUM;

    NlmCmFile__printf("\n\t Performing LTR-%d searches on SMT-0\n", ltr_num);
    NlmCm__printf("\t =================================\n");

    /* One range entry is picked from the range database and all range values
      * within this entry are searched.
      */
    range = &refAppData_p->smt0Ranges[NLM_GTM_REFAPP_SMT0_RANGE_ID];
    NlmCm__printf("\t Using range_start = %d, range_end = %d\n\n", range->m_start, range->m_end);
    
    /* Master Key : 0.1.1.1.1_1.<16b range>_<16b for range expansion> */
    cb_info.m_data[0] = TABLE_ID_80B_0;
    NlmCm__memset( &cb_info.m_data[1], 1, 5 );
    
    for( num_searches = 0,iter = range->m_start; iter <= range->m_end; iter++, num_searches++ )
    {
        /* Range is stitched at [31:16] and [15:0] are reserved for range expansion. */
        WriteBitsInRegs( cb_info.m_data, 31, 16, iter );
        
        NlmCm__memset(&search_result,0,sizeof(NlmDevCmpResult));

        cb_info.m_datalen = 10;
        cb_info.m_cbStartAddr = 0;
        
        /* Fire Compare1 instruction now */
#ifndef NLMPLATFORM_BCM
            if( NLMERR_OK != bcm_kbp_dm_cbwcmp1( refAppData_p->devMgr_p, NLMDEV_PORT_0, ltr_num,
                                                    &cb_info, &search_result, &reason ) )
#else
        if( NLMERR_OK != kbp_dm_cbwcmp1( refAppData_p->devMgr_p, NLMDEV_PORT_0, ltr_num,
                                                    &cb_info, &search_result, &reason ) )
#endif
        {
                NlmCmFile__printf("Compare1 Instruction with LTR-%d failed. Exiting...\n", ltr_num);
            printReasonCode( reason );

                return NLMERR_FAIL;
            }
            else
        {
            search_result0 = search_result.m_hitOrMiss[NLMDEV_PARALLEL_SEARCH_0]; 
            search_index0  = search_result.m_hitIndex[NLMDEV_PARALLEL_SEARCH_0];
                
            /* Hit is expected */
            if( search_result0 == NLMDEV_MISS)
            {
                NlmCmFile__printf("Got Miss with MasterKey#%u\n", iter);
            }
            else
            {
                NlmCm__printf("\t Got hit at index = 0x%x\n", search_index0);
            }
            }
    } /* tbl0 loop */

    NlmCmFile__printf("\n\t Number of keys searches = %u\n", num_searches);
    
    return RETURN_STATUS_OK;
}


/* Perform searches using ranges */
nlm_u32  Perform_LTR64_RangeSearches(
    genericTblMgrRefAppData *refAppData_p
    )
{
    NlmRange    *range;
        NlmReasonCode   reason = NLMRSC_REASON_OK;
    nlm_u16 iter, num_searches;
    nlm_u32 search_index2 = 0x0ffffffff;
    nlm_u8  ltr_num;

    /* Device Manager declarations */
    NlmDevCtxBufferInfo      cb_info;
    NlmDevCmpResult     search_result;
    NlmDevMissHit search_result2 = NLMDEV_HIT;
    
    ltr_num = NLM_GTM_REFAPP_SMT1_LTR_NUM;
    
    NlmCmFile__printf("\n\t Performing LTR-%d searches on SMT-1\n", ltr_num);
    NlmCm__printf("\t ==================================\n");

    /* One range entry is picked from the range database and all range values
      * within this entry are searched.
      */
    range = &refAppData_p->smt1Ranges[NLM_GTM_REFAPP_SMT1_RANGE_ID];
    NlmCm__printf("\t Using range_start = %d, range_end = %d\n\n", range->m_start, range->m_end);
                
    /* Master key : 1.2.2.2.2_2.2.2.2.2_2.2.2.2.2.2_2.<16b range>_<8b for range expansion> */
    cb_info.m_data[0] = TABLE_ID_160B_1;
    NlmCm__memset( &cb_info.m_data[1], 2, 16 );
    
    for( num_searches = 0, iter = range->m_start; iter <= range->m_end; iter++, num_searches++)
    {
        /* WriteBitsInRegs expects 10 bytes only hence [10]. Range is stitched at [23:8]
          * and [7:0] are reserved for range expansion.
          */
        WriteBitsInRegs( &cb_info.m_data[10], 23, 8, iter );
        
        NlmCm__memset(&search_result,0,sizeof(NlmDevCmpResult));

        cb_info.m_datalen = 20;
        cb_info.m_cbStartAddr = 8;
        
        /* Fire Compare1 instruction now */
#ifndef NLMPLATFORM_BCM
            if( NLMERR_OK != bcm_kbp_dm_cbwcmp1( refAppData_p->devMgr_p, NLMDEV_PORT_1, ltr_num,
                                                    &cb_info, &search_result, &reason ) )
#else
        if( NLMERR_OK != kbp_dm_cbwcmp1( refAppData_p->devMgr_p, NLMDEV_PORT_1, ltr_num,
                                                    &cb_info, &search_result, &reason ) )
#endif
        {
                NlmCmFile__printf("Compare1 Instruction failed with LTR-%d. Exiting...\n", ltr_num);
            printReasonCode( reason );

                return NLMERR_FAIL;
            }
            else
        {
            search_result2 = search_result.m_hitOrMiss[NLMDEV_PARALLEL_SEARCH_2];
            search_index2  = search_result.m_hitIndex[NLMDEV_PARALLEL_SEARCH_2];
            
            /* Hit is expected */
            if( search_result2 == NLMDEV_MISS)
            {
                NlmCmFile__printf("\t Got Miss with MasterKey#%u\n", iter);
            }
            else
            {
                NlmCm__printf(" \t Got hit at index = 0x%x\n", search_index2);
            }
            }
    } /* tbl0 loop */

    NlmCmFile__printf("\n\t Number of searches =  %u\n", num_searches);
    
    return RETURN_STATUS_OK;
}


/* Add records to GTM tables with ranges */
nlm_u32  AddRecordsToTables_range(
    genericTblMgrRefAppData *refAppData_p,
    NlmGenericTbl *tbl_p,
    NlmPortNum  portNum,
    nlm_u16 adwidth
    )
{
    NlmGenericTblRecord  record;
    NlmRecordIndex  index;
    NlmRangeDb  *rangeDb =  NULL;
    NlmRangeEncoded     *rangeEncodings;
    NlmReasonCode   reason = NLMRSC_REASON_OK;
    NlmBankNum  bankNum;
    nlm_u32 iter, numRecs, rangeId, offset;
    nlm_u8  data[2 * TABLE_WIDTH_IN_BYTES_80B], mask[2 * TABLE_WIDTH_IN_BYTES_80B];
    nlm_u8 adData[8], end, start;
    nlm_u8  adByte;

    NlmCmFile__printf("\n\t Adding records to table [%d]\n", tbl_p->m_tblId);
    NlmCm__printf("\t ==========================\n");

    NlmCm__memset( &record, 0, sizeof(record) );

    record.m_data = data;
    record.m_mask = mask;

    NlmCm__memset( data, 0, 2 * TABLE_WIDTH_IN_BYTES_80B);
    NlmCm__memset( mask, 0, 2 * TABLE_WIDTH_IN_BYTES_80B);

    /* Just one entry is expanded with range encodings of one range  */
    
    if(portNum == NLMDEV_PORT_0) /* tbl-0 */
    {
        rangeDb = refAppData_p->smt0Db_p;
        rangeId  = NLM_GTM_REFAPP_SMT0_RANGE_ID;
        
        bankNum = NLMDEV_BANK_0;

        /* 0.1.1.1.1_1.<32b range encodings> */
        data[0] = TABLE_ID_80B_0;
        NlmCm__memset( &data[1], 1, 5 );

        record.m_len = TABLE_WIDTH_80B_0;
        offset = 0;
        end = 31;
        start = 0;
    }
    else /* tbl-1 */
    {
        rangeDb = refAppData_p->smt1Db_p;
        rangeId  = NLM_GTM_REFAPP_SMT1_RANGE_ID;
        
        bankNum = NLMDEV_BANK_1;

        /* 1.2.2.2.2_2.2.2.2.2_2.2.2.2.2.2_2_<24b range encodings> */
        data[0] = TABLE_ID_160B_1;
        NlmCm__memset( &data[1], 2, 16 );

        offset = 10;
        end = 23;
        start = 0;
        record.m_len = TABLE_WIDTH_160B_1;
    }
#ifndef NLMPLATFORM_BCM    
    if( NULL == ( rangeEncodings = bcm_kbp_rm_get_range_encoding( refAppData_p->rangeMgr_p[bankNum],
                                             rangeDb, rangeId, &reason ) ) )
#else
    if( NULL == ( rangeEncodings = kbp_rm_get_range_encoding( refAppData_p->rangeMgr_p[bankNum],
                                             rangeDb, rangeId, &reason ) ) )
#endif
    {
        NlmCmFile__printf("Could not get encodings for rangeId, SMT-%d = %u. Exiting...\n", bankNum, rangeId);
        printReasonCode( reason );

            return NLMERR_FAIL;
    }

    numRecs = rangeEncodings->m_num_entries;
    NlmCm__printf("\t Expanding entry with [%d] range encodings\n\n", numRecs);
    
    for( iter = 0, adByte = 0; iter < numRecs; iter++, adByte++ )
    {
        /* Stitch Range entries */
        WriteBitsInRegs( (data + offset), end, start, ((rangeEncodings->m_entries_p)+ iter)->m_data );
        WriteBitsInRegs( (mask + offset), end, start, ((rangeEncodings->m_entries_p) + iter)->m_mask );

        NlmCm__memset( adData, adByte, adwidth/8);

        /* Add record now */
#ifndef NLMPLATFORM_BCM
        if( NLMERR_OK != bcm_kbp_gtm_add_record( tbl_p,
                                                      portNum,
                                                      &record,
                                                       adData,
                                                      0, /* group id */
                                                      0, /* priority */
                                                      &index,
                                                      &reason ) )
#else
        if( NLMERR_OK != kbp_gtm_add_record( tbl_p,
                                                      portNum,
                                                      &record,
                                                       adData,
                                                      0, /* group id */
                                                      0, /* priority */
                                                      &index,
                                                      &reason ) )
#endif
        {
            NlmCmFile__printf("\ttable [%d] insertions: could not add record#%d\n",
                                tbl_p->m_tblId, iter); 
            printReasonCode( reason );

            return NLMERR_FAIL;
        }
        NlmCm__printf("\t Record-%d added with index [0x%x]\n", iter, index);
        
    } /* end of for loop */

    return RETURN_STATUS_OK;
}



/* This function performs Range Manager related Inits */
nlm_32  InitRangeManager( genericTblMgrRefAppData *refAppData_p )
{
    NlmReasonCode   reason;
    NlmDevShadowDevice *shadowDev_p = ((NlmDevShadowDevice*)refAppData_p->dev_p->m_shadowDevice_p);

    /* {Range ID, Start Range, End Range, Pointer to encoded bitmaps} */
    NlmRange              smt0_ranges[ NUM_OF_SMT0_DB_RANGES] = 
                                               {
                                                 { 0, 1, 5, NULL        },
                                                 { 1, 4, 14, NULL         },
                                                 { 2, 10, 25, NULL        },
                                                 { 3, 1, 5, NULL          },
                                                 { 4, 500, 510, NULL      },
                                                 { 5, 200, 210, NULL      },
                                                 { 6, 150, 250, NULL      },
                                                 { 7, 1024, 1030, NULL    },
                                                 { 8, 505, 510, NULL      },
                                                 { 9, 1000, 1050, NULL    },
                                                 { 10, 10000, 10005, NULL },
                                                 { 11, 5000, 6025, NULL   }
                                               };

    NlmRange              smt1_ranges[NUM_OF_SMT1_DB_RANGES] = 
                                               {
                                                 { 0, 1, 5, NULL          },
                                                 { 1, 3, 13, NULL         },
                                                 { 2, 9, 24, NULL         },
                                                 { 3, 1, 5, NULL          },
                                                 { 4, 515, 525, NULL      },
                                                 { 5, 200, 235, NULL      },
                                                 { 6, 150, 250, NULL      },
                                                 { 7, 1050, 1070, NULL    },
                                                 { 8, 500, 510, NULL      },
                                                 { 9, 1000, 1005, NULL    },
                                                 { 10, 10000, 10008, NULL },
                                                 { 11, 2500, 3200, NULL   }
                                               };
    NlmRangeEncodingType encodingType[4] = { NLM_RANGE_3B_ENCODING,
                                             NLM_RANGE_3B_ENCODING,
                                             NLM_RANGE_3B_ENCODING,
                                             NLM_RANGE_3B_ENCODING
                                            };

    nlm_u32  iter;
    nlm_u8  db_id;
    NlmPortNum   portNum = NLMDEV_PORT_0;
    NlmBankNum  bankNum = NLMDEV_BANK_0;
    NlmRangeDbAttrSet smt0RangeDbAttrs;
    NlmRangeDbAttrSet smt1RangeDbAttrs;
    NlmRangeSrchAttrs rangeAttrs, *rangeAttrs_p;
    nlm_u8 keyNum;

    rangeAttrs_p = &rangeAttrs;
        
    /* Enable Range functionality by enabling Range Engine Enable Bit of Device Config Register. */
    shadowDev_p->m_global->m_devConfig.m_rangeEnable = NLMDEV_ENABLE;
#ifndef NLMPLATFORM_BCM
    if(NLMERR_OK != bcm_kbp_dm_global_reg_write(refAppData_p->dev_p,
                                                  portNum,
                                                  NLMDEV_DEVICE_CONFIG_REG,
                                                  &(shadowDev_p->m_global->m_devConfig),
                                                  &reason))
#else
    if(NLMERR_OK != kbp_dm_global_reg_write(refAppData_p->dev_p,
                                                  portNum,
                                                  NLMDEV_DEVICE_CONFIG_REG,
                                                  &(shadowDev_p->m_global->m_devConfig),
                                                  &reason))
#endif
    {
        NlmCmFile__printf("\tCould not Enable Range Engine...\n" );
        printReasonCode( reason );
        return RETURN_STATUS_ABORT;
    }
    NlmCm__printf("\n\t Range Engine enabled.\n");
    
    /* Initialize the Range Manager. One instance per bank/smt. 
      * Instance-0 is created on Port-0/Bank-0 AND instance-1 is created on
      * Port-1/Bank-1
      */
    for(portNum = NLMDEV_PORT_0, bankNum = NLMDEV_BANK_0; bankNum <= NLMDEV_BANK_1;
        portNum++, bankNum++)
    {
#ifndef NLMPLATFORM_BCM
        if( NULL == ( refAppData_p->rangeMgr_p[bankNum] = bcm_kbp_rm_init( refAppData_p->alloc_p,
                                                                refAppData_p->devMgr_p,
                                                                NLM_DEVTYPE_3,
                                                                bankNum,
                                                                portNum,
                                                                encodingType,
                                                                &reason
                                                              ) ) )
#else
        if( NULL == ( refAppData_p->rangeMgr_p[bankNum] = kbp_rm_init( refAppData_p->alloc_p,
                                                                refAppData_p->devMgr_p,
                                                                NLM_DEVTYPE_3,
                                                                bankNum,
                                                                portNum,
                                                                encodingType,
                                                                &reason
                                                              ) ) )
#endif
        {
            NlmCmFile__printf("\tCould not initialize Range Manager on Port-%d, Bank-%d \n", portNum, bankNum );
            printReasonCode( reason );

            return RETURN_STATUS_ABORT;
        }
        NlmCmFile__printf("\t Range Manager Initialized Successfully on Port-%d, Bank-%d \n", portNum, bankNum);
    }
    NlmCm__printf("\n");

    /* Now create Range databases. 
      * One database on SMT-0 and another on SMT-1.
      *
     *  Range Database has three attributes :
     * (1) number of bits available for encoding (2) Number of valid bits of range (3) Encoding type
     * Number of bits available for Range Manager for encoding is given by m_num_bits . 
     * Number of valid range bits specifies how many (LSB)  bits of the 16b range field are valid;  
     * Encoding type specifies which range encoding to use
     *
     * First database has 32 bits for range expansion and 16 bits are valid in the range in the search key
     * Second database has 24 bits for range expansion and 16 bits are valid in the range in the search key
     */
    smt0RangeDbAttrs.m_num_bits = 32;
    smt0RangeDbAttrs.m_valid_bits = 16;
       smt0RangeDbAttrs.m_encodingType = NLM_RANGE_3B_ENCODING;

    db_id = 0; /* used for database id */
#ifndef NLMPLATFORM_BCM
    if( NULL == ( refAppData_p->smt0Db_p = bcm_kbp_rm_create_db( refAppData_p->rangeMgr_p[NLMDEV_BANK_0],
                                                                 db_id,
                                                                 &smt0RangeDbAttrs,
                                                                 &reason
                                                               ) ) )
#else
    if( NULL == ( refAppData_p->smt0Db_p = kbp_rm_create_db( refAppData_p->rangeMgr_p[NLMDEV_BANK_0],
                                                                 db_id,
                                                                 &smt0RangeDbAttrs,
                                                                 &reason
                                                               ) ) )
#endif
    {
        NlmCmFile__printf("Could not create dabase-%d on SMT-0. Exiting...\n", db_id);
        printReasonCode( reason );

        return RETURN_STATUS_ABORT;
    }
    NlmCmFile__printf("\t SMT-0 Range Database [Database ID = %d] Created\n", db_id);

    /* Number of bits available for encoding is 24. No MCOR; only DIRPE encoding. */
    smt1RangeDbAttrs.m_num_bits = 24;
    smt1RangeDbAttrs.m_valid_bits = 16;
       smt1RangeDbAttrs.m_encodingType = NLM_RANGE_3B_ENCODING;
    db_id++;
#ifndef NLMPLATFORM_BCM
    if( NULL == ( refAppData_p->smt1Db_p = bcm_kbp_rm_create_db( refAppData_p->rangeMgr_p[NLMDEV_BANK_1],
                                                                 db_id,
                                                                 &smt1RangeDbAttrs,
                                                                 &reason
                                                               ) ) )
#else
    if( NULL == ( refAppData_p->smt1Db_p = kbp_rm_create_db( refAppData_p->rangeMgr_p[NLMDEV_BANK_1],
                                                                 db_id,
                                                                 &smt1RangeDbAttrs,
                                                                 &reason
                                                               ) ) )
#endif
    {
        NlmCmFile__printf("Could not create dabase-%d on SMT-1. Exiting...\n", db_id);
        printReasonCode( reason );

        return RETURN_STATUS_ABORT;
    }
    NlmCmFile__printf("\t SMT-1 Range Database [Database ID = %d] Created \n\n", db_id);

    /* Hardware has 4 range blocks per SMT/Bank. 
      * Only SMT-0 database uses MCOR.  SMT-1 database does not use MCOR. 
      * Range database association to hardware range block is needed only for
      * MCORs.
     */
#ifndef NLMPLATFORM_BCM
    if( NLMERR_OK != bcm_kbp_rm_assign_range( refAppData_p->rangeMgr_p[NLMDEV_BANK_0],
                                               refAppData_p->smt0Db_p,
                                               NLM_RANGE_TYPE_A,
                                               &reason ) )
#else
    if( NLMERR_OK != kbp_rm_assign_range( refAppData_p->rangeMgr_p[NLMDEV_BANK_0],
                                               refAppData_p->smt0Db_p,
                                               NLM_RANGE_TYPE_A,
                                               &reason ) )
#endif
    {
        NlmCmFile__printf("SMT-0 database Range Assignment failed...\n");
        printReasonCode( reason );

        return RETURN_STATUS_FAIL;
    }
    NlmCmFile__printf("\t SMT-0 Database associated with Range-A H/W Block \n\n");
    
    /* Add ranges to the databases. First in SMT-0 db followed by SMT-1 db */
    for( iter = 0; iter < NUM_OF_SMT0_DB_RANGES; iter++ )
    {
        refAppData_p->smt0Ranges[ iter ] = smt0_ranges[ iter ];
#ifndef NLMPLATFORM_BCM
        if( NLMERR_OK != bcm_kbp_rm_add_range( refAppData_p->rangeMgr_p[NLMDEV_BANK_0],
                                                refAppData_p->smt0Db_p,
                                                &refAppData_p->smt0Ranges[ iter ],
                                                &reason ) )
#else
        if( NLMERR_OK != kbp_rm_add_range( refAppData_p->rangeMgr_p[NLMDEV_BANK_0],
                                                refAppData_p->smt0Db_p,
                                                &refAppData_p->smt0Ranges[ iter ],
                                                &reason ) )
#endif
        {
            NlmCmFile__printf("Could not add range-%u to SMT-0 range database...\n", iter);
            printReasonCode( reason );

            return RETURN_STATUS_FAIL;
        }
    }
    NlmCmFile__printf("\t [%d] ranges added to SMT-0 Range Database\n", iter);

    for( iter = 0; iter < NUM_OF_SMT1_DB_RANGES; iter++ )
    {
        refAppData_p->smt1Ranges[ iter ] = smt1_ranges[ iter ];
#ifndef NLMPLATFORM_BCM
        if( NLMERR_OK != bcm_kbp_rm_add_range( refAppData_p->rangeMgr_p[NLMDEV_BANK_1],
                                                refAppData_p->smt1Db_p,
                                                &refAppData_p->smt1Ranges[ iter ],
                                                &reason ) )
#else
        if( NLMERR_OK != kbp_rm_add_range( refAppData_p->rangeMgr_p[NLMDEV_BANK_1],
                                                refAppData_p->smt1Db_p,
                                                &refAppData_p->smt1Ranges[ iter ],
                                                &reason ) )
#endif
        {
            NlmCmFile__printf("Could not add range-%u to SMT-0 range database...\n", iter);
            printReasonCode( reason );

            return RETURN_STATUS_FAIL;
        }
    }
    NlmCmFile__printf("\t [%d] ranges added to SMT-1 Range Database \n\n", iter);

    /* Let Range Manager do range compression now */
#ifndef NLMPLATFORM_BCM
    if( NLMERR_OK != bcm_kbp_rm_create_encodings( refAppData_p->rangeMgr_p[NLMDEV_BANK_0],
                                                   refAppData_p->smt0Db_p,
                                                   &reason ) )
#else
    if( NLMERR_OK != kbp_rm_create_encodings( refAppData_p->rangeMgr_p[NLMDEV_BANK_0],
                                                   refAppData_p->smt0Db_p,
                                                   &reason ) )
#endif
    {
        NlmCmFile__printf("CreateEncodings failed on source port database...\n");
        printReasonCode( reason );

        return RETURN_STATUS_FAIL;
    }
    NlmCmFile__printf("\t CreateEncodings of SMT-0 Range database successful \n");
#ifndef NLMPLATFORM_BCM
    if( NLMERR_OK != bcm_kbp_rm_create_encodings( refAppData_p->rangeMgr_p[NLMDEV_BANK_1],
                                                   refAppData_p->smt1Db_p,
                                                   &reason ) )
#else
    if( NLMERR_OK != kbp_rm_create_encodings( refAppData_p->rangeMgr_p[NLMDEV_BANK_1],
                                                   refAppData_p->smt1Db_p,
                                                   &reason ) )
#endif
    {
        NlmCmFile__printf("CreateEncodings failed on destination port database...\n");
        printReasonCode( reason );

        return RETURN_STATUS_FAIL;
    }
    NlmCmFile__printf("\t CreateEncodings of SMT-1 Range database successful \n\n");

    /* Valid search data in SMT-0 master key is 80b. 16b range value is present at byte position 2
      * in the master key. Byte positions 0 and 1 are reserved for range expansion in the
      * generated key. Key-0 is used for the search.
      */
    rangeAttrs_p->m_extraction_startByte_rangeA = 2;
    rangeAttrs_p->m_rangeA_db = refAppData_p->smt0Db_p;
            
    /* Putting the initial insert values as DO NOT INSERT */
    for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
    {
        rangeAttrs_p->m_keyInsert_startByte_rangeA[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
        rangeAttrs_p->m_keyInsert_startByte_rangeB[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
        rangeAttrs_p->m_keyInsert_startByte_rangeC[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
        rangeAttrs_p->m_keyInsert_startByte_rangeD[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
    }
    
    /* Bits [31:0] of Key 0 will have Range A encodings */
    rangeAttrs_p->m_keyInsert_startByte_rangeA[NLMDEV_KEY_0] = 0;

#ifndef NLMPLATFORM_BCM
    if( NLMERR_OK != bcm_kbp_rm_config_range_matching( refAppData_p->rangeMgr_p[NLMDEV_BANK_0],
                                                        NLM_GTM_REFAPP_SMT0_LTR_NUM,
                                                        rangeAttrs_p,               
                                                        &reason ) )
#else
    if( NLMERR_OK != kbp_rm_config_range_matching( refAppData_p->rangeMgr_p[NLMDEV_BANK_0],
                                                        NLM_GTM_REFAPP_SMT0_LTR_NUM,
                                                        rangeAttrs_p,               
                                                        &reason ) )
#endif

    {
        NlmCmFile__printf("\tLTR#%d ConfigRangeMatching failed...\n", NLM_GTM_REFAPP_SMT0_LTR_NUM);
        printReasonCode( reason );
        
        return RETURN_STATUS_FAIL;
    }
    NlmCm__printf("\t LTR-%d ConfigRangeMatching successful on SMT-0 \n", NLM_GTM_REFAPP_SMT0_LTR_NUM);

    /* Valid search data in SMT-1 master key is 160b. 16b range value is present at byte position 1
      * in the master key. Byte positions 0 is reserved for range expansion in the
      * generated key. Key-2 is used for the search.
      */
    rangeAttrs_p->m_extraction_startByte_rangeA = 1;
    rangeAttrs_p->m_rangeA_db = refAppData_p->smt1Db_p;
            
    /* Putting the initial insert values as DO NOT INSERT */
    for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
    {
        rangeAttrs_p->m_keyInsert_startByte_rangeA[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
        rangeAttrs_p->m_keyInsert_startByte_rangeB[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
        rangeAttrs_p->m_keyInsert_startByte_rangeC[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
        rangeAttrs_p->m_keyInsert_startByte_rangeD[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
    }
    
    /* Bits [23:0] of Key 2 will have Range A encodings */
    rangeAttrs_p->m_keyInsert_startByte_rangeA[NLMDEV_KEY_2] = 0;
   
#ifndef NLMPLATFORM_BCM
    if( NLMERR_OK != bcm_kbp_rm_config_range_matching( refAppData_p->rangeMgr_p[NLMDEV_BANK_1],
                                                        NLM_GTM_REFAPP_SMT1_LTR_NUM,
                                                        rangeAttrs_p,
                                                        &reason ) )
#else
    if( NLMERR_OK != kbp_rm_config_range_matching( refAppData_p->rangeMgr_p[NLMDEV_BANK_1],
                                                        NLM_GTM_REFAPP_SMT1_LTR_NUM,
                                                        rangeAttrs_p,
                                                        &reason ) )
#endif
    {
        NlmCmFile__printf("\tLTR#%d ConfigRangeMatching failed...\n", NLM_GTM_REFAPP_SMT1_LTR_NUM);
        printReasonCode( reason );
        
        return RETURN_STATUS_FAIL;
    }
    NlmCm__printf("\t LTR-%d ConfigRangeMatching successful on SMT-1 \n", NLM_GTM_REFAPP_SMT1_LTR_NUM);
    
    return RETURN_STATUS_OK;
}


/* Destroy Range Manager data structures  */
nlm_u32  DestroyRangeMgr( genericTblMgrRefAppData *refAppData_p )
{
    NlmReasonCode   reason;

    NlmCmFile__printf("\n");
    
    /* Destroy range databases first and then the manager */
    NlmCmFile__printf("\t Destroying SMT-0 Range database\n");
#ifndef NLMPLATFORM_BCM
    bcm_kbp_rm_destroy_db( refAppData_p->rangeMgr_p[NLMDEV_BANK_0], 
                                    refAppData_p->smt0Db_p, &reason );
#else
    kbp_rm_destroy_db( refAppData_p->rangeMgr_p[NLMDEV_BANK_0], 
                                    refAppData_p->smt0Db_p, &reason );
#endif

    NlmCmFile__printf("\t Destroying SMT-1 Range database\n");
#ifndef NLMPLATFORM_BCM
    bcm_kbp_rm_destroy_db( refAppData_p->rangeMgr_p[NLMDEV_BANK_1], 
                                    refAppData_p->smt1Db_p, &reason );
#else
    kbp_rm_destroy_db( refAppData_p->rangeMgr_p[NLMDEV_BANK_1], 
                                    refAppData_p->smt1Db_p, &reason );
#endif
                                    
    NlmCmFile__printf("\t Destroying SMT-0 Range Manager instance\n");
#ifndef NLMPLATFORM_BCM
    bcm_kbp_rm_destroy( refAppData_p->rangeMgr_p[NLMDEV_BANK_0], &reason );
#else
    kbp_rm_destroy( refAppData_p->rangeMgr_p[NLMDEV_BANK_0], &reason );
#endif

    NlmCmFile__printf("\t Destroying SMT-1 Range Manager instance\n");
#ifndef NLMPLATFORM_BCM
    bcm_kbp_rm_destroy( refAppData_p->rangeMgr_p[NLMDEV_BANK_1], &reason );
#else
    kbp_rm_destroy( refAppData_p->rangeMgr_p[NLMDEV_BANK_1], &reason );
#endif

    return RETURN_STATUS_OK;
}


/* Top level main function to run with Ranges. */
nlm_u32 NlmGenericTableManager_range(genericTblMgrRefAppData *refAppData_p)

{
    nlm_u16 adwidth = 32;

    /* Initialize Range Manager now */
    if( RETURN_STATUS_OK != InitRangeManager( refAppData_p) )
    {
        NlmCmFile__printf("\tRange Manager Initialization failed. Exiting...\n");

        return RETURN_STATUS_ABORT;
    }

    /* Initialize Generic Table Manager now */
    if( RETURN_STATUS_OK != InitGenericTableManager_smt( refAppData_p, NlmTrue) )
    {
        NlmCmFile__printf("\tGeneric Table Manager Initialization failed. Exiting...\n");

        return RETURN_STATUS_ABORT;
    }

    /* Add records to tbl-0 followed by tbl-1. tbl-0 records are added from port-0 and
      * tbl-1 records are added from port-1
      */
    AddRecordsToTables_range( refAppData_p, refAppData_p->g_gtmInfo[0].tbl_p, NLMDEV_PORT_0, adwidth);
    AddRecordsToTables_range( refAppData_p, refAppData_p->g_gtmInfo[1].tbl_p, NLMDEV_PORT_1, (adwidth + 32) );
    
    /* perform searches */
    Perform_LTR0_RangeSearches( refAppData_p);
    Perform_LTR64_RangeSearches(refAppData_p);

    /* Destroy Range Manager data structrures */
    DestroyRangeMgr( refAppData_p );

    /* destroying GTM */
    if( RETURN_STATUS_OK != DestroyGenericTableManager_smt( refAppData_p, NLMDEV_BANK_0, NlmTrue ) )
    {
        NlmCmFile__printf("\tGeneric Table Manager Destroy failed. Exiting...\n");

        return RETURN_STATUS_ABORT;
    }

    if( RETURN_STATUS_OK != DestroyGenericTableManager_smt( refAppData_p, NLMDEV_BANK_1, NlmTrue ) )
    {
        NlmCmFile__printf("\tGeneric Table Manager Destroy failed. Exiting...\n");

        return RETURN_STATUS_ABORT;
    }

    return RETURN_STATUS_OK;
}


int main(int argc, char *argv[])
{
    genericTblMgrRefAppData  refAppData;
    nlm_32  break_alloc_id = -1;
    nlm_u8 cmdcount = 1;
    nlm_u8 idx = 1;
    NlmBool isSmtMode =  NlmFalse;
    nlm_u32 errNum = RETURN_STATUS_OK;
    NlmBool isRangeEnabled = NlmFalse;

#ifdef NLMPLATFORM_BCM  
    if(0 != sal_console_init())
        return NLMERR_FAIL;

#endif

#ifdef NLM_NETOS
    /* Default no-smt mode, set isSmtMode to run in SMT mode
       to run with rangem set isRangeEnabled (isSmtMode also set) */

    /*
        isSmtMode =  NlmTrue;
        isRangeEnabled = NlmFalse;

        if(isRangeEnabled == NlmTrue)
            isSmtMode =  NlmTrue;
    */
    (void)idx;
    (void)cmdcount;
#else
    if(argc > cmdcount)
    {
        if( NlmCm__strcasecmp("-smt", argv[idx]) == 0 )
        {
            isSmtMode = NlmTrue;
        }
        else if( NlmCm__strcasecmp("-range", argv[idx]) == 0 )
        {
            /* Range is enabled only in SMT mode:
               enabling SMT mode along with range support */
            isSmtMode      = NlmTrue;
            isRangeEnabled = NlmTrue;
        }
        cmdcount++;
    }
#endif

    NlmCmDebug__Setup( break_alloc_id, NLMCM_DBG_EBM_ENABLE );

    NlmCmFile__printf ("\n\tGeneric Table Manager Application Reference Code Using\n");
    NlmCmFile__printf ("\tGeneric Table Manager Module. \n\n\n");


    /* Initialize customer specific entities: Memory Allocator, XPT Interface */
    if( RETURN_STATUS_OK != InitEnvironment( &refAppData ) )
    {
        NlmCmFile__printf("\tEnvironment Initialization failed. Exiting...\n");

        return RETURN_STATUS_ABORT;
    }

    /* bank mode and bank type */
    if(isSmtMode)
    {
        refAppData.smtMode  = NLMDEV_DUAL_SMT_MODE;
        refAppData.portMode = NLMDEV_DUAL_PORT;
        refAppData.g_gtmInfo[0].bankNum = NLMDEV_BANK_0;
        refAppData.g_gtmInfo[1].bankNum = NLMDEV_BANK_1;
        if(isRangeEnabled == NlmFalse)
            NlmCmFile__printf("\tDevice working in 2-PORT/2-SMT MODE...\n");
        else
            NlmCmFile__printf("\tDevice working in 2-PORT/2-SMT MODE with Range...\n");
    }
    else
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

    if(isSmtMode)
    {
        if(isRangeEnabled == NlmFalse)
            errNum = NlmGenericTableManager_smt( &refAppData );
        else
            errNum = NlmGenericTableManager_range( &refAppData );
    }
    else
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

