/* fujiraw.h -- the transactions fujinet-lib's call shapes cannot express on a
 * 1K machine, built on the lib's own exported mailbox primitives.
 *
 * fuji_bus_call() takes its payload as one contiguous buffer and copies its
 * reply into another; both are the right shape everywhere RAM is plentiful.
 * Here, four commands would each need a 256-byte staging buffer the console
 * does not have -- so these stream their payloads byte by byte through the
 * lib's FN_TOUCH/fn_regwr, pulling from the reply window (host slots,
 * filenames) and from bounded RAM strings, and pad on the fly. Everything
 * else in this program goes through fujinet-lib proper.
 */

#ifndef FUJIRAW_H
#define FUJIRAW_H

#include <stdbool.h>

/* OPEN_DIRECTORY: "path\0filter\0" NUL-padded to exactly 256 bytes --
 * fujiDevice splits at the first NUL and treats the rest as the pattern.
 * (fuji_open_directory_filter() in the lib does the same job through a
 * static 256-byte buffer; do not link it here.) */
bool fnraw_open_directory(unsigned char host_slot,
                          const char *dirpath, const char *pattern);

/* WRITE_HOST_SLOTS takes no parameters and one 256-byte payload -- all eight
 * 32-byte slots, every time. Re-reads the slots and streams them straight
 * back out of the reply window with slot `slot` replaced by `name`. */
bool fnraw_write_host_slot(unsigned char slot, const char *name);

/* SET_DEVICE_FULLPATH with the filename streamed from the reply window
 * (`name` points into it), prefixed by the RAM-resident directory path. */
bool fnraw_set_device_path_from_reply(unsigned char dev, unsigned char host_slot,
                                      unsigned char mode, const char *prefix,
                                      volatile unsigned char *name);

/* SET_DEVICE_FULLPATH from a plain string (the lobby's fixed path). */
bool fnraw_set_device_path(unsigned char dev, unsigned char host_slot,
                           unsigned char mode, const char *fullpath);

/* SET_SSID: nparam >= 1 (value ignored) and exactly ssid[33]+password[64]. */
bool fnraw_set_ssid(const char *ssid, const char *password);

/* COPY_FILE: "sourcefullpath|destdir+basename(source)" at its EXACT length --
 * no NUL, no padding; the firmware reads dataAsString(), and padding would
 * bury NULs inside the destination filename. Slots are 1-BASED, unlike every
 * other command (the firmware rejects 0 and then decrements). Waits out the
 * cartridge's 60-second budget: the ACK arrives only when the host-side copy
 * is done. */
bool fnraw_copy_file(unsigned char src_slot1, unsigned char dst_slot1,
                     const char *source_spec, const char *dest_dir);

#endif /* FUJIRAW_H */
