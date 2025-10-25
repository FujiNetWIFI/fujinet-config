/**
 * Bar routines
 */

#include <video/tms99x8.h>
#include <stdbool.h>
#include <conio.h>

#define DISPLAY_ROWS 17
#define DISPLAY_LENGTH 60
#define ADDR_PATH_LINE MODE2_ATTR
#define ATTR_PATH_LINE 0xF5
#define ADDR_FILE_LIST ADDR_PATH_LINE + 256
#define ATTR_FILE_LIST 0x1F
#define ADDR_FILE_TYPE ADDR_FILE_LIST
#define ATTR_FILE_TYPE 0xF5
#define ATTR_BAR 0x13

#define ROW(x) (x<<8)
#define COL(x) (x<<3)

/**
 * static local variables for bar y, max, and index.
 */
// static unsigned char bar_y=3, bar_c=1, bar_m=1, bar_i=0, bar_oldi=0, bar_co=0x13;
static unsigned char bar_y=3, bar_c=3, bar_m=1, bar_i=0, bar_oldi=0, bar_co=0x17;

/**
* Reset text to default
*/
void bar_clear(bool oldRow)
{
  // Fill column color in old row
  // vdp_vfill(ADDR_FILE_LIST + ROW(bar_y+(oldRow==true ? bar_oldi : bar_i)), ATTR_PATH_LINE, COL(bar_c));

  // Fill background color in old row
  // vdp_vfill(ADDR_FILE_LIST + ROW(bar_y+(oldRow==true ? bar_oldi : bar_i))+COL(bar_c),ATTR_FILE_LIST,256-COL(bar_c));
  //
  //

  gotoxy(2, bar_y+(oldRow==true ? bar_oldi : bar_i)+1);
  cputc(' ');
  gotoxy(30, bar_y+(oldRow==true ? bar_oldi : bar_i)+1);
  cputc(' ');

  uint16_t addr = ADDR_FILE_LIST + ROW(bar_y+(oldRow==true ? bar_oldi : bar_i))+COL(bar_c)-8;
  for (uint8_t i = 0; i < 256-COL(bar_c); i++) {
    vdp_vpoke(addr++, i % 2 ? 0xF4 : 0xF5);
  }
}

/**
 * Update bar display
 */
void bar_update(void)
{
    int coff = bar_c << 3;

    bar_clear(true);

    gotoxy(2, bar_y+bar_i+1);
    // cputc('<');
    cputc(0x91);
    gotoxy(30, bar_y+bar_i+1);
    // cputc('>');
    cputc(0x92);

    // Fill bar color in new row
    // vdp_vfill(ADDR_FILE_LIST + ROW(bar_y+bar_i) + coff,bar_co,256 - coff);
    // vdp_vfill(ADDR_FILE_LIST + ROW(bar_y+bar_i) + coff,0x4F,256 - coff);
    // vdp_vfill(ADDR_FILE_LIST + ROW(bar_y+bar_i) + coff,0x1F,256 - coff);

    // white
    // vdp_vfill(ADDR_FILE_LIST + ROW(bar_y+bar_i) + coff, 0x1F, 256 - coff - 16);
    // cyan
    // vdp_vfill(ADDR_FILE_LIST + ROW(bar_y+bar_i) + coff, 0x17, 256 - coff - 16);
    // fuschia
    // vdp_vfill(ADDR_FILE_LIST + ROW(bar_y+bar_i) + coff, 0xFD, 256 - coff - 16);
    // red
    vdp_vfill(ADDR_FILE_LIST + ROW(bar_y+bar_i) + coff, 0xF6, 256 - coff - 16);

    uint16_t addr = ADDR_FILE_LIST + ROW(bar_y+bar_i) + coff - 8;
    for (uint8_t i = 0; i < 8; i++) {
      // white
      // vdp_vpoke(addr, i % 2 ? 0x4F : 0x5F);
      // vdp_vpoke(addr + 28*8, i % 2 ? 0x4F : 0x5F);
      // cyan
      // vdp_vpoke(addr, i % 2 ? 0x47 : 0x57);
      // vdp_vpoke(addr + 28*8, i % 2 ? 0x47 : 0x57);
      // fuschia
      // vdp_vpoke(addr, i % 2 ? 0x4D : 0x5D);
      // vdp_vpoke(addr + 28*8, i % 2 ? 0x4D : 0x5D);
      // red
      // vdp_vpoke(addr, i % 2 ? 0x46 : 0x56);
      // vdp_vpoke(addr + 28*8, i % 2 ? 0x46 : 0x56);
      // red - inverted
      vdp_vpoke(addr, i % 2 ? 0x64 : 0x65);
      vdp_vpoke(addr + 28*8, i % 2 ? 0x64 : 0x65);


      addr++;
    }

    // uint16_t addr = ADDR_FILE_LIST + ROW(bar_y+bar_i) + coff;
    // for (uint8_t i = 0; i < 256-coff; i++) {
    //   // vdp_vpoke(addr++, i % 2 ? 0xF5 : 0xF7);
    //   // vdp_vpoke(addr++, i % 2 ? 0x15 : 0x17);
    //   vdp_vpoke(addr++, i % 2 ? 0x4F : 0x5F);
    // }
}

/**
 * Set up bar and start display on row
 * @param y Y column for bar display
 * @param c column size in characters
 * @param m number of items
 * @param i item index to start display
 */
void bar_set(unsigned char y, unsigned char c, unsigned char m, unsigned char i)
{
  bar_y = y;
  bar_c = c;
  bar_m = (m == 0 ? 0 : m-1);
  bar_i = i;
  bar_oldi = bar_i;
  bar_update();
}

/**
 * Move bar upward until index 0
 */
void bar_up()
{
  bar_oldi=bar_i;

  if (bar_i > 0)
    {
      bar_i--;
      bar_update();
    }
}

/**
 * Move bar downward until index m
 */
void bar_down()
{
  bar_oldi=bar_i;

  if (bar_i < bar_m)
    {
      bar_i++;
      bar_update();
    }
}

/**
 * Jump to bar slot
 */
void bar_jump(unsigned char i)
{
  bar_oldi=bar_i;
  bar_i=i;
  bar_update();
}

/**
 * Get current bar position
 * @return bar index
 */
unsigned char bar_get()
{
  return bar_i;
}

void bar_color(unsigned char f,unsigned char b)
{
  bar_co = (f << 4) | b;
}
