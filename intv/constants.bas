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
    CONST CS_ADVANCE    = $2000   ' Advance the color stack by one position.

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
    CONST GRID_ROW0       = 3
    CONST GRID_COL0       = 2
    CONST GRID_ROWS       = 6
    CONST GRID_COLS       = 16
    CONST GRID_ACTION_ROW = 10
    CONST GRID_ACT_COL0   = 2    ' SPC
    CONST GRID_ACT_COL1   = 7    ' DEL
    CONST GRID_ACT_COL2   = 12   ' OK
    CONST GRID_ACT_COL3   = 17   ' ESC

    CONST ENTRIES_PER_PAGE = 10
    CONST FILES_START_ROW  = 1
    CONST DIR_MAX_LEN      = 36    ' short-form entry length from READ_DIR_ENTRY
    CONST NUM_HOST_SLOTS   = 8
    CONST HOST_NAME_LEN    = 32

    ' Cross-module selection state (device slot 0 is the only one this
    ' program ever uses -- see the plan doc's scope: no device-slot screen).
    CONST DEVICE_SLOT = 0
    DIM host_slot, sel_row

    ' -------------------------------------------------------------------
    ' Scratch RAM ($9000-$97FF) -- ours, outside the mailbox proper ($9800+).
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
    ' $9510-$97FF free

    ' Directory-browsing state shared between st_hosts.bas (which resets it
    ' on mount) and st_file.bas.
    DIM dir_eof, num_rows, pstk_depth
    DIM #dp_start   ' absolute directory position the current page starts at
