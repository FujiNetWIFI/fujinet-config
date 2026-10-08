/* state.h -- the shared globals and the seams between the screen modules.
 *
 * One invariant to keep when adding code: the reply window at $B000 is only
 * repainted by committing a transaction. Any flow that edits something and
 * writes it back must therefore re-read first and stream out of the fresh
 * window (the way rename and the lobby claim do) -- never hold a window
 * across an intervening transaction. fn_edit() runs no transactions, which
 * is what makes "fetch, edit, stream back" safe.
 */

#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

#include <fujinet-fuji.h>
#include <fujinet-sms.h>
#include <fujinet-bus-sms.h>

#include "constants.h"

extern unsigned char state;

extern unsigned char host;       /* the mounted/browsed host slot */
extern unsigned char src_host;   /* copy: the source's host slot */
extern unsigned char copy_mode;  /* nonzero while a copy source is marked */

extern unsigned char cur;        /* selected row within the window */
extern unsigned char nrows;      /* rows the current list actually has */
extern unsigned char at_end;     /* the visible page ends the directory */
extern unsigned int  top;        /* first visible entry (16-bit: big dirs) */
extern unsigned char hosts_mask; /* bit i set = host slot i is nonempty */

extern char path[PATH_MAX_LEN];  /* current dir: '/'-prefixed, '/'-terminated */
extern char src_spec[SPEC_MAX_LEN]; /* copy source full path; also the
                                       custom-SSID scratch (never both) */

/* config.c */
void status_line(const char *s);            /* message line 1, typed out */
void status_now(const char *s);             /* line 1 at once, sound settled:
                                               say it before anything long */
void legend_line(const char *s);            /* message line 2 */
void fail(const char *what);                /* error buzz, line 1 + FN_ERRCODE */
void fail_code(const char *what, unsigned char code);
void draw_frame(const char *title);         /* main window cleared + retitled,
                                               message lines blank */
void wait_frames(unsigned char n);          /* n frames, input left alone */
void list_select(unsigned char i, bool on); /* the cursor onto/off list row i */
void bar_move(signed char d);               /* one-page lists: moves, wraps */
/* Pause: pick one of a screen's actions from a Phantasy Star COMMAND pop-up;
 * returns the event that action stands for, or IN_NONE. */
unsigned char menu_pick(const char *const *names, const unsigned char *events,
                        unsigned char n);

/* One screen per state; each draws itself, runs its own event loop, and
 * returns once it has set `state` to something else. */
void st_check_wifi(void);
void st_set_wifi(void);
void st_connect_wifi(void);
void st_hosts(void);
void st_files(void);
void st_info(void);
void st_lobby(void);

/* st_files.c -- shared with the copy and boot flows */
bool dir_seek(unsigned int pos);
bool dir_read_entry(unsigned char maxlen);  /* false at end/error; guards the
                                               '.'/'..' poison, see the impl */
bool dir_open(void);                        /* OPEN_DIRECTORY: host, path,
                                               filter (= fn_entry) */
void files_draw(void);                      /* full redraw */
void files_legend(void);                    /* just the message lines */
void leave_host(void);                      /* close dir, back to ST_HOSTS */

/* st_copy.c */
void copy_mark(void);                       /* COPY FILE, no copy in flight */
void copy_here(void);                       /* COPY HERE, copy in flight */
void copy_cancel(void);                     /* back to the source browse */

/* st_boot.c */
void boot_reply_entry(void);                /* boot path + name-in-window;
                                               returns only on failure */
void boot_mount_swap(void);                 /* MOUNT_IMAGE + progress + swap;
                                               returns only on failure */

#endif /* STATE_H */
