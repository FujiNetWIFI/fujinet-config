#ifndef COCO_INPUT_H
#define COCO_INPUT_H

#include "coco_screen.h"

#define KEY_REDRAW 0x7F

extern bool input_abortable;
extern bool input_aborted;

byte waitkey_joystick(void);
unsigned char input_handle_joystick(void);

#endif
