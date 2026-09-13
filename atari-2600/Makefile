# CONFIG for the Atari 2600.
#
# Standalone, like ../intv/, ../coleco/ and ../channelf/: the shared src/ core
# assumes a keyboard, four device slots and kilobytes of buffers, and this
# console has a joystick, one device slot and 128 bytes of RAM.
ASL   ?= $(HOME)/asl/asl
P2BIN ?= $(HOME)/asl/p2bin

# The firmware tree owns two things this port must not fork: fuji_mailbox.h,
# which src/fujinet.inc mirrors and tools/checkdefs.py enforces, and emu/,
# the MAME cartridge device.
FN2600 ?= $(HOME)/Workspace/fujinet-firmware/pico/atari-2600

all: build/config.bin

build/config.bin: $(wildcard src/*.asm src/*.inc) build.sh tools/checkrom.py tools/checkdefs.py
	ASL=$(ASL) P2BIN=$(P2BIN) FN2600=$(FN2600) ./build.sh

# Bake the image into the cartridge firmware, the way ../intv/ does. CONFIG is
# the RP2040's own boot ROM: at power-up there is no network to load one from.
rom.h: build/config.bin
	python3 tools/mkromh.py $< > $(FN2600)/firmware/include/fujiconfigrom.h
	@echo "wrote $(FN2600)/firmware/include/fujiconfigrom.h"

clean:
	rm -rf build

.PHONY: all clean rom.h
