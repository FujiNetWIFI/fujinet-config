#ifdef BUILD_APPLE2
/**
 * FujiNet CONFIG for #Apple2
 *
 * SmartPort MLI Routines
 */

#ifndef SP_H
#define SP_H

#include <stdint.h>

#define SP_CMD_STATUS 0
#define SP_CMD_CONTROL 4

extern uint8_t sp_payload[1024];
extern uint16_t sp_count;

uint8_t sp_status(uint8_t dest, uint8_t statcode);

#endif /* SP_H */
#endif /* BUILD_APPLE2 */

