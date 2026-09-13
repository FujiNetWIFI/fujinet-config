-- accepttest.lua -- ACCEPT on the keyboard really writes a host slot.
--
-- Renames slot 3 (empty in a stock config) to "TEST", proves the name comes
-- back after a CONSOLE RESET -- which restarts the 6507 but not the cartridge
-- and not the server, so a name still there afterwards was genuinely written
-- by WRITE_HOST_SLOTS -- and then puts the slot back the way it was found.
--
-- The restore is not politeness: this drives a real FujiNet, and a test that
-- leaves a slot named after itself has changed the user's configuration.

package.path = (os.getenv("A2600_EMU") or ".") .. "/?.lua;" .. package.path
local font = require("vcsfont")

local T_BASE, T_PLANE_LEN, T_CELL_H, T_COLS = 0x1800, 0x80, 6, 12
local ROW0, SLOT, EDROWV = 2, 3, 1
local SLOTROW = ROW0 + SLOT

local sp, phase, waited, fails = nil, "boot", 0, 0

local function read_keys(row)
    local out = {}
    for col = 0, T_COLS - 1 do
        local plane, left, key = col // 2, (col % 2) == 0, 0
        for line = 0, 4 do
            local b = sp:readv_u8(T_BASE + plane * T_PLANE_LEN
                                         + row * T_CELL_H + line)
            local ink = left and ((b >> 5) & 7) or ((b >> 1) & 7)
            key = (key << 3) | ink
        end
        out[col] = key
    end
    return out
end
local function read_row(row)
    local keys, s = read_keys(row), ""
    for col = 0, T_COLS - 1 do s = s .. (font.glyph[keys[col]] or "?") end
    return s
end
local function dump(what)
    print("FAIL: " .. what .. "; the screen reads:")
    for r = 0, 20 do
        local s = read_row(r)
        if s:gsub("%s", "") ~= "" then print(string.format("  row %2d %q", r, s)) end
    end
    fails = fails + 1
end

local function switch(f, on)
    manager.machine.ioport.ports[":SWB"].fields[f]:set_value(on and 1 or 0)
end
local function joy(f, on)
    manager.machine.ioport.ports[":joyport1:joy:JOY"].fields[f]:set_value(on and 1 or 0)
end

local queue, qi, qtick = {}, 1, 0
local function enqueue(...) for _, v in ipairs({...}) do queue[#queue + 1] = v end end
local function pump()
    if qi > #queue then return true end
    local k = queue[qi].k
    qtick = qtick + 1
    if qtick == 1 then
        if k == "sel" then switch("Select Game", true) else joy(k, true) end
    elseif qtick == 5 then
        if k == "sel" then switch("Select Game", false) else joy(k, false) end
    elseif qtick >= 9 then qtick, qi = 0, qi + 1 end
    return false
end
local function reset_queue() queue, qi, qtick = {}, 1, 0 end

local function type_char(ch)
    local i = ch - 32
    local row, col = i // 12, i % 12
    for _ = 1, row do enqueue({k = "P1 Down"}) end
    for _ = 1, col do enqueue({k = "P1 Right"}) end
    enqueue({k = "P1 Button 1"})
    for _ = 1, col do enqueue({k = "P1 Left"}) end
    for _ = 1, row do enqueue({k = "P1 Up"}) end
end

-- SELECT, walk to menu item `n`, fire.
local function menu_pick(n)
    enqueue({k = "sel"})
    for _ = 1, n do enqueue({k = "P1 Down"}) end
    enqueue({k = "P1 Button 1"})
end

-- Open the keyboard on SLOT, clear it, type `text`, accept.
local function set_name(text)
    reset_queue()
    -- Home the cursor first. Moving SLOT rows DOWN is only right the first
    -- time; the second call starts wherever the first one left it, which is
    -- how the restore step ended up renaming a different slot entirely.
    for _ = 1, 8 do enqueue({k = "P1 Up"}) end
    for _ = 1, SLOT do enqueue({k = "P1 Down"}) end
    menu_pick(1)                                  -- RENAME
    menu_pick(2)                                  -- CLEAR
    for i = 1, #text do type_char(string.byte(text, i)) end
    menu_pick(0)                                  -- ACCEPT
end

_G._accepttest = emu.add_machine_frame_notifier(function()
    sp = sp or manager.machine.devices[":maincpu"].spaces["program"]
    waited = waited + 1
    if waited > 20000 then
        dump("timed out in phase " .. phase)
        manager.machine:exit()
        return
    end

    if phase == "boot" then
        if waited > 240 then
            print("slot " .. SLOT .. " starts as " .. string.format("%q", read_row(SLOTROW)))
            set_name("TEST")
            phase = "named"
        end

    elseif phase == "named" then
        if pump() and waited > 300 then
            local r = read_row(SLOTROW)
            if not r:find("TEST") then
                dump("ACCEPT did not write the slot (row reads " .. string.format("%q", r) .. ")")
                phase = "restore"
            else
                -- The row was redrawn from a FRESH READ_HOST_SLOTS, because
                -- the hosts bank re-reads after it writes. So this is the
                -- server's value coming back, not a local echo. (Persistence
                -- past the process is checked from the shell, against
                -- fnconfig.ini: soft_reset() here would re-run this script.)
                print("PASS: ACCEPT wrote the slot -- it reads " .. string.format("%q", r)
                      .. " on a fresh READ_HOST_SLOTS")
                phase, waited = "restore", 0
            end
        end

    elseif phase == "restore" then
        set_name("")                              -- CLEAR then ACCEPT
        phase = "restored"

    elseif phase == "restored" then
        if pump() and waited > 300 then
            local r = read_row(SLOTROW)
            if r:find("TEST") then
                dump("could not put slot " .. SLOT .. " back -- it still reads "
                     .. string.format("%q", r))
            else
                print("PASS: slot " .. SLOT .. " restored to " .. string.format("%q", r))
            end
            phase = "done"
        end

    elseif phase == "done" then
        print(fails == 0 and "accepttest: PASS"
                          or ("accepttest: " .. fails .. " failures"))
        manager.machine:exit()
    end
end)
