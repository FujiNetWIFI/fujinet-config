#ifdef BUILD_C64
/**
 * ASCII <-> PETSCII for text exchanged with FujiNet
 *
 * The constants are numeric because cc65 translates character
 * literals to PETSCII too.
 */

#include "petscii.h"

/**
 * Translate as cc65 does for string literals, so names from FujiNet look
 * like CONFIG's own text.
 */
unsigned char ascii_to_petscii(unsigned char c)
{
	if (c >= 0x41 && c <= 0x5A) // A-Z
		return c | 0x80;
	if (c >= 0x61 && c <= 0x7A) // a-z
		return c & 0xDF;

	switch (c)
	{
	case 0x5C: // backslash
		return 0xBF;
	case 0x5F: // underscore
		return 0xA4;
	case 0x60: // backtick
		return 0xAD;
	case 0x7B: // {
		return 0xB3;
	case 0x7C: // |
		return 0xDD;
	case 0x7D: // }
		return 0xAB;
	case 0x7E: // ~
		return 0xB1;
	default:
		return c;
	}
}

/**
 * Translate a typed letter
 */
unsigned char petscii_to_ascii(unsigned char c)
{
	if (c >= 0x41 && c <= 0x5A) // unshifted
		return c | 0x20;
	if (c >= 0xC1 && c <= 0xDA) // shifted
		return c & 0x7F;
	return c;
}

#endif /* BUILD_C64 */
