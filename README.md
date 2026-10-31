# fujinet-config

CONFIG is the program a FujiNet boots into when no disk is mounted. It sets up WiFi, browses
hosts, mounts disk images into the device slots, copies files between hosts, and shows the
FujiNet's network settings.

## Building

The cross-compilers and disk tools are in the `fozztexx/defoogi` container.

```sh
make                # every platform in PLATFORMS
make <platform>     # one, e.g. make coco
```

Each build leaves the program and its disk image in `r2r/<platform>/`. `FUJINET_LIB` in the
`Makefile` pins the fujinet-lib release; platforms left out of `PLATFORMS` say why there.

A push to `main` opens or updates a fujinet-firmware PR with the new images:

| Platform | Image | In fujinet-firmware `data/webui/device_specific/` |
|---|---|---|
| Adam | `config.ddp` | `BUILD_ADAM/autorun.ddp` |
| Apple II | `config.po` | `BUILD_APPLE/autorun.po` |
| Atari | `config.atr` | `BUILD_ATARI/autorun.atr` |
| C64 | `config.prg` | `BUILD_IEC/config.prg` |
| CoCo | `config.dsk` | `BUILD_COCO/autorun.dsk` |
| Dragon | `config.vdk`, `AUTOLOAD.DWL`, `LOGO.DWL`, `STAGE2.DWL`, `CONFIG.DWL` | `BUILD_COCO/autorund.vdk` and the `.DWL` files |
| MS-DOS | `config.img` | `BUILD_RS232/autorun.img` |
| TRS-80 Model II | `config.imd` | `BUILD_RS232/autorun.imd` |

The Model II CONFIG runs under Pickles & Trout CP/M 2.2 and reaches the FujiNet over serial port
A; mounted images appear as drives through the
[FujiNet CP/M driver](https://github.com/FozzTexx/fujinet-trs80-model2-driver). The H89 has its
own notes in [`dist.h89/`](dist.h89/).

## Game consoles

Each console has a standalone CONFIG with its own `Makefile` and `README.md`:
[Arcadia 2001](arcadia/), [Astrocade](astrocade/), [Atari 2600](atari-2600/),
[Atari 5200](atari5200/), [Atari 7800](atari7800/), [ColecoVision](coleco/),
[Intellivision](intv/), [NES](nes/), [Odyssey 2](o2/), [Sega Master System](sms/),
[Studio II](studio2/).

## Contributing

See [`CONTRIBUTING.md`](CONTRIBUTING.md).
