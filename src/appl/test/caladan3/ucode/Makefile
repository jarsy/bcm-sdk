# Makefile,v 1.2 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
# $Id: Makefile,v 1.6 2012/09/24 15:23:22 rgodwin Exp $
# Makefile for Caladan3 tests Library
#
LOCALDIR = src/appl/test/caladan3/ucode

#
# Java Settings
#
JAVAPATH ?= /tools/bin
JAVA = $(JAVAPATH)/java -Xmx1024M

ifeq ($(SDK),)
$(error SDK env not set, Please run this within SDK tree)
endif

#
# ASM3 Tools Path Settings
#
CALADAN3_TOOLS_PATH ?= $(SDK)/../caladan_ucode/tools/java_tools
CLASSPATH = $(CALADAN3_TOOLS_PATH)/lib

ifeq "$(wildcard $(CLASSPATH)/*.jar)" ""
$(error Cannot find ASM tools at $(CLASSPATH), check your CALADAN3_TOOLS_PATH)
endif
#
# ASM3 Settings -- DO NOT MODIFY
#
ASM3_LIBS += antlr-3.3-complete.jar c3tools.jar
ASM3_LIBS_EXPANDED = $(addprefix $(CLASSPATH)/, $(ASM3_LIBS))

#
# ASM3 Commands
#
BLANK = $(eval) $(eval)
CALADAN3_CLASSPATH = -classpath $(subst $(BLANK),:,$(ASM3_LIBS_EXPANDED))
BRCM_MDE_PKG_PREFIX = com.broadcom.mde
CALADAN3_ASM = $(JAVA) $(CALADAN3_CLASSPATH) $(BRCM_MDE_PKG_PREFIX).c3plc.Asm3 

TMU_TAPS_SRC = tmu_taps_ut
TMU_TAPS1_SRC = tmu_taps1_ut
TMU_TAPSP_SRC = tmu_tapsp_ut
TMU_TAPS_IPV6_SRC = tmu_taps_ipv6_ut
TMU_HASH_SRC = tmu_hash_ut
TMU_HASH144_SRC = tmu_hash144_ut
TMU_DM_SRC = tmu_dm_ut

tmutaps: $(TMU_TAPS_SRC).asm
	@echo "Assembling $(TMU_TAPS_SRC).asm"
	@$(CALADAN3_ASM) -ia $(TMU_TAPS_SRC).asm -o1 $(TMU_TAPS_SRC).txt

tmutaps1: $(TMU_TAPS1_SRC).asm
	@echo "Assembling $(TMU_TAPS1_SRC).asm"
	@$(CALADAN3_ASM) -ia $(TMU_TAPS1_SRC).asm -o1 $(TMU_TAPS1_SRC).txt

tmutapsp: $(TMU_TAPSP_SRC).asm
	@echo "Assembling $(TMU_TAPS1P_SRC).asm"
	@$(CALADAN3_ASM) -ia $(TMU_TAPSP_SRC).asm -o1 $(TMU_TAPSP_SRC).txt

tmutapsipv6: $(TMU_TAPS_IPV6_SRC).asm
	@echo "Assembling $(TMU_TAPS_IPV6_SRC).asm"
	@$(CALADAN3_ASM) -ia $(TMU_TAPS_IPV6_SRC).asm -o1 $(TMU_TAPS_IPV6_SRC).txt

tmuhash: $(TMU_HASH_SRC).asm
	@echo "Assembling $(TMU_HASH_SRC).asm"
	@$(CALADAN3_ASM) -ia $(TMU_HASH_SRC).asm -o1 $(TMU_HASH_SRC).txt

tmuhash144: $(TMU_HASH144_SRC).asm
	@echo "Assembling $(TMU_HASH144_SRC).asm"
	@$(CALADAN3_ASM) -ia $(TMU_HASH144_SRC).asm -o1 $(TMU_HASH144_SRC).txt

tmudm: $(TMU_DM_SRC).asm
	@echo "Assembling $(TMU_DM_SRC).asm"
	@$(CALADAN3_ASM) -ia $(TMU_DM_SRC).asm -o1 $(TMU_DM_SRC).txt

all: tmutaps tmuhash tmuhash144 tmudm tmutaps1 tmutapsipv6


clean:
	@echo "Cleaning Test ucode area"
	@rm -f *.txt
