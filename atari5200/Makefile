# FujiNet CONFIG for the Atari 5200.
#
# Standalone, like ../atari7800 and ../nes: a 32K client at $4000-$BFFF
# around the cart's mailbox, built with cc65's `atari5200` target and
# fujinet-lib-experimental's `atari5200` target (the mailbox transport, the
# crt0 that stands in for the BIOS, the linker config and the romstamp).
#
#   make            build/config.bin, what the cart bakes and MAME's -cart
#                   takes, checked
#   make install    copy it into the pico bring-up tree's build/
#   make cart       install, then that tree's build-cart.sh
#   make run        launch in MAME against a live fujinet-pc (its run.sh)
#   make clean
#
# Environment overrides:
#   FNLIB      fujinet-lib-experimental   (default ~/Workspace/fnlib-5200)
#   PICO_5200  the cartridge bring-up     (default ~/Workspace/fujinet-firmware/pico/atari-5200)

FNLIB     ?= $(HOME)/Workspace/fnlib-5200
PICO_5200 ?= $(HOME)/Workspace/fujinet-firmware/pico/atari-5200

SRCS = config.c st_wifi.c st_hosts.c st_files.c st_copy.c st_info.c st_boot.c \
       st_lobby.c fujiraw.c fujidisp.c fujiin.c fujiedit.c sfx.c antic.c
ASRCS = antic_dl.s
OBJS = $(patsubst %.c,build/%.o,$(SRCS)) $(patsubst %.s,build/%.o,$(ASRCS))

FNLIB_LIB = $(FNLIB)/r2r/atari5200/fujinet.atari5200.lib
CFG       = $(FNLIB)/makefiles/atari5200-fujinet.cfg
ROMSTAMP  = $(FNLIB)/makefiles/atari5200-romstamp.py

CFLAGS  = -t atari5200 -O --cpu 6502 -I$(FNLIB)/include -I$(FNLIB)/bus/atari5200
# The top 1K of RAM is the screen (antic.c).
LDFLAGS = -t atari5200 -C $(CFG) -Wl -D,__RESERVED_MEMORY__=0x400 -m build/config.map

all: build/config.bin

.DELETE_ON_ERROR:

$(FNLIB_LIB):
	$(MAKE) -C $(FNLIB) atari5200

build/%.o: %.c $(wildcard *.h) | build
	cl65 $(CFLAGS) -c -o $@ $<

build/antic_dl.o: antic_dl.s build/font.inc | build
	cl65 -t atari5200 --asm-include-dir build -c -o $@ $<

# The font, frame and bar tiles, as an ANTIC mode 2 character set.
build/font.inc: font.txt tools/mkfont.py | build
	python3 tools/mkfont.py font.txt $@

build:
	mkdir -p build

build/config.bin: $(OBJS) $(FNLIB_LIB) | build
	cl65 $(LDFLAGS) -o $@ $(OBJS) $(FNLIB_LIB)
	python3 $(ROMSTAMP) --map build/config.map $@
	python3 $(PICO_5200)/tools/checkrom.py --claim --map build/config.map $@

install: build/config.bin
	mkdir -p $(PICO_5200)/build
	cp build/config.bin $(PICO_5200)/build/

cart: install
	cd $(PICO_5200) && ./build-cart.sh

run: install
	cd $(PICO_5200) && ./run.sh config

clean:
	rm -rf build

.PHONY: all install cart run clean
