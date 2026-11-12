/**
 * @brief Pause
 */

#ifdef _CMOC_VERSION_

#include <cmoc.h>
#include <fujinet-fuji.h>
/* Not ../pause.h: there, fuji_sleep_ms() is this function */

void dragon_sleep_ms(unsigned ms)
{
	asm
	{
    lda     $FF03 // get control register of PIA0, port B
    ora     #1
    sta     $FF03 // enable 60 Hz interrupt

				// Unmask interrupts to allow the timer IRQ to be processed.
    andcc   #$AF
	}

	fuji_sleep_ms(ms);
}

#endif /* _CMOC_VERSION_ */
