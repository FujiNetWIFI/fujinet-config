-- shot.lua: snapshots for art and palette review (make shot, BARTEST=1).
-- SHOT_FRAMES is a comma list of frames to snap (default 150,420); the
-- machine exits 30 frames after the last one. Frames, not seconds:
-- machine.time.seconds is an integer in this MAME.
local want = {}
for f in string.gmatch(os.getenv("SHOT_FRAMES") or "150,420", "%d+") do
    want[#want + 1] = tonumber(f)
end
local frame, idx = 0, 1
_G.SHOT_SUB = emu.add_machine_frame_notifier(function()
    frame = frame + 1
    if idx <= #want and frame == want[idx] then
        manager.machine.video:snapshot()
        idx = idx + 1
    elseif idx > #want and frame >= want[#want] + 30 then
        manager.machine:exit()
    end
end)
