-- wifitest.lua -- the WiFi scan list.
--
-- Deliberately stops short of connecting: this drives a real FujiNet, and a
-- test that associates it with something else has reconfigured the user's
-- network. It scans, walks the list, and backs out with SELECT.
--
-- The claim worth proving is the one the Intellivision port learned the hard
-- way. GET_SCAN_RESULT returns ONE network per round trip, so a list that
-- redrew on every cursor step would stall for most of a second a step. Here
-- the list is drawn once and the cursor is two FN_BLIT_TCELL pokes, so this
-- also checks that moving it changes exactly two characters and disturbs
-- nothing else on the screen.

package.path = (os.getenv("A2600_EMU") or ".") .. "/?.lua;" .. package.path
local font = require("vcsfont")

local T_BASE, T_PLANE_LEN, T_CELL_H, T_COLS = 0x1800, 0x80, 6, 12
local WSROW0 = 2

local sp, phase, waited, fails = nil, "boot", 0, 0
local snapshot = nil

local function keys(row)
    local out = {}
    for col = 0, T_COLS - 1 do
        local plane, left, k = col // 2, (col % 2) == 0, 0
        for line = 0, 4 do
            local b = sp:readv_u8(T_BASE + plane*T_PLANE_LEN + row*T_CELL_H + line)
            local ink = left and ((b >> 5) & 7) or ((b >> 1) & 7)
            k = (k << 3) | ink
        end
        out[col] = k
    end
    return out
end
local function read_row(row)
    local kk, s = keys(row), ""
    for c = 0, T_COLS - 1 do s = s .. (font.glyph[kk[c]] or "?") end
    return s
end
local function screen()
    local t = {}
    for r = 0, 20 do t[r] = read_row(r) end
    return t
end
local function dump(what)
    print("FAIL: " .. what .. "; the screen reads:")
    for r = 0, 20 do
        local s = read_row(r)
        if s:gsub("%s", "") ~= "" then print(string.format("  row %2d %q", r, s)) end
    end
    fails = fails + 1
end

local function switch(f,on) manager.machine.ioport.ports[":SWB"].fields[f]:set_value(on and 1 or 0) end
local function joy(f,on) manager.machine.ioport.ports[":joyport1:joy:JOY"].fields[f]:set_value(on and 1 or 0) end
local q, qi, qt = {}, 1, 0
local function enq(...) for _,v in ipairs({...}) do q[#q+1]=v end end
local function reset_q() q, qi, qt = {}, 1, 0 end
local function pump()
    if qi > #q then return true end
    local k = q[qi].k
    qt = qt + 1
    if qt == 1 then if k=="sel" then switch("Select Game",true) else joy(k,true) end
    elseif qt == 5 then if k=="sel" then switch("Select Game",false) else joy(k,false) end
    elseif qt >= 9 then qt, qi = 0, qi+1 end
    return false
end

-- Which row carries the cursor gutter?
local function cursor_row()
    for r = WSROW0, 18 do
        if keys(r)[0] == font.key[">"] then return r end
    end
    return nil
end

_G._wifitest = emu.add_machine_frame_notifier(function()
    sp = sp or manager.machine.devices[":maincpu"].spaces["program"]
    waited = waited + 1
    if waited > 8000 then
        dump("timed out in phase " .. phase)
        manager.machine:exit()
        return
    end

    if phase == "boot" then
        if waited > 300 then
            reset_q()
            enq({k="sel"}, {k="P1 Down"}, {k="P1 Down"}, {k="P1 Button 1"})  -- WIFI
            phase = "toscan"
        end

    elseif phase == "toscan" then
        if pump() and waited > 600 then
            if not read_row(0):find("PICK NET") then
                dump("the WIFI menu item did not reach the scan list")
                phase = "done"
            else
                local n = 0
                for r = WSROW0, 18 do
                    if read_row(r):gsub("%s","") ~= "" then n = n + 1 end
                end
                print("PASS: scan finished; " .. n .. " rows listed, "
                      .. "cursor on row " .. tostring(cursor_row()))
                print("      row " .. WSROW0 .. " = " .. string.format("%q", read_row(WSROW0)))
                snapshot = screen()
                reset_q(); enq({k="P1 Down"})
                phase = "moved"
            end
        end

    elseif phase == "moved" then
        if pump() then
            local now, diffs = screen(), {}
            for r = 0, 20 do
                if now[r] ~= snapshot[r] then diffs[#diffs+1] = r end
            end
            local cr = cursor_row()
            if cr ~= WSROW0 + 1 then
                dump("DOWN did not move the cursor to row " .. (WSROW0+1)
                     .. " (it is on " .. tostring(cr) .. ")")
            elseif #diffs ~= 2 then
                dump("moving the cursor changed " .. #diffs .. " rows, not 2")
            else
                print("PASS: the cursor moved and exactly two rows changed ("
                      .. diffs[1] .. ", " .. diffs[2] .. ") -- no redraw, no round trip")
            end
            -- The last row must be OTHER..., so a hidden network is reachable.
            local last = nil
            for r = 18, WSROW0, -1 do
                if read_row(r):gsub("%s","") ~= "" then last = r break end
            end
            if last and read_row(last):find("OTHER") then
                print("PASS: the list ends with the OTHER row (row " .. last .. ")")
            else
                dump("the list does not end with an OTHER row")
            end
            reset_q(); enq({k="sel"})              -- SKIP, without connecting
            phase = "skipped"
        end

    elseif phase == "skipped" then
        if pump() and waited > 200 then
            if not read_row(0):find("FN HOSTS") then
                dump("SELECT did not skip back to the hosts screen")
            else
                print("PASS: SELECT skips wifi setup and returns to the hosts")
            end
            phase = "done"
        end

    elseif phase == "done" then
        print(fails == 0 and "wifitest: PASS" or ("wifitest: " .. fails .. " failures"))
        manager.machine:exit()
    end
end)
