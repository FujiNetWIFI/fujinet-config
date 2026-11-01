## 2026-11-01

* [Adam] - A new disk named without an extension gets .ddp or .dsk from the media type picked, so the FujiNet can mount it and write its directory.

## 2026-10-31

* [C64] - Use all 8 of the IEC FujiNet's drive slots, so mounting works again.
* [All] - Update fujinet-lib to 5.1.0.
* [Odyssey 2] - Show the FujiNet logo.

## 2026-10-30

* [All] - Retry a failed slot read instead of showing, and later saving, stale slots.
* [Odyssey 2] - New: CONFIG for the Magnavox Odyssey 2.

## 2026-10-29

* [Adam] - Redraw the hosts screen after skipping WiFi setup.

## 2026-10-28

* [MS-DOS] - Fit 40-column mode, fix stray selection bar highlights, and take Y/N answers with a single key.

## 2026-10-24

* [CoCo] - Keep the activated joystick when switching between the MAIN, FILES and WIFI programs.

## 2026-10-23

* [C64] - Boot the mounted drive. Show names in the right case, and make the keys and screens match their hints.
* [Model II] - New: CONFIG for the TRS-80 Model II under CP/M.

## 2026-10-22

* [CoCo] - Show ^, _ and ` correctly on the CoCo 3.
* [Atari 2600] - Show a live progress bar and full names while booting, and NOT A ROM for a file that isn't one.

## 2026-10-20

* [Atari 2600] - Add color, a frame, a selection bar and sound cues.

## 2026-10-19

* [H89] - New: CONFIG for the Heathkit H89 under HDOS and CP/M.
* [Arcadia] - New: CONFIG for the Emerson Arcadia 2001.
* [Atari 5200] - New: CONFIG for the Atari 5200.
* [S-100] - New: CONFIG for the FujiNet S-100 card.
* [CoCo] - Remount the config disk only after slot 0 changes.
* [Adam, CoCo, Dragon] - Return to the folder after creating a new disk.
* [All] - Mount new disks read/write.
* [Adam] - Don't offer an EOS directory on a disk over 65535 blocks.
* [Adam] - Rescan before writing a new disk's EOS directory, and report a failure.
* [All] - Leave the drive slot alone when creating a new disk fails.

## 2026-10-17

* [All] - Return to the current folder after leaving the slot picker from New.
* [Apple II] - Keep the line editor inside its field's row.
* [All] - Clear create mode when the slot picker is cancelled.
* [CoCo] - Limit the new disk drive count to 255.
* [Adam, Apple II, C64, CoCo, Dragon] - Keep new disk name and size input inside their buffers.
* [Atari] - Brighten the selection bar so it shows on mono monitors.
* [CoCo] - Remount the config disk before switching programs.
* [Adam, Atari, C64, Dragon, MS-DOS] - Show the CONFIG commit on the info screen.
* [CoCo, Dragon] - Detect a ROM in slot 0 by its full filename.

## 2026-10-16

* [All] - Update fujinet-lib to 5.0.0.
* [MS-DOS] - Build with fujinet-lib.
* [CoCo] - Show the color keys on the screens that take them.
* [CoCo] - Let BREAK back out of text fields, and ignore non-printing keys.
* [Atari] - Keep long filenames and paths inside their rows.
* [Atari] - Show long filenames after paging and HOME/END.

## 2026-10-12

* [Studio II] - New: CONFIG for the RCA Studio II.

## 2026-10-10

* [Atari] - Launch the Atari BBS Gateway client with G.
* [Atari 7800] - New: CONFIG for the Atari 7800.
* [CoCo] - Scroll long names only for files, not folders.

## 2026-10-08

* [Sega Master System] - New: CONFIG for the Sega Master System.

## 2026-10-05

* [NES] - New: CONFIG for the NES.
* [CoCo] - Reset the stack at the start of each program.

## 2026-10-03

* [Atari 2600] - New: CONFIG for the Atari 2600.

## 2026-10-02

* [CoCo] - New 42x24 hi-res text screens, with CONFIG split into WIFI, MAIN and FILES programs.
* [CoCo] - New CoCo 3 version: 40x24 GIME graphics, color sets, RGB or composite.
* [CoCo] - Change colors with SHIFT-LEFT/RIGHT.
* [CoCo] - Fix the app key read buffer size, and show the W drive marker in bold.

## 2026-09-28

* [Adam] - Boot the mounted disks on F6 instead of remounting the highlighted file.

## 2026-09-24

* [Apple II] - Detect whether a joystick is plugged in.

## 2026-09-18

* [Intellivision] - Support the ECS keyboard.

## 2026-09-16

* [ColecoVision] - New: CONFIG for the ColecoVision.

## 2026-09-07

* [Dragon] - New splash screen.
* [CoCo, Dragon] - Show a caps lock indicator.

## 2026-09-03

* [Astrocade] - New: CONFIG for the Bally Astrocade.

## 2026-09-01

* [All] - Refresh the drive slots much faster, by reading them once instead of once per drive.

## 2026-08-31

* [Intellivision] - Add color.

## 2026-08-29

* [MSX] - Show 4 drive slots plus the Disk ROM by default, and adjust when a ROM is mounted.

## 2026-08-26

* [Apple II] - Restart directly from the FujiNet slot.

## 2026-08-25

* [Intellivision] - New: CONFIG for the Intellivision.

## 2026-08-23

* [Adam] - Fix the filename display on the device slot assignment screen.

## 2026-07-22

* [Apple II] - Add joystick support.
* [CoCo] - Add joystick support.

## 2026-07-21

* [Dragon] - Add a splash screen.

## 2026-07-12

* [CoCo] - Fix mounting and booting the lobby when a ROM image is mounted.

## 2026-06-25

* [MSX] - Disable the CONFIG ROM before rebooting to BASIC.

## 2026-06-22

* [Atari] - Fix copying a file.
* [Atari] - Fix the WiFi connected state after connecting.
* [MSX] - Fix the file filter.

## 2026-06-21

* [MSX] - Use "prev"/"next" in the file picker instead of "...".
* [MSX] - Fix WiFi setup.

## 2026-06-20

* [MSX] - Fix the command that enables the ROM.
* [MSX] - Show 8 device slots.
* [MSX] - Fix the file picker keys: B boots, left and right arrows page.

## 2026-06-19

* [Atari] - Put a .CAS image in device slot 8, read-only, so it mounts as the cassette.
* [Atari] - Add an option to boot with high speed SIO for disks, FujiNet and network.

## 2026-06-11

* [Adam] - Fix the host slot used when mounting.

## 2026-06-10

* [All] - Fix copying a file found with a search.

## 2026-06-08

* [CoCo] - Update the command that starts a user ROM on the FujiNet cartridge.
* [All] - Fix mounting a different file into a drive slot that was already mounted.

## 2026-06-07

* [CoCo] - Boot a ROM image mounted in drive 0.

## 2026-06-03

* [MSX] - Fix navigation.

## 2026-05-28

* [All] - Keep a read/write mode change after reboot, and come back to the bottom of the list when scrolling back in the file picker.

## 2026-05-15

* [MS-DOS] - Offer to install the CONFIG TSR on exit.

## 2026-05-08

* [CoCo] - Add Skip to the WiFi selection screen.

## 2026-04-15

* [MS-DOS] - Fix the display after ejecting all disks.
* [Apple II] - Read the slots when toggling the drive view.

## 2026-04-14

* [MS-DOS] - Improve launching the lobby.

## 2026-04-10

* [MS-DOS] - Exit properly, and mount and boot the lobby.

## 2026-04-08

* [Apple II] - Support 8 SmartPort drives, and toggle between the SmartPort and Disk II views.

## 2026-03-31

* [Atari] - Make the CONFIG disk fast-loading.

## 2026-03-25

* [MS-DOS] - Put the FujiNet MS-DOS drivers on the CONFIG disk.

## 2026-03-19

* [MS-DOS] - Add a configurable box border attribute.

## 2026-03-17

* [CoCo] - BREAK cancels connecting to WiFi.

## 2026-03-16

* [MSX] - New: CONFIG for MSX.
* [MS-DOS] - New: CONFIG for MS-DOS.

## 2026-03-05

* [Apple II] - Fix a UI bug when ejecting a mount.

## 2026-03-04

* [Apple II] - Fix changing read/write mounts.

## 2026-02-28

* [Apple II] - Fix WiFi connection handling.

## 2026-01-20

* [All] - Update fujinet-lib to 4.9.0, which adds file copy.

## 2025-11-21

* [Apple II] - Don't use the hi-res logo on the AppleSqueezer.

## 2025-11-19

* [CoCo] - Move the splash screen buffer to 0x6600 to stop screen corruption.

## 2025-11-17

* [CoCo] - Fix starting CONFIG from 40- or 80-column mode on the CoCo 3.

## 2025-11-15

* [CoCo] - Stop drawing screens twice.

## 2025-11-14

* [CoCo] - Add the FujiNet logo splash screen.

## 2025-11-05

* [CoCo] - Scroll long filenames.

## 2025-10-30

* [Apple II] - Fix the first two characters missing from the directory listing.

## 2025-10-28

* [CoCo] - Boot the lobby from CONFIG.

## 2025-10-27

* [Apple II] - Fix the left-side artifact in the logo on the Apple III.

## 2025-09-27

* [All] - Handle an invalid network count from the WiFi scan.

## 2025-09-21

* [Dragon] - New: CONFIG for the Dragon.

## 2025-08-23

* [Atari] - Fix filename, date and size in the file list.

## 2025-07-31

* [CoCo] - Show a read/write change on the disk slot screen at once, clear the indicators when ejecting, and show mounted names in uppercase on exit.

## 2025-07-28

* [CoCo] - Number the disk slots 0-3 on the slot screen, and let 1-8 pick a host slot.

## 2025-07-17

* [CoCo] - Don't wait for a key while connecting to WiFi.

## 2025-07-15

* [CoCo] - Accept C and E in either case on the hosts screen.

## 2025-05-21

* [All] - ESC cancels connecting to WiFi.
* [Adam] - Faster screen drawing.

## 2025-04-26

* [Adam] - Faster, cleaner screens, and show the device slot modes.

## 2025-04-19

* [Apple II] - Update fujinet-lib to 4.7.5, which fixes machines with more than one SmartPort card.

## 2025-04-04

* [Apple II] - Fix "Selsect" typo.

## 2025-03-29

* [Apple II] - Fix disk corruption when ejecting a Disk II slot.

## 2025-03-22

* [Apple II] - Create DOS 3.3 disks.

## 2025-03-18

* [Apple II] - Use the alternate character set on every IIe, not only the enhanced one.

## 2025-02-21

* [All] - Treat a directory entry as a folder only when it ends with /, and skip empty entries.

## 2025-02-10

* [Atari] - Add TK-II key bindings.

## 2025-01-10

* [Apple II] - Fix disk drive detection on the Apple III, and stop SoftSP triggering its internal drive at boot.

## 2025-01-01

* [All Systems] - If a host entry is changed, eject any disks that were mounted from the "old" host since they aren't valid anymore.

## 2024-12-28

 * [Atari] - Remove unused variables.

## 2024-12-25

 * [Atari] - Upgrade fujinet-lib to 4.7.4
 * [Atari] - Changes to wifi network selection screen to accomodate passwords up to 64 characters in a more consistent visual style. The main password line was set as 40x8x1, but when it wrapped the next line was set to 20x8x2. Shifted down the two larger 20 column lines to leave two 40 column lines for the password.
 

## 2024-04-16

* [All] - Preserve file filter after mounting an image to a slot and returning to the file browser.
* [Atari] - Upgrade fujinet-lib to 3.0.3

## 2023-07-15

* [Atari] - Added new FujiNet command 0xD6 - `io_set_boot_mode(unsigned char mode)`, needed to boot directly into the Lobby client.
* [Atari] - Added 'L' command to boot into the FujiNet Lobby client. Available from the main host/device screen.
  * User will need to respond with 'Y' to boot the lobby. Any other key will be consider a 'N'o.
* [Atari] - Remove use of 'sprintf' to reduce the memory footprint of the program
* [Atari] - Changed display header from "TNFS HOST LIST" to just "HOST LIST" since there can be host types other than TNFS (SD for example)
* [Atari] - Removed verbose output from the "mount and boot" screen.

## 2022-11-25

* [Atari] - Added folder icon to directory entries when browsing a host.
* [Atari] - Fixed "config" screen corruption (rolling, starting in middle of screen)

