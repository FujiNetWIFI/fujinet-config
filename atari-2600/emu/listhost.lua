-- listhost.lua -- mount host SLOT and print its root listing. A probe, not a test.
package.path = (os.getenv("A2600_EMU") or ".") .. "/?.lua;" .. package.path
local font = require("vcsfont")
local T_BASE, T_PLANE_LEN, T_CELL_H, T_COLS = 0x1800, 0x80, 6, 12
local ROW0, NROWS = 2, 14
local SLOT = tonumber(os.getenv("LIST_SLOT") or "7")
local DESCEND = os.getenv("LIST_DIR")
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
local function show(what)
    print("--- " .. what .. " (path row: " .. string.format("%q", read_row(1)) .. ") ---")
    for r = ROW0, ROW0 + NROWS - 1 do
        local s = read_row(r):gsub("%s+$", "")
        if s ~= "" then print("  " .. s) end
    end
    local e = read_row(18):gsub("%s+$","")
    if e ~= "" then print("  [row 18] " .. e) end
end
local function joy(f,on) manager.machine.ioport.ports[":joyport1:joy:JOY"].fields[f]:set_value(on and 1 or 0) end
local q, qi, qt = {}, 1, 0
local function enq(...) for _,v in ipairs({...}) do q[#q+1]=v end end
local function reset_q() q, qi, qt = {}, 1, 0 end
local function pump()
    if qi > #q then return true end
    qt = qt + 1
    if qt == 1 then joy(q[qi].k, true)
    elseif qt == 5 then joy(q[qi].k, false)
    elseif qt >= 9 then qt, qi = 0, qi+1 end
    return false
end

_G._listhost = emu.add_machine_frame_notifier(function()
    sp = sp or manager.machine.devices[":maincpu"].spaces["program"]
    waited = waited + 1
    if waited > 20000 then print("timed out in " .. phase) manager.machine:exit() return end

    if phase == "boot" then
        if waited > 400 then
            reset_q()
            for _ = 1, SLOT do enq({k="P1 Down"}) end
            enq({k="P1 Button 1"})
            phase, waited = "root", 0
        end
    elseif phase == "root" then
        if pump() and waited > 1500 then
            show("host slot " .. SLOT .. " root")
            if DESCEND then
                local target = nil
                for r = ROW0, ROW0 + NROWS - 1 do
                    if read_row(r):sub(2):gsub("%s+$",""):upper() == DESCEND:upper() then target = r end
                end
                if target then
                    reset_q()
                    for _ = ROW0, target - 1 do enq({k="P1 Down"}) end
                    enq({k="P1 Button 1"})
                    phase, waited = "sub", 0
                else
                    print("(" .. DESCEND .. " not found)")
                    manager.machine:exit()
                end
            else
                manager.machine:exit()
            end
        end
    elseif phase == "sub" then
        if pump() and waited > 1500 then
            show(DESCEND)
            manager.machine:exit()
        end
    end
end)
