#include "../system.h"
#include "../pause.h"
#include "../globals.h"
#include "../input.h"

// "!LDCART!" reports a 5-byte status/length/checksum block at
// DW_CART_STATUS_ADDR and the four addresses right after it (see
// drivewire_main.c), instead of a bare ack byte - this lets
// system_enable_user_rom() independently recompute the same checksum by
// walking the activated cart's own memory afterward, and only jump if it
// agrees with what the plugin reports it received.
static uint8_t g_cart_status;
static uint8_t g_cart_len_lo, g_cart_len_hi;
static uint8_t g_cart_cksum_lo, g_cart_cksum_hi;

// LDA $FF02 (clears PIA0's latched CB1 flag, the real 60Hz tick ack);
// RTI. See system_enable_user_rom()'s jump sequence - this is what $010C
// gets repointed at before every jump.
static const uint8_t g_irq_tick_stub[4] = { 0xB6, 0xFF, 0x02, 0x3B };

// Hand-assembled (not a CMOC function - entered via an indirect call
// through $A002, so prologue/epilogue must be exactly what an external
// caller expects: character in A, A/X/Y preserved, plain RTS). Cursor
// counter lives in its own last 2 bytes, PC-relative addressed so the
// routine works wherever this array ends up linked. lwasm source kept
// in the scratchpad this was assembled from, not duplicated here.
static const uint8_t g_putchr_stub[42] = {
    0x34, 0x32, 0xae, 0x8d, 0x00, 0x22, 0x81, 0x0d, 0x27, 0x0c, 0x1f, 0x12,
    0x31, 0xa9, 0x04, 0x00, 0xa7, 0xa4, 0x30, 0x01, 0x20, 0x03, 0x30, 0x88,
    0x20, 0x8c, 0x02, 0x00, 0x25, 0x03, 0x8e, 0x00, 0x00, 0xaf, 0x8d, 0x00,
    0x03, 0x35, 0x32, 0x39, 0x00, 0x00
};

// printf()/putchar() on this target (see cmoc-0.1.100/src/stdlib/
// putchar_a.asm) jump through CHROUT, a pointer this build's own crt.asm
// caches once at program startup by reading $A002 - the classic CoCo BASIC
// "current PUTCHR" vector, which HDBDOS points at its own charout code
// inside $C000-$DFFF. The instant the plugin switches the active slot, that
// same address holds the new cart's own data instead of HDBDOS, so
// printf/putchar after a successful load would jump into garbage. Redirect
// both to this hook instead (setConsoleOutHook(), cmoc.h), which only ever
// touches ordinary CoCo screen RAM and never calls through CHROUT at all.
static uint16_t g_console_pos;

static void safe_console_out(void)
{
    // Contract (cmoc.h): character arrives in A; must preserve B, X, Y, U.
    // CR (13) is what putchar_a.asm turns a C '\n' into before calling
    // CHROUT, so that's what we see here too for a newline - not 10.
    asm
    {
        pshs    x,y
        ldx     g_console_pos
        cmpa    #13
        beq     cout_newline@
        tfr     x,y
        leay    $0400,y
        sta     ,y
        leax    1,x
        bra     cout_wrap@
cout_newline@
        leax    32,x
cout_wrap@
        cmpx    #512
        blo     cout_store@
        ldx     #0
cout_store@
        stx     g_console_pos
        puls    x,y
    }
}

// Sends the "!LDCART!" knock once and polls for the plugin's status report -
// not the bounded resend dwonewrite.asm's DWWrite uses, because a resent
// knock here would read the cart's own live memory once the plugin
// switches slots, the same bus-wedging access pattern the jump sequence
// below avoids. One knock, one generous wait, then give up.
// g_cart_status: 0 (success) or 255 (failure) from the plugin; $FE means no
// response at all. g_cart_len_*/g_cart_cksum_* only valid when status is 0.
static uint8_t onerom_load_cart(void)
{
    // No blank lines from here through onerom_done@ - lwasm starts a new
    // local-label scope at every blank line (comments don't break it).
    asm
    {
        pshs    x,y,u,cc
        orcc    #$50            ; mask interrupts - see dwonewrite.asm
#ifdef COCO3
        ; Super Extended BASIC's own CoCo3 cold boot copies the cart's
        ; first 8K into RAM and runs in all-RAM mode (see coco3.asm).
        ; Without switching GIME to ROM mode first, the knock below and
        ; the status poll never reach the One ROM at all - they'd hit
        ; that RAM copy instead and this whole function would be chasing
        ; garbage. $FFDE is the same SAM-compatible strobe CoCo1/2 use
        ; below, right before the jump; GIME needs it this early too,
        ; plus its own INIT0 (write-only, so always a full value, never
        ; read-modify-write - $4C matches text_gime.c's own txt_open(),
        ; MC1:MC0=00 shows the cart at $C000). $FExx still reads as
        ; constant RAM here (MC3 set) - harmless, since the knock/poll
        ; below never touches $FE00-$FEFF; cleared separately before the
        ; checksum walk needs it.
        clr     $FFDE
        lda     #$4C
        sta     $FF90
#endif
        ldu     #$C100
        ldb     #'!
        lda     b,u
        ldb     #'L
        lda     b,u
        ldb     #'D
        lda     b,u
        ldb     #'C
        lda     b,u
        ldb     #'A
        lda     b,u
        ldb     #'R
        lda     b,u
        ldb     #'T
        lda     b,u
        ldb     #'!
        lda     b,u
        ldu     #$C210           ; DW_CART_STATUS_ADDR, mapped into cart space
        ldx     #8               ; outer budget - extends the total wait only, never resends
onerom_poll_outer@
        ldy     #$4000           ; matches DWWACKTIMEOUT - Y, not D: LDA ,U below
                                  ; loads into D's own high byte (A), so counting
                                  ; down D here would overwrite it with whatever
                                  ; was just read and never reliably reach zero.
onerom_poll@
        lda     ,u
        beq     onerom_success@  ; 0 = DW_CART_STATUS_SUCCESS
        cmpa    #$FF
        beq     onerom_failed@   ; 255 = DW_CART_STATUS_FAILURE
        leay    -1,y             ; LEAY affects Z on the 6809, so this BNE is valid
        bne     onerom_poll@
        leax    -1,x
        bne     onerom_poll_outer@
        bra     onerom_giveup@
onerom_success@
        ldu     #$C211
        lda     ,u
        sta     g_cart_len_lo
        ldu     #$C212
        lda     ,u
        sta     g_cart_len_hi
        ldu     #$C213
        lda     ,u
        sta     g_cart_cksum_lo
        ldu     #$C214
        lda     ,u
        sta     g_cart_cksum_hi
        clra
        sta     g_cart_status
        ; The plugin just switched slots, so HDBDOS's IRQ handler at $D8AF
        ; (what $010C points at) is now cart data. PIA hardware latches the
        ; 60Hz edge regardless of the CC mask, so this puls's own
        ; re-enable would fire straight into it. Disable (not just clear)
        ; both PIAs' CA1/CB1 sources instead - pause()/getTimer() stop
        ; working from here on; the caller must not rely on them.
        lda     $FF00
        lda     $FF01
        anda    #$FE
        sta     $FF01
        lda     $FF02
        lda     $FF03
        anda    #$FE
        sta     $FF03
        lda     $FF20
        lda     $FF21
        anda    #$FE
        sta     $FF21
        lda     $FF22
        lda     $FF23
        anda    #$FE
        sta     $FF23
        ; Margin against the gap between the plugin's status write and its
        ; active-slot switch completing - the checksum walk below reads
        ; $C000 as the activated cart and must not start first. Not a
        ; measured bound, just generous; cheap regardless given this path
        ; already tolerates multi-second pause()s elsewhere.
        ldd     #$A000
onerom_margin@
        subd    #1
        bne     onerom_margin@
        ldb     #1
        bra     onerom_done@
onerom_failed@
        lda     #$FF
        sta     g_cart_status
        clrb
        bra     onerom_done@
onerom_giveup@
        lda     #$FE             ; no response at all - distinct from an explicit failure
        sta     g_cart_status
        clrb
onerom_done@
        puls    cc,u,y,x
    }
}

void system_enable_user_rom(void)
{
  uint8_t acked = onerom_load_cart();

  // Redirect printf/putchar away from CHROUT before anything else calls
  // either - see safe_console_out()'s own comment. Starting a couple of
  // rows down leaves whatever "BOOTING USR ROM..." already printed via the
  // real CHROUT alone.
  g_console_pos = 64;
  setConsoleOutHook(safe_console_out);

  // pause() depends on the 60Hz jiffy clock, which onerom_load_cart() just
  // permanently disabled on the success path (see its own comment) - safe
  // to call only when it didn't take that path.
  if (!acked)
    pause(60);

  if (acked)
  {
#ifdef COCO3
    // Clear MC3 - with it still set from the $4C written before the knock,
    // $FE00-$FEFF reads as constant RAM, so a 16K image's last page would
    // never be seen here. Set the COCO bit in the same write, since nothing
    // between here and the jump still needs GIME-native addressing; INIT0
    // is write-only, so this is always a full value.
    *(unsigned char *)0xFF90u = 0xC4;
#endif
    // Walk the activated cart's own memory ($C000 onward - see
    // drivewire_do_load_cart() in drivewire_main.c) and recompute the same
    // plain-sum checksum the plugin reports it received, both capped at
    // the real $FF00 I/O boundary - see readable_len below. Only jump if
    // they agree; a loaded-but-unverified image is not worth jumping into
    // blind.
    uint16_t reported_len = (uint16_t)g_cart_len_lo | ((uint16_t)g_cart_len_hi << 8);
    uint16_t reported_cksum = (uint16_t)g_cart_cksum_lo | ((uint16_t)g_cart_cksum_hi << 8);
    uint16_t calc_cksum = 0;
    uint16_t i, outer, inner;
    // $FF00-$FFFF is always real hardware I/O (PIA/SAM) on every CoCo,
    // never cartridge, regardless of image size - cap the walk at that
    // absolute boundary (not a flat "len-256", which would wrongly
    // truncate an image nowhere near it) so this is comparable to the
    // plugin's/ESP32's own checksums, which are capped the same way.
    uint16_t readable_len = (reported_len < 0x3F00u) ? reported_len : 0x3F00u;
    for (i = 0; i < readable_len; i++)
      calc_cksum = (uint16_t)(calc_cksum + *(unsigned char *)(0xC000u + i));

    if (calc_cksum != reported_cksum)
    {
      printf("\nCKSUM MISMATCH - NOT JUMPING\nLEN=%u\nOURS=%04x PLUGIN=%04x\n",
             reported_len, calc_cksum, reported_cksum);
      // Plain busy-wait, not pause(): onerom_load_cart()'s own success
      // path already disabled both PIAs' interrupt sources (see its own
      // comment), so the jiffy clock pause() needs is gone regardless of
      // which branch this is. Long enough to read/note the values above
      // before returning to system_boot(), which cold-starts and wipes
      // the screen.
      for (outer = 0; outer < 600; outer++)
        for (inner = 0; inner < 2000; inner++) { }
      return;
    }

#ifdef COCO3
    // Re-set MC3, now that the checksum walk above is done needing it
    // clear. With MC3 clear, $FE00-$FEFF shows the cart's own bytes
    // instead of the RAM page holding the hardware vectors' LBRA
    // redirect into $0100-page software vectors (coco3.asm's INTIMAGE,
    // copied to INT.FLAG at $FE00-page boot) - the first interrupt after
    // the jump would branch into cart data instead of our own $010C
    // stub. $CC matches what a native CoCo 3 writes before starting a
    // cart itself (COCO|MMUEN|MC3|MC2).
    *(unsigned char *)0xFF90u = 0xCC;
#endif
    // A One ROM answered and already loaded the cartridge image - jump
    // straight into it, mirroring how the real FUJI_IO_REG write below never
    // returns either.
    //
    // $0100-$01FF needs no touching: carts commonly repurpose this RAM
    // page themselves (Pinball sets DP=$01 and overwrites part of it),
    // and nothing calls back into HDBDOS's own vectors there once a cart
    // has taken over. Stack is restored to INISTK (this program's own
    // entry-time value) rather than left wherever C's own usage has it -
    // a true jump, not a call, same as dwload_cmoc.c's dwload_clone().
    //
    // CC is unmasked to match how EXEC &HC000 hands off to the same cart
    // (BASIC runs with interrupts enabled). Only PIA0's CB1 - the 60Hz
    // tick most carts need - is enabled below; PIA0's CA1 (HSync,
    // ~15.7kHz) and PIA1's CA1/CB1 are left as inherited, since enabling
    // a source this code doesn't also ack causes an interrupt storm.
    //
    // $010C, the real IRQ vector, is always repointed at g_irq_tick_stub
    // (acks CB1 the way $D8AF - HDBDOS's own handler, now cart data -
    // would, then RTIs without BASIC's jiffy code). The plugin's cart
    // push mirrors a smaller image across the whole slot, so $D8AF is
    // always cart data regardless of image size.
    {
      unsigned int stub_addr = (unsigned int)g_irq_tick_stub;
      *(unsigned char *)0x010Du = (unsigned char)(stub_addr >> 8);
      *(unsigned char *)0x010Eu = (unsigned char)(stub_addr & 0xFF);
    }

    // $A002 - the classic "current PUTCHR" vector - is the same problem
    // as $010C above: HDBDOS points it at charout code that's now cart
    // data. Our own C code is covered by setConsoleOutHook() instead,
    // but a cart built to run alongside BASIC (EdTASM+) can call through
    // $A002 directly for its own output. Needs to keep working for as
    // long as the cart calls it, so repointed at g_putchr_stub rather
    // than a no-op. Holds a plain 2-byte address (not a JMP opcode).
    {
      unsigned int putchr_addr = (unsigned int)g_putchr_stub;
      *(unsigned char *)0xA002u = (unsigned char)(putchr_addr >> 8);
      *(unsigned char *)0xA003u = (unsigned char)(putchr_addr & 0xFF);
    }

    asm
    {
        orcc    #$50
        ; Only PIA0 CB1's enable bit is forced on - see comment above.
        lda     $FF00           ; clear PIA0 CA1 flag, enable bit untouched
        lda     $FF02           ; clear PIA0 CB1 flag
        lda     $FF03           ; PIA0 CRB
        ora     #$01
        sta     $FF03
        lda     $FF20           ; clear PIA1 CA1 flag, enable bit untouched
        lda     $FF22           ; clear PIA1 CB1 flag, enable bit untouched
        ; HDBDOS commonly runs the SAM in Map Type 1 (all 64K RAM) to get
        ; extra buffer space, which makes $C000 ordinary CoCo RAM instead
        ; of the cartridge - confirmed the hard way: jumping there while
        ; still under HDBDOS hangs into whatever garbage is in RAM, while
        ; a real reset (which resets the SAM to Map Type 0) followed by a
        ; manual EXEC &HC000 boots the same cart image fine. Any access to
        ; $FFDE - not its value - strobes Map Type back to 0 (ROM/cart
        ; visible at $8000-$FEFF), same power-on default a reset leaves it
        ; in.
        clr     $FFDE
#ifdef COCO3
        ; Harmless re-strobe above (already in ROM mode since
        ; onerom_load_cart()'s own entry). Super Extended BASIC's own
        ; GIME video setup and $FExx page are no longer valid for a cart,
        ; same reasoning as the $010C/$A002 redirects above - undo them.
        ; $FF98-$FF9F matches coco3.asm's own IM.TEXT table exactly.
        lda     #$00
        sta     $FF98
        sta     $FF99
        sta     $FF9A
        sta     $FF9B
        lda     #$0F
        sta     $FF9C
        lda     #$E0
        sta     $FF9D
        lda     #$00
        sta     $FF9E
        sta     $FF9F
        clr     $FFD8            ; normal speed - txt_open() (text_gime.c)
                                  ; leaves double speed active, and game
                                  ; timing assumes 0.89 MHz
        clr     $FF91            ; MMU task 0, same as GFX_LEAVE()
#endif
        ; Margin against the jump landing before the companion's serving
        ; path has settled for sustained, full-speed instruction fetches -
        ; a different access pattern than the checksum walk's own slow,
        ; one-byte reads. Cheap insurance.
        ldd     #$0400
onerom_jump_margin@
        subd    #1
        bne     onerom_jump_margin@
INISTK  IMPORT
        lds     INISTK
        ; A real 6809 reset clears DP to $00 (confirmed against XRoar's
        ; mc6809.c; A/B/X/Y/U/S are not reset, so not zeroed here either).
        ; Every CoCo program relies on that convention without setting it
        ; itself; this jump never touched DP before, leaving it at
        ; whatever HDBDOS/this program's own C runtime left it.
        clra
        tfr     a,dp
        andcc   #$AF            ; unmask F and I - see comment above
        jmp     $C000
    }
  }
  // Known limitation: some carts (Demon Attack) still fail intermittently
  // past a clean checksum match - real autostart ties the cart's FIRQ
  // line to the clock, so BASIC jumps to $C000 much earlier in its own
  // boot than this path (jumping from an already-running HDBDOS session)
  // can reproduce. XRoar's own autostart approximation hits the same
  // cart intermittently for the same reason, per its cart.c comment.
  //
  // Known limitation: 32K carts that bank-switch on CoCo3 (Arkanoid) load
  // and checksum clean, then show garbage once running - the cart detects
  // CoCo3 and swaps in a second bank over edge-connector lines the One ROM
  // doesn't carry. A hardware reset into the same cart on the same One ROM
  // hits the identical wall, so this is a companion hardware limit, not
  // something this jump sequence can route around.

  printf("\nFALLBACK IOREG\n");
  pause(60);
  fuji_enable_user_rom();
}
