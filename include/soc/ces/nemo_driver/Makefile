#
# $Id$
#
# Copyright 2011 BATM
#
include $(ROOT_DIR)/make.dir


TGT_DIR	= C:/Synergy/BetaIstt/binos
LIB_BASE_NAME   = nd_drv
LIB_SHORT = ces_$(LIB_BASE_NAME)
LIB_FULL = lib$(LIB_SHORT).so
LIB_NAME = lib$(LIB_SHORT).a
PROG = fdbTest




SUBDIRS =

MODULE_ROOT=$(ROOT_DIR)/nemo_driver

#LIBS
NEMO_DRIVER_INC_DIR = $(MODULE_ROOT)/pub

PRE_INCLUDE=     \
                 -I$(NEMO_DRIVER_INC_DIR) \
		 -I$(MODULE_ROOT)  


#----------------------------------------------------------------------------
# COMPILE macros
#----------------------------------------------------------------------------
#CFLAGS	+= -Wall -I. -Itarget_h -I- -D_GNU_SOURCE -D_REENTRANT
#CFLAGS	+= -Wall   -I-
#BATMITT
#CFLAGS += -DBC1_BOARD
CFLAGS += $(CES_CFLAGS)


.c.o:
	$(CC) -g -O0   $(CFLAGS)  $(GEN_INCLUDE)  $(PRE_INCLUDE)  -c $*.c
##-mfpu=vfp -mfpu=fpa



#----------------------------------------------------------------------------
# Make the program...
#----------------------------------------------------------------------------
C_FILES_EXCLUDE =  nd_test.c

C_FILES_ALL = $(wildcard *.c)
C_FILES_EXCLUDE_GO = $(foreach file,$(C_FILES_EXCLUDE),$(dir $(file))$(notdir $(file)))
C_FILES_ALL_GO = $(foreach file,$(C_FILES_ALL),$(dir $(file))$(notdir $(file)))
C_FILES_TODO = $(filter-out $(C_FILES_EXCLUDE_GO),$(C_FILES_ALL_GO))

OBJS = $(notdir $(C_FILES_TODO:.c=.o))






all:	$(LIB_SHORT)
# $(PROG) 

$(LIB_FULL): $(OBJS) Makefile
	$(CC)  $(CFLAGS) $(OBJS) -o $(LIB_FULL) 
#	$(CC) -shared $(CFLAGS) $(OBJS) -o $(LIB_FULL) -lpthread


$(LIB_SHORT): $(OBJS) 
	$(AR) crs $(LIB_DIR)/$(LIB_NAME) $(OBJS)


$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS)  -o $(PROG) -L. libv2lin.so

#----------------------------------------------------------------------------
# Compile modules w/ Inference rules
#----------------------------------------------------------------------------
clean:
	rm -f $(OBJS) $(PROG) $(LIB_FULL)

depend:
	makedepend -s "# DO NOT DELETE" -- *.c

# DO NOT DELETE THIS LINE -- make depend depends on it.


