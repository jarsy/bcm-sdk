#!/bin/bash

from_dir=$1   # BCM SDK source code directory (probably, just current directory)
bcm_to_dir=$2 # BCM SDK artifacts directory in "main" project

to_dir_include=$bcm_to_dir/include
if [ ! -d $to_dir_include ] ; then
    mkdir $to_dir_include
fi

to_dir_x86=$bcm_to_dir/x86
if [ ! -d $to_dir_x86 ] ; then
    mkdir $to_dir_x86
fi

cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/version.o                          $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/platform_defines.o                 $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libdiag.a                          $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libdiag_dpp.a                      $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libdiag_dfe.a                      $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libdiag_dcmn.a                     $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libtest.a                          $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libdiscover.a                      $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libstktask.a                       $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libdiagcint.a                      $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libcint.a                          $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libdiagapi.a                       $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsal_appl_editline.a             $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsal_appl.a                      $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsal_appl_plat.a                 $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/liblubde.a                         $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libbcm.a                           $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libbcm_rpc.a                       $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libbcm_compat.a                    $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libbcm_dpp.a                       $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libbcm_dfe.a                       $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libcputrans.a                      $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libcpudb.a                         $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoccommon.a                     $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_phy.a                       $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_wcmod.a                     $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_tscmod.a                    $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_dpp.a                       $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_mcm.a                       $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_dcmn.a                      $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libbcm_dpp_ppd.a                   $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libbcm_dpp_ppc.a                   $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libbcm_dpp_sand_fm.a               $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libbcm_dpp_sand_mgmt.a             $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libbcm_dpp_sand_utils.a            $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libbcm_dpp_tmc.a                   $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_dpp_arad_nif.a              $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_dpp_arad.a                  $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libbcm_dpp_arad_pp.a               $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libappl_dcmn_rx_los.a              $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libappl_dpp_interrupts.a           $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libappl_dfe_interrupts.a           $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libappl_dcmn_interrupts.a          $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_dfe.a                       $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_dfe_cmn.a                   $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_dfe_fe1600.a                $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libbcm_common.a                    $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_shared.a                    $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libshared.a                        $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_i2c.a                       $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsal_core.a                      $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsal_core_plat.a                 $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libcustomer.a                      $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc.a                           $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_dpp_drc.a                   $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_dpp_port.a                  $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libphymod.a                        $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_portmod.a                   $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libshared_swstate.a                $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_hwstate.a                   $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libdiag_phymod.a                   $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_dfe_fe3200.a                $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_portmod_pms.a               $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libdiag_portmod.a                  $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libappl_dpp_FecAllocaiton.a        $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libshared_swstate_access.a         $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libshared_swstate.a                $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libshared_swstate_layout.a         $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libxml.a                           $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libappl_dpp_FecPerformance.a       $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libappl_dpp_ui.a                   $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libappl_dpp_ui_ppd.a               $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_dpp_jer.a                   $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_dpp_jer_pp.a                $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libshared_dbx.a                    $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libshared_shrextend.a              $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libshared_utilex.a                 $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libdiag_sand.a                     $to_dir_x86
cp -p $from_dir/src/soc/kbp/alg_kbp/lib/unix-user/x86-smp_generic-2_6/libsoc_alg_kbp.a $to_dir_x86
cp -p $from_dir/build/unix-user/x86-smp_generic-2_6/libsoc_alg_kbp_portable.a          $to_dir_x86
