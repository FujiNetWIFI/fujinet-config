/**
 * @file layout.h
 * @brief Where things go on the H19's 80x24 screen and its 25th line.
 *
 * Every screen is one frame: the title in the top border, panes divided by
 * lines that carry their names, an info line at the bottom of the frame, two
 * lines of key hints under it, and the status on the 25th line.
 */

#ifndef H89_LAYOUT_H
#define H89_LAYOUT_H

#define FRAME_TOP_Y     0
#define PANE_Y          1   /* divider naming the first pane */
#define LIST_Y          2   /* first line inside it */
#define LIST_ROWS       17  /* lines 2-18 */
#define FOOT_Y          19  /* divider above the info line */
#define INFO_Y          20
#define FRAME_BOTTOM_Y  21
#define HINT1_Y         22
#define HINT2_Y         23
#define STATUS_Y        24

/* Hosts & Devices: two panes of eight */
#define HOSTS_Y         2
#define DRIVES_PANE_Y   10
#define DRIVES_Y        11

/* Columns inside the frame */
#define BAR_X0          1   /* the bar's arrow */
#define BAR_X1          78
#define SLOT_X          3   /* slot number */
#define NAME_X          5   /* host name */
#define MODE_X          5   /* R/O, R/W */
#define FILE_X          9   /* drive's image */
#define FILE_W          36
#define DRIVE_HOST_X    46  /* drive's host name */
#define DRIVE_HOST_W    32
#define ENTRY_X         3   /* directory entries */
#define ENTRY_W         74
#define TEXT_X          2   /* info and other text */
#define TEXT_W          76

#endif /* H89_LAYOUT_H */
