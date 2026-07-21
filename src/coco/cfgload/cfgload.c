#include <cmoc.h>
#include <coco.h>

// Confirmed-safe on real CoCo hardware (512 bytes headroom below 0x8000/ROM).
// Moving this toward 0x8000 hung real hardware in testing -- don't, without
// hardware evidence of what's actually reserved up there.
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

/* The ZX0-compressed logo is loaded into LOGO_SCRATCH before cfgload.bin
 * runs: over DriveWire on Dragon (LOGO.DWL via dwload_clone()), via
 * LOADM"LOGO" in autoexec.bas on CoCo (DriveWire mount for LOGO.DWL never
 * reaches the FujiNet server on real CoCo hardware).
 *
 * Must match the org logo_data.asm/logo_data_coco.asm was assembled with,
 * and clear cfgload.bin's own program_end. */
#ifdef DRAGON
#define LOGO_SCRATCH ((unsigned char *) 0x2600)
#else
#define LOGO_SCRATCH ((unsigned char *) 0x4400)
#endif

static void unpack_zx0(const unsigned char *src, unsigned char *dst)
{
    /* zx0_decompress uses U/Y internally; save/restore CMOC's frame pointer. */
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
    // Load-only fetch; a missing splash isn't worth aborting the boot over.
    if (dwload_clone("LOGO.DWL", 0) != 0) {
        return;
    }
#endif
    // On CoCo, LOGO.BIN is already loaded into LOGO_SCRATCH via autoexec.bas.

    // Top/bottom trimmed to save space; adjust the draw offset to compensate.
    unpack_zx0(LOGO_SCRATCH, SCREEN_BUFFER+LOGO_FULL_SIZE-LOGO_BOTTOM_TRIMMED_BYTES-LOGO_DECOMPRESSED_SIZE);
}

int main(void)
{
    width(32);
    pmode(4, SCREEN_BUFFER);
    pcls(0xff);
    screen(1, 1);

    draw_logo();

#ifdef DRAGON
    // Hand off to the second-stage loader (see stage2.c) instead of
    // fetching CONFIG.DWL directly -- this program is too big to sit at
    // a low enough org to leave CONFIG.DWL headroom.
    dwload_clone("STAGE2.DWL",1);
#else
    runm("CONFIG");
#endif

   return 0;
}

