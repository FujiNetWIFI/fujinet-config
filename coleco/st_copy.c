/* st_copy.c -- host-to-host copy, in the intv/st_copy.bas folded shape:
 *
 *   1. keypad 5 on a highlighted FILE marks it (src_spec, src_host) and
 *      bounces to ST_HOSTS, which renders itself as "COPY TO WHICH HOST?".
 *   2. picking a host there mounts it and browses it like any other.
 *   3. keypad 5 again, anywhere in the destination tree, performs the copy
 *      and puts the browser back where the copy started.
 *
 * There is no confirmation step, matching every other platform's CONFIG, and
 * no progress stream to show: COPY_FILE is one host-side transaction whose
 * ACK arrives when the copy is done (fnraw_copy_file waits it out).
 */

#include <string.h>

#include "fujidisp.h"
#include "fujiedit.h"
#include "fujiraw.h"
#include "state.h"

void copy_mark(void)
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
        status_line("CANNOT COPY A FOLDER");
        wait_frames(90);
        files_legend();
        return;
    }

    plen = (unsigned char)strlen(path);
    if (plen + n > SPEC_MAX_LEN - 1) {
        status_line("PATH TOO LONG");
        wait_frames(90);
        files_legend();
        return;
    }
    for (i = 0; i < plen; i++)
        src_spec[i] = path[i];
    for (i = 0; i < n; i++)
        src_spec[plen + i] = (char)name[i];
    src_spec[plen + n] = 0;

    src_host = host;
    copy_mode = 1;
    fuji_close_directory();
    cur = src_host;             /* the hosts bar starts on the source */
    state = ST_HOSTS;
}

/* Back to the source browse: derive the directory from src_spec, remount and
 * reopen. On success the caller finds state == ST_FILES and redraws (or the
 * dispatcher does); on failure the host list is the safe place to land. */
static void copy_return(void)
{
    unsigned char len, i;

    copy_mode = 0;
    host = src_host;
    len = (unsigned char)strlen(src_spec);
    while (len > 0 && src_spec[len - 1] != '/')
        len--;
    for (i = 0; i < len; i++)
        path[i] = src_spec[i];
    path[len] = 0;
    fn_entry[0] = 0;            /* the filter belonged to the other browse */
    top = 0;
    cur = 0;

    fuji_close_directory();
    if (!fuji_mount_host_slot(host)) {
        fail("EHOST");
        wait_frames(90);
        cur = host;
        state = ST_HOSTS;
        return;
    }
    if (!dir_open()) {
        fail("EOPEN");
        wait_frames(90);
        cur = host;
        state = ST_HOSTS;
        return;
    }
    state = ST_FILES;
}

void copy_here(void)
{
    status_line("COPYING...");
    if (fnraw_copy_file((unsigned char)(src_host + 1), (unsigned char)(host + 1),
                        src_spec, path))
        status_line("COPIED");
    else
        fail("ECOPY");
    wait_frames(120);
    copy_return();
}

void copy_cancel(void)
{
    copy_return();
}
