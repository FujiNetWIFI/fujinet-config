/* fujisnd.c -- the SN76489 on port $7F: the sequencer. See fujisnd.h. */

#include <arch/sms.h>

#include "fujiin.h"
#include "fujisnd.h"

#define MAX_PERIOD 1023

/* A2 up to G#3; each octave above halves the period. */
static const unsigned int note_period[12] = {
    1017, 960, 906, 855, 807, 762, 719, 679, 641, 605, 571, 539
};

/* Vibrato only ever bends flat (a longer period), as the original's does. */
static const unsigned char vib_offset[8] = { 0, 1, 3, 5, 5, 3, 1, 0 };

typedef struct {
    const unsigned char *p;     /* next stream byte; NULL = idle */
    const unsigned char *env0;  /* envelope, from the top */
    const unsigned char *env;   /* envelope, where this note has got to */
    unsigned int period;        /* tone period, or noise control */
    unsigned char left;         /* frames left of this note */
    unsigned char rest;
    unsigned char vol;
    unsigned char vib;
    unsigned char age;          /* frames since the note began */
    unsigned char chorus;       /* T1 only: T2 doubles it, this much flatter */
} voice_t;

static voice_t voice[4];
static bool slaved;             /* T2 is following T1 */

static void psg_tone(unsigned char ch, unsigned int period)
{
    if (period > MAX_PERIOD)
        period = MAX_PERIOD;
    IO_PSG = (unsigned char)(0x80 | (ch << 5) | (period & 15));
    IO_PSG = (unsigned char)((period >> 4) & 0x3F);
}

static void psg_att(unsigned char ch, unsigned char att)
{
    IO_PSG = (unsigned char)(0x90 | (ch << 5) | att);
}

void snd_init(void)
{
    unsigned char k;

    for (k = 0; k < 4; k++) {
        voice[k].p = 0;
        psg_att(k, 15);
    }
    slaved = false;
}

void snd_play(unsigned char id)
{
    const snd_def *d = &snd_defs[id];
    unsigned char k;

    for (k = 0; k < 4; k++) {
        voice_t *v = &voice[k];

        if (!d->v[k])
            continue;
        v->p = d->v[k];
        v->env0 = snd_envs[0];
        v->left = 0;
        v->vol = d->vol;
        v->vib = 0;
        v->chorus = 0;
    }
    if (d->v[2])
        slaved = false;
    if (d->v[1]) {
        if (slaved && !d->chorus && !d->v[2])
            psg_att(2, 15);     /* T2 loses its leader */
        voice[1].chorus = d->chorus;
        slaved = d->chorus != 0;
        if (slaved)
            voice[2].p = 0;
    }
}

bool snd_busy(void)
{
    return voice[0].p || voice[1].p || voice[2].p || voice[3].p;
}

void snd_settle(void)
{
    unsigned char start = in_frames();

    while (snd_busy() && (unsigned char)(in_frames() - start) < 20)
        ;
    snd_init();
}

/* Read the stream up to the next note or rest; false at its end. */
static bool next_note(unsigned char k)
{
    voice_t *v = &voice[k];
    unsigned char b;

    for (;;) {
        b = *v->p++;
        if (b == 0xFF) {
            v->p = 0;
            return false;
        }
        if (b < 0x60) {
            v->period = (k == 3) ? b : (note_period[b % 12] >> (b / 12));
            v->rest = 0;
            break;
        }
        if (b == 0x60) {
            v->rest = 1;
            break;
        }
        if (b < 0x68) {
            v->period = ((unsigned int)(b & 3) << 8) | *v->p++;
            v->rest = 0;
            break;
        }
        if (b < 0x80)
            v->env0 = snd_envs[b & 15];
        else if (b < 0x90)
            v->vol = (unsigned char)(b & 15);
        else
            v->vib = (unsigned char)(b & 1);
    }
    v->left = *v->p++;
    v->env = v->env0;
    v->age = 0;
    if (k == 3 && !v->rest)
        IO_PSG = (unsigned char)(0xE0 | v->period);  /* resets the noise */
    return true;
}

static void voice_tick(unsigned char k)
{
    voice_t *v = &voice[k];
    unsigned char att;
    unsigned int period;

    if (!v->p)
        return;
    if (v->left == 0 && !next_note(k)) {
        psg_att(k, 15);
        if (k == 1 && slaved) {
            psg_att(2, 15);
            slaved = false;
        }
        return;
    }

    if (v->rest) {
        att = 15;
    } else {
        att = (unsigned char)(*v->env + v->vol);
        if (att > 15)
            att = 15;
        if (v->env[1] != 0xFE)
            v->env++;
        if (k < 3) {
            period = v->period;
            if (v->vib && v->age >= 10)
                period += vib_offset[(v->age - 10) & 7];
            psg_tone(k, period);
            if (k == 1 && slaved)
                psg_tone(2, period + v->chorus);
        }
    }
    psg_att(k, att);
    if (k == 1 && slaved)
        psg_att(2, (unsigned char)(att < 15 ? att + 1 : 15));

    if (v->age != 0xFF)
        v->age++;
    v->left--;
}

void snd_tick(void)
{
    voice_tick(0);
    voice_tick(1);
    voice_tick(2);
    voice_tick(3);
}
