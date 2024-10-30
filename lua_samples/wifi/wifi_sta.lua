-- Wifi station test
-- Suppose you have a TCP server, whose IP is 192.168.99.218,
-- and it listens the PORT 8181, then you can run the following
-- source to create a client to connect to the server.

PORT = 8181
ADDR = "192.168.99.218"

recv_cb = function(sock, c)
  print(c);
end

wifi.setmode(wifi.STATION);
wifi.start();
wifi.sta.config({ssid="newifi_doit3305", pwd="doit3305"});
tmr.delay(10);
info = wifi.sta.getconfig();
count = 0;
timeout = false;
while(string.len(info.ssid) == 0) do	-- wait for connection established
  tmr.delay(1);
  info = wifi.sta.getconfig();
  count = count + 1;
  if(count == 3) then 
    print("Connect timeout");
    timeout = true;
    break; 
  end
end

if(timeout ~= true) then
  conn = net.createConnection(net.TCP, 0);
  conn:on("receive", recv_cb);
  conn:connect(PORT, ADDR);
  print("Connect to server");
end