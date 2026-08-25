INTYBASIC ?= intybasic
AS1600    ?= as1600
LIBDIR    ?= $(HOME)/Workspace/IntyBASIC/intybasic/

PRODUCT = config
SOURCES = config.bas constants.bas fujinet.bas fujicmd.bas screen.bas \
          scroll.bas input.bas st_wifi.bas st_hosts.bas st_file.bas \
          st_info.bas st_boot.bas st_lobby.bas st_copy.bas

.PHONY: all clean run rom.h
all: $(PRODUCT).bin $(PRODUCT).rom

$(PRODUCT).asm: $(SOURCES)
	$(INTYBASIC) config.bas $(PRODUCT).asm $(LIBDIR)

$(PRODUCT).bin: $(PRODUCT).asm       # .bin + .cfg, for PiRTO II / SD-flashed boot
	$(AS1600) -o $(PRODUCT).bin -l $(PRODUCT).lst $(PRODUCT).asm

$(PRODUCT).rom: $(PRODUCT).asm       # Intellicart format, for jzIntv
	$(AS1600) -o $(PRODUCT).rom $(PRODUCT).asm

run: $(PRODUCT).rom
	./run.sh

# Packs config.bin into the _bootrom[] C array format
# fujinet-firmware/pico/intellivision/firmware/rom.h expects. Copy the
# result over that file to bake this build in as the flash boot ROM --
# see that repo's README for the exact path and rebuild steps.
rom.h: $(PRODUCT).bin
	python3 mkromh.py $(PRODUCT).bin > config_rom.h
	@echo "Wrote config_rom.h -- copy over fujinet-firmware's pico/intellivision/firmware/rom.h to bake this in."

clean:
	rm -f $(PRODUCT).asm $(PRODUCT).bin $(PRODUCT).cfg $(PRODUCT).rom $(PRODUCT).lst config_rom.h
