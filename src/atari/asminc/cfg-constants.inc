.ifndef ATESC
    .include    "atari.inc"
.endif

CFK_ESC     = ATESC         ; Escape
CFK_ENTER   = ATEOL         ; Return/Enter
CFK_TAB     = ATTAB         ; Tab
CFK_BS      = ATRUB         ; Backspace
CFK_DEL     = ATDEL         ; Delete
CFK_INS     = ATINS         ; Insert
CFK_LEFT    = ATLRW         ; Left Arrow
CFK_LEFT2   = $2B           ; Left Arrow   Alt "+"
CFK_RIGHT   = ATRRW         ; Right Arrow
CFK_RIGHT2  = $2A           ; Right Arrow  Alt "*" 
CFK_UP      = ATURW         ; Up Arrow
CFK_UP2     = $2D           ; Up Arrow     Alt "-"
CFK_DOWN    = ATDRW         ; Down Arrow
CFK_DOWN2   = $3D           ; Down Arrow   Alt "="
CFK_ASCIIL  = $20           ; Lowest ASCII code inclusive considered normal key press
CFK_ASCIIH  = $7D           ; Highest ASCII code inclusive considered normal key press
CFK_HOME    = $01           ; ctrl-a, start of line char
CFK_END     = $05           ; ctrl-e, end of line char
CFK_KILL    = $0B           ; ctrl-k, kill to end of line
CFK_PARENT  = $3C           ; "<" used in files to go to parent dir
CFK_FILTER  = $46           ; "F" Filter
CFK_FILTER2 = $66           ; "f" Filter
