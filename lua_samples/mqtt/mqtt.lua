-- mqtt.new(host, client_id, keepalive, connected_callback, [username], [password], [encrypt])
--   host: the mqtt host IP
--   client_id: client id
--   keepalive: keep alive seconds
--   connected_callback: a callback when connection established
--   username: optional argument, mqtt user name
--   password: 
--   encrypt: not support encrypt now

-- mqtt.on(handle, event, callback): the method is used to register callback to monitor the following event: 'data'/'publish'/'subscribe'/'disconnect'

-- mqtt.unsubscribe(handle, topic): unsubscribe a topic

-- mqtt.close(handle): use this method to close an mqtt connection

-- Note: 
--   1. the mqtt.new retern a handle, which is used for subscribe/publish, remember to save the handle.
--   2. you can create 3 mqtt connections at most. You will fail to create more than 3 mqtt connections.


-- ccb=function() print("mqtt connected\n"); end
-- dcb=function(t,d) print(t);print(d); end
-- wifi.setmode(1);
-- wifi.start();
-- wifi.sta.config({ssid="Doit",pwd="doit3305"});
-- tmr.delay(15);
-- handle=mqtt.new('test.mosquitto.org','c1',600,ccb);  -- if you don't need connect callback, write like this: mqtt.new('test.mosquitto.org','c1',600,nil)
-- tmr.delay(8);
-- mqtt.subscribe(handle,'/chann1',mqtt.QOS0);
-- tmr.delay(3);
-- mqtt.on(handle,'data',dcb);
-- mqtt.publish(handle,'/chann1','test');

-- new a client and connect to server
c = mqtt.new() 

-- upload broker.emqx.io.ca.crt to target board
-- for ssl mqtt, need change the port from 1883 to 8883
-- fd = io.open("broker.emqx.io.ca.crt")
-- cert = fd:read('a')
-- fd:close()
-- c:ca(cert)

-- connect
c:on("conack", function () print("lua recv connect ack") end)
c:on("disconack", function () print("lua recv dis-connect ack") end)
c:connect({host="broker.emqx.io", port=1883, client_id="tuya-open-sdk-for-device-01", user_name="emqx", passwd="public"})

-- subscribe
c:on("message", function (t,q,l,p,u) print(t,q,l,p,u) end)
c:on("suback", function () print("lua recv subscibe ack") end)
c:subscribe("test", 0) 
c:subscribe("test1", 1)

-- unsubscribe
c:on("unsuback", function () print("lua recv unsubscibe ack") end)
c:unsubscribe("test", 0)
c:unsubscribe("test1", 1)

-- publish, sync and asyc
c:on("puback", function () print("lua recv publish ack") end)
c:on("publish", function () print("lua process publish") end)
c:publish("test2", "a publish example") -- qos=0, async publish
c:publish("test2", "another publish example", {qos=1}) -- qos=1, async publish
c:publish("test2", "another publish example", {qos=1}) -- qos=1, with a publish callback async publish
c:publish("test2", "another publish example", {qos=1, sync=1}) -- qos=1, sync publish
c:publish("test2", "another publish example", {qos=1, timeout_ms=5000}) -- qos=1, sync publish, with 5000ms timeout(default is 8000ms)
