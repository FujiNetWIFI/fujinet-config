#ifndef MSX_DEBUG_H
#define MSX_DEBUG_H
#include <stdio.h>

extern char _mybuffer[2000];

#define debugf(fmt, ...) \
        do { sprintf(_mybuffer, fmt, __VA_ARGS__); _debug(); } while(0)

#define debug(s) \
        do { sprintf(_mybuffer, s); _debug(); } while(0)

void _debug();

#endif
