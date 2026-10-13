EXEC_SUFFIX = .com
DISK = $(R2R_PD)/$(PRODUCT_BASE).h8d
LIBRARY = $(R2R_PD)/$(PRODUCT_BASE).$(PLATFORM).lib
DISK_TOOL = z88dk-appmake
DISK_TOOL_INFO = https://github.com/z88dk/z88dk

MWD := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))..)
include $(MWD)/common.mk
include $(MWD)/toolchains/z88dk.mk

MSX_FLAGS = +cpm -create-app
CFLAGS += $(MSX_FLAGS) -D__H89__
LDFLAGS += $(MSX_FLAGS)
ifneq ($(IS_LIBRARY),1)
  LDFLAGS += -create-app
endif

r2r:: $(BUILD_DISK) $(BUILD_EXEC) $(BUILD_LIB) $(R2R_EXTRA_DEPS)
	make -f $(PLATFORM_MK) $(PLATFORM)/r2r-post

# A 100K H17 disk in Heath CP/M's format (hz89). appmake names the -b file
# after its host file, so the first executable is copied in under its CP/M
# name; everything else goes on with -a host:CPMNAME.
cpm-name = $(shell echo $(notdir $(1)) | tr '[:lower:]' '[:upper:]')
DISK_BOOT_NAME = $(call cpm-name,$(DISK_BOOT_EXEC))
DISK_ADD_FILES = $(filter-out $(DISK_BOOT_EXEC),$(DISK_EXECUTABLES)) $(DISK_EXTRA_FILES)

$(BUILD_DISK): $(DISK_EXECUTABLES) $(DISK_EXTRA_DEPS) $(DISK_EXTRA_FILES) | $(R2R_PD)
	$(call require,$(DISK_TOOL),$(DISK_TOOL_INFO))
	$(RM) -r $(CACHE_PLATFORM)/disk
	$(MKDIR_P) $(CACHE_PLATFORM)/disk
	cp $(DISK_BOOT_EXEC) $(CACHE_PLATFORM)/disk/$(DISK_BOOT_NAME)
	cd $(CACHE_PLATFORM)/disk && $(DISK_TOOL) +cpmdisk -f hz89 --container raw --no-com-file \
	  -b $(DISK_BOOT_NAME) $(foreach f,$(DISK_ADD_FILES),-a $(abspath $(f)):$(call cpm-name,$(f))) -o disk
	$(MWD)/hz89-dir-fixup.py $(CACHE_PLATFORM)/disk/disk
	mv $(CACHE_PLATFORM)/disk/disk $@
	make -f $(PLATFORM_MK) $(PLATFORM)/disk-post
