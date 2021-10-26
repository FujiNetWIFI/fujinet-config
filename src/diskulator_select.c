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

void diskulator_select_select_file(Context *context, SubState *ss)
{
  smartkeys_display(NULL,NULL," UP  DIR","  NEW\n DISK"," FILTER","  BOOT");
  
  while(1)
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
