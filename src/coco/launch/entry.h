/* Handoff between the apps; sits just above their --limit in the Makefile. */
struct entry_data
{
  unsigned char magic[2];
  unsigned char entry;
  unsigned char host_slot;
  unsigned char device_slot;
  unsigned char joystick;    /* JoySide; survives entry_take(), cleared by cfgload */
};

#define ENTRY_DATA ((struct entry_data *)0x7BF8)

/* The CoCo 3 builds are WIFI3, MAIN3 and FILES3 on disk. */
#ifdef COCO3
#define APP(name) name "3"
#else
#define APP(name) name
#endif

typedef enum
{
  JOY_NONE,
  JOY_LEFT,
  JOY_RIGHT
} JoySide;

#define ENTRY_NONE  0
#define ENTRY_INFO  1
#define ENTRY_FILES 2
#define ENTRY_HOSTS 3

/* An eject or mode change in slot 0 takes CONFIG's disk out of drive 0. Remounting loads the
   firmware's built-in CONFIG, so only do it after one of those. */
#define remount_config_disk() do { if (config_disk_gone) fuji_set_boot_mode(FUJI_BOOT_MODE_CONFIG); } while (0)

void entry_set(unsigned char entry, unsigned char host_slot, unsigned char device_slot);

/* Returns the pending entry code (ENTRY_NONE if none) and clears it. */
unsigned char entry_take(void);
