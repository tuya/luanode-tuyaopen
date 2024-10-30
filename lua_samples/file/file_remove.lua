-- Remove file

res = file.open("test.lua", "w");

while true do
  if(res == false) then
	print("Open file failed");
	break;
  end

  file.close();
  file.remove("test.lua");	-- remove file

  break;
end
