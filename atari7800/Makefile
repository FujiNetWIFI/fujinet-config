# FujiNet CONFIG for the Atari 7800.
#
# Standalone, like ../nes and ../coleco: a 32K client at $8000 with the
# cart's 16K of RAM at $4000, built with cc65's `atari7800` target and
# fujinet-lib-experimental's `atari7800` target (the mailbox transport, the
# crt0 that leaves INPTCTRL unlocked, the linker config and the romstamp).
#
#   make            build/config.bin (what the cart bakes) and build/config.a78
#                   (what MAME's -cart takes), stamped and checked
#   make install    copy both into the pico bring-up tree's build/
#   make cart       install, then that tree's build-cart.sh if it has one
#   make run        launch in MAME against a live fujinet-pc (its run.sh)
#   make clean
#
# Environment overrides:
#   FNLIB      fujinet-lib-experimental   (default ~/Workspace/fujinet-lib-experimental)
#   PICO_7800  the cartridge bring-up     (default ~/Workspace/fujinet-firmware/pico/atari-7800)

FNLIB     ?= $(HOME)/Workspace/fujinet-lib-experimental
PICO_7800 ?= $(HOME)/Workspace/fujinet-firmware/pico/atari-7800

SRCS = config.c st_wifi.c st_hosts.c st_files.c st_copy.c st_info.c st_boot.c \
       st_lobby.c fujiraw.c fujidisp.c fujiin.c fujiedit.c sfx.c
ASRCS = maria.s
OBJS = $(patsubst %.c,build/%.o,$(SRCS)) $(patsubst %.s,build/%.o,$(ASRCS)) \
       build/tiles.o

FNLIB_LIB = $(FNLIB)/r2r/atari7800/fujinet.atari7800.lib
CFG       = $(FNLIB)/makefiles/atari7800-fujinet.cfg
ROMSTAMP  = $(FNLIB)/makefiles/atari7800-romstamp.py

CFLAGS  = -t atari7800 -O --cpu 6502 -I$(FNLIB)/include -I$(FNLIB)/bus/atari7800
LDFLAGS = -t atari7800 -C $(CFG) -m build/config.map

all: build/config.a78

.DELETE_ON_ERROR:

$(FNLIB_LIB):
	$(MAKE) -C $(FNLIB) atari7800

build/%.o: %.c $(wildcard *.h) | build
	cl65 $(CFLAGS) -c -o $@ $<

build/%.o: %.s | build
	cl65 -t atari7800 -c -o $@ $<

# The font and frame tiles, as MARIA 320B two-byte characters.
build/tiles.s: font.txt tools/mktiles.py | build
	python3 tools/mktiles.py font.txt $@

build/tiles.o: build/tiles.s
	cl65 -t atari7800 -c -o $@ $<

build:
	mkdir -p build

build/config.bin: $(OBJS) $(FNLIB_LIB) | build
	cl65 $(LDFLAGS) -o $@ $(OBJS) $(FNLIB_LIB)
	python3 $(ROMSTAMP) --stamp --map build/config.map $@
	python3 $(PICO_7800)/tools/checkrom.py --claim --notia --map build/config.map $@

build/config.a78: build/config.bin
	python3 $(PICO_7800)/tools/mka78.py $< $@ "FUJINET CONFIG"

install: build/config.a78
	mkdir -p $(PICO_7800)/build
	cp build/config.bin build/config.a78 $(PICO_7800)/build/

cart: install
	if [ -x $(PICO_7800)/build-cart.sh ]; then \
	    cd $(PICO_7800) && ./build-cart.sh; \
	else \
	    echo "no $(PICO_7800)/build-cart.sh yet: installed only"; \
	fi

run: install
	cd $(PICO_7800) && ./run.sh config

clean:
	rm -rf build

.PHONY: all install cart run clean
