#!/usr/bin/env python3
"""
Patch MSX ROM with FujiNet bus code.

This script patches the config.rom file with:
1. code_fujibus section at the address specified in fujibus.s
2. fujibus_anchor at the address specified in fujibus.s

The addresses are read from fujibus.s by parsing ORG directives.
"""

import sys
import os
import re

def parse_fujibus_addresses(fujibus_s_path):
    """Parse ORG directives from fujibus.s to get memory addresses."""
    
    if not os.path.exists(fujibus_s_path):
        print(f"Error: fujibus.s not found: {fujibus_s_path}", file=sys.stderr)
        return None, None
    
    fujibus_offset = None
    anchor_offset = None
    
    try:
        with open(fujibus_s_path, 'r') as f:
            in_code_fujibus = False
            in_fujibus_anchor = False
            
            for line in f:
                line = line.strip()
                
                # Track which section we're in
                if 'SECTION code_fujibus' in line:
                    in_code_fujibus = True
                    in_fujibus_anchor = False
                elif 'SECTION fujibus_anchor' in line:
                    in_code_fujibus = False
                    in_fujibus_anchor = True
                elif line.startswith('SECTION'):
                    in_code_fujibus = False
                    in_fujibus_anchor = False
                
                # Parse ORG directives
                if line.startswith('ORG'):
                    match = re.match(r'ORG\s+(0x[0-9A-Fa-f]+|[0-9]+)', line)
                    if match:
                        addr = int(match.group(1), 0)
                        if in_code_fujibus and fujibus_offset is None:
                            fujibus_offset = addr
                        elif in_fujibus_anchor and anchor_offset is None:
                            anchor_offset = addr
    
    except Exception as e:
        print(f"Error parsing fujibus.s: {e}", file=sys.stderr)
        return None, None
    
    if fujibus_offset is None:
        print("Error: Could not find ORG directive in code_fujibus section", file=sys.stderr)
    if anchor_offset is None:
        print("Error: Could not find ORG directive in fujibus_anchor section", file=sys.stderr)
    
    return fujibus_offset, anchor_offset

def patch_rom(rom_path, fujibus_bin_path, anchor_bin_path, fujibus_offset, anchor_offset):
    """Patch the ROM file with fujibus code and anchor."""
    
    # Constants
    FUJIBUS_OFFSET = fujibus_offset
    FUJIBUS_MAX_SIZE = 0xC000 - fujibus_offset - 7 # 4 bytes for IO and 3 bytes for jmp
    ANCHOR_OFFSET = anchor_offset  # 3 bytes before the IO
    ROM_START = 0x4000
    ANCHOR_SIZE = 3
    
    # Check if files exist
    if not os.path.exists(rom_path):
        print(f"Error: ROM file not found: {rom_path}", file=sys.stderr)
        return 1
    
    if not os.path.exists(fujibus_bin_path):
        print(f"Error: FujiNet bus binary not found: {fujibus_bin_path}", file=sys.stderr)
        return 1
    
    if not os.path.exists(anchor_bin_path):
        print(f"Error: Anchor binary not found: {anchor_bin_path}", file=sys.stderr)
        return 1
    
    # Read the ROM file
    try:
        with open(rom_path, 'rb') as f:
            rom = bytearray(f.read())
    except Exception as e:
        print(f"Error reading ROM file: {e}", file=sys.stderr)
        return 1
    
    # Read the fujibus binary
    try:
        with open(fujibus_bin_path, 'rb') as f:
            fujibus = f.read()
    except Exception as e:
        print(f"Error reading FujiNet bus binary: {e}", file=sys.stderr)
        return 1
    
    # Read the anchor binary
    try:
        with open(anchor_bin_path, 'rb') as f:
            anchor = f.read()
    except Exception as e:
        print(f"Error reading anchor binary: {e}", file=sys.stderr)
        return 1
    
    # Validate fujibus size
    fujibus_size = len(fujibus)
    if fujibus_size > FUJIBUS_MAX_SIZE:
        print(f"Error: FujiNet bus code is too large!", file=sys.stderr)
        print(f"  Size: {fujibus_size} bytes", file=sys.stderr)
        print(f"  Maximum: {FUJIBUS_MAX_SIZE} bytes", file=sys.stderr)
        print(f"  Overflow: {fujibus_size - FUJIBUS_MAX_SIZE} bytes", file=sys.stderr)
        return 1
    
    # Validate anchor size
    if len(anchor) != ANCHOR_SIZE:
        print(f"Error: Anchor must be exactly {ANCHOR_SIZE} bytes, got {len(anchor)}", file=sys.stderr)
        return 1
    
    # Ensure ROM is large enough
    min_rom_size = max(FUJIBUS_OFFSET + fujibus_size - ROM_START, ANCHOR_OFFSET + ANCHOR_SIZE - ROM_START)
    if len(rom) < min_rom_size:
        print(f"Error: ROM file is too small ({len(rom)} bytes, need at least {min_rom_size})", file=sys.stderr)
        return 1
    
    # Patch the ROM
    print(f"Patching ROM: {rom_path}")
    print(f"  FujiNet bus code: {fujibus_size} bytes at offset 0x{FUJIBUS_OFFSET:04X}")
    print(f"  Anchor: {len(anchor)} bytes at offset 0x{ANCHOR_OFFSET:04X}")
    
    rom[(FUJIBUS_OFFSET - ROM_START):(FUJIBUS_OFFSET - ROM_START) + fujibus_size] = fujibus
    rom[(ANCHOR_OFFSET - ROM_START):(ANCHOR_OFFSET - ROM_START) + ANCHOR_SIZE] = anchor
    
    # Write the patched ROM
    try:
        with open(rom_path, 'wb') as f:
            f.write(rom)
    except Exception as e:
        print(f"Error writing patched ROM: {e}", file=sys.stderr)
        return 1
    
    print("ROM patched successfully!")
    return 0

def main():
    if len(sys.argv) != 5:
        print("Usage: patch_rom.py <rom_file> <fujibus_bin> <anchor_bin> <fujibus_s>", file=sys.stderr)
        print("", file=sys.stderr)
        print("Example:", file=sys.stderr)
        print("  patch_rom.py r2r/msxrom/config.rom r2r/msxrom/config_code_fujibus.bin r2r/msxrom/config_fujibus_anchor.bin src/msx/fujibus.s", file=sys.stderr)
        return 1
    
    rom_path = sys.argv[1]
    fujibus_bin_path = sys.argv[2]
    anchor_bin_path = sys.argv[3]
    fujibus_s_path = sys.argv[4]
    
    # Parse addresses from fujibus.s
    print(f"Reading addresses from: {fujibus_s_path}")
    fujibus_offset, anchor_offset = parse_fujibus_addresses(fujibus_s_path)
    
    if fujibus_offset is None or anchor_offset is None:
        return 1
    
    print(f"Found addresses: fujibus=0x{fujibus_offset:04X}, anchor=0x{anchor_offset:04X}")
    
    return patch_rom(rom_path, fujibus_bin_path, anchor_bin_path, fujibus_offset, anchor_offset)

if __name__ == '__main__':
    sys.exit(main())
