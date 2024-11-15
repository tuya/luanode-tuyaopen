-- luaproc: https://github.com/askyrme/luaproc/blob/master/README.md
-- The thread will run a string
-- check how many package was loaded in package.preload
luaproc.newproc("for k,v in pairs(package.loaded) do print(k,v) end")
