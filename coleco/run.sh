#!/usr/bin/env bash
# run.sh -- build, install into the pico bring-up tree, and run in MAME
# through that tree's own run.sh (which needs a MAME with emu/apply.sh run
# against it for -cartslot fujinet).
#
#   ./run.sh                          interactive
#   ./run.sh --headless "SELECT A HOST"   headless, PASS/FAIL on a string
#   ./run.sh --drive                  headless, drive the controller
#
# Env: PICO_COLECO (default ~/Workspace/fujinet-firmware/pico/coleco), plus
# everything the bring-up's run.sh honors: MAME_DIR, SCREEN_EXPECT, SCREEN_AT,
# SCREEN_RESET_AT, DRIVE_SCRIPT, DRIVE_HOST, DRIVE_DOWN, FUJINET_DEBUG...
set -euo pipefail
cd "$(dirname "$0")"

PICO_COLECO=${PICO_COLECO:-$HOME/Workspace/fujinet-firmware/pico/coleco}

make
cp build/config.bin "$PICO_COLECO/build/config.bin"
exec "$PICO_COLECO/run.sh" config "$@"
