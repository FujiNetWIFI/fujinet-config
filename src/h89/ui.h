/**
 * @file ui.h
 * @brief The frame, hints, info and status lines - Heathkit H89.
 */

#ifndef H89_UI_H
#define H89_UI_H

#include <stdint.h>
#include <stdbool.h>

/** Start a screen: the frame, its title, and a plain divider over the info
    line. The first pane's divider is left to the screen. */
void ui_frame(void);

/** A divider across the frame on line y, with optional titles left and
    right */
void ui_divider(uint8_t y, const char *left, const char *right);

/** A title at the right end of the divider on line y */
void ui_title_right(uint8_t y, const char *s);

/** A pane's divider, its title in reverse while it has the bar */
void ui_pane(uint8_t y, const char *title, bool active);

/** A prompt on the info line; its field starts at ui_field_x */
void ui_prompt(const char *label);
extern uint8_t ui_field_x;

/** Text centred on line y */
void ui_center(uint8_t y, const char *s);

/** The two hint lines. Text in [brackets] is a key, shown in reverse. */
void ui_hints(const char *line1, const char *line2);

/** The info line inside the bottom of the frame */
void ui_info(const char *s);

/** A message on the info line, sent at once */
void ui_message(const char *s);

/** Text at x,y inside the frame, padded to the right edge */
void ui_line(uint8_t x, uint8_t y, const char *s);

/** A label and its value: "label: value" */
void ui_labelled(uint8_t y, const char *label, const char *value);

/** The status on the 25th line, read fresh from the FujiNet when refresh is
    set */
void ui_status(bool refresh);

/** Decimal digits of n into buf, which must hold 11 bytes; returns buf */
char *ui_num(uint32_t n, char *buf);

#endif /* H89_UI_H */
