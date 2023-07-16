# Change Log

## 2023-07-15

* [Atari] - Added new FujiNet command 0xD6 - `io_set_boot_mode(unsigned char mode)`, needed to boot directly into the Lobby client.
* [Atari] - Added 'L' command to boot into the FujiNet Lobby client. Available from the main host/device screen.
* [Atari] - Remove use of 'sprintf' to reduce the memory footprint of the program
* [Atari] - Changed display header from "TNFS HOST LIST" to just "HOST LIST" since there can be host types other than TNFS (SD for example)

## 2022-11-25

* [Atari] - Added folder icon to directory entries when browsing a host.
* [Atari] - Fixed "config" screen corruption (rolling, starting in middle of screen)

