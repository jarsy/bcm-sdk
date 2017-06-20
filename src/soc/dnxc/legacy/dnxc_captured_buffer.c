/*
 * $Id: dnxc_captured_buffer.c,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC DNXC CAPTURED BUFFER
 */
 
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FABRIC
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>

#include <soc/defs.h>
#include <soc/dnxc/legacy/dnxc_error.h>
#include <soc/dnxc/legacy/dnxc_captured_buffer.h>


soc_error_t 
dnxc_captured_buffer_create(int unit, cyclic_buffer_t* buffer, int max_buffered_cells)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(buffer);

    rc = cyclic_buffer_create(unit, buffer, sizeof(dnxc_captured_cell_t), max_buffered_cells, "captured_buffer"); 
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXC_FUNC_RETURN; 
}

soc_error_t 
dnxc_captured_buffer_destroy(int unit, cyclic_buffer_t* buffer)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(buffer);

    rc = cyclic_buffer_destroy(unit, buffer);
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXC_FUNC_RETURN; 
}

soc_error_t 
dnxc_captured_buffer_add(int unit, cyclic_buffer_t* buffer, const dnxc_captured_cell_t* new_cell)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;
    
    DNXC_NULL_CHECK(buffer);
    DNXC_NULL_CHECK(new_cell);

    rc = cyclic_buffer_add(unit, buffer, (const void*)new_cell);
    DNXC_IF_ERR_EXIT(rc);
    
exit:
    DNXC_FUNC_RETURN; 
}

soc_error_t 
dnxc_captured_buffer_get(int unit, cyclic_buffer_t* buffer, dnxc_captured_cell_t* received_cell)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(buffer);
    DNXC_NULL_CHECK(received_cell);

    rc = cyclic_buffer_get(unit, buffer, (void*)received_cell);
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXC_FUNC_RETURN; 
}

soc_error_t 
dnxc_captured_buffer_is_empty(int unit, const cyclic_buffer_t* buffer, int* is_empty)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(buffer);
    DNXC_NULL_CHECK(is_empty);

    rc = cyclic_buffer_is_empty(unit, buffer, is_empty);
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXC_FUNC_RETURN; 
}

soc_error_t 
dnxc_captured_buffer_is_full(int unit, const cyclic_buffer_t* buffer, int* is_full)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(buffer);
    DNXC_NULL_CHECK(is_full);

    rc = cyclic_buffer_is_full(unit, buffer, is_full);
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXC_FUNC_RETURN; 
}

soc_error_t 
dnxc_captured_buffer_cells_count(int unit, const cyclic_buffer_t* buffer, int* count)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(buffer);
    DNXC_NULL_CHECK(count);

    rc = cyclic_buffer_cells_count(unit, buffer, count);
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXC_FUNC_RETURN; 
}


#undef _ERR_MSG_MODULE_NAME

