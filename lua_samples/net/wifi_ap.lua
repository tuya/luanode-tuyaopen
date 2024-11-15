-- test luanode-tuyaopen network module
-- test Wi-Fi ap functions if the network connction type is Wi-Fi, 
-- note:
--     the network connection module was default starup in tuyaos demo
--     so you don't need to call the conn.init() if you already call tuyaos.start_device()

-- init the network connection module
conn.init()

-- check if the network have Wi-Fi connection
conn.current()

-- start a open ap
-- for a WPA/WPA2 ap: ap_info = {ssid="test-ssid", passwd="1Q@w3e$R", ip="192.168.176.1"}
ap_info = {ssid="test-ssid", ip="192.168.176.1"}
conn.start_ap(ap_info)

-- stop the ap
conn.stop_ap()



