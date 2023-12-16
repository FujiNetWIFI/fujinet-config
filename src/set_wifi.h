/**
 * FujiNet for #Adam configuration program
 *
 * Set to existing WiFi Connection
 */

#ifndef SET_WIFI_H
#define SET_WIFI_H

#ifdef _CMOC_VERSION_
void set_wifi_set_ssid(int i);
#else
void set_wifi_set_ssid(unsigned char i);
#endif
void set_wifi(void);

#endif /* SET_WIFI */
