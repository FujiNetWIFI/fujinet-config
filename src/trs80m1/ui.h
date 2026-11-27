/**
 * @file ui.h
 * @brief The title line, dividers, hints and info line - TRS-80 Model I.
 */

#ifndef TRS80M1_UI_H
#define TRS80M1_UI_H

#include <stdint.h>
#include <stdbool.h>

/** Start a screen: the title line with the WiFi status, and the divider over
    the info line. The pane's divider is left to the screen. */
void ui_frame(void);

/** A divider across line row, with optional titles left and right */
void ui_divider(uint8_t row, const char *left, const char *right);

/** A title at the right end of line row */
void ui_title_right(uint8_t row, const char *text);

/** The pane's divider on line row, with its title */
void ui_pane(uint8_t row, const char *title);

/** A prompt on the info line; its field starts at ui_field_x */
void ui_prompt(const char *label);
extern uint8_t ui_field_x;

/** Text centered on line row */
void ui_center(uint8_t row, const char *text);

/** The two hint lines */
void ui_hints(const char *line1, const char *line2);

/** The info line */
void ui_info(const char *text);

/** A message on the info line, written at once */
void ui_message(const char *text);

/** Text at col,row, the rest of the line cleared */
void ui_line(uint8_t col, uint8_t row, const char *text);

/** A label and its value on line row: "label: value" */
void ui_labelled(uint8_t row, const char *label, const char *value);

/** The WiFi status in the title line, read fresh from the FujiNet when
    refresh is set */
void ui_status(bool refresh);

/** Decimal digits of num into buf, which must hold 11 bytes; returns buf */
char *ui_num(uint32_t num, char *buf);

#endif /* TRS80M1_UI_H */
