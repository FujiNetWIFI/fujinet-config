#include <cmoc.h>
#include <coco.h>

// 0x6600 (giving the buffer 512 bytes of headroom below 0x8000/ROM) is
// the confirmed-safe address on real CoCo hardware. An experiment moving
// this up to 0x6800 -- intended to buy more headroom against
// config.bin's own runtime heap/stack growth (its measured code+data+bss
// footprint ends at 0x63A5, and its own build allows it to use memory up
// to 0x7C00, see LDFLAGS_EXTRA_COCO's --limit=7C00) -- instead hung the
// machine completely (no logo, no CONFIG boot) on real hardware. That
// means something critical (likely HDB-DOS's own reserved control
// structures) sits closer to 0x7E00-0x8000 than assumed, and the
// original 512-byte gap below 0x8000 was load-bearing. Do not move this
// toward 0x8000 again without hardware evidence of what's actually
// reserved up there. The known-but-unfixed issue this was trying to
// address: config.bin's own memory usage can still grow up into the
// tail of this buffer during its startup, corrupting the bottom rows of
// the logo -- see the conversation/commit history for the diagnosis.
#define SCREEN_BUFFER (byte*) 0x6600

#define LOGO_FULL_SIZE 6144
#define LOGO_BOTTOM_TRIMMED_BYTES 320

/* Use the same path as a typed DWLOAD command:
 * - CHARAD points to the argument text buffer
 * - call DWLOAD indirectly through the extender dispatch table entry
 */
#define DRAGON_CHARAD_ADDR ((unsigned int *) 0x00A6)
//#define DRAGON_CMD_BUF ((char *) 0x02DD)
#define DRAGON_NAME_BUF ((char *) 0x01d1)
#define DRAGON_DWLOAD_VECTOR 0xE686

int dwload_clone(const char *filename, uint8_t execute_nonzero);

void DWLOAD(const char *filename)
{
    char *p = DRAGON_NAME_BUF+1;
    const char *s = filename;
    unsigned char len = 0;

    /* Build "FILENAME" followed by CR like a command line argument. */
    //*p++ = '"';
    //*p++;

#if 1    
    while (*s != '\0')
    {
        *p++ = *s++;
        len++;
    }
    //*p++ = '"';
    //*p++ = '\r';
    //*p = '\0';
#endif    
    p= DRAGON_NAME_BUF;
    *p= len;


    //*DRAGON_CHARAD_ADDR = 0x02DD;

    /* Indirect call via extender table: [$E686] contains the dwload entry. */
    asm
    {
        jsr     0xf8e7 // [DRAGON_DWLOAD_VECTOR]
    }
}

extern void zx0_decompress(void);

#define LOGO_DECOMPRESSED_SIZE 4048

/* The ZX0-compressed logo (516 bytes) is kept out of cfgload.bin's own
 * image to keep it off this program's size budget, and instead loaded
 * into LOGO_SCRATCH before cfgload.bin ever runs -- decompressed from
 * there into the screen buffer and then never touched again.
 *
 * Both platforms load it as a plain LOADM-style machine-language binary
 * (lwasm --dragon, embedding the load address in a DECB-format header)
 * to the address it was assembled at:
 * - Dragon: fetched over DriveWire (LOGO.DWL, via dwload_clone(), which
 *   parses that same header itself -- see dwload_cmoc.c).
 * - CoCo: LOADM"LOGO" in autoexec.bas, *before* LOADM"CFGLOAD":EXEC --
 *   real hardware testing found that CoCo's real DriveWire mount request
 *   for LOGO.DWL never reaches the FujiNet server at all (confirmed via
 *   server logs), even though the same fetch works in emulation and the
 *   DW ROM vectors are correct. A prior attempt at reading it from
 *   cfgload.bin itself via disk.c's DSKCON-based file access worked, but
 *   only after a long chain of fixes for disk.c's call chain nesting
 *   deep enough on real hardware to land return addresses on top of the
 *   still-visible splash screen buffer -- loading it via BASIC's own
 *   already-proven LOADM (same mechanism CFGLOAD.BIN/CONFIG.BIN use)
 *   sidesteps all of that, at the cost of a missing/corrupt LOGO.BIN now
 *   being autoexec.bas's problem instead of something draw_logo() can
 *   gracefully skip past.
 *
 * LOGO_SCRATCH just has to *agree* with the org logo_data.asm/
 * logo_data_coco.asm was assembled with, or unpack_zx0() decompresses
 * whatever unrelated bytes happen to already be at the wrong address.
 * Must clear cfgload.bin's own program_end (see the Makefile's cfgload
 * rules for each platform's --org/--limit) -- check this after changing
 * either program's own static footprint. */
#ifdef DRAGON
#define LOGO_SCRATCH ((unsigned char *) 0x2600)
#else
#define LOGO_SCRATCH ((unsigned char *) 0x4400)
#endif

static void unpack_zx0(const unsigned char *src, unsigned char *dst)
{
    /* CMOC uses U as this function's frame pointer (and expects Y preserved
     * too), but zx0_decompress uses U as its write cursor and clobbers Y.
     * Save/restore both around the call so the frame is intact on return. */
    asm
    {
        pshs u,y
        ldx :src
        ldu :dst
        jsr zx0_decompress
        puls y,u
    }
}

#ifndef DRAGON
/// @brief Invokes the CoCo BASIC RUNM command
/// @param filename
void runm(const char *filename)
{
    // This reproduces the state when executing RUNM"FILE in BASIC
    // by filling in the command line buffer and jumping directly
    // to the Basic RUN procedure

    // Set beginning of (compressed) command: M"
    *((uint16_t *)0x2dd) = 0x4D22;

    // Add the filename to the command
    strcpy(0x2df, filename);

    // Set command pointer
    *((uint16_t *)0xa6) = 0x2dd; // set CHARAD

    // Jump to "RUNM" command
    asm
    {
        // Observed this value in actual RUNM. Not working unless 4Dxx is set
        ldd     #$4D1C

        // Jump to RUN procedure
        jmp     $AE75
    }
}
#endif

void draw_logo()
{
#ifdef DRAGON
    // Fetch the still-compressed logo into scratch RAM (load-only, don't
    // execute -- it's data, not code). If this fails, we just skip the
    // logo and continue straight to loading STAGE2.DWL; a missing splash
    // isn't worth aborting the boot over.
    if (dwload_clone("LOGO.DWL", 0) != 0) {
        return;
    }
#endif
    // On CoCo, LOGO.BIN is already loaded into LOGO_SCRATCH by the time
    // this runs -- LOADM"LOGO" in autoexec.bas, before LOADM"CFGLOAD":EXEC.

    // To save space, we trimmed the top of the logo that is 0xff (white) and the bottom.
    // Below calculates where to draw the trimmed logo on screen to preserve the original location
    unpack_zx0(LOGO_SCRATCH, SCREEN_BUFFER+LOGO_FULL_SIZE-LOGO_BOTTOM_TRIMMED_BYTES-LOGO_DECOMPRESSED_SIZE);
}

// Show the fujinet splash screen,
// Then load config.bin
int main(void)
{
    // Reset text mode to default
    width(32);

    // Setup graphics mode and clear screen
    pmode(4, SCREEN_BUFFER);
    pcls(0xff);
    screen(1, 1);

    // Draw the logo
    draw_logo();

#ifdef DRAGON
    // Hand off to the second-stage loader (org=c00, see stage2.c) rather
    // than fetching CONFIG.DWL directly from here: this program is too
    // big (needs pmode/pcls/screen/the ZX0 decompressor) to sit at a low
    // enough org to leave CONFIG.DWL much headroom below the screen
    // buffer without the instability we saw testing low orgs for a
    // program this size. stage2.c has none of that and can safely sit
    // much lower.
    dwload_clone("STAGE2.DWL",1);
#else
    // Load config
    runm("CONFIG");
#endif

   return 0;
}

