-- test luanode-tuyaopen network connections module
-- note:
--     the network connection module was default starup in tuyaos demo
--     so you don't need to call the conn.init() if you already call tuyaos.start_device()

-- init the network connection module
-- it will manage the network connections of the board, Wi-Fi or wired.
conn.init()

-- get the network connection type
conn.supported()

-- check the network status
conn.isup()             -- the connection status
-- conn.isup("wifi")       -- the connection status of Wi-Fi link (if have Wi-Fi link)
-- conn.isup("wired")      -- the connection status of wired link (if have wired link)

-- get the network info
conn.ip()
conn.mac()


-- change the default network connction
-- this command works only when more than 1 connections up
-- conn.switch("wired")
-- conn.switch("wifi")