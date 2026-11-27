#ifdef BUILD_TRS80M1

/**
 * @file screen.c
 * @brief Screens - TRS-80 Model I.
 *
 * Every screen is drawn whole into vid.c's buffer, which writes only what
 * differs from what is already on the screen. Lists are 64 columns by 10
 * lines, so hosts and drives take turns in the same place.
 */

#include <stdlib.h>
#include <string.h>
#include "../screen.h"
#include "../globals.h"
#include "../constants.h"
#include "../hosts_and_devices.h"
#include "vid.h"
#include "trs80m1.h"
#include "con.h"
#include "layout.h"
#include "ui.h"

/* fujinet-lib's port layer, to drop a reply nobody asked for (below) */
extern int port_getc_timeout(uint16_t timeout) __z88dk_fastcall;

extern char source_filename[];
extern NetConfig nc;

char _visibleEntries;

static bool status_stale = true;
static bool drives_shown;
static bool more_above, more_below;
static char buf[300];

static const char hints_hosts1[] = "ENTER Browse  E Edit  1-8 Slot  RIGHT Drives";
static const char hints_hosts2[] = "C Info/WiFi  K Card  BREAK Mount and boot";
static const char hints_drives1[] = "E Eject  R Read only  W Read/write  X Eject all";
static const char hints_drives2[] = "LEFT Hosts  C Info/WiFi  K Card  BREAK Boot";
static const char hints_files1[] = "ENTER Open  LEFT Up  < > Page  F Filter  N New  C Copy";
static const char hints_files2[] = "BREAK Back to the hosts";
static const char hints_copy1[] = "ENTER Open  LEFT Up  < > Page  F Filter  C Copy here";
static const char hints_copy2[] = "BREAK Cancel the copy";
static const char hints_edit[] = "ENTER Done  BREAK Cancel  LEFT Rub out";

/** "host:path", shortened from the left to fit w */
static const char *host_path(const char *host, const char *name, uint8_t width)
{
  static uint16_t len;

  strcpy(buf, host);
  strcat(buf, ":");
  strncat(buf, name, sizeof(buf) - strlen(buf) - 1);
  len = strlen(buf);
  if (len <= width)
    return buf;
  buf[len - width] = '.';
  buf[len - width + 1] = '.';
  buf[len - width + 2] = '.';
  return &buf[len - width];
}

void screen_init(void)
{
  con_di();
  vid_init();

  ui_frame();
  ui_pane(PANE_Y, "FujiNet");
  ui_center(8, "Contacting FujiNet...");
  vid_flush();
}

void screen_end(void)
{
}

void screen_error(const char *message)
{
  ui_message(message);
}

/*
 * Hosts and devices
 */

static void host_row(uint8_t slot)
{
  static uint8_t row;

  row = HOSTS_Y + slot;
  ui_line(SLOT_X, row, NULL);
  vid_put(SLOT_X, row, '1' + slot);
  vid_text(NAME_X, row, hostSlots[slot][0] ? (const char *)hostSlots[slot] : "Empty", 32, false);
}

void screen_drive_row(uint8_t row, uint8_t drive)
{
  static DeviceSlot *dev;

  dev = &deviceSlots[drive];
  ui_line(SLOT_X, row, NULL);
  vid_put(SLOT_X, row, '1' + drive);
  if (dev->hostSlot >= FUJI_MAX_HOSTS || !dev->file[0])
  {
    vid_text(MODE_X, row, "Empty", 5, false);
    return;
  }
  vid_text(MODE_X, row, (dev->mode & (FUJI_DISK_MODE_READ | FUJI_DISK_MODE_WRITE)) == FUJI_DISK_MODE_WRITE ? "R/W" : "R/O", 3, false);
  vid_text(FILE_X, row, (const char *)dev->file, FILE_W, false);
  vid_text(DRIVE_HOST_X, row, (const char *)hostSlots[dev->hostSlot], DRIVE_HOST_W, false);
}

void screen_hosts_and_devices_host_slots(HostSlot *hosts)
{
  static uint8_t slot;

  (void)hosts;
  for (slot = 0; slot < FUJI_MAX_HOSTS; slot++)
    host_row(slot);
}

void screen_hosts_and_devices_device_slots(uint8_t row, DeviceSlot *devs, const bool *enabled)
{
  static uint8_t slot;

  (void)devs;
  (void)enabled;
  for (slot = 0; slot < FUJI_MAX_DISKS; slot++)
    screen_drive_row(row + slot, slot);
}

void screen_hosts_and_devices(HostSlot *hosts, DeviceSlot *devs, bool *enabled)
{
  (void)hosts;
  (void)devs;
  (void)enabled;
  if (status_stale)
  {
    ui_status(true);
    status_stale = false;
  }

  ui_frame();
}

void screen_hosts_and_devices_hosts(void)
{
  drives_shown = false;
  ui_pane(PANE_Y, "Host Slots");
  screen_hosts_and_devices_host_slots(hostSlots);
  ui_info(NULL);
  ui_hints(hints_hosts1, hints_hosts2);
  bar_set(HOSTS_Y, BAR_X0, FUJI_MAX_HOSTS, selected_host_slot);
}

void screen_hosts_and_devices_devices(void)
{
  drives_shown = true;
  ui_pane(PANE_Y, "Drive Slots");
  screen_hosts_and_devices_device_slots(DRIVES_Y, deviceSlots, deviceEnabled);
  ui_info(NULL);
  ui_hints(hints_drives1, hints_drives2);
  bar_set(DRIVES_Y, BAR_X0, FUJI_MAX_DISKS, selected_device_slot);
}

void screen_hosts_and_devices_again(void)
{
  screen_hosts_and_devices(hostSlots, deviceSlots, deviceEnabled);
  if (drives_shown)
  {
    screen_hosts_and_devices_devices();
    screen_drive_path();
  }
  else
    screen_hosts_and_devices_hosts();
}

void screen_hosts_and_devices_clear_host_slot(uint_fast8_t slot)
{
  ui_line(SLOT_X, HOSTS_Y + slot, NULL);
  vid_put(SLOT_X, HOSTS_Y + slot, '1' + slot);
}

void screen_edit_hints(void)
{
  ui_hints(hints_edit, NULL);
}

void screen_hosts_and_devices_edit_host_slot(uint_fast8_t slot)
{
  (void)slot;
  bar_clear(false);
  ui_info("Host name, or SD for the FujiNet's SD card:");
  screen_edit_hints();
}

void screen_hosts_and_devices_host_slot_empty(uint_fast8_t slot)
{
  host_row(slot);
}

void screen_hosts_and_devices_eject(uint8_t drive)
{
  screen_drive_row(DRIVES_Y + drive, drive);
}

void screen_hosts_and_devices_long_filename(const char *name)
{
  static DeviceSlot *dev;

  dev = &deviceSlots[selected_device_slot];
  if (dev->hostSlot >= FUJI_MAX_HOSTS || !dev->file[0])
  {
    ui_info(NULL);
    return;
  }
  ui_info(host_path((const char *)hostSlots[dev->hostSlot], name, TEXT_W));
}

void screen_drive_path(void)
{
  static DeviceSlot *dev;

  dev = &deviceSlots[selected_device_slot];
  if (dev->hostSlot >= FUJI_MAX_HOSTS || !dev->file[0])
    ui_info(NULL);
  else
    hosts_and_devices_long_filename();
}

void screen_hosts_and_devices_devices_clear_all(void)
{
  ui_hints("Please wait.", NULL);
  ui_message("Ejecting all drives...");
}

/*
 * Mount & Boot
 */

void screen_mount_and_boot(void)
{
  ui_frame();
  ui_pane(PANE_Y, "Mounting Drives");
  screen_hosts_and_devices_device_slots(DRIVES_Y, deviceSlots, deviceEnabled);
  ui_info("Mounting the drives...");
  ui_hints(NULL, NULL);
  vid_flush();
}

void screen_mount_and_boot_slot(uint8_t drive, uint8_t state)
{
  static uint8_t row;

  row = DRIVES_Y + drive;
  vid_fill(STATE_X - 1, row, VID_COLS - STATE_X + 1, ' ');
  if (state == MOUNT_BUSY)
    vid_text(STATE_X, row, "...", 3, false);
  else if (state == MOUNT_OK)
    vid_text(STATE_X, row, "OK", 2, false);
  else
    vid_text(STATE_X, row, "FAIL", 4, false);
  vid_flush_rows(row, row);
}

void screen_mount_and_boot_done(uint8_t failed)
{
  if (failed)
  {
    ui_info(boot_ready ? "A drive could not be mounted." : "Slot 1 could not be mounted.");
    ui_hints(boot_ready ? "Press any key to boot." : "Press any key to go back.", NULL);
    return;
  }
  ui_info("Booting from slot 1...");
  vid_flush();
}

/*
 * Select file
 */

void screen_select_file(void)
{
  ui_frame();
  ui_pane(PANE_Y, selected_host_name);
  ui_hints(copy_mode ? hints_copy1 : hints_files1, copy_mode ? hints_copy2 : hints_files2);
  ui_message("Opening the host...");
}

void screen_select_file_display(char *dir, char *filter_text)
{
  static uint8_t row;

  /* The path is shortened to leave the filter room */
  ui_pane(PANE_Y, host_path(selected_host_name, dir, (filter_text && filter_text[0]) ? 30 : 56));
  if (filter_text && filter_text[0])
  {
    strcpy(buf, "Filter: ");
    strncat(buf, filter_text, 20);
    ui_title_right(PANE_Y, buf);
  }
  ui_divider(FOOT_Y, NULL, NULL);

  for (row = LIST_Y; row < LIST_Y + LIST_ROWS; row++)
    ui_line(ENTRY_X, row, NULL);

  more_above = more_below = false;
  bar_clear(false);
  vid_cursor(VID_NONE, 0);

  if (copy_mode)
  {
    strcpy(buf, "Copy ");
    strncat(buf, source_filename, 24);
    strcat(buf, ": open its folder, press C.");
    ui_info(buf);
  }
  else
    ui_info(NULL);
  ui_hints(copy_mode ? hints_copy1 : hints_files1, copy_mode ? hints_copy2 : hints_files2);

  vid_flush();
}

void screen_select_file_display_entry(uint8_t line, const char *entry, uint16_t entryType)
{
  (void)entryType;
  vid_text(ENTRY_X, LIST_Y + line, entry, ENTRY_W, true);
  /* Each entry is a FujiNet round trip, so show them as they come */
  vid_flush_rows(0, LIST_Y + line);
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
    vid_text(ENTRY_X, LIST_Y, "Nothing here.", ENTRY_W, true);
    bar_clear(false);
    return;
  }
  bar_set(LIST_Y, BAR_X0, visibleEntries, prev_page ? visibleEntries - 1 : 0);
}

void screen_select_file_display_long_filename(const char *entry)
{
  (void)entry;
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

void screen_select_file_new_size(uint8_t size_key)
{
  (void)size_key;
  bar_clear(false);
}

void screen_select_file_new_custom(void)
{
}

void screen_select_file_new_name(void)
{
  ui_prompt("New JV1 disk (.dsk), name:");
  screen_edit_hints();
}

void screen_select_file_new_creating(void)
{
  ui_message("Creating the disk image...");
}

/*
 * Select slot
 */

static char *two(uint8_t num, char *out)
{
  *out++ = '0' + num / 10;
  *out++ = '0' + num % 10;
  return out;
}

void screen_select_slot(const char *entry_data)
{
  static const FujiDirEntry *entry;
  static char *out;
  static char num[12];

  ui_frame();
  ui_pane(PANE_Y, host_path(selected_host_name, path, 56));

  if (create)
  {
    ui_labelled(LIST_Y, "New disk", path);
    ui_line(TEXT_X, LIST_Y + 1, NULL);
  }
  else
  {
    entry = (const FujiDirEntry *)entry_data;
    ui_labelled(LIST_Y, "File", entry->filename);

    if (entry->size >= 10240UL)
    {
      strcpy(buf, ui_num((entry->size + 1023) >> 10, num));
      strcat(buf, "K");
    }
    else
    {
      strcpy(buf, ui_num(entry->size, num));
      strcat(buf, " bytes");
    }
    strcat(buf, "  ");

    /* Years from 1970 */
    out = ui_num(1970 + entry->modified.year, num);
    strcat(buf, out);
    out = &buf[strlen(buf)];
    *out++ = '-';
    out = two(entry->modified.month, out);
    *out++ = '-';
    out = two(entry->modified.day, out);
    *out = 0;
    ui_labelled(LIST_Y + 1, "Size", buf);
  }

  screen_hosts_and_devices_device_slots(PICK_DRIVES_Y, deviceSlots, deviceEnabled);
  ui_info("Put it in which drive?");
  ui_hints(create ? "ENTER Create here  E Eject  1-8 Slot"
                  : "ENTER Mount R/O  W Mount R/W  E Eject  1-8 Slot",
           "BREAK Cancel");
  bar_set(PICK_DRIVES_Y, BAR_X0, FUJI_MAX_DISKS, selected_device_slot);
}

void screen_select_slot_choose(void)
{
}

void screen_select_slot_eject(uint8_t drive)
{
  screen_drive_row(PICK_DRIVES_Y + drive, drive);
}

/*
 * Copy
 */

void screen_destination_host_slot(char *host, char *name)
{
  ui_frame();
  ui_pane(PANE_Y, "Copy to which host?");
  screen_hosts_and_devices_host_slots(hostSlots);
  ui_info(host_path(host, name, TEXT_W));
  ui_hints("ENTER Choose  1-8 Slot", "BREAK Cancel the copy");
}

void screen_destination_host_slot_choose(void)
{
  bar_set(HOSTS_Y, BAR_X0, FUJI_MAX_HOSTS, selected_host_slot);
}

void screen_perform_copy(char *src_host, char *src_path, char *dest_host, char *dest_path)
{
  ui_frame();
  ui_pane(PANE_Y, "Copying a File");
  ui_labelled(LIST_Y + 2, "From", host_path(src_host, src_path, 50));
  ui_labelled(LIST_Y + 4, "To", host_path(dest_host, dest_path, 50));
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
  ui_pane(PANE_Y, "WiFi Networks");
  strcpy(buf, "MAC ");
  strcat(buf, acx->sMacAddress);
  ui_title_right(PANE_Y, buf);
  ui_hints(NULL, NULL);
  ui_message("Looking for networks...");
}

void screen_set_wifi_display_ssid(char idx, SSIDInfo *ssid_info)
{
  static uint8_t row, level, idx;
  static char num[8];

  if ((uint8_t)idx >= LIST_ROWS - 1)
    return;
  row = LIST_Y + idx;

  ui_line(ENTRY_X, row, NULL);
  vid_text(ENTRY_X, row, ssid_info->ssid, 32, false);

  /* Signal strength as solid blocks, then the dBm */
  if (ssid_info->rssi > -55)
    level = 4;
  else if (ssid_info->rssi > -67)
    level = 3;
  else if (ssid_info->rssi > -78)
    level = 2;
  else
    level = 1;
  for (idx = 0; idx < 4; idx++)
    vid_put(40 + idx, row, idx < level ? G_BLOCK : ' ');
  num[0] = '-';
  ui_num(ssid_info->rssi < 0 ? -ssid_info->rssi : ssid_info->rssi, &num[1]);
  strcat(num, " dBm");
  vid_text(45, row, num, 8, false);

  vid_flush_rows(row, row);
}

void screen_set_wifi_select_network(uint8_t count)
{
  ui_line(ENTRY_X, LIST_Y + count, "<Enter a network name>");
  ui_info(count ? "Choose a network." : "No networks found.");
  ui_hints("ENTER Choose  R Look again", "BREAK Skip");
  bar_set(LIST_Y, BAR_X0, count + 1, 0);
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
  strncat(buf, nc.ssid, 24);
  strcat(buf, ":");
  ui_prompt(buf);
  screen_edit_hints();
}

void screen_connect_wifi(NetConfig *net_config)
{
  /* A failed SET SSID can leave a second NAK on its way from the FujiNet;
     taken now, it can't answer the next call */
  while (port_getc_timeout(10) >= 0)
    ;

  status_stale = true;
  ui_frame();
  ui_pane(PANE_Y, "Connecting to WiFi");
  ui_center(6, "Connecting to");
  ui_center(8, net_config->ssid);
  ui_info(NULL);
  ui_hints("BREAK Stop", NULL);
  vid_flush();
}

void screen_show_info_extended(bool printerEnabled, AdapterConfigExtended *acx)
{
  (void)printerEnabled;

  ui_status(false);
  ui_frame();
  ui_pane(PANE_Y, "FujiNet");
  ui_labelled(LIST_Y, "SSID", acx->ssid);
  ui_labelled(LIST_Y + 1, "Hostname", acx->hostname);
  ui_labelled(LIST_Y + 2, "IP address", acx->sLocalIP);
  ui_labelled(LIST_Y + 3, "Gateway", acx->sGateway);
  ui_labelled(LIST_Y + 4, "DNS", acx->sDnsIP);
  ui_labelled(LIST_Y + 5, "Netmask", acx->sNetmask);
  ui_labelled(LIST_Y + 6, "MAC", acx->sMacAddress);
  ui_labelled(LIST_Y + 7, "BSSID", acx->sBssid);
  ui_labelled(LIST_Y + 8, "Firmware", acx->fn_version);
  strcpy(buf, GIT_VERSION);
#ifdef FNLIB_VERSION_FULL
  strcat(buf, "  lib ");
  strcat(buf, FNLIB_VERSION_FULL);
#endif
  ui_labelled(LIST_Y + 9, "CONFIG", buf);
  ui_info(NULL);
  ui_hints("S Change WiFi  R Reconnect  K Card settings", "Any other key: back to the hosts");
}

#endif /* BUILD_TRS80M1 */
