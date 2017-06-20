/*
 * $Id: sw_an.c $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * SW AN thread is an optional feature to support 
 * consortium AN mode on devices using PM4x25 D0. THis module is responsible for 
 * driving the Page exchange An state machine as recomennded by IEEE802.3by/MSA spec
 * Once the page exchange is completed and speed is resolved HW will take over and complete
 * the Autoneg.  
 */

#include <shared/bsl.h>
#ifdef SW_AUTONEG_SUPPORT 

#include <sal/types.h>
#include <shared/alloc.h>
#include <sal/core/spl.h>
#include <soc/drv.h>

#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm_int/esw/port.h>
#include <bcm_int/common/sw_an.h>

#include <soc/phy/phymod_ctrl.h>
#include <soc/phy/phyctrl.h>

#include <sal/appl/io.h>



/* 
 * SW AN Port context
 */

typedef struct _sw_an_port_ctxt_s {
	bcm_port_sw_an_event_t    an_event;
	phymod_sw_an_ctxt_t       phymod_sw_an_ctxt;
	int                       an_retry;
	uint32_t                  an_state;
} _sw_an_port_ctxt_t;

/*
 * AW AN module context structure 
 */
typedef struct _sw_autoneg_ctxt_s {
	int                     enable; /* enable SW AN Thread */ 
    sal_mutex_t	  		    lock; /* lock to access the port bitmap */
	sal_sem_t       		signal_event; /* semaphore to signal new event to AN thread */
	sal_thread_t	  		thread_id; /* Therad id */
	char                    thread_name[SAL_THREAD_NAME_MAX_LEN]; 
	pbmp_t          		add_ports; /* bitmap of ports that are part of the SW AN */
	pbmp_t                  process_ports; /* list of ports currently serviced by SW AN */
	_sw_an_port_ctxt_t      port_sw_an_ctxt[SOC_MAX_NUM_PORTS]; /* per port SW AN context */
	uint32_t                polling_interval; /* AN thread polling intervakl in msec */
} _sw_autoneg_ctxt_t;

/* static alocation of SW AN CTXT for all units */
_sw_autoneg_ctxt_t    *_sw_autoneg_ctxt[BCM_LOCAL_UNITS_MAX];

#define SW_AN_CTXT(unit)    (_sw_autoneg_ctxt[unit])

typedef enum _sw_an_page_exchange_sm_cmd_e {
	SW_AN_PAGE_EXCHANGE_SM_CMD_STOP = 0,
	SW_AN_PAGE_EXCHANGE_SM_CMD_START,
	SW_AN_PAGE_EXCHANGE_SM_CMD_MAX
} _sw_an_page_exchange_sm_cmd_t;

/*
 * Define:
 *     SW_AN_LOCK/SW_AN_UNLOCK
 * Purpose:
 *     Serialization Macros for access to _sw_autoneg_ctxt_t structure.
 */

#define SW_AN_LOCK(unit) \
        sal_mutex_take(_sw_autoneg_ctxt[unit]->lock, sal_mutex_FOREVER)

#define SW_AN_UNLOCK(unit) \
        sal_mutex_give(_sw_autoneg_ctxt[unit]->lock)

#define SW_AN_WAIT_FOR_EVENT(unit) \
		sal_sem_take(_sw_autoneg_ctxt[unit]->signal_event, sal_sem_FOREVER)

#define SW_AN_POST_EVENT(unit) \
		sal_sem_give(_sw_autoneg_ctxt[unit]->signal_event)		        

#define UNIT_VALID_CHECK(unit) \
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) { return BCM_E_UNIT; }

#define UNIT_INIT_CHECK(unit) \
    do { \
        UNIT_VALID_CHECK(unit); \
        if (_sw_autoneg_ctxt[unit] == NULL) { return BCM_E_INIT; } \
    } while (0)


#define SW_AN_THREAD_PRIORITY 50 /* 0 the highest and 255 the lowest*/
#define SW_AN_NUM_PAGES_TO_TX 0x3 

#define SW_AN_POLLING_INTERVAL 50

static int _bcm_sw_an_page_exchange_sm(int unit, bcm_port_t port, bcm_sw_an_fsm_event_t event);
int _bcm_sw_an_event_get(int unit, bcm_port_t port, bcm_sw_an_fsm_event_t *event);
phymod_phy_access_t * _bcm_sw_an_phy_access_get(int unit, bcm_port_t port);

int 
bcm_sw_an_module_deinit(int unit) 
{
    _sw_autoneg_ctxt_t    *sw_an_ctxt;

	UNIT_VALID_CHECK(unit);

	sw_an_ctxt = SW_AN_CTXT(unit);

	if (sw_an_ctxt == NULL) {
		return BCM_E_NONE;
	}

	if (sw_an_ctxt->signal_event != NULL) {
		sal_sem_destroy(sw_an_ctxt->signal_event);
		sw_an_ctxt->signal_event = NULL;
	}

	if (sw_an_ctxt->lock != NULL) {
		sal_mutex_destroy(sw_an_ctxt->lock);
	}

    /* sal_thread_destroy is supported only with netbsd
     * thread is should be NULL once we reach this 
     * function. Incase if we delete a thread while its holding lock
     * we may have unpredicatble behaviour in non linux machines  
     */
    if (sw_an_ctxt->thread_id != NULL) {
    	sal_thread_destroy(sw_an_ctxt->thread_id);
    }
	sal_free(sw_an_ctxt);

	SW_AN_CTXT(unit) = NULL;

	return BCM_E_NONE;
}

int 
bcm_sw_an_module_init(int unit) 
{
	_sw_autoneg_ctxt_t    *sw_an_ctxt;
	uint32                size;
	uint32                polling_interval;
	int                   rv = BCM_E_NONE;


	UNIT_VALID_CHECK(unit);

	if (SW_AN_CTXT(unit) != NULL) {
		/* free the allocated resource */
		rv = bcm_sw_an_module_deinit(unit);
		if (rv < BCM_E_NONE) {
			return rv;
		}
	}

	size = sizeof(_sw_autoneg_ctxt_t);
	if ((sw_an_ctxt = sal_alloc(size, "sw_an_module")) == NULL) {
		return BCM_E_MEMORY;
	}
	sal_memset(sw_an_ctxt, 0, size);

	/* initalize the mutex and semaphore */
	sw_an_ctxt->lock = sal_mutex_create("SW_AN_LOCK");
	if (sw_an_ctxt->lock == NULL) {
		rv = BCM_E_MEMORY;
	}

	if (BCM_SUCCESS(rv)) {
		sw_an_ctxt->signal_event = sal_sem_create("SW_AN_signal_event",
												  sal_sem_BINARY,
											      0);
		if (sw_an_ctxt->signal_event == NULL) {
			sal_mutex_destroy(sw_an_ctxt->lock);
			rv = BCM_E_MEMORY;
		}
	}

	if (BCM_FAILURE(rv)) {
		sal_free(sw_an_ctxt);
	}

	/* initialize the port bitmap */
	BCM_PBMP_CLEAR(sw_an_ctxt->add_ports);
	BCM_PBMP_CLEAR(sw_an_ctxt->process_ports);

	sw_an_ctxt->thread_id = NULL;

	polling_interval = SW_AN_POLLING_INTERVAL;
	polling_interval = soc_property_get(unit, spn_SW_AUTONEG_POLLING_INTERVAL, 
						polling_interval);

	sw_an_ctxt->polling_interval = polling_interval;

    _sw_autoneg_ctxt[unit] = sw_an_ctxt;

    return BCM_E_NONE;
}

/* Run the through each port in the bit map and if they are
 * scheduled to run service that port 
 */
STATIC void
_bcm_sw_an_thread(int unit) 
{
	_sw_autoneg_ctxt_t    *sw_an_ctxt;
	_sw_an_port_ctxt_t    *port_an_ctxt;
	bcm_sw_an_fsm_event_t event; 
    bcm_port_t     		  port;
    int                   rv = BCM_E_NONE;
    int                   is_process = 0;

    sw_an_ctxt = SW_AN_CTXT(unit);
    if (sw_an_ctxt == NULL) {
  	    sal_thread_exit(0);
  	    return;
    }

    sw_an_ctxt->thread_id = sal_thread_self();

    while(sw_an_ctxt->enable) {
    	SW_AN_LOCK(unit);
    	SOC_PBMP_ASSIGN(sw_an_ctxt->process_ports,
    					sw_an_ctxt->add_ports);
    	SW_AN_UNLOCK(unit);
    	/* run through the port bitmap */
    	do 
    	{
    		is_process = 0;
	    	PBMP_ITER(sw_an_ctxt->process_ports, port) {
	    		/* check if the port needs to be serviced */
	    		port_an_ctxt = &(sw_an_ctxt->port_sw_an_ctxt[port]);
	    		if (port_an_ctxt->an_event != BCM_PORT_SW_AN_EVENT_AN_STOP) {
	    			is_process = 1;
		    		switch(port_an_ctxt->an_event) {
		    			case BCM_PORT_SW_AN_EVENT_AN_START:
		  				    PORT_LOCK(unit);
		    				rv = _bcm_sw_an_page_exchange_sm(unit, port, BCM_FSM_SW_AN_EVT_START);
		   				    PORT_UNLOCK(unit);
		    				if (rv < BCM_E_NONE) {
		    					SW_AN_LOCK(unit);
		    					port_an_ctxt->an_event = BCM_PORT_SW_AN_EVENT_AN_DOWN;
		    					SW_AN_UNLOCK(unit);
		    					/* log error */
		    				}
		    				break;
						case BCM_PORT_SW_AN_EVENT_AN_RESTART:
						case BCM_PORT_SW_AN_EVENT_AN_LINK_DOWN: 
							/* update the an retry count */
							port_an_ctxt->an_retry++;
							PORT_LOCK(unit);
							rv = _bcm_sw_an_page_exchange_sm(unit, port, BCM_FSM_SW_AN_EVT_SEND_RESTART);
		   				    PORT_UNLOCK(unit);
							if (rv < BCM_E_NONE) {
								SW_AN_LOCK(unit);
		    					port_an_ctxt->an_event = BCM_PORT_SW_AN_EVENT_AN_DOWN;
		    					SW_AN_UNLOCK(unit);
								/* log error */
							}
							break;
						case BCM_PORT_SW_AN_EVENT_AN_IN_PROCESS:
							PORT_LOCK(unit);
							_bcm_sw_an_event_get(unit, port, &event);
							rv = _bcm_sw_an_page_exchange_sm(unit, port, event);
		   				    PORT_UNLOCK(unit);
							if (rv < BCM_E_NONE) {
								SW_AN_LOCK(unit);
		    					port_an_ctxt->an_event = BCM_PORT_SW_AN_EVENT_AN_DOWN;
		    					SW_AN_UNLOCK(unit);
								/* log error */
							}
							break;
						case BCM_PORT_SW_AN_EVENT_AN_STOP:
							break;
						default:
							break;		    				
		    		}
	    		}
    		}
    		SW_AN_LOCK(unit);
    		SOC_PBMP_ASSIGN(sw_an_ctxt->process_ports,
    					sw_an_ctxt->add_ports);
    		SW_AN_UNLOCK(unit);
    		sal_msleep(sw_an_ctxt->polling_interval); 
    	} while(is_process);

    	/* wait on a semaphore */
    	SW_AN_WAIT_FOR_EVENT(unit);
    }

    sw_an_ctxt->thread_id = NULL;
    sal_thread_exit(0);
    return;
}



int 
bcm_sw_an_enable_set(int unit, int enable) 
{
	_sw_autoneg_ctxt_t   *sw_an_ctxt = NULL;
	int 				 rv          = BCM_E_NONE;
	soc_timeout_t   	 to;
	sal_usecs_t          wait_usec;

	/* Time to wait for thread to start/end (longer for BCMSIM) */
    wait_usec = (SAL_BOOT_BCMSIM || SAL_BOOT_QUICKTURN) ? 30000000 : 10000000;

    UNIT_INIT_CHECK(unit);

    sw_an_ctxt = SW_AN_CTXT(unit);

    if (enable) {

    	if (sw_an_ctxt->thread_id != NULL) {
    		/* wakeup the SW AN thread */
    		SW_AN_POST_EVENT(unit);
    		return BCM_E_NONE;
    	}

    	sw_an_ctxt->enable = enable;

		sal_snprintf(sw_an_ctxt->thread_name,
	                 sizeof (sw_an_ctxt->thread_name),
	                 "SW_AN.%0x",
	                 unit);

		if (sal_thread_create(sw_an_ctxt->thread_name,
                              SAL_THREAD_STKSZ,
                              SW_AN_THREAD_PRIORITY,
                              (void (*)(void*))_bcm_sw_an_thread,
                              INT_TO_PTR(unit)) == SAL_THREAD_ERROR) {
			rv = BCM_E_MEMORY;
		} else {
			/* wait for the timeout period to expire to 
			 * check if the thread creation is successful
			 */
			soc_timeout_init(&to, wait_usec, 0);

			while(sw_an_ctxt->thread_id == NULL) {
				if ((soc_timeout_check(&to))) {
					LOG_ERROR(BSL_LS_BCM_PORT,
                              (BSL_META_U(unit,
                                          "%s: Thread did not start\n"),
                               			  sw_an_ctxt->thread_name));
                    rv = BCM_E_INTERNAL;
                    break;
				}
			}

		}
    } else {
    	/* set the enable flag to 0 and wait for the 
    	 * the thread to exit
    	 */
    	sw_an_ctxt->enable = enable; 

    }   
    return rv;
}

int 
bcm_sw_an_enable_get(int unit, int *enable) 
{
	_sw_autoneg_ctxt_t   *sw_an_ctxt = NULL;

	UNIT_INIT_CHECK(unit);

    sw_an_ctxt = SW_AN_CTXT(unit);

    *enable = sw_an_ctxt->enable;

    return BCM_E_NONE;
}

int 
bcm_sw_an_post_event(int unit, bcm_port_t port, bcm_port_sw_an_event_t event) 
{
	_sw_autoneg_ctxt_t    *sw_an_ctxt;
	_sw_an_port_ctxt_t    *port_an_ctxt;

    UNIT_INIT_CHECK(unit);

    if (event <= BCM_PORT_SW_AN_EVENT_NONE ||
    	event >= BCM_PORT_SW_AN_EVENT_AN_MAX)  {
    	/* nothing to notify the thread */
    	return BCM_E_NONE;
    }

    sw_an_ctxt = SW_AN_CTXT(unit);

    if (!SOC_PBMP_MEMBER(sw_an_ctxt->add_ports, port)) {
    	/* given port is not part of SW AN process thread 
    	 * nothing to notify
    	 */
    	return BCM_E_NONE; 
    }

    port_an_ctxt = &(sw_an_ctxt->port_sw_an_ctxt[port]);

    SW_AN_LOCK(unit);
    port_an_ctxt->an_event = event;
    SW_AN_UNLOCK(unit);

    /* notify the SW AN thread */
    SW_AN_POST_EVENT(unit);

    return BCM_E_NONE;
}

int
bcm_sw_an_port_register(int unit, bcm_port_t port) 
{
	_sw_autoneg_ctxt_t    *sw_an_ctxt;
    int                   enable = 0;
    int                   rv = BCM_E_NONE;

	UNIT_INIT_CHECK(unit);

	sw_an_ctxt = SW_AN_CTXT(unit);

    if (!SOC_PBMP_MEMBER(PBMP_PORT_ALL(unit), port) || 
    	!SOC_PORT_VALID(unit, port) || !IS_PORT(unit, port)) {
    	return BCM_E_PORT;
	}

	 /* if AN Thread is not enabled enable the AN thread */
    rv = bcm_sw_an_enable_get(unit, &enable);
    if (rv != BCM_E_NONE) {
    	return rv;
    }

    if (!enable) {
    	rv = bcm_sw_an_enable_set(unit, 1);	
    	if (rv != BCM_E_NONE) {
    		return rv;
    	}
    }

    if (!SOC_PBMP_MEMBER(sw_an_ctxt->add_ports, port)) {
		SW_AN_LOCK(unit);
		BCM_PBMP_PORT_ADD(sw_an_ctxt->add_ports, port);
		SW_AN_UNLOCK(unit);

	    rv = bcm_sw_an_post_event(unit, port, BCM_PORT_SW_AN_EVENT_AN_START);
	    return rv;
	} else {
		/* SW AN is already enabled for this port  restart SW AN */
		rv = bcm_sw_an_post_event(unit, port, BCM_PORT_SW_AN_EVENT_AN_RESTART);
	}

	return BCM_E_NONE;	
}

int
bcm_sw_an_port_unregister(int unit, bcm_port_t port) 
{
	_sw_autoneg_ctxt_t    *sw_an_ctxt;
	phymod_phy_access_t   *phy = NULL;

	UNIT_INIT_CHECK(unit);

	sw_an_ctxt = SW_AN_CTXT(unit);

    if (!SOC_PBMP_MEMBER(sw_an_ctxt->add_ports, port) || 
    	!SOC_PORT_VALID(unit, port) || !IS_PORT(unit, port)) {
    	return BCM_E_PORT;
	}
	
	SW_AN_LOCK(unit);
	BCM_PBMP_PORT_REMOVE(sw_an_ctxt->add_ports, port);
	SW_AN_UNLOCK(unit);

	phy = _bcm_sw_an_phy_access_get(unit, port);
	if (NULL == phy) {
		return BCM_E_INTERNAL;
	}

	PHYMOD_IF_ERR_RETURN
		(phymod_phy_sw_autoneg_enable(phy, 0));

	return BCM_E_NONE;	
}

phymod_phy_access_t *
_bcm_sw_an_phy_access_get(int unit, bcm_port_t port) 
{
	phy_ctrl_t                *pc;
    soc_phymod_ctrl_t         *pmc;
    soc_phymod_phy_t          *phy;

    /* locate phy control */
    pc = INT_PHY_SW_STATE(unit, port);
    if (pc == NULL) {
        return NULL;
    }

    pmc = &pc->phymod_ctrl;

    /* only request autoneg on the first core */
    phy = pmc->phy[0];
    if (phy == NULL) {
        return NULL;
    }

    return (&phy->pm_phy);
}

int
_bcm_sw_an_event_get(int unit, bcm_port_t port, bcm_sw_an_fsm_event_t *event) 
{
	phymod_sw_an_ctrl_status_t		an_ctrl_status;
	phymod_phy_access_t             *phy = NULL;

	phymod_sw_an_ctrl_status_t_init(&an_ctrl_status);
	phy = _bcm_sw_an_phy_access_get(unit, port);
	if (NULL == phy) {
		return BCM_E_INTERNAL;
	}
	/* call the phymod API to get the event*/
	PHYMOD_IF_ERR_RETURN
		(phymod_phy_sw_an_control_status_get(phy, &an_ctrl_status));

	*event = BCM_FSM_SW_AN_EVT_WAIT;

	if (an_ctrl_status.seq_restart) {
		/* not handled */
	} else if (an_ctrl_status.page_req) {
		/* handled as part of lp_page_rdy*/
	} else if (an_ctrl_status.lp_page_rdy) {
		*event = BCM_FSM_SW_AN_EVT_LP_PAGE_RDY;
	} else if (an_ctrl_status.an_completed) {
		*event = BCM_FSM_SW_AN_EVT_PAGE_EX_COMPLETE;
	} else {
		/* do nothing */
	}
	return BCM_E_NONE;
}

int
_bcm_sw_an_page_exchange_sm(int unit, bcm_port_t port, bcm_sw_an_fsm_event_t event) 
{
	_sw_autoneg_ctxt_t    			 *sw_an_ctxt;
	_sw_an_port_ctxt_t               *port_an_ctxt;
	phymod_sw_an_ctxt_t              *phymod_an_ctxt;
	phymod_phy_access_t              *phy;
	int                              rv; 

	UNIT_INIT_CHECK(unit);

	sw_an_ctxt = SW_AN_CTXT(unit);
	rv    = BCM_E_NONE;

	phy = _bcm_sw_an_phy_access_get(unit, port);
    if (NULL == phy) {
    	return BCM_E_INTERNAL;
    }

    port_an_ctxt = &(sw_an_ctxt->port_sw_an_ctxt[port]);
    phymod_an_ctxt = &port_an_ctxt->phymod_sw_an_ctxt;

    port_an_ctxt->an_state |= 0x1 << event;

	switch(event) {
		case BCM_FSM_SW_AN_EVT_START:
		    phymod_an_ctxt->tx_pages_cnt = 0;
			phymod_an_ctxt->rx_pages_cnt = 0;
			phymod_an_ctxt->no_pages_to_be_txed = SW_AN_NUM_PAGES_TO_TX;
			PHYMOD_IF_ERR_RETURN
				(phymod_phy_sw_an_base_page_exchange_handler(phy, phymod_an_ctxt));
			bcm_sw_an_post_event(unit, port, BCM_PORT_SW_AN_EVENT_AN_IN_PROCESS);	
			break;
		case BCM_FSM_SW_AN_EVT_SEND_RESTART:
			phymod_an_ctxt->tx_pages_cnt = 0;
			phymod_an_ctxt->rx_pages_cnt = 0;
			phymod_an_ctxt->no_pages_to_be_txed = SW_AN_NUM_PAGES_TO_TX;
			/* Diable the SW AN to toggle SW AN bit */
			PHYMOD_IF_ERR_RETURN
				(phymod_phy_sw_autoneg_enable(phy, 0));
			PHYMOD_IF_ERR_RETURN
				(phymod_phy_sw_an_base_page_exchange_handler(phy, phymod_an_ctxt));
			bcm_sw_an_post_event(unit, port, BCM_PORT_SW_AN_EVENT_AN_IN_PROCESS);
			break;
		case BCM_FSM_SW_AN_EVT_LP_PAGE_RDY:
			PHYMOD_IF_ERR_RETURN
				(phymod_phy_sw_an_lp_page_rdy_handler(phy, phymod_an_ctxt));
			bcm_sw_an_post_event(unit, port, BCM_PORT_SW_AN_EVENT_AN_IN_PROCESS);	
			break;
		case BCM_FSM_SW_AN_EVT_LD_PAGE:
			/* do nothing */
			break;
		case BCM_FSM_SW_AN_EVT_PAGE_EX_COMPLETE:
				bcm_sw_an_post_event(unit, port, BCM_PORT_SW_AN_EVENT_AN_STOP);
			break;
		case BCM_FSM_SW_AN_EVT_WAIT:
			bcm_sw_an_post_event(unit, port, BCM_PORT_SW_AN_EVENT_AN_IN_PROCESS);
			break;	
		default:
			break;					
	}
    return rv;
}

int 
bcm_sw_an_advert_set(int unit, bcm_port_t port, phymod_autoneg_ability_t *ability) 
{
	_sw_autoneg_ctxt_t    			 *sw_an_ctxt;
	phymod_sw_an_ctxt_t              *phymod_an_ctxt;
	phymod_phy_access_t              *phy;
	uint8_t                          sw_an_mode = 0;

	UNIT_INIT_CHECK(unit);

	phy = _bcm_sw_an_phy_access_get(unit, port);
    if (NULL == phy) {
    	return BCM_E_INTERNAL;
    }

	sw_an_ctxt = SW_AN_CTXT(unit);
	phymod_an_ctxt = &(sw_an_ctxt->port_sw_an_ctxt[port].phymod_sw_an_ctxt);

	sw_an_mode = soc_property_port_get(unit, port,
                       spn_PHY_AN_C73, sw_an_mode);

	switch(sw_an_mode) {
		case SW_AN_MODE_CL73_MSA:
			SW_AN_LOCK(unit);
			phymod_an_ctxt->an_mode = phymod_AN_MODE_CL73_MSA;
			SW_AN_UNLOCK(unit);
			break;
		case SW_AN_MODE_MSA_ONLY:
			SW_AN_LOCK(unit);
			phymod_an_ctxt->an_mode = phymod_AN_MODE_MSA;
			SW_AN_UNLOCK(unit);
			break;
		default:
			/* Ideally we should not hit this case*/
			SW_AN_LOCK(unit);
			phymod_an_ctxt->an_mode = phymod_AN_MODE_CL73_MSA;
			SW_AN_UNLOCK(unit);
			break;
	}

	PHYMOD_IF_ERR_RETURN
		(phymod_phy_sw_an_advert_set(phy, ability, phymod_an_ctxt));

	return BCM_E_NONE;	
}

int
bcm_sw_an_port_diag(int unit, int port) 
{
	_sw_autoneg_ctxt_t    			 *sw_an_ctxt;
	phymod_sw_an_ctxt_t              *phymod_an_ctxt;
	_sw_an_port_ctxt_t               *port_an_ctxt;

	UNIT_INIT_CHECK(unit);

	sw_an_ctxt = SW_AN_CTXT(unit);
	phymod_an_ctxt = &(sw_an_ctxt->port_sw_an_ctxt[port].phymod_sw_an_ctxt);
	port_an_ctxt = &(sw_an_ctxt->port_sw_an_ctxt[port]);

    sal_printf("********** SW AN CONTEXT INFORMATION FOR THE UNIT:%0x PORT:%0x **********\n", unit, port);
    sal_printf("PORT SW AN STATE:        \t %0x\n", port_an_ctxt->an_state);
    sal_printf("PORT SW AN TX PAGES:     \t %0x\n", phymod_an_ctxt->tx_pages_cnt);
    sal_printf("PORT SW AN RX PAGES:     \t %0x\n", phymod_an_ctxt->rx_pages_cnt);
    
    sal_printf("PORT SW AN TX BASE PAGE: \t (48-32):%0x (31-16):%0x (15-0):%0x\n", 
    	phymod_an_ctxt->tx_pages.base_page.page_2, phymod_an_ctxt->tx_pages.base_page.page_1, 
    	phymod_an_ctxt->tx_pages.base_page.page_0);
        sal_printf("PORT SW AN TX MSG PAGE:  \t (48-32):%0x (31-16):%0x (15-0):%0x\n", 
    	phymod_an_ctxt->tx_pages.msg_page.page_2, phymod_an_ctxt->tx_pages.msg_page.page_1, 
    	phymod_an_ctxt->tx_pages.msg_page.page_0);
    sal_printf("PORT SW AN TX UP PAGE:   \t (48-32):%0x (31-16):%0x (15-0):%0x\n", 
    	phymod_an_ctxt->tx_pages.ufmt_page.page_2, phymod_an_ctxt->tx_pages.ufmt_page.page_1, 
    	phymod_an_ctxt->tx_pages.ufmt_page.page_0);

    sal_printf("PORT SW AN RX BASE PAGE: \t (48-32):%0x (31-16):%0x (15-0):%0x\n", 
    	phymod_an_ctxt->rx_pages.base_page.page_2, phymod_an_ctxt->rx_pages.base_page.page_1, 
    	phymod_an_ctxt->rx_pages.base_page.page_0);
    sal_printf("PORT SW AN RX MSG PAGE:  \t (48-32):%0x (31-16):%0x (15-0):%0x\n", 
    	phymod_an_ctxt->rx_pages.msg_page.page_2, phymod_an_ctxt->rx_pages.msg_page.page_1, 
    	phymod_an_ctxt->rx_pages.msg_page.page_0);
    sal_printf("PORT SW AN RX UP PAGE:   \t (48-32):%0x (31-16):%0x (15-0):%0x\n", 
    	phymod_an_ctxt->rx_pages.ufmt_page.page_2, phymod_an_ctxt->rx_pages.ufmt_page.page_1, 
    	phymod_an_ctxt->rx_pages.ufmt_page.page_0);

    sal_printf("PORT SW AN  EXT PAGE:    \t (48-32):%0x (31-16):%0x (15-0):%0x\n", 
    	phymod_an_ctxt->tx_pages.null_page.page_2, phymod_an_ctxt->tx_pages.null_page.page_1, 
    	phymod_an_ctxt->tx_pages.null_page.page_0);
    sal_printf("PORT SW AN  EXT PAGE:    \t (48-32):%0x (31-16):%0x (15-0):%0x\n", 
    	phymod_an_ctxt->rx_pages.null_page.page_2, phymod_an_ctxt->rx_pages.null_page.page_1, 
    	phymod_an_ctxt->rx_pages.null_page.page_0);

    return BCM_E_NONE;
}
#endif /* SW_AUTONEG_SUPPORT */

