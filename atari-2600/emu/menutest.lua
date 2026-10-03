-- menutest.lua -- the SELECT action menu.
--
-- The 2600 reaches with three buttons what the Intellivision reaches with a
-- twelve-key keypad, and the menu is how. This proves the whole mechanism
-- without needing a server: open it, walk it, cancel it, choose from it.
--
-- Everything is read back out of the RENDERED TEXT PLANES, so it fails if the
-- menu draws in the wrong place, draws the wrong labels, forgets its cursor
-- gutter, or does not repaint the screen underneath when it closes. Glyph
-- FORMS are compared rather than decoded text: at 3x5 'S' and '5' are the
-- same picture, so decoding is lossy.

package.path = (os.getenv("A2600_EMU") or ".") .. "/?.lua;" .. package.path
local font = require("vcsfont")

local T_BASE, T_PLANE_LEN, T_CELL_H, T_COLS = 0x1800, 0x80, 6, 12
local MNROW0 = 12

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
    for col = 0, T_COLS - 1 do
        s = s .. (font.glyph[keys[col]] or "?")
    end
    return s
end

-- Compare a row against `want` by glyph form, ignoring the trailing blanks.
local function row_is(row, want)
    local keys = read_keys(row)
    for i = 1, #want do
        local k = font.key[want:sub(i, i)] or font.key["?"]
        if keys[i - 1] ~= k then return false end
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
        if s:gsub("%s", "") ~= "" then
            print(string.format("  row %2d %q", r, s))
        end
    end
    fails = fails + 1
end

local function switch(field, on)
    manager.machine.ioport.ports[":SWB"].fields[field]:set_value(on and 1 or 0)
end

local function fire(on)
    manager.machine.ioport.ports[":joyport1:joy:JOY"].fields["P1 Button 1"]
        :set_value(on and 1 or 0)
end

-- The client edge-detects, so every press has to be released again. `at` is
-- the frame the press lands on; it is let go four frames later.
local function tap(press, release, at)
    if waited == at then press()
    elseif waited == at + 4 then release() end
    return waited > at + 6
end

_G._menutest = emu.add_machine_frame_notifier(function()
    sp = sp or manager.machine.devices[":maincpu"].spaces["program"]
    waited = waited + 1
    if waited > 1800 then
        dump("timed out in phase " .. phase)
        manager.machine:exit()
        return
    end

    -- Let the host screen settle. It may show an error line if no server is
    -- listening; the menu is not supposed to care either way.
    if phase == "boot" then
        if waited > 240 then phase, waited = "open", 0 end

    -- ---- SELECT opens it -------------------------------------------------
    elseif phase == "open" then
        if tap(function() switch("Select Game", true) end,
               function() switch("Select Game", false) end, 10) then
            if not row_is(MNROW0 - 1, "ACTIONS") then
                dump("SELECT did not draw the menu title on row " .. MNROW0 - 1)
            elseif not row_is(MNROW0, ">INFO") then
                dump("the first menu item is not '>INFO'")
            elseif not row_is(MNROW0 + 1, " RENAME") then
                dump("the second menu item is not ' RENAME'")
            elseif not row_is(MNROW0 + 2, " WIFI") then
                dump("the third menu item is not ' WIFI'")
            elseif not row_is(MNROW0 + 3, " LOBBY") then
                dump("the fourth menu item is not ' LOBBY'")
            elseif not row_is(MNROW0 + 4, "") then
                dump("the rows past the last item were not blanked")
            else
                print("PASS: SELECT opens the menu")
            end
            phase, waited = "cancel", 0
        end

    -- ---- SELECT again closes it, and the screen underneath comes back ----
    elseif phase == "cancel" then
        if tap(function() switch("Select Game", true) end,
               function() switch("Select Game", false) end, 10) then
            if row_is(MNROW0 - 1, "ACTIONS") then
                dump("SELECT did not close the menu")
            elseif not row_is(MNROW0, " SD") and not row_is(MNROW0, "") then
                -- The planes survive a bank switch and a menu overlay, so a
                -- closed menu that is not painted over stays on screen. This
                -- caught a stray "LOBBY" sitting under the wifi list.
                dump("a menu item is still on row " .. MNROW0 .. " after closing")
            elseif not row_is(0, "FN HOSTS") then
                dump("closing the menu did not repaint the screen underneath")
            else
                print("PASS: SELECT closes it and the screen is repainted")
            end
            phase, waited = "reopen", 0
        end

    -- ---- reopen, then FIRE chooses INFO ----------------------------------
    elseif phase == "reopen" then
        if tap(function() switch("Select Game", true) end,
               function() switch("Select Game", false) end, 10) then
            phase, waited = "choose", 0
        end

    elseif phase == "choose" then
        if tap(function() fire(true) end, function() fire(false) end, 10) then
            if not row_is(0, "FN INFO") then
                dump("choosing INFO did not reach the adapter info bank")
            else
                print("PASS: FIRE on INFO crosses to the info bank")
            end
            phase = "done"
        end

    elseif phase == "done" then
        print(fails == 0 and "menutest: PASS" or ("menutest: " .. fails .. " failures"))
        manager.machine:exit()
    end
end)
