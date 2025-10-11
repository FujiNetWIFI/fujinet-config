#ifdef BUILD_ATARI
// Y position on screen where the device slots start.
#define DEVICES_START_Y 13
// Y position on screen where the device slots end.
#define DEVICES_END_Y   (DEVICES_START_Y + NUM_DEVICE_SLOTS - 1)

// Y position of the start of the file list from host.
#define FILES_START_Y   6

// Y position on screen where the list of hosts starts.
#define HOSTS_START_Y 2
// Y position on screen where the list of hosts ends
#define HOSTS_END_Y (HOSTS_START_Y + NUM_HOST_SLOTS - 1)

// Y position on screen where the list of available wireless networks start.
#define NETWORKS_START_Y    4

// When mounting an image the device list/drive slots are on the screen by themselves at a different location than when the hosts are also shown.
#define DEVICES_START_MOUNT_Y   3
#define DEVICES_END_MOUNT_Y (DEVICES_START_MOUNT_Y + NUM_DEVICE_SLOTS-1)

#define COLOR_SETTING_FAILED 0x33
#define COLOR_SETTING_SUCCESSFUL 0xB4

#endif /* BUILD_ATARI */

