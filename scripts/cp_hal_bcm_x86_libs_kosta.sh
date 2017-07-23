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

art_src=`find $from_dir/build/unix-user/ -type d -name "x86-smp_generic-*"`
if [ -z $art_src ] ; then
	echo "Couldn't locate artifacts source folder"
	exit 1
fi;

cp -p $art_src/version.o                          $to_dir_x86
cp -p $art_src/platform_defines.o                 $to_dir_x86
cp -p $art_src/libdiag.a                          $to_dir_x86
cp -p $art_src/libdiag_dpp.a                      $to_dir_x86
cp -p $art_src/libdiag_dfe.a                      $to_dir_x86
cp -p $art_src/libdiag_dcmn.a                     $to_dir_x86
cp -p $art_src/libtest.a                          $to_dir_x86
cp -p $art_src/libdiscover.a                      $to_dir_x86
cp -p $art_src/libstktask.a                       $to_dir_x86
cp -p $art_src/libdiagcint.a                      $to_dir_x86
cp -p $art_src/libcint.a                          $to_dir_x86
cp -p $art_src/libdiagapi.a                       $to_dir_x86
cp -p $art_src/libsal_appl_editline.a             $to_dir_x86
cp -p $art_src/libsal_appl.a                      $to_dir_x86
cp -p $art_src/libsal_appl_plat.a                 $to_dir_x86
cp -p $art_src/liblubde.a                         $to_dir_x86
cp -p $art_src/libbcm.a                           $to_dir_x86
cp -p $art_src/libbcm_rpc.a                       $to_dir_x86
cp -p $art_src/libbcm_compat.a                    $to_dir_x86
cp -p $art_src/libbcm_dpp.a                       $to_dir_x86
cp -p $art_src/libbcm_dfe.a                       $to_dir_x86
cp -p $art_src/libcputrans.a                      $to_dir_x86
cp -p $art_src/libcpudb.a                         $to_dir_x86
cp -p $art_src/libsoccommon.a                     $to_dir_x86
cp -p $art_src/libsoc_phy.a                       $to_dir_x86
cp -p $art_src/libsoc_wcmod.a                     $to_dir_x86
cp -p $art_src/libsoc_tscmod.a                    $to_dir_x86
cp -p $art_src/libsoc_dpp.a                       $to_dir_x86
cp -p $art_src/libsoc_mcm.a                       $to_dir_x86
cp -p $art_src/libsoc_dcmn.a                      $to_dir_x86
cp -p $art_src/libbcm_dpp_ppd.a                   $to_dir_x86
cp -p $art_src/libbcm_dpp_ppc.a                   $to_dir_x86
cp -p $art_src/libbcm_dpp_sand_fm.a               $to_dir_x86
cp -p $art_src/libbcm_dpp_sand_mgmt.a             $to_dir_x86
cp -p $art_src/libbcm_dpp_sand_utils.a            $to_dir_x86
cp -p $art_src/libbcm_dpp_tmc.a                   $to_dir_x86
cp -p $art_src/libsoc_dpp_arad_nif.a              $to_dir_x86
cp -p $art_src/libsoc_dpp_arad.a                  $to_dir_x86
cp -p $art_src/libbcm_dpp_arad_pp.a               $to_dir_x86
cp -p $art_src/libappl_dcmn_rx_los.a              $to_dir_x86
cp -p $art_src/libappl_dpp_interrupts.a           $to_dir_x86
cp -p $art_src/libappl_dfe_interrupts.a           $to_dir_x86
cp -p $art_src/libappl_dcmn_interrupts.a          $to_dir_x86
cp -p $art_src/libsoc_dfe.a                       $to_dir_x86
cp -p $art_src/libsoc_dfe_cmn.a                   $to_dir_x86
cp -p $art_src/libsoc_dfe_fe1600.a                $to_dir_x86
cp -p $art_src/libbcm_common.a                    $to_dir_x86
cp -p $art_src/libsoc_shared.a                    $to_dir_x86
cp -p $art_src/libshared.a                        $to_dir_x86
cp -p $art_src/libsoc_i2c.a                       $to_dir_x86
cp -p $art_src/libsal_core.a                      $to_dir_x86
cp -p $art_src/libsal_core_plat.a                 $to_dir_x86
cp -p $art_src/libcustomer.a                      $to_dir_x86
cp -p $art_src/libsoc.a                           $to_dir_x86
cp -p $art_src/libsoc_dpp_drc.a                   $to_dir_x86
cp -p $art_src/libsoc_dpp_port.a                  $to_dir_x86
cp -p $art_src/libphymod.a                        $to_dir_x86
cp -p $art_src/libsoc_portmod.a                   $to_dir_x86
cp -p $art_src/libshared_swstate.a                $to_dir_x86
cp -p $art_src/libsoc_hwstate.a                   $to_dir_x86
cp -p $art_src/libdiag_phymod.a                   $to_dir_x86
cp -p $art_src/libsoc_dfe_fe3200.a                $to_dir_x86
cp -p $art_src/libsoc_portmod_pms.a               $to_dir_x86
cp -p $art_src/libdiag_portmod.a                  $to_dir_x86
cp -p $art_src/libappl_dpp_FecAllocaiton.a        $to_dir_x86
cp -p $art_src/libshared_swstate_access.a         $to_dir_x86
cp -p $art_src/libshared_swstate.a                $to_dir_x86
cp -p $art_src/libshared_swstate_layout.a         $to_dir_x86
cp -p $art_src/libxml.a                           $to_dir_x86
cp -p $art_src/libappl_dpp_FecPerformance.a       $to_dir_x86
cp -p $art_src/libappl_dpp_ui.a                   $to_dir_x86
cp -p $art_src/libappl_dpp_ui_ppd.a               $to_dir_x86
cp -p $art_src/libsoc_dpp_jer.a                   $to_dir_x86
cp -p $art_src/libsoc_dpp_jer_pp.a                $to_dir_x86
cp -p $art_src/libshared_dbx.a                    $to_dir_x86
cp -p $art_src/libshared_shrextend.a              $to_dir_x86
cp -p $art_src/libshared_utilex.a                 $to_dir_x86
cp -p $art_src/libdiag_sand.a                     $to_dir_x86
cp -p $from_dir/src/soc/kbp/alg_kbp/lib/unix-user/x86-smp_generic-2_6/libsoc_alg_kbp.a $to_dir_x86
cp -p $art_src/libsoc_alg_kbp_portable.a          $to_dir_x86
