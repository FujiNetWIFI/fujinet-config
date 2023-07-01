/**
 * FujiNet for #Adam configuration program
 *
 * Select file from Host Slot
 */

#ifndef SELECT_FILE_H
#define SELECT_FILE_H

#define ENTRY_TIMER_DUR 128

void select_file(void);
void select_display_long_filename(void);
void select_file_set_source_filename(void);
unsigned char select_file_is_folder(void);
unsigned select_file_entry_type(void);

#endif /* SELECT_FILE */
