-- pagetest.lua -- the browser's parity features, in one directory.
--
-- SOAK/ holds .bin files each with a .cfg sibling, so it exercises the three
-- things at once: the .cfg suppression (a pair is ONE item to a person, and
-- listing both doubles every page), the path row (which could not be drawn at
-- all before the cartridge learned to render a buffer), and the filter.
--
-- Also checks that moving the cursor costs nothing: before this the browser
-- re-listed the whole directory on every step, which on real hardware is
-- about a second a press.

package.path = (os.getenv("A2600_EMU") or ".") .. "/?.lua;" .. package.path
local font = require("vcsfont")

local T_BASE, T_PLANE_LEN, T_CELL_H, T_COLS = 0x1800, 0x80, 6, 12
local ROW0, NROWS, PATHROW = 2, 14, 1

local sp, phase, waited, fails, snap = nil, "boot", 0, 0, nil

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
local function screen() local t={} for r=0,20 do t[r]=read_row(r) end return t end
local function dump(what)
    print("FAIL: " .. what .. "; the screen reads:")
    for r = 0, 20 do
        local s = read_row(r)
        if s:gsub("%s","") ~= "" then print(string.format("  row %2d %q", r, s)) end
    end
    fails = fails + 1
end
local function list()
    local out = {}
    for r = ROW0, ROW0 + NROWS - 1 do
        local s = read_row(r):sub(2):gsub("%s+$", "")
        if s ~= "" then out[#out+1] = s end
    end
    return out
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
local function type_char(ch)
    local i = ch - 32
    local row, col = i // 12, i % 12
    for _ = 1, row do enq({k="P1 Down"}) end
    for _ = 1, col do enq({k="P1 Right"}) end
    enq({k="P1 Button 1"})
    for _ = 1, col do enq({k="P1 Left"}) end
    for _ = 1, row do enq({k="P1 Up"}) end
end
local function menu_pick(n)
    enq({k="sel"})
    for _ = 1, n do enq({k="P1 Down"}) end
    enq({k="P1 Button 1"})
end

_G._pagetest = emu.add_machine_frame_notifier(function()
    sp = sp or manager.machine.devices[":maincpu"].spaces["program"]
    waited = waited + 1
    if waited > 20000 then dump("timed out in phase " .. phase) manager.machine:exit() return end

    if phase == "boot" then
        if waited > 300 then
            reset_q(); enq({k="P1 Button 1"})       -- mount host 0
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
        if pump() and waited > 700 then
            local items = list()
            local ncfg = 0
            for _, v in ipairs(items) do
                if v:upper():sub(-4) == ".CFG" then ncfg = ncfg + 1 end
            end
            if #items == 0 then
                dump("SOAK/ listed nothing")
            elseif ncfg > 0 then
                dump(ncfg .. " .cfg siblings were listed; they must be suppressed")
            else
                print("PASS: " .. #items .. " entries listed and not one .cfg sibling")
            end
            local pr = read_row(PATHROW)
            if pr:find("SOAK") then
                print("PASS: the path row shows where we are: " .. string.format("%q", pr))
            else
                dump("the path row does not show SOAK (it reads " .. string.format("%q", pr) .. ")")
            end
            snap = screen()
            reset_q(); enq({k="P1 Down"})
            phase = "moved"
        end

    elseif phase == "moved" then
        if pump() then
            local now, diffs = screen(), {}
            for r = 0, 20 do if now[r] ~= snap[r] then diffs[#diffs+1] = r end end
            if #diffs ~= 2 then
                dump("moving the cursor changed " .. #diffs .. " rows, not 2 -- it re-listed")
            else
                print("PASS: the cursor moved and only two rows changed -- no re-listing")
            end
            -- Filter to *F6* -- SOAK/ holds soakf6.bin and soakf6sc.bin and
            -- eleven others, so a working filter leaves exactly two. "Zero
            -- entries" would be indistinguishable from a filter that never
            -- reached the server, which is why the pattern matches SOME.
            reset_q()
            menu_pick(1)                              -- FILTER
            for _, c in ipairs({42, 70, 54, 42}) do type_char(c) end   -- *F6*
            menu_pick(0)                              -- ACCEPT
            phase, waited = "filtered", 0
        end

    elseif phase == "filtered" then
        if pump() and waited > 900 then
            local items, bad = list(), 0
            for _, v in ipairs(items) do
                if not v:upper():find("F6") then bad = bad + 1 end
            end
            if #items == 0 then
                dump("the *F6* filter matched nothing; it should match two")
            elseif bad > 0 then
                dump(bad .. " entries survived a *F6* filter that should not have")
            else
                print("PASS: the *F6* filter narrowed 13 entries to " .. #items
                      .. ", all matching")
            end
            -- Clearing it must bring everything back, or a filter would be a
            -- one-way door.
            reset_q()
            menu_pick(1)                              -- FILTER
            menu_pick(2)                              -- CLEAR
            menu_pick(0)                              -- ACCEPT
            phase, waited = "cleared", 0
        end

    elseif phase == "cleared" then
        if pump() and waited > 900 then
            local items = list()
            if #items < 13 then
                dump("clearing the filter did not bring the listing back (got "
                     .. #items .. " of 13)")
            else
                print("PASS: clearing the filter restores all " .. #items .. " entries")
            end
            phase = "done"
        end

    elseif phase == "done" then
        print(fails == 0 and "pagetest: PASS" or ("pagetest: " .. fails .. " failures"))
        manager.machine:exit()
    end
end)
