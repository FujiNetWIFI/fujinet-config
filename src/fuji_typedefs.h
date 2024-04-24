/**
 * CONFIG custom types
 */

#ifndef FUJI_TYPEDEFS_H
#define FUJI_TYPEDEFS_H

#ifdef USING_FUJINET_LIB
#include "fujinet-fuji.h"
#else
// Source the types in fujinet-fuji.h for platforms that are not yet using fujinet-lib
#include "fuji_typedefs_io.h"
#endif

#define MODE_READ 1
#define MODE_WRITE 2

#define MAX_HOST_LEN 32
#define NUM_HOST_SLOTS 8

typedef enum _entry_types
{
  ENTRY_TYPE_TEXT,
  ENTRY_TYPE_FOLDER,
  ENTRY_TYPE_FILE,
  ENTRY_TYPE_LINK,
  ENTRY_TYPE_MENU
} EntryType;

typedef unsigned short DirectoryPosition;

#endif /* FUJI_TYPEDEFS_H */
