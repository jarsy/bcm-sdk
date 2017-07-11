#!/bin/bash

from_dir=$1   # BCM SDK source code directory.
bcm_to_dir=$2 # BCM SDK artifacts directory in "main" project.

to=$bcm_to_dir

cp --parents -p    $from_dir/include/appl/diag/cmdlist.h                     $to
cp --parents -p    $from_dir/include/appl/diag/parse.h                       $to
cp --parents -p    $from_dir/include/appl/diag/shell.h                       $to
cp --parents -p    $from_dir/include/appl/diag/sysconf.h                     $to
cp --parents -p    $from_dir/include/appl/diag/system.h                      $to
cp --parents -p    $from_dir/include/appl/diag/warmboot.h                    $to
cp --parents -p    $from_dir/include/assert.h                                $to
cp --parents -p    $from_dir/include/bcm/error.h                             $to
cp --parents -p    $from_dir/include/bcm/field.h                             $to
cp --parents -p    $from_dir/include/bcm/l2.h                                $to
cp --parents -p    $from_dir/include/bcm/l3.h                                $to
cp --parents -p    $from_dir/include/bcm/multicast.h                         $to
cp --parents -p    $from_dir/include/bcm/pkt.h                               $to
cp --parents -p    $from_dir/include/bcm/policer.h                           $to
cp --parents -p    $from_dir/include/bcm/port.h                              $to
cp --parents -p    $from_dir/include/bcm/rx.h                                $to
cp --parents -p    $from_dir/include/bcm/stat.h                              $to
cp --parents -p    $from_dir/include/bcm/switch.h                            $to
cp --parents -p    $from_dir/include/bcm/tunnel.h                            $to
cp --parents -p    $from_dir/include/bcm/tx.h                                $to
cp --parents -p    $from_dir/include/bcm/types.h                             $to
cp --parents -p    $from_dir/include/bcm/vlan.h                              $to
cp --parents -p    $from_dir/include/bcm/udf.h                               $to
cp --parents -p    $from_dir/include/ibde.h                                  $to
cp --parents -p    $from_dir/include/sal/appl/config.h                       $to
cp --parents -p    $from_dir/include/sal/appl/io.h                           $to
cp --parents -p    $from_dir/include/sal/appl/sal.h                          $to
cp --parents -p    $from_dir/include/sal/compiler.h                          $to
cp --parents -p    $from_dir/include/sal/core/alloc.h                        $to
cp --parents -p    $from_dir/include/sal/core/boot.h                         $to
cp --parents -p    $from_dir/include/sal/core/libc.h                         $to
cp --parents -p    $from_dir/include/sal/core/spl.h                          $to
cp --parents -p    $from_dir/include/sal/core/sync.h                         $to
cp --parents -p    $from_dir/include/sal/core/thread.h                       $to
cp --parents -p    $from_dir/include/sal/core/time.h                         $to
cp --parents -p    $from_dir/include/sal/core/dpc.h                          $to
cp --parents -p    $from_dir/include/sal/types.h                             $to
cp --parents -p    $from_dir/include/sdk_config.h                            $to
cp --parents -p    $from_dir/include/shared/alloc.h                          $to
cp --parents -p    $from_dir/include/shared/avl.h                            $to
cp --parents -p    $from_dir/include/shared/bitop.h                          $to
cp --parents -p    $from_dir/include/shared/cosq.h                           $to
cp --parents -p    $from_dir/include/shared/enumgen.h                        $to
cp --parents -p    $from_dir/include/shared/error.h                          $to
cp --parents -p    $from_dir/include/shared/fifo.h                           $to
cp --parents -p    $from_dir/include/shared/gport.h                          $to
cp --parents -p    $from_dir/include/shared/l3.h                             $to
cp --parents -p    $from_dir/include/shared/mpls.h                           $to
cp --parents -p    $from_dir/include/shared/pbmp.h                           $to
cp --parents -p    $from_dir/include/shared/phyconfig.h                      $to
cp --parents -p    $from_dir/include/shared/phyreg.h                         $to
cp --parents -p    $from_dir/include/shared/port_ability.h                   $to
cp --parents -p    $from_dir/include/shared/port.h                           $to
cp --parents -p    $from_dir/include/shared/portmode.h                       $to
cp --parents -p    $from_dir/include/shared/rx.h                             $to
cp --parents -p    $from_dir/include/shared/switch.h                         $to
cp --parents -p    $from_dir/include/shared/types.h                          $to
cp --parents -p    $from_dir/include/shared/util.h                           $to
cp --parents -p    $from_dir/include/shared/warmboot.h                       $to
cp --parents -p    $from_dir/include/shared/bsl.h                            $to
cp --parents -p    $from_dir/include/shared/bsltypes.h                       $to
cp --parents -p    $from_dir/include/shared/bslenum.h                        $to
cp --parents -p    $from_dir/include/shared/fabric.h                         $to
cp --parents -p    $from_dir/include/soc/axp.h                               $to
cp --parents -p    $from_dir/include/soc/cfp.h                               $to
cp --parents -p    $from_dir/include/soc/chip.h                              $to
cp --parents -p    $from_dir/include/soc/cm.h                                $to
cp --parents -p    $from_dir/include/soc/cmic.h                              $to
cp --parents -p    $from_dir/include/soc/cmicm.h                             $to
cp --parents -p    $from_dir/include/soc/cmtypes.h                           $to
cp --parents -p    $from_dir/include/soc/counter.h                           $to
cp --parents -p    $from_dir/include/soc/dcb.h                               $to
cp --parents -p    $from_dir/include/soc/debug.h                             $to
cp --parents -p    $from_dir/include/soc/defs.h                              $to
cp --parents -p    $from_dir/include/soc/devids.h                            $to
cp --parents -p    $from_dir/include/soc/dma.h                               $to
cp --parents -p    $from_dir/include/soc/dport.h                             $to
cp --parents -p    $from_dir/include/soc/drv.h                               $to
cp --parents -p    $from_dir/include/soc/drv_if.h                            $to
cp --parents -p    $from_dir/include/soc/drvtypes.h                          $to
cp --parents -p    $from_dir/include/soc/enet.h                              $to
cp --parents -p    $from_dir/include/soc/error.h                             $to
cp --parents -p    $from_dir/include/soc/ethdma.h                            $to
cp --parents -p    $from_dir/include/soc/feature.h                           $to
cp --parents -p    $from_dir/include/soc/field.h                             $to
cp --parents -p    $from_dir/include/soc/intr.h                              $to
cp --parents -p    $from_dir/include/soc/ll.h                                $to
cp --parents -p    $from_dir/include/soc/macipadr.h                          $to
cp --parents -p    $from_dir/include/soc/maxdef.h                            $to
cp --parents -p    $from_dir/include/soc/mcm/allenum.h                       $to
cp --parents -p    $from_dir/include/soc/mcm/allfields.h                     $to
cp --parents -p    $from_dir/include/soc/mcm/cmicm.h                         $to
cp --parents -p    $from_dir/include/soc/mcm/memacc.h                        $to
cp --parents -p    $from_dir/include/soc/mcm/memregs.h                       $to
cp --parents -p    $from_dir/include/soc/mem.h                               $to
cp --parents -p    $from_dir/include/soc/memory.h                            $to
cp --parents -p    $from_dir/include/soc/mmuerr.h                            $to
cp --parents -p    $from_dir/include/soc/phyctrl.h                           $to
cp --parents -p    $from_dir/include/soc/port_ability.h                      $to
cp --parents -p    $from_dir/include/soc/portmode.h                          $to
cp --parents -p    $from_dir/include/soc/property.h                          $to
cp --parents -p    $from_dir/include/soc/rcpu.h                              $to
cp --parents -p    $from_dir/include/soc/register.h                          $to
cp --parents -p    $from_dir/include/soc/robo_fp.h                           $to
cp --parents -p    $from_dir/include/soc/rx.h                                $to
cp --parents -p    $from_dir/include/soc/sbusdma.h                           $to
cp --parents -p    $from_dir/include/soc/scache.h                            $to
cp --parents -p    $from_dir/include/soc/schanmsg.h                          $to
cp --parents -p    $from_dir/include/soc/shared/mos_intr_common.h            $to
cp --parents -p    $from_dir/include/soc/shared/mos_msg_common.h             $to
cp --parents -p    $from_dir/include/soc/timesync.h                          $to
cp --parents -p    $from_dir/include/soc/types.h                             $to
cp --parents -p    $from_dir/include/soc/util.h                              $to
cp --parents -p    $from_dir/include/soc/vm.h                                $to
cp --parents -p    $from_dir/systems/bde/linux/include/linux-bde.h           $to
cp --parents -p    $from_dir/systems/bde/pli/plibde.h                        $to
cp --parents -p    $from_dir/include/shared/bfd.h                            $to
cp --parents -p    $from_dir/include/shared/bhh.h                            $to
cp --parents -p    $from_dir/include/shared/cyclic_buffer.h                  $to
cp --parents -p    $from_dir/include/shared/evlog.h                          $to
cp --parents -p    $from_dir/include/shared/fcmap.h                          $to
cp --parents -p    $from_dir/include/shared/hash_tbl.h                       $to
cp --parents -p    $from_dir/include/shared/idents.h                         $to
cp --parents -p    $from_dir/include/shared/idxres_afl.h                     $to
cp --parents -p    $from_dir/include/shared/idxres_fl.h                      $to
cp --parents -p    $from_dir/include/shared/idxres_mdb.h                     $to
cp --parents -p    $from_dir/include/shared/pack.h                           $to
cp --parents -p    $from_dir/include/shared/shr_allocator.h                  $to
cp --parents -p    $from_dir/include/shared/shr_res_bitmap.h                 $to
cp --parents -p    $from_dir/include/shared/shr_res_tag_bitmap.h             $to
cp --parents -p    $from_dir/include/shared/shr_resmgr.h                     $to
cp --parents -p    $from_dir/include/shared/shr_template.h                   $to
cp --parents -p    $from_dir/include/shared/sram.h                           $to
cp --parents -p    $from_dir/include/soc/cmdebug.h                           $to
cp --parents -p    $from_dir/include/bcm/stack.h                             $to
cp --parents -p    $from_dir/include/bcm/cosq.h                              $to
cp --parents -p    $from_dir/include/bcm/fabric.h                            $to
cp --parents -p    $from_dir/include/bcm/vswitch.h                           $to
cp --parents -p    $from_dir/include/bcm/link.h                              $to
cp --parents -p    $from_dir/include/bcm/qos.h                               $to
cp --parents -p    $from_dir/include/bcm/mpls.h                              $to
cp --parents -p    $from_dir/include/bcm/failover.h                          $to
cp --parents -p    $from_dir/include/bcm/ipmc.h                              $to
cp --parents -p    $from_dir/include/bcm/pkt.h                               $to
cp --parents -p    $from_dir/include/bcm/tx.h                                $to
cp --parents -p    $from_dir/include/bcm/rx.h                                $to
cp --parents -p    $from_dir/include/bcm/l3.h                                $to
cp --parents -p    $from_dir/include/bcm/trunk.h                             $to
cp --parents -p    $from_dir/include/bcm/mirror.h                            $to
cp --parents -p    $from_dir/include/soc/uc_msg.h                            $to
cp --parents -p    $from_dir/include/shared/multicast.h                      $to
cp --parents -p    $from_dir/include/bcm/stg.h                               $to
cp --parents -p    $from_dir/include/appl/diag/dcmn/init_deinit.h            $to
cp --parents -p    $from_dir/include/soc/hwstate/hw_log.h                    $to
cp --parents -p    $from_dir/include/shared/swstate/access/sw_state_access.h $to
cp --parents -p    $from_dir/include/shared/pkt.h                            $to
cp --parents -p    $from_dir/include/bcm/oam.h                               $to
cp --parents -p    $from_dir/include/bcm_int/dpp/oam.h                       $to
cp --parents -p    $from_dir/src/appl/test/testlist.h                        $to
cp --parents -p    $from_dir/src/appl/test/cache_mem_test.h                  $to
cp --parents -p    $from_dir/include/appl/diag/test.h                        $to
cp --parents -p    $from_dir/include/sal/limits.h                            $to
cp --parents -p    $from_dir/include/soc/shmoo_combo28.h                     $to
cp --parents -p    $from_dir/include/bcm/debug.h                             $to
cp --parents -p -r $from_dir/include/soc/dcmn/                               $to
cp --parents -p -r $from_dir/include/soc/dpp/                                $to
cp --parents -p    $from_dir/include/soc/ser.h                               $to
cp --parents -p    $from_dir/include/bcm/lb.h                                $to
cp --parents -p    $from_dir/include/soc/shmoo_combo16.h                     $to
cp --parents -p    $from_dir/include/soc/kbp/alg_kbp/include/errors.h        $to
cp --parents -p    $from_dir/include/soc/kbp/alg_kbp/include/kbp_portable.h  $to
cp --parents -p    $from_dir/include/soc/kbp/alg_kbp/include/db.h            $to
cp --parents -p    $from_dir/include/soc/kbp/alg_kbp/include/device.h        $to
cp --parents -p    $from_dir/include/soc/kbp/alg_kbp/include/init.h          $to

# copy release file
cp -p $from_dir/RELEASE $bcm_to_dir

to_dir_x86=$bcm_to_dir/x86
if [ ! -d $to_dir_x86 ] ; then
    mkdir $to_dir_x86
fi

echo -n "SDK_VER=" >       $to_dir_x86/bcm_sdk.mk
cat   $from_dir/RELEASE >> $to_dir_x86/bcm_sdk.mk
