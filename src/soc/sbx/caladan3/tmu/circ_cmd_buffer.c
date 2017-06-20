/*
 * $Id: circ_cmd_buffer.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * Purpose: Thread safe circular buffer 
 */

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <shared/util.h>
#include <sal/appl/sal.h>
#include <soc/sbx/caladan3/tmu/circ_cmd_buffer.h>

int circ_cmd_buffer_init(int unit, circ_cmd_buffer_t *cbuf)
{
    if (cbuf == NULL) {
        return SOC_E_PARAM;
    }

    cbuf->read_pos = cbuf->write_pos = 0;
    cbuf->length = 0;
    cbuf->buffer = NULL;

    cbuf->mutex = sal_mutex_create("CIRC_BUF_MUTEX");
    if (cbuf->mutex == NULL) {
        LOG_CLI((BSL_META_U(unit,
                            "failed to create mutex\n")));
        return SOC_E_RESOURCE;
    }

    return SOC_E_NONE;
}

int circ_cmd_buffer_initFromBuf(int unit, circ_cmd_buffer_t *cbuf,
                                soc_sbx_caladan3_tmu_cmd_t **buffer, int length)
{
    int status;

    if (cbuf == NULL || buffer == NULL || length <= 0) {
        return SOC_E_PARAM;
    }

    status = circ_cmd_buffer_init(unit, cbuf);

    if (status == SOC_E_NONE) {
        cbuf->buffer = buffer;
        cbuf->length = length;
    }

    return status;
}

int circ_cmd_buffer_destroy(int unit, circ_cmd_buffer_t *cbuf)
{
    if (cbuf == NULL) {
        return SOC_E_PARAM;
    }

    if(cbuf->mutex) {
        sal_mutex_destroy(cbuf->mutex);
    }
    return SOC_E_NONE;
}

int circ_cmd_buffer_full(int unit, circ_cmd_buffer_t *cbuf)
{
    uint8 full=FALSE;
    sal_mutex_take(cbuf->mutex, sal_mutex_FOREVER);
    full = (((cbuf->write_pos+1)%cbuf->length) == cbuf->read_pos)? TRUE: FALSE; 
    sal_mutex_give(cbuf->mutex);
    return full;
}

int circ_cmd_buffer_empty(int unit, circ_cmd_buffer_t *cbuf)
{
    uint8 empty=FALSE;
    sal_mutex_take(cbuf->mutex, sal_mutex_FOREVER);
    empty = (cbuf->read_pos == cbuf->write_pos)? TRUE: FALSE; 
    sal_mutex_give(cbuf->mutex);
    return empty;
}

int circ_cmd_buffer_put(int unit, circ_cmd_buffer_t *cbuf, soc_sbx_caladan3_tmu_cmd_t *element, uint8 overflow)
{
    int status = SOC_E_NONE;

    if (overflow || !circ_cmd_buffer_full(unit, cbuf)) {
        sal_mutex_take(cbuf->mutex, sal_mutex_FOREVER);
        cbuf->buffer[cbuf->write_pos] = element;
        cbuf->write_pos++;
        cbuf->write_pos %= cbuf->length;
        sal_mutex_give(cbuf->mutex);
    } else {
        status = SOC_E_FULL;
    }

    return status;
}

int circ_cmd_buffer_get(int unit, circ_cmd_buffer_t *cbuf, soc_sbx_caladan3_tmu_cmd_t **element)
{
    int status = SOC_E_NONE;

    if (!circ_cmd_buffer_empty(unit, cbuf)) {
        sal_mutex_take(cbuf->mutex, sal_mutex_FOREVER);
        *element = cbuf->buffer[cbuf->read_pos];
        cbuf->read_pos++;
        cbuf->read_pos %= cbuf->length;
        sal_mutex_give(cbuf->mutex);
    } else {
        status = SOC_E_EMPTY;
    }

    return status;
}

void circ_cmd_buffer_printf(int unit, circ_cmd_buffer_t *cbuf)
{
    int index;
    sal_mutex_take(cbuf->mutex, sal_mutex_FOREVER);
    LOG_CLI((BSL_META_U(unit,
                        "-- Circ Buffer Fill[%d] Empty[%d] Capacity[%d] --\n"),
             circ_cmd_buffer_full(unit,cbuf),
             circ_cmd_buffer_empty(unit,cbuf),
             cbuf->length-1));

    if (circ_cmd_buffer_empty(unit,cbuf)) return;

    for (index=0; index < cbuf->length; index++) {
        if (cbuf->buffer[index] != NULL) {
            LOG_CLI((BSL_META_U(unit,
                                "\n Index= %d: %p"), index, (void *)cbuf->buffer[index]));
            /*tmu_cmd_printf(unit, cbuf->buffer[index]);*/
        }
    }

    LOG_CLI((BSL_META_U(unit,
                        "\n")));
    sal_mutex_give(cbuf->mutex);
}

#endif
