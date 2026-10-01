# FujiNet CONFIG for the NES.
#
# Standalone, like ../coleco and ../intv: a 32K NROM client with 8K CHR-ROM
# (cc65's own conio font) and the cartridge's 8K WRAM at $6000, built with
# cc65's `nes` target and fujinet-lib-experimental's `nes` target (the
# mailbox transport, the FujiNet linker config and the claim stamp).
#
#   make            build/config.nes (stamped, RMW-checked)
#   make install    copy it into the pico bring-up tree, where build-cart.sh
#                   prefers it over the testrom stand-in
#   make cart       install + build the RP2354B firmware with it baked in
#   make run        launch in MAME against a live fujinet-pc (see run.sh)
#   make clean
#
# Environment overrides:
#   FNLIB      fujinet-lib-experimental       (default ~/Workspace/fujinet-lib-experimental)
#   PICO_NES   the cartridge bring-up tree    (default ~/Workspace/fujinet-firmware/pico/nes)

FNLIB    ?= $(HOME)/Workspace/fujinet-lib-experimental
PICO_NES ?= $(HOME)/Workspace/fujinet-firmware/pico/nes

SRCS = config.c st_wifi.c st_hosts.c st_files.c st_info.c st_boot.c \
       fujiraw.c fujidisp.c fujiin.c fujiedit.c
OBJS = $(patsubst %.c,build/%.o,$(SRCS))

FNLIB_LIB = $(FNLIB)/r2r/nes/fujinet.nes.lib
NES_CFG   = $(FNLIB)/makefiles/nes-fujinet.cfg
ROMSTAMP  = $(FNLIB)/makefiles/nes-romstamp.py

CFLAGS  = -t nes -O --cpu 6502 -DBUILD_NES -I$(FNLIB)/include -I$(FNLIB)/bus/nes
LDFLAGS = -t nes -C $(NES_CFG) -m build/config.map

all: build/config.nes

.DELETE_ON_ERROR:

$(FNLIB_LIB):
	$(MAKE) -C $(FNLIB) nes

build/%.o: %.c $(wildcard *.h) | build
	cl65 $(CFLAGS) -c -o $@ $<

build:
	mkdir -p build

build/config.nes: $(OBJS) $(FNLIB_LIB) | build
	cl65 $(LDFLAGS) -o $@ $(OBJS) $(FNLIB_LIB)
	python3 $(ROMSTAMP) --stamp $@
	python3 $(PICO_NES)/tools/checkrom.py --claim --map build/config.map $@

install: build/config.nes
	cp build/config.nes $(PICO_NES)/build/config.nes

cart: install
	cd $(PICO_NES) && ./build-cart.sh

run: install
	cd $(PICO_NES) && ./run.sh config

clean:
	rm -rf build

.PHONY: all install cart run clean
