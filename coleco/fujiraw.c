/* fujiraw.c -- streamed-payload transactions over fujinet-lib's mailbox
 * primitives. See fujiraw.h for why these exist at all.
 *
 * Everything here leans on what bus/coleco exports: fn_sink (the volatile
 * sink that keeps sccz80 from discarding a hotspot read), FN_TOUCH,
 * fn_regwr() and fn_commit(). The one thing duplicated is the commit loop,
 * once, with a longer fuse -- COPY_FILE's ACK arrives only when the host-side
 * copy finishes, and the lib's fn_commit() gives up after the ordinary
 * 5-second budget.
 */

#include <fujinet-fuji.h>
#include <fujinet-bus-coleco.h>

#include "fujiraw.h"

#define PAYLOAD_LEN 256

static void raw_begin(unsigned char cmd)
{
    fn_regwr(FNR_DATA_RST, 0);
    fn_regwr(FNR_DEVICE, FUJI_DEVICEID_FUJINET);
    fn_regwr(FNR_CMD, cmd);
    fn_regwr(FNR_NPARAM, 0);
}

/* One 8-bit parameter; `count` is the running parameter count including this
 * one. Parameters ride the TX stream as {size, value}. */
static void raw_param8(unsigned char v, unsigned char count)
{
    FN_TOUCH(FN_TXPAGE + 1);
    FN_TOUCH(FN_TXPAGE + v);
    fn_regwr(FNR_NPARAM, count);
}

static void raw_tx(unsigned char b)
{
    FN_TOUCH(FN_TXPAGE + b);
}

static void raw_tx_str(const char *s)
{
    while (*s)
        FN_TOUCH(FN_TXPAGE + (unsigned char)*s++);
}

static void raw_tx_padded(const char *s, unsigned int total)
{
    unsigned int n = 0;

    while (*s && n < total) {
        FN_TOUCH(FN_TXPAGE + (unsigned char)*s++);
        n++;
    }
    while (n++ < total)
        FN_TOUCH(FN_TXPAGE);
}

static bool raw_finish(void)
{
    return fn_commit() == FN_OK && FN_REPLYCMD == FUJICMD_ACK;
}

/* fn_commit() with the fuse scaled ~20x, past the cartridge's own 60-second
 * budget for the slow commands, so a real timeout still surfaces as the
 * cart's error code rather than as ours. Same discipline as the lib's: the
 * sequence comes from the cart's own ACKSEQ, never a local counter. */
static bool raw_finish_long(void)
{
    unsigned char want = (unsigned char)(FN_ACKSEQ + 1);
    unsigned int outer, inner;

    if (want == 0)
        want = 1;               /* 0 means "never used" */
    fn_regwr(FNR_SEQ, want);

    for (outer = 0; outer < 48000u; outer++) {
        for (inner = 0; inner < 250u; inner++) {
            if (FN_ACKSEQ == want)
                return FN_ERRCODE == FN_OK && FN_REPLYCMD == FUJICMD_ACK;
        }
    }
    return false;
}

bool fnraw_open_directory(unsigned char host_slot,
                          const char *dirpath, const char *pattern)
{
    unsigned int n = 2;         /* the two NULs */
    const char *p;

    raw_begin(FUJICMD_OPEN_DIRECTORY);
    raw_param8(host_slot, 1);
    for (p = dirpath; *p; p++, n++)
        raw_tx((unsigned char)*p);
    raw_tx(0);
    for (p = pattern; *p; p++, n++)
        raw_tx((unsigned char)*p);
    raw_tx(0);
    while (n++ < PAYLOAD_LEN)
        raw_tx(0);
    return raw_finish();
}

bool fnraw_write_host_slot(unsigned char slot, const char *name)
{
    volatile unsigned char *r;
    unsigned char i, j;

    /* Fresh window first: the write streams the other seven slots straight
     * back out of it. The reply is only repainted by a commit, so it holds
     * still while the TX stream is built. */
    if (!FUJICALL(FUJICMD_READ_HOST_SLOTS))
        return false;

    raw_begin(FUJICMD_WRITE_HOST_SLOTS);
    for (i = 0; i < 8; i++) {
        if (i == slot) {
            raw_tx_padded(name, 32);
        } else {
            r = FN_REPLY + (unsigned int)i * 32;
            for (j = 0; j < 32; j++)
                FN_TOUCH(FN_TXPAGE + r[j]);
        }
    }
    return raw_finish();
}

static void raw_devpath_params(unsigned char cmd, unsigned char dev,
                               unsigned char host_slot, unsigned char mode)
{
    raw_begin(cmd);
    raw_param8(dev, 1);
    raw_param8(host_slot, 2);
    raw_param8(mode, 3);
}

bool fnraw_set_device_path_from_reply(unsigned char dev, unsigned char host_slot,
                                      unsigned char mode, const char *prefix,
                                      volatile unsigned char *name)
{
    unsigned int n = 0;

    raw_devpath_params(FUJICMD_SET_DEVICE_FULLPATH, dev, host_slot, mode);
    while (*prefix && n < PAYLOAD_LEN) {
        FN_TOUCH(FN_TXPAGE + (unsigned char)*prefix++);
        n++;
    }
    while (n < PAYLOAD_LEN) {
        unsigned char c = *name++;

        if (c == 0)
            break;
        FN_TOUCH(FN_TXPAGE + c);
        n++;
    }
    while (n++ < PAYLOAD_LEN)
        FN_TOUCH(FN_TXPAGE);
    return raw_finish();
}

bool fnraw_set_device_path(unsigned char dev, unsigned char host_slot,
                           unsigned char mode, const char *fullpath)
{
    raw_devpath_params(FUJICMD_SET_DEVICE_FULLPATH, dev, host_slot, mode);
    raw_tx_padded(fullpath, PAYLOAD_LEN);
    return raw_finish();
}

bool fnraw_set_ssid(const char *ssid, const char *password)
{
    raw_begin(FUJICMD_SET_SSID);
    raw_param8(0, 1);           /* required, value ignored */
    raw_tx_padded(ssid, 33);
    raw_tx_padded(password, 64);
    return raw_finish();
}

bool fnraw_copy_file(unsigned char src_slot1, unsigned char dst_slot1,
                     const char *source_spec, const char *dest_dir)
{
    const char *base = source_spec;
    const char *p;

    /* Only the BASENAME of the source lands in the destination directory --
     * same strrchr() fix as src/perform_copy.c, so a source that itself
     * contains '/' is never asked for as a nested path the destination has
     * no directories for. */
    for (p = source_spec; *p; p++)
        if (*p == '/')
            base = p + 1;

    raw_begin(FUJICMD_COPY_FILE);
    raw_param8(src_slot1, 1);
    raw_param8(dst_slot1, 2);
    raw_tx_str(source_spec);
    raw_tx('|');
    raw_tx_str(dest_dir);       /* always ends in '/' */
    raw_tx_str(base);
    return raw_finish_long();
}
