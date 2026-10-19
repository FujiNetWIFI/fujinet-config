EXEC_SUFFIX = .bin
LIBRARY = $(R2R_PD)/$(PRODUCT_BASE).$(PLATFORM).lib

MWD := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))..)
include $(MWD)/common.mk
include $(MWD)/toolchains/z88dk.mk

# A raw 8080 image for the S-100 card to load at 0100h and run with no OS
S100_FLAGS = +z80 -clib=8080
CFLAGS += $(S100_FLAGS) -D__S100__
ASFLAGS += -m8080
LDFLAGS += $(S100_FLAGS) -zorg=0x100

r2r:: $(BUILD_EXEC) $(BUILD_LIB) $(R2R_EXTRA_DEPS)
	make -f $(PLATFORM_MK) $(PLATFORM)/r2r-post
