#!/usr/bin/env bash
# build.sh -- assemble CONFIG with zmac into build/config.bin: 32768 bytes,
# the code from 8000H, FFH filler, the "FUJI" claim at 7CFCH, and
# tools/checkrom.py's layout check passed.
#
# Test builds (never ship one):
#   BARTEST=1        booting a file runs the progress bar with no mount
#   WIFIFAKE=1       always go through the WiFi setup screens
#   LOBBY_HOST=...   the lobby's host (default ec.tnfs.io)
#   ZMAC=path        the assembler; else zmac on PATH, else
#                    ~/Workspace/zmac-1.3/zmac
set -euo pipefail
cd "$(dirname "$0")"

WINDOW=32768
ROM_TOP=$((0x7800))     # image offset: the mailbox pages start here
CLAIM_OFF=$((0x7CFC))

if [ -z "${ZMAC:-}" ]; then
    ZMAC=$(command -v zmac 2>/dev/null || echo "$HOME/Workspace/zmac-1.3/zmac")
fi
[ -x "$ZMAC" ] || { echo "build.sh: no zmac (set ZMAC=)" >&2; exit 1; }

rm -rf build/stage
mkdir -p build/stage
cp config.asm ./*.inc build/stage/
python3 tools/mkfont.py font.txt build/stage/font.inc
{
    echo "BARTEST         EQU     ${BARTEST:-0}"
    echo "WIFIFAKE        EQU     ${WIFIFAKE:-0}"
} > build/stage/flags.inc
printf 'str_lobby_host:         DB      "%s",0\n' "${LOBBY_HOST:-ec.tnfs.io}" \
    > build/stage/lobby.inc

# zmac resolves INCLUDE against the working directory.
(cd build/stage && "$ZMAC" -o ../config.raw -x ../config.lst config.asm)

# zmac's raw output starts at the lowest address assembled, so anything
# emitted below 8000H would shift every address in the image.
if [ "$(head -c 2 build/config.raw | xxd -p)" != "55aa" ]; then
    echo "build.sh: the image does not start with the cartridge header" >&2
    exit 1
fi
size=$(wc -c < build/config.raw)
if [ "$size" -gt "$ROM_TOP" ]; then
    echo "build.sh: $size bytes of code, over the $ROM_TOP-byte ROM top" >&2
    exit 1
fi

{
    cat build/config.raw
    head -c $((WINDOW - size)) /dev/zero | tr '\0' '\377'
} > build/config.bin
printf 'FUJI' | dd of=build/config.bin bs=1 seek=$CLAIM_OFF conv=notrunc status=none
rm -f build/config.raw
python3 tools/checkrom.py build/config.bin

ram_end=$(grep -o 'ram_end *=[0-9a-f]*' build/config.lst | sed 's/.*=//')
echo "config.bin: $size of $ROM_TOP bytes of ROM; variables 7000H-${ram_end}H," \
     "$((0x7400 - 0x$ram_end)) bytes for the stack"
