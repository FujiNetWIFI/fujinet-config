#!/usr/bin/env bash
# build.sh -- assemble CONFIG for the Atari 2600.
#
# Standalone, like ../intv/, ../coleco/ and ../channelf/: the shared src/ core
# assumes a keyboard, four device slots and kilobytes of buffers, and this
# console has a joystick, one device slot and 128 bytes of RAM.
#
# Macroassembler AS, the same assembler the Channel F (F8), Arcadia (2650) and
# O2 (8048) ports use. Not dasm: it is not installed and cannot be fetched in
# CI, and src/vcs.inc is written from the hardware register map rather than
# copied from the dasm world's vcs.h, which is not redistributable.
#
# THE IMAGE IS N BANKS OF 2K THEN THE FIXED HALF, in that order, because that
# is what vcs_set_image() expects. (N+1)*2048 must land on MAME's
# vcs_cart_slot_device::call_load() whitelist, so N is one of 1, 3, 7 or 15.
set -euo pipefail
cd "$(dirname "$0")"

ASL="${ASL:-}"
P2BIN="${P2BIN:-}"
if [ -z "$ASL" ]; then
    if command -v asl >/dev/null 2>&1; then
        ASL=asl P2BIN=p2bin
    elif [ -x "$HOME/asl/asl" ]; then
        ASL="$HOME/asl/asl" P2BIN="$HOME/asl/p2bin"
    else
        echo "build.sh: no Macroassembler AS found (tried PATH and ~/asl)" >&2
        exit 1
    fi
fi

# The firmware tree owns two things this port must not fork: the mailbox
# header that src/fujinet.inc mirrors, and the MAME cartridge device.
FN2600="${FN2600:-$HOME/Workspace/fujinet-firmware/pico/atari-2600}"

die() { echo "build.sh: $*" >&2; exit 1; }

mkdir -p build

# The equates are hand-mirrored from the firmware header. Check them BEFORE
# assembling anything: a drifted address assembles cleanly, runs, and silently
# talks to a page that decodes nothing.
if [ -f "$FN2600/firmware/include/fuji_mailbox.h" ]; then
    python3 tools/checkdefs.py "$FN2600/firmware/include/fuji_mailbox.h" src/fujinet.inc
else
    echo "build.sh: no fuji_mailbox.h at $FN2600 -- skipping the equate check" >&2
fi

BANKS=(hosts browse info edit wifi boot copy)
TAIL=config

# p2bin prints "(N Bytes)" -- the span it actually transferred out of the .p,
# which is exactly "how much of this range the assembler emitted". Reading it
# beats parsing the AS listing: a listing line like "D0 FB   bcc LABEL" has a
# mnemonic made entirely of hex digits, and a regex that counts byte columns
# counts that too.
emitted() {
    "$P2BIN" "$1" "$2" -r "$3" -l 0 2>&1 \
        | sed -n 's/.*(\([0-9][0-9]*\) Bytes).*/\1/p' | head -1
}

asm() {
    ( cd src && "$ASL" -q -L "$1.asm" )
    mv "src/$1.p" "build/$1.p"
    mv -f "src/$1.lst" "build/$1.lst" 2>/dev/null || true
}

# A bank that grows past $17FF is SILENTLY TRUNCATED by p2bin -r, and the
# client then jumps into whatever the next bank has at that offset. Nothing in
# the toolchain says a word about it, so ask directly how many bytes landed in
# the half that belongs to the mailbox: the answer must be none.
assert_fits() {
    local spill
    spill=$(emitted "build/$1.p" "build/$1.spill" '$1800-$1FFF')
    rm -f "build/$1.spill"
    [ "${spill:-0}" -eq 0 ] \
        || die "bank '$1' puts $spill byte(s) past \$17FF -- p2bin would truncate it"
}

parts=()
for b in "${BANKS[@]}"; do
    asm "$b"
    used=$(emitted "build/$b.p" "build/$b.bin" '$1000-$17FF')
    assert_fits "$b"
    printf 'bank %-8s %4d/2048 used  %4d free\n' "$b" "$used" $((2048 - used))
    parts+=("build/$b.bin")
    rm -f "build/$b.p"
done

# Pad up to the next whitelisted bank count with all-$FF filler, so adding a
# screen never changes the image arithmetic.
n=${#parts[@]}
for want in 1 3 7 15; do
    if [ "$n" -le "$want" ]; then target=$want; break; fi
done
[ -n "${target:-}" ] || die "too many banks: $n (the largest whitelisted image is 15 + tail)"
if [ "$n" -lt "$target" ]; then
    head -c 2048 /dev/zero | tr '\0' '\377' > build/filler.bin
    while [ "$n" -lt "$target" ]; do parts+=(build/filler.bin); n=$((n + 1)); done
    echo "build.sh: padded $((target - ${#BANKS[@]})) filler bank(s) to reach $target"
fi

# The fixed half. Its object goes to a distinct name because the tail source
# is config.asm and the finished image is config.bin.
asm "$TAIL"
emitted "build/$TAIL.p" build/fixedhalf.bin '$1800-$1FFF' >/dev/null
rm -f "build/$TAIL.p"

cat "${parts[@]}" build/fixedhalf.bin > build/config.bin
rm -f build/filler.bin build/fixedhalf.bin
for b in "${BANKS[@]}"; do rm -f "build/$b.bin"; done

# The claim. An image carrying "FUJI" at FN_R_CLAIM promises the mailbox pages
# are not its own code, so the cartridge keeps decoding after it boots. The
# fixed half is the LAST 2K whatever the image size, so the offset is a
# function of that size and cannot live in the source.
size=$(stat -c%s build/config.bin)
printf 'FUJI' | dd of=build/config.bin bs=1 seek=$(( size - 0x800 + 0x0710 )) \
    conv=notrunc status=none

python3 tools/checkrom.py build/config.bin
echo "build/config.bin: $size bytes"
