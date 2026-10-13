/* constants.h -- the Atari 5200 CONFIG's fixed numbers.
 *
 * The NES CONFIG's discipline is kept: lists are drawn straight out of the
 * cartridge's reply window at $B000 and streamed straight back into the
 * next transaction, so a 1K reply never has to be copied anywhere.
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

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

/* Screen geometry: 40 x 24. Menus sit in a double frame on rows 0-23,
 * cols 1-38 (fujidisp.h), with a 36-column text area inside: row 1 the
 * path, rows 3-18 the list window, rows 20-21 status and legend. */
#define LIST_TOP     3
#define LIST_ROWS    16
#define PATH_ROW     1
#define NAMELEN      35         /* display width: col 3..37 */
#define FULLLEN      120        /* re-read width when building a full path */
#define STATUS_ROW   20
#define LEGEND_ROW   21

/* The 5200 has exactly one thing to mount into: the cartridge. */
#define DEVICE_SLOT  0
#define MODE_READ    1

/* The FujiNet Game Lobby: the hosts screen's ninth row boots it. */
#define LOBBY_HOST   "ec.tnfs.io"
#define LOBBY_PATH   "/atari5200/lobby.a52"

#define PATH_MAX_LEN 128        /* current directory, '/'-terminated */
#define SPEC_MAX_LEN 128        /* copy source full path / custom-SSID scratch */

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

#endif /* CONSTANTS_H */
