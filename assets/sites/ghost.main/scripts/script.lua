-- Main script
function OnInit()
    print("Lua: Приложение запущено!")
    return "Привет из Lua!"
end

function OnTick(delta_time)
    -- Здесь будет логика для каждого кадра
    return delta_time * 10 --nonsense
end