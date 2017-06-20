/*
 * $Id: soc_sw_db.h,v 1.2.10.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef _SOC_SBX_CALADAN3_SW_DB_H_
#define _SOC_SBX_CALADAN3_SW_DB_H_


#include <soc/types.h>
#include <shared/bsl.h>

#define SOC_CALADAN3_ALLOW_WARMBOOT_WRITE(operation, _rv)\
            SOC_ALLOW_WB_WRITE(unit, operation, _rv)

#define SOC_CALADAN3_ALLOW_WARMBOOT_WRITE_NO_ERR(operation, _rv)\
        do { \
            SOC_ALLOW_WB_WRITE(unit, operation, _rv); \
            if (_rv != SOC_E_UNIT) { \
                _rv = SOC_E_NONE; \
            } \
        } while(0)


#define SOC_CALADAN3_WARMBOOT_RELEASE_HW_MUTEX(_rv)\
        do {\
            _rv = soc_schan_override_disable(unit); \
        } while(0)


/* scache operations */
typedef enum soc_caladan3_scache_oper_e {
    socScacheRetrieve,
    socScacheCreate,
    socScacheRealloc,
    socScacheFreeCreate
} soc_caladan3_scache_oper_t;


/* consistency flags */
#define SOC_CALADAN3_SCACHE_EXISTS_ERROR                0x00000001
#define SOC_CALADAN3_SCACHE_DOWNGRADE_INVALID           0x00000002
#define SOC_CALADAN3_SCACHE_UPGRADE_INVALID             0x00000004
#define SOC_CALADAN3_SCACHE_DEFAULT                     (SOC_CALADAN3_SCACHE_DOWNGRADE_INVALID)
                                                    

extern int
soc_caladan3_sw_db_sync(int unit, int arg);

extern int
soc_caladan3_scache_ptr_get(int unit, soc_scache_handle_t handle, soc_caladan3_scache_oper_t oper,
                                 int flags, uint32 *size, uint8 **scache_ptr,
                                 uint16 ver, uint16 *recovered_ver, int *already_exists);

extern void
soc_sbx_warmboot_signatures(int unit);

#endif /* _SOC_SBX_CALADAN3_SW_DB_H_ */
