#ifdef BUILD_H89

/**
 * @file screen.c
 * @brief Screens - Heathkit H89 (H19 terminal).
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 *
 * Every screen is drawn whole into h19.c's buffer, which sends only what
 * differs from what the terminal already shows. Drawing the same screen
 * again (the hosts and devices loop does on every TAB) costs nothing on the
 * wire.
 */

#include <stdlib.h>
#include <string.h>
#include "../screen.h"
#include "../globals.h"
#include "../constants.h"
#include "../hosts_and_devices.h"
#include "h19.h"
#include "h89.h"
#include "layout.h"
#include "ui.h"

/* fujinet-lib's port layer, to drop a reply nobody asked for (below) */
extern int port_getc_timeout(uint16_t timeout);

extern char source_filename[];
extern NetConfig nc;

char _visibleEntries;

static bool status_stale = true;
static bool more_above, more_below;
static char buf[300];

static const char hints_hosts1[] =
  "[RET] Browse  [E] Edit  [1]-[8] Slot  [TAB] Drives  [C] Info & WiFi";
static const char hints_hosts2[] =
  "[ESC] Mount drives & exit  [Q] Quit";
static const char hints_drives1[] =
  "[E] Eject  [R] Read only  [W] Read/write  [ERASE] Eject all  [1]-[8] Slot";
static const char hints_drives2[] =
  "[TAB] Hosts  [C] Info & WiFi  [ESC] Mount drives & exit  [Q] Quit";
static const char hints_files1[] =
  "[RET] Open  [BS] Parent  [<] [>] Page  [F] Filter  [N] New disk  [C] Copy";
static const char hints_files2[] =
  "[ESC] Back to hosts";
static const char hints_copy1[] =
  "[RET] Open  [BS] Parent  [<] [>] Page  [F] Filter  [C] Copy here";
static const char hints_copy2[] =
  "[ESC] Cancel the copy";
static const char hints_edit[] =
  "[RET] Done  [ESC] Cancel  [BS] Rub out  [ERASE] Clear";

/** "host:path", shortened from the left to fit w */
static const char *host_path(const char *host, const char *p, uint8_t w)
{
  static uint16_t n;

  strcpy(buf, host);
  strcat(buf, ":");
  strncat(buf, p, sizeof(buf) - strlen(buf) - 1);
  n = strlen(buf);
  if (n <= w)
    return buf;
  buf[n - w] = '.';
  buf[n - w + 1] = '.';
  buf[n - w + 2] = '.';
  return &buf[n - w];
}

void screen_init(void)
{
  h19_init();
  atexit(screen_end);

  ui_frame();
  ui_divider(PANE_Y, NULL, NULL);
  ui_center(9, "Contacting FujiNet...");
  h19_flush();
}

void screen_end(void)
{
  h19_exit();
}

void screen_error(const char *s)
{
  ui_message(s);
}

/*
 * Hosts and devices
 */

static void host_row(uint8_t i)
{
  static uint8_t y;

  y = HOSTS_Y + i;
  ui_line(SLOT_X, y, NULL);
  h19_put(SLOT_X, y, '1' + i);
  h19_text(NAME_X, y, hostSlots[i][0] ? (const char *)hostSlots[i] : "Empty", 32, 0, false);
}

void screen_drive_row(uint8_t y, uint8_t ds)
{
  static DeviceSlot *d;

  d = &deviceSlots[ds];
  ui_line(SLOT_X, y, NULL);
  h19_put(SLOT_X, y, '1' + ds);
  if (d->hostSlot >= NUM_HOST_SLOTS || !d->file[0])
  {
    h19_text(FILE_X, y, "Empty", 5, 0, false);
    return;
  }
  h19_text(MODE_X, y, (d->mode & 0x03) == MODE_WRITE ? "R/W" : "R/O", 3, 0, false);
  h19_text(FILE_X, y, (const char *)d->file, FILE_W, 0, false);
  h19_text(DRIVE_HOST_X, y, (const char *)hostSlots[d->hostSlot], DRIVE_HOST_W, 0, false);
}

void screen_hosts_and_devices_host_slots(HostSlot *h)
{
  static uint8_t i;

  (void)h;
  for (i = 0; i < NUM_HOST_SLOTS; i++)
    host_row(i);
}

void screen_hosts_and_devices_device_slots(uint8_t y, DeviceSlot *d, const bool *e)
{
  static uint8_t i;

  (void)d;
  (void)e;
  for (i = 0; i < NUM_DEVICE_SLOTS; i++)
    screen_drive_row(y + i, i);
}

void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, bool *e)
{
  if (status_stale)
  {
    ui_status(true);
    status_stale = false;
  }

  ui_frame();
  ui_pane(PANE_Y, "Host Slots", false);
  screen_hosts_and_devices_host_slots(h);
  ui_pane(DRIVES_PANE_Y, "Drive Slots", false);
  screen_hosts_and_devices_device_slots(DRIVES_Y, d, e);
}

void screen_hosts_and_devices_hosts(void)
{
  ui_pane(PANE_Y, "Host Slots", true);
  ui_pane(DRIVES_PANE_Y, "Drive Slots", false);
  ui_info(NULL);
  ui_hints(hints_hosts1, hints_hosts2);
  bar_set(HOSTS_Y, BAR_X0, NUM_HOST_SLOTS, selected_host_slot);
}

void screen_hosts_and_devices_devices(void)
{
  ui_pane(PANE_Y, "Host Slots", false);
  ui_pane(DRIVES_PANE_Y, "Drive Slots", true);
  ui_info(NULL);
  ui_hints(hints_drives1, hints_drives2);
  bar_set(DRIVES_Y, BAR_X0, NUM_DEVICE_SLOTS, selected_device_slot);
}

void screen_hosts_and_devices_clear_host_slot(uint_fast8_t i)
{
  ui_line(SLOT_X, HOSTS_Y + i, NULL);
  h19_put(SLOT_X, HOSTS_Y + i, '1' + i);
}

void screen_edit_hints(void)
{
  ui_hints(hints_edit, NULL);
}

void screen_hosts_and_devices_edit_host_slot(uint_fast8_t i)
{
  (void)i;
  bar_clear(false);
  ui_info("Host name, or SD for the FujiNet's SD card:");
  screen_edit_hints();
}

void screen_hosts_and_devices_host_slot_empty(uint_fast8_t hs)
{
  host_row(hs);
}

void screen_hosts_and_devices_eject(uint8_t ds)
{
  screen_drive_row(DRIVES_Y + ds, ds);
}

void screen_hosts_and_devices_long_filename(const char *f)
{
  static DeviceSlot *d;

  d = &deviceSlots[selected_device_slot];
  if (d->hostSlot >= NUM_HOST_SLOTS || !d->file[0])
  {
    ui_info(NULL);
    return;
  }
  ui_info(host_path((const char *)hostSlots[d->hostSlot], f, TEXT_W));
}

void screen_drive_path(void)
{
  static DeviceSlot *d;

  d = &deviceSlots[selected_device_slot];
  if (d->hostSlot >= NUM_HOST_SLOTS || !d->file[0])
    ui_info(NULL);
  else
    hosts_and_devices_long_filename();
}

void screen_hosts_and_devices_devices_clear_all(void)
{
  ui_pane(DRIVES_PANE_Y, "Drive Slots", true);
  ui_hints("Please wait.", NULL);
  ui_message("Ejecting all drives...");
}

/*
 * Mount & Exit
 */

void screen_mount_and_boot(void)
{
  ui_frame();
  ui_pane(PANE_Y, "Host Slots", false);
  screen_hosts_and_devices_host_slots(hostSlots);
  ui_pane(DRIVES_PANE_Y, "Mounting Drives", true);
  screen_hosts_and_devices_device_slots(DRIVES_Y, deviceSlots, deviceEnabled);
  ui_info("Mounting the drives...");
  ui_hints(NULL, NULL);
  h19_flush();
}

void screen_mount_and_boot_slot(uint8_t ds, uint8_t state)
{
  static uint8_t y;

  y = DRIVES_Y + ds;
  h19_fill(70, y, 8, ' ');
  if (state == MOUNT_BUSY)
    h19_text(72, y, "....", 4, 0, false);
  else if (state == MOUNT_OK)
    h19_text(72, y, " OK ", 4, 0, false);
  else
    h19_text(72, y, "FAIL", 4, REV, false);
  h19_flush_rows(y, y);
}

void screen_mount_and_boot_done(uint8_t failed)
{
  if (!failed)
    return;
  ui_info("A drive could not be mounted.");
  ui_hints("Press any key to return to the OS.", NULL);
  h19_bell();
}

/*
 * Select file
 */

void screen_select_file(void)
{
  ui_frame();
  ui_pane(PANE_Y, selected_host_name, true);
  ui_hints(copy_mode ? hints_copy1 : hints_files1, copy_mode ? hints_copy2 : hints_files2);
  ui_message("Opening the host...");
}

void screen_select_file_display(char *p, char *f)
{
  static uint8_t y;

  /* The path is shortened to leave the filter room */
  ui_pane(PANE_Y, host_path(selected_host_name, p, (f && f[0]) ? 36 : 70), true);
  if (f && f[0])
  {
    strcpy(buf, "Filter: ");
    strncat(buf, f, 31);
    ui_title_right(PANE_Y, buf);
  }
  ui_divider(FOOT_Y, NULL, NULL);

  for (y = LIST_Y; y < LIST_Y + LIST_ROWS; y++)
    ui_line(ENTRY_X, y, NULL);

  more_above = more_below = false;
  bar_clear(false);
  h19_cursor(H19_NONE, 0);

  if (copy_mode)
  {
    strcpy(buf, "Copy ");
    strncat(buf, source_filename, 40);
    strcat(buf, ": open the folder for it, then press C.");
    ui_info(buf);
  }
  else
    ui_info(NULL);
  ui_hints(copy_mode ? hints_copy1 : hints_files1, copy_mode ? hints_copy2 : hints_files2);

  h19_flush_rows(0, PANE_Y);
}

void screen_select_file_display_entry(uint8_t y, const char *e, uint16_t entryType)
{
  (void)entryType;
  h19_text(ENTRY_X, LIST_Y + y, e, ENTRY_W, 0, true);
  /* Each entry is a FujiNet round trip, so show them as they come */
  h19_flush_rows(0, LIST_Y + y);
}

void screen_select_file_next(void)
{
  more_below = true;
}

void screen_select_file_prev(void)
{
  more_above = true;
}

void screen_select_file_choose(char visibleEntries)
{
  _visibleEntries = visibleEntries;

  ui_divider(FOOT_Y, more_above ? "< previous page" : NULL, more_below ? "next page >" : NULL);

  if (visibleEntries == 0)
  {
    h19_text(ENTRY_X, LIST_Y, "Nothing here.", ENTRY_W, 0, true);
    bar_clear(false);
    return;
  }
  bar_set(LIST_Y, BAR_X0, visibleEntries, prev_page ? visibleEntries - 1 : 0);
}

void screen_select_file_display_long_filename(const char *e)
{
  (void)e;
}

void screen_select_file_clear_long_filename(void)
{
}

void screen_select_file_filter(void)
{
  bar_clear(false);
  ui_prompt("Show only names matching:");
  screen_edit_hints();
}

void screen_select_file_new_type(void)
{
}

void screen_select_file_new_size(uint8_t k)
{
  (void)k;
  bar_clear(false);
  ui_info("New disk image: how big?");
  ui_hints("[1] 100K  [2] 400K  [3] 720K  [4] 1.44M  [C] Custom", "[ESC] Cancel");
}

void screen_select_file_new_custom(void)
{
  ui_prompt("Number of 512-byte sectors:");
  screen_edit_hints();
}

void screen_select_file_new_name(void)
{
  ui_prompt("Name of the new disk image:");
  screen_edit_hints();
}

void screen_select_file_new_creating(void)
{
  ui_message("Creating the disk image...");
}

/*
 * Select slot
 */

static char *two(uint8_t n, char *p)
{
  *p++ = '0' + n / 10;
  *p++ = '0' + n % 10;
  return p;
}

void screen_select_slot(const char *e)
{
  static const uint8_t *u;
  static uint32_t size;
  static char *p;
  static char num[12];

  ui_frame();
  ui_pane(PANE_Y, create ? "New Disk Image" : "Mount Disk Image", true);
  ui_labelled(LIST_Y, "Host", selected_host_name);

  if (create)
  {
    ui_labelled(LIST_Y + 1, "New image", path);
  }
  else
  {
    /* The entry has its details ahead of its name: a timestamp (years
       from 1970), the size, flags and the media type */
    u = (const uint8_t *)e;
    ui_labelled(LIST_Y + 1, "Folder", path);
    ui_labelled(LIST_Y + 2, "File", e + 12);

    size = (uint32_t)u[6] | ((uint32_t)u[7] << 8) | ((uint32_t)u[8] << 16) | ((uint32_t)u[9] << 24);
    if (size >= 10240UL)
    {
      strcpy(buf, ui_num((size + 1023) >> 10, num));
      strcat(buf, "K");
    }
    else
    {
      strcpy(buf, ui_num(size, num));
      strcat(buf, " bytes");
    }
    ui_labelled(LIST_Y + 3, "Size", buf);

    p = ui_num(1970 + u[0], buf) + 4;
    *p++ = '-';
    p = two(u[1], p);
    *p++ = '-';
    p = two(u[2], p);
    *p++ = ' ';
    p = two(u[3], p);
    *p++ = ':';
    p = two(u[4], p);
    *p = 0;
    ui_labelled(LIST_Y + 4, "Date", buf);
  }

  ui_pane(DRIVES_PANE_Y, "Drive Slots", true);
  screen_hosts_and_devices_device_slots(DRIVES_Y, deviceSlots, deviceEnabled);
  ui_info("Put it in which drive?");
  ui_hints(create ? "[RET] Create it here  [E] Eject  [1]-[8] Slot"
                  : "[RET] Mount read only  [W] Mount read/write  [E] Eject  [1]-[8] Slot",
           "[ESC] Cancel");
  bar_set(DRIVES_Y, BAR_X0, NUM_DEVICE_SLOTS, selected_device_slot);
}

void screen_select_slot_choose(void)
{
}

void screen_select_slot_eject(uint8_t ds)
{
  screen_drive_row(DRIVES_Y + ds, ds);
}

/*
 * Copy
 */

void screen_destination_host_slot(char *h, char *p)
{
  ui_frame();
  ui_pane(PANE_Y, "Copy to which host?", true);
  ui_pane(DRIVES_PANE_Y, "Copying", false);
  ui_labelled(DRIVES_Y, "From host", h);
  ui_labelled(DRIVES_Y + 1, "File", p);
  ui_info(NULL);
  ui_hints("[RET] Choose  [1]-[8] Slot", "[ESC] Cancel the copy");
}

void screen_destination_host_slot_choose(void)
{
  bar_set(HOSTS_Y, BAR_X0, NUM_HOST_SLOTS, selected_host_slot);
}

void screen_perform_copy(char *sh, char *p, char *dh, char *dp)
{
  ui_frame();
  ui_pane(PANE_Y, "Copying a File", true);
  ui_labelled(LIST_Y + 2, "From", host_path(sh, p, 60));
  ui_labelled(LIST_Y + 4, "To", host_path(dh, dp, 60));
  ui_hints(NULL, NULL);
  ui_message("Copying... this can take a while.");
}

/*
 * WiFi
 */

void screen_set_wifi_extended(AdapterConfigExtended *acx)
{
  ui_status(false);
  ui_frame();
  ui_pane(PANE_Y, "WiFi Networks", true);
  strcpy(buf, "MAC ");
  strcat(buf, acx->sMacAddress);
  ui_title_right(PANE_Y, buf);
  ui_hints(NULL, NULL);
  ui_message("Looking for networks...");
}

void screen_set_wifi_display_ssid(char n, SSIDInfo *s)
{
  static uint8_t y, level, i;
  static const uint8_t meter[4] = { G_BAR_BOTTOM, G_QUAD_LR, G_HALF_RIGHT, ' ' | REV };
  static char num[8];

  if ((uint8_t)n >= LIST_ROWS - 1)
    return;
  y = LIST_Y + n;

  ui_line(ENTRY_X, y, NULL);
  h19_text(ENTRY_X, y, s->ssid, 32, 0, false);

  /* Signal strength as rising bars, then the dBm */
  if (s->rssi > -55)
    level = 4;
  else if (s->rssi > -67)
    level = 3;
  else if (s->rssi > -78)
    level = 2;
  else
    level = 1;
  for (i = 0; i < 4; i++)
    h19_put(64 + i, y, i < level ? meter[i] : ' ');
  num[0] = '-';
  ui_num(s->rssi < 0 ? -s->rssi : s->rssi, &num[1]);
  strcat(num, " dBm");
  h19_text(70, y, num, 8, 0, false);

  h19_flush_rows(y, y);
}

void screen_set_wifi_select_network(uint8_t nn)
{
  ui_line(ENTRY_X, LIST_Y + nn, "<Enter a network name>");
  ui_info(nn ? "Choose a network." : "No networks found.");
  ui_hints("[RET] Choose  [R] Look again", "[ESC] Skip");
  bar_set(LIST_Y, BAR_X0, nn + 1, 0);
}

void screen_set_wifi_custom(void)
{
  bar_clear(false);
  ui_prompt("Network name:");
  screen_edit_hints();
}

void screen_set_wifi_password(void)
{
  bar_clear(false);
  strcpy(buf, "Password for ");
  strncat(buf, nc.ssid, 32);
  strcat(buf, ":");
  ui_prompt(buf);
  screen_edit_hints();
}

void screen_connect_wifi(NetConfig *nc)
{
  /* A failed SET SSID can leave a second NAK on its way from the FujiNet;
     taken now, it can't answer the next call */
  while (port_getc_timeout(10) >= 0)
    ;

  status_stale = true;
  ui_frame();
  ui_pane(PANE_Y, "Connecting to WiFi", true);
  ui_center(8, "Connecting to");
  ui_center(10, nc->ssid);
  ui_info(NULL);
  ui_hints("[ESC] Stop", NULL);
  h19_flush();
}

void screen_show_info_extended(bool printerEnabled, AdapterConfigExtended *acx)
{
  (void)printerEnabled;

  ui_status(false);
  ui_frame();
  ui_pane(PANE_Y, "FujiNet", true);
  ui_labelled(LIST_Y + 1, "SSID", acx->ssid);
  ui_labelled(LIST_Y + 2, "Hostname", acx->hostname);
  ui_labelled(LIST_Y + 3, "IP address", acx->sLocalIP);
  ui_labelled(LIST_Y + 4, "Gateway", acx->sGateway);
  ui_labelled(LIST_Y + 5, "DNS", acx->sDnsIP);
  ui_labelled(LIST_Y + 6, "Netmask", acx->sNetmask);
  ui_labelled(LIST_Y + 7, "MAC", acx->sMacAddress);
  ui_labelled(LIST_Y + 8, "BSSID", acx->sBssid);
  ui_labelled(LIST_Y + 9, "Firmware", acx->fn_version);
  ui_labelled(LIST_Y + 11, "CONFIG", GIT_VERSION);
#ifdef FNLIB_VERSION_FULL
  ui_labelled(LIST_Y + 12, "fujinet-lib", FNLIB_VERSION_FULL);
#endif
  ui_info(NULL);
  ui_hints("[S] Change WiFi network  [R] Reconnect", "Any other key: back to the hosts");
}

#endif /* BUILD_H89 */
