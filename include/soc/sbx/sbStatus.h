#ifndef _SB_STATUS_H_
#define _SB_STATUS_H_
/* --------------------------------------------------------------------------
** $Id: sbStatus.h,v 1.10 Broadcom SDK $
**
** $Copyright: (c) 2016 Broadcom.
** Broadcom Proprietary and Confidential. All rights reserved.$
** --------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

typedef enum sbVendorID_es
{
  SB_VENDOR_ID_SANDBURST = 1,
  SB_VENDOR_ID_LAST
} sbVendorID_et;

typedef enum sbModuleID_es
{
  SB_MODULE_ID_FE_ILIB = 1,
  SB_MODULE_ID_QE_ELIB = 2,
  SB_MODULE_ID_FAB_ILIB = 3,
  SB_MODULE_FE1000 = 4,
  SB_MODULE_ID_FLIB2 = 5,
  SB_MODULE_ID_MEM = 6,
  SB_MODULE_ID_FE2000MEM = 7,
  SB_MODULE_ID_FE2000_INIT = 8,
  SB_MODULE_ID_LAST
} sbModuleID_et;

typedef int sbStatus_t;
typedef int* sbStatus_pt;

#define SB_BUILD_ERR_CODE(v, m, e) (( (v & 0x000000ff) << 24) | \
                                    ( (m & 0x000000ff) << 16) | \
                                    ( (e & 0x0000ffff) ) )

#define SB_GET_ERROR_VENDOR(e) ( (e & 0xff000000) >> 24)
#define SB_GET_ERROR_MODULE(e) ( (e & 0x00ff0000) >> 16)
#define SB_GET_ERROR_NUM(e) (e & 0x0000ffff)

/* Generic failure code */
#define SB_FAILED -1

/* NOTE: includes placed after definition of shared MACROS */
#include <soc/sbx/sbFeIlibStatus.h>
#include <soc/sbx/fe2k_common/sbFe2000InitStatus.h>

char* sbGetVendorString(int vendor);
char* sbGetModuleString(int module);
char* sbGetErrorString(int error);

#ifdef __cplusplus
}
#endif
#endif
