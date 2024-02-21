#ifndef FUJI_TYPEDEFS_IO_H
#define FUJI_TYPEDEFS_IO_H

// These entries also exist in fujinet-lib, but for systems that are not
// using fujinet-lib yet, we define them here. This decouples systems
// that don't use fujinet-lib yet from including fujinet-io.h from it.

#define FILE_MAXLEN 36
#define SSID_MAXLEN 33 /* 32 + NULL */

/**
 * Returned info for a single SSID entry
 * from a WiFi scan
 */
typedef struct {
  char ssid[SSID_MAXLEN];
  signed char rssi;
} SSIDInfo;

/**
 * The current wifi SSID and password
 */
typedef struct
{
  char ssid[SSID_MAXLEN];
  char password[64];
} NetConfig;

/**
 * The current network adapter configuration
 */
typedef struct
{
  char ssid[SSID_MAXLEN];
  char hostname[64];
  unsigned char localIP[4];
  unsigned char gateway[4];
  unsigned char netmask[4];
  unsigned char dnsIP[4];
  unsigned char macAddress[6];
  unsigned char bssid[6];
  char fn_version[15];
} AdapterConfig;

typedef struct
{
  char ssid[SSID_MAXLEN];
  char hostname[64];
  unsigned char localIP[4];
  unsigned char gateway[4];
  unsigned char netmask[4];
  unsigned char dnsIP[4];
  unsigned char macAddress[6];
  unsigned char bssid[6];
  char fn_version[15];
  char sLocalIP[16];
  char sGateway[16];
  char sNetmask[16];
  char sDnsIP[16];
  char sMacAddress[18];
  char sBssid[18];
} AdapterConfigExtended;

typedef unsigned char HostSlot[32];

typedef struct {
  unsigned char hostSlot;
  unsigned char mode;
  unsigned char file[FILE_MAXLEN];
} DeviceSlot;

typedef struct
{
  unsigned short numSectors;
  unsigned short sectorSize;
  unsigned char hostSlot;
  unsigned char deviceSlot;
  char filename[256];
} NewDisk;

#endif /* FUJI_TYPEDEFS_IO_H */