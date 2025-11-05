#ifdef _CMOC_VERSION_

/**
 * Input routines
 */

#include <cmoc.h>
#include <coco.h>
#include "../input.h"
#include "../screen.h"
#include "../globals.h"
#include "mount_and_boot.h"
#include "strendswith.h"
#include "../key_codes.h"
#include "../hosts_and_devices.h"
#include "../select_file.h"
#include "../set_wifi.h"
#include "../select_slot.h"
#include "scroll.h"

unsigned char selected_network;
extern bool copy_mode;
extern unsigned char copy_host_slot;
unsigned short custom_numSectors;
unsigned short custom_sectorSize;
extern char fn[256];
bool mounting = false;
extern unsigned short entry_timer;
extern HDSubState hd_subState;

/**
 * @brief this routine is needed because waitkey() and readline()
 *        always emit uppercase. argh!
 */
uint8_t input()
{
	char shift = false;
	char k;

	while (true)
	{
		k = inkey();

		if (isKeyPressed(KEY_PROBE_SHIFT, KEY_BIT_SHIFT))
		{
			shift = 0x00;
		}
		else
		{
			if (k > '@' && k < '[')
				shift = 0x20;
		}

		if (k)
			return k + shift;
	}
}

unsigned char input_ucase()
{
	return 0;
}

unsigned char input_handle_joystick(void)
{
	return 0;
}

// Old cursor position
char ox = -1;
char oy = -1;
char oc = 0;

void input_cursor(char x, char y)
{
	screen_put(x, y, 0xAF); // Blue cursor
}

void input_line(uint8_t x, uint8_t y, uint8_t unknown, char *c, uint8_t l, bool password)
{
	int o = strlen(c);
	char k = 0;
	char *b = c;

	// Place string pointer at end of string
	while (*c)
	{
		putchar(*c);
		c++;
		x++;
		if (x > 31)
		{
			y++;
			x = 0;
		}
	}

	input_cursor(x, y);

	while (k != KEY_ENTER) // ENTER
	{
		locate(x, y);
		input_cursor(x, y);
		k = (char)waitkey(false);

		switch (k)
		{
		case KEY_LEFT_ARROW:
			if (c > b)
			{
				putchar(KEY_LEFT_ARROW);
				c--;
				x--;
				*c = 0;
			}
			break;
		case KEY_RIGHT_ARROW:
			if (*c)
			{
				c++;
				x++;
			}
			break;
		case KEY_ENTER: // Ignore it.
			break;
		default:
			if (password)
			{
				putchar('*');
			}
			else
			{
				putchar(k);
			}

			if (x > 31)
			{
				x = 0;
				y++;
			}
			else
				x++;

			*c = k;
			c++;
		}

		input_cursor(x, y);
	}
}

void input_line_set_wifi_custom(char *c)
{
  input_line(0, 15, 0, c, 32, false);
}

void input_line_set_wifi_password(char *c)
{
  input_line(0, 15, 0, c, 64, true);
}

void input_line_hosts_and_devices_host_slot(uint_fast8_t i, uint_fast8_t o, char *c)
{
	bar_clear(true);
	input_line(1, (unsigned char)i + 1, 0, c, 32, false);
}

void input_line_filter(char *c)
{
  input_line(0, 15, 0, c, 31, false);
}

unsigned char input_select_file_new_type(void)
{
	return 1; // We always wanna make a DSK
}

unsigned long input_select_file_new_size(unsigned char t)
{
	char c[16];

	memset(c, 0, sizeof(c));

	input_line(0, 15, 0, c, 16, false);

	return (long)atol(c);
}

unsigned long input_select_file_new_custom(void)
{
	return 0;
}

void input_select_file_new_name(char *c)
{
  input_line(0, 15, 0, c, 255, false);

	if (!strendswith(c, ".dsk") && !strendswith(c, ".DSK"))
	{
		strcat(c, ".DSK");
	}
}

bool input_select_slot_build_eos_directory(void)
{
	return false;
}

void input_select_slot_build_eos_directory_label(char *c) {}

WSSubState input_set_wifi_select(void)
{
	char k = waitkey(true);

	switch (k)
	{
	case KEY_ENTER: // ENTER
		set_wifi_set_ssid(bar_get());
		return WS_PASSWORD;
	case 'H':
	case 'h':
		return WS_CUSTOM;
	case 'R':
	case 'r':
		return WS_SCAN;
	case KEY_UP_ARROW:
		bar_up();
		break;
	case KEY_DOWN_ARROW:
		bar_down();
		break;
	default:
		return WS_SELECT;
	}

	return WS_SELECT;
}

/**
 * Handle global console keys.
 */
unsigned char input_handle_console_keys(void)
{
	return 0;
}

HDSubState input_hosts_and_devices_hosts(void)
{
	locate(31, 14);

	char k = waitkey(true);

	switch (k)
	{
	case KEY_BREAK:
		return HD_DONE;
		break;
	case KEY_UP_ARROW: // up
		bar_up();
		break;
	case KEY_DOWN_ARROW: // down
		bar_down();
		break;
	case KEY_ENTER: // enter
		selected_host_slot = (unsigned char)bar_get();
		if (hostSlots[selected_host_slot][0] != 0)
		{
			strcpy((char *)selected_host_name, (char *)hostSlots[selected_host_slot]);
			state = SELECT_FILE;
			return HD_DONE;
		}
		else
			return HD_HOSTS;
	case KEY_LEFT_ARROW:
	case KEY_RIGHT_ARROW:
		return HD_DEVICES;
	case 'c':
	case 'C':
		state = SHOW_INFO;
		return HD_DONE;
	case 'e':
	case 'E':
		hosts_and_devices_edit_host_slot(bar_get());
		bar_clear(true);
		bar_jump(selected_host_slot);
		k = 0;
		return HD_HOSTS;
	case 'l':
  	case 'L':
    	mount_and_boot_lobby();
    	return HD_HOSTS;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
		bar_clear(false);
		bar_jump(k - '1');

		break;
	}
	return HD_HOSTS;
}

HDSubState input_hosts_and_devices_devices(void)
{
	locate(31, 15);

	char k = waitkey(true);

	switch (k)
	{
	case 'C':
	case 'c':
		state = SHOW_INFO;
		return HD_DONE;
	case 'E':
	case 'e':
		hosts_and_devices_eject((byte)bar_get());
		break;
	case 'L':
  	case 'l':
    	mount_and_boot_lobby();
    	return HD_DEVICES;
	case 'R':
	case 'r':
		selected_device_slot = (byte)bar_get();
		hosts_and_devices_devices_set_mode(MODE_READ);
		return HD_DEVICES;
	case 'W':
	case 'w':
		selected_device_slot = (byte)bar_get();
		hosts_and_devices_devices_set_mode(MODE_WRITE);
		return HD_DEVICES;
	case KEY_BREAK:
		return HD_DONE;
	case KEY_UP_ARROW: // up
		bar_up();
		break;
	case KEY_DOWN_ARROW: // down
		bar_down();
		break;
	case KEY_CLEAR: // CLEAR
		return HD_CLEAR_ALL_DEVICES;
	case KEY_LEFT_ARROW:
	case KEY_RIGHT_ARROW:
		return HD_HOSTS;
	case KEY_0:
	case KEY_1:
	case KEY_2:
	case KEY_3:
		bar_clear(false);
		bar_jump(k - '0');
		break;
		/* default: */
		/*   locate(0,10); */
		/*   printf("%02x",k); */
		/*   break; */
	}
	return HD_DEVICES;
}

SFSubState input_select_file_choose(void)
{
	char k;
	unsigned entryType = 0;

	scroll_reset(true);

	locate(31, 15);

	while (true)
	{
		word now = getTimer();
    	k = inkey();

		switch (k)
		{
		case 'C':
		case 'c':
			scroll_reset(false);
			if (copy_mode == true)
			{
				return SF_DONE;
			}
			else
			{
				pos += bar_get();
				select_file_set_source_filename();
				copy_host_slot = selected_host_slot;
				return SF_COPY;
			}
			return SF_COPY;
		case 'F':
		case 'f':
			return SF_FILTER;
		case 'N':
		case 'n':
			return SF_NEW;
		case KEY_BREAK: // BREAK
			state = HOSTS_AND_DEVICES;
			return SF_DONE;
		case KEY_LEFT_ARROW: // LEFT
			return strcmp(path, "/") == 0 ? SF_CHOOSE : SF_DEVANCE_FOLDER;
		case KEY_RIGHT_ARROW: // RIGHT
		case KEY_ENTER:		  // ENTER
			pos += bar_get();
			entryType = select_file_entry_type();
			if (entryType == ENTRY_TYPE_FOLDER)
				return SF_ADVANCE_FOLDER;
			else if (entryType == ENTRY_TYPE_LINK)
				return SF_LINK;
			else
				return SF_DONE;
		case KEY_UP_ARROW: // up arrow
			scroll_reset(false);
			if ((bar_get() == 0) && (pos > 0))
				return SF_PREV_PAGE;
			else
			{
				/* long_entry_displayed=false; */
				entry_timer = ENTRY_TIMER_DUR;
				bar_up();
				select_display_long_filename();
				return SF_CHOOSE;
			}
		case KEY_SHIFT_UP_ARROW: // shifted up arrow
			scroll_reset(false);
			if (pos > 0)
				return SF_PREV_PAGE;
			break;
		case KEY_DOWN_ARROW: // down arrow
			scroll_reset(false);
			if ((bar_get() == 9) && (dir_eof == false))
				return SF_NEXT_PAGE;
			else
			{
				/* long_entry_displayed=false; */
				entry_timer = ENTRY_TIMER_DUR;
				bar_down();
				select_display_long_filename();
				return SF_CHOOSE;
			}
			break;
		case KEY_SHIFT_DOWN_ARROW: // shifted down arrow
			scroll_reset(false);
			if (dir_eof == false)
				return SF_NEXT_PAGE;
			break;
		}

		// Handle idle countdown + timed scroll
		if ((word)(now - lastTimer) >= SCROLL_DELAY_TICKS)
		{
			lastTimer = now;

			if (idleCounter > 0)
			{
				idleCounter--;
			}
			else
			{
				scroll_step();
			}
		}
	}

	return SF_CHOOSE;
}

SSSubState input_select_slot_choose(void)
{
	locate(31, 14);
	char c = waitkey(true);
	switch (c)
	{
	case KEY_BREAK: // BREAK
		state = HOSTS_AND_DEVICES;
		return SS_ABORT;
	case 'E':
		select_slot_eject((char)bar_get());
		break;
	case KEY_ENTER: // ENTER
	case 'R':
	case 'r':
		mode = MODE_READ;
		selected_device_slot = (char)bar_get();
		strncpy(source_path, path, 224);
		old_pos = pos;
		return SS_DONE;
	case 'W':
	case 'w':
		mode = MODE_WRITE;
		selected_device_slot = (char)bar_get();
		return SS_DONE;
	case KEY_UP_ARROW:
		bar_up();
		return SS_CHOOSE;
	case KEY_DOWN_ARROW:
		bar_down();
		return SS_CHOOSE;
	default:
		return SS_CHOOSE;
	}
	return SS_CHOOSE;
}

unsigned char input_select_slot_mode(char *mode)
{
	return 1;
}

/*
 *  Handle inupt for the "show info" screen.
 *
 *  'C' - Reconnect Wifi
 *  'S' - Change SSID
 *  Any other key - return to main hosts and devices screen
 *
 */
SISubState input_show_info(void)
{
	char c = waitkey(true);
	switch (c)
	{
	case 'c':
	case 'C':
		state = SET_WIFI;
		return SI_DONE;
	case 'r':
	case 'R':
		state = CONNECT_WIFI;
		return SI_DONE;
	default:
		state = HOSTS_AND_DEVICES;
		return SI_DONE;
	}

	return SI_SHOWINFO;
}

DHSubState input_destination_host_slot_choose(void)
{
	locate(31, 14);

	char k = waitkey(true);

	switch (k)
	{
	case KEY_BREAK:
		state = HOSTS_AND_DEVICES;
		return DH_ABORT;
	case KEY_UP_ARROW: // up
		bar_up();
		return DH_CHOOSE;
	case KEY_DOWN_ARROW: // down
		bar_down();
		return DH_CHOOSE;
	case KEY_ENTER: // enter
		selected_host_slot = (unsigned char)bar_get();
		copy_mode = true;
		strcpy((char *)selected_host_name, (char *)hostSlots[selected_host_slot]);
		return DH_DONE;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
		bar_clear(false);
		bar_jump(k - '1');
		return DH_CHOOSE;
	default:
		return DH_CHOOSE;
	}
}

void set_device_slot_mode(unsigned char slot, unsigned char mode) {}

#endif /* _CMOC_VERSION_ */
