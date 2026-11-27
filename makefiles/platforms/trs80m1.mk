EXEC_SUFFIX = .cmd
LIBRARY = $(R2R_PD)/$(PRODUCT_BASE).$(PLATFORM).lib

MWD := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))..)
include $(MWD)/common.mk
include $(MWD)/toolchains/z88dk.mk

TRS80M1_FLAGS = +trs80 -subtype=disk
CFLAGS += $(TRS80M1_FLAGS) -D__TRS80M1__
LDFLAGS += $(TRS80M1_FLAGS)
ifneq ($(IS_LIBRARY),1)
  LDFLAGS += -create-app
endif

r2r:: $(BUILD_EXEC) $(BUILD_LIB) $(R2R_EXTRA_DEPS)
	make -f $(PLATFORM_MK) $(PLATFORM)/r2r-post
