#if defined(BUILD_COCO) || defined(BUILD_DRAGON)
#include "system.h"
#include "globals.h"

bool path_is_rom_file(const char *name)
{
  size_t len = strlen(name);

  if (len < 4)
    return false;
  name += len - 4;
  return stricmp(name, ".CCC") == 0 || stricmp(name, ".ROM") == 0;
}

bool system_slot0_is_rom(void)
{
  return fuji_get_device_filename(0, response) && path_is_rom_file(response);
}
#endif /* BUILD_COCO || BUILD_DRAGON */
