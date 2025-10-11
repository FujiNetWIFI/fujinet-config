/**
 * Input Routines
 */

#ifndef INPUT_H
#define INPUT_H

#include "typedefs.h"

/**
 * Get input from keyboard/joystick
 * @return keycode (or synthesized keycode if joystick)
 */
uint8_t input(void);

/**
 * Get input from keyboard/joystick, translating lowercase presses to uppercase
 * @return keycode (or synthesized keycode if joystick)
 */
uint8_t input_ucase(void);

/**
 * Get line of input into c
 * @param x X position
 * @param y Y position
 * @param o start at character offset
 * @param c target buffer
 * @param l Length
 * @param password echoes characters.
 */
void input_line(uint8_t x, uint8_t y, uint8_t o, char *c, uint8_t l, bool password);

DHSubState input_destination_host_slot_choose(void);
SFSubState input_select_file_choose(void);
uint8_t input_select_file_new_type(void);
uint32_t input_select_file_new_size(uint8_t t);
uint32_t input_select_file_new_custom(void);
void input_select_file_new_name(char *c);

SSSubState input_select_slot_choose(void);
uint8_t input_select_slot_mode(char *mode);
SISubState input_show_info(void);

HDSubState input_hosts_and_devices_hosts(void);
HDSubState input_hosts_and_devices_devices(void);

void input_line_set_wifi_custom(char *c);
void input_line_set_wifi_password(char *c);
WSSubState input_set_wifi_select(void);
void input_line_hosts_and_devices_host_slot(uint_fast8_t i, uint_fast8_t o, char *c);
void input_line_filter(char *c);

bool input_select_slot_build_eos_directory(void);
void input_select_slot_build_eos_directory_label(char *c);

#ifdef BUILD_ADAM
#include "adam/key_codes.h"
#endif /* BUILD_ADAM */

#if defined(BUILD_APPLE2) || defined(BUILD_ATARI)
#include <conio.h>
#define KEY_ABORT CH_ESC
#endif

#ifdef BUILD_C64
#define KEY_ABORT 0x5F
#endif

#ifdef BUILD_ATARI
#define KCODE_ESCAPE CH_ESC
#define KCODE_RETURN 0x9B // is the ATASCI equivlant of 155 End Of Line (return)
#define KCODE_BACKSP 0x7E

#define CH_KEY_LABEL_L "\xD9" // Left arrow on the keyboard
#endif

#ifdef BUILD_PMD85
#define KEY_ABORT 0x1b
#endif /* BUILD_PMD85 */

#endif /* INPUT_H */
