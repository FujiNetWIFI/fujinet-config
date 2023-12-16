/**
 * FujiNet for #Adam configuration program
 *
 * Hosts and Devices
 */

#ifndef HOSTS_AND_DEVICES_H
#define HOSTS_AND_DEVICES_H

#ifdef _CMOC_VERSION_
void hosts_and_devices_edit_host_slot(int i);
#else
void hosts_and_devices_edit_host_slot(unsigned char i);
#endif
void hosts_and_devices(void);
void hosts_and_devices_devices_set_mode(unsigned char m);
void hosts_and_devices_long_filename(void);
void hosts_and_devices_devices_clear_all(void);
void hosts_and_devices_eject(unsigned char ds);
void hosts_and_devices_devices_enable_toggle(unsigned char ds);

#endif /* HOSTS_AND_DEVICES */
