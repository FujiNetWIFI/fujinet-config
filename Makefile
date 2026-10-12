PRODUCT = config
PLATFORMS += adam
PLATFORMS += apple2
PLATFORMS += atari
PLATFORMS += c64
PLATFORMS += coco
PLATFORMS += dragon
PLATFORMS += msdos

# Won't build until a fujinet-lib release includes merged PR FujiNetWIFI/fujinet-lib#125
#PLATFORMS += msxrom

# Not currently in buildable state
#PLATFORMS += pc6001
#PLATFORMS += pc8801
#PLATFORMS += pmd85
#PLATFORMS += rc2014

# Require special toolchains
#PLATFORMS += apple2cda
#PLATFORMS += apple2gs

# Need a fujinet-lib with the S-100 bus, not in a release yet:
#   make FUJINET_LIB=<fujinet-lib checkout> s100-cpm/r2r s100-rom/r2r
#PLATFORMS += s100-cpm
#PLATFORMS += s100-rom

# Heathkit H89 needs a fujinet-lib with H89 support, which no release has yet.
# Build them with make h89 (or make h89-cpm/r2r, make h89-hdos/r2r).
#PLATFORMS += h89-cpm
#PLATFORMS += h89-hdos

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
# Pinned, because finding the latest release goes through the GitHub API,
# whose rate limit CI runs hit.
FUJINET_LIB = 5.0.0

# The H89 lib also needs 9e6d4dd, which stops each program start from
# rebooting the ESP32 (and dropping what CONFIG just mounted).
# FUJINET_LIB=<dir> on the command line overrides this.
FUJINET_LIB_H89 ?= https://github.com/tschak909/fujinet-lib.git\#h89-keep-esp32-running
ifneq ($(filter h89-cpm h89-hdos,$(PLATFORM)),)
  FUJINET_LIB = $(FUJINET_LIB_H89)
endif

# Define extra dirs ("combos") that expand with a platform.
# Format: platform+=combo1,combo2
PLATFORM_COMBOS = \
  c64+=commodore \
  atarixe+=atari \
  msxrom+=msx \
  msxdos+=msx \
  dragon+=coco \
  s100-cpm+=s100 \
  s100-rom+=s100,s100-cpm \
  h89-cpm+=h89 \
  h89-hdos+=h89

include makefiles/toplevel-rules.mk
include atari-fastloader.mk

########################################
# Common things

# h89-cpm is BUILD_H89_CPM: a hyphen can't be part of a macro name.
CFLAGS = -DBUILD_$(subst -,_,$(PLATFORM_UC))
CFLAGS_EXTRA_CC65 = -Os
CFLAGS_EXTRA_Z88DK = -Os

########################################
# CoCo customization

# RUNM keeps the caller's stack, so every app resets S or each hop sinks it toward $7BF8.
# --initial-s only takes effect when compiling main().
CFLAGS_EXTRA_COCO = -Wno-assign-in-condition --initial-s=7F00
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
  $(CO)/coco/pause.o $(CO)/coco/screen.o $(CO)/coco/strendswith.o $(CO)/coco/system.o \
  $(CO)/coco/text_hires.o
COCO_WIFI_OBJS = $(CO)/coco/wifi/wifi.o $(CO)/check_wifi.o $(CO)/connect_wifi.o \
  $(CO)/set_wifi.o $(CO)/show_info.o $(CO)/coco/screen_wifi.o $(CO)/coco/input_wifi.o
COCO_MAIN_OBJS = $(CO)/coco/cfg/main.o $(CO)/hosts_and_devices.o $(CO)/coco/mount_and_boot.o \
  $(CO)/coco/screen_slots.o $(CO)/coco/screen_hosts.o $(CO)/coco/input_hosts.o
COCO_FILES_OBJS = $(CO)/coco/files/main.o $(CO)/coco/files/globals.o $(CO)/select_file.o \
  $(CO)/select_slot.o $(CO)/destination_host_slot.o $(CO)/perform_copy.o \
  $(CO)/coco/scroll.o $(CO)/coco/screen_slots.o $(CO)/coco/screen_files.o \
  $(CO)/coco/input_files.o
COCO_SPLIT_BINS = r2r/coco/wifi.bin r2r/coco/main.bin r2r/coco/files.bin

# CoCo 3 apps (WIFI3, MAIN3, FILES3): the same sources with -DCOCO3, which swaps the
# hirestxt backend for the GIME one (src/coco/text_gime.c). cfgload picks the set.
CO3 = build/config/coco3/src
COCO3_COMMON_OBJS = $(subst $(CO)/,$(CO3)/,$(subst text_hires,text_gime,$(COCO_COMMON_OBJS)))
COCO3_SPLIT_BINS = r2r/coco/wifi3.bin r2r/coco/main3.bin r2r/coco/files3.bin

$(CO3)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -DCOCO3 --intdir=$(dir $@) -o $@ $<

ifeq ($(PLATFORM),coco)
DISK_EXECUTABLES = $(COCO_SPLIT_BINS) $(COCO3_SPLIT_BINS)
endif

r2r/coco/wifi.bin: $(COCO_WIFI_OBJS) $(COCO_COMMON_OBJS)
r2r/coco/main.bin: $(COCO_MAIN_OBJS) $(COCO_COMMON_OBJS)
r2r/coco/files.bin: $(COCO_FILES_OBJS) $(COCO_COMMON_OBJS)
r2r/coco/wifi3.bin: $(subst $(CO)/,$(CO3)/,$(COCO_WIFI_OBJS)) $(COCO3_COMMON_OBJS)
r2r/coco/main3.bin: $(subst $(CO)/,$(CO3)/,$(COCO_MAIN_OBJS)) $(COCO3_COMMON_OBJS)
r2r/coco/files3.bin: $(subst $(CO)/,$(CO3)/,$(COCO_FILES_OBJS)) $(COCO3_COMMON_OBJS)

$(COCO_SPLIT_BINS) $(COCO3_SPLIT_BINS):
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

########################################
# S-100 customization

# s100-cpm is CONFIG.COM; s100-rom is CONFIG.BIN, the card's image 0, which
# its boot page runs before any OS. One BUILD_ macro covers both.
ifneq ($(filter s100-cpm s100-rom,$(PLATFORM)),)
CFLAGS = -DBUILD_S100
endif
CFLAGS_EXTRA_S100-ROM = -DS100_ROM
LDFLAGS_EXTRA_S100-CPM = -pragma-define:CRT_COMMANDLINE_REDIRECTION=0 -pragma-define:CRT_TRIM_BSS=1
# src/s100/rom/crt_preamble.asm sets the stack before the CRT runs
LDFLAGS_EXTRA_S100-ROM = -pragma-define:CRT_INCLUDE_PREAMBLE=1 -Ca-I$(CURDIR)/src/s100/rom \
  -pragma-define:CRT_TRIM_BSS=1

# CRT_TRIM_BSS leaves BSS out of the image, which the CRT clears anyway,
# and writes it to a file of zeros nothing needs
s100-cpm/executable-post::
	rm -f $(R2R_PD)/$(PRODUCT_BASE)_BSS.bin

s100-rom/executable-post::
	rm -f $(R2R_PD)/$(PRODUCT_BASE)_BSS.bin
	cp $(R2R_PD)/$(PRODUCT_BASE).bin $(R2R_PD)/CONFIG.BIN

########################################
# Heathkit H89 (HDOS and CP/M) customization

# Both builds share src/h89 (PLATFORM_COMBOS above).
CFLAGS_EXTRA_H89-CPM = -DBUILD_H89
CFLAGS_EXTRA_H89-HDOS = -DBUILD_H89
LDFLAGS_EXTRA_H89-CPM = -m
# HDOS loads programs from 2280H (USERFWA). z88dk's HDOS startup links them
# at 4480H instead (a misreading of Heath's split octal 042.200), which wastes
# 8.7K and leaves too little room once a driver like FN: has loaded. Link at
# 2A80H, with the stack in the 2K under the program.
LDFLAGS_EXTRA_H89-HDOS = -m -pragma-define:CRT_ORG_CODE=0x2A80 -pragma-define:REGISTER_SP=0x2A80
H89_APPMAKE ?= z88dk-appmake

# CONFIG.ABS alone on a 100K H17 disk: MOUNT SY1: then RUN SY1:CONFIG.ABS.
# appmake wants the bare program, so the 8-byte ABS header comes off and its
# load address (which HDOS also uses as the entry) goes to --org.
h89-hdos/r2r-post::
	@mkdir -p $(CACHE_PLATFORM)
	tail -c +9 $(R2R_PD)/config.abs > $(CACHE_PLATFORM)/config.bin
	cd $(CACHE_PLATFORM) && rm -f config.h8d && $(H89_APPMAKE) +hdos -b config.bin \
	  --org $$(od -An -tu2 -j2 -N2 $(CURDIR)/$(R2R_PD)/config.abs | tr -d ' ') -o CONFIG
	mv $(CACHE_PLATFORM)/config.h8d $(R2R_PD)/config-hdos.h8d

# CONFIG.COM alone on a 100K H17 CP/M disk (hz89 format): B:CONFIG.
h89-cpm/r2r-post::
	@mkdir -p $(CACHE_PLATFORM)/disk
	cp $(R2R_PD)/config.com $(CACHE_PLATFORM)/disk/CONFIG.COM
	cd $(CACHE_PLATFORM)/disk && rm -f disk && $(H89_APPMAKE) +cpmdisk -f hz89 \
	  --container raw --no-com-file -b CONFIG.COM -o disk
	python3 -I dist.h89/cpm-dir-fixup.py $(CACHE_PLATFORM)/disk/disk
	mv $(CACHE_PLATFORM)/disk/disk $(R2R_PD)/config-cpm.h8d

h89:
	$(MAKE) h89-cpm/r2r
	$(MAKE) h89-hdos/r2r
.PHONY: h89
