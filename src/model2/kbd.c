#ifdef BUILD_MODEL2

/**
 * @file kbd.c
 * @brief Keyboard - TRS-80 Model II.
 */

#include "key_codes.h"
#include "con.h"
#include "vid.h"
#include "kbd.h"

uint8_t kbd_get(void)
{
  static int c;

  c = con_in();
  if (c <= 0)
    return 0;
  if (c == KEY_F1)
    return KEY_UP_ARROW;
  if (c == KEY_F2)
    return KEY_DOWN_ARROW;
  return (uint8_t)c;
}

uint8_t kbd_wait(void)
{
  static uint8_t k;

  vid_flush();
  while (!(k = kbd_get()))
    ;
  return k;
}

#endif /* BUILD_MODEL2 */
