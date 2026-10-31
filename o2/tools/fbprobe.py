#!/usr/bin/env python3
"""fbprobe.py -- one Fuji command to a fujinet-pc over BoIP, as the O2 cart
sends it (FujiBus: SLIP-framed, pico/common/src/fujibus.c), and its reply.

    fbprobe.py HOST:PORT CMD [P1[:SIZE] ...] [--data HEX|--text STR [--pad N]]

CMD and the parameters are numbers (0x.. for hex); a parameter's SIZE is
1, 2 or 4 (1). The reply's ACK/NAK and its bytes go to stdout as a hex dump.
For checking a payload's layout against FujiNet without an emulator; never
point it at the everyday fujinet-pc on 9995 or a production one.
"""
import socket
import sys

END, ESC, ESC_END, ESC_ESC = 0xC0, 0xDB, 0xDC, 0xDD
FSIZE = [0, 1, 1, 1, 1, 2, 2, 4]
FCOUNT = [0, 1, 2, 3, 4, 1, 2, 1]


def cksum(b):
    c = 0
    for x in b:
        c += x
        c = (c >> 8) + (c & 0xFF)
    return c & 0xFF


def request(dev, cmd, params, payload):
    pkt = bytearray(6)
    if params:
        descr = []
        for i, (v, size) in enumerate(params):
            d = 1 + (4 if size > 1 else 0) + (2 if size > 2 else 0)
            descr.append(d | (0x80 if i + 1 < len(params) else 0))
        pkt[5] = descr[0]
        pkt += bytes(descr[1:])
        for v, size in params:
            pkt += v.to_bytes(size, "little")
    pkt += payload
    pkt[0], pkt[1] = dev, cmd
    pkt[2:4] = len(pkt).to_bytes(2, "little")
    pkt[4] = cksum(pkt)
    out = bytearray([END])
    for x in pkt:
        out += bytes([ESC, ESC_END if x == END else ESC_ESC]) if x in (END, ESC) else bytes([x])
    return bytes(out + bytes([END]))


def reply(sock):
    buf = bytearray()
    started = False
    while True:
        b = sock.recv(1)
        if not b:
            raise SystemExit("fbprobe: connection closed")
        x = b[0]
        if x == END:
            if started and buf:
                break
            started = True
            continue
        buf.append(x)
    dec = bytearray()
    i = 0
    while i < len(buf):
        if buf[i] == ESC:
            i += 1
            dec.append(END if buf[i] == ESC_END else ESC)
        else:
            dec.append(buf[i])
        i += 1
    off = 6
    descr = [dec[5]]
    while descr[-1] & 0x80:
        descr.append(dec[off])
        off += 1
    for d in descr:
        off += FSIZE[d & 7] * FCOUNT[d & 7]
    return dec[1], bytes(dec[off:])


def main(argv):
    host, port = argv[0].rsplit(":", 1)
    cmd = int(argv[1], 0)
    params, payload, pad = [], b"", 0
    i = 2
    while i < len(argv):
        a = argv[i]
        if a == "--data":
            payload = bytes.fromhex(argv[i + 1])
            i += 2
        elif a == "--text":
            payload = argv[i + 1].encode().replace(b"\\0", b"\0")
            i += 2
        elif a == "--pad":
            pad = int(argv[i + 1])
            i += 2
        else:
            v, _, s = a.partition(":")
            params.append((int(v, 0), int(s or 1)))
            i += 1
    if pad:
        payload = payload.ljust(pad, b"\0")
    with socket.create_connection((host, int(port)), timeout=30) as s:
        s.sendall(request(0x70, cmd, params, payload))
        ack, data = reply(s)
    print("%s %d bytes" % ({6: "ACK", 0x15: "NAK"}.get(ack, "%02X" % ack), len(data)))
    for o in range(0, len(data), 16):
        row = data[o:o + 16]
        print("%04X  %-48s %s" % (o, " ".join("%02X" % x for x in row),
                                  "".join(chr(x) if 32 <= x < 127 else "." for x in row)))


if __name__ == "__main__":
    main(sys.argv[1:])
