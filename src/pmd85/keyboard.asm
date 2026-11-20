; Keyboard for the PMD 85: own scan and translation, and kbhit()/cgetc()
; with CONFIG's key repeat timing.
;
; z88dk's in_Inkey scans the same way, but its table maps the |<- and ->|
; keys to nothing, and its getk() swaps 10 and 13 (EOL and END). This table
; follows z88dk's codes otherwise, so it could become a patch to
; libsrc/target/pmd85/input/in_keytranstbl.asm.
;
; Hardware: write a column (0-14) to port F4h (8255 port A), read port F5h
; (port B): bits 0-4 are that column's keys, bit 5 SHIFT, bit 6 STOP; a
; pressed key reads 0.

    SECTION code_user

    PUBLIC  _pmd85_inkey
    PUBLIC  _pmd85_kbhit
    PUBLIC  _pmd85_cgetc

    defc    COLUMNS=15
    defc    REPEAT_START=30             ; x 10 ms before a held key repeats
    defc    REPEAT_RATE=60              ; ms between repeats
    defc    MS_LOOPS=85                 ; 2.048 MHz / 24 T-states per loop

; unsigned char pmd85_inkey(void)
; The key held now, translated, or 0.
_pmd85_inkey:
    call    scan
    ld      l, a
    ld      h, 0
    ret

; Exit: a = key code or 0, (key_pos) = the key's position + 1 or 0.
; Uses bc, de, hl.
scan:
    ld      b, 0                        ; column
    ld      e, 0                        ; table index of the column's first key
scan_column:
    ld      a, b
    out     ($f4), a
    in      a, ($f5)
    cpl
    ld      c, a                        ; c = pressed keys and modifiers
    and     @00011111
    jr      nz, scan_key
    ld      a, e
    add     a, 5
    ld      e, a
    inc     b
    ld      a, b
    cp      COLUMNS
    jr      nz, scan_column
scan_none:
    xor     a
    ld      (key_pos), a
    ret

scan_key:
    rrca                                ; lowest pressed key in the column
    jr      c, scan_table
    inc     e
    jr      scan_key

scan_table:
    ld      a, e
    inc     a
    ld      (key_pos), a
    ld      d, 0
    ld      hl, keys
    ld      a, c
    and     @00100000                   ; SHIFT
    jr      z, scan_stop
    ld      hl, keys_shift
    jr      scan_lookup
scan_stop:
    ld      a, c
    and     @01000000                   ; STOP
    jr      z, scan_lookup
    ld      hl, keys_stop
scan_lookup:
    add     hl, de
    ld      a, (hl)
    cp      255
    ret     nz
    jr      scan_none

; unsigned char pmd85_kbhit(void)
; 1 when a key is waiting. A held key is reported at once, again after
; REPEAT_START, then every REPEAT_RATE. Held means the same key position, so
; letting go of SHIFT before the letter doesn't count as a new key.
_pmd85_kbhit:
    ld      a, (pending)
    or      a
    jr      nz, kbhit_yes
    call    scan
    or      a
    jr      z, kbhit_released
    ld      c, a
    ld      a, (key_pos)
    ld      hl, last_pos
    cp      (hl)
    jr      z, kbhit_held
    ld      (hl), a                     ; fresh press: report it at once
    ld      a, c
    ld      (last_key), a
    ld      (pending), a
    xor     a
    ld      (repeating), a
    jr      kbhit_yes

kbhit_held:
    ld      a, (repeating)
    or      a
    jr      nz, kbhit_repeat
    ld      b, REPEAT_START             ; wait for the repeat, unless released
kbhit_wait:
    push    bc
    ld      hl, 10
    call    delay_ms
    call    scan
    pop     bc
    ld      a, (key_pos)
    ld      hl, last_pos
    cp      (hl)
    jr      nz, kbhit_released
    djnz    kbhit_wait
    ld      a, 1
    ld      (repeating), a
    jr      kbhit_report

kbhit_repeat:
    ld      hl, REPEAT_RATE
    call    delay_ms
kbhit_report:
    ld      a, (last_key)
    ld      (pending), a
kbhit_yes:
    ld      hl, 1
    ret

kbhit_released:
    xor     a
    ld      (last_pos), a
    ld      hl, 0
    ret

; unsigned char pmd85_cgetc(void)
; Waits for a key and returns it.
_pmd85_cgetc:
    call    _pmd85_kbhit
    ld      a, l
    or      a
    jr      z, _pmd85_cgetc
    ld      a, (pending)
    ld      l, a
    ld      h, 0
    xor     a
    ld      (pending), a
    ret

; Busy wait hl milliseconds. Uses a, de, hl.
delay_ms:
    ld      a, h
    or      l
    ret     z
    ld      de, MS_LOOPS
delay_loop:
    dec     de                          ; 5
    ld      a, d                        ; 5
    or      e                           ; 4
    jr      nz, delay_loop              ; 10 (jp on the 8080)
    dec     hl
    jr      delay_ms

    SECTION rodata_user

; 15 columns x 5 keys (bits 0-4). z88dk's codes, except: no 10/13 swap
; (EOL is 13, END 10), and |<- is 143, ->| 144.
keys:
    defb    27, '1', 'q', 'a', ' '      ; K0  1 Q A SPACE
    defb    128, '2', 'w', 's', 'y'     ; K1  2 W S Y
    defb    129, '3', 'e', 'd', 'x'     ; K2  3 E D X
    defb    130, '4', 'r', 'f', 'c'     ; K3  4 R F C
    defb    131, '5', 't', 'g', 'v'     ; K4  5 T G V
    defb    132, '6', 'z', 'h', 'b'     ; K5  6 Z H B
    defb    133, '7', 'u', 'j', 'n'     ; K6  7 U J N
    defb    134, '8', 'i', 'k', 'm'     ; K7  8 I K M
    defb    135, '9', 'o', 'l', ','     ; K8  9 O L ,
    defb    136, '0', 'p', ';', '.'     ; K9  0 P ; .
    defb    137, '_', '@', ':', '/'     ; K10 _ @ : /
    defb    138, '{', '\\', '[', 255    ; K11 { \ [
    defb    139, 255, 12, 143, 255      ; WRK INS LEFT |<-
    defb    7, 127, 11, 10, 13          ; C-D DEL UP END EOL
    defb    141, 142, 9, 144, 13        ; CLR RCL RIGHT ->| EOL2

keys_shift:
    defb    27, '!', 'Q', 'A', ' '
    defb    128, '\"', 'W', 'S', 'Y'
    defb    129, '#', 'E', 'D', 'X'
    defb    130, '$', 'R', 'F', 'C'
    defb    131, '%', 'T', 'G', 'V'
    defb    132, '&', 'Z', 'H', 'B'
    defb    133, '\'', 'U', 'J', 'N'
    defb    134, '(', 'I', 'K', 'M'
    defb    135, ')', 'O', 'L', '<'
    defb    136, '-', 'P', '+', '>'
    defb    137, '=', '`', '*', '?'
    defb    138, '}', '^', ']', 255
    defb    139, 127, 8, 143, 255
    defb    140, 127, 11, 10, 13
    defb    141, 142, 9, 144, 13

keys_stop:
    defb    27, '1', 17, 1, ' '
    defb    128, '2', 23, 19, 25
    defb    129, '3', 5, 4, 24
    defb    130, '4', 18, 6, 3
    defb    131, '5', 20, 7, 22
    defb    132, '6', 26, 8, 2
    defb    133, '7', 21, 10, 14
    defb    134, '8', 9, 11, 13
    defb    135, '9', 15, 12, ','
    defb    136, '0', 16, ';', '.'
    defb    137, '_', '@', ':', '/'
    defb    138, '{', '\\', '[', 255
    defb    139, 12, 8, 143, 255
    defb    140, 127, 11, 10, 13
    defb    141, 142, 9, 144, 13

    SECTION bss_user

pending:
    defb    0
last_key:                               ; code reported for the held key
    defb    0
last_pos:                               ; position + 1 of the held key, or 0
    defb    0
key_pos:                                ; position + 1 from the last scan
    defb    0
repeating:
    defb    0
