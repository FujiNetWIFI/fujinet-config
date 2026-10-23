/**
 * @file model2.h
 * @brief What the Model II screens, input and mounting share among themselves.
 */

#ifndef MODEL2_MODEL2_H
#define MODEL2_MODEL2_H

#include <stdint.h>
#include <stdbool.h>

/* A drive's state while mounting, in its line's right end */
#define MOUNT_BUSY 0
#define MOUNT_OK   1
#define MOUNT_FAIL 2

/** Draw drive slot ds on line y */
void screen_drive_row(uint8_t y, uint8_t ds);

/** Show drive ds's state while Mount & Exit mounts it */
void screen_mount_and_boot_slot(uint8_t ds, uint8_t state);

/** Mount & Exit is done: wait for a key if a drive failed */
void screen_mount_and_boot_done(uint8_t failed);

/** Show the selected drive's full path on the info line */
void screen_drive_path(void);

/** The hints for editing a line of text */
void screen_edit_hints(void);

#endif /* MODEL2_MODEL2_H */
