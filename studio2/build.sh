#!/usr/bin/env bash
# build.sh -- assemble CONFIG for the RCA Studio II.
#
# Macroassembler AS with CPU 1802, case-sensitive (-U): fn_frames is not
# FN_FRAMES. checkrom.py rejects 3-cycle opcodes and stray RF use in the
# listing, fitcheck.py an RFIT smaller than its routine; mkst2.py turns the
# .p into the ST2 the cart serves. Everything lands in build/.
set -euo pipefail
cd "$(dirname "$0")"

ASL="${ASL:-$HOME/asl/asl}"
[ -x "$ASL" ] || ASL=$(command -v asl) || { echo "build.sh: no asl (Macroassembler AS)" >&2; exit 1; }

mkdir -p build
( cd src && "$ASL" -q -U -L -i . -o ../build/config.p -olist ../build/config.lst config.asm )
python3 -B tools/checkrom.py build/config.lst
python3 -B tools/fitcheck.py build/config.lst
python3 -B tools/mkst2.py build/config.p build/config.st2 CONFIG
