/**
 * Perform Copy operation
 */

#include "perform_copy.h"
#include "typedefs.h"
#include "compat.h"
#include "screen.h"
#include "globals.h"

extern char source_filename[128];

unsigned char copy_host_slot;
char copySpec[256];

extern bool copy_mode;

void perform_copy(void)
{
  /*
   * source_filename can be a full relative path (e.g. when the source
   * was picked from a recursive filter search like "!Carnival"). The
   * destination is the directory the user navigated to (path) plus only
   * the basename of the source, so strip any leading directory portion.
   */
  char *base = strrchr(source_filename, '/');
  if (base != NULL)
    base++;
  else
    base = source_filename;

  strcpy(copySpec, source_path);
  strcat(copySpec, "|");
  strcat(copySpec, path);
  strcat(copySpec, base);

  screen_perform_copy((char *)hostSlots[copy_host_slot],(char *)source_path,(char *)hostSlots[selected_host_slot],(char *)path);

  /**
   * NOTE: On the Fuji, command 0xD8 (copy file) expects the slots to
   * be 1-8, not 0-7 like most things.  That's why we add one, since
   * config is tracking the slots as 0-7
   */
  fuji_copy_file(copy_host_slot + 1, selected_host_slot + 1, copySpec);

  copy_mode = false;
  
  state=SELECT_FILE;
  backFromCopy = true;
}
