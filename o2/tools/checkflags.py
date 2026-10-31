#!/usr/bin/env python3
"""checkflags.py -- fail a build on a test of A that some MOV has overwritten.

The 8048's JZ/JNZ test the accumulator itself, not a flag, so
    xrl a,#6 / mov a,r6 / jnz loop
tests r6, not the comparison -- an easy slip that bit three times. Flags this
shape: an ALU operation on A, then a MOV into A, then JZ or JNZ.
"""
import re
import sys

bad = 0
for path in sys.argv[1:]:
    lines = [l.split(";")[0].strip() for l in open(path)]
    for i in range(len(lines) - 2):
        a, b, c = lines[i:i + 3]
        if (re.match(r"(xrl|anl|orl|add|addc|dec a|inc a|cpl a)\b", a)
                and re.match(r"mov a,", b) and re.match(r"(jz|jnz)\b", c)):
            print("%s:%d: '%s' then '%s' -- %s tests the MOV, not the %s"
                  % (path, i + 1, a, b, c.split()[0], a.split()[0]), file=sys.stderr)
            bad = 1
sys.exit(bad)
