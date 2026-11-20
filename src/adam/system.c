#include "../system.h"
#include "../globals.h"
#include <eos.h>
#include <string.h>

// eos.h: strip the high bit of an EOS result; 0 is success
#define EOS_OK(r) (((r) & 0x7F) == EOS_ERR_NONE)

char response[1024]; // one EOS block; system_build_directory writes whole blocks from it

void system_boot(void)
{
  eos_init();
}

bool system_create_new(unsigned char selected_host_slot,unsigned char selected_device_slot,unsigned long selected_size,char *path)
{
  NewDisk nd;

  memset(&nd, 0, sizeof(nd));
  nd.hostSlot = selected_host_slot;
  nd.deviceSlot = selected_device_slot;
  nd.numBlocks = selected_size;
  strcpy(nd.filename, path);

  return fuji_create_new(&nd);
}

// EOS 5 directory entry: 26 bytes, little-endian fields
enum { ENT_SIZE = 26, ENT_ATTR = 12, ENT_START = 13, ENT_ALLOC = 17, ENT_USED = 19, ENT_LAST = 21, ENT_DATE = 23 };

// 12-byte name field: up to 11 chars, ETX, blank padded
static void put_name(unsigned char *e, const char *s)
{
  unsigned char i = 0;

  while (i < 11 && s[i])
    {
      e[i] = s[i];
      i++;
    }
  e[i++] = 0x03;
  while (i < 12)
    e[i++] = ' ';
}

static void put_word(unsigned char *e, unsigned int w)
{
  e[0] = (unsigned char)w;
  e[1] = (unsigned char)(w >> 8);
}

static unsigned char *put_entry(unsigned char *e, const char *name, unsigned char attr, unsigned int start, unsigned int alloc, unsigned int used)
{
  put_name(e, name);
  e[ENT_ATTR] = attr;
  put_word(e + ENT_START, start);
  put_word(e + ENT_ALLOC, alloc);
  put_word(e + ENT_USED, used);
  return e + ENT_SIZE;
}

// Lay out an empty EOS volume the way eosfs does, instead of trusting the ROM's directory init
bool system_build_directory(unsigned char ds, unsigned long numBlocks, char *v)
{
  unsigned int nb = (unsigned short)numBlocks;
  unsigned char db, b;
  unsigned char *e = (unsigned char *)response;

  // EOS only finds devices at boot; rescan so it sees the disk just mounted
  eos_scan_for_devices();

  // Adjust device slot to EOS device #
  ds += 4;

  if (nb > 719)
    db = 6;
  else if (nb > 319)
    db = 3;
  else if (nb > 160)
    db = 2;
  else
    db = 1;

  memset(response, 0, sizeof(response));

  // Directory blocks after the first stay empty
  for (b = 2; b <= db; b++)
    {
      if (!EOS_OK(eos_write_block(ds, b, response)))
        return false;
    }

  // Block 0: DI, JP SmartWriter
  response[0] = 0xF3;
  response[1] = 0xC3;
  response[2] = 0xE7;
  response[3] = 0xFC;
  if (!EOS_OK(eos_write_block(ds, 0UL, response)))
    return false;
  memset(response, 0, 4);

  // VOLUME, with the EOS check code in its start field
  put_name(e, v);
  e[ENT_ATTR] = 0x80 | db;
  e[ENT_START] = 0x55;
  e[ENT_START + 1] = 0xAA;
  e[ENT_START + 3] = 0xFF;
  put_word(e + ENT_ALLOC, nb);
  e += ENT_SIZE;

  e = put_entry(e, "BOOT", 0x88, 0, 1, 1);
  e = put_entry(e, "DIRECTORY", 0xC8, 1, db, db);
  put_word(e - ENT_SIZE + ENT_LAST, 1024);

  // BLOCKS LEFT: the free extent after the directory
  put_entry(e, "BLOCKS LEFT", 0x01, db + 1, nb - db - 1, 0);
  e[ENT_DATE] = 0x57;
  e[ENT_DATE + 1] = 0x07;
  e[ENT_DATE + 2] = 0x11;

  return EOS_OK(eos_write_block(ds, 1UL, response));
}
