#!/usr/bin/env bash
# run.sh -- build (if needed) and launch CONFIG in MAME against a live
# fujinet-pc, mirroring the bring-up tree's run.sh.
#
#   FUJINET_TCP=host:port   fujinet-pc BoIP listener (default 127.0.0.1:9995)
#   FUJINET_DEBUG=1         per-transaction stderr log (default on here)
#   FUJINET_BOOTDUMP=prefix write pushed streams to prefix.rom/.cfg
#   MAME_DIR=path           MAME tree with the fujinet cart device applied
#                           (fujinet-firmware/pico/arcadia/emu/apply.sh)
#
# The keypad is the PC numpad: Clear = numpad -, Enter = numpad +, and
# keypad 2 is also the FIRE button. The disc is the arrow keys.

set -euo pipefail
cd "$(dirname "$0")"

MAME_DIR=${MAME_DIR:-$HOME/Workspace/mame}

make

export FUJINET_TCP=${FUJINET_TCP:-127.0.0.1:9995}
export FUJINET_DEBUG=${FUJINET_DEBUG:-1}

exec "$MAME_DIR/mame" arcadia \
    -cartslot fujinet -cart "$PWD/build/config.bin" \
    -window -nomax \
    "$@"
