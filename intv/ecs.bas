' ecs.bas -- Mattel ECS keyboard driver.
'
' CONFIG is normally driven by the hand controller; this adds the ECS's
' 48-key QWERTY keyboard as a second source feeding the SAME three outputs
' in_poll already publishes, plus three new ones for text entry. Nothing
' here runs unless an ECS is actually attached (ecs_on), so a base
' Intellivision never writes to the ECS PSG at all.
'
' Lives in the $F000 segment (see config.bas) because the $5000/$6000
' auto-flow region has only ~309 words left before it would spill into
' $7000 and produce a cart that fails EXEC's boot detection.
'
' *** Why not CONT3/CONT4 ***
' IntyBASIC's CONT3/CONT4 (and SOUND 5-9) set the compiler's internal
' ecs_used flag, which makes the prologue boot the cart from $4800 and page
' out the ECS ROMs -- that would need a whole new cart segment plus a
' matching mm_add() in fujinet-firmware's fuji_config_map(). ECS.AVAILABLE
' does NOT set the flag (IntyBASIC.cpp:1227-1234): it just reads bit 1 of
' _ntsc, which the prologue already fills in on every boot by probing ECS
' RAM at $4040/$4041 with $55/$AA. So detection is free and the ROM layout
' is untouched. Everything below is plain PEEK/POKE, which is also free.
'
' *** The hardware ***
' ECS PSG at $00F0. $00F8 bit 6 = port A direction, bit 7 = port B, 1 =
' output. $00FE drives the matrix row, $00FF reads the column -- BOTH
' active low, so a pressed key reads as a 0 and every read is XORed with
' 255. Ground truth is jzIntv's src/pads/pads.c:192-372 (the emulation) and
' src/cfg/mapping.c:443-456 (the matrix), cross-checked against the
' ecs_key_codes[] table in fujinet-go-intv-desktop's core/jzintv/intv_host.c.
'
'         b0      b1      b2   b3   b4     b5      b6      b7
'   row 0 left    .       ;    p    esc    0       enter   --
'   row 1 ,       m       k    i    9      8       o       l
'   row 2 n       b       h    y    7      6       u       j
'   row 3 v       c       f    r    5      4       t       g
'   row 4 x       z       s    w    3      2       e       d
'   row 5 space   down    up   q    1      right   ctl     a
'   row 6 --      --      --   --   --     --      --      shift

    ' Sentinels for the non-printable keys. Deliberately 1-8, below ASCII
    ' space, so one variable can carry "a character" or "a special" and
    ' ek_ch >= 32 is exactly the test for "this is printable".
    '
    ' *** KB_, not EK_, and that matters. *** IntyBASIC identifiers are
    ' CASE-INSENSITIVE, so constants and variables share one namespace: a
    ' CONST EK_SHIFT and a DIM ek_shift are the same name. The compiler
    ' accepts it silently, the constant wins every read, and the only
    ' symptom is that the ECS keyboard types as though SHIFT were welded
    ' down. Keep constants on KB_ and variables on ek_.
    CONST KB_LEFT  = 1
    CONST KB_ESC   = 2
    CONST KB_ENTER = 3
    CONST KB_DOWN  = 4
    CONST KB_UP    = 5
    CONST KB_RIGHT = 6
    CONST KB_CTRL  = 7
    CONST KB_SHIFT = 8

' ---------------------------------------------------------------------------
' ecs_init: called once from cfg_start, before the first in_poll.
' ---------------------------------------------------------------------------
ecs_init: PROCEDURE
    ecs_on = 0
    IF ECS.AVAILABLE THEN ecs_on = 1

    ' Every output has to start inert, because in_poll folds these in
    ' unconditionally and ecs_scan never runs when there is no ECS.
    ' ek_key especially: IntyBASIC zeroes 8-bit RAM at boot and 0 is
    ' KEYPAD_0, so leaving it zero would press keypad 0 on every frame.
    ek_key  = KEYPAD_NONE
    ek_disc = 0
    ek_btn  = 0
    ek_ch   = 0
    ek_pch  = 0
    in_char = 0
    in_bs   = 0
    in_esc  = 0
    in_ret  = 0
END

' ---------------------------------------------------------------------------
' ecs_scan: one full keyboard read. Called from in_poll, once per frame,
' only when ecs_on.
'
' Two passes, because the QWERTY matrix has no diodes and jzIntv models the
' resulting buffer fights faithfully (pads.c:310-318). Holding SHIFT (row 6 /
' bit 7) ghosts out every other key in bit 7 during a normal scan -- that is
' A, D, G, J and L. A transposed scan has the mirror problem and loses row 6
' (E, O, T, U) instead. So: normal scan for rows 0-5 bits 0-6, then ONE
' transposed pass driving bit 7 to pick up L/J/G/D/A cleanly. SHIFT is read
' in both passes and OR'd, so whichever pass is being ghosted, the other
' still sees it. Net result: the whole alphabet and every ECS symbol is
' typeable.
' ---------------------------------------------------------------------------
ecs_scan: PROCEDURE
    ek_code  = 0
    ek_shift = 0

    ' --- pass 1: normal. Drive rows on $00FE, read columns on $00FF. ---
    ' $00F8 must be set BEFORE the port write: pads.c only latches a write
    ' to $00FE/$00FF when that port is already configured as an output.
    POKE ECS_PSG_CTRL, ECS_CTRL_NORMAL
    FOR ek_r = 0 TO 6
        POKE ECS_PSG_PA, ecs_rowmask(ek_r)
        ek_bits = (PEEK(ECS_PSG_PB) AND 255) XOR 255
        ' The overwhelmingly common case is "nothing held in this row", so
        ' skip the inner loop entirely -- that keeps a quiet frame down to
        ' seven poke/peek pairs rather than 56 array reads.
        IF ek_bits <> 0 THEN
            IF ek_r = 6 THEN
                IF (ek_bits AND 128) <> 0 THEN ek_shift = 1
            ELSE
                FOR ek_c = 0 TO 6
                    IF (ek_bits AND ecs_bit(ek_c)) <> 0 THEN ek_code = ek_r * 8 + ek_c + 1
                NEXT ek_c
            END IF
        END IF
    NEXT ek_r

    ' --- pass 2: transposed. Drive column 7 on $00FF, read rows on $00FE. ---
    POKE ECS_PSG_CTRL, ECS_CTRL_TRANS
    POKE ECS_PSG_PB, 127
    ek_bits = (PEEK(ECS_PSG_PA) AND 255) XOR 255
    ' Put the ports back the way the prologue left them before doing
    ' anything else, so nothing on the machine ever observes them driven.
    POKE ECS_PSG_CTRL, ECS_CTRL_IDLE
    IF ek_bits <> 0 THEN
        IF (ek_bits AND 64) <> 0 THEN ek_shift = 1
        FOR ek_r = 1 TO 5
            IF (ek_bits AND ecs_bit(ek_r)) <> 0 THEN ek_code = ek_r * 8 + 8
        NEXT ek_r
    END IF

    ' --- decode ---
    ek_ch = 0
    IF ek_code <> 0 THEN
        IF ek_shift <> 0 THEN
            ek_ch = ecs_shifted(ek_code - 1)
        ELSE
            ek_ch = ecs_normal(ek_code - 1)
        END IF
    END IF

    ' --- levels: arrows and RETURN ride the existing disc/button logic ---
    ' Levels, not edges, deliberately: in_poll's auto-repeat gate and button
    ' edge detector then treat a held key exactly like a held disc/button,
    ' with no second copy of that logic here.
    ek_disc = 0
    IF ek_ch = KB_UP THEN ek_disc = DISC_UP
    IF ek_ch = KB_DOWN THEN ek_disc = DISC_DOWN
    IF ek_ch = KB_RIGHT THEN ek_disc = DISC_RIGHT
    ek_btn = 0
    IF ek_ch = KB_ENTER THEN ek_btn = 1

    ' --- edges: characters, ESC, and the digit-as-keypad shortcuts ---
    ' Same one-code-of-history scheme as in_poll's own in_pkey: CONFIG is a
    ' one-key-at-a-time UI, so a single previous code is enough.
    in_char = 0
    in_esc  = 0
    in_ret  = 0
    ek_key  = KEYPAD_NONE
    IF ek_ch <> 0 AND ek_ch <> ek_pch THEN
        IF ek_ch >= 32 THEN in_char = ek_ch
        ' Unshifted digits double as the keypad shortcuts every screen
        ' already understands (1 up a dir, 2/3 page, 4 filter, 5 copy, 9
        ' info, 0 lobby). Shifted they are symbols, so ek_ch is not 48-57
        ' and no shortcut fires -- SHIFT+1 types "=", it does not page.
        IF ek_ch >= 48 AND ek_ch <= 57 THEN ek_key = ek_ch - 48
        IF ek_ch = KB_ESC THEN in_esc = 1 : ek_key = KEYPAD_CLEAR
        ' RETURN reaches every other screen as the action button (via ek_btn
        ' above), but grid_entry reads the action button as "type the
        ' character under the cursor" -- so it needs RETURN separately, to
        ' mean accept.
        IF ek_ch = KB_ENTER THEN in_ret = 1
    END IF
    ek_pch = ek_ch

    ' --- left arrow = backspace, with its own repeat gate ---
    ' The ECS has no BACKSPACE key, so the left arrow is it. This is the one
    ' arrow that does NOT feed ek_disc, which costs nothing: no screen
    ' outside grid_entry uses DISC_LEFT. Held-to-repeat matters here more
    ' than anywhere else -- deleting a mistyped passphrase one frame-edge at
    ' a time would be miserable.
    in_bs = 0
    IF ek_ch <> KB_LEFT THEN
        ek_bsdelay = 0
    ELSE
        IF ek_bsdelay = 0 THEN
            in_bs = 1 : ek_bsdelay = IN_REPEAT_DELAY
        ELSE
            ek_bsdelay = ek_bsdelay - 1
            IF ek_bsdelay = 0 THEN in_bs = 1 : ek_bsdelay = IN_REPEAT_RATE
        END IF
    END IF
END

' ---------------------------------------------------------------------------
' Tables. Row drive values are active low (one 0 bit selects that row).
' ---------------------------------------------------------------------------
ecs_rowmask:
    DATA 254,253,251,247,239,223,191

ecs_bit:
    DATA 1,2,4,8,16,32,64,128

' ecs_normal / ecs_shifted: ASCII (or a KB_* sentinel) for each matrix
' position, indexed row*8+col. Written as explicit numbers, NOT string
' literals -- DATA "..." converts to Intellivision GROM card codes, not
' ASCII, which is not what #ge_dst wants.
'
' Unshifted letters are lowercase and SHIFT gives uppercase. That is the
' PC convention rather than the ECS's uppercase key caps, and it is what
' makes typing through fujinet-go-intv-desktop work: it resolves host keys
' by CHARACTER, sending plain A for "a" and SHIFT+A for "A".
ecs_normal:
    '     b0            b1       b2       b3       b4       b5        b6        b7
    DATA KB_LEFT,       46,      59,     112, KB_ESC,        48, KB_ENTER,        0   ' left . ; p esc 0 ent --
    DATA      44,      109,     107,     105,     57,        56,      111,      108   ' , m k i 9 8 o l
    DATA     110,       98,     104,     121,     55,        54,      117,      106   ' n b h y 7 6 u j
    DATA     118,       99,     102,     114,     53,        52,      116,      103   ' v c f r 5 4 t g
    DATA     120,      122,     115,     119,     51,        50,      101,      100   ' x z s w 3 2 e d
    DATA      32,  KB_DOWN,   KB_UP,     113,     49,  KB_RIGHT,  KB_CTRL,       97   ' spc dn up q 1 rt ctl a
    DATA       0,        0,       0,       0,      0,         0,        0, KB_SHIFT   ' -- shift

' The shifted layer is the ECS's OWN, which bears no relation to a PC's:
' SHIFT+1..0 are = " # $ + - / * ( ), and % ' ^ ? live on SHIFT plus the
' four arrow keys. Decoding exactly that is what lets someone type "=" on a
' PC and get "=" here, because the frontend sends whatever chord the ECS
' puts that character on. Characters the ECS has nowhere at all (! @ & [ _
' and friends) simply cannot be typed -- the character grid is still there
' for those.
ecs_shifted:
    '     b0            b1       b2       b3       b4       b5        b6        b7
    DATA      37,       62,      58,      80, KB_ESC,        41, KB_ENTER,        0   ' % > : P esc ) ent --
    DATA      60,       77,      75,      73,     40,        42,       79,       76   ' < M K I ( * O L
    DATA      78,       66,      72,      89,     47,        45,       85,       74   ' N B H Y / - U J
    DATA      86,       67,      70,      82,     43,        36,       84,       71   ' V C F R + $ T G
    DATA      88,       90,      83,      87,     35,        34,       69,       68   ' X Z S W # " E D
    DATA      32,       63,      94,      81,     61,        39,  KB_CTRL,       65   ' spc ? ^ Q = ' ctl A
    DATA       0,        0,       0,       0,      0,         0,        0, KB_SHIFT   ' -- shift
