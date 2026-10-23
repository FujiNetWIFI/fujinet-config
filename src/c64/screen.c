#ifdef BUILD_C64
/**
 * FujiNet CONFIG FOR C64
 *
 * Screen Routines
 */

#include "../screen.h"
#include "../globals.h"
#include "petscii.h"
#include <conio.h>
#include <string.h>
#include <c64.h>

#define STATUS_BAR 21

#define UNUSED(x) (void)(x);

static const char *empty = "EMPTY";

unsigned char *mousetext = (unsigned char *)0xC00E;

/**
 * Print text from FujiNet, which is ASCII, padded or cut to w columns
 */
static void screen_puts_ascii(const char *s, unsigned char w)
{
	for (; w > 0; w--)
		cputc(*s ? ascii_to_petscii(*s++) : ' ');
}

/**
 * As screen_puts_ascii(), keeping the end of a long path
 */
static void screen_puts_ascii_tail(const char *s, unsigned char w)
{
	size_t l = strlen(s);

	screen_puts_ascii(l > w ? s + l - w : s, w);
}

static void screen_host_slot(unsigned char i, const char *h)
{
	gotoxy(0, i + 1);
	cprintf("%d ", i + 1);
	if (h[0] == 0x00)
		cprintf("%-38s", empty);
	else
		screen_puts_ascii(h, 38);
}

static void screen_device_slot(unsigned char y, unsigned char i, const DeviceSlot *d)
{
	char m;

	gotoxy(0, y);
	if (d->file[0] == 0x00)
	{
		cprintf("%d %-38s", i + 1, empty);
		return;
	}

	switch (d->mode & 0x0f) // without MODE_MOUNTED
	{
	case MODE_READ:
		m = 'R';
		break;
	case MODE_WRITE:
		m = 'W';
		break;
	default:
		m = '?';
		break;
	}
	cprintf("%d %c %d:", i + 1, m, d->hostSlot + 1);
	screen_puts_ascii((const char *)d->file, 34);
}

extern bool copy_mode;
extern unsigned char copy_host_slot;

void screen_init(void)
{
	clrscr();
}

void screen_inverse_line(unsigned char y)
{
	unsigned char i;

	for (i = 0; i < 40; i++)
	{
		char c;
		gotoxy(i, y);
		c = cpeekc();
		revers(1);
		cputcxy(c, i, y);
		revers(0);
	}
}

void screen_put_inverse(const char c)
{
	revers(1);
	cputc(c);
	revers(0);
}

void screen_print_inverse(const char *s)
{
	char i;
	for (i = 0; i < strlen(s); i++)
	{
		screen_put_inverse(s[i]);
	}
}

void screen_print_menu(const char *si, const char *sc)
{
	screen_print_inverse(si);
	cprintf(sc);
}

void screen_error(const char *c)
{
	cclearxy(0, STATUS_BAR, 120);
	gotoxy(0, STATUS_BAR + 1);
	cprintf("%-40s", c);
	screen_inverse_line(STATUS_BAR + 1);
}

void screen_putlcc(char)
{
}

void screen_set_wifi_extended(AdapterConfigExtended *ac)
{
	clrscr();
	gotoxy(9, 0);
	cprintf("WELCOME TO FUJINET!");
	gotoxy(0, 2);
	cprintf("MAC Address: ");
	screen_puts_ascii(ac->sMacAddress, 27);
	gotoxy(7, STATUS_BAR);
	cprintf("SCANNING FOR NETWORKS...");
}

void screen_set_wifi_display_ssid(char n, SSIDInfo *s)
{
	char meter[4] = {0x20, 0x20, 0x20, 0x00};

	if (s->rssi > -50)
	{
		meter[0] = '*';
		meter[1] = '*';
		meter[2] = '*';
	}
	else if (s->rssi > -70)
	{
		meter[0] = '*';
		meter[1] = '*';
	}
	else
	{
		meter[0] = '*';
	}

	gotoxy(2, n + 4);
	screen_puts_ascii(s->ssid, 32);
	gotoxy(36, n + 4);
	cprintf("%s", meter);
}

void screen_set_wifi_select_network(unsigned char nn)
{
	cclearxy(0, STATUS_BAR, 120);
	bar_set(4, 1, nn, 0);
	gotoxy(10, STATUS_BAR);
	cprintf("FOUND %d NETWORKS.\r\n", nn);
	gotoxy(4, STATUS_BAR + 1);
	screen_print_menu("H", "IDDEN SSID  ");
	screen_print_menu("R", "ESCAN  ");
	screen_print_menu("S", "KIP\r\n");
	gotoxy(10, STATUS_BAR + 2);
	screen_print_menu("RETURN", " TO SELECT");
}

void screen_set_wifi_custom(void)
{
	cclearxy(0, STATUS_BAR, 120);
	gotoxy(0, STATUS_BAR);
	cprintf("ENTER NAME OF HIDDEN NETWORK");
}

void screen_set_wifi_password(void)
{
	char ostype;
	ostype = get_ostype() & 0xF0;

	cclearxy(0, STATUS_BAR, 120);
	gotoxy(0, STATUS_BAR);
	cprintf("ENTER NET PASSWORD AND PRESS [RETURN]");
	gotoxy(0, STATUS_BAR + 1);
	cputc(']');
}

void screen_connect_wifi(NetConfig *nc)
{
	cclearxy(0, STATUS_BAR, 120);
	gotoxy(0, STATUS_BAR);
	cprintf("CONNECTING TO NETWORK: ");
	screen_puts_ascii(nc->ssid, 32);
}

void screen_destination_host_slot(char *h, char *p)
{
	clrscr();
	gotoxy(0, 12);
	cprintf("COPY FROM HOST SLOT");
	chlinexy(0, 13, 40);
	gotoxy(0, 14);
	screen_puts_ascii(h, 40);
	gotoxy(0, 15);
	screen_puts_ascii_tail(p, 120);
}

void screen_destination_host_slot_choose(void)
{
	const char t[] = "COPY TO HOST SLOT";

	// Title on row 0, as on the hosts screen: the slots fill rows 1-8
	gotoxy(0, 0);
	cprintf("%40s", t);
	chlinexy(0, 0, 40 - sizeof(t));

	cclearxy(0, STATUS_BAR, 120);
	gotoxy(0, STATUS_BAR);
	screen_print_menu("1-8", ":CHOOSE SLOT\r\n");
	screen_print_menu("RETURN", ":SELECT SLOT\r\n");
	screen_print_menu("\x5F", " TO ABORT");

	bar_set(1, 1, 8, selected_host_slot);
}

void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *d, const bool *e)
{
	char i;

	UNUSED(e); // no platform queries the enabled state yet
	for (i = 0; i < 4; i++)
		screen_device_slot(y + i, i, &d[i]);
}

void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, bool *e)
{
	const char hl[] = "HOST LIST";
	const char ds[] = "DRIVE SLOTS";
	char i;

	clrscr();
	gotoxy(0, 0);
	cprintf("%40s", hl); // screen_inverse(0);
	chlinexy(0, 0, 40 - sizeof(hl));

	gotoxy(0, 10);
	cprintf("%40s", ds); // screen_inverse(10);
	chlinexy(0, 10, 40 - sizeof(ds));

	for (i = 0; i < 8; i++)
		screen_host_slot(i, (char *)h[i]);

	screen_hosts_and_devices_device_slots(11, d, e);
}

void screen_hosts_and_devices_hosts(void)
{
	bar_set(1, 1, 8, selected_host_slot);
	cclearxy(0, STATUS_BAR, 120);
	gotoxy(0, STATUS_BAR);
	screen_print_menu("1-8", ":SLOT  ");
	screen_print_menu("E", "DIT  ");
	screen_print_menu("RETURN", ":SELECT FILES\r\n ");
	screen_print_menu("C", "ONFIG  ");
	screen_print_menu("D", "RIVE SLOTS  ");
	screen_print_menu("\x5F", ":BOOT");
}

void screen_hosts_and_devices_host_slots(HostSlot *h)
{
	char i;

	for (i = 0; i < 8; i++)
		screen_host_slot(i, (char *)h[i]);
}

void screen_hosts_and_devices_devices(void)
{
	bar_set(11, 1, 4, 0);
	cclearxy(0, STATUS_BAR, 120);
	gotoxy(0, STATUS_BAR);
	screen_print_menu("E", "JECT  ");
	screen_print_menu("R", "EAD ONLY  ");
	screen_print_menu("W", "RITE\r\n");
	screen_print_menu("H", "OST SLOTS  ");
	screen_print_menu("\x5F", ":BOOT");
}

void screen_hosts_and_devices_clear_host_slot(unsigned char i)
{
	cclearxy(1, i + 1, 39);
}

void screen_hosts_and_devices_edit_host_slot(unsigned char i)
{
	cclearxy(0, STATUS_BAR, 120);
	gotoxy(0, STATUS_BAR);
	cprintf("EDIT THE HOST NAME FOR SLOT %d\r\nPRESS [RETURN] WHEN DONE.", i + 1);
}

void screen_perform_copy(char *sh, char *p, char *dh, char *dp)
{
	clrscr();
	gotoxy(0, 0);
	cprintf("COPYING FILE FROM:");
	gotoxy(0, 1);
	screen_puts_ascii(sh, 40);
	gotoxy(0, 2);
	screen_puts_ascii_tail(p, 120);
	gotoxy(0, 6);
	cprintf("TO:");
	gotoxy(0, 7);
	screen_puts_ascii(dh, 40);
	gotoxy(0, 8);
	screen_puts_ascii_tail(dp, 120);
}

static void screen_info_line(const char *l, const char *v)
{
	cprintf("%10s", l);
	screen_puts_ascii(v, 30);
}

void screen_show_info_extended(bool printerEnabled, AdapterConfigExtended *ac)
{
	clrscr();

	gotoxy(4, 5);
	revers(1);
	cprintf("F U J I N E T      C O N F I G");
	revers(0);
	gotoxy(0, 8);
	// Each line fills the row, so the next starts below it
	screen_info_line("SSID: ", ac->ssid);
	screen_info_line("HOSTNAME: ", ac->hostname);
	screen_info_line("IP: ", ac->sLocalIP);
	screen_info_line("NETMASK: ", ac->sNetmask);
	screen_info_line("DNS: ", ac->sDnsIP);
	screen_info_line("MAC: ", ac->sMacAddress);
	screen_info_line("BSSID: ", ac->sBssid);
	screen_info_line("FNVER: ", ac->fn_version);
	cprintf("%10s%s\r\n", "CONFIG: ", GIT_VERSION);

	gotoxy(6, STATUS_BAR);
	screen_print_menu("C", "HANGE SSID  ");
	screen_print_menu("R", "ECONNECT\r\n");
	cprintf("   PRESS ANY KEY TO RETURN TO HOSTS\r\n");
	cprintf("      FUJINET PRINTER %s", (printerEnabled) ? "ENABLED" : "DISABLED");
}

void screen_select_file(void)
{
	cclearxy(0, STATUS_BAR, 120);
	gotoxy(0, STATUS_BAR);
	cprintf("OPENING...");
}

void screen_select_file_display(char *p, char *f)
{
	clrscr();

	// Update content area
	gotoxy(0, 0);
	screen_puts_ascii(selected_host_name, 40);
	gotoxy(0, 1);
	if (f[0] == 0x00)
		screen_puts_ascii_tail(p, 40);
	else
	{
		screen_puts_ascii_tail(p, 32);
		screen_puts_ascii(f, 8);
	}
}

void screen_select_file_prev(void)
{
	gotoxy(0, 2);
	cprintf("%-40s", "[...]");
}

void screen_select_file_display_long_filename(const char *)
{
}

void screen_select_file_next(void)
{
	gotoxy(0, 18);
	cprintf("%-40s", "[...]");
}

void screen_select_file_display_entry(unsigned char y, const char *e, unsigned)
{
	gotoxy(0, y + 3);
	screen_puts_ascii(e, 40);
}

void screen_select_file_clear_long_filename(void)
{
}

void screen_select_file_new_type(void)
{
	cclearxy(0, STATUS_BAR, 120);
	gotoxy(0, STATUS_BAR);
	screen_print_menu(" NEW MEDIA: SELECT TYPE \r\n", "");
	screen_print_menu("P", "O  ");
	screen_print_menu("2", "MG  ");
}

void screen_select_file_new_size(unsigned char k)
{
	UNUSED(k); // Image type is not used.
	cclearxy(0, STATUS_BAR, 120);
	gotoxy(0, STATUS_BAR);
	screen_print_menu(" NEW MEDIA: SELECT SIZE \r\n", "");
	screen_print_menu("1", "40K  ");
	screen_print_menu("8", "00K  ");
	screen_print_menu("3", "2MB  ");
}

void screen_select_file_new_custom(void)
{
	cclearxy(0, STATUS_BAR, 120);
	gotoxy(0, STATUS_BAR);
	screen_print_menu(" NEW MEDIA: CUSTOM SIZE \r\n", "");
	screen_print_menu("ENTER SIZE IN # OF 512-BYTE BLOCKS\r\n", "");
}

void screen_select_file_choose(char visibleEntries)
{
	bar_set(3, 2, visibleEntries, prev_page ? visibleEntries-1 : 0);
	cclearxy(0, STATUS_BAR, 120);
	gotoxy(0, STATUS_BAR);
	if (copy_mode)
	{
		screen_print_menu("RETURN", ":OPEN FOLDER\r\n");
		screen_print_menu("^", ":UPDIR  ");
		screen_print_menu("\x5F", ":ABORT  ");
		screen_print_menu("C", "OPY HERE");
	}
	else
	{
		// No NEW: IEC FujiNet can't create disks
		screen_print_menu("RETURN", ":SELECT FILE TO MOUNT\r\n");
		screen_print_menu("^", ":UPDIR  ");
		screen_print_menu("\x5F", ":ABORT  ");
		screen_print_menu("F", "ILTER  ");
		screen_print_menu("C", "OPY");
	}
}

void screen_select_file_filter(void)
{
	cclearxy(0, STATUS_BAR, 120);
	gotoxy(0, STATUS_BAR);
	cprintf("ENTER A WILDCARD FILTER.\r\n E.G. *Apple*");
}

void screen_select_slot(const char *e)
{
	unsigned int s;
	unsigned char low;
	unsigned char high;

	clrscr();

	gotoxy(0, 7);
	cprintf("%-40s", "File Details");
	cprintf("%8s %u-%02u-%02u %02u:%02u:%02u\r\n", "MTIME:", (unsigned char)(*e++) + 1900, *e++, *e++, *e++, *e++, *e++);

	low = *e++;
	high = *e++;
	s = low + (high << 8);
	cprintf("%8s %u\r\n\r\n", "SIZE:", s);

	e += 2; // skip is_dir, media_type
	// Filename is next
	screen_puts_ascii(e, 40);

	screen_hosts_and_devices_device_slots(1, &deviceSlots[0], &deviceEnabled[0]);

	bar_set(1, 1, 4, 0);
}

void screen_select_slot_choose(void)
{
	cclearxy(0, STATUS_BAR, 120);
	gotoxy(3, STATUS_BAR);
	screen_print_menu("1-4", " SELECT SLOT OR USE ARROW KEYS\r\n ");
	screen_print_menu("RETURN/R", ":INSERT READ ONLY\r\n ");
	screen_print_menu("W", ":INSERT READ/WRITE\r\n ");
	screen_print_menu("\x5F", " TO ABORT.");
}

void screen_select_file_new_name(void)
{
	cclearxy(0, STATUS_BAR, 120);
	gotoxy(3, STATUS_BAR);
	screen_print_menu(" NEW MEDIA: ENTER FILENAME \r\n", "");
}

void screen_hosts_and_devices_long_filename(const char *f)
{
	// TODO: implement
	if (strlen(f) > 31)
	{
		gotoxy(0, STATUS_BAR - 3);
		screen_puts_ascii_tail(f, 120);
	}
	else
		cclearxy(0, STATUS_BAR - 3, 120);
}

void screen_hosts_and_devices_devices_clear_all(void)
{
	cclearxy(0, STATUS_BAR, 120);
	gotoxy(3, STATUS_BAR);
	screen_print_menu(" CLEARING ALL DEVICE SLOTS ", "");
}

void screen_select_file_new_creating(void)
{
	cclearxy(0, STATUS_BAR, 120);
	gotoxy(3, STATUS_BAR);
	screen_print_menu(" CREATING DISK IMAGE \r\n", "PLEASE WAIT...");
}

void screen_select_slot_mode(void)
{
	cclearxy(0, STATUS_BAR, 120);
	gotoxy(1, STATUS_BAR);
	screen_print_menu("R", "EAD ONLY  ");
	screen_print_menu("W", ": READ/WRITE");
}

void screen_select_slot_eject(unsigned char ds)
{
	gotoxy(0, 1 + ds);
	cprintf("%d %-38s", ds + 1, empty);
	bar_jump(bar_get());
}

void screen_hosts_and_devices_eject(unsigned char ds)
{
	gotoxy(0, 11 + ds);
	cprintf("%d %-38s", ds + 1, empty);
	bar_jump(bar_get());
}

void screen_hosts_and_devices_host_slot_empty(unsigned char hs)
{
	screen_host_slot(hs, "");
}

void screen_end(void)
{
  return;
}

#endif /* BUILD_C64 */
