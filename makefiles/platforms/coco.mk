EXECUTABLE = $(R2R_PD)/$(PRODUCT_BASE).bin
DISK = $(R2R_PD)/$(PRODUCT_BASE).dsk
LIBRARY = $(R2R_PD)/lib$(PRODUCT_BASE).$(PLATFORM).a
DISK_TOOL = decb

MWD := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))..)
include $(MWD)/common.mk
include $(MWD)/toolchains/cmoc.mk

r2r:: $(BUILD_DISK) $(BUILD_LIB) $(R2R_EXTRA_DEPS_$(PLATFORM_UC))
	@make -f $(PLATFORM_MK) $(PLATFORM)/r2r-post

$(BUILD_DISK): $(BUILD_EXEC) $(DISK_EXTRA_DEPS_$(PLATFORM_UC)) | $(R2R_PD)
	$(RM) $@
	$(DISK_TOOL) dskini $@
	$(DISK_TOOL) copy -b -2 $< $@,$(shell echo $(PRODUCT_BASE) | tr '[:lower:]' '[:upper:]').BIN
	@make -f $(PLATFORM_MK) $(PLATFORM)/disk-post
