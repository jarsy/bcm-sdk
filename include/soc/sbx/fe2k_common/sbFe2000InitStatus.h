#ifndef __SB_FE2000_INIT_STATUS_H__
#define __SB_FE2000_INIT_STATUS_H__
/* -*- Mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */
/*
 * $Id: sbFe2000InitStatus.h,v 1.5 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * sbFe2000InitStatus.h : Enumerated FE2000 status codes
 *
 *-----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * Enumerated Type of Return Codes
 *
 **/
typedef enum sbFe2000InitStatus_es {
  SB_FE2000_STS_INIT_OK_K = 0,
  SB_FE2000_STS_INIT_BAD_ARGS = SB_BUILD_ERR_CODE(SB_VENDOR_ID_SANDBURST, SB_MODULE_ID_FE2000_INIT, 0x1),
  SB_FE2000_STS_INIT_QM_TIMEOUT_ERR_K, /* 17301506 */
  SB_FE2000_STS_INIT_PT_SPI0_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_PT_SPI1_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_PT_AG0_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_PT_AG1_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_PT_XG0_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_PT_XG1_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_PT_PCI_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_PT_PED_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_PR_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_PP_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_AGM_WRITE_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_AGM_READ_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_XGM_WRITE_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_XGM_READ_TIMEOUT_ERR_K,
  SB_FE2000_STS_INIT_IIC_READ_TIMEOUT_ERR,
  SB_FE2000_STS_INIT_IIC_WRITE_TIMEOUT_ERR,
  SB_FE2000_STS_INIT_MII_WRITE_TIMEOUT_ERR,
  SB_FE2000_STS_INIT_MII_READ_TIMEOUT_ERR,
  SB_FE2000_STS_INIT_LR_UCODESWAP_TIMEOUT_ERR,
  /* leave as last */
  SB_FE2000_STS_INIT_LAST
} sbFe2000InitStatus_et;

char * sbGetFe2000InitStatusString(sbStatus_t status);
#ifdef __cplusplus
}
#endif

#endif /*__SB_FE2000_INIT_STATUS_H__ */
