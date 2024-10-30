-- test lua base module
-- such as sting, table

print("Hello world")

str="test";
print(string.len(str));		-- lua string module
print(string.sub(str, 1, 2));

t={11,22,33,44,55};
for k,v in pairs(t) do print(k,v); end
table.remove(t, 3);		-- use table module to remove the third element from table t
for k,v in pairs(t) do print(k,v); end
table.insert(t, 2, 99);	-- insert 99 to t at index 2


foo = function()
  print("This is a function");
end
foo();	-- function test
