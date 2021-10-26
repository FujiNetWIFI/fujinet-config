/**
 * #FujiNet AdamNet calls
 */

#ifndef FUJINET_SIO_H
#define FUJINET_SIO_H

#include <stdbool.h>
#include "fuji_typedefs.h"

/**
 * Get SSID
 */
unsigned char fuji_adamnet_get_ssid(NetConfig* n);

/**
 * Return number of networks
 */
unsigned char fuji_adamnet_do_scan(void);

/**
 * Return Network entry from last scan
 */
bool fuji_adamnet_scan_result(unsigned char n, SSIDInfo* ssidInfo);

/**
 * Write desired SSID and password to ADAMNET
 */
bool fuji_adamnet_set_ssid(bool save, NetConfig* netConfig);

/**
 * Get WiFi Network Status
 */
bool fuji_adamnet_get_wifi_status(unsigned char* wifiStatus);

/**
 * Read host slots
 */
void fuji_adamnet_read_host_slots(HostSlots* hostSlots);

/**
 * Read drive tables
 */
void fuji_adamnet_read_device_slots(DeviceSlots* deviceSlots);

/**
 * Mount a host slot
 */
void fuji_adamnet_mount_host(unsigned char c, HostSlots* hostSlots);

/**
 * Read #FujiNet Adapter configuration
 */
void fuji_adamnet_read_adapter_config(AdapterConfig* adapterConfig);

/**
 * Write host slots
 */
void fuji_adamnet_write_host_slots(HostSlots* hostSlots);

/**
 * Open Directory
 */
void fuji_adamnet_open_directory(unsigned char hs, char *p);

/**
 * Read next dir entry
 */
void fuji_adamnet_read_directory(char *e, unsigned char len, unsigned char addtl);

/**
 * Get current directory cursor
 */
DirectoryPosition fuji_adamnet_get_directory_position(void);

/**
 * Set current directory cursor
 */
void fuji_adamnet_set_directory_position(DirectoryPosition pos);

/**
 * Close directory
 */
void fuji_adamnet_close_directory(unsigned char hs);

#endif /* FUJINET_ADAM_H */
