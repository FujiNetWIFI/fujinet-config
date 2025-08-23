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

#endif /* STRRCHR_H */
