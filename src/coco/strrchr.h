/**
 * @brief Borrowed strrchr from bdb
 */

#ifndef STRRCHR_H
#define STRRCHR_H

/*
 * strrchr --
 *
 * PUBLIC: #ifndef HAVE_STRRCHR
 * PUBLIC: char *strrchr __P((const char *, int));
 * PUBLIC: #endif
 */
char *strrchr(const char *p, int ch);

#endif /* STRRCHR_H */
