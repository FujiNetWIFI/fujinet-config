PRODUCT = config
PLATFORMS += adam
PLATFORMS += apple2
PLATFORMS += atari
PLATFORMS += c64
PLATFORMS += coco
PLATFORMS += dragon

# Only in lib-experimental currently
# Use make-exp <platform> to build them.
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

# FUJINET_LIB can be
# - a version number such as 4.7.6
# - a directory which contains the libs for each platform
# - a zip file with an archived fujinet-lib
# - a URL to a git repo
# - empty which will use whatever is the latest
# - undefined, no fujinet-lib will be used
#FUJINET_LIB = https://github.com/FozzTexx/fujinet-lib-experimental.git
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
include atari-fastloader.mk

########################################
# Common things

CFLAGS = -DBUILD_$(PLATFORM_UC)
CFLAGS_EXTRA_CC65 = -Os
CFLAGS_EXTRA_Z88DK = -Os

########################################
# CoCo customization

LDFLAGS_EXTRA_COCO = --org=0E00 --limit=7C00
AUTOEXEC_COCO = dist.coco/autoexec.bas
# logo_zx0.asm (the ZX0 decompressor) is shared with Dragon. CoCo's logo
# (LOGO.BIN) is loaded via BASIC's own LOADM (see coco/disk-post and
# AUTOEXEC_COCO) rather than over DriveWire like Dragon's LOGO.DWL: on
# real CoCo 2 hardware the DriveWire mount request never reached the
# FujiNet server at all (see LOGO_SCRATCH's comment in cfgload.c), even
# though the DW ROM vectors are correct and this works fine in emulation.
# A prior attempt read it from within cfgload.bin itself via disk.c's
# DSKCON-based file access; that's no longer needed now that LOADM
# handles it, so dwload_cmoc.c/disk.c aren't linked into CoCo's
# cfgload.bin at all -- CONFIG.BIN hand-off already goes through BASIC's
# RUNM, not DWLOAD.
CFGLOAD_COCO = src/coco/cfgload/cfgload.c src/coco/cfgload/logo_zx0.asm
CFGLOAD_BIN = r2r/coco/cfgload.bin
LOGO_BIN_COCO = r2r/coco/LOGO.BIN
DISK_EXTRA_DEPS_COCO := $(AUTOEXEC_COCO) $(CFGLOAD_BIN) $(LOGO_BIN_COCO)

# No explicit --org/--limit here (matches this rule's prior behavior).
# LOGO_SCRATCH in cfgload.c must stay comfortably above cfgload.bin's
# actual program_end -- check the map after changing any of these
# sources. Was `cmoc -o $@ $<`, which silently only ever compiled
# cfgload.c itself ($< is only the first prerequisite) -- other sources
# were listed as prerequisites but never actually passed to the
# compiler. Using $^ now that there's more than one real source file to
# compile.
$(CFGLOAD_BIN):: $(CFGLOAD_COCO) | $(R2R_PD)
	cmoc -o $@ $^

# Same compressed logo bytes as Dragon's LOGO.DWL, but built in CoCo's
# actual DECB .bin format (lwasm --decb), NOT --dragon: DragonDOS binary
# format uses a different header layout ($55 <type> <load hi/lo> <len
# hi/lo> <exec hi/lo> $AA), which is fine for Dragon's LOGO.DWL since
# dwload_clone() parses that header itself, but LOADM (see
# coco/disk-post) expects the DECB header CFGLOAD.BIN/CONFIG.BIN
# actually use ($00 <len hi/lo> <load hi/lo> ... $FF $00 $00 <exec
# hi/lo>) to find the load address. See LOGO_SCRATCH's comment in
# cfgload.c and logo_data_coco.asm for the full story.
$(LOGO_BIN_COCO):: src/coco/cfgload/logo_data_coco.asm | $(R2R_PD)
	lwasm --decb -o $@ $<

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
	$(call coco-copy,-b -2,$(LOGO_BIN_COCO),$(DISK))
	@# LOGO.BIN is on the disk image (unlike Dragon's LOGO.DWL, which is
	@# a loose r2r/dragon/ output file fetched over DriveWire at
	@# runtime): AUTOEXEC_COCO's LOADM"LOGO" reads it through the normal
	@# DECB directory, same as CFGLOAD.BIN itself.


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
# CoCo customization

CFLAGS_EXTRA_COCO = -Wno-assign-in-condition

########################################
# Dragon customization

CFLAGS_EXTRA_DRAGON = --verbose -Wno-assign-in-condition --dragon -DDRAGON
#
# Boot chain, in order:
#   AUTOLOAD.DWL (cfgload.bin, org=1600) -- draws the splash logo, then
#     fetches and executes STAGE2.DWL.
#   STAGE2.DWL (org=c00) -- fetches and executes CONFIG.DWL. Nothing else.
#   CONFIG.DWL (config.bin, org=1600) -- the actual config app.
#
# Why two loader stages: cfgload.bin needs pmode/pcls/screen/the ZX0
# decompressor to draw the splash, which makes it too big to sit at a low
# org reliably -- org values below e00 caused instability on real
# hardware for a program that size (see git history/comments below for
# what was tried). But CONFIG.DWL's own org just needs to clear whatever
# program is *actively downloading it*, and that doesn't need to be the
# splash program. So cfgload.bin hands off to a second, minimal program
# (stage2.c) that does nothing but fetch+launch CONFIG.DWL, and can sit
# much lower/tighter than cfgload.bin ever could.
#
# Two separate constraints on CONFIG.DWL's org, don't conflate them:
#   1. It must clear STAGE2's *actual* peak memory usage (code+stack) at
#      the moment STAGE2 is downloading it -- same overlap hazard as
#      cfgload.bin/STAGE2 overlapping their own downloads (see their
#      rules below). STAGE2's own --limit is a generous nominal ceiling,
#      not its real usage (~1586 bytes total), so org here doesn't need
#      to exactly equal STAGE2's --limit -- it just needs comfortable
#      margin above STAGE2's real footprint.
#   2. See dwload_clone()'s handling of the execute case in
#      dwload_cmoc.c: launching the next stage must be a true jump, not a
#      call, or the launched program inherits a stack it'll eventually
#      corrupt and crash on exit (this was the "crashes after config.dwl
#      exits, but only when chain-loaded" bug). This is independent of
#      constraint 1 above -- fixing it didn't remove the need for org to
#      clear STAGE2's footprint, it only fixed what happens when
#      config.dwl later calls exit().
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

# org: this is cfgload.bin (the splash/logo program), not the program
# that ends up bounding CONFIG.DWL's org anymore (see above) -- so it
# doesn't need to be tightly budgeted, just clearly safe. History: e00
# was the original stable value; c00 and 600 were both tried later to
# free up more room for CONFIG.DWL directly and both caused instability
# on real hardware for a program cfgload.bin's size, before the
# exit()-corrupts-the-stack bug (see dwload_clone() in dwload_cmoc.c) was
# understood -- it's possible some of that instability was actually this
# same bug rather than a memory-map issue, so revisit with fresh eyes if
# retrying a lower value here. Limit just needs to clear cfgload's own
# actual usage (~2300 bytes) with a comfortable margin and
# stay under SCREEN_BUFFER (0x6600); LOGO_SCRATCH in cfgload.c must sit
# at or above this limit for the same reason STAGE2 must sit at or above
# stage2's own limit (see below).
$(CFGLOAD_BIN_DRAGON):: $(CFGLOAD_DRAGON) | $(R2R_PD)
	cmoc -i -DDRAGON --dragon --limit=2600 --org=1600 -o $@ $^

# The compressed splash logo lives in its own DriveWire-servable file
# instead of being embedded in cfgload.bin, so it doesn't count against
# the loader's size budget. logo_data.asm's org (see that file) must match
# LOGO_SCRATCH in cfgload.c.
$(LOGO_DWL_DRAGON):: src/coco/cfgload/logo_data.asm | $(R2R_PD)
	lwasm --dragon -o $@ $<

# stage2.c: the tiny second-stage loader that actually fetches CONFIG.DWL
# (see the long comment above). Actual usage as of this writing is ~1586
# bytes (org=c00 -> program_end=0x1232); limit=1600 leaves a ~974-byte
# stack margin and is also CONFIG.DWL's own org (LDFLAGS_EXTRA_DRAGON) --
# that memory is free for CONFIG.DWL to reuse once stage2 has already
# fetched and executed it, same pattern as LOGO_SCRATCH above.
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

msxrom-exp msdos-exp::
	$(MAKE) FUJINET_LIB=https://github.com/FozzTexx/fujinet-lib-experimental.git PLATFORMS=$(@:-exp=) $(@:-exp=)
