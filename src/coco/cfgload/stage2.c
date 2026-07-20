#include <cmoc.h>

/* Second-stage loader: fetches and executes CONFIG.DWL. Fetched and
 * executed by cfgload.bin once the splash is drawn -- split out so it
 * can sit at a much lower org than cfgload.bin, leaving CONFIG.DWL more
 * headroom (see the Makefile). */

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
