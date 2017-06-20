# $Id: Makefile,v 1.41 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Compile sample LED driver programs
#
#
.KEEP_STATE:

.SUFFIXES:	.asm .hex

ASM = ../../tools/ledasm

SRCS = $(wildcard *.asm)
OBJS = $(patsubst %.asm,%.hex,$(SRCS))

all: $(OBJS)

remake: clobber all

%.hex: %.asm
	$(ASM) $*

clean:
	rm -f *.lst

clobber: clean
	rm -f *.hex
	rm -f *.c
