#ifndef MSX_GFXUTIL_H
#define MSX_GFXUTIL_H

#define CH_BOX_UL   0x93
#define CH_BOX_U    0x94
#define CH_BOX_UR   0x95
#define CH_BOX_L    0x96
#define CH_BOX_R    0x97
#define CH_BOX_BL   0x98
#define CH_BOX_B    0x99
#define CH_BOX_BR   0x9A
#define CH_TAB_L    0x9B
#define CH_TAB_R    0x9C

extern const unsigned char row_pattern[256];
extern const unsigned char udg[256];
extern const unsigned char blank[8];
extern const unsigned char pill_left[8];
extern const unsigned char pill_right[8];

#endif // MSX_GFXUTIL_H