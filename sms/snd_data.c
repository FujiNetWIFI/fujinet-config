/* snd_data.c -- CONFIG's sounds, written for this project in the manner of
 * Phantasy Star's: effects doubled on T1+T2 a few period units apart, a lead
 * that decays and only then starts to waver, a swelling bass, a plucked
 * arpeggio, noise hats and snare. The phrases are our own; nothing here is
 * taken from the original's data. Stream format: fujisnd.h.
 */

#include "fujisnd.h"

/* Notes as semitones above A2: N_C(4) is middle C. */
#define N_C(o)   (3 + 12 * ((o) - 3))
#define N_CS(o)  (N_C(o) + 1)
#define N_D(o)   (N_C(o) + 2)
#define N_E(o)   (N_C(o) + 4)
#define N_F(o)   (N_C(o) + 5)
#define N_FS(o)  (N_C(o) + 6)
#define N_G(o)   (N_C(o) + 7)
#define N_A(o)   (N_C(o) + 9)
#define N_AS(o)  (N_C(o) + 10)
#define N_B(o)   (N_C(o) + 11)

#define NOTE(n, len)  (n), (len)
#define REST(len)     0x60, (len)
#define RAW(p, len)   (0x64 | ((p) >> 8)), ((p) & 0xFF), (len)
#define ENV(e)        (0x70 | (e))
#define VOL(v)        (0x80 | (v))
#define VIB(on)       (0x90 | (on))
#define END           0xFF

/* Noise control: white noise at the PSG's two highest shift rates. */
#define NZ_HAT    0x04
#define NZ_SNARE  0x05

/* ---- envelopes: attenuation per frame (0 loudest, 15 off), $FE holds ---- */

enum { ENV_FLAT, ENV_BLIP, ENV_LEAD, ENV_BASS, ENV_PLUCK, ENV_HAT, ENV_SNARE,
       ENV_CHIME, ENV_BUZZ, ENV_SPARK, ENV_TICK };

static const unsigned char env_flat[]  = { 0, 0xFE };
static const unsigned char env_blip[]  = { 0, 0, 1, 2, 2, 3, 15, 0xFE };
static const unsigned char env_lead[]  = { 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4,
                                           5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8,
                                           8, 8, 9, 9, 9, 9, 10, 10, 10, 10, 11, 0xFE };
static const unsigned char env_bass[]  = { 6, 5, 4, 5, 6, 6, 7, 7, 8, 0xFE };
static const unsigned char env_pluck[] = { 0, 2, 4, 6, 8, 10, 0xFE };
static const unsigned char env_hat[]   = { 4, 6, 15, 0xFE };
static const unsigned char env_snare[] = { 2, 2, 3, 4, 4, 5, 6, 6, 8, 10, 12, 15, 0xFE };
static const unsigned char env_chime[] = { 0, 1, 1, 2, 2, 3, 3, 4, 5, 6, 7, 9, 11, 15, 0xFE };
static const unsigned char env_buzz[]  = { 1, 1, 1, 2, 2, 3, 3, 4, 4, 5, 6, 7, 8, 10, 12,
                                           15, 0xFE };
static const unsigned char env_spark[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 0xFE };
static const unsigned char env_tick[]  = { 0, 3, 15, 0xFE };

const unsigned char *const snd_envs[] = {
    env_flat, env_blip, env_lead, env_bass, env_pluck, env_hat, env_snare,
    env_chime, env_buzz, env_spark, env_tick
};

/* ---- effects ------------------------------------------------------------ */

/* The blip: about 432 Hz, gone in six frames. */
static const unsigned char move_t1[] = { ENV(ENV_BLIP), RAW(0x103, 7), END };
static const unsigned char ok_t1[] = {
    ENV(ENV_BLIP), NOTE(N_A(4), 3), NOTE(N_E(5), 7), END
};
static const unsigned char back_t1[] = {
    ENV(ENV_BLIP), NOTE(N_A(4), 3), NOTE(N_E(4), 7), END
};
static const unsigned char type_nz[] = { ENV(ENV_TICK), NOTE(NZ_HAT, 3), END };

/* Down a tritone, low and wide, so the two copies beat against each other. */
static const unsigned char error_t1[] = {
    ENV(ENV_BUZZ), NOTE(N_FS(3), 6), NOTE(N_C(3), 16), END
};

/* Two rising C major arpeggios, the second a frame behind and a third up. */
static const unsigned char connect_t1[] = {
    ENV(ENV_SPARK), NOTE(N_C(5), 3), NOTE(N_E(5), 3), NOTE(N_G(5), 3),
    NOTE(N_C(6), 3), NOTE(N_E(6), 3), NOTE(N_G(6), 3), NOTE(N_C(7), 8), END
};
static const unsigned char connect_t2[] = {
    ENV(ENV_SPARK), VOL(3), REST(1), NOTE(N_E(5), 3), NOTE(N_G(5), 3),
    NOTE(N_C(6), 3), NOTE(N_E(6), 3), NOTE(N_G(6), 3), NOTE(N_C(7), 3),
    NOTE(N_E(7), 6), END
};

/* Pick-up notes into a ringing high B. */
static const unsigned char ready_t1[] = {
    ENV(ENV_CHIME), NOTE(N_G(4), 2), REST(1), NOTE(N_B(4), 2), REST(1),
    NOTE(N_D(5), 2), NOTE(N_G(5), 2), NOTE(N_B(5), 12), END
};

/* ---- the splash fanfare: Dm Bb | C A, a 12-frame beat, 192 frames ------- */

#define U 12

static const unsigned char title_lead[] = {
    ENV(ENV_LEAD), VIB(1),
    NOTE(N_D(5), 2 * U), NOTE(N_A(4), U), NOTE(N_D(5), U),
    NOTE(N_F(5), 2 * U), NOTE(N_G(5), U), NOTE(N_F(5), U),
    NOTE(N_E(5), U), NOTE(N_C(5), U), NOTE(N_E(5), U), NOTE(N_G(5), U),
    NOTE(N_A(5), 4 * U),
    END
};

#define BASS_HALF(n) NOTE(n, 18), NOTE(n, 6), NOTE(n, 24)

static const unsigned char title_bass[] = {
    ENV(ENV_BASS),
    BASS_HALF(N_D(3)), BASS_HALF(N_AS(2)),
    BASS_HALF(N_C(3)), BASS_HALF(N_A(2)),
    END
};

#define ARP(a, b, c, d) NOTE(a, 6), NOTE(b, 6), NOTE(c, 6), NOTE(d, 6), \
                        NOTE(b, 6), NOTE(c, 6), NOTE(d, 6), NOTE(c, 6)

static const unsigned char title_arp[] = {
    ENV(ENV_PLUCK), VOL(4),
    ARP(N_D(4), N_F(4), N_A(4), N_D(5)),
    ARP(N_AS(3), N_D(4), N_F(4), N_AS(4)),
    ARP(N_C(4), N_E(4), N_G(4), N_C(5)),
    ARP(N_A(3), N_CS(4), N_E(4), N_A(4)),
    END
};

#define DRUM_BAR ENV(ENV_HAT), NOTE(NZ_HAT, 6), NOTE(NZ_HAT, 6), NOTE(NZ_HAT, 6), \
                 NOTE(NZ_HAT, 6), ENV(ENV_SNARE), NOTE(NZ_SNARE, 12), \
                 ENV(ENV_HAT), NOTE(NZ_HAT, 6), NOTE(NZ_HAT, 6)

static const unsigned char title_drums[] = {
    VOL(1), DRUM_BAR, DRUM_BAR, DRUM_BAR, DRUM_BAR, END
};

const snd_def snd_defs[] = {
    /* SND_MOVE    */ { { 0, move_t1, 0, 0 }, 9, 5 },
    /* SND_OK      */ { { 0, ok_t1, 0, 0 }, 9, 4 },
    /* SND_BACK    */ { { 0, back_t1, 0, 0 }, 9, 4 },
    /* SND_TYPE    */ { { 0, 0, 0, type_nz }, 0, 3 },
    /* SND_ERROR   */ { { 0, error_t1, 0, 0 }, 20, 2 },
    /* SND_CONNECT */ { { 0, connect_t1, connect_t2, 0 }, 0, 1 },
    /* SND_READY   */ { { 0, ready_t1, 0, 0 }, 8, 1 },
    /* SND_TITLE   */ { { title_lead, title_bass, title_arp, title_drums }, 0, 0 },
};
