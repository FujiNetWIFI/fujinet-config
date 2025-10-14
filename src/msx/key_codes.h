/**
 * FujiNet CONFIG for MSX
 *
 * MSX Key codes
 */

#ifndef MSX_KEY_CODES_H
#define MSX_KEY_CODES_H

#include <stdlib.h>

#define KEY_EOL          0x0A
#define KEY_RETURN       KEY_EOL
#define KEY_ESCAPE       0x1B
#define KEY_SPACE        0x20
#define KEY_K0           0x1B
#define KEY_K1           0x80
#define KEY_K2           0x81
#define KEY_K3           0x82
#define KEY_K4           0x83
#define KEY_K5           0x84
#define KEY_K6           0x85
#define KEY_K7           0x86
#define KEY_K8           0x87
#define KEY_K9           0x88
#define KEY_K10          0x89
#define KEY_K11          0x8A
#define KEY_TAB          0x09
#define KEY_DELETE       0x7F
#define KEY_1            0x31
#define KEY_2            0x32
#define KEY_3            0x33
#define KEY_4            0x34
#define KEY_5            0x35
#define KEY_6            0x36
#define KEY_7            0x37
#define KEY_8            0x38
#define KEY_9            0x39
#define KEY_0            0x30
#define KEY_RIGHT_ARROW  0x1C
#define KEY_LEFT_ARROW   0x1D
#define KEY_HOME         0x1E
#define KEY_END          0x1F
#define KEY_UP_ARROW     KEY_HOME
#define KEY_DOWN_ARROW   KEY_END
#define KEY_RCL          0x8D
#define KEY_CLR          0x8E

#define KEY_ABORT KEY_ESCAPE

#endif /* MSX_KEY_CODES_H */
