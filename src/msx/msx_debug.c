#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define PORT_DEBUG_CTRL            0x2E
#define PORT_DEBUG_DATA            0x2F

void debug_break() {
  outp(PORT_DEBUG_CTRL, 0xFF);
}

char _mybuffer[1000];

void _debug() {
  outp(PORT_DEBUG_DATA, ((uint16_t)_mybuffer) & 0xFF);
  outp(PORT_DEBUG_DATA, (((uint16_t)_mybuffer) >> 8));
}
