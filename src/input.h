/**
 * #FUJINET Config
 *
 * Common input routines
 */

#ifndef INPUT_H
#define INPUT_H

/**
 * Handle common nav keys
 * k is the input ATASCII key
 * max is the maximum row
 * i is the current index
 */
void input_handle_nav_keys(char k, unsigned char pos, unsigned char max, unsigned char *i);

/**
 * Handle global console keys.
 */
unsigned char input_handle_console_keys(void);

/**
 * Get an input from keyboard/joystick
 * Returns an 'atascii key' regardless of input
 */
unsigned char input_handle_key(void);
unsigned char input_handle_key_ucase(void);

#endif /* INPUT_H */
