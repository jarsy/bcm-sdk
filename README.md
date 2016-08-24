Note
====
This README is written following Github Flavored Markdown (GFM):
https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet


Build
=====
Two basic variables should be available for the compilation scripts:
 - `TOOLCHAIN_DIR`
 - `KERNDIR`

Hence, export them before running the build script:
```
export KERNDIR=FULL_PATH_TO_COMPILED_LINUX_KERNEL
export TOOLCHAIN_DIR=FULL_PATH_TO_TOOLCHAIN_BIN_DIRECTORY
```

For example:
```
export KERNDIR=~/root-fs-x86/src/linux/
export TOOLCHAIN_DIR=~/clovis_2015_lk4_4/buildtools/i686-nptl-linux-gnu
```

Run `scripts/build.sh <Scripts dir> <BCM SDK path> <Install dir path>`.
For example:
```
scripts/build.sh ./scripts . /tmp/bcm_sdk_artifacts
```

The full example is:
```
cd ~/projects/bcm_sdk
export KERNDIR=~/root-fs-x86/src/linux/
export TOOLCHAIN_DIR=~/clovis_2015_lk4_4/buildtools/i686-nptl-linux-gnu
scripts/build.sh ./scripts . /tmp/bcm_sdk_artifacts
```


Build Artifacts
===============
The build artifacts are found under `./build` directory.
The below command may be used to print the important files:
```
find build/ -not -type d -not -name "*.P" -not -name "*.cmd" -not -name "*.force" -not -name "*.sig" -not -name "*.order" -not -name "*.mod" -not -name "*.tree" -not -name "*.d" -not -name "*.o" -not -name "*.c" -not -name "*.h" | sort | vim -
```

Also, take a look at the `scripts/copy_artifacts.sh` script
that copied the needed build artifacts to the designated place.


SDK Upgrade
===============
When upgrading BCM SDK verify the below:
 - Merge the changes done in the file from previous SDK:
   `Makefile.linux-x86-generic-common-2_6`
 - Change maxpayload to 128 (bytes) at `linux-kernel-bde.c` (~line 148)
 - Merge changes in `src/appl/diag/system.c` (`#ifdef` by Jayant) within `diag_shell()`
