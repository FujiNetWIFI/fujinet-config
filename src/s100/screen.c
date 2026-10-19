#ifdef BUILD_S100
/**
 * Screen routines for an 80x24 ANSI terminal.
 *
 * Every character drawn is also kept in s100_shadow, so the bar can redraw
 * a row in reverse video without asking the caller for its text again.
 */

#include <string.h>
#include "../screen.h"
#include "../globals.h"
#include "../constants.h"
#include "s100.h"

char s100_shadow[SCR_ROWS * SCR_COLS];
char _visibleEntries;

static const char text_empty[] = "Empty";
static bool long_shown;

static void esc(const char *s)
{
    con_putc(0x1B);
    con_putc('[');
    con_print(s);
}

char *s100_utoa(uint16_t v, char *buf)
{
    char t[6], *p = &t[5];

    *p = 0;
    do
    {
        *--p = '0' + v % 10;
        v /= 10;
    } while (v);
    return strcpy(buf, p);
}

void s100_goto(uint8_t x, uint8_t y)
{
    char b[6];

    esc(s100_utoa(y + 1, b));
    con_putc(';');
    con_print(s100_utoa(x + 1, b));
    con_putc('H');
}

/* p is the shadow at column x */
static void out(char *p, uint8_t x, const char *s)
{
    char c;

    while ((c = *s++) && x++ < SCR_COLS)
    {
        if (c < ' ' || c > '~')
            c = '?';
        *p++ = c;
        con_putc(c);
    }
}

void s100_puts(uint8_t x, uint8_t y, const char *s)
{
    s100_goto(x, y);
    out(&s100_shadow[y * SCR_COLS + x], x, s);
}

void s100_putc(uint8_t x, uint8_t y, char c)
{
    char s[2];

    s[0] = c;
    s[1] = 0;
    s100_puts(x, y, s);
}

/* Clears the row from x on, then draws s there */
static void puts_clr(uint8_t x, uint8_t y, const char *s)
{
    char *p = &s100_shadow[y * SCR_COLS + x];

    s100_goto(x, y);
    esc("K");
    memset(p, ' ', SCR_COLS - x);
    out(p, x, s);
}

void s100_reverse(bool on)
{
    esc(on ? "7m" : "m");
}

void s100_cursor(bool on)
{
    esc(on ? "?25h" : "?25l");
}

void s100_status(const char *help)
{
    puts_clr(0, ROW_HELP, help);
}

void screen_clear(void)
{
    esc("H");
    esc("2J");
    memset(s100_shadow, ' ', sizeof(s100_shadow));
    bar_forget();
    long_shown = false;
}

void screen_clear_line(unsigned char y)
{
    puts_clr(0, y, "");
}

static void title(const char *t)
{
    char b[SCR_COLS];

    screen_clear();
    memset(b, ' ', SCR_COLS - 1);
    b[SCR_COLS - 1] = 0;
    memcpy(&b[1], "FujiNet CONFIG", 14);
    memcpy(&b[20], t, strlen(t));
    s100_reverse(true);
    s100_puts(0, 0, b);
    s100_reverse(false);
}

void screen_init(void)
{
    con_init();
    esc("m");
    s100_cursor(false);
    screen_clear();
}

void screen_end(void)
{
    esc("m");
    screen_clear();
    s100_cursor(true);
}

void screen_error(const char *s)
{
    puts_clr(1, ROW_MSG, s);
}

/* ---- hosts and devices ---- */

static void host_row(uint8_t y, uint8_t i, const char *h)
{
    s100_putc(2, y + i, '1' + i);
    puts_clr(5, y + i, h[0] ? h : text_empty);
}

static void device_row(uint8_t y, uint8_t i, const DeviceSlot *d)
{
    char b[4];

    s100_putc(2, y + i, '1' + i);
    b[0] = b[1] = ' ';
    b[2] = 0;
    if (d->file[0])
    {
        b[0] = '1' + d->hostSlot;
        b[1] = d->mode & MODE_WRITE ? 'W' : 'R';
    }
    s100_puts(4, y + i, b);
    puts_clr(7, y + i, d->file[0] ? (const char *)d->file : text_empty);
}

void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, bool *e)
{
    title("Hosts and Devices");
    s100_puts(1, HOSTS_START_Y - 1, "HOST SLOTS");
    s100_puts(1, DEVICES_START_Y - 1, "DRIVE SLOTS (FDC drives 0-7)");
    screen_hosts_and_devices_host_slots(h);
    screen_hosts_and_devices_device_slots(DEVICES_START_Y, d, e);
    /* This redraw comes just before the slots are mounted */
    if (hd_subState == HD_DONE)
        screen_error(s100_quit ? "Mounting, then back to CP/M..." : "Mounting and booting...");
}

void screen_hosts_and_devices_host_slots(HostSlot *h)
{
    uint8_t i;

    for (i = 0; i < NUM_HOST_SLOTS; i++)
        host_row(HOSTS_START_Y, i, (const char *)h[i]);
}

void screen_hosts_and_devices_device_slots(uint8_t y, DeviceSlot *d, const bool *e)
{
    uint8_t i;

    for (i = 0; i < NUM_DEVICE_SLOTS; i++)
        device_row(y, i, &d[i]);
}

void screen_hosts_and_devices_hosts(void)
{
    screen_clear_line(ROW_INFO);
#ifdef S100_ROM
    s100_status("[1-8] Slot [E]dit [RETURN] Browse [TAB] Drives [C]onfig [ESC] Boot");
#else
    s100_status("[1-8] Slot [E]dit [RETURN] Browse [TAB] Drives [C]onfig [ESC] Boot [Q]uit");
#endif
    bar_set(HOSTS_START_Y, 1, NUM_HOST_SLOTS, selected_host_slot);
}

void screen_hosts_and_devices_devices(void)
{
#ifdef S100_ROM
    s100_status("[1-8] Slot [E]ject [DEL] Eject all [R]ead [W]rite [TAB] Hosts [ESC] Boot");
#else
    s100_status("[1-8] [E]ject [DEL] Eject all [R]ead [W]rite [TAB] Hosts [ESC] Boot [Q]uit");
#endif
    bar_set(DEVICES_START_Y, 1, NUM_DEVICE_SLOTS, selected_device_slot);
}

void screen_hosts_and_devices_clear_host_slot(uint_fast8_t i)
{
    puts_clr(5, HOSTS_START_Y + i, "");
}

void screen_hosts_and_devices_edit_host_slot(uint_fast8_t i)
{
    bar_clear(false);
    s100_status("Edit the host name, [RETURN] when done, [ESC] to cancel");
}

void screen_hosts_and_devices_host_slot_empty(uint_fast8_t hs)
{
    s100_puts(5, HOSTS_START_Y + hs, text_empty);
}

void screen_hosts_and_devices_eject(uint8_t ds)
{
    device_row(DEVICES_START_Y, ds, &deviceSlots[ds]);
    bar_update();
}

void screen_hosts_and_devices_devices_clear_all(void)
{
    screen_error("Ejecting all drive slots...");
}

void screen_hosts_and_devices_long_filename(const char *f)
{
    puts_clr(1, ROW_INFO, f);
}

/* ---- file browser ---- */

void screen_select_file(void)
{
    title("Disk Images");
    if (copy_mode)
        s100_status("Pick the destination: [RETURN] Open [BS] Up [C] Copy here [ESC] Abort");
    else
        s100_status("[RETURN] Choose [BS] Up [<>] Page [F]ilter [N]ew [C]opy [ESC] Hosts");
}

void screen_select_file_display(char *p, char *f)
{
    uint8_t i;

    puts_clr(1, 1, "Host: ");
    s100_puts(7, 1, selected_host_name);
    if (f[0])
    {
        s100_puts(48, 1, "Filter: ");
        s100_puts(56, 1, f);
    }
    puts_clr(1, 2, "Path: ");
    s100_puts(7, 2, p);
    for (i = FILES_START_Y - 1; i <= FILES_START_Y + ENTRIES_PER_PAGE; i++)
        screen_clear_line(i);
}

void screen_select_file_prev(void)
{
    s100_puts(3, FILES_START_Y - 1, "[Previous page]");
}

void screen_select_file_next(void)
{
    s100_puts(3, FILES_START_Y + ENTRIES_PER_PAGE, "[Next page]");
}

void screen_select_file_display_entry(uint8_t y, const char *e, uint16_t entryType)
{
    s100_puts(4, FILES_START_Y + y, e);
}

void screen_select_file_display_long_filename(const char *e)
{
    puts_clr(1, ROW_INFO, e);
    long_shown = true;
}

/* Called on every idle poll, so it only draws when there is something to remove */
void screen_select_file_clear_long_filename(void)
{
    if (long_shown)
        screen_clear_line(ROW_INFO);
    long_shown = false;
}

void screen_select_file_choose(char visibleEntries)
{
    screen_clear_line(ROW_MSG); /* a filter or new-image prompt */
    _visibleEntries = visibleEntries;
    bar_set(FILES_START_Y, 3, visibleEntries, prev_page ? visibleEntries - 1 : 0);
}

void screen_select_file_filter(void)
{
    puts_clr(1, ROW_MSG, "Filter: ");
    s100_puts(9, ROW_MSG, filter);
}

void screen_select_file_new_type(void)
{
}

void screen_select_file_new_size(uint8_t k)
{
    screen_error("Size: [1] 8\" Altair  [2] Minidisk  [C]ustom  [ESC] Cancel");
}

void screen_select_file_new_custom(void)
{
    screen_error("Number of 137-byte sectors: ");
}

void screen_select_file_new_name(void)
{
    screen_error("New image name: ");
}

void screen_select_file_new_creating(void)
{
    screen_error("Creating the image...");
}

/* ---- mount ---- */

static char *two(char *p, uint8_t v)
{
    p[0] = '0' + v / 10;
    p[1] = '0' + v % 10;
    return p + 2;
}

void screen_select_slot(const char *e)
{
    const FujiDirEntry *f = (const FujiDirEntry *)e;
    char b[24], *p;

    title("Mount to Drive Slot");
    s100_puts(1, DEVICES_START_MOUNT_Y - 1, "DRIVE SLOTS");
    screen_hosts_and_devices_device_slots(DEVICES_START_MOUNT_Y, deviceSlots, deviceEnabled);

    if (create)
    {
        s100_puts(1, 13, "New image: ");
        s100_puts(12, 13, &e[11]);
    }
    else
    {
        s100_puts(1, 13, "File: ");
        s100_puts(7, 13, path);
        s100_puts(7 + strlen(path), 13, f->filename);

        p = &b[4];
        s100_utoa(1970 + f->modified.year, b);
        *p++ = '-';
        p = two(p, f->modified.month);
        *p++ = '-';
        p = two(p, f->modified.day);
        *p++ = ' ';
        p = two(p, f->modified.hour);
        *p++ = ':';
        p = two(p, f->modified.minute);
        *p = 0;
        s100_puts(1, 14, "Date: ");
        s100_puts(7, 14, b);

        strcat(s100_utoa((uint16_t)(f->size >> 10), b), "K");
        s100_puts(1, 15, "Size: ");
        s100_puts(7, 15, b);
    }

    s100_status("[1-8] Slot [RETURN] Read only [W] Read/Write [E]ject [ESC] Back");
    bar_set(DEVICES_START_MOUNT_Y, 1, NUM_DEVICE_SLOTS, 0);
}

void screen_select_slot_choose(void)
{
}

void screen_select_slot_eject(uint8_t ds)
{
    device_row(DEVICES_START_MOUNT_Y, ds, &deviceSlots[ds]);
    bar_update();
}

/* ---- copy ---- */

void screen_destination_host_slot(char *h, char *p)
{
    title("Copy to Host Slot");
    s100_puts(1, HOSTS_START_Y - 1, "HOST SLOTS");
    s100_puts(1, 13, "Copy from: ");
    s100_puts(12, 13, h);
    s100_puts(12, 14, p);
    s100_status("[1-8] Slot [RETURN] Select [ESC] Abort");
}

void screen_destination_host_slot_choose(void)
{
    bar_set(HOSTS_START_Y, 1, NUM_HOST_SLOTS, 0);
}

void screen_perform_copy(char *sh, char *p, char *dh, char *dp)
{
    title("Copying File");
    s100_puts(1, 3, "Copying, please wait...");
}

/* ---- adapter info and WiFi ---- */

static char *hex2(char *p, uint8_t v)
{
    static const char hex[] = "0123456789ABCDEF";

    p[0] = hex[v >> 4];
    p[1] = hex[v & 15];
    return p + 2;
}

void screen_show_info_extended(bool printerEnabled, AdapterConfigExtended *acx)
{
    static const char *const labels[] = {"SSID:", "Hostname:", "IP:", "Gateway:", "DNS:",
                                         "Netmask:", "MAC:", "BSSID:", "Version:"};
    const char *v[9];
    char b[48], *p;
    uint8_t i;

    v[0] = acx->ssid;
    v[1] = acx->hostname;
    v[2] = acx->sLocalIP;
    v[3] = acx->sGateway;
    v[4] = acx->sDnsIP;
    v[5] = acx->sNetmask;
    v[6] = acx->sMacAddress;
    v[7] = acx->sBssid;
    v[8] = acx->fn_version;

    title("Adapter Info");
    for (i = 0; i < 9; i++)
    {
        s100_puts(14 - strlen(labels[i]), 3 + i, labels[i]);
        s100_puts(15, 3 + i, v[i]);
    }

    strcpy(b, "pipe at ");
    p = hex2(&b[8], s100.base);
    p = strcpy(p, "h, boot page at ") + 16;
    p = hex2(p, s100.page);
    strcpy(p, s100.flags & S100_INFOF_PHANTOM ? "00h (PHANTOM)" : "00h");
    s100_puts(3, 13, "S-100 card:");
    s100_puts(15, 13, b);

    s100_status("[R]econnect  [S] Change SSID  Any other key: back");
}

void screen_set_wifi_extended(AdapterConfigExtended *acx)
{
    title("WiFi Setup");
    s100_puts(1, 1, "MAC: ");
    s100_puts(6, 1, acx->sMacAddress);
    s100_puts(1, NETWORKS_START_Y - 1, "NETWORKS");
    screen_error("Scanning for networks...");
}

void screen_set_wifi_display_ssid(char n, SSIDInfo *s)
{
    s100_puts(3, NETWORKS_START_Y + n, s->ssid);
    s100_puts(40, NETWORKS_START_Y + n, s->rssi > -50 ? "***" : s->rssi > -70 ? "**" : "*");
}

void screen_set_wifi_select_network(uint8_t nn)
{
    s100_puts(3, NETWORKS_START_Y + nn, "<Enter a specific SSID>");
    screen_clear_line(ROW_MSG);
    s100_status("[RETURN] Select  [ESC] Re-scan  [S] Skip");
    bar_set(NETWORKS_START_Y, 1, nn + 1, 0);
}

void screen_set_wifi_custom(void)
{
    screen_error("SSID: ");
    s100_status("Enter the network name and press [RETURN]");
}

void screen_set_wifi_password(void)
{
    screen_error("Password: ");
    s100_status("Enter the password and press [RETURN]");
}

void screen_connect_wifi(NetConfig *nc)
{
    title("WiFi");
    s100_puts(1, 3, "Connecting to: ");
    s100_puts(16, 3, nc->ssid);
    s100_status("[ESC] Abort");
}

#endif /* BUILD_S100 */
