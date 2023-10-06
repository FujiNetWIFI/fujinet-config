/**
 * CONFIG custom types
 */

#ifndef FUJI_TYPEDEFS_H
#define FUJI_TYPEDEFS_H

#define FILE_MAXLEN 36
#define SSID_MAXLEN 33 /* 32 + NULL */

#define MODE_READ 1
#define MODE_WRITE 2

#define MAX_HOST_LEN 32
#define NUM_HOST_SLOTS 8

typedef enum _entry_types
{
  ENTRY_TYPE_TEXT,
  ENTRY_TYPE_FOLDER,
  ENTRY_TYPE_FILE,
  ENTRY_TYPE_LINK,
  ENTRY_TYPE_MENU
} EntryType;

typedef unsigned short DirectoryPosition;

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

#endif /* FUJI_TYPEDEFS_H */
