#ifdef BUILD_PMD85
/**
 * FujiNet CONFIG FOR PMD85
 *
 * Screen Routines
 */

#include "screen.h"

#include <conio.h>
#include <sys/ioctl.h>
#include <string.h>

#include "globals.h"
#include "colors.h"
#include "bar.h"

#define UNUSED(x) (void)(x);

static const char *empty = "EMPTY";
static const char *off = "OFF";
//static const char *eol = "\x1bb$[\x1bb/EOL\x1bb$]\x1bb/";

extern bool copy_mode;
extern char copy_host_name[32];

static char tmp_str[41];
char *strshort(const char *s, unsigned char l)
{
	if (l>40) l=40;
	strlcpy(tmp_str, s, l);
	return tmp_str;
}

unsigned char strcut(char *d, char *s, unsigned char l)
{
	strncpy(d, s, l+1);
	if (d[l])
	{
		d[l] = '\0';
		return l;
	}
	return 0;
}

screen_print_multiline(ypos, const char *longstr)
{
	char *s = longstr;
	unsigned char n;
	unsigned char y = ypos;
	do 
	{
		gotoxy(SCR_X0, y);
		n = strcut(tmp_str, s, 40);
		cprintf("%s", tmp_str);
		if (y < SCR_Y0 + 29) y++;
		s += n;
	} while (n);
}

/* Enable/disable reverse character display.
*/
unsigned char revers(unsigned char onoff)
{
  if (onoff)
    cputs("\x1Bp");
  else
    cputs("\x1Bq");
}

void chlinexy(char x, char y, int len)
{
  gotoxy(x, y);
  // chline(len);
  for (int i = 0; i < len; i++)
    cputc('-');
}

void cclearxy(char x, char y, int len)
{
  	// gotoxy(x, y);
  	// cclear(length);
	screen_set_region(x, y, len, 1);
	screen_fill_region(PATTERN_BLANK);
}

void screen_clear()
{
	// fill the "background area" with pattern
	screen_set_region(SCR_X0 - 2, SCR_Y0 - 1, 44, 26);
	screen_fill_region(BACKGROUND_PATTERN);
	revers(0); textcolor(TEXT_COLOR);
}

void screen_clear_status()
{
  	screen_set_region(SCR_X0, STATUS_BAR, 40, 3);
  	screen_fill_region(STATUSBAR_PATTERN);
	revers(0); textcolor(STATUSBAR_COLOR);
}

void screen_init(void)
{
	clrscr(); // erase the whole screen area
	revers(0); textcolor(TEXT_COLOR);
  	//screen_fujinetlogo();
}

void screen_put_inverse(const char c)
{
  revers(1);
  cputc(c);
  revers(0);
}

void screen_print_inverse(const char *s)
{
  revers(1);
  cprintf(s);
  revers(0);
}

void screen_print_menu(const char *si, const char *sc)
{
	revers(0); textcolor(MENU_HOTKEY_COLOR);
	cputs(si);
	textcolor(MENU_ITEM_COLOR);
	cputs(sc);
}

void screen_fujinetlogo(void)
{
  unsigned char i, j;

  gotoxy(SCR_X0 + 18, SCR_Y0 + 12);
  cprintf("O");

  for (i = 0; i < 11; i++)
  {
      gotoxy(SCR_X0 + 2 + i, SCR_Y0 + 12);    // fuji scrolling across left to centre
      cprintf(" FUJI*");
      gotoxy(SCR_X0 + 29 - i, SCR_Y0 + 12);   // net scrolling back right to centre
      cprintf("*NET ");
      gotoxy(SCR_X0 + 18, SCR_Y0 + i);      // * coming down from the top
      cprintf(" ");
      gotoxy(SCR_X0 + 18, SCR_Y0 + 1 + i);
      cprintf("*");
      gotoxy(SCR_X0 + 19, SCR_Y0 + 23 - i);   // * coming up from bottom
      cprintf("*");
      gotoxy(SCR_X0 + 19, SCR_Y0 + 24 - i);
      cprintf(" ");
   }
   for(i = 0; i < 10; i++) // delay a bit
   {
      for(j = 0; j < 255; j++);
   }
}

void screen_draw_list_area(unsigned char ypos, unsigned char lines, const char *title, unsigned char vbar)
{
	// main area
	screen_set_region(SCR_X0 + vbar, ypos + 1, 40 - vbar, lines);
	screen_fill_region(LIST_PATTERN);
	// vertical bar
	if (vbar)
	{
		screen_set_region(SCR_X0, ypos + 1, vbar, lines);
		screen_fill_region(LIST_VBAR_PATTERN);
	}
	// title bar
	screen_set_region(SCR_X0, ypos, 40, 1);
	screen_fill_region(LIST_TITLE_PATTERN);
	// title text
	if (title && title[0])
	{
		revers(0); textcolor(LIST_TITLE_COLOR);
		gotoxy(SCR_X0 + 38 - strlen(title), ypos);
		cprintf(" %s ", title);
	}
	// // shadows
	// screen_set_region(SCR_X0+40, ypos + 1, 1, lines);
	// screen_fill_region(PATTERN_CHESSBOARD | ACE_BLUE);
	// screen_set_region(SCR_X0+1, ypos + 1 + lines, 40, 1);
	// screen_fill_region(PATTERN_CHESSBOARD | ACE_BLUE);
}

void screen_debug(char *message)
{
  gotoxy(0,29); cputs(message);
}

void screen_error(const char *c)
{
	//cclearxy(SCR_X0, STATUS_BAR + 2, 40);
  	gotoxy(SCR_X0, STATUS_BAR + 2);
  	revers(0); textcolor(ERROR_COLOR);
  	cputs(c);
}

void screen_set_wifi(AdapterConfigExtended *acx)
{
	// screen_clear();
	// make full screen clear here, in case we need to clear some rubbish
	clrscr();
	gotoxy(SCR_X0 + 10, SCR_Y0);
	revers(0); textcolor(TITLE1_COLOR);
	cprintf("WELCOME TO FUJINET!");

	// screen_set_region(SCR_X0, SCR_Y0 + 2, 40, 1);
	// screen_fill_region(PATTERN_SOLID | ACE_CYAN);
	gotoxy(SCR_X0 + 4, SCR_Y0 + 2);
	textcolor(TITLE2_COLOR);
	cprintf("MAC Address: %s", acx->sMacAddress);

	screen_clear_status();
	gotoxy(SCR_X0 + 9, STATUS_BAR); cprintf("SCANNING FOR NETWORKS...");
}

void screen_set_wifi_display_ssid(char n, SSIDInfo *s)
{
	static char meter[4] = {0x00, 0x00, 0x00, 0x00};
	static char ds[32];

	memset(meter, 0x0, sizeof(meter));
	strncpy(ds, s->ssid, 32);

	// // text background
    // screen_set_region(SCR_X0, SCR_Y0 + 3 + n, 40, 1);
    // screen_fill_region(PATTERN_SOLID | ACE_WHITE);
	// // shadow
    // screen_set_region(SCR_X0 + 40, SCR_Y0 + 3 + n, 1, 1);
    // screen_fill_region(PATTERN_BLANK);
    // screen_set_region(SCR_X0 + 1, SCR_Y0 + 4 + n, 40, 1);
    // screen_fill_region(PATTERN_BLANK);

	gotoxy(SCR_X0 + 1, SCR_Y0 + 4 + n);
	textcolor(TEXT_COLOR); cprintf("%s", ds);

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
	gotoxy(SCR_X0 + 36, SCR_Y0 + 4 + n);
	cprintf("%s", meter);
}

void screen_set_wifi_select_network(unsigned char nn)
{

	screen_clear_status();

	gotoxy(SCR_X0 + 11, STATUS_BAR);
	cprintf("FOUND %d NETWORKS.", nn);

	gotoxy(SCR_X0, STATUS_BAR + 1);
	screen_print_menu("EOL", ":SELECT  ");
	screen_print_menu("R", "ESCAN  ");
	screen_print_menu("H", "IDDEN SSID  ");
	screen_print_menu("K0", ":SKIP");
	//gotoxy(SCR_X0, STATUS_BAR + 2);

	bar_set(SCR_Y0 + 4, SCR_X0, nn, 0);
}

void screen_set_wifi_custom(void)
{
  screen_clear_status();
  gotoxy(SCR_X0, STATUS_BAR); cprintf("ENTER NAME OF HIDDEN NETWORK");
}

void screen_set_wifi_password(void)
{
  screen_clear_status();
  gotoxy(SCR_X0, STATUS_BAR); cprintf("ENTER NETWORK PASSWORD AND PRESS [EOL]");
}

void screen_connect_wifi(NetConfig *nc)
{
	screen_clear_status();
	gotoxy(SCR_X0, STATUS_BAR);
	cputs("CONNECTING TO NETWORK:");
	gotoxy(SCR_X0, STATUS_BAR+1);
	cprintf("%s", nc->ssid);
}

void screen_destination_host_slot(char *h, char *p)
{
	screen_clear();
	gotoxy(0, 12);
	cprintf("COPY FROM HOST SLOT");
	chlinexy(0, 13, 40);
	gotoxy(0, 14);
	cprintf("%-32s", h);
	gotoxy(0, 15);
	cprintf("%-128s", p);
}

void screen_destination_host_slot_choose(void)
{
	gotoxy(0, 0);
	cprintf("COPY TO HOST SLOT");
	chlinexy(0, 1, 40);

	screen_clear_status();
	gotoxy(0, STATUS_BAR);
	screen_print_menu("1-8", ":CHOOSE SLOT\r\n");
	screen_print_menu("EOL", ":SELECT SLOT\r\n");
	screen_print_menu("K0", ":ABORT");

	bar_set(2, 1, 8, selected_host_slot);
}

void screen_hosts_and_devices(HostSlot *h, DeviceSlot *d, bool *e)
{
	static const char hl[] = "HOST SLOTS";
	static const char ds[] = "DEVICE SLOTS";
	char i;

	screen_clear();

	// host list area
	screen_draw_list_area(SCR_Y0, 8, hl, 1);
	// device list area
	screen_draw_list_area(SCR_Y0 + 11, 4, ds, 1);

	// host list data
	screen_hosts_and_devices_host_slots(h);
	// device list data
	screen_hosts_and_devices_device_slots(SCR_Y0 + 12, d, e);
}

char *screen_hosts_and_devices_slot(char *c)
{
	return c[0] == 0x00 ? &empty[0] : c;
}

void screen_hosts_and_devices_host_slots(HostSlot *h)
{
	char i;

	for (i = 0; i < 8; i++)
	{
		gotoxy(SCR_X0, SCR_Y0 + 1 + i);
		textcolor(LIST_VBAR_COLOR); cprintf("%d", 1 + i);
		gotoxy(SCR_X0 + 3, SCR_Y0 + 1 + i);
		textcolor(LIST_COLOR); cprintf("%s", screen_hosts_and_devices_slot((char *)h[i]));
	}
}

const char *screen_hosts_and_devices_device_slot(unsigned char hs, bool e, char *fn)
{
	UNUSED(hs);
	if (fn[0] != 0x00)
		return fn;
	else if (e == false)
		return &off[0];
	else
		return &empty[0];
}

void screen_hosts_and_devices_device_slots(unsigned char y, DeviceSlot *d, bool *e)
{
	char i;
	static char devlab[4] = {'1', '2', 'T', 'M'}; // Drive 1, 2, Tape, Memory Module

	for (i = 0; i < 4; i++)
	{
		gotoxy(SCR_X0, y + i);
		textcolor(LIST_VBAR_COLOR); cprintf("%c", devlab[i]);
		gotoxy(SCR_X0 + 3, y + i);
		textcolor(LIST_COLOR); cprintf("%s", screen_hosts_and_devices_device_slot(d[i].hostSlot, e[i], (char *)d[i].file));
	}
}

void screen_hosts_and_devices_hosts(void)
{
	screen_clear_status();

	gotoxy(SCR_X0, STATUS_BAR);
	screen_print_menu("1-8", ":SLOT  ");
	screen_print_menu("E", "DIT  ");
	screen_print_menu("EOL", ":BROWSE  ");
	screen_print_menu("C-D", ":DEVICES");
	gotoxy(SCR_X0 + 12, STATUS_BAR+1);
	screen_print_menu("C", "ONFIG  ");
	screen_print_menu("K10", ":BOOT  ");

	bar_set(SCR_Y0 + 1, SCR_X0 + 2, 8, selected_host_slot);
}

void screen_hosts_and_devices_devices(void)
{
	screen_clear_status();
	gotoxy(SCR_X0, STATUS_BAR);
	screen_print_menu("1-4", ":SLOT  ");
	screen_print_menu("E", "JECT  ");
	screen_print_menu("R", "EAD  ");
	screen_print_menu("W", "RITE  ");
	screen_print_menu("C-D", ":HOSTS");
	gotoxy(SCR_X0+12, STATUS_BAR + 1);
	screen_print_menu("C", "ONFIG  ");
	screen_print_menu("K10", ":BOOT");

	bar_set(SCR_Y0 + 12, SCR_X0 + 2, 4, selected_device_slot);
}

void screen_hosts_and_devices_clear_host_slot(unsigned char i)
{
	cclearxy(SCR_X0 + 1, SCR_Y0 + 1 + i, 39);
}

void screen_hosts_and_devices_edit_host_slot(unsigned char i)
{
	screen_clear_status();
	gotoxy(SCR_X0, STATUS_BAR);
	cprintf("EDIT THE HOST NAME FOR SLOT %d", i + 1);
	gotoxy(SCR_X0, STATUS_BAR +1);
	cprintf("PRESS [EOL] WHEN DONE.");
}

void screen_perform_copy(char *sh, char *p, char *dh, char *dp)
{
	screen_clear();
	gotoxy(0, 0);
	cprintf("%32s", "COPYING FILE FROM:");
	gotoxy(0, 1);
	cprintf("%32s", sh);
	gotoxy(0, 2);
	cprintf("%-128s", p);
	gotoxy(0, 6);
	cprintf("%32s", dh);
	gotoxy(0, 7);
	cprintf("%-128s", dp);
}

char *_info_str(const char *s)
{
	return strshort(s, 26);
}

void screen_show_info(bool printerEnabled, AdapterConfigExtended *acx)
{
	screen_clear();

	gotoxy(SCR_X0 + 5, SCR_Y0 + 2);
	textcolor(TITLE3_COLOR);
	cprintf("F U J I N E T      C O N F I G");
	textcolor(TEXT_COLOR);
	gotoxy(SCR_X0 + 8, SCR_Y0 + 6);
	cprintf(       "SSID: %s", _info_str(acx->ssid));
	gotoxy(SCR_X0 + 4, SCR_Y0 + 7);
	cprintf(   "HOSTNAME: %s", _info_str(acx->hostname));
	gotoxy(SCR_X0 + 10, SCR_Y0 + 8);
	cprintf(         "IP: %s", _info_str(acx->sLocalIP));
	gotoxy(SCR_X0 + 5, SCR_Y0 + 9);
	cprintf(    "NETMASK: %s", _info_str(acx->sNetmask));
	gotoxy(SCR_X0 + 9, SCR_Y0 + 10);
	cprintf(        "DNS: %s", _info_str(acx->sDnsIP));
	gotoxy(SCR_X0 + 9, SCR_Y0 + 11);
	cprintf(        "MAC: %s", _info_str(acx->sMacAddress));
	gotoxy(SCR_X0 + 7, SCR_Y0 + 12);
	cprintf(      "BSSID: %s", _info_str(acx->sBssid));
	gotoxy(SCR_X0 + 5, SCR_Y0 + 13);
	cprintf(    "VERSION: %s", _info_str(acx->fn_version));

	screen_clear_status();
	gotoxy(SCR_X0 + 9, STATUS_BAR);
	screen_print_menu("C", "HANGE SSID  ");
	screen_print_menu("R", "ECONNECT");
	gotoxy(SCR_X0 + 4, STATUS_BAR + 1);
	cprintf("PRESS ANY KEY TO RETURN TO HOSTS");
}

void screen_select_file(void)
{
	screen_clear_status();
	gotoxy(SCR_X0 + 15, STATUS_BAR);
	cprintf("OPENING...");
}

void screen_select_file_display(char *p, char *f)
{
	//screen_clear();
	unsigned char l = strlen(p);
	if (l > 39)
	{
		tmp_str[0] = '.';
		tmp_str[1] = '.';
		tmp_str[2] = '.';
		strcpy(tmp_str+3, p+l-36);
		l = 39;
	}
	else
		strcpy(tmp_str, p);

	// content area
	screen_draw_list_area(SCR_Y0, 18, selected_host_name, 0);
	// "address" line
	screen_set_region(SCR_X0, SCR_Y0 + 1, 40, 1);
	screen_fill_region(EDITLINE_PATTERN_ON);
	// path
	gotoxy(SCR_X0 + 39 - l, SCR_Y0 + 1);
	textcolor(PATH_COLOR);
	cprintf("%s", tmp_str);
	// filter
	if (f[0] != '\0')
	{
		gotoxy(SCR_X0 + 39 - strlen(f), SCR_Y0 + 2);
		textcolor(FILTER_COLOR); 
		cprintf("%s", f);
	}
	//revers(1);

	screen_clear_status();
	gotoxy(SCR_X0, STATUS_BAR);
	screen_print_menu("EOL", ":CHOOSE  ");
    screen_print_menu("K0",":ABORT  ");
    screen_print_menu("F","ILTER  ");
    screen_print_menu("","NEW  "); // TODO
    screen_print_menu("","COPY");  // TODO
	if (l > 1)
	{
		gotoxy(SCR_X0 + 15, STATUS_BAR+1);
		screen_print_menu("RCL", ":UPDIR");
	}
}

void screen_print_bracket_dots()
{
	textcolor(LIST_VBAR_COLOR); cputc('[');
	textcolor(TEXT_COLOR); cputs("...");
	textcolor(LIST_VBAR_COLOR); cputc(']');
}

void screen_select_file_prev(void)
{
	gotoxy(SCR_X0 + 1, SCR_Y0 + 2);
	screen_print_bracket_dots();
	gotoxy(SCR_X0 + 7, STATUS_BAR+1);
    screen_print_menu("<",":PREV");
}

void screen_select_file_display_long_filename(char *e)
{
	// gotoxy(SCR_X0 + 1, SCR_Y0 + 18);
	// textcolor(WHITE); revers(0);
	// screen_print_multiline(SCR_Y0 + 18, e);
}

void screen_select_file_next(void)
{
	gotoxy(SCR_X0 + 1, SCR_Y0 + 18);
	screen_print_bracket_dots();
	gotoxy(SCR_X0 + 26, STATUS_BAR+1);
    screen_print_menu(">",":NEXT");
}

void screen_select_file_display_entry(unsigned char y, char *e, unsigned entryType)
{
	gotoxy(SCR_X0 + 1, SCR_Y0 + 3 + y);
	textcolor(LIST_COLOR); cprintf(e);
}

void screen_select_file_clear_long_filename(void)
{
	// screen_set_region(SCR_X0, SCR_Y0 + 19, 40, 1);
	// screen_fill_region(BACKGROUND_PATTERN);
}

void screen_select_file_new_type(void)
{
	screen_clear_status();
	gotoxy(SCR_X0, STATUS_BAR);
	screen_print_menu("NEW MEDIA: SELECT TYPE", "");
	gotoxy(SCR_X0, STATUS_BAR+1);
	screen_print_menu("P", "O  ");
	screen_print_menu("2", "MG  ");
}

void screen_select_file_new_size(unsigned char k)
{
	UNUSED(k); // Image type is not used.
	screen_clear_status();
	gotoxy(SCR_X0, STATUS_BAR);
	screen_print_menu("NEW MEDIA: SELECT SIZE", "");
	gotoxy(SCR_X0, STATUS_BAR+1);
	screen_print_menu("1", "40K  ");
	screen_print_menu("8", "00K  ");
	screen_print_menu("3", "2MB  ");
}

void screen_select_file_new_custom(void)
{
	screen_clear_status();
	gotoxy(SCR_X0, STATUS_BAR);
	screen_print_menu("NEW MEDIA: CUSTOM SIZE", "");
	gotoxy(SCR_X0, STATUS_BAR+1);
	screen_print_menu("ENTER SIZE IN # OF 512-BYTE BLOCKS", "");
}

void screen_select_file_choose(char visibleEntries)
{
	bar_set(SCR_Y0 + 3, SCR_X0, visibleEntries, 0); // TODO: Handle previous

	// screen_clear_status();
	// gotoxy(SCR_X0 + 8, STATUS_BAR);
	// screen_print_menu("EOL", ":SELECT FILE TO MOUNT");
	// gotoxy(SCR_X0, STATUS_BAR+1);
	// screen_print_menu("RCL", ":UPDIR  ");
    // screen_print_menu("K0",":ABORT  ");
    // screen_print_menu("F","ILTER  ");
    // screen_print_menu("N","EW  ");
    // screen_print_menu("C","OPY");
}

void screen_select_file_filter(void)
{
	screen_clear_status();
	gotoxy(SCR_X0, STATUS_BAR);
	cprintf("ENTER A WILDCARD FILTER. e.g. *PoMiDor*");
}

void screen_select_slot(char *e)
{
	struct _additl_info
	{
		char year;
		unsigned char month;
		unsigned char day;
		unsigned char hour;
		unsigned char min;
		unsigned char sec;
		unsigned long size;
		//unsigned char flags;
    	unsigned char isdir;
    	unsigned char trunc;
		unsigned char type;
		char filename[0];
	} *info = (struct _additl_info *)e;

	screen_clear();

	screen_draw_list_area(SCR_Y0, NUM_DEVICE_SLOTS, "PLACE IN DEVICE SLOT", 1);

	screen_hosts_and_devices_device_slots(SCR_Y0 + 1, &deviceSlots[0], &deviceEnabled[0]);

  	gotoxy(SCR_X0, SCR_Y0 + 8);
	revers(0); textcolor(TITLE4_COLOR);
  	cputs("FILE DETAILS");

  	gotoxy(SCR_X0, SCR_Y0 + 10); textcolor(TEXT_COLOR);
  	cprintf("MODIFIED: %u-%02u-%02u %02u:%02u:%02u",
		2000 + info->year, info->month, info->day, info->hour, info->min, info->sec);

  	gotoxy(SCR_X0, SCR_Y0 + 11);
  	cprintf("    SIZE: %lu", info->size);

	// handle long filename
	screen_print_multiline(SCR_Y0 + 13, info->filename);

	bar_set(SCR_Y0 + 1, SCR_X0 + 2, NUM_DEVICE_SLOTS, 0);
}

void screen_select_slot_choose(void)
{
	screen_clear_status();
	gotoxy(SCR_X0 + 0, STATUS_BAR);
	screen_print_menu("1-4", ":SLOT ");
	screen_print_menu("EOL\x1b" "b/:\x1b" "b$R", "EAD  ");
	screen_print_menu("W", "RITE  ");
	screen_print_menu("E", "JECT ");
	screen_print_menu("K0", ":ABORT");
}

void screen_select_file_new_name(void)
{
	screen_clear_status();
	gotoxy(3, STATUS_BAR);
	screen_print_menu(" NEW MEDIA: ENTER FILENAME \r\n", "");
}

void screen_hosts_and_devices_long_filename(char *f)
{
	// // TODO: implement
	// if (strlen(f) > 31)
	// {
	// 	gotoxy(0, STATUS_BAR - 3);
	// 	cprintf("%s", f);
	// }
	// else
	// 	cclearxy(0, STATUS_BAR - 3, 120);
}

void screen_hosts_and_devices_devices_clear_all(void)
{
	screen_clear_status();
	gotoxy(3, STATUS_BAR);
	screen_print_menu(" CLEARING ALL DEVICE SLOTS ", "");
}

void screen_select_file_new_creating(void)
{
	screen_clear_status();
	gotoxy(3, STATUS_BAR);
	screen_print_menu(" CREATING DISK IMAGE \r\n", "PLEASE WAIT...");
}

void screen_select_slot_mode(void)
{
	screen_clear_status();
	gotoxy(1, STATUS_BAR);
	screen_print_menu("R", "EAD ONLY  ");
	screen_print_menu("W", ": READ/WRITE");
}

void screen_print_empty_device_slot(unsigned char ypos)
{
	bar_clear(false);
	cclearxy(SCR_X0 + 1, ypos, 39);
	gotoxy(SCR_X0 + 3, ypos);
	textcolor(LIST_COLOR);
	cprintf(empty);
	bar_jump(bar_get());
}

void screen_select_slot_eject(unsigned char ds)
{
	screen_print_empty_device_slot(SCR_Y0 + 1 + ds);
}

void screen_hosts_and_devices_eject(unsigned char ds)
{
	screen_print_empty_device_slot(SCR_Y0 + 12 + ds);
}

void screen_hosts_and_devices_host_slot_empty(unsigned char hs)
{
	gotoxy(SCR_X0 + 3, SCR_Y0 + 1 + hs);
	textcolor(LIST_COLOR);
	cprintf(empty);
}
#endif /* BUILD_PMD85 */
