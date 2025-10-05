EXECUTABLE = $(R2R_PD)/$(PRODUCT).com
DISK = $(R2R_PD)/$(PRODUCT).atr

MWD := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))..)
include $(MWD)/common.mk
include $(MWD)/toolchains/cc65.mk

r2r:: $(DISK) $(R2R_EXTRA_DEPS_$(PLATFORM_UC))
	make -f $(PLATFORM_MK) $(PLATFORM)/r2r-post

PICOBOOT_BIN = picoboot.bin
ATRBOOT := $(CACHE_PLATFORM)/$(PICOBOOT_BIN)

$(DISK): $(EXECUTABLE) $(ATRBOOT) $(DISK_EXTRA_DEPS_$(PLATFORM_UC)) | $(R2R_PD)
	$(RM) $@
	$(MKDIR_P) $(CACHE_PLATFORM)/disk
	cp $< $(CACHE_PLATFORM)/disk
	dir2atr -m -S -B $(ATRBOOT) $@ $(CACHE_PLATFORM)/disk
	make -f $(PLATFORM_MK) $(PLATFORM)/disk-post

PICOBOOT_DOWNLOAD_URL = https://github.com/FujiNetWIFI/assets/releases/download/picobin
$(ATRBOOT): | $(CACHE_PLATFORM)
	curl -L -o $@ $(PICOBOOT_DOWNLOAD_URL)/$(PICOBOOT_BIN)
