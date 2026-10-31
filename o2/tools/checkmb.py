#!/usr/bin/env python3
"""Fail a build when a cross-bank CALL does not restore the MB flag.

CALL and JMP on the 8048 carry 11 address bits; the twelfth comes from the MB
flip-flop. Macroassembler AS emits the SEL MB *before* a call whose target is in
the other bank -- and nothing after it. So this assembles clean and silently
jumps 2K low, into the console BIOS:

       800 : E5 94 05        call low_sub       ; E5 = SEL MB0
       803 : 04 00           jmp  high_sub      ; MB is still 0 -> $000

The firmware tree's answer was to keep every callable routine below $800, which
costs 1K -- and the browser cart already fills it to $7EF. This game needs ~2.5K,
so it puts code in both banks and pays one byte per call site instead:

    high-bank code:  call lo_x   ->  sel mb1   (F5)
    low-bank code:   call hi_x   ->  sel mb0   (E5)

`call lo_x` / `ret` is the subtle case: RET restores the PC but not MB, so a
high-bank caller resumes with MB=0 and its next intra-bank jump lands in the
BIOS. The restore is therefore required even when nothing in the routine itself
looks like it cares.

The one exception, and it is exact rather than a heuristic: a cross-bank call
that is IMMEDIATELY followed by another cross-bank call needs no restore between
them, because AS emits that second call's own SEL MB and it runs before anything
can observe the flag. The invariant then holds inductively -- whichever call is
last in such a run still has to restore.

Reads the AS listing rather than the binary: a raw byte stream cannot be walked
without knowing where instructions begin, and a table of card glyphs is full of
bytes that look like opcodes.
"""
import re
import sys

CALL_OPCODES = {0x14, 0x34, 0x54, 0x74, 0x94, 0xB4, 0xD4, 0xF4}  # aaa1_0100
JMP_OPCODES  = {0x04, 0x24, 0x44, 0x64, 0x84, 0xA4, 0xC4, 0xE4}  # aaa0_0100
SEL_MB0 = 0xE5
SEL_MB1 = 0xF5

# A cross-bank JMP carries its own SEL just like a CALL does, but nothing
# afterwards -- and RET does not restore MB. So a routine that TAIL-jumps across
# the banks hands its caller back with MB pointing at the callee's half, and the
# caller's next intra-bank jump lands 2K away. It is the same trap as the missing
# restore, except that there is no restore site to look at, so it cannot be
# checked by inspection: it depends on whether the target ever returns.
#
# One-way transfers are fine -- a screen hands off to the next screen and never
# comes back. Tail calls are not. There is no way to tell them apart from the
# byte stream, so they are declared here instead, and anything new fails the
# build until someone has decided which kind it is.
ONEWAY = {
    "st_boot":     "the boot enters page 0 and never comes back",
}

# Bytes, then two or more spaces, then the source text. AS pads the byte column,
# so the separation is unambiguous for the lowercase mnemonics this project uses.
LINE = re.compile(r"^(?:\(\d+\))?\s*\d+/\s*([0-9A-F]{3,4}) : ((?:[0-9A-F]{2} )+)\s{2,}(.*)$")

# A label, and only a label: it has to start the listing's source column. The
# obvious `\bword:` also matches a file citation inside a comment, and ONEWAY is
# keyed on the label, so a loose match would move an exemption onto the wrong
# routine.
LABEL = re.compile(r" : (?:(?:[0-9A-F]{2} )+)?\s*([a-z_][a-z_0-9]*):")


def check(path):
    mem = {}
    calls = []
    jumps = []
    sels = []
    label = "(start)"

    for line in open(path, errors="ignore"):
        m = LABEL.search(line)
        if m:
            label = m.group(1)
        m = LINE.match(line)
        if not m:
            continue
        addr = int(m.group(1), 16)
        data = [int(b, 16) for b in m.group(2).split()]
        src = m.group(3).strip()
        for i, b in enumerate(data):
            mem[addr + i] = b
        # Data is not code: a table byte that happens to be $E5 is no SEL.
        if re.match(r"(?:[a-z_][a-z_0-9]*:\s*)?d[bw]\b", src, re.I):
            continue
        # A cross-bank call is the SEL AS emitted, then the call itself. An
        # in-bank call has no prefix and needs no restore.
        if len(data) == 3 and data[0] in (SEL_MB0, SEL_MB1) and data[1] in CALL_OPCODES:
            calls.append((addr, data[0], src, label))
        if len(data) == 3 and data[0] in (SEL_MB0, SEL_MB1) and data[1] in JMP_OPCODES:
            jumps.append((addr, src, label))
        # A SEL on a line of its own. Classified per LINE and not per byte,
        # because an address operand is free to equal $E5 or $F5 -- `call
        # show_quads` assembles to F5 xx F5 when show_quads happens to sit at
        # $xF5, and a byte-walker reads that trailing F5 as an instruction.
        if data in ([SEL_MB0], [SEL_MB1]):
            sels.append((addr, data[0], src, label))

    bad = []
    for addr, sel, src, label in calls:
        caller_high = addr >= 0x800
        # AS's prefix must agree with which way the call goes; if it does not,
        # the listing is not what this checker thinks it is.
        if (sel == SEL_MB1) == caller_high:
            bad.append((addr, label, src, "prefix %02X from $%03X makes no sense" % (sel, addr)))
            continue
        want = SEL_MB1 if caller_high else SEL_MB0
        got = mem.get(addr + 3)
        # Another cross-bank call right behind this one carries its own SEL.
        chained = got in (SEL_MB0, SEL_MB1) and mem.get(addr + 4) in CALL_OPCODES
        if got != want and not chained:
            bad.append((addr, label, src,
                        "must be followed by %s (%02X), found %s"
                        % ("sel mb1" if want == SEL_MB1 else "sel mb0", want,
                           "%02X" % got if got is not None else "end of segment")))

    # The other direction, and the one that bites when code MOVES between the
    # halves: a restore that is no longer needed. Once a routine is reassembled
    # into the same half as what it calls, AS stops emitting the SEL in front of
    # the call -- and the hand-written restore behind it is then a live `sel mb`
    # that sets MB the wrong way. AS does not track explicit SELs (write one in
    # front of a cross-bank JMP and it still emits its own), so it will not save
    # you, and the resulting jump is as silent as the missing-restore case.
    #
    # Every standalone SEL in this program is a restore, so the rule is exact:
    # one has to sit three bytes past a cross-bank CALL.
    after_call = {addr + 3 for addr, _, _, _ in calls}
    for addr, sel, src, label in sels:
        if addr not in after_call:
            bad.append((addr, label, src,
                        "stray %s: nothing in front of it is a cross-bank call, "
                        "so this sets MB and never puts it back"
                        % ("sel mb1" if sel == SEL_MB1 else "sel mb0")))

    for addr, src, label in jumps:
        if label not in ONEWAY:
            bad.append((addr, label, src,
                        "cross-bank JMP: RET does not restore MB, so this is safe "
                        "only if it never returns. Declare it in ONEWAY, or make "
                        "it a call with a restore"))

    if bad:
        print("%s: MB RESTORE MISSING" % path, file=sys.stderr)
        for addr, label, src, why in sorted(bad):
            print("  $%03X in '%s': %s -- %s" % (addr, label, src, why), file=sys.stderr)
        return 1

    print("%s: %d cross-bank call(s), all restored; %d one-way jump(s); "
          "%d SEL(s), all accounted for" % (path, len(calls), len(jumps), len(sels)))
    return 0


sys.exit(max(check(p) for p in sys.argv[1:]))
