# FujiNet CONFIG for the Emerson Arcadia 2001.
#
# Self-contained: the image tools (mkimage, checkrom, checkdepth, mkromh,
# checksize) are vendored under tools/ (copies of the bring-up tree's and
# fujinet-battleship/arcadia's -- keep them in step). Only Macroassembler
# AS comes from outside: asl on PATH, else ~/asl.
#
#   make            build/config.bin (8K, claim stamped, every check clean)
#   make run        launch in MAME against a live fujinet-pc (see run.sh)
#   make drive      a scripted headless session: STEPS='t:HOSTS;k:fire;d'
#                   (emu/drive.lua lists the step language)
#   make smoke      boot, the host list, the first folder
#   make shot       BARTEST build + snapshots of the widget test screens
#   make rom.h      fujiconfigrom.h, to copy over the cart firmware's
#                   firmware/include/fujiconfigrom.h, then rebuild it
#   make install    build/config.bin into the bring-up tree's build/, where
#                   its build-cart.sh picks it up first

FUJI_PICO   ?= $(HOME)/Workspace/fujinet-firmware/pico/arcadia
MAME_DIR    ?= $(HOME)/Workspace/mame
FUJINET_TCP ?= 127.0.0.1:9995
STEPS       ?= t:HOSTS;d;q

SOURCES = config.asm $(wildcard *.inc) build.sh

MAMEHL = FUJINET_TCP=$(FUJINET_TCP) $(MAME_DIR)/mame arcadia \
	  -cartslot fujinet -cart $(CURDIR)/build/config.bin \
	  -video none -sound none -nothrottle -snapshot_directory build/snap \
	  -cfg_directory build/cfg -nvram_directory build/nvram

.PHONY: all run drive smoke shot rom.h install clean
all: build/config.bin

build/config.bin: $(SOURCES)
	./build.sh

run: build/config.bin
	./run.sh

drive: build/config.bin
	DRIVE_STEPS='$(STEPS)' $(MAMEHL) -autoboot_script emu/drive.lua \
	  -seconds_to_run 120

smoke: build/config.bin
	DRIVE_STEPS='t:HOSTS;d;k:fire;t:1 UP;d;q' $(MAMEHL) \
	  -autoboot_script emu/drive.lua -seconds_to_run 60

shot:
	BARTEST=1 ./build.sh
	$(MAME_DIR)/mame arcadia -cart $(CURDIR)/build/config.bin \
	  -autoboot_script emu/shot.lua -video none -sound none -nothrottle \
	  -snapshot_directory build/snap -cfg_directory build/cfg
	rm -f build/config.bin           # never leave a test build behind

rom.h: build/config.bin
	python3 tools/mkromh.py build/config.bin > fujiconfigrom.h
	@echo "Wrote fujiconfigrom.h -- copy over the cart firmware's"
	@echo "firmware/include/fujiconfigrom.h and rebuild it to bake this in."

install: build/config.bin
	mkdir -p $(FUJI_PICO)/build
	cp build/config.bin $(FUJI_PICO)/build/config.bin

clean:
	rm -rf build fujiconfigrom.h
