#include "../system.h"
#include "../pause.h"
#include "../globals.h"
#include "../input.h"
#include "strendswith.h"

#define FUJI_IO_REG ((unsigned char *)0xFF41)

/* Big enough to double as system_create_new()'s NewDisk; nothing else is using it then */
char response[sizeof(NewDisk)];

// Sends the "!LDCART!" knock and polls for a One ROM plugin's ack, with the
// same bounded resend-on-timeout dwonewrite.asm's DWWrite uses for its own
// knock (see its DWWACKTIMEOUT/DWWACKRETRIES comment) - this is both how a
// genuinely-connected plugin recovers from an occasionally-missed knock, and
// how a system with no One ROM at all (real FujiNet cart hardware instead)
// gives up cleanly rather than hanging. This is just the trigger: by the
// time the plugin acks, it has already pulled and loaded the cartridge image
// on its own, over its own link to FujiNet - see
// plugins/user/drivewire/src/drivewire_main.c's drivewire_do_load_cart().
// Returns non-zero if the plugin acked, zero on timeout (no One ROM present,
// or the load failed).
static uint8_t onerom_load_cart(void)
{
    // No blank lines from here through onerom_done@ - lwasm starts a new
    // local-label scope at every blank line (comments don't break it), and
    // onerom_knock@ in particular needs to stay reachable across this whole
    // block - see dwonewrite.asm's identical note on its own DWWCODE.
    asm
    {
        pshs    x,y,u,cc
        orcc    #$50            ; mask interrupts - see dwonewrite.asm
        ldd     #8               ; retry budget, matches DWWACKRETRIES
        pshs    d
onerom_knock@
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
        ldu     #$C200
        ldd     #$4000           ; matches DWWACKTIMEOUT
onerom_poll@
        lda     ,u
        cmpa    #$A5
        beq     onerom_acked@
        subd    #1
        bne     onerom_poll@
        puls    d                ; this attempt timed out - consume one retry
        subd    #1
        beq     onerom_giveup@
        pshs    d
        bra     onerom_knock@
onerom_acked@
        leas    2,s              ; drop the retry budget without touching d/b
        ; Safety margin against the (already tiny, ~100x clock ratio) gap
        ; between the plugin's ack and its active-slot switch actually
        ; completing - pure cycle-counting, no interrupt/timer dependency,
        ; since anything that relies on one (e.g. pause()) is no longer safe
        ; to call once this returns: the plugin's own IRQVEC hook lived
        ; inside the ROM image that is being replaced. This function does
        ; not return until it is genuinely safe to jump.
        ldd     #$1000
onerom_margin@
        subd    #1
        bne     onerom_margin@
        ldb     #1
        bra     onerom_done@
onerom_giveup@
        clrb
onerom_done@
        puls    cc,u,y,x
    }
}

void system_enable_user_rom(void)
{
  if (onerom_load_cart())
  {
    // A One ROM answered and already loaded the cartridge image - jump
    // straight into it, mirroring how the real FUJI_IO_REG write below never
    // returns either.
    //
    // Nothing in $0100-$01FF needs touching before this jump. HDBDOS's own
    // init fills this page with RAM-resident vectors (IRQVEC, NMIVEC at
    // $0109, the RVEC0-RVEC5 GET/PUT and disk-command dispatch table at
    // $015E-$016F, among others), all pointing into HDBDOS/Color BASIC ROM
    // outside the $C000-$CFFF range the cart image actually replaces - none
    // of it is corrupted by the cart load itself. Cart games do commonly
    // treat this page as theirs to repurpose: confirmed by disassembly,
    // Pinball's own startup sets the Direct Page register to $01 and copies
    // its own data over part of the RVEC table within its first few
    // instructions. But nothing calls back into that table once the cart
    // has taken over, so this is harmless - confirmed by tracing the actual
    // machine state under XRoar's GDB stub.
    //
    // An earlier version of this fix zeroed the whole page and rebuilt
    // IRQVEC to jump straight to Extended BASIC's $8955 jiffy-tick handler,
    // on the assumption that Pinball's page reuse meant it needed a
    // freshly-reset machine's untouched low RAM. That was actively wrong,
    // and it was the real cause of these games hanging: $010C is the actual
    // 6809 hardware IRQ vector target (the CPU vector table at $FFF8/$FFF9
    // points straight at it, with no ROM dispatcher above it), and its
    // real, as-booted value is JMP $D8AF, deep in HDBDOS's own ROM at
    // $D000-$DFFF - safely outside the $C000-$CFFF the cart replaces, and so
    // never corrupted in the first place. $D8AF reads PIA0's $FF03/$FF02 to
    // clear the pending CB1 interrupt flag before handing off to $8955;
    // jumping straight to $8955 instead skips that read, so the flag never
    // clears and the CPU re-enters the IRQ handler again immediately after
    // every RTI - an interrupt storm that starves the cart of virtually all
    // CPU time.
    //
    // The stack pointer is restored to INISTK (this program's own
    // entry-time value, high in RAM) rather than left wherever this call
    // chain's own C-compiled stack usage happens to have it - same
    // "true jump, not a call" pattern dwload_cmoc.c's dwload_clone() already
    // uses.
    //
    // Interrupts are masked in CC, and also disabled and cleared at their
    // PIA source, before the jump: a real cart autostart hands off to $C000
    // with nothing pending, and a cart is free to re-arm whatever it needs
    // once it is ready. Masking CC alone is not equivalent - a PIA whose
    // CA1/CB1 interrupt-enable bit is still set can still leave an edge
    // latched, which fires the instant the cart (or anything else) clears
    // I/F again. Reading each side's data register clears any already-
    // latched CA1/CB1 flag (standard 6821 behaviour), and clearing bit 0 of
    // CRA/CRB stops a new edge from re-latching one; bit 2 (DDR/data-register
    // select) and the CA2/CB2 control bits are left untouched.
    //
    // NOTE: unlike the CC mask, this is not self-undoing. Pinball's own
    // startup re-enables interrupts with a plain ANDCC #$AF (confirmed by
    // disassembly) and never itself writes to $FF01/$FF03 - so if that
    // 60Hz tick from PIA0's CB1 (traced back to HDBDOS's $D8AF handler) is
    // load-bearing for Pinball once it is running, clearing CRB's enable
    // bit here would leave it CC-unmasked but never actually interrupted.
    // Needs retesting against Pinball as well as Atom.
    asm
    {
        orcc    #$50
        lda     $FF00           ; clear PIA0 CA1 flag
        lda     $FF01           ; PIA0 CRA
        anda    #$FE
        sta     $FF01
        lda     $FF02           ; clear PIA0 CB1 flag
        lda     $FF03           ; PIA0 CRB
        anda    #$FE
        sta     $FF03
        lda     $FF20           ; clear PIA1 CA1 flag
        lda     $FF21           ; PIA1 CRA
        anda    #$FE
        sta     $FF21
        lda     $FF22           ; clear PIA1 CB1 flag
        lda     $FF23           ; PIA1 CRB
        anda    #$FE
        sta     $FF23
INISTK  IMPORT
        lds     INISTK
        jmp     $C000
    }
  }

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

// Called right after a .CCC/.ROM is mounted to device slot 0 from the file
// picker, instead of waiting for the next full reboot to reach system_boot()'s
// own system_slot0_is_rom() check. Confirms with the user, then arms the same
// CART trigger.
bool system_confirm_and_boot_user_rom(const char *filename)
{
  char k;

  printf("\nLOAD AND RUN\n%s\n(Y)ES  (N)O\n", filename);

  do
  {
    k = (char)input();
  } while (k != 'Y' && k != 'y' && k != 'N' && k != 'n');

  if (k == 'N' || k == 'n')
    return false;

  printf("\nBOOTING CART ROM...\n");
  //pause(120);
  system_enable_user_rom();
  return true;
}

void system_boot(void)
{
  static const char msg[] = "\rBOOTING USER ROM...\r";

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

