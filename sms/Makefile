# FujiNet CONFIG for the Sega Master System.
#
# Standalone like ../coleco and ../nes. Builds with z88dk's +sms target and
# fujinet-lib-experimental's sms target (the mailbox transport and the claim
# stamp). z88dk writes zcc_opt.def into its working directory, so it runs in
# build/.
#
#   make            build/config.sms (32K, stamped)
#   make install    copy it into the pico bring-up tree, where build-cart.sh
#                   prefers it over the testrom stand-in
#   make cart       install + build the RP2354B firmware with it baked in
#   make clean
#
# Environment overrides:
#   Z88DK      z88dk root                 (default ~/Workspace/z88dk)
#   FNLIB      fujinet-lib-experimental   (default ~/Workspace/fujinet-lib-experimental)
#   PICO_SMS   the cartridge bring-up     (default ~/Workspace/fujinet-firmware/pico/sms)

Z88DK    ?= $(HOME)/Workspace/z88dk
FNLIB    ?= $(HOME)/Workspace/fujinet-lib-experimental
PICO_SMS ?= $(HOME)/Workspace/fujinet-firmware/pico/sms

export ZCCCFG = $(Z88DK)/lib/config
export PATH  := $(Z88DK)/bin:$(PATH)

APP  = config.c st_wifi.c st_hosts.c st_files.c st_copy.c st_info.c st_boot.c \
       st_lobby.c fujiraw.c
PLAT = fujidisp.c fujiin.c fujisnd.c fujiedit.c fujisplash.c fujisplash_data.c
SRCS = $(APP) $(PLAT)

FNLIB_LIB = $(FNLIB)/r2r/sms/fujinet.sms.lib

all: build/config.sms

.DELETE_ON_ERROR:

$(FNLIB_LIB):
	$(MAKE) -C $(FNLIB) sms/r2r

build/config.sms: $(SRCS) $(wildcard *.h) $(FNLIB_LIB)
	mkdir -p build
	cd build && zcc +sms -O2 -I.. -I$(FNLIB)/include -I$(FNLIB)/bus/sms -m \
	    -pragma-define:CRT_ENABLE_STDIO=0 -pragma-define:CLIB_FOPEN_MAX=0 \
	    $(addprefix ../,$(SRCS)) -L$(FNLIB)/r2r/sms -lfujinet.sms.lib \
	    -o config -create-app
	python3 $(FNLIB)/makefiles/sms-romstamp.py --stamp $@

install: build/config.sms
	mkdir -p $(PICO_SMS)/build
	cp build/config.sms $(PICO_SMS)/build/config.sms

cart: install
	cd $(PICO_SMS) && ./build-cart.sh

clean:
	rm -rf build

.PHONY: all install cart clean
