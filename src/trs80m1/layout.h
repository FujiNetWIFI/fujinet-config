/**
 * @file layout.h
 * @brief Where things go on the Model I's 64x16 screen.
 *
 * Line 0 carries the title and the WiFi status, line 1 names the pane, a
 * divider closes the list and under it come an info line and two lines of
 * key hints. Hosts and drives share the list area, one pane at a time.
 */

#ifndef TRS80M1_LAYOUT_H
#define TRS80M1_LAYOUT_H

#define TITLE_Y         0
#define PANE_Y          1   /* divider naming the pane */
#define LIST_Y          2   /* first line of the list */
#define LIST_ROWS       10  /* lines 2-11 */
#define FOOT_Y          12  /* divider above the info and hint lines */
#define INFO_Y          13
#define HINT1_Y         14
#define HINT2_Y         15

#define HOSTS_Y         2
#define DRIVES_Y        2
#define PICK_DRIVES_Y   4   /* the drive list under a chosen file */

#define BAR_X0          0   /* the bar's > */
#define SLOT_X          2   /* slot number */
#define NAME_X          4   /* host name */
#define MODE_X          4   /* R/O, R/W */
#define FILE_X          8   /* drive's image */
#define FILE_W          34
#define DRIVE_HOST_X    43  /* drive's host name */
#define DRIVE_HOST_W    15
#define STATE_X         59  /* a drive's state while mounting */
#define ENTRY_X         2   /* directory entries */
#define ENTRY_W         62
#define TEXT_X          1   /* info and other text */
#define TEXT_W          62

#endif /* TRS80M1_LAYOUT_H */
