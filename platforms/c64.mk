EXECUTABLE = $(R2R_PD)/$(PRODUCT).prg
DISK = $(R2R_PD)/$(PRODUCT).dsk

MWD := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))..)
include $(MWD)/common.mk
include $(MWD)/toolchains/cc65.mk

r2r:: $(EXECUTABLE) $(R2R_EXTRA_DEPS_$(PLATFORM_UC))
	make -f $(PLATFORM_MK) $(PLATFORM)/r2r-post
