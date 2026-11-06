# CONFIG for the Fairchild Channel F.
#
# Standalone, like ../intv/ and ../coleco/: the shared src/ core assumes a
# keyboard, four device slots and kilobytes of buffers.
ASL ?= $(HOME)/asl/asl
P2BIN ?= $(HOME)/asl/p2bin

all: build/config.bin

build/config.bin: $(wildcard src/*.asm src/*.inc) tools/mkfont.py
	ASL=$(ASL) P2BIN=$(P2BIN) ./build.sh

clean:
	rm -rf build

.PHONY: all clean
