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
 * Get SSID
 */
unsigned char fuji_adamnet_get_ssid(NetConfig* n)
{
  char get_ssid_cmd = 0xFE;

  // Issue command
  fuji_write_sync(&get_ssid_cmd,sizeof(get_ssid_cmd));

  // Wait for response
  fuji_read_sync(n->rawData,sizeof(n->rawData));

  return true;
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

/**
 * Read host slots
 */
void fuji_adamnet_read_host_slots(HostSlots* hostSlots)
{
  char read_host_slots_cmd=0xF4;

  // Issue command
  fuji_write_sync(&read_host_slots_cmd,1);

  // Get response
  fuji_read_sync((char *)hostSlots,sizeof(HostSlots));    
}

/**
 * Read drive tables
 */
void fuji_adamnet_read_device_slots(DeviceSlots* deviceSlots)
{
  char read_device_slots_cmd=0xF2;

  // Issue command
  fuji_write_sync(&read_device_slots_cmd,1);

  // Get response
  fuji_read_sync((char *)deviceSlots,304);
}

/**
 * Mount a host slot
 */
void fuji_adamnet_mount_host(unsigned char c, HostSlots* hostSlots)
{
  char mount_host_cmd[2]={0xF9,0x00};

  // Issue command
  mount_host_cmd[1]=c;
  fuji_write_sync(mount_host_cmd,sizeof(mount_host_cmd));
}

/**
 * Read #FujiNet Adapter configuration
 */
void fuji_adamnet_read_adapter_config(AdapterConfig* adapterConfig)
{
  char get_adapter_config_cmd=0xE8;

  // Issue Command
  fuji_write_sync(&get_adapter_config_cmd,1);

  // Get Response
  fuji_read_sync((char *)&adapterConfig->rawData,sizeof(adapterConfig->rawData));
}

/**
 * Write host slots
 */
void fuji_adamnet_write_host_slots(HostSlots* hostSlots)
{
  char write_host_slots_cmd[257];
  write_host_slots_cmd[0]=0xF3;
  memcpy(&write_host_slots_cmd[1],hostSlots,sizeof(HostSlots));

  // Issue Command
  fuji_write_sync(write_host_slots_cmd,sizeof(write_host_slots_cmd));
}
