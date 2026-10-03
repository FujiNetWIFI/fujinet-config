#ifndef COCO_INPUT_H
#define COCO_INPUT_H

#include "coco_screen.h"

#define KEY_REDRAW 0x7F
#define KEY_SHIFT_LEFT_ARROW  0x15
#define KEY_SHIFT_RIGHT_ARROW 0x5D
#define KEY_SHIFT_CLEAR       0x5C

extern bool input_abortable;
extern bool input_aborted;
extern bool input_digits_only;

byte waitkey_joystick(void);
/* 0: not a color key; 1: handled; 2: handled, redraw the screen. */
byte input_color_key(byte k);
unsigned char input_handle_joystick(void);

#endif
