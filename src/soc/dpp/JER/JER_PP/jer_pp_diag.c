/* $Id: arad_pp_diag.c,v 1.93 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_DIAG

/*************
 * INCLUDES  *
 *************/
/* { */

#include <shared/bsl.h>


#include <soc/dcmn/error.h>
#include <soc/dcmn/utils.h>

#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_diag.h>
#include <soc/dpp/PPC/ppc_api_diag.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* } */
/*************
 * MACROS    *
 *************/
/* { */

/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

soc_error_t
soc_jer_diag_glem_signals_get(int unit, int core_id, soc_ppc_diag_glem_signals_t *result){
    soc_error_t rv;
    ARAD_PP_DIAG_REG_FIELD fld;
    uint32 regs_val[ARAD_PP_DIAG_DBG_VAL_LEN], 
        tmp, tmp_bits;
    soc_ppc_diag_glem_outlif_t *first_outlif, *second_outlif;
    
    
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(result);
    SOCDNX_CLEAR(result, soc_ppc_diag_glem_signals_t, 1);
    first_outlif = &result->outlifs[0];
    second_outlif = &result->outlifs[1];

    /*
     * Step one: Get the global lif source. It could be either from 
     *  multicast or from FTMH/EEI.
     */

    /* This bit indicates multicast. */
    /* dpp_dsig_read(unit, core_id, "ETPP", NULL, NULL, "TM_Action_is_MC", tmp, 2); */
    SOCDNX_DIAG_FLD_GET(ARAD_EPNI_ID, core_id, 2, 1, 15, 15, tmp);

    if (tmp) {
        /* Last lookup was multicast. Read the CUDs.*/

        /* 1. Outlif from Cud1 */
        /* dpp_dsig_read(unit, core_id, "ERPP", NULL, NULL, "CUD", first_outlif->global_outlif, 2); */
        SOCDNX_DIAG_FLD_GET(ARAD_EGQ_ID, core_id, 4, 3, 50, 33, tmp);
        first_outlif->global_outlif = tmp;
        first_outlif->source = soc_ppc_diag_glem_outlif_source_cud1;

        /* 2. Outlif from Cud2 */
        /* dpp_dsig_read(unit, core_id, "ERPP", NULL, NULL, "New_CUD", second_outlif->global_outlif, 2); */
        SOCDNX_DIAG_FLD_GET(ARAD_EGQ_ID, core_id, 4, 3, 32, 15, tmp);
        second_outlif->global_outlif = tmp;
        second_outlif->source = soc_ppc_diag_glem_outlif_source_cud2;
    } else {
        /* Last lookup was not multicast. Look for outlif/eei. */

        /* 
         * 1. Outlif from FTMH outlif.
         *      This signal is split into two signals. 
         *      These are the lsbs:
         */
        /* dpp_dsig_read(unit, core_id, "ETPP", NULL, NULL, "MC_ID_or_Out_LIF", first_outlif->global_outlif, 1); */
        SOCDNX_DIAG_FLD_GET(ARAD_EPNI_ID, core_id, 2, 0, 255, 252, tmp);
        first_outlif->global_outlif |= tmp;

        /* These are the MSBs: */
        SOCDNX_DIAG_FLD_GET(ARAD_EPNI_ID, core_id, 2, 1, 14, 0, tmp);
        first_outlif->global_outlif |= tmp << 4;

        first_outlif->source = soc_ppc_diag_glem_outlif_source_ftmh;

        /* 
         * 2. Outlif from EEI
         */
        /* dpp_dsig_read(unit, core_id, "ETPP", NULL, NULL, "PPH_EEI_Extension", tmp, 1); */
        SOCDNX_DIAG_FLD_GET(ARAD_EPNI_ID, core_id, 2, 0, 63, 40, tmp);

        /* EEI doesn't necessarily represent outlif. Check the encoding. */
        rv = soc_sand_bitstream_get_any_field(&tmp,22,2,&tmp_bits);
        SOCDNX_SAND_IF_ERR_EXIT(rv);
        if (tmp_bits == 3) {
            /* EEI represent outlif.*/ 

            /* 
             *  EEI outlif is also split into MSBs and LSBs.
             *  LSBs:
             */
            rv = soc_sand_bitstream_get_any_field(&tmp,20,2,&tmp_bits);
            SOCDNX_SAND_IF_ERR_EXIT(rv);
            second_outlif->global_outlif |= tmp_bits << 16;

            /* MSBs: */
            rv = soc_sand_bitstream_get_any_field(&tmp,0,16,&tmp_bits);
            SOCDNX_SAND_IF_ERR_EXIT(rv);
            second_outlif->global_outlif |= tmp_bits;

            second_outlif->source = soc_ppc_diag_glem_outlif_source_eei;
        }
    }

    /*
     * Step two: Get the lookup results. 
     *  This register holds all the results: 
     */
    /* dpp_dsig_read(unit, core_id, "ETPP", NULL, NULL, "GLEM_Data", regs_val, 2); */
    SOCDNX_DIAG_FLD_READ(&fld, core_id, ARAD_EPNI_ID, 0, 0, 59, 19);
    

    /* First outlif results: */
    if (first_outlif->global_outlif) {
        rv = soc_sand_bitstream_get_any_field(regs_val, 18, 1, &tmp_bits);
        SOCDNX_SAND_IF_ERR_EXIT(rv);
        first_outlif->found = tmp_bits;

        if (first_outlif->found) {
            rv = soc_sand_bitstream_get_any_field(regs_val, 0, 18, &tmp_bits);
            SOCDNX_SAND_IF_ERR_EXIT(rv);
            first_outlif->local_outlif = tmp_bits;
        }

    }


    /* Second outlif results: */
    if (second_outlif->global_outlif) {
        rv = soc_sand_bitstream_get_any_field(regs_val, 38, 1, &tmp_bits);
        SOCDNX_SAND_IF_ERR_EXIT(rv);
        second_outlif->found = tmp_bits;

        if (second_outlif->found) {
            rv = soc_sand_bitstream_get_any_field(regs_val, 20, 18, &tmp_bits);
            SOCDNX_SAND_IF_ERR_EXIT(rv);
            second_outlif->local_outlif = tmp_bits;
        }
    }

exit:
  SOCDNX_FUNC_RETURN;
}

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>
