-- setslot.lua -- set host slot $SET_SLOT to $SET_NAME. A tool, not a test.
package.path = (os.getenv("A2600_EMU") or ".") .. "/?.lua;" .. package.path
local font = require("vcsfont")
local T_BASE, T_PLANE_LEN, T_CELL_H, T_COLS = 0x1800, 0x80, 6, 12
local ROW0 = 2
local SLOT = tonumber(os.getenv("SET_SLOT") or "7")
local NAME = os.getenv("SET_NAME") or ""
local SLOTROW = ROW0 + SLOT
local sp, phase, waited = nil, "boot", 0

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
local function joy(f,on) manager.machine.ioport.ports[":joyport1:joy:JOY"].fields[f]:set_value(on and 1 or 0) end
local function switch(f,on) manager.machine.ioport.ports[":SWB"].fields[f]:set_value(on and 1 or 0) end
local q, qi, qt = {}, 1, 0
local function enq(...) for _,v in ipairs({...}) do q[#q+1]=v end end
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

_G._setslot = emu.add_machine_frame_notifier(function()
    sp = sp or manager.machine.devices[":maincpu"].spaces["program"]
    waited = waited + 1
    if waited > 30000 then print("timed out in " .. phase) manager.machine:exit() return end

    if phase == "boot" then
        if waited > 400 then
            print("slot " .. SLOT .. " was " .. string.format("%q", read_row(SLOTROW)))
            for _ = 1, 8 do enq({k="P1 Up"}) end
            for _ = 1, SLOT do enq({k="P1 Down"}) end
            menu_pick(1)                      -- RENAME
            menu_pick(2)                      -- CLEAR
            for i = 1, #NAME do type_char(string.byte(NAME, i)) end
            menu_pick(0)                      -- ACCEPT
            phase, waited = "set", 0
        end
    elseif phase == "set" then
        if pump() and waited > 600 then
            print("slot " .. SLOT .. " now " .. string.format("%q", read_row(SLOTROW)))
            manager.machine:exit()
        end
    end
end)
