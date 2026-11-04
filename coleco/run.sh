#!/usr/bin/env bash
# run.sh -- build CONFIG and run it in MAME, windowed, against a fujinet-pc.
#
#   FUJINET_TCP=host:port   the fujinet-pc's BoIP listener (default
#                           127.0.0.1:9995)
#   MAME_DIR=path           a MAME with the fujinet cartridge device applied
#                           (fujinet-firmware/pico/coleco/emu/apply.sh)
#
# The controller is MAME's: the keypad is the PC numpad (* is numpad +,
# # numpad -), the stick the arrow keys, the two buttons as MAME maps them.
set -euo pipefail
cd "$(dirname "$0")"

MAME_DIR=${MAME_DIR:-$HOME/Workspace/mame}
make
export FUJINET_TCP=${FUJINET_TCP:-127.0.0.1:9995}

# MAME runs from its own tree.
cd "$MAME_DIR"
exec ./mame coleco -cartslot fujinet -cart "$OLDPWD/build/config.bin" \
    -window -nomax "$@"
