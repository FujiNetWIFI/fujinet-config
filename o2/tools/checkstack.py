#!/usr/bin/env python3
"""checkstack.py -- fail a build that can overflow the 8048's 8-level stack.

    checkstack.py [--far N,...] LISTING...
                                  one per page; every page's fixed half is the
                                  same; --far: the pages farcall can reach (by
                                  their place in the list), default all but 0

The stack wraps silently: a ninth call overwrites the first return address
and the program returns somewhere else, sooner or later. So the depth is
computed, not guessed: control flow is followed from each root through the
listing's bytes -- jumps and branches within a routine, calls into routines,
JMPP through the dispatch tables (the table named by the `add a,#tab&0FFh`
just before it) -- and the deepest chain below each root is the budget it
needs:

    interrupt   vbl_isr / tim_isr: 1 for the interrupt itself, plus calls
    main        page 0's boot and every page's farcall entries; farcall's own
                `call 800h` lands in some other page's entry, so it is charged
                the deepest of those

and main + interrupt must fit in 8. A routine that loops back on itself or
recurses fails as well: depth would be unbounded.
"""
import re
import sys

LINE = re.compile(r"^(?:\(\d+\))?\s*\d+/\s*([0-9A-F]{1,4}) : ((?:[0-9A-F]{2} )*)\s*(.*)$")
LABEL = re.compile(r"^([a-z_][a-z_0-9]*):")
ADDTAB = re.compile(r"add a,#\(?([a-z_][a-z_0-9]*)\)?&0FFh", re.I)

COND = {0xF6, 0xE6, 0xC6, 0x96, 0x36, 0x26, 0x56, 0x46, 0xB6, 0x76, 0x16, 0x86}
COND |= {0x12 + 0x20 * b for b in range(8)}          # JB0..JB7
DJNZ = set(range(0xE8, 0xF0))
# the two-byte instructions: immediates, conditional jumps, DJNZ (JMP and CALL
# are decoded on their own)
TWO = COND | DJNZ | {0x03, 0x13, 0x23, 0x43, 0x53, 0xD3,      # A,#data
                     0x88, 0x89, 0x8A, 0x98, 0x99, 0x9A,      # ORL/ANL port,#data
                     0xB0, 0xB1} | set(range(0xB8, 0xC0))     # MOV @Ri/Rn,#data
RET, RETR, JMPP = 0x83, 0x93, 0xB3
SEL_MB0, SEL_MB1 = 0xE5, 0xF5
LIMIT = 8


def is_call(op):
    return op & 0x1F == 0x14


def is_jmp(op):
    return op & 0x1F == 0x04


class Image:
    def __init__(self, path):
        self.mem, self.labels, self.addtab = {}, {}, {}
        self.order = []
        for line in open(path, errors="ignore"):
            m = LINE.match(line)
            if not m:
                continue
            addr = int(m.group(1), 16)
            if addr < 0x400:
                continue                # a macro's definition, listed at 0
            data = [int(b, 16) for b in m.group(2).split()]
            src = m.group(3)
            lm = LABEL.match(src.strip())
            if lm:
                self.labels.setdefault(lm.group(1), addr)
            am = ADDTAB.search(src.split(";")[0])
            if am:
                self.addtab[addr + len(data)] = am.group(1)
            for i, b in enumerate(data):
                self.mem[addr + i] = b
        self.byaddr = sorted((a, n) for n, a in self.labels.items())

    def table(self, name):
        """The bytes from label `name` to the next label: a dispatch table."""
        start = self.labels[name]
        nxt = min((a for a, _ in self.byaddr if a > start), default=start + 1)
        return [self.mem[a] for a in range(start, nxt) if a in self.mem]


def name_of(img, addr):
    best = None
    for a, n in img.byaddr:
        if a <= addr:
            best = (a, n)
    return "%s+%d" % (best[1], addr - best[0]) if best and best[0] != addr else (best[1] if best else "$%03X" % addr)


class Checker:
    def chain(self, start, isr):
        out, key = [], (start, isr)
        while key in self.why:
            pc, tgt, isr = self.why[key]
            out.append("%s calls %s" % (name_of(self.img, pc), name_of(self.img, tgt)))
            key = (tgt, isr)
        return out

    def __init__(self, img, name):
        self.img, self.name = img, name
        self.memo = {}
        self.why = {}               # key -> (call site, callee) of the deepest
        self.stack = set()
        self.errors = []
        self.farcall_sites = []

    def depth(self, start, isr):
        key = (start, isr)
        if key in self.memo:
            return self.memo[key]
        if key in self.stack:
            self.errors.append("recursion through $%03X" % start)
            return 0
        self.stack.add(key)
        best = 0
        seen = set()
        work = [(start, start >> 11 & 1 if not isr else 0)]
        mem = self.img.mem
        while work:
            pc, mb = work.pop()
            if pc in seen or pc not in mem:
                continue
            seen.add(pc)
            op = mem[pc]
            if op in (SEL_MB0, SEL_MB1):
                work.append((pc + 1, 0 if isr else (1 if op == SEL_MB1 else 0)))
                continue
            if is_call(op) or is_jmp(op):
                tgt = ((op >> 5) << 8 | mem.get(pc + 1, 0)) | (0 if isr else mb << 11)
                if is_call(op):
                    if self.img.labels.get("farcall") == tgt:
                        self.farcall_sites.append(pc)
                    d = 1 + self.depth(tgt, isr)
                    if d > best:
                        best = d
                        self.why[key] = (pc, tgt, isr)
                    work.append((pc + 2, mb))
                elif tgt != self.img.labels.get("far_go"):
                    # far_go hands over to a page's entry at depth 0, where
                    # every entry is counted anyway: the path ends here
                    work.append((tgt, mb))
                continue
            if op in (RET, RETR):
                continue
            if op == JMPP:
                tab = self.img.addtab.get(pc)
                if tab is None and pc - 1 in self.img.mem:
                    tab = None
                targets = []
                if tab is not None:
                    targets = self.img.table(tab)
                elif "jtab" in self.img.labels and pc < self.img.labels["jtab"] < pc + 4:
                    targets = self.img.table("jtab")
                else:
                    self.errors.append("JMPP at $%03X: no table found" % pc)
                for t in targets:
                    work.append(((pc & 0xF00) | t, mb))
                continue
            if op in COND or op in DJNZ:
                work.append(((pc & 0xF00) | mem.get(pc + 1, 0), mb))
                work.append((pc + 2, mb))
                continue
            work.append((pc + (2 if op in TWO else 1), mb))
        self.stack.discard(key)
        self.memo[key] = best
        return best


def run(imgs, paths, farcall_cost):
    checks = [Checker(i, p) for i, p in zip(imgs, paths)]
    for ck in checks:
        fc = ck.img.labels.get("farcall")
        if fc is not None:
            ck.memo[(fc, False)] = farcall_cost
    entries = []
    for ck in checks:
        if "pg_tab" in ck.img.labels:
            for t in ck.img.table("pg_tab"):
                entries.append((ck.depth(0x800 | t, False), ck.name, 0x800 | t))
    return checks, entries


def main(args):
    far = None
    if args and args[0] == "--far":
        far = {int(n) for n in args[1].split(",")}
        args = args[2:]
    paths = args
    farpaths = [p for i, p in enumerate(paths) if (i in far if far is not None else i != 0)]
    imgs = [Image(p) for p in paths]
    # farcall is `call farcall` -> `call 800h` -> some page's entry. Pass 1
    # charges it nothing below the entry to find the deepest entry; pass 2
    # charges every farcall that entry, so a farcall inside an entry -- which
    # fixed.inc forbids -- shows up as depth.
    _, entries = run(imgs, paths, 1)
    # page 0's entry is the boot, reached by the reset jump, never by farcall
    deepest = max([e[0] for e in entries if e[1] in farpaths] or [0])
    checks, entries = run(imgs, paths, 1 + deepest)
    first = checks[0]
    isr = 1 + max(first.depth(first.img.labels["vbl_isr"], True),
                  first.depth(first.img.labels["tim_isr"], True))
    worst = max(entries)
    errors = sorted({e for ck in checks for e in ck.errors})
    print("checkstack: interrupts %d, main %d (%s $%03X), limit %d"
          % (isr, worst[0], worst[1].split("/")[-1], worst[2], LIMIT))
    if errors or worst[0] + isr > LIMIT:
        for e in errors:
            print("  " + e, file=sys.stderr)
        if worst[0] + isr > LIMIT:
            print("  the main program nests %d deep and the interrupts %d: %d > %d"
                  % (worst[0], isr, worst[0] + isr, LIMIT), file=sys.stderr)
            ck = [c for c in checks if c.name == worst[1]][0]
            for step in ck.chain(worst[2], False):
                print("    " + step, file=sys.stderr)
        return 1
    return 0


sys.exit(main(sys.argv[1:]))
