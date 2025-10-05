EXECUTABLE = $(R2R_PD)/$(PRODUCT).exe

MWD := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))..)
include $(MWD)/common.mk
include $(MWD)/toolchains/ow2.mk

CFLAGS += -D__MSDOS__

r2r:: $(EXECUTABLE) $(R2R_EXTRA_DEPS_$(PLATFORM_UC))
	make -f $(PLATFORM_MK) $(PLATFORM)/r2r-post
