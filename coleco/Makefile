# FujiNet CONFIG for the ColecoVision.
#
# Standalone, like intv/ -- not wired into the MekkoGX framework above it.
# Builds with z88dk's +coleco target, os7lib (the OS7 BIOS bindings) and
# fujinet-lib-experimental's coleco target (the mailbox transport).
#
#   make            build/config.bin (stamped, exactly 32768 bytes)
#   make install    copy it into the pico bring-up tree, where build-cart.sh
#                   prefers it over the testrom stand-in
#   make cart       install + build the RP2040 firmware with it baked in
#   make clean
#
# Environment overrides:
#   Z88DK        z88dk root                     (default ~/Workspace/z88dk)
#   OS7LIB       os7lib checkout                (default ~/Workspace/os7lib)
#   FNLIB        fujinet-lib-experimental       (default ~/Workspace/fujinet-lib-experimental)
#   PICO_COLECO  the cartridge bring-up tree    (default ~/Workspace/fujinet-firmware/pico/coleco)

Z88DK       ?= $(HOME)/Workspace/z88dk
OS7LIB      ?= $(HOME)/Workspace/os7lib
FNLIB       ?= $(HOME)/Workspace/fujinet-lib-experimental
PICO_COLECO ?= $(HOME)/Workspace/fujinet-firmware/pico/coleco

export ZCCCFG = $(Z88DK)/lib/config
export PATH  := $(Z88DK)/bin:$(PATH)

APP  = config.c st_wifi.c st_hosts.c st_files.c st_copy.c st_info.c st_boot.c \
       st_lobby.c fujiraw.c
PLAT = fujidisp.c fujiin.c fujisnd.c fujiedit.c fujisplash.c fujisplash_data.c
SRCS = $(APP) $(PLAT)

FNLIB_LIB = $(FNLIB)/r2r/coleco/fujinet.coleco.lib

CFLAGS = +coleco -O2 -I. -I$(OS7LIB)/src -I$(FNLIB)/include -I$(FNLIB)/bus/coleco

# The link line is the pico bring-up's build.sh, which is a shipped game's
# plus the 32K/fence contract; see that script for the why of each pragma.
LDFLAGS = +coleco -m -s \
  -pragma-define:CRT_ORG_BSS=0x702C \
  -pragma-define:CRT_COLECO_SPRITE_NAME_SIZE=128 \
  -pragma-define:CRT_COLECO_SPRITE_ORDER_SIZE=32 \
  -pragma-define:CRT_COLECO_BIOS_BUFFER_SIZE=32 \
  -pragma-define:CRT_COLECO_BIOS_CONTROLLER_SIZE=12 \
  -pragma-define:CRT_ENABLE_STDIO=0 \
  -pragma-define:REGISTER_SP=0x73B8 \
  -pragma-define:CLIB_FOPEN_MAX=0 \
  -pragma-define:fputc_cons=0 \
  -pragma-define:CLIB_DEFAULT_SCREEN_MODE=-1 \
  -Cz--romsize=32768 -Cz--rombase=32768 \
  -Cz--code-fence=0xF800 -Cz--data-fence=0xF800 \
  -L$(OS7LIB) -los7 \
  -L$(FNLIB)/r2r/coleco -lfujinet.coleco.lib

all: build/config.bin

# A config.bin that failed the stamp or the RAM guard must not survive with a
# fresh mtime, or it would never be re-checked.
.DELETE_ON_ERROR:

$(FNLIB_LIB):
	$(MAKE) -C $(FNLIB) coleco/r2r

build/config.bin: $(SRCS) $(wildcard *.h) $(FNLIB_LIB)
	mkdir -p build
	zcc $(CFLAGS) $(LDFLAGS) $(SRCS) -o build/config -create-app
	@if [ -f build/config.rom ]; then mv build/config.rom $@; else mv build/config $@; fi
	@rm -f *.sym   # zcc -s writes per-source symbol files next to the sources
	python3 $(FNLIB)/makefiles/coleco-romstamp.py --stamp $@
	./bsscheck.py build/config.map

install: build/config.bin
	cp build/config.bin $(PICO_COLECO)/build/config.bin

cart: install
	cd $(PICO_COLECO) && ./build-cart.sh

clean:
	rm -rf build

.PHONY: all install cart clean
