-- luaproc: https://github.com/askyrme/luaproc/blob/master/README.md
-- Thread run only once
-- The thread will run a string
fd = io.open("test.lua", "w")
fd:write("print('hello, world!')")
fd:close()

luaproc.newproc("dofile('test.lua')")
