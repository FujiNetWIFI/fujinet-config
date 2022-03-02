#ifdef BUILD_APPLE2
/**
 * FujiNet CONFIG for #Apple2
 *
 * SmartPort MLI Routines
 */

#ifndef SP_H
#define SP_H

#define SP_CMD_STATUS 0
#define SP_CMD_CONTROL 4

extern unsigned char sp_payload[1024];
extern unsigned int sp_count;

unsigned char sp_status(unsigned char statcode);

#endif /* SP_H */
#endif /* BUILD_APPLE2 */

