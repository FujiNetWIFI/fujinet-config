/**
 * #FujiNet AdamNet calls
 */

#ifndef FUJINET_SIO_H
#define FUJINET_SIO_H

#include <stdbool.h>
#include "fuji_typedefs.h"

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

#endif /* FUJINET_ADAM_H */
