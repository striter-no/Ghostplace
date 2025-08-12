---@param ... any
function intlog(...) end

---@class Event
---@field type string
---@field mouse_x integer
---@field mouse_y integer
---@field btn integer
---@field unich integer
---@field elem_uid integer
---@field alted boolean
---@field shifted boolean
---@field ctrled boolean

-- Main script
function OnInit()
    print("OnInit was called !2!");
end

function OnTick(delta_time)
    -- print("current dt" .. delta_time)
    local a = delta_time + 1
end

---@param event Event
function OnEvent(event)
    if event.type == "key" then
        intlog("key event")
    elseif event.type == "mouse" then
        intlog("mouse event")
    elseif event.type == "elem" then
        intlog("elen event")
    end
end