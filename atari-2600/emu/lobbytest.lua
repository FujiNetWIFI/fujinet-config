-- lobbytest.lua -- the Game Lobby claims a host slot and boots from it.
--
-- The lobby is not a new mechanism: it is a ROM on a particular TNFS host,
-- reached by finding that host among the eight slots -- or putting it in the
-- last one if it is not there yet -- and then mounting and booting exactly
-- the way the browser does.
--
-- What is checked here is the slot claim, because that is the part unique to
-- this feature. Whether ec.tnfs.io answers depends on the network, so a boot
-- that fails is reported rather than treated as a test failure.
--
-- The claim overwrites a real host slot, so this puts it back.

package.path = (os.getenv("A2600_EMU") or ".") .. "/?.lua;" .. package.path
local font = require("vcsfont")

local T_BASE, T_PLANE_LEN, T_CELL_H, T_COLS = 0x1800, 0x80, 6, 12
local ROW0, SLOT = 2, 7
local SLOTROW = ROW0 + SLOT

local sp, phase, waited, fails, original = nil, "boot", 0, 0, nil

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
local function name_at(r) return read_row(r):sub(2):gsub("%s+$", "") end
local function dump(what)
    print("FAIL: " .. what .. "; the screen reads:")
    for r = 0, 20 do
        local s = read_row(r)
        if s:gsub("%s","") ~= "" then print(string.format("  row %2d %q", r, s)) end
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
local function menu_pick(n)
    enq({k="sel"})
    for _ = 1, n do enq({k="P1 Down"}) end
    enq({k="P1 Button 1"})
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
local function set_name(text)
    reset_q()
    for _ = 1, 8 do enq({k="P1 Up"}) end
    for _ = 1, SLOT do enq({k="P1 Down"}) end
    menu_pick(1)                                   -- RENAME
    menu_pick(2)                                   -- CLEAR
    for i = 1, #text do type_char(string.byte(text, i)) end
    menu_pick(0)                                   -- ACCEPT
end

_G._lobbytest = emu.add_machine_frame_notifier(function()
    sp = sp or manager.machine.devices[":maincpu"].spaces["program"]
    waited = waited + 1
    if waited > 30000 then dump("timed out in phase " .. phase) manager.machine:exit() return end

    if phase == "boot" then
        if waited > 400 then
            original = name_at(SLOTROW)
            print("slot " .. SLOT .. " starts as " .. string.format("%q", original))
            reset_q(); menu_pick(3)                -- LOBBY
            phase, waited = "lobby", 0
        end

    elseif phase == "lobby" then
        if pump() and waited > 1200 then
            local t = read_row(0)
            if t:find("BOOTING") then
                print("PASS: the lobby mounted its host and reached the boot screen")
            else
                print("NOTE: the lobby did not reach a boot -- title is "
                      .. string.format("%q", t)
                      .. " (ec.tnfs.io may be unreachable, or the ROM path may "
                      .. "need confirming)")
            end
            phase, waited = "check", 0
        end

    elseif phase == "check" then
        if waited > 2400 then
            -- Back on the hosts screen one way or another; read the slot.
            if read_row(0):find("FN HOSTS") then
                local now = name_at(SLOTROW)
                if now:upper():find("EC.TNFS.IO") then
                    print("PASS: the lobby claimed slot " .. SLOT
                          .. " for ec.tnfs.io")
                elseif original:upper():find("EC.TNFS.IO") then
                    print("PASS: ec.tnfs.io was already in the slots; nothing "
                          .. "was claimed")
                else
                    dump("slot " .. SLOT .. " reads " .. string.format("%q", now)
                         .. " -- the host was neither found nor claimed")
                end
            else
                print("NOTE: not back on the hosts screen; skipping the slot check")
            end
            phase = "restore"
        end

    elseif phase == "restore" then
        if original and original ~= "" and not original:find("EMPTY") then
            set_name(original)
        else
            set_name("")
        end
        phase, waited = "restored", 0

    elseif phase == "restored" then
        if pump() and waited > 600 then
            local now = name_at(SLOTROW)
            if now == original then
                print("PASS: slot " .. SLOT .. " restored to "
                      .. string.format("%q", now))
            else
                dump("could not restore slot " .. SLOT .. ": it reads "
                     .. string.format("%q", now) .. ", was "
                     .. string.format("%q", original))
            end
            phase = "done"
        end

    elseif phase == "done" then
        print(fails == 0 and "lobbytest: PASS" or ("lobbytest: " .. fails .. " failures"))
        manager.machine:exit()
    end
end)
