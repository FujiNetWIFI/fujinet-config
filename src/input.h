/**
 * Input Routines
 */

#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

/**
 * Get input from keyboard/joystick
 * @return keycode (or synthesized keycode if joystick)
 */
unsigned char input();

/**
 * Get input from keyboard/joystick, translating lowercase presses to uppercase
 * @return keycode (or synthesized keycode if joystick)
 */
unsigned char input_ucase();

/**
 * Get line of input into c
 * @param x X position
 * @param y Y position
 * @param o start at character offset
 * @param c target buffer
 * @param l Length
 * @param password echoes characters.
 */
void input_line(unsigned char x, unsigned char y, unsigned char o, char *c, unsigned char l, bool password);

void input_line_set_wifi_custom(char *c);
void input_line_set_wifi_password(char *c);
void input_line_hosts_and_devices_host_slot(unsigned char i, unsigned char o, char *c);
void input_line_filter(char *c);

#endif /* INPUT_H */
