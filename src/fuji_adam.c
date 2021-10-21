/**
 * #FujiNet SIO calls
 */

#include <eos.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "fuji_adam.h"

#define FUJI 0x0f 
#define FUJI_MAX_RESPONSE_SIZE 1024

unsigned char fuji_write_sync(char* buf, unsigned short len)
{
  eos_start_write_character_device(FUJI, buf, len);
  while (!(eos_end_write_character_device(FUJI) & 0x80))
    {
      // sit and spin until acknowledgement
    }

  return eos_end_write_character_device(FUJI);
}

unsigned char fuji_read_sync(char* buf, unsigned short len)
{
  eos_start_read_character_device(FUJI,buf,FUJI_MAX_RESPONSE_SIZE);
  while (!(eos_end_read_character_device(FUJI) & 0x80))
    {
      // sit and spin until acknowledgement
    }

  return eos_end_read_character_device(FUJI);
}

/**
 * Return number of networks
 */
unsigned char fuji_adamnet_do_scan(void)
{
  char num_networks=0;
  char scan_cmd = 0xFD;

  // Issue Command
  fuji_write_sync(&scan_cmd,sizeof(scan_cmd));

  // Wait for response
  fuji_read_sync(&num_networks,1);

  return num_networks;
}

/**
 * Return Network entry from last scan
 */
bool fuji_adamnet_scan_result(unsigned char n, SSIDInfo* ssidInfo)
{
  char result_cmd[2] = {0xFC,0x00};

  result_cmd[1] = n;

  // Issue Command
  fuji_write_sync(result_cmd,sizeof(result_cmd));

  // Wait for response
  fuji_read_sync(ssidInfo->rawData,sizeof(ssidInfo->rawData));

  return true;
}

/**
 * Write desired SSID and password to ADAMNET
 */
bool fuji_adamnet_set_ssid(bool save, NetConfig* netConfig)
{
  char set_ssid_cmd[97];

  set_ssid_cmd[0]=0xFB;

  memcpy(&set_ssid_cmd[1],netConfig->rawData,sizeof(netConfig->rawData));

  // Issue command
  fuji_write_sync(set_ssid_cmd,sizeof(set_ssid_cmd));

  return true;
}

/**
 * Get WiFi Network Status
 */
bool fuji_adamnet_get_wifi_status(unsigned char* wifiStatus)
{
  char get_wifi_status_cmd=0xFA;

  // Issue command
  fuji_write_sync(&get_wifi_status_cmd,1);

  // Get response
  fuji_read_sync((char *)wifiStatus,1);  
}

