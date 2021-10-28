/**
 * #FUJINET CONFIG
 * Diskulator Select Disk image screen
 */

#include <msx.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <smartkeys.h>
#include <eos.h>
#include "diskulator_select.h"
#include "fuji_adam.h"
#include "bar.h"
#include "input.h"

typedef enum _substate
  {
   SELECT_FILE,
   PREV_PAGE,
   NEXT_PAGE,
   ADVANCE_DIR,
   DEVANCE_DIR,
   DONE
  } SubState;

#define DIRECTORY_LIST_ENTRIES_PER_PAGE 15
#define DIRECTORY_LIST_SCREEN_WIDTH 31

void diskulator_select_open_directory(Context *context)
{
  DirectoryPosition pos=context->dir_page*DIRECTORY_LIST_ENTRIES_PER_PAGE;

  if (context->filter[0]!=0x00)
    {
      memset(context->directory_plus_filter,0,sizeof(context->directory_plus_filter));
      strcpy(context->directory_plus_filter,context->directory);
      strcpy(&context->directory_plus_filter[strlen(context->directory_plus_filter)+1],context->filter);
      fuji_adamnet_open_directory(context->host_slot,context->directory_plus_filter);
    }
  else
    fuji_adamnet_open_directory(context->host_slot,context->directory_plus_filter);

  // Position directory cursor
  fuji_adamnet_set_directory_position(pos);
}

void diskulator_select_display_directory_page(Context *context)
{
  char current_entry[DIRECTORY_LIST_SCREEN_WIDTH];

  msx_color(1,15,7);
  
  // Clear content area
  msx_vfill(0x0000,0x00,5120);

  // Open Directory
  diskulator_select_open_directory(context);

  // Iterate over directory display
  for (char i=0;i<DIRECTORY_LIST_ENTRIES_PER_PAGE;i++)
    {
      gotoxy(1,i+1);
      fuji_adamnet_read_directory(current_entry,DIRECTORY_LIST_SCREEN_WIDTH,0);
      if (current_entry[0]==0x7F)
	{
	  if (i==0)
	    cprintf("--EMPTY--");
	  break;
	}
      else
	{
	  cprintf("%31s",current_entry);
	}
    }
 
  // Close directory
  fuji_adamnet_close_directory(context->host_slot);
}

void diskulator_select_select_file(Context *context, SubState *ss)
{
  smartkeys_display(NULL,NULL," UP  DIR","  NEW\n DISK"," FILTER","  BOOT");
  diskulator_select_display_directory_page(context);
  while (1)
    {
    }
}

void diskulator_select_devance_directory(Context *context)
{
}

void diskulator_select_display_directory_path(Context *context)
{
}

void diskulator_select_setup(Context *context)
{
  memset(context->filename,0,sizeof(context->filename));
  
  if (context->directory[0]==0x00)
    strcpy(context->directory,"/");
  
  context->newDisk = false;

  smartkeys_set_mode();
  clrscr();

  msx_vfill(MODE2_ATTR,0xF4,256);
  msx_vfill(MODE2_ATTR+256,0x1F,4096);
  msx_vfill_v(MODE2_ATTR,0xF4,136);
  
}

/**
 * Diskulator select disk image
 */
State diskulator_select(Context *context)
{
  SubState ss=SELECT_FILE;

  diskulator_select_setup(context);

  while (ss != DONE)
    {
      switch(ss)
        {
        case SELECT_FILE:
          diskulator_select_select_file(context,&ss);
          break;
        case PREV_PAGE:
          context->dir_page--;
          ss=SELECT_FILE;
          break;
        case NEXT_PAGE:
          context->dir_page++;
          ss=SELECT_FILE;
          break;
        case ADVANCE_DIR:
          context->dir_page=0;
          context->dir_eof=false;
          ss=SELECT_FILE;
          break;
        case DEVANCE_DIR:
          diskulator_select_devance_directory(context);
          diskulator_select_display_directory_path(context);
          context->dir_page=0;
          context->dir_eof=false;
          ss=SELECT_FILE;
          break;
        }
    }

  return context->state;
}
