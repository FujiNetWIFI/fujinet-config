-- drive.lua -- a scripted CONFIG session in MAME (make drive STEPS=...).
--
-- DRIVE_STEPS is a ';'-separated list:
--   w:N      wait N frames
--   k:KEY    hold KEY 6 frames, then 10 off: up down left right fire back
--            star hash 0-9 (fire is the left button, back the right)
--   t:TEXT   wait up to 1800 frames for TEXT on screen; fail if it never
--            shows
--   n:TEXT   fail if TEXT is on screen now
--   p:ADDR   print the byte at hex ADDR
--   d        print the screen as text
--   s:NAME   save the screen as NAME.png in the snapshot directory
--   r        soft reset (the cart keeps its state)
--   q        quit
-- The screen is read from the name table at VRAM 1800H: the font is
-- ASCII, frame tiles read as + - |, the pointer as >, the bar as [ . = # ].
-- A dump's header gives CONFIG's state and the pointer's row from RAM
-- 7000H-7002H (config.asm keeps them there). Time is counted in frames.

-- MAME runs this script again after a soft reset; the first run carries on.
if _G.DRIVE_SUB then return end

local cpu = manager.machine.devices[":maincpu"]
local mem = cpu.spaces["program"]
local vdp = manager.machine.devices[":tms9928a"]
local vram = vdp.spaces["vram"]

local function find_port(suffix)
    for tag, port in pairs(manager.machine.ioport.ports) do
        if tag:sub(-#suffix) == suffix then return port end
    end
    error("drive.lua: no input port ending in " .. suffix)
end
local joy = find_port("STD_JOY1")
local pad = find_port("STD_KEYPAD1")

local KEYS = {
    up = {joy, 0x01}, right = {joy, 0x02}, down = {joy, 0x04},
    left = {joy, 0x08}, fire = {joy, 0x40}, back = {pad, 0x4000},
    hash = {pad, 0x0400}, star = {pad, 0x0800},
}
for n = 0, 9 do KEYS[tostring(n)] = {pad, 1 << n} end

local function glyph(b)
    if b >= 0x20 and b < 0x7F then return string.char(b) end
    if b == 0 then return " " end
    if b >= 0x01 and b <= 0x04 then return "+" end
    if b == 0x05 or b == 0x06 then return "-" end
    if b == 0x07 or b == 0x08 then return "|" end
    if b == 0x09 or b == 0x18 then return ">" end
    if b == 0x0A then return "[" end
    if b == 0x0B then return "]" end
    if b == 0x0C then return "." end
    if b == 0x14 then return "#" end
    if b > 0x0C and b < 0x14 then return "=" end
    if b >= 0x80 and b < 0xE0 then return "@" end
    return "?"
end

local function screen()
    local rows = {}
    for r = 0, 23 do
        local s = {}
        for c = 0, 31 do s[#s + 1] = glyph(vram:read_u8(0x1800 + r * 32 + c)) end
        rows[#rows + 1] = table.concat(s)
    end
    return rows
end

local function dump()
    print(string.format("drive: state %d, pointer %s", mem:read_u8(0x7002),
        mem:read_u8(0x7000) ~= 0 and ("on row " .. mem:read_u8(0x7001)) or "off"))
    for i, l in ipairs(screen()) do print(string.format("%2d %s", i - 1, l)) end
end

local function contains(text)
    return table.concat(screen(), "\n"):find(text, 1, true) ~= nil
end

local steps = {}
for s in string.gmatch(os.getenv("DRIVE_STEPS") or "t:HOSTS;d;q", "[^;]+") do
    steps[#steps + 1] = s
end

local frame, idx, waitn, held, phase, deadline = 0, 1, 0, nil, 0, 0
local function finish(code)
    if code ~= 0 then dump() end
    print("drive: " .. (code == 0 and "PASS" or "FAIL") .. " at frame " .. frame)
    manager.machine:exit()
end

_G.DRIVE_SUB = emu.add_machine_frame_notifier(function()
    frame = frame + 1
    if frame < 30 then return end
    if waitn > 0 then waitn = waitn - 1; return end
    if held then
        phase = phase + 1
        if phase == 6 then held:clear_value() end
        if phase < 16 then return end
        held = nil
    end
    local s = steps[idx]
    if not s then return finish(0) end
    local op, arg = s:match("^(%a):?(.*)$")
    if op == "w" then
        waitn = tonumber(arg); idx = idx + 1
    elseif op == "k" then
        local k = KEYS[arg]
        if not k then error("drive.lua: unknown key " .. arg) end
        held = k[1]:field(k[2]); held:set_value(1); phase = 0; idx = idx + 1
    elseif op == "t" then
        if deadline == 0 then deadline = frame + 1800 end
        if contains(arg) then deadline = 0; idx = idx + 1
        elseif frame > deadline then
            print("drive: timed out waiting for '" .. arg .. "'")
            return finish(1)
        end
    elseif op == "n" then
        if contains(arg) then
            print("drive: unexpected '" .. arg .. "'")
            return finish(1)
        end
        idx = idx + 1
    elseif op == "p" then
        local a = tonumber(arg, 16)
        print(string.format("drive: [%04X] = %02X", a, mem:read_u8(a)))
        idx = idx + 1
    elseif op == "r" then manager.machine:soft_reset(); idx = idx + 1
    elseif op == "d" then dump(); idx = idx + 1
    elseif op == "s" then
        manager.machine.screens[":screen"]:snapshot(arg .. ".png"); idx = idx + 1
    elseif op == "q" then return finish(0)
    else error("drive.lua: bad step " .. s) end
end)
