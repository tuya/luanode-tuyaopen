-- test luanode-tuyaopen network module
-- test lua-socket udp server
-- note:
--     the network connection module was default starup in tuyaos demo
--     so you don't need to call the conn.init() if you already call tuyaos.start_device()

-- init the network connection module
-- it will manage the network connections of the board, Wi-Fi or wired.
conn.init()

-- if the board connect to network use Wi-Fi, we should connect to router first
-- conn.connect("test-wifi-ssid", "test-wifi-passwd")

-- wait until the Wi-Fi link is up
while true do
    -- get the linkstatus
    isup = conn.isup()
    if isup then
        -- if network is valid, start a udp server
        -- send broadcast very 5sec
        -- when receive a "stop" message, exit
        fd = socket.udp()
        fd:setoption("broadcast", true)
        fd:setsockname("0.0.0.0", 6666)
        fd:settimeout(0)
        print("UDP server listening on " .. host .. ":" .. port)
        
        -- broadcast interval is 5sec
        local broadcastAddress = "255.255.255.255" -- broadcast address
        local broadcastPort = 6666                 -- src port is same as dst oirt  
        local lastBroadcastTime = socket.gettime()
        local broadcastInterval = 5 
        
        -- send broadcast
        while true do
            -- check if ontime, send broadcast
            local currentTime = socket.gettime()
            if currentTime - lastBroadcastTime >= broadcastInterval then
                local message = "Hello, this is a broadcast message!"
                fd:sendto(message, broadcastAddress, broadcastPort)
                print("Broadcast message sent.")
                lastBroadcastTime = currentTime
            end
        
            -- recv
            local data, ip, port = fd:receivefrom()
            if data then
                print("Received: " .. data .. " from " .. ip .. ":" .. port)
                if data == "stop" then
                    fd:sendto("Broadcast server exited!", ip, port)
                    break
                end
            end
        
            -- sleep 100 ms
            sys.sleep_ms(100)
        end    

        fd:close()
        break
    end

    -- sleep 1000ms 
    sys.sleep_ms(1000)
end