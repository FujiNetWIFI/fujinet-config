#include "typedefs.h"

extern void system_boot(void);
extern void system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                              uint32_t selected_size, const char *path);
extern void system_build_directory(unsigned char ds, unsigned long numBlocks, char *v);

#ifdef _CMOC_VERSION_
#define MAX_WIFI_NETWORKS 11
#elif defined(BUILD_MSDOS)
#define MAX_WIFI_NETWORKS 18
#else
#define MAX_WIFI_NETWORKS 16
#endif

#ifdef BUILD_COCO
extern void system_enable_user_rom(void);
extern bool system_slot0_is_rom(void);
#endif /* BUILD_COCO */

#ifdef BUILD_APPLE2
extern void system_list_devs(void);
#endif /* BUILD_APPLE2 */

#ifdef BUILD_ATARI
extern void cold_start(void);
extern void rtclr(void);
#endif /* BUILD_ATARI */

#ifdef BUILD_MSDOS
#include <stdint.h>

typedef struct {
    uint8_t query;
    char    signature[4];
    uint8_t unit;
} fuji_ioctl_query;

#define FUJI_SIGNATURE "FUJI"

int  find_drive_letter(int drive);
char system_find_drive_letter_for_slot(uint8_t device_slot);
void system_refresh_drive_letters(void);

void install_tsr_now(void);
void system_load_tsr_setting(void);
void system_save_tsr_setting(void);

extern char deviceDriveLetters[8];

#endif /* BUILD_MSDOS */
