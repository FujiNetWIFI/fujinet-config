/**
 * CONFIG custom types
 */

#ifndef FUJI_TYPEDEFS_H
#define FUJI_TYPEDEFS_H

#define FILE_MAXLEN 36

#define MODE_READ 1
#define MODE_WRITE 2

typedef unsigned short DirectoryPosition;

/**
 * Returned info for a single SSID entry
 * from a WiFi scan
 */
typedef union {
  struct
  {
    char ssid[32];
    signed char rssi;
  };
  unsigned char rawData[33];
} SSIDInfo;

/**
 * The current wifi SSID and password
 */
typedef union {
  struct
  {
    char ssid[32];
    char password[64];
  };
  unsigned char rawData[96];
} NetConfig;

/**
 * The current network adapter configuration
 */
typedef union {
  struct
  {
    char ssid[32];
    char hostname[64];
    unsigned char localIP[4];
    unsigned char gateway[4];
    unsigned char netmask[4];
    unsigned char dnsIP[4];
    unsigned char macAddress[6];
    unsigned char bssid[6];
    char fn_version[15];
  };
  unsigned char rawData[139];
} AdapterConfig;

typedef union {
    unsigned char host[8][32];
    unsigned char rawData[256];
} HostSlots;

typedef union {
    struct
    {
        unsigned char hostSlot;
        unsigned char mode;
        unsigned char file[FILE_MAXLEN];
    } slot[8];
    unsigned char rawData[304];
} DeviceSlots;

typedef union {
    struct
    {
        unsigned short numSectors;
        unsigned short sectorSize;
        unsigned char hostSlot;
        unsigned char deviceSlot;
        char filename[256];
    };
    unsigned char rawData[262];
} NewDisk;

#endif /* FUJI_TYPEDEFS_H */
