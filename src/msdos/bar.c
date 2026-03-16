#ifdef BUILD_MSDOS

/**
 * @brief   Bar routines for MS-DOS
 * @author  Thomas Cherryhomes
 * @email   thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details.
 */

#include <stdlib.h> // for NULL.
#include <dos.h>
#include "screen.h"

/**
 * static local variables for bar y, max, and index.
 */
static int bar_y=3, bar_c=1, bar_m=1, bar_i=0, bar_oldi=0;

/* Two-buffer save/restore for mixed row attributes.
 * saved_current holds the original (pre-highlight) attributes of the currently
 * highlighted row.  saved_next is a scratch buffer used during bar_update.
 * saved_current_row tracks which absolute screen row saved_current belongs to. */
static unsigned char saved_current[80];
static unsigned char saved_next[80];
static int saved_current_row = -1;

static void save_row_attrs(int y, unsigned char *buf)
{
    int o = (y * screen_cols + bar_c) * 2 + 1;
    int w = screen_cols - 2 * bar_c;
    int i;
    if (w > 80) w = 80;
    for (i = 0; i < w; i++) { buf[i] = (unsigned char)video[o]; o += 2; }
}

static void restore_row_attrs(int y, unsigned char *buf)
{
    int o = (y * screen_cols + bar_c) * 2 + 1;
    int w = screen_cols - 2 * bar_c;
    int i;
    if (w > 80) w = 80;
    for (i = 0; i < w; i++) { video[o] = buf[i]; o += 2; }
}

/**
 * @brief Set up bar and start display on row.
 * @param y Starting row for bar display.
 * @param c Column size in characters (left/right margin).
 * @param m Total number of items.
 * @param i Item index to start display at.
 */
void bar_set(unsigned char y, unsigned char c, unsigned char m, unsigned char i)
{
  bar_oldi = bar_i;
  bar_y = y;
  bar_c = c;
  bar_m = (m == 0 ? 0 : m-1);
  bar_i = i;
  bar_update();
}

/**
 * @brief Clear the highlight attribute from the current or previous bar row.
 * @param old If true, clear the previously highlighted row; if false, clear the current row.
 */
void bar_clear(bool old)
{
  if (old)
    {
      bar_draw(bar_y+bar_oldi, true);
    }
  else
    {
      int row = bar_y + bar_i;
      if (saved_current_row == row)
        restore_row_attrs(row, saved_current);
      else
        bar_draw(row, true);
      saved_current_row = -1;
    }
}

/**
 * @brief Draw or clear the highlight bar at a given screen row.
 *
 * Respects bar_c as the left/right margin so box borders at column 0 and
 * screen_cols-1 are not overwritten when bar_c > 0.
 *
 * @param y     Absolute screen row to draw on.
 * @param clear If true, restore normal attribute; if false, apply selected attribute.
 */
void bar_draw(int y, bool clear)
{
    int o = (y * screen_cols + bar_c) * 2 + 1;
    int w = screen_cols - 2 * bar_c;
    int i=0;

    for (i=0;i<w;i++)
    {
        if (clear)
	{
            video[o] = ATTRIBUTE_NORMAL;
	}
        else
	{
            video[o] = ATTRIBUTE_SELECTED;
	}

        o += 2;
    }
}

/**
 * Get current bar position
 * @return bar index
 */
uint_fast8_t bar_get()
{
  return bar_i;
}

/**
 * @brief Move the bar highlight up by one item, stopping at index 0.
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
 * @brief Move the bar highlight down by one item, stopping at the last index.
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
 * @brief Jump the bar directly to a given index and redraw.
 * @param i Item index to jump to.
 */
void bar_jump(uint_fast8_t i)
{
  bar_i=i;
  bar_update();
}

/**
 * @brief Redraw the bar: restore the old row's original attributes, then
 *        highlight the new row, preserving any mixed BOLD/NORMAL content.
 */
void bar_update(void)
{
  int new_row = bar_y + bar_i;
  int old_row = bar_y + bar_oldi;

  /* Save new row's attributes BEFORE anything modifies them.
   * This must happen first so that when old_row == new_row (initialisation),
   * we capture the real mixed attrs before bar_draw(clear) overwrites them. */
  save_row_attrs(new_row, saved_next);

  /* Restore old row: use saved attrs if available, else fall back to NORMAL. */
  if (saved_current_row == old_row)
    restore_row_attrs(old_row, saved_current);
  else
    bar_draw(old_row, true);

  /* Promote saved_next to saved_current for the row we are about to highlight. */
  {
    int i;
    int w = screen_cols - 2 * bar_c;
    if (w > 80) w = 80;
    for (i = 0; i < w; i++) saved_current[i] = saved_next[i];
  }
  saved_current_row = new_row;

  bar_draw(new_row, false);
}

#endif /* BUILD_MSDOS */
