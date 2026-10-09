/* fujisnd.h -- the SN76489 on port $7F: silence, and a short click. */

#ifndef FUJISND_H
#define FUJISND_H

void snd_init(void);
void snd_click(void);
/* Once a frame (fujiin calls it as it polls the frame flag). */
void snd_tick(void);

#endif /* FUJISND_H */
