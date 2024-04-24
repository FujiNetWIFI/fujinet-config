#ifdef BUILD_APPLE2

#include "sp.h"
#include "screen.h"
#include <conio.h>
#include <apple2.h>

/* Theoretical maximum # of SmartPort devices is 127
 * Call out to each device and if they respond (err = 0),
 * then display it's name
 */
void sp_list_devs(void)
{
  int8_t err, num = 127, i, j;

  clrscr();
  revers(1);
  cprintf(" SMARTPORT DEVICE LIST \r\n\r\n");
  revers(0);

  for (i = 1; i < num; i++)
  {
    err = sp_status(i, 0x03);
    if (err == 0){
      cprintf("UNIT #%d NAME: ", i);
      for (j = 0; j < sp_payload[4]; j++)
        cputc(sp_payload[5 + j]);
      cputs("\r\n");
    }
  }
  revers(1);
  cprintf("\r\n PRESS ANY KEY TO CONTINUE \r\n");
  revers(0);
  cgetc();
}

#endif