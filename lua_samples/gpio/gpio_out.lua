-- GPIO sample
-- gpio.open(pin, mode, value)
-- value is default status, for OUTPUT, value can be: LOW, HIGH

gpio.mode(2, gpio.OUTPUT, gpio.LOW);
gpio.write(2, gpio.HIGH);  -- led on
gpio.write(2, gpio.LOW);  -- led off

