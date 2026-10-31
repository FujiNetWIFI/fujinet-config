#!/usr/bin/env bash
# run.sh -- the CONFIG in MAME's Odyssey 2 with the FujiNet cart, in a
# window, against a live fujinet-pc.
#
#   ./run.sh                          build/config.bin (built if missing)
#   ./run.sh path/to/image.bin        another image (more MAME options after it)
#   MACHINE=videopac ./run.sh         the PAL console
#
# Needs ~/Workspace/mame (MAME_DIR) with the O2 FujiNet cart device grafted in
# (fujinet-firmware pico/o2/emu/apply-mame.sh) and a fujinet-pc BoIP listener
# on FUJINET_TCP (default 127.0.0.1:9995). MAME runs the image as the cart's
# baked CONFIG. FUJINET_PACE_MS=40 slows a push down enough to watch the bar.
#
# MAME's keys: the arrow keys are the stick, Left Ctrl its button; letters
# and digits are the console's keyboard, Enter is ENT and Backspace CLR, the
# keypad's * and / are x and divide; F1 the console's RESET.
set -euo pipefail
cd "$(dirname "$0")"
HERE="$PWD"

MAME_DIR="${MAME_DIR:-$HOME/Workspace/mame}"
export FUJINET_TCP="${FUJINET_TCP:-127.0.0.1:9995}"
IMAGE="${1:-$HERE/build/config.bin}"
[ "${IMAGE#/}" = "$IMAGE" ] && IMAGE="$HERE/$IMAGE"

[ -f build/config.bin ] || ./build.sh
[ -f "$IMAGE" ] || { echo "run.sh: no $IMAGE" >&2; exit 1; }
[ -x "$MAME_DIR/mame" ] || { echo "run.sh: no MAME at $MAME_DIR" >&2; exit 1; }

mkdir -p "$HERE/build/snap" "$HERE/build/cfg"
cd "$MAME_DIR"
exec ./mame "${MACHINE:-odyssey2}" -cartslot fujinet -cart "$IMAGE" \
    -snapshot_directory "$HERE/build/snap" -cfg_directory "$HERE/build/cfg" \
    -window -nomax "${@:2}"
