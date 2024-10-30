-- FS information, format operation

size = file.fsinfo();
print(size);	-- print total size of the FS

-- print FS detail
detail = file.list();
for k,v in pairs(detail) do
  print(k, v);
end


-- format FS, remove all content in FS
file.format();
detail = file.list();
for k,v in pairs(detail) do
  print(k, v);	-- nothing to print
end
