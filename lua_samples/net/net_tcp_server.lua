-- Create a TCP server

PORT = 8181;
ADDR = "192.168.4.1";

wifi.setmode(wifi.SOFTAP);
wifi.start();

tmr.delay(2);
srv = net.createServer(net.TCP, 30);
srv:listen(PORT, ADDR, function(sock)
  sock:on("receive", function(sock, c)
    print(c);
  end)
end)
