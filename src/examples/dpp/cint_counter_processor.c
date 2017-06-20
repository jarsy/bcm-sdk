/*~~~~~~~~~~~~~~~~~~~~~~~~~~Cosq: VOQ counter processor~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
 *
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        cint_counter_processor.c
 * Purpose:     Example of configuring counter processor statistics gatheting using dynamic APIs.
 * 
 * *
 * The settings include:
 *  - Configuring engine's counting source, and counting formats.
 *  - Configuring LIF counting ranges.
 *
 */

int lif_counting_set(int unit,                                           /*in*/ 
                     bcm_stat_counter_source_type_t source,              /*in*/ 
                     int command_id,                                     /*in*/ 
                     bcm_stat_counter_lif_range_id_t lif_counting_mask,  /*in*/ 
                     bcm_stat_counter_lif_stack_id_t lif_stack_to_map,   /*in*/ 
                     bcm_stat_counter_lif_stack_id_t lif_stack_to_count  /*in*/ 
                ) {
    bcm_stat_counter_lif_range_id_t 
        lif_ranges[BCM_STAT_COUNT_LIF_NUMBER_OF_STACK_IDS] = {
            bcmStatCounterLifRangeIdLifInvalid, 
            bcmStatCounterLifRangeIdNotInAny, 
            bcmBcmStatCounterLifRangeId0, 
            bcmBcmStatCounterLifRangeId1};
    bcm_error_t rv = BCM_E_NONE;
    uint32 flags = 0;
    int bitmap = 0;
    int match;
    int lif_0 = 0,
        lif_1 = 0,
        lif_2 = 0,
        lif_3 = 0;
    bcm_stat_counter_source_t counter_source;
    bcm_stat_counter_lif_mask_t counting_mask;
    counter_source.engine_source = source;
    counter_source.command_id = command_id;
    for (bitmap = 0; bitmap < 0x100 ;bitmap++) {
        lif_0 = bitmap & 0x3;
        lif_1 = (bitmap >> 2) & 0x3;
        lif_2 = (bitmap >> 4) & 0x3;
        lif_3 = (bitmap >> 6) & 0x3;
        counting_mask.lif_counting_mask[0] = lif_ranges[lif_0];
        counting_mask.lif_counting_mask[1] = lif_ranges[lif_1];
        counting_mask.lif_counting_mask[2] = lif_ranges[lif_2];
        counting_mask.lif_counting_mask[3] = lif_ranges[lif_3];

        match = 0;
        if (lif_stack_to_map != bcmStatCounterLifStackIdNone) {
            if (counting_mask.lif_counting_mask[lif_stack_to_map] == lif_counting_mask) {match = 1;}
        } else {
            if (counting_mask.lif_counting_mask[0] == lif_counting_mask) {match = 1;}
            if (counting_mask.lif_counting_mask[1] == lif_counting_mask) {match = 1;}
            if (counting_mask.lif_counting_mask[2] == lif_counting_mask) {match = 1;}
            if (counting_mask.lif_counting_mask[3] == lif_counting_mask) {match = 1;}
        }
        if (match) {
            rv = bcm_stat_counter_lif_counting_set (unit, flags, &counter_source, &counting_mask, lif_stack_to_count);
            if (rv != BCM_E_NONE) {
                printf("bcm_stat_counter_lif_counting_set() failed $rv\n");
                return rv;
            }
        }
    }
    return rv;
}

int lif_counting_get(int unit,                                           /*in*/ 
                     bcm_stat_counter_source_type_t source,              /*in*/ 
                     int command_id,                                     /*in*/ 
                     bcm_stat_counter_lif_range_id_t  lif_counting_mask, /*in*/ 
                     bcm_stat_counter_lif_stack_id_t  lif_stack_to_map,  /*in*/ 
                     bcm_stat_counter_lif_stack_id_t* lif_stack_to_count /*out*/
                ) {
    bcm_stat_counter_lif_range_id_t 
        lif_ranges[BCM_STAT_COUNT_LIF_NUMBER_OF_STACK_IDS] = {
            bcmStatCounterLifRangeIdLifInvalid, 
            bcmStatCounterLifRangeIdNotInAny, 
            bcmBcmStatCounterLifRangeId0, 
            bcmBcmStatCounterLifRangeId1};
    bcm_error_t rv = BCM_E_NONE;
    uint32 flags = 0;
    int bitmap = 0;
    int lif_0 = 0,
        lif_1 = 0,
        lif_2 = 0,
        lif_3 = 0;
    bcm_stat_counter_source_t counter_source;
    bcm_stat_counter_lif_mask_t counting_mask;
    counter_source.engine_source = source;
    counter_source.command_id = command_id;
    bitmap = sal_rand() & 0xff;
    lif_0 = bitmap & 0x3;
    lif_1 = (bitmap >> 2) & 0x3;
    lif_2 = (bitmap >> 4) & 0x3;
    lif_3 = (bitmap >> 6) & 0x3;
    counting_mask.lif_counting_mask[0] = lif_ranges[lif_0];
    counting_mask.lif_counting_mask[1] = lif_ranges[lif_1];
    counting_mask.lif_counting_mask[2] = lif_ranges[lif_2];
    counting_mask.lif_counting_mask[3] = lif_ranges[lif_3];

    
    if (lif_stack_to_map != bcmStatCounterLifStackIdNone) {
        counting_mask.lif_counting_mask[lif_stack_to_map] = lif_counting_mask;
    } else {
        rv = BCM_E_PARAM;
        return rv;
    }
    rv = bcm_stat_counter_lif_counting_get(unit, flags, &counter_source, &counting_mask, lif_stack_to_count);
    if (rv != BCM_E_NONE) {
        printf("bcm_stat_counter_lif_counting_get() failed $rv\n");
        return rv;
    }
    
    return rv;
}

int lif_counting_test(int unit,                                           /*in*/ 
                      bcm_stat_counter_source_type_t source,              /*in*/ 
                      int command_id,                                     /*in*/ 
                      bcm_stat_counter_lif_range_id_t lif_counting_mask,  /*in*/ 
                      bcm_stat_counter_lif_stack_id_t lif_stack_to_map,   /*in*/ 
                      bcm_stat_counter_lif_stack_id_t lif_stack_to_count, /*in*/
                      int nof_iterations                                  /*in*/
                ) {
    bcm_stat_counter_lif_stack_id_t lif_stack_to_count_get;
    bcm_error_t rv = BCM_E_NONE;
    int idx_iteration;
    rv = lif_counting_set(unit, source, command_id, lif_counting_mask, lif_stack_to_map, lif_stack_to_count);
    if (rv != BCM_E_NONE) {
        printf("lif_counting_set() failed $rv\n");
        return rv;
    }

    if (lif_stack_to_map != bcmStatCounterLifStackIdNone) {
        for (idx_iteration = 0; idx_iteration < nof_iterations; idx_iteration++) {
            rv = lif_counting_get(unit, source, command_id, lif_counting_mask, lif_stack_to_map, &lif_stack_to_count_get);
            if (rv != BCM_E_NONE) {
                printf("lif_counting_get() failed $rv\n");
                return rv;
            }
            if (lif_stack_to_count != lif_stack_to_count_get) {
                printf("expected lif_stack_to_count_get to be $d but got %d.\n", lif_stack_to_count, lif_stack_to_count_get);
                return BCM_E_CONFIG;
            }
        }
    }
    return rv;
}

int lif_counting_get_test(int unit,                                                    /*in*/ 
                          bcm_stat_counter_source_type_t source,                       /*in*/ 
                          int command_id,                                              /*in*/ 
                          bcm_stat_counter_lif_stack_id_t expected_lif_stack_to_count, /*in*/
                          int nof_iterations                                           /*in*/)
{
    bcm_error_t rv = BCM_E_NONE;
    uint32 flags = 0;
    int idx_iteration; 
    int lif_mask_iter;
    bcm_stat_counter_source_t counter_source;
    bcm_stat_counter_lif_mask_t counting_mask;

    counter_source.engine_source = source;
    counter_source.command_id = command_id;
    int lif_stack_to_count;
    bcm_stat_counter_lif_range_id_t 
        lif_ranges[BCM_STAT_COUNT_LIF_NUMBER_OF_STACK_IDS] = {
            bcmStatCounterLifRangeIdLifInvalid, 
            bcmStatCounterLifRangeIdNotInAny, 
            bcmBcmStatCounterLifRangeId0, 
            bcmBcmStatCounterLifRangeId1};
    for (idx_iteration = 0; idx_iteration < nof_iterations; idx_iteration++) {
        for (lif_mask_iter = bcmStatCounterLifStackId0; lif_mask_iter < BCM_STAT_COUNT_LIF_NUMBER_OF_STACK_IDS; lif_mask_iter++) {
            if (lif_mask_iter == expected_lif_stack_to_count) {
                counting_mask.lif_counting_mask[lif_mask_iter] = (command_id == 0) ? bcmBcmStatCounterLifRangeId0 : bcmBcmStatCounterLifRangeId1; 
            } else {
                counting_mask.lif_counting_mask[lif_mask_iter] = lif_ranges[sal_rand() & 0x3];
            }
        }
        rv = _bcm_petra_stat_counter_lif_counting_get(unit, flags, &counter_source, &counting_mask, &lif_stack_to_count); 
        if (rv != BCM_E_NONE) {
            printf("bcm_stat_counter_lif_counting_get() failed %d\n", rv);
            return rv;
        }
        if (lif_stack_to_count != expected_lif_stack_to_count) {
            printf("bcm_stat_counter_lif_counting_get() expected %d, got %d\n", expected_lif_stack_to_count, lif_stack_to_count);
            return BCM_E_CONFIG;
        }
    }
    return rv;
}

