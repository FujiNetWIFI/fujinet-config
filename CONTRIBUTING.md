# Contributing to fujinet-config

CONFIG is the program a FujiNet boots into: it sets up WiFi, browses hosts, and mounts disk images
into the device slots. One set of shared sources in `src/` is built for every computer in the
`PLATFORMS` list of the top-level `Makefile` (Adam, Apple II, Atari, C64, CoCo, Dragon, MS-DOS,
TRS-80 Model II). The game consoles each have a standalone CONFIG in a directory of its own.

This is the working reference for anyone changing the code: new contributors, regulars, and AI
coding assistants alike.

## Before you start

- Check `git log --oneline -20 -- <paths>` and the open pull requests before touching an area. If
  recent work or an open PR overlaps, raise it before proceeding.
- For anything beyond a contained fix (a new screen, a change to the shared flow, a pattern you
  intend to repeat across platforms) describe the approach and get agreement first.
- If a maintainer asks you to hold off because another change is coming, stop.
- Ask on Discord or in the issue if any of this is unclear.

## Build

The cross-compilers are cc65 (Apple II, Atari, C64), CMOC (CoCo, Dragon), z88dk (Adam, Model II)
and Open Watcom (MS-DOS). The `fozztexx/defoogi` container has all of them and the disk tools; CI
uses it.

```sh
make                # every platform in PLATFORMS
make <platform>     # one, e.g. make coco; the disk image lands in r2r/<platform>/
```

`make clean` before a build you mean to trust. `FUJINET_LIB` in the `Makefile` pins the
fujinet-lib release. To try a lib change, override it on the command line
(`make FUJINET_LIB=<lib checkout> coco`) rather than editing it; the pin changes only to move to a
new release.

A build must not leave files in the source tree. Intermediate output goes under `build/`, `r2r/` or
`_cache/`. Check `git status` before committing; never commit stray files or add them to
`.gitignore` to hide them.

## Layout

- `src/`: the flow every platform shares (hosts and devices, file browser, slot picker, WiFi). It
  draws and reads keys through the `screen_*` and `input_*` functions in `screen.h` and `input.h`.
- `src/<platform>/`: each platform's screens, input, mount-and-boot and system calls.
  `PLATFORM_COMBOS` in the `Makefile` adds shared directories to a platform's build.
- `src/coco/`: three programs (WIFI, MAIN, FILES), each with a CoCo 3 build, handing off through
  `struct entry_data`. Dragon keeps its own code in `src/dragon/`.
- `dist.<platform>/`: files the disk-image rules copy onto a platform's disk.
- `makefiles/`: the MekkoGX build framework, shared with other FujiNet projects. Don't customize it;
  project rules go in the top-level `Makefile`.
- Console directories (`arcadia`, `astrocade`, `atari-2600`, `atari5200`, `atari7800`, `coleco`,
  `intv`, `nes`, `o2`, `sms`, `studio2`): standalone CONFIGs, each with its own `Makefile` and
  `README.md`. The top-level build and CI don't build them.

## Design principles

Reviewers hold changes to this. A change that works but cuts against it will be sent back, so raise
the design first when yours doesn't fit.

- The flow is written once, in `src/`. Where a platform differs, the difference goes in that
  platform's `screen_*`/`input_*` functions or its other files in `src/<platform>/`.
- Shared files still carry `BUILD_*` conditionals and direct conio calls from before this rule.
  Don't add more; when you change one, move the difference into the platform's files if you can.
- Every FujiNet call goes through fujinet-lib. If the lib lacks a call, add it there instead of
  writing a local copy.
- Adding a platform means `src/<platform>/`, a `PLATFORMS` line, a CI matrix line, and a line in
  the firmware-PR copy list if the firmware ships it, not edits scattered through `src/*.c`.

## Size

CONFIG has to fit on the smallest machines it runs on.

- Apple II CONFIG has had only tens of bytes to spare. The CoCo programs must stay under their
  `--limit`, and the Model II `.COM` inside the CP/M TPA.
- Keep arithmetic as narrow as the values allow. A 32-bit shift, multiply or divide pulls in a
  compiler runtime routine.
- Don't add a static buffer when an existing one will do.
- When a change grows shared code, build every platform and look at the map file when one is close.
  Two PRs that each fit can fail together.

## Validate a change before proposing it

CI builds each platform in its matrix and runs nothing. A push to `main` opens or updates a
fujinet-firmware PR with the new disk images, so `main` must work on every platform it ships. So:

1. `make clean`, then `make` for every platform, not just the one you changed, when you touched
   `src/*.c` or a shared header.
2. Run the screens you changed on an emulator or hardware connected to a FujiNet (FujiNet-LWM
   works).
3. `git diff` and re-read every comment in it: each must still be true of the code as changed.
4. `git diff --stat` and `git status`: every file listed is one you meant to change, and the build
   left nothing behind.

## Code style

- Match the surrounding file. Don't reformat lines your change doesn't otherwise touch, and don't
  reorder includes.
- Give the values of one parameter (modes, states, key codes) an `enum`, not a list of `#define`s,
  and keep byte-wide fields `uint8_t`. A lone constant can stay a `#define`.
- Name a literal whose meaning isn't obvious (a screen row, a key code, a buffer size).
- Use fixed-width types for anything on the wire.
- Keep platform lists (`PLATFORMS` in the `Makefile`, the CI matrix) in alphabetical order,
  commented-out entries included, so a duplicate stands out.

## Comments

Keep comments short: one or two lines above the code, or a brief trailing comment.

- Comment the *why* of a non-obvious choice (a ROM quirk, a memory limit, a firmware behavior).
  Don't restate the code.
- Don't narrate your edit: no "added to fix X", no dated notes, no before/after. That goes in the
  commit message and the PR.
- Don't leave commented-out code.

Before every commit, re-read each comment in `git diff`, including ones you didn't write whose code
you changed, and fix or delete any the change made untrue.

## Changelog

Add a line to `CHANGELOG.md` for a real bug fix, a new feature, or a fujinet-lib update, under a
`## YYYY-MM-DD` heading for the day, newest first, tagged by platform when it is one:
`* [Atari] - ...`. CONFIG has no release versions. Tests, CI, build plumbing, refactors and docs
get no entry.

## Scope of a change

One concern per pull request.

- Don't combine a refactor with a behavior change, a fix with a feature, or a move with either.
  Land the move first, then the change that needed it.
- Keep formatting and comment edits out of a functional diff.
- Every PR targets `main` and stands on its own; don't chain one onto another open branch.
- When you find an unrelated problem mid-change, leave it and open an issue for it.

## Commits and pull requests

- Work on a topic branch, never directly on `main`.
- The project squash-merges, and GitHub adds the `(#NNN)` suffix; don't type it yourself. Use
  imperative mood ("Add foo", not "Added foo").
- An optional `[tag]` prefix names the platform or area: `[adam]`, `[coco, dragon]`, `[ci]`. Leave
  it off a change to the shared flow.
- Name no AI tool or its vendor in anything that lands in the repo or on GitHub: code, comments,
  docs, file and directory names, commit messages and PR descriptions. That includes the
  `Co-Authored-By:` trailer and "Generated with" footer some assistants add by default.
- Never commit an assistant's own instruction, memory or scratch files. `AGENTS.md` is the one such
  file the project keeps.

When the change fixes a tracked issue, put `Fixes #NNN` on its own line in the **pull request
description**, one keyword per issue. Use `Refs #NNN` for partial work.

## Write a short pull request description

Say why the change exists, what to look at first, and what you did not verify. Aim for one screen:
a sentence or two, a few bullets, then testing.

- One bullet per real change, not per file.
- Don't paste build output; quote the one line that matters, if any does.
- Don't narrate the work or restate the title.

This applies with particular force to descriptions written by an AI assistant, which by default run
several times longer than a reviewer needs. Write the short version.

## Say what you did not test

Say which platforms you built and which you ran, emulator or hardware (and which machine), and the
FujiNet firmware version. "Builds everywhere, tested in ADAMEm and on a CoCo 3, not on an Apple II"
is a good answer.

## Common mistakes

- **A new `BUILD_*` conditional in `src/*.c`** instead of the difference going in the platform's
  files.
- **A local copy of a FujiNet call** that belongs in fujinet-lib.
- **Building one platform** after a change to shared code.
- **Growing shared code** without checking that Apple II and the CoCo programs still fit.
- **Assuming CI tests CONFIG.** It only builds it, and what merges ships in the firmware PR.
- **Committing a changed `FUJINET_LIB`** that was only for local testing.
- **A build that leaves files** in the source tree, or committing them.
- **A Changelog line** for a test, a build change or a refactor.
- **Leaving a comment that the change made untrue.**
- **A PR description that restates the diff.**

## Where to look

- The top-level `Makefile`: the platform list, the lib pin, and each platform's build quirks.
- `makefiles/README.md`: how the MekkoGX build works.
- `src/screen.h` and `src/input.h`: what a platform implements.
- `notes/all-about-adamnet.md`: the Adam's bus.
- `git log --oneline -20 -- <path>` is the quickest way to learn a directory's local convention.
