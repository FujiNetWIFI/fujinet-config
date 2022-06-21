#ifdef BUILD_ATARI
/**
 * FujiNet Configurator
 *
 * Function to die, wait for keypress and then do coldstart
 */

#ifndef DIE_H
#define DIE_H

/**
 * Stop, wait for keypress, then coldstart
 */
void die(void);

/**
 * Wait a moment.
 */
void wait_a_moment(void);

#endif /* DIE_H */
#endif