EXECUTABLE = $(R2R_PD)/$(PRODUCT_BASE).com
DISK = $(R2R_PD)/$(PRODUCT_BASE).atr
LIBRARY = $(R2R_PD)/$(PRODUCT_BASE).$(PLATFORM).lib

MWD := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))..)
include $(MWD)/common.mk
include $(MWD)/toolchains/cc65.mk

r2r:: $(BUILD_DISK) $(BUILD_LIB) $(R2R_EXTRA_DEPS_$(PLATFORM_UC))
	make -f $(PLATFORM_MK) $(PLATFORM)/r2r-post

PICOBOOT_BIN = picoboot.bin
ATRBOOT := $(CACHE_PLATFORM)/$(PICOBOOT_BIN)

$(BUILD_DISK): $(BUILD_EXEC) $(ATRBOOT) $(DISK_EXTRA_DEPS_$(PLATFORM_UC)) | $(R2R_PD)
	$(RM) $@
	$(RM) -rf $(CACHE_PLATFORM)/disk
	$(MKDIR_P) $(CACHE_PLATFORM)/disk
	cp $< $(CACHE_PLATFORM)/disk
	dir2atr -m -S -B $(ATRBOOT) $@ $(CACHE_PLATFORM)/disk
	make -f $(PLATFORM_MK) $(PLATFORM)/disk-post

PICOBOOT_DOWNLOAD_URL = https://github.com/FujiNetWIFI/assets/releases/download/picobin
$(ATRBOOT): | $(CACHE_PLATFORM)
	curl -L -o $@ $(PICOBOOT_DOWNLOAD_URL)/$(PICOBOOT_BIN)
