; equ.inc -- constants: the console, the cartridge, the screen, FujiNet.

; ---- P1 (all active low but COPY and LUM) ----
; Main code runs with external RAM and the cartridge on the bus; the interrupts
; switch to the VDC and put P1 back. COPY low always, or the cart's write
; strobe is gated off. The bank bits are left high: APPBANK pages are picked
; through the mailbox, not by P10/P11.
P1_MAIN equ 0AFh                ; XRAM + cart selected, VDC parked
P1_VDC  equ 0B7h                ; VDC selected
P1_JOY  equ 0BBh                ; keyboard scanner (joystick) selected

; ---- the mailbox (fuji_mailbox.h, protocol 2) ----
FN_W_REGSEL     equ 0E0h
FN_W_REGDATA    equ 0E1h
FN_W_DATA       equ 0E2h
FN_W_SEQ        equ 0E3h
FN_REG_DEVICE   equ 000h
FN_REG_CMD      equ 001h
FN_REG_NPARAM   equ 002h
FN_REG_DATA_RST equ 005h
FN_REG_CODEBANK equ 008h
FN_REG_POKE_LO  equ 00Ah
FN_REG_POKE_HI  equ 00Bh
FN_REG_POKE     equ 00Ch
FN_REG_RXWIN    equ 00Dh
FN_R_ACKSEQ     equ 0F20h
FN_R_STATUS     equ 0F21h
FN_R_ERR        equ 0F22h
FN_R_REPLY_CMD  equ 0F23h
FN_R_RXLEN_LO   equ 0F24h
FN_R_RXLEN_HI   equ 0F25h
FN_R_BOOT_STATE equ 0F26h
FN_R_BOOT_PCT   equ 0F27h
FN_R_BOOT_ERR   equ 0F28h
FN_R_MAGIC0     equ 0F29h
FN_R_MAGIC1     equ 0F2Ah
FN_R_PROTO_VER  equ 0F2Bh
FN_R_DATA       equ 0F30h       ; the read window: 208 bytes from RXWIN*16
FN_PROTO_VER    equ 2
O2_APP_HDR      equ 0F30h
O2_APP_VERSION  equ 1

FN_BOOT_IDLE    equ 0           ; BOOT_STATE
FN_BOOT_XFER    equ 1
FN_BOOT_READY   equ 2
FN_BOOT_FAILED  equ 80h

; ---- the Fuji device (fujiCmd.h) ----
FUJI_DEVICEID   equ 070h
FC_ACK          equ 006h
FC_NAK          equ 015h
FC_COPYFILE     equ 0D8h        ; COPY_FILE (src, dst 1-based; "src|dst")
FC_ADAPTEXT     equ 0C4h        ; GET_ADAPTERCONFIG_EXTENDED
FC_HPREFIXGET   equ 0E0h        ; GET_HOST_PREFIX (host)
FC_HPREFIXSET   equ 0E1h        ; SET_HOST_PREFIX (host; 256-byte path)
FC_DEVPATH      equ 0E2h        ; SET_DEVICE_FULLPATH (slot, host, mode; 256)
FC_DIRPOS       equ 0E4h        ; SET_DIRECTORY_POSITION (u16)
FC_WIFIEN       equ 0EAh        ; GET_WIFI_ENABLED
FC_WRHOSTS      equ 0F3h        ; WRITE_HOST_SLOTS (256)
FC_RDHOSTS      equ 0F4h        ; READ_HOST_SLOTS
FC_CLOSEDIR     equ 0F5h        ; CLOSE_DIRECTORY
FC_READDIR      equ 0F6h        ; READ_DIR_ENTRY (maxlen, addtl)
FC_OPENDIR      equ 0F7h        ; OPEN_DIRECTORY (host; "path\0filter\0" 256)
FC_MOUNTIMG     equ 0F8h        ; MOUNT_IMAGE (slot, mode)
FC_MOUNTHOST    equ 0F9h        ; MOUNT_HOST (host)
FC_WIFISTAT     equ 0FAh        ; GET_WIFISTATUS
FC_SETSSID      equ 0FBh        ; SET_SSID (save; ssid 33, password 64)
FC_SCANRES      equ 0FCh        ; GET_SCAN_RESULT (n)
FC_SCAN         equ 0FDh        ; SCAN_NETWORKS
FC_GETSSID      equ 0FEh        ; GET_SSID

WL_CONNECTED    equ 3           ; GET_WIFISTATUS

; ---- the VDC's character generator (o2doc appendix C) ----
G_SPACE equ 12
G_DOT   equ 39
G_DASH  equ 40
G_BLOCK equ 47
CH_TIMES equ 09h                ; codes $00-$1F in text: glyph code+32
CH_DIV   equ 0Ah                ; (mkcharmap.py)
CH_BLOCK equ 0Fh
CH_ARROW equ 16h

; Character colours, as MAME's i8244 draws them (control byte bits 1-3).
; A character can't be black: 0 is the darkest, a dark grey.
C_DGREY  equ 0
C_RED    equ 1
C_GREEN  equ 2
C_TAN    equ 3
C_BLUE   equ 4
C_VIOLET equ 5
C_CYAN   equ 6
C_WHITE  equ 7

; The colour register ($A3): bits 5-3 the background (MAME bitswap 3,4,5:
; bit 5 red, 4 green, 3 blue), bits 6 and 2-0 the grid (bitswap 6,0,1,2:
; bit 6 bright, 2 red, 1 green, 0 blue).
BG_BLACK equ 000h
BG_BLUE  equ 008h
GRID_BLUE  equ 001h             ; the selection bar: dark blue
GRID_LBLUE equ 041h
GRID_LGREEN equ 042h            ; the boot's progress bar

; The control register ($A0). The selection bar is the grid in fill mode: a
; vertical segment is a whole 16 x 24 cell.
CTL_TOP equ vdc_ctrl_fore                               ; T0: the grid hidden
CTL_ALL equ vdc_ctrl_fill|vdc_ctrl_fore|vdc_ctrl_grid   ; the rows
CTL_GON equ vdc_ctrl_fill|vdc_ctrl_grid                 ; a band: grid on
CTL_OFF equ 0                                           ; a band: all off

; ---- the screen ----
; Characters are 7 rows of 2 lines. Every screen is T0 at the top and T1 at
; the bottom (the quads, 16 characters) and eight rows of twelve singles
; between them (fixed.inc). The grid's rows are 24 lines from line 24, its
; columns 16 units from X 8 (a character's X register; it draws 5 units
; right); row r's characters sit in grid row r.
X_TEXT  equ 16                  ; 16 characters at 8 px, centred
TXT_LEN equ 16
TXT_TAB equ 1+2*TXT_LEN         ; a text table: Y, then 16 x (pointer, control)
    ifndef Y_T0
Y_T0    equ 16                  ; MAME hides characters above Y=14 (NTSC)
    endif
L_TOP   equ 0                   ; say_line's lines: T0, T1
L_BOT   equ 1
NCOLS   equ 12                  ; a row's singles
ROW_TAB equ 2*NCOLS+1           ; a row: NCOLS x (pointer, control), sprite 1's Y
ROW_SPR equ 2*NCOLS             ; ... its last byte
NROWS   equ 8
    ifndef Y_ROW0
Y_ROW0  equ 30                  ; row 0's characters; row r's is +24r
    endif
ROW_PITCH equ 24
    ifndef Y_T1
Y_T1    equ 228                 ; T1 (MAME draws to line 242)
    endif
    ifndef LEAD
LEAD    equ 1                   ; lines a band starts before its line ends
    endif
    ifndef GRIDNOP
GRIDNOP equ 3                   ; h_grid: nops to put its write in the blank
    endif                       ; (NTSC)
PALPASS equ 190                 ; pal_check (p0.inc): the passes a frame from
                                ; which a console is PAL (MAME: NTSC ~100, PAL ~290)
    ifndef GRIDNOPP
GRIDNOPP equ 5                  ; ... and on PAL, more (h_gridp's jmp: 2 more)
    endif
    ifndef X_ROWTXT
X_ROWTXT equ 36                 ; a row's 12 characters, centred
    endif
X_ICON  equ X_ROWTXT-12         ; sprite 1 (a folder) left of a row
SPR_OFF equ 0F8h                ; a sprite's Y below the screen
    ifndef SPKNOIN
SPKNOIN equ 0                   ; spike: no input polls
    endif
    ifndef PADN
PADN    equ 0                   ; spike: pad a row band by 2+2n cycles
    endif

; ---- the stick (input.inc): JOY / JOYEV bits ----
J_UP    equ 01h
J_DOWN  equ 02h
J_LEFT  equ 04h
J_RIGHT equ 08h
J_FIRE  equ 10h

; ---- the keyboard (input.inc): BIOS readkey codes, row x 8 + column ----
K_0     equ 00h                 ; 0-9 are 00h-09h
K_SPACE equ 0Ch
K_QUEST equ 0Dh
K_L     equ 0Eh
K_P     equ 0Fh
K_PLUS  equ 10h
K_W     equ 11h
K_E     equ 12h
K_R     equ 13h
K_T     equ 14h
K_U     equ 15h
K_I     equ 16h
K_O     equ 17h
K_Q     equ 18h
K_S     equ 19h
K_D     equ 1Ah
K_F     equ 1Bh
K_G     equ 1Ch
K_H     equ 1Dh
K_J     equ 1Eh
K_K     equ 1Fh
K_A     equ 20h
K_Z     equ 21h
K_X     equ 22h
K_C     equ 23h
K_V     equ 24h
K_B     equ 25h
K_M     equ 26h
K_DOT   equ 27h
K_MINUS equ 28h
K_TIMES equ 29h                 ; x: the shift key
K_DIV   equ 2Ah                 ; divide: the second shift
K_EQ    equ 2Bh
K_YES   equ 2Ch
K_NO    equ 2Dh
K_CLR   equ 2Eh
K_ENT   equ 2Fh
K_NONE  equ 0FFh

; ---- sounds (snd.inc): beep ids; from SND_SOFT on they never cut another ----
SND_SELECT equ 0
SND_OK     equ 1                ; connected, copied
SND_ERROR  equ 2
SND_BOOT   equ 3                ; the fanfare
SND_BACK   equ 4
SND_SOFT   equ 5
SND_MOVE   equ 5                ; a click
SND_KEY    equ 6                ; a key typed
SND_TICK   equ 7                ; the progress bar's tick
SND_N1     equ 8                ; the jingle's notes (snd.inc), then BOOT
SND_N2     equ 9
SND_N3     equ 10

; ---- pages, and their entries ----
PG_BOOT  equ 0
PG_LIB   equ 1                  ; the display's routines (p1.inc), by farcall
PG_HOSTS equ 2
PG_FILES equ 3
PG_MOUNT equ 4
PG_EDIT  equ 5
PG_WIFI  equ 6
PG_MISC  equ 7                  ; info, copy, the lobby
PG_HELP  equ 8
PG_DIR   equ 9                  ; the browser's folders: open, count, up, in
PG_EDLIB equ 10                 ; the editor's texts and magnifier, by farcall
PG_COPY  equ 11
E_RUN    equ 0                  ; every page's: its screen
E_CHECK  equ 1                  ; the WiFi page's: the boot's check

CFG_VER  equ "1.0"              ; the splash's version

; HELP: the help screen on context ctx, back to page pg's entry en.
HELP    macro ctx,pg,en
    mov r0,#HELPCTX
    mov a,#ctx
    movx @r0,a
    mov r0,#HRET
    mov a,#(en<<4)|pg
    movx @r0,a
    mov r7,#E_RUN
    mov a,#PG_HELP
    jmp far_go
    endm

; GOTO: one way to another page's entry (page.inc far_go).
GOTO    macro pg,en
    mov r7,#en
    mov a,#pg
    jmp far_go
    endm
E_EDDONE equ 2                  ; the WiFi page's: the editor is done
E_LIST   equ 3                  ; ... the list again, no new scan
E_CONN   equ 4                  ; ... connect to the SSID saved

E_MOUNT  equ 1                  ; the boot page's: the path is set (the lobby)
E_HRET   equ 1                  ; the editor's: back from the help
E_INFO   equ 0                  ; the misc page's: info (E_RUN)
E_LOBBY  equ 1                  ; ... the lobby

MQ_WAIT  equ 45                 ; a long name rests this long at each end
MQ_STEP  equ 8                  ; ... and steps a character this often

; the editor (p5.inc; its texts and magnifier, edtext.inc)
ED_ROW0 equ 1                   ; the field's first row
ED_ROWS equ 6
ED_MAGX equ 138                 ; the magnifier: sprite 2, zoomed (16 x 28),
ED_MAGY equ Y_T0                ; T0 and row 0: no band blanks it there
SH_NONE equ 0                   ; SHIFT
SH_ONE  equ 1                   ; x: the next key
SH_SYM  equ 2                   ; divide: the next key
SH_LOCK equ 3                   ; x x: until x

; the editor's kinds, the help's screens (p8.inc)
ED_SSID   equ 0
ED_PASS   equ 1
ED_HOST   equ 2
ED_FILTER equ 3
HC_WIFI   equ 0
HC_HOSTS  equ 1
HC_FILES  equ 2
HC_EDIT   equ 3
HC_BOOT   equ 4
HC_INFO   equ 5

; the library's entries (p1.inc; local.inc's doors)
L_SAYLINE  equ 0
L_SAYLINEC equ 1
L_ROWSTR   equ 2
L_ROWLINE  equ 3
L_SCRTEXT  equ 4
L_BAR      equ 5
L_GRID     equ 6
L_SPRAT    equ 7
L_SPRHIDE  equ 8
L_GCOL     equ 9
L_FOLDER   equ 10
L_COLOR    equ 11

; the editor's library's entries (p10.inc)
L_EDPROMPT equ 0
L_EDSTAT   equ 1
L_EDCHART  equ 2
L_MAG      equ 3
E_PAGE   equ 1                  ; the browser's: the same page again
E_FEDIT  equ 2                  ; ... the filter edited
E_SHOW   equ 3                  ; ... the page at POS, the folder open
E_OPEN   equ 0                  ; the folders' (PG_DIR): open, count, show
E_REOPEN equ 1                  ; ... open, show (counted already)
E_PARENT equ 2                  ; ... up a folder
E_ENTER  equ 3                  ; ... into the bar's folder
E_FILTER equ 4                  ; ... the filter to the editor
E_MARK   equ 5                  ; ... the bar's file marked for a copy
E_NEW    equ 6                  ; ... a folder from its top
E_FEDONE equ 7                  ; ... the editor back with the filter
DIRPAGES equ 4                  ; READ_DIR_ENTRY block mode: 4 x 256 bytes,
DIRBLOCK equ 0C1h               ; ... one entry a group (fujiDevice.cpp)
