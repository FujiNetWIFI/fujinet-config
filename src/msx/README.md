# MSX ROM FujiNet Bus Code Placement

## Overview

This MSX ROM build uses a custom post-processing step to place FujiNet bus communication code at specific memory locations within page2 (0x8000-0xBFFF) of the ROM. This is necessary because the I/O ports at 0xBFFC-0xBFFF must be accessible when the fujinet bus code is running.

## Architecture

### Memory Layout

```
MSX ROM Memory Map:
0x4000-0x7FFF : Page 1 your application or fujinet-config this page does not need to
                be mapped in during the fujinet call
0x8000-0xBFFF : Page 2 (FujiNet bus code + FujiNet I/O ports + other)

Page 2 Detail:
0x8000-0xB8FF : Application code (from z88dk linker)
0xB900-0xBFF8 : FujiNet bus code (code_fujibus section)
0xBFF9-0xBFFB : Anchor jump (stable entry point)
0xBFFC-0xBFFF : I/O ports (hardware)
```

### Why This Design?

1. **Inter-slot calls**: The anchor at 0xBFF9 provides a stable, fixed entry point for inter-slot calls to `fuji_bus_call`. This address never changes, making it safe for code in other ROM slots to call.

2. **Proximity to I/O**: The FujiNet bus code is placed near the end of page2, close to the I/O ports at 0xBFFC-0xBFFF. When page2 is selected, all necessary resources (code and I/O) are available.

3. **Page isolation**: By keeping all FujiNet bus communication code on page2, we ensure it's always accessible when needed without page switching complications.  page 1 is not required during the call.

## Build Process

### 1. Source Files

**`src/msx/fujibus.s`**
Defines the memory layout for FujiNet bus code:
```assembly
SECTION code_fujibus
ORG 0xB900              ; Start of FujiNet bus code

EXTERN _fuji_bus_call
SECTION fujibus_anchor
ORG 0xBFF9              ; Fixed entry point (3 bytes before I/O)
    jmp _fuji_bus_call  ; Jump to actual implementation
```

This file is the **single source of truth** for memory addresses. Changing the ORG directives here will automatically update the entire build process.

### 2. Linker Output

The z88dk linker produces separate binary files:
- `config.rom` - Main ROM image (without FujiNet bus code)
- `config_code_fujibus.bin` - FujiNet bus code section
- `config_fujibus_anchor.bin` - 3-byte anchor (JMP instruction)

### 3. Post-Processing

**`src/msx/tools/patch_rom.py`**
This Python script:
1. Parses `fujibus.s` to extract ORG addresses
2. Validates that `code_fujibus` fits in available space
3. Patches the ROM by inserting:
   - `code_fujibus` at the address from first ORG directive
   - `fujibus_anchor` at the address from second ORG directive

The script is invoked automatically by the `msxrom/r2r-post` Makefile target.

## Modifying Memory Layout

To change where FujiNet bus code is placed:

1. Edit `src/msx/fujibus.s` and change the ORG directives
2. When the fujibus code gets too big the build will fail.  When that happens 
move code_fujibus ORG lower.
2. Rebuild - the patch script will automatically use the new addresses
