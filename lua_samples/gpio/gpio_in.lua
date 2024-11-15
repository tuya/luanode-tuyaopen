-- Read gpio pin
-- gpio.open(pin, mode, value)
-- for INPUT, value can be: PULLUP, PULLDOWN and FLOATING

gpio.open(3, gpio.INPUT, gpio.FLOATING) --     
ret, level = gpio.read(3)	            -- read gpio level
print(ret, level)                       -- ret: 0 is success
