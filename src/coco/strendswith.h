/**
 * @brief Found on the web, this function checks if a string ends with a given suffix.
 * @param str The string to check.
 * @param suffix The suffix to check for.
 * @return true if str ends with suffix, false otherwise.
 */

#ifndef STRENDSWITH_H
#define STRENDSWITH_H

/*
 * strendswith --
 *
 * PUBLIC: #ifndef HAVE_STRENDSWITH
 * PUBLIC: int strendswith __P((const char *, const char *));
 * PUBLIC: #endif
 */
int strendswith(const char *str, const char *suffix);

// Case-insensitive .CCC/.ROM suffix check, shared by system_slot0_is_rom()
// and the file browser's own ROM-cartridge detection so the suffix rules
// live in one place.
int filename_has_rom_suffix(const char *name);

#endif /* STRRCHR_H */
