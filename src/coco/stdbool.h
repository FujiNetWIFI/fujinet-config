/**
 * @brief stdbool definition needed by config
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see LICENSE for details
 */

#ifndef STDBOOL_H
#define STDBOOL_H

#define true 1
#define false 0

#ifndef bool
#define bool _Bool
typedef unsigned char _Bool;
#endif /* bool */

#endif /* STDBOOL_H */
