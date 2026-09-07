' constants.bas -- screen/color/input constants and the shared scratch-RAM
' map. Values for BACKTAB/CS_*/DISC_*/BUTTON_*/KEYPAD_* taken from the
' standard IntyBASIC constants.bas (Mark Ball) as already used by
' fujinet-5cardstud/intv/constants.bas; trimmed to what this program needs.
'
' *** RAM BUDGET RULE ***
' IntyBASIC gives this program 228 bytes of 8-bit scratchpad vars (222 once
' CONT.KEY is used, which costs 6) and 47 16-bit vars, and NO string type at
' all. NEVER buffer tabular/list data (directory pages, host slot names,
' filenames) in DIM'd variables. Directory pages are drawn straight out of
' the mailbox RX window (FN_RX) as each reply lands; anything that must
' survive across multiple mailbox transactions lives in the SC_* scratch
' block below, which is ordinary cart RAM outside the mailbox, addressed
' with PEEK/POKE exactly like the mailbox is.

    CONST BACKTAB_ADDR       = $0200   ' Start of the BACKtab in RAM.
    CONST SCREEN_ROWS        = 12
    CONST SCREEN_COLS        = 20

    DEF FN screenpos(aColumn, aRow)  = (((aRow)*SCREEN_COLS)+(aColumn))
    DEF FN screenaddr(aColumn, aRow) = (BACKTAB_ADDR+(((aRow)*SCREEN_COLS)+(aColumn)))

    ' Color-stack mode colors (foreground; also usable as background via CS_ADVANCE).
    CONST CS_BLACK      = $0000
    CONST CS_BLUE       = $0001
    CONST CS_RED        = $0002
    CONST CS_TAN        = $0003
    CONST CS_DARKGREEN  = $0004
    CONST CS_GREEN      = $0005
    CONST CS_YELLOW     = $0006
    CONST CS_WHITE      = $0007
    ' 8-15 are BACKGROUND ONLY here: a GROM card's foreground is bits 0-2 plus
    ' bit 12, and bit 12 doubles as the Colored Squares selector, so the STIC
    ' requires foreground bit 3 to be 0 for GROM cards. Color stack REGISTERS
    ' take the full 0-15 though, which is how CS_CYAN reaches the screen.
    CONST CS_CYAN       = $0009
    CONST CS_PURPLE     = $000F
    CONST CS_ADVANCE    = $2000   ' Advance the color stack by one position.

    ' MOB (sprite) register bits, from the same standard IntyBASIC constants.bas
    ' this file is otherwise derived from. Only the character grid uses a MOB --
    ' see input.bas's inverse-video cursor.
    '
    ' *** SPR_BEHIND is bit 13 of the ATTRIBUTE register and means PRIORITY. ***
    ' The IntyBASIC manual (manual.txt:944) calls it "Change color stack", which
    ' is the BACKTAB meaning copy-pasted into the SPRITE section by mistake. The
    ' STIC's own documentation is unambiguous -- jzintv/doc/programming/
    ' stic.txt:247, "PRIO: if set, the MOB appears behind background cards".

    ' GRAM cards. The program defines no others, so 0-2 are the whole set.
    ' These live here, not in csbar.bas which supplies their bitmaps, because
    ' input.bas uses GLYPH_BLOCK and is INCLUDEd long before csbar.bas.
    CONST GLYPH_FOLDER  = 0
    CONST GLYPH_CART    = 1
    ' GLYPH_BLOCK is shared: input.bas parks it behind the character grid's
    ' selected cell as an inverse-video MOB, and st_boot.bas draws it straight
    ' into BACKTAB for the boot progress bar.
    CONST GLYPH_BLOCK   = 2
    CONST GRAM_SELECT   = $0800   ' bit 11: take the card from GRAM, not GROM

    CONST SPR_HIT       = $0100   ' X reg: collision detection (unused here)
    CONST SPR_VISIBLE   = $0200   ' X reg
    CONST SPR_ZOOMY2    = $0100   ' Y reg: 1x card-pixel tall (MOBs are half-pixel by default)
    CONST SPR_BEHIND    = $2000   ' A reg bit 13 = PRIO

    ' Foreground/background (FGBG) mode backgrounds, ORed into a BACKTAB word
    ' on top of the usual card*8+foreground. FGBG gives every cell its own
    ' background from all 16 colors, at the price of only GROM cards 0-63
    ' (ASCII 32-95, no lowercase) -- see fgbg.bas. ST_HOSTS and ST_INFO use it.
    '
    ' *** The bit order is NOT what the IntyBASIC manual says. ***
    ' manual.txt:965-967 claims $1000 is background bit 2 and $2000 bit 3;
    ' the hardware has them the other way round. Ground truth is jzIntv's
    ' decoder, src/stic/stic.c:1481:
    '     bg_clr = ((card >> 9) & 0xB) | ((card >> 11) & 0x4)
    ' i.e. background bits 3,2,1,0 come from word bits 12,13,10,9 -- which is
    ' what jzintv/doc/programming/stic.txt:465-470 documents. So:
    '     bg bit 0 = $0200   bg bit 1 = $0400
    '     bg bit 2 = $2000   bg bit 3 = $1000
    ' Following the manual instead would render DARKGREEN (4) as grey (8).
    CONST BG_BLUE       = $0200   '  1
    CONST BG_RED        = $0400   '  2
    CONST BG_TAN        = $0600   '  3
    CONST BG_DARKGREEN  = $2000   '  4
    CONST BG_GREEN      = $2200   '  5
    CONST BG_YELLOW     = $2400   '  6
    CONST BG_PURPLE     = $3600   ' 15

    ' Disc directions (8-way, used for menu/grid navigation).
    CONST DISC_UP     = $0004
    CONST DISC_RIGHT  = $0002
    CONST DISC_DOWN   = $0001
    CONST DISC_LEFT   = $0008

    ' Action buttons.
    CONST BUTTON_1     = $A0   ' Top left or top right.
    CONST BUTTON_2     = $60   ' Bottom left.
    CONST BUTTON_3     = $C0   ' Bottom right.
    CONST BUTTON_MASK  = $E0

    ' Keypad, as DECODED by CONT.KEY (manual: "0-9 for numbers, 10-Clear,
    ' 11-Enter, 12-Not pressed") -- these are the values in_key actually
    ' takes in input.bas, NOT the raw button-matrix bit patterns (those only
    ' matter if you read CONT1/CONT2 directly instead of using .KEY, which
    ' this program never does).
    CONST KEYPAD_0      = 0
    CONST KEYPAD_1      = 1
    CONST KEYPAD_2      = 2
    CONST KEYPAD_3      = 3
    CONST KEYPAD_4      = 4
    CONST KEYPAD_5      = 5
    CONST KEYPAD_6      = 6
    CONST KEYPAD_7      = 7
    CONST KEYPAD_8      = 8
    CONST KEYPAD_9      = 9
    CONST KEYPAD_CLEAR  = 10
    CONST KEYPAD_ENTER  = 11
    CONST KEYPAD_NONE   = 12

    ' -------------------------------------------------------------------
    ' Application-level state machine (mirrors fujinet-config's src/main.c)
    ' -------------------------------------------------------------------
    ' 0-based: `ON state GOSUB label1,...,labelN` in IntyBASIC uses `state`
    ' directly as the jump-table index (MVI@ R1,PC over table+state), NOT
    ' state-1 the way classic BASIC's 1-based ON...GOTO does -- confirmed
    ' by reading the compiled config.asm. These values must stay 0-based
    ' and in the exact same order as the ON GOSUB list in config.bas, or
    ' every state dispatches one slot off from what it names.
    CONST ST_CHECK_WIFI    = 0
    CONST ST_CONNECT_WIFI  = 1
    CONST ST_SET_WIFI      = 2
    CONST ST_HOSTS         = 3
    CONST ST_SELECT_FILE   = 4
    CONST ST_INFO          = 5
    CONST ST_BOOT          = 6

    ' set_wifi substates (mirrors src/typedefs.h WSSubState)
    CONST WS_SCAN     = 0
    CONST WS_SELECT   = 1
    CONST WS_CUSTOM   = 2
    CONST WS_PASSWORD = 3
    CONST WS_DONE     = 4

    ' select_file substates (mirrors src/typedefs.h SFSubState)
    CONST SF_INIT     = 0
    CONST SF_DISPLAY  = 1
    CONST SF_CHOOSE   = 2
    CONST SF_NEXT     = 3
    CONST SF_PREV     = 4
    CONST SF_ADV      = 5   ' advance into a subfolder
    CONST SF_DEV      = 6   ' devance out of a subfolder
    CONST SF_DONE     = 7

    ' hosts substates
    CONST HD_LIST  = 0
    CONST HD_EDIT  = 1
    CONST HD_DONE  = 2

    ' -------------------------------------------------------------------
    ' Directory/paging layout
    ' -------------------------------------------------------------------
    ' -------------------------------------------------------------------
    ' Character-grid text entry layout (input.bas's grid_entry). All 95
    ' printable ASCII characters (32-126) fit in 6 rows x 16 columns, so
    ' there's no SHIFT/SYM paging -- the highlighted cell's position IS the
    ' character: ch = 32 + gy*16 + gx. The action row (g_y=6, a sentinel
    ' value outside the charset) has 4 buttons, evenly spaced to exactly
    ' fill the 20-column screen: SPC / DEL / OK / ESC, 3 columns apart plus
    ' a gap.
    ' -------------------------------------------------------------------
    CONST GRID_VALUE_ROW  = 1
    CONST GRID_PANEL_ROW  = 3    ' dark green starts here: a blank row of padding
    CONST GRID_ROW0       = 4
    CONST GRID_COL0       = 2
    CONST GRID_ROWS       = 6
    CONST GRID_COLS       = 16
    CONST GRID_ACTION_ROW = 11
    CONST GRID_ACT_COL0   = 2    ' SPC
    CONST GRID_ACT_COL1   = 7    ' DEL
    CONST GRID_ACT_COL2   = 12   ' OK
    CONST GRID_ACT_COL3   = 17   ' ESC

    ' Pixel offset from background card (0,0) to MOB coordinates, for the
    ' inverse-video cursor. Measured against the emulator by pinning the cursor
    ' on a known cell and reading back where the block landed: both axes are
    ' offset by 8, i.e. MOB (8,8) is the top-left card. Don't infer this from
    ' the IntyBASIC manual's coordinate ranges (X 0-168, Y 0-95) -- they imply
    ' an asymmetry that isn't there.
    CONST GRID_MOB_X0     = 8
    CONST GRID_MOB_Y0     = 8

    ' 9, not 10, and the row below the last entry (row 10) is left permanently
    ' blank. That spacer is load-bearing, not cosmetic: the file browser draws
    ' in color stack mode as RED / content / selected / content / RED, which
    ' fits the 4-entry stack exactly -- but only while a content row exists
    ' BOTH above and below the selection bar. Row 10 is what guarantees the
    ' lower one when the bottom entry is selected. See csbar.bas.
    CONST ENTRIES_PER_PAGE = 9
    CONST FILES_SPACER_ROW = 10
    CONST FILES_START_ROW  = 1
    CONST DIR_MAX_LEN      = 36    ' short-form entry length from READ_DIR_ENTRY
    CONST NUM_HOST_SLOTS   = 8
    CONST HOST_NAME_LEN    = 32
    CONST FILTER_LEN       = 32    ' matches src/select_file.c's char filter[32]

    ' Cross-module selection state (device slot 0 is the only one this
    ' program ever uses -- see the plan doc's scope: no device-slot screen).
    '
    ' copy_mode mirrors src/select_file.c's bool of the same name: 0 = an
    ' ordinary browse, 1 = a copy is in flight and the host list / file
    ' browser are picking the DESTINATION. copy_host_slot remembers the
    ' source host across that hand-off (src/perform_copy.c:13).
    CONST DEVICE_SLOT = 0
    DIM host_slot, sel_row
    DIM copy_mode, copy_host_slot

    ' Video mode currently programmed into the STIC, and the one the current
    ' state wants: 0 = color stack (IntyBASIC's default, black backgrounds),
    ' 1 = FGBG (ST_HOSTS only). config.bas's main loop issues MODE only when
    ' these differ, since MODE costs a frame and clobbers PRINT's color.
    DIM vid_now, vid_want

    ' -------------------------------------------------------------------
    ' Scratch RAM ($9000-$97FF) -- ours, outside the mailbox proper ($9C00+).
    ' Holds state that must survive across multiple mailbox transactions,
    ' since the mailbox's own TX/RX buffers are overwritten by every call.
    ' -------------------------------------------------------------------
    CONST SC_HOSTS = $9000  ' 256  8 x 32, verbatim READ_HOST_SLOTS/WRITE_HOST_SLOTS mirror
    CONST SC_PATH  = $9100  ' 192  current directory path, NUL-terminated, leading '/'
    CONST SC_ENTRY = $91C0  ' 128  highlighted long filename (bounce-scroll source)
    CONST SC_SSID  = $9240  '  36  SSID being configured
    CONST SC_PASS  = $9264  '  68  passphrase being configured
    CONST SC_EDIT  = $92B0  ' 256  grid-entry accumulator (host names, custom SSID)
    CONST SC_ELEN  = $93B0  '  16  per-visible-row entry length (10 used)
    CONST SC_EDIR  = $93C0  '  16  per-visible-row is-directory flag (10 used)
    CONST SC_EPOS  = $93D0  '  32  per-visible-row ABSOLUTE directory position, 10 x 2 LE
    CONST SC_PSTK  = $93F0  '  20  page-start position stack, 10 x 2 LE (prev-page)
    CONST SC_BOOTPATH = $9410  ' 256  full path (SC_PATH + chosen filename) for SET_DEVICE_FULLPATH
    CONST SC_FILTER = $9510  '  32  active directory filter, NUL-terminated ("" = none)
    CONST SC_SRC    = $9530  ' 224  copy source: full path of the marked file
    ' $9610-$97FF free

    ' Directory-browsing state shared between st_hosts.bas (which resets it
    ' on mount) and st_file.bas.
    DIM dir_eof, num_rows, pstk_depth
    DIM #dp_start   ' absolute directory position the current page starts at
