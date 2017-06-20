/*
 * $Id: auth.c,v 1.60 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * 802.1x Port-Based Network Access Control for RoboSwitch
 * Purpose: API to set 802.1x registers.
 *
 *
 * 802.1x is a port based authentication protocol. If a user port 
 * (supplicant) want to get service from another port(authenticator),
 * it must get approved by authenticator. Authenticator usually will
 * pass authentication protocol (EAP) to an authentication server,
 * which has all security information.
 * EAP(Extensive Authentication Protocol) is the higher layer protocol
 * used for authentication purpose. In order for layer 2 ports to 
 * participate in EAP protocol more efficiently, 802.1x created another
 * layer 2 protocol called EAPOL ( EAP over LAN). With EAPOL, layer2 can
 * initiate/stop authentication functions by itself.
 * So basically, if a port want to get service from another port, 
 * it need be authenticated by that port. EAPOL is the protocol used by
 * authentication process.
 *
 * Basically we need to detect EAPOL frame and pass it to CPU(MII) port.
 * After reset, if EAP_EN=1, and EAP_BLK_MODE=1, we will block all 
 * traffic except EAPOL and other special frames if we specify. 
 * After authentication process finished, we will add some limitations
 * to each port based on result of authentication.
 *
 * The way to detect EAPOL traffic:
 * 1) DA requirement:
 *    Bcast DA: DA=48'h0180c2000003
 *    or Ucast DA: DA = EAP_UNI_DA if enabled.
 *
 * 2) Ethernet type : 16'h888E (only in 802.3/Ethernet frame)
 *
 * 3) Packet type: 
 *    EAP_packet(8'h00)
 *    EAPOL_start(8'h01)
 *    EAPOL_logoff(8'h02)
 *    EAPOL_key(8'h03)
 *    EAPOL_Encapsulated_ASF_Alert(8'h04)
 *
 * 4)non_802.1q frame (un_tagged, or pri_tagged frame are OK)
 *
 */
#include <shared/bsl.h>

#include <sal/types.h>

#include <soc/debug.h>
#include <soc/drv.h>

#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/l2.h>
#include <bcm/auth.h>
#include <bcm/link.h>

#include <bcm_int/robo/auth.h>

typedef struct auth_mac_s *auth_mac_p;

typedef struct auth_mac_s {
    bcm_mac_t       mac;  
    auth_mac_p      next;
} auth_mac_t;

typedef struct auth_s
{
    uint32      flags; /* Store the current Auth. mode(ex. BCM_AUTH_XXX) */
    int            secMacNum;  /* Total valid security mac */
    int            dynaMacNum; /* The dynamic mac address threshold */
    int            secMode;    /* The security mode */
    auth_mac_t  *macList;  /* security mac */
} auth_t;

typedef struct auth_info_s
{
    int    init; /* TRUE if 802.1x module has been inited */
    auth_t portAuth[SOC_MAX_NUM_PORTS]; /* port auth info */
} auth_info_t;

typedef struct auth_cb_cntl_s {
    int             registered;
    bcm_auth_cb_t   auth_cbs;
    void            *auth_cb_data;
} auth_cb_cntl_t;

static auth_info_t robo_auth_info[BCM_MAX_NUM_UNITS];
static auth_cb_cntl_t cb_cntl[BCM_MAX_NUM_UNITS];

static bcm_pbmp_t auth_pbmp[BCM_MAX_NUM_UNITS];

#define AUTH_INIT(unit)                    \
        if (!robo_auth_info[unit].init)            \
        return BCM_E_INIT

/* Valid port verification :
 *     - GE port in Robo Chip(bcm5324) are not supported for EAP function. So we 
 *      have to exclude GE port from initial routine.
 */
#define AUTH_PORT(unit, port) \
        if (!SOC_PORT_VALID(unit, port) || !IS_E_PORT(unit, port) \
            ||!PBMP_MEMBER(auth_pbmp[unit], port)) \
        { return (BCM_E_PORT); }



/*
 * Check if 802.1x supported by auth_pbmp which is gotten from driver layer.
 * Value 0 means not supported.
 */
#define AUTH_SUPPORT(unit)    \
        if (!SOC_PBMP_WORD_GET(auth_pbmp[unit], 0))    \
        return BCM_E_UNAVAIL

STATIC void _robo_auth_linkscan_cb(int unit, bcm_port_t port, bcm_port_info_t *info);

/*
 * Function:
 *      _robo_auth_maclist_lookup
 * Description:
 *      Lookup a MAC address in the list
 * Parameters:
 *      list - list to be lookuped 
 *      mac -  MAC address
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

STATIC int
_robo_auth_maclist_lookup(auth_mac_t **list, bcm_mac_t mac)
{
    while (*list != NULL) {
        if (!sal_memcmp((*list)->mac, mac, sizeof(bcm_mac_t))) { 
            return TRUE;
        }
        list = &(*list)->next;
    }

    return FALSE;
}

/*
 * Function:
 *      _robo_auth_maclist_insert 
 * Description:
 *      Add a MAC address to the list 
 * Parameters:
 *      list - list to be inserted 
 *      mac -  MAC address 
 *      ins -  (OUT) entry inserted  
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

STATIC int
_robo_auth_maclist_insert(auth_mac_t **list, bcm_mac_t mac, auth_mac_p *ins)
{
    auth_mac_p    entry;
    int rv;

    if ((rv = _robo_auth_maclist_lookup(list, mac)) > 0) {
        return BCM_E_EXISTS;
    } 

    if ((entry = sal_alloc(sizeof(auth_mac_t), "maclist")) == NULL) {
        return BCM_E_MEMORY;
    }

    sal_memset(entry, 0, sizeof(auth_mac_t));
    sal_memcpy(entry->mac, mac, sizeof(bcm_mac_t)); 
    entry->next = *list;
    *list = entry;
    *ins = entry;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _robo_auth_maclist_remove
 * Description:
 *      Remove a MAC address from the list
 * Parameters:
 *      list - list to be removed 
 *      mac -  MAC address
 *      ins -  (OUT) entry deleted 
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

STATIC int
_robo_auth_maclist_remove(auth_mac_t **list, bcm_mac_t mac, auth_mac_p *del)
{
    auth_mac_p    entry;

    while (*list != NULL) {
        if (!sal_memcmp((*list)->mac, mac, sizeof(bcm_mac_t))) { 
            entry = *list;
            *list = entry->next;
            *del = entry;
            return BCM_E_NONE;
        }
        list = &(*list)->next;
    }

    return BCM_E_NOT_FOUND;
}


/*
 * Function:
 *      _robo_auth_maclist_destroy
 * Description:
 *      Destroy all MAC addresses in the list
 * Parameters:
 *      list - list to be destroyed 
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

STATIC int
_robo_auth_maclist_destroy(auth_mac_t **list)
{
    auth_mac_p entry;

    if (*list == NULL) {
        return BCM_E_EMPTY;
    }

    sal_memset(&entry, 0, sizeof(auth_mac_p));
    while (*list != NULL) {
        _robo_auth_maclist_remove(list, (*list)->mac, &entry);
        sal_free(entry);
    }
    return BCM_E_NONE;
}

#ifdef AUTH_DEBUG
/*
 * Function:
 *      _auth_maclist_dump
 * Description:
 *      Dump all MAC addresses in the list
 * Parameters:
 *      list - list to be dumped 
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

STATIC int
_auth_maclist_dump(auth_mac_t **list)
{
    while (*list != NULL) {
        LOG_CLI((BSL_META("%02x:%02x:%02x:%02x:%02x:%02x\n"),
                 (*list)->mac[0], (*list)->mac[1],
                 (*list)->mac[2], (*list)->mac[3],
                 (*list)->mac[4], (*list)->mac[5]));
        list = &(*list)->next;
    }

    return BCM_E_NONE;
}
#endif

/*
 * Function:
 *     bcm_robo_auth_init
 * Purpose:
 *     initialize any internal state for the auth module on the given unit.
 *     All ports are marked as being in the uncontrolled state.
 * Parameters:
 *    unit - RoboSwitch unit number.
 * Returns:
 *     BCM_E_NONE
 * Notes:
 *     - Now, the MII port is not included in the 802.1x function ports.
 *     - GE port in Robo Chip(bcm5324) are not supported for EAP function. So we 
 *      have to exclude GE port from initial routine.
 */
int 
bcm_robo_auth_init(int unit)
{
    bcm_pbmp_t  bmp_all;
    bcm_port_t  port;
    uint32 value[SOC_PBMP_WORD_MAX];
    int i;
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_auth_init()..\n")));
            
    /* init the valid portBitMap for authenticating feature */
    BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_AUTH_PBMP, &value[0]));

    for (i=0; i<SOC_PBMP_WORD_MAX; i++){
        SOC_PBMP_WORD_SET(auth_pbmp[unit], i, value[i]);
    }
    AUTH_SUPPORT(unit);
    /*
    1) Disable the 802.1x function for all ports. Let all ports
       in uncontrolled state.
       Set EAP_EN=0 and EAP_BLK_MODE=0 in 
       PORT_EAP_CONr(page-40h/addr-00h).
    2) Disable the security function for all ports.
       Set MAC_SEC_CON_Px=0 in MAC_SECU_MACSEC_CTRL0r 
       and MAC_SECU_MACSEC_CTRL1r.
    3) Make sure TX/RX enable packets in uncontrolled.
       Set RX_DISABLE=0 and TX_DISABLE=0 in GBL_CTRL_FE_PCTRLr.
    */

    sal_memset(&robo_auth_info[unit], 0, sizeof(auth_info_t));
    
    bmp_all = PBMP_E_ALL(unit);
    BCM_PBMP_AND(bmp_all, auth_pbmp[unit]);
    
    /* init custom EAP feature on all port :
     *  - bcm53115/53118/5395/53242/53262/5348/5347 support custom EAP_DA 
     *      feature. 
     *  - init value is disable this port basis feature.
     */
    if (SOC_IS_ROBO_ARCH_VULCAN(unit) || SOC_IS_ROBO53242(unit) || 
        SOC_IS_ROBO53262(unit)){
        bcm_mac_t   temp_mac = {0,0,0,0,0,0};
        BCM_IF_ERROR_RETURN(DRV_MAC_SET(
                unit, bmp_all, DRV_MAC_CUSTOM_EAP, temp_mac, 0));
    }

    
    BCM_IF_ERROR_RETURN(DRV_SECURITY_SET
        (unit, bmp_all, 
        DRV_SECURITY_PORT_UNCONTROLLED, 
        DRV_SECURITY_VIOLATION_NONE));

    /* Init the S/W info */
    robo_auth_info[unit].init = TRUE;
    PBMP_ITER(bmp_all, port){
        robo_auth_info[unit].portAuth[port].flags = 
                            BCM_AUTH_MODE_UNCONTROLLED;
    }
    if (!cb_cntl[unit].registered) {
        BCM_IF_ERROR_RETURN(
            bcm_linkscan_register(unit, _robo_auth_linkscan_cb)); 
        cb_cntl[unit].registered = TRUE;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_robo_auth_detach
 * Purpose:
 *     Stop all auth module processing. All ports are moved to the
 *     uncontrolled state. All internal callbacks are removed.
 * Parameters:
 *    unit - RoboSwitch unit number.
 * Returns:
 *     BCM_E_NONE
 */
int 
bcm_robo_auth_detach(int unit)
{
    bcm_port_t  port;
    bcm_pbmp_t  t_pbm;
    int rv;

    AUTH_INIT(unit);
    
    AUTH_SUPPORT(unit);
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_auth_detach()..\n")));
    t_pbm = PBMP_E_ALL(unit);
    BCM_PBMP_AND(t_pbm, auth_pbmp[unit]);

    /* release each port's mac list */
    BCM_PBMP_ITER(t_pbm, port){
        _robo_auth_maclist_destroy(&robo_auth_info[unit].portAuth[port].macList);
    }
    
    /*
    1) Disable the 802.1x function for all ports. Let all ports
       in uncontrolled state.
       Set EAP_EN=0 in PORT_EAP_CONr.
    2) Disable the security function for all ports.
       Set MAC_SEC_CON_Px=0 in MAC_SECU_MACSEC_CTRL0r. 
       and MAC_SECU_MACSEC_CTRL1r.
    3) Make sure TX/RX enable packets in uncontrolled.
       Set RX_DISABLE=0 and TX_DISABLE=0 in GBL_CTRL_FE_PCTRLr.
    4) Remove all the internal callbacks.
    */

    BCM_IF_ERROR_RETURN(bcm_auth_init(unit));
    /* Remove all the internal callbacks. (will be implemented)*/
    
    cb_cntl[unit].auth_cbs = NULL;
    cb_cntl[unit].auth_cb_data = NULL;
    

    if (cb_cntl[unit].registered) {
        rv = bcm_linkscan_unregister(unit, _robo_auth_linkscan_cb); 
        /* If linkscan thread was restarted callback might be gone. */
        if ((BCM_FAILURE(rv)) && (BCM_E_NOT_FOUND != rv)) {
            return (rv);
        }
        cb_cntl[unit].registered = FALSE;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_robo_auth_mode_set
 * Purpose:
 *    Set the authentication mode and options for a port. If port is -1,
 *    the _set call applies to all non-stacking(non-uplink) ethernet 
 *    compatible ports.  Mode can be one of
 *      BCM_AUTH_MODE_UNCONTROLLED   uncontrolled state
 *      BCM_AUTH_MODE_UNAUTH         unauthorized state
 *      BCM_AUTH_MODE_AUTH           authorized state
 *     
 * Parameters:
 *    unit - RoboSwitch unit number.
 *    port - RoboSwitch port number.
 *    mode - The authentication mode(BCM_AUTH_MODE_XXX) for this port.
 * Returns:
 *     BCM_E_NONE
 */
int 
bcm_robo_auth_mode_set(int unit, int port, uint32 mode)
{
    bcm_pbmp_t      t_pbm;
    uint32          state, mask = 0;
    bcm_port_t  loc_port;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_auth_mode_set()..port=%d, mode=0x%x\n"),
                 port, mode));

    AUTH_INIT(unit);
    
    AUTH_SUPPORT(unit);

    if (BCM_GPORT_IS_SET(port)) {
        if ((BCM_GPORT_MODPORT_MODID_GET(port) == _SHR_GPORT_MODID_MASK) &&
            (BCM_GPORT_MODPORT_PORT_GET(port) == _SHR_GPORT_PORT_MASK)) {
            /* System configuration for gport type */
            loc_port = -1;
        } else {
            BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &loc_port));
            AUTH_PORT(unit, loc_port);
        }
    } else {
        loc_port = port;
    }

    if (loc_port < 0) { 
        t_pbm = PBMP_E_ALL(unit);
    } else {
        AUTH_PORT(unit, loc_port);
        BCM_PBMP_PORT_SET(t_pbm, loc_port);
    }

    if (mode & BCM_AUTH_MODE_UNCONTROLLED) {
        /*
             This port is uncontrolled port.
             Any packets may flow in or out of the port and 
             normal l2 learning takes place.
               1) Disable the 802.1x and port security function.
        2) Make sure the tx/rx enable for this port.
         */
        state = DRV_SECURITY_PORT_UNCONTROLLED;
        mask = 0;
                        
    } else if (mode & BCM_AUTH_MODE_UNAUTH) {
        /*
        
        We must Set EAP_EN=1 in PORT_EAP_CONr to enable 802.1x function.
        
        This port is controlled port, and the port is unauthorized.
        All l2 mac addresses associated with the port are removed,
        and l2 learning is disable.
        1) We can use bcm_l2_addr_remove_by_port() to remove l2 macs.
        2) For disabling l2 learning, we can set the port security mode
           to BCM_AUTH_SEC_SA_NUM, and set the DYN_MAX_MAC_NO_Px=0 in
           the MAC_SECU_PDYN_LNTHRSHr register for this port.
        
        There are two flags may be or'ed into this mode.
        1) BCM_AUTH_BLOCK_IN            allow outgoing packets
        2) BCM_AUTH_BLOCK_INOUT(default)    do not allow in or out packets
        
        While in this mode, packet transfer is blocked as specified by the
        BCM_AUTH_BLOCK_* flags.   
        
        IF BCM_AUTH_BLOCK_IN set, we can do the follows:
            1) Set the EAP_BLK_MODE=1 in PORT_EAP_CONr.
               (802.1x_special_frame will not be blocked)
            2) set RX_DISABLE=1 in TH_PCTLr.
        
        IF BCM_AUTH_BLOCK_INOUT set, we can do the follows:
            1) Set the EAP_BLK_MODE=1 in PORT_EAP_CONr.
               (802.1x_special_frame will not be blocked)
            2) set RX_DISABLE=TX_DISABLE=1 in TH_PCTLr.
        
        Outgoing EAPOL frames to be sent must be sent with a pair of
        bcm_auth_egress_set calls, enabling the egress before the
        EAPOL packet and disabling the egress after the packet is
        sent. If the BCM_AUTH_BLOCK_IN flag has been given, then the
        bcm_auth_egress_set calls will not do anything.
         */
        state = DRV_SECURITY_PORT_UNAUTHENTICATE;
        mask = DRV_SECURITY_VIOLATION_SA_NUM;

        if (mode & BCM_AUTH_BLOCK_IN) {
            mask |= DRV_SECURITY_BLOCK_IN;
        } else {
            /* Default is BCM_AUTH_BLOCK_INOUT */
            mode |= BCM_AUTH_BLOCK_INOUT;
            mask |= DRV_SECURITY_BLOCK_INOUT;
        }
        
    } else if (mode & BCM_AUTH_MODE_AUTH){

        /*
        We must Set EAP_EN=1 in PORT_EAP_CONr to enable 802.1x function.
        But set the EAP_BLK_MODE=0.
        
        There are three flags may be or'ed into this mode.
        1) BCM_AUTH_LEARN        allow l2 learning while authorized
        2) BCM_AUTH_IGNORE_LINK        do not unauthorize upon link down
        3) BCM_AUTH_IGNORE_VIOLATION    do not unauth upon security violation
           (How to implement in robo ?)
        
        IF BCM_AUTH_LEARN set, We can call the following APIs to set the 
        security/learning mode:
            1) bcm_auth_sec_mode_set().
            2) bcm_auth_sec_mac_add() if security mode is 
               BCM_AUTH_SEC_STATIC_XXX.
        
        IF BCM_AUTH_IGNORE_LINK is not set, and the link goes down on the port
        while in the authorized    state , then the port will be moved to the 
        unauthorized state.
        
        Currently, in ROBO, when violation occur, the packet will always 
        be dropped. So, how to implement this ????????
        How do we know the violation occur ????????
        How do we implement moving port from a authorized state to 
        unauthorized state when violation occur ????????
         */
        
        state = DRV_SECURITY_PORT_AUTHENTICATED;
        mask |= (mode & BCM_AUTH_LEARN) ? 
                        DRV_SECURITY_LEARN : 0;
        mask |= (mode & BCM_AUTH_IGNORE_LINK) ? 
                        DRV_SECURITY_IGNORE_LINK : 0;
        mask |= (mode & BCM_AUTH_IGNORE_VIOLATION) ? 
                        DRV_SECURITY_IGNORE_VIOLATION : 0;
                        
        mask |= (mode & BCM_AUTH_SEC_NONE) ? 
                        DRV_SECURITY_VIOLATION_NONE :
                (mode & BCM_AUTH_SEC_STATIC_ACCEPT) ? 
                        DRV_SECURITY_VIOLATION_STATIC_ACCEPT : 
                (mode & BCM_AUTH_SEC_STATIC_REJECT) ? 
                        DRV_SECURITY_VIOLATION_STATIC_REJECT : 
                (mode & BCM_AUTH_SEC_SA_NUM) ? 
                        DRV_SECURITY_VIOLATION_SA_NUM : 
                (mode & BCM_AUTH_SEC_SA_MATCH) ? 
                        DRV_SECURITY_VIOLATION_SA_MATCH : 
                (mode & BCM_AUTH_SEC_SA_MOVEMENT) ? 
                        DRV_SECURITY_VIOLATION_SA_MOVEMENT: 0;

        mask |= (mode & BCM_AUTH_SEC_EXTEND_MODE) ? 
                        DRV_SECURITY_EAP_MODE_EXTEND : 
                (mode & BCM_AUTH_SEC_SIMPLIFY_MODE) ? 
                        DRV_SECURITY_EAP_MODE_SIMPLIFIED : 0;
                
    } else {
        return BCM_E_PARAM;
    }

    if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        /* 
          * To indicate EAP PDU CPUCopy is enable (default) or 
          * not (drop EAP packet) for all ports (now only for TB)
          */
        mask |= (mode & BCM_AUTH_SEC_RX_EAP_DROP) ? 
                        DRV_SECURITY_RX_EAP_DROP : 0;
#endif
    } else {
        if (mode & BCM_AUTH_SEC_RX_EAP_DROP) {
            return BCM_E_UNAVAIL;
        }
    }

    BCM_IF_ERROR_RETURN(DRV_SECURITY_SET
        (unit, t_pbm, state, mask));

    /* update the s/w info */
    BCM_PBMP_ITER(t_pbm, loc_port) {
        robo_auth_info[unit].portAuth[loc_port].flags = mode;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_robo_auth_mode_get
 * Purpose:
 *    Get the authentication mode and options for a port. 
 *    Mode can be one of
 *      BCM_AUTH_MODE_UNCONTROLLED   uncontrolled state
 *      BCM_AUTH_MODE_UNAUTH         unauthorized state
 *      BCM_AUTH_MODE_AUTH           authorized state
 * Parameters:
 *    unit  - RoboSwitch unit number.
 *    port  - RoboSwitch port number.
 *    modep - The authentication mode(BCM_AUTH_MODE_XXX) for this port.
 * Returns:
 *     BCM_E_NONE
 */
int 
bcm_robo_auth_mode_get(int unit, int port, uint32 *modep)
{

    AUTH_INIT(unit);

    AUTH_SUPPORT(unit);

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_port_local_get(unit, port, &port));
    }
    AUTH_PORT(unit, port);

    *modep = robo_auth_info[unit].portAuth[port].flags;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_auth_mode_get()....port=%d, mode=0x%x\n"),
                 port, *modep));

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_robo_auth_unauth_callback
 * Purpose:
 *    Calls func when a port on the unit has been moved from
 *    authorized to unauthorized state.  Func's signature is:
 *    void func(int unit, int port, void *cookie, int reason)
 *    Reason can be one of:
 *        BCM_AUTH_REASON_UNKNOWN
 *        BCM_AUTH_REASON_LINK
 *        BCM_AUTH_REASON_VIOLATION
 * Parameters:
 *    unit   - RoboSwitch unit number.
 *    func   - callback func.
 *    cookie - cookie.
 * Returns:
 *     BCM_E_NONE
 */

int 
bcm_robo_auth_unauth_callback(int unit,
                    bcm_auth_cb_t func, void *cookie)
{
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_auth_unauth_callback()..\n")));

    AUTH_INIT(unit);

    cb_cntl[unit].auth_cbs = func;
    cb_cntl[unit].auth_cb_data = cookie;
  
  return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_robo_auth_egress_set
 * Purpose:
 *    Enable or disable the ability of packets to be sent out a
 *    particular port. This call should only be used around calls
 *    that a cpu uses to transmit an EAPOL frame out the port. If
 *    the port is in an unauthorized state with BCM_AUTH_BLOCK_IN
 *    set or is not in an unauthorized state, then this call does
 *    nothing.
 * Parameters:
 *    unit   - RoboSwitch unit number.
 *    port   - RoboSwitch port number.
 *    enable - 1 for enable tx; 0 for disable tx.
 * Returns:
 *     BCM_E_NONE
 */

int 
bcm_robo_auth_egress_set(int unit, int port, int enable)
{
    bcm_pbmp_t      t_pbm;
    uint32          status;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_auth_egress_set()..\n")));

    AUTH_INIT(unit);
    
    AUTH_SUPPORT(unit);

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_port_local_get(unit, port, &port));
    }
    AUTH_PORT(unit, port);
    
    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);
    
    status = (enable) ? TRUE : FALSE;
    BCM_IF_ERROR_RETURN(DRV_SECURITY_EGRESS_SET
        (unit, t_pbm, status));

    /* update the s/w info */
    if (enable) {
       if ((robo_auth_info[unit].portAuth[port].flags & BCM_AUTH_MODE_UNAUTH) &&
           !(robo_auth_info[unit].portAuth[port].flags & BCM_AUTH_BLOCK_IN)) {
           robo_auth_info[unit].portAuth[port].flags &= ~BCM_AUTH_BLOCK_INOUT;
           robo_auth_info[unit].portAuth[port].flags |= BCM_AUTH_BLOCK_IN;
       }
    }
    else {
       if ((robo_auth_info[unit].portAuth[port].flags & BCM_AUTH_MODE_UNAUTH) &&
           (robo_auth_info[unit].portAuth[port].flags & BCM_AUTH_BLOCK_IN)) {
           robo_auth_info[unit].portAuth[port].flags &= ~BCM_AUTH_BLOCK_IN;
           robo_auth_info[unit].portAuth[port].flags |= BCM_AUTH_BLOCK_INOUT;
        }
    }
   
    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_robo_auth_egress_get
 * Purpose:
 *    Get the egress status
 * Parameters:
 *    unit   - RoboSwitch unit number.
 *    port   - RoboSwitch port number.
 *    enable - 1 for enable tx; 0 for disable tx.
 * Returns:
 *     BCM_E_NONE
 */
int 
bcm_robo_auth_egress_get(int unit, int port, int *enable)
{
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_auth_egress_get()..\n")));

    AUTH_INIT(unit);
    
    AUTH_SUPPORT(unit);

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_port_local_get(unit, port, &port));
    }
    AUTH_PORT(unit, port);
    
    if (robo_auth_info[unit].portAuth[port].flags & BCM_AUTH_MODE_UNAUTH) {
        if (robo_auth_info[unit].portAuth[port].flags & BCM_AUTH_BLOCK_IN) {
            *enable = TRUE;
        } else {
            *enable = FALSE;
        }
    } else {
        *enable = FALSE;
    }
    
    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_robo_auth_mac_add
 * Purpose:
 *    Add a mac to the Static MAC Security Table for a specific port.
 * Parameters:
 *    unit - RoboSwitch unit number.
 *    port - RoboSwitch port number.
 *    mac  - Security MAC address.
 * Returns:
 *     BCM_E_NONE
 */
int 
bcm_robo_auth_mac_add(int unit, int port, bcm_mac_t mac)
{
    bcm_mac_t       mac_zero = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    bcm_mac_t       mac_resv = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x03};
    bcm_pbmp_t      t_pbm, pbm;
    auth_mac_p      entry;
    uint32          max_sec_mac = 0;
    bcm_port_t  p;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_auth_mac_add()..\n")));

    AUTH_INIT(unit);
    
    AUTH_SUPPORT(unit);

    if (BCM_GPORT_IS_SET(port)) {
        if ((BCM_GPORT_MODPORT_MODID_GET(port) == _SHR_GPORT_MODID_MASK) &&
            (BCM_GPORT_MODPORT_PORT_GET(port) == _SHR_GPORT_PORT_MASK)) {
            /* System configuration for gport type */
            p = -1;
        } else {
            BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &p));
            AUTH_PORT(unit, p);
        }
    } else {
        p = port;
    }

    if (p >= 0) {
        AUTH_PORT(unit, p);
    }

    if ((!sal_memcmp(mac, mac_zero, sizeof(bcm_mac_t))) ||
        (!sal_memcmp(mac, mac_resv, sizeof(bcm_mac_t)))) {
        return BCM_E_PARAM;
    }
    
    /* get the number of supported per port Max sec mac number in this unit */
    BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_SEC_MAC_NUM_PER_PORT, &max_sec_mac));

    if (max_sec_mac == 0) { 
        /* max_sec_mac=0 means we can not add any sec mac*/
        return BCM_E_UNAVAIL;
    }
    
    if (p < 0) { 
		t_pbm = PBMP_E_ALL(unit);
    } else {
        BCM_PBMP_PORT_SET(t_pbm, p);
    }

    if (SOC_IS_ROBO_ARCH_VULCAN(unit) || SOC_IS_ROBO53242(unit) || 
        SOC_IS_ROBO53262(unit)){
        bcm_mac_t   temp_mac;

        BCM_PBMP_ITER(t_pbm, p) {
            BCM_IF_ERROR_RETURN(DRV_MAC_GET(
                    unit, p, DRV_MAC_CUSTOM_EAP, &pbm, temp_mac));
    
            /* bcm53115/53118 support one custom EAP_DA only :
             *  - get original setting to see if any confilct existed.
             *  - FULL condition is check through SW database on secMacNum.
             */
            if (SAL_MAC_ADDR_EQUAL(temp_mac, mac)) {
                return BCM_E_EXISTS;
            }
    
            /* Check the S/W security mac numbers is FULL or not */
            if (robo_auth_info[unit].portAuth[p].secMacNum == max_sec_mac){
                return BCM_E_FULL;
            }
    
            BCM_PBMP_CLEAR(pbm); 
            BCM_PBMP_PORT_ADD(pbm, p);
            BCM_IF_ERROR_RETURN(DRV_MAC_SET(
                    unit, pbm, DRV_MAC_CUSTOM_EAP, mac, 0));

            (robo_auth_info[unit].portAuth[p].secMacNum)++;
        }
    } else {
        BCM_PBMP_ITER(t_pbm, p) {
            /* Check the S/W security mac numbers is FULL or not */
            if (robo_auth_info[unit].portAuth[p].secMacNum == max_sec_mac){
                return BCM_E_FULL;
            }

            BCM_PBMP_CLEAR(pbm); 
            BCM_PBMP_PORT_ADD(pbm, p);
	
            /* add this mac into STSEC_MAC table(s) */
            BCM_IF_ERROR_RETURN(DRV_MAC_SET
                (unit, pbm, DRV_MAC_SECURITY_ADD, mac, 0));
            
            /* Update the S/W info */
            BCM_IF_ERROR_RETURN(_robo_auth_maclist_insert
                            (&robo_auth_info[unit].portAuth[p].macList, 
                            mac, &entry));
    
            (robo_auth_info[unit].portAuth[p].secMacNum)++;
        }
    }

    return BCM_E_NONE;

}

/*
 * Function:
 *     bcm_robo_auth_mac_delete
 * Purpose:
 *    Delete a mac from the Static MAC Security Table for a specific port.
 * Parameters:
 *    unit - RoboSwitch unit number.
 *    port - RoboSwitch port number.
 *    mac  - Security MAC address.
 * Returns:
 *     BCM_E_NONE
 */
int 
bcm_robo_auth_mac_delete(int unit, int port, bcm_mac_t mac)
{
    bcm_pbmp_t      t_pbm;
    auth_mac_p      entry;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_auth_mac_delete()..\n")));

    AUTH_INIT(unit);
    
    AUTH_SUPPORT(unit);

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_port_local_get(unit, port, &port));
    }

    AUTH_PORT(unit, port);

    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);
    
    if (SOC_IS_ROBO_ARCH_VULCAN(unit) || SOC_IS_ROBO53242(unit) || 
        SOC_IS_ROBO53262(unit)){
        bcm_mac_t   temp_mac;
        
        BCM_IF_ERROR_RETURN(DRV_MAC_GET(
                unit, port, DRV_MAC_CUSTOM_EAP, &t_pbm, temp_mac));
        
        if (BCM_MAC_IS_ZERO(mac)){
            /* invalid to delete a zero MAC address */
            return BCM_E_PARAM;
        } else if (!SAL_MAC_ADDR_EQUAL(temp_mac, mac)){
            return BCM_E_NOT_FOUND;
        } else {
            BCM_PBMP_CLEAR(t_pbm); 
            BCM_PBMP_PORT_ADD(t_pbm, port);
            sal_memset(temp_mac, 0, sizeof(bcm_mac_t));
            BCM_IF_ERROR_RETURN(DRV_MAC_SET(
                    unit, t_pbm, DRV_MAC_CUSTOM_EAP, temp_mac, 0));
        }
    } else {

        /* delete this mac from STSEC_MAC table(s) */
        BCM_IF_ERROR_RETURN(DRV_MAC_SET
            (unit, t_pbm, DRV_MAC_SECURITY_REMOVE, mac, 0));
    
        /* Update the S/W info */
        BCM_IF_ERROR_RETURN( _robo_auth_maclist_remove
                        (&robo_auth_info[unit].portAuth[port].macList, 
                        mac, &entry));
    }
    (robo_auth_info[unit].portAuth[port].secMacNum)--;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_auth_mac_delete_all
 * Purpose:
 *      Delete all switch's MAC addresses.
 * Parameters:
 *      unit - Device number
 *    port - Port number 
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

int 
bcm_robo_auth_mac_delete_all(int unit, int port)
{
    bcm_pbmp_t      t_pbm;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_auth_mac_delete_all()..\n")));

    AUTH_INIT(unit);
    
    AUTH_SUPPORT(unit);

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_port_local_get(unit, port, &port));
    }

    AUTH_PORT(unit, port);

    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);

    if (SOC_IS_ROBO_ARCH_VULCAN(unit) || SOC_IS_ROBO53242(unit) || 
        SOC_IS_ROBO53262(unit)){
        bcm_mac_t   temp_mac;
        
        BCM_IF_ERROR_RETURN(DRV_MAC_GET(
                unit, port, DRV_MAC_CUSTOM_EAP, &t_pbm, temp_mac));
        
        if (BCM_MAC_IS_ZERO(temp_mac)){
            /* no existed custom EAP_DA already */
            return BCM_E_NONE;
        } else {
            BCM_PBMP_CLEAR(t_pbm); 
            BCM_PBMP_PORT_ADD(t_pbm, port);
            sal_memset(temp_mac, 0, sizeof(bcm_mac_t));
            BCM_IF_ERROR_RETURN(DRV_MAC_SET(
                    unit, t_pbm, DRV_MAC_CUSTOM_EAP, temp_mac, 0));
        }
    } else {

        /* delete this mac from STSEC_MAC table(s) */
        BCM_IF_ERROR_RETURN(DRV_MAC_SET
            (unit, t_pbm, DRV_MAC_SECURITY_CLEAR, 0, 0));
    
        /* Update the S/W info */
        
        BCM_IF_ERROR_RETURN( _robo_auth_maclist_destroy
                        (&robo_auth_info[unit].portAuth[port].macList));
    }
    robo_auth_info[unit].portAuth[port].secMacNum= 0;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_robo_auth_sec_mode_set
 * Purpose:
 *      Set the MAC security access control mode.
 * Parameters:
 *      unit    - RoboSwitch unit number.
 *    port    - RoboSwitch port number.
 *    mode    - BCM_AUTH_SEC_XXX.
 *    mac_num - When mode is BCM_AUTH_SEC_SA_XXX, 
 *              mac_num is the maximum allowable number 
 *              MACs can be learned in ARL table.
 *          When mode is BCM_AUTH_SEC_STATIC_XXX, 
 *              mac_num is fixed on 16, will be set in bcm_auth_init().
 * Returns:
 *      BCM_E_XXX
 * Note :
 *      This API is not been prompted to be the standard API currently. 
 *      we implemented this API here for upper layer application on 
 *      applying to port security request but named this API as an 
 *      bcm internal API.
 */
int 
_bcm_robo_auth_sec_mode_set(int unit, bcm_port_t port, int mode,
                 int mac_num)
{
    bcm_pbmp_t      t_pbm;
    uint32          sec_mode = 0;
    uint32          sec_num = 0;
    AUTH_INIT(unit);

    AUTH_SUPPORT(unit);

    AUTH_PORT(unit, port);

    BCM_PBMP_CLEAR(t_pbm); 
    BCM_PBMP_PORT_ADD(t_pbm, port);

    if (mode == BCM_AUTH_SEC_NONE){
        sec_mode = DRV_PORT_PROP_SEC_MAC_MODE_NONE;
    } else if (mode == BCM_AUTH_SEC_STATIC_ACCEPT){
        sec_mode = DRV_PORT_PROP_SEC_MAC_MODE_STATIC_ACCEPT;
    } else if (mode == BCM_AUTH_SEC_STATIC_REJECT){
        sec_mode = DRV_PORT_PROP_SEC_MAC_MODE_STATIC_REJECT;
    } else if (mode == BCM_AUTH_SEC_SA_NUM) {
        sec_mode = DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM;
        sec_num = mac_num;
        if (mac_num > 4095) {
            return BCM_E_PARAM;
        }
    } else if (mode == BCM_AUTH_SEC_SA_MATCH){
        sec_mode = DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH;
    } else {
        return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN(DRV_PORT_SET
        (unit, t_pbm, sec_mode, sec_num));
    /* Update the S/W info */
    robo_auth_info[unit].portAuth[port].secMode = mode;
    robo_auth_info[unit].portAuth[port].dynaMacNum = mac_num;

    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_robo_auth_sec_mode_get
 * Purpose:
 *     Get the MAC security access control mode.
 * Parameters:
 *      unit    - RoboSwitch unit number.
 *    port    - RoboSwitch port number.
 *    mode    - BCM_AUTH_SEC_XXX.
 *    mac_num - This is valid only for mode is BCM_AUTH_SEC_SA_NUM, 
 *          mac_num is the maximum allowable number 
 *          MACs can be learned in ARL table.
 *          Other modes, don't care this field.
 * Returns:
 *      BCM_E_XXX
 * Note :
 *      This API is not been prompted to be the standard API currently. 
 *      we implemented this API here for upper layer application on 
 *      applying to port security request but named this API as an 
 *      bcm internal API.
 */
int 
_bcm_robo_auth_sec_mode_get(int unit, bcm_port_t port, int *mode,
                 int *mac_num)
{

    AUTH_INIT(unit);

    AUTH_SUPPORT(unit);

    AUTH_PORT(unit, port);

    *mode = robo_auth_info[unit].portAuth[port].secMode;

    if (*mode == BCM_AUTH_SEC_SA_NUM) {
        *mac_num = robo_auth_info[unit].portAuth[port].dynaMacNum;
    } else {
        /* don't care in other modes */
        *mac_num = 0;
    }

    return BCM_E_NONE;
}
/*
 * Function:
 *      _robo_auth_linkscan_cb
 * Description:
 *      Put authorized state to unauthorized state if link down,
 *      given BCM_AUTH_IGNORE_LINK not set
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      info - pointer to structure giving status
 * Returns:
 *      None
 * Notes:
 */

STATIC void
_robo_auth_linkscan_cb(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    if ((IS_E_PORT(unit, port))
        && !(robo_auth_info[unit].portAuth[port].flags & 
            BCM_AUTH_MODE_UNCONTROLLED)) {
        if (!info->linkstatus) {
            if ((robo_auth_info[unit].portAuth[port].flags & 
                    BCM_AUTH_MODE_AUTH) &&  
                !(robo_auth_info[unit].portAuth[port].flags & 
                    BCM_AUTH_IGNORE_LINK)) {
                bcm_auth_mode_set(unit, port, BCM_AUTH_MODE_UNAUTH);
                if (cb_cntl[unit].auth_cbs) {
                    cb_cntl[unit].auth_cbs(cb_cntl[unit].auth_cb_data,
                                  unit, port, BCM_AUTH_REASON_LINK);
                }
            }
        }
    }
}

/*
 * Function: 
 *     bcm_robo_auth_mac_control_set
 * Purpose: 
 *    Set an auth mac control type to enable or disable bypass for EAP packet.
 * Parameters: 
 *    unit - (IN)RoboSwitch unit number.
 *    type - (IN)auth mac control type.
 *    value  - (OUT)Enable or disable.
 * Returns: 
 *     BCM_E_NONE 
 */
 int 
 bcm_robo_auth_mac_control_set(int unit, 
    bcm_auth_mac_control_t type, uint32 value)
{
    int rv = BCM_E_NONE;

    switch(type) {
        case bcmEapControlL2UserAddr:
            BCM_IF_ERROR_RETURN(DRV_SECURITY_EAP_CONTROL_SET
                (unit, DRV_DEV_CTRL_EAP_BYPASS_USERADDR, (value) ? 1 : 0));
            break;
        case bcmEapControlDHCP:
            BCM_IF_ERROR_RETURN(DRV_SECURITY_EAP_CONTROL_SET
                (unit, DRV_DEV_CTRL_EAP_BYPASS_DHCP, (value) ? 1 : 0));
            break;
        case bcmEapControlARP:
            BCM_IF_ERROR_RETURN(DRV_SECURITY_EAP_CONTROL_SET
                (unit, DRV_DEV_CTRL_EAP_BYPASS_ARP, (value) ? 1 : 0));
            break;
        case bcmEapControlMAC2X:
            BCM_IF_ERROR_RETURN(DRV_SECURITY_EAP_CONTROL_SET
                (unit, DRV_DEV_CTRL_EAP_BYPASS_MAC_22_2F, (value) ? 1 : 0));
            break;
        case bcmEapControlGVRP:
            BCM_IF_ERROR_RETURN(DRV_SECURITY_EAP_CONTROL_SET
                (unit, DRV_DEV_CTRL_EAP_BYPASS_MAC_21, (value) ? 1 : 0));
            break;
        case bcmEapControlGMRP:
            BCM_IF_ERROR_RETURN(DRV_SECURITY_EAP_CONTROL_SET
                (unit, DRV_DEV_CTRL_EAP_BYPASS_MAC_20, (value) ? 1 : 0));
            break;
        case bcmEapControlMAC1X:
            BCM_IF_ERROR_RETURN(DRV_SECURITY_EAP_CONTROL_SET
                (unit, DRV_DEV_CTRL_EAP_BYPASS_MAC_11_1F, (value) ? 1 : 0));
            break;
        case bcmEapControlAllBridges:
            BCM_IF_ERROR_RETURN(DRV_SECURITY_EAP_CONTROL_SET
                (unit, DRV_DEV_CTRL_EAP_BYPASS_MAC_10, (value) ? 1 : 0));
            break;
        case bcmEapControlMAC0X:
            BCM_IF_ERROR_RETURN(DRV_SECURITY_EAP_CONTROL_SET
                (unit, DRV_DEV_CTRL_EAP_BYPASS_MAC_0X, (value) ? 1 : 0));
            break;
        case bcmEapControlMACBPDU:
            BCM_IF_ERROR_RETURN(DRV_SECURITY_EAP_CONTROL_SET
                (unit, DRV_DEV_CTRL_EAP_BYPASS_MAC_BPDU, (value) ? 1 : 0));
            break;
        default:
            rv = BCM_E_PARAM;
    }

    return rv;
}

/* 
 * Function: 
 *     bcm_robo_auth_mac_control_get
 * Purpose: 
 *    Get the the EAP mac control status of given auth mac control type.
 * Parameters: 
 *    unit - (IN)RoboSwitch unit number.
 *    type - (IN)auth mac control type.
 *    value  - (OUT)Enable or disable.
 * Returns: 
 *     BCM_E_NONE 
 */
int 
bcm_robo_auth_mac_control_get(int unit, 
    bcm_auth_mac_control_t type, uint32 *value)
{
    uint32  temp = 0;
    int rv = BCM_E_NONE;

    switch(type) {
        case bcmEapControlL2UserAddr:
            BCM_IF_ERROR_RETURN(DRV_SECURITY_EAP_CONTROL_GET
                (unit, DRV_DEV_CTRL_EAP_BYPASS_USERADDR, &temp));
            break;
        case bcmEapControlDHCP:
            BCM_IF_ERROR_RETURN(DRV_SECURITY_EAP_CONTROL_GET
                (unit, DRV_DEV_CTRL_EAP_BYPASS_DHCP, &temp));
            break;
        case bcmEapControlARP:
            BCM_IF_ERROR_RETURN(DRV_SECURITY_EAP_CONTROL_GET
                (unit, DRV_DEV_CTRL_EAP_BYPASS_ARP, &temp));
            break;
        case bcmEapControlMAC2X:
            BCM_IF_ERROR_RETURN(DRV_SECURITY_EAP_CONTROL_GET
                (unit, DRV_DEV_CTRL_EAP_BYPASS_MAC_22_2F, &temp));
            break;
        case bcmEapControlGVRP:
            BCM_IF_ERROR_RETURN(DRV_SECURITY_EAP_CONTROL_GET
                (unit, DRV_DEV_CTRL_EAP_BYPASS_MAC_21, &temp));
            break;
        case bcmEapControlGMRP:
            BCM_IF_ERROR_RETURN(DRV_SECURITY_EAP_CONTROL_GET
                (unit, DRV_DEV_CTRL_EAP_BYPASS_MAC_20, &temp));
            break;
        case bcmEapControlMAC1X:
            BCM_IF_ERROR_RETURN(DRV_SECURITY_EAP_CONTROL_GET
                (unit, DRV_DEV_CTRL_EAP_BYPASS_MAC_11_1F, &temp));
            break;
        case bcmEapControlAllBridges:
            BCM_IF_ERROR_RETURN(DRV_SECURITY_EAP_CONTROL_GET
                (unit, DRV_DEV_CTRL_EAP_BYPASS_MAC_10, &temp));
            break;
        case bcmEapControlMAC0X:
            BCM_IF_ERROR_RETURN(DRV_SECURITY_EAP_CONTROL_GET
                (unit, DRV_DEV_CTRL_EAP_BYPASS_MAC_0X, &temp));
            break;
        case bcmEapControlMACBPDU:
            BCM_IF_ERROR_RETURN(DRV_SECURITY_EAP_CONTROL_GET
                (unit, DRV_DEV_CTRL_EAP_BYPASS_MAC_BPDU, &temp));
            break;
        default:
            rv = BCM_E_PARAM;
    }

    *value = (temp) ? 1 : 0;

    return rv;
}

