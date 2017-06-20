This document gives the directory structure of the SDK package
===============================================================

The SDK is provided as a zip file. Unzipping the file by command
(bzip2 -cd <tarball_name> | tar xf -) will create directory
"sdk" at root level with the following structure:

 ->     docs         (Release notes)
 ->     include      (all include files needed by user applications)
 ->     src          (source code, binaries, libraries: release specific [portability, pcie/device_driver, lib])

binary file: 
                  kbp_diags -> diagnostic tool

Usage of Libs and include files:
                  libsoc_alg_kbp.a   => Library contains complete algorithmic functionality.
                  Use case -> To build the user application, include all the header files and link the lib "libsoc_alg_kbp.a".

Optional Libs:
              1.  libkbp_alg_only.a  => Library contains implementation of algorithmic modes and should be used with MP source package.
  	 
              2.  libkbppcie.a       => Library contains the implementation of the XPT for OP

For the packages which has both 32b and 64b libs, 64b libs will be present in src/soc/kbp/alg_kbp/lib/lib64

To install pcie device driver please follow below steps.
 -> cd device_driver
 -> make
 -> insmod kbp_driver.ko

-----------------------------------------------------------------------------------------
once you extract the SDK package, you see the following directory contents


sdk
|-- VERSION
|-- demos
|   `-- mem_test_demo.c
|-- docs
|   |-- ARAD_JERICHO_ADDENDUM_KBP_SDK_1.4.10.pdf
|   |-- README_ARAD.txt
|   `-- RELEASE_NOTES_KBP_SDK_1.4.10.pdf
|-- include
|   `-- soc
|       `-- kbp
|           `-- alg_kbp
|               `-- include
|                   |-- ad.h
|                   |-- allocator.h
|                   |-- db.h
|                   |-- default_allocator.h
|                   |-- device.h
|                   |-- dma.h
|                   |-- error_tbl.def
|                   |-- errors.h
|                   |-- hw_limits.h
|                   |-- init.h
|                   |-- instruction.h
|                   |-- kbp_hb.h
|                   |-- kbp_legacy.h
|                   |-- kbp_mem_test.h
|                   |-- kbp_pcie.h
|                   |-- kbp_portable.h
|                   |-- key.h
|                   |-- xpt_12k.h
|                   |-- xpt_kaps.h
|                   `-- xpt_op.h
`-- src
    `-- soc
        `-- kbp
            `-- alg_kbp
                |-- bin
                |   |-- bin64
                |   |   `-- kbp_diags
                |   `-- kbp_diags
                |-- lib
                |   |-- lib64
                |   |   |-- libkbp_alg_only.a
                |   |   |-- libkbp_alg_only.so
                |   |   |-- libkbppcie.a
                |   |   |-- libkbppcie.so
                |   |   |-- libsoc_alg_kbp.a
                |   |   `-- libsoc_alg_kbp.so
                |   |-- libkbp_alg_only.a
                |   |-- libkbp_alg_only.so
                |   |-- libkbppcie.a
                |   |-- libkbppcie.so
                |   |-- libsoc_alg_kbp.a
                |   `-- libsoc_alg_kbp.so
                |-- portability
                |   |-- default_allocator.c
                |   `-- kbp_portable.c
                `-- xpt
                    `-- pcie
                        `-- device_driver
                            |-- Makefile
                            |-- README
                            |-- kbp_driver.c
                            `-- kbp_driver.h

19 directories, 45 files
