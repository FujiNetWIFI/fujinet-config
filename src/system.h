#include "typedefs.h"

extern void system_boot(void);
extern void system_create_new(uint8_t selected_host_slot, uint8_t selected_device_slot,
                              uint32_t selected_size, const char *path);

#ifdef _CMOC_VERSION_
#define MAX_WIFI_NETWORKS 11
#else /* ! _CMOC_VERSION_ */
#define MAX_WIFI_NETWORKS 16
#endif /* _CMOC_VERSION_ */

#ifdef BUILD_APPLE2
extern void system_list_devs(void);
#endif /* BUILD_APPLE2 */
