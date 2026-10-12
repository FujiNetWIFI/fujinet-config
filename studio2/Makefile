# CONFIG for the RCA Studio II.
#
# Standalone, like ../channelf/: the shared src/ core assumes a keyboard,
# four device slots and kilobytes of buffers; this console has 512 bytes.
ASL ?= $(HOME)/asl/asl
FW_STUDIO2 ?= $(HOME)/Workspace/fujinet-firmware/pico/studio2

all: build/config.st2

build/config.st2: $(wildcard src/*.asm src/*.inc) $(wildcard tools/*.py) build.sh
	ASL=$(ASL) ./build.sh

# The client the cart (and run.sh's MAME) serves at power-on.
install: build/config.st2
	mkdir -p $(FW_STUDIO2)/build
	cp build/config.st2 $(FW_STUDIO2)/build/config.st2

clean:
	rm -rf build

.PHONY: all install clean
