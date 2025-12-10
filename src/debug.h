#ifndef DEBUG_H
#define DEBUG_H

#ifdef BUILD_MSXROM
#include "msx/msx_debug.h"
#else
void debug();
#endif

#endif
