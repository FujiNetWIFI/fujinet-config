#ifndef FC_PAUSE_H
#define FC_PAUSE_H

#ifdef BUILD_MSXROM
#include "msx/msx_pause.h"
#else
void pause(unsigned char delay);
#endif

#endif /* FC_PAUSE_H */
