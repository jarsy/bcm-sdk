/*
 * $Id: caladan3_cmds.c,v 1.52.10.25 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        caladan3_cmds.c
 * Purpose:     Caladan3-specific diagnostic shell commands
 * Requires:
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <sal/types.h>
#include <sal/appl/sal.h>
#include <sal/appl/io.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <appl/diag/system.h>
#include <appl/diag/shell.h>
#include <appl/diag/parse.h>
#include <appl/diag/sbx/caladan3_cmds.h>
#include <appl/diag/sbx/register.h>
#include <soc/sbx/caladan3.h>
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/caladan3/port.h>
#include <soc/sbx/caladan3/ppe.h>
#include <soc/sbx/caladan3/ped.h>
#include <soc/sbx/caladan3/sws.h>
#include <soc/sbx/caladan3/tmu/tmu.h>
#include <soc/sbx/caladan3/tmu/taps/taps.h>
#include <soc/sbx/caladan3/ucodemgr.h>
#include <soc/sbx/caladan3/lrp.h>
#include <soc/sbx/caladan3/ppe.h>
#include <soc/sbx/caladan3/asm3/debug.h>
#include <soc/sbx/caladan3/asm3/asm3_pkg_intf.h>
#include <soc/sbx/caladan3/etu.h>
#include <soc/sbx/caladan3/etu_tcam.h>
#include <soc/sbx/caladan3/etu_xpt.h>
#include <soc/sbx/caladan3/sws.h>
#include <soc/sbx/caladan3/util.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/counter.h>
#include <soc/er_tcam.h>
#include <bcm_int/sbx/caladan3/cosq.h>
#ifndef __KERNEL__
#include <stdio.h>
#include <time.h>
#endif

/* Set this to enable calculating and displaying the time required
 * do do the entire fast reconfig
 */
#define CHECK_RECONFIG_TIME 1
#define CMD_SBX_CALADAN3_MAX_POLICER_NUM 65536

typedef struct cmd_sbx_caladan3_meter_monitor_s {
    uint32 attached;
    uint32 monitor_id;
}cmd_sbx_caladan3_meter_monitor_t;

cmd_sbx_caladan3_meter_monitor_t *g_cmd_sbx_caladan3_meter_monitor[SOC_SBX_CALADAN3_COP_NUM_COP][SOC_SBX_CALADAN3_COP_NUM_SEGMENT]; 

#ifdef NLMPLATFORM_BCM
extern int nlmdiag_refapp_main(int argc, char**argv);
extern int nlmdevmgr_mt_refapp_main(int argc, char**argv);
extern int nlmdevmgr_refapp_main(int argc, char**argv);
extern int nlmgtmftm_refapp_main(int argc, char**argv);
extern int nlmrangemgr_refapp_main(int argc, char**argv);
extern int nlmrangemgr_refapp2_main(int argc, char**argv);
extern int nlmrangemgr_refapp3_main(int argc, char**argv);
extern int nlmgenerictblmgr_refapp_main(int argc, char**argv);
extern int nlmfibtblmgr_refapp_main(int argc, char**argv);
#endif



cmd_result_t sbx_caladan3_ocm_dump_allocator(int unit, int port)
{
    int index;
    int ocm_mem_idx;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("only supported on caladan3\n");
        return CMD_FAIL;
    }

    ocm_total_alloc_size_in_bits[unit] = 0;


    for (ocm_mem_idx = 0; ocm_mem_idx < SOC_SBX_CALADAN3_OCM_NUM_MEM; ocm_mem_idx++) {
        ocm_mem_bank_occupation[unit][ocm_mem_idx] = 0;
    }

        
    for(index=SOC_SBX_CALADAN3_OCM_LRP0_PORT;
        index < SOC_SBX_CALADAN3_MAX_OCM_PORT; index++) {
        if (port >= 0 && port != index) {
            continue;
        } 
        soc_sbx_caladan3_ocm_util_allocator_dump(unit,index);
    }
    cli_out("\n\n\n");
    cli_out("========================================================================================\n");
    cli_out("========================================================================================\n");
    cli_out("OCM MEM ALLOCATION TOTALS:\n");

    for(index = 0;
        index < SOC_SBX_CALADAN3_OCM_NUM_MEM;
        index++) {
        float percent_occupied = 0;
        if (ocm_mem_bank_occupation[unit][index] > SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM)
            {
                cli_out("\n\n  FATAL ERROR ENCOUNTERED!  OCM Memory Exhausted of OCM MEM [%d] \n", index);
                continue;
            }
        cli_out("\n\n  OCM MEM [%d]  MEM ALLOC STATS:\n", index);
        percent_occupied = (float)ocm_mem_bank_occupation[unit][index]/(float)SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM;
        percent_occupied *= 100;
        cli_out("    Total Allocation of Physical Blocks:  [%d PBs] Consume [%.1f%%] of Total[%d]   [%d PBs] Available.\n",
                ocm_mem_bank_occupation[unit][index],
                percent_occupied,
                SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM,
                SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM - ocm_mem_bank_occupation[unit][index]);

        cli_out("        %d Mbs (%.1f%%) of Total %d Mbs\n", 
                ocm_mem_bank_occupation[unit][index], 
                percent_occupied, SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM);

    }


    cli_out("\n\n\n\nTOTAL OCM MEM ALLOC SIZE FOR OCM PORTS: [%d kb]  [%d KB]\n", 
            ocm_total_alloc_size_in_bits[unit]/1024,
            ocm_total_alloc_size_in_bits[unit]/1024/8);

    cli_out("========================================================================================\n");


    return CMD_OK;
}


extern int soc_etu_nl_mdio_read(int unit,   
				unsigned mdio_portid,
				unsigned mdio_dev_id, 
				unsigned mdio_addr,
				uint16   *rd_data
				);

extern int
soc_etu_nl_mdio_write(int      unit,   
              unsigned mdio_portid,
              unsigned mdio_dev_id,
              unsigned mdio_addr,
              uint16   wr_data,
              unsigned verify_wr
		      );

char cmd_sbx_caladan3_ocm_allocator_dump_usage[] = "\n"
    " OcmAlloctorDump port=x\n"
    " invalid port numbers (eg.,-1) dumps all ports allocations\n";

cmd_result_t
cmd_sbx_caladan3_ocm_dump_allocator(int unit, args_t *args)
{
    int port = -1;
    parse_table_t pt;
    int ret_code;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("only supported on caladan3\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args)) {
        parse_table_init(0,&pt);
        parse_table_add(&pt,"port",PQ_INT,
                        0, &port, NULL);
        
        if (!parseEndOk(args,&pt,&ret_code)) {
            return ret_code;
        }
    }

    return sbx_caladan3_ocm_dump_allocator(unit, port);
}

char cmd_sbx_caladan3_tmu_allocator_dump_usage[] = "\n"
    " TmuAlloctorDump table=x\n"
    " invalid table numbers (eg.,-1) dumps all tables allocations\n";

cmd_result_t sbx_caladan3_tmu_dump_allocator(int unit, int table)
{
    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("only supported on caladan3\n");
        return CMD_FAIL;
    }

    cli_out("\n\n\n");
    cli_out("========================================================================================\n");
    cli_out("========================================================================================\n");
    cli_out("TMU MEM ALLOCATION TOTALS:\n");

    soc_sbx_caladan3_tmu_table_map_dump(unit, table, table);

    return CMD_OK;
}

cmd_result_t
cmd_sbx_caladan3_tmu_dump_allocator(int unit, args_t *args)
{
    int table = -1;
    parse_table_t pt;
    int ret_code;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("only supported on caladan3\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args)) {
        parse_table_init(0,&pt);
        parse_table_add(&pt,"table_id",PQ_INT,
                        0, &table, NULL);
        
        if (!parseEndOk(args,&pt,&ret_code)) {
            return ret_code;
        }
    }

    return sbx_caladan3_tmu_dump_allocator(unit, table);
}



char cmd_sbx_caladan3_port_map_dump_usage[] = "\n"
    " PortMapDump [V|v|h] [port]\n"
    " Dumps all port mapping information\n";

cmd_result_t
cmd_sbx_caladan3_port_map_dump(int unit, args_t *args)
{
    char *opt;
    int ver = 0, port = -1;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("only supported on caladan3\n");
        return CMD_FAIL;
    }
    if ((opt = ARG_GET(args)) != NULL) {
        /* hotswap info from the bcm layer not committed */
        if (opt[0] == 'h') {
            bcm_c3_cosq_info_dump(unit);
        } else {
            if ((opt[0] == 'v') || (opt[0] == 'V')) {
                ver = 1;
                opt = ARG_GET(args);
            } 
            if (opt != NULL) {
                port = _shr_ctoi(opt);
                if ((port < 0) || (port > 100)) { return CMD_FAIL;}
            }
            soc_sbx_caladan3_port_info_dump(unit, ver, port);
        }
    } else {
        soc_sbx_caladan3_port_info_dump(unit, 0, -1);
    }
    return CMD_OK;
}


cmd_result_t
cmd_sbx_caladan3_ppe_hc(int unit, int on, int clear, int queue, 
                        int str, int var, int varmask)
{ 
    int rv;
    rv = soc_sbx_caladan3_ppe_hc_control(unit, on, clear, 
                                         queue, str, var, varmask);
    if (SOC_SUCCESS(rv)) {
        return CMD_OK;
    } else {
        return CMD_FAIL;
    }
}

cmd_result_t
cmd_sbx_caladan3_pd_hc(int unit, int queue, int on, int drop)
{
    uint32 regval;
    int index;
    int copyreg[] = {PD_HDR_COPY_CHECK0r, PD_HDR_COPY_CHECK1r, PD_HDR_COPY_CHECK2r, 
                     PD_HDR_COPY_CHECK3r, PD_HDR_COPY_CHECK4r, PD_HDR_COPY_CHECK5r, 
                     PD_HDR_COPY_CHECK6r, PD_HDR_COPY_CHECK7r, PD_HDR_COPY_CHECK8r, 
                     PD_HDR_COPY_CHECK9r, PD_HDR_COPY_CHECK10r, PD_HDR_COPY_CHECK11r, 
                     PD_HDR_COPY_CHECK12r, PD_HDR_COPY_CHECK13r, PD_HDR_COPY_CHECK14r, 
                     PD_HDR_COPY_CHECK15r};
    int copymask[] = {PD_HDR_COPY_MASK0r, PD_HDR_COPY_MASK1r, PD_HDR_COPY_MASK2r, 
                     PD_HDR_COPY_MASK3r, PD_HDR_COPY_MASK4r, PD_HDR_COPY_MASK5r, 
                     PD_HDR_COPY_MASK6r, PD_HDR_COPY_MASK7r, PD_HDR_COPY_MASK8r, 
                     PD_HDR_COPY_MASK9r, PD_HDR_COPY_MASK10r, PD_HDR_COPY_MASK11r, 
                     PD_HDR_COPY_MASK12r, PD_HDR_COPY_MASK13r, PD_HDR_COPY_MASK14r, 
                     PD_HDR_COPY_MASK15r};



    /* By default HC bit set packets are always copied */
    /* to copy all packets, set data/mask to match all packets */
    /* If drop only packets are requested, enable pd debug & disable copy of all packets */
    if (drop) { 
        on = 0;
    }
    
    for (index=0; index < 16; index++) {
        if (on) {
            /* clear all mask */
            regval = 0;
        } else {
            /* use 0xffff data & enable all mask */
            regval = 0xffffffff;
        }
        soc_reg32_set(unit, copyreg[index], index, 0, regval);
        soc_reg32_set(unit, copymask[index], index, 0, regval);
    }

    drop = (drop > 0) ? 1:0;
    SOC_IF_ERROR_RETURN(READ_PD_DEBUGr(unit, &regval));
    soc_reg_field_set(unit, PD_DEBUGr, &regval, COPY_DROPf, drop);
    SOC_IF_ERROR_RETURN(WRITE_PD_DEBUGr(unit, regval));

    return CMD_OK;
}

cmd_result_t
cmd_sbx_caladan3_ppe_show(int unit)
{
    uint32 regval;
    uint32 copy_enable_mask, on, queue, str, var, varmask, number;

    SOC_IF_ERROR_RETURN(READ_PP_HDR_COPY_CONTROL0r(unit, &regval));
    copy_enable_mask =
        soc_reg_field_get(unit, PP_HDR_COPY_CONTROL0r, regval, COPY_MASKf);
    on =
        soc_reg_field_get(unit, PP_HDR_COPY_CONTROL0r, regval, COPY_ENABLEf);
    queue =
        soc_reg_field_get(unit, PP_HDR_COPY_CONTROL0r, regval, COPY_SQUEUEf);
    str =
        soc_reg_field_get(unit, PP_HDR_COPY_CONTROL0r, regval, COPY_STREAMf);
    number =
        soc_reg_field_get(unit, PP_HDR_COPY_CONTROL0r, regval, COPY_COUNTf);
    SOC_IF_ERROR_RETURN(READ_PP_HDR_COPY_CONTROL1r(unit, &regval));
    var =
        soc_reg_field_get(unit, PP_HDR_COPY_CONTROL1r, regval, COPY_VARIABLEf);
    SOC_IF_ERROR_RETURN(READ_PP_HDR_COPY_CONTROL2r(unit, &regval));
    varmask =
        soc_reg_field_get(unit, PP_HDR_COPY_CONTROL2r, regval, COPY_VARIABLE_MASKf);
    cli_out("PPE: on=%d, ", on);
    /* Print variable, queue and stream by mask */
    if (!(copy_enable_mask & SOC_SBX_CALADAN3_PPE_HDR_CAPTURE_IGNORE_VARIABLE)) {
    	cli_out("variable=%d, variable_mask=%d, ", var, varmask);
    }
    if (!(copy_enable_mask & SOC_SBX_CALADAN3_PPE_HDR_CAPTURE_IGNORE_SQUEUE)) {
    	cli_out("queue=%d, ", queue);
    }
    if (!(copy_enable_mask & SOC_SBX_CALADAN3_PPE_HDR_CAPTURE_IGNORE_STREAM)) {
    	cli_out("stream=%d, ", str);
    }
    if (number <= 1) {
    	cli_out("(PPE captured %d packet)\n", number);
    }
    else {
    	cli_out("(PPE captured %d packets)\n", number);
    }

    return CMD_OK;
}

cmd_result_t
cmd_sbx_caladan3_ped_show(int unit)
{
    uint32 regval;
    uint32 drop, data, mask, npedin, npedout;
    int index;
    int copyreg[] = {PD_HDR_COPY_CHECK0r, PD_HDR_COPY_CHECK1r, PD_HDR_COPY_CHECK2r,
                     PD_HDR_COPY_CHECK3r, PD_HDR_COPY_CHECK4r, PD_HDR_COPY_CHECK5r,
                     PD_HDR_COPY_CHECK6r, PD_HDR_COPY_CHECK7r, PD_HDR_COPY_CHECK8r,
                     PD_HDR_COPY_CHECK9r, PD_HDR_COPY_CHECK10r, PD_HDR_COPY_CHECK11r,
                     PD_HDR_COPY_CHECK12r, PD_HDR_COPY_CHECK13r, PD_HDR_COPY_CHECK14r,
                     PD_HDR_COPY_CHECK15r};
    int copymask[] = {PD_HDR_COPY_MASK0r, PD_HDR_COPY_MASK1r, PD_HDR_COPY_MASK2r,
                     PD_HDR_COPY_MASK3r, PD_HDR_COPY_MASK4r, PD_HDR_COPY_MASK5r,
                     PD_HDR_COPY_MASK6r, PD_HDR_COPY_MASK7r, PD_HDR_COPY_MASK8r,
                     PD_HDR_COPY_MASK9r, PD_HDR_COPY_MASK10r, PD_HDR_COPY_MASK11r,
                     PD_HDR_COPY_MASK12r, PD_HDR_COPY_MASK13r, PD_HDR_COPY_MASK14r,
                     PD_HDR_COPY_MASK15r};

    /* Read copy drop */
    SOC_IF_ERROR_RETURN(READ_PD_DEBUGr(unit, &regval));
    drop =
        soc_reg_field_get(unit, PD_DEBUGr, regval, COPY_DROPf);
    SOC_IF_ERROR_RETURN(READ_PD_COPY_BUF_LEVELr(unit, &regval));
    npedin = soc_reg_field_get(unit, PD_COPY_BUF_LEVELr,
                            regval, CI_LVLf);
    npedout = soc_reg_field_get(unit, PD_COPY_BUF_LEVELr,
                            regval, CO_LVLf);

    cli_out("PED: drop=%d, ", drop);
    if ((npedin/2) <= 1) {
    	cli_out("(PEDIn captured %d packet, PEDOut captured %d packet)\n", npedin/2, npedout/2);
    }
    else {
    	cli_out("(PEDIn captured %d packets, PEDOut captured %d packets)\n", npedin/2, npedout/2);
    }

    /* Read data & mask */
    for (index=0; index < 16; index++) {
        soc_reg32_get(unit, copyreg[15-index], index, 0, &regval);
        data = regval;
        soc_reg32_get(unit, copymask[15-index], index, 0, &regval);
        mask = regval;
        cli_out("\tWORD%02d:(0x%08x/0x%08x)", index, data, mask);
        if((index % 4) == 3) {
        	cli_out("\n");
        }
    }

    return CMD_OK;
}

cmd_result_t
cmd_sbx_caladan3_ped_check(int unit, int word, int data, int data_mask)
{
    int copyreg[] = {PD_HDR_COPY_CHECK0r, PD_HDR_COPY_CHECK1r, PD_HDR_COPY_CHECK2r,
                     PD_HDR_COPY_CHECK3r, PD_HDR_COPY_CHECK4r, PD_HDR_COPY_CHECK5r,
                     PD_HDR_COPY_CHECK6r, PD_HDR_COPY_CHECK7r, PD_HDR_COPY_CHECK8r,
                     PD_HDR_COPY_CHECK9r, PD_HDR_COPY_CHECK10r, PD_HDR_COPY_CHECK11r,
                     PD_HDR_COPY_CHECK12r, PD_HDR_COPY_CHECK13r, PD_HDR_COPY_CHECK14r,
                     PD_HDR_COPY_CHECK15r};
    int copymask[] = {PD_HDR_COPY_MASK0r, PD_HDR_COPY_MASK1r, PD_HDR_COPY_MASK2r,
                     PD_HDR_COPY_MASK3r, PD_HDR_COPY_MASK4r, PD_HDR_COPY_MASK5r,
                     PD_HDR_COPY_MASK6r, PD_HDR_COPY_MASK7r, PD_HDR_COPY_MASK8r,
                     PD_HDR_COPY_MASK9r, PD_HDR_COPY_MASK10r, PD_HDR_COPY_MASK11r,
                     PD_HDR_COPY_MASK12r, PD_HDR_COPY_MASK13r, PD_HDR_COPY_MASK14r,
                     PD_HDR_COPY_MASK15r};

    if ((word < 0) || word > 15) {
    	return CMD_USAGE;
    }

    soc_reg32_set(unit, copyreg[15-word], word, 0, data);
    soc_reg32_set(unit, copymask[15-word], word, 0, data_mask);

    return CMD_OK;
}

cmd_result_t
cmd_sbx_caladan3_ped_clear(int unit)
{
    int npedin, npedout;
    uint32 regval = 0;

    READ_PD_COPY_BUF_LEVELr(unit, &regval);
    npedin = soc_reg_field_get(unit, PD_COPY_BUF_LEVELr,
                            regval, CI_LVLf);
    npedout = soc_reg_field_get(unit, PD_COPY_BUF_LEVELr,
                            regval, CO_LVLf);

    regval = 0;
    soc_reg_field_set(unit, PD_COPY_BUF_CTRLr, &regval, POP_INf, 1);
    while(npedin-- > 0) {
       WRITE_PD_COPY_BUF_CTRLr(unit, regval);
    }
    regval = 0;
    soc_reg_field_set(unit, PD_COPY_BUF_CTRLr, &regval, POP_OUTf, 1);
    while(npedout-- > 0) {
       WRITE_PD_COPY_BUF_CTRLr(unit, regval);
    }

    return SOC_E_NONE;
}

char cmd_sbx_caladan3_region_map_dump_usage[] = "\n"
    " RegionMapDump <min> <max> \n"
    " Dump TMU region to DRAM mapping\n";

cmd_result_t
cmd_sbx_caladan3_tmu_region_map_dump(int unit, args_t *args)
{
    parse_table_t pt;
    int min=-1, max=-1;
    int ret_code;

    if (ARG_CNT(args)) {
        parse_table_init(0,&pt);
        parse_table_add(&pt,"min",PQ_INT, 0, &min, NULL);
        parse_table_add(&pt,"max",PQ_INT, 0, &max, NULL);        

        if (!parseEndOk(args,&pt,&ret_code)) {
            return ret_code;
        }
    }

    soc_sbx_caladan3_tmu_region_map_dump(unit, min, max);
    return CMD_OK;
}

cmd_result_t
cmd_sbx_caladan3_ppe_hd(int unit, int parsed)
{
    int rv;
    rv = soc_sbx_caladan3_ppe_hd(unit, parsed, NULL);
    if (SOC_SUCCESS(rv)) {
        return CMD_OK;
    } else {
        return CMD_FAIL;
    }
}

cmd_result_t
cmd_sbx_caladan3_ped_hd(int unit, int pedin, int pedout, int parsed)
{
    int rv = 0;
    if ((pedin < 0) && (pedout < 0)) {
        /* dump both */
        pedin = pedout = 1;
    }
    rv = soc_sbx_caladan3_ped_hd(unit, pedin, pedout, parsed, NULL, NULL);
    if (SOC_SUCCESS(rv)) {
        return CMD_OK;
    } else {
        return CMD_FAIL;
    }
}

char cmd_sbx_caladan3_ppe_arm_usage[] = "\n"
    " PpeArm enable=[0/1] queue=[0-127] <mode>\n"
    "    Enables/Disables ppe cam trace \n"
    "    (no support for mode)\n";

cmd_result_t 
cmd_sbx_caladan3_ppe_arm(int unit, args_t *args)
{
    parse_table_t pt;
    int enable=1, queue=0, mode=-1;
    int ret_code;

    if (ARG_CNT(args)) {
        parse_table_init(0,&pt);
        parse_table_add(&pt,"enable",PQ_BOOL, 0, &enable, NULL);
        parse_table_add(&pt,"queue",PQ_INT, 0, &queue, NULL);        
        parse_table_add(&pt,"mode",PQ_BOOL, 0, &mode, NULL);        

        if (!parseEndOk(args,&pt,&ret_code)) {
            return ret_code;
        }
    }
    if ((queue < 0) || (queue > 127)) {
        cli_out("Invalid Squeue, range is 0-127\n");
        return CMD_FAIL;
    }
    soc_sbx_caladan3_ppe_cam_trace_set(unit, enable, queue, mode);
    return CMD_OK;
}

char cmd_sbx_caladan3_ppe_dump_usage[] = "\n"
    " PpeDump <cam> <entry>\n"
    "    Dumps a captured cam trace or a specific cam entry\n"
    "    Without any arguments dumps the cam trace, if any\n"
    "    cam is 0-14, entry is 0-127\n";

cmd_result_t 
cmd_sbx_caladan3_ppe_dump(int unit, args_t *args)
{
    parse_table_t pt;
    int cam=-1, entry=-1;
    int ret_code;
    int index;

    if (ARG_CNT(args)) {
        parse_table_init(0,&pt);
        parse_table_add(&pt,"cam",PQ_INT, 0, &cam, NULL);
        parse_table_add(&pt,"entry",PQ_INT, 0, &entry, NULL);        

        if (!parseEndOk(args,&pt,&ret_code)) {
            return ret_code;
        }
        if ((cam <0) || (cam > 13)) {
            cli_out("Invalid cam index %d, range is 0-13\n", cam);
            return CMD_FAIL;
        }
        if ((entry <0) || (entry > 127)) {
            cli_out("Invalid cam entry %d, range is 0-127\n", entry);
            return CMD_FAIL;
        }

        index = (entry + (cam * SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX));
        cmd_sbx_cmic_do_dump_table(unit, PP_CAM_RAMm,
                        SOC_MEM_BLOCK_MIN(unit, PP_CAM_RAMm),
                        index, 1, DUMP_TABLE_VERTICAL);
    } else {
        soc_sbx_caladan3_ppe_cam_trace_dump(unit);
    }
    return CMD_OK;
}

char cmd_sbx_caladan3_meter_monitor_usage[] = "\n"
	"MeterMonitor \n"
	"             init cop=<cop> segment=<segment>\n"
	"             attach  cop=<cop> segment=<segment> id=<policer_id>\n"
	"             detach  cop=<cop> segment=<segment> id=<policer_id>\n"
	"             display cop=<cop> segment=<segment> id=<policer_id> clear=<0/1>\n"
	;

cmd_result_t 
cmd_sbx_caladan3_meter_monitor(int unit, args_t *args)
{
	int rv = CMD_OK;
    int init = 0;
	int attach = 0;
	int display = 0;
	int detach = 0;
	int cop = -1;
	int segment = -1;
	int policer = -1;
	int monitor_id = 0;
	int clear = 0;
	parse_table_t   pt;
	soc_sbx_caladan3_cop_policer_monitor_counter_t meter_moniter_inst;
	if (ARG_CNT(args)) {
		parse_table_init(unit, &pt);
        parse_table_add(&pt, "init", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &init, NULL);
		parse_table_add(&pt, "attach", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &attach, NULL);
		parse_table_add(&pt, "display", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &display, NULL);
		parse_table_add(&pt, "detach", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &detach, NULL);
		parse_table_add(&pt, "cop", PQ_DFL | PQ_INT, 0, &cop, NULL);
		parse_table_add(&pt, "segment", PQ_DFL | PQ_INT, 0, &segment, NULL);
		parse_table_add(&pt, "id", PQ_DFL | PQ_INT, 0, &policer, NULL);
		parse_table_add(&pt, "clear", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT, 0, &clear, NULL);
		if (!parseEndOk(args, &pt, &rv)) {
			return CMD_USAGE;
		}
	}
    if (cop < 0 || segment < 0) {
        return CMD_USAGE;
    }
    
    if (init) {
        if (g_cmd_sbx_caladan3_meter_monitor[cop][segment] == NULL) {
            g_cmd_sbx_caladan3_meter_monitor[cop][segment] = 
                sal_alloc(sizeof(cmd_sbx_caladan3_meter_monitor_t) * CMD_SBX_CALADAN3_MAX_POLICER_NUM,
                                "diag policer buffer");
            if (NULL == g_cmd_sbx_caladan3_meter_monitor[cop][segment]) {
                return CMD_FAIL;
            }
        }
	} else if (attach) {
	    if (policer < 0 || g_cmd_sbx_caladan3_meter_monitor[cop][segment] == NULL) {
            return CMD_USAGE;
        }
        if (!g_cmd_sbx_caladan3_meter_monitor[cop][segment][policer].attached) {
            rv = soc_sbx_caladan3_cop_diag_policer_monitor_setup(unit, cop,
											segment, policer, &monitor_id);
    		if (SOC_FAILURE(rv)) {
    			cli_out("Fail to setup meter monitor instance\n");
    			return CMD_FAIL;
    		}
            cli_out("Attached policer %d with monitor %d\n", policer, monitor_id);
            g_cmd_sbx_caladan3_meter_monitor[cop][segment][policer].attached = TRUE;
            g_cmd_sbx_caladan3_meter_monitor[cop][segment][policer].monitor_id = monitor_id;
        }
	} else if (detach) {
	    if (policer < 0 || g_cmd_sbx_caladan3_meter_monitor[cop][segment] == NULL) {
            return CMD_USAGE;
        }
        if (g_cmd_sbx_caladan3_meter_monitor[cop][segment][policer].attached) {
            monitor_id = g_cmd_sbx_caladan3_meter_monitor[cop][segment][policer].monitor_id;
            rv = soc_sbx_caladan3_cop_diag_policer_monitor_free(unit, monitor_id); 
            if (SOC_FAILURE(rv)) {
                cli_out("Fail to free meter monitor instance\n");
                return CMD_FAIL;
            }
            cli_out("Detached policer %d with monitor %d\n", policer, monitor_id);
            g_cmd_sbx_caladan3_meter_monitor[cop][segment][policer].attached = FALSE;
        }
    } else if (display) {
        if (policer < 0 || g_cmd_sbx_caladan3_meter_monitor[cop][segment] == NULL) {
            return CMD_USAGE;
        }
        if (g_cmd_sbx_caladan3_meter_monitor[cop][segment][policer].attached) {
            monitor_id = g_cmd_sbx_caladan3_meter_monitor[cop][segment][policer].monitor_id;
            rv = soc_sbx_caladan3_cop_diag_policer_monitor_read(unit, monitor_id, clear, &meter_moniter_inst);
    		if (SOC_FAILURE(rv)) {
    			cli_out("Fail to read policer data\n");
    			return CMD_FAIL;
    		}
    		cli_out("green_to_green:   pkts:%08x%08x, bytes:%08x%08x\n", 
                        COMPILER_64_HI(meter_moniter_inst.green_to_green_pkts), COMPILER_64_LO(meter_moniter_inst.green_to_green_pkts), 
                        COMPILER_64_HI(meter_moniter_inst.green_to_green_bytes), COMPILER_64_LO(meter_moniter_inst.green_to_green_bytes));
    		cli_out("green_to_yellow:  pkts:%08x%08x, bytes:%08x%08x\n", 
                        COMPILER_64_HI(meter_moniter_inst.green_to_yellow_pkts), COMPILER_64_LO(meter_moniter_inst.green_to_yellow_pkts), 
                        COMPILER_64_HI(meter_moniter_inst.green_to_yellow_bytes), COMPILER_64_LO(meter_moniter_inst.green_to_yellow_bytes));
    		cli_out("green_to_red:     pkts:%08x%08x, bytes:%08x%08x\n", 
                        COMPILER_64_HI(meter_moniter_inst.green_to_red_pkts), COMPILER_64_LO(meter_moniter_inst.green_to_red_pkts), 
                        COMPILER_64_HI(meter_moniter_inst.green_to_red_bytes), COMPILER_64_LO(meter_moniter_inst.green_to_red_bytes));
    		cli_out("green_to_drop:    pkts:%08x%08x, bytes:%08x%08x\n", 
                        COMPILER_64_HI(meter_moniter_inst.green_to_drop_pkts), COMPILER_64_LO(meter_moniter_inst.green_to_drop_pkts), 
                        COMPILER_64_HI(meter_moniter_inst.green_to_drop_bytes), COMPILER_64_LO(meter_moniter_inst.green_to_drop_bytes));
    		cli_out("yellow_to_green:  pkts:%08x%08x, bytes:%08x%08x\n", 
                        COMPILER_64_HI(meter_moniter_inst.yellow_to_green_pkts), COMPILER_64_LO(meter_moniter_inst.yellow_to_green_pkts), 
                        COMPILER_64_HI(meter_moniter_inst.yellow_to_green_bytes), COMPILER_64_LO(meter_moniter_inst.yellow_to_green_bytes));
    		cli_out("yellow_to_yellow: pkts:%08x%08x, bytes:%08x%08x\n", 
                        COMPILER_64_HI(meter_moniter_inst.yellow_to_yellow_pkts), COMPILER_64_LO(meter_moniter_inst.yellow_to_yellow_pkts), 
                        COMPILER_64_HI(meter_moniter_inst.yellow_to_yellow_bytes), COMPILER_64_LO(meter_moniter_inst.yellow_to_yellow_bytes));
    		cli_out("yellow_to_red:    pkts:%08x%08x, bytes:%08x%08x\n", 
                        COMPILER_64_HI(meter_moniter_inst.yellow_to_red_pkts), COMPILER_64_LO(meter_moniter_inst.yellow_to_red_pkts), 
                        COMPILER_64_HI(meter_moniter_inst.yellow_to_red_bytes), COMPILER_64_LO(meter_moniter_inst.yellow_to_red_bytes));
    		cli_out("yellow_to_drop:   pkts:%08x%08x, bytes:%08x%08x\n", 
                        COMPILER_64_HI(meter_moniter_inst.yellow_to_drop_pkts), COMPILER_64_LO(meter_moniter_inst.yellow_to_drop_pkts), 
                        COMPILER_64_HI(meter_moniter_inst.yellow_to_drop_bytes), COMPILER_64_LO(meter_moniter_inst.yellow_to_drop_bytes));
    		cli_out("red_to_green:     pkts:%08x%08x, bytes:%08x%08x\n", 
                        COMPILER_64_HI(meter_moniter_inst.red_to_green_pkts), COMPILER_64_LO(meter_moniter_inst.red_to_green_pkts), 
                        COMPILER_64_HI(meter_moniter_inst.red_to_green_bytes), COMPILER_64_LO(meter_moniter_inst.red_to_green_bytes));
    		cli_out("red_to_yellow:    pkts:%08x%08x, bytes:%08x%08x\n", 
                        COMPILER_64_HI(meter_moniter_inst.red_to_yellow_pkts), COMPILER_64_LO(meter_moniter_inst.red_to_yellow_pkts), 
                        COMPILER_64_HI(meter_moniter_inst.red_to_yellow_bytes), COMPILER_64_LO(meter_moniter_inst.red_to_yellow_bytes));
    		cli_out("red_to_red:       pkts:%08x%08x, bytes:%08x%08x\n", 
                        COMPILER_64_HI(meter_moniter_inst.red_to_red_pkts), COMPILER_64_LO(meter_moniter_inst.red_to_red_pkts), 
                        COMPILER_64_HI(meter_moniter_inst.red_to_red_bytes), COMPILER_64_LO(meter_moniter_inst.red_to_red_bytes));
    		cli_out("nop:              pkts:%08x%08x, bytes:%08x%08x\n", 
                        COMPILER_64_HI(meter_moniter_inst.nop_pkts), COMPILER_64_LO(meter_moniter_inst.nop_pkts), 
                        COMPILER_64_HI(meter_moniter_inst.nop_bytes), COMPILER_64_LO(meter_moniter_inst.nop_bytes));
    		cli_out("overflow_error:   pkts:%08x%08x, bytes:%08x%08x\n", 
                        COMPILER_64_HI(meter_moniter_inst.meter_bucket_overflow_error_pkts), COMPILER_64_LO(meter_moniter_inst.meter_bucket_overflow_error_pkts), 
                        COMPILER_64_HI(meter_moniter_inst.meter_bucket_overflow_error_bytes), COMPILER_64_LO(meter_moniter_inst.meter_bucket_overflow_error_bytes));
    		cli_out("ocm_error:        pkts:%08x%08x, bytes:%08x%08x\n", 
                        COMPILER_64_HI(meter_moniter_inst.ocm_error_pkts), COMPILER_64_LO(meter_moniter_inst.ocm_error_pkts), 
                        COMPILER_64_HI(meter_moniter_inst.ocm_error_bytes), COMPILER_64_LO(meter_moniter_inst.ocm_error_bytes));
    		cli_out("ecc_error:        pkts:%08x%08x, bytes:%08x%08x\n", 
                        COMPILER_64_HI(meter_moniter_inst.ecc_error_pkts), COMPILER_64_LO(meter_moniter_inst.ecc_error_pkts), 
                        COMPILER_64_HI(meter_moniter_inst.ecc_error_bytes), COMPILER_64_LO(meter_moniter_inst.ecc_error_bytes));
        }
	}

	return CMD_OK;
}

char cmd_sbx_caladan3_sws_param_dump_usage[] = "\n"
    " SwsParamDump [tdm] \n"
    "    Dumps SWS internal parameters \n"
    "    Optionally Supported TDM list\n";

cmd_result_t 
cmd_sbx_caladan3_sws_param_dump(int unit, args_t *args)
{
    char *a;
    if (ARG_CNT(args) < 1) {
        soc_sbx_caladan3_sws_config_dump(unit);
    } else {
        a = ARG_GET(args);
        if (a && (sal_strcasecmp(a, "tdm") == 0)) {
             soc_sbx_caladan3_sws_tdm_show_all(unit);
        }
    }
    return CMD_OK;
}


char cmd_sbx_caladan3_sws_get_usage[] = "\n"
    "   Dumps SWS ICC tcam config:\n"
    "       SwsIccGet pr=<0/1> index=<0/255> num=<1/256>\n";

char cmd_sbx_caladan3_sws_set_usage[] = "\n"
    "SwsIccSet\n"
#ifndef COMPILER_STRING_CONST_LIMIT
    "\tpr={0|1}\n"
    "\tindex={0..255}\n"
    "\tenable={True|False}\n"
    "\tkey={Up to 25 hex bytes of key\n"
    "\tstate_key={3 hex bytes of key state\n"
    "\tmask={Up to 25 hex bytes of mask\n"
    "\tstate_mask={3 hex bytes of mask state\n"
    "\tlast={0|1}\n"
    "\tResult entries valid when last=1\n"
    "\t  queue={0..127}\n"
    "\t  queue_action={0-default, 2-lookup, 3-indexed}\n"
    "\t  drop={0|1}\n"
    "\t  cos={0..7}\n"
    "\t  dp={0..3}\n"
    "\t  default_de={0..2}\n"
    "\t  select_de={0|1}\n"
    "\tResult entries valid when last=0\n"
    "\t  state={0-0xffffff}\n"
    "\t  shift={0-31}\n"
#endif
;

static int cmd_char_simple_convert(char letter) {
    int val = (int) letter;

    if ((val == 88) || (val == 120)) {
        /* 'X' or 'x' */
        return 0;
    } else if ((val >= 48) && (val <= 57)) {
        /* '0' - '9' */
        return val - 48;
    } else if ((val >= 65) && (val <= 70)) {
        /* 'A' - 'F' */
        return val - 65 + 10;
    } else if ((val >= 97) && (val <= 102)) {
        /* 'a' - 'f' */
        return val - 97 + 10;
    } else {
        return -1;
    }
}


void soc_sbx_caladan3_sws_pr_icc_tcam_dump(int unit, int pr_in, int s_idx, int num)
{
    int     idx, ii;
    int     valid;
    int		pr;
    uint16              dev_id;
    uint8               rev_id;
    uint8   state[4], state_mask[4], pkt_key[26], pkt_mask[26];
    soc_sbx_calandan3_icc_state_info_t      state_info, state_mask_info;
    soc_sbx_caladan3_pr_icc_lookup_data_t   data;

    pr = SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(pr_in);

    soc_cm_get_id(unit, &dev_id, &rev_id);

    for (idx=s_idx; idx<256 && num>0; idx++, num--)
    {
        valid = 0;
        soc_sbx_caladan3_sws_pr_icc_tcam_entry_get(unit, pr, idx, &valid,
                            pkt_key, pkt_mask, state, state_mask, &data);
        if (valid != SOC_SBX_CALADAN3_SWS_PR_TCAM_ENTRY_VALID)
        {
            continue;
        }

        cli_out("***************************sws tcam inx %d info***************************\n\r", idx);
        cli_out("Rule:\n\r");
        /* print state/state-mask info */
        soc_sbx_calandan3_sws_icc_state_key_unpack(&state_info, state);
        soc_sbx_calandan3_sws_icc_state_key_unpack(&state_mask_info, state_mask);
        cli_out("    %-14s%02X%02X%02X (flag:%X, stage:%X, unuse:0, profile: %X, port:%X)\n\r",
                "State", state[2], state[1], state[0],
                state_info.flag, state_info.stage, state_info.profile, state_info.port);
        cli_out("    %-14s%02X%02X%02X (flag:%X, stage:%X, unuse:0, profile: %X, port:%X)\n\r",
                "State-mask", state_mask[2], state_mask[1], state_mask[0],
                state_mask_info.flag, state_mask_info.stage,
                state_mask_info.profile, state_mask_info.port);
        /* print 25byte packet key/mask info */
        cli_out("    %-14s", "Packet:");
        for (ii=0; ii<25; ii++)
        {
            if (((ii%4) == 0) && (ii != 0))
            {
                cli_out(" ");
            }
            cli_out("%02X", pkt_key[ii]);
        }
        cli_out("\n\r");
        cli_out("    %-14s", "Packet-mask:");
        for (ii=0; ii<25; ii++)
        {
            if (((ii%4) == 0) && (ii != 0))
            {
                cli_out(" ");
            }
            cli_out("%02X", pkt_mask[ii]);
        }
        cli_out("\n\r");

        /* print tcam result */
        cli_out("Result(last=%d):\n\r", data.last);
        if (data.last)
        {
            cli_out("    %-14s", "action:");
            if (data.drop)
            {
                cli_out("Drop ");
            }
            if (data.queue_action == SOC_SBX_CALADAN3_PR_QUEUE_ACTION_DEFAULT)
            {
                cli_out("queue-default ");
            }
            else if (data.queue_action == SOC_SBX_CALADAN3_PR_QUEUE_ACTION_LOOKUP)
            {
                cli_out("queue-lookup:%d ", data.queue);
            }
            else if (data.queue_action == SOC_SBX_CALADAN3_PR_QUEUE_ACTION_INDEXED)
            {
                cli_out("queue-indexed:%d ", data.queue);
            }
            if (data.select_de)
            {
                cli_out("select-de:%d ", data.default_de);
            }
            else
            {
                cli_out("default-de:%d ", data.default_de);
            }
            cli_out("cos=%d, dp=%d\n\r\n\r", data.cos, data.dp);
        }
        else
        {
            /*UN_PACK_DATA(data.state, state[3], state[2], state[1], state[0]);*/
            state[2] = (data.state & 0xFF0000) >> 16;
            state[1] = (data.state & 0xFF00) >> 8;
            state[0] = (data.state & 0xFF);
            soc_sbx_calandan3_sws_icc_state_key_unpack(&state_info, state);
            cli_out("    %-14s%02X%02X%02X (flag:%X, stage:%X, unuse:0, profile: %X, port:%X)\n\r",
                    "State", state[2], state[1], state[0],
                    state_info.flag, state_info.stage, state_info.profile, state_info.port);
            cli_out("    %-14s%d\n\r", "Shift:", data.shift);
            if (rev_id == BCM88030_B0_REV_ID) 
            {
                if(data.state & 0xf80000)
                {
                    cli_out("    %-14s", "Latch-action:");
                }
                if(data.state & 0x800000)
                {
                   cli_out("queue-action:%d queue:%d ", ((data.state >> 8 ) & 0x3), (data.state & 0xff));
                }
                if(data.state & 0x400000)
                {
                   cli_out("drop:%d ", ((data.state >> 10 ) & 0x1));
                }
                if(data.state & 0x200000)
                {
                   cli_out("cos:%d ", ((data.state >> 11 ) & 0x7));
                }
                if(data.state & 0x100000)
                {
                   cli_out("dp:%d ", ((data.state >> 17 ) & 0x3));
                }
                if(data.state & 0x080000)
                {
                   cli_out("select-de:%d default-de:%d ", ((data.state >> 14 ) & 0x1), ((data.state >> 15) & 0x3));
                }
            }
            cli_out("\n\r");
        }
    }
}

cmd_result_t
cmd_sbx_caladan3_sws_icc_get(int unit, args_t *args)
{
    parse_table_t   pt;
    int     		pr=0, index=0, num=1;

    if (!ARG_CNT(args))
    {
        return CMD_USAGE;
    }

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "pr",    PQ_INT, (void*)0, &pr, NULL);
    parse_table_add(&pt, "index", PQ_INT, (void*)0, &index, NULL);
    parse_table_add(&pt, "num",   PQ_INT, (void*)1, &num, NULL);
    if (parse_arg_eq(args, &pt) < 0)
    {
        cli_out("%s ERROR: parsing arguments\n", ARG_CMD(args));
        parse_arg_eq_done(&pt);
        return CMD_FAIL;
    }
    parse_arg_eq_done(&pt);

    pr = SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(pr);

    soc_sbx_caladan3_sws_pr_icc_tcam_dump(0, pr, index, num);

    return CMD_OK;
}

cmd_result_t 
cmd_sbx_caladan3_sws_icc_set(int unit, args_t *args)
{
    int rv = CMD_OK;
    int pr = 0;
    int index = 0;
    int valid = 3, enable = 0;
    char *key_str = NULL, *state_key_str = NULL, *mask_str = NULL, *state_mask_str = NULL;
    uint8 iccKey[26] = {0}, iccMask[26] = {0};
    uint8 stateKey[3] = {0}, stateMask[3] = {0};
    uint32 shift = 0;
    uint32 select_de = 0;
    uint32 queue_action = 0;
    uint32 queue = 0;
    uint32 last = 1;
    uint32 drop = 0;
    uint32 dp = 0;
    uint32 state = 0;
    uint32 default_de = 0;
    uint32 cos = 0;
    int i, num;
    soc_sbx_caladan3_pr_icc_lookup_data_t data;
    parse_table_t   pt;
    if (ARG_CNT(args)) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "pr",    PQ_BOOL | PQ_NO_EQ_OPT, 0, &pr, NULL);
        parse_table_add(&pt, "index", PQ_INT, 0, &index, NULL);
        parse_table_add(&pt, "enable", PQ_BOOL | PQ_NO_EQ_OPT, (void *) TRUE, &enable, NULL);
        parse_table_add(&pt, "key",   PQ_STRING , 
                        (void *) "0000000000000000000000000000000000000000000000000000", &key_str, NULL);
        parse_table_add(&pt, "mask",  PQ_STRING , 
                        (void *) "0000000000000000000000000000000000000000000000000000", &mask_str, NULL);
        parse_table_add(&pt, "state_key",   PQ_STRING , (void *) "000000", &state_key_str, NULL);
        parse_table_add(&pt, "state_mask",  PQ_STRING , (void *) "000000", &state_mask_str, NULL);
        parse_table_add(&pt, "shift", PQ_INT, 0, &shift, NULL);
        parse_table_add(&pt, "select_de", PQ_BOOL, 0, &select_de, NULL);
        parse_table_add(&pt, "queue_action", PQ_INT, 0, &queue_action, NULL);
        parse_table_add(&pt, "queue", PQ_INT, 0, &queue, NULL);
        parse_table_add(&pt, "last",  PQ_BOOL, (void *) TRUE, &last, NULL);
        parse_table_add(&pt, "drop",  PQ_BOOL, 0, &drop, NULL);
        parse_table_add(&pt, "dp",    PQ_INT, 0, &dp, NULL);
        parse_table_add(&pt, "state",    PQ_INT, 0, &state, NULL);
        parse_table_add(&pt, "default_de", PQ_INT, 0, &default_de, NULL);
        parse_table_add(&pt, "cos",   PQ_INT, 0, &cos, NULL);
        if (parse_arg_eq(args, &pt) < 0) {
            cli_out("%s ERROR: parsing arguments\n", ARG_CMD(args));
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }

        if (key_str) {
            num=sal_strlen(key_str)-2;
            if (sal_strlen(key_str) != 0) {
                int res;
                for (i=0; i<num; i++) {
                    res = cmd_char_simple_convert(key_str[i+2]);
                    if (res == -1) {
                        cli_out("ERROR: Improperly formatted key string\n");
                        return CMD_FAIL;
                    }
                    if (i&1) {
                        iccKey[i/2] += res;
                    } else {
                        iccKey[i/2] += res * 16;
                    }
                }
            }
        }
        
        if (state_key_str) {
            num=sal_strlen(state_key_str)-2;
            if (sal_strlen(state_key_str) != 0) {
                int res;
                for (i=0; i<num; i++) {
                    res = cmd_char_simple_convert(state_key_str[i+2]);
                    if (res == -1) {
                        cli_out("ERROR: Improperly formatted state key string\n");
                        return CMD_FAIL;
                    }
                    if (i&1) {
                        stateKey[(sizeof(stateKey)-1)-i/2] += res;
                    } else {
                        stateKey[(sizeof(stateKey)-1)-i/2] += res * 16;
                    }
                }
            }
        }
        
        if (mask_str) {
            num=sal_strlen(mask_str)-2;
            if (sal_strlen(mask_str) != 0) {
                int res;
                for (i=0; i<num; i++) {
                    res = cmd_char_simple_convert(mask_str[i+2]);
                    if (res == -1) {
                        cli_out("ERROR: Improperly formatted mask string\n");
                        return CMD_FAIL;
                    }
                    if (i&1) {
                        iccMask[i/2] += res;
                    } else {
                        iccMask[i/2] += res * 16;
                    }
                }
            }
        }

        if (state_mask_str) {
            num=sal_strlen(state_mask_str)-2;
            if (sal_strlen(state_mask_str) != 0) {
                int res;
                for (i=0; i<num; i++) {
                    res = cmd_char_simple_convert(state_mask_str[i+2]);
                    if (res == -1) {
                        cli_out("ERROR: Improperly formatted state_mask string\n");
                        return CMD_FAIL;
                    }
                    if (i&1) {
                        stateMask[(sizeof(stateMask)-1)-i/2] += res;
                    } else {
                        stateMask[(sizeof(stateMask)-1)-i/2] += res * 16;
                    }
                }
            }
        }
        
        parse_arg_eq_done(&pt);
    }
    else
    {
        return CMD_USAGE;
    }

    if ((pr > 1) || (index > 255) || 
        (queue > 127) || (queue_action > 3) ||
        (cos > 7) || (drop > 1) || (dp > 3) || (state > 0xffffff) ||
        (default_de > 2) || (select_de > 1) || (last > 1)) {
        return CMD_USAGE;
    }

    pr = SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(pr);
    if (enable == 0) {
        valid = 0;
    }

    sal_memset(&data, 0, sizeof(data));

    data.shift = shift;
    data.select_de = select_de;
    data.queue_action = queue_action;
    data.queue = queue;
    data.last = last;
    data.state = state;
    data.drop = drop;
    data.dp = dp;
    data.default_de = default_de;
    data.cos = cos;

    soc_sbx_caladan3_sws_pr_icc_tcam_program(unit, pr, index, valid, 
                                             iccKey,  iccMask,
                                             stateKey,  stateMask, &data);
    return rv;
}



void
cmd_sbx_caladan3_print_queue_info(int unit, int queue)
{
    if (queue >= SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE &&
        queue < SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE) {
        cmd_sbx_cmic_do_dump_table(unit, QM_SOURCE_QUEUE_CONFIGm,
                SOC_MEM_BLOCK_MIN(unit, QM_SOURCE_QUEUE_CONFIGm),
                queue, 1, DUMP_TABLE_VERTICAL);

        cmd_sbx_cmic_do_dump_table(unit, QM_SOURCE_QUEUE_STATE0m,
                        SOC_MEM_BLOCK_MIN(unit, QM_SOURCE_QUEUE_STATE0m),
                        queue, 1, DUMP_TABLE_VERTICAL);

        cmd_sbx_cmic_do_dump_table(unit, QM_SOURCE_QUEUE_STATE1m,
                        SOC_MEM_BLOCK_MIN(unit, QM_SOURCE_QUEUE_STATE1m),
                        queue, 1, DUMP_TABLE_VERTICAL);
        cli_out("Initial Queue State Table:\n");
        cmd_sbx_cmic_do_dump_table(unit, PP_IQSMm,
                        SOC_MEM_BLOCK_MIN(unit, PP_IQSMm),
                        queue, 1, DUMP_TABLE_VERTICAL);
        cmd_sbx_cmic_do_dump_table(unit, HPTE_DQUEUE_LOOKUPm,
                        SOC_MEM_BLOCK_MIN(unit, HPTE_DQUEUE_LOOKUPm),
                        queue, 1, DUMP_TABLE_VERTICAL);
    } else if (queue >= SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE &&
               queue <= SOC_SBX_CALADAN3_SWS_MAX_QUEUE_ID) {
        queue = queue - SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE;
        cmd_sbx_cmic_do_dump_table(unit, QM_DEST_QUEUE_STATEm,
                        SOC_MEM_BLOCK_MIN(unit, QM_DEST_QUEUE_STATEm),
                        queue, 1, DUMP_TABLE_VERTICAL);
    } else {
        cli_out("Invalid queue id: %d\n", queue);
    }
}


#define CMD_SBX_G3_TEST_USAGE \
    "   g3util test\n" \
    "   - expansion of G3 Utilities\n"

static cmd_result_t
_cmd_sbx_g3_test(int unit, args_t *args)
{
    cli_out("G3Util Test\n");

    return CMD_OK;
}

#define CMD_SBX_G3_LR_SHARED_USAGE \
    "   g3util lrshared\n" \
    "   op= data= addr=\n"

static cmd_result_t
_cmd_sbx_g3_lr_shared_reg(int unit, args_t *args)
{
    int op = 0;
    uint32 data;
    uint32 address = 0;
    parse_table_t pt;
    int ret_code;

    sal_memset(&data, 0, sizeof(uint32));

    if (ARG_CNT(args)) {
        parse_table_init(0,&pt);
        parse_table_add(&pt,"addr",PQ_INT, 0, &address, NULL);
        parse_table_add(&pt,"data",PQ_INT, 0, &data, NULL);
        parse_table_add(&pt,"op",PQ_INT, 0, &op, NULL);

        if (!parseEndOk(args,&pt,&ret_code)) {
            return ret_code;
        }
    }

    soc_sbx_caladan3_lr_shared_register_iaccess(unit, op, address, &data);

    cli_out("G3Util LRP Shared Register %x %x\n", address, data);

    return CMD_OK;
}

#define CMD_SBX_G3_PPE_PROP_USAGE \
    "   g3util ppeprop\n" \
    "   op= data0= data1= data2= addr=\n"

static cmd_result_t
_cmd_sbx_g3_ppe_prop_table(int unit, args_t *args)
{
    int op = 0;
    uint32 data[2];
    uint32 address = 0;
    parse_table_t pt;
    int ret_code;

    sal_memset(&data[0], 0, sizeof(data));

    if (ARG_CNT(args)) {
        parse_table_init(0,&pt);
        parse_table_add(&pt,"addr",PQ_INT, 0, &address, NULL);
        parse_table_add(&pt,"data0",PQ_INT, 0, &data[0], NULL);
        parse_table_add(&pt,"data1",PQ_INT, 0, &data[1], NULL);
        parse_table_add(&pt,"op",PQ_INT, 0, &op, NULL);

        if (!parseEndOk(args,&pt,&ret_code)) {
            return ret_code;
        }
    }

    soc_sbx_caladan3_ppe_property_table_iaccess(unit, op, address, data);

    if (op) {
        cli_out("G3Util PPE property Table %x %x %x\n", address, data[0], data[1]);
    }

    return CMD_OK;
}

#define CMD_SBX_G3_RELOAD_USAGE \
    "   g3util reload [reset] filename\n" \
    "   - Load Microcode from binary file with option to reset globals. Reset defaults to false.\n" \
    "   - reset || reset=<true|false> || reset=<1|0>\n" \
    "   - force || force=<true|false> || force=<1|0>"

#ifdef BCM_CALADAN3_G3P1_SUPPORT
int soc_sbx_g3p1_globals_reinit(int unit);
#endif
#ifdef BCM_CALADAN3_T3P1_SUPPORT
int soc_sbx_t3p1_globals_reinit(int unit);
#endif

static cmd_result_t
_cmd_sbx_g3_reload(int unit, args_t *args)
{
#ifndef NO_FILEIO
    char *ucodeFile = NULL;
    FILE *rf;
    unsigned char *rbuf=NULL, *bbuf=NULL, *tmpbuf = NULL;
    int rlen=0, blen=0;
    int readlen;
    soc_sbx_caladan3_ucode_pkg_t *pi = NULL;
    int rv;
    parse_table_t pt;
    int reset = 0;
    int force = 0;
    char *ucodestr = NULL;
    char *ucodesuffix = "_ucode.bin";

    if (ARG_CNT(args)) {
        parse_table_init(0,&pt);
        parse_table_add(&pt,"reset",PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &reset, NULL);
        parse_table_add(&pt,"force",PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &force, NULL);
        if ((parse_arg_eq(args, &pt) < 0) || ARG_CNT(args)) {
             ucodeFile = ARG_GET(args);
        }
    }

    if(NULL == ucodeFile){
        /* get current ucode loaded */
        ucodestr = soc_property_get_str(unit, spn_BCM88030_UCODE);
        if (ucodestr == NULL) {
            /* Broadcom standard application */
            ucodestr = "g3p1";
        }
        
        /* load the default file from current directory: ucodeFile = ucodestr_ucode.bin */
        ucodeFile = sal_alloc(strlen(ucodestr) + strlen(ucodesuffix) + 1, "ucodeFile mem");
        if (ucodeFile == NULL) {
            cli_out("G3Util Reload Microcode failed to allocate filename\n");
            return SOC_E_RESOURCE;
        }
	/* coverity[secure_coding] */
        strcpy(ucodeFile, ucodestr);
	/* coverity[secure_coding] */
        strcat(ucodeFile, ucodesuffix);
        cli_out("G3Util Reload Default Microcode %s\n", ucodeFile);
    } else {
        cli_out("G3Util Reload Microcode %s\n", ucodeFile);
    }

    cli_out("G3Util Reload Microcode %s reset=%d force=%d\n", ucodeFile, reset, force);

    /* open the text file */
    rf = fopen(ucodeFile, "r");
    if(rf == 0) {
          cli_out("did not find the binary ucode image.\n");
          sal_free(ucodeFile);
          return SB_UCODE_BAD_IMAGE_ERR_CODE;
    }

    /* get length of the text file */
    fseek(rf, 0, SEEK_END);
    rlen = ftell(rf);
    rewind(rf);
    if (rlen < 0) {
          fclose (rf);
          cli_out("file size cannot be found got(%d).\n", rlen);
          sal_free(ucodeFile);
          return SB_UCODE_BAD_IMAGE_ERR_CODE;
    }

    rbuf = sal_alloc(rlen, "ucode_buffer");
    if(!rbuf) {
        fclose (rf);
        cli_out("system error: out of memory\n");
        sal_free(ucodeFile);
        return SOC_E_RESOURCE;
    }

    /* read file to the buffer */
    tmpbuf = rbuf;
    readlen = fread (tmpbuf, 1, rlen, rf);
    if (readlen != rlen) {
        fclose (rf);
        cli_out("could not read the binary ucode image.\n");
        sal_free(rbuf);
        sal_free(ucodeFile);
        return SB_UCODE_BAD_IMAGE_ERR_CODE;
    }

    /* close the file */
    fclose (rf);

    /* calculate length of the binary */
    blen = rlen / 2;

    /* allocate binary buffer */
    bbuf = sal_alloc(blen, "ucode_buffer");
    if (!bbuf) {
        cli_out("system error: out of memory\n");
        sal_free(rbuf);
        sal_free(ucodeFile);
        return SOC_E_RESOURCE;
    }

    /* convert ascii buffer into the binary buffer */
    C3Asm3__s2b(bbuf, blen, rbuf, rlen);

    /* free ascii buffer */
    sal_free(rbuf);

    /* allocate package interface structure */
    pi = sal_alloc(sizeof(soc_sbx_caladan3_ucode_pkg_t), "ucode_pkg");
    if(!pi) {
        sal_free(bbuf);
        sal_free(ucodeFile);
        cli_out("system error: out of memory\n");
        return SOC_E_RESOURCE;
    }

    /* initialize package interface structure */
    C3Asm3__PkgInt__init(pi);

    /* read binary buffer into the interface structture */
    C3Asm3__PkgInt__readBuf(pi, bbuf, blen);

    /* free binary buffer */
    sal_free(bbuf);

    /* download ucode here */
    rv = soc_sbx_caladan3_ucodemgr_loadimg_from_pkg(unit, pi, reset, force);

    /* free package */
    C3Asm3__PkgInt__destroy(pi);

    ucodestr = soc_property_get_str(unit, spn_BCM88030_UCODE);
    if (ucodestr == NULL) {
        /* Broadcom standard application */
        ucodestr = "g3p1";
    }
    /* Reinit CLI cache of pushdown values */
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    if (ucodestr[0] == 'g') {
        soc_sbx_g3p1_globals_reinit(unit);
    }
#endif
#ifdef BCM_CALADAN3_T3P1_SUPPORT
    if (ucodestr[0] == 't') {
        soc_sbx_t3p1_globals_reinit(unit);
    }
#endif

    /* replay break points */
    soc_sbx_caladan3_ucode_debug_bp_replay(unit);

    if (SOC_SUCCESS(rv)) {
        cli_out("G3Util Reload Microcode - complete\n");
    } else {
        cli_out("G3Util Reload Microcode - error rc=%x\n", rv);
        sal_free(ucodeFile);
        return CMD_FAIL;
    }
    sal_free(ucodeFile);
    return CMD_OK;

#else

    cli_out("G3Util Reload Microcode - file i/o not supported\n");
    return CMD_OK;

#endif


}

static cmd_t _cmd_sbx_g3_list[] = {
    {"lrshared", _cmd_sbx_g3_lr_shared_reg, "\n" CMD_SBX_G3_LR_SHARED_USAGE, NULL},
    {"ppeprop", _cmd_sbx_g3_ppe_prop_table, "\n" CMD_SBX_G3_PPE_PROP_USAGE, NULL},
    {"reload", _cmd_sbx_g3_reload, "\n" CMD_SBX_G3_RELOAD_USAGE, NULL},
    {"test", _cmd_sbx_g3_test, "\n" CMD_SBX_G3_TEST_USAGE, NULL}
};

char cmd_sbx_g3_util_usage[] =
"Usage:\n"
"  g3util help               - displays this messge\n"
"  g3util lrshared           - access lrp shared registers\n"
"  g3util ppeprop            - access ppe property table\n"
"  g3util reload             - reload microcode\n"
"  g3util test               - test command\n";

cmd_result_t
cmd_sbx_g3_util(int unit, args_t *args)
{
    return subcommand_execute(unit, args, _cmd_sbx_g3_list, COUNTOF(_cmd_sbx_g3_list));
}

/**
 ** Caladan3 Ucode Debug 
 **  - Allows minimal interfacing to the Debug driver for various operations
 **  - Please see help for complete list of supported commands
 **/
char cmd_sbx_caladan3_debugger_usage[] = "\n"
#ifndef COMPILER_STRING_CONST_LIMIT
    "C3Debug init(i)      -> init the debugger interface\n"
    "C3Debug server   -> start the debugger server\n"
    "C3Debug break(b) str=<snum> inst=<inum>   \n"
    "                  -> set a break point at stream snum inst inum \n"
    "C3Debug next(n)      -> single step \n"
    "C3Debug continue(c)  -> run \n"
    "C3Debug end(e)       -> end session \n"
    "C3Debug getreg(g) rname=<rname>  \n"
    "                  -> get the register value\n"
    "C3Debug setreg(s) rname=<rname> value=<value> \n"
    "                  -> set the register value \n"
    "C3Debug clear(cr) [bp=<bpnum>]  \n"
    "                  -> clear all or the given break point\n"
    "C3Debug list(l)      -> list all current breakpoints\n" 
    "C3Debug dump(d)      -> dump registers and header record\n"
#endif
;

cmd_result_t 
cmd_sbx_caladan3_debugger(int unit, args_t *args)
{
    int rv = CMD_OK;
    int init = 0, server = 0;
    int brk = 0, cont = 0, nxt = 0, gr = 0, sr = 0, clear = 0, list = 0, end = 0;
    int bpnum = 0, snum = -1, inum = -1 , check = 0, dump = 0, upd = 0, start = 0, stop = 0;
    int bpnumclear = -1;
    char regname[20], *rname = NULL;
    char *s_snum = NULL, *s_inum = NULL, *s_value = NULL;
    char buffer[256] = {0};
    uint32 value = -1;
    uint32 internal = 0;
    parse_table_t pt;


    if (!ARG_CNT(args)) {
        return CMD_USAGE;
    } else {
        parse_table_init(unit, &pt);

        parse_table_add(&pt, "init",   PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &init, NULL);
        parse_table_add(&pt, "i",   PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &init, NULL);
        parse_table_add(&pt, "server", PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &server, NULL);
        parse_table_add(&pt, "dump",   PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &dump, NULL);
        parse_table_add(&pt, "d",   PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &dump, NULL);
        parse_table_add(&pt, "break",  PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &brk, NULL);
        parse_table_add(&pt, "b",  PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &brk, NULL);
        parse_table_add(&pt, "next",   PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &nxt, NULL);
        parse_table_add(&pt, "n",   PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &nxt, NULL);
        parse_table_add(&pt, "continue", PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &cont, NULL);
        parse_table_add(&pt, "c", PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &cont, NULL);
        parse_table_add(&pt, "clear", PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &clear, NULL);
        parse_table_add(&pt, "cr", PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &clear, NULL);
        parse_table_add(&pt, "check",   PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &check, NULL);
        parse_table_add(&pt, "end",   PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &end, NULL);
        parse_table_add(&pt, "e",   PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &end, NULL);
        parse_table_add(&pt, "update",   PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &upd, NULL);
        parse_table_add(&pt, "up",   PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &upd, NULL);
        parse_table_add(&pt, "list",  PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &list, NULL);
        parse_table_add(&pt, "l",  PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &list, NULL);
        parse_table_add(&pt, "getreg",  PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &gr, NULL);
        parse_table_add(&pt, "g",  PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &gr, NULL);
        parse_table_add(&pt, "setreg",  PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &sr, NULL);
        parse_table_add(&pt, "s",  PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &sr, NULL);
        parse_table_add(&pt, "rname",  PQ_DFL | PQ_STRING, 0, &rname, NULL);
        parse_table_add(&pt, "value",  PQ_DFL | PQ_INT, 0, &value, NULL);
        parse_table_add(&pt, "internal",  PQ_DFL | PQ_INT, 0, &internal, NULL);
        parse_table_add(&pt, "in",  PQ_DFL | PQ_INT, 0, &internal, NULL);
        parse_table_add(&pt, "str",  PQ_DFL | PQ_INT, 0, &snum, NULL);
        parse_table_add(&pt, "inst", PQ_DFL | PQ_INT, 0, &inum, NULL);
        parse_table_add(&pt, "bp",   PQ_DFL | PQ_INT, 0, &bpnumclear, NULL);
        parse_table_add(&pt, "start",   PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &start, NULL);
        parse_table_add(&pt, "stop",   PQ_NO_EQ_OPT| PQ_DFL | PQ_BOOL, 0, &stop, NULL);

        if (parse_arg_eq(args, &pt) < 0) {
            cli_out("%s: Invalid argument: %s\n", ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        if (gr || sr) {
            if (rname == NULL) {
                if ((gr && args->a_argc != 3) || 
                    (sr && args->a_argc != 4)) {
                    return CMD_USAGE;
                }
                rname = ARG_GET(args);
            } 
            /* coverity[var_deref_model] */
            sal_memcpy(regname, rname, 16);
        }
        parse_arg_eq_done(&pt);

        if (init) {
            rv = soc_sbx_caladan3_ucode_debug_thread_start(unit);
        } else if (server) {
            if (start || (!start && !stop))
                rv = mde_agent_start(unit);
            else
                rv = mde_agent_stop();
        } else if (dump) {
            rv  = soc_sbx_caladan3_ucode_debug_dump_all(unit, internal);
        } else if (check==1) {
            rv = soc_sbx_caladan3_ucode_debug_request_context(unit);
            if (SOC_SUCCESS(rv)) 
                rv  = soc_sbx_caladan3_ucode_debug_poll(unit);
            if (rv==SOC_E_TIMEOUT) {
                cli_out("Command timeed out\n");
                rv = SOC_E_NONE;
            }
        } else if (upd==1) {
            rv = soc_sbx_caladan3_ucode_debug_pull_context(unit);
        } else if (brk) {
            if (snum == -1) {
                if (args->a_argc != 4) {
                    return CMD_USAGE;
                }
                s_snum = ARG_GET(args);
                snum = sal_atoi(s_snum);
            }
            if (inum == -1) {
                if (args->a_argc != 4) {
                    return CMD_USAGE;
                }
                s_inum = ARG_GET(args);
                inum = sal_atoi(s_inum);
            }
            rv  = soc_sbx_caladan3_ucode_debug_bp_set(unit, snum, inum, &bpnum);
        } else if (nxt) {
            rv = soc_sbx_caladan3_ucode_debug_next(unit);
            if (SOC_SUCCESS(rv)) {
                int s, i, j;
                int count = 0;
                uint8 inst[12];
                do {
                    rv = soc_sbx_caladan3_ucode_debug_get_current_frame(unit,
                         100000,
                          &s, &i, inst);
                    count++;
                    sal_udelay(1000);
                } while (SOC_FAILURE(rv) && (count < 10));
                if (SOC_SUCCESS(rv)) {
                    soc_sbx_caladan3_ucode_debug_decode_inst(buffer, inst);
                    cli_out("C3Debug[str(%d):inst(%d)]#> %s\n", 
                            s, i, buffer);
                    cli_out("C3Debug[raw instruction]#> ");
                    for(j=0; j<12; j++)
                        cli_out(" %02x", inst[j]);
                    cli_out("\n");
                } else {
                    cli_out("Timeout waiting for breakpoint to be hit\n");
                }
            }
        } else if (cont) {
            rv = soc_sbx_caladan3_ucode_debug_continue(unit);
        } else if (end) {
            rv = soc_sbx_caladan3_ucode_debug_end(unit);
        } else if (clear) {
            if (bpnumclear < 0) {
                rv = soc_sbx_caladan3_ucode_debug_bp_clear_all(unit);
            } else {
                rv = soc_sbx_caladan3_ucode_debug_bp_clear(unit, bpnumclear );
            }
        } else if (list) {
            rv = soc_sbx_caladan3_ucode_debug_bp_list(unit);
        } else if (gr) {
            rv = soc_sbx_caladan3_ucode_debug_get_reg(unit, regname, &value);
            if (SOC_SUCCESS(rv)) {
                cli_out("%s => %x\n", regname, value);
            }
        } else if (sr) {
            if (value == -1) {
                if (args->a_argc != 4) {
                    return CMD_USAGE;
                }
                s_value = ARG_GET(args);
                value = sal_atoi(s_value);
            }
            cli_out("Setting %s => %x\n", regname, value);
            rv = soc_sbx_caladan3_ucode_debug_set_reg(unit, regname, value, 1);
        }
    }

    if (SOC_SUCCESS(rv)) {
        return CMD_OK;
    }

    return CMD_FAIL;

}


char cmd_sbx_caladan3_reconfig_usage[] = "\n"
    "Reconfig\n";
    
extern int c3_port_init[BCM_MAX_NUM_UNITS];

#define SOC_SBX_CALADAN3_SWS_MAX_PT_INSTANCE (2)


cmd_result_t 
cmd_sbx_caladan3_reconfig(int unit, args_t *args)
{
    char                *c;
    uint16              dev_id, dev_id_driver;
    uint8               rev_id, rev_id_driver;
#if CHECK_RECONFIG_TIME
    sal_usecs_t         start_time, end_time;
    float               seconds;
#endif
    int                 status = SOC_E_NONE;
    soc_persist_t       *sop;
    bcm_port_t          port;
    pbmp_t              pbm;


    if (!ARG_CNT(args)) {
        return CMD_USAGE;
    } else {
        c = ARG_GET(args);
        if (c == NULL) {
            return CMD_USAGE;
        }
        if (sal_strcasecmp(c, "tdm") == 0) {
            cli_out("Reconfiguring TDM\n");

            /* Set the reconfig boot flag */
            SOC_CONTROL(unit)->reconfig_flag = TRUE;

#if CHECK_RECONFIG_TIME
            start_time = sal_time_usecs();
#endif

            soc_cm_get_id(unit, &dev_id, &rev_id);
            soc_cm_get_id_driver(dev_id, rev_id, &dev_id_driver, &rev_id_driver);

            status = system_shutdown(unit, 0);
            if (status != SOC_E_NONE) {
                cli_out("%s: system_shutdown failed: %s\n",
                        FUNCTION_NAME(), bcm_errmsg(status));
            }

            status = soc_sbx_counter_detach(unit);
            if (status != SOC_E_NONE) {
                cli_out("%s: soc_sbx_counter_detach FAILED: %s\n",
                        FUNCTION_NAME(), soc_errmsg(status));
            }

            status = soc_counter_detach(unit);
            if (status != SOC_E_NONE) {
                cli_out("%s: soc_counter_detach failed: %s\n",
                        FUNCTION_NAME(), soc_errmsg(status));
            }

            /* Get the current port information */
            SOC_PBMP_ASSIGN(SOC_CONTROL(unit)->all_skip_pbm, PBMP_ALL(unit));
            for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
                SOC_CONTROL(unit)->port_l2p_mapping[port] =
                    SOC_INFO(unit).port_l2p_mapping[port];
                soc_sbx_caladan3_get_intftype(unit, port,
                    &SOC_CONTROL(unit)->intftype[port]);
            }

            status = soc_sbx_caladan3_reconfig_port_cleanup(unit);
            if (status != SOC_E_NONE) {
                cli_out("%s: soc_sbx_caladan3_reconfig_port_cleanup failed: %s\n",
                        FUNCTION_NAME(), soc_errmsg(status));
            }

            status = soc_sbx_init(unit);
            if (status != SOC_E_NONE) {
                cli_out("%s: soc_sbx_init failed: %s\n",
                        FUNCTION_NAME(), soc_errmsg(status));
            }

            /* The IPG values used to program the MACs are normally configured 
            * only during the attach. But it must be done again here. 
            */ 
            sop = SOC_PERSIST(unit); 
            PBMP_ALL_ITER(unit, port) { 
                sop->ipg[port].hd_10    = 96; 
                sop->ipg[port].hd_100   = 96; 
                sop->ipg[port].hd_1000  = 96; 
                sop->ipg[port].hd_2500  = 96; 

                sop->ipg[port].fd_10    = 96; 
                sop->ipg[port].fd_100   = 96; 
                sop->ipg[port].fd_1000  = 96; 
                sop->ipg[port].fd_2500  = 96; 
                sop->ipg[port].fd_10000 = 96; 
                sop->ipg[port].fd_xe    = 96; 
                sop->ipg[port].fd_hg    = 64; 
            } 

            status = soc_counter_attach(unit);
            if (status != SOC_E_NONE) {
                cli_out("%s: soc_counter_attach failed: %s\n",
                        FUNCTION_NAME(), soc_errmsg(status));
            }

            status = bcm_port_init(unit);
            if (status != SOC_E_NONE) {
                cli_out("%s: bcm_port_init failed: %s\n",
                        FUNCTION_NAME(), bcm_errmsg(status));
            }

            status = bcm_stg_init(unit);
            if (status != SOC_E_NONE) {
                cli_out("%s: bcm_stg_init failed: %s\n",
                        FUNCTION_NAME(), bcm_errmsg(status));
            }

            status = bcm_vlan_init(unit);
            if (status != SOC_E_NONE) {
                cli_out("%s: bcm_vlan_init failed: %s\n",
                        FUNCTION_NAME(), bcm_errmsg(status));
            }

            status = bcm_cosq_init(unit);
            if (status != SOC_E_NONE) {
                cli_out("%s: bcm_cosq_init failed: %s\n",
                        __FUNCTION__, bcm_errmsg(status));
            }

            status = bcm_stat_init(unit);
            if (status != SOC_E_NONE) {
                cli_out("%s: bcm_stat_init failed: %s\n",
                        FUNCTION_NAME(), bcm_errmsg(status));
            }

            status = bcm_linkscan_init(unit);
            if (status != SOC_E_NONE) {
                cli_out("%s: bcm_linkscan_init failed: %s\n",
                        FUNCTION_NAME(), bcm_errmsg(status));
            }

            /* End of tdm reset */

#if CHECK_RECONFIG_TIME
            end_time = sal_time_usecs();
            seconds = (float)(end_time - start_time) / 1000.0;
            cli_out("reconfig tdm: total init time: %f mS\n", seconds);
#endif

            c3_port_init[unit] = 0;

            /* Clear the reconfig boot flag */
            SOC_CONTROL(unit)->reconfig_flag = FALSE;

            SOC_PBMP_CLEAR(SOC_CONTROL(unit)->all_skip_pbm);
            SOC_PBMP_CLEAR(SOC_CONTROL(unit)->mac_phy_skip_pbm);

        } else if (sal_strcasecmp(c, "flush") == 0) {
            c = ARG_GET(args);
            if (c == NULL) {
                cli_out("%s: Must specify ports\n", FUNCTION_NAME());
                return CMD_FAIL;
            }
            if (parse_pbmp(unit, c, &pbm) < 0) {
                cli_out("%s: Invalid ports: %s\n", FUNCTION_NAME(), c);
                return CMD_FAIL;
            }
            status = soc_sbx_caladan3_flush_ports(unit, &pbm);
            if (status != SOC_E_NONE) {
                cli_out("%s: soc_sbx_caladan3_flush_ports failed: %s\n",
                        FUNCTION_NAME(), soc_errmsg(status));
            }
        } else {
            cli_out("No match\n");
            ARG_DISCARD(args);
        }
    }

    return CMD_OK;
}

/**
 ** Caladan3 diag monitor thread
 **  - Can be used to run any diag shell command periodically
 **  - multiple commands can be combined ( ';' separated) 
 **/

char cmd_sbx_caladan3_bgnd_monitor_usage[] =     \
    "c3monitor delay \"any diag command\" \n"                                      \
    "      monitor periodically any command, recommended delay is > 10sec\n"       \
    "c3monitor show  \n"                                                           \
    "      show current status\n";

int c3_watch_running = 0;
int c3_watch_delay = 0;
sal_mutex_t c3_watch_mutex;
char c3_watch_command[256] = {0};

void caladan3_bgnd_monitor(void *handle) 
{
    int unit = PTR_TO_INT(handle);
#ifndef __KERNEL__
    sal_time_t val = 0;
#endif
    char buf[64] = {0};
    cli_out("C3 Monitor Started\n");
    c3_watch_running = 1;

    do {
#ifndef __KERNEL__
        sal_date_get(&val);
        strftime(buf, sizeof (buf), "%Y/%m/%d %H:%M:%S %Z", localtime((time_t *) &val));
#endif
        cli_out("C3 Monitor: %s \n", buf);
        sal_mutex_take(c3_watch_mutex, sal_mutex_FOREVER);
        sh_process_command(unit, c3_watch_command);
        sal_mutex_give(c3_watch_mutex);
        sal_usleep(c3_watch_delay);
    } while (c3_watch_delay > 0);
    c3_watch_running = 0;
    sal_mutex_destroy(c3_watch_mutex);
    c3_watch_mutex = NULL;
    cli_out("C3 Monitor Stopped\n");
}

cmd_result_t
cmd_sbx_caladan3_bgnd_monitor(int unit, args_t *args)
{
    /*int len = 0;*/
    char *c;
    int delay = 0;

    if (!ARG_CNT(args)) {
        return CMD_USAGE;
    } else {
        c = ARG_GET(args);
        if (sal_strcasecmp(c, "show") == 0) {
             if (c3_watch_running) {
                 cli_out("unit %d: monitoring enabled for %s \n", unit, c3_watch_command);
             } else {
                 cli_out("unit %d: monitoring not enabled ", unit);
             }
             return CMD_OK;
        } else {
            delay = _shr_ctoi(c);
            if (delay < 0) {
                return CMD_OK;
            } 
            c3_watch_delay = delay*1000*1000;
            c = ARG_GET(args);
            if (!c) {
                return CMD_USAGE;
            } else {
                if (c3_watch_mutex == NULL) {
                    c3_watch_mutex = sal_mutex_create("C3 Monitor");
                    if (c3_watch_mutex == NULL) {
                        cli_out("Unit %d failed allocating mutex", unit);
                        return CMD_FAIL;
                    }
                }
                sal_mutex_take(c3_watch_mutex, sal_mutex_FOREVER);
		/* size of c3_watch_cmd is 256 which is large enough */
		/* coverity[secure_coding] */
                sprintf(c3_watch_command, "%s", c);
                sal_mutex_give(c3_watch_mutex);
            }
        }
    }
    if (c3_watch_running == 0) {
        sal_thread_create("bcmBkgnd",
                          SAL_THREAD_STKSZ,
                          50,
                          (void (*)(void*))caladan3_bgnd_monitor,
                          INT_TO_PTR(unit));
        cli_out("unit %d: monitoring enabled \n", unit);
    }
    return CMD_OK;
}

void
soc_sbx_caladan3_etu_tcam_nl_ref_app(int unit, args_t *a)
{
#ifdef NLMPLATFORM_BCM
    char *app = NULL;
    int appid = 0;

    if (a) {
        app = ARG_GET(a);
    }
    if (app) {
        appid = parse_integer(app);
        switch (appid) {
            case 0:
                nlmdiag_refapp_main(a->a_argc-3, &a->a_argv[3]);
                break;
            case 1:
                nlmdevmgr_refapp_main(a->a_argc-3, &a->a_argv[3]);
                break;
            case 2:
                nlmgenerictblmgr_refapp_main(a->a_argc-3, &a->a_argv[3]);
                break;
            case 3:
                nlmfibtblmgr_refapp_main(a->a_argc-3, &a->a_argv[3]);
                break;
            case 4:
                nlmgtmftm_refapp_main(a->a_argc-3, &a->a_argv[3]);
                break;
            case 5:
                nlmrangemgr_refapp_main(a->a_argc-3, &a->a_argv[3]);
                break;
            case 6:
                nlmrangemgr_refapp2_main(a->a_argc-3, &a->a_argv[3]);
                break;
            case 7:
                nlmrangemgr_refapp3_main(a->a_argc-3, &a->a_argv[3]);
                break;
        }
        ARG_DISCARD(a);
        return;
    }
    cli_out(" 0 --> nlmdiag_refapp\n");
    cli_out(" 1 --> nlmdevmgr_refapp\n");
    cli_out(" 2 --> nlmgenerictblmgr_refapp\n");
    cli_out(" 3 --> nlmdevmgr_mt_refapp\n");
    cli_out(" 4 --> nlmgtmftm_refapp\n");
    cli_out(" 5 --> nlmrangemgr_refapp\n");
    cli_out(" 6 --> nlmrangemgr_refapp2\n");
    cli_out(" 7 --> nlmrangemgr_refapp3\n");
#else
    cli_out(" netlogic reference application not compiled in\n");
    return;
#endif
}


/**
 ** Caladan3 external TCAM access
 **/
char cmd_sbx_caladan3_tcam_access_usage[] =     
    "\nTcam Mdio  <devid> <addr> [value] \n"                         
#ifndef COMPILER_STRING_CONST_LIMIT                                  
    "  - perform mdio config/status space read/write \n"             
    "    without optional param value, do read register \n"          
    "    with optional param value, do write register \n"            
    "Tcam verbose <0|1> \n"                                          
    "   - Dump messages\n"                                           
    "Tcam Reg <addr> [<data>] \n"                                    
    "Tcam Db  Read   <block> <addr> \n"                              
    "Tcam Db  Write  <block> <addr> [<data>] [<mask>] [<valid>] [<mode>]\n"
    "Tcam Db  Lookup <block> <addr> [<data>] [<mask>] \n"            
    "Tcam Db  Dump   <block> <addr> <count> \n"                      
    "  - perform functional register/db space read/write \n"         
    "  where, \n"                                                    
    "      data, mask are hex strings with no separations\n"         
    "      eg 0x0123456789abcdef upto 80bits\n"                      
    "      data is required for write \n"                            
    "      mask is optional, defaults to all 1s \n"                  
    "      valid is optional, defaults to 1 \n"                      
    "      mode is optional, defaults to 1(X/Y mode) \n"             
    "      block is 0-127, addr is 0-4k\n"                           
    "Tcam Status \n"                                                 
    "  - Dump External tcam status\n"
    "\t  tcam prbs <set>|<get>|<clear> [poly={0|1|2|3}] <seed> \n"
    "\t       -For ex: \n" 
    "\t        tcam prbs set poly=3 seed=0xaaaaaaaa\n" 

#endif
;

void
cmd_print_hex_string(char *str, uint8 *value, int bitsize) 
{
    int i, max;
    int marker = (bitsize  == 80) ? 2 : 4;
    max = bitsize >> 3;
    if (str)
        cli_out("%s 0x", str);
    for(i=0; i < max; i++) {
        cli_out("%02x", value[i]);
        if ((i > 0) && ((i+1) < max) && (((i-1) % marker) == 0)) {
            cli_out("_");
        }
    }
    
}

#define HEXP(a)  ((a) >= 'a' ? (10+(a)-'a') : (((a) >= 'A') ? (10+(a)-'A') : (a)-'0'))

#define HEX_CHECK(v)                                                          \
        do { if ((((v) < '0') && ((v) > '9')) ||                              \
                (((v) < 'a') && ((v) > 'f')) ||                               \
                (((v) < 'A') && ((v) > 'F')))   {                             \
                cli_out(\
                        "cmd_parse_hex_string: Invalid param\n");         \
                return (CMD_FAIL);                                             \
            }                                                                  \
        } while(0)


static  cmd_result_t
cmd_parse_hex_string(char *v, uint8 *value, int width) 
{
    int i;
    char *p = v;
    if ((!value) || (!v)) {
        cli_out("cmd_parse_hex_string: Invalid param\n");
        return (CMD_FAIL);
    }
    /* coverity[suspicious_sizeof] */
    sal_memset(value, 0, sizeof(value));
    if ((*v == '0') && (*(v+1) == 'x')) v+=2;
    i = 0;
    while ((*v != 0) && (*(v+1)) != 0) {
        if (*v == '_') {v++; continue;} /* tolerate arbitary _ seperators */
        HEX_CHECK(*v);
        HEX_CHECK(*(v+1));
        value[i++] = (((HEXP(*v)) << 4) | HEXP(*(v+1)));
        v += 2;
        if (i==width>>3) break;
    }
    if (*v != 0) {
	cli_out("\nMalformed hex string: %s at offset %d", p, (int)(v-p));
        return CMD_FAIL;
    }
    cmd_print_hex_string("Debug:", value, width);
    cli_out("\n");
    return CMD_OK;
}


int
c3_nl_mdio_prbs_chk(int unit, uint16 mdio_portid, uint16 prbs, uint32 seed, 
                 uint16 crx, uint32 enable)
{
    uint16 nl_tcams = 1;/*((soc_tcam_info_t *)SOC_CONTROL(unit)->tcam_info)->num_tcams;*/
    uint16 mdio_devid_max = crx? 10 : 7;
    uint16 mdio_devid_min = 2;
    uint16 mdio_devid;

    seed = enable && (seed!=0)? seed : 0xAAAAAAAA;
    /* PRBS control [2:1] - poly */
    prbs = enable? (prbs == 1? 0x4: ((prbs == 2)? 0x2: 0x0)) : 0x0;
    /* PRBS control [0] - enable */
    prbs = enable? prbs | 0x1: prbs;

    for(mdio_devid = mdio_devid_min; mdio_devid <= mdio_devid_max; ++mdio_devid) {
        /* If cascaded tcam, request lines are just 16 lanes not 24,
           Skip for devid 6 & 7.
         */
        /* coverity[dead_error_line] */
        if((nl_tcams>1) && ((mdio_devid == 6) || (mdio_devid == 7))) {
            continue;
        }
        SOC_IF_ERROR_RETURN
            (soc_etu_nl_mdio_write(unit, mdio_portid, mdio_devid, 0x12, seed & 0xffff, 1));
        SOC_IF_ERROR_RETURN
            (soc_etu_nl_mdio_write(unit, mdio_portid, mdio_devid, 0x13, (seed >> 16) & 0xffff, 1));
        SOC_IF_ERROR_RETURN
            (soc_etu_nl_mdio_write(unit, mdio_portid, mdio_devid, 0x11, prbs, 1));
    }

    _SHR_RETURN(SOC_E_NONE);
}






int
c3_nl_mdio_prbs_gen(int unit, uint16 mdio_portid, uint16 prbs, uint32 seed, 
                 uint16 ctx, uint32 enable)
{
    uint16 nl_tcams = 1; /*((soc_tcam_info_t *)SOC_CONTROL(unit)->tcam_info)->num_tcams;*/
    /* coverity[dead_error_line] */
    uint16 mdio_devid_max = ctx? ((nl_tcams>1)? 17: 19) : 13;
    uint16 mdio_devid_min = 11;
    uint16 mdio_devid;

    seed = enable && (seed!=0)? seed : 0xAAAAAAAA;
    /* PRBS control [2:1] - poly */
    prbs = enable? (prbs == 1? 0x4: ((prbs == 2)? 0x2: 0x0)) : 0x0;
    /* PRBS control [0] - enable */
    prbs = enable? prbs | 0x1: 0x0;

    for(mdio_devid = mdio_devid_min; mdio_devid <= mdio_devid_max; ++mdio_devid) {
        SOC_IF_ERROR_RETURN
            (soc_etu_nl_mdio_write(unit, mdio_portid, mdio_devid, 0x12, seed & 0xffff, 1));
        SOC_IF_ERROR_RETURN
            (soc_etu_nl_mdio_write(unit, mdio_portid, mdio_devid, 0x13, (seed >> 16) & 0xffff, 1));
        SOC_IF_ERROR_RETURN
            (soc_etu_nl_mdio_write(unit, mdio_portid, mdio_devid, 0x11, prbs, 1));
    }

    _SHR_RETURN(SOC_E_NONE);
}

int
c3_nl_mdio_prbs_chk_err(int unit, uint16 mdio_portid, uint16 crx)
{
    uint16 nl_tcams = 1; /*((soc_tcam_info_t *)SOC_CONTROL(unit)->tcam_info)->num_tcams;*/
    uint16 mdio_devid_max = crx?10:7;
    uint16 mdio_devid_min = 2;
    uint16 mdio_devid;
    uint16 prbs_ctrl, prbs_pass = 1;
    uint16 err_count_l0 = 0, err_count_l1 = 0, err_count_l2, err_count_l3 = 0;

    for(mdio_devid = mdio_devid_min; mdio_devid <= mdio_devid_max; ++mdio_devid) {
        /* If cascaded tcam, request lines are just 16 lanes not 24,
           Skip for devid 6 & 7.
         */
        /* coverity[dead_error_line] */
        if((nl_tcams>1) && ((mdio_devid == 6) || (mdio_devid == 7))) {
            continue;
        }
        SOC_IF_ERROR_RETURN
            (soc_etu_nl_mdio_read(unit, mdio_portid, mdio_devid, 0x11, &prbs_ctrl));
        if((prbs_ctrl & 0x1) == 0x0) {
            LOG_WARN(BSL_LS_APPL_COMMON,
                     (BSL_META_U(unit,
                                 "PRBS is not enabled for NL TCAM mdio_portid=%d\n"), mdio_portid));
            return SOC_E_FAIL;
        }
        SOC_IF_ERROR_RETURN
            (soc_etu_nl_mdio_read(unit, mdio_portid, mdio_devid, 0x14, &err_count_l0));
        SOC_IF_ERROR_RETURN
            (soc_etu_nl_mdio_read(unit, mdio_portid, mdio_devid, 0x15, &err_count_l1));
        SOC_IF_ERROR_RETURN
            (soc_etu_nl_mdio_read(unit, mdio_portid, mdio_devid, 0x16, &err_count_l2));
        SOC_IF_ERROR_RETURN
            (soc_etu_nl_mdio_read(unit, mdio_portid, mdio_devid, 0x17, &err_count_l3));

#if 0
        /* Read twice to clear  - nl_mdio_prbs_ignore_err*/
        SOC_IF_ERROR_RETURN
            (soc_etu_nl_mdio_read(unit, mdio_portid, mdio_devid, 0x14, &err_count_l0));
        SOC_IF_ERROR_RETURN
            (soc_etu_nl_mdio_read(unit, mdio_portid, mdio_devid, 0x15, &err_count_l1));
        SOC_IF_ERROR_RETURN
            (soc_etu_nl_mdio_read(unit, mdio_portid, mdio_devid, 0x16, &err_count_l2));
        SOC_IF_ERROR_RETURN
            (soc_etu_nl_mdio_read(unit, mdio_portid, mdio_devid, 0x17, &err_count_l3));
#endif

        if(err_count_l0 || err_count_l1 || err_count_l2 || err_count_l2) {
            LOG_WARN(BSL_LS_APPL_COMMON,
                     (BSL_META_U(unit,
                                 "PRBS error count at mdio_portid %d mdio_devid %d: \n"),mdio_portid, mdio_devid));
          LOG_WARN(BSL_LS_APPL_COMMON,
                   (BSL_META_U(unit,
                               "\t->lane0=%d\n"),err_count_l0));
          LOG_WARN(BSL_LS_APPL_COMMON,
                   (BSL_META_U(unit,
                               "\t->lane1=%d\n"),err_count_l1));
          LOG_WARN(BSL_LS_APPL_COMMON,
                   (BSL_META_U(unit,
                               "\t->lane2=%d\n"),err_count_l2));
          LOG_WARN(BSL_LS_APPL_COMMON,
                   (BSL_META_U(unit,
                               "\t->lane3=%d\n"),err_count_l3));
          prbs_pass = 0;    
        }
    }
    if(prbs_pass) {
        LOG_WARN(BSL_LS_APPL_COMMON,
                 (BSL_META_U(unit,
                             "\t->PRBS OK \n")));
    }
    _SHR_RETURN(SOC_E_NONE);
}



/* Entry size in bits */
static int soc_sbx_caladan3_tcam_diag_entry_width = 80; 


cmd_result_t
cmd_sbx_caladan3_tcam_access(int unit, args_t *a)
{
    char * subcmd;
    int rv, cmd_rv;
    int portid = 0;  /* Accessing only dev 0, extend in future */
    char *mask_str, *data_str, *valid_str, *wr_mode_str, *rd_mode_str, *cnt_str;
    char *reg, *val, *dev, *blk_str, *idx_str;
    uint8 mask[256], data[256], value[256], x[256], y[256], valid, wr_mode, rd_mode;
    int p, i = 0, op = 0;
    int blk, idx, cnt, max, debug_flag;
    uint16 devid, mdio_reg, mdio_regval;
    uint32 regaddr = 0;
    etu_tcam_lookup_t lkup;
    etu_tcam_result_t res[4];
    parse_table_t pt;

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    if (sal_strcasecmp(subcmd, "init") == 0) {
        soc_sbx_caladan3_etu_tcam_config(unit);
    } else if (sal_strcasecmp(subcmd, "refapp") == 0) {
        soc_sbx_caladan3_etu_tcam_nl_ref_app(unit, a);
    } else if (sal_strcasecmp(subcmd, "verbose") == 0)  {
        dev = ARG_GET(a);
        if (dev) {
           debug_flag = parse_integer(dev); 
           soc_sbx_caladan3_etu_debug_set(unit, debug_flag);
           return (CMD_OK);
        } else {
           return (CMD_USAGE);
        }
    } else if ((sal_strcasecmp(subcmd, "status") == 0) || (subcmd[0] == 'S')) {
        soc_etu_nl_mdio_print_csm_status(unit, portid);
        return (CMD_OK);
    } else if ((sal_strcasecmp(subcmd, "error") == 0) || (subcmd[0] == 'E')) {
        soc_etu_nl_mdio_chk_error_counters_status(unit, portid, 0);
        return (CMD_OK);
    } else if (sal_strcasecmp(subcmd, "entry") == 0) {
        dev = ARG_GET(a);
        if (dev) {
            soc_sbx_caladan3_tcam_diag_entry_width = parse_integer(dev); 
            if ((soc_sbx_caladan3_tcam_diag_entry_width < 80) || 
                (soc_sbx_caladan3_tcam_diag_entry_width > 640)) {
                cli_out("Unsupported size %s, only (80-640) allowed", dev);
                soc_sbx_caladan3_tcam_diag_entry_width = 80;
                return (CMD_FAIL);
            }
        } else {
            cli_out("Current TCAM access entry size is %d\n", 
                    soc_sbx_caladan3_tcam_diag_entry_width);
            cli_out("this is used for all raw TCAM access\n");
        }
        return (CMD_OK);
    } else if ((sal_strcasecmp(subcmd, "mdio") == 0) || (subcmd[0]=='M')) {
        dev = ARG_GET(a);
        devid = parse_integer(dev);
        if (devid > 31) {
            cli_out("Invalid devid :%s", dev);
            return CMD_FAIL;
        }
        reg = ARG_GET(a);
        mdio_reg = parse_integer(reg);
        if (mdio_reg >= 0xffff) {
            cli_out("Invalid register :%s\n", reg);
            return CMD_FAIL;
        }
        val = ARG_GET(a);
        if (val) {
            mdio_regval = parse_integer(val);
            if (mdio_regval >= 0xffff) {
                cli_out("Invalid value :%s\n", val);
                return CMD_FAIL;
            }
            op = 0;
        } else {
            op = 1;
        }
        /* accessing only device 0, extend to cascaded devices in future */
        p = soc_sbx_caladan3_mdio_portid_get(unit, portid);
        rv =soc_etu_nk_mdio_register_access(unit, op, p, devid, mdio_reg, &mdio_regval);
        if (SOC_FAILURE(rv)) {
            cli_out("Failed to %s register 0x%04x", (op ? "Read" : "Write"), regaddr);
            return (CMD_FAIL);
        }
        cli_out("Addr: 0x%04x Data: 0x%04x\n", mdio_reg, mdio_regval);
        return (CMD_OK);
    } else if ((sal_strcasecmp(subcmd, "reg") == 0) || (subcmd[0] == 'R')) {
        reg = ARG_GET(a);
        regaddr = parse_integer(reg);
        if (regaddr >= 0xfffff) {
            /* 20bit space */
            cli_out("Invalid register :%s\n", reg);
            return CMD_FAIL;
        }
        val = ARG_GET(a);
        if (!val) {
            rv = soc_sbx_caladan3_etu_tcam_reg_read(unit, portid, regaddr, value);
            if (SOC_SUCCESS(rv)) {
                cli_out("Register: 0x%04x ", regaddr);
                cmd_print_hex_string("Data: ", value, soc_sbx_caladan3_tcam_diag_entry_width);
                cli_out("\n");
            }
        } else {
            cmd_rv =  cmd_parse_hex_string(val, value, 80);
            if (cmd_rv == CMD_OK) {
                rv = soc_sbx_caladan3_etu_tcam_reg_write(unit, portid, regaddr, value, 0);
                if (SOC_FAILURE(rv)) {
                    cli_out("Tcam reg write failed %d\n", rv);
                    return (CMD_FAIL);
                }
            } else {
                return (CMD_FAIL);
            }
        }
        return (CMD_OK);

    } else if ((sal_strcasecmp(subcmd, "db") == 0) || (subcmd[0] == 'D')) {

        if ((subcmd = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }
        if ((blk_str = ARG_GET(a)) == NULL) {
            cli_out("Missing Blk\n");
            return CMD_USAGE;
        }
        blk = parse_integer(blk_str);
        if ((blk <0) || (blk > 127)) {
            cli_out("Invalid block number :%s\n", blk_str);
            return CMD_FAIL;
        }
        if ((idx_str = ARG_GET(a)) == NULL) {
            cli_out("Missing idx\n");
            return CMD_USAGE;
        }
        idx = parse_integer(idx_str);
        if ((idx <0) || (idx > 4095)) {
            cli_out("Invalid Entry number :%s\n", idx_str);
            return CMD_FAIL;
        }
        if ((sal_strcasecmp(subcmd, "read") == 0) || (subcmd[0] == 'R')) {
            rd_mode_str = ARG_GET(a);
            if (rd_mode_str) {
                rd_mode = parse_integer(rd_mode_str);
                if ((rd_mode != 1) && (rd_mode != 0)) {
                    cli_out("Invalid Value for rd_mode :%s", rd_mode_str);
                    return CMD_FAIL;
                }
            } else {
		/* default to X/Y mode */
                rd_mode = 1;
            }
            sal_memset(mask, 0, sizeof(mask));
            sal_memset(data, 0, sizeof(data));
            sal_memset(x, 0, sizeof(x));
            sal_memset(y, 0, sizeof(y));
            rv = soc_sbx_caladan3_etu_tcam_db_read(unit, portid, 
                                                  ((blk << 12) | (idx)), 
                                                   x, y, &valid); 
            if (SOC_SUCCESS(rv)) {
		if (rd_mode == 0) {
		    /* convert to data/mask mode */
		    for(idx = 0; idx < soc_sbx_caladan3_tcam_diag_entry_width/8; idx++)
		    {
			mask[idx] = ~(x[idx] ^ y[idx]);
			data[idx] = x[idx];
		    }
		} else {
		    for(idx = 0; idx < soc_sbx_caladan3_tcam_diag_entry_width/8; idx++)
		    {
			data[idx] = x[idx];
			mask[idx] = y[idx];
		    }
		}
		cmd_print_hex_string("Data: ", data, soc_sbx_caladan3_tcam_diag_entry_width);
		cmd_print_hex_string(" Mask: ", mask, soc_sbx_caladan3_tcam_diag_entry_width);
		cli_out(" Valid: %d\n", valid);
		return (CMD_OK);
            } else {
               cli_out("Failed on DB read %d", rv);
               return (CMD_FAIL);
            }
        } else if ((sal_strcasecmp(subcmd, "write") == 0) || (subcmd[0] == 'W')) {
            data_str = ARG_GET(a);
            if (!data_str) {
                cli_out("Missing Data \n");
                return (CMD_USAGE);
            }
            cmd_rv =  cmd_parse_hex_string(data_str, data, soc_sbx_caladan3_tcam_diag_entry_width);
            if (cmd_rv != CMD_OK) {
                cli_out("Failed parsing data\n");
                return (cmd_rv);
            }
            mask_str = ARG_GET(a);
            if (mask_str) {
                cmd_rv =  cmd_parse_hex_string(mask_str, mask, soc_sbx_caladan3_tcam_diag_entry_width);
                if (cmd_rv != CMD_OK) {
                    cli_out("Failed parsing mask\n");
                    return (cmd_rv);
                }
            } else {
                sal_memset(mask, 0xff, sizeof(mask));
            }
            valid_str = ARG_GET(a);
            if (valid_str) {
                valid = parse_integer(valid_str);
                if ((valid != 1) && (valid != 0)) {
                    cli_out("Invalid Value for valid :%s", valid_str);
                    return CMD_FAIL;
                }
            } else {
                valid = 1;
            }
            wr_mode_str = ARG_GET(a);
            if (wr_mode_str) {
                wr_mode = parse_integer(wr_mode_str);
                if ((wr_mode != 1) && (wr_mode != 0)) {
                    cli_out("Invalid Value for wr_mode :%s", wr_mode_str);
                    return CMD_FAIL;
                }
            } else {
                wr_mode = 1;
            }
            rv = soc_sbx_caladan3_etu_tcam_db_write(unit, portid, 
                                                   ((blk << 12) | (idx)), 
						    data, mask, valid, wr_mode, 0);
            if (SOC_SUCCESS(rv)) {
                return (CMD_OK);
            } else {
                return (CMD_FAIL);
            }
        } else if ((sal_strcasecmp(subcmd, "lookup") == 0) || (subcmd[0] == 'L')) {
            data_str = ARG_GET(a);
            if (!data_str) {
                cli_out("Missing Data \n");
                return (CMD_USAGE);
            }
            cmd_rv =  cmd_parse_hex_string(data_str, data, soc_sbx_caladan3_tcam_diag_entry_width);
            if (cmd_rv != CMD_OK) {
                cli_out("Failed parsing data\n");
                return (cmd_rv);
            }
            sal_memset(&lkup, 0, sizeof(etu_tcam_lookup_t));
            sal_memset(&res[0], 0, sizeof(res));
            lkup.ltr = 0;
            lkup.key = data;
            rv = soc_sbx_caladan3_etu_tcam_db_lookup(unit, &lkup, &res[0]);
            if (SOC_SUCCESS(rv)) {
                for (i=0; i < 4; i++) {
                    cli_out(" Valid: %d\n", res[i].valid);
                    cli_out(" H/M: %d\n", res[i].hit);
                    cli_out(" Result: %d\n", res[i].result);
                }
                return (CMD_OK);
            } else {
                cli_out("Failed on DB Lookup %d", rv);
                return (CMD_FAIL);
            }
        } else if ((sal_strcasecmp(subcmd, "dump") == 0) || (subcmd[0] == 'D')) {

            cnt_str = ARG_GET(a);
            if (!cnt_str) {
                cli_out("Missing Count \n");
                return (CMD_USAGE);
            }
            cnt = parse_integer(cnt_str);
            if ((cnt <0) || (cnt > 4096)) {
                cli_out("Invalid Entry number :%s\n", cnt_str);
                return CMD_FAIL;
            }
            for (max = idx+cnt, i=idx; i < max;  i++) {
                sal_memset(mask, 0, sizeof(mask));
                sal_memset(data, 0, sizeof(data));
                if (i > 4095) { blk++; max -= i; i=0; }
                rv = soc_sbx_caladan3_etu_tcam_db_read(unit, portid, 
                                                      ((blk << 12) | (i)), 
                                                       data, mask, &valid); 
                if (SOC_SUCCESS(rv)) {
                   cmd_print_hex_string("Data: ", data, soc_sbx_caladan3_tcam_diag_entry_width);
                   cmd_print_hex_string(" Mask: ", mask, soc_sbx_caladan3_tcam_diag_entry_width);
                   cli_out(" Valid: %d\n", valid);
                } else {
                   cli_out("Failed on DB read %d\n", rv);
                   return (CMD_FAIL);
                }
            }
            return (CMD_OK);
        }
    } else if (sal_strcasecmp(subcmd, "prbs") == 0) {
        char *str = NULL;
        uint32 seed = 0x0, poly = 0;
        uint32 enable = 0;
        uint16 cmd = 0;
        uint32 status;
        uint16 max_lane = 0, num_tcams = 0, nl_dev_id = 0, lane = 0;
        uint16 crx_lanes = 0, ctx_lanes = 0, mdio_portid = 0, prbs_pass = 1;
        enum { _PHY_PRBS_SET_CMD, _PHY_PRBS_GET_CMD, _PHY_PRBS_CLEAR_CMD };

        if ((str = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }

        if (sal_strcasecmp(str, "set") == 0) {
            cmd = _PHY_PRBS_SET_CMD;
            enable = 1;
        } else if (sal_strcasecmp(str, "get") == 0) {
            cmd = _PHY_PRBS_GET_CMD;
            enable = 0;
        } else if (sal_strcasecmp(str, "clear") == 0) {
            cmd = _PHY_PRBS_CLEAR_CMD;
            enable = 0;
        } else return CMD_USAGE;

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "poly", PQ_DFL|PQ_INT,
                (void *)(0), &poly, NULL);
        parse_table_add(&pt, "seed", PQ_DFL|PQ_INT,
                (void *)(0), &seed, NULL);

        if (parse_arg_eq(a, &pt) < 0) {
            cli_out("Error: invalid option: %s\n", ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
        }

        /* Now free allocated strings */
        parse_arg_eq_done(&pt);

        num_tcams = 1; /*((soc_tcam_info_t *)SOC_CONTROL(unit)->tcam_info)->num_tcams;*/
        max_lane = 23;

        
        if(num_tcams >1) {
	  /* coverity[dead_error_line] */
            max_lane = 15;
        }

        if((cmd == _PHY_PRBS_SET_CMD) || 
                (cmd == _PHY_PRBS_CLEAR_CMD)) {

            /* Enable prbs generation from TR3 */ 
            for(lane=0; lane <= max_lane; ++lane) {
                if (poly <= 3) {
                    rv = wcmod_esm_serdes_control_set(unit, 
                            lane, 
                            SOC_PHY_CONTROL_PRBS_POLYNOMIAL,   
                            &poly);
                    if( rv != BCM_E_NONE) {
                        cli_out("Setting prbs polynomial failed on Warpcore\n");
                        return CMD_FAIL;
                    }
                    /* Tx enable */
                    rv = wcmod_esm_serdes_control_set(unit, 
                            lane, 
                            SOC_PHY_CONTROL_PRBS_TX_ENABLE,   
                            &enable);
                    if( rv != BCM_E_NONE) {
                        cli_out("Setting prbs tx enable failed\n");
                        return CMD_FAIL;
                    }
                } else {
                    cli_out("Polynomial must be between 0 & 3, inclusive.\n");
                    return CMD_FAIL;
                }

            }
            /* Enable prbs checkers on all the tcams */
            for(nl_dev_id=0; nl_dev_id < num_tcams; ++nl_dev_id) {
                mdio_portid = soc_sbx_caladan3_mdio_portid_get(unit, nl_dev_id);
                /* CRX lanes active for all tcams except the first
                   TCAM in the chain directly connected to request \
                   line(tx) of serdes (nl_dev_id = 0)*/
                crx_lanes = nl_dev_id? 1 : 0;
                rv = c3_nl_mdio_prbs_chk(unit, mdio_portid, poly, seed, crx_lanes, enable);
                if(rv < 0) {
                    cli_out("Enabling prbs checker on tcam(mdio_portid %d) failed \n",mdio_portid);
                    return CMD_FAIL;
                }
            }

            /* Enable prbs generators in NL */
            for(nl_dev_id=0; nl_dev_id < num_tcams; ++nl_dev_id) {
                mdio_portid = soc_sbx_caladan3_mdio_portid_get(unit, nl_dev_id);
                /* CTX lanes active for all tcams except the last
                   TCAM in the chain directly connected to response 
                   line(rx) of serdes */
		/* coverity[dead_error_line] */
                ctx_lanes = (nl_dev_id == (num_tcams-1))? 0 : 1;
                rv = c3_nl_mdio_prbs_gen(unit, mdio_portid, poly, seed, ctx_lanes, enable);
                if(rv < 0) {
                    cli_out("Enabling prbs generator on tcam(mdio_portid %d) failed \n",mdio_portid);
                    return CMD_FAIL;
                }
            }


            max_lane = 23; 
            /* Configure prbs checkers in TR3 for upper 12 rx lanes */
            for(lane=0; lane <= max_lane; ++lane) {
                if (poly <= 3) {
                    rv = wcmod_esm_serdes_control_set(unit, 
                            lane, 
                            SOC_PHY_CONTROL_PRBS_POLYNOMIAL,   
                            &poly);
                    if( rv != BCM_E_NONE) {
                        cli_out("Enabling prbs checker on WC failed\n");
                        return CMD_FAIL;
                    }
                    /* Tx enable */
                    rv = wcmod_esm_serdes_control_set(unit, 
                            lane, 
                            SOC_PHY_CONTROL_PRBS_TX_ENABLE,   
                            &enable);
                    if( rv != BCM_E_NONE) {
                        cli_out("Setting prbs tx enable failed\n");
                        return CMD_FAIL;
                    }
                } else {
                    cli_out("Polynomial must be between 0 & 3, inclusive.\n");        
                    return CMD_FAIL;
                }
            }
        } else if(cmd == _PHY_PRBS_GET_CMD) {
            /* Check for errors in NL prbs checkers, TR3 prbs checkers */
            for(nl_dev_id=0; nl_dev_id < num_tcams; ++nl_dev_id) {
                mdio_portid = soc_sbx_caladan3_mdio_portid_get(unit, nl_dev_id);
                /* CRX lanes active for all tcams except the first
                   TCAM in the chain directly connected to request \
                   line(tx) of serdes (nl_dev_id = 0)*/
		/* coverity[dead_error_line] */
                crx_lanes = nl_dev_id? 1 : 0;
                cli_out("CHECKING PRBS: NL Tcam (%d)\n", mdio_portid);
                rv = c3_nl_mdio_prbs_chk_err(unit, mdio_portid, crx_lanes);
                if(rv < 0) {
                    cli_out("Getting prbs status on tcam(mdio_portid %d) failed \n",mdio_portid);
                    return CMD_FAIL;
                }

            }

            /* Check for errors in C3 prbs checkers */
            cli_out("CHECKING PRBS: C3 Warpcore Serdes\n");
            max_lane = 11;
            for(lane=0; lane <= max_lane; ++lane) {
                rv = wcmod_esm_serdes_control_get(unit, 
                        lane, 
                        SOC_PHY_CONTROL_PRBS_RX_STATUS,   
                        &status);
                if( rv != BCM_E_NONE) {
                    cli_out("WC: Getting prbs status failed (lane:%d) \n", lane);
                    return CMD_FAIL;
                }
                switch (status) {
                    case 0:
                        /*cli_out("(%d):  WC PRBS OK!\n", lane);*/
                        break;
                    case -1:
                        cli_out("\t->WC PRBS Failed! (%d)\n", lane);
                        prbs_pass = 0;
                        break;
                    default:
                        cli_out("\t->WC PRBS has %d errors! (%d)\n", lane, status);
                        prbs_pass = 0;
                        break;
                }
            }         
            if(prbs_pass) {
                cli_out("\t->PRBS OK\n");
            }
        }


    } else {
        cli_out("Unknown command: %s\n", subcmd);
        return CMD_FAIL;
    }

    return CMD_OK;
}





char cmd_sbx_caladan3_taps_configure_usage[] =     \
    "Tap Configuration:\n"                       \
    "Taps em=<0|1>  ->If set,use taps for EM search, otherwise for LPM search";
    

cmd_result_t
cmd_sbx_caladan3_taps_configure(int unit, args_t *a)
{
    char *subcmd, *enable_em;
    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    if (sal_strcasecmp(subcmd, "em") == 0) {
        enable_em = ARG_GET(a);
        taps_used_as_em = parse_integer(enable_em);
    } else {
        return CMD_USAGE;
    }
    return CMD_OK;
}

cmd_result_t
sbx_caladan3_squeue_flow_control_status(int unit, int squeue)
{
    uint32 regval;
    uint32 q_offset;
    
    if (squeue >= 128) {
        cli_out("squeue(%d) out of range, squeue must be < 128\n", squeue);
        return CMD_FAIL; 
    }
    
    if (squeue < 32) {
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, QM_FC_STATUS0r, 0, 0, &regval));
    } else if (squeue < 64) {
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, QM_FC_STATUS1r, 0, 0, &regval));
    } else if (squeue < 96) {
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, QM_FC_STATUS2r, 0, 0, &regval));
    } else {
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, QM_FC_STATUS3r, 0, 0, &regval));
    }
    q_offset = squeue%32;
    
    cli_out("QM_FC_STATUS xoff state:");

    if (regval & (1 << q_offset)) {
        cli_out("    q(%02d) XOFF\n", squeue);
    } else {
        cli_out("    q(%02d) XON\n", squeue);
    }
    
    return CMD_OK;
}

char cmd_sbx_caladan3_flow_control_status_usage[] = "\n"
    " Dumps flow control status of some blocks\n" \
    " [squeue=] dump for a particular squeue 0-127\n" ;

cmd_result_t
cmd_sbx_caladan3_flow_control_status(int unit, args_t *args)
{
    pbmp_t eth_pbm, il_pbm;
    soc_port_t port;
    uint32 regval;
    int block, ifnum;
    uint64 reg64;
    uint32 regval_lo, regval_hi;
    int i;
    int newline = 0;
    int first = TRUE;
    int squeue = -1;
    parse_table_t parsetable;
    int ret_code;
    uint32 pt;
    uint32 state;
    
    if (ARG_CNT(args)) {
        parse_table_init(0,&parsetable);
        parse_table_add(&parsetable,"squeue",PQ_INT, 0, &squeue, NULL);
        
        if (!parseEndOk(args,&parsetable,&ret_code)) {
            return ret_code;
        }
    }

    if (squeue != -1) {
        ret_code = sbx_caladan3_squeue_flow_control_status(unit, squeue);
        return ret_code;
    }
    
    SOC_PBMP_ASSIGN(eth_pbm,PBMP_E_ALL(unit));

    BCM_PBMP_ITER(eth_pbm, port) {
        SOC_IF_ERROR_RETURN(READ_TXPFr(unit, port, &reg64));
        reg64 = soc_reg64_field_get(unit, TXPFr, reg64, COUNTf);        
        regval_lo = COMPILER_64_LO(reg64);
        regval_hi = COMPILER_64_HI(reg64);
        if ((regval_lo != 0) || (regval_hi !=0)) {
            if (first == TRUE) {
                cli_out("   TRANSMIT FLOW CONTROL STATUS ETHERNET\n");
                cli_out("*********************************************\n");
                first = FALSE;
            }
            cli_out("port(%02d) tx pause lo (TXPF) 0x%08x\n", port, regval_lo);
            cli_out("port(%02d) tx pause hi (TXPF) 0x%08x\n", port, regval_hi);
            SOC_IF_ERROR_RETURN(WRITE_TXPFr(unit, port, reg64));
        }
    }
    
    first = TRUE;
    BCM_PBMP_ASSIGN(il_pbm,PBMP_IL_ALL(unit));
    BCM_PBMP_ITER(PBMP_HG_ALL(unit), port) {
        soc_sbx_caladan3_flow_control_type_t mode;
        soc_sbx_caladan3_port_flow_control_mode_get(unit, port, &mode);
        if (mode == SOC_SBX_CALADAN3_FC_TYPE_ILKN_OOB) {
            BCM_PBMP_PORT_ADD(il_pbm, port);
            /* One port is enough to access the il block */
            break;
        }
    }
    BCM_PBMP_ITER(il_pbm, port) {
        ifnum = soc_sbx_caladan3_is_line_port(unit, port)? 0:1;
        block = SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(ifnum);
        regval = 0xffffffff;
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_HCFC_TX_STATUS0r, block, 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_HCFC_TX_STATUS0r, block, 0, &regval));
        if (regval != 0) {
            if (first == TRUE) {
                cli_out("\n*********************************************\n");
                cli_out("  %s TRANSMIT FLOW CONTROL STATUS HCFC\n", (ifnum) ? "FABRIC SIDE" : "LINE SIDE");
                cli_out("*********************************************\n");
                first = FALSE;
            }
            cli_out("port(%02d) IL_HCFC_TX_STATUS0 0x%08x\n", port, regval);
            for (i=0, newline=0; i<32; i++) {
                if (regval & (1 << i)) {
                    if ((newline++ % 4) == 3) {
                        cli_out("ch(%02d) XOFF\n", i);
                    } else {
                        cli_out("ch(%02d) XOFF  ", i);
                    }
                }
            }
            cli_out("\n");
        }
        
        regval = 0xffffffff;
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_HCFC_TX_STATUS1r, block, 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_HCFC_TX_STATUS1r, block, 0, &regval));
        if (regval != 0) {
            if (first == TRUE) {
                cli_out("\n*********************************************\n");
                cli_out("  %s TRANSMIT FLOW CONTROL STATUS HCFC\n", (ifnum) ? "FABRIC SIDE" : "LINE SIDE");
                cli_out("*********************************************\n");
                first = FALSE;
            }
            cli_out("port(%02d) IL_HCFC_TX_STATUS1 0x%08x\n", port, regval);
            for (i=0, newline=0; i<32; i++) {
                if (regval & (1 << i)) {
                    if ((newline++ % 4) == 3) {
                        cli_out("ch(%02d) XOFF\n", i+32);
                    } else {
                        cli_out("ch(%02d) XOFF  ", i+32);
                    }
                }
            }
            cli_out("\n");
        }
    }
    first = TRUE;
    BCM_PBMP_ITER(il_pbm, port) {
        ifnum = soc_sbx_caladan3_is_line_port(unit, port)? 0:1;
        block = SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(ifnum);
        /* Clear and read status fresh */
        regval = 0xffffffff;
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_FLOWCONTROL_TXFC_STS0r, block, 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_FLOWCONTROL_TXFC_STS0r, block, 0, &regval));
        if (regval != 0) {
            if (first == TRUE) {
                cli_out("\n*********************************************\n");
                cli_out("  %s TRANSMIT FLOW CONTROL STATUS ILKN OOB \n", (ifnum) ? "FABRIC SIDE" : "LINE SIDE");
                cli_out("*********************************************\n");
                first = FALSE;
            }
            cli_out("port(%02d) IL_FLOWCONTROL_TXFC_STS0 0x%08x\n", port, regval);
            for (i=0, newline=0; i<32; i++) {
                if (regval & (1 << i)) {
                    if ((newline++ % 4) == 3) {
                        cli_out("ch(%02d) XOFF\n", i);
                    } else {
                        cli_out("ch(%02d) XOFF  ", i);
                    }
                }
            }
            cli_out("\n");
        }

        /* Clear and read status fresh */
        regval = 0xffffffff;
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_FLOWCONTROL_TXFC_STS1r, block, 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_FLOWCONTROL_TXFC_STS1r, block, 0, &regval));
        if (regval != 0) {
            if (first == TRUE) {
                cli_out("\n*********************************************\n");
                cli_out("  %s TRANSMIT FLOW CONTROL STATUS ILKN OOB \n", (ifnum) ? "FABRIC SIDE" : "LINE SIDE");
                cli_out("*********************************************\n");
                first = FALSE;
            }
            cli_out("port(%02d) IL_FLOWCONTROL_TXFC_STS1 0x%08x\n", port, regval);
            for (i=0, newline=0; i<32; i++) {
                if (regval & (1 << i)) {
                    if ((newline++ % 4) == 3) {
                        cli_out("ch(%02d) XOFF\n", i+32);
                    } else {
                        cli_out("ch(%02d) XOFF  ", i+32);
                    }
                }
            }
            cli_out("\n");
        }
    }
    
    first = TRUE;
    pt = 0;
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_FLOW_CONTROL_DEBUG0r, pt, 0, &regval));
    if (regval != 0) {
        if (first == TRUE) {
            cli_out("\n*********************************************\n");
            cli_out("   PT0 CHANNELIZED FLOW CONTROL STATUS\n");
            cli_out("*********************************************\n");
            first = FALSE;
        }
        cli_out("PT_IPTE_FLOW_CONTROL_DEBUG0 CHANNEL 31:00 0x%08x\n", regval);
        for (i=0, newline=0; i<32; i++) {
            if (regval & (1 << i)) {
                if ((newline++ % 4) == 3) {
                    cli_out("ch(%02d) XOFF\n", i);
                } else {
                    cli_out("ch(%02d) XOFF  ", i);
                }
            }
        }
        cli_out("\n");
    }
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_FLOW_CONTROL_DEBUG1r, pt, 0, &regval));
    if (regval != 0) {
        if (first == TRUE) {
            cli_out("\n*********************************************\n");
            cli_out("   PT0 CHANNELIZED FLOW CONTROL STATUS\n");
            cli_out("*********************************************\n");
            first = FALSE;
        }
        cli_out("PT_IPTE_FLOW_CONTROL_DEBUG1 CHANNEL 63:32 0x%08x\n", regval);
        for (i=0, newline=0; i<32; i++) {
            if (regval & (1 << i)) {
                if ((newline++ % 4) == 3) {
                    cli_out("ch(%02d) XOFF\n", i+32);
                } else {
                    cli_out("ch(%02d) XOFF  ", i+32);
                }
            }
        }
        cli_out("\n");
    }
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_FLOW_CONTROL_DEBUG4r, pt, 0, &regval));
    if (regval != 0) {
        if (first == TRUE) {
            cli_out("\n*********************************************\n");
            cli_out("   PT0 QUEUE FLOW CONTROL STATUS\n");
            cli_out("*********************************************\n");
            first = FALSE;
        }
        cli_out("PT_IPTE_FLOW_CONTROL_DEBUG4 QUEUE   31:00 0x%08x\n", regval);
        for (i=0, newline=0; i<32; i++) {
            if (regval & (1 << i)) {
                if ((newline++ % 4) == 3) {
                    cli_out("q(%02d) XOFF\n", i);
                } else {
                    cli_out("q(%02d) XOFF  ", i);
                }
            }
        }
        cli_out("\n");
    }
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_FLOW_CONTROL_DEBUG5r, pt, 0, &regval));
    if (regval != 0) {
        if (first == TRUE) {
            cli_out("\n*********************************************\n");
            cli_out("   PT0 QUEUE FLOW CONTROL STATUS\n");
            cli_out("*********************************************\n");
            first = FALSE;
        }
        cli_out("PT_IPTE_FLOW_CONTROL_DEBUG5 QUEUE   63:32 0x%08x\n", regval);
        for (i=0, newline=0; i<32; i++) {
            if (regval & (1 << i)) {
                if ((newline++ % 4) == 3) {
                    cli_out("q(%02d) XOFF\n", i+32);
                } else {
                    cli_out("q(%02d) XOFF  ", i+32);
                }
            }
        }
        cli_out("\n");
    }
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_FLOW_CONTROL_DEBUG6r, pt, 0, &regval));
    if (regval != 0) {
        if (first == TRUE) {
            cli_out("\n*********************************************\n");
            cli_out("   PT0 FLOW CONTROL STATUS STICKY STATE\n");
            cli_out("*********************************************\n");
            first = FALSE;
        }
        cli_out("PT_IPTE_FLOW_CONTROL_DEBUG6 STICKY  31:00 0x%08x\n", regval);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_FLOW_CONTROL_DEBUG6r, pt, 0, regval));
    }
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_FLOW_CONTROL_DEBUG7r, pt, 0, &regval));
    if (regval != 0) {
        if (first == TRUE) {
            cli_out("\n*********************************************\n");
            cli_out("   PT0 FLOW CONTROL STATUS\n");
            cli_out("*********************************************\n");
            first = FALSE;
        }
        cli_out("PT_IPTE_FLOW_CONTROL_DEBUG7 STICKY  63:32 0x%08x\n", regval);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_FLOW_CONTROL_DEBUG7r, pt, 0, regval));
    }
    
    first = TRUE;
    pt = 1;
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_FLOW_CONTROL_DEBUG0r, pt, 0, &regval));
    if (regval != 0) {
        if (first == TRUE) {
            cli_out("\n*********************************************\n");
            cli_out("   PT1 FLOW CONTROL STATUS\n");
            cli_out("*********************************************\n");
            first = FALSE;
        }
        cli_out("PT_IPTE_FLOW_CONTROL_DEBUG0 CHANNEL 31:00 0x%08x\n", regval);
        for (i=0, newline=0; i<32; i++) {
            if (regval & (1 << i)) {
                if ((newline++ % 4) == 3) {
                    cli_out("ch(%02d) XOFF\n", i);
                } else {
                    cli_out("ch(%02d) XOFF  ", i);
                }
            }
        }
        cli_out("\n");
    }
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_FLOW_CONTROL_DEBUG1r, pt, 0, &regval));
    if (regval != 0) {
        if (first == TRUE) {
            cli_out("\n*********************************************\n");
            cli_out("   PT1 FLOW CONTROL STATUS\n");
            cli_out("*********************************************\n");
            first = FALSE;
        }
        cli_out("PT_IPTE_FLOW_CONTROL_DEBUG1 CHANNEL 63:32 0x%08x\n", regval);
        for (i=0, newline=0; i<32; i++) {
            if (regval & (1 << i)) {
                if ((newline++ % 4) == 3) {
                    cli_out("ch(%02d) XOFF\n", i+32);
                } else {
                    cli_out("ch(%02d) XOFF  ", i+32);
                }
            }
        }
        cli_out("\n");
    }
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_FLOW_CONTROL_DEBUG4r, pt, 0, &regval));
    if (regval != 0) {
        if (first == TRUE) {
            cli_out("\n*********************************************\n");
            cli_out("   PT1 FLOW CONTROL STATUS\n");
            cli_out("*********************************************\n");
            first = FALSE;
        }
        cli_out("PT_IPTE_FLOW_CONTROL_DEBUG4 QUEUE   31:00 0x%08x\n", regval);
        for (i=0, newline=0; i<32; i++) {
            if (regval & (1 << i)) {
                if ((newline++ % 4) == 3) {
                    cli_out("q(%02d) XOFF\n", i);
                } else {
                    cli_out("q(%02d) XOFF  ", i);
                }
            }
        }
        cli_out("\n");
    }
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_FLOW_CONTROL_DEBUG5r, pt, 0, &regval));
    if (regval != 0) {
        if (first == TRUE) {
            cli_out("\n*********************************************\n");
            cli_out("   PT1 FLOW CONTROL STATUS\n");
            cli_out("*********************************************\n");
            first = FALSE;
        }
        cli_out("PT_IPTE_FLOW_CONTROL_DEBUG5 QUEUE   63:32 0x%08x\n", regval);
        for (i=0, newline=0; i<32; i++) {
            if (regval & (1 << i)) {
                if ((newline++ % 4) == 3) {
                    cli_out("q(%02d) XOFF\n", i+32);
                } else {
                    cli_out("q(%02d) XOFF  ", i+32);
                }
            }
        }
        cli_out("\n");
    }
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_FLOW_CONTROL_DEBUG6r, pt, 0, &regval));
    if (regval != 0) {
        if (first == TRUE) {
            cli_out("\n*********************************************\n");
            cli_out("   PT1 FLOW CONTROL STATUS\n");
            cli_out("*********************************************\n");
            first = FALSE;
        }
        cli_out("PT_IPTE_FLOW_CONTROL_DEBUG6 STICKY  31:0 0x%08x\n", regval);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_FLOW_CONTROL_DEBUG6r, pt, 0, regval));
    }
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, PT_IPTE_FLOW_CONTROL_DEBUG7r, pt, 0, &regval));
    if (regval != 0) {
        if (first == TRUE) {
            cli_out("\n*********************************************\n");
            cli_out("   PT1 FLOW CONTROL STATUS\n");
            cli_out("*********************************************\n");
            first = FALSE;
        }
        cli_out("PT_IPTE_FLOW_CONTROL_DEBUG7 STICKY  63:32 0x%08x\n", regval);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, PT_IPTE_FLOW_CONTROL_DEBUG7r, pt, 0, regval));
    }

    first = TRUE;
    BCM_PBMP_ITER(eth_pbm, port) {
        SOC_IF_ERROR_RETURN(READ_RXPFr(unit, port, &reg64));
        reg64 = soc_reg64_field_get(unit, RXPFr, reg64, COUNTf);        
        regval_lo = COMPILER_64_LO(reg64);
        regval_hi = COMPILER_64_HI(reg64);
        if ((regval_lo != 0) || (regval_hi != 0)){
            if (first == TRUE) {
            cli_out("\n*********************************************\n");
                cli_out("   RECEIVE FLOW CONTROL STATUS ETHERNET\n");
                cli_out("*********************************************\n");
                first = FALSE;
            }
            cli_out("port(%02d) rx pause lo (RXPF) 0x%08x\n", port, regval_lo);
            cli_out("port(%02d) tx pause hi (TXPF) 0x%08x\n", port, regval_hi);
            SOC_IF_ERROR_RETURN(WRITE_RXPFr(unit, port, reg64));
        }
    }

    first = TRUE;
    BCM_PBMP_ITER(il_pbm, port) {
        ifnum = soc_sbx_caladan3_is_line_port(unit, port)? 0:1;
        block = SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(ifnum);
        regval = 0xffffffff;
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_HCFC_RX_STATUS0r, block, 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_HCFC_RX_STATUS0r, block, 0, &regval));
        if (regval != 0) {
            if (first == TRUE) {
            cli_out("\n*********************************************\n");
                cli_out("  %s RECEIVE FLOW CONTROL STATUS HCFC \n", (ifnum) ? "FABRIC SIDE" : "LINE SIDE");
                cli_out("*********************************************\n");
                first = FALSE;
            }
            cli_out("port(%02d) IL_HCFC_RX_STATUS0 0x%08x\n", port, regval);
            for (i=0, newline=0; i<32; i++) {
                if (regval & (1 << i)) {
                    if ((newline++ % 4) == 3) {
                        cli_out("ch(%02d) XOFF\n", i);
                    } else {
                        cli_out("ch(%02d) XOFF  ", i);
                    }
                }
            }
            cli_out("\n");
        }
        regval = 0xffffffff;
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_HCFC_RX_STATUS0r, block, 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_HCFC_RX_STATUS1r, block, 0, &regval));
        if (regval != 0) {
            if (first == TRUE) {
            cli_out("\n*********************************************\n");
                cli_out("  %s RECEIVE FLOW CONTROL STATUS HCFC \n", (ifnum) ? "FABRIC SIDE" : "LINE SIDE");
                cli_out("*********************************************\n");
                first = FALSE;
            }
            cli_out("port(%02d) IL_HCFC_RX_STATUS1 0x%08x\n", port, regval);
            for (i=0, newline=0; i<32; i++) {
                if (regval & (1 << i)) {
                    if ((newline++ % 4) == 3) {
                        cli_out("ch(%02d) XOFF\n", i+32);
                    } else {
                        cli_out("ch(%02d) XOFF  ", i+32);
                    }
                }
            }
            cli_out("\n");
        }
    }
    
    first = TRUE;
    BCM_PBMP_ITER(il_pbm, port) {
        ifnum = soc_sbx_caladan3_is_line_port(unit, port)? 0:1;
        block = SOC_SBX_CALADAN3_REG_BLOCK_INSTANCE(ifnum);
        /* Clear and read status fresh */
        regval = 0xffffffff;
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_FLOWCONTROL_RXFC_STS0r, block, 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_FLOWCONTROL_RXFC_STS0r, block, 0, &regval));
        if (regval != 0) {
            if (first == TRUE) {
                cli_out("\n*********************************************\n");
                cli_out("  %s RECEIVE FLOW CONTROL STATUS ILKN OOB\n", (ifnum) ? "FABRIC SIDE" : "LINE SIDE");
                cli_out("*********************************************\n");
                first = FALSE;
            }
            cli_out("port(%02d) IL_FLOWCONTROL_RXFC_STS0 0x%08x\n", port, regval);
            for (i=0, newline=0; i<32; i++) {
                if (regval & (1 << i)) {
                    if ((newline++ % 4) == 3) {
                        cli_out("ch(%02d) XOFF\n", i);
                    } else {
                        cli_out("ch(%02d) XOFF  ", i);
                    }
                }
            }
            cli_out("\n");
        }
        /* Clear and read status fresh */
        regval = 0xffffffff;
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, IL_FLOWCONTROL_RXFC_STS1r, block, 0, regval));
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, IL_FLOWCONTROL_RXFC_STS1r, block, 0, &regval));
        if (regval != 0) {
            if (first == TRUE) {
                cli_out("\n*********************************************\n");
                cli_out("  %s RECEIVE FLOW CONTROL STATUS ILKN OOB\n", (ifnum) ? "FABRIC SIDE" : "LINE SIDE");
                cli_out("*********************************************\n");
                first = FALSE;
            }
            cli_out("port(%02d) IL_FLOWCONTROL_RXFC_STS1 0x%08x\n", port, regval);
            for (i=0, newline=0; i<32; i++) {
                if (regval & (1 << i)) {
                    if ((newline++ % 4) == 3) {
                        cli_out("ch(%02d) XOFF\n", i+32);
                    } else {
                        cli_out("ch(%02d) XOFF  ", i+32);
                    }
                }
            }
            cli_out("\n");
        }
    }

    first = TRUE;
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, QM_FC_STATUS0r, 0, 0, &regval));
    if (regval != 0) {
        if (first == TRUE) {
            cli_out("\n*********************************************\n");
            cli_out("   QM FLOW CONTROL STATUS\n");
            cli_out("*********************************************\n");
            first = FALSE;
        }
        cli_out("QM_FC_STATUS0 xoff state sent to PR queue 31:00  0x%08x\n", regval);
        for (i=0, newline=0; i<32; i++) {
            if (regval & (1 << i)) {
                if ((newline++ % 4) == 3) {
                    cli_out("q(%02d) XOFF\n", i);
                } else {
                    cli_out("q(%02d) XOFF  ", i);
                }
            }
        }
        cli_out("\n");
    }
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, QM_FC_STATUS1r, 0, 0, &regval));
    if (regval != 0) {
        if (first == TRUE) {
            cli_out("\n*********************************************\n");
            cli_out("   QM FLOW CONTROL STATUS\n");
            cli_out("*********************************************\n");
            first = FALSE;
        }
        cli_out("QM_FC_STATUS1 xoff state sent to PR queue 63:32  0x%08x\n", regval);
        for (i=0, newline=0; i<32; i++) {
            if (regval & (1 << i)) {
                if ((newline++ % 4) == 3) {
                    cli_out("q(%02d) XOFF\n", i+32);
                } else {
                    cli_out("q(%02d) XOFF  ", i+32);
                }
            }
        }
        cli_out("\n");
    }
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, QM_FC_STATUS2r, 0, 0, &regval));
    if (regval != 0) {
        if (first == TRUE) {
            cli_out("\n*********************************************\n");
            cli_out("   QM FLOW CONTROL STATUS\n");
            cli_out("*********************************************\n");
            first = FALSE;
        }
        cli_out("QM_FC_STATUS2 xoff state sent to PR queue 95:64  0x%08x\n", regval);
        for (i=0, newline=0; i<32; i++) {
            if (regval & (1 << i)) {
                if ((newline++ % 4) == 3) {
                    cli_out("q(%02d) XOFF\n", i+64);
                } else {
                    cli_out("q(%02d) XOFF  ", i+64);
                }
            }
        }
        cli_out("\n");
    }
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, QM_FC_STATUS3r, 0, 0, &regval));
    if (regval != 0) {
        if (first == TRUE) {
            cli_out("\n*********************************************\n");
            cli_out("   QM FLOW CONTROL STATUS\n");
            cli_out("*********************************************\n");
            first = FALSE;
        }
        cli_out("QM_FC_STATUS3 xoff state sent to PR queue 127:96 0x%08x\n", regval);
        for (i=0, newline=0; i<32; i++) {
            if (regval & (1 << i)) {
                if ((newline++ % 4) == 3) {
                    cli_out("q(%02d) XOFF\n", i+96);
                } else {
                    cli_out("q(%02d) XOFF  ", i+96);
                }
            }
        }
        cli_out("\n");
    }
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, QM_FC_STATUS4r, 0, 0, &regval));
    if (regval != 0) {
        if (first == TRUE) {
            cli_out("\n*********************************************\n");
            cli_out("   QM FLOW CONTROL STATUS\n");
            cli_out("*********************************************\n");
            first = FALSE;
        }
        state = soc_reg_field_get(unit, QM_FC_STATUS4r, regval,  FC_EGRESS_INT_STATEf);
        cli_out(" fc_egress_int_state=  %d\n", state);
        state = soc_reg_field_get(unit, QM_FC_STATUS4r, regval,  FC_EGRESS_STATEf);
        cli_out(" fc_egress_state=      %d\n", state);
        state = soc_reg_field_get(unit, QM_FC_STATUS4r, regval,  FC_INGRESS_INT_STATEf);
        cli_out(" fc_ingress_int_state= %d\n", state);
        state = soc_reg_field_get(unit, QM_FC_STATUS4r, regval,  FC_INGRESS_STATEf);
        cli_out(" fc_ingress_state=     %d\n", state);
        state = soc_reg_field_get(unit, QM_FC_STATUS4r, regval,  FC_TOTAL_BUFFER_STATEf);
        cli_out(" fc_total_buffer_state=%d\n", state);
        
    }

    return CMD_OK;
}

#endif /* BCM_CALADAN3_SUPPORT */
