#ifdef BUILD_C64

#include <peekpoke.h>

#define JIFFY_LOW 0xA2 // KERNAL clock, 60 Hz on PAL and NTSC

/**
 * Wait delay jiffies (1/60 s)
 */
void pause(unsigned char delay)
{
	unsigned char t;

	while (delay--)
	{
		t = PEEK(JIFFY_LOW);
		while (PEEK(JIFFY_LOW) == t)
			;
	}
}

#endif
