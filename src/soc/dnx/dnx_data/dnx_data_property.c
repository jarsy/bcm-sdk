/** \file dnx_data_property.c
 * 
 * MODULE DATA CUSTOME PROPRTY - 
 * Includes all custom functions implementations
 * 
 * Device Data
 * SW component that maintains per device data
 * The data is static and won't be changed after device initialization.
 *     
 * Supported data types:
 *     - Define             - a 'uint32' number (a max value for all devices is maintained)
 *     - feature            - 1 bit per each feature (supported/not supported) - support soc properties 
 *     - table              - the data is accessed with keys and/or can maintain multiple values and/or set by soc property
 *     - numeric            - a 'uint32' number that support soc properties
 * 
 * User interface for DNX DATA component can be found in "dnx_data_if.h" and "dnx_data_if_#module#.h"
 * 
 * Adding the data is done via XMLs placed in "tools/autocoder/DeviceData/dnx/.." 
 * "How to" User Guide can be found in confluence. 
 */

/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SOCDNX_DATA

/*
 * INCLUDE FILES:
 * {
 */
#include <soc/dnx/dnx_data/dnx_data.h>
#include <soc/dnx/dnx_data/dnx_data_internal.h>
#include <sal/appl/sal.h>

/*
 * }
 */

/*
 * MODULE: FABRIC:
 * {
 */

/*
 * submodule pipes
 */
/*
 * See .h file
 */
shr_error_e
dnx_data_property_fabric_pipes_map_read(
    int unit,
    dnx_data_fabric_pipes_map_t map)
{
     /*TBD*/ return _SHR_E_NONE;
}

/*
 * }
 */

/*
 * MODULE: MODULE_TESTING
 * {
 */

/*
 * submodule property_methods
 */

/*
 * TBD: it's example only - should be rewritten
 */
/*
 * See .h file
 */
shr_error_e
dnx_data_property_module_testing_property_methods_custom_read(
    int unit,
    int link,
    int pipe,
    dnx_data_module_testing_property_methods_custom_t * custom)
{
    SHR_FUNC_INIT_VARS(unit);

    if (pipe == 1)
    {
        custom->val = soc_property_suffix_num_get(unit, link, "dnx_data_custom", "link", custom->val);
    }

    SHR_FUNC_EXIT;
}

/*
 * }
 */

/*
 * MODULE: NIF 
 * { 
 */
/*
 * submodule ports
 */

/*
 * See .h file
 */
shr_error_e
dnx_data_property_port_general_fabric_port_base_read(
    int unit,
    uint32 * fabric_port_base)
{
    bcm_port_t fabric_port;
    SHR_FUNC_INIT_VARS(unit);

    /** Read SoC property */
    *fabric_port_base = soc_property_get(unit, spn_FABRIC_LOGICAL_PORT_BASE, *fabric_port_base);
    /** Verify SoC property */
    if (*fabric_port_base > SOC_MAX_NUM_PORTS - dnx_data_fabric.links.nof_links_get(unit))
    {
        SHR_ERR_EXIT(_SHR_E_CONFIG, "fabric logical port base value not supported - [0 - %d] \n",
                     SOC_MAX_NUM_PORTS - dnx_data_fabric.links.nof_links_get(unit));
    }

    /*
     * Special handling! 
     * Set port name - 
     * Required in this stage to be able to use soc_porperty_port_get
     */
    for (fabric_port = *fabric_port_base; fabric_port < *fabric_port_base + dnx_data_fabric.links.nof_links_get(unit);
         fabric_port++)
    {
        sal_snprintf(SOC_INFO(unit).port_name[fabric_port], sizeof(SOC_INFO(unit).port_name[fabric_port]), "sfi%d",
                     fabric_port);
        sal_snprintf(SOC_INFO(unit).port_name_alter[fabric_port], sizeof(SOC_INFO(unit).port_name[fabric_port]),
                     "fabric%d", fabric_port);
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief 
 * required info per interface type in order parse ucode_port SoC property
 */
typedef struct
{
    /**
     * interface type name as expected in ucode_port val
     */
    char *interface_type_name;
    /**
     * Matching Interface type define 
     */
    bcm_port_if_t interface;
    /**
     * port name - used to read soc properties using soc_property_port_get
     */
    char *port_name;
    /**
     * flags - addiotional info - see ucode_port_info_flags 
     * see DNX_DATA_PROPERTY_UCODE_FLAG_* 
     */
    uint32 flags;
} dnx_data_property_ucode_port_info_t;

/**
 * \brief - flags for ucode_ports_info 
 */
#define DNX_DATA_PROPERTY_UCODE_FLAG_NOT_REQUIRED_INDEX (0x1)

/* *INDENT-OFF* */
/**
 * \brief 
 * holds info per interface type required to parse ucode_port SoC property
 */
static dnx_data_property_ucode_port_info_t ucode_ports_info[] = {
    /*if type name  if typedef              port name   flags*/
    {"XE",          BCM_PORT_IF_XFI,        "xe",       0},
    {"10GBase-R",   BCM_PORT_IF_XFI,        "xe",       0}, /*same as XE, legacy name*/
    {"XLGE2_",      BCM_PORT_IF_XLAUI2,     "xl",       0},
    {"XLGE",        BCM_PORT_IF_XLAUI,      "xl",       0},
    {"CGE",         BCM_PORT_IF_CAUI,       "ce",       0},
    {"ILKN",        BCM_PORT_IF_ILKN,       "il",       0},
    {"GE",          BCM_PORT_IF_SGMII,      "ge",       0},
    {"SGMII",       BCM_PORT_IF_SGMII,      "ge",       0}, /*same as GE, legacy name*/
    {"XAUI",        BCM_PORT_IF_XAUI,       "xe",       0},
    {"RXAUI",       BCM_PORT_IF_RXAUI,      "xe",       0},
    {"QSGMII",      BCM_PORT_IF_QSGMII,     "qsgmii",   0},
    {"CPU",         BCM_PORT_IF_CPU,        "cpu",      0},
    {"RCY",         BCM_PORT_IF_RCY,        "rcy",      0},
    {NULL,          BCM_PORT_IF_NULL,       NULL,       0}  /*last*/
};
/* *INDENT-ON* */

/*
 * See .h file
 */
shr_error_e
dnx_data_property_port_general_ucode_port_read(
    int unit,
    int port,
    dnx_data_port_general_ucode_port_t * ucode_port)
{
    char *propval;
    char *str_pointer, *str_pointer_end;
    char *interface_type_str;
    int interface_type_str_length;
    int idx;
    SHR_FUNC_INIT_VARS(unit);

    /*
     * Read SoC property
     */
    propval = soc_property_port_get_str(unit, port, spn_UCODE_PORT);

    /*
     * If set - parse SoC property 
     * Expected format: 
     * "#interface_type##interface_id#[.#channel_num#]:core#core-id#.#tm-port#[:#stat#]" 
     */
    if (propval)
    {
        str_pointer = propval;
        idx = 0;

        /*
         * Find interface type and interface offset 
         * Iterate over ucode_port_info enteries 
         */
        while (ucode_ports_info[idx].interface_type_name)
        {
            interface_type_str = ucode_ports_info[idx].interface_type_name;
            interface_type_str_length = sal_strlen(interface_type_str);

            /*
             * compare inteface type to current ucode_port_info entry
             */
            if (!sal_strncasecmp(str_pointer, interface_type_str, interface_type_str_length))
            {
                str_pointer += interface_type_str_length;
                ucode_port->interface = ucode_ports_info[idx].interface;

                /*
                 * If interface type requires interface offset - parse it
                 */
                if ((ucode_ports_info[idx].flags & DNX_DATA_PROPERTY_UCODE_FLAG_NOT_REQUIRED_INDEX) == 0)
                {
                    /*
                     * Parse interface offset
                     */
                    ucode_port->interface_offset = sal_ctoi(str_pointer, &str_pointer_end);
                    /** Make sure that interface offset found */
                    if (str_pointer == str_pointer_end)
                    {
                        SHR_ERR_EXIT(_SHR_E_FAIL, "No interface index in (\"%s\") for %s\n", propval, spn_UCODE_PORT);
                    }
                    str_pointer = str_pointer_end;
                }

                break;
            }

            /** increment to next ucode_port_info entry */
            idx++;
        }

        /*
         * Make sure that interace type found
         */
        if (ucode_ports_info[idx].interface_type_name == NULL /* not found */ )
        {
            SHR_ERR_EXIT(_SHR_E_FAIL, "Unexpected property value (\"%s\") for %s", propval, spn_UCODE_PORT);
        }

        /*
         * TBD: extract addtional ucode_port info
         */

        /*
         * Special handling 
         * Set port name - 
         * Required in this stage to be able to use soc_porperty_port_get
         */
        sal_snprintf(SOC_INFO(unit).port_name[port], sizeof(SOC_INFO(unit).port_name[port]), "%s%d",
                     ucode_ports_info[idx].port_name, port);

    }

exit:
    SHR_FUNC_EXIT;
}

/*
 * }
 */
