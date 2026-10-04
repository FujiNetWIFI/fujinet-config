/* constants.h -- the NES CONFIG's fixed numbers.
 *
 * The NES has 2K of internal RAM and, behind a FujiNet cartridge, 8K of WRAM
 * at $6000 that cc65's NES target already uses for DATA/BSS. That is roomy
 * next to the ColecoVision this is ported from, but the same discipline is
 * kept: lists are drawn straight out of the cartridge's reply window at
 * $5000 and streamed straight back into the next transaction, so a 1K reply
 * never has to be copied anywhere.
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

enum {
  ST_CHECK_WIFI = 0,
  ST_SET_WIFI,
  ST_CONNECT_WIFI,
  ST_HOSTS,
  ST_FILES,
  ST_INFO
};

#define HOST_SLOTS   8
#define HOST_STRIDE  32         /* READ_HOST_SLOTS: 8 x 32 bytes */

/* Screen geometry: cc65's conio gives 32 columns by 28 rows (NTSC). Menus
 * sit in a double frame on rows 2-25, cols 1-30 (fujidisp.h), leaving
 * Family BASIC's 28-column text area inside: row 3 the path, rows 5-20 the
 * list window, rows 22-23 status and legend. */
#define DISP_COLS    32
#define LIST_TOP     5
#define LIST_ROWS    16
#define NAMELEN      27         /* display width: col 3..29 */
#define FULLLEN      120        /* re-read width when building a full path */
#define PAYLOAD_LEN  256        /* the fixed buffer the ESP32 expects */
#define STATUS_ROW   22
#define LEGEND_ROW   23

/* The NES has exactly one thing to mount into: the cartridge. */
#define DEVICE_SLOT  0
#define MODE_READ    1

#define PATH_MAX_LEN 128        /* current directory, '/'-terminated */
#define SPEC_MAX_LEN 96         /* custom-SSID scratch */

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
