#!/usr/bin/env bash
# mksd.sh -- the rig's SD card: DIR/SD, for test/rig.sh's private fujinet-pc.
#
#   test/mksd.sh DIR
#
# Games where this machine has them (a missing one is skipped): K.C.
# Munchkin from ~/tnfs, the FujiNet O2 apps from their repos' builds (FUJI-
# claimed APPBANK images), GAME8K (a non-claiming 8K test image, default
# the firmware rig's). Then the browser's cases: a .cfg sibling, deep and long
# names, 70 files for paging, an image too big, a file that is not a ROM.
set -euo pipefail
D=$1/SD
rm -rf "$D"
mkdir -p "$D/FujiNet" "$D/games" "$D/apps" "$D/nested/one/two" "$D/long" "$D/many" "$D/Mixed Case"
put() { [ -f "$1" ] && cp "$1" "$2" || echo "mksd: no $1 (skipped)" >&2; }
put "$HOME/tnfs/K.C. Munchkin (USA).bin" "$D/games/"
put "${GAME8K:-/tmp/claude-1000/-home-thomc-Workspace-fujinet-config/e83c92df-41fa-4041-9c4b-91273636565c/scratchpad/fw/fnpc/SD/game8k.bin}" "$D/games/game8k.bin"
put "$HOME/Workspace/fujinet-5cardstud/o2/build/5card.bin" "$D/apps/5card.bin"
put "$HOME/Workspace/fujinet-battleship/o2/build/battleship.bin" "$D/apps/battleship.bin"
put "$HOME/Workspace/netcat/o2/build/nc.bin" "$D/apps/netcat.bin"
if [ -f "$D/games/game8k.bin" ]; then
    cp "$D/games/game8k.bin" "$D/games/x.bin"
    printf 'cfg for x\n' > "$D/games/x.cfg"
    cp "$D/games/game8k.bin" "$D/nested/one/two/deep.bin"
    cp "$D/games/game8k.bin" "$D/long/a very long file name to scroll on the odyssey two.bin"
    cp "$D/games/game8k.bin" "$D/Mixed Case/Tnfs_Game-v2.bin"
fi
for i in $(seq -w 0 69); do printf 'f%s' "$i" > "$D/many/file$i.bin"; done
head -c 70000 /dev/zero > "$D/games/big.bin"
printf 'not a cartridge\n' > "$D/readme.txt"
