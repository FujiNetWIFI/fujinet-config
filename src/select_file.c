/**
 * FujiNet for #Adam configuration program
 *
 * Select file from Host Slot
 */

#include <conio.h>
#include <string.h>
#include "select_file.h"

#ifdef BUILD_ADAM
#include "adam/fuji_typedefs.h"
#include "adam/screen.h"
#include "adam/io.h"
#include "adam/globals.h"
#include "adam/input.h"
#include "adam/bar.h"
#endif /* BUILD_ADAM */

#ifdef BUILD_APPLE2
#include "apple2/fuji_typedefs.h"
#include "apple2/screen.h"
#include "apple2/io.h"
#include "apple2/globals.h"
#include "apple2/input.h"
#include "apple2/bar.h"
#endif /* BUILD_APPLE2 */

#ifdef BUILD_C64
#include "c64/fuji_typedefs.h"
#include "c64/screen.h"
#include "c64/io.h"
#include "c64/globals.h"
#include "c64/input.h"
#include "c64/bar.h"
#endif /* BUILD_C64 */

#define ENTRIES_PER_PAGE 15

char path[224];
char filter[32];
DirectoryPosition pos=0;
bool dir_eof=false;
bool quick_boot=false;
unsigned long selected_size=0;

static enum
  {
   SF_INIT,
   SF_DISPLAY,
   SF_NEXT_PAGE,
   SF_PREV_PAGE,
   SF_CHOOSE,
   SF_FILTER,
   SF_ADVANCE_FOLDER,
   SF_DEVANCE_FOLDER,
   SF_NEW,
   SF_DONE
  } subState;

void select_file_init(void)
{
  pos=0;
  memset(path,0,256);
  path[0]='/';
  memset(filter,0,32);
  screen_select_file();
  subState=SF_DISPLAY;
  quick_boot=dir_eof=false;
}

unsigned char select_file_display(void)
{
  char visibleEntries=0;
  char i;
  char *e;
  
  io_mount_host_slot(selected_host_slot);

  if (io_error())
    {
      screen_error("  COULD NOT MOUNT HOST SLOT.");
      subState=SF_DONE;
      state=SET_WIFI;
      return 0;
    }

  screen_select_file_display(path,filter);
  
  io_open_directory(selected_host_slot,path,filter);
  
  if (io_error())
    {
      screen_error("  COULD NOT OPEN DIRECTORY.");
      subState=SF_DONE;
      state=SET_WIFI;
      return 0;
    }
  
  if (pos>0)
    io_set_directory_position(pos);
  
  for (i=0;i<ENTRIES_PER_PAGE;i++)
    {
      e = io_read_directory(31,0);
      if (e[2]==0x7F)
	{
	  dir_eof=true;
	  break;
	}
      else
	{
	  visibleEntries++;
	  screen_select_file_display_entry(i,e);
	}
    }

  // Do one more read to check EOF
  e = io_read_directory(31,0);
  if (e[2]==0x7F)
    dir_eof=true;
  
  io_close_directory();

  if (pos > 0)
    screen_select_file_prev();
  
  if (dir_eof != true)
    screen_select_file_next();
  
  subState=SF_CHOOSE;
  return visibleEntries;
}

void select_next_page(void)
{
  bar_clear(false);
  pos += ENTRIES_PER_PAGE;
  subState=SF_DISPLAY;
  dir_eof=false;
}

void select_prev_page(void)
{
  bar_clear(false);
  pos -= ENTRIES_PER_PAGE;
  subState=SF_DISPLAY;
  dir_eof=false;
}

void select_file_filter(void)
{
  memset(filter,0,32);
  screen_select_file_filter();
  input_line_filter(filter);
  dir_eof=quick_boot=false;
  pos=0;
  subState=SF_DISPLAY;
}

void select_file_choose(char visibleEntries)
{
  char k=0;
  
  screen_select_file_choose(visibleEntries);

  while (subState==SF_CHOOSE)
    {
      k=input();
      switch(k)
	{
	case 0x0d:
	  pos+=bar_get();
	  subState=SF_DONE;
	  break;
	case 0x1b:
	  subState=SF_DONE;
	  state=HOSTS_AND_DEVICES;
	  break;
	case 0x80:
	  pos=0;
	  dir_eof=quick_boot=false;
	  subState=SF_DISPLAY;
	  break;
	case 0x84:
	  subState=strcmp(path,"/") == 0 ? SF_CHOOSE : SF_DEVANCE_FOLDER;
	  break;
	case 0x85:
	  subState=SF_FILTER;
	  break;
	case 0x86:
	  quick_boot=true;
	  pos+=bar_get();
	  subState=SF_DONE;
	  state=SELECT_SLOT;
	  break;
	case 0x94:
	  subState=SF_NEW;
	  break;
	case 0xA0:
	  if ((bar_get() == 0) && (pos > 0))
	    subState=SF_PREV_PAGE;
	  else
	    bar_up();
	  break;
	case 0xA2:
	  if ((bar_get() == 14) && (dir_eof==false))
	    subState=SF_NEXT_PAGE;
	  else
	    bar_down();
	  break;
	case 0xA4:
	  if (pos>0)
	    subState=SF_PREV_PAGE;
	  break;
	case 0xA6:
	  if (dir_eof==false)
	    subState=SF_NEXT_PAGE;
	  break;
	}
    }
}

void select_file_advance(void)
{
  char *e;

  bar_clear(false);
  
  io_open_directory(selected_host_slot,path,filter);

  io_set_directory_position(pos);
  
  e = io_read_directory(128,1);
  
  io_close_directory();

  strcat(path,e); // append directory entry to end of current path

  pos=0;
  dir_eof=quick_boot=false;
  
  subState=SF_DISPLAY; // and display the result.
}

void select_file_devance(void)
{
  char *p = strrchr(path,'/'); // find end of directory string (last /)

  bar_clear(false);

  while (*--p != '/'); // scoot backward until we reach next /

  p++;
  
  *p = 0; // truncate string.

  pos=0;
  dir_eof=quick_boot=false;
  
  subState=SF_DISPLAY; // And display the result.
}

bool select_file_is_folder(void)
{
  char *e;

  io_open_directory(selected_host_slot,path,filter);

  io_set_directory_position(pos);

  e = io_read_directory(128,0);

  io_close_directory();
 
  return strrchr(e,'/') != NULL; // Offset 10 = directory flag.
}

void select_file_new(void)
{
  char f[128];
  char k;

  memset(f,0,128);
  
  screen_select_file_new_type();
  k=input_select_file_new_type();
  if (k==0)
    {
      subState=SF_CHOOSE;
      return;
    }

  screen_select_file_new_size(k);
  selected_size=input_select_file_new_size(k);

  if (selected_size == 1) // User selected custom
    {
      screen_select_file_new_custom();
      selected_size=input_select_file_new_custom();
    }

  if (selected_size==0) // Aborted from size
    {
      subState=SF_CHOOSE;
      return;
    }

  screen_select_file_new_name();
  input_select_file_new_name(f);

  if (f[0]==0x00)
    {
      subState=SF_CHOOSE;
      return;
    }
  else
    {
      create=true;
      strcat(path,f);
      state=SELECT_SLOT;
    }
}

void select_file_done(void)
{
  if (select_file_is_folder())
      subState=SF_ADVANCE_FOLDER;
  else
    state=SELECT_SLOT;
}

void select_file(void)
{
  char visibleEntries=0;

  subState=SF_INIT;
  
  while (state==SELECT_FILE)
    {
      switch(subState)
	{
	case SF_INIT:
	  select_file_init();
	  break;
	case SF_DISPLAY:
	  visibleEntries=select_file_display();
	  break;
	case SF_NEXT_PAGE:
	  select_next_page();
	  break;
	case SF_PREV_PAGE:
	  select_prev_page();
	  break;
	case SF_CHOOSE:
	  select_file_choose(visibleEntries);
	  break;
	case SF_FILTER:
	  select_file_filter();
	  break;
	case SF_ADVANCE_FOLDER:
	  select_file_advance();
	  break;
	case SF_DEVANCE_FOLDER:
	  select_file_devance();
	  break;
	case SF_NEW:
	  select_file_new();
	  break;
	case SF_DONE:
	  select_file_done();
	  break;
	}
    }
}
