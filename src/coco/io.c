#ifdef _CMOC_VERSION_
/**
 * FujiNet Drivewire calls
 */

#include <cmoc.h>
#include "stdbool.h"
#include "io.h"
#include "globals.h"
#include "screen.h"
#include "bar.h"

#define OP_FUJI 0xE2
#define CMD_READY 0x00
#define CMD_RESPONSE 0x01

extern NetConfig nc;
static AdapterConfig adapterConfig;
static SSIDInfo ssidInfo;
NewDisk newDisk;
unsigned char wifiEnabled=true;
byte response[256];
int _dirpos=0;
byte orig_casflag;

/**
 * @brief Read string to s from DriveWire with expected length l
 * @param s pointer to string buffer
 * @param l expected length of string (0-65535 bytes)
 * @return 1 = read successful, 0 = not successful
 */
byte dwread(byte *s, int l)
{
  asm
    {
    pshs x,y
    ldx :s
    ldy :l
    jsr [0xD93F]
    puls y,x
    tfr cc,b
    lsrb
    lsrb
    andb #$01
    }
}

/**
 * @brief Write string at s to DriveWire with length l
 * @param s pointer to string buffer
 * @param l length of string (0-65535 bytes)
 * @return # of bytes actually written
 */
int dwwrite(byte *s, int l)
{
  asm
    {
        pshs x,y
        ldx :s
	ldy :l
        jsr [0xD941]
	tfr cc,d
	puls y,x
    }
}

/**
 * @brief wait for FujiNet device ready
 */
void io_ready(void)
{
    struct _readycmd
    {
        byte opcode;
        byte command;
    } rc;

    byte z=0, r=0;
    
    rc.opcode = OP_FUJI;
    rc.command = CMD_READY;
    
    while (!z)
    {
        dwwrite((byte *)&rc,sizeof(rc));
        z = dwread((byte *)&r,sizeof(r));
    }
}

/**
 * @brief Get response data from last command
 * @param devid The device ID (0-255) 
 * @param buf Target buffer 
 * @param len Length 
 */
byte io_get_response(byte *buf, int len)
{
    struct _getresponsecmd
    {
        byte opcode;
        byte command;
    } grc;

    byte z=0;
    
    grc.opcode = OP_FUJI;
    grc.command = CMD_RESPONSE;

    io_ready();
    dwwrite((byte *)&grc, sizeof(grc));
    dwread((byte *)buf, len);
    
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

void io_init(void)
{
	// There's no partnering exit function for screen_init, so we'll set up
	//   how casing is being handled here.  We default to lowercase.
	asm {
		lda $011A
		sta orig_casflag
		clr $011A
	}
}

bool io_get_wifi_enabled(void)
{
  bool r=0;

  struct _get_wifi_enabled
  {
      byte opcode;
      byte cmd;
  } gwec;

  gwec.opcode = OP_FUJI;
  gwec.cmd = 0xEA;
  
  io_ready();
  dwwrite((byte *)&gwec, sizeof(gwec));

  io_ready();
  io_get_response(&r,1);
  
  return r;
}

unsigned char io_get_wifi_status(void)
{
  byte r;

  struct _get_wifi_status
  {
      byte opcode;
      byte cmd;
  } gwsc;

  gwsc.opcode = OP_FUJI;
  gwsc.cmd = 0xFA;

  io_ready();
  dwwrite((byte *)&gwsc, sizeof(gwsc));

  io_ready();
  io_get_response(&r, 1);
    
  return r;
}

NetConfig *io_get_ssid(void)
{
  struct _get_ssid
  {
      byte opcode;
      byte cmd;
  } gsc;

  gsc.opcode = OP_FUJI;
  gsc.cmd = 0xFE;

  io_ready();
  dwwrite((byte *)&gsc,sizeof(gsc));

  io_ready();
  io_get_response((byte *)&nc, sizeof(nc));
    
  return &nc;
}

unsigned char io_scan_for_networks(void)
{
  char r=-1;

  struct _scan_for_networks
  {
      byte opcode;
      byte cmd;
  } sfnc;

  sfnc.opcode = OP_FUJI;
  sfnc.cmd = 0xFD;

  io_ready();
  dwwrite((byte *)&sfnc, sizeof(sfnc));

  io_ready();
  io_get_response((byte *)&r,1);
  
  if (r > 11)
    r=11;
  
  return r;
}

SSIDInfo *io_get_scan_result(int n)
{
    struct _get_scan_result
    {
        byte opcode;
        byte cmd;
        byte n;
    } gsrc;

    gsrc.opcode = OP_FUJI;
    gsrc.cmd = 0xFC;
    gsrc.n = (byte)n;

    io_ready();
    dwwrite((byte *)&gsrc,sizeof(gsrc));

    io_ready();
    io_get_response((byte *)&ssidInfo, sizeof(SSIDInfo));
    
    return &ssidInfo;
}

AdapterConfig *io_get_adapter_config(void)
{
    // Wait for adapter ready
    io_ready();
                  
    // Send Get Adapter Config command
    dwwrite((byte *)"\xE2\xE8",2);

    // Check again for adapter ready
    io_ready();

    // Retrieve response.
    io_get_response((byte *)&adapterConfig,sizeof(AdapterConfig));
    
    return &adapterConfig;
}

int io_set_ssid(NetConfig *nc)
{
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

  dwwrite((byte *)&s,sizeof(s));
  
  return false;
}

void io_get_device_slots(DeviceSlot *d)
{
    struct _get_device_slots
    {
        byte opcode;
        byte cmd;
    } gdsc;

    gdsc.opcode = OP_FUJI;
    gdsc.cmd = 0xF2;

    io_ready();
    dwwrite((byte *)&gdsc, sizeof(gdsc));

    io_ready();
    io_get_response((byte *)d, 152);    
}

void io_get_host_slots(HostSlot *h)
{
    struct _get_host_slots
    {
        byte opcode;
        byte cmd;
    } ghsc;

    ghsc.opcode = OP_FUJI;
    ghsc.cmd = 0xF4;

    io_ready();
    dwwrite((byte *)&ghsc, sizeof(ghsc));

    io_ready();
    io_get_response(h, 256);
}

void io_put_host_slots(HostSlot *h)
{
    io_ready();
    dwwrite((byte *)"\xE2\xF3",2);
    dwwrite((byte *)h,256);
}

void io_put_device_slots(DeviceSlot *d)
{
    io_ready();
    dwwrite((byte *)"\xE2\xF1",2);
    dwwrite((byte *)d,304);
}

void io_mount_host_slot(unsigned char hs)
{
    io_ready();
    dwwrite((byte *)"\xE2\xF9",2);
    dwwrite(&hs,1);
}

void io_open_directory(unsigned char hs, char *p, char *f)
{
    struct _open_directory
    {
        byte opcode;
        byte cmd;
        byte hs;
        char p[256];
    } odc;

    odc.opcode = OP_FUJI;
    odc.cmd = 0xF7;
    odc.hs = hs;

    memset(odc.p,0,sizeof(odc.p));
    strcpy(odc.p,p);
    strcpy(&odc.p[strlen(odc.p)+1],filter);

    io_ready();
    dwwrite((byte *)&odc, sizeof(odc));

    io_ready();
}

const char *io_read_directory(unsigned char maxlen, unsigned char a)
{
    struct _read_directory
    {
        byte opcode;
        byte cmd;
        byte maxlen;
        byte a;
    } rdc;

    if (a)
        maxlen += 10;

    rdc.opcode = OP_FUJI;
    rdc.cmd = 0xF6;
    rdc.maxlen = maxlen;
    rdc.a = a;
  
  memset(response,0,sizeof(response));

  io_ready();
  dwwrite((byte *)&rdc, sizeof(rdc));

  io_ready();
  io_get_response(response, maxlen);
    
  return (const char *)response;
}

void io_close_directory(void)
{
    io_ready();
    dwwrite((byte *)"\xE2\xF5",2);
}

void io_set_directory_position(DirectoryPosition pos)
{
    io_ready();
    dwwrite((byte *)"\xE2\xE4",2);
    dwwrite((byte *)&pos,sizeof(pos));
    _dirpos = pos;
}

void io_set_device_filename(unsigned char ds, char* e)
{
  char fn[256];

  strcpy(fn,e);

  io_ready();
  dwwrite((byte *)"\xE2\xE2",2);
  dwwrite((byte *)&ds,1);
  dwwrite((byte *)&selected_host_slot,1);
  dwwrite((byte *)&mode,1); 
  dwwrite((byte *)fn,256);
}

const char *io_get_device_filename(unsigned char slot)
{
    struct _get_device_filename
    {
        byte opcode;
        byte cmd;
        byte slot;
    } gdfc;

    gdfc.opcode = OP_FUJI;
    gdfc.cmd = 0xDA;
    gdfc.slot = slot;

    io_ready();
    dwwrite((byte *)&gdfc, sizeof(gdfc));

    io_ready();
    memset(response,0,sizeof(response));
    io_get_response(response, 256);
    
    return (const char *)response;
}

void io_set_boot_config(unsigned char toggle)
{
}

void io_set_boot_mode(unsigned char mode)
{
}


void io_mount_disk_image(unsigned char ds, unsigned char mode)
{
    io_ready();
    dwwrite((byte *)"\xE2\xF8",2);
    dwwrite((byte *)&ds,1);
    dwwrite((byte *)&mode,1);
}

void io_umount_disk_image(unsigned char ds)
{
    io_ready();
    dwwrite((byte *)"\xE2\xE9",2);
    dwwrite((byte *)&ds,1);
}

void io_boot(void)
{
	// Restore the original casing flag.
	asm {
		lda orig_casflag
		sta $011A
	}
	exit(0);
}

void io_create_new(unsigned char selected_host_slot, unsigned char selected_device_slot, unsigned long selected_size, char *path)
{
    struct _newdisk
    {
        byte fuji;
        byte cmd;
        byte numDisks;
        byte host_slot;
        byte device_slot;
        char filename[256];
    } nd;

    nd.fuji = 0xE2;
    nd.cmd = 0xE7;
    nd.numDisks = (byte)selected_size;
    nd.host_slot = selected_host_slot;
    nd.device_slot = selected_device_slot;
    strcpy(nd.filename,path);

    io_ready();

    dwwrite((byte *)&nd,sizeof(nd));
    io_ready(); // Temporary, make new disk status command!
}

void io_build_directory(unsigned char ds, unsigned long numBlocks, char *v)
{
}

bool io_get_device_enabled_status(unsigned char d)
{
  return false;
}

void io_update_devices_enabled(bool *e)
{
}

void io_enable_device(unsigned char d)
{
}

void io_disable_device(unsigned char d)
{
}

/**
 * NOTE: On the Fuji, command 0xD8 (copy file) expects the slots to be 1-8, not 0-7 like most things.
 * That's why we add one, since config is tracking the slots as 0-7
 */
void io_copy_file(unsigned char source_slot, unsigned char destination_slot)
{
}

unsigned char io_device_slot_to_device(unsigned char ds)
{
  return 0;
}

/**
 * Get filename for device slot
 */
void io_get_filename_for_device_slot(unsigned char slot, const char *filename)
{
}

/**
 * Mount all hosts and devices
 */
unsigned char io_mount_all(void)
{
  return 1;
}

#endif /* _CMOC_VERSION_ */
