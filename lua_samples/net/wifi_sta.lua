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
ssid = "test-wifi-ssid"
passwd = "test-wifi-passwd"
conn.connect(ssid, passwd)

-- stop the ap
while true do
    isup = conn.isup()
    if isup then
        print("connect to "..ssid.." passwd "..passwd.." success!")
        break
    end

    -- delay 1000ms waiting for connecting
    sys.delay_ms(1000)
end

conn.disconnect()

