Note
====
This README is written following Github Flavored Markdown (GFM):
https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet


Build
=====
Run `scripts/build.sh <Scripts dir> <BCM SDK path> <Install dir path>`.
For example:
```
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
