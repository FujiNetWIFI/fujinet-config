PRODUCT = config
PLATFORMS = coco apple2 atari c64 adam pmd85

# Not currently in buildable state
#PLATFORMS += dragon
#PLATFORMS += msdos
#PLATFORMS += pc6001
#PLATFORMS += pc8801
#PLATFORMS += rc2014

# Require special toolchains
#PLATFORMS += apple2cda
#PLATFORMS += apple2gs

# You can run 'make <platform>' to build for a specific platform,
# or 'make <platform>/<target>' for a platform-specific target.
# Example shortcuts:
#   make coco        → build for coco
#   make apple2/disk → build the 'disk' target for apple2

# SRC_DIRS may use the literal %PLATFORM% token.
# It expands to the chosen PLATFORM plus any of its combos.
SRC_DIRS = src src/%PLATFORM%

# FUJINET_LIB can be
# - a version number such as 4.7.6
# - a directory which contains the libs for each platform
# - a zip file with an archived fujinet-lib
# - a URL to a git repo
# - empty which will use whatever is the latest
# - undefined, no fujinet-lib will be used
FUJINET_LIB =

# Some platforms don’t use FUJINET_LIB; set this to allow builds to continue
# even if the library isn’t present.
FUJINET_LIB_OPTIONAL = 1

# Define extra dirs ("combos") that expand with a platform.
# Format: platform+=combo1,combo2
PLATFORM_COMBOS = \
  c64+=commodore \
  atarixe+=atari \
  msxrom+=msx \
  msxdos+=msx \
  dragon+=coco

include makefiles/toplevel-rules.mk

########################################
# Common things

CFLAGS = -DBUILD_$(PLATFORM_UC)
CFLAGS_EXTRA_CC65 = -Os
CFLAGS_EXTRA_Z88DK = -Os

########################################
# CoCo customization

AUTOEXEC_COCO = dist.coco/autoexec.bas
CFGLOAD_COCO = src/coco/cfgload/cfgload.asm
CFGLOAD_BIN = r2r/coco/cfgload.bin
DISK_EXTRA_DEPS_COCO := $(AUTOEXEC_COCO) $(CFGLOAD_BIN)

$(CFGLOAD_BIN):: $(CFGLOAD_COCO) | $(R2R_PD)
	lwasm -b -9 -o $@ $<

# $1 == decb flags
# $2 == source file
# $3 == disk image
define coco-copy
  DEST="$$(basename $2 | tr '[:lower:]' '[:upper:]')" ; \
    decb copy $1 $2 $3,$${DEST}
endef

coco/disk-post::
	$(call coco-copy,-t -0,$(AUTOEXEC_COCO),$(DISK))
	$(call coco-copy,-b -2,$(CFGLOAD_BIN),$(DISK))

########################################
# Apple II customization

A2_LINKER_CFG = src/apple2/config.cfg
EXECUTABLE_EXTRA_DEPS_APPLE2 = $(A2_LINKER_CFG)
LDFLAGS_EXTRA_APPLE2 = -C $(A2_LINKER_CFG)

########################################
# Atari customization

EXTRA_INCLUDE_ATARI = src/atari/asminc

########################################
# Commodore 64 customization

CFLAGS_EXTRA_C64 = -DUSE_EDITSTRING

########################################
# CoCo customization

CFLAGS_EXTRA_COCO = -Wno-assign-in-condition
