#include "../screen.h"

#define GRAPHICS_0_SCREEN_SIZE (40 * 25) // Defines the memory size in bytes
#define DISPLAY_LIST 0x0600              // Memory address to store DISPLAY_LIST.  0x0600 is the first address available for user space memory (1)
#define DISPLAY_MEMORY 0x7400            // Memory address to store DISPLAY_MEMORY.
#define FONT_MEMORY 0x7800

#define CH_FOLDER "\x04"      // Set the character folder to #
#define CH_SERVER "\x06"      // Set the server folder to atari heart.
#define CH_KEY_LABEL_L "\xD9" // Left arrow on the keyboard
#define CH_KEY_LABEL_R "\x19" // Right arrow on the keyboard

// Inverse of internal character codes
#define CH_INV_MINUS "\x8d"
#define CH_INV_LT "\x9C"
#define CH_INV_GT "\x9E"

#define CH_INV_A "\xA1"
#define CH_INV_B "\xA2"
#define CH_INV_C "\xA3"
#define CH_INV_D "\xA4"
#define CH_INV_E "\xA5"
#define CH_INV_F "\xA6"
#define CH_INV_G "\xA7"
#define CH_INV_H "\xA8"
#define CH_INV_I "\xA9"
#define CH_INV_J "\xAA"
#define CH_INV_K "\xAB"
#define CH_INV_L "\xAC"
#define CH_INV_M "\xAD"
#define CH_INV_N "\xAE"
#define CH_INV_O "\xAF"
#define CH_INV_P "\xB0"
#define CH_INV_Q "\xB1"
#define CH_INV_R "\xB2"
#define CH_INV_S "\xB3"
#define CH_INV_T "\xB4"
#define CH_INV_U "\xB5"
#define CH_INV_V "\xB6"
#define CH_INV_W "\xB7"
#define CH_INV_X "\xB8"
#define CH_INV_Y "\xB9"
#define CH_INV_Z "\xBA"

#define CH_INV_1 "\x91"
#define CH_INV_2 "\x92"
#define CH_INV_3 "\x93"
#define CH_INV_4 "\x94"
#define CH_INV_5 "\x95"
#define CH_INV_6 "\x96"
#define CH_INV_7 "\x97"
#define CH_INV_8 "\x98"
#define CH_INV_9 "\x99"
#define CH_INV_LEFT "\xDE"
#define CH_INV_RIGHT "\xDF"

#define CH_KEY_1TO8 CH_KEY_LABEL_L CH_INV_1 CH_INV_MINUS CH_INV_8 CH_KEY_LABEL_R
#define CH_KEY_ESC CH_KEY_LABEL_L CH_INV_E CH_INV_S CH_INV_C CH_KEY_LABEL_R
#define CH_KEY_TAB CH_KEY_LABEL_L CH_INV_T CH_INV_A CH_INV_B CH_KEY_LABEL_R
#define CH_KEY_DELETE CH_KEY_LABEL_L CH_INV_D CH_INV_E CH_INV_L CH_INV_E CH_INV_T CH_INV_E CH_KEY_LABEL_R
#define CH_KEY_LEFT CH_KEY_LABEL_L CH_INV_LEFT CH_KEY_LABEL_R
#define CH_KEY_RIGHT CH_KEY_LABEL_L CH_INV_RIGHT CH_KEY_LABEL_R
#define CH_KEY_RETURN CH_KEY_LABEL_L CH_INV_R CH_INV_E CH_INV_T CH_INV_U CH_INV_R CH_INV_N CH_KEY_LABEL_R
#define CH_KEY_OPTION CH_KEY_LABEL_L CH_INV_O CH_INV_P CH_INV_T CH_INV_I CH_INV_O CH_INV_N CH_KEY_LABEL_R
#define CH_KEY_C CH_KEY_LABEL_L CH_INV_C CH_KEY_LABEL_R
#define CH_KEY_N CH_KEY_LABEL_L CH_INV_N CH_KEY_LABEL_R
#define CH_KEY_F CH_KEY_LABEL_L CH_INV_F CH_KEY_LABEL_R
#define CH_KEY_LT CH_KEY_LABEL_L CH_INV_LT CH_KEY_LABEL_R
#define CH_KEY_GT CH_KEY_LABEL_L CH_INV_GT CH_KEY_LABEL_R
#define CH_KEY_ESC CH_KEY_LABEL_L CH_INV_E CH_INV_S CH_INV_C CH_KEY_LABEL_R

extern void bar_setup_regs();
void fastcall bar_show(unsigned char y);
