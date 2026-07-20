typedef unsigned char uint8_t;
typedef unsigned int uint16_t;

/* CMOC clone of the DWLOAD core behavior. Bypasses BASIC's command-line
 * parser state. */

#define OP_READEX ('R' + 128)

#define DW_OP_NAMEOBJ_MOUNT 0x01
#define DW_DNUM_ADDR 0x0012

#define BASIC_TXTTAB_PTR 0x0019
#define BASIC_VARTAB_PTR 0x001B
#define BASIC_ARYTAB_PTR 0x001D
#define BASIC_STRTAB_PTR 0x001F
#define BASIC_EXEC_PTR   0x009D

#define ROM_BASVECT2 0x83ED
#define ROM_BASVECT1 0x841F
#define ROM_RUNBASIC 0x849F


/* Shared argument block with dwload_cmoc_bridge.asm */
uint16_t dw_arg_ptr;
uint16_t dw_arg_len;
uint16_t dw_arg_lsn;
uint16_t dw_arg_checksum;
uint16_t dw_arg_addr;
uint8_t dw_arg_uint8_t;
uint8_t dw_status;

extern void dw_set_drive_from_arg(void);
extern void dw_write_from_args(void);
extern void dw_read_from_args(void);
extern void dw_doread_from_args(void);
extern void rom_call_from_arg(void);

static uint8_t stream_buf[256];
static uint16_t stream_sector;
static uint16_t stream_pos;

static void copy_uint8_ts(uint8_t *dst, const uint8_t *src, uint16_t n)
{
    while (n--) {
        *dst++ = *src++;
    }
}

static uint16_t be16(const uint8_t *p)
{
    return ((uint16_t) p[0] << 8) | (uint16_t) p[1];
}

static void poke16(uint16_t addr, uint16_t value)
{
    *((uint8_t *) addr) = (uint8_t) (value >> 8);
    *((uint8_t *) (addr + 1)) = (uint8_t) value;
}

static uint16_t peek16(uint16_t addr)
{
    uint16_t hi = *((uint8_t *) addr);
    uint16_t lo = *((uint8_t *) (addr + 1));
    return (uint16_t) ((hi << 8) | lo);
}

static int dw_ok(void)
{
    /* CC bits captured in dw_status: bit0=C, bit2=Z */
    return ((dw_status & 0x01) == 0) && ((dw_status & 0x04) != 0);
}

static int dw_write_packet(const uint8_t *s, uint16_t l)
{
    /* Save/restore U/Y: the DriveWire ROM vector isn't CMOC code and may
     * clobber CMOC's frame pointer/callee-saved registers. */
    asm
    {
        pshs x,y,u
        ldx :s
        ldy :l
#ifdef DRAGON
        jsr [0xFA00]
#else
        jsr [0xD941]
#endif
        tfr cc,d
        puls u,y,x
    }
}

static int dw_read_packet(uint8_t *s, uint16_t l)
{
    /* See dw_write_packet. */
    asm
    {
        pshs x,y,u
        ldx :s
        ldy :l
#ifdef DRAGON
        jsr [0xF9FE]
#else
        jsr [0xD93F]
#endif
        puls u,y,x
        tfr cc,b
        lsrb
        lsrb
        andb #$01
    }
}

#if 0
static int dw_read_sector(uint16_t lsn, uint8_t *buf)
{
    dw_arg_lsn = lsn;
    dw_arg_ptr = (uint16_t) buf;
    dw_doread_from_args();
    return dw_ok() ? 0 : -1;
}
#endif

// Must match fujinet-firmware's drivewire_checksum(): a plain sum over all
// `len` bytes.
uint16_t drivewire_checksum(uint8_t *buf, unsigned short len)
{
    uint16_t chk = 0;

    for (unsigned short i = 0; i < len; i++)
        chk += buf[i];

    return chk;
}

//int network_read_coco(char* devicespec, uint8_t *buf, unsigned int len)
int dw_read_sector(uint16_t lsn, uint8_t *buf)
{
    uint8_t z = 0;
    uint16_t len = 256;
    uint16_t c1 = 0, c2 = 0;

    struct _read_ex
    {
        uint8_t opcode;
        uint8_t drive_no;  // 0 for our purpose here to implement dwload
        uint8_t lsn_hi;    // limiting to 0 for dwload purposes
        uint8_t lsn_mid;
        uint8_t lsn_lo;
    } rc;

    rc.opcode = OP_READEX;
    rc.drive_no = 0;
    rc.lsn_hi = 0;
    rc.lsn_mid = (uint8_t)(lsn >> 8);
    rc.lsn_lo =  (uint8_t)(lsn & 0xff);

    dw_write_packet((uint8_t *)&rc, sizeof(rc));

    // read the sector
    dw_read_packet(buf, len);
    c1 = drivewire_checksum(buf, len);
    dw_write_packet((uint8_t *)&c1, 2); // send checksum
    dw_read_packet(&z, 1);

    return z; /* Yeah, I know, FIXME. */
}

#if 0
static void call_rom(uint16_t addr)
{
    dw_arg_addr = addr;
    rom_call_from_arg();
}
#endif

static int stream_fill(void)
{
    if (dw_read_sector(stream_sector, stream_buf) != 0) {
        return -1;
    }
    stream_sector++;
    stream_pos = 0;
    return 0;
}

static int stream_read(uint8_t *dst, uint16_t n)
{
//    printf("at %d... dst=%p, n=%d\n", __LINE__, dst, n);
    while (n != 0) {
        uint16_t avail;
        uint16_t chunk;

        if (stream_pos >= 256 && stream_fill() != 0) {
            return -1;
        }

        avail = (uint16_t) (256 - stream_pos);
        chunk = (n < avail) ? n : avail;
        copy_uint8_ts(dst, stream_buf + stream_pos, chunk);
        dst += chunk;
        stream_pos += chunk;
        n -= chunk;
    }
    return 0;
}

/*
 * Returns 0 on success, non-zero on failure.
 * execute_nonzero:
 *   0 => load only
 *   1 => load and run (DWLOAD default)
 */
int dwload_clone(const char *filename, uint8_t execute_nonzero)
{
    static uint8_t req[258];
    uint8_t hdr[5];
    uint8_t ddb[4];
    uint8_t drive;
    uint16_t name_len = 0;

    while (filename[name_len] != '\0') {
        if (name_len >= 255) {
            return 1;
        }
        req[2 + name_len] = (uint8_t) filename[name_len];
        name_len++;
    }

    req[0] = DW_OP_NAMEOBJ_MOUNT;
    req[1] = (uint8_t) name_len;

    if (dw_write_packet(req, (uint16_t) (name_len + 2)) != 0) {
        //printf("dwload_clone: failed to send mount request\n");
        //return 2;
    }

    /* Server always sends back a 1-byte ack after the mount request;
     * must be read or it shifts every subsequent stream read by one byte. */
    dw_read_packet(&drive, 1);

    dw_arg_uint8_t = drive;
//    dw_set_drive_from_arg();

    stream_sector = 0;
    stream_pos = 256;

    for (;;) {
        uint16_t len;

        if (stream_read(hdr, sizeof(hdr)) != 0) {
            return 5;
        }

        if (hdr[0] == 0x00) {
            /* Standard DECB ML segment: 00 lenHi lenLo addrHi addrLo */
            uint16_t load_addr = be16(&hdr[3]);
            len = be16(&hdr[1]);
            if (stream_read((uint8_t *) load_addr, len) != 0) {
                ("at %d...", __LINE__);
                return 6;
            }
            continue;
        }

        if (hdr[0] == 0x55) {
            uint8_t type;
            uint16_t exec_addr;

            /* DragonDOS header variant handled similarly to ROM code. */
            if (stream_read(ddb, sizeof(ddb)) != 0) {
                //printf("at %d...", __LINE__);
                //return 7;
            }

            type = hdr[1];
#if 0            
            if (type == 0x01) {
                /* DragonDOS BASIC payload. */
                uint16_t dst = peek16(BASIC_TXTTAB_PTR);
                len = be16(&ddb[1]);
                if (stream_read((uint8_t *) dst, len) != 0) {
                    return 8;
                }

                dst = (uint16_t) (dst + len);
                poke16(BASIC_VARTAB_PTR, dst);
                poke16(BASIC_ARYTAB_PTR, dst);
                poke16(BASIC_STRTAB_PTR, dst);

                if (execute_nonzero) {
                    call_rom(ROM_BASVECT1);
                    call_rom(ROM_BASVECT2);
                    call_rom(ROM_RUNBASIC);
                }
                return 0;
            }
#endif
            /* DragonDOS binary payload. */
            {
                uint16_t load_addr = be16(&hdr[2]);
                len = ((uint16_t) hdr[4] << 8) | (uint16_t) ddb[0];
                if (stream_read((uint8_t *) load_addr, len) != 0) {
                //    return 9;
                }

                exec_addr = be16(&ddb[1]);
                poke16(BASIC_EXEC_PTR, exec_addr);
                if (execute_nonzero && (uint8_t) (exec_addr >> 8) != 0xFF) {
                    /* True jump, not a call: restore S to our own
                     * entry-time value (INISTK) first so the launched
                     * program inherits a clean stack instead of ours,
                     * which would leave garbage under its return address
                     * by the time it calls exit(). */
                    asm
                    {
INISTK                  IMPORT
                        ldx :exec_addr
                        lds INISTK
                        jmp ,x
                    }
                }
                return 0;
            }
        }
#if 0
        if (hdr[0] == 0xFF) {
            /* BASIC token file special case as in DWLOAD:
             * first header in first sector: FF lenHi lenLo uint8_t0 uint8_t1 ...
             */
            if (stream_sector == 1 && stream_pos == 5) {
                uint16_t dst = peek16(BASIC_TXTTAB_PTR);
                len = be16(&hdr[1]);
                if (len >= 1) {
                    ((uint8_t *) dst)[0] = hdr[3];
                }
                if (len >= 2) {
                    ((uint8_t *) dst)[1] = hdr[4];
                }
                if (len > 2 && stream_read((uint8_t *) (dst + 2), (uint16_t) (len - 2)) != 0) {
                    return 10;
                }

                dst = (uint16_t) (dst + len);
                poke16(BASIC_VARTAB_PTR, dst);
                poke16(BASIC_ARYTAB_PTR, dst);
                poke16(BASIC_STRTAB_PTR, dst);

                if (execute_nonzero) {
                    call_rom(ROM_BASVECT1);
                    call_rom(ROM_BASVECT2);
                    call_rom(ROM_RUNBASIC);
                }
                return 0;
            }

            /* End segment: FF spHi spLo execHi execLo */
            {
                uint16_t exec_addr = be16(&hdr[3]);
                poke16(BASIC_EXEC_PTR, exec_addr);

                if (execute_nonzero && (uint8_t) (exec_addr >> 8) != 0xFF) {
                    call_rom(exec_addr);
                }
            }
              
            return 0;
        }
#endif  
        return 11;
    }
    return 0;
}

int dwload_autoload_clone(uint8_t execute_nonzero)
{
    return dwload_clone("AUTOLOAD.DWL", execute_nonzero);
}
