#ifdef _CMOC_VERSION_

/**
 * Host and device slot lists, shared by the hosts/devices and file screens.
 */

#include "coco_screen.h"
#include "../globals.h"
#include "../constants.h"

static char line[LIST_W];

/* crole is the role of the column-5 mode marker, trole that of the text. */
static void put_row(byte y, char a, char b, char c, byte crole, const char *text, byte trole, byte textx)
{
  byte n = (byte)strlen(text);
  byte x, role;

  memset(line, ' ', LIST_W);
  line[1] = a;
  if (b)
    line[3] = b;
  if (c)
    line[5] = c;
  if (n > LIST_W - textx)
    n = LIST_W - textx;
  memcpy(line + textx, text, n);
  for (x = 0; x < LIST_W; x++)
  {
    role = ROLE_TEXT;
    if (x == 5 && c)
      role = crole;
    else if (x >= textx && x < textx + n)
      role = trole;
    screen_put_role(LIST_X + x, y, (byte)line[x], role);
  }
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
    put_row(HOSTS_Y + i, '1' + i, 0, 0, 0, p[0] ? (char *)p : text_empty, p[0] ? ROLE_TEXT : ROLE_EMPTY, 4);
    p += 32;
  }
}

void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *dslot, const bool *e)
{
  byte i;
  char host;
  char mode;

  /* Shared code redraws after R/W with y = 1, meaning the hosts/devices screen's list. */
  if (y == 1)
    y = DEVICES_Y;

  for (i = 0; i < NUM_DEVICE_SLOTS; i++, dslot++)
  {
    host = dslot->hostSlot == 0xFF ? ' ' : dslot->hostSlot + '1';
    mode = dslot->file[0] ? device_slot_mode(dslot->mode) : ' ';
    put_row(y + i, '0' + i, host, mode, mode == 'W' ? ROLE_WRITE : ROLE_READ,
            dslot->file[0] ? (char *)dslot->file : text_empty, dslot->file[0] ? ROLE_TEXT : ROLE_EMPTY, 7);
  }
}

void screen_hosts_and_devices_clear_host_slot(int i)
{
  put_row((byte)(HOSTS_Y + i), (char)('1' + i), 0, 0, 0, "", ROLE_TEXT, 4);
}

void screen_hosts_and_devices_host_slot_empty(int hs)
{
  put_row((byte)(HOSTS_Y + hs), (char)('1' + hs), 0, 0, 0, text_empty, ROLE_EMPTY, 4);
}

#endif
