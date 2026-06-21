#ifndef PAUSE_H
#define PAUSE_H
#include <stdint.h>

void __FASTCALL__ pause(uint8_t jiffies);
void __FASTCALL__ pausesec(uint8_t seconds);

#endif
