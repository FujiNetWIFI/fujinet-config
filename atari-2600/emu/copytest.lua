-- copytest.lua -- host-to-host copy, end to end.
--
-- Marks SOAK/soak2k.bin, walks up to the root, and copies it there. The proof
-- is that the file appears in a FRESH listing of the destination, found by
-- glyph form -- not that COPY_FILE returned ACK, which it would also do if the
-- payload named the wrong file.
--
-- This writes to the SD card, so the shell deletes the copy afterwards; the
-- test prints the path it created.

package.path = (os.getenv("A2600_EMU") or ".") .. "/?.lua;" .. package.path
local font = require("vcsfont")

local T_BASE, T_PLANE_LEN, T_CELL_H, T_COLS = 0x1800, 0x80, 6, 12
local ROW0, NROWS, MNROW0 = 2, 14, 12
local WANT = "SOAK2K.BIN"

local sp, phase, waited, fails = nil, "boot", 0, 0

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
local function read_row(r)
    local kk, s = keys(r), ""
    for c = 0, T_COLS - 1 do s = s .. (font.glyph[kk[c]] or "?") end
    return s
end
local function dump(what)
    print("FAIL: " .. what .. "; the screen reads:")
    for r = 0, 20 do
        local s = read_row(r)
        if s:gsub("%s","") ~= "" then print(string.format("  row %2d %q", r, s)) end
    end
    fails = fails + 1
end
local function find(name)
    for r = ROW0, ROW0 + NROWS - 1 do
        if read_row(r):sub(2):gsub("%s+$","") == name then return r end
    end
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
local function menu_pick(n)
    enq({k="sel"})
    for _ = 1, n do enq({k="P1 Down"}) end
    enq({k="P1 Button 1"})
end

_G._copytest = emu.add_machine_frame_notifier(function()
    sp = sp or manager.machine.devices[":maincpu"].spaces["program"]
    waited = waited + 1
    if waited > 30000 then dump("timed out in phase " .. phase) manager.machine:exit() return end

    if phase == "boot" then
        if waited > 300 then
            reset_q(); enq({k="P1 Button 1"})           -- mount host 0
            phase = "root"
        end

    elseif phase == "root" then
        if pump() and waited > 700 then
            local r = find("SOAK/")
            if not r then dump("SOAK/ is not in the root listing") phase = "done"
            else
                reset_q()
                for _ = ROW0, r - 1 do enq({k="P1 Down"}) end
                enq({k="P1 Button 1"})
                phase, waited = "insoak", 0
            end
        end

    elseif phase == "insoak" then
        if pump() and waited > 900 then
            local r = find(WANT:lower()) or find(WANT)
            if not r then dump(WANT .. " is not in SOAK/") phase = "done"
            else
                reset_q()
                for _ = ROW0, r - 1 do enq({k="P1 Down"}) end
                menu_pick(3)                            -- COPY: mark the source
                phase, waited = "marked", 0
            end
        end

    elseif phase == "marked" then
        if pump() and waited > 700 then
            if not read_row(0):find("COPY TO") then
                dump("marking a source did not put the hosts screen in copy livery")
                phase = "done"
            else
                print("PASS: the source is marked and the hosts screen says COPY TO")
                reset_q(); enq({k="P1 Button 1"})       -- destination host 0 root
                phase, waited = "dest", 0
            end
        end

    elseif phase == "dest" then
        if pump() and waited > 900 then
            local pr = read_row(1)
            print("      destination directory is " .. string.format("%q", pr))
            reset_q(); menu_pick(4)                     -- COPY HERE
            phase, waited = "copying", 0
        end

    elseif phase == "copying" then
        -- Catch the copy screen while it is up. It reports its result and
        -- then returns to the browser by itself after about three seconds, so
        -- waiting a fixed number of frames sails straight past it -- which is
        -- how this test first "passed" while reading a browser row.
        if pump() then
            if read_row(0):find("COPYING") then
                local msg = read_row(8)
                if msg:gsub("%s","") ~= "" then
                    if msg:find("FAILED") then
                        dump("the copy reported failure: " .. string.format("%q", msg))
                        phase = "done"
                    else
                        print("PASS: the copy screen reports " .. string.format("%q", msg))
                        phase, waited = "verify", 0
                    end
                end
            elseif waited > 1500 then
                dump("COPY HERE never reached the copy screen")
                phase = "done"
            end
        end

    elseif phase == "verify" then
        -- No button press here: the copy screen returns to the browser on its
        -- own, and the browser re-lists on the way in, so this listing comes
        -- from the server.
        if waited > 1500 then
            -- A fresh listing of the destination. The browser re-lists on the
            -- way back in, so this is the server's directory, not a cached one.
            local r = find(WANT:lower()) or find(WANT)
            if r then
                print("PASS: " .. WANT .. " is in the destination listing at row " .. r)
            else
                dump(WANT .. " is not in the destination listing")
            end
            phase = "done"
        end

    elseif phase == "done" then
        print(fails == 0 and "copytest: PASS" or ("copytest: " .. fails .. " failures"))
        manager.machine:exit()
    end
end)
