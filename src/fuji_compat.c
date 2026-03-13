#if defined(BUILD_APPLE2) || defined(BUILD_ATARI)
#include "typedefs.h"
#include "globals.h"
#include <string.h>

#warning "FIXME - this should be part of fujinet-lib"

bool fuji_open_directory2(uint8_t hs, char *p, char *f)
{
  char *_p = p;
  if (f[0] != 0x00)
  {
    // We have a filter, create a directory+filter string
    memset(response, 0, 256);
    strcpy(response, p);
    strcpy(&response[strlen(response) + 1], f);
    _p = &response[0];
  }
  return fuji_open_directory(hs, _p);
}
#endif /* BUILD_APPLE2 || BUILD_ATARI */
