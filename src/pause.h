#ifndef FC_PAUSE_H
#define FC_PAUSE_H

#include <fujinet-fuji.h>

#ifdef DRAGON
/* The Dragon's 60 Hz interrupt, which fuji_sleep_ms() counts, may be off */
void dragon_sleep_ms(unsigned ms);
#undef fuji_sleep_ms
#define fuji_sleep_ms(ms) dragon_sleep_ms(ms)
#endif

#endif /* FC_PAUSE_H */
