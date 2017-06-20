/*
* $Id:mpls.c,v 1.1 2013/6/28 09:05:00 Jianping Exp $
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
*
* This file contains MPLS functions
*/

#include <soc/drv.h>
#include <soc/robo/pae.h>
#include <bcm/error.h>
#include <bcm/debug.h>
#include <bcm/mpls.h>
#include <bcm/port.h>
#include <bcm_int/robo/mpls.h>
#include <bcm_int/robo/l3.h>
#include <bcm_int/robo_dispatch.h>
#include <soc/defs.h>


#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)


extern _bcm_robo_l3_entry_t *_bcm_robo_l3_intf_get(int unit, bcm_if_t intf_id);
static int _bcm_robo_mpls_port_type_get(bcm_mpls_port_t *mpls_port, _mpls_port_type_t *port_type);

#define _MPLS_VPN_PORT_INDEX_INVALID -1


static int32 _mpls_port_index_seed = 0;

static _bcm_robo_mpls_vpws_vpn_t     _bcm_robo_mpls_vpns[BCM_MAX_NUM_UNITS] = {{0,},};
static _bcm_robo_mpls_port_t         _bcm_robo_mpls_ports[BCM_MAX_NUM_UNITS] = {{0,},};
static _bcm_robo_mpls_switch_entry_t _bcm_robo_mpls_switch_table[BCM_MAX_NUM_UNITS] = {{0,},};
static _bcm_robo_mpls_lookup_entry_t _bcm_robo_mpls_lookup_table[BCM_MAX_NUM_UNITS] = {{0,},};

#define _BCM_ROBO_MPLS_LINK_ADD(head, node) do{ \
    node->p_next = head->p_next; \
    if(head->p_next){ \
        head->p_next->p_pre = node; \
    } \
    head->p_next = node; \
    node->p_pre  = head; \
} while(0)

#define _BCM_ROBO_MPLS_LINK_DEL(head, node) do{ \
    node->p_pre->p_next = node->p_next; \
    if(node->p_next){ \
        node->p_next->p_pre = node->p_pre; \
    } \
    sal_free(node); \
} while(0)        

#define _BCM_ROBO_MPLS_LINK_SRCH_1_KEY(head, field, field_val, ptr) do{ \
    ptr = head->p_next; \
    while(ptr){ \
        if(ptr->field == field_val){ \
            break; \
        } \
        ptr = ptr->p_next; \
    } \
}while(0)

#define _BCM_ROBO_MPLS_LINK_SRCH_2_KEY(head, field_1, field_val_1, field_2, field_val_2, ptr) do{ \
    ptr = head->p_next; \
    while(ptr){ \
        if((ptr->field_1 == field_val_1) && (ptr->field_2 == field_val_2)){ \
            break; \
        } \
        ptr = ptr->p_next; \
    } \
}while(0)

uint32 _bcm_robo_l2b_endian(uint32 little_endian)
{
    uint32 big_endian=0;
    big_endian=(little_endian&0xFF000000)>>24;
    big_endian|=(little_endian&0x00FF0000)>>8;
    big_endian|=(little_endian&0x0000FF00)<<8;
    big_endian|=(little_endian&0x000000FF)<<24;
    return big_endian;
}

/*convert the 20 bits label from little endian to big endian
    0xABCDE
    little endian: 0xDE 0xBC 0x0A
    big   endian: 0xAB 0xCD,0xE0
*/
uint32 _bcm_robo_mpls_label_l2b_endian(uint32 little_endian)
{
    uint32 big_endian=0;
    big_endian = (little_endian&0x000F0000)>>12;
    big_endian|=(little_endian&0x0000F000)>>12;
    big_endian|=(little_endian&0x00000F00)<<4;
    big_endian|=(little_endian&0x000000F0)<<4;
    big_endian|=(little_endian&0x0000000F)<<20;
    return big_endian;
}

/*
 * Function:
 *      _bcm_robo_mpls_vpn_get
 * Description:
 *      find the vpn entry int the table  
 * Parameters:
 *      unit  - (IN) BCM device number
 *      vpn   - (IN) vpn id
 * Returns     : 
 *      NULL or the found entry pointer
 */
static _bcm_robo_mpls_vpws_vpn_t*
_bcm_robo_mpls_vpn_get(int unit, bcm_vpn_t vpn)
{
    _bcm_robo_mpls_vpws_vpn_t *p_vpn = NULL;
    
    _BCM_ROBO_MPLS_LINK_SRCH_1_KEY((&_bcm_robo_mpls_vpns[unit]), vpn, vpn, p_vpn);

    return p_vpn;
}

/*
 * Function:
 *      _bcm_robo_mpls_vpn_port_valid_check
 * Description:
 *      check whether the vpn port type is valid.
 *      check whether the vpn port has been added into the vpn.
 * Parameters:
 *      p_vpn        - (IN) pointer to a valid vpn structure
 *      port_type    - (IN) only MPLS_PORT_TYPE_NNI &  MPLS_PORT_TYPE_UNI is supported
 * Returns     : 
 *      BCM_E_XXX
 */
static int 
_bcm_robo_mpls_vpn_port_valid_check(_bcm_robo_mpls_vpws_vpn_t *p_vpn, _mpls_port_type_t port_type)
{
    /*check if the two valid ports alreay added*/
    if((p_vpn->nni_port_index != _MPLS_VPN_PORT_INDEX_INVALID) && 
        (p_vpn->uni_port_index != _MPLS_VPN_PORT_INDEX_INVALID)){
        return BCM_E_EXISTS;
    }
    
    if((port_type == MPLS_PORT_TYPE_NNI) && 
        (p_vpn->nni_port_index != _MPLS_VPN_PORT_INDEX_INVALID)){
        return BCM_E_EXISTS;
    }
    else if((port_type == MPLS_PORT_TYPE_UNI) && 
        (p_vpn->uni_port_index != _MPLS_VPN_PORT_INDEX_INVALID)){
        return BCM_E_EXISTS;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_robo_mpls_port_conv_bcm_2_robo
 * Description:
 *      Convert the bcm_mpls_port_t object to internal _bcm_robo_mpls_port_t object.
 *      check whether the vpn port has been added into the vpn.
 * Parameters:
 *      unit        - (IN) BCM device number
 *      mpls_port   - (IN) pointer to bcm_mpls_port_t object 
 *      p_port      - (INOUT) pointer to _bcm_robo_mpls_port_t object
 * Returns     : 
 *      BCM_E_XXX
 */
static int 
_bcm_robo_mpls_port_conv_bcm_2_robo(int unit, bcm_mpls_port_t *mpls_port, _bcm_robo_mpls_port_t *p_port)
{
    int rv;
    _mpls_port_type_t       port_type;
    bcm_port_t              local_port;

    rv = _bcm_robo_mpls_port_type_get(mpls_port, &port_type);
    if(BCM_E_NONE != rv){
        return rv;
    }
    
    rv = bcm_robo_port_local_get(unit, mpls_port->port, &local_port);
    if(BCM_E_NONE != rv){
        return rv;
    }
    sal_memset(p_port,0,sizeof(_bcm_robo_mpls_port_t));
    p_port->port_type           = port_type;
    p_port->port_index          = ++_mpls_port_index_seed;
    p_port->criteria            = mpls_port->criteria;
    p_port->port_id             = local_port;

    if(port_type ==MPLS_PORT_TYPE_NNI ){
        p_port->egr_intf_index      = mpls_port->egress_tunnel_if;
        p_port->vlan_id             = mpls_port->match_vlan;
        p_port->match_vc_mpls_label = _bcm_robo_mpls_label_l2b_endian(mpls_port->match_label&0xFFFFF);
        p_port->egr_vc_mpls_label   = _bcm_robo_mpls_label_l2b_endian(mpls_port->egress_label.label&0xFFFFF);
        p_port->egr_vc_mpls_label   |= (0x01<<23);/*stack bottom*/

        if((mpls_port->egress_label.flags) & BCM_MPLS_EGRESS_LABEL_TTL_SET){
            p_port->egr_vc_mpls_label |= (mpls_port->egress_label.ttl)<<24;
        }
        else{
            p_port->egr_vc_mpls_label |= 0xFF<<24;
        }
    }

    return rv;

}

/*
 * Function:
 *      _bcm_robo_mpls_port_conv_bcm_2_robo
 * Description:
 *      Convert the bcm_mpls_port_t object to internal _bcm_robo_mpls_port_t object.
 *      check whether the vpn port has been added into the vpn.
 * Parameters:
 *      unit        - (IN) BCM device number
 *      mpls_port   - (IN) pointer to bcm_mpls_port_t object 
 *      p_port      - (INOUT) pointer to _bcm_robo_mpls_port_t object
 * Returns     : 
 *      BCM_E_XXX
 */
static int
_bcm_robo_mpls_port_conv_robo_2_bcm(int unit, _bcm_robo_mpls_port_t *p_port, bcm_mpls_port_t *mpls_port)
{
    int rv;
    bcm_gport_t gport;

    rv = bcm_robo_port_gport_get(unit, p_port->port_id, &gport);
    if(BCM_E_NONE != rv){
        return rv;
    }

    sal_memset(mpls_port, 0, sizeof(bcm_mpls_port_t));

    mpls_port->port               = gport;
    mpls_port->criteria           = p_port->criteria;
    mpls_port->egress_tunnel_if   = p_port->egr_intf_index;
    mpls_port->match_vlan         = p_port->vlan_id;
    mpls_port->match_label        = p_port->match_vc_mpls_label;
    mpls_port->egress_label.label = (p_port->egr_vc_mpls_label >> 12);
    mpls_port->egress_label.ttl   = (p_port->egr_vc_mpls_label & 0xFF);

    return rv;
}

/*
 * Function:
 *      _bcm_robo_mpls_port_create
 * Description:
 *      create a new port according to the input parameter and insert it to 
 *      internal _bcm_robo_mpls_port_t link list head.
 * Parameters:
 *      unit         - (IN) BCM device number
 *      mpls_port    - (IN) pointer to bcm_mpls_port_t object 
 * Returns     : 
 *      pointer to the created _bcm_robo_mpls_port_t structure.
 */
static _bcm_robo_mpls_port_t*
_bcm_robo_mpls_port_create(int unit, bcm_mpls_port_t *mpls_port)
{
    int rv = BCM_E_NONE;
    _bcm_robo_mpls_port_t *p_port;

    p_port = sal_alloc(sizeof(_bcm_robo_mpls_port_t),"robo mpls port create");
    if(!p_port){
        return NULL;
    }

    sal_memset(p_port,0,sizeof(_bcm_robo_mpls_port_t));
    rv = _bcm_robo_mpls_port_conv_bcm_2_robo(unit, mpls_port, p_port);
    if(BCM_E_NONE != rv){
        sal_free(p_port);
        return NULL;
    }

    _BCM_ROBO_MPLS_LINK_ADD((&_bcm_robo_mpls_ports[unit]), p_port);

    return p_port; 
}

/*
 * Function:
 *      _bcm_robo_mpls_port_destroy
 * Description:
 *      remove the object from the link list and free the pointer
 * Parameters:
 *      unit         - (IN) BCM device number
 *      p_port       - (IN) pointer to _bcm_robo_mpls_port_t object 
 * Returns     : 
 *      void
 */
static void
_bcm_robo_mpls_port_destroy(int unit, _bcm_robo_mpls_port_t *p_port)
{
    _BCM_ROBO_MPLS_LINK_DEL((&_bcm_robo_mpls_ports[unit]), p_port);
}

/*
 * Function:
 *      _bcm_robo_mpls_port_find
 * Description:
 *      find the mpls port entry int the table  
 *      which is used before this port is created
 * Parameters:
 *      unit         - (IN) BCM device number
 *      mpls_port    - (IN) port information
 * Returns     : 
 *      NULL or the found entry pointer
 */
static _bcm_robo_mpls_port_t*
_bcm_robo_mpls_port_find(int unit, bcm_mpls_port_t *mpls_port)
{
    _bcm_robo_mpls_port_t *p_port = NULL;
    bcm_port_t            local_port;

    if(BCM_E_NONE != bcm_robo_port_local_get(unit, mpls_port->port, &local_port)){
        return NULL;
    }

    switch(mpls_port->criteria){
        case BCM_MPLS_PORT_MATCH_LABEL:
            _BCM_ROBO_MPLS_LINK_SRCH_2_KEY((&_bcm_robo_mpls_ports[unit]), port_id, local_port,
                match_vc_mpls_label, mpls_port->match_label, p_port);
            break;

        case BCM_MPLS_PORT_MATCH_PORT:
            _BCM_ROBO_MPLS_LINK_SRCH_1_KEY((&_bcm_robo_mpls_ports[unit]), port_id, local_port, p_port);
            break;

        case BCM_MPLS_PORT_MATCH_PORT_VLAN:
            _BCM_ROBO_MPLS_LINK_SRCH_2_KEY((&_bcm_robo_mpls_ports[unit]), port_id, local_port, 
                vlan_id, mpls_port->match_vlan, p_port);
            break;        

        default:
            return NULL;
    }

    /* Validation for p_port should be done by the caller. */
    return p_port;
}

/*
 * Function:
 *      _bcm_robo_mpls_port_type_get
 * Description:
 *      check the mpls_port's flags and criteria field to 
 *      get the correct port type. 
 * Parameters:
 *      mpls_port - (IN) port information
 *      port_type - (OUT) port type
 * Returns     : 
 *      BCM_E_XXX
 */
static int 
_bcm_robo_mpls_port_type_get(bcm_mpls_port_t *mpls_port, _mpls_port_type_t *port_type)
{
    if(mpls_port->flags & BCM_MPLS_PORT_EGRESS_TUNNEL){
        *port_type = MPLS_PORT_TYPE_NNI;
        if(mpls_port->criteria != BCM_MPLS_PORT_MATCH_LABEL){/* Matched key should always be label for NNI */
            return BCM_E_PARAM;
        }
    }
    else{
        *port_type = MPLS_PORT_TYPE_UNI;
        if(mpls_port->criteria != BCM_MPLS_PORT_MATCH_PORT_VLAN){/* Matched key should always not be label for UNI */
            return BCM_E_PARAM;
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_robo_mpls_switch_entry_create
 * Description:
 *      alloc memory , create a switch entry and insert to it to 
 *      local link list.
 * Parameters:
 *      unit - (IN) BCM device number
 *      info - (IN) pointer object
 * Returns     : 
 *      NULL or pointer
 */
static _bcm_robo_mpls_switch_entry_t*
_bcm_robo_mpls_switch_entry_create(int unit, bcm_mpls_tunnel_switch_t *info)
{
    bcm_port_t local_port;
    _bcm_robo_l3_entry_t          *p_intf = NULL;
    _bcm_robo_mpls_vpws_vpn_t     *p_vpn  = NULL;
    _bcm_robo_mpls_switch_entry_t *p_switch_entry = NULL;

    if(BCM_E_NONE != bcm_robo_port_local_get(unit, info->port, &local_port)){
        return NULL;
    }

    switch(info->action){
        case BCM_MPLS_SWITCH_ACTION_POP:
            p_vpn = _bcm_robo_mpls_vpn_get(unit, info->vpn);
            if(!p_vpn){
                return NULL;
            }

            /* Not allowed to create switch entry without nni_port_index in LER mode*/
            if(p_vpn->nni_port_index == _MPLS_VPN_PORT_INDEX_INVALID){
                return NULL;
            }

            break;

        case BCM_MPLS_SWITCH_ACTION_SWAP:
            p_intf = _bcm_robo_l3_intf_get(unit, info->egress_if);
            if(!p_intf){
                return NULL;
            }

            break;

        default:
            return NULL;
    }

    p_switch_entry = sal_alloc(sizeof(_bcm_robo_mpls_switch_entry_t),"robo mpls switch enty");
    if(!p_switch_entry){
        return NULL;
    }

    sal_memset(p_switch_entry, 0, sizeof(_bcm_robo_mpls_switch_entry_t));

    p_switch_entry->action            = info->action;
    p_switch_entry->ing_port_id       = local_port;
    p_switch_entry->ing_vp_mpls_label = _bcm_robo_mpls_label_l2b_endian(info->label&0xFFFFF);

    if(info->action == BCM_MPLS_SWITCH_ACTION_SWAP){
        p_switch_entry->egr_intf_index    = info->egress_if;
        p_switch_entry->egr_vp_mpls_label = _bcm_robo_mpls_label_l2b_endian(info->egress_label.label&0xFFFFF);

        if(info->egress_label.flags & BCM_MPLS_EGRESS_LABEL_TTL_SET){
            p_switch_entry->egr_vp_mpls_label |= (info->egress_label.ttl)<<24;
        }
        else{
            p_switch_entry->egr_vp_mpls_label |= 0xFF<<24;
        }
    }
    else{
        p_switch_entry->vpn = info->vpn;
    }
    
    _BCM_ROBO_MPLS_LINK_ADD((&_bcm_robo_mpls_switch_table[unit]), p_switch_entry);

    return p_switch_entry; 
}

/*
 * Function:
 *      _bcm_robo_mpls_switch_entry_destroy
 * Description:
 *      delete the pointer object from the local link list 
 *      and free the memory
 * Parameters:
 *      unit      - (IN) BCM device number
 *      p_entry   - (IN) pointer object
 * Returns     : 
 *      void
 */
static void
_bcm_robo_mpls_switch_entry_destroy(int unit, _bcm_robo_mpls_switch_entry_t *p_entry)
{
    _BCM_ROBO_MPLS_LINK_DEL((&_bcm_robo_mpls_switch_table[unit]), p_entry);
}

/*
 * Function:
 *      _bcm_robo_mpls_switch_entry_get
 * Description:
 *      find the _bcm_robo_mpls_switch_entry in the table  
 * Parameters:
 *      unit  - (IN) BCM device number
 *      info  - (IN) switch entry info
 * Returns     : 
 *      NULL or the found entry pointer
 */
static _bcm_robo_mpls_switch_entry_t*
_bcm_robo_mpls_switch_entry_get(int unit, bcm_mpls_tunnel_switch_t *info)
{
    bcm_port_t local_port;
    _bcm_robo_mpls_switch_entry_t *p_entry = NULL;

    if(BCM_E_NONE != bcm_robo_port_local_get(unit, info->port, &local_port)){
        return NULL;
    }

    _BCM_ROBO_MPLS_LINK_SRCH_2_KEY((&_bcm_robo_mpls_switch_table[unit]), ing_vp_mpls_label, info->label,
                ing_port_id, local_port, p_entry);

    return p_entry;
}

/*
 * Function:
 *      _bcm_robo_mpls_lookup_entry_from_uni_create
 * Description:
 *      alloc memory, create the lookup entry which is from UNI port 
 *      to NNI portand insert it into the link list
 * Parameters:
 *      unit    - (IN) BCM device number
 *      p_vpn   - (IN) pointer to the vpn object
 *      p_port  - (IN) pointer to the port object
 * Returns     : 
 *      BCM_E_XXX
 */
static int
_bcm_robo_mpls_lookup_entry_from_uni_create(int unit, _bcm_robo_mpls_vpws_vpn_t *p_vpn, _bcm_robo_mpls_port_t *p_nni_port)
{
    _bcm_robo_mpls_lookup_entry_t *p_entry = NULL;
    _bcm_robo_mpls_port_t *p_uni_port=NULL;
    _bcm_robo_l3_entry_t * p_l3=NULL;
    uint8 eth_type[2];
    uint8 encap_len=0;
    pae_response_code res_code;
    uint32 r5_waite_time=0;
    uint32 pad_addr=0;

    _BCM_ROBO_MPLS_LINK_SRCH_1_KEY((&_bcm_robo_mpls_ports[unit]), port_index, p_vpn->uni_port_index, p_uni_port);
    p_l3=_bcm_robo_l3_intf_get(unit,p_nni_port->egr_intf_index);

    p_entry = sal_alloc(sizeof(_bcm_robo_mpls_lookup_entry_t),"robo lookup entry");
    if(!p_entry){
        return BCM_E_MEMORY;
    }
    sal_memset(p_entry,0,sizeof(_bcm_robo_mpls_lookup_entry_t));
    p_entry->vpn            = p_vpn->vpn;
    p_entry->uni_port_index = p_vpn->uni_port_index;
    p_entry->nni_port_index = p_vpn->nni_port_index;
    p_entry->l3_intf_index  = p_nni_port->egr_intf_index;

    /*key and data builder default host little endian and convert to big endian*/
    if(soc_pae_lue_idx_alloc(unit, PAE_DB_NUMBER,&p_entry->idx)!=BCM_E_NONE){
        sal_free(p_entry);
        return BCM_E_FAIL;
    }
    
    /*key builder*/
    p_entry->key[0]=(p_uni_port->port_id&0x0F)|0xC0;
    p_entry->key[1]=(p_uni_port->vlan_id&0x0F00)>>8;
    p_entry->key[2]=(p_uni_port->vlan_id&0xFF);

    /*associate data builder without encapsulation length*/
    pad_addr=soc_pae_get_sratch_pad_entry_addr(unit, p_entry->idx);
    p_entry->data[0]=pad_addr&0xFF;
    p_entry->data[1]=(pad_addr>>8)&0xFF;
    p_entry->data[2]=(pad_addr>>16)&0xFF;
    p_entry->data[3]=(pad_addr>>24)&0xFF;
    p_entry->data[5]=p_nni_port->port_id&0x0F;

    sal_memcpy(p_entry->encap,p_l3->dst_mac,6);/*dest mac addr*/
    sal_memcpy(p_entry->encap+6,p_l3->src_mac,6);/*src mac addr*/
    encap_len=12;

    eth_type[0]=0x81;
    eth_type[1]=0x00;
    sal_memcpy(p_entry->encap+encap_len,eth_type,2);/*vlan type*/
    encap_len+=2;
    p_entry->encap[encap_len]=p_nni_port->port_id&0x0F;
    p_entry->encap[encap_len+1]=0;
    encap_len+=2;
    eth_type[0]=0x88;
    eth_type[1]=0x47;
    sal_memcpy(p_entry->encap+encap_len,eth_type,2);
    encap_len+=2;

    sal_memcpy(p_entry->encap+encap_len,&p_l3->vp_mpls_label,4);
    encap_len+=4;	
    sal_memcpy(p_entry->encap+encap_len,&p_nni_port->egr_vc_mpls_label,4);
    encap_len+=4;	

    /*control word*/
    eth_type[0]=0;
    eth_type[1]=0;
    sal_memcpy(p_entry->encap+encap_len,eth_type,2);
    encap_len+=2;	
    sal_memcpy(p_entry->encap+encap_len,eth_type,2);
    encap_len+=2;	

    p_entry->data[4]=encap_len;

    soc_pae_set_cmd_response(unit, PAE_RES_INVALID);
    soc_pae_send_msg(unit, PAE_WRITE_BUF,pad_addr,encap_len,p_entry->encap);
    for(r5_waite_time=0;r5_waite_time<PAE_RES_WAIT_MS;r5_waite_time++){
        sal_usleep(1000);
        res_code=soc_pae_get_cmd_response(unit);
        if(res_code==PAE_RES_OK)
        break;
    }
    
    if(res_code!=PAE_RES_OK){/*R5 did not response OK code*/
        soc_pae_lue_idx_free(unit, PAE_DB_NUMBER, p_entry->idx);
        sal_free(p_entry);
        return BCM_E_FAIL;
    }
    
    if(soc_pae_lue_rule_add(unit, PAE_DB_NUMBER,p_entry->key,24,p_entry->data,p_entry->idx)!=BCM_E_NONE){
        sal_free(p_entry);
        return BCM_E_FAIL;
    }

    _BCM_ROBO_MPLS_LINK_ADD((&_bcm_robo_mpls_lookup_table[unit]), p_entry);

    return BCM_E_NONE; 
}

/*
 * Function:
 *      _bcm_robo_mpls_lookup_entry_from_uni_destroy
 * Description:
 *      delete the lookup entry which is from UNI port to NNI port
 *      and free the memory.
 * Parameters:
 *      unit    - (IN) BCM device number
 *      p_vpn   - (IN) pointer to the vpn object
 * Returns     : 
 *      void
 */
static void
_bcm_robo_mpls_lookup_entry_from_uni_destroy(int unit, _bcm_robo_mpls_vpws_vpn_t *p_vpn)
{
    _bcm_robo_mpls_lookup_entry_t *p_entry = NULL;
    
    _BCM_ROBO_MPLS_LINK_SRCH_2_KEY((&_bcm_robo_mpls_lookup_table[unit]), vpn, p_vpn->vpn, 
                                   uni_port_index, p_vpn->uni_port_index, p_entry);
    if(p_entry){
        soc_pae_lue_rule_del(unit, 0, p_entry->idx);
        _BCM_ROBO_MPLS_LINK_DEL((&_bcm_robo_mpls_lookup_table[unit]), p_entry);
    }
}

/*
 * Function:
 *      _bcm_robo_mpls_lookup_entry_from_nni_create
 * Description:
 *      alloc memory, create the lookup entry which is from NNI port
 *      and insert it to the local link list.
 * Parameters:
 *      unit                - (IN) BCM device number
 *      act                 - (IN)  swap and pop supported here
 *      p_switch_entry      - (IN) pointer to switch entry
 * Returns     : 
 *      BCM_E_XXX
 */
static int
_bcm_robo_mpls_lookup_entry_from_nni_create(int unit, 
                  _bcm_robo_mpls_switch_entry_t *p_switch_entry)
{
    /*add the lookup enty to hw*/
    _bcm_robo_mpls_lookup_entry_t *p_entry = NULL;
    _bcm_robo_mpls_vpws_vpn_t     *p_vpn   = NULL;
    bcm_mpls_switch_action_t act;
    uint8 eth_type[2];
    uint8 encap_len=0;
    pae_response_code res_code;
    uint32 pad_addr=0;

    act=p_switch_entry->action;
    p_entry = sal_alloc(sizeof(_bcm_robo_mpls_lookup_entry_t),"robo lookup entry");
    if(!p_entry){
        return BCM_E_MEMORY;
    }
    
    sal_memset(p_entry,0,sizeof(_bcm_robo_mpls_lookup_entry_t));
    if(act == BCM_MPLS_SWITCH_ACTION_SWAP){
        /*
        _bcm_robo_l3_entry_t * p_l3=_bcm_robo_l3_intf_get(unit, p_switch_entry->egr_intf_index);
        p_entry->port           = p_switch_entry->ing_port_id;
        p_entry->vp_mpls_label  = p_switch_entry->ing_vp_mpls_label;
        p_entry->l3_intf_index  = p_switch_entry->egr_intf_index;
        p_entry->key[0]=0x80| (p_entry->port&0x0F);
        sal_memcpy( p_entry->key+1,&p_switch_entry->ing_vp_mpls_label,3);
        p_entry->data[0]=p_l3->egr_port;
        sal_memcpy(p_entry->data+1,p_l3->dst_mac,6);
        p_entry->data[7]=(0x0F00&p_l3->vlan)>>8;
        p_entry->data[8]=(p_l3->vlan&0xFF)<<8;
        sal_memcpy(p_entry->data+9,&p_switch_entry->egr_vp_mpls_label,4);
        if(soc_pae_lue_rule_add(unit, 0,p_entry->key,4*8,p_entry->data,13*8,&p_entry->idx)!=BCM_E_NONE){
            sal_free(p_entry);
            return BCM_E_FAIL;
        }
        */
        return BCM_E_PARAM;
    }
    else {/*pop*/
        _bcm_robo_mpls_port_t * p_port_uni=NULL;
        _bcm_robo_mpls_port_t * p_port_nni=NULL;
        _BCM_ROBO_MPLS_LINK_SRCH_1_KEY((&_bcm_robo_mpls_vpns[unit]), vpn, p_switch_entry->vpn, p_vpn);
        if(!p_vpn){
            sal_free(p_entry);
            return BCM_E_NOT_FOUND;
        }

        _BCM_ROBO_MPLS_LINK_SRCH_1_KEY((&_bcm_robo_mpls_ports[unit]), port_index, p_vpn->nni_port_index, p_port_nni);
        _BCM_ROBO_MPLS_LINK_SRCH_1_KEY((&_bcm_robo_mpls_ports[unit]), port_index, p_vpn->uni_port_index, p_port_uni);

        if(!p_port_uni || !p_port_nni ){
            sal_free(p_entry);
            return BCM_E_FAIL;
        }
        
        p_entry->nni_port_index = p_vpn->nni_port_index;
        p_entry->vpn            = p_switch_entry->vpn;
        p_entry->port           = p_switch_entry->ing_port_id;
        p_entry->vp_mpls_label  = p_switch_entry->ing_vp_mpls_label;

        if(soc_pae_lue_idx_alloc(unit, PAE_DB_NUMBER, &p_entry->idx)!=BCM_E_NONE){
            sal_free(p_entry);
            return BCM_E_FAIL;
        }
        p_entry->key[0]=0x80|(p_entry->port&0x0F);
        sal_memcpy( p_entry->key+1,&p_switch_entry->ing_vp_mpls_label,3);
        sal_memcpy( p_entry->key+4,&p_port_nni->match_vc_mpls_label,3);

        encap_len=0;
        pad_addr=soc_pae_get_sratch_pad_entry_addr(unit, p_entry->idx);
        p_entry->data[0]=pad_addr&0xFF;
        p_entry->data[1]=(pad_addr>>8)&0xFF;
        p_entry->data[2]=(pad_addr>>16)&0xFF;
        p_entry->data[3]=(pad_addr>>24)&0xFF;
        p_entry->data[5]=p_port_uni->port_id&0xFF;

        eth_type[0]=0x81;
        eth_type[1]=0x00;
        sal_memcpy(p_entry->encap+encap_len,eth_type,2);/*vlan type*/
        encap_len+=2;
        p_entry->encap[encap_len]=p_port_uni->port_id&0x0F;
        p_entry->encap[encap_len+1]=0;
        encap_len+=2;
        p_entry->data[4]=encap_len; /*4bytes for vlan tag*/

        res_code=soc_pae_send_msg(unit, PAE_WRITE_BUF,pad_addr,encap_len,p_entry->encap);
        if(res_code!=PAE_RES_OK){/*R5 did not response OK code*/
            soc_pae_lue_idx_free(unit, PAE_DB_NUMBER, p_entry->idx);
            sal_free(p_entry);
            return BCM_E_FAIL;
        }

        if(soc_pae_lue_rule_add(unit, PAE_DB_NUMBER,p_entry->key,7*8,p_entry->data,p_entry->idx)!=BCM_E_NONE){
            sal_free(p_entry);
            return BCM_E_FAIL;
        }
    }

    _BCM_ROBO_MPLS_LINK_ADD((&_bcm_robo_mpls_lookup_table[unit]), p_entry);

    return BCM_E_NONE; 
}


/*
 * Function:
 *      _bcm_robo_mpls_lookup_entry_from_nni_destroy
 * Description:
 *      remove the lookup entry which is from NNI port
 *      from the local link list and free the memory.
 * Parameters:
 *      unit                - (IN) BCM device number
 *      act                 - (IN) swap and pop supported here
 *      p_switch_entry      - (IN) pointer to switch entry
 * Returns     : 
 *      BCM_E_XXX
 */
static int
_bcm_robo_mpls_lookup_entry_from_nni_destroy(int unit,  
                                                        _bcm_robo_mpls_switch_entry_t *p_switch_entry)
{
    _bcm_robo_mpls_lookup_entry_t *p_entry = NULL;
    bcm_mpls_switch_action_t act=p_switch_entry->action;

    if(act == BCM_MPLS_SWITCH_ACTION_SWAP){
        _BCM_ROBO_MPLS_LINK_SRCH_2_KEY((&_bcm_robo_mpls_lookup_table[unit]), port, p_switch_entry->ing_port_id,
            vp_mpls_label, p_switch_entry->ing_vp_mpls_label, p_entry);
        if(!p_entry){
            return BCM_E_NOT_FOUND;
        }

        soc_pae_lue_rule_del(unit, 0, p_entry->idx);
        _BCM_ROBO_MPLS_LINK_DEL((&_bcm_robo_mpls_lookup_table[unit]), p_entry);
    }
    else {/*pop*/
        _BCM_ROBO_MPLS_LINK_SRCH_2_KEY((&_bcm_robo_mpls_lookup_table[unit]), port, p_switch_entry->ing_port_id,
            vpn, p_switch_entry->vpn, p_entry);

        if(!p_entry){
            return BCM_E_NOT_FOUND;
        }
        soc_pae_lue_rule_del(unit, 0, p_entry->idx);
        _BCM_ROBO_MPLS_LINK_DEL((&_bcm_robo_mpls_lookup_table[unit]), p_entry);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_robo_mpls_lookup_entry_add
 * Description:
 *      when create port and add it to vpn, check whether the lookup 
 *      table also needed to be updated(create).
 *      basic logical:
 *              if both uni & nni ports are added into vpn, 
 *                  check whether switch entry exists, 
 *                      if exists, add the lookup entry (UNI->NNI, NNI->UNI) bidirection entries;
 *                      else only add (UNI->NNI) unidirection entry.
 *              else do nothing.
 * Parameters:
 *      unit   - (IN) BCM device number
 *      p_vpn  - (IN) pointer to the vpn object
  * Returns     : 
 *      BCM_E_XXX
 */
static int
_bcm_robo_mpls_lookup_entry_add(int unit, _bcm_robo_mpls_vpws_vpn_t *p_vpn)
{
    _bcm_robo_mpls_switch_entry_t *p_switch_entry = NULL;
    _bcm_robo_mpls_port_t         *p_port = NULL;
    int rv = BCM_E_NONE;

    /* Only when both ports are in the vpn, the lookup entries should be added. */
    if((p_vpn->nni_port_index == _MPLS_VPN_PORT_INDEX_INVALID) || (p_vpn->uni_port_index == _MPLS_VPN_PORT_INDEX_INVALID)){
        return BCM_E_NONE;
    }

    /* 
        1. If switch_entry exists for the vpn, add lookup entries for both (UNI->NNI) and (NNI->UNI) directions.
        2. If switch_entry not exists for the vpn, add lookup entry for (UNI->NNI) direction.
    */
    _BCM_ROBO_MPLS_LINK_SRCH_1_KEY((&_bcm_robo_mpls_ports[unit]), port_index, p_vpn->nni_port_index, p_port);
    if(!p_port){
        return BCM_E_PARAM;
    }

    rv = _bcm_robo_mpls_lookup_entry_from_uni_create(unit, p_vpn, p_port);
    if(BCM_E_NONE != rv){
        return rv;
    }

    _BCM_ROBO_MPLS_LINK_SRCH_1_KEY((&_bcm_robo_mpls_switch_table[unit]), vpn, p_vpn->vpn, p_switch_entry);
    if(p_switch_entry){
        /*need to check the enry already in the hw table ?*/
        rv = _bcm_robo_mpls_lookup_entry_from_nni_create(unit, p_switch_entry);
        if(rv != BCM_E_NONE){
            _bcm_robo_mpls_lookup_entry_from_uni_destroy(unit, p_vpn);
        }
    }

    return rv;
}

/*
 * Function:
 *      _bcm_robo_mpls_lookup_entry_add
 * Description:
 *      when delete port and remove it from vpn, check whether the lookup 
 *      table also needed to be updated(delete).
 *      basic logical:
 *              if both uni & nni ports are added into vpn, 
 *                  check whether switch entry exists, 
 *                      if exists, remove the lookup entry (UNI->NNI, NNI->UNI) bidirection entries;
 *                      else only delete (UNI->NNI) unidirection entry.
 *              else do nothing.
 * Parameters:
 *      unit   - (IN) BCM device number
 *      p_vpn  - (IN) pointer to the vpn object
  * Returns     : 
 *      BCM_E_XXX
 */
static int
_bcm_robo_mpls_lookup_entry_del(int unit, _bcm_robo_mpls_vpws_vpn_t *p_vpn)
{
    _bcm_robo_mpls_switch_entry_t *p_switch_entry = NULL;

    /* Only when both ports are in the vpn, the lookup entry should be deleted. */
    if((p_vpn->nni_port_index == _MPLS_VPN_PORT_INDEX_INVALID) || (p_vpn->uni_port_index == _MPLS_VPN_PORT_INDEX_INVALID)){
        return BCM_E_NONE;
    }

    /* Remove all the lookup entries. */
    _bcm_robo_mpls_lookup_entry_from_uni_destroy(unit, p_vpn);

    _BCM_ROBO_MPLS_LINK_SRCH_1_KEY((&_bcm_robo_mpls_switch_table[unit]), vpn, p_vpn->vpn, p_switch_entry);
    if(p_switch_entry){
        return (_bcm_robo_mpls_lookup_entry_from_nni_destroy(unit, p_switch_entry));
    }

    return BCM_E_NONE;
}
#endif
#endif
#endif

/*
 * Function:
 *      bcm_robo_mpls_vpn_id_create
 * Purpose:
 *      Create a VPN
 * Parameters:
 *      unit  - (IN)  Device Number
 *      info  - (IN/OUT) VPN configuration info
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_mpls_vpn_id_create(int unit, bcm_mpls_vpn_config_t *info)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    _bcm_robo_mpls_vpws_vpn_t *p_vpn = NULL;

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    if(!(info->flags & BCM_MPLS_VPN_VPWS)){
        return BCM_E_UNAVAIL;
    }
    
    _BCM_ROBO_MPLS_LINK_SRCH_1_KEY((&_bcm_robo_mpls_vpns[unit]), vpn, info->vpn, p_vpn);
    if(p_vpn){
        return BCM_E_EXISTS;
    }

    p_vpn = sal_alloc(sizeof(_bcm_robo_mpls_vpws_vpn_t),"robo mpls vpn entry");
    if(!p_vpn){
        return BCM_E_MEMORY;
    }

    p_vpn->vpn            = info->vpn;
    p_vpn->uni_port_index = _MPLS_VPN_PORT_INDEX_INVALID;
    p_vpn->nni_port_index = _MPLS_VPN_PORT_INDEX_INVALID;

    _BCM_ROBO_MPLS_LINK_ADD((&_bcm_robo_mpls_vpns[unit]), p_vpn);

    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */
        
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */
        
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */

}

/*
 * Function:
 *      bcm_robo_mpls_vpn_id_destroy
 * Purpose:
 *      Destroy a VPN
 * Parameters:
 *      unit   - (IN)  Device Number
 *      vpn    - (IN)  VPN instance
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_mpls_vpn_id_destroy(int unit, bcm_vpn_t vpn)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    _bcm_robo_mpls_vpws_vpn_t *p_vpn = NULL;

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    _BCM_ROBO_MPLS_LINK_SRCH_1_KEY((&_bcm_robo_mpls_vpns[unit]), vpn, vpn, p_vpn);
    if(!p_vpn){
        return BCM_E_NOT_FOUND;
    }

    /* If the vpn is referenced by port, it can't be destroyed. */
    if((p_vpn->nni_port_index != _MPLS_VPN_PORT_INDEX_INVALID) ||
       (p_vpn->uni_port_index != _MPLS_VPN_PORT_INDEX_INVALID) ){
        return BCM_E_UNAVAIL;
    }
    
    _BCM_ROBO_MPLS_LINK_DEL((&_bcm_robo_mpls_vpns[unit]), p_vpn);

    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */
        
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */
        
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */

}

/*
 * Function:
 *      bcm_robo_mpls_vpn_id_destroy_all
 * Purpose:
 *      Destroy all VPNs
 * Parameters:
 *      unit   - (IN)  Device Number
 * Returns:
 *      BCM_E_XXX
 */

/*#include <stdio.h> */
int
bcm_robo_mpls_vpn_id_destroy_all(int unit)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    int rv = BCM_E_NONE;
    _bcm_robo_mpls_vpws_vpn_t *p_vpn = _bcm_robo_mpls_vpns[unit].p_next;

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    while(p_vpn){
        rv = bcm_robo_mpls_vpn_id_destroy(unit, p_vpn->vpn);
        if(BCM_E_NONE != rv)
            return rv;

        /* _bcm_robo_mpls_vpns[unit].p_next has been modified in the previous operation. */
        p_vpn = _bcm_robo_mpls_vpns[unit].p_next;          
     }
    return rv;
#else
    return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */
        
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */
        
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */

}

/*
 * Function:
 *      bcm_robo_mpls_vpn_id_get
 * Purpose:
 *      Get a VPN
 * Parameters:
 *      unit  - (IN)  Device Number
 *      vpn   - (IN)  VPN instance
 *      info  - (OUT) VPN configuration info
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_mpls_vpn_id_get(int unit, bcm_vpn_t vpn, bcm_mpls_vpn_config_t *info)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    _bcm_robo_mpls_vpws_vpn_t *p_vpn;

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    _BCM_ROBO_MPLS_LINK_SRCH_1_KEY((&_bcm_robo_mpls_vpns[unit]), vpn, vpn, p_vpn);
    if(!p_vpn){
        return BCM_E_NOT_FOUND;
    }

    info->vpn    = p_vpn->vpn;
    info->flags |= BCM_MPLS_VPN_VPWS;

    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */
        
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */
        
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */

}

/*
 * Function:
 *      bcm_robo_mpls_port_add
 * Purpose:
 *      Add an mpls port to a VPN
 * Parameters:
 *      unit      - (IN) Device Number
 *      vpn       - (IN) VPN instance ID
 *      mpls_port - (IN/OUT) mpls port information (OUT : mpls_port_id)
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_mpls_port_add(int unit, bcm_vpn_t vpn, bcm_mpls_port_t *mpls_port)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    int rv = BCM_E_NONE;
    _mpls_port_type_t         port_type;
    _bcm_robo_mpls_port_t     *p_port = NULL;
    _bcm_robo_mpls_vpws_vpn_t *p_vpn  = NULL;

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    /*check the parameter of port and get the port type*/
    rv= _bcm_robo_mpls_port_type_get(mpls_port, &port_type);
    if(BCM_E_NONE != rv){
        return rv;
    }

    /* Get the vpn. If the vpn not exists, return error */
    p_vpn = _bcm_robo_mpls_vpn_get(unit, vpn);
    if(!p_vpn){
        return BCM_E_NOT_FOUND;
    }

    /*check if the port to be added is alreay exist*/
    rv = _bcm_robo_mpls_vpn_port_valid_check(p_vpn, port_type);
    if(BCM_E_NONE != rv){
        return rv;
    }

    /* Get the port. If the port not exists, create it. */
    p_port = _bcm_robo_mpls_port_find(unit, mpls_port);
    if(!p_port){
        p_port = _bcm_robo_mpls_port_create(unit, mpls_port);
        if(!p_port){
            return BCM_E_FAIL;
        }
    }

    /* Add the port to vpn. */
    if(port_type == MPLS_PORT_TYPE_UNI){
        p_vpn->uni_port_index = p_port->port_index;
    }
    else{
        p_vpn->nni_port_index = p_port->port_index;
    }
    
    /* Hardware adding logical hehe. */
    rv = _bcm_robo_mpls_lookup_entry_add(unit, p_vpn);
    if(rv != BCM_E_NONE){
        _bcm_robo_mpls_port_destroy(unit, p_port);
    }

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */
        
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */
        
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */

}

/*
 * Function:
 *      bcm_robo_mpls_port_delete
 * Purpose:
 *      Delete an mpls port from a VPN
 * Parameters:
 *      unit         - (IN) Device Number
 *      vpn          - (IN) VPN instance ID
 *      mpls_port_id - (IN) mpls port ID
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_mpls_port_delete(int unit, bcm_vpn_t vpn, bcm_gport_t mpls_port_id)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    int rv;
    _bcm_robo_mpls_port_t         *p_port         = _bcm_robo_mpls_ports[unit].p_next;
    _bcm_robo_mpls_vpws_vpn_t     *p_vpn          = NULL;
    _bcm_robo_mpls_switch_entry_t *p_switch_entry = NULL;
    _mpls_port_type_t             port_type;
    bcm_port_t                    local_port;

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    rv = bcm_robo_port_local_get(unit, mpls_port_id, &local_port);
    if(BCM_E_NONE != rv){
        return rv;
    }

    p_vpn = _bcm_robo_mpls_vpn_get(unit, vpn);
    if(!p_vpn){
        return BCM_E_NOT_FOUND;
    }

    while(p_port){
        if((p_port->port_id == local_port) && (p_port->port_type == MPLS_PORT_TYPE_UNI) 
            &&(p_port->port_index == p_vpn->uni_port_index)){
            port_type = MPLS_PORT_TYPE_UNI;
            break;
        }
    
        if((p_port->port_id == local_port) && (p_port->port_type == MPLS_PORT_TYPE_NNI) 
            &&(p_port->port_index == p_vpn->nni_port_index)){
            port_type = MPLS_PORT_TYPE_NNI;

            /* Ensure the mpls switch table should be removed before NNI port is removed. */
            _BCM_ROBO_MPLS_LINK_SRCH_2_KEY((&_bcm_robo_mpls_switch_table[unit]), vpn, p_vpn->vpn, 
                action, BCM_MPLS_SWITCH_ACTION_POP, p_switch_entry);
            if(p_switch_entry){
                return BCM_E_FAIL;
            }

            break;
        }

        p_port = p_port->p_next;
    }

    if(!p_port){
        return BCM_E_NOT_FOUND;
    }

    _bcm_robo_mpls_lookup_entry_del(unit, p_vpn);

    if(port_type == MPLS_PORT_TYPE_NNI){
        p_vpn->nni_port_index = _MPLS_VPN_PORT_INDEX_INVALID;
    }
    else{
        p_vpn->uni_port_index = _MPLS_VPN_PORT_INDEX_INVALID;
    }

    _bcm_robo_mpls_port_destroy(unit, p_port);
    
    return rv;
#else
    return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */
        
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */
        
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */

}

/*
 * Function:
 *      bcm_robo_mpls_port_delete_all
 * Purpose:
 *      Delete all mpls ports from a VPN
 * Parameters:
 *      unit - (IN) Device Number
 *      vpn  - (IN) VPN instance ID
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_mpls_port_delete_all(int unit, bcm_vpn_t vpn)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    int rv;
    _bcm_robo_mpls_port_t         *p_port  = NULL;
    _bcm_robo_mpls_vpws_vpn_t     *p_vpn   = NULL;
    _bcm_robo_mpls_switch_entry_t *p_switch_entry = NULL;

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }
    
    p_vpn = _bcm_robo_mpls_vpn_get(unit, vpn);
    if(!p_vpn){
        return BCM_E_NOT_FOUND;
    }

    rv = _bcm_robo_mpls_lookup_entry_del(unit, p_vpn);
    if(BCM_E_NONE != rv){
        return rv;
    }

    _BCM_ROBO_MPLS_LINK_SRCH_1_KEY((&_bcm_robo_mpls_ports[unit]), port_index, p_vpn->nni_port_index, p_port);
    if(p_port){
        /* Ensure the mpls switch table should be removed before NNI port is removed. */
        _BCM_ROBO_MPLS_LINK_SRCH_2_KEY((&_bcm_robo_mpls_switch_table[unit]), vpn, p_vpn->vpn, 
            action, BCM_MPLS_SWITCH_ACTION_POP, p_switch_entry);
        if(p_switch_entry){
            return BCM_E_FAIL;
        }

        _bcm_robo_mpls_port_destroy(unit, p_port);
    }
    
    _BCM_ROBO_MPLS_LINK_SRCH_1_KEY((&_bcm_robo_mpls_ports[unit]), port_index, p_vpn->uni_port_index, p_port);
    if(p_port){
        _bcm_robo_mpls_port_destroy(unit, p_port);
    }

    p_vpn->nni_port_index = _MPLS_VPN_PORT_INDEX_INVALID;
    p_vpn->uni_port_index = _MPLS_VPN_PORT_INDEX_INVALID;
    
    return rv;
#else
    return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */
        
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */
        
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */

}

/*
 * Function:
 *      bcm_robo_mpls_port_get_all
 * Purpose:
 *      Get an mpls port from a VPN
 * Parameters:
 *      unit       - (IN) Device Number
 *      vpn        - (IN) VPN instance ID
 *      port_max   - (IN) Maximum number of ports in array
 *      port_array - (OUT) Array of mpls ports
 *      port_count - (OUT) Number of ports returned in array
 *
 */
int
bcm_robo_mpls_port_get_all(int unit, bcm_vpn_t vpn, int port_max,
                           bcm_mpls_port_t *port_array, int *port_count)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    int rv = BCM_E_NONE; 
    bcm_mpls_port_t              *p_mpls_port = port_array;
    _bcm_robo_mpls_port_t         *p_port  = NULL;
    _bcm_robo_mpls_vpws_vpn_t     *p_vpn   = NULL;

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }
    
    p_vpn = _bcm_robo_mpls_vpn_get(unit, vpn);
    if(!p_vpn){
        return BCM_E_NOT_FOUND;
    }

    if(port_max <= 0){
        return BCM_E_PARAM;
    }

    *port_count = 0;

    if(p_vpn->uni_port_index != _MPLS_VPN_PORT_INDEX_INVALID){
        if(*port_count > port_max){
            return BCM_E_NONE;
        }

        _BCM_ROBO_MPLS_LINK_SRCH_1_KEY((&_bcm_robo_mpls_ports[unit]), port_index, p_vpn->uni_port_index, p_port);
        if(p_port){
            rv = _bcm_robo_mpls_port_conv_robo_2_bcm(unit, p_port, p_mpls_port);
            if(BCM_E_NONE != rv){
                return rv;
            }
            (*port_count)++;
            p_mpls_port++;
        }
    }

    if(p_vpn->nni_port_index != _MPLS_VPN_PORT_INDEX_INVALID){
        if(*port_count > port_max){
            return BCM_E_NONE;
        }

        _BCM_ROBO_MPLS_LINK_SRCH_1_KEY((&_bcm_robo_mpls_ports[unit]), port_index, p_vpn->nni_port_index, p_port);
        if(p_port){
            rv = _bcm_robo_mpls_port_conv_robo_2_bcm(unit, p_port, p_mpls_port);
            if(BCM_E_NONE != rv){
                return rv;
            }
            (*port_count)++;
            p_mpls_port++;
        }
    }

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */
        
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */
        
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */

}

/*
 * Function:
 *      bcm_robo_mpls_tunnel_initiator_set
 * Purpose:
 *      Set the MPLS tunnel initiator parameters for an L3 interface.
 * Parameters:
 *      unit        - (IN) Device Number
 *      intf        - (IN) The egress L3 interface
 *      num_labels  - (IN) Number of labels in the array
 *      label_array - (IN) Array of MPLS label and header information
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_mpls_tunnel_initiator_set(int unit, bcm_if_t intf, int num_labels, 
                                    bcm_mpls_egress_label_t *label_array)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    /* should l3 entry is created and found, if yes, keep the label info to the _bcm_robo_l3_table */
    _bcm_robo_l3_entry_t *p_entry = NULL;

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    p_entry = _bcm_robo_l3_intf_get(unit, intf);
    if(!p_entry){
        return BCM_E_NOT_FOUND;
    }
    
    if(num_labels != 1){
        return BCM_E_PARAM;
    }

    /*construct the label  as the 32bit format*/
    p_entry->vp_mpls_label = _bcm_robo_mpls_label_l2b_endian(label_array->label&0xFFFFF);
    if(label_array->flags & BCM_MPLS_EGRESS_LABEL_TTL_SET){
        p_entry->vp_mpls_label |= (label_array->ttl)<<24;
    }
    else{
        p_entry->vp_mpls_label |= 0xFF<<24;
    }

    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */
        
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */
        
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */

}

/*
 * Function:
 *      bcm_robo_mpls_tunnel_switch_add
 * Purpose:
 *      Add an MPLS label entry.
 * Parameters:
 *      unit - (IN) Device Number
 *      info - (IN) Label (switch) information
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_mpls_tunnel_switch_add(int unit, bcm_mpls_tunnel_switch_t *info)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    _bcm_robo_mpls_switch_entry_t *p_switch_entry = NULL;

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    p_switch_entry = _bcm_robo_mpls_switch_entry_get(unit, info);
    /* Already exists. */
    if(p_switch_entry){
        return BCM_E_EXISTS;
    }

    p_switch_entry = _bcm_robo_mpls_switch_entry_create(unit, info);
    if(!p_switch_entry){
        return BCM_E_FAIL;
    }

    /*add the hw entry to lookup table*/
    return (_bcm_robo_mpls_lookup_entry_from_nni_create(unit, p_switch_entry));
#else
    return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */
        
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */
        
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */ 
}

/*
 * Function:
 *      bcm_robo_mpls_tunnel_switch_delete
 * Purpose:
 *      Delete an MPLS label entry.
 * Parameters:
 *      unit - (IN) Device Number
 *      info - (IN) Label (switch) information
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_mpls_tunnel_switch_delete(int unit, bcm_mpls_tunnel_switch_t *info)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    _bcm_robo_mpls_switch_entry_t *p_switch_entry = NULL;

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    p_switch_entry = _bcm_robo_mpls_switch_entry_get(unit, info);
    if(!p_switch_entry){
        return BCM_E_NOT_FOUND;
    }

    /* Ignore the operation for lookup entry since it maybe destroyed in mpls_port_delete. */
    _bcm_robo_mpls_lookup_entry_from_nni_destroy(unit, p_switch_entry);

    _bcm_robo_mpls_switch_entry_destroy(unit, p_switch_entry);

    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */
            
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */
            
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */

}

/*
 * Function:
 *      bcm_robo_mpls_tunnel_switch_delete_all
 * Purpose:
 *      Delete all MPLS label entries.
 * Parameters:
 *      unit   - (IN) Device Number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_mpls_tunnel_switch_delete_all(int unit)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    _bcm_robo_mpls_switch_entry_t *p_switch_entry = &_bcm_robo_mpls_switch_table[unit];

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    while(p_switch_entry){
        /* Ignore the operation for lookup entry since it maybe destroyed in mpls_port_delete. */
        _bcm_robo_mpls_lookup_entry_from_nni_destroy(unit,p_switch_entry);

        _bcm_robo_mpls_switch_entry_destroy(unit, p_switch_entry);

        p_switch_entry = &_bcm_robo_mpls_switch_table[unit];
    }

    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */
            
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */
            
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */

}

/*
 * Function:
 *      bcm_robo_mpls_tunnel_switch_get
 * Purpose:
 *      Get an MPLS label entry.
 * Parameters:
 *      unit - (IN) Device Number
 *      info - (IN) Label (switch) information
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_mpls_tunnel_switch_get(int unit, bcm_mpls_tunnel_switch_t *info)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    int rv = BCM_E_NONE;
    _bcm_robo_mpls_switch_entry_t *p_switch_entry = _bcm_robo_mpls_switch_entry_get(unit, info);
    bcm_gport_t  gport;

    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    if(!p_switch_entry){
        return BCM_E_NOT_FOUND;
    }

    rv = bcm_robo_port_gport_get(unit, p_switch_entry->ing_port_id, &gport);
    if(BCM_E_NONE != rv){
        return rv;
    }

    switch(p_switch_entry->action){
        case BCM_MPLS_SWITCH_ACTION_POP:
            info->action = p_switch_entry->action;
            info->label  = p_switch_entry->ing_vp_mpls_label;
            info->vpn    = p_switch_entry->vpn;
            info->port   = gport;

            info->vpn    = p_switch_entry->vpn;
            break;

        case BCM_MPLS_SWITCH_ACTION_SWAP:
            info->action = p_switch_entry->action;
            info->label  = p_switch_entry->ing_vp_mpls_label;
            info->vpn    = p_switch_entry->vpn;
            info->port   = gport;
            
            info->egress_if          = p_switch_entry->egr_intf_index;
            info->egress_label.label = (p_switch_entry->egr_vp_mpls_label >> 12);
            info->egress_label.ttl   = (p_switch_entry->egr_vp_mpls_label & 0xFF);
            break;

        default:

            return BCM_E_PARAM;
    }

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */
            
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */
            
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */

}

/*
 * Function:
 *      bcm_mpls_init
 * Purpose:
 *      Initialize the MPLS software module
 * Parameters:
 *      unit     - Device Number
 * Returns:
 *      BCM_E_XXXX
 */


int
bcm_robo_mpls_init(int unit)
{
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#ifdef LINUX
#if !defined(__KERNEL__)
    if (!SOC_IS_NORTHSTARPLUS(unit)) {
        return BCM_E_UNAVAIL;
    }
    
    return soc_pae_lue_db_config(
        unit,
        PAE_DB_NUMBER,
        PAE_DB_ENABLE_FLAG,
        PAE_RULE_SET,
        PAE_RULE_BASE,
        PAE_META_BASE,
        PAE_KEY_TYPE,
        6*8);
#else
    return BCM_E_UNAVAIL;
#endif /* endif !defined(__KERNEL__) */
            
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef LINUX */
            
#else
    return BCM_E_UNAVAIL;
#endif /* endif ifdef BCM_NORTHSTARPLUS_SUPPORT */

}



