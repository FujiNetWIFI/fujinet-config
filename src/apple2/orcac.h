#ifdef BUILD_APPLE2
/**
 * FujiNet Config for Apple2
 *
 * ORCA/C compiler specifics
 */

#ifndef ORCAC_H
#define ORCAC_H

/* Enforce strict type compatibility checks */
/* Allow // comments */
#pragma ignore 0x0008

/* Perform all lint checks but treat them as warnings */
#pragma lint -1;0

#ifdef BUILD_A2CDA
#pragma cda "FujiNet Config" Start ShutDown
#endif /* BUILD_A2CDA */

#endif /* ORCAC_H */
#endif /* BUILD_APPLE2 */
