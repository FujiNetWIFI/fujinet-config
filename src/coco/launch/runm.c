#include <cmoc.h>

/* RUNM"filename" from a running program: fill BASIC's command buffer and jump into RUN (D must hold $4Dxx). */
void runm(const char *filename)
{
    *((unsigned *)0x2dd) = 0x4D22;
    strcpy((char *)0x2df, filename);
    *((unsigned *)0xa6) = 0x2dd;

    asm
    {
        ldd     #$4D1C
        jmp     $AE75
    }
}
