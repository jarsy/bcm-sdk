/*
 * $Id: wred.c,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 *  Field driver service.
 *  Purpose: Handle the chip variant design for Field Processor
 * 
 */

#include <soc/error.h>
#include <soc/types.h>
#include <soc/drv.h>
#include <soc/drv_if.h>
#include <soc/robo/northstarplus_service.h>
#include "robo_northstarplus.h"


/* Reserve the WRED profile entry 0 for thoes that didn't enable the WRED */
#define _NSP_WRED_PROFILE_ZERO_RESERVED         (1)

/* Invalid default wred profile index */
#define _NSP_DEFAULT_WRED_PROFILE_INVALID       (-1)


#define MAX_HW_DROP_VALUE       (15)
/*
 * index : chip drop value
 * Max drop probability table at MAX threshold
 */
STATIC int
_drv_nsp_max_drop_prob_table[] =
{
    0, 6, 13, 19, 25, 31, 38, 44,
    50, 56, 63, 69, 75, 81, 88, 94
};


STATIC uint32   wred_usage;
STATIC drv_wred_config_t drv_wred[DRV_NORTHSTARPLUS_WRED_NUM];
STATIC int      wred_default_index;
STATIC int      drv_wred_profile_index[SOC_ROBO_MAX_NUM_PORTS][DRV_NORTHSTARPLUS_COS_QUEUE_NUM][4];

#define _DRV_NSP_WRED_TABLE_OP_READ     (0)
#define _DRV_NSP_WRED_TABLE_OP_WRITE    (1)


/* Get the maximum cells of  the queue */ 
STATIC int
_drv_nsp_queue_max_cell_get(int unit, soc_port_t port, int queue, 
            uint32 *cell)
{
    uint32  reg_val, fld_val = 0;
    int     q_id;

    /* Check it is global mode or not */
    SOC_IF_ERROR_RETURN(
        REG_READ_FC_CTRL_MODEr(unit, &reg_val));
    soc_FC_CTRL_MODEr_field_get(unit, &reg_val, FC_MODEf, &fld_val);
    
    if (fld_val) {
        /* Select the port */
        SOC_IF_ERROR_RETURN(
            REG_READ_FC_CTRL_PORTr(unit, &reg_val));
        if (port == -1) {
            fld_val = 0;
        } else {
            fld_val = port;
        }
        soc_FC_CTRL_PORTr_field_set(unit, &reg_val, FC_PORT_SELf, &fld_val);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_FC_CTRL_PORTr(unit, &reg_val));
    }

    if (queue == -1) {
        q_id = 0;
    } else {
        q_id = queue;
    }
    SOC_IF_ERROR_RETURN(
        REG_READ_FC_LAN_TXQ_THD_DROP_QNr(unit, q_id, &reg_val));
    soc_FC_LAN_TXQ_THD_DROP_QNr_field_get(unit, &reg_val, 
        TXQ_DROP_THDf, &fld_val);
    *cell = fld_val;

    return SOC_E_NONE;
    
}



STATIC int
_drv_nsp_wred_map_set(int unit, uint32 wred_idx, drv_wred_map_info_t *map)
{
    int i, j, port;
    uint32                  reg_val, temp, old_idx = 0;
    uint8                   tc_map, dei, flow_mark_map;
    pbmp_t                  pbmp;
    
    SOC_PBMP_CLEAR(pbmp);
    if (map->port == -1) {
        SOC_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
    } else {
        SOC_PBMP_PORT_ADD(pbmp, map->port);
    }
    
    SOC_IF_ERROR_RETURN(
        REG_READ_PN_TC2COS_MAPr(unit, map->port, &reg_val));
    tc_map = 0;
    if (map->cosq == -1) {
        tc_map = 0xff;
    } else {
        for (i = 0; i < 8; i++) {
            temp = (reg_val >> (i * 3)) & 0x7; 
            if (temp == map->cosq) {
                tc_map |= (0x1 << i);
            }
        }
    }

    /* bit 0 : flow mark 0, bit 1: flow mark 1 */
    if ((map->flags & DRV_WRED_MAP_FLAGS_COLOR_ALL) == 
               DRV_WRED_MAP_FLAGS_COLOR_ALL) {
        flow_mark_map = 3;
    } else if ((map->flags & DRV_WRED_MAP_FLAGS_COLOR_RED) ||
        (map->flags & DRV_WRED_MAP_FLAGS_COLOR_YELLOW)) {
        flow_mark_map = 2;
    } else {
        flow_mark_map = 1;
    }

    if (map->flags & DRV_WRED_MAP_FLAGS_DEI) {
        dei = 1;
    } else {
        dei = 0;
    }

    /* Write the mapping table */
    SOC_PBMP_ITER(pbmp, port) {
        if (!SOC_PORT_VALID(unit, port)) { 
            return SOC_E_PORT;
        }
        for (i = 0; i <= DRV_NORTHSTARPLUS_TC_MAX; i++) {
            if (tc_map & (0x1 << i)) {
                for (j = 0; j < 2; j++) {
                    if (flow_mark_map & (0x1 << j)) {
                        SOC_IF_ERROR_RETURN(
                            REG_READ_TC2RED_PROFILE_TABLEr(unit, &reg_val));
                        /* 
                                         * bit 4 : flow mark
                                         * bit 5 : DEI bit
                                         * bit 6 ~ 8 : TC
                                         * bit 9 ~ 12 : port number
                                         */
                        temp = (port << 5) | (i << 2) | (dei << 1) | j;
                        soc_TC2RED_PROFILE_TABLEr_field_set(unit, &reg_val,
                            TC2RED_TABLE_ADDRf, &temp);
                        temp = wred_idx;
                        soc_TC2RED_PROFILE_TABLEr_field_set(unit, &reg_val,
                            TC2RED_TABLE_DATAf, &temp);
                        temp = _DRV_NSP_WRED_TABLE_OP_WRITE;
                        soc_TC2RED_PROFILE_TABLEr_field_set(unit, &reg_val,
                            TC2RED_TABLE_WR_RDf, &temp);
                        SOC_IF_ERROR_RETURN(
                            REG_WRITE_TC2RED_PROFILE_TABLEr(unit, &reg_val));
                        /* Remove the old profile index */
                        old_idx = drv_wred_profile_index[port][i][(dei << 1 | j)];
                        if (wred_usage & (0x1 << old_idx)) {
                            drv_wred[old_idx].refer_count--;
                        }
                        /* record the new profile index value */
                        drv_wred_profile_index[port][i][(dei << 1 | j)] = 
                            wred_idx;
                        drv_wred[wred_idx].refer_count++;
                        
                    }
                }
            }
        }
    }
    
    return SOC_E_NONE;
    
}

STATIC int
_drv_nsp_wred_config_set(int unit, int index, drv_wred_config_t *config)
{
    uint32  reg_val, fld_val;
    uint32  cell;
    int i;

    if (index >= DRV_NORTHSTARPLUS_WRED_NUM) {
        return SOC_E_PARAM;
    }

    /* Check the parameters */
    if (config->max_threshold < config->min_threshold) {
        return SOC_E_PARAM;
    }

    if (index != wred_default_index) {

        /* Check if max > queue max depth */
        SOC_IF_ERROR_RETURN(
            _drv_nsp_queue_max_cell_get(unit, -1, -1, &cell));
        cell *= DRV_NORTHSTARPLUS_CELL_UNIT; 
        if (config->max_threshold > cell) {
            return SOC_E_PARAM;
        } 
    }

    /* Set the configuration into the chip */
    SOC_IF_ERROR_RETURN(
        REG_READ_RED_PROFILE_Nr(unit, index, &reg_val));
 
    /* Configure HW drop value */
    for (i = MAX_HW_DROP_VALUE; i >= 0; i--) {
        if (config->drop_prob >= _drv_nsp_max_drop_prob_table[i]) {
            break;
        }
    }
    fld_val = i;
    soc_RED_PROFILE_Nr_field_set(unit, &reg_val, RED_DROP_PROBf, &fld_val);

    /* MAX threshold */
    fld_val = config->max_threshold / DRV_NORTHSTARPLUS_CELL_UNIT;
    soc_RED_PROFILE_Nr_field_set(unit, &reg_val, RED_MAX_THDf, &fld_val);

    /* MIN Threshold */
    fld_val = config->min_threshold / DRV_NORTHSTARPLUS_CELL_UNIT;
    soc_RED_PROFILE_Nr_field_set(unit, &reg_val, RED_MIN_THDf, &fld_val);
    SOC_IF_ERROR_RETURN(
        REG_WRITE_RED_PROFILE_Nr(unit, index, &reg_val));

    /* Check if the gain value is different with the original one */
    SOC_IF_ERROR_RETURN(
        drv_nsp_wred_control_get(unit, -1, -1,
            DRV_WRED_CONTROL_AQD_EXPONENT, &fld_val));
    if (fld_val != config->gain) {
        fld_val = config->gain;
        SOC_IF_ERROR_RETURN(
            drv_nsp_wred_control_set(unit, -1, -1,
                DRV_WRED_CONTROL_AQD_EXPONENT, fld_val));
    }
    
    return SOC_E_NONE;
}


STATIC int
_drv_nsp_wred_default_index_set(int unit, int index)
{
    uint32 reg_val, fld_val;
    if (index >= DRV_NORTHSTARPLUS_WRED_NUM) {
        return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(
        REG_READ_RED_PROFILE_DEFAULTr(unit, &reg_val));
    fld_val = index;
    soc_RED_PROFILE_DEFAULTr_field_set(unit, &reg_val,
        RED_PROFILE_DEFAULTf, &fld_val);
    SOC_IF_ERROR_RETURN(
        REG_WRITE_RED_PROFILE_DEFAULTr(unit, &reg_val));
    
    return SOC_E_NONE;
}


int
drv_nsp_wred_init(int unit)
{
#if _NSP_WRED_PROFILE_ZERO_RESERVED
    uint32 reg_val = 0, cell_num = 0;
#endif /* _NSP_WRED_PROFILE_ZERO_RESERVED */

    /* Disable WRED by default */
    SOC_IF_ERROR_RETURN(
        drv_nsp_wred_control_set(unit, -1, -1, 
        DRV_WRED_CONTROL_ENABLE, FALSE));
    /* Initialize the WRED software data structure */
    wred_usage = 0;
    sal_memset(drv_wred, 0, 
        (sizeof(drv_wred_config_t) * DRV_NORTHSTARPLUS_WRED_NUM));
    wred_default_index = _NSP_DEFAULT_WRED_PROFILE_INVALID;
    sal_memset(drv_wred_profile_index, 0,
        (sizeof(drv_wred_profile_index)));
#if _NSP_WRED_PROFILE_ZERO_RESERVED
    /* Reserve the profile for WRED disabled case */ 
    wred_usage |= 0x1;

    /* Set the max threshold as max cell value */
    SOC_IF_ERROR_RETURN(
        _drv_nsp_queue_max_cell_get(unit, 0, 0, &cell_num));
    soc_RED_PROFILE_Nr_field_set(unit, &reg_val, RED_MAX_THDf, &cell_num);
    SOC_IF_ERROR_RETURN(
        REG_WRITE_RED_PROFILE_Nr(unit, 0, &reg_val));
    drv_wred[0].refer_count = (SOC_ROBO_MAX_NUM_PORTS * 
        DRV_NORTHSTARPLUS_COS_QUEUE_NUM * 4);
#endif /* _NSP_WRED_PROFILE_ZERO_RESERVED */

    return SOC_E_NONE;
}


/* Allocation a WRED profile or find a WRED profile with the same configuration */
int
drv_nsp_wred_config_create(int unit, uint32 flags, 
            drv_wred_config_t *config, int *wred_id)
{
    int i;
    int found;

    if (config == NULL) {
        return SOC_E_PARAM;
    }

    found = 0;
    
    /* Find an unused wred profile */ 
    for (i = 0; i < DRV_NORTHSTARPLUS_WRED_NUM; i++) {
        if (!(wred_usage & (0x1 << i))) {
            continue;
        }
        
        if ((drv_wred[i].drop_prob == config->drop_prob) &&
            (drv_wred[i].max_threshold == config->max_threshold) &&
            (drv_wred[i].min_threshold == config->min_threshold)) {
            *wred_id = i;
            found = TRUE; 
            break;
        }
    }

    /* Find an unused wred profile */ 
    if (!found) {
        for (i = 0; i < DRV_NORTHSTARPLUS_WRED_NUM; i++) {
            if (!(wred_usage & (0x1 << i))) {
                wred_usage |= (0x1 << i);
                break;
            }
        }

        if (i == DRV_NORTHSTARPLUS_WRED_NUM) { 
            return SOC_E_RESOURCE;
        }

        sal_memcpy(&(drv_wred[i]), config, sizeof(drv_wred_config_t));
    } 

    /* If the flag of default profile is set, configure the default profile index */ 
    if (config->flags & DRV_WRED_CONFIG_FLAGS_DEFAULT_PROFILE) {
        wred_default_index = i;
        SOC_IF_ERROR_RETURN(
            _drv_nsp_wred_default_index_set(
                        unit, wred_default_index));
    }

    if (found) { 
        return SOC_E_EXISTS;
    }
 
    /* refer_count and map list should be empty */
    drv_wred[i].hw_index = i;
    drv_wred[i].refer_count = 0;

    /* Write the configuration back */
    sal_memcpy(config, &(drv_wred[i]), sizeof(drv_wred_config_t));
 
    *wred_id = i;

    return SOC_E_NONE;
    
}


/* Configure the created WRED profile */
int
drv_nsp_wred_config_set(int unit, int wred_id, drv_wred_config_t *config)
{
    drv_wred_config_t       *drv_config;
    if ((config == NULL) || (wred_id >= DRV_NORTHSTARPLUS_WRED_NUM))  {
        return SOC_E_PARAM;
    }

    if ((wred_usage & (0x1 << wred_id)) == 0) {
        return SOC_E_PARAM;
    }

    drv_config = &drv_wred[wred_id];
    /* Check if the refer_count and map are empty */
    if (drv_config->refer_count != 0) {
        return SOC_E_PARAM;
    }

    if (config->flags & DRV_WRED_CONFIG_FLAGS_DEFAULT_PROFILE) {
        if (wred_default_index != wred_id) {
            wred_default_index = wred_id;
            SOC_IF_ERROR_RETURN(
                _drv_nsp_wred_default_index_set(
                            unit, wred_default_index));
        }
    }

    drv_config->drop_prob = config->drop_prob;
    drv_config->max_threshold = config->max_threshold;
    drv_config->min_threshold = config->min_threshold;
    SOC_IF_ERROR_RETURN(
        _drv_nsp_wred_config_set(unit, wred_id, config));

    return SOC_E_NONE;
}


/* Retrieve the configuration of the WRED profile */
int
drv_nsp_wred_config_get(int unit, int wred_id, drv_wred_config_t *config)
{
    if ((config == NULL) || (wred_id >= DRV_NORTHSTARPLUS_WRED_NUM))  {
        return SOC_E_PARAM;
    }

    if ((wred_usage & (0x1 << wred_id)) == 0) {
        return SOC_E_PARAM;
    }

    sal_memcpy(config, &drv_wred[wred_id], sizeof(drv_wred_config_t));
    return SOC_E_NONE;
}


/* Free the WRED profile */
int
drv_nsp_wred_config_destroy(int unit, int wred_id)
{
    drv_wred_config_t       *drv_config;
    uint32      reg_val, fld_val;
    
    if (wred_id >= DRV_NORTHSTARPLUS_WRED_NUM)  {
        return SOC_E_PARAM;
    }

    if ((wred_usage & (0x1 << wred_id)) == 0) {
        return SOC_E_NONE;
    }

    drv_config = &drv_wred[wred_id];
    /* Check if the refer_count and map are empty */
    if (drv_config->refer_count != 0) {
        return SOC_E_PARAM;
    }

    wred_usage &= ~(0x1 << wred_id);

    /* Invalid Default WRED index */
    if (wred_id == wred_default_index) {
        wred_default_index = _NSP_DEFAULT_WRED_PROFILE_INVALID;
    }
    
    /* set probability to zero */
    SOC_IF_ERROR_RETURN(
        REG_READ_RED_PROFILE_Nr(unit, drv_config->hw_index, &reg_val));
    fld_val = 0;
    soc_RED_PROFILE_Nr_field_set(unit, &reg_val, RED_DROP_PROBf, &fld_val);
    /* Max threshold as max cell number */
    SOC_IF_ERROR_RETURN(
        _drv_nsp_queue_max_cell_get(unit, 0, 0, &fld_val));
    soc_RED_PROFILE_Nr_field_set(unit, &reg_val, RED_MAX_THDf, &fld_val);
    SOC_IF_ERROR_RETURN(
        REG_WRITE_RED_PROFILE_Nr(unit, drv_config->hw_index, &reg_val));

    sal_memset(drv_config, 0, sizeof(drv_wred_config_t));
    
    return SOC_E_NONE;
    
}

int
drv_nsp_wred_map_attach(int unit, int wred_id, drv_wred_map_info_t *map)
{
    drv_wred_config_t       *drv_config;
    int                     rv = SOC_E_NONE;
 
    /* Parameter checking */
    if (((wred_usage & (0x1 << wred_id)) == 0) || 
        (wred_id >= DRV_NORTHSTARPLUS_WRED_NUM) ||
        (map == NULL))  { 
        return SOC_E_PARAM;
    } 

    drv_config = &drv_wred[wred_id];
   
 
    if (drv_config->refer_count == 0) {
        /* Set configuration into the chip */ 
        SOC_IF_ERROR_RETURN(
            _drv_nsp_wred_config_set(unit, wred_id, drv_config));        
    } 

    /* Configure the WRED mapping table */
    SOC_IF_ERROR_RETURN(
        _drv_nsp_wred_map_set(unit, drv_config->hw_index, map)); 

    /* Enable per-port WRED function */
    rv = drv_nsp_wred_control_set(unit, map->port, -1,
        DRV_WRED_CONTROL_ENABLE, TRUE);
    
    return rv;
}

int
drv_nsp_wred_map_deattach(int unit, int wred_id, drv_wred_map_info_t *map)
{
#ifndef _NSP_WRED_PROFILE_ZERO_RESERVED    
    int                     i, found;
    uint32                  wred_idx;
#endif /* _NSP_WRED_PROFILE_ZERO_RESERVED */    
    
    if (((wred_usage & (0x1 << wred_id)) == 0) || 
        (wred_id >= DRV_NORTHSTARPLUS_WRED_NUM) ||
        (map == NULL))  {
        return SOC_E_PARAM;
    }    

    /* Find an used WRED to map */
#if _NSP_WRED_PROFILE_ZERO_RESERVED
    SOC_IF_ERROR_RETURN(
        _drv_nsp_wred_map_set(unit, 0, map));
#else /* !_NSP_WRED_PROFILE_ZERO_RESERVED */
    found = FALSE;
    for (i = (DRV_NORTHSTARPLUS_WRED_NUM - 1); i >= 0; i--) {
        if (!(wred_usage & (0x1 << i))) {
            found = TRUE;
            wred_idx = i;
            break;
        }
    }

    if (found) {
        return (_drv_nsp_wred_map_set(unit, wred_idx, map));
    }
#endif /* _NSP_WRED_PROFILE_ZERO_RESERVED */    
  
    return SOC_E_NONE;
}

int
drv_nsp_wred_map_get(int unit, int *wred_id, drv_wred_map_info_t *map)
{
    int idx, port = 0, cosq = 0, dei = 0, flow_mark = 0;
    
    if (map == NULL)  {
        return SOC_E_PARAM;
    }

    /* Return the default wred index */
    if (map->flags & DRV_WRED_CONFIG_FLAGS_DEFAULT_PROFILE) {
        /* Check if the WRED profile is valid or not */
        if (_NSP_DEFAULT_WRED_PROFILE_INVALID != wred_default_index) {
            *wred_id = wred_default_index;
            return SOC_E_NONE;
        }
        return SOC_E_NOT_FOUND;
    }

    if (map->port == -1) {
        PBMP_E_ITER(unit, port) {
            break;
        }
    } else {
        port = map->port;
    }
    if (!SOC_PORT_VALID(unit, port)) { 
        return SOC_E_PORT;
    }
    if (port >= SOC_ROBO_MAX_NUM_PORTS) {
        return SOC_E_PORT;
    }
    
    if (map->cosq == -1) {
        cosq = 0;
    } else {
        cosq = map->cosq;
    }

    if (map->flags & DRV_WRED_MAP_FLAGS_DEI) {
        dei = 1;
    }

    if (map->flags & (DRV_WRED_MAP_FLAGS_COLOR_RED |
        DRV_WRED_MAP_FLAGS_COLOR_YELLOW)) {
        flow_mark = 1;
    }

    
    idx = drv_wred_profile_index[port][cosq][(dei << 1 | flow_mark)]; 
#if _NSP_WRED_PROFILE_ZERO_RESERVED
    if (idx == 0) {
        return SOC_E_NOT_FOUND;
    }
#endif /* _NSP_WRED_PROFILE_ZERO_RESERVED */

    *wred_id = idx;   

    return SOC_E_NONE;
}



int
drv_nsp_wred_control_set(int unit, soc_port_t port, int queue, 
            drv_wred_control_t type, uint32 value)
{
    uint32  reg_val, fld_val = 0;
    uint32  temp;


    /* Check the parameters */
    if (queue != -1) {
        /* For all queues */
        return SOC_E_PARAM;
    }

    if ((port != -1) &&
        ((type == DRV_WRED_CONTROL_AQD_PERIOD) ||
        (type == DRV_WRED_CONTROL_AQD_EXPONENT) ||
        (type == DRV_WRED_CONTROL_AQD_FAST_CORRECTION))) {
        /* Global control bit */
        return SOC_E_PARAM;
    }
    
    
    switch (type) {
        case DRV_WRED_CONTROL_ENABLE:
            SOC_IF_ERROR_RETURN(
                REG_READ_RED_CONTROLr(unit, &reg_val));
            soc_RED_CONTROLr_field_get(unit, &reg_val, RED_ENf, &fld_val);
            if (port == -1) {
                temp = SOC_PBMP_WORD_GET(PBMP_ALL(unit), 0);
            } else {
                temp = (0x1 << port);
            }
            if (value) {
                fld_val |= temp;
            } else {
                fld_val &= ~(temp); 
            }
            soc_RED_CONTROLr_field_set(unit, &reg_val, RED_ENf, &fld_val);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_RED_CONTROLr(unit, &reg_val));
            break;
        case DRV_WRED_CONTROL_AQD_PERIOD:
            if (value > DRV_NORTHSTARPLUS_WRED_MAX_AQD_PERIOD) {
                return SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN(
                REG_READ_RED_AQD_CONTROLr(unit, &reg_val));
            fld_val = value / DRV_NORTHSTARPLUS_WRED_AQD_PERIOD_UNIT;
            soc_RED_AQD_CONTROLr_field_set(unit, &reg_val, 
                AQD_PERIODf, &fld_val);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_RED_AQD_CONTROLr(unit, &reg_val));
            break;
        case DRV_WRED_CONTROL_AQD_EXPONENT:
            SOC_IF_ERROR_RETURN(
                REG_READ_RED_EXPONENTr(unit, &reg_val));
            fld_val = value;
            soc_RED_EXPONENTr_field_set(unit, &reg_val, 
                RED_EXPONENTf, &fld_val);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_RED_EXPONENTr(unit, &reg_val));
            break;
        case DRV_WRED_CONTROL_AQD_FAST_CORRECTION:
            SOC_IF_ERROR_RETURN(
                REG_READ_RED_AQD_CONTROLr(unit, &reg_val));
            if (value) {
                fld_val = 1;
            } else {
                fld_val = 0;
            }
            soc_RED_AQD_CONTROLr_field_set(unit, &reg_val, 
                RED_FAST_CORRf, &fld_val);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_RED_AQD_CONTROLr(unit, &reg_val));
            break;
        case DRV_WRED_CONTROL_DROP_BYPASS:
            SOC_IF_ERROR_RETURN(
                REG_READ_RED_EGRESS_BYPASSr(unit, &reg_val));
            soc_RED_EGRESS_BYPASSr_field_get(unit, &reg_val, 
                RED_EGRESS_BYPASSf, &fld_val);
            if (port == -1) {
                temp = SOC_PBMP_WORD_GET(PBMP_ALL(unit), 0);
            } else {
                temp = (0x1 << port);
            }
            if (value) {
                fld_val |= (temp);
            } else {
                fld_val &= ~(temp); 
            }
            soc_RED_EGRESS_BYPASSr_field_set(unit, &reg_val, 
                RED_EGRESS_BYPASSf, &fld_val);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_RED_EGRESS_BYPASSr(unit, &reg_val));
            break;
        default:
            return SOC_E_UNAVAIL;
    }

    return SOC_E_NONE;
}

int
drv_nsp_wred_control_get(int unit, soc_port_t port, int queue, 
            drv_wred_control_t type, uint32 *value)
{
    uint32  reg_val, fld_val = 0;


    /* Check the parameters */
    if ((port != -1) &&
        ((type == DRV_WRED_CONTROL_AQD_PERIOD) ||
        (type == DRV_WRED_CONTROL_AQD_EXPONENT) ||
        (type == DRV_WRED_CONTROL_AQD_FAST_CORRECTION))) {
        /* Global control bit */
        return SOC_E_PARAM;
    }
    
    
    switch (type) {
        case DRV_WRED_CONTROL_ENABLE:
            SOC_IF_ERROR_RETURN(
                REG_READ_RED_CONTROLr(unit, &reg_val));
            soc_RED_CONTROLr_field_get(unit, &reg_val, RED_ENf, &fld_val);
            if (port == -1) {
                *value = (fld_val) ? 1 : 0;
            } else {
                if (fld_val & (0x1 << port)) {
                    *value = TRUE;
                } else {
                    *value = FALSE;
                }
            }
            break;
        case DRV_WRED_CONTROL_AQD_PERIOD:
            SOC_IF_ERROR_RETURN(
                REG_READ_RED_AQD_CONTROLr(unit, &reg_val));
            soc_RED_AQD_CONTROLr_field_get(unit, &reg_val, 
                AQD_PERIODf, &fld_val);
            *value = fld_val * DRV_NORTHSTARPLUS_WRED_AQD_PERIOD_UNIT;
            break;
        case DRV_WRED_CONTROL_AQD_EXPONENT:
            SOC_IF_ERROR_RETURN(
                REG_READ_RED_EXPONENTr(unit, &reg_val));
            soc_RED_EXPONENTr_field_get(unit, &reg_val, 
                RED_EXPONENTf, &fld_val);
            *value = fld_val;
            break;
        case DRV_WRED_CONTROL_AQD_FAST_CORRECTION:
            SOC_IF_ERROR_RETURN(
                REG_READ_RED_AQD_CONTROLr(unit, &reg_val));
            soc_RED_AQD_CONTROLr_field_get(unit, &reg_val, 
                RED_FAST_CORRf, &fld_val);
            if (fld_val) {
                *value = TRUE;
            } else {
                *value = FALSE;
            };
            break;
        case DRV_WRED_CONTROL_DROP_BYPASS:
            SOC_IF_ERROR_RETURN(
                REG_READ_RED_EGRESS_BYPASSr(unit, &reg_val));
            soc_RED_EGRESS_BYPASSr_field_get(unit, &reg_val, 
                RED_EGRESS_BYPASSf, &fld_val);
            if (port == -1) {
                *value = (fld_val) ? 1 : 0;
            } else {
                if (fld_val & (0x1 << port)) {
                    *value = TRUE;
                } else {
                    *value = FALSE;
                }
            }
            break;
        case DRV_WRED_CONTROL_MAX_QUEUE_SIZE:
            /* Get the max cell count */
            SOC_IF_ERROR_RETURN(
                _drv_nsp_queue_max_cell_get(unit, port, queue, &fld_val));
            *value = fld_val * DRV_NORTHSTARPLUS_CELL_UNIT;
            break;
        default:
            return SOC_E_UNAVAIL;
    }

    return SOC_E_NONE;
}


int
drv_nsp_wred_counter_enable_set(int unit, soc_port_t port, int queue,
    drv_wred_counter_t type, int enable) 
{

    /* counter types */
    if ((type != DRV_WRED_COUNTER_DROP_PACKETS) &&
        (type != DRV_WRED_COUNTER_DROP_BYTES)) {
        return SOC_E_UNAVAIL;
    }
    
    /* always enabled */
    if (!enable) {
        return SOC_E_UNAVAIL;
    }

    return SOC_E_NONE;
}

int
drv_nsp_wred_counter_enable_get(int unit, soc_port_t port, int queue,
    drv_wred_counter_t type, int *enable) {

    if ((type != DRV_WRED_COUNTER_DROP_PACKETS) &&
        (type != DRV_WRED_COUNTER_DROP_BYTES)) {
        return SOC_E_UNAVAIL;
    }
    
    /* always enabled */
    *enable = TRUE;

    return SOC_E_NONE;
}


int
drv_nsp_wred_counter_set(int unit, soc_port_t port, int queue,
        drv_wred_counter_t type, uint64 value)
{
    uint32 reg_val, fld_val = 0;
    
    if (!COMPILER_64_IS_ZERO(value)) {
         return SOC_E_UNAVAIL;
    }

    if ((type != DRV_WRED_COUNTER_DROP_PACKETS) &&
        (type != DRV_WRED_COUNTER_DROP_BYTES)) {
        return SOC_E_UNAVAIL;
    }
    
    if (queue != -1) {
        /* For all queues */
        return SOC_E_PARAM;
    }

    /* Reset the counter */
    SOC_IF_ERROR_RETURN(
        REG_READ_RED_DROP_CNTR_RSTr(unit, &reg_val));
    soc_RED_DROP_CNTR_RSTr_field_get(unit, &reg_val, 
        RED_DROP_CNTR_RSTf, &fld_val);
    fld_val |= (0x1 << port);
    soc_RED_DROP_CNTR_RSTr_field_set(unit, &reg_val, 
        RED_DROP_CNTR_RSTf, &fld_val);
    SOC_IF_ERROR_RETURN(
        REG_WRITE_RED_DROP_CNTR_RSTr(unit, &reg_val));

    return SOC_E_NONE;
}

int
drv_nsp_wred_counter_get(int unit, soc_port_t port, int queue,
        drv_wred_counter_t type, uint64 *value)
{
    uint32 reg_val;

    if (queue != -1) {
        /* For all queues */
        return SOC_E_PARAM;
    }

    switch (type) {
        case DRV_WRED_COUNTER_DROP_PACKETS:
            SOC_IF_ERROR_RETURN(
                REG_READ_PN_PORT_RED_PKT_DROP_CNTRr(unit, port, &reg_val));
            COMPILER_64_SET(*value, 0, reg_val);
            break;
        case DRV_WRED_COUNTER_DROP_BYTES:
            SOC_IF_ERROR_RETURN(
                REG_READ_PN_PORT_RED_BYTE_DROP_CNTRr(unit, port, &reg_val));
            COMPILER_64_SET(*value, 0, reg_val);
            break;
        default:
            return SOC_E_UNAVAIL;
    }

    return SOC_E_NONE;
}


