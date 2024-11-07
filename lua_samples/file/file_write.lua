-- file create and write
-- To write a file, you should open it first 

fd = io.open("test.lua","w")  
if fd then
  fd:write("hello")
  fd:write(" ")
  fd:write("world")
  fd:close()
  
  fd = io.open("test.lua","r") 
  content = fd:read('a')
  print(content, #content)
  fd:close()
  io.remove("test.lua")
end


