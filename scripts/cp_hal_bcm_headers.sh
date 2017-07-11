#! /bin/bash
#------------------------------------------------------------------------
# target folder is stated as "include" since --parent is in use
# the file will be based in the sdk_lib
#------------------------------------------------------------------------

targetDir=`pwd | grep -o "[0-9]\+\.[0-9]\+\.[0-9]\+"`

mkdir -p ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/appl/diag/cmdlist.h ../../../bcm_artifacts/$targetDir/include
# cp --parents -p ../include/appl/diag/debug.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/appl/diag/parse.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/appl/diag/shell.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/appl/diag/sysconf.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/appl/diag/system.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/appl/diag/warmboot.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/assert.h ../../../bcm_artifacts/$targetDir/include
# cp --parents -p ../include/bcm/ces.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/error.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/field.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/l2.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/l3.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/multicast.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/pkt.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/policer.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/port.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/rx.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/stat.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/switch.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/tunnel.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/tx.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/types.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/vlan.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/udf.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/ibde.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/sal/appl/config.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/sal/appl/io.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/sal/appl/sal.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/sal/compiler.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/sal/core/alloc.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/sal/core/boot.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/sal/core/libc.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/sal/core/spl.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/sal/core/sync.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/sal/core/thread.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/sal/core/time.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/sal/core/dpc.h ../../../bcm_artifacts/$targetDir/include

cp --parents -p ../include/sal/types.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/sdk_config.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/alloc.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/avl.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/bitop.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/cosq.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/enumgen.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/error.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/fifo.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/gport.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/l3.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/mpls.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/pbmp.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/phyconfig.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/phyreg.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/port_ability.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/port.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/portmode.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/rx.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/switch.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/types.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/util.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/warmboot.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/bsl.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/bsltypes.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/bslenum.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/fabric.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/axp.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/cfp.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/chip.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/cm.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/cmic.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/cmicm.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/cmtypes.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/counter.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/dcb.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/debug.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/defs.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/devids.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/dma.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/dport.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/drv.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/drv_if.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/drvtypes.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/enet.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/error.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/ethdma.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/feature.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/field.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/intr.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/ll.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/macipadr.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/maxdef.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/mcm/allenum.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/mcm/allfields.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/mcm/cmicm.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/mcm/memacc.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/mcm/memregs.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/mem.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/memory.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/mmuerr.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/phyctrl.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/port_ability.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/portmode.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/property.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/rcpu.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/register.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/robo_fp.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/rx.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/sbusdma.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/scache.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/schanmsg.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/shared/mos_intr_common.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/shared/mos_msg_common.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/timesync.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/types.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/util.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/vm.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../systems/bde/linux/include/linux-bde.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../systems/bde/pli/plibde.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/bfd.h  ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/bhh.h  ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/cyclic_buffer.h  ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/evlog.h  ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/fcmap.h  ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/hash_tbl.h  ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/idents.h  ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/idxres_afl.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/idxres_fl.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/idxres_mdb.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/pack.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/shr_allocator.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/shr_res_bitmap.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/shr_res_tag_bitmap.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/shr_resmgr.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/shr_template.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/sram.h ../../../bcm_artifacts/$targetDir/include

cp --parents -p ../include/soc/cmdebug.h ../../../bcm_artifacts/$targetDir/include
# cp --parents -p ../include/soc/mdebug.h ../../../bcm_artifacts/$targetDir/include
# cp --parents -p ../include/soc/cmmdebug.h ../../../bcm_artifacts/$targetDir/include
# cp --parents -p ../include/appl/diag/mdebug.h ../../../bcm_artifacts/$targetDir/include

cp --parents -p ../include/bcm/stack.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/cosq.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/fabric.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/vswitch.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/link.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/qos.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/mpls.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/failover.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/ipmc.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/pkt.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/tx.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/rx.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/l3.h ../../../bcm_artifacts/$targetDir/include

cp --parents -p ../include/bcm/trunk.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/mirror.h ../../../bcm_artifacts/$targetDir/include

# --- includes added in SDK 6.4.4
cp --parents -p ../include/soc/uc_msg.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/multicast.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/stg.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/appl/diag/dcmn/init_deinit.h ../../../bcm_artifacts/$targetDir/include

# --- includes added in SDK 6.4.11
#cp --parents -p ../include/shared/swstate/sw_state.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/hwstate/hw_log.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/swstate/access/sw_state_access.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/shared/pkt.h ../../../bcm_artifacts/$targetDir/include
#cp --parents -p ../include/shared/swstate/layout/sw_state_defs_layout.h ../../../bcm_artifacts/$targetDir/include

# --- OAM headers
cp --parents -p ../include/bcm/oam.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm_int/dpp/oam.h ../../../bcm_artifacts/$targetDir/include

# --- Includes for producion testing utility ----
cp --parents -p ../src/appl/test/testlist.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../src/appl/test/cache_mem_test.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/appl/diag/test.h ../../../bcm_artifacts/$targetDir/include

# --- Include headers for MAC table count
cp --parents -p ../include/sal/limits.h    ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/shmoo_combo28.h  ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/debug.h     ../../../bcm_artifacts/$targetDir/include
cp --parents -p -r ../include/soc/dcmn/    ../../../bcm_artifacts/$targetDir/include
cp --parents -p -r ../include/soc/dpp/     ../../../bcm_artifacts/$targetDir/include

# --- Include headers for SDK 6.5.7
cp --parents -p ../include/soc/ser.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/bcm/lb.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/shmoo_combo16.h ../../../bcm_artifacts/$targetDir/include
# --- Include headers for KBP
cp --parents -p ../include/soc/kbp/alg_kbp/include/errors.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/kbp/alg_kbp/include/kbp_portable.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/kbp/alg_kbp/include/db.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/kbp/alg_kbp/include/device.h ../../../bcm_artifacts/$targetDir/include
cp --parents -p ../include/soc/kbp/alg_kbp/include/init.h ../../../bcm_artifacts/$targetDir/include

cp -p ../RELEASE ../../../bcm_artifacts/$targetDir
echo -n "SDK_VER=" >../../../bcm_artifacts/$targetDir/x86/bcm_sdk.mk
cat  ../RELEASE >> ../../../bcm_artifacts/$targetDir/x86/bcm_sdk.mk
