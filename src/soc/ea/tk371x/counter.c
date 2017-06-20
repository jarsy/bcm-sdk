/*
 * $Id: counter.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    counter.c
 * Purpose:     Tracks and manages onu chip stats.
 *      
 */    
#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <sal/core/spl.h>
#include <sal/core/sync.h>
#include <sal/core/time.h>
     
#include <soc/drv.h>
#include <soc/counter.h>
#include <soc/ll.h>
#include <soc/debug.h>
#include <soc/error.h>
#include <soc/cm.h>
#include <soc/ea/tk371x/TkDefs.h>
#include <soc/ea/tk371x/counter.h>
#include <soc/ea/tk371x/TkStatsApi.h>
 /*
  * Function:
  *      soc_ea_counter_attach
  * Purpose:
  *      Initialize counter module.
  * Notes:
  *      Allocates counter collection buffers.
  */
 int
 _soc_ea_counter_attach(int unit)
 {
     soc_control_t       *soc;
     int         n_bytes;
     int         tot_bytes;
     soc_port_t  ponNum;
     soc_port_t  geNum;
     soc_port_t  feNum;
     soc_port_t  llidNum;
     soc_port_t  phy_port;
     int         blk;
     int         bindex;
     int         blktype;
     soc_ea_counter_t *counter = NULL;
          
     assert(SOC_UNIT_VALID(unit));
 
     soc = SOC_CONTROL(unit);
     
     ponNum = 0;
     geNum = 0;
     feNum = 0;
     llidNum = 0;
     
     for (phy_port = 0; ; phy_port++) {
         blk = SOC_PORT_INFO(unit, phy_port).blk;
         bindex = SOC_PORT_INFO(unit, phy_port).bindex;
         if (blk < 0 && bindex < 0) {                    /* end of list */
             break;
         }
         
         blktype = SOC_BLOCK_INFO(unit, blk).type;
         switch(blktype){
            case SOC_EA_BLK_PON:
                ponNum++;
                break;
            case SOC_EA_BLK_GE:
                geNum++;
                break;
            case SOC_EA_BLK_FE:
                feNum++;
                break;
            case SOC_EA_BLK_LLID:
                llidNum++;
                break;
         }
     }

    tot_bytes = 0;
    n_bytes = sizeof(soc_ea_counter_t);
    counter = sal_alloc(n_bytes, "ea counter");
    if (NULL == counter) {
        goto error;
    }
    soc->counter_buf64 = (uint64 *)counter;
    tot_bytes = 0;

    n_bytes = 0;
    n_bytes = ponNum * sizeof(_soc_ea_pon_counter_group_t);
    assert(n_bytes > 0);

    counter->pon_counter = sal_alloc(n_bytes, "pon counter");
    if(counter->pon_counter == NULL){
        goto error;
    }
	sal_memset(counter->pon_counter, 0, n_bytes);
    tot_bytes += n_bytes;
    
    n_bytes = 0;
    n_bytes = geNum * sizeof(_soc_ea_uni_counter_group_t);
    assert(n_bytes > 0);

    counter->ge_counter = sal_alloc(n_bytes, "ge counter");
    if(counter->ge_counter == NULL){
        goto error;
    }
	sal_memset(counter->ge_counter, 0, n_bytes);
    tot_bytes += n_bytes;

    n_bytes = 0;
    n_bytes = feNum * sizeof(_soc_ea_uni_counter_group_t);
    assert(n_bytes > 0);

    counter->fe_counter = sal_alloc(n_bytes, "fe counter");
    if(counter->fe_counter == NULL){
        goto error;
    }
	sal_memset(counter->fe_counter, 0, n_bytes);
    tot_bytes += n_bytes;

    n_bytes = 0;
    n_bytes = llidNum * sizeof(_soc_ea_llid_counter_group_t);
    assert(n_bytes > 0);

    counter->llid_counter = sal_alloc(n_bytes, "llid counter");
    if(counter->llid_counter == NULL){
        goto error;
    }
	sal_memset(counter->llid_counter, 0, n_bytes);
    tot_bytes += n_bytes;

    return SOC_E_NONE;
error:
    if (counter != NULL) {
        if (counter->pon_counter != NULL) {
            sal_free(counter->pon_counter);
            counter->pon_counter = NULL;
        }

        if (counter->ge_counter != NULL) {
            sal_free(counter->ge_counter);
            counter->ge_counter = NULL;
        }

        if (counter->fe_counter != NULL) {
            sal_free(counter->fe_counter);
            counter->fe_counter = NULL;
        }

        if (counter->llid_counter != NULL){
            sal_free(counter->llid_counter);
            counter->llid_counter = NULL;
        }
        
        sal_free(counter);
        soc->counter_buf64 = NULL;
    }

    return SOC_E_MEMORY;
}
 
 /*
  * Function:
  *      soc_ea_counter_detach
  * Purpose:
  *      Finalize counter module.
  * Notes:
  *      Stops counter task if running.
  *      Deallocates counter collection buffers.
  */
 int
 _soc_ea_counter_detach(int unit)
 {
    soc_control_t          *soc;
    soc_ea_counter_t       *counter = NULL;

    assert(SOC_UNIT_VALID(unit));

    soc = SOC_CONTROL(unit);

    counter = (soc_ea_counter_t *)soc->counter_buf64;

    if (counter != NULL) {
        if (counter->pon_counter != NULL) {
           sal_free(counter->pon_counter);
           counter->pon_counter = NULL;
        }

        if (counter->ge_counter != NULL) {
            sal_free(counter->ge_counter);
            counter->ge_counter = NULL;
        }

        if (counter->fe_counter != NULL) {
            sal_free(counter->fe_counter);
            counter->fe_counter = NULL;
        }

        if (counter->llid_counter != NULL) {
            sal_free(counter->llid_counter);
            counter->llid_counter = NULL;
        }

        sal_free(counter);
        soc->counter_buf64 = NULL;
    }

    return SOC_E_NONE;
}

 /*
 * Function:
 *  soc_ea_counter_sync
 * Purpose:
 *  Force an immediate counter update
 * Parameters:
 *  unit - uint number.
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *  Ensures that ALL counter activity that occurred before the sync
 *  is reflected in the results of any soc_ea_counter_get()-type
 *  routine that is called after the sync.
 */

int
_soc_ea_counter_sync(int unit)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
    int rv = SOC_E_NONE;
    soc_ea_counter_t    *counter = NULL; 
    soc_port_t  phy_port;
	soc_port_t  ponNum;
    soc_port_t  geNum;
    soc_port_t  feNum;
    soc_port_t  llidNum;
	int         blk;
    int         bindex;
    int         blktype;
    
    assert(SOC_UNIT_VALID(unit));
 
    soc = SOC_CONTROL(unit);

    if (!SOC_IS_EA(unit)) {
        return SOC_E_UNIT;
    }

    counter = (soc_ea_counter_t *)soc->counter_buf64;
     
    if (counter != NULL) {
        if (counter->pon_counter == NULL 
            || counter->ge_counter == NULL
            || counter->fe_counter == NULL
            || counter->llid_counter == NULL){
            return SOC_E_UNAVAIL;
        }
    } else {
        return SOC_E_UNAVAIL;
    }

	ponNum = 0;
	geNum = 0;
	feNum = 0;
	llidNum = 0;

	for (phy_port = 0; phy_port < NUM_PORT(unit); phy_port++) {
		blk = SOC_PORT_INFO(unit, phy_port).blk;
	 	bindex = SOC_PORT_INFO(unit, phy_port).bindex;
	 	if (blk < 0 && bindex < 0) {                    /* end of list */
			rv = SOC_E_INTERNAL;
			break;
	 	}
	 
	 	blktype = SOC_BLOCK_INFO(unit, blk).type;
	 	switch(blktype){
	    	case SOC_EA_BLK_PON:
				if(SOC_IS_TK371X(unit)){
					rv = TkExtOamGetPonStatsGroup(unit, 0,
						(TkPonStatsGroup *)counter->pon_counter);
					if (SOC_E_NONE != rv) {
						return SOC_E_FAIL;
					}
				}
	        	ponNum++;
	        	break;
	    	case SOC_EA_BLK_GE:
				if(SOC_IS_TK371X(unit)){
					rv = TkExtOamGetUniStatsGroup(unit, 0, phy_port, 
                		&(((TkUniStatsGroup *)counter->ge_counter)[geNum]));
					if (SOC_E_NONE != rv) {
						return SOC_E_FAIL;
					}
				}
	        	geNum++;
				break;
	    	case SOC_EA_BLK_FE:
				if(SOC_IS_TK371X(unit)){
					rv = TkExtOamGetUniStatsGroup(unit, 0, phy_port, 
                		&(((TkUniStatsGroup *)counter->fe_counter)[feNum]));
					if (SOC_E_NONE != rv) {
						return SOC_E_FAIL;
					}
				}
	        	feNum++;
	        	break;
	    	case SOC_EA_BLK_LLID:
				if(SOC_IS_TK371X(unit)){
					rv = TkExtOamGetLlidStatsGroup(unit, 0, 
						phy_port - SOC_PORT_MIN(unit,llid), 
                		&(((TkLlidStatsGroup *)counter->llid_counter)[llidNum]));
					if (SOC_E_NONE != rv) {
						return SOC_E_FAIL;
					}
				}
	        	llidNum++;
	        	break;
	 	}
	}
    return rv;
}

/*
 * Function:
 *  _soc_ea_port_counter_get
 * Purpose:
 *  Get the port counter
 * Parameters:
 *  unit - uint number.
 *  port - pon, ge,fe
 *  count - union data structure pointer
 * Returns:
 *  SOC_E_XXX
 * Notes:
 */
int 
_soc_ea_port_counter_get(int unit, int port, 
    _soc_ea_port_counter_t *count)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
    soc_ea_counter_t    *counter = NULL; 
    soc_port_t  phy_port;
    int index = 0;
    
    assert(SOC_UNIT_VALID(unit));

    if(NULL == count){
        return SOC_E_PARAM;
    }
 
    soc = SOC_CONTROL(unit);

    if (!SOC_IS_EA(unit)) {
        return SOC_E_UNIT;
    }

    counter = (soc_ea_counter_t *)soc->counter_buf64;

    if(NULL == counter){
        return SOC_E_UNAVAIL;
    }

    if(NULL == counter->fe_counter 
        || NULL == counter->ge_counter
        || NULL == counter->fe_counter
        || NULL == counter->llid_counter){
        return SOC_E_UNAVAIL;
    }
    
    if(port >= SOC_PORT_MIN(unit,pon) && port <= SOC_PORT_MAX(unit,pon)){
        for (index = 0, phy_port = SOC_PORT_MIN(unit,pon); 
            phy_port <= SOC_PORT_MAX(unit,pon); phy_port++) {
            sal_memcpy(&(count->pon_counter), 
                &((_soc_ea_pon_counter_group_t *)counter->pon_counter)[index], 
                sizeof(_soc_ea_pon_counter_group_t));
        }
        return SOC_E_NONE;
    }

    if(port >= SOC_PORT_MIN(unit,ge) && port <= SOC_PORT_MAX(unit,ge)){
        for (index = 0, phy_port = SOC_PORT_MIN(unit,ge); 
            phy_port <= SOC_PORT_MAX(unit,ge); phy_port++) {
            sal_memcpy(&(count->uni_counter), 
                &((_soc_ea_uni_counter_group_t *)counter->ge_counter)[index], 
                sizeof(_soc_ea_uni_counter_group_t));
        }
        return SOC_E_NONE;
    }

    if(port >= SOC_PORT_MIN(unit,fe) && port <= SOC_PORT_MAX(unit,fe)){
        for (index = 0, phy_port = SOC_PORT_MIN(unit,fe); 
            phy_port <= SOC_PORT_MAX(unit,fe); phy_port++) {
            sal_memcpy(&(count->uni_counter), 
                &((_soc_ea_uni_counter_group_t *)counter->fe_counter)[index], 
                sizeof(_soc_ea_uni_counter_group_t));
        }
        return SOC_E_NONE;
    }

    return SOC_E_PORT;
}

/*
 * Function:
 *  _soc_ea_llid_counter_get
 * Purpose:
 *  Get the llid counter
 * Parameters:
 *  unit - uint number.
 *  llid - llid index
 *  count - union data structure pointer
 * Returns:
 *  SOC_E_XXX
 * Notes:
 */
int 
_soc_ea_llid_counter_get(int unit, int llid_id, 
    _soc_ea_port_counter_t *count)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
    soc_ea_counter_t    *counter = NULL;
    
    assert(SOC_UNIT_VALID(unit));

    if(NULL == count){
        return SOC_E_PARAM;
    }
 
    soc = SOC_CONTROL(unit);

    if (!SOC_IS_EA(unit)) {
        return SOC_E_UNIT;
    }

    counter = (soc_ea_counter_t *)soc->counter_buf64;

    if(NULL == counter){
        return SOC_E_UNAVAIL;
    }

    if(NULL == counter->fe_counter 
        || NULL == counter->ge_counter
        || NULL == counter->fe_counter
        || NULL == counter->llid_counter){
        return SOC_E_UNAVAIL;
    }

    if(llid_id >= SOC_PORT_NUM(unit,llid)){
        return SOC_E_PARAM;
    }

    sal_memcpy(&(count->llid_counter), 
                &((_soc_ea_llid_counter_group_t *)counter->llid_counter)[llid_id], 
                sizeof(_soc_ea_llid_counter_group_t));
    
    return SOC_E_NONE;
}

