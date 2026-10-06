# FujiNet CONFIG for the Sega Master System

CONFIG for the FujiNet Master System cartridge (`fujinet-firmware/pico/sms`): a 32K z88dk `+sms`
client the cartridge serves at power-on. Ported from `../coleco/`: same state machine, same
"draw straight out of the cartridge's reply window" discipline, with a Mode 4 platform layer.

Built with z88dk's `+sms` target and **fujinet-lib-experimental's `sms` target**, which carries the
mailbox transport and the claim stamp (`makefiles/sms-romstamp.py`, which also re-stamps the SEGA
header's checksum).

## Layout

| file | purpose |
|---|---|
| `config.c` | entry point, state dispatcher, drawing helpers, the Pause menu |
| `st_wifi.c` | check / scan / pick / password / custom SSID |
| `st_hosts.c` | host slots: open, rename, game lobby, adapter info |
| `st_files.c` | file browser: paging, descend, climb, filter |
| `st_copy.c` | host-to-host copy |
| `st_info.c` | adapter info (GET_ADAPTERCONFIG_EXTENDED) |
| `st_lobby.c` | the FujiNet Game Lobby |
| `st_boot.c` | mount into device slot 0, progress, jump to the cart's loader page |
| `fujiraw.c` | the transactions that stream their payload byte by byte |
| `fujidisp.c` `fujiin.c` `fujisnd.c` `fujiedit.c` | Mode 4 text with a RAM shadow for the selection bar; the pad and Pause; the PSG click; the on-screen keyboard |
| `fujisplash.c` `fujisplash_data.c` | the splash: the FujiNet wordmark from `../coleco`, coloured for Mode 4 |
| `constants.h` `state.h` | states, geometry, shared globals |

## Controls

| screen | d-pad | 1 | 2 | Pause |
|---|---|---|---|---|
| HOSTS | move | open host | WiFi setup | menu: info, lobby, rename, WiFi |
| FILES | up/down move, left/right page | open dir / boot file | up a directory (at `/`: hosts) | menu: filter, copy, hosts |
| WIFI | move | pick network | back to hosts | rescan |
| INFO | | back | back | |
| keyboard | grid | pick | delete | OK (cancel is the ESC cell) |

In the Pause menu, left/right choose, 1 runs it, 2 or Pause cancels.

## Build

    make            # build/config.sms
    make install    # into ~/Workspace/fujinet-firmware/pico/sms/build, where build-cart.sh bakes it
    make cart       # install, then build the cartridge firmware

`Z88DK`, `FNLIB` and `PICO_SMS` override the default paths.
