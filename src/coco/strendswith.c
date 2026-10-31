#ifdef _CMOC_VERSION_

#include <cmoc.h>

/*
 * strendswith --
 *
 * PUBLIC: #ifndef HAVE_STRENDSWITH
 * PUBLIC: int strendswith __P((const char *, const char *));
 * PUBLIC: #endif
 */

int strendswith(const char *str, const char *suffix)
{
  size_t str_len = strlen(str);
  size_t suffix_len = strlen(suffix);

  return (str_len >= suffix_len) &&
         (!memcmp(str + str_len - suffix_len, suffix, suffix_len));
}

int filename_has_rom_suffix(const char *name)
{
  size_t len = strlen(name);
  char ext[5];

  if (len < 4)
    return 0;

  strcpy(ext, name + len - 4);
  strupr(ext);
  return !strcmp(ext, ".CCC") || !strcmp(ext, ".ROM");
}
#endif /* _CMOC_VERSION_ */
