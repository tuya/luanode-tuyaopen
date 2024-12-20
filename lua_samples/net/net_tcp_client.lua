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
        c = socket.tcp()
        c:settimeout(0)

        -- connect to server
        local serverAddress = "127.0.0.1"
        local serverPort = 6666
        local success, err = c:connect(serverAddress, serverPort) 
        if not success then
            print("Failed to connect to server: " .. err)
            break
        end

        -- send message to server
        local message = "Hello, Server!"
        c:send(message .. "\n")    

        -- receive message from server
        local response, err = c:receive()
        if not err then
            print("Received from server: " .. response)
        else
            print("Failed to receive from server: " .. err)
        end

        c:close()
    end

    -- delay 1000ms waiting for connecting
    sys.delay_ms(1000)
end