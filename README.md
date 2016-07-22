Note
====
This README is written following Github Flavored Markdown (GFM):
https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet


Compilation
===========
Export environment variables:
```
export ARCH=x86
export TARGET=linux-x86-smp_generic-2_6
export TOOLCHAIN_DIR=~/clovis_2015/buildtools/i686-nptl-linux-gnu
export PATH=/usr/bin:$TOOLCHAIN_DIR/bin:$PATH
export CROSS_COMPILE=i686-nptl-linux-gnu-
export LDFLAGS="-L $TOOLCHAIN_DIR/lib"
export OPT_CFLAGS=" -m32"
export MODULE_LDFLAGS=" -m elf_i386"
export KERNDIR=full-path-to-compiled-linux-kernel-source-code-tree
```

Compile using `make` utility from appropriate directory:
```
cd systems/linux/user/x86-smp_generic-2_6
make
```


Build Artifacts
===============
The build artifacts are found under `./build` directory.
The below command may be used to print the important files:
```
find build/ -not -type d -not -name "*.P" -not -name "*.cmd" -not -name "*.force" -not -name "*.sig" -not -name "*.order" -not -name "*.mod" -not -name "*.tree" -not -name "*.d" -not -name "*.o" | sort | vim -
```


SDK Upgrade
===============
When upgrading BCM SDK verify the below:
 - Merge the changes done in the file from previous SDK:
   `Makefile.linux-x86-generic-common-2_6`
 - Change maxpayload to 128 (bytes) at `linux-kernel-bde.c` (~line 148)
 - Merge changes in `src/appl/diag/system.c` (`#ifdef` by Jayant) within `diag_shell()`
