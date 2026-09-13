-- edittest.lua -- the on-screen keyboard, and where the string it builds lives.
--
-- The console has about forty bytes of RAM free and a passphrase is 63
-- characters, so the text being typed is held in the CARTRIDGE, in path
-- buffer 3, and painted back with FN_BLIT_PATH. This drives the keyboard
-- through MAME and reads the value row out of the rendered planes: if the
-- characters appear there, they came from the cartridge, because there is
-- nowhere in the console they could have been kept.
--
-- Covers typing, the backspace cell, backspacing past empty, and -- the one
-- that matters most -- that CANCEL leaves the original name alone.

package.path = (os.getenv("A2600_EMU") or ".") .. "/?.lua;" .. package.path
local font = require("vcsfont")

local T_BASE, T_PLANE_LEN, T_CELL_H, T_COLS = 0x1800, 0x80, 6, 12
local EDROWV, EDROW0, MNROW0 = 1, 4, 12

local sp, phase, waited, fails, step = nil, "boot", 0, 0, 0

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
    for col = 0, T_COLS - 1 do
        s = s .. (font.glyph[keys[col]] or "?")
    end
    return s
end

-- Compare by glyph FORM, not decoded text: at 3x5 'S' and '5' are one
-- picture, so a decoded comparison would be lossy on real names.
local function row_is(row, want)
    local keys = read_keys(row)
    for i = 1, #want do
        if keys[i - 1] ~= (font.key[want:sub(i, i)] or font.key["?"]) then
            return false
        end
    end
    for c = #want, T_COLS - 1 do
        if keys[c] ~= font.key[" "] then return false end
    end
    return true
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

-- A queue of presses. Everything here edge-detects, so each one is held for a
-- few frames and then released with a gap before the next.
local queue, qi, qtick = {}, 1, 0
local function enqueue(...) for _, v in ipairs({...}) do queue[#queue + 1] = v end end
local function pump()
    if qi > #queue then return true end
    local it = queue[qi]
    qtick = qtick + 1
    if qtick == 1 then
        if it.k == "sel" then switch("Select Game", true) else joy(it.k, true) end
    elseif qtick == 5 then
        if it.k == "sel" then switch("Select Game", false) else joy(it.k, false) end
    elseif qtick >= 9 then
        qtick, qi = 0, qi + 1
    end
    return false
end
local function reset_queue() queue, qi, qtick = {}, 1, 0 end

-- Walk the grid cursor to the cell holding `ch` and fire. ch = 32+row*12+col.
local function type_char(ch)
    local i = ch - 32
    local row, col = i // 12, i % 12
    for _ = 1, row do enqueue({k = "P1 Down"}) end
    for _ = 1, col do enqueue({k = "P1 Right"}) end
    enqueue({k = "P1 Button 1"})
    for _ = 1, col do enqueue({k = "P1 Left"}) end
    for _ = 1, row do enqueue({k = "P1 Up"}) end
end

_G._edittest = emu.add_machine_frame_notifier(function()
    sp = sp or manager.machine.devices[":maincpu"].spaces["program"]
    waited = waited + 1
    if waited > 12000 then
        dump("timed out in phase " .. phase)
        manager.machine:exit()
        return
    end

    if phase == "boot" then
        if waited > 240 then
            -- SELECT, down to RENAME, fire.
            reset_queue()
            enqueue({k = "sel"}, {k = "P1 Down"}, {k = "P1 Button 1"})
            phase = "toedit"
        end

    elseif phase == "toedit" then
        if pump() then
            if not row_is(0, "HOST NAME") then
                dump("RENAME did not reach the keyboard")
                phase = "done"
            else
                print("PASS: RENAME opens the keyboard on the slot's name")
                print("      value row starts as " .. string.format("%q", read_row(EDROWV)))
                reset_queue()
                type_char(string.byte("A"))
                type_char(string.byte("B"))
                phase, step = "typed", 0
            end
        end

    elseif phase == "typed" then
        if pump() then
            local v = read_row(EDROWV)
            if not v:find("AB") then
                dump("typing A then B did not reach the value row (got " .. string.format("%q", v) .. ")")
            else
                print("PASS: typed characters appear -- they can only have come "
                      .. "from the cartridge")
            end
            reset_queue()
            type_char(0x7F)                       -- the backspace cell
            phase = "backspaced"
        end

    elseif phase == "backspaced" then
        if pump() then
            local v = read_row(EDROWV)
            if v:find("AB") then
                dump("the backspace cell did not remove a character")
            else
                print("PASS: the backspace cell removes one character")
            end
            -- Now empty it completely and keep going: bottoming out must be
            -- ordinary, not an underflow into a 65535-byte payload.
            reset_queue()
            for _ = 1, 20 do type_char(0x7F) end
            phase = "emptied"
        end

    elseif phase == "emptied" then
        if pump() then
            if not row_is(EDROWV, "") then
                dump("backspacing past empty left something on the value row")
            else
                print("PASS: backspacing past empty bottoms out cleanly")
            end
            -- Type a mark, then CANCEL, and check the host list is unchanged.
            reset_queue()
            type_char(string.byte("Z"))
            enqueue({k = "sel"}, {k = "P1 Down"}, {k = "P1 Button 1"})  -- CANCEL
            phase = "cancelled"
        end

    elseif phase == "cancelled" then
        if pump() then
            if not row_is(0, "FN HOSTS") then
                dump("CANCEL did not return to the host list")
            else
                local r1 = read_row(2)
                if r1:find("Z") then
                    dump("CANCEL wrote the edit through anyway: row 2 is "
                         .. string.format("%q", r1))
                else
                    print("PASS: CANCEL returns and leaves the slot untouched ("
                          .. string.format("%q", r1) .. ")")
                end
            end
            phase = "done"
        end

    elseif phase == "done" then
        print(fails == 0 and "edittest: PASS"
                          or ("edittest: " .. fails .. " failures"))
        manager.machine:exit()
    end
end)
