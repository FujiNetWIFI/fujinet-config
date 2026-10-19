EXEC_SUFFIX = .com
LIBRARY = $(R2R_PD)/$(PRODUCT_BASE).$(PLATFORM).lib

MWD := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))..)
include $(MWD)/common.mk
include $(MWD)/toolchains/z88dk.mk

# 8080 code, so it runs on an Altair or IMSAI as well as a Z80 card
S100_FLAGS = +cpm -clib=8080
CFLAGS += $(S100_FLAGS) -D__S100__
ASFLAGS += -m8080
LDFLAGS += $(S100_FLAGS)
ifneq ($(IS_LIBRARY),1)
  LDFLAGS += -create-app
endif

r2r:: $(BUILD_EXEC) $(BUILD_LIB) $(R2R_EXTRA_DEPS)
	make -f $(PLATFORM_MK) $(PLATFORM)/r2r-post
