EXEC_SUFFIX = .com
DISK = $(R2R_PD)/$(PRODUCT_BASE).imd
LIBRARY = $(R2R_PD)/$(PRODUCT_BASE).$(PLATFORM).lib
DISK_TOOL = cpmcp
MKFS_TOOL = mkfs.cpm
CONVERT_TOOL = raw2imd
DISKDEF = ibm-3740

MWD := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))..)
include $(MWD)/common.mk
include $(MWD)/toolchains/z88dk.mk

MODEL2_FLAGS = +cpm
CFLAGS += $(MODEL2_FLAGS) -D__TRS80M2__
LDFLAGS += $(MODEL2_FLAGS)

r2r:: $(BUILD_DISK) $(BUILD_EXEC) $(BUILD_LIB) $(R2R_EXTRA_DEPS)
	make -f $(PLATFORM_MK) $(PLATFORM)/r2r-post

CPM_BLANK = blank-pt.img
CPM_DISK = $(CACHE_PLATFORM)/$(CPM_BLANK)

$(CPM_DISK): | $(CACHE_PLATFORM)
	$(call require,$(MKFS_TOOL),$(DISK_TOOL_INFO))
	dd if=/dev/zero of=$@.tmp bs=128 count=2002
	$(MKFS_TOOL) -f $(DISKDEF) $@.tmp
	mv $@.tmp $@


$(BUILD_DISK): $(DISK_EXECUTABLES) $(CPM_DISK) $(DISK_EXTRA_DEPS) $(DISK_EXTRA_FILES) | $(R2R_PD)
	$(call require,$(DISK_TOOL),$(DISK_TOOL_INFO))
	$(call require,$(CONVERT_TOOL),https://www.seasip.info/Unix/LibDsk/)
	cp $(CPM_DISK) $@.raw.tmp
	$(foreach f,$(DISK_EXECUTABLES) $(DISK_EXTRA_FILES),$(call copy-to-disk,,$(f),0:,$@.raw.tmp);)
	$(CONVERT_TOOL) -8 -c 77 -h 1 -s 26 -l 128 $@.raw.tmp $@
	rm -f $@.raw.tmp
	make -f $(PLATFORM_MK) $(PLATFORM)/disk-post

# Arguments:
# $1 == DISK_TOOL flags
# $2 == source file
# $3 == destination name
# $4 == temporary raw disk target
define copy-to-disk
  $(DISK_TOOL) -f $(DISKDEF) $1 $4 $2 $3
endef
