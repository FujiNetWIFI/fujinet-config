#include <cmoc.h>

/*
 * Second-stage loader: the entire job of this program is to fetch and
 * execute CONFIG.DWL. It is itself fetched and executed by cfgload.bin
 * (org=1a00, the splash/logo loader) once the splash screen is drawn.
 *
 * Splitting this out of cfgload.bin exists purely to shrink the memory
 * footprint that's "live" (and therefore off-limits to CONFIG.DWL's own
 * incoming bytes) at the moment CONFIG.DWL is actually being streamed
 * in. cfgload.bin needs pmode/pcls/screen/the ZX0 decompressor to draw
 * the splash, which makes it too big to sit at a low org reliably (org
 * values below e00 were unstable on real hardware for a program that
 * size). This program does none of that -- it only needs the DriveWire
 * protocol code in dwload_cmoc.c -- so it can sit much lower (org=c00,
 * see the Makefile), leaving CONFIG.DWL far more low-memory headroom
 * below the c00-loaded program than it would have below a c00-loaded
 * cfgload.bin, which is what caused the instability before.
 */

int dwload_clone(const char *filename, unsigned char execute_nonzero);

int main(void)
{
#ifdef DRAGON
    dwload_clone("CONFIG.DWL", 1);
#endif

    // If we get here, the load/execute above failed -- freeze rather
    // than fall through to garbage.
    while (1) { }
    return 0;
}
