#!/usr/bin/env python3
"""Fail the build if statics have crept into the stack.

The ColecoVision gives this program $702C-$73B8 for BSS+DATA+stack -- 908
bytes, of which OS7's crt0 tables take the first 204. The ROM side has
checkrom's fences; this is the RAM side's equivalent: read the linker map and
refuse the build if the end of BSS+DATA rises above the line that guarantees
at least ~216 bytes of stack. If this fires, look for a new buffer that
should be reading the reply window instead (see constants.h's RAM RULES).
"""

import re
import sys

LIMIT = 0x72E0

def main():
    if len(sys.argv) != 2:
        sys.exit(f"usage: {sys.argv[0]} <mapfile>")

    end = None
    pat = re.compile(r"^(\w+)\s*=\s*\$([0-9A-Fa-f]+)")
    with open(sys.argv[1]) as f:
        for line in f:
            m = pat.match(line)
            if not m:
                continue
            name, val = m.group(1), int(m.group(2), 16)
            if name in ("__BSS_END_tail", "__DATA_END_tail", "__BSS_tail",
                        "__DATA_tail"):
                if end is None or val > end:
                    end = val

    if end is None:
        sys.exit("bsscheck: no BSS/DATA end symbol in the map -- "
                 "did the map format change?")
    stack = 0x73B8 - end
    print(f"bsscheck: statics end at ${end:04X}, {stack} bytes of stack")
    if end > LIMIT:
        sys.exit(f"bsscheck: FAIL -- statics end above ${LIMIT:04X}. "
                 "A buffer needs to become a reply-window read.")

if __name__ == "__main__":
    main()
