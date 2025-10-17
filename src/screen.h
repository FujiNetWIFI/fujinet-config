/**
 * Screen routines
 */

#ifndef SCREEN_H
#define SCREEN_H

#include "typedefs.h"

#if defined(BUILD_APPLE2) || defined(BUILD_PMD85)
#include <conio.h>
#endif /* BUILD_APPLE2 || BUILD_PMD85 */

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

void screen_init(void);
void screen_end(void);
void screen_error(const char *s);

void screen_putlcc(char c);
void screen_put_inverse(char c);
void screen_print_inverse(const char *s);
void screen_print_menu(const char *si, const char *sc);

void screen_set_wifi(AdapterConfig *ac);
void screen_set_wifi_extended(AdapterConfigExtended* acx);
void screen_set_wifi_display_ssid(char n, SSIDInfo *s);
void screen_set_wifi_select_network(uint8_t nn);
void screen_set_wifi_custom(void);
void screen_set_wifi_password(void);

void screen_connect_wifi(NetConfig *nc);

void screen_destination_host_slot(char *h, char *p);
void screen_destination_host_slot_choose(void);

const char* screen_hosts_and_devices_device_slot(uint8_t hs, bool e, const char *fn);
void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, bool *e);
void screen_hosts_and_devices_hosts(void);
void screen_hosts_and_devices_host_slots(HostSlot *h);
void screen_hosts_and_devices_devices(void);
void screen_hosts_and_devices_devices_selected(char selected_slot);
void screen_hosts_and_devices_device_slots(uint8_t y, DeviceSlot *d, const bool *e);

void screen_hosts_and_devices_clear_host_slot(uint_fast8_t i);
void screen_hosts_and_devices_edit_host_slot(uint_fast8_t i);

void screen_hosts_and_devices_eject(uint8_t ds);
void screen_hosts_and_devices_host_slot_empty(uint_fast8_t hs);

void screen_perform_copy(char *sh, char *p, char *dh, char *dp);

//void screen_show_info(bool printerEnabled, AdapterConfig* ac);
void screen_show_info_extended(bool printerEnabled, AdapterConfigExtended* acx);

void screen_select_file(void);
void screen_select_file_display(char *p, char *f);
void screen_select_file_filter(void);
void screen_select_file_next(void);
void screen_select_file_new_type(void);
void screen_select_file_prev(void);
void screen_select_file_display_long_filename(const char *e);
void screen_select_file_display_entry(uint8_t y, const char* e, uint16_t entryType);
void screen_select_file_clear_long_filename(void);
void screen_select_file_choose(char visibleEntries);
void screen_select_file_new_size(uint8_t k);
void screen_select_file_new_custom(void);
void screen_select_file_new_name(void);
void screen_hosts_and_devices_long_filename(const char *f);
void screen_hosts_and_devices_devices_clear_all(void);
void screen_select_file_new_creating(void);

void screen_select_slot(const char *e);
void screen_select_slot_choose(void);
void screen_select_slot_mode(void);
void screen_select_slot_eject(uint8_t ds);

bool screen_mount_and_boot_lobby(void);

void screen_select_slot_build_eos_directory(void);
void screen_select_slot_build_eos_directory_label(void);
void screen_select_slot_build_eos_directory_creating(void);

/**
 * Clear the currently displayed bar from screen
 */
void bar_clear(bool oldRow);

/**
 * Set up bar and start display on row
 * @param y Y column for bar display
 * @param c # of columns for left
 * @param m number of items
 * @param i item index to start display
 */
void bar_set(uint8_t y, uint8_t c, uint8_t m, uint8_t i);

/**
 * Set up bar and start display on row
 * @param y Y column for bar display
 * @param c # of columns for left
 * @param m number of items
 * @param i item index to start display
 * @param s s split s lines (used to allow for diskII separater)
 */
void bar_set_split(uint8_t y, uint8_t c, uint8_t m, uint8_t i, uint8_t s);

/**
 * Move bar upward until index 0
 */
void bar_up(void);

/**
 * Move bar downward until index m
 */
void bar_down(void);

/**
 * Jump to location
 * @param i new y offset
 */
void bar_jump(uint_fast8_t i);

/**
 * Draw bar at y
 */
void bar_draw(int y, bool clear);

void bar_update(void);

/**
 * Get current bar position
 * @return bar index
 */
uint_fast8_t bar_get(void);

void set_cursor(unsigned char x, unsigned char y);
void bar_set_color(unsigned char c);

int edit_line(unsigned char x, unsigned char y, char *s, unsigned char maxlen, bool is_password);

void screen_puts(unsigned char x, unsigned char y, const char *s);
void screen_clear_line(unsigned char y);
void screen_mount_and_boot();
void set_active_screen(unsigned char screen);
void screen_clear(void);
void screen_dlist_mount_and_boot(void);
void screen_dlist_show_info(void);
void screen_dlist_set_wifi(void);
void screen_dlist_select_slot(void);
void screen_dlist_select_file(void);
void screen_dlist_hosts_and_devices(void);
void screen_put(int x, int y, uint8_t c);

extern void bar_setup_regs();

#ifdef BUILD_PMD85
#define SCR_X0    4
#define SCR_Y0    4
#define STATUS_BAR (SCR_Y0 + 21)

#include "pmd85/colors.h"
#endif /* BUILD_PMD85 */

#endif /* SCREEN_H */
