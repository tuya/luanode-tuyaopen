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
        fd = socket.udp()
        fd:settimeout(5)

        -- server info
        local serverAddress = "127.0.0.1"
        local serverPort = 6666
        local message = "Hello, UDP Server!"

        -- send message to server
        fd:sendto(message, serverAddress, serverPort)
        print("Message sent to " .. serverAddress .. ":" .. serverPort)


        local response, err = fd:receive()
        if response then
            print("Received response: " .. response)
        else
            print("Failed to receive response: " .. err)
        end

        fd:close()
    end

    -- sleep 1000ms 
    sys.sleep_ms(1000)
end