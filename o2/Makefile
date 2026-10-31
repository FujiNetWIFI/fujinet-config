# Makefile -- the FujiNet CONFIG for the Odyssey 2 / Videopac: build/config.bin
# (build.sh). Standalone, like the other consoles' CONFIGs: not part of the
# root build. The firmware's pico/o2/build-cart.sh bakes build/config.bin in.
#
#   make                  the image
#   make run              in a MAME window (run.sh), against FUJINET_TCP
#   make test             the scenarios (test/t) on a private rig, checked
#                         against the screen's text (tools/cfgtest.py)
#   make copytest         the copy: needs a tnfsd on localhost:16384 and a
#                         fujinet-pc with the COPY_FILE fix (FNPC_BIN)
#   make progress         a paced boot, throttled: the bar's snapshots
#   make rows             frame stability, NTSC and PAL (tools/rowcheck.py)
#   make spike shot       the engine's test pattern, and its slack

FNPC_BIN ?= $(HOME)/Workspace/fn-o2-config/build-pc/fujinet
CHECK = O2SIM_FAST=1 python3 tools/cfgtest.py

all:
	./build.sh

run: all
	./run.sh

test: all
	WIFIFAKE=1 OUT=build/wf ./build.sh
	LOBBY_HOST=SD LOBBY_PATH=/apps/5card.bin OUT=build/lob ./build.sh
	$(CHECK) test/t/hosts.scen build/run/t_hosts 60
	$(CHECK) test/t/browse.scen build/run/t_browse 90
	$(CHECK) test/t/boot.scen build/run/t_boot 60
	$(CHECK) test/t/edit.scen build/run/t_edit 60
	$(CHECK) test/t/wifi.scen build/run/t_wifi 60 build/wf/config.bin
	$(CHECK) test/t/misc.scen build/run/t_misc 60 build/lob/config.bin

copytest: all
	HOSTS=SD,localhost FNPC_BIN=$(FNPC_BIN) $(CHECK) test/t/copy.scen build/run/t_copy 60

progress: all
	FUJINET_PACE_MS=40 test/rig.sh test/scen/boot8k.scen build/run/progress 60

rows: all
	O2SIM_FAST=1 test/rig.sh test/scen/rows.scen build/run/rows 60
	python3 tools/rowcheck.py build/run/rows hosts
	python3 tools/rowcheck.py build/run/rows files
	O2SIM_MACHINE=videopac O2SIM_FAST=1 test/rig.sh test/scen/rows.scen build/run/rows_pal 60
	python3 tools/rowcheck.py build/run/rows_pal hosts
	python3 tools/rowcheck.py build/run/rows_pal files

spike:
	SPIKE=1 OUT=build/spike ./build.sh

shot: spike
	O2SIM_SECONDS=20 python3 tools/o2sim.py test/scen/spike.scen build/spike/run build/spike/config.bin
	python3 tools/rowcheck.py build/spike/run rows

clean:
	rm -rf build

.PHONY: all run test copytest progress rows spike shot clean
