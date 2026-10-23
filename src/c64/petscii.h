#ifndef C64_PETSCII_H
#define C64_PETSCII_H

/**
 * FujiNet speaks ASCII; the C64 screen and keyboard use PETSCII.
 */
unsigned char ascii_to_petscii(unsigned char c);
unsigned char petscii_to_ascii(unsigned char c);

#endif /* C64_PETSCII_H */
