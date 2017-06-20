/*
 * $Id: bcm_dlist.h,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BCM_DLIST_ENTRY


#ifdef BCM_ROBO_SUPPORT
BCM_DLIST_ENTRY(robo)
#endif

#ifdef BCM_ESW_SUPPORT
BCM_DLIST_ENTRY(esw)
#endif

#ifdef BCM_SBX_SUPPORT
BCM_DLIST_ENTRY(sbx)
#endif

#ifdef BCM_FE2000_SUPPORT
BCM_DLIST_ENTRY(fe2000)
#endif

#ifdef BCM_PETRA_SUPPORT
BCM_DLIST_ENTRY(petra)
#endif

#ifdef BCM_DFE_SUPPORT
BCM_DLIST_ENTRY(dfe)
#endif

#ifdef BCM_DNX_SUPPORT
BCM_DLIST_ENTRY(dnx)
#endif

#ifdef BCM_RPC_SUPPORT
BCM_DLIST_ENTRY(client) 
#endif

#ifdef BCM_LOOP_SUPPORT
BCM_DLIST_ENTRY(loop)
#endif

#ifdef BCM_TR3_SUPPORT
BCM_DLIST_ENTRY(tr3)
#endif

#ifdef BCM_SHADOW_SUPPORT
BCM_DLIST_ENTRY(shadow)
#endif   
      
#ifdef BCM_TK371X_SUPPORT
BCM_DLIST_ENTRY(tk371x)
#endif

#ifdef BCM_CALADAN3_SUPPORT
BCM_DLIST_ENTRY(caladan3)
#endif

#undef BCM_DLIST_ENTRY


#endif /* BCM_DLIST_ENTRY */
