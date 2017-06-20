/*
 * $Id: ramon_fe1600_multicast.c,v 1.10 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC RAMON_FE1600 MULTICAST
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_MCAST
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/mem.h>

#include <soc/defs.h>
#include <soc/error.h>
#include <soc/mcm/allenum.h>
#include <soc/mcm/memregs.h>

#include <soc/dnxf/cmn/dnxf_drv.h>
#include <shared/bitop.h>

#if defined(BCM_88790_A0)

#include <soc/dnxf/fe1600/fe1600_defs.h>
#include <soc/dnxf/fe1600/fe1600_fabric_topology.h>
#include <soc/dnxf/fe1600/fe1600_multicast.h>
#include <soc/dnxf/fe1600/fe1600_fabric_multicast.h>

/*
 * Function:
 *      soc_ramon_fe1600_multicast_egress_add
 * Purpose:
 *      Add port to a multicast group
 * Parameters:
 *      unit     - (IN) Unit number.
 *      group    - (IN) Multicast group ID
 *      port     - (IN) Physical GPORT ID 
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_multicast_egress_add(int unit, bcm_multicast_t group, soc_gport_t port)
{
    uint32 data[SOC_RAMON_FE1600_MULTICAST_ROW_SIZE_IN_UINT32];
    DNXC_INIT_FUNC_DEFS;
    
    /*MULTI_CAST_TABLE_UPDATE is Write Only, so we're reading information from instance 0*/
    DNXC_IF_ERR_EXIT(READ_RTP_MCTm(unit, 0, MEM_BLOCK_ANY, group, data));
      
    SHR_BITSET(data, port);
    
    /*MULTI_CAST_TABLE_UPDATE duplicates information to all MC tables*/
    DNXC_IF_ERR_EXIT(WRITE_RTP_MULTI_CAST_TABLE_UPDATEm(unit, MEM_BLOCK_ALL, group, data));
    
exit:
    DNXC_FUNC_RETURN; 
}

/*
 * Function:
 *      soc_ramon_fe1600_multicast_egress_delete
 * Purpose:
 *      Delete port from a multicast group
 * Parameters:
 *      unit     - (IN) Unit number.
 *      group    - (IN) Multicast group ID
 *      port     - (IN) Physical GPORT ID 
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_multicast_egress_delete(int unit, bcm_multicast_t group, soc_gport_t port)
{
    uint32 data[SOC_RAMON_FE1600_MULTICAST_ROW_SIZE_IN_UINT32];
    DNXC_INIT_FUNC_DEFS;
    
    DNXC_IF_ERR_EXIT(READ_RTP_MCTm(unit, 0, MEM_BLOCK_ANY, group, data));
      
    SHR_BITCLR(data, port);
    
    DNXC_IF_ERR_EXIT(WRITE_RTP_MULTI_CAST_TABLE_UPDATEm(unit, MEM_BLOCK_ALL, group, data));
    
exit:
    DNXC_FUNC_RETURN; 
}

/*
 * Function:
 *      soc_ramon_fe1600_multicast_egress_delete_all
 * Purpose:
 *      Clear multicast group from table
 * Parameters:
 *      unit  - (IN) Unit number.
 *      group - (IN) Multicast group ID 
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_ramon_fe1600_multicast_egress_delete_all(int unit, bcm_multicast_t group)
{
    uint32 data[SOC_RAMON_FE1600_MULTICAST_ROW_SIZE_IN_UINT32];
    DNXC_INIT_FUNC_DEFS;
    
    sal_memset(data, 0, sizeof(uint32)*SOC_RAMON_FE1600_MULTICAST_ROW_SIZE_IN_UINT32);
      
    DNXC_IF_ERR_EXIT(WRITE_RTP_MULTI_CAST_TABLE_UPDATEm(unit, MEM_BLOCK_ALL, group, data));
     
exit:
    DNXC_FUNC_RETURN; 
}

/*
 * Function:
 *      soc_ramon_fe1600_multicast_egress_get
 * Purpose:
 *      Get the set of ports which comprises a multicast group's replication list
 * Parameters:
 *      unit           - (IN)  Unit number.
 *      group          - (IN)  Multicast group ID 
 *      port_max       - (IN)  Number of items allocated in the array 
 *      port_array     - (OUT) Array of physical ports 
 *      port_count     - (OUT) Actual number of ports returned in the array 
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_multicast_egress_get(int unit, bcm_multicast_t group, int port_max, soc_gport_t *port_array, int *port_count)
{
    uint32 data[SOC_RAMON_FE1600_MULTICAST_ROW_SIZE_IN_UINT32];
    soc_gport_t i;
    int table, index, is_first_half;
    uint32 mc_table_entry_size;
    int bit, rv;
    DNXC_INIT_FUNC_DEFS;
    
    DNXC_IF_ERR_EXIT(soc_ramon_fe1600_fabric_multicast_read_info(unit, group, &table, &index, &is_first_half));
    DNXC_IF_ERR_EXIT(READ_RTP_MCTm(unit, table, MEM_BLOCK_ANY, index, data));
    
    *port_count = 0;

    rv = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_multicast_table_entry_size_get, (unit, &mc_table_entry_size));
    DNXC_IF_ERR_EXIT(rv);

    for(i = 0; i < mc_table_entry_size; i++)
    {
        if (is_first_half)
        {
            bit = i;
        }
        else 
        {
            bit = i + mc_table_entry_size;
        }
        if(SHR_BITGET(data, bit))
        {
            if(*port_count >= port_max)
            {
                *port_count = 0;
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("port_array is too small")));
            }
            
            port_array[*port_count] = i;
            (*port_count)++;
        }
    }
     
exit:
    DNXC_FUNC_RETURN; 
}

/*
 * Function:
 *      soc_ramon_fe1600_multicast_egress_set
 * Purpose:
 *      Set the set of ports which comprises a multicast group's replication list
 * Parameters:
 *      unit           - (IN)  Unit number.
 *      group          - (IN)  Multicast group ID 
 *      port_count     - (IN) Number of ports in the array
 *      port_array     - (IN) Array of physical ports 
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_multicast_egress_set(int unit, bcm_multicast_t group, int port_count, soc_gport_t *port_array)
{
    uint32 data[SOC_RAMON_FE1600_MULTICAST_ROW_SIZE_IN_UINT32];
    int i;
    DNXC_INIT_FUNC_DEFS;
    
    sal_memset(data, 0, sizeof(uint32)*SOC_RAMON_FE1600_MULTICAST_ROW_SIZE_IN_UINT32);
  
    for(i = 0 ; i<port_count ; i++)
    {
        SHR_BITSET(data,port_array[i]);
    }
    
    DNXC_IF_ERR_EXIT(WRITE_RTP_MULTI_CAST_TABLE_UPDATEm(unit, MEM_BLOCK_ANY, group, data));
     
exit:
    DNXC_FUNC_RETURN; 
}

/*
 * Function:
 *      soc_ramon_fe1600_multicast_table_size_get
 * Purpose:
 *      Get MC table size
 * Parameters:
 *      unit           - (IN)  Unit number.
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_ramon_fe1600_multicast_table_size_get(int unit, uint32* mc_table_size)
{
	soc_dnxf_multicast_table_mode_t multicast_mode;
    DNXC_INIT_FUNC_DEFS;
	
#ifdef BCM_88754_A0
    if (SOC_IS_BCM88754_A0(unit))
    {
        *mc_table_size = 32*1024;
    }
#endif /*BCM_88754_A0*/
	DNXC_IF_ERR_EXIT(soc_ramon_fe1600_multicast_mode_get(unit, &multicast_mode));
	switch (multicast_mode)
	{
		case soc_dnxf_multicast_table_mode_64k:
			*mc_table_size = 64*1024;
			break;
		default:
			DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("wrong mc_table_mode value %d"),SOC_DNXF_CONFIG(unit).fe_mc_id_range));
	}
exit:
    DNXC_FUNC_RETURN;    
}

soc_error_t
soc_ramon_fe1600_multicast_mode_get(int unit, soc_dnxf_multicast_table_mode_t* multicast_mode)
{
	DNXC_INIT_FUNC_DEFS;
	*multicast_mode = soc_dnxf_multicast_table_mode_64k; /* default */
	DNXC_FUNC_RETURN;
}

soc_error_t
soc_ramon_fe1600_multicast_table_entry_size_get(int unit, uint32 *entry_size)
{
	DNXC_INIT_FUNC_DEFS;
	*entry_size = 128;
	DNXC_FUNC_RETURN;
}

#endif /*defined(BCM_88790_A0)*/

#undef _ERR_MSG_MODULE_NAME

