# FujiNet CONFIG for the Bally Astrocade.
#
# Self-contained: the Z80 mailbox transport (fujilib.inc), display helpers
# (fujidisp.inc) and HVGLIB.H are vendored under lib/ (copies of the
# bring-up tree's testrom/ files -- keep them in step), and the layout
# check / firmware packer live under tools/. Only the assembler comes from
# outside; ZMAC points at any zmac 1.3 on PATH, the standalone build in
# ~/Workspace/zmac-1.3, or the bring-up's vendored tools/zmac.
#
#   make            build/config.bin (8K, claim stamped, checkrom-clean)
#   make rom.h      fujiconfigrom.h, to copy over the cart firmware's
#                   include/fujiconfigrom.h, then rebuild that firmware
#   make run        launch in MAME against a live fujinet-pc (see run.sh)

FUJI_PICO ?= $(HOME)/Workspace/fujinet-firmware/pico/astrocade

# zmac: PATH first, then the standalone build, then the bring-up's copy.
ZMAC ?= $(shell command -v zmac 2>/dev/null || \
          ls $(HOME)/Workspace/zmac-1.3/zmac 2>/dev/null || \
          echo $(FUJI_PICO)/tools/zmac/zmac)

SOURCES = config.asm fujicmd.inc input.inc font.inc ui.inc edit.inc \
          wifi.inc hosts.inc browse.inc copy.inc info.inc boot.inc
STAGED  = lib/fujilib.inc lib/fujidisp.inc lib/HVGLIB.H

WINDOW    = 8192
ROM_TOP   = 6912
CLAIM_OFF = 7420

.PHONY: all clean run rom.h
all: build/config.bin

build/config.raw: $(SOURCES) $(STAGED)
	mkdir -p build/stage
	cp $(SOURCES) build/stage/
	cp $(STAGED) build/stage/
	cd build/stage && '$(ZMAC)' -o ../config.raw -x ../config.lst config.asm
	@size=$$(wc -c < build/config.raw); \
	 echo "config.raw: $$size of $(ROM_TOP) bytes"; \
	 if [ $$size -gt $(ROM_TOP) ]; then \
	   echo "over the 0x1B00 ROM top" >&2; exit 1; fi

build/config.bin: build/config.raw
	{ cat build/config.raw; \
	  head -c $$(( $(WINDOW) - $$(wc -c < build/config.raw) )) /dev/zero; } > $@
	printf 'FUJI' | dd of=$@ bs=1 seek=$(CLAIM_OFF) conv=notrunc status=none
	python3 tools/checkrom.py $@

rom.h: build/config.bin
	python3 tools/mkromh.py build/config.bin > fujiconfigrom.h
	@echo "Wrote fujiconfigrom.h -- copy over the cart firmware's"
	@echo "include/fujiconfigrom.h and rebuild it to bake this in."

run: build/config.bin
	./run.sh

clean:
	rm -rf build fujiconfigrom.h
