/* constants.h -- the ColecoVision CONFIG's fixed numbers.
 *
 * RAM RULES (the machine has 1K, of which ~700 bytes are usable once OS7's
 * tables and the stack are paid for -- the analog of intv/constants.bas's
 * preamble):
 *
 *   - NEVER buffer tabular or list data: host slots, directory pages, scan
 *     results and the adapter config are drawn straight out of the cartridge's
 *     reply window at $F800 and streamed straight back into the next
 *     transaction's payload. `path` and `src_spec` below are the only
 *     reply-to-RAM copies in the program, and both are bounded.
 *   - No arrays as locals, no printf family. The stack is ~250 bytes.
 *   - The browser's filter has no buffer of its own: it IS fn_entry (the
 *     on-screen keyboard's edit buffer) for as long as ST_FILES is on screen.
 *     That aliasing is safe because the rename and WiFi editors are not
 *     reachable from inside the browser; if that ever changes, give the
 *     filter its own bytes.
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

/* States. Mirrors the shape of src/typedefs.h's State enum the way
 * intv/constants.bas does -- same flow, fewer stops: SELECT_SLOT collapses
 * into "mount into device slot 0 and boot" (one device slot), and
 * DESTINATION_HOST_SLOT folds into ST_HOSTS's copy livery. */
enum {
  ST_CHECK_WIFI = 0,
  ST_SET_WIFI,
  ST_CONNECT_WIFI,
  ST_HOSTS,
  ST_FILES,
  ST_INFO,
  ST_LOBBY
};

#define HOST_SLOTS   8
#define HOST_STRIDE  32         /* READ_HOST_SLOTS: 8 x 32 bytes */

/* Screen geometry: 32x24 Graphics 1. Row 1 title, row 3 subtitle, rows 5-20
 * the list window, rows 22-23 the keypad legend (the smartkeys replacement --
 * also the status/error line). */
#define LIST_TOP     5
#define LIST_ROWS    16
#define NAMELEN      29         /* display width: col 2..30 */
#define FULLLEN      120        /* re-read width when building a full path */
#define PAYLOAD_LEN  256        /* the fixed buffer the ESP32 expects */
#define STATUS_ROW   22
#define LEGEND_ROW   23

/* The ColecoVision has exactly one thing to mount into: the cartridge. */
#define DEVICE_SLOT  0
#define MODE_READ    1

#define PATH_MAX_LEN 96         /* current directory, '/'-terminated */
#define SPEC_MAX_LEN 96         /* copy source full path / custom-SSID scratch */

#define WIFI_MAX     15         /* networks shown; row 16 is always OTHER */
#define SSID_LEN     33         /* SSIDInfo/NetConfig: char ssid[33] */
#define PASS_MAX     63
#define FILTER_MAX   30

/* AdapterConfigExtended field offsets, from
 * fujinet-firmware/lib/device/fujiDevice/fujiDevice.h. The 240-byte struct
 * never leaves the reply window; these index into it. */
#define ACX_SSID     0
#define ACX_HOSTNAME 33
#define ACX_VERSION  125
#define ACX_SLOCALIP 140
#define ACX_SGATEWAY 156
#define ACX_SNETMASK 172
#define ACX_SDNSIP   188
#define ACX_SMAC     204
#define ACX_SBSSID   222

/* Keypad 0 on the hosts screen: the FujiNet Game Lobby, same policy as
 * intv/st_lobby.bas (find ec.tnfs.io among the slots or claim the last one). */
#define LOBBY_HOST "ec.tnfs.io"
#define LOBBY_PATH "/coleco/lobby.rom"

#endif /* CONSTANTS_H */
