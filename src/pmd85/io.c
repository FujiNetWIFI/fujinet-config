#ifdef BUILD_PMD85
/**
 * FujiNet CONFIG for #Adam
 *
 * I/O Routines
 */

#include <conio.h> // for sleep() 
#include <stdlib.h>
#include <string.h>
#include "io.h"

#include "dwread.h"
#include "dwwrite.h"

#define OP_FUJI 0xE2
#define CMD_READY 0x00
#define CMD_RESPONSE 0x01

// #define MOCK_WIFI
// #define MOCK_DEVICES
// #define MOCK_HOSTS

#define FUJINET_TIMEOUT 15000

typedef enum {
    FUJINET_RC_OK,
    FUJINET_RC_NOT_IMPLEMENTED,
    FUJINET_RC_NOT_SUPPORTED,
    FUJINET_RC_INVALID,
    FUJINET_RC_TIMEOUT,
    FUJINET_RC_NO_ACK,
    FUJINET_RC_NO_COMPLETE,
} FUJINET_RC;

extern NetConfig nc;
static AdapterConfigExtended adapterConfig;
static SSIDInfo ssidInfo;
uint8_t response[256];
FUJINET_RC last_rc = FUJINET_RC_OK;


#ifdef MOCK_WIFI
// AdapterConfig mock_cfg = {
//         "mock wifi test",
//         "localhost",
//         { 192, 168, 0, 100 },
//         { 192, 168, 0, 1 },
//         { 255, 255, 255, 0 },
//         { 8, 8, 8, 8 },
//         { 1, 2, 3, 4, 5, 6 },
//         { 81, 82, 83, 84, 85, 86 },
//         "v0.0-pmd85mock"
// };
AdapterConfigExtended mock_cfg = {
        "mock wifi test",
        "localhost",
        { 192, 168, 0, 100 },
        { 192, 168, 0, 1 },
        { 255, 255, 255, 0 },
        { 8, 8, 8, 8 },
        { 1, 2, 3, 4, 5, 6 },
        { 81, 82, 83, 84, 85, 86 },
        "v0.0-pmd85mock",
        "192.168.0.100",
        "192.168.0.1",
        "255.255.255.0",
        "8.8.8.8",
        "01:02:03:04:05:06",
        "81:82:83:84:85:86"
};

SSIDInfo mock_ssid[] = {
        { "mock wifi test", -30 },
        { "wifi 2", -50 },
        { "ssid 1", -60 },
        { "Wifi 3", -80 }
};
int mock_ssid_num = 4;

NetConfig mock_netconfig = {
        "my-test-wifi", "password"
};
#endif

#ifdef MOCK_DEVICES
DeviceSlot mock_devices[8] = {
        { 0, 1, "TEST.DSK" },
        { 1, 0, {} },
        { 2, 0, {} },
        { 3, 0, {} },
        { 4, 0, {} },
        { 5, 0, {} },
        { 6, 0, {} },
        { 7, 0, {} },
};
#endif

#ifdef MOCK_HOSTS
HostSlot mock_hosts[8] = {
        { "SD" },
        { "fujinet.test.offline" },
        { "pmd85-apps.irata.online" },
        { {} },
        { {} },
        { {} },
        { {} },
        { {} }
};
#endif

#ifdef MOCK_HOSTS
#define MOCK_FILES_NUM 40
static unsigned short mock_files = 0;
#endif

static void io_command_and_response(void* buf, unsigned short len)
{
}

void io_init(void)
{
  // fujinet_init();
}

// unsigned char io_status(void)
// {
//   return last_rc;
// }

// bool io_error(void)
// {
//   return last_rc != FUJINET_RC_OK;
// }

/**
 * @brief wait for FujiNet device ready
 */
void io_ready(void)
{
    struct _readycmd
    {
        unsigned char opcode;
        unsigned char command;
    } rc;

    unsigned char z=0, r=0;
    
    rc.opcode = OP_FUJI;
    rc.command = CMD_READY;
    
    while (!z)
    {
        dwwrite((char *)&rc,sizeof(rc));
        z = dwread((char *)&r,sizeof(r));
    }
}

/**
 * @brief Get response data from last command
 * @param devid The device ID (0-255) 
 * @param buf Target buffer 
 * @param len Length 
 */
unsigned char io_get_response(char *buf, unsigned short len)
{
    struct _getresponsecmd
    {
        unsigned char opcode;
        unsigned char command;
    } grc;

    unsigned char z=0;
    
    grc.opcode = OP_FUJI;
    grc.command = CMD_RESPONSE;

    io_ready();
    dwwrite((char *)&grc, sizeof(grc));
    dwread(buf, len);
    
    return z;
}

bool io_error(void)
{
  return false;
}

unsigned char io_status(void)
{
  return 0;
}

bool io_get_wifi_enabled(void)
{
#ifdef MOCK_WIFI
  return true;
#else
  bool r=0;

  struct _get_wifi_enabled
  {
      unsigned char opcode;
      unsigned char cmd;
  } gwec;

  gwec.opcode = OP_FUJI;
  gwec.cmd = 0xEA;

  io_ready();
  dwwrite((char *)&gwec, sizeof(gwec));

  //io_ready();
  io_get_response((char *)&r,1);

  return r;
#endif
}

unsigned char io_get_wifi_status(void)
{
#ifdef MOCK_WIFI
  last_rc = FUJINET_RC_OK;
  return 3;
#else
  unsigned char r;

  struct _get_wifi_status
  {
      unsigned char opcode;
      unsigned char cmd;
  } gwsc;

  gwsc.opcode = OP_FUJI;
  gwsc.cmd = 0xFA;

  //io_ready();
  dwwrite((char *)&gwsc, sizeof(gwsc));

  io_ready();
  io_get_response((char *)&r, 1);
    
  return r;
#endif
}

NetConfig* io_get_ssid(void)
{
#ifdef MOCK_WIFI
  last_rc = FUJINET_RC_OK;
  return mock_netconfig;
#else
  struct _get_ssid
  {
      unsigned char opcode;
      unsigned char cmd;
  } gsc;

  gsc.opcode = OP_FUJI;
  gsc.cmd = 0xFE;

  io_ready();
  dwwrite((char *)&gsc,sizeof(gsc));

  //io_ready();
  io_get_response((char *)&nc, sizeof(nc));
    
  return &nc;
#endif
}

unsigned char io_scan_for_networks(void)
{
#ifdef MOCK_WIFI
  sleep(1);
  last_rc = FUJINET_RC_OK;
  return mock_ssid_num;
#else
  char r = -1;

  struct _scan_for_networks
  {
      unsigned char opcode;
      unsigned char cmd;
  } sfnc;

  sfnc.opcode = OP_FUJI;
  sfnc.cmd = 0xFD;

  io_ready();
  dwwrite((char *)&sfnc, sizeof(sfnc));

  //io_ready();
  io_get_response((char *)&r,1);
  
  return r;
#endif
}

SSIDInfo *io_get_scan_result(unsigned char n)
{
#ifdef MOCK_WIFI
  last_rc = FUJINET_RC_OK;
  return &mock_ssid[n];
#else
    struct _get_scan_result
    {
        unsigned char opcode;
        unsigned char cmd;
        unsigned char n;
    } gsrc;

    gsrc.opcode = OP_FUJI;
    gsrc.cmd = 0xFC;
    gsrc.n = n;

    io_ready();
    dwwrite((char *)&gsrc,sizeof(gsrc));

    //io_ready();
    io_get_response((char *)&ssidInfo, sizeof(SSIDInfo));
    return &ssidInfo;
#endif
}

AdapterConfigExtended *io_get_adapter_config(void)
{
#ifdef MOCK_WIFI
  last_rc = FUJINET_RC_OK;
  return mock_cfg;
#else
    // Wait for adapter ready
    io_ready();
                  
    // Send Get Adapter Config command
    dwwrite("\xE2\xC4", 2);

    // // Check again for adapter ready
    // io_ready();

    // Retrieve response.
    io_get_response((char *)&adapterConfig, sizeof(adapterConfig));
    
    return &adapterConfig;
#endif
}

int io_set_ssid(NetConfig *nc)
{
#ifdef MOCK_WIFI
  memcpy(&mock_netconfig, nc, sizeof(NetConfig));
  memcpy(&mock_cfg.ssid, nc->ssid, sizeof(mock_cfg.ssid));
  last_rc = FUJINET_RC_OK;
#else
  struct _setssid
  {
    char fuji;
    char cmd;
    char ssid[33];
    char password[64];
  } s;

  s.fuji=0xE2;
  s.cmd=0xFB;
  memcpy(s.ssid,nc->ssid,33);
  memcpy(s.password,nc->password,64);

  dwwrite((char *)&s,sizeof(s));
#endif
  return 0;
}

void io_get_device_slots(DeviceSlot *d)
{
#ifdef MOCK_DEVICES
  memcpy(d,mock_devices,sizeof(DeviceSlot) * 8);
  last_rc = FUJINET_RC_OK;
#else
    struct _get_device_slots
    {
        unsigned char opcode;
        unsigned char cmd;
    } gdsc;

    gdsc.opcode = OP_FUJI;
    gdsc.cmd = 0xF2;

    io_ready();
    dwwrite((char *)&gdsc, sizeof(gdsc));

    //io_ready();
    io_get_response((char *)d, 152);
#endif
  for (int i = 0; i < 8; i++)
    d[i].file[FILE_MAXLEN-1] = '\0';
}

void io_get_host_slots(HostSlot *h)
{
#ifdef MOCK_HOSTS
  memcpy(h,mock_hosts,sizeof(HostSlot) * 8);
  last_rc = FUJINET_RC_OK;
#else
    struct _get_host_slots
    {
        unsigned char opcode;
        unsigned char cmd;
    } ghsc;

    ghsc.opcode = OP_FUJI;
    ghsc.cmd = 0xF4;

    //io_ready();
    dwwrite((char *)&ghsc, sizeof(ghsc));

    io_ready();
    io_get_response((char *)h, 256);
#endif
}

void io_put_host_slots(HostSlot *h)
{
#ifdef MOCK_HOSTS
  memcpy(mock_hosts, h, sizeof(HostSlot) * 8);
  last_rc = FUJINET_RC_OK;
#else
    io_ready();
    dwwrite((char *)"\xE2\xF3",2);
    dwwrite((char *)h,256);
#endif
}

void io_put_device_slots(DeviceSlot *d)
{
#ifdef MOCK_DEVICES
  memcpy(mock_devices, d, sizeof(DeviceSlot) * 8);
  last_rc = FUJINET_RC_OK;
#else
    io_ready();
    dwwrite((char *)"\xE2\xF1",2);
    dwwrite((char *)d,304);
#endif
}

void io_mount_host_slot(unsigned char hs)
{
#ifdef MOCK_HOSTS
  /* do nothing */
  last_rc = FUJINET_RC_OK;
#else
    io_ready();
    dwwrite((char *)"\xE2\xF9",2);
    dwwrite((char *)&hs,1);
#endif
}

void io_open_directory(unsigned char hs, char *p, char *f)
{
#ifdef MOCK_HOSTS
  /* do nothing */
  last_rc = FUJINET_RC_OK;
  mock_files = 0;
#else
    struct _open_directory
    {
        unsigned char opcode;
        unsigned char cmd;
        unsigned char hs;
        char p[256];
    } odc;

    odc.opcode = OP_FUJI;
    odc.cmd = 0xF7;
    odc.hs = hs;

    memset(odc.p,0,sizeof(odc.p));
    strcpy(odc.p,p);
    strcpy(&odc.p[strlen(odc.p)+1],f);

    io_ready();
    dwwrite((char *)&odc, sizeof(odc));

    //io_ready();
#endif
}

char *io_read_directory(unsigned char maxlen, unsigned char a)
{
  struct _additl_info
	{
		char year;
		unsigned char month;
		unsigned char day;
		unsigned char hour;
		unsigned char min;
		unsigned char sec;
		unsigned long size;
		//unsigned char flags;
    unsigned char isdir;
    unsigned char trunc;
		unsigned char type;
		char filename[0];
	} *info = (struct _additl_info *)response;

#ifdef MOCK_HOSTS

  char *fn = response;
  if (a & 0x80)
    fn = &info->filename[0];

  if (mock_files < MOCK_FILES_NUM)
  {
    mock_files++;
    if (a & 0x80) 
    {
      info->year = 84 + mock_files;
      info->month = 8;
      info->day = 11;
      info->hour = 17;
      info->min = 15;
      info->sec = 4;
      info->size = ((unsigned long)(mock_files) << 12) | mock_files;
      //info->flags = 0;
      info->isdir = 0;
      info->trunc = 0;
      info->type = 0;
    }
    // test some long file names
    if (mock_files >= 20 && mock_files <= 25)
    {
      unsigned char ll = 2*mock_files;
      memset(fn, mock_files + ('A' - 20), ll);
      fn[ll] = '\0';
      if (ll > maxlen)
      {
        fn[maxlen] = '\0';
        if (a &0x80)
          info->trunc |= 0x02;
      }
    }
    else
    {
      sprintf(fn, "mock-%d.mrm", mock_files);
    }
  }
  else
    sprintf(response, "\x7F\x7F"); // end of dir

  last_rc = FUJINET_RC_OK;
#else
    struct _read_directory
    {
        unsigned char opcode;
        unsigned char cmd;
        unsigned char maxlen;
        unsigned char a;
    } rdc;

    if (a)
        maxlen += 10;

    rdc.opcode = OP_FUJI;
    rdc.cmd = 0xF6;
    rdc.maxlen = maxlen;
    rdc.a = a;
  
  memset(response,0,sizeof(response));

  io_ready();
  dwwrite((char *)&rdc, sizeof(rdc));

  //io_ready();
  io_get_response(response, maxlen);
  if (a & 0x80)
  {
    // big to little endian (CoCo is big-endian)
    char *szptr = &info->size;
    char tmp = szptr[0];
    szptr[0] = szptr[3]; szptr[3] = tmp;
    tmp = szptr[1];
    szptr[1] = szptr[2]; szptr[2] = tmp;
  }
#endif

  return response;
}

void io_close_directory(void)
{
#ifdef MOCK_HOSTS
  /* do nothing */
  last_rc = FUJINET_RC_OK;
  mock_files = 0;
#else
    io_ready();
    dwwrite((char *)"\xE2\xF5",2);
#endif
}

void io_set_directory_position(DirectoryPosition pos)
{
#ifdef MOCK_HOSTS
  mock_files = pos;
  last_rc = FUJINET_RC_OK;
#else
    struct _set_directory_position
    {
        unsigned char opcode;
        unsigned char cmd;
        unsigned char pos_hi;
        unsigned char pos_lo;
    } sdpc;

    sdpc.opcode = OP_FUJI;
    sdpc.cmd = 0xE4;
    sdpc.pos_hi = pos >> 8;
    sdpc.pos_lo = pos & 0xFF;

    io_ready();
    dwwrite((char *)&sdpc,sizeof(sdpc));
    //_dirpos = pos;
#endif
}

void io_set_device_filename(unsigned char ds, char* e)
{
#ifdef MOCK_DEVICES
  /* do nothing */
  last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
  char fn[256];

  strcpy(fn,e);

  io_ready();
  dwwrite((char *)"\xE2\xE2",2);
  dwwrite((char *)&ds,1);
  dwwrite((char *)&selected_host_slot,1);
  dwwrite((char *)&mode,1); 
  dwwrite((char *)fn,256);
#endif
}

char *io_get_device_filename(unsigned char slot)
{
#ifdef MOCK_DEVICES
  /* do nothing */
  last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    struct _get_device_filename
    {
        unsigned char opcode;
        unsigned char cmd;
        unsigned char slot;
    } gdfc;

    gdfc.opcode = OP_FUJI;
    gdfc.cmd = 0xDA;
    gdfc.slot = slot;

    io_ready();
    dwwrite((char *)&gdfc, sizeof(gdfc));

    //io_ready();
    memset(response,0,sizeof(response));
    io_get_response(response, 256);
#endif

  return response;
}

void io_create_new(unsigned char selected_host_slot,unsigned char selected_device_slot,unsigned long selected_size,char *path)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    // last_rc = fujinet_create_new(selected_host_slot, selected_device_slot, selected_size, path);
#endif
}

void io_mount_disk_image(unsigned char ds, unsigned char mode)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    io_ready();
    dwwrite((char *)"\xE2\xF8",2);
    dwwrite((char *)&ds,1);
    dwwrite((char *)&mode,1);
    io_ready(); // make sure mount completed (before reboot)
#endif
}

void io_set_boot_config(unsigned char toggle)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    // last_rc = fujinet_set_boot_config(toggle);
#endif
}

void io_umount_disk_image(unsigned char ds)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    io_ready();
    dwwrite((char *)"\xE2\xE9",2);
    dwwrite((char *)&ds,1);
#endif
}

void io_boot(void)
{
#asm
    jmp 0x8000
#endasm
}

// The enabled status area needs visiting. this requires a LOT of status codes to send to FN, as there's no payload to specify which device.
// And in FN it always returns true anyway, so this functionality is pretty broken at the moment.
bool io_get_device_enabled_status(unsigned char d)
{
#ifdef MOCK_DEVICES
    last_rc = FUJINET_RC_OK;
    return true;
#else
    //last_rc = fujinet_get_device_enabled_status(d + RC2014_DEVICEID_DISK, response);
    //return response[0];
    return true;
#endif
}

void io_enable_device(unsigned char d)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    // last_rc = fujinet_enable_device(d + RC2014_DEVICEID_DISK);
#endif
}

void io_disable_device(unsigned char d)
{
#ifdef MOCK_DEVICES
    /* do nothing */
    last_rc = FUJINET_RC_NOT_IMPLEMENTED;
#else
    // last_rc = fujinet_disable_device(d + RC2014_DEVICEID_DISK);
#endif
}

void io_update_devices_enabled(bool *e)
{
  char i;

  for (i=0;i<4;i++)
  {
    e[i]=io_get_device_enabled_status(io_device_slot_to_device(i));
  }
}

void io_copy_file(unsigned char source_slot, unsigned char destination_slot)
{
  char cf[259]={0xD8,0x00,0x00};
  
  cf[1]=source_slot;
  cf[2]=destination_slot;
  strcpy(&cf[3],copySpec);
  
//  eos_write_character_device(FUJI_DEV,cf,sizeof(cf));
  last_rc = FUJINET_RC_NOT_IMPLEMENTED;
}

unsigned char io_device_slot_to_device(unsigned char ds)
{
	return ds;
}

#endif /* BUILD_PMD85 */
