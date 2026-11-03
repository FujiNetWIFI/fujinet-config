/* st_files.c -- the file browser: paging, descend/devance, filter.
 *
 * One seek per page then sequential reads (READ_DIR_ENTRY advances the
 * cursor itself), names drawn straight from the reply window at display
 * width. The firmware shortens a longer name with "...", so a bar that rests
 * on one re-reads it at full width and scrolls it; so does acting on one. `path` is
 * '/'-prefixed, '/'-terminated, appended to on descend and truncated at the
 * previous '/' on devance.
 */

#include <string.h>

#include "fujidisp.h"
#include "fujiin.h"
#include "fujiedit.h"
#include "fujiraw.h"
#include "sfx.h"
#include "state.h"

/* The marquee on the selected row. `saved` keeps the row as the page drew
 * it. `scroll_len` is the full name's length once fetched, 0 until then. */
static char saved[IN_W];
static unsigned int idle;
static unsigned char scroll_len, scroll_off, scroll_wait;
static bool scroll_dead;        /* nothing to scroll on this row */

bool dir_seek(unsigned int pos)
{
    return fuji_set_directory_position(pos);
}

bool dir_read_entry(unsigned char maxlen)
{
    volatile unsigned char *r;
    unsigned char b0, b1, b2;

    if (!FUJICALL_A1_A2(FUJICMD_READ_DIR_ENTRY, maxlen, 0))
        return false;

    /* End of directory is two 0x7F bytes. Not every host stops there --
     * reading past the end of an SD root gives back ".." forever and poisons
     * SET_DIRECTORY_POSITION for the rest of the session -- so "." and ".."
     * count as the end too, and are never read past. */
    r = FN_REPLY;
    b0 = r[0];
    b1 = r[1];
    b2 = r[2];
    if (b0 == 0x7F && b1 == 0x7F)
        return false;
    if (b0 == '.' && (b1 == 0 || (b1 == '.' && b2 == 0)))
        return false;
    return true;
}

bool dir_open(void)
{
    return fnraw_open_directory(host, path, fn_entry);
}

static bool dir_reopen(void)
{
    fuji_close_directory();     /* best effort */
    return dir_open();
}

void files_legend(void)
{
    status_line("A--OPEN B--BACK SEL--FILTER");
    legend_line("<>--PAGE  START--HOSTS");
}

/* Blank the list window from row `i` down. A short page must not leave the
 * tail of the page before it on screen. */
static void list_clear_from(unsigned char i)
{
    for (; i < LIST_ROWS; i++)
        disp_row_clear((unsigned char)(LIST_TOP + i));
}

/* Put the selected row back as the page drew it and restart the marquee. */
static void scroll_clear(void)
{
    idle = 0;
    scroll_len = 0;
    scroll_off = 0;
    scroll_dead = false;
}

static void scroll_reset(void)
{
    if (scroll_len != 0 && nrows != 0)
        disp_row_restore((unsigned char)(LIST_TOP + cur), saved, true);
    scroll_clear();
}

static void scroll_paint(void)
{
    volatile unsigned char *name = FN_REPLY;
    unsigned char col, at;

    for (col = 0; col < NAMELEN; col++) {
        at = (unsigned char)(scroll_off + col);
        disp_char_hi((unsigned char)(3 + col), (unsigned char)(LIST_TOP + cur),
                     at < scroll_len ? (char)name[at] : ' ', true);
    }
}

/* One call per idle frame. */
static void files_tick(void)
{
    volatile unsigned char *name;
    unsigned char row = (unsigned char)(LIST_TOP + cur);
    unsigned char shown;

    if (nrows == 0 || scroll_dead)
        return;
    if (scroll_len == 0) {
        if (++idle < SCROLL_DELAY)
            return;
        disp_row_save(row, saved);
        for (shown = IN_W; shown > 0 && saved[shown - 1] == ' '; shown--)
            ;
        /* The firmware cuts a name at NAMELEN - 1 characters, so a shorter
         * one is whole. */
        if (shown < NAMELEN - 1 ||
            !dir_seek(top + cur) || !dir_read_entry(FULLLEN)) {
            scroll_dead = true;
            return;
        }
        name = FN_REPLY;
        for (shown = 0; shown < FULLLEN && name[shown] != 0; shown++)
            ;
        if (shown <= NAMELEN) {
            scroll_dead = true;
            return;
        }
        scroll_len = shown;
        scroll_off = 0;
        scroll_wait = SCROLL_HOLD;
        scroll_paint();
        return;
    }
    if (scroll_wait != 0) {
        scroll_wait--;
        return;
    }
    if (scroll_off + NAMELEN >= scroll_len) {
        scroll_off = 0;
        scroll_wait = SCROLL_HOLD;
    } else {
        scroll_off++;
        scroll_wait = (unsigned char)(scroll_off + NAMELEN >= scroll_len ?
                                      SCROLL_HOLD : SCROLL_STEP);
    }
    scroll_paint();
}

static void files_page(void)
{
    volatile unsigned char *name;
    unsigned char i;

    scroll_clear();
    if (!dir_seek(top)) {
        list_clear_from(0);
        fail("SEEK");
        nrows = 0;
        return;
    }

    nrows = 0;
    at_end = 0;
    for (i = 0; i < LIST_ROWS; i++) {
        unsigned char row = (unsigned char)(LIST_TOP + i);
        unsigned char col;

        disp_row_clear(row);
        if (!dir_read_entry(NAMELEN)) {
            at_end = 1;
            break;
        }
        name = FN_REPLY;
        for (col = 0; col < NAMELEN && name[col] != 0; col++)
            disp_char((unsigned char)(3 + col), row, (char)name[col]);
        nrows = (unsigned char)(i + 1);
    }
    list_clear_from(nrows);

    if (nrows == 0) {
        disp_at(3, LIST_TOP, fn_entry[0] ? "(NO MATCHES)" : "(EMPTY)");
        return;
    }
    if (cur >= nrows)
        cur = (unsigned char)(nrows - 1);
    disp_row_invert((unsigned char)(LIST_TOP + cur), true);
}

void files_draw(void)
{
    unsigned char plen = (unsigned char)strlen(path);
    char title[7];

    strcpy(title, "HOST 1");
    title[5] = (char)('1' + host);
    draw_frame(title);
    disp_at(IN_L, 3, plen > IN_W ? path + (plen - IN_W) : path);
    files_legend();
    files_page();
}

static void fmove(signed char d)
{
    if (d < 0) {
        if (nrows != 0 && cur > 0) {
            scroll_reset();
            disp_row_invert((unsigned char)(LIST_TOP + cur), false);
            cur--;
            disp_row_invert((unsigned char)(LIST_TOP + cur), true);
        } else if (top >= LIST_ROWS) {
            top -= LIST_ROWS;
            cur = LIST_ROWS - 1;
            files_page();
        }
    } else {
        if (nrows == 0)
            return;
        if ((unsigned char)(cur + 1) < nrows) {
            scroll_reset();
            disp_row_invert((unsigned char)(LIST_TOP + cur), false);
            cur++;
            disp_row_invert((unsigned char)(LIST_TOP + cur), true);
        } else if (!at_end) {
            top += LIST_ROWS;
            cur = 0;
            files_page();
        }
    }
}

void leave_host(void)
{
    fuji_close_directory();
    cur = host;
    state = ST_HOSTS;
}

static void devance(void)
{
    unsigned char len = (unsigned char)strlen(path);

    if (len <= 1) {
        leave_host();
        return;
    }
    len--;
    while (len > 0 && path[len - 1] != '/')
        len--;
    path[len] = 0;
    top = 0;
    cur = 0;
    if (!dir_reopen()) {
        fail("OPEN");
        return;
    }
    files_draw();
}

static void open_or_boot(void)
{
    volatile unsigned char *name;
    unsigned char plen;
    unsigned int n, i;

    if (nrows == 0)
        return;
    status_line("READING...");
    if (!dir_seek(top + cur) || !dir_read_entry(FULLLEN)) {
        fail("READ");
        return;
    }
    name = FN_REPLY;
    n = 0;
    while (n < FULLLEN && name[n] != 0)
        n++;
    if (n == 0) {
        fail("NAME");
        return;
    }

    if (name[n - 1] == '/') {
        plen = (unsigned char)strlen(path);
        if (plen + n > PATH_MAX_LEN - 1) {
            status_line("?PATH TOO LONG");
            sfx_beep();
            wait_frames(90);
            files_legend();
            return;
        }
        for (i = 0; i < n; i++)
            path[plen + i] = (char)name[i];
        path[plen + n] = 0;
        top = 0;
        cur = 0;
        if (!dir_reopen()) {
            fail("OPEN");
            wait_frames(90);
            path[plen] = 0;
            dir_reopen();
        }
        files_draw();
        return;
    }

    boot_reply_entry();         /* only failure comes back */
    wait_frames(120);
    files_draw();
}

static void do_filter(void)
{
    fn_entry[FILTER_MAX] = 0;
    fn_edit("FILTER", FILTER_MAX);
    top = 0;
    cur = 0;
    if (!dir_reopen())
        fail("OPEN");
    files_draw();
}

void st_files(void)
{
    files_draw();
    while (state == ST_FILES) {
        unsigned char ev = in_read();

        switch (ev) {
        case IN_NONE:
            files_tick();
            break;
        case IN_UP:
            fmove(-1);
            break;
        case IN_DOWN:
            fmove(1);
            break;
        case IN_LEFT:
            if (top >= LIST_ROWS) {
                top -= LIST_ROWS;
                cur = 0;
                files_page();
            }
            break;
        case IN_RIGHT:
            if (nrows != 0 && !at_end) {
                top += LIST_ROWS;
                cur = 0;
                files_page();
            }
            break;
        case IN_FIRE:
            open_or_boot();
            break;
        case IN_BACK:
            devance();
            break;
        case IN_SELECT:
            do_filter();
            break;
        case IN_START:
            leave_host();
            break;
        }
    }
}
