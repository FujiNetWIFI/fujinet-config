#ifdef BUILD_TRS80M1

/**
 * @file kbd.c
 * @brief Keyboard - TRS-80 Model I.
 *
 * The ROM reports a key for as long as it is down, so a key is taken once
 * and then ignored until it has been let go.
 */

#include "key_codes.h"
#include "con.h"
#include "vid.h"
#include "kbd.h"

uint8_t kbd_get(void)
{
  static uint8_t key, last;

  key = con_key();
  if (key == last)
    return 0;
  last = key;
  if (key == KEY_CLEAR)
    return KEY_ESCAPE;
  return key;
}

uint8_t kbd_wait(void)
{
  static uint8_t key;

  vid_flush();
  while (!(key = kbd_get()))
    ;
  return key;
}

#endif /* BUILD_TRS80M1 */
