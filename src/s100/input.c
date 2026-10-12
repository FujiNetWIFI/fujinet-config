#ifdef BUILD_S100
/**
 * Input routines: keys from the terminal, arrows decoded from ANSI escape sequences.
 */

#include <stdlib.h>
#include <string.h>
#include "../input.h"
#include "../globals.h"
#include "../constants.h"
#include "../screen.h"
#include "../hosts_and_devices.h"
#include "../select_file.h"
#include "../select_slot.h"
#include "../set_wifi.h"
#include "s100.h"

/* An escape sequence's bytes arrive back to back; a lone ESC is followed by nothing */
#define ESC_WAIT_MS 50

/* Sector counts for the new-image sizes, 137-byte sectors */
#define SECTORS_8IN  2464 /* 77 tracks of 32 */
#define SECTORS_MINI 560  /* 35 tracks of 16 */

extern unsigned short entry_timer;
extern unsigned char copy_host_slot;

bool s100_quit;

static uint8_t key_csi(void)
{
    uint8_t c, n = 0;

    while ((c = con_getc_wait(ESC_WAIT_MS)) >= '0' && c <= '9')
        n = c;
    switch (c)
    {
    case 'A':
        return KEY_UP_ARROW;
    case 'B':
        return KEY_DOWN_ARROW;
    case 'C':
        return KEY_RIGHT_ARROW;
    case 'D':
        return KEY_LEFT_ARROW;
    case 'H':
        return KEY_HOME;
    case 'F':
        return KEY_END;
    case '~':
        switch (n)
        {
        case '1':
        case '7':
            return KEY_HOME;
        case '3':
            return KEY_DELETE;
        case '4':
        case '8':
            return KEY_END;
        case '5':
            return KEY_PAGE_UP;
        case '6':
            return KEY_PAGE_DOWN;
        }
    }
    return 0;
}

/* Does not wait: 0 when no key is waiting */
uint8_t input(void)
{
    uint8_t c = con_getc();

    switch (c)
    {
    case 0:
        if (entry_timer)
            entry_timer--;
        break;
    case 0x05: /* the WordStar diamond */
        return KEY_UP_ARROW;
    case 0x18:
        return KEY_DOWN_ARROW;
    case 0x13:
        return KEY_LEFT_ARROW;
    case 0x04:
        return KEY_RIGHT_ARROW;
    case 0x7F:
        return KEY_BACKSPACE;
    case KEY_ESCAPE:
        c = con_getc_wait(ESC_WAIT_MS);
        if (c == '[' || c == 'O')
            return key_csi();
        return KEY_ESCAPE;
    }
    return c;
}

static uint8_t key(void)
{
    uint8_t k;

    while (!(k = input()))
        ;
    return k;
}

/* l is the size of c; ESC puts back what c held */
void input_line(uint8_t x, uint8_t y, uint8_t o, char *c, uint8_t l, bool password)
{
    char save[65];
    uint8_t i = o, k;

    strncpy(save, c, sizeof(save) - 1);
    save[sizeof(save) - 1] = 0;
    s100_cursor(true);
    for (;;)
    {
        s100_goto(x + i, y);
        k = key();
        if (k == KEY_RETURN)
            break;
        if (k == KEY_ESCAPE)
        {
            strcpy(c, save);
            break;
        }
        if (k == KEY_BACKSPACE || k == KEY_LEFT_ARROW)
        {
            if (i)
            {
                c[--i] = 0;
                s100_putc(x + i, y, ' ');
            }
        }
        else if (k >= ' ' && k < 0x7F && i + 1 < l)
        {
            s100_putc(x + i, y, password ? '*' : k);
            c[i++] = k;
            c[i] = 0;
        }
    }
    s100_cursor(false);
}

void input_line_set_wifi_custom(char *c)
{
    memset(c, 0, SSID_MAXLEN);
    input_line(7, ROW_MSG, 0, c, SSID_MAXLEN, false);
}

void input_line_set_wifi_password(char *c)
{
    input_line(11, ROW_MSG, 0, c, MAX_PASSWORD_LEN, true);
}

void input_line_hosts_and_devices_host_slot(uint_fast8_t i, uint_fast8_t o, char *c)
{
    input_line(5, HOSTS_START_Y + i, o, c, sizeof(HostSlot), false);
}

void input_line_filter(char *c)
{
    input_line(9, ROW_MSG, strlen(c), c, 32, false);
}

uint8_t input_select_file_new_type(void)
{
    return 1;
}

uint32_t input_select_file_new_size(uint8_t t)
{
    switch (key())
    {
    case '1':
        return SECTORS_8IN;
    case '2':
        return SECTORS_MINI;
    case 'C':
    case 'c':
        return 1;
    }
    return 0;
}

uint32_t input_select_file_new_custom(void)
{
    char b[6];

    b[0] = 0;
    input_line(29, ROW_MSG, 0, b, sizeof(b), false);
    return atoi(b);
}

void input_select_file_new_name(char *c)
{
    input_line(17, ROW_MSG, 0, c, 64, false);
}

WSSubState input_set_wifi_select(void)
{
    switch (input())
    {
    case KEY_UP_ARROW:
        bar_up();
        break;
    case KEY_DOWN_ARROW:
        bar_down();
        break;
    case KEY_ESCAPE:
        return WS_SCAN;
    case 'S':
    case 's':
        state = HOSTS_AND_DEVICES;
        return WS_DONE;
    case KEY_RETURN:
        if (bar_get() >= numNetworks)
            return WS_CUSTOM;
        set_wifi_set_ssid(bar_get());
        return WS_PASSWORD;
    }
    return WS_SELECT;
}

SISubState input_show_info(void)
{
    switch (key())
    {
    case 'R':
    case 'r':
        state = CONNECT_WIFI;
        break;
    case 'S':
    case 's':
        state = SET_WIFI;
        break;
    default:
        state = HOSTS_AND_DEVICES;
    }
    return SI_DONE;
}

/* ESC boots; under CP/M, Q mounts the same way and then goes back to the CCP */
static HDSubState hd_done(bool quit)
{
    s100_quit = quit;
    return HD_DONE;
}

/* The keys both panes share; 0 when k is none of them */
static HDSubState hd_common(uint8_t k)
{
    switch (k)
    {
    case 'C':
    case 'c':
        state = SHOW_INFO;
        return HD_DONE;
    case KEY_ESCAPE:
        return hd_done(false);
#ifndef S100_ROM
    case 'Q':
    case 'q':
        return hd_done(true);
#endif
    }
    return 0;
}

HDSubState input_hosts_and_devices_hosts(void)
{
    uint8_t k = input();
    HDSubState r;

    if (k >= '1' && k <= '8')
    {
        bar_jump(k - '1');
        selected_host_slot = bar_get();
        return HD_HOSTS;
    }
    switch (k)
    {
    case KEY_UP_ARROW:
        bar_up();
        selected_host_slot = bar_get();
        break;
    case KEY_DOWN_ARROW:
        bar_down();
        selected_host_slot = bar_get();
        break;
    case 'E':
    case 'e':
        hosts_and_devices_edit_host_slot(bar_get());
        break;
    case KEY_TAB:
        return HD_DEVICES;
    case KEY_RETURN:
        selected_host_slot = bar_get();
        if (hostSlots[selected_host_slot][0])
        {
            strcpy(selected_host_name, (const char *)hostSlots[selected_host_slot]);
            state = SELECT_FILE;
            return HD_DONE;
        }
        break;
    default:
        if ((r = hd_common(k)))
            return r;
    }
    return HD_HOSTS;
}

HDSubState input_hosts_and_devices_devices(void)
{
    uint8_t k = input();
    HDSubState r;

    if (k >= '1' && k <= '8')
        bar_jump(k - '1');
    switch (k)
    {
    case KEY_UP_ARROW:
        bar_up();
        break;
    case KEY_DOWN_ARROW:
        bar_down();
        break;
    case 'E':
    case 'e':
        hosts_and_devices_eject(bar_get());
        return HD_DEVICES;
    case KEY_DELETE:
        return HD_CLEAR_ALL_DEVICES;
    case KEY_TAB:
        return HD_HOSTS;
    case 'R':
    case 'r':
    case 'W':
    case 'w':
        selected_device_slot = bar_get();
        hosts_and_devices_devices_set_mode((k | 0x20) == 'w' ? MODE_WRITE : MODE_READ);
        screen_hosts_and_devices_device_slots(DEVICES_START_Y, deviceSlots, deviceEnabled);
        bar_update();
        return HD_DEVICES;
    default:
        if ((r = hd_common(k)))
            return r;
        if (k < '1' || k > '8')
            return HD_DEVICES;
    }
    selected_device_slot = bar_get();
    hosts_and_devices_long_filename();
    return HD_DEVICES;
}

SFSubState input_select_file_choose(void)
{
    uint8_t k = input();
    unsigned t;

    switch (k)
    {
    case KEY_HOME:
        bar_jump(0);
        break;
    case KEY_END:
        bar_jump(_visibleEntries - 1);
        break;
    case KEY_PAGE_UP:
    case '<':
    case ',':
        if (pos > 0)
            return SF_PREV_PAGE;
        break;
    case KEY_PAGE_DOWN:
    case '>':
    case '.':
        if (!dir_eof)
            return SF_NEXT_PAGE;
        break;
    case KEY_UP_ARROW:
        entry_timer = ENTRY_TIMER_DUR;
        if (bar_get() == 0 && pos > 0)
            return SF_PREV_PAGE;
        bar_up();
        break;
    case KEY_DOWN_ARROW:
        entry_timer = ENTRY_TIMER_DUR;
        if (bar_get() == _visibleEntries - 1 && !dir_eof)
            return SF_NEXT_PAGE;
        bar_down();
        break;
    case KEY_RETURN:
        pos += bar_get();
        screen_select_file_clear_long_filename();
        t = select_file_entry_type();
        if (t == ENTRY_TYPE_FOLDER)
            return SF_ADVANCE_FOLDER;
        if (t == ENTRY_TYPE_LINK)
            return SF_LINK;
        strncpy(source_path, path, 224);
        old_pos = pos;
        return SF_DONE;
    case KEY_BACKSPACE:
    case KEY_LEFT_ARROW:
        if (path[1]) /* not at the root */
            return SF_DEVANCE_FOLDER;
        break;
    case KEY_ESCAPE:
        copy_mode = false;
        state = HOSTS_AND_DEVICES;
        return SF_DONE;
    case 'F':
    case 'f':
        return SF_FILTER;
    case 'N':
    case 'n':
        return SF_NEW;
    case 'C':
    case 'c':
        if (copy_mode)
            return SF_DONE;
        pos += bar_get();
        select_file_set_source_filename();
        copy_host_slot = selected_host_slot;
        return SF_COPY;
    }
    return SF_CHOOSE;
}

SSSubState input_select_slot_choose(void)
{
    uint8_t k = input();

    if (k >= '1' && k <= '8')
        bar_jump(k - '1');
    switch (k)
    {
    case KEY_UP_ARROW:
        bar_up();
        break;
    case KEY_DOWN_ARROW:
        bar_down();
        break;
    case 'E':
    case 'e':
        select_slot_eject(bar_get());
        break;
    case KEY_ESCAPE:
        state = SELECT_FILE;
        backToFiles = true;
        return SS_DONE;
    case KEY_RETURN:
    case 'R':
    case 'r':
        selected_device_slot = bar_get();
        mode = MODE_READ;
        return SS_DONE;
    case 'W':
    case 'w':
        selected_device_slot = bar_get();
        mode = MODE_WRITE;
        return SS_DONE;
    }
    return SS_CHOOSE;
}

DHSubState input_destination_host_slot_choose(void)
{
    uint8_t k = input();

    if (k >= '1' && k <= '8')
        bar_jump(k - '1');
    switch (k)
    {
    case KEY_UP_ARROW:
        bar_up();
        break;
    case KEY_DOWN_ARROW:
        bar_down();
        break;
    case KEY_RETURN:
        selected_host_slot = bar_get();
        copy_mode = true;
        return DH_DONE;
    case KEY_ESCAPE:
        state = HOSTS_AND_DEVICES;
        return DH_ABORT;
    }
    return DH_CHOOSE;
}

#endif /* BUILD_S100 */
