#include <string.h>
#include "../mount_and_boot.h"
#include "../system.h"
#include "../screen.h"
#include "../globals.h"
#include "../constants.h"
#include "device_slots.h"

// Override with make LOBBY_URL=<url> msxrom
#ifndef LOBBY_URL
#define LOBBY_URL "TNFS://tnfs.nwah.dev/MSX/Lobby/lobby.rom"
#endif

// The lobby replaces whatever is in the first slot.
#define LOBBY_DEVICE_SLOT 0

// Point a host slot and the lobby slot at url, then mount and boot it.
static void mount_and_boot_url(const char *url)
{
  const char *host = url;
  const char *path = strstr(url, "://");
  uint8_t len, hs, free_hs = FUJI_MAX_HOSTS;

  // A TNFS host slot holds the bare name; other schemes keep theirs.
  if (path)
  {
    path += 3;
    if (path - url == 7 && strncasecmp(url, "tnfs", 4) == 0)
      host = path;
  }
  else
    path = url;

  path = strchr(path, '/');
  if (!path || path - host >= (int)sizeof(HostSlot))
  {
    screen_error("BAD LOBBY URL");
    return;
  }
  len = path - host;

  for (hs = 0; hs < FUJI_MAX_HOSTS; hs++)
  {
    if (strncmp((char *)hostSlots[hs], host, len) == 0 && hostSlots[hs][len] == 0)
      break;
    if (hostSlots[hs][0] == 0 && free_hs == FUJI_MAX_HOSTS)
      free_hs = hs;
  }

  if (hs == FUJI_MAX_HOSTS)
  {
    if (free_hs == FUJI_MAX_HOSTS)
    {
      screen_error("NO FREE HOST SLOT FOR LOBBY");
      return;
    }
    hs = free_hs;
    memset(hostSlots[hs], 0, sizeof(HostSlot));
    memcpy(hostSlots[hs], host, len);
    fuji_put_host_slots(&hostSlots[0], FUJI_MAX_HOSTS);
  }

  // The FujiNet reads a whole MAX_FILENAME_LEN buffer, not just the string.
  strncpy(response, path, MAX_FILENAME_LEN);
  fuji_set_device_filename(FUJI_DISK_MODE_READ, hs, LOBBY_DEVICE_SLOT, response);
  slots_dirty = true;

  mount_and_boot();
}

void mount_and_boot_lobby(void)
{
  if (screen_mount_and_boot_lobby())
    mount_and_boot_url(LOBBY_URL);
}

void mount_and_boot(void)
{
  screen_mount_and_boot();

  if (!fuji_get_device_slots(deviceSlots, FUJI_MAX_DISKS))
  {
    screen_error("ERROR READING DEVICE SLOTS");
    die();
  }

  if (!fuji_get_host_slots(hostSlots, FUJI_MAX_HOSTS))
  {
    screen_error("ERROR READING HOST SLOTS");
    die();
  }

  if (!fuji_mount_all())
  {
    screen_error("ERROR MOUNTING ALL");
    state = HOSTS_AND_DEVICES;
  }
  else
  {
    fuji_set_boot_config(0);
    system_boot();
  }
}
