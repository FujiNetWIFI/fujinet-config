#ifndef COMPAT_H
#define COMPAT_H

#ifndef _CMOC_VERSION_
#include <string.h>
#endif /* _CMOC_VERSION_ */

#ifdef BUILD_MSDOS
/* Watcom limits identifiers to 31 chars; provide short aliases */
#define screen_set_wifi_select_network screen_set_wifi_sel_net
void screen_set_wifi_sel_net(unsigned char nn);
#endif /* BUILD_MSDOS */

#endif /* COMPAT_H */
