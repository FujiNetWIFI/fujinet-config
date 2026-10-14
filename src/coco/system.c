#include "../system.h"
#include "../pause.h"
#include "../globals.h"
#include "../input.h"
#include "strendswith.h"

#define FUJI_IO_REG ((unsigned char *)0xFF41)

/* Big enough to double as system_create_new()'s NewDisk; nothing else is using it then */
char response[sizeof(NewDisk)];

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
// gets repointed at for a cart big enough to have overwritten $D8AF.
static const uint8_t g_irq_tick_stub[4] = { 0xB6, 0xFF, 0x02, 0x3B };

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

// Sends the "!LDCART!" knock exactly once and polls for the plugin's status
// report - deliberately NOT the bounded resend-on-timeout
// dwonewrite.asm's DWWrite uses for its own knock (DWWACKTIMEOUT/
// DWWACKRETRIES). That pattern is safe for an ordinary write knock because
// the companion is never anywhere else it could be in the meantime, but
// "!LDCART!" is different: on success, the plugin switches the active slot
// and permanently stops its own knock-matching loop (see
// drivewire_do_load_cart() in drivewire_main.c) - so if this side's poll
// window happens to miss the status write and resends the knock, every one
// of those resend reads lands on the cart's own live memory instead of
// anything listening for a knock. That is exactly the access pattern the
// big comment in system_enable_user_rom() already documents as wedging the
// bus outright on real hardware (confirmed there for the CPU's own
// instruction fetches after a successful load; a resent knock's reads are
// the same kind of unpaced, full-speed access against the same just-
// activated memory). So: one knock, one generous wait, and if nothing
// comes back, give up rather than ever touching $C1xx/$C2xx again.
// g_cart_status on return: 0 (DW_CART_STATUS_SUCCESS) or 255
// (DW_CART_STATUS_FAILURE) came from the plugin; $FE means this function
// gave up without ever seeing either (no response at all - most likely no
// One ROM present). g_cart_len_*/g_cart_cksum_* are only meaningful when
// g_cart_status is 0. Returns non-zero only in that case.
static uint8_t onerom_load_cart(void)
{
    // No blank lines from here through onerom_done@ - lwasm starts a new
    // local-label scope at every blank line (comments don't break it).
    asm
    {
        pshs    x,y,u,cc
        orcc    #$50            ; mask interrupts - see dwonewrite.asm
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
        ldd     #$4000           ; matches DWWACKTIMEOUT
onerom_poll@
        lda     ,u
        beq     onerom_success@  ; 0 = DW_CART_STATUS_SUCCESS
        cmpa    #$FF
        beq     onerom_failed@   ; 255 = DW_CART_STATUS_FAILURE
        subd    #1
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
        ; The plugin has just switched the active slot, which means HDBDOS
        ; - including its own IRQ handler at $D8AF, which the real 6809 IRQ
        ; vector ($010C) points straight at - is already gone, replaced by
        ; the cart image. PIA hardware keeps latching the 60Hz edge that
        ; drives it regardless of the CC mask above, so the instant this
        ; function's own puls below re-enables interrupts, a pending edge
        ; fires immediately, straight into whatever garbage the cart now
        ; has at $D8AF - before this function can even reach its own RTS.
        ; Disabling (not just clearing) both PIAs' CA1/CB1 sources here -
        ; same technique as the jump sequence below already uses, just
        ; needed here too now that execution can return to ordinary CMOC
        ; code instead of always jumping away immediately - means neither
        ; of them can ever latch another edge, so nothing after this point
        ; can try to call through $D8AF again. pause()/getTimer() stop
        ; working from here on, same trade the jump sequence always made;
        ; the caller must not rely on them once this returns successfully.
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
        ; Safety margin against the (already tiny, ~100x clock ratio) gap
        ; between the plugin's status write and its active-slot switch
        ; actually completing - needed here because the checksum walk in
        ; system_enable_user_rom() reads $C000 as the activated cart, not
        ; slot 0, and must not start before the switch is genuinely done.
        ldd     #$1000
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
      printf("\nCKSUM MISMATCH - NOT JUMPING\n");
      // Plain busy-wait, not pause(): onerom_load_cart()'s own success
      // path already disabled both PIAs' interrupt sources (see its own
      // comment), so the jiffy clock pause() needs is gone regardless of
      // which branch this is. Just long enough to read the message before
      // returning to system_boot(), which cold-starts and wipes the screen.
      for (outer = 0; outer < 200; outer++)
        for (inner = 0; inner < 2000; inner++) { }
      return;
    }

    // A One ROM answered and already loaded the cartridge image - jump
    // straight into it, mirroring how the real FUJI_IO_REG write below never
    // returns either.
    //
    // Nothing in $0100-$01FF needs touching before this jump except the IRQ
    // vector handling below. HDBDOS's own init fills this page with
    // RAM-resident vectors (IRQVEC, NMIVEC at $0109, the RVEC0-RVEC5
    // GET/PUT and disk-command dispatch table at $015E-$016F, among
    // others), all pointing into HDBDOS/Color BASIC ROM outside the
    // $C000-$CFFF range a small cart image replaces. Cart games do commonly
    // treat this page as theirs to repurpose regardless - Pinball's own
    // startup sets the Direct Page register to $01 and copies its own data
    // over part of the RVEC table within its first few instructions - but
    // nothing calls back into that table once the cart has taken over, so
    // this is harmless (confirmed by tracing the actual machine state under
    // XRoar's GDB stub).
    //
    // The stack pointer is restored to INISTK (this program's own
    // entry-time value, high in RAM) rather than left wherever this call
    // chain's own C-compiled stack usage happens to have it - same
    // "true jump, not a call" pattern dwload_cmoc.c's dwload_clone() already
    // uses.
    //
    // CC is unmasked (not left masked) right before the jump, matching how
    // EXEC &HC000 hands off to the same cart - BASIC runs with interrupts
    // enabled, so a cart that assumes it's entered that way, rather than
    // issuing its own CLI, otherwise never sees its 60Hz tick. Only PIA0's
    // CB1 - the actual tick source Atom needs - is enabled below; PIA0's
    // CA1 (HSync, ~15.7kHz) and PIA1's CA1/CB1 are left exactly as
    // inherited, since enabling a source this code doesn't also ack in
    // g_irq_tick_stub below causes an interrupt storm bad enough that the
    // cart never even reaches its own splash screen. Every side still gets
    // its data register read once first, to clear whatever latched edge
    // this plugin's own presence on the ROM bus may have left pending,
    // without forcing an enable bit this code doesn't also own.
    //
    // $010C is the real 6809 hardware IRQ vector target (the CPU vector
    // table at $FFF8/$FFF9 points straight at it, no ROM dispatcher above
    // it), and its as-booted value is JMP $D8AF, deep in HDBDOS's own ROM.
    // For a cart small enough to leave $D8AF unwritten (Pinball), that's
    // still the real HDBDOS handler and works correctly as-is - it reads
    // PIA0's $FF02 to ack CB1 before handing off to Extended BASIC's jiffy
    // update at $8955; skipping that ack and jumping straight to $8955 is
    // its own interrupt-storm trap. A cart big enough to reach $010C's
    // target offset ($18AF bytes in - Atom's 16K image included) overwrites
    // $D8AF with cart data instead, so $010C needs repointing at
    // g_irq_tick_stub, which does the same CB1 ack $D8AF would, then RTIs
    // without forwarding to BASIC's jiffy code, which a cart doesn't need.
    // Repointing the JMP's target rather than overwriting its opcode keeps
    // the 3-byte vector slot itself untouched either way.
    if (reported_len > 0x18AFu)
    {
      unsigned int stub_addr = (unsigned int)g_irq_tick_stub;
      *(unsigned char *)0x010Du = (unsigned char)(stub_addr >> 8);
      *(unsigned char *)0x010Eu = (unsigned char)(stub_addr & 0xFF);
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
INISTK  IMPORT
        lds     INISTK
        andcc   #$AF            ; unmask F and I - see comment above
        jmp     $C000
    }
  }

  printf("\nFALLBACK IOREG\n");
  pause(60);
  *FUJI_IO_REG = 7;
}

bool system_slot0_is_rom(void)
{
  char *name;

  if (!fuji_get_device_filename(0, response))
    return false;
  name = strupr(response);
  return strendswith(name, ".CCC") || strendswith(name, ".ROM");
}

void system_boot(void)
{
  static const char msg[] = "\rBOOTING USER ROM...\r";

  // Mount every configured slot now, synchronously - this is also what
  // pushes a cartridge image to a One ROM companion if slot 0 holds one
  // (see fujicmd_mount_all_success() in centi-fuji), so it has to happen
  // before the slot0-is-rom check below can have anything to jump into.
  fuji_mount_all();

  if (system_slot0_is_rom()) {
    // putstr, not printf: keeps the printf library out of apps that never print.
    putstr(msg, sizeof(msg) - 1);
    pause(120);
    system_enable_user_rom();   // write 7 (ROM_MODE | USERROM_ENABLE | AUTOSTART_ENABLE) to IO_CONTROL. Firmware arms the CART trigger and triggers RESET.
  }
  else
  {
    pause(120);
  }

  // On a Rev0000 FujiNet the ROM boot never happens, so cold start to boot normally.
#ifndef DRAGON
  coldStart();
#endif
  exit(0);
}

bool system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                       uint32_t selected_size, const char *path)
{
  NewDisk *nd = (NewDisk *)response;

  memset(nd, 0, sizeof(NewDisk));
  nd->numDisks = (byte)selected_size;
  nd->hostSlot = selected_host_slot;
  nd->deviceSlot = selected_device_slot;
  strcpy(nd->filename, path);

  return fuji_create_new(nd);
}

