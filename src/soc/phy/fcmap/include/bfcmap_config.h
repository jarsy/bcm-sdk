/*
 * $Id: bfcmap_config.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef __BFCMAP_CONFIG_H__
#define __BFCMAP_CONFIG_H__

#include <buser_config.h>

#ifndef BFCMAP_MAX_UNITS
#define BFCMAP_MAX_UNITS                    BMF_MAX_UNITS
#endif

/* Maximum number of ports per chip supported */
#ifndef BFCMAP_MAX_PORTS
#define BFCMAP_MAX_PORTS                    BMF_MAX_PORTS
#endif

#define BFCMAP_NUM_PORT                     BMF_NUM_PORT

#ifndef BFCMAP_CONFIG_DEFINE_UINT8_T
#define BFCMAP_CONFIG_DEFINE_UINT8_T        BMF_CONFIG_DEFINE_UINT8_T
#endif

/* Default type definition for uint8 */
#ifndef BFCMAP_CONFIG_TYPE_UINT8_T
#define BFCMAP_CONFIG_TYPE_UINT8_T          BMF_CONFIG_TYPE_UINT8_T
#endif

#ifndef BFCMAP_CONFIG_DEFINE_INT8_T
#define BFCMAP_CONFIG_DEFINE_INT8_T         BMF_CONFIG_DEFINE_INT8_T
#endif

#ifndef BFCMAP_CONFIG_TYPE_INT8_T
#define BFCMAP_CONFIG_TYPE_INT8_T           BMF_CONFIG_TYPE_INT8_T
#endif

/* Type buint16_t is not provided by the system */
#ifndef BFCMAP_CONFIG_DEFINE_UINT16_T
#define BFCMAP_CONFIG_DEFINE_UINT16_T       BMF_CONFIG_DEFINE_UINT16_T
#endif

/* Default type definition for uint16 */
#ifndef BFCMAP_CONFIG_TYPE_UINT16_T
#define BFCMAP_CONFIG_TYPE_UINT16_T         BMF_CONFIG_TYPE_UINT16_T
#endif

#ifndef BFCMAP_CONFIG_DEFINE_INT16_T
#define BFCMAP_CONFIG_DEFINE_INT16_T        BMF_CONFIG_DEFINE_INT16_T
#endif

#ifndef BFCMAP_CONFIG_TYPE_INT16_T
#define BFCMAP_CONFIG_TYPE_INT16_T          BMF_CONFIG_TYPE_INT16_T
#endif

/* Type buint32_t is not provided by the system */
#ifndef BFCMAP_CONFIG_DEFINE_UINT32_T
#define BFCMAP_CONFIG_DEFINE_UINT32_T       BMF_CONFIG_DEFINE_UINT32_T
#endif

/* Default type definition for uint32 */
#ifndef BFCMAP_CONFIG_TYPE_UINT32_T
#define BFCMAP_CONFIG_TYPE_UINT32_T         BMF_CONFIG_TYPE_UINT32_T
#endif

#ifndef BFCMAP_CONFIG_DEFINE_INT32_T
#define BFCMAP_CONFIG_DEFINE_INT32_T        BMF_CONFIG_DEFINE_INT32_T
#endif

#ifndef BFCMAP_CONFIG_TYPE_INT32_T
#define BFCMAP_CONFIG_TYPE_INT32_T          BMF_CONFIG_TYPE_INT32_T
#endif

#ifndef BFCMAP_CONFIG_DEFINE_UINT64_T
#define BFCMAP_CONFIG_DEFINE_UINT64_T       BMF_CONFIG_DEFINE_UINT64_T
#endif

/* Default type definition for uint64 */
#ifndef BFCMAP_CONFIG_TYPE_UINT64_T
#define BFCMAP_CONFIG_TYPE_UINT64_T         BMF_CONFIG_TYPE_UINT64_T
#endif

#ifndef BFCMAP_CONFIG_DEFINE_INT64_T
#define BFCMAP_CONFIG_DEFINE_INT64_T        BMF_CONFIG_DEFINE_INT64_T
#endif

#ifndef BFCMAP_CONFIG_TYPE_INT64_T
#define BFCMAP_CONFIG_TYPE_INT64_T          BMF_CONFIG_TYPE_INT64_T
#endif


#endif /* __BFCMAP_CONFIG_H__ */


