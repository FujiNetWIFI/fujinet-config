#ifdef BUILD_PC8801
/**
 * Input Routines
 */

#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include "../typedefs.h"

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
unsigned char input_select_file_new_type(void);
unsigned long input_select_file_new_size(unsigned char t);
unsigned long input_select_file_new_custom(void);
void input_select_file_new_name(char *c);
bool input_select_slot_build_eos_directory(void);
void input_select_slot_build_eos_directory_label(char *c);
WSSubState input_set_wifi_select(void);
HDSubState input_hosts_and_devices_hosts(void);
HDSubState input_hosts_and_devices_devices(void);
SFSubState input_select_file_choose(void);
SSSubState input_select_slot_choose(void);
SISubState input_show_info(void);
DHSubState input_destination_host_slot_choose(void);

/**
 * @brief The abort key
 */
#define KEY_ABORT 0x1B

#endif /* INPUT_H */
#endif /* BUILD_PC8801 */
