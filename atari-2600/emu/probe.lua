-- probe.lua -- where is the keyboard's caret, really?
package.path = (os.getenv("A2600_EMU") or ".") .. "/?.lua;" .. package.path
local font = require("vcsfont")
local T_BASE, T_PLANE_LEN, T_CELL_H, T_COLS = 0x1800, 0x80, 6, 12
local EDROW0 = 4
local sp, waited, phase, n = nil, 0, "boot", 0

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
local function caret()
    for r = 0, 7 do
        local kk = keys(EDROW0 + r*2 + 1)
        for c = 0, T_COLS - 1 do
            if kk[c] == font.key["^"] then return r, c end
        end
    end
    return nil, nil
end
local function switch(f,on) manager.machine.ioport.ports[":SWB"].fields[f]:set_value(on and 1 or 0) end
local function joy(f,on) manager.machine.ioport.ports[":joyport1:joy:JOY"].fields[f]:set_value(on and 1 or 0) end

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

_G._probe = emu.add_machine_frame_notifier(function()
    sp = sp or manager.machine.devices[":maincpu"].spaces["program"]
    waited = waited + 1
    if waited > 4000 then manager.machine:exit() return end

    if phase == "boot" then
        if waited > 240 then
            -- exactly what accepttest's set_name() does before it types
            for _ = 1, 3 do enq({k="P1 Down"}) end             -- to slot 3
            enq({k="sel"},{k="P1 Down"},{k="P1 Button 1"})     -- RENAME
            enq({k="sel"},{k="P1 Down"},{k="P1 Down"},{k="P1 Button 1"}) -- CLEAR
            phase = "toedit"
        end
    elseif phase == "toedit" then
        if pump() then
            local r,c = caret()
            print(string.format("after the set_name prefix: caret row=%s col=%s", tostring(r), tostring(c)))
            q, qi, qt = {}, 1, 0
            enq({k="P1 Right"})
            phase = "step"
        end
    elseif phase == "step" then
        if pump() then
            n = n + 1
            local r,c = caret()
            print(string.format("after %d Right: caret row=%s col=%s", n, tostring(r), tostring(c)))
            if n >= 4 then phase = "downs"; q, qi, qt = {}, 1, 0; enq({k="P1 Down"}); n = 0
            else q, qi, qt = {}, 1, 0; enq({k="P1 Right"}) end
        end
    elseif phase == "downs" then
        if pump() then
            n = n + 1
            local r,c = caret()
            print(string.format("after %d Down: caret row=%s col=%s", n, tostring(r), tostring(c)))
            if n >= 3 then manager.machine:exit()
            else q, qi, qt = {}, 1, 0; enq({k="P1 Down"}) end
        end
    end
end)
