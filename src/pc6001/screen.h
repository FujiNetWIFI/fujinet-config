#ifdef BUILD_PC6001
/**
 * FujiNet Config for #Adam
 * 
 * Screen routines
 */

#ifndef SCREEN_H
#define SCREEN_H

#include "../fuji_typedefs.h"

void screen_init(void);
void screen_error(const char *c);

void screen_set_wifi(AdapterConfig* ac);
void screen_set_wifi_display_ssid(char n, SSIDInfo *s);
void screen_set_wifi_select_network(unsigned char nn);
void screen_set_wifi_custom(void);
void screen_set_wifi_password(void);

void screen_connect_wifi(NetConfig *nc);

void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d);
void screen_hosts_and_devices_hosts(void);
void screen_hosts_and_devices_devices(void);
void screen_hosts_and_devices_host_slots(HostSlot *h);
void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *d);

void screen_hosts_and_devices_devices_clear_all(void);
void screen_hosts_and_devices_clear_host_slot(unsigned char i);
void screen_hosts_and_devices_edit_host_slot(unsigned char i);

void screen_hosts_and_devices_eject(unsigned char ds);
void screen_hosts_and_devices_host_slot_empty(unsigned char hs);

void screen_hosts_and_devices_long_filename(char *f);

void screen_show_info(AdapterConfig* ac);

void screen_select_file(void);
void screen_select_file_display(char *p, char *f);
void screen_select_file_display_long_filename(char *e);
void screen_select_file_clear_long_filename(void);
void screen_select_file_filter(void);
void screen_select_file_next(void);
void screen_select_file_prev(void);
void screen_select_file_display_entry(unsigned char y, char* e, unsigned entryType);
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
#endif /* BUILD_PC6001 */
