/*
 * $Id: fe1600_fabric_links.c,v 1.27 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC FE1600 FABRIC LINKS
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FABRIC
#include <shared/bsl.h>
#include <soc/dcmn/error.h>
#include <soc/defs.h>
#include <soc/error.h>
#include <soc/mcm/memregs.h>
#include <soc/phy/phyctrl.h>

#include <soc/dfe/cmn/dfe_drv.h>
#include <shared/bitop.h>

#if defined(BCM_88750_A0)

#include <soc/dfe/fe1600/fe1600_defs.h>
#include <soc/dfe/fe1600/fe1600_fabric_links.h>
#include <soc/dfe/fe1600/fe1600_port.h>


#define SOC_FE1600_LINKS_IN_BLOCK 32
#define SOC_FE1600_FABRIC_MAX_NOF_TRAFFIC_DISABLE_BMP 5

#define SOC_FE1600_FABRIC_LINK_QTR(link,qtr) \
        if(link<(SOC_FE1600_LINKS_IN_BLOCK)) \
          qtr = 1; \
        else if(link<SOC_FE1600_LINKS_IN_BLOCK*2) \
          qtr = 2; \
        else if(link<(SOC_FE1600_LINKS_IN_BLOCK*3)) \
          qtr = 3; \
        else \
          qtr = 4;

/*MAX cell size including the header and CRC field (bytes)*/
#define SOC_FE1600_FABRIC_LINKS_VSC128_MAX_CELL_SIZE        (128 /*data*/ + 8 /*header*/ + 1 /*CRC*/)
#define SOC_FE1600_FABRIC_LINKS_VSC256_MAX_CELL_SIZE        (256 /*data*/ + 16 /*header*/ + 1/*CRC*/)
#define SOC_FE1600_FABRIC_LINKS_ALDWP_MIN                   (0x2)

/*
 * Function:
 *      soc_fe1600_fabric_links_nof_links_get
 * Purpose:
 *      Get number of links
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      nof_links  - (OUT) Number of links.
 * Returns:
 *      Number of links
 */
soc_error_t
soc_fe1600_fabric_links_nof_links_get(int unit, int *nof_links) {
   SOCDNX_INIT_FUNC_DEFS;
   SOC_FE1600_ONLY(unit);

   SOCDNX_NULL_CHECK(nof_links);

   *nof_links = SOC_DFE_DEFS_GET(unit, nof_links);

exit:
   SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_fe1600_fabric_links_validate_link
 * Purpose:
 *      validate link number
 * Parameters:
 *      unit  - (IN) Unit number.
 *      link  - (IN) Link 
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
soc_fe1600_fabric_links_validate_link(int unit, soc_port_t link) {
   soc_info_t          *si;
   soc_pbmp_t pbmp_valid;
   int nof_links;
   SOCDNX_INIT_FUNC_DEFS;
   

   /*get valid ports*/
   si  = &SOC_INFO(unit);
   SOC_PBMP_ASSIGN(pbmp_valid, si->sfi.bitmap);

   nof_links = SOC_DFE_DEFS_GET(unit, nof_links);

   if (link < nof_links  && link >= 0 && SOC_PBMP_MEMBER(pbmp_valid, link)) {
      SOC_EXIT;
   } else {
      SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("link %d is out of range"),link));
   }

exit:
   SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_fe1600_fabric_links_isolate_set
 * Purpose:
 *      Activate / Isolate link
 * Parameters:
 *      unit  - (IN) Unit number.
 *      link  - (IN) Link to activate / isolate
 *      val   - (IN) Activate / Isolate
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
soc_fe1600_fabric_links_isolate_set(int unit, soc_port_t link, soc_dcmn_isolation_status_t val) {
    soc_reg_above_64_val_t reachability_allowed_bm;
    soc_reg_above_64_val_t specific_link_bm;
    uint32 reg32_val;
    uint64 reg64_val, reg64_ones;
    SOCDNX_INIT_FUNC_DEFS;

    if (soc_dcmn_isolation_status_active == val) {

        SOCDNX_IF_ERR_EXIT(READ_RTP_REACHABILITY_ALLOWED_LINKS_REGISTERr(unit, reachability_allowed_bm));
        SOC_REG_ABOVE_64_CREATE_MASK(specific_link_bm, 1, link);
        SOC_REG_ABOVE_64_OR(reachability_allowed_bm, specific_link_bm);
        SOCDNX_IF_ERR_EXIT(WRITE_RTP_REACHABILITY_ALLOWED_LINKS_REGISTERr(unit, reachability_allowed_bm));

    } else {

        if (SOC_IS_FE1600(unit))
        {
            SOCDNX_IF_ERR_EXIT(READ_RTP_REG_0063r(unit, &reg32_val));
            SOCDNX_IF_ERR_EXIT(WRITE_RTP_REG_0063r(unit, 0x0));
        } else {
            SOCDNX_IF_ERR_EXIT(READ_RTP_RESERVED_10r(unit, &reg32_val));
            SOCDNX_IF_ERR_EXIT(WRITE_RTP_RESERVED_10r(unit, 0x0));
        }

        SOCDNX_IF_ERR_EXIT(READ_RTP_REACHABILITY_ALLOWED_LINKS_REGISTERr(unit, reachability_allowed_bm));

        SOC_REG_ABOVE_64_CREATE_MASK(specific_link_bm, 1, link);

        SOC_REG_ABOVE_64_NOT(specific_link_bm);
        SOC_REG_ABOVE_64_AND(reachability_allowed_bm, specific_link_bm);

        SOCDNX_IF_ERR_EXIT(WRITE_RTP_REACHABILITY_ALLOWED_LINKS_REGISTERr(unit, reachability_allowed_bm));

        sal_usleep(10000);

        if (SOC_IS_FE1600(unit))
        {
            SOCDNX_IF_ERR_EXIT(WRITE_RTP_REG_0063r(unit, reg32_val));
        } else {
            SOCDNX_IF_ERR_EXIT(WRITE_RTP_RESERVED_10r(unit, reg32_val));
        }

        /* Set UnicastRouteUpdateDirty table to copy RMH into unicast tables after periodic update is restored*/
        SOCDNX_IF_ERR_EXIT(READ_RTP_UNICAST_ROUTE_UPDATE_DIRTYr(unit, &reg64_val));
        COMPILER_64_ALLONES(reg64_ones);
        SOCDNX_IF_ERR_EXIT(WRITE_RTP_UNICAST_ROUTE_UPDATE_DIRTYr(unit, reg64_ones));

        sal_usleep(1000);

        SOCDNX_IF_ERR_EXIT(WRITE_RTP_UNICAST_ROUTE_UPDATE_DIRTYr(unit, reg64_val));
    }

    exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_fe1600_fabric_links_bmp_isolate_set
 * Purpose:
 *      Activate / Isolate link by bitmap
 * Parameters:
 *      unit  - (IN) Unit number.
 *      bitmap- (IN) Links to activate / isolate
 *      val   - (IN) Activate / Isolate
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
soc_fe1600_fabric_links_bmp_isolate_set(int unit, soc_pbmp_t bitmap, soc_dcmn_isolation_status_t val) {
   soc_reg_above_64_val_t reachability_allowed_bm;
   soc_reg_above_64_val_t specific_link_bm;
   soc_port_t link;
   SOCDNX_INIT_FUNC_DEFS;

   SOCDNX_IF_ERR_EXIT(READ_RTP_REACHABILITY_ALLOWED_LINKS_REGISTERr(unit, reachability_allowed_bm));

   SOC_PBMP_ITER(bitmap, link) {
      
      SOC_REG_ABOVE_64_CREATE_MASK(specific_link_bm, 1, link);

      if (soc_dcmn_isolation_status_active == val) {
         /*turn on relevant link bit*/
         SOC_REG_ABOVE_64_OR(reachability_allowed_bm, specific_link_bm);
      } else {
         /* turn off relevant link bit*/
         SOC_REG_ABOVE_64_NOT(specific_link_bm);
         SOC_REG_ABOVE_64_AND(reachability_allowed_bm, specific_link_bm);
      }
   }

    SOCDNX_IF_ERR_EXIT(WRITE_RTP_REACHABILITY_ALLOWED_LINKS_REGISTERr(unit, reachability_allowed_bm));

   sal_usleep(20000);

exit:
   SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_fe1600_fabric_links_isolate_get
 * Purpose:
 *      Get link state (Activated / Isolated)
 * Parameters:
 *      unit  - (IN)  Unit number.
 *      link  - (IN)  Link
 *      val   - (OUT) Link state (Activated / Isolated)
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
soc_fe1600_fabric_links_isolate_get(int unit, soc_port_t link, soc_dcmn_isolation_status_t *val) {
   soc_reg_above_64_val_t reachability_allowed_bm;
   soc_reg_above_64_val_t specific_link_bm;
   SOCDNX_INIT_FUNC_DEFS;

   SOCDNX_IF_ERR_EXIT(READ_RTP_REACHABILITY_ALLOWED_LINKS_REGISTERr(unit, reachability_allowed_bm));

   SOC_REG_ABOVE_64_CREATE_MASK(specific_link_bm, 1, link);

   SOC_REG_ABOVE_64_AND(specific_link_bm, reachability_allowed_bm);
   if (!SOC_REG_ABOVE_64_IS_ZERO(specific_link_bm)) *val = soc_dcmn_isolation_status_active;
   else *val = soc_dcmn_isolation_status_isolated;

exit:
   SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_fe1600_fabric_links_cell_format_verify(int unit, soc_port_t link, soc_dfe_fabric_link_cell_size_t val) 
{
    SOCDNX_INIT_FUNC_DEFS;

    switch (val)
    {
       case soc_dfe_fabric_link_cell_size_VSC256_V1:
       case soc_dfe_fabric_link_cell_size_VSC128:
           break;

       default:
           SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unsupported  cell format val %d"), val));
           break;

    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_fe1600_fabric_links_cell_format_set
 * Purpose:
 *      Set link cell format
 * Parameters:
 *      unit  - (IN) Unit number.
 *      link  - (IN) Link
 *      val   - (IN) Cell format
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
soc_fe1600_fabric_links_cell_format_set(int unit, soc_port_t link, soc_dfe_fabric_link_cell_size_t val) {
   soc_reg_above_64_val_t vcs_links_bm;
   soc_reg_above_64_val_t vcs_specific_link_bm;
   uint32 vcs_links_bm_32[1];
   soc_port_t inner_lnk;
   int qtr, blk_id, rc;
   uint32 enable_interleaving;
   SOCDNX_INIT_FUNC_DEFS;
   SOC_FE1600_ONLY(unit);

   /********************************************/
   /* UPDATE VSC_256_LINK_BITMAP_REGISTER (CCS)*/
   /********************************************/
   SOCDNX_IF_ERR_EXIT(READ_CCS_VSC_256_LINK_BITMAP_REGISTERr(unit, 0, vcs_links_bm));

   SOC_REG_ABOVE_64_CREATE_MASK(vcs_specific_link_bm, 1, link);

   /*In VSC_256_LINK_BITMAP_REGISTER set bit means 256*/
   if (soc_dfe_fabric_link_cell_size_VSC256_V1 == val) {
      /*turn on relevant link bit*/
      SOC_REG_ABOVE_64_OR(vcs_links_bm, vcs_specific_link_bm);
   } else {
      /*turn off relevant link bit*/
      SOC_REG_ABOVE_64_NOT(vcs_specific_link_bm);
      SOC_REG_ABOVE_64_AND(vcs_links_bm, vcs_specific_link_bm);
   }

   /*write to CCS0 and CCS1*/
   SOCDNX_IF_ERR_EXIT(WRITE_CCS_VSC_256_LINK_BITMAP_REGISTERr(unit, 0, vcs_links_bm));
   SOCDNX_IF_ERR_EXIT(WRITE_CCS_VSC_256_LINK_BITMAP_REGISTERr(unit, 1, vcs_links_bm));

   /***************************************/
   /* UPDATE GLOBAL Registers (ECI)       */
   /***************************************/
   SOC_FE1600_FABRIC_LINK_QTR(link, qtr)
   blk_id = qtr - 1;
   switch (qtr) {
   case 1:
      SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_2r(unit, vcs_links_bm_32));
      break;
   case 2:
      SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_3r(unit, vcs_links_bm_32));
      break;
   case 3:
      SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_4r(unit, vcs_links_bm_32));
      break;
   case 4:
   default:
      SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_5r(unit, vcs_links_bm_32));
   }

   /*In the GLOBAL registers set bit means 128*/
   inner_lnk = link % SOC_FE1600_LINKS_IN_BLOCK;

   if (soc_dfe_fabric_link_cell_size_VSC128 == val) {
      /*turn on relevant link bit*/
      SHR_BITSET(vcs_links_bm_32, inner_lnk);
   } else {
      /*turn off relevant link bit*/
      SHR_BITCLR(vcs_links_bm_32, inner_lnk);
   }

   /* Write links to ECI, DCL and DCH bitmaps */
   switch (qtr) {
   case 1:
      SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_2r(unit, *vcs_links_bm_32));
      break;
   case 2:
      SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_3r(unit, *vcs_links_bm_32));
      break;
   case 3:
      SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_4r(unit, *vcs_links_bm_32));
      break;
   case 4:
   default:
      SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_5r(unit, *vcs_links_bm_32));
   }
   SOCDNX_IF_ERR_EXIT(WRITE_DCL_PETRA_BMPr(unit, blk_id, *vcs_links_bm_32));
   SOCDNX_IF_ERR_EXIT(WRITE_DCH_PETRA_BMPr(unit, blk_id, *vcs_links_bm_32));

   /*********************************************/
   /*          UPDATE interleaving             */
   /*********************************************/
   enable_interleaving = (val ==  soc_dfe_fabric_link_cell_size_VSC256_V1);
   rc = soc_fe1600_fabric_links_cell_interleaving_set(unit, link, enable_interleaving);
   SOCDNX_IF_ERR_EXIT(rc);

   /********************************************/
   /*          UPDATE port_speed_max           */
   /********************************************/

   rc = soc_fe1600_port_speed_max(unit, link, &(SOC_INFO(unit).port_speed_max[link]));
   SOCDNX_IF_ERR_EXIT(rc);


exit:
   SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_fe1600_fabric_links_cell_format_get
 * Purpose:
 *      Get link cell format
 * Parameters:
 *      unit  - (IN)  Unit number.
 *      link  - (IN)  Link
 *      val   - (OUT) Cell format
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
soc_fe1600_fabric_links_cell_format_get(int unit, soc_port_t link, soc_dfe_fabric_link_cell_size_t *val) {
   soc_reg_above_64_val_t vcs_links_bm;
   soc_reg_above_64_val_t vcs_specific_link_bm;
   SOCDNX_INIT_FUNC_DEFS;
   SOC_FE1600_ONLY(unit);

   SOCDNX_IF_ERR_EXIT(READ_CCS_VSC_256_LINK_BITMAP_REGISTERr(unit, 0, vcs_links_bm));

   SOC_REG_ABOVE_64_CREATE_MASK(vcs_specific_link_bm, 1, link);

   /*set bit means 256*/
   SOC_REG_ABOVE_64_AND(vcs_links_bm, vcs_specific_link_bm);
   if (!SOC_REG_ABOVE_64_IS_ZERO(vcs_links_bm)) {
      *val = soc_dfe_fabric_link_cell_size_VSC256_V1;
   } else {
      *val = soc_dfe_fabric_link_cell_size_VSC128;
   }

exit:
   SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_fe1600_fabric_links_cell_interleaving_set
 * Purpose:
 *      Set link interleaving mode
 * Parameters:
 *      unit  - (IN) Unit number.
 *      link  - (IN) Link
 *      val   - (IN) Is link in interleaving mode (0-false, 1-true)
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
soc_fe1600_fabric_links_cell_interleaving_set(int unit, soc_port_t link, int val) {
   uint32 interleaving_bm_32[1];
   uint32 blk_id;
   soc_port_t inner_lnk;
   SOCDNX_INIT_FUNC_DEFS;
   SOC_FE1600_ONLY(unit);

   blk_id = INT_DEVIDE(link, 4);
   inner_lnk = link % 4;

   SOCDNX_IF_ERR_EXIT(READ_FMAC_CNTRL_INTRLVD_MODE_REGr(unit, blk_id, interleaving_bm_32));

   if (val) {
      /*turn on relevant link bit*/
      SHR_BITSET(interleaving_bm_32, inner_lnk);
   } else {
      /*turn off relevant link bit*/
      SHR_BITCLR(interleaving_bm_32, inner_lnk);
   }

   SOCDNX_IF_ERR_EXIT(WRITE_FMAC_CNTRL_INTRLVD_MODE_REGr(unit, blk_id, *interleaving_bm_32));

exit:
   SOCDNX_FUNC_RETURN;

}

/*
 * Function:
 *      soc_fe1600_fabric_links_cell_interleaving_get
 * Purpose:
 *      Get link interleaving mode
 * Parameters:
 *      unit  - (IN)  Unit number.
 *      link  - (IN)  Link
 *      val   - (OUT) Is link in interleaving mode (0-false, 1-true)
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
soc_fe1600_fabric_links_cell_interleaving_get(int unit, soc_port_t link, int *val) {
   uint32 interleaving_bm_32[1];
   uint32 blk_id;
   soc_port_t inner_lnk;
   SOCDNX_INIT_FUNC_DEFS;
   SOC_FE1600_ONLY(unit);

   blk_id = INT_DEVIDE(link, 4);
   inner_lnk = link % 4;

   SOCDNX_IF_ERR_EXIT(READ_FMAC_CNTRL_INTRLVD_MODE_REGr(unit, blk_id, interleaving_bm_32));

   if (SHR_BITGET(interleaving_bm_32, inner_lnk)) {
      *val = 1;
   } else {
      *val = 0;
   }

exit:
   SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_fe1600_fabric_links_weight_validate
 * Purpose:
 *      Validate link weight
 * Parameters:
 *      unit     - (IN) Unit number.
 *      link     - (IN) Link
 *      is_prim  - (IN) Refer to primary (1) or secondary (0) pipe weight
 *      val      - (IN) Weight
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
soc_fe1600_fabric_links_weight_validate(int unit, int val) {
   uint32 mask;
   SOCDNX_INIT_FUNC_DEFS;
   SOC_FE1600_ONLY(unit);

   mask = 0x7F;
   if ((val & ~mask) != 0 || val == 0) {
      SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("arg %d is not a valid weight"),val));
   }

exit:
   SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_fe1600_fabric_links_weight_set
 * Purpose:
 *      Set link primary / secondary weight
 * Parameters:
 *      unit     - (IN) Unit number.
 *      link     - (IN) Link
 *      is_prim  - (IN) Refer to primary (1) or secondary (0) pipe weight
 *      val      - (IN) Weight
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
soc_fe1600_fabric_links_weight_set(int unit, soc_port_t link, int is_prim, int val) {
   uint32 reg_val, mask;
   soc_port_t link1;
   uint32 blk_id;
   SOCDNX_INIT_FUNC_DEFS;
   SOC_FE1600_ONLY(unit);

   blk_id = INT_DEVIDE(link, SOC_FE1600_LINKS_IN_BLOCK);
   link1 = link % SOC_FE1600_LINKS_IN_BLOCK;
   mask = 0x7F;

   /*if odd link in the second helf of the rgister*/
   if (link1 % 2 != 0) {
      val = val << 14;
      mask = mask << 14;
   }

   /*if secondary move bits*/
   if (!is_prim) {
      val = val << 7;
      mask = mask << 7;
   }

   /*get current value*/
   switch (link1) {
   case 0:
   case 1:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_0r(unit, blk_id, &reg_val));
      break;
   case 2:
   case 3:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_1r(unit, blk_id, &reg_val));
      break;
   case 4:
   case 5:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_2r(unit, blk_id, &reg_val));
      break;
   case 6:
   case 7:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_3r(unit, blk_id, &reg_val));
      break;
   case 8:
   case 9:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_4r(unit, blk_id, &reg_val));
      break;
   case 10:
   case 11:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_5r(unit, blk_id, &reg_val));
      break;
   case 12:
   case 13:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_6r(unit, blk_id, &reg_val));
      break;
   case 14:
   case 15:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_7r(unit, blk_id, &reg_val));
      break;
   case 16:
   case 17:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_8r(unit, blk_id, &reg_val));
      break;
   case 18:
   case 19:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_9r(unit, blk_id, &reg_val));
      break;
   case 20:
   case 21:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_10r(unit, blk_id, &reg_val));
      break;
   case 22:
   case 23:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_11r(unit, blk_id, &reg_val));
      break;
   case 24:
   case 25:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_12r(unit, blk_id, &reg_val));
      break;
   case 26:
   case 27:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_13r(unit, blk_id, &reg_val));
      break;
   case 28:
   case 29:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_14r(unit, blk_id, &reg_val));
      break;
   case 30:
   case 31:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_15r(unit, blk_id, &reg_val));
      break;
   default:
      SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("can't find register for link %d"),link));
   }

   /*set relevant bits*/
   reg_val &= (~mask);
   reg_val |= val;

   /*set new value*/
   switch (link1) {
   case 0:
   case 1:
      SOCDNX_IF_ERR_EXIT(WRITE_DCL_LINKS_WEIGHT_CONFIG_0r(unit, blk_id, reg_val));
      break;
   case 2:
   case 3:
      SOCDNX_IF_ERR_EXIT(WRITE_DCL_LINKS_WEIGHT_CONFIG_1r(unit, blk_id, reg_val));
      break;
   case 4:
   case 5:
      SOCDNX_IF_ERR_EXIT(WRITE_DCL_LINKS_WEIGHT_CONFIG_2r(unit, blk_id, reg_val));
      break;
   case 6:
   case 7:
      SOCDNX_IF_ERR_EXIT(WRITE_DCL_LINKS_WEIGHT_CONFIG_3r(unit, blk_id, reg_val));
      break;
   case 8:
   case 9:
      SOCDNX_IF_ERR_EXIT(WRITE_DCL_LINKS_WEIGHT_CONFIG_4r(unit, blk_id, reg_val));
      break;
   case 10:
   case 11:
      SOCDNX_IF_ERR_EXIT(WRITE_DCL_LINKS_WEIGHT_CONFIG_5r(unit, blk_id, reg_val));
      break;
   case 12:
   case 13:
      SOCDNX_IF_ERR_EXIT(WRITE_DCL_LINKS_WEIGHT_CONFIG_6r(unit, blk_id, reg_val));
      break;
   case 14:
   case 15:
      SOCDNX_IF_ERR_EXIT(WRITE_DCL_LINKS_WEIGHT_CONFIG_7r(unit, blk_id, reg_val));
      break;
   case 16:
   case 17:
      SOCDNX_IF_ERR_EXIT(WRITE_DCL_LINKS_WEIGHT_CONFIG_8r(unit, blk_id, reg_val));
      break;
   case 18:
   case 19:
      SOCDNX_IF_ERR_EXIT(WRITE_DCL_LINKS_WEIGHT_CONFIG_9r(unit, blk_id, reg_val));
      break;
   case 20:
   case 21:
      SOCDNX_IF_ERR_EXIT(WRITE_DCL_LINKS_WEIGHT_CONFIG_10r(unit, blk_id, reg_val));
      break;
   case 22:
   case 23:
      SOCDNX_IF_ERR_EXIT(WRITE_DCL_LINKS_WEIGHT_CONFIG_11r(unit, blk_id, reg_val));
      break;
   case 24:
   case 25:
      SOCDNX_IF_ERR_EXIT(WRITE_DCL_LINKS_WEIGHT_CONFIG_12r(unit, blk_id, reg_val));
      break;
   case 26:
   case 27:
      SOCDNX_IF_ERR_EXIT(WRITE_DCL_LINKS_WEIGHT_CONFIG_13r(unit, blk_id, reg_val));
      break;
   case 28:
   case 29:
      SOCDNX_IF_ERR_EXIT(WRITE_DCL_LINKS_WEIGHT_CONFIG_14r(unit, blk_id, reg_val));
      break;
   case 30:
   case 31:
   default:
      SOCDNX_IF_ERR_EXIT(WRITE_DCL_LINKS_WEIGHT_CONFIG_15r(unit, blk_id, reg_val));
   }

exit:
   SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_fe1600_fabric_links_weight_get
 * Purpose:
 *      Get link primary / secondary weight
 * Parameters:
 *      unit     - (IN)  Unit number.
 *      link     - (IN)  Link
 *      is_prim  - (IN)  Refer to primary (1) or secondary (0) pipe weight
 *      val      - (OUT) Weight
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
soc_fe1600_fabric_links_weight_get(int unit, soc_port_t link, int is_prim, int *val) {
   uint32 reg_val, mask;
   soc_port_t link1;
   uint32 blk_id;
   SOCDNX_INIT_FUNC_DEFS;
   SOC_FE1600_ONLY(unit);

   blk_id = INT_DEVIDE(link, SOC_FE1600_LINKS_IN_BLOCK);
   link1 = link % SOC_FE1600_LINKS_IN_BLOCK;
   mask = 0x7F;

   /*if odd link in the second helf of the rgister*/
   if (link1 % 2 != 0) {
      mask = mask << 14;
   }

   /*if secondary move bits*/
   if (!is_prim) {
      mask = mask << 7;
   }

   /*get current value*/
   switch (link1) {
   case 0:
   case 1:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_0r(unit, blk_id, &reg_val));
      break;
   case 2:
   case 3:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_1r(unit, blk_id, &reg_val));
      break;
   case 4:
   case 5:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_2r(unit, blk_id, &reg_val));
      break;
   case 6:
   case 7:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_3r(unit, blk_id, &reg_val));
      break;
   case 8:
   case 9:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_4r(unit, blk_id, &reg_val));
      break;
   case 10:
   case 11:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_5r(unit, blk_id, &reg_val));
      break;
   case 12:
   case 13:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_6r(unit, blk_id, &reg_val));
      break;
   case 14:
   case 15:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_7r(unit, blk_id, &reg_val));
      break;
   case 16:
   case 17:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_8r(unit, blk_id, &reg_val));
      break;
   case 18:
   case 19:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_9r(unit, blk_id, &reg_val));
      break;
   case 20:
   case 21:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_10r(unit, blk_id, &reg_val));
      break;
   case 22:
   case 23:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_11r(unit, blk_id, &reg_val));
      break;
   case 24:
   case 25:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_12r(unit, blk_id, &reg_val));
      break;
   case 26:
   case 27:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_13r(unit, blk_id, &reg_val));
      break;
   case 28:
   case 29:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_14r(unit, blk_id, &reg_val));
      break;
   case 30:
   case 31:
      SOCDNX_IF_ERR_EXIT(READ_DCL_LINKS_WEIGHT_CONFIG_15r(unit, blk_id, &reg_val));
      break;
   default:
      SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("can't find register for link %d"),link));
   }

   reg_val &= mask;

   /*if odd link in the second helf of the rgister*/
   if (link1 % 2 != 0) {
      reg_val = reg_val >> 14;
   }

   /*if secondary move bits*/
   if (!is_prim) {
      reg_val = reg_val >>  7;
   }

   *val = reg_val;

exit:
   SOCDNX_FUNC_RETURN;

}

/*
 * Function:
 *      soc_fe1600_fabric_links_secondary_only_set
 * Purpose:
 *     When the device is in dual switch mode, and a BCM88X4X device is
 *     connected to a link, determines whether the cells that are received are mapped to the primary or to the
 *     secondary pipes.
 * Parameters:
 *      unit     - (IN)  Unit number.
 *      link     - (IN)  Link
 *      val      - (OUT) is secondary?
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
soc_fe1600_fabric_links_secondary_only_set(int unit, soc_port_t link, int val) {
   uint32 reg_val[1];
   soc_port_t inner_lnk;
   int blk_id;
   SOCDNX_INIT_FUNC_DEFS;
   SOC_FE1600_ONLY(unit);

   blk_id = INT_DEVIDE(link, 32);
   inner_lnk = link % 32;

   /*DCH*/
   SOCDNX_IF_ERR_EXIT(READ_DCH_PETRA_PIPE_BMPr(unit, blk_id, reg_val));

   if (val) SHR_BITCLR(reg_val, inner_lnk);
   else SHR_BITSET(reg_val, inner_lnk);

   SOCDNX_IF_ERR_EXIT(WRITE_DCH_PETRA_PIPE_BMPr(unit, blk_id, *reg_val));

   /*DCL*/
   SOCDNX_IF_ERR_EXIT(READ_DCL_PETRA_PIPE_BMPr(unit, blk_id, reg_val));

   if (val) SHR_BITCLR(reg_val, inner_lnk);
   else SHR_BITSET(reg_val, inner_lnk);

   SOCDNX_IF_ERR_EXIT(WRITE_DCL_PETRA_PIPE_BMPr(unit, blk_id, *reg_val));

exit:
   SOCDNX_FUNC_RETURN;

}

/*
 * Function:
 *      soc_fe1600_fabric_links_secondary_only_get
 * Purpose:
 *     When the device is in dual switch mode, and a BCM88X4X device is
 *     connected to a link, determines whether the cells that are received are mapped to the primary or to the
 *     secondary pipes.
 * Parameters:
 *      unit     - (IN)  Unit number.
 *      link     - (IN)  Link
 *      val      - (OUT) is secondary?
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
soc_fe1600_fabric_links_secondary_only_get(int unit, soc_port_t link, int *val) {
   uint32 reg_val[1];
   soc_port_t inner_lnk;
   int blk_id;
   SOCDNX_INIT_FUNC_DEFS;
   SOC_FE1600_ONLY(unit);

   blk_id = INT_DEVIDE(link, 32);
   inner_lnk = link % 32;

   SOCDNX_IF_ERR_EXIT(READ_DCH_PETRA_PIPE_BMPr(unit, blk_id, reg_val));

   if (SHR_BITGET(reg_val, inner_lnk)) *val = 0;
   else *val = 1;

exit:
   SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_fe1600_fabric_links_llf_control_source_set
 * Purpose:
 *      Set LLFC control source
 * Parameters:
 *      unit     - (IN) Unit number.
 *      link     - (IN) Link
 *      val      - (IN) LLFC control source
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
soc_fe1600_fabric_links_llf_control_source_set(int unit, soc_port_t link, soc_dcmn_fabric_pipe_t val) {
   soc_port_t inner_lnk;
   uint32 blk_id, reg_val;
   uint32 field_val_rx[1], field_val_tx[1];
   SOCDNX_INIT_FUNC_DEFS;

   blk_id = INT_DEVIDE(link, 4);
   inner_lnk = link % 4;

   /*Enable\disable LLFC*/
   SOCDNX_IF_ERR_EXIT(READ_FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr(unit, blk_id, &reg_val));
   *field_val_rx = soc_reg_field_get(unit, FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr, reg_val, LNK_LVL_FC_RX_ENf);
   *field_val_tx = soc_reg_field_get(unit, FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr, reg_val, LNK_LVL_FC_TX_ENf);
   if (SOC_DCMN_FABRIC_PIPE_ALL_PIPES_IS_CLEAR(val)) {
      SHR_BITCLR(field_val_rx, inner_lnk);
      SHR_BITCLR(field_val_tx, inner_lnk);
   } else {
      SHR_BITSET(field_val_rx, inner_lnk);
      SHR_BITSET(field_val_tx, inner_lnk);
   }
   soc_reg_field_set(unit, FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr, &reg_val, LNK_LVL_FC_RX_ENf, *field_val_rx);
   soc_reg_field_set(unit, FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr, &reg_val, LNK_LVL_FC_TX_ENf, *field_val_tx);
   SOCDNX_IF_ERR_EXIT(WRITE_FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr(unit, blk_id, reg_val));

exit:
   SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_fe1600_fabric_links_llf_control_source_get
 * Purpose:
 *      Get LLFC control source
 * Parameters:
 *      unit     - (IN)  Unit number.
 *      link     - (IN)  Link
 *      val      - (OUT) LLFC control source
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
soc_fe1600_fabric_links_llf_control_source_get(int unit, soc_port_t link, soc_dcmn_fabric_pipe_t *val) {
   soc_port_t inner_lnk;
   int nof_pipes;
   uint32 blk_id, reg_val;
   uint32 field_val_rx[1], field_val_tx[1];
   SOCDNX_INIT_FUNC_DEFS;

   blk_id = INT_DEVIDE(link, 4);
   inner_lnk = link % 4;
   nof_pipes=SOC_DFE_FABRIC_PIPES_CONFIG(unit).nof_pipes;
   /*First check LLFC enablers*/
   SOCDNX_IF_ERR_EXIT(READ_FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr(unit, blk_id, &reg_val));
   *field_val_rx = soc_reg_field_get(unit, FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr, reg_val, LNK_LVL_FC_RX_ENf);
   *field_val_tx = soc_reg_field_get(unit, FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr, reg_val, LNK_LVL_FC_TX_ENf);
   if (!SHR_BITGET(field_val_rx, inner_lnk) && !SHR_BITGET(field_val_tx, inner_lnk)) {
       SOC_DCMN_FABRIC_PIPE_INIT(*val);
   } else {
       SOC_DCMN_FABRIC_PIPE_ALL_PIPES_SET(val,nof_pipes);       
   }

exit:
   SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_fe1600_fabric_links_aldwp_config
 * Purpose:
 *      Configure ALDWP per single link speed change.
 * Parameters:
 *      unit     - (IN)  Unit number.
 *      port     - (IN)  Link
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_fe1600_fabric_links_aldwp_config(int unit, soc_port_t port)
{
    int rv;
    uint32 max_cell_size, aldwp, highest_aldwp, max_aldwp;
    int speed;
    int dch_instance, dch_inner_link;
    int offset=0;
    soc_dfe_fabric_link_device_mode_t device_mode;
    soc_dfe_fabric_link_cell_size_t cell_type;
    soc_port_t port_index;
    uint32 reg32_val;
    soc_dcmn_port_pcs_t encoding = 0;
    int enable = 0;
    soc_pbmp_t pbmp;

    SOCDNX_INIT_FUNC_DEFS;

    /* 
     * Calc the max number of clock ticks to receive 3 cells:
     *  
     *          3 *  max cell size * core clock
     *  aldwp =   ------------------------------
     *                   link rate
     *  
     * Note: should find the highest aldwp at the specific DCH instance 
     */

    /*Reterive the relevant parameters: link rate and cell_format*/

    /*Find the highest aldwp over a specific DCH*/
    highest_aldwp = 0;
    

    rv = MBCM_DFE_DRIVER_CALL(unit, mbcm_dfe_drv_link_to_block_mapping, (unit, port, &dch_instance, &dch_inner_link, SOC_BLK_DCH));
    SOCDNX_IF_ERR_EXIT(rv);

    SOCDNX_IF_ERR_EXIT(MBCM_DFE_DRIVER_CALL(unit,mbcm_dfe_fabric_link_device_mode_get,(unit,port, 1/*rx*/, &device_mode)));

    rv = MBCM_DFE_DRIVER_CALL(unit, mbcm_dfe_drv_block_pbmp_get, (unit, SOC_BLK_DCH, dch_instance, &pbmp));
    SOCDNX_IF_ERR_EXIT(rv);
    SOC_PBMP_ITER(pbmp, port_index)
    {
        
        rv = MBCM_DFE_DRIVER_CALL(unit, mbcm_dfe_port_phy_enable_get, (unit, port_index,&enable)); 
        SOCDNX_IF_ERR_EXIT(rv);
        if (!enable)
        {
            /*port is disabled*/
            continue;
        }

        
        rv = MBCM_DFE_DRIVER_CALL(unit, mbcm_dfe_port_speed_get, (unit, port_index, &speed));
        SOCDNX_IF_ERR_EXIT(rv);
        if (speed == 0)
        {
            /*port is disabled*/
            continue;
        }

        rv = MBCM_DFE_DRIVER_CALL(unit, mbcm_dfe_port_control_pcs_get, (unit, port_index, &encoding));
        if (encoding == soc_dcmn_port_pcs_8_10)
        {
            speed = (speed * FE1600_PORT_PCS_8_10_EFFECTIVE_RATE_PERCENT) / 100;
        }

        rv = MBCM_DFE_DRIVER_CALL(unit, mbcm_dfe_fabric_links_cell_format_get, (unit, port_index, &cell_type));
        SOCDNX_IF_ERR_EXIT(rv);
        switch (cell_type)
        {
            case soc_dfe_fabric_link_cell_size_VSC128:
                if (SOC_DFE_CONFIG(unit).fabric_merge_cells)
                {
                    max_cell_size = 2 * SOC_FE1600_FABRIC_LINKS_VSC128_MAX_CELL_SIZE; /*2 cells are merged*/
                } else {
                    max_cell_size = SOC_FE1600_FABRIC_LINKS_VSC128_MAX_CELL_SIZE;
                }
                break;
            case soc_dfe_fabric_link_cell_size_VSC256_V1:
            case soc_dfe_fabric_link_cell_size_VSC256_V2: /*fall through*/
                max_cell_size = SOC_FE1600_FABRIC_LINKS_VSC256_MAX_CELL_SIZE;
                break;
            
            default:
                SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOCDNX_MSG("Unknown cell format: %u"), cell_type));
        }

        /*Calc the max number of tiks to receive 3 cells*/
        aldwp = (3* max_cell_size * SOC_DFE_CONFIG(unit).core_clock_speed) / (speed / 8 /*bits to bytes*/); /*mili ticks*/
        aldwp = aldwp /(1024*64) /*units of 64 ticks*/ + ((aldwp % (1024*64) != 0) ? 1 : 0) /*round up*/;

        /*update MAX*/
        highest_aldwp = (aldwp > highest_aldwp ? aldwp : highest_aldwp); 
        
    }

    /*calculate offset to max based on device mode*/
    if (highest_aldwp != 0 /*at least one link is enabled*/)
    {
        if (device_mode == soc_dfe_fabric_link_device_mode_fe2)
        {
            offset=1;
        }
        else if (device_mode == soc_dfe_fabric_link_device_mode_multi_stage_fe3)
        {
            offset=2;
        }
         highest_aldwp += offset;
    }


    max_aldwp = SOC_DFE_DEFS_GET(unit,aldwp_max_val);
    /* if FE is in repeater mode, aldwp should be 0 */
    if (device_mode == soc_dfe_fabric_link_device_mode_repeater)
    {
        highest_aldwp=0;
    }
    else if (highest_aldwp == 0) /*Check reterived values*/
    {
        /*All port are disabled or powered down*/
        SOC_EXIT;
    } else if (highest_aldwp < SOC_FE1600_FABRIC_LINKS_ALDWP_MIN)
    {
        highest_aldwp = SOC_FE1600_FABRIC_LINKS_ALDWP_MIN;
    } else if (highest_aldwp > max_aldwp)
    {
        highest_aldwp = max_aldwp;
    }

    
    /*Configure relevant register*/
#ifdef BCM_88950
    if (SOC_IS_FE3200(unit))
    {
        uint64 reg64_val;

        SOCDNX_IF_ERR_EXIT(READ_DCH_DCH_ENABLERS_REGISTER_1r(unit, dch_instance, &reg64_val));
        soc_reg64_field32_set(unit, DCH_DCH_ENABLERS_REGISTER_1r, &reg64_val, FIELD_21_26f, highest_aldwp);
        SOCDNX_IF_ERR_EXIT(WRITE_DCH_DCH_ENABLERS_REGISTER_1r(unit, dch_instance, reg64_val));
    } else
#endif /*BCM_88950*/
    {
        SOCDNX_IF_ERR_EXIT(READ_DCH_REG_0060r(unit, dch_instance, &reg32_val));
        soc_reg_field_set(unit, DCH_REG_0060r, &reg32_val, FIELD_21_24f, highest_aldwp);
        SOCDNX_IF_ERR_EXIT(WRITE_DCH_REG_0060r(unit, dch_instance, reg32_val));
    }

exit:
   SOCDNX_FUNC_RETURN;
}


/*
 * Function:
 *      soc_fe1600_fabric_links_aldwp_init
 * Purpose:
 *      configure aldwp according to init speed values.
 * Parameters:
 *      unit     - (IN)  Unit number.
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_fe1600_fabric_links_aldwp_init(int unit)
{
    int rv;
    int dch_instance,
        nof_dch,
        nof_links_in_dcq;
    SOCDNX_INIT_FUNC_DEFS;

    nof_dch = SOC_DFE_DEFS_GET(unit, nof_instances_dch);
    nof_links_in_dcq = SOC_DFE_DEFS_GET(unit, nof_links_in_dcq);

    for (dch_instance = 0; dch_instance < nof_dch; dch_instance++)
    {
        rv = soc_fe1600_fabric_links_aldwp_config(unit, dch_instance *  nof_links_in_dcq/*First link in dch*/);
        SOCDNX_IF_ERR_EXIT(rv);
    }
 
exit:
   SOCDNX_FUNC_RETURN;
}

soc_error_t 
soc_fe1600_fabric_link_device_mode_get(int unit,soc_port_t port, int is_rx, soc_dfe_fabric_link_device_mode_t *link_device_mode)
{
    int nof_links, asymmetrical_quad = 0;
    SOCDNX_INIT_FUNC_DEFS;

    nof_links = SOC_DFE_DEFS_GET(unit, nof_links);
    switch (SOC_DFE_CONFIG(unit).fabric_device_mode)
    {
       case soc_dfe_fabric_device_mode_multi_stage_fe2:
       case soc_dfe_fabric_device_mode_single_stage_fe2:
           *link_device_mode=soc_dfe_fabric_link_device_mode_fe2;
           break;

       case soc_dfe_fabric_device_mode_repeater:
           *link_device_mode=soc_dfe_fabric_link_device_mode_repeater;
           break;

       case soc_dfe_fabric_device_mode_multi_stage_fe13:
           if (is_rx)
           {
               *link_device_mode = (port < nof_links / 2) ? soc_dfe_fabric_link_device_mode_multi_stage_fe1 : soc_dfe_fabric_link_device_mode_multi_stage_fe3;
           } else {
               *link_device_mode = (port < nof_links / 2) ? soc_dfe_fabric_link_device_mode_multi_stage_fe3 : soc_dfe_fabric_link_device_mode_multi_stage_fe1;
           }
           break;

       case soc_dfe_fabric_device_mode_multi_stage_fe13_asymmetric:
           MBCM_DFE_DRIVER_CALL(unit, mbcm_dfe_drv_asymmetrical_quad_get, (unit, port, &asymmetrical_quad));
           if ((asymmetrical_quad == 0 || asymmetrical_quad == 1)) /* changed quad */
           {
               if (is_rx)
               {
                   *link_device_mode = soc_dfe_fabric_link_device_mode_multi_stage_fe1;
               }
               else
               {
                   *link_device_mode = soc_dfe_fabric_link_device_mode_multi_stage_fe3;
               }
           }
           else /* unchanged quad*/
           {
               if (is_rx)
               {
                   *link_device_mode = (port < nof_links / 2) ? soc_dfe_fabric_link_device_mode_multi_stage_fe1 : soc_dfe_fabric_link_device_mode_multi_stage_fe3;
               } 
               else 
               {
                   *link_device_mode = (port < nof_links / 2) ? soc_dfe_fabric_link_device_mode_multi_stage_fe3 : soc_dfe_fabric_link_device_mode_multi_stage_fe1;
               }
           }
           break;

       default:
           SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("device is not in a valid mode")));
           break;
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_fe1600_fabric_links_pcp_enable_get
 * Purpose:
 *      Get Packet-Cell-Packing enable
 * Parameters:
 *      unit      - (IN)  Unit number.
 *      port      - (IN)  port number 
 *      enable  - (OUT) 
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_fe1600_fabric_links_pcp_enable_get(int unit, soc_port_t port, int *enable)
{
    SOCDNX_INIT_FUNC_DEFS;

    /*PCP is not supported*/
    *enable = 0;

    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_fe1600_fabric_link_repeater_enable_get
 * Purpose:
 *      Enable/Disable link connected to repeater (Always disabled - repeater link is not supported for FE1600)
 * Parameters:
 *      unit            - (IN)  Unit number.
 *      port            - (IN)  port number 
 *      enable          - (OUT) enable/disable
 *      empty_cell_size - (OUT) releavant empty cell size
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */


soc_error_t
soc_fe1600_fabric_link_repeater_enable_get(int unit, soc_port_t port, int *enable, int *empty_cell_size)
{
   SOCDNX_INIT_FUNC_DEFS;
   SOC_FE1600_ONLY(unit);

    *empty_cell_size = SOC_DFE_DEFS_GET(unit, repeater_default_empty_cell_size); 
    *enable = 0; /*repeater link is not supported*/

   SOCDNX_FUNC_RETURN;
}

#endif /*defined(BCM_88750_A0)*/

#undef _ERR_MSG_MODULE_NAME

