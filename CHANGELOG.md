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

