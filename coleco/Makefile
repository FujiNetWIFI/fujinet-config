# FujiNet CONFIG for the ColecoVision, in Z80 assembly.
#
# Standalone, like the other console CONFIGs: not part of the MekkoGX build
# above it. Needs zmac 1.3 (ZMAC=, else zmac on PATH, else
# ~/Workspace/zmac-1.3/zmac) and python3.
#
#   make            build/config.bin (32768 bytes, claim stamped, checked)
#   make install    into the pico tree's build/, where build-cart.sh
#                   bakes it into the cartridge firmware
#   make cart       install, then build that firmware
#   make run        MAME, windowed, against a fujinet-pc (see run.sh)
#   make drive      a scripted headless session: STEPS='t:HOSTS;k:fire;d'
#                   (emu/drive.lua lists the step language)
#   make clean
#
# PICO_COLECO is the cartridge tree, MAME_DIR a MAME with its fujinet
# cartridge device applied, FUJINET_TCP a fujinet-pc's BoIP listener.

PICO_COLECO ?= $(HOME)/Workspace/fujinet-firmware/pico/coleco
MAME_DIR    ?= $(HOME)/Workspace/mame
FUJINET_TCP ?= 127.0.0.1:9995
STEPS       ?= t:HOSTS;d;q
SECONDS     ?= 180

SOURCES = config.asm $(wildcard *.inc) font.txt tools/mkfont.py build.sh

.PHONY: all install cart run drive clean
all: build/config.bin

build/config.bin: $(SOURCES)
	./build.sh

install: build/config.bin
	mkdir -p $(PICO_COLECO)/build
	cp build/config.bin $(PICO_COLECO)/build/config.bin

cart: install
	cd $(PICO_COLECO) && ./build-cart.sh

run: build/config.bin
	./run.sh

# MAME runs from its own tree, or it ignores -autoboot_script.
drive: build/config.bin
	mkdir -p build/snap
	cd $(MAME_DIR) && DRIVE_STEPS='$(STEPS)' FUJINET_TCP=$(FUJINET_TCP) \
	  ./mame coleco -cartslot fujinet -cart $(CURDIR)/build/config.bin \
	  -video none -sound none -nothrottle -seconds_to_run $(SECONDS) \
	  -snapshot_directory $(CURDIR)/build/snap \
	  -autoboot_script $(CURDIR)/emu/drive.lua

clean:
	rm -rf build
