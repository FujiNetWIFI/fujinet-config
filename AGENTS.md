# AGENTS.md

The development standards for this repository are in [`CONTRIBUTING.md`](CONTRIBUTING.md).
**Read it before changing anything.** It is the single source of truth; this file only points at
it and repeats the few rules that cause the most damage when missed.

- Build with `make <platform>` or `make` for every platform; the cross-compilers are in the
  `fozztexx/defoogi` container. After touching `src/*.c` or a shared header, build every platform.
- No new `BUILD_*` conditional in `src/*.c`. A platform difference goes in that platform's
  `screen_*`/`input_*` functions or other files in `src/<platform>/`.
- Every FujiNet call goes through fujinet-lib. Test a lib change with `make FUJINET_LIB=<dir>`;
  don't commit a changed pin except to move to a new release.
- Apple II CONFIG has had tens of bytes to spare. Keep arithmetic narrow and check that every
  platform still links.
- CI only builds, and every merge to `main` ships in a fujinet-firmware PR. Run what you changed on
  an emulator or hardware with a FujiNet, and say in the PR what you ran and what you did not.
- A build must leave no files in the source tree; check `git status` before committing.
- Changelog lines are for real fixes, features and fujinet-lib updates only, under the date.
- Keep comments short, keep one concern per pull request, and re-read the comments in your diff
  before committing to confirm the change did not make them untrue.
- Keep the pull request description short: why the change exists, what to look at, and what you
  did not test.

@CONTRIBUTING.md
