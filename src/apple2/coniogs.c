#ifdef __ORCAC__

/**
 * FujiNet CONFIG for Apple IIgs
 *
 * cc65 compatibility for ORCA/C
 */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <texttool.h>
#include <coniogs.h>
#include <apple2gs.h>

static char buffer[121];
static unsigned char x;

/* Functions from conio */

void clrscr (void)
/* Clear the whole screen and put the cursor into the top left corner */
{
  WriteChar(0x8c);
}

void gotox (unsigned char x)
/* Set the cursor to the specified X position, leave the Y position untouched */
{
  POKE(0x24, x);
  POKE(0x57b, x);
}

void gotoy (unsigned char y)
/* Set the cursor to the specified Y position, leave the X position untouched */
{
  x = PEEK(0x57b);
  POKE(0x25, y-1);
  WriteChar(0x8d);
  gotox(x);
}

void gotoxy (unsigned char x, unsigned char y)
/* Set the cursor to the specified position */
{
  POKE(0x25, y-1);
  WriteChar(0x8d);
  gotox(x);
}

void cputc (char c)
/* Output one character at the current cursor position */
{
  WriteChar(c);
}

void cputcxy (unsigned char x, unsigned char y, char c)
/* Same as "gotoxy (x, y); cputc (c);" */
{
  gotoxy(x, y);
  WriteChar(c);
}


void cputs (const char* s)
/* Output a NUL-terminated string at the current cursor position */
{
  while (x = *s) {
    if (*s == '\r' && PEEK(0x25) == 23)
    {
      POKE(0x25, 0xff);
      WriteChar(0x8d);
      POKE(0x24, 0);
      POKE(0x57b, 0);
    }
    else
      cputc(*s);
    if (*++s == '\n' && x == '\r')
      s++;
  }
}

void cputsxy (unsigned char x, unsigned char y, const char* s)
/* Same as "gotoxy (x, y); puts (s);" */
{
  gotoxy(x, y);
  cputs(s);
}

int cprintf (const char* format, ...)
/* Like printf(), but uses direct screen output */
{
  va_list args;

  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);
  cputs(buffer);
}

char cgetc (void)
/* Return a character from the keyboard. If there is no character available,
** the function waits until the user does press a key. No cursor displayed.
*/
{
  unsigned char c;

  while ((c = PEEK(0xc000)) < 128); /* >= 128 when key down */
  PEEK(0xc010);   /* Clear keyboard strobe */
  if (PEEK(0xc061) < 128) /* Unless OpenApple is down, clear high bit */
    c &= 0x7F;
  return c;
}

unsigned char revers (unsigned char onoff)
/* Enable/disable reverse character display. This may not be supported by
** the output device. Return the old setting.
*/
{
  if (onoff)
    WriteChar(0x8f);
  else
    WriteChar(0x8e);
}

void chline (unsigned char length)
/* Output a horizontal line with the given length starting at the current
** cursor position.
*/
{
  memset(buffer, '-', length);
  buffer[length] = '\0';
  WriteCString(buffer);
}

void chlinexy (unsigned char x, unsigned char y, unsigned char length)
/* Same as "gotoxy (x, y); chline (length);" */
{
  gotoxy(x, y);
  chline(length);
}

void cclear (unsigned char length)
/* Clear part of a line (write length spaces). */
{
  memset(buffer, 0xa0, length-1);
  buffer[length-1] = '\0';
  WriteCString(buffer);
}

void cclearxy (unsigned char x, unsigned char y, unsigned char length)
/* Same as "gotoxy (x, y); cclear (length);" */
{
  POKE(0x25, y-1);
  WriteChar(0x8d);
  POKE(0x24, x);
  POKE(0x57b, x);
  cclear(length);
}

#endif /* __ORCAC__ */