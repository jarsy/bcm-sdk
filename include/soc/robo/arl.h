/*
 * $Id: $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * 
 */
   

#ifndef _SOC_ROBO_ARL_H
#define _SOC_ROBO_ARL_H

#include <shared/bsl.h>

#include <soc/arl.h>

#define DRV_ARL_WARN(stuff)         LOG_WARN(BSL_LS_SOC_ARL, stuff)
#define DRV_ARL_ERR(stuff)          LOG_ERROR(BSL_LS_SOC_ARL, stuff)
#define DRV_ARL_VERB(stuff)         LOG_VERBOSE(BSL_LS_SOC_ARL, stuff)
#define DRV_ARL_VVERB(stuff)        LOG_DEBUG(BSL_LS_SOC_ARL, stuff)


#define _ROBO_SEARCH_LOCK (1 << 0)
#define _ROBO_SCAN_LOCK (1 << 1)


#define TBX_ARL_SCAN_LOCK(unit, soc)\
    do{\
        MEM_RWCTRL_REG_LOCK(soc);\
        soc->arl_exit &= ~(_ROBO_SCAN_LOCK);\
        DRV_ARL_VVERB(("%s %d MEM_RWCTRL_REG_LOCK\n",FUNCTION_NAME(),__LINE__));\
    }while(0)


#define TBX_ARL_SCAN_UNLOCK(unit, soc)\
    do{\
        DRV_ARL_VVERB(("%s %d MEM_RWCTRL_REG_UNLOCK\n",FUNCTION_NAME(),__LINE__));\
        soc->arl_exit |= _ROBO_SCAN_LOCK;\
        MEM_RWCTRL_REG_UNLOCK(soc);\
    }while(0)


#define VO_ARL_SEARCH_LOCK(unit, soc) \
    do{\
        if(SOC_IS_VO(unit)) {\
            ARL_MEM_SEARCH_LOCK(soc);\
            soc->arl_exit &= ~(_ROBO_SEARCH_LOCK);\
            DRV_ARL_VVERB(("%s %d ARL_MEM_SEARCH_LOCK--\n",FUNCTION_NAME(),__LINE__));\
        }\
    }while(0)


#define VO_ARL_SEARCH_UNLOCK(unit, soc) \
do{\
    if(SOC_IS_VO(unit)) {\
        DRV_ARL_VVERB(("%s %d ARL_MEM_SEARCH_UNLOCK\n",FUNCTION_NAME(),__LINE__));\
        soc->arl_exit |= _ROBO_SEARCH_LOCK;\
        ARL_MEM_SEARCH_UNLOCK(soc);\
    }\
}while(0)

#endif /* _SOC_ROBO_ARL_H */
