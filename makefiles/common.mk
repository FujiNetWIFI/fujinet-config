define pop
  $(wordlist 1, $(shell echo $$(($(words $(1)) - 1))), $(1))
endef

# Automatically figure out PLATFORM from the .mk file that included us
PLATFORM_MK := $(call pop,$(MAKEFILE_LIST))
PLATFORM := $(basename $(notdir $(lastword $(PLATFORM_MK))))
PLATFORM_UC := $(shell echo "$(PLATFORM)" | tr '[:lower:]' '[:upper:]')
$(info Building for PLATFORM=$(PLATFORM))

include $(MWD)/../Makefile

# Add GIT_VERSION macro define to CFLAGS, includes tag if available,
# short commit hash, appends '*' if changes haven't been commited
CFLAGS += -DGIT_VERSION='"$(shell git rev-parse --short HEAD)$(shell git status --porcelain | grep -q '^[ MADRCU]' && echo '*')"'

IS_LIBRARY := $(if $(filter %.lib,$(PRODUCT)),1,0)
ifeq ($(IS_LIBRARY),1)
  PRODUCT_BASE = $(basename $(PRODUCT))
  BUILD_LIB = $(LIBRARY)
else
  PRODUCT_BASE = $(PRODUCT)
  BUILD_EXEC = $(EXECUTABLE)
  BUILD_DISK = $(DISK)
endif

# Only set DEFAULT if the specific tool is non-empty
ifneq ($(strip $(CC_$(TOOLCHAIN_UC))),)
CC_DEFAULT = $(CC_$(TOOLCHAIN_UC))
endif

ifneq ($(strip $(AS_$(TOOLCHAIN_UC))),)
AS_DEFAULT = $(AS_$(TOOLCHAIN_UC))
endif

ifneq ($(strip $(LD_$(TOOLCHAIN_UC))),)
LD_DEFAULT = $(LD_$(TOOLCHAIN_UC))
endif

ifneq ($(strip $(PC_$(TOOLCHAIN_UC))),)
PC_DEFAULT = $(PC_$(TOOLCHAIN_UC))
endif

R2R_PD := $(R2R_DIR)/$(PLATFORM)
OBJ_DIR := $(BUILD_DIR)/$(PLATFORM)
CACHE_PLATFORM := $(CACHE_DIR)/$(PLATFORM)
MKDIR_P ?= mkdir -p

# Expand PLATFORM_COMBOS entries into a lookup form
#   c64+=commodore,eightbit -> c64 commodore eightbit
# PLATFORM_COMBOS is a flat list of entries like "dragon+=coco"
# $1 = the platform to expand
get_combos = $(foreach e,$(PLATFORM_COMBOS),\
  $(if $(filter $1+=%, $(e)), $(lastword $(subst +=, ,$(e)))))

# Expands patterns with %PLATFORM% to the platform + its combos
expand_platform_pattern = \
  $(foreach d,$(1), \
    $(if $(findstring %PLATFORM%,$(d)), \
      $(foreach p,$(PLATFORM) $(call get_combos,$(PLATFORM)), \
        $(subst %PLATFORM%,$(p),$(d))), \
      $(d)))

# The fully expanded list of source directories
SRC_DIRS_EXPANDED := $(call expand_platform_pattern,$(SRC_DIRS))

# Find all the CFILES and AFILES
CFILES := $(foreach dir,$(SRC_DIRS_EXPANDED),$(wildcard $(dir)/*.c))
AFILES := $(foreach dir,$(SRC_DIRS_EXPANDED),$(wildcard $(dir)/*.s)) \
          $(foreach dir,$(SRC_DIRS_EXPANDED),$(wildcard $(dir)/*.asm))
PFILES := $(foreach dir,$(SRC_DIRS_EXPANDED),$(wildcard $(dir)/*.pas))

# Need two steps: AFILES may be .s or .asm; `make` swaps one suffix at a time
NORM_AFILES := $(AFILES:.asm=.s)
OBJS := $(addprefix $(OBJ_DIR)/, $(notdir $(CFILES:.c=.o) $(NORM_AFILES:.s=.o) $(PFILES:.pas=.o)))

$(BUILD_EXEC):: $(OBJS) $(EXECUTABLE_EXTRA_DEPS_$(PLATFORM_UC)) | $(R2R_PD)
	$(call link-bin,$@,$(OBJS))
	@$(MAKE) -f $(PLATFORM_MK) $(PLATFORM)/executable-post

$(BUILD_LIB):: $(OBJS) $(LIBRARY_EXTRA_DEPS_$(PLATFORM_UC)) | $(R2R_PD)
	$(call link-lib,$@,$(OBJS))
	@$(MAKE) -f $(PLATFORM_MK) $(PLATFORM)/library-post

# auto-created dirs
AUTO_DIRS := $(OBJ_DIR) $(R2R_PD) $(CACHE_PLATFORM)
$(AUTO_DIRS):
	$(MKDIR_P) $@

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(call compile,$@,$<)
$(OBJ_DIR)/%.o: %.s | $(OBJ_DIR)
	$(call assemble,$@,$<)
$(OBJ_DIR)/%.o: %.asm | $(OBJ_DIR)
	$(call assemble,$@,$<)
$(OBJ_DIR)/%.o: %.pas | $(OBJ_DIR)
	$(call compile-pas,$@,$<)

vpath %.c $(SRC_DIRS_EXPANDED)
vpath %.s $(SRC_DIRS_EXPANDED)
vpath %.asm $(SRC_DIRS_EXPANDED)
vpath %.pas $(SRC_DIRS_EXPANDED)

.PHONY: clean debug r2r $(PLATFORM)/r2r disk $(PLATFORM)/disk

clean::
	rm -rf $(OBJ_DIR) $(CACHE_PLATFORM) $(R2R_PD)

debug::
	echo 'What should debug target do?'
	exit 1

# These targets allow adding platform-specific steps from the top-level Makefile.
# Examples:
#   coco/r2r:: coco/custom-step1
#   coco/r2r:: coco/custom-step2
# or with a single colon:
#   apple2/r2r: apple2/custom-step1 apple2/custom-step2
# The double-colon form appends without overwriting existing deps.
r2r:: $(PLATFORM)/r2r
disk:: $(PLATFORM)/disk

# Fallback rule so every <platform>/disk-post target exists.
# Does nothing by default (@:).
# In the top-level Makefile you can extend it with the literal platform name,
# e.g.:
#   coco/disk-post:: coco/extra-file
$(PLATFORM)/disk-post::
	@:

# Same as $(PLATFORM)/disk-post above
$(PLATFORM)/r2r-post::
	@:

# Same as $(PLATFORM)/disk-post above
$(PLATFORM)/executable-post::
	@:

# Same as $(PLATFORM)/disk-post above
$(PLATFORM)/library-post::
	@:

# include autodeps
-include $(wildcard $(OBJ_DIR)/*.d)

FUJINET_LIB ?= __UNDEFINED__
ifeq ($(FUJINET_LIB),__UNDEFINED__)
  $(info FUJINET_LIB not defined)
else
  # Fill in the FUINET_LIB_* variables by calling fnlib.py. It's a bit
  # messy because of workarounds needed for dealing with newlines and
  # and the $(eval)
  define _newline


  endef
  $(eval $(subst |,$(_newline),$(shell PLATFORM=$(PLATFORM) CACHE_DIR=$(CACHE_DIR) \
      PLATFORM_COMBOS="$(PLATFORM_COMBOS)" $(MWD)/fnlib.py \
        $(if $(FUJINET_LIB_OPTIONAL),--skip-if-missing) \
        $(FUJINET_LIB) | tr '\n' '|')))
  ifeq ($(strip $(FUJINET_LIB_LDLIB)),)
    ifeq ($(FUJINET_LIB_OPTIONAL),)
      $(error fujinet-lib not available)
    else
      $(info fujinet-lib not available, but skipping because FUJINET_LIB_SKIP_MISSING is set)
    endif
  endif
endif # FUJINET_LIB
