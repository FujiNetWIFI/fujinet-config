PRODUCT = config
PLATFORMS += adam
PLATFORMS += apple2
PLATFORMS += atari
PLATFORMS += c64
PLATFORMS += coco
PLATFORMS += dragon

# Only in fujinet-lib-experimental currently
# Build them with make msdos-exp or make msxrom-exp.
#PLATFORMS += msdos
#PLATFORMS += msxrom

# Not currently in buildable state
#PLATFORMS += pc6001
#PLATFORMS += pc8801
#PLATFORMS += pmd85
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

# Dragon keeps the original 32-column code; the combo below still selects its fujinet-lib.
ifeq ($(PLATFORM),dragon)
SRC_DIRS = src src/dragon
endif

# FUJINET_LIB can be
# - a version number such as 4.7.6
# - a directory which contains the libs for each platform
# - a zip file with an archived fujinet-lib
# - a URL to a git repo
# - empty which will use whatever is the latest
#FUJINET_LIB = https://github.com/FozzTexx/fujinet-lib-experimental.git
FUJINET_LIB =

# Define extra dirs ("combos") that expand with a platform.
# Format: platform+=combo1,combo2
PLATFORM_COMBOS = \
  c64+=commodore \
  atarixe+=atari \
  msxrom+=msx \
  msxdos+=msx \
  dragon+=coco

include makefiles/toplevel-rules.mk
include atari-fastloader.mk

########################################
# Common things

CFLAGS = -DBUILD_$(PLATFORM_UC)
CFLAGS_EXTRA_CC65 = -Os
CFLAGS_EXTRA_Z88DK = -Os

########################################
# CoCo customization

CFLAGS_EXTRA_COCO = -Wno-assign-in-condition
AUTOEXEC_COCO = dist.coco/autoexec.bas
# logo_zx0.asm is shared with Dragon; LOGO.BIN is loaded via BASIC's LOADM
# (see coco/disk-post), not over DriveWire.
CFGLOAD_COCO = src/coco/cfgload/cfgload.c src/coco/cfgload/logo_zx0.asm
CFGLOAD_BIN = r2r/coco/cfgload.bin
LOGO_BIN_COCO = r2r/coco/LOGO.BIN
DISK_EXTRA_DEPS_COCO := $(AUTOEXEC_COCO) $(CFGLOAD_BIN) $(LOGO_BIN_COCO)

$(CFGLOAD_BIN):: $(CFGLOAD_COCO) | $(R2R_PD)
	cmoc -o $@ $^

# DECB .bin format, not --dragon: LOADM expects the DECB header.
$(LOGO_BIN_COCO):: src/coco/cfgload/logo_data_coco.asm | $(R2R_PD)
	lwasm --decb -o $@ $<

# $1 == decb flags
# $2 == source file
# $3 == disk image
define coco-copy
  DEST="$$(basename $2 | tr '[:lower:]' '[:upper:]')" ; \
    decb copy $1 $2 $3,$${DEST}
endef

ifeq ($(PLATFORM),coco)
HIRESTXT_LIB = 0.5.1.7
endif

# CoCo apps: WIFI -> MAIN <-> FILES (browse, slot select, copy); MAIN <-> WIFI (info).
# They hand off through struct entry_data (src/coco/launch/entry.h) and link explicit
# object lists, since the SRC_DIRS glob would merge them into one program. All three
# load at $2600, above the hirestxt/splash buffer ($0E00-$25FF); --limit=7BF8 keeps
# each below the handoff data.
CO = build/config/coco/src
COCO_COMMON_OBJS = $(CO)/debug.o $(CO)/fuji_compat.o $(CO)/coco/launch/runm.o \
  $(CO)/coco/launch/entry.o $(CO)/coco/bar.o $(CO)/coco/color.o $(CO)/coco/die.o $(CO)/coco/input.o \
  $(CO)/coco/pause.o $(CO)/coco/screen.o $(CO)/coco/strendswith.o $(CO)/coco/system.o
COCO_WIFI_OBJS = $(CO)/coco/wifi/wifi.o $(CO)/check_wifi.o $(CO)/connect_wifi.o \
  $(CO)/set_wifi.o $(CO)/show_info.o $(CO)/coco/screen_wifi.o $(CO)/coco/input_wifi.o
COCO_MAIN_OBJS = $(CO)/coco/cfg/main.o $(CO)/hosts_and_devices.o $(CO)/coco/mount_and_boot.o \
  $(CO)/coco/screen_slots.o $(CO)/coco/screen_hosts.o $(CO)/coco/input_hosts.o
COCO_FILES_OBJS = $(CO)/coco/files/main.o $(CO)/coco/files/globals.o $(CO)/select_file.o \
  $(CO)/select_slot.o $(CO)/destination_host_slot.o $(CO)/perform_copy.o \
  $(CO)/coco/scroll.o $(CO)/coco/screen_slots.o $(CO)/coco/screen_files.o \
  $(CO)/coco/input_files.o
COCO_SPLIT_BINS = r2r/coco/wifi.bin r2r/coco/main.bin r2r/coco/files.bin
ifeq ($(PLATFORM),coco)
DISK_EXECUTABLES = $(COCO_SPLIT_BINS)
endif

r2r/coco/wifi.bin: $(COCO_WIFI_OBJS) $(COCO_COMMON_OBJS)
r2r/coco/main.bin: $(COCO_MAIN_OBJS) $(COCO_COMMON_OBJS)
r2r/coco/files.bin: $(COCO_FILES_OBJS) $(COCO_COMMON_OBJS)

$(COCO_SPLIT_BINS):
	@mkdir -p r2r/coco
	$(LD) -o $@ --org=2600 --limit=7BF8 $^ $(LIBS)

coco/disk-post::
	$(call coco-copy,-t -0,$(AUTOEXEC_COCO),$(DISK))
	$(call coco-copy,-b -2,$(CFGLOAD_BIN),$(DISK))
	$(call coco-copy,-b -2,$(LOGO_BIN_COCO),$(DISK))


########################################
# Apple II customization

A2_LINKER_CFG = src/apple2/config.cfg
EXECUTABLE_EXTRA_DEPS_APPLE2 = $(A2_LINKER_CFG)
LDFLAGS_EXTRA_APPLE2 = -C $(A2_LINKER_CFG)
WITHOUT_PRODOS_BOOT = dist.apple2/bootable.po

apple2/disk-post::
	cp $(WITHOUT_PRODOS_BOOT) $(BUILD_DISK)
	ac -as $(BUILD_DISK) $(PRODUCT_BASE).SYSTEM < $(BUILD_EXEC)
#	ac -p $(BUILD_DISK) $(PRODUCT_BASE).SYSTEM SYS 0x2000 < $(BUILD_EXEC)

########################################
# Atari customization

ATARI_LINKER_CFG = src/atari/atari.cfg
EXECUTABLE_EXTRA_DEPS_ATARI = $(ATARI_LINKER_CFG)
LDFLAGS_EXTRA_ATARI = -C $(ATARI_LINKER_CFG)
EXTRA_INCLUDE_ATARI = src/atari/asminc

########################################
# Commodore 64 customization

CFLAGS_EXTRA_C64 = -DUSE_EDITSTRING

########################################
# Dragon customization

CFLAGS_EXTRA_DRAGON = --verbose -Wno-assign-in-condition --dragon -DDRAGON
# Boot chain: AUTOLOAD.DWL (cfgload.bin, splash) -> STAGE2.DWL -> CONFIG.DWL.
# Two loader stages because cfgload.bin (splash + ZX0 decompressor) is too
# big to sit at a low org reliably on real hardware; stage2.c is a tiny
# fetch-and-launch stub that can sit low instead. CONFIG.DWL's org must
# clear stage2's actual footprint (see STAGE2_DWL_DRAGON below), and
# dwload_clone() must tail-jump rather than call into the next stage or
# the launched program inherits a stack it corrupts on exit.
LDFLAGS_EXTRA_DRAGON = --dragon --verbose --limit=7fff --org=1600 -i

dragon/disk-post::
	cp $(CFGLOAD_BIN_DRAGON) $(R2R_PD)/AUTOLOAD.DWL
	cp $(R2R_PD)/config.bin $(R2R_PD)/CONFIG.DWL


CFGLOAD_DRAGON = src/coco/cfgload/cfgload.c src/coco/cfgload/dwload_cmoc.c src/coco/cfgload/logo_zx0.asm
CFGLOAD_BIN_DRAGON = r2r/dragon/cfgload.bin
LOGO_DWL_DRAGON = r2r/dragon/LOGO.DWL
STAGE2_DRAGON = src/coco/cfgload/stage2.c src/coco/cfgload/dwload_cmoc.c
STAGE2_DWL_DRAGON = r2r/dragon/STAGE2.DWL
DISK_EXTRA_DEPS_DRAGON := $(CFGLOAD_BIN_DRAGON) $(LOGO_DWL_DRAGON) $(STAGE2_DWL_DRAGON)

# limit must clear cfgload's own usage and stay under SCREEN_BUFFER (0x6600);
# LOGO_SCRATCH in cfgload.c must sit at or above this limit.
$(CFGLOAD_BIN_DRAGON):: $(CFGLOAD_DRAGON) | $(R2R_PD)
	cmoc -i -DDRAGON --dragon --limit=2600 --org=1600 -o $@ $^

# logo_data.asm's org must match LOGO_SCRATCH in cfgload.c.
$(LOGO_DWL_DRAGON):: src/coco/cfgload/logo_data.asm | $(R2R_PD)
	lwasm --dragon -o $@ $<

# limit=1600 is also CONFIG.DWL's own org (LDFLAGS_EXTRA_DRAGON) -- that
# memory is free for CONFIG.DWL once stage2 has fetched and executed it.
$(STAGE2_DWL_DRAGON):: $(STAGE2_DRAGON) | $(R2R_PD)
	cmoc -i -DDRAGON --dragon --limit=6600 --org=c00 -o $@ $^

########################################
# Adam customization

LDFLAGS_EXTRA_ADAM = -lndos

########################################
# MSX customization

LDFLAGS_EXTRA_MSXROM = -pragma-redirect:fputc_cons=fputc_cons_generic -pragma-redirect:CRT_FONT=_font_shifted -Ca-Isrc/msx/header

########################################
# MS-DOS customization

FUJINET_MSDOS_REPO = https://github.com/FujiNetWIFI/fujinet-msdos.git
FUJINET_MSDOS_CACHE = $(CACHE_DIR)/fujinet-msdos

msdos/disk-post::
	@if [ ! -d $(FUJINET_MSDOS_CACHE) ]; then \
	  git clone $(FUJINET_MSDOS_REPO) $(FUJINET_MSDOS_CACHE); \
	fi
	$(MAKE) -C $(FUJINET_MSDOS_CACHE) clean disk
	mkdir -p $(CACHE_DIR)/msdos-files
	mcopy -i $(FUJINET_MSDOS_CACHE)/fn-msdos.img '::*' $(CACHE_DIR)/msdos-files/
	mcopy -i $(DISK) $(CACHE_DIR)/msdos-files/* '::/'
	mcopy -i $(DISK) dist.msdos/AUTOEXEC.BAT '::/AUTOEXEC.BAT'

FUJINET_LIB_EXP_REPO = https://github.com/FozzTexx/fujinet-lib-experimental.git

msdos-exp::
	$(MAKE) FUJINET_LIB=$(FUJINET_LIB_EXP_REPO) PLATFORMS=$(@:-exp=) $(@:-exp=)

########################################
# MSX customization

# fujinet-lib-experimental main regressed the MSX transport in 902032d
# ("Changed fuji_bus_call to use varargs"): sccz80 mis-passes the arguments, so the
# header leaves the MSX corrupted - device 0x70 arrives as 0x10 and command
# 0xEA as 0xC4 - and the firmware never answers, so CONFIG sits on the splash
# spinning on IO_STATUS. Pin to the last commit known to talk to the firmware.
# When that is fixed upstream, drop this and fold msxrom back into msdos-exp.
# The pin has to live here because $(CACHE_DIR) is gitignored, so a checkout
# made by hand in the cache would silently revert to main the next time the
# cache is cleared.
MSXROM_FNLIB_REV = b0a7959
MSXROM_FNLIB_CACHE = $(CACHE_DIR)/fujinet-lib-msx-$(MSXROM_FNLIB_REV)

msxrom-exp::
	@if [ ! -d $(MSXROM_FNLIB_CACHE) ]; then \
	  git clone $(FUJINET_LIB_EXP_REPO) $(MSXROM_FNLIB_CACHE) && \
	  git -C $(MSXROM_FNLIB_CACHE) checkout --detach $(MSXROM_FNLIB_REV); \
	fi
	$(MAKE) -C $(MSXROM_FNLIB_CACHE) msx/r2r
	$(MAKE) FUJINET_LIB=$(abspath $(MSXROM_FNLIB_CACHE)) PLATFORMS=msxrom msxrom
