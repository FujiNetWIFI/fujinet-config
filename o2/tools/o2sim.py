#!/usr/bin/env python3
"""o2sim.py -- run 5 Card Stud for the O2 headless in MAME from a scenario.

    o2sim.py SCENARIO OUTDIR [IMAGE]

MAME's odyssey2 with the FujiNet cart device (`-cartslot fujinet`, grafted
by fujinet-firmware's pico/o2/emu/apply-mame.sh). SCENARIO is a file of
steps, one per line (# comments):

    wait N              N frames
    hold KEYS N         hold the stick (up down left right fire, joined by +)
                        for N frames, then let go for 4
    press KEYS          hold 4 frames, release 4
    type TEXT           type TEXT on the keyboard (0-9 A-Z ? . - * / = +; _ is
                        SPACE), each key held 4 frames, then 4 up
    key NAME [N]        one key, held N frames (4): a character above, or
                        SPACE YES NO CLR ENT
    snap NAME           a screenshot, OUTDIR/NAME.png
    burst NAME N        a screenshot every frame for N frames, NAME_000 ...
    burstkey NAME N KEY the same, with KEY held down throughout
    watch XRAM N [LEN]  for N frames, print external RAM XRAM (LEN bytes)
                        each time it changes: WATCH frame value ...
    xram ADDR [LEN]     print the console's external RAM
    iram ADDR [LEN]     print the 8048's internal RAM
    cart ADDR [LEN]     print the cart's program space ($400-$FFF)
    until XRAM V [N]    wait (at most N frames) until external RAM XRAM == V
    tap XRAM [LEN]      from here on, print each write to external RAM
                        XRAM..XRAM+LEN-1: TAP frame addr value
    pc                  print the PC, PSW and A

O2SIM_MACHINE=videopac: the PAL console instead of the NTSC one.
O2SIM_FAST=1: unthrottled even against a live FujiNet (the cart's pushes then
run inside one write: no progress to see, but quick).
O2SIM_SHOTS=N: a snapshot every N frames, shot_<frame> (for picking screenshots).
O2SIM_WAV=FILE: record the sound to FILE (tools/sndcheck.py reads it).

Offline unless O2SIM_FUJINET=host:port (then FUJINET_TCP points there); the
everyday fujinet-pc on 9995 is never used. Prints its lines prefixed so a
caller can grep; exits MAME hard (SIGKILL) if it outlives O2SIM_SECONDS.
"""
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
# O2SIM_MACHINE=videopac: the PAL G7000 (its lines are longer in CPU cycles)
MACHINE = os.environ.get("O2SIM_MACHINE", "odyssey2")
MAME = os.path.expanduser(os.environ.get("MAME", "~/Workspace/mame"))
KEYS = {"up": 0x01, "right": 0x02, "down": 0x04, "left": 0x08, "fire": 0x10}
# the keyboard: code = row x 8 + column (MAME's KEY.row bit column)
KBD = "0123456789\0\0_?LP+WERTUIOQSDFGHJKAZXCVBM.-*/=YN"
KBD_NAMED = {"SPACE": 12, "YES": 44, "NO": 45, "CLR": 46, "ENT": 47}


def keystep(code, n=4):
    return '{"key",%d,%d,%d}' % (code >> 3, 1 << (code & 7), n)

LUA = r"""
local steps = { %(steps)s }
local out = "%(out)s"
local frames, i, hold, holdleft, waitleft, untiladdr = 0, 1, 0, 0, 0, nil
local untilval, untilleft = 0, 0
local burstname, burstk, burstleft = "", 0, 0
local watchaddr, watchleft, watchlast, watchlen = 0, 0, "", 1
local cpu = manager.machine.devices[":maincpu"]
local prog = cpu.spaces["program"]
local ram = nil
local joy = manager.machine.ioport.ports[":JOY.0"]
local fields = {}
for name, f in pairs(joy.fields) do fields[#fields + 1] = f end
local names = {}
local keyport = {}
for k = 0, 5 do keyport[k] = manager.machine.ioport.ports[":KEY." .. k] end
local keyheld = nil
local function setkey(port, mask, v)
  for _, f in pairs(keyport[port].fields) do
    if f.mask == mask then f:set_value(v) end
  end
end
local function setjoy(mask)
  for _, f in ipairs(fields) do
    local bit = f.mask
    f:set_value((mask & bit) ~= 0 and 1 or 0)
  end
end
local function xram(a)
  if ram == nil then
    local items = manager.machine.devices[":"].items
    for k, v in pairs(items) do
      if string.find(k, "m_ram") then ram = emu.item(v) end
    end
  end
  return ram and ram:read(a) or -1
end
local function dump(kind, a, n)
  local t = {}
  for k = 0, n - 1 do
    local v
    if kind == "xram" then v = xram(a + k)
    elseif kind == "iram" then v = cpu.spaces["data"]:read_u8(a + k)
    else v = prog:read_u8(a + k) end
    t[#t + 1] = string.format("%%02X", v)
  end
  print(string.format("PEEK %%s %%03X: %%s", kind, a, table.concat(t, " ")))
end
local shotevery = %(shotevery)d
local function step()
  frames = frames + 1
  if shotevery > 0 and frames %% shotevery == 0 then
    manager.machine.video:snapshot()
    names[#names + 1] = string.format("shot_%%06d", frames)
  end
  if frames > %(maxframes)d then print("O2SIM: out of frames"); manager.machine:exit(); return end
  if holdleft > 0 then
    holdleft = holdleft - 1
    if holdleft == 0 then
      setjoy(0)
      if keyheld then setkey(keyheld[1], keyheld[2], 0); keyheld = nil end
      waitleft = 4
    end
    return
  end
  if waitleft > 0 then waitleft = waitleft - 1; return end
  if watchleft > 0 then
    local t = {}
    for k = 0, watchlen - 1 do t[#t + 1] = tostring(xram(watchaddr + k)) end
    local v = table.concat(t, " ")
    if v ~= watchlast then print(string.format("WATCH %%d %%s", frames, v)); watchlast = v end
    watchleft = watchleft - 1
    return
  end
  if burstleft > 0 then
    manager.machine.video:snapshot()
    names[#names + 1] = string.format("%%s_%%03d", burstname, burstk)
    burstk = burstk + 1; burstleft = burstleft - 1
    if burstleft == 0 and keyheld then setkey(keyheld[1], keyheld[2], 0); keyheld = nil end
    return
  end
  if untiladdr then
    if xram(untiladdr) == untilval or untilleft <= 0 then
      print(string.format("UNTIL %%03X = %%02X after %%d frames", untiladdr, xram(untiladdr), frames))
      untiladdr = nil
    else
      untilleft = untilleft - 1
      return
    end
  end
  while i <= #steps do
    local s = steps[i]; i = i + 1
    if s[1] == "wait" then waitleft = s[2]; return
    elseif s[1] == "hold" then setjoy(s[2]); holdleft = s[3]; return
    elseif s[1] == "key" then
      keyheld = {s[2], s[3]}; setkey(s[2], s[3], 1); holdleft = s[4]; return
    elseif s[1] == "snap" then
      manager.machine.video:snapshot(); names[#names + 1] = s[2]; waitleft = 1; return
    elseif s[1] == "until" then untiladdr = s[2]; untilval = s[3]; untilleft = s[4]; return
    elseif s[1] == "burst" then burstname = s[2]; burstk = 0; burstleft = s[3]; return
    elseif s[1] == "burstkey" then
      keyheld = {s[4], s[5]}; setkey(s[4], s[5], 1)
      burstname = s[2]; burstk = 0; burstleft = s[3]; return
    elseif s[1] == "tap" then
      local io = cpu.spaces["io"]
      local lo, hi = s[2], s[2] + s[3] - 1
      _G._o2taps = _G._o2taps or {}           -- each kept, or it's collected
      _G._o2taps[#_G._o2taps + 1] = io:install_write_tap(lo, hi, "o2tap" .. lo, function(offset, data, mask)
        if (cpu.state["P1"].value & 0x10) == 0 then
          print(string.format("TAP %%d %%02X %%02X", frames, offset, data))
        end
      end)
    elseif s[1] == "watch" then watchaddr = s[2]; watchleft = s[3]; watchlen = s[4]; watchlast = ""; return
    elseif s[1] == "pc" then
      print(string.format("PC %%03X SP %%d A %%02X P1 %%02X P2 %%02X TC %%02X", cpu.state["PC"].value, cpu.state["SP"].value, cpu.state["A"].value, cpu.state["P1"].value, cpu.state["P2"].value, cpu.state["TC"].value))
    else dump(s[1], s[2], s[3]) end
  end
  if i > #steps then
    local f = io.open(out .. "/snaps.txt", "w"); f:write(table.concat(names, " ")); f:close()
    manager.machine:exit()
  end
end
_G._o2sim = emu.add_machine_frame_notifier(function()
  local ok, err = pcall(step)
  if not ok then print("O2SIM LUA ERROR: " .. tostring(err)); manager.machine:exit() end
end)
"""


def parse(path):
    steps = []
    for line in open(path):
        w = line.split("#")[0].split()
        if not w:
            continue
        if w[0] == "wait":
            steps.append('{"wait",%d}' % int(w[1]))
        elif w[0] in ("hold", "press"):
            mask = 0
            for k in w[1].split("+"):
                mask |= KEYS[k]
            n = int(w[2]) if w[0] == "hold" else 4
            steps.append('{"hold",%d,%d}' % (mask, n))
        elif w[0] == "type":
            for ch in w[1]:
                steps.append(keystep(KBD.index(ch.upper())))
        elif w[0] == "key":
            n = w[1].upper()
            steps.append(keystep(KBD_NAMED[n] if n in KBD_NAMED else KBD.index(n),
                                 int(w[2]) if len(w) > 2 else 4))
        elif w[0] == "snap":
            steps.append('{"snap","%s"}' % w[1])
        elif w[0] in ("xram", "iram", "cart"):
            n = int(w[2], 0) if len(w) > 2 else 1
            steps.append('{"%s",%d,%d}' % (w[0], int(w[1], 16), n))
        elif w[0] == "burstkey":     # burstkey NAME N KEY: shots while KEY is held
            n = w[3].upper()
            code = KBD_NAMED[n] if n in KBD_NAMED else KBD.index(n)
            steps.append('{"burstkey","%s",%d,%d,%d}' % (w[1], int(w[2]), code >> 3, 1 << (code & 7)))
        elif w[0] == "burst":
            steps.append('{"burst","%s",%d}' % (w[1], int(w[2])))
        elif w[0] == "tap":
            steps.append('{"tap",%d,%d}' % (int(w[1], 16), int(w[2]) if len(w) > 2 else 1))
        elif w[0] == "watch":
            steps.append('{"watch",%d,%d,%d}' % (int(w[1], 16), int(w[2]), int(w[3]) if len(w) > 3 else 1))
        elif w[0] == "pc":
            steps.append('{"pc"}')
        elif w[0] == "until":
            n = int(w[3]) if len(w) > 3 else 600
            steps.append('{"until",%d,%d,%d}' % (int(w[1], 16), int(w[2], 0), n))
        else:
            sys.exit("o2sim: %s: unknown step %r" % (path, w[0]))
    return steps


def main(scen, out, image=None):
    image = os.path.abspath(image or os.path.join(HERE, "..", "build", "5card.bin"))
    out = os.path.abspath(out)
    os.makedirs(out, exist_ok=True)
    secs = int(os.environ.get("O2SIM_SECONDS", "120"))
    lua = LUA % {"steps": ", ".join(parse(scen)), "out": out,
                 "maxframes": 60 * secs,
                 "shotevery": int(os.environ.get("O2SIM_SHOTS", "0"))}
    luafile = os.path.join(out, "scenario.lua")
    open(luafile, "w").write(lua)
    snapdir = os.path.join(out, "raw")
    subprocess.run(["rm", "-rf", snapdir, os.path.join(out, "snaps.txt")])
    live = os.environ.get("O2SIM_FUJINET")
    env = dict(os.environ, FUJINET_TCP=live or "127.0.0.1:9",
               SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy")
    cmd = ["timeout", "-s", "KILL", str(secs + 60),
           "./mame", MACHINE, "-cartslot", "fujinet", "-cart", image,
           "-video", "none", "-sound", "none",
           "-snapshot_directory", snapdir,
           "-cfg_directory", os.path.join(out, "cfg"),
           "-nvram_directory", os.path.join(out, "nvram"),
           "-autoboot_script", luafile, "-skip_gameinfo",
           # the host's own keyboard and joysticks out: a gamepad on this
           # machine read as the stick held LEFT, mid-scenario
           "-keyboardprovider", "none", "-joystickprovider", "none",
           "-mouseprovider", "none", "-lightgunprovider", "none"]
    if os.environ.get("O2SIM_WAV"):
        cmd += ["-wavwrite", os.path.abspath(os.environ["O2SIM_WAV"])]
    if not live or os.environ.get("O2SIM_FAST"):
        cmd.append("-nothrottle")
    r = subprocess.run(cmd, capture_output=True, text=True, env=env, cwd=MAME)
    if os.environ.get("O2SIM_VERBOSE"):
        print(r.stdout, r.stderr)
    for line in (r.stdout + r.stderr).splitlines():
        if line.startswith(("PEEK", "UNTIL", "O2SIM", "PC ", "WATCH", "TAP", "fujinet:")):
            print(line)
    names = []
    if os.path.exists(os.path.join(out, "snaps.txt")):
        names = open(os.path.join(out, "snaps.txt")).read().split()
    d = os.path.join(snapdir, MACHINE)
    shots = sorted(os.listdir(d)) if os.path.isdir(d) else []
    for n, f in zip(names, shots):
        os.replace(os.path.join(d, f), os.path.join(out, n + ".png"))
        print(os.path.join(out, n + ".png"))


if __name__ == "__main__":
    main(*sys.argv[1:4])
