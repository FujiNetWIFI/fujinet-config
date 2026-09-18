#!/bin/sh
# run.sh -- build (if needed) and launch FujiNet CONFIG in the FujiNet-
# patched jzIntv, connected to a real fujinet-firmware instance over BoIP.
#
# Override any of these on the command line, e.g.:
#   JZINTV=/path/to/jzintv FUJINET_TARGET=localhost:9995 ./run.sh
#   ./run.sh --fujinet-debug        # extra flags are passed straight to jzintv
#   ECS=1 ./run.sh                  # attach a Mattel ECS (keyboard support)
#
# With ECS=1, press F7 inside jzIntv to switch it to keymap 2, its ECS
# keyboard map -- until you do, host keypresses still go to the hand
# controller and nothing will type.

set -e

cd "$(dirname "$0")"
export SDL_AUDIODRIVER=${SDL_AUDIODRIVER:-pulseaudio}
JZINTV_DIR=${JZINTV_DIR:-$HOME/Workspace/jzintv-20200712-src}
JZINTV=${JZINTV:-$JZINTV_DIR/bin/jzintv}
EXEC_BIN=${EXEC_BIN:-$JZINTV_DIR/rom/exec.bin}
GROM_BIN=${GROM_BIN:-$JZINTV_DIR/rom/grom.bin}
FUJINET_TARGET=${FUJINET_TARGET:-localhost:9995}
ECS=${ECS:-0}
ECS_BIN=${ECS_BIN:-$JZINTV_DIR/rom/ecs.bin}

if [ ! -x "$JZINTV" ]; then
    echo "jzIntv not found or not executable at: $JZINTV" >&2
    echo "Set JZINTV_DIR or JZINTV to point at your FujiNet-patched jzIntv build." >&2
    exit 1
fi
if [ ! -f "$EXEC_BIN" ] || [ ! -f "$GROM_BIN" ]; then
    echo "Missing EXEC/GROM BIOS images:" >&2
    echo "  EXEC_BIN=$EXEC_BIN" >&2
    echo "  GROM_BIN=$GROM_BIN" >&2
    exit 1
fi

# jzIntv refuses to start with --ecs=1 and no ECS ROM image, so check here
# where the message can say which knob produced the requirement.
if [ "$ECS" != "0" ]; then
    if [ ! -f "$ECS_BIN" ]; then
        echo "ECS=1 needs an ECS ROM image, not found at: $ECS_BIN" >&2
        echo "Set ECS_BIN to point at one." >&2
        exit 1
    fi
    set -- --ecs=1 --ecsimg="$ECS_BIN" "$@"
fi

# Rebuild only if the ROM is missing or a source file changed since it was
# last built.
if [ ! -f config.rom ] || [ -n "$(find . -maxdepth 1 -name '*.bas' -newer config.rom)" ]; then
    echo "Building config.rom..."
    make
fi

echo "Launching jzIntv against FujiNet at $FUJINET_TARGET ..."
[ "$ECS" != "0" ] && echo "ECS enabled -- press F7 in jzIntv for its ECS keyboard map."
exec "$JZINTV" \
    -z 4 \
    -e "$EXEC_BIN" \
    -g "$GROM_BIN" \
    --fujinet="$FUJINET_TARGET" \
    "$@" \
    config.rom
