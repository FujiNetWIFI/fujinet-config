/**
 * FujiNet for #Adam configuration program
 *
 * Select file from Host Slot
 */

#include <string.h>
#include "fuji_typedefs.h"
#include "screen.h"
#include "io.h"
#include "select_file.h"
#include "globals.h"
#include "input.h"
#include "bar.h"

#define ENTRIES_PER_PAGE 15

char path[224];
char filter[32];
DirectoryPosition pos=0;

static enum
  {
   INIT,
   DISPLAY,
   NEXT_PAGE,
   PREV_PAGE,
   CHOOSE,
   DONE
  } subState;

void select_file_init(void)
{
  pos=0;
  memset(path,0,256);
  memset(filter,0,32);
  screen_select_file();
  subState=DISPLAY;
}

unsigned char select_file_display(void)
{
  char visibleEntries=0;
  
  io_mount_host_slot(selected_host_slot);

  if (io_error())
    {
      screen_error("  COULD NOT MOUNT HOST SLOT.");
      subState=DONE;
      state=SET_WIFI;
      return;
    }

  screen_select_file_display(path,filter);
  
  io_open_directory(selected_host_slot,path,filter);
  
  if (io_error())
    {
      screen_error("  COULD NOT OPEN DIRECTORY.");
      subState=DONE;
      state=SET_WIFI;
      return;
    }
  
  if (pos>0)
    io_set_directory_position(pos);
  
  for (char i=0;i<ENTRIES_PER_PAGE;i++)
    {
      char *e = io_read_directory(31,0);
      if (e[1]==0x7F)
	break;
      else
	{
	  visibleEntries++;
	  screen_select_file_display_entry(i,e);
	}
    }
  
  io_close_directory();
  subState=CHOOSE;
  return visibleEntries;
}

void select_next_page(void)
{
}

void select_prev_page(void)
{
  
}

void select_file_choose(char visibleEntries)
{
  char k=0;
  
  screen_select_file_choose(visibleEntries);

  while (subState==CHOOSE)
    {
      k=input();
      switch(k)
	{
	case 0x0d:
	  pos+=bar_get();
	  subState=DONE;
	  state=SELECT_SLOT;
	  break;
	case 0x1b:
	  subState=DONE;
	  state=HOSTS_AND_DEVICES;
	  break;
	case 0xA0:
	  bar_up();
	  break;
	case 0xA2:
	  bar_down();
	  break;
	}
    }
}

void select_file_done(void)
{
  state=SELECT_SLOT;
}

void select_file(void)
{
  char visibleEntries=0;

  subState=INIT;
  
  while (state==SELECT_FILE)
    {
      switch(subState)
	{
	case INIT:
	  select_file_init();
	  break;
	case DISPLAY:
	  visibleEntries=select_file_display();
	  break;
	case NEXT_PAGE:
	  select_next_page();
	  break;
	case PREV_PAGE:
	  select_prev_page();
	  break;
	case CHOOSE:
	  select_file_choose(visibleEntries);
	  break;
	case DONE:
	  select_file_done();
	  break;
	}
    }
}
