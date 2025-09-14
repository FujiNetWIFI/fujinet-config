/**
 * @brief Pause
 */

#ifdef _CMOC_VERSION_

#include <cmoc.h>
#include <coco.h>

void pause(unsigned char delay)
{
#ifdef DRAGON  
asm
{
    // We come here from a JMP $C000 in Color Basic (normally at
    // $A10A in v1.2). At this point, the 60 Hz interrupt has
    // not been enabled yet, so enable it.
    lda     $FF03   // get control register of PIA0, port B
    ora     #1
    sta     $FF03   // enable 60 Hz interrupt

    // Unmask interrupts to allow the timer IRQ to be processed.
    andcc   #$AF
}
#endif
  sleep(delay/20);
}

#endif /* _CMOC_VERSION */
