/*
 * $Id: tdm_main.c.$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * All Rights Reserved.$
 *
 * TDM core main functions
 */
#ifdef _TDM_STANDALONE
	#include <tdm_top.h>
#else
	#include <soc/tdm/core/tdm_top.h>
#endif


/**
@name: tdm_core_init
@param:

**/
int
tdm_core_init( tdm_mod_t *_tdm )
{
	int index;
	
	for (index=0; index<(_tdm->_core_data.vmap_max_wid); index++) {
		TDM_MSET(_tdm->_core_data.vmap[index],_tdm->_chip_data.soc_pkg.num_ext_ports,_tdm->_core_data.vmap_max_len);
	}
	
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.y8=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.y7=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.y1=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.y2=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.y3=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.y4=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.y5=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.y6=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.yy=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.z8=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.z7=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.z6=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.z1=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.z2=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.z3=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.z4=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.z5=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.z11=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.z22=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.z33=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.z44=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.z55=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.z66=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.z77=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.z88=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.z99=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.zaa=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.zbb=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.zcc=0;
	_tdm->_core_data.vars_pkg.m_tdm_core_vbs_scheduler.lr_vec_cnt=0;
	
	_tdm->_core_data.vars_pkg.lr_1=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.lr_10=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.lr_20=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.lr_25=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.lr_40=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.lr_50=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.lr_100=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.lr_120=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.lr_enable=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.os_1=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.os_10=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.os_20=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.os_25=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.os_40=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.os_50=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.os_100=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.os_120=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.os_enable=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.HG4X106G_3X40G=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.HG1X106G_xX40G_OVERSUB=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.HGXx120G_Xx100G=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.m_tdm_vmap_alloc.num_lr=0;
	_tdm->_core_data.vars_pkg.m_tdm_vmap_alloc.num_os=0;
	_tdm->_core_data.vars_pkg.m_tdm_vmap_alloc.num_40g=0;
	_tdm->_core_data.vars_pkg.m_tdm_vmap_alloc.num_100g=0;
	_tdm->_core_data.vars_pkg.m_tdm_vmap_alloc.tri_chk=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.m_tdm_vmap_alloc.tri_en_50=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.m_tdm_vmap_alloc.tri_en_40=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.m_tdm_vmap_alloc.tri_en_20=BOOL_FALSE;
	_tdm->_core_data.vars_pkg.m_tdm_vmap_alloc.prox_swap=_tdm->_chip_data.soc_pkg.num_ext_ports;
	
	TDM_MSET(_tdm->_core_data.vars_pkg.m_tdm_vmap_alloc.os1,_tdm->_chip_data.soc_pkg.num_ext_ports,TDM_AUX_SIZE);
	TDM_MSET(_tdm->_core_data.vars_pkg.m_tdm_vmap_alloc.os10,_tdm->_chip_data.soc_pkg.num_ext_ports,TDM_AUX_SIZE);
	TDM_MSET(_tdm->_core_data.vars_pkg.m_tdm_vmap_alloc.os20,_tdm->_chip_data.soc_pkg.num_ext_ports,TDM_AUX_SIZE);
	TDM_MSET(_tdm->_core_data.vars_pkg.m_tdm_vmap_alloc.os25,_tdm->_chip_data.soc_pkg.num_ext_ports,TDM_AUX_SIZE);
	TDM_MSET(_tdm->_core_data.vars_pkg.m_tdm_vmap_alloc.os40,_tdm->_chip_data.soc_pkg.num_ext_ports,TDM_AUX_SIZE);
	TDM_MSET(_tdm->_core_data.vars_pkg.m_tdm_vmap_alloc.os50,_tdm->_chip_data.soc_pkg.num_ext_ports,TDM_AUX_SIZE);
	TDM_MSET(_tdm->_core_data.vars_pkg.m_tdm_vmap_alloc.os100,_tdm->_chip_data.soc_pkg.num_ext_ports,TDM_AUX_SIZE);
	TDM_MSET(_tdm->_core_data.vars_pkg.m_tdm_vmap_alloc.os120,_tdm->_chip_data.soc_pkg.num_ext_ports,TDM_AUX_SIZE);
	
	_tdm->_core_data.vars_pkg.m_tdm_pick_vec.triport_priority=BOOL_FALSE;
	
	return ( _tdm->_core_exec[TDM_CORE_EXEC__ALLOC]( _tdm ) );
	/* All TDM class data until return to SDK should now have heap allocation and be initialized to macro values */
}


/**
@name: tdm_core_post
@param:

**/
int
tdm_core_post( tdm_mod_t *_tdm )
{
	return ( _tdm->_chip_exec[TDM_CHIP_EXEC__POST]( _tdm ) );
}


/**
@name: tdm_core_null
@param:

**/
int
tdm_core_null( tdm_mod_t *_tdm )
{
	TDM_PRINT4("TDM: Null function was dereferenced | Pipe ID: %0d | Cal ID %0d | Grp ID %0d | Port #%0d\n",_tdm->_core_data.vars_pkg.pipe,_tdm->_core_data.vars_pkg.cal_id,_tdm->_core_data.vars_pkg.grp_id,_tdm->_core_data.vars_pkg.port);

	return PASS;
}
