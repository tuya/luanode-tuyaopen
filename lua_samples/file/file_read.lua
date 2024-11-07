-- Read an existing file

-- read all character from the file
fd = io.open("init.lua", "r")
if fd then
    content = fd:read('a')
    print(content)

    fd:seek("cur", -#content)
    content = fd:read(10)
    print(content)

    content = fd:read('L')
    print(content)
    fd:close()
end
