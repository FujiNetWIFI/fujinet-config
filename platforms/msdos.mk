PRODUCT_BASE = $(PRODUCT_BASE)
EXECUTABLE = $(R2R_PD)/$(PRODUCT_BASE).exe
LIBRARY = $(R2R_PD)/lib$(PRODUCT_BASE).$(PLATFORM).a

MWD := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))..)
include $(MWD)/common.mk
include $(MWD)/toolchains/ow2.mk

CFLAGS += -D__MSDOS__

r2r:: $(BUILD_EXEC) $(BUILD_LIB) $(R2R_EXTRA_DEPS_$(PLATFORM_UC))
	make -f $(PLATFORM_MK) $(PLATFORM)/r2r-post
