#!/bin/bash

BCM_SDK=$1
DST_DIR=$2


################################################################################
# Check script user-supplied parameters
#
if [ -z $BCM_SDK ] ; then
	echo "Error: Provide BCM SDK directory!"
	echo "Example: $0 ~/My/MRV/OPX/BCM_SDK/Dev/git/bcm_sdk /tmp/bcm_sdk_artifacts"
	exit 1;
fi

if [ -z $DST_DIR ] ; then
	echo "Error: Provide destination directory!"
	echo "Example: $0 ~/My/MRV/OPX/BCM_SDK/Dev/git/bcm_sdk /tmp/bcm_sdk_artifacts"
	exit 1;
fi


################################################################################
# Create directory structure
#
mkdir -p $DST_DIR/include
mkdir -p $DST_DIR/x86

echo "BCM SDK directory:     [$BCM_SDK]"
echo "Destination directory: [$DST_DIR]"
echo "Copying header files from BCM SDK to destination directory..."


################################################################################
# Copy all header files
#
cp --parents -p $BCM_SDK/include/appl/diag/cmdlist.h           $DST_DIR
# cp --parents -p $BCM_SDK/include/appl/diag/debug.h           $DST_DIR
cp --parents -p $BCM_SDK/include/appl/diag/parse.h             $DST_DIR
cp --parents -p $BCM_SDK/include/appl/diag/shell.h             $DST_DIR
cp --parents -p $BCM_SDK/include/appl/diag/sysconf.h           $DST_DIR
cp --parents -p $BCM_SDK/include/appl/diag/system.h            $DST_DIR
cp --parents -p $BCM_SDK/include/appl/diag/warmboot.h          $DST_DIR
cp --parents -p $BCM_SDK/include/assert.h                      $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/ces.h                     $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/error.h                   $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/field.h                   $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/l2.h                      $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/l3.h                      $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/multicast.h               $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/pkt.h                     $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/policer.h                 $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/port.h                    $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/rx.h                      $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/stat.h                    $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/switch.h                  $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/tunnel.h                  $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/tx.h                      $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/types.h                   $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/vlan.h                    $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/udf.h                     $DST_DIR
cp --parents -p $BCM_SDK/include/ibde.h                        $DST_DIR
cp --parents -p $BCM_SDK/include/sal/appl/config.h             $DST_DIR
cp --parents -p $BCM_SDK/include/sal/appl/io.h                 $DST_DIR
cp --parents -p $BCM_SDK/include/sal/appl/sal.h                $DST_DIR
cp --parents -p $BCM_SDK/include/sal/compiler.h                $DST_DIR
cp --parents -p $BCM_SDK/include/sal/core/alloc.h              $DST_DIR
cp --parents -p $BCM_SDK/include/sal/core/boot.h               $DST_DIR
cp --parents -p $BCM_SDK/include/sal/core/libc.h               $DST_DIR
cp --parents -p $BCM_SDK/include/sal/core/spl.h                $DST_DIR
cp --parents -p $BCM_SDK/include/sal/core/sync.h               $DST_DIR
cp --parents -p $BCM_SDK/include/sal/core/thread.h             $DST_DIR
cp --parents -p $BCM_SDK/include/sal/core/time.h               $DST_DIR
cp --parents -p $BCM_SDK/include/sal/core/dpc.h                $DST_DIR

cp --parents -p $BCM_SDK/include/sal/types.h                   $DST_DIR
cp --parents -p $BCM_SDK/include/sdk_config.h                  $DST_DIR
cp --parents -p $BCM_SDK/include/shared/alloc.h                $DST_DIR
cp --parents -p $BCM_SDK/include/shared/avl.h                  $DST_DIR
cp --parents -p $BCM_SDK/include/shared/bitop.h                $DST_DIR
cp --parents -p $BCM_SDK/include/shared/cosq.h                 $DST_DIR
cp --parents -p $BCM_SDK/include/shared/enumgen.h              $DST_DIR
cp --parents -p $BCM_SDK/include/shared/error.h                $DST_DIR
cp --parents -p $BCM_SDK/include/shared/fifo.h                 $DST_DIR
cp --parents -p $BCM_SDK/include/shared/gport.h                $DST_DIR
cp --parents -p $BCM_SDK/include/shared/l3.h                   $DST_DIR
cp --parents -p $BCM_SDK/include/shared/mpls.h                 $DST_DIR
cp --parents -p $BCM_SDK/include/shared/pbmp.h                 $DST_DIR
cp --parents -p $BCM_SDK/include/shared/phyconfig.h            $DST_DIR
cp --parents -p $BCM_SDK/include/shared/phyreg.h               $DST_DIR
cp --parents -p $BCM_SDK/include/shared/port_ability.h         $DST_DIR
cp --parents -p $BCM_SDK/include/shared/port.h                 $DST_DIR
cp --parents -p $BCM_SDK/include/shared/portmode.h             $DST_DIR
cp --parents -p $BCM_SDK/include/shared/rx.h                   $DST_DIR
cp --parents -p $BCM_SDK/include/shared/switch.h               $DST_DIR
cp --parents -p $BCM_SDK/include/shared/types.h                $DST_DIR
cp --parents -p $BCM_SDK/include/shared/util.h                 $DST_DIR
cp --parents -p $BCM_SDK/include/shared/warmboot.h             $DST_DIR
cp --parents -p $BCM_SDK/include/shared/bsl.h                  $DST_DIR
cp --parents -p $BCM_SDK/include/shared/bsltypes.h             $DST_DIR
cp --parents -p $BCM_SDK/include/shared/bslenum.h              $DST_DIR
cp --parents -p $BCM_SDK/include/shared/fabric.h               $DST_DIR
cp --parents -p $BCM_SDK/include/soc/axp.h                     $DST_DIR
cp --parents -p $BCM_SDK/include/soc/cfp.h                     $DST_DIR
cp --parents -p $BCM_SDK/include/soc/chip.h                    $DST_DIR
cp --parents -p $BCM_SDK/include/soc/cm.h                      $DST_DIR
cp --parents -p $BCM_SDK/include/soc/cmic.h                    $DST_DIR
cp --parents -p $BCM_SDK/include/soc/cmicm.h                   $DST_DIR
cp --parents -p $BCM_SDK/include/soc/cmtypes.h                 $DST_DIR
cp --parents -p $BCM_SDK/include/soc/counter.h                 $DST_DIR
cp --parents -p $BCM_SDK/include/soc/dcb.h                     $DST_DIR
cp --parents -p $BCM_SDK/include/soc/debug.h                   $DST_DIR
cp --parents -p $BCM_SDK/include/soc/defs.h                    $DST_DIR
cp --parents -p $BCM_SDK/include/soc/devids.h                  $DST_DIR
cp --parents -p $BCM_SDK/include/soc/dma.h                     $DST_DIR
cp --parents -p $BCM_SDK/include/soc/dport.h                   $DST_DIR
cp --parents -p $BCM_SDK/include/soc/drv.h                     $DST_DIR
cp --parents -p $BCM_SDK/include/soc/drv_if.h                  $DST_DIR
cp --parents -p $BCM_SDK/include/soc/drvtypes.h                $DST_DIR
cp --parents -p $BCM_SDK/include/soc/enet.h                    $DST_DIR
cp --parents -p $BCM_SDK/include/soc/error.h                   $DST_DIR
cp --parents -p $BCM_SDK/include/soc/ethdma.h                  $DST_DIR
cp --parents -p $BCM_SDK/include/soc/feature.h                 $DST_DIR
cp --parents -p $BCM_SDK/include/soc/field.h                   $DST_DIR
cp --parents -p $BCM_SDK/include/soc/intr.h                    $DST_DIR
cp --parents -p $BCM_SDK/include/soc/ll.h                      $DST_DIR
cp --parents -p $BCM_SDK/include/soc/macipadr.h                $DST_DIR
cp --parents -p $BCM_SDK/include/soc/maxdef.h                  $DST_DIR
cp --parents -p $BCM_SDK/include/soc/mcm/allenum.h             $DST_DIR
cp --parents -p $BCM_SDK/include/soc/mcm/allfields.h           $DST_DIR
cp --parents -p $BCM_SDK/include/soc/mcm/cmicm.h               $DST_DIR
cp --parents -p $BCM_SDK/include/soc/mcm/memacc.h              $DST_DIR
cp --parents -p $BCM_SDK/include/soc/mcm/memregs.h             $DST_DIR
cp --parents -p $BCM_SDK/include/soc/mem.h                     $DST_DIR
cp --parents -p $BCM_SDK/include/soc/memory.h                  $DST_DIR
cp --parents -p $BCM_SDK/include/soc/mmuerr.h                  $DST_DIR
cp --parents -p $BCM_SDK/include/soc/phyctrl.h                 $DST_DIR
cp --parents -p $BCM_SDK/include/soc/port_ability.h            $DST_DIR
cp --parents -p $BCM_SDK/include/soc/portmode.h                $DST_DIR
cp --parents -p $BCM_SDK/include/soc/property.h                $DST_DIR
cp --parents -p $BCM_SDK/include/soc/rcpu.h                    $DST_DIR
cp --parents -p $BCM_SDK/include/soc/register.h                $DST_DIR
cp --parents -p $BCM_SDK/include/soc/robo_fp.h                 $DST_DIR
cp --parents -p $BCM_SDK/include/soc/rx.h                      $DST_DIR
cp --parents -p $BCM_SDK/include/soc/sbusdma.h                 $DST_DIR
cp --parents -p $BCM_SDK/include/soc/scache.h                  $DST_DIR
cp --parents -p $BCM_SDK/include/soc/schanmsg.h                $DST_DIR
cp --parents -p $BCM_SDK/include/soc/shared/mos_intr_common.h  $DST_DIR
cp --parents -p $BCM_SDK/include/soc/shared/mos_msg_common.h   $DST_DIR
cp --parents -p $BCM_SDK/include/soc/timesync.h                $DST_DIR
cp --parents -p $BCM_SDK/include/soc/types.h                   $DST_DIR
cp --parents -p $BCM_SDK/include/soc/util.h                    $DST_DIR
cp --parents -p $BCM_SDK/include/soc/vm.h                      $DST_DIR
cp --parents -p $BCM_SDK/systems/bde/linux/include/linux-bde.h $DST_DIR
cp --parents -p $BCM_SDK/systems/bde/pli/plibde.h              $DST_DIR
cp --parents -p $BCM_SDK/include/shared/bfd.h                  $DST_DIR
cp --parents -p $BCM_SDK/include/shared/bhh.h                  $DST_DIR
cp --parents -p $BCM_SDK/include/shared/cyclic_buffer.h        $DST_DIR
cp --parents -p $BCM_SDK/include/shared/evlog.h                $DST_DIR
cp --parents -p $BCM_SDK/include/shared/fcmap.h                $DST_DIR
cp --parents -p $BCM_SDK/include/shared/hash_tbl.h             $DST_DIR
cp --parents -p $BCM_SDK/include/shared/idents.h               $DST_DIR
cp --parents -p $BCM_SDK/include/shared/idxres_afl.h           $DST_DIR
cp --parents -p $BCM_SDK/include/shared/idxres_fl.h            $DST_DIR
cp --parents -p $BCM_SDK/include/shared/idxres_mdb.h           $DST_DIR
cp --parents -p $BCM_SDK/include/shared/pack.h                 $DST_DIR
cp --parents -p $BCM_SDK/include/shared/shr_allocator.h        $DST_DIR
cp --parents -p $BCM_SDK/include/shared/shr_res_bitmap.h       $DST_DIR
cp --parents -p $BCM_SDK/include/shared/shr_res_tag_bitmap.h   $DST_DIR
cp --parents -p $BCM_SDK/include/shared/shr_resmgr.h           $DST_DIR
cp --parents -p $BCM_SDK/include/shared/shr_template.h         $DST_DIR
cp --parents -p $BCM_SDK/include/shared/sram.h                 $DST_DIR

cp --parents -p $BCM_SDK/include/soc/cmdebug.h                 $DST_DIR
# cp --parents -p $BCM_SDK/include/soc/mdebug.h                $DST_DIR
# cp --parents -p $BCM_SDK/include/soc/cmmdebug.h              $DST_DIR
# cp --parents -p $BCM_SDK/include/appl/diag/mdebug.h          $DST_DIR

cp --parents -p $BCM_SDK/include/bcm/stack.h                   $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/cosq.h                    $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/fabric.h                  $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/vswitch.h                 $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/link.h                    $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/qos.h                     $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/mpls.h                    $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/failover.h                $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/ipmc.h                    $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/pkt.h                     $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/tx.h                      $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/rx.h                      $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/l3.h                      $DST_DIR

cp --parents -p $BCM_SDK/include/bcm/trunk.h                   $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/mirror.h                  $DST_DIR

# --- includes added in SDK 6.4.4
cp --parents -p $BCM_SDK/include/soc/uc_msg.h                  $DST_DIR
cp --parents -p $BCM_SDK/include/shared/multicast.h            $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/stg.h                     $DST_DIR
cp --parents -p $BCM_SDK/include/appl/diag/dcmn/init_deinit.h  $DST_DIR

# --- OAM headers
cp --parents -p $BCM_SDK/include/bcm/oam.h                     $DST_DIR
cp --parents -p $BCM_SDK/include/bcm_int/dpp/oam.h             $DST_DIR

# --- Includes for producion testing utility ----
cp --parents -p $BCM_SDK/src/appl/test/testlist.h              $DST_DIR
cp --parents -p $BCM_SDK/src/appl/test/cache_mem_test.h        $DST_DIR
cp --parents -p $BCM_SDK/include/appl/diag/test.h              $DST_DIR

# --- Include headers for MAC table count
cp --parents -p $BCM_SDK/include/sal/limits.h                  $DST_DIR
cp --parents -p $BCM_SDK/include/soc/shmoo_combo28.h           $DST_DIR
cp --parents -p $BCM_SDK/include/bcm/debug.h                   $DST_DIR
cp --parents -p -r $BCM_SDK/include/soc/dcmn/                  $DST_DIR
cp --parents -p -r $BCM_SDK/include/soc/dpp/                   $DST_DIR


################################################################################
# Copy all libs
#
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/version.o                   $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/platform_defines.o          $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libdiag.a                   $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libdiag_dpp.a               $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libdiag_dfe.a               $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libdiag_dcmn.a              $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libtest.a                   $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libdiscover.a               $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libstktask.a                $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libdiagcint.a               $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libcint.a                   $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libdiagapi.a                $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsal_appl_editline.a      $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsal_appl.a               $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsal_appl_plat.a          $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/liblubde.a                  $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libbcm.a                    $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libbcm_rpc.a                $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libbcm_compat.a             $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libbcm_dpp.a                $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libbcm_dfe.a                $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libcputrans.a               $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libcpudb.a                  $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoccommon.a              $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc_phy.a                $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc_wcmod.a              $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc_tscmod.a             $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc_dpp.a                $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc_mcm.a                $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc_dcmn.a               $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libbcm_dpp_ppd.a            $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libbcm_dpp_ppc.a            $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libbcm_dpp_pcp.a            $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libbcm_dpp_sand_fm.a        $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libbcm_dpp_sand_mgmt.a      $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libbcm_dpp_sand_utils.a     $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libbcm_dpp_tmc.a            $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc_dpp_petra.a          $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libbcm_dpp_pb_tm.a          $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libbcm_dpp_pb_pp.a          $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libappl_dpp_pcp.a           $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc_dpp_arad_nif.a       $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc_dpp_arad.a           $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libbcm_dpp_arad_pp.a        $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libappl_dcmn_rx_los.a       $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libappl_dpp_interrupts.a    $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libappl_dfe_interrupts.a    $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libappl_dcmn_interrupts.a   $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc_dfe.a                $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc_dfe_cmn.a            $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc_dfe_fe1600.a         $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libbcm_common.a             $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc_shared.a             $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libshared.a                 $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc_i2c.a                $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsal_core.a               $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsal_core_plat.a          $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libcustomer.a               $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc.a                    $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc_dpp_drc.a            $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc_dpp_port.a           $DST_DIR/x86/

# Added libs in SDK 6.4.4
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libphymod.a                 $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc_portmod.a            $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libshared_swstate.a         $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc_hwstate.a            $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libdiag_phymod.a            $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc_dfe_fe3200.a         $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libsoc_portmod_pms.a        $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libdiag_portmod.a           $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libappl_dpp_FecAllocaiton.a $DST_DIR/x86/

# Added to support DUNE UI
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libappl_dpp_ui.a            $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libappl_dpp_ui_pb.a         $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libappl_dpp_ui_pcp.a        $DST_DIR/x86/
cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libappl_dpp_ui_ppd.a        $DST_DIR/x86/

# cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libeth.a                  $DST_DIR/x86/
# cp -p $BCM_SDK/build/unix-user/x86-smp_generic-2_6/libeth.a                  $DST_DIR/x86/


################################################################################
# Add meta-data
#
cp -p $BCM_SDK/RELEASE $DST_DIR
echo -n "SDK_VER=" > $DST_DIR/x86/bcm_sdk.mk
cat $BCM_SDK/RELEASE >> $DST_DIR/x86/bcm_sdk.mk
