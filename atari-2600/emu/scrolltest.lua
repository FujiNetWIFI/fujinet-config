-- scrolltest.lua -- a name too long for the row bounces through it.
--
-- SOAK/ has names of 12 characters (soakf4sc.bin) beside names of 9, and a row
-- shows 11. Rest on a long one: the row must slide (its text, less the first
-- character, appears), a keypress must put it back as listed, and a short name
-- must never move. Needs the same SOAK/ as pagetest on host slot 0.

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


local cur = ROW0                       -- the cursor's row, as far as we have driven it
local function move_to(r)
    for _ = cur, r - 1 do enq({k="P1 Down"}) end
    for _ = r, cur - 1 do enq({k="P1 Up"}) end
    cur = r
end
local function name_at(r) return (read_row(r):sub(2):gsub("%s+$", "")) end
local function rows_where(long)
    for r = ROW0, ROW0 + NROWS - 1 do
        local n = name_at(r)
        if n ~= "" and ((#n >= T_COLS - 1) == long) then return r end
    end
end
local init, seen, shifted_row

_G._scrolltest = emu.add_machine_frame_notifier(function()
    sp = sp or manager.machine.devices[":maincpu"].spaces["program"]
    waited = waited + 1
    if waited > 20000 then dump("timed out in phase " .. phase) manager.machine:exit() return end

    if phase == "boot" then
        if waited > 300 then
            reset_q(); enq({k="P1 Button 1"})
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
            cur = ROW0
            local s = rows_where(false)
            if not s then dump("no short name in SOAK/") phase = "done" return end
            reset_q(); move_to(s)
            phase, waited, snap = "toshort", 0, nil
        end

    elseif phase == "toshort" then
        if pump() then
            snap = snap or screen()
            if waited > 400 then
                local now, diffs = screen(), 0
                for r = 0, 20 do if now[r] ~= snap[r] then diffs = diffs + 1 end end
                if diffs ~= 0 then
                    dump("resting on a short name changed " .. diffs .. " rows")
                else
                    print("PASS: a short name does not move")
                end
                local l = rows_where(true)
                if not l then dump("no long name in SOAK/") phase = "done" return end
                reset_q(); move_to(l)
                shifted_row, phase, waited, init, seen = l, "tolong", 0, nil, false
            end
        end

    elseif phase == "tolong" then
        if pump() then
            if not init then init = read_row(shifted_row) end
            local now = read_row(shifted_row)
            if now ~= init and now:sub(1, 1) == ">"
               and now:sub(2, 11) == init:sub(3, 12) then
                seen = true
            end
            if waited > 500 then
                if seen then
                    print("PASS: the long name slid one character along its row")
                else
                    dump("the long name never scrolled (row reads " .. string.format("%q", now) .. ")")
                end
                reset_q(); enq({k="P1 Down"}); cur = cur + 1
                phase, waited = "away", 0
            end
        end

    elseif phase == "away" then
        if pump() then
            local back = read_row(shifted_row):sub(2)
            if back == init:sub(2) then
                print("PASS: moving off a scrolled name puts it back as listed")
            else
                dump("the scrolled row was not restored (reads " .. string.format("%q", back) .. ")")
            end
            phase = "done"
        end

    elseif phase == "done" then
        print(fails == 0 and "scrolltest: PASS" or ("scrolltest: " .. fails .. " failures"))
        manager.machine:exit()
    end
end)
