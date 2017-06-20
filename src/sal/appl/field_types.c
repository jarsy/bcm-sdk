/*! \file field_types.c
 * Purpose: Scan and print different field types
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <sal/types.h>
#include <sal/appl/field_types.h>

char* sal_field_type_names[] = SAL_FIELD_TYPE_STRINGS;

sal_field_type_e
sal_field_scan_str(char *source, sal_field_type_e in_field_type, void *value, int value_size)
{
    sal_field_type_e out_field_type = SAL_FIELD_TYPE_NONE;

    return out_field_type;
}

void
sal_field_format_str(char *target, int target_size, sal_field_type_e out_field_type, void *value, int value_size)
{

}

/*
 * Return field type name
 */
char*
sal_field_type_str(sal_field_type_e field_type)
{
    if(field_type >= SAL_FIELD_TYPE_MAX)
        field_type = SAL_FIELD_TYPE_NONE;

    return sal_field_type_names[field_type];
}
