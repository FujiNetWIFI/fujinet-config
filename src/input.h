/**
 * Input Routines
 */

#ifndef INPUT_H
#define INPUT_H

/**
 * Get input from keyboard/joystick
 * @return keycode (or synthesized keycode if joystick)
 */
unsigned char input();

/**
 * Get line of input into c
 */
unsigned char input_line(char *c, bool password);


#endif /* INPUT_H */
