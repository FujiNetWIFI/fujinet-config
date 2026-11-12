/**
 * @brief Pause
 */

#ifdef _CMOC_VERSION_

#include <cmoc.h>
#include <fujinet-fuji.h>
#include "../pause.h"

void pause(unsigned char delay)
{
	asm
	{
		// fuji_wait_jiffies() counts the 60 Hz interrupt, which may not be on yet
    lda     $FF03 // get control register of PIA0, port B
    ora     #1
    sta     $FF03 // enable 60 Hz interrupt

				// Unmask interrupts to allow the timer IRQ to be processed.
    andcc   #$AF
	}

	fuji_wait_jiffies(delay);
}

#endif /* _CMOC_VERSION_ */
