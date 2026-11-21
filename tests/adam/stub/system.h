#ifndef STUB_SYSTEM_H
#define STUB_SYSTEM_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct
{
  unsigned char hostSlot;
  unsigned char deviceSlot;
  unsigned long numBlocks;
  char filename[256];
} NewDisk;

#define fuji_create_new(nd) true

bool system_build_directory(unsigned char ds, unsigned long numBlocks, char *v);

#endif
