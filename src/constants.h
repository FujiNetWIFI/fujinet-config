#ifndef CONSTANTS_H
#define CONSTANTS_H

#if defined(BUILD_ATARI)
#define NUM_DEVICE_SLOTS 8
#elif defined(BUILD_APPLE2)
#define NUM_DEVICE_SLOTS 6
#else
#define NUM_DEVICE_SLOTS 4
#endif

#ifdef _CMOC_VERSION_
#define ENTRIES_PER_PAGE 10
#else /* ! _CMOC_VERSION_ */
#define ENTRIES_PER_PAGE 15
#endif /* CMOC_VERSION */

#if defined(BUILD_COCO) || defined(BUILD_ADAM)  \
  || defined(BUILD_APPLE2) || defined(BUILD_RC2014)
#define LEGACY_DIR_ENTRY
//#warning "FujiNet firmware should be updated to use new directory entry structure"
#endif

#ifdef LEGACY_DIR_ENTRY
#define DIR_MAX_LEN 31
#else
#define DIR_MAX_LEN 36
#endif

#if defined(BUILED_APPLE2) || defined(BUILD_C64)
#define PRINTER 0
#else
#define PRINTER 2
#endif

#ifdef BUILD_ATARI
#include "atari/atari_constants.h"
#endif

#endif /* CONSTANTS_H */
