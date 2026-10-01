#include <fujinet-fuji.h>
#include <fujinet-bus-nes.h>

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
    fn_tx(1);
    fn_tx(v);
    fn_regwr(FNR_NPARAM, count);
}

static void raw_tx_padded(const char *s, unsigned int total)
{
    unsigned int n = 0;

    while (*s && n < total) {
        fn_tx((unsigned char)*s++);
        n++;
    }
    while (n++ < total)
        fn_tx(0);
}

static bool raw_finish(void)
{
    return fn_commit() == FN_OK && FN_REPLYCMD == FUJICMD_ACK;
}

bool fnraw_open_directory(unsigned char host_slot,
                          const char *dirpath, const char *pattern)
{
    unsigned int n = 2;         /* the two NULs */
    const char *p;

    raw_begin(FUJICMD_OPEN_DIRECTORY);
    raw_param8(host_slot, 1);
    for (p = dirpath; *p; p++, n++)
        fn_tx((unsigned char)*p);
    fn_tx(0);
    for (p = pattern; *p; p++, n++)
        fn_tx((unsigned char)*p);
    fn_tx(0);
    while (n++ < PAYLOAD_LEN)
        fn_tx(0);
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
                fn_tx(r[j]);
        }
    }
    return raw_finish();
}

bool fnraw_set_device_path_from_reply(unsigned char dev, unsigned char host_slot,
                                      unsigned char mode, const char *prefix,
                                      volatile unsigned char *name)
{
    unsigned int n = 0;

    raw_begin(FUJICMD_SET_DEVICE_FULLPATH);
    raw_param8(dev, 1);
    raw_param8(host_slot, 2);
    raw_param8(mode, 3);
    while (*prefix && n < PAYLOAD_LEN) {
        fn_tx((unsigned char)*prefix++);
        n++;
    }
    while (n < PAYLOAD_LEN) {
        unsigned char c = *name++;

        if (c == 0)
            break;
        fn_tx(c);
        n++;
    }
    while (n++ < PAYLOAD_LEN)
        fn_tx(0);
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
