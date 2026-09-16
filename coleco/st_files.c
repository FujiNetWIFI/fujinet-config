/* st_files.c -- the file browser: paging, descend/devance, filter.
 *
 * Mirrors src/select_file.c's flow with the testrom's proven mechanics: one
 * seek per page then sequential reads (READ_DIR_ENTRY advances the cursor
 * itself), names drawn straight from the reply window at display width and
 * re-read at full width the moment one is acted on -- what is on screen was
 * crunched to 29 columns, and a path built from that would name a file that
 * does not exist.
 *
 * `path` is the one string in RAM: '/'-prefixed, '/'-terminated, appended to
 * on descend (the sole reply-to-RAM copy in the program) and truncated at the
 * previous '/' on devance. Deliberately unlike intv there is no page-position
 * stack: devance lands on page one of the parent, matching src/select_file.c.
 */

#include <string.h>

#include "fujidisp.h"
#include "fujisnd.h"
#include "fujiin.h"
#include "fujiedit.h"
#include "fujiraw.h"
#include "state.h"

bool dir_seek(unsigned int pos)
{
    /* 16-bit on the wire: a big directory has more than 255 entries, and the
     * testrom's 8-bit version silently seeked pos & 0xFF. */
    return fuji_set_directory_position(pos);
}

bool dir_read_entry(unsigned char maxlen)
{
    volatile unsigned char *r;
    unsigned char b0, b1, b2;

    if (!FUJICALL_A1_A2(FUJICMD_READ_DIR_ENTRY, maxlen, 0))
        return false;

    /* End of directory is two 0x7F bytes. But not every host stops there --
     * reading past the end of an SD root gives back ".." forever instead, and
     * doing that also poisons SET_DIRECTORY_POSITION for the rest of the
     * session. Neither "." nor ".." is ever something to act on, so treat
     * them as the end too and never read past it. */
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

/* Close-then-open, for every open after the first: fujinet-pc keeps one
 * directory handle per session and a stale one left open costs nothing to
 * close. */
static bool dir_reopen(void)
{
    fuji_close_directory();     /* best effort */
    return dir_open();
}

void files_legend(void)
{
    if (copy_mode) {
        status_line("5 COPY HERE  FIRE OPEN DIR");
        legend_line("1 UP  < > PAGE  * CANCEL");
    } else {
        status_line("FIRE OPEN/BOOT 1 UP 4 FILTER");
        legend_line("5 COPY  < > PAGE  * HOSTS");
    }
}

static void files_page(void)
{
    volatile unsigned char *name;
    unsigned char i;

    if (!dir_seek(top)) {
        fail("ESEEK");
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
            disp_char((unsigned char)(2 + col), row, (char)name[col]);
        nrows = (unsigned char)(i + 1);
    }

    if (nrows == 0) {
        disp_at(2, LIST_TOP, fn_entry[0] ? "(no matches)" : "(empty)");
        return;
    }
    if (cur >= nrows)
        cur = (unsigned char)(nrows - 1);
    disp_row_invert((unsigned char)(LIST_TOP + cur), true);
}

void files_draw(void)
{
    unsigned char plen = (unsigned char)strlen(path);

    draw_frame(NULL);
    /* Row 3: the tail of the current path -- the leading part is the least
     * interesting part when it does not fit. */
    disp_at(1, 3, plen > 30 ? path + (plen - 30) : path);
    files_legend();
    files_page();
}

/* The browser's bar, with page-crossing. Up from row 0 of a later page goes
 * back one page even when the current page is empty -- paging one past the
 * end of an exactly-full directory must not trap the cursor. */
static void fmove(signed char d)
{
    if (d < 0) {
        if (nrows != 0 && cur > 0) {
            disp_row_invert((unsigned char)(LIST_TOP + cur), false);
            cur--;
            disp_row_invert((unsigned char)(LIST_TOP + cur), true);
            snd_click();
        } else if (top >= LIST_ROWS) {
            top -= LIST_ROWS;
            cur = LIST_ROWS - 1;
            snd_click();
            files_page();
        }
    } else {
        if (nrows == 0)
            return;
        if ((unsigned char)(cur + 1) < nrows) {
            disp_row_invert((unsigned char)(LIST_TOP + cur), false);
            cur++;
            disp_row_invert((unsigned char)(LIST_TOP + cur), true);
            snd_click();
        } else if (!at_end) {
            top += LIST_ROWS;
            cur = 0;
            snd_click();
            files_page();
        }
    }
}

void leave_host(void)
{
    fuji_close_directory();     /* best effort: the host page redraws anyway */
    cur = host;
    state = ST_HOSTS;
}

static void devance(void)
{
    unsigned char len = (unsigned char)strlen(path);

    if (len <= 1) {
        /* Already at the root: up means back to the host list. In copy mode
         * that is simply picking a different destination host. */
        leave_host();
        return;
    }
    len--;                      /* step inside the trailing '/' */
    while (len > 0 && path[len - 1] != '/')
        len--;
    path[len] = 0;
    top = 0;
    cur = 0;
    snd_click();
    if (!dir_reopen()) {
        fail("EOPEN");
        return;
    }
    files_draw();
}

/* Fire: re-read the highlighted entry at full width, then descend into a
 * directory, boot a file -- or, mid-copy, point at the 5 key. */
static void open_or_boot(void)
{
    volatile unsigned char *name;
    unsigned char plen;
    unsigned int n, i;

    if (nrows == 0)
        return;
    status_line("READING...");
    if (!dir_seek(top + cur) || !dir_read_entry(FULLLEN)) {
        fail("EREAD");
        return;
    }
    name = FN_REPLY;
    n = 0;
    while (n < FULLLEN && name[n] != 0)
        n++;
    if (n == 0) {
        fail("EEMPTY");
        return;
    }

    if (name[n - 1] == '/') {
        /* A directory (the firmware appends the '/'). Append it to the path,
         * trailing slash and all -- the one reply-to-RAM copy here. */
        plen = (unsigned char)strlen(path);
        if (plen + n > PATH_MAX_LEN - 1) {
            status_line("PATH TOO LONG");
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
            fail("EOPEN");
            wait_frames(90);
            path[plen] = 0;     /* fall back to the parent */
            dir_reopen();
        }
        files_draw();
        return;
    }

    if (copy_mode) {
        status_line("PRESS 5 TO COPY HERE");
        return;
    }

    boot_reply_entry();         /* only failure comes back */
    wait_frames(120);
    files_draw();
}

static void do_filter(void)
{
    /* The filter IS fn_entry while this screen is up -- preloaded here so the
     * editor shows what is already applied. The editor mutates it in place,
     * so OK and ESC both leave the edited text as the filter; documented in
     * the README rather than fought with a scratch copy there is no RAM for. */
    fn_entry[FILTER_MAX] = 0;
    fn_edit("FILTER (EMPTY SHOWS ALL)", FILTER_MAX);
    top = 0;
    cur = 0;
    if (!dir_reopen())
        fail("EOPEN");
    files_draw();
}

void st_files(void)
{
    files_draw();
    while (state == ST_FILES) {
        unsigned char ev = in_read();

        switch (ev) {
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
                snd_click();
                files_page();
            }
            break;
        case IN_RIGHT:
            if (nrows != 0 && !at_end) {
                top += LIST_ROWS;
                cur = 0;
                snd_click();
                files_page();
            }
            break;
        case IN_FIRE:
            open_or_boot();
            break;
        case IN_KEY0 + 1:
            devance();
            break;
        case IN_KEY0 + 4:
            do_filter();
            break;
        case IN_KEY0 + 5:
            if (copy_mode) {
                copy_here();
                if (state == ST_FILES)
                    files_draw();
            } else {
                copy_mark();
            }
            break;
        case IN_KEYSTAR:
            if (copy_mode) {
                copy_cancel();
                if (state == ST_FILES)
                    files_draw();
            } else {
                leave_host();
            }
            break;
        }
    }
}
