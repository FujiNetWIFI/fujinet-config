#ifndef DW_CMD_IDS_H
#define DW_CMD_IDS_H        

/* Operation Codes */
#define OP_NOP         0
#define OP_JEFF        0xA5
#define OP_SERREAD     'C'
#define OP_SERREADM    'c'
#define OP_SERWRITE    0xC3
#define OP_SERWRITEM   0x64
#define OP_GETSTAT     'G'
#define OP_SETSTAT     'S'
#define OP_SERGETSTAT  'D'
#define OP_SERSETSTAT  'D'+128
#define OP_READ        'R'
#define OP_READEX      'R'+128
#define OP_WRITE       'W'
#define OP_REREAD      'r'
#define OP_REREADEX    'r'+128
#define OP_REWRITE     'w'
#define OP_INIT        'I'
#define OP_SERINIT     'E'
#define OP_SERTERM     'E'+128
#define OP_DWINIT      'Z'
#define OP_TERM        'T'
#define OP_TIME        '#'
#define OP_RESET3      0xF8
#define OP_RESET2      0xFE
#define OP_RESET1      0xFF
#define OP_PRINT       'P'
#define OP_PRINTFLUSH  'F'
#define OP_VPORT_READ  'C'
#define OP_FUJI        0xE2
#define OP_NET         0xE3
#define OP_CPM         0xE4
#define OP_CLOCK       0xE5
#define OP_NAMEOBJ_MNT 0x01

#endif /* DW_CMD_IDS_H */