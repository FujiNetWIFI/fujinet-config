#ifdef BUILD_ADAM
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
 * @brief Key syms
 */

#define KEY_BACKSPACE    0x08
#define KEY_TAB          0x09
#define KEY_RETURN       0x0D
#define KEY_ESCAPE       0x1B
#define KEY_SPACE        0x20
#define KEY_1            0x31
#define KEY_2            0x32
#define KEY_3            0x33
#define KEY_4            0x34
#define KEY_5            0x35
#define KEY_6            0x36
#define KEY_7            0x37
#define KEY_8            0x38
#define KEY_HOME         0x80
#define KEY_SMART_I      0x81
#define KEY_SMART_II     0x82
#define KEY_SMART_III    0x83
#define KEY_SMART_IV     0x84
#define KEY_SMART_V      0x85
#define KEY_SMART_VI     0x86
#define KEY_WILD_CARD    0x90
#define KEY_UNDO         0x91
#define KEY_MOVE         0x9A
#define KEY_GET          0x93
#define KEY_INSERT       0x94
#define KEY_PRINT        0x95
#define KEY_CLEAR        0x96
#define KEY_DELETE       0x97
#define KEY_COPY         0x92
#define KEY_STORE        0x9B
#define KEY_S_INSERT     0x9C
#define KEY_S_PRINT      0x9D
#define KEY_S_CLEAR      0x9E
#define KEY_S_DELETE     0x9F
#define KEY_UP_ARROW     0xA0
#define KEY_DOWN_ARROW   0xA2
#define KEY_C_UP_ARROW   0xA4
#define KEY_C_DOWN_ARROW 0xA6


/**
 * @brief The abort key
 */
#define KEY_ABORT KEY_ESCAPE

#endif /* INPUT_H */
#endif /* BUILD_ADAM */
