#/bin/sh -x
function make_bcm()
{
	echo "building bcm sdk for x86 arch..."
	source env_x86_4_mrv
	export ARCH=x86
	#ib_console -c bcm_sdk
	export MRV_STANDALONE=1 
	make 
	#-j 11
    if [ $? != 0 ]
    then
        echo -e " FAILED\nERROR: failed to build BCM_SDK $SDK_VER"
        exit 1
    fi
	#copying bcm targets to x86 root-fs
	echo -n "copying targets to bcm_lib..."
	mkdir -p $ROOT_DIR/3rd_party/bcm_artifacts/$SDK_VER/deploy
	cp linux-kernel-bde.ko $ROOT_DIR/3rd_party/bcm_artifacts/$SDK_VER/deploy
	cp linux-user-bde.ko $ROOT_DIR/3rd_party/bcm_artifacts/$SDK_VER/deploy
	cd $SDK/scripts
	# copy bcm headers to bcm_lib
	./cp_hal_bcm_headers.sh $SDK_VER
	# copy bcm libraries to bcm_lib
	./cp_hal_bcm_x86_libs.sh $SDK_VER
	echo " done"
	espeak "done build SDK"
}

time make_bcm "${@}"
date 1>&2


