/**
 * Mount every device slot before booting
 */

#include "hosts_and_devices.h"
#include "screen.h"
#include "compat.h"
#include "constants.h"
#include "globals.h"
#if defined(BUILD_H89) || defined(BUILD_MODEL2)
#include "mount_and_boot.h"
#endif /* BUILD_H89 || BUILD_MODEL2 */

#ifdef _CMOC_VERSION_
/* A literal, not a static const array: CMOC also keeps an unused copy of an array's initializer */
#define MOUNTING_MSG "MOUNTING DISKS...\r\r"
#endif

void hosts_and_devices_done(void)
{
  unsigned char i;
#if defined(BUILD_H89) || defined(BUILD_MODEL2)
  /* Nothing can boot from a FujiNet drive on the H89 or Model II, so this
     mounts every slot (with progress) and done() then returns to the OS. */
  mount_and_boot();
  state = DONE;
  return;
#endif /* BUILD_H89 || BUILD_MODEL2 */
#ifdef BUILD_MSDOS
  /* If the user armed the TSR option, install CFGTSR.EXE BEFORE the
     mount-all loop below swaps the config disk image away from us. */
  {
    extern void install_tsr_now(void);
    install_tsr_now();
  }
#endif
#ifdef _CMOC_VERSION_
  /* putstr, not printf: keeps the printf library out of the CoCo FILES app */
  cls(1);
  putstr(MOUNTING_MSG, sizeof(MOUNTING_MSG) - 1);
#endif

#ifdef BUILD_APPLE2
  // Display Loading message since TNFS Disk II images can take some time
  char s;
  clrscr();
  cprintf("MOUNTING DISKS...\r\n\r\n");
#endif
#ifdef BUILD_PMD85
  char y = 1;
  char s = '0';
  clrscr();
  gotoxy(SCR_X0, SCR_Y0);
  textcolor(LIST_TITLE_COLOR); cprintf("MOUNTING DEVICES...");
#endif
  for (i = 0; i < FUJI_MAX_DISKS; i++)
  {
    if (deviceSlots[i].hostSlot != 0xFF)
    {
#ifdef _CMOC_VERSION_
      putchar('0' + i);
      putchar(':');
      putstr((char *)deviceSlots[i].file, strlen((char *)deviceSlots[i].file));
      putchar('\r');
#endif
#ifdef BUILD_APPLE2
      s = i + 1;
      if (i < MAX_SMARTPORT)
        cprintf("     SMARTPORT DRIVE %d\r\n", s);
      else
        cprintf("       DISK II DRIVE %d\r\n", s - MAX_SMARTPORT);
#endif
#ifdef BUILD_PMD85
      if (i == 2) s = 'T';
      else if (i == 3) s = 'M';
      else s++;
      y++;
      gotoxy(SCR_X0, SCR_Y0 + y);
      textcolor(LIST_VBAR_COLOR); cprintf("%c:", s);
      textcolor(TEXT_COLOR); cprintf("%s", deviceSlots[i].file);
#endif
#ifdef BUILD_APPLE2
      if(!(deviceSlots[i].mode & FUJI_DISK_MODE_MOUNTED)) // skip if disk was already mounted
#endif
      {
        fuji_mount_host_slot(deviceSlots[i].hostSlot);
        if (!fuji_mount_disk_image(i, deviceSlots[i].mode))
          deviceSlots[i].hostSlot = 0xFF; // didn't mount: empty from here on
      }
    }
  }

  state = DONE;
}
