#ifdef BUILD_APPLE2
#ifdef __ORCAC__

#ifndef CONIOGS_H
#define CONIOGS_H

#include "orcac.h"

#define wherex() PEEK(0x57b)
#define wherey() PEEK(0x25)

void clrscr (void);
void gotox (unsigned char x);
void gotoy (unsigned char y);
void gotoxy (unsigned char x, unsigned char y);
void cputc (char c);
void cputcxy (unsigned char x, unsigned char y, char c);
void cputs (const char* s);
void cputsxy (unsigned char x, unsigned char y, const char* s);
int cprintf (const char* format, ...);
char cgetc (void);
unsigned char revers (unsigned char onoff);
void chline (unsigned char length);
void chlinexy (unsigned char x, unsigned char y, unsigned char length);
void cclear (unsigned char length);
void cclearxy (unsigned char x, unsigned char y, unsigned char length);

#endif /* CONIOGS_H */
#endif /* __ORCAC__ */
#endif /* BUILD_APPLE2 */