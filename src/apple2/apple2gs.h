#ifdef BUILD_APPLE2
#ifdef __ORCAC__

#ifndef APPLE2GS_H
#define APPLE2GS_H

/* Return codes for get_ostype */
#define APPLE_UNKNOWN   0x00
#define APPLE_II        0x10  /* Apple ][                    */
#define APPLE_IIPLUS    0x11  /* Apple ][+                   */
#define APPLE_IIIEM     0x20  /* Apple /// (emulation)       */
#define APPLE_IIE       0x30  /* Apple //e                   */
#define APPLE_IIEENH    0x31  /* Apple //e (enhanced)        */
#define APPLE_IIECARD   0x32  /* Apple //e Option Card       */
#define APPLE_IIC       0x40  /* Apple //c                   */
#define APPLE_IIC35     0x41  /* Apple //c (3.5 ROM)         */
#define APPLE_IICEXP    0x43  /* Apple //c (Mem. Exp.)       */
#define APPLE_IICREV    0x44  /* Apple //c (Rev. Mem. Exp.)  */
#define APPLE_IICPLUS   0x45  /* Apple //c Plus              */
#define APPLE_IIGS      0x80  /* Apple IIgs                  */
#define APPLE_IIGS1     0x81  /* Apple IIgs (ROM 1)          */
#define APPLE_IIGS3     0x83  /* Apple IIgs (ROM 3)          */

#define get_ostype() APPLE_IIGS
#define allow_lowercase(onoff)

#endif /* APPLE2GS_H */
#endif /* __ORCAC__ */
#endif /* BUILD_APPLE2 */