-- Read an existing file

fd = io.open("init.lua", "r");

while true do
  if(res == false) then 
    print("Open file failed");
    break;
  end
  
  -- read 64 character from the file
  content = fd:read(64);
  print(content);
  fd:close();

  break;
end
