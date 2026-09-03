#!/usr/bin/env bash
# run.sh -- build (if needed) and launch CONFIG in MAME against a live
# fujinet-pc, mirroring the bring-up tree's run.sh.
#
#   FUJINET_TCP=host:port   fujinet-pc BoIP listener (default 127.0.0.1:9995)
#   FUJINET_DEBUG=1         per-transaction stderr log (default on here)
#   FUJINET_BOOTDUMP=prefix write pushed streams to prefix.rom/.cfg
#   MAME_DIR=path           MAME tree with the fujinet device applied
#                           (see $FUJI_PICO/emu/apply.sh)
#
# Headless smoke run: add
#   -autoboot_script $FUJI_PICO/emu/drive.lua
# which presses keypad 1 at t=3 (CONFIG is first on the SELECT GAME menu).

set -euo pipefail
cd "$(dirname "$0")"

MAME_DIR=${MAME_DIR:-$HOME/Workspace/mame}
FUJI_PICO=${FUJI_PICO:-$HOME/Workspace/fujinet-firmware/pico/astrocade}

make

export FUJINET_TCP=${FUJINET_TCP:-127.0.0.1:9995}
export FUJINET_DEBUG=${FUJINET_DEBUG:-1}

exec "$MAME_DIR/mame" astrocde \
    -rompath "$MAME_DIR/roms" \
    -cartslot fujinet -cart "$PWD/build/config.bin" \
    -window -nomax \
    "$@"
