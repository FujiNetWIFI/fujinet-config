/**
 * @file trs80m1.h
 * @brief What the Model I screens, input and mounting share among themselves.
 */

#ifndef TRS80M1_TRS80M1_H
#define TRS80M1_TRS80M1_H

#include <stdint.h>
#include <stdbool.h>

/* A drive's state while mounting, in its line's right end */
#define MOUNT_BUSY 0
#define MOUNT_OK   1
#define MOUNT_FAIL 2

/** Slot 1 is mounted: the ROM can boot from drive 0 */
extern bool boot_ready;

/** Draw drive slot drive on line row */
void screen_drive_row(uint8_t row, uint8_t drive);

/** Show the state of a drive while Mount & Boot mounts it */
void screen_mount_and_boot_slot(uint8_t drive, uint8_t state);

/** Mount & Boot is done: say what comes next */
void screen_mount_and_boot_done(uint8_t failed);

/** Show the selected drive's full path on the info line */
void screen_drive_path(void);

/** The hints for editing a line of text */
void screen_edit_hints(void);

/** The card's settings screen, until BREAK or CLEAR */
void card_settings(void);

/** Draw the hosts and drives screen again, after a screen of our own */
void screen_hosts_and_devices_again(void);

#endif /* TRS80M1_TRS80M1_H */
