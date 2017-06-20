/*
 * $Id: soc_cint_data.c,v 1.70 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * soc_cint_data.c
 *
 * Hand-coded support for a few SAL core routines.
 */
int soc_core_cint_data_not_empty;
#include <sdk_config.h>

#if defined(INCLUDE_LIB_CINT)

#include <cint_config.h>
#include <cint_types.h>
#include <cint_porting.h>
#include <soc/property.h>
#include <soc/drv.h>
#include <soc/intr.h>
#include <soc/i2c.h>
#include <shared/util.h>
#include <shared/bitop.h>
#if defined(BCM_PETRA_SUPPORT) && defined(INCLUDE_KBP) && !defined(BCM_88030)
#include <soc/dpp/ARAD/arad_kbp.h>
#include <soc/dpp/ARAD/arad_kbp_rop.h>
#endif

#if defined(BCM_PETRA_SUPPORT)
#include <soc/dpp/ARAD/arad_dram.h>
#include <soc/dpp/DRC/drc_combo28_init.h>
#include <soc/dpp/DRC/drc_combo28_cb.h>
#include <soc/dpp/DRC/drc_combo28.h>
#endif

#if defined(BCM_CALADAN3_SUPPORT)
#include <soc/sbx/caladan3/rce.h>
#include <soc/sbx/caladan3/cop.h>
#include <soc/sbx/caladan3/lrp.h>
#include <soc/sbx/caladan3/sws.h>
#include <soc/sbx/caladan3/simintf.h>
extern unsigned long int strtoul(const char *nptr, char **endptr, int base);


static unsigned long int __strtoul(const char *nptr, char **endptr, int base)
{
   return sal_strtoul(nptr, endptr, base);
}


int soc_c3_is_pvv2e_taps_enabled()
{
#ifdef PVV_TAPS_ENABLE
    return TRUE;
#else
    return FALSE;
#endif

}


static int soc_c3_is_sim()
{
  if (SAL_BOOT_BCMSIM) {
    return 1;
  }
  else {
    return 0;
  }
}

static cint_parameter_desc_t __cint_parameters__soc_c3_rce_program_traverse_cb_t[] =
{
    {
        "int"
        "r",
        0,
        0
    },
    {
        "int",
        "unit",
        0,
        0
    },
    {
        "int",
        "programId",
        0,
        0
    },
    {
        "void",
        "extras",
        1,
        0
    },
    CINT_ENTRY_LAST
};

static cint_parameter_desc_t __cint_parameters__soc_c3_rce_group_traverse_cb_t[] =
{
    {
        "int",
        "r",
        0,
        0
    },
    {
        "int",
        "unit",
        0,
        0
    },
    {
        "int",
        "programId",
        0,
        0
    },
    {
        "int",
        "groupId",
        0,
        0
    },
    {
        "void",
        "extras",
        1,
        0
    },
    CINT_ENTRY_LAST
};

static cint_parameter_desc_t __cint_parameters__soc_c3_rce_entry_traverse_cb_t[] =
{
    {
        "int",
        "r",
        0,
        0
    },
    {
        "int",
        "unit",
        0,
        0
    },
    {
        "int",
        "programId",
        0,
        0
    },
    {
        "int",
        "groupId",
        0,
        0
    },
    {
        "int",
        "entryId",
        0,
        0
    },
    {
        "void",
        "extras",
        1,
        0
    },
    CINT_ENTRY_LAST
};

static cint_parameter_desc_t __cint_parameters__soc_c3_rce_entry_qualify_range_traverse_cb_t[] =
{
    {
        "int",
        "r",
        0,
        0
    },
    {
        "int",
        "unit",
        0,
        0
    },
    {
        "int",
        "programId",
        0,
        0
    },
    {
        "int",
        "groupId",
        0,
        0
    },
    {
        "int",
        "entryId",
        0,
        0
    },
    {
        "int",
        "rangeId",
        0,
        0
    },
    {
        "int",
        "qualify",
        0,
        0
    },
    {
        "void",
        "extras",
        1,
        0
    },
    CINT_ENTRY_LAST
};

static cint_parameter_desc_t __cint_parameters__soc_c3_rce_range_traverse_cb_t[] =
{
    {
        "int",
        "r",
        0,
        0
    },
    {
        "int",
        "unit",
        0,
        0
    },
    {
        "int",
        "rangeId",
        0,
        0
    },
    {
        "void",
        "extras",
        1,
        0
    },
    CINT_ENTRY_LAST
};

static int
__cint_fpointer__soc_c3_rce_program_traverse_cb_t(int unit,
                                                  int programId,
                                                  void *extras);
static int
__cint_fpointer__soc_c3_rce_group_traverse_cb_t(int unit,
                                                int programId,
                                                int groupId,
                                                void *extras);
static int
__cint_fpointer__soc_c3_rce_entry_traverse_cb_t(int unit,
                                                int programId,
                                                int groupId,
                                                int entryId,
                                                void *extras);
static int
__cint_fpointer__soc_c3_rce_entry_qualify_range_traverse_cb_t(int unit,
                                                              int programId,
                                                              int groupId,
                                                              int entryId,
                                                              int rangeId,
                                                              int qualify,
                                                              void *extras);
static int
__cint_fpointer__soc_c3_rce_range_traverse_cb_t(int unit,
                                                int rangeId,
                                                void *extras);
#endif /* defined (BCM_CALADAN3_SUPPORT) */

static cint_function_pointer_t __cint_soc_function_pointers[] =
{
#if defined(BCM_CALADAN3_SUPPORT)
    {
        "soc_c3_rce_program_traverse_cb_t",
        (cint_fpointer_t) __cint_fpointer__soc_c3_rce_program_traverse_cb_t,
        __cint_parameters__soc_c3_rce_program_traverse_cb_t
    },
    {
        "soc_c3_rce_group_traverse_cb_t",
        (cint_fpointer_t) __cint_fpointer__soc_c3_rce_group_traverse_cb_t,
        __cint_parameters__soc_c3_rce_group_traverse_cb_t
    },
    {
        "soc_c3_rce_entry_traverse_cb_t",
        (cint_fpointer_t) __cint_fpointer__soc_c3_rce_entry_traverse_cb_t,
        __cint_parameters__soc_c3_rce_entry_traverse_cb_t
    },
    {
        "soc_c3_rce_entry_qualify_range_traverse_cb_t",
        (cint_fpointer_t) __cint_fpointer__soc_c3_rce_entry_qualify_range_traverse_cb_t,
        __cint_parameters__soc_c3_rce_entry_qualify_range_traverse_cb_t
    },
    {
        "soc_c3_rce_range_traverse_cb_t",
        (cint_fpointer_t) __cint_fpointer__soc_c3_rce_range_traverse_cb_t,
        __cint_parameters__soc_c3_rce_range_traverse_cb_t
    },
#endif /* defined (BCM_CALADAN3_SUPPORT) */
    CINT_ENTRY_LAST
};

CINT_FWRAPPER_CREATE_RP1(int, int, 0, 0,
                         soc_init, \
                         int,int,unit,0,0);

CINT_FWRAPPER_CREATE_RP2(char*,char,1,0,
                         soc_property_get_str,
                         int,int,unit,0,0,
                         char*,char,name,1,0);

CINT_FWRAPPER_CREATE_RP3(void*, void, 1, 0,
                         soc_cm_salloc,
                         int,int,unit,0,0,
                         int,int,size,0,0,
                         void*,void,name,1,0);

CINT_FWRAPPER_CREATE_VP2(soc_cm_sfree,
                         int,int,unit,0,0,
                         void*,void,ptr,1,0);

CINT_FWRAPPER_CREATE_RP3(uint32,uint32,0,0,
                         soc_property_get,
                         int,int,unit,0,0,
                         char*,char,name,1,0,
                         uint32,uint32,def,0,0);
CINT_FWRAPPER_CREATE_RP3(pbmp_t,pbmp_t,0,0,
                         soc_property_get_pbmp,
                         int,int,unit,0,0,
                         char*,char,name,1,0,
                         int,int,defneg,0,0);

CINT_FWRAPPER_CREATE_RP3(pbmp_t,pbmp_t,0,0,
                         soc_property_get_pbmp_default,
                         int,int,unit,0,0,
                         char*,char,name,1,0,
                         pbmp_t,pbmp_t,def,0,0);

CINT_FWRAPPER_CREATE_RP3(char*,char,1,0,
                         soc_property_port_get_str,
                         int,int,unit,0,0,
                         int,int,port,0,0,
                         char*,char,name,1,0);

CINT_FWRAPPER_CREATE_RP4(uint32,uint32,0,0,
                         soc_property_port_get,
                         int,int,unit,0,0,
                         int,int,port,0,0,
                         char*,char,name,1,0,
                         uint32,uint32,def,0,0);

CINT_FWRAPPER_CREATE_RP5(uint32,uint32,0,0,
                         soc_property_suffix_num_get,
                         int,int,unit,0,0,
                         int,int,tc,0,0,
                         char*,char,name,1,0,
                         char*,char,suffix,1,0,
                         uint32,uint32,def,0,0);

CINT_FWRAPPER_CREATE_RP4(uint32,uint32,0,0,
                         soc_property_cos_get,
                         int,int,unit,0,0,
                         soc_cos_t,soc_cos_t,cos,0,0,
                         char*,char,name,1,0,
                         uint32,uint32,def,0,0);

#if defined(BCM_ESW_SUPPORT)
#if defined(INCLUDE_I2C)
CINT_FWRAPPER_CREATE_RP4(int,int,0,0,
                         soc_i2c_read_word_data,
                         int,int,unit,0,0,
                         uint8,uint8,saddr,0,0,
                         uint8,uint8,com,0,0,
                         uint16*,uint16,value,1,0);

CINT_FWRAPPER_CREATE_RP4(int,int,0,0,
                         soc_i2c_write_word_data,
                         int,int,unit,0,0,
                         uint8,uint8,saddr,0,0,
                         uint8,uint8,com,0,0,
                         uint16,uint16,value,0,0);

CINT_FWRAPPER_CREATE_RP1(int, int, 0, 0,
                         soc_i2c_probe, \
                         int,int,unit,0,0);
#endif
CINT_FWRAPPER_CREATE_RP1(uint16, uint16, 0, 0,
                         _shr_swap16, \
                         uint16,uint16,val,0,0);
#endif

#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined(BCM_DFE_SUPPORT)|| defined(BCM_PETRA_SUPPORT)
CINT_FWRAPPER_CREATE_RP5(int,int,0,0,
                         soc_active_interrupts_get,
                         int,int,unit,0,0,
                         int,int,flags,0,0,
                         int,int,max_interrupts_size,0,0,
                         soc_interrupt_cause_t*,soc_interrupt_cause_t,interrupts,1,0,
                         int*,int,total_interrupts,1,0);

CINT_FWRAPPER_CREATE_RP2(int,int,0,0,
                         soc_nof_interrupts,
                         int,int,unit,0,0,
                         int*,int,nof_interrupts,1,0);

CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_interrupt_info_get,
                         int,int,unit,0,0,
                         int,int,interrupt_id,0,0,
                         soc_interrupt_db_t*,soc_interrupt_db_t,inter,1,0);

CINT_FWRAPPER_CREATE_RP6(int,int,0,0,
                         soc_get_interrupt_id,
                         int,int,unit,0,0,
                         int,int,reg,0,0,
                         int,int,reg_index,0,0,
                         int,int,field,0,0,
                         int,int,bit_in_field,0,0,
                         int*,int,interrupt_id,1,0);

CINT_FWRAPPER_CREATE_RP5(int,int,0,0,
                         soc_get_interrupt_id_specific,
                         int,int,unit,0,0,
                         int,int,reg_adress,0,0,
                         int,int,reg_block,0,0,
                         int,int,field_bit,0,0,
                         int*,int,interrupt_id,1,0);

CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_interrupt_is_supported,
                         int,int,unit,0,0,
                         int,int,block_instance,0,0,
                         int,int,inter_id,0,0);

#if defined(BCM_PETRA_SUPPORT) && defined(INCLUDE_KBP) && !defined(BCM_88030)

CINT_FWRAPPER_CREATE_RP3(uint32,uint32,0,0,
                         arad_kbp_cpu_lookup_reply,
                         uint32,uint32,core,0,0,
                         int,int,unit,0,0,
                         uint32*,uint32,data,1,0);

CINT_FWRAPPER_CREATE_RP5(uint32,uint32,0,0,
                         arad_kbp_cpu_record_send,
                         int,int,unit,0,0,
                         uint32,uint32,opcode,0,0,
                         uint32*,uint32,msb_data,1,0,
                         uint32*,uint32,lsb_data,1,0,
                         int,int,lsb_enable,0,0);
CINT_FWRAPPER_CREATE_RP7(uint32,uint32,0,0,
                         aradplus_kbp_cpu_record_send,
                         int,int,unit,0,0,
                         uint32,uint32,core,0,0,
                         uint32,uint32,opcode,0,0,
                         uint32*,uint32,msb_data,1,0,
                         uint32*,uint32,lsb_data,1,0,
                         int,int,lsb_enable,0,0,
                         soc_reg_above_64_val_t,soc_reg_above_64_val_t,read_data,0,0);

CINT_FWRAPPER_CREATE_RP5(uint32,uint32,0,0,
                         arad_kbp_lut_write,
                         int,int,unit,0,0,
                         uint32,uint32,core,0,0,
                         uint8,uint8,addr,0,0,
                         arad_kbp_lut_data_t*,arad_kbp_lut_data_t,lut_data,1,0,
                         uint32*,uint32,lut_data_row,1,0);

CINT_FWRAPPER_CREATE_RP5(uint32,uint32,0,0,
                         arad_kbp_lut_read,
                         int,int,unit,0,0,
                         uint32,uint32,core,0,0,
                         uint8,uint8,addr,0,0,
                         arad_kbp_lut_data_t*,arad_kbp_lut_data_t,lut_data,1,0,
                         uint32*,uint32,lut_data_row,1,0);

CINT_FWRAPPER_CREATE_RP3(uint32,uint32,0,0,
                         arad_kbp_rop_write,
                         int,int,unit,0,0,
                         uint32,uint32,core,0,0,
                         arad_kbp_rop_write_t*,arad_kbp_rop_write_t,wr_data,1,0);

CINT_FWRAPPER_CREATE_RP3(uint32,uint32,0,0,
                         arad_kbp_rop_read,
                         int,int,unit,0,0,
                         uint32,uint32,core,0,0,
                         arad_kbp_rop_read_t*,arad_kbp_rop_read_t,rd_data,1,0);
#endif /* #if defined(BCM_PETRA_SUPPORT) && defined(INCLUDE_KBP) */

CINT_FWRAPPER_CREATE_RP5(int,int,0,0,
                         soc_port_phy_pll_os_set,
                         int,int,unit,0,0,
                         int,int,port,0,0,
                         uint32,uint32,vco_freq,0,0,
                         uint32,uint32,oversample_mode,0,0,
                         uint32,uint32,pll_divider,0,0);

#if defined(BCM_PETRA_SUPPORT)
/* 
 * Dram user access access functions
 */
CINT_FWRAPPER_CREATE_RP4(uint32,uint32,0,0,
                         arad_dram_logical2physical_addr_mapping,
                         int,int,unit,0,0,
                         int,int,buf_num,0,0,
                         int,int,index,0,0,
                         uint32*,uint32,phy_addr,1,0);

CINT_FWRAPPER_CREATE_RP6(int,int,0,0,
                         soc_arad_user_buffer_dram_access,
                         int,int,unit,0,0,
                         uint32,uint32,flags,0,0,
                         uint32,uint32,access_type,0,0,
                         uint8*,uint8,buf,1,0,
                         int,int,offset,0,0,
                         int,int,nbytes,0,0);

CINT_FWRAPPER_CREATE_RP5(int,int,0,0,
                         soc_arad_user_buffer_dram_read,
                         int,int,unit,0,0,
                         uint32,uint32,flags,0,0,
                         uint8*,uint8,buf,1,0,
                         int,int,offset,0,0,
                         int,int,nbytes,0,0);

CINT_FWRAPPER_CREATE_RP5(int,int,0,0,
                         soc_arad_user_buffer_dram_write,
                         int,int,unit,0,0,
                         uint32,uint32,flags,0,0,
                         uint8*,uint8,buf,1,0,
                         int,int,offset,0,0,
                         int,int,nbytes,0,0);
                         
/*
 * DRC init functions
 */

CINT_FWRAPPER_CREATE_RP2(int,int,0,0,
                         soc_dpp_drc_combo28_dprc_out_of_reset,
                         int,int,unit,0,0,
                         int,int,dram_ndx,0,0);

CINT_FWRAPPER_CREATE_RP2(int,int,0,0,
                         soc_dpp_drc_combo28_drc_clam_shell_cfg,
                         int,int,dram_ndx,0,0,
                         int,int,init,0,0);

CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_dpp_drc_combo28_drc_soft_init,
                         int,int,unit,0,0,
                         int,int,dram_ndx,0,0,
                         int,int,init,0,0);
/*
 * Shmoo library functions 
 */
CINT_FWRAPPER_CREATE_RP2(int,int,0,0,
                         soc_combo28_shmoo_phy_cfg_pll,
                         int,int,unit,0,0,
                         int,int,dram_ndx,0,0);

CINT_FWRAPPER_CREATE_RP2(int,int,0,0,
                         soc_combo28_shmoo_phy_init,
                         int,int,unit,0,0,
                         int,int,dram_ndx,0,0);

/*
 * CallBack Functions 
 */
CINT_FWRAPPER_CREATE_RP4(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_phy_reg_read,
                         int,int,unit,0,0,
                         int,int,dram_ndx,0,0,
                         uint32,uint32,addr,0,0,
                         uint32*,uint32,data,1,0);

CINT_FWRAPPER_CREATE_RP4(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_phy_reg_write,
                         int,int,unit,0,0,
                         int,int,dram_ndx,0,0,
                         uint32,uint32,addr,0,0,
                         uint32,uint32,data,0,0);

CINT_FWRAPPER_CREATE_RP5(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_phy_reg_modify,
                         int,int,unit,0,0,
                         int,int,dram_ndx,0,0,
                         uint32,uint32,addr,0,0,
                         uint32,uint32,data,0,0,
                         uint32,uint32,mask,0,0);

CINT_FWRAPPER_CREATE_RP2(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_drc_trigger_dram_init,
                         int,int,unit,0,0,
                         int,int,dram_ndx,0,0);

CINT_FWRAPPER_CREATE_RP2(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_wait_dram_init_done,
                         int,int,unit,0,0,
                         int,int,dram_ndx,0,0);

CINT_FWRAPPER_CREATE_RP4(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_load_mrs,
                         int,int,unit,0,0,
                         int,int,dram_ndx,0,0,
                         uint32,uint32,mrs_num,0,0,
                         uint32,uint32,mrs_opcode,0,0);

CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_drc_enable_adt,
                         int,int,unit,0,0,
                         int,int,dram_ndx,0,0,
                         int,int,action,0,0);

CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_drc_enable_wck2ck_training,
                         int,int,unit,0,0,
                         int,int,dram_ndx,0,0,
                         int,int,action,0,0);

CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_drc_pll_set,
                         int,int,unit,0,0,
                         int,int,dram_ndx,0,0,
                         CONST combo28_drc_pll_t*,combo28_drc_pll_t,pll_info,1,0);

CINT_FWRAPPER_CREATE_RP5(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_drc_enable_write_leveling,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         uint32,uint32,command_parity_lattency,0,0,
                         int,int,use_continious_gddr5_dqs,0,0,
                         int,int,enable,0,0);

CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_drc_mpr_en,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         int,int,enable,0,0);

CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_drc_enable_gddr5_training_protocol,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         int,int,enable,0,0);

CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_vendor_info_get,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         combo28_vendor_info_t*,combo28_vendor_info_t,info,1,0);

CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_drc_dqs_pulse_gen,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         int, int, use_continious_gddr5_dqs,0 ,0);

CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_dram_init,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         int, int, phase,0 ,0);

CINT_FWRAPPER_CREATE_RP5(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_modify_mrs,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         uint32, uint32, mr_ndx, 0 ,0,
                         uint32, uint32, data, 0 ,0,
                         uint32, uint32, mask, 0 ,0);

CINT_FWRAPPER_CREATE_RP8(int,int,0,0,
                         soc_dpp_drc_combo28_dram_cpu_command,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         uint32, uint32, ras_n, 0 ,0,
                         uint32, uint32, cas_n, 0 ,0,
                         uint32, uint32, we_n, 0 ,0,
                         uint32, uint32, act_n, 0 ,0,
                         uint32, uint32, bank, 0 ,0,
                         uint32, uint32, address, 0 ,0);
CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_drc_bist_conf_set,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         combo28_bist_info_t,combo28_bist_info_t,info,0,0);

CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_drc_bist_err_cnt,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         combo28_bist_err_cnt_t*,combo28_bist_err_cnt_t,info,1,0);

CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_dpp_drc_combo28_gddr5_shmoo_drc_bist_conf_set,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         combo28_gddr5_bist_info_t,combo28_gddr5_bist_info_t,info,0,0);

CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_dpp_drc_combo28_gddr5_shmoo_drc_bist_err_cnt,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         combo28_gddr5_bist_err_cnt_t*,combo28_gddr5_bist_err_cnt_t,info,1,0);

CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_dpp_drc_combo28_gddr5_shmoo_drc_dq_byte_pairs_swap_info_get,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         int*,int, pairs_were_swapped,1,0);

CINT_FWRAPPER_CREATE_RP2(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_drc_precharge_all,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0);

CINT_FWRAPPER_CREATE_RP2(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_drc_active_gddr5_cmd,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0);

CINT_FWRAPPER_CREATE_RP7(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_write_mpr,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         uint32, uint32, intial_calib_mpr_addr, 0 ,0,
                         uint32, uint32, mpr_mode, 0 ,0,
                         uint32, uint32, mpr_page, 0 ,0,
                         uint32, uint32, mrs_readout, 0 ,0,
                         int, int, enable_mpr, 0 ,0);

CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_mpr_load,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         uint8*, uint8, mpr_pattern, 1 ,0);

CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_enable_wr_crc,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         int,int,enable,0,0);
                         
CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_enable_rd_crc,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         int,int,enable,0,0);
                         
CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_enable_wr_dbi,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         int,int,enable,0,0);

CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_enable_rd_dbi,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         int,int,enable,0,0);

CINT_FWRAPPER_CREATE_RP5(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_enable_refresh,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         int,int,enable,0,0,
                         uint32,uint32,new_trefi,0,0,
                         uint32*,uint32,curr_refi,1,0);

CINT_FWRAPPER_CREATE_RP4(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_force_dqs,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0,
                         uint32,uint32,force_dqs_val,0,0,
                         uint32,uint32,force_dqs_oeb,0,0);

CINT_FWRAPPER_CREATE_RP2(int,int,0,0,
                         soc_dpp_drc_combo28_soft_reset_drc_without_dram,
                         int,int,unit,0,0,
                         int,int,drc_ndx,0,0);

CINT_FWRAPPER_CREATE_RP2(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_dram_info_access,
                         int,int,unit,0,0,
                         combo28_shmoo_dram_info_t**,combo28_shmoo_dram_info_t,shmoo_info,0,0);

CINT_FWRAPPER_CREATE_RP2(int,int,0,0,
                         soc_dpp_drc_combo28_shmoo_vendor_info_access,
                         int,int,unit,0,0,
                         combo28_vendor_info_t**,combo28_vendor_info_t,vendor_info,0,0);

#endif /* BCM_PETRA_SUPPORT */

#endif /* defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined(BCM_DFE_SUPPORT)|| defined(BCM_PETRA_SUPPORT) */


#if defined(BCM_CALADAN3_SUPPORT)
static cint_enum_map_t __cint_enum_map__soc_c3_rce_qual_type_t[] =
{
    { "socC3RCEQualType_prefix", socC3RCEQualType_prefix },
    { "socC3RCEQualType_postfix", socC3RCEQualType_postfix },
    { "socC3RCEQualType_masked", socC3RCEQualType_masked },
    { "socC3RCEQualType_exact", socC3RCEQualType_exact },
    { "socC3RCEQualType_prefix_sparse", socC3RCEQualType_prefix_sparse },
    { "socC3RCEQualType_postfix_sparse", socC3RCEQualType_postfix_sparse },
    { "socC3RCEQualType_masked_sparse", socC3RCEQualType_masked_sparse },
    { "socC3RCEQualType_exact_sparse", socC3RCEQualType_exact_sparse },
    { "socC3RCEQualTypeCount", socC3RCEQualTypeCount },
    CINT_ENTRY_LAST
};
static cint_enum_map_t __cint_enum_map__soc_c3_rce_action_type_t[] =
{
    { "socC3RCEActionType_bitfield", socC3RCEActionType_bitfield },
    { "socC3RCEActionTypeCount", socC3RCEActionTypeCount },
    CINT_ENTRY_LAST
};
static cint_enum_map_t __cint_enum_map__soc_c3_rce_data_header_t[] =
{
    { "socC3RceDataDirectKey", socC3RceDataDirectKey },
    { "socC3RceDataMetadata", socC3RceDataMetadata },
    { "socC3RceDataHeaderRaw", socC3RceDataHeaderRaw },
    { "socC3RceDataHeaderEther", socC3RceDataHeaderEther },
    { "socC3RceDataHeaderVlan", socC3RceDataHeaderVlan },
    { "socC3RceDataHeaderMpls", socC3RceDataHeaderMpls },
    { "socC3RceDataHeaderIpv4", socC3RceDataHeaderIpv4 },
    { "socC3RceDataHeaderIpv6", socC3RceDataHeaderIpv6 },
    { "socC3RceDataHeaderTcpUdp", socC3RceDataHeaderTcpUdp },
    { "socC3RceDataOffsetCount", socC3RceDataOffsetCount },
    CINT_ENTRY_LAST
};
static cint_enum_map_t __cint_enum_map__soc_c3_rce_metadata_type_t[] =
{
    { "socC3RceMetadataInPortNum", socC3RceMetadataInPortNum },
    { "socC3RceMetadataOutPortNum", socC3RceMetadataOutPortNum },
    { "socC3RceMetadataCount", socC3RceMetadataCount },
    CINT_ENTRY_LAST
};
static cint_enum_map_t __cint_enum_map__soc_c3_rce_action_uc_type_t[] =
{
    { "socC3RceActionEnable", socC3RceActionEnable },
    { "socC3RceActionNewVsi", socC3RceActionNewVsi },
    { "socC3RceActionNewFtIndex", socC3RceActionNewFtIndex },
    { "socC3RceActionMirror", socC3RceActionMirror },
    { "socC3RceActionException", socC3RceActionException },
    { "socC3RceActionCopyToCpu", socC3RceActionCopyToCpu },
    { "socC3RceActionDrop", socC3RceActionDrop },
    { "socC3RceActionPolicer", socC3RceActionPolicer },
    { "socC3RceActionNewPrio", socC3RceActionNewPrio },
    { "socC3RceActionNewDp", socC3RceActionNewDp },
    { "socC3RceActionCounter", socC3RceActionCounter },
    { "socC3RceActionCustom", socC3RceActionCustom },
    { "socC3RceActionCount", socC3RceActionCount },
    CINT_ENTRY_LAST
};

static cint_parameter_desc_t __cint_struct_members__soc_c3_rce_header_field_info_t[] =
{
    {
        "soc_c3_rce_data_header_t",
        "header",
        0,
        0
    },
    {
        "int",
        "startBit",
        0,
        0
    },
    {
        "unsigned int",
        "numBits",
        0,
        0
    },
    {
        "char",
        "fieldName",
        1,
        0
    },
    CINT_ENTRY_LAST
};
static void*
__cint_maddr__soc_c3_rce_header_field_info_t(void* p, int mnum, cint_struct_type_t* parent)
{
    soc_c3_rce_header_field_info_t* s = (soc_c3_rce_header_field_info_t*) p;

    switch(mnum) {
    case 0: return &(s->header);
    case 1: return &(s->startBit);
    case 2: return &(s->numBits);
    case 3: return &(s->fieldName);
    default: return NULL;
    }
}
static cint_parameter_desc_t __cint_struct_members__soc_c3_rce_start_len_t[] =
{
    {
        "uint16",
        "startBit",
        0,
        0
    },
    {
        "uint16",
        "numBits",
        0,
        0
    },
    CINT_ENTRY_LAST
};
static void*
__cint_maddr__soc_c3_rce_start_len_t(void* p, int mnum, cint_struct_type_t* parent)
{
    soc_c3_rce_start_len_t* s = (soc_c3_rce_start_len_t*) p;

    switch(mnum) {
    case 0: return &(s->startBit);
    case 1: return &(s->numBits);
    default: return NULL;
    }
}
static cint_parameter_desc_t __cint_struct_members__soc_c3_rce_qual_uc_desc_t[] =
{
    {
        "soc_c3_rce_header_field_info_t",
        "hdr",
        0,
        0
    },
    {
        "soc_c3_rce_start_len_t",
        "loc",
        0,
        SOC_C3_RCE_MAX_SEG_PER_QUALIFIER,
    },
    CINT_ENTRY_LAST
};
static void*
__cint_maddr__soc_c3_rce_qual_uc_desc_t(void* p, int mnum, cint_struct_type_t* parent)
{
    soc_c3_rce_qual_uc_desc_t* s = (soc_c3_rce_qual_uc_desc_t*) p;

    switch(mnum) {
    case 0: return &(s->hdr);
    case 1: return &(s->loc);
    default: return NULL;
    }
}
static cint_parameter_desc_t __cint_struct_members__soc_c3_rce_action_uc_desc_t[] =
{
    {
        "soc_c3_rce_action_uc_type_t",
        "action",
        0,
        0
    },
    {
        "char",
        "actionName",
        1,
        0
    },
    {
        "uint16",
        "enableIndex",
        0,
        0
    },
    {
        "uint32",
        "disableVal",
        0,
        0
    },
    {
        "soc_c3_rce_start_len_t",
        "loc",
        0,
        SOC_C3_RCE_MAX_SEG_PER_ACTION
    },
    CINT_ENTRY_LAST
};
static void*
__cint_maddr__soc_c3_rce_action_uc_desc_t(void* p, int mnum, cint_struct_type_t* parent)
{
    soc_c3_rce_action_uc_desc_t* s = (soc_c3_rce_action_uc_desc_t*) p;

    switch(mnum) {
    case 0: return &(s->action);
    case 1: return &(s->actionName);
    case 2: return &(s->enableIndex);
    case 3: return &(s->disableVal);
    case 4: return &(s->loc);
    default: return NULL;
    }
}
static cint_parameter_desc_t __cint_struct_members__soc_c3_rce_qual_desc_t[] =
{
    {
        "soc_c3_rce_qual_type_t",
        "qualType",
        0,
        0
    },
    {
        "char",
        "qualName",
        1,
        0
    },
    {
        "unsigned int",
        "paramCount",
        0,
        0
    },
    {
        "int",
        "param",
        1,
        0
    },
    CINT_ENTRY_LAST
};
static void*
__cint_maddr__soc_c3_rce_qual_desc_t(void* p, int mnum, cint_struct_type_t* parent)
{
    soc_c3_rce_qual_desc_t* s = (soc_c3_rce_qual_desc_t*) p;

    switch(mnum) {
    case 0: return &(s->qualType);
    case 1: return &(s->qualName);
    case 2: return &(s->paramCount);
    case 3: return &(s->param);
    default: return NULL;
    }
}
static cint_parameter_desc_t __cint_struct_members__soc_c3_rce_action_desc_t[] =
{
    {
        "soc_c3_rce_action_type_t",
        "actionType",
        0,
        0
    },
    {
        "unsigned int",
        "paramCount",
        0,
        0
    },
    {
        "int",
        "param",
        1,
        0
    },
    CINT_ENTRY_LAST
};
static void*
__cint_maddr__soc_c3_rce_action_desc_t(void* p, int mnum, cint_struct_type_t* parent)
{
    soc_c3_rce_action_desc_t* s = (soc_c3_rce_action_desc_t*) p;

    switch(mnum) {
    case 0: return &(s->actionType);
    case 1: return &(s->paramCount);
    case 2: return &(s->param);
    default: return NULL;
    }
}
static cint_parameter_desc_t __cint_struct_members__soc_c3_rce_unit_desc_t[] =
{
    {
        "unsigned int",
        "programCount",
        0,
        0
    },
    {
        "unsigned int",
        "actionTableCount",
        0,
        0
    },
    {
        "unsigned int",
        "rangeMaxCount",
        0,
        0
    },
    {
        "unsigned int",
        "groupMaxCount",
        0,
        0
    },
    {
        "unsigned int",
        "entryMaxCount",
        0,
        0
    },
    {
        "unsigned int",
        "rangesInUse",
        0,
        0
    },
    {
        "unsigned int",
        "groupsInUse",
        0,
        0
    },
    {
        "unsigned int",
        "entriesInUse",
        0,
        0
    },
    CINT_ENTRY_LAST
};
static void*
__cint_maddr__soc_c3_rce_unit_desc_t(void* p, int mnum, cint_struct_type_t* parent)
{
    soc_c3_rce_unit_desc_t* s = (soc_c3_rce_unit_desc_t*) p;

    switch(mnum) {
    case 0: return &(s->programCount);
    case 1: return &(s->actionTableCount);
    case 2: return &(s->rangeMaxCount);
    case 3: return &(s->groupMaxCount);
    case 4: return &(s->entryMaxCount);
    case 5: return &(s->rangesInUse);
    case 6: return &(s->groupsInUse);
    case 7: return &(s->entriesInUse);
    default: return NULL;
    }
}
static cint_parameter_desc_t __cint_struct_members__soc_c3_rce_program_desc_t[] =
{
    {
        "uint32",
        "tmuProg",
        0,
        0
    },
    {
        "unsigned int",
        "entryMaxCount",
        0,
        0
    },
    {
        "unsigned int",
        "groupMaxCount",
        0,
        0
    },
    {
        "unsigned int",
        "keyFieldCount",
        0,
        0
    },
    {
        "soc_c3_rce_qual_uc_desc_t",
        "keyFields",
        1,
        0
    },
    {
        "uint8",
        "actionTable",
        0,
        0,
        CINT_PARAM_VL,
        1,
        { SOC_C3_RCE_RESULT_REGISTER_COUNT, 0, 0, 0 }
    },
    {
        "unsigned int",
        "entriesInUse",
        0,
        0
    },
    {
        "unsigned int",
        "groupsInUse",
        0,
        0
    },
    CINT_ENTRY_LAST
};
static void*
__cint_maddr__soc_c3_rce_program_desc_t(void* p, int mnum, cint_struct_type_t* parent)
{
    soc_c3_rce_program_desc_t* s = (soc_c3_rce_program_desc_t*) p;

    switch(mnum) {
    case 0: return &(s->tmuProg);
    case 1: return &(s->entryMaxCount);
    case 2: return &(s->groupMaxCount);
    case 3: return &(s->keyFieldCount);
    case 4: return &(s->keyFields);
    case 5: return &(s->actionTable);
    case 6: return &(s->entriesInUse);
    case 7: return &(s->groupsInUse);
    default: return NULL;
    }
}
static cint_parameter_desc_t __cint_struct_members__soc_c3_rce_actiontable_desc_t[] =
{
    {
        "char",
        "tableName",
        1,
        0
    },
    {
        "char",
        "counterName",
        1,
        0
    },
    {
        "unsigned int",
        "entryBlocks",
        0,
        0
    },
    {
        "unsigned int",
        "actionCount",
        0,
        0
    },
    {
        "soc_c3_rce_action_uc_desc_t",
        "actFields",
        1,
        0
    },
    {
        "unsigned int",
        "entryBlocksUsed",
        0,
        0
    },
    CINT_ENTRY_LAST
};
static void*
__cint_maddr__soc_c3_rce_actiontable_desc_t(void* p, int mnum, cint_struct_type_t* parent)
{
    soc_c3_rce_actiontable_desc_t* s = (soc_c3_rce_actiontable_desc_t*) p;

    switch(mnum) {
    case 0: return &(s->tableName);
    case 1: return &(s->counterName);
    case 2: return &(s->entryBlocks);
    case 3: return &(s->actionCount);
    case 4: return &(s->actFields);
    case 5: return &(s->entryBlocksUsed);
    default: return NULL;
    }
}
static cint_parameter_desc_t __cint_struct_members__soc_c3_rce_group_desc_t[] =
{
    {
        "uint8",
        "resultLrp",
        0,
        0
    },
    {
        "uint8",
        "resultRce",
        0,
        0
    },
    {
        "uint8",
        "rceProgram",
        0,
        0
    },
    {
        "int",
        "groupPriority",
        0,
        0
    },
    {
        "unsigned int",
        "rangesPerFilterSet",
        0,
        0
    },
    {
        "unsigned int",
        "maxFilterSets",
        0,
        0
    },
    {
        "unsigned int",
        "qualCount",
        0,
        0
    },
    {
        "soc_c3_rce_qual_desc_t",
        "qualData",
        1,
        0,
        CINT_PARAM_VL,
        1,
        { SOC_C3_RCE_GROUP_QUAL_MAX, 0, 0, 0 }
    },
    {
        "unsigned int",
        "entriesInUse",
        0,
        0
    },
    {
        "unsigned int",
        "instrCount",
        0,
        0
    },
    {
        "unsigned int",
        "filterSetCount",
        0,
        0
    },
    CINT_ENTRY_LAST
};
static void*
__cint_maddr__soc_c3_rce_group_desc_t(void* p, int mnum, cint_struct_type_t* parent)
{
    soc_c3_rce_group_desc_t* s = (soc_c3_rce_group_desc_t*) p;

    switch(mnum) {
    case 0: return &(s->resultLrp);
    case 1: return &(s->resultRce);
    case 2: return &(s->rceProgram);
    case 3: return &(s->groupPriority);
    case 4: return &(s->rangesPerFilterSet);
    case 5: return &(s->maxFilterSets);
    case 6: return &(s->qualCount);
    case 7: return &(s->qualData);
    case 8: return &(s->entriesInUse);
    case 9: return &(s->instrCount);
    case 10: return &(s->filterSetCount);
    default: return NULL;
    }
}
static cint_parameter_desc_t __cint_struct_members__soc_c3_rce_entry_desc_t[] =
{
    {
        "uint32",
        "entryFlags",
        0,
        0
    },
    {
        "int",
        "groupId",
        0,
        0
    },
    {
        "int",
        "entryPriority",
        0,
        0
    },
    CINT_ENTRY_LAST
};
static void*
__cint_maddr__soc_c3_rce_entry_desc_t(void* p, int mnum, cint_struct_type_t* parent)
{
    soc_c3_rce_entry_desc_t* s = (soc_c3_rce_entry_desc_t*) p;

    switch(mnum) {
    case 0: return &(s->entryFlags);
    case 1: return &(s->groupId);
    case 2: return &(s->entryPriority);
    default: return NULL;
    }
}
static cint_parameter_desc_t __cint_struct_members__soc_c3_rce_range_desc_t[] =
{
    {
        "uint32",
        "rangeFlags",
        0,
        0
    },
    {
        "soc_c3_rce_header_field_info_t",
        "headerField",
        0,
        0
    },
    {
        "int",
        "lowerBound",
        0,
        0
    },
    {
        "int",
        "upperBound",
        0,
        0
    },
    {
        "uint32",
        "validProgs",
        0,
        0
    },
    {
        "unsigned int",
        "refCount",
        0,
        0
    },
    CINT_ENTRY_LAST
};
static void*
__cint_maddr__soc_c3_rce_range_desc_t(void* p, int mnum, cint_struct_type_t* parent)
{
    soc_c3_rce_range_desc_t* s = (soc_c3_rce_range_desc_t*) p;

    switch(mnum) {
    case 0: return &(s->rangeFlags);
    case 1: return &(s->headerField);
    case 2: return &(s->lowerBound);
    case 3: return &(s->upperBound);
    case 4: return &(s->validProgs);
    case 5: return &(s->refCount);
    default: return NULL;
    }
}
static int
__cint_fpointer__soc_c3_rce_program_traverse_cb_t(int unit,
                                                  int programId,
                                                  void *extras)
{
    int returnVal;
    cint_interpreter_callback(__cint_soc_function_pointers+0,
                              3,
                              1,
                              &unit,
                              &programId,
                              &extras,
                              &returnVal);
    return returnVal;
}
static int
__cint_fpointer__soc_c3_rce_group_traverse_cb_t(int unit,
                                                int programId,
                                                int groupId,
                                                void *extras)
{
    int returnVal;
    cint_interpreter_callback(__cint_soc_function_pointers+1,
                              4,
                              1,
                              &unit,
                              &programId,
                              &groupId,
                              &extras,
                              &returnVal);
    return returnVal;
}
static int
__cint_fpointer__soc_c3_rce_entry_traverse_cb_t(int unit,
                                                int programId,
                                                int groupId,
                                                int entryId,
                                                void *extras)
{
    int returnVal;
    cint_interpreter_callback(__cint_soc_function_pointers+2,
                              5,
                              1,
                              &unit,
                              &programId,
                              &groupId,
                              &entryId,
                              &extras,
                              &returnVal);
    return returnVal;
}
static int
__cint_fpointer__soc_c3_rce_entry_qualify_range_traverse_cb_t(int unit,
                                                              int programId,
                                                              int groupId,
                                                              int entryId,
                                                              int rangeId,
                                                              int qualify,
                                                              void *extras)
{
    int returnVal;
    cint_interpreter_callback(__cint_soc_function_pointers+3,
                              7,
                              1,
                              &unit,
                              &programId,
                              &groupId,
                              &entryId,
                              &rangeId,
                              &qualify,
                              &extras,
                              &returnVal);
    return returnVal;
}
static int
__cint_fpointer__soc_c3_rce_range_traverse_cb_t(int unit,
                                                int rangeId,
                                                void *extras)
{
    int returnVal;
    cint_interpreter_callback(__cint_soc_function_pointers+4,
                              3,
                              1,
                              &unit,
                              &rangeId,
                              &extras,
                              &returnVal);
    return returnVal;
}

CINT_FWRAPPER_CREATE2_RP1(int, int, 0, 0, soc_c3_rce_init,
                          int, int, unit, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP1(int, int, 0, 0, soc_c3_rce_detach,
                          int, int, unit, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP1(int, int, 0, 0, soc_c3_rce_wb_immed_sync,
                          int, int, unit, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP2(int, int, 0, 0, soc_c3_rce_state_check,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          uint32, uint32, flags, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP2(int, int, 0, 0, soc_c3_rce_info_get,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_unit_desc_t**, soc_c3_rce_unit_desc_t, unitInfo, 2, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_VP2(soc_c3_rce_info_free,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_unit_desc_t*, soc_c3_rce_unit_desc_t, unitInfo, 1, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP3(int, int, 0, 0, soc_c3_rce_dump,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          uint32, uint32, flags, 0, 0, CINT_PARAM_IN,
                          const char*, char, prefix, 1, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP5(int, int, 0, 0, soc_c3_rce_debug_capture_parse,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          uint32*, uint32, flags, 1, 0, CINT_PARAM_OUT,
                          uint8*, uint8, keyData, 1, 0, CINT_PARAM_OUT,
                          int*, int, programId, 1, 0, CINT_PARAM_OUT,
                          int*, int, entryIds, 1, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_RP6(int, int, 0, 0, soc_c3_rce_debug_capture_set,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          uint32, uint32, flags, 0, 0, CINT_PARAM_IN,
                          uint32, uint32, threshold, 0, 0, CINT_PARAM_IN,
                          const uint8*, uint8, keyData, 1, 0, CINT_PARAM_IN,
                          const uint8*, uint8, keyMask, 1, 0, CINT_PARAM_IN,
                          int, int, programId, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP7(int, int, 0, 0, soc_c3_rce_debug_capture_get,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          uint32*, uint32, flags, 1, 0, CINT_PARAM_OUT,
                          uint32*, uint32, threshold, 1, 0, CINT_PARAM_OUT,
                          uint32*, uint32, threshCount, 1, 0, CINT_PARAM_OUT,
                          uint8*, uint8, keyData, 1, 0, CINT_PARAM_OUT,
                          uint8*, uint8, keyMask, 1, 0, CINT_PARAM_OUT,
                          int*, int, programId, 1, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_RP3(int, int, 0, 0, soc_c3_rce_result_hit_counter_read,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, hitCounter, 0, 0, CINT_PARAM_IN,
                          uint32*, uint32, value, 1, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_RP4(int, int, 0, 0, soc_c3_rce_result_hit_counter_set,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, hitCounter, 0, 0, CINT_PARAM_IN,
                          int, int, entryId0, 0, 0, CINT_PARAM_IN,
                          int, int, entryId1, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP3(int, int, 0, 0, soc_c3_rce_program_next_existing,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, currProgramId, 0, 0, CINT_PARAM_IN,
                          int*, int, nextProgramId, 1, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_RP3(int, int, 0, 0, soc_c3_rce_program_info_get,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, programId, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_program_desc_t**, soc_c3_rce_program_desc_t, programInfo, 2, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_VP2(soc_c3_rce_program_info_free,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_program_desc_t*, soc_c3_rce_program_desc, programInfo, 1, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP3(int, int, 0, 0, soc_c3_rce_program_traverse,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_program_traverse_cb_t, soc_c3_rce_program_traverse_cb_t, callback, 0, 0, CINT_PARAM_IN,
                          void*, void, extras, 1, 0, CINT_PARAM_INOUT);
CINT_FWRAPPER_CREATE2_RP4(int, int, 0, 0, soc_c3_rce_program_dump,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          uint32, uint32, flags, 0, 0, CINT_PARAM_IN,
                          const char*, char, prefix, 1, 0, CINT_PARAM_IN,
                          int, int, programId, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP5(int, int, 0, 0, soc_c3_rce_program_qualifier_build,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, programId, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_header_field_info_t*, soc_c3_rce_header_field_info_t, headerField, 1, 0, CINT_PARAM_INOUT,
                          soc_c3_rce_qual_type_t, soc_c3_rce_qual_type_t, qualType, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_qual_desc_t**, soc_c3_rce_qual_desc_t, qualDesc, 2, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_VP2(soc_c3_rce_program_qualifier_free,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_qual_desc_t*, soc_c3_rce_qual_desc_t, qualDesc, 1, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP4(int, int, 0, 0, soc_c3_rce_program_scan,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, programId, 0, 0, CINT_PARAM_IN,
                          const uint8*, uint8, keyData, 1, 0, CINT_PARAM_IN,
                          int*, int, hitEntries, 1, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_RP3(int, int, 0, 0, soc_c3_rce_actiontable_info_get,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          unsigned int, unsigned int, actionTable, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_actiontable_desc_t**, soc_c3_rce_actiontable_desc_t, actionTableInfo, 2, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_VP2(soc_c3_rce_actiontable_info_free,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_actiontable_desc_t*, soc_c3_rce_actiontable_desc_t, actionTableInfo, 1, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP2(int, int, 0, 0, soc_c3_rce_group_first_avail,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int*, int, firstAvailGroupId, 1, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_RP3(int, int, 0, 0, soc_c3_rce_group_next_existing,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, currGroupId, 0, 0, CINT_PARAM_IN,
                          int*, int, nextGroupId, 1, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_RP3(int, int, 0, 0, soc_c3_rce_group_info_get,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, groupId, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_group_desc_t**, soc_c3_rce_group_desc_t, groupInfo, 2, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_VP2(soc_c3_rce_group_info_free,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_group_desc_t*, soc_c3_rce_group_desc_t, groupInfo, 1, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP4(int, int, 0, 0, soc_c3_rce_group_traverse,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, programId, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_group_traverse_cb_t, soc_c3_rce_group_traverse_cb_t, callback, 0, 0, CINT_PARAM_IN,
                          void*, void, extras, 1, 0, CINT_PARAM_INOUT);
CINT_FWRAPPER_CREATE2_RP4(int, int, 0, 0, soc_c3_rce_group_dump,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          uint32, uint32, flags, 0, 0, CINT_PARAM_IN,
                          const char*, char, prefix, 1, 0, CINT_PARAM_IN,
                          int, int, groupId, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP3(int, int, 0, 0, soc_c3_rce_group_create,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, groupId, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_group_desc_t*, soc_c3_rce_group_desc_t, groupInfo, 1, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP2(int, int, 0, 0, soc_c3_rce_group_destroy,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, groupId, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP2(int, int, 0, 0, soc_c3_rce_group_install,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, groupId, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP2(int, int, 0, 0, soc_c3_rce_group_remove,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, groupId, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP2(int, int, 0, 0, soc_c3_rce_group_compress,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, groupId, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP2(int, int, 0, 0, soc_c3_rce_entry_first_avail,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int*, int, firstAvailEntryId, 1, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_RP3(int, int, 0, 0, soc_c3_rce_entry_next_existing,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, currEntryId, 0, 0, CINT_PARAM_IN,
                          int*, int, nextEntryId, 1, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_RP3(int, int, 0, 0, soc_c3_rce_entry_info_get,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, entryId, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_entry_desc_t**, soc_c3_rce_entry_desc_t, entryInfo, 2, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_VP2(soc_c3_rce_entry_info_free,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_entry_desc_t*, soc_c3_rce_entry_desc_t, entryInfo, 1, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP4(int, int, 0, 0, soc_c3_rce_entry_traverse,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, groupId, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_entry_traverse_cb_t, soc_c3_rce_entry_traverse_cb_t, callback, 0, 0, CINT_PARAM_IN,
                          void*, void, extras, 1, 0, CINT_PARAM_INOUT);
CINT_FWRAPPER_CREATE2_RP4(int, int, 0, 0, soc_c3_rce_entry_dump,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          uint32, uint32, flags, 0, 0, CINT_PARAM_IN,
                          const char*, char, prefix, 1, 0, CINT_PARAM_IN,
                          int, int, entryId, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP3(int, int, 0, 0, soc_c3_rce_entry_create,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, entryId, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_entry_desc_t*, soc_c3_rce_entry_desc_t, entryInfo, 1, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP5(int, int, 0, 0, soc_c3_rce_entry_copy,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          uint32, uint32, copyFlags, 0, 0, CINT_PARAM_IN,
                          int, int, entryId, 0, 0, CINT_PARAM_IN,
                          const soc_c3_rce_entry_desc_t*, soc_c3_rce_entry_desc_t, entryInfo, 1, 0, CINT_PARAM_IN,
                          int, int, sourceEntryId, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP3(int, int, 0, 0, soc_c3_rce_entry_priority_set,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, entryId, 0, 0, CINT_PARAM_IN,
                          int, int, entryPriority, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP2(int, int, 0, 0, soc_c3_rce_entry_destroy,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, entryId, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP5(int, int, 0, 0, soc_c3_rce_entry_qualify_set,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, entryId, 0, 0, CINT_PARAM_IN,
                          unsigned int, unsigned int, qualIdx, 0, 0, CINT_PARAM_IN,
                          const uint8*, uint8, data, 1, 0, CINT_PARAM_IN,
                          const uint8*, uint8, mask, 1, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP5(int, int, 0, 0, soc_c3_rce_entry_qualify_get,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, entryId, 0, 0, CINT_PARAM_IN,
                          unsigned int, unsigned int, qualIdx, 0, 0, CINT_PARAM_IN,
                          uint8*, uint8, data, 1, 0, CINT_PARAM_OUT,
                          uint8*, uint8, mask, 1, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_RP4(int, int, 0, 0, soc_c3_rce_entry_qualify_range_traverse,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, entryId, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_entry_qualify_range_traverse_cb_t, soc_c3_rce_entry_qualify_range_traverse_cb_t, callback, 0, 0, CINT_PARAM_IN,
                          void*, void, extras, 1, 0, CINT_PARAM_INOUT);
CINT_FWRAPPER_CREATE2_RP4(int, int, 0, 0, soc_c3_rce_entry_qualify_range_set,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, entryId, 0, 0, CINT_PARAM_IN,
                          int, int, rangeId, 0, 0, CINT_PARAM_IN,
                          int, int, qualify, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP4(int, int, 0, 0, soc_c3_rce_entry_qualify_range_get,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, entryId, 0, 0, CINT_PARAM_IN,
                          int, int, rangeId, 0, 0, CINT_PARAM_IN,
                          int*, int, qualify, 1, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_RP2(int, int, 0, 0, soc_c3_rce_entry_qualify_clear,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, entryId, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP4(int, int, 0, 0, soc_c3_rce_entry_action_set,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, entryId, 0, 0, CINT_PARAM_IN,
                          unsigned int, unsigned int, actIdx, 0, 0, CINT_PARAM_IN,
                          const uint8*, uint8, value, 1, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP4(int, int, 0, 0, soc_c3_rce_entry_action_get,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, entryId, 0, 0, CINT_PARAM_IN,
                          unsigned int, unsigned int, actIdx, 0, 0, CINT_PARAM_IN,
                          uint8*, uint8, value, 1, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_RP2(int, int, 0, 0, soc_c3_rce_entry_action_clear,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, entryId, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP2(int, int, 0, 0, soc_c3_rce_entry_install,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, entryId, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP2(int, int, 0, 0, soc_c3_rce_entry_remove,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, entryId, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP3(int, int, 0, 0, soc_c3_rce_entry_counter_set,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, entryId, 0, 0, CINT_PARAM_IN,
                          const uint64*, uint64, counters, 1, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP4(int, int, 0, 0, soc_c3_rce_entry_counter_get,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, entryId, 0, 0, CINT_PARAM_IN,
                          int, int, clear, 0, 0, CINT_PARAM_IN,
                          uint64*, uint64, counters, 1, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_RP2(int, int, 0, 0, soc_c3_rce_range_first_avail,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int*, int, firstAvailRangeId, 1, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_RP3(int, int, 0, 0, soc_c3_rce_range_next_existing,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, currRangeId, 0, 0, CINT_PARAM_IN,
                          int*, int, nextRangeId, 1, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_RP3(int, int, 0, 0, soc_c3_rce_range_info_get,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, rangeId, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_range_desc_t**, soc_c3_rce_range_desc_t, rangeInfo, 2, 0, CINT_PARAM_OUT);
CINT_FWRAPPER_CREATE2_VP2(soc_c3_rce_range_info_free,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_range_desc_t*, soc_c3_rce_range_desc_t, rangeInfo, 1, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP3(int, int, 0, 0, soc_c3_rce_range_traverse,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          soc_c3_rce_range_traverse_cb_t, soc_c3_rce_range_traverse_cb_t, callback, 0, 0, CINT_PARAM_IN,
                          void*, void, extras, 1, 0, CINT_PARAM_INOUT);
CINT_FWRAPPER_CREATE2_RP4(int, int, 0, 0, soc_c3_rce_range_dump,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          uint32, uint32, flags, 0, 0, CINT_PARAM_IN,
                          const char*, char, prefix, 1, 0, CINT_PARAM_IN,
                          int, int, rangeId, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP3(int, int, 0, 0, soc_c3_rce_range_create,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, rangeId, 0, 0, CINT_PARAM_IN,
                          const soc_c3_rce_range_desc_t *, soc_c3_rce_range_desc_t, rangeInfo, 1, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP2(int, int, 0, 0, soc_c3_rce_range_destroy,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, rangeId, 0, 0, CINT_PARAM_IN);

CINT_FWRAPPER_CREATE_RP3(int, int, 0, 0, soc_sbx_caladan3_cop_policer_pkt_mode_len_get,
                         int, int, unit, 0, 0,
                         int, int, cop, 0, 0,
                         uint32*, uint32, len, 1, 0);

CINT_FWRAPPER_CREATE_RP3(int, int, 0, 0, soc_sbx_caladan3_cop_policer_pkt_mode_len_set,
                         int, int, unit, 0, 0,
                         int, int, cop, 0, 0,
                         uint32, uint32, len, 0, 0);

CINT_FWRAPPER_CREATE_RP4(int, int, 0, 0, soc_sbx_caladan3_cop_policer_token_number_get,
                         int, int, unit, 0, 0,
                         uint32, uint32, handle, 0, 0,
                         uint32*, uint32, token_c, 1, 0,
                         uint32*, uint32, token_e, 1, 0);

CINT_FWRAPPER_CREATE2_RP3(int, int, 0, 0, soc_sbx_caladan3_lr_bubble_enable,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          uint32, uint32, enable, 0, 0, CINT_PARAM_IN,
                          uint32, uint32, size_in_bytes, 0, 0, CINT_PARAM_IN);

CINT_FWRAPPER_CREATE2_RP5(int, int, 0, 0, soc_sbx_caladan3_get_queues_from_port,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          int, int, port, 0, 0, CINT_PARAM_IN,
                          int*, int, squeue, 1, 0, CINT_PARAM_OUT,
                          int*, int, dqueue, 1, 0, CINT_PARAM_OUT,
                          int*, int, numcos, 1, 0, CINT_PARAM_OUT);

CINT_FWRAPPER_CREATE_RP3(int,int,0,0,
                         __strtoul,
                         char*,char,nptr,1,0,
                         char**,char,endptr,2,0,
                         int,int,base,0,0);

CINT_FWRAPPER_CREATE_RP0(int, int, 0, 0,
                         soc_c3_is_pvv2e_taps_enabled);

CINT_FWRAPPER_CREATE_RP0(int,int,0,0,
                         soc_c3_is_sim);

CINT_FWRAPPER_CREATE_RP3(int, int, 0, 0, soc_sbx_caladan3_sim_sendrcv,
                         int, int, unit, 0, 0,
                         char*, char, buffer, 1, 0,
                         int*, int, size, 1, 0);

#endif /* defined (BCM_CALADAN3_SUPPORT) */


#define TEST_TCAM

#ifdef BCM_PETRA_SUPPORT
#ifdef TEST_TCAM

#include <soc/dpp/ARAD/arad_tbl_access.h>


#include <bcm_int/dpp/error.h>


static uint32
arad_pp_ihb_tcam_tbl_read_unsafe_lcl(
                                 SOC_SAND_IN   int                              unit,
                                 SOC_SAND_IN   uint32                           bank_ndx,
                                 SOC_SAND_IN   uint32                           nof_entries_in_line,
                                 SOC_SAND_IN   uint32                           entry_offset,
                                 SOC_SAND_OUT  ARAD_PP_IHB_TCAM_BANK_TBL_DATA    *data
                                 )
{
    uint32
        soc_sand_rc;

    soc_sand_rc = arad_pp_ihb_tcam_tbl_read_unsafe(unit, bank_ndx, nof_entries_in_line, entry_offset, data);
    if(SOC_SAND_FAILURE(soc_sand_rc)){
        return -1;
    }
    return 0;
}


static uint32
arad_pp_ihb_tcam_tbl_write_unsafe_lcl(
                                  SOC_SAND_IN   int                              unit,
                                  SOC_SAND_IN   uint32                           bank_ndx,
                                  SOC_SAND_IN   uint32                           nof_entries_in_line,
                                  SOC_SAND_IN   uint32                            entry_offset,
                                  SOC_SAND_IN   ARAD_PP_IHB_TCAM_BANK_TBL_DATA     *data
                                  )
{
    uint32
        soc_sand_rc;

    soc_sand_rc = arad_pp_ihb_tcam_tbl_write_unsafe(unit, bank_ndx, nof_entries_in_line, entry_offset, data);
    if(SOC_SAND_FAILURE(soc_sand_rc)){
        return -1;
    }
    return 0;
}

static uint32
arad_pp_ihb_tcam_tbl_compare_unsafe_lcl(
                                    SOC_SAND_IN   int                                unit,
                                    SOC_SAND_IN   uint32                             bank_ndx,
                                    SOC_SAND_IN   uint32                             nof_entries_in_line,
                                    SOC_SAND_IN   ARAD_PP_IHB_TCAM_BANK_TBL_DATA     *compare_data,
                                    SOC_SAND_OUT  ARAD_PP_IHB_TCAM_COMPARE_DATA      *found_data
                                    )
{
    uint32
        soc_sand_rc;

    soc_sand_rc = arad_pp_ihb_tcam_tbl_compare_unsafe(unit, bank_ndx, nof_entries_in_line, compare_data, found_data);
    if(SOC_SAND_FAILURE(soc_sand_rc)){
        return -1;
    }
    return 0;
}


CINT_FWRAPPER_CREATE_RP5(uint32, uint32, 0, 0,
                         arad_pp_ihb_tcam_tbl_read_unsafe_lcl,
                         uint32, uint32, unit, 0, 0,
                         uint32, uint32, bank_ndx, 0, 0,
                         uint32, uint32, nof_entries_in_line, 0, 0,
                         uint32, uint32, entry_offset, 0, 0,
                         ARAD_PP_IHB_TCAM_BANK_TBL_DATA*, ARAD_PP_IHB_TCAM_BANK_TBL_DATA, data, 1, 0);

CINT_FWRAPPER_CREATE_RP5(uint32, uint32, 0, 0,
                         arad_pp_ihb_tcam_tbl_write_unsafe_lcl,
                         uint32, uint32, unit, 0, 0,
                         uint32, uint32, bank_ndx, 0, 0,
                         uint32, uint32, nof_entries_in_line, 0, 0,
                         uint32, uint32, entry_offset, 0, 0,
                         ARAD_PP_IHB_TCAM_BANK_TBL_DATA*, ARAD_PP_IHB_TCAM_BANK_TBL_DATA, data, 1, 0);

CINT_FWRAPPER_CREATE_RP5(uint32, uint32, 0, 0,
                         arad_pp_ihb_tcam_tbl_compare_unsafe_lcl,
                         uint32, uint32, unit, 0, 0,
                         uint32, uint32, bank_ndx, 0, 0,
                         uint32, uint32, nof_entries_in_line, 0, 0,
                         ARAD_PP_IHB_TCAM_BANK_TBL_DATA*, ARAD_PP_IHB_TCAM_BANK_TBL_DATA, compare_data, 1, 0,
                         ARAD_PP_IHB_TCAM_COMPARE_DATA*, ARAD_PP_IHB_TCAM_COMPARE_DATA, found_data, 1, 0);

#endif
#endif /* BCM_PETRA_SUPPORT */

/*
 * bitop macros
 */
static void __SHR_BITSET(uint32* data, int n) { SHR_BITSET(data, n); }
CINT_FWRAPPER_CREATE_VP2(__SHR_BITSET, uint32*,int,data,1,0,int,int,n,0,0); 

static void __SHR_BITCLR(uint32* data, int n) { SHR_BITCLR(data, n); }
CINT_FWRAPPER_CREATE_VP2(__SHR_BITCLR, uint32*,int,data,1,0,int,int,n,0,0); 

static int __SHR_BITGET(uint32* data, int n) {return SHR_BITGET(data, n); }
CINT_FWRAPPER_CREATE_RP2(int, int, 0, 0, __SHR_BITGET, uint32*,int,data,1,0,int,int,n,0,0); 

static cint_function_t __cint_soc_functions[] =
    {
        CINT_FWRAPPER_ENTRY(soc_init),
        CINT_FWRAPPER_ENTRY(soc_property_get_str),
        CINT_FWRAPPER_ENTRY(soc_property_get),
        CINT_FWRAPPER_ENTRY(soc_property_get_pbmp),
        CINT_FWRAPPER_ENTRY(soc_property_get_pbmp_default),
        CINT_FWRAPPER_ENTRY(soc_property_port_get_str),
        CINT_FWRAPPER_ENTRY(soc_property_port_get),
        CINT_FWRAPPER_ENTRY(soc_property_suffix_num_get),
        CINT_FWRAPPER_ENTRY(soc_property_cos_get),
        CINT_FWRAPPER_ENTRY(soc_cm_salloc),
        CINT_FWRAPPER_ENTRY(soc_cm_sfree),
#if defined(BCM_ESW_SUPPORT)
#if defined(INCLUDE_I2C)
        CINT_FWRAPPER_ENTRY(soc_i2c_read_word_data),
        CINT_FWRAPPER_ENTRY(soc_i2c_write_word_data),
        CINT_FWRAPPER_ENTRY(soc_i2c_probe),
#endif
        CINT_FWRAPPER_ENTRY(_shr_swap16),
#endif
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined(BCM_DFE_SUPPORT)|| defined(BCM_PETRA_SUPPORT)
        CINT_FWRAPPER_ENTRY(soc_nof_interrupts),
        CINT_FWRAPPER_ENTRY(soc_active_interrupts_get),
        CINT_FWRAPPER_ENTRY(soc_interrupt_info_get),
        CINT_FWRAPPER_ENTRY(soc_get_interrupt_id),
        CINT_FWRAPPER_ENTRY(soc_get_interrupt_id_specific),
        CINT_FWRAPPER_ENTRY(soc_interrupt_is_supported),
        CINT_FWRAPPER_ENTRY(soc_port_phy_pll_os_set),
#ifdef BCM_PETRA_SUPPORT
        CINT_FWRAPPER_ENTRY(arad_dram_logical2physical_addr_mapping),
        CINT_FWRAPPER_ENTRY(soc_arad_user_buffer_dram_access),
        CINT_FWRAPPER_ENTRY(soc_arad_user_buffer_dram_read),
        CINT_FWRAPPER_ENTRY(soc_arad_user_buffer_dram_write),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_dprc_out_of_reset),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_drc_clam_shell_cfg),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_drc_soft_init),
        CINT_FWRAPPER_ENTRY(soc_combo28_shmoo_phy_cfg_pll),
        CINT_FWRAPPER_ENTRY(soc_combo28_shmoo_phy_init),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_phy_reg_read),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_phy_reg_write),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_phy_reg_modify),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_drc_trigger_dram_init),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_wait_dram_init_done),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_load_mrs),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_drc_enable_adt),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_drc_enable_wck2ck_training),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_drc_pll_set),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_drc_enable_write_leveling),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_drc_mpr_en),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_drc_enable_gddr5_training_protocol),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_vendor_info_get),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_drc_dqs_pulse_gen),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_dram_init),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_modify_mrs),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_dram_cpu_command),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_vendor_info_get),  
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_drc_bist_conf_set),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_drc_bist_err_cnt), 
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_gddr5_shmoo_drc_bist_conf_set),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_gddr5_shmoo_drc_bist_err_cnt),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_gddr5_shmoo_drc_dq_byte_pairs_swap_info_get),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_drc_precharge_all),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_drc_active_gddr5_cmd),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_write_mpr),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_mpr_load),  
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_enable_wr_crc),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_enable_rd_crc),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_enable_wr_dbi),          
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_enable_rd_dbi),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_enable_refresh),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_force_dqs),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_soft_reset_drc_without_dram),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_dram_info_access),
        CINT_FWRAPPER_ENTRY(soc_dpp_drc_combo28_shmoo_vendor_info_access),

#endif
#endif
#ifdef BCM_PETRA_SUPPORT
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
        CINT_FWRAPPER_ENTRY(arad_kbp_cpu_lookup_reply),
        CINT_FWRAPPER_ENTRY(arad_kbp_cpu_record_send),
        CINT_FWRAPPER_ENTRY(aradplus_kbp_cpu_record_send),
        CINT_FWRAPPER_ENTRY(arad_kbp_lut_write),
        CINT_FWRAPPER_ENTRY(arad_kbp_lut_read),
        CINT_FWRAPPER_ENTRY(arad_kbp_rop_write),
        CINT_FWRAPPER_ENTRY(arad_kbp_rop_read),
#endif
#ifdef TEST_TCAM
        CINT_FWRAPPER_ENTRY(arad_pp_ihb_tcam_tbl_read_unsafe_lcl),
        CINT_FWRAPPER_ENTRY(arad_pp_ihb_tcam_tbl_write_unsafe_lcl),
        CINT_FWRAPPER_ENTRY(arad_pp_ihb_tcam_tbl_compare_unsafe_lcl),
#endif
#endif /* BCM_PETRA_SUPPORT */
#if defined(BCM_CALADAN3_SUPPORT)
        CINT_FWRAPPER_ENTRY(soc_c3_rce_init),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_detach),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_wb_immed_sync),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_state_check),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_info_get),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_info_free),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_dump),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_debug_capture_parse),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_debug_capture_set),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_debug_capture_get),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_result_hit_counter_read),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_result_hit_counter_set),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_program_next_existing),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_program_info_get),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_program_info_free),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_program_traverse),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_program_dump),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_program_qualifier_build),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_program_qualifier_free),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_program_scan),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_actiontable_info_get),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_actiontable_info_free),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_group_first_avail),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_group_next_existing),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_group_info_get),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_group_info_free),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_group_traverse),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_group_dump),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_group_create),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_group_destroy),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_group_install),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_group_remove),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_group_compress),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_first_avail),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_next_existing),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_info_get),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_info_free),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_traverse),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_dump),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_create),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_copy),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_priority_set),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_destroy),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_qualify_set),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_qualify_get),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_qualify_range_traverse),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_qualify_range_set),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_qualify_range_get),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_qualify_clear),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_action_set),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_action_get),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_action_clear),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_install),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_remove),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_counter_set),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_entry_counter_get),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_range_first_avail),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_range_next_existing),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_range_info_get),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_range_info_free),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_range_traverse),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_range_dump),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_range_create),
        CINT_FWRAPPER_ENTRY(soc_c3_rce_range_destroy),
        CINT_FWRAPPER_ENTRY(soc_sbx_caladan3_cop_policer_pkt_mode_len_get),
        CINT_FWRAPPER_ENTRY(soc_sbx_caladan3_cop_policer_pkt_mode_len_set),
        CINT_FWRAPPER_ENTRY(soc_sbx_caladan3_cop_policer_token_number_get),
        CINT_FWRAPPER_ENTRY(soc_sbx_caladan3_lr_bubble_enable),
        CINT_FWRAPPER_ENTRY(soc_sbx_caladan3_get_queues_from_port),
        CINT_FWRAPPER_ENTRY(soc_c3_is_pvv2e_taps_enabled),
        CINT_FWRAPPER_ENTRY(soc_c3_is_sim),
        CINT_FWRAPPER_ENTRY(soc_sbx_caladan3_sim_sendrcv),
        CINT_FWRAPPER_NENTRY("sal_strtoul", __strtoul),
#endif /* defined (BCM_CALADAN3_SUPPORT) */
        CINT_FWRAPPER_NENTRY("SHR_BITSET", __SHR_BITSET),
        CINT_FWRAPPER_NENTRY("SHR_BITGET", __SHR_BITGET),
        CINT_FWRAPPER_NENTRY("SHR_BITCLR", __SHR_BITCLR),

        CINT_ENTRY_LAST

    };

#ifdef BCM_PETRA_SUPPORT
#ifdef TEST_TCAM
static cint_parameter_desc_t __cint_struct_members__ARAD_PP_IHB_TCAM_BANK_TBL_DATA[] =
{
    {
        "uint32",
        "mask",
        0,
        ARAD_PP_IHB_TCAM_DATA_WIDTH
    },
    {
        "uint32",
         "value",
         0,
         ARAD_PP_IHB_TCAM_DATA_WIDTH
    },
    {
        "uint32",
        "valid",
        0,
        0
    },
    { NULL }
};

static void*
__cint_maddr__ARAD_PP_IHB_TCAM_BANK_TBL_DATA(void* p, int mnum, cint_struct_type_t* parent)
{
    void* rv;
    ARAD_PP_IHB_TCAM_BANK_TBL_DATA* s = (ARAD_PP_IHB_TCAM_BANK_TBL_DATA*) p;

    switch(mnum)
    {
    case 0: rv = &(s->mask); break;
    case 1: rv = &(s->value); break;
    case 2: rv = &(s->valid); break;
    default: rv = NULL; break;
    }

    return rv;
}

static cint_parameter_desc_t __cint_struct_members__ARAD_PP_IHB_TCAM_COMPARE_DATA[] =
{
    {
        "uint32",
        "found",
        0,
        0
    },
    {
        "uint32",
        "address",
        0,
        0
    },
    {NULL}
};

static void*
__cint_maddr__ARAD_PP_IHB_TCAM_COMPARE_DATA(void* p, int mnum, cint_struct_type_t* parent)
{
    void* rv;
    ARAD_PP_IHB_TCAM_COMPARE_DATA* s = (ARAD_PP_IHB_TCAM_COMPARE_DATA*) p;

    switch(mnum)
    {
    case 0: rv = &(s->found); break;
    case 1: rv = &(s->address); break;
    default: rv = NULL; break;
    }

    return rv;
}
static cint_parameter_desc_t __cint_struct_members__combo28_drc_pll_t[] =
{
    {
        "uint32",
        "iso_in",
        0,
        0
    },
    {
        "uint32",
        "cp",
        0,
        0
    },
    {
        "uint32",
        "cp1",
        0,
        0
    },
    {
        "uint32",
        "cz",
        0,
        0
    },
    {
        "uint32",
        "icp",
        0,
        0
    },
    {
        "uint32",
        "rp",
        0,
        0
    },    
    {
        "uint32",
        "rz",
        0,
        0
    },
    {
        "uint32",
        "ldo_ctrl",
        0,
        0
    },
    {
        "uint32",
        "msc_ctrl",
        0,
        0
    },
    {
        "uint32",
        "ndiv_frac",
        0,
        0
    },
    {
        "uint32",
        "ndiv_int",
        0,
        0
    },
    {
        "uint32",
        "pdiv",
        0,
        0
    },
    {
        "uint32",
        "ssc_limit",
        0,
        0
    },
    {
        "uint32",
        "ssc_mode",
        0,
        0
    },
    {
        "uint32",
        "ssc_step",
        0,
        0
    },
    {
        "uint32",
        "vco_gain",
        0,
        0
    },
    { NULL }
};

static cint_parameter_desc_t __cint_struct_members__combo28_vendor_info_t[] =
{
    {
        "uint32",
        "dram_density",
        0,
        0
    },
    {
        "uint32",
        "fifo_depth",
        0,
        0
    },
    {
        "uint32",
        "revision_id",
        0,
        0
    },
    {
        "uint32",
        "manufacture_id",
        0,
        0
    },
    { NULL }
};


static cint_parameter_desc_t __cint_struct_members__combo28_bist_info_t[] =
{
    {
        "uint32",
        "write_weight",
        0,
        0
    },
    {
        "uint32",
        "read_weight",
        0,
        0
    },
    {
        "uint32",
        "bist_timer_us",
        0,
        0
    },
    {
        "uint32",
        "bist_num_actions",
        0,
        0
    },
    {
        "uint32",
        "bist_start_address",
        0,
        0
    },
    {
        "uint32",
        "bist_end_address",
        0,
        0
    },
    {
        "int",
        "mpr_mode",
        0,
        0
    },
    { NULL }
};



static cint_parameter_desc_t __cint_struct_members__combo28_gddr5_bist_info_t[] =
{
    {
        "uint32",
        "fifo_depth",
        0,
        0
    },    
    {
        "int",
        "bist_mode",
        0,
        0
    },
    {
        "uint32*",
        "data_pattern",
        1,
        0
    },  
    {
        "uint8*",
        "dbi_pattern",
        1,
        0
    }, 
    {
        "uint8*",
        "edd_pattern",
        1,
        0
    }, 
    { NULL }
};

static cint_parameter_desc_t __cint_struct_members__combo28_bist_err_cnt_t[] =
{
    {
        "uint32",
        "uint32 bist_err_occur",
        0,
        0
    },
    {
        "uint32",
        "uint32 bist_full_err_cnt",
        0,
        0
    },
    {
        "uint32",
        "uint32 bist_single_err_cnt",
        0,
        0
    },
    {
        "uint32",
        "uint32 bist_global_err_cnt",
        0,
        0
    },
    { NULL }
};


static cint_parameter_desc_t __cint_struct_members__combo28_gddr5_bist_err_cnt_t[] =
{
    {
        "uint32",
        "uint32 bist_data_err_occur",
        0,
        0
    },
    {
        "uint32",
        "uint32 bist_dbi_err_occur",
        0,
        0
    },
    {
        "uint32",
        "uint32 bist_edc_err_occur",
        0,
        0
    },
    {
        "uint32",
        "uint32 bist_adt_err_occur",
        0,
        0
    },
    { NULL }
};


static void*
__cint_maddr__combo28_drc_pll_t(void* p, int mnum, cint_struct_type_t* parent)
{
    void* rv;
    combo28_drc_pll_t* s = (combo28_drc_pll_t*) p;

    switch(mnum)
    {
    case 0: rv = &(s->iso_in); break;
    case 1: rv = &(s->cp); break;
    case 2: rv = &(s->cp1); break;
    case 3: rv = &(s->cz); break;
    case 4: rv = &(s->icp); break;
    case 5: rv = &(s->rp); break;
    case 6: rv = &(s->rz); break;
    case 7: rv = &(s->ldo_ctrl); break;
    case 8: rv = &(s->msc_ctrl); break;
    case 9: rv = &(s->ndiv_frac); break;
    case 10: rv = &(s->ndiv_int); break;
    case 11: rv = &(s->pdiv); break;
    case 12: rv = &(s->ssc_limit); break;
    case 13: rv = &(s->ssc_mode); break;
    case 14: rv = &(s->ssc_step); break;
    case 15: rv = &(s->vco_gain); break;
    default: rv = NULL; break;
    }

    return rv;
}
static void*
__cint_maddr__combo28_vendor_info_t(void* p, int mnum, cint_struct_type_t* parent)
{
    void* rv;
    combo28_vendor_info_t* s = (combo28_vendor_info_t*) p;

    switch(mnum)
    {
    case 0: rv = &(s->dram_density); break;
    case 1: rv = &(s->fifo_depth); break;
    case 2: rv = &(s->revision_id); break;
    case 3: rv = &(s->manufacture_id); break;
    default: rv = NULL; break;
    }

    return rv;
}

static void*
__cint_maddr__combo28_bist_info_t(void* p, int mnum, cint_struct_type_t* parent)
{
    void* rv;
    combo28_bist_info_t* s = (combo28_bist_info_t*) p;

    switch(mnum)
    {
    case 0: rv = &(s->write_weight); break;
    case 1: rv = &(s->read_weight); break;
    case 2: rv = &(s->bist_timer_us); break;
    case 3: rv = &(s->bist_num_actions); break;
    case 4: rv = &(s->bist_start_address); break;
    case 5: rv = &(s->bist_end_address); break;
    case 6: rv = &(s->mpr_mode); break;
    default: rv = NULL; break;
    }

    return rv;
}


static void*
__cint_maddr__combo28_gddr5_bist_info_t(void* p, int mnum, cint_struct_type_t* parent)
{
    void* rv;
    combo28_gddr5_bist_info_t* s = (combo28_gddr5_bist_info_t*) p;

    switch(mnum)
    {
    case 0: rv = &(s->fifo_depth); break;
    case 1: rv = &(s->bist_mode); break;
    case 2: rv = &(s->data_pattern); break;
    case 3: rv = &(s->dbi_pattern); break;
    case 4: rv = &(s->edc_pattern); break;
    default: rv = NULL; break;
    }

    return rv;
}

static void*
__cint_maddr__combo28_bist_err_cnt_t(void* p, int mnum, cint_struct_type_t* parent)
{
    void* rv;
    combo28_bist_err_cnt_t* s = (combo28_bist_err_cnt_t*) p;

    switch(mnum)
    {
    case 0: rv = &(s->bist_err_occur); break;
    case 1: rv = &(s->bist_full_err_cnt); break;
    case 2: rv = &(s->bist_single_err_cnt); break;
    case 3: rv = &(s->bist_global_err_cnt); break;
    default: rv = NULL; break;
    }

    return rv;
}


static void*
__cint_maddr__combo28_gddr5_bist_err_cnt_t(void* p, int mnum, cint_struct_type_t* parent)
{
    void* rv;
    combo28_gddr5_bist_err_cnt_t* s = (combo28_gddr5_bist_err_cnt_t*) p;

    switch(mnum)
    {
    case 0: rv = &(s->bist_data_err_occur); break;
    case 1: rv = &(s->bist_dbi_err_occur); break;
    case 2: rv = &(s->bist_edc_err_occur); break;
    case 3: rv = &(s->bist_adt_err_occur); break;
    default: rv = NULL; break;
    }

    return rv;
}


#endif
#endif /* BCM_PETRA_SUPPORT */
static cint_parameter_desc_t __cint_struct_members__soc_interrupt_cause_t[] =
{
    {
        "int",
        "id",
        0,
        0
    },
    {
        "int",
        "index",
        0,
        0
    },
    { NULL }
};

static cint_parameter_desc_t __cint_struct_members__soc_interrupt_db_t[] =
{
#if !defined(SOC_NO_NAMES)
    {
        "char",
        "name",
        1,
        0
    },
#endif
    {
        "int",
        "id",
        0,
        0
    },
    {
        "int",
        "reg",
        0,
        0
    },
    {
        "int",
        "reg_index",
        0,
        0
    },
    {
        "int",
        "field",
        0,
        0
    },
    {
        "int",
        "mask_reg",
        0,
        0
    },
    {
        "int",
        "mask_reg_index",
        0,
        0
    },
    {
        "int",
        "mask_field",
        0,
        0
    },
    {
        "int",
        "vector_id",
        0,
        0
    },
    {
        "soc_interrupt_tree_t",
        "vector_info",
        1,
        0
    },
    {
        "soc_block_type_t",
        "block_type",
        0,
        0
    },
    {
        "clear_func",
        "interrupt_clear",
        0,
        0
    },
    {
        "void",
        "interrupt_clear_param1",
        1,
        0
    },
    {
        "void",
        "interrupt_clear_param2",
        1,
        0
    },
    {
        "uint32",
        "statistics_count",
        1,
        0
    },
    {
        "uint32",
        "storm_detection_occurrences",
        1,
        0
    },
    {
        "uint32",
        "storm_detection_start_time",
        1,
        0
    },
    {
        "uint32",
        "storm_nominal_ct",
        1,
        0
    },
    {
        "int",
        "bit_in_field",
        0,
        0
    },
    { NULL }
};

static void*
__cint_maddr__soc_interrupt_cause_t(void* p, int mnum, cint_struct_type_t* parent)
{
    void* rv;
    soc_interrupt_cause_t* s = (soc_interrupt_cause_t*) p;

    switch(mnum)
    {
        case 0: rv = &(s->id); break;
        case 1: rv = &(s->index); break;
        default: rv = NULL; break;
    }

    return rv;
}

static void*
__cint_maddr__soc_interrupt_db_t(void* p, int mnum, cint_struct_type_t* parent)
{
    void* rv;
    soc_interrupt_db_t* s = (soc_interrupt_db_t*) p;

    switch(mnum)
    {
#if !defined(SOC_NO_NAMES)
        case 0: rv = &(s->name); break;
        case 1: rv = &(s->id); break;
        case 2: rv = &(s->reg); break;
        case 3: rv = &(s->reg_index); break;
        case 4: rv = &(s->field); break;
        case 5: rv = &(s->mask_reg); break;
        case 6: rv = &(s->mask_reg_index); break;
        case 7: rv = &(s->mask_field); break;
        case 8: rv = &(s->vector_id); break;
        case 9: rv = &(s->vector_info); break;
        case 10: rv = &(s->block_type); break;
        case 11: rv = &(s->interrupt_clear); break;
        case 12: rv = &(s->interrupt_clear_param1); break;
        case 13: rv = &(s->interrupt_clear_param2); break;
        case 14: rv = &(s->statistics_count); break;
        case 15: rv = &(s->storm_detection_occurrences); break;
        case 16: rv = &(s->storm_detection_start_time); break;
        case 17: rv = &(s->storm_nominal_count); break;
        case 18: rv = &(s->bit_in_field); break;
#else
        case 0: rv = &(s->id); break;
        case 1: rv = &(s->reg); break;
        case 2: rv = &(s->reg_index); break;
        case 3: rv = &(s->field); break;
        case 4: rv = &(s->mask_reg); break;
        case 5: rv = &(s->mask_reg_index); break;
        case 6: rv = &(s->mask_field); break;
        case 7: rv = &(s->vector_id); break;
        case 8: rv = &(s->vector_info); break;
        case 9: rv = &(s->block_type); break;
        case 10: rv = &(s->interrupt_clear); break;
        case 11: rv = &(s->interrupt_clear_param1); break;
        case 12: rv = &(s->interrupt_clear_param2); break;
        case 13: rv = &(s->statistics_count); break;
        case 14: rv = &(s->storm_detection_occurrences); break;
        case 15: rv = &(s->storm_detection_start_time); break;
        case 16: rv = &(s->storm_nominal_count); break;
        case 17: rv = &(s->bit_in_field); break;
#endif
        default: rv = NULL; break;
    }

    return rv;
}

#if defined(BCM_PETRA_SUPPORT) && defined(INCLUDE_KBP) && !defined(BCM_88030)
static cint_parameter_desc_t __cint_struct_members__arad_kbp_lut_data_t[] =
{
    {
        "uint32",
        "rec_size",
        0,
        0
    },
    {
        "uint32",
        "rec_type",
        0,
        0
    },
    {
        "uint32",
        "rec_is_valid",
        0,
        0
    },
    {
        "uint32",
        "mode",
        0,
        0
    },
    {
        "uint32",
        "key_config",
        0,
        0
    },
    {
        "uint32",
        "lut_key_data",
        0,
        0
    },
    {
        "uint32",
        "instr",
        0,
        0
    },
    {
        "uint32",
        "key_w_cpd_gt_80",
        0,
        0
    },
    {
        "uint32",
        "copy_data_cfg",
        0,
        0
    },
    {
        "uint32",
        "result0_idx_ad_cfg",
        0,
        0
    },
    {
        "uint32",
        "result0_idx_or_ad",
        0,
        0
    },
   {
        "uint32",
        "result1_idx_ad_cfg",
        0,
        0
    },
    {
        "uint32",
        "result1_idx_or_ad",
        0,
        0
    },
   {
        "uint32",
        "result2_idx_ad_cfg",
        0,
        0
    },
    {
        "uint32",
        "result2_idx_or_ad",
        0,
        0
    },
 {
        "uint32",
        "result3_idx_ad_cfg",
        0,
        0
    },
    {
        "uint32",
        "result3_idx_or_ad",
        0,
        0
    },
   {
        "uint32",
        "result4_idx_ad_cfg",
        0,
        0
    },
    {
        "uint32",
        "result4_idx_or_ad",
        0,
        0
    },
 {
        "uint32",
        "result5_idx_ad_cfg",
        0,
        0
    },
    {
        "uint32",
        "result5_idx_or_ad",
        0,
        0
    },
    { NULL }
};

static void*
__cint_maddr__arad_kbp_lut_data_t(void* p, int mnum, cint_struct_type_t* parent)
{
    void* rv;
    arad_kbp_lut_data_t* s = (arad_kbp_lut_data_t*) p;

    switch(mnum)
    {
        case 0: rv = &(s->rec_size); break;
        case 1: rv = &(s->rec_type); break;
        case 2: rv = &(s->rec_is_valid); break;
        case 3: rv = &(s->mode); break;
        case 4: rv = &(s->key_config); break;
        case 5: rv = &(s->lut_key_data); break;
        case 6: rv = &(s->instr); break;
        case 7: rv = &(s->key_w_cpd_gt_80); break;
        case 8: rv = &(s->copy_data_cfg); break;
        case 9: rv = &(s->result0_idx_ad_cfg); break;
        case 10: rv = &(s->result0_idx_or_ad); break;
        case 11: rv = &(s->result1_idx_ad_cfg); break;
        case 12: rv = &(s->result1_idx_or_ad); break;
        case 13: rv = &(s->result2_idx_ad_cfg); break;
        case 14: rv = &(s->result2_idx_or_ad); break;
        case 15: rv = &(s->result3_idx_ad_cfg); break;
        case 16: rv = &(s->result3_idx_or_ad); break;
        case 17: rv = &(s->result4_idx_ad_cfg); break;
        case 18: rv = &(s->result4_idx_or_ad); break;
        case 19: rv = &(s->result5_idx_ad_cfg); break;
        case 20: rv = &(s->result5_idx_or_ad); break;
        default: rv = NULL; break;
    }

    return rv;
}

static cint_enum_map_t __cint_enum_map__NlmAradWriteMode[] =
{
    { "NLM_ARAD_WRITE_MODE_DATABASE_DM", NLM_ARAD_WRITE_MODE_DATABASE_DM },
    { "NLM_ARAD_WRITE_MODE_DATABASE_XY", NLM_ARAD_WRITE_MODE_DATABASE_XY },
    { NULL }
};

static cint_parameter_desc_t __cint_struct_members__arad_kbp_rop_write_t[] =
{
    {
        "uint8",
        "addr",
        0,
        NLMDEV_REG_ADDR_LEN_IN_BYTES
    },
    {
        "uint8",
        "data",
        0,
        NLM_DATA_WIDTH_BYTES
    },
    {
        "uint8",
        "mask",
        0,
        NLM_DATA_WIDTH_BYTES
    },
    {
        "uint8",
        "addr_short",
        0,
        NLMDEV_REG_ADDR_LEN_IN_BYTES
    },
    {
        "uint8",
        "vBit",
        0,
        0
    },
    {
        "NlmAradWriteMode",
        "writeMode",
        0,
        0
    },
    { NULL }
};

static void*
__cint_maddr__arad_kbp_rop_write_t(void* p, int mnum, cint_struct_type_t* parent)
{
    void* rv;
    arad_kbp_rop_write_t* s = (arad_kbp_rop_write_t*) p;

    switch(mnum)
    {
        case 0: rv = &(s->addr); break;
        case 1: rv = &(s->data); break;
        case 2: rv = &(s->mask); break;
        case 3: rv = &(s->addr_short); break;
        case 4: rv = &(s->vBit); break;
        case 5: rv = &(s->writeMode); break;
        default: rv = NULL; break;
    }

    return rv;
}

static cint_enum_map_t __cint_enum_map__NlmAradReadMode[] =
{
    { "NLM_ARAD_READ_MODE_DATA_X", NLM_ARAD_READ_MODE_DATA_X },
    { "NLM_ARAD_READ_MODE_DATA_Y", NLM_ARAD_READ_MODE_DATA_Y },
    { NULL }
};

static cint_parameter_desc_t __cint_struct_members__arad_kbp_rop_read_t[] =
{
    {
        "uint8",
        "addr",
        0,
        NLMDEV_REG_ADDR_LEN_IN_BYTES
    },
    {
        "uint8",
        "vBit",
        0,
        0
    },
    {
        "NlmAradReadMode",
        "dataType",
        0,
        0
    },
    {
        "uint8",
        "data",
        0,
        NLM_DATA_WIDTH_BYTES + 1
    },
    { NULL }
};



static void*
__cint_maddr__arad_kbp_rop_read_t(void* p, int mnum, cint_struct_type_t* parent)
{
    void* rv;
    arad_kbp_rop_read_t* s = (arad_kbp_rop_read_t*) p;

    switch(mnum)
    {
        case 0: rv = &(s->addr); break;
        case 1: rv = &(s->vBit); break;
        case 2: rv = &(s->dataType); break;
        case 3: rv = &(s->data); break;
        default: rv = NULL; break;
    }

    return rv;
}
#endif /* #if defined(BCM_PETRA_SUPPORT) && defined(INCLUDE_KBP) */
static cint_struct_type_t __cint_soc_structures[] =
{
    {
    "soc_interrupt_cause_t",
    sizeof(soc_interrupt_cause_t),
    __cint_struct_members__soc_interrupt_cause_t,
    __cint_maddr__soc_interrupt_cause_t
    },
      {
    "soc_interrupt_db_t",
    sizeof(soc_interrupt_db_t),
    __cint_struct_members__soc_interrupt_db_t,
    __cint_maddr__soc_interrupt_db_t
    },
#ifdef BCM_PETRA_SUPPORT
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    {
    "arad_kbp_lut_data_t",
    sizeof(arad_kbp_lut_data_t),
    __cint_struct_members__arad_kbp_lut_data_t,
    __cint_maddr__arad_kbp_lut_data_t
    },
    {
    "arad_kbp_rop_write_t",
    sizeof(arad_kbp_rop_write_t),
    __cint_struct_members__arad_kbp_rop_write_t,
    __cint_maddr__arad_kbp_rop_write_t
    },
    {
    "arad_kbp_rop_read_t",
    sizeof(arad_kbp_rop_read_t),
    __cint_struct_members__arad_kbp_rop_read_t,
    __cint_maddr__arad_kbp_rop_read_t
    },
#endif
#ifdef TEST_TCAM
    {
        "ARAD_PP_IHB_TCAM_BANK_TBL_DATA",
            sizeof(ARAD_PP_IHB_TCAM_BANK_TBL_DATA),
            __cint_struct_members__ARAD_PP_IHB_TCAM_BANK_TBL_DATA,
            __cint_maddr__ARAD_PP_IHB_TCAM_BANK_TBL_DATA
    },
    {
        "ARAD_PP_IHB_TCAM_COMPARE_DATA",
        sizeof(ARAD_PP_IHB_TCAM_COMPARE_DATA),
        __cint_struct_members__ARAD_PP_IHB_TCAM_COMPARE_DATA,
        __cint_maddr__ARAD_PP_IHB_TCAM_COMPARE_DATA
    },
#endif
    {
        "combo28_drc_pll_t",
        sizeof(combo28_drc_pll_t),
        __cint_struct_members__combo28_drc_pll_t,
        __cint_maddr__combo28_drc_pll_t
    },
    {
        "combo28_vendor_info_t",
        sizeof(combo28_vendor_info_t),
        __cint_struct_members__combo28_vendor_info_t,
        __cint_maddr__combo28_vendor_info_t
    },
    {
        "combo28_bist_info_t",
        sizeof(combo28_bist_info_t),
        __cint_struct_members__combo28_bist_info_t,
        __cint_maddr__combo28_bist_info_t
    },
    {
        "combo28_gddr5_bist_info_t",
        sizeof(combo28_gddr5_bist_info_t),
        __cint_struct_members__combo28_gddr5_bist_info_t,
        __cint_maddr__combo28_gddr5_bist_info_t
    },
    {
        "combo28_bist_err_cnt_t",
        sizeof(combo28_bist_err_cnt_t),
        __cint_struct_members__combo28_bist_err_cnt_t,
        __cint_maddr__combo28_bist_err_cnt_t
    },   
    {
        "combo28_gddr5_bist_err_cnt_t",
        sizeof(combo28_gddr5_bist_err_cnt_t),
        __cint_struct_members__combo28_gddr5_bist_err_cnt_t,
        __cint_maddr__combo28_gddr5_bist_err_cnt_t
    },   
#endif /* BCM_PETRA_SUPPORT */
#if defined(BCM_CALADAN3_SUPPORT)
    {
        "soc_c3_rce_qual_desc_t",
        sizeof(soc_c3_rce_qual_desc_t),
        __cint_struct_members__soc_c3_rce_qual_desc_t,
        __cint_maddr__soc_c3_rce_qual_desc_t
    },
    {
        "soc_c3_rce_action_desc_t",
        sizeof(soc_c3_rce_action_desc_t),
        __cint_struct_members__soc_c3_rce_action_desc_t,
        __cint_maddr__soc_c3_rce_action_desc_t
    },
    {
        "soc_c3_rce_group_desc_t",
        sizeof(soc_c3_rce_group_desc_t),
        __cint_struct_members__soc_c3_rce_group_desc_t,
        __cint_maddr__soc_c3_rce_group_desc_t
    },
    {
        "soc_c3_rce_entry_desc_t",
        sizeof(soc_c3_rce_entry_desc_t),
        __cint_struct_members__soc_c3_rce_entry_desc_t,
        __cint_maddr__soc_c3_rce_entry_desc_t
    },
    {
        "soc_c3_rce_header_field_info_t",
        sizeof(soc_c3_rce_header_field_info_t),
        __cint_struct_members__soc_c3_rce_header_field_info_t,
        __cint_maddr__soc_c3_rce_header_field_info_t
    },
    {
        "soc_c3_rce_start_len_t",
        sizeof(soc_c3_rce_start_len_t),
        __cint_struct_members__soc_c3_rce_start_len_t,
        __cint_maddr__soc_c3_rce_start_len_t
    },
    {
        "soc_c3_rce_qual_uc_desc_t",
        sizeof(soc_c3_rce_qual_uc_desc_t),
        __cint_struct_members__soc_c3_rce_qual_uc_desc_t,
        __cint_maddr__soc_c3_rce_qual_uc_desc_t
    },
    {
        "soc_c3_rce_action_uc_desc_t",
        sizeof(soc_c3_rce_action_uc_desc_t),
        __cint_struct_members__soc_c3_rce_action_uc_desc_t,
        __cint_maddr__soc_c3_rce_action_uc_desc_t
    },
    {
        "soc_c3_rce_program_desc_t",
        sizeof(soc_c3_rce_program_desc_t),
        __cint_struct_members__soc_c3_rce_program_desc_t,
        __cint_maddr__soc_c3_rce_program_desc_t
    },
    {
        "soc_c3_rce_actiontable_desc_t",
        sizeof(soc_c3_rce_actiontable_desc_t),
        __cint_struct_members__soc_c3_rce_actiontable_desc_t,
        __cint_maddr__soc_c3_rce_actiontable_desc_t
    },
    {
        "soc_c3_rce_unit_desc_t",
        sizeof(soc_c3_rce_unit_desc_t),
        __cint_struct_members__soc_c3_rce_unit_desc_t,
        __cint_maddr__soc_c3_rce_unit_desc_t
    },
    {
        "soc_c3_rce_range_desc_t",
        sizeof(soc_c3_rce_range_desc_t),
        __cint_struct_members__soc_c3_rce_range_desc_t,
        __cint_maddr__soc_c3_rce_range_desc_t
    },

#endif /* defined(BCM_CALADAN3_SUPPORT) */
    { NULL }
};

static cint_enum_type_t __cint_soc_enums[] =
{
#if defined(BCM_PETRA_SUPPORT) && defined(INCLUDE_KBP) && !defined(BCM_88030)
    { "NlmAradWriteMode", __cint_enum_map__NlmAradWriteMode },
    { "NlmAradReadMode", __cint_enum_map__NlmAradReadMode },
#endif
#if defined(BCM_CALADAN3_SUPPORT)
    { "soc_c3_rce_qual_type_t", __cint_enum_map__soc_c3_rce_qual_type_t },
    { "soc_c3_rce_action_type_t", __cint_enum_map__soc_c3_rce_action_type_t },
    { "soc_c3_rce_data_header_t", __cint_enum_map__soc_c3_rce_data_header_t },
    { "soc_c3_rce_metadata_type_t", __cint_enum_map__soc_c3_rce_metadata_type_t },
    { "soc_c3_rce_action_uc_type_t", __cint_enum_map__soc_c3_rce_action_uc_type_t },
#endif /* defined (BCM_CALADAN3_SUPPORT) */
    CINT_ENTRY_LAST
};

static cint_parameter_desc_t __cint_soc_typedefs[] =
{
    {
        "int",
        "soc_port_t",
        0,
        0
    },
    {NULL}
};

static cint_constants_t __cint_soc_constants[] =
{
#if defined(BCM_CALADAN3_SUPPORT)
    { "SOC_C3_RCE_KEY_BIT_MINIMUM", SOC_C3_RCE_KEY_BIT_MINIMUM },
    { "SOC_C3_RCE_KEY_BIT_MAXIMUM", SOC_C3_RCE_KEY_BIT_MAXIMUM },
    { "SOC_C3_RCE_PROGRAM_COUNT", SOC_C3_RCE_PROGRAM_COUNT },
    { "SOC_C3_RCE_RESULT_REGISTER_COUNT", SOC_C3_RCE_RESULT_REGISTER_COUNT },
    { "SOC_C3_RCE_GROUP_QUAL_MAX", SOC_C3_RCE_GROUP_QUAL_MAX },
    { "SOC_C3_RCE_PROGRAM_ACTION_MAX", SOC_C3_RCE_PROGRAM_ACTION_MAX },
    { "SOC_C3_RCE_MAX_SEG_PER_QUALIFIER", SOC_C3_RCE_MAX_SEG_PER_QUALIFIER },
    { "SOC_C3_RCE_MAX_SEG_PER_ACTION", SOC_C3_RCE_MAX_SEG_PER_ACTION },
    { "SOC_C3_RCE_DUMP_UNIT_INCLUDE_IMEM_ALLOC", SOC_C3_RCE_DUMP_UNIT_INCLUDE_IMEM_ALLOC },
    { "SOC_C3_RCE_DUMP_UNIT_INCLUDE_ACT_ALLOC", SOC_C3_RCE_DUMP_UNIT_INCLUDE_ACT_ALLOC },
    { "SOC_C3_RCE_DUMP_UNIT_INCLUDE_ACT_DFLT", SOC_C3_RCE_DUMP_UNIT_INCLUDE_ACT_DFLT },
    { "SOC_C3_RCE_DUMP_UNIT_INCLUDE_ACT_DATA", SOC_C3_RCE_DUMP_UNIT_INCLUDE_ACT_DATA },
    { "SOC_C3_RCE_DUMP_UNIT_INCLUDE_ACTIONS", SOC_C3_RCE_DUMP_UNIT_INCLUDE_ACTIONS },
    { "SOC_C3_RCE_DUMP_UNIT_INCLUDE_RANGES", SOC_C3_RCE_DUMP_UNIT_INCLUDE_RANGES },
    { "SOC_C3_RCE_DUMP_UNIT_INCLUDE_PADDING", SOC_C3_RCE_DUMP_UNIT_INCLUDE_PADDING },
    { "SOC_C3_RCE_DUMP_UNIT_INCLUDE_PROGRAMS", SOC_C3_RCE_DUMP_UNIT_INCLUDE_PROGRAMS },
    { "SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_KEY", SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_KEY },
    { "SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_FSETS", SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_FSETS },
    { "SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_DISASM", SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_DISASM },
    { "SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_GRP_DATA", SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_GRP_DATA },
    { "SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_GRP_ID", SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_GRP_ID },
    { "SOC_C3_RCE_DUMP_FSET_INCLUDE_RANGES", SOC_C3_RCE_DUMP_FSET_INCLUDE_RANGES },
    { "SOC_C3_RCE_DUMP_FSET_INCLUDE_ENTRIES", SOC_C3_RCE_DUMP_FSET_INCLUDE_ENTRIES },
    { "SOC_C3_RCE_DUMP_GROUP_INCLUDE_DFLTPATT", SOC_C3_RCE_DUMP_GROUP_INCLUDE_DFLTPATT },
    { "SOC_C3_RCE_DUMP_GROUP_INCLUDE_QUALS", SOC_C3_RCE_DUMP_GROUP_INCLUDE_QUALS },
    { "SOC_C3_RCE_DUMP_GROUP_INCLUDE_DISASM", SOC_C3_RCE_DUMP_GROUP_INCLUDE_DISASM },
    { "SOC_C3_RCE_DUMP_GROUP_INCLUDE_ENT_DATA", SOC_C3_RCE_DUMP_GROUP_INCLUDE_ENT_DATA },
    { "SOC_C3_RCE_DUMP_GROUP_INCLUDE_ENT_ID", SOC_C3_RCE_DUMP_GROUP_INCLUDE_ENT_ID },
    { "SOC_C3_RCE_DUMP_ENTRY_INCLUDE_RANGES", SOC_C3_RCE_DUMP_ENTRY_INCLUDE_RANGES },
    { "SOC_C3_RCE_DUMP_ENTRY_INCLUDE_H_ACTS", SOC_C3_RCE_DUMP_ENTRY_INCLUDE_H_ACTS },
    { "SOC_C3_RCE_DUMP_ENTRY_INCLUDE_A_ACTS", SOC_C3_RCE_DUMP_ENTRY_INCLUDE_A_ACTS },
    { "SOC_C3_RCE_DUMP_ENTRY_INCLUDE_H_QUALS", SOC_C3_RCE_DUMP_ENTRY_INCLUDE_H_QUALS },
    { "SOC_C3_RCE_DUMP_ENTRY_INCLUDE_A_QUALS", SOC_C3_RCE_DUMP_ENTRY_INCLUDE_A_QUALS },
    { "SOC_C3_RCE_DUMP_ENTRY_INCLUDE_H_ACTION", SOC_C3_RCE_DUMP_ENTRY_INCLUDE_H_ACTION },
    { "SOC_C3_RCE_DUMP_ENTRY_INCLUDE_A_ACTION", SOC_C3_RCE_DUMP_ENTRY_INCLUDE_A_ACTION },
    { "SOC_C3_RCE_DUMP_ENTRY_INCLUDE_H_PATT", SOC_C3_RCE_DUMP_ENTRY_INCLUDE_H_PATT },
    { "SOC_C3_RCE_DUMP_ENTRY_INCLUDE_A_PATT", SOC_C3_RCE_DUMP_ENTRY_INCLUDE_A_PATT },
    { "SOC_C3_RCE_ENTRY_COPY_SOURCE_QUALS", SOC_C3_RCE_ENTRY_COPY_SOURCE_QUALS },
    { "SOC_C3_RCE_ENTRY_COPY_SOURCE_ACTS", SOC_C3_RCE_ENTRY_COPY_SOURCE_ACTS },
    { "SOC_C3_RCE_ENTRY_COPY_SOURCE_INST", SOC_C3_RCE_ENTRY_COPY_SOURCE_INST },
    { "SOC_C3_RCE_ENTRY_COPY_SOURCE_CTRS", SOC_C3_RCE_ENTRY_COPY_SOURCE_CTRS },
    { "SOC_C3_RCE_ENTRY_COPY_SOURCE_ALL", SOC_C3_RCE_ENTRY_COPY_SOURCE_ALL },
    { "SOC_C3_RCE_ENTRY_FLAG_MODIFIED", SOC_C3_RCE_ENTRY_FLAG_MODIFIED },
    { "SOC_C3_RCE_ENTRY_FLAG_INSTALLED", SOC_C3_RCE_ENTRY_FLAG_INSTALLED },
    { "SOC_C3_RCE_RANGE_FLAG_INVERT", SOC_C3_RCE_RANGE_FLAG_INVERT },
    { "SOC_C3_RCE_RESULT_TRACE_ENABLE", SOC_C3_RCE_RESULT_TRACE_ENABLE },
    { "SOC_C3_RCE_RESULT_TRACE_FROM_KEY", SOC_C3_RCE_RESULT_TRACE_FROM_KEY },
    { "SOC_C3_RCE_RESULT_TRACE_CAPTURED", SOC_C3_RCE_RESULT_TRACE_CAPTURED },
    { "SOC_C3_RCE_KEY_TRACE_ENABLE", SOC_C3_RCE_KEY_TRACE_ENABLE },
    { "SOC_C3_RCE_KEY_TRACE_INTERRUPT", SOC_C3_RCE_KEY_TRACE_INTERRUPT },
    { "SOC_C3_RCE_KEY_TRACE_HALT", SOC_C3_RCE_KEY_TRACE_HALT },
    { "SOC_C3_RCE_KEY_TRACE_CAPTURED", SOC_C3_RCE_KEY_TRACE_CAPTURED },
    { "SOC_C3_RCE_RESULT_HIT_COUNTER_COUNT", SOC_C3_RCE_RESULT_HIT_COUNTER_COUNT },
    { "SOC_C3_RCE_RESULT_HIT_ANY_COUNTER", SOC_C3_RCE_RESULT_HIT_ANY_COUNTER },
    { "SOC_C3_RCE_RESULT_MISS_COUNTER", SOC_C3_RCE_RESULT_MISS_COUNTER },
    { "SOC_C3_RCE_STATE_CHECK_ABORT_ERRORS", SOC_C3_RCE_STATE_CHECK_ABORT_ERRORS },
    { "SOC_C3_RCE_STATE_CHECK_ABORT_MISMATCH", SOC_C3_RCE_STATE_CHECK_ABORT_MISMATCH },
    { "SOC_C3_RCE_STATE_CHECK_ABORT_INVALID", SOC_C3_RCE_STATE_CHECK_ABORT_INVALID },
    { "SOC_C3_RCE_STATE_CHECK_ABORT_CORRUPT", SOC_C3_RCE_STATE_CHECK_ABORT_CORRUPT },
#endif /* defined (BCM_CALADAN3_SUPPORT) */
   CINT_ENTRY_LAST
};

cint_data_t soc_cint_data =
    {
        NULL,
        __cint_soc_functions,
        __cint_soc_structures,
        __cint_soc_enums,
        __cint_soc_typedefs,
        __cint_soc_constants,
        __cint_soc_function_pointers
    };

#endif /* INCLUDE_LIB_CINT */



