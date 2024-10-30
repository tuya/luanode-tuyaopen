-- NVS: Non-volatile storage, the data store in NVS will not be cleaned when power off
-- This is a Lua module. User can use it to access NVS from Lua.
-- The following methods are provided by this module:
--	nvs.open()
--	nvs.close()
--	nvs.put(key, value)
--	nvs.get(key)
--	nvs.exist(key)
--	nvs.remove(key)
--	nvs.clearall()

nvs.open();
nvs.put("k","hello");
v=nvs.get("k");
print(v);		-- this will print out "hello"
print(nvs.exist("k"));	-- will output "1", if key exists nvs.exist return 1, otherwise, return 0
nvs.remove("k");	-- if remove successfully, nvs.remove return 0, otherwise, return 1
print(nvs.exist("k"));	-- will output "0"
nvs.close();

