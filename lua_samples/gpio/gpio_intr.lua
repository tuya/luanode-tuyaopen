-- connect GPIO5 to GPIO4
-- GPIO4 will be triggered when GPIO5 output different level

-- gpio.mode(pin, pin_type, intr_type, callback)
--   pin: pin to set
--   pin_type: the value can be: gpio.INPUT/gpio.OUTPUT/gpio.INT/gpio.INOUT
--   intr_type: how to trigger, the value is string, can be: "up"/"down"/"both"/"low"/"high"
--   callback: callback function

-- to remove a callback call: gpio.remove(pin)
-- to remove all GPIO ISR call: gpio.uninstall()

pin_intr = 4;
pin_out = 5;
callback = function()
  print("run callback");
end
gpio.mode(pin_intr, gpio.INT, "both", callback);
gpio.mode(pin_out, gpio.OUTPUT);
tmr.delay(2);
gpio.write(pin_out, 1);
tmr.delay(1);
gpio.write(pin_out, 0);
