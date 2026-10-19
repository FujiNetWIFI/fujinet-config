-- drive.lua: a scripted CONFIG session in MAME (make drive STEPS=...).
--
-- DRIVE_STEPS is a ';'-separated list:
--   w:N      wait N frames
--   k:KEY    press KEY 8 frames, release 8 frames. KEY is 0-9, fire (= 2),
--            clr, ent, or a disc direction up/down/left/right
--   t:TEXT   wait (up to 900 frames) for TEXT on screen; exit 1 if not
--   n:TEXT   fail if TEXT is on screen now
--   p:ADDR   print the byte at hex ADDR (program space)
--   r        console RESET (soft reset: the cart keeps its state)
--   d        print the screen as text
--   s        snapshot (-snapshot_directory)
--   q        quit, exit code 0
-- The screen is decoded from UVI RAM ($1800, 16 x 13): glyph & $3F
-- through the charset below; a row is shown lower-case where the
-- highlight (background bit 7 clear) sits, and '=' marks block-graphics
-- cells. Frames, never seconds: machine.time.seconds is an integer here.

local cpu = manager.machine.devices[":maincpu"]
local mem = cpu.spaces["program"]
local ports = manager.machine.ioport.ports

local CH = " /\\#^|_|++++<>><0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ.,+$/-_()%?:"

local function screen()
    local rows = {}
    for r = 0, 12 do
        local s, gfx = {}, false
        for c = 0, 15 do
            local b = mem:read_u8(0x1800 + r * 16 + c)
            if b == 0xC0 then gfx = true end
            local ch
            if gfx then
                ch = (b == 0xC0) and " " or ((b & 0x3F) == 0 and "." or "=")
            else
                local g = b & 0x3F
                ch = CH:sub(g + 1, g + 1)
                if (b & 0x80) == 0 and ch:match("%a") then ch = ch:lower() end
            end
            s[#s + 1] = ch
        end
        rows[#rows + 1] = table.concat(s)
    end
    return rows
end

local function dump()
    print("+----------------+")
    for _, l in ipairs(screen()) do print("|" .. l .. "|") end
    print("+----------------+")
end

local function contains(text)
    local all = table.concat(screen(), "\n"):upper()
    return all:find(text:upper(), 1, true) ~= nil
end

local KEYS = {
    ["1"] = {":controller1_col1", "P1 Keypad 1"},
    ["4"] = {":controller1_col1", "P1 Keypad 4"},
    ["7"] = {":controller1_col1", "P1 Keypad 7"},
    clr   = {":controller1_col1", "P1 Keypad Clear"},
    ["2"] = {":controller1_col2", "P1 Keypad 2/Button"},
    fire  = {":controller1_col2", "P1 Keypad 2/Button"},
    ["5"] = {":controller1_col2", "P1 Keypad 5"},
    ["8"] = {":controller1_col2", "P1 Keypad 8"},
    ["0"] = {":controller1_col2", "P1 Keypad 0"},
    ["3"] = {":controller1_col3", "P1 Keypad 3"},
    ["6"] = {":controller1_col3", "P1 Keypad 6"},
    ["9"] = {":controller1_col3", "P1 Keypad 9"},
    ent   = {":controller1_col3", "P1 Keypad Enter"},
}
local DISC = {up = 0x08, down = 0x04, left = 0x01, right = 0x02}

local function field(key)
    if DISC[key] then
        for _, f in pairs(ports[":joysticks"].fields) do
            if f.mask == DISC[key] then return f end
        end
    end
    local k = KEYS[key]
    if not k then error("unknown key " .. key) end
    return ports[k[1]].fields[k[2]]
end

local steps = {}
for s in string.gmatch(os.getenv("DRIVE_STEPS") or "w:300;d;q", "[^;]+") do
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
    if frame < 30 then return end           -- ioports are not ready at once
    if waitn > 0 then waitn = waitn - 1; return end
    if held then
        phase = phase + 1
        if phase == 8 then held:clear_value() end
        if phase < 16 then return end
        held = nil
    end
    local s = steps[idx]
    if not s then return finish(0) end
    local op, arg = s:match("^(%a):?(.*)$")
    if op == "w" then
        waitn = tonumber(arg); idx = idx + 1
    elseif op == "k" then
        held = field(arg); held:set_value(1); phase = 0; idx = idx + 1
    elseif op == "t" then
        if deadline == 0 then deadline = frame + 900 end
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
    elseif op == "s" then manager.machine.video:snapshot(); idx = idx + 1
    elseif op == "q" then return finish(0)
    else error("bad step " .. s) end
end)
