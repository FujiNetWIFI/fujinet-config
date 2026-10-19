#!/usr/bin/env python3
"""checkdepth.py -- bound the 2650 subroutine-call depth of a client.

The Signetics 2650's return address stack is EIGHT entries deep and on
chip; there is no RAM stack. A call graph deeper than 8 silently corrupts
returns (MAME wraps the stack), so every client must stay within it. This
walks the assembler source, builds the static call graph from BSTA/BSTR
(subroutine branches) to local labels, and reports the deepest chain.

It is a linter, not an assembler: it treats every `LABEL:` as a node and
every `BSTA,cc TARGET` / `BSTR,cc TARGET` whose TARGET is a known label as
an edge from the enclosing label. Tail calls (BCTA/BCTR) are branches, not
calls, so they do not add depth -- which is exactly why the clients use
them for dispatch. Indirect calls (BSXA and friends) are not used by any
client and are flagged if seen.

--flow adds what the plain walk misses (it attributes a call only to the
label directly above it, so a routine whose calls sit under local labels
reads shallower than it is): every label also gets zero-cost edges to
whatever it branches to (BC*, BD*, BI*, BRN*, BX*) and to the next label
it falls through to, the graph is condensed into strongly connected
components, and the depth is the longest chain of CALL edges through
it, counting return-stack entries only. A cycle that contains a call is
reported as a stack leak (a subroutine that branches into a state that
eventually calls it again) but is not fatal: the 2650's stack pointer
wraps, so an abandoned frame costs nothing -- the leaked call is left
out of the depth.

  checkdepth.py [--flow] [--max N] [--warn N] file.asm [file2.asm ...]

Exit nonzero if the deepest chain exceeds --max (default 8) or a cycle
(recursion -- unbounded on this stack) is found.
"""

import re
import sys

CALL_RE = re.compile(r'^\s*BS(?:TA|TR|XA)\s*,\s*\w+\s+([A-Za-z_.$][\w.$]*)',
                     re.IGNORECASE)
INDIRECT_RE = re.compile(r'^\s*BSXA\b', re.IGNORECASE)
LABEL_RE = re.compile(r'^([A-Za-z_.$][\w.$]*)\s*:')


def parse(paths):
    """Return {label: set(callee_labels)} and the ordered label list."""
    edges = {}
    order = []
    cur = None
    indirect = []
    for path in paths:
        with open(path) as f:
            for lineno, line in enumerate(f, 1):
                line = line.split(';', 1)[0]          # strip comments
                m = LABEL_RE.match(line)
                if m:
                    cur = m.group(1)
                    if cur not in edges:
                        edges[cur] = set()
                        order.append(cur)
                    # a label line can also carry an instruction after it
                    line = line[m.end():]
                if INDIRECT_RE.search(line):
                    indirect.append((path, lineno))
                c = CALL_RE.match(line if line.startswith((' ', '\t'))
                                  else ' ' + line)
                if c and cur is not None:
                    edges[cur].add(c.group(1))
    return edges, order, indirect


def deepest(edges):
    """Max call depth over the graph; raise on a cycle."""
    memo = {}
    stack = []

    def visit(node):
        if node in stack:
            raise ValueError("recursion: " + " -> ".join(stack + [node]))
        if node in memo:
            return memo[node]
        stack.append(node)
        best = 0
        best_path = [node]
        for callee in edges.get(node, ()):
            if callee not in edges:          # external / data label: leaf
                continue
            d = visit(callee)
            if d + 1 > best:
                best = d + 1
                best_path = [node] + memo_path[callee]
        stack.pop()
        memo[node] = best
        memo_path[node] = best_path
        return best

    memo_path = {}
    overall = 0
    overall_path = []
    for node in edges:
        d = visit(node)
        if d + 1 > overall:              # +1: the node itself occupies a slot
            overall = d + 1
            overall_path = memo_path[node]
    return overall, overall_path


BRANCH_RE = re.compile(
    r'^\s*(BC[TF][RA]|BD[RA][RA]|BI[RA][RA]|BRN[RA]|BXA)\s*,\s*\w+\s+'
    r'([A-Za-z_.$][\w.$]*)', re.IGNORECASE)
END_RE = re.compile(r'^\s*(BCT[RA]\s*,\s*UN|RETC\s*,\s*UN|RETE\s*,\s*UN|'
                    r'BXA|HALT)\b', re.IGNORECASE)
INSN_RE = re.compile(r'^\s+[A-Za-z]')


ZCALL_RE = re.compile(r'^\s*(ZBSR|ZBRR)\s+\*?([A-Za-z_.$][\w.$]*)',
                      re.IGNORECASE)
VEC_RE = re.compile(r'^([A-Za-z_.$][\w.$]*)\s*:\s*DWBE\s+([A-Za-z_.$][\w.$]*)',
                    re.IGNORECASE)


def parse_flow(paths):
    """Return (calls, jumps): {label: set}, both keyed by every label.

    ZBSR/ZBRR through a zero-page vector (LABEL: DWBE TARGET) count as a
    call/jump to TARGET."""
    vec = {}
    for path in paths:
        with open(path) as f:
            for line in f:
                m = VEC_RE.match(line.split(';', 1)[0])
                if m:
                    vec[m.group(1)] = m.group(2)
    calls, jumps = {}, {}
    cur = None
    live = False          # can control fall off the end of `cur`?
    for path in paths:
        with open(path) as f:
            for line in f:
                line = line.split(';', 1)[0].rstrip()
                if not line.strip():
                    continue
                m = LABEL_RE.match(line)
                if m:
                    lab = m.group(1)
                    calls.setdefault(lab, set())
                    jumps.setdefault(lab, set())
                    if cur is not None and live:
                        jumps[cur].add(lab)
                    cur = lab
                    live = True
                    line = ' ' + line[m.end():]
                    if not line.strip():
                        continue
                if cur is None or not INSN_RE.match(line):
                    continue
                word = line.split()[0].upper()
                if word in ('DB', 'DW', 'DWBE', 'ORG', 'EQU', 'STRAT'):
                    live = False          # data: nothing falls through
                    continue
                c = CALL_RE.match(line)
                if c:
                    calls[cur].add(c.group(1))
                z = ZCALL_RE.match(line)
                if z:
                    tgt = vec.get(z.group(2), z.group(2))
                    (calls if z.group(1).upper() == 'ZBSR'
                     else jumps)[cur].add(tgt)
                    if z.group(1).upper() == 'ZBRR':
                        live = False
                        continue
                b = BRANCH_RE.match(line)
                if b:
                    jumps[cur].add(b.group(2))
                live = not END_RE.match(line)
    return calls, jumps


def deepest_flow(calls, jumps):
    nodes = list(calls)
    succ = {n: [(t, 1) for t in calls[n] if t in calls] +
               [(t, 0) for t in jumps[n] if t in calls] for n in nodes}
    # Tarjan SCC (iterative)
    index, low, onstk, stk, comp = {}, {}, set(), [], {}
    counter = [0]
    sccs = []
    for root in nodes:
        if root in index:
            continue
        work = [(root, 0)]
        while work:
            v, i = work.pop()
            if i == 0:
                index[v] = low[v] = counter[0]; counter[0] += 1
                stk.append(v); onstk.add(v)
            edges = succ[v]
            if i < len(edges):
                work.append((v, i + 1))
                w = edges[i][0]
                if w not in index:
                    work.append((w, 0))
                elif w in onstk:
                    low[v] = min(low[v], index[w])
                continue
            for w, _ in edges:
                if w in onstk:
                    low[v] = min(low[v], low[w])
            if low[v] == index[v]:
                c = []
                while True:
                    w = stk.pop(); onstk.discard(w); comp[w] = len(sccs)
                    c.append(w)
                    if w == v:
                        break
                sccs.append(c)
    leaks = []
    for n in nodes:
        for t, cost in succ[n]:
            if cost and comp[t] == comp[n]:
                leaks.append((n, t))
    memo = {}

    def depth(ci):
        if ci in memo:
            return memo[ci]
        memo[ci] = (0, [])
        best = (0, [])
        for n in sccs[ci]:
            for t, cost in succ[n]:
                cj = comp[t]
                if cj == ci:
                    continue
                d, path = depth(cj)
                if d + cost > best[0]:
                    best = (d + cost, ([n + '->' + t] if cost else []) + path)
        memo[ci] = best
        return best

    import sys as _s
    _s.setrecursionlimit(100000)
    top = max((depth(i) for i in range(len(sccs))), key=lambda x: x[0])
    return top, leaks


def main():
    args = sys.argv[1:]
    hard, warn = 8, 6
    files = []
    flowmode = False
    i = 0
    while i < len(args):
        if args[i] == '--flow':
            flowmode = True; i += 1
        elif args[i] == '--max':
            hard = int(args[i + 1]); i += 2
        elif args[i] == '--warn':
            warn = int(args[i + 1]); i += 2
        else:
            files.append(args[i]); i += 1
    if not files:
        print(__doc__.strip(), file=sys.stderr)
        return 2

    if flowmode:
        calls, jumps = parse_flow(files)
        (depth, path), leaks = deepest_flow(calls, jumps)
        for n, t in sorted(set(leaks)):
            print(f"checkdepth: note: {n} calls {t} inside a cycle "
                  f"(a stack leak; harmless on the wrapping RAS)")
        chain = ", ".join(path)
        if depth > hard:
            print(f"checkdepth: FAIL depth {depth} > {hard}: {chain}",
                  file=sys.stderr)
            return 1
        flag = "  (WARN: close to the 8-deep limit)" if depth > warn else ""
        print(f"checkdepth: ok, max return-stack depth {depth}: {chain}{flag}")
        return 0

    edges, _, indirect = parse(files)
    if indirect:
        for path, ln in indirect:
            print(f"checkdepth: {path}:{ln}: indirect call (BSXA) "
                  f"cannot be bounded statically", file=sys.stderr)
        return 1
    try:
        depth, path = deepest(edges)
    except ValueError as e:
        print(f"checkdepth: {e}", file=sys.stderr)
        return 1

    chain = " -> ".join(path)
    if depth > hard:
        print(f"checkdepth: FAIL depth {depth} > {hard} (RAS is 8): {chain}",
              file=sys.stderr)
        return 1
    flag = "  (WARN: close to the 8-deep limit)" if depth > warn else ""
    print(f"checkdepth: ok, max call depth {depth}: {chain}{flag}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
