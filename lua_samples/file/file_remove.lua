-- Remove file

fd = io.open("test.lua", "w")
if fd then
    fd:close()
    for k,v in pairs(io.list("/")) do print(k, v) end
    io.remove("test.lua")
    for k,v in pairs(io.list("/")) do print(k, v) end
end
