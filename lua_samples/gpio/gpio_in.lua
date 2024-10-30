-- Read gpio pin

gpio.mode(3, gpio.INPUT);
level = gpio.read(3);	-- read gpio level
print(level);
