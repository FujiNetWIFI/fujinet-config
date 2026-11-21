// Runs the real system_build_directory() against a RAM disk and dumps the image.
// usage: eosdir_test <device slot> <blocks> <label> <out file>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eos.h"

#define BLOCK 1024
#define DEV_OFFSET 4 // device slot -> EOS device number

extern bool system_build_directory(unsigned char ds, unsigned long numBlocks, char *v);

static unsigned char *disk;
static unsigned long disk_blocks;
static unsigned char want_dev;
static unsigned char written[65536];
static bool rescanned;

void eos_init(void) {}

void eos_scan_for_devices(void)
{
  rescanned = true;
}

unsigned char eos_write_block(unsigned char dev, unsigned long blockno, void *buf)
{
  if (dev != want_dev || blockno >= disk_blocks)
    {
      fprintf(stderr, "bad write: dev %u block %lu\n", dev, blockno);
      exit(2);
    }
  memcpy(disk + blockno * BLOCK, buf, BLOCK);
  written[blockno] = 1;
  return EOS_ERR_NONE;
}

int main(int argc, char **argv)
{
  char label[12];
  unsigned long dir_blocks, b;
  FILE *fp;

  if (argc != 5)
    {
      fprintf(stderr, "usage: %s slot blocks label out\n", argv[0]);
      return 2;
    }

  want_dev = (unsigned char)(atoi(argv[1]) + DEV_OFFSET);
  disk_blocks = strtoul(argv[2], NULL, 0);
  memset(label, 0, sizeof(label));
  strncpy(label, argv[3], sizeof(label) - 1);
  disk = calloc(disk_blocks, BLOCK);

  if (!system_build_directory((unsigned char)atoi(argv[1]), disk_blocks, label))
    {
      fprintf(stderr, "system_build_directory failed\n");
      return 1;
    }
  if (!rescanned)
    {
      fprintf(stderr, "devices were not rescanned\n");
      return 1;
    }

  // boot block plus every directory block must have been written
  dir_blocks = disk_blocks > 719 ? 6 : disk_blocks > 319 ? 3 : disk_blocks > 160 ? 2 : 1;
  for (b = 0; b <= dir_blocks; b++)
    if (!written[b])
      {
        fprintf(stderr, "block %lu was not written\n", b);
        return 1;
      }

  fp = fopen(argv[4], "wb");
  if (!fp || fwrite(disk, BLOCK, disk_blocks, fp) != disk_blocks)
    {
      fprintf(stderr, "cannot write %s\n", argv[4]);
      return 2;
    }
  fclose(fp);
  return 0;
}
