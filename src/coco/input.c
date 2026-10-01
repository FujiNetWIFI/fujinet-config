#ifdef _CMOC_VERSION_

/**
 * Input routines: keyboard, joystick and line editing.
 */

#include "coco_input.h"
#include "../input.h"
#include "../globals.h"
#include "../key_codes.h"

unsigned char selected_network;
unsigned short custom_numSectors;
unsigned short custom_sectorSize;
bool mounting = false;
bool input_abortable = false;
bool input_aborted = false;

#define JOY_CENTER   31
#define JOY_HALF     16
#define JOY_LOW_TH   (JOY_CENTER - JOY_HALF)
#define JOY_HIGH_TH  (JOY_CENTER + JOY_HALF)

#define JOY_REPEAT_DELAY     25
#define JOY_REPEAT_INTERVAL  6

/* Positions are ignored until a button is pressed at least once, so a
   floating (disconnected) analog stick can't produce phantom movement. */
static bool joy_right_selected = false;
static bool joy_left_selected = false;
static bool joy_btn_released = true;
static byte joy_last_dir = 0;
static word joy_next_repeat = 0;

/**
 * @brief this routine is needed because waitkey() and readline()
 *        always emit uppercase. argh!
 */
uint8_t input()
{
	char shift = false;
	char k;

	while (true)
	{
		k = inkey();

		if (isKeyPressed(KEY_PROBE_SHIFT, KEY_BIT_SHIFT))
		{
			shift = 0x00;
		}
		else
		{
			if (k > '@' && k < '[')
				shift = 0x20;
		}

		if (k)
			return k + shift;
	}
}

unsigned char input_ucase()
{
	return 0;
}

/* Bitmask: 1=up 2=down 4=left 8=right 16=btn1 32=btn2. */
static byte readJoystick(void)
{
	byte value = 0;
	bool lbtn1, lbtn2, rbtn1, rbtn2;
	byte h, v;

	byte buttons = readJoystickButtons();

	/* The enum in coco.h has the button masks wrong; use raw masks. */
	lbtn1 = (buttons & 0x02) == 0;
	lbtn2 = (buttons & 0x08) == 0;
	rbtn1 = (buttons & 0x01) == 0;
	rbtn2 = (buttons & 0x04) == 0;

	if (lbtn1 || lbtn2)
	{
		joy_left_selected = true;
		joy_right_selected = false;
	}
	else if (rbtn1 || rbtn2)
	{
		joy_right_selected = true;
		joy_left_selected = false;
	}

	if (!joy_left_selected && !joy_right_selected)
		return 0;

	{
		const byte *joy = readJoystickPositions();

		if (joy_left_selected)
		{
			h = joy[JOYSTK_LEFT_HORIZ];
			v = joy[JOYSTK_LEFT_VERT];
			if (lbtn1) value |= 16;
			if (lbtn2) value |= 32;
		}
		else
		{
			h = joy[JOYSTK_RIGHT_HORIZ];
			v = joy[JOYSTK_RIGHT_VERT];
			if (rbtn1) value |= 16;
			if (rbtn2) value |= 32;
		}

		if (v <= JOY_LOW_TH)  value |= 1;
		if (v >= JOY_HIGH_TH) value |= 2;
		if (h <= JOY_LOW_TH)  value |= 4;
		if (h >= JOY_HIGH_TH) value |= 8;
	}

	return value;
}

unsigned char input_handle_joystick(void)
{
	bool was_active = joy_left_selected || joy_right_selected;
	byte value = readJoystick();
	byte dir = value & 0x0F;
	byte btn = value & 0x30;

	if (btn)
	{
		if (joy_btn_released)
		{
			joy_btn_released = false;
			/* First press only activates the joystick; swallow it so the
			   wake-up press can't accidentally navigate. */
			if (!was_active)
				return 0;
			if (value & 16) return KEY_ENTER;
			if (value & 32) return KEY_BREAK;
		}
		return 0;
	}
	joy_btn_released = true;

	/* Left maps to parent-dir; right is unused. */
	if (dir & 1)  dir = 1;
	else if (dir & 2) dir = 2;
	else if (dir & 4) dir = 4;
	else { joy_last_dir = 0; return 0; }

	if (dir != joy_last_dir)
	{
		joy_last_dir = dir;
		joy_next_repeat = getTimer() + JOY_REPEAT_DELAY;
	}
	else if ((int)(getTimer() - joy_next_repeat) >= 0)
	{
		joy_next_repeat = getTimer() + JOY_REPEAT_INTERVAL;
	}
	else
	{
		return 0;
	}

	switch (dir)
	{
	case 1: return KEY_UP_ARROW;
	case 2: return KEY_DOWN_ARROW;
	case 4: return KEY_LEFT_ARROW;
	}
	return 0;
}

byte waitkey_joystick(void)
{
	byte k;
	for (;;)
	{
		k = inkey();
		if (k == ' ')
		{
			color_toggle();
			return KEY_REDRAW;
		}
		if (k)
			return k;
		k = input_handle_joystick();
		if (k)
			return k;
	}
}

// SHIFT-0 toggles $011A between 0 (lowercase allowed) and nonzero (forced uppercase).
void input_case_indicator(void)
{
  byte case_flag;

  asm
  {
    lda $011A
    sta :case_flag
  }

  screen_puts(SCREEN_COLS - 11, MENU_Y1, case_flag ? "case:ABC" : "case:abc");
}

#define FIELD_RIGHT (SCREEN_COLS - 2)

static void draw_field(byte x0, byte y, const char *b, byte len, bool password, bool cursor)
{
  byte w = FIELD_RIGHT - x0;
  byte i = len > w ? len - w : 0;
  byte x = x0;

  for (; i < len; i++)
    screen_put(x++, y, password ? '*' : b[i]);
  screen_put(x, y, ' ');
  if (cursor)
    screen_put(x, y, 0);
  for (x++; x <= FIELD_RIGHT; x++)
    screen_put(x, y, ' ');
}

void input_line(uint8_t x, uint8_t y, uint8_t unknown, char *c, uint8_t l, bool password)
{
  char *b = c;
  char k = 0;

  while (*c)
    c++;

  while (k != KEY_ENTER)
  {
    draw_field(x, y, b, (byte)(c - b), password, true);

    k = 0;
    while (!k)
    {
      if (password)
        input_case_indicator();
      k = inkey();
    }

    switch (k)
    {
    case KEY_LEFT_ARROW:
      if (c > b)
      {
        c--;
        *c = 0;
      }
      break;
    case KEY_BREAK:
      if (input_abortable)
      {
        input_aborted = true;
        k = KEY_ENTER;
      }
      break;
    case KEY_RIGHT_ARROW:
    case KEY_ENTER:
    case KEY_CLEAR:
      break;
    default:
      if ((c - b) < l)
      {
        *c = k;
        c++;
      }
    }
  }
  draw_field(x, y, b, (byte)(c - b), password, false);
}

#endif
