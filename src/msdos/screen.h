#ifdef __WATCOMC__

#ifndef SCREEN_H
#define SCREEN_H

#include <stdbool.h>
#include "../fuji_typedefs.h"

/**
 * @brief The standard RGBI color palette for text mode.
 */
#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define MAGENTA 5
#define BROWN 6
#define LIGHT_GRAY 7
#define DARK_GRAY 8
#define LIGHT_BLUE 9
#define LIGHT_GREEN 10
#define LIGHT_CYAN 11
#define LIGHT_RED 12
#define LIGHT_MAGENTA 13
#define YELLOW 14
#define WHITE 15

#define ATTRIBUTE_HEADER 0x09
#define ATTRIBUTE_NORMAL 0x07
#define ATTRIBUTE_BOLD 0x0F
#define ATTRIBUTE_SELECTED 0x70

/**
 * @brief screen variables
 */
extern unsigned char screen_mode;
extern unsigned char screen_cols;
extern unsigned char far *video;

void screen_mount_and_boot();
void screen_clear(void);
void set_wifi_print_rssi(SSIDInfo *s, unsigned char i);

void screen_select_slot_mode(void);
void font_init();
void screen_putc(unsigned char x, unsigned char y, unsigned char a, const char c);
void screen_puts(unsigned char x, unsigned char y, unsigned char a, const char *s);
void screen_puts_center(unsigned char y, unsigned char a, const char *s);
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

void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, bool *e);
void screen_hosts_and_devices_hosts(void);
void screen_hosts_and_devices_devices(void);
void screen_hosts_and_devices_host_slots(HostSlot *h);
void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *d, bool *e);

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

#endif /* SCREEN_H */

#endif /*__WATCOMC__ */
