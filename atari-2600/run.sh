#!/usr/bin/env bash
# run.sh -- run CONFIG in MAME.
#
#   ./run.sh [lua-script]
#
# With a script it runs headless and exits; without one it opens a window.
#
# The cartridge device lives in the firmware tree (see $FN2600/emu/apply.sh,
# which grafts it into MAME). This port's own harnesses live in emu/ here.
#
# Two environment facts this wraps, both of which cost time to rediscover:
#   - MAME must run FROM ITS OWN TREE or -autoboot_script is silently ignored.
#   - SDL_VIDEODRIVER=dummy is required wherever there is no DISPLAY; without
#     it MAME dies with "Could not initialize SDL No available video device"
#     even under -video none, because SDL is brought up before the video
#     backend is chosen.
#
# fujinet-pc's BoIP listener takes ONE client (backlog 1), so a MAME left
# running silently starves the next run and the symptom is a hang, not an
# error. Kill any stray first.

set -euo pipefail
cd "$(dirname "$0")"
HERE=$(pwd)

SCRIPT=${1:-}
MAME=${MAME:-$HOME/Workspace/mame}
SLOT=${SLOT:-fujinet}
SNAP=${SNAP:-$HERE/build/snap}
FN2600=${FN2600:-$HOME/Workspace/fujinet-firmware/pico/atari-2600}

[ -f "$HERE/build/config.bin" ] || { echo "run.sh: no build/config.bin -- run make" >&2; exit 1; }

pkill -f "mame a2600" 2>/dev/null || true
mkdir -p "$SNAP"

args=(a2600 -cartslot "$SLOT" -cart "$HERE/build/config.bin"
      -snapshot_directory "$SNAP")

# Lua harnesses write their artefacts next to the build, not into the MAME
# tree we have to cd into.
export DRIVE_EXPECT="${DRIVE_EXPECT:-$HERE/build/expect.txt}"
# Harnesses require() shared modules from this port's emu/ first, then the
# firmware tree's, which is where vcsfont.lua and the older drivers live.
export A2600_EMU="$HERE/emu"
export A2600_FWEMU="$FN2600/emu"

if [ -n "$SCRIPT" ]; then
    if [ -f "$HERE/emu/$SCRIPT.lua" ]; then
        LUA="$HERE/emu/$SCRIPT.lua"
    elif [ -f "$FN2600/emu/$SCRIPT.lua" ]; then
        LUA="$FN2600/emu/$SCRIPT.lua"
    else
        echo "run.sh: no harness '$SCRIPT.lua' in emu/ or $FN2600/emu/" >&2
        exit 1
    fi
    args+=(-autoboot_script "$LUA"
           -video none -sound none -nothrottle
           -seconds_to_run "${SECS:-10}")
fi

[ -n "${DISPLAY:-}" ] || export SDL_VIDEODRIVER=dummy

cd "$MAME"
exec ./mame "${args[@]}"
