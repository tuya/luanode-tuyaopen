-- FS information, format operation
-- List
for k,v in pairs(io.list("/")) do print(k, v) end

-- exist
print(io.exists("init.lua"))
print(io.exists("test.lua"))

-- size
print(io.filesize("init.lua"))
