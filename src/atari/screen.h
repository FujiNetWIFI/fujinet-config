#ifdef BUILD_ATARI

#ifndef SCREEN_H
#define SCREEN_H

#include "fuji_typedefs.h"

#define FONT_MEMORY 0x7800

// Used to identify what screen we're currently displaying. The set_cursor() method uses this so it know what screen we're on so it
// can correctly calculate the memory offset in video memory when given an x,y coordinate pair.
//
typedef enum
{
   SCREEN_HOSTS_AND_DEVICES,
   SCREEN_SHOW_INFO,
   SCREEN_SET_WIFI,
   SCREEN_SELECT_FILE,
   SCREEN_SELECT_SLOT,
   SCREEN_MOUNT_AND_BOOT,
   SCREEN_CONNECT_WIFI
} _screen;

#define screen_input(x, y, s) _screen_input((x), (y), (s), sizeof(s))

#ifdef DEBUG
void show_line_nums(void);
void screen_debug(char *message);
#endif // DEBUG

void set_active_screen(unsigned char screen);
void screen_mount_and_boot();
int _screen_input(unsigned char x, unsigned char y, char *s, unsigned char maxlen);
void screen_dlist_connect_wifi(void);
void screen_dlist_hosts_and_devices(void);
void screen_dlist_show_info(void);
void screen_dlist_select_file(void);
void screen_dlist_set_wifi(void);
void screen_dlist_mount_and_boot(void);
void screen_dlist_select_slot(void);
void screen_print_ip(unsigned char x, unsigned char y, unsigned char *buf);
void screen_print_mac(unsigned char x, unsigned char y, unsigned char *buf);
void itoa_hex(unsigned char val, char *buf);
void screen_clear(void);
void set_wifi_print_rssi(SSIDInfo *s, unsigned char i);
// void screen_set_wifi_display_mac_address(AdapterConfig *adapterConfig);

void screen_select_slot_mode(void);
void font_init();
extern void bar_setup_regs();
void screen_puts(unsigned char x, unsigned char y, char *s);
void screen_clear_line(unsigned char y);
void screen_error(const char *msg);

void screen_init(void);
void screen_error(const char *c);

void screen_set_wifi(AdapterConfig *ac);
void screen_set_wifi_display_ssid(char n, SSIDInfo *s);
void screen_set_wifi_select_network(unsigned char nn);
void screen_set_wifi_custom(void);
void screen_set_wifi_password(void);

void screen_connect_wifi(NetConfig *nc);

void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, unsigned char *e);
void screen_hosts_and_devices_hosts(void);
void screen_hosts_and_devices_devices(void);
void screen_hosts_and_devices_host_slots(HostSlot *h);
void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *d, unsigned char *e);

void screen_hosts_and_devices_devices_clear_all(void);
void screen_hosts_and_devices_clear_host_slot(unsigned char i);
void screen_hosts_and_devices_edit_host_slot(unsigned char i);

void screen_hosts_and_devices_eject(unsigned char ds);
void screen_hosts_and_devices_host_slot_empty(unsigned char hs);

void screen_hosts_and_devices_long_filename(char *f);

void screen_show_info(int printerEnabled, AdapterConfig *ac);

void screen_select_file(void);
void screen_select_file_display(char *p, char *f);
void screen_select_file_display_long_filename(char *e);
void screen_select_file_clear_long_filename(void);
void screen_select_file_filter(void);
void screen_select_file_next(void);
void screen_select_file_prev(void);
void screen_select_file_display_entry(unsigned char y, char *e, unsigned entryType);
void screen_select_file_choose(char visibleEntries);
void screen_select_file_new_type(void);
void screen_select_file_new_size(unsigned char k);
void screen_select_file_new_custom(void);
void screen_select_file_new_name(void);
void screen_select_file_new_creating(void);

void screen_select_slot(char *e);
void screen_select_slot_choose(void);
void screen_select_slot_eject(unsigned char ds);
void screen_select_slot_build_eos_directory(void);
void screen_select_slot_build_eos_directory_label(void);
void screen_select_slot_build_eos_directory_creating(void);

void screen_destination_host_slot(char *h, char *p);
void screen_destination_host_slot_choose(void);

void screen_perform_copy(char *sh, char *p, char *dh, char *dp);


extern unsigned char *cursor_ptr;
#define GRAPHICS_0_SCREEN_SIZE (40 * 25) // Defines the memory size in bytes
#define DISPLAY_LIST 0x0600              // Memory address to store DISPLAY_LIST.  0x0600 is the first address available for user space memory (1)
#define DISPLAY_MEMORY 0x7400            // Memory address to store DISPLAY_MEMORY.

/**
 * The following defines assign an Atari internal code (2) character to specific labels used for display.
 **/

#define CH_FOLDER "\x04"      // Set the character folder to #
#define CH_SERVER "\x06"      // Set the server folder to atari heart.
#define CH_KEY_LABEL_L "\xD9" // Left arrow on the keyboard
#define CH_KEY_LABEL_R "\x19" // Right arrow on the keyboard

// Inverse of internal character codes
#define CH_INV_MINUS "\x8d"
#define CH_INV_LT "\x9C"
#define CH_INV_GT "\x9E"

#define CH_INV_A "\xA1"
#define CH_INV_B "\xA2"
#define CH_INV_C "\xA3"
#define CH_INV_D "\xA4"
#define CH_INV_E "\xA5"
#define CH_INV_F "\xA6"
#define CH_INV_G "\xA7"
#define CH_INV_H "\xA8"
#define CH_INV_I "\xA9"
#define CH_INV_J "\xAA"
#define CH_INV_K "\xAB"
#define CH_INV_L "\xAC"
#define CH_INV_M "\xAD"
#define CH_INV_N "\xAE"
#define CH_INV_O "\xAF"
#define CH_INV_P "\xB0"
#define CH_INV_Q "\xB1"
#define CH_INV_R "\xB2"
#define CH_INV_S "\xB3"
#define CH_INV_T "\xB4"
#define CH_INV_U "\xB5"
#define CH_INV_V "\xB6"
#define CH_INV_W "\xB7"
#define CH_INV_X "\xB8"
#define CH_INV_Y "\xB9"
#define CH_INV_Z "\xBA"

#define CH_INV_1 "\x91"
#define CH_INV_2 "\x92"
#define CH_INV_3 "\x93"
#define CH_INV_4 "\x94"
#define CH_INV_5 "\x95"
#define CH_INV_6 "\x96"
#define CH_INV_7 "\x97"
#define CH_INV_8 "\x98"
#define CH_INV_9 "\x99"
#define CH_INV_LEFT "\xDE"
#define CH_INV_RIGHT "\xDF"

#define CH_KEY_1TO8 CH_KEY_LABEL_L CH_INV_1 CH_INV_MINUS CH_INV_8 CH_KEY_LABEL_R
#define CH_KEY_ESC CH_KEY_LABEL_L CH_INV_E CH_INV_S CH_INV_C CH_KEY_LABEL_R
#define CH_KEY_TAB CH_KEY_LABEL_L CH_INV_T CH_INV_A CH_INV_B CH_KEY_LABEL_R
#define CH_KEY_DELETE CH_KEY_LABEL_L CH_INV_D CH_INV_E CH_INV_L CH_INV_E CH_INV_T CH_INV_E CH_KEY_LABEL_R
#define CH_KEY_LEFT CH_KEY_LABEL_L CH_INV_LEFT CH_KEY_LABEL_R
#define CH_KEY_RIGHT CH_KEY_LABEL_L CH_INV_RIGHT CH_KEY_LABEL_R
#define CH_KEY_RETURN CH_KEY_LABEL_L CH_INV_R CH_INV_E CH_INV_T CH_INV_U CH_INV_R CH_INV_N CH_KEY_LABEL_R
#define CH_KEY_OPTION CH_KEY_LABEL_L CH_INV_O CH_INV_P CH_INV_T CH_INV_I CH_INV_O CH_INV_N CH_KEY_LABEL_R
#define CH_KEY_C CH_KEY_LABEL_L CH_INV_C CH_KEY_LABEL_R
#define CH_KEY_N CH_KEY_LABEL_L CH_INV_N CH_KEY_LABEL_R
#define CH_KEY_F CH_KEY_LABEL_L CH_INV_F CH_KEY_LABEL_R
#define CH_KEY_LT CH_KEY_LABEL_L CH_INV_LT CH_KEY_LABEL_R
#define CH_KEY_GT CH_KEY_LABEL_L CH_INV_GT CH_KEY_LABEL_R
#define CH_KEY_ESC CH_KEY_LABEL_L CH_INV_E CH_INV_S CH_INV_C CH_KEY_LABEL_R

/**
 * Define key code to detect during keyboard capture
 */
#define KCODE_RETURN 0x9B // is the ATASCI equivlant of 155 End Of Line (return)
#define KCODE_ESCAPE 0x1B
#define KCODE_BACKSP 0x7E

#endif /* SCREEN_H */
#endif
