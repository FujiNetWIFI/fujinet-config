/**
 * MSX device slot layout
 *
 * The MSX cartridge can present at most two ROM images and four disk
 * images at a time, so the device list is sized to what is actually
 * mountable rather than to the eight slots the firmware stores. The list
 * is kept contiguous: four disk rows are always shown, and every mounted
 * ROM adds one more row on top of them, growing the list to five and then
 * six. Ejecting a ROM pulls every following slot up one position so the
 * list shrinks back down without leaving a hole behind.
 */

#ifndef MSX_DEVICE_SLOTS_H
#define MSX_DEVICE_SLOTS_H

#include <stdbool.h>
#include <stdint.h>

#define MSX_NUM_DISK_SLOTS 4
#define MSX_MAX_ROM_SLOTS  2
#define MSX_MAX_SLOTS      (MSX_NUM_DISK_SLOTS + MSX_MAX_ROM_SLOTS)

/* Returned when there is no selectable slot in the requested direction. */
#define MSX_NO_SLOT        0xFF

/**
 * Is this device slot filename a ROM image?
 */
bool msx_device_slot_is_rom(const char *filename);

/**
 * Number of device slots to display and allow selection of, which is
 * MSX_NUM_DISK_SLOTS plus one row per mounted ROM.
 * @return slot count, between MSX_NUM_DISK_SLOTS and MSX_MAX_SLOTS
 */
uint8_t msx_visible_device_slots(void);

/**
 * Pull every device slot after ds up one position, clearing the last
 * slot. Called after ejecting a ROM, whose row is going away.
 * @param ds slot that was just emptied
 */
void msx_compact_device_slots(uint8_t ds);

/**
 * Record what kind of image the slot picker is about to place, so it can
 * offer somewhere to put a ROM.
 * @param is_rom true when a ROM image is being mounted
 */
void msx_set_mount_is_rom(bool is_rom);

/**
 * Number of slots the slot picker should offer. This is
 * msx_visible_device_slots(), plus one spare row when a ROM is being
 * mounted, there is still room for another ROM, and every visible slot is
 * already taken - without the spare there would be nowhere to put it.
 * @return slot count, at most MSX_MAX_SLOTS
 */
uint8_t msx_mount_target_slots(void);

/**
 * Is the picker limited to replacing one of the ROMs already mounted?
 * True when a ROM is being mounted and both ROM slots are in use.
 */
bool msx_mount_replaces_rom(void);

/**
 * Can the image being mounted go into this slot? Anywhere in the list,
 * unless a third ROM is being mounted, in which case only the slots
 * holding the existing ROMs will take it.
 * @param ds slot to test
 */
bool msx_mount_slot_allowed(uint8_t ds);

/**
 * Slot the picker should start on.
 * @return lowest selectable slot
 */
uint8_t msx_first_mount_slot(void);

/**
 * Next selectable slot in a direction, skipping any that the image
 * cannot go into.
 * @param ds slot to move from
 * @param down true to move down the list, false to move up
 * @return the new slot, or MSX_NO_SLOT if there is none that way
 */
uint8_t msx_next_mount_slot(uint8_t ds, bool down);

#endif /* MSX_DEVICE_SLOTS_H */
