-- cfgreset.lua -- CONFIG survives the RESET switch.
--
-- This connector has no reset line, so the RESET switch restarts the 6507 but
-- NOT the cartridge: the mailbox, the sequence number and the four path
-- buffers all carry on. A client that kept its own sequence counter in RAM
-- would restart it at 1 and every transaction afterwards would be ignored as
-- a replay, so FNGO derives the sequence from the cart's own ACKSEQ instead.
--
-- The firmware tree's resettest.lua asserts ACKSEQ is exactly before+1, which
-- is right for a client that issues one transaction at startup. CONFIG issues
-- several -- the wifi bank's decision tree runs before anything is drawn --
-- so what is checked here is the property rather than the count: the sequence
-- CONTINUED from where the cartridge had it, and did not restart.

local FN_ACKSEQ, FN_MAG0, FN_MAG1 = 0x1F00, 0x1F09, 0x1F0A
local sp, waited = nil, 0

-- MAME re-executes an autoboot script on a soft reset, so the state has to
-- live in _G or the second run starts over and resets forever. Same reason
-- and same shape as the firmware tree's resettest.lua.
if _G._cfgreset_state == nil then _G._cfgreset_state = { phase = "first" } end
local st = _G._cfgreset_state

local function magic()
    return sp:readv_u8(FN_MAG0) == 0x46 and sp:readv_u8(FN_MAG1) == 0x4E
end

_G._cfgreset = emu.add_machine_frame_notifier(function()
    sp = sp or manager.machine.devices[":maincpu"].spaces["program"]
    if st.done then return end
    waited = waited + 1
    if waited > 3000 then
        print("FAIL: timed out in phase " .. st.phase)
        manager.machine:exit()
        return
    end

    local seq = sp:readv_u8(FN_ACKSEQ)

    if st.phase == "first" then
        -- Let the cold decision tree finish so the sequence has moved on.
        if magic() and seq ~= 0 and waited > 400 then
            st.before = seq
            print(string.format("before reset: ACKSEQ=%02X", st.before))
            st.phase = "second"
            manager.machine:soft_reset()
            waited = 0
        end

    elseif st.phase == "second" then
        if magic() and seq ~= st.before and waited > 400 then
            -- Anything other than a restart at 1. The sequence skips 0, so
            -- "1" after a reset from a nonzero value is the counter-in-RAM
            -- signature; anything past `before` is the cart's own value + n.
            local ok = (seq ~= 1) or (st.before == 0)
            print(string.format("after reset:  ACKSEQ=%02X", seq))
            if ok then
                print("PASS: the sequence continued from the cartridge's own "
                      .. "ACKSEQ across a console reset")
                print("cfgreset: PASS")
            else
                print("FAIL: the sequence restarted -- the client is counting "
                      .. "in RAM, which a console reset clears")
            end
            st.done = true
            manager.machine:exit()
        end
    end
end)
