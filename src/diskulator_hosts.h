/**
 * #FUJINET CONFIG
 * Diskulator Hosts / Device Slots screen
 */

#ifndef DISKULATOR_HOSTS_H
#define DISKULATOR_HOSTS_H

#define ORIGIN_HOST_SLOTS 2
#define ORIGIN_DEVICE_SLOTS 13

#include "state.h"

/**
 * Display Hosts Slots
 */
void diskulator_hosts_display_host_slots(unsigned char y, Context *context);

/**
 * Display device slots
 */
void diskulator_hosts_display_device_slots(unsigned char y, Context *context);

/**
 * Eject image from device slot
 */
void diskulator_hosts_eject_device_slot(unsigned char i, unsigned char pos, Context *context);

/**
 * Set device slot to read or write
 */
void diskulator_hosts_set_device_slot_mode(unsigned char i, unsigned char mode, Context *context);

/**
 * Hosts/Devices screen
 */
State diskulator_hosts(Context *context);

#endif /* DISKULATOR_HOSTS_H */
