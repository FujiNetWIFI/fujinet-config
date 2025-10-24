EXECUTABLE = $(R2R_PD)/$(PRODUCT_BASE).bin
DISK = $(R2R_PD)/$(PRODUCT_BASE).vdk
LIBRARY = $(R2R_PD)/lib$(PRODUCT_BASE).$(PLATFORM).a
DISK_TOOL = dragondos

MWD := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))..)
include $(MWD)/common.mk
include $(MWD)/toolchains/cmoc.mk

r2r:: $(BUILD_DISK) $(BUILD_LIB) $(R2R_EXTRA_DEPS_$(PLATFORM_UC))
	@make -f $(PLATFORM_MK) $(PLATFORM)/r2r-post

$(BUILD_DISK): $(BUILD_EXEC) $(DISK_EXTRA_DEPS_$(PLATFORM_UC)) | $(R2R_PD)
	$(RM) $@
	$(DISK_TOOL) new $@ 360
	$(DISK_TOOL) insertbinary $@ $< 0x2601 0x2601
	@make -f $(PLATFORM_MK) $(PLATFORM)/disk-post
