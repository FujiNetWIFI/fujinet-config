/* fujiraw.h -- the transactions fujinet-lib's buffer-shaped calls do not
 * fit, built on the lib's own exported mailbox primitives.
 *
 * fuji_bus_call() takes its payload as one contiguous buffer. Four commands
 * here want a 256-byte payload assembled from pieces -- a RAM path prefix
 * plus a filename that only exists in the cartridge's reply window, or all
 * eight host slots with one replaced -- so these stream their payloads byte
 * by byte through the lib's fn_tx()/fn_regwr() and pad on the fly.
 */

#ifndef FUJIRAW_H
#define FUJIRAW_H

#include <stdbool.h>

/* OPEN_DIRECTORY: "path\0filter\0" NUL-padded to exactly 256 bytes. */
bool fnraw_open_directory(unsigned char host_slot,
                          const char *dirpath, const char *pattern);

/* WRITE_HOST_SLOTS takes one 256-byte payload -- all eight 32-byte slots.
 * Re-reads the slots and streams them straight back out of the reply window
 * with slot `slot` replaced by `name`. */
bool fnraw_write_host_slot(unsigned char slot, const char *name);

/* SET_DEVICE_FULLPATH with the filename streamed from the reply window
 * (`name` points into it), prefixed by the RAM-resident directory path. */
bool fnraw_set_device_path_from_reply(unsigned char dev, unsigned char host_slot,
                                      unsigned char mode, const char *prefix,
                                      volatile unsigned char *name);

/* SET_SSID: nparam >= 1 (value ignored) and exactly ssid[33]+password[64]. */
bool fnraw_set_ssid(const char *ssid, const char *password);

#endif /* FUJIRAW_H */
