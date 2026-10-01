#ifdef _CMOC_VERSION_

/**
 * Host and device slot lists, shared by the hosts/devices and file screens.
 */

#include "coco_screen.h"
#include "../globals.h"
#include "../constants.h"

static char line[LIST_W + 1];

static void put_row(byte y, char a, char b, char c, const char *text, byte textx)
{
  byte n = (byte)strlen(text);

  memset(line, ' ', LIST_W);
  line[LIST_W] = 0;
  line[1] = a;
  if (b)
    line[3] = b;
  if (c)
    line[5] = c;
  if (n > LIST_W - textx)
    n = LIST_W - textx;
  memcpy(line + textx, text, n);
  screen_puts(LIST_X, y, line);
}

static char device_slot_mode(unsigned char mode)
{
  switch (mode & ~MODE_MOUNTED)
  {
  case MODE_READ:
    return 'R';
  case MODE_WRITE:
    return 'W';
  }
  return ' ';
}

void screen_hosts_and_devices_host_slots(HostSlot *h)
{
  byte *p = &h[0];
  byte i;

  for (i = 0; i < NUM_HOST_SLOTS; i++)
  {
    put_row(HOSTS_Y + i, '1' + i, 0, 0, p[0] ? (char *)p : text_empty, 4);
    p += 32;
  }
}

void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *dslot, const bool *e)
{
  byte i;
  char host;
  char mode;

  for (i = 0; i < NUM_DEVICE_SLOTS; i++, dslot++)
  {
    host = dslot->hostSlot == 0xFF ? ' ' : dslot->hostSlot + '1';
    mode = dslot->file[0] ? device_slot_mode(dslot->mode) : ' ';
    put_row(y + i, '0' + i, host, mode, dslot->file[0] ? (char *)dslot->file : text_empty, 7);
  }
}

void screen_hosts_and_devices_clear_host_slot(int i)
{
  put_row((byte)(HOSTS_Y + i), (char)('1' + i), 0, 0, "", 4);
}

void screen_hosts_and_devices_host_slot_empty(int hs)
{
  put_row((byte)(HOSTS_Y + hs), (char)('1' + hs), 0, 0, text_empty, 4);
}

#endif
