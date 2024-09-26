#ifdef BUILD_APPLE2

#include "sp.h"
#include "screen.h"
#ifdef __ORCAC__
#include <coniogs.h>
#else
#include <conio.h>
#endif

/* Theoretical maximum # of SmartPort devices is 127
 * Call out to each device and if they respond (err = 0),
 * then display it's name
 */
void sp_list_devs(void)
{
  int8_t err, num = 127, i, j;

  clrscr();
  screen_print_inverse(" SMARTPORT DEVICE LIST \r\n\r\n");

  for (i = 1; i < num; i++)
  {
    err = sp_status(i, 0x03);
    if (err == 0){
      cprintf("Unit #%-2d Name: ", i);
      for (j = 0; j < sp_payload[4]; j++)
        cputc(sp_payload[5 + j]);
      cputs("\r\n");
    }
  }
  screen_print_inverse("\r\n Press any key to continue \r\n");
  cgetc();
}

#endif
