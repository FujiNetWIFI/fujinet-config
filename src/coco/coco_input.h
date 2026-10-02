#ifndef COCO_INPUT_H
#define COCO_INPUT_H

#include "coco_screen.h"

#define KEY_REDRAW 0x7F
#define KEY_SHIFT_LEFT_ARROW  0x15
#define KEY_SHIFT_RIGHT_ARROW 0x5D

extern bool input_abortable;
extern bool input_aborted;

byte waitkey_joystick(void);
bool input_color_key(byte k);
unsigned char input_handle_joystick(void);

#endif
