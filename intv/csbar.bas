' csbar.bas -- color-stack backgrounds and file-type glyphs for the file
' browser (st_file.bas). The other two coloured screens use FGBG mode, which
' can only reach GROM cards 0-63 and so renders uppercase-only; filenames are
' the whole point of this screen, so it stays in color stack mode instead and
' the layout was chosen to fit what the stack can express.
'
' The stack holds four background entries, resets to position 0 at the top of
' each frame, and only ever advances forward, wrapping 3->0. In raster order
' this screen is GREEN / content / selected / content / GREEN -- five runs, and
' the fifth wraps neatly back onto position 0, so config.bas programs it as:
'
'     MODE 0, CS_DARKGREEN, CS_BLUE, CS_CYAN, CS_BLUE
'              p0       p1       p2       p3
'
' That only holds while BOTH content runs are non-empty, which is why two
' things about the layout are not negotiable:
'
'   * The selection bar spans columns 1-19 only. Column 0 keeps the content
'     colour on every row, selected or not, and is the non-empty content run
'     when the TOP entry is selected. It is also exactly where the file-type
'     glyph goes -- the constraint and the feature are the same cell.
'   * Row 10 is a permanent blank spacer (see ENTRIES_PER_PAGE in
'     constants.bas) and is the non-empty content run when the BOTTOM entry
'     is selected. Without it the command row would have to give up its
'     leftmost cell.
'
' Placed in the $D000 segment (INCLUDEd after st_boot.bas in config.bas) for
' the same reason st_lobby.bas and sf_draw_hint are: the default $5000-$6FFF
' segment is essentially full, and st_file.bas has more claim on it.

    ' GRAM card numbers and GRAM_SELECT live in constants.bas: input.bas
    ' references GLYPH_BLOCK and is INCLUDEd well before this file, so a CONST
    ' declared here would not exist yet at its point of use -- IntyBASIC would
    ' silently treat it as an unassigned variable reading 0, and the grid's
    ' cursor would draw the folder glyph instead of the solid block.


' ---------------------------------------------------------------------------
' File-type glyphs, lifted from the other FujiNet config clients. No bit
' twiddling was needed: Intellivision GRAM is 8x8, one byte per row, MSB
' leftmost -- identical to the TMS9918 layout these came from.
'
' The folder is byte-for-byte the same shape in the ADAM (src/adam/screen.c,
' chars $83/$84 composited), MSX (src/msx/gfxutil.c) and Atari
' (src/atari/screen.c, CH_FOLDER) clients. The cartridge is the ADAM client's
' ROM icon (chars $89/$8a composited).
'
' Those two clients get the icon handed to them: the firmware prepends icon
' bytes to each directory entry, but only when maxlen is 31 (see adamFuji.cpp).
' This client asks for DIR_MAX_LEN = 36, so entries arrive clean and the type
' is worked out here instead -- from the trailing '/' st_file.bas already
' tests for, plus sf_is_cart below.
' ---------------------------------------------------------------------------
lit_glyphs:
    ' folder: $00,$78,$87,$FF,$FF,$FF,$FF,$00
    BITMAP "........"
    BITMAP ".####..."
    BITMAP "#....###"
    BITMAP "########"
    BITMAP "########"
    BITMAP "########"
    BITMAP "########"
    BITMAP "........"
    ' cartridge: $FF,$81,$BD,$BD,$BD,$81,$FF,$00
    BITMAP "########"
    BITMAP "#......#"
    BITMAP "#.####.#"
    BITMAP "#.####.#"
    BITMAP "#.####.#"
    BITMAP "#......#"
    BITMAP "########"
    BITMAP "........"
    ' a solid block, shared by two callers. input.bas parks it behind the
    ' character grid's selected cell as a MOB: every pixel is "on", so it fills
    ' the whole card except where the glyph's own foreground pixels win.
    ' st_boot.bas draws it into BACKTAB directly for the progress bar.
    BITMAP "########"
    BITMAP "########"
    BITMAP "########"
    BITMAP "########"
    BITMAP "########"
    BITMAP "########"
    BITMAP "########"
    BITMAP "########"

' ---------------------------------------------------------------------------
' cb_define_glyphs: upload both cards to GRAM. Called once from cfg_start;
' DEFINE takes effect on the next frame, hence the WAIT (the manual's rough
' 18-cards-per-frame ceiling is no concern at 2).
' ---------------------------------------------------------------------------
cb_define_glyphs: PROCEDURE
    DEFINE GLYPH_FOLDER, 3, lit_glyphs
    WAIT
END

' ---------------------------------------------------------------------------
' sf_draw_glyph: paint column 0 of row cb_row from the type code in cb_i
' (SC_EDIR's enum: 0 = plain file, 1 = folder, 2 = cartridge). A color stack
' GRAM cell is card*8 + GRAM_SELECT + foreground.
'
' The gutter keeps the content background whether or not its row is selected,
' so the glyph column stays visually still as the cursor moves over it.
' ---------------------------------------------------------------------------
sf_draw_glyph: PROCEDURE
    IF cb_i = 1 THEN
        #BACKTAB(cb_row * SCREEN_COLS) = GLYPH_FOLDER * 8 + GRAM_SELECT + COL_HILIGHT
    ELSEIF cb_i = 2 THEN
        #BACKTAB(cb_row * SCREEN_COLS) = GLYPH_CART * 8 + GRAM_SELECT + COL_NORMAL
    ELSE
        #BACKTAB(cb_row * SCREEN_COLS) = CS_BLACK    ' blank, colour irrelevant
    END IF
END

' ---------------------------------------------------------------------------
' sf_is_cart: sets fc_c = 1 iff the entry in FN_RX (length fn_len, as the
' caller has just computed) ends in ".rom" or ".bin", case-insensitively.
' Same shape and same reason as st_file.bas's sf_is_cfg -- IntyBASIC
' PROCEDUREs take no arguments, hence the sf_lowerN helpers it shares.
' ---------------------------------------------------------------------------
sf_is_cart: PROCEDURE
    fc_c = 0
    IF fn_len >= 4 THEN
        sf_ch1 = PEEK(FN_RX + fn_len - 4) AND 255 : GOSUB sf_lower1
        sf_ch2 = PEEK(FN_RX + fn_len - 3) AND 255 : GOSUB sf_lower2
        sf_ch3 = PEEK(FN_RX + fn_len - 2) AND 255 : GOSUB sf_lower3
        sf_ch4 = PEEK(FN_RX + fn_len - 1) AND 255 : GOSUB sf_lower4
        IF sf_ch1 = 46 THEN
            ' ".rom"
            IF sf_ch2 = 114 AND sf_ch3 = 111 AND sf_ch4 = 109 THEN fc_c = 1
            ' ".bin"
            IF sf_ch2 = 98 AND sf_ch3 = 105 AND sf_ch4 = 110 THEN fc_c = 1
        END IF
    END IF
END

' ---------------------------------------------------------------------------
' sf_apply_stack: stamp all four color stack advance bits, for a screen that
' has just been drawn from scratch. Safe to run as one pass only because
' sf_display's scr_clear zeroed every word first -- PRINT and scr_puts both
' write a bare card*8+colour and would otherwise have wiped these.
'
'   A1 (1, 0)       green   -> content
'   A2 (SEL, 1)     content -> selected
'   A3 (SEL+1, 0)   selected -> content
'   A4 (11, 0)      content -> green
'
' SEL is the selected screen row, 1..9, so A3 lands on rows 2..10 and always
' has a real content row under it. scr_recolor and scr_hilite_digits mask with
' AND $FFF8 and leave bit 13 alone, so re-highlighting never disturbs these.
'
' All four bits go down even when the listing is empty (a mount/dir error, or
' a filter that matched nothing). The count matters as much as the positions:
' skipping the bar's two advances would leave row 11 sitting on p2 -- cyan --
' instead of wrapping onto p0, turning the command bar the wrong colour. With
' nothing to highlight the pair is parked on the last two cells of the blank
' spacer row, which costs one cyan cell at (10,18) and hides the other on
' col 19, already content-coloured.
' ---------------------------------------------------------------------------
sf_apply_stack: PROCEDURE
    #BACKTAB(FILES_START_ROW * SCREEN_COLS) = #BACKTAB(FILES_START_ROW * SCREEN_COLS) OR CS_ADVANCE
    IF num_rows > 0 THEN
        GOSUB sf_bar_set
    ELSE
        #BACKTAB(FILES_SPACER_ROW * SCREEN_COLS + 18) = #BACKTAB(FILES_SPACER_ROW * SCREEN_COLS + 18) OR CS_ADVANCE
        #BACKTAB(FILES_SPACER_ROW * SCREEN_COLS + 19) = #BACKTAB(FILES_SPACER_ROW * SCREEN_COLS + 19) OR CS_ADVANCE
    END IF
    #BACKTAB(11 * SCREEN_COLS) = #BACKTAB(11 * SCREEN_COLS) OR CS_ADVANCE
END

' ---------------------------------------------------------------------------
' sf_bar_set / sf_bar_clr: add or remove the selection bar's two advance bits
' for the row currently in sel_row. The glyph and text underneath are
' untouched -- only bit 13 moves -- so the cursor can be walked without
' redrawing anything.
' ---------------------------------------------------------------------------
sf_bar_set: PROCEDURE
    cb_row = FILES_START_ROW + sel_row
    #BACKTAB(cb_row * SCREEN_COLS + 1) = #BACKTAB(cb_row * SCREEN_COLS + 1) OR CS_ADVANCE
    #BACKTAB((cb_row + 1) * SCREEN_COLS) = #BACKTAB((cb_row + 1) * SCREEN_COLS) OR CS_ADVANCE
END

sf_bar_clr: PROCEDURE
    cb_row = FILES_START_ROW + sel_row
    #BACKTAB(cb_row * SCREEN_COLS + 1) = #BACKTAB(cb_row * SCREEN_COLS + 1) AND $DFFF
    #BACKTAB((cb_row + 1) * SCREEN_COLS) = #BACKTAB((cb_row + 1) * SCREEN_COLS) AND $DFFF
END

' ---------------------------------------------------------------------------
' sf_bar: re-arm A4 after anything has printed over row 11. Every message on
' that row starts at column 0 and so wipes the advance bit that makes the
' command bar green -- without this the whole bar drops to the content colour.
' Called at each print site rather than once per frame, so that a bar shown
' during a blocking operation (COPYING..., the ws_pause on an error) is right
' for the whole time it is up, not just once the input loop resumes.
' ---------------------------------------------------------------------------
sf_bar: PROCEDURE
    #BACKTAB(11 * SCREEN_COLS) = #BACKTAB(11 * SCREEN_COLS) OR CS_ADVANCE
END
