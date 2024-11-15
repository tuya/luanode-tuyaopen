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
        s = socket.tcp()
        s:settimeout(0)
        s:bind("*", 6666)
        s:listen(2)
        while true do
            -- accept a cleint
            c = s:accept()
            c:settimeout(10)
            line, err = c:receive()
            if not err then
                print("Receive from client "..line)
                c:send("hello, this is tcp server!")
            end

            -- close client
            c:close()
        end
    end

    -- delay 1000ms waiting for connecting
    sys.delay_ms(1000)
end