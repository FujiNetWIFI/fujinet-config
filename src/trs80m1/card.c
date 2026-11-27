#ifdef BUILD_TRS80M1

/**
 * @file card.c
 * @brief The card's settings screen - TRS-80 Model I.
 *
 * The card's RAM, its Expansion Interface (disk, latches, printer) and its
 * RS-232 board are each switched on and off by CONTROL commands to the pipe,
 * not by FujiNet calls. A block switched on answers at once; one switched off
 * goes quiet at the next reset, so CONFIG can't pull its own memory out from
 * under itself. A DIP switch forces a block off whatever is saved.
 */

#include <arch/z80.h>
#include "key_codes.h"
#include "vid.h"
#include "kbd.h"
#include "layout.h"
#include "ui.h"
#include "trs80m1.h"
#include "../screen.h"

#define PIPE_CONTROL 0xD1
#define CTL_FLUSH    0x08
#define CTL_ON       0x40
#define CTL_OFF      0x50
#define CTL_INFO     0x60
#define INFO_LEN     5
#define INFO_WAIT_MS 300

#define BLOCK_RAM    0x01
#define BLOCK_EI     0x02
#define BLOCK_RS232  0x04
#define BLOCKS       3

#define NOW_X        32
#define RESET_X      38

/* fujinet-lib's port layer */
extern int port_getc_timeout(uint16_t timeout) __z88dk_fastcall;

static uint8_t saved, forced_off, in_force;
static const char *note;

static const char *const block_names[BLOCKS] = {
  "32K RAM, 8000H-FFFFH",
  "Expansion Interface: disk",
  "RS-232 modem",
};

/** Ask the card for its settings; false when no Model I card answers */
static bool card_info(void)
{
  static uint8_t info[INFO_LEN], idx;
  static int got;

  z80_outp(PIPE_CONTROL, CTL_FLUSH);
  z80_outp(PIPE_CONTROL, CTL_INFO);
  for (idx = 0; idx < INFO_LEN; idx++)
  {
    got = port_getc_timeout(INFO_WAIT_MS);
    if (got < 0)
      return false;
    info[idx] = (uint8_t)got;
  }
  if (info[0] != 'M' || info[1] != '1')
    return false;

  saved = info[2];
  forced_off = info[3];
  in_force = info[4];
  return true;
}

static void draw(uint8_t selected)
{
  static uint8_t idx, mask, row;

  ui_frame();
  ui_pane(PANE_Y, "Model I Card Settings");
  ui_line(TEXT_X, LIST_Y, NULL);
  vid_text(SLOT_X, LIST_Y, "Block", 5, false);
  vid_text(NOW_X, LIST_Y, "Now", 3, false);
  vid_text(RESET_X, LIST_Y, "After reset", 11, false);

  for (idx = 0; idx < BLOCKS; idx++)
  {
    mask = 1 << idx;
    row = LIST_Y + 1 + idx;
    ui_line(TEXT_X, row, NULL);
    vid_put(SLOT_X, row, '1' + idx);
    vid_text(NAME_X, row, block_names[idx], 27, false);
    vid_text(NOW_X, row, (in_force & mask) ? "On" : "Off", 3, false);
    if (forced_off & mask)
      vid_text(RESET_X, row, "Off (DIP switch)", 16, false);
    else
      vid_text(RESET_X, row, (saved & mask) ? "On" : "Off", 3, false);
  }
  bar_set(LIST_Y + 1, BAR_X0, BLOCKS, selected);

  ui_info(note);
  note = NULL;
  ui_hints("ENTER Switch on/off  1-3 Block", "BREAK Back");
}

/** Switch a block on now and for every reset, or off from the next reset */
static void toggle(uint8_t block)
{
  static uint8_t mask;

  mask = 1 << block;
  if (forced_off & mask)
  {
    note = "A DIP switch holds that block off.";
    return;
  }

  if (saved & mask)
  {
    z80_outp(PIPE_CONTROL, CTL_OFF | mask);
    note = mask == BLOCK_EI ? "Off at the next reset: then no disk drives."
                            : "Off from the next reset.";
  }
  else
  {
    z80_outp(PIPE_CONTROL, CTL_ON | mask);
    note = "On now, and at every reset.";
  }
}

void card_settings(void)
{
  static uint8_t key, selected;

  selected = 0;
  if (!card_info())
  {
    ui_message("No Model I card answered. Press any key.");
    kbd_wait();
    return;
  }

  for (;;)
  {
    draw(selected);
    key = kbd_wait();
    switch (key)
    {
    case KEY_UP_ARROW:
      if (selected)
        selected--;
      break;
    case KEY_DOWN_ARROW:
      if (selected < BLOCKS - 1)
        selected++;
      break;
    case '1':
    case '2':
    case '3':
      selected = key - '1';
      break;
    case KEY_RETURN:
    case KEY_SPACE:
      toggle(selected);
      if (!card_info())
        return;
      break;
    case KEY_ESCAPE:
      return;
    }
  }
}

#endif /* BUILD_TRS80M1 */
